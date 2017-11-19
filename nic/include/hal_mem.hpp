// {C} Copyright 2017 Pensando Systems Inc. All rights reserved

#ifndef __HAL_MEM_HPP__
#define __HAL_MEM_HPP__

#include "nic/include/base.h"
#include "nic/include/mtrack.hpp"

namespace hal {

// HAL memory slabs
typedef enum hal_slab_e {
    HAL_SLAB_NONE,
    HAL_SLAB_HANDLE,
    HAL_SLAB_HANDLE_HT_ENTRY,
    HAL_SLAB_HANDLE_LIST_ENTRY,
    HAL_SLAB_HANDLE_ID_HT_ENTRY,
    HAL_SLAB_HANDLE_ID_LIST_ENTRY,
    HAL_SLAB_VRF,
    HAL_SLAB_NETWORK,
    HAL_SLAB_L2SEG,
    HAL_SLAB_LIF,
    HAL_SLAB_IF,
    HAL_SLAB_ENIC_L2SEG_ENTRY,
    HAL_SLAB_PORT,
    HAL_SLAB_EP,
    HAL_SLAB_EP_IP_ENTRY,
    HAL_SLAB_EP_L3_ENTRY,
    HAL_SLAB_FLOW,
    HAL_SLAB_SESSION,
    HAL_SLAB_SECURITY_PROFILE,
    HAL_SLAB_TLSCB,
    HAL_SLAB_TCPCB,
    HAL_SLAB_BUF_POOL,
    HAL_SLAB_QUEUE,
    HAL_SLAB_POLICER,
    HAL_SLAB_ACL,
    HAL_SLAB_WRING,
    HAL_SLAB_PROXY,
    HAL_SLAB_PROXY_FLOW_INFO,
    HAL_SLAB_IPSECCB,
    HAL_SLAB_L4LB,
    HAL_SLAB_CPUCB,
    HAL_SLAB_RAWRCB,
    HAL_SLAB_RAWCCB,
    HAL_SLAB_NWSEC_POLICY,
    HAL_SLAB_NWSEC_GROUP,
    HAL_SLAB_NWSEC_POLICY_RULES,
    HAL_SLAB_NWSEC_POLICY_CFG,
    HAL_SLAB_NWSEC_POLICY_SVC,
    HAL_SLAB_NWSEC_POLICY_EP_INFO,
    HAL_SLAB_DOS_POLICY,
    HAL_SLAB_DOS_POLICY_SG_LIST,
    HAL_SLAB_LKLSHIM_FLOWDB,
    HAL_SLAB_LKLSHIM_LSOCKDB,
    HAL_SLAB_LKLSHIM_LSOCKS,
    HAL_SLAB_ARP_LEARN,
    HAL_SLAB_DHCP_LEARN,
    HAL_SLAB_EVENT_MAP,
    HAL_SLAB_EVENT_MAP_LISTENER,
    HAL_SLAB_EVENT_LISTENER,
    HAL_SLAB_PI_MAX,  // NOTE: MUST be last PI slab id

    // PD Slabs
    HAL_SLAB_VRF_PD,
    HAL_SLAB_L2SEG_PD,
    HAL_SLAB_LIF_PD,
    HAL_SLAB_UPLINKIF_PD,
    HAL_SLAB_UPLINKPC_PD,
    HAL_SLAB_ENICIF_PD,
    HAL_SLAB_IF_L2SEG_PD,
    HAL_SLAB_TUNNELIF_PD,
    HAL_SLAB_CPUIF_PD,
    HAL_SLAB_DOS_POLICY_PD,
    HAL_SLAB_SECURITY_PROFILE_PD,
    HAL_SLAB_EP_PD,
    HAL_SLAB_EP_IP_ENTRY_PD,
    HAL_SLAB_SESSION_PD,
    HAL_SLAB_TLSCB_PD,
    HAL_SLAB_TCPCB_PD,
    HAL_SLAB_BUF_POOL_PD,
    HAL_SLAB_QUEUE_PD,
    HAL_SLAB_POLICER_PD,
    HAL_SLAB_ACL_PD,
    HAL_SLAB_WRING_PD,
    HAL_SLAB_IPSECCB_PD,
    HAL_SLAB_IPSECCB_DECRYPT_PD,
    HAL_SLAB_L4LB_PD,
    HAL_SLAB_RW_PD,
    HAL_SLAB_TUNNEL_RW_PD,
    HAL_SLAB_CPUCB_PD,
    HAL_SLAB_CPUPKT_PD,
    HAL_SLAB_RAWRCB_PD,
    HAL_SLAB_RAWCCB_PD,
    HAL_SLAB_CPUPKT_QINST_INFO_PD,
    HAL_SLAB_DIRECTMAP_ENTRY,
    HAL_SLAB_DIRECTMAP_DATA,
    HAL_SLAB_PORT_PD,
    HAL_SLAB_PD_MAX,                 // NOTE: MUST be last PD slab id

    HAL_SLAB_RSVD,  // all non-delay delete slabs can use this

    HAL_SLAB_MAX,
} hal_slab_t;

enum {
    HAL_MEM_ALLOC_NONE,
    HAL_MEM_ALLOC_INFRA,
    HAL_MEM_ALLOC_LIB_HT,
    HAL_MEM_ALLOC_LIB_SLAB,
    HAL_MEM_ALLOC_LIB_BITMAP,
    HAL_MEM_ALLOC_LIB_TWHEEL,
    HAL_MEM_ALLOC_IF,
    HAL_MEM_ALLOC_L2,
    HAL_MEM_ALLOC_L3,
    HAL_MEM_ALLOC_EP,
    HAL_MEM_ALLOC_SFW,
    HAL_MEM_ALLOC_L4LB,
    HAL_MEM_ALLOC_FLOW,
    HAL_MEM_ALLOC_PD,
    HAL_MEM_ALLOC_QOS,
    HAL_MEM_ALLOC_LIB_ACL_TCAM,
    HAL_MEM_ALLOC_DLLIST,
    HAL_MEM_ALLOC_EVENT_MGR,
    HAL_MEM_ALLOC_CATALOG,
    HAL_MEM_ALLOC_DEBUG_CLI,
    HAL_MEM_ALLOC_ALG,
    HAL_MEM_ALLOC_BLOCK_LIST,
    HAL_MEM_ALLOC_BLOCK_LIST_NODE,
    HAL_MEM_ALLOC_OTHER,
    HAL_MEM_ALLOC_RSVD = 0xFFFFFFFF,    // reserved and shouldn't be used
};

//------------------------------------------------------------------------------
// use these memory allocation and free macros when one off allocation is
// needed, otherwise use slabs
//------------------------------------------------------------------------------
#define HAL_MALLOC(alloc_id, size)    (hal::utils::g_hal_mem_mgr.mtrack_alloc(static_cast<uint32_t>(alloc_id), false, size, __FUNCTION__, __LINE__))
#define HAL_CALLOC(alloc_id, size)    (hal::utils::g_hal_mem_mgr.mtrack_alloc(static_cast<uint32_t>(alloc_id), true, size, __FUNCTION__, __LINE__))
#define HAL_FREE(alloc_id, ptr)       (hal::utils::g_hal_mem_mgr.mtrack_free(static_cast<uint32_t>(alloc_id), ptr, __FUNCTION__, __LINE__))

hal_ret_t free_to_slab(hal_slab_t slab_id, void *elem);

namespace pd {

hal_ret_t free_to_slab(hal_slab_t slab_id, void *elem);

}    // namespace pd

}    // namespace hal

#endif    // __HAL_MEM_HPP__

