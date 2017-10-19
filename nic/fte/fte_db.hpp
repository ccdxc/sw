#pragma once

#include "nic/include/base.h"
#include "nic/utils/ht/ht.hpp"

namespace fte {

// ALG types
#define ALG_PROTO_STATE(ENTRY)                                         \
    ENTRY(ALG_PROTO_STATE_NONE,        0,  "ALG_PROTO_STATE_NONE")      \
    ENTRY(ALG_PROTO_STATE_TFTP_RRQ,    1,  "ALG_PROTO_STATE_TFTP_RRQ")  \
    ENTRY(ALG_PROTO_STATE_TFTP_WRQ,    2,  "ALG_PROTO_STATE_TFTP_WRQ")  \
    ENTRY(ALG_PROTO_STATE_FTP,         3,  "ALG_PROTO_STATE_FTP")       \
    ENTRY(ALG_PROTO_STATE_DNS,         4,  "ALG_PROTO_STATE_DNS")       \
    ENTRY(ALG_PROTO_STATE_RPC_INIT,    5,  "ALG_PROTO_STATE_RPC_INIT")  \
    ENTRY(ALG_PROTO_STATE_RPC_GETPORT, 6,  "ALG_PROTO_STATE_RPC_GETPORT") \
    ENTRY(ALG_PROTO_STATE_RPC_CALLIT,  7,  "ALG_PROTO_STATE_RPC_CALLIT") \

DEFINE_ENUM(alg_proto_state_t, ALG_PROTO_STATE)
#undef ALG_PROTO_STATE

#define FTE_MAX_ALG_KEYS 524288

typedef struct alg_entry_s {
    hal::flow_key_t         key;
    hal::session_t         *session;
    alg_proto_state_t       alg_proto_state;
    hal::flow_role_t        role;

    // meta data maintained for flow
    hal::utils::ht_ctxt_t   flow_key_ht_ctxt;  // Flow key based hash table
} alg_entry_t;

class fte_db {

public:
    static fte_db *factory(void);
    ~fte_db();

    hal_ret_t init(void);
    void rlock(void) { rwlock_.rlock(); }
    void runlock(void) { rwlock_.runlock(); }
    void wlock(void) { rwlock_.wlock(); }
    void wunlock(void) { rwlock_.wunlock(); }

    hal::utils::ht *alg_flow_key_ht(void) const { return alg_flow_key_ht_; }

private:
    struct {
        hal::utils::ht      *alg_flow_key_ht_;
    } __PACK__;

    wp_rwlock    rwlock_;
};

extern fte_db  *g_fte_db;

void *alg_flow_get_key_func(void *entry);
uint32_t alg_flow_compute_hash_func (void *key, uint32_t ht_size);
bool alg_flow_compare_key_func (void *key1, void *key2);

}
