/*
* Copyright (c) 2018, Pensando Systems Inc.
*/

#ifndef __HAL_CLIENT_HPP__
#define __HAL_CLIENT_HPP__

#include <map>
#include <grpc++/grpc++.h>

#include "gen/proto/types.grpc.pb.h"
#include "gen/proto/vrf.grpc.pb.h"
#include "gen/proto/interface.grpc.pb.h"
#include "gen/proto/endpoint.grpc.pb.h"
#include "gen/proto/l2segment.grpc.pb.h"
#include "gen/proto/multicast.grpc.pb.h"
#include "gen/proto/rdma.grpc.pb.h"
#include "gen/proto/accel_rgroup.grpc.pb.h"
#include "gen/proto/internal.grpc.pb.h"
#include "gen/proto/crypto_keys.pb.h"
#include "gen/proto/crypto_keys.grpc.pb.h"

#include "platform/src/lib/hal_api/include/ethlif.hpp"


using namespace kh;
using namespace types;
using namespace vrf;
using namespace intf;
using namespace internal;
using namespace endpoint;
using namespace l2segment;
using namespace multicast;
using namespace rdma;
using namespace accelRGroup;
using namespace cryptokey;

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
 * Platform Modes
 */
typedef enum platform_s {
    PLATFORM_NONE,
    PLATFORM_SIM,
    PLATFORM_HW,
    PLATFORM_HAPS,
    PLATFORM_RTL,
    PLATFORM_MOCK,
} platform_t;

static inline bool
platform_is_hw(platform_t platform)
{
    return (platform == PLATFORM_HW) || (platform == PLATFORM_HAPS);
}

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
  uint64_t lif_id;
  uint64_t hw_lif_id;
  types::LifType lif_type;
  bool enable_rdma;
  uint64_t qstate_addr[NUM_QUEUE_TYPES];
};

/**
 * Accelerator ring group ring info response structure
 */
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
/*
 * Accelerator ring group ring indices response structure
 */
typedef struct {
    uint32_t    ring_handle;
    uint32_t    sub_ring;
    uint32_t    pndx;
    uint32_t    cndx;
} accel_rgroup_rindices_rsp_t;

typedef void (*accel_rgroup_rindices_rsp_cb_t)(void *user_ctx,
                                               const accel_rgroup_rindices_rsp_t& indices);
/**
 * Client for interacting with HAL
 */
class HalClient {
public:
  HalClient(enum ForwardingMode fwd_mode);

  ForwardingMode get_fwd_mode() { return this->fwd_mode; }

  /* Segment APIs */
  int L2SegmentProbe();

  uint64_t L2SegmentGet(uint64_t l2seg_id);

  uint64_t L2SegmentCreate(uint64_t vrf_id,
                           uint64_t l2seg_id,
                           uint16_t vlan_id);

  int AddL2SegmentOnUplink(uint64_t uplink_id,
                           uint64_t l2seg_id);

  /* VRF APIs */
  int VrfProbe();

  uint64_t VrfGet(uint64_t vrf_id);

  uint64_t VrfCreate(uint64_t vrf_id);

  /* Uplink APIs */
  int UplinkProbe();

  uint64_t UplinkCreate(uint64_t uplink_id,
                        uint64_t port_num,
                        uint64_t native_l2seg_id);

  int UplinkDelete(uint64_t uplink_id);

  /* Endpoint NIC APIs */
  uint64_t EnicCreate(uint64_t enic_id,
                      uint64_t lif_id,
                      uint64_t native_l2seg_id,
                      vector<uint64_t>& nonnative_l2seg_id);

  int EnicDelete(uint64_t enic_id);

  /* Endpoint APIs */
  int EndpointProbe();

  uint64_t EndpointCreate(uint64_t vrf_id,
                          uint64_t l2seg_id,
                          uint64_t enic_id,
                          uint64_t mac_addr);

  int EndpointDelete(uint64_t vrf_id,
                     uint64_t l2seg_id,
                     uint64_t enic_id,
                     uint64_t mac_addr);

  /* Interface APIs */
  int InterfaceProbe();

  int InterfaceDelete(uint64_t if_id);

  /* LIF APIs */
  int LifProbe();

  uint64_t LifGet(uint64_t lif_id, struct lif_info *lif_info);

  uint64_t LifCreate(uint64_t lif_id,
                     struct queue_info* queue_info,
                     struct lif_info *lif_info,
                     uint64_t uplink_id,
                     bool enable_rdma,
                     uint32_t max_pt_entries,
                     uint32_t max_keys,
                     uint32_t max_ahs,
                     uint32_t hw_lif_id = 0);

  uint64_t LifCreate(uint64_t lif_id,
                     Uplink *uplink,
                     struct queue_info *queue_info,
                     struct lif_info *lif_info);

  uint64_t LifDelete(uint64_t lif_id);

  int LifSetVlanOffload(uint64_t lif_id, bool strip, bool insert);

  int LifSetFilterMode(uint64_t lif_id, bool bcast, bool all_mc, bool promisc);

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
               uint16_t num_rrq_wqes, uint32_t pd_num,
               uint32_t sq_cq_num, uint32_t rq_cq_num, uint32_t page_size,
               uint32_t pmtu,
               int service, int flags,
               uint32_t sq_pt_size,
               uint32_t pt_size, uint64_t *pt_table,
               uint64_t cmb_bar_base, uint64_t cmb_bar_size);

  int ModifyQP(uint64_t lif_id, uint32_t qp_num, uint32_t attr_mask,
               uint32_t dest_qp_num, uint32_t q_key,
               uint32_t e_psn, uint32_t sq_psn,
               uint32_t header_template_ah_id, uint32_t header_template_size,
               unsigned char *header, uint32_t pmtu, uint8_t qstate);

  int CreateAh(uint64_t lif_id, uint32_t ah_id, uint32_t pd_id,
               uint32_t header_template_size, unsigned char *header);

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
  int AccelRGroupAdd(const std::string& rgroup_name);
  int AccelRGroupDel(const std::string& rgroup_name);
  int AccelRGroupRingAdd(const std::string& rgroup_name,
      const std::vector<std::pair<const std::string,uint32_t>>& ring_vec);
  int AccelRGroupRingDel(const std::string& rgroup_name,
      const std::vector<std::pair<const std::string,uint32_t>>& ring_vec);
  int AccelRGroupResetSet(const std::string& rgroup_name,
                          uint32_t sub_ring,
                          bool reset_sense);
  int AccelRGroupEnableSet(const std::string& rgroup_name,
                           uint32_t sub_ring,
                           bool enable_sense);
  int AccelRGroupPndxSet(const std::string& rgroup_name,
                         uint32_t sub_ring,
                         uint32_t val,
                         bool conditional);
  int AccelRGroupInfoGet(const std::string& rgroup_name,
                          uint32_t sub_ring,
                          accel_rgroup_rinfo_rsp_cb_t rsp_cb_func,
                          void *user_ctx,
                          uint32_t& ret_num_entries);
  int AccelRGroupIndicesGet(const std::string& rgroup_name,
                            uint32_t sub_ring,
                            accel_rgroup_rindices_rsp_cb_t rsp_cb_func,
                            void *user_ctx,
                            uint32_t& ret_num_entries);
  int crypto_key_index_update(uint32_t key_index,
                              types::CryptoKeyType key_type,
                              void *key,
                              uint32_t key_size);

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

  map<uint64_t, EthLif*> eth_lif_map;


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
  std::unique_ptr<AccelRGroup::Stub> accel_rgroup_stub_;
  std::unique_ptr<CryptoKey::Stub> crypto_stub_;
};

#define   IB_QP_STATE        (1 << 0)
#define   IB_QP_QKEY         (1 << 6)
#define   IB_QP_AV           (1 << 7)
#define   IB_QP_PATH_MTU     (1 << 8)
#define   IB_QP_DEST_QPN     (1 << 20)
#define   IB_QP_RQ_PSN       (1 << 12)
#define   IB_QP_SQ_PSN       (1 << 16)

#define   AC_LOCAL_WRITE       0x1
#define   AC_REMOTE_WRITE      0x2
#define   AC_REMOTE_READ       0x4
#define   AC_REMOTE_ATOMIC     0x8

#endif /* __HAL_CLIENT_HPP__ */
