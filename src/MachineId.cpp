/*
 * Copyright (C) 2026 Fation Coga
 * SPDX-License-Identifier: LicenseRef-Proprietary
 * Shared license code: this file is identical in SerialPortManager and Reactor Control (see LICENSING.md).
 */

/*!
 * \file MachineId.cpp
 * \brief Implementation of MachineId.h.
 */

#include "MachineId.h"
#include "License.h"
#include "TextUtils.h"

#ifdef _WIN32
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#else
#include <unistd.h>
#endif

namespace {
#ifdef _WIN32
	std::string narrow(const wchar_t* text) {
		const int size = WideCharToMultiByte(CP_UTF8, 0, text, -1, nullptr, 0, nullptr, nullptr);
		if (size <= 1) return {};
		std::string out(static_cast<size_t>(size - 1), '\0');
		WideCharToMultiByte(CP_UTF8, 0, text, -1, out.data(), size, nullptr, nullptr);
		return out;
	}
#endif
}

namespace MachineId {

std::string machineSecret() {
#ifdef _WIN32
	wchar_t buffer[128] = {};
	DWORD size = sizeof(buffer);
	// RRF_SUBKEY_WOW6464KEY: the 64-bit registry view, where Windows keeps MachineGuid - also from a 32-bit build.
	if (RegGetValueW(HKEY_LOCAL_MACHINE, L"SOFTWARE\\Microsoft\\Cryptography", L"MachineGuid",
		RRF_RT_REG_SZ | RRF_SUBKEY_WOW6464KEY, nullptr, buffer, &size) != ERROR_SUCCESS) return {};
	return Utils::Str::toLower(Utils::Str::trim(narrow(buffer)));
#else
	for (const char* file : { "/etc/machine-id", "/var/lib/dbus/machine-id" }) {
		if (auto text = Utils::Files::readText(file)) {
			const std::string id = Utils::Str::toLower(Utils::Str::trim(*text));
			if (!id.empty()) return id;
		}
	}
	return {};
#endif
}

std::string uid(std::string_view product) {
	return License::uidFromMachineSecret(product, machineSecret());
}

std::string computerName() {
#ifdef _WIN32
	wchar_t buffer[MAX_COMPUTERNAME_LENGTH + 1] = {};
	DWORD size = MAX_COMPUTERNAME_LENGTH + 1;
	return GetComputerNameW(buffer, &size) ? narrow(buffer) : std::string();
#else
	char buffer[256] = {};
	return gethostname(buffer, sizeof(buffer) - 1) == 0 ? std::string(buffer) : std::string();
#endif
}

} // namespace MachineId
