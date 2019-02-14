#ifndef __HAL_PD_SESSION_HPP__
#define __HAL_PD_SESSION_HPP__

#include "nic/include/base.hpp"
#include "nic/include/pd.hpp"
#include "nic/hal/pd/iris/hal_state_pd.hpp"
#include "nic/sdk/include/sdk/table.hpp"

namespace hal {
namespace pd {

typedef struct pd_flow_s {
    // HW ID of the associated data (info and stats)
    uint32_t assoc_hw_id;
    uint8_t installed:1;
    uint8_t valid:1;
    uint8_t spare:6;
} __PACK__ pd_flow_t;

struct pd_session_s {
    void *session; // PI session
    // PD specific state
    pd_flow_t iflow; // iflow's PD state
    pd_flow_t rflow; // rflow's PD state
    pd_flow_t iflow_aug; // augmented iflow's PD state
    pd_flow_t rflow_aug; // augmented rflow's PD state
    uint32_t session_state_idx; // flow/session state index, if any
    uint8_t  conn_track_en:1; // connection tracking enabled or not
    uint8_t  spare:7;
} __PACK__;

// allocate a session pd instance
static inline pd_session_t *
session_pd_alloc (void)
{
    pd_session_t    *session_pd;

    session_pd = (pd_session_t *)g_hal_state_pd->session_slab()->alloc();
    if (session_pd == NULL) {
        return NULL;
    }

    return session_pd;
}

// initialize a session pd instance
static inline pd_session_t *
session_pd_init (pd_session_t *session_pd)
{
    if (!session_pd) {
        return NULL;
    }
    session_pd->session = NULL;

    return session_pd;
}

// allocate and initialize a session pd instance
static inline pd_session_t *
session_pd_alloc_init (void)
{
    return session_pd_init(session_pd_alloc());
}

// free session pd instance
static inline hal_ret_t
session_pd_free (pd_session_t *session_pd)
{
    hal::pd::delay_delete_to_slab(HAL_SLAB_SESSION_PD, session_pd);
    return HAL_RET_OK;
}

}   // namespace pd
}   // namespace hal

#endif    // __HAL_PD_SESSION_HPP__

