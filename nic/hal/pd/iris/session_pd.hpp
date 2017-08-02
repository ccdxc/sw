#ifndef __HAL_PD_SESSION_HPP__
#define __HAL_PD_SESSION_HPP__

#include <base.h>
#include <pd.hpp>
#include <hal_state_pd.hpp>

namespace hal {
namespace pd {

typedef struct pd_flow_s {
    uint32_t    flow_hash_hw_id;
    uint32_t    flow_stats_hw_id;
    uint32_t    session_state_hw_id;
} __PACK__ pd_flow_t;

struct pd_session_s {
    void         *session;           // PI session

    // PD specific state
    uint32_t     session_state_idx;     // flow/session state index, if any
    pd_flow_t    iflow;              // iflow's PD state
    pd_flow_t    rflow;              // rflow's PD state
    uint8_t      rflow_valid:1;      // TRUE if rflow is valid
    uint8_t      conn_track_en:1;    // connection tracking enabled or not
} __PACK__;

typedef struct p4pd_flow_hash_data_s {
    uint8_t     export_en:1;
    uint32_t    flow_index;
} __PACK__ p4pd_flow_hash_data_t;

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
    g_hal_state_pd->session_slab()->free(session_pd);
    return HAL_RET_OK;
}

}   // namespace pd
}   // namespace hal

#endif    // __HAL_PD_SESSION_HPP__

