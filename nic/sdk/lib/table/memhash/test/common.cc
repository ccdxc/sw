//------------------------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//------------------------------------------------------------------------------
#include <arpa/inet.h>
#include <stdio.h>
#include <string.h>

#include "include/sdk/table.hpp"
#include "nic/sdk/lib/table/memhash/test/p4pd_mock/mem_hash_p4pd_mock.hpp"
#include "common.hpp"

h5_entry_t g_h5_cache[H5_MAX_ENTRIES];
uint32_t g_h5_cache_index = 0;

uint32_t
h5_gencrc32 (bool nextindex, bool nexthint)
{
    static uint32_t index = 1;
    static uint32_t hint = 1;
    static h5_crc32_t crc32;

    index = nextindex ? index + 1: index;
    hint = nexthint ? hint + 1: hint;

    crc32.hint = hint;
    crc32.index = index;

    return crc32.val;
}

void*
h5_genkey ()
{
    static mem_hash_h5_key_t key;
    static uint16_t sport = 1;
    static uint16_t dport = 1;
    static uint32_t src = 1;
    static uint32_t dst = 1;
    static uint8_t proto = 1;
    static uint16_t tag = 1;
    char srcstr[64];
    char dststr[64];

    key.tag = tag++;
    key.proto = proto++;
    key.sport = sport++;
    key.dport = dport++;
    src++;
    sprintf(srcstr, "2000::%04d:%04d", (src >> 16) & 0xFFFF, src & 0xFFFF);
    inet_pton(AF_INET6, srcstr, key.src);

    dst++;
    sprintf(dststr, "3000::%04d:%04d", (dst >> 16) & 0xFFFF, dst & 0xFFFF);
    inet_pton(AF_INET6, dststr, key.dst);
    
    key.type = 1;

    return (void *)&key;
}

char*
h5_key2str(void *key)
{
    static char         str[512];
    mem_hash_h5_key_t   *swkey = (mem_hash_h5_key_t*)key;
    char                srcstr[INET6_ADDRSTRLEN];
    char                dststr[INET6_ADDRSTRLEN];

    inet_ntop(AF_INET6, swkey->src, srcstr, INET6_ADDRSTRLEN);
    inet_ntop(AF_INET6, swkey->dst, dststr, INET6_ADDRSTRLEN);
    sprintf(str, "Type:%d Src:%s Dst:%s Dport:%d Sport:%d Proto:%d Tag:%d",
            swkey->type, srcstr, dststr, swkey->dport, swkey->sport, 
            swkey->proto, swkey->tag);
    return str;
}

void*
h5_gendata ()
{
    static uint32_t index = 1;
    static mem_hash_h5_appdata_t data;

    data.d1 = 1;
    data.d2 = index++;

    return (void *)&data;
}

char*
h5_appdata2str(void *data)
{
    static char str[512];
    mem_hash_h5_appdata_t *appdata = (mem_hash_h5_appdata_t*)data;
    sprintf(str, "D1:%d D2:%d", appdata->d1, appdata->d2);
    return str;
}

char*
h5_data2str(void *data)
{
    static char                 str[512];
    uint32_t                    len = 0;
    mem_hash_h5_actiondata_t    *acdata = (mem_hash_h5_actiondata_t*)data;

    len += sprintf(str, "Valid:%d D1:%d D2:%d",
                   acdata->action_u.info.entry_valid, 
                   acdata->action_u.info.d1,
                   acdata->action_u.info.d2);

    if (acdata->action_u.info.hint1) {
        len += sprintf(str + len, "Hash1:%#x Hint1:%d ",
                       acdata->action_u.info.hash1,
                       acdata->action_u.info.hint1);
    }
    if (acdata->action_u.info.hint2) {
        len += sprintf(str + len, "Hash2:%#x Hint2:%d ",
                       acdata->action_u.info.hash2,
                       acdata->action_u.info.hint2);
    }
    if (acdata->action_u.info.hint3) {
        len += sprintf(str + len, "Hash3:%#x Hint3:%d ",
                       acdata->action_u.info.hash3,
                       acdata->action_u.info.hint3);
    }
    if (acdata->action_u.info.hint4) {
        len += sprintf(str + len, "Hash4:%#x Hint4:%d ",
                       acdata->action_u.info.hash4,
                       acdata->action_u.info.hint4);
    }
    if (acdata->action_u.info.hint5) {
        len += sprintf(str + len, "Hash5:%#x Hint5:%d ",
                       acdata->action_u.info.hash5,
                       acdata->action_u.info.hint5);
    }
    if (acdata->action_u.info.more_hints) {
        len += sprintf(str + len, "MoreHashs:%#x MoreHints:%d",
                      acdata->action_u.info.more_hashs,
                      acdata->action_u.info.more_hints);
    }

    return str;
}

static h5_entry_t*
h5_alloc_cache_entry()
{
    return &g_h5_cache[g_h5_cache_index++];
}

h5_entry_t *
h5_alloc_entry()
{
    return (h5_entry_t *)malloc(sizeof(h5_entry_t));
}

void
h5_free_entry(h5_entry_t *entry)
{
    free(entry);
}

static void
fill_params (sdk_table_api_params_t *params,
             h5_entry_t *entry, bool with_handle)
{
    if (params) {
        params->key = &entry->key;
        params->appdata = &entry->appdata;
        params->hash_32b = entry->crc32.val;
        params->hash_valid = entry->crc32_valid;
        if (with_handle) {
            params->handle = entry->handle;
        }
    }
    return;
}

h5_entry_t*
h5_gen_cache_entry (h5_crc32_t *crc32,
                    sdk_table_api_params_t *params, 
                    bool gen_crc32)
{
    void *key = NULL;
    void *data = NULL;
    h5_entry_t *entry = h5_alloc_cache_entry();

    memset(entry, 0, sizeof(h5_entry_t));

    key = h5_genkey();
    memcpy(&entry->key, key, sizeof(entry->key));

    data = h5_gendata();
    memcpy(&entry->appdata, data, sizeof(entry->appdata));

    if (gen_crc32) {
        if (crc32) {
            entry->crc32 = *crc32;
        } else {
            entry->crc32.val = h5_gencrc32();
        }
        entry->crc32_valid = true;
    } else {
        entry->crc32.val = 0;
        entry->crc32_valid = false;
    }

    fill_params(params, entry, true);
 
    return entry;
}

uint32_t
h5_get_cache_count ()
{
    return g_h5_cache_index;
}

h5_entry_t *
h5_get_cache_entry (uint32_t index,
                    sdk_table_api_params_t *params,
                    bool with_handle)
{
    h5_entry_t *entry = &g_h5_cache[index];
    fill_params(params, entry, with_handle);
    return entry;
}

h5_entry_t *
h5_get_updated_cache_entry (uint32_t index, sdk_table_api_params_t *params,
                            bool with_handle)
{
    void *data = NULL;
    h5_entry_t *entry = h5_get_cache_entry(index, params, with_handle);
    data = h5_gendata();
    memcpy(&entry->appdata, data, sizeof(entry->appdata));
    fill_params(params, entry, with_handle);
    return entry;
}

void
h5_reset_cache()
{
    // zero out only the used entries.
    bzero(g_h5_cache, sizeof(h5_entry_t) * g_h5_cache_index);
    g_h5_cache_index = 0;
    return;
}
