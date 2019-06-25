//-----------------------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------
#ifndef __FTLITE_BUCKET_HPP__
#define __FTLITE_BUCKET_HPP__

#include <stdint.h>
#include "include/sdk/base.hpp"
#include "ftlite_indexer.hpp"
#include "ftlite_utils.hpp"

using namespace sdk;

namespace ftlite {
namespace internal {

struct __attribute__ ((packed)) bucket_t {
public:
    uint8_t valid : 1;
    uint8_t hint1_valid : 1;
    uint8_t hint2_valid : 1;
    uint8_t hint3_valid : 1;
    uint8_t hint4_valid : 1;
    uint8_t more_hints_valid : 1;
    uint8_t spare : 2;

public:
    inline __attribute__((always_inline))
    sdk_ret_t validate(ipv4_entry_t *entry) {
        if (valid != entry->entry_valid) {
            SDK_TRACE_ERR("v4: bucket valid:%d entry valid:%d",
                          valid, entry->entry_valid);
            return SDK_RET_HW_SW_OO_SYNC;
        }
        return SDK_RET_OK;
    }

    inline __attribute__((always_inline))
    sdk_ret_t validate(ipv6_entry_t *entry) {
        if (valid != entry->entry_valid) {
            SDK_TRACE_ERR("v6: bucket valid:%d entry valid:%d",
                          valid, entry->entry_valid);
            return SDK_RET_HW_SW_OO_SYNC;
        }
        return SDK_RET_OK;
    }

    inline __attribute__((always_inline))
    sdk_ret_t validate(ipv6_entry_t *entry,
                       uint32_t slot, uint32_t index) {
        FTLITE_TRACE_DEBUG("ipv6 parent: index:%d "
               "bucket:%d entry:%d slot:%d hint1:%d hint2:%d "
               "hint3:%d hint4:%d more:%d", index,
               valid, entry->entry_valid, slot, hint1_valid, hint2_valid, 
               hint3_valid, hint4_valid, more_hints_valid);
        if (valid != entry->entry_valid ||
            (slot == 1 && hint1_valid) ||
            (slot == 2 && hint2_valid) ||
            (slot == 3 && hint3_valid) ||
            (slot == 4 && hint4_valid) ||
            (slot == 5 && more_hints_valid)) {
            FTLITE_TRACE_ERR("Error: OOSYNC");
            FTLITE_TRACE_ERR("[%s]", rawstr(entry, entry->size()));
            return SDK_RET_HW_SW_OO_SYNC;
        }
        return SDK_RET_OK;
    }

    inline __attribute__((always_inline))
    sdk_ret_t validate(ipv4_entry_t *entry,
                       uint32_t slot, uint32_t index) {
        FTLITE_TRACE_DEBUG("ipv4 parent: index:%d "
               "bucket:%d entry:%d slot:%d hint1:%d hint2:%d more:%d",
               index, valid, entry->entry_valid,
               slot, hint1_valid, hint2_valid, 
               more_hints_valid);
        if (valid != entry->entry_valid ||
            (slot == 1 && hint1_valid) ||
            (slot == 2 && hint2_valid) ||
            (slot == 5 && more_hints_valid)) {
            FTLITE_TRACE_ERR("Error: OOSYNC ");
            FTLITE_TRACE_ERR("[%s]", rawstr(entry, entry->size()));
            return SDK_RET_HW_SW_OO_SYNC;
        }
        return SDK_RET_OK;
    }
};

} // namespace internal
} // namespace ftlite

#endif
