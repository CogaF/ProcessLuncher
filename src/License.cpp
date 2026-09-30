/*
 * Copyright (C) 2026 Fation Coga
 * SPDX-License-Identifier: LicenseRef-Proprietary
 * Shared license code: this file is identical in SerialPortManager and Reactor Control (see LICENSING.md).
 */

/*!
 * \file License.cpp
 * \brief Implementation of License.h.
 */

#include "License.h"
#include "HashUtils.h"
#include "TextUtils.h"

#include <algorithm>
#include <cstdio>
#include <map>

namespace {
	// Domain separation: the signature is over this header plus the license lines, so a signature
	// made by the same key for anything else can never pass as a license signature.
	constexpr std::string_view kMessageHeader = "SignedLicense-v2\n";
	constexpr std::string_view kFormat = "2";
	constexpr std::string_view kRequestFormat = "request-1";
	constexpr std::string_view kNever = "never";
	// Changing these changes every UID (and so invalidates every license issued).
	constexpr std::string_view kUidSalt = "|machine-uid-v2|";
	constexpr std::string_view kUidCheckSalt = "uid-check|";
	constexpr char kBase32[] = "0123456789ABCDEFGHJKMNPQRSTVWXYZ"; // Crockford
	constexpr size_t kUidDataChars = 30;
	constexpr size_t kUidChars = 32;

	std::string base32Bits(const uint8_t* bytes, size_t chars) {
		std::string out;
		for (size_t c = 0; c < chars; ++c) {
			unsigned value = 0;
			for (size_t b = 0; b < 5; ++b) {
				const size_t bit = c * 5 + b;
				value = (value << 1) | ((bytes[bit / 8] >> (7 - bit % 8)) & 1u);
			}
			out += kBase32[value];
		}
		return out;
	}

	std::string uidChecksum(std::string_view data) {
		const std::string input = std::string(kUidCheckSalt) + std::string(data);
		const auto digest = Utils::Hash::sha256(input.data(), input.size());
		return base32Bits(digest.data(), 2);
	}

	// Value text safe for one license line: no line breaks, no surrounding blanks.
	std::string oneLine(std::string_view value) {
		std::string s(value);
		std::replace(s.begin(), s.end(), '\r', ' ');
		std::replace(s.begin(), s.end(), '\n', ' ');
		return Utils::Str::trim(s);
	}

	bool validKey(std::string_view key) {
		if (key.empty()) return false;
		return std::all_of(key.begin(), key.end(), [](char c) { return (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9') || c == '_'; });
	}

	/*! \brief A license or request text split into its lines (see License.h for the rules). */
	struct Parsed {
		std::vector<std::string> signedLines;          // normalized, in order
		std::multimap<std::string, std::string> values; // KEY -> value
		std::string signature;                          // SIG value
		bool hasSignature = false;
		std::string error;                              // structural problem, empty if none
	};

	Parsed parseLines(std::string_view text) {
		Parsed p;
		if (text.substr(0, 3) == "\xEF\xBB\xBF") text.remove_prefix(3); // UTF-8 BOM (Notepad)
		for (const std::string& raw : Utils::Str::split(text, '\n')) {
			const std::string line = Utils::Str::trim(raw); // also drops the '\r' of CRLF files
			if (line.empty()) continue;
			if (p.hasSignature) { p.error = "text after SIG"; return p; }
			if (line[0] == '#') { p.signedLines.push_back(line); continue; }
			const size_t eq = line.find('=');
			if (eq == std::string::npos) { p.error = "line without '=': " + Utils::Str::ellipsize(line, 40); return p; }
			const std::string key = Utils::Str::toUpper(Utils::Str::trim(std::string_view(line).substr(0, eq)));
			const std::string value = Utils::Str::trim(std::string_view(line).substr(eq + 1));
			if (!validKey(key)) { p.error = "invalid key: " + Utils::Str::ellipsize(key, 40); return p; }
			if (key == "SIG") { p.signature = value; p.hasSignature = true; continue; }
			if (key != "NOTE" && p.values.count(key) != 0) { p.error = "duplicate " + key; return p; }
			p.values.emplace(key, value);
			p.signedLines.push_back(key + "=" + value);
		}
		return p;
	}

	std::string signedMessage(const std::vector<std::string>& lines) {
		std::string message(kMessageHeader);
		for (const std::string& line : lines) message += line + "\n";
		return message;
	}

	std::string value(const Parsed& p, const std::string& key) {
		const auto it = p.values.find(key);
		return it == p.values.end() ? std::string() : it->second;
	}

	std::vector<std::string> listOf(std::string_view text) {
		std::vector<std::string> out;
		for (const std::string& item : Utils::Str::split(text, ',', false)) {
			std::string t = Utils::Str::trim(item);
			if (!t.empty()) out.push_back(t);
		}
		return out;
	}

	// Fills info from the parsed values; returns the first problem found (empty if none).
	std::string readInfo(const Parsed& p, License::Info& info) {
		std::string problem;
		auto fail = [&](const std::string& what) { if (problem.empty()) problem = what; };

		info.product = value(p, "PRODUCT");
		info.id = value(p, "LICENSE_ID");
		info.keyId = value(p, "KEY_ID");
		info.licensee = value(p, "LICENSEE");
		info.email = value(p, "EMAIL");
		info.edition = value(p, "EDITION");
		for (auto [it, end] = p.values.equal_range("NOTE"); it != end; ++it) info.notes.push_back(it->second);

		if (value(p, "FORMAT") != kFormat) fail("FORMAT");
		if (info.product.empty()) fail("PRODUCT");
		if (info.id.empty()) fail("LICENSE_ID");
		if (info.licensee.empty()) fail("LICENSEE");
		if (info.keyId.empty()) fail("KEY_ID");

		const std::string features = value(p, "FEATURES");
		if (features == "*") info.allFeatures = true;
		else info.features = listOf(features);
		if (!info.allFeatures && info.features.empty()) fail("FEATURES");

		const std::string machines = value(p, "MACHINES");
		if (machines == "*") info.anyMachine = true;
		else {
			for (const std::string& m : listOf(machines)) {
				if (auto uid = License::normalizeUid(m)) info.machines.push_back(*uid);
				else fail("MACHINES");
			}
			if (info.machines.empty()) fail("MACHINES");
		}

		if (auto d = License::parseDate(value(p, "ISSUED"))) info.issued = *d; else fail("ISSUED");
		auto optionalDate = [&](const char* key, std::optional<int64_t>& out) {
			const std::string text = value(p, key);
			if (Utils::Str::equalsIgnoreCase(text, kNever)) out.reset();
			else if (auto d = License::parseDate(text)) out = *d;
			else fail(key);
		};
		optionalDate("EXPIRES", info.expires);
		optionalDate("UPDATES_UNTIL", info.updatesUntil);
		return problem;
	}
}

namespace License {

// ------------------------------------------------------------------------------------------------
// Dates
// ------------------------------------------------------------------------------------------------

int64_t daysFromCivil(int year, int month, int day) {
	// Howard Hinnant's days_from_civil.
	const int64_t y = static_cast<int64_t>(year) - (month <= 2 ? 1 : 0);
	const int64_t era = (y >= 0 ? y : y - 399) / 400;
	const int64_t yoe = y - era * 400;
	const int64_t mp = (month + 9) % 12;
	const int64_t doy = (153 * mp + 2) / 5 + day - 1;
	const int64_t doe = yoe * 365 + yoe / 4 - yoe / 100 + doy;
	return era * 146097 + doe - 719468;
}

std::optional<int64_t> parseDate(std::string_view text) {
	const std::string t = Utils::Str::trim(text);
	if (t.size() != 10 || t[4] != '-' || t[7] != '-') return std::nullopt;
	for (size_t i : { 0u, 1u, 2u, 3u, 5u, 6u, 8u, 9u })
		if (t[i] < '0' || t[i] > '9') return std::nullopt;
	const int y = std::stoi(t.substr(0, 4)), m = std::stoi(t.substr(5, 2)), d = std::stoi(t.substr(8, 2));
	if (m < 1 || m > 12 || d < 1 || d > 31) return std::nullopt;
	const int64_t day = daysFromCivil(y, m, d);
	if (formatDate(day) != t) return std::nullopt; // 2026-02-30 and the like
	return day;
}

std::string formatDate(int64_t day) {
	// Howard Hinnant's civil_from_days.
	const int64_t z = day + 719468;
	const int64_t era = (z >= 0 ? z : z - 146096) / 146097;
	const int64_t doe = z - era * 146097;
	const int64_t yoe = (doe - doe / 1460 + doe / 36524 - doe / 146096) / 365;
	const int64_t doy = doe - (365 * yoe + yoe / 4 - yoe / 100);
	const int64_t mp = (5 * doy + 2) / 153;
	const int64_t d = doy - (153 * mp + 2) / 5 + 1;
	const int64_t m = mp < 10 ? mp + 3 : mp - 9;
	const int64_t y = yoe + era * 400 + (m <= 2 ? 1 : 0);
	char buffer[16];
	std::snprintf(buffer, sizeof(buffer), "%04d-%02d-%02d", static_cast<int>(y), static_cast<int>(m), static_cast<int>(d));
	return buffer;
}

// ------------------------------------------------------------------------------------------------
// Machine UID
// ------------------------------------------------------------------------------------------------

std::string uidFromMachineSecret(std::string_view product, std::string_view machineSecret) {
	if (machineSecret.empty()) return {};
	const std::string input = std::string(product) + std::string(kUidSalt) + std::string(machineSecret);
	const auto digest = Utils::Hash::sha256(input.data(), input.size());
	const std::string data = base32Bits(digest.data(), kUidDataChars);
	return data + uidChecksum(data);
}

std::optional<std::string> normalizeUid(std::string_view text) {
	std::string uid;
	for (char c : text) {
		if (c == '-' || c == ' ' || c == '\t') continue;
		if (c >= 'a' && c <= 'z') c = static_cast<char>(c - 'a' + 'A');
		if (c == 'O') c = '0';
		if (c == 'I' || c == 'L') c = '1';
		if (std::string_view(kBase32).find(c) == std::string_view::npos) return std::nullopt;
		uid += c;
	}
	if (uid.size() != kUidChars) return std::nullopt;
	if (uidChecksum(std::string_view(uid).substr(0, kUidDataChars)) != uid.substr(kUidDataChars)) return std::nullopt;
	return uid;
}

std::string displayUid(std::string_view uid) {
	std::string out;
	for (size_t i = 0; i < uid.size(); ++i) {
		if (i > 0 && i % 4 == 0) out += '-';
		out += uid[i];
	}
	return out;
}

// ------------------------------------------------------------------------------------------------
// Hex
// ------------------------------------------------------------------------------------------------

std::string toHex(const uint8_t* data, size_t size) {
	static constexpr char kDigits[] = "0123456789abcdef";
	std::string out;
	out.reserve(size * 2);
	for (size_t i = 0; i < size; ++i) { out += kDigits[data[i] >> 4]; out += kDigits[data[i] & 0x0F]; }
	return out;
}

std::optional<std::vector<uint8_t>> fromHex(std::string_view hex) {
	auto nibble = [](char c) -> int {
		if (c >= '0' && c <= '9') return c - '0';
		if (c >= 'a' && c <= 'f') return c - 'a' + 10;
		if (c >= 'A' && c <= 'F') return c - 'A' + 10;
		return -1;
	};
	if (hex.size() % 2 != 0) return std::nullopt;
	std::vector<uint8_t> out;
	out.reserve(hex.size() / 2);
	for (size_t i = 0; i < hex.size(); i += 2) {
		const int hi = nibble(hex[i]), lo = nibble(hex[i + 1]);
		if (hi < 0 || lo < 0) return std::nullopt;
		out.push_back(static_cast<uint8_t>(hi * 16 + lo));
	}
	return out;
}

// ------------------------------------------------------------------------------------------------
// License
// ------------------------------------------------------------------------------------------------

bool Info::hasFeature(std::string_view feature) const {
	return allFeatures || std::find(features.begin(), features.end(), feature) != features.end();
}

bool Info::coversMachine(std::string_view uid) const {
	if (anyMachine) return true;
	if (uid.empty()) return false;
	return std::find(machines.begin(), machines.end(), uid) != machines.end();
}

std::string sign(const Info& info, const Utils::Crypto::Ed25519::Seed& seed) {
	std::vector<std::string> lines;
	lines.push_back("# " + oneLine(info.product) + " license - any change to the lines below makes it invalid.");
	auto add = [&](const char* key, std::string_view v) { lines.push_back(std::string(key) + "=" + oneLine(v)); };
	add("FORMAT", kFormat);
	add("PRODUCT", info.product);
	add("LICENSE_ID", info.id);
	add("LICENSEE", info.licensee);
	if (!oneLine(info.email).empty()) add("EMAIL", info.email);
	if (!oneLine(info.edition).empty()) add("EDITION", info.edition);
	add("FEATURES", info.allFeatures ? std::string("*") : Utils::Str::join(info.features, ","));
	std::vector<std::string> machines;
	for (const std::string& m : info.machines) machines.push_back(displayUid(m));
	add("MACHINES", info.anyMachine ? std::string("*") : Utils::Str::join(machines, ","));
	add("ISSUED", formatDate(info.issued));
	add("EXPIRES", info.expires ? formatDate(*info.expires) : std::string(kNever));
	add("UPDATES_UNTIL", info.updatesUntil ? formatDate(*info.updatesUntil) : std::string(kNever));
	for (const std::string& note : info.notes)
		if (!oneLine(note).empty()) add("NOTE", note);
	add("KEY_ID", info.keyId);

	const std::string message = signedMessage(lines);
	const auto signature = Utils::Crypto::Ed25519::sign(message.data(), message.size(), seed);
	std::string text;
	for (const std::string& line : lines) text += line + "\n";
	text += "SIG=" + toHex(signature.data(), signature.size()) + "\n";
	return text;
}

const char* statusName(Status status) {
	switch (status) {
	case Status::Valid:             return "valid";
	case Status::Missing:           return "missing";
	case Status::Unreadable:        return "unreadable";
	case Status::Malformed:         return "malformed";
	case Status::WrongProduct:      return "wrong product";
	case Status::NoKeys:            return "no verification key in this build";
	case Status::UnknownKey:        return "unknown key";
	case Status::BadSignature:      return "bad signature";
	case Status::Revoked:           return "revoked";
	case Status::NoMachineId:       return "no machine id";
	case Status::OtherMachine:      return "other machine";
	case Status::ClockRolledBack:   return "clock set back";
	case Status::Expired:           return "expired";
	case Status::UpdatesNotCovered: return "version not covered";
	}
	return "?";
}

Evaluation evaluate(std::string_view text, const Context& context) {
	Evaluation e;
	const Parsed p = parseLines(text);
	if (!p.error.empty()) { e.status = Status::Malformed; e.detail = p.error; return e; }
	const std::string fieldProblem = readInfo(p, e.info);
	if (!p.hasSignature) { e.status = Status::Malformed; e.detail = "SIG"; return e; }
	if (e.info.product != context.product) {
		e.status = e.info.product.empty() ? Status::Malformed : Status::WrongProduct;
		e.detail = "PRODUCT";
		return e;
	}

	// Authenticity first: nothing else in the file means anything until the signature holds.
	if (context.keys.empty()) { e.status = Status::NoKeys; return e; }
	const auto key = std::find_if(context.keys.begin(), context.keys.end(),
		[&](const PublicKeyEntry& k) { return k.id && e.info.keyId == k.id; });
	const auto keyBytes = key == context.keys.end() || !key->hex ? std::nullopt : fromHex(key->hex);
	if (!keyBytes || keyBytes->size() != 32) { e.status = Status::UnknownKey; return e; }
	const auto sigBytes = fromHex(p.signature);
	if (!sigBytes || sigBytes->size() != 64) { e.status = Status::Malformed; e.detail = "SIG"; return e; }

	Utils::Crypto::Ed25519::PublicKey publicKey{};
	std::copy(keyBytes->begin(), keyBytes->end(), publicKey.begin());
	Utils::Crypto::Ed25519::Signature signature{};
	std::copy(sigBytes->begin(), sigBytes->end(), signature.begin());
	const std::string message = signedMessage(p.signedLines);
	if (!Utils::Crypto::Ed25519::verify(signature, message.data(), message.size(), publicKey)) {
		e.status = Status::BadSignature;
		return e;
	}
	e.authentic = true;
	if (!fieldProblem.empty()) { e.status = Status::Malformed; e.detail = fieldProblem; return e; }

	if (std::find(context.revokedIds.begin(), context.revokedIds.end(), e.info.id) != context.revokedIds.end()) {
		e.status = Status::Revoked;
		return e;
	}
	if (context.checkMachine && !e.info.anyMachine) {
		if (context.machineUid.empty()) { e.status = Status::NoMachineId; return e; }
		if (!e.info.coversMachine(context.machineUid)) { e.status = Status::OtherMachine; return e; }
	}
	if (e.info.expires) {
		// A clock set back (or a date before the license was even issued) could stretch a
		// time-limited license forever; a perpetual one does not care.
		if (context.clockRolledBack || context.today + 1 < e.info.issued) { e.status = Status::ClockRolledBack; return e; }
		if (context.today > *e.info.expires) { e.status = Status::Expired; return e; }
	}
	if (e.info.updatesUntil && context.releaseDay > *e.info.updatesUntil) {
		e.status = Status::UpdatesNotCovered;
		return e;
	}
	e.status = Status::Valid;
	return e;
}

// ------------------------------------------------------------------------------------------------
// Request
// ------------------------------------------------------------------------------------------------

std::string formatRequest(const Request& r) {
	std::string text = "# " + oneLine(r.product) + " license request - send this file to the license issuer.\n";
	text += "FORMAT=" + std::string(kRequestFormat) + "\n";
	text += "PRODUCT=" + oneLine(r.product) + "\n";
	text += "UID=" + displayUid(r.uid) + "\n";
	text += "COMPUTER=" + oneLine(r.computer) + "\n";
	text += "VERSION=" + oneLine(r.version) + "\n";
	text += "CREATED=" + oneLine(r.created) + "\n";
	if (!oneLine(r.currentLicense).empty()) text += "CURRENT_LICENSE=" + oneLine(r.currentLicense) + "\n";
	return text;
}

std::optional<Request> parseRequest(std::string_view text) {
	if (auto uid = normalizeUid(Utils::Str::trim(text))) { // just the UID, pasted
		Request r;
		r.uid = *uid;
		return r;
	}
	const Parsed p = parseLines(text);
	if (!p.error.empty()) return std::nullopt;
	auto uid = normalizeUid(value(p, "UID"));
	if (!uid) return std::nullopt;
	Request r;
	r.uid = *uid;
	r.product = value(p, "PRODUCT");
	r.computer = value(p, "COMPUTER");
	r.version = value(p, "VERSION");
	r.created = value(p, "CREATED");
	r.currentLicense = value(p, "CURRENT_LICENSE");
	return r;
}

} // namespace License
