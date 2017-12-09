#include "nic/include/fte_db.hpp"

namespace fte {

// ALG types
#define ALG_PROTO_STATE(ENTRY)                                         \
    ENTRY(ALG_PROTO_STATE_NONE,           0,  "ALG_PROTO_STATE_NONE")      \
    ENTRY(ALG_PROTO_STATE_TFTP_RRQ,       1,  "ALG_PROTO_STATE_TFTP_RRQ")  \
    ENTRY(ALG_PROTO_STATE_TFTP_WRQ,       2,  "ALG_PROTO_STATE_TFTP_WRQ")  \
    ENTRY(ALG_PROTO_STATE_FTP,            3,  "ALG_PROTO_STATE_FTP")       \
    ENTRY(ALG_PROTO_STATE_DNS,            4,  "ALG_PROTO_STATE_DNS")       \
    ENTRY(ALG_PROTO_STATE_SUNRPC_INIT,    5,  "ALG_PROTO_STATE_SURPRPC_INIT")  \
    ENTRY(ALG_PROTO_STATE_RPC_GETPORT,    6,  "ALG_PROTO_STATE_RPC_GETPORT") \
    ENTRY(ALG_PROTO_STATE_RPC_CALLIT,     7,  "ALG_PROTO_STATE_RPC_CALLIT") \
    ENTRY(ALG_PROTO_STATE_RPC_DUMP,       8,  "ALG_PROTO_STATE_RPC_DUMP") \
    ENTRY(ALG_PROTO_STATE_SUNRPC_DATA,    9,  "ALG_PROTO_STATE_SUNRPC_DATA") \
    ENTRY(ALG_PROTO_STATE_MSRPC_INIT,     10, "ALG_PROTO_STATE_MSRPC_INIT") \
    ENTRY(ALG_PROTO_STATE_MSRPC_BIND,     11, "ALG_PROTO_STATE_MSRPC_BIND") \
    ENTRY(ALG_PROTO_STATE_MSRPC_BOUND,    12, "ALG_PROTO_STATE_MSRPC_BOUND") \
    ENTRY(ALG_PROTO_STATE_MSRPC_EPM,      13, "ALG_PROTO_STATE_MSRPC_EPM") \
    ENTRY(ALG_PROTO_STATE_MSRPC_FRAG_REQ, 14, "ALG_PROTO_STATE_MSRPC_FRAG_REQ") \
    ENTRY(ALG_PROTO_STATE_MSRPC_DATA,     15, "ALG_PROTO_STATE_MSRPC_DATA") \

DEFINE_ENUM(alg_proto_state_t, ALG_PROTO_STATE)
#undef ALG_PROTO_STATE

// Address Family
#define ALG_ADDRESS_FAMILY(ENTRY)                                      \
    ENTRY(ALG_ADDRESS_FAMILY_IPV4,     0,  "ALG_ADDRESS_FAMILY_IPV4")  \
    ENTRY(ALG_ADDRESS_FAMILY_IPV6,     1,  "ALG_ADDRESS_FAMILY_IPV6")  \

DEFINE_ENUM(alg_addr_family_t, ALG_ADDRESS_FAMILY)
#undef ALG_ADDRESS_FAMILY

#define MAX_RPC          1000     // Revisit this

typedef struct map {
    alg_addr_family_t    addr_family;
    ipvx_addr_t          ip;
    uint32_t             prot;
    uint32_t             dport;
    uint32_t             vers;
    union {
        // SUN RPC Information
        struct {
            uint32_t             xid;
            uint32_t             prog_num;
        } __PACK__;
        // MS RPC Information
        struct {
            uint32_t             call_id;
            uint8_t              act_id[16];
            uint8_t              uuid[16];
        } __PACK__;
    } __PACK__;
} RPCMap;

typedef struct rpcinfo {
    uint8_t     rpc_frag_cont;
    uint8_t     msrpc_64bit;
    uint8_t     msrpc_ctxt_id[256];
    uint8_t     num_msrpc_ctxt;
    uint8_t     rpcvers;
    RPCMap      rpc_map;
} RPCInfo;

// Todo (Pavithra) -- make it thread safe class
typedef struct alg_entry_s {
    expected_flow_t         entry;           /* Flow key and handler */
    hal::session_t         *session;         /* Back pointer to control session */
    alg_proto_state_t       alg_proto_state;
    bool                    skip_sfw;
    union {   
        // ALG Protocol specific Information
        RPCInfo                 rpcinfo;
    } __PACK__;
} alg_entry_t;

std::ostream& operator<<(std::ostream& os, const alg_entry_t& val);

alg_entry_t *insert_alg_entry(alg_entry_t *entry);
alg_entry_t *alloc_and_init_alg_entry(fte::ctx_t& ctx);
void alg_completion_hdlr (fte::ctx_t& ctx, bool status);

}

namespace hal {
namespace net {

// Big-Endian util
inline uint64_t
__be_pack_uint64(const uint8_t *buf, uint32_t *idx)
{
    int shift = 56;
    uint64_t val = 0;

    do {
       val = val | (buf[(*idx)++]<<shift);
       shift -= 8;
    } while (shift >= 0);

    return val;
}

inline uint32_t
__be_pack_uint32(const uint8_t *buf, uint32_t *idx)
{
    int shift = 24;
    uint32_t val = 0;

    do {
       val = val | (buf[(*idx)++]<<shift);
       shift -= 8;
    } while (shift >= 0);

    return val;
}

inline uint16_t
__be_pack_uint16(const uint8_t *buf, uint32_t *idx)
{
    int shift = 8;
    uint32_t val = 0;

    do {
       val = val | (buf[(*idx)++]<<shift);
       shift -= 8;
    } while (shift >= 0);

    return val;
}

//Little Endian util
inline uint64_t
__le_pack_uint64(const uint8_t *buf, uint32_t *idx)
{
    int shift = 0;
    uint64_t val = 0;

    do {
       val = val | (buf[(*idx)++]<<shift);
       shift += 8;
    } while (shift <= 56);

    return val;
}

inline uint32_t
__le_pack_uint32(const uint8_t *buf, uint32_t *idx)
{
    int shift = 0;
    uint32_t val = 0;

    do {
       val = val | (buf[(*idx)++]<<shift);
       shift += 8;
    } while (shift <= 24);

    return val;
}

inline uint32_t
__pack_uint32(const uint8_t *buf, uint32_t *idx, uint8_t format=0)
{
    if (format == 1) {
        return (__le_pack_uint32(buf, idx));
    } else {
        return (__be_pack_uint32(buf, idx));
    }
}

inline uint16_t
__le_pack_uint16(const uint8_t *buf, uint32_t *idx)
{
    int shift = 0;
    uint32_t val = 0;

    do {
       val = val | (buf[(*idx)++]<<shift);
       shift += 8;
    } while (shift <= 8);

    return val;
}

inline uint16_t
__pack_uint16(const uint8_t *buf, uint32_t *idx, uint8_t format=0)
{
    if (format == 1) {
        return (__le_pack_uint16(buf, idx));
    } else {
        return (__be_pack_uint16(buf, idx));
    }
}

inline uint64_t
__pack_uint64(const uint8_t *buf, uint32_t *idx, uint8_t format=0)
{
    if (format == 1) {
        return (__le_pack_uint64(buf, idx));
    } else {
        return (__be_pack_uint64(buf, idx));
    }
}


void insert_rpc_entry(fte::ctx_t& ctx, fte::RPCMap *map, fte::alg_proto_state_t proto_state);
void cleanup_alg_entry(fte::ctx_t& ctx, hal::flow_key_t key);

}
}
