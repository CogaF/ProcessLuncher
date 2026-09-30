/*
 * Copyright (C) 2026 Fation Coga
 * SPDX-License-Identifier: LicenseRef-Proprietary
 * Shared license code: this file is identical in SerialPortManager and Reactor Control (see LICENSING.md).
 */

/*!
 * \file Ed25519.cpp
 * \brief Implementation of Ed25519.h - the field and group arithmetic of TweetNaCl (public domain),
 * rewritten in C++ with the original structure kept so it can be compared line by line.
 *
 * Field elements (integers mod p = 2^255 - 19) are 16 limbs of 16 bits held in int64_t; points are
 * in extended coordinates (X, Y, Z, T). Checked against the RFC 8032 test vectors and an independent
 * implementation in tests/LicenseSelfTest.cpp.
 */

#include "Ed25519.h"
#include "HashUtils.h"

#include <cstring>
#include <vector>

namespace {
	using Gf = int64_t[16];

	const Gf kGf0 = {};
	const Gf kGf1 = { 1 };
	// d = -121665/121666, 2d, the base point (X, Y) and sqrt(-1) - checked by the self-test.
	const Gf kD = { 0x78a3, 0x1359, 0x4dca, 0x75eb, 0xd8ab, 0x4141, 0x0a4d, 0x0070, 0xe898, 0x7779, 0x4079, 0x8cc7, 0xfe73, 0x2b6f, 0x6cee, 0x5203 };
	const Gf kD2 = { 0xf159, 0x26b2, 0x9b94, 0xebd6, 0xb156, 0x8283, 0x149a, 0x00e0, 0xd130, 0xeef3, 0x80f2, 0x198e, 0xfce7, 0x56df, 0xd9dc, 0x2406 };
	const Gf kX = { 0xd51a, 0x8f25, 0x2d60, 0xc956, 0xa7b2, 0x9525, 0xc760, 0x692c, 0xdc5c, 0xfdd6, 0xe231, 0xc0a4, 0x53fe, 0xcd6e, 0x36d3, 0x2169 };
	const Gf kY = { 0x6658, 0x6666, 0x6666, 0x6666, 0x6666, 0x6666, 0x6666, 0x6666, 0x6666, 0x6666, 0x6666, 0x6666, 0x6666, 0x6666, 0x6666, 0x6666 };
	const Gf kI = { 0xa0b0, 0x4a0e, 0x1b27, 0xc4ee, 0xe478, 0xad2f, 0x1806, 0x2f43, 0xd7a7, 0x3dfb, 0x0099, 0x2b4d, 0xdf0b, 0x4fc1, 0x2480, 0x2b83 };
	// The group order L = 2^252 + 27742317777372353535851937790883648493, little-endian bytes.
	const int64_t kL[32] = { 0xed, 0xd3, 0xf5, 0x5c, 0x1a, 0x63, 0x12, 0x58, 0xd6, 0x9c, 0xf7, 0xa2, 0xde, 0xf9, 0xde, 0x14,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0x10 };

	void set(Gf r, const Gf a) { for (int i = 0; i < 16; ++i) r[i] = a[i]; }

	void carry(Gf o) {
		for (int i = 0; i < 16; ++i) {
			o[i] += (int64_t(1) << 16);
			const int64_t c = o[i] >> 16;
			o[(i + 1) * (i < 15)] += c - 1 + 37 * (c - 1) * (i == 15);
			o[i] -= c * 65536;
		}
	}

	// Swaps p and q when b is 1, without branching on b.
	void select(Gf p, Gf q, int b) {
		const int64_t c = ~(int64_t(b) - 1);
		for (int i = 0; i < 16; ++i) {
			const int64_t t = c & (p[i] ^ q[i]);
			p[i] ^= t;
			q[i] ^= t;
		}
	}

	void pack(uint8_t* o, const Gf n) {
		Gf m, t;
		set(t, n);
		carry(t); carry(t); carry(t);
		for (int j = 0; j < 2; ++j) {
			m[0] = t[0] - 0xffed;
			for (int i = 1; i < 15; ++i) {
				m[i] = t[i] - 0xffff - ((m[i - 1] >> 16) & 1);
				m[i - 1] &= 0xffff;
			}
			m[15] = t[15] - 0x7fff - ((m[14] >> 16) & 1);
			const int b = static_cast<int>((m[15] >> 16) & 1);
			m[14] &= 0xffff;
			select(t, m, 1 - b);
		}
		for (int i = 0; i < 16; ++i) {
			o[2 * i] = static_cast<uint8_t>(t[i] & 0xff);
			o[2 * i + 1] = static_cast<uint8_t>(t[i] >> 8);
		}
	}

	bool equalBytes32(const uint8_t* a, const uint8_t* b) {
		unsigned diff = 0;
		for (int i = 0; i < 32; ++i) diff |= static_cast<unsigned>(a[i] ^ b[i]);
		return diff == 0;
	}

	bool notEqual(const Gf a, const Gf b) {
		uint8_t c[32], d[32];
		pack(c, a);
		pack(d, b);
		return !equalBytes32(c, d);
	}

	uint8_t parity(const Gf a) {
		uint8_t d[32];
		pack(d, a);
		return d[0] & 1;
	}

	void unpack(Gf o, const uint8_t* n) {
		for (int i = 0; i < 16; ++i) o[i] = n[2 * i] + (int64_t(n[2 * i + 1]) << 8);
		o[15] &= 0x7fff;
	}

	void add(Gf o, const Gf a, const Gf b) { for (int i = 0; i < 16; ++i) o[i] = a[i] + b[i]; }
	void sub(Gf o, const Gf a, const Gf b) { for (int i = 0; i < 16; ++i) o[i] = a[i] - b[i]; }

	void mul(Gf o, const Gf a, const Gf b) {
		int64_t t[31] = {};
		for (int i = 0; i < 16; ++i)
			for (int j = 0; j < 16; ++j) t[i + j] += a[i] * b[j];
		for (int i = 0; i < 15; ++i) t[i] += 38 * t[i + 16];
		for (int i = 0; i < 16; ++i) o[i] = t[i];
		carry(o);
		carry(o);
	}

	void square(Gf o, const Gf a) { mul(o, a, a); }

	void invert(Gf o, const Gf in) {
		Gf c;
		set(c, in);
		for (int a = 253; a >= 0; --a) {
			square(c, c);
			if (a != 2 && a != 4) mul(c, c, in);
		}
		set(o, c);
	}

	// in^((p-5)/8), used to take square roots.
	void pow2523(Gf o, const Gf in) {
		Gf c;
		set(c, in);
		for (int a = 250; a >= 0; --a) {
			square(c, c);
			if (a != 1) mul(c, c, in);
		}
		set(o, c);
	}

	struct Point { Gf v[4]; }; // X, Y, Z, T

	void pointAdd(Point& p, const Point& q) {
		Gf a, b, c, d, t, e, f, g, h;
		sub(a, p.v[1], p.v[0]);
		sub(t, q.v[1], q.v[0]);
		mul(a, a, t);
		add(b, p.v[0], p.v[1]);
		add(t, q.v[0], q.v[1]);
		mul(b, b, t);
		mul(c, p.v[3], q.v[3]);
		mul(c, c, kD2);
		mul(d, p.v[2], q.v[2]);
		add(d, d, d);
		sub(e, b, a);
		sub(f, d, c);
		add(g, d, c);
		add(h, b, a);
		mul(p.v[0], e, f);
		mul(p.v[1], h, g);
		mul(p.v[2], g, f);
		mul(p.v[3], e, h);
	}

	void pointSwap(Point& p, Point& q, int b) {
		for (int i = 0; i < 4; ++i) select(p.v[i], q.v[i], b);
	}

	void pointPack(uint8_t* r, const Point& p) {
		Gf tx, ty, zi;
		invert(zi, p.v[2]);
		mul(tx, p.v[0], zi);
		mul(ty, p.v[1], zi);
		pack(r, ty);
		r[31] ^= static_cast<uint8_t>(parity(tx) << 7);
	}

	// p = s * q (s: 32-byte little-endian scalar); q is destroyed.
	void scalarMult(Point& p, Point& q, const uint8_t* s) {
		set(p.v[0], kGf0);
		set(p.v[1], kGf1);
		set(p.v[2], kGf1);
		set(p.v[3], kGf0);
		for (int i = 255; i >= 0; --i) {
			const int b = (s[i / 8] >> (i & 7)) & 1;
			pointSwap(p, q, b);
			pointAdd(q, p);
			pointAdd(p, p);
			pointSwap(p, q, b);
		}
	}

	void scalarBase(Point& p, const uint8_t* s) {
		Point q;
		set(q.v[0], kX);
		set(q.v[1], kY);
		set(q.v[2], kGf1);
		mul(q.v[3], kX, kY);
		scalarMult(p, q, s);
	}

	// r = x mod L (x: 64 limbs, destroyed).
	void modL(uint8_t* r, int64_t x[64]) {
		int64_t carryValue;
		for (int i = 63; i >= 32; --i) {
			carryValue = 0;
			int j;
			for (j = i - 32; j < i - 12; ++j) {
				x[j] += carryValue - 16 * x[i] * kL[j - (i - 32)];
				carryValue = (x[j] + 128) >> 8;
				x[j] -= carryValue * 256;
			}
			x[j] += carryValue;
			x[i] = 0;
		}
		carryValue = 0;
		for (int j = 0; j < 32; ++j) {
			x[j] += carryValue - (x[31] >> 4) * kL[j];
			carryValue = x[j] >> 8;
			x[j] &= 255;
		}
		for (int j = 0; j < 32; ++j) x[j] -= carryValue * kL[j];
		for (int i = 0; i < 32; ++i) {
			x[i + 1] += x[i] >> 8;
			r[i] = static_cast<uint8_t>(x[i] & 255);
		}
	}

	// r (64 bytes in, 32 bytes out) = r mod L.
	void reduce(uint8_t* r) {
		int64_t x[64];
		for (int i = 0; i < 64; ++i) x[i] = r[i];
		for (int i = 0; i < 64; ++i) r[i] = 0;
		modL(r, x);
	}

	// Decodes a public key into -A (negated, as verification needs); false if it is not a curve point.
	bool unpackNegated(Point& r, const uint8_t* p) {
		Gf t, chk, num, den, den2, den4, den6;
		set(r.v[2], kGf1);
		unpack(r.v[1], p);
		square(num, r.v[1]);
		mul(den, num, kD);
		sub(num, num, r.v[2]);
		add(den, r.v[2], den);

		square(den2, den);
		square(den4, den2);
		mul(den6, den4, den2);
		mul(t, den6, num);
		mul(t, t, den);

		pow2523(t, t);
		mul(t, t, num);
		mul(t, t, den);
		mul(t, t, den);
		mul(r.v[0], t, den);

		square(chk, r.v[0]);
		mul(chk, chk, den);
		if (notEqual(chk, num)) mul(r.v[0], r.v[0], kI);

		square(chk, r.v[0]);
		mul(chk, chk, den);
		if (notEqual(chk, num)) return false;

		if (parity(r.v[0]) == (p[31] >> 7)) sub(r.v[0], kGf0, r.v[0]);
		mul(r.v[3], r.v[0], r.v[1]);
		return true;
	}

	// true if the 32-byte little-endian scalar s is below L (a canonical signature S).
	bool scalarIsCanonical(const uint8_t* s) {
		for (int i = 31; i >= 0; --i) {
			if (s[i] < kL[i]) return true;
			if (s[i] > kL[i]) return false;
		}
		return false; // equal to L
	}

	std::array<uint8_t, 64> hashOf(const std::vector<uint8_t>& data) {
		return Utils::Hash::sha512(data.data(), data.size());
	}
}

namespace Utils::Crypto::Ed25519 {

PublicKey publicKeyFromSeed(const Seed& seed) {
	std::array<uint8_t, 64> d = Utils::Hash::sha512(seed.data(), seed.size());
	d[0] &= 248;
	d[31] &= 127;
	d[31] |= 64;
	Point p;
	scalarBase(p, d.data());
	PublicKey pk{};
	pointPack(pk.data(), p);
	return pk;
}

Signature sign(const void* message, size_t size, const Seed& seed) {
	const PublicKey pk = publicKeyFromSeed(seed);
	std::array<uint8_t, 64> d = Utils::Hash::sha512(seed.data(), seed.size());
	d[0] &= 248;
	d[31] &= 127;
	d[31] |= 64;
	const auto* msg = static_cast<const uint8_t*>(message);

	// r = H(prefix || M) mod L; R = r*B
	std::vector<uint8_t> buffer(d.begin() + 32, d.end());
	buffer.insert(buffer.end(), msg, msg + size);
	std::array<uint8_t, 64> r = hashOf(buffer);
	reduce(r.data());
	Point p;
	scalarBase(p, r.data());
	Signature sig{};
	pointPack(sig.data(), p);

	// h = H(R || A || M) mod L; S = r + h*a mod L
	buffer.assign(sig.begin(), sig.begin() + 32);
	buffer.insert(buffer.end(), pk.begin(), pk.end());
	buffer.insert(buffer.end(), msg, msg + size);
	std::array<uint8_t, 64> h = hashOf(buffer);
	reduce(h.data());
	int64_t x[64] = {};
	for (int i = 0; i < 32; ++i) x[i] = r[i];
	for (int i = 0; i < 32; ++i)
		for (int j = 0; j < 32; ++j) x[i + j] += int64_t(h[i]) * int64_t(d[j]);
	modL(sig.data() + 32, x);
	return sig;
}

bool verify(const Signature& signature, const void* message, size_t size, const PublicKey& publicKey) {
	if (!scalarIsCanonical(signature.data() + 32)) return false;
	Point q;
	if (!unpackNegated(q, publicKey.data())) return false;

	// Checks R == S*B - h*A, i.e. packs S*B + h*(-A) and compares with R.
	const auto* msg = static_cast<const uint8_t*>(message);
	std::vector<uint8_t> buffer(signature.begin(), signature.begin() + 32);
	buffer.insert(buffer.end(), publicKey.begin(), publicKey.end());
	buffer.insert(buffer.end(), msg, msg + size);
	std::array<uint8_t, 64> h = hashOf(buffer);
	reduce(h.data());

	Point p, sb;
	scalarMult(p, q, h.data());
	scalarBase(sb, signature.data() + 32);
	pointAdd(p, sb);
	uint8_t t[32];
	pointPack(t, p);
	return equalBytes32(signature.data(), t);
}

} // namespace Utils::Crypto::Ed25519
