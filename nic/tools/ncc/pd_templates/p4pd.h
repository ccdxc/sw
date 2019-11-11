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
//::    filename = p4prog + 'p4pd.h'
//:: else:
//::    caps_p4prog = ''
//::    prefix = 'p4pd'
//::	start_table_base = 1
//::    filename = 'p4pd.h'
//:: #endif
/*
 * ${filename}
 * Pensando Systems
 */
/* This file contains data structures and APIs needed to operate on each 
 * P4 Table. 
 *
 * For every P4table, 
 *   1. A "C" structure to build match key is provided. This "C" structure 
 *      aka software key (swkey) is used by asic-library to work with p4-tables.
 *      This table key structure is not same as what is represented in hardware.
 *      Hardware representation is kept transparent to application or users of
 *      P4-table by providing necessary function to build hardware key from 
 *      swkey.
 *
 *   2. A list of action identifiers associated with table. This list is 
 *      provided as enumertion values. Action ID has to be used in order to 
 *      associate action with match-key.
 *
 *   3. A union of structures where each structure is built using action 
 *      parameters associated with an action. Each structure corresponds to
 *      one of the many actions associated with the match table.
 *
 *   4. In order to keep application layer agnostic of hardware representation 
 *      folllowing APIs are provided.
 *      5.a  <table_name>_hwkey_query() which returns hwkey length, 
 *           hwactiondata length associated with the key. These lengths
 *           can be used to allocate memory before using p4 table operation
 *           APIs. All table operation APIs expect caller of the APIs to
 *           provide necessary memory for table operation API arguments.
 *      5.b  <table_name>_hwkey_build() which converts table key from software
 *           representation to hardware representation.
 *
 *   5. In order to install an entry into P4 table or read an entry from p4-table
 *      following APIs are provided
 *      <table_name>_entry_write()
 *      <table_name>_entry_read()
 */

/*
 * This file groups data structure and APIs to operate on p4 table on each
 * P4-table basis.
 */

/*
 * This file is generated from P4 program. Any changes made to this file will
 * be lost.
 */
#ifndef __P4PD${caps_p4prog}_H__
#define __P4PD${caps_p4prog}_H__

#include <string.h>

//::    tabledict = {}
//::    tableid = start_table_base
//::    hash_min = tableid
//::    hash_max = tableid
//::    for table in pddict['tables']:
//::       if pddict['tables'][table]['type'] == 'Hash':
//::           tabledict[table] = tableid
//::           tableid += 1
//::       #endif
//::    #endfor
//::    if tableid > hash_max:
//::       hash_max = tableid - 1
//::    #endif
//::    hash_otcam_min = tableid
//::    hash_otcam_max = tableid
//::    for table in pddict['tables']:
//::       if pddict['tables'][table]['type'] == 'Hash_OTcam':
//::           tabledict[table] = tableid
//::           tableid += 1
//::       #endif
//::    #endfor
//::    if tableid > hash_otcam_max:
//::       hash_otcam_max = tableid - 1
//::    #endif
//::    tcam_min = tableid
//::    tcam_max = tableid
//::    for table in pddict['tables']:
//::       if pddict['tables'][table]['type'] == 'Ternary':
//::           tabledict[table] = tableid
//::           tableid += 1
//::       #endif
//::    #endfor
//::    if tableid > tcam_max:
//::       tcam_max = tableid - 1
//::    #endif
//::    index_min = tableid
//::    index_max = tableid
//::    for table in pddict['tables']:
//::       if pddict['tables'][table]['type'] == 'Index':
//::           tabledict[table] = tableid
//::           tableid += 1
//::       #endif
//::    #endfor
//::    if tableid > index_max:
//::       index_max = tableid - 1
//::    #endif
//::    mpu_min = tableid
//::    mpu_max = tableid
//::    for table in pddict['tables']:
//::       if pddict['tables'][table]['type'] == 'Mpu':
//::           tabledict[table] = tableid
//::           tableid += 1
//::       #endif
//::    #endfor
//::    if tableid > mpu_max:
//::       mpu_max = tableid - 1
//::    #endif
//::    tblid_max = tableid
//::    table_min_dict = {}
//::    table_min_dict['P4' + caps_p4prog + 'TBL_ID_HASH_MIN'] = hash_min
//::    table_min_dict['P4' + caps_p4prog + 'TBL_ID_HASH_MAX'] = hash_max
//::    table_min_dict['P4' + caps_p4prog + 'TBL_ID_HASH_OTCAM_MIN'] = hash_otcam_min
//::    table_min_dict['P4' + caps_p4prog + 'TBL_ID_HASH_OTCAM_MAX'] = hash_otcam_max
//::    table_min_dict['P4' + caps_p4prog + 'TBL_ID_TCAM_MIN'] = tcam_min
//::    table_min_dict['P4' + caps_p4prog + 'TBL_ID_TCAM_MAX'] = tcam_max
//::    table_min_dict['P4' + caps_p4prog + 'TBL_ID_INDEX_MIN'] = index_min
//::    table_min_dict['P4' + caps_p4prog + 'TBL_ID_INDEX_MAX'] = index_max
//::    table_min_dict['P4' + caps_p4prog + 'TBL_ID_MPU_MIN'] = mpu_min
//::    table_min_dict['P4' + caps_p4prog + 'TBL_ID_MPU_MAX'] = mpu_max
//::    table_min_dict['P4' + caps_p4prog + 'TBL_ID_TBLMIN'] = start_table_base
//::    table_min_dict['P4' + caps_p4prog + 'TBL_ID_TBLMAX'] = tblid_max
//::    _context['P4TBL_TYPES'] = table_min_dict


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
typedef union __${table}_union${i}_t { /* Sourced from field union */
//::                    for fields in un_fields:
//::                        (p4fldname, p4fldwidth, mask, key_byte_format, key_bit_format) = fields
//::                        _fld_un_pad = (_max_un_field_width - p4fldwidth + 7) >> 3
    struct {
//::                        if _fld_un_pad:
        uint8_t __fld_${p4fldname}_un_pad[${_fld_un_pad}];
//::                        #endif
//::                        if (p4fldwidth <= 8):
        uint8_t ${p4fldname};
//::                        elif (p4fldwidth <= 16):
        uint16_t ${p4fldname};
//::                        elif (p4fldwidth <= 32):
        uint32_t ${p4fldname};
//::                        else:
//::                            p4fldwidth_byte = (p4fldwidth / 8) + (1 if p4fldwidth % 8 else 0)
        uint8_t ${p4fldname}[${p4fldwidth_byte}];
//::                        #endif
    };
//::                    #endfor
} ${table}_union${i}_t;
//::                #endif
//::                i+=1
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
//::                else:
//::                    for fields in un_fields:
//::                        (p4fldname, p4fldwidth, mask, key_byte_format, key_bit_format) = fields
//::                        if (p4fldwidth <= 8):
    uint8_t ${p4fldname};/* Sourced from field union */
//::                        elif (p4fldwidth <= 16):
    uint16_t ${p4fldname};/* Sourced from field union */
//::                        elif (p4fldwidth <= 32):
    uint32_t ${p4fldname};/* Sourced from field union */
//::                        else:
//::                            p4fldwidth_byte = (p4fldwidth / 8) + (1 if p4fldwidth % 8 else 0)
    uint8_t ${p4fldname}[${p4fldwidth_byte}];/* Sourced from field union */
//::                        #endif
//::                    #endfor
//::                #endif
//::                i+=1
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
//::                            p4fldwidth_byte = (p4fldwidth / 8) + (1 if p4fldwidth % 8 else 0)
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
typedef union __${table}_mask_union${i}_t { /* Sourced from field union */
//::                        for fields in un_fields:
//::                            (p4fldname, p4fldwidth, mask, key_byte_format, key_bit_format) = fields
//::                            _fld_un_pad = (_max_un_field_width - p4fldwidth + 7) >> 3
    struct {
//::                            if _fld_un_pad:
        uint8_t __fld_${p4fldname}_un_pad_mask[${_fld_un_pad}];
//::                            #endif
//::                            if (p4fldwidth <= 8):
        uint8_t ${p4fldname}_mask;
//::                            elif (p4fldwidth <= 16):
        uint16_t ${p4fldname}_mask;
//::                            elif (p4fldwidth <= 32):
        uint32_t ${p4fldname}_mask;
//::                            else:
//::                                p4fldwidth_byte = (p4fldwidth / 8) + (1 if p4fldwidth % 8 else 0)
        uint8_t ${p4fldname}_mask[${p4fldwidth_byte}];
//::                            #endif
    };
//::                        #endfor
} ${table}_mask_union${i}_t;
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
//::                    elif (p4fldwidth <= 16):
    uint16_t ${p4fldname}_mask;/* Sourced from header union */
//::                    elif (p4fldwidth <= 32):
    uint32_t ${p4fldname}_mask;/* Sourced from header union */
//::                    else:
//::                                p4fldwidth_byte = (p4fldwidth / 8) + (1 if p4fldwidth % 8 else 0)
    uint8_t ${p4fldname}_mask[${p4fldwidth_byte}];/* Sourced from header union */
//::                    #endif
//::                #endfor
} ${table}_swkey_mask_t;
//::            #endif
//::        #endif

//::        if len(pddict['tables'][table]['actions']):
//::            i = 0
//::            tbl = table.upper()
typedef enum ${table}_actions_enum {
//::            for action in pddict['tables'][table]['actions']:
//::                (actionname, actionfldlist, _) = action
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
//::                            if (actionfldwidth <= 8):
    uint8_t ${actionfldname};
//::                            elif (actionfldwidth <= 16):
    uint16_t ${actionfldname};
//::                            elif (actionfldwidth <= 32):
    uint32_t ${actionfldname};
//::                            else:
//::                                actionfldwidth_byte = (actionfldwidth / 8) + (1 if actionfldwidth % 8 else 0)
    uint8_t ${actionfldname}[${actionfldwidth_byte}];
//::                            #endif
//::                        #endwhile
} ${table}_${actionname}_t;
//::                    #endif
//::                    if 'appdatafields' in pddict['tables'][table].keys() and len(pddict['tables'][table]['appdatafields']) > 0:
//::                        actionname = pddict['tables'][table]['appdatafields'].keys()[0]
//::                        fieldlist = pddict['tables'][table]['appdatafields'][actionname]
//::                        if len(fieldlist):

typedef struct __attribute__((__packed__)) __${table}_appdata {
//::                            for fieldname, fieldwidth in fieldlist:
//::                                if (fieldwidth <= 8):
    uint8_t ${fieldname};
//::                                elif (fieldwidth <= 16):
    uint16_t ${fieldname};
//::                                elif (fieldwidth <= 32):
    uint32_t ${fieldname};
//::                                else:
//::                                    byte_fldwidth = (fieldwidth + 7) >> 3
    uint8_t ${fieldname}[${byte_fldwidth}];
//::                                #endif
//::                            #endfor
} ${table}_appdata_t;
//::                        #endif
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
//::        for  tblname in sorted(tabledict, key=tabledict.get):
//::            caps_tblname = tblname.upper() 
    P4${caps_p4prog}TBL_ID_${caps_tblname} = ${tabledict[tblname]},
//::        #endfor
    __P4${caps_p4prog}TBL_ID_TBLMAX = ${tblid_max}
} ${prefix}_table_id;


#define P4${caps_p4prog}TBL_NAME_MAX_LEN 80 /* p4 table name will be truncated to 80 characters */

//::        if pddict['p4plus']:
//::            api_prefix = 'p4pd_' + pddict['p4program']

void
${api_prefix}_raw_table_hwentry_query(uint32_t tableid, uint8_t action_id,
                                      uint32_t *hwactiondata_len);

int
${api_prefix}_entry_pack(uint32_t tableid,
                         uint8_t action_id,
                         void    *actiondata,
                         uint8_t *packed_entry);
int
${api_prefix}_entry_unpack(uint32_t tableid,
                           uint8_t actionid,
                           uint8_t *hwentry,
                           uint32_t entry_width,
                           void    *actiondata);
int
${api_prefix}_is_raw_table(uint32_t tableid);
//::        #endif

//::    #endif


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

//:: if not pddict['p4plus']:
#define P4_REPL_TABLE_NAME                 "replication_table"
#define P4_REPL_TABLE_ID                   __P4TBL_ID_TBLMAX
#define P4_REPL_TABLE_DEPTH                (64 * 1024)
#define P4_REPL_ENTRY_WIDTH                (sizeof(p4_replication_data_t))
#define CAPRI_REPL_NUM_P4_ENTRIES_PER_NODE (60 / P4_REPL_ENTRY_WIDTH)
//:: #endif
#endif
