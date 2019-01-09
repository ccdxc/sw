//------------------------------------------------------------------------------
// {C} Copyright 2017 Pensando Systems Inc. All rights reserved
//------------------------------------------------------------------------------
#include "nic/include/base.hpp"
#include "lib/p4/p4_api.hpp"
#include "gen/p4gen/p4/include/p4pd.h"

#define P4PD_MEM_HASH_TABLE_MAX_ENTRIES 4*1024*1024
#define P4PD_MEM_HASH_KEY_SIZE sizeof(flow_hash_swkey_t)
#define P4PD_MEM_HASH_DATA_SIZE sizeof(flow_hash_flow_hash_info_t)

typedef struct p4pd_mem_hash_entry_s {
    flow_hash_swkey_t           key;
    flow_hash_actiondata_t      data;
} p4pd_mem_hash_entry_t;


typedef struct p4pd_mem_hash_table_s {
    p4pd_mem_hash_entry_t   entries[P4PD_MEM_HASH_TABLE_MAX_ENTRIES];
} p4pd_mem_hash_table_t;

static p4pd_mem_hash_table_t    mhtable[2];

typedef int p4pd_error_t;

int p4pd_entry_write(unsigned int tableid, unsigned int index, unsigned char *hwkey,
                                        unsigned char *hwkey_y, void *actiondata)
{
    assert(index < P4PD_MEM_HASH_TABLE_MAX_ENTRIES);
    memcpy(&mhtable[tableid-1].entries[index].key, hwkey, P4PD_MEM_HASH_KEY_SIZE);
    memcpy(&mhtable[tableid-1].entries[index].data, actiondata, P4PD_MEM_HASH_DATA_SIZE);
    return 0;
}

int p4pd_entry_read(uint32_t tableid, uint32_t index, void *swkey,
                                       void *swkey_mask, void *actiondata)
{
    assert(index < P4PD_MEM_HASH_TABLE_MAX_ENTRIES);
    memcpy(swkey, &mhtable[tableid-1].entries[index].key, P4PD_MEM_HASH_KEY_SIZE);
    memcpy(actiondata, &mhtable[tableid-1].entries[index].data, P4PD_MEM_HASH_DATA_SIZE);
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

int p4pd_global_entry_write_with_datamask(uint32_t tableid, uint32_t index, uint8_t *hwkey,
                                          uint8_t *hwkey_y, void *actiondata,
                                          void *actiondata_mask)
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

p4pd_error_t
p4pd_table_properties_get (uint32_t tableid, p4pd_table_properties_t *props)
{
    memset(props, 0, sizeof(p4pd_table_properties_t));

    props->key_struct_size = P4PD_MEM_HASH_KEY_SIZE;
    props->actiondata_struct_size = P4PD_MEM_HASH_DATA_SIZE;
    props->hash_type = 0;

    if (tableid == P4TBL_ID_FLOW_HASH) {
        props->tablename = (char *) "MemHashMainTable";
        props->tabledepth = 2*1024*1024;
        props->has_oflow_table = 1;
        props->oflow_table_id = P4TBL_ID_FLOW_HASH_OVERFLOW;
    } else {
        props->tablename = (char *) "MemHashHintTable";
        props->tabledepth = 256*1024;
    }

    return 0;
}

#if 0
int p4pd_global_table_ds_decoded_string_get(uint32_t   tableid,
                                            uint32_t   index,
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
#endif

int p4pd_global_table_properties_get(uint32_t tableid,
                                        void *tbl_ctx)
{
    return 0;
}

#if 0
// swkey => hwkey build API
int p4pd_hwkey_hwmask_build(uint32_t   tableid,
                            void       *swkey,
                            void       *swkey_mask,
                            uint8_t    *hw_key,
                            uint8_t    *hw_key_y)
{
    return 0;
}
#define p4pd_entry_write    p4pd_stub_entry_write
#define p4pd_entry_read     p4pd_stub_entry_read
#define p4pd_entry_create   p4pd_stub_entry_create


static inline int p4pd_stub_entry_write(uint32_t tableid, uint32_t index, uint8_t *hwkey,
                                        uint8_t *hwkey_y, void *actiondata)
{
    return 0;
}

static inline int p4pd_stub_entry_read(uint32_t tableid, uint32_t index, void *swkey,
                                       void *swkey_mask, void *actiondata)
{
    return 0;
}

#define p4pd_global_hwentry_query  p4pd_global_stub_hwentry_query
#define p4pd_global_entry_write    p4pd_global_stub_entry_write
#define p4pd_global_entry_read     p4pd_global_stub_entry_read
#define p4pd_global_entry_create   p4pd_global_stub_entry_create
#define p4pd_global_table_ds_decoded_string_get   p4pd_global_stub_table_ds_decoded_string_get
#define p4pd_global_entry_write_with_datamask     p4pd_global_stub_entry_write_with_datamask

static inline void p4pd_global_stub_hwentry_query(uint32_t tableid,
                                                  uint32_t *hwkey_len,
                                                  uint32_t *hwkeymask_len,
                                                  uint32_t *hwactiondata_len)
{
    return;
}

static inline int p4pd_global_stub_entry_write(uint32_t tableid, uint32_t index, uint8_t *hwkey,
                                        uint8_t *hwkey_y, void *actiondata)
{
    return 0;
}

static inline int p4pd_global_stub_entry_write_with_datamask(uint32_t tableid,
                                                             uint32_t index,
                                                             uint8_t *hwkey,
                                                             uint8_t *hwkey_y,
                                                             void *actiondata,
                                                             void *actiondata_mask)
{
    return 0;
}

static inline int p4pd_global_stub_entry_read(uint32_t tableid, uint32_t index, void *swkey,
                                       void *swkey_mask, void *actiondata)
{
    return 0;
}

static inline int p4pd_global_stub_entry_create(uint32_t tableid, void *swkey, void *swkey_mask,
                                         void *actiondata, uint8_t *hwentry)
{
    return 0;
}

static inline int p4pd_global_stub_table_ds_decoded_string_get(uint32_t   tableid,
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

static inline int p4pd_global_stub_table_properties_get(uint32_t tableid,
                                        void *tbl_ctx)
{
    return 0;
}
#endif

p4pd_error_t
p4pd_mem_hash_entry_set_key(uint32_t table_id,
                            uint8_t *swkey,
                            uint8_t *key)
{
    memcpy(swkey, key, sizeof(flow_hash_swkey_t));
    return 0;
}

p4pd_error_t
p4pd_mem_hash_entry_clear_key(uint32_t table_id,
                              uint8_t *swkey)
{
    memset(swkey, 0, sizeof(flow_hash_swkey_t));
    return 0;
}

p4pd_error_t
p4pd_mem_hash_entry_set_action_id(uint32_t table_id,
                                  uint8_t *swdata,
                                  uint32_t action_id)
{
    flow_hash_actiondata_t *acdata = (flow_hash_actiondata_t *)swdata;
    acdata->action_id = action_id;
    return 0;
}

p4pd_error_t
p4pd_mem_hash_entry_set_entry_valid(uint32_t table_id,
                                    uint8_t *swdata,
                                    uint8_t entry_valid)
{
    flow_hash_actiondata_t *acdata = (flow_hash_actiondata_t *)swdata;
    acdata->action_u.flow_hash_flow_hash_info.entry_valid = entry_valid;
    return 0;
}

p4pd_error_t
p4pd_mem_hash_entry_set_data(uint32_t table_id,
                             uint8_t *swdata,
                             uint8_t *data)
{
    flow_hash_actiondata_t *acdata = (flow_hash_actiondata_t *)swdata;
    flow_hash_actiondata_t *newdata = (flow_hash_actiondata_t *)data;
    acdata->action_u.flow_hash_flow_hash_info.export_en = newdata->action_u.flow_hash_flow_hash_info.export_en;
    acdata->action_u.flow_hash_flow_hash_info.flow_index = newdata->action_u.flow_hash_flow_hash_info.flow_index;
    return 0;
}

p4pd_error_t
p4pd_mem_hash_entry_clear_data(uint32_t table_id,
                               uint8_t *swdata)
{
    flow_hash_actiondata_t *acdata = (flow_hash_actiondata_t *)swdata;
    acdata->action_u.flow_hash_flow_hash_info.export_en = 0;
    acdata->action_u.flow_hash_flow_hash_info.flow_index = 0;
    return 0;
}

p4pd_error_t
p4pd_mem_hash_entry_set_hint(uint32_t table_id,
                             uint8_t *swdata,
                             uint32_t hint,
                             uint32_t slot)
{
    flow_hash_actiondata_t *acdata = (flow_hash_actiondata_t *)swdata;
    if (slot == 1) {
        acdata->action_u.flow_hash_flow_hash_info.hint1 = hint;
    } else if (slot == 2) {
        acdata->action_u.flow_hash_flow_hash_info.hint2 = hint;
    } else if (slot == 3) {
        acdata->action_u.flow_hash_flow_hash_info.hint3 = hint;
    } else if (slot == 4) {
        acdata->action_u.flow_hash_flow_hash_info.hint4 = hint;
    } else if (slot == 5) {
        acdata->action_u.flow_hash_flow_hash_info.hint5 = hint;
    } else {
        assert(0);
    }
    return 0;
}

p4pd_error_t
p4pd_mem_hash_entry_set_hash(uint32_t table_id,
                             uint8_t *swdata,
                             uint32_t hash,
                             uint32_t slot)
{
    flow_hash_actiondata_t *acdata = (flow_hash_actiondata_t *)swdata;
    if (slot == 1) {
        acdata->action_u.flow_hash_flow_hash_info.hash1 = hash;
    } else if (slot == 2) {
        acdata->action_u.flow_hash_flow_hash_info.hash2 = hash;
    } else if (slot == 3) {
        acdata->action_u.flow_hash_flow_hash_info.hash3 = hash;
    } else if (slot == 4) {
        acdata->action_u.flow_hash_flow_hash_info.hash4 = hash;
    } else if (slot == 5) {
        acdata->action_u.flow_hash_flow_hash_info.hash5 = hash;
    } else {
        assert(0);
    }
    return 0;
}

p4pd_error_t
p4pd_mem_hash_entry_set_more_hints(uint32_t table_id,
                                   uint8_t *swdata,
                                   uint8_t more_hints)
{
    flow_hash_actiondata_t *acdata = (flow_hash_actiondata_t *)swdata;
    acdata->action_u.flow_hash_flow_hash_info.more_hints = more_hints;
    return 0;
}

p4pd_error_t
p4pd_mem_hash_entry_set_more_hashs(uint32_t table_id,
                                   uint8_t *swdata,
                                   uint32_t more_hashs)
{
    flow_hash_actiondata_t *acdata = (flow_hash_actiondata_t *)swdata;
    acdata->action_u.flow_hash_flow_hash_info.more_hashs = more_hashs;
    return 0;
}

uint32_t
p4pd_mem_hash_entry_get_hash(uint32_t table_id,
                             uint8_t *swdata,
                             uint32_t slot)
{
    flow_hash_actiondata_t *acdata = (flow_hash_actiondata_t *)swdata;
    if (slot == 1) {
        return acdata->action_u.flow_hash_flow_hash_info.hash1;
    } else if (slot == 2) {
        return acdata->action_u.flow_hash_flow_hash_info.hash2;
    } else if (slot == 3) {
        return acdata->action_u.flow_hash_flow_hash_info.hash3;
    } else if (slot == 4) {
        return acdata->action_u.flow_hash_flow_hash_info.hash4;
    } else if (slot == 5) {
        return acdata->action_u.flow_hash_flow_hash_info.hash5;
    } else {
        assert(0);
    }

    return 0;
}

uint32_t
p4pd_mem_hash_entry_get_hint(uint32_t table_id,
                             uint8_t *swdata,
                             uint32_t slot)
{
    flow_hash_actiondata_t *acdata = (flow_hash_actiondata_t *)swdata;
    if (slot == 1) {
        return acdata->action_u.flow_hash_flow_hash_info.hint1;
    } else if (slot == 2) {
        return acdata->action_u.flow_hash_flow_hash_info.hint2;
    } else if (slot == 3) {
        return acdata->action_u.flow_hash_flow_hash_info.hint3;
    } else if (slot == 4) {
        return acdata->action_u.flow_hash_flow_hash_info.hint4;
    } else if (slot == 5) {
        return acdata->action_u.flow_hash_flow_hash_info.hint5;
    } else {
        assert(0);
    }

    return 0;
}

uint8_t
p4pd_mem_hash_entry_get_more_hashs(uint32_t table_id,
                                   uint8_t *swdata)
{
    flow_hash_actiondata_t *acdata = (flow_hash_actiondata_t *)swdata;
    return acdata->action_u.flow_hash_flow_hash_info.more_hashs;
}

uint32_t
p4pd_mem_hash_entry_get_more_hints(uint32_t table_id,
                                   uint8_t *swdata)
{
    flow_hash_actiondata_t *acdata = (flow_hash_actiondata_t *)swdata;
    return acdata->action_u.flow_hash_flow_hash_info.more_hints;
}

bool
p4pd_mem_hash_entry_compare_key(uint32_t table_id,
                                uint8_t *swdata,
                                uint8_t *data)
{
    return !memcmp(swdata, data, sizeof(flow_hash_swkey_t));
}

uint8_t *
p4pd_mem_hash_entry_key_str(uint32_t table_id, void *key)
{
    static char         str[512];
    flow_hash_swkey_t   *swkey = (flow_hash_swkey_t*)key;
    char                srcstr[INET6_ADDRSTRLEN];
    char                dststr[INET6_ADDRSTRLEN];

    inet_ntop(AF_INET6, swkey->flow_lkp_metadata_lkp_src,
              srcstr, INET6_ADDRSTRLEN);
    inet_ntop(AF_INET6, swkey->flow_lkp_metadata_lkp_dst,
              dststr, INET6_ADDRSTRLEN);

    sprintf(str, "Type:%d Inst:%d Dir:%d Src:%s Dst:%s"
            " Dport:%d Sport:%d Proto:%d Vrf:%d",
            swkey->flow_lkp_metadata_lkp_type,
            swkey->flow_lkp_metadata_lkp_inst,
            swkey->flow_lkp_metadata_lkp_dir,
            srcstr, dststr,
            swkey->flow_lkp_metadata_lkp_dport,
            swkey->flow_lkp_metadata_lkp_sport,
            swkey->flow_lkp_metadata_lkp_proto,
            swkey->flow_lkp_metadata_lkp_vrf);

    return (uint8_t *)str;
}

uint8_t *
p4pd_mem_hash_entry_swdata_str(uint32_t table_id, void *data)
{
    static char                 str[512];
    flow_hash_actiondata_t  *acdata = (flow_hash_actiondata_t*)data;

    sprintf(str, "ExportEn:%d FlowIndex:%d",
            acdata->action_u.flow_hash_flow_hash_info.export_en,
            acdata->action_u.flow_hash_flow_hash_info.flow_index);

    return (uint8_t *)str;
}

uint8_t *
p4pd_mem_hash_entry_data_str(uint32_t table_id, void *data)
{
    uint32_t                len = 0;
    static char             str[512];

    flow_hash_actiondata_t  *acdata = (flow_hash_actiondata_t*)data;

    len += sprintf(str, "Valid:%d ExportEn:%d FlowIndex:%d ",
                   acdata->action_u.flow_hash_flow_hash_info.entry_valid, 
                   acdata->action_u.flow_hash_flow_hash_info.export_en,
                   acdata->action_u.flow_hash_flow_hash_info.flow_index);

    if (acdata->action_u.flow_hash_flow_hash_info.hint1) {
        len += sprintf(str + len, "Hash1:%#x Hint1:%d ",
                       acdata->action_u.flow_hash_flow_hash_info.hash1,
                       acdata->action_u.flow_hash_flow_hash_info.hint1);
    }
    if (acdata->action_u.flow_hash_flow_hash_info.hint2) {
        len += sprintf(str + len, "Hash2:%#x Hint2:%d ",
                       acdata->action_u.flow_hash_flow_hash_info.hash2,
                       acdata->action_u.flow_hash_flow_hash_info.hint2);
    }
    if (acdata->action_u.flow_hash_flow_hash_info.hint3) {
        len += sprintf(str + len, "Hash3:%#x Hint3:%d ",
                       acdata->action_u.flow_hash_flow_hash_info.hash3,
                       acdata->action_u.flow_hash_flow_hash_info.hint3);
    }
    if (acdata->action_u.flow_hash_flow_hash_info.hint4) {
        len += sprintf(str + len, "Hash4:%#x Hint4:%d ",
                       acdata->action_u.flow_hash_flow_hash_info.hash4,
                       acdata->action_u.flow_hash_flow_hash_info.hint4);
    }
    if (acdata->action_u.flow_hash_flow_hash_info.hint5) {
        len += sprintf(str + len, "Hash5:%#x Hint5:%d ",
                       acdata->action_u.flow_hash_flow_hash_info.hash5,
                       acdata->action_u.flow_hash_flow_hash_info.hint5);
    }
    if (acdata->action_u.flow_hash_flow_hash_info.more_hints) {
        len += sprintf(str + len, "MoreHashs:%#x MoreHints:%d",
                      acdata->action_u.flow_hash_flow_hash_info.more_hashs,
                      acdata->action_u.flow_hash_flow_hash_info.more_hints);
    }

    return (uint8_t *)str;
}


uint8_t
p4pd_mem_hash_entry_get_num_hints(uint32_t table_id)
{
    return 5;
}
