//:: import os, pdb
//:: from collections import OrderedDict
//:: pddict = _context['pddict']
//:: if pddict['p4plus']:
//::    p4prog = pddict['p4program'] + '_'
//::    hdrdir = pddict['p4program']
//::    pipeline = pddict['pipeline']
//::    caps_p4prog = '_' + pddict['p4program'].upper() + '_'
//::    prefix = 'p4pd_' + pddict['p4program']
//::    if pddict['p4plus_module'] == 'rxdma':
//::        start_table_base = 101
//::    elif pddict['p4plus_module'] == 'txdma':
//::	    start_table_base = 201
//::    else:
//::	    start_table_base = 301
//::    #endif
//::    filename = p4prog + 'p4pd.cc'
//:: else:
//::    p4prog = ''
//::    hdrdir = pddict['p4program']
//::    pipeline = pddict['pipeline']
//::    caps_p4prog = ''
//::    prefix = 'p4pd'
//::	start_table_base = 1
//::    filename = 'p4pd.cc'
//:: #endif
//:: #define here any constants needed.
//:: ACTION_PC_LEN = 8 # in bits
/*
 * ${filename}
 * Pensando Systems
 */

/* This file contains API implementation needed to operate on each P4+ raw tables.
 *
 * For every P4+ table,
 *
 *   1. In order to keep application layer agnostic of hardware representation
 *      folllowing APIs are provided.
 *      1.a  <table_name>_hwkey_query() which returns hwkey length,
 *           hwactiondata length associated with the key. These lengths
 *           can be used to allocate memory before using p4 table operation
 *           APIs. All table operation APIs expect caller of the APIs to
 *           provide necessary memory for table operation API arguments.
 *      1.b  <table_name>_pack() which converts raw table action data structure into byte
 *           stream that can be written to hardware table entry.
 *      1.c  <table_name>_unpack() which converts raw table action data byte stream
 *           back to action data structure that application/drivers can use.
 *
 */

/*
 * This file is generated from P4 program. Any changes made to this file will
 * be lost.
 */

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include "gen/p4gen/${hdrdir}/include/${p4prog}p4pd_internal.h"
#include "gen/p4gen/${hdrdir}/include/${p4prog}p4pd.h"
#include "nic/sdk/lib/p4/p4_utils.hpp"

char ${prefix}_tbl_names[__P4${caps_p4prog}TBL_ID_TBLMAX][P4${caps_p4prog}TBL_NAME_MAX_LEN];
uint16_t ${prefix}_tbl_swkey_size[__P4${caps_p4prog}TBL_ID_TBLMAX];
uint16_t ${prefix}_tbl_sw_action_data_size[__P4${caps_p4prog}TBL_ID_TBLMAX];

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
//::        if pddict['tables'][table]['is_toeplitz_hash']:
//::            continue
//::        #endif
//::        if not pddict['tables'][table]['is_raw'] and not pddict['tables'][table]['is_raw_index']:
//::            continue
//::        #endif
static void
${table}_hwentry_query(uint32_t tableid, uint8_t action_id, uint32_t* hwactiondata_len)
{
    *hwactiondata_len = 0;
    switch(action_id) {
//::            for action in pddict['tables'][table]['actions']:
//::                (actionname, actionflddict, _) = action
//::                actname = actionname.upper()
//::                actionfldlen = 0
//::                if len(actionflddict):
//::                    for actionfld in actionflddict:
//::                        actionfldname  = actionfld['p4_name']
//::                        actionfldwidth = actionfld['len']
//::                        actionfldlen += actionfldwidth
//::                    #endfor
//::                #endif
        case ${tbl}_${actname}_ID:
            *hwactiondata_len = ${actionfldlen};
        break;
//::            #endfor
    }
    return;
}

static void
${table}_pack_action_data(uint32_t tableid, uint8_t action_id, ${table}_actiondata_t *actiondata, uint8_t *packed_actiondata)
{
    switch(action_id) {
//::            for action in pddict['tables'][table]['actions']:
//::                (actionname, actionflddict, _) = action
//::                actname = actionname.upper()
//::                if not len(actionflddict):
//::                    continue
//::                #endif
//::                total_bits = sum([ad['len'] for ad in actionflddict])
        case ${tbl}_${actname}_ID:
            {
                ${table}_actiondata_bitfield_t *__actiondata = (${table}_actiondata_bitfield_t *)packed_actiondata;
//::                for actionfld in actionflddict:
//::                    actionfldname  = actionfld['p4_name']
//::                    actionfldwidth = actionfld['len']
//::                    if actionfldwidth <= 8:
                __actiondata->action_u.${table}_${actionname}.${actionfldname} =
                             (uint8_t)(actiondata->action_u.${table}_${actionname}.${actionfldname});
//::                    elif actionfldwidth <= 16:
                __actiondata->action_u.${table}_${actionname}.${actionfldname} =
                             (uint16_t)(actiondata->action_u.${table}_${actionname}.${actionfldname});
//::                    elif actionfldwidth <= 32:
                __actiondata->action_u.${table}_${actionname}.${actionfldname} =
                             (uint32_t)(actiondata->action_u.${table}_${actionname}.${actionfldname});
//::                    elif actionfldwidth <= 64:
                __actiondata->action_u.${table}_${actionname}.${actionfldname} =
                             *(uint64_t*)(&(actiondata->action_u.${table}_${actionname}.${actionfldname}));
//::                    else:
//::                        actionfldwidth_byte = (actionfldwidth / 8) + (1 if actionfldwidth % 8 else 0)
                memcpy(__actiondata->action_u.${table}_${actionname}.${actionfldname},
                       actiondata->action_u.${table}_${actionname}.${actionfldname},
                       ${actionfldwidth_byte});
//::                    #endif
//::                #endfor
                unsigned int s = (${total_bits} + 7) >> 3; // Packed action data bytes length
                for (unsigned int i = 0; i < (s >> 1); i++) {
                    uint8_t b = packed_actiondata[i];
                    packed_actiondata[i] = packed_actiondata[s - i - 1];
                    packed_actiondata[s - i - 1] = b;
                }
            }
        break;
//::            #endfor
    }
}

static void
${table}_unpack_action_data(uint32_t tableid, uint8_t actionid, uint8_t *packed_actiondata, ${table}_actiondata_t *actiondata)
{
    memset(actiondata, 0, sizeof(${table}_actiondata_t));
    switch(actionid) {
//::            for action in pddict['tables'][table]['actions']:
//::                (actionname, actionflddict, _) = action
//::                actname = actionname.upper()
//::                if not len(actionflddict):
//::                    continue
//::                #endif
//::                total_bits = sum([ad['len'] for ad in actionflddict])
        case ${tbl}_${actname}_ID:
            {
                unsigned int s = (${total_bits} + 7) >> 3; // Packed action data bytes length
                for (unsigned int i = 0; i < (s >> 1); i++) {
                    uint8_t b = packed_actiondata[i];
                    packed_actiondata[i] = packed_actiondata[s - i - 1];
                    packed_actiondata[s - i - 1] = b;
                }
                ${table}_actiondata_bitfield_t *__actiondata = (${table}_actiondata_bitfield_t *)packed_actiondata;
//::                for actionfld in actionflddict:
//::                    actionfldname  = actionfld['p4_name']
//::                    actionfldwidth = actionfld['len']
//::                    if actionfldwidth <= 8:
                actiondata->action_u.${table}_${actionname}.${actionfldname} =
                                 (uint8_t)(__actiondata->action_u.${table}_${actionname}.${actionfldname});
//::                    elif actionfldwidth <= 16:
                actiondata->action_u.${table}_${actionname}.${actionfldname} =
                                 (uint16_t)(__actiondata->action_u.${table}_${actionname}.${actionfldname});
//::                    elif actionfldwidth <= 32:
                actiondata->action_u.${table}_${actionname}.${actionfldname} =
                                 (uint32_t)(__actiondata->action_u.${table}_${actionname}.${actionfldname});
//::                    elif actionfldwidth <= 64:
                *(uint64_t*)(&(actiondata->action_u.${table}_${actionname}.${actionfldname})) =
                            (uint64_t)(__actiondata->action_u.${table}_${actionname}.${actionfldname});
//::                    else:
//::                        actionfldwidth_byte = (actionfldwidth / 8) + (1 if actionfldwidth % 8 else 0)
                memcpy(actiondata->action_u.${table}_${actionname}.${actionfldname},
                       __actiondata->action_u.${table}_${actionname}.${actionfldname},
                       ${actionfldwidth_byte});
//::                    #endif
//::                #endfor
            }
        break;
//::            #endfor
    }
}

static p4pd_error_t
${table}_entry_decode(uint32_t tableid, uint8_t actionid, uint8_t *hwentry, ${table}_actiondata_t* actiondata)
{
    ${table}_unpack_action_data(tableid, actionid, hwentry, actiondata);
    return (P4PD_SUCCESS);
}

static p4pd_error_t
${table}_entry_pack(uint32_t tableid, uint8_t action_id, ${table}_actiondata_t *actiondata, uint8_t *packed_entry)
{
    ${table}_pack_action_data(tableid, action_id, actiondata, packed_entry);
    return (P4PD_SUCCESS);
}

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
 *  OUT : uint32_t *hwactiondata_len   : Action data length.
 *
 * Return Value:
 *  None
 */
void
${api_prefix}_raw_table_hwentry_query(uint32_t tableid, uint8_t action_id, uint32_t *hwactiondata_len)
{
    switch (tableid) {
//::        for table, tid in tabledict.items():
//::            if not pddict['tables'][table]['is_raw'] and not pddict['tables'][table]['is_raw_index']:
//::                continue
//::            #endif
//::            caps_tablename = table.upper()
        case P4${caps_p4prog}TBL_ID_${caps_tablename}: /* p4-table '${table}' */
            ${table}_hwentry_query(tableid, action_id, hwactiondata_len);
        break;

//::        #endfor
    }
}

/*
 * Builds byte stream of P4-table entry that can be installed into hardware table.
 * Memory for byte stream used by this function needs to be provided by caller.
 *
 * Arguments:
 *
 *  IN  : uint32_t tableid       : Table Id that identifies
 *                                 P4 table. This id is obtained
 *                                 from p4pd_table_id_enum.
 *  IN  : uint8_t action_id      : Action ID associated with raw table.
 *  IN  : void    *actiondata    : Action data associated with the key.
 *                                 Action data structure generated per p4 table.
 *                                 Refer to p4pd.h for structure details
 *                                 Per p4 table action data structure should
 *                                 provided as void* actiondata.
 * OUT: uint8_t *packed_entry   : Byte stream where table action is packed and copied into.
 *                                Memory size to be used by caller can be obtained using
 *                                API {program_name}_raw_table_hwentry_query()
 *
 * Return Value:
 *  p4pd_error_t                : P4PD_SUCCESS when no error.
 */
p4pd_error_t
${api_prefix}_entry_pack(uint32_t tableid, uint8_t action_id, void *actiondata, uint8_t *packed_entry)
{
    switch (tableid) {
//::        for table, tid in tabledict.items():
//::            if not pddict['tables'][table]['is_raw'] and not pddict['tables'][table]['is_raw_index']:
//::                continue
//::            #endif
//::            caps_tablename = table.upper()
        case P4${caps_p4prog}TBL_ID_${caps_tablename}: /* p4-table '${table}' */
            return (${table}_entry_pack(tableid, action_id,
                                        (${table}_actiondata_t*)actiondata, packed_entry));
        break;

//::        #endfor
        default:
            // Invalid tableid
            return (-1);
        break;
    }
    return (-1);
}

/*
 * Returns action data structure of P4-table entry. This API will not work if table has more than
 * one use action.
 *
 * Arguments:
 *
 *  IN  : uint32_t tableid       : Table Id that identifies
 *                                 P4 table. This id is obtained
 *                                 from p4pd_table_id_enum.
 *  IN  : uint8_t action_pc      : Action PC to include in table entry.
 *                                 If table has only one action, then this
 *                                 value is not used. Caller can specify and value.
 *  IN  : uint32_t entry_width   : Table entry width in bits.
 *  OUT : void    *actiondata    : Action data associated with the key.
 *                                 Action data structure generated per p4 table.
 *                                 Per p4 table action data structure returned
 *                                 as void* actiondata should be casted to correct
 *                                 structure to use it.
 * Return Value:
 *  p4pd_error_t                 : P4PD_SUCCESS when no error.
 */
int
${api_prefix}_entry_unpack(uint32_t tableid, uint8_t actionid, uint8_t *hwentry, uint32_t entry_width, void *actiondata)
{
    switch (tableid) {
//::        for table, tid in tabledict.items():
//::            if not pddict['tables'][table]['is_raw'] and not pddict['tables'][table]['is_raw_index']:
//::                continue
//::            #endif
//::            caps_tablename = table.upper()
        case P4${caps_p4prog}TBL_ID_${caps_tablename}: /* p4-table '${table}' */
            return (${table}_entry_decode(tableid, actionid, hwentry,
                                        (${table}_actiondata_t*)actiondata));
        break;

//::        #endfor
        default:
            // Invalid tableid
            return (P4PD_FAIL);
        break;
    }
    return (P4PD_SUCCESS);
}

/*
 * Returns TRUE if P4+ table is raw-table lookedup or updated by address. For any other type of
 * table like SRAM, raw table looked up by index will return FALSE.
 *
 * Arguments:
 *
 *  IN  : uint32_t tableid       : Table Id that identifies
 *                                 P4 table. This id is obtained
 *                                 from p4pd_table_id_enum.
 * Return Value:
 *  bool                         : True/False
 */
int
${api_prefix}_is_raw_table(uint32_t tableid)
{
    switch (tableid) {
//::        for table, tid in tabledict.items():
//::            caps_tablename = table.upper()
        case P4${caps_p4prog}TBL_ID_${caps_tablename}: /* p4-table '${table}' */
//::            if pddict['tables'][table]['is_raw'] or pddict['tables'][table]['is_raw_index']:
            return true;
//::            else:
            return false;
//::            #endif
        break;

//::        #endfor
        default:
            // Invalid tableid
            return (false);
        break;
    }
    return (false);
}
//::    #endif
