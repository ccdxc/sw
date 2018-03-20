#include "alg_db.hpp"
#include "nic/include/hal_mem.hpp"
#include "nic/hal/plugins/alg_utils/core.hpp"
#include "sdk/list.hpp"

namespace hal {
namespace plugins {
namespace alg_utils {

#define ALG_EXP_FLOW_TIMER_ID          0xFF
#define ALG_EXP_FLOW_GRACE_TIMER_ID    0x100
#define ALG_EXP_FLOW_GRACE_TIME_INTVL  (15 * 1000) // 15 secs

/*
 * digit2bin
 */
static inline int xdigit2bin(char c, int delim)
{
    if (c == delim || c == '\0')
        return IN6PTON_DELIM;
    if (c == ':')
        return IN6PTON_COLON_MASK;
    if (c == '.')
        return IN6PTON_DOT;
    if (c >= '0' && c <= '9')
        return (IN6PTON_XDIGIT | IN6PTON_DIGIT| (c - '0'));
    if (c >= 'a' && c <= 'f')
        return (IN6PTON_XDIGIT | (c - 'a' + 10));
    if (c >= 'A' && c <= 'F')
        return (IN6PTON_XDIGIT | (c - 'A' + 10));
    if (delim == -1)
        return IN6PTON_DELIM;
    return IN6PTON_UNKNOWN;
}

/*
 * in4_pton
 *
 * Convert IPv4 printable address to binary form
 */
int in4_pton(const char *src, int srclen, uint8_t *dst,
             int delim, const char **end)
{
    const char *s;
    uint8_t *d;
    uint8_t dbuf[4];
    int ret = 0;
    int i;
    int w = 0;

    if (srclen < 0)
        srclen = strlen(src);
    s = src;
    d = dbuf;
    i = 0;
    while(1) {
        int c;
        c = xdigit2bin(srclen > 0 ? *s : '\0', delim);
        if (!(c & (IN6PTON_DIGIT | IN6PTON_DOT |
                   IN6PTON_DELIM | IN6PTON_COLON_MASK))) {
            goto out;
        }
        if (c & (IN6PTON_DOT | IN6PTON_DELIM | IN6PTON_COLON_MASK)) {
            if (w == 0)
                goto out;
            *d++ = w & 0xff;
            w = 0;
            i++;
            if (c & (IN6PTON_DELIM | IN6PTON_COLON_MASK)) {
                if (i != 4)
                    goto out;
                break;
            }
            goto cont;
        }
        w = (w * 10) + c;
        if ((w & 0xffff) > 255) {
            goto out;
        }
cont:
        if (i >= 4)
            goto out;
        s++;
        srclen--;
    }
    ret = 1;
    memcpy(dst, dbuf, sizeof(dbuf));
out:
    if (end)
        *end = s;
    return ret;
}

/*
 * in6_pton
 *
 * Convert IPv6 printable address to binary form
 */
int in6_pton(const char *src, int srclen, uint8_t *dst,
             int delim, const char **end)
{
    const char *s, *tok = NULL;
    uint8_t *d, *dc = NULL;
    uint8_t dbuf[16];
    int ret = 0;
    int i;
    int state = IN6PTON_COLON_1_2 | IN6PTON_XDIGIT | IN6PTON_NULL;
    int w = 0;

    memset(dbuf, 0, sizeof(dbuf));

    s = src;
    d = dbuf;
    if (srclen < 0)
        srclen = strlen(src);

    while (1) {
        int c;

        c = xdigit2bin(srclen > 0 ? *s : '\0', delim);
        if (!(c & state))
            goto out;
        if (c & (IN6PTON_DELIM | IN6PTON_COLON_MASK)) {
            /* process one 16-bit word */
            if (!(state & IN6PTON_NULL)) {
                *d++ = (w >> 8) & 0xff;
                *d++ = w & 0xff;
            }
            w = 0;
            if (c & IN6PTON_DELIM) {
                /* We've processed last word */
                break;
            }
            /*
             * COLON_1 => XDIGIT
             * COLON_2 => XDIGIT|DELIM
             * COLON_1_2 => COLON_2
             */
            switch (state & IN6PTON_COLON_MASK) {
            case IN6PTON_COLON_2:
                dc = d;
                state = IN6PTON_XDIGIT | IN6PTON_DELIM;
                if ((dc - dbuf) >= (int)sizeof(dbuf))
                    state |= IN6PTON_NULL;
                break;
            case IN6PTON_COLON_1|IN6PTON_COLON_1_2:
                state = IN6PTON_XDIGIT | IN6PTON_COLON_2;
                break;
            case IN6PTON_COLON_1:
                state = IN6PTON_XDIGIT;
                break;
            case IN6PTON_COLON_1_2:
                state = IN6PTON_COLON_2;
                break;
            default:
                state = 0;
            }
            tok = s + 1;
            goto cont;
        }

        if (c & IN6PTON_DOT) {
            ret = in4_pton(tok ? tok : s, srclen + (int)(s - tok),
                           d, delim, &s);
            if (ret > 0) {
                d += 4;
                break;
            }
            goto out;
        }

        w = (w << 4) | (0xff & c);
        state = IN6PTON_COLON_1 | IN6PTON_DELIM;
        if (!(w & 0xf000)) {
            state |= IN6PTON_XDIGIT;
        }
        if (!dc && d + 2 < dbuf + sizeof(dbuf)) {
            state |= IN6PTON_COLON_1_2;
            state &= ~IN6PTON_DELIM;
        }
        if (d + 2 >= dbuf + sizeof(dbuf)) {
            state &= ~(IN6PTON_COLON_1|IN6PTON_COLON_1_2);
        }
cont:
        if ((dc && d + 4 < dbuf + sizeof(dbuf)) ||
            d + 4 == dbuf + sizeof(dbuf)) {
            state |= IN6PTON_DOT;
        }
        if (d >= dbuf + sizeof(dbuf)) {
            state &= ~(IN6PTON_XDIGIT|IN6PTON_COLON_MASK);
        }
        s++;
        srclen--;
    }

    i = 15; d--;

    if (dc) {
        while(d >= dc)
            dst[i--] = *d--;
        while(i >= dc - dbuf)
            dst[i--] = 0;
        while(i >= 0)
            dst[i--] = *d--;
    } else
        memcpy(dst, dbuf, sizeof(dbuf));

    ret = 1;
out:
    if (end)
        *end = s;
    return ret;
}


static void * app_sess_get_key_func(void *entry)
{
    HAL_ASSERT(entry != NULL);
    return (void *)&(((app_session_t *)entry)->key);
}

static uint32_t app_sess_compute_hash_func(void *key, uint32_t ht_size)
{
    return (sdk::lib::hash_algo::fnv_hash(key,
                                          sizeof(hal::flow_key_t)) % ht_size);
}

static bool app_sess_compare_key_func (void *key1, void *key2)
{
    HAL_ASSERT((key1 != NULL) && (key2 != NULL));
    return (memcmp(key1, key2, sizeof(hal::flow_key_t)) == 0);
}

void alg_state::init(const char* feature_name, slab *app_sess_slab,
                   slab *l4_sess_slab, slab *alg_info_slab,
                   app_sess_cleanup_hdlr_t app_sess_clnup_hdlr,
                   l4_sess_cleanup_hdlr_t l4_sess_clnup_hdlr) {
     feature_ = feature_name;
     app_sess_slab_ = app_sess_slab;
     l4_sess_slab_ = l4_sess_slab;
     alg_info_slab_ = alg_info_slab;
     app_sess_cleanup_hdlr_ = app_sess_clnup_hdlr;
     l4_sess_cleanup_hdlr_ = l4_sess_clnup_hdlr;

     app_sess_ht_ = sdk::lib::ht::factory(ALG_UTILS_MAX_APP_SESS,
                                           app_sess_get_key_func,
                                           app_sess_compute_hash_func,
                                           app_sess_compare_key_func);

     return; 
}

alg_state_t *alg_state::factory(const char* feature_name, slab *app_sess_slab,
                              slab *l4_sess_slab, slab *alg_info_slab,
                              app_sess_cleanup_hdlr_t app_sess_clnup_hdlr,
                              l4_sess_cleanup_hdlr_t l4_sess_clnup_hdlr) {
    void         *mem = NULL;
    alg_state    *state = NULL;

    mem = (alg_state_t *)HAL_CALLOC(hal::HAL_MEM_ALLOC_ALG,
                                       sizeof(alg_state_t));
    HAL_ABORT(mem != NULL);
    state = new (mem) alg_state();
    
    state->init(feature_name, app_sess_slab, l4_sess_slab, 
               alg_info_slab, app_sess_clnup_hdlr, l4_sess_clnup_hdlr);

    return state;
}

static bool app_sess_walk_cb(void *entry, void *ctxt) {
     ((alg_state_t *)ctxt)->cleanup_app_session((app_session_t *)entry);
     return TRUE;
}

alg_state::~alg_state() {
     app_sess_ht()->walk_safe(app_sess_walk_cb, (void *)this);
}

void alg_state::exp_flow_timeout_cb (void *timer, uint32_t timer_id, void *ctxt) {
    exp_flow_timer_cb_t  *timer_ctxt = (exp_flow_timer_cb_t *)ctxt;
    l4_alg_status_t      *exp_flow = timer_ctxt->exp_flow;
    alg_state_t          *alg_state = timer_ctxt->alg_state;

    // Grace timer expiry. Is it ok to cleanup now ?
    // Should we have retries ?
    if (exp_flow->entry.deleting == true) {
        goto cleanup;
    }

    exp_flow->entry.deleting = true;
    if (ref_is_shared(&exp_flow->entry.ref_count)) {
        // Start a grace timer
        start_expected_flow_timer(&exp_flow->entry, ALG_EXP_FLOW_GRACE_TIMER_ID,
                                  ALG_EXP_FLOW_GRACE_TIME_INTVL, 
                                  exp_flow_timeout_cb, (void *)timer_ctxt);
        return;
    }

cleanup:
    HAL_TRACE_DEBUG("Cleaning up expected flow with key: {}", exp_flow->entry.key);
    alg_state->cleanup_exp_flow(exp_flow); 

    HAL_FREE(hal::HAL_MEM_ALLOC_ALG, timer_ctxt);
}

hal_ret_t alg_state::alloc_and_insert_exp_flow(app_session_t *app_sess,
                      hal::flow_key_t key, l4_alg_status_t **expected_flow,
                      bool enable_timer, uint32_t time_intvl) {
    exp_flow_timer_cb_t   *timer_ctxt = NULL;
    l4_alg_status_t       *exp_flow = NULL;

    exp_flow = (l4_alg_status_t *)l4_sess_slab()->alloc();
    if (exp_flow == NULL) {
        return HAL_RET_OOM;
    }

    HAL_SPINLOCK_LOCK(&app_sess->slock);
    exp_flow->app_session = app_sess;
    memcpy(&exp_flow->entry.key, &key, sizeof(hal::flow_key_t));
    insert_expected_flow(&exp_flow->entry);
    dllist_reset(&exp_flow->exp_flow_lentry);
    dllist_add(&app_sess->exp_flow_lhead, &exp_flow->exp_flow_lentry);
    HAL_SPINLOCK_UNLOCK(&app_sess->slock);

    if (enable_timer == true) {
        HAL_TRACE_DEBUG("Starting timer for expected flow with key: {}", key); 
        timer_ctxt = (exp_flow_timer_cb_t *)HAL_CALLOC(hal::HAL_MEM_ALLOC_ALG,
                                       sizeof(exp_flow_timer_cb_t));
        timer_ctxt->exp_flow = exp_flow;
        timer_ctxt->alg_state = this;
        start_expected_flow_timer(&exp_flow->entry, ALG_EXP_FLOW_TIMER_ID, 
                                  time_intvl, exp_flow_timeout_cb, 
                                  (void *)timer_ctxt);
    }

    *expected_flow = exp_flow;

    return HAL_RET_OK; 
}

hal_ret_t alg_state::alloc_and_insert_l4_sess(app_session_t *app_sess,
                                              l4_alg_status_t **l4_sess) {
    l4_alg_status_t *alg_status = NULL;

    alg_status = (l4_alg_status_t *)l4_sess_slab()->alloc();
    if (alg_status == NULL) {
        return HAL_RET_OOM;
    }

    HAL_SPINLOCK_LOCK(&app_sess->slock); 
    alg_status->app_session = app_sess; 
    dllist_reset(&alg_status->l4_sess_lentry);
    dllist_add(&app_sess->l4_sess_lhead, &alg_status->l4_sess_lentry);
    HAL_SPINLOCK_UNLOCK(&app_sess->slock);

    *l4_sess = alg_status;

    return HAL_RET_OK;
}

hal_ret_t alg_state::lookup_app_sess(hal::flow_key_t key, app_session_t *app_sess) {
    app_sess = (app_session_t *)app_sess_ht()->lookup((void *)&key);
    if (app_sess) {
        return HAL_RET_OK;
    }

    return HAL_RET_ENTRY_NOT_FOUND;
}

hal_ret_t alg_state::alloc_and_init_app_sess(hal::flow_key_t key, app_session_t **app_session) {
    hal_ret_t       ret = HAL_RET_OK;
    app_session_t  *app_sess = NULL;

    // Lookup if app session already exists
    ret = lookup_app_sess(key, app_sess);
    if (ret != HAL_RET_ENTRY_NOT_FOUND) {
        *app_session = app_sess;
        return HAL_RET_ENTRY_EXISTS;
    }   

    app_sess = (app_session_t *)app_sess_slab()->alloc();
    if (app_sess == NULL) {
        return HAL_RET_OOM;
    }

    HAL_SPINLOCK_INIT(&app_sess->slock, PTHREAD_PROCESS_PRIVATE);
    memcpy(&(app_sess)->key, &key, sizeof(hal::flow_key_t));
    app_sess_ht()->insert(app_sess, &app_sess->app_sess_ht_ctxt);

    *app_session = app_sess;

    return HAL_RET_OK;
}

void alg_state::move_expflow_to_l4sess(app_session_t *app_sess, 
                                            l4_alg_status_t *exp_flow) {
    remove_expected_flow(exp_flow->entry.key);

    HAL_SPINLOCK_LOCK(&app_sess->slock);
    dllist_del(&exp_flow->exp_flow_lentry);
    dllist_reset(&exp_flow->l4_sess_lentry);
    dllist_add(&app_sess->l4_sess_lhead, &exp_flow->l4_sess_lentry);
    exp_flow->alg = nwsec::APP_SVC_NONE;
    HAL_SPINLOCK_UNLOCK(&app_sess->slock);
}

/*
 * Helper to get the control session expected flow.
 */
l4_alg_status_t *alg_state::get_next_expflow(app_session_t *app_sess) {
    sdk::lib::dllist_ctxt_t   *lentry, *next;

    HAL_SPINLOCK_LOCK(&app_sess->slock);
    dllist_for_each_safe(lentry, next, &app_sess->exp_flow_lhead)
    {
        l4_alg_status_t *exp_flow = dllist_entry(lentry,
                                  l4_alg_status_t, exp_flow_lentry);
        HAL_ASSERT(exp_flow != NULL);
        return exp_flow;
    }
    HAL_SPINLOCK_UNLOCK(&app_sess->slock);

    return NULL;
}

/*
 * Helper to get the control L4 alg session.
 */
l4_alg_status_t *alg_state::get_ctrl_l4sess(app_session_t *app_sess) {
    sdk::lib::dllist_ctxt_t   *lentry, *next;

    HAL_SPINLOCK_LOCK(&app_sess->slock);
    dllist_for_each_safe(lentry, next, &app_sess->l4_sess_lhead)
    {
        l4_alg_status_t *l4_sess = dllist_entry(lentry,
                                  l4_alg_status_t, l4_sess_lentry);
        HAL_ASSERT(l4_sess != NULL);
        if (l4_sess->isCtrl == TRUE)
            return l4_sess;
    }
    HAL_SPINLOCK_UNLOCK(&app_sess->slock);

    return NULL;
}

void alg_state::cleanup_l4_sess(l4_alg_status_t *l4_sess) {
    dllist_del(&l4_sess->l4_sess_lentry);
    if (l4_sess_cleanup_hdlr_)
        l4_sess_cleanup_hdlr_(l4_sess);
    l4_sess_slab()->free(l4_sess);
}

void alg_state::cleanup_exp_flow(l4_alg_status_t *exp_flow) {
    remove_expected_flow(exp_flow->entry.key);
    dllist_del(&exp_flow->exp_flow_lentry);
    if (l4_sess_cleanup_hdlr_)
        l4_sess_cleanup_hdlr_(exp_flow);
    l4_sess_slab()->free(exp_flow);
}

void alg_state::cleanup_app_session(app_session_t *app_sess) {
    dllist_ctxt_t   *lentry, *next;

    // Take the lock 
    HAL_SPINLOCK_LOCK(&app_sess->slock);

    dllist_for_each_safe(lentry, next, &app_sess->exp_flow_lhead)
    {
        l4_alg_status_t *exp_flow = dllist_entry(lentry, 
                                  l4_alg_status_t, exp_flow_lentry);
        cleanup_exp_flow(exp_flow);
    }

    dllist_for_each_safe(lentry, next, &app_sess->l4_sess_lhead)
    {
        l4_alg_status_t *l4_sess = dllist_entry(lentry,
                                   l4_alg_status_t, l4_sess_lentry);
        cleanup_l4_sess(l4_sess);
    }

    // Callback to remove the app session from
    // hash context
    if (app_sess_cleanup_hdlr_)
        app_sess_cleanup_hdlr_(app_sess);

    HAL_SPINLOCK_UNLOCK(&app_sess->slock);

    HAL_SPINLOCK_DESTROY(&app_sess->slock);

    app_sess_slab()->free(app_sess); 
}

} //namespace alg_utils
} //namespace plugins
} //namespace hal
