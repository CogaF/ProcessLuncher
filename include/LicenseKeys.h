/*
 * Copyright (C) 2026 Fation Coga
 * SPDX-License-Identifier: LicenseRef-Proprietary
 * This file is part of Process Launcher.
 */

#pragma once

#include "License.h"

/*!
 * \file LicenseKeys.h
 * \brief The license verification keys and the revoked license IDs compiled into the application.
 *
 * Maintained by the license generator (see LICENSING.md) - it inserts lines above the "licgen:"
 * markers, so keep the markers:
 *  - New Signing Key (LicGen.exe with this folder as its product folder, or "LicGenCli keygen")
 *    creates a key pair: the PRIVATE half goes to privateData/ (git-ignored, never commit it), the
 *    public half is added here;
 *  - Revoke License ID (or "LicGenCli revoke <LICENSE_ID>") adds an ID that this and later builds refuse.
 *
 * The public keys are public by design: they can only CHECK a signature, never make one. Several
 * keys can be listed at once (key rotation): each license names its key in KEY_ID=, so licenses
 * signed with an older key keep working while new ones are signed with the newer key. Removing a
 * key invalidates every license signed with it.
 *
 * While no key is listed, no license can be valid (the License window says so); the trial works.
 * Process Launcher ships with NO key: create one with LicGen (see LICENSING.md) and rebuild.
 */

/*! \brief Verification keys; the entry with a null id ends the list. */
inline constexpr License::PublicKeyEntry kLicensePublicKeys[] = {
	{ "processluncher-2026a", "eb2bd2679006a61f14615d05e7cafee978de626faca7b9cfc84d6787acd6d2fb" },
	// licgen:public-keys
	{ nullptr, nullptr }
};

/*! \brief Revoked license IDs; nullptr ends the list. */
inline constexpr const char* kRevokedLicenseIds[] = {
	// licgen:revoked
	nullptr
};
