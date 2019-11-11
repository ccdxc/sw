//:: import os, pdb
//:: from collections import OrderedDict
//:: pddict = _context['pddict']
//:: #pdb.set_trace()
//:: if pddict['p4plus']:
//::    p4prog = pddict['p4program'] + '_'
//::    caps_p4prog = '_' + pddict['p4program'].upper() + '_'
//::    prefix = 'p4pd_' + pddict['p4program']
//::    if pddict['p4plus_module'] == 'rxdma':
//::        start_table_base = 101
//::    elif pddict['p4plus_module'] == 'txdma':
//::        start_table_base = 201
//::    else:
//::        start_table_base = 301
//::    #endif
//:: else:
//::    caps_p4prog = ''
//::    p4prog = ''
//::    prefix = 'p4pd'
//::	start_table_base = 1
//:: #endif
/*
 * ${p4prog}p4pd_cli_swig.h
 * Pensando Systems
 */

/*
 * This file is generated from P4 program. Any changes made to this file will
 * be lost.
 */
#ifndef __P4PD${caps_p4prog}SWIG_CLI_H__
#define __P4PD${caps_p4prog}SWIG_CLI_H__

#include <string.h>
#include "nic/include/hal_pd_error.hpp"

//::     tabledict = {}
//::     tableid = start_table_base
//::     hash_min = tableid
//::     for table in pddict['tables']:
//::        if pddict['tables'][table]['type'] == 'Hash':
//::            tabledict[table] = tableid
//::            tableid += 1
//::        #endif
//::     #endfor
//::     hash_max = tableid - 1
//::     hash_otcam_min = tableid
//::     for table in pddict['tables']:
//::        if pddict['tables'][table]['type'] == 'Hash_OTcam':
//::            tabledict[table] = tableid
//::            tableid += 1
//::        #endif
//::     #endfor
//::     hash_otcam_max = tableid - 1
//::     tcam_min = tableid
//::     for table in pddict['tables']:
//::        if pddict['tables'][table]['type'] == 'Ternary':
//::            tabledict[table] = tableid
//::            tableid += 1
//::        #endif
//::     #endfor
//::     tcam_max = tableid - 1
//::     index_min = tableid
//::     for table in pddict['tables']:
//::        if pddict['tables'][table]['type'] == 'Index':
//::            tabledict[table] = tableid
//::            tableid += 1
//::        #endif
//::     #endfor
//::     index_max = tableid - 1
//::     mpu_min = tableid
//::     for table in pddict['tables']:
//::        if pddict['tables'][table]['type'] == 'Mpu':
//::            tabledict[table] = tableid
//::            tableid += 1
//::        #endif
//::     mpu_max = tableid - 1
//::     tblid_max = tableid
//::     #endfor


//::     for table in pddict['tables']:

/* Software Key Structure for p4-table '${table}' */
//::        if pddict['tables'][table]['type'] == 'Hash':
/* P4-table '${table}' is hash table */
//::        elif pddict['tables'][table]['type'] == 'Hash_OTcam':
/* P4-table '${table}' is hash table with over flow tcam */
//::        elif pddict['tables'][table]['type'] == 'Index':
/* P4-table '${table}' is index table */
//::        elif pddict['tables'][table]['type'] == 'Ternary':
/* P4-table '${table}' ternary table.*/
//::        else:
/* P4-table '${table}' Mpu/Keyless table.*/
//::        #endif

//::        if True or pddict['tables'][table]['type'] != 'Index':
//::            i = 1
//::            for fldunion in pddict['tables'][table]['fldunion_keys']:
//::                (container_field, un_fields) = fldunion
//::                if len(un_fields) > 1:
//::                    _max_un_field_width = max([p4fldwidth for (_, p4fldwidth, _, _, _) in un_fields])
//::                    cnt = 0
//::                    union_str = 'typedef union __' + table + '_union' + str(i) + '_t { /* Sourced from field union */\n'
//::                    for fields in un_fields:
//::                        (p4fldname, p4fldwidth, mask, key_byte_format, key_bit_format) = fields
//::                        p4fldname_data = table + '_union_' + p4fldname + '_data_' + str(cnt)
//::                        _fld_un_pad = (_max_un_field_width - p4fldwidth + 7) >> 3
    typedef struct {
//::                        if _fld_un_pad:
        uint8_t __fld_${p4fldname}_un_pad[${_fld_un_pad}];
//::                        #endif
//::                        if (p4fldwidth <= 8):
        uint8_t ${p4fldname};
//::                        elif (p4fldwidth <= 16):
        uint16_t ${p4fldname};
//::                        elif (p4fldwidth <= 32):
        uint32_t ${p4fldname};
//::                        elif (p4fldwidth <= 64):
        uint64_t ${p4fldname};
//::                        else:
//::                            p4fldwidth_byte = (p4fldwidth / 8) + (1 if p4fldwidth % 8 else 0)
        uint8_t ${p4fldname}[${p4fldwidth_byte}];
//::                        #endif
    } ${p4fldname_data};
//::                        union_str += '    ' + p4fldname_data + ' ' + 'data_' + str(cnt) + ';\n'
//::                        cnt = cnt + 1
//::                    #endfor
//::                    union_str += '} ' + table + '_union' + str(i) + '_t;\n'
${union_str}
//::                    i+=1
//::                #endif
//::            #endfor

typedef struct __attribute__((__packed__)) ${table}_swkey {
//::            for fields in pddict['tables'][table]['keys']:
//::                (p4fldname, p4fldwidth, mask, key_byte_format, key_bit_format) = fields
//::                if (p4fldwidth <= 8):
    uint8_t ${p4fldname};
//::                elif (p4fldwidth <= 16):
    uint16_t ${p4fldname};
//::                elif (p4fldwidth <= 32):
    uint32_t ${p4fldname};
//::                elif (p4fldwidth <= 64):
    uint64_t ${p4fldname};
//::                else:
//::                    p4fldwidth_byte = (p4fldwidth / 8) + (1 if p4fldwidth % 8 else 0)
    uint8_t ${p4fldname}[${p4fldwidth_byte}];
//::                #endif
//::            #endfor
//::            i = 1
//::            for fldunion in pddict['tables'][table]['fldunion_keys']:
//::                (container_field, un_fields) = fldunion
//::                if len(un_fields) > 1:
    ${table}_union${i}_t ${table}_u${i};
//::                    i+=1
//::                else:
//::                    for fields in un_fields:
//::                        (p4fldname, p4fldwidth, mask, key_byte_format, key_bit_format) = fields
//::                        if (p4fldwidth <= 8):
    uint8_t ${p4fldname};/* Sourced from field union */
//::                        elif (p4fldwidth <= 16):
    uint16_t ${p4fldname};/* Sourced from field union */
//::                        elif (p4fldwidth <= 32):
    uint32_t ${p4fldname};/* Sourced from field union */
//::                        elif (p4fldwidth <= 64):
    uint64_t ${p4fldname};/* Sourced from field union */
//::                        else:
//::                            p4fldwidth_byte = (p4fldwidth / 8) + (1 if p4fldwidth % 8 else 0)
    uint8_t ${p4fldname}[${p4fldwidth_byte}];/* Sourced from field union */
//::                        #endif
//::                    #endfor
//::                #endif
//::            #endfor
//::            for fields in pddict['tables'][table]['hdrunion_keys']:
//::                (p4fldname, p4fldwidth, mask, key_byte_format, key_bit_format) = fields
//::                if (p4fldwidth <= 8):
    uint8_t ${p4fldname};/* Sourced from header union */
//::                elif (p4fldwidth <= 16):
    uint16_t ${p4fldname};/* Sourced from header union */
//::                elif (p4fldwidth <= 32):
    uint32_t ${p4fldname};/* Sourced from header union */
//::                else:
//::                    p4fldwidth_byte = (p4fldwidth / 8) + (1 if p4fldwidth % 8 else 0)
    uint8_t ${p4fldname}[${p4fldwidth_byte}];/* Sourced from header union */
//::                #endif
//::            #endfor
} ${table}_swkey_t;

//::            if pddict['tables'][table]['type'] == 'Ternary' or pddict['tables'][table]['type'] == 'OTcam':

//::                i = 1
//::                for fldunion in pddict['tables'][table]['fldunion_keys']:
//::                    (container_field, un_fields) = fldunion
//::                    if len(un_fields) > 1:
//::                        _max_un_field_width = max([p4fldwidth for (_, p4fldwidth, _, _, _) in un_fields])
//::                        cnt = 0
//::                        union_str = 'typedef union __' + table + '_mask_union' + str(i) + '_t { /* Sourced from field union */\n'
//::                        for fields in un_fields:
//::                            (p4fldname, p4fldwidth, mask, key_byte_format, key_bit_format) = fields
//::                            _fld_un_pad = (_max_un_field_width - p4fldwidth + 7) >> 3
//::                            p4fldname_data = table + '_union_mask_' + p4fldname + '_data_' + str(cnt)
    typedef struct {
//::                            if _fld_un_pad:
        uint8_t __fld_${p4fldname}_un_pad_mask[${_fld_un_pad}];
//::                            #endif
//::                            if (p4fldwidth <= 8):
        uint8_t ${p4fldname}_mask;
//::                            elif (p4fldwidth <= 16):
        uint16_t ${p4fldname}_mask;
//::                            elif (p4fldwidth <= 32):
        uint32_t ${p4fldname}_mask;
//::                            elif (p4fldwidth <= 64):
        uint64_t ${p4fldname}_mask;
//::                            else:
//::                                p4fldwidth_byte = (p4fldwidth / 8) + (1 if p4fldwidth % 8 else 0)
        uint8_t ${p4fldname}_mask[${p4fldwidth_byte}];
//::                            #endif
    } ${p4fldname_data};
//::                            union_str += '    ' + p4fldname_data + ' ' + 'data_' + str(cnt) + ';\n'
//::                            cnt = cnt + 1
//::                        #endfor
//::                        union_str += '} ' + table + '_mask_union' + str(i) + '_t;\n'
${union_str}
//::                        i += 1
//::                    #endif
//::                #endfor

typedef struct __attribute__((__packed__)) ${table}_swkey_mask {
//::                for fields in pddict['tables'][table]['keys']:
//::                    (p4fldname, p4fldwidth, mask, key_byte_format, key_bit_format) = fields
//::                    if (p4fldwidth <= 8):
    uint8_t ${p4fldname}_mask;
//::                    elif (p4fldwidth <= 16):
    uint16_t ${p4fldname}_mask;
//::                    elif (p4fldwidth <= 32):
    uint32_t ${p4fldname}_mask;
//::                    elif (p4fldwidth <= 64):
    uint64_t ${p4fldname}_mask;
//::                    else:
//::                        p4fldwidth_byte = (p4fldwidth / 8) + (1 if p4fldwidth % 8 else 0)
    uint8_t ${p4fldname}_mask[${p4fldwidth_byte}];
//::                    #endif
//::                #endfor
//::                i = 1
//::                for fldunion in pddict['tables'][table]['fldunion_keys']:
//::                    (container_field, un_fields) = fldunion
//::                    if len(un_fields) > 1:
    ${table}_mask_union${i}_t ${table}_mask_u${i};
//::                        i += 1
//::                    else:
//::                        for fields in un_fields:
//::                            (p4fldname, p4fldwidth, mask, key_byte_format, key_bit_format) = fields
//::                            if (p4fldwidth <= 8):
    uint8_t ${p4fldname}_mask;/* Sourced from field union */
//::                            elif (p4fldwidth <= 16):
    uint16_t ${p4fldname}_mask;/* Sourced from field union */
//::                            elif (p4fldwidth <= 32):
    uint32_t ${p4fldname}_mask;/* Sourced from field union */
//::                            elif (p4fldwidth <= 64):
    uint64_t ${p4fldname}_mask;/* Sourced from field union */
//::                            else:
//::                                p4fldwidth_byte = (p4fldwidth / 8) + (1 if p4fldwidth % 8 else 0)
    uint8_t ${p4fldname}_mask[${p4fldwidth_byte}];/* Sourced from field union */
//::                            #endif
//::                        #endfor
//::                    #endif
//::                #endfor
//::                for fields in pddict['tables'][table]['hdrunion_keys']:
//::                    (p4fldname, p4fldwidth, mask, key_byte_format, key_bit_format) = fields
//::                    if (p4fldwidth <= 8):
    uint8_t ${p4fldname}_mask;/* Sourced from header union */
//::                        elif (p4fldwidth <= 16):
    uint16_t ${p4fldname}_mask;/* Sourced from header union */
//::                        elif (p4fldwidth <= 32):
    uint32_t ${p4fldname}_mask;/* Sourced from header union */
//::                        else:
//::                                p4fldwidth_byte = (p4fldwidth / 8) + (1 if p4fldwidth % 8 else 0)
    uint8_t ${p4fldname}_mask[${p4fldwidth_byte}];/* Sourced from header union */
//::                        #endif
//::                #endfor
} ${table}_swkey_mask_t;
//::            #endif
//::        #endif

//::        if len(pddict['tables'][table]['actions']):
//::            i = 0
//::            tbl = table.upper()
typedef enum ${table}_actions_enum {
//::            for action in pddict['tables'][table]['actions']:
//::                (actionname, actionflddict, _) = action
//::                actname = actionname.upper()
    ${tbl}_${actname}_ID = ${i},
//::                i+=1
//::            #endfor
    ${tbl}_MAX_ID = ${i}
} ${table}_actions_en;

//::                for action in pddict['tables'][table]['actions']:
//::                    (actionname, actionflddict, _) = action
//::                    if len(actionflddict):
typedef struct __attribute__((__packed__)) __${table}_${actionname} {
//::                        iter = 0
//::                        while iter < len(actionflddict):
//::                            actionfld = actionflddict[iter]
//::                            actionfldname  = actionfld['p4_name']
//::                            actionfldwidth = actionfld['len']
//::                            if ((iter + 1) < len(actionflddict)):
//::                                nextactionfld = actionflddict[iter + 1]
//::                                nextactionfldname = nextactionfld['p4_name']
//::                                if actionfldname == nextactionfldname:
//::                                    iter = iter + 1;
//::                                    actionfldwidth += nextactionfld['len']
//::                                #endif
//::                            #endif
//::                            iter = iter + 1
//::                            if not (pddict['tables'][table]['is_raw']):
//::                                if (actionfldwidth <= 8):
    uint8_t ${actionfldname};
//::                                elif (actionfldwidth <= 16):
    uint16_t ${actionfldname};
//::                                elif (actionfldwidth <= 32):
    uint32_t ${actionfldname};
//::                                elif (actionfldwidth <= 64):
    uint64_t ${actionfldname};
//::                                else:
//::                                    actionfldwidth_byte = (actionfldwidth / 8) + (1 if actionfldwidth % 8 else 0)
    uint8_t ${actionfldname}[${actionfldwidth_byte}];
//::                                #endif
//::                            else:
//::                                if (actionfldwidth <= 64):
    uint64_t ${actionfldname} : ${actionfldwidth};
//::                                else:
    uint8_t ${actionfldname}[${actionfldwidth}/8]; // bitwidth = ${actionfldwidth}
//::                                #endif
//::                            #endif
//::                        #endwhile
} ${table}_${actionname}_t;
//::                    #endif
//::                #endfor

typedef union __${table}_action_union {
//::                for action in pddict['tables'][table]['actions']:
//::                    (actionname, actionfldlist, _) = action
//::                    if len(actionfldlist):
    ${table}_${actionname}_t ${table}_${actionname};
//::                    #endif
//::                #endfor
} ${table}_action_union_t;

typedef struct __attribute__((__packed__)) __${table}_actiondata {
//::            if not (pddict['tables'][table]['is_raw']):
    uint8_t action_id;
//::            #endif
    ${table}_action_union_t action_u;
} ${table}_actiondata_t;
//::        #endif

//::     #endfor

//::    if len(tabledict):


/* List of Table Ids assigned for every p4 table.
 * This table id is needed to operate on any P4-table
 * using APIs
 *   p4pd_hwentry_query()
 *   p4pd_hwkey_hwmask_build()
 *   p4pd_entry_write()
 *   p4pd_entry_read()
 */
typedef enum ${prefix}_table_ids_ {
    P4${caps_p4prog}TBL_ID_HASH_MIN = ${hash_min},
    P4${caps_p4prog}TBL_ID_HASH_MAX = ${hash_max},
    P4${caps_p4prog}TBL_ID_HASH_OTCAM_MIN = ${hash_otcam_min},
    P4${caps_p4prog}TBL_ID_HASH_OTCAM_MAX = ${hash_otcam_max},
    P4${caps_p4prog}TBL_ID_TCAM_MIN = ${tcam_min},
    P4${caps_p4prog}TBL_ID_TCAM_MAX = ${tcam_max},
    P4${caps_p4prog}TBL_ID_INDEX_MIN = ${index_min},
    P4${caps_p4prog}TBL_ID_INDEX_MAX = ${index_max},
    P4${caps_p4prog}TBL_ID_MPU_MIN = ${mpu_min},
    P4${caps_p4prog}TBL_ID_MPU_MAX = ${mpu_max},
    P4${caps_p4prog}TBL_ID_TBLMIN = ${start_table_base},
//::        for  tblname in sorted(tabledict, key=tabledict.get):
//::            caps_tblname = tblname.upper() 
    P4${caps_p4prog}TBL_ID_${caps_tblname} = ${tabledict[tblname]},
//::        #endfor
    P4${caps_p4prog}TBL_ID_TBLMAX = ${tblid_max}
} ${prefix}_table_id;


#define P4${caps_p4prog}TBL_NAME_MAX_LEN 80 /* p4 table name will be truncated to 80 characters */

extern char ${prefix}_tbl_names[P4${caps_p4prog}TBL_ID_TBLMAX][P4${caps_p4prog}TBL_NAME_MAX_LEN];
extern uint16_t ${prefix}_tbl_swkey_size[P4${caps_p4prog}TBL_ID_TBLMAX];
extern uint16_t ${prefix}_tbl_sw_action_data_size[P4${caps_p4prog}TBL_ID_TBLMAX];

inline void ${prefix}_prep_p4tbl_names()
{
//::        for  tblname in sorted(tabledict, key=tabledict.get):
//::            caps_tblname = tblname.upper() 
    strncpy(${prefix}_tbl_names[P4${caps_p4prog}TBL_ID_${caps_tblname}], "${tblname}", strlen("${tblname}"));
//::        #endfor
}

inline void ${prefix}_prep_p4tbl_sw_struct_sizes()
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

inline int ${prefix}_get_max_action_id(uint32_t tableid)
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

inline void ${prefix}_get_action_name(uint32_t tableid, int actionid, char *action_name)
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

//::    for i in range(len(pddict['egr-hdrs'])):
//::        if pddict['egr-hdrs'][i].name == 'tm_replication_data':
//::            tot_width = 0
//::            for field in pddict['egr-hdrs'][i].fields:
//::                tot_width += field.width
//::            #endfor
//::            width_done = 0
//::            bits_available = 0
typedef struct __attribute__((__packed__)) __p4_replication_data_t {
//::            for field in reversed(pddict['egr-hdrs'][i].fields):
//::                if bits_available < field.width:
//::                    if tot_width - width_done <= 8:
//::                        type = 'uint8_t'
//::                        bits_available += 8
//::                        width_done += 8
//::                    elif tot_width - width_done <= 16:
//::                        type = 'uint16_t'
//::                        bits_available += 16
//::                        width_done += 16
//::                    elif tot_width - width_done <= 32:
//::                        type = 'uint32_t'
//::                        bits_available += 32
//::                        width_done += 32
//::                    else:
//::                        type = 'uint64_t'
//::                        bits_available += 64
//::                        width_done += 64
//::                    #endif
//::                #endif
    ${type} ${field.name}:${field.width};
//::                bits_available -= field.width
//::            #endfor
} p4_replication_data_t;
//::            break
//::        #endif
//::    #endfor

#define P4_REPL_TABLE_NAME                 "replication_table"
#define P4_REPL_TABLE_ID                   P4TBL_ID_TBLMAX
#define P4_REPL_TABLE_DEPTH                (64 * 1024)
#define P4_REPL_ENTRY_WIDTH                (sizeof(p4_replication_data_t))
#define CAPRI_REPL_NUM_P4_ENTRIES_PER_NODE (60 / P4_REPL_ENTRY_WIDTH)
#endif
