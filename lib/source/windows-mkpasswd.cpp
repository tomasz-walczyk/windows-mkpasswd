/**
 * Copyright (C) 2018 Tomasz Walczyk
 *
 * This file is subject to the terms and conditions defined
 * in 'LICENSE' file which is part of this source code package.
 *
 */

#include <windows-mkpasswd/windows-mkpasswd.hpp>
#include <Windows.h>
#include <random>

///////////////////////////////////////////////////////////////////////////////

const std::size_t PASSWORD_LEN_MIN   = 1;

// WARNING: Starting from 128 characters encrypted passwords generated by this library differs from
// encrypted passwords generated by native Linux mkpasswd. I'm not sure if this is due to buffer overflow 
// in the Linux application of bug in this library. For now I will just limit password lenght to 127 characters.
const std::size_t PASSWORD_LEN_MAX   = 127;
const std::size_t SALT_LEN_MIN       = 8;
const std::size_t SALT_LEN_MAX       = 16;
const std::size_t ROUNDS_MIN         = 1000;
const std::size_t ROUNDS_MAX         = 999999999;
const std::size_t ROUNDS_DEFAULT     = 5000;

///////////////////////////////////////////////////////////////////////////////
//
// Functions to copy data between possibly-unaligned byte buffers
// and machine integers, fixing the endianness.
//
// Written by Zack Weinberg <zackw at panix.com> in 2017.
// To the extent possible under law, Zack Weinberg has waived all
// copyright and related or neighboring rights to this work.
//
// See https://creativecommons.org/publicdomain/zero/1.0/ for further details.
//
///////////////////////////////////////////////////////////////////////////////

namespace
{
  inline uint64_t be64_to_cpu(const unsigned char *buf)
  {
    return ((((uint64_t)buf[0]) << 56) |
      (((uint64_t)buf[1]) << 48) |
      (((uint64_t)buf[2]) << 40) |
      (((uint64_t)buf[3]) << 32) |
      (((uint64_t)buf[4]) << 24) |
      (((uint64_t)buf[5]) << 16) |
      (((uint64_t)buf[6]) << 8) |
      (((uint64_t)buf[7]) << 0));
  }

  inline void cpu_to_be64(unsigned char *buf, uint64_t n)
  {
    buf[0] = (unsigned char)((n & 0xFF00000000000000ull) >> 56);
    buf[1] = (unsigned char)((n & 0x00FF000000000000ull) >> 48);
    buf[2] = (unsigned char)((n & 0x0000FF0000000000ull) >> 40);
    buf[3] = (unsigned char)((n & 0x000000FF00000000ull) >> 32);
    buf[4] = (unsigned char)((n & 0x00000000FF000000ull) >> 24);
    buf[5] = (unsigned char)((n & 0x0000000000FF0000ull) >> 16);
    buf[6] = (unsigned char)((n & 0x000000000000FF00ull) >> 8);
    buf[7] = (unsigned char)((n & 0x00000000000000FFull) >> 0);
  }
} // namespace

///////////////////////////////////////////////////////////////////////////////
//
// Declaration of functions and data types used for SHA512 sum computing
// library functions.
// Copyright (C) 2007-2017 Free Software Foundation, Inc.
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation; either version 2.1 of
// the License, or (at your option) any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along with this library; if not, see
// <https://www.gnu.org/licenses/>.
//
///////////////////////////////////////////////////////////////////////////////

namespace
{
  /* Structure to save state of computation between the single steps.  */
  struct sha512_ctx
  {
    uint64_t H[8];
    uint64_t total[2];
    uint32_t buflen;
    unsigned char buffer[256];
  };

  /* Constants for SHA512 from FIPS 180-2:4.2.3.  */
  static const uint64_t K[80] =
  {
    UINT64_C(0x428a2f98d728ae22), UINT64_C(0x7137449123ef65cd),
    UINT64_C(0xb5c0fbcfec4d3b2f), UINT64_C(0xe9b5dba58189dbbc),
    UINT64_C(0x3956c25bf348b538), UINT64_C(0x59f111f1b605d019),
    UINT64_C(0x923f82a4af194f9b), UINT64_C(0xab1c5ed5da6d8118),
    UINT64_C(0xd807aa98a3030242), UINT64_C(0x12835b0145706fbe),
    UINT64_C(0x243185be4ee4b28c), UINT64_C(0x550c7dc3d5ffb4e2),
    UINT64_C(0x72be5d74f27b896f), UINT64_C(0x80deb1fe3b1696b1),
    UINT64_C(0x9bdc06a725c71235), UINT64_C(0xc19bf174cf692694),
    UINT64_C(0xe49b69c19ef14ad2), UINT64_C(0xefbe4786384f25e3),
    UINT64_C(0x0fc19dc68b8cd5b5), UINT64_C(0x240ca1cc77ac9c65),
    UINT64_C(0x2de92c6f592b0275), UINT64_C(0x4a7484aa6ea6e483),
    UINT64_C(0x5cb0a9dcbd41fbd4), UINT64_C(0x76f988da831153b5),
    UINT64_C(0x983e5152ee66dfab), UINT64_C(0xa831c66d2db43210),
    UINT64_C(0xb00327c898fb213f), UINT64_C(0xbf597fc7beef0ee4),
    UINT64_C(0xc6e00bf33da88fc2), UINT64_C(0xd5a79147930aa725),
    UINT64_C(0x06ca6351e003826f), UINT64_C(0x142929670a0e6e70),
    UINT64_C(0x27b70a8546d22ffc), UINT64_C(0x2e1b21385c26c926),
    UINT64_C(0x4d2c6dfc5ac42aed), UINT64_C(0x53380d139d95b3df),
    UINT64_C(0x650a73548baf63de), UINT64_C(0x766a0abb3c77b2a8),
    UINT64_C(0x81c2c92e47edaee6), UINT64_C(0x92722c851482353b),
    UINT64_C(0xa2bfe8a14cf10364), UINT64_C(0xa81a664bbc423001),
    UINT64_C(0xc24b8b70d0f89791), UINT64_C(0xc76c51a30654be30),
    UINT64_C(0xd192e819d6ef5218), UINT64_C(0xd69906245565a910),
    UINT64_C(0xf40e35855771202a), UINT64_C(0x106aa07032bbd1b8),
    UINT64_C(0x19a4c116b8d2d0c8), UINT64_C(0x1e376c085141ab53),
    UINT64_C(0x2748774cdf8eeb99), UINT64_C(0x34b0bcb5e19b48a8),
    UINT64_C(0x391c0cb3c5c95a63), UINT64_C(0x4ed8aa4ae3418acb),
    UINT64_C(0x5b9cca4f7763e373), UINT64_C(0x682e6ff3d6b2b8a3),
    UINT64_C(0x748f82ee5defb2fc), UINT64_C(0x78a5636f43172f60),
    UINT64_C(0x84c87814a1f0ab72), UINT64_C(0x8cc702081a6439ec),
    UINT64_C(0x90befffa23631e28), UINT64_C(0xa4506cebde82bde9),
    UINT64_C(0xbef9a3f7b2c67915), UINT64_C(0xc67178f2e372532b),
    UINT64_C(0xca273eceea26619c), UINT64_C(0xd186b8c721c0c207),
    UINT64_C(0xeada7dd6cde0eb1e), UINT64_C(0xf57d4f7fee6ed178),
    UINT64_C(0x06f067aa72176fba), UINT64_C(0x0a637dc5a2c898a6),
    UINT64_C(0x113f9804bef90dae), UINT64_C(0x1b710b35131c471b),
    UINT64_C(0x28db77f523047d84), UINT64_C(0x32caab7b40c72493),
    UINT64_C(0x3c9ebe0a15c9bebc), UINT64_C(0x431d67c49c100d4c),
    UINT64_C(0x4cc5d4becb3e42b6), UINT64_C(0x597f299cfc657e2a),
    UINT64_C(0x5fcb6fab3ad6faec), UINT64_C(0x6c44198c4a475817)
  };

  /* Process LEN bytes of BUFFER, accumulating context into CTX.
  It is assumed that LEN % 128 == 0.  */
  void sha512_process_block(const void *buffer, size_t len, sha512_ctx *ctx)
  {
    unsigned int t;
    const unsigned char *p = (const unsigned char *)buffer;
    size_t nwords = len / sizeof(uint64_t);
    uint64_t a = ctx->H[0];
    uint64_t b = ctx->H[1];
    uint64_t c = ctx->H[2];
    uint64_t d = ctx->H[3];
    uint64_t e = ctx->H[4];
    uint64_t f = ctx->H[5];
    uint64_t g = ctx->H[6];
    uint64_t h = ctx->H[7];

    /* First increment the byte count.  FIPS 180-2 specifies the possible
    length of the file up to 2^128 bits.  Here we only compute the
    number of bytes.  Do a double word increment.  */
    ctx->total[0] += len;
    if (ctx->total[0] < len)
      ++ctx->total[1];

    /* Process all bytes in the buffer with 128 bytes in each round of
    the loop.  */
    while (nwords > 0) {
      uint64_t W[80];
      uint64_t a_save = a;
      uint64_t b_save = b;
      uint64_t c_save = c;
      uint64_t d_save = d;
      uint64_t e_save = e;
      uint64_t f_save = f;
      uint64_t g_save = g;
      uint64_t h_save = h;

/* Operators defined in FIPS 180-2:4.1.2.  */
#define Ch(x, y, z) ((x & y) ^ (~x & z))
#define Maj(x, y, z) ((x & y) ^ (x & z) ^ (y & z))
#define S0(x) (CYCLIC (x, 28) ^ CYCLIC (x, 34) ^ CYCLIC (x, 39))
#define S1(x) (CYCLIC (x, 14) ^ CYCLIC (x, 18) ^ CYCLIC (x, 41))
#define R0(x) (CYCLIC (x, 1) ^ CYCLIC (x, 8) ^ (x >> 7))
#define R1(x) (CYCLIC (x, 19) ^ CYCLIC (x, 61) ^ (x >> 6))
#define CYCLIC(w, s) ((w >> s) | (w << (64 - s)))

      /* Compute the message schedule according to FIPS 180-2:6.3.2 step 2.  */
      for (t = 0; t < 16; ++t) {
        W[t] = be64_to_cpu(p);
        p += 8;
      }
      for (t = 16; t < 80; ++t)
        W[t] = R1(W[t - 2]) + W[t - 7] + R0(W[t - 15]) + W[t - 16];

      /* The actual computation according to FIPS 180-2:6.3.2 step 3.  */
      for (t = 0; t < 80; ++t) {
        uint64_t T1 = h + S1(e) + Ch(e, f, g) + K[t] + W[t];
        uint64_t T2 = S0(a) + Maj(a, b, c);
        h = g;
        g = f;
        f = e;
        e = d + T1;
        d = c;
        c = b;
        b = a;
        a = T1 + T2;
      }

      /* Add the starting values of the context according to FIPS 180-2:6.3.2
      step 4.  */
      a += a_save;
      b += b_save;
      c += c_save;
      d += d_save;
      e += e_save;
      f += f_save;
      g += g_save;
      h += h_save;

      /* Prepare for the next round.  */
      nwords -= 16;
    }

    /* Put checksum in context given as argument.  */
    ctx->H[0] = a;
    ctx->H[1] = b;
    ctx->H[2] = c;
    ctx->H[3] = d;
    ctx->H[4] = e;
    ctx->H[5] = f;
    ctx->H[6] = g;
    ctx->H[7] = h;
  }

  /* Initialize structure containing state of computation.
  (FIPS 180-2:5.3.3)  */
  void sha512_init_ctx(sha512_ctx *ctx)
  {
    ctx->H[0] = UINT64_C(0x6a09e667f3bcc908);
    ctx->H[1] = UINT64_C(0xbb67ae8584caa73b);
    ctx->H[2] = UINT64_C(0x3c6ef372fe94f82b);
    ctx->H[3] = UINT64_C(0xa54ff53a5f1d36f1);
    ctx->H[4] = UINT64_C(0x510e527fade682d1);
    ctx->H[5] = UINT64_C(0x9b05688c2b3e6c1f);
    ctx->H[6] = UINT64_C(0x1f83d9abfb41bd6b);
    ctx->H[7] = UINT64_C(0x5be0cd19137e2179);

    ctx->total[0] = ctx->total[1] = 0;
    ctx->buflen = 0;
  }

  /* Process the remaining bytes in the internal buffer and the usual
  prolog according to the standard and write the result to RESBUF.

  IMPORTANT: On some systems it is required that RESBUF is correctly
  aligned for a 32 bits value.  */
  void *sha512_finish_ctx(sha512_ctx *ctx, void *resbuf)
  {
    /* Take yet unprocessed bytes into account.  */
    uint32_t bytes = ctx->buflen;
    size_t pad;
    unsigned int i;
    unsigned char *rp = (unsigned char *)resbuf;

    /* Now count remaining bytes.  */
    ctx->total[0] += bytes;
    if (ctx->total[0] < bytes)
      ++ctx->total[1];

    pad = bytes >= 112 ? 128 + 112 - bytes : 112 - bytes;
    /* The first byte of padding should be 0x80 and the rest should be
    zero.  (FIPS 180-2:5.1.2) */
    ctx->buffer[bytes] = 0x80u;
    SecureZeroMemory(&ctx->buffer[bytes + 1], pad - 1);

    /* Put the 128-bit file length in big-endian *bits* at the end of
    the buffer.  */
    cpu_to_be64(&ctx->buffer[bytes + pad],
      (ctx->total[1] << 3) | (ctx->total[0] >> 61));
    cpu_to_be64(&ctx->buffer[bytes + pad + 8],
      ctx->total[0] << 3);

    /* Process last bytes.  */
    sha512_process_block(ctx->buffer, bytes + pad + 16, ctx);

    /* Put result from CTX in first 64 bytes following RESBUF.  */
    for (i = 0; i < 8; ++i)
      cpu_to_be64(rp + i * 8, ctx->H[i]);

    SecureZeroMemory(ctx, sizeof(sha512_ctx));
    return resbuf;
  }

  void sha512_process_bytes(const void *buffer, size_t len, sha512_ctx *ctx)
  {
    /* When we already have some bits in our internal buffer concatenate
    both inputs first.  */
    if (ctx->buflen != 0) {
      uint32_t left_over = ctx->buflen;
      uint32_t add = 256 - left_over > len ? (uint32_t)len : 256 - left_over;

      memcpy(&ctx->buffer[left_over], buffer, add);
      ctx->buflen += add;

      if (ctx->buflen > 128) {
        sha512_process_block(ctx->buffer, ctx->buflen & ~127u, ctx);

        ctx->buflen &= 127;
        /* The regions in the following copy operation cannot overlap.  */
        memcpy(ctx->buffer, &ctx->buffer[(left_over + add) & ~127u],
          ctx->buflen);
      }

      buffer = (const char *)buffer + add;
      len -= add;
    }

    /* Process available complete blocks.  */
    if (len > 128) {
      sha512_process_block(buffer, len & ~127u, ctx);
      buffer = (const char *)buffer + (len & ~127u);
      len &= 127;
    }

    /* Move remaining bytes into internal buffer.  */
    if (len > 0) {
      size_t left_over = ctx->buflen;

      memcpy(&ctx->buffer[left_over], buffer, len);
      left_over += len;
      if (left_over >= 128) {
        sha512_process_block(ctx->buffer, 128, ctx);
        left_over -= 128;
        memcpy(ctx->buffer, &ctx->buffer[128], left_over);
      }
      ctx->buflen = (uint32_t)left_over;
    }
  }
} // namespace

///////////////////////////////////////////////////////////////////////////////
//
// One way encryption based on the SHA512-based Unix crypt implementation.
//
// Written by Ulrich Drepper <drepper at redhat.com> in 2007 [1].
// Modified by Zack Weinberg <zackw at panix.com> in 2017, 2018.
// Composed by Björn Esser <besser82 at fedoraproject.org> in 2018.
// To the extent possible under law, the named authors have waived all
// copyright and related or neighboring rights to this work.
//
// See https://creativecommons.org/publicdomain/zero/1.0/ for further details.
//
///////////////////////////////////////////////////////////////////////////////

namespace
{
  struct sha512_buffer
  {
    sha512_ctx ctx;
    uint8_t result[64];
    uint8_t p_bytes[64];
    uint8_t s_bytes[64];
  };

  void sha512_process_recycled_bytes(unsigned char block[64], size_t len, sha512_ctx *ctx)
  {
    size_t cnt;
    for (cnt = len; cnt >= 64; cnt -= 64)
      sha512_process_bytes(block, 64, ctx);
    sha512_process_bytes(block, cnt, ctx);
  }

  std::string crypt_sha512_rn(const std::string& password, std::size_t rounds, const std::string& salt)
  {
    if (password.size() < PASSWORD_LEN_MIN
      || password.size() > PASSWORD_LEN_MAX
      || salt.size() < SALT_LEN_MIN
      || salt.size() > SALT_LEN_MAX
      || rounds < ROUNDS_MIN
      || rounds > ROUNDS_MAX) {
      return "";
    }

    sha512_buffer buffer = {};
    sha512_ctx *ctx = &buffer.ctx;
    uint8_t *result = buffer.result;
    uint8_t *p_bytes = buffer.p_bytes;
    uint8_t *s_bytes = buffer.s_bytes;

    size_t cnt = 0;

    /* Compute alternate SHA512 sum with input PHRASE, SALT, and PHRASE.
    The final result will be added to the first context.  */
    sha512_init_ctx(ctx);

    /* Add phrase.  */
    sha512_process_bytes(password.data(), password.size(), ctx);

    /* Add salt.  */
    sha512_process_bytes(salt.data(), salt.size(), ctx);

    /* Add phrase again.  */
    sha512_process_bytes(password.data(), password.size(), ctx);

    /* Now get result of this (64 bytes) and add it to the other context.  */
    sha512_finish_ctx(ctx, result);

    /* Prepare for the real work.  */
    sha512_init_ctx(ctx);

    /* Add the phrase string.  */
    sha512_process_bytes(password.data(), password.size(), ctx);

    /* The last part is the salt string.  This must be at most 8
    characters and it ends at the first `$' character (for
    compatibility with existing implementations).  */
    sha512_process_bytes(salt.data(), salt.size(), ctx);

    /* Add for any character in the phrase one byte of the alternate sum.  */
    for (cnt = password.size(); cnt > 64; cnt -= 64) {
      sha512_process_bytes(result, 64, ctx);
    }
    sha512_process_bytes(result, cnt, ctx);

    /* Take the binary representation of the length of the phrase and for every
    1 add the alternate sum, for every 0 the phrase.  */
    for (cnt = password.size(); cnt > 0; cnt >>= 1) {
      if ((cnt & 1) != 0) {
        sha512_process_bytes(result, 64, ctx);
      } else {
        sha512_process_bytes(password.data(), password.size(), ctx);
      }
    }

    /* Create intermediate result.  */
    sha512_finish_ctx(ctx, result);

    /* Start computation of P byte sequence.  */
    sha512_init_ctx(ctx);

    /* For every character in the password add the entire password.  */
    for (cnt = 0; cnt < password.size(); ++cnt) {
      sha512_process_bytes(password.data(), password.size(), ctx);
    }

    /* Finish the digest.  */
    sha512_finish_ctx(ctx, p_bytes);

    /* Start computation of S byte sequence.  */
    sha512_init_ctx(ctx);

    /* For every character in the password add the entire password.  */
    for (cnt = 0; cnt < (size_t)16 + (size_t)result[0]; ++cnt) {
      sha512_process_bytes(salt.data(), salt.size(), ctx);
    }

    /* Finish the digest.  */
    sha512_finish_ctx(ctx, s_bytes);

    /* Repeatedly run the collected hash value through SHA512 to burn CPU cycles.  */
    for (cnt = 0; cnt < rounds; ++cnt) {
      /* New context.  */
      sha512_init_ctx(ctx);

      /* Add phrase or last result.  */
      if ((cnt & 1) != 0) {
        sha512_process_recycled_bytes(p_bytes, password.size(), ctx);
      } else {
        sha512_process_bytes(result, 64, ctx);
      }

      /* Add salt for numbers not divisible by 3.  */
      if (cnt % 3 != 0) {
        sha512_process_recycled_bytes(s_bytes, salt.size(), ctx);
      }

      /* Add phrase for numbers not divisible by 7.  */
      if (cnt % 7 != 0) {
        sha512_process_recycled_bytes(p_bytes, password.size(), ctx);
      }

      /* Add phrase or last result.  */
      if ((cnt & 1) != 0) {
        sha512_process_bytes(result, 64, ctx);
      } else {
        sha512_process_recycled_bytes(p_bytes, password.size(), ctx);
      }

      /* Create intermediate result.  */
      sha512_finish_ctx(ctx, result);
    }

    /* Now we can construct the result string. */
    const size_t SHA512_HASH_STR_SIZE = 86;
    const char SHA512_HASH_ID[]       = "$6";
    const char SHA512_HASH_PREFIX[]   = "$";
    const char SHA512_SALT_PREFIX[]   = "$";
    const char SHA512_ROUNDS_PREFIX[] = "$rounds=";
    const char b64t[] = "./0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";

    std::string hash = SHA512_HASH_ID;
    hash += SHA512_ROUNDS_PREFIX;
    hash += std::to_string(rounds);
    hash += SHA512_SALT_PREFIX;
    hash += salt;
    hash += SHA512_HASH_PREFIX;
    hash += std::string(SHA512_HASH_STR_SIZE, '\0');
    char *cp = &hash[hash.size() - SHA512_HASH_STR_SIZE];

#define b64_from_24bit(B2, B1, B0, N)                   \
  do {                                                  \
    unsigned int w = ((((unsigned int)(B2)) << 16) |    \
                      (((unsigned int)(B1)) << 8) |     \
                      ((unsigned int)(B0)));            \
    int n = (N);                                        \
    while (n-- > 0)                                     \
      {                                                 \
        *cp++ = b64t[w & 0x3f];                         \
        w >>= 6;                                        \
      }                                                 \
  } while (0)

    b64_from_24bit(result[0], result[21], result[42], 4);
    b64_from_24bit(result[22], result[43], result[1], 4);
    b64_from_24bit(result[44], result[2], result[23], 4);
    b64_from_24bit(result[3], result[24], result[45], 4);
    b64_from_24bit(result[25], result[46], result[4], 4);
    b64_from_24bit(result[47], result[5], result[26], 4);
    b64_from_24bit(result[6], result[27], result[48], 4);
    b64_from_24bit(result[28], result[49], result[7], 4);
    b64_from_24bit(result[50], result[8], result[29], 4);
    b64_from_24bit(result[9], result[30], result[51], 4);
    b64_from_24bit(result[31], result[52], result[10], 4);
    b64_from_24bit(result[53], result[11], result[32], 4);
    b64_from_24bit(result[12], result[33], result[54], 4);
    b64_from_24bit(result[34], result[55], result[13], 4);
    b64_from_24bit(result[56], result[14], result[35], 4);
    b64_from_24bit(result[15], result[36], result[57], 4);
    b64_from_24bit(result[37], result[58], result[16], 4);
    b64_from_24bit(result[59], result[17], result[38], 4);
    b64_from_24bit(result[18], result[39], result[60], 4);
    b64_from_24bit(result[40], result[61], result[19], 4);
    b64_from_24bit(result[62], result[20], result[41], 4);
    b64_from_24bit(0, 0, result[63], 2);

    SecureZeroMemory(&buffer, sizeof(sha512_buffer));
    return hash;
  }
} // namespace

///////////////////////////////////////////////////////////////////////////////

namespace
{
  std::string rand_salt()
  {
    static const char charset[] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";

    static std::random_device device{};
    static std::mt19937 engine{ device() };

    const std::uniform_int_distribution<std::size_t> size_dist(SALT_LEN_MIN, SALT_LEN_MAX);
    const std::uniform_int_distribution<std::size_t> char_dist(0, (sizeof(charset) - 2));

    std::string salt(size_dist(engine), '\0');
    for (auto i = 0; i < salt.size(); ++i) {
      salt[i] = charset[char_dist(engine)];
    }
    return salt;
  }
} // namespace

///////////////////////////////////////////////////////////////////////////////

std::string sha512_crypt(const std::string& password)
{
  return crypt_sha512_rn(password, ROUNDS_DEFAULT, rand_salt());
}

///////////////////////////////////////////////////////////////////////////////

std::string sha512_crypt(const std::string& password, std::size_t rounds)
{
  return crypt_sha512_rn(password, rounds, rand_salt());
}

///////////////////////////////////////////////////////////////////////////////

std::string sha512_crypt(const std::string& password, std::size_t rounds, const std::string& salt)
{
  return crypt_sha512_rn(password, rounds, salt);
}

///////////////////////////////////////////////////////////////////////////////

void secure_clear_string(std::string& string)
{
  if (!string.empty()) {
    SecureZeroMemory(&string[0], sizeof(char) * string.size());
    string.clear();
  }
}
