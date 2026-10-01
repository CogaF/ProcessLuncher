/*
 * Copyright (C) 2026 Fation Coga
 * SPDX-License-Identifier: LicenseRef-Proprietary
 * Shared license code: this file is identical in SerialPortManager and Reactor Control (see LICENSING.md).
 */

#pragma once

#include <cstdint>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

#include "Ed25519.h"

/*!
 * \file License.h
 * \brief The license file format: parsing, signing, verification and evaluation against a PC and a
 * date. Plain C++ (no wxWidgets, no Windows API), shared by the application (through
 * LicenseManager.h), the license generator (tools/LicenseGenerator) and the self-test.
 *
 * A license is a small UTF-8 text file (license.lic), readable and safe to send by e-mail:
 * \code
 * # SerialPortManager license - any change to the lines below makes it invalid.
 * FORMAT=2
 * PRODUCT=SerialPM
 * LICENSE_ID=SPM-20260927-4F2A91
 * LICENSEE=Universita di Padova
 * EMAIL=lab@example.org
 * EDITION=professional
 * FEATURES=*
 * MACHINES=7K2M-Q0PA-...,*
 * ISSUED=2026-09-27
 * EXPIRES=never
 * UPDATES_UNTIL=2027-09-27
 * NOTE=Lab 3, rig A
 * KEY_ID=k1
 * SIG=<128 hex chars: Ed25519 signature>
 * \endcode
 *
 * The signature covers every non-empty line before SIG= (comments included), trimmed, in order,
 * after a header naming the format - so nothing can be edited, added, removed or reordered.
 *  - FEATURES: comma-separated feature keys (LicensePolicy.h), or "*" for all, future ones included.
 *  - MACHINES: comma-separated machine UIDs (a license for several PCs), or "*" for any PC (site).
 *  - EXPIRES: last day of use (inclusive, local date), or "never".
 *  - UPDATES_UNTIL: the license covers versions RELEASED up to that day (the release date of each
 *    version is in Version.h), or "never". A newer version refuses it; the older ones keep working.
 *  - NOTE: free text shown in the License window; may repeat.
 * Unknown keys are accepted (and signed), so a newer generator can add fields.
 *
 * Machine UID: 32 characters of Crockford base-32 (digits and letters without I, L, O, U), shown
 * in 8 groups of 4, e.g. "7K2M-Q0PA-...". 30 characters come from SHA-256 of the product code and the
 * PC's machine secret (MachineId.h); the last 2 are a checksum, so a mistyped UID is caught before
 * a license is issued for it. Reading back is forgiving: lower case, spaces, dashes, O for 0 and
 * I/L for 1 are accepted.
 */
namespace License {
	// --- Dates: days since 1970-01-01 (proleptic Gregorian calendar) ---
	/*! \brief The day number of a calendar date. */
	int64_t daysFromCivil(int year, int month, int day);
	/*! \brief "YYYY-MM-DD" -> day number; nullopt if it is not a real date. */
	std::optional<int64_t> parseDate(std::string_view text);
	/*! \brief Day number -> "YYYY-MM-DD". */
	std::string formatDate(int64_t day);

	// --- Machine UID ---
	/*! \brief The UID (32 characters, no dashes) of a machine secret for product; empty if secret is empty. */
	std::string uidFromMachineSecret(std::string_view product, std::string_view machineSecret);
	/*! \brief Canonical form of a typed or pasted UID (see the file comment); nullopt if not a valid UID. */
	std::optional<std::string> normalizeUid(std::string_view text);
	/*! \brief "XXXX-XXXX-..." for display. */
	std::string displayUid(std::string_view uid);

	// --- Hex ---
	/*! \brief Lowercase hex of bytes. */
	std::string toHex(const uint8_t* data, size_t size);
	/*! \brief Hex (either case) -> bytes; nullopt on a bad character or odd length. */
	std::optional<std::vector<uint8_t>> fromHex(std::string_view hex);

	// --- License content ---
	/*! \brief The fields of a license (see the file comment). */
	struct Info {
		std::string product;                /*!< PRODUCT */
		std::string id;                     /*!< LICENSE_ID */
		std::string keyId;                  /*!< KEY_ID: which public key checks it */
		std::string licensee;               /*!< LICENSEE */
		std::string email;                  /*!< EMAIL (optional) */
		std::string edition;                /*!< EDITION (a label; FEATURES decides) */
		bool allFeatures = false;           /*!< FEATURES=* */
		std::vector<std::string> features;  /*!< FEATURES otherwise */
		bool anyMachine = false;            /*!< MACHINES=* */
		std::vector<std::string> machines;  /*!< MACHINES otherwise (canonical UIDs) */
		int64_t issued = 0;                 /*!< ISSUED */
		std::optional<int64_t> expires;     /*!< EXPIRES, nullopt = never */
		std::optional<int64_t> updatesUntil;/*!< UPDATES_UNTIL, nullopt = never */
		std::vector<std::string> notes;     /*!< NOTE lines */

		/*! \brief true if the license unlocks feature. */
		bool hasFeature(std::string_view feature) const;
		/*! \brief true if the license may be used on the PC with this UID. */
		bool coversMachine(std::string_view uid) const;
	};

	/*! \brief One compiled-in verification key (see LicenseKeys.h). */
	struct PublicKeyEntry {
		const char* id;  /*!< KEY_ID it answers to */
		const char* hex; /*!< 64 hex chars */
	};

	/*!
	 * \brief The full license text for info, signed with seed (info.keyId must name the matching
	 * public key). Used by the generator and the self-test; the application never signs.
	 */
	std::string sign(const Info& info, const Utils::Crypto::Ed25519::Seed& seed);

	/*! \brief Result of checking a license, from the most basic problem to the most specific. */
	enum class Status {
		Valid,             /*!< usable on this PC today */
		Missing,           /*!< no license file */
		Unreadable,        /*!< the file exists but cannot be read */
		Malformed,         /*!< not a license file, or a field is invalid (see Evaluation::detail) */
		WrongProduct,      /*!< a license for another product */
		NoKeys,            /*!< this build has no verification key (LicenseKeys.h) */
		UnknownKey,        /*!< KEY_ID is not one of the compiled-in keys */
		BadSignature,      /*!< not issued by the license owner, or edited afterwards */
		Revoked,           /*!< the license ID was revoked (LicenseKeys.h) */
		NoMachineId,       /*!< this PC's UID could not be read */
		OtherMachine,      /*!< issued for other PCs */
		ClockRolledBack,   /*!< time-limited license and the PC clock was set back */
		Expired,           /*!< past EXPIRES */
		UpdatesNotCovered  /*!< this version was released after UPDATES_UNTIL */
	};
	/*! \brief English name of a status (for the log; the GUI shows translated texts). */
	const char* statusName(Status status);

	/*! \brief The outcome of evaluate(). info is filled as far as the text could be parsed. */
	struct Evaluation {
		Status status = Status::Missing; /*!< the verdict */
		Info info;                       /*!< the license's fields */
		bool authentic = false;          /*!< the signature is valid (info can be trusted) */
		std::string detail;              /*!< which field is wrong, for Status::Malformed */
	};

	/*! \brief Everything evaluate() compares the license with. */
	struct Context {
		std::string product;                    /*!< this application's product code */
		std::vector<PublicKeyEntry> keys;       /*!< verification keys */
		std::vector<std::string> revokedIds;    /*!< revoked LICENSE_IDs */
		std::string machineUid;                 /*!< this PC (empty: unknown - only authenticity is checked if checkMachine is false) */
		bool checkMachine = true;               /*!< false: skip the PC check (the generator's "verify") */
		int64_t today = 0;                      /*!< the current local date */
		int64_t releaseDay = 0;                 /*!< release date of this version (UPDATES_UNTIL) */
		bool clockRolledBack = false;           /*!< the PC clock was found set back (LicenseManager) */
	};

	/*! \brief Parses and verifies a license text and checks it against context. */
	Evaluation evaluate(std::string_view text, const Context& context);

	// --- License request (what an unlicensed PC sends to the issuer) ---
	/*! \brief The content of a license request file. */
	struct Request {
		std::string product;        /*!< PRODUCT */
		std::string uid;            /*!< UID (canonical) */
		std::string computer;       /*!< COMPUTER: host name, to recognise the PC */
		std::string version;        /*!< VERSION of the application that wrote it */
		std::string created;        /*!< CREATED date */
		std::string currentLicense; /*!< CURRENT_LICENSE: ID of a license already on the PC (renewals), may be empty */
	};
	/*! \brief The text of a request file. */
	std::string formatRequest(const Request& request);
	/*! \brief Parses a request file (or just a UID pasted alone); nullopt without a valid UID. */
	std::optional<Request> parseRequest(std::string_view text);
}
