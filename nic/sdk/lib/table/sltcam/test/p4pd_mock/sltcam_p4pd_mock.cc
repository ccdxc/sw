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
#include "sltcam_p4pd_mock.hpp"

typedef int p4pd_error_t;

static sltcam_mock_table_t mocktable;

int
sltcam_mock_init()
{
    memset(&mocktable, 0xFF, sizeof(mocktable));
    return 0;
}

uint32_t
sltcam_mock_get_valid_count (uint32_t table_id)
{
    uint32_t count = 0;
    for (uint32_t i = 0; i < SLTCAM_MOCK_TABLE_SIZE; i++) {
        if (mocktable.entries[i].key.entry_inactive == 0) {
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
    assert(index < SLTCAM_MOCK_TABLE_SIZE);
    mocktable.entries[index].key = *(sltcam_table_key_t *)hwkey;
    mocktable.entries[index].mask = *(sltcam_table_mask_t *)hwkey_y;
    mocktable.entries[index].data = *(sltcam_table_actiondata_t*)actiondata;
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
    assert(index < SLTCAM_MOCK_TABLE_SIZE);
    *(sltcam_table_key_t*)swkey = mocktable.entries[index].key;
    *(sltcam_table_mask_t *)swkey_mask = mocktable.entries[index].mask;
    *(sltcam_table_actiondata_t*)actiondata = mocktable.entries[index].data;
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
    props->tablename = (char *) "SltcamMockTable";
    props->key_struct_size = KEY_SIZE;
    props->actiondata_struct_size = DATA_SIZE;
    props->hash_type = 0;
    props->tabledepth = SLTCAM_MOCK_TABLE_SIZE;
    props->hbm_layout.entry_width = 64;
    return 0;
}
p4pd_error_t
p4pd_global_table_properties_get (uint32_t table_id, p4pd_table_properties_t *props)
{
    return p4pd_table_properties_get(table_id, props);
}

p4pd_error_t
p4pd_global_table_ds_decoded_string_get (uint32_t   tableid,
                                         uint32_t   index,
                                         void*      sw_key,
                                         /* Valid only in case of TCAM;
                                          * Otherwise can be NULL) */
                                         void*      sw_key_mask,
                                         void*      action_data,
                                         char*      buffer,
                                         uint16_t   buf_len) 
{
    sltcam_table_key_t *key = (sltcam_table_key_t*)sw_key;
    sltcam_table_mask_t *mask = (sltcam_table_mask_t*)sw_key_mask;
    sltcam_table_actiondata_t *acdata = (sltcam_table_actiondata_t*)action_data;
    memset(buffer, 0, buf_len);

    snprintf(buffer, buf_len, 
             "KEY = V:%#x,K:%#x\n"
             "MSK = V:%#x,K:%#x\n"
             "DAT = D:%#x\n",
             key->entry_inactive, key->k, mask->entry_inactive_mask, mask->m,
             acdata->action_u.info.d);

    return 0;
}

