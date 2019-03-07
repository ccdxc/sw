//-----------------------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------
#ifndef __DEVAPI_TYPES_HPP__
#define __DEVAPI_TYPES_HPP__

#include "include/sdk/base.hpp"

namespace sdk {
namespace platform {

#define NUM_QUEUE_TYPES 8

typedef uint64_t mac_t;
typedef uint32_t vlan_t;

typedef enum lif_qpurpose_e {
    LIF_QUEUE_PURPOSE_NONE,
    LIF_QUEUE_PURPOSE_ADMIN,
    LIF_QUEUE_PURPOSE_TX,
    LIF_QUEUE_PURPOSE_RX,
    LIF_QUEUE_PURPOSE_RDMA_SEND,
    LIF_QUEUE_PURPOSE_RDMA_RECV,
    LIF_QUEUE_PURPOSE_CQ,
    LIF_QUEUE_PURPOSE_EQ,
    LIF_QUEUE_PURPOSE_NVME,
    LIF_QUEUE_PURPOSE_STORAGE,
    LIF_QUEUE_PURPOSE_VIRTIO_TX,
    LIF_QUEUE_PURPOSE_VIRTIO_RX,
    LIF_QUEUE_PURPOSE_SVC
} lif_qpurpose_t;

typedef enum lif_type_e {
    LIF_TYPE_NONE,
    LIF_TYPE_HOST,
    LIF_TYPE_HOST_MANAGEMENT,
    LIF_TYPE_MNIC_OOB_MANAGEMENT,
    LIF_TYPE_MNIC_INTERNAL_MANAGEMENT,
    LIF_TYPE_MNIC_INBAND_MANAGEMENT
} lif_type_t;

typedef struct lif_queue_info_s {
    uint32_t type_num;           /* HW Queue Type */
    uint32_t size;               /* Qstate Size: 2^size */
    uint32_t entries;            /* Number of Queues: 2^entries */
    lif_qpurpose_t purpose;      /* Queue Purpose */
    const char* prog;            /* Program File Name */
    const char* label;           /* Program Entry Label */
    const char* qstate;          /* Qstate structure */
} __PACK__ lif_queue_info_t;

typedef struct lif_info_s {
    uint64_t lif_id;
    char name[256];
    lif_type_t type;
    uint32_t pinned_uplink_port_num;
    bool is_management;
    bool vlan_strip_en;
    bool vlan_insert_en;
    bool receive_broadcast;
    bool receive_all_multicast;
    bool receive_promiscuous;
    bool enable_rdma;
    uint64_t rx_limit_bytes;
    uint64_t rx_burst_bytes;
    uint64_t tx_limit_bytes;
    uint64_t tx_burst_bytes;
    uint32_t max_vlan_filters;
    uint32_t max_mac_filters;
    uint32_t max_mac_vlan_filters;
    bool pushed_to_hal;  // TODO: Remove from here
    lif_queue_info_t queue_info[NUM_QUEUE_TYPES];
    uint64_t qstate_addr[NUM_QUEUE_TYPES];
} __PACK__ lif_info_t;

typedef enum fwd_mode_e {
    FWD_MODE_CLASSIC,
    FWD_MODE_SMART,
} fwd_mode_t;

typedef struct port_config_s {
    uint32_t    speed;
    uint32_t    mtu;
    uint8_t     state;
    uint8_t     an_enable;
    uint8_t     fec_type;
    uint8_t     pause_type;
    uint8_t     loopback_mode;
} __PACK__ port_config_t;

typedef struct xcvr_status_s {
    uint8_t     state;
    uint8_t     phy;
    uint16_t    pid;
    uint8_t     sprom[256];
} __PACK__ xcvr_status_t;

typedef struct port_status_s {
    uint32_t       speed;
    uint8_t        id;
    uint8_t        status;
    xcvr_status_t  xcvr;
} __PACK__ port_status_t;

}    // namespace platform
}    // namespace sdk

using sdk::platform::mac_t;
using sdk::platform::vlan_t;
using sdk::platform::lif_queue_info_t;
using sdk::platform::lif_info_t;
using sdk::platform::fwd_mode_t;
using sdk::platform::port_config_t;
using sdk::platform::port_status_t;
using sdk::platform::xcvr_status_t;

#endif    // __DEVAPI_TYPES_HPP__
