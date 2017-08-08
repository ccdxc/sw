



/*
 * p4pd.c
 * Mahesh Shirshyad (Pensando Systems)
 */

/* This file contains API implementation needed to operate on each P4 Table. 
 *
 * For every P4table, 
 *
 *   1. In order to keep application layer agnostic of hardware representation 
 *      folllowing APIs are provided.
 *      1.a  <table_name>_hwkey_query() which returns hwkey length, 
 *           hwactiondata length associated with the key. These lengths
 *           can be used to allocate memory before using p4 table operation
 *           APIs. All table operation APIs expect caller of the APIs to
 *           provide necessary memory for table operation API arguments.
 *      1.b  <table_name>_hwkey_build() which converts table key from software
 *           representation to hardware representation.
 *
 *   2. In order to install an entry into P4 table or read an entry from p4-table
 *      following APIs are provided
 *          <table_name>_entry_write()
 *          <table_name>_entry_read()
 */

/*
 * This file groups APIs to operate on p4 table on each P4-table basis.
 */

/*
 * This file is generated from P4 program. Any changes made to this file will
 * be lost.
 */

#include <stdio.h>
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <assert.h>
#include "p4pd.h"

#define P4PD_LOG_TABLE_UPDATES  /* Disable this once no more table writes logging is needed */

#define P4PD_MAX_ACTION_DATA_LEN (512)
#define P4PD_MAX_MATCHKEY_LEN    (512) /* When multiple flits can be
                                        * chained, then key max size
                                        * can be more than 512. For
                                        * assuming one flit.
                                        */

#define P4PD_ACTIONPC_BITS (8)      /* For now assume 8bit actionPC.
                                     * 9bits, change it to 2 bytes.
                                     */
#define P4PD_TCAM_WORD_CHUNK_LEN (16) /* Tcam word chunk */

char p4pd_tbl_names[P4TBL_ID_TBLMAX][P4TBL_NAME_MAX_LEN];
uint16_t p4pd_tbl_swkey_size[P4TBL_ID_TBLMAX];
uint16_t p4pd_tbl_sw_action_data_size[P4TBL_ID_TBLMAX];

extern int capri_table_entry_write(uint32_t tableid,
                            uint32_t index,
                            uint8_t  *hwentry,
                            uint16_t hwentry_bit_len);


extern int capri_table_entry_read(uint32_t tableid,
                           uint32_t index,
                           uint8_t  *hwentry,
                           uint16_t *hwentry_bit_len);


extern int capri_tcam_table_entry_write(uint32_t tableid,
                                 uint32_t index,
                                 uint8_t  *trit_x,
                                 uint8_t  *trit_y,
                                 uint16_t hwentry_bit_len);

extern int capri_tcam_table_entry_read(uint32_t tableid,
                                uint32_t index,
                                uint8_t  *trit_x,
                                uint8_t  *trit_y,
                                uint16_t *hwentry_bit_len);

extern int capri_hbm_table_entry_write(uint32_t tableid,
                                       uint32_t index,
                                       uint8_t *hwentry,
                                       uint16_t entry_size);

extern int capri_hbm_table_entry_read(uint32_t tableid,
                                      uint32_t index,
                                      uint8_t *hwentry,
                                      uint16_t *entry_size);

extern uint8_t capri_get_action_pc(uint32_t tableid, 
                                   uint8_t actionid);

extern uint8_t capri_get_action_id(uint32_t tableid, 
                                   uint8_t actionpc);

p4pd_error_t p4pd_table_ds_decoded_string_get(uint32_t   tableid,
                                              void*      sw_key,
                                              /* Valid only in case of TCAM;
                                               * Otherwise can be NULL) 
                                               */
                                              void*      sw_key_mask,
                                              void*      action_data,
                                              char*      buffer,
                                              uint16_t   buf_len);

/* This function copies a byte at time or a single bit that goes 
 * into table memory 
 */
static void p4pd_copy_into_hwentry(uint8_t *dest,
                                   uint16_t dest_start_bit,
                                   uint8_t *src,
                                   uint16_t src_start_bit,
                                   uint16_t num_bits)
{


    (void)p4pd_copy_into_hwentry;


    uint8_t src_byte, dest_byte, dest_byte_mask;
    uint8_t bits_in_src_byte1, bits_in_src_byte2;

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
     * Data copied into SRAM/TCAM/HBM tables will be as per KM format.
     * KM will specify which byte of key will be placed in which byte position
     * in memory.
     */
    if (num_bits > 1) {
        // copying a byte from source to destination
        bits_in_src_byte1 = (*src >> src_start_bit % 8);
        bits_in_src_byte2 = (*(src + 1)  & ((1 << (src_start_bit % 8)) - 1 ));
        bits_in_src_byte2 = bits_in_src_byte2  << (8 - src_start_bit % 8);
        src_byte = bits_in_src_byte2 | bits_in_src_byte1;

        // When copying a byte from source to destination, 
        // destination start bit should also be on byte aligned boundary.
    
        // So just copy the entire byte.
        *dest = src_byte;   
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

/* This function copies a byte at time or a single bit that goes 
 * into p4field (match key struct p4field)
 */
static void p4pd_copy_into_p4field(uint8_t *dest,
                                   uint16_t dest_start_bit,
                                   uint8_t *src,
                                   uint16_t src_start_bit,
                                   uint16_t num_bits)
{
    (void)p4pd_copy_into_p4field;

    uint8_t src_byte, dest_byte, dest_byte_mask;
    uint8_t bits_in_dest_byte1, bits_in_dest_byte2;

    if (!num_bits || src == NULL || num_bits > 8) {
        return;
    }
    
    // When copying into p4field, clear destination byte.
    // *dest = 0; <--- Cannot do this; Clears out earlier set bits.

    // Source start bit is offset from the start of the hwentry.
    src += src_start_bit / 8;

    // destination start bit is in bit within a byte.. 
    // It might be required to copy into next byte depending
    // on the start bit 
    if (num_bits > 1) {
        // copying a byte from source to destination
        bits_in_dest_byte1 = (*src << dest_start_bit % 8);
        bits_in_dest_byte2 = (*src >> (8 - dest_start_bit % 8));
        *dest &= ((1 << (dest_start_bit % 8)) - 1);
        *dest |= bits_in_dest_byte1;
        *(dest + 1) &= 0xFF ^ ((1 << (8 - dest_start_bit % 8)) - 1);
        *(dest + 1) |= bits_in_dest_byte2;
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


static void p4pd_copy_actiondata_be_order(uint8_t *dest,
                                uint16_t dest_start_bit,
                                uint8_t *src,
                                uint16_t src_start_bit,
                                uint16_t num_bits)
{
    (void)p4pd_copy_actiondata_be_order;

    if (!num_bits || src == NULL) {
        return;
    }

    uint8_t * _dest = dest;
    for (int k = 0; k < num_bits; k++) {
        _dest = dest + ((dest_start_bit + k) / 8);
        //  b7 b6 b5 b4 b3 b2 b1 b0 b7 b6 b5 b4b b3 b2 b1 b0
        p4pd_copy_into_p4field(_dest,
                               7 - ((dest_start_bit + k) % 8),
                               src,
                               src_start_bit + num_bits - 1 - k,
                               1);
    }
}

static void p4pd_place_key_and_actiondata_in_byte(uint8_t *dest,
                                uint16_t dest_start_bit,
                                uint8_t *src,
                                uint16_t src_start_bit,
                                uint16_t num_bits)
{
    (void)p4pd_place_key_and_actiondata_in_byte;

    if (!num_bits || src == NULL) {
        return;
    }

    uint8_t * _dest = dest;
    _dest = dest + ((dest_start_bit) / 8);

    p4pd_copy_into_p4field(_dest,
                           (dest_start_bit % 8),
                           src,
                           7 - (src_start_bit % 8),
                           1);
}

static void p4pd_place_actiondata_bit7_to_bit0_order(uint8_t *dest,
                                uint16_t dest_start_bit,
                                uint8_t *src,
                                uint16_t src_start_bit,
                                uint16_t num_bits)
{
    (void)p4pd_place_actiondata_bit7_to_bit0_order;

    if (!num_bits || src == NULL) {
        return;
    }

    uint8_t * _dest = dest;
    uint8_t * _src = src;
    _dest = dest + ((dest_start_bit) / 8);
    _src = src + ((src_start_bit) / 8);

    p4pd_copy_into_p4field(_dest,
                           7 - (dest_start_bit % 8),
                           _src,
                           7 - (src_start_bit % 8),
                           1);
}


static void p4pd_copy_multibyte(uint8_t *dest,
                                uint16_t dest_start_bit,
                                uint8_t *src,
                                uint16_t src_start_bit,
                                uint16_t num_bits)
{
    (void)p4pd_copy_multibyte;

    if (!num_bits || src == NULL) {
        return;
    }
    
    // destination start bit is in bit.. Get byte corresponding to it.
    dest += dest_start_bit / 8;

    int to_copy_bits = num_bits;
    while (to_copy_bits >= 8) {
        p4pd_copy_into_p4field(dest,
                               dest_start_bit,
                               src,
                               src_start_bit,
                               8);
        to_copy_bits -= 8;
        dest_start_bit += 8;
        src_start_bit += 8;
        dest++;
    }
    // Remaning bits  (less than 8) need to be copied.
    // TODO : Optimize this.
    for (int k = 0; k < to_copy_bits; k++) {
        p4pd_copy_into_p4field(dest,
                               dest_start_bit,
                               src,
                               src_start_bit,
                               1);
        dest_start_bit++;
        src_start_bit++;
    }
}


static void p4pd_swizzle_bytes(uint8_t *hwentry, uint16_t hwentry_bit_len) 
{

    (void)p4pd_swizzle_bytes;

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


static uint32_t p4pd_hash_table_entry_prepare(uint8_t *hwentry, 
                                            uint8_t action_pc, 
                                            uint8_t match_key_start_byte,
                                            uint8_t *hwkey, 
                                            uint16_t keylen,
                                            uint8_t *packed_actiondata_before_matchkey,
                                            uint16_t actiondata_before_matchkey_len,
                                            uint8_t *packed_actiondata_after_matchkey,
                                            uint16_t actiondata_after_matchkey_len)
{
    (void)p4pd_hash_table_entry_prepare;

    uint16_t dest_start_bit = 0;

    p4pd_copy_multibyte(hwentry,
                   dest_start_bit,
                   &action_pc,
                   0, /* Starting from 0th bit in source */
                   P4PD_ACTIONPC_BITS);
    dest_start_bit += P4PD_ACTIONPC_BITS;


    // TODO: Copying action data before key need to be fixed.
    p4pd_copy_multibyte(hwentry,
                   dest_start_bit,
                   packed_actiondata_before_matchkey,
                   0, /* Starting from 0th bit in source */
                   actiondata_before_matchkey_len);
    dest_start_bit += actiondata_before_matchkey_len;

    p4pd_copy_multibyte(hwentry,
                   dest_start_bit,
                   hwkey,
                   //TODO: Take match_key_start_bit argument -- needed when match-key starts within a byte.
                   match_key_start_byte * 8, /* Starting key bit in hwkey which is always 512b */
                   keylen);
    dest_start_bit += keylen;

    int actiondata_startbit = 0;
    int key_byte_shared_bits = 0;

    if (dest_start_bit % 8) {
        // Action data and key bits are packed within a byte.
        // Handle this case...
        for (int k = 7 - (dest_start_bit % 8); k >= 0 ; k--) {
            p4pd_place_key_and_actiondata_in_byte(hwentry,
                                                  dest_start_bit - (dest_start_bit % 8) + k,
                   packed_actiondata_after_matchkey,
                   actiondata_startbit++,
                   1);
        }
        key_byte_shared_bits = 8 - (dest_start_bit % 8);
        
    }
    dest_start_bit +=  key_byte_shared_bits;

    // TODO : Optimize this... For now copying 1 bit at a time.
    for (int k = 0; k < actiondata_after_matchkey_len - key_byte_shared_bits; k++) {
        p4pd_place_actiondata_bit7_to_bit0_order(hwentry,
                                             dest_start_bit + k,
                                             packed_actiondata_after_matchkey,
                                             actiondata_startbit++,
                                             1);
    }

    dest_start_bit += (actiondata_after_matchkey_len - key_byte_shared_bits);

    p4pd_swizzle_bytes(hwentry, dest_start_bit);

    // When swizzling bytes, 16b unit is used. Hence increase size.
    if (dest_start_bit % 16) {
        return (dest_start_bit + 16 - (dest_start_bit % 16));
    } else {
        return (dest_start_bit);
    }
}

// Return hw table entry width
static uint32_t p4pd_p4table_entry_prepare(uint8_t *hwentry, 
                                           uint8_t action_pc, 
                                           uint8_t *hwkey, 
                                           uint16_t keylen,
                                           uint8_t *packed_actiondata,
                                           uint16_t actiondata_len)
{
    (void)p4pd_p4table_entry_prepare;

    uint16_t dest_start_bit = 0;

    p4pd_copy_multibyte(hwentry,
                   dest_start_bit,
                   &action_pc,
                   0, /* Starting from 0th bit in source */
                   P4PD_ACTIONPC_BITS);
    dest_start_bit += P4PD_ACTIONPC_BITS;


    p4pd_copy_multibyte(hwentry,
                   dest_start_bit,
                   hwkey,
                   0, /* Starting from 0th bit in source */
                   keylen);
    dest_start_bit += keylen;

    int actiondata_startbit = 0;
    int key_byte_shared_bits = 0;

    if (dest_start_bit % 8) {
        // Action data and key bits are packed within a byte.
        // Handle this case...
        for (int k = 7 - (dest_start_bit % 8); k >= 0 ; k--) {
            p4pd_place_key_and_actiondata_in_byte(hwentry,
                                                  dest_start_bit - (dest_start_bit % 8) + k,
                   packed_actiondata,
                   actiondata_startbit++,
                   1);
        }
        key_byte_shared_bits = 8 - (dest_start_bit % 8);
    }
    dest_start_bit +=  key_byte_shared_bits;

    for (int k = 0; k < actiondata_len - key_byte_shared_bits; k++) {
        p4pd_place_actiondata_bit7_to_bit0_order(hwentry,
                                             dest_start_bit + k,
                                             packed_actiondata,
                                             actiondata_startbit++,
                                             1);
    }

    dest_start_bit += (actiondata_len - key_byte_shared_bits);

    p4pd_swizzle_bytes(hwentry, dest_start_bit);

    // When swizzling bytes, 16b unit is used. Hence increase size.
    if (dest_start_bit % 16) {
        return (dest_start_bit + 16 - (dest_start_bit % 16));
    } else {
        return (dest_start_bit);
    }
}


/* ------ Per table Functions  ------- */




/* Query key details for p4-table 'D_mpu_only'
 *
 * Arguments: 
 *  IN  : uint32_t tableid             : Table Id that identifies
 *                                       P4 table. This id is obtained
 *                                       from p4pd_table_id_enum.
 *
 *  OUT : uint32_t *hwkey_len          : hardware key length in bits.
 *  OUT : uint32_t *hwactiondata_len   : Action data length in bits. 
 *
 * Return Value: 
 *  None
 */
static void D_mpu_only_hwentry_query(uint32_t tableid, 
                                   uint32_t* hwactiondata_len)
{
    /* Among all actions of the table, this length is set to maximum
     * action data len so that higher layer can allocate maximum 
     * required memory to handle any action.
     */
    *hwactiondata_len = 0; 
    return;
}



static uint32_t D_mpu_only_pack_action_data(uint32_t tableid,
                                             D_mpu_only_actiondata *actiondata,
                                             uint8_t *packed_actiondata)
                                      
{
    uint16_t dest_start_bit;

    dest_start_bit = 0;
    (void)dest_start_bit;

    switch(actiondata->actionid) {
    }
    return (0);
}




/* Install entry into P4-table 'D_mpu_only'
 * at location 'hashindex'.
 *
 * Arguments: 
 *  IN  : uint32_t tableid                  : Table Id that identifies
 *                                            P4 table. This id is obtained
 *                                            from p4pd_table_id_enum.
 *  IN  : uint32_t hashindex                : Hash index where entry is 
 *                                            installed.
 *
 *  IN  : uint8_t *hwkey                    : Hardware key to be installed
 *                                            into P4-table
 *  IN  : D_mpu_only_actions_un *actiondata   : Action data associated with 
 *                                            the key.
 * 
 * Return Value: 
 *  pd_error_t                              : P4PD_SUCCESS / P4PD_FAIL
 */
static p4pd_error_t D_mpu_only_entry_write(uint32_t tableid,
                                         uint32_t index, 
                                         D_mpu_only_actiondata *actiondata)
{
    uint8_t  action_pc;
    // For entry update performance reasons 
    // avoid mem allocate/free. Memory for action data and 
    // table entry memory line is allocated on stack.
    uint8_t  packed_actiondata[P4PD_MAX_ACTION_DATA_LEN] = {0};
    uint8_t  hwentry[P4PD_MAX_MATCHKEY_LEN + P4PD_MAX_ACTION_DATA_LEN] = {0};
    uint16_t entry_size, actiondatalen;


    action_pc = capri_get_action_pc(tableid, actiondata->actionid);

    actiondatalen = D_mpu_only_pack_action_data(tableid, actiondata,
                                              packed_actiondata);

    entry_size = p4pd_p4table_entry_prepare(hwentry,
                                            action_pc,
                                            NULL /* Index Table. No MatchKey*/,
                                            0, /* Zero matchkeylen */
                                            packed_actiondata,
                                            actiondatalen);
    capri_table_entry_write(tableid, index, hwentry, entry_size);
    return (P4PD_SUCCESS);
}

static uint32_t D_mpu_only_unpack_action_data(uint32_t tableid,
                                            uint8_t actionid,
                                            uint8_t *packed_actiondata,
                                            D_mpu_only_actiondata *actiondata)
{
    uint16_t src_start_bit;
    uint16_t actiondatalen;
    
    (void)src_start_bit;

    actiondatalen = 0;
    src_start_bit = 0;

    memset(actiondata, 0, sizeof(D_mpu_only_actiondata));
    actiondata->actionid = actionid;

    switch(actiondata->actionid) {
    }
    return (actiondatalen);
}




/* Read hardware entry from P4-table 'D_mpu_only'. Read hw entry is decoded
 * and used to fill up matchkey and actiondata structures.
 *
 * Arguments: 
 *  IN   : uint32_t tableid                 : Table Id that identifies
 *                                            P4 table. This id is obtained
 *                                            from p4pd_table_id_enum.
 *
 *
 *  IN   : uint32_t hashindex               : Hash index where table entry is 
 *                                            read from.
 *  OUT  : D_mpu_only_swkey_t *swkey          : Software key read from P4-table
 *  OUT  : D_mpu_only_actions_un* *actiondata : Action data associated with the key.
 * 
 * Return Value: 
 *  pd_error_t                              : P4PD_SUCCESS / P4PD_FAIL
 */
static p4pd_error_t D_mpu_only_entry_read(uint32_t tableid,
                                        uint32_t index, 
                                        D_mpu_only_actiondata* actiondata)
{
    uint8_t  hwentry[P4PD_MAX_MATCHKEY_LEN + P4PD_MAX_ACTION_DATA_LEN] = {0};
    uint16_t hwentry_bit_len;
    uint8_t  actionpc;
    uint8_t *packed_actiondata_after_key;
    uint16_t actiondata_len_after_key, key_bit_len;
    
    (void)packed_actiondata_after_key;
    (void)actiondata_len_after_key;
    (void)key_bit_len;

    capri_table_entry_read(tableid, index, hwentry, &hwentry_bit_len);

    if (!hwentry_bit_len) {
        // Zero len!!
        return (P4PD_SUCCESS);
    }
    p4pd_swizzle_bytes(hwentry, hwentry_bit_len);
    // Split HW entry into 
    //  - actionPC
    //  - Data

    //TODO : How to handle case where actionpc is not in table...
    actionpc = hwentry[0]; // First byte is always action-pc
    actiondata->actionid = capri_get_action_id(tableid, actionpc);
    D_mpu_only_unpack_action_data(tableid, actiondata->actionid, hwentry+1, actiondata);
                       
    return (P4PD_SUCCESS);
}

/* Create hardware entry from P4-table 'D_mpu_only'. 
 * Created entry data encodes both Key and Action data.
 *
 * Arguments: 
 *  IN  : uint32_t tableid                 : Table Id that identifies
 *                                           P4 table. This id is obtained
 *                                           from p4pd_table_id_enum.
 *
 *  IN  : D_mpu_only_swkey_t *swkey          : Software key used in P4-table
 *  IN  : D_mpu_only_actions_un  *actiondata : Action data associated with the key.
 *
 *  OUT : uint8_t *hwentry                 : Action data associated with the key.
 * 
 * Return Value: 
 *  pd_error_t                             : P4PD_SUCCESS / P4PD_FAIL
 */
static p4pd_error_t D_mpu_only_hwentry_create(uint32_t tableid,
                                            D_mpu_only_actiondata* actiondata,
                                            uint8_t *hwentry)
{
    return (P4PD_SUCCESS);
}


/* Decode hardware entry from P4-table 'D_mpu_only'.
 *
 * Arguments: 
 *  IN   : uint32_t tableid                 : Table Id that identifies
 *                                            P4 table. This id is obtained
 *                                            from p4pd_table_id_enum.
 *
 *  OUT  : D_mpu_only_swkey_t *swkey          : Software key read from P4-table
 *  OUT  : D_mpu_only_actions_un* *actiondata : Action data associated with the key.
 * 
 * Return Value: 
 *  pd_error_t                              : P4PD_SUCCESS / P4PD_FAIL
 */
static p4pd_error_t D_mpu_only_entry_decode(uint32_t tableid,
                                          uint8_t *hwentry,
                                          uint16_t hwentry_len,
                                          D_mpu_only_actiondata* actiondata)
{
    uint8_t  actionpc;
    
    // Split HW entry into 
    //  - actionPC
    //  - Data

    //TODO : How to handle case where actionpc is not in table...
    actionpc = hwentry[0]; // First byte is always action-pc
    actiondata->actionid = capri_get_action_id(tableid, actionpc);
    D_mpu_only_unpack_action_data(tableid, actiondata->actionid, hwentry+1, actiondata);
                       
    return (P4PD_SUCCESS);
}


/* Query key details for p4-table 'C_idx'
 *
 * Arguments: 
 *  IN  : uint32_t tableid             : Table Id that identifies
 *                                       P4 table. This id is obtained
 *                                       from p4pd_table_id_enum.
 *
 *  OUT : uint32_t *hwactiondata_len   : Action data length in bits. 
 *
 * Return Value: 
 *  None
 */
static void C_idx_hwentry_query(uint32_t tableid, 
                                   uint32_t* hwactiondata_len)
{
    /* Among all actions of the table, this length is set to maximum
     * action data len so that higher layer can allocate maximum 
     * required memory to handle any action.
     */
    *hwactiondata_len = 8; 
    return;
}



static uint32_t C_idx_pack_action_data(uint32_t tableid,
                                             C_idx_actiondata *actiondata,
                                             uint8_t *packed_actiondata)
                                      
{
    uint16_t dest_start_bit;

    dest_start_bit = 0;
    (void)dest_start_bit;

    switch(actiondata->actionid) {
        case C_IDX_UPDATE_TTL_ID:
            if ((dest_start_bit + 8)
                >= P4PD_MAX_ACTION_DATA_LEN) {
                assert(0);
            }
            p4pd_copy_actiondata_be_order(packed_actiondata,
                           dest_start_bit,
                           (uint8_t*)&(actiondata->C_idx_action_u.\
                                C_idx_update_ttl.ttl_val),
                           0, /* Start bit in source */
                           8);
            dest_start_bit += 8;
        break;
    }
    return (8);
}




/* Install entry into P4-table 'C_idx'
 * at location 'index'
 *
 * Arguments: 
 *  IN  : uint32_t tableid                  : Table Id that identifies
 *                                            P4 table. This id is obtained
 *                                            from p4pd_table_id_enum.
 *  IN  : uint32_t index                    : Direct/Indexed table location where
 *                                            entry is written to.
 *
 *  IN  : C_idx_actions_un *actiondata   : Action data associated with 
 *                                            the key.
 * 
 * Return Value: 
 *  pd_error_t                              : P4PD_SUCCESS / P4PD_FAIL
 */
static p4pd_error_t C_idx_entry_write(uint32_t tableid,
                                         uint32_t index, 
                                         C_idx_actiondata *actiondata)
{
    uint8_t  action_pc;
    // For entry update performance reasons 
    // avoid mem allocate/free. Memory for action data and 
    // table entry memory line is allocated on stack.
    uint8_t  packed_actiondata[P4PD_MAX_ACTION_DATA_LEN] = {0};
    uint8_t  hwentry[P4PD_MAX_MATCHKEY_LEN + P4PD_MAX_ACTION_DATA_LEN] = {0};
    uint16_t entry_size, actiondatalen;


    action_pc = capri_get_action_pc(tableid, actiondata->actionid);

    actiondatalen = C_idx_pack_action_data(tableid, actiondata,
                                              packed_actiondata);

    entry_size = p4pd_p4table_entry_prepare(hwentry,
                                            action_pc,
                                            NULL /* Index Table. No MatchKey*/,
                                            0, /* Zero matchkeylen */
                                            packed_actiondata,
                                            actiondatalen);
    capri_table_entry_write(tableid, index, hwentry, entry_size);
    return (P4PD_SUCCESS);
}

static uint32_t C_idx_unpack_action_data(uint32_t tableid,
                                            uint8_t actionid,
                                            uint8_t *packed_actiondata,
                                            C_idx_actiondata *actiondata)
{
    uint16_t src_start_bit;
    uint16_t actiondatalen;
    
    (void)src_start_bit;

    actiondatalen = 0;
    src_start_bit = 0;

    memset(actiondata, 0, sizeof(C_idx_actiondata));
    actiondata->actionid = actionid;

    switch(actiondata->actionid) {
        case C_IDX_UPDATE_TTL_ID:
            if ((src_start_bit + 8)
                >= P4PD_MAX_ACTION_DATA_LEN) {
                assert(0);
            }
            p4pd_copy_multibyte(
                           (uint8_t*)&(actiondata->C_idx_action_u.\
                                C_idx_update_ttl.ttl_val),
                           0, /* start bit in action-data destination field */
                           packed_actiondata,
                           src_start_bit, /* Start bit in packed actiondata source */
                           8);
            src_start_bit += 8;
            actiondatalen += 8;
        break;
    }
    return (actiondatalen);
}




/* Read hardware entry from P4-table 'C_idx'. Read hw entry is decoded
 * and used to fill up matchkey and actiondata structures.
 *
 * Arguments: 
 *  IN   : uint32_t tableid                 : Table Id that identifies
 *                                            P4 table. This id is obtained
 *                                            from p4pd_table_id_enum.
 *
 *  IN   : uint32_t index                   : Direct/Indexed table index where
 *                                            table entry is read from.
 *  OUT  : C_idx_actions_un* *actiondata : Action data associated with the key.
 * 
 * Return Value: 
 *  pd_error_t                              : P4PD_SUCCESS / P4PD_FAIL
 */
static p4pd_error_t C_idx_entry_read(uint32_t tableid,
                                        uint32_t index, 
                                        C_idx_actiondata* actiondata)
{
    uint8_t  hwentry[P4PD_MAX_MATCHKEY_LEN + P4PD_MAX_ACTION_DATA_LEN] = {0};
    uint16_t hwentry_bit_len;
    uint8_t  actionpc;
    uint8_t *packed_actiondata_after_key;
    uint16_t actiondata_len_after_key, key_bit_len;
    
    (void)packed_actiondata_after_key;
    (void)actiondata_len_after_key;
    (void)key_bit_len;

    capri_table_entry_read(tableid, index, hwentry, &hwentry_bit_len);

    if (!hwentry_bit_len) {
        // Zero len!!
        return (P4PD_SUCCESS);
    }
    p4pd_swizzle_bytes(hwentry, hwentry_bit_len);
    // Split HW entry into 
    //  - actionPC
    //  - Data

    //TODO : How to handle case where actionpc is not in table...
    actionpc = hwentry[0]; // First byte is always action-pc
    actiondata->actionid = capri_get_action_id(tableid, actionpc);
    C_idx_unpack_action_data(tableid, actiondata->actionid, hwentry+1, actiondata);
                       
    return (P4PD_SUCCESS);
}

/* Create hardware entry from P4-table 'C_idx'. 
 * Created entry data encodes both Key and Action data.
 *
 * Arguments: 
 *  IN  : uint32_t tableid                 : Table Id that identifies
 *                                           P4 table. This id is obtained
 *                                           from p4pd_table_id_enum.
 *
 *  IN  : C_idx_actions_un  *actiondata : Action data associated with the key.
 *
 *  OUT : uint8_t *hwentry                 : Action data associated with the key.
 * 
 * Return Value: 
 *  pd_error_t                             : P4PD_SUCCESS / P4PD_FAIL
 */
static p4pd_error_t C_idx_hwentry_create(uint32_t tableid,
                                            C_idx_actiondata* actiondata,
                                            uint8_t *hwentry)
{
    return (P4PD_SUCCESS);
}


/* Decode hardware entry from P4-table 'C_idx'.
 *
 * Arguments: 
 *  IN   : uint32_t tableid                 : Table Id that identifies
 *                                            P4 table. This id is obtained
 *                                            from p4pd_table_id_enum.
 *
 *  OUT  : C_idx_actions_un* *actiondata : Action data associated with the key.
 * 
 * Return Value: 
 *  pd_error_t                              : P4PD_SUCCESS / P4PD_FAIL
 */
static p4pd_error_t C_idx_entry_decode(uint32_t tableid,
                                          uint8_t *hwentry,
                                          uint16_t hwentry_len,
                                          C_idx_actiondata* actiondata)
{
    uint8_t  actionpc;
    
    // Split HW entry into 
    //  - actionPC
    //  - Data

    //TODO : How to handle case where actionpc is not in table...
    actionpc = hwentry[0]; // First byte is always action-pc
    actiondata->actionid = capri_get_action_id(tableid, actionpc);
    C_idx_unpack_action_data(tableid, actiondata->actionid, hwentry+1, actiondata);
                       
    return (P4PD_SUCCESS);
}


/* Query key details for p4-table 'A_hash'
 *
 * Arguments: 
 *  IN  : uint32_t tableid             : Table Id that identifies
 *                                       P4 table. This id is obtained
 *                                       from p4pd_table_id_enum.
 *
 *  OUT : uint32_t *hwkey_len          : hardware key length in bits.
 *  OUT : uint32_t *hwactiondata_len   : Action data length in bits. 
 *
 * Return Value: 
 *  None
 */
static void A_hash_hwentry_query(uint32_t tableid,
                                   uint32_t *hwkey_len, 
                                   uint32_t *hwactiondata_len)
{
    // For hash case always return key length as 512 bits.
    *hwkey_len = 512;

    *hwactiondata_len = 0;

    return;
}

static void hash_A_hash_key_len(uint32_t tableid,
                                  uint16_t *hwkey_len)
{ 
    /* Total bit len of all matchkeys of this table. */
    *hwkey_len = 64;
    return;
}



static uint32_t A_hash_pack_action_data(uint32_t tableid,
                                         A_hash_actiondata *actiondata,
                                         uint8_t *packed_actiondata_before_key, 
                                         uint16_t *actiondata_len_before_key,
                                         uint8_t *packed_actiondata_after_key,
                                         uint16_t *actiondata_len_after_key)
{
    uint16_t dest_start_bit;
    uint16_t bits_before_mat_key;
    uint16_t before_key_adata_start_bit = 0;
    uint16_t after_key_adata_start_bit = 0, source_start_bit = 0;
    bool     copy_before_key = true;
    uint8_t  *packed_action_data;
    uint16_t mat_key_start_bit, mat_key_bit_length, bits_to_copy;

    (void)bits_before_mat_key;
    (void)before_key_adata_start_bit;
    (void)after_key_adata_start_bit;
    (void)copy_before_key;
    (void)packed_action_data;
    (void)mat_key_start_bit;
    (void)mat_key_bit_length;
    (void)bits_to_copy;
    (void)source_start_bit;

    dest_start_bit = 0;
    *actiondata_len_before_key = 0;
    *actiondata_len_after_key = 0;

    switch(actiondata->actionid) {
    }
    return (0);
}



/* Build hardware key function for p4-table 'A_hash'
 * Arguments: 
 *  IN  : uint32_t tableid                  : Table Id that identifies
 *                                            P4 table. This id is obtained
 *                                            from p4pd_table_id_enum.
 *  IN  : A_hash_swkey_t *swkey           : software key to be converted to  hardware key
 *  OUT : uint8_t *hwkey_x                  : hardware key returned as byte stream
 * 
 * Return Value: 
 *  uint32_t                                : Length of hardware key in bytes.
 */
static uint32_t A_hash_hwkey_build(uint32_t tableid,
                                     A_hash_swkey_t *swkey, 
                                     uint8_t *hwkey)
{
    uint32_t key_len = 0;
    /*
     * [ P4Table   Match      Byte Location      ByteLocation ]
     * [ MatchKey  Key        in HW table(HBM    in KeyMaker  ]
     * [ Name,     start      or P4pipe memory),              ]
     * [           bit        before actionpc                 ]
     * [                      is prepended or                 ]
     * [                      before byte                     ]
     * [                      swizzling is done               ]
     * ________________________________________________________
     *
     * [ethernet_dstAddr, 0,  0, 0 ]
     * [ethernet_dstAddr, 8,  1, 1 ]
     * [ethernet_dstAddr, 16,  2, 2 ]
     * [ethernet_dstAddr, 24,  3, 3 ]
     * [ethernet_dstAddr, 32,  4, 4 ]
     * [ethernet_dstAddr, 40,  5, 5 ]
     * [udp_dstPort, 0,  6, 6 ]
     * [udp_dstPort, 8,  7, 7 ]
     *
     * [ P4Table    Match       Bit Location       BitLocation ]
     * [ MatchKey   KeyBit,     in HW table(HBM    in KeyMaker ]
     * [ Name,                  or P4pipe memory),             ]
     * [                        before actionpc                ]
     * [                        is prepended or                ]
     * [                        before byte                    ]
     * [                        swizzling is done              ]
     * _________________________________________________________
     *
     */
    /* Key byte */
    p4pd_copy_into_hwentry(hwkey,
                   (0 - 0) * 8, /* Dest bit position */
                   &(swkey->ethernet_dstAddr[5]),
                   (48 - 0) % 8, /* Start bit in source field */
                   8 /* 8 bits */);
    key_len += 8;
    /* Key byte */
    p4pd_copy_into_hwentry(hwkey,
                   (1 - 0) * 8, /* Dest bit position */
                   &(swkey->ethernet_dstAddr[4]),
                   (48 - 8) % 8, /* Start bit in source field */
                   8 /* 8 bits */);
    key_len += 8;
    /* Key byte */
    p4pd_copy_into_hwentry(hwkey,
                   (2 - 0) * 8, /* Dest bit position */
                   &(swkey->ethernet_dstAddr[3]),
                   (48 - 16) % 8, /* Start bit in source field */
                   8 /* 8 bits */);
    key_len += 8;
    /* Key byte */
    p4pd_copy_into_hwentry(hwkey,
                   (3 - 0) * 8, /* Dest bit position */
                   &(swkey->ethernet_dstAddr[2]),
                   (48 - 24) % 8, /* Start bit in source field */
                   8 /* 8 bits */);
    key_len += 8;
    /* Key byte */
    p4pd_copy_into_hwentry(hwkey,
                   (4 - 0) * 8, /* Dest bit position */
                   &(swkey->ethernet_dstAddr[1]),
                   (48 - 32) % 8, /* Start bit in source field */
                   8 /* 8 bits */);
    key_len += 8;
    /* Key byte */
    p4pd_copy_into_hwentry(hwkey,
                   (5 - 0) * 8, /* Dest bit position */
                   &(swkey->ethernet_dstAddr[0]),
                   (48 - 40) % 8, /* Start bit in source field */
                   8 /* 8 bits */);
    key_len += 8;
    /* Key byte */
    p4pd_copy_into_hwentry(hwkey,
                   (6 - 0) * 8, /* Dest bit position */
                   (uint8_t*)((uint8_t*)&(swkey->udp_dstPort) + 1),
                   (16 - 0) % 8, /* Start bit in source field */
                   8 /* 8 bits */);
    key_len += 8;
    /* Key byte */
    p4pd_copy_into_hwentry(hwkey,
                   (7 - 0) * 8, /* Dest bit position */
                   (uint8_t*)((uint8_t*)&(swkey->udp_dstPort) + 0),
                   (16 - 8) % 8, /* Start bit in source field */
                   8 /* 8 bits */);
    key_len += 8;


    return (key_len);
}

/* Install entry into P4-table 'A_hash'
 * at location 'hashindex'.
 *
 * Arguments: 
 *  IN  : uint32_t tableid                  : Table Id that identifies
 *                                            P4 table. This id is obtained
 *                                            from p4pd_table_id_enum.
 *  IN  : uint32_t hashindex                : Hash index where entry is 
 *                                            installed.
 *
 *  IN  : uint8_t *hwkey                    : Hardware key to be installed
 *                                            into P4-table
 *  IN  : A_hash_actions_un *actiondata   : Action data associated with 
 *                                            the key.
 * 
 * Return Value: 
 *  pd_error_t                              : P4PD_SUCCESS / P4PD_FAIL
 */
static p4pd_error_t A_hash_entry_write(uint32_t tableid,
                                         uint32_t hashindex,
                                         uint8_t *hwkey,
                                         A_hash_actiondata *actiondata)
{
    uint32_t hwactiondata_len, hwkey_len;
    uint8_t  action_pc;
    // For entry update performance reasons
    // avoid mem allocate/free. Memory for action data and
    // table entry memory line is allocated on stack.
    uint8_t  packed_actiondata_after_key[P4PD_MAX_ACTION_DATA_LEN] = {0};
    uint8_t  packed_actiondata_before_key[P4PD_MAX_ACTION_DATA_LEN] = {0};
    uint8_t  hwentry[P4PD_MAX_MATCHKEY_LEN + P4PD_MAX_ACTION_DATA_LEN] = {0};
    uint16_t entry_size, actiondatalen, key_len;
    uint16_t actiondata_len_before_key, actiondata_len_after_key;

    (void)packed_actiondata_before_key;
    (void)actiondata_len_after_key;
    (void)actiondata_len_before_key;
    (void)actiondatalen;
    (void)hwkey_len; // always 512 for hash tables..

    A_hash_hwentry_query(tableid, &hwkey_len, &hwactiondata_len);
    hash_A_hash_key_len(tableid, &key_len);
                          
    action_pc = capri_get_action_pc(tableid, actiondata->actionid);

    /* For hash tables in both pipe and HBM, pack entries such that
     * key bits in memory is aligned with key bits in KM
     */
    // Need to pack actiondata on left of hwkey
    // in case MatchKey is prepended by i1 bytes in KM
    actiondatalen = A_hash_pack_action_data(tableid, actiondata,
                                     packed_actiondata_before_key,
                                     &actiondata_len_before_key,
                                     packed_actiondata_after_key,
                                     &actiondata_len_after_key);

    entry_size = p4pd_hash_table_entry_prepare(hwentry,
                                             action_pc,
                                             0,/*MatchKeyStartByte */
                                             hwkey,
                                             key_len,
                                             packed_actiondata_before_key,
                                             actiondata_len_before_key,
                                             packed_actiondata_after_key,
                                             actiondata_len_after_key);

    capri_table_entry_write(tableid, hashindex, hwentry, entry_size);
    
    return (P4PD_SUCCESS);
}

static uint32_t hash_A_hash_unpack_action_data(uint32_t tableid,
                                                uint8_t  actionid,
                                         uint8_t *packed_actiondata_before_key,
                                         uint16_t actiondata_len_before_key,
                                         uint8_t *packed_actiondata_after_key,
                                         uint16_t actiondata_len_after_key,
                                         A_hash_actiondata *actiondata)
{
    uint16_t src_start_bit;
    uint16_t actiondatalen, bits_from_adata_before_key;
    bool copy_before_key;
    uint8_t *packed_action_data;
    uint16_t bits_to_copy;
    uint16_t dest_start_bit;

    (void)bits_from_adata_before_key;
    (void)*packed_action_data;
    (void)bits_to_copy;
    (void)dest_start_bit;

    actiondatalen = 0;
    src_start_bit = 0; /* TODO: When mat-key start bit is in middle of byte ?? */

    memset(actiondata, 0, sizeof(A_hash_actiondata));

    actiondata->actionid = actionid;

    switch(actiondata->actionid) {
    }
    return (actiondatalen);
}


/* Unbuild hardware key function for p4-table 'A_hash'
 * Arguments:
 *  IN  : uint32_t tableid                  : Table Id that identifies
 *                                            P4 table. This id is obtained
 *                                            from p4pd_table_id_enum.
 *  IN : uint8_t *hw_key                    : hardware key as byte stream
 *  OUT  : A_hash_swkey_t *swkey          : software key built from hardware key
 *
 * Return Value:
 *  uint32_t                                : Length of hardware key in bits.
 */
static uint32_t A_hash_hwkey_unbuild(uint32_t tableid,
                                       uint8_t *hwkey,
                                       uint16_t hwkey_len,
                                       A_hash_swkey_t *swkey)
{
    /*
     * [ P4Table   Match      Byte Location      ByteLocation ]
     * [ MatchKey  Key        in HW table(HBM    in KeyMaker  ]
     * [ Name,     start      or P4pipe memory),              ]
     * [           bit        before actionpc                 ]
     * [                      is prepended or                 ]
     * [                      before byte                     ]
     * [                      swizzling is done               ]
     * ________________________________________________________
     *
     * [ethernet_dstAddr, 0,  0, 0 ]
     * [ethernet_dstAddr, 8,  1, 1 ]
     * [ethernet_dstAddr, 16,  2, 2 ]
     * [ethernet_dstAddr, 24,  3, 3 ]
     * [ethernet_dstAddr, 32,  4, 4 ]
     * [ethernet_dstAddr, 40,  5, 5 ]
     * [udp_dstPort, 0,  6, 6 ]
     * [udp_dstPort, 8,  7, 7 ]
     *
     * [ P4Table    Match       Bit Location       BitLocation ]
     * [ MatchKey   KeyBit,     in HW table(HBM    in KeyMaker ]
     * [ Name,                  or P4pipe memory),             ]
     * [                        before actionpc                ]
     * [                        is prepended or                ]
     * [                        before byte                    ]
     * [                        swizzling is done              ]
     * _________________________________________________________
     *
     */
    memset(swkey, 0, sizeof(A_hash_swkey_t));
    /* Copying one byte from table-key into correct p4fld place */
    p4pd_copy_into_p4field(
                   &(swkey->ethernet_dstAddr[5]),
                   (48 - 0) % 8, /* Start bit in destination */
                   hwkey,
                   (0 - 0) * 8, /* source bit position */
                   8 /* 8 bits */);
    /* Copying one byte from table-key into correct p4fld place */
    p4pd_copy_into_p4field(
                   &(swkey->ethernet_dstAddr[4]),
                   (48 - 8) % 8, /* Start bit in destination */
                   hwkey,
                   (1 - 0) * 8, /* source bit position */
                   8 /* 8 bits */);
    /* Copying one byte from table-key into correct p4fld place */
    p4pd_copy_into_p4field(
                   &(swkey->ethernet_dstAddr[3]),
                   (48 - 16) % 8, /* Start bit in destination */
                   hwkey,
                   (2 - 0) * 8, /* source bit position */
                   8 /* 8 bits */);
    /* Copying one byte from table-key into correct p4fld place */
    p4pd_copy_into_p4field(
                   &(swkey->ethernet_dstAddr[2]),
                   (48 - 24) % 8, /* Start bit in destination */
                   hwkey,
                   (3 - 0) * 8, /* source bit position */
                   8 /* 8 bits */);
    /* Copying one byte from table-key into correct p4fld place */
    p4pd_copy_into_p4field(
                   &(swkey->ethernet_dstAddr[1]),
                   (48 - 32) % 8, /* Start bit in destination */
                   hwkey,
                   (4 - 0) * 8, /* source bit position */
                   8 /* 8 bits */);
    /* Copying one byte from table-key into correct p4fld place */
    p4pd_copy_into_p4field(
                   &(swkey->ethernet_dstAddr[0]),
                   (48 - 40) % 8, /* Start bit in destination */
                   hwkey,
                   (5 - 0) * 8, /* source bit position */
                   8 /* 8 bits */);
    /* Copying one byte from table-key into correct p4fld place */
    p4pd_copy_into_p4field(
                   (uint8_t*)((uint8_t*)&(swkey->udp_dstPort) + 1),
                   (16 - 0) % 8, /* Start bit in destination */
                   hwkey,
                   (6 - 0) * 8, /* source bit position */
                   8 /* 8 bits */);
    /* Copying one byte from table-key into correct p4fld place */
    p4pd_copy_into_p4field(
                   (uint8_t*)((uint8_t*)&(swkey->udp_dstPort) + 0),
                   (16 - 8) % 8, /* Start bit in destination */
                   hwkey,
                   (7 - 0) * 8, /* source bit position */
                   8 /* 8 bits */);


    return (hwkey_len);
}


/* Read hardware entry from P4-table 'A_hash'. Read hw entry is decoded
 * and used to fill up matchkey and actiondata structures.
 *
 * Arguments: 
 *  IN   : uint32_t tableid                 : Table Id that identifies
 *                                            P4 table. This id is obtained
 *                                            from p4pd_table_id_enum.
 *
 *
 *  IN   : uint32_t hashindex               : Hash index where table entry is 
 *                                            read from.
 *  OUT  : A_hash_swkey_t *swkey          : Software key read from P4-table
 *  OUT  : A_hash_actions_un* *actiondata : Action data associated with the key.
 * 
 * Return Value: 
 *  pd_error_t                              : P4PD_SUCCESS / P4PD_FAIL
 */
static p4pd_error_t A_hash_entry_read(uint32_t tableid,
                                        uint32_t hashindex, 
                                        A_hash_swkey_t *swkey, 
                                        A_hash_actiondata *actiondata)
{
    uint8_t  hwentry[P4PD_MAX_MATCHKEY_LEN + P4PD_MAX_ACTION_DATA_LEN] = {0};
    uint16_t hwentry_bit_len;
    uint8_t  actionpc, *packed_actiondata_before_key;
    uint8_t *packed_actiondata_after_key;
    uint16_t actiondata_len_before_key;
    uint16_t actiondata_len_after_key, key_bit_len;

    (void)key_bit_len;
    
    capri_table_entry_read(tableid, hashindex, hwentry, &hwentry_bit_len);

    if (!hwentry_bit_len) {
        // Zero len!!
        return (P4PD_SUCCESS);
    }
    p4pd_swizzle_bytes(hwentry, hwentry_bit_len);
    // Split HW entry into 
    //  - actionPC
    //  - Key 
    //  - Data


    key_bit_len = A_hash_hwkey_unbuild(tableid, hwentry + 0, 
                                         64, swkey);

    //TODO : How to handle case where actionpc is not in table...
    actionpc = hwentry[0]; // First byte is always action-pc
    actiondata->actionid = capri_get_action_id(tableid, actionpc);
    // Since actionpc is not in KM, when unbuilding key into p4fld,
    // pass pointer to hwentry byte stream after action-pc
    packed_actiondata_before_key = (hwentry + 1); // After action-pc
    packed_actiondata_after_key = (hwentry + 0 + (64 >> 3));
    actiondata_len_before_key = -1 * 8; // bit len without actionpc
    actiondata_len_after_key = hwentry_bit_len - (0 * 8 + 64); // bit len
    hash_A_hash_unpack_action_data(tableid,
                                    actiondata->actionid,
                                    packed_actiondata_before_key,
                                    actiondata_len_before_key,
                                    packed_actiondata_after_key,
                                    actiondata_len_after_key,
                                    actiondata);

    return (P4PD_SUCCESS);
}

/* Create hardware entry from P4-table 'A_hash'. 
 * Created entry data encodes both Key and Action data.
 *
 * Arguments: 
 *  IN  : uint32_t tableid                 : Table Id that identifies
 *                                           P4 table. This id is obtained
 *                                           from p4pd_table_id_enum.
 *
 *  IN  : A_hash_swkey_t *swkey          : Software key used in P4-table
 *  IN  : A_hash_actions_un  *actiondata : Action data associated with the key.
 *
 *  OUT : uint8_t *hwentry                 : Action data associated with the key.
 * 
 * Return Value: 
 *  pd_error_t                             : P4PD_SUCCESS / P4PD_FAIL
 */
static p4pd_error_t A_hash_hwentry_create(uint32_t tableid,
                                            A_hash_swkey_t *swkey, 
                                            A_hash_actiondata *actiondata,
                                            uint8_t *hwentry)
{
    return (P4PD_SUCCESS);
}


/* Decode hardware entry from P4-table 'A_hash'.
 *
 * Arguments: 
 *  IN   : uint32_t tableid                 : Table Id that identifies
 *                                            P4 table. This id is obtained
 *                                            from p4pd_table_id_enum.
 *
 *  OUT  : A_hash_swkey_t *swkey          : Software key read from P4-table
 *  OUT  : A_hash_actions_un* *actiondata : Action data associated with the key.
 * 
 * Return Value: 
 *  pd_error_t                              : P4PD_SUCCESS / P4PD_FAIL
 */
static p4pd_error_t A_hash_entry_decode(uint32_t tableid,
                                          uint8_t *hwentry,
                                          uint16_t hwentry_len,
                                          A_hash_swkey_t *swkey, 
                                          A_hash_actiondata *actiondata)
{
    uint8_t  actionpc;
    uint16_t key_bit_len;
    
    // Split HW entry into 
    //  - actionPC
    //  - Key 
    //  - Data

    // Since actionpc is not in KM, when unbuilding key into p4fld,
    // pass pointer to hwentry byte stream after action-pc
    key_bit_len = A_hash_hwkey_unbuild(tableid, hwentry, hwentry_len, swkey);

    //TODO : How to handle case where actionpc is not in table...
    actionpc = hwentry[0]; // First byte is always action-pc
    actiondata->actionid = capri_get_action_id(tableid, actionpc);
    uint8_t *packed_actiondata_before_key;
    uint8_t *packed_actiondata_after_key;
    uint16_t actiondata_len_before_key;
    uint16_t actiondata_len_after_key;
    packed_actiondata_before_key = (hwentry + 1); // After action-pc
    packed_actiondata_after_key = (hwentry + 0);
    actiondata_len_before_key = -1 * 8; // bit len without actionpc
    actiondata_len_after_key = hwentry_len - (0 * 8 + key_bit_len); // bit len
    hash_A_hash_unpack_action_data(tableid,
                                    actiondata->actionid,
                                    packed_actiondata_before_key,
                                    actiondata_len_before_key,
                                    packed_actiondata_after_key,
                                    actiondata_len_after_key,
                                    actiondata);

    return (P4PD_SUCCESS);
}


/* Query key details for p4-table 'B_tcam'
 *
 * Arguments: 
 *  IN  : uint32_t tableid             : Table Id that identifies
 *                                       P4 table. This id is obtained
 *                                       from p4pd_table_id_enum.
 *
 *  OUT : uint32_t *hwkey_len          : hardware key length in bits.
 *  OUT : uint32_t *hwkeymask_len      : hardware key mask length in bits.
 *  OUT : uint32_t *hwactiondata_len   : Action data length in bits. 
 *
 * Return Value: 
 *  None
 */

static void B_tcam_hwentry_query(uint32_t tableid, 
                                   uint32_t *hwkey_len, 
                                   uint32_t *hwkeymask_len,
                                   uint32_t *hwactiondata_len)
{
    *hwkey_len = 162; /* Total bit len of all matchkeys of this table. */
    *hwkeymask_len = 162; /* Total bit len of all matchkeys of this table. */
    *hwkeymask_len += (*hwkeymask_len % P4PD_TCAM_WORD_CHUNK_LEN); 
                                                           /* Tcam memory line is 
                                                            * allocated in chunks of
                                                            * P4PD_TCAM_WORD_CHUNK_LEN
                                                            */
    /* Among all actions of the table, this length is set to maximum
     * action data len so that higher layer can allocate maximum 
     * required memory to handle any action.
     */
    *hwactiondata_len = 0; 
    //*hwactiondata_len += (P4PD_ACTIONPC_BITS); /* Add space for actionpc */
    return;
}



static uint32_t B_tcam_pack_action_data(uint32_t tableid,
                                             B_tcam_actiondata *actiondata,
                                             uint8_t *packed_actiondata)
                                      
{
    uint16_t dest_start_bit;

    dest_start_bit = 0;
    (void)dest_start_bit;

    switch(actiondata->actionid) {
    }
    return (0);
}



/* Build hardware key function for p4-table 'B_tcam'
 * This table implements ternary lookup. The function will also return hw_mask
 * whose length in bytes is same as hw_key. When installing entry into table,
 * both hw_key and hw_mask has to be provided to 
 * p4pd_error_t B_tcam_entry_write() API
 * Arguments: 
 *  IN  : uint32_t tableid                  : Table Id that identifies
 *                                            P4 table. This id is obtained
 *                                            from p4pd_table_id_enum.
 *  IN  : B_tcam_swkey_t *swkey           : software key to be converted to  hardware key
 *  IN  : B_tcam_swkey_mask_t *swkey_mask : software key mask to be 
 *                                            applied for tcam match.
 *  OUT : uint8_t *hwkey_x                  : hardware key returned as byte stream
 *  OUT : uint8_t *hwkey_y                  : hardware key mask returned as byte stream
 * 
 * Return Value: 
 *  uint32_t                                : Length of hardware key in bytes.
 */
static uint32_t B_tcam_hwkey_hwmask_build(uint32_t tableid,
                                            B_tcam_swkey_t *swkey, 
                                            B_tcam_swkey_mask_t *swkey_mask, 
                                            uint8_t *hwkey_x, 
                                            uint8_t *hwkey_y)
{
    uint8_t trit_x, trit_y, k, m;
    uint32_t key_len = 0;
    /*
     * [ P4Table   Match      Byte Location      ByteLocation ]
     * [ MatchKey  Key        in HW table(HBM    in KeyMaker  ]
     * [ Name,     start      or P4pipe memory),              ]
     * [           bit        before actionpc                 ]
     * [                      is prepended or                 ]
     * [                      before byte                     ]
     * [                      swizzling is done               ]
     * ________________________________________________________
     *
     * [ipv6_dstAddr, 0,  0, 0 ]
     * [ipv6_dstAddr, 8,  1, 1 ]
     * [ipv6_dstAddr, 16,  2, 2 ]
     * [ipv6_dstAddr, 24,  3, 3 ]
     * [ipv6_dstAddr, 32,  4, 4 ]
     * [ipv6_dstAddr, 40,  5, 5 ]
     * [ipv6_dstAddr, 48,  6, 6 ]
     * [ipv6_dstAddr, 56,  7, 7 ]
     * [ipv6_dstAddr, 64,  8, 8 ]
     * [ipv6_dstAddr, 72,  9, 9 ]
     * [ipv6_dstAddr, 80,  10, 10 ]
     * [ipv6_dstAddr, 88,  11, 11 ]
     * [ipv6_dstAddr, 96,  12, 12 ]
     * [ipv6_dstAddr, 104,  13, 13 ]
     * [ipv6_dstAddr, 112,  14, 14 ]
     * [ipv6_dstAddr, 120,  15, 15 ]
     * [ipv4_dstAddr, 0,  16, 16 ]
     * [ipv4_dstAddr, 8,  17, 17 ]
     * [ipv4_dstAddr, 16,  18, 18 ]
     * [ipv4_dstAddr, 24,  19, 19 ]
     *
     * [ P4Table    Match       Bit Location       BitLocation ]
     * [ MatchKey   KeyBit,     in HW table(HBM    in KeyMaker ]
     * [ Name,                  or P4pipe memory),             ]
     * [                        before actionpc                ]
     * [                        is prepended or                ]
     * [                        before byte                    ]
     * [                        swizzling is done              ]
     * _________________________________________________________
     *
     *
     * [ ipv4_valid, 0,  160, 160 ]
     * [ ipv6_valid, 0,  161, 161 ]
     */

    /* Key bit */
    k = *((uint8_t*)&(swkey->ipv4_valid) + 0);
    m = *((uint8_t*)&(swkey_mask->ipv4_valid_mask) + 0);
    trit_x = ((k & m) >> 0) & 0x1;
    trit_y = ((~k & m) >>0) & 0x1;

    p4pd_copy_into_hwentry(hwkey_x,
                    ((160 - (160 % 8)) + (7 - (160 % 8))) - (0 * 8), /* Dest bit position */
                   &trit_x,
                   0,
                   1 /* bits to copy */);
    p4pd_copy_into_hwentry(hwkey_y,
                    ((160 - (160 % 8)) + (7 - (160 % 8))) - (0 * 8), /* Dest bit position */
                   &trit_y,
                   0,
                   1 /* bits to copy */);
    key_len += 1;
    /* Key bit */
    k = *((uint8_t*)&(swkey->ipv6_valid) + 0);
    m = *((uint8_t*)&(swkey_mask->ipv6_valid_mask) + 0);
    trit_x = ((k & m) >> 0) & 0x1;
    trit_y = ((~k & m) >>0) & 0x1;

    p4pd_copy_into_hwentry(hwkey_x,
                    ((161 - (161 % 8)) + (7 - (161 % 8))) - (0 * 8), /* Dest bit position */
                   &trit_x,
                   0,
                   1 /* bits to copy */);
    p4pd_copy_into_hwentry(hwkey_y,
                    ((161 - (161 % 8)) + (7 - (161 % 8))) - (0 * 8), /* Dest bit position */
                   &trit_y,
                   0,
                   1 /* bits to copy */);
    key_len += 1;
    /* Key byte */
    k = *((uint8_t*)&(swkey->ipv6_dstAddr[15]));
    m = *((uint8_t*)&(swkey_mask->ipv6_dstAddr_mask[15]));

    trit_x = k & m;
    trit_y = ~k & m;

    p4pd_copy_into_hwentry(hwkey_x,
                   (0 - 0) * 8, /* Dest bit position */
                   &trit_x,
                   0,
                   8 /* 8 bits */);

    p4pd_copy_into_hwentry(hwkey_y,
                   (0 - 0) * 8, /* Dest bit position */
                   &trit_y,
                   0,
                   8 /* 8 bits */);
    key_len += 8;
    /* Key byte */
    k = *((uint8_t*)&(swkey->ipv6_dstAddr[14]));
    m = *((uint8_t*)&(swkey_mask->ipv6_dstAddr_mask[14]));

    trit_x = k & m;
    trit_y = ~k & m;

    p4pd_copy_into_hwentry(hwkey_x,
                   (1 - 0) * 8, /* Dest bit position */
                   &trit_x,
                   0,
                   8 /* 8 bits */);

    p4pd_copy_into_hwentry(hwkey_y,
                   (1 - 0) * 8, /* Dest bit position */
                   &trit_y,
                   0,
                   8 /* 8 bits */);
    key_len += 8;
    /* Key byte */
    k = *((uint8_t*)&(swkey->ipv6_dstAddr[13]));
    m = *((uint8_t*)&(swkey_mask->ipv6_dstAddr_mask[13]));

    trit_x = k & m;
    trit_y = ~k & m;

    p4pd_copy_into_hwentry(hwkey_x,
                   (2 - 0) * 8, /* Dest bit position */
                   &trit_x,
                   0,
                   8 /* 8 bits */);

    p4pd_copy_into_hwentry(hwkey_y,
                   (2 - 0) * 8, /* Dest bit position */
                   &trit_y,
                   0,
                   8 /* 8 bits */);
    key_len += 8;
    /* Key byte */
    k = *((uint8_t*)&(swkey->ipv6_dstAddr[12]));
    m = *((uint8_t*)&(swkey_mask->ipv6_dstAddr_mask[12]));

    trit_x = k & m;
    trit_y = ~k & m;

    p4pd_copy_into_hwentry(hwkey_x,
                   (3 - 0) * 8, /* Dest bit position */
                   &trit_x,
                   0,
                   8 /* 8 bits */);

    p4pd_copy_into_hwentry(hwkey_y,
                   (3 - 0) * 8, /* Dest bit position */
                   &trit_y,
                   0,
                   8 /* 8 bits */);
    key_len += 8;
    /* Key byte */
    k = *((uint8_t*)&(swkey->ipv6_dstAddr[11]));
    m = *((uint8_t*)&(swkey_mask->ipv6_dstAddr_mask[11]));

    trit_x = k & m;
    trit_y = ~k & m;

    p4pd_copy_into_hwentry(hwkey_x,
                   (4 - 0) * 8, /* Dest bit position */
                   &trit_x,
                   0,
                   8 /* 8 bits */);

    p4pd_copy_into_hwentry(hwkey_y,
                   (4 - 0) * 8, /* Dest bit position */
                   &trit_y,
                   0,
                   8 /* 8 bits */);
    key_len += 8;
    /* Key byte */
    k = *((uint8_t*)&(swkey->ipv6_dstAddr[10]));
    m = *((uint8_t*)&(swkey_mask->ipv6_dstAddr_mask[10]));

    trit_x = k & m;
    trit_y = ~k & m;

    p4pd_copy_into_hwentry(hwkey_x,
                   (5 - 0) * 8, /* Dest bit position */
                   &trit_x,
                   0,
                   8 /* 8 bits */);

    p4pd_copy_into_hwentry(hwkey_y,
                   (5 - 0) * 8, /* Dest bit position */
                   &trit_y,
                   0,
                   8 /* 8 bits */);
    key_len += 8;
    /* Key byte */
    k = *((uint8_t*)&(swkey->ipv6_dstAddr[9]));
    m = *((uint8_t*)&(swkey_mask->ipv6_dstAddr_mask[9]));

    trit_x = k & m;
    trit_y = ~k & m;

    p4pd_copy_into_hwentry(hwkey_x,
                   (6 - 0) * 8, /* Dest bit position */
                   &trit_x,
                   0,
                   8 /* 8 bits */);

    p4pd_copy_into_hwentry(hwkey_y,
                   (6 - 0) * 8, /* Dest bit position */
                   &trit_y,
                   0,
                   8 /* 8 bits */);
    key_len += 8;
    /* Key byte */
    k = *((uint8_t*)&(swkey->ipv6_dstAddr[8]));
    m = *((uint8_t*)&(swkey_mask->ipv6_dstAddr_mask[8]));

    trit_x = k & m;
    trit_y = ~k & m;

    p4pd_copy_into_hwentry(hwkey_x,
                   (7 - 0) * 8, /* Dest bit position */
                   &trit_x,
                   0,
                   8 /* 8 bits */);

    p4pd_copy_into_hwentry(hwkey_y,
                   (7 - 0) * 8, /* Dest bit position */
                   &trit_y,
                   0,
                   8 /* 8 bits */);
    key_len += 8;
    /* Key byte */
    k = *((uint8_t*)&(swkey->ipv6_dstAddr[7]));
    m = *((uint8_t*)&(swkey_mask->ipv6_dstAddr_mask[7]));

    trit_x = k & m;
    trit_y = ~k & m;

    p4pd_copy_into_hwentry(hwkey_x,
                   (8 - 0) * 8, /* Dest bit position */
                   &trit_x,
                   0,
                   8 /* 8 bits */);

    p4pd_copy_into_hwentry(hwkey_y,
                   (8 - 0) * 8, /* Dest bit position */
                   &trit_y,
                   0,
                   8 /* 8 bits */);
    key_len += 8;
    /* Key byte */
    k = *((uint8_t*)&(swkey->ipv6_dstAddr[6]));
    m = *((uint8_t*)&(swkey_mask->ipv6_dstAddr_mask[6]));

    trit_x = k & m;
    trit_y = ~k & m;

    p4pd_copy_into_hwentry(hwkey_x,
                   (9 - 0) * 8, /* Dest bit position */
                   &trit_x,
                   0,
                   8 /* 8 bits */);

    p4pd_copy_into_hwentry(hwkey_y,
                   (9 - 0) * 8, /* Dest bit position */
                   &trit_y,
                   0,
                   8 /* 8 bits */);
    key_len += 8;
    /* Key byte */
    k = *((uint8_t*)&(swkey->ipv6_dstAddr[5]));
    m = *((uint8_t*)&(swkey_mask->ipv6_dstAddr_mask[5]));

    trit_x = k & m;
    trit_y = ~k & m;

    p4pd_copy_into_hwentry(hwkey_x,
                   (10 - 0) * 8, /* Dest bit position */
                   &trit_x,
                   0,
                   8 /* 8 bits */);

    p4pd_copy_into_hwentry(hwkey_y,
                   (10 - 0) * 8, /* Dest bit position */
                   &trit_y,
                   0,
                   8 /* 8 bits */);
    key_len += 8;
    /* Key byte */
    k = *((uint8_t*)&(swkey->ipv6_dstAddr[4]));
    m = *((uint8_t*)&(swkey_mask->ipv6_dstAddr_mask[4]));

    trit_x = k & m;
    trit_y = ~k & m;

    p4pd_copy_into_hwentry(hwkey_x,
                   (11 - 0) * 8, /* Dest bit position */
                   &trit_x,
                   0,
                   8 /* 8 bits */);

    p4pd_copy_into_hwentry(hwkey_y,
                   (11 - 0) * 8, /* Dest bit position */
                   &trit_y,
                   0,
                   8 /* 8 bits */);
    key_len += 8;
    /* Key byte */
    k = *((uint8_t*)&(swkey->ipv6_dstAddr[3]));
    m = *((uint8_t*)&(swkey_mask->ipv6_dstAddr_mask[3]));

    trit_x = k & m;
    trit_y = ~k & m;

    p4pd_copy_into_hwentry(hwkey_x,
                   (12 - 0) * 8, /* Dest bit position */
                   &trit_x,
                   0,
                   8 /* 8 bits */);

    p4pd_copy_into_hwentry(hwkey_y,
                   (12 - 0) * 8, /* Dest bit position */
                   &trit_y,
                   0,
                   8 /* 8 bits */);
    key_len += 8;
    /* Key byte */
    k = *((uint8_t*)&(swkey->ipv6_dstAddr[2]));
    m = *((uint8_t*)&(swkey_mask->ipv6_dstAddr_mask[2]));

    trit_x = k & m;
    trit_y = ~k & m;

    p4pd_copy_into_hwentry(hwkey_x,
                   (13 - 0) * 8, /* Dest bit position */
                   &trit_x,
                   0,
                   8 /* 8 bits */);

    p4pd_copy_into_hwentry(hwkey_y,
                   (13 - 0) * 8, /* Dest bit position */
                   &trit_y,
                   0,
                   8 /* 8 bits */);
    key_len += 8;
    /* Key byte */
    k = *((uint8_t*)&(swkey->ipv6_dstAddr[1]));
    m = *((uint8_t*)&(swkey_mask->ipv6_dstAddr_mask[1]));

    trit_x = k & m;
    trit_y = ~k & m;

    p4pd_copy_into_hwentry(hwkey_x,
                   (14 - 0) * 8, /* Dest bit position */
                   &trit_x,
                   0,
                   8 /* 8 bits */);

    p4pd_copy_into_hwentry(hwkey_y,
                   (14 - 0) * 8, /* Dest bit position */
                   &trit_y,
                   0,
                   8 /* 8 bits */);
    key_len += 8;
    /* Key byte */
    k = *((uint8_t*)&(swkey->ipv6_dstAddr[0]));
    m = *((uint8_t*)&(swkey_mask->ipv6_dstAddr_mask[0]));

    trit_x = k & m;
    trit_y = ~k & m;

    p4pd_copy_into_hwentry(hwkey_x,
                   (15 - 0) * 8, /* Dest bit position */
                   &trit_x,
                   0,
                   8 /* 8 bits */);

    p4pd_copy_into_hwentry(hwkey_y,
                   (15 - 0) * 8, /* Dest bit position */
                   &trit_y,
                   0,
                   8 /* 8 bits */);
    key_len += 8;
    /* Key byte */
    k = *((uint8_t*)&(swkey->ipv4_dstAddr) + 3);
    m = *((uint8_t*)&(swkey_mask->ipv4_dstAddr_mask) + 3);


    trit_x = k & m;
    trit_y = ~k & m;

    p4pd_copy_into_hwentry(hwkey_x,
                   (16 - 0) * 8, /* Dest bit position */
                   &trit_x,
                   0,
                   8 /* 8 bits */);

    p4pd_copy_into_hwentry(hwkey_y,
                   (16 - 0) * 8, /* Dest bit position */
                   &trit_y,
                   0,
                   8 /* 8 bits */);
    key_len += 8;
    /* Key byte */
    k = *((uint8_t*)&(swkey->ipv4_dstAddr) + 2);
    m = *((uint8_t*)&(swkey_mask->ipv4_dstAddr_mask) + 2);


    trit_x = k & m;
    trit_y = ~k & m;

    p4pd_copy_into_hwentry(hwkey_x,
                   (17 - 0) * 8, /* Dest bit position */
                   &trit_x,
                   0,
                   8 /* 8 bits */);

    p4pd_copy_into_hwentry(hwkey_y,
                   (17 - 0) * 8, /* Dest bit position */
                   &trit_y,
                   0,
                   8 /* 8 bits */);
    key_len += 8;
    /* Key byte */
    k = *((uint8_t*)&(swkey->ipv4_dstAddr) + 1);
    m = *((uint8_t*)&(swkey_mask->ipv4_dstAddr_mask) + 1);


    trit_x = k & m;
    trit_y = ~k & m;

    p4pd_copy_into_hwentry(hwkey_x,
                   (18 - 0) * 8, /* Dest bit position */
                   &trit_x,
                   0,
                   8 /* 8 bits */);

    p4pd_copy_into_hwentry(hwkey_y,
                   (18 - 0) * 8, /* Dest bit position */
                   &trit_y,
                   0,
                   8 /* 8 bits */);
    key_len += 8;
    /* Key byte */
    k = *((uint8_t*)&(swkey->ipv4_dstAddr) + 0);
    m = *((uint8_t*)&(swkey_mask->ipv4_dstAddr_mask) + 0);


    trit_x = k & m;
    trit_y = ~k & m;

    p4pd_copy_into_hwentry(hwkey_x,
                   (19 - 0) * 8, /* Dest bit position */
                   &trit_x,
                   0,
                   8 /* 8 bits */);

    p4pd_copy_into_hwentry(hwkey_y,
                   (19 - 0) * 8, /* Dest bit position */
                   &trit_y,
                   0,
                   8 /* 8 bits */);
    key_len += 8;





    return (key_len);
}

/* Install entry into P4-table 'B_tcam'
 * at location 'index'
 *
 * Arguments: 
 *  IN  : uint32_t tableid                  : Table Id that identifies
 *                                            P4 table. This id is obtained
 *                                            from p4pd_table_id_enum.
 *
 *  IN  : uint32_t index                    : TCAM table index where entry is 
 *                                            installed.
 *                                            Caller of the API is expected to
 *                                            provide this index based on
 *                                            placement decision made for the key.
 *
 *  IN  : uint8_t *hwkey                    : Hardware key to be installed
 *                                            into P4-table
 *  IN  : uint8_t *hwkey_y                  : Trit match mask used in Tcam 
 *                                            to represent key.
 *  IN  : B_tcam_actions_un *actiondata   : Action data associated with 
 *                                            the key.
 * 
 * Return Value: 
 *  pd_error_t                              : P4PD_SUCCESS / P4PD_FAIL
 */
static p4pd_error_t B_tcam_entry_write(uint32_t tableid,
                                         uint32_t index,
                                         uint8_t *hwkey, 
                                         uint8_t *hwkey_y,
                                         B_tcam_actiondata *actiondata)
{
    uint8_t  packed_actiondata[P4PD_MAX_ACTION_DATA_LEN] = {0};
    uint8_t  sram_hwentry[P4PD_MAX_MATCHKEY_LEN + P4PD_MAX_ACTION_DATA_LEN] = {0};
    uint32_t hwkey_len, hwkeymask_len, actiondatalen;
    uint16_t action_pc, entry_size;

    action_pc = capri_get_action_pc(tableid, actiondata->actionid);
    actiondatalen = B_tcam_pack_action_data(tableid, actiondata, 
                                                packed_actiondata);
    entry_size = p4pd_p4table_entry_prepare(sram_hwentry, 
                                            action_pc, 
                                            NULL /* No MatchKey */, 
                                            0, /* Zero matchkeylen */
                                            packed_actiondata,
                                            actiondatalen);
    capri_table_entry_write(tableid, index, sram_hwentry, entry_size);

    // Install Key in TCAM
    B_tcam_hwentry_query(tableid, &hwkey_len, &hwkeymask_len, &actiondatalen);

    // Swizzle Key installed in TCAM before writing to TCAM memory
    // because TCAM entry is not built using p4pd_p4table_entry_prepare
    // function where bytes are swizzled.
    p4pd_swizzle_bytes(hwkey, hwkey_len);
    p4pd_swizzle_bytes(hwkey_y, hwkeymask_len);

    int pad = 0;
    if (hwkey_len % 16) {
        pad =  16 - (hwkey_len % 16);
    }

    capri_tcam_table_entry_write(tableid, index, hwkey, hwkey_y, hwkey_len + pad);

    return (P4PD_SUCCESS);
}

static uint32_t B_tcam_unpack_action_data(uint32_t tableid,
                                            uint8_t actionid,
                                            uint8_t *packed_actiondata,
                                            B_tcam_actiondata *actiondata)
{
    uint16_t src_start_bit;
    uint16_t actiondatalen;
    
    (void)src_start_bit;

    actiondatalen = 0;
    src_start_bit = 0;

    memset(actiondata, 0, sizeof(B_tcam_actiondata));
    actiondata->actionid = actionid;

    switch(actiondata->actionid) {
    }
    return (actiondatalen);
}


/* Unbuild hardware key function for p4-table 'B_tcam'
 * Arguments:
 *  IN  : uint32_t tableid                  : Table Id that identifies
 *                                            P4 table. This id is obtained
 *                                            from p4pd_table_id_enum.
 *  IN : uint8_t *hw_key                    : hardware key as byte stream
 *  IN : uint8_t *hw_key_mask               : hardware key mask as byte stream
 *  OUT  : B_tcam_swkey_t *swkey          : software key built from hardware key
 *  OUT  : B_tcam_swkey_mask_t *swkey_mask: software key mask built from
 *                                            tcam match key.
 *
 * Return Value:
 *  uint32_t                                : Length of hardware key in bits.
 */
static uint32_t B_tcam_hwkey_hwmask_unbuild(uint32_t tableid,
                                              uint8_t *hw_key,
                                              uint8_t *hw_key_mask,
                                              uint16_t hwkey_len,
                                              B_tcam_swkey_t *swkey,
                                              B_tcam_swkey_mask_t *swkey_mask)
{
    uint8_t trit_x, trit_y, k, m;
    /*
     * [ P4Table   Match      Byte Location      ByteLocation ]
     * [ MatchKey  Key        in HW table(HBM    in KeyMaker  ]
     * [ Name,     start      or P4pipe memory),              ]
     * [           bit        before actionpc                 ]
     * [                      is prepended or                 ]
     * [                      before byte                     ]
     * [                      swizzling is done               ]
     * ________________________________________________________
     *
     * [ipv6_dstAddr, 0,  0, 0 ]
     * [ipv6_dstAddr, 8,  1, 1 ]
     * [ipv6_dstAddr, 16,  2, 2 ]
     * [ipv6_dstAddr, 24,  3, 3 ]
     * [ipv6_dstAddr, 32,  4, 4 ]
     * [ipv6_dstAddr, 40,  5, 5 ]
     * [ipv6_dstAddr, 48,  6, 6 ]
     * [ipv6_dstAddr, 56,  7, 7 ]
     * [ipv6_dstAddr, 64,  8, 8 ]
     * [ipv6_dstAddr, 72,  9, 9 ]
     * [ipv6_dstAddr, 80,  10, 10 ]
     * [ipv6_dstAddr, 88,  11, 11 ]
     * [ipv6_dstAddr, 96,  12, 12 ]
     * [ipv6_dstAddr, 104,  13, 13 ]
     * [ipv6_dstAddr, 112,  14, 14 ]
     * [ipv6_dstAddr, 120,  15, 15 ]
     * [ipv4_dstAddr, 0,  16, 16 ]
     * [ipv4_dstAddr, 8,  17, 17 ]
     * [ipv4_dstAddr, 16,  18, 18 ]
     * [ipv4_dstAddr, 24,  19, 19 ]
     *
     * [ P4Table    Match       Bit Location       BitLocation ]
     * [ MatchKey   KeyBit,     in HW table(HBM    in KeyMaker ]
     * [ Name,                  or P4pipe memory),             ]
     * [                        before actionpc                ]
     * [                        is prepended or                ]
     * [                        before byte                    ]
     * [                        swizzling is done              ]
     * _________________________________________________________
     *
     *
     * [ ipv4_valid, 0,  160, 160 ]
     * [ ipv6_valid, 0,  161, 161 ]
     */
    memset(swkey, 0, sizeof(B_tcam_swkey_t));
    memset(swkey_mask, 0, sizeof(B_tcam_swkey_mask_t));


    /* Key bit */
    trit_x = 0;
    trit_y = 0;
    p4pd_copy_into_p4field(&trit_x,
                           0,
                           hw_key,
                           ((160 - (160 % 8)) + (7 - (160 % 8))) - (0 * 8), /* Source bit position */
                           1 /* bits to copy */);
    p4pd_copy_into_p4field(&trit_y,
                           0,
                           hw_key_mask,
                           ((160 - (160 % 8)) + (7 - (160 % 8))) - (0 * 8), /* Source bit position */
                           1 /* bits to copy */);
    m = trit_x ^ trit_y;
    k = trit_x & m;
    *((uint8_t*)&(swkey->ipv4_valid) + 0) &= ~(1 << 0);
    *((uint8_t*)&(swkey->ipv4_valid) + 0) |= k << (0);
    *((uint8_t*)&(swkey_mask->ipv4_valid_mask) + 0) &= ~(1 << 0);
    *((uint8_t*)&(swkey_mask->ipv4_valid_mask) + 0) |= m << (0);

    /* Key bit */
    trit_x = 0;
    trit_y = 0;
    p4pd_copy_into_p4field(&trit_x,
                           0,
                           hw_key,
                           ((161 - (161 % 8)) + (7 - (161 % 8))) - (0 * 8), /* Source bit position */
                           1 /* bits to copy */);
    p4pd_copy_into_p4field(&trit_y,
                           0,
                           hw_key_mask,
                           ((161 - (161 % 8)) + (7 - (161 % 8))) - (0 * 8), /* Source bit position */
                           1 /* bits to copy */);
    m = trit_x ^ trit_y;
    k = trit_x & m;
    *((uint8_t*)&(swkey->ipv6_valid) + 0) &= ~(1 << 0);
    *((uint8_t*)&(swkey->ipv6_valid) + 0) |= k << (0);
    *((uint8_t*)&(swkey_mask->ipv6_valid_mask) + 0) &= ~(1 << 0);
    *((uint8_t*)&(swkey_mask->ipv6_valid_mask) + 0) |= m << (0);
    /* Key byte */
    trit_x = 0;
    trit_y = 0;
    p4pd_copy_into_p4field(&trit_x,
                           0,
                           hw_key,
                           (0 - 0) * 8, /* source bit position */
                           8 /* 8 bits */);

    p4pd_copy_into_p4field(&trit_y,
                           0, 
                           hw_key_mask,
                           (0 - 0) * 8, /* source bit position */
                           8 /* 8 bits */);

    m = trit_x ^ trit_y;
    k = trit_x & m;
    *((uint8_t*)&(swkey->ipv6_dstAddr[15])) = k;
    *((uint8_t*)&(swkey_mask->ipv6_dstAddr_mask)[15]) = m;
    /* Key byte */
    trit_x = 0;
    trit_y = 0;
    p4pd_copy_into_p4field(&trit_x,
                           0,
                           hw_key,
                           (1 - 0) * 8, /* source bit position */
                           8 /* 8 bits */);

    p4pd_copy_into_p4field(&trit_y,
                           0, 
                           hw_key_mask,
                           (1 - 0) * 8, /* source bit position */
                           8 /* 8 bits */);

    m = trit_x ^ trit_y;
    k = trit_x & m;
    *((uint8_t*)&(swkey->ipv6_dstAddr[14])) = k;
    *((uint8_t*)&(swkey_mask->ipv6_dstAddr_mask)[14]) = m;
    /* Key byte */
    trit_x = 0;
    trit_y = 0;
    p4pd_copy_into_p4field(&trit_x,
                           0,
                           hw_key,
                           (2 - 0) * 8, /* source bit position */
                           8 /* 8 bits */);

    p4pd_copy_into_p4field(&trit_y,
                           0, 
                           hw_key_mask,
                           (2 - 0) * 8, /* source bit position */
                           8 /* 8 bits */);

    m = trit_x ^ trit_y;
    k = trit_x & m;
    *((uint8_t*)&(swkey->ipv6_dstAddr[13])) = k;
    *((uint8_t*)&(swkey_mask->ipv6_dstAddr_mask)[13]) = m;
    /* Key byte */
    trit_x = 0;
    trit_y = 0;
    p4pd_copy_into_p4field(&trit_x,
                           0,
                           hw_key,
                           (3 - 0) * 8, /* source bit position */
                           8 /* 8 bits */);

    p4pd_copy_into_p4field(&trit_y,
                           0, 
                           hw_key_mask,
                           (3 - 0) * 8, /* source bit position */
                           8 /* 8 bits */);

    m = trit_x ^ trit_y;
    k = trit_x & m;
    *((uint8_t*)&(swkey->ipv6_dstAddr[12])) = k;
    *((uint8_t*)&(swkey_mask->ipv6_dstAddr_mask)[12]) = m;
    /* Key byte */
    trit_x = 0;
    trit_y = 0;
    p4pd_copy_into_p4field(&trit_x,
                           0,
                           hw_key,
                           (4 - 0) * 8, /* source bit position */
                           8 /* 8 bits */);

    p4pd_copy_into_p4field(&trit_y,
                           0, 
                           hw_key_mask,
                           (4 - 0) * 8, /* source bit position */
                           8 /* 8 bits */);

    m = trit_x ^ trit_y;
    k = trit_x & m;
    *((uint8_t*)&(swkey->ipv6_dstAddr[11])) = k;
    *((uint8_t*)&(swkey_mask->ipv6_dstAddr_mask)[11]) = m;
    /* Key byte */
    trit_x = 0;
    trit_y = 0;
    p4pd_copy_into_p4field(&trit_x,
                           0,
                           hw_key,
                           (5 - 0) * 8, /* source bit position */
                           8 /* 8 bits */);

    p4pd_copy_into_p4field(&trit_y,
                           0, 
                           hw_key_mask,
                           (5 - 0) * 8, /* source bit position */
                           8 /* 8 bits */);

    m = trit_x ^ trit_y;
    k = trit_x & m;
    *((uint8_t*)&(swkey->ipv6_dstAddr[10])) = k;
    *((uint8_t*)&(swkey_mask->ipv6_dstAddr_mask)[10]) = m;
    /* Key byte */
    trit_x = 0;
    trit_y = 0;
    p4pd_copy_into_p4field(&trit_x,
                           0,
                           hw_key,
                           (6 - 0) * 8, /* source bit position */
                           8 /* 8 bits */);

    p4pd_copy_into_p4field(&trit_y,
                           0, 
                           hw_key_mask,
                           (6 - 0) * 8, /* source bit position */
                           8 /* 8 bits */);

    m = trit_x ^ trit_y;
    k = trit_x & m;
    *((uint8_t*)&(swkey->ipv6_dstAddr[9])) = k;
    *((uint8_t*)&(swkey_mask->ipv6_dstAddr_mask)[9]) = m;
    /* Key byte */
    trit_x = 0;
    trit_y = 0;
    p4pd_copy_into_p4field(&trit_x,
                           0,
                           hw_key,
                           (7 - 0) * 8, /* source bit position */
                           8 /* 8 bits */);

    p4pd_copy_into_p4field(&trit_y,
                           0, 
                           hw_key_mask,
                           (7 - 0) * 8, /* source bit position */
                           8 /* 8 bits */);

    m = trit_x ^ trit_y;
    k = trit_x & m;
    *((uint8_t*)&(swkey->ipv6_dstAddr[8])) = k;
    *((uint8_t*)&(swkey_mask->ipv6_dstAddr_mask)[8]) = m;
    /* Key byte */
    trit_x = 0;
    trit_y = 0;
    p4pd_copy_into_p4field(&trit_x,
                           0,
                           hw_key,
                           (8 - 0) * 8, /* source bit position */
                           8 /* 8 bits */);

    p4pd_copy_into_p4field(&trit_y,
                           0, 
                           hw_key_mask,
                           (8 - 0) * 8, /* source bit position */
                           8 /* 8 bits */);

    m = trit_x ^ trit_y;
    k = trit_x & m;
    *((uint8_t*)&(swkey->ipv6_dstAddr[7])) = k;
    *((uint8_t*)&(swkey_mask->ipv6_dstAddr_mask)[7]) = m;
    /* Key byte */
    trit_x = 0;
    trit_y = 0;
    p4pd_copy_into_p4field(&trit_x,
                           0,
                           hw_key,
                           (9 - 0) * 8, /* source bit position */
                           8 /* 8 bits */);

    p4pd_copy_into_p4field(&trit_y,
                           0, 
                           hw_key_mask,
                           (9 - 0) * 8, /* source bit position */
                           8 /* 8 bits */);

    m = trit_x ^ trit_y;
    k = trit_x & m;
    *((uint8_t*)&(swkey->ipv6_dstAddr[6])) = k;
    *((uint8_t*)&(swkey_mask->ipv6_dstAddr_mask)[6]) = m;
    /* Key byte */
    trit_x = 0;
    trit_y = 0;
    p4pd_copy_into_p4field(&trit_x,
                           0,
                           hw_key,
                           (10 - 0) * 8, /* source bit position */
                           8 /* 8 bits */);

    p4pd_copy_into_p4field(&trit_y,
                           0, 
                           hw_key_mask,
                           (10 - 0) * 8, /* source bit position */
                           8 /* 8 bits */);

    m = trit_x ^ trit_y;
    k = trit_x & m;
    *((uint8_t*)&(swkey->ipv6_dstAddr[5])) = k;
    *((uint8_t*)&(swkey_mask->ipv6_dstAddr_mask)[5]) = m;
    /* Key byte */
    trit_x = 0;
    trit_y = 0;
    p4pd_copy_into_p4field(&trit_x,
                           0,
                           hw_key,
                           (11 - 0) * 8, /* source bit position */
                           8 /* 8 bits */);

    p4pd_copy_into_p4field(&trit_y,
                           0, 
                           hw_key_mask,
                           (11 - 0) * 8, /* source bit position */
                           8 /* 8 bits */);

    m = trit_x ^ trit_y;
    k = trit_x & m;
    *((uint8_t*)&(swkey->ipv6_dstAddr[4])) = k;
    *((uint8_t*)&(swkey_mask->ipv6_dstAddr_mask)[4]) = m;
    /* Key byte */
    trit_x = 0;
    trit_y = 0;
    p4pd_copy_into_p4field(&trit_x,
                           0,
                           hw_key,
                           (12 - 0) * 8, /* source bit position */
                           8 /* 8 bits */);

    p4pd_copy_into_p4field(&trit_y,
                           0, 
                           hw_key_mask,
                           (12 - 0) * 8, /* source bit position */
                           8 /* 8 bits */);

    m = trit_x ^ trit_y;
    k = trit_x & m;
    *((uint8_t*)&(swkey->ipv6_dstAddr[3])) = k;
    *((uint8_t*)&(swkey_mask->ipv6_dstAddr_mask)[3]) = m;
    /* Key byte */
    trit_x = 0;
    trit_y = 0;
    p4pd_copy_into_p4field(&trit_x,
                           0,
                           hw_key,
                           (13 - 0) * 8, /* source bit position */
                           8 /* 8 bits */);

    p4pd_copy_into_p4field(&trit_y,
                           0, 
                           hw_key_mask,
                           (13 - 0) * 8, /* source bit position */
                           8 /* 8 bits */);

    m = trit_x ^ trit_y;
    k = trit_x & m;
    *((uint8_t*)&(swkey->ipv6_dstAddr[2])) = k;
    *((uint8_t*)&(swkey_mask->ipv6_dstAddr_mask)[2]) = m;
    /* Key byte */
    trit_x = 0;
    trit_y = 0;
    p4pd_copy_into_p4field(&trit_x,
                           0,
                           hw_key,
                           (14 - 0) * 8, /* source bit position */
                           8 /* 8 bits */);

    p4pd_copy_into_p4field(&trit_y,
                           0, 
                           hw_key_mask,
                           (14 - 0) * 8, /* source bit position */
                           8 /* 8 bits */);

    m = trit_x ^ trit_y;
    k = trit_x & m;
    *((uint8_t*)&(swkey->ipv6_dstAddr[1])) = k;
    *((uint8_t*)&(swkey_mask->ipv6_dstAddr_mask)[1]) = m;
    /* Key byte */
    trit_x = 0;
    trit_y = 0;
    p4pd_copy_into_p4field(&trit_x,
                           0,
                           hw_key,
                           (15 - 0) * 8, /* source bit position */
                           8 /* 8 bits */);

    p4pd_copy_into_p4field(&trit_y,
                           0, 
                           hw_key_mask,
                           (15 - 0) * 8, /* source bit position */
                           8 /* 8 bits */);

    m = trit_x ^ trit_y;
    k = trit_x & m;
    *((uint8_t*)&(swkey->ipv6_dstAddr[0])) = k;
    *((uint8_t*)&(swkey_mask->ipv6_dstAddr_mask)[0]) = m;

    /* Key byte */
    trit_x = 0;
    trit_y = 0;
    p4pd_copy_into_p4field(&trit_x,
                           0,
                           hw_key,
                           (16 - 0) * 8, /* source bit position */
                           8 /* 8 bits */);

    p4pd_copy_into_p4field(&trit_y,
                           0, 
                           hw_key_mask,
                           (16 - 0) * 8, /* source bit position */
                           8 /* 8 bits */);

    m = trit_x ^ trit_y;
    k = trit_x & m;
    *((uint8_t*)&(swkey->ipv4_dstAddr) + 3) = k;
    *((uint8_t*)&(swkey_mask->ipv4_dstAddr_mask) + 3) = m;

    /* Key byte */
    trit_x = 0;
    trit_y = 0;
    p4pd_copy_into_p4field(&trit_x,
                           0,
                           hw_key,
                           (17 - 0) * 8, /* source bit position */
                           8 /* 8 bits */);

    p4pd_copy_into_p4field(&trit_y,
                           0, 
                           hw_key_mask,
                           (17 - 0) * 8, /* source bit position */
                           8 /* 8 bits */);

    m = trit_x ^ trit_y;
    k = trit_x & m;
    *((uint8_t*)&(swkey->ipv4_dstAddr) + 2) = k;
    *((uint8_t*)&(swkey_mask->ipv4_dstAddr_mask) + 2) = m;

    /* Key byte */
    trit_x = 0;
    trit_y = 0;
    p4pd_copy_into_p4field(&trit_x,
                           0,
                           hw_key,
                           (18 - 0) * 8, /* source bit position */
                           8 /* 8 bits */);

    p4pd_copy_into_p4field(&trit_y,
                           0, 
                           hw_key_mask,
                           (18 - 0) * 8, /* source bit position */
                           8 /* 8 bits */);

    m = trit_x ^ trit_y;
    k = trit_x & m;
    *((uint8_t*)&(swkey->ipv4_dstAddr) + 1) = k;
    *((uint8_t*)&(swkey_mask->ipv4_dstAddr_mask) + 1) = m;

    /* Key byte */
    trit_x = 0;
    trit_y = 0;
    p4pd_copy_into_p4field(&trit_x,
                           0,
                           hw_key,
                           (19 - 0) * 8, /* source bit position */
                           8 /* 8 bits */);

    p4pd_copy_into_p4field(&trit_y,
                           0, 
                           hw_key_mask,
                           (19 - 0) * 8, /* source bit position */
                           8 /* 8 bits */);

    m = trit_x ^ trit_y;
    k = trit_x & m;
    *((uint8_t*)&(swkey->ipv4_dstAddr) + 0) = k;
    *((uint8_t*)&(swkey_mask->ipv4_dstAddr_mask) + 0) = m;




    return (hwkey_len);
}


/* Read hardware entry from P4-table 'B_tcam'. Read hw entry is decoded
 * and used to fill up matchkey and actiondata structures.
 *
 * Arguments: 
 *  IN   : uint32_t tableid                 : Table Id that identifies
 *                                            P4 table. This id is obtained
 *                                            from p4pd_table_id_enum.
 *
 *
 *  IN   : uint32_t index                   : TCAM table index where entry is 
 *                                            installed. 
 *                                            Caller of the API is expected to
 *                                            provide this index based on
 *                                            placement decision made for the key.
 *  OUT  : B_tcam_swkey_t *swkey          : Software key read from P4-table
 *  OUT  : B_tcam_swkey_mask_t *swkey_mask: Software key read from P4-table
 *  OUT  : B_tcam_actions_un* *actiondata : Action data associated with the key.
 * 
 * Return Value: 
 *  pd_error_t                              : P4PD_SUCCESS / P4PD_FAIL
 */
static p4pd_error_t B_tcam_entry_read(uint32_t tableid,
                                        uint32_t index,
                                        B_tcam_swkey_t *swkey, 
                                        B_tcam_swkey_mask_t *swkey_mask,
                                        B_tcam_actiondata *actiondata)
{
    uint8_t  hwentry_x[P4PD_MAX_MATCHKEY_LEN + P4PD_MAX_ACTION_DATA_LEN] = {0};
    uint8_t  hwentry_y[P4PD_MAX_MATCHKEY_LEN + P4PD_MAX_ACTION_DATA_LEN] = {0};
    uint8_t  hwentry[P4PD_MAX_MATCHKEY_LEN + P4PD_MAX_ACTION_DATA_LEN] = {0};
    uint16_t hwentry_bit_len, i;
    uint8_t  actionpc;

    capri_tcam_table_entry_read(tableid, index, hwentry_x, hwentry_y,
                                &hwentry_bit_len);
    if (!hwentry_bit_len) {
        // Zero len!!
        return (P4PD_FAIL);
    }
    p4pd_swizzle_bytes(hwentry_x, hwentry_bit_len);
    // convert trit to match mask
    // xy
    // 01 - match 0
    // 10 - match 1
    // 11 - illegal
    // 00 - dont care
    for (i = 0; i < hwentry_bit_len / 8; i++) {
        hwentry_y[i] = hwentry_y[i] ^ hwentry_x[i];
    }
    if (hwentry_bit_len % 8) {
        hwentry_y[i] = hwentry_y[i] ^ hwentry_x[i];
    }
    B_tcam_hwkey_hwmask_unbuild(tableid, hwentry_x, hwentry_y, hwentry_bit_len,
                                  swkey, swkey_mask);
    // TODO : Read associated SRAM and populate actiondata
    capri_table_entry_read(tableid, index, hwentry, &hwentry_bit_len);
    if (!hwentry_bit_len) {
        // Zero len!!
        return (P4PD_SUCCESS);
    }
    p4pd_swizzle_bytes(hwentry, hwentry_bit_len);
    // Split HW entry into 
    //  - actionPC
    //  - Data
    //TODO : How to handle case where actionpc is not in table...
    actionpc = hwentry[0]; // First byte is always action-pc
    actiondata->actionid = capri_get_action_id(tableid, actionpc);
    B_tcam_unpack_action_data(tableid, actiondata->actionid, hwentry+1, actiondata);
    return (P4PD_SUCCESS);
}

/* Create hardware entry from P4-table 'B_tcam'. 
 * Created entry data encodes both Key and Action data.
 *
 * Arguments: 
 *  IN  : uint32_t tableid                 : Table Id that identifies
 *                                           P4 table. This id is obtained
 *                                           from p4pd_table_id_enum.
 *
 *  IN  : B_tcam_swkey_t *swkey          : Software key used in P4-table
 *  IN  : B_tcam_swkey_mask_t *swkey_mask: Software key used in P4-table
 *  IN  : B_tcam_actions_un  *actiondata : Action data associated with the key.
 *
 *  OUT : uint8_t *hwentry                 : Action data associated with the key.
 * 
 * Return Value: 
 *  pd_error_t                             : P4PD_SUCCESS / P4PD_FAIL
 */
static p4pd_error_t B_tcam_hwentry_create(uint32_t tableid,
                                            B_tcam_swkey_t *swkey, 
                                            B_tcam_swkey_mask_t *swkey_mask,
                                            B_tcam_actiondata *actiondata,
                                            uint8_t *hwentry)
{
    return (P4PD_SUCCESS);
}


/* Decode hardware entry from P4-table 'B_tcam'.
 *
 * Arguments: 
 *  IN   : uint32_t tableid                 : Table Id that identifies
 *                                            P4 table. This id is obtained
 *                                            from p4pd_table_id_enum.
 *
 *  OUT  : B_tcam_swkey_t *swkey          : Software key read from P4-table
 *  OUT  : B_tcam_swkey_mask_t *swkey_mask: Software key read from P4-table
 *  OUT  : B_tcam_actions_un* *actiondata : Action data associated with the key.
 * 
 * Return Value: 
 *  pd_error_t                              : P4PD_SUCCESS / P4PD_FAIL
 */
static p4pd_error_t B_tcam_key_decode(uint32_t tableid,
                                          uint8_t *hwentry_x,
                                          uint8_t *hwentry_y,
                                          uint16_t hwentry_len,
                                          B_tcam_swkey_t *swkey, 
                                          B_tcam_swkey_mask_t *swkey_mask)
{

    B_tcam_hwkey_hwmask_unbuild(tableid, hwentry_x, hwentry_y, hwentry_len,
                                  swkey, swkey_mask);
    return (P4PD_SUCCESS);
}






/* Query key details for p4-table
 *
 * Arguments: 
 *
 *  IN  : uint32_t tableid             : Table Id that identifies
 *                                       P4 table. This id is obtained
 *                                       from p4pd_table_id_enum.
 *  OUT : uint32_t *hwkey_len          : Hardware key length
 *                                       Returned value is ZERO if
 *                                       tableid identifies Index table.
 *  OUT : uint32_t *hwkeymask_len      : hardware key mask length.
 *                                       Returned value is ZERO if
 *                                       tableid identifies Ternary/TCAM
 *                                       table.
 *  OUT : uint32_t *hwactiondata_len   : Action data length. 
 *
 * Return Value: 
 *  None
 */
void p4pd_hwentry_query(uint32_t tableid, 
                        uint32_t *hwkey_len, 
                        uint32_t *hwkeymask_len, 
                        uint32_t *hwactiondata_len)
{
    switch (tableid) {
        case P4TBL_ID_D_MPU_ONLY: /* p4-table 'D_mpu_only' */
            (void)hwkey_len;
            (void)hwkeymask_len;
            D_mpu_only_hwentry_query(tableid, hwactiondata_len);
        break;

        case P4TBL_ID_C_IDX: /* p4-table 'C_idx' */
            (void)hwkey_len;
            (void)hwkeymask_len;
            C_idx_hwentry_query(tableid, hwactiondata_len);
        break;

        case P4TBL_ID_A_HASH: /* p4-table 'A_hash' */
            (void)hwkeymask_len;
            A_hash_hwentry_query(tableid, hwkey_len, hwactiondata_len);
        break;

        case P4TBL_ID_B_TCAM: /* p4-table 'B_tcam' */
            B_tcam_hwentry_query(tableid, hwkey_len, hwkeymask_len, hwactiondata_len);
        break;

    }
}


/* Build hardware key function for p4-table. The returned hw key
 * is byte stream that cannot be interpreted in meaningful way.
 * This byte stream of key should be used to compute hash 
 * install/write key into hardware table using 
 * p4pd_entry_write() API
 *
 * Arguments: 
 *
 *  IN  : uint32_t tableid      : Table Id that identifies
 *                                P4 table. This id is obtained
 *                                from p4pd_table_id_enum.
 *  IN  : void *swkey           : Software key to be converted to  hardware key
 *                                Can be NULL if tableid identifies 
 *                                table type as Index table.
 *                                A software key structure is generated for every
 *                                p4-table. Refer to p4pd.h for structure details.
 *                                Such Per p4 table key data structure should
 *                                provided as void* swkey.
 *  IN  : void *swkey_mask      : Software keymask to be applied in case of
 *                                ternary match. Can be NULL if tableid identifies 
 *                                table type as Index table or Exact match table.
 *                                A software keymask structure is generated for every
 *                                p4-table. Refer to p4pd.h for structure details.
 *                                Such Per p4 table keymask data structure should
 *                                provided as void* swkey_mask.
 *  OUT : uint8_t *hw_key       : hardware key returned as byte stream.
 *                                In case of TCAM table, this is byte stream of 
 *                                trit bit x
 *  OUT : uint8_t *hw_key_y     : Valid only in case of TCAM table.
 *                                Will be NULL if tableid identifies 
 *                                table type as Index table or Exact match table.
 *                                In case of TCAM table, this is byte stream of 
 *                                trit bit y (Both hw_key and hw_key_y need to be
 *                                written in case of TCAM. When installing entry
 *                                in using p4pd_entry_write(), both hw_key 
 *                                and hw_key_y should be provided.
 * 
 * Return Value 
 *  pd_error_t                  : P4PD_SUCCESS / P4PD_FAIL
 */
p4pd_error_t p4pd_hwkey_hwmask_build(uint32_t   tableid,
                                 void       *swkey, 
                                 void       *swkey_mask, 
                                 uint8_t    *hw_key, 
                                 uint8_t    *hw_key_y)
{

#ifdef P4PD_LOG_TABLE_UPDATES
    char            buffer[2048];
    p4pd_table_ds_decoded_string_get(tableid,
                                     swkey,
                                     swkey_mask,
                                     NULL,
                                     buffer,
                                     sizeof(buffer));
    printf("%s\n", buffer);
#endif
    /*
     * 1. Table match key bits are compared with key bits in Key-Maker
     * 2. The ordering of key bits should match ordering of key bits in Key maker.
     */

    switch (tableid) {
        case P4TBL_ID_D_MPU_ONLY: /* p4-table 'D_mpu_only' */
            return (P4PD_SUCCESS); /* No hardwre key for index based lookup tables. */
        break;

        case P4TBL_ID_C_IDX: /* p4-table 'C_idx' */
            return (P4PD_SUCCESS); /* No hardwre key for index based lookup tables. */
        break;

        case P4TBL_ID_A_HASH: /* p4-table 'A_hash' */
            return (A_hash_hwkey_build(tableid, (A_hash_swkey_t *)swkey, hw_key) > 0 ? P4PD_SUCCESS : P4PD_FAIL);
        break;

        case P4TBL_ID_B_TCAM: /* p4-table 'B_tcam' */
            return (B_tcam_hwkey_hwmask_build(tableid, (B_tcam_swkey_t *)swkey, 
                                                (B_tcam_swkey_mask_t *)swkey_mask,
                                                hw_key, hw_key_y) > 0 ? P4PD_SUCCESS : P4PD_FAIL);
        break;

        default:
            // Invalid tableid
            return (P4PD_FAIL);
        break;
    }
    return (P4PD_FAIL);
}

/* Install entry into P4-table.
 *
 * Arguments: 
 *
 *  IN  : uint32_t tableid       : Table Id that identifies
 *                                 P4 table. This id is obtained
 *                                 from p4pd_table_id_enum.
 *  IN  : uint32_t index         : Table index where entry is installed.
 *                                 Caller of the API is expected to provide
 *                                 this index based on placement decision made.
 *                                 If tableid identifies hash lookup table,
 *                                 then index is hash value computed using key
 *                                 bits. If table id identifies ternary lookup
 *                                 table, then index is priority or order
 *                                 determining relative to other enties.
 *                                 If table is index table, then index value
 *                                 is same as the key used to lookup table.
 *  IN  : uint8_t *hwkey         : Hardware key to be installed into P4-table
 *                                 Can be NULL if table id identifies index
 *                                 based lookup table.
 *  IN  : uint8_t *hwkey_y       : Key match trit bit mask used in ternary matching.
 *                                 This value is obtained by using 
 *                                 p4pd_hwkey_hwmask_build().
 *                                 Can be NULL if table id identifies
 *                                 exact match table (hash based lookup) or
 *                                 when table id identifies index based lookup
 *                                 table.
 *  IN  : void    *actiondata    : Action data associated with the key.
 *                                 Action data structure generated per p4 table.
 *                                 Refer to p4pd.h for structure details
 *                                 Per p4 table action data structure should
 *                                 provided as void* actiondata.
 * 
 * Return Value: 
 *  pd_error_t                              : P4PD_SUCCESS / P4PD_FAIL
 */
p4pd_error_t p4pd_entry_write(uint32_t tableid,
                              uint32_t index,
                              uint8_t *hwkey, 
                              uint8_t *hwkey_y,
                              void    *actiondata)
{
    /* P4 Table can reside in P4pipe or in HBM. Depending on where it resides
     * and depending on if table is lookedup using hash or using index
     * table line format is as described below.
     */
    /* 
     * Case 1: 
     *  Table is looked using hash and Table resides in P4pipe.
     *
     *  +-----------------------------------------+
     *  | Action-PC[8] | Match-Key  | Action-Data |
     *  +-----------------------------------------+
     *
     *  Action-PC   : is dervied from actiondata.actionid
     *  Match-Key   : is byte stream built using B_tcam_hwkey_build()
     *  Action-Data : B_tcam_actiondata* actiondata is
     *                bit packed and installed in HW memory.
     *                Action Field order in Action-data should match
     *                with ASM output that MPU uses to fetch action-data
     */
    /* 
     * Case 2: 
     *  Table is looked using hash and Table resides in HBM.
     *
     *  HBM Hash Table Entry Format has to be aligned with Key-Maker format.
     *  Match Key position in both HBM memory and in KM has to at same position.
     *
     *  when Key maker format is 
     *     +--------------------------------------------------------------------+
     *     | Action-PC[8] (not part  | Zero or more | Match-Key  | Zero or more |
     *     | of KM but should be     | I1 bytes     |            | I2 bytes     |
     *     | allocated so that key   |              |            |              |
     *     | in KM and SRAM can align|              |            |              |
     *     +--------------------------------------------------------------------+
     *
     *  HBM memory line should be
     *     +------------------------------------------------------------------+
     *     | Action-PC[8]            | Zero or more | Match-Key  | Remaining  |
     *     |                         | Action-Data  |            | Action-Data|
     *     |                         | bytes        |            | bytes      |
     *     +------------------------------------------------------------------+
     *  Note: Match-key start in both KM and HBM memory are aligned.
     *
     *  Action-PC   : is dervied from actiondata.actionid
     *  Match-Key   : is byte stream built using B_tcam_hwkey_build()
     *  Action-Data : B_tcam_actiondata* actiondata is
     *                bit packed and installed in HW memory. This Action-Data
     *                can be overlayed in byte positions where I1 / I2 bytes
     *                occupy in KM. HBM memory doesn't (cannot) store I1/I2
     *                Hence to optimize HBM memory width, it makes sense to
     *                reuse I1/I2 byte locations in KM for action data.
     */
    /* 
     * Case 3: 
     *  Table is looked using Index and Table resides in HBM or P4pipe.
     *
     * +----------------------------+
     * | Action-PC[8] | Action-Data |
     * +----------------------------+
     *
     * Action-PC   : is dervied from actiondata.actionid
     * Action-Data : B_tcam_actiondata* actiondata is
     *               bit packed and installed in HW memory.
     *               Action Field order in Action-data should match
     *               with ASM output that MPU uses to fetch action-data
     */

#ifdef P4PD_LOG_TABLE_UPDATES
    char            buffer[2048];
    p4pd_table_ds_decoded_string_get(tableid,
                                     NULL,
                                     NULL,
                                     actiondata,
                                     buffer,
                                     sizeof(buffer));
    printf("%s\n", buffer);
#endif
    switch (tableid) {
        case P4TBL_ID_D_MPU_ONLY: /* p4-table 'D_mpu_only' */
            return (D_mpu_only_entry_write(tableid, index, 
                                         (D_mpu_only_actiondata*)actiondata));
        break;

        case P4TBL_ID_C_IDX: /* p4-table 'C_idx' */
            return (C_idx_entry_write(tableid, index, 
                                         (C_idx_actiondata*)actiondata));
        break;

        case P4TBL_ID_A_HASH: /* p4-table 'A_hash' */
            return (A_hash_entry_write(tableid, index, hwkey, 
                                         (A_hash_actiondata*)actiondata));
        break;

        case P4TBL_ID_B_TCAM: /* p4-table 'B_tcam' */
            return (B_tcam_entry_write(tableid, index,
                                         hwkey, hwkey_y,
                                         (B_tcam_actiondata*)actiondata));
        break;

        default:
            // Invalid tableid
            return (P4PD_FAIL);
        break;
    }
    return (P4PD_SUCCESS);
}

/* Read P4 table hardware entry.
 *
 * Arguments: 
 *
 *  IN  : uint32_t tableid       : Table Id that identifies
 *                                 P4 table. This id is obtained
 *                                 from p4pd_table_id_enum.
 *  IN  : uint32_t index         : Table index where entry is installed.
 *                                 Caller of the API is expected to provide
 *                                 this index based on placement decision made.
 *                                 If tableid identifies hash lookup table,
 *                                 then index is hash value computed using key
 *                                 bits. If table id identifies ternary lookup
 *                                 table, then index is priority or order
 *                                 determining relative to other enties.
 *                                 If table is index table, then index value
 *                                 is same as the key used to lookup table.
 *  OUT : void    *swkey         : Hardware key data read from hardware table is 
 *                                 converted to software key. A software key
 *                                 structure is generated for every p4-table.
 *                                 Refer to p4pd.h for structure details.
 *                                 Can be NULL if table id identifies index
 *                                 based lookup table.
 *  OUT : void    *swkey_mask    : Key match mask used in ternary matching.
 *                                 Can be NULL if table id identifies
 *                                 exact match table (hash based lookup) or
 *                                 when table id identifies index based lookup
 *                                 table.
 *  OUT : void    *actiondata    : Action data associated with the key.
 *                                 Data bits read from hardware are returned
 *                                 action data structure generated per p4 table.
 *                                 Refer to p4pd.h for structure details
 * 
 * Return Value: 
 *  pd_error_t                              : P4PD_SUCCESS / P4PD_FAIL
 */
p4pd_error_t p4pd_entry_read(uint32_t   tableid,
                             uint32_t   index,
                             void       *swkey, 
                             void       *swkey_mask,
                             void       *actiondata)
{
    switch (tableid) {
        case P4TBL_ID_D_MPU_ONLY: /* p4-table 'D_mpu_only' */
            return (D_mpu_only_entry_read(tableid, index, 
                            (D_mpu_only_actiondata*) actiondata));
        break;

        case P4TBL_ID_C_IDX: /* p4-table 'C_idx' */
            return (C_idx_entry_read(tableid, index, 
                            (C_idx_actiondata*) actiondata));
        break;

        case P4TBL_ID_A_HASH: /* p4-table 'A_hash' */
            return (A_hash_entry_read(tableid, index, 
                            (A_hash_swkey_t *)swkey, 
                            (A_hash_actiondata*)actiondata));
        break;

        case P4TBL_ID_B_TCAM: /* p4-table 'B_tcam' */
            return (B_tcam_entry_read(tableid, index,
                            (B_tcam_swkey_t*)swkey, 
                            (B_tcam_swkey_mask_t*)swkey_mask,
                            (B_tcam_actiondata*) actiondata));
        break;

        default:
            // Invalid tableid
            return (P4PD_FAIL);
        break;
    }
    return (P4PD_SUCCESS);
}


/* Create P4 table encoded hardware entry.
 *
 * Arguments: 
 *
 *  IN  : uint32_t tableid       : Table Id that identifies
 *                                 P4 table. This id is obtained
 *                                 from p4pd_table_id_enum.
 *  IN  : void    *swkey         : Hardware key data read from hardware table is 
 *                                 converted to software key. A software key
 *                                 structure is generated for every p4-table.
 *                                 Refer to p4pd.h for structure details.
 *                                 Can be NULL if table id identifies index
 *                                 based lookup table.
 *  IN  : void    *swkey_mask    : Key match mask used in ternary matching.
 *                                 Can be NULL if table id identifies
 *                                 exact match table (hash based lookup) or
 *                                 when table id identifies index based lookup
 *                                 table.
 *  IN  : void    *actiondata    : Action data associated with the key.
 *                                 Action data structure generated per p4 table.
 *                                 Refer to p4pd.h for structure details
 *  OUT : uint8_t *hwentry       : HW entry that would get installed .
 * 
 * Return Value: 
 *  pd_error_t                   : P4PD_SUCCESS / P4PD_FAIL
 */
p4pd_error_t p4pd_entry_create(uint32_t   tableid,
                               void       *swkey, 
                               void       *swkey_mask,
                               void       *actiondata,
                               uint8_t    *hwentry)
{
    switch (tableid) {
        case P4TBL_ID_D_MPU_ONLY: /* p4-table 'D_mpu_only' */
            return (D_mpu_only_hwentry_create(tableid,
                    (D_mpu_only_actiondata*)actiondata, hwentry));
        break;

        case P4TBL_ID_C_IDX: /* p4-table 'C_idx' */
            return (C_idx_hwentry_create(tableid,
                    (C_idx_actiondata*)actiondata, hwentry));
        break;

        case P4TBL_ID_A_HASH: /* p4-table 'A_hash' */
            return (A_hash_hwentry_create(tableid, 
                            (A_hash_swkey_t *)swkey, 
                            (A_hash_actiondata*)actiondata, hwentry));
        break;

        case P4TBL_ID_B_TCAM: /* p4-table 'B_tcam' */
            return (B_tcam_hwentry_create(tableid,
                            (B_tcam_swkey_t*)swkey, 
                            (B_tcam_swkey_mask_t*)swkey_mask,
                            (B_tcam_actiondata*) actiondata, hwentry));
        break;

        default:
            // Invalid tableid
            return (P4PD_FAIL);
        break;
    }
    return (P4PD_SUCCESS);
}

/* Return Log string of decoded P4 table hardware entry.
 *
 * Arguments: 
 *
 *  IN  : uint32_t tableid       : Table Id that identifies
 *                                 P4 table. This id is obtained
 *                                 from p4pd_table_id_enum.
 *  IN  : uint8_t  hwentry       : Table entry bytes as read from device/hardware.
 *  IN  : uint8_t  hwentry_y     : TCAM Table entry trit_y bytes as read from 
 *                                 device/hardware. In non TCAM case, NULL.
 *  IN  : uint16_t hwentry_len   : Table entry length in bits.
 *  IN  : uint16_t buf_len       : Size of buffer into which decoded log
 *                                 string is copied.
 *  OUT:  char*    buffer        : Printable/Loggable bufffer with p4field 
 *                                 name and value.
 * Return Value: 
 *  pd_error_t                   : P4PD_SUCCESS / P4PD_FAIL
 */
p4pd_error_t p4pd_table_entry_decoded_string_get(uint32_t   tableid,
                                                 uint32_t   index,
                                                 uint8_t*   hwentry,
                                                 /* Valid only in case of TCAM;
                                                  * Otherwise can be NULL) 
                                                  */
                                                 uint8_t*   hwentry_y,
                                                 uint16_t   hwentry_len,
                                                 char*      buffer,
                                                 uint16_t   buf_len)
{
    char *buf = buffer;
    int  blen = buf_len, b;

    uint8_t  _hwentry[P4PD_MAX_MATCHKEY_LEN + P4PD_MAX_ACTION_DATA_LEN] = {0};
    uint8_t  _hwentry_y[P4PD_MAX_MATCHKEY_LEN + P4PD_MAX_ACTION_DATA_LEN] = {0};


    // TODO : Fix this.. For now return.

    return (P4PD_SUCCESS);

    memcpy(_hwentry, hwentry, hwentry_len);
    p4pd_swizzle_bytes(_hwentry, hwentry_len);
    if (hwentry_y) {
        memcpy(_hwentry_y, hwentry_y, hwentry_len);
        p4pd_swizzle_bytes(_hwentry_y, hwentry_len);
    }

    memset(buffer, 0, buf_len);
    if (hwentry_y) {
        b = snprintf(buf, blen, "*** Decode HW table entry into Table Key *** \n");
    } else {
        b = snprintf(buf, blen, "*** Decode HW table entry into (Table Key + Actiondata)/Actiondata *** \n");
    }
    buf += b;
    blen -= b;
    if (blen <= 0) {
        return (P4PD_SUCCESS);
    }
    switch (tableid) {
        case P4TBL_ID_D_MPU_ONLY: /* p4-table 'D_mpu_only' */
        {
            b = snprintf(buf, blen, "Table: %s, Index %d\n", "P4TBL_ID_D_MPU_ONLY", index);
            buf += b;
            blen -= b;
            if (blen <= 0) {
                return (P4PD_SUCCESS);
            }
            D_mpu_only_actiondata actiondata;
            D_mpu_only_entry_decode(tableid, _hwentry,  hwentry_len,
                                  &actiondata);
            switch(actiondata.actionid) {
                case D_MPU_ONLY_MPU_ONLY_ACTION_ID:
                {
                    b = snprintf(buf, blen, "Action: %s\n", "B_TCAM_MPU_ONLY_ACTION_ID");
                    buf += b;
                    blen -= b;
                    if (blen <= 0) {
                        return (P4PD_SUCCESS);
                    }
                }
                break;
            }
        }
        break;
        case P4TBL_ID_C_IDX: /* p4-table 'C_idx' */
        {
            b = snprintf(buf, blen, "Table: %s, Index %d\n", "P4TBL_ID_C_IDX", index);
            buf += b;
            blen -= b;
            if (blen <= 0) {
                return (P4PD_SUCCESS);
            }
            C_idx_actiondata actiondata;
            C_idx_entry_decode(tableid, _hwentry,  hwentry_len,
                                  &actiondata);
            switch(actiondata.actionid) {
                case C_IDX_UPDATE_TTL_ID:
                {
                    b = snprintf(buf, blen, "Action: %s\n", "B_TCAM_UPDATE_TTL_ID");
                    buf += b;
                    blen -= b;
                    if (blen <= 0) {
                        return (P4PD_SUCCESS);
                    }
                    b = snprintf(buf, blen, "%s: 0x%x\n", "ttl_val",
                             actiondata.C_idx_action_u.\
                             C_idx_update_ttl.ttl_val);
                    buf += b;
                    blen -= b;
                    if (blen <= 0) {
                        return (P4PD_SUCCESS);
                    }
                }
                break;
            }
        }
        break;
        case P4TBL_ID_A_HASH: /* p4-table 'A_hash' */
        {
            b = snprintf(buf, blen, "Table: %s, Index %d\n", "P4TBL_ID_A_HASH", index);
            buf += b;
            blen -= b;
            if (blen <= 0) {
                return (P4PD_SUCCESS);
            }
            A_hash_swkey_t swkey;
            A_hash_actiondata actiondata;
            A_hash_entry_decode(tableid, _hwentry, hwentry_len, 
                                  &swkey, 
                                  &actiondata);
            b = snprintf(buf, blen, "%s: \n", "ethernet_dstAddr");
            buf += b;
            blen -= b;
            if (blen <= 0) {
                return (P4PD_SUCCESS);
            }
            for (int j = 0; j < 6; j++) {
                b = snprintf(buf, blen, "0x%x : ", swkey.ethernet_dstAddr[j]);
                buf += b;
                blen -= b;
                if (blen <= 0) {
                    return (P4PD_SUCCESS);
                }
            }
            b = snprintf(buf, blen, "\n");
            buf += b;
            blen -= b;
            if (blen <= 0) {
                return (P4PD_SUCCESS);
            }
            b = snprintf(buf, blen, "%s: 0x%x\n", "udp_dstPort", swkey.udp_dstPort);
            buf += b;
            blen -= b;
            if (blen <= 0) {
                return (P4PD_SUCCESS);
            }
            switch(actiondata.actionid) {
                case A_HASH_NOP_ID:
                {
                    b = snprintf(buf, blen, "Action: %s\n", "B_TCAM_NOP_ID");
                    buf += b;
                    blen -= b;
                    if (blen <= 0) {
                        return (P4PD_SUCCESS);
                    }
                }
                break;
            }
        }
        break;
        case P4TBL_ID_B_TCAM: /* p4-table 'B_tcam' */
        {
            b = snprintf(buf, blen, "Table: %s, Index %d\n", "P4TBL_ID_B_TCAM", index);
            buf += b;
            blen -= b;
            if (blen <= 0) {
                return (P4PD_SUCCESS);
            }
            if (hwentry_y) { /* mask is not null. hence decode tcam key else actiondata */
                B_tcam_swkey_t swkey;
                B_tcam_swkey_mask_t swkey_mask;
                B_tcam_key_decode(tableid, _hwentry, 
                                  _hwentry_y, hwentry_len,
                                  &swkey, 
                                  &swkey_mask);
                b = snprintf(buf, blen, "%s: 0x%x\n", "ipv4_valid", swkey.ipv4_valid);
                buf += b;
                blen -= b;
                if (blen <= 0) {
                    return (P4PD_SUCCESS);
                }
                b = snprintf(buf, blen, "%s: 0x%x\n", "ipv4_valid_mask", swkey_mask.ipv4_valid_mask);
                buf += b;
                blen -= b;
                if (blen <= 0) {
                    return (P4PD_SUCCESS);
                }
                b = snprintf(buf, blen, "%s: 0x%x\n", "ipv6_valid", swkey.ipv6_valid);
                buf += b;
                blen -= b;
                if (blen <= 0) {
                    return (P4PD_SUCCESS);
                }
                b = snprintf(buf, blen, "%s: 0x%x\n", "ipv6_valid_mask", swkey_mask.ipv6_valid_mask);
                buf += b;
                blen -= b;
                if (blen <= 0) {
                    return (P4PD_SUCCESS);
                }
                b = snprintf(buf, blen, "%s: \n", "ipv6_dstAddr");
                buf += b;
                blen -= b;
                if (blen <= 0) {
                    return (P4PD_SUCCESS);
                }
                for (int j = 0; j < 16; j++) {
                    b = snprintf(buf, blen, "0x%x : ", swkey.ipv6_dstAddr[j]);
                    buf += b;
                    blen -= b;
                    if (blen <= 0) {
                        return (P4PD_SUCCESS);
                    }
                }
                b = snprintf(buf, blen, "\n");
                buf += b;
                blen -= b;
                if (blen <= 0) {
                    return (P4PD_SUCCESS);
                }
                b = snprintf(buf, blen, "%s: \n", "ipv6_dstAddr_mask");
                buf += b;
                blen -= b;
                if (blen <= 0) {
                    return (P4PD_SUCCESS);
                }
                for (int j = 0; j < 16; j++) {
                    b = snprintf(buf, blen, "0x%x : ", swkey_mask.ipv6_dstAddr_mask[j]);
                    buf += b;
                    blen -= b;
                    if (blen <= 0) {
                        return (P4PD_SUCCESS);
                    }
                }
                b = snprintf(buf, blen, "\n");
                buf += b;
                blen -= b;
                if (blen <= 0) {
                    return (P4PD_SUCCESS);
                }
                b = snprintf(buf, blen, "%s: 0x%x\n", "ipv4_dstAddr", swkey.ipv4_dstAddr);
                buf += b;
                blen -= b;
                if (blen <= 0) {
                    return (P4PD_SUCCESS);
                }
                b = snprintf(buf, blen, "%s: 0x%x\n", "ipv4_dstAddr_mask", swkey_mask.ipv4_dstAddr_mask);
                buf += b;
                blen -= b;
                if (blen <= 0) {
                    return (P4PD_SUCCESS);
                }
            } else { /* Decode actiondata associated with TCAM key */
                b = snprintf(buf, blen, "*** Decode Tcam table Action Data *** \n");
                buf += b;
                blen -= b;
                if (blen <= 0) {
                    return (P4PD_SUCCESS);
                }
                B_tcam_actiondata actiondata;
                actiondata.actionid = capri_get_action_id(tableid, _hwentry[0]);
                B_tcam_unpack_action_data(tableid, actiondata.actionid, _hwentry+1,
                                            &actiondata);
                switch(actiondata.actionid) {
                    case B_TCAM_UPDATE_IPV4_ID:
                    {
                        b = snprintf(buf, blen, "Action: %s\n", "B_TCAM_UPDATE_IPV4_ID");
                        buf += b;
                        blen -= b;
                        if (blen <= 0) {
                            return (P4PD_SUCCESS);
                        }
                    }
                break;
                    case B_TCAM_UPDATE_IPV6_ID:
                    {
                        b = snprintf(buf, blen, "Action: %s\n", "B_TCAM_UPDATE_IPV6_ID");
                        buf += b;
                        blen -= b;
                        if (blen <= 0) {
                            return (P4PD_SUCCESS);
                        }
                    }
                break;
                }
            }
        }
        break;
        default:
            // Invalid tableid
            return (P4PD_FAIL);
        break;
    }
    return (P4PD_SUCCESS);
}

/* Return Log string of decoded P4 table structure (key, actiondata structures).
 *
 * Arguments: 
 *
 *  IN  : uint32_t tableid       : Table Id that identifies
 *                                 P4 table. This id is obtained
 *                                 from p4pd_table_id_enum.
 *  IN  : (void*)  swkey         : Table key structure.
 *  IN  : (void*)  swkey_mask    : Table key mask structure.
 *                                 In non TCAM case, NULL.
 *  IN  : (void*)  actiondata    : Table actiondata
 *  IN  : uint16_t buf_len       : Size of buffer into which decoded log
 *                                 string is copied.
 *  OUT:  char*    buffer        : Printable/Loggable bufffer with p4field 
 *                                 name and value.
 * Return Value: 
 *  pd_error_t                   : P4PD_SUCCESS / P4PD_FAIL
 */
p4pd_error_t p4pd_table_ds_decoded_string_get(uint32_t   tableid,
                                              void*      sw_key,
                                              /* Valid only in case of TCAM;
                                               * Otherwise can be NULL) 
                                               */
                                              void*      sw_key_mask,
                                              void*      action_data,
                                              char*      buffer,
                                              uint16_t   buf_len)
{
    char *buf = buffer;
    int  blen = buf_len, b;

    (void)sw_key;
    (void)sw_key_mask;
    (void)action_data;

    memset(buffer, 0, buf_len);

    if (!sw_key) {
        b = snprintf(buf, blen, "*** Print Table Actiondata before Encoding ***\n");
    } else {
        b = snprintf(buf, blen, "*** Print Table Key before Encoding ***\n");
    }
    buf += b;
    blen -= b;
    if (blen <= 0) {
        return (P4PD_SUCCESS);
    }

    switch (tableid) {
        case P4TBL_ID_D_MPU_ONLY: /* p4-table 'D_mpu_only' */
        {
            b = snprintf(buf, blen, "Table: %s\n", "P4TBL_ID_D_MPU_ONLY");
            buf += b;
            blen -= b;
            if (blen <= 0) {
                return (P4PD_SUCCESS);
            }
            D_mpu_only_actiondata *actiondata = (D_mpu_only_actiondata *)action_data;
            if (!actiondata) {
                break;
            }
            switch(actiondata->actionid) {
                case D_MPU_ONLY_MPU_ONLY_ACTION_ID:
                {
                    b = snprintf(buf, blen, "Action: %s\n", "D_MPU_ONLY_MPU_ONLY_ACTION_ID");
                    buf += b;
                    blen -= b;
                    if (blen <= 0) {
                        return (P4PD_SUCCESS);
                    }
                }
                break;
            }
        }
        break;
        case P4TBL_ID_C_IDX: /* p4-table 'C_idx' */
        {
            b = snprintf(buf, blen, "Table: %s\n", "P4TBL_ID_C_IDX");
            buf += b;
            blen -= b;
            if (blen <= 0) {
                return (P4PD_SUCCESS);
            }
            C_idx_actiondata *actiondata = (C_idx_actiondata *)action_data;
            if (!actiondata) {
                break;
            }
            switch(actiondata->actionid) {
                case C_IDX_UPDATE_TTL_ID:
                {
                    b = snprintf(buf, blen, "Action: %s\n", "C_IDX_UPDATE_TTL_ID");
                    buf += b;
                    blen -= b;
                    if (blen <= 0) {
                        return (P4PD_SUCCESS);
                    }
                    b = snprintf(buf, blen, "%s: 0x%x\n", "ttl_val",
                             actiondata->C_idx_action_u.\
                             C_idx_update_ttl.ttl_val);
                    buf += b;
                    blen -= b;
                    if (blen <= 0) {
                        return (P4PD_SUCCESS);
                    }
                }
                break;
            }
        }
        break;
        case P4TBL_ID_A_HASH: /* p4-table 'A_hash' */
        {
            b = snprintf(buf, blen, "Table: %s\n", "P4TBL_ID_A_HASH");
            buf += b;
            blen -= b;
            if (blen <= 0) {
                return (P4PD_SUCCESS);
            }
            A_hash_swkey_t *swkey = (A_hash_swkey_t *)sw_key;
            A_hash_actiondata *actiondata = (A_hash_actiondata *)action_data;
            if (!swkey) {
                break;
            }
            b = snprintf(buf, blen, "Decode Key:\n");
            buf += b;
            blen -= b;
            if (blen <= 0) {
                return (P4PD_SUCCESS);
            }
            b = snprintf(buf, blen, "%s: \n", "ethernet_dstAddr");
            buf += b;
            blen -= b;
            if (blen <= 0) {
                return (P4PD_SUCCESS);
            }
            for (int j = 0; j < 6; j++) {
                b = snprintf(buf, blen, "0x%x : ", swkey->ethernet_dstAddr[j]);
                buf += b;
                blen -= b;
                if (blen <= 0) {
                    return (P4PD_SUCCESS);
                }
            }
            b = snprintf(buf, blen, "\n");
            buf += b;
            blen -= b;
            if (blen <= 0) {
                return (P4PD_SUCCESS);
            }
            b = snprintf(buf, blen, "%s: 0x%x\n", "udp_dstPort", swkey->udp_dstPort);
            buf += b;
            blen -= b;
            if (blen <= 0) {
                return (P4PD_SUCCESS);
            }
            if (!actiondata) {
                break;
            }
            switch(actiondata->actionid) {
                case A_HASH_NOP_ID:
                {
                    b = snprintf(buf, blen, "Action: %s\n", "B_TCAM_NOP_ID");
                    buf += b;
                    blen -= b;
                    if (blen <= 0) {
                        return (P4PD_SUCCESS);
                    }
                }
                break;
            }
        }
        break;
        case P4TBL_ID_B_TCAM: /* p4-table 'B_tcam' */
        {
            b = snprintf(buf, blen, "Table: %s\n", "P4TBL_ID_B_TCAM");
            buf += b;
            blen -= b;
            if (blen <= 0) {
                return (P4PD_SUCCESS);
            }
            B_tcam_swkey_t *swkey = (B_tcam_swkey_t *)sw_key;
            B_tcam_swkey_mask_t *swkey_mask = (B_tcam_swkey_mask_t *)sw_key_mask;
            if (swkey && swkey_mask) {
                b = snprintf(buf, blen, "Decode Key\n");
                buf += b;
                blen -= b;
                if (blen <= 0) {
                    return (P4PD_SUCCESS);
                }
                b = snprintf(buf, blen, "%s: 0x%x\n", "ipv4_valid", swkey->ipv4_valid);
                buf += b;
                blen -= b;
                if (blen <= 0) {
                    return (P4PD_SUCCESS);
                }
                b = snprintf(buf, blen, "%s: 0x%x\n", "ipv4_valid_mask", swkey_mask->ipv4_valid_mask);
                buf += b;
                blen -= b;
                if (blen <= 0) {
                    return (P4PD_SUCCESS);
                }
                b = snprintf(buf, blen, "%s: 0x%x\n", "ipv6_valid", swkey->ipv6_valid);
                buf += b;
                blen -= b;
                if (blen <= 0) {
                    return (P4PD_SUCCESS);
                }
                b = snprintf(buf, blen, "%s: 0x%x\n", "ipv6_valid_mask", swkey_mask->ipv6_valid_mask);
                buf += b;
                blen -= b;
                if (blen <= 0) {
                    return (P4PD_SUCCESS);
                }
                b = snprintf(buf, blen, "%s: \n", "ipv6_dstAddr");
                buf += b;
                blen -= b;
                if (blen <= 0) {
                    return (P4PD_SUCCESS);
                }
                for (int j = 0; j < 16; j++) {
                    b = snprintf(buf, blen, "0x%x : ", swkey->ipv6_dstAddr[j]);
                    buf += b;
                    blen -= b;
                    if (blen <= 0) {
                        return (P4PD_SUCCESS);
                    }
                }
                b = snprintf(buf, blen, "\n");
                buf += b;
                blen -= b;
                if (blen <= 0) {
                    return (P4PD_SUCCESS);
                }
                b = snprintf(buf, blen, "%s: \n", "ipv6_dstAddr_mask");
                buf += b;
                blen -= b;
                if (blen <= 0) {
                    return (P4PD_SUCCESS);
                }
                for (int j = 0; j < 16; j++) {
                    b = snprintf(buf, blen, "0x%x : ", swkey_mask->ipv6_dstAddr_mask[j]);
                    buf += b;
                    blen -= b;
                    if (blen <= 0) {
                        return (P4PD_SUCCESS);
                    }
                }
                b = snprintf(buf, blen, "\n");
                buf += b;
                blen -= b;
                if (blen <= 0) {
                    return (P4PD_SUCCESS);
                }
                b = snprintf(buf, blen, "%s: 0x%x\n", "ipv4_dstAddr", swkey->ipv4_dstAddr);
                buf += b;
                blen -= b;
                if (blen <= 0) {
                    return (P4PD_SUCCESS);
                }
                b = snprintf(buf, blen, "%s: 0x%x\n", "ipv4_dstAddr_mask", swkey_mask->ipv4_dstAddr_mask);
                buf += b;
                blen -= b;
                if (blen <= 0) {
                    return (P4PD_SUCCESS);
                }
            }
            B_tcam_actiondata *actiondata = (B_tcam_actiondata *)action_data;
            if (!actiondata) {
                break;
            }
            switch(actiondata->actionid) {
                case B_TCAM_UPDATE_IPV4_ID:
                {
                    b = snprintf(buf, blen, "Action: %s\n", "B_TCAM_UPDATE_IPV4_ID");
                    buf += b;
                    blen -= b;
                    if (blen <= 0) {
                        return (P4PD_SUCCESS);
                    }
                }
                break;
                case B_TCAM_UPDATE_IPV6_ID:
                {
                    b = snprintf(buf, blen, "Action: %s\n", "B_TCAM_UPDATE_IPV6_ID");
                    buf += b;
                    blen -= b;
                    if (blen <= 0) {
                        return (P4PD_SUCCESS);
                    }
                }
                break;
            }
        }
        break;
        default:
            // Invalid tableid
            return (P4PD_FAIL);
        break;
    }
    return (P4PD_SUCCESS);
}



#ifdef P4PDGEN_COMPILE
#include <stdio.h>
int
main()
{
    printf ("Testing p4pd.c\n");
    return 0;
}
#endif
