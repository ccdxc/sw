#include "base.h"

int p4pd_entry_write(unsigned int tableid, unsigned int index, unsigned char *hwkey,
                                        unsigned char *hwkey_y, void *actiondata)
{
    return 0;
}

int p4pd_entry_read(uint32_t tableid, uint32_t index, void *swkey,
                                       void *swkey_mask, void *actiondata)
{
    return 0;
}
void p4pd_global_hwentry_query(uint32_t tableid,
                                                  uint32_t *hwkey_len,
                                                  uint32_t *hwkeymask_len,
                                                  uint32_t *hwactiondata_len)
{
    return;
}

int p4pd_global_entry_write(uint32_t tableid, uint32_t index, uint8_t *hwkey,
                                        uint8_t *hwkey_y, void *actiondata)
{
    return 0;
}

int p4pd_global_entry_read(uint32_t tableid, uint32_t index, void *swkey,
                                       void *swkey_mask, void *actiondata)
{
    return 0;
}

int p4pd_global_entry_create(uint32_t tableid, void *swkey, void *swkey_mask,
                                         void *actiondata, uint8_t *hwentry)
{
    return 0;
}

int p4pd_global_table_ds_decoded_string_get(uint32_t   tableid,
                                        void*      sw_key,
                                        /* Valid only in case of TCAM;
                                         * Otherwise can be NULL)
                                         */
                                        void*      sw_key_mask,
                                        void*      action_data,
                                        char*      buffer,
                                        uint16_t   buf_len)
{
    return 0;
}

int p4pd_global_table_properties_get(uint32_t tableid,
                                        void *tbl_ctx)
{
    return 0;
}
