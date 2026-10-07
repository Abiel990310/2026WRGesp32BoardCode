#pragma once
#include <cctype>
#include <cstring>

inline bool fleetNormalizeMac(const char* input, char* output) {
  if (!input || strlen(input) != 17) return false;
  unsigned bytes[6] = {};
  for (int i = 0; i < 17; ++i) {
    if (i % 3 == 2) { if (input[i] != ':') return false; output[i] = ':'; }
    else {
      const unsigned char c = static_cast<unsigned char>(input[i]);
      if (!isxdigit(c)) return false;
      output[i] = static_cast<char>(tolower(c));
      bytes[i / 3] = bytes[i / 3] * 16 + (isdigit(c) ? c - '0' : tolower(c) - 'a' + 10);
    }
  }
  output[17] = 0;
  bool zero = true, broadcast = true;
  for (unsigned b : bytes) { zero &= b == 0; broadcast &= b == 255; }
  return !(bytes[0] & 1) && !zero && !broadcast;
}
inline bool fleetValidNonce(const char* nonce) {
  if (!nonce || strlen(nonce) != 16) return false;
  for (int i = 0; i < 16; ++i) if (!isxdigit(static_cast<unsigned char>(nonce[i]))) return false;
  return true;
}
inline int fleetAxis(uint8_t value) {
  const int delta = int(value) - 128;
  return delta < 0 ? delta * 4 : delta * 512 / 127;
}
