//------------------------------------------------------------------------------
// {C} Copyright 2017 Pensando Systems Inc. All rights reserved
//
// p4pd mock for table management libraries' tests
//------------------------------------------------------------------------------
#include "sdk/base.hpp"

// namespace sdk {
// namespace pd {
// namespace p4 {

// read API
int p4pd_entry_read(uint32_t tableid, uint32_t index, void *swkey,
                    void *swkey_mask, void *actiondata)
{
    return 0;
}

// write API
int p4pd_entry_write(unsigned int tableid, unsigned int index, unsigned char *hwkey,
                     unsigned char *hwkey_y, void *actiondata)
{
    return 0;
}

// swkey => hwkey build API
int p4pd_hwkey_hwmask_build(uint32_t   tableid,
                            void       *swkey,
                            void       *swkey_mask,
                            uint8_t    *hw_key,
                            uint8_t    *hw_key_y)
{
    return 0;
}

// entry to string API
int p4pd_table_ds_decoded_string_get(uint32_t   tableid,
                                     uint32_t   index,
                                     void*      sw_key,
                                     void*      sw_key_mask,
                                     void*      action_data,
                                     char*      buffer,
                                     uint16_t   buf_len)
{
    return 0;
}

// hwentry get API
void p4pd_global_hwentry_query(uint32_t tableid,
                               uint32_t *hwkey_len,
                               uint32_t *hwkeymask_len,
                               uint32_t *hwactiondata_len)
{
    return;
}

// hwentry get API
void
p4pd_hwentry_query(uint32_t tableid,
                   uint32_t *hwkey_len,
                   uint32_t *hwkeymask_len,
                   uint32_t *hwactiondata_len)
{
    return;
}

// global write API
int p4pd_global_entry_write(uint32_t tableid, uint32_t index, uint8_t *hwkey,
                            uint8_t *hwkey_y, void *actiondata)
{
    return 0;
}

// global read API
int p4pd_global_entry_read(uint32_t tableid, uint32_t index, void *swkey,
                           void *swkey_mask, void *actiondata)
{
    return 0;
}

// global entry create API
int p4pd_global_entry_create(uint32_t tableid, void *swkey, void *swkey_mask,
                             void *actiondata, uint8_t *hwentry)
{
    return 0;
}

// global entry to string API
int p4pd_global_table_ds_decoded_string_get(uint32_t   tableid,
                                            uint32_t   index,
                                            void*      sw_key,
                                            void*      sw_key_mask,
                                            void*      action_data,
                                            char*      buffer,
                                            uint16_t   buf_len)
{
    return 0;
}

// global table properties get
int p4pd_global_table_properties_get(uint32_t tableid,
                                     void *tbl_ctx)
{
    return 0;
}

// }   // namespace p4
// }   // namespace pd
// }   // namespace sdk
