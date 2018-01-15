#ifndef __HAL_PD_APP_REDIR_IF_HPP__
#define __HAL_PD_APP_REDIR_IF_HPP__

#include "nic/include/base.h"
#include "nic/include/pd.hpp"

namespace hal {
namespace pd {

struct pd_app_redir_if_s {
    uint32_t    lport_id;

    void        *pi_if;         // pi ptr
} __PACK__;

// ----------------------------------------------------------------------------
// Allocate APPREDIR IF Instance
// ----------------------------------------------------------------------------
static inline pd_app_redir_if_t *
pd_app_redir_if_alloc (void)
{
    pd_app_redir_if_t    *app_redir_if;

    app_redir_if = (pd_app_redir_if_t *)g_hal_state_pd->app_redir_if_pd_slab()->alloc();
    if (app_redir_if == NULL) {
        return NULL;
    }
    return app_redir_if;
}

// ----------------------------------------------------------------------------
// Initialize APPREDIR IF PD instance
// ----------------------------------------------------------------------------
static inline pd_app_redir_if_t *
pd_app_redir_if_init (pd_app_redir_if_t *app_redir_if)
{
    // Nothing to do currently
    if (!app_redir_if) {
        return NULL;
    }

    // Set here if you want to initialize any fields

    return app_redir_if;
}

// ----------------------------------------------------------------------------
// Allocate and Initialize APPREDIR If PD Instance
// ----------------------------------------------------------------------------
static inline pd_app_redir_if_t *
pd_app_redir_if_alloc_init(void)
{
    return pd_app_redir_if_init(pd_app_redir_if_alloc());
}

// ----------------------------------------------------------------------------
// Freeing APPREDIR IF PD
// ----------------------------------------------------------------------------
static inline hal_ret_t
pd_app_redir_if_free (pd_app_redir_if_t *app_redir_if)
{
    g_hal_state_pd->app_redir_if_pd_slab()->free(app_redir_if);
    return HAL_RET_OK;
}

// ----------------------------------------------------------------------------
// Freeing APPREDIR IF PD memory
// ----------------------------------------------------------------------------
static inline hal_ret_t
pd_app_redir_if_pd_mem_free (pd_app_redir_if_t *app_redir_if)
{
    g_hal_state_pd->app_redir_if_pd_slab()->free(app_redir_if);
    return HAL_RET_OK;
}

// ----------------------------------------------------------------------------
// Linking PI <-> PD
// ----------------------------------------------------------------------------
static inline void 
app_redir_if_link_pi_pd(pd_app_redir_if_t *pd_app_redir_if, if_t *pi_if)
{
    pd_app_redir_if->pi_if = pi_if;
    if_set_pd_if(pi_if, pd_app_redir_if);
}

// ----------------------------------------------------------------------------
// Un-Linking PI <-> PD
// ----------------------------------------------------------------------------
static inline void 
app_redir_if_delink_pi_pd(pd_app_redir_if_t *pd_app_redir_if, if_t *pi_if)
{
    pd_app_redir_if->pi_if = NULL;
    if_set_pd_if(pi_if, NULL);
}


hal_ret_t pd_app_redir_if_cleanup(pd_app_redir_if_t *upif_pd);

pd_lif_t *pd_app_redir_if_get_pd_lif(pd_app_redir_if_t *pd_app_redir_if);

hal_ret_t pd_app_redir_if_create(pd_if_args_t *args);
hal_ret_t pd_app_redir_if_update(pd_if_args_t *args);
hal_ret_t pd_app_redir_if_delete(pd_if_args_t *args);
hal_ret_t pd_app_redir_if_make_clone(if_t *hal_if, if_t *clone);
hal_ret_t pd_app_redir_if_mem_free(pd_if_args_t *args);

}   // namespace pd
}   // namespace hal
#endif    // __HAL_PD_APP_REDIR_IF_HPP__

