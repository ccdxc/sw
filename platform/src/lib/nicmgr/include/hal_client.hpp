/*
* Copyright (c) 2018, Pensando Systems Inc.
*/

#ifndef __HAL_CLIENT_HPP__
#define __HAL_CLIENT_HPP__

#include <map>
#include <grpc++/grpc++.h>

#include "vrf.grpc.pb.h"
#include "types.grpc.pb.h"
#include "interface.grpc.pb.h"
#include "internal.grpc.pb.h"
#include "endpoint.grpc.pb.h"
#include "l2segment.grpc.pb.h"
#include "multicast.grpc.pb.h"
#include "rdma.grpc.pb.h"

using namespace kh;
using namespace types;
using namespace vrf;
using namespace intf;
using namespace internal;
using namespace endpoint;
using namespace l2segment;
using namespace multicast;
using namespace rdma;

using namespace grpc;
using namespace std;

#define NUM_QUEUE_TYPES 8
#define MAX_HOST_LIFS 1000

/**
 * Forwarding Modes
 */
enum ForwardingMode
{
    FWD_MODE_CLASSIC_NIC,
    FWD_MODE_SMART_NIC,
};

/**
 * Queue info structure for LifCreate
 */
struct queue_info {
  uint32_t type_num;           /* HW Queue Type */
  uint32_t size;               /* Qstate Size: 2^size */
  uint32_t entries;            /* Number of Queues: 2^entries */
  ::intf::LifQPurpose purpose; /* Queue Purpose */
  const char* prog;            /* Program File Name */
  const char* label;           /* Program Entry Label */
  const char* qstate;          /* Qstate structure */
};

/**
 * Lif info structure set by LifCreateF
 */
struct lif_info {
  uint64_t hw_lif_id;
  uint64_t qstate_addr[NUM_QUEUE_TYPES];
};

/**
 * Client for interacting with HAL
 */
class HalClient {
public:
  HalClient(enum ForwardingMode fwd_mode);

  /* Segment APIs */
  int L2SegmentProbe();

  uint64_t L2SegmentGet(uint64_t l2seg_id);

  uint64_t L2SegmentCreate(uint64_t vrf_id,
                           uint64_t l2seg_id,
                           uint16_t vlan_id);

  int AddL2SegmentOnUplink(uint64_t uplink_if_id,
                           uint64_t l2seg_id);

  /* VRF APIs */
  int VrfProbe();

  uint64_t VrfGet(uint64_t vrf_id);

  uint64_t VrfCreate(uint64_t vrf_id);

  /* Uplink APIs */
  int UplinkProbe();

  uint64_t UplinkGet(uint64_t port_num);

  uint64_t UplinkCreate(uint64_t uplink_if_id,
                        uint64_t port_num,
                        uint64_t native_l2seg_id);

  int UplinkDelete(uint64_t uplink_if_id);

  /* Endpoint NIC APIs */
  uint64_t EnicCreate(uint64_t enic_id,
                      uint64_t lif_id,
                      uint64_t uplink_id,
                      uint64_t native_l2seg_id,
                      vector<uint64_t>& nonnative_l2seg_id);

  int EnicDelete(uint64_t enic_id);

  /* Endpoint APIs */
  int EndpointProbe();

  uint64_t EndpointCreate(uint64_t vrf_id,
                          uint64_t l2seg_id,
                          uint64_t enic_id,
                          uint64_t mac_addr);

  int EndpointDelete(uint64_t vrf_id, uint64_t handle);

  /* Interface APIs */
  int InterfaceProbe();

  int InterfaceDelete(uint64_t if_id);

  /* LIF APIs */
  int LifProbe();

  uint64_t LifGet(uint64_t lif_id, struct lif_info *lif_info);

  uint64_t LifCreate(uint64_t lif_id,
                     struct queue_info* queue_info,
                     struct lif_info *lif_info,
                     bool enable_rdma,
                     uint32_t max_pt_entries,
                     uint32_t max_keys);

  int LifDelete(uint64_t lif_id);

  int LifSetVlanStrip(uint64_t lif_id, bool enable);

  int LifSetVlanInsert(uint64_t lif_id, bool enable);

  int LifSetBroadcast(uint64_t lif_id, bool enable);

  int LifSetAllMulticast(uint64_t lif_id, bool enable);

  int LifSetPromiscuous(uint64_t lif_id, bool enable);

  int LifSetRssConfig(uint64_t lif_id, LifRssType type, string key, string table);

  int PgmBaseAddrGet(const char *prog_name, uint64_t *base_addr);
  int AllocHbmAddress(const char *handle, uint64_t *addr, uint32_t *size);

  /* Multicast APIs */

  // (vrf_id, l2seg_id, group) > enic_id
  map<tuple<uint64_t, uint64_t, uint64_t>, vector<uint64_t>> mcast_groups;

  int MulticastProbe();

  int MulticastGroupGet(uint64_t group,
                        uint64_t vrf_id,
                        uint64_t l2seg_id);

  int MulticastGroupCreate(uint64_t group,
                           uint64_t vrf_id,
                           uint64_t l2seg_id);

  int MulticastGroupUpdate(uint64_t group,
                           uint64_t vrf_id,
                           uint64_t l2seg_id,
                           vector<uint64_t>& oifs_list);

  int MulticastGroupJoin(uint64_t group,
                         uint64_t vrf_id,
                         uint64_t l2seg_id,
                         uint64_t enic_id);

  int MulticastGroupLeave(uint64_t group,
                          uint64_t vrf_id,
                          uint64_t l2seg_id,
                          uint64_t enic_id);

  int MulticastGroupDelete(uint64_t group,
                           uint64_t vrf_id,
                           uint64_t l2seg_id);

  /* RDMA APIs */
  
  int CreateMR(uint64_t lif_id, uint32_t pd, uint64_t va, uint64_t length,
               uint16_t access_flags, uint32_t l_key, uint32_t r_key,
               uint32_t page_size, uint64_t *pt_table, uint32_t pt_size);

  int CreateCQ(uint32_t lif_id,
               uint32_t cq_num, uint16_t cq_wqe_size, uint16_t num_cq_wqes,
               uint32_t host_page_size,
               uint64_t *pt_table, uint32_t pt_size);

  int CreateQP(uint64_t lif_id, uint32_t qp_num, uint16_t sq_wqe_size,
               uint16_t rq_wqe_size, uint16_t num_sq_wqes,
               uint16_t num_rq_wqes, uint16_t num_rsq_wqes,
               uint16_t num_rrq_wqes, uint8_t pd_num,
               uint32_t sq_cq_num, uint32_t rq_cq_num, uint32_t page_size,
               uint32_t pmtu,
               int service,
               uint32_t sq_pt_size,
               uint32_t pt_size, uint64_t *pt_table);

  int ModifyQP(uint64_t lif_id, uint32_t qp_num, uint32_t attr_mask,
               uint32_t dest_qp_num, uint32_t q_key,
               uint32_t e_psn, uint32_t sq_psn,
               uint32_t header_template_ah_id, uint32_t header_template_size,
               unsigned char *header);

  int RDMACreateEQ(uint64_t lif_id, uint32_t eq_num,
                   uint32_t num_eq_wqes, uint32_t eq_wqe_size,
                   uint32_t eqe_base_addr_pa, uint32_t int_num);

  int RDMACreateCQ(uint64_t lif_id,
                   uint32_t cq_num, uint16_t cq_wqe_size, uint16_t num_cq_wqes,
                   uint32_t host_pg_size,
                   uint64_t pa, uint32_t eq_num);

  int RDMACreateAdminQ(uint64_t lif_id, uint32_t aq_num,
                       uint32_t log_num_wqes, uint32_t log_wqe_size,
                       uint64_t va, uint32_t cq_num);

  /* Filter APIs */
  int FilterAdd(uint64_t lif_id, uint64_t mac, uint32_t vlan);
  int FilterDel(uint64_t lif_id, uint64_t mac, uint32_t vlan);

  /* State */
  map<uint64_t, LifSpec> lif_map;               /* lif_id to lif_spec */
  map<uint64_t, uint64_t> lif2enic_map;         /* lif_id to enic_id */

  map<uint64_t, InterfaceSpec> enic_map;        /* enic_id to enic_spec */
  map<uint64_t, vector<uint64_t>> enic2ep_map;  /* enic_id to ep_handle */

  map<uint64_t, uint64_t> uplink2id;            /* uplink_port to uplink_id */
  map<uint32_t, uint64_t> seg2vlan;             /* l2seg_id to vlan_id */
  map<uint32_t, uint64_t> vlan2seg;             /* vlan_id to l2seg_id */

  map<uint64_t, uint64_t> vrf_id2handle;        /* vrf_id to vrf_handle */
  map<uint64_t, uint64_t> enic_id2handle;       /* enic_handle to enic_id */
  map<uint64_t, uint64_t> l2seg_id2handle;      /* l2seg_id to l2seg_handle */

  friend ostream& operator<<(ostream&, const HalClient&);

  enum ForwardingMode fwd_mode;

private:
  shared_ptr<Channel> channel;
  std::unique_ptr<Vrf::Stub> vrf_stub_;
  std::unique_ptr<Interface::Stub> intf_stub_;
  std::unique_ptr<Internal::Stub> internal_stub_;
  std::unique_ptr<Endpoint::Stub> ep_stub_;
  std::unique_ptr<L2Segment::Stub> l2seg_stub_;
  std::unique_ptr<Multicast::Stub> multicast_stub_;
  std::unique_ptr<Rdma::Stub> rdma_stub_;
};

#define   IB_QP_QKEY         (1 << 6)
#define   IB_QP_AV           (1 << 7)
#define   IB_QP_DEST_QPN     (1 << 20)
#define   IB_QP_RQ_PSN       (1 << 12)
#define   IB_QP_SQ_PSN       (1 << 16)
 
#define   AC_LOCAL_WRITE       0x1
#define   AC_REMOTE_WRITE      0x2
#define   AC_REMOTE_READ       0x4
#define   AC_REMOTE_ATOMIC     0x8

#endif /* __HAL_CLIENT_HPP__ */
