//-----------------------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------
#ifndef __DEVAPI_TYPES_HPP__
#define __DEVAPI_TYPES_HPP__

#include "include/sdk/base.hpp"
#include "include/sdk/eth.hpp"
#include "include/sdk/accel_metrics.h"
#include "nic/sdk/platform/capri/capri_barco_crypto.hpp"
#include <set>

namespace sdk {
namespace platform {
using std::set;

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
    LIF_QUEUE_PURPOSE_SVC,
    LIF_QUEUE_PURPOSE_MAX
} lif_qpurpose_t;

typedef enum lif_type_e {
    LIF_TYPE_NONE,
    LIF_TYPE_HOST,
    LIF_TYPE_HOST_MGMT,
    LIF_TYPE_MNIC_OOB_MGMT,
    LIF_TYPE_MNIC_INTERNAL_MGMT,
    LIF_TYPE_MNIC_INBAND_MGMT,
    LIF_TYPE_MNIC_CPU,
    LIF_TYPE_SWM,
    LIF_TYPE_LEARN,
    LIF_TYPE_SERVICE,
    LIF_TYPE_CONTROL,
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

typedef struct lif_bcast_filter_s {
    bool    arp;
    bool    dhcp_client;
    bool    dhcp_server;
    bool    netbios;
} __PACK__ lif_bcast_filter_t;

typedef struct lif_mcast_filter_s {
    bool    ipv6_neigh_adv;
    bool    ipv6_router_adv;
    bool    dhcpv6_relay;
    bool    dhcpv6_mcast;
    bool    ipv6_mld;
    bool    ipv6_neigh_sol;
} __PACK__ lif_mcast_filter_t;

typedef struct channel_info_s {
    uint32_t channel;
    uint32_t port_num;

    uint32_t swm_lif_id;
    std::set<mac_t> mac_table;
    std::set<vlan_t> vlan_table;
    bool receive_broadcast;
    bool receive_all_multicast;
    bool receive_promiscuous;
    bool rx_en;
    bool tx_en;
    lif_bcast_filter_t bcast_filter;
    lif_mcast_filter_t mcast_filter;
    bool vlan_enable;
    uint32_t vlan_mode;
} channel_info_t;

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
    bool rdma_sniff;
    bool rx_en; // Only for SWM lifs
    uint64_t rx_limit_bytes;
    uint64_t rx_burst_bytes;
    uint64_t tx_limit_bytes;
    uint64_t tx_burst_bytes;
    uint32_t max_vlan_filters;
    uint32_t max_mac_filters;
    uint32_t max_mac_vlan_filters;
    uint32_t tx_sched_table_offset;
    uint32_t tx_sched_num_table_entries;
    uint64_t tx_sched_bulk_eval_start_addr;
    uint8_t tx_sched_num_coses;
    bool pushed_to_hal;  // TODO: Remove from here
    lif_queue_info_t queue_info[NUM_QUEUE_TYPES];
    uint64_t qstate_addr[NUM_QUEUE_TYPES];
    lif_state_t lif_state;
    mac_addr_t mac;
    lif_bcast_filter_t bcast_filter;
    lif_mcast_filter_t mcast_filter;
} __PACK__ lif_info_t;

typedef struct port_config_s {
    uint32_t    speed;      // Mbps
    uint32_t    mtu;
    uint8_t     state;      // 0: down, 1: up
    uint8_t     an_enable;
    uint8_t     fec_type;   // TODO: port_fec_type_t
#define PORT_CFG_PAUSE_TYPE_MASK	0x0f
#define PORT_CFG_PAUSE_FLAGS_MASK	0xf0
#define PORT_CFG_PAUSE_F_TX		0x10
#define PORT_CFG_PAUSE_F_RX		0x20
    uint8_t     pause_type; // TODO: port_pause_type_t
    uint8_t     loopback_mode; // TODO: port_loopback_mode_t
    uint8_t     reset_mac_stats;
} __PACK__ port_config_t;

typedef struct xcvr_status_s {
    uint8_t     state;
    uint8_t     phy;
    uint16_t    pid;
    uint8_t     sprom[256];
} __PACK__ xcvr_status_t;

typedef struct port_status_s {
    uint32_t       id;
    uint32_t       speed;
    uint8_t        status;
    uint8_t        fec_type;    // TODO: port_fec_type_t
    uint8_t        rsvd[50];
    xcvr_status_t  xcvr;
} __PACK__ port_status_t;

typedef enum qos_class_e {
    QOS_CLASS_DEFAULT         = 0,
    QOS_CLASS_USER_DEFINED_1  = 1,
    QOS_CLASS_USER_DEFINED_2  = 2,
    QOS_CLASS_USER_DEFINED_3  = 3,
    QOS_CLASS_USER_DEFINED_4  = 4,
    QOS_CLASS_USER_DEFINED_5  = 5,
    QOS_CLASS_USER_DEFINED_6  = 6,
} qos_class_t;

typedef enum qos_class_type_e {
    QOS_CLASS_TYPE_NONE       = 0,
    QOS_CLASS_TYPE_PCP        = 1,
    QOS_CLASS_TYPE_DSCP       = 2,
} qos_class_type_t;

typedef enum pause_type_e {
    PAUSE_TYPE_NONE       = 0,
    PAUSE_TYPE_LINK_LEVEL = 1,
    PAUSE_TYPE_PFC        = 2,
} pause_type_t;

typedef enum qos_sched_e {
    QOS_SCHED_TYPE_STRICT = 0,
    QOS_SCHED_TYPE_DWRR   = 1,
} qos_sched_type_t;

typedef struct qos_class_info_s {
    uint8_t     group;          /* qos_class_t */
    uint32_t    mtu;

    uint8_t     pause_type;     /* pause_type_t */
    uint8_t     pause_dot1q_pcp;
    uint32_t    pause_xon_threshold;
    uint32_t    pause_xoff_threshold;

    uint8_t     sched_type;     /* qos_sched_type_t */
    uint64_t    sched_strict_rlmt;
    uint8_t     sched_dwrr_weight;

    uint8_t     class_type;     /* qos_class_type_t */
    uint8_t     class_dot1q_pcp;
    uint8_t     class_ndscp;
    uint8_t     class_ip_dscp[64];

    uint8_t     rewrite_dot1q_pcp_en;
    uint8_t     rewrite_dot1q_pcp;
    uint8_t     rewrite_ip_dscp_en;
    uint8_t     rewrite_ip_dscp;

    bool        no_drop;
} qos_class_info_t;

typedef struct {
     uint32_t    ring_handle;
     uint32_t    sub_ring;
     uint64_t    base_pa;
     uint64_t    pndx_pa;
     uint64_t    shadow_pndx_pa;
     uint64_t    opaque_tag_pa;
     uint32_t    opaque_tag_size;
     uint32_t    ring_size;
     uint32_t    desc_size;
     uint32_t    pndx_size;
     uint32_t    sw_reset_capable;
     uint32_t    sw_enable_capable;
 } accel_rgroup_rinfo_rsp_t;

typedef void (*accel_rgroup_rinfo_rsp_cb_t)(void *user_ctx,
                                            const accel_rgroup_rinfo_rsp_t& info);

 typedef struct {
     uint32_t    ring_handle;
     uint32_t    sub_ring;
     uint32_t    pndx;          /* producer index */
     uint32_t    cndx;          /* consumer index */
     uint32_t    endx;          /* error index */
 } accel_rgroup_rindices_rsp_t;

typedef void (*accel_rgroup_rindices_rsp_cb_t)(void *user_ctx,
                                               const accel_rgroup_rindices_rsp_t& indices);

 typedef struct {
     uint32_t    ring_handle;
     uint32_t    sub_ring;
     uint64_t    input_bytes;
     uint64_t    output_bytes;
     uint64_t    soft_resets;
 } accel_rgroup_rmetrics_rsp_t;

typedef void (*accel_rgroup_rmetrics_rsp_cb_t)(void *user_ctx,
                                               const accel_rgroup_rmetrics_rsp_t& indices);

 typedef struct {
     uint32_t    ring_handle;
     uint32_t    sub_ring;
     uint32_t    num_reg_vals;
     accel_ring_reg_val_t reg_val[ACCEL_RING_NUM_REGS_MAX];
 } accel_rgroup_rmisc_rsp_t;

typedef void (*accel_rgroup_rmisc_rsp_cb_t)(void *user_ctx,
                                            const accel_rgroup_rmisc_rsp_t& indices);

}    // namespace platform
}    // namespace sdk

using sdk::platform::mac_t;
using sdk::platform::vlan_t;
using sdk::platform::lif_queue_info_t;
using sdk::platform::lif_info_t;
using sdk::platform::lif_type_t;
using sdk::platform::port_config_t;
using sdk::platform::port_status_t;
using sdk::platform::xcvr_status_t;
using sdk::platform::accel_rgroup_rinfo_rsp_t;
using sdk::platform::accel_rgroup_rindices_rsp_t;
using sdk::platform::accel_rgroup_rmetrics_rsp_t;
using sdk::platform::accel_rgroup_rmisc_rsp_t;
using sdk::platform::accel_rgroup_rinfo_rsp_cb_t;
using sdk::platform::accel_rgroup_rindices_rsp_cb_t;
using sdk::platform::accel_rgroup_rmetrics_rsp_cb_t;
using sdk::platform::accel_rgroup_rmisc_rsp_cb_t;
using sdk::platform::lif_qpurpose_t;
using sdk::platform::qos_class_info_t;
using sdk::platform::qos_sched_type_t;
using sdk::platform::qos_class_type_t;
using sdk::platform::pause_type_t;
using sdk::platform::lif_bcast_filter_t;
using sdk::platform::lif_mcast_filter_t;
using sdk::platform::channel_info_t;

#endif    // __DEVAPI_TYPES_HPP__
