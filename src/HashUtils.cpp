/*
 * Copyright (C) 2026 Fation Coga
 * SPDX-License-Identifier: LicenseRef-Proprietary
 * Shared license code: this file is identical in SerialPortManager and Reactor Control (see LICENSING.md).
 */

/*!
 * \file HashUtils.cpp
 * \brief Implementation of HashUtils.h (straightforward FIPS 180-4 SHA-256 and SHA-512).
 */

#include "HashUtils.h"

#include <bit>
#include <vector>

namespace {
	constexpr uint32_t kRound[64] = {
		0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5, 0x3956c25b, 0x59f111f1, 0x923f82a4, 0xab1c5ed5,
		0xd807aa98, 0x12835b01, 0x243185be, 0x550c7dc3, 0x72be5d74, 0x80deb1fe, 0x9bdc06a7, 0xc19bf174,
		0xe49b69c1, 0xefbe4786, 0x0fc19dc6, 0x240ca1cc, 0x2de92c6f, 0x4a7484aa, 0x5cb0a9dc, 0x76f988da,
		0x983e5152, 0xa831c66d, 0xb00327c8, 0xbf597fc7, 0xc6e00bf3, 0xd5a79147, 0x06ca6351, 0x14292967,
		0x27b70a85, 0x2e1b2138, 0x4d2c6dfc, 0x53380d13, 0x650a7354, 0x766a0abb, 0x81c2c92e, 0x92722c85,
		0xa2bfe8a1, 0xa81a664b, 0xc24b8b70, 0xc76c51a3, 0xd192e819, 0xd6990624, 0xf40e3585, 0x106aa070,
		0x19a4c116, 0x1e376c08, 0x2748774c, 0x34b0bcb5, 0x391c0cb3, 0x4ed8aa4a, 0x5b9cca4f, 0x682e6ff3,
		0x748f82ee, 0x78a5636f, 0x84c87814, 0x8cc70208, 0x90befffa, 0xa4506ceb, 0xbef9a3f7, 0xc67178f2 };

	constexpr uint64_t kRound512[80] = {
		0x428a2f98d728ae22ULL, 0x7137449123ef65cdULL, 0xb5c0fbcfec4d3b2fULL, 0xe9b5dba58189dbbcULL,
		0x3956c25bf348b538ULL, 0x59f111f1b605d019ULL, 0x923f82a4af194f9bULL, 0xab1c5ed5da6d8118ULL,
		0xd807aa98a3030242ULL, 0x12835b0145706fbeULL, 0x243185be4ee4b28cULL, 0x550c7dc3d5ffb4e2ULL,
		0x72be5d74f27b896fULL, 0x80deb1fe3b1696b1ULL, 0x9bdc06a725c71235ULL, 0xc19bf174cf692694ULL,
		0xe49b69c19ef14ad2ULL, 0xefbe4786384f25e3ULL, 0x0fc19dc68b8cd5b5ULL, 0x240ca1cc77ac9c65ULL,
		0x2de92c6f592b0275ULL, 0x4a7484aa6ea6e483ULL, 0x5cb0a9dcbd41fbd4ULL, 0x76f988da831153b5ULL,
		0x983e5152ee66dfabULL, 0xa831c66d2db43210ULL, 0xb00327c898fb213fULL, 0xbf597fc7beef0ee4ULL,
		0xc6e00bf33da88fc2ULL, 0xd5a79147930aa725ULL, 0x06ca6351e003826fULL, 0x142929670a0e6e70ULL,
		0x27b70a8546d22ffcULL, 0x2e1b21385c26c926ULL, 0x4d2c6dfc5ac42aedULL, 0x53380d139d95b3dfULL,
		0x650a73548baf63deULL, 0x766a0abb3c77b2a8ULL, 0x81c2c92e47edaee6ULL, 0x92722c851482353bULL,
		0xa2bfe8a14cf10364ULL, 0xa81a664bbc423001ULL, 0xc24b8b70d0f89791ULL, 0xc76c51a30654be30ULL,
		0xd192e819d6ef5218ULL, 0xd69906245565a910ULL, 0xf40e35855771202aULL, 0x106aa07032bbd1b8ULL,
		0x19a4c116b8d2d0c8ULL, 0x1e376c085141ab53ULL, 0x2748774cdf8eeb99ULL, 0x34b0bcb5e19b48a8ULL,
		0x391c0cb3c5c95a63ULL, 0x4ed8aa4ae3418acbULL, 0x5b9cca4f7763e373ULL, 0x682e6ff3d6b2b8a3ULL,
		0x748f82ee5defb2fcULL, 0x78a5636f43172f60ULL, 0x84c87814a1f0ab72ULL, 0x8cc702081a6439ecULL,
		0x90befffa23631e28ULL, 0xa4506cebde82bde9ULL, 0xbef9a3f7b2c67915ULL, 0xc67178f2e372532bULL,
		0xca273eceea26619cULL, 0xd186b8c721c0c207ULL, 0xeada7dd6cde0eb1eULL, 0xf57d4f7fee6ed178ULL,
		0x06f067aa72176fbaULL, 0x0a637dc5a2c898a6ULL, 0x113f9804bef90daeULL, 0x1b710b35131c471bULL,
		0x28db77f523047d84ULL, 0x32caab7b40c72493ULL, 0x3c9ebe0a15c9bebcULL, 0x431d67c49c100d4cULL,
		0x4cc5d4becb3e42b6ULL, 0x597f299cfc657e2aULL, 0x5fcb6fab3ad6faecULL, 0x6c44198c4a475817ULL };

	template <size_t N>
	std::string toHexString(const std::array<uint8_t, N>& digest) {
		static constexpr char kDigits[] = "0123456789abcdef";
		std::string out;
		out.reserve(2 * N);
		for (uint8_t b : digest) { out += kDigits[b >> 4]; out += kDigits[b & 0x0F]; }
		return out;
	}
}

namespace Utils::Hash {

std::array<uint8_t, 32> sha256(const void* data, size_t size) {
	uint32_t h[8] = { 0x6a09e667, 0xbb67ae85, 0x3c6ef372, 0xa54ff53a, 0x510e527f, 0x9b05688c, 0x1f83d9ab, 0x5be0cd19 };
	const auto* bytes = static_cast<const uint8_t*>(data);
	std::vector<uint8_t> msg(bytes, bytes + size);
	const uint64_t bitLength = static_cast<uint64_t>(size) * 8;
	msg.push_back(0x80);
	while (msg.size() % 64 != 56) msg.push_back(0);
	for (int i = 7; i >= 0; --i) msg.push_back(static_cast<uint8_t>(bitLength >> (8 * i)));

	for (size_t chunk = 0; chunk < msg.size(); chunk += 64) {
		uint32_t w[64];
		for (int i = 0; i < 16; ++i) {
			w[i] = (uint32_t(msg[chunk + 4 * i]) << 24) | (uint32_t(msg[chunk + 4 * i + 1]) << 16) |
				(uint32_t(msg[chunk + 4 * i + 2]) << 8) | uint32_t(msg[chunk + 4 * i + 3]);
		}
		for (int i = 16; i < 64; ++i) {
			const uint32_t s0 = std::rotr(w[i - 15], 7) ^ std::rotr(w[i - 15], 18) ^ (w[i - 15] >> 3);
			const uint32_t s1 = std::rotr(w[i - 2], 17) ^ std::rotr(w[i - 2], 19) ^ (w[i - 2] >> 10);
			w[i] = w[i - 16] + s0 + w[i - 7] + s1;
		}
		uint32_t a = h[0], b = h[1], c = h[2], d = h[3], e = h[4], f = h[5], g = h[6], k = h[7];
		for (int i = 0; i < 64; ++i) {
			const uint32_t S1 = std::rotr(e, 6) ^ std::rotr(e, 11) ^ std::rotr(e, 25);
			const uint32_t ch = (e & f) ^ (~e & g);
			const uint32_t t1 = k + S1 + ch + kRound[i] + w[i];
			const uint32_t S0 = std::rotr(a, 2) ^ std::rotr(a, 13) ^ std::rotr(a, 22);
			const uint32_t maj = (a & b) ^ (a & c) ^ (b & c);
			const uint32_t t2 = S0 + maj;
			k = g; g = f; f = e; e = d + t1; d = c; c = b; b = a; a = t1 + t2;
		}
		h[0] += a; h[1] += b; h[2] += c; h[3] += d; h[4] += e; h[5] += f; h[6] += g; h[7] += k;
	}
	std::array<uint8_t, 32> out{};
	for (int i = 0; i < 8; ++i) {
		out[4 * i] = static_cast<uint8_t>(h[i] >> 24);
		out[4 * i + 1] = static_cast<uint8_t>(h[i] >> 16);
		out[4 * i + 2] = static_cast<uint8_t>(h[i] >> 8);
		out[4 * i + 3] = static_cast<uint8_t>(h[i]);
	}
	return out;
}

std::string sha256Hex(std::string_view text) {
	return toHexString(sha256(text.data(), text.size()));
}

std::array<uint8_t, 64> sha512(const void* data, size_t size) {
	uint64_t h[8] = { 0x6a09e667f3bcc908ULL, 0xbb67ae8584caa73bULL, 0x3c6ef372fe94f82bULL, 0xa54ff53a5f1d36f1ULL,
		0x510e527fade682d1ULL, 0x9b05688c2b3e6c1fULL, 0x1f83d9abfb41bd6bULL, 0x5be0cd19137e2179ULL };
	const auto* bytes = static_cast<const uint8_t*>(data);
	std::vector<uint8_t> msg(bytes, bytes + size);
	const uint64_t bitLength = static_cast<uint64_t>(size) * 8; // the 128-bit length's high half is 0
	msg.push_back(0x80);
	while (msg.size() % 128 != 112) msg.push_back(0);
	for (int i = 0; i < 8; ++i) msg.push_back(0);
	for (int i = 7; i >= 0; --i) msg.push_back(static_cast<uint8_t>(bitLength >> (8 * i)));

	for (size_t chunk = 0; chunk < msg.size(); chunk += 128) {
		uint64_t w[80];
		for (int i = 0; i < 16; ++i) {
			w[i] = 0;
			for (int j = 0; j < 8; ++j) w[i] = (w[i] << 8) | msg[chunk + 8 * i + j];
		}
		for (int i = 16; i < 80; ++i) {
			const uint64_t s0 = std::rotr(w[i - 15], 1) ^ std::rotr(w[i - 15], 8) ^ (w[i - 15] >> 7);
			const uint64_t s1 = std::rotr(w[i - 2], 19) ^ std::rotr(w[i - 2], 61) ^ (w[i - 2] >> 6);
			w[i] = w[i - 16] + s0 + w[i - 7] + s1;
		}
		uint64_t a = h[0], b = h[1], c = h[2], d = h[3], e = h[4], f = h[5], g = h[6], k = h[7];
		for (int i = 0; i < 80; ++i) {
			const uint64_t S1 = std::rotr(e, 14) ^ std::rotr(e, 18) ^ std::rotr(e, 41);
			const uint64_t ch = (e & f) ^ (~e & g);
			const uint64_t t1 = k + S1 + ch + kRound512[i] + w[i];
			const uint64_t S0 = std::rotr(a, 28) ^ std::rotr(a, 34) ^ std::rotr(a, 39);
			const uint64_t maj = (a & b) ^ (a & c) ^ (b & c);
			const uint64_t t2 = S0 + maj;
			k = g; g = f; f = e; e = d + t1; d = c; c = b; b = a; a = t1 + t2;
		}
		h[0] += a; h[1] += b; h[2] += c; h[3] += d; h[4] += e; h[5] += f; h[6] += g; h[7] += k;
	}
	std::array<uint8_t, 64> out{};
	for (int i = 0; i < 8; ++i)
		for (int j = 0; j < 8; ++j) out[8 * i + j] = static_cast<uint8_t>(h[i] >> (56 - 8 * j));
	return out;
}

std::string sha512Hex(std::string_view text) {
	return toHexString(sha512(text.data(), text.size()));
}

} // namespace Utils::Hash
