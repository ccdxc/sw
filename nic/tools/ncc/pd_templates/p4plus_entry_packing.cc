//:: import os, pdb
//:: from collections import OrderedDict
//:: pddict = _context['pddict']
//:: if pddict['p4plus']:
//::    p4prog = pddict['p4program'] + '_'
//::    hdrdir = pddict['p4program']
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
//::    hdrdir = pddict['p4program']
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
#include "nic/gen/${hdrdir}/include/${p4prog}p4pd.h"

#define P4PD_FAIL (-1)
#define P4PD_SUCCESS (0)
#define P4PD_MAX_ACTION_DATA_LEN (512)
#define P4PD_MAX_MATCHKEY_LEN    (512) /* When multiple flits can be
                                        * chained, then key max size
                                        * can be more than 512. For
                                        * assuming one flit.
                                        */
#define P4PD_ACTIONPC_BITS       (8)   /* For now assume 8bit actionPC.
                                        * 9bits, change it to 2 bytes.
                                        */

char ${prefix}_tbl_names[__P4${caps_p4prog}TBL_ID_TBLMAX][P4${caps_p4prog}TBL_NAME_MAX_LEN];
uint16_t ${prefix}_tbl_swkey_size[__P4${caps_p4prog}TBL_ID_TBLMAX];
uint16_t ${prefix}_tbl_sw_action_data_size[__P4${caps_p4prog}TBL_ID_TBLMAX];
typedef int p4pd_error_t;



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
 *  OUT : uint32_t *hwactiondata_len   : Action data length in bits. 
 *
 * Return Value: 
 *  None
 */

//::        if pddict['tables'][table]['type'] == 'Index' or pddict['tables'][table]['type'] == 'Mpu':
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
                > P4PD_MAX_ACTION_DATA_LEN) {
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
//::                    if len(hdrukeys) > 1:
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
                > P4PD_MAX_ACTION_DATA_LEN) {
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


//::        if pddict['tables'][table]['type'] == 'Index' or pddict['tables'][table]['type'] == 'Mpu':
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
    actiondata->actionid = 0xff;
    int adatabyte = 1;
//::            else:
    actiondata->actionid = 0;
    int adatabyte = 0;
//::            #endif

    ${table}_unpack_action_data(tableid, actiondata->actionid,
                                hwentry + adatabyte, actiondata);

    return (P4PD_SUCCESS);
}
//::        #endif



/* Pack P4-table '${table}' entry and return packed table entry bits to caller
 * without installing entry into hardware/device. Entry packing API is only
 * supported on non ternary matching P4-tables.
 *
 * Arguments:
 *  IN  : uint32_t tableid                  : Table Id that identifies
 *                                            P4 table. This id is obtained
 *                                            from p4pd_table_id_enum.
 *
 *  IN  : ${table}_actions_un *actiondata   : Action data associated with
 *                                            the key.
 *   OUT:  uint32_t *entry_width            : Table entry width in bits
 *
 * Return Value:
 * uint8_t*                                 : Byte array of table entry bits.
 */
//::        if pddict['tables'][table]['type'] == 'Index' or pddict['tables'][table]['type'] == 'Mpu':
static uint8_t*
${table}_entry_pack(uint32_t tableid, uint8_t action_pc,
                    ${table}_actiondata *actiondata,
                    uint32_t *entry_width)
{
    uint8_t  _action_pc;
    uint8_t  packed_actiondata[P4PD_MAX_ACTION_DATA_LEN] = {0};
    uint8_t  hwentry[P4PD_MAX_MATCHKEY_LEN + P4PD_MAX_ACTION_DATA_LEN] = {0};
    uint16_t entry_size, actiondatalen;
    uint8_t  *_hwentry = &hwentry[0];

//::            if len(pddict['tables'][table]['actions']) > 1:
//::                add_action_pc = True
//::            else:
//::                add_action_pc = False
//::            #endif
//::            if add_action_pc:
    _action_pc = action_pc;
//::            else:
    _action_pc = 0xff;
//::            #endif
    actiondatalen = ${table}_pack_action_data(tableid, actiondata,
                                              packed_actiondata);
    entry_size = p4pd_p4table_entry_prepare(hwentry,
                                            _action_pc,
                                            NULL /* Index Table. No MatchKey*/,
                                            0, /* Zero matchkeylen */
                                            packed_actiondata,
                                            actiondatalen);
//::            if pddict['tables'][table]['location'] != 'HBM':
    p4pd_swizzle_bytes(hwentry, entry_size);
//::            #endif
    *entry_width = entry_size;
    return (_hwentry);
}

//::        #endif
//::     #endfor


//::    if len(tabledict):

//::        if pddict['p4plus']:
//::            api_prefix = 'p4pd_' + pddict['p4program']
//::        else:
//::            api_prefix = 'p4pd'
//::        #endif

p4pd_error_t ${api_prefix}_table_ds_decoded_string_get(uint32_t   tableid,
                                              void*      action_data,
                                              char*      buffer,
                                              uint16_t   buf_len);


/* Query key details for p4-table
 *
 * Arguments:
 *
 *  IN  : uint32_t tableid             : Table Id that identifies
 *                                       P4 table. This id is obtained
 *                                       from p4pd_table_id_enum.
 *  OUT : uint32_t *hwactiondata_len   : Action data length.
 *
 * Return Value:
 *  None
 */
void 
${api_prefix}_hwentry_query(uint32_t tableid,
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
            ${table}_hwentry_query(tableid, hwactiondata_len);
//::            #endif
        break;

//::        #endfor
    }
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


/*
 * Returns byte stream of P4-table entry that can be installed into hardware table.
 * Byte stream returned by this function needs to be copied by caller in its local
 * buffer. Width of the table entry in bits is returned in function argument that
 * can be used to copy table entry. This packing API is only supported for Index
 * lookup based P4-tables.
 *
 * Arguments:
 *
 *  IN  : uint32_t tableid       : Table Id that identifies
 *                                 P4 table. This id is obtained
 *                                 from p4pd_table_id_enum.
 *  IN  : uint8_t action_pc      : Action PC to include in table entry.
 *                                 If table has only one action, then this
 *                                 value is not used. Caller can specify and value.
 *  IN  : void    *actiondata    : Action data associated with the key.
 *                                 Action data structure generated per p4 table.
 *                                 Refer to p4pd.h for structure details
 *                                 Per p4 table action data structure should
 *                                 provided as void* actiondata.
 * OUT: uint32_t *entry_width    : Table entry width in bits.
 *
 * Return Value:
 *  uint8_t*                     : Packed table entry byte stream returned to caller.
 *                                 In case of invalid tableid, NULL is returned.
 */
uint8_t*
${api_prefix}_entry_pack(uint32_t tableid,
                         uint8_t action_pc,
                         void    *actiondata,
                         uint32_t *entry_width)
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
//::            if pddict['tables'][table]['type'] == 'Index' or pddict['tables'][table]['type'] == 'Mpu':
            return (${table}_entry_pack(tableid, action_pc,
                                        (${table}_actiondata*)actiondata, entry_width));
//::            #endif
        break;

//::        #endfor
        default:
            // Invalid tableid
            return (NULL);
        break;
    }
    return (NULL);
}

/* Return Log string of decoded P4 table hardware entry.
 *
 * Arguments: 
 *
 *  IN  : uint32_t tableid       : Table Id that identifies
 *                                 P4 table. This id is obtained
 *                                 from p4pd_table_id_enum.
 *  IN  : uint8_t  hwentry       : Table entry bytes as read from device/hardware.
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
                                             uint16_t   hwentry_len,
                                             char*      buffer,
                                             uint16_t   buf_len)
{
    char *buf = buffer;
    int  blen = buf_len, b;

    uint8_t  _hwentry[P4PD_MAX_MATCHKEY_LEN + P4PD_MAX_ACTION_DATA_LEN] = {0};

    memset(buffer, 0, buf_len);
    b = snprintf(buf, blen, "!!!! Decode HW table entry into (Table Key + Actiondata)/Actiondata!!!! \n");
    buf += b;
    blen -= b;
    if (blen <= 0) {
        return (P4PD_SUCCESS);
    }
    switch (tableid) {
//::        for table, tid in tabledict.items():
//::            caps_tablename = table.upper()
//::            if pddict['tables'][table]['type'] != 'Index' and pddict['tables'][table]['type'] != 'Mpu':
//::                continue
//::            #endif
        case P4${caps_p4prog}TBL_ID_${caps_tablename}: /* p4-table '${table}' */
        {
            memcpy(_hwentry, hwentry, (hwentry_len + 7) >> 3);
//::        if pddict['tables'][table]['location'] != 'HBM':
            p4pd_swizzle_bytes(_hwentry, hwentry_len);
//::        #endif
            b = snprintf(buf, blen, "Table: %s, Index 0x%x\n", "P4TBL_ID_${caps_tablename}", index);
            buf += b;
            blen -= b;
            if (blen <= 0) {
                return (P4PD_SUCCESS);
            }
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
                                          uint32_t   index,
                                          void*      action_data,
                                          char*      buffer,
                                          uint16_t   buf_len)
{
    char *buf = buffer;
    int  blen = buf_len, b;

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
//::            if pddict['tables'][table]['type'] != 'Index' and pddict['tables'][table]['type'] != 'Mpu':
//::                continue
//::            #endif
        case P4${caps_p4prog}TBL_ID_${caps_tablename}: /* p4-table '${table}' */
        {
            b = snprintf(buf, blen, "Table: %s, Index 0x%x\n", "P4TBL_ID_${caps_tablename}", index);
            buf += b;
            blen -= b;
            if (blen <= 0) {
                return (P4PD_SUCCESS);
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
                    b = snprintf(buf, blen, "%s: 0x", "${actionfldname}");
                    buf += b;
                    blen -= b;
                    if (blen <= 0) {
                        return (P4PD_SUCCESS);
                    }
                    for (int j = 0; j < ${p4fldwidth_byte}; j++) {
                        b = snprintf(buf, blen, "%02x",
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
//
//::    if len(tabledict):

void ${prefix}_prep_p4tbl_names()
{
//::        for  tblname in sorted(tabledict, key=tabledict.get):
//::            caps_tblname = tblname.upper() 
    strncpy(${prefix}_tbl_names[P4${caps_p4prog}TBL_ID_${caps_tblname}], "${tblname}", strlen("${tblname}"));
//::        #endfor
}

void ${prefix}_prep_p4tbl_sw_struct_sizes()
{
//::        for  tblname in sorted(tabledict, key=tabledict.get):
//::            caps_tblname = tblname.upper() 
//::        if pddict['tables'][tblname]['type'] != 'Index':
    ${prefix}_tbl_swkey_size[P4${caps_p4prog}TBL_ID_${caps_tblname}] = sizeof(${tblname}_swkey);
//::        #endif
    ${prefix}_tbl_sw_action_data_size[P4${caps_p4prog}TBL_ID_${caps_tblname}]= sizeof(${tblname}_actiondata);
//::        #endfor
}

//::    #endif

int ${prefix}_get_max_action_id(uint32_t tableid)
{
    switch(tableid) {
//::        for  tblname in sorted(tabledict, key=tabledict.get):
//::            caps_tblname = tblname.upper() 
        case P4${caps_p4prog}TBL_ID_${caps_tblname}:
            return (${caps_tblname}_MAX_ID);
        break;
//::        #endfor
    }
    // Not found tableid case
    return (0);
}

void ${prefix}_get_action_name(uint32_t tableid, int actionid, char *action_name)
{
    switch(tableid) {
//::        for  tblname in sorted(tabledict, key=tabledict.get):
//::            caps_tblname = tblname.upper() 
        case P4${caps_p4prog}TBL_ID_${caps_tblname}:
//::            if len(pddict['tables'][tblname]['actions']):
            switch(actionid) {
//::                for action in pddict['tables'][tblname]['actions']:
//::                    (actionname, actionfldlist) = action
//::                    actname = actionname.upper()
                case ${caps_tblname}_${actname}_ID:
                    strcpy(action_name, "${actionname}");
                    return;
                break;
//::                #endfor
            }
//::            #endif
        break;
//::        #endfor
    }
    *action_name = '\0';
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
