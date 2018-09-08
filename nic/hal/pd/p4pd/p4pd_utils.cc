// {C} Copyright 2017 Pensando Systems Inc. All rights reserved
#include <stdio.h>
#include <string>
#include <errno.h>
#include <stdlib.h>
#include <assert.h>

#include "nic/hal/pd/p4pd/p4pd_utils.hpp"
/* This function copies a byte at time or a single bit that goes
 * into table memory
 */
void
p4pd_utils_copy_into_hwentry(uint8_t *dest,
                             uint16_t dest_start_bit,
                             uint8_t *src,
                             uint16_t src_start_bit,
                             uint16_t num_bits)
{

    (void)p4pd_utils_copy_into_hwentry;

    uint8_t src_byte, dest_byte, dest_byte_mask;
    uint8_t bits_in_src_byte1, bits_in_src_byte2 = 0;

    if (!num_bits || src == NULL || num_bits > 8) {
        return;
    }

    // destination start bit is in bit.. Get byte corresponding to it.
    dest += dest_start_bit / 8;

    /* Data packing should be in BE format. The data provided to  MPU and
     * KM (for comparing key) is in BE. Hence when lookup hit occurs, the
     * return memory line of data (K,D) should be in the format that is
     * agreeable to KM and MPU. Current understanding is that KM and MPU
     * see key bits and action-data bits in BE format.
     */
    if (num_bits == 8) {
        // copying a byte from source to destination
        if (src_start_bit) {
            bits_in_src_byte1 = (*src << (7 - src_start_bit % 8));
            bits_in_src_byte2 = (*(src - 1)  >> ((src_start_bit % 8) + 1));
        } else {
            bits_in_src_byte1 = (*src >> src_start_bit % 8);
        }
        src_byte = bits_in_src_byte1 | bits_in_src_byte2;
        if (dest_start_bit % 8) {
            assert(0);
            // the following code can be enabled if we hit the assert later
            // copying a byte starting from non byte aligned destination start bit.
            for (int q = 0; q < 8; q++) {
                if (((dest_start_bit % 8) + q) == 8) {
                    //use next byte
                    dest++;
                }
                dest_byte = *dest;
                dest_byte_mask = 0xFF ^ ( 1 << ((dest_start_bit + q) % 8));
                dest_byte &= dest_byte_mask;
                // get that single bit from source
                src_byte = (*src >> ((src_start_bit + q) % 8)) & 0x1;
                // position src bit at place where it should go in dest
                src_byte <<= ((dest_start_bit + q) % 8);
                dest_byte |= src_byte;
                *dest |= dest_byte;
            }
        } else {
            // When copying a byte from source to destination,
            // destination start bit is on byte aligned boundary.
            // So just copy the entire byte.
            *dest = src_byte;
        }
    } else {
        // copying a single bit from src to destination
        // clear out single bit in destination where a bit
        // from source will be copied into.
        dest_byte = *dest;
        dest_byte_mask = 0xFF ^ ( 1 << (dest_start_bit % 8));
        dest_byte &= dest_byte_mask;
        // get that single bit from source
        src_byte = (*src >> (src_start_bit % 8)) & 0x1;
        // position src bit at place where it should go in dest
        src_byte <<= (dest_start_bit % 8);
        dest_byte |= src_byte;
        *dest |= dest_byte;
    }
}



/* dest_start bit is 0 - 7 within dest
 * src_start_bit  is 0 - 7 within src */
void
p4pd_utils_copy_single_bit(uint8_t *dest,
                           uint16_t dest_start_bit,
                           uint8_t *src,
                           uint16_t src_start_bit,
                           uint16_t num_bits)
{
    (void)p4pd_utils_copy_single_bit;
    uint8_t src_byte, dest_byte, dest_byte_mask;
    if (!num_bits || src == NULL || num_bits != 1) {
        return;
    }
    // copying a single bit from src to destination
    // clear out single bit in destination where a bit
    // from source will be copied into.
    dest_byte = *dest;
    dest_byte_mask = 0xFF ^ ( 1 << (dest_start_bit % 8));
    dest_byte &= dest_byte_mask;
    // get that single bit from source
    src_byte = (*src >> (src_start_bit % 8)) & 0x1;
    // position src bit at place where it should go in dest
    src_byte <<= (dest_start_bit % 8);
    dest_byte |= src_byte;
    *dest |= dest_byte;
}


void
p4pd_utils_copy_le_src_to_be_dest(uint8_t *dest,
                                  uint16_t dest_start_bit,
                                  uint8_t *src,
                                  uint16_t src_start_bit,
                                  uint16_t num_bits)
{
    (void)p4pd_utils_copy_le_src_to_be_dest;

    if (!num_bits || src == NULL) {
        return;
    }

    for (int k = 0; k < num_bits; k++) {
        uint8_t *_dest = dest + ((dest_start_bit + k) / 8);
        // Read from Msbit in source to lsb
        uint8_t *_src = src + ((src_start_bit + num_bits - 1 - k) / 8);
        p4pd_utils_copy_single_bit(_dest,
                             7 - ((dest_start_bit + k) % 8),
                             _src,
                             (src_start_bit + num_bits - 1 - k) % 8,
                             1);
    }
}

void
p4pd_utils_copy_be_src_to_be_dest(uint8_t *dest,
                                  uint16_t dest_start_bit,
                                  uint8_t *src,
                                  uint16_t src_start_bit,
                                  uint16_t num_bits)
{
    (void)p4pd_utils_copy_be_src_to_be_dest;

    if (!num_bits || src == NULL) {
        return;
    }

    /* when both src and dest start on byte boundary, optimize copy */
    if (!(dest_start_bit % 8) && !(src_start_bit % 8)) {
        dest += (dest_start_bit >> 3);
        src += (src_start_bit >> 3);
        while (num_bits >= 32) {
            *(uint32_t*)dest = *(uint32_t*)src;
            dest += 4;
            src += 4;
            num_bits -= 32;
        }
        while (num_bits >= 16) {
            *(uint16_t*)dest = *(uint16_t*)src;
            num_bits -= 16;
            dest += 2;
            src += 2;
        }
        while (num_bits >= 8) {
            *dest = *src;
            num_bits -= 8;
            dest++;
            src++;
        }
        dest_start_bit = 0;
        src_start_bit = 0;
    }

    for (int k = 0; k < num_bits; k++) {
        uint8_t *_dest = dest + ((dest_start_bit + k) / 8);
        uint8_t *_src = src + ((src_start_bit + k) / 8);
        p4pd_utils_copy_single_bit(_dest,
                             7 - ((dest_start_bit + k) % 8),
                             _src,
                             7 - ((src_start_bit + k) % 8),
                             1);
    }
}

void
p4pd_utils_copy_be_src_to_le_dest(uint8_t *dest,
                                  uint16_t dest_start_bit,
                                  uint8_t *src,
                                  uint16_t src_start_bit,
                                  uint16_t num_bits)
{
    (void)p4pd_utils_copy_be_src_to_le_dest;

    if (!num_bits || src == NULL) {
        return;
    }

    for (int k = 0; k < num_bits; k++) {
        uint8_t *_src = src + ((src_start_bit + k) / 8);
        // Copy into Msbit in destination
        uint8_t *_dest = dest - ((dest_start_bit + k) / 8);
        p4pd_utils_copy_single_bit(_dest,
                             (dest_start_bit + num_bits - 1 - k) % 8,
                             _src,
                             7 - ((src_start_bit + k) % 8),
                             1);
    }
}

void
p4pd_utils_copy_be_adata_to_le_dest(uint8_t *dest,
                                    uint16_t dest_start_bit,
                                    uint8_t *src,
                                    uint16_t src_start_bit,
                                    uint16_t num_bits)
{
    (void)p4pd_utils_copy_be_adata_to_le_dest;

    if (!num_bits || src == NULL) {
        return;
    }

    for (int k = 0; k < num_bits; k++) {
        uint8_t *_src = src + ((src_start_bit + k) >> 3);
        uint8_t *_dest = dest + ((dest_start_bit + num_bits - 1 - k) >> 3);
        p4pd_utils_copy_single_bit(_dest,
                             (dest_start_bit + num_bits - 1 - k) & 0x7,
                             _src,
                             7 - ((src_start_bit + k) & 0x7),
                             1);
    }
}

void
p4pd_utils_copy_byte_aligned_src_and_dest(uint8_t *dest,
                                          uint16_t dest_start_bit,
                                          uint8_t *src,
                                          uint16_t src_start_bit,
                                          uint16_t num_bits)
{
    (void)p4pd_utils_copy_byte_aligned_src_and_dest;

    if (!num_bits || src == NULL) {
        return;
    }

    // destination start bit is in bit.. Get byte corresponding to it.
    dest += dest_start_bit / 8;
    src += src_start_bit / 8;

    int to_copy_bits = num_bits;
    while (to_copy_bits >= 32) {
        *(uint32_t*)dest = *(uint32_t*)src;
        dest += 4;
        src += 4;
        to_copy_bits -= 32;
    }
    while (to_copy_bits >= 16) {
        *(uint16_t*)dest = *(uint16_t*)src;
        to_copy_bits -= 16;
        dest += 2;
        src += 2;
    }
    while (to_copy_bits >= 8) {
        *dest = *src;
        to_copy_bits -= 8;
        dest++;
        src++;
    }
    // Remaning bits  (less than 8) need to be copied.
    // They need to be copied from MS bit to LSB
    for (int k = 0; k < to_copy_bits; k++) {
        p4pd_utils_copy_single_bit(dest,
                             7 - k,
                             src,
                             7 - k,
                             1);
    }
}


void
p4pd_utils_swizzle_bytes(uint8_t *hwentry, uint16_t hwentry_bit_len)
{

    (void)p4pd_utils_swizzle_bytes;

    uint16_t entry_byte_len = hwentry_bit_len;
    entry_byte_len += (hwentry_bit_len % 16) ?  (16 - (hwentry_bit_len % 16)) : 0;
    entry_byte_len >>= 3;
    // Swizzle hwentry bytes
    // Go over half of the byte array and swap bytes
    for (int i = 0; i < (entry_byte_len >> 1); i++) {
        uint8_t temp = hwentry[i];
        hwentry[i] = hwentry[entry_byte_len - i - 1];
        hwentry[entry_byte_len - i - 1] = temp;
    }
}

uint32_t
p4pd_utils_widekey_hash_table_entry_prepare(uint8_t *hwentry,
                                            uint8_t action_pc,
                                            uint16_t match_key_start_bit,
                                            uint8_t *hwkey,
                                            uint16_t keylen,
                                            uint8_t *packed_actiondata_before_matchkey,
                                            uint16_t actiondata_before_matchkey_len,
                                            uint8_t *packed_actiondata_after_matchkey,
                                            uint16_t actiondata_after_matchkey_len,
                                            uint16_t *axi_shift_bytes)
{
    (void)p4pd_utils_widekey_hash_table_entry_prepare;
    uint16_t dest_start_bit = (keylen - (keylen % 512)), delta_bits = 0;

    p4pd_utils_copy_be_src_to_be_dest(hwentry,
                   0,
                   hwkey,
                   0, /* Starting key bit in hwkey*/
                   dest_start_bit);

    if (action_pc != 0xff) {
        if (match_key_start_bit > (actiondata_before_matchkey_len + P4PD_ACTIONPC_BITS)){
            delta_bits = (match_key_start_bit - (actiondata_before_matchkey_len
                                                     + P4PD_ACTIONPC_BITS));
        }
    } else {
        if (match_key_start_bit > actiondata_before_matchkey_len) {
            delta_bits = match_key_start_bit - actiondata_before_matchkey_len;
        }
    }
    *axi_shift_bytes = ((delta_bits >> 4) << 1);

    // For wide key table, axi_shift will not make sense.
    assert(*axi_shift_bytes == 0);

    if (action_pc != 0xff) {
        *(hwentry + (dest_start_bit >> 3)) = action_pc; // ActionPC is a byte
        dest_start_bit += P4PD_ACTIONPC_BITS;
    }

    p4pd_utils_copy_byte_aligned_src_and_dest(hwentry,
                   dest_start_bit,
                   packed_actiondata_before_matchkey,
                   0, /* Starting from 0th bit in source */
                   actiondata_before_matchkey_len);

    /* write key portion that is in last flit */
    p4pd_utils_copy_be_src_to_be_dest(hwentry,
                   dest_start_bit + actiondata_before_matchkey_len,
                   hwkey,
                   dest_start_bit + actiondata_before_matchkey_len, /* Starting key bit in hwkey */
                   (keylen - dest_start_bit));

    dest_start_bit += keylen - dest_start_bit;
    dest_start_bit += actiondata_before_matchkey_len;

    p4pd_utils_copy_be_src_to_be_dest(hwentry,
                                dest_start_bit,
                                packed_actiondata_after_matchkey,
                                0,
                                actiondata_after_matchkey_len);

    dest_start_bit += actiondata_after_matchkey_len;

    /* when computing size of entry, drop axi shift bit len */
    dest_start_bit -= (*axi_shift_bytes * 8);
    if (dest_start_bit % 16) {
        return (dest_start_bit + 16 - (dest_start_bit % 16));
    } else {
        return (dest_start_bit);
    }
}

uint32_t
p4pd_utils_hash_table_entry_prepare(uint8_t *hwentry,
                                    uint8_t action_pc,
                                    uint16_t match_key_start_bit,
                                    uint8_t *hwkey,
                                    uint16_t keylen,
                                    uint8_t *packed_actiondata_before_matchkey,
                                    uint16_t actiondata_before_matchkey_len,
                                    uint8_t *packed_actiondata_after_matchkey,
                                    uint16_t actiondata_after_matchkey_len,
                                    uint16_t *axi_shift_bytes)
{
    (void)p4pd_utils_hash_table_entry_prepare;

    uint16_t dest_start_bit = 0, delta_bits = 0;

    if (action_pc != 0xff) {
        if (match_key_start_bit > (actiondata_before_matchkey_len + P4PD_ACTIONPC_BITS)){
            delta_bits = (match_key_start_bit - (actiondata_before_matchkey_len
                                                     + P4PD_ACTIONPC_BITS));
        }
    } else {
        if (match_key_start_bit > actiondata_before_matchkey_len) {
            delta_bits = match_key_start_bit - actiondata_before_matchkey_len;
        }
    }
    *axi_shift_bytes = ((delta_bits >> 4) << 1);
    dest_start_bit = (*axi_shift_bytes  * 8);

    if (action_pc != 0xff) {
        *(hwentry + (dest_start_bit >> 3)) = action_pc; // ActionPC is a byte
        dest_start_bit += P4PD_ACTIONPC_BITS;
    }

    p4pd_utils_copy_byte_aligned_src_and_dest(hwentry,
                   dest_start_bit,
                   packed_actiondata_before_matchkey,
                   0, /* Starting from 0th bit in source */
                   actiondata_before_matchkey_len);
    dest_start_bit += actiondata_before_matchkey_len;

    //For hash tables, match-key-start position has to align.
    dest_start_bit = match_key_start_bit;

    p4pd_utils_copy_be_src_to_be_dest(hwentry,
                   dest_start_bit,
                   hwkey,
                   match_key_start_bit, /* Starting key bit in hwkey*/
                   keylen);
    dest_start_bit += keylen;

    int actiondata_startbit = 0;
    int key_byte_shared_bits = 0;

    p4pd_utils_copy_be_src_to_be_dest(hwentry,
                                dest_start_bit,
                                packed_actiondata_after_matchkey,
                                actiondata_startbit,
                                (actiondata_after_matchkey_len - key_byte_shared_bits));

    dest_start_bit += (actiondata_after_matchkey_len - key_byte_shared_bits);

    /* when computing size of entry, drop axi shift bit len */
    dest_start_bit -= (*axi_shift_bytes * 8);
    if (dest_start_bit % 16) {
        return (dest_start_bit + 16 - (dest_start_bit % 16));
    } else {
        return (dest_start_bit);
    }
}

// Return hw table entry width
uint32_t
p4pd_utils_p4table_entry_prepare(uint8_t *hwentry,
                                 uint8_t action_pc,
                                 uint8_t *hwkey,
                                 uint16_t keylen,
                                 uint8_t *packed_actiondata,
                                 uint16_t actiondata_len)
{
    (void)p4pd_utils_p4table_entry_prepare;

    uint16_t dest_start_bit = 0;

    if (action_pc != 0xff) {
        *(hwentry + (dest_start_bit >> 3)) = action_pc; // ActionPC is a byte
        dest_start_bit += P4PD_ACTIONPC_BITS;
    }

    p4pd_utils_copy_byte_aligned_src_and_dest(hwentry,
                   dest_start_bit,
                   hwkey,
                   0, /* Starting from 0th bit in source */
                   keylen);
    dest_start_bit += keylen;

    int actiondata_startbit = 0;
    int key_byte_shared_bits = 0;

    p4pd_utils_copy_be_src_to_be_dest(hwentry,
                                dest_start_bit,
                                packed_actiondata,
                                actiondata_startbit,
                                (actiondata_len - key_byte_shared_bits));

    dest_start_bit += (actiondata_len - key_byte_shared_bits);

    // When swizzling bytes, 16b unit is used. Hence increase size.
    if (dest_start_bit % 16) {
        return (dest_start_bit + 16 - (dest_start_bit % 16));
    } else {
        return (dest_start_bit);
    }
}

