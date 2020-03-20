//------------------------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
// stateless tcam table management library
//------------------------------------------------------------------------------
#include <string.h>

#include "lib/p4/p4_api.hpp"
#include "lib/utils/crc_fast.hpp"
#include "include/sdk/table.hpp"
#include "include/sdk/base.hpp"

#include "slhash_properties.hpp"
#include "slhash_api_context.hpp"
#include "slhash_utils.hpp"

namespace sdk {
namespace table {
namespace slhash_internal {

char*
rawstr(void *data, uint32_t len) {
    static char str[512];
    uint32_t i = 0;
    uint32_t slen = 0;
    for (i = 0; i < len; i++) {
        slen += sprintf(str+slen, "%02x", ((uint8_t*)data)[i]);
    }
    return str;
}

//----------------------------------------------------------------------------
// Calculate the CRC32 Hash
//----------------------------------------------------------------------------
sdk_ret_t
slhctx::calchash() {
    if (!params->hash_valid) {
        SLHASH_TRACE_DEBUG("Generating Hash: TableID=%d, KeyLength=%d, "
                           "DataLength=%d", props->table_id,
                           props->hwkey_len, props->hwdata_len);
        auto p4pdret = p4pd_global_hwkey_hwmask_build(props->table_id,
                                                      params->key, NULL,
                                                      hwkey, NULL);
        SDK_ASSERT(p4pdret == P4PD_SUCCESS);
        SLHASH_TRACE_DEBUG("HW Key:[%s]", rawstr(hwkey, props->hwkey_len));
        hash_32b = sdk::utils::crc32(hwkey, props->hwkey_len, props->hash_poly);
        hash_valid = true;
    } else {
        SLHASH_TRACE_VERBOSE("Using Hash32b value from input params");
        hash_32b = params->hash_32b;
        hash_valid = true;
    }

    index = hash_32b % props->table_size;
    index_valid = true;

    SLHASH_TRACE_DEBUG("Hash:%#x Index:%#x", hash_32b, index);
    return sdk::SDK_RET_OK;
}

//----------------------------------------------------------------------------
// Read entry from HW
//----------------------------------------------------------------------------
sdk_ret_t
slhctx::read() {
    // index must be valid by now
    SDK_ASSERT(index_valid);

    SLHASH_TRACE_DEBUG("index:%d", index);
    auto p4pdret =  p4pd_global_entry_read(props->table_id, index,
                                           swkey, swkeymask, swdata);
    print_sw();
    return p4pdret == P4PD_SUCCESS ? sdk::SDK_RET_OK : sdk::SDK_RET_ERR;
}

void
slhctx::clear_() {
    // Clear HW and SW fields
    memset(hwkey, 0, SDK_TABLE_MAX_HW_KEY_LEN);
    memset(swkey, 0, SDK_TABLE_MAX_SW_KEY_LEN);
    memset(swkeymask, 0, SDK_TABLE_MAX_SW_KEY_LEN);
    memset(swdata, 0, SDK_TABLE_MAX_SW_DATA_LEN);
    return;
}

void
slhctx::copyin_() {
    memcpy(swkey, params->key, props->swkey_len);
    memset(swkeymask, ~0, props->swkey_len);
    memcpy(swdata, params->appdata, props->swdata_len);
    return;
}

//----------------------------------------------------------------------------
// Write entry to HW
//----------------------------------------------------------------------------
sdk_ret_t
slhctx::write() {
    static char buff[4096] = {0};

    if (op == sdk::table::SDK_TABLE_API_INSERT ||
        op == sdk::table::SDK_TABLE_API_UPDATE) {
        // Copy input params
        copyin_();
    } else if (op == SDK_TABLE_API_REMOVE) {
        // Clear all contents for deletion
        clear_();
    } else {
        assert(0);
    }

    if (props->entry_trace_en) {
        p4pd_global_table_ds_decoded_string_get(props->table_id, index,
                                                swkey, swkeymask, swdata,
                                                buff, sizeof(buff));
        SLHASH_TRACE_DEBUG("Table: %s, EntryIndex:%u\n%s",
                           props->name, index, buff);
    }
    SLHASH_TRACE_DEBUG("Table: %s, EntryIndex:%u",
                       props->name, index);
    print_hw();

    auto p4pdret = p4pd_global_entry_install(props->table_id, index,
                                             swkey, swkeymask, swdata);
    return p4pdret == P4PD_SUCCESS ? sdk::SDK_RET_OK : sdk::SDK_RET_ERR;
}

sdk::table::handle_t
slhctx::inhandle() {
    return params ? params->handle : sdk::table::handle_t::null();
}

sdk::table::handle_t
slhctx::outhandle() {
    if (inhandle().valid()) {
        return inhandle();
    }

    ohandle.clear();
    ohandle.pindex(index);
    if (tcam_params_valid) {
        SDK_ASSERT(tcam_params.handle.pvalid());
        ohandle.sindex(tcam_params.handle.pindex());
    }
    return ohandle;
}

int
slhctx::keycompare() {
    if (inhandle().valid()) {
        return 0;
    }
    return memcmp(params->key, swkey, props->swkey_len);
}

void
slhctx::copyout() {
    if (!outhandle().svalid()) {
        // Copy out the 'sw' fields only if the entry is in hash table.
        // For TCAM entries, the tcam.get() api itself would have copied
        // the data.
        if (params->key) {
            memcpy(params->key, swkey, props->swkey_len);
        }
        if (params->mask) {
            memcpy(params->mask, swkeymask, props->swkey_len);
        }
        if (params->appdata) {
            memcpy(params->appdata, swdata, props->swdata_len);
        }
    }
    return;
}

static inline void
printbytes(const char *name, bytes2str_t b2s, void *b, uint32_t len) {
    if (b) {
        SLHASH_TRACE_VERBOSE("- %s:[%s]", name, b2s ? b2s(b) : rawstr(b, len));
    }
}

void
slhctx::print_sw() {
    SLHASH_TRACE_VERBOSE("SW Fields");
    printbytes("Key", props->key2str, swkey, props->swkey_len);
    printbytes("Mask", props->key2str, swkeymask, props->swkey_len);
    printbytes("Data", props->data2str, swdata, props->swdata_len);
}

void
slhctx::print_hw() {
    SLHASH_TRACE_VERBOSE("HW Fields");
    printbytes("Key", props->key2str, hwkey, props->hwkey_len);
}

void
slhctx::print_params() {
    if (params) {
        SLHASH_TRACE_DEBUG("Input Params");
        printbytes("Key", props->key2str, params->key, props->swkey_len);
        printbytes("Mask", props->key2str, params->mask, props->swkey_len);
        printbytes("Data", props->data2str, params->appdata, props->swdata_len);
        SLHASH_TRACE_DEBUG("Handle: %s", params->handle.tostr());
        if (params->hash_valid) {
            SLHASH_TRACE_DEBUG("Hash32b:%#x", params->hash_32b);
        }
    }
}

sdk_ret_t
slhctx::init(sdk_table_api_op_t op,
             sdk::table::sdk_table_api_params_t *params,
             sdk::table::slhash_internal::properties *props) {

    this->params = params;
    this->props = props;
    this->op = op;
    this->hash_32b = 0;
    this->hash_valid = false;

    this->tcam_params_valid = false;
    if (params) {
        this->tcam_params = *params;
    } else {
        memset(&this->tcam_params, 0, sizeof(this->tcam_params));
    }

    this->index = inhandle().pindex();
    this->index_valid = inhandle().pvalid();

    if (inhandle().svalid()) {
        this->tcam_params.handle.pindex(inhandle().sindex());
    }

    this->ohandle.clear();

    clear_();
    print_params();
    return sdk::SDK_RET_OK;
}

} // namespace slhash_internal
} // namespace table
} // namespace sdk
