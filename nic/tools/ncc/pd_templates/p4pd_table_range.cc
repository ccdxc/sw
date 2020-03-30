//:: import os, pdb
//:: from collections import OrderedDict
//:: pddict = _context['pddict']
//:: #pdb.set_trace()
//:: if pddict['p4plus']:
//::    p4prog = pddict['p4program'] + '_'
//::    caps_p4prog = '_' + pddict['p4program'].upper() + '_'
//::    if pddict['p4plus_module'] == 'rxdma':
//::        start_table_base = 101
//::        prefix = 'p4pd_rxdma'
//::    elif pddict['p4plus_module'] == 'txdma':
//::        start_table_base = 201
//::        prefix = 'p4pd_txdma'
//::    elif pddict['p4plus_module'] == 'sxdma':
//::        start_table_base = 401
//::        prefix = 'p4pd_sxdma'
//::    else:
//::        start_table_base = 301
//::        prefix = 'p4pd_p4plus'
//::    #endif
//:: else:
//::    caps_p4prog = ''
//::    prefix = 'p4pd'
//::	start_table_base = 1
//:: #endif
/*
 * p4pd_table_range.cc
 * Pensando Systems
 */

/*
 * This file provides table-id APIs based on lookup type. For each lookup
 * type min and max table-id is returned. All tableid within the returned
 * min and max range are lookuped in pipeline in the same way.
 */

/*
 * This file is generated from P4 program. Any changes made to this file will
 * be lost.
 */

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
//::     mpu_max = tableid
//::     for table in pddict['tables']:
//::        if pddict['tables'][table]['type'] == 'Mpu':
//::            tabledict[table] = tableid
//::            tableid += 1
//::        #endif
//::        mpu_max = tableid - 1
//::     #endfor
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
#include <stdint.h>

#if 0
uint32_t ${prefix}_hash_tableid_min_get()
{
    return(${hash_min});
}

uint32_t ${prefix}_hash_tableid_max_get()
{
    return(${hash_max});
}

uint32_t ${prefix}_hash_otcam_tableid_min_get()
{
    return(${hash_otcam_min});
}

uint32_t ${prefix}_hash_otcam_tableid_max_get()
{
    return(${hash_otcam_max});
}

uint32_t ${prefix}_index_tableid_min_get()
{
    return(${index_min});
}

uint32_t ${prefix}_tcam_tableid_min_get()
{
    return(${tcam_min});
}

uint32_t ${prefix}_tcam_tableid_max_get()
{
    return(${tcam_max});
}

uint32_t ${prefix}_index_tableid_max_get()
{
    return(${index_max});
}

uint32_t ${prefix}_mpu_tableid_min_get()
{
    return(${mpu_min});
}

uint32_t ${prefix}_mpu_tableid_max_get()
{
    return(${mpu_max});
}
#endif
uint32_t ${prefix}_tableid_min_get()
{
    return(${start_table_base});
}

uint32_t ${prefix}_tableid_max_get()
{
    return(${tblid_max});
}
