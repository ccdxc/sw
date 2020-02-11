//:: import os, pdb
//:: from collections import OrderedDict
//:: pddict = _context['pddict']
//:: if pddict['p4plus']:
//::    p4prog = pddict['p4program'] + '_'
//::    hdrdir = pddict['p4program']
//::    pipeline = pddict['pipeline']
//::    caps_p4prog = '_' + pddict['p4program'].upper() + '_'
//::    if pddict['p4plus_module'] == 'rxdma':
//::        start_table_base = 101
//::        prefix = 'p4pd_rxdma'
//::    elif pddict['p4plus_module'] == 'txdma':
//::	    start_table_base = 201
//::        prefix = 'p4pd_txdma'
//::    else:
//::	    start_table_base = 301
//::        prefix = 'p4pd_' + pddict['p4program']
//::    #endif
//:: else:
//::    p4prog = ''
//::    hdrdir = pddict['p4program']
//::    pipeline = pddict['pipeline']
//::    caps_p4prog = ''
//::    prefix = 'p4pd'
//::	start_table_base = 1
//:: #endif
//:: #define here any constants needed.
//:: ACTION_PC_LEN = 8 # in bits
//:: ACTION_NAME_MAX_LEN = 100 # Action name will be truncated to 100 Max characters
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
#include "gen/p4gen/${hdrdir}/include/${p4prog}p4pd.h"
#include "nic/sdk/lib/p4/p4_utils.hpp"
#include "nic/sdk/asic/pd/pd.hpp"
#include "nic/sdk/lib/utils/time_profile.hpp"

char ${prefix}_tbl_names[__P4${caps_p4prog}TBL_ID_TBLMAX][P4${caps_p4prog}TBL_NAME_MAX_LEN];
uint16_t ${prefix}_tbl_swkey_size[__P4${caps_p4prog}TBL_ID_TBLMAX];
uint16_t ${prefix}_tbl_sw_action_data_size[__P4${caps_p4prog}TBL_ID_TBLMAX];

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
//::            (actionname, actionflddict, _) = action
//::            actname = actionname.upper()
//::            if not len(actionflddict):
//::                continue
//::            #endif
//::            actionfldlen = 0
//::            for actionfld in actionflddict:
//::                actionfldname  = actionfld['p4_name']
//::                actionfldwidth = actionfld['len']
//::                actionfldlen += actionfldwidth
//::            #endfor
//::            if actionfldlen > max_actionfld_len:
//::                max_actionfld_len = actionfldlen
//::            #endif
//::        #endfor
//::        #Toeplitz hash tables do not table programming APIs.
//::        #There is no lookup table to match toeplitz key
//::        if pddict['tables'][table]['is_toeplitz_hash'] or pddict['tables'][table]['is_raw']:
//::            continue
//::        #endif

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
//::            if pddict['tables'][table]['is_wide_key']:
    *hwkey_len = ${keylen};
//::            else:
    // For hash case always return key length as 512 bits.
    *hwkey_len = 512;
//::            #endif
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
//::        if (pddict['tables'][table]['type'] != 'Ternary' and pddict['tables'][table]['type'] != 'Index' and pddict['tables'][table]['type'] != 'Mpu') or pddict['tables'][table]['otcam']:

static uint32_t
${table}_pack_action_data(uint32_t tableid, ${table}_actiondata_t *actiondata,
                          uint8_t *packed_actiondata_before_key,
                          uint16_t *actiondata_len_before_key,
                          uint8_t *packed_actiondata_after_key,
                          uint16_t *actiondata_len_after_key)
{
    uint16_t dest_start_bit = 0;
    uint16_t source_start_bit = 0;
    uint8_t  *packed_action_data;
    uint16_t mat_key_start_bit, mat_key_bit_length, bits_to_copy;

    (void)packed_action_data;
    (void)mat_key_start_bit;
    (void)mat_key_bit_length;
    (void)bits_to_copy;
    (void)source_start_bit;
    (void)dest_start_bit;

    *actiondata_len_before_key = 0;
    *actiondata_len_after_key = 0;
//::            mat_key_start_bit = pddict['tables'][table]['match_key_start_bit']
//::            mat_key_bit_length = pddict['tables'][table]['match_key_bit_length']
//::            if pddict['tables'][table]['is_wide_key']:
//::                mat_key_bit_length = pddict['tables'][table]['wide_key_len']
//::            #endif
//::            apc_offset = 0
//::            if len(pddict['tables'][table]['actions']) > 1:
//::                add_action_pc = True
//::                apc_offset = 8
//::            else:
//::                add_action_pc = False
//::            #endif
    switch(actiondata->action_id) {
//::            for action in pddict['tables'][table]['actions']:
//::                (actionname, actionflddict, _) = action
//::                actname = actionname.upper()
//::                if not len(actionflddict):
//::                    continue
//::                #endif
        case ${tbl}_${actname}_ID:
//::                apc_offset = 0
//::                if add_action_pc:
            mat_key_start_bit = ${mat_key_start_bit} - 8 /* 8 bits for action pc */; /* MatchKeyStart with APC before Key */
//::                else:
            mat_key_start_bit = ${mat_key_start_bit}; /* MatchKeyStart without APC before Key */
//::                #endif
            mat_key_bit_length = ${mat_key_bit_length}; /* MatchKeyLen */

//::                iter = 0
//::                while iter < len(actionflddict):
//::                    actionfld = actionflddict[iter]
//::                    actionfldname  = actionfld['p4_name']
//::                    actionfldwidth = actionfld['len']
//::                    dest_start_bit = actionfld['dvec_start'] - apc_offset
//::                    src_start_bit  = 0
//::                    if ((iter + 1) < len(actionflddict)):
//::                        nextactionfld = actionflddict[iter + 1]
//::                        nextactionfldname = nextactionfld['p4_name']
//::                        if actionfldname == nextactionfldname:
//::                            src_start_bit = nextactionfld['len']
//::                        #endif
//::                    #endif
            /* Field: ${actionfldname} */
            dest_start_bit   = ${dest_start_bit};
            bits_to_copy     = ${actionfldwidth};
            source_start_bit = ${src_start_bit};

            if ((*actiondata_len_before_key + *actiondata_len_after_key + bits_to_copy)
                > P4PD_MAX_ACTION_DATA_LEN) {
                assert(0);
            }

            if ((dest_start_bit >= mat_key_start_bit) &&
                    (dest_start_bit < (mat_key_start_bit + mat_key_bit_length))) {
                assert(0);
            }

            if (dest_start_bit < mat_key_start_bit) {
                packed_action_data = packed_actiondata_before_key;
                (*actiondata_len_before_key) += bits_to_copy;
            } else {
                packed_action_data = packed_actiondata_after_key;
                dest_start_bit -= (mat_key_start_bit + mat_key_bit_length);
                (*actiondata_len_after_key) = dest_start_bit + bits_to_copy;
            }

            p4pd_utils_copy_le_src_to_be_dest(packed_action_data,
                    dest_start_bit,
//::                    if actionfldwidth <= 32:
                    (uint8_t*)&(actiondata->action_u.${table}_${actionname}.${actionfldname}),
//::                    else:
                    (uint8_t*)(actiondata->action_u.${table}_${actionname}.${actionfldname}),
//::                    #endif
                    source_start_bit, bits_to_copy);

//::                    iter = iter + 1
//::                #endwhile
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
                          ${table}_actiondata_t *actiondata,
                          uint8_t *packed_actiondata)

{
    switch(actiondata->action_id) {
//::            apc_offset = 0
//::            if len(pddict['tables'][table]['actions']) > 1:
//::                apc_offset = 8
//::            #endif
//::            for action in pddict['tables'][table]['actions']:
//::                (actionname, actionflddict, _) = action
//::                actname = actionname.upper()
//::                if not len(actionflddict):
//::                    continue
//::                #endif
        case ${tbl}_${actname}_ID:
//::                for actionfld in actionflddict:
//::                    actionfldname  = actionfld['p4_name']
//::                    actionfldwidth = actionfld['len']
//::                    fld_start_bit  = actionfld['field_start']
//::                    dest_start_bit = actionfld['dvec_start'] - apc_offset

            /* Field: ${actionfldname} */
            if ((${dest_start_bit} + ${actionfldwidth}) > P4PD_MAX_ACTION_DATA_LEN) {
                assert(0);
            }
            p4pd_utils_copy_le_src_to_be_dest(packed_actiondata,
                    ${dest_start_bit},
//::                    if actionfldwidth <= 32:
                    (uint8_t*)&(actiondata->action_u. ${table}_${actionname}.${actionfldname}),
//::                    else:
                    (uint8_t*)(actiondata->action_u.${table}_${actionname}.${actionfldname}),
//::                    #endif
                    ${fld_start_bit}, /* Start bit in source */
                    ${actionfldwidth});
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
    time_profile_begin(sdk::utils::time_profile::P4PD_HWKEY_HWMASK_BUILD);
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
//::                for fields in pddict['tables'][table]['hdrunion_keys']:
//::                    (p4fldname, p4fldwidth, mask, key_byte_format, key_bit_format) = fields
//::                    if len(key_byte_format):
//::                        for kmbyte, kbit, _, _ in key_byte_format:
//::                            tablebyte = kmbyte
     * [${p4fldname}, ${kbit},  ${tablebyte}, ${kmbyte}. Sourced from hdr union key]
//::                        #endfor
//::                    #endif
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
//::                for fields in pddict['tables'][table]['hdrunion_keys']:
//::                    (p4fldname, p4fldwidth, mask, key_byte_format, key_bit_format) = fields
//::                    if len(key_bit_format):
//::                        for kmbit, kbit in key_bit_format:
//::                            tablebit = kmbit
     * [ ${p4fldname}, ${kbit},  ${tablebit}, ${kmbit}. Sourced from hdr union key]
//::                        #endfor
//::                    #endif
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
//::                                else:
    k = *((uint8_t*)&(swkey->${p4fldname}) + ${kbyte});
    m = *((uint8_t*)&(swkey_mask->${p4fldname}_mask) + ${kbyte});
//::                                #endif

//::                            else:
//::                                if kbit < 7:
    /* P4 field is extended.. a less than byte field is extended to byte */
    k = *((uint8_t*)&(swkey->${p4fldname}[${kbyte}]));
    m = *((uint8_t*)&(swkey_mask->${p4fldname}_mask[${kbyte}]));
//::                                else:
    k = *((uint8_t*)&(swkey->${p4fldname}[${kbyte}]));
    m = *((uint8_t*)&(swkey_mask->${p4fldname}_mask[${kbyte}]));
//::                                #endif
//::                            #endif
    trit_x = k & m;
    trit_y = ~k & m;
//::                            if width == 1:
    p4pd_utils_copy_into_hwentry(hwkey_x,
                   (${tablebyte} * 8) + (7 - ${containerstart}) - ${mat_key_start_bit}, /* Dest bit position */
                   &trit_x,
                   0,
                   ${width});

    p4pd_utils_copy_into_hwentry(hwkey_y,
                   (${tablebyte} * 8) + (7 - ${containerstart}) - ${mat_key_start_bit}, /* Dest bit position */
                   &trit_y,
                   0,
                   ${width});
//::                            else:
    p4pd_utils_copy_into_hwentry(hwkey_x,
                   (${tablebyte} * 8) + ${containerstart} - ${mat_key_start_bit}, /* Dest bit position */
                   &trit_x,
                   0,
                   ${width});

    p4pd_utils_copy_into_hwentry(hwkey_y,
                   (${tablebyte} * 8) + ${containerstart} - ${mat_key_start_bit}, /* Dest bit position */
                   &trit_y,
                   0,
                   ${width});
//::                            #endif
    key_len += ${width};
//::                        #endfor
//::                    #endif
//::                    if len(key_bit_format):
//::                        for kmbit, _kbit in key_bit_format:
//::                            tablebit = kmbit
//::                            kbyte = (p4fldwidth - 1 - _kbit) / 8
//::                            kbit = (p4fldwidth - 1 - _kbit) % 8
//::                            ## bit number is converted to LE notation where bit0 = lsb in a byte
//::                            hwkey_bit_le = (kmbit - (kmbit % 8) + 7 - (kmbit % 8)) - mat_key_start_bit
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
    p4pd_utils_copy_into_hwentry(hwkey_x,
                    ${hwkey_bit_le}, /* Dest bit position */
                   &trit_x,
                   0,
                   1 /* bits to copy */);
    p4pd_utils_copy_into_hwentry(hwkey_y,
                    ${hwkey_bit_le}, /* Dest bit position */
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
//::                                    else:
    k = *((uint8_t*)&(swkey->${ustr}${p4fldname}) + ${kbyte});
    m = *((uint8_t*)&(swkey_mask->${ustr_mask}${p4fldname}_mask) + ${kbyte});
//::                                    #endif
//::                                else:
//::                                    if kbit < 7:
    /* P4 field is extended.. a less than byte field is extended to byte */
    k = *((uint8_t*)&(swkey->${ustr}${p4fldname}[${kbyte}]));
    m = *((uint8_t*)&(swkey_mask->${ustr_mask}${p4fldname}_mask[${kbyte}]));
//::                                    else:
    k = *((uint8_t*)&(swkey->${ustr}${p4fldname}[${kbyte}]));
    m = *((uint8_t*)&(swkey_mask->${ustr_mask}${p4fldname}_mask[${kbyte}]));
//::                                    #endif
//::                                #endif
    trit_x = k & m;
    trit_y = ~k & m;
    p4pd_utils_copy_into_hwentry(hwkey_x,
                   (${tablebyte} * 8) + ${containerstart} - ${mat_key_start_bit}, /* Dest bit position */
                   &trit_x,
                   0,
                   ${width});
    p4pd_utils_copy_into_hwentry(hwkey_y,
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
    k = *((uint8_t*)&(swkey->${ustr}${p4fldname}) + ${kbyte});
    m = *((uint8_t*)&(swkey_mask->${ustr}${p4fldname}_mask) + ${kbyte});
//::                                else:
    k = *((uint8_t*)&(swkey->${ustr}${p4fldname}[${kbyte}]));
    m = *((uint8_t*)&(swkey_mask->${ustr}${p4fldname}_mask[${kbyte}]));
//::                                #endif

    trit_x = ((k & m) >> ${kbit}) & 0x1;
    trit_y = ((~k & m) >>${kbit}) & 0x1;
    p4pd_utils_copy_into_hwentry(hwkey_x,
                    ((${kmbit} - (${kmbit} % 8)) + (7 - (${kmbit} % 8)))- (${mat_key_start_bit}), /* Dest bit position */
                   &trit_x,
                   0,
                   1 /* bits to copy */);

    p4pd_utils_copy_into_hwentry(hwkey_y,
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
//::                for fields in pddict['tables'][table]['hdrunion_keys']:
//::                    ustr=''
//::                    ustr_mask=''
//::                    (p4fldname, p4fldwidth, mask, key_byte_format, key_bit_format) = fields
//::                    if len(key_byte_format):
//::                        for kmbyte, _kbit, width, containerstart in key_byte_format:
//::                            tablebyte = kmbyte
//::                            kbyte = (p4fldwidth - 1 - _kbit) / 8
//::                            kbit = (p4fldwidth - 1 - _kbit) % 8
    /* Header Union Key byte */
//::                            if p4fldwidth <= 32:
//::                                if kbit < 7:
    /* P4 field is extended.. a less than byte field is extended to byte */
    k = *((uint8_t*)&(swkey->${ustr}${p4fldname}) + ${kbyte});
    m = *((uint8_t*)&(swkey_mask->${ustr_mask}${p4fldname}_mask) + ${kbyte});
//::                                else:
    /* P4 field is extended.. a less than byte field is extended to byte */
    k = *((uint8_t*)&(swkey->${ustr}${p4fldname}) + ${kbyte});
    m = *((uint8_t*)&(swkey_mask->${ustr_mask}${p4fldname}_mask) + ${kbyte});
//::                                #endif
//::                            else:
//::                                if kbit < 7:
    k = *((uint8_t*)&(swkey->${ustr}${p4fldname}[${kbyte}]));
    m = *((uint8_t*)&(swkey_mask->${ustr_mask}${p4fldname}_mask[${kbyte}]));
//::                                else:
    k = *((uint8_t*)&(swkey->${ustr}${p4fldname}[${kbyte}]));
    m = *((uint8_t*)&(swkey_mask->${ustr_mask}${p4fldname}_mask[${kbyte}]));
//::                                #endif
//::                            #endif
    trit_x = k & m;
    trit_y = ~k & m;
    p4pd_utils_copy_into_hwentry(hwkey_x,
                   (${tablebyte} * 8) + ${containerstart} - ${mat_key_start_bit}, /* Dest bit position */
                   &trit_x,
                   0,
                   ${width});
    p4pd_utils_copy_into_hwentry(hwkey_y,
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
    /* Header Union Key bit */
//::                            if p4fldwidth <= 32:
    k = *((uint8_t*)&(swkey->${ustr}${p4fldname}) + ${kbyte});
    m = *((uint8_t*)&(swkey_mask->${ustr}${p4fldname}_mask) + ${kbyte});
//::                            else:
    k = *((uint8_t*)&(swkey->${ustr}${p4fldname}[${kbyte}]));
    m = *((uint8_t*)&(swkey_mask->${ustr}${p4fldname}_mask[${kbyte}]));
//::                            #endif
    trit_x = ((k & m) >> ${kbit}) & 0x1;
    trit_y = ((~k & m) >>${kbit}) & 0x1;
    p4pd_utils_copy_into_hwentry(hwkey_x,
                    ((${kmbit} - (${kmbit} % 8)) + (7 - (${kmbit} % 8)))- (${mat_key_start_bit}), /* Dest bit position */
                   &trit_x,
                   0, /* start bit */
                   1 /* bits to copy */);
    p4pd_utils_copy_into_hwentry(hwkey_y,
                    ((${kmbit} - (${kmbit} % 8)) + (7 - (${kmbit} % 8)))- (${mat_key_start_bit}), /* Dest bit position */
                   &trit_y,
                   0, /* start bit */
                   1 /* bits to copy */);
    key_len += 1;
//::                        #endfor
//::                    #endif
//::                #endfor
//::                for kbyte in pddict['tables'][table]['not_my_key_bytes']:
//::                    tablebyte = kbyte
    /* ${kbyte} does not belong to my table. Hence set do not match */
    trit_x = 0x0;/* Do not match case. Set both x any y to 1 */
    trit_y = 0x0;
    p4pd_utils_copy_into_hwentry(hwkey_x,
                   (${tablebyte} * 8) - ${mat_key_start_bit}, /* Dest bit position */
                   &trit_x,
                   0, /* Start bit in source */
                   8 /* 8 bits */);
    p4pd_utils_copy_into_hwentry(hwkey_y,
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
    p4pd_utils_copy_into_hwentry(hwkey_x,
                    ((${kmbit} - (${kmbit} % 8)) + (7 - (${kmbit} % 8)))- (${mat_key_start_bit}), /* Dest bit position */
                   &trit_x,
                   0, /* Start bit in source */
                   1 /* 1 bits */);
    p4pd_utils_copy_into_hwentry(hwkey_y,
                    ((${kmbit} - (${kmbit} % 8)) + (7 - (${kmbit} % 8)))- (${mat_key_start_bit}), /* Dest bit position */
                   &trit_y,
                   0, /* Start bit in source */
                   1 /* 1 bits */);
    key_len += 1;
//::                #endfor
    time_profile_end(sdk::utils::time_profile::P4PD_HWKEY_HWMASK_BUILD);
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
//::                for fields in pddict['tables'][table]['hdrunion_keys']:
//::                    (p4fldname, p4fldwidth, mask, key_byte_format, key_bit_format) = fields
//::                    if len(key_bit_format):
//::                        for kmbyte, kbit, _, _ in key_byte_format:
//::                            tablebyte = kmbyte
     * [ ${p4fldname}, ${kbit},  ${tablebyte}, ${kmbyte}. Sourced from hdr union key]
//::                        #endfor
//::                    #endif
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
//::                for fields in pddict['tables'][table]['hdrunion_keys']:
//::                    (p4fldname, p4fldwidth, mask, key_byte_format, key_bit_format) = fields
//::                    if len(key_bit_format):
//::                        for kmbit, kbit in key_bit_format:
//::                            tablebit = kmbit
     * [ ${p4fldname}, ${kbit},  ${tablebit}, ${kmbit}. Sourced from hdr union key]
//::                        #endfor
//::                    #endif
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
    p4pd_utils_copy_into_hwentry(hwkey,
                   (${tablebyte} * 8) + ${containerstart} - ${mat_key_start_bit}, /* Dest bit position */
//::                            if p4fldwidth <= 32:
                   (uint8_t*)((uint8_t*)&(swkey->${p4fldname}) + ${kbyte}),
//::                            else:
                   &(swkey->${p4fldname}[${kbyte}]),
//::                            #endif
                   (${p4fldwidth} - 1 - ${kbit}) % 8, /* Start bit in source field */
                   ${width});
    key_len += ${width};
//::                        #endfor
//::                    #endif
//::                    if len(key_bit_format):
//::                        for kmbit, kbit in key_bit_format:
    /* Key bit */
    p4pd_utils_copy_into_hwentry(hwkey,
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
    p4pd_utils_copy_into_hwentry(hwkey,
                   (${tablebyte} * 8) + ${containerstart} - ${mat_key_start_bit}, /* Dest bit position */
//::                                if p4fldwidth <= 32:
                   (uint8_t*)((uint8_t*)&(swkey->${ustr}${p4fldname}) + ${kbyte}),
//::                                else:
                   &(swkey->${ustr}${p4fldname}[${kbyte}]),
//::                                #endif
                   (${p4fldwidth} - 1 - ${kbit}) % 8, /* Start bit in source field */
                   ${width});
    key_len += ${width};
//::                            #endfor
//::                        #endif
//::                        if len(key_bit_format):
//::                            for kmbit, kbit in key_bit_format:
    /* Field Union Key bit */
    p4pd_utils_copy_into_hwentry(hwkey,
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
//::                for fields in pddict['tables'][table]['hdrunion_keys']:
//::                    ustr=''
//::                    (p4fldname, p4fldwidth, mask, key_byte_format, key_bit_format) = fields
//::                    if len(key_byte_format):
//::                        for kmbyte, kbit, width, containerstart in key_byte_format:
//::                            kbyte = (p4fldwidth - 1 - kbit) / 8
//::                            tablebyte = kmbyte
    /* Header Union Key byte */
    p4pd_utils_copy_into_hwentry(hwkey,
                   (${tablebyte} * 8) + ${containerstart} - ${mat_key_start_bit}, /* Dest bit position */
//::                             if p4fldwidth <= 32:
                   (uint8_t*)((uint8_t*)&(swkey->${ustr}${p4fldname}) + ${kbyte}),
//::                             else:
                   &(swkey->${ustr}${p4fldname}[${kbyte}]),
//::                             #endif
                   (${p4fldwidth} - 1 - ${kbit}) % 8, /* Start bit in source field */
                   ${width});
    key_len += ${width};
//::                        #endfor
//::                    #endif
//::                    if len(key_bit_format):
//::                        for kmbit, kbit in key_bit_format:
    /* Header Union Key bit */
    p4pd_utils_copy_into_hwentry(hwkey,
                   ((${kmbit} - (${kmbit} % 8)) + (7 - (${kmbit} % 8)))- (${mat_key_start_bit}), /* Dest bit position */
//::                            if p4fldwidth <= 32:
                   (uint8_t*)((uint8_t*)&(swkey->${ustr}${p4fldname}) + ((${p4fldwidth} - ${kbit})/8)),
//::                            else:
                   &(swkey->${ustr}${p4fldname}[${kbit}/8]),
//::                            #endif
                   (${p4fldwidth} - 1 - ${kbit}) % 8 /* source bit; KM bit 0 is MSB */,
                   1 /* copy single bit */);
    key_len += 1;
//::                        #endfor
//::                    #endif
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
//::                for fields in pddict['tables'][table]['hdrunion_keys']:
//::                    (p4fldname, p4fldwidth, mask, key_byte_format, key_bit_format) = fields
//::                    if len(key_bit_format):
//::                        for kmbyte, kbit, _, _ in key_byte_format:
//::                            tablebyte = kmbyte
     * [${p4fldname}, ${kbit},    ${tablebyte}, ${kmbyte} ; Sourced from fld union key]
//::                        #endfor
//::                    #endif
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
//::                for fields in pddict['tables'][table]['hdrunion_keys']:
//::                    (p4fldname, p4fldwidth, mask, key_byte_format, key_bit_format) = fields
//::                    if len(key_bit_format):
//::                        for kmbit, kbit in key_bit_format:
//::                            tablebit = kmbit
     * [ ${p4fldname}, ${kbit},  ${tablebit}, ${kmbit}. Sourced from hdr union key]
//::                        #endfor
//::                    #endif
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
//::                            ## get bit position within key bits and convert to LE notation
//::                            key_end_bit = mat_key_start_bit + mat_key_bit_length
//::                            hwkey_bit_le = key_end_bit - kmbit - 1
    /* Key bit */
    p4pd_utils_copy_into_hwentry(hwkey,
                    ${hwkey_bit_le}, /* Dest bit position */
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
        = (*(uint8_t*)((uint8_t*)&(swkey->${ustr}${p4fldname}) + ${kbyte})) << (8 - ${width} - ${containerstart});
//::                            #endfor
//::                        #endif
//::                        if len(key_bit_format):
//::                            for kmbit, kbit in key_bit_format:
//::                            ## get bit position within key bits and convert to LE notation
//::                            key_end_bit = mat_key_start_bit + mat_key_bit_length
//::                            hwkey_bit_le = key_end_bit - kmbit - 1
    /* Field Union Key bit */
    p4pd_utils_copy_into_hwentry(hwkey,
                    ${hwkey_bit_le}, /* Dest bit position */
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
//::                for fields in pddict['tables'][table]['hdrunion_keys']:
//::                    ustr=''
//::                    (p4fldname, p4fldwidth, mask, key_byte_format, key_bit_format) = fields
//::                    if len(key_byte_format):
//::                        for kmbyte, kbit, width, containerstart in key_byte_format:
//::                            kbyte = (p4fldwidth - 1 - kbit) / 8
//::                            tablebyte = kmbyte
    /* Header Union Key byte */
   *(hwkey + ((${mat_key_bit_length} - ${width} - ((${tablebyte} * 8) + ${containerstart}  - ${mat_key_start_bit})/* Dest bit position */) >> 3))
        = (*(uint8_t*)((uint8_t*)&(swkey->${ustr}${p4fldname}) + ${kbyte})) << (8 - ${width} - ${containerstart});
//::                            #endfor
//::                    #endif
//::                    if len(key_bit_format):
//::                        for kmbit, kbit in key_bit_format:
//::                            ## get bit position within key bits and convert to LE notation
//::                            key_end_bit = mat_key_start_bit + mat_key_bit_length
//::                            hwkey_bit_le = key_end_bit - kmbit - 1
    /* Header Union Key bit */
    p4pd_utils_copy_into_hwentry(hwkey,
                    ${hwkey_bit_le}, /* Dest bit position */
//::                            if p4fldwidth <= 32:
                   (uint8_t*)((uint8_t*)&(swkey->${ustr}${p4fldname}) + ((${p4fldwidth} - ${kbit})/8)),
//::                            else:
                   &(swkey->${ustr}${p4fldname}[${kbit}/8]),
//::                            #endif
                   (${p4fldwidth} - 1 - ${kbit}) % 8 /* source bit; KM bit 0 is MSB */,
                   1 /* copy single bit */);
//::                        #endfor
//::                    #endif
//::                #endfor
    return (_hwkey);
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
//::        if pddict['tables'][table]['type'] == 'Ternary' and pddict['tables'][table]['otcam']:
static p4pd_error_t
${table}_entry_write(uint32_t tableid,
                     uint32_t index,
                     uint8_t *hwkey,
                     uint8_t *hwkey_y,
                     ${table}_actiondata_t *actiondata,
                     ${table}_actiondata_t *actiondata_mask)
{
    uint32_t hwkey_len, hwkeymask_len;
    uint8_t  action_pc;
    uint8_t  packed_actiondata_after_key[P4PD_MAX_ACTION_DATA_LEN] = {0};
    uint8_t  packed_actiondata[P4PD_MAX_ACTION_DATA_LEN] = {0};
    uint8_t  *packed_actiondata_before_key = packed_actiondata;
    uint8_t  sram_entry[P4PD_MAX_MATCHKEY_LEN + P4PD_MAX_ACTION_DATA_LEN] = {0};
    uint16_t entry_size, actiondatalen, axi_shift_len;
    uint16_t actiondata_len_before_key, actiondata_len_after_key;
    uint8_t  *_sram_entry = &sram_entry[0];
    if (actiondata_mask) {
        return P4PD_FAIL;
    }
    (void)packed_actiondata_before_key;
    (void)actiondata_len_after_key;
    (void)actiondata_len_before_key;
    (void)actiondatalen;
    tcam_${table}_hwkey_len(tableid, &hwkey_len, &hwkeymask_len);
//::            if len(pddict['tables'][table]['actions']) > 1:
//::                add_action_pc = True
//::            else:
//::                add_action_pc = False
//::            #endif
//::            if add_action_pc:
    action_pc = sdk::asic::pd::asicpd_get_action_pc(tableid, actiondata->action_id);
    assert(action_pc != 0xff);
//::            else:
    action_pc = 0xff;
//::            #endif
    /* For hash otcam tables, action data packing in TCAM's SRAM should be
     * same as how it is packed for regular hash table.
     */
    actiondatalen = ${table}_pack_action_data(tableid, actiondata,
                                     packed_actiondata_before_key,
                                     &actiondata_len_before_key,
                                     packed_actiondata_after_key,
                                     &actiondata_len_after_key);

    /* For hash otcam tables action data packing in TCAM's SRAM should be
     * same as how it packed for regular hash table. Hence use hash packing
     * function to prepare sram entry.
     */
    entry_size = p4pd_utils_hash_table_entry_prepare(sram_entry,
                                             action_pc,
                                             ${pddict['tables'][table]['match_key_start_bit']},/*MatchKeyStartBit */
                                             NULL,
                                             hwkey_len,
                                             actiondatalen,
                                             packed_actiondata_before_key,
                                             actiondata_len_before_key,
                                             packed_actiondata_after_key,
                                             actiondata_len_after_key,
                                             &axi_shift_len);
    if (axi_shift_len) {
        /* Due to leading axi_shift space, actual entry line
         * does not start at byte zero.
         */
        _sram_entry += axi_shift_len;
    }
    p4pd_utils_swizzle_bytes(_sram_entry, entry_size);
    // Write to SRAM  area that is associated with TCAM. This SRAM area is
    // in the bottom portion of hash-table's sram area.
    // Hence increment index by size of hash table.
    sdk::asic::pd::asicpd_table_entry_write(tableid,
                       index + ${pddict['tables'][table]['parent_hash_table_size']},
                       _sram_entry, entry_size, NULL);
    // Install Key in TCAM
    // Swizzle Key installed in TCAM before writing to TCAM memory
    // because TCAM entry is not built using p4pd_utils_p4table_entry_prepare
    // function where bytes are swizzled.
    p4pd_utils_swizzle_bytes(hwkey, hwkey_len);
    p4pd_utils_swizzle_bytes(hwkey_y, hwkeymask_len);
    int pad = (hwkey_len % 16) ? (16 - (hwkey_len % 16)) : 0;
    sdk::asic::pd::asicpd_tcam_table_entry_write(tableid, index, hwkey,
                                                 hwkey_y, hwkey_len + pad);

    return (P4PD_SUCCESS);
}
//::        elif pddict['tables'][table]['type'] == 'Ternary':
static p4pd_error_t
${table}_entry_write(uint32_t tableid,
                     uint32_t index,
                     uint8_t *hwkey,
                     uint8_t *hwkey_y,
                     ${table}_actiondata_t *actiondata,
                     ${table}_actiondata_t *actiondata_mask)
{
    uint8_t  packed_data[P4PD_MAX_ACTION_DATA_LEN] = {0};
    uint8_t  *packed_actiondata = packed_data;
    uint8_t  sram_hwentry[P4PD_MAX_MATCHKEY_LEN + P4PD_MAX_ACTION_DATA_LEN] = {0};
    uint32_t hwkey_len, hwkeymask_len, actiondatalen;
    uint16_t action_pc, entry_size;
    if (actiondata_mask) {
        return P4PD_FAIL;
    }
//::            if len(pddict['tables'][table]['actions']) > 1:
//::                add_action_pc = True
//::            else:
//::                add_action_pc = False
//::            #endif
//::            if add_action_pc:
    action_pc = sdk::asic::pd::asicpd_get_action_pc(tableid, actiondata->action_id);
    assert(action_pc != 0xff);
//::            else:
    action_pc = 0xff;
//::            #endif
    actiondatalen = ${table}_pack_action_data(tableid, actiondata,
                                                packed_actiondata);

    entry_size = p4pd_utils_p4table_entry_prepare(sram_hwentry,
                                            action_pc,
                                            NULL /* No MatchKey */,
                                            0, /* Zero matchkeylen */
                                            packed_actiondata,
                                            actiondatalen);
    p4pd_utils_swizzle_bytes(sram_hwentry, entry_size);
    sdk::asic::pd::asicpd_table_entry_write(tableid, index, sram_hwentry,
                                            entry_size, NULL);
    // Install Key in TCAM
    tcam_${table}_hwkey_len(tableid, &hwkey_len, &hwkeymask_len);
    // Swizzle Key installed in TCAM before writing to TCAM memory
    // because TCAM entry is not built using p4pd_utils_p4table_entry_prepare
    // function where bytes are swizzled.
    p4pd_utils_swizzle_bytes(hwkey, hwkey_len);
    p4pd_utils_swizzle_bytes(hwkey_y, hwkeymask_len);
    int pad = (hwkey_len % 16) ? (16 - (hwkey_len % 16)) : 0;
    sdk::asic::pd::asicpd_tcam_table_entry_write(tableid, index, hwkey,
                                                 hwkey_y, hwkey_len + pad);
    return (P4PD_SUCCESS);
}
//::        elif pddict['tables'][table]['type'] == 'Index' or pddict['tables'][table]['type'] == 'Mpu':
static p4pd_error_t
${table}_entry_write(uint32_t tableid,
                     uint32_t index,
                     ${table}_actiondata_t *actiondata,
                     ${table}_actiondata_t *actiondata_mask)
{
    uint8_t  action_pc;
    uint8_t  packed_data[P4PD_MAX_ACTION_DATA_LEN] = {0};
    uint8_t  *packed_actiondata = packed_data;
    uint8_t  packed_actiondata_mask[P4PD_MAX_ACTION_DATA_LEN] = {0};
    uint8_t  hwentry[P4PD_MAX_MATCHKEY_LEN + P4PD_MAX_ACTION_DATA_LEN] = {0};
    uint8_t  hwentry_mask[P4PD_MAX_MATCHKEY_LEN + P4PD_MAX_ACTION_DATA_LEN] = {0};
    uint8_t  *_hwentry_mask = NULL;
    uint16_t entry_size, actiondatalen;
    (void)_hwentry_mask;
//::            if len(pddict['tables'][table]['actions']) > 1:
//::                add_action_pc = True
//::            else:
//::                add_action_pc = False
//::            #endif
//::            if add_action_pc:
    action_pc = sdk::asic::pd::asicpd_get_action_pc(tableid, actiondata->action_id);
    assert(action_pc != 0xff);
//::            else:
    action_pc = 0xff;
//::            #endif
    if (actiondata_mask) {
        actiondatalen = ${table}_pack_action_data(tableid, actiondata_mask,
                                                  packed_actiondata_mask);
        entry_size = p4pd_utils_p4table_entry_prepare(hwentry_mask,
                                                action_pc,
                                                NULL /* Index Table. No MatchKey*/,
                                                0, /* Zero matchkeylen */
                                                packed_actiondata_mask,
                                                actiondatalen);
    }
    actiondatalen = ${table}_pack_action_data(tableid, actiondata,
                                              packed_actiondata);

    entry_size = p4pd_utils_p4table_entry_prepare(hwentry,
                                            action_pc,
                                            NULL /* Index Table. No MatchKey*/,
                                            0, /* Zero matchkeylen */
                                            packed_actiondata,
                                            actiondatalen);

//::            if pddict['tables'][table]['location'] == 'HBM':
    sdk::asic::pd::asicpd_hbm_table_entry_write(tableid, index,
                                                hwentry, entry_size);
//::            else:
    p4pd_utils_swizzle_bytes(hwentry, entry_size);
    if (actiondata_mask) {
        p4pd_utils_swizzle_bytes(hwentry_mask, entry_size);
        _hwentry_mask = hwentry_mask;
    }

    sdk::asic::pd::asicpd_table_entry_write(tableid, index, hwentry,
                                            entry_size, _hwentry_mask);
//::            #endif
    return (P4PD_SUCCESS);
}
//::        else:
static p4pd_error_t
${table}_entry_write(uint32_t tableid,
                     uint32_t hashindex,
                     uint8_t *hwkey,
                     ${table}_actiondata_t *actiondata,
                     ${table}_actiondata_t *actiondata_mask)
{
    uint32_t hwactiondata_len, hwkey_len;
    uint8_t  action_pc;
    uint8_t  packed_actiondata_after_key[P4PD_MAX_ACTION_DATA_LEN] = {0};
    uint8_t  packed_data[P4PD_MAX_ACTION_DATA_LEN] = {0};
    uint8_t  *packed_actiondata_before_key = packed_data;
//::            if pddict['tables'][table]['is_wide_key']:
    uint8_t  hwentry[P4PD_MAX_PHV_LEN] = {0};
//::            else:
    uint8_t  hwentry[P4PD_MAX_MATCHKEY_LEN + P4PD_MAX_ACTION_DATA_LEN] = {0};
//::            #endif
    uint16_t entry_size, actiondatalen, key_len, axi_shift_len;
    uint16_t actiondata_len_before_key, actiondata_len_after_key;
    uint8_t  *_hwentry = &hwentry[0];
    (void)packed_actiondata_before_key;
    (void)actiondata_len_after_key;
    (void)actiondata_len_before_key;
    (void)actiondatalen;
    (void)hwkey_len; // always 512 for hash tables..
    if (actiondata_mask) {
        return P4PD_FAIL;
    }
    ${table}_hwentry_query(tableid, &hwkey_len, &hwactiondata_len);
    hash_${table}_key_len(tableid, &key_len);
//::            if len(pddict['tables'][table]['actions']) > 1:
//::                add_action_pc = True
//::            else:
//::                add_action_pc = False
//::            #endif
//::            if add_action_pc:
    action_pc = sdk::asic::pd::asicpd_get_action_pc(tableid, actiondata->action_id);
    assert(action_pc != 0xff);
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

    /* p4pd_utils_hash_table_entry_prepare() function expects the packed_actiondata
     * without the space reserved for actionpc, but sorrento generates the dvector with
     * space reserved for action pc. So increment the packed_actiondata_before_key location
     * by a byte size for action pc.
     */
    if (action_pc != 0xff) {
        packed_actiondata_before_key += (P4PD_ACTIONPC_BITS/8);
    }

//::            if pddict['tables'][table]['is_wide_key']:
    entry_size = p4pd_utils_widekey_hash_table_entry_prepare(hwentry,
                                             action_pc,
                                             ${pddict['tables'][table]['match_key_start_bit']},/*MatchKeyStartBit */
                                             hwkey,
                                             key_len,
                                             actiondatalen,
                                             packed_actiondata_before_key,
                                             actiondata_len_before_key,
                                             packed_actiondata_after_key,
                                             actiondata_len_after_key,
                                             &axi_shift_len);

    if (axi_shift_len) {
        /* Due to leading axi_shift space, actual entry line
         * does not start at byte zero.
         */
        _hwentry += axi_shift_len;
    }
//::            else:
    entry_size = p4pd_utils_hash_table_entry_prepare(hwentry,
                                             action_pc,
                                             ${pddict['tables'][table]['match_key_start_bit']},/*MatchKeyStartBit */
                                             hwkey,
                                             key_len,
                                             actiondatalen,
                                             packed_actiondata_before_key,
                                             actiondata_len_before_key,
                                             packed_actiondata_after_key,
                                             actiondata_len_after_key,
                                             &axi_shift_len);
    if (axi_shift_len) {
        /* Due to leading axi_shift space, actual entry line
         * does not start at byte zero.
         */
        _hwentry += axi_shift_len;
    }
//::            #endif
//::            if pddict['tables'][table]['location'] == 'HBM':
    sdk::asic::pd::asicpd_hbm_table_entry_write(tableid, hashindex,
                                                _hwentry, entry_size);
//::            else:
    p4pd_utils_swizzle_bytes(_hwentry, entry_size);
    sdk::asic::pd::asicpd_table_entry_write(tableid, hashindex, _hwentry,
                                            entry_size, NULL);
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
                                 ${table}_actiondata_t *actiondata)
{
    uint16_t src_start_bit = 0, dest_start_bit = 0;
    uint16_t actionfldwidth = 0;
    uint16_t actiondatalen = 0;
    uint8_t *packed_action_data;
    (void)src_start_bit;
    (void)*packed_action_data;
    (void)dest_start_bit;
    (void)src_start_bit;

    memset(actiondata, 0, sizeof(${table}_actiondata_t));
    actiondata->action_id = actionid;
//::            mat_key_start_bit = pddict['tables'][table]['match_key_start_bit']
//::            mat_key_bit_length = pddict['tables'][table]['match_key_bit_length']
//::            actiondatalen = 0
//::            if pddict['tables'][table]['is_wide_key']:
//::                mat_key_bit_length = pddict['tables'][table]['wide_key_len']
//::            #endif
//::            apc_offset = 0
//::            if len(pddict['tables'][table]['actions']) > 1:
//::                apc_offset = 8
//::            #endif

    switch(actiondata->action_id) {
//::            for action in pddict['tables'][table]['actions']:
//::                (actionname, actionflddict, _) = action
//::                actname = actionname.upper()
//::                if not len(actionflddict):
//::                    continue
//::                #endif
        case ${tbl}_${actname}_ID:

//::                iter = 0
//::                while iter < len(actionflddict):
//::                    actionfld      = actionflddict[iter]
//::                    actionfldname  = actionfld['p4_name']
//::                    actionfldwidth = actionfld['len']
//::                    src_start_bit  = actionfld['dvec_start'] - apc_offset
//::                    # need to compute adata_start offset within the last byte of hwentry
//::                    # where key does not end on byte boundary
//::                    end_key_bit = mat_key_start_bit + mat_key_bit_length
//::                    after_key_bit_offset = end_key_bit % 8
//::                    dest_start_bit = 0
//::                    if ((iter + 1) < len(actionflddict)):
//::                        nextactionfld     = actionflddict[iter + 1]
//::                        nextactionfldname = nextactionfld['p4_name']
//::                        if actionfldname == nextactionfldname:
//::                            dest_start_bit = nextactionfld['len']
//::                        #endif
//::                    #endif

            /* Field: ${actionfldname} */
            dest_start_bit     = ${dest_start_bit};
            actionfldwidth     = ${actionfldwidth};
            actiondatalen      += actionfldwidth;
//::                    if src_start_bit < mat_key_start_bit:
            src_start_bit      = ${src_start_bit};
            packed_action_data = packed_actiondata_before_key;
//::                    else:
            src_start_bit      = ${src_start_bit} - ${end_key_bit} + ${after_key_bit_offset};
            packed_action_data = packed_actiondata_after_key;
//::                    #endif

            p4pd_utils_copy_be_adata_to_le_dest(
//::                    if actionfldwidth <= 32:
                    (uint8_t*)&(actiondata->action_u.${table}_${actionname}.${actionfldname}),
//::                    else:
                    (uint8_t*)(actiondata->action_u.${table}_${actionname}.${actionfldname}),
//::                    #endif
                    dest_start_bit, packed_action_data, src_start_bit, actionfldwidth);
//::                    actiondatalen += actionfldwidth
//::                    iter = iter + 1
//::                #endwhile
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
                            ${table}_actiondata_t *actiondata)
{
    uint16_t src_start_bit;
    uint16_t actionfldwidth;
    uint16_t actiondatalen = 0;

    (void)src_start_bit;
    (void)actionfldwidth;

    src_start_bit = 0;
    actionfldwidth = 0;

    memset(actiondata, 0, sizeof(${table}_actiondata_t));
    actiondata->action_id = actionid;

    switch(actiondata->action_id) {
//::            apc_offset = 0
//::            if len(pddict['tables'][table]['actions']) > 1:
//::                apc_offset = 8
//::            #endif
//::            actiondatalen = 0
//::            for action in pddict['tables'][table]['actions']:
//::                (actionname, actionflddict, _) = action
//::                actname = actionname.upper()
//::                if not len(actionflddict):
//::                    continue
//::                #endif
        case ${tbl}_${actname}_ID:
//::                for actionfld in actionflddict:
//::                    actionfldname  = actionfld['p4_name']
//::                    actionfldwidth = actionfld['len']
//::                    src_start_bit  = actionfld['dvec_start'] - apc_offset

            /* Field: ${actionfldname} */
            src_start_bit      = ${src_start_bit};
            actionfldwidth     = ${actionfldwidth};
            actiondatalen      += actionfldwidth;

            p4pd_utils_copy_be_adata_to_le_dest(
//::                    if actionfldwidth <= 32:
                    (uint8_t*)&(actiondata->action_u.${table}_${actionname}.${actionfldname}),
//::                    else:
                    (uint8_t*)(actiondata->action_u.${table}_${actionname}.${actionfldname}),
//::                    #endif
                    0, packed_actiondata, src_start_bit, actionfldwidth);
//::                actiondatalen += actionfldwidth
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
//::                for fields in pddict['tables'][table]['hdrunion_keys']:
//::                    (p4fldname, p4fldwidth, mask, key_byte_format, key_bit_format) = fields
//::                    if len(key_byte_format):
//::                        for kmbyte, kbit, _, _ in key_byte_format:
//::                            tablebyte = kmbyte
     * [${p4fldname}, ${kbit},  ${tablebyte}, ${kmbyte}. Sourced from hdr union key]
//::                        #endfor
//::                    #endif
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
//::                for fields in pddict['tables'][table]['hdrunion_keys']:
//::                    (p4fldname, p4fldwidth, mask, key_byte_format, key_bit_format) = fields
//::                    if len(key_bit_format):
//::                        for kmbit, kbit in key_bit_format:
//::                            tablebit = kmbit
     * [ ${p4fldname}, ${kbit},  ${tablebit}, ${kmbit}. Sourced from hdr union key]
//::                        #endfor
//::                    #endif
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
    p4pd_utils_copy_be_src_to_le_dest(&trit_x,
                           0,
                           hw_key,
                           (${tablebyte} * 8) + ${containerstart}  - ${mat_key_start_bit}, /* source bit position */
                           ${width});

    p4pd_utils_copy_be_src_to_le_dest(&trit_y,
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
    p4pd_utils_copy_single_bit(&trit_x,
                           0,
                           hw_key + (${kmbit - mat_key_start_bit} >> 3),
                           ((${kmbit} - (${kmbit} % 8)) + (7 - (${kmbit} % 8))) - (${mat_key_start_bit}), /* Source bit position */
                           1 /* bits to copy */);
    p4pd_utils_copy_single_bit(&trit_y,
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
    p4pd_utils_copy_be_src_to_le_dest(&trit_x,
                           0,
                           hw_key,
                           (${tablebyte} * 8) + ${containerstart} - ${mat_key_start_bit}, /* source bit position */
                           ${width});

    p4pd_utils_copy_be_src_to_le_dest(&trit_y,
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
    p4pd_utils_copy_single_bit(&trit_x,
                           0,
                           hw_key + (${kmbit - mat_key_start_bit} >> 8),
                           ((${kmbit} - (${kmbit} % 8)) + (7 - (${kmbit} % 8))) - (${mat_key_start_bit}), /* Source bit position */
                           1 /* bits to copy */);
    p4pd_utils_copy_single_bit(&trit_y,
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
//::                for fields in pddict['tables'][table]['hdrunion_keys']:
//::                    ustr=''
//::                    ustr_mask=''
//::                    (p4fldname, p4fldwidth, mask, key_byte_format, key_bit_format) = fields
//::                    if len(key_byte_format):
//::                        for kmbyte, _kbit, width, containerstart in key_byte_format:
//::                            tablebyte = kmbyte
//::                            kbyte = (p4fldwidth - 1 - _kbit) / 8
//::                            kbit = (p4fldwidth - 1 - _kbit) % 8
    /* Header Union Key byte */
    trit_x = 0;
    trit_y = 0;
    p4pd_utils_copy_be_src_to_le_dest(&trit_x,
                           0,
                           hw_key,
                           (${tablebyte} * 8) + ${containerstart} - ${mat_key_start_bit}, /* source bit position */
                           ${width});

    p4pd_utils_copy_be_src_to_le_dest(&trit_y,
                           0,
                           hw_key_mask,
                           (${tablebyte} * 8) + ${containerstart} - ${mat_key_start_bit}, /* source bit position */
                           ${width});
    m = trit_x ^ trit_y;
    k = trit_x & m;
//::                            kbyte = kbit/8
//::                            if p4fldwidth <= 32:
//::                                if kbit < 7:
    /* P4 field is extended.. a less than byte field is extended to byte */
//::                                #endif
    *((uint8_t*)&(swkey->${ustr}${p4fldname}) + ${kbyte}) = k;
    *((uint8_t*)&(swkey_mask->${ustr_mask}${p4fldname}_mask) + ${kbyte}) = m;
//::                            else:
//::                                if kbit < 7:
    /* P4 field is extended.. a less than byte field is extended to byte */
//::                                #endif
    *((uint8_t*)&(swkey->${ustr}${p4fldname}[${kbyte}])) = k;
    *((uint8_t*)&(swkey_mask->${ustr_mask}${p4fldname}_mask[${kbyte}])) = m;
//::                            #endif
//::                        #endfor
//::                    #endif
//::                    if len(key_bit_format):
//::                        for kmbit, _kbit in key_bit_format:
//::                            kbyte = (p4fldwidth - 1 - _kbit) / 8
//::                            kbit = (p4fldwidth - 1 - _kbit) % 8
    /* Header Union Key bit */
    trit_x = 0;
    trit_y = 0;
    p4pd_utils_copy_single_bit(&trit_x,
                           0,
                           hw_key + (${kmbit - mat_key_start_bit} >> 3),
                           ((${kmbit} - (${kmbit} % 8)) + (7 - (${kmbit} % 8))) - (${mat_key_start_bit}), /* Source bit position */
                           1 /* bits to copy */);
    p4pd_utils_copy_single_bit(&trit_y,
                           0,
                           hw_key_mask + (${kmbit - mat_key_start_bit} >> 3),
                           ((${kmbit} - (${kmbit} % 8)) + (7 - (${kmbit} % 8))) - (${mat_key_start_bit}), /* Source bit position */
                           1 /* bits to copy */);
    m = trit_x ^ trit_y;
    k = trit_x & m;
//::                            if p4fldwidth <= 32:
    *((uint8_t*)&(swkey->${ustr}${p4fldname}) + ${kbyte}) &= ~(1 << ${kbit});
    *((uint8_t*)&(swkey->${ustr}${p4fldname}) + ${kbyte}) |= k << (${kbit});
    *((uint8_t*)&(swkey_mask->${ustr_mask}${p4fldname}_mask) + ${kbyte}) &= ~(1 << ${kbit});
    *((uint8_t*)&(swkey_mask->${ustr_mask}${p4fldname}_mask) + ${kbyte}) |= m << (${kbit});
//::                            else:
    *((uint8_t*)&(swkey->${ustr}${p4fldname}[${kbyte}])) &= ~(1 << ${kbit});
    *((uint8_t*)&(swkey->${ustr}${p4fldname}[${kbyte}])) |= k << (${kbit});
    *((uint8_t*)&(swkey_mask->{ustr}${p4fldname}_mask[${kbyte}])) &= ~(1 << ${kbit});
    *((uint8_t*)&(swkey_mask->{ustr}${p4fldname}_mask[${kbyte}])) |= m << (${kbit});
//::                            #endif
//::                        #endfor
//::                    #endif
//::                #endfor
    return (hwkey_len);
}
//::            else:
static uint32_t
${table}_hwkey_unbuild(uint32_t tableid,
                       uint8_t *hwkey,
                       uint16_t hwkey_len,
                       ${table}_swkey_t *swkey,
                       uint16_t axi_shift_bytes)
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
//::                        (p4fldname, p4fldwidth, mask, key_byte_format, key_bit_format) = fields
//::                        if len(key_byte_format):
//::                            for kmbyte, kbit, _, _ in key_byte_format:
//::                                tablebyte = kmbyte
     * [${p4fldname}, ${kbit},     ${tablebyte}, ${kmbyte} ; Sourced from fld union key]
//::                            #endfor
//::                        #endif
//::                    #endfor
//::                #endfor
//::                for fields in pddict['tables'][table]['hdrunion_keys']:
//::                    (p4fldname, p4fldwidth, mask, key_byte_format, key_bit_format) = fields
//::                    if len(key_bit_format):
//::                        for kmbyte, kbit, _, _ in key_byte_format:
//::                            tablebyte = kmbyte
     * [${p4fldname}, ${kbit},  ${tablebyte}, ${kmbyte} ; Sourced from hdr union key]
//::                        #endfor
//::                    #endif
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
//::                for fields in pddict['tables'][table]['hdrunion_keys']:
//::                    (p4fldname, p4fldwidth, mask, key_byte_format, key_bit_format) = fields
//::                    if len(key_bit_format):
//::                        for kmbit, kbit in key_bit_format:
//::                            tablebit = kmbit
     * [ ${p4fldname}, ${kbit},  ${tablebit}, ${kmbit}. Sourced from hdr union key]
//::                        #endfor
//::                    #endif
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
    p4pd_utils_copy_be_src_to_le_dest(
//::                            if p4fldwidth <= 32:
                   (uint8_t*)((uint8_t*)&(swkey->${p4fldname}) + ${kbyte}),
//::                            else:
                   &(swkey->${p4fldname}[${kbyte}]),
//::                            #endif
                   (${p4fldwidth} - ${kbit}) % 8, /* Start bit in destination */
                   hwkey,
                   (${tablebyte} * 8) + ${containerstart} - ${mat_key_start_bit} - (axi_shift_bytes * 8), /* source bit position */
                   ${width});
//::                        #endfor
//::                    #endif
//::                    if len(key_bit_format):
//::                        for kmbit, kbit in key_bit_format:
    /* Copying one bit from table-key into correct place */
    p4pd_utils_copy_single_bit(
//::                            if p4fldwidth <= 32:
                   (uint8_t*)((uint8_t*)&(swkey->${p4fldname}) + ((${p4fldwidth} - ${kbit})/8)),
//::                            else:
                   &(swkey->${p4fldname}[${kbit}/8]),
//::                            #endif
                   (${p4fldwidth} - 1 - ${kbit}) % 8 /* start bit in destination */,
                   hwkey + (${kmbit - mat_key_start_bit} >> 3) - axi_shift_bytes,
                   ((${kmbit} - (${kmbit} % 8)) + (7 - (${kmbit} % 8)))- (${mat_key_start_bit}) - (axi_shift_bytes * 8), /* Source bit position */
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
    p4pd_utils_copy_be_src_to_le_dest(
//::                                if p4fldwidth <= 32:
                   (uint8_t*)((uint8_t*)&(swkey->${ustr}${p4fldname}) + ${kbyte}),
//::                                else:
                   &(swkey->${ustr}${p4fldname}[${kbyte}]),
//::                                #endif
                   (${p4fldwidth} - ${kbit}) % 8, /* Start bit in destination */
                   hwkey,
                   (${tablebyte} * 8) + ${containerstart} - ${mat_key_start_bit} - (axi_shift_bytes * 8), /* source bit position */
                   ${width});
//::                            #endfor
//::                        #endif
//::                        if len(key_bit_format):
//::                            for kmbit, kbit in key_bit_format:
    /* Copying one bit from table-key into correct place */
    p4pd_utils_copy_single_bit(
//::                                if p4fldwidth <= 32:
                   (uint8_t*)((uint8_t*)&(swkey->${ustr}${p4fldname}) + ((${p4fldwidth} - ${kbit})/8)),
//::                                else:
                   &(swkey->${ustr}${p4fldname}[${kbit}/8]),
//::                                #endif
                   (${p4fldwidth} - 1 - ${kbit}) % 8 /* start bit in destination */,
                   hwkey + (${kmbit - mat_key_start_bit} >> 3) - axi_shift_bytes,
                   ((${kmbit} - (${kmbit} % 8)) + (7 - (${kmbit} % 8)))- (${mat_key_start_bit}) - (axi_shift_bytes * 8), /* Source bit position */
                   1 /* copy single bit */);
//::                            #endfor
//::                        #endif
//::                    #endfor
//::                    i += 1
//::                #endfor
//::                for fields in pddict['tables'][table]['hdrunion_keys']:
//::                    ustr=''
//::                    (p4fldname, p4fldwidth, mask, key_byte_format, key_bit_format) = fields
//::                    if len(key_byte_format):
//::                        for kmbyte, kbit, width, containerstart in key_byte_format:
//::                            kbyte = (p4fldwidth - 1 - kbit) / 8
//::                            tablebyte = kmbyte
    /* Copying one byte from table-key into correct place */
    p4pd_utils_copy_be_src_to_le_dest(
//::                            if p4fldwidth <= 32:
                   (uint8_t*)((uint8_t*)&(swkey->${ustr}${p4fldname}) + ${kbyte}),
//::                            else:
                   &(swkey->${ustr}${p4fldname}[${kbyte}]),
//::                            #endif
                   (${p4fldwidth} - ${kbit}) % 8, /* Start bit in destination */
                   hwkey,
                   (${tablebyte} * 8) + ${containerstart} - ${mat_key_start_bit} - (axi_shift_bytes * 8), /* source bit position */
                   ${width});
//::                        #endfor
//::                    #endif
//::                    if len(key_bit_format):
//::                        for kmbit, kbit in key_bit_format:
    /* Copying one bit from table-key into correct place */
    p4pd_utils_copy_single_bit(
//::                            if p4fldwidth <= 32:
                   (uint8_t*)((uint8_t*)&(swkey->${ustr}${p4fldname}) + ((${p4fldwidth} - ${kbit})/8)),
//::                            else:
                   &(swkey->${ustr}${p4fldname}[${kbit}/8]),
//::                            #endif
                   (${p4fldwidth} - 1 - ${kbit}) % 8 /* start bit in destination */,
                   hwkey + (${kmbit - mat_key_start_bit} >> 3) - axi_shift_bytes,
                   ((${kmbit} - (${kmbit} % 8)) + (7 - (${kmbit} % 8)))- (${mat_key_start_bit}) - (axi_shift_bytes * 8), /* Source bit position */
                   1 /* copy single bit */);
//::                        #endfor
//::                    #endif
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
//::        if pddict['tables'][table]['type'] == 'Ternary' and pddict['tables'][table]['otcam']:
static p4pd_error_t
${table}_entry_read(uint32_t tableid,
                    uint32_t index,
                    ${table}_swkey_t *swkey,
                    ${table}_swkey_mask_t *swkey_mask,
                    ${table}_actiondata_t *actiondata)
{
    uint8_t  hwentry_x[P4PD_MAX_MATCHKEY_LEN + P4PD_MAX_ACTION_DATA_LEN] = {0};
    uint8_t  hwentry_y[P4PD_MAX_MATCHKEY_LEN + P4PD_MAX_ACTION_DATA_LEN] = {0};
    uint8_t  hwentry[P4PD_MAX_MATCHKEY_LEN + P4PD_MAX_ACTION_DATA_LEN] = {0};
    uint16_t hwentry_bit_len;
    sdk::asic::pd::asicpd_tcam_table_hw_entry_read(tableid, index, hwentry_x,
                                                   hwentry_y, &hwentry_bit_len);
    if (!hwentry_bit_len) {
        return (P4PD_FAIL);
    }
    int pad = (hwentry_bit_len % 16) ? (16 - (hwentry_bit_len % 16)) : 0;
    p4pd_utils_swizzle_bytes(hwentry_x, hwentry_bit_len + pad);
    p4pd_utils_swizzle_bytes(hwentry_y, hwentry_bit_len + pad);
    ${table}_hwkey_hwmask_unbuild(tableid, hwentry_x, hwentry_y, hwentry_bit_len,
                                  swkey, swkey_mask);
    sdk::asic::pd::asicpd_table_hw_entry_read(tableid,
                       index + ${pddict['tables'][table]['parent_hash_table_size']},
                       hwentry, &hwentry_bit_len);
    if (!hwentry_bit_len) {
        return (P4PD_SUCCESS);
    }
    pad = (hwentry_bit_len % 16) ? (16 - (hwentry_bit_len % 16)) : 0;
    p4pd_utils_swizzle_bytes(hwentry, hwentry_bit_len+pad);
//::            if len(pddict['tables'][table]['actions']) > 1:
//::                action_pc_added = True
//::            else:
//::                action_pc_added = False
//::            #endif
//::            if action_pc_added:
    uint8_t actionpc = hwentry[0]; // First byte is always action-pc
    actiondata->action_id = sdk::asic::pd::asicpd_get_action_id(tableid, actionpc);
    int adatabyte = 1;
//::            else:
    actiondata->action_id = 0;
    int adatabyte = 0;
//::            #endif
    ${table}_unpack_action_data(tableid, actiondata->action_id,
                                hwentry + adatabyte, actiondata);
    return (P4PD_SUCCESS);
}
//::        elif pddict['tables'][table]['type'] == 'Ternary':
static p4pd_error_t
${table}_entry_read(uint32_t tableid,
                    uint32_t index,
                    ${table}_swkey_t *swkey,
                    ${table}_swkey_mask_t *swkey_mask,
                    ${table}_actiondata_t *actiondata)
{
    uint8_t  hwentry_x[P4PD_MAX_MATCHKEY_LEN + P4PD_MAX_ACTION_DATA_LEN] = {0};
    uint8_t  hwentry_y[P4PD_MAX_MATCHKEY_LEN + P4PD_MAX_ACTION_DATA_LEN] = {0};
    uint8_t  hwentry[P4PD_MAX_MATCHKEY_LEN + P4PD_MAX_ACTION_DATA_LEN] = {0};
    uint16_t hwentry_bit_len;
    sdk::asic::pd::asicpd_tcam_table_hw_entry_read(tableid, index, hwentry_x,
                                                   hwentry_y, &hwentry_bit_len);
    if (!hwentry_bit_len) {
        return (P4PD_FAIL);
    }
    int pad = (hwentry_bit_len % 16) ? (16 - (hwentry_bit_len % 16)) : 0;
    p4pd_utils_swizzle_bytes(hwentry_x, hwentry_bit_len + pad);
    p4pd_utils_swizzle_bytes(hwentry_y, hwentry_bit_len + pad);
    ${table}_hwkey_hwmask_unbuild(tableid, hwentry_x, hwentry_y, hwentry_bit_len,
                                  swkey, swkey_mask);
    sdk::asic::pd::asicpd_table_hw_entry_read(tableid, index,
                                              hwentry, &hwentry_bit_len);
    if (!hwentry_bit_len) {
        return (P4PD_SUCCESS);
    }
    pad = (hwentry_bit_len % 16) ? (16 - (hwentry_bit_len % 16)) : 0;
    p4pd_utils_swizzle_bytes(hwentry, hwentry_bit_len+pad);
//::            if len(pddict['tables'][table]['actions']) > 1:
//::                action_pc_added = True
//::            else:
//::                action_pc_added = False
//::            #endif
//::            if action_pc_added:
    uint8_t actionpc = hwentry[0]; // First byte is always action-pc
    actiondata->action_id = sdk::asic::pd::asicpd_get_action_id(tableid, actionpc);
    int adatabyte = 1;
//::            else:
    actiondata->action_id = 0;
    int adatabyte = 0;
//::            #endif
    ${table}_unpack_action_data(tableid, actiondata->action_id,
                                hwentry + adatabyte, actiondata);
    return (P4PD_SUCCESS);
}
//::        elif pddict['tables'][table]['type'] == 'Index' or pddict['tables'][table]['type'] == 'Mpu':
static p4pd_error_t
${table}_entry_read(uint32_t tableid,
                    uint32_t index,
                    ${table}_actiondata_t* actiondata)
{
    uint8_t  hwentry[P4PD_MAX_MATCHKEY_LEN + P4PD_MAX_ACTION_DATA_LEN] = {0};
    uint16_t hwentry_bit_len;
    uint8_t *packed_actiondata_after_key;
    uint16_t actiondata_len_after_key, key_bit_len;
    (void)packed_actiondata_after_key;
    (void)actiondata_len_after_key;
    (void)key_bit_len;
//::            if pddict['tables'][table]['location'] == 'HBM':
    sdk::asic::pd::asicpd_hbm_table_entry_read(tableid, index,
                                               hwentry, &hwentry_bit_len);
//::            else:
    sdk::asic::pd::asicpd_table_hw_entry_read(tableid, index,
                                              hwentry, &hwentry_bit_len);
    p4pd_utils_swizzle_bytes(hwentry, hwentry_bit_len);
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
    actiondata->action_id = sdk::asic::pd::asicpd_get_action_id(tableid, actionpc);
    int adatabyte = 1;
//::            else:
    actiondata->action_id = 0;
    int adatabyte = 0;
//::            #endif
    ${table}_unpack_action_data(tableid, actiondata->action_id,
                                hwentry + adatabyte, actiondata);
    return (P4PD_SUCCESS);
}
//::        else:
static p4pd_error_t
${table}_entry_read(uint32_t tableid,
                    uint32_t hashindex,
                    ${table}_swkey_t *swkey,
                    ${table}_actiondata_t *actiondata)
{
//::            if pddict['tables'][table]['is_wide_key']:
    uint8_t  hwentry[P4PD_MAX_PHV_LEN] = {0};
//::            else:
    uint8_t  hwentry[P4PD_MAX_MATCHKEY_LEN + P4PD_MAX_ACTION_DATA_LEN] = {0};
//::            #endif
    uint16_t hwentry_bit_len;
    uint8_t  *packed_actiondata_before_key;
    uint8_t *packed_actiondata_after_key;
    uint16_t actiondata_len_before_key;
    uint16_t actiondata_len_after_key, key_bit_len;
    (void)key_bit_len;
//::            if pddict['tables'][table]['location'] == 'HBM':
    sdk::asic::pd::asicpd_hbm_table_entry_read(tableid, hashindex,
                                               hwentry, &hwentry_bit_len);
//::            else:
    sdk::asic::pd::asicpd_table_hw_entry_read(tableid, hashindex,
                                              hwentry, &hwentry_bit_len);
    p4pd_utils_swizzle_bytes(hwentry, hwentry_bit_len);
//::            #endif
    if (!hwentry_bit_len) {
        // Zero len!!
        return (P4PD_SUCCESS);
    }
//::            mat_key_start_byte = pddict['tables'][table]['match_key_start_byte']
//::            mat_key_start_bit = pddict['tables'][table]['match_key_start_bit']
//::            mat_key_bit_length = pddict['tables'][table]['match_key_bit_length']
//::            if pddict['tables'][table]['is_wide_key']:
//::                mat_key_start_byte = (keylen - (keylen % 512)) / 8
//::                mat_key_bit_length = pddict['tables'][table]['wide_key_len']
//::            #endif
//::            spilled_adata_bits = 0
//::            delta_bits = 0
//::            if mat_key_start_bit > 0 and max_actionfld_len < mat_key_start_bit and (mat_key_start_bit - (max_actionfld_len - (max_actionfld_len % 16))) > 16:
//::                spilled_adata_bits = max_actionfld_len % 16
//::            #endif
//::            if len(pddict['tables'][table]['actions']) > 1:
//::                action_pc_added = True
//::                max_adata_bits_before_key = mat_key_start_bit
//::                max_adata_bits_before_key -= 8
//::            else:
//::                action_pc_added = False
//::                max_adata_bits_before_key = mat_key_start_bit
//::            #endif
//::            if action_pc_added:
//::                delta_bits = mat_key_start_bit - (max_actionfld_len - spilled_adata_bits) - 8 if max_actionfld_len < mat_key_start_bit else 0
    uint8_t actionpc = hwentry[0]; // First byte is always action-pc
    actiondata->action_id = sdk::asic::pd::asicpd_get_action_id(tableid, actionpc);
    int adatabyte = 1;
//::            else:
//::                delta_bits = mat_key_start_bit - (max_actionfld_len - spilled_adata_bits) if max_actionfld_len < mat_key_start_bit else 0
    actiondata->action_id = 0;
    int adatabyte = 0;
//::            #endif
//::            max_adata_bits_before_key -= delta_bits
    uint16_t axi_shift_bytes = ((${delta_bits} >> 4) << 1);
    // when axi_shift_bytes > 0, table entry is left shifted by positive number of bytes.
    ${table}_hwkey_unbuild(tableid, hwentry,
                           ${mat_key_bit_length}, swkey, axi_shift_bytes);
    packed_actiondata_before_key = (hwentry + adatabyte);
    packed_actiondata_after_key = (hwentry + ${mat_key_start_byte} + (${mat_key_bit_length} >> 3));
    actiondata_len_before_key = ${max_adata_bits_before_key};
    actiondata_len_after_key = ${max_actionfld_len} - ${max_adata_bits_before_key};
    hash_${table}_unpack_action_data(tableid,
                                    actiondata->action_id,
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
                      ${table}_actiondata_t* actiondata)
{
//::            if len(pddict['tables'][table]['actions']) > 1:
//::                action_pc_added = True
//::            else:
//::                action_pc_added = False
//::            #endif
//::            if action_pc_added:
    uint8_t actionpc = hwentry[0]; // First byte is always action-pc
    actiondata->action_id = sdk::asic::pd::asicpd_get_action_id(tableid, actionpc);
    int adatabyte = 1;
//::            else:
    actiondata->action_id = 0;
    int adatabyte = 0;
//::            #endif
    ${table}_unpack_action_data(tableid, actiondata->action_id,
                                hwentry + adatabyte, actiondata);
    return (P4PD_SUCCESS);
}
//::        else:
static p4pd_error_t
${table}_entry_decode(uint32_t tableid,
                      uint8_t *hwentry,
                      uint16_t hwentry_len,
                      ${table}_swkey_t *swkey,
                      ${table}_actiondata_t *actiondata)
{
//::            mat_key_start_byte = pddict['tables'][table]['match_key_start_byte']
//::            mat_key_start_bit = pddict['tables'][table]['match_key_start_bit']
//::            mat_key_bit_length = pddict['tables'][table]['match_key_bit_length']
//::            if pddict['tables'][table]['is_wide_key']:
//::                mat_key_start_byte = (keylen - (keylen % 512)) / 8
//::                mat_key_bit_length = pddict['tables'][table]['wide_key_len']
//::            #endif
//::            spilled_adata_bits = 0
//::            delta_bits = 0
//::            if mat_key_start_bit > 0 and max_actionfld_len < mat_key_start_bit and (mat_key_start_bit - (max_actionfld_len - (max_actionfld_len % 16))) > 16:
//::                spilled_adata_bits = max_actionfld_len % 16
//::            #endif
//::            if len(pddict['tables'][table]['actions']) > 1:
//::                action_pc_added = True
//::                max_adata_bits_before_key = mat_key_start_bit
//::                max_adata_bits_before_key -= 8
//::            else:
//::                action_pc_added = False
//::                max_adata_bits_before_key = mat_key_start_bit
//::            #endif
//::            if action_pc_added:
//::                delta_bits = mat_key_start_bit - (max_actionfld_len - spilled_adata_bits) - 8 if max_actionfld_len < mat_key_start_bit else 0
    uint8_t actionpc = hwentry[0]; // First byte is always action-pc
    actiondata->action_id = sdk::asic::pd::asicpd_get_action_id(tableid, actionpc);
    int adatabyte = 1;
//::            else:
//::                delta_bits = mat_key_start_bit - (max_actionfld_len - spilled_adata_bits) if max_actionfld_len < mat_key_start_bit else 0
    actiondata->action_id = 0;
    int adatabyte = 0;
//::            #endif
//::            max_adata_bits_before_key -= delta_bits
    uint16_t axi_shift_bytes = ((${delta_bits} >> 4) << 1);
    // when axi_shift_bytes > 0, table entry is left shifted by positive number of bytes.
    ${table}_hwkey_unbuild(tableid, hwentry, hwentry_len, swkey, axi_shift_bytes);
    uint8_t *packed_actiondata_before_key;
    uint8_t *packed_actiondata_after_key;
    uint16_t actiondata_len_before_key;
    uint16_t actiondata_len_after_key;
    packed_actiondata_before_key = (hwentry + adatabyte);
    packed_actiondata_after_key = (hwentry + ${mat_key_start_byte} + (${mat_key_bit_length} >> 3));
    actiondata_len_before_key = ${max_adata_bits_before_key}; // bit len without actionpc
    actiondata_len_after_key = ${max_actionfld_len} - ${max_adata_bits_before_key};
    hash_${table}_unpack_action_data(tableid,
                                    actiondata->action_id,
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
//::            if pddict['p4plus_module']:
//::                api_prefix = 'p4pd_' + pddict['p4plus_module']
//::            else:
//::                api_prefix = 'p4pd_' + pddict['p4program']
//::            #endif
//::        else:
//::            api_prefix = 'p4pd'
//::        #endif
//::        genhwfields_actiondata_api = False
//::        for table, tid in tabledict.items():
//::            if 'hwfields' in pddict['tables'][table]:
//::                genhwfields_actiondata_api = True
//::                break
//::            #endif
//::        #endfor
//::        genappdatafield_api = False
//::        for table, tid in tabledict.items():
//::            if 'appdatafields' in pddict['tables'][table] and len(pddict['tables'][table]['appdatafields']) > 0:
//::                genappdatafield_api = True
//::                break
//::            #endif
//::        #endfor

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
//::            if pddict['tables'][table]['is_toeplitz_hash'] or pddict['tables'][table]['is_raw']:
//::                continue
//::            #endif
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
//::            if pddict['tables'][table]['is_toeplitz_hash'] or pddict['tables'][table]['is_raw']:
//::                continue
//::            #endif
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
//::            if pddict['tables'][table]['is_toeplitz_hash'] or pddict['tables'][table]['is_raw']:
//::                continue
//::            #endif
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

/* Install entry into P4-table with masked data (hw performs read-modify-write)
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
 *  IN  : void    *actiondata_mask : Action data mask associated with the key.
 *
 *
 * Return Value:
 *  pd_error_t                              : P4PD_SUCCESS / P4PD_FAIL
 */
p4pd_error_t
${api_prefix}_entry_write_with_datamask(uint32_t tableid,
                                        uint32_t index,
                                        uint8_t *hwkey,
                                        uint8_t *hwkey_y,
                                        void    *actiondata,
                                        void    *actiondata_mask)
{
    switch (tableid) {
//::        for table, tid in tabledict.items():
//::            if pddict['tables'][table]['is_toeplitz_hash'] or pddict['tables'][table]['is_raw']:
//::                continue
//::            #endif
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
                                         (${table}_actiondata_t*)actiondata,
                                         (${table}_actiondata_t*)actiondata_mask));
//::            #endif
//::            if pddict['tables'][table]['type'] == 'Hash' or pddict['tables'][table]['type'] == 'Hash_OTcam':
            return (${table}_entry_write(tableid, index, hwkey,
                                         (${table}_actiondata_t*)actiondata,
                                         (${table}_actiondata_t*)actiondata_mask));
//::            #endif
//::            if pddict['tables'][table]['type'] == 'Ternary':
            return (${table}_entry_write(tableid, index,
                                         hwkey, hwkey_y,
                                         (${table}_actiondata_t*)actiondata,
                                         (${table}_actiondata_t*)actiondata_mask));
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
    return ${api_prefix}_entry_write_with_datamask(tableid, index, hwkey,
                                                   hwkey_y, actiondata, NULL);
}

/* Install entry into P4-table (using sw keys).
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
 *  IN  : uint8_t *swkey         : Software key to be installed into P4-table
 *                                 Can be NULL if table id identifies index
 *                                 based lookup table.
 *  IN  : uint8_t *swkey_mask    : Key mask bits mask used in ternary matching.
 *                                 This data structure is of same type as the Key.
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
 *  pd_error_t                   : P4PD_SUCCESS / P4PD_FAIL
 */
p4pd_error_t
${api_prefix}_entry_install(uint32_t tableid,
                            uint32_t index,
                            void    *swkey,
                            void    *swkey_mask,
                            void    *actiondata)
{
    uint8_t  hwkey[P4PD_MAX_PHV_LEN] = {0};
    uint8_t  hwkey_mask[P4PD_MAX_PHV_LEN] = {0};
    p4pd_error_t ret;

    time_profile_begin(sdk::utils::time_profile::P4PD_ENTRY_INSTALL);
    ${api_prefix}_hwkey_hwmask_build(tableid, swkey, swkey_mask, hwkey, hwkey_mask);
    ret = ${api_prefix}_entry_write_with_datamask(tableid, index, hwkey,
                                                  hwkey_mask, actiondata, NULL);
    time_profile_end(sdk::utils::time_profile::P4PD_ENTRY_INSTALL);
    return ret;
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
    p4pd_error_t ret = P4PD_SUCCESS;
    time_profile_begin(sdk::utils::time_profile::P4PD_ENTRY_READ);
    switch (tableid) {
//::        for table, tid in tabledict.items():
//::            if pddict['tables'][table]['is_toeplitz_hash'] or pddict['tables'][table]['is_raw']:
//::                continue
//::            #endif
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
            ret = (${table}_entry_read(tableid, index,
                            (${table}_actiondata_t*) actiondata));
//::            #endif
//::            if pddict['tables'][table]['type'] == 'Hash' or pddict['tables'][table]['type'] == 'Hash_OTcam':
            ret = (${table}_entry_read(tableid, index,
                            (${table}_swkey_t *)swkey,
                            (${table}_actiondata_t*)actiondata));
//::            #endif
//::            if pddict['tables'][table]['type'] == 'Ternary':
            ret = (${table}_entry_read(tableid, index,
                            (${table}_swkey_t*)swkey,
                            (${table}_swkey_mask_t*)swkey_mask,
                            (${table}_actiondata_t*) actiondata));
//::            #endif
        break;
//::        #endfor
        default:
            // Invalid tableid
            ret = (P4PD_FAIL);
        break;
    }
    time_profile_end(sdk::utils::time_profile::P4PD_ENTRY_READ);
    return (ret);
}

#define MOVE_BUFFER_PTR(buf, b, blen) \
{                                     \
    buf += b;                         \
    blen -= b;                        \
    if (blen <= 0) {                  \
        return (P4PD_SUCCESS);        \
    }                                 \
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
        p4pd_utils_swizzle_bytes(_hwentry_y, hwentry_len);
    }
    memset(buffer, 0, buf_len);
    if (hwentry_y) {
        b = snprintf(buf, blen, "!!!! Decode HW table entry into Table Key !!!! \n");
    } else {
        b = snprintf(buf, blen, "!!!! Decode HW table entry into (Table Key + Actiondata)/Actiondata!!!! \n");
    }
    MOVE_BUFFER_PTR(buf, b, blen);
    switch (tableid) {
//::        for table, tid in tabledict.items():
//::            if pddict['tables'][table]['is_toeplitz_hash'] or pddict['tables'][table]['is_raw']:
//::                continue
//::            #endif
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
            p4pd_utils_swizzle_bytes(_hwentry, hwentry_len);
//::        #endif
//::            if len(pddict['tables'][table]['hash_overflow_tbl']):
            if (tableid == P4${caps_p4prog}TBL_ID_${caps_tablename}) {
                b = snprintf(buf, blen, "Table: %s, Index 0x%x/%u\n",
                             "P4TBL_ID_${caps_tablename}", index, index);
            } else {
                b = snprintf(buf, blen, "Table: %s, Index 0x%x/%u\n",
                             "P4TBL_ID_${caps_tbl_}", index, index);
            }
//::            else:
            b = snprintf(buf, blen, "Table: %s, Index 0x%x/%u\n",
                         "P4TBL_ID_${caps_tablename}", index, index);
//::            #endif
            MOVE_BUFFER_PTR(buf, b, blen);
//::            if pddict['tables'][table]['type'] == 'Index' or pddict['tables'][table]['type'] == 'Mpu':
            ${table}_actiondata_t actiondata;
            ${table}_entry_decode(tableid, _hwentry,  hwentry_len,
                                  &actiondata);
            switch(actiondata.action_id) {
//::                for action in pddict['tables'][table]['actions']:
//::                    (actionname, actionflddict, _) = action
//::                    actname = actionname.upper()
                case ${caps_tablename}_${actname}_ID:
                {
                    b = snprintf(buf, blen, "Action: %s\n", "${caps_tablename}_${actname}_ID");
                    MOVE_BUFFER_PTR(buf, b, blen);
//::                    for actionfld in actionflddict:
//::                        actionfldname  = actionfld['p4_name']
//::                        actionfldwidth = actionfld['len']
//::                        if actionfldwidth <= 32:
                    b = snprintf(buf, blen, "%s: 0x%x\n", "${actionfldname}",
                             actiondata.action_u.\
                             ${table}_${actionname}.${actionfldname});
                    MOVE_BUFFER_PTR(buf, b, blen);
//::                        else:
//::                            p4fldwidth_byte = (actionfldwidth / 8) + (1 if actionfldwidth % 8 else 0)
                    b = snprintf(buf, blen, "%s: \n", "${actionfldname}");
                    MOVE_BUFFER_PTR(buf, b, blen);
                    for (int j = 0; j < ${p4fldwidth_byte}; j++) {
                        b = snprintf(buf, blen, "  0x%x : ",
                                 actiondata.action_u.\
                                 ${table}_${actionname}.${actionfldname}[j]);
                        MOVE_BUFFER_PTR(buf, b, blen);
                    }
                    b = snprintf(buf, blen, "\n");
                    MOVE_BUFFER_PTR(buf, b, blen);
//::                        #endif
//::                    #endfor
                }
                break;
//::                #endfor
            }
//::            #endif
//::            if pddict['tables'][table]['type'] == 'Hash' or pddict['tables'][table]['type'] == 'Hash_OTcam':
            ${table}_swkey_t swkey;
            ${table}_actiondata_t actiondata;
            ${table}_entry_decode(tableid, _hwentry, hwentry_len,
                                  &swkey,
                                  &actiondata);
//::                for fields in pddict['tables'][table]['keys']:
//::                    (p4fldname, p4fldwidth, mask, key_byte_format, key_bit_format) = fields
//::                    if (p4fldwidth <= 32):
            b = snprintf(buf, blen, "%s: 0x%x\n", "${p4fldname}", swkey.${p4fldname});
            MOVE_BUFFER_PTR(buf, b, blen);
//::                    else:
//::                        p4fldwidth_byte = (p4fldwidth / 8) + (1 if p4fldwidth % 8 else 0)
            b = snprintf(buf, blen, "%s: \n", "${p4fldname}");
            MOVE_BUFFER_PTR(buf, b, blen);
            for (int j = 0; j < ${p4fldwidth_byte}; j++) {
                b = snprintf(buf, blen, "0x%x : ", swkey.${p4fldname}[j]);
                MOVE_BUFFER_PTR(buf, b, blen);
            }
            b = snprintf(buf, blen, "\n");
            MOVE_BUFFER_PTR(buf, b, blen);
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
//::                            for kmbyte, kbit, _, _ in key_byte_format:
//::                                kbyte = kbit / 8
//::                                tablebyte = kmbyte
//::                                if p4fldwidth <= 32:
            b = snprintf(buf, blen, "%s: 0x%x\n", "${p4fldname}",
                        (swkey.${ustr}${p4fldname}));
//::                                else:
            b = snprintf(buf, blen, "%s: \n", "${p4fldname}");
            MOVE_BUFFER_PTR(buf, b, blen);
//::                                    p4fldwidth_byte = (p4fldwidth / 8) + (1 if p4fldwidth % 8 else 0)
            for (int j = 0; j < ${p4fldwidth_byte}; j++) {
                b = snprintf(buf, blen, "  0x%x : ", swkey.${ustr}${p4fldname}[j]);
                MOVE_BUFFER_PTR(buf, b, blen);
            }
            b = snprintf(buf, blen, "\n");
            MOVE_BUFFER_PTR(buf, b, blen);
//::                                #endif
//::                            #endfor
//::                        #endif
//::                    #endfor
//::                    i += 1
//::                #endfor
//::                for fields in pddict['tables'][table]['hdrunion_keys']:
//::                    ustr=''
//::                    (p4fldname, p4fldwidth, mask, key_byte_format, key_bit_format) = fields
//::                    if len(key_byte_format):
//::                        for kmbyte, kbit, width, containerstart in key_byte_format:
//::                            kbyte = kbit / 8
//::                            tablebyte = kmbyte
//::                            if p4fldwidth <= 32:
            b = snprintf(buf, blen, "%s: 0x%x\n", "${p4fldname}",
                         (swkey.${ustr}${p4fldname}));
//::                            else:
//::                                p4fldwidth_byte = (p4fldwidth / 8) + (1 if p4fldwidth % 8 else 0)
            b = snprintf(buf, blen, "%s: \n", "${p4fldname}");
            MOVE_BUFFER_PTR(buf, b, blen);
            for (int j = 0; j < ${p4fldwidth_byte}; j++) {
                b = snprintf(buf, blen, "  0x%x : ", swkey.${ustr}${p4fldname}[j]);
                MOVE_BUFFER_PTR(buf, b, blen);
            }
            b = snprintf(buf, blen, "\n");
            MOVE_BUFFER_PTR(buf, b, blen);
//::                            #endif
//::                        #endfor
//::                    #endif
//::                #endfor
            switch(actiondata.action_id) {
//::                for action in pddict['tables'][table]['actions']:
//::                    (actionname, actionflddict, _) = action
//::                    actname = actionname.upper()
                case ${caps_tablename}_${actname}_ID:
                {
                    b = snprintf(buf, blen, "Action: %s\n", "${caps_tablename}_${actname}_ID");
                    MOVE_BUFFER_PTR(buf, b, blen);
//::                    iter = 0
//::                    while iter < len(actionflddict):
//::                        actionfld      = actionflddict[iter]
//::                        actionfldname  = actionfld['p4_name']
//::                        actionfldwidth = actionfld['len']
//::                        if ((iter + 1) < len(actionflddict)):
//::                            nextactionfld     = actionflddict[iter + 1]
//::                            nextactionfldname = nextactionfld['p4_name']
//::                            if actionfldname == nextactionfldname:
//::                                iter = iter + 1;
//::                                actionfldwidth = nextactionfld['len']
//::                            #endif
//::                        #endif
//::                        iter = iter + 1
//::                        if actionfldwidth <= 32:
                    b = snprintf(buf, blen, "%s: 0x%x\n", "${actionfldname}",
                             actiondata.action_u.\
                             ${table}_${actionname}.${actionfldname});
                    MOVE_BUFFER_PTR(buf, b, blen);
//::                        else:
//::                            p4fldwidth_byte = (actionfldwidth / 8) + (1 if actionfldwidth % 8 else 0)
                    b = snprintf(buf, blen, "%s: \n", "${actionfldname}");
                    MOVE_BUFFER_PTR(buf, b, blen);
                    for (int j = 0; j < ${p4fldwidth_byte}; j++) {
                        b = snprintf(buf, blen, "  0x%x : ",
                                 actiondata.action_u.\
                                 ${table}_${actionname}.${actionfldname}[j]);
                        MOVE_BUFFER_PTR(buf, b, blen);
                    }
                    b = snprintf(buf, blen, "\n");
                    MOVE_BUFFER_PTR(buf, b, blen);
//::                        #endif
//::                    #endwhile
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
                MOVE_BUFFER_PTR(buf, b, blen);
                b = snprintf(buf, blen, "%s: 0x%x\n", "${p4fldname}_mask", swkey_mask.${p4fldname}_mask);
                MOVE_BUFFER_PTR(buf, b, blen);
//::                    else:
//::                        p4fldwidth_byte = (p4fldwidth / 8) + (1 if p4fldwidth % 8 else 0)
                b = snprintf(buf, blen, "%s: \n", "${p4fldname}");
                MOVE_BUFFER_PTR(buf, b, blen);
                for (int j = 0; j < ${p4fldwidth_byte}; j++) {
                    b = snprintf(buf, blen, "0x%x : ", swkey.${p4fldname}[j]);
                    MOVE_BUFFER_PTR(buf, b, blen);
                }
                b = snprintf(buf, blen, "\n");
                MOVE_BUFFER_PTR(buf, b, blen);
                b = snprintf(buf, blen, "%s: \n", "${p4fldname}_mask");
                MOVE_BUFFER_PTR(buf, b, blen);
                for (int j = 0; j < ${p4fldwidth_byte}; j++) {
                    b = snprintf(buf, blen, "0x%x : ", swkey_mask.${p4fldname}_mask[j]);
                    MOVE_BUFFER_PTR(buf, b, blen);
                }
                b = snprintf(buf, blen, "\n");
                MOVE_BUFFER_PTR(buf, b, blen);
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

//::                        (p4fldname, p4fldwidth, mask, key_byte_format, key_bit_format) = fields
//::                        if (p4fldwidth <= 32):
                b = snprintf(buf, blen, "%s: 0x%x\n", "${p4fldname}", swkey.${ustr}${p4fldname});
                MOVE_BUFFER_PTR(buf, b, blen);
                b = snprintf(buf, blen, "%s: 0x%x\n", "${p4fldname}_mask", swkey_mask.${ustr_mask}${p4fldname}_mask);
                MOVE_BUFFER_PTR(buf, b, blen);
//::                        else:
//::                            p4fldwidth_byte = (p4fldwidth / 8) + (1 if p4fldwidth % 8 else 0)
                b = snprintf(buf, blen, "%s: \n", "${p4fldname}");
                MOVE_BUFFER_PTR(buf, b, blen);
                for (int j = 0; j < ${p4fldwidth_byte}; j++) {
                    b = snprintf(buf, blen, "0x%x : ", swkey.${ustr}${p4fldname}[j]);
                    MOVE_BUFFER_PTR(buf, b, blen);
                }
                b = snprintf(buf, blen, "\n");
                MOVE_BUFFER_PTR(buf, b, blen);
                b = snprintf(buf, blen, "%s: \n", "${p4fldname}_mask");
                MOVE_BUFFER_PTR(buf, b, blen);
                for (int j = 0; j < ${p4fldwidth_byte}; j++) {
                    b = snprintf(buf, blen, "0x%x : ", swkey_mask.${ustr_mask}${p4fldname}_mask[j]);
                    MOVE_BUFFER_PTR(buf, b, blen);
                }
                b = snprintf(buf, blen, "\n");
                MOVE_BUFFER_PTR(buf, b, blen);
//::                        #endif
//::                    #endfor
//::                    i += 1
//::                #endfor
//::                i = 1
//::                for fields in pddict['tables'][table]['hdrunion_keys']:
//::                    ustr=''
//::                    ustr_mask=''
//::                    (p4fldname, p4fldwidth, mask, key_byte_format, key_bit_format) = fields
//::                    if (p4fldwidth <= 32):
                b = snprintf(buf, blen, "%s: 0x%x\n", "${p4fldname}", swkey.${ustr}${p4fldname});
                MOVE_BUFFER_PTR(buf, b, blen);
                b = snprintf(buf, blen, "%s: 0x%x\n", "${p4fldname}_mask", swkey_mask.${ustr_mask}${p4fldname}_mask);
                MOVE_BUFFER_PTR(buf, b, blen);
//::                    else:
//::                        p4fldwidth_byte = (p4fldwidth / 8) + (1 if p4fldwidth % 8 else 0)
                b = snprintf(buf, blen, "%s: \n", "${p4fldname}");
                MOVE_BUFFER_PTR(buf, b, blen);
                for (int j = 0; j < ${p4fldwidth_byte}; j++) {
                    b = snprintf(buf, blen, "0x%x : ", swkey.${ustr}${p4fldname}[j]);
                    MOVE_BUFFER_PTR(buf, b, blen);
                }
                b = snprintf(buf, blen, "\n");
                MOVE_BUFFER_PTR(buf, b, blen);
                b = snprintf(buf, blen, "%s: \n", "${p4fldname}_mask");
                MOVE_BUFFER_PTR(buf, b, blen);
                for (int j = 0; j < ${p4fldwidth_byte}; j++) {
                    b = snprintf(buf, blen, "0x%x : ", swkey_mask.${ustr_mask}${p4fldname}_mask[j]);
                    MOVE_BUFFER_PTR(buf, b, blen);
                }
                b = snprintf(buf, blen, "\n");
                MOVE_BUFFER_PTR(buf, b, blen);
//::                    #endif
//::                #endfor
            } else { /* Decode actiondata associated with TCAM key */
                b = snprintf(buf, blen, "!!!! Decode Tcam table Action Data !!!! \n");
                MOVE_BUFFER_PTR(buf, b, blen);
                ${table}_actiondata_t actiondata;
                actiondata.action_id = sdk::asic::pd::asicpd_get_action_id(tableid, _hwentry[0]);
                ${table}_unpack_action_data(tableid, actiondata.action_id, _hwentry+1,
                                            &actiondata);
                switch(actiondata.action_id) {
//::                for action in pddict['tables'][table]['actions']:
//::                    (actionname, actionflddict, _) = action
//::                    actname = actionname.upper()
                    case ${caps_tablename}_${actname}_ID:
                    {
                        b = snprintf(buf, blen, "Action: %s\n", "${caps_tablename}_${actname}_ID");
                        MOVE_BUFFER_PTR(buf, b, blen);
//::                    for actionfld in actionflddict:
//::                        actionfldname  = actionfld['p4_name']
//::                        actionfldwidth = actionfld['len']
//::                        if actionfldwidth <= 32:
                        b = snprintf(buf, blen, "%s: 0x%x\n", "${actionfldname}",
                             actiondata.action_u.\
                             ${table}_${actionname}.${actionfldname});
                        MOVE_BUFFER_PTR(buf, b, blen);
//::                        else:
//::                            p4fldwidth_byte = (actionfldwidth / 8) + (1 if actionfldwidth % 8 else 0)
                        b = snprintf(buf, blen, "%s: \n", "${actionfldname}");
                        MOVE_BUFFER_PTR(buf, b, blen);
                        for (int j = 0; j < ${p4fldwidth_byte}; j++) {
                            b = snprintf(buf, blen, "  0x%x : ",
                                     actiondata.action_u.\
                                     ${table}_${actionname}.${actionfldname}[j]);
                            MOVE_BUFFER_PTR(buf, b, blen);
                        }
                        b = snprintf(buf, blen, "\n");
                        MOVE_BUFFER_PTR(buf, b, blen);
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
                                          uint32_t   index,
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
    MOVE_BUFFER_PTR(buf, b, blen);
    switch (tableid) {
//::        for table, tid in tabledict.items():
//::            if pddict['tables'][table]['is_toeplitz_hash'] or pddict['tables'][table]['is_raw']:
//::                continue
//::            #endif
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
//::            if len(pddict['tables'][table]['hash_overflow_tbl']):
            if (tableid == P4${caps_p4prog}TBL_ID_${caps_tablename}) {
                b = snprintf(buf, blen, "Table : %s, Index : 0x%x/%u\n",
                             "P4TBL_ID_${caps_tablename}", index, index);
            } else {
                b = snprintf(buf, blen, "Table : %s, Index : 0x%x/%u\n",
                             "P4TBL_ID_${caps_tbl_}", index, index);
            }
//::            else:
            b = snprintf(buf, blen, "Table : %s, Index : 0x%x/%u\n",
                         "P4TBL_ID_${caps_tablename}", index, index);
//::            #endif
            MOVE_BUFFER_PTR(buf, b, blen);
//::            if pddict['tables'][table]['type'] == 'Index' or pddict['tables'][table]['type'] == 'Mpu':
            ${table}_actiondata_t *actiondata = (${table}_actiondata_t *)action_data;
            if (!actiondata) {
                break;
            }
            switch(actiondata->action_id) {
//::                for action in pddict['tables'][table]['actions']:
//::                    (actionname, actionflddict, _) = action
//::                    actname = actionname.upper()
                case ${caps_tablename}_${actname}_ID:
                {
                    b = snprintf(buf, blen, "Action : %s\n",
                                 "${caps_tablename}_${actname}_ID");
                    MOVE_BUFFER_PTR(buf, b, blen);
//::                    for actionfld in actionflddict:
//::                        actionfldname  = actionfld['p4_name']
//::                        actionfldwidth = actionfld['len']
//::                        if actionfldwidth <= 32:
                    b = snprintf(buf, blen, "%s : 0x%x\n", "${actionfldname}",
                             actiondata->action_u.\
                             ${table}_${actionname}.${actionfldname});
                    MOVE_BUFFER_PTR(buf, b, blen);
//::                        else:
//::                            p4fldwidth_byte = (actionfldwidth / 8) + (1 if actionfldwidth % 8 else 0)
                    b = snprintf(buf, blen, "%s : 0x", "${actionfldname}");
                    MOVE_BUFFER_PTR(buf, b, blen);
                    for (int j = 0; j < ${p4fldwidth_byte}; j++) {
                        b = snprintf(buf, blen, "%02x",
                                 actiondata->action_u.\
                                 ${table}_${actionname}.${actionfldname}[j]);
                        MOVE_BUFFER_PTR(buf, b, blen);
                    }
                    b = snprintf(buf, blen, "\n");
                    MOVE_BUFFER_PTR(buf, b, blen);
//::                        #endif
//::                    #endfor
                }
                break;
//::                #endfor
            }
//::            #endif
//::            if pddict['tables'][table]['type'] == 'Hash' or pddict['tables'][table]['type'] == 'Hash_OTcam':
            ${table}_swkey_t *swkey = (${table}_swkey_t *)sw_key;
            ${table}_actiondata_t *actiondata = (${table}_actiondata_t *)action_data;
            if (!swkey) {
                break;
            }
            b = snprintf(buf, blen, "Key :\n");
            MOVE_BUFFER_PTR(buf, b, blen);
//::                for fields in pddict['tables'][table]['keys']:
//::                    (p4fldname, p4fldwidth, mask, key_byte_format, key_bit_format) = fields
//::                    if (p4fldwidth <= 32):
            b = snprintf(buf, blen, "%s : 0x%x\n", "${p4fldname}", swkey->${p4fldname});
            MOVE_BUFFER_PTR(buf, b, blen);
//::                    else:
//::                        p4fldwidth_byte = (p4fldwidth / 8) + (1 if p4fldwidth % 8 else 0)
            b = snprintf(buf, blen, "%s : 0x", "${p4fldname}");
            MOVE_BUFFER_PTR(buf, b, blen);
            for (int j = 0; j < ${p4fldwidth_byte}; j++) {
                b = snprintf(buf, blen, "%02x", swkey->${p4fldname}[j]);
                MOVE_BUFFER_PTR(buf, b, blen);
            }
            b = snprintf(buf, blen, "\n");
            MOVE_BUFFER_PTR(buf, b, blen);
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
//::                            if p4fldwidth <= 32:
            b = snprintf(buf, blen, "%s : 0x%x\n", "${p4fldname}",
                        (swkey->${ustr}${p4fldname}));
            MOVE_BUFFER_PTR(buf, b, blen);
//::                            else:
            b = snprintf(buf, blen, "%s : 0x", "${p4fldname}");
            MOVE_BUFFER_PTR(buf, b, blen);
//::                                p4fldwidth_byte = (p4fldwidth / 8) + (1 if p4fldwidth % 8 else 0)
            for (int j = 0; j < ${p4fldwidth_byte}; j++) {
                b = snprintf(buf, blen, "%02x", swkey->${ustr}${p4fldname}[j]);
                MOVE_BUFFER_PTR(buf, b, blen);
            }
            b = snprintf(buf, blen, "\n");
            MOVE_BUFFER_PTR(buf, b, blen);
//::                            #endif
//::                        #endif
//::                    #endfor
//::                    i += 1
//::                #endfor
//::                for fields in pddict['tables'][table]['hdrunion_keys']:
//::                    ustr=''
//::                    (p4fldname, p4fldwidth, mask, key_byte_format, key_bit_format) = fields
//::                    if len(key_byte_format):
//::                        if p4fldwidth <= 32:
            b = snprintf(buf, blen, "%s : 0x%x\n", "${p4fldname}",
                         (swkey->${ustr}${p4fldname}));
            MOVE_BUFFER_PTR(buf, b, blen);
//::                        else:
            b = snprintf(buf, blen, "%s : ", "${p4fldname}");
            MOVE_BUFFER_PTR(buf, b, blen);
//::                            p4fldwidth_byte = (p4fldwidth / 8) + (1 if p4fldwidth % 8 else 0)
            for (int j = 0; j < ${p4fldwidth_byte}; j++) {
                b = snprintf(buf, blen, "%02x", swkey->${ustr}${p4fldname}[j]);
                buf += b;
                blen -= b;
                if (blen <= 0) {
                    return (P4PD_SUCCESS);
                }
            }
            b = snprintf(buf, blen, "\n");
            MOVE_BUFFER_PTR(buf, b, blen);
//::                        #endif
//::                    #endif
//::                #endfor
            if (!actiondata) {
                break;
            }
            switch(actiondata->action_id) {
//::                for action in pddict['tables'][table]['actions']:
//::                    (actionname, actionflddict, _) = action
//::                    actname = actionname.upper()
                case ${caps_tablename}_${actname}_ID:
                {
                    b = snprintf(buf, blen, "Action : %s\n", "${caps_tablename}_${actname}_ID");
                    buf += b;
                    blen -= b;
                    if (blen <= 0) {
                        return (P4PD_SUCCESS);
                    }
//::                    iter = 0
//::                    while iter < len(actionflddict):
//::                        actionfld      = actionflddict[iter]
//::                        actionfldname  = actionfld['p4_name']
//::                        actionfldwidth = actionfld['len']
//::                        if ((iter + 1) < len(actionflddict)):
//::                            nextactionfld     = actionflddict[iter + 1]
//::                            nextactionfldname = nextactionfld['p4_name']
//::                            if actionfldname == nextactionfldname:
//::                                iter = iter + 1;
//::                                actionfldwidth = nextactionfld['len']
//::                            #endif
//::                        #endif
//::                        iter = iter + 1
//::                        if actionfldwidth <= 32:
                    b = snprintf(buf, blen, "%s : 0x%x\n", "${actionfldname}",
                             actiondata->action_u.\
                             ${table}_${actionname}.${actionfldname});
                    MOVE_BUFFER_PTR(buf, b, blen);
//::                        else:
//::                            p4fldwidth_byte = (actionfldwidth / 8) + (1 if actionfldwidth % 8 else 0)
                    b = snprintf(buf, blen, "%s : 0x", "${actionfldname}");
                    MOVE_BUFFER_PTR(buf, b, blen);
                    for (int j = 0; j < ${p4fldwidth_byte}; j++) {
                        b = snprintf(buf, blen, "%02x",
                                 actiondata->action_u.\
                                 ${table}_${actionname}.${actionfldname}[j]);
                        MOVE_BUFFER_PTR(buf, b, blen);
                    }
                    b = snprintf(buf, blen, "\n");
                    MOVE_BUFFER_PTR(buf, b, blen);
//::                        #endif
//::                    #endwhile
                }
                break;
//::                #endfor
            }
//::            #endif
//::            if pddict['tables'][table]['type'] == 'Ternary':
            ${table}_swkey_t *swkey = (${table}_swkey_t *)sw_key;
            ${table}_swkey_mask_t *swkey_mask = (${table}_swkey_mask_t *)sw_key_mask;
            if (swkey && swkey_mask) {
                b = snprintf(buf, blen, "Key :\n");
                MOVE_BUFFER_PTR(buf, b, blen);
//::                for fields in pddict['tables'][table]['keys']:
//::                    (p4fldname, p4fldwidth, mask, key_byte_format, key_bit_format) = fields
//::                    if (p4fldwidth <= 32):
                b = snprintf(buf, blen, "%s : 0x%x\n", "${p4fldname}", swkey->${p4fldname});
                MOVE_BUFFER_PTR(buf, b, blen);
                b = snprintf(buf, blen, "%s : 0x%x\n", "${p4fldname}_mask", swkey_mask->${p4fldname}_mask);
                MOVE_BUFFER_PTR(buf, b, blen);
//::                    else:
//::                        p4fldwidth_byte = (p4fldwidth / 8) + (1 if p4fldwidth % 8 else 0)
                b = snprintf(buf, blen, "%s : 0x", "${p4fldname}");
                MOVE_BUFFER_PTR(buf, b, blen);
                for (int j = 0; j < ${p4fldwidth_byte}; j++) {
                    b = snprintf(buf, blen, "%02x", swkey->${p4fldname}[j]);
                    MOVE_BUFFER_PTR(buf, b, blen);
                }
                b = snprintf(buf, blen, "\n");
                MOVE_BUFFER_PTR(buf, b, blen);
                b = snprintf(buf, blen, "%s : 0x", "${p4fldname}_mask");
                MOVE_BUFFER_PTR(buf, b, blen);
                for (int j = 0; j < ${p4fldwidth_byte}; j++) {
                    b = snprintf(buf, blen, "%02x", swkey_mask->${p4fldname}_mask[j]);
                    buf += b;
                    blen -= b;
                    if (blen <= 0) {
                        return (P4PD_SUCCESS);
                    }
                }
                b = snprintf(buf, blen, "\n");
                MOVE_BUFFER_PTR(buf, b, blen);
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

//::                        (p4fldname, p4fldwidth, mask, key_byte_format, key_bit_format) = fields
//::                        if (p4fldwidth <= 32):
                b = snprintf(buf, blen, "%s : 0x%x\n", "${p4fldname}", swkey->${ustr}${p4fldname});
                MOVE_BUFFER_PTR(buf, b, blen);
                b = snprintf(buf, blen, "%s : 0x%x\n", "${p4fldname}_mask", swkey_mask->${ustr_mask}${p4fldname}_mask);
                MOVE_BUFFER_PTR(buf, b, blen);
//::                        else:
//::                            p4fldwidth_byte = (p4fldwidth / 8) + (1 if p4fldwidth % 8 else 0)
                b = snprintf(buf, blen, "%s : 0x", "${p4fldname}");
                MOVE_BUFFER_PTR(buf, b, blen);
                for (int j = 0; j < ${p4fldwidth_byte}; j++) {
                    b = snprintf(buf, blen, "%02x", swkey->${ustr}${p4fldname}[j]);
                    MOVE_BUFFER_PTR(buf, b, blen);
                }
                b = snprintf(buf, blen, "\n");
                MOVE_BUFFER_PTR(buf, b, blen);
                b = snprintf(buf, blen, "%s : ", "${p4fldname}_mask");
                MOVE_BUFFER_PTR(buf, b, blen);
                for (int j = 0; j < ${p4fldwidth_byte}; j++) {
                    b = snprintf(buf, blen, "%02x", swkey_mask->${ustr_mask}${p4fldname}_mask[j]);
                    MOVE_BUFFER_PTR(buf, b, blen);
                }
                b = snprintf(buf, blen, "\n");
                MOVE_BUFFER_PTR(buf, b, blen);
//::                        #endif
//::                    #endfor
//::                    i += 1
//::                #endfor
//::                for fields in pddict['tables'][table]['hdrunion_keys']:
//::                    ustr=''
//::                    ustr_mask=''
//::                    (p4fldname, p4fldwidth, mask, key_byte_format, key_bit_format) = fields
//::                    if (p4fldwidth <= 32):
                b = snprintf(buf, blen, "%s : 0x%x\n", "${p4fldname}", swkey->${ustr}${p4fldname});
                MOVE_BUFFER_PTR(buf, b, blen);
                b = snprintf(buf, blen, "%s : 0x%x\n", "${p4fldname}_mask", swkey_mask->${ustr_mask}${p4fldname}_mask);
                MOVE_BUFFER_PTR(buf, b, blen);
//::                    else:
//::                        p4fldwidth_byte = (p4fldwidth / 8) + (1 if p4fldwidth % 8 else 0)
                b = snprintf(buf, blen, "%s : 0x", "${p4fldname}");
                MOVE_BUFFER_PTR(buf, b, blen);
                for (int j = 0; j < ${p4fldwidth_byte}; j++) {
                    b = snprintf(buf, blen, "%02x", swkey->${ustr}${p4fldname}[j]);
                    MOVE_BUFFER_PTR(buf, b, blen);
                }
                b = snprintf(buf, blen, "\n");
                MOVE_BUFFER_PTR(buf, b, blen);
                b = snprintf(buf, blen, "%s : 0x", "${p4fldname}_mask");
                MOVE_BUFFER_PTR(buf, b, blen);
                for (int j = 0; j < ${p4fldwidth_byte}; j++) {
                    b = snprintf(buf, blen, "%02x", swkey_mask->${ustr_mask}${p4fldname}_mask[j]);
                    MOVE_BUFFER_PTR(buf, b, blen);
                }
                b = snprintf(buf, blen, "\n");
                MOVE_BUFFER_PTR(buf, b, blen);
//::                    #endif
//::                #endfor
            }
            ${table}_actiondata_t *actiondata = (${table}_actiondata_t *)action_data;
            if (!actiondata) {
                break;
            }
            switch(actiondata->action_id) {
//::                for action in pddict['tables'][table]['actions']:
//::                    (actionname, actionflddict, _) = action
//::                    actname = actionname.upper()
                case ${caps_tablename}_${actname}_ID:
                {
                    b = snprintf(buf, blen, "Action : %s\n", "${caps_tablename}_${actname}_ID");
                    MOVE_BUFFER_PTR(buf, b, blen);
//::                    for actionfld in actionflddict:
//::                        actionfldname  = actionfld['p4_name']
//::                        actionfldwidth = actionfld['len']
//::                        if actionfldwidth <= 32:
                    b = snprintf(buf, blen, "%s : 0x%x\n", "${actionfldname}",
                             actiondata->action_u.\
                             ${table}_${actionname}.${actionfldname});
                    MOVE_BUFFER_PTR(buf, b, blen);
//::                        else:
//::                            p4fldwidth_byte = (actionfldwidth / 8) + (1 if actionfldwidth % 8 else 0)
                    b = snprintf(buf, blen, "%s : 0x", "${actionfldname}");
                    MOVE_BUFFER_PTR(buf, b, blen);
                    for (int j = 0; j < ${p4fldwidth_byte}; j++) {
                        b = snprintf(buf, blen, "%02x",
                                 actiondata->action_u.\
                                 ${table}_${actionname}.${actionfldname}[j]);
                        MOVE_BUFFER_PTR(buf, b, blen);
                    }
                    b = snprintf(buf, blen, "\n");
                    MOVE_BUFFER_PTR(buf, b, blen);
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

uint32_t
${api_prefix}_actiondata_appdata_size_get(uint32_t   tableid,
                            uint8_t    actionid)
{
//::        if genappdatafield_api:
    switch (tableid) {
//::            for table, tid in tabledict.items():
//::                if pddict['tables'][table]['appdatafields']:
//::                    caps_tablename = table.upper()
        case P4${caps_p4prog}TBL_ID_${caps_tablename}: /* p4-table '${table}' */
            switch (actionid) {
//::                    for action in pddict['tables'][table]['actions']:
//::                        actionname, _, _ = action
//::                        caps_actname = actionname.upper()
//::                        fieldlist = pddict['tables'][table]['appdatafields'][actionname]
//::                        if len(fieldlist):
                case ${caps_tablename}_${caps_actname}_ID:
                return sizeof(${table}_appdata_t);
                break;
//::                        #endif
//::                    #endfor
                default:
                    // Invalid action
                    assert(0);
                    return (P4PD_FAIL);
                break;
            }
        break;
//::                #else:
//::                    continue
//::                #endif
//::            #endfor
        default:
            // Invalid tableid
            assert(0);
            return (P4PD_FAIL);
        break;
    }
//::        #endif
    assert(0);
    return 0;
}

p4pd_error_t
${api_prefix}_actiondata_appdata_set(uint32_t   tableid,
                            uint8_t    actionid,
                            void       *appdata,
                            void       *actiondata)
{
//::        if genappdatafield_api:
    switch (tableid) {
//::            for table, tid in tabledict.items():
//::                if pddict['tables'][table]['appdatafields']:
//::                    caps_tablename = table.upper()
        case P4${caps_p4prog}TBL_ID_${caps_tablename}: /* p4-table '${table}' */
            switch (actionid) {
//::                    for action in pddict['tables'][table]['actions']:
//::                        actionname, _, _ = action
//::                        caps_actname = actionname.upper()
//::                        fieldlist = pddict['tables'][table]['appdatafields'][actionname]
//::                        if len(fieldlist):
                case ${caps_tablename}_${caps_actname}_ID:
//::                            for fieldname, fieldwidth in fieldlist:
                memcpy(&((${table}_actiondata_t*)actiondata)->action_u.${table}_${actionname}.${fieldname},
                       &((${table}_appdata_t*)appdata)->${fieldname},
                       sizeof(((${table}_appdata_t*)appdata)->${fieldname}));
//::                            #endfor
                break;
//::                        #endif
//::                    #endfor
                default:
                    // Invalid action
                    return (P4PD_FAIL);
                break;
            }
        break;
//::                #else:
//::                    continue
//::                #endif
//::            #endfor
        default:
            // Invalid tableid
            return (P4PD_FAIL);
        break;
    }
    return (P4PD_SUCCESS);
//::        else:
    assert(0);
    return (P4PD_FAIL);
//::        #endif
}

p4pd_error_t
${api_prefix}_actiondata_appdata_get(uint32_t   tableid,
                            uint8_t    actionid,
                            void       *appdata,
                            void       *actiondata)
{
//::        if genappdatafield_api:
    switch (tableid) {
//::            for table, tid in tabledict.items():
//::                if pddict['tables'][table]['appdatafields']:
//::                    caps_tablename = table.upper()
        case P4${caps_p4prog}TBL_ID_${caps_tablename}: /* p4-table '${table}' */
            switch (actionid) {
//::                    for action in pddict['tables'][table]['actions']:
//::                        actionname, _, _ = action
//::                        caps_actname = actionname.upper()
//::                        fieldlist = pddict['tables'][table]['appdatafields'][actionname]
//::                        if len(fieldlist):
                case ${caps_tablename}_${caps_actname}_ID:
//::                            for fieldname, fieldwidth in fieldlist:
                memcpy(&((${table}_appdata_t*)appdata)->${fieldname},
                       &((${table}_actiondata_t*)actiondata)->action_u.${table}_${actionname}.${fieldname},
                       sizeof(((${table}_appdata_t*)appdata)->${fieldname}));
//::                            #endfor
                break;
//::                        #endif
//::                    #endfor
                default:
                    // Invalid action
                    return (P4PD_FAIL);
                break;
            }
        break;
//::                #else:
//::                    continue
//::                #endif
//::            #endfor
        default:
            // Invalid tableid
            return (P4PD_FAIL);
        break;
    }
    return (P4PD_SUCCESS);
//::        else:
    assert(0);
    return (P4PD_FAIL);
//::        #endif
}

uint32_t
${api_prefix}_actiondata_hwfields_count_get(uint32_t tableid, uint8_t actionid)
{
//::        if genhwfields_actiondata_api:
    switch (tableid) {
//::            for table, tid in tabledict.items():
//::                if pddict['tables'][table]['hwfields']:
//::                    caps_tablename = table.upper()
        case P4${caps_p4prog}TBL_ID_${caps_tablename}: /* p4-table '${table}' */
            switch (actionid) {
//::                    for action in pddict['tables'][table]['actions']:
//::                        actionname, _, _ = action
//::                        caps_actname = actionname.upper()
//::                        fieldlist = pddict['tables'][table]['appdatafields'][actionname]
//::                        if len(fieldlist):
                case ${caps_tablename}_${caps_actname}_ID:
//::                    argc = len(pddict['tables'][table]['hwfields'][actionname])
                    return (${argc});
                break;
//::                        #endif
//::                    #endfor
                default:
                    // Invalid action
                    return (0);
                break;
            }
        break;
//::                #else:
//::                    continue
//::                #endif
//::            #endfor
        default:
            return (0);
        break;
    }
    return (0);
//::        else:
    assert(0);
    return (0);
//::        #endif
}

p4pd_error_t
${api_prefix}_actiondata_hwfield_set(uint32_t   tableid,
                            uint8_t    actionid,
                            uint32_t   argument_slotid,
                            uint8_t    *argumentvalue,
                            void       *actiondata)
{
//::        if genhwfields_actiondata_api:
    switch (tableid) {
//::            for table, tid in tabledict.items():
//::                if pddict['tables'][table]['hwfields']:
//::                    caps_tablename = table.upper()
        case P4${caps_p4prog}TBL_ID_${caps_tablename}: /* p4-table '${table}' */
            ((${table}_actiondata_t*)actiondata)->action_id = actionid;
            switch (actionid) {
//::                    for action in pddict['tables'][table]['actions']:
//::                        actionname, _, _ = action
//::                        caps_actname = actionname.upper()
//::                        hwfieldlist = pddict['tables'][table]['hwfields'][actionname]
//::                        if len(hwfieldlist):
                case ${caps_tablename}_${caps_actname}_ID:
                    {
//::                            argc = len(pddict['tables'][table]['hwfields'][actionname])
                        if (argument_slotid >= ${argc}) {
                            return (P4PD_FAIL);
                        }
                        static uint32_t argument_offsets[] = {
//::                            for fieldname, _   in hwfieldlist:
                            offsetof(${table}_${actionname}_t, ${fieldname}),
//::                            #endfor
                        };
                        static uint32_t argument_byte_width[] = {
//::                            for fieldname, fieldwidth  in hwfieldlist:
//::                                if (fieldwidth <= 8):
                            sizeof(uint8_t),
//::                                elif (fieldwidth <= 16):
                            sizeof(uint16_t),
//::                                elif (fieldwidth <= 32):
                            sizeof(uint32_t),
//::                                else:
//::                                    byte_fldwidth = (fieldwidth + 7) >> 3
                            ${byte_fldwidth},
//::                                #endif
//::                            #endfor
                        };
                        uint8_t* structbase = (uint8_t*)&(((${table}_actiondata_t*)actiondata)->action_u);
                        memcpy((structbase + argument_offsets[argument_slotid]),
                                argumentvalue, argument_byte_width[argument_slotid]);
                    }
                break;
//::                        #endif
//::                    #endfor
                default:
                    // Invalid action
                    return (P4PD_FAIL);
                break;
            }
            break;
//::                #else:
//::                    continue
//::                #endif
//::            #endfor
        default:
            // Invalid tableid
            return (P4PD_FAIL);
        break;
    }
    return (P4PD_SUCCESS);
//::        else:
    assert(0);
    return (P4PD_FAIL);
//::        #endif
}

p4pd_error_t
${api_prefix}_actiondata_hwfield_get(uint32_t   tableid,
                            uint8_t    actionid,
                            uint32_t   argument_slotid,
                            uint8_t    *argumentvalue,
                            void       *actiondata)
{
//::        if genhwfields_actiondata_api:
    switch (tableid) {
//::            for table, tid in tabledict.items():
//::                if pddict['tables'][table]['hwfields']:
//::                    caps_tablename = table.upper()
        case P4${caps_p4prog}TBL_ID_${caps_tablename}: /* p4-table '${table}' */
            switch (actionid) {
//::                    for action in pddict['tables'][table]['actions']:
//::                        actionname, _, _ = action
//::                        caps_actname = actionname.upper()
//::                        hwfieldlist = pddict['tables'][table]['hwfields'][actionname]
//::                        if len(hwfieldlist):
                case ${caps_tablename}_${caps_actname}_ID:
                    {
//::                            argc = len(pddict['tables'][table]['hwfields'][actionname])
                        if (argument_slotid >= ${argc}) {
                            return (P4PD_FAIL);
                        }
                        static uint32_t argument_offsets[] = {
//::                            for fieldname, _  in hwfieldlist:
                            offsetof(${table}_${actionname}_t, ${fieldname}),
//::                            #endfor
                        };
                        static uint32_t argument_byte_width[] = {
//::                            for fieldname, fieldwidth  in hwfieldlist:
//::                                if (fieldwidth <= 8):
                            sizeof(uint8_t),
//::                                elif (fieldwidth <= 16):
                            sizeof(uint16_t),
//::                                elif (fieldwidth <= 32):
                            sizeof(uint32_t),
//::                                else:
//::                                    byte_fldwidth = (fieldwidth + 7) >> 3
                            ${byte_fldwidth},
//::                                #endif
//::                            #endfor
                        };
                        uint8_t* structbase = (uint8_t*)&(((${table}_actiondata_t*)actiondata)->action_u);
                        memcpy(argumentvalue, (structbase + argument_offsets[argument_slotid]),
                               argument_byte_width[argument_slotid]);
                    }
                break;
//::                        #endif
//::                    #endfor
                default:
                    // Invalid action
                    return (P4PD_FAIL);
                break;
            }
            break;
//::                #else:
//::                    continue
//::                #endif
//::            #endfor
        default:
            // Invalid tableid
            return (P4PD_FAIL);
        break;
    }
    return (P4PD_SUCCESS);
//::        else:
    assert(0);
    return (P4PD_FAIL);
//::        #endif
}

//::    #endif

//::    if len(tabledict):

void ${api_prefix}_prep_p4tbl_names()
{
//::        for  tblname in sorted(tabledict, key=tabledict.get):
//::            caps_tblname = tblname.upper()
    strncpy(${prefix}_tbl_names[P4${caps_p4prog}TBL_ID_${caps_tblname}], "${tblname}", strlen("${tblname}"));
//::        #endfor
}

void ${api_prefix}_prep_p4tbl_sw_struct_sizes()
{
//::        for  tblname in sorted(tabledict, key=tabledict.get):
//::            caps_tblname = tblname.upper()
//::        if pddict['tables'][tblname]['type'] != 'Index':
    ${prefix}_tbl_swkey_size[P4${caps_p4prog}TBL_ID_${caps_tblname}] = sizeof(${tblname}_swkey);
//::        #endif
    ${prefix}_tbl_sw_action_data_size[P4${caps_p4prog}TBL_ID_${caps_tblname}]= sizeof(${tblname}_actiondata_t);
//::        #endfor
}

//::    #endif

int ${api_prefix}_get_max_action_id(uint32_t tableid)
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

void ${api_prefix}_get_action_name(uint32_t tableid, int actionid, char *action_name)
{
    switch(tableid) {
//::        for  tblname in sorted(tabledict, key=tabledict.get):
//::            caps_tblname = tblname.upper()
        case P4${caps_p4prog}TBL_ID_${caps_tblname}:
//::            if len(pddict['tables'][tblname]['actions']):
            switch(actionid) {
//::                for action in pddict['tables'][tblname]['actions']:
//::                    (actionname, actionfldlist, _) = action
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

void ${api_prefix}_get_error_action_name(uint32_t tableid, char *action_name)
{
    switch(tableid) {
//::        for table, tid in tabledict.items():
//::            caps_tblname = table.upper()
//::            if 'error_action' in pddict['tables'][table].keys():
        case P4${caps_p4prog}TBL_ID_${caps_tblname}:
//::                err_actionname = pddict['tables'][table]['error_action']
//::                if len(err_actionname) > ACTION_NAME_MAX_LEN:
            strncpy(action_name, "${err_actionname}", P4${caps_p4prog}ACTION_NAME_MAX_LEN);
//::                else:
            strncpy(action_name, "${err_actionname}", strlen("${err_actionname}"));
//::                #endif
            return;
//::            #endif
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
