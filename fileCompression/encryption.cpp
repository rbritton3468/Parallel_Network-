#include <vector>
#include <iostream>
#include <cstdint>
#include <cstring>
#include "encryption.hpp"

const u_int8_t global_key[32] = {
    0x50, 0x61, 0x62, 0x6C, 0x6F, 0x20, 0x66, 0x6F,
    0x72, 0x20, 0x50, 0x72, 0x65, 0x73, 0x69, 0x64,
    0x65, 0x6E, 0x74, 0x20, 0x6F, 0x66, 0x20, 0x43,
    0x6F, 0x6C, 0x6F, 0x6D, 0x62, 0x69, 0x61, 0x21
};

u_int32_t rotate_word(u_int32_t input, int shift) {
    return (input << shift) | (input >> (32 - shift));
}

void update_constants(u_int32_t& a, u_int32_t& b, u_int32_t& c, u_int32_t& d) {
    a += b;
    d ^= a;
    d = rotate_word(d, 16);

    c += d;
    b ^= c;
    b = rotate_word(b, 12);

    a += b;
    d ^= a;
    d = rotate_word(d, 8);

    c += d;
    b ^= c;
    b = rotate_word(b, 7);
}

void generate_random_pattern(u_int32_t pattern[16], const u_int32_t key[8], const uint32_t nonce[3], u_int32_t counter) {
    const uint32_t constants[4] = {
        0x61707865,
        0x3320646e,
        0x79622d32,
        0x6b206574
    };

    uint32_t state[16];

    memcpy(&state[0], constants, sizeof(constants));
    memcpy(&state[4], key, 32);
    state[12] = counter;
    memcpy(&state[13], nonce, 12);

    memcpy(pattern, state, 64);

    for (int i = 0; i < 10; i++) {
        update_constants(pattern[0], pattern[4], pattern[8], pattern[12]);
        update_constants(pattern[1], pattern[5], pattern[9], pattern[13]);
        update_constants(pattern[2], pattern[6], pattern[10], pattern[14]);
        update_constants(pattern[3], pattern[7], pattern[11], pattern[15]);

        update_constants(pattern[0], pattern[5], pattern[10], pattern[15]);
        update_constants(pattern[1], pattern[6], pattern[11], pattern[12]);
        update_constants(pattern[2], pattern[7], pattern[8], pattern[13]);
        update_constants(pattern[3], pattern[4], pattern[9], pattern[14]);
    }

    for (int i = 0; i < 16; ++i) {
        pattern[i] += state[i];
    }
}

std::vector<uint8_t> encrypt_data(const std::vector<uint8_t>& data, const u_int8_t key[32], const uint8_t nonce[12]) {
    std::vector<uint8_t> encrypted(data.size());
    u_int32_t counter = 0;

    u_int32_t key_words[8];
    u_int32_t nonce_words[3];
    for (int i = 0; i < 8; ++i) {
        key_words[i] = ((uint32_t *)key)[i];
    }
    for (int i = 0; i < 3; ++i) {
        nonce_words[i] = ((uint32_t *)nonce)[i];
    }

    //Loop through data in blocks of 64 bytes
    for (size_t i = 0; i < data.size(); i += 64) {
        u_int32_t pattern[16];
        generate_random_pattern(pattern, key_words, nonce_words, counter++);

        // XOR the data with the generated pattern
        for (size_t j = 0; j < 64 && (i + j) < data.size(); ++j) {
            encrypted[i + j] = data[i + j] ^ ((uint8_t *)pattern)[j];
        }
    }

    return encrypted;
}