//------------------------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//------------------------------------------------------------------------------
#include <assert.h>
#include <string.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>

#include "lib/p4/p4_api.hpp"
#include "slhash_p4pd_mock.hpp"

typedef int p4pd_error_t;

static slhash_mock_table_t mocktable;
static slhash_mock_table_t mocktable_otcam;

#define GET_TABLE(table_id) \
        (table_id == SLHASH_TABLE_ID_MOCK) ? &mocktable : &mocktable_otcam

int
slhash_mock_init()
{
    memset(&mocktable, 0, sizeof(mocktable));
    memset(&mocktable_otcam, 0, sizeof(mocktable_otcam));
    return 0;
}

uint32_t
slhash_mock_get_valid_count (uint32_t table_id)
{
    slhash_mock_table_t *tab = GET_TABLE(table_id);
    uint32_t count = 0;
    for (uint32_t i = 0; i < TABLE_SIZE; i++) {
        if (tab->entries[i].data.action_u.info.valid1 ||
            tab->entries[i].data.action_u.info.valid2) {
            count++;
        }
    }
    return count;
}

void
p4pd_hwentry_query(uint32_t tableid, uint32_t *hwkey_len,
                   uint32_t *hwkeymask_len, uint32_t *hwactiondata_len)
{
    *hwkey_len = 512;
    *hwactiondata_len = 192;
    return;
}

int 
p4pd_entry_write (unsigned int table_id, unsigned int index, unsigned char *hwkey,
                  unsigned char *hwkey_y, void *actiondata)
{
    slhash_mock_table_t *tab = GET_TABLE(table_id);
    assert(index < TABLE_SIZE);
    tab->entries[index].key = *(slhash_table_key_t *)hwkey;
    tab->entries[index].mask = *(slhash_table_mask_t *)hwkey_y;
    tab->entries[index].data = *(slhash_table_actiondata_t*)actiondata;
    return 0;
}

p4pd_error_t
p4pd_entry_install (uint32_t tableid, uint32_t index, void *swkey,
                    void *swkey_mask, void *actiondata)
{
    return p4pd_entry_write(tableid, index,
                            (unsigned char *)swkey,
                            (unsigned char *)swkey_mask, actiondata);
}

int 
p4pd_entry_read(uint32_t table_id, uint32_t index, void *swkey,
                void *swkey_mask, void *actiondata)
{
    slhash_mock_table_t *tab = GET_TABLE(table_id);
    *(slhash_table_key_t*)swkey = tab->entries[index].key;
    *(slhash_table_mask_t *)swkey_mask = tab->entries[index].mask;
    *(slhash_table_actiondata_t*)actiondata = tab->entries[index].data;
    return 0;
}

p4pd_error_t
p4pd_hwkey_hwmask_build(uint32_t tableid, void *swkey, void *swkey_mask,
                        uint8_t *hw_key, uint8_t *hw_key_y)
{
    memcpy(hw_key, swkey, KEY_SIZE);
    return 0;
}


p4pd_error_t
p4pd_table_properties_get (uint32_t table_id, p4pd_table_properties_t *props)
{
    memset(props, 0, sizeof(p4pd_table_properties_t));
    props->tablename = (char *) "SlhashMockTable";
    props->key_struct_size = KEY_SIZE;
    props->actiondata_struct_size = DATA_SIZE;
    props->hash_type = 0;
    props->tabledepth = TABLE_SIZE;
    props->hbm_layout.entry_width = 64;
    
    if (table_id == SLHASH_TABLE_ID_MOCK) {
        props->has_oflow_table = 1;
        props->oflow_table_id = SLHASH_TABLE_ID_OTCAM_MOCK;
    } else {
        props->tablename = (char *) "SlTcamMockTable";
    }

    return 0;
}

p4pd_error_t
p4pd_global_table_properties_get (uint32_t table_id, p4pd_table_properties_t *props)
{
    return p4pd_table_properties_get(table_id, props);
}

p4pd_error_t
p4pd_global_table_ds_decoded_string_get (uint32_t   table_id,
                                         uint32_t   index,
                                         void*      sw_key,
                                         /* Valid only in case of TCAM;
                                          * Otherwise can be NULL) */
                                         void*      sw_key_mask,
                                         void*      action_data,
                                         char*      buffer,
                                         uint16_t   buf_len) 
{
    slhash_table_key_t *key = (slhash_table_key_t*)sw_key;
    slhash_table_mask_t *mask = (slhash_table_mask_t*)sw_key_mask;
    slhash_table_actiondata_t *acdata = (slhash_table_actiondata_t*)action_data;
    slhash_table_info_t *info = (slhash_table_info_t*)(&acdata->action_u.info);

    memset(buffer, 0, buf_len);
    
    if (table_id == SLHASH_TABLE_ID_MOCK) {
        snprintf(buffer, buf_len, 
                 "KEY = k1:%#x,k2:%#x\n"
                 "DAT = d1:%#x,d2:%#x,epoch1:%d,epoch2:%d,v1:%d,v2:%d\n",
                 key->k1, key->k2, info->d1, info->d2, info->epoch1,
                 info->epoch2, info->valid1, info->valid2);
    } else {
        snprintf(buffer, buf_len, 
                 "KEY = k1:%#x,k2:%#x\n"
                 "MSK = k1:%#x,k2:%#x\n"
                 "DAT = d1:%#x,d2:%#x,epoch1:%d,epoch2:%d,v1:%d,v2:%d\n",
                 key->k1, key->k2, mask->k1, mask->k2,
                 info->d1, info->d2, info->epoch1,
                 info->epoch2, info->valid1, info->valid2);
    }

    return 0;
}

