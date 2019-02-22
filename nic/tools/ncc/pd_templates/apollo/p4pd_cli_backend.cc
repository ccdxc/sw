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

/*
 * p4pd_backend.cc
 * Pensando Systems
 */

#define MAX(x, y) (((x) > (y)) ? (x) : (y))

#include <iostream>
#include "nic/sdk/include/sdk/base.hpp"
#include "nic/sdk/asic/rw/asicrw.hpp"
#include "gen/p4gen/${hdrdir}/include/${p4prog}p4pd.h"
#include "nic/sdk/lib/p4/p4_utils.hpp"
#include "nic/sdk/lib/p4/p4_api.hpp"

using std::cout;

//::
//::     tabledict = OrderedDict() # key=table-name
//::     tableid = start_table_base
//::     for table in pddict['tables']:
//::        tabledict[table] = tableid
//::        tableid += 1
//::     #endfor
//::
//::     if pddict['p4plus']:
//::         api_prefix = 'p4pd_' + pddict['p4program']
//::     else:
//::         api_prefix = 'p4pd'
//::     #endif

p4pd_error_t
${api_prefix}_entry_write (uint32_t tableid,
                           uint32_t index,
                           void     *swkey,
                           void     *swkey_mask,
                           void     *actiondata)
{
#if 0
    p4pd_error_t    ret;
    void            *swkey = NULL, *swkey_mask = NULL, *actiondata = NULL;
    uint32_t        hwkey_len = 0, hwkeymask_len = 0, hwactiondata_len = 0;
    void            *hwkey = NULL, *hwkeymask = NULL;

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
            actiondata = (void *)(${table}_actiondata_t*)actiondata;
            break;

        //::            #endif
        //::            if pddict['tables'][table]['type'] == 'Hash' or pddict['tables'][table]['type'] == 'Hash_OTcam':
            swkey = (void *)((${table}_swkey_t*)swkey);

            actiondata = (void *)((${table}_actiondata_t*)actiondata);
            break;

        //::            #endif
        //::            if pddict['tables'][table]['type'] == 'Ternary':
            swkey = (void *)((${table}_swkey_t*)swkey);

            swkey_mask = (void *)((${table}_swkey_mask_t*)swkey_mask);

            actiondata = (void *)((${table}_actiondata_t*)actiondata);
            break;

        //::            #endif
        //::        #endfor
        default:
            // Invalid tableid
            return (P4PD_FAIL);
            break;
    }

    p4pd_hwentry_query(tableid, &hwkey_len, &hwkeymask_len, &hwactiondata_len);

    hwkey = new(nothrow) char[(hwkey_len + 7)/8];
    if (!hwkey) 
        return P4PD_FAIL;

    hwkeymask = new(nothrow) char[(hwkeymask_len + 7)/8];
    if (!hwkeymask) {
        delete[] hwkey;
        return P4PD_FAIL;
    }

    memset(hwkey,     0, (hwkey_len     + 7)/8);
    memset(hwkeymask, 0, (hwkeymask_len + 7)/8);

    ret = p4pd_hwkey_hwmask_build(tableid,
                                  swkey,
                                  swkey_mask,
                                  (uint8_t*)hwkey,
                                  (uint8_t*)hwkeymask);
    if (et != P4PD_SUCCESS) {
        delete[] hwkey;
        delete[] hwkeymask;
        return ret;
    }

    ret = p4pd_global_entry_write(tableid,
                                  index,
                                  (uint8_t*)hwkey,
                                  (uint8_t*)hwkeymask,
                                  actiondata);

    delete[] hwkey;
    delete[] hwkeymask;
    return ret;
#endif
    return 0;
}

#define MAX_RESP_SIZE_FOR_ONE_INDEX      512 /* Max table read response size for 1 index */ 
/* Key:128B, Key Mask: 128B, data 256B */
#define MAX_KEY_SZ 128
#define MAX_ACTION_SZ 256
#define MAX_RESP_SIZE  (MAX_RESP_SIZE_FOR_ONE_INDEX * 10) /* To tune based on the tables being used */

void *
allocate_debug_response_msg (void)
{
    return calloc(1, MAX_RESP_SIZE);
}

void
free_debug_response_msg (void *mem_rsp_msg)
{
    free(mem_rsp_msg);
}


/* read the table info */
p4pd_error_t
${api_prefix}_entry_read (uint32_t  tableid,
                          uint32_t  index,
                          void      *swkey,
                          void      *swkey_mask,
                          void      *actiondata,
                          void      *mem_rsp_msg,
                          int       *size)
{
    p4pd_error_t    pd_err;
    int             num_indices;

    if (index == 0xffffffff) {
        p4pd_table_properties_t tbl_ctx;

        pd_err = p4pd_global_table_properties_get(tableid, &tbl_ctx);
        if (pd_err != P4PD_SUCCESS) {
            return P4PD_FAIL;
        }

        index = *size; /* In case num indices is greater than MAX we can read in one request */
        num_indices = MAX(tbl_ctx.tabledepth - index, 10);
    } else {
        num_indices = 1;
    }

    for (int i = index; i < index + num_indices; ++i) {

        pd_err = p4pd_global_entry_read(tableid, index, swkey,
                                        swkey_mask, actiondata);
        if (pd_err != P4PD_SUCCESS) {
            return (P4PD_FAIL);
        }

        {
            char    buffer[2048];

            memset(buffer, 0, sizeof(buffer));
            p4pd_global_table_ds_decoded_string_get(tableid, index,
                                                    (void *)swkey,
                                                    (void *)swkey_mask,
                                                    (void *)actiondata,
                                                    buffer, sizeof(buffer));
            printf("%s\n", buffer);
        }
    }
    *size = num_indices;
    fflush(stdout);

    return P4PD_SUCCESS;
}

p4pd_error_t
${api_prefix}_entry_populate (uint32_t  tableid,
                              void      *swkey,
                              void      *swkey_mask,
                              void      *actiondata,
                              void      *mem_rsp_msg,
                              int       response_index)
{
    uint8_t    *ptr = (uint8_t *)mem_rsp_msg;

    if (ptr == NULL) {
        return P4PD_FAIL;
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
            //::            if pddict['tables'][table]['type'] == 'Index' or pddict['tables'][table]['type'] == 'Mpu':
            memcpy(actiondata, ptr + response_index * MAX_RESP_SIZE_FOR_ONE_INDEX + MAX_KEY_SZ + MAX_KEY_SZ,
                   sizeof(${table}_actiondata_t));
            break;

            //::            #endif
            //::            if pddict['tables'][table]['type'] == 'Hash' or pddict['tables'][table]['type'] == 'Hash_OTcam':
            memcpy(swkey, ptr + response_index * MAX_RESP_SIZE_FOR_ONE_INDEX, sizeof(${table}_swkey_t));

            memcpy(actiondata, ptr + response_index * MAX_RESP_SIZE_FOR_ONE_INDEX + MAX_KEY_SZ + MAX_KEY_SZ,
                   sizeof(${table}_actiondata_t));
            break;

            //::            #endif
            //::            if pddict['tables'][table]['type'] == 'Ternary':
            memcpy(swkey, ptr + response_index * MAX_RESP_SIZE_FOR_ONE_INDEX, sizeof(${table}_swkey_t));

            memcpy(swkey_mask, ptr + response_index * MAX_RESP_SIZE_FOR_ONE_INDEX + MAX_KEY_SZ, sizeof(${table}_swkey_mask_t));

            memcpy(actiondata, ptr + response_index * MAX_RESP_SIZE_FOR_ONE_INDEX + MAX_KEY_SZ + MAX_KEY_SZ,
                   sizeof(${table}_actiondata_t));
            break;
            //
            //::            #endif
            //::        #endfor
        default:
            // Invalid tableid
            return (P4PD_FAIL);
    }

    return (P4PD_SUCCESS);
}

void
${api_prefix}_register_entry_read (std::string block_name,
                                   std::string reg_name, std::string filename)
{
}

void
${api_prefix}_register_list (std::string block_name,
                             std::string reg_name, std::string filename)
{
}

#define RAW_MEM_READ_MAX        1024

//::    if pddict['p4plus']:

p4pd_error_t
${api_prefix}_raw_table_entry_read (uint32_t tableid,
                                    uint8_t     actionid,
                                    void        *actiondata,
                                    uint64_t    address)
{
    uint32_t        len = 0;
    p4pd_error_t    ret = P4PD_FAIL;
    uint8_t         mem[RAW_MEM_READ_MAX];

    ${api_prefix}_raw_table_hwentry_query(tableid, actionid,
            &len);
    assert((len % 8) == 0);

    len /= 8;

    ret = sdk::asic::asic_mem_read(address, mem, len);
    if (ret != SDK_RET_OK)
        return P4PD_FAIL;

    ${api_prefix}_entry_unpack(tableid, actionid,
                                mem, 0, actiondata);
    return P4PD_SUCCESS;
}

void
${api_prefix}_raw_table_entry_write (uint32_t tableid,
                                     uint8_t     actionid,
                                     void        *actiondata,
                                     uint16_t    actiondata_sz,
                                     uint64_t    address)
{
}
//::    #endif
