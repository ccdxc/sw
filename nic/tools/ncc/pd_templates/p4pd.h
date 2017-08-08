//:: import os, pdb
//:: from collections import OrderedDict

//:: pddict = _context['pddict']
//:: #pdb.set_trace()


//:: if pddict['p4plus']:
//::    caps_p4prog = '_' + pddict['p4program'].upper() + '_'
//::    prefix = 'p4pd_' + pddict['p4program']
//:: else:
//::    caps_p4prog = ''
//::    prefix = 'p4pd'
//:: #endif

#ifndef __P4PD${caps_p4prog}_H__
#define __P4PD${caps_p4prog}_H__


#include <string.h>
#include <hal_pd_error.hpp>

/*
 * p4pd.h
 * Mahesh Shirshyad (Pensando Systems)
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

//::     tabledict = {}
//::     tableid = 1
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

//::        if pddict['tables'][table]['type'] != 'Index':

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
    union { /* Sourced from field union */
//::                    for fields in un_fields:
//::                        (p4fldname, p4fldwidth, mask, key_byte_format, key_bit_format) = fields
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
//::                    #endfor
    } ${table}_u${i};
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
//::                        else:
//::                            p4fldwidth_byte = (p4fldwidth / 8) + (1 if p4fldwidth % 8 else 0)
    uint8_t ${p4fldname}[${p4fldwidth_byte}];/* Sourced from field union */
//::                        #endif
//::                    #endfor
//::                #endif
//::            #endfor
//::            for hdrunion in pddict['tables'][table]['hdrunion_keys']:
//::                (union_name, un_fields) = hdrunion
//::                if len(un_fields) > 1:
    union { /* Sourced from header union */
//::                    for fields in un_fields:
//::                        (p4fldname, p4fldwidth, mask, key_byte_format, key_bit_format) = fields
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
//::                    #endfor
    } ${table}_hdr_u${i};
//::                    i+=1
//::                else:
//::                    for fields in un_fields:
//::                        (p4fldname, p4fldwidth, mask, key_byte_format, key_bit_format) = fields
//::                        if (p4fldwidth <= 8):
    uint8_t ${p4fldname};/* Sourced from header union */
//::                        elif (p4fldwidth <= 16):
    uint16_t ${p4fldname};/* Sourced from header union */
//::                        elif (p4fldwidth <= 32):
    uint32_t ${p4fldname};/* Sourced from header union */
//::                        else:
//::                            p4fldwidth_byte = (p4fldwidth / 8) + (1 if p4fldwidth % 8 else 0)
    uint8_t ${p4fldname}[${p4fldwidth_byte}];/* Sourced from header union */
//::                        #endif
//::                    #endfor
//::                #endif
//::            #endfor
} ${table}_swkey_t;


//::            if pddict['tables'][table]['type'] == 'Ternary' or pddict['tables'][table]['type'] == 'OTcam':
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
    union { /* Sourced from field union */
//::                        for fields in un_fields:
//::                            (p4fldname, p4fldwidth, mask, key_byte_format, key_bit_format) = fields
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
//::                        #endfor
    } ${table}_mask_u${i};
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
//::                for fldunion in pddict['tables'][table]['hdrunion_keys']:
//::                    (unionname, un_fields) = fldunion
//::                    if len(un_fields) > 1:
    union { /* Sourced from header union */
//::                        for fields in un_fields:
//::                            (p4fldname, p4fldwidth, mask, key_byte_format, key_bit_format) = fields
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
//::                        #endfor
    } ${table}_mask_hdr_u${i};
//::                        i += 1
//::                    else:
//::                        for fields in un_fields:
//::                            (p4fldname, p4fldwidth, mask, key_byte_format, key_bit_format) = fields
//::                            if (p4fldwidth <= 8):
    uint8_t ${p4fldname}_mask;/* Sourced from header union */
//::                            elif (p4fldwidth <= 16):
    uint16_t ${p4fldname}_mask;/* Sourced from header union */
//::                            elif (p4fldwidth <= 32):
    uint32_t ${p4fldname}_mask;/* Sourced from header union */
//::                            else:
//::                                p4fldwidth_byte = (p4fldwidth / 8) + (1 if p4fldwidth % 8 else 0)
    uint8_t ${p4fldname}_mask[${p4fldwidth_byte}];/* Sourced from header union */
//::                            #endif
//::                        #endfor
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
//::                (actionname, actionfldlist) = action
//::                actname = actionname.upper()
    ${tbl}_${actname}_ID = ${i},
//::                i+=1
//::            #endfor
    ${tbl}_MAX_ID = ${i}
} ${table}_actions_en;

typedef struct __attribute__((__packed__)) __${table}_actiondata {
    uint8_t actionid;
    union {
//::                for action in pddict['tables'][table]['actions']:
//::                    (actionname, actionfldlist) = action
//::                    if len(actionfldlist):
        struct __attribute__((__packed__)) __${table}_${actionname} {
//::                        for actionfld in actionfldlist:
//::                            actionfldname, actionfldwidth = actionfld
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
//::                        #endfor
        } ${table}_${actionname};
//::                    #endif
//::                #endfor
    } ${table}_action_u;
} ${table}_actiondata;
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
    P4${caps_p4prog}TBL_ID_TBLMIN = 1,
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
    ${prefix}_tbl_sw_action_data_size[P4${caps_p4prog}TBL_ID_${caps_tblname}]= sizeof(${tblname}_actiondata);
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

#endif 
