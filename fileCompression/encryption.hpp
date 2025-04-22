#pragma once
#include <vector>

// This encryption algorithm is symmetric, encrypt the encrypted data with the same key and nonce to decrypt it.

// The global key is included and should remain constant, but the nonce will need to be random generated for each file and passed with the data.

// The nonce is an array of 12 uint8_t that should be generated with a pseudorandom generator or a hash of the file.

std::vector<u_int8_t> encrypt_data(const std::vector<u_int8_t>& data, const u_int8_t key[32], const u_int8_t nonce[12]);