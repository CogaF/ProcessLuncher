/*
 * Copyright (C) 2026 Fation Coga
 * SPDX-License-Identifier: LicenseRef-Proprietary
 * Shared license code: this file is identical in SerialPortManager and Reactor Control (see LICENSING.md).
 */

/*!
 * \file LicenseManager.cpp
 * \brief Implementation of LicenseManager.h.
 */

#include "LicenseManager.h"
#include "HashUtils.h"
#include "LicenseHost.h"
#include "LicenseKeys.h"
#include "LicensePolicy.h"
#include "MachineId.h"
#include "TextUtils.h"

#include <algorithm>
#include <ctime>
#include <system_error>

#ifdef _WIN32
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#endif

namespace {
	constexpr const char* kLicenseFileName = "license.lic";
	constexpr const char* kRequestFileName = "license-request.txt";
	constexpr const char* kStateFileName = "license-state.dat";

	struct State {
		bool initialized = false;
		std::string uid;
		License::Evaluation evaluation;
		std::filesystem::path licenseFile;
		int64_t checkedDay = 0;   // the date evaluation was made for
		int64_t trialFirst = 0;   // trial start (day)
		int64_t lastSeen = 0;     // newest date ever seen (day)
		bool tampered = false;
		bool rolledBack = false;
	};
	State& state() { static State s; return s; }

	int64_t today() {
		const std::time_t now = std::time(nullptr);
		std::tm local{};
#ifdef _WIN32
		localtime_s(&local, &now);
#else
		localtime_r(&now, &local);
#endif
		return License::daysFromCivil(local.tm_year + 1900, local.tm_mon + 1, local.tm_mday);
	}

	int64_t releaseDay() {
		const auto day = License::parseDate(kAppVersionHistory[0].date);
		return day ? *day : 0;
	}

	License::Context context() {
		License::Context c;
		c.product = LicensePolicy::kProduct;
		for (const License::PublicKeyEntry& k : kLicensePublicKeys)
			if (k.id) c.keys.push_back(k);
		for (const char* id : kRevokedLicenseIds)
			if (id) c.revokedIds.push_back(id);
		c.machineUid = state().uid;
		c.today = today();
		c.releaseDay = releaseDay();
		c.clockRolledBack = state().rolledBack;
		return c;
	}

	// --- Trial / clock state: "v1;<first day>;<last seen day>;<check>" in two places ---

	std::string stateCheck(int64_t first, int64_t last) {
		return Utils::Hash::sha256Hex(std::string(LicensePolicy::kProduct) + "|trial-state|" + state().uid + "|" +
			std::to_string(first) + "|" + std::to_string(last)).substr(0, 20);
	}

	std::string stateText(int64_t first, int64_t last) {
		return "v1;" + std::to_string(first) + ";" + std::to_string(last) + ";" + stateCheck(first, last);
	}

	// nullopt if text is empty (no copy); {0,0} marks a copy that exists but was edited.
	std::optional<std::pair<int64_t, int64_t>> parseState(const std::string& text) {
		const std::string t = Utils::Str::trim(text);
		if (t.empty()) return std::nullopt;
		const std::vector<std::string> parts = Utils::Str::split(t, ';');
		if (parts.size() == 4 && parts[0] == "v1") {
			const auto first = Utils::Str::toInt(parts[1]), last = Utils::Str::toInt(parts[2]);
			if (first && last && *first > 0 && *last >= *first && parts[3] == stateCheck(*first, *last))
				return std::make_pair(*first, *last);
		}
		return std::make_pair(int64_t(0), int64_t(0));
	}

#ifdef _WIN32
	std::wstring registryKey() {
		const std::string name = std::string("Software\\") + AppInfo::kName + "\\License";
		return std::wstring(name.begin(), name.end()); // the app name is ASCII
	}

	std::string readRegistryState() {
		wchar_t buffer[256] = {};
		DWORD size = sizeof(buffer);
		if (RegGetValueW(HKEY_CURRENT_USER, registryKey().c_str(), L"State", RRF_RT_REG_SZ, nullptr, buffer, &size) != ERROR_SUCCESS)
			return {};
		std::string out;
		for (const wchar_t* p = buffer; *p; ++p) out += static_cast<char>(*p < 128 ? *p : '?');
		return out;
	}

	void writeRegistryState(const std::string& text) {
		const std::wstring value(text.begin(), text.end());
		RegSetKeyValueW(HKEY_CURRENT_USER, registryKey().c_str(), L"State", REG_SZ, value.c_str(),
			static_cast<DWORD>((value.size() + 1) * sizeof(wchar_t)));
	}
#else
	std::string readRegistryState() { return {}; }
	void writeRegistryState(const std::string&) {}
#endif

	void loadTrialState() {
		State& s = state();
		const int64_t now = today();
		const auto fileText = Utils::Files::readText(DataDir::file(kStateFileName));
		std::vector<std::pair<int64_t, int64_t>> copies;
		for (const std::string& text : { fileText ? *fileText : std::string(), readRegistryState() }) {
			if (auto copy = parseState(text)) {
				if (copy->first == 0) s.tampered = true;
				else copies.push_back(*copy);
			}
		}
		if (copies.empty()) {
			// First start on this PC (or both copies edited: then the trial is simply over).
			s.trialFirst = now;
			s.lastSeen = now;
		}
		else {
			s.trialFirst = std::min_element(copies.begin(), copies.end())->first;
			s.lastSeen = 0;
			for (const auto& c : copies) s.lastSeen = std::max(s.lastSeen, c.second);
		}
		// An edited copy ends the trial - and the state written back says so, so it stays over.
		if (s.tampered) s.trialFirst = std::min(s.trialFirst, now - LicensePolicy::kTrialDays);
		s.rolledBack = now + LicensePolicy::kClockToleranceDays < s.lastSeen;
		if (!s.rolledBack) s.lastSeen = std::max(s.lastSeen, now);

		const std::string text = stateText(s.trialFirst, s.lastSeen);
		Utils::Files::writeTextAtomic(DataDir::file(kStateFileName), text + "\n");
		writeRegistryState(text);
	}

	std::filesystem::path findLicenseFile() {
		const std::filesystem::path inData = DataDir::file(kLicenseFileName);
		const std::filesystem::path besideExe = DataDir::exeDirectory() / kLicenseFileName;
		std::error_code ec;
		if (!std::filesystem::exists(inData, ec) && std::filesystem::exists(besideExe, ec)) return besideExe;
		return inData;
	}

	void evaluateLicenseFile() {
		State& s = state();
		s.licenseFile = findLicenseFile();
		s.checkedDay = today();
		std::error_code ec;
		if (!std::filesystem::exists(s.licenseFile, ec)) {
			s.evaluation = License::Evaluation();
			s.evaluation.status = License::Status::Missing;
			return;
		}
		const auto text = Utils::Files::readText(s.licenseFile);
		if (!text) {
			s.evaluation = License::Evaluation();
			s.evaluation.status = License::Status::Unreadable;
			return;
		}
		s.evaluation = License::evaluate(*text, context());
	}

	void logState() {
		const State& s = state();
		const License::Evaluation& e = s.evaluation;
		if (e.status == License::Status::Valid) {
			Log::info("License " + e.info.id + " valid, licensed to " + e.info.licensee +
				(e.info.expires ? ", expires " + License::formatDate(*e.info.expires) : std::string(", perpetual")));
			return;
		}
		std::string text = "No valid license (" + std::string(License::statusName(e.status));
		if (!e.detail.empty()) text += ": " + e.detail;
		text += ", file " + s.licenseFile.string() + ")";
		if (Licensing::trialDaysLeft() > 0) text += " - trial, " + std::to_string(Licensing::trialDaysLeft()) + " day(s) left";
		else if (LicensePolicy::kTrialDays > 0) text += " - trial over";
		Log::warning(text);
		if (s.tampered) Log::warning("The license trial state was edited - the trial is over.");
		if (s.rolledBack) Log::warning("The PC clock is set back (last date seen " + License::formatDate(s.lastSeen) +
			") - the trial and time-limited licenses are suspended until it is right.");
	}
}

namespace Licensing {

void initialize() {
	State& s = state();
	s.uid = MachineId::uid(LicensePolicy::kProduct);
	if (s.uid.empty()) Log::warning("This PC's machine id could not be read - licenses bound to a PC cannot be used.");
	loadTrialState();
	evaluateLicenseFile();
	s.initialized = true;
	logState();
	if (mode() != Mode::Licensed && !s.uid.empty()) {
		std::string error;
		if (!writeRequest(error)) Log::warning("License request not written: " + error);
	}
}

void refresh() {
	evaluateLicenseFile();
}

bool refreshIfDayChanged() {
	if (today() == state().checkedDay) return false;
	const Mode before = mode();
	loadTrialState(); // moves "last seen" forward, notices a clock set back
	evaluateLicenseFile();
	if (mode() != before) logState();
	return true;
}

Mode mode() {
	if (state().evaluation.status == License::Status::Valid) return Mode::Licensed;
	return trialDaysLeft() > 0 ? Mode::Trial : Mode::Unlicensed;
}

const License::Evaluation& evaluation() { return state().evaluation; }

bool allows(std::string_view feature) {
	switch (mode()) {
	case Mode::Licensed: return state().evaluation.info.hasFeature(feature);
	case Mode::Trial:    return true;
	default:             return false;
	}
}

int trialDaysLeft() {
	const State& s = state();
	if (LicensePolicy::kTrialDays <= 0 || s.tampered || s.rolledBack || s.trialFirst == 0) return 0;
	const int64_t left = s.trialFirst + LicensePolicy::kTrialDays - today();
	return left > 0 ? static_cast<int>(left) : 0;
}

bool trialStateTampered() { return state().tampered; }
bool clockRolledBack() { return state().rolledBack; }

int licenseDaysLeft() {
	const License::Evaluation& e = state().evaluation;
	if (e.status != License::Status::Valid || !e.info.expires) return -1;
	return static_cast<int>(std::max<int64_t>(0, *e.info.expires - today()));
}

const std::string& machineUid() { return state().uid; }
std::filesystem::path licenseFile() { return state().licenseFile; }
std::filesystem::path installedLicenseFile() { return DataDir::file(kLicenseFileName); }
std::filesystem::path requestFile() { return DataDir::file(kRequestFileName); }

License::Evaluation check(std::string_view licenseText) {
	return License::evaluate(licenseText, context());
}

bool install(std::string_view licenseText, License::Evaluation& result, std::string& error) {
	result = check(licenseText);
	if (result.status != License::Status::Valid) return false;
	const std::filesystem::path target = installedLicenseFile();
	std::error_code ec;
	if (std::filesystem::exists(target, ec)) std::filesystem::copy_file(target, target.string() + ".bak",
		std::filesystem::copy_options::overwrite_existing, ec);
	if (!Utils::Files::writeTextAtomic(target, licenseText)) {
		error = "cannot write " + target.string();
		return false;
	}
	evaluateLicenseFile();
	Log::info("License " + result.info.id + " installed (" + target.string() + ").");
	return true;
}

bool uninstall(std::string& error) {
	const std::filesystem::path target = installedLicenseFile();
	std::error_code ec;
	if (std::filesystem::exists(target, ec)) {
		const std::filesystem::path backup = target.string() + ".bak";
		std::filesystem::remove(backup, ec);
		std::filesystem::rename(target, backup, ec);
		if (ec) {
			error = "cannot remove " + target.string() + ": " + ec.message();
			return false;
		}
		Log::info("License removed (kept as " + target.string() + ".bak).");
	}
	evaluateLicenseFile();
	return true;
}

std::string requestText() {
	License::Request r;
	r.product = LicensePolicy::kProduct;
	r.uid = state().uid;
	r.computer = MachineId::computerName();
	r.version = GetAppVersion();
	r.created = License::formatDate(today());
	if (state().evaluation.authentic) r.currentLicense = state().evaluation.info.id;
	return License::formatRequest(r);
}

bool writeRequest(std::string& error) {
	if (state().uid.empty()) {
		error = "this PC's machine id could not be read";
		return false;
	}
	if (!Utils::Files::writeTextAtomic(requestFile(), requestText())) {
		error = "cannot write " + requestFile().string();
		return false;
	}
	return true;
}

} // namespace Licensing
