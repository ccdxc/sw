//------------------------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//
// SDK types header file
//------------------------------------------------------------------------------
#ifndef __SDK_TABLE_HPP__
#define __SDK_TABLE_HPP__

#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string>

#define SDK_TABLE_BITS_TO_BYTES(_b) (((_b) >> 3) + (((_b) & 0x7) ? 1 : 0))
#define SDK_TABLE_ALIGN_TO_64B(_s) \
        (((_s)*8) <= 512) ? (_s) : (((_s)%64) ? ((_s)+(64-((_s)%64))) : (_s))

#define SDK_TABLE_HANDLE_INVALID    0
#define SDK_TABLE_MAX_RECIRC        8

using namespace std;

namespace sdk {
namespace table {

typedef enum health_s {
    HEALTH_GREEN,
    HEALTH_YELLOW,
    HEALTH_RED
} health_t;

#define SDK_TABLE_HANDLE_STR_LEN 64
struct handle_t {
public:
    static handle_t null() {
        static handle_t nullhdl;
        return nullhdl;
    }

    handle_t() { value_ = 0; }
    handle_t(uint64_t v) { value_ = v; }
    // Handle Valid or Not
    uint32_t valid() { return (pvalid_ || svalid_); }
    // Handle's Primary Index Valid or Not
    uint32_t pvalid() { return pvalid_; }
    // Handle's Secondary Index Valid or Not
    uint32_t svalid() { return svalid_; }
    // Handle's Primary Index (GET)
    uint32_t pindex() { return pindex_; }
    // Handle's Secondary Index (GET)
    uint32_t sindex() { return sindex_; }
    // Handle's Primary Index (SET)
    void pindex(uint32_t p) { pindex_ = p; pvalid_ = 1; }
    // Handle's Secondary Index (SET)
    void sindex(uint32_t s) { sindex_ = s; svalid_ = 1; }
    bool operator ==(const handle_t& h) { return value_ == h.value_; }
    bool operator !=(const handle_t& h) { return value_ != h.value_; }
    char *tostr(char *buff, uint32_t len) {
        assert(len >= SDK_TABLE_HANDLE_STR_LEN);
        snprintf(buff, len, "%d.%d.%d.%d", pvalid_, pindex_, svalid_, sindex_);
        return buff;
    }
    char *tostr() {
        static char hdlstr[SDK_TABLE_HANDLE_STR_LEN];
        sprintf(hdlstr, "%d.%d.%d.%d", pvalid_, pindex_, svalid_, sindex_);
        return hdlstr;
    }
    uint64_t tou64() { return value_; }
    void clear() { value_ = 0; }

private:
    union {
        struct {
            uint32_t pindex_ : 32;
            uint32_t sindex_ : 30;
            uint32_t pvalid_ : 1;
            uint32_t svalid_ : 1;
        };
        uint64_t value_;
    } __attribute__((__packed__));
};

#define SDK_TABLE_MAX_SW_KEY_LEN 64
#define SDK_TABLE_MAX_SW_DATA_LEN 96
#define SDK_TABLE_MAX_HW_KEY_LEN 64
#define SDK_TABLE_MAX_HW_DATA_LEN 64

#define SIZE_BITS_TO_BYTES(_sizebits) \
        (((_sizebits) >> 3) + ((_sizebits) & 0x7) ? 1 : 0)

// Forward declaration
typedef struct sdk_table_api_params_ sdk_table_api_params_t;
typedef char* (*bytes2str_t)(void *bytes);
typedef void (*iterate_t)(sdk_table_api_params_t *params);

typedef enum sdk_table_api_op_ {
    SDK_TABLE_API_NONE,
    SDK_TABLE_API_INSERT,
    SDK_TABLE_API_REMOVE,
    SDK_TABLE_API_UPDATE,
    SDK_TABLE_API_GET,
    SDK_TABLE_API_RESERVE,
    SDK_TABLE_API_RELEASE,
    SDK_TABLE_API_ITERATE,
    SDK_TABLE_API_CLEAR,
    SDK_TABLE_API_VALIDATE,
} sdk_table_api_op_t;

#define SDK_TABLE_API_OP_IS_CRUD(_op) \
        (((_op) == SDK_TABLE_API_INSERT) || ((_op) == SDK_TABLE_API_REMOVE) || \
         ((_op) == SDK_TABLE_API_UPDATE) || ((_op) == SDK_TABLE_API_GET) || \
         ((_op) == SDK_TABLE_API_RESERVE) || ((_op) == SDK_TABLE_API_RELEASE))

typedef enum sdk_table_health_state_s {
    SDK_TABLE_HEALTH_GREEN,
    SDK_TABLE_HEALTH_YELLOW,
    SDK_TABLE_HEALTH_RED
} sdk_table_health_state_t;

// Callback on every INSERT and DELETE of a table lib.
// Callback has to be implemented to set new state based on capacity & usage.
//typedef void (*table_health_monitor_func_t)(uint32_t table_id,
//                     char *name, sdk_table_health_state_t curr_state,
//                     uint32_t capacity, uint32_t usage,
//                     sdk_table_health_state_t *new_state);

typedef struct sdk_table_factory_params_ {
    // TableID of this table given by P4
    uint32_t table_id;
    // If this table uses hints to resolve collisions,
    // specific the number of hints used by this table.
    uint32_t num_hints;
    // If collision in this table is resolved using
    // recircs, then specify the maximum number of
    // recircs allowed.
    uint32_t max_recircs;
    // Convert key to string
    bytes2str_t key2str;
    // Convert mask to string
    bytes2str_t mask2str;
    // Convert data to string
    bytes2str_t appdata2str;
    // Enable entry tracing
    bool entry_trace_en;
    // Health state
    //table_health_state_t health_state_; // health state
    // Health monitoring callback
    //table_health_monitor_func_t health_monitor_func;
    uint32_t thread_id;
} sdk_table_factory_params_t;

typedef struct sdk_table_api_params_ {
    union {
        // [Input] Key of the entry
        void *key;
        // [Input] Entry (Key + Data)
        void *entry;
    };
    // [Input] Key mask of the entry
    void *mask;
    union {
        // [Input] Data of the entry
        void *appdata;
        // [Input] Action data directly
        void *actiondata;
    };
    // [Input] Action Data mask
    void *actiondata_mask;
    // [Input] ActionID of the entry
    uint8_t action_id;
    // [Input/Output] (Input is Optional)
    // valid only for HASH tables
    bool hash_valid;
    // [Input/Output] (Input is Optional)
    // Upto 32bits of hash value, valid only for HASH tables
    uint32_t hash_32b;
    // [Input/Output]
    // Handle of the entry, encoding differs for each table.
    handle_t handle;
    // [Input]
    // Iterator callback function
    iterate_t itercb;
    // [Input]
    // Callback data for table iteration
    void *cbdata;
    // [Input]
    // Set this to true to force reading h/w state without checking s/w state,
    // valid for HASH tables only.
    bool force_hwread;
    // [Input]
    // Set this to true to allocate from top of range for indices
    bool highest;
} sdk_table_api_params_t;

typedef struct sdk_table_api_stats_ {
    uint32_t insert;
    uint32_t insert_duplicate;
    uint32_t insert_fail;
    uint32_t insert_recirc_fail;
    uint32_t remove;
    uint32_t remove_not_found;
    uint32_t remove_fail;
    uint32_t update;
    uint32_t update_fail;
    uint32_t get;
    uint32_t get_fail;
    uint32_t reserve;
    uint32_t reserve_fail;
    uint32_t release;
    uint32_t release_fail;
} sdk_table_api_stats_t;

typedef struct sdk_table_stats_ {
    uint32_t entries;
    uint32_t collisions;
    uint32_t insert;
    uint32_t remove;
    uint32_t read;
    uint32_t write;
    uint32_t insert_lvl[SDK_TABLE_MAX_RECIRC];
    uint32_t remove_lvl[SDK_TABLE_MAX_RECIRC];
} sdk_table_stats_t;

typedef struct properties_ {
    std::string name;
    // Primary table id
    uint32_t ptable_id;
    // Primary table size
    uint32_t ptable_size;
    // Secondary table id
    uint32_t stable_id;
    // Secondary table size
    uint32_t stable_size;
    // Number of hints to the secondary table
    uint32_t num_hints;
    // Software key length
    uint32_t swkey_len;
    // Software data length
    uint32_t swdata_len;
    // Software appdata length
    uint32_t swappdata_len;
    // Hardware key length
    uint32_t hwkey_len;
    // Hardware data length
    uint32_t hwdata_len;
    // Maximum number of recircs allowed
    uint32_t max_recircs;
    // Hash polynomial used by this table
    uint32_t hash_poly;
    // Entry tracing enabled
    bool entry_trace_en;
    // Key to string method
    bytes2str_t key2str;
    // Data to string method
    bytes2str_t data2str;
    // Primary Table Base Physical Address
    uint64_t ptable_base_mem_pa;
    // Primary Base Virtual Address
    uint64_t ptable_base_mem_va;
    // Secondary Table Base Physical Address
    uint64_t stable_base_mem_pa;
    // Secondary Base Virtual Address
    uint64_t stable_base_mem_va;
} properties_t;


} // namespace table
} // namespace sdk
#endif // __SDK_TABLE_HPP__
