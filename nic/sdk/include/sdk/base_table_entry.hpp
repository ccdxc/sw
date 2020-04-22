//-----------------------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------

#ifndef __BASE_TABLE_ENTRY_HPP__
#define __BASE_TABLE_ENTRY_HPP__

#include "include/sdk/base.hpp"

struct __attribute__((__packed__)) base_table_entry_t {
    uint64_t entry_valid : 1;

    virtual void copy_data(void *s) { SDK_ASSERT(0); }
    virtual uint8_t get_entry_valid (void) { SDK_ASSERT(0); return 0; }
    virtual void set_entry_valid (uint8_t _entry_valid) { SDK_ASSERT(0); }
    virtual void build_key(void *s) { SDK_ASSERT(0); }
    virtual base_table_entry_t *construct(uint32_t size = 0) { SDK_ASSERT(0); return NULL; }
    virtual void copy_key_data(void *s) { SDK_ASSERT(0); }
    virtual void clear_data(void) { SDK_ASSERT(0); }
    virtual void clear_key_data(void) { SDK_ASSERT(0); }
    virtual bool compare_key(void *s) { SDK_ASSERT(0); return false; }
    virtual bool compare_data(void *s) { SDK_ASSERT(0); return false; }
    virtual bool compare_key_data(void *s) { SDK_ASSERT(0); return false; }
    virtual void set_hint_hash(uint32_t slot, uint32_t hint, uint32_t hash) { SDK_ASSERT(0); }
    virtual void get_hint_hash(uint32_t slot, uint32_t &hint, uint16_t &hash) { SDK_ASSERT(0); }
    virtual void get_hint(uint32_t slot, uint32_t &hint) { SDK_ASSERT(0); }
    virtual uint32_t get_more_hint_slot(void) { SDK_ASSERT(0); return 0; }
    virtual bool is_hint_slot_valid(uint32_t slot) { SDK_ASSERT(0); return false; }
    virtual uint32_t find_last_hint(void) { SDK_ASSERT(0); return 0; }
    virtual int tostr(char *buff, uint32_t len) { SDK_ASSERT(0); return 0; }
    virtual void clear(void) { SDK_ASSERT(0); }
    virtual void set_epoch(uint8_t val) {}
    virtual uint32_t entry_size(void) { SDK_ASSERT(0); return 0; }
};

#endif    // __BASE_TABLE_ENTRY_HPP__
