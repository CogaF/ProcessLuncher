/*
 * Copyright (C) 2026 Fation Coga
 * SPDX-License-Identifier: LicenseRef-Proprietary
 * Shared license code: this file is identical in SerialPortManager and Reactor Control (see LICENSING.md).
 */

#pragma once

#include <string>
#include <string_view>

/*!
 * \file MachineId.h
 * \brief Identifies the PC a license is bound to. Plain C++, shared by the application and the
 * license generator ("LicGenCli uid" prints the UID of the PC it runs on, for the product folder it uses).
 *
 * The machine secret is the Windows MachineGuid (HKLM\SOFTWARE\Microsoft\Cryptography), created
 * when Windows is installed: it survives hardware changes and updates, only a reinstall changes it.
 * Other systems use /etc/machine-id (the self-test runs there). The secret itself never leaves the
 * PC: only its salted hash, the UID (License::uidFromMachineSecret), is shown and sent.
 */
namespace MachineId {
	/*! \brief The raw machine secret (lower case), or empty if it could not be read. */
	std::string machineSecret();
	/*! \brief This PC's license UID for product (32 characters, see License.h), or empty if unknown. */
	std::string uid(std::string_view product);
	/*! \brief The computer's network name (written in license requests to recognise the PC). */
	std::string computerName();
}
