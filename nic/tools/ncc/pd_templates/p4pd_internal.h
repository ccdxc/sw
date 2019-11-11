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
//::    filename = p4prog + 'p4pd_internal.h'
//:: else:
//::    caps_p4prog = ''
//::    prefix = 'p4pd'
//::	start_table_base = 1
//::    filename = 'p4pd_internal.h'
//:: #endif
/*
 * ${filename}
 * Pensando Systems
 */

/*
 * This file is generated from P4 program. Any changes made to this file will
 * be lost.
 */
#ifndef __P4PD_INTERNAL${caps_p4prog}_H__
#define __P4PD_INTERNAL${caps_p4prog}_H__

#include <stdint.h>

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
typedef union __${table}_union${i}_bitfield_t { /* Sourced from field union */
//::                    for fields in reversed(un_fields):
//::                        (p4fldname, p4fldwidth, mask, key_byte_format, key_bit_format) = fields
//::                        _fld_un_pad = (_max_un_field_width - p4fldwidth + 7) >> 3
    struct {
//::                        if _fld_un_pad:
        uint64_t __fld_${p4fldname}_un_pad : ${_fld_un_pad};
//::                        #endif
//::                        if (p4fldwidth <= 64):
        uint64_t ${p4fldname} : ${p4fldwidth};
//::                        else:
//::                            p4fldwidth_byte = (p4fldwidth / 8) + (1 if p4fldwidth % 8 else 0)
        uint8_t ${p4fldname}[${p4fldwidth_byte}];
//::                        #endif
    };
//::                    #endfor
} ${table}_union${i}_bitfield_t;

//::                #endif
//::                i+=1
//::            #endfor

typedef struct __attribute__((__packed__)) ${table}_bitfield_swkey {
//::            if pddict['tables'][table]['keys']:
//::                for fields in reversed(pddict['tables'][table]['keys']):
//::                    (p4fldname, p4fldwidth, mask, key_byte_format, key_bit_format) = fields
//::                    if (p4fldwidth <= 64):
    uint64_t ${p4fldname} : ${p4fldwidth};
//::                    else:
//::                    p4fldwidth_byte = (p4fldwidth / 8) + (1 if p4fldwidth % 8 else 0)
    uint8_t ${p4fldname}[${p4fldwidth_byte}];
//::                    #endif
//::                #endfor
//::            #endif
//::            i = 1
//::            for fldunion in pddict['tables'][table]['fldunion_keys']:
//::                (container_field, un_fields) = fldunion
//::                if len(un_fields) > 1:
    ${table}_union${i}_bitfield_t ${table}_u${i};
//::                else:
//::                    for fields in reversed(un_fields):
//::                        (p4fldname, p4fldwidth, mask, key_byte_format, key_bit_format) = fields
//::                        if (p4fldwidth <= 64):
    uint64_t ${p4fldname} : ${p4fldwidth};/* Sourced from field union */
//::                        else:
//::                            p4fldwidth_byte = (p4fldwidth / 8) + (1 if p4fldwidth % 8 else 0)
    uint8_t ${p4fldname}[${p4fldwidth_byte}];/* Sourced from field union */
//::                        #endif
//::                    #endfor
//::                #endif
//::                i+=1
//::            #endfor
//::            for fields in reversed(pddict['tables'][table]['hdrunion_keys']):
//::                (p4fldname, p4fldwidth, mask, key_byte_format, key_bit_format) = fields
//::                if (p4fldwidth <= 64):
    uint64_t ${p4fldname} : ${p4fldwidth};/* Sourced from header union */
//::                else:
//::                    p4fldwidth_byte = (p4fldwidth / 8) + (1 if p4fldwidth % 8 else 0)
    uint8_t ${p4fldname}[${p4fldwidth_byte}];/* Sourced from header union */
//::                #endif
//::            #endfor
} ${table}_swkey_bitfield_t;

//::            if pddict['tables'][table]['type'] == 'Ternary' or pddict['tables'][table]['type'] == 'OTcam':
//::                i = 1
//::                for fldunion in pddict['tables'][table]['fldunion_keys']:
//::                    (container_field, un_fields) = fldunion
//::                    if len(un_fields) > 1:
//::                        _max_un_field_width = max([p4fldwidth for (_, p4fldwidth, _, _, _) in un_fields])
typedef union __${table}_mask_union${i}_bitfield_t { /* Sourced from field union */
//::                        for fields in reversed(un_fields):
//::                            (p4fldname, p4fldwidth, mask, key_byte_format, key_bit_format) = fields
//::                            _fld_un_pad = (_max_un_field_width - p4fldwidth + 7) >> 3
    struct {
//::                            if _fld_un_pad:
        uint64_t __fld_${p4fldname}_un_pad_mask : ${_fld_un_pad};
//::                            #endif
//::                            if (p4fldwidth <= 64):
        uint64_t ${p4fldname}_mask : ${p4fldwidth};
//::                            else:
//::                                p4fldwidth_byte = (p4fldwidth / 8) + (1 if p4fldwidth % 8 else 0)
        uint8_t ${p4fldname}_mask[${p4fldwidth_byte}];
//::                            #endif
    };
//::                        #endfor
} ${table}_mask_union${i}_bitfield_t;

//::                        i += 1
//::                    #endif
//::                #endfor
typedef struct __attribute__((__packed__)) ${table}_swkey_mask_bitfield {
//::                if pddict['tables'][table]['keys']:
//::                    for fields in reversed(pddict['tables'][table]['keys']):
//::                        (p4fldname, p4fldwidth, mask, key_byte_format, key_bit_format) = fields
//::                        if (p4fldwidth <= 64):
    uint64_t ${p4fldname}_mask : ${p4fldwidth};
//::                        else:
//::                            p4fldwidth_byte = (p4fldwidth / 8) + (1 if p4fldwidth % 8 else 0)
    uint8_t ${p4fldname}_mask[${p4fldwidth_byte}];
//::                        #endif
//::                    #endfor
//::                #endif
//::                i = 1
//::                for fldunion in pddict['tables'][table]['fldunion_keys']:
//::                    (container_field, un_fields) = fldunion
//::                    if len(un_fields) > 1:
    ${table}_mask_union${i}_bitfield_t ${table}_mask_u${i};
//::                        i += 1
//::                    else:
//::                        for fields in reversed(un_fields):
//::                            (p4fldname, p4fldwidth, mask, key_byte_format, key_bit_format) = fields
//::                            if (p4fldwidth <= 64):
    uint64_t ${p4fldname}_mask : ${p4fldwidth};/* Sourced from field union */
//::                            else:
//::                                p4fldwidth_byte = (p4fldwidth / 8) + (1 if p4fldwidth % 8 else 0)
    uint8_t ${p4fldname}_mask[${p4fldwidth_byte}];/* Sourced from field union */
//::                            #endif
//::                        #endfor
//::                    #endif
//::                #endfor
//::                for fields in reversed(pddict['tables'][table]['hdrunion_keys']):
//::                    (p4fldname, p4fldwidth, mask, key_byte_format, key_bit_format) = fields
//::                    if (p4fldwidth <= 64):
    uint64_t ${p4fldname}_mask : ${p4fldwidth};/* Sourced from header union */
//::                    else:
//::                                p4fldwidth_byte = (p4fldwidth / 8) + (1 if p4fldwidth % 8 else 0)
    uint8_t ${p4fldname}_mask[${p4fldwidth_byte}];/* Sourced from header union */
//::                    #endif
//::                #endfor
} ${table}_swkey_mask_bitfield_t;
//::            #endif
//::        #endif
//::        if len(pddict['tables'][table]['actions']):
//::            i = 0
//::            tbl = table.upper()
//::                for action in pddict['tables'][table]['actions']:
//::                    (actionname, actionflddict, _) = action
//::                    if len(actionflddict):
typedef struct __attribute__((__packed__)) __${table}_${actionname}_bitfield {
//::                        total_bits = 0
//::                        for actionfld in actionflddict:
//::                            total_bits += actionfld['len']
//::                        #endfor
//::                        byte_align_pad = 0
//::                        if (total_bits % 8) > 0: byte_align_pad = 8 - (total_bits % 8)
//::                            if byte_align_pad > 0:
    uint64_t _${actionname}_pad : ${byte_align_pad};
//::                            #endif
//::                            # reverse the order of d fields
//::                            flds = OrderedDict()
//::                            for fld in actionflddict:
//::                                actionfldname  = fld['p4_name']
//::                                actionfldwidth = fld['len']
//::                                if actionfldname in flds:
//::                                    flds[actionfldname] = flds[actionfldname] + actionfldwidth
//::                                else:
//::                                    flds[actionfldname] = actionfldwidth
//::                                #endif
//::                            #endfor
//::                            for fldname, fldlen in reversed(flds.items()):
//::                                if (fldlen <= 64):
    uint64_t ${fldname} : ${fldlen};
//::                                else:
//::                                    actionfldwidth_byte = (fldlen / 8) + (1 if fldlen % 8 else 0)
    uint8_t ${fldname}[${actionfldwidth_byte}];
//::                                #endif
//::                            #endfor
} ${table}_${actionname}_bitfield_t;
//::                        #endif
//::                #endfor
typedef union __${table}_action_union_bitfield {
//::                for action in pddict['tables'][table]['actions']:
//::                    (actionname, actionflddict, _) = action
//::                    if len(actionflddict):
    ${table}_${actionname}_bitfield_t ${table}_${actionname};
//::                    #endif
//::                #endfor
} ${table}_action_union_bitfield_t;

typedef struct __attribute__((__packed__)) __${table}_actiondata_bitfield {
    ${table}_action_union_bitfield_t action_u;
//::            if not (pddict['tables'][table]['is_raw']) and len(pddict['tables'][table]['actions']) > 1:
    uint64_t action_id : 8;
//::            #endif
} ${table}_actiondata_bitfield_t;

//::        #endif
//::     #endfor

#endif
