
#ifndef __HAL_TYPES_HPP__
#define __HAL_TYPES_HPP__

#include "gen/proto/kh.grpc.pb.h"
#include "gen/proto/interface.grpc.pb.h"

#define NUM_QUEUE_TYPES     8

#define LIF_DEFAULT_MAX_VLAN_FILTERS        400
#define LIF_DEFAULT_MAX_MAC_FILTERS         400
#define LIF_DEFAULT_MAX_MAC_VLAN_FILTERS    400

#define NATIVE_VLAN_ID 8192
/*
 * Types
 */

typedef uint64_t mac_t;
typedef uint16_t vlan_t;
typedef uint64_t l2seg_id_t;
typedef uint32_t uplink_id_t;
typedef kh::FilterType filter_type_t;
typedef std::tuple<mac_t, vlan_t> mac_vlan_t;
typedef std::tuple<filter_type_t, mac_t, vlan_t> mac_vlan_filter_t;

/**
 * Ethernet Lifs
 */
class Uplink;
// Similar to queue_info in hal_client.hpp
typedef struct hal_queue_info_s {
  uint32_t type_num;           /* HW Queue Type */
  uint32_t size;               /* Qstate Size: 2^size */
  uint32_t entries;            /* Number of Queues: 2^entries */
  intf::LifQPurpose purpose; /* Queue Purpose */
  const char* prog;            /* Program File Name */
  const char* label;           /* Program Entry Label */
  const char* qstate;          /* Qstate structure */
} hal_queue_info_t;

typedef struct hal_lif_info_s {
    uint64_t id;
    uint64_t hw_lif_id;
    std::string name;
    types::LifType type;
    uint32_t pinned_uplink_port_num;
    // Uplink *pinned_uplink;
    bool is_management;
    bool vlan_strip_en;
    bool vlan_insert_en;
    bool receive_broadcast;
    bool receive_all_multicast;
    bool receive_promiscuous;
    bool enable_rdma;
    uint32_t max_vlan_filters;
    uint32_t max_mac_filters;
    uint32_t max_mac_vlan_filters;
    bool pushed_to_hal;
    hal_queue_info_t queue_info[NUM_QUEUE_TYPES];
    uint64_t qstate_addr[NUM_QUEUE_TYPES];
} hal_lif_info_t;

enum HalEthQtype {
    HAL_ETH_QTYPE_RX = 0,
    HAL_ETH_QTYPE_TX = 1,
    HAL_ETH_QTYPE_ADMIN = 2,
    HAL_ETH_QTYPE_SQ = 3,
    HAL_ETH_QTYPE_RQ = 4,
    HAL_ETH_QTYPE_CQ = 5,
    HAL_ETH_QTYPE_EQ = 6,
};

struct eth_spec {
    uint8_t uplink;
    // Resources
    uint32_t rxq_count;
    uint32_t txq_count;
    uint32_t adminq_count;
    uint32_t rdma_sq_size;
    uint32_t rdma_sq_count;
    uint32_t rdma_rq_size;
    uint32_t rdma_rq_count;
    uint32_t rdma_cq_size;
    uint32_t rdma_cq_count;
    uint32_t rdma_eq_size;
    uint32_t rdma_eq_count;
    uint32_t intr_count;
    uint32_t eq_count;
    uint32_t rdma_pid_count;
    // Features
    // TODO: Implement feature flags
    bool enable_rdma;
    // TODO: Implement feature spec
    uint32_t max_pt_entries;
    uint32_t max_keys;
    //in 8MB units
    uint32_t barmap_size;
};

typedef enum {
    HAL_IRISC_RET_SUCCESS,
    HAL_IRISC_RET_LIMIT_REACHED,
    HAL_IRISC_DUP_ADDR_ADD
} hal_irisc_ret_t;

#endif  /* __HAL_TYPES_HPP__ */
