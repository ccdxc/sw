#ifndef __HAL_PD_API_HPP__
#define __HAL_PD_API_HPP__

#include <tenant.hpp>
#include <l2segment.hpp>
#include <nwsec.hpp>
#include <interface.hpp>
#include <endpoint.hpp>
#include <session.hpp>
#include <tlscb.hpp>
#include <tcpcb.hpp>
#include <qos.hpp>

namespace hal {
namespace pd {

using hal::tenant_t;
using hal::l2seg_t;
using hal::nwsec_profile_t;
using hal::if_t;
using hal::lif_t;
using hal::session_t;
using hal::flow_key_t;
using hal::flow_t;
using hal::tlscb_t;
using hal::tcpcb_t;
using hal::buf_pool_t;
using hal::queue_t;
using hal::policer_t;

typedef uint16_t    l2seg_hw_id_t;

typedef struct pd_tenant_args_s {
    tenant_t           *tenant;
    nwsec_profile_t    *nwsec_profile;
} __PACK__ pd_tenant_args_t;

typedef struct pd_l2seg_args_s {
    tenant_t           *tenant;
    l2seg_t            *l2seg;
} __PACK__ pd_l2seg_args_t;

typedef struct pd_nwsec_profile_args_s {
    nwsec_profile_t    *nwsec_profile;
} __PACK__ pd_nwsec_profile_args_t;

typedef struct pd_lif_args_s {
    lif_t    *lif;
} __PACK__ pd_lif_args_t;

typedef struct pd_if_args_s {
    if_t    *intf;
} __PACK__ pd_if_args_t;

typedef struct pd_ep_args_s {
    tenant_t        *tenant;
    l2seg_t         *l2seg;
    if_t            *intf;
    ep_t            *ep;
} __PACK__ pd_ep_args_t;

typedef struct pd_sessoin_args_s {
    tenant_t           *tenant;
    nwsec_profile_t    *nwsec_prof;
    l2seg_t            *l2seg_s;
    l2seg_t            *l2seg_d;
    if_t               *sif;
    if_t               *dif;
    ep_t               *sep;
    ep_t               *dep;
    session_t          *session;
    session_state_t    *session_state;
} __PACK__ pd_session_args_t;

typedef struct pd_l2seg_uplink_args_s {
    l2seg_t     *l2seg;
    if_t        *intf;
} __PACK__ pd_l2seg_uplink_args_t;

typedef struct pd_tlscb_args_s {
    tlscb_t            *tlscb;
} __PACK__ pd_tlscb_args_t;

typedef struct pd_tcpcb_args_s {
    tcpcb_t            *tcpcb;
} __PACK__ pd_tcpcb_args_t;

static inline void
pd_tenant_args_init (pd_tenant_args_t *args)
{
    args->tenant = NULL;
    return;
}

static inline void
pd_l2seg_args_init (pd_l2seg_args_t *args)
{
    args->tenant = NULL;
    args->l2seg = NULL;
    return;
}

static inline void
pd_nwsec_profile_args_init (pd_nwsec_profile_args_t *args)
{
    args->nwsec_profile = NULL;
    return;
}

static inline void
pd_lif_args_init (pd_lif_args_t *args)
{
    args->lif = NULL;
    return;
}

static inline void
pd_if_args_init (pd_if_args_t *args)
{
    args->intf = NULL;
    return;
}

static inline void
pd_ep_args_init (pd_ep_args_t *args)
{
    args->tenant = NULL;
    args->l2seg = NULL;
    args->intf = NULL;
    args->ep = NULL;
    return;
}

static inline void
pd_session_args_init (pd_session_args_t *args)
{
    args->tenant = NULL;
    args->l2seg_s = args->l2seg_d = NULL;
    args->sif = args->dif = NULL;
    args->sep = args->dep = NULL;
    args->session = NULL;
    args->session_state = NULL;
    return;
}


static inline void
pd_l2seg_uplinkif_args_init (pd_l2seg_uplink_args_t *args)
{
    args->l2seg = NULL;
    args->intf = NULL;
    return;
}

static inline void
pd_tlscb_args_init (pd_tlscb_args_t *args)
{
    args->tlscb = NULL;
    return;
}

static inline void
pd_tcpcb_args_init (pd_tcpcb_args_t *args)
{
    args->tcpcb = NULL;
    return;
}

hal_ret_t pd_tenant_create(pd_tenant_args_t *tenant);
hal_ret_t pd_tenant_update(pd_tenant_args_t *tenant);
hal_ret_t pd_tenant_delete(pd_tenant_args_t *tenant);

hal_ret_t pd_l2seg_create(pd_l2seg_args_t *l2seg);
hal_ret_t pd_l2seg_update(pd_l2seg_args_t *l2seg);
hal_ret_t pd_l2seg_delete(pd_l2seg_args_t *l2seg);
l2seg_t *find_l2seg_by_hwid (l2seg_hw_id_t hwid);

hal_ret_t pd_nwsec_profile_create(pd_nwsec_profile_args_t *nwsec_profile);
hal_ret_t pd_nwsec_profile_update(pd_nwsec_profile_args_t *nwsec_profile);

hal_ret_t pd_lif_create(pd_lif_args_t *lif);
hal_ret_t pd_lif_update(pd_lif_args_t *lif);
hal_ret_t pd_lif_delete(pd_lif_args_t *lif);

hal_ret_t pd_if_create(pd_if_args_t *hal_if);
hal_ret_t pd_if_update(pd_if_args_t *hal_if);
hal_ret_t pd_if_delete(pd_if_args_t *hal_if);

hal_ret_t pd_ep_create(pd_ep_args_t *hal_if);
hal_ret_t pd_ep_update(pd_ep_args_t *hal_if);
hal_ret_t pd_ep_delete(pd_ep_args_t *hal_if);

hal_ret_t pd_session_create(pd_session_args_t *session);
hal_ret_t pd_session_update(pd_session_args_t *session);
hal_ret_t pd_session_delete(pd_session_args_t *session);

hal_ret_t pd_add_l2seg_uplink(pd_l2seg_uplink_args_t *args);

hal_ret_t pd_tlscb_create(pd_tlscb_args_t *tlscb);
hal_ret_t pd_tlscb_update(pd_tlscb_args_t *tlscb);
hal_ret_t pd_tlscb_delete(pd_tlscb_args_t *tlscb);

hal_ret_t pd_tcpcb_create(pd_tcpcb_args_t *tcpcb);
hal_ret_t pd_tcpcb_update(pd_tcpcb_args_t *tcpcb);
hal_ret_t pd_tcpcb_delete(pd_tcpcb_args_t *tcpcb);

typedef struct pd_buf_pool_args_s {
    buf_pool_t    *buf_pool;
} __PACK__ pd_buf_pool_args_t;

static inline void
pd_buf_pool_args_init (pd_buf_pool_args_t *args)
{
    args->buf_pool = NULL;
    return;
}

hal_ret_t pd_buf_pool_create(pd_buf_pool_args_t *buf_pool);

typedef struct pd_queue_args_s {
    uint32_t    cnt_l0;
    queue_t    **l0_nodes;
    uint32_t    cnt_l1;
    queue_t    **l1_nodes;
    uint32_t    cnt_l2;
    queue_t    **l2_nodes;
} __PACK__ pd_queue_args_t;

static inline void
pd_queue_args_init (pd_queue_args_t *args)
{
    args->cnt_l0 = 0;
    args->cnt_l1 = 0;
    args->cnt_l2 = 0;
    args->l0_nodes = NULL;
    args->l1_nodes = NULL;
    args->l2_nodes = NULL;
    return;
}

hal_ret_t pd_queue_create(pd_queue_args_t *queue);

typedef struct pd_policer_args_s {
    policer_t    *policer;
} __PACK__ pd_policer_args_t;

static inline void
pd_policer_args_init (pd_policer_args_t *args)
{
    args->policer = NULL;
    return;
}

hal_ret_t pd_policer_create(pd_policer_args_t *policer);

}    // namespace pd
}    // namespace hal

#endif    // __HAL_PD_API_HPP__
