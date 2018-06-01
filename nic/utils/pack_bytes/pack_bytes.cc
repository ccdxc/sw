// {C} Copyright 2017 Pensando Systems Inc. All rights reserved

#include "pack_bytes.hpp"

namespace hal{
namespace utils {

void pack_bytes_pack(uint8_t *bytes, uint32_t start, uint32_t width, uint64_t val) {
    if (width) {
        uint32_t start_byte = start >> 3; // Index of the first byte = bits / 8
        uint8_t start_bits = start & 0x07; // Index of the first bit = bits % 8
        uint32_t end = start + width - 1; // Index of the last bit
        uint32_t end_byte = end >> 3; // Index of the last byte = bits / 8
        uint8_t end_bits = end & 0x07; // Index of the last bit = bits % 8
        uint8_t start_masks[8] = {0xFF, 0xFE, 0xFC, 0xF8, 0xF0, 0xE0, 0xC0, 0x80};
        uint8_t end_masks[8] = {0x01, 0x03, 0x07, 0x0F, 0x1F, 0x3F, 0x7F, 0xFF};

        for (uint32_t i = start_byte; i <= end_byte; i++) {
            uint8_t mask = 0xFF;
            uint8_t shft = 0;

            if (i == start_byte) {
                mask &= start_masks[start_bits];
                shft = start_bits;
            }
            if (i == end_byte) {
                mask &= end_masks[end_bits];
            }

            bytes[i] &= (~mask);
            bytes[i] |= ((val << shft) & mask);
            val = (val >> (8 - shft));
        }
    }
}

uint64_t pack_bytes_unpack(uint8_t *bytes, uint32_t start, uint32_t width) {
    uint64_t val = 0;

    if (width) {
        uint32_t start_byte = start >> 3; // Index of the first byte = bits / 8
        uint8_t start_bits = start & 0x07; // Index of the first bit = bits % 8
        uint32_t end = start + width - 1; // Index of the last bit
        uint32_t end_byte = end >> 3; // Index of the last byte = bits / 8
        uint8_t end_bits = end & 0x07; // Index of the last bit = bits % 8
        uint8_t start_masks[8] = {0xFF, 0xFE, 0xFC, 0xF8, 0xF0, 0xE0, 0xC0, 0x80};
        uint8_t end_masks[8] = {0x01, 0x03, 0x07, 0x0F, 0x1F, 0x3F, 0x7F, 0xFF};

        for (uint32_t i = start_byte; i <= end_byte; i++) {
            uint8_t mask = 0xFF;

            if (i == start_byte) {
                mask &= start_masks[start_bits];
            }
            if (i == end_byte) {
                mask &= end_masks[end_bits];
            }

            val |= ((((uint64_t)(bytes[i] & mask)) << ((i - start_byte) * 8)) >> start_bits);
        }
    }

    return val;
}
}
}