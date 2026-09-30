/*
 * Copyright (C) 2026 Fation Coga
 * SPDX-License-Identifier: LicenseRef-Proprietary
 * Shared license code: this file is identical in SerialPortManager and Reactor Control (see LICENSING.md).
 */

#pragma once

#include <array>
#include <cstddef>
#include <cstdint>

/*!
 * \file Ed25519.h
 * \brief Ed25519 digital signatures (RFC 8032), portable - no operating-system crypto API needed.
 *
 * Used by the license system (License.h): the issuer signs a license with the private key, the
 * application only VERIFIES with the public key compiled in, so a license cannot be forged or edited
 * without the private key. The same code runs in the application, in the license generator
 * (tools/LicenseGenerator) and in the self-tests, on Windows and elsewhere.
 *
 * The arithmetic follows TweetNaCl (Bernstein, Janssen, Lange, Schwabe - public domain), a small,
 * well-reviewed reference implementation. It is not constant-time-optimised for fast signing of
 * many messages; that does not matter here (one verification at startup, signing only in the
 * generator). Verification additionally rejects a non-canonical S (S >= L), as RFC 8032 requires.
 */
namespace Utils::Crypto::Ed25519 {
	/*! \brief A private key is its 32-byte seed; keep it secret (the license generator stores it). */
	using Seed = std::array<uint8_t, 32>;
	/*! \brief A public key: 32 bytes (compressed Edwards point). */
	using PublicKey = std::array<uint8_t, 32>;
	/*! \brief A signature: R (32 bytes) || S (32 bytes). */
	using Signature = std::array<uint8_t, 64>;

	/*! \brief The public key of a private seed. */
	PublicKey publicKeyFromSeed(const Seed& seed);
	/*! \brief Signs message with seed (deterministic: the same inputs always give the same signature). */
	Signature sign(const void* message, size_t size, const Seed& seed);
	/*! \brief true if signature was made over message by the private key of publicKey. */
	bool verify(const Signature& signature, const void* message, size_t size, const PublicKey& publicKey);
}
