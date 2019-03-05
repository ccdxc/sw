//-----------------------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------
#ifndef __DEVAPI_TYPES_HPP__
#define __DEVAPI_TYPES_HPP__

namespace sdk {
namespace platform {

#define NUM_QUEUE_TYPES 8

typedef uint64_t mac_t;
typedef uint32_t vlan_t;

typedef enum lif_qpurpose_ {
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

typedef enum lif_type_ {
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
} lif_queue_info_t;

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
} lif_info_t;

typedef enum fwd_mode_ {
    FWD_MODE_CLASSIC,
    FWD_MODE_SMART,
} fwd_mode_t;

} // namespace platform
} // namespace sdk

using sdk::platform::mac_t;
using sdk::platform::vlan_t;
using sdk::platform::lif_queue_info_t;
using sdk::platform::lif_info_t;
using sdk::platform::fwd_mode_t;

#endif  // __DEVAPI_TYPES_HPP__
