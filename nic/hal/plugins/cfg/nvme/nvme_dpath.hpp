//-----------------------------------------------------------------------------
// {C} Copyright 2017 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------

#ifndef _NVME_DPATH_HPP_
#define _NVME_DPATH_HPP_

#include "nic/include/base.hpp"
#include <memory>
#include <map>
#include "nic/hal/plugins/cfg/nw/interface.hpp"
#include "gen/proto/nvme.pb.h"
#include "nic/include/hal.hpp"
#include "lib/bm_allocator/bm_allocator.hpp"

namespace hal {

#define HOSTMEM_PAGE_SIZE  (1 << 12)  //4096 Bytes
#define MAX_LIFS           2048

#define PACKED __attribute__((__packed__))

#define HBM_PAGE_SIZE 4096
#define HBM_PAGE_SIZE_SHIFT 12
#define HBM_PAGE_ALIGN(x) (((x) + (HBM_PAGE_SIZE - 1)) & \
                           ~(uint64_t)(HBM_PAGE_SIZE - 1))

typedef struct lif_init_attr_s {
    uint32_t max_sq;
    uint32_t max_cq;
    uint32_t max_ns;
    uint32_t max_sess;
} lif_init_attr_t;

//4 Bytes
typedef struct qstate_ring_s {
    uint16_t  c_index;
    uint16_t  p_index;
} qstate_ring_t;

//8 Bytes
typedef struct qstate_intrinsic_s {
    uint16_t  pid;
    uint8_t   host_rings:4; // Swapping host/total nibbles since it is accessed by doorbell-module in Little-Endian.
    uint8_t   total_rings:4;
    uint8_t   eval_last;
    uint8_t   cos_selector;
    uint8_t   cosA:4; // Swapping nibbles cosA/cosB since it is accessed by doorbell-module in Little-Endian.
    uint8_t   cosB:4;
    uint8_t   rsvd;
    uint8_t   pc;
} PACKED qstate_intrinsic_base_t;

/*====================  TYPES.H ===================*/

#define BYTES_TO_BITS(__B) ((__B)*8)
#define BITS_TO_BYTES(__b) ((__b)/8)

#define SQ_RING_ID      RING_ID_0

#define MAX_SQ_RINGS 1
#define MAX_SQ_HOST_RINGS 1

//64 Bytes
typedef struct nvme_sqcb_s {

    uint8_t  pad2[28];

    uint32_t pad: 25;
    uint32_t cq_id:24;
    uint32_t log_num_wqes:5;
    uint32_t log_wqe_size:5;
    uint32_t log_host_page_size:5;

    uint64_t base_addr;

    qstate_ring_t           rings[MAX_SQ_RINGS];

    // intrinsic
    qstate_intrinsic_base_t ring_header;
} PACKED nvme_sqcb_t;


#define CQ_RING_ID      RING_ID_0

#define MAX_CQ_RINGS 1
#define MAX_CQ_HOST_RINGS 1

//64 Bytes
typedef struct nvme_cqcb_s {

    uint8_t  pad2[28];

    uint64_t pad: 51;
    uint64_t cq_full:1;
    uint64_t cq_full_hint:1;
    uint64_t color:1;
    uint64_t log_num_wqes:5;
    uint64_t log_wqe_size:5;

    uint64_t int_assert_addr;
    uint64_t base_addr;

    qstate_ring_t           rings[MAX_CQ_RINGS];

    // intrinsic
    qstate_intrinsic_base_t ring_header;
} PACKED nvme_cqcb_t;

typedef struct nvme_nscb_s {
    uint64_t nsid: 32;
    uint64_t backend_nsid: 32;
    uint64_t sessprodcb_base_addr: 64;
    uint64_t size: 64;
    uint64_t log_lba_size: 8;
    uint64_t rsvd0: 56;
    uint64_t rsvd[4];
} nvme_nscb_t;

typedef struct nvme_sessprodcb_s {
} nvme_sessprodcb_t;

typedef struct nvme_cmd_context_s {
    uint8_t pad[4096];
} nvme_cmd_context_t;

typedef struct nvme_cmd_context_ring_entry_s {
    uint16_t id;
} nvme_cmd_context_ring_entry_t;

}  // namespace hal

#endif // _NVME_DPATH_HPP_
