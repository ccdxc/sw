#include "nic/hal/plugins/alg_utils/core.hpp"
#include "nic/utils/list/list.hpp"
#include "nic/include/fte_db.hpp"

namespace hal {
namespace plugins {
namespace alg_utils {

void alg_state::init(const char* feature_name, slab *app_sess_slab,
                   slab *l4_sess_slab, slab *alg_state_slab,
                   app_sess_cleanup_hdlr_t app_sess_clnup_hdlr,
                   l4_sess_cleanup_hdlr_t exp_flow_clnup_hdlr) {
     feature_ = feature_name;
     app_sess_slab_ = app_sess_slab;
     l4_sess_slab_ = l4_sess_slab;
     alg_state_slab_ = alg_state_slab;
     app_sess_cleanup_hdlr_ = app_sess_clnup_hdlr;
     l4_sess_cleanup_hdlr_ = exp_flow_clnup_hdlr;

     return; 
}

alg_state_t *alg_state::factory(const char* feature_name, slab *app_sess_slab,
                              slab *l4_sess_slab, slab *alg_state_slab,
                              app_sess_cleanup_hdlr_t app_sess_clnup_hdlr,
                              l4_sess_cleanup_hdlr_t exp_flow_clnup_hdlr) {
    void         *mem = NULL;
    alg_state    *state = NULL;

    mem = alg_state_slab->alloc();
    HAL_ABORT(mem != NULL);
    state = new (mem) alg_state();
    
    state->init(feature_name, app_sess_slab, l4_sess_slab, 
               alg_state_slab, app_sess_clnup_hdlr, exp_flow_clnup_hdlr);

    return state;
}

static bool app_sess_walk_cb(void *entry, void *ctxt) {
     ((alg_state_t *)ctxt)->cleanup_app_session((app_session_t *)entry);
     return TRUE;
}

alg_state::~alg_state() {
     app_sess_ht()->walk_safe(app_sess_walk_cb, (void *)this);
}

hal_ret_t alg_state::alloc_and_insert_exp_flow(app_session_t *app_sess, 
                                               l4_alg_status_t *alg_status) {
    alg_status = (l4_alg_status_t *)l4_sess_slab()->alloc();
    if (alg_status == NULL) {
        return HAL_RET_OOM;
    }

    memset(alg_status, 0, sizeof(l4_alg_status_t));
    dllist_reset(&alg_status->exp_flow_lentry);
    dllist_add(&app_sess->exp_flow_lhead, &alg_status->exp_flow_lentry);

    return HAL_RET_OK; 
}

hal_ret_t alg_state::alloc_and_insert_l4_sess(app_session_t *app_sess,
                                              l4_alg_status_t *alg_status) {
    alg_status = (l4_alg_status_t *)l4_sess_slab()->alloc();
    if (alg_status == NULL) {
        return HAL_RET_OOM;
    }

    memset(alg_status, 0, sizeof(l4_alg_status_t));
    dllist_reset(&alg_status->l4_sess_lentry);
    dllist_add(&app_sess->l4_sess_lhead, &alg_status->l4_sess_lentry);

    return HAL_RET_OK;
}

hal_ret_t alg_state::alloc_and_init_app_sess(app_session_t *app_sess) {
    app_sess = (app_session_t *)app_sess_slab()->alloc();
    if (app_sess == NULL) {
        return HAL_RET_OOM;
    }
    memset(app_sess, 0, sizeof(app_session_t));

    app_sess_ht()->insert(app_sess, &app_sess->app_sess_ht_ctxt);

    return HAL_RET_OK;
}

void alg_state::cleanup_app_session(app_session_t *app_sess) {
    hal::utils::dllist_ctxt_t   *lentry, *next;

    // Cleanup any ALG specific info
    app_sess_cleanup_hdlr_(app_sess); 

    dllist_for_each_safe(lentry, next, &app_sess->exp_flow_lhead)
    {
        l4_alg_status_t *exp_flow = dllist_entry(lentry, 
                                  l4_alg_status_t, exp_flow_lentry);

        fte::remove_expected_flow(exp_flow->entry.key);
        hal::utils::dllist_del(&exp_flow->exp_flow_lentry);
        if (l4_sess_cleanup_hdlr_)
            l4_sess_cleanup_hdlr_(exp_flow);
        l4_sess_slab()->free(exp_flow);
    }

    dllist_for_each_safe(lentry, next, &app_sess->l4_sess_lhead)
    {
        l4_alg_status_t *l4_sess = dllist_entry(lentry,
                                   l4_alg_status_t, l4_sess_lentry);

        hal::utils::dllist_del(&l4_sess->l4_sess_lentry);
        if (l4_sess_cleanup_hdlr_)
            l4_sess_cleanup_hdlr_(l4_sess); 
        l4_sess_slab()->free(l4_sess);
    }

    // Callback to remove the app session from
    // hash context
    if (app_sess_cleanup_hdlr_)
        app_sess_cleanup_hdlr_(app_sess);

    app_sess_slab()->free(app_sess); 
}

} //namespace alg_utils
} //namespace plugins
} //namespace hal
