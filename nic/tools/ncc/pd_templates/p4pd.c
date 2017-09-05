//:: import os, pdb
//:: from collections import OrderedDict
//:: pddict = _context['pddict']
//:: if pddict['p4plus']:
//::    p4prog = pddict['p4program'] + '_'
//::    caps_p4prog = '_' + pddict['p4program'].upper() + '_'
//::    prefix = 'p4pd_' + pddict['p4program']
//::    if pddict['p4program'] == 'common_rxdma_actions':
//::        start_table_base = 101
//::    elif pddict['p4program'] == 'common_txdma_actions':
//::	    start_table_base = 201
//::    else:
//::	    start_table_base = 301
//::    #endif
//:: else:
//::    p4prog = ''
//::    caps_p4prog = ''
//::    prefix = 'p4pd'
//::	start_table_base = 1
//:: #endif
//:: #define here any constants needed.
//:: ACTION_PC_LEN = 8 # in bits
/*
 * p4pd.c
 * Pensando Systems
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
#include "${p4prog}p4pd.h"

#define P4PD_MAX_ACTION_DATA_LEN (512)
#define P4PD_MAX_MATCHKEY_LEN    (512) /* When multiple flits can be
                                        * chained, then key max size
                                        * can be more than 512. For
                                        * assuming one flit.
                                        */
#define P4PD_ACTIONPC_BITS       (8)   /* For now assume 8bit actionPC.
                                        * 9bits, change it to 2 bytes.
                                        */
#define P4PD_TCAM_WORD_CHUNK_LEN (16)  /* Tcam word chunk */

char ${prefix}_tbl_names[P4${caps_p4prog}TBL_ID_TBLMAX][P4${caps_p4prog}TBL_NAME_MAX_LEN];
uint16_t ${prefix}_tbl_swkey_size[P4${caps_p4prog}TBL_ID_TBLMAX];
uint16_t ${prefix}_tbl_sw_action_data_size[P4${caps_p4prog}TBL_ID_TBLMAX];

extern int capri_table_entry_write(uint32_t tableid,
                                   uint32_t index,
                                   uint8_t  *hwentry,
                                   uint16_t hwentry_bit_len);
extern int capri_table_entry_read(uint32_t  tableid,
                                  uint32_t  index,
                                  uint8_t   *hwentry,
                                  uint16_t  *hwentry_bit_len);
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


/* This function copies a byte at time or a single bit that goes 
 * into table memory 
 */
static void
p4pd_copy_into_hwentry(uint8_t *dest,
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
     */
    if (num_bits == 8) {
        // copying a byte from source to destination
        bits_in_src_byte1 = (*src >> src_start_bit % 8);
        bits_in_src_byte2 = (*(src + 1)  & ((1 << (src_start_bit % 8)) - 1 ));
        bits_in_src_byte2 = bits_in_src_byte2  << (8 - src_start_bit % 8);
        src_byte = bits_in_src_byte2 | bits_in_src_byte1;
        if (dest_start_bit % 8) {
            assert(0);
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
static void
p4pd_copy_single_bit(uint8_t *dest,
                     uint16_t dest_start_bit,
                     uint8_t *src,
                     uint16_t src_start_bit,
                     uint16_t num_bits)
{
    (void)p4pd_copy_single_bit;
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


static void
p4pd_copy_le_src_to_be_dest(uint8_t *dest,
                            uint16_t dest_start_bit,
                            uint8_t *src,
                            uint16_t src_start_bit,
                            uint16_t num_bits)
{
    (void)p4pd_copy_le_src_to_be_dest;

    if (!num_bits || src == NULL) {
        return;
    }

    for (int k = 0; k < num_bits; k++) {
        uint8_t *_dest = dest + ((dest_start_bit + k) / 8);
        // Read from Msbit in source to lsb
        uint8_t *_src = src + ((src_start_bit + num_bits - 1 - k) / 8);
        p4pd_copy_single_bit(_dest,
                             7 - ((dest_start_bit + k) % 8),
                             _src,
                             (src_start_bit + num_bits - 1 - k) % 8,
                             1);
    }
}

static void
p4pd_copy_be_src_to_be_dest(uint8_t *dest,
                            uint16_t dest_start_bit,
                            uint8_t *src,
                            uint16_t src_start_bit,
                            uint16_t num_bits)
{
    (void)p4pd_copy_be_src_to_be_dest;

    if (!num_bits || src == NULL) {
        return;
    }

    for (int k = 0; k < num_bits; k++) {
        uint8_t *_dest = dest + ((dest_start_bit + k) / 8);
        uint8_t *_src = src + ((src_start_bit + k) / 8);
        p4pd_copy_single_bit(_dest,
                             7 - ((dest_start_bit + k) % 8),
                             _src,
                             7 - ((src_start_bit + k) % 8),
                             1);
    }
}

static void
p4pd_copy_be_src_to_le_dest(uint8_t *dest,
                            uint16_t dest_start_bit,
                            uint8_t *src,
                            uint16_t src_start_bit,
                            uint16_t num_bits)
{
    (void)p4pd_copy_be_src_to_le_dest;

    if (!num_bits || src == NULL) {
        return;
    }

    for (int k = 0; k < num_bits; k++) {
        uint8_t *_src = src + ((src_start_bit + k) / 8);
        // Copy into Msbit in destination
        uint8_t *_dest = dest + ((dest_start_bit + k) / 8);
        p4pd_copy_single_bit(_dest,
                             (dest_start_bit + num_bits - 1 - k) % 8,
                             _src,
                             7 - ((src_start_bit + k) % 8),
                             1);
    }
}

static void
p4pd_copy_be_adata_to_le_dest(uint8_t *dest,
                            uint16_t dest_start_bit,
                            uint8_t *src,
                            uint16_t src_start_bit,
                            uint16_t num_bits)
{
    (void)p4pd_copy_be_adata_to_le_dest;

    if (!num_bits || src == NULL) {
        return;
    }

    for (int k = 0; k < num_bits; k++) {
        uint8_t *_src = src + ((src_start_bit + k) >> 3);
        uint8_t *_dest = dest + ((dest_start_bit + num_bits - 1 - k) >> 3);
        p4pd_copy_single_bit(_dest,
                             (dest_start_bit + num_bits - 1 - k) & 0x7,
                             _src,
                             7 - ((src_start_bit + k) & 0x7),
                             1);
    }
}

static void
p4pd_copy_byte_aligned_src_and_dest(uint8_t *dest,
                                    uint16_t dest_start_bit,
                                    uint8_t *src,
                                    uint16_t src_start_bit,
                                    uint16_t num_bits)
{
    (void)p4pd_copy_byte_aligned_src_and_dest;

    if (!num_bits || src == NULL) {
        return;
    }
    
    // destination start bit is in bit.. Get byte corresponding to it.
    dest += dest_start_bit / 8;
    src += src_start_bit / 8;

    int to_copy_bits = num_bits;
    while (to_copy_bits >= 8) {
        *dest = *src;
        to_copy_bits -= 8;
        dest++;
        src++;
    }
    // Remaning bits  (less than 8) need to be copied.
    // They need to be copied from MS bit to LSB
    for (int k = 0; k < to_copy_bits; k++) {
        p4pd_copy_single_bit(dest,
                             7 - k,
                             src,
                             7 - k,
                             1);
    }
}


static void
p4pd_swizzle_bytes(uint8_t *hwentry, uint16_t hwentry_bit_len) 
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


static uint32_t
p4pd_hash_table_entry_prepare(uint8_t *hwentry, 
                              uint8_t action_pc, 
                              uint8_t match_key_start_bit,
                              uint8_t *hwkey, 
                              uint16_t keylen,
                              uint8_t *packed_actiondata_before_matchkey,
                              uint16_t actiondata_before_matchkey_len,
                              uint8_t *packed_actiondata_after_matchkey,
                              uint16_t actiondata_after_matchkey_len)
{
    (void)p4pd_hash_table_entry_prepare;

    uint16_t dest_start_bit = 0;

    if (action_pc != 0xff) {
        *(hwentry + (dest_start_bit >> 3)) = action_pc; // ActionPC is a byte
        dest_start_bit += P4PD_ACTIONPC_BITS;
    }

    p4pd_copy_byte_aligned_src_and_dest(hwentry,
                   dest_start_bit,
                   packed_actiondata_before_matchkey,
                   0, /* Starting from 0th bit in source */
                   actiondata_before_matchkey_len);
    dest_start_bit += actiondata_before_matchkey_len;

    //When actiondata doesn't pack all the way where matchkey starts, set dest_start_bit
    dest_start_bit = match_key_start_bit;

    p4pd_copy_be_src_to_be_dest(hwentry,
                   dest_start_bit,
                   hwkey,
                   match_key_start_bit, /* Starting key bit in hwkey*/
                   keylen);
    dest_start_bit += keylen;

    int actiondata_startbit = 0;
    int key_byte_shared_bits = 0;

    p4pd_copy_be_src_to_be_dest(hwentry,
                                dest_start_bit,
                                packed_actiondata_after_matchkey,
                                actiondata_startbit,
                                (actiondata_after_matchkey_len - key_byte_shared_bits));

    dest_start_bit += (actiondata_after_matchkey_len - key_byte_shared_bits);

    // When swizzling bytes, 16b unit is used. Hence increase size.
    if (dest_start_bit % 16) {
        return (dest_start_bit + 16 - (dest_start_bit % 16));
    } else {
        return (dest_start_bit);
    }
}

// Return hw table entry width
static uint32_t
p4pd_p4table_entry_prepare(uint8_t *hwentry, 
                           uint8_t action_pc, 
                           uint8_t *hwkey, 
                           uint16_t keylen,
                           uint8_t *packed_actiondata,
                           uint16_t actiondata_len)
{
    (void)p4pd_p4table_entry_prepare;

    uint16_t dest_start_bit = 0;

    if (action_pc != 0xff) {
        *(hwentry + (dest_start_bit >> 3)) = action_pc; // ActionPC is a byte
        dest_start_bit += P4PD_ACTIONPC_BITS;
    }

    p4pd_copy_byte_aligned_src_and_dest(hwentry,
                   dest_start_bit,
                   hwkey,
                   0, /* Starting from 0th bit in source */
                   keylen);
    dest_start_bit += keylen;

    int actiondata_startbit = 0;
    int key_byte_shared_bits = 0;

    p4pd_copy_be_src_to_be_dest(hwentry,
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



#define P4PD_LOG_TABLE_UPDATES   /* Disable this once no more table 
                                  * writes logging is needed 
                                  */
p4pd_error_t p4pd_table_ds_decoded_string_get(uint32_t   tableid,
                                              void*      sw_key,
                                              /* Valid only in case of TCAM;
                                               * Otherwise can be NULL) 
                                               */
                                              void*      sw_key_mask,
                                              void*      action_data,
                                              char*      buffer,
                                              uint16_t   buf_len);

/* ------ Per table Functions  ------- */



//::     tabledict = OrderedDict() # key=table-name
//::     tableid = start_table_base
//::     for table in pddict['tables']:
//::        tabledict[table] = tableid
//::        tableid += 1
//::        if pddict['tables'][table]['hash_overflow'] and not pddict['tables'][table]['otcam']:
//::            continue
//::        #endif
//::        tbl = table.upper()
//::        keylen = pddict['tables'][table]['keysize']
//::        max_actionfld_len = 0
//::        for action in pddict['tables'][table]['actions']:
//::            (actionname, actionfldlist) = action
//::            actname = actionname.upper()
//::            if not len(actionfldlist):
//::                continue
//::            #endif
//::            actionfldlen = 0
//::            for actionfld in actionfldlist:
//::                actionfldname, actionfldwidth = actionfld
//::                actionfldlen += actionfldwidth
//::            #endfor
//::            if actionfldlen > max_actionfld_len:
//::                max_actionfld_len = actionfldlen
//::            #endif
//::        #endfor

/* Query key details for p4-table '${table}'
 *
 * Arguments: 
 *  IN  : uint32_t tableid             : Table Id that identifies
 *                                       P4 table. This id is obtained
 *                                       from p4pd_table_id_enum.
 *
//::        if pddict['tables'][table]['type'] != 'Index':
 *  OUT : uint32_t *hwkey_len          : hardware key length in bits.
//::        #endif
//::        if pddict['tables'][table]['type'] == 'Ternary':
 *  OUT : uint32_t *hwkeymask_len      : hardware key mask length in bits.
//::        #endif
 *  OUT : uint32_t *hwactiondata_len   : Action data length in bits. 
 *
 * Return Value: 
 *  None
 */
//::        if pddict['tables'][table]['type'] == 'Ternary':
//::            keylen = pddict['tables'][table]['match_key_bit_length']
static void
${table}_hwentry_query(uint32_t tableid, 
                       uint32_t *hwkey_len, 
                       uint32_t *hwkeymask_len,
                       uint32_t *hwactiondata_len)
{
    *hwkey_len = ${keylen}; /* Total bit len of all matchkeys of this table. */
    *hwkeymask_len = ${keylen}; /* Total bit len of all matchkeys of this table. */
    *hwkeymask_len += P4PD_TCAM_WORD_CHUNK_LEN - (*hwkeymask_len % P4PD_TCAM_WORD_CHUNK_LEN); 
    *hwkey_len += P4PD_TCAM_WORD_CHUNK_LEN - (*hwkey_len % P4PD_TCAM_WORD_CHUNK_LEN); 
    /* Among all actions of the table, this length is set to maximum
     * action data len so that higher layer can allocate maximum 
     * required memory to handle any action.
     */
    *hwactiondata_len = ${max_actionfld_len}; 
    return;
}

static void
tcam_${table}_hwkey_len(uint32_t tableid, 
                        uint32_t *hwkey_len, 
                        uint32_t *hwkeymask_len)
{
    *hwkey_len = ${keylen};
    *hwkeymask_len = ${keylen};
    return;
}

//::        elif pddict['tables'][table]['type'] == 'Index' or pddict['tables'][table]['type'] == 'Mpu':
static void
${table}_hwentry_query(uint32_t tableid, 
                       uint32_t* hwactiondata_len)
{
    /* Among all actions of the table, this length is set to maximum
     * action data len so that higher layer can allocate maximum 
     * required memory to handle any action.
     */
    *hwactiondata_len = ${max_actionfld_len}; 
    return;
}
//::        else:
static void
${table}_hwentry_query(uint32_t tableid,
                       uint32_t *hwkey_len, 
                       uint32_t *hwactiondata_len)
{
    // For hash case always return key length as 512 bits.
    *hwkey_len = 512;

    *hwactiondata_len = ${max_actionfld_len};

    return;
}

static void
hash_${table}_key_len(uint32_t tableid,
                      uint16_t *hwkey_len)
{ 
    /* Total bit len of all matchkeys of this table. */
    *hwkey_len = ${keylen};
    return;
}
//::        #endif



//::        if pddict['tables'][table]['type'] != 'Ternary' and pddict['tables'][table]['type'] != 'Index' and pddict['tables'][table]['type'] != 'Mpu':
static uint32_t
${table}_pack_action_data(uint32_t tableid,
                          ${table}_actiondata *actiondata,
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

//::            if len(pddict['tables'][table]['actions']) > 1:
//::                add_action_pc = True
//::            else:
//::                add_action_pc = False
//::            #endif

    switch(actiondata->actionid) {
//::            for action in pddict['tables'][table]['actions']:
//::                (actionname, actionfldlist) = action
//::                actname = actionname.upper()
//::                if not len(actionfldlist):
//::                    continue
//::                #endif
        case ${tbl}_${actname}_ID:
//::                mat_key_start_byte = pddict['tables'][table]['match_key_start_byte']
//::                mat_key_start_bit = pddict['tables'][table]['match_key_start_bit']
//::                mat_key_bit_length = pddict['tables'][table]['match_key_bit_length']
//::                if add_action_pc:
            mat_key_start_bit = ${mat_key_start_bit} - 8 /* 8 bits for action pc */; /* MatchKeyStart with APC before Key */ 
//::                else:
            mat_key_start_bit = ${mat_key_start_bit}; /* MatchKeyStart without APC before Key */ 
//::                #endif
            mat_key_bit_length = ${mat_key_bit_length}; /* MatchKeyLen */
            before_key_adata_start_bit = 0;
            after_key_adata_start_bit = mat_key_start_bit + mat_key_bit_length;    
            copy_before_key = true;
            packed_action_data = packed_actiondata_before_key;
//::                for actionfld in actionfldlist:
//::                    actionfldname, actionfldwidth = actionfld
            if ((*actiondata_len_before_key + *actiondata_len_after_key + ${actionfldwidth})
                >= P4PD_MAX_ACTION_DATA_LEN) {
                assert(0);
            }
            source_start_bit = 0;
            bits_to_copy = ${actionfldwidth}; /* = length of actiondata field */
            if (copy_before_key) {
                if ((before_key_adata_start_bit + 
                    ${actionfldwidth}) > mat_key_start_bit) {
                    /* Copy part of the field before MatchKey and 
                     * part of the key after MatchKey 
                     */
                    bits_before_mat_key = mat_key_start_bit - dest_start_bit;
                    p4pd_copy_le_src_to_be_dest(packed_action_data,
                                   dest_start_bit,
//::                    if actionfldwidth <= 32:
                                   (uint8_t*)&(actiondata->${table}_action_u.\
                                   ${table}_${actionname}.${actionfldname}),
//::                    else:
                                   (uint8_t*)(actiondata->${table}_action_u.\
                                   ${table}_${actionname}.${actionfldname}),
//::                    #endif
                                   ${actionfldwidth} - bits_before_mat_key,
                                   bits_before_mat_key);
                    (*actiondata_len_before_key) += bits_before_mat_key;
                    copy_before_key = false;
                    packed_action_data = packed_actiondata_after_key;
                    dest_start_bit = 0;
                    /* remaining field bits to be copied after end of match key */
                    bits_to_copy = ${actionfldwidth} - bits_before_mat_key;
                    source_start_bit = 0;

                }
            }
            p4pd_copy_le_src_to_be_dest(packed_action_data,
                           dest_start_bit,
//::                    if actionfldwidth <= 32:
                           (uint8_t*)&(actiondata->${table}_action_u.\
                                ${table}_${actionname}.${actionfldname}),
//::                    else:
                           (uint8_t*)(actiondata->${table}_action_u.\
                                ${table}_${actionname}.${actionfldname}),
//::                    #endif
                           source_start_bit,
                           bits_to_copy);
            dest_start_bit += bits_to_copy;
            if (!copy_before_key) {
                after_key_adata_start_bit += bits_to_copy;
                (*actiondata_len_after_key) += bits_to_copy;
            } else {
                before_key_adata_start_bit += bits_to_copy;
                (*actiondata_len_before_key) += bits_to_copy;
            }
        //::        #endfor
        break;
//::            #endfor
    }
//::            # Always return max actiondata len. This is needed when
//::            # an entry's action changes from action1 --> action2,
//::            # if not all action bits are updated, stale action data
//::            # bits will linger in memory and will be fed to MPU 
//::            # (happens action2 datalen is less than action1 datalen)
    return (${max_actionfld_len});
}
//::        else:
static uint32_t
${table}_pack_action_data(uint32_t tableid,
                          ${table}_actiondata *actiondata,
                          uint8_t *packed_actiondata)
                                      
{
    uint16_t dest_start_bit;

    dest_start_bit = 0;
    (void)dest_start_bit;

    switch(actiondata->actionid) {
//::            for action in pddict['tables'][table]['actions']:
//::                (actionname, actionfldlist) = action
//::                actname = actionname.upper()
//::                if not len(actionfldlist):
//::                    continue
//::                #endif
        case ${tbl}_${actname}_ID:
//::                for actionfld in actionfldlist:
//::                    actionfldname, actionfldwidth = actionfld
            if ((dest_start_bit + ${actionfldwidth})
                >= P4PD_MAX_ACTION_DATA_LEN) {
                assert(0);
            }
            p4pd_copy_le_src_to_be_dest(packed_actiondata,
                           dest_start_bit,
//::                    if actionfldwidth <= 32:
                           (uint8_t*)&(actiondata->${table}_action_u.\
                                ${table}_${actionname}.${actionfldname}),
//::                    else:
                           (uint8_t*)(actiondata->${table}_action_u.\
                                ${table}_${actionname}.${actionfldname}),
//::                    #endif
                           0, /* Start bit in source */
                           ${actionfldwidth});
            dest_start_bit += ${actionfldwidth};
//::                #endfor
        break;
//::            #endfor
    }
//::            # Always return max actiondata len. This is needed when
//::            # an entry's action changes from action1 --> action2,
//::            # if not all action bits are updated, stale action data
//::            # bits will linger in memory and will be fed to MPU 
//::            # (happens action2 datalen is less than action1 datalen)
    return (${max_actionfld_len});
}
//::        #endif



//::        if pddict['tables'][table]['type'] != 'Index' and pddict['tables'][table]['type'] != 'Mpu':
/* Build hardware key function for p4-table '${table}'
//::            if pddict['tables'][table]['type'] == 'Ternary':
 * This table implements ternary lookup. The function will also return hw_mask
 * whose length in bytes is same as hw_key. When installing entry into table,
 * both hw_key and hw_mask has to be provided to 
 * p4pd_error_t ${table}_entry_write() API
//::            #endif
 * Arguments: 
 *  IN  : uint32_t tableid                  : Table Id that identifies
 *                                            P4 table. This id is obtained
 *                                            from p4pd_table_id_enum.
 *  IN  : ${table}_swkey_t *swkey           : software key to be converted to  hardware key
//::            if pddict['tables'][table]['type'] == 'Ternary':
 *  IN  : ${table}_swkey_mask_t *swkey_mask : software key mask to be 
 *                                            applied for tcam match.
//::            #endif
 *  OUT : uint8_t *hwkey_x                  : hardware key returned as byte stream
//::            if pddict['tables'][table]['type'] == 'Ternary':
 *  OUT : uint8_t *hwkey_y                  : hardware key mask returned as byte stream
//::            #endif
 * 
 * Return Value: 
 *  uint32_t                                : Length of hardware key in bytes.
 */
//::            if pddict['tables'][table]['type'] == 'Ternary':
static uint32_t
${table}_hwkey_hwmask_build(uint32_t tableid,
                            ${table}_swkey_t *swkey, 
                            ${table}_swkey_mask_t *swkey_mask, 
                            uint8_t *hwkey_x, 
                            uint8_t *hwkey_y)
{
    uint8_t trit_x, trit_y, k, m;
    uint32_t key_len = 0;
//::                # Do not include Action-PC in hwkey. action-pc will be
//::                # prepended when combining key, action-data
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
//::                for fields in pddict['tables'][table]['keys']:
//::                    (p4fldname, p4fldwidth, mask, key_byte_format, key_bit_format) = fields
//::                    if len(key_byte_format):
//::                        for kmbyte, kbit, _, _ in key_byte_format:
//::                            tablebyte = kmbyte
     * [${p4fldname}, ${kbit},  ${tablebyte}, ${kmbyte} ]
//::                        #endfor
//::                    #endif
//::                #endfor
//::                for fld_un_containers in pddict['tables'][table]['fldunion_keys']:
//::                    fldcontainer, fldukeys = fld_un_containers
//::                    for fields in fldukeys:
//::                        (p4fldname, p4fldwidth, mask, key_byte_format, key_bit_format) = fields
//::                        if len(key_byte_format):
//::                            for kmbyte, kbit, _, _ in key_byte_format:
//::                                tablebyte = kmbyte
     * [${p4fldname}, ${kbit},  ${tablebyte}, ${kmbyte}. Sourced from fld union key]
//::                            #endfor
//::                        #endif
//::                    #endfor
//::                #endfor
//::                for hdr_un_containers in pddict['tables'][table]['hdrunion_keys']:
//::                    hdrcontainer, hdrukeys = hdr_un_containers
//::                    for fields in hdrukeys:
//::                        (p4fldname, p4fldwidth, mask, key_byte_format, key_bit_format) = fields
//::                        if len(key_byte_format):
//::                            for kmbyte, kbit, _, _ in key_byte_format:
//::                                tablebyte = kmbyte
     * [${p4fldname}, ${kbit},  ${tablebyte}, ${kmbyte}. Sourced from hdr union key]
//::                            #endfor
//::                        #endif
//::                    #endfor
//::                #endfor
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
//::                for fields in pddict['tables'][table]['keys']:
//::                    (p4fldname, p4fldwidth, mask, key_byte_format, key_bit_format) = fields
//::                    if len(key_bit_format):
//::                        for kmbit, kbit in key_bit_format:
//::                            tablebit = kmbit
     * [ ${p4fldname}, ${kbit},  ${tablebit}, ${kmbit} ]
//::                        #endfor
//::                    #endif
//::                #endfor
//::                for fld_un_containers in pddict['tables'][table]['fldunion_keys']:
//::                    fldcontainer, fldukeys = fld_un_containers
//::                    for fields in fldukeys:
//::                        (p4fldname, p4fldwidth, mask, key_byte_format, key_bit_format) = fields
//::                        if len(key_bit_format):
//::                            for kmbit, kbit in key_bit_format:
//::                                tablebit = kmbit
     * [ ${p4fldname}, ${kbit},  ${tablebit}, ${kmbit}. Sourced from fld union key]
//::                            #endfor
//::                        #endif
//::                    #endfor
//::                #endfor
//::                for hdr_un_containers in pddict['tables'][table]['hdrunion_keys']:
//::                    hdrcontainer, hdrukeys = hdr_un_containers
//::                    for fields in hdrukeys:
//::                        (p4fldname, p4fldwidth, mask, key_byte_format, key_bit_format) = fields
//::                        if len(key_bit_format):
//::                            for kmbit, kbit in key_bit_format:
//::                                tablebit = kmbit
     * [ ${p4fldname}, ${kbit},  ${tablebit}, ${kmbit}. Sourced from hdr union key]
//::                            #endfor
//::                        #endif
//::                    #endfor
//::                #endfor
     */
//::                if len(pddict['tables'][table]['not_my_key_bytes']):
    /*
     * [ P4Table   Match      Byte Location      ByteLocation ]
     * [ MatchKey  Key,       in HW table(HBM    in KeyMaker  ]
     * [ Name,     start      or P4pipe memory),              ]
     * [           bit        before actionpc                 ]
     * [                      is prepended or                 ]
     * [                      before byte                     ]
     * [                      swizzling is done               ]
     * ________________________________________________________
     *
//::                    for kmbyte in pddict['tables'][table]['not_my_key_bytes']:
//::                        tablebyte = kmbyte
     * [XXXXXXX, nnnnnnnn, ${tablebyte}, ${kmbyte} ]
//::                    #endfor
     */
//::                #endif
//::                if len(pddict['tables'][table]['not_my_key_bits']):
    /*
     *
     * [ P4Table   Match      Bit Location       ByteLocation ]
     * [ MatchKey  KeyBit,    in HW table(HBM    in KeyMaker  ]
     * [ Name,                or P4pipe memory),              ]
     * [                      before actionpc                 ]
     * [                      is prepended or                 ]
     * [                      before byte                     ]
     * [                      swizzling is done               ]
     * ________________________________________________________
     *
//::                    for kmbit in pddict['tables'][table]['not_my_key_bits']:
//::                        tablebit = kmbit
     * [XXXXXXX, nnnnnnnn, ${tablebit}, ${kmbit} ]
//::                    #endfor
     */
//::                #endif

//::                mat_key_start_byte = pddict['tables'][table]['match_key_start_byte']
//::                mat_key_start_bit = pddict['tables'][table]['match_key_start_bit']
//::                for fields in pddict['tables'][table]['keys']:
//::                    (p4fldname, p4fldwidth, mask, key_byte_format, key_bit_format) = fields
//::                    if len(key_byte_format):
//::                        for kmbyte, _kbit, width, containerstart in key_byte_format:
//::                            tablebyte = kmbyte
//::                            kbyte = (p4fldwidth - 1 - _kbit) / 8
//::                            kbit = (p4fldwidth - 1 - _kbit) % 8
    /* Key byte */
//::                            if p4fldwidth <= 32:
//::                                if kbit < 7:
    /* P4 field is extended.. a less than byte field is extended to byte */
    k = *((uint8_t*)&(swkey->${p4fldname}) + ${kbyte});
    m = *((uint8_t*)&(swkey_mask->${p4fldname}_mask) + ${kbyte});
    k &= (1 << ${p4fldwidth}) - 1;
    m &= (1 << ${p4fldwidth}) - 1;
//::                                else:
    k = *((uint8_t*)&(swkey->${p4fldname}) + ${kbyte});
    m = *((uint8_t*)&(swkey_mask->${p4fldname}_mask) + ${kbyte});
//::                                #endif

//::                            else:
//::                                if kbit < 7:
    /* P4 field is extended.. a less than byte field is extended to byte */
    k = *((uint8_t*)&(swkey->${p4fldname}[${kbyte}]));
    m = *((uint8_t*)&(swkey_mask->${p4fldname}_mask[${kbyte}]));
    k &= (1 << ${p4fldwidth}) - 1;
    m &= (1 << ${p4fldwidth}) - 1;
//::                                else:
    k = *((uint8_t*)&(swkey->${p4fldname}[${kbyte}]));
    m = *((uint8_t*)&(swkey_mask->${p4fldname}_mask[${kbyte}]));
//::                                #endif
//::                            #endif

    trit_x = k & m;
    trit_y = ~k & m;

    p4pd_copy_into_hwentry(hwkey_x,
                   (${tablebyte} * 8) + ${containerstart} - ${mat_key_start_bit}, /* Dest bit position */
                   &trit_x,
                   0,
                   ${width});

    p4pd_copy_into_hwentry(hwkey_y,
                   (${tablebyte} * 8) + ${containerstart} - ${mat_key_start_bit}, /* Dest bit position */
                   &trit_y,
                   0,
                   ${width});
    key_len += ${width};
//::                        #endfor
//::                    #endif
//::                    if len(key_bit_format):
//::                        for kmbit, _kbit in key_bit_format:
//::                            tablebit = kmbit
//::                            kbyte = (p4fldwidth - 1 - _kbit) / 8
//::                            kbit = (p4fldwidth - 1 - _kbit) % 8
    /* Key bit */
//::                            if p4fldwidth <= 32:
    k = *((uint8_t*)&(swkey->${p4fldname}) + ${kbyte});
    m = *((uint8_t*)&(swkey_mask->${p4fldname}_mask) + ${kbyte});
//::                            else:
    k = *((uint8_t*)&(swkey->${p4fldname}[${kbyte}]));
    m = *((uint8_t*)&(swkey_mask->${p4fldname}_mask[${kbyte}]));
//::                            #endif
    trit_x = ((k & m) >> ${kbit}) & 0x1;
    trit_y = ((~k & m) >>${kbit}) & 0x1;

    p4pd_copy_into_hwentry(hwkey_x,
                    ((${kmbit} - (${kmbit} % 8)) + (7 - (${kmbit} % 8))) - (${mat_key_start_bit}), /* Dest bit position */
                   &trit_x,
                   0,
                   1 /* bits to copy */);
    p4pd_copy_into_hwentry(hwkey_y,
                    ((${kmbit} - (${kmbit} % 8)) + (7 - (${kmbit} % 8))) - (${mat_key_start_bit}), /* Dest bit position */
                   &trit_y,
                   0,
                   1 /* bits to copy */);
    key_len += 1;
//::                        #endfor
//::                    #endif
//::                #endfor

//::                i = 1
//::                for fld_un_containers in pddict['tables'][table]['fldunion_keys']:
//::                    fldcontainer, fldukeys = fld_un_containers
//::                    if len(fldukeys) > 1:
//::                        ustr = table + '_u' + str(i) + '.'
//::                        ustr_mask = table + '_mask_u' + str(i) + '.'
//::                    else:
//::                        ustr=''
//::                        ustr_mask=''
//::                    #endif
//::                    for fields in fldukeys:
//::                        (p4fldname, p4fldwidth, mask, key_byte_format, key_bit_format) = fields
//::                        if len(key_byte_format):
//::                            for kmbyte, _kbit, width, containerstart in key_byte_format:
//::                                kbyte = (p4fldwidth - 1 - _kbit) / 8
//::                                kbit = (p4fldwidth - 1 - _kbit) % 8
//::                                tablebyte = kmbyte
    /* Field Union Key byte */
//::                                if p4fldwidth <= 32:
//::                                    if kbit < 7:
    /* P4 field is extended.. a less than byte field is extended to byte */
    k = *((uint8_t*)&(swkey->${ustr}${p4fldname}) + ${kbyte});
    m = *((uint8_t*)&(swkey_mask->${ustr_mask}${p4fldname}_mask) + ${kbyte});
    k &= (1 << ${p4fldwidth}) - 1;
    m &= (1 << ${p4fldwidth}) - 1;
//::                                    else:
    k = *((uint8_t*)&(swkey->${ustr}${p4fldname}) + ${kbyte});
    m = *((uint8_t*)&(swkey_mask->${ustr_mask}${p4fldname}_mask) + ${kbyte});
//::                                    #endif
//::                                else:
//::                                    if kbit < 7:
    /* P4 field is extended.. a less than byte field is extended to byte */
    k = *((uint8_t*)&(swkey->${ustr}${p4fldname}[${kbyte}]));
    m = *((uint8_t*)&(swkey_mask->${ustr_mask}${p4fldname}_mask[${kbyte}]));
    k &= (1 << ${p4fldwidth}) - 1;
    m &= (1 << ${p4fldwidth}) - 1;
//::                                    else:
    k = *((uint8_t*)&(swkey->${ustr}${p4fldname}[${kbyte}]));
    m = *((uint8_t*)&(swkey_mask->${ustr_mask}${p4fldname}_mask[${kbyte}]));
//::                                    #endif
//::                                #endif

    trit_x = k & m;
    trit_y = ~k & m;
    p4pd_copy_into_hwentry(hwkey_x,
                   (${tablebyte} * 8) + ${containerstart} - ${mat_key_start_bit}, /* Dest bit position */
                   &trit_x,
                   0,
                   ${width});
    p4pd_copy_into_hwentry(hwkey_y,
                   (${tablebyte} * 8) + ${containerstart} - ${mat_key_start_bit}, /* Dest bit position */
                   &trit_y,
                   0,
                   ${width});
    key_len += ${width};
//::                            #endfor
//::                        #endif
//::                        if len(key_bit_format):
//::                            for kmbit, _kbit in key_bit_format:
//::                                tablebit = kmbit
//::                                kbyte = (p4fldwidth - 1 - _kbit) / 8
//::                                kbit = (p4fldwidth - 1 - _kbit) % 8
    /* Field Union Key bit */
//::                                if p4fldwidth <= 32:
    k = *((uint8_t*)&(swkey->${ustr}${p4fldname}) + ${kbyte}) << (${kbit} % 8);
    m = *((uint8_t*)&(swkey_mask->${ustr}${p4fldname}_mask) + ${kbyte}) << (${kbit} % 8);
//::                                else:
    k = *((uint8_t*)&(swkey->${ustr}${p4fldname}[${kbyte}])) << (${kbit} % 8);
    m = *((uint8_t*)&(swkey_mask->${ustr}${p4fldname}_mask[${kbyte}])) << (${kbit} % 8);
//::                                #endif

    trit_x = ((k & m) >> ${kbit}) & 0x1;
    trit_y = ((~k & m) >>${kbit}) & 0x1;
    p4pd_copy_into_hwentry(hwkey_x,
                    ((${kmbit} - (${kmbit} % 8)) + (7 - (${kmbit} % 8)))- (${mat_key_start_bit}), /* Dest bit position */
                   &trit_x,
                   0,
                   1 /* bits to copy */);

    p4pd_copy_into_hwentry(hwkey_y,
                    ((${kmbit} - (${kmbit} % 8)) + (7 - (${kmbit} % 8)))- (${mat_key_start_bit}), /* Dest bit position */
                   &trit_y,
                   0,
                   1 /* bits to copy */);
    key_len += 1;
//::                            #endfor
//::                        #endif
//::                    #endfor
//::                    i += 1
//::                #endfor

//::                i = 1
//::                for hdr_un_containers in pddict['tables'][table]['hdrunion_keys']:
//::                    hdrcontainer, hdrukeys = hdr_un_containers
//::                    if len(hdrukeys) > 1:
//::                        ustr = table + '_hdr_u' + str(i) + '.'
//::                        ustr_mask = table + '_mask_hdr_u' + str(i) + '.'
//::                    else:
//::                        ustr=''
//::                        ustr_mask=''
//::                    #endif
//::                    for fields in hdrukeys:
//::                        (p4fldname, p4fldwidth, mask, key_byte_format, key_bit_format) = fields
//::                        if len(key_byte_format):
//::                            for kmbyte, _kbit, width, containerstart in key_byte_format:
//::                                tablebyte = kmbyte
//::                                kbyte = (p4fldwidth - 1 - _kbit) / 8
//::                                kbit = (p4fldwidth - 1 - _kbit) % 8
    /* Header Union Key byte */
//::                                if p4fldwidth <= 32:
//::                                    if kbit < 7:
    /* P4 field is extended.. a less than byte field is extended to byte */
    k = *((uint8_t*)&(swkey->${ustr}${p4fldname}) + ${kbyte});
    m = *((uint8_t*)&(swkey_mask->${ustr_mask}${p4fldname}_mask) + ${kbyte});
    k &= (1 << ${p4fldwidth}) - 1;
    m &= (1 << ${p4fldwidth}) - 1;
//::                                    else:
    /* P4 field is extended.. a less than byte field is extended to byte */
    k = *((uint8_t*)&(swkey->${ustr}${p4fldname}) + ${kbyte});
    m = *((uint8_t*)&(swkey_mask->${ustr_mask}${p4fldname}_mask) + ${kbyte});
    k &= (1 << ${p4fldwidth}) - 1;
    m &= (1 << ${p4fldwidth}) - 1;
//::                                    #endif
//::                                else:
//::                                    if kbit < 7:
    k = *((uint8_t*)&(swkey->${ustr}${p4fldname}[${kbyte}]));
    m = *((uint8_t*)&(swkey_mask->${ustr_mask}${p4fldname}_mask[${kbyte}]));
//::                                    else:
    k = *((uint8_t*)&(swkey->${ustr}${p4fldname}[${kbyte}]));
    m = *((uint8_t*)&(swkey_mask->${ustr_mask}${p4fldname}_mask[${kbyte}]));
//::                                    #endif
//::                                #endif


    trit_x = k & m;
    trit_y = ~k & m;
    p4pd_copy_into_hwentry(hwkey_x,
                   (${tablebyte} * 8) + ${containerstart} - ${mat_key_start_bit}, /* Dest bit position */
                   &trit_x,
                   0,
                   ${width});
    p4pd_copy_into_hwentry(hwkey_y,
                   (${tablebyte} * 8) + ${containerstart} - ${mat_key_start_bit}, /* Dest bit position */
                   &trit_y,
                   0,
                   ${width});
    key_len += ${width};
//::                            #endfor
//::                        #endif
//::                        if len(key_bit_format):
//::                            for kmbit, _kbit in key_bit_format:
//::                                tablebit = kmbit
//::                                kbyte = (p4fldwidth - 1 - _kbit) / 8
//::                                kbit = (p4fldwidth - 1 - _kbit) % 8
    /* Header Union Key bit */
//::                                if p4fldwidth <= 32:
    k = *((uint8_t*)&(swkey->${ustr}${p4fldname}) + ${kbyte}) << (${kbit} % 8);
    m = *((uint8_t*)&(swkey_mask->${ustr}${p4fldname}_mask) + ${kbyte}) << (${kbit} % 8);
//::                                else:
    k = *((uint8_t*)&(swkey->${ustr}${p4fldname}[${kbyte}])) << (${kbit} % 8);
    m = *((uint8_t*)&(swkey_mask->${ustr}${p4fldname}_mask[${kbyte}])) << (${kbit} % 8);
//::                                #endif

    trit_x = ((k & m) >> ${kbit}) & 0x1;
    trit_y = ((~k & m) >>${kbit}) & 0x1;
    p4pd_copy_into_hwentry(hwkey_x,
                    ((${kmbit} - (${kmbit} % 8)) + (7 - (${kmbit} % 8)))- (${mat_key_start_bit}), /* Dest bit position */
                   &trit_x,
                   0, /* start bit */
                   1 /* bits to copy */);

    p4pd_copy_into_hwentry(hwkey_y,
                    ((${kmbit} - (${kmbit} % 8)) + (7 - (${kmbit} % 8)))- (${mat_key_start_bit}), /* Dest bit position */
                   &trit_y,
                   0, /* start bit */
                   1 /* bits to copy */);
    key_len += 1;
//::                            #endfor
//::                        #endif
//::                    #endfor
//::                    i += 1
//::                #endfor


//::                for kbyte in pddict['tables'][table]['not_my_key_bytes']:
//::                    tablebyte = kbyte
    /* ${kbyte} does not belong to my table. Hence set do not match */
    trit_x = 0x0;/* Do not match case. Set both x any y to 1 */
    trit_y = 0x0; 
    p4pd_copy_into_hwentry(hwkey_x,
                   (${tablebyte} * 8) - ${mat_key_start_bit}, /* Dest bit position */
                   &trit_x,
                   0, /* Start bit in source */
                   8 /* 8 bits */);
    p4pd_copy_into_hwentry(hwkey_y,
                   (${tablebyte} * 8) - ${mat_key_start_bit}, /* Dest bit position */
                   &trit_y,
                   0, /* Start bit in source */
                   8 /* 8 bits */);
    key_len += 8;

//::                #endfor
//::                for kmbit in pddict['tables'][table]['not_my_key_bits']:
//::                    tablebit = kmbit
    /* ${kmbit} does not belong to my table. Hence set do not match */
    trit_x = 0x0;/* Do not match case. Set both x any y to 1 */
    trit_y = 0x0; 
    p4pd_copy_into_hwentry(hwkey_x,
                    ((${kmbit} - (${kmbit} % 8)) + (7 - (${kmbit} % 8)))- (${mat_key_start_bit}), /* Dest bit position */
                   &trit_x,
                   0, /* Start bit in source */
                   1 /* 1 bits */);
    p4pd_copy_into_hwentry(hwkey_y,
                    ((${kmbit} - (${kmbit} % 8)) + (7 - (${kmbit} % 8)))- (${mat_key_start_bit}), /* Dest bit position */
                   &trit_y,
                   0, /* Start bit in source */
                   1 /* 1 bits */);
    key_len += 1;

//::                #endfor

    return (key_len);
}
//::            elif pddict['tables'][table]['type'] != 'Index' and pddict['tables'][table]['type'] != 'Mpu':
static uint32_t
${table}_hwkey_build(uint32_t tableid,
                     ${table}_swkey_t *swkey, 
                     uint8_t *hwkey)
{
    uint32_t key_len = 0;
//::                # Do not include Action-PC in hwkey. action-pc will be
//::                # prepended when combining key, action-data
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
//::                for fields in pddict['tables'][table]['keys']:
//::                    (p4fldname, p4fldwidth, mask, key_byte_format, key_bit_format) = fields
//::                    if len(key_byte_format):
//::                        for kmbyte, kbit, _, _ in key_byte_format:
//::                            tablebyte = kmbyte
     * [${p4fldname}, ${kbit},  ${tablebyte}, ${kmbyte} ]
//::                        #endfor
//::                    #endif
//::                #endfor
//::                for fld_un_containers in pddict['tables'][table]['fldunion_keys']:
//::                    fldcontainer, fldukeys = fld_un_containers
//::                    for fields in fldukeys:
//::                        (p4fldname, p4fldwidth, mask, key_byte_format, key_bit_format) = fields
//::                        if len(key_byte_format):
//::                            for kmbyte, kbit, _, _ in key_byte_format:
//::                                tablebyte = kmbyte
     * [${p4fldname}, ${kbit},     ${tablebyte}, ${kmbyte} ; Sourced from fld union key]
//::                            #endfor
//::                        #endif
//::                    #endfor
//::                #endfor
//::                for hdr_un_containers in pddict['tables'][table]['hdrunion_keys']:
//::                    hdrcontainer, hdrukeys = hdr_un_containers
//::                    for fields in hdrukeys:
//::                        (p4fldname, p4fldwidth, mask, key_byte_format, key_bit_format) = fields
//::                        if len(key_bit_format):
//::                            for kmbyte, kbit, _, _ in key_byte_format:
//::                                tablebyte = kmbyte
     * [ ${p4fldname}, ${kbit},  ${tablebyte}, ${kmbyte}. Sourced from hdr union key]
//::                            #endfor
//::                        #endif
//::                    #endfor
//::                #endfor
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
//::                for fields in pddict['tables'][table]['keys']:
//::                    (p4fldname, p4fldwidth, mask, key_byte_format, key_bit_format) = fields
//::                    if len(key_bit_format):
//::                        for kmbit, kbit in key_bit_format:
//::                            tablebit = kmbit
     * [ ${p4fldname}, ${kbit},  ${tablebit}, ${kmbit} ]
//::                        #endfor
//::                    #endif
//::                #endfor
//::                for fld_un_containers in pddict['tables'][table]['fldunion_keys']:
//::                    fldcontainer, fldukeys = fld_un_containers
//::                    for fields in fldukeys:
//::                        (p4fldname, p4fldwidth, mask, key_byte_format, key_bit_format) = fields
//::                        if len(key_bit_format):
//::                            for kmbit, kbit in key_bit_format:
//::                                tablebit = kmbit
     * [ ${p4fldname}, ${kbit},  ${tablebit}, ${kmbit} ; Sourced from fld union key]
//::                            #endfor
//::                        #endif
//::                    #endfor
//::                #endfor
//::                for hdr_un_containers in pddict['tables'][table]['hdrunion_keys']:
//::                    hdrcontainer, hdrukeys = hdr_un_containers
//::                    for fields in hdrukeys:
//::                        (p4fldname, p4fldwidth, mask, key_byte_format, key_bit_format) = fields
//::                        if len(key_bit_format):
//::                            for kmbit, kbit in key_bit_format:
//::                                tablebit = kmbit
     * [ ${p4fldname}, ${kbit},  ${tablebit}, ${kmbit}. Sourced from hdr union key]
//::                            #endfor
//::                        #endif
//::                    #endfor
//::                #endfor
     */
//::                mat_key_start_byte = pddict['tables'][table]['match_key_start_byte']
//::                mat_key_start_bit = pddict['tables'][table]['match_key_start_bit']
//::                if pddict['tables'][table]['type'] == 'Hash' or pddict['tables'][table]['type'] == 'Hash_OTcam':
//::                    # In case of HASH table, entire 512b are used for hash computation.
//::                    # Hence place key bits at same position where they appear in KM and rest zero.
//::                    mat_key_start_byte = 0
//::                    mat_key_start_bit = 0
//::                #endif
//::                for fields in pddict['tables'][table]['keys']:
//::                    (p4fldname, p4fldwidth, mask, key_byte_format, key_bit_format) = fields
//::                    if len(key_byte_format):
//::                        for kmbyte, kbit, width, containerstart in key_byte_format:
//::                            kbyte = (p4fldwidth - 1 - kbit) / 8
//::                            tablebyte = kmbyte
    /* Key byte */
    p4pd_copy_into_hwentry(hwkey,
                   (${tablebyte} * 8) + ${containerstart} - ${mat_key_start_bit}, /* Dest bit position */
//::                            if p4fldwidth <= 32:
                   (uint8_t*)((uint8_t*)&(swkey->${p4fldname}) + ${kbyte}),
//::                            else:
                   &(swkey->${p4fldname}[${kbyte}]),
//::                            #endif
                   (${p4fldwidth} - ${kbit}) % 8, /* Start bit in source field */
                   ${width});
    key_len += ${width};
//::                        #endfor
//::                    #endif
//::                    if len(key_bit_format):
//::                        for kmbit, kbit in key_bit_format:
    /* Key bit */
    p4pd_copy_into_hwentry(hwkey,
                    ((${kmbit} - (${kmbit} % 8)) + (7 - (${kmbit} % 8)))- (${mat_key_start_bit}), /* Dest bit position */
//::                            if p4fldwidth <= 32:
                   (uint8_t*)((uint8_t*)&(swkey->${p4fldname}) + ((${p4fldwidth} - ${kbit})/8)),
//::                            else:
                   &(swkey->${p4fldname}[${kbit}/8]),
//::                            #endif
                   (${p4fldwidth} - 1 - ${kbit}) % 8 /* source bit; KM bit 0 is MSB */,
                   1 /* copy single bits */);
    key_len += 1;
//::                        #endfor
//::                    #endif
//::                #endfor

//::                i = 1
//::                for fld_un_containers in pddict['tables'][table]['fldunion_keys']:
//::                    fldcontainer, fldukeys = fld_un_containers
//::                    if len(fldukeys) > 1:
//::                        ustr = table + '_u' + str(i) + '.'
//::                    else:
//::                        ustr=''
//::                    #endif
//::                    for fields in fldukeys:
//::                        (p4fldname, p4fldwidth, mask, key_byte_format, key_bit_format) = fields
//::                        if len(key_byte_format):
//::                            for kmbyte, kbit, width, containerstart in key_byte_format:
//::                                kbyte = (p4fldwidth - 1 - kbit) / 8
//::                                tablebyte = kmbyte
    /* Field Union Key byte */
    p4pd_copy_into_hwentry(hwkey,
                   (${tablebyte} * 8) + ${containerstart} - ${mat_key_start_bit}, /* Dest bit position */
//::                                if p4fldwidth <= 32:
                   (uint8_t*)((uint8_t*)&(swkey->${ustr}${p4fldname}) + ${kbyte}),
//::                                else:
                   &(swkey->${ustr}${p4fldname}[${kbyte}]),
//::                                #endif
                   (${p4fldwidth} - ${kbit}) % 8, /* Start bit in source field */
                   ${width});
    key_len += ${width};
//::                            #endfor
//::                        #endif
//::                        if len(key_bit_format):
//::                            for kmbit, kbit in key_bit_format:
    /* Field Union Key bit */
    p4pd_copy_into_hwentry(hwkey,
                    ((${kmbit} - (${kmbit} % 8)) + (7 - (${kmbit} % 8)))- (${mat_key_start_bit}), /* Dest bit position */
//::                                if p4fldwidth <= 32:
                   (uint8_t*)((uint8_t*)&(swkey->${ustr}${p4fldname}) + ((${p4fldwidth} - ${kbit})/8)),
//::                                else:
                   &(swkey->${ustr}${p4fldname}[${kbit}/8]),
//::                                #endif
                   (${p4fldwidth} - 1 - ${kbit}) % 8 /* source bit; KM bit 0 is MSB */,
                   1 /* copy single bits */);
//::                            #endfor
//::                        #endif
//::                    #endfor
//::                    i += 1
    key_len += 1;
//::                #endfor

//::                i = 1
//::                for hdr_un_containers in pddict['tables'][table]['hdrunion_keys']:
//::                    hdrcontainer, hdrukeys = hdr_un_containers
//::                    if len(fldukeys) > 1:
//::                        ustr = table + '_hdr_u' + str(i) + '.'
//::                    else:
//::                        ustr=''
//::                    #endif
//::                    for fields in hdrukeys:
//::                        (p4fldname, p4fldwidth, mask, key_byte_format, key_bit_format) = fields
//::                        if len(key_byte_format):
//::                            for kmbyte, kbit, width, containerstart in key_byte_format:
//::                                kbyte = (p4fldwidth - 1 - kbit) / 8
//::                                tablebyte = kmbyte
    /* Header Union Key byte */
    p4pd_copy_into_hwentry(hwkey,
                   (${tablebyte} * 8) + ${containerstart} - ${mat_key_start_bit}, /* Dest bit position */
//::                                if p4fldwidth <= 32:
                   (uint8_t*)((uint8_t*)&(swkey->${ustr}${p4fldname}) + ${kbyte}),
//::                                else:
                   &(swkey->${ustr}${p4fldname}[${kbyte}]),
//::                                #endif
                   (${p4fldwidth} - ${kbit}) % 8, /* Start bit in source field */
                   ${width});
    key_len += ${width};
//::                            #endfor
//::                        #endif
//::                        if len(key_bit_format):
//::                            for kmbit, kbit in key_bit_format:
    /* Header Union Key bit */
    p4pd_copy_into_hwentry(hwkey,
                   ((${kmbit} - (${kmbit} % 8)) + (7 - (${kmbit} % 8)))- (${mat_key_start_bit}), /* Dest bit position */
//::                                if p4fldwidth <= 32:
                   (uint8_t*)((uint8_t*)&(swkey->${ustr}${p4fldname}) + ((${p4fldwidth} - ${kbit})/8)),
//::                                else:
                   &(swkey->${ustr}${p4fldname}[${kbit}/8]),
//::                                #endif
                   (${p4fldwidth} - 1 - ${kbit}) % 8 /* source bit; KM bit 0 is MSB */,
                   1 /* copy single bit */);
    key_len += 1;
//::                            #endfor
//::                        #endif
//::                    #endfor
//::                    i += 1
//::                #endfor
    return (key_len);
}
//::            #endif
//::        #endif

//::        if pddict['tables'][table]['type'] == 'Index':
static uint64_t
${table}_index_mapper(uint32_t tableid,
                      ${table}_swkey_t *swkey)
{
    uint64_t _hwkey = 0;
    uint8_t *hwkey = (uint8_t*)&_hwkey;
//::                # Do not include Action-PC in hwkey. action-pc will be
//::                # prepended when combining key, action-data
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
//::                for fields in pddict['tables'][table]['keys']:
//::                    (p4fldname, p4fldwidth, mask, key_byte_format, key_bit_format) = fields
//::                    if len(key_byte_format):
//::                        for kmbyte, kbit, _, _  in key_byte_format:
//::                            tablebyte = kmbyte
     * [${p4fldname}, ${kbit},  ${tablebyte}, ${kmbyte} ]
//::                        #endfor
//::                    #endif
//::                #endfor
//::                for fld_un_containers in pddict['tables'][table]['fldunion_keys']:
//::                    fldcontainer, fldukeys = fld_un_containers
//::                    for fields in fldukeys:
//::                        (p4fldname, p4fldwidth, mask, key_byte_format, key_bit_format) = fields
//::                        if len(key_byte_format):
//::                            for kmbyte, kbit, _, _ in key_byte_format:
//::                                tablebyte = kmbyte
     * [${p4fldname}, ${kbit},     ${tablebyte}, ${kmbyte} ; Sourced from fld union key]
//::                            #endfor
//::                        #endif
//::                    #endfor
//::                #endfor
//::                for hdr_un_containers in pddict['tables'][table]['hdrunion_keys']:
//::                    hdrcontainer, hdrukeys = hdr_un_containers
//::                    for fields in hdrukeys:
//::                        (p4fldname, p4fldwidth, mask, key_byte_format, key_bit_format) = fields
//::                        if len(key_bit_format):
//::                            for kmbyte, kbit, _, _ in key_byte_format:
//::                                tablebyte = kmbyte
     * [${p4fldname}, ${kbit},    ${tablebyte}, ${kmbyte} ; Sourced from fld union key]
//::                            #endfor
//::                        #endif
//::                    #endfor
//::                #endfor
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
//::                for fields in pddict['tables'][table]['keys']:
//::                    (p4fldname, p4fldwidth, mask, key_byte_format, key_bit_format) = fields
//::                    if len(key_bit_format):
//::                        for kmbit, kbit in key_bit_format:
//::                            tablebit = kmbit
     * [ ${p4fldname}, ${kbit},  ${tablebit}, ${kmbit} ]
//::                        #endfor
//::                    #endif
//::                #endfor
//::                for fld_un_containers in pddict['tables'][table]['fldunion_keys']:
//::                    fldcontainer, fldukeys = fld_un_containers
//::                    for fields in fldukeys:
//::                        (p4fldname, p4fldwidth, mask, key_byte_format, key_bit_format) = fields
//::                        if len(key_bit_format):
//::                            for kmbit, kbit in key_bit_format:
//::                                tablebit = kmbit
     * [ ${p4fldname}, ${kbit},  ${tablebit}, ${kmbit} ; Sourced from fld union key]
//::                            #endfor
//::                        #endif
//::                    #endfor
//::                #endfor
//::                for hdr_un_containers in pddict['tables'][table]['hdrunion_keys']:
//::                    hdrcontainer, hdrukeys = hdr_un_containers
//::                    for fields in hdrukeys:
//::                        (p4fldname, p4fldwidth, mask, key_byte_format, key_bit_format) = fields
//::                        if len(key_bit_format):
//::                            for kmbit, kbit in key_bit_format:
//::                                tablebit = kmbit
     * [ ${p4fldname}, ${kbit},  ${tablebit}, ${kmbit}. Sourced from hdr union key]
//::                            #endfor
//::                        #endif
//::                    #endfor
//::                #endfor
     */
//::                mat_key_start_byte = pddict['tables'][table]['match_key_start_byte']
//::                mat_key_start_bit = pddict['tables'][table]['match_key_start_bit']
//::                mat_key_bit_length = pddict['tables'][table]['match_key_bit_length']
//::                if pddict['tables'][table]['type'] == 'Hash' or pddict['tables'][table]['type'] == 'Hash_OTcam':
//::                    # In case of HASH table, entire 512b are used for hash computation.
//::                    # Hence place key bits at same position where they appear in KM and rest zero.
//::                    mat_key_start_byte = 0
//::                    mat_key_start_bit = 0
//::                #endif
//::                for fields in pddict['tables'][table]['keys']:
//::                    (p4fldname, p4fldwidth, mask, key_byte_format, key_bit_format) = fields
//::                    if len(key_byte_format):
//::                        for kmbyte, kbit, width, containerstart in key_byte_format:
//::                            kbyte = (p4fldwidth - 1 - kbit) / 8
//::                            tablebyte = kmbyte
    /* Key byte */
   *(hwkey + ((${mat_key_bit_length} - ${width} - ((${tablebyte} * 8) + ${containerstart}  - ${mat_key_start_bit})/* Dest bit position */) >> 3)) 
        = (*(uint8_t*)((uint8_t*)&(swkey->${p4fldname}) + ${kbyte})) << (8 - ${width} - ${containerstart});
//::                        #endfor
//::                    #endif
//::                    if len(key_bit_format):
//::                        for kmbit, kbit in key_bit_format:
    /* Key bit */
    p4pd_copy_into_hwentry(hwkey,
                    (((${kmbit} - (${kmbit} % 8)) + (7 - (${kmbit} % 8)))- (${mat_key_start_bit})), /* Dest bit position */
//::                            if p4fldwidth <= 32:
                   (uint8_t*)((uint8_t*)&(swkey->${p4fldname}) + ((${p4fldwidth} - ${kbit})/8)),
//::                            else:
                   &(swkey->${p4fldname}[${kbit}/8]),
//::                            #endif
                   (${p4fldwidth} - 1 - ${kbit}) % 8 /* source bit; KM bit 0 is MSB */,
                   1 /* copy single bits */);
//::                        #endfor
//::                    #endif
//::                #endfor

//::                i = 1
//::                for fld_un_containers in pddict['tables'][table]['fldunion_keys']:
//::                    fldcontainer, fldukeys = fld_un_containers
//::                    if len(fldukeys) > 1:
//::                        ustr = table + '_u' + str(i) + '.'
//::                    else:
//::                        ustr=''
//::                    #endif
//::                    for fields in fldukeys:
//::                        (p4fldname, p4fldwidth, mask, key_byte_format, key_bit_format) = fields
//::                        if len(key_byte_format):
//::                            for kmbyte, kbit, width, containerstart in key_byte_format:
//::                                kbyte = (p4fldwidth - 1 - kbit) / 8
//::                                tablebyte = kmbyte
    /* Field Union Key byte */
   *(hwkey + ((${mat_key_bit_length} - ${width} - ((${tablebyte} * 8) + ${containerstart}  - ${mat_key_start_bit})/* Dest bit position */) >> 3)) 
        = (*(uint8_t*)((uint8_t*)&(swkey->${p4fldname}) + ${kbyte})) << (8 - ${width} - ${containerstart});
//::                            #endfor
//::                        #endif
//::                        if len(key_bit_format):
//::                            for kmbit, kbit in key_bit_format:
    /* Field Union Key bit */
    p4pd_copy_into_hwentry(hwkey,
                    (((${kmbit} - (${kmbit} % 8)) + (7 - (${kmbit} % 8)))- (${mat_key_start_bit})), /* Dest bit position */
//::                                if p4fldwidth <= 32:
                   (uint8_t*)((uint8_t*)&(swkey->${ustr}${p4fldname}) + ((${p4fldwidth} - ${kbit})/8)),
//::                                else:
                   &(swkey->${ustr}${p4fldname}[${kbit}/8]),
//::                                #endif
                   (${p4fldwidth} - 1 - ${kbit}) % 8 /* source bit; KM bit 0 is MSB */,
                   1 /* copy single bits */);
//::                            #endfor
//::                        #endif
//::                    #endfor
//::                    i += 1
//::                #endfor

//::                i = 1
//::                for hdr_un_containers in pddict['tables'][table]['hdrunion_keys']:
//::                    hdrcontainer, hdrukeys = hdr_un_containers
//::                    if len(fldukeys) > 1:
//::                        ustr = table + '_hdr_u' + str(i) + '.'
//::                    else:
//::                        ustr=''
//::                    #endif
//::                    for fields in hdrukeys:
//::                        (p4fldname, p4fldwidth, mask, key_byte_format, key_bit_format) = fields
//::                        if len(key_byte_format):
//::                            for kmbyte, kbit, width, containerstart in key_byte_format:
//::                                kbyte = (p4fldwidth - 1 - kbit) / 8
//::                                tablebyte = kmbyte
    /* Header Union Key byte */
   *(hwkey + ((${mat_key_bit_length} - ${width} - ((${tablebyte} * 8) + ${containerstart}  - ${mat_key_start_bit})/* Dest bit position */) >> 3)) 
        = (*(uint8_t*)((uint8_t*)&(swkey->${p4fldname}) + ${kbyte})) << (8 - ${width} - ${containerstart});
//::                            #endfor
//::                        #endif
//::                        if len(key_bit_format):
//::                            for kmbit, kbit in key_bit_format:
    /* Header Union Key bit */
    p4pd_copy_into_hwentry(hwkey,
                   (((${kmbit} - (${kmbit} % 8)) + (7 - (${kmbit} % 8)))- (${mat_key_start_bit})), /* Dest bit position */
//::                                if p4fldwidth <= 32:
                   (uint8_t*)((uint8_t*)&(swkey->${ustr}${p4fldname}) + ((${p4fldwidth} - ${kbit})/8)),
//::                                else:
                   &(swkey->${ustr}${p4fldname}[${kbit}/8]),
//::                                #endif
                   (${p4fldwidth} - 1 - ${kbit}) % 8 /* source bit; KM bit 0 is MSB */,
                   1 /* copy single bit */);
//::                            #endfor
//::                        #endif
//::                    #endfor
//::                    i += 1
//::                #endfor
    return (*hwkey);
}
//::        #endif

/* Install entry into P4-table '${table}'
//::        if pddict['tables'][table]['type'] == 'Index':
 * at location 'index'
//::        elif pddict['tables'][table]['type'] == 'Ternary':
 * at location 'index'
//::        else:
 * at location 'hashindex'.
//::        #endif
 *
 * Arguments: 
 *  IN  : uint32_t tableid                  : Table Id that identifies
 *                                            P4 table. This id is obtained
 *                                            from p4pd_table_id_enum.
//::        if pddict['tables'][table]['type'] == 'Ternary':
 *
 *  IN  : uint32_t index                    : TCAM table index where entry is 
 *                                            installed.
 *                                            Caller of the API is expected to
 *                                            provide this index based on
 *                                            placement decision made for the key.
//::        elif pddict['tables'][table]['type'] == 'Index':
 *  IN  : uint32_t index                    : Direct/Indexed table location where
 *                                            entry is written to.
//::        else:
 *  IN  : uint32_t hashindex                : Hash index where entry is 
 *                                            installed.
//::        #endif
 *
//::        if pddict['tables'][table]['type'] != 'Index':
 *  IN  : uint8_t *hwkey                    : Hardware key to be installed
 *                                            into P4-table
//::        #endif
//::        if pddict['tables'][table]['type'] == 'Ternary':
 *  IN  : uint8_t *hwkey_y                  : Trit match mask used in Tcam 
 *                                            to represent key.
//::        #endif
 *  IN  : ${table}_actions_un *actiondata   : Action data associated with 
 *                                            the key.
 * 
 * Return Value: 
 *  pd_error_t                              : P4PD_SUCCESS / P4PD_FAIL
 */
//::        if pddict['tables'][table]['type'] == 'Ternary':
static p4pd_error_t
${table}_entry_write(uint32_t tableid,
                     uint32_t index,
                     uint8_t *hwkey, 
                     uint8_t *hwkey_y,
                     ${table}_actiondata *actiondata)
{
    uint8_t  packed_actiondata[P4PD_MAX_ACTION_DATA_LEN] = {0};
    uint8_t  sram_hwentry[P4PD_MAX_MATCHKEY_LEN + P4PD_MAX_ACTION_DATA_LEN] = {0};
    uint32_t hwkey_len, hwkeymask_len, actiondatalen;
    uint16_t action_pc, entry_size;

//::            if len(pddict['tables'][table]['actions']) > 1:
//::                add_action_pc = True
//::            else:
//::                add_action_pc = False
//::            #endif
//::            if add_action_pc:
    action_pc = capri_get_action_pc(tableid, actiondata->actionid);
//::            else:
    action_pc = 0xff;
//::            #endif
    actiondatalen = ${table}_pack_action_data(tableid, actiondata, 
                                                packed_actiondata);
    entry_size = p4pd_p4table_entry_prepare(sram_hwentry, 
                                            action_pc, 
                                            NULL /* No MatchKey */, 
                                            0, /* Zero matchkeylen */
                                            packed_actiondata,
                                            actiondatalen);
    p4pd_swizzle_bytes(sram_hwentry, entry_size);
    capri_table_entry_write(tableid, index, sram_hwentry, entry_size);

    // Install Key in TCAM
    tcam_${table}_hwkey_len(tableid, &hwkey_len, &hwkeymask_len);
    // Swizzle Key installed in TCAM before writing to TCAM memory
    // because TCAM entry is not built using p4pd_p4table_entry_prepare
    // function where bytes are swizzled.
    p4pd_swizzle_bytes(hwkey, hwkey_len);
    p4pd_swizzle_bytes(hwkey_y, hwkeymask_len);
    int pad = (hwkey_len % 16) ? (16 - (hwkey_len % 16)) : 0;
    capri_tcam_table_entry_write(tableid, index, hwkey, hwkey_y, hwkey_len + pad);

    return (P4PD_SUCCESS);
}
//::        elif pddict['tables'][table]['type'] == 'Index' or pddict['tables'][table]['type'] == 'Mpu':
static p4pd_error_t
${table}_entry_write(uint32_t tableid,
                     uint32_t index, 
                     ${table}_actiondata *actiondata)
{
    uint8_t  action_pc;
    uint8_t  packed_actiondata[P4PD_MAX_ACTION_DATA_LEN] = {0};
    uint8_t  hwentry[P4PD_MAX_MATCHKEY_LEN + P4PD_MAX_ACTION_DATA_LEN] = {0};
    uint16_t entry_size, actiondatalen;

//::            if len(pddict['tables'][table]['actions']) > 1:
//::                add_action_pc = True
//::            else:
//::                add_action_pc = False
//::            #endif
//::            if add_action_pc:
    action_pc = capri_get_action_pc(tableid, actiondata->actionid);
//::            else:
    action_pc = 0xff;
//::            #endif
    actiondatalen = ${table}_pack_action_data(tableid, actiondata,
                                              packed_actiondata);
    entry_size = p4pd_p4table_entry_prepare(hwentry,
                                            action_pc,
                                            NULL /* Index Table. No MatchKey*/,
                                            0, /* Zero matchkeylen */
                                            packed_actiondata,
                                            actiondatalen);
//::            if pddict['tables'][table]['location'] == 'HBM':
    capri_hbm_table_entry_write(tableid, index, hwentry, entry_size);
//::            else:
    p4pd_swizzle_bytes(hwentry, entry_size);
    capri_table_entry_write(tableid, index, hwentry, entry_size);
//::            #endif
    return (P4PD_SUCCESS);
}
//::        else:
static p4pd_error_t
${table}_entry_write(uint32_t tableid,
                     uint32_t hashindex,
                     uint8_t *hwkey,
                     ${table}_actiondata *actiondata)
{
    uint32_t hwactiondata_len, hwkey_len;
    uint8_t  action_pc;
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

    ${table}_hwentry_query(tableid, &hwkey_len, &hwactiondata_len);
    hash_${table}_key_len(tableid, &key_len);
                          
//::            if len(pddict['tables'][table]['actions']) > 1:
//::                add_action_pc = True
//::            else:
//::                add_action_pc = False
//::            #endif
//::            if add_action_pc:
    action_pc = capri_get_action_pc(tableid, actiondata->actionid);
//::            else:
    action_pc = 0xff;
//::            #endif

    /* For hash tables in both pipe and HBM, pack entries such that
     * key bits in memory is aligned with key bits in KM
     */
    actiondatalen = ${table}_pack_action_data(tableid, actiondata,
                                     packed_actiondata_before_key,
                                     &actiondata_len_before_key,
                                     packed_actiondata_after_key,
                                     &actiondata_len_after_key);

    entry_size = p4pd_hash_table_entry_prepare(hwentry,
                                             action_pc,
                                             ${pddict['tables'][table]['match_key_start_bit']},/*MatchKeyStartBit */
                                             hwkey,
                                             key_len,
                                             packed_actiondata_before_key,
                                             actiondata_len_before_key,
                                             packed_actiondata_after_key,
                                             actiondata_len_after_key);

//::            if pddict['tables'][table]['location'] == 'HBM':
    capri_hbm_table_entry_write(tableid, hashindex, hwentry, entry_size);
//::            else:
    p4pd_swizzle_bytes(hwentry, entry_size);
    capri_table_entry_write(tableid, hashindex, hwentry, entry_size);
//::            #endif
    
    return (P4PD_SUCCESS);
}
//::        #endif

//::        #Hash tables in both HBM and PIPE are unpacked the same way
//::        if pddict['tables'][table]['type'] != 'Ternary' and pddict['tables'][table]['type'] != 'Index' and pddict['tables'][table]['type'] != 'Mpu':
static uint32_t
hash_${table}_unpack_action_data(uint32_t tableid,
                                 uint8_t  actionid,
                                 uint8_t *packed_actiondata_before_key,
                                 uint16_t actiondata_len_before_key,
                                 uint8_t *packed_actiondata_after_key,
                                 uint16_t actiondata_len_after_key,
                                 ${table}_actiondata *actiondata)
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

    memset(actiondata, 0, sizeof(${table}_actiondata));

    actiondata->actionid = actionid;

    switch(actiondata->actionid) {
//::            for action in pddict['tables'][table]['actions']:
//::                (actionname, actionfldlist) = action
//::                actname = actionname.upper()
//::                if not len(actionfldlist):
//::                    continue
//::                #endif
        case ${tbl}_${actname}_ID:
//::                mat_key_start_byte = pddict['tables'][table]['match_key_start_byte']
//::                mat_key_start_bit = pddict['tables'][table]['match_key_start_bit']
//::                mat_key_bit_length = pddict['tables'][table]['match_key_bit_length']
            if (actiondata_len_before_key) {
                copy_before_key = true;
                packed_action_data = packed_actiondata_before_key;
            } else {
                copy_before_key = false;
                packed_action_data = packed_actiondata_after_key;
            }
//::                for actionfld in actionfldlist:
//::                    actionfldname, actionfldwidth = actionfld
            bits_to_copy = ${actionfldwidth}; /* = length of actiondata field */
            dest_start_bit = 0;
            if (copy_before_key) {
                if ((src_start_bit +
                    ${actionfldwidth}) > actiondata_len_before_key ) {
                    /* Copy part of the field before from actiondata before key
                     * remaining part from action data after key
                     */
                    bits_from_adata_before_key = actiondata_len_before_key - src_start_bit;
                    p4pd_copy_be_adata_to_le_dest(
//::                    if actionfldwidth <= 32:
                                   (uint8_t*)&(actiondata->${table}_action_u.\
                                   ${table}_${actionname}.${actionfldname}),
//::                    else:
                                   (uint8_t*)(actiondata->${table}_action_u.\
                                   ${table}_${actionname}.${actionfldname}),
//::                    #endif
                                   ${actionfldwidth} - bits_from_adata_before_key,
                                   packed_action_data,
                                   src_start_bit,
                                   bits_from_adata_before_key);
                    actiondatalen += bits_from_adata_before_key;
                    dest_start_bit = 0;
                    copy_before_key = false;
                    packed_action_data = packed_actiondata_after_key;
                    /* remaining field bits to be copied after end of match key */
                    bits_to_copy = ${actionfldwidth} - bits_from_adata_before_key;
                    /* startbit of second portion of packed actiondata could
                     *  start in middle of byte (after last key bit)
                     */
                    src_start_bit = ${mat_key_bit_length % 8};
                }
            }
            p4pd_copy_be_adata_to_le_dest(
//::                    if actionfldwidth <= 32:
                           (uint8_t*)&(actiondata->${table}_action_u.\
                                ${table}_${actionname}.${actionfldname}),
//::                    else:
                           (uint8_t*)(actiondata->${table}_action_u.\
                                ${table}_${actionname}.${actionfldname}),
//::                    #endif
                           dest_start_bit,
                           packed_action_data,
                           src_start_bit,
                           bits_to_copy);
            actiondatalen += bits_to_copy;
            src_start_bit += bits_to_copy;


        //::        #endfor
        break;
//::            #endfor
    }
    return (actiondatalen);
}
//::        else:
static uint32_t
${table}_unpack_action_data(uint32_t tableid,
                            uint8_t actionid,
                            uint8_t *packed_actiondata,
                            ${table}_actiondata *actiondata)
{
    uint16_t src_start_bit;
    uint16_t actiondatalen;
    
    (void)src_start_bit;

    actiondatalen = 0;
    src_start_bit = 0;

    memset(actiondata, 0, sizeof(${table}_actiondata));
    actiondata->actionid = actionid;

    switch(actiondata->actionid) {
//::            for action in pddict['tables'][table]['actions']:
//::                (actionname, actionfldlist) = action
//::                actname = actionname.upper()
//::                if not len(actionfldlist):
//::                    continue
//::                #endif
        case ${tbl}_${actname}_ID:
//::                for actionfld in actionfldlist:
//::                    actionfldname, actionfldwidth = actionfld
            if ((src_start_bit + ${actionfldwidth})
                >= P4PD_MAX_ACTION_DATA_LEN) {
                assert(0);
            }
            p4pd_copy_be_adata_to_le_dest(
//::                    if actionfldwidth <= 32:
                           (uint8_t*)&(actiondata->${table}_action_u.\
                                ${table}_${actionname}.${actionfldname}),
//::                    else:
                           (uint8_t*)(actiondata->${table}_action_u.\
                                ${table}_${actionname}.${actionfldname}),
//::                    #endif
                           0, /* start bit in action-data destination field */
                           packed_actiondata,
                           src_start_bit, /* Start bit in packed actiondata source */
                           ${actionfldwidth});
            src_start_bit += ${actionfldwidth};
            actiondatalen += ${actionfldwidth};
//::                #endfor
        break;
//::            #endfor
    }
    return (actiondatalen);
}
//::        #endif


//::        if pddict['tables'][table]['type'] != 'Index' and pddict['tables'][table]['type'] != 'Mpu':
/* Unbuild hardware key function for p4-table '${table}'
 * Arguments:
 *  IN  : uint32_t tableid                  : Table Id that identifies
 *                                            P4 table. This id is obtained
 *                                            from p4pd_table_id_enum.
 *  IN : uint8_t *hw_key                    : hardware key as byte stream
//::            if pddict['tables'][table]['type'] == 'Ternary':
 *  IN : uint8_t *hw_key_mask               : hardware key mask as byte stream
//::            #endif
 *  OUT  : ${table}_swkey_t *swkey          : software key built from hardware key
//::            if pddict['tables'][table]['type'] == 'Ternary':
 *  OUT  : ${table}_swkey_mask_t *swkey_mask: software key mask built from
 *                                            tcam match key.
//::            #endif
 *
 * Return Value:
 *  uint32_t                                : Length of hardware key in bits.
 */
//::            if pddict['tables'][table]['type'] == 'Ternary':
static uint32_t
${table}_hwkey_hwmask_unbuild(uint32_t tableid,
                              uint8_t *hw_key,
                              uint8_t *hw_key_mask,
                              uint16_t hwkey_len,
                              ${table}_swkey_t *swkey,
                              ${table}_swkey_mask_t *swkey_mask)
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
//::                for fields in pddict['tables'][table]['keys']:
//::                    (p4fldname, p4fldwidth, mask, key_byte_format, key_bit_format) = fields
//::                    if len(key_byte_format):
//::                        for kmbyte, kbit, _, _ in key_byte_format:
//::                            tablebyte = kmbyte
     * [${p4fldname}, ${kbit},  ${tablebyte}, ${kmbyte} ]
//::                        #endfor
//::                    #endif
//::                #endfor
//::                for fld_un_containers in pddict['tables'][table]['fldunion_keys']:
//::                    fldcontainer, fldukeys = fld_un_containers
//::                    for fields in fldukeys:
//::                        (p4fldname, p4fldwidth, mask, key_byte_format, key_bit_format) = fields
//::                        if len(key_byte_format):
//::                            for kmbyte, kbit,_, _ in key_byte_format:
//::                                tablebyte = kmbyte
     * [${p4fldname}, ${kbit},  ${tablebyte}, ${kmbyte}. Sourced from fld union key]
//::                            #endfor
//::                        #endif
//::                    #endfor
//::                #endfor
//::                for hdr_un_containers in pddict['tables'][table]['hdrunion_keys']:
//::                    hdrcontainer, hdrukeys = hdr_un_containers
//::                    for fields in hdrukeys:
//::                        (p4fldname, p4fldwidth, mask, key_byte_format, key_bit_format) = fields
//::                        if len(key_byte_format):
//::                            for kmbyte, kbit, _, _ in key_byte_format:
//::                                tablebyte = kmbyte
     * [${p4fldname}, ${kbit},  ${tablebyte}, ${kmbyte}. Sourced from hdr union key]
//::                            #endfor
//::                        #endif
//::                    #endfor
//::                #endfor
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
//::                for fields in pddict['tables'][table]['keys']:
//::                    (p4fldname, p4fldwidth, mask, key_byte_format, key_bit_format) = fields
//::                    if len(key_bit_format):
//::                        for kmbit, kbit in key_bit_format:
//::                            tablebit = kmbit
     * [ ${p4fldname}, ${kbit},  ${tablebit}, ${kmbit} ]
//::                        #endfor
//::                    #endif
//::                #endfor
//::                for fld_un_containers in pddict['tables'][table]['fldunion_keys']:
//::                    fldcontainer, fldukeys = fld_un_containers
//::                    for fields in fldukeys:
//::                        (p4fldname, p4fldwidth, mask, key_byte_format, key_bit_format) = fields
//::                        if len(key_bit_format):
//::                            for kmbit, kbit in key_bit_format:
//::                                tablebit = kmbit
     * [ ${p4fldname}, ${kbit},  ${tablebit}, ${kmbit}. Sourced from fld union key]
//::                            #endfor
//::                        #endif
//::                    #endfor
//::                #endfor
//::                for hdr_un_containers in pddict['tables'][table]['hdrunion_keys']:
//::                    hdrcontainer, hdrukeys = hdr_un_containers
//::                    for fields in hdrukeys:
//::                        (p4fldname, p4fldwidth, mask, key_byte_format, key_bit_format) = fields
//::                        if len(key_bit_format):
//::                            for kmbit, kbit in key_bit_format:
//::                                tablebit = kmbit
     * [ ${p4fldname}, ${kbit},  ${tablebit}, ${kmbit}. Sourced from hdr union key]
//::                            #endfor
//::                        #endif
//::                    #endfor
//::                #endfor
     */
//::                if len(pddict['tables'][table]['not_my_key_bytes']):
    /*
     * [ P4Table   Match      Byte Location      ByteLocation ]
     * [ MatchKey  Key,       in HW table(HBM    in KeyMaker  ]
     * [ Name,     start      or P4pipe memory),              ]
     * [           bit        before actionpc                 ]
     * [                      is prepended or                 ]
     * [                      before byte                     ]
     * [                      swizzling is done               ]
     * ________________________________________________________
     *
//::                    for kmbyte in pddict['tables'][table]['not_my_key_bytes']:
//::                        tablebyte = kmbyte
     * [XXXXXXX, nnnnnnnn, ${tablebyte}, ${kmbyte} ]
//::                    #endfor
     */
//::                #endif
//::                if len(pddict['tables'][table]['not_my_key_bits']):
    /*
     *
     * [ P4Table   Match      Bit Location       ByteLocation ]
     * [ MatchKey  KeyBit,    in HW table(HBM    in KeyMaker  ]
     * [ Name,                or P4pipe memory),              ]
     * [                      before actionpc                 ]
     * [                      is prepended or                 ]
     * [                      before byte                     ]
     * [                      swizzling is done               ]
     * ________________________________________________________
     *
//::                    for kmbit in pddict['tables'][table]['not_my_key_bits']:
//::                        tablebit = kmbit
     * [XXXXXXX, nnnnnnnn, ${tablebit}, ${kmbit} ]
//::                    #endfor
     */
//::                #endif
    memset(swkey, 0, sizeof(${table}_swkey_t));
    memset(swkey_mask, 0, sizeof(${table}_swkey_mask_t));

//::                mat_key_start_byte = pddict['tables'][table]['match_key_start_byte']
//::                mat_key_start_bit = pddict['tables'][table]['match_key_start_bit']
//::                for fields in pddict['tables'][table]['keys']:
//::                    (p4fldname, p4fldwidth, mask, key_byte_format, key_bit_format) = fields
//::                    if len(key_byte_format):
//::                        for kmbyte, _kbit, width, containerstart in key_byte_format:
//::                            tablebyte = kmbyte
//::                            kbyte = (p4fldwidth - 1 - _kbit) / 8
//::                            kbit = (p4fldwidth - 1 - _kbit) % 8
    /* Key byte */
    trit_x = 0;
    trit_y = 0;
    p4pd_copy_be_src_to_le_dest(&trit_x,
                           0,
                           hw_key,
                           (${tablebyte} * 8) + ${containerstart}  - ${mat_key_start_bit}, /* source bit position */
                           ${width});

    p4pd_copy_be_src_to_le_dest(&trit_y,
                           0, 
                           hw_key_mask,
                           (${tablebyte} * 8) + ${containerstart} - ${mat_key_start_bit}, /* source bit position */
                           ${width});

    m = trit_x ^ trit_y;
    k = trit_x & m;
//::                            if p4fldwidth <= 32:
//::                                if kbit < 7:
    /* P4 field is extended.. a less than byte field is extended to byte */
//::                                #endif
    *((uint8_t*)&(swkey->${p4fldname}) + ${kbyte}) = k;
    *((uint8_t*)&(swkey_mask->${p4fldname}_mask) + ${kbyte}) = m;

//::                            else:
//::                                if kbit < 7:
    /* P4 field is extended.. a less than byte field is extended to byte */
//::                                #endif
    *((uint8_t*)&(swkey->${p4fldname}[${kbyte}])) = k;
    *((uint8_t*)&(swkey_mask->${p4fldname}_mask)[${kbyte}]) = m;
//::                            #endif
//::                        #endfor
//::                    #endif

//::                    if len(key_bit_format):
//::                        for kmbit, _kbit in key_bit_format:
//::                            kbyte = (p4fldwidth - 1 - _kbit) / 8
//::                            kbit = (p4fldwidth - 1 - _kbit) % 8
    /* Key bit */
    trit_x = 0;
    trit_y = 0;
    p4pd_copy_single_bit(&trit_x,
                           0,
                           hw_key + (${kmbit - mat_key_start_bit} >> 3),
                           ((${kmbit} - (${kmbit} % 8)) + (7 - (${kmbit} % 8))) - (${mat_key_start_bit}), /* Source bit position */
                           1 /* bits to copy */);
    p4pd_copy_single_bit(&trit_y,
                           0,
                           hw_key_mask + (${kmbit - mat_key_start_bit} >> 3),
                           ((${kmbit} - (${kmbit} % 8)) + (7 - (${kmbit} % 8))) - (${mat_key_start_bit}), /* Source bit position */
                           1 /* bits to copy */);
    m = trit_x ^ trit_y;
    k = trit_x & m;
//::                            if p4fldwidth <= 32:
    *((uint8_t*)&(swkey->${p4fldname}) + ${kbyte}) &= ~(1 << ${kbit});
    *((uint8_t*)&(swkey->${p4fldname}) + ${kbyte}) |= k << (${kbit});
    *((uint8_t*)&(swkey_mask->${p4fldname}_mask) + ${kbyte}) &= ~(1 << ${kbit});
    *((uint8_t*)&(swkey_mask->${p4fldname}_mask) + ${kbyte}) |= m << (${kbit});
//::                            else:
    *((uint8_t*)&(swkey->${p4fldname}[${kbyte}])) &= ~(1 << ${kbit});
    *((uint8_t*)&(swkey->${p4fldname}[${kbyte}])) |= k << (${kbit});
    *((uint8_t*)&(swkey_mask->${p4fldname}_mask[${kbyte}])) &= ~(1 << ${kbit});
    *((uint8_t*)&(swkey_mask->${p4fldname}_mask[${kbyte}])) |= m << (${kbit});
//::                            #endif
//::                        #endfor
//::                    #endif
//::                #endfor

//::                i = 1
//::                for fld_un_containers in pddict['tables'][table]['fldunion_keys']:
//::                    fldcontainer, fldukeys = fld_un_containers
//::                    if len(fldukeys) > 1:
//::                        ustr = table + '_u' + str(i) + '.'
//::                        ustr_mask = table + '_mask_u' + str(i) + '.'
//::                    else:
//::                        ustr=''
//::                        ustr_mask=''
//::                    #endif
//::                    for fields in fldukeys:
//::                        (p4fldname, p4fldwidth, mask, key_byte_format, key_bit_format) = fields
//::                        if len(key_byte_format):
//::                            for kmbyte, _kbit, width, containerstart in key_byte_format:
//::                                tablebyte = kmbyte
//::                                kbyte = (p4fldwidth - 1 - _kbit) / 8
//::                                kbit = (p4fldwidth - 1 - _kbit) % 8
    /* Field Union Key byte */
    trit_x = 0;
    trit_y = 0;
    p4pd_copy_be_src_to_le_dest(&trit_x,
                           0,
                           hw_key,
                           (${tablebyte} * 8) + ${containerstart} - ${mat_key_start_bit}, /* source bit position */
                           ${width});

    p4pd_copy_be_src_to_le_dest(&trit_y,
                           0, 
                           hw_key_mask,
                           (${tablebyte} * 8) + ${containerstart} - ${mat_key_start_bit}, /* source bit position */
                           ${width});

    m = trit_x ^ trit_y;
    k = trit_x & m;
//::                                if p4fldwidth <= 32:
//::                                    if kbit < 7:
    /* P4 field is extended.. a less than byte field is extended to byte */
//::                                    #endif
    *((uint8_t*)&(swkey->${ustr}${p4fldname}) + ${kbyte}) = k;
    *((uint8_t*)&(swkey_mask->${ustr_mask}${p4fldname}_mask) + ${kbyte}) = m;
//::                                else:
//::                                    if kbit < 7:
    /* P4 field is extended.. a less than byte field is extended to byte */
//::                                    #endif
    *((uint8_t*)&(swkey->${ustr}${p4fldname}[${kbyte}])) = k;
    *((uint8_t*)&(swkey_mask->${ustr_mask}${p4fldname}_mask[${kbyte}])) = m;
//::                                #endif

//::                            #endfor
//::                        #endif
//::                        if len(key_bit_format):
//::                            for kmbit, _kbit in key_bit_format:
//::                                kbyte = (p4fldwidth - 1 - _kbit) / 8
//::                                kbit = (p4fldwidth - 1 - _kbit) % 8
    /* Field Union Key bit */
    trit_x = 0;
    trit_y = 0;
    p4pd_copy_single_bit(&trit_x,
                           0,
                           hw_key + (${kmbit - mat_key_start_bit} >> 8),
                           ((${kmbit} - (${kmbit} % 8)) + (7 - (${kmbit} % 8))) - (${mat_key_start_bit}), /* Source bit position */
                           1 /* bits to copy */);
    p4pd_copy_single_bit(&trit_y,
                           0,
                           hw_key_mask + (${kmbit - mat_key_start_bit} >> 3),
                           ((${kmbit} - (${kmbit} % 8)) + (7 - (${kmbit} % 8))) - (${mat_key_start_bit}), /* Source bit position */
                           1 /* bits to copy */);
    m = trit_x ^ trit_y;
    k = trit_x & m;

//::                                if p4fldwidth <= 32:
    *((uint8_t*)&(swkey->${ustr}${p4fldname}) + ${kbyte}) &= ~(1 << ${kbit});
    *((uint8_t*)&(swkey->${ustr}${p4fldname}) + ${kbyte}) |= k << (${kbit});
    *((uint8_t*)&(swkey_mask->${ustr_mask}${p4fldname}_mask) + ${kbyte}) &= ~(1 << ${kbit});
    *((uint8_t*)&(swkey_mask->${ustr_mask}${p4fldname}_mask) + ${kbyte}) |= m << (${kbit});
//::                                else:
    *((uint8_t*)&(swkey->${ustr}${p4fldname}[${kbyte}])) &= ~(1 << ${kbit} % 8);
    *((uint8_t*)&(swkey->${ustr}${p4fldname}[${kbyte}])) |= k << (${kbit} % 8);
    *((uint8_t*)&(swkey_mask->{ustr}${p4fldname}_mask[${kbyte}])) &= ~(1 << ${kbit});
    *((uint8_t*)&(swkey_mask->{ustr}${p4fldname}_mask[${kbyte}])) |= m << (${kbit});
//::                                #endif
//::                            #endfor
//::                        #endif
//::                    #endfor
//::                    i += 1
//::                #endfor

//::                i = 1
//::                for hdr_un_containers in pddict['tables'][table]['hdrunion_keys']:
//::                    hdrcontainer, hdrukeys = hdr_un_containers
//::                    if len(hdrukeys) > 1:
//::                        ustr = table + '_hdr_u' + str(i) + '.'
//::                        ustr_mask = table + '_mask_hdr_u' + str(i) + '.'
//::                    else:
//::                        ustr=''
//::                        ustr_mask=''
//::                    #endif
//::                    for fields in hdrukeys:
//::                        (p4fldname, p4fldwidth, mask, key_byte_format, key_bit_format) = fields
//::                        if len(key_byte_format):
//::                            for kmbyte, _kbit, width, containerstart in key_byte_format:
//::                                tablebyte = kmbyte
//::                                kbyte = (p4fldwidth - 1 - _kbit) / 8
//::                                kbit = (p4fldwidth - 1 - _kbit) % 8
    /* Header Union Key byte */
    trit_x = 0;
    trit_y = 0;
    p4pd_copy_be_src_to_le_dest(&trit_x,
                           0,
                           hw_key,
                           (${tablebyte} * 8) + ${containerstart} - ${mat_key_start_bit}, /* source bit position */
                           ${width});

    p4pd_copy_be_src_to_le_dest(&trit_y,
                           0, 
                           hw_key_mask,
                           (${tablebyte} * 8) + ${containerstart} - ${mat_key_start_bit}, /* source bit position */
                           ${width});

    m = trit_x ^ trit_y;
    k = trit_x & m;
//::                                kbyte = kbit/8
//::                                if p4fldwidth <= 32:
//::                                    if kbit < 7:
    /* P4 field is extended.. a less than byte field is extended to byte */
//::                                    #endif
    *((uint8_t*)&(swkey->${ustr}${p4fldname}) + ${kbyte}) = k;
    *((uint8_t*)&(swkey_mask->${ustr_mask}${p4fldname}_mask) + ${kbyte}) = m;
//::                                else:
//::                                    if kbit < 7:
    /* P4 field is extended.. a less than byte field is extended to byte */
//::                                    #endif
    *((uint8_t*)&(swkey->${ustr}${p4fldname}[${kbyte}])) = k;
    *((uint8_t*)&(swkey_mask->${ustr_mask}${p4fldname}_mask[${kbyte}])) = m;
//::                                #endif
//::                            #endfor
//::                        #endif

//::                        if len(key_bit_format):
//::                            for kmbit, kbit in key_bit_format:
//::                                kbyte = (p4fldwidth - 1 - _kbit) / 8
//::                                kbit = (p4fldwidth - 1 - _kbit) % 8
    /* Field Union Key bit */
    trit_x = 0;
    trit_y = 0;
    p4pd_copy_single_bit(&trit_x,
                           0,
                           hw_key + (${kmbit - mat_key_start_bit} >> 3),
                           ((${kmbit} - (${kmbit} % 8)) + (7 - (${kmbit} % 8))) - (${mat_key_start_bit}), /* Source bit position */
                           1 /* bits to copy */);
    p4pd_copy_single_bit(&trit_y,
                           0,
                           hw_key_mask + (${kmbit - mat_key_start_bit} >> 3),
                           ((${kmbit} - (${kmbit} % 8)) + (7 - (${kmbit} % 8))) - (${mat_key_start_bit}), /* Source bit position */
                           1 /* bits to copy */);
    m = trit_x ^ trit_y;
    k = trit_x & m;
//::                                if p4fldwidth <= 32:
    *((uint8_t*)&(swkey->${ustr}${p4fldname}) + ${kbyte}) &= ~(1 << ${kbit});
    *((uint8_t*)&(swkey->${ustr}${p4fldname}) + ${kbyte}) |= k << (${kbit});
    *((uint8_t*)&(swkey_mask->${ustr_mask}${p4fldname}_mask) + ${kbyte}) &= ~(1 << ${kbit});
    *((uint8_t*)&(swkey_mask->${ustr_mask}${p4fldname}_mask) + ${kbyte}) |= m << (${kbit});
//::                                else:
    *((uint8_t*)&(swkey->${ustr}${p4fldname}[${kbyte}])) &= ~(1 << ${kbit});
    *((uint8_t*)&(swkey->${ustr}${p4fldname}[${kbyte}])) |= k << (${kbit});
    *((uint8_t*)&(swkey_mask->{ustr}${p4fldname}_mask[${kbyte}])) &= ~(1 << ${kbit});
    *((uint8_t*)&(swkey_mask->{ustr}${p4fldname}_mask[${kbyte}])) |= m << (${kbit});
//::                                #endif
//::                            #endfor
//::                        #endif
//::                    #endfor
//::                    i += 1
//::                #endfor
    return (hwkey_len);
}
//::            else:
static uint32_t
${table}_hwkey_unbuild(uint32_t tableid,
                       uint8_t *hwkey,
                       uint16_t hwkey_len,
                       ${table}_swkey_t *swkey)
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
//::                for fields in pddict['tables'][table]['keys']:
//::                    (p4fldname, p4fldwidth, mask, key_byte_format, key_bit_format) = fields
//::                    if len(key_byte_format):
//::                        for kmbyte, kbit, _, _ in key_byte_format:
//::                            tablebyte = kmbyte
     * [${p4fldname}, ${kbit},  ${tablebyte}, ${kmbyte} ]
//::                        #endfor
//::                    #endif
//::                #endfor
//::                for fld_un_containers in pddict['tables'][table]['fldunion_keys']:
//::                    fldcontainer, fldukeys = fld_un_containers
//::                    for fields in fldukeys:
//::                        (p4fldname, p4fldwidth, mask, key_byte_format, key_bit_format) = fields[1]
//::                        if len(key_byte_format):
//::                            for kmbyte, kbit, _, _ in key_byte_format:
//::                                tablebyte = kmbyte
     * [${p4fldname}, ${kbit},     ${tablebyte}, ${kmbyte} ; Sourced from fld union key]
//::                            #endfor
//::                        #endif
//::                    #endfor
//::                #endfor
//::                for hdr_un_containers in pddict['tables'][table]['hdrunion_keys']:
//::                    hdrcontainer, hdrukeys = hdr_un_containers
//::                    for fields in hdrukeys:
//::                        (p4fldname, p4fldwidth, mask, key_byte_format, key_bit_format) = fields
//::                        if len(key_bit_format):
//::                            for kmbyte, kbit, _, _ in key_byte_format:
//::                                tablebyte = kmbyte
     * [${p4fldname}, ${kbit},     ${tablebyte}, ${kmbyte} ; Sourced from hdr union key]
//::                            #endfor
//::                        #endif
//::                    #endfor
//::                #endfor
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
//::                for fields in pddict['tables'][table]['keys']:
//::                    (p4fldname, p4fldwidth, mask, key_byte_format, key_bit_format) = fields
//::                    if len(key_bit_format):
//::                        for kmbit, kbit in key_bit_format:
//::                            tablebit = kmbit
     * [ ${p4fldname}, ${kbit},  ${tablebit}, ${kmbit} ]
//::                        #endfor
//::                    #endif
//::                #endfor
//::                for fld_un_containers in pddict['tables'][table]['fldunion_keys']:
//::                    fldcontainer, fldukeys = fld_un_containers
//::                    for fields in fldukeys:
//::                        (p4fldname, p4fldwidth, mask, key_byte_format, key_bit_format) = fields[1]
//::                        if len(key_bit_format):
//::                            for kmbit, kbit in key_bit_format:
//::                                tablebit = kmbit
     * [ ${p4fldname}, ${kbit},  ${tablebit}, ${kmbit} ; Sourced from fld union key]
//::                            #endfor
//::                        #endif
//::                    #endfor
//::                #endfor
//::                for hdr_un_containers in pddict['tables'][table]['hdrunion_keys']:
//::                    hdrcontainer, hdrukeys = hdr_un_containers
//::                    for fields in hdrukeys:
//::                        (p4fldname, p4fldwidth, mask, key_byte_format, key_bit_format) = fields
//::                        if len(key_bit_format):
//::                            for kmbit, kbit in key_bit_format:
//::                                tablebit = kmbit
     * [ ${p4fldname}, ${kbit},  ${tablebit}, ${kmbit}. Sourced from hdr union key]
//::                            #endfor
//::                        #endif
//::                    #endfor
//::                #endfor
     */
    memset(swkey, 0, sizeof(${table}_swkey_t));
//::                mat_key_start_byte = pddict['tables'][table]['match_key_start_byte']
//::                mat_key_start_bit = pddict['tables'][table]['match_key_start_bit']
//::                if pddict['tables'][table]['type'] == 'Hash' or pddict['tables'][table]['type'] == 'Hash_OTcam':
//::                    # In case of HASH table, entire 512b are used for hash computation.
//::                    # Hence place key bits at same position where they appear in KM and rest zero.
//::                    mat_key_start_byte = 0
//::                    mat_key_start_bit = 0
//::                #endif
//::                for fields in pddict['tables'][table]['keys']:
//::                    (p4fldname, p4fldwidth, mask, key_byte_format, key_bit_format) = fields
//::                    if len(key_byte_format):
//::                        for kmbyte, kbit, width, containerstart in key_byte_format:
//::                            kbyte = (p4fldwidth - 1 - kbit) / 8
//::                            tablebyte = kmbyte
    /* Copying one byte from table-key into correct p4fld place */
    p4pd_copy_be_src_to_le_dest(
//::                            if p4fldwidth <= 32:
                   (uint8_t*)((uint8_t*)&(swkey->${p4fldname}) + ${kbyte}),
//::                            else:
                   &(swkey->${p4fldname}[${kbyte}]),
//::                            #endif
                   (${p4fldwidth} - ${kbit}) % 8, /* Start bit in destination */
                   hwkey,
                   (${tablebyte} * 8) + ${containerstart} - ${mat_key_start_bit}, /* source bit position */
                   ${width});
//::                        #endfor
//::                    #endif
//::                    if len(key_bit_format):
//::                        for kmbit, kbit in key_bit_format:
    /* Copying one bit from table-key into correct place */
    p4pd_copy_single_bit(
//::                            if p4fldwidth <= 32:
                   (uint8_t*)((uint8_t*)&(swkey->${p4fldname}) + (${kbit}/8)),
//::                            else:
                   &(swkey->${p4fldname}[${kbit}/8]),
//::                            #endif
                   (${p4fldwidth} - 1 - ${kbit}) % 8 /* start bit in destination */,
                   hwkey + (${kmbit - mat_key_start_bit} >> 3),
                   ((${kmbit} - (${kmbit} % 8)) + (7 - (${kmbit} % 8)))- (${mat_key_start_bit}), /* Source bit position */
                   1 /* copy single bit */);
//::                        #endfor
//::                    #endif
//::                #endfor

//::                i = 1
//::                for fld_un_containers in pddict['tables'][table]['fldunion_keys']:
//::                    fldcontainer, fldukeys = fld_un_containers
//::                    if len(fldukeys) > 1:
//::                        ustr = table + '_u' + str(i) + '.'
//::                    else:
//::                        ustr=''
//::                    #endif
//::                    for fields in fldukeys:
//::                        (p4fldname, p4fldwidth, mask, key_byte_format, key_bit_format) = fields
//::                        if len(key_byte_format):
//::                            for kmbyte, kbit, width, containerstart in key_byte_format:
//::                                kbyte = (p4fldwidth - 1 - kbit) / 8
//::                                tablebyte = kmbyte
    /* Copying one byte from table-key into correct place */
    p4pd_copy_be_src_to_le_dest(
//::                                if p4fldwidth <= 32:
                   (uint8_t*)((uint8_t*)&(swkey->${ustr}${p4fldname}) + ${kbyte}),
//::                                else:
                   &(swkey->${ustr}${p4fldname}[${kbyte}]),
//::                                #endif
                   (${p4fldwidth} - ${kbit}) % 8, /* Start bit in destination */
                   hwkey,
                   (${tablebyte} * 8) + ${containerstart} - ${mat_key_start_bit}, /* source bit position */
                   ${width});
//::                            #endfor
//::                        #endif
//::                        if len(key_bit_format):
//::                            for kmbit, kbit in key_bit_format:
    /* Copying one bit from table-key into correct place */
    p4pd_copy_single_bit(
//::                                if p4fldwidth <= 32:
                   (uint8_t*)((uint8_t*)&(swkey->${ustr}${p4fldname}) + (${kbit}/8)),
//::                                else:
                   &(swkey->${ustr}${p4fldname}[${kbit}/8]),
//::                                #endif
                   (${p4fldwidth} - 1 - ${kbit}) % 8 /* start bit in destination */,
                   hwkey + (${kmbit - mat_key_start_bit} >> 3),
                   ((${kmbit} - (${kmbit} % 8)) + (7 - (${kmbit} % 8)))- (${mat_key_start_bit}), /* Source bit position */
                   1 /* copy single bit */);
//::                            #endfor
//::                        #endif
//::                    #endfor
//::                    i += 1
//::                #endfor

//::                i = 1
//::                for hdr_un_containers in pddict['tables'][table]['hdrunion_keys']:
//::                    hdrcontainer, hdrukeys = hdr_un_containers
//::                    if len(fldukeys) > 1:
//::                        ustr = table + '_hdr_u' + str(i) + '.'
//::                    else:
//::                        ustr=''
//::                    #endif
//::                    for fields in hdrukeys:
//::                        (p4fldname, p4fldwidth, mask, key_byte_format, key_bit_format) = fields
//::                        if len(key_byte_format):
//::                            for kmbyte, kbit, width, containerstart in key_byte_format:
//::                                kbyte = (p4fldwidth - 1 - kbit) / 8
//::                                tablebyte = kmbyte
    /* Copying one byte from table-key into correct place */
    p4pd_copy_be_src_to_le_dest(
//::                                if p4fldwidth <= 32:
                   (uint8_t*)((uint8_t*)&(swkey->${ustr}${p4fldname}) + ${kbyte}),
//::                                else:
                   &(swkey->${ustr}${p4fldname}[${kbyte}]),
//::                                #endif
                   (${p4fldwidth} - ${kbit}) % 8, /* Start bit in destination */
                   hwkey,
                   (${tablebyte} * 8) + ${containerstart} - ${mat_key_start_bit}, /* source bit position */
                   ${width});
//::                            #endfor
//::                        #endif
//::                        if len(key_bit_format):
//::                            for kmbit, kbit in key_bit_format:
    /* Copying one bit from table-key into correct place */
    p4pd_copy_single_bit(
//::                                if p4fldwidth <= 32:
                   (uint8_t*)((uint8_t*)&(swkey->${ustr}${p4fldname}) + (${kbit}/8)),
//::                                else:
                   &(swkey->${ustr}${p4fldname}[${kbit}/8]),
//::                                #endif
                   (${p4fldwidth} - 1 - ${kbit}) % 8 /* start bit in destination */,
                   hwkey + (${kmbit - mat_key_start_bit} >> 3),
                   ((${kmbit} - (${kmbit} % 8)) + (7 - (${kmbit} % 8)))- (${mat_key_start_bit}), /* Source bit position */
                   1 /* copy single bit */);
//::                            #endfor
//::                        #endif
//::                    #endfor
//::                    i += 1
//::                #endfor
    return (hwkey_len);
}
//::            #endif
//::        #endif


/* Read hardware entry from P4-table '${table}'. Read hw entry is decoded
 * and used to fill up matchkey and actiondata structures.
 *
 * Arguments: 
 *  IN   : uint32_t tableid                 : Table Id that identifies
 *                                            P4 table. This id is obtained
 *                                            from p4pd_table_id_enum.
 *
//::        if pddict['tables'][table]['type'] == 'Ternary':
 *
 *  IN   : uint32_t index                   : TCAM table index where entry is 
 *                                            installed. 
 *                                            Caller of the API is expected to
 *                                            provide this index based on
 *                                            placement decision made for the key.
//::        elif pddict['tables'][table]['type'] == 'Index':
 *  IN   : uint32_t index                   : Direct/Indexed table index where
 *                                            table entry is read from.
//::        else:
 *
 *  IN   : uint32_t hashindex               : Hash index where table entry is 
 *                                            read from.
//::        #endif
//::        if pddict['tables'][table]['type'] != 'Index':
 *  OUT  : ${table}_swkey_t *swkey          : Software key read from P4-table
//::        #endif
//::        if pddict['tables'][table]['type'] == 'Ternary':
 *  OUT  : ${table}_swkey_mask_t *swkey_mask: Software key read from P4-table
//::        #endif
 *  OUT  : ${table}_actions_un* *actiondata : Action data associated with the key.
 * 
 * Return Value: 
 *  pd_error_t                              : P4PD_SUCCESS / P4PD_FAIL
 */
//::        if pddict['tables'][table]['type'] == 'Ternary':
static p4pd_error_t
${table}_entry_read(uint32_t tableid,
                    uint32_t index,
                    ${table}_swkey_t *swkey, 
                    ${table}_swkey_mask_t *swkey_mask,
                    ${table}_actiondata *actiondata)
{
    uint8_t  hwentry_x[P4PD_MAX_MATCHKEY_LEN + P4PD_MAX_ACTION_DATA_LEN] = {0};
    uint8_t  hwentry_y[P4PD_MAX_MATCHKEY_LEN + P4PD_MAX_ACTION_DATA_LEN] = {0};
    uint8_t  hwentry[P4PD_MAX_MATCHKEY_LEN + P4PD_MAX_ACTION_DATA_LEN] = {0};
    uint16_t hwentry_bit_len;

    capri_tcam_table_entry_read(tableid, index, hwentry_x, hwentry_y,
                                &hwentry_bit_len);
    if (!hwentry_bit_len) {
        return (P4PD_FAIL);
    }
    int pad = (hwentry_bit_len % 16) ? (16 - (hwentry_bit_len % 16)) : 0;
    p4pd_swizzle_bytes(hwentry_x, hwentry_bit_len + pad);
    p4pd_swizzle_bytes(hwentry_y, hwentry_bit_len + pad);
    ${table}_hwkey_hwmask_unbuild(tableid, hwentry_x, hwentry_y, hwentry_bit_len,
                                  swkey, swkey_mask);
    capri_table_entry_read(tableid, index, hwentry, &hwentry_bit_len);
    if (!hwentry_bit_len) {
        return (P4PD_SUCCESS);
    }
    pad = (hwentry_bit_len % 16) ? (16 - (hwentry_bit_len % 16)) : 0;
    p4pd_swizzle_bytes(hwentry, hwentry_bit_len+pad);
//::            if len(pddict['tables'][table]['actions']) > 1:
//::                action_pc_added = True
//::            else:
//::                action_pc_added = False
//::            #endif
//::            if action_pc_added:
    uint8_t actionpc = hwentry[0]; // First byte is always action-pc
    actiondata->actionid = capri_get_action_id(tableid, actionpc);
    int adatabyte = 1;
//::            else:
    actiondata->actionid = 0;
    int adatabyte = 0;
//::            #endif
    ${table}_unpack_action_data(tableid, actiondata->actionid,
                                hwentry + adatabyte, actiondata);
    return (P4PD_SUCCESS);
}
//::        elif pddict['tables'][table]['type'] == 'Index' or pddict['tables'][table]['type'] == 'Mpu':
static p4pd_error_t
${table}_entry_read(uint32_t tableid,
                    uint32_t index, 
                    ${table}_actiondata* actiondata)
{
    uint8_t  hwentry[P4PD_MAX_MATCHKEY_LEN + P4PD_MAX_ACTION_DATA_LEN] = {0};
    uint16_t hwentry_bit_len;
    uint8_t *packed_actiondata_after_key;
    uint16_t actiondata_len_after_key, key_bit_len;
    
    (void)packed_actiondata_after_key;
    (void)actiondata_len_after_key;
    (void)key_bit_len;

//::            if pddict['tables'][table]['location'] == 'HBM':
    capri_hbm_table_entry_read(tableid, index, hwentry, &hwentry_bit_len);
//::            else:
    capri_table_entry_read(tableid, index, hwentry, &hwentry_bit_len);
    p4pd_swizzle_bytes(hwentry, hwentry_bit_len);
//::            #endif

    if (!hwentry_bit_len) {
        // Zero len!!
        return (P4PD_SUCCESS);
    }
//::            if len(pddict['tables'][table]['actions']) > 1:
//::                action_pc_added = True
//::            else:
//::                action_pc_added = False
//::            #endif
//::            if action_pc_added:
    uint8_t actionpc = hwentry[0]; // First byte is always action-pc
    actiondata->actionid = capri_get_action_id(tableid, actionpc);
    int adatabyte = 1;
//::            else:
    actiondata->actionid = 0;
    int adatabyte = 0;
//::            #endif
    ${table}_unpack_action_data(tableid, actiondata->actionid,
                                hwentry + adatabyte, actiondata);
                       
    return (P4PD_SUCCESS);
}
//::        else:
static p4pd_error_t
${table}_entry_read(uint32_t tableid,
                    uint32_t hashindex, 
                    ${table}_swkey_t *swkey, 
                    ${table}_actiondata *actiondata)
{
    uint8_t  hwentry[P4PD_MAX_MATCHKEY_LEN + P4PD_MAX_ACTION_DATA_LEN] = {0};
    uint16_t hwentry_bit_len;
    uint8_t  *packed_actiondata_before_key;
    uint8_t *packed_actiondata_after_key;
    uint16_t actiondata_len_before_key;
    uint16_t actiondata_len_after_key, key_bit_len;

    (void)key_bit_len;
    
//::            if pddict['tables'][table]['location'] == 'HBM':
    capri_hbm_table_entry_read(tableid, hashindex, hwentry, &hwentry_bit_len);
//::            else:
    capri_table_entry_read(tableid, hashindex, hwentry, &hwentry_bit_len);
    p4pd_swizzle_bytes(hwentry, hwentry_bit_len);
//::            #endif

    if (!hwentry_bit_len) {
        // Zero len!!
        return (P4PD_SUCCESS);
    }
//::            mat_key_start_byte = pddict['tables'][table]['match_key_start_byte']
//::            mat_key_start_bit = pddict['tables'][table]['match_key_start_bit']
//::            mat_key_bit_length = pddict['tables'][table]['match_key_bit_length']

    ${table}_hwkey_unbuild(tableid, hwentry, 
                           ${mat_key_bit_length}, swkey);

//::            if len(pddict['tables'][table]['actions']) > 1:
//::                action_pc_added = True
//::            else:
//::                action_pc_added = False
//::            #endif
//::            if action_pc_added:
    uint8_t actionpc = hwentry[0]; // First byte is always action-pc
    actiondata->actionid = capri_get_action_id(tableid, actionpc);
    int adatabyte = 1;
//::            else:
    actiondata->actionid = 0;
    int adatabyte = 0;
//::            #endif
    // Since actionpc is not in KM, when unbuilding key into p4fld,
    // pass pointer to hwentry byte stream after action-pc
    packed_actiondata_before_key = (hwentry + adatabyte);
    packed_actiondata_after_key = (hwentry + ${mat_key_start_byte} + (${mat_key_bit_length} >> 3));
    actiondata_len_before_key = (${mat_key_start_byte} - adatabyte) * 8;
    actiondata_len_after_key = ${max_actionfld_len} - actiondata_len_before_key;
    hash_${table}_unpack_action_data(tableid,
                                    actiondata->actionid,
                                    packed_actiondata_before_key,
                                    actiondata_len_before_key,
                                    packed_actiondata_after_key,
                                    actiondata_len_after_key,
                                    actiondata);

    return (P4PD_SUCCESS);
}
//::        #endif

/* Decode hardware entry from P4-table '${table}'.
 *
 * Arguments: 
 *  IN   : uint32_t tableid                 : Table Id that identifies
 *                                            P4 table. This id is obtained
 *                                            from p4pd_table_id_enum.
 *
//::        if pddict['tables'][table]['type'] != 'Index':
 *  OUT  : ${table}_swkey_t *swkey          : Software key read from P4-table
//::        #endif
//::        if pddict['tables'][table]['type'] == 'Ternary':
 *  OUT  : ${table}_swkey_mask_t *swkey_mask: Software key read from P4-table
//::        #endif
 *  OUT  : ${table}_actions_un* *actiondata : Action data associated with the key.
 * 
 * Return Value: 
 *  pd_error_t                              : P4PD_SUCCESS / P4PD_FAIL
 */
//::        if pddict['tables'][table]['type'] == 'Ternary':
static p4pd_error_t
${table}_key_decode(uint32_t tableid,
                    uint8_t *hwentry_x,
                    uint8_t *hwentry_y,
                    uint16_t hwentry_len,
                    ${table}_swkey_t *swkey, 
                    ${table}_swkey_mask_t *swkey_mask)
{

    ${table}_hwkey_hwmask_unbuild(tableid, hwentry_x, hwentry_y, hwentry_len,
                                  swkey, swkey_mask);
    return (P4PD_SUCCESS);
}
//::        elif pddict['tables'][table]['type'] == 'Index' or pddict['tables'][table]['type'] == 'Mpu':
static p4pd_error_t
${table}_entry_decode(uint32_t tableid,
                      uint8_t *hwentry,
                      uint16_t hwentry_len,
                      ${table}_actiondata* actiondata)
{
//::            if len(pddict['tables'][table]['actions']) > 1:
//::                action_pc_added = True
//::            else:
//::                action_pc_added = False
//::            #endif
//::            if action_pc_added:
    uint8_t actionpc = hwentry[0]; // First byte is always action-pc
    actiondata->actionid = capri_get_action_id(tableid, actionpc);
    int adatabyte = 1;
//::            else:
    actiondata->actionid = 0;
    int adatabyte = 0;
//::            #endif

    ${table}_unpack_action_data(tableid, actiondata->actionid,
                                hwentry + adatabyte, actiondata);
                       
    return (P4PD_SUCCESS);
}
//::        else:
static p4pd_error_t
${table}_entry_decode(uint32_t tableid,
                      uint8_t *hwentry,
                      uint16_t hwentry_len,
                      ${table}_swkey_t *swkey, 
                      ${table}_actiondata *actiondata)
{
    // Since actionpc is not in KM, when unbuilding key into p4fld,
    // pass pointer to hwentry byte stream after action-pc
    ${table}_hwkey_unbuild(tableid, hwentry, hwentry_len, swkey);
//::            if len(pddict['tables'][table]['actions']) > 1:
//::                action_pc_added = True
//::            else:
//::                action_pc_added = False
//::            #endif
//::            if action_pc_added:
    uint8_t actionpc = hwentry[0]; // First byte is always action-pc
    actiondata->actionid = capri_get_action_id(tableid, actionpc);
    int adatabyte = 1;
//::            else:
    actiondata->actionid = 0;
    int adatabyte = 0;
//::            #endif
//::            mat_key_start_byte = pddict['tables'][table]['match_key_start_byte']
//::            mat_key_bit_length = pddict['tables'][table]['match_key_bit_length']
    uint8_t *packed_actiondata_before_key;
    uint8_t *packed_actiondata_after_key;
    uint16_t actiondata_len_before_key;
    uint16_t actiondata_len_after_key;
    packed_actiondata_before_key = (hwentry + adatabyte);
    packed_actiondata_after_key = (hwentry + ${mat_key_start_byte} + (${mat_key_bit_length} >> 3));
    actiondata_len_before_key = (${mat_key_start_byte} - adatabyte) * 8; // bit len without actionpc
    actiondata_len_after_key = ${max_actionfld_len} - actiondata_len_before_key;
    hash_${table}_unpack_action_data(tableid,
                                    actiondata->actionid,
                                    packed_actiondata_before_key,
                                    actiondata_len_before_key,
                                    packed_actiondata_after_key,
                                    actiondata_len_after_key,
                                    actiondata);
    return (P4PD_SUCCESS);
}
//::        #endif

//::     #endfor


//::    if len(tabledict):

//::        if pddict['p4plus']:
//::            api_prefix = 'p4pd_' + pddict['p4program']
//::        else:
//::            api_prefix = 'p4pd'
//::        #endif


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
void 
${api_prefix}_hwentry_query(uint32_t tableid, 
                            uint32_t *hwkey_len, 
                            uint32_t *hwkeymask_len, 
                            uint32_t *hwactiondata_len)
{
    switch (tableid) {
//::        for table, tid in tabledict.items():
//::            caps_tablename = table.upper()
//::            if pddict['tables'][table]['hash_overflow'] and not pddict['tables'][table]['otcam']:
//::                continue
//::            #endif
        case P4${caps_p4prog}TBL_ID_${caps_tablename}: /* p4-table '${table}' */
//::            if len(pddict['tables'][table]['hash_overflow_tbl']):
//::                tbl_ = pddict['tables'][table]['hash_overflow_tbl']
//::                caps_tbl_ = tbl_.upper()
        case P4${caps_p4prog}TBL_ID_${caps_tbl_}: /* p4-table '${tbl_}' */
//::            #endif
//::            if pddict['tables'][table]['type'] == 'Index' or pddict['tables'][table]['type'] == 'Mpu':
            (void)hwkey_len;
            (void)hwkeymask_len;
            ${table}_hwentry_query(tableid, hwactiondata_len);
//::            #endif
//::            if pddict['tables'][table]['type'] == 'Hash' or pddict['tables'][table]['type'] == 'Hash_OTcam':
            (void)hwkeymask_len;
            ${table}_hwentry_query(tableid, hwkey_len, hwactiondata_len);
//::            #endif
//::            if pddict['tables'][table]['type'] == 'Ternary':
            ${table}_hwentry_query(tableid, hwkey_len, hwkeymask_len, hwactiondata_len);
//::            #endif
        break;

//::        #endfor
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
p4pd_error_t
${api_prefix}_hwkey_hwmask_build(uint32_t   tableid,
                                 void       *swkey, 
                                 void       *swkey_mask, 
                                 uint8_t    *hw_key, 
                                 uint8_t    *hw_key_y)
{

    switch (tableid) {
//::        for table, tid in tabledict.items():
//::            caps_tablename = table.upper()
//::            if pddict['tables'][table]['hash_overflow'] and not pddict['tables'][table]['otcam']:
//::                continue
//::            #endif
        case P4${caps_p4prog}TBL_ID_${caps_tablename}: /* p4-table '${table}' */
//::            if len(pddict['tables'][table]['hash_overflow_tbl']):
//::                tbl_ = pddict['tables'][table]['hash_overflow_tbl']
//::                caps_tbl_ = tbl_.upper()
        case P4${caps_p4prog}TBL_ID_${caps_tbl_}: /* p4-table '${tbl_}' */
//::            #endif
//::            if pddict['tables'][table]['type'] == 'Index' or pddict['tables'][table]['type'] == 'Mpu':
            return (P4PD_SUCCESS); /* No hardwre key for index based lookup tables. */
//::            #endif
//::            if pddict['tables'][table]['type'] == 'Hash' or pddict['tables'][table]['type'] == 'Hash_OTcam':
            return (${table}_hwkey_build(tableid, (${table}_swkey_t *)swkey, hw_key) > 0 ? P4PD_SUCCESS : P4PD_FAIL);
//::            #endif
//::            if pddict['tables'][table]['type'] == 'Ternary':
            return (${table}_hwkey_hwmask_build(tableid, (${table}_swkey_t *)swkey, 
                                                (${table}_swkey_mask_t *)swkey_mask,
                                                hw_key, hw_key_y) > 0 ? P4PD_SUCCESS : P4PD_FAIL);
//::            #endif
        break;

//::        #endfor
        default:
            // Invalid tableid
            return (P4PD_FAIL);
        break;
    }
    return (P4PD_FAIL);
}

/* 
 * Build index value that pipeline uses to lookup 
 * p4-table (index based lookup tables). The returned index
 * is where the table entry should be installed into hardware
 * table using p4pd_entry_write() API
 *
 * Arguments: 
 *
 *  IN  : uint32_t tableid      : Table Id that identifies
 *                                P4 table. This id is obtained
 *                                from p4pd_table_id_enum.
 * 
 *  IN  : void *swkey           : Software key structure containing all p4-fields
 *                                that form table index.
 * 
 * Return Value 
 *  uint64_t                   : hw_index
 */
uint64_t
${api_prefix}_index_to_hwindex_map(uint32_t   tableid,
                                   void       *swkey)
{

    switch (tableid) {
//::        for table, tid in tabledict.items():
//::            caps_tablename = table.upper()
//::            if pddict['tables'][table]['type'] != 'Index':
//::                continue
//::            #endif
        case P4${caps_p4prog}TBL_ID_${caps_tablename}: /* p4-table '${table}' */
            return (${table}_index_mapper(tableid, (${table}_swkey_t *)swkey));
        break;

//::        #endfor
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
p4pd_error_t
${api_prefix}_entry_write(uint32_t tableid,
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
     *  Match-Key   : is byte stream built using ${table}_hwkey_build()
     *  Action-Data : ${table}_actiondata* actiondata is
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
     *  Match-Key   : is byte stream built using ${table}_hwkey_build()
     *  Action-Data : ${table}_actiondata* actiondata is
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
     * Action-Data : ${table}_actiondata* actiondata is
     *               bit packed and installed in HW memory.
     *               Action Field order in Action-data should match
     *               with ASM output that MPU uses to fetch action-data
     */

    switch (tableid) {
//::        for table, tid in tabledict.items():
//::            caps_tablename = table.upper()
//::            if pddict['tables'][table]['hash_overflow'] and not pddict['tables'][table]['otcam']:
//::                continue
//::            #endif
        case P4${caps_p4prog}TBL_ID_${caps_tablename}: /* p4-table '${table}' */
//::            if len(pddict['tables'][table]['hash_overflow_tbl']):
//::                tbl_ = pddict['tables'][table]['hash_overflow_tbl']
//::                caps_tbl_ = tbl_.upper()
        case P4${caps_p4prog}TBL_ID_${caps_tbl_}: /* p4-table '${tbl_}' */
//::            #endif
//::            if pddict['tables'][table]['type'] == 'Index' or pddict['tables'][table]['type'] == 'Mpu':
            return (${table}_entry_write(tableid, index, 
                                         (${table}_actiondata*)actiondata));
//::            #endif
//::            if pddict['tables'][table]['type'] == 'Hash' or pddict['tables'][table]['type'] == 'Hash_OTcam':
            return (${table}_entry_write(tableid, index, hwkey, 
                                         (${table}_actiondata*)actiondata));
//::            #endif
//::            if pddict['tables'][table]['type'] == 'Ternary':
            return (${table}_entry_write(tableid, index,
                                         hwkey, hwkey_y,
                                         (${table}_actiondata*)actiondata));
//::            #endif
        break;

//::        #endfor
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
p4pd_error_t
${api_prefix}_entry_read(uint32_t   tableid,
                         uint32_t   index,
                         void       *swkey, 
                         void       *swkey_mask,
                         void       *actiondata)
{
    switch (tableid) {
//::        for table, tid in tabledict.items():
//::            caps_tablename = table.upper()
//::            if pddict['tables'][table]['hash_overflow'] and not pddict['tables'][table]['otcam']:
//::                continue
//::            #endif
        case P4${caps_p4prog}TBL_ID_${caps_tablename}: /* p4-table '${table}' */
//::            if len(pddict['tables'][table]['hash_overflow_tbl']):
//::                tbl_ = pddict['tables'][table]['hash_overflow_tbl']
//::                caps_tbl_ = tbl_.upper()
        case P4${caps_p4prog}TBL_ID_${caps_tbl_}: /* p4-table '${tbl_}' */
//::            #endif
//::            if pddict['tables'][table]['type'] == 'Index' or pddict['tables'][table]['type'] == 'Mpu':
            return (${table}_entry_read(tableid, index, 
                            (${table}_actiondata*) actiondata));
//::            #endif
//::            if pddict['tables'][table]['type'] == 'Hash' or pddict['tables'][table]['type'] == 'Hash_OTcam':
            return (${table}_entry_read(tableid, index, 
                            (${table}_swkey_t *)swkey, 
                            (${table}_actiondata*)actiondata));
//::            #endif
//::            if pddict['tables'][table]['type'] == 'Ternary':
            return (${table}_entry_read(tableid, index,
                            (${table}_swkey_t*)swkey, 
                            (${table}_swkey_mask_t*)swkey_mask,
                            (${table}_actiondata*) actiondata));
//::            #endif
        break;

//::        #endfor
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
p4pd_error_t
${api_prefix}_table_entry_decoded_string_get(uint32_t   tableid,
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

    if (hwentry_y) {
        memcpy(_hwentry_y, hwentry_y, (hwentry_len + 7) >> 3);
        p4pd_swizzle_bytes(_hwentry_y, hwentry_len);
    }

    memset(buffer, 0, buf_len);
    if (hwentry_y) {
        b = snprintf(buf, blen, "!!!! Decode HW table entry into Table Key !!!! \n");
    } else {
        b = snprintf(buf, blen, "!!!! Decode HW table entry into (Table Key + Actiondata)/Actiondata!!!! \n");
    }
    buf += b;
    blen -= b;
    if (blen <= 0) {
        return (P4PD_SUCCESS);
    }
    switch (tableid) {
//::        for table, tid in tabledict.items():
//::            caps_tablename = table.upper()
//::            if pddict['tables'][table]['hash_overflow'] and not pddict['tables'][table]['otcam']:
//::                continue
//::            #endif
        case P4${caps_p4prog}TBL_ID_${caps_tablename}: /* p4-table '${table}' */
//::            if len(pddict['tables'][table]['hash_overflow_tbl']):
//::                tbl_ = pddict['tables'][table]['hash_overflow_tbl']
//::                caps_tbl_ = tbl_.upper()
        case P4${caps_p4prog}TBL_ID_${caps_tbl_}: /* p4-table '${tbl_}' */
//::            #endif
        {
            memcpy(_hwentry, hwentry, (hwentry_len + 7) >> 3);
//::        if pddict['tables'][table]['location'] != 'HBM':
            p4pd_swizzle_bytes(_hwentry, hwentry_len);
//::        #endif
            b = snprintf(buf, blen, "Table: %s, Index %d\n", "P4TBL_ID_${caps_tablename}", index);
            buf += b;
            blen -= b;
            if (blen <= 0) {
                return (P4PD_SUCCESS);
            }
//::            if pddict['tables'][table]['type'] == 'Index' or pddict['tables'][table]['type'] == 'Mpu':
            ${table}_actiondata actiondata;
            ${table}_entry_decode(tableid, _hwentry,  hwentry_len,
                                  &actiondata);
            switch(actiondata.actionid) {
//::                for action in pddict['tables'][table]['actions']:
//::                    (actionname, actionfldlist) = action
//::                    actname = actionname.upper()
                case ${caps_tablename}_${actname}_ID:
                {
                    b = snprintf(buf, blen, "Action: %s\n", "${caps_tablename}_${actname}_ID");
                    buf += b;
                    blen -= b;
                    if (blen <= 0) {
                        return (P4PD_SUCCESS);
                    }
//::                    for actionfld in actionfldlist:
//::                        actionfldname, actionfldwidth = actionfld
//::                        if actionfldwidth <= 32:
                    b = snprintf(buf, blen, "%s: 0x%x\n", "${actionfldname}",
                             actiondata.${table}_action_u.\
                             ${table}_${actionname}.${actionfldname});
                    buf += b;
                    blen -= b;
                    if (blen <= 0) {
                        return (P4PD_SUCCESS);
                    }
//::                        else:
//::                            p4fldwidth_byte = (actionfldwidth / 8) + (1 if actionfldwidth % 8 else 0)
                    b = snprintf(buf, blen, "%s: \n", "${actionfldname}");
                    buf += b;
                    blen -= b;
                    if (blen <= 0) {
                        return (P4PD_SUCCESS);
                    }
                    for (int j = 0; j < ${p4fldwidth_byte}; j++) {
                        b = snprintf(buf, blen, "  0x%x : ", 
                                 actiondata.${table}_action_u.\
                                 ${table}_${actionname}.${actionfldname}[j]);
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
//::                        #endif
//::                    #endfor
                }
                break;
//::                #endfor
            }
//::            #endif
//::            if pddict['tables'][table]['type'] == 'Hash' or pddict['tables'][table]['type'] == 'Hash_OTcam':
            ${table}_swkey_t swkey;
            ${table}_actiondata actiondata;
            ${table}_entry_decode(tableid, _hwentry, hwentry_len, 
                                  &swkey, 
                                  &actiondata);
//::                for fields in pddict['tables'][table]['keys']:
//::                    (p4fldname, p4fldwidth, mask, key_byte_format, key_bit_format) = fields
//::                    if (p4fldwidth <= 32):
            b = snprintf(buf, blen, "%s: 0x%x\n", "${p4fldname}", swkey.${p4fldname});
            buf += b;
            blen -= b;
            if (blen <= 0) {
                return (P4PD_SUCCESS);
            }
//::                    else:
//::                        p4fldwidth_byte = (p4fldwidth / 8) + (1 if p4fldwidth % 8 else 0)
            b = snprintf(buf, blen, "%s: \n", "${p4fldname}");
            buf += b;
            blen -= b;
            if (blen <= 0) {
                return (P4PD_SUCCESS);
            }
            for (int j = 0; j < ${p4fldwidth_byte}; j++) {
                b = snprintf(buf, blen, "0x%x : ", swkey.${p4fldname}[j]);
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
//::                    #endif
//::                #endfor
//::                i = 1
//::                for fld_un_containers in pddict['tables'][table]['fldunion_keys']:
//::                    fldcontainer, fldukeys = fld_un_containers
//::                    if len(fldukeys) > 1:
//::                        ustr = table + '_u' + str(i) + '.'
//::                    else:
//::                        ustr=''
//::                    #endif
//::                    for fields in fldukeys:
//::                        (p4fldname, p4fldwidth, mask, key_byte_format, key_bit_format) = fields
//::                        if len(key_byte_format):
//::                            for kmbyte, kbit in key_byte_format:
//::                                kbyte = kbit / 8
//::                                tablebyte = kmbyte
//::                                if p4fldwidth <= 32:
            b = snprintf(buf, blen, "%s: 0x%x\n", "${p4fldname}",
                        (swkey.${ustr}${p4fldname}));
//::                                else:
            b = snprintf(buf, blen, "%s: \n", "${p4fldname}");
            buf += b;
            blen -= b;
            if (blen <= 0) {
                return (P4PD_SUCCESS);
            }
//::                                    p4fldwidth_byte = (p4fldwidth / 8) + (1 if p4fldwidth % 8 else 0)
            for (int j = 0; j < ${p4fldwidth_byte}; j++) {
                b = snprintf(buf, blen, "  0x%x : ", swkey.${ustr}${p4fldname}[j]);
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
//::                                #endif
//::                            #endfor
//::                        #endif
//::                    #endfor
//::                    i += 1
//::                #endfor
//::                i = 1
//::                for hdr_un_containers in pddict['tables'][table]['hdrunion_keys']:
//::                    hdrcontainer, hdrukeys = hdr_un_containers
//::                    if len(fldukeys) > 1:
//::                        ustr = table + '_hdr_u' + str(i) + '.'
//::                    else:
//::                        ustr=''
//::                    #endif
//::                    for fields in hdrukeys:
//::                        (p4fldname, p4fldwidth, mask, key_byte_format, key_bit_format) = fields
//::                        if len(key_byte_format):
//::                            for kmbyte, kbit in key_byte_format:
//::                                kbyte = kbit / 8
//::                                tablebyte = kmbyte
//::                                if p4fldwidth <= 32:
            b = snprintf(buf, blen, "%s: 0x%x\n", "${p4fldname}",
                         (swkey.${ustr}${p4fldname}));
//::                                else:
//::                                    p4fldwidth_byte = (p4fldwidth / 8) + (1 if p4fldwidth % 8 else 0)
            b = snprintf(buf, blen, "%s: \n", "${p4fldname}");
            buf += b;
            blen -= b;
            if (blen <= 0) {
                return (P4PD_SUCCESS);
            }
            for (int j = 0; j < ${p4fldwidth_byte}; j++) {
                b = snprintf(buf, blen, "  0x%x : ", swkey.${ustr}${p4fldname}[j]);
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
//::                                #endif
//::                            #endfor
//::                        #endif
//::                    #endfor
//::                    i += 1
//::                #endfor
            switch(actiondata.actionid) {
//::                for action in pddict['tables'][table]['actions']:
//::                    (actionname, actionfldlist) = action
//::                    actname = actionname.upper()
                case ${caps_tablename}_${actname}_ID:
                {
                    b = snprintf(buf, blen, "Action: %s\n", "${caps_tablename}_${actname}_ID");
                    buf += b;
                    blen -= b;
                    if (blen <= 0) {
                        return (P4PD_SUCCESS);
                    }
//::                    for actionfld in actionfldlist:
//::                        actionfldname, actionfldwidth = actionfld
//::                        if actionfldwidth <= 32:
                    b = snprintf(buf, blen, "%s: 0x%x\n", "${actionfldname}",
                             actiondata.${table}_action_u.\
                             ${table}_${actionname}.${actionfldname});
                    buf += b;
                    blen -= b;
                    if (blen <= 0) {
                        return (P4PD_SUCCESS);
                    }
//::                        else:
//::                            p4fldwidth_byte = (actionfldwidth / 8) + (1 if actionfldwidth % 8 else 0)
                    b = snprintf(buf, blen, "%s: \n", "${actionfldname}");
                    buf += b;
                    blen -= b;
                    if (blen <= 0) {
                        return (P4PD_SUCCESS);
                    }
                    for (int j = 0; j < ${p4fldwidth_byte}; j++) {
                        b = snprintf(buf, blen, "  0x%x : ", 
                                 actiondata.${table}_action_u.\
                                 ${table}_${actionname}.${actionfldname}[j]);
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
//::                        #endif
//::                    #endfor
                }
                break;
//::                #endfor
            }
//::            #endif
//::            if pddict['tables'][table]['type'] == 'Ternary':
            if (hwentry_y) { /* mask is not null. hence decode tcam key else actiondata */
                ${table}_swkey_t swkey;
                ${table}_swkey_mask_t swkey_mask;
                ${table}_key_decode(tableid, _hwentry, 
                                  _hwentry_y, hwentry_len,
                                  &swkey, 
                                  &swkey_mask);
//::                for fields in pddict['tables'][table]['keys']:
//::                    (p4fldname, p4fldwidth, mask, key_byte_format, key_bit_format) = fields
//::                    if (p4fldwidth <= 32):
                b = snprintf(buf, blen, "%s: 0x%x\n", "${p4fldname}", swkey.${p4fldname});
                buf += b;
                blen -= b;
                if (blen <= 0) {
                    return (P4PD_SUCCESS);
                }
                b = snprintf(buf, blen, "%s: 0x%x\n", "${p4fldname}_mask", swkey_mask.${p4fldname}_mask);
                buf += b;
                blen -= b;
                if (blen <= 0) {
                    return (P4PD_SUCCESS);
                }
//::                    else:
//::                        p4fldwidth_byte = (p4fldwidth / 8) + (1 if p4fldwidth % 8 else 0)
                b = snprintf(buf, blen, "%s: \n", "${p4fldname}");
                buf += b;
                blen -= b;
                if (blen <= 0) {
                    return (P4PD_SUCCESS);
                }
                for (int j = 0; j < ${p4fldwidth_byte}; j++) {
                    b = snprintf(buf, blen, "0x%x : ", swkey.${p4fldname}[j]);
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
                b = snprintf(buf, blen, "%s: \n", "${p4fldname}_mask");
                buf += b;
                blen -= b;
                if (blen <= 0) {
                    return (P4PD_SUCCESS);
                }
                for (int j = 0; j < ${p4fldwidth_byte}; j++) {
                    b = snprintf(buf, blen, "0x%x : ", swkey_mask.${p4fldname}_mask[j]);
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
//::                    #endif
//::                #endfor
//::                for fld_un_containers in pddict['tables'][table]['fldunion_keys']:
//::                    fldcontainer, fldukeys = fld_un_containers
//::                    if len(fldukeys) > 1:
//::                        ustr = table + '_u' + str(i) + '.'
//::                        ustr_mask = table + '_mask_u' + str(i) + '.'
//::                    else:
//::                        ustr=''
//::                        ustr_mask=''
//::                    #endif
//::                    for fields in fldukeys:
//::                        (p4fldname, p4fldwidth, mask, key_byte_format, key_bit_format) = fields

//::                        (p4fldname, p4fldwidth, mask, key_byte_format, key_bit_format) = fields
//::                        if (p4fldwidth <= 32):
                b = snprintf(buf, blen, "%s: 0x%x\n", "${p4fldname}", swkey.${ustr}${p4fldname});
                buf += b;
                blen -= b;
                if (blen <= 0) {
                    return (P4PD_SUCCESS);
                }
                b = snprintf(buf, blen, "%s: 0x%x\n", "${p4fldname}_mask", swkey_mask.${ustr_mask}${p4fldname}_mask);
                buf += b;
                blen -= b;
                if (blen <= 0) {
                    return (P4PD_SUCCESS);
                }
//::                        else:
//::                            p4fldwidth_byte = (p4fldwidth / 8) + (1 if p4fldwidth % 8 else 0)
                b = snprintf(buf, blen, "%s: \n", "${p4fldname}");
                buf += b;
                blen -= b;
                if (blen <= 0) {
                    return (P4PD_SUCCESS);
                }
                for (int j = 0; j < ${p4fldwidth_byte}; j++) {
                    b = snprintf(buf, blen, "0x%x : ", swkey.${ustr}${p4fldname}[j]);
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
                b = snprintf(buf, blen, "%s: \n", "${p4fldname}_mask");
                buf += b;
                blen -= b;
                if (blen <= 0) {
                    return (P4PD_SUCCESS);
                }
                for (int j = 0; j < ${p4fldwidth_byte}; j++) {
                    b = snprintf(buf, blen, "0x%x : ", swkey_mask.${ustr_mask}${p4fldname}_mask[j]);
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
//::                        #endif
//::                    #endfor
//::                #endfor
//::                for hdr_un_containers in pddict['tables'][table]['hdrunion_keys']:
//::                    hdrcontainer, hdrukeys = hdr_un_containers
//::                    if len(hdrukeys) > 1:
//::                        ustr = table + '_hdr_u' + str(i) + '.'
//::                        ustr_mask = table + '_mask_hdr_u' + str(i) + '.'
//::                    else:
//::                        ustr=''
//::                        ustr_mask=''
//::                    #endif
//::                    for fields in hdrukeys:
//::                        (p4fldname, p4fldwidth, mask, key_byte_format, key_bit_format) = fields
//::                        if (p4fldwidth <= 32):
                b = snprintf(buf, blen, "%s: 0x%x\n", "${p4fldname}", swkey.${ustr}${p4fldname});
                buf += b;
                blen -= b;
                if (blen <= 0) {
                    return (P4PD_SUCCESS);
                }
                b = snprintf(buf, blen, "%s: 0x%x\n", "${p4fldname}_mask", swkey_mask.${ustr_mask}${p4fldname}_mask);
                buf += b;
                blen -= b;
                if (blen <= 0) {
                    return (P4PD_SUCCESS);
                }
//::                        else:
//::                            p4fldwidth_byte = (p4fldwidth / 8) + (1 if p4fldwidth % 8 else 0)
                b = snprintf(buf, blen, "%s: \n", "${p4fldname}");
                buf += b;
                blen -= b;
                if (blen <= 0) {
                    return (P4PD_SUCCESS);
                }
                for (int j = 0; j < ${p4fldwidth_byte}; j++) {
                    b = snprintf(buf, blen, "0x%x : ", swkey.${ustr}${p4fldname}[j]);
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
                b = snprintf(buf, blen, "%s: \n", "${p4fldname}_mask");
                buf += b;
                blen -= b;
                if (blen <= 0) {
                    return (P4PD_SUCCESS);
                }
                for (int j = 0; j < ${p4fldwidth_byte}; j++) {
                    b = snprintf(buf, blen, "0x%x : ", swkey_mask.${ustr_mask}${p4fldname}_mask[j]);
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
//::                        #endif
//::                    #endfor
//::                #endfor
            } else { /* Decode actiondata associated with TCAM key */
                b = snprintf(buf, blen, "!!!! Decode Tcam table Action Data !!!! \n");
                buf += b;
                blen -= b;
                if (blen <= 0) {
                    return (P4PD_SUCCESS);
                }
                ${table}_actiondata actiondata;
                actiondata.actionid = capri_get_action_id(tableid, _hwentry[0]);
                ${table}_unpack_action_data(tableid, actiondata.actionid, _hwentry+1,
                                            &actiondata);
                switch(actiondata.actionid) {
//::                for action in pddict['tables'][table]['actions']:
//::                    (actionname, actionfldlist) = action
//::                    actname = actionname.upper()
                    case ${caps_tablename}_${actname}_ID:
                    {
                        b = snprintf(buf, blen, "Action: %s\n", "${caps_tablename}_${actname}_ID");
                        buf += b;
                        blen -= b;
                        if (blen <= 0) {
                            return (P4PD_SUCCESS);
                        }
//::                    for actionfld in actionfldlist:
//::                        actionfldname, actionfldwidth = actionfld
//::                        if actionfldwidth <= 32:
                        b = snprintf(buf, blen, "%s: 0x%x\n", "${actionfldname}",
                             actiondata.${table}_action_u.\
                             ${table}_${actionname}.${actionfldname});
                        buf += b;
                        blen -= b;
                        if (blen <= 0) {
                            return (P4PD_SUCCESS);
                        }
//::                        else:
//::                            p4fldwidth_byte = (actionfldwidth / 8) + (1 if actionfldwidth % 8 else 0)
                        b = snprintf(buf, blen, "%s: \n", "${actionfldname}");
                        buf += b;
                        blen -= b;
                        if (blen <= 0) {
                            return (P4PD_SUCCESS);
                        }
                        for (int j = 0; j < ${p4fldwidth_byte}; j++) {
                            b = snprintf(buf, blen, "  0x%x : ", 
                                     actiondata.${table}_action_u.\
                                     ${table}_${actionname}.${actionfldname}[j]);
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
//::                        #endif
//::                    #endfor
                    }
                break;
//::                #endfor
                }
            }
//::            #endif
        }
        break;
//::        #endfor
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
p4pd_error_t
${api_prefix}_table_ds_decoded_string_get(uint32_t   tableid,
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

    b = snprintf(buf, blen, "Table Entry Fields\n");

    buf += b;
    blen -= b;
    if (blen <= 0) {
        return (P4PD_SUCCESS);
    }

    switch (tableid) {
//::        for table, tid in tabledict.items():
//::            caps_tablename = table.upper()
//::            if pddict['tables'][table]['hash_overflow'] and not pddict['tables'][table]['otcam']:
//::                continue
//::            #endif
        case P4${caps_p4prog}TBL_ID_${caps_tablename}: /* p4-table '${table}' */
//::            if len(pddict['tables'][table]['hash_overflow_tbl']):
//::                tbl_ = pddict['tables'][table]['hash_overflow_tbl']
//::                caps_tbl_ = tbl_.upper()
        case P4${caps_p4prog}TBL_ID_${caps_tbl_}: /* p4-table '${tbl_}' */
//::            #endif
        {
            b = snprintf(buf, blen, "Table: %s\n", "P4TBL_ID_${caps_tablename}");
            buf += b;
            blen -= b;
            if (blen <= 0) {
                return (P4PD_SUCCESS);
            }
//::            if pddict['tables'][table]['type'] == 'Index' or pddict['tables'][table]['type'] == 'Mpu':
            ${table}_actiondata *actiondata = (${table}_actiondata *)action_data;
            if (!actiondata) {
                break;
            }
            switch(actiondata->actionid) {
//::                for action in pddict['tables'][table]['actions']:
//::                    (actionname, actionfldlist) = action
//::                    actname = actionname.upper()
                case ${caps_tablename}_${actname}_ID:
                {
                    b = snprintf(buf, blen, "Action: %s\n", "${caps_tablename}_${actname}_ID");
                    buf += b;
                    blen -= b;
                    if (blen <= 0) {
                        return (P4PD_SUCCESS);
                    }
//::                    for actionfld in actionfldlist:
//::                        actionfldname, actionfldwidth = actionfld
//::                        if actionfldwidth <= 32:
                    b = snprintf(buf, blen, "%s: 0x%x\n", "${actionfldname}",
                             actiondata->${table}_action_u.\
                             ${table}_${actionname}.${actionfldname});
                    buf += b;
                    blen -= b;
                    if (blen <= 0) {
                        return (P4PD_SUCCESS);
                    }
//::                        else:
//::                            p4fldwidth_byte = (actionfldwidth / 8) + (1 if actionfldwidth % 8 else 0)
                    b = snprintf(buf, blen, "%s: \n", "${actionfldname}");
                    buf += b;
                    blen -= b;
                    if (blen <= 0) {
                        return (P4PD_SUCCESS);
                    }
                    for (int j = 0; j < ${p4fldwidth_byte}; j++) {
                        b = snprintf(buf, blen, "  0x%x : ", 
                                 actiondata->${table}_action_u.\
                                 ${table}_${actionname}.${actionfldname}[j]);
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
//::                        #endif
//::                    #endfor
                }
                break;
//::                #endfor
            }
//::            #endif
//::            if pddict['tables'][table]['type'] == 'Hash' or pddict['tables'][table]['type'] == 'Hash_OTcam':
            ${table}_swkey_t *swkey = (${table}_swkey_t *)sw_key;
            ${table}_actiondata *actiondata = (${table}_actiondata *)action_data;
            if (!swkey) {
                break;
            }
            b = snprintf(buf, blen, "Key:\n");
            buf += b;
            blen -= b;
            if (blen <= 0) {
                return (P4PD_SUCCESS);
            }
//::                for fields in pddict['tables'][table]['keys']:
//::                    (p4fldname, p4fldwidth, mask, key_byte_format, key_bit_format) = fields
//::                    if (p4fldwidth <= 32):
            b = snprintf(buf, blen, "%s: 0x%x\n", "${p4fldname}", swkey->${p4fldname});
            buf += b;
            blen -= b;
            if (blen <= 0) {
                return (P4PD_SUCCESS);
            }
//::                    else:
//::                        p4fldwidth_byte = (p4fldwidth / 8) + (1 if p4fldwidth % 8 else 0)
            b = snprintf(buf, blen, "%s: \n", "${p4fldname}");
            buf += b;
            blen -= b;
            if (blen <= 0) {
                return (P4PD_SUCCESS);
            }
            for (int j = 0; j < ${p4fldwidth_byte}; j++) {
                b = snprintf(buf, blen, "0x%x : ", swkey->${p4fldname}[j]);
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
//::                    #endif
//::                #endfor
//::                i = 1
//::                for fld_un_containers in pddict['tables'][table]['fldunion_keys']:
//::                    fldcontainer, fldukeys = fld_un_containers
//::                    if len(fldukeys) > 1:
//::                        ustr = table + '_u' + str(i) + '.'
//::                    else:
//::                        ustr=''
//::                    #endif
//::                    for fields in fldukeys:
//::                        (p4fldname, p4fldwidth, mask, key_byte_format, key_bit_format) = fields
//::                        if len(key_byte_format):
//::                            for kmbyte, kbit in key_byte_format:
//::                                kbyte = kbit / 8
//::                                tablebyte = kmbyte
//::                                if p4fldwidth <= 32:
            b = snprintf(buf, blen, "%s: 0x%x\n", "${p4fldname}",
                        (swkey->${ustr}${p4fldname}));
//::                                else:
            b = snprintf(buf, blen, "%s: \n", "${p4fldname}");
            buf += b;
            blen -= b;
            if (blen <= 0) {
                return (P4PD_SUCCESS);
            }
//::                                    p4fldwidth_byte = (p4fldwidth / 8) + (1 if p4fldwidth % 8 else 0)
            for (int j = 0; j < ${p4fldwidth_byte}; j++) {
                b = snprintf(buf, blen, "  0x%x : ", swkey->${ustr}${p4fldname}[j]);
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
//::                                #endif
//::                            #endfor
//::                        #endif
//::                    #endfor
//::                    i += 1
//::                #endfor
//::                i = 1
//::                for hdr_un_containers in pddict['tables'][table]['hdrunion_keys']:
//::                    hdrcontainer, hdrukeys = hdr_un_containers
//::                    if len(fldukeys) > 1:
//::                        ustr = table + '_hdr_u' + str(i) + '.'
//::                    else:
//::                        ustr=''
//::                    #endif
//::                    for fields in hdrukeys:
//::                        (p4fldname, p4fldwidth, mask, key_byte_format, key_bit_format) = fields
//::                        if len(key_byte_format):
//::                            for kmbyte, kbit in key_byte_format:
//::                                kbyte = kbit / 8
//::                                tablebyte = kmbyte
//::                                if p4fldwidth <= 32:
            b = snprintf(buf, blen, "%s: 0x%x\n", "${p4fldname}",
                         (swkey->${ustr}${p4fldname}));
//::                                else:
//::                                    p4fldwidth_byte = (p4fldwidth / 8) + (1 if p4fldwidth % 8 else 0)
            b = snprintf(buf, blen, "%s: \n", "${p4fldname}");
            buf += b;
            blen -= b;
            if (blen <= 0) {
                return (P4PD_SUCCESS);
            }
            for (int j = 0; j < ${p4fldwidth_byte}; j++) {
                b = snprintf(buf, blen, "  0x%x : ", swkey->${ustr}${p4fldname}[j]);
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
//::                                #endif
//::                            #endfor
//::                        #endif
//::                    #endfor
//::                    i += 1
//::                #endfor
            if (!actiondata) {
                break;
            }
            switch(actiondata->actionid) {
//::                for action in pddict['tables'][table]['actions']:
//::                    (actionname, actionfldlist) = action
//::                    actname = actionname.upper()
                case ${caps_tablename}_${actname}_ID:
                {
                    b = snprintf(buf, blen, "Action: %s\n", "${caps_tablename}_${actname}_ID");
                    buf += b;
                    blen -= b;
                    if (blen <= 0) {
                        return (P4PD_SUCCESS);
                    }
//::                    for actionfld in actionfldlist:
//::                        actionfldname, actionfldwidth = actionfld
//::                        if actionfldwidth <= 32:
                    b = snprintf(buf, blen, "%s: 0x%x\n", "${actionfldname}",
                             actiondata->${table}_action_u.\
                             ${table}_${actionname}.${actionfldname});
                    buf += b;
                    blen -= b;
                    if (blen <= 0) {
                        return (P4PD_SUCCESS);
                    }
//::                        else:
//::                            p4fldwidth_byte = (actionfldwidth / 8) + (1 if actionfldwidth % 8 else 0)
                    b = snprintf(buf, blen, "%s: \n", "${actionfldname}");
                    buf += b;
                    blen -= b;
                    if (blen <= 0) {
                        return (P4PD_SUCCESS);
                    }
                    for (int j = 0; j < ${p4fldwidth_byte}; j++) {
                        b = snprintf(buf, blen, "  0x%x : ", 
                                 actiondata->${table}_action_u.\
                                 ${table}_${actionname}.${actionfldname}[j]);
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
//::                        #endif
//::                    #endfor
                }
                break;
//::                #endfor
            }
//::            #endif
//::            if pddict['tables'][table]['type'] == 'Ternary':
            ${table}_swkey_t *swkey = (${table}_swkey_t *)sw_key;
            ${table}_swkey_mask_t *swkey_mask = (${table}_swkey_mask_t *)sw_key_mask;
            if (swkey && swkey_mask) {
                b = snprintf(buf, blen, "Key:\n");
                buf += b;
                blen -= b;
                if (blen <= 0) {
                    return (P4PD_SUCCESS);
                }
//::                for fields in pddict['tables'][table]['keys']:
//::                    (p4fldname, p4fldwidth, mask, key_byte_format, key_bit_format) = fields
//::                    if (p4fldwidth <= 32):
                b = snprintf(buf, blen, "%s: 0x%x\n", "${p4fldname}", swkey->${p4fldname});
                buf += b;
                blen -= b;
                if (blen <= 0) {
                    return (P4PD_SUCCESS);
                }
                b = snprintf(buf, blen, "%s: 0x%x\n", "${p4fldname}_mask", swkey_mask->${p4fldname}_mask);
                buf += b;
                blen -= b;
                if (blen <= 0) {
                    return (P4PD_SUCCESS);
                }
//::                    else:
//::                        p4fldwidth_byte = (p4fldwidth / 8) + (1 if p4fldwidth % 8 else 0)
                b = snprintf(buf, blen, "%s: \n", "${p4fldname}");
                buf += b;
                blen -= b;
                if (blen <= 0) {
                    return (P4PD_SUCCESS);
                }
                for (int j = 0; j < ${p4fldwidth_byte}; j++) {
                    b = snprintf(buf, blen, "0x%x : ", swkey->${p4fldname}[j]);
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
                b = snprintf(buf, blen, "%s: \n", "${p4fldname}_mask");
                buf += b;
                blen -= b;
                if (blen <= 0) {
                    return (P4PD_SUCCESS);
                }
                for (int j = 0; j < ${p4fldwidth_byte}; j++) {
                    b = snprintf(buf, blen, "0x%x : ", swkey_mask->${p4fldname}_mask[j]);
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
//::                    #endif
//::                #endfor
//::                for fld_un_containers in pddict['tables'][table]['fldunion_keys']:
//::                    fldcontainer, fldukeys = fld_un_containers
//::                    if len(fldukeys) > 1:
//::                        ustr = table + '_u' + str(i) + '.'
//::                        ustr_mask = table + '_mask_u' + str(i) + '.'
//::                    else:
//::                        ustr=''
//::                        ustr_mask=''
//::                    #endif
//::                    for fields in fldukeys:
//::                        (p4fldname, p4fldwidth, mask, key_byte_format, key_bit_format) = fields

//::                        (p4fldname, p4fldwidth, mask, key_byte_format, key_bit_format) = fields
//::                        if (p4fldwidth <= 32):
                b = snprintf(buf, blen, "%s: 0x%x\n", "${p4fldname}", swkey->${ustr}${p4fldname});
                buf += b;
                blen -= b;
                if (blen <= 0) {
                    return (P4PD_SUCCESS);
                }
                b = snprintf(buf, blen, "%s: 0x%x\n", "${p4fldname}_mask", swkey_mask->${ustr_mask}${p4fldname}_mask);
                buf += b;
                blen -= b;
                if (blen <= 0) {
                    return (P4PD_SUCCESS);
                }
//::                        else:
//::                            p4fldwidth_byte = (p4fldwidth / 8) + (1 if p4fldwidth % 8 else 0)
                b = snprintf(buf, blen, "%s: \n", "${p4fldname}");
                buf += b;
                blen -= b;
                if (blen <= 0) {
                    return (P4PD_SUCCESS);
                }
                for (int j = 0; j < ${p4fldwidth_byte}; j++) {
                    b = snprintf(buf, blen, "0x%x : ", swkey->${ustr}${p4fldname}[j]);
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
                b = snprintf(buf, blen, "%s: \n", "${p4fldname}_mask");
                buf += b;
                blen -= b;
                if (blen <= 0) {
                    return (P4PD_SUCCESS);
                }
                for (int j = 0; j < ${p4fldwidth_byte}; j++) {
                    b = snprintf(buf, blen, "0x%x : ", swkey_mask->${ustr_mask}${p4fldname}_mask[j]);
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
//::                        #endif
//::                    #endfor
//::                #endfor
//::                for hdr_un_containers in pddict['tables'][table]['hdrunion_keys']:
//::                    hdrcontainer, hdrukeys = hdr_un_containers
//::                    if len(hdrukeys) > 1:
//::                        ustr = table + '_hdr_u' + str(i) + '.'
//::                        ustr_mask = table + '_mask_hdr_u' + str(i) + '.'
//::                    else:
//::                        ustr=''
//::                        ustr_mask=''
//::                    #endif
//::                    for fields in hdrukeys:
//::                        (p4fldname, p4fldwidth, mask, key_byte_format, key_bit_format) = fields
//::                        if (p4fldwidth <= 32):
                b = snprintf(buf, blen, "%s: 0x%x\n", "${p4fldname}", swkey->${ustr}${p4fldname});
                buf += b;
                blen -= b;
                if (blen <= 0) {
                    return (P4PD_SUCCESS);
                }
                b = snprintf(buf, blen, "%s: 0x%x\n", "${p4fldname}_mask", swkey_mask->${ustr_mask}${p4fldname}_mask);
                buf += b;
                blen -= b;
                if (blen <= 0) {
                    return (P4PD_SUCCESS);
                }
//::                        else:
//::                            p4fldwidth_byte = (p4fldwidth / 8) + (1 if p4fldwidth % 8 else 0)
                b = snprintf(buf, blen, "%s: \n", "${p4fldname}");
                buf += b;
                blen -= b;
                if (blen <= 0) {
                    return (P4PD_SUCCESS);
                }
                for (int j = 0; j < ${p4fldwidth_byte}; j++) {
                    b = snprintf(buf, blen, "0x%x : ", swkey->${ustr}${p4fldname}[j]);
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
                b = snprintf(buf, blen, "%s: \n", "${p4fldname}_mask");
                buf += b;
                blen -= b;
                if (blen <= 0) {
                    return (P4PD_SUCCESS);
                }
                for (int j = 0; j < ${p4fldwidth_byte}; j++) {
                    b = snprintf(buf, blen, "0x%x : ", swkey_mask->${ustr_mask}${p4fldname}_mask[j]);
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
//::                        #endif
//::                    #endfor
//::                #endfor
            }
            ${table}_actiondata *actiondata = (${table}_actiondata *)action_data;
            if (!actiondata) {
                break;
            }
            switch(actiondata->actionid) {
//::                for action in pddict['tables'][table]['actions']:
//::                    (actionname, actionfldlist) = action
//::                    actname = actionname.upper()
                case ${caps_tablename}_${actname}_ID:
                {
                    b = snprintf(buf, blen, "Action: %s\n", "${caps_tablename}_${actname}_ID");
                    buf += b;
                    blen -= b;
                    if (blen <= 0) {
                        return (P4PD_SUCCESS);
                    }
//::                    for actionfld in actionfldlist:
//::                        actionfldname, actionfldwidth = actionfld
//::                        if actionfldwidth <= 32:
                    b = snprintf(buf, blen, "%s: 0x%x\n", "${actionfldname}",
                             actiondata->${table}_action_u.\
                             ${table}_${actionname}.${actionfldname});
                    buf += b;
                    blen -= b;
                    if (blen <= 0) {
                        return (P4PD_SUCCESS);
                    }
//::                        else:
//::                            p4fldwidth_byte = (actionfldwidth / 8) + (1 if actionfldwidth % 8 else 0)
                    b = snprintf(buf, blen, "%s: \n", "${actionfldname}");
                    buf += b;
                    blen -= b;
                    if (blen <= 0) {
                        return (P4PD_SUCCESS);
                    }
                    for (int j = 0; j < ${p4fldwidth_byte}; j++) {
                        b = snprintf(buf, blen, "  0x%x : ", 
                                 actiondata->${table}_action_u.\
                                 ${table}_${actionname}.${actionfldname}[j]);
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
//::                        #endif
//::                    #endfor
                }
                break;
//::                #endfor
            }
//::            #endif
        }
        break;
//::        #endfor
        default:
            // Invalid tableid
            return (P4PD_FAIL);
        break;
    }
    return (P4PD_SUCCESS);
}


//::    #endif

#ifdef P4PDGEN_COMPILE
#include <stdio.h>
int
main()
{
    printf ("Testing p4pd.c\n");
    return 0;
}
#endif
