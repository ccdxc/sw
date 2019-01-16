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
#include "gen/proto/internal.pb.h"
#include "gen/proto/port.grpc.pb.h"

#include "platform/src/lib/hal_api/include/ethlif.hpp"
#include "platform/src/lib/hal_api/include/qos.hpp"

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
using namespace port;

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
/*
 * Accelerator ring group ring metrics response structure
 */
typedef struct {
    uint32_t    ring_handle;
    uint32_t    sub_ring;
    uint64_t    input_bytes;
    uint64_t    output_bytes;
    uint64_t    soft_resets;
} accel_rgroup_rmetrics_rsp_t;

typedef void (*accel_rgroup_rmetrics_rsp_cb_t)(void *user_ctx,
                                               const accel_rgroup_rmetrics_rsp_t& indices);

/**
 * Port Status information
 */
typedef struct {
    uint32_t    port_id;
    uint32_t    port_speed;
    bool        oper_status;
} port_status_t;

/**
 * Client for interacting with HAL
 */
class HalClient {
public:
  HalClient(enum ForwardingMode fwd_mode);

  ForwardingMode get_fwd_mode() { return this->fwd_mode; }

  uint64_t LifCreate(hal_lif_info_t *hal_lif_info);

  int PgmBaseAddrGet(const char *prog_name, uint64_t *base_addr);

  /* Accel APIs */
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
  int AccelRGroupMetricsGet(const std::string& rgroup_name,
                            uint32_t sub_ring,
                            accel_rgroup_rmetrics_rsp_cb_t rsp_cb_func,
                            void *user_ctx,
                            uint32_t& ret_num_entries);
  int CryptoKeyIndexUpdate(uint32_t key_index,
                            types::CryptoKeyType key_type,
                            void *key,
                            uint32_t key_size);

  /* Port APIs */
  int PortStatusGet(uint32_t portnum, port_status_t &pi);

  /* State */
  map<uint64_t, EthLif*> eth_lif_map;

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
  std::unique_ptr<Internal::Stub> crypto_stub_;
  std::unique_ptr<Port::Stub> port_stub_;
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
