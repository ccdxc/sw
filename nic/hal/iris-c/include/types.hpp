
#ifndef __TYPES_HPP__
#define __TYPES_HPP__

#include "kh.grpc.pb.h"

#define NUM_QUEUE_TYPES     8

/*
 * Types
 */

typedef uint64_t mac_t;
typedef uint16_t vlan_t;
typedef uint32_t l2seg_id_t;
typedef uint32_t uplink_id_t;
typedef kh::FilterType filter_type_t;
typedef std::tuple<mac_t, vlan_t> mac_vlan_t;
typedef std::tuple<filter_type_t, mac_t, vlan_t> mac_vlan_filter_t;

/**
 * Ethernet Lifs
 */
class Uplink;
typedef struct lif_info_s {
    uint32_t hw_lif_id;
    Uplink *pinned_uplink;
    bool is_management;
    bool vlan_strip_en;
    bool vlan_insert_en;
    bool receive_broadcast;
    bool receive_all_multicast;
    bool receive_promiscuous;
} lif_info_t;



enum EthQtype {
    ETH_QTYPE_RX = 0,
    ETH_QTYPE_TX = 1,
    ETH_QTYPE_ADMIN = 2,
    ETH_QTYPE_SQ = 3,
    ETH_QTYPE_RQ = 4,
    ETH_QTYPE_CQ = 5,
    ETH_QTYPE_EQ = 6,
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
};

#endif  /* __TYPES_HPP__ */
