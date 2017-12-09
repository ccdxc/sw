#pragma once
#include "nic/hal/src/session.hpp"
#include "nic/utils/list/list.hpp"
#include "nic/fte/fte_db.hpp"
#include "nic/hal/src/session.hpp"

namespace hal {
namespace plugins {
namespace alg_utils {

//-----------------------------------------------------------------------------
// L4 ALG status that is (1) kept per expected flow entry until the pinhole is 
// opened up. (2) kept per L4 HAL session after the session is created. This has 
// back pointer to app_session that it belongs to. 
//-----------------------------------------------------------------------------
typedef struct l4_alg_status {
    fte::expected_flow_t  entry;                     // Flow key and handler
    nwsec::ALGName        alg;                       // ALG applied on this L4-session
    session_t            *session;                   // Back pointer to L4-session
    void                 *info;                      // Per-ALG L4 session oper status
    app_session_t        *app_session;               // Back pointer to app session this L4 session is part of
    dllist_ctxt_t         l4_sess_lentry;            // L4 Session list context
    dllist_ctxt_t         exp_flow_lentry;           // Expected flow list context
} l4_alg_status_t;

//------------------------------------------------------------------------------
// app sessions are the L7 session placeholder that consists of bunch of L4 
// sessions(control/data). For example, ftp app session can consist of one control 
// L4 session and multiple data sessions.
//------------------------------------------------------------------------------
typedef struct app_session_s {
    hal_spinlock_t       slock;                     // lock to protect this structure
    void                *oper;                      // per-ALG app session operational status
    dllist_ctxt_t        l4_sess_lhead;             // List of all L4 control and data sessions for this app session
    dllist_ctxt_t        exp_flow_lhead;            // List of expected flows that this app session has created
    ht_ctxt_t            app_sess_ht_ctxt;          // App session hash table context
} app_session_t;

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
                             slab *l4_sess_slab, slab *alg_state_slab, 
                             app_sess_cleanup_hdlr_t app_sess_clnup_hdlr, 
                             l4_sess_cleanup_hdlr_t exp_flow_clnup_hdlr);

    void init(const char* feature_name, slab *app_sess_slab,
              slab *l4_sess_slab, slab *alg_state_slab,
              app_sess_cleanup_hdlr_t app_sess_clnup_hdlr,
              l4_sess_cleanup_hdlr_t exp_flow_clnup_hdlr);

    ~alg_state();

    void rlock(void) { rwlock_.rlock(); }
    void runlock(void) { rwlock_.runlock(); }
    void wlock(void) { rwlock_.wlock(); }
    void wunlock(void) { rwlock_.wunlock(); }

    const char *feature() const { return feature_; }
    slab *app_sess_slab() const { return app_sess_slab_; }
    slab *l4_sess_slab() const { return l4_sess_slab_; }
    slab *alg_state_slab() const { return alg_state_slab_; }
    ht *app_sess_ht(void) const { return app_sess_ht_; }
    void cleanup_app_session(app_session_t *app);
    hal_ret_t alloc_and_insert_exp_flow(app_session_t *app_sess, 
                                        l4_alg_status_t *alg_status);
    hal_ret_t alloc_and_insert_l4_sess(app_session_t *app_sess,
                                       l4_alg_status_t *alg_status); 
    hal_ret_t alloc_and_init_app_sess(app_session_t *app_sess);

private:
    wp_rwlock                      rwlock_;                   // Read-write lock to alg_state
    const char                    *feature_;                  // Feature name for this instance
    slab                          *app_sess_slab_;            // Slab to allocate memory from for app_sess
    slab                          *l4_sess_slab_;             // L4 Session slab to allocate memory from    
    slab                          *alg_state_slab_;           // Infra Slab to init ALG state
    ht                            *app_sess_ht_;              // App session hash table for the feature
    l4_sess_cleanup_hdlr_t         l4_sess_cleanup_hdlr_;     // Callback to cleanup any feature specific info 
    app_sess_cleanup_hdlr_t        app_sess_cleanup_hdlr_;    // Callback to cleanup any feature specific info
};

} // namespace alg_utils
} // namespace plugins
} // namespace hal
