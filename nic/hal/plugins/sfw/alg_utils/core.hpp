//-----------------------------------------------------------------------------
// {C} Copyright 2017 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------

#pragma once
#include "alg_db.hpp"
#include "nic/hal/plugins/cfg/nw/session.hpp"
#include "lib/list/list.hpp"
#include "nic/fte/fte.hpp"
#include "nic/hal/plugins/cfg/nw/session.hpp"
#include "nic/hal/plugins/sfw/alg_utils/tcp_buffer.hpp"
#include "nic/hal/iris/include/hal_state.hpp"

using session::EXPECTEDFlows;
using session::ACTIVESessions;

namespace hal {
namespace plugins {
namespace alg_utils {

#define IN6PTON_XDIGIT      0x00010000
#define IN6PTON_DIGIT       0x00020000
#define IN6PTON_COLON_MASK  0x00700000
#define IN6PTON_COLON_1     0x00100000  /* single : requested */
#define IN6PTON_COLON_2     0x00200000  /* second : requested */
#define IN6PTON_COLON_1_2   0x00400000  /* :: requested */
#define IN6PTON_DOT         0x00800000  /* . */
#define IN6PTON_DELIM       0x10000000
#define IN6PTON_NULL        0x20000000  /* first/tail */
#define IN6PTON_UNKNOWN     0x40000000
#define ALG_UTILS_MAX_APP_SESS 65535
#define DIR_IFLOW           0
#define DIR_RFLOW           1
#define DIR_MAX             2
#define ALG_EXP_FLOW_TIMER_ID          HAL_TIMER_ID_ALG_MIN
#define ALG_EXP_FLOW_GRACE_TIMER_ID    HAL_TIMER_ID_ALG_MIN + 1
#define DNS_ALG_SESS_TIMER_ID          HAL_TIMER_ID_ALG_MIN + 2

/*
 * Function Declarations
 */

int in4_pton(const char *src, int srclen, uint8_t *dst,
                 int delim, const char **end);

int in6_pton(const char *src, int srclen, uint8_t *dst,
                 int delim, const char **end);

/*
 * Inlines
 */

inline uint8_t
isNullip(ipvx_addr_t ip, uint8_t addr_family) {
    uint8_t null_ipv6[16];

    memset(null_ipv6, 0, sizeof(null_ipv6));
    if ((addr_family == IP_PROTO_IPV6 &&
        !memcmp(ip.v6_addr.addr8, null_ipv6, sizeof(null_ipv6))) ||
        (addr_family == IP_PROTO_IPV4 && !ip.v4_addr)) {
        return 1;
    }

    return 0;
}

// Big-Endian util
inline uint64_t __be_pack_uint64(const uint8_t *buf, uint32_t *idx) {
    int shift = 56;
    uint64_t val = 0;

    do {
       uint64_t tmpbuf = buf[(*idx)++];
       val = val | (tmpbuf<<shift);
       shift -= 8;
    } while (shift >= 0);

    return val;
}

inline uint32_t __be_pack_uint32(const uint8_t *buf, uint32_t *idx) {
    int shift = 24;
    uint32_t val = 0;

    do {
       val = val | (buf[(*idx)++]<<shift);
       shift -= 8;
    } while (shift >= 0);

    return val;
}

inline uint16_t __be_pack_uint16(const uint8_t *buf, uint32_t *idx) {
    int shift = 8;
    uint32_t val = 0;

    do {
       val = val | (buf[(*idx)++]<<shift);
       shift -= 8;
    } while (shift >= 0);

    return val;
}

//Little Endian util
inline uint64_t __le_pack_uint64(const uint8_t *buf, uint32_t *idx) {
    int shift = 0;
    uint64_t val = 0;

    do {
       uint64_t tmpbuf = buf[(*idx)++];
       val = val | (tmpbuf<<shift);
       shift += 8;
    } while (shift <= 56);

    return val;
}

inline uint32_t __le_pack_uint32(const uint8_t *buf, uint32_t *idx) {
    int shift = 0;
    uint32_t val = 0;

    do {
       val = val | (buf[(*idx)++]<<shift);
       shift += 8;
    } while (shift <= 24);

    return val;
}

inline uint32_t __pack_uint32(const uint8_t *buf, uint32_t *idx,
                                                uint8_t format=0) {
    if (format == 1) {
        return (__le_pack_uint32(buf, idx));
    } else {
        return (__be_pack_uint32(buf, idx));
    }
}

inline uint16_t __le_pack_uint16(const uint8_t *buf, uint32_t *idx) {
    int shift = 0;
    uint32_t val = 0;

    do {
       val = val | (buf[(*idx)++]<<shift);
       shift += 8;
    } while (shift <= 8);

    return val;
}

inline uint16_t __pack_uint16(const uint8_t *buf, uint32_t *idx,
                                                 uint8_t format=0) {
    if (format == 1) {
        return (__le_pack_uint16(buf, idx));
    } else {
        return (__be_pack_uint16(buf, idx));
    }
}

inline uint64_t __pack_uint64(const uint8_t *buf, uint32_t *idx,
                                                 uint8_t format=0) {
    if (format == 1) {
        return (__le_pack_uint64(buf, idx));
    } else {
        return (__be_pack_uint64(buf, idx));
    }
}

//------------------------------------------------------------------------------
// app sessions are the L7 session placeholder that consists of bunch of L4
// sessions(control/data). For example, ftp app session can consist of one control
// L4 session and multiple data sessions.
//------------------------------------------------------------------------------
typedef struct app_session_s {
    sdk_spinlock_t       slock;                     // lock to protect this structure
    hal::flow_key_t      key;                       // Key for app session hash table
    bool                 isCtrl;                    // Is this a control APP session (RTSP/SIP)
    void                *oper;                      // per-ALG app session operational status
    dllist_ctxt_t        l4_sess_lhead;             // List of all L4 control and data sessions for this app session
    dllist_ctxt_t        exp_flow_lhead;            // List of expected flows that this app session has created
    dllist_ctxt_t        app_sess_lentry;            // List of App sessions that this App session has created
    ht_ctxt_t            app_sess_ht_ctxt;          // App session hash table context
    app_session_s       *ctrl_app_sess;             // Pointer back to control app session 
} app_session_t;

//-----------------------------------------------------------------------------
// L4 ALG status that is (1) kept per expected flow entry until the pinhole is
// opened up. (2) kept per L4 HAL session after the session is created. This has
// back pointer to app_session that it belongs to.
//-----------------------------------------------------------------------------
typedef struct l4_alg_status {
    expected_flow_t                 entry;                     // Flow key and handler
    nwsec::ALGName                  alg;                       // ALG applied on this L4-session
    bool                            isCtrl;                    // Is this a control session
    hal_handle_t                    sess_hdl;                  // Back pointer to L4-session
    uint64_t                        rule_id;                   // Control session rule-id
    uint32_t                        idle_timeout;              // Timeout for the app
    void                           *info;                      // Per-ALG L4 session oper_status/info
    app_session_t                  *app_session;               // Back pointer to app session this L4 session is part of
    tcp_buffer_t                   *tcpbuf[DIR_MAX];           // pointer to TCP buffer for reassembly
    fte::feature_session_state_t    fte_feature_state;         // Feature session state to link this to actual L4 sess
    dllist_ctxt_t                   l4_sess_lentry;            // L4 Session list context
    dllist_ctxt_t                   exp_flow_lentry;           // Expected flow list context
} l4_alg_status_t;

inline l4_alg_status_t* alg_status(fte::feature_session_state_t *fte_feature_state) {
    return (l4_alg_status_t *)container_of(fte_feature_state, l4_alg_status_t, fte_feature_state);
}

#define expected_flow(timer_ptr)                             \
       ((expected_flow_t *)((char *)timer_ptr -              \
         offsetof(expected_flow_t, timer)))

typedef void (*l4_sess_cleanup_hdlr_t) (l4_alg_status_t *exp_flow);
typedef void (*app_sess_cleanup_hdlr_t) (app_session_t *app);

typedef class alg_state alg_state_t;

//------------------------------------------------------------------
//  Per-ALG state information
//  - Helps keep (1) hash table of app session information
//  (2) List of expected flows (3) APIs to clean up the app session
//  and expected flows
//-------------------------------------------------------------------
class alg_state {
public:
    static alg_state *factory(const char* feature_name, slab *app_sess_slab,
                              slab *l4_sess_slab, slab *alg_info_slab,
                              app_sess_cleanup_hdlr_t app_sess_clnup_hdlr,
                              l4_sess_cleanup_hdlr_t exp_flow_clnup_hdlr,
                              ht::ht_get_key_func_t ht_get_key_func = NULL,
                              uint32_t ht_key_size = 0);

    void init(const char* feature_name, slab *app_sess_slab,
              slab *l4_sess_slab, slab *alg_state_slab,
              app_sess_cleanup_hdlr_t app_sess_clnup_hdlr,
              l4_sess_cleanup_hdlr_t l4_sess_clnup_hdlr,
              ht::ht_get_key_func_t ht_get_key_func = NULL,
              uint32_t ht_key_size = 0);

    ~alg_state();

    void rlock(void) { rwlock_.rlock(); }
    void runlock(void) { rwlock_.runlock(); }
    void wlock(void) { rwlock_.wlock(); }
    void wunlock(void) { rwlock_.wunlock(); }

    const char *feature() const { return feature_; }
    slab *app_sess_slab() const { return app_sess_slab_; }
    slab *l4_sess_slab() const { return l4_sess_slab_; }
    slab *alg_info_slab() const { return alg_info_slab_; }
    ht *app_sess_ht(void) const { return app_sess_ht_; }
    void cleanup_exp_flow(l4_alg_status_t *exp_flow);
    void cleanup_l4_sess(l4_alg_status_t *l4_sess);
    void cleanup_app_session(app_session_t *app);
    hal_ret_t alloc_and_insert_exp_flow(app_session_t *app_sess,
                                        hal::flow_key_t key,
                                        l4_alg_status_t **alg_status,
                                        bool enable_timer=false,
                                        uint32_t time_intvl=0, bool find_existing=false);
    hal_ret_t alloc_and_insert_l4_sess(app_session_t *app_sess,
                                       l4_alg_status_t **alg_status);
    hal_ret_t alloc_and_init_app_sess(hal::flow_key_t key, app_session_t **app_sess);
    hal_ret_t insert_app_sess(app_session_t *app_sess, app_session_t *ctrl_app_sess=NULL);
    hal_ret_t lookup_app_sess(const void *key, app_session_t **app_sess);

    void move_expflow_to_l4sess(app_session_t *app_sess,
                                l4_alg_status_t *alg_status);
    l4_alg_status_t *get_next_expflow(app_session_t *app_sess);
    l4_alg_status_t *get_ctrl_l4sess(app_session_t *app_sess);
    void expected_flows_to_proto_buf(app_session_t *app_sess, EXPECTEDFlows *exp_flows);
    void active_data_sessions_to_proto_buf(app_session_t *app_sess, ACTIVESessions *active_sessions);

private:
    sdk::wp_rwlock                 rwlock_;                   // Read-write lock to alg_state
    const char                    *feature_;                  // Feature name for this instance
    slab                          *app_sess_slab_;            // Slab to allocate memory from for app_sess
    slab                          *l4_sess_slab_;             // L4 Session slab to allocate memory from
    slab                          *alg_info_slab_;            // per-ALG L4 sess oper status & info slab
    ht                            *app_sess_ht_;              // App session hash table for the feature
    l4_sess_cleanup_hdlr_t         l4_sess_cleanup_hdlr_;     // Callback to cleanup any feature specific info
    app_sess_cleanup_hdlr_t        app_sess_cleanup_hdlr_;    // Callback to cleanup any feature specific info

    static void exp_flow_timeout_cb (void *timer, uint32_t timer_id, void *ctxt);
};

//--------------------------------------------------------------------------------
// Expected flow timer context
//--------------------------------------------------------------------------------
typedef struct exp_flow_timer_cb_ {
    exp_flow_key_t   exp_flow_key;
    alg_state_t     *alg_state;
} exp_flow_timer_cb_t;

} // namespace alg_utils
} // namespace plugins
} // namespace hal
