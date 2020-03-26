//-----------------------------------------------------------------------------
// {C} Copyright 2020 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------


#include <stdio.h>
#include <string>
#include <errno.h>
#include <stdlib.h>
#include <assert.h>
#include <map>
#include <cmath>

#include "include/sdk/base.hpp"
#include "include/sdk/mem.hpp"
#include "include/sdk/lock.hpp"
#include "platform/elba/elba_p4.hpp"
#include "platform/elba/elba_tm_rw.hpp"
#include "platform/elba/elba_state.hpp"
#include "lib/bitmap/bitmap.hpp"
#include "gen/platform/mem_regions.hpp"

#include "third-party/asic/elba/model/utils/elb_blk_reg_model.h"
#include "third-party/asic/elba/model/elb_top/elb_top_csr.h"
#include "third-party/asic/elba/model/elb_pb/elb_pbc_csr.h"
#include "third-party/asic/elba/verif/apis/elb_pb_api.h"
#include "third-party/asic/elba/model/elb_pb/elb_pbc_decoders.h"
#include "third-party/asic/elba/design/common/gen/elb_pf_decoders.h"

namespace sdk {
namespace platform {
namespace elba {

static cpp_int_helper cpp_helper;

qos_profile_t qos_profile = {true, true, 9216, 8, 25, 27, 16, 2, {0, 24}};

typedef struct elba_tm_cfg_profile_s {
    uint32_t num_qs[NUM_TM_PORT_TYPES];
    uint32_t jumbo_mtu[NUM_TM_PORT_TYPES];
    uint32_t num_active_uplink_ports;
    uint64_t hbm_fifo_base;
    uint32_t hbm_fifo_size;
    bool     sw_init_enabled;
    bool     sw_cfg_write_enabled;
    tm_q_t   *p4_high_perf_qs;
    uint32_t num_p4_high_perf_qs;
} elba_tm_cfg_profile_t;

typedef struct elba_tm_port_asic_profile_s {
    uint32_t reserved_mtus;
    uint32_t headroom_cells;
    uint32_t recirc_q;
    bool     uses_credits;
} elba_tm_asic_port_profile_t;

typedef struct elba_tm_asic_profile_s {
    uint32_t                     cell_alloc_units;
    uint32_t                     hbm_fifo_alloc_units;
    uint32_t                     hbm_fifo_control_scale_factor;
    elba_tm_asic_port_profile_t port[NUM_TM_PORT_TYPES];
    uint32_t                     hbm_fifo_reserved_bytes_per_context[NUM_TM_HBM_FIFO_TYPES];
    uint32_t                     cpu_copy_tail_drop_threshold;
    uint32_t                     span_tail_drop_threshold;
} elba_tm_asic_profile_t;

typedef struct elba_tm_buf_hbm_cfg_s {
    bool valid;
    uint64_t payload_offset;
    uint64_t control_offset;
    uint32_t payload_chunks;
    uint32_t control_chunks;
} elba_tm_buf_hbm_cfg_t;

typedef struct elba_tm_buf_cfgs_s {
    uint32_t               chunks_per_q[NUM_TM_PORT_TYPES];
    elba_tm_buf_hbm_cfg_t hbm_fifo[NUM_TM_HBM_FIFO_TYPES][ELBA_TM_MAX_HBM_CONTEXTS];
} elba_tm_buf_cfg_t;

typedef struct elba_tm_hbm_context_stats_s {
    uint64_t good_pkts_in;
    uint64_t good_pkts_out;
    uint64_t errored_pkts_in;
    uint32_t max_oflow_fifo_depth;
} elba_tm_hbm_context_stats_t;

typedef struct elba_tm_shadow_stats_s {
    sdk_spinlock_t slock; // Lock for accessing the stats
    elba_tm_hbm_context_stats_t cur_vals[NUM_TM_HBM_FIFO_TYPES][ELBA_TM_MAX_HBM_CONTEXTS];
    elba_tm_hbm_context_stats_t prev_vals[NUM_TM_HBM_FIFO_TYPES][ELBA_TM_MAX_HBM_CONTEXTS];
} elba_tm_shadow_stats_t;

typedef struct elba_tm_ctx_s {
    elba_tm_shadow_stats_t stats;
    elba_tm_asic_profile_t asic_profile;
    elba_tm_cfg_profile_t  cfg_profile;
    elba_tm_buf_cfg_t      buf_cfg;
    std::atomic<bool>       init_complete;
} elba_tm_ctx_t;

elba_tm_ctx_t g_tm_ctx_;
elba_tm_ctx_t *g_tm_ctx;

static void
set_tm_ctx (elba_tm_cfg_profile_t *tm_cfg_profile,
            elba_tm_asic_profile_t *asic_profile)
{
    if (!g_tm_ctx) {
        g_tm_ctx_.cfg_profile = *tm_cfg_profile;
        g_tm_ctx_.asic_profile = *asic_profile;
        SDK_SPINLOCK_INIT(&g_tm_ctx_.stats.slock, PTHREAD_PROCESS_PRIVATE);
        g_tm_ctx = &g_tm_ctx_;
    }
}

static elba_tm_ctx_t *
tm_ctx (void)
{
    return g_tm_ctx;
}

static elba_tm_asic_profile_t *
tm_asic_profile (void)
{
    return &tm_ctx()->asic_profile;
}

static elba_tm_cfg_profile_t *
tm_cfg_profile (void)
{
    return &tm_ctx()->cfg_profile;
}

static inline bool
tm_sw_init_enabled (void)
{
    return tm_cfg_profile()->sw_init_enabled;
}

static inline bool
tm_sw_cfg_write_enabled (void)
{
    return tm_cfg_profile()->sw_cfg_write_enabled;
}

static void
populate_asic_profile (elba_tm_asic_profile_t *asic_profile)
{
    // These are values based on performance numbers seen during rtl simulation
    // When reserved_mtus is zero, it indicates that allocate whatever is left
    memset(asic_profile, 0, sizeof(*asic_profile));
    asic_profile->cell_alloc_units = 4;
    asic_profile->hbm_fifo_alloc_units = ELBA_TM_HBM_FIFO_ALLOC_SIZE;
    asic_profile->hbm_fifo_control_scale_factor = 50;
    asic_profile->cpu_copy_tail_drop_threshold = 900;
    asic_profile->span_tail_drop_threshold = 900;

    asic_profile->port[TM_PORT_TYPE_UPLINK].reserved_mtus = 0;
    asic_profile->port[TM_PORT_TYPE_UPLINK].headroom_cells = 100;

    asic_profile->port[TM_PORT_TYPE_P4IG].reserved_mtus = 3;
    asic_profile->port[TM_PORT_TYPE_P4IG].headroom_cells = 0;
    asic_profile->port[TM_PORT_TYPE_P4IG].uses_credits = true;
    asic_profile->port[TM_PORT_TYPE_P4IG].recirc_q = TM_P4_RECIRC_QUEUE;

    asic_profile->port[TM_PORT_TYPE_P4EG].reserved_mtus = 3;
    asic_profile->port[TM_PORT_TYPE_P4EG].headroom_cells = 0;
    asic_profile->port[TM_PORT_TYPE_P4EG].uses_credits = true;
    asic_profile->port[TM_PORT_TYPE_P4EG].recirc_q = TM_P4_RECIRC_QUEUE;

    asic_profile->port[TM_PORT_TYPE_DMA].reserved_mtus = 0;
    asic_profile->port[TM_PORT_TYPE_DMA].headroom_cells = 100;

    asic_profile->hbm_fifo_reserved_bytes_per_context[TM_HBM_FIFO_TYPE_UPLINK] = 3*1024*1024; // 3MB
    asic_profile->hbm_fifo_reserved_bytes_per_context[TM_HBM_FIFO_TYPE_TXDMA] = 9*1024*1024;; // 9MB
}

static inline uint32_t
bytes_to_cells (uint32_t bytes)
{
    return (bytes + ELBA_TM_CELL_SIZE - 1)/ELBA_TM_CELL_SIZE;
}

static inline uint32_t
cells_to_bytes (uint32_t cells)
{
    return cells * ELBA_TM_CELL_SIZE;
}

static inline uint32_t
cells_to_chunks (uint32_t cells)
{
    return (cells + tm_asic_profile()->cell_alloc_units - 1)/
        tm_asic_profile()->cell_alloc_units;
}

static inline uint32_t
chunks_to_cells (uint32_t chunks)
{
    return chunks * tm_asic_profile()->cell_alloc_units;
}

static inline uint32_t
hbm_bytes_to_chunks (uint32_t bytes)
{
    return (bytes + tm_asic_profile()->hbm_fifo_alloc_units - 1)/
        tm_asic_profile()->hbm_fifo_alloc_units;
}

static inline uint32_t
hbm_chunks_to_bytes (uint32_t chunks)
{
    return chunks * tm_asic_profile()->hbm_fifo_alloc_units;
}

static inline uint32_t
elba_tm_get_max_cell_chunks_for_island (uint32_t island)
{
    uint32_t cells = 0;
    SDK_ASSERT(island < ELBA_TM_NUM_BUFFER_ISLANDS);
    if (tm_cfg_profile()->num_active_uplink_ports > 2) {
        island = ELBA_TM_NUM_BUFFER_ISLANDS - island - 1;
    }
    if (island == 0) {
        cells = ELBA_TM_BUFFER_ISLAND_0_CELL_COUNT;
    } else if (island == 1) {
        cells = ELBA_TM_BUFFER_ISLAND_1_CELL_COUNT;
    }
    return cells_to_chunks(cells);
}

static inline tm_port_type_e
elba_tm_get_port_type (tm_port_t port)
{
    switch(port) {
        case ELBA_TM_PORT_UPLINK_0:
        case ELBA_TM_PORT_UPLINK_1:
        case ELBA_TM_PORT_UPLINK_2:
        case ELBA_TM_PORT_UPLINK_3:
        case ELBA_TM_PORT_NCSI:
            return TM_PORT_TYPE_UPLINK;
        case ELBA_TM_PORT_DMA:
            return TM_PORT_TYPE_DMA;
        case ELBA_TM_PORT_EGRESS:
            return TM_PORT_TYPE_P4EG;
        case ELBA_TM_PORT_INGRESS:
            return TM_PORT_TYPE_P4IG;
    }
    return NUM_TM_PORT_TYPES;
}

static inline bool
is_active_uplink_port (tm_port_t port)
{
    if (port < tm_cfg_profile()->num_active_uplink_ports) {
        return true;
    }
    return false;
}

static inline bool
is_active_port (tm_port_t port)
{
    tm_port_type_e port_type;

    port_type = elba_tm_get_port_type(port);
    if (port_type == TM_PORT_TYPE_UPLINK) {
        return (is_active_uplink_port(port) ||
                (port == ELBA_TM_PORT_NCSI));
    }
    return true;
}

static inline uint32_t
elba_tm_max_hbm_contexts_for_fifo (uint32_t fifo_type)
{
    switch (fifo_type) {
        case TM_HBM_FIFO_TYPE_UPLINK:
            return ELBA_TM_MAX_HBM_ETH_CONTEXTS;
        case TM_HBM_FIFO_TYPE_TXDMA:
            return ELBA_TM_MAX_HBM_DMA_CONTEXTS;
        case NUM_TM_HBM_FIFO_TYPES:
            return 0;
    }
    return 0;
}

static inline bool
port_supports_hbm_contexts (tm_port_t port)
{
    return is_active_uplink_port(port) || (port == ELBA_TM_PORT_DMA);
}

static inline uint32_t
elba_tm_get_num_iqs_for_port_type (tm_port_type_e port_type)
{
    switch(port_type) {
        case TM_PORT_TYPE_UPLINK:
            return 8;
        case TM_PORT_TYPE_P4IG:
        case TM_PORT_TYPE_P4EG:
            return 32;
        case TM_PORT_TYPE_DMA:
            return 16;
        case NUM_TM_PORT_TYPES:
            return 0;
    }
    return 0;
}

uint32_t
elba_tm_get_num_iqs_for_port (tm_port_t port)
{
    return elba_tm_get_num_iqs_for_port_type(elba_tm_get_port_type(port));
}

static inline uint32_t
elba_tm_get_num_oqs_for_port_type (tm_port_type_e port_type)
{
    switch(port_type) {
        case TM_PORT_TYPE_UPLINK:
            return 16;
        case TM_PORT_TYPE_P4IG:
        case TM_PORT_TYPE_P4EG:
        case TM_PORT_TYPE_DMA:
            return 32;
        case NUM_TM_PORT_TYPES:
            return 0;
    }
    return 0;
}

uint32_t
elba_tm_get_num_oqs_for_port (tm_port_t port)
{
    return elba_tm_get_num_oqs_for_port_type(elba_tm_get_port_type(port));
}

static inline uint32_t
elba_tm_get_island_for_port_type (tm_port_type_e port_type)
{
    switch(port_type) {
        case TM_PORT_TYPE_UPLINK:
        case TM_PORT_TYPE_P4EG:
            return 1;
        case TM_PORT_TYPE_P4IG:
        case TM_PORT_TYPE_DMA:
            return 0;
        case NUM_TM_PORT_TYPES:
            return 0;
    }

    return 0;
}

static inline tm_hbm_fifo_type_e
elba_tm_get_fifo_type_for_port (tm_port_t port)
{
    switch(elba_tm_get_port_type(port)) {
        case TM_PORT_TYPE_UPLINK:
            return TM_HBM_FIFO_TYPE_UPLINK;
        case TM_PORT_TYPE_DMA:
            return TM_HBM_FIFO_TYPE_TXDMA;
        case TM_PORT_TYPE_P4EG:
        case TM_PORT_TYPE_P4IG:
        case NUM_TM_PORT_TYPES:
            return NUM_TM_HBM_FIFO_TYPES;
    }
    return NUM_TM_HBM_FIFO_TYPES;
}

static inline bool
tm_is_high_perf_q (tm_q_t q)
{
    for (unsigned int i = 0; i < tm_cfg_profile()->num_p4_high_perf_qs; i++) {
        if (q == tm_cfg_profile()->p4_high_perf_qs[i]) {
            return true;
        }
    }
    return false;
}


void
elba_tm_dump_debug_regs (void)
{
    elb_top_csr_t &elb0 = ELB_BLK_REG_MODEL_ACCESS(elb_top_csr_t, 0, 0);
    elb_pbc_csr_t &pbc_csr = elb0.pb.pbc;
    stringstream data;
    data << hex << endl;
    pbc_csr.cnt_axi_timestamp.read();
    data <<"pbc_csr.cnt_axi_timestamp.all: 0x" << pbc_csr.cnt_axi_timestamp.all() << endl;
data <<"pbc_csr.cnt_axi_timestamp.write: 0x" << pbc_csr.cnt_axi_timestamp.write() << endl;

    pbc_csr.cnt_flush[0].read();
    data <<"pbc_csr.cnt_flush[0].all: 0x" << pbc_csr.cnt_flush[0].all() << endl;
data <<"pbc_csr.cnt_flush[0].packets: 0x" << pbc_csr.cnt_flush[0].packets() << endl;

    pbc_csr.cnt_flush[1].read();
    data <<"pbc_csr.cnt_flush[1].all: 0x" << pbc_csr.cnt_flush[1].all() << endl;
data <<"pbc_csr.cnt_flush[1].packets: 0x" << pbc_csr.cnt_flush[1].packets() << endl;

    pbc_csr.cnt_flush[2].read();
    data <<"pbc_csr.cnt_flush[2].all: 0x" << pbc_csr.cnt_flush[2].all() << endl;
data <<"pbc_csr.cnt_flush[2].packets: 0x" << pbc_csr.cnt_flush[2].packets() << endl;

    pbc_csr.cnt_flush[3].read();
    data <<"pbc_csr.cnt_flush[3].all: 0x" << pbc_csr.cnt_flush[3].all() << endl;
data <<"pbc_csr.cnt_flush[3].packets: 0x" << pbc_csr.cnt_flush[3].packets() << endl;

    pbc_csr.cnt_flush[4].read();
    data <<"pbc_csr.cnt_flush[4].all: 0x" << pbc_csr.cnt_flush[4].all() << endl;
data <<"pbc_csr.cnt_flush[4].packets: 0x" << pbc_csr.cnt_flush[4].packets() << endl;

    pbc_csr.cnt_flush[5].read();
    data <<"pbc_csr.cnt_flush[5].all: 0x" << pbc_csr.cnt_flush[5].all() << endl;
data <<"pbc_csr.cnt_flush[5].packets: 0x" << pbc_csr.cnt_flush[5].packets() << endl;

    pbc_csr.cnt_flush[6].read();
    data <<"pbc_csr.cnt_flush[6].all: 0x" << pbc_csr.cnt_flush[6].all() << endl;
data <<"pbc_csr.cnt_flush[6].packets: 0x" << pbc_csr.cnt_flush[6].packets() << endl;

    pbc_csr.cnt_flush[7].read();
    data <<"pbc_csr.cnt_flush[7].all: 0x" << pbc_csr.cnt_flush[7].all() << endl;
data <<"pbc_csr.cnt_flush[7].packets: 0x" << pbc_csr.cnt_flush[7].packets() << endl;

    pbc_csr.pbccache.sta_ctl_pcache.read();
    data <<"pbc_csr.pbccache.sta_ctl_pcache.all: 0x" << pbc_csr.pbccache.sta_ctl_pcache.all() << endl;
data <<"pbc_csr.pbccache.sta_ctl_pcache.pending_rcount: 0x" << pbc_csr.pbccache.sta_ctl_pcache.pending_rcount() << endl;
data <<"pbc_csr.pbccache.sta_ctl_pcache.pending_wcount: 0x" << pbc_csr.pbccache.sta_ctl_pcache.pending_wcount() << endl;

    pbc_csr.pbccache.sta_pcache.read();
    data <<"pbc_csr.pbccache.sta_pcache.all: 0x" << pbc_csr.pbccache.sta_pcache.all() << endl;
data <<"pbc_csr.pbccache.sta_pcache.bist_done_pass: 0x" << pbc_csr.pbccache.sta_pcache.bist_done_pass() << endl;
data <<"pbc_csr.pbccache.sta_pcache.bist_done_fail: 0x" << pbc_csr.pbccache.sta_pcache.bist_done_fail() << endl;

    pbc_csr.pbcdesc_0.sta_ecc_desc.read();
    data <<"pbc_csr.pbcdesc_0.sta_ecc_desc.all: 0x" << pbc_csr.pbcdesc_0.sta_ecc_desc.all() << endl;
data <<"pbc_csr.pbcdesc_0.sta_ecc_desc.addr: 0x" << pbc_csr.pbcdesc_0.sta_ecc_desc.addr() << endl;
data <<"pbc_csr.pbcdesc_0.sta_ecc_desc.uncorrectable: 0x" << pbc_csr.pbcdesc_0.sta_ecc_desc.uncorrectable() << endl;
data <<"pbc_csr.pbcdesc_0.sta_ecc_desc.correctable: 0x" << pbc_csr.pbcdesc_0.sta_ecc_desc.correctable() << endl;
data <<"pbc_csr.pbcdesc_0.sta_ecc_desc.syndrome: 0x" << pbc_csr.pbcdesc_0.sta_ecc_desc.syndrome() << endl;

    pbc_csr.pbcdesc_1.sta_ecc_desc.read();
    data <<"pbc_csr.pbcdesc_1.sta_ecc_desc.all: 0x" << pbc_csr.pbcdesc_1.sta_ecc_desc.all() << endl;
data <<"pbc_csr.pbcdesc_1.sta_ecc_desc.addr: 0x" << pbc_csr.pbcdesc_1.sta_ecc_desc.addr() << endl;
data <<"pbc_csr.pbcdesc_1.sta_ecc_desc.uncorrectable: 0x" << pbc_csr.pbcdesc_1.sta_ecc_desc.uncorrectable() << endl;
data <<"pbc_csr.pbcdesc_1.sta_ecc_desc.correctable: 0x" << pbc_csr.pbcdesc_1.sta_ecc_desc.correctable() << endl;
data <<"pbc_csr.pbcdesc_1.sta_ecc_desc.syndrome: 0x" << pbc_csr.pbcdesc_1.sta_ecc_desc.syndrome() << endl;

    pbc_csr.pbcll_0.sta_ecc_ll.read();
    data <<"pbc_csr.pbcll_0.sta_ecc_ll.all: 0x" << pbc_csr.pbcll_0.sta_ecc_ll.all() << endl;
data <<"pbc_csr.pbcll_0.sta_ecc_ll.addr: 0x" << pbc_csr.pbcll_0.sta_ecc_ll.addr() << endl;
data <<"pbc_csr.pbcll_0.sta_ecc_ll.uncorrectable: 0x" << pbc_csr.pbcll_0.sta_ecc_ll.uncorrectable() << endl;
data <<"pbc_csr.pbcll_0.sta_ecc_ll.correctable: 0x" << pbc_csr.pbcll_0.sta_ecc_ll.correctable() << endl;
data <<"pbc_csr.pbcll_0.sta_ecc_ll.syndrome: 0x" << pbc_csr.pbcll_0.sta_ecc_ll.syndrome() << endl;

    pbc_csr.pbcll_1.sta_ecc_ll.read();
    data <<"pbc_csr.pbcll_1.sta_ecc_ll.all: 0x" << pbc_csr.pbcll_1.sta_ecc_ll.all() << endl;
data <<"pbc_csr.pbcll_1.sta_ecc_ll.addr: 0x" << pbc_csr.pbcll_1.sta_ecc_ll.addr() << endl;
data <<"pbc_csr.pbcll_1.sta_ecc_ll.uncorrectable: 0x" << pbc_csr.pbcll_1.sta_ecc_ll.uncorrectable() << endl;
data <<"pbc_csr.pbcll_1.sta_ecc_ll.correctable: 0x" << pbc_csr.pbcll_1.sta_ecc_ll.correctable() << endl;
data <<"pbc_csr.pbcll_1.sta_ecc_ll.syndrome: 0x" << pbc_csr.pbcll_1.sta_ecc_ll.syndrome() << endl;

    pbc_csr.pbcsched.sta_sched.read();
    data <<"pbc_csr.pbcsched.sta_sched.all: 0x" << pbc_csr.pbcsched.sta_sched.all() << endl;
data <<"pbc_csr.pbcsched.sta_sched.ecc_correctable: 0x" << pbc_csr.pbcsched.sta_sched.ecc_correctable() << endl;
data <<"pbc_csr.pbcsched.sta_sched.bist_done_pass: 0x" << pbc_csr.pbcsched.sta_sched.bist_done_pass() << endl;
data <<"pbc_csr.pbcsched.sta_sched.bist_done_fail: 0x" << pbc_csr.pbcsched.sta_sched.bist_done_fail() << endl;
data <<"pbc_csr.pbcsched.sta_sched.ecc_uncorrectable: 0x" << pbc_csr.pbcsched.sta_sched.ecc_uncorrectable() << endl;
data <<"pbc_csr.pbcsched.sta_sched.ecc_syndrome: 0x" << pbc_csr.pbcsched.sta_sched.ecc_syndrome() << endl;
data <<"pbc_csr.pbcsched.sta_sched.ecc_addr: 0x" << pbc_csr.pbcsched.sta_sched.ecc_addr() << endl;

    pbc_csr.port_0.cnt_flits.read();
    data <<"pbc_csr.port_0.cnt_flits.all: 0x" << pbc_csr.port_0.cnt_flits.all() << endl;
data <<"pbc_csr.port_0.cnt_flits.eop_in: 0x" << pbc_csr.port_0.cnt_flits.eop_in() << endl;
data <<"pbc_csr.port_0.cnt_flits.srdy_no_drdy_in: 0x" << pbc_csr.port_0.cnt_flits.srdy_no_drdy_in() << endl;
data <<"pbc_csr.port_0.cnt_flits.sop_out: 0x" << pbc_csr.port_0.cnt_flits.sop_out() << endl;
data <<"pbc_csr.port_0.cnt_flits.eop_out: 0x" << pbc_csr.port_0.cnt_flits.eop_out() << endl;
data <<"pbc_csr.port_0.cnt_flits.sop_in: 0x" << pbc_csr.port_0.cnt_flits.sop_in() << endl;
data <<"pbc_csr.port_0.cnt_flits.srdy_no_drdy_out: 0x" << pbc_csr.port_0.cnt_flits.srdy_no_drdy_out() << endl;

    pbc_csr.port_0.cnt_write_error.read();
    data <<"pbc_csr.port_0.cnt_write_error.all: 0x" << pbc_csr.port_0.cnt_write_error.all() << endl;
data <<"pbc_csr.port_0.cnt_write_error.intrinsic_drop: 0x" << pbc_csr.port_0.cnt_write_error.intrinsic_drop() << endl;
data <<"pbc_csr.port_0.cnt_write_error.out_of_cells1: 0x" << pbc_csr.port_0.cnt_write_error.out_of_cells1() << endl;
data <<"pbc_csr.port_0.cnt_write_error.out_of_credit: 0x" << pbc_csr.port_0.cnt_write_error.out_of_credit() << endl;
data <<"pbc_csr.port_0.cnt_write_error.out_of_cells: 0x" << pbc_csr.port_0.cnt_write_error.out_of_cells() << endl;
data <<"pbc_csr.port_0.cnt_write_error.port_disabled: 0x" << pbc_csr.port_0.cnt_write_error.port_disabled() << endl;
data <<"pbc_csr.port_0.cnt_write_error.admitted: 0x" << pbc_csr.port_0.cnt_write_error.admitted() << endl;
data <<"pbc_csr.port_0.cnt_write_error.discarded: 0x" << pbc_csr.port_0.cnt_write_error.discarded() << endl;
data <<"pbc_csr.port_0.cnt_write_error.enqueue: 0x" << pbc_csr.port_0.cnt_write_error.enqueue() << endl;
data <<"pbc_csr.port_0.cnt_write_error.oq_range: 0x" << pbc_csr.port_0.cnt_write_error.oq_range() << endl;
data <<"pbc_csr.port_0.cnt_write_error.tail_drop_span: 0x" << pbc_csr.port_0.cnt_write_error.tail_drop_span() << endl;
data <<"pbc_csr.port_0.cnt_write_error.port_range: 0x" << pbc_csr.port_0.cnt_write_error.port_range() << endl;
data <<"pbc_csr.port_0.cnt_write_error.tail_drop_cpu: 0x" << pbc_csr.port_0.cnt_write_error.tail_drop_cpu() << endl;
data <<"pbc_csr.port_0.cnt_write_error.no_dest: 0x" << pbc_csr.port_0.cnt_write_error.no_dest() << endl;
data <<"pbc_csr.port_0.cnt_write_error.min_size_viol: 0x" << pbc_csr.port_0.cnt_write_error.min_size_viol() << endl;
data <<"pbc_csr.port_0.cnt_write_error.truncation: 0x" << pbc_csr.port_0.cnt_write_error.truncation() << endl;

    pbc_csr.port_0.sta_account.read();
    data <<"pbc_csr.port_0.sta_account.all: 0x" << pbc_csr.port_0.sta_account.all() << endl;
data <<"pbc_csr.port_0.sta_account.occupancy_5: 0x" << pbc_csr.port_0.sta_account.occupancy_5() << endl;
data <<"pbc_csr.port_0.sta_account.occupancy_4: 0x" << pbc_csr.port_0.sta_account.occupancy_4() << endl;
data <<"pbc_csr.port_0.sta_account.occupancy_7: 0x" << pbc_csr.port_0.sta_account.occupancy_7() << endl;
data <<"pbc_csr.port_0.sta_account.occupancy_6: 0x" << pbc_csr.port_0.sta_account.occupancy_6() << endl;
data <<"pbc_csr.port_0.sta_account.occupancy_1: 0x" << pbc_csr.port_0.sta_account.occupancy_1() << endl;
data <<"pbc_csr.port_0.sta_account.occupancy_0: 0x" << pbc_csr.port_0.sta_account.occupancy_0() << endl;
data <<"pbc_csr.port_0.sta_account.occupancy_3: 0x" << pbc_csr.port_0.sta_account.occupancy_3() << endl;
data <<"pbc_csr.port_0.sta_account.occupancy_2: 0x" << pbc_csr.port_0.sta_account.occupancy_2() << endl;
data <<"pbc_csr.port_0.sta_account.sp_held_5: 0x" << pbc_csr.port_0.sta_account.sp_held_5() << endl;
data <<"pbc_csr.port_0.sta_account.sp_held_4: 0x" << pbc_csr.port_0.sta_account.sp_held_4() << endl;
data <<"pbc_csr.port_0.sta_account.sp_held_7: 0x" << pbc_csr.port_0.sta_account.sp_held_7() << endl;
data <<"pbc_csr.port_0.sta_account.sp_held_6: 0x" << pbc_csr.port_0.sta_account.sp_held_6() << endl;
data <<"pbc_csr.port_0.sta_account.sp_held_1: 0x" << pbc_csr.port_0.sta_account.sp_held_1() << endl;
data <<"pbc_csr.port_0.sta_account.sp_held_0: 0x" << pbc_csr.port_0.sta_account.sp_held_0() << endl;
data <<"pbc_csr.port_0.sta_account.sp_held_3: 0x" << pbc_csr.port_0.sta_account.sp_held_3() << endl;
data <<"pbc_csr.port_0.sta_account.sp_held_2: 0x" << pbc_csr.port_0.sta_account.sp_held_2() << endl;

    pbc_csr.port_0.sta_account_pg_0_watermark.read();
    data <<"pbc_csr.port_0.sta_account_pg_0_watermark.all: 0x" << pbc_csr.port_0.sta_account_pg_0_watermark.all() << endl;
data <<"pbc_csr.port_0.sta_account_pg_0_watermark.shared_cells: 0x" << pbc_csr.port_0.sta_account_pg_0_watermark.shared_cells() << endl;
data <<"pbc_csr.port_0.sta_account_pg_0_watermark.total_cells: 0x" << pbc_csr.port_0.sta_account_pg_0_watermark.total_cells() << endl;

    pbc_csr.port_0.sta_account_pg_1_watermark.read();
    data <<"pbc_csr.port_0.sta_account_pg_1_watermark.all: 0x" << pbc_csr.port_0.sta_account_pg_1_watermark.all() << endl;
data <<"pbc_csr.port_0.sta_account_pg_1_watermark.shared_cells: 0x" << pbc_csr.port_0.sta_account_pg_1_watermark.shared_cells() << endl;
data <<"pbc_csr.port_0.sta_account_pg_1_watermark.total_cells: 0x" << pbc_csr.port_0.sta_account_pg_1_watermark.total_cells() << endl;

    pbc_csr.port_0.sta_account_pg_2_watermark.read();
    data <<"pbc_csr.port_0.sta_account_pg_2_watermark.all: 0x" << pbc_csr.port_0.sta_account_pg_2_watermark.all() << endl;
data <<"pbc_csr.port_0.sta_account_pg_2_watermark.shared_cells: 0x" << pbc_csr.port_0.sta_account_pg_2_watermark.shared_cells() << endl;
data <<"pbc_csr.port_0.sta_account_pg_2_watermark.total_cells: 0x" << pbc_csr.port_0.sta_account_pg_2_watermark.total_cells() << endl;

    pbc_csr.port_0.sta_account_pg_3_watermark.read();
    data <<"pbc_csr.port_0.sta_account_pg_3_watermark.all: 0x" << pbc_csr.port_0.sta_account_pg_3_watermark.all() << endl;
data <<"pbc_csr.port_0.sta_account_pg_3_watermark.shared_cells: 0x" << pbc_csr.port_0.sta_account_pg_3_watermark.shared_cells() << endl;
data <<"pbc_csr.port_0.sta_account_pg_3_watermark.total_cells: 0x" << pbc_csr.port_0.sta_account_pg_3_watermark.total_cells() << endl;

    pbc_csr.port_0.sta_account_pg_4_watermark.read();
    data <<"pbc_csr.port_0.sta_account_pg_4_watermark.all: 0x" << pbc_csr.port_0.sta_account_pg_4_watermark.all() << endl;
data <<"pbc_csr.port_0.sta_account_pg_4_watermark.shared_cells: 0x" << pbc_csr.port_0.sta_account_pg_4_watermark.shared_cells() << endl;
data <<"pbc_csr.port_0.sta_account_pg_4_watermark.total_cells: 0x" << pbc_csr.port_0.sta_account_pg_4_watermark.total_cells() << endl;

    pbc_csr.port_0.sta_account_pg_5_watermark.read();
    data <<"pbc_csr.port_0.sta_account_pg_5_watermark.all: 0x" << pbc_csr.port_0.sta_account_pg_5_watermark.all() << endl;
data <<"pbc_csr.port_0.sta_account_pg_5_watermark.shared_cells: 0x" << pbc_csr.port_0.sta_account_pg_5_watermark.shared_cells() << endl;
data <<"pbc_csr.port_0.sta_account_pg_5_watermark.total_cells: 0x" << pbc_csr.port_0.sta_account_pg_5_watermark.total_cells() << endl;

    pbc_csr.port_0.sta_account_pg_6_watermark.read();
    data <<"pbc_csr.port_0.sta_account_pg_6_watermark.all: 0x" << pbc_csr.port_0.sta_account_pg_6_watermark.all() << endl;
data <<"pbc_csr.port_0.sta_account_pg_6_watermark.shared_cells: 0x" << pbc_csr.port_0.sta_account_pg_6_watermark.shared_cells() << endl;
data <<"pbc_csr.port_0.sta_account_pg_6_watermark.total_cells: 0x" << pbc_csr.port_0.sta_account_pg_6_watermark.total_cells() << endl;

    pbc_csr.port_0.sta_account_pg_7_watermark.read();
    data <<"pbc_csr.port_0.sta_account_pg_7_watermark.all: 0x" << pbc_csr.port_0.sta_account_pg_7_watermark.all() << endl;
data <<"pbc_csr.port_0.sta_account_pg_7_watermark.shared_cells: 0x" << pbc_csr.port_0.sta_account_pg_7_watermark.shared_cells() << endl;
data <<"pbc_csr.port_0.sta_account_pg_7_watermark.total_cells: 0x" << pbc_csr.port_0.sta_account_pg_7_watermark.total_cells() << endl;

    pbc_csr.port_0.sta_write.read();
    data <<"pbc_csr.port_0.sta_write.all: 0x" << pbc_csr.port_0.sta_write.all() << endl;
data <<"pbc_csr.port_0.sta_write.cache_entries: 0x" << pbc_csr.port_0.sta_write.cache_entries() << endl;

    pbc_csr.port_1.cnt_flits.read();
    data <<"pbc_csr.port_1.cnt_flits.all: 0x" << pbc_csr.port_1.cnt_flits.all() << endl;
data <<"pbc_csr.port_1.cnt_flits.eop_in: 0x" << pbc_csr.port_1.cnt_flits.eop_in() << endl;
data <<"pbc_csr.port_1.cnt_flits.srdy_no_drdy_in: 0x" << pbc_csr.port_1.cnt_flits.srdy_no_drdy_in() << endl;
data <<"pbc_csr.port_1.cnt_flits.sop_out: 0x" << pbc_csr.port_1.cnt_flits.sop_out() << endl;
data <<"pbc_csr.port_1.cnt_flits.eop_out: 0x" << pbc_csr.port_1.cnt_flits.eop_out() << endl;
data <<"pbc_csr.port_1.cnt_flits.sop_in: 0x" << pbc_csr.port_1.cnt_flits.sop_in() << endl;
data <<"pbc_csr.port_1.cnt_flits.srdy_no_drdy_out: 0x" << pbc_csr.port_1.cnt_flits.srdy_no_drdy_out() << endl;

    pbc_csr.port_1.cnt_write_error.read();
    data <<"pbc_csr.port_1.cnt_write_error.all: 0x" << pbc_csr.port_1.cnt_write_error.all() << endl;
data <<"pbc_csr.port_1.cnt_write_error.intrinsic_drop: 0x" << pbc_csr.port_1.cnt_write_error.intrinsic_drop() << endl;
data <<"pbc_csr.port_1.cnt_write_error.out_of_cells1: 0x" << pbc_csr.port_1.cnt_write_error.out_of_cells1() << endl;
data <<"pbc_csr.port_1.cnt_write_error.out_of_credit: 0x" << pbc_csr.port_1.cnt_write_error.out_of_credit() << endl;
data <<"pbc_csr.port_1.cnt_write_error.out_of_cells: 0x" << pbc_csr.port_1.cnt_write_error.out_of_cells() << endl;
data <<"pbc_csr.port_1.cnt_write_error.port_disabled: 0x" << pbc_csr.port_1.cnt_write_error.port_disabled() << endl;
data <<"pbc_csr.port_1.cnt_write_error.admitted: 0x" << pbc_csr.port_1.cnt_write_error.admitted() << endl;
data <<"pbc_csr.port_1.cnt_write_error.discarded: 0x" << pbc_csr.port_1.cnt_write_error.discarded() << endl;
data <<"pbc_csr.port_1.cnt_write_error.enqueue: 0x" << pbc_csr.port_1.cnt_write_error.enqueue() << endl;
data <<"pbc_csr.port_1.cnt_write_error.oq_range: 0x" << pbc_csr.port_1.cnt_write_error.oq_range() << endl;
data <<"pbc_csr.port_1.cnt_write_error.tail_drop_span: 0x" << pbc_csr.port_1.cnt_write_error.tail_drop_span() << endl;
data <<"pbc_csr.port_1.cnt_write_error.port_range: 0x" << pbc_csr.port_1.cnt_write_error.port_range() << endl;
data <<"pbc_csr.port_1.cnt_write_error.tail_drop_cpu: 0x" << pbc_csr.port_1.cnt_write_error.tail_drop_cpu() << endl;
data <<"pbc_csr.port_1.cnt_write_error.no_dest: 0x" << pbc_csr.port_1.cnt_write_error.no_dest() << endl;
data <<"pbc_csr.port_1.cnt_write_error.min_size_viol: 0x" << pbc_csr.port_1.cnt_write_error.min_size_viol() << endl;
data <<"pbc_csr.port_1.cnt_write_error.truncation: 0x" << pbc_csr.port_1.cnt_write_error.truncation() << endl;

    pbc_csr.port_1.sta_account.read();
    data <<"pbc_csr.port_1.sta_account.all: 0x" << pbc_csr.port_1.sta_account.all() << endl;
data <<"pbc_csr.port_1.sta_account.occupancy_5: 0x" << pbc_csr.port_1.sta_account.occupancy_5() << endl;
data <<"pbc_csr.port_1.sta_account.occupancy_4: 0x" << pbc_csr.port_1.sta_account.occupancy_4() << endl;
data <<"pbc_csr.port_1.sta_account.occupancy_7: 0x" << pbc_csr.port_1.sta_account.occupancy_7() << endl;
data <<"pbc_csr.port_1.sta_account.occupancy_6: 0x" << pbc_csr.port_1.sta_account.occupancy_6() << endl;
data <<"pbc_csr.port_1.sta_account.occupancy_1: 0x" << pbc_csr.port_1.sta_account.occupancy_1() << endl;
data <<"pbc_csr.port_1.sta_account.occupancy_0: 0x" << pbc_csr.port_1.sta_account.occupancy_0() << endl;
data <<"pbc_csr.port_1.sta_account.occupancy_3: 0x" << pbc_csr.port_1.sta_account.occupancy_3() << endl;
data <<"pbc_csr.port_1.sta_account.occupancy_2: 0x" << pbc_csr.port_1.sta_account.occupancy_2() << endl;
data <<"pbc_csr.port_1.sta_account.sp_held_5: 0x" << pbc_csr.port_1.sta_account.sp_held_5() << endl;
data <<"pbc_csr.port_1.sta_account.sp_held_4: 0x" << pbc_csr.port_1.sta_account.sp_held_4() << endl;
data <<"pbc_csr.port_1.sta_account.sp_held_7: 0x" << pbc_csr.port_1.sta_account.sp_held_7() << endl;
data <<"pbc_csr.port_1.sta_account.sp_held_6: 0x" << pbc_csr.port_1.sta_account.sp_held_6() << endl;
data <<"pbc_csr.port_1.sta_account.sp_held_1: 0x" << pbc_csr.port_1.sta_account.sp_held_1() << endl;
data <<"pbc_csr.port_1.sta_account.sp_held_0: 0x" << pbc_csr.port_1.sta_account.sp_held_0() << endl;
data <<"pbc_csr.port_1.sta_account.sp_held_3: 0x" << pbc_csr.port_1.sta_account.sp_held_3() << endl;
data <<"pbc_csr.port_1.sta_account.sp_held_2: 0x" << pbc_csr.port_1.sta_account.sp_held_2() << endl;

    pbc_csr.port_1.sta_account_pg_0_watermark.read();
    data <<"pbc_csr.port_1.sta_account_pg_0_watermark.all: 0x" << pbc_csr.port_1.sta_account_pg_0_watermark.all() << endl;
data <<"pbc_csr.port_1.sta_account_pg_0_watermark.shared_cells: 0x" << pbc_csr.port_1.sta_account_pg_0_watermark.shared_cells() << endl;
data <<"pbc_csr.port_1.sta_account_pg_0_watermark.total_cells: 0x" << pbc_csr.port_1.sta_account_pg_0_watermark.total_cells() << endl;

    pbc_csr.port_1.sta_account_pg_1_watermark.read();
    data <<"pbc_csr.port_1.sta_account_pg_1_watermark.all: 0x" << pbc_csr.port_1.sta_account_pg_1_watermark.all() << endl;
data <<"pbc_csr.port_1.sta_account_pg_1_watermark.shared_cells: 0x" << pbc_csr.port_1.sta_account_pg_1_watermark.shared_cells() << endl;
data <<"pbc_csr.port_1.sta_account_pg_1_watermark.total_cells: 0x" << pbc_csr.port_1.sta_account_pg_1_watermark.total_cells() << endl;

    pbc_csr.port_1.sta_account_pg_2_watermark.read();
    data <<"pbc_csr.port_1.sta_account_pg_2_watermark.all: 0x" << pbc_csr.port_1.sta_account_pg_2_watermark.all() << endl;
data <<"pbc_csr.port_1.sta_account_pg_2_watermark.shared_cells: 0x" << pbc_csr.port_1.sta_account_pg_2_watermark.shared_cells() << endl;
data <<"pbc_csr.port_1.sta_account_pg_2_watermark.total_cells: 0x" << pbc_csr.port_1.sta_account_pg_2_watermark.total_cells() << endl;

    pbc_csr.port_1.sta_account_pg_3_watermark.read();
    data <<"pbc_csr.port_1.sta_account_pg_3_watermark.all: 0x" << pbc_csr.port_1.sta_account_pg_3_watermark.all() << endl;
data <<"pbc_csr.port_1.sta_account_pg_3_watermark.shared_cells: 0x" << pbc_csr.port_1.sta_account_pg_3_watermark.shared_cells() << endl;
data <<"pbc_csr.port_1.sta_account_pg_3_watermark.total_cells: 0x" << pbc_csr.port_1.sta_account_pg_3_watermark.total_cells() << endl;

    pbc_csr.port_1.sta_account_pg_4_watermark.read();
    data <<"pbc_csr.port_1.sta_account_pg_4_watermark.all: 0x" << pbc_csr.port_1.sta_account_pg_4_watermark.all() << endl;
data <<"pbc_csr.port_1.sta_account_pg_4_watermark.shared_cells: 0x" << pbc_csr.port_1.sta_account_pg_4_watermark.shared_cells() << endl;
data <<"pbc_csr.port_1.sta_account_pg_4_watermark.total_cells: 0x" << pbc_csr.port_1.sta_account_pg_4_watermark.total_cells() << endl;

    pbc_csr.port_1.sta_account_pg_5_watermark.read();
    data <<"pbc_csr.port_1.sta_account_pg_5_watermark.all: 0x" << pbc_csr.port_1.sta_account_pg_5_watermark.all() << endl;
data <<"pbc_csr.port_1.sta_account_pg_5_watermark.shared_cells: 0x" << pbc_csr.port_1.sta_account_pg_5_watermark.shared_cells() << endl;
data <<"pbc_csr.port_1.sta_account_pg_5_watermark.total_cells: 0x" << pbc_csr.port_1.sta_account_pg_5_watermark.total_cells() << endl;

    pbc_csr.port_1.sta_account_pg_6_watermark.read();
    data <<"pbc_csr.port_1.sta_account_pg_6_watermark.all: 0x" << pbc_csr.port_1.sta_account_pg_6_watermark.all() << endl;
data <<"pbc_csr.port_1.sta_account_pg_6_watermark.shared_cells: 0x" << pbc_csr.port_1.sta_account_pg_6_watermark.shared_cells() << endl;
data <<"pbc_csr.port_1.sta_account_pg_6_watermark.total_cells: 0x" << pbc_csr.port_1.sta_account_pg_6_watermark.total_cells() << endl;

    pbc_csr.port_1.sta_account_pg_7_watermark.read();
    data <<"pbc_csr.port_1.sta_account_pg_7_watermark.all: 0x" << pbc_csr.port_1.sta_account_pg_7_watermark.all() << endl;
data <<"pbc_csr.port_1.sta_account_pg_7_watermark.shared_cells: 0x" << pbc_csr.port_1.sta_account_pg_7_watermark.shared_cells() << endl;
data <<"pbc_csr.port_1.sta_account_pg_7_watermark.total_cells: 0x" << pbc_csr.port_1.sta_account_pg_7_watermark.total_cells() << endl;

    pbc_csr.port_1.sta_write.read();
    data <<"pbc_csr.port_1.sta_write.all: 0x" << pbc_csr.port_1.sta_write.all() << endl;
data <<"pbc_csr.port_1.sta_write.cache_entries: 0x" << pbc_csr.port_1.sta_write.cache_entries() << endl;

    pbc_csr.port_2.cnt_flits.read();
    data <<"pbc_csr.port_2.cnt_flits.all: 0x" << pbc_csr.port_2.cnt_flits.all() << endl;
data <<"pbc_csr.port_2.cnt_flits.eop_in: 0x" << pbc_csr.port_2.cnt_flits.eop_in() << endl;
data <<"pbc_csr.port_2.cnt_flits.srdy_no_drdy_in: 0x" << pbc_csr.port_2.cnt_flits.srdy_no_drdy_in() << endl;
data <<"pbc_csr.port_2.cnt_flits.sop_out: 0x" << pbc_csr.port_2.cnt_flits.sop_out() << endl;
data <<"pbc_csr.port_2.cnt_flits.eop_out: 0x" << pbc_csr.port_2.cnt_flits.eop_out() << endl;
data <<"pbc_csr.port_2.cnt_flits.sop_in: 0x" << pbc_csr.port_2.cnt_flits.sop_in() << endl;
data <<"pbc_csr.port_2.cnt_flits.srdy_no_drdy_out: 0x" << pbc_csr.port_2.cnt_flits.srdy_no_drdy_out() << endl;

    pbc_csr.port_2.cnt_write_error.read();
    data <<"pbc_csr.port_2.cnt_write_error.all: 0x" << pbc_csr.port_2.cnt_write_error.all() << endl;
data <<"pbc_csr.port_2.cnt_write_error.intrinsic_drop: 0x" << pbc_csr.port_2.cnt_write_error.intrinsic_drop() << endl;
data <<"pbc_csr.port_2.cnt_write_error.out_of_cells1: 0x" << pbc_csr.port_2.cnt_write_error.out_of_cells1() << endl;
data <<"pbc_csr.port_2.cnt_write_error.out_of_credit: 0x" << pbc_csr.port_2.cnt_write_error.out_of_credit() << endl;
data <<"pbc_csr.port_2.cnt_write_error.out_of_cells: 0x" << pbc_csr.port_2.cnt_write_error.out_of_cells() << endl;
data <<"pbc_csr.port_2.cnt_write_error.port_disabled: 0x" << pbc_csr.port_2.cnt_write_error.port_disabled() << endl;
data <<"pbc_csr.port_2.cnt_write_error.admitted: 0x" << pbc_csr.port_2.cnt_write_error.admitted() << endl;
data <<"pbc_csr.port_2.cnt_write_error.discarded: 0x" << pbc_csr.port_2.cnt_write_error.discarded() << endl;
data <<"pbc_csr.port_2.cnt_write_error.enqueue: 0x" << pbc_csr.port_2.cnt_write_error.enqueue() << endl;
data <<"pbc_csr.port_2.cnt_write_error.oq_range: 0x" << pbc_csr.port_2.cnt_write_error.oq_range() << endl;
data <<"pbc_csr.port_2.cnt_write_error.tail_drop_span: 0x" << pbc_csr.port_2.cnt_write_error.tail_drop_span() << endl;
data <<"pbc_csr.port_2.cnt_write_error.port_range: 0x" << pbc_csr.port_2.cnt_write_error.port_range() << endl;
data <<"pbc_csr.port_2.cnt_write_error.tail_drop_cpu: 0x" << pbc_csr.port_2.cnt_write_error.tail_drop_cpu() << endl;
data <<"pbc_csr.port_2.cnt_write_error.no_dest: 0x" << pbc_csr.port_2.cnt_write_error.no_dest() << endl;
data <<"pbc_csr.port_2.cnt_write_error.min_size_viol: 0x" << pbc_csr.port_2.cnt_write_error.min_size_viol() << endl;
data <<"pbc_csr.port_2.cnt_write_error.truncation: 0x" << pbc_csr.port_2.cnt_write_error.truncation() << endl;

    pbc_csr.port_2.sta_account.read();
    data <<"pbc_csr.port_2.sta_account.all: 0x" << pbc_csr.port_2.sta_account.all() << endl;
data <<"pbc_csr.port_2.sta_account.occupancy_5: 0x" << pbc_csr.port_2.sta_account.occupancy_5() << endl;
data <<"pbc_csr.port_2.sta_account.occupancy_4: 0x" << pbc_csr.port_2.sta_account.occupancy_4() << endl;
data <<"pbc_csr.port_2.sta_account.occupancy_7: 0x" << pbc_csr.port_2.sta_account.occupancy_7() << endl;
data <<"pbc_csr.port_2.sta_account.occupancy_6: 0x" << pbc_csr.port_2.sta_account.occupancy_6() << endl;
data <<"pbc_csr.port_2.sta_account.occupancy_1: 0x" << pbc_csr.port_2.sta_account.occupancy_1() << endl;
data <<"pbc_csr.port_2.sta_account.occupancy_0: 0x" << pbc_csr.port_2.sta_account.occupancy_0() << endl;
data <<"pbc_csr.port_2.sta_account.occupancy_3: 0x" << pbc_csr.port_2.sta_account.occupancy_3() << endl;
data <<"pbc_csr.port_2.sta_account.occupancy_2: 0x" << pbc_csr.port_2.sta_account.occupancy_2() << endl;
data <<"pbc_csr.port_2.sta_account.sp_held_5: 0x" << pbc_csr.port_2.sta_account.sp_held_5() << endl;
data <<"pbc_csr.port_2.sta_account.sp_held_4: 0x" << pbc_csr.port_2.sta_account.sp_held_4() << endl;
data <<"pbc_csr.port_2.sta_account.sp_held_7: 0x" << pbc_csr.port_2.sta_account.sp_held_7() << endl;
data <<"pbc_csr.port_2.sta_account.sp_held_6: 0x" << pbc_csr.port_2.sta_account.sp_held_6() << endl;
data <<"pbc_csr.port_2.sta_account.sp_held_1: 0x" << pbc_csr.port_2.sta_account.sp_held_1() << endl;
data <<"pbc_csr.port_2.sta_account.sp_held_0: 0x" << pbc_csr.port_2.sta_account.sp_held_0() << endl;
data <<"pbc_csr.port_2.sta_account.sp_held_3: 0x" << pbc_csr.port_2.sta_account.sp_held_3() << endl;
data <<"pbc_csr.port_2.sta_account.sp_held_2: 0x" << pbc_csr.port_2.sta_account.sp_held_2() << endl;

    pbc_csr.port_2.sta_account_pg_0_watermark.read();
    data <<"pbc_csr.port_2.sta_account_pg_0_watermark.all: 0x" << pbc_csr.port_2.sta_account_pg_0_watermark.all() << endl;
data <<"pbc_csr.port_2.sta_account_pg_0_watermark.shared_cells: 0x" << pbc_csr.port_2.sta_account_pg_0_watermark.shared_cells() << endl;
data <<"pbc_csr.port_2.sta_account_pg_0_watermark.total_cells: 0x" << pbc_csr.port_2.sta_account_pg_0_watermark.total_cells() << endl;

    pbc_csr.port_2.sta_account_pg_1_watermark.read();
    data <<"pbc_csr.port_2.sta_account_pg_1_watermark.all: 0x" << pbc_csr.port_2.sta_account_pg_1_watermark.all() << endl;
data <<"pbc_csr.port_2.sta_account_pg_1_watermark.shared_cells: 0x" << pbc_csr.port_2.sta_account_pg_1_watermark.shared_cells() << endl;
data <<"pbc_csr.port_2.sta_account_pg_1_watermark.total_cells: 0x" << pbc_csr.port_2.sta_account_pg_1_watermark.total_cells() << endl;

    pbc_csr.port_2.sta_account_pg_2_watermark.read();
    data <<"pbc_csr.port_2.sta_account_pg_2_watermark.all: 0x" << pbc_csr.port_2.sta_account_pg_2_watermark.all() << endl;
data <<"pbc_csr.port_2.sta_account_pg_2_watermark.shared_cells: 0x" << pbc_csr.port_2.sta_account_pg_2_watermark.shared_cells() << endl;
data <<"pbc_csr.port_2.sta_account_pg_2_watermark.total_cells: 0x" << pbc_csr.port_2.sta_account_pg_2_watermark.total_cells() << endl;

    pbc_csr.port_2.sta_account_pg_3_watermark.read();
    data <<"pbc_csr.port_2.sta_account_pg_3_watermark.all: 0x" << pbc_csr.port_2.sta_account_pg_3_watermark.all() << endl;
data <<"pbc_csr.port_2.sta_account_pg_3_watermark.shared_cells: 0x" << pbc_csr.port_2.sta_account_pg_3_watermark.shared_cells() << endl;
data <<"pbc_csr.port_2.sta_account_pg_3_watermark.total_cells: 0x" << pbc_csr.port_2.sta_account_pg_3_watermark.total_cells() << endl;

    pbc_csr.port_2.sta_account_pg_4_watermark.read();
    data <<"pbc_csr.port_2.sta_account_pg_4_watermark.all: 0x" << pbc_csr.port_2.sta_account_pg_4_watermark.all() << endl;
data <<"pbc_csr.port_2.sta_account_pg_4_watermark.shared_cells: 0x" << pbc_csr.port_2.sta_account_pg_4_watermark.shared_cells() << endl;
data <<"pbc_csr.port_2.sta_account_pg_4_watermark.total_cells: 0x" << pbc_csr.port_2.sta_account_pg_4_watermark.total_cells() << endl;

    pbc_csr.port_2.sta_account_pg_5_watermark.read();
    data <<"pbc_csr.port_2.sta_account_pg_5_watermark.all: 0x" << pbc_csr.port_2.sta_account_pg_5_watermark.all() << endl;
data <<"pbc_csr.port_2.sta_account_pg_5_watermark.shared_cells: 0x" << pbc_csr.port_2.sta_account_pg_5_watermark.shared_cells() << endl;
data <<"pbc_csr.port_2.sta_account_pg_5_watermark.total_cells: 0x" << pbc_csr.port_2.sta_account_pg_5_watermark.total_cells() << endl;

    pbc_csr.port_2.sta_account_pg_6_watermark.read();
    data <<"pbc_csr.port_2.sta_account_pg_6_watermark.all: 0x" << pbc_csr.port_2.sta_account_pg_6_watermark.all() << endl;
data <<"pbc_csr.port_2.sta_account_pg_6_watermark.shared_cells: 0x" << pbc_csr.port_2.sta_account_pg_6_watermark.shared_cells() << endl;
data <<"pbc_csr.port_2.sta_account_pg_6_watermark.total_cells: 0x" << pbc_csr.port_2.sta_account_pg_6_watermark.total_cells() << endl;

    pbc_csr.port_2.sta_account_pg_7_watermark.read();
    data <<"pbc_csr.port_2.sta_account_pg_7_watermark.all: 0x" << pbc_csr.port_2.sta_account_pg_7_watermark.all() << endl;
data <<"pbc_csr.port_2.sta_account_pg_7_watermark.shared_cells: 0x" << pbc_csr.port_2.sta_account_pg_7_watermark.shared_cells() << endl;
data <<"pbc_csr.port_2.sta_account_pg_7_watermark.total_cells: 0x" << pbc_csr.port_2.sta_account_pg_7_watermark.total_cells() << endl;

    pbc_csr.port_2.sta_write.read();
    data <<"pbc_csr.port_2.sta_write.all: 0x" << pbc_csr.port_2.sta_write.all() << endl;
data <<"pbc_csr.port_2.sta_write.cache_entries: 0x" << pbc_csr.port_2.sta_write.cache_entries() << endl;

    pbc_csr.port_3.cnt_flits.read();
    data <<"pbc_csr.port_3.cnt_flits.all: 0x" << pbc_csr.port_3.cnt_flits.all() << endl;
data <<"pbc_csr.port_3.cnt_flits.eop_in: 0x" << pbc_csr.port_3.cnt_flits.eop_in() << endl;
data <<"pbc_csr.port_3.cnt_flits.srdy_no_drdy_in: 0x" << pbc_csr.port_3.cnt_flits.srdy_no_drdy_in() << endl;
data <<"pbc_csr.port_3.cnt_flits.sop_out: 0x" << pbc_csr.port_3.cnt_flits.sop_out() << endl;
data <<"pbc_csr.port_3.cnt_flits.eop_out: 0x" << pbc_csr.port_3.cnt_flits.eop_out() << endl;
data <<"pbc_csr.port_3.cnt_flits.sop_in: 0x" << pbc_csr.port_3.cnt_flits.sop_in() << endl;
data <<"pbc_csr.port_3.cnt_flits.srdy_no_drdy_out: 0x" << pbc_csr.port_3.cnt_flits.srdy_no_drdy_out() << endl;

    pbc_csr.port_3.cnt_write_error.read();
    data <<"pbc_csr.port_3.cnt_write_error.all: 0x" << pbc_csr.port_3.cnt_write_error.all() << endl;
data <<"pbc_csr.port_3.cnt_write_error.intrinsic_drop: 0x" << pbc_csr.port_3.cnt_write_error.intrinsic_drop() << endl;
data <<"pbc_csr.port_3.cnt_write_error.out_of_cells1: 0x" << pbc_csr.port_3.cnt_write_error.out_of_cells1() << endl;
data <<"pbc_csr.port_3.cnt_write_error.out_of_credit: 0x" << pbc_csr.port_3.cnt_write_error.out_of_credit() << endl;
data <<"pbc_csr.port_3.cnt_write_error.out_of_cells: 0x" << pbc_csr.port_3.cnt_write_error.out_of_cells() << endl;
data <<"pbc_csr.port_3.cnt_write_error.port_disabled: 0x" << pbc_csr.port_3.cnt_write_error.port_disabled() << endl;
data <<"pbc_csr.port_3.cnt_write_error.admitted: 0x" << pbc_csr.port_3.cnt_write_error.admitted() << endl;
data <<"pbc_csr.port_3.cnt_write_error.discarded: 0x" << pbc_csr.port_3.cnt_write_error.discarded() << endl;
data <<"pbc_csr.port_3.cnt_write_error.enqueue: 0x" << pbc_csr.port_3.cnt_write_error.enqueue() << endl;
data <<"pbc_csr.port_3.cnt_write_error.oq_range: 0x" << pbc_csr.port_3.cnt_write_error.oq_range() << endl;
data <<"pbc_csr.port_3.cnt_write_error.tail_drop_span: 0x" << pbc_csr.port_3.cnt_write_error.tail_drop_span() << endl;
data <<"pbc_csr.port_3.cnt_write_error.port_range: 0x" << pbc_csr.port_3.cnt_write_error.port_range() << endl;
data <<"pbc_csr.port_3.cnt_write_error.tail_drop_cpu: 0x" << pbc_csr.port_3.cnt_write_error.tail_drop_cpu() << endl;
data <<"pbc_csr.port_3.cnt_write_error.no_dest: 0x" << pbc_csr.port_3.cnt_write_error.no_dest() << endl;
data <<"pbc_csr.port_3.cnt_write_error.min_size_viol: 0x" << pbc_csr.port_3.cnt_write_error.min_size_viol() << endl;
data <<"pbc_csr.port_3.cnt_write_error.truncation: 0x" << pbc_csr.port_3.cnt_write_error.truncation() << endl;

    pbc_csr.port_3.sta_account.read();
    data <<"pbc_csr.port_3.sta_account.all: 0x" << pbc_csr.port_3.sta_account.all() << endl;
data <<"pbc_csr.port_3.sta_account.occupancy_5: 0x" << pbc_csr.port_3.sta_account.occupancy_5() << endl;
data <<"pbc_csr.port_3.sta_account.occupancy_4: 0x" << pbc_csr.port_3.sta_account.occupancy_4() << endl;
data <<"pbc_csr.port_3.sta_account.occupancy_7: 0x" << pbc_csr.port_3.sta_account.occupancy_7() << endl;
data <<"pbc_csr.port_3.sta_account.occupancy_6: 0x" << pbc_csr.port_3.sta_account.occupancy_6() << endl;
data <<"pbc_csr.port_3.sta_account.occupancy_1: 0x" << pbc_csr.port_3.sta_account.occupancy_1() << endl;
data <<"pbc_csr.port_3.sta_account.occupancy_0: 0x" << pbc_csr.port_3.sta_account.occupancy_0() << endl;
data <<"pbc_csr.port_3.sta_account.occupancy_3: 0x" << pbc_csr.port_3.sta_account.occupancy_3() << endl;
data <<"pbc_csr.port_3.sta_account.occupancy_2: 0x" << pbc_csr.port_3.sta_account.occupancy_2() << endl;
data <<"pbc_csr.port_3.sta_account.sp_held_5: 0x" << pbc_csr.port_3.sta_account.sp_held_5() << endl;
data <<"pbc_csr.port_3.sta_account.sp_held_4: 0x" << pbc_csr.port_3.sta_account.sp_held_4() << endl;
data <<"pbc_csr.port_3.sta_account.sp_held_7: 0x" << pbc_csr.port_3.sta_account.sp_held_7() << endl;
data <<"pbc_csr.port_3.sta_account.sp_held_6: 0x" << pbc_csr.port_3.sta_account.sp_held_6() << endl;
data <<"pbc_csr.port_3.sta_account.sp_held_1: 0x" << pbc_csr.port_3.sta_account.sp_held_1() << endl;
data <<"pbc_csr.port_3.sta_account.sp_held_0: 0x" << pbc_csr.port_3.sta_account.sp_held_0() << endl;
data <<"pbc_csr.port_3.sta_account.sp_held_3: 0x" << pbc_csr.port_3.sta_account.sp_held_3() << endl;
data <<"pbc_csr.port_3.sta_account.sp_held_2: 0x" << pbc_csr.port_3.sta_account.sp_held_2() << endl;

    pbc_csr.port_3.sta_account_pg_0_watermark.read();
    data <<"pbc_csr.port_3.sta_account_pg_0_watermark.all: 0x" << pbc_csr.port_3.sta_account_pg_0_watermark.all() << endl;
data <<"pbc_csr.port_3.sta_account_pg_0_watermark.shared_cells: 0x" << pbc_csr.port_3.sta_account_pg_0_watermark.shared_cells() << endl;
data <<"pbc_csr.port_3.sta_account_pg_0_watermark.total_cells: 0x" << pbc_csr.port_3.sta_account_pg_0_watermark.total_cells() << endl;

    pbc_csr.port_3.sta_account_pg_1_watermark.read();
    data <<"pbc_csr.port_3.sta_account_pg_1_watermark.all: 0x" << pbc_csr.port_3.sta_account_pg_1_watermark.all() << endl;
data <<"pbc_csr.port_3.sta_account_pg_1_watermark.shared_cells: 0x" << pbc_csr.port_3.sta_account_pg_1_watermark.shared_cells() << endl;
data <<"pbc_csr.port_3.sta_account_pg_1_watermark.total_cells: 0x" << pbc_csr.port_3.sta_account_pg_1_watermark.total_cells() << endl;

    pbc_csr.port_3.sta_account_pg_2_watermark.read();
    data <<"pbc_csr.port_3.sta_account_pg_2_watermark.all: 0x" << pbc_csr.port_3.sta_account_pg_2_watermark.all() << endl;
data <<"pbc_csr.port_3.sta_account_pg_2_watermark.shared_cells: 0x" << pbc_csr.port_3.sta_account_pg_2_watermark.shared_cells() << endl;
data <<"pbc_csr.port_3.sta_account_pg_2_watermark.total_cells: 0x" << pbc_csr.port_3.sta_account_pg_2_watermark.total_cells() << endl;

    pbc_csr.port_3.sta_account_pg_3_watermark.read();
    data <<"pbc_csr.port_3.sta_account_pg_3_watermark.all: 0x" << pbc_csr.port_3.sta_account_pg_3_watermark.all() << endl;
data <<"pbc_csr.port_3.sta_account_pg_3_watermark.shared_cells: 0x" << pbc_csr.port_3.sta_account_pg_3_watermark.shared_cells() << endl;
data <<"pbc_csr.port_3.sta_account_pg_3_watermark.total_cells: 0x" << pbc_csr.port_3.sta_account_pg_3_watermark.total_cells() << endl;

    pbc_csr.port_3.sta_account_pg_4_watermark.read();
    data <<"pbc_csr.port_3.sta_account_pg_4_watermark.all: 0x" << pbc_csr.port_3.sta_account_pg_4_watermark.all() << endl;
data <<"pbc_csr.port_3.sta_account_pg_4_watermark.shared_cells: 0x" << pbc_csr.port_3.sta_account_pg_4_watermark.shared_cells() << endl;
data <<"pbc_csr.port_3.sta_account_pg_4_watermark.total_cells: 0x" << pbc_csr.port_3.sta_account_pg_4_watermark.total_cells() << endl;

    pbc_csr.port_3.sta_account_pg_5_watermark.read();
    data <<"pbc_csr.port_3.sta_account_pg_5_watermark.all: 0x" << pbc_csr.port_3.sta_account_pg_5_watermark.all() << endl;
data <<"pbc_csr.port_3.sta_account_pg_5_watermark.shared_cells: 0x" << pbc_csr.port_3.sta_account_pg_5_watermark.shared_cells() << endl;
data <<"pbc_csr.port_3.sta_account_pg_5_watermark.total_cells: 0x" << pbc_csr.port_3.sta_account_pg_5_watermark.total_cells() << endl;

    pbc_csr.port_3.sta_account_pg_6_watermark.read();
    data <<"pbc_csr.port_3.sta_account_pg_6_watermark.all: 0x" << pbc_csr.port_3.sta_account_pg_6_watermark.all() << endl;
data <<"pbc_csr.port_3.sta_account_pg_6_watermark.shared_cells: 0x" << pbc_csr.port_3.sta_account_pg_6_watermark.shared_cells() << endl;
data <<"pbc_csr.port_3.sta_account_pg_6_watermark.total_cells: 0x" << pbc_csr.port_3.sta_account_pg_6_watermark.total_cells() << endl;

    pbc_csr.port_3.sta_account_pg_7_watermark.read();
    data <<"pbc_csr.port_3.sta_account_pg_7_watermark.all: 0x" << pbc_csr.port_3.sta_account_pg_7_watermark.all() << endl;
data <<"pbc_csr.port_3.sta_account_pg_7_watermark.shared_cells: 0x" << pbc_csr.port_3.sta_account_pg_7_watermark.shared_cells() << endl;
data <<"pbc_csr.port_3.sta_account_pg_7_watermark.total_cells: 0x" << pbc_csr.port_3.sta_account_pg_7_watermark.total_cells() << endl;

    pbc_csr.port_3.sta_write.read();
    data <<"pbc_csr.port_3.sta_write.all: 0x" << pbc_csr.port_3.sta_write.all() << endl;
data <<"pbc_csr.port_3.sta_write.cache_entries: 0x" << pbc_csr.port_3.sta_write.cache_entries() << endl;

    pbc_csr.port_4.cnt_flits.read();
    data <<"pbc_csr.port_4.cnt_flits.all: 0x" << pbc_csr.port_4.cnt_flits.all() << endl;
data <<"pbc_csr.port_4.cnt_flits.eop_in: 0x" << pbc_csr.port_4.cnt_flits.eop_in() << endl;
data <<"pbc_csr.port_4.cnt_flits.srdy_no_drdy_in: 0x" << pbc_csr.port_4.cnt_flits.srdy_no_drdy_in() << endl;
data <<"pbc_csr.port_4.cnt_flits.sop_out: 0x" << pbc_csr.port_4.cnt_flits.sop_out() << endl;
data <<"pbc_csr.port_4.cnt_flits.eop_out: 0x" << pbc_csr.port_4.cnt_flits.eop_out() << endl;
data <<"pbc_csr.port_4.cnt_flits.sop_in: 0x" << pbc_csr.port_4.cnt_flits.sop_in() << endl;
data <<"pbc_csr.port_4.cnt_flits.srdy_no_drdy_out: 0x" << pbc_csr.port_4.cnt_flits.srdy_no_drdy_out() << endl;

    pbc_csr.port_4.cnt_write_error.read();
    data <<"pbc_csr.port_4.cnt_write_error.all: 0x" << pbc_csr.port_4.cnt_write_error.all() << endl;
data <<"pbc_csr.port_4.cnt_write_error.intrinsic_drop: 0x" << pbc_csr.port_4.cnt_write_error.intrinsic_drop() << endl;
data <<"pbc_csr.port_4.cnt_write_error.out_of_cells1: 0x" << pbc_csr.port_4.cnt_write_error.out_of_cells1() << endl;
data <<"pbc_csr.port_4.cnt_write_error.out_of_credit: 0x" << pbc_csr.port_4.cnt_write_error.out_of_credit() << endl;
data <<"pbc_csr.port_4.cnt_write_error.out_of_cells: 0x" << pbc_csr.port_4.cnt_write_error.out_of_cells() << endl;
data <<"pbc_csr.port_4.cnt_write_error.port_disabled: 0x" << pbc_csr.port_4.cnt_write_error.port_disabled() << endl;
data <<"pbc_csr.port_4.cnt_write_error.admitted: 0x" << pbc_csr.port_4.cnt_write_error.admitted() << endl;
data <<"pbc_csr.port_4.cnt_write_error.discarded: 0x" << pbc_csr.port_4.cnt_write_error.discarded() << endl;
data <<"pbc_csr.port_4.cnt_write_error.enqueue: 0x" << pbc_csr.port_4.cnt_write_error.enqueue() << endl;
data <<"pbc_csr.port_4.cnt_write_error.oq_range: 0x" << pbc_csr.port_4.cnt_write_error.oq_range() << endl;
data <<"pbc_csr.port_4.cnt_write_error.tail_drop_span: 0x" << pbc_csr.port_4.cnt_write_error.tail_drop_span() << endl;
data <<"pbc_csr.port_4.cnt_write_error.port_range: 0x" << pbc_csr.port_4.cnt_write_error.port_range() << endl;
data <<"pbc_csr.port_4.cnt_write_error.tail_drop_cpu: 0x" << pbc_csr.port_4.cnt_write_error.tail_drop_cpu() << endl;
data <<"pbc_csr.port_4.cnt_write_error.no_dest: 0x" << pbc_csr.port_4.cnt_write_error.no_dest() << endl;
data <<"pbc_csr.port_4.cnt_write_error.min_size_viol: 0x" << pbc_csr.port_4.cnt_write_error.min_size_viol() << endl;
data <<"pbc_csr.port_4.cnt_write_error.truncation: 0x" << pbc_csr.port_4.cnt_write_error.truncation() << endl;

    pbc_csr.port_4.sta_account.read();
    data <<"pbc_csr.port_4.sta_account.all: 0x" << pbc_csr.port_4.sta_account.all() << endl;
data <<"pbc_csr.port_4.sta_account.occupancy_5: 0x" << pbc_csr.port_4.sta_account.occupancy_5() << endl;
data <<"pbc_csr.port_4.sta_account.occupancy_4: 0x" << pbc_csr.port_4.sta_account.occupancy_4() << endl;
data <<"pbc_csr.port_4.sta_account.occupancy_7: 0x" << pbc_csr.port_4.sta_account.occupancy_7() << endl;
data <<"pbc_csr.port_4.sta_account.occupancy_6: 0x" << pbc_csr.port_4.sta_account.occupancy_6() << endl;
data <<"pbc_csr.port_4.sta_account.occupancy_1: 0x" << pbc_csr.port_4.sta_account.occupancy_1() << endl;
data <<"pbc_csr.port_4.sta_account.occupancy_0: 0x" << pbc_csr.port_4.sta_account.occupancy_0() << endl;
data <<"pbc_csr.port_4.sta_account.occupancy_3: 0x" << pbc_csr.port_4.sta_account.occupancy_3() << endl;
data <<"pbc_csr.port_4.sta_account.occupancy_2: 0x" << pbc_csr.port_4.sta_account.occupancy_2() << endl;
data <<"pbc_csr.port_4.sta_account.sp_held_5: 0x" << pbc_csr.port_4.sta_account.sp_held_5() << endl;
data <<"pbc_csr.port_4.sta_account.sp_held_4: 0x" << pbc_csr.port_4.sta_account.sp_held_4() << endl;
data <<"pbc_csr.port_4.sta_account.sp_held_7: 0x" << pbc_csr.port_4.sta_account.sp_held_7() << endl;
data <<"pbc_csr.port_4.sta_account.sp_held_6: 0x" << pbc_csr.port_4.sta_account.sp_held_6() << endl;
data <<"pbc_csr.port_4.sta_account.sp_held_1: 0x" << pbc_csr.port_4.sta_account.sp_held_1() << endl;
data <<"pbc_csr.port_4.sta_account.sp_held_0: 0x" << pbc_csr.port_4.sta_account.sp_held_0() << endl;
data <<"pbc_csr.port_4.sta_account.sp_held_3: 0x" << pbc_csr.port_4.sta_account.sp_held_3() << endl;
data <<"pbc_csr.port_4.sta_account.sp_held_2: 0x" << pbc_csr.port_4.sta_account.sp_held_2() << endl;

    pbc_csr.port_4.sta_account_pg_0_watermark.read();
    data <<"pbc_csr.port_4.sta_account_pg_0_watermark.all: 0x" << pbc_csr.port_4.sta_account_pg_0_watermark.all() << endl;
data <<"pbc_csr.port_4.sta_account_pg_0_watermark.shared_cells: 0x" << pbc_csr.port_4.sta_account_pg_0_watermark.shared_cells() << endl;
data <<"pbc_csr.port_4.sta_account_pg_0_watermark.total_cells: 0x" << pbc_csr.port_4.sta_account_pg_0_watermark.total_cells() << endl;

    pbc_csr.port_4.sta_account_pg_1_watermark.read();
    data <<"pbc_csr.port_4.sta_account_pg_1_watermark.all: 0x" << pbc_csr.port_4.sta_account_pg_1_watermark.all() << endl;
data <<"pbc_csr.port_4.sta_account_pg_1_watermark.shared_cells: 0x" << pbc_csr.port_4.sta_account_pg_1_watermark.shared_cells() << endl;
data <<"pbc_csr.port_4.sta_account_pg_1_watermark.total_cells: 0x" << pbc_csr.port_4.sta_account_pg_1_watermark.total_cells() << endl;

    pbc_csr.port_4.sta_account_pg_2_watermark.read();
    data <<"pbc_csr.port_4.sta_account_pg_2_watermark.all: 0x" << pbc_csr.port_4.sta_account_pg_2_watermark.all() << endl;
data <<"pbc_csr.port_4.sta_account_pg_2_watermark.shared_cells: 0x" << pbc_csr.port_4.sta_account_pg_2_watermark.shared_cells() << endl;
data <<"pbc_csr.port_4.sta_account_pg_2_watermark.total_cells: 0x" << pbc_csr.port_4.sta_account_pg_2_watermark.total_cells() << endl;

    pbc_csr.port_4.sta_account_pg_3_watermark.read();
    data <<"pbc_csr.port_4.sta_account_pg_3_watermark.all: 0x" << pbc_csr.port_4.sta_account_pg_3_watermark.all() << endl;
data <<"pbc_csr.port_4.sta_account_pg_3_watermark.shared_cells: 0x" << pbc_csr.port_4.sta_account_pg_3_watermark.shared_cells() << endl;
data <<"pbc_csr.port_4.sta_account_pg_3_watermark.total_cells: 0x" << pbc_csr.port_4.sta_account_pg_3_watermark.total_cells() << endl;

    pbc_csr.port_4.sta_account_pg_4_watermark.read();
    data <<"pbc_csr.port_4.sta_account_pg_4_watermark.all: 0x" << pbc_csr.port_4.sta_account_pg_4_watermark.all() << endl;
data <<"pbc_csr.port_4.sta_account_pg_4_watermark.shared_cells: 0x" << pbc_csr.port_4.sta_account_pg_4_watermark.shared_cells() << endl;
data <<"pbc_csr.port_4.sta_account_pg_4_watermark.total_cells: 0x" << pbc_csr.port_4.sta_account_pg_4_watermark.total_cells() << endl;

    pbc_csr.port_4.sta_account_pg_5_watermark.read();
    data <<"pbc_csr.port_4.sta_account_pg_5_watermark.all: 0x" << pbc_csr.port_4.sta_account_pg_5_watermark.all() << endl;
data <<"pbc_csr.port_4.sta_account_pg_5_watermark.shared_cells: 0x" << pbc_csr.port_4.sta_account_pg_5_watermark.shared_cells() << endl;
data <<"pbc_csr.port_4.sta_account_pg_5_watermark.total_cells: 0x" << pbc_csr.port_4.sta_account_pg_5_watermark.total_cells() << endl;

    pbc_csr.port_4.sta_account_pg_6_watermark.read();
    data <<"pbc_csr.port_4.sta_account_pg_6_watermark.all: 0x" << pbc_csr.port_4.sta_account_pg_6_watermark.all() << endl;
data <<"pbc_csr.port_4.sta_account_pg_6_watermark.shared_cells: 0x" << pbc_csr.port_4.sta_account_pg_6_watermark.shared_cells() << endl;
data <<"pbc_csr.port_4.sta_account_pg_6_watermark.total_cells: 0x" << pbc_csr.port_4.sta_account_pg_6_watermark.total_cells() << endl;

    pbc_csr.port_4.sta_account_pg_7_watermark.read();
    data <<"pbc_csr.port_4.sta_account_pg_7_watermark.all: 0x" << pbc_csr.port_4.sta_account_pg_7_watermark.all() << endl;
data <<"pbc_csr.port_4.sta_account_pg_7_watermark.shared_cells: 0x" << pbc_csr.port_4.sta_account_pg_7_watermark.shared_cells() << endl;
data <<"pbc_csr.port_4.sta_account_pg_7_watermark.total_cells: 0x" << pbc_csr.port_4.sta_account_pg_7_watermark.total_cells() << endl;

    pbc_csr.port_4.sta_oq_bx_ecc.read();
    data <<"pbc_csr.port_4.sta_oq_bx_ecc.all: 0x" << pbc_csr.port_4.sta_oq_bx_ecc.all() << endl;
data <<"pbc_csr.port_4.sta_oq_bx_ecc.syndrome: 0x" << pbc_csr.port_4.sta_oq_bx_ecc.syndrome() << endl;
data <<"pbc_csr.port_4.sta_oq_bx_ecc.uncorrectable: 0x" << pbc_csr.port_4.sta_oq_bx_ecc.uncorrectable() << endl;
data <<"pbc_csr.port_4.sta_oq_bx_ecc.addr: 0x" << pbc_csr.port_4.sta_oq_bx_ecc.addr() << endl;
data <<"pbc_csr.port_4.sta_oq_bx_ecc.correctable: 0x" << pbc_csr.port_4.sta_oq_bx_ecc.correctable() << endl;

    pbc_csr.port_4.sta_write.read();
    data <<"pbc_csr.port_4.sta_write.all: 0x" << pbc_csr.port_4.sta_write.all() << endl;
data <<"pbc_csr.port_4.sta_write.cache_entries: 0x" << pbc_csr.port_4.sta_write.cache_entries() << endl;

    pbc_csr.port_5.cnt_flits.read();
    data <<"pbc_csr.port_5.cnt_flits.all: 0x" << pbc_csr.port_5.cnt_flits.all() << endl;
data <<"pbc_csr.port_5.cnt_flits.eop_in: 0x" << pbc_csr.port_5.cnt_flits.eop_in() << endl;
data <<"pbc_csr.port_5.cnt_flits.srdy_no_drdy_in: 0x" << pbc_csr.port_5.cnt_flits.srdy_no_drdy_in() << endl;
data <<"pbc_csr.port_5.cnt_flits.sop_out: 0x" << pbc_csr.port_5.cnt_flits.sop_out() << endl;
data <<"pbc_csr.port_5.cnt_flits.eop_out: 0x" << pbc_csr.port_5.cnt_flits.eop_out() << endl;
data <<"pbc_csr.port_5.cnt_flits.sop_in: 0x" << pbc_csr.port_5.cnt_flits.sop_in() << endl;
data <<"pbc_csr.port_5.cnt_flits.srdy_no_drdy_out: 0x" << pbc_csr.port_5.cnt_flits.srdy_no_drdy_out() << endl;

    pbc_csr.port_5.cnt_write_error.read();
    data <<"pbc_csr.port_5.cnt_write_error.all: 0x" << pbc_csr.port_5.cnt_write_error.all() << endl;
data <<"pbc_csr.port_5.cnt_write_error.intrinsic_drop: 0x" << pbc_csr.port_5.cnt_write_error.intrinsic_drop() << endl;
data <<"pbc_csr.port_5.cnt_write_error.out_of_cells1: 0x" << pbc_csr.port_5.cnt_write_error.out_of_cells1() << endl;
data <<"pbc_csr.port_5.cnt_write_error.out_of_credit: 0x" << pbc_csr.port_5.cnt_write_error.out_of_credit() << endl;
data <<"pbc_csr.port_5.cnt_write_error.out_of_cells: 0x" << pbc_csr.port_5.cnt_write_error.out_of_cells() << endl;
data <<"pbc_csr.port_5.cnt_write_error.port_disabled: 0x" << pbc_csr.port_5.cnt_write_error.port_disabled() << endl;
data <<"pbc_csr.port_5.cnt_write_error.admitted: 0x" << pbc_csr.port_5.cnt_write_error.admitted() << endl;
data <<"pbc_csr.port_5.cnt_write_error.discarded: 0x" << pbc_csr.port_5.cnt_write_error.discarded() << endl;
data <<"pbc_csr.port_5.cnt_write_error.enqueue: 0x" << pbc_csr.port_5.cnt_write_error.enqueue() << endl;
data <<"pbc_csr.port_5.cnt_write_error.oq_range: 0x" << pbc_csr.port_5.cnt_write_error.oq_range() << endl;
data <<"pbc_csr.port_5.cnt_write_error.tail_drop_span: 0x" << pbc_csr.port_5.cnt_write_error.tail_drop_span() << endl;
data <<"pbc_csr.port_5.cnt_write_error.port_range: 0x" << pbc_csr.port_5.cnt_write_error.port_range() << endl;
data <<"pbc_csr.port_5.cnt_write_error.tail_drop_cpu: 0x" << pbc_csr.port_5.cnt_write_error.tail_drop_cpu() << endl;
data <<"pbc_csr.port_5.cnt_write_error.no_dest: 0x" << pbc_csr.port_5.cnt_write_error.no_dest() << endl;
data <<"pbc_csr.port_5.cnt_write_error.min_size_viol: 0x" << pbc_csr.port_5.cnt_write_error.min_size_viol() << endl;
data <<"pbc_csr.port_5.cnt_write_error.truncation: 0x" << pbc_csr.port_5.cnt_write_error.truncation() << endl;

    pbc_csr.port_5.sta_account.read();
    data <<"pbc_csr.port_5.sta_account.all: 0x" << pbc_csr.port_5.sta_account.all() << endl;
data <<"pbc_csr.port_5.sta_account.occupancy_5: 0x" << pbc_csr.port_5.sta_account.occupancy_5() << endl;
data <<"pbc_csr.port_5.sta_account.occupancy_4: 0x" << pbc_csr.port_5.sta_account.occupancy_4() << endl;
data <<"pbc_csr.port_5.sta_account.occupancy_7: 0x" << pbc_csr.port_5.sta_account.occupancy_7() << endl;
data <<"pbc_csr.port_5.sta_account.occupancy_6: 0x" << pbc_csr.port_5.sta_account.occupancy_6() << endl;
data <<"pbc_csr.port_5.sta_account.occupancy_1: 0x" << pbc_csr.port_5.sta_account.occupancy_1() << endl;
data <<"pbc_csr.port_5.sta_account.occupancy_0: 0x" << pbc_csr.port_5.sta_account.occupancy_0() << endl;
data <<"pbc_csr.port_5.sta_account.occupancy_3: 0x" << pbc_csr.port_5.sta_account.occupancy_3() << endl;
data <<"pbc_csr.port_5.sta_account.occupancy_2: 0x" << pbc_csr.port_5.sta_account.occupancy_2() << endl;
data <<"pbc_csr.port_5.sta_account.sp_held_5: 0x" << pbc_csr.port_5.sta_account.sp_held_5() << endl;
data <<"pbc_csr.port_5.sta_account.sp_held_4: 0x" << pbc_csr.port_5.sta_account.sp_held_4() << endl;
data <<"pbc_csr.port_5.sta_account.sp_held_7: 0x" << pbc_csr.port_5.sta_account.sp_held_7() << endl;
data <<"pbc_csr.port_5.sta_account.sp_held_6: 0x" << pbc_csr.port_5.sta_account.sp_held_6() << endl;
data <<"pbc_csr.port_5.sta_account.sp_held_1: 0x" << pbc_csr.port_5.sta_account.sp_held_1() << endl;
data <<"pbc_csr.port_5.sta_account.sp_held_0: 0x" << pbc_csr.port_5.sta_account.sp_held_0() << endl;
data <<"pbc_csr.port_5.sta_account.sp_held_3: 0x" << pbc_csr.port_5.sta_account.sp_held_3() << endl;
data <<"pbc_csr.port_5.sta_account.sp_held_2: 0x" << pbc_csr.port_5.sta_account.sp_held_2() << endl;
data <<"pbc_csr.port_5.sta_account.sp_held_9: 0x" << pbc_csr.port_5.sta_account.sp_held_9() << endl;
data <<"pbc_csr.port_5.sta_account.sp_held_8: 0x" << pbc_csr.port_5.sta_account.sp_held_8() << endl;
data <<"pbc_csr.port_5.sta_account.occupancy_9: 0x" << pbc_csr.port_5.sta_account.occupancy_9() << endl;
data <<"pbc_csr.port_5.sta_account.occupancy_8: 0x" << pbc_csr.port_5.sta_account.occupancy_8() << endl;
data <<"pbc_csr.port_5.sta_account.occupancy_11: 0x" << pbc_csr.port_5.sta_account.occupancy_11() << endl;
data <<"pbc_csr.port_5.sta_account.occupancy_10: 0x" << pbc_csr.port_5.sta_account.occupancy_10() << endl;
data <<"pbc_csr.port_5.sta_account.occupancy_13: 0x" << pbc_csr.port_5.sta_account.occupancy_13() << endl;
data <<"pbc_csr.port_5.sta_account.occupancy_12: 0x" << pbc_csr.port_5.sta_account.occupancy_12() << endl;
data <<"pbc_csr.port_5.sta_account.occupancy_15: 0x" << pbc_csr.port_5.sta_account.occupancy_15() << endl;
data <<"pbc_csr.port_5.sta_account.occupancy_14: 0x" << pbc_csr.port_5.sta_account.occupancy_14() << endl;
data <<"pbc_csr.port_5.sta_account.sp_held_15: 0x" << pbc_csr.port_5.sta_account.sp_held_15() << endl;
data <<"pbc_csr.port_5.sta_account.sp_held_14: 0x" << pbc_csr.port_5.sta_account.sp_held_14() << endl;
data <<"pbc_csr.port_5.sta_account.sp_held_11: 0x" << pbc_csr.port_5.sta_account.sp_held_11() << endl;
data <<"pbc_csr.port_5.sta_account.sp_held_10: 0x" << pbc_csr.port_5.sta_account.sp_held_10() << endl;
data <<"pbc_csr.port_5.sta_account.sp_held_13: 0x" << pbc_csr.port_5.sta_account.sp_held_13() << endl;
data <<"pbc_csr.port_5.sta_account.sp_held_12: 0x" << pbc_csr.port_5.sta_account.sp_held_12() << endl;

    pbc_csr.port_5.sta_account_pg_0_watermark.read();
    data <<"pbc_csr.port_5.sta_account_pg_0_watermark.all: 0x" << pbc_csr.port_5.sta_account_pg_0_watermark.all() << endl;
data <<"pbc_csr.port_5.sta_account_pg_0_watermark.shared_cells: 0x" << pbc_csr.port_5.sta_account_pg_0_watermark.shared_cells() << endl;
data <<"pbc_csr.port_5.sta_account_pg_0_watermark.total_cells: 0x" << pbc_csr.port_5.sta_account_pg_0_watermark.total_cells() << endl;

    pbc_csr.port_5.sta_account_pg_10_watermark.read();
    data <<"pbc_csr.port_5.sta_account_pg_10_watermark.all: 0x" << pbc_csr.port_5.sta_account_pg_10_watermark.all() << endl;
data <<"pbc_csr.port_5.sta_account_pg_10_watermark.shared_cells: 0x" << pbc_csr.port_5.sta_account_pg_10_watermark.shared_cells() << endl;
data <<"pbc_csr.port_5.sta_account_pg_10_watermark.total_cells: 0x" << pbc_csr.port_5.sta_account_pg_10_watermark.total_cells() << endl;

    pbc_csr.port_5.sta_account_pg_11_watermark.read();
    data <<"pbc_csr.port_5.sta_account_pg_11_watermark.all: 0x" << pbc_csr.port_5.sta_account_pg_11_watermark.all() << endl;
data <<"pbc_csr.port_5.sta_account_pg_11_watermark.shared_cells: 0x" << pbc_csr.port_5.sta_account_pg_11_watermark.shared_cells() << endl;
data <<"pbc_csr.port_5.sta_account_pg_11_watermark.total_cells: 0x" << pbc_csr.port_5.sta_account_pg_11_watermark.total_cells() << endl;

    pbc_csr.port_5.sta_account_pg_12_watermark.read();
    data <<"pbc_csr.port_5.sta_account_pg_12_watermark.all: 0x" << pbc_csr.port_5.sta_account_pg_12_watermark.all() << endl;
data <<"pbc_csr.port_5.sta_account_pg_12_watermark.shared_cells: 0x" << pbc_csr.port_5.sta_account_pg_12_watermark.shared_cells() << endl;
data <<"pbc_csr.port_5.sta_account_pg_12_watermark.total_cells: 0x" << pbc_csr.port_5.sta_account_pg_12_watermark.total_cells() << endl;

    pbc_csr.port_5.sta_account_pg_13_watermark.read();
    data <<"pbc_csr.port_5.sta_account_pg_13_watermark.all: 0x" << pbc_csr.port_5.sta_account_pg_13_watermark.all() << endl;
data <<"pbc_csr.port_5.sta_account_pg_13_watermark.shared_cells: 0x" << pbc_csr.port_5.sta_account_pg_13_watermark.shared_cells() << endl;
data <<"pbc_csr.port_5.sta_account_pg_13_watermark.total_cells: 0x" << pbc_csr.port_5.sta_account_pg_13_watermark.total_cells() << endl;

    pbc_csr.port_5.sta_account_pg_14_watermark.read();
    data <<"pbc_csr.port_5.sta_account_pg_14_watermark.all: 0x" << pbc_csr.port_5.sta_account_pg_14_watermark.all() << endl;
data <<"pbc_csr.port_5.sta_account_pg_14_watermark.shared_cells: 0x" << pbc_csr.port_5.sta_account_pg_14_watermark.shared_cells() << endl;
data <<"pbc_csr.port_5.sta_account_pg_14_watermark.total_cells: 0x" << pbc_csr.port_5.sta_account_pg_14_watermark.total_cells() << endl;

    pbc_csr.port_5.sta_account_pg_15_watermark.read();
    data <<"pbc_csr.port_5.sta_account_pg_15_watermark.all: 0x" << pbc_csr.port_5.sta_account_pg_15_watermark.all() << endl;
data <<"pbc_csr.port_5.sta_account_pg_15_watermark.shared_cells: 0x" << pbc_csr.port_5.sta_account_pg_15_watermark.shared_cells() << endl;
data <<"pbc_csr.port_5.sta_account_pg_15_watermark.total_cells: 0x" << pbc_csr.port_5.sta_account_pg_15_watermark.total_cells() << endl;

    pbc_csr.port_5.sta_account_pg_1_watermark.read();
    data <<"pbc_csr.port_5.sta_account_pg_1_watermark.all: 0x" << pbc_csr.port_5.sta_account_pg_1_watermark.all() << endl;
data <<"pbc_csr.port_5.sta_account_pg_1_watermark.shared_cells: 0x" << pbc_csr.port_5.sta_account_pg_1_watermark.shared_cells() << endl;
data <<"pbc_csr.port_5.sta_account_pg_1_watermark.total_cells: 0x" << pbc_csr.port_5.sta_account_pg_1_watermark.total_cells() << endl;

    pbc_csr.port_5.sta_account_pg_2_watermark.read();
    data <<"pbc_csr.port_5.sta_account_pg_2_watermark.all: 0x" << pbc_csr.port_5.sta_account_pg_2_watermark.all() << endl;
data <<"pbc_csr.port_5.sta_account_pg_2_watermark.shared_cells: 0x" << pbc_csr.port_5.sta_account_pg_2_watermark.shared_cells() << endl;
data <<"pbc_csr.port_5.sta_account_pg_2_watermark.total_cells: 0x" << pbc_csr.port_5.sta_account_pg_2_watermark.total_cells() << endl;

    pbc_csr.port_5.sta_account_pg_3_watermark.read();
    data <<"pbc_csr.port_5.sta_account_pg_3_watermark.all: 0x" << pbc_csr.port_5.sta_account_pg_3_watermark.all() << endl;
data <<"pbc_csr.port_5.sta_account_pg_3_watermark.shared_cells: 0x" << pbc_csr.port_5.sta_account_pg_3_watermark.shared_cells() << endl;
data <<"pbc_csr.port_5.sta_account_pg_3_watermark.total_cells: 0x" << pbc_csr.port_5.sta_account_pg_3_watermark.total_cells() << endl;

    pbc_csr.port_5.sta_account_pg_4_watermark.read();
    data <<"pbc_csr.port_5.sta_account_pg_4_watermark.all: 0x" << pbc_csr.port_5.sta_account_pg_4_watermark.all() << endl;
data <<"pbc_csr.port_5.sta_account_pg_4_watermark.shared_cells: 0x" << pbc_csr.port_5.sta_account_pg_4_watermark.shared_cells() << endl;
data <<"pbc_csr.port_5.sta_account_pg_4_watermark.total_cells: 0x" << pbc_csr.port_5.sta_account_pg_4_watermark.total_cells() << endl;

    pbc_csr.port_5.sta_account_pg_5_watermark.read();
    data <<"pbc_csr.port_5.sta_account_pg_5_watermark.all: 0x" << pbc_csr.port_5.sta_account_pg_5_watermark.all() << endl;
data <<"pbc_csr.port_5.sta_account_pg_5_watermark.shared_cells: 0x" << pbc_csr.port_5.sta_account_pg_5_watermark.shared_cells() << endl;
data <<"pbc_csr.port_5.sta_account_pg_5_watermark.total_cells: 0x" << pbc_csr.port_5.sta_account_pg_5_watermark.total_cells() << endl;

    pbc_csr.port_5.sta_account_pg_6_watermark.read();
    data <<"pbc_csr.port_5.sta_account_pg_6_watermark.all: 0x" << pbc_csr.port_5.sta_account_pg_6_watermark.all() << endl;
data <<"pbc_csr.port_5.sta_account_pg_6_watermark.shared_cells: 0x" << pbc_csr.port_5.sta_account_pg_6_watermark.shared_cells() << endl;
data <<"pbc_csr.port_5.sta_account_pg_6_watermark.total_cells: 0x" << pbc_csr.port_5.sta_account_pg_6_watermark.total_cells() << endl;

    pbc_csr.port_5.sta_account_pg_7_watermark.read();
    data <<"pbc_csr.port_5.sta_account_pg_7_watermark.all: 0x" << pbc_csr.port_5.sta_account_pg_7_watermark.all() << endl;
data <<"pbc_csr.port_5.sta_account_pg_7_watermark.shared_cells: 0x" << pbc_csr.port_5.sta_account_pg_7_watermark.shared_cells() << endl;
data <<"pbc_csr.port_5.sta_account_pg_7_watermark.total_cells: 0x" << pbc_csr.port_5.sta_account_pg_7_watermark.total_cells() << endl;

    pbc_csr.port_5.sta_account_pg_8_watermark.read();
    data <<"pbc_csr.port_5.sta_account_pg_8_watermark.all: 0x" << pbc_csr.port_5.sta_account_pg_8_watermark.all() << endl;
data <<"pbc_csr.port_5.sta_account_pg_8_watermark.shared_cells: 0x" << pbc_csr.port_5.sta_account_pg_8_watermark.shared_cells() << endl;
data <<"pbc_csr.port_5.sta_account_pg_8_watermark.total_cells: 0x" << pbc_csr.port_5.sta_account_pg_8_watermark.total_cells() << endl;

    pbc_csr.port_5.sta_account_pg_9_watermark.read();
    data <<"pbc_csr.port_5.sta_account_pg_9_watermark.all: 0x" << pbc_csr.port_5.sta_account_pg_9_watermark.all() << endl;
data <<"pbc_csr.port_5.sta_account_pg_9_watermark.shared_cells: 0x" << pbc_csr.port_5.sta_account_pg_9_watermark.shared_cells() << endl;
data <<"pbc_csr.port_5.sta_account_pg_9_watermark.total_cells: 0x" << pbc_csr.port_5.sta_account_pg_9_watermark.total_cells() << endl;

    pbc_csr.port_5.sta_write.read();
    data <<"pbc_csr.port_5.sta_write.all: 0x" << pbc_csr.port_5.sta_write.all() << endl;
data <<"pbc_csr.port_5.sta_write.cache_entries: 0x" << pbc_csr.port_5.sta_write.cache_entries() << endl;

    pbc_csr.port_6.cnt_axi_rpl.read();
    data <<"pbc_csr.port_6.cnt_axi_rpl.all: 0x" << pbc_csr.port_6.cnt_axi_rpl.all() << endl;
data <<"pbc_csr.port_6.cnt_axi_rpl.read: 0x" << pbc_csr.port_6.cnt_axi_rpl.read() << endl;

    pbc_csr.port_6.cnt_flits.read();
    data <<"pbc_csr.port_6.cnt_flits.all: 0x" << pbc_csr.port_6.cnt_flits.all() << endl;
data <<"pbc_csr.port_6.cnt_flits.eop_in: 0x" << pbc_csr.port_6.cnt_flits.eop_in() << endl;
data <<"pbc_csr.port_6.cnt_flits.srdy_no_drdy_in: 0x" << pbc_csr.port_6.cnt_flits.srdy_no_drdy_in() << endl;
data <<"pbc_csr.port_6.cnt_flits.sop_out: 0x" << pbc_csr.port_6.cnt_flits.sop_out() << endl;
data <<"pbc_csr.port_6.cnt_flits.eop_out: 0x" << pbc_csr.port_6.cnt_flits.eop_out() << endl;
data <<"pbc_csr.port_6.cnt_flits.sop_in: 0x" << pbc_csr.port_6.cnt_flits.sop_in() << endl;
data <<"pbc_csr.port_6.cnt_flits.srdy_no_drdy_out: 0x" << pbc_csr.port_6.cnt_flits.srdy_no_drdy_out() << endl;

    pbc_csr.port_6.cnt_write_error.read();
    data <<"pbc_csr.port_6.cnt_write_error.all: 0x" << pbc_csr.port_6.cnt_write_error.all() << endl;
data <<"pbc_csr.port_6.cnt_write_error.intrinsic_drop: 0x" << pbc_csr.port_6.cnt_write_error.intrinsic_drop() << endl;
data <<"pbc_csr.port_6.cnt_write_error.out_of_cells1: 0x" << pbc_csr.port_6.cnt_write_error.out_of_cells1() << endl;
data <<"pbc_csr.port_6.cnt_write_error.out_of_credit: 0x" << pbc_csr.port_6.cnt_write_error.out_of_credit() << endl;
data <<"pbc_csr.port_6.cnt_write_error.out_of_cells: 0x" << pbc_csr.port_6.cnt_write_error.out_of_cells() << endl;
data <<"pbc_csr.port_6.cnt_write_error.port_disabled: 0x" << pbc_csr.port_6.cnt_write_error.port_disabled() << endl;
data <<"pbc_csr.port_6.cnt_write_error.admitted: 0x" << pbc_csr.port_6.cnt_write_error.admitted() << endl;
data <<"pbc_csr.port_6.cnt_write_error.discarded: 0x" << pbc_csr.port_6.cnt_write_error.discarded() << endl;
data <<"pbc_csr.port_6.cnt_write_error.enqueue: 0x" << pbc_csr.port_6.cnt_write_error.enqueue() << endl;
data <<"pbc_csr.port_6.cnt_write_error.oq_range: 0x" << pbc_csr.port_6.cnt_write_error.oq_range() << endl;
data <<"pbc_csr.port_6.cnt_write_error.tail_drop_span: 0x" << pbc_csr.port_6.cnt_write_error.tail_drop_span() << endl;
data <<"pbc_csr.port_6.cnt_write_error.port_range: 0x" << pbc_csr.port_6.cnt_write_error.port_range() << endl;
data <<"pbc_csr.port_6.cnt_write_error.tail_drop_cpu: 0x" << pbc_csr.port_6.cnt_write_error.tail_drop_cpu() << endl;
data <<"pbc_csr.port_6.cnt_write_error.no_dest: 0x" << pbc_csr.port_6.cnt_write_error.no_dest() << endl;
data <<"pbc_csr.port_6.cnt_write_error.min_size_viol: 0x" << pbc_csr.port_6.cnt_write_error.min_size_viol() << endl;
data <<"pbc_csr.port_6.cnt_write_error.truncation: 0x" << pbc_csr.port_6.cnt_write_error.truncation() << endl;

    pbc_csr.port_6.sta_account.read();
    data <<"pbc_csr.port_6.sta_account.all: 0x" << pbc_csr.port_6.sta_account.all() << endl;
data <<"pbc_csr.port_6.sta_account.occupancy_5: 0x" << pbc_csr.port_6.sta_account.occupancy_5() << endl;
data <<"pbc_csr.port_6.sta_account.occupancy_4: 0x" << pbc_csr.port_6.sta_account.occupancy_4() << endl;
data <<"pbc_csr.port_6.sta_account.occupancy_7: 0x" << pbc_csr.port_6.sta_account.occupancy_7() << endl;
data <<"pbc_csr.port_6.sta_account.occupancy_6: 0x" << pbc_csr.port_6.sta_account.occupancy_6() << endl;
data <<"pbc_csr.port_6.sta_account.occupancy_1: 0x" << pbc_csr.port_6.sta_account.occupancy_1() << endl;
data <<"pbc_csr.port_6.sta_account.occupancy_0: 0x" << pbc_csr.port_6.sta_account.occupancy_0() << endl;
data <<"pbc_csr.port_6.sta_account.occupancy_3: 0x" << pbc_csr.port_6.sta_account.occupancy_3() << endl;
data <<"pbc_csr.port_6.sta_account.occupancy_2: 0x" << pbc_csr.port_6.sta_account.occupancy_2() << endl;
data <<"pbc_csr.port_6.sta_account.sp_held_5: 0x" << pbc_csr.port_6.sta_account.sp_held_5() << endl;
data <<"pbc_csr.port_6.sta_account.sp_held_4: 0x" << pbc_csr.port_6.sta_account.sp_held_4() << endl;
data <<"pbc_csr.port_6.sta_account.sp_held_7: 0x" << pbc_csr.port_6.sta_account.sp_held_7() << endl;
data <<"pbc_csr.port_6.sta_account.sp_held_6: 0x" << pbc_csr.port_6.sta_account.sp_held_6() << endl;
data <<"pbc_csr.port_6.sta_account.sp_held_1: 0x" << pbc_csr.port_6.sta_account.sp_held_1() << endl;
data <<"pbc_csr.port_6.sta_account.sp_held_0: 0x" << pbc_csr.port_6.sta_account.sp_held_0() << endl;
data <<"pbc_csr.port_6.sta_account.sp_held_3: 0x" << pbc_csr.port_6.sta_account.sp_held_3() << endl;
data <<"pbc_csr.port_6.sta_account.sp_held_2: 0x" << pbc_csr.port_6.sta_account.sp_held_2() << endl;
data <<"pbc_csr.port_6.sta_account.occupancy_28: 0x" << pbc_csr.port_6.sta_account.occupancy_28() << endl;
data <<"pbc_csr.port_6.sta_account.occupancy_29: 0x" << pbc_csr.port_6.sta_account.occupancy_29() << endl;
data <<"pbc_csr.port_6.sta_account.occupancy_21: 0x" << pbc_csr.port_6.sta_account.occupancy_21() << endl;
data <<"pbc_csr.port_6.sta_account.occupancy_20: 0x" << pbc_csr.port_6.sta_account.occupancy_20() << endl;
data <<"pbc_csr.port_6.sta_account.sp_held_9: 0x" << pbc_csr.port_6.sta_account.sp_held_9() << endl;
data <<"pbc_csr.port_6.sta_account.occupancy_22: 0x" << pbc_csr.port_6.sta_account.occupancy_22() << endl;
data <<"pbc_csr.port_6.sta_account.occupancy_23: 0x" << pbc_csr.port_6.sta_account.occupancy_23() << endl;
data <<"pbc_csr.port_6.sta_account.occupancy_24: 0x" << pbc_csr.port_6.sta_account.occupancy_24() << endl;
data <<"pbc_csr.port_6.sta_account.occupancy_25: 0x" << pbc_csr.port_6.sta_account.occupancy_25() << endl;
data <<"pbc_csr.port_6.sta_account.occupancy_26: 0x" << pbc_csr.port_6.sta_account.occupancy_26() << endl;
data <<"pbc_csr.port_6.sta_account.sp_held_8: 0x" << pbc_csr.port_6.sta_account.sp_held_8() << endl;
data <<"pbc_csr.port_6.sta_account.sp_held_28: 0x" << pbc_csr.port_6.sta_account.sp_held_28() << endl;
data <<"pbc_csr.port_6.sta_account.sp_held_29: 0x" << pbc_csr.port_6.sta_account.sp_held_29() << endl;
data <<"pbc_csr.port_6.sta_account.sp_held_20: 0x" << pbc_csr.port_6.sta_account.sp_held_20() << endl;
data <<"pbc_csr.port_6.sta_account.sp_held_21: 0x" << pbc_csr.port_6.sta_account.sp_held_21() << endl;
data <<"pbc_csr.port_6.sta_account.sp_held_22: 0x" << pbc_csr.port_6.sta_account.sp_held_22() << endl;
data <<"pbc_csr.port_6.sta_account.sp_held_23: 0x" << pbc_csr.port_6.sta_account.sp_held_23() << endl;
data <<"pbc_csr.port_6.sta_account.sp_held_24: 0x" << pbc_csr.port_6.sta_account.sp_held_24() << endl;
data <<"pbc_csr.port_6.sta_account.sp_held_25: 0x" << pbc_csr.port_6.sta_account.sp_held_25() << endl;
data <<"pbc_csr.port_6.sta_account.sp_held_26: 0x" << pbc_csr.port_6.sta_account.sp_held_26() << endl;
data <<"pbc_csr.port_6.sta_account.sp_held_27: 0x" << pbc_csr.port_6.sta_account.sp_held_27() << endl;
data <<"pbc_csr.port_6.sta_account.occupancy_27: 0x" << pbc_csr.port_6.sta_account.occupancy_27() << endl;
data <<"pbc_csr.port_6.sta_account.occupancy_9: 0x" << pbc_csr.port_6.sta_account.occupancy_9() << endl;
data <<"pbc_csr.port_6.sta_account.occupancy_8: 0x" << pbc_csr.port_6.sta_account.occupancy_8() << endl;
data <<"pbc_csr.port_6.sta_account.occupancy_11: 0x" << pbc_csr.port_6.sta_account.occupancy_11() << endl;
data <<"pbc_csr.port_6.sta_account.occupancy_10: 0x" << pbc_csr.port_6.sta_account.occupancy_10() << endl;
data <<"pbc_csr.port_6.sta_account.occupancy_13: 0x" << pbc_csr.port_6.sta_account.occupancy_13() << endl;
data <<"pbc_csr.port_6.sta_account.occupancy_12: 0x" << pbc_csr.port_6.sta_account.occupancy_12() << endl;
data <<"pbc_csr.port_6.sta_account.occupancy_15: 0x" << pbc_csr.port_6.sta_account.occupancy_15() << endl;
data <<"pbc_csr.port_6.sta_account.occupancy_14: 0x" << pbc_csr.port_6.sta_account.occupancy_14() << endl;
data <<"pbc_csr.port_6.sta_account.occupancy_17: 0x" << pbc_csr.port_6.sta_account.occupancy_17() << endl;
data <<"pbc_csr.port_6.sta_account.occupancy_16: 0x" << pbc_csr.port_6.sta_account.occupancy_16() << endl;
data <<"pbc_csr.port_6.sta_account.occupancy_19: 0x" << pbc_csr.port_6.sta_account.occupancy_19() << endl;
data <<"pbc_csr.port_6.sta_account.occupancy_18: 0x" << pbc_csr.port_6.sta_account.occupancy_18() << endl;
data <<"pbc_csr.port_6.sta_account.occupancy_31: 0x" << pbc_csr.port_6.sta_account.occupancy_31() << endl;
data <<"pbc_csr.port_6.sta_account.occupancy_30: 0x" << pbc_csr.port_6.sta_account.occupancy_30() << endl;
data <<"pbc_csr.port_6.sta_account.sp_held_31: 0x" << pbc_csr.port_6.sta_account.sp_held_31() << endl;
data <<"pbc_csr.port_6.sta_account.sp_held_30: 0x" << pbc_csr.port_6.sta_account.sp_held_30() << endl;
data <<"pbc_csr.port_6.sta_account.sp_held_15: 0x" << pbc_csr.port_6.sta_account.sp_held_15() << endl;
data <<"pbc_csr.port_6.sta_account.sp_held_14: 0x" << pbc_csr.port_6.sta_account.sp_held_14() << endl;
data <<"pbc_csr.port_6.sta_account.sp_held_17: 0x" << pbc_csr.port_6.sta_account.sp_held_17() << endl;
data <<"pbc_csr.port_6.sta_account.sp_held_16: 0x" << pbc_csr.port_6.sta_account.sp_held_16() << endl;
data <<"pbc_csr.port_6.sta_account.sp_held_11: 0x" << pbc_csr.port_6.sta_account.sp_held_11() << endl;
data <<"pbc_csr.port_6.sta_account.sp_held_10: 0x" << pbc_csr.port_6.sta_account.sp_held_10() << endl;
data <<"pbc_csr.port_6.sta_account.sp_held_13: 0x" << pbc_csr.port_6.sta_account.sp_held_13() << endl;
data <<"pbc_csr.port_6.sta_account.sp_held_12: 0x" << pbc_csr.port_6.sta_account.sp_held_12() << endl;
data <<"pbc_csr.port_6.sta_account.sp_held_19: 0x" << pbc_csr.port_6.sta_account.sp_held_19() << endl;
data <<"pbc_csr.port_6.sta_account.sp_held_18: 0x" << pbc_csr.port_6.sta_account.sp_held_18() << endl;

    pbc_csr.port_6.sta_account_pg_0_watermark.read();
    data <<"pbc_csr.port_6.sta_account_pg_0_watermark.all: 0x" << pbc_csr.port_6.sta_account_pg_0_watermark.all() << endl;
data <<"pbc_csr.port_6.sta_account_pg_0_watermark.shared_cells: 0x" << pbc_csr.port_6.sta_account_pg_0_watermark.shared_cells() << endl;
data <<"pbc_csr.port_6.sta_account_pg_0_watermark.total_cells: 0x" << pbc_csr.port_6.sta_account_pg_0_watermark.total_cells() << endl;

    pbc_csr.port_6.sta_account_pg_10_watermark.read();
    data <<"pbc_csr.port_6.sta_account_pg_10_watermark.all: 0x" << pbc_csr.port_6.sta_account_pg_10_watermark.all() << endl;
data <<"pbc_csr.port_6.sta_account_pg_10_watermark.shared_cells: 0x" << pbc_csr.port_6.sta_account_pg_10_watermark.shared_cells() << endl;
data <<"pbc_csr.port_6.sta_account_pg_10_watermark.total_cells: 0x" << pbc_csr.port_6.sta_account_pg_10_watermark.total_cells() << endl;

    pbc_csr.port_6.sta_account_pg_11_watermark.read();
    data <<"pbc_csr.port_6.sta_account_pg_11_watermark.all: 0x" << pbc_csr.port_6.sta_account_pg_11_watermark.all() << endl;
data <<"pbc_csr.port_6.sta_account_pg_11_watermark.shared_cells: 0x" << pbc_csr.port_6.sta_account_pg_11_watermark.shared_cells() << endl;
data <<"pbc_csr.port_6.sta_account_pg_11_watermark.total_cells: 0x" << pbc_csr.port_6.sta_account_pg_11_watermark.total_cells() << endl;

    pbc_csr.port_6.sta_account_pg_12_watermark.read();
    data <<"pbc_csr.port_6.sta_account_pg_12_watermark.all: 0x" << pbc_csr.port_6.sta_account_pg_12_watermark.all() << endl;
data <<"pbc_csr.port_6.sta_account_pg_12_watermark.shared_cells: 0x" << pbc_csr.port_6.sta_account_pg_12_watermark.shared_cells() << endl;
data <<"pbc_csr.port_6.sta_account_pg_12_watermark.total_cells: 0x" << pbc_csr.port_6.sta_account_pg_12_watermark.total_cells() << endl;

    pbc_csr.port_6.sta_account_pg_13_watermark.read();
    data <<"pbc_csr.port_6.sta_account_pg_13_watermark.all: 0x" << pbc_csr.port_6.sta_account_pg_13_watermark.all() << endl;
data <<"pbc_csr.port_6.sta_account_pg_13_watermark.shared_cells: 0x" << pbc_csr.port_6.sta_account_pg_13_watermark.shared_cells() << endl;
data <<"pbc_csr.port_6.sta_account_pg_13_watermark.total_cells: 0x" << pbc_csr.port_6.sta_account_pg_13_watermark.total_cells() << endl;

    pbc_csr.port_6.sta_account_pg_14_watermark.read();
    data <<"pbc_csr.port_6.sta_account_pg_14_watermark.all: 0x" << pbc_csr.port_6.sta_account_pg_14_watermark.all() << endl;
data <<"pbc_csr.port_6.sta_account_pg_14_watermark.shared_cells: 0x" << pbc_csr.port_6.sta_account_pg_14_watermark.shared_cells() << endl;
data <<"pbc_csr.port_6.sta_account_pg_14_watermark.total_cells: 0x" << pbc_csr.port_6.sta_account_pg_14_watermark.total_cells() << endl;

    pbc_csr.port_6.sta_account_pg_15_watermark.read();
    data <<"pbc_csr.port_6.sta_account_pg_15_watermark.all: 0x" << pbc_csr.port_6.sta_account_pg_15_watermark.all() << endl;
data <<"pbc_csr.port_6.sta_account_pg_15_watermark.shared_cells: 0x" << pbc_csr.port_6.sta_account_pg_15_watermark.shared_cells() << endl;
data <<"pbc_csr.port_6.sta_account_pg_15_watermark.total_cells: 0x" << pbc_csr.port_6.sta_account_pg_15_watermark.total_cells() << endl;

    pbc_csr.port_6.sta_account_pg_16_watermark.read();
    data <<"pbc_csr.port_6.sta_account_pg_16_watermark.all: 0x" << pbc_csr.port_6.sta_account_pg_16_watermark.all() << endl;
data <<"pbc_csr.port_6.sta_account_pg_16_watermark.shared_cells: 0x" << pbc_csr.port_6.sta_account_pg_16_watermark.shared_cells() << endl;
data <<"pbc_csr.port_6.sta_account_pg_16_watermark.total_cells: 0x" << pbc_csr.port_6.sta_account_pg_16_watermark.total_cells() << endl;

    pbc_csr.port_6.sta_account_pg_17_watermark.read();
    data <<"pbc_csr.port_6.sta_account_pg_17_watermark.all: 0x" << pbc_csr.port_6.sta_account_pg_17_watermark.all() << endl;
data <<"pbc_csr.port_6.sta_account_pg_17_watermark.shared_cells: 0x" << pbc_csr.port_6.sta_account_pg_17_watermark.shared_cells() << endl;
data <<"pbc_csr.port_6.sta_account_pg_17_watermark.total_cells: 0x" << pbc_csr.port_6.sta_account_pg_17_watermark.total_cells() << endl;

    pbc_csr.port_6.sta_account_pg_18_watermark.read();
    data <<"pbc_csr.port_6.sta_account_pg_18_watermark.all: 0x" << pbc_csr.port_6.sta_account_pg_18_watermark.all() << endl;
data <<"pbc_csr.port_6.sta_account_pg_18_watermark.shared_cells: 0x" << pbc_csr.port_6.sta_account_pg_18_watermark.shared_cells() << endl;
data <<"pbc_csr.port_6.sta_account_pg_18_watermark.total_cells: 0x" << pbc_csr.port_6.sta_account_pg_18_watermark.total_cells() << endl;

    pbc_csr.port_6.sta_account_pg_19_watermark.read();
    data <<"pbc_csr.port_6.sta_account_pg_19_watermark.all: 0x" << pbc_csr.port_6.sta_account_pg_19_watermark.all() << endl;
data <<"pbc_csr.port_6.sta_account_pg_19_watermark.shared_cells: 0x" << pbc_csr.port_6.sta_account_pg_19_watermark.shared_cells() << endl;
data <<"pbc_csr.port_6.sta_account_pg_19_watermark.total_cells: 0x" << pbc_csr.port_6.sta_account_pg_19_watermark.total_cells() << endl;

    pbc_csr.port_6.sta_account_pg_1_watermark.read();
    data <<"pbc_csr.port_6.sta_account_pg_1_watermark.all: 0x" << pbc_csr.port_6.sta_account_pg_1_watermark.all() << endl;
data <<"pbc_csr.port_6.sta_account_pg_1_watermark.shared_cells: 0x" << pbc_csr.port_6.sta_account_pg_1_watermark.shared_cells() << endl;
data <<"pbc_csr.port_6.sta_account_pg_1_watermark.total_cells: 0x" << pbc_csr.port_6.sta_account_pg_1_watermark.total_cells() << endl;

    pbc_csr.port_6.sta_account_pg_20_watermark.read();
    data <<"pbc_csr.port_6.sta_account_pg_20_watermark.all: 0x" << pbc_csr.port_6.sta_account_pg_20_watermark.all() << endl;
data <<"pbc_csr.port_6.sta_account_pg_20_watermark.shared_cells: 0x" << pbc_csr.port_6.sta_account_pg_20_watermark.shared_cells() << endl;
data <<"pbc_csr.port_6.sta_account_pg_20_watermark.total_cells: 0x" << pbc_csr.port_6.sta_account_pg_20_watermark.total_cells() << endl;

    pbc_csr.port_6.sta_account_pg_21_watermark.read();
    data <<"pbc_csr.port_6.sta_account_pg_21_watermark.all: 0x" << pbc_csr.port_6.sta_account_pg_21_watermark.all() << endl;
data <<"pbc_csr.port_6.sta_account_pg_21_watermark.shared_cells: 0x" << pbc_csr.port_6.sta_account_pg_21_watermark.shared_cells() << endl;
data <<"pbc_csr.port_6.sta_account_pg_21_watermark.total_cells: 0x" << pbc_csr.port_6.sta_account_pg_21_watermark.total_cells() << endl;

    pbc_csr.port_6.sta_account_pg_22_watermark.read();
    data <<"pbc_csr.port_6.sta_account_pg_22_watermark.all: 0x" << pbc_csr.port_6.sta_account_pg_22_watermark.all() << endl;
data <<"pbc_csr.port_6.sta_account_pg_22_watermark.shared_cells: 0x" << pbc_csr.port_6.sta_account_pg_22_watermark.shared_cells() << endl;
data <<"pbc_csr.port_6.sta_account_pg_22_watermark.total_cells: 0x" << pbc_csr.port_6.sta_account_pg_22_watermark.total_cells() << endl;

    pbc_csr.port_6.sta_account_pg_23_watermark.read();
    data <<"pbc_csr.port_6.sta_account_pg_23_watermark.all: 0x" << pbc_csr.port_6.sta_account_pg_23_watermark.all() << endl;
data <<"pbc_csr.port_6.sta_account_pg_23_watermark.shared_cells: 0x" << pbc_csr.port_6.sta_account_pg_23_watermark.shared_cells() << endl;
data <<"pbc_csr.port_6.sta_account_pg_23_watermark.total_cells: 0x" << pbc_csr.port_6.sta_account_pg_23_watermark.total_cells() << endl;

    pbc_csr.port_6.sta_account_pg_24_watermark.read();
    data <<"pbc_csr.port_6.sta_account_pg_24_watermark.all: 0x" << pbc_csr.port_6.sta_account_pg_24_watermark.all() << endl;
data <<"pbc_csr.port_6.sta_account_pg_24_watermark.shared_cells: 0x" << pbc_csr.port_6.sta_account_pg_24_watermark.shared_cells() << endl;
data <<"pbc_csr.port_6.sta_account_pg_24_watermark.total_cells: 0x" << pbc_csr.port_6.sta_account_pg_24_watermark.total_cells() << endl;

    pbc_csr.port_6.sta_account_pg_25_watermark.read();
    data <<"pbc_csr.port_6.sta_account_pg_25_watermark.all: 0x" << pbc_csr.port_6.sta_account_pg_25_watermark.all() << endl;
data <<"pbc_csr.port_6.sta_account_pg_25_watermark.shared_cells: 0x" << pbc_csr.port_6.sta_account_pg_25_watermark.shared_cells() << endl;
data <<"pbc_csr.port_6.sta_account_pg_25_watermark.total_cells: 0x" << pbc_csr.port_6.sta_account_pg_25_watermark.total_cells() << endl;

    pbc_csr.port_6.sta_account_pg_26_watermark.read();
    data <<"pbc_csr.port_6.sta_account_pg_26_watermark.all: 0x" << pbc_csr.port_6.sta_account_pg_26_watermark.all() << endl;
data <<"pbc_csr.port_6.sta_account_pg_26_watermark.shared_cells: 0x" << pbc_csr.port_6.sta_account_pg_26_watermark.shared_cells() << endl;
data <<"pbc_csr.port_6.sta_account_pg_26_watermark.total_cells: 0x" << pbc_csr.port_6.sta_account_pg_26_watermark.total_cells() << endl;

    pbc_csr.port_6.sta_account_pg_27_watermark.read();
    data <<"pbc_csr.port_6.sta_account_pg_27_watermark.all: 0x" << pbc_csr.port_6.sta_account_pg_27_watermark.all() << endl;
data <<"pbc_csr.port_6.sta_account_pg_27_watermark.shared_cells: 0x" << pbc_csr.port_6.sta_account_pg_27_watermark.shared_cells() << endl;
data <<"pbc_csr.port_6.sta_account_pg_27_watermark.total_cells: 0x" << pbc_csr.port_6.sta_account_pg_27_watermark.total_cells() << endl;

    pbc_csr.port_6.sta_account_pg_28_watermark.read();
    data <<"pbc_csr.port_6.sta_account_pg_28_watermark.all: 0x" << pbc_csr.port_6.sta_account_pg_28_watermark.all() << endl;
data <<"pbc_csr.port_6.sta_account_pg_28_watermark.shared_cells: 0x" << pbc_csr.port_6.sta_account_pg_28_watermark.shared_cells() << endl;
data <<"pbc_csr.port_6.sta_account_pg_28_watermark.total_cells: 0x" << pbc_csr.port_6.sta_account_pg_28_watermark.total_cells() << endl;

    pbc_csr.port_6.sta_account_pg_29_watermark.read();
    data <<"pbc_csr.port_6.sta_account_pg_29_watermark.all: 0x" << pbc_csr.port_6.sta_account_pg_29_watermark.all() << endl;
data <<"pbc_csr.port_6.sta_account_pg_29_watermark.shared_cells: 0x" << pbc_csr.port_6.sta_account_pg_29_watermark.shared_cells() << endl;
data <<"pbc_csr.port_6.sta_account_pg_29_watermark.total_cells: 0x" << pbc_csr.port_6.sta_account_pg_29_watermark.total_cells() << endl;

    pbc_csr.port_6.sta_account_pg_2_watermark.read();
    data <<"pbc_csr.port_6.sta_account_pg_2_watermark.all: 0x" << pbc_csr.port_6.sta_account_pg_2_watermark.all() << endl;
data <<"pbc_csr.port_6.sta_account_pg_2_watermark.shared_cells: 0x" << pbc_csr.port_6.sta_account_pg_2_watermark.shared_cells() << endl;
data <<"pbc_csr.port_6.sta_account_pg_2_watermark.total_cells: 0x" << pbc_csr.port_6.sta_account_pg_2_watermark.total_cells() << endl;

    pbc_csr.port_6.sta_account_pg_30_watermark.read();
    data <<"pbc_csr.port_6.sta_account_pg_30_watermark.all: 0x" << pbc_csr.port_6.sta_account_pg_30_watermark.all() << endl;
data <<"pbc_csr.port_6.sta_account_pg_30_watermark.shared_cells: 0x" << pbc_csr.port_6.sta_account_pg_30_watermark.shared_cells() << endl;
data <<"pbc_csr.port_6.sta_account_pg_30_watermark.total_cells: 0x" << pbc_csr.port_6.sta_account_pg_30_watermark.total_cells() << endl;

    pbc_csr.port_6.sta_account_pg_31_watermark.read();
    data <<"pbc_csr.port_6.sta_account_pg_31_watermark.all: 0x" << pbc_csr.port_6.sta_account_pg_31_watermark.all() << endl;
data <<"pbc_csr.port_6.sta_account_pg_31_watermark.shared_cells: 0x" << pbc_csr.port_6.sta_account_pg_31_watermark.shared_cells() << endl;
data <<"pbc_csr.port_6.sta_account_pg_31_watermark.total_cells: 0x" << pbc_csr.port_6.sta_account_pg_31_watermark.total_cells() << endl;

    pbc_csr.port_6.sta_account_pg_3_watermark.read();
    data <<"pbc_csr.port_6.sta_account_pg_3_watermark.all: 0x" << pbc_csr.port_6.sta_account_pg_3_watermark.all() << endl;
data <<"pbc_csr.port_6.sta_account_pg_3_watermark.shared_cells: 0x" << pbc_csr.port_6.sta_account_pg_3_watermark.shared_cells() << endl;
data <<"pbc_csr.port_6.sta_account_pg_3_watermark.total_cells: 0x" << pbc_csr.port_6.sta_account_pg_3_watermark.total_cells() << endl;

    pbc_csr.port_6.sta_account_pg_4_watermark.read();
    data <<"pbc_csr.port_6.sta_account_pg_4_watermark.all: 0x" << pbc_csr.port_6.sta_account_pg_4_watermark.all() << endl;
data <<"pbc_csr.port_6.sta_account_pg_4_watermark.shared_cells: 0x" << pbc_csr.port_6.sta_account_pg_4_watermark.shared_cells() << endl;
data <<"pbc_csr.port_6.sta_account_pg_4_watermark.total_cells: 0x" << pbc_csr.port_6.sta_account_pg_4_watermark.total_cells() << endl;

    pbc_csr.port_6.sta_account_pg_5_watermark.read();
    data <<"pbc_csr.port_6.sta_account_pg_5_watermark.all: 0x" << pbc_csr.port_6.sta_account_pg_5_watermark.all() << endl;
data <<"pbc_csr.port_6.sta_account_pg_5_watermark.shared_cells: 0x" << pbc_csr.port_6.sta_account_pg_5_watermark.shared_cells() << endl;
data <<"pbc_csr.port_6.sta_account_pg_5_watermark.total_cells: 0x" << pbc_csr.port_6.sta_account_pg_5_watermark.total_cells() << endl;

    pbc_csr.port_6.sta_account_pg_6_watermark.read();
    data <<"pbc_csr.port_6.sta_account_pg_6_watermark.all: 0x" << pbc_csr.port_6.sta_account_pg_6_watermark.all() << endl;
data <<"pbc_csr.port_6.sta_account_pg_6_watermark.shared_cells: 0x" << pbc_csr.port_6.sta_account_pg_6_watermark.shared_cells() << endl;
data <<"pbc_csr.port_6.sta_account_pg_6_watermark.total_cells: 0x" << pbc_csr.port_6.sta_account_pg_6_watermark.total_cells() << endl;

    pbc_csr.port_6.sta_account_pg_7_watermark.read();
    data <<"pbc_csr.port_6.sta_account_pg_7_watermark.all: 0x" << pbc_csr.port_6.sta_account_pg_7_watermark.all() << endl;
data <<"pbc_csr.port_6.sta_account_pg_7_watermark.shared_cells: 0x" << pbc_csr.port_6.sta_account_pg_7_watermark.shared_cells() << endl;
data <<"pbc_csr.port_6.sta_account_pg_7_watermark.total_cells: 0x" << pbc_csr.port_6.sta_account_pg_7_watermark.total_cells() << endl;

    pbc_csr.port_6.sta_account_pg_8_watermark.read();
    data <<"pbc_csr.port_6.sta_account_pg_8_watermark.all: 0x" << pbc_csr.port_6.sta_account_pg_8_watermark.all() << endl;
data <<"pbc_csr.port_6.sta_account_pg_8_watermark.shared_cells: 0x" << pbc_csr.port_6.sta_account_pg_8_watermark.shared_cells() << endl;
data <<"pbc_csr.port_6.sta_account_pg_8_watermark.total_cells: 0x" << pbc_csr.port_6.sta_account_pg_8_watermark.total_cells() << endl;

    pbc_csr.port_6.sta_account_pg_9_watermark.read();
    data <<"pbc_csr.port_6.sta_account_pg_9_watermark.all: 0x" << pbc_csr.port_6.sta_account_pg_9_watermark.all() << endl;
data <<"pbc_csr.port_6.sta_account_pg_9_watermark.shared_cells: 0x" << pbc_csr.port_6.sta_account_pg_9_watermark.shared_cells() << endl;
data <<"pbc_csr.port_6.sta_account_pg_9_watermark.total_cells: 0x" << pbc_csr.port_6.sta_account_pg_9_watermark.total_cells() << endl;

    pbc_csr.port_6.sta_write.read();
    data <<"pbc_csr.port_6.sta_write.all: 0x" << pbc_csr.port_6.sta_write.all() << endl;
data <<"pbc_csr.port_6.sta_write.cache_entries: 0x" << pbc_csr.port_6.sta_write.cache_entries() << endl;

    pbc_csr.port_7.cnt_axi_rpl.read();
    data <<"pbc_csr.port_7.cnt_axi_rpl.all: 0x" << pbc_csr.port_7.cnt_axi_rpl.all() << endl;
data <<"pbc_csr.port_7.cnt_axi_rpl.read: 0x" << pbc_csr.port_7.cnt_axi_rpl.read() << endl;

    pbc_csr.port_7.cnt_flits.read();
    data <<"pbc_csr.port_7.cnt_flits.all: 0x" << pbc_csr.port_7.cnt_flits.all() << endl;
data <<"pbc_csr.port_7.cnt_flits.eop_in: 0x" << pbc_csr.port_7.cnt_flits.eop_in() << endl;
data <<"pbc_csr.port_7.cnt_flits.srdy_no_drdy_in: 0x" << pbc_csr.port_7.cnt_flits.srdy_no_drdy_in() << endl;
data <<"pbc_csr.port_7.cnt_flits.sop_out: 0x" << pbc_csr.port_7.cnt_flits.sop_out() << endl;
data <<"pbc_csr.port_7.cnt_flits.eop_out: 0x" << pbc_csr.port_7.cnt_flits.eop_out() << endl;
data <<"pbc_csr.port_7.cnt_flits.sop_in: 0x" << pbc_csr.port_7.cnt_flits.sop_in() << endl;
data <<"pbc_csr.port_7.cnt_flits.srdy_no_drdy_out: 0x" << pbc_csr.port_7.cnt_flits.srdy_no_drdy_out() << endl;

    pbc_csr.port_7.cnt_write_error.read();
    data <<"pbc_csr.port_7.cnt_write_error.all: 0x" << pbc_csr.port_7.cnt_write_error.all() << endl;
data <<"pbc_csr.port_7.cnt_write_error.intrinsic_drop: 0x" << pbc_csr.port_7.cnt_write_error.intrinsic_drop() << endl;
data <<"pbc_csr.port_7.cnt_write_error.out_of_cells1: 0x" << pbc_csr.port_7.cnt_write_error.out_of_cells1() << endl;
data <<"pbc_csr.port_7.cnt_write_error.out_of_credit: 0x" << pbc_csr.port_7.cnt_write_error.out_of_credit() << endl;
data <<"pbc_csr.port_7.cnt_write_error.out_of_cells: 0x" << pbc_csr.port_7.cnt_write_error.out_of_cells() << endl;
data <<"pbc_csr.port_7.cnt_write_error.port_disabled: 0x" << pbc_csr.port_7.cnt_write_error.port_disabled() << endl;
data <<"pbc_csr.port_7.cnt_write_error.admitted: 0x" << pbc_csr.port_7.cnt_write_error.admitted() << endl;
data <<"pbc_csr.port_7.cnt_write_error.discarded: 0x" << pbc_csr.port_7.cnt_write_error.discarded() << endl;
data <<"pbc_csr.port_7.cnt_write_error.enqueue: 0x" << pbc_csr.port_7.cnt_write_error.enqueue() << endl;
data <<"pbc_csr.port_7.cnt_write_error.oq_range: 0x" << pbc_csr.port_7.cnt_write_error.oq_range() << endl;
data <<"pbc_csr.port_7.cnt_write_error.tail_drop_span: 0x" << pbc_csr.port_7.cnt_write_error.tail_drop_span() << endl;
data <<"pbc_csr.port_7.cnt_write_error.port_range: 0x" << pbc_csr.port_7.cnt_write_error.port_range() << endl;
data <<"pbc_csr.port_7.cnt_write_error.tail_drop_cpu: 0x" << pbc_csr.port_7.cnt_write_error.tail_drop_cpu() << endl;
data <<"pbc_csr.port_7.cnt_write_error.no_dest: 0x" << pbc_csr.port_7.cnt_write_error.no_dest() << endl;
data <<"pbc_csr.port_7.cnt_write_error.min_size_viol: 0x" << pbc_csr.port_7.cnt_write_error.min_size_viol() << endl;
data <<"pbc_csr.port_7.cnt_write_error.truncation: 0x" << pbc_csr.port_7.cnt_write_error.truncation() << endl;

    pbc_csr.port_7.sta_account.read();
    data <<"pbc_csr.port_7.sta_account.all: 0x" << pbc_csr.port_7.sta_account.all() << endl;
data <<"pbc_csr.port_7.sta_account.occupancy_5: 0x" << pbc_csr.port_7.sta_account.occupancy_5() << endl;
data <<"pbc_csr.port_7.sta_account.occupancy_4: 0x" << pbc_csr.port_7.sta_account.occupancy_4() << endl;
data <<"pbc_csr.port_7.sta_account.occupancy_7: 0x" << pbc_csr.port_7.sta_account.occupancy_7() << endl;
data <<"pbc_csr.port_7.sta_account.occupancy_6: 0x" << pbc_csr.port_7.sta_account.occupancy_6() << endl;
data <<"pbc_csr.port_7.sta_account.occupancy_1: 0x" << pbc_csr.port_7.sta_account.occupancy_1() << endl;
data <<"pbc_csr.port_7.sta_account.occupancy_0: 0x" << pbc_csr.port_7.sta_account.occupancy_0() << endl;
data <<"pbc_csr.port_7.sta_account.occupancy_3: 0x" << pbc_csr.port_7.sta_account.occupancy_3() << endl;
data <<"pbc_csr.port_7.sta_account.occupancy_2: 0x" << pbc_csr.port_7.sta_account.occupancy_2() << endl;
data <<"pbc_csr.port_7.sta_account.sp_held_5: 0x" << pbc_csr.port_7.sta_account.sp_held_5() << endl;
data <<"pbc_csr.port_7.sta_account.sp_held_4: 0x" << pbc_csr.port_7.sta_account.sp_held_4() << endl;
data <<"pbc_csr.port_7.sta_account.sp_held_7: 0x" << pbc_csr.port_7.sta_account.sp_held_7() << endl;
data <<"pbc_csr.port_7.sta_account.sp_held_6: 0x" << pbc_csr.port_7.sta_account.sp_held_6() << endl;
data <<"pbc_csr.port_7.sta_account.sp_held_1: 0x" << pbc_csr.port_7.sta_account.sp_held_1() << endl;
data <<"pbc_csr.port_7.sta_account.sp_held_0: 0x" << pbc_csr.port_7.sta_account.sp_held_0() << endl;
data <<"pbc_csr.port_7.sta_account.sp_held_3: 0x" << pbc_csr.port_7.sta_account.sp_held_3() << endl;
data <<"pbc_csr.port_7.sta_account.sp_held_2: 0x" << pbc_csr.port_7.sta_account.sp_held_2() << endl;
data <<"pbc_csr.port_7.sta_account.occupancy_28: 0x" << pbc_csr.port_7.sta_account.occupancy_28() << endl;
data <<"pbc_csr.port_7.sta_account.occupancy_29: 0x" << pbc_csr.port_7.sta_account.occupancy_29() << endl;
data <<"pbc_csr.port_7.sta_account.occupancy_21: 0x" << pbc_csr.port_7.sta_account.occupancy_21() << endl;
data <<"pbc_csr.port_7.sta_account.occupancy_20: 0x" << pbc_csr.port_7.sta_account.occupancy_20() << endl;
data <<"pbc_csr.port_7.sta_account.sp_held_9: 0x" << pbc_csr.port_7.sta_account.sp_held_9() << endl;
data <<"pbc_csr.port_7.sta_account.occupancy_22: 0x" << pbc_csr.port_7.sta_account.occupancy_22() << endl;
data <<"pbc_csr.port_7.sta_account.occupancy_23: 0x" << pbc_csr.port_7.sta_account.occupancy_23() << endl;
data <<"pbc_csr.port_7.sta_account.occupancy_24: 0x" << pbc_csr.port_7.sta_account.occupancy_24() << endl;
data <<"pbc_csr.port_7.sta_account.occupancy_25: 0x" << pbc_csr.port_7.sta_account.occupancy_25() << endl;
data <<"pbc_csr.port_7.sta_account.occupancy_26: 0x" << pbc_csr.port_7.sta_account.occupancy_26() << endl;
data <<"pbc_csr.port_7.sta_account.sp_held_8: 0x" << pbc_csr.port_7.sta_account.sp_held_8() << endl;
data <<"pbc_csr.port_7.sta_account.sp_held_28: 0x" << pbc_csr.port_7.sta_account.sp_held_28() << endl;
data <<"pbc_csr.port_7.sta_account.sp_held_29: 0x" << pbc_csr.port_7.sta_account.sp_held_29() << endl;
data <<"pbc_csr.port_7.sta_account.sp_held_20: 0x" << pbc_csr.port_7.sta_account.sp_held_20() << endl;
data <<"pbc_csr.port_7.sta_account.sp_held_21: 0x" << pbc_csr.port_7.sta_account.sp_held_21() << endl;
data <<"pbc_csr.port_7.sta_account.sp_held_22: 0x" << pbc_csr.port_7.sta_account.sp_held_22() << endl;
data <<"pbc_csr.port_7.sta_account.sp_held_23: 0x" << pbc_csr.port_7.sta_account.sp_held_23() << endl;
data <<"pbc_csr.port_7.sta_account.sp_held_24: 0x" << pbc_csr.port_7.sta_account.sp_held_24() << endl;
data <<"pbc_csr.port_7.sta_account.sp_held_25: 0x" << pbc_csr.port_7.sta_account.sp_held_25() << endl;
data <<"pbc_csr.port_7.sta_account.sp_held_26: 0x" << pbc_csr.port_7.sta_account.sp_held_26() << endl;
data <<"pbc_csr.port_7.sta_account.sp_held_27: 0x" << pbc_csr.port_7.sta_account.sp_held_27() << endl;
data <<"pbc_csr.port_7.sta_account.occupancy_27: 0x" << pbc_csr.port_7.sta_account.occupancy_27() << endl;
data <<"pbc_csr.port_7.sta_account.occupancy_9: 0x" << pbc_csr.port_7.sta_account.occupancy_9() << endl;
data <<"pbc_csr.port_7.sta_account.occupancy_8: 0x" << pbc_csr.port_7.sta_account.occupancy_8() << endl;
data <<"pbc_csr.port_7.sta_account.occupancy_11: 0x" << pbc_csr.port_7.sta_account.occupancy_11() << endl;
data <<"pbc_csr.port_7.sta_account.occupancy_10: 0x" << pbc_csr.port_7.sta_account.occupancy_10() << endl;
data <<"pbc_csr.port_7.sta_account.occupancy_13: 0x" << pbc_csr.port_7.sta_account.occupancy_13() << endl;
data <<"pbc_csr.port_7.sta_account.occupancy_12: 0x" << pbc_csr.port_7.sta_account.occupancy_12() << endl;
data <<"pbc_csr.port_7.sta_account.occupancy_15: 0x" << pbc_csr.port_7.sta_account.occupancy_15() << endl;
data <<"pbc_csr.port_7.sta_account.occupancy_14: 0x" << pbc_csr.port_7.sta_account.occupancy_14() << endl;
data <<"pbc_csr.port_7.sta_account.occupancy_17: 0x" << pbc_csr.port_7.sta_account.occupancy_17() << endl;
data <<"pbc_csr.port_7.sta_account.occupancy_16: 0x" << pbc_csr.port_7.sta_account.occupancy_16() << endl;
data <<"pbc_csr.port_7.sta_account.occupancy_19: 0x" << pbc_csr.port_7.sta_account.occupancy_19() << endl;
data <<"pbc_csr.port_7.sta_account.occupancy_18: 0x" << pbc_csr.port_7.sta_account.occupancy_18() << endl;
data <<"pbc_csr.port_7.sta_account.occupancy_31: 0x" << pbc_csr.port_7.sta_account.occupancy_31() << endl;
data <<"pbc_csr.port_7.sta_account.occupancy_30: 0x" << pbc_csr.port_7.sta_account.occupancy_30() << endl;
data <<"pbc_csr.port_7.sta_account.sp_held_31: 0x" << pbc_csr.port_7.sta_account.sp_held_31() << endl;
data <<"pbc_csr.port_7.sta_account.sp_held_30: 0x" << pbc_csr.port_7.sta_account.sp_held_30() << endl;
data <<"pbc_csr.port_7.sta_account.sp_held_15: 0x" << pbc_csr.port_7.sta_account.sp_held_15() << endl;
data <<"pbc_csr.port_7.sta_account.sp_held_14: 0x" << pbc_csr.port_7.sta_account.sp_held_14() << endl;
data <<"pbc_csr.port_7.sta_account.sp_held_17: 0x" << pbc_csr.port_7.sta_account.sp_held_17() << endl;
data <<"pbc_csr.port_7.sta_account.sp_held_16: 0x" << pbc_csr.port_7.sta_account.sp_held_16() << endl;
data <<"pbc_csr.port_7.sta_account.sp_held_11: 0x" << pbc_csr.port_7.sta_account.sp_held_11() << endl;
data <<"pbc_csr.port_7.sta_account.sp_held_10: 0x" << pbc_csr.port_7.sta_account.sp_held_10() << endl;
data <<"pbc_csr.port_7.sta_account.sp_held_13: 0x" << pbc_csr.port_7.sta_account.sp_held_13() << endl;
data <<"pbc_csr.port_7.sta_account.sp_held_12: 0x" << pbc_csr.port_7.sta_account.sp_held_12() << endl;
data <<"pbc_csr.port_7.sta_account.sp_held_19: 0x" << pbc_csr.port_7.sta_account.sp_held_19() << endl;
data <<"pbc_csr.port_7.sta_account.sp_held_18: 0x" << pbc_csr.port_7.sta_account.sp_held_18() << endl;

    pbc_csr.port_7.sta_account_pg_0_watermark.read();
    data <<"pbc_csr.port_7.sta_account_pg_0_watermark.all: 0x" << pbc_csr.port_7.sta_account_pg_0_watermark.all() << endl;
data <<"pbc_csr.port_7.sta_account_pg_0_watermark.shared_cells: 0x" << pbc_csr.port_7.sta_account_pg_0_watermark.shared_cells() << endl;
data <<"pbc_csr.port_7.sta_account_pg_0_watermark.total_cells: 0x" << pbc_csr.port_7.sta_account_pg_0_watermark.total_cells() << endl;

    pbc_csr.port_7.sta_account_pg_10_watermark.read();
    data <<"pbc_csr.port_7.sta_account_pg_10_watermark.all: 0x" << pbc_csr.port_7.sta_account_pg_10_watermark.all() << endl;
data <<"pbc_csr.port_7.sta_account_pg_10_watermark.shared_cells: 0x" << pbc_csr.port_7.sta_account_pg_10_watermark.shared_cells() << endl;
data <<"pbc_csr.port_7.sta_account_pg_10_watermark.total_cells: 0x" << pbc_csr.port_7.sta_account_pg_10_watermark.total_cells() << endl;

    pbc_csr.port_7.sta_account_pg_11_watermark.read();
    data <<"pbc_csr.port_7.sta_account_pg_11_watermark.all: 0x" << pbc_csr.port_7.sta_account_pg_11_watermark.all() << endl;
data <<"pbc_csr.port_7.sta_account_pg_11_watermark.shared_cells: 0x" << pbc_csr.port_7.sta_account_pg_11_watermark.shared_cells() << endl;
data <<"pbc_csr.port_7.sta_account_pg_11_watermark.total_cells: 0x" << pbc_csr.port_7.sta_account_pg_11_watermark.total_cells() << endl;

    pbc_csr.port_7.sta_account_pg_12_watermark.read();
    data <<"pbc_csr.port_7.sta_account_pg_12_watermark.all: 0x" << pbc_csr.port_7.sta_account_pg_12_watermark.all() << endl;
data <<"pbc_csr.port_7.sta_account_pg_12_watermark.shared_cells: 0x" << pbc_csr.port_7.sta_account_pg_12_watermark.shared_cells() << endl;
data <<"pbc_csr.port_7.sta_account_pg_12_watermark.total_cells: 0x" << pbc_csr.port_7.sta_account_pg_12_watermark.total_cells() << endl;

    pbc_csr.port_7.sta_account_pg_13_watermark.read();
    data <<"pbc_csr.port_7.sta_account_pg_13_watermark.all: 0x" << pbc_csr.port_7.sta_account_pg_13_watermark.all() << endl;
data <<"pbc_csr.port_7.sta_account_pg_13_watermark.shared_cells: 0x" << pbc_csr.port_7.sta_account_pg_13_watermark.shared_cells() << endl;
data <<"pbc_csr.port_7.sta_account_pg_13_watermark.total_cells: 0x" << pbc_csr.port_7.sta_account_pg_13_watermark.total_cells() << endl;

    pbc_csr.port_7.sta_account_pg_14_watermark.read();
    data <<"pbc_csr.port_7.sta_account_pg_14_watermark.all: 0x" << pbc_csr.port_7.sta_account_pg_14_watermark.all() << endl;
data <<"pbc_csr.port_7.sta_account_pg_14_watermark.shared_cells: 0x" << pbc_csr.port_7.sta_account_pg_14_watermark.shared_cells() << endl;
data <<"pbc_csr.port_7.sta_account_pg_14_watermark.total_cells: 0x" << pbc_csr.port_7.sta_account_pg_14_watermark.total_cells() << endl;

    pbc_csr.port_7.sta_account_pg_15_watermark.read();
    data <<"pbc_csr.port_7.sta_account_pg_15_watermark.all: 0x" << pbc_csr.port_7.sta_account_pg_15_watermark.all() << endl;
data <<"pbc_csr.port_7.sta_account_pg_15_watermark.shared_cells: 0x" << pbc_csr.port_7.sta_account_pg_15_watermark.shared_cells() << endl;
data <<"pbc_csr.port_7.sta_account_pg_15_watermark.total_cells: 0x" << pbc_csr.port_7.sta_account_pg_15_watermark.total_cells() << endl;

    pbc_csr.port_7.sta_account_pg_16_watermark.read();
    data <<"pbc_csr.port_7.sta_account_pg_16_watermark.all: 0x" << pbc_csr.port_7.sta_account_pg_16_watermark.all() << endl;
data <<"pbc_csr.port_7.sta_account_pg_16_watermark.shared_cells: 0x" << pbc_csr.port_7.sta_account_pg_16_watermark.shared_cells() << endl;
data <<"pbc_csr.port_7.sta_account_pg_16_watermark.total_cells: 0x" << pbc_csr.port_7.sta_account_pg_16_watermark.total_cells() << endl;

    pbc_csr.port_7.sta_account_pg_17_watermark.read();
    data <<"pbc_csr.port_7.sta_account_pg_17_watermark.all: 0x" << pbc_csr.port_7.sta_account_pg_17_watermark.all() << endl;
data <<"pbc_csr.port_7.sta_account_pg_17_watermark.shared_cells: 0x" << pbc_csr.port_7.sta_account_pg_17_watermark.shared_cells() << endl;
data <<"pbc_csr.port_7.sta_account_pg_17_watermark.total_cells: 0x" << pbc_csr.port_7.sta_account_pg_17_watermark.total_cells() << endl;

    pbc_csr.port_7.sta_account_pg_18_watermark.read();
    data <<"pbc_csr.port_7.sta_account_pg_18_watermark.all: 0x" << pbc_csr.port_7.sta_account_pg_18_watermark.all() << endl;
data <<"pbc_csr.port_7.sta_account_pg_18_watermark.shared_cells: 0x" << pbc_csr.port_7.sta_account_pg_18_watermark.shared_cells() << endl;
data <<"pbc_csr.port_7.sta_account_pg_18_watermark.total_cells: 0x" << pbc_csr.port_7.sta_account_pg_18_watermark.total_cells() << endl;

    pbc_csr.port_7.sta_account_pg_19_watermark.read();
    data <<"pbc_csr.port_7.sta_account_pg_19_watermark.all: 0x" << pbc_csr.port_7.sta_account_pg_19_watermark.all() << endl;
data <<"pbc_csr.port_7.sta_account_pg_19_watermark.shared_cells: 0x" << pbc_csr.port_7.sta_account_pg_19_watermark.shared_cells() << endl;
data <<"pbc_csr.port_7.sta_account_pg_19_watermark.total_cells: 0x" << pbc_csr.port_7.sta_account_pg_19_watermark.total_cells() << endl;

    pbc_csr.port_7.sta_account_pg_1_watermark.read();
    data <<"pbc_csr.port_7.sta_account_pg_1_watermark.all: 0x" << pbc_csr.port_7.sta_account_pg_1_watermark.all() << endl;
data <<"pbc_csr.port_7.sta_account_pg_1_watermark.shared_cells: 0x" << pbc_csr.port_7.sta_account_pg_1_watermark.shared_cells() << endl;
data <<"pbc_csr.port_7.sta_account_pg_1_watermark.total_cells: 0x" << pbc_csr.port_7.sta_account_pg_1_watermark.total_cells() << endl;

    pbc_csr.port_7.sta_account_pg_20_watermark.read();
    data <<"pbc_csr.port_7.sta_account_pg_20_watermark.all: 0x" << pbc_csr.port_7.sta_account_pg_20_watermark.all() << endl;
data <<"pbc_csr.port_7.sta_account_pg_20_watermark.shared_cells: 0x" << pbc_csr.port_7.sta_account_pg_20_watermark.shared_cells() << endl;
data <<"pbc_csr.port_7.sta_account_pg_20_watermark.total_cells: 0x" << pbc_csr.port_7.sta_account_pg_20_watermark.total_cells() << endl;

    pbc_csr.port_7.sta_account_pg_21_watermark.read();
    data <<"pbc_csr.port_7.sta_account_pg_21_watermark.all: 0x" << pbc_csr.port_7.sta_account_pg_21_watermark.all() << endl;
data <<"pbc_csr.port_7.sta_account_pg_21_watermark.shared_cells: 0x" << pbc_csr.port_7.sta_account_pg_21_watermark.shared_cells() << endl;
data <<"pbc_csr.port_7.sta_account_pg_21_watermark.total_cells: 0x" << pbc_csr.port_7.sta_account_pg_21_watermark.total_cells() << endl;

    pbc_csr.port_7.sta_account_pg_22_watermark.read();
    data <<"pbc_csr.port_7.sta_account_pg_22_watermark.all: 0x" << pbc_csr.port_7.sta_account_pg_22_watermark.all() << endl;
data <<"pbc_csr.port_7.sta_account_pg_22_watermark.shared_cells: 0x" << pbc_csr.port_7.sta_account_pg_22_watermark.shared_cells() << endl;
data <<"pbc_csr.port_7.sta_account_pg_22_watermark.total_cells: 0x" << pbc_csr.port_7.sta_account_pg_22_watermark.total_cells() << endl;

    pbc_csr.port_7.sta_account_pg_23_watermark.read();
    data <<"pbc_csr.port_7.sta_account_pg_23_watermark.all: 0x" << pbc_csr.port_7.sta_account_pg_23_watermark.all() << endl;
data <<"pbc_csr.port_7.sta_account_pg_23_watermark.shared_cells: 0x" << pbc_csr.port_7.sta_account_pg_23_watermark.shared_cells() << endl;
data <<"pbc_csr.port_7.sta_account_pg_23_watermark.total_cells: 0x" << pbc_csr.port_7.sta_account_pg_23_watermark.total_cells() << endl;

    pbc_csr.port_7.sta_account_pg_24_watermark.read();
    data <<"pbc_csr.port_7.sta_account_pg_24_watermark.all: 0x" << pbc_csr.port_7.sta_account_pg_24_watermark.all() << endl;
data <<"pbc_csr.port_7.sta_account_pg_24_watermark.shared_cells: 0x" << pbc_csr.port_7.sta_account_pg_24_watermark.shared_cells() << endl;
data <<"pbc_csr.port_7.sta_account_pg_24_watermark.total_cells: 0x" << pbc_csr.port_7.sta_account_pg_24_watermark.total_cells() << endl;

    pbc_csr.port_7.sta_account_pg_25_watermark.read();
    data <<"pbc_csr.port_7.sta_account_pg_25_watermark.all: 0x" << pbc_csr.port_7.sta_account_pg_25_watermark.all() << endl;
data <<"pbc_csr.port_7.sta_account_pg_25_watermark.shared_cells: 0x" << pbc_csr.port_7.sta_account_pg_25_watermark.shared_cells() << endl;
data <<"pbc_csr.port_7.sta_account_pg_25_watermark.total_cells: 0x" << pbc_csr.port_7.sta_account_pg_25_watermark.total_cells() << endl;

    pbc_csr.port_7.sta_account_pg_26_watermark.read();
    data <<"pbc_csr.port_7.sta_account_pg_26_watermark.all: 0x" << pbc_csr.port_7.sta_account_pg_26_watermark.all() << endl;
data <<"pbc_csr.port_7.sta_account_pg_26_watermark.shared_cells: 0x" << pbc_csr.port_7.sta_account_pg_26_watermark.shared_cells() << endl;
data <<"pbc_csr.port_7.sta_account_pg_26_watermark.total_cells: 0x" << pbc_csr.port_7.sta_account_pg_26_watermark.total_cells() << endl;

    pbc_csr.port_7.sta_account_pg_27_watermark.read();
    data <<"pbc_csr.port_7.sta_account_pg_27_watermark.all: 0x" << pbc_csr.port_7.sta_account_pg_27_watermark.all() << endl;
data <<"pbc_csr.port_7.sta_account_pg_27_watermark.shared_cells: 0x" << pbc_csr.port_7.sta_account_pg_27_watermark.shared_cells() << endl;
data <<"pbc_csr.port_7.sta_account_pg_27_watermark.total_cells: 0x" << pbc_csr.port_7.sta_account_pg_27_watermark.total_cells() << endl;

    pbc_csr.port_7.sta_account_pg_28_watermark.read();
    data <<"pbc_csr.port_7.sta_account_pg_28_watermark.all: 0x" << pbc_csr.port_7.sta_account_pg_28_watermark.all() << endl;
data <<"pbc_csr.port_7.sta_account_pg_28_watermark.shared_cells: 0x" << pbc_csr.port_7.sta_account_pg_28_watermark.shared_cells() << endl;
data <<"pbc_csr.port_7.sta_account_pg_28_watermark.total_cells: 0x" << pbc_csr.port_7.sta_account_pg_28_watermark.total_cells() << endl;

    pbc_csr.port_7.sta_account_pg_29_watermark.read();
    data <<"pbc_csr.port_7.sta_account_pg_29_watermark.all: 0x" << pbc_csr.port_7.sta_account_pg_29_watermark.all() << endl;
data <<"pbc_csr.port_7.sta_account_pg_29_watermark.shared_cells: 0x" << pbc_csr.port_7.sta_account_pg_29_watermark.shared_cells() << endl;
data <<"pbc_csr.port_7.sta_account_pg_29_watermark.total_cells: 0x" << pbc_csr.port_7.sta_account_pg_29_watermark.total_cells() << endl;

    pbc_csr.port_7.sta_account_pg_2_watermark.read();
    data <<"pbc_csr.port_7.sta_account_pg_2_watermark.all: 0x" << pbc_csr.port_7.sta_account_pg_2_watermark.all() << endl;
data <<"pbc_csr.port_7.sta_account_pg_2_watermark.shared_cells: 0x" << pbc_csr.port_7.sta_account_pg_2_watermark.shared_cells() << endl;
data <<"pbc_csr.port_7.sta_account_pg_2_watermark.total_cells: 0x" << pbc_csr.port_7.sta_account_pg_2_watermark.total_cells() << endl;

    pbc_csr.port_7.sta_account_pg_30_watermark.read();
    data <<"pbc_csr.port_7.sta_account_pg_30_watermark.all: 0x" << pbc_csr.port_7.sta_account_pg_30_watermark.all() << endl;
data <<"pbc_csr.port_7.sta_account_pg_30_watermark.shared_cells: 0x" << pbc_csr.port_7.sta_account_pg_30_watermark.shared_cells() << endl;
data <<"pbc_csr.port_7.sta_account_pg_30_watermark.total_cells: 0x" << pbc_csr.port_7.sta_account_pg_30_watermark.total_cells() << endl;

    pbc_csr.port_7.sta_account_pg_31_watermark.read();
    data <<"pbc_csr.port_7.sta_account_pg_31_watermark.all: 0x" << pbc_csr.port_7.sta_account_pg_31_watermark.all() << endl;
data <<"pbc_csr.port_7.sta_account_pg_31_watermark.shared_cells: 0x" << pbc_csr.port_7.sta_account_pg_31_watermark.shared_cells() << endl;
data <<"pbc_csr.port_7.sta_account_pg_31_watermark.total_cells: 0x" << pbc_csr.port_7.sta_account_pg_31_watermark.total_cells() << endl;

    pbc_csr.port_7.sta_account_pg_3_watermark.read();
    data <<"pbc_csr.port_7.sta_account_pg_3_watermark.all: 0x" << pbc_csr.port_7.sta_account_pg_3_watermark.all() << endl;
data <<"pbc_csr.port_7.sta_account_pg_3_watermark.shared_cells: 0x" << pbc_csr.port_7.sta_account_pg_3_watermark.shared_cells() << endl;
data <<"pbc_csr.port_7.sta_account_pg_3_watermark.total_cells: 0x" << pbc_csr.port_7.sta_account_pg_3_watermark.total_cells() << endl;

    pbc_csr.port_7.sta_account_pg_4_watermark.read();
    data <<"pbc_csr.port_7.sta_account_pg_4_watermark.all: 0x" << pbc_csr.port_7.sta_account_pg_4_watermark.all() << endl;
data <<"pbc_csr.port_7.sta_account_pg_4_watermark.shared_cells: 0x" << pbc_csr.port_7.sta_account_pg_4_watermark.shared_cells() << endl;
data <<"pbc_csr.port_7.sta_account_pg_4_watermark.total_cells: 0x" << pbc_csr.port_7.sta_account_pg_4_watermark.total_cells() << endl;

    pbc_csr.port_7.sta_account_pg_5_watermark.read();
    data <<"pbc_csr.port_7.sta_account_pg_5_watermark.all: 0x" << pbc_csr.port_7.sta_account_pg_5_watermark.all() << endl;
data <<"pbc_csr.port_7.sta_account_pg_5_watermark.shared_cells: 0x" << pbc_csr.port_7.sta_account_pg_5_watermark.shared_cells() << endl;
data <<"pbc_csr.port_7.sta_account_pg_5_watermark.total_cells: 0x" << pbc_csr.port_7.sta_account_pg_5_watermark.total_cells() << endl;

    pbc_csr.port_7.sta_account_pg_6_watermark.read();
    data <<"pbc_csr.port_7.sta_account_pg_6_watermark.all: 0x" << pbc_csr.port_7.sta_account_pg_6_watermark.all() << endl;
data <<"pbc_csr.port_7.sta_account_pg_6_watermark.shared_cells: 0x" << pbc_csr.port_7.sta_account_pg_6_watermark.shared_cells() << endl;
data <<"pbc_csr.port_7.sta_account_pg_6_watermark.total_cells: 0x" << pbc_csr.port_7.sta_account_pg_6_watermark.total_cells() << endl;

    pbc_csr.port_7.sta_account_pg_7_watermark.read();
    data <<"pbc_csr.port_7.sta_account_pg_7_watermark.all: 0x" << pbc_csr.port_7.sta_account_pg_7_watermark.all() << endl;
data <<"pbc_csr.port_7.sta_account_pg_7_watermark.shared_cells: 0x" << pbc_csr.port_7.sta_account_pg_7_watermark.shared_cells() << endl;
data <<"pbc_csr.port_7.sta_account_pg_7_watermark.total_cells: 0x" << pbc_csr.port_7.sta_account_pg_7_watermark.total_cells() << endl;

    pbc_csr.port_7.sta_account_pg_8_watermark.read();
    data <<"pbc_csr.port_7.sta_account_pg_8_watermark.all: 0x" << pbc_csr.port_7.sta_account_pg_8_watermark.all() << endl;
data <<"pbc_csr.port_7.sta_account_pg_8_watermark.shared_cells: 0x" << pbc_csr.port_7.sta_account_pg_8_watermark.shared_cells() << endl;
data <<"pbc_csr.port_7.sta_account_pg_8_watermark.total_cells: 0x" << pbc_csr.port_7.sta_account_pg_8_watermark.total_cells() << endl;

    pbc_csr.port_7.sta_account_pg_9_watermark.read();
    data <<"pbc_csr.port_7.sta_account_pg_9_watermark.all: 0x" << pbc_csr.port_7.sta_account_pg_9_watermark.all() << endl;
data <<"pbc_csr.port_7.sta_account_pg_9_watermark.shared_cells: 0x" << pbc_csr.port_7.sta_account_pg_9_watermark.shared_cells() << endl;
data <<"pbc_csr.port_7.sta_account_pg_9_watermark.total_cells: 0x" << pbc_csr.port_7.sta_account_pg_9_watermark.total_cells() << endl;

    pbc_csr.port_7.sta_write.read();
    data <<"pbc_csr.port_7.sta_write.all: 0x" << pbc_csr.port_7.sta_write.all() << endl;
data <<"pbc_csr.port_7.sta_write.cache_entries: 0x" << pbc_csr.port_7.sta_write.cache_entries() << endl;

    pbc_csr.sta_account_xoff_timeout.read();
    data <<"pbc_csr.sta_account_xoff_timeout.all: 0x" << pbc_csr.sta_account_xoff_timeout.all() << endl;
data <<"pbc_csr.sta_account_xoff_timeout.pg4: 0x" << pbc_csr.sta_account_xoff_timeout.pg4() << endl;
data <<"pbc_csr.sta_account_xoff_timeout.pg2: 0x" << pbc_csr.sta_account_xoff_timeout.pg2() << endl;
data <<"pbc_csr.sta_account_xoff_timeout.pg3: 0x" << pbc_csr.sta_account_xoff_timeout.pg3() << endl;
data <<"pbc_csr.sta_account_xoff_timeout.pg0: 0x" << pbc_csr.sta_account_xoff_timeout.pg0() << endl;
data <<"pbc_csr.sta_account_xoff_timeout.pg1: 0x" << pbc_csr.sta_account_xoff_timeout.pg1() << endl;

    pbc_csr.sta_axi.read();
    data <<"pbc_csr.sta_axi.all: 0x" << pbc_csr.sta_axi.all() << endl;
data <<"pbc_csr.sta_axi.ts_bresp: 0x" << pbc_csr.sta_axi.ts_bresp() << endl;
data <<"pbc_csr.sta_axi.ts_bid: 0x" << pbc_csr.sta_axi.ts_bid() << endl;

    pbc_csr.sta_bist.read();
    data <<"pbc_csr.sta_bist.all: 0x" << pbc_csr.sta_bist.all() << endl;
data <<"pbc_csr.sta_bist.desc_done_pass: 0x" << pbc_csr.sta_bist.desc_done_pass() << endl;
data <<"pbc_csr.sta_bist.linked_list_done_fail: 0x" << pbc_csr.sta_bist.linked_list_done_fail() << endl;
data <<"pbc_csr.sta_bist.free_cell_done_pass: 0x" << pbc_csr.sta_bist.free_cell_done_pass() << endl;
data <<"pbc_csr.sta_bist.num_cells_done_pass: 0x" << pbc_csr.sta_bist.num_cells_done_pass() << endl;
data <<"pbc_csr.sta_bist.oq_bx_done_pass: 0x" << pbc_csr.sta_bist.oq_bx_done_pass() << endl;
data <<"pbc_csr.sta_bist.refcount_done_pass: 0x" << pbc_csr.sta_bist.refcount_done_pass() << endl;
data <<"pbc_csr.sta_bist.unicast_mgr_done_fail: 0x" << pbc_csr.sta_bist.unicast_mgr_done_fail() << endl;
data <<"pbc_csr.sta_bist.oq_bx_done_fail: 0x" << pbc_csr.sta_bist.oq_bx_done_fail() << endl;
data <<"pbc_csr.sta_bist.oq_rwr_done_fail: 0x" << pbc_csr.sta_bist.oq_rwr_done_fail() << endl;
data <<"pbc_csr.sta_bist.desc_done_fail: 0x" << pbc_csr.sta_bist.desc_done_fail() << endl;
data <<"pbc_csr.sta_bist.oq_pck_rpl_done_pass: 0x" << pbc_csr.sta_bist.oq_pck_rpl_done_pass() << endl;
data <<"pbc_csr.sta_bist.oq_pck_done_fail: 0x" << pbc_csr.sta_bist.oq_pck_done_fail() << endl;
data <<"pbc_csr.sta_bist.num_cells_done_fail: 0x" << pbc_csr.sta_bist.num_cells_done_fail() << endl;
data <<"pbc_csr.sta_bist.free_cell_done_fail: 0x" << pbc_csr.sta_bist.free_cell_done_fail() << endl;
data <<"pbc_csr.sta_bist.oq_pck_done_pass: 0x" << pbc_csr.sta_bist.oq_pck_done_pass() << endl;
data <<"pbc_csr.sta_bist.refcount_done_fail: 0x" << pbc_csr.sta_bist.refcount_done_fail() << endl;
data <<"pbc_csr.sta_bist.oq_rwr_done_pass: 0x" << pbc_csr.sta_bist.oq_rwr_done_pass() << endl;
data <<"pbc_csr.sta_bist.oq_pck_rpl_done_fail: 0x" << pbc_csr.sta_bist.oq_pck_rpl_done_fail() << endl;
data <<"pbc_csr.sta_bist.unicast_mgr_done_pass: 0x" << pbc_csr.sta_bist.unicast_mgr_done_pass() << endl;
data <<"pbc_csr.sta_bist.linked_list_done_pass: 0x" << pbc_csr.sta_bist.linked_list_done_pass() << endl;

    pbc_csr.sta_ecc_bx_sideband.read();
    data <<"pbc_csr.sta_ecc_bx_sideband.all: 0x" << pbc_csr.sta_ecc_bx_sideband.all() << endl;
data <<"pbc_csr.sta_ecc_bx_sideband.addr: 0x" << pbc_csr.sta_ecc_bx_sideband.addr() << endl;
data <<"pbc_csr.sta_ecc_bx_sideband.uncorrectable: 0x" << pbc_csr.sta_ecc_bx_sideband.uncorrectable() << endl;
data <<"pbc_csr.sta_ecc_bx_sideband.correctable: 0x" << pbc_csr.sta_ecc_bx_sideband.correctable() << endl;
data <<"pbc_csr.sta_ecc_bx_sideband.syndrome: 0x" << pbc_csr.sta_ecc_bx_sideband.syndrome() << endl;

    pbc_csr.sta_ecc_fc_0.read();
    data <<"pbc_csr.sta_ecc_fc_0.all: 0x" << pbc_csr.sta_ecc_fc_0.all() << endl;
data <<"pbc_csr.sta_ecc_fc_0.addr: 0x" << pbc_csr.sta_ecc_fc_0.addr() << endl;
data <<"pbc_csr.sta_ecc_fc_0.uncorrectable: 0x" << pbc_csr.sta_ecc_fc_0.uncorrectable() << endl;
data <<"pbc_csr.sta_ecc_fc_0.correctable: 0x" << pbc_csr.sta_ecc_fc_0.correctable() << endl;
data <<"pbc_csr.sta_ecc_fc_0.syndrome: 0x" << pbc_csr.sta_ecc_fc_0.syndrome() << endl;

    pbc_csr.sta_ecc_fc_1.read();
    data <<"pbc_csr.sta_ecc_fc_1.all: 0x" << pbc_csr.sta_ecc_fc_1.all() << endl;
data <<"pbc_csr.sta_ecc_fc_1.addr: 0x" << pbc_csr.sta_ecc_fc_1.addr() << endl;
data <<"pbc_csr.sta_ecc_fc_1.uncorrectable: 0x" << pbc_csr.sta_ecc_fc_1.uncorrectable() << endl;
data <<"pbc_csr.sta_ecc_fc_1.correctable: 0x" << pbc_csr.sta_ecc_fc_1.correctable() << endl;
data <<"pbc_csr.sta_ecc_fc_1.syndrome: 0x" << pbc_csr.sta_ecc_fc_1.syndrome() << endl;

    pbc_csr.sta_ecc_nc.read();
    data <<"pbc_csr.sta_ecc_nc.all: 0x" << pbc_csr.sta_ecc_nc.all() << endl;
data <<"pbc_csr.sta_ecc_nc.addr: 0x" << pbc_csr.sta_ecc_nc.addr() << endl;
data <<"pbc_csr.sta_ecc_nc.uncorrectable: 0x" << pbc_csr.sta_ecc_nc.uncorrectable() << endl;
data <<"pbc_csr.sta_ecc_nc.correctable: 0x" << pbc_csr.sta_ecc_nc.correctable() << endl;
data <<"pbc_csr.sta_ecc_nc.syndrome: 0x" << pbc_csr.sta_ecc_nc.syndrome() << endl;

    pbc_csr.sta_ecc_pack.read();
    data <<"pbc_csr.sta_ecc_pack.all: 0x" << pbc_csr.sta_ecc_pack.all() << endl;
data <<"pbc_csr.sta_ecc_pack.addr: 0x" << pbc_csr.sta_ecc_pack.addr() << endl;
data <<"pbc_csr.sta_ecc_pack.uncorrectable: 0x" << pbc_csr.sta_ecc_pack.uncorrectable() << endl;
data <<"pbc_csr.sta_ecc_pack.correctable: 0x" << pbc_csr.sta_ecc_pack.correctable() << endl;
data <<"pbc_csr.sta_ecc_pack.syndrome: 0x" << pbc_csr.sta_ecc_pack.syndrome() << endl;

    pbc_csr.sta_ecc_pck_rpl.read();
    data <<"pbc_csr.sta_ecc_pck_rpl.all: 0x" << pbc_csr.sta_ecc_pck_rpl.all() << endl;
data <<"pbc_csr.sta_ecc_pck_rpl.addr: 0x" << pbc_csr.sta_ecc_pck_rpl.addr() << endl;
data <<"pbc_csr.sta_ecc_pck_rpl.uncorrectable: 0x" << pbc_csr.sta_ecc_pck_rpl.uncorrectable() << endl;
data <<"pbc_csr.sta_ecc_pck_rpl.correctable: 0x" << pbc_csr.sta_ecc_pck_rpl.correctable() << endl;
data <<"pbc_csr.sta_ecc_pck_rpl.syndrome: 0x" << pbc_csr.sta_ecc_pck_rpl.syndrome() << endl;

    pbc_csr.sta_ecc_rwr.read();
    data <<"pbc_csr.sta_ecc_rwr.all: 0x" << pbc_csr.sta_ecc_rwr.all() << endl;
data <<"pbc_csr.sta_ecc_rwr.addr: 0x" << pbc_csr.sta_ecc_rwr.addr() << endl;
data <<"pbc_csr.sta_ecc_rwr.uncorrectable: 0x" << pbc_csr.sta_ecc_rwr.uncorrectable() << endl;
data <<"pbc_csr.sta_ecc_rwr.correctable: 0x" << pbc_csr.sta_ecc_rwr.correctable() << endl;
data <<"pbc_csr.sta_ecc_rwr.syndrome: 0x" << pbc_csr.sta_ecc_rwr.syndrome() << endl;

    pbc_csr.sta_ecc_rwr_sideband.read();
    data <<"pbc_csr.sta_ecc_rwr_sideband.all: 0x" << pbc_csr.sta_ecc_rwr_sideband.all() << endl;
data <<"pbc_csr.sta_ecc_rwr_sideband.addr: 0x" << pbc_csr.sta_ecc_rwr_sideband.addr() << endl;
data <<"pbc_csr.sta_ecc_rwr_sideband.uncorrectable: 0x" << pbc_csr.sta_ecc_rwr_sideband.uncorrectable() << endl;
data <<"pbc_csr.sta_ecc_rwr_sideband.correctable: 0x" << pbc_csr.sta_ecc_rwr_sideband.correctable() << endl;
data <<"pbc_csr.sta_ecc_rwr_sideband.syndrome: 0x" << pbc_csr.sta_ecc_rwr_sideband.syndrome() << endl;

    pbc_csr.sta_ecc_rxd_sideband.read();
    data <<"pbc_csr.sta_ecc_rxd_sideband.all: 0x" << pbc_csr.sta_ecc_rxd_sideband.all() << endl;
data <<"pbc_csr.sta_ecc_rxd_sideband.addr: 0x" << pbc_csr.sta_ecc_rxd_sideband.addr() << endl;
data <<"pbc_csr.sta_ecc_rxd_sideband.uncorrectable: 0x" << pbc_csr.sta_ecc_rxd_sideband.uncorrectable() << endl;
data <<"pbc_csr.sta_ecc_rxd_sideband.correctable: 0x" << pbc_csr.sta_ecc_rxd_sideband.correctable() << endl;
data <<"pbc_csr.sta_ecc_rxd_sideband.syndrome: 0x" << pbc_csr.sta_ecc_rxd_sideband.syndrome() << endl;

    pbc_csr.sta_ecc_sideband.read();
    data <<"pbc_csr.sta_ecc_sideband.all: 0x" << pbc_csr.sta_ecc_sideband.all() << endl;
data <<"pbc_csr.sta_ecc_sideband.addr: 0x" << pbc_csr.sta_ecc_sideband.addr() << endl;
data <<"pbc_csr.sta_ecc_sideband.uncorrectable: 0x" << pbc_csr.sta_ecc_sideband.uncorrectable() << endl;
data <<"pbc_csr.sta_ecc_sideband.correctable: 0x" << pbc_csr.sta_ecc_sideband.correctable() << endl;
data <<"pbc_csr.sta_ecc_sideband.syndrome: 0x" << pbc_csr.sta_ecc_sideband.syndrome() << endl;

    pbc_csr.sta_ecc_uc.read();
    data <<"pbc_csr.sta_ecc_uc.all: 0x" << pbc_csr.sta_ecc_uc.all() << endl;
data <<"pbc_csr.sta_ecc_uc.addr: 0x" << pbc_csr.sta_ecc_uc.addr() << endl;
data <<"pbc_csr.sta_ecc_uc.uncorrectable: 0x" << pbc_csr.sta_ecc_uc.uncorrectable() << endl;
data <<"pbc_csr.sta_ecc_uc.correctable: 0x" << pbc_csr.sta_ecc_uc.correctable() << endl;
data <<"pbc_csr.sta_ecc_uc.syndrome: 0x" << pbc_csr.sta_ecc_uc.syndrome() << endl;

    pbc_csr.sta_fc_mgr_0.read();
    data <<"pbc_csr.sta_fc_mgr_0.all: 0x" << pbc_csr.sta_fc_mgr_0.all() << endl;
data <<"pbc_csr.sta_fc_mgr_0.cells_available: 0x" << pbc_csr.sta_fc_mgr_0.cells_available() << endl;
data <<"pbc_csr.sta_fc_mgr_0.init_done: 0x" << pbc_csr.sta_fc_mgr_0.init_done() << endl;

    pbc_csr.sta_fc_mgr_1.read();
    data <<"pbc_csr.sta_fc_mgr_1.all: 0x" << pbc_csr.sta_fc_mgr_1.all() << endl;
data <<"pbc_csr.sta_fc_mgr_1.cells_available: 0x" << pbc_csr.sta_fc_mgr_1.cells_available() << endl;
data <<"pbc_csr.sta_fc_mgr_1.init_done: 0x" << pbc_csr.sta_fc_mgr_1.init_done() << endl;

    pbc_csr.sta_oq[0].read();
    data <<"pbc_csr.sta_oq[0].all: 0x" << pbc_csr.sta_oq[0].all() << endl;
data <<"pbc_csr.sta_oq[0].depth_value: 0x" << pbc_csr.sta_oq[0].depth_value() << endl;

    pbc_csr.sta_oq[1].read();
    data <<"pbc_csr.sta_oq[1].all: 0x" << pbc_csr.sta_oq[1].all() << endl;
data <<"pbc_csr.sta_oq[1].depth_value: 0x" << pbc_csr.sta_oq[1].depth_value() << endl;

    pbc_csr.sta_oq[2].read();
    data <<"pbc_csr.sta_oq[2].all: 0x" << pbc_csr.sta_oq[2].all() << endl;
data <<"pbc_csr.sta_oq[2].depth_value: 0x" << pbc_csr.sta_oq[2].depth_value() << endl;

    pbc_csr.sta_oq[3].read();
    data <<"pbc_csr.sta_oq[3].all: 0x" << pbc_csr.sta_oq[3].all() << endl;
data <<"pbc_csr.sta_oq[3].depth_value: 0x" << pbc_csr.sta_oq[3].depth_value() << endl;

    pbc_csr.sta_oq[4].read();
    data <<"pbc_csr.sta_oq[4].all: 0x" << pbc_csr.sta_oq[4].all() << endl;
data <<"pbc_csr.sta_oq[4].depth_value: 0x" << pbc_csr.sta_oq[4].depth_value() << endl;

    pbc_csr.sta_oq[5].read();
    data <<"pbc_csr.sta_oq[5].all: 0x" << pbc_csr.sta_oq[5].all() << endl;
data <<"pbc_csr.sta_oq[5].depth_value: 0x" << pbc_csr.sta_oq[5].depth_value() << endl;

    pbc_csr.sta_oq[6].read();
    data <<"pbc_csr.sta_oq[6].all: 0x" << pbc_csr.sta_oq[6].all() << endl;
data <<"pbc_csr.sta_oq[6].depth_value: 0x" << pbc_csr.sta_oq[6].depth_value() << endl;

    pbc_csr.sta_oq[7].read();
    data <<"pbc_csr.sta_oq[7].all: 0x" << pbc_csr.sta_oq[7].all() << endl;
data <<"pbc_csr.sta_oq[7].depth_value: 0x" << pbc_csr.sta_oq[7].depth_value() << endl;

    pbc_csr.sta_oq_8.read();
    data <<"pbc_csr.sta_oq_8.all: 0x" << pbc_csr.sta_oq_8.all() << endl;
data <<"pbc_csr.sta_oq_8.depth_value: 0x" << pbc_csr.sta_oq_8.depth_value() << endl;

    pbc_csr.sta_oq_head[0].read();
    data <<"pbc_csr.sta_oq_head[0].all: 0x" << pbc_csr.sta_oq_head[0].all() << endl;
data <<"pbc_csr.sta_oq_head[0].ptr: 0x" << pbc_csr.sta_oq_head[0].ptr() << endl;

    pbc_csr.sta_oq_head[1].read();
    data <<"pbc_csr.sta_oq_head[1].all: 0x" << pbc_csr.sta_oq_head[1].all() << endl;
data <<"pbc_csr.sta_oq_head[1].ptr: 0x" << pbc_csr.sta_oq_head[1].ptr() << endl;

    pbc_csr.sta_oq_head[2].read();
    data <<"pbc_csr.sta_oq_head[2].all: 0x" << pbc_csr.sta_oq_head[2].all() << endl;
data <<"pbc_csr.sta_oq_head[2].ptr: 0x" << pbc_csr.sta_oq_head[2].ptr() << endl;

    pbc_csr.sta_oq_head[3].read();
    data <<"pbc_csr.sta_oq_head[3].all: 0x" << pbc_csr.sta_oq_head[3].all() << endl;
data <<"pbc_csr.sta_oq_head[3].ptr: 0x" << pbc_csr.sta_oq_head[3].ptr() << endl;

    pbc_csr.sta_oq_head[4].read();
    data <<"pbc_csr.sta_oq_head[4].all: 0x" << pbc_csr.sta_oq_head[4].all() << endl;
data <<"pbc_csr.sta_oq_head[4].ptr: 0x" << pbc_csr.sta_oq_head[4].ptr() << endl;

    pbc_csr.sta_oq_head[5].read();
    data <<"pbc_csr.sta_oq_head[5].all: 0x" << pbc_csr.sta_oq_head[5].all() << endl;
data <<"pbc_csr.sta_oq_head[5].ptr: 0x" << pbc_csr.sta_oq_head[5].ptr() << endl;

    pbc_csr.sta_oq_head[6].read();
    data <<"pbc_csr.sta_oq_head[6].all: 0x" << pbc_csr.sta_oq_head[6].all() << endl;
data <<"pbc_csr.sta_oq_head[6].ptr: 0x" << pbc_csr.sta_oq_head[6].ptr() << endl;

    pbc_csr.sta_oq_head[7].read();
    data <<"pbc_csr.sta_oq_head[7].all: 0x" << pbc_csr.sta_oq_head[7].all() << endl;
data <<"pbc_csr.sta_oq_head[7].ptr: 0x" << pbc_csr.sta_oq_head[7].ptr() << endl;

    pbc_csr.sta_oq_head_8.read();
    data <<"pbc_csr.sta_oq_head_8.all: 0x" << pbc_csr.sta_oq_head_8.all() << endl;
data <<"pbc_csr.sta_oq_head_8.ptr: 0x" << pbc_csr.sta_oq_head_8.ptr() << endl;

    pbc_csr.sta_pck_rpl_err.read();
    data <<"pbc_csr.sta_pck_rpl_err.all: 0x" << pbc_csr.sta_pck_rpl_err.all() << endl;
data <<"pbc_csr.sta_pck_rpl_err.replication_ptr: 0x" << pbc_csr.sta_pck_rpl_err.replication_ptr() << endl;
data <<"pbc_csr.sta_pck_rpl_err.rid: 0x" << pbc_csr.sta_pck_rpl_err.rid() << endl;
data <<"pbc_csr.sta_pck_rpl_err.rresp: 0x" << pbc_csr.sta_pck_rpl_err.rresp() << endl;

    pbc_csr.sta_port_mon_in.read();
    data <<"pbc_csr.sta_port_mon_in.all: 0x" << pbc_csr.sta_port_mon_in.all() << endl;
data <<"pbc_csr.sta_port_mon_in.ecc_correctable: 0x" << pbc_csr.sta_port_mon_in.ecc_correctable() << endl;
data <<"pbc_csr.sta_port_mon_in.init_done: 0x" << pbc_csr.sta_port_mon_in.init_done() << endl;
data <<"pbc_csr.sta_port_mon_in.bist_done_pass: 0x" << pbc_csr.sta_port_mon_in.bist_done_pass() << endl;
data <<"pbc_csr.sta_port_mon_in.bist_done_fail: 0x" << pbc_csr.sta_port_mon_in.bist_done_fail() << endl;
data <<"pbc_csr.sta_port_mon_in.ecc_uncorrectable: 0x" << pbc_csr.sta_port_mon_in.ecc_uncorrectable() << endl;
data <<"pbc_csr.sta_port_mon_in.ecc_syndrome: 0x" << pbc_csr.sta_port_mon_in.ecc_syndrome() << endl;
data <<"pbc_csr.sta_port_mon_in.ecc_addr: 0x" << pbc_csr.sta_port_mon_in.ecc_addr() << endl;

    pbc_csr.sta_port_mon_out.read();
    data <<"pbc_csr.sta_port_mon_out.all: 0x" << pbc_csr.sta_port_mon_out.all() << endl;
data <<"pbc_csr.sta_port_mon_out.ecc_correctable: 0x" << pbc_csr.sta_port_mon_out.ecc_correctable() << endl;
data <<"pbc_csr.sta_port_mon_out.init_done: 0x" << pbc_csr.sta_port_mon_out.init_done() << endl;
data <<"pbc_csr.sta_port_mon_out.bist_done_pass: 0x" << pbc_csr.sta_port_mon_out.bist_done_pass() << endl;
data <<"pbc_csr.sta_port_mon_out.bist_done_fail: 0x" << pbc_csr.sta_port_mon_out.bist_done_fail() << endl;
data <<"pbc_csr.sta_port_mon_out.ecc_uncorrectable: 0x" << pbc_csr.sta_port_mon_out.ecc_uncorrectable() << endl;
data <<"pbc_csr.sta_port_mon_out.ecc_syndrome: 0x" << pbc_csr.sta_port_mon_out.ecc_syndrome() << endl;
data <<"pbc_csr.sta_port_mon_out.ecc_addr: 0x" << pbc_csr.sta_port_mon_out.ecc_addr() << endl;

    pbc_csr.sta_rc.read();
    data <<"pbc_csr.sta_rc.all: 0x" << pbc_csr.sta_rc.all() << endl;
data <<"pbc_csr.sta_rc.init_done: 0x" << pbc_csr.sta_rc.init_done() << endl;

    pbc_csr.sta_rc_ecc.read();
    data <<"pbc_csr.sta_rc_ecc.all: 0x" << pbc_csr.sta_rc_ecc.all() << endl;
data <<"pbc_csr.sta_rc_ecc.addr: 0x" << pbc_csr.sta_rc_ecc.addr() << endl;
data <<"pbc_csr.sta_rc_ecc.uncorrectable: 0x" << pbc_csr.sta_rc_ecc.uncorrectable() << endl;
data <<"pbc_csr.sta_rc_ecc.correctable: 0x" << pbc_csr.sta_rc_ecc.correctable() << endl;
data <<"pbc_csr.sta_rc_ecc.syndrome: 0x" << pbc_csr.sta_rc_ecc.syndrome() << endl;

    pbc_csr.sta_rpl_err.read();
    data <<"pbc_csr.sta_rpl_err.all: 0x" << pbc_csr.sta_rpl_err.all() << endl;
data <<"pbc_csr.sta_rpl_err.replication_ptr: 0x" << pbc_csr.sta_rpl_err.replication_ptr() << endl;
data <<"pbc_csr.sta_rpl_err.rid: 0x" << pbc_csr.sta_rpl_err.rid() << endl;
data <<"pbc_csr.sta_rpl_err.rresp: 0x" << pbc_csr.sta_rpl_err.rresp() << endl;

    SDK_TRACE_DEBUG("%s", data.str().c_str());
}

void
elba_tm_dump_config_regs (void)
{
    elb_top_csr_t &elb0 = ELB_BLK_REG_MODEL_ACCESS(elb_top_csr_t, 0, 0);
    elb_pbc_csr_t &pbc_csr = elb0.pb.pbc;
    stringstream data;
    data << hex << endl;
    pbc_csr.cfg_axi.read();
    data <<"pbc_csr.cfg_axi.all: 0x" << pbc_csr.cfg_axi.all() << endl;
data <<"pbc_csr.cfg_axi.base_addr: 0x" << pbc_csr.cfg_axi.base_addr() << endl;

    pbc_csr.cfg_cache_axi.read();
    data <<"pbc_csr.cfg_cache_axi.all: 0x" << pbc_csr.cfg_cache_axi.all() << endl;
data <<"pbc_csr.cfg_cache_axi.read_attr_prot: 0x" << pbc_csr.cfg_cache_axi.read_attr_prot() << endl;
data <<"pbc_csr.cfg_cache_axi.read_attr_cache: 0x" << pbc_csr.cfg_cache_axi.read_attr_cache() << endl;
data <<"pbc_csr.cfg_cache_axi.read_attr_lock: 0x" << pbc_csr.cfg_cache_axi.read_attr_lock() << endl;
data <<"pbc_csr.cfg_cache_axi.read_attr_qos: 0x" << pbc_csr.cfg_cache_axi.read_attr_qos() << endl;

    pbc_csr.cfg_credits_hdr_insertion_growth_5.read();
    data <<"pbc_csr.cfg_credits_hdr_insertion_growth_5.all: 0x" << pbc_csr.cfg_credits_hdr_insertion_growth_5.all() << endl;
data <<"pbc_csr.cfg_credits_hdr_insertion_growth_5.cells: 0x" << pbc_csr.cfg_credits_hdr_insertion_growth_5.cells() << endl;

    pbc_csr.cfg_credits_hdr_insertion_growth_6.read();
    data <<"pbc_csr.cfg_credits_hdr_insertion_growth_6.all: 0x" << pbc_csr.cfg_credits_hdr_insertion_growth_6.all() << endl;
data <<"pbc_csr.cfg_credits_hdr_insertion_growth_6.cells: 0x" << pbc_csr.cfg_credits_hdr_insertion_growth_6.cells() << endl;

    pbc_csr.cfg_credits_hdr_insertion_growth_7.read();
    data <<"pbc_csr.cfg_credits_hdr_insertion_growth_7.all: 0x" << pbc_csr.cfg_credits_hdr_insertion_growth_7.all() << endl;
data <<"pbc_csr.cfg_credits_hdr_insertion_growth_7.cells: 0x" << pbc_csr.cfg_credits_hdr_insertion_growth_7.cells() << endl;

    pbc_csr.cfg_credits_max_growth_5.read();
    data <<"pbc_csr.cfg_credits_max_growth_5.all: 0x" << pbc_csr.cfg_credits_max_growth_5.all() << endl;
data <<"pbc_csr.cfg_credits_max_growth_5.cells: 0x" << pbc_csr.cfg_credits_max_growth_5.cells() << endl;

    pbc_csr.cfg_credits_max_growth_6.read();
    data <<"pbc_csr.cfg_credits_max_growth_6.all: 0x" << pbc_csr.cfg_credits_max_growth_6.all() << endl;
data <<"pbc_csr.cfg_credits_max_growth_6.cells: 0x" << pbc_csr.cfg_credits_max_growth_6.cells() << endl;

    pbc_csr.cfg_credits_max_growth_7.read();
    data <<"pbc_csr.cfg_credits_max_growth_7.all: 0x" << pbc_csr.cfg_credits_max_growth_7.all() << endl;
data <<"pbc_csr.cfg_credits_max_growth_7.cells: 0x" << pbc_csr.cfg_credits_max_growth_7.cells() << endl;

    pbc_csr.cfg_debug_port.read();
    data <<"pbc_csr.cfg_debug_port.all: 0x" << pbc_csr.cfg_debug_port.all() << endl;
data <<"pbc_csr.cfg_debug_port.param: 0x" << pbc_csr.cfg_debug_port.param() << endl;

    pbc_csr.cfg_dhs_mem.read();
    data <<"pbc_csr.cfg_dhs_mem.all: 0x" << pbc_csr.cfg_dhs_mem.all() << endl;
data <<"pbc_csr.cfg_dhs_mem.address: 0x" << pbc_csr.cfg_dhs_mem.address() << endl;

    pbc_csr.cfg_eg_ts_ctrl.read();
    data <<"pbc_csr.cfg_eg_ts_ctrl.all: 0x" << pbc_csr.cfg_eg_ts_ctrl.all() << endl;
data <<"pbc_csr.cfg_eg_ts_ctrl.enable: 0x" << pbc_csr.cfg_eg_ts_ctrl.enable() << endl;
data <<"pbc_csr.cfg_eg_ts_ctrl.base: 0x" << pbc_csr.cfg_eg_ts_ctrl.base() << endl;

    pbc_csr.cfg_enqueue_mux.read();
    data <<"pbc_csr.cfg_enqueue_mux.all: 0x" << pbc_csr.cfg_enqueue_mux.all() << endl;
data <<"pbc_csr.cfg_enqueue_mux.rate_limit_threshold_1: 0x" << pbc_csr.cfg_enqueue_mux.rate_limit_threshold_1() << endl;
data <<"pbc_csr.cfg_enqueue_mux.rate_limit_threshold_0: 0x" << pbc_csr.cfg_enqueue_mux.rate_limit_threshold_0() << endl;
data <<"pbc_csr.cfg_enqueue_mux.emergency_stop_threshold_1: 0x" << pbc_csr.cfg_enqueue_mux.emergency_stop_threshold_1() << endl;
data <<"pbc_csr.cfg_enqueue_mux.emergency_stop_threshold_0: 0x" << pbc_csr.cfg_enqueue_mux.emergency_stop_threshold_0() << endl;

    pbc_csr.cfg_fc_mgr_0.read();
    data <<"pbc_csr.cfg_fc_mgr_0.all: 0x" << pbc_csr.cfg_fc_mgr_0.all() << endl;
data <<"pbc_csr.cfg_fc_mgr_0.min_cell: 0x" << pbc_csr.cfg_fc_mgr_0.min_cell() << endl;
data <<"pbc_csr.cfg_fc_mgr_0.max_row: 0x" << pbc_csr.cfg_fc_mgr_0.max_row() << endl;
data <<"pbc_csr.cfg_fc_mgr_0.init_start: 0x" << pbc_csr.cfg_fc_mgr_0.init_start() << endl;
data <<"pbc_csr.cfg_fc_mgr_0.ecc_disable_cor: 0x" << pbc_csr.cfg_fc_mgr_0.ecc_disable_cor() << endl;
data <<"pbc_csr.cfg_fc_mgr_0.dhs_eccbypass: 0x" << pbc_csr.cfg_fc_mgr_0.dhs_eccbypass() << endl;
data <<"pbc_csr.cfg_fc_mgr_0.bist_run: 0x" << pbc_csr.cfg_fc_mgr_0.bist_run() << endl;
data <<"pbc_csr.cfg_fc_mgr_0.ecc_disable_det: 0x" << pbc_csr.cfg_fc_mgr_0.ecc_disable_det() << endl;
data <<"pbc_csr.cfg_fc_mgr_0.init_reset: 0x" << pbc_csr.cfg_fc_mgr_0.init_reset() << endl;

    pbc_csr.cfg_fc_mgr_1.read();
    data <<"pbc_csr.cfg_fc_mgr_1.all: 0x" << pbc_csr.cfg_fc_mgr_1.all() << endl;
data <<"pbc_csr.cfg_fc_mgr_1.min_cell: 0x" << pbc_csr.cfg_fc_mgr_1.min_cell() << endl;
data <<"pbc_csr.cfg_fc_mgr_1.max_row: 0x" << pbc_csr.cfg_fc_mgr_1.max_row() << endl;
data <<"pbc_csr.cfg_fc_mgr_1.init_start: 0x" << pbc_csr.cfg_fc_mgr_1.init_start() << endl;
data <<"pbc_csr.cfg_fc_mgr_1.ecc_disable_cor: 0x" << pbc_csr.cfg_fc_mgr_1.ecc_disable_cor() << endl;
data <<"pbc_csr.cfg_fc_mgr_1.dhs_eccbypass: 0x" << pbc_csr.cfg_fc_mgr_1.dhs_eccbypass() << endl;
data <<"pbc_csr.cfg_fc_mgr_1.bist_run: 0x" << pbc_csr.cfg_fc_mgr_1.bist_run() << endl;
data <<"pbc_csr.cfg_fc_mgr_1.ecc_disable_det: 0x" << pbc_csr.cfg_fc_mgr_1.ecc_disable_det() << endl;
data <<"pbc_csr.cfg_fc_mgr_1.init_reset: 0x" << pbc_csr.cfg_fc_mgr_1.init_reset() << endl;

    pbc_csr.cfg_island_control.read();
    data <<"pbc_csr.cfg_island_control.all: 0x" << pbc_csr.cfg_island_control.all() << endl;
data <<"pbc_csr.cfg_island_control.map: 0x" << pbc_csr.cfg_island_control.map() << endl;

    pbc_csr.cfg_nc.read();
    data <<"pbc_csr.cfg_nc.all: 0x" << pbc_csr.cfg_nc.all() << endl;
data <<"pbc_csr.cfg_nc.ecc_disable_cor: 0x" << pbc_csr.cfg_nc.ecc_disable_cor() << endl;
data <<"pbc_csr.cfg_nc.bist_run: 0x" << pbc_csr.cfg_nc.bist_run() << endl;
data <<"pbc_csr.cfg_nc.ecc_disable_det: 0x" << pbc_csr.cfg_nc.ecc_disable_det() << endl;
data <<"pbc_csr.cfg_nc.dhs_eccbypass: 0x" << pbc_csr.cfg_nc.dhs_eccbypass() << endl;

    pbc_csr.cfg_oq_7_pck_rpl_mem.read();
    data <<"pbc_csr.cfg_oq_7_pck_rpl_mem.all: 0x" << pbc_csr.cfg_oq_7_pck_rpl_mem.all() << endl;
data <<"pbc_csr.cfg_oq_7_pck_rpl_mem.ecc_disable_cor: 0x" << pbc_csr.cfg_oq_7_pck_rpl_mem.ecc_disable_cor() << endl;
data <<"pbc_csr.cfg_oq_7_pck_rpl_mem.bist_run: 0x" << pbc_csr.cfg_oq_7_pck_rpl_mem.bist_run() << endl;
data <<"pbc_csr.cfg_oq_7_pck_rpl_mem.ecc_disable_det: 0x" << pbc_csr.cfg_oq_7_pck_rpl_mem.ecc_disable_det() << endl;
data <<"pbc_csr.cfg_oq_7_pck_rpl_mem.dhs_eccbypass: 0x" << pbc_csr.cfg_oq_7_pck_rpl_mem.dhs_eccbypass() << endl;

    pbc_csr.cfg_parser0.read();
    data <<"pbc_csr.cfg_parser0.all: 0x" << pbc_csr.cfg_parser0.all() << endl;
data <<"pbc_csr.cfg_parser0.default_cos: 0x" << pbc_csr.cfg_parser0.default_cos() << endl;
data <<"pbc_csr.cfg_parser0.oq_map: 0x" << pbc_csr.cfg_parser0.oq_map() << endl;
data <<"pbc_csr.cfg_parser0.default_port: 0x" << pbc_csr.cfg_parser0.default_port() << endl;

    pbc_csr.cfg_parser1.read();
    data <<"pbc_csr.cfg_parser1.all: 0x" << pbc_csr.cfg_parser1.all() << endl;
data <<"pbc_csr.cfg_parser1.default_cos: 0x" << pbc_csr.cfg_parser1.default_cos() << endl;
data <<"pbc_csr.cfg_parser1.oq_map: 0x" << pbc_csr.cfg_parser1.oq_map() << endl;
data <<"pbc_csr.cfg_parser1.default_port: 0x" << pbc_csr.cfg_parser1.default_port() << endl;

    pbc_csr.cfg_parser2.read();
    data <<"pbc_csr.cfg_parser2.all: 0x" << pbc_csr.cfg_parser2.all() << endl;
data <<"pbc_csr.cfg_parser2.default_cos: 0x" << pbc_csr.cfg_parser2.default_cos() << endl;
data <<"pbc_csr.cfg_parser2.oq_map: 0x" << pbc_csr.cfg_parser2.oq_map() << endl;
data <<"pbc_csr.cfg_parser2.default_port: 0x" << pbc_csr.cfg_parser2.default_port() << endl;

    pbc_csr.cfg_parser3.read();
    data <<"pbc_csr.cfg_parser3.all: 0x" << pbc_csr.cfg_parser3.all() << endl;
data <<"pbc_csr.cfg_parser3.default_cos: 0x" << pbc_csr.cfg_parser3.default_cos() << endl;
data <<"pbc_csr.cfg_parser3.oq_map: 0x" << pbc_csr.cfg_parser3.oq_map() << endl;
data <<"pbc_csr.cfg_parser3.default_port: 0x" << pbc_csr.cfg_parser3.default_port() << endl;

    pbc_csr.cfg_parser4.read();
    data <<"pbc_csr.cfg_parser4.all: 0x" << pbc_csr.cfg_parser4.all() << endl;
data <<"pbc_csr.cfg_parser4.default_cos: 0x" << pbc_csr.cfg_parser4.default_cos() << endl;
data <<"pbc_csr.cfg_parser4.oq_map: 0x" << pbc_csr.cfg_parser4.oq_map() << endl;
data <<"pbc_csr.cfg_parser4.default_port: 0x" << pbc_csr.cfg_parser4.default_port() << endl;

    pbc_csr.cfg_pbc_control.read();
    data <<"pbc_csr.cfg_pbc_control.all: 0x" << pbc_csr.cfg_pbc_control.all() << endl;
data <<"pbc_csr.cfg_pbc_control.sw_reset: 0x" << pbc_csr.cfg_pbc_control.sw_reset() << endl;
data <<"pbc_csr.cfg_pbc_control.enqueue_mux_bypass: 0x" << pbc_csr.cfg_pbc_control.enqueue_mux_bypass() << endl;
data <<"pbc_csr.cfg_pbc_control.credit_reset: 0x" << pbc_csr.cfg_pbc_control.credit_reset() << endl;

    pbc_csr.cfg_port_mon_in.read();
    data <<"pbc_csr.cfg_port_mon_in.all: 0x" << pbc_csr.cfg_port_mon_in.all() << endl;
data <<"pbc_csr.cfg_port_mon_in.init_start: 0x" << pbc_csr.cfg_port_mon_in.init_start() << endl;
data <<"pbc_csr.cfg_port_mon_in.ecc_disable_cor: 0x" << pbc_csr.cfg_port_mon_in.ecc_disable_cor() << endl;
data <<"pbc_csr.cfg_port_mon_in.dhs_eccbypass: 0x" << pbc_csr.cfg_port_mon_in.dhs_eccbypass() << endl;
data <<"pbc_csr.cfg_port_mon_in.bist_run: 0x" << pbc_csr.cfg_port_mon_in.bist_run() << endl;
data <<"pbc_csr.cfg_port_mon_in.ecc_disable_det: 0x" << pbc_csr.cfg_port_mon_in.ecc_disable_det() << endl;
data <<"pbc_csr.cfg_port_mon_in.eth: 0x" << pbc_csr.cfg_port_mon_in.eth() << endl;
data <<"pbc_csr.cfg_port_mon_in.dhs_selection: 0x" << pbc_csr.cfg_port_mon_in.dhs_selection() << endl;

    pbc_csr.cfg_port_mon_out.read();
    data <<"pbc_csr.cfg_port_mon_out.all: 0x" << pbc_csr.cfg_port_mon_out.all() << endl;
data <<"pbc_csr.cfg_port_mon_out.init_start: 0x" << pbc_csr.cfg_port_mon_out.init_start() << endl;
data <<"pbc_csr.cfg_port_mon_out.port_2: 0x" << pbc_csr.cfg_port_mon_out.port_2() << endl;
data <<"pbc_csr.cfg_port_mon_out.port_3: 0x" << pbc_csr.cfg_port_mon_out.port_3() << endl;
data <<"pbc_csr.cfg_port_mon_out.port_0: 0x" << pbc_csr.cfg_port_mon_out.port_0() << endl;
data <<"pbc_csr.cfg_port_mon_out.port_1: 0x" << pbc_csr.cfg_port_mon_out.port_1() << endl;
data <<"pbc_csr.cfg_port_mon_out.dhs_eccbypass: 0x" << pbc_csr.cfg_port_mon_out.dhs_eccbypass() << endl;
data <<"pbc_csr.cfg_port_mon_out.bist_run: 0x" << pbc_csr.cfg_port_mon_out.bist_run() << endl;
data <<"pbc_csr.cfg_port_mon_out.ecc_disable_cor: 0x" << pbc_csr.cfg_port_mon_out.ecc_disable_cor() << endl;
data <<"pbc_csr.cfg_port_mon_out.ecc_disable_det: 0x" << pbc_csr.cfg_port_mon_out.ecc_disable_det() << endl;
data <<"pbc_csr.cfg_port_mon_out.dhs_selection: 0x" << pbc_csr.cfg_port_mon_out.dhs_selection() << endl;

    pbc_csr.cfg_prp_mux.read();
    data <<"pbc_csr.cfg_prp_mux.all: 0x" << pbc_csr.cfg_prp_mux.all() << endl;
data <<"pbc_csr.cfg_prp_mux.sel: 0x" << pbc_csr.cfg_prp_mux.sel() << endl;

    pbc_csr.cfg_rc.read();
    data <<"pbc_csr.cfg_rc.all: 0x" << pbc_csr.cfg_rc.all() << endl;
data <<"pbc_csr.cfg_rc.init_start: 0x" << pbc_csr.cfg_rc.init_start() << endl;
data <<"pbc_csr.cfg_rc.ecc_disable_cor: 0x" << pbc_csr.cfg_rc.ecc_disable_cor() << endl;
data <<"pbc_csr.cfg_rc.dhs_eccbypass: 0x" << pbc_csr.cfg_rc.dhs_eccbypass() << endl;
data <<"pbc_csr.cfg_rc.bist_run: 0x" << pbc_csr.cfg_rc.bist_run() << endl;
data <<"pbc_csr.cfg_rc.ecc_disable_det: 0x" << pbc_csr.cfg_rc.ecc_disable_det() << endl;
data <<"pbc_csr.cfg_rc.init_reset: 0x" << pbc_csr.cfg_rc.init_reset() << endl;

    pbc_csr.cfg_rpl.read();
    data <<"pbc_csr.cfg_rpl.all: 0x" << pbc_csr.cfg_rpl.all() << endl;
data <<"pbc_csr.cfg_rpl.base: 0x" << pbc_csr.cfg_rpl.base() << endl;
data <<"pbc_csr.cfg_rpl.token_size: 0x" << pbc_csr.cfg_rpl.token_size() << endl;

    pbc_csr.cfg_sp.read();
    data <<"pbc_csr.cfg_sp.all: 0x" << pbc_csr.cfg_sp.all() << endl;
data <<"pbc_csr.cfg_sp.buffer_3: 0x" << pbc_csr.cfg_sp.buffer_3() << endl;
data <<"pbc_csr.cfg_sp.buffer_2: 0x" << pbc_csr.cfg_sp.buffer_2() << endl;
data <<"pbc_csr.cfg_sp.buffer_1: 0x" << pbc_csr.cfg_sp.buffer_1() << endl;
data <<"pbc_csr.cfg_sp.buffer_0: 0x" << pbc_csr.cfg_sp.buffer_0() << endl;

    pbc_csr.cfg_spare.read();
    data <<"pbc_csr.cfg_spare.all: 0x" << pbc_csr.cfg_spare.all() << endl;
data <<"pbc_csr.cfg_spare.data: 0x" << pbc_csr.cfg_spare.data() << endl;

    pbc_csr.cfg_src_port_to_lif_map.read();
    data <<"pbc_csr.cfg_src_port_to_lif_map.all: 0x" << pbc_csr.cfg_src_port_to_lif_map.all() << endl;
data <<"pbc_csr.cfg_src_port_to_lif_map.entry_4: 0x" << pbc_csr.cfg_src_port_to_lif_map.entry_4() << endl;
data <<"pbc_csr.cfg_src_port_to_lif_map.entry_2: 0x" << pbc_csr.cfg_src_port_to_lif_map.entry_2() << endl;
data <<"pbc_csr.cfg_src_port_to_lif_map.entry_3: 0x" << pbc_csr.cfg_src_port_to_lif_map.entry_3() << endl;
data <<"pbc_csr.cfg_src_port_to_lif_map.entry_0: 0x" << pbc_csr.cfg_src_port_to_lif_map.entry_0() << endl;
data <<"pbc_csr.cfg_src_port_to_lif_map.entry_1: 0x" << pbc_csr.cfg_src_port_to_lif_map.entry_1() << endl;

    pbc_csr.cfg_ts_axi.read();
    data <<"pbc_csr.cfg_ts_axi.all: 0x" << pbc_csr.cfg_ts_axi.all() << endl;
data <<"pbc_csr.cfg_ts_axi.write_attr_cache: 0x" << pbc_csr.cfg_ts_axi.write_attr_cache() << endl;
data <<"pbc_csr.cfg_ts_axi.write_attr_prot: 0x" << pbc_csr.cfg_ts_axi.write_attr_prot() << endl;
data <<"pbc_csr.cfg_ts_axi.write_attr_qos: 0x" << pbc_csr.cfg_ts_axi.write_attr_qos() << endl;
data <<"pbc_csr.cfg_ts_axi.write_attr_lock: 0x" << pbc_csr.cfg_ts_axi.write_attr_lock() << endl;
data <<"pbc_csr.cfg_ts_axi.id: 0x" << pbc_csr.cfg_ts_axi.id() << endl;

    pbc_csr.cfg_uc.read();
    data <<"pbc_csr.cfg_uc.all: 0x" << pbc_csr.cfg_uc.all() << endl;
data <<"pbc_csr.cfg_uc.ecc_disable_cor: 0x" << pbc_csr.cfg_uc.ecc_disable_cor() << endl;
data <<"pbc_csr.cfg_uc.bist_run: 0x" << pbc_csr.cfg_uc.bist_run() << endl;
data <<"pbc_csr.cfg_uc.ecc_disable_det: 0x" << pbc_csr.cfg_uc.ecc_disable_det() << endl;
data <<"pbc_csr.cfg_uc.dhs_eccbypass: 0x" << pbc_csr.cfg_uc.dhs_eccbypass() << endl;

    pbc_csr.pbccache.cfg_pcache.read();
    data <<"pbc_csr.pbccache.cfg_pcache.all: 0x" << pbc_csr.pbccache.cfg_pcache.all() << endl;
data <<"pbc_csr.pbccache.cfg_pcache.bist_run: 0x" << pbc_csr.pbccache.cfg_pcache.bist_run() << endl;

    pbc_csr.pbccache.cfg_pcache_control.read();
    data <<"pbc_csr.pbccache.cfg_pcache_control.all: 0x" << pbc_csr.pbccache.cfg_pcache_control.all() << endl;
data <<"pbc_csr.pbccache.cfg_pcache_control.enable: 0x" << pbc_csr.pbccache.cfg_pcache_control.enable() << endl;

    pbc_csr.pbccache.cfg_pcache_dbg.read();
    data <<"pbc_csr.pbccache.cfg_pcache_dbg.all: 0x" << pbc_csr.pbccache.cfg_pcache_dbg.all() << endl;
data <<"pbc_csr.pbccache.cfg_pcache_dbg.addr: 0x" << pbc_csr.pbccache.cfg_pcache_dbg.addr() << endl;
data <<"pbc_csr.pbccache.cfg_pcache_dbg.eccbypass: 0x" << pbc_csr.pbccache.cfg_pcache_dbg.eccbypass() << endl;

    pbc_csr.pbccache.cfg_pcache_inval.read();
    data <<"pbc_csr.pbccache.cfg_pcache_inval.all: 0x" << pbc_csr.pbccache.cfg_pcache_inval.all() << endl;
data <<"pbc_csr.pbccache.cfg_pcache_inval.addr_valid: 0x" << pbc_csr.pbccache.cfg_pcache_inval.addr_valid() << endl;
data <<"pbc_csr.pbccache.cfg_pcache_inval.addr: 0x" << pbc_csr.pbccache.cfg_pcache_inval.addr() << endl;

    pbc_csr.pbcdesc_0.cfg_desc.read();
    data <<"pbc_csr.pbcdesc_0.cfg_desc.all: 0x" << pbc_csr.pbcdesc_0.cfg_desc.all() << endl;
data <<"pbc_csr.pbcdesc_0.cfg_desc.ecc_disable_cor: 0x" << pbc_csr.pbcdesc_0.cfg_desc.ecc_disable_cor() << endl;
data <<"pbc_csr.pbcdesc_0.cfg_desc.bist_run: 0x" << pbc_csr.pbcdesc_0.cfg_desc.bist_run() << endl;
data <<"pbc_csr.pbcdesc_0.cfg_desc.ecc_disable_det: 0x" << pbc_csr.pbcdesc_0.cfg_desc.ecc_disable_det() << endl;
data <<"pbc_csr.pbcdesc_0.cfg_desc.dhs_eccbypass: 0x" << pbc_csr.pbcdesc_0.cfg_desc.dhs_eccbypass() << endl;

    pbc_csr.pbcdesc_1.cfg_desc.read();
    data <<"pbc_csr.pbcdesc_1.cfg_desc.all: 0x" << pbc_csr.pbcdesc_1.cfg_desc.all() << endl;
data <<"pbc_csr.pbcdesc_1.cfg_desc.ecc_disable_cor: 0x" << pbc_csr.pbcdesc_1.cfg_desc.ecc_disable_cor() << endl;
data <<"pbc_csr.pbcdesc_1.cfg_desc.bist_run: 0x" << pbc_csr.pbcdesc_1.cfg_desc.bist_run() << endl;
data <<"pbc_csr.pbcdesc_1.cfg_desc.ecc_disable_det: 0x" << pbc_csr.pbcdesc_1.cfg_desc.ecc_disable_det() << endl;
data <<"pbc_csr.pbcdesc_1.cfg_desc.dhs_eccbypass: 0x" << pbc_csr.pbcdesc_1.cfg_desc.dhs_eccbypass() << endl;

    pbc_csr.pbcll_0.cfg_ll.read();
    data <<"pbc_csr.pbcll_0.cfg_ll.all: 0x" << pbc_csr.pbcll_0.cfg_ll.all() << endl;
data <<"pbc_csr.pbcll_0.cfg_ll.ecc_disable_cor: 0x" << pbc_csr.pbcll_0.cfg_ll.ecc_disable_cor() << endl;
data <<"pbc_csr.pbcll_0.cfg_ll.bist_run: 0x" << pbc_csr.pbcll_0.cfg_ll.bist_run() << endl;
data <<"pbc_csr.pbcll_0.cfg_ll.ecc_disable_det: 0x" << pbc_csr.pbcll_0.cfg_ll.ecc_disable_det() << endl;
data <<"pbc_csr.pbcll_0.cfg_ll.dhs_eccbypass: 0x" << pbc_csr.pbcll_0.cfg_ll.dhs_eccbypass() << endl;

    pbc_csr.pbcll_1.cfg_ll.read();
    data <<"pbc_csr.pbcll_1.cfg_ll.all: 0x" << pbc_csr.pbcll_1.cfg_ll.all() << endl;
data <<"pbc_csr.pbcll_1.cfg_ll.ecc_disable_cor: 0x" << pbc_csr.pbcll_1.cfg_ll.ecc_disable_cor() << endl;
data <<"pbc_csr.pbcll_1.cfg_ll.bist_run: 0x" << pbc_csr.pbcll_1.cfg_ll.bist_run() << endl;
data <<"pbc_csr.pbcll_1.cfg_ll.ecc_disable_det: 0x" << pbc_csr.pbcll_1.cfg_ll.ecc_disable_det() << endl;
data <<"pbc_csr.pbcll_1.cfg_ll.dhs_eccbypass: 0x" << pbc_csr.pbcll_1.cfg_ll.dhs_eccbypass() << endl;

    pbc_csr.pbcsched.cfg_dhs_mem.read();
    data <<"pbc_csr.pbcsched.cfg_dhs_mem.all: 0x" << pbc_csr.pbcsched.cfg_dhs_mem.all() << endl;
data <<"pbc_csr.pbcsched.cfg_dhs_mem.address: 0x" << pbc_csr.pbcsched.cfg_dhs_mem.address() << endl;

    pbc_csr.pbcsched.cfg_sched.read();
    data <<"pbc_csr.pbcsched.cfg_sched.all: 0x" << pbc_csr.pbcsched.cfg_sched.all() << endl;
data <<"pbc_csr.pbcsched.cfg_sched.ecc_disable_cor: 0x" << pbc_csr.pbcsched.cfg_sched.ecc_disable_cor() << endl;
data <<"pbc_csr.pbcsched.cfg_sched.dhs_eccbypass: 0x" << pbc_csr.pbcsched.cfg_sched.dhs_eccbypass() << endl;
data <<"pbc_csr.pbcsched.cfg_sched.enable_wrr: 0x" << pbc_csr.pbcsched.cfg_sched.enable_wrr() << endl;
data <<"pbc_csr.pbcsched.cfg_sched.timer: 0x" << pbc_csr.pbcsched.cfg_sched.timer() << endl;
data <<"pbc_csr.pbcsched.cfg_sched.bist_run: 0x" << pbc_csr.pbcsched.cfg_sched.bist_run() << endl;
data <<"pbc_csr.pbcsched.cfg_sched.ecc_disable_det: 0x" << pbc_csr.pbcsched.cfg_sched.ecc_disable_det() << endl;
data <<"pbc_csr.pbcsched.cfg_sched.dhs_selection: 0x" << pbc_csr.pbcsched.cfg_sched.dhs_selection() << endl;

    pbc_csr.port_0.cfg_account_clear_xoff.read();
    data <<"pbc_csr.port_0.cfg_account_clear_xoff.all: 0x" << pbc_csr.port_0.cfg_account_clear_xoff.all() << endl;
data <<"pbc_csr.port_0.cfg_account_clear_xoff.bitmap: 0x" << pbc_csr.port_0.cfg_account_clear_xoff.bitmap() << endl;

    pbc_csr.port_0.cfg_account_control.read();
    data <<"pbc_csr.port_0.cfg_account_control.all: 0x" << pbc_csr.port_0.cfg_account_control.all() << endl;
data <<"pbc_csr.port_0.cfg_account_control.sp_enable: 0x" << pbc_csr.port_0.cfg_account_control.sp_enable() << endl;
data <<"pbc_csr.port_0.cfg_account_control.clear_wm_index: 0x" << pbc_csr.port_0.cfg_account_control.clear_wm_index() << endl;
data <<"pbc_csr.port_0.cfg_account_control.clear_wm_enable: 0x" << pbc_csr.port_0.cfg_account_control.clear_wm_enable() << endl;
data <<"pbc_csr.port_0.cfg_account_control.use_sp_as_wm: 0x" << pbc_csr.port_0.cfg_account_control.use_sp_as_wm() << endl;

    pbc_csr.port_0.cfg_account_credit_return.read();
    data <<"pbc_csr.port_0.cfg_account_credit_return.all: 0x" << pbc_csr.port_0.cfg_account_credit_return.all() << endl;
data <<"pbc_csr.port_0.cfg_account_credit_return.enable: 0x" << pbc_csr.port_0.cfg_account_credit_return.enable() << endl;

    pbc_csr.port_0.cfg_account_mtu_table.read();
    data <<"pbc_csr.port_0.cfg_account_mtu_table.all: 0x" << pbc_csr.port_0.cfg_account_mtu_table.all() << endl;
data <<"pbc_csr.port_0.cfg_account_mtu_table.pg6: 0x" << pbc_csr.port_0.cfg_account_mtu_table.pg6() << endl;
data <<"pbc_csr.port_0.cfg_account_mtu_table.pg7: 0x" << pbc_csr.port_0.cfg_account_mtu_table.pg7() << endl;
data <<"pbc_csr.port_0.cfg_account_mtu_table.pg4: 0x" << pbc_csr.port_0.cfg_account_mtu_table.pg4() << endl;
data <<"pbc_csr.port_0.cfg_account_mtu_table.pg5: 0x" << pbc_csr.port_0.cfg_account_mtu_table.pg5() << endl;
data <<"pbc_csr.port_0.cfg_account_mtu_table.pg2: 0x" << pbc_csr.port_0.cfg_account_mtu_table.pg2() << endl;
data <<"pbc_csr.port_0.cfg_account_mtu_table.pg3: 0x" << pbc_csr.port_0.cfg_account_mtu_table.pg3() << endl;
data <<"pbc_csr.port_0.cfg_account_mtu_table.pg0: 0x" << pbc_csr.port_0.cfg_account_mtu_table.pg0() << endl;
data <<"pbc_csr.port_0.cfg_account_mtu_table.pg1: 0x" << pbc_csr.port_0.cfg_account_mtu_table.pg1() << endl;

    pbc_csr.port_0.cfg_account_pause_timer.read();
    data <<"pbc_csr.port_0.cfg_account_pause_timer.all: 0x" << pbc_csr.port_0.cfg_account_pause_timer.all() << endl;
data <<"pbc_csr.port_0.cfg_account_pause_timer.xoff_timeout_auto_clear: 0x" << pbc_csr.port_0.cfg_account_pause_timer.xoff_timeout_auto_clear() << endl;
data <<"pbc_csr.port_0.cfg_account_pause_timer.xoff_timeout_auto_xon: 0x" << pbc_csr.port_0.cfg_account_pause_timer.xoff_timeout_auto_xon() << endl;
data <<"pbc_csr.port_0.cfg_account_pause_timer.xoff_timeout: 0x" << pbc_csr.port_0.cfg_account_pause_timer.xoff_timeout() << endl;

    pbc_csr.port_0.cfg_account_pause_timer_enable.read();
    data <<"pbc_csr.port_0.cfg_account_pause_timer_enable.all: 0x" << pbc_csr.port_0.cfg_account_pause_timer_enable.all() << endl;
data <<"pbc_csr.port_0.cfg_account_pause_timer_enable.iq: 0x" << pbc_csr.port_0.cfg_account_pause_timer_enable.iq() << endl;

    pbc_csr.port_0.cfg_account_pg_0.read();
    data <<"pbc_csr.port_0.cfg_account_pg_0.all: 0x" << pbc_csr.port_0.cfg_account_pg_0.all() << endl;
data <<"pbc_csr.port_0.cfg_account_pg_0.alpha: 0x" << pbc_csr.port_0.cfg_account_pg_0.alpha() << endl;
data <<"pbc_csr.port_0.cfg_account_pg_0.xon_threshold: 0x" << pbc_csr.port_0.cfg_account_pg_0.xon_threshold() << endl;
data <<"pbc_csr.port_0.cfg_account_pg_0.headroom: 0x" << pbc_csr.port_0.cfg_account_pg_0.headroom() << endl;
data <<"pbc_csr.port_0.cfg_account_pg_0.reserved_min: 0x" << pbc_csr.port_0.cfg_account_pg_0.reserved_min() << endl;
data <<"pbc_csr.port_0.cfg_account_pg_0.low_limit: 0x" << pbc_csr.port_0.cfg_account_pg_0.low_limit() << endl;

    pbc_csr.port_0.cfg_account_pg_1.read();
    data <<"pbc_csr.port_0.cfg_account_pg_1.all: 0x" << pbc_csr.port_0.cfg_account_pg_1.all() << endl;
data <<"pbc_csr.port_0.cfg_account_pg_1.alpha: 0x" << pbc_csr.port_0.cfg_account_pg_1.alpha() << endl;
data <<"pbc_csr.port_0.cfg_account_pg_1.xon_threshold: 0x" << pbc_csr.port_0.cfg_account_pg_1.xon_threshold() << endl;
data <<"pbc_csr.port_0.cfg_account_pg_1.headroom: 0x" << pbc_csr.port_0.cfg_account_pg_1.headroom() << endl;
data <<"pbc_csr.port_0.cfg_account_pg_1.reserved_min: 0x" << pbc_csr.port_0.cfg_account_pg_1.reserved_min() << endl;
data <<"pbc_csr.port_0.cfg_account_pg_1.low_limit: 0x" << pbc_csr.port_0.cfg_account_pg_1.low_limit() << endl;

    pbc_csr.port_0.cfg_account_pg_2.read();
    data <<"pbc_csr.port_0.cfg_account_pg_2.all: 0x" << pbc_csr.port_0.cfg_account_pg_2.all() << endl;
data <<"pbc_csr.port_0.cfg_account_pg_2.alpha: 0x" << pbc_csr.port_0.cfg_account_pg_2.alpha() << endl;
data <<"pbc_csr.port_0.cfg_account_pg_2.xon_threshold: 0x" << pbc_csr.port_0.cfg_account_pg_2.xon_threshold() << endl;
data <<"pbc_csr.port_0.cfg_account_pg_2.headroom: 0x" << pbc_csr.port_0.cfg_account_pg_2.headroom() << endl;
data <<"pbc_csr.port_0.cfg_account_pg_2.reserved_min: 0x" << pbc_csr.port_0.cfg_account_pg_2.reserved_min() << endl;
data <<"pbc_csr.port_0.cfg_account_pg_2.low_limit: 0x" << pbc_csr.port_0.cfg_account_pg_2.low_limit() << endl;

    pbc_csr.port_0.cfg_account_pg_3.read();
    data <<"pbc_csr.port_0.cfg_account_pg_3.all: 0x" << pbc_csr.port_0.cfg_account_pg_3.all() << endl;
data <<"pbc_csr.port_0.cfg_account_pg_3.alpha: 0x" << pbc_csr.port_0.cfg_account_pg_3.alpha() << endl;
data <<"pbc_csr.port_0.cfg_account_pg_3.xon_threshold: 0x" << pbc_csr.port_0.cfg_account_pg_3.xon_threshold() << endl;
data <<"pbc_csr.port_0.cfg_account_pg_3.headroom: 0x" << pbc_csr.port_0.cfg_account_pg_3.headroom() << endl;
data <<"pbc_csr.port_0.cfg_account_pg_3.reserved_min: 0x" << pbc_csr.port_0.cfg_account_pg_3.reserved_min() << endl;
data <<"pbc_csr.port_0.cfg_account_pg_3.low_limit: 0x" << pbc_csr.port_0.cfg_account_pg_3.low_limit() << endl;

    pbc_csr.port_0.cfg_account_pg_4.read();
    data <<"pbc_csr.port_0.cfg_account_pg_4.all: 0x" << pbc_csr.port_0.cfg_account_pg_4.all() << endl;
data <<"pbc_csr.port_0.cfg_account_pg_4.alpha: 0x" << pbc_csr.port_0.cfg_account_pg_4.alpha() << endl;
data <<"pbc_csr.port_0.cfg_account_pg_4.xon_threshold: 0x" << pbc_csr.port_0.cfg_account_pg_4.xon_threshold() << endl;
data <<"pbc_csr.port_0.cfg_account_pg_4.headroom: 0x" << pbc_csr.port_0.cfg_account_pg_4.headroom() << endl;
data <<"pbc_csr.port_0.cfg_account_pg_4.reserved_min: 0x" << pbc_csr.port_0.cfg_account_pg_4.reserved_min() << endl;
data <<"pbc_csr.port_0.cfg_account_pg_4.low_limit: 0x" << pbc_csr.port_0.cfg_account_pg_4.low_limit() << endl;

    pbc_csr.port_0.cfg_account_pg_5.read();
    data <<"pbc_csr.port_0.cfg_account_pg_5.all: 0x" << pbc_csr.port_0.cfg_account_pg_5.all() << endl;
data <<"pbc_csr.port_0.cfg_account_pg_5.alpha: 0x" << pbc_csr.port_0.cfg_account_pg_5.alpha() << endl;
data <<"pbc_csr.port_0.cfg_account_pg_5.xon_threshold: 0x" << pbc_csr.port_0.cfg_account_pg_5.xon_threshold() << endl;
data <<"pbc_csr.port_0.cfg_account_pg_5.headroom: 0x" << pbc_csr.port_0.cfg_account_pg_5.headroom() << endl;
data <<"pbc_csr.port_0.cfg_account_pg_5.reserved_min: 0x" << pbc_csr.port_0.cfg_account_pg_5.reserved_min() << endl;
data <<"pbc_csr.port_0.cfg_account_pg_5.low_limit: 0x" << pbc_csr.port_0.cfg_account_pg_5.low_limit() << endl;

    pbc_csr.port_0.cfg_account_pg_6.read();
    data <<"pbc_csr.port_0.cfg_account_pg_6.all: 0x" << pbc_csr.port_0.cfg_account_pg_6.all() << endl;
data <<"pbc_csr.port_0.cfg_account_pg_6.alpha: 0x" << pbc_csr.port_0.cfg_account_pg_6.alpha() << endl;
data <<"pbc_csr.port_0.cfg_account_pg_6.xon_threshold: 0x" << pbc_csr.port_0.cfg_account_pg_6.xon_threshold() << endl;
data <<"pbc_csr.port_0.cfg_account_pg_6.headroom: 0x" << pbc_csr.port_0.cfg_account_pg_6.headroom() << endl;
data <<"pbc_csr.port_0.cfg_account_pg_6.reserved_min: 0x" << pbc_csr.port_0.cfg_account_pg_6.reserved_min() << endl;
data <<"pbc_csr.port_0.cfg_account_pg_6.low_limit: 0x" << pbc_csr.port_0.cfg_account_pg_6.low_limit() << endl;

    pbc_csr.port_0.cfg_account_pg_7.read();
    data <<"pbc_csr.port_0.cfg_account_pg_7.all: 0x" << pbc_csr.port_0.cfg_account_pg_7.all() << endl;
data <<"pbc_csr.port_0.cfg_account_pg_7.alpha: 0x" << pbc_csr.port_0.cfg_account_pg_7.alpha() << endl;
data <<"pbc_csr.port_0.cfg_account_pg_7.xon_threshold: 0x" << pbc_csr.port_0.cfg_account_pg_7.xon_threshold() << endl;
data <<"pbc_csr.port_0.cfg_account_pg_7.headroom: 0x" << pbc_csr.port_0.cfg_account_pg_7.headroom() << endl;
data <<"pbc_csr.port_0.cfg_account_pg_7.reserved_min: 0x" << pbc_csr.port_0.cfg_account_pg_7.reserved_min() << endl;
data <<"pbc_csr.port_0.cfg_account_pg_7.low_limit: 0x" << pbc_csr.port_0.cfg_account_pg_7.low_limit() << endl;

    pbc_csr.port_0.cfg_account_pg_to_sp.read();
    data <<"pbc_csr.port_0.cfg_account_pg_to_sp.all: 0x" << pbc_csr.port_0.cfg_account_pg_to_sp.all() << endl;
data <<"pbc_csr.port_0.cfg_account_pg_to_sp.map: 0x" << pbc_csr.port_0.cfg_account_pg_to_sp.map() << endl;

    pbc_csr.port_0.cfg_account_sp_fetch_quanta.read();
    data <<"pbc_csr.port_0.cfg_account_sp_fetch_quanta.all: 0x" << pbc_csr.port_0.cfg_account_sp_fetch_quanta.all() << endl;
data <<"pbc_csr.port_0.cfg_account_sp_fetch_quanta.pg6: 0x" << pbc_csr.port_0.cfg_account_sp_fetch_quanta.pg6() << endl;
data <<"pbc_csr.port_0.cfg_account_sp_fetch_quanta.pg7: 0x" << pbc_csr.port_0.cfg_account_sp_fetch_quanta.pg7() << endl;
data <<"pbc_csr.port_0.cfg_account_sp_fetch_quanta.pg4: 0x" << pbc_csr.port_0.cfg_account_sp_fetch_quanta.pg4() << endl;
data <<"pbc_csr.port_0.cfg_account_sp_fetch_quanta.pg5: 0x" << pbc_csr.port_0.cfg_account_sp_fetch_quanta.pg5() << endl;
data <<"pbc_csr.port_0.cfg_account_sp_fetch_quanta.pg2: 0x" << pbc_csr.port_0.cfg_account_sp_fetch_quanta.pg2() << endl;
data <<"pbc_csr.port_0.cfg_account_sp_fetch_quanta.pg3: 0x" << pbc_csr.port_0.cfg_account_sp_fetch_quanta.pg3() << endl;
data <<"pbc_csr.port_0.cfg_account_sp_fetch_quanta.pg0: 0x" << pbc_csr.port_0.cfg_account_sp_fetch_quanta.pg0() << endl;
data <<"pbc_csr.port_0.cfg_account_sp_fetch_quanta.pg1: 0x" << pbc_csr.port_0.cfg_account_sp_fetch_quanta.pg1() << endl;

    pbc_csr.port_0.cfg_account_sp_fetch_threshold.read();
    data <<"pbc_csr.port_0.cfg_account_sp_fetch_threshold.all: 0x" << pbc_csr.port_0.cfg_account_sp_fetch_threshold.all() << endl;
data <<"pbc_csr.port_0.cfg_account_sp_fetch_threshold.pg6: 0x" << pbc_csr.port_0.cfg_account_sp_fetch_threshold.pg6() << endl;
data <<"pbc_csr.port_0.cfg_account_sp_fetch_threshold.pg7: 0x" << pbc_csr.port_0.cfg_account_sp_fetch_threshold.pg7() << endl;
data <<"pbc_csr.port_0.cfg_account_sp_fetch_threshold.pg4: 0x" << pbc_csr.port_0.cfg_account_sp_fetch_threshold.pg4() << endl;
data <<"pbc_csr.port_0.cfg_account_sp_fetch_threshold.pg5: 0x" << pbc_csr.port_0.cfg_account_sp_fetch_threshold.pg5() << endl;
data <<"pbc_csr.port_0.cfg_account_sp_fetch_threshold.pg2: 0x" << pbc_csr.port_0.cfg_account_sp_fetch_threshold.pg2() << endl;
data <<"pbc_csr.port_0.cfg_account_sp_fetch_threshold.pg3: 0x" << pbc_csr.port_0.cfg_account_sp_fetch_threshold.pg3() << endl;
data <<"pbc_csr.port_0.cfg_account_sp_fetch_threshold.pg0: 0x" << pbc_csr.port_0.cfg_account_sp_fetch_threshold.pg0() << endl;
data <<"pbc_csr.port_0.cfg_account_sp_fetch_threshold.pg1: 0x" << pbc_csr.port_0.cfg_account_sp_fetch_threshold.pg1() << endl;

    pbc_csr.port_0.cfg_account_tc_to_pg.read();
    data <<"pbc_csr.port_0.cfg_account_tc_to_pg.all: 0x" << pbc_csr.port_0.cfg_account_tc_to_pg.all() << endl;
data <<"pbc_csr.port_0.cfg_account_tc_to_pg.table: 0x" << pbc_csr.port_0.cfg_account_tc_to_pg.table() << endl;

    pbc_csr.port_0.cfg_latency_cache.read();
    data <<"pbc_csr.port_0.cfg_latency_cache.all: 0x" << pbc_csr.port_0.cfg_latency_cache.all() << endl;
data <<"pbc_csr.port_0.cfg_latency_cache.enable: 0x" << pbc_csr.port_0.cfg_latency_cache.enable() << endl;

    pbc_csr.port_0.cfg_mac_xoff.read();
    data <<"pbc_csr.port_0.cfg_mac_xoff.all: 0x" << pbc_csr.port_0.cfg_mac_xoff.all() << endl;
data <<"pbc_csr.port_0.cfg_mac_xoff.enable: 0x" << pbc_csr.port_0.cfg_mac_xoff.enable() << endl;

    pbc_csr.port_0.cfg_oq.read();
    data <<"pbc_csr.port_0.cfg_oq.all: 0x" << pbc_csr.port_0.cfg_oq.all() << endl;
data <<"pbc_csr.port_0.cfg_oq.enable: 0x" << pbc_csr.port_0.cfg_oq.enable() << endl;
data <<"pbc_csr.port_0.cfg_oq.num_hdr_bytes: 0x" << pbc_csr.port_0.cfg_oq.num_hdr_bytes() << endl;
data <<"pbc_csr.port_0.cfg_oq.num_hdr_bytes_crypto: 0x" << pbc_csr.port_0.cfg_oq.num_hdr_bytes_crypto() << endl;
data <<"pbc_csr.port_0.cfg_oq.span_cpu_num_cells: 0x" << pbc_csr.port_0.cfg_oq.span_cpu_num_cells() << endl;
data <<"pbc_csr.port_0.cfg_oq.flush_hw_error: 0x" << pbc_csr.port_0.cfg_oq.flush_hw_error() << endl;
data <<"pbc_csr.port_0.cfg_oq.eg_ts_enable: 0x" << pbc_csr.port_0.cfg_oq.eg_ts_enable() << endl;
data <<"pbc_csr.port_0.cfg_oq.ipg_bytes: 0x" << pbc_csr.port_0.cfg_oq.ipg_bytes() << endl;
data <<"pbc_csr.port_0.cfg_oq.flush: 0x" << pbc_csr.port_0.cfg_oq.flush() << endl;
data <<"pbc_csr.port_0.cfg_oq.flow_control_enable_xoff: 0x" << pbc_csr.port_0.cfg_oq.flow_control_enable_xoff() << endl;
data <<"pbc_csr.port_0.cfg_oq.hw_error_to_pbus: 0x" << pbc_csr.port_0.cfg_oq.hw_error_to_pbus() << endl;
data <<"pbc_csr.port_0.cfg_oq.rewrite_enable: 0x" << pbc_csr.port_0.cfg_oq.rewrite_enable() << endl;

    pbc_csr.port_0.cfg_oq_arb_l1_selection.read();
    data <<"pbc_csr.port_0.cfg_oq_arb_l1_selection.all: 0x" << pbc_csr.port_0.cfg_oq_arb_l1_selection.all() << endl;
data <<"pbc_csr.port_0.cfg_oq_arb_l1_selection.node_13: 0x" << pbc_csr.port_0.cfg_oq_arb_l1_selection.node_13() << endl;
data <<"pbc_csr.port_0.cfg_oq_arb_l1_selection.node_14: 0x" << pbc_csr.port_0.cfg_oq_arb_l1_selection.node_14() << endl;
data <<"pbc_csr.port_0.cfg_oq_arb_l1_selection.node_9: 0x" << pbc_csr.port_0.cfg_oq_arb_l1_selection.node_9() << endl;
data <<"pbc_csr.port_0.cfg_oq_arb_l1_selection.node_8: 0x" << pbc_csr.port_0.cfg_oq_arb_l1_selection.node_8() << endl;
data <<"pbc_csr.port_0.cfg_oq_arb_l1_selection.node_7: 0x" << pbc_csr.port_0.cfg_oq_arb_l1_selection.node_7() << endl;
data <<"pbc_csr.port_0.cfg_oq_arb_l1_selection.node_6: 0x" << pbc_csr.port_0.cfg_oq_arb_l1_selection.node_6() << endl;
data <<"pbc_csr.port_0.cfg_oq_arb_l1_selection.node_5: 0x" << pbc_csr.port_0.cfg_oq_arb_l1_selection.node_5() << endl;
data <<"pbc_csr.port_0.cfg_oq_arb_l1_selection.node_4: 0x" << pbc_csr.port_0.cfg_oq_arb_l1_selection.node_4() << endl;
data <<"pbc_csr.port_0.cfg_oq_arb_l1_selection.node_3: 0x" << pbc_csr.port_0.cfg_oq_arb_l1_selection.node_3() << endl;
data <<"pbc_csr.port_0.cfg_oq_arb_l1_selection.node_2: 0x" << pbc_csr.port_0.cfg_oq_arb_l1_selection.node_2() << endl;
data <<"pbc_csr.port_0.cfg_oq_arb_l1_selection.node_1: 0x" << pbc_csr.port_0.cfg_oq_arb_l1_selection.node_1() << endl;
data <<"pbc_csr.port_0.cfg_oq_arb_l1_selection.node_0: 0x" << pbc_csr.port_0.cfg_oq_arb_l1_selection.node_0() << endl;
data <<"pbc_csr.port_0.cfg_oq_arb_l1_selection.node_11: 0x" << pbc_csr.port_0.cfg_oq_arb_l1_selection.node_11() << endl;
data <<"pbc_csr.port_0.cfg_oq_arb_l1_selection.node_10: 0x" << pbc_csr.port_0.cfg_oq_arb_l1_selection.node_10() << endl;
data <<"pbc_csr.port_0.cfg_oq_arb_l1_selection.node_15: 0x" << pbc_csr.port_0.cfg_oq_arb_l1_selection.node_15() << endl;
data <<"pbc_csr.port_0.cfg_oq_arb_l1_selection.node_12: 0x" << pbc_csr.port_0.cfg_oq_arb_l1_selection.node_12() << endl;

    pbc_csr.port_0.cfg_oq_arb_l1_strict.read();
    data <<"pbc_csr.port_0.cfg_oq_arb_l1_strict.all: 0x" << pbc_csr.port_0.cfg_oq_arb_l1_strict.all() << endl;
data <<"pbc_csr.port_0.cfg_oq_arb_l1_strict.priority: 0x" << pbc_csr.port_0.cfg_oq_arb_l1_strict.priority() << endl;

    pbc_csr.port_0.cfg_oq_arb_l2_selection.read();
    data <<"pbc_csr.port_0.cfg_oq_arb_l2_selection.all: 0x" << pbc_csr.port_0.cfg_oq_arb_l2_selection.all() << endl;
data <<"pbc_csr.port_0.cfg_oq_arb_l2_selection.node_3: 0x" << pbc_csr.port_0.cfg_oq_arb_l2_selection.node_3() << endl;
data <<"pbc_csr.port_0.cfg_oq_arb_l2_selection.node_2: 0x" << pbc_csr.port_0.cfg_oq_arb_l2_selection.node_2() << endl;
data <<"pbc_csr.port_0.cfg_oq_arb_l2_selection.node_1: 0x" << pbc_csr.port_0.cfg_oq_arb_l2_selection.node_1() << endl;
data <<"pbc_csr.port_0.cfg_oq_arb_l2_selection.node_0: 0x" << pbc_csr.port_0.cfg_oq_arb_l2_selection.node_0() << endl;

    pbc_csr.port_0.cfg_oq_arb_l2_strict.read();
    data <<"pbc_csr.port_0.cfg_oq_arb_l2_strict.all: 0x" << pbc_csr.port_0.cfg_oq_arb_l2_strict.all() << endl;
data <<"pbc_csr.port_0.cfg_oq_arb_l2_strict.priority: 0x" << pbc_csr.port_0.cfg_oq_arb_l2_strict.priority() << endl;

    pbc_csr.port_0.cfg_oq_dhs_mem.read();
    data <<"pbc_csr.port_0.cfg_oq_dhs_mem.all: 0x" << pbc_csr.port_0.cfg_oq_dhs_mem.all() << endl;
data <<"pbc_csr.port_0.cfg_oq_dhs_mem.address: 0x" << pbc_csr.port_0.cfg_oq_dhs_mem.address() << endl;

    pbc_csr.port_0.cfg_oq_queue.read();
    data <<"pbc_csr.port_0.cfg_oq_queue.all: 0x" << pbc_csr.port_0.cfg_oq_queue.all() << endl;
data <<"pbc_csr.port_0.cfg_oq_queue.enable: 0x" << pbc_csr.port_0.cfg_oq_queue.enable() << endl;
data <<"pbc_csr.port_0.cfg_oq_queue.flush: 0x" << pbc_csr.port_0.cfg_oq_queue.flush() << endl;

    pbc_csr.port_0.cfg_oq_queue_dynamic.read();
    data <<"pbc_csr.port_0.cfg_oq_queue_dynamic.all: 0x" << pbc_csr.port_0.cfg_oq_queue_dynamic.all() << endl;
data <<"pbc_csr.port_0.cfg_oq_queue_dynamic.threshold: 0x" << pbc_csr.port_0.cfg_oq_queue_dynamic.threshold() << endl;

    pbc_csr.port_0.cfg_oq_queue_write_one.read();
    data <<"pbc_csr.port_0.cfg_oq_queue_write_one.all: 0x" << pbc_csr.port_0.cfg_oq_queue_write_one.all() << endl;
data <<"pbc_csr.port_0.cfg_oq_queue_write_one.enable: 0x" << pbc_csr.port_0.cfg_oq_queue_write_one.enable() << endl;
data <<"pbc_csr.port_0.cfg_oq_queue_write_one.disable: 0x" << pbc_csr.port_0.cfg_oq_queue_write_one.disable() << endl;

    pbc_csr.port_0.cfg_oq_xoff2oq.read();
    data <<"pbc_csr.port_0.cfg_oq_xoff2oq.all: 0x" << pbc_csr.port_0.cfg_oq_xoff2oq.all() << endl;
data <<"pbc_csr.port_0.cfg_oq_xoff2oq.map: 0x" << pbc_csr.port_0.cfg_oq_xoff2oq.map() << endl;

    pbc_csr.port_0.cfg_tail_drop.read();
    data <<"pbc_csr.port_0.cfg_tail_drop.all: 0x" << pbc_csr.port_0.cfg_tail_drop.all() << endl;
data <<"pbc_csr.port_0.cfg_tail_drop.cpu_threshold: 0x" << pbc_csr.port_0.cfg_tail_drop.cpu_threshold() << endl;
data <<"pbc_csr.port_0.cfg_tail_drop.span_threshold: 0x" << pbc_csr.port_0.cfg_tail_drop.span_threshold() << endl;

    pbc_csr.port_0.cfg_write_control.read();
    data <<"pbc_csr.port_0.cfg_write_control.all: 0x" << pbc_csr.port_0.cfg_write_control.all() << endl;
data <<"pbc_csr.port_0.cfg_write_control.release_cells: 0x" << pbc_csr.port_0.cfg_write_control.release_cells() << endl;
data <<"pbc_csr.port_0.cfg_write_control.enable: 0x" << pbc_csr.port_0.cfg_write_control.enable() << endl;
data <<"pbc_csr.port_0.cfg_write_control.drop_on_error: 0x" << pbc_csr.port_0.cfg_write_control.drop_on_error() << endl;
data <<"pbc_csr.port_0.cfg_write_control.rewrite: 0x" << pbc_csr.port_0.cfg_write_control.rewrite() << endl;
data <<"pbc_csr.port_0.cfg_write_control.span_pg_enable: 0x" << pbc_csr.port_0.cfg_write_control.span_pg_enable() << endl;
data <<"pbc_csr.port_0.cfg_write_control.span_pg_index: 0x" << pbc_csr.port_0.cfg_write_control.span_pg_index() << endl;
data <<"pbc_csr.port_0.cfg_write_control.enforce_oq24_31: 0x" << pbc_csr.port_0.cfg_write_control.enforce_oq24_31() << endl;
data <<"pbc_csr.port_0.cfg_write_control.packing_timeout_cycles: 0x" << pbc_csr.port_0.cfg_write_control.packing_timeout_cycles() << endl;
data <<"pbc_csr.port_0.cfg_write_control.min_size: 0x" << pbc_csr.port_0.cfg_write_control.min_size() << endl;
data <<"pbc_csr.port_0.cfg_write_control.packing_same_pg: 0x" << pbc_csr.port_0.cfg_write_control.packing_same_pg() << endl;
data <<"pbc_csr.port_0.cfg_write_control.packing: 0x" << pbc_csr.port_0.cfg_write_control.packing() << endl;
data <<"pbc_csr.port_0.cfg_write_control.enqueue_mux_bypass: 0x" << pbc_csr.port_0.cfg_write_control.enqueue_mux_bypass() << endl;
data <<"pbc_csr.port_0.cfg_write_control.enable_drdy: 0x" << pbc_csr.port_0.cfg_write_control.enable_drdy() << endl;
data <<"pbc_csr.port_0.cfg_write_control.rate_limiter: 0x" << pbc_csr.port_0.cfg_write_control.rate_limiter() << endl;
data <<"pbc_csr.port_0.cfg_write_control.cut_thru: 0x" << pbc_csr.port_0.cfg_write_control.cut_thru() << endl;
data <<"pbc_csr.port_0.cfg_write_control.enable_mnic_multicast: 0x" << pbc_csr.port_0.cfg_write_control.enable_mnic_multicast() << endl;
data <<"pbc_csr.port_0.cfg_write_control.enable_bmc_multicast: 0x" << pbc_csr.port_0.cfg_write_control.enable_bmc_multicast() << endl;

    pbc_csr.port_1.cfg_account_clear_xoff.read();
    data <<"pbc_csr.port_1.cfg_account_clear_xoff.all: 0x" << pbc_csr.port_1.cfg_account_clear_xoff.all() << endl;
data <<"pbc_csr.port_1.cfg_account_clear_xoff.bitmap: 0x" << pbc_csr.port_1.cfg_account_clear_xoff.bitmap() << endl;

    pbc_csr.port_1.cfg_account_control.read();
    data <<"pbc_csr.port_1.cfg_account_control.all: 0x" << pbc_csr.port_1.cfg_account_control.all() << endl;
data <<"pbc_csr.port_1.cfg_account_control.sp_enable: 0x" << pbc_csr.port_1.cfg_account_control.sp_enable() << endl;
data <<"pbc_csr.port_1.cfg_account_control.clear_wm_index: 0x" << pbc_csr.port_1.cfg_account_control.clear_wm_index() << endl;
data <<"pbc_csr.port_1.cfg_account_control.clear_wm_enable: 0x" << pbc_csr.port_1.cfg_account_control.clear_wm_enable() << endl;
data <<"pbc_csr.port_1.cfg_account_control.use_sp_as_wm: 0x" << pbc_csr.port_1.cfg_account_control.use_sp_as_wm() << endl;

    pbc_csr.port_1.cfg_account_credit_return.read();
    data <<"pbc_csr.port_1.cfg_account_credit_return.all: 0x" << pbc_csr.port_1.cfg_account_credit_return.all() << endl;
data <<"pbc_csr.port_1.cfg_account_credit_return.enable: 0x" << pbc_csr.port_1.cfg_account_credit_return.enable() << endl;

    pbc_csr.port_1.cfg_account_mtu_table.read();
    data <<"pbc_csr.port_1.cfg_account_mtu_table.all: 0x" << pbc_csr.port_1.cfg_account_mtu_table.all() << endl;
data <<"pbc_csr.port_1.cfg_account_mtu_table.pg6: 0x" << pbc_csr.port_1.cfg_account_mtu_table.pg6() << endl;
data <<"pbc_csr.port_1.cfg_account_mtu_table.pg7: 0x" << pbc_csr.port_1.cfg_account_mtu_table.pg7() << endl;
data <<"pbc_csr.port_1.cfg_account_mtu_table.pg4: 0x" << pbc_csr.port_1.cfg_account_mtu_table.pg4() << endl;
data <<"pbc_csr.port_1.cfg_account_mtu_table.pg5: 0x" << pbc_csr.port_1.cfg_account_mtu_table.pg5() << endl;
data <<"pbc_csr.port_1.cfg_account_mtu_table.pg2: 0x" << pbc_csr.port_1.cfg_account_mtu_table.pg2() << endl;
data <<"pbc_csr.port_1.cfg_account_mtu_table.pg3: 0x" << pbc_csr.port_1.cfg_account_mtu_table.pg3() << endl;
data <<"pbc_csr.port_1.cfg_account_mtu_table.pg0: 0x" << pbc_csr.port_1.cfg_account_mtu_table.pg0() << endl;
data <<"pbc_csr.port_1.cfg_account_mtu_table.pg1: 0x" << pbc_csr.port_1.cfg_account_mtu_table.pg1() << endl;

    pbc_csr.port_1.cfg_account_pause_timer.read();
    data <<"pbc_csr.port_1.cfg_account_pause_timer.all: 0x" << pbc_csr.port_1.cfg_account_pause_timer.all() << endl;
data <<"pbc_csr.port_1.cfg_account_pause_timer.xoff_timeout_auto_clear: 0x" << pbc_csr.port_1.cfg_account_pause_timer.xoff_timeout_auto_clear() << endl;
data <<"pbc_csr.port_1.cfg_account_pause_timer.xoff_timeout_auto_xon: 0x" << pbc_csr.port_1.cfg_account_pause_timer.xoff_timeout_auto_xon() << endl;
data <<"pbc_csr.port_1.cfg_account_pause_timer.xoff_timeout: 0x" << pbc_csr.port_1.cfg_account_pause_timer.xoff_timeout() << endl;

    pbc_csr.port_1.cfg_account_pause_timer_enable.read();
    data <<"pbc_csr.port_1.cfg_account_pause_timer_enable.all: 0x" << pbc_csr.port_1.cfg_account_pause_timer_enable.all() << endl;
data <<"pbc_csr.port_1.cfg_account_pause_timer_enable.iq: 0x" << pbc_csr.port_1.cfg_account_pause_timer_enable.iq() << endl;

    pbc_csr.port_1.cfg_account_pg_0.read();
    data <<"pbc_csr.port_1.cfg_account_pg_0.all: 0x" << pbc_csr.port_1.cfg_account_pg_0.all() << endl;
data <<"pbc_csr.port_1.cfg_account_pg_0.alpha: 0x" << pbc_csr.port_1.cfg_account_pg_0.alpha() << endl;
data <<"pbc_csr.port_1.cfg_account_pg_0.xon_threshold: 0x" << pbc_csr.port_1.cfg_account_pg_0.xon_threshold() << endl;
data <<"pbc_csr.port_1.cfg_account_pg_0.headroom: 0x" << pbc_csr.port_1.cfg_account_pg_0.headroom() << endl;
data <<"pbc_csr.port_1.cfg_account_pg_0.reserved_min: 0x" << pbc_csr.port_1.cfg_account_pg_0.reserved_min() << endl;
data <<"pbc_csr.port_1.cfg_account_pg_0.low_limit: 0x" << pbc_csr.port_1.cfg_account_pg_0.low_limit() << endl;

    pbc_csr.port_1.cfg_account_pg_1.read();
    data <<"pbc_csr.port_1.cfg_account_pg_1.all: 0x" << pbc_csr.port_1.cfg_account_pg_1.all() << endl;
data <<"pbc_csr.port_1.cfg_account_pg_1.alpha: 0x" << pbc_csr.port_1.cfg_account_pg_1.alpha() << endl;
data <<"pbc_csr.port_1.cfg_account_pg_1.xon_threshold: 0x" << pbc_csr.port_1.cfg_account_pg_1.xon_threshold() << endl;
data <<"pbc_csr.port_1.cfg_account_pg_1.headroom: 0x" << pbc_csr.port_1.cfg_account_pg_1.headroom() << endl;
data <<"pbc_csr.port_1.cfg_account_pg_1.reserved_min: 0x" << pbc_csr.port_1.cfg_account_pg_1.reserved_min() << endl;
data <<"pbc_csr.port_1.cfg_account_pg_1.low_limit: 0x" << pbc_csr.port_1.cfg_account_pg_1.low_limit() << endl;

    pbc_csr.port_1.cfg_account_pg_2.read();
    data <<"pbc_csr.port_1.cfg_account_pg_2.all: 0x" << pbc_csr.port_1.cfg_account_pg_2.all() << endl;
data <<"pbc_csr.port_1.cfg_account_pg_2.alpha: 0x" << pbc_csr.port_1.cfg_account_pg_2.alpha() << endl;
data <<"pbc_csr.port_1.cfg_account_pg_2.xon_threshold: 0x" << pbc_csr.port_1.cfg_account_pg_2.xon_threshold() << endl;
data <<"pbc_csr.port_1.cfg_account_pg_2.headroom: 0x" << pbc_csr.port_1.cfg_account_pg_2.headroom() << endl;
data <<"pbc_csr.port_1.cfg_account_pg_2.reserved_min: 0x" << pbc_csr.port_1.cfg_account_pg_2.reserved_min() << endl;
data <<"pbc_csr.port_1.cfg_account_pg_2.low_limit: 0x" << pbc_csr.port_1.cfg_account_pg_2.low_limit() << endl;

    pbc_csr.port_1.cfg_account_pg_3.read();
    data <<"pbc_csr.port_1.cfg_account_pg_3.all: 0x" << pbc_csr.port_1.cfg_account_pg_3.all() << endl;
data <<"pbc_csr.port_1.cfg_account_pg_3.alpha: 0x" << pbc_csr.port_1.cfg_account_pg_3.alpha() << endl;
data <<"pbc_csr.port_1.cfg_account_pg_3.xon_threshold: 0x" << pbc_csr.port_1.cfg_account_pg_3.xon_threshold() << endl;
data <<"pbc_csr.port_1.cfg_account_pg_3.headroom: 0x" << pbc_csr.port_1.cfg_account_pg_3.headroom() << endl;
data <<"pbc_csr.port_1.cfg_account_pg_3.reserved_min: 0x" << pbc_csr.port_1.cfg_account_pg_3.reserved_min() << endl;
data <<"pbc_csr.port_1.cfg_account_pg_3.low_limit: 0x" << pbc_csr.port_1.cfg_account_pg_3.low_limit() << endl;

    pbc_csr.port_1.cfg_account_pg_4.read();
    data <<"pbc_csr.port_1.cfg_account_pg_4.all: 0x" << pbc_csr.port_1.cfg_account_pg_4.all() << endl;
data <<"pbc_csr.port_1.cfg_account_pg_4.alpha: 0x" << pbc_csr.port_1.cfg_account_pg_4.alpha() << endl;
data <<"pbc_csr.port_1.cfg_account_pg_4.xon_threshold: 0x" << pbc_csr.port_1.cfg_account_pg_4.xon_threshold() << endl;
data <<"pbc_csr.port_1.cfg_account_pg_4.headroom: 0x" << pbc_csr.port_1.cfg_account_pg_4.headroom() << endl;
data <<"pbc_csr.port_1.cfg_account_pg_4.reserved_min: 0x" << pbc_csr.port_1.cfg_account_pg_4.reserved_min() << endl;
data <<"pbc_csr.port_1.cfg_account_pg_4.low_limit: 0x" << pbc_csr.port_1.cfg_account_pg_4.low_limit() << endl;

    pbc_csr.port_1.cfg_account_pg_5.read();
    data <<"pbc_csr.port_1.cfg_account_pg_5.all: 0x" << pbc_csr.port_1.cfg_account_pg_5.all() << endl;
data <<"pbc_csr.port_1.cfg_account_pg_5.alpha: 0x" << pbc_csr.port_1.cfg_account_pg_5.alpha() << endl;
data <<"pbc_csr.port_1.cfg_account_pg_5.xon_threshold: 0x" << pbc_csr.port_1.cfg_account_pg_5.xon_threshold() << endl;
data <<"pbc_csr.port_1.cfg_account_pg_5.headroom: 0x" << pbc_csr.port_1.cfg_account_pg_5.headroom() << endl;
data <<"pbc_csr.port_1.cfg_account_pg_5.reserved_min: 0x" << pbc_csr.port_1.cfg_account_pg_5.reserved_min() << endl;
data <<"pbc_csr.port_1.cfg_account_pg_5.low_limit: 0x" << pbc_csr.port_1.cfg_account_pg_5.low_limit() << endl;

    pbc_csr.port_1.cfg_account_pg_6.read();
    data <<"pbc_csr.port_1.cfg_account_pg_6.all: 0x" << pbc_csr.port_1.cfg_account_pg_6.all() << endl;
data <<"pbc_csr.port_1.cfg_account_pg_6.alpha: 0x" << pbc_csr.port_1.cfg_account_pg_6.alpha() << endl;
data <<"pbc_csr.port_1.cfg_account_pg_6.xon_threshold: 0x" << pbc_csr.port_1.cfg_account_pg_6.xon_threshold() << endl;
data <<"pbc_csr.port_1.cfg_account_pg_6.headroom: 0x" << pbc_csr.port_1.cfg_account_pg_6.headroom() << endl;
data <<"pbc_csr.port_1.cfg_account_pg_6.reserved_min: 0x" << pbc_csr.port_1.cfg_account_pg_6.reserved_min() << endl;
data <<"pbc_csr.port_1.cfg_account_pg_6.low_limit: 0x" << pbc_csr.port_1.cfg_account_pg_6.low_limit() << endl;

    pbc_csr.port_1.cfg_account_pg_7.read();
    data <<"pbc_csr.port_1.cfg_account_pg_7.all: 0x" << pbc_csr.port_1.cfg_account_pg_7.all() << endl;
data <<"pbc_csr.port_1.cfg_account_pg_7.alpha: 0x" << pbc_csr.port_1.cfg_account_pg_7.alpha() << endl;
data <<"pbc_csr.port_1.cfg_account_pg_7.xon_threshold: 0x" << pbc_csr.port_1.cfg_account_pg_7.xon_threshold() << endl;
data <<"pbc_csr.port_1.cfg_account_pg_7.headroom: 0x" << pbc_csr.port_1.cfg_account_pg_7.headroom() << endl;
data <<"pbc_csr.port_1.cfg_account_pg_7.reserved_min: 0x" << pbc_csr.port_1.cfg_account_pg_7.reserved_min() << endl;
data <<"pbc_csr.port_1.cfg_account_pg_7.low_limit: 0x" << pbc_csr.port_1.cfg_account_pg_7.low_limit() << endl;

    pbc_csr.port_1.cfg_account_pg_to_sp.read();
    data <<"pbc_csr.port_1.cfg_account_pg_to_sp.all: 0x" << pbc_csr.port_1.cfg_account_pg_to_sp.all() << endl;
data <<"pbc_csr.port_1.cfg_account_pg_to_sp.map: 0x" << pbc_csr.port_1.cfg_account_pg_to_sp.map() << endl;

    pbc_csr.port_1.cfg_account_sp_fetch_quanta.read();
    data <<"pbc_csr.port_1.cfg_account_sp_fetch_quanta.all: 0x" << pbc_csr.port_1.cfg_account_sp_fetch_quanta.all() << endl;
data <<"pbc_csr.port_1.cfg_account_sp_fetch_quanta.pg6: 0x" << pbc_csr.port_1.cfg_account_sp_fetch_quanta.pg6() << endl;
data <<"pbc_csr.port_1.cfg_account_sp_fetch_quanta.pg7: 0x" << pbc_csr.port_1.cfg_account_sp_fetch_quanta.pg7() << endl;
data <<"pbc_csr.port_1.cfg_account_sp_fetch_quanta.pg4: 0x" << pbc_csr.port_1.cfg_account_sp_fetch_quanta.pg4() << endl;
data <<"pbc_csr.port_1.cfg_account_sp_fetch_quanta.pg5: 0x" << pbc_csr.port_1.cfg_account_sp_fetch_quanta.pg5() << endl;
data <<"pbc_csr.port_1.cfg_account_sp_fetch_quanta.pg2: 0x" << pbc_csr.port_1.cfg_account_sp_fetch_quanta.pg2() << endl;
data <<"pbc_csr.port_1.cfg_account_sp_fetch_quanta.pg3: 0x" << pbc_csr.port_1.cfg_account_sp_fetch_quanta.pg3() << endl;
data <<"pbc_csr.port_1.cfg_account_sp_fetch_quanta.pg0: 0x" << pbc_csr.port_1.cfg_account_sp_fetch_quanta.pg0() << endl;
data <<"pbc_csr.port_1.cfg_account_sp_fetch_quanta.pg1: 0x" << pbc_csr.port_1.cfg_account_sp_fetch_quanta.pg1() << endl;

    pbc_csr.port_1.cfg_account_sp_fetch_threshold.read();
    data <<"pbc_csr.port_1.cfg_account_sp_fetch_threshold.all: 0x" << pbc_csr.port_1.cfg_account_sp_fetch_threshold.all() << endl;
data <<"pbc_csr.port_1.cfg_account_sp_fetch_threshold.pg6: 0x" << pbc_csr.port_1.cfg_account_sp_fetch_threshold.pg6() << endl;
data <<"pbc_csr.port_1.cfg_account_sp_fetch_threshold.pg7: 0x" << pbc_csr.port_1.cfg_account_sp_fetch_threshold.pg7() << endl;
data <<"pbc_csr.port_1.cfg_account_sp_fetch_threshold.pg4: 0x" << pbc_csr.port_1.cfg_account_sp_fetch_threshold.pg4() << endl;
data <<"pbc_csr.port_1.cfg_account_sp_fetch_threshold.pg5: 0x" << pbc_csr.port_1.cfg_account_sp_fetch_threshold.pg5() << endl;
data <<"pbc_csr.port_1.cfg_account_sp_fetch_threshold.pg2: 0x" << pbc_csr.port_1.cfg_account_sp_fetch_threshold.pg2() << endl;
data <<"pbc_csr.port_1.cfg_account_sp_fetch_threshold.pg3: 0x" << pbc_csr.port_1.cfg_account_sp_fetch_threshold.pg3() << endl;
data <<"pbc_csr.port_1.cfg_account_sp_fetch_threshold.pg0: 0x" << pbc_csr.port_1.cfg_account_sp_fetch_threshold.pg0() << endl;
data <<"pbc_csr.port_1.cfg_account_sp_fetch_threshold.pg1: 0x" << pbc_csr.port_1.cfg_account_sp_fetch_threshold.pg1() << endl;

    pbc_csr.port_1.cfg_account_tc_to_pg.read();
    data <<"pbc_csr.port_1.cfg_account_tc_to_pg.all: 0x" << pbc_csr.port_1.cfg_account_tc_to_pg.all() << endl;
data <<"pbc_csr.port_1.cfg_account_tc_to_pg.table: 0x" << pbc_csr.port_1.cfg_account_tc_to_pg.table() << endl;

    pbc_csr.port_1.cfg_latency_cache.read();
    data <<"pbc_csr.port_1.cfg_latency_cache.all: 0x" << pbc_csr.port_1.cfg_latency_cache.all() << endl;
data <<"pbc_csr.port_1.cfg_latency_cache.enable: 0x" << pbc_csr.port_1.cfg_latency_cache.enable() << endl;

    pbc_csr.port_1.cfg_mac_xoff.read();
    data <<"pbc_csr.port_1.cfg_mac_xoff.all: 0x" << pbc_csr.port_1.cfg_mac_xoff.all() << endl;
data <<"pbc_csr.port_1.cfg_mac_xoff.enable: 0x" << pbc_csr.port_1.cfg_mac_xoff.enable() << endl;

    pbc_csr.port_1.cfg_oq.read();
    data <<"pbc_csr.port_1.cfg_oq.all: 0x" << pbc_csr.port_1.cfg_oq.all() << endl;
data <<"pbc_csr.port_1.cfg_oq.enable: 0x" << pbc_csr.port_1.cfg_oq.enable() << endl;
data <<"pbc_csr.port_1.cfg_oq.num_hdr_bytes: 0x" << pbc_csr.port_1.cfg_oq.num_hdr_bytes() << endl;
data <<"pbc_csr.port_1.cfg_oq.num_hdr_bytes_crypto: 0x" << pbc_csr.port_1.cfg_oq.num_hdr_bytes_crypto() << endl;
data <<"pbc_csr.port_1.cfg_oq.span_cpu_num_cells: 0x" << pbc_csr.port_1.cfg_oq.span_cpu_num_cells() << endl;
data <<"pbc_csr.port_1.cfg_oq.flush_hw_error: 0x" << pbc_csr.port_1.cfg_oq.flush_hw_error() << endl;
data <<"pbc_csr.port_1.cfg_oq.eg_ts_enable: 0x" << pbc_csr.port_1.cfg_oq.eg_ts_enable() << endl;
data <<"pbc_csr.port_1.cfg_oq.ipg_bytes: 0x" << pbc_csr.port_1.cfg_oq.ipg_bytes() << endl;
data <<"pbc_csr.port_1.cfg_oq.flush: 0x" << pbc_csr.port_1.cfg_oq.flush() << endl;
data <<"pbc_csr.port_1.cfg_oq.flow_control_enable_xoff: 0x" << pbc_csr.port_1.cfg_oq.flow_control_enable_xoff() << endl;
data <<"pbc_csr.port_1.cfg_oq.hw_error_to_pbus: 0x" << pbc_csr.port_1.cfg_oq.hw_error_to_pbus() << endl;
data <<"pbc_csr.port_1.cfg_oq.rewrite_enable: 0x" << pbc_csr.port_1.cfg_oq.rewrite_enable() << endl;

    pbc_csr.port_1.cfg_oq_arb_l1_selection.read();
    data <<"pbc_csr.port_1.cfg_oq_arb_l1_selection.all: 0x" << pbc_csr.port_1.cfg_oq_arb_l1_selection.all() << endl;
data <<"pbc_csr.port_1.cfg_oq_arb_l1_selection.node_13: 0x" << pbc_csr.port_1.cfg_oq_arb_l1_selection.node_13() << endl;
data <<"pbc_csr.port_1.cfg_oq_arb_l1_selection.node_14: 0x" << pbc_csr.port_1.cfg_oq_arb_l1_selection.node_14() << endl;
data <<"pbc_csr.port_1.cfg_oq_arb_l1_selection.node_9: 0x" << pbc_csr.port_1.cfg_oq_arb_l1_selection.node_9() << endl;
data <<"pbc_csr.port_1.cfg_oq_arb_l1_selection.node_8: 0x" << pbc_csr.port_1.cfg_oq_arb_l1_selection.node_8() << endl;
data <<"pbc_csr.port_1.cfg_oq_arb_l1_selection.node_7: 0x" << pbc_csr.port_1.cfg_oq_arb_l1_selection.node_7() << endl;
data <<"pbc_csr.port_1.cfg_oq_arb_l1_selection.node_6: 0x" << pbc_csr.port_1.cfg_oq_arb_l1_selection.node_6() << endl;
data <<"pbc_csr.port_1.cfg_oq_arb_l1_selection.node_5: 0x" << pbc_csr.port_1.cfg_oq_arb_l1_selection.node_5() << endl;
data <<"pbc_csr.port_1.cfg_oq_arb_l1_selection.node_4: 0x" << pbc_csr.port_1.cfg_oq_arb_l1_selection.node_4() << endl;
data <<"pbc_csr.port_1.cfg_oq_arb_l1_selection.node_3: 0x" << pbc_csr.port_1.cfg_oq_arb_l1_selection.node_3() << endl;
data <<"pbc_csr.port_1.cfg_oq_arb_l1_selection.node_2: 0x" << pbc_csr.port_1.cfg_oq_arb_l1_selection.node_2() << endl;
data <<"pbc_csr.port_1.cfg_oq_arb_l1_selection.node_1: 0x" << pbc_csr.port_1.cfg_oq_arb_l1_selection.node_1() << endl;
data <<"pbc_csr.port_1.cfg_oq_arb_l1_selection.node_0: 0x" << pbc_csr.port_1.cfg_oq_arb_l1_selection.node_0() << endl;
data <<"pbc_csr.port_1.cfg_oq_arb_l1_selection.node_11: 0x" << pbc_csr.port_1.cfg_oq_arb_l1_selection.node_11() << endl;
data <<"pbc_csr.port_1.cfg_oq_arb_l1_selection.node_10: 0x" << pbc_csr.port_1.cfg_oq_arb_l1_selection.node_10() << endl;
data <<"pbc_csr.port_1.cfg_oq_arb_l1_selection.node_15: 0x" << pbc_csr.port_1.cfg_oq_arb_l1_selection.node_15() << endl;
data <<"pbc_csr.port_1.cfg_oq_arb_l1_selection.node_12: 0x" << pbc_csr.port_1.cfg_oq_arb_l1_selection.node_12() << endl;

    pbc_csr.port_1.cfg_oq_arb_l1_strict.read();
    data <<"pbc_csr.port_1.cfg_oq_arb_l1_strict.all: 0x" << pbc_csr.port_1.cfg_oq_arb_l1_strict.all() << endl;
data <<"pbc_csr.port_1.cfg_oq_arb_l1_strict.priority: 0x" << pbc_csr.port_1.cfg_oq_arb_l1_strict.priority() << endl;

    pbc_csr.port_1.cfg_oq_arb_l2_selection.read();
    data <<"pbc_csr.port_1.cfg_oq_arb_l2_selection.all: 0x" << pbc_csr.port_1.cfg_oq_arb_l2_selection.all() << endl;
data <<"pbc_csr.port_1.cfg_oq_arb_l2_selection.node_3: 0x" << pbc_csr.port_1.cfg_oq_arb_l2_selection.node_3() << endl;
data <<"pbc_csr.port_1.cfg_oq_arb_l2_selection.node_2: 0x" << pbc_csr.port_1.cfg_oq_arb_l2_selection.node_2() << endl;
data <<"pbc_csr.port_1.cfg_oq_arb_l2_selection.node_1: 0x" << pbc_csr.port_1.cfg_oq_arb_l2_selection.node_1() << endl;
data <<"pbc_csr.port_1.cfg_oq_arb_l2_selection.node_0: 0x" << pbc_csr.port_1.cfg_oq_arb_l2_selection.node_0() << endl;

    pbc_csr.port_1.cfg_oq_arb_l2_strict.read();
    data <<"pbc_csr.port_1.cfg_oq_arb_l2_strict.all: 0x" << pbc_csr.port_1.cfg_oq_arb_l2_strict.all() << endl;
data <<"pbc_csr.port_1.cfg_oq_arb_l2_strict.priority: 0x" << pbc_csr.port_1.cfg_oq_arb_l2_strict.priority() << endl;

    pbc_csr.port_1.cfg_oq_dhs_mem.read();
    data <<"pbc_csr.port_1.cfg_oq_dhs_mem.all: 0x" << pbc_csr.port_1.cfg_oq_dhs_mem.all() << endl;
data <<"pbc_csr.port_1.cfg_oq_dhs_mem.address: 0x" << pbc_csr.port_1.cfg_oq_dhs_mem.address() << endl;

    pbc_csr.port_1.cfg_oq_queue.read();
    data <<"pbc_csr.port_1.cfg_oq_queue.all: 0x" << pbc_csr.port_1.cfg_oq_queue.all() << endl;
data <<"pbc_csr.port_1.cfg_oq_queue.enable: 0x" << pbc_csr.port_1.cfg_oq_queue.enable() << endl;
data <<"pbc_csr.port_1.cfg_oq_queue.flush: 0x" << pbc_csr.port_1.cfg_oq_queue.flush() << endl;

    pbc_csr.port_1.cfg_oq_queue_dynamic.read();
    data <<"pbc_csr.port_1.cfg_oq_queue_dynamic.all: 0x" << pbc_csr.port_1.cfg_oq_queue_dynamic.all() << endl;
data <<"pbc_csr.port_1.cfg_oq_queue_dynamic.threshold: 0x" << pbc_csr.port_1.cfg_oq_queue_dynamic.threshold() << endl;

    pbc_csr.port_1.cfg_oq_queue_write_one.read();
    data <<"pbc_csr.port_1.cfg_oq_queue_write_one.all: 0x" << pbc_csr.port_1.cfg_oq_queue_write_one.all() << endl;
data <<"pbc_csr.port_1.cfg_oq_queue_write_one.enable: 0x" << pbc_csr.port_1.cfg_oq_queue_write_one.enable() << endl;
data <<"pbc_csr.port_1.cfg_oq_queue_write_one.disable: 0x" << pbc_csr.port_1.cfg_oq_queue_write_one.disable() << endl;

    pbc_csr.port_1.cfg_oq_xoff2oq.read();
    data <<"pbc_csr.port_1.cfg_oq_xoff2oq.all: 0x" << pbc_csr.port_1.cfg_oq_xoff2oq.all() << endl;
data <<"pbc_csr.port_1.cfg_oq_xoff2oq.map: 0x" << pbc_csr.port_1.cfg_oq_xoff2oq.map() << endl;

    pbc_csr.port_1.cfg_tail_drop.read();
    data <<"pbc_csr.port_1.cfg_tail_drop.all: 0x" << pbc_csr.port_1.cfg_tail_drop.all() << endl;
data <<"pbc_csr.port_1.cfg_tail_drop.cpu_threshold: 0x" << pbc_csr.port_1.cfg_tail_drop.cpu_threshold() << endl;
data <<"pbc_csr.port_1.cfg_tail_drop.span_threshold: 0x" << pbc_csr.port_1.cfg_tail_drop.span_threshold() << endl;

    pbc_csr.port_1.cfg_write_control.read();
    data <<"pbc_csr.port_1.cfg_write_control.all: 0x" << pbc_csr.port_1.cfg_write_control.all() << endl;
data <<"pbc_csr.port_1.cfg_write_control.release_cells: 0x" << pbc_csr.port_1.cfg_write_control.release_cells() << endl;
data <<"pbc_csr.port_1.cfg_write_control.enable: 0x" << pbc_csr.port_1.cfg_write_control.enable() << endl;
data <<"pbc_csr.port_1.cfg_write_control.drop_on_error: 0x" << pbc_csr.port_1.cfg_write_control.drop_on_error() << endl;
data <<"pbc_csr.port_1.cfg_write_control.rewrite: 0x" << pbc_csr.port_1.cfg_write_control.rewrite() << endl;
data <<"pbc_csr.port_1.cfg_write_control.span_pg_enable: 0x" << pbc_csr.port_1.cfg_write_control.span_pg_enable() << endl;
data <<"pbc_csr.port_1.cfg_write_control.span_pg_index: 0x" << pbc_csr.port_1.cfg_write_control.span_pg_index() << endl;
data <<"pbc_csr.port_1.cfg_write_control.enforce_oq24_31: 0x" << pbc_csr.port_1.cfg_write_control.enforce_oq24_31() << endl;
data <<"pbc_csr.port_1.cfg_write_control.packing_timeout_cycles: 0x" << pbc_csr.port_1.cfg_write_control.packing_timeout_cycles() << endl;
data <<"pbc_csr.port_1.cfg_write_control.min_size: 0x" << pbc_csr.port_1.cfg_write_control.min_size() << endl;
data <<"pbc_csr.port_1.cfg_write_control.packing_same_pg: 0x" << pbc_csr.port_1.cfg_write_control.packing_same_pg() << endl;
data <<"pbc_csr.port_1.cfg_write_control.packing: 0x" << pbc_csr.port_1.cfg_write_control.packing() << endl;
data <<"pbc_csr.port_1.cfg_write_control.enqueue_mux_bypass: 0x" << pbc_csr.port_1.cfg_write_control.enqueue_mux_bypass() << endl;
data <<"pbc_csr.port_1.cfg_write_control.enable_drdy: 0x" << pbc_csr.port_1.cfg_write_control.enable_drdy() << endl;
data <<"pbc_csr.port_1.cfg_write_control.rate_limiter: 0x" << pbc_csr.port_1.cfg_write_control.rate_limiter() << endl;
data <<"pbc_csr.port_1.cfg_write_control.cut_thru: 0x" << pbc_csr.port_1.cfg_write_control.cut_thru() << endl;
data <<"pbc_csr.port_1.cfg_write_control.enable_mnic_multicast: 0x" << pbc_csr.port_1.cfg_write_control.enable_mnic_multicast() << endl;
data <<"pbc_csr.port_1.cfg_write_control.enable_bmc_multicast: 0x" << pbc_csr.port_1.cfg_write_control.enable_bmc_multicast() << endl;

    pbc_csr.port_2.cfg_account_clear_xoff.read();
    data <<"pbc_csr.port_2.cfg_account_clear_xoff.all: 0x" << pbc_csr.port_2.cfg_account_clear_xoff.all() << endl;
data <<"pbc_csr.port_2.cfg_account_clear_xoff.bitmap: 0x" << pbc_csr.port_2.cfg_account_clear_xoff.bitmap() << endl;

    pbc_csr.port_2.cfg_account_control.read();
    data <<"pbc_csr.port_2.cfg_account_control.all: 0x" << pbc_csr.port_2.cfg_account_control.all() << endl;
data <<"pbc_csr.port_2.cfg_account_control.sp_enable: 0x" << pbc_csr.port_2.cfg_account_control.sp_enable() << endl;
data <<"pbc_csr.port_2.cfg_account_control.clear_wm_index: 0x" << pbc_csr.port_2.cfg_account_control.clear_wm_index() << endl;
data <<"pbc_csr.port_2.cfg_account_control.clear_wm_enable: 0x" << pbc_csr.port_2.cfg_account_control.clear_wm_enable() << endl;
data <<"pbc_csr.port_2.cfg_account_control.use_sp_as_wm: 0x" << pbc_csr.port_2.cfg_account_control.use_sp_as_wm() << endl;

    pbc_csr.port_2.cfg_account_credit_return.read();
    data <<"pbc_csr.port_2.cfg_account_credit_return.all: 0x" << pbc_csr.port_2.cfg_account_credit_return.all() << endl;
data <<"pbc_csr.port_2.cfg_account_credit_return.enable: 0x" << pbc_csr.port_2.cfg_account_credit_return.enable() << endl;

    pbc_csr.port_2.cfg_account_mtu_table.read();
    data <<"pbc_csr.port_2.cfg_account_mtu_table.all: 0x" << pbc_csr.port_2.cfg_account_mtu_table.all() << endl;
data <<"pbc_csr.port_2.cfg_account_mtu_table.pg6: 0x" << pbc_csr.port_2.cfg_account_mtu_table.pg6() << endl;
data <<"pbc_csr.port_2.cfg_account_mtu_table.pg7: 0x" << pbc_csr.port_2.cfg_account_mtu_table.pg7() << endl;
data <<"pbc_csr.port_2.cfg_account_mtu_table.pg4: 0x" << pbc_csr.port_2.cfg_account_mtu_table.pg4() << endl;
data <<"pbc_csr.port_2.cfg_account_mtu_table.pg5: 0x" << pbc_csr.port_2.cfg_account_mtu_table.pg5() << endl;
data <<"pbc_csr.port_2.cfg_account_mtu_table.pg2: 0x" << pbc_csr.port_2.cfg_account_mtu_table.pg2() << endl;
data <<"pbc_csr.port_2.cfg_account_mtu_table.pg3: 0x" << pbc_csr.port_2.cfg_account_mtu_table.pg3() << endl;
data <<"pbc_csr.port_2.cfg_account_mtu_table.pg0: 0x" << pbc_csr.port_2.cfg_account_mtu_table.pg0() << endl;
data <<"pbc_csr.port_2.cfg_account_mtu_table.pg1: 0x" << pbc_csr.port_2.cfg_account_mtu_table.pg1() << endl;

    pbc_csr.port_2.cfg_account_pause_timer.read();
    data <<"pbc_csr.port_2.cfg_account_pause_timer.all: 0x" << pbc_csr.port_2.cfg_account_pause_timer.all() << endl;
data <<"pbc_csr.port_2.cfg_account_pause_timer.xoff_timeout_auto_clear: 0x" << pbc_csr.port_2.cfg_account_pause_timer.xoff_timeout_auto_clear() << endl;
data <<"pbc_csr.port_2.cfg_account_pause_timer.xoff_timeout_auto_xon: 0x" << pbc_csr.port_2.cfg_account_pause_timer.xoff_timeout_auto_xon() << endl;
data <<"pbc_csr.port_2.cfg_account_pause_timer.xoff_timeout: 0x" << pbc_csr.port_2.cfg_account_pause_timer.xoff_timeout() << endl;

    pbc_csr.port_2.cfg_account_pause_timer_enable.read();
    data <<"pbc_csr.port_2.cfg_account_pause_timer_enable.all: 0x" << pbc_csr.port_2.cfg_account_pause_timer_enable.all() << endl;
data <<"pbc_csr.port_2.cfg_account_pause_timer_enable.iq: 0x" << pbc_csr.port_2.cfg_account_pause_timer_enable.iq() << endl;

    pbc_csr.port_2.cfg_account_pg_0.read();
    data <<"pbc_csr.port_2.cfg_account_pg_0.all: 0x" << pbc_csr.port_2.cfg_account_pg_0.all() << endl;
data <<"pbc_csr.port_2.cfg_account_pg_0.alpha: 0x" << pbc_csr.port_2.cfg_account_pg_0.alpha() << endl;
data <<"pbc_csr.port_2.cfg_account_pg_0.xon_threshold: 0x" << pbc_csr.port_2.cfg_account_pg_0.xon_threshold() << endl;
data <<"pbc_csr.port_2.cfg_account_pg_0.headroom: 0x" << pbc_csr.port_2.cfg_account_pg_0.headroom() << endl;
data <<"pbc_csr.port_2.cfg_account_pg_0.reserved_min: 0x" << pbc_csr.port_2.cfg_account_pg_0.reserved_min() << endl;
data <<"pbc_csr.port_2.cfg_account_pg_0.low_limit: 0x" << pbc_csr.port_2.cfg_account_pg_0.low_limit() << endl;

    pbc_csr.port_2.cfg_account_pg_1.read();
    data <<"pbc_csr.port_2.cfg_account_pg_1.all: 0x" << pbc_csr.port_2.cfg_account_pg_1.all() << endl;
data <<"pbc_csr.port_2.cfg_account_pg_1.alpha: 0x" << pbc_csr.port_2.cfg_account_pg_1.alpha() << endl;
data <<"pbc_csr.port_2.cfg_account_pg_1.xon_threshold: 0x" << pbc_csr.port_2.cfg_account_pg_1.xon_threshold() << endl;
data <<"pbc_csr.port_2.cfg_account_pg_1.headroom: 0x" << pbc_csr.port_2.cfg_account_pg_1.headroom() << endl;
data <<"pbc_csr.port_2.cfg_account_pg_1.reserved_min: 0x" << pbc_csr.port_2.cfg_account_pg_1.reserved_min() << endl;
data <<"pbc_csr.port_2.cfg_account_pg_1.low_limit: 0x" << pbc_csr.port_2.cfg_account_pg_1.low_limit() << endl;

    pbc_csr.port_2.cfg_account_pg_2.read();
    data <<"pbc_csr.port_2.cfg_account_pg_2.all: 0x" << pbc_csr.port_2.cfg_account_pg_2.all() << endl;
data <<"pbc_csr.port_2.cfg_account_pg_2.alpha: 0x" << pbc_csr.port_2.cfg_account_pg_2.alpha() << endl;
data <<"pbc_csr.port_2.cfg_account_pg_2.xon_threshold: 0x" << pbc_csr.port_2.cfg_account_pg_2.xon_threshold() << endl;
data <<"pbc_csr.port_2.cfg_account_pg_2.headroom: 0x" << pbc_csr.port_2.cfg_account_pg_2.headroom() << endl;
data <<"pbc_csr.port_2.cfg_account_pg_2.reserved_min: 0x" << pbc_csr.port_2.cfg_account_pg_2.reserved_min() << endl;
data <<"pbc_csr.port_2.cfg_account_pg_2.low_limit: 0x" << pbc_csr.port_2.cfg_account_pg_2.low_limit() << endl;

    pbc_csr.port_2.cfg_account_pg_3.read();
    data <<"pbc_csr.port_2.cfg_account_pg_3.all: 0x" << pbc_csr.port_2.cfg_account_pg_3.all() << endl;
data <<"pbc_csr.port_2.cfg_account_pg_3.alpha: 0x" << pbc_csr.port_2.cfg_account_pg_3.alpha() << endl;
data <<"pbc_csr.port_2.cfg_account_pg_3.xon_threshold: 0x" << pbc_csr.port_2.cfg_account_pg_3.xon_threshold() << endl;
data <<"pbc_csr.port_2.cfg_account_pg_3.headroom: 0x" << pbc_csr.port_2.cfg_account_pg_3.headroom() << endl;
data <<"pbc_csr.port_2.cfg_account_pg_3.reserved_min: 0x" << pbc_csr.port_2.cfg_account_pg_3.reserved_min() << endl;
data <<"pbc_csr.port_2.cfg_account_pg_3.low_limit: 0x" << pbc_csr.port_2.cfg_account_pg_3.low_limit() << endl;

    pbc_csr.port_2.cfg_account_pg_4.read();
    data <<"pbc_csr.port_2.cfg_account_pg_4.all: 0x" << pbc_csr.port_2.cfg_account_pg_4.all() << endl;
data <<"pbc_csr.port_2.cfg_account_pg_4.alpha: 0x" << pbc_csr.port_2.cfg_account_pg_4.alpha() << endl;
data <<"pbc_csr.port_2.cfg_account_pg_4.xon_threshold: 0x" << pbc_csr.port_2.cfg_account_pg_4.xon_threshold() << endl;
data <<"pbc_csr.port_2.cfg_account_pg_4.headroom: 0x" << pbc_csr.port_2.cfg_account_pg_4.headroom() << endl;
data <<"pbc_csr.port_2.cfg_account_pg_4.reserved_min: 0x" << pbc_csr.port_2.cfg_account_pg_4.reserved_min() << endl;
data <<"pbc_csr.port_2.cfg_account_pg_4.low_limit: 0x" << pbc_csr.port_2.cfg_account_pg_4.low_limit() << endl;

    pbc_csr.port_2.cfg_account_pg_5.read();
    data <<"pbc_csr.port_2.cfg_account_pg_5.all: 0x" << pbc_csr.port_2.cfg_account_pg_5.all() << endl;
data <<"pbc_csr.port_2.cfg_account_pg_5.alpha: 0x" << pbc_csr.port_2.cfg_account_pg_5.alpha() << endl;
data <<"pbc_csr.port_2.cfg_account_pg_5.xon_threshold: 0x" << pbc_csr.port_2.cfg_account_pg_5.xon_threshold() << endl;
data <<"pbc_csr.port_2.cfg_account_pg_5.headroom: 0x" << pbc_csr.port_2.cfg_account_pg_5.headroom() << endl;
data <<"pbc_csr.port_2.cfg_account_pg_5.reserved_min: 0x" << pbc_csr.port_2.cfg_account_pg_5.reserved_min() << endl;
data <<"pbc_csr.port_2.cfg_account_pg_5.low_limit: 0x" << pbc_csr.port_2.cfg_account_pg_5.low_limit() << endl;

    pbc_csr.port_2.cfg_account_pg_6.read();
    data <<"pbc_csr.port_2.cfg_account_pg_6.all: 0x" << pbc_csr.port_2.cfg_account_pg_6.all() << endl;
data <<"pbc_csr.port_2.cfg_account_pg_6.alpha: 0x" << pbc_csr.port_2.cfg_account_pg_6.alpha() << endl;
data <<"pbc_csr.port_2.cfg_account_pg_6.xon_threshold: 0x" << pbc_csr.port_2.cfg_account_pg_6.xon_threshold() << endl;
data <<"pbc_csr.port_2.cfg_account_pg_6.headroom: 0x" << pbc_csr.port_2.cfg_account_pg_6.headroom() << endl;
data <<"pbc_csr.port_2.cfg_account_pg_6.reserved_min: 0x" << pbc_csr.port_2.cfg_account_pg_6.reserved_min() << endl;
data <<"pbc_csr.port_2.cfg_account_pg_6.low_limit: 0x" << pbc_csr.port_2.cfg_account_pg_6.low_limit() << endl;

    pbc_csr.port_2.cfg_account_pg_7.read();
    data <<"pbc_csr.port_2.cfg_account_pg_7.all: 0x" << pbc_csr.port_2.cfg_account_pg_7.all() << endl;
data <<"pbc_csr.port_2.cfg_account_pg_7.alpha: 0x" << pbc_csr.port_2.cfg_account_pg_7.alpha() << endl;
data <<"pbc_csr.port_2.cfg_account_pg_7.xon_threshold: 0x" << pbc_csr.port_2.cfg_account_pg_7.xon_threshold() << endl;
data <<"pbc_csr.port_2.cfg_account_pg_7.headroom: 0x" << pbc_csr.port_2.cfg_account_pg_7.headroom() << endl;
data <<"pbc_csr.port_2.cfg_account_pg_7.reserved_min: 0x" << pbc_csr.port_2.cfg_account_pg_7.reserved_min() << endl;
data <<"pbc_csr.port_2.cfg_account_pg_7.low_limit: 0x" << pbc_csr.port_2.cfg_account_pg_7.low_limit() << endl;

    pbc_csr.port_2.cfg_account_pg_to_sp.read();
    data <<"pbc_csr.port_2.cfg_account_pg_to_sp.all: 0x" << pbc_csr.port_2.cfg_account_pg_to_sp.all() << endl;
data <<"pbc_csr.port_2.cfg_account_pg_to_sp.map: 0x" << pbc_csr.port_2.cfg_account_pg_to_sp.map() << endl;

    pbc_csr.port_2.cfg_account_sp_fetch_quanta.read();
    data <<"pbc_csr.port_2.cfg_account_sp_fetch_quanta.all: 0x" << pbc_csr.port_2.cfg_account_sp_fetch_quanta.all() << endl;
data <<"pbc_csr.port_2.cfg_account_sp_fetch_quanta.pg6: 0x" << pbc_csr.port_2.cfg_account_sp_fetch_quanta.pg6() << endl;
data <<"pbc_csr.port_2.cfg_account_sp_fetch_quanta.pg7: 0x" << pbc_csr.port_2.cfg_account_sp_fetch_quanta.pg7() << endl;
data <<"pbc_csr.port_2.cfg_account_sp_fetch_quanta.pg4: 0x" << pbc_csr.port_2.cfg_account_sp_fetch_quanta.pg4() << endl;
data <<"pbc_csr.port_2.cfg_account_sp_fetch_quanta.pg5: 0x" << pbc_csr.port_2.cfg_account_sp_fetch_quanta.pg5() << endl;
data <<"pbc_csr.port_2.cfg_account_sp_fetch_quanta.pg2: 0x" << pbc_csr.port_2.cfg_account_sp_fetch_quanta.pg2() << endl;
data <<"pbc_csr.port_2.cfg_account_sp_fetch_quanta.pg3: 0x" << pbc_csr.port_2.cfg_account_sp_fetch_quanta.pg3() << endl;
data <<"pbc_csr.port_2.cfg_account_sp_fetch_quanta.pg0: 0x" << pbc_csr.port_2.cfg_account_sp_fetch_quanta.pg0() << endl;
data <<"pbc_csr.port_2.cfg_account_sp_fetch_quanta.pg1: 0x" << pbc_csr.port_2.cfg_account_sp_fetch_quanta.pg1() << endl;

    pbc_csr.port_2.cfg_account_sp_fetch_threshold.read();
    data <<"pbc_csr.port_2.cfg_account_sp_fetch_threshold.all: 0x" << pbc_csr.port_2.cfg_account_sp_fetch_threshold.all() << endl;
data <<"pbc_csr.port_2.cfg_account_sp_fetch_threshold.pg6: 0x" << pbc_csr.port_2.cfg_account_sp_fetch_threshold.pg6() << endl;
data <<"pbc_csr.port_2.cfg_account_sp_fetch_threshold.pg7: 0x" << pbc_csr.port_2.cfg_account_sp_fetch_threshold.pg7() << endl;
data <<"pbc_csr.port_2.cfg_account_sp_fetch_threshold.pg4: 0x" << pbc_csr.port_2.cfg_account_sp_fetch_threshold.pg4() << endl;
data <<"pbc_csr.port_2.cfg_account_sp_fetch_threshold.pg5: 0x" << pbc_csr.port_2.cfg_account_sp_fetch_threshold.pg5() << endl;
data <<"pbc_csr.port_2.cfg_account_sp_fetch_threshold.pg2: 0x" << pbc_csr.port_2.cfg_account_sp_fetch_threshold.pg2() << endl;
data <<"pbc_csr.port_2.cfg_account_sp_fetch_threshold.pg3: 0x" << pbc_csr.port_2.cfg_account_sp_fetch_threshold.pg3() << endl;
data <<"pbc_csr.port_2.cfg_account_sp_fetch_threshold.pg0: 0x" << pbc_csr.port_2.cfg_account_sp_fetch_threshold.pg0() << endl;
data <<"pbc_csr.port_2.cfg_account_sp_fetch_threshold.pg1: 0x" << pbc_csr.port_2.cfg_account_sp_fetch_threshold.pg1() << endl;

    pbc_csr.port_2.cfg_account_tc_to_pg.read();
    data <<"pbc_csr.port_2.cfg_account_tc_to_pg.all: 0x" << pbc_csr.port_2.cfg_account_tc_to_pg.all() << endl;
data <<"pbc_csr.port_2.cfg_account_tc_to_pg.table: 0x" << pbc_csr.port_2.cfg_account_tc_to_pg.table() << endl;

    pbc_csr.port_2.cfg_latency_cache.read();
    data <<"pbc_csr.port_2.cfg_latency_cache.all: 0x" << pbc_csr.port_2.cfg_latency_cache.all() << endl;
data <<"pbc_csr.port_2.cfg_latency_cache.enable: 0x" << pbc_csr.port_2.cfg_latency_cache.enable() << endl;

    pbc_csr.port_2.cfg_mac_xoff.read();
    data <<"pbc_csr.port_2.cfg_mac_xoff.all: 0x" << pbc_csr.port_2.cfg_mac_xoff.all() << endl;
data <<"pbc_csr.port_2.cfg_mac_xoff.enable: 0x" << pbc_csr.port_2.cfg_mac_xoff.enable() << endl;

    pbc_csr.port_2.cfg_oq.read();
    data <<"pbc_csr.port_2.cfg_oq.all: 0x" << pbc_csr.port_2.cfg_oq.all() << endl;
data <<"pbc_csr.port_2.cfg_oq.enable: 0x" << pbc_csr.port_2.cfg_oq.enable() << endl;
data <<"pbc_csr.port_2.cfg_oq.num_hdr_bytes: 0x" << pbc_csr.port_2.cfg_oq.num_hdr_bytes() << endl;
data <<"pbc_csr.port_2.cfg_oq.num_hdr_bytes_crypto: 0x" << pbc_csr.port_2.cfg_oq.num_hdr_bytes_crypto() << endl;
data <<"pbc_csr.port_2.cfg_oq.span_cpu_num_cells: 0x" << pbc_csr.port_2.cfg_oq.span_cpu_num_cells() << endl;
data <<"pbc_csr.port_2.cfg_oq.flush_hw_error: 0x" << pbc_csr.port_2.cfg_oq.flush_hw_error() << endl;
data <<"pbc_csr.port_2.cfg_oq.eg_ts_enable: 0x" << pbc_csr.port_2.cfg_oq.eg_ts_enable() << endl;
data <<"pbc_csr.port_2.cfg_oq.ipg_bytes: 0x" << pbc_csr.port_2.cfg_oq.ipg_bytes() << endl;
data <<"pbc_csr.port_2.cfg_oq.flush: 0x" << pbc_csr.port_2.cfg_oq.flush() << endl;
data <<"pbc_csr.port_2.cfg_oq.flow_control_enable_xoff: 0x" << pbc_csr.port_2.cfg_oq.flow_control_enable_xoff() << endl;
data <<"pbc_csr.port_2.cfg_oq.hw_error_to_pbus: 0x" << pbc_csr.port_2.cfg_oq.hw_error_to_pbus() << endl;
data <<"pbc_csr.port_2.cfg_oq.rewrite_enable: 0x" << pbc_csr.port_2.cfg_oq.rewrite_enable() << endl;

    pbc_csr.port_2.cfg_oq_arb_l1_selection.read();
    data <<"pbc_csr.port_2.cfg_oq_arb_l1_selection.all: 0x" << pbc_csr.port_2.cfg_oq_arb_l1_selection.all() << endl;
data <<"pbc_csr.port_2.cfg_oq_arb_l1_selection.node_13: 0x" << pbc_csr.port_2.cfg_oq_arb_l1_selection.node_13() << endl;
data <<"pbc_csr.port_2.cfg_oq_arb_l1_selection.node_14: 0x" << pbc_csr.port_2.cfg_oq_arb_l1_selection.node_14() << endl;
data <<"pbc_csr.port_2.cfg_oq_arb_l1_selection.node_9: 0x" << pbc_csr.port_2.cfg_oq_arb_l1_selection.node_9() << endl;
data <<"pbc_csr.port_2.cfg_oq_arb_l1_selection.node_8: 0x" << pbc_csr.port_2.cfg_oq_arb_l1_selection.node_8() << endl;
data <<"pbc_csr.port_2.cfg_oq_arb_l1_selection.node_7: 0x" << pbc_csr.port_2.cfg_oq_arb_l1_selection.node_7() << endl;
data <<"pbc_csr.port_2.cfg_oq_arb_l1_selection.node_6: 0x" << pbc_csr.port_2.cfg_oq_arb_l1_selection.node_6() << endl;
data <<"pbc_csr.port_2.cfg_oq_arb_l1_selection.node_5: 0x" << pbc_csr.port_2.cfg_oq_arb_l1_selection.node_5() << endl;
data <<"pbc_csr.port_2.cfg_oq_arb_l1_selection.node_4: 0x" << pbc_csr.port_2.cfg_oq_arb_l1_selection.node_4() << endl;
data <<"pbc_csr.port_2.cfg_oq_arb_l1_selection.node_3: 0x" << pbc_csr.port_2.cfg_oq_arb_l1_selection.node_3() << endl;
data <<"pbc_csr.port_2.cfg_oq_arb_l1_selection.node_2: 0x" << pbc_csr.port_2.cfg_oq_arb_l1_selection.node_2() << endl;
data <<"pbc_csr.port_2.cfg_oq_arb_l1_selection.node_1: 0x" << pbc_csr.port_2.cfg_oq_arb_l1_selection.node_1() << endl;
data <<"pbc_csr.port_2.cfg_oq_arb_l1_selection.node_0: 0x" << pbc_csr.port_2.cfg_oq_arb_l1_selection.node_0() << endl;
data <<"pbc_csr.port_2.cfg_oq_arb_l1_selection.node_11: 0x" << pbc_csr.port_2.cfg_oq_arb_l1_selection.node_11() << endl;
data <<"pbc_csr.port_2.cfg_oq_arb_l1_selection.node_10: 0x" << pbc_csr.port_2.cfg_oq_arb_l1_selection.node_10() << endl;
data <<"pbc_csr.port_2.cfg_oq_arb_l1_selection.node_15: 0x" << pbc_csr.port_2.cfg_oq_arb_l1_selection.node_15() << endl;
data <<"pbc_csr.port_2.cfg_oq_arb_l1_selection.node_12: 0x" << pbc_csr.port_2.cfg_oq_arb_l1_selection.node_12() << endl;

    pbc_csr.port_2.cfg_oq_arb_l1_strict.read();
    data <<"pbc_csr.port_2.cfg_oq_arb_l1_strict.all: 0x" << pbc_csr.port_2.cfg_oq_arb_l1_strict.all() << endl;
data <<"pbc_csr.port_2.cfg_oq_arb_l1_strict.priority: 0x" << pbc_csr.port_2.cfg_oq_arb_l1_strict.priority() << endl;

    pbc_csr.port_2.cfg_oq_arb_l2_selection.read();
    data <<"pbc_csr.port_2.cfg_oq_arb_l2_selection.all: 0x" << pbc_csr.port_2.cfg_oq_arb_l2_selection.all() << endl;
data <<"pbc_csr.port_2.cfg_oq_arb_l2_selection.node_3: 0x" << pbc_csr.port_2.cfg_oq_arb_l2_selection.node_3() << endl;
data <<"pbc_csr.port_2.cfg_oq_arb_l2_selection.node_2: 0x" << pbc_csr.port_2.cfg_oq_arb_l2_selection.node_2() << endl;
data <<"pbc_csr.port_2.cfg_oq_arb_l2_selection.node_1: 0x" << pbc_csr.port_2.cfg_oq_arb_l2_selection.node_1() << endl;
data <<"pbc_csr.port_2.cfg_oq_arb_l2_selection.node_0: 0x" << pbc_csr.port_2.cfg_oq_arb_l2_selection.node_0() << endl;

    pbc_csr.port_2.cfg_oq_arb_l2_strict.read();
    data <<"pbc_csr.port_2.cfg_oq_arb_l2_strict.all: 0x" << pbc_csr.port_2.cfg_oq_arb_l2_strict.all() << endl;
data <<"pbc_csr.port_2.cfg_oq_arb_l2_strict.priority: 0x" << pbc_csr.port_2.cfg_oq_arb_l2_strict.priority() << endl;

    pbc_csr.port_2.cfg_oq_dhs_mem.read();
    data <<"pbc_csr.port_2.cfg_oq_dhs_mem.all: 0x" << pbc_csr.port_2.cfg_oq_dhs_mem.all() << endl;
data <<"pbc_csr.port_2.cfg_oq_dhs_mem.address: 0x" << pbc_csr.port_2.cfg_oq_dhs_mem.address() << endl;

    pbc_csr.port_2.cfg_oq_queue.read();
    data <<"pbc_csr.port_2.cfg_oq_queue.all: 0x" << pbc_csr.port_2.cfg_oq_queue.all() << endl;
data <<"pbc_csr.port_2.cfg_oq_queue.enable: 0x" << pbc_csr.port_2.cfg_oq_queue.enable() << endl;
data <<"pbc_csr.port_2.cfg_oq_queue.flush: 0x" << pbc_csr.port_2.cfg_oq_queue.flush() << endl;

    pbc_csr.port_2.cfg_oq_queue_dynamic.read();
    data <<"pbc_csr.port_2.cfg_oq_queue_dynamic.all: 0x" << pbc_csr.port_2.cfg_oq_queue_dynamic.all() << endl;
data <<"pbc_csr.port_2.cfg_oq_queue_dynamic.threshold: 0x" << pbc_csr.port_2.cfg_oq_queue_dynamic.threshold() << endl;

    pbc_csr.port_2.cfg_oq_queue_write_one.read();
    data <<"pbc_csr.port_2.cfg_oq_queue_write_one.all: 0x" << pbc_csr.port_2.cfg_oq_queue_write_one.all() << endl;
data <<"pbc_csr.port_2.cfg_oq_queue_write_one.enable: 0x" << pbc_csr.port_2.cfg_oq_queue_write_one.enable() << endl;
data <<"pbc_csr.port_2.cfg_oq_queue_write_one.disable: 0x" << pbc_csr.port_2.cfg_oq_queue_write_one.disable() << endl;

    pbc_csr.port_2.cfg_oq_xoff2oq.read();
    data <<"pbc_csr.port_2.cfg_oq_xoff2oq.all: 0x" << pbc_csr.port_2.cfg_oq_xoff2oq.all() << endl;
data <<"pbc_csr.port_2.cfg_oq_xoff2oq.map: 0x" << pbc_csr.port_2.cfg_oq_xoff2oq.map() << endl;

    pbc_csr.port_2.cfg_tail_drop.read();
    data <<"pbc_csr.port_2.cfg_tail_drop.all: 0x" << pbc_csr.port_2.cfg_tail_drop.all() << endl;
data <<"pbc_csr.port_2.cfg_tail_drop.cpu_threshold: 0x" << pbc_csr.port_2.cfg_tail_drop.cpu_threshold() << endl;
data <<"pbc_csr.port_2.cfg_tail_drop.span_threshold: 0x" << pbc_csr.port_2.cfg_tail_drop.span_threshold() << endl;

    pbc_csr.port_2.cfg_write_control.read();
    data <<"pbc_csr.port_2.cfg_write_control.all: 0x" << pbc_csr.port_2.cfg_write_control.all() << endl;
data <<"pbc_csr.port_2.cfg_write_control.release_cells: 0x" << pbc_csr.port_2.cfg_write_control.release_cells() << endl;
data <<"pbc_csr.port_2.cfg_write_control.enable: 0x" << pbc_csr.port_2.cfg_write_control.enable() << endl;
data <<"pbc_csr.port_2.cfg_write_control.drop_on_error: 0x" << pbc_csr.port_2.cfg_write_control.drop_on_error() << endl;
data <<"pbc_csr.port_2.cfg_write_control.rewrite: 0x" << pbc_csr.port_2.cfg_write_control.rewrite() << endl;
data <<"pbc_csr.port_2.cfg_write_control.span_pg_enable: 0x" << pbc_csr.port_2.cfg_write_control.span_pg_enable() << endl;
data <<"pbc_csr.port_2.cfg_write_control.span_pg_index: 0x" << pbc_csr.port_2.cfg_write_control.span_pg_index() << endl;
data <<"pbc_csr.port_2.cfg_write_control.enforce_oq24_31: 0x" << pbc_csr.port_2.cfg_write_control.enforce_oq24_31() << endl;
data <<"pbc_csr.port_2.cfg_write_control.packing_timeout_cycles: 0x" << pbc_csr.port_2.cfg_write_control.packing_timeout_cycles() << endl;
data <<"pbc_csr.port_2.cfg_write_control.min_size: 0x" << pbc_csr.port_2.cfg_write_control.min_size() << endl;
data <<"pbc_csr.port_2.cfg_write_control.packing_same_pg: 0x" << pbc_csr.port_2.cfg_write_control.packing_same_pg() << endl;
data <<"pbc_csr.port_2.cfg_write_control.packing: 0x" << pbc_csr.port_2.cfg_write_control.packing() << endl;
data <<"pbc_csr.port_2.cfg_write_control.enqueue_mux_bypass: 0x" << pbc_csr.port_2.cfg_write_control.enqueue_mux_bypass() << endl;
data <<"pbc_csr.port_2.cfg_write_control.enable_drdy: 0x" << pbc_csr.port_2.cfg_write_control.enable_drdy() << endl;
data <<"pbc_csr.port_2.cfg_write_control.rate_limiter: 0x" << pbc_csr.port_2.cfg_write_control.rate_limiter() << endl;
data <<"pbc_csr.port_2.cfg_write_control.cut_thru: 0x" << pbc_csr.port_2.cfg_write_control.cut_thru() << endl;
data <<"pbc_csr.port_2.cfg_write_control.enable_mnic_multicast: 0x" << pbc_csr.port_2.cfg_write_control.enable_mnic_multicast() << endl;
data <<"pbc_csr.port_2.cfg_write_control.enable_bmc_multicast: 0x" << pbc_csr.port_2.cfg_write_control.enable_bmc_multicast() << endl;

    pbc_csr.port_3.cfg_account_clear_xoff.read();
    data <<"pbc_csr.port_3.cfg_account_clear_xoff.all: 0x" << pbc_csr.port_3.cfg_account_clear_xoff.all() << endl;
data <<"pbc_csr.port_3.cfg_account_clear_xoff.bitmap: 0x" << pbc_csr.port_3.cfg_account_clear_xoff.bitmap() << endl;

    pbc_csr.port_3.cfg_account_control.read();
    data <<"pbc_csr.port_3.cfg_account_control.all: 0x" << pbc_csr.port_3.cfg_account_control.all() << endl;
data <<"pbc_csr.port_3.cfg_account_control.sp_enable: 0x" << pbc_csr.port_3.cfg_account_control.sp_enable() << endl;
data <<"pbc_csr.port_3.cfg_account_control.clear_wm_index: 0x" << pbc_csr.port_3.cfg_account_control.clear_wm_index() << endl;
data <<"pbc_csr.port_3.cfg_account_control.clear_wm_enable: 0x" << pbc_csr.port_3.cfg_account_control.clear_wm_enable() << endl;
data <<"pbc_csr.port_3.cfg_account_control.use_sp_as_wm: 0x" << pbc_csr.port_3.cfg_account_control.use_sp_as_wm() << endl;

    pbc_csr.port_3.cfg_account_credit_return.read();
    data <<"pbc_csr.port_3.cfg_account_credit_return.all: 0x" << pbc_csr.port_3.cfg_account_credit_return.all() << endl;
data <<"pbc_csr.port_3.cfg_account_credit_return.enable: 0x" << pbc_csr.port_3.cfg_account_credit_return.enable() << endl;

    pbc_csr.port_3.cfg_account_mtu_table.read();
    data <<"pbc_csr.port_3.cfg_account_mtu_table.all: 0x" << pbc_csr.port_3.cfg_account_mtu_table.all() << endl;
data <<"pbc_csr.port_3.cfg_account_mtu_table.pg6: 0x" << pbc_csr.port_3.cfg_account_mtu_table.pg6() << endl;
data <<"pbc_csr.port_3.cfg_account_mtu_table.pg7: 0x" << pbc_csr.port_3.cfg_account_mtu_table.pg7() << endl;
data <<"pbc_csr.port_3.cfg_account_mtu_table.pg4: 0x" << pbc_csr.port_3.cfg_account_mtu_table.pg4() << endl;
data <<"pbc_csr.port_3.cfg_account_mtu_table.pg5: 0x" << pbc_csr.port_3.cfg_account_mtu_table.pg5() << endl;
data <<"pbc_csr.port_3.cfg_account_mtu_table.pg2: 0x" << pbc_csr.port_3.cfg_account_mtu_table.pg2() << endl;
data <<"pbc_csr.port_3.cfg_account_mtu_table.pg3: 0x" << pbc_csr.port_3.cfg_account_mtu_table.pg3() << endl;
data <<"pbc_csr.port_3.cfg_account_mtu_table.pg0: 0x" << pbc_csr.port_3.cfg_account_mtu_table.pg0() << endl;
data <<"pbc_csr.port_3.cfg_account_mtu_table.pg1: 0x" << pbc_csr.port_3.cfg_account_mtu_table.pg1() << endl;

    pbc_csr.port_3.cfg_account_pause_timer.read();
    data <<"pbc_csr.port_3.cfg_account_pause_timer.all: 0x" << pbc_csr.port_3.cfg_account_pause_timer.all() << endl;
data <<"pbc_csr.port_3.cfg_account_pause_timer.xoff_timeout_auto_clear: 0x" << pbc_csr.port_3.cfg_account_pause_timer.xoff_timeout_auto_clear() << endl;
data <<"pbc_csr.port_3.cfg_account_pause_timer.xoff_timeout_auto_xon: 0x" << pbc_csr.port_3.cfg_account_pause_timer.xoff_timeout_auto_xon() << endl;
data <<"pbc_csr.port_3.cfg_account_pause_timer.xoff_timeout: 0x" << pbc_csr.port_3.cfg_account_pause_timer.xoff_timeout() << endl;

    pbc_csr.port_3.cfg_account_pause_timer_enable.read();
    data <<"pbc_csr.port_3.cfg_account_pause_timer_enable.all: 0x" << pbc_csr.port_3.cfg_account_pause_timer_enable.all() << endl;
data <<"pbc_csr.port_3.cfg_account_pause_timer_enable.iq: 0x" << pbc_csr.port_3.cfg_account_pause_timer_enable.iq() << endl;

    pbc_csr.port_3.cfg_account_pg_0.read();
    data <<"pbc_csr.port_3.cfg_account_pg_0.all: 0x" << pbc_csr.port_3.cfg_account_pg_0.all() << endl;
data <<"pbc_csr.port_3.cfg_account_pg_0.alpha: 0x" << pbc_csr.port_3.cfg_account_pg_0.alpha() << endl;
data <<"pbc_csr.port_3.cfg_account_pg_0.xon_threshold: 0x" << pbc_csr.port_3.cfg_account_pg_0.xon_threshold() << endl;
data <<"pbc_csr.port_3.cfg_account_pg_0.headroom: 0x" << pbc_csr.port_3.cfg_account_pg_0.headroom() << endl;
data <<"pbc_csr.port_3.cfg_account_pg_0.reserved_min: 0x" << pbc_csr.port_3.cfg_account_pg_0.reserved_min() << endl;
data <<"pbc_csr.port_3.cfg_account_pg_0.low_limit: 0x" << pbc_csr.port_3.cfg_account_pg_0.low_limit() << endl;

    pbc_csr.port_3.cfg_account_pg_1.read();
    data <<"pbc_csr.port_3.cfg_account_pg_1.all: 0x" << pbc_csr.port_3.cfg_account_pg_1.all() << endl;
data <<"pbc_csr.port_3.cfg_account_pg_1.alpha: 0x" << pbc_csr.port_3.cfg_account_pg_1.alpha() << endl;
data <<"pbc_csr.port_3.cfg_account_pg_1.xon_threshold: 0x" << pbc_csr.port_3.cfg_account_pg_1.xon_threshold() << endl;
data <<"pbc_csr.port_3.cfg_account_pg_1.headroom: 0x" << pbc_csr.port_3.cfg_account_pg_1.headroom() << endl;
data <<"pbc_csr.port_3.cfg_account_pg_1.reserved_min: 0x" << pbc_csr.port_3.cfg_account_pg_1.reserved_min() << endl;
data <<"pbc_csr.port_3.cfg_account_pg_1.low_limit: 0x" << pbc_csr.port_3.cfg_account_pg_1.low_limit() << endl;

    pbc_csr.port_3.cfg_account_pg_2.read();
    data <<"pbc_csr.port_3.cfg_account_pg_2.all: 0x" << pbc_csr.port_3.cfg_account_pg_2.all() << endl;
data <<"pbc_csr.port_3.cfg_account_pg_2.alpha: 0x" << pbc_csr.port_3.cfg_account_pg_2.alpha() << endl;
data <<"pbc_csr.port_3.cfg_account_pg_2.xon_threshold: 0x" << pbc_csr.port_3.cfg_account_pg_2.xon_threshold() << endl;
data <<"pbc_csr.port_3.cfg_account_pg_2.headroom: 0x" << pbc_csr.port_3.cfg_account_pg_2.headroom() << endl;
data <<"pbc_csr.port_3.cfg_account_pg_2.reserved_min: 0x" << pbc_csr.port_3.cfg_account_pg_2.reserved_min() << endl;
data <<"pbc_csr.port_3.cfg_account_pg_2.low_limit: 0x" << pbc_csr.port_3.cfg_account_pg_2.low_limit() << endl;

    pbc_csr.port_3.cfg_account_pg_3.read();
    data <<"pbc_csr.port_3.cfg_account_pg_3.all: 0x" << pbc_csr.port_3.cfg_account_pg_3.all() << endl;
data <<"pbc_csr.port_3.cfg_account_pg_3.alpha: 0x" << pbc_csr.port_3.cfg_account_pg_3.alpha() << endl;
data <<"pbc_csr.port_3.cfg_account_pg_3.xon_threshold: 0x" << pbc_csr.port_3.cfg_account_pg_3.xon_threshold() << endl;
data <<"pbc_csr.port_3.cfg_account_pg_3.headroom: 0x" << pbc_csr.port_3.cfg_account_pg_3.headroom() << endl;
data <<"pbc_csr.port_3.cfg_account_pg_3.reserved_min: 0x" << pbc_csr.port_3.cfg_account_pg_3.reserved_min() << endl;
data <<"pbc_csr.port_3.cfg_account_pg_3.low_limit: 0x" << pbc_csr.port_3.cfg_account_pg_3.low_limit() << endl;

    pbc_csr.port_3.cfg_account_pg_4.read();
    data <<"pbc_csr.port_3.cfg_account_pg_4.all: 0x" << pbc_csr.port_3.cfg_account_pg_4.all() << endl;
data <<"pbc_csr.port_3.cfg_account_pg_4.alpha: 0x" << pbc_csr.port_3.cfg_account_pg_4.alpha() << endl;
data <<"pbc_csr.port_3.cfg_account_pg_4.xon_threshold: 0x" << pbc_csr.port_3.cfg_account_pg_4.xon_threshold() << endl;
data <<"pbc_csr.port_3.cfg_account_pg_4.headroom: 0x" << pbc_csr.port_3.cfg_account_pg_4.headroom() << endl;
data <<"pbc_csr.port_3.cfg_account_pg_4.reserved_min: 0x" << pbc_csr.port_3.cfg_account_pg_4.reserved_min() << endl;
data <<"pbc_csr.port_3.cfg_account_pg_4.low_limit: 0x" << pbc_csr.port_3.cfg_account_pg_4.low_limit() << endl;

    pbc_csr.port_3.cfg_account_pg_5.read();
    data <<"pbc_csr.port_3.cfg_account_pg_5.all: 0x" << pbc_csr.port_3.cfg_account_pg_5.all() << endl;
data <<"pbc_csr.port_3.cfg_account_pg_5.alpha: 0x" << pbc_csr.port_3.cfg_account_pg_5.alpha() << endl;
data <<"pbc_csr.port_3.cfg_account_pg_5.xon_threshold: 0x" << pbc_csr.port_3.cfg_account_pg_5.xon_threshold() << endl;
data <<"pbc_csr.port_3.cfg_account_pg_5.headroom: 0x" << pbc_csr.port_3.cfg_account_pg_5.headroom() << endl;
data <<"pbc_csr.port_3.cfg_account_pg_5.reserved_min: 0x" << pbc_csr.port_3.cfg_account_pg_5.reserved_min() << endl;
data <<"pbc_csr.port_3.cfg_account_pg_5.low_limit: 0x" << pbc_csr.port_3.cfg_account_pg_5.low_limit() << endl;

    pbc_csr.port_3.cfg_account_pg_6.read();
    data <<"pbc_csr.port_3.cfg_account_pg_6.all: 0x" << pbc_csr.port_3.cfg_account_pg_6.all() << endl;
data <<"pbc_csr.port_3.cfg_account_pg_6.alpha: 0x" << pbc_csr.port_3.cfg_account_pg_6.alpha() << endl;
data <<"pbc_csr.port_3.cfg_account_pg_6.xon_threshold: 0x" << pbc_csr.port_3.cfg_account_pg_6.xon_threshold() << endl;
data <<"pbc_csr.port_3.cfg_account_pg_6.headroom: 0x" << pbc_csr.port_3.cfg_account_pg_6.headroom() << endl;
data <<"pbc_csr.port_3.cfg_account_pg_6.reserved_min: 0x" << pbc_csr.port_3.cfg_account_pg_6.reserved_min() << endl;
data <<"pbc_csr.port_3.cfg_account_pg_6.low_limit: 0x" << pbc_csr.port_3.cfg_account_pg_6.low_limit() << endl;

    pbc_csr.port_3.cfg_account_pg_7.read();
    data <<"pbc_csr.port_3.cfg_account_pg_7.all: 0x" << pbc_csr.port_3.cfg_account_pg_7.all() << endl;
data <<"pbc_csr.port_3.cfg_account_pg_7.alpha: 0x" << pbc_csr.port_3.cfg_account_pg_7.alpha() << endl;
data <<"pbc_csr.port_3.cfg_account_pg_7.xon_threshold: 0x" << pbc_csr.port_3.cfg_account_pg_7.xon_threshold() << endl;
data <<"pbc_csr.port_3.cfg_account_pg_7.headroom: 0x" << pbc_csr.port_3.cfg_account_pg_7.headroom() << endl;
data <<"pbc_csr.port_3.cfg_account_pg_7.reserved_min: 0x" << pbc_csr.port_3.cfg_account_pg_7.reserved_min() << endl;
data <<"pbc_csr.port_3.cfg_account_pg_7.low_limit: 0x" << pbc_csr.port_3.cfg_account_pg_7.low_limit() << endl;

    pbc_csr.port_3.cfg_account_pg_to_sp.read();
    data <<"pbc_csr.port_3.cfg_account_pg_to_sp.all: 0x" << pbc_csr.port_3.cfg_account_pg_to_sp.all() << endl;
data <<"pbc_csr.port_3.cfg_account_pg_to_sp.map: 0x" << pbc_csr.port_3.cfg_account_pg_to_sp.map() << endl;

    pbc_csr.port_3.cfg_account_sp_fetch_quanta.read();
    data <<"pbc_csr.port_3.cfg_account_sp_fetch_quanta.all: 0x" << pbc_csr.port_3.cfg_account_sp_fetch_quanta.all() << endl;
data <<"pbc_csr.port_3.cfg_account_sp_fetch_quanta.pg6: 0x" << pbc_csr.port_3.cfg_account_sp_fetch_quanta.pg6() << endl;
data <<"pbc_csr.port_3.cfg_account_sp_fetch_quanta.pg7: 0x" << pbc_csr.port_3.cfg_account_sp_fetch_quanta.pg7() << endl;
data <<"pbc_csr.port_3.cfg_account_sp_fetch_quanta.pg4: 0x" << pbc_csr.port_3.cfg_account_sp_fetch_quanta.pg4() << endl;
data <<"pbc_csr.port_3.cfg_account_sp_fetch_quanta.pg5: 0x" << pbc_csr.port_3.cfg_account_sp_fetch_quanta.pg5() << endl;
data <<"pbc_csr.port_3.cfg_account_sp_fetch_quanta.pg2: 0x" << pbc_csr.port_3.cfg_account_sp_fetch_quanta.pg2() << endl;
data <<"pbc_csr.port_3.cfg_account_sp_fetch_quanta.pg3: 0x" << pbc_csr.port_3.cfg_account_sp_fetch_quanta.pg3() << endl;
data <<"pbc_csr.port_3.cfg_account_sp_fetch_quanta.pg0: 0x" << pbc_csr.port_3.cfg_account_sp_fetch_quanta.pg0() << endl;
data <<"pbc_csr.port_3.cfg_account_sp_fetch_quanta.pg1: 0x" << pbc_csr.port_3.cfg_account_sp_fetch_quanta.pg1() << endl;

    pbc_csr.port_3.cfg_account_sp_fetch_threshold.read();
    data <<"pbc_csr.port_3.cfg_account_sp_fetch_threshold.all: 0x" << pbc_csr.port_3.cfg_account_sp_fetch_threshold.all() << endl;
data <<"pbc_csr.port_3.cfg_account_sp_fetch_threshold.pg6: 0x" << pbc_csr.port_3.cfg_account_sp_fetch_threshold.pg6() << endl;
data <<"pbc_csr.port_3.cfg_account_sp_fetch_threshold.pg7: 0x" << pbc_csr.port_3.cfg_account_sp_fetch_threshold.pg7() << endl;
data <<"pbc_csr.port_3.cfg_account_sp_fetch_threshold.pg4: 0x" << pbc_csr.port_3.cfg_account_sp_fetch_threshold.pg4() << endl;
data <<"pbc_csr.port_3.cfg_account_sp_fetch_threshold.pg5: 0x" << pbc_csr.port_3.cfg_account_sp_fetch_threshold.pg5() << endl;
data <<"pbc_csr.port_3.cfg_account_sp_fetch_threshold.pg2: 0x" << pbc_csr.port_3.cfg_account_sp_fetch_threshold.pg2() << endl;
data <<"pbc_csr.port_3.cfg_account_sp_fetch_threshold.pg3: 0x" << pbc_csr.port_3.cfg_account_sp_fetch_threshold.pg3() << endl;
data <<"pbc_csr.port_3.cfg_account_sp_fetch_threshold.pg0: 0x" << pbc_csr.port_3.cfg_account_sp_fetch_threshold.pg0() << endl;
data <<"pbc_csr.port_3.cfg_account_sp_fetch_threshold.pg1: 0x" << pbc_csr.port_3.cfg_account_sp_fetch_threshold.pg1() << endl;

    pbc_csr.port_3.cfg_account_tc_to_pg.read();
    data <<"pbc_csr.port_3.cfg_account_tc_to_pg.all: 0x" << pbc_csr.port_3.cfg_account_tc_to_pg.all() << endl;
data <<"pbc_csr.port_3.cfg_account_tc_to_pg.table: 0x" << pbc_csr.port_3.cfg_account_tc_to_pg.table() << endl;

    pbc_csr.port_3.cfg_latency_cache.read();
    data <<"pbc_csr.port_3.cfg_latency_cache.all: 0x" << pbc_csr.port_3.cfg_latency_cache.all() << endl;
data <<"pbc_csr.port_3.cfg_latency_cache.enable: 0x" << pbc_csr.port_3.cfg_latency_cache.enable() << endl;

    pbc_csr.port_3.cfg_mac_xoff.read();
    data <<"pbc_csr.port_3.cfg_mac_xoff.all: 0x" << pbc_csr.port_3.cfg_mac_xoff.all() << endl;
data <<"pbc_csr.port_3.cfg_mac_xoff.enable: 0x" << pbc_csr.port_3.cfg_mac_xoff.enable() << endl;

    pbc_csr.port_3.cfg_oq.read();
    data <<"pbc_csr.port_3.cfg_oq.all: 0x" << pbc_csr.port_3.cfg_oq.all() << endl;
data <<"pbc_csr.port_3.cfg_oq.enable: 0x" << pbc_csr.port_3.cfg_oq.enable() << endl;
data <<"pbc_csr.port_3.cfg_oq.num_hdr_bytes: 0x" << pbc_csr.port_3.cfg_oq.num_hdr_bytes() << endl;
data <<"pbc_csr.port_3.cfg_oq.num_hdr_bytes_crypto: 0x" << pbc_csr.port_3.cfg_oq.num_hdr_bytes_crypto() << endl;
data <<"pbc_csr.port_3.cfg_oq.span_cpu_num_cells: 0x" << pbc_csr.port_3.cfg_oq.span_cpu_num_cells() << endl;
data <<"pbc_csr.port_3.cfg_oq.flush_hw_error: 0x" << pbc_csr.port_3.cfg_oq.flush_hw_error() << endl;
data <<"pbc_csr.port_3.cfg_oq.eg_ts_enable: 0x" << pbc_csr.port_3.cfg_oq.eg_ts_enable() << endl;
data <<"pbc_csr.port_3.cfg_oq.ipg_bytes: 0x" << pbc_csr.port_3.cfg_oq.ipg_bytes() << endl;
data <<"pbc_csr.port_3.cfg_oq.flush: 0x" << pbc_csr.port_3.cfg_oq.flush() << endl;
data <<"pbc_csr.port_3.cfg_oq.flow_control_enable_xoff: 0x" << pbc_csr.port_3.cfg_oq.flow_control_enable_xoff() << endl;
data <<"pbc_csr.port_3.cfg_oq.hw_error_to_pbus: 0x" << pbc_csr.port_3.cfg_oq.hw_error_to_pbus() << endl;
data <<"pbc_csr.port_3.cfg_oq.rewrite_enable: 0x" << pbc_csr.port_3.cfg_oq.rewrite_enable() << endl;

    pbc_csr.port_3.cfg_oq_arb_l1_selection.read();
    data <<"pbc_csr.port_3.cfg_oq_arb_l1_selection.all: 0x" << pbc_csr.port_3.cfg_oq_arb_l1_selection.all() << endl;
data <<"pbc_csr.port_3.cfg_oq_arb_l1_selection.node_13: 0x" << pbc_csr.port_3.cfg_oq_arb_l1_selection.node_13() << endl;
data <<"pbc_csr.port_3.cfg_oq_arb_l1_selection.node_14: 0x" << pbc_csr.port_3.cfg_oq_arb_l1_selection.node_14() << endl;
data <<"pbc_csr.port_3.cfg_oq_arb_l1_selection.node_9: 0x" << pbc_csr.port_3.cfg_oq_arb_l1_selection.node_9() << endl;
data <<"pbc_csr.port_3.cfg_oq_arb_l1_selection.node_8: 0x" << pbc_csr.port_3.cfg_oq_arb_l1_selection.node_8() << endl;
data <<"pbc_csr.port_3.cfg_oq_arb_l1_selection.node_7: 0x" << pbc_csr.port_3.cfg_oq_arb_l1_selection.node_7() << endl;
data <<"pbc_csr.port_3.cfg_oq_arb_l1_selection.node_6: 0x" << pbc_csr.port_3.cfg_oq_arb_l1_selection.node_6() << endl;
data <<"pbc_csr.port_3.cfg_oq_arb_l1_selection.node_5: 0x" << pbc_csr.port_3.cfg_oq_arb_l1_selection.node_5() << endl;
data <<"pbc_csr.port_3.cfg_oq_arb_l1_selection.node_4: 0x" << pbc_csr.port_3.cfg_oq_arb_l1_selection.node_4() << endl;
data <<"pbc_csr.port_3.cfg_oq_arb_l1_selection.node_3: 0x" << pbc_csr.port_3.cfg_oq_arb_l1_selection.node_3() << endl;
data <<"pbc_csr.port_3.cfg_oq_arb_l1_selection.node_2: 0x" << pbc_csr.port_3.cfg_oq_arb_l1_selection.node_2() << endl;
data <<"pbc_csr.port_3.cfg_oq_arb_l1_selection.node_1: 0x" << pbc_csr.port_3.cfg_oq_arb_l1_selection.node_1() << endl;
data <<"pbc_csr.port_3.cfg_oq_arb_l1_selection.node_0: 0x" << pbc_csr.port_3.cfg_oq_arb_l1_selection.node_0() << endl;
data <<"pbc_csr.port_3.cfg_oq_arb_l1_selection.node_11: 0x" << pbc_csr.port_3.cfg_oq_arb_l1_selection.node_11() << endl;
data <<"pbc_csr.port_3.cfg_oq_arb_l1_selection.node_10: 0x" << pbc_csr.port_3.cfg_oq_arb_l1_selection.node_10() << endl;
data <<"pbc_csr.port_3.cfg_oq_arb_l1_selection.node_15: 0x" << pbc_csr.port_3.cfg_oq_arb_l1_selection.node_15() << endl;
data <<"pbc_csr.port_3.cfg_oq_arb_l1_selection.node_12: 0x" << pbc_csr.port_3.cfg_oq_arb_l1_selection.node_12() << endl;

    pbc_csr.port_3.cfg_oq_arb_l1_strict.read();
    data <<"pbc_csr.port_3.cfg_oq_arb_l1_strict.all: 0x" << pbc_csr.port_3.cfg_oq_arb_l1_strict.all() << endl;
data <<"pbc_csr.port_3.cfg_oq_arb_l1_strict.priority: 0x" << pbc_csr.port_3.cfg_oq_arb_l1_strict.priority() << endl;

    pbc_csr.port_3.cfg_oq_arb_l2_selection.read();
    data <<"pbc_csr.port_3.cfg_oq_arb_l2_selection.all: 0x" << pbc_csr.port_3.cfg_oq_arb_l2_selection.all() << endl;
data <<"pbc_csr.port_3.cfg_oq_arb_l2_selection.node_3: 0x" << pbc_csr.port_3.cfg_oq_arb_l2_selection.node_3() << endl;
data <<"pbc_csr.port_3.cfg_oq_arb_l2_selection.node_2: 0x" << pbc_csr.port_3.cfg_oq_arb_l2_selection.node_2() << endl;
data <<"pbc_csr.port_3.cfg_oq_arb_l2_selection.node_1: 0x" << pbc_csr.port_3.cfg_oq_arb_l2_selection.node_1() << endl;
data <<"pbc_csr.port_3.cfg_oq_arb_l2_selection.node_0: 0x" << pbc_csr.port_3.cfg_oq_arb_l2_selection.node_0() << endl;

    pbc_csr.port_3.cfg_oq_arb_l2_strict.read();
    data <<"pbc_csr.port_3.cfg_oq_arb_l2_strict.all: 0x" << pbc_csr.port_3.cfg_oq_arb_l2_strict.all() << endl;
data <<"pbc_csr.port_3.cfg_oq_arb_l2_strict.priority: 0x" << pbc_csr.port_3.cfg_oq_arb_l2_strict.priority() << endl;

    pbc_csr.port_3.cfg_oq_dhs_mem.read();
    data <<"pbc_csr.port_3.cfg_oq_dhs_mem.all: 0x" << pbc_csr.port_3.cfg_oq_dhs_mem.all() << endl;
data <<"pbc_csr.port_3.cfg_oq_dhs_mem.address: 0x" << pbc_csr.port_3.cfg_oq_dhs_mem.address() << endl;

    pbc_csr.port_3.cfg_oq_queue.read();
    data <<"pbc_csr.port_3.cfg_oq_queue.all: 0x" << pbc_csr.port_3.cfg_oq_queue.all() << endl;
data <<"pbc_csr.port_3.cfg_oq_queue.enable: 0x" << pbc_csr.port_3.cfg_oq_queue.enable() << endl;
data <<"pbc_csr.port_3.cfg_oq_queue.flush: 0x" << pbc_csr.port_3.cfg_oq_queue.flush() << endl;

    pbc_csr.port_3.cfg_oq_queue_dynamic.read();
    data <<"pbc_csr.port_3.cfg_oq_queue_dynamic.all: 0x" << pbc_csr.port_3.cfg_oq_queue_dynamic.all() << endl;
data <<"pbc_csr.port_3.cfg_oq_queue_dynamic.threshold: 0x" << pbc_csr.port_3.cfg_oq_queue_dynamic.threshold() << endl;

    pbc_csr.port_3.cfg_oq_queue_write_one.read();
    data <<"pbc_csr.port_3.cfg_oq_queue_write_one.all: 0x" << pbc_csr.port_3.cfg_oq_queue_write_one.all() << endl;
data <<"pbc_csr.port_3.cfg_oq_queue_write_one.enable: 0x" << pbc_csr.port_3.cfg_oq_queue_write_one.enable() << endl;
data <<"pbc_csr.port_3.cfg_oq_queue_write_one.disable: 0x" << pbc_csr.port_3.cfg_oq_queue_write_one.disable() << endl;

    pbc_csr.port_3.cfg_oq_xoff2oq.read();
    data <<"pbc_csr.port_3.cfg_oq_xoff2oq.all: 0x" << pbc_csr.port_3.cfg_oq_xoff2oq.all() << endl;
data <<"pbc_csr.port_3.cfg_oq_xoff2oq.map: 0x" << pbc_csr.port_3.cfg_oq_xoff2oq.map() << endl;

    pbc_csr.port_3.cfg_tail_drop.read();
    data <<"pbc_csr.port_3.cfg_tail_drop.all: 0x" << pbc_csr.port_3.cfg_tail_drop.all() << endl;
data <<"pbc_csr.port_3.cfg_tail_drop.cpu_threshold: 0x" << pbc_csr.port_3.cfg_tail_drop.cpu_threshold() << endl;
data <<"pbc_csr.port_3.cfg_tail_drop.span_threshold: 0x" << pbc_csr.port_3.cfg_tail_drop.span_threshold() << endl;

    pbc_csr.port_3.cfg_write_control.read();
    data <<"pbc_csr.port_3.cfg_write_control.all: 0x" << pbc_csr.port_3.cfg_write_control.all() << endl;
data <<"pbc_csr.port_3.cfg_write_control.release_cells: 0x" << pbc_csr.port_3.cfg_write_control.release_cells() << endl;
data <<"pbc_csr.port_3.cfg_write_control.enable: 0x" << pbc_csr.port_3.cfg_write_control.enable() << endl;
data <<"pbc_csr.port_3.cfg_write_control.drop_on_error: 0x" << pbc_csr.port_3.cfg_write_control.drop_on_error() << endl;
data <<"pbc_csr.port_3.cfg_write_control.rewrite: 0x" << pbc_csr.port_3.cfg_write_control.rewrite() << endl;
data <<"pbc_csr.port_3.cfg_write_control.span_pg_enable: 0x" << pbc_csr.port_3.cfg_write_control.span_pg_enable() << endl;
data <<"pbc_csr.port_3.cfg_write_control.span_pg_index: 0x" << pbc_csr.port_3.cfg_write_control.span_pg_index() << endl;
data <<"pbc_csr.port_3.cfg_write_control.enforce_oq24_31: 0x" << pbc_csr.port_3.cfg_write_control.enforce_oq24_31() << endl;
data <<"pbc_csr.port_3.cfg_write_control.packing_timeout_cycles: 0x" << pbc_csr.port_3.cfg_write_control.packing_timeout_cycles() << endl;
data <<"pbc_csr.port_3.cfg_write_control.min_size: 0x" << pbc_csr.port_3.cfg_write_control.min_size() << endl;
data <<"pbc_csr.port_3.cfg_write_control.packing_same_pg: 0x" << pbc_csr.port_3.cfg_write_control.packing_same_pg() << endl;
data <<"pbc_csr.port_3.cfg_write_control.packing: 0x" << pbc_csr.port_3.cfg_write_control.packing() << endl;
data <<"pbc_csr.port_3.cfg_write_control.enqueue_mux_bypass: 0x" << pbc_csr.port_3.cfg_write_control.enqueue_mux_bypass() << endl;
data <<"pbc_csr.port_3.cfg_write_control.enable_drdy: 0x" << pbc_csr.port_3.cfg_write_control.enable_drdy() << endl;
data <<"pbc_csr.port_3.cfg_write_control.rate_limiter: 0x" << pbc_csr.port_3.cfg_write_control.rate_limiter() << endl;
data <<"pbc_csr.port_3.cfg_write_control.cut_thru: 0x" << pbc_csr.port_3.cfg_write_control.cut_thru() << endl;
data <<"pbc_csr.port_3.cfg_write_control.enable_mnic_multicast: 0x" << pbc_csr.port_3.cfg_write_control.enable_mnic_multicast() << endl;
data <<"pbc_csr.port_3.cfg_write_control.enable_bmc_multicast: 0x" << pbc_csr.port_3.cfg_write_control.enable_bmc_multicast() << endl;

    pbc_csr.port_4.cfg_account_clear_xoff.read();
    data <<"pbc_csr.port_4.cfg_account_clear_xoff.all: 0x" << pbc_csr.port_4.cfg_account_clear_xoff.all() << endl;
data <<"pbc_csr.port_4.cfg_account_clear_xoff.bitmap: 0x" << pbc_csr.port_4.cfg_account_clear_xoff.bitmap() << endl;

    pbc_csr.port_4.cfg_account_control.read();
    data <<"pbc_csr.port_4.cfg_account_control.all: 0x" << pbc_csr.port_4.cfg_account_control.all() << endl;
data <<"pbc_csr.port_4.cfg_account_control.sp_enable: 0x" << pbc_csr.port_4.cfg_account_control.sp_enable() << endl;
data <<"pbc_csr.port_4.cfg_account_control.clear_wm_index: 0x" << pbc_csr.port_4.cfg_account_control.clear_wm_index() << endl;
data <<"pbc_csr.port_4.cfg_account_control.clear_wm_enable: 0x" << pbc_csr.port_4.cfg_account_control.clear_wm_enable() << endl;
data <<"pbc_csr.port_4.cfg_account_control.use_sp_as_wm: 0x" << pbc_csr.port_4.cfg_account_control.use_sp_as_wm() << endl;

    pbc_csr.port_4.cfg_account_credit_return.read();
    data <<"pbc_csr.port_4.cfg_account_credit_return.all: 0x" << pbc_csr.port_4.cfg_account_credit_return.all() << endl;
data <<"pbc_csr.port_4.cfg_account_credit_return.enable: 0x" << pbc_csr.port_4.cfg_account_credit_return.enable() << endl;

    pbc_csr.port_4.cfg_account_mtu_table.read();
    data <<"pbc_csr.port_4.cfg_account_mtu_table.all: 0x" << pbc_csr.port_4.cfg_account_mtu_table.all() << endl;
data <<"pbc_csr.port_4.cfg_account_mtu_table.pg6: 0x" << pbc_csr.port_4.cfg_account_mtu_table.pg6() << endl;
data <<"pbc_csr.port_4.cfg_account_mtu_table.pg7: 0x" << pbc_csr.port_4.cfg_account_mtu_table.pg7() << endl;
data <<"pbc_csr.port_4.cfg_account_mtu_table.pg4: 0x" << pbc_csr.port_4.cfg_account_mtu_table.pg4() << endl;
data <<"pbc_csr.port_4.cfg_account_mtu_table.pg5: 0x" << pbc_csr.port_4.cfg_account_mtu_table.pg5() << endl;
data <<"pbc_csr.port_4.cfg_account_mtu_table.pg2: 0x" << pbc_csr.port_4.cfg_account_mtu_table.pg2() << endl;
data <<"pbc_csr.port_4.cfg_account_mtu_table.pg3: 0x" << pbc_csr.port_4.cfg_account_mtu_table.pg3() << endl;
data <<"pbc_csr.port_4.cfg_account_mtu_table.pg0: 0x" << pbc_csr.port_4.cfg_account_mtu_table.pg0() << endl;
data <<"pbc_csr.port_4.cfg_account_mtu_table.pg1: 0x" << pbc_csr.port_4.cfg_account_mtu_table.pg1() << endl;

    pbc_csr.port_4.cfg_account_pause_timer.read();
    data <<"pbc_csr.port_4.cfg_account_pause_timer.all: 0x" << pbc_csr.port_4.cfg_account_pause_timer.all() << endl;
data <<"pbc_csr.port_4.cfg_account_pause_timer.xoff_timeout_auto_clear: 0x" << pbc_csr.port_4.cfg_account_pause_timer.xoff_timeout_auto_clear() << endl;
data <<"pbc_csr.port_4.cfg_account_pause_timer.xoff_timeout_auto_xon: 0x" << pbc_csr.port_4.cfg_account_pause_timer.xoff_timeout_auto_xon() << endl;
data <<"pbc_csr.port_4.cfg_account_pause_timer.xoff_timeout: 0x" << pbc_csr.port_4.cfg_account_pause_timer.xoff_timeout() << endl;

    pbc_csr.port_4.cfg_account_pause_timer_enable.read();
    data <<"pbc_csr.port_4.cfg_account_pause_timer_enable.all: 0x" << pbc_csr.port_4.cfg_account_pause_timer_enable.all() << endl;
data <<"pbc_csr.port_4.cfg_account_pause_timer_enable.iq: 0x" << pbc_csr.port_4.cfg_account_pause_timer_enable.iq() << endl;

    pbc_csr.port_4.cfg_account_pg_0.read();
    data <<"pbc_csr.port_4.cfg_account_pg_0.all: 0x" << pbc_csr.port_4.cfg_account_pg_0.all() << endl;
data <<"pbc_csr.port_4.cfg_account_pg_0.alpha: 0x" << pbc_csr.port_4.cfg_account_pg_0.alpha() << endl;
data <<"pbc_csr.port_4.cfg_account_pg_0.xon_threshold: 0x" << pbc_csr.port_4.cfg_account_pg_0.xon_threshold() << endl;
data <<"pbc_csr.port_4.cfg_account_pg_0.headroom: 0x" << pbc_csr.port_4.cfg_account_pg_0.headroom() << endl;
data <<"pbc_csr.port_4.cfg_account_pg_0.reserved_min: 0x" << pbc_csr.port_4.cfg_account_pg_0.reserved_min() << endl;
data <<"pbc_csr.port_4.cfg_account_pg_0.low_limit: 0x" << pbc_csr.port_4.cfg_account_pg_0.low_limit() << endl;

    pbc_csr.port_4.cfg_account_pg_1.read();
    data <<"pbc_csr.port_4.cfg_account_pg_1.all: 0x" << pbc_csr.port_4.cfg_account_pg_1.all() << endl;
data <<"pbc_csr.port_4.cfg_account_pg_1.alpha: 0x" << pbc_csr.port_4.cfg_account_pg_1.alpha() << endl;
data <<"pbc_csr.port_4.cfg_account_pg_1.xon_threshold: 0x" << pbc_csr.port_4.cfg_account_pg_1.xon_threshold() << endl;
data <<"pbc_csr.port_4.cfg_account_pg_1.headroom: 0x" << pbc_csr.port_4.cfg_account_pg_1.headroom() << endl;
data <<"pbc_csr.port_4.cfg_account_pg_1.reserved_min: 0x" << pbc_csr.port_4.cfg_account_pg_1.reserved_min() << endl;
data <<"pbc_csr.port_4.cfg_account_pg_1.low_limit: 0x" << pbc_csr.port_4.cfg_account_pg_1.low_limit() << endl;

    pbc_csr.port_4.cfg_account_pg_2.read();
    data <<"pbc_csr.port_4.cfg_account_pg_2.all: 0x" << pbc_csr.port_4.cfg_account_pg_2.all() << endl;
data <<"pbc_csr.port_4.cfg_account_pg_2.alpha: 0x" << pbc_csr.port_4.cfg_account_pg_2.alpha() << endl;
data <<"pbc_csr.port_4.cfg_account_pg_2.xon_threshold: 0x" << pbc_csr.port_4.cfg_account_pg_2.xon_threshold() << endl;
data <<"pbc_csr.port_4.cfg_account_pg_2.headroom: 0x" << pbc_csr.port_4.cfg_account_pg_2.headroom() << endl;
data <<"pbc_csr.port_4.cfg_account_pg_2.reserved_min: 0x" << pbc_csr.port_4.cfg_account_pg_2.reserved_min() << endl;
data <<"pbc_csr.port_4.cfg_account_pg_2.low_limit: 0x" << pbc_csr.port_4.cfg_account_pg_2.low_limit() << endl;

    pbc_csr.port_4.cfg_account_pg_3.read();
    data <<"pbc_csr.port_4.cfg_account_pg_3.all: 0x" << pbc_csr.port_4.cfg_account_pg_3.all() << endl;
data <<"pbc_csr.port_4.cfg_account_pg_3.alpha: 0x" << pbc_csr.port_4.cfg_account_pg_3.alpha() << endl;
data <<"pbc_csr.port_4.cfg_account_pg_3.xon_threshold: 0x" << pbc_csr.port_4.cfg_account_pg_3.xon_threshold() << endl;
data <<"pbc_csr.port_4.cfg_account_pg_3.headroom: 0x" << pbc_csr.port_4.cfg_account_pg_3.headroom() << endl;
data <<"pbc_csr.port_4.cfg_account_pg_3.reserved_min: 0x" << pbc_csr.port_4.cfg_account_pg_3.reserved_min() << endl;
data <<"pbc_csr.port_4.cfg_account_pg_3.low_limit: 0x" << pbc_csr.port_4.cfg_account_pg_3.low_limit() << endl;

    pbc_csr.port_4.cfg_account_pg_4.read();
    data <<"pbc_csr.port_4.cfg_account_pg_4.all: 0x" << pbc_csr.port_4.cfg_account_pg_4.all() << endl;
data <<"pbc_csr.port_4.cfg_account_pg_4.alpha: 0x" << pbc_csr.port_4.cfg_account_pg_4.alpha() << endl;
data <<"pbc_csr.port_4.cfg_account_pg_4.xon_threshold: 0x" << pbc_csr.port_4.cfg_account_pg_4.xon_threshold() << endl;
data <<"pbc_csr.port_4.cfg_account_pg_4.headroom: 0x" << pbc_csr.port_4.cfg_account_pg_4.headroom() << endl;
data <<"pbc_csr.port_4.cfg_account_pg_4.reserved_min: 0x" << pbc_csr.port_4.cfg_account_pg_4.reserved_min() << endl;
data <<"pbc_csr.port_4.cfg_account_pg_4.low_limit: 0x" << pbc_csr.port_4.cfg_account_pg_4.low_limit() << endl;

    pbc_csr.port_4.cfg_account_pg_5.read();
    data <<"pbc_csr.port_4.cfg_account_pg_5.all: 0x" << pbc_csr.port_4.cfg_account_pg_5.all() << endl;
data <<"pbc_csr.port_4.cfg_account_pg_5.alpha: 0x" << pbc_csr.port_4.cfg_account_pg_5.alpha() << endl;
data <<"pbc_csr.port_4.cfg_account_pg_5.xon_threshold: 0x" << pbc_csr.port_4.cfg_account_pg_5.xon_threshold() << endl;
data <<"pbc_csr.port_4.cfg_account_pg_5.headroom: 0x" << pbc_csr.port_4.cfg_account_pg_5.headroom() << endl;
data <<"pbc_csr.port_4.cfg_account_pg_5.reserved_min: 0x" << pbc_csr.port_4.cfg_account_pg_5.reserved_min() << endl;
data <<"pbc_csr.port_4.cfg_account_pg_5.low_limit: 0x" << pbc_csr.port_4.cfg_account_pg_5.low_limit() << endl;

    pbc_csr.port_4.cfg_account_pg_6.read();
    data <<"pbc_csr.port_4.cfg_account_pg_6.all: 0x" << pbc_csr.port_4.cfg_account_pg_6.all() << endl;
data <<"pbc_csr.port_4.cfg_account_pg_6.alpha: 0x" << pbc_csr.port_4.cfg_account_pg_6.alpha() << endl;
data <<"pbc_csr.port_4.cfg_account_pg_6.xon_threshold: 0x" << pbc_csr.port_4.cfg_account_pg_6.xon_threshold() << endl;
data <<"pbc_csr.port_4.cfg_account_pg_6.headroom: 0x" << pbc_csr.port_4.cfg_account_pg_6.headroom() << endl;
data <<"pbc_csr.port_4.cfg_account_pg_6.reserved_min: 0x" << pbc_csr.port_4.cfg_account_pg_6.reserved_min() << endl;
data <<"pbc_csr.port_4.cfg_account_pg_6.low_limit: 0x" << pbc_csr.port_4.cfg_account_pg_6.low_limit() << endl;

    pbc_csr.port_4.cfg_account_pg_7.read();
    data <<"pbc_csr.port_4.cfg_account_pg_7.all: 0x" << pbc_csr.port_4.cfg_account_pg_7.all() << endl;
data <<"pbc_csr.port_4.cfg_account_pg_7.alpha: 0x" << pbc_csr.port_4.cfg_account_pg_7.alpha() << endl;
data <<"pbc_csr.port_4.cfg_account_pg_7.xon_threshold: 0x" << pbc_csr.port_4.cfg_account_pg_7.xon_threshold() << endl;
data <<"pbc_csr.port_4.cfg_account_pg_7.headroom: 0x" << pbc_csr.port_4.cfg_account_pg_7.headroom() << endl;
data <<"pbc_csr.port_4.cfg_account_pg_7.reserved_min: 0x" << pbc_csr.port_4.cfg_account_pg_7.reserved_min() << endl;
data <<"pbc_csr.port_4.cfg_account_pg_7.low_limit: 0x" << pbc_csr.port_4.cfg_account_pg_7.low_limit() << endl;

    pbc_csr.port_4.cfg_account_pg_to_sp.read();
    data <<"pbc_csr.port_4.cfg_account_pg_to_sp.all: 0x" << pbc_csr.port_4.cfg_account_pg_to_sp.all() << endl;
data <<"pbc_csr.port_4.cfg_account_pg_to_sp.map: 0x" << pbc_csr.port_4.cfg_account_pg_to_sp.map() << endl;

    pbc_csr.port_4.cfg_account_sp_fetch_quanta.read();
    data <<"pbc_csr.port_4.cfg_account_sp_fetch_quanta.all: 0x" << pbc_csr.port_4.cfg_account_sp_fetch_quanta.all() << endl;
data <<"pbc_csr.port_4.cfg_account_sp_fetch_quanta.pg6: 0x" << pbc_csr.port_4.cfg_account_sp_fetch_quanta.pg6() << endl;
data <<"pbc_csr.port_4.cfg_account_sp_fetch_quanta.pg7: 0x" << pbc_csr.port_4.cfg_account_sp_fetch_quanta.pg7() << endl;
data <<"pbc_csr.port_4.cfg_account_sp_fetch_quanta.pg4: 0x" << pbc_csr.port_4.cfg_account_sp_fetch_quanta.pg4() << endl;
data <<"pbc_csr.port_4.cfg_account_sp_fetch_quanta.pg5: 0x" << pbc_csr.port_4.cfg_account_sp_fetch_quanta.pg5() << endl;
data <<"pbc_csr.port_4.cfg_account_sp_fetch_quanta.pg2: 0x" << pbc_csr.port_4.cfg_account_sp_fetch_quanta.pg2() << endl;
data <<"pbc_csr.port_4.cfg_account_sp_fetch_quanta.pg3: 0x" << pbc_csr.port_4.cfg_account_sp_fetch_quanta.pg3() << endl;
data <<"pbc_csr.port_4.cfg_account_sp_fetch_quanta.pg0: 0x" << pbc_csr.port_4.cfg_account_sp_fetch_quanta.pg0() << endl;
data <<"pbc_csr.port_4.cfg_account_sp_fetch_quanta.pg1: 0x" << pbc_csr.port_4.cfg_account_sp_fetch_quanta.pg1() << endl;

    pbc_csr.port_4.cfg_account_sp_fetch_threshold.read();
    data <<"pbc_csr.port_4.cfg_account_sp_fetch_threshold.all: 0x" << pbc_csr.port_4.cfg_account_sp_fetch_threshold.all() << endl;
data <<"pbc_csr.port_4.cfg_account_sp_fetch_threshold.pg6: 0x" << pbc_csr.port_4.cfg_account_sp_fetch_threshold.pg6() << endl;
data <<"pbc_csr.port_4.cfg_account_sp_fetch_threshold.pg7: 0x" << pbc_csr.port_4.cfg_account_sp_fetch_threshold.pg7() << endl;
data <<"pbc_csr.port_4.cfg_account_sp_fetch_threshold.pg4: 0x" << pbc_csr.port_4.cfg_account_sp_fetch_threshold.pg4() << endl;
data <<"pbc_csr.port_4.cfg_account_sp_fetch_threshold.pg5: 0x" << pbc_csr.port_4.cfg_account_sp_fetch_threshold.pg5() << endl;
data <<"pbc_csr.port_4.cfg_account_sp_fetch_threshold.pg2: 0x" << pbc_csr.port_4.cfg_account_sp_fetch_threshold.pg2() << endl;
data <<"pbc_csr.port_4.cfg_account_sp_fetch_threshold.pg3: 0x" << pbc_csr.port_4.cfg_account_sp_fetch_threshold.pg3() << endl;
data <<"pbc_csr.port_4.cfg_account_sp_fetch_threshold.pg0: 0x" << pbc_csr.port_4.cfg_account_sp_fetch_threshold.pg0() << endl;
data <<"pbc_csr.port_4.cfg_account_sp_fetch_threshold.pg1: 0x" << pbc_csr.port_4.cfg_account_sp_fetch_threshold.pg1() << endl;

    pbc_csr.port_4.cfg_account_tc_to_pg.read();
    data <<"pbc_csr.port_4.cfg_account_tc_to_pg.all: 0x" << pbc_csr.port_4.cfg_account_tc_to_pg.all() << endl;
data <<"pbc_csr.port_4.cfg_account_tc_to_pg.table: 0x" << pbc_csr.port_4.cfg_account_tc_to_pg.table() << endl;

    pbc_csr.port_4.cfg_latency_cache.read();
    data <<"pbc_csr.port_4.cfg_latency_cache.all: 0x" << pbc_csr.port_4.cfg_latency_cache.all() << endl;
data <<"pbc_csr.port_4.cfg_latency_cache.enable: 0x" << pbc_csr.port_4.cfg_latency_cache.enable() << endl;

    pbc_csr.port_4.cfg_mac_xoff.read();
    data <<"pbc_csr.port_4.cfg_mac_xoff.all: 0x" << pbc_csr.port_4.cfg_mac_xoff.all() << endl;
data <<"pbc_csr.port_4.cfg_mac_xoff.enable: 0x" << pbc_csr.port_4.cfg_mac_xoff.enable() << endl;

    pbc_csr.port_4.cfg_oq.read();
    data <<"pbc_csr.port_4.cfg_oq.all: 0x" << pbc_csr.port_4.cfg_oq.all() << endl;
data <<"pbc_csr.port_4.cfg_oq.enable: 0x" << pbc_csr.port_4.cfg_oq.enable() << endl;
data <<"pbc_csr.port_4.cfg_oq.enable_bmc_multicast: 0x" << pbc_csr.port_4.cfg_oq.enable_bmc_multicast() << endl;
data <<"pbc_csr.port_4.cfg_oq.num_hdr_bytes: 0x" << pbc_csr.port_4.cfg_oq.num_hdr_bytes() << endl;
data <<"pbc_csr.port_4.cfg_oq.ecc_disable_cor: 0x" << pbc_csr.port_4.cfg_oq.ecc_disable_cor() << endl;
data <<"pbc_csr.port_4.cfg_oq.num_hdr_bytes_crypto: 0x" << pbc_csr.port_4.cfg_oq.num_hdr_bytes_crypto() << endl;
data <<"pbc_csr.port_4.cfg_oq.span_cpu_num_cells: 0x" << pbc_csr.port_4.cfg_oq.span_cpu_num_cells() << endl;
data <<"pbc_csr.port_4.cfg_oq.flush_hw_error: 0x" << pbc_csr.port_4.cfg_oq.flush_hw_error() << endl;
data <<"pbc_csr.port_4.cfg_oq.dhs_eccbypass: 0x" << pbc_csr.port_4.cfg_oq.dhs_eccbypass() << endl;
data <<"pbc_csr.port_4.cfg_oq.mux_bypass: 0x" << pbc_csr.port_4.cfg_oq.mux_bypass() << endl;
data <<"pbc_csr.port_4.cfg_oq.eg_ts_enable: 0x" << pbc_csr.port_4.cfg_oq.eg_ts_enable() << endl;
data <<"pbc_csr.port_4.cfg_oq.ipg_bytes: 0x" << pbc_csr.port_4.cfg_oq.ipg_bytes() << endl;
data <<"pbc_csr.port_4.cfg_oq.bist_run: 0x" << pbc_csr.port_4.cfg_oq.bist_run() << endl;
data <<"pbc_csr.port_4.cfg_oq.ecc_disable_det: 0x" << pbc_csr.port_4.cfg_oq.ecc_disable_det() << endl;
data <<"pbc_csr.port_4.cfg_oq.flush: 0x" << pbc_csr.port_4.cfg_oq.flush() << endl;
data <<"pbc_csr.port_4.cfg_oq.flow_control_enable_xoff: 0x" << pbc_csr.port_4.cfg_oq.flow_control_enable_xoff() << endl;
data <<"pbc_csr.port_4.cfg_oq.hw_error_to_pbus: 0x" << pbc_csr.port_4.cfg_oq.hw_error_to_pbus() << endl;
data <<"pbc_csr.port_4.cfg_oq.rewrite_enable: 0x" << pbc_csr.port_4.cfg_oq.rewrite_enable() << endl;
data <<"pbc_csr.port_4.cfg_oq.dhs_selection: 0x" << pbc_csr.port_4.cfg_oq.dhs_selection() << endl;
data <<"pbc_csr.port_4.cfg_oq.enable_mnic_multicast: 0x" << pbc_csr.port_4.cfg_oq.enable_mnic_multicast() << endl;

    pbc_csr.port_4.cfg_oq_arb_l1_selection.read();
    data <<"pbc_csr.port_4.cfg_oq_arb_l1_selection.all: 0x" << pbc_csr.port_4.cfg_oq_arb_l1_selection.all() << endl;
data <<"pbc_csr.port_4.cfg_oq_arb_l1_selection.node_13: 0x" << pbc_csr.port_4.cfg_oq_arb_l1_selection.node_13() << endl;
data <<"pbc_csr.port_4.cfg_oq_arb_l1_selection.node_14: 0x" << pbc_csr.port_4.cfg_oq_arb_l1_selection.node_14() << endl;
data <<"pbc_csr.port_4.cfg_oq_arb_l1_selection.node_9: 0x" << pbc_csr.port_4.cfg_oq_arb_l1_selection.node_9() << endl;
data <<"pbc_csr.port_4.cfg_oq_arb_l1_selection.node_8: 0x" << pbc_csr.port_4.cfg_oq_arb_l1_selection.node_8() << endl;
data <<"pbc_csr.port_4.cfg_oq_arb_l1_selection.node_7: 0x" << pbc_csr.port_4.cfg_oq_arb_l1_selection.node_7() << endl;
data <<"pbc_csr.port_4.cfg_oq_arb_l1_selection.node_6: 0x" << pbc_csr.port_4.cfg_oq_arb_l1_selection.node_6() << endl;
data <<"pbc_csr.port_4.cfg_oq_arb_l1_selection.node_5: 0x" << pbc_csr.port_4.cfg_oq_arb_l1_selection.node_5() << endl;
data <<"pbc_csr.port_4.cfg_oq_arb_l1_selection.node_4: 0x" << pbc_csr.port_4.cfg_oq_arb_l1_selection.node_4() << endl;
data <<"pbc_csr.port_4.cfg_oq_arb_l1_selection.node_3: 0x" << pbc_csr.port_4.cfg_oq_arb_l1_selection.node_3() << endl;
data <<"pbc_csr.port_4.cfg_oq_arb_l1_selection.node_2: 0x" << pbc_csr.port_4.cfg_oq_arb_l1_selection.node_2() << endl;
data <<"pbc_csr.port_4.cfg_oq_arb_l1_selection.node_1: 0x" << pbc_csr.port_4.cfg_oq_arb_l1_selection.node_1() << endl;
data <<"pbc_csr.port_4.cfg_oq_arb_l1_selection.node_0: 0x" << pbc_csr.port_4.cfg_oq_arb_l1_selection.node_0() << endl;
data <<"pbc_csr.port_4.cfg_oq_arb_l1_selection.node_11: 0x" << pbc_csr.port_4.cfg_oq_arb_l1_selection.node_11() << endl;
data <<"pbc_csr.port_4.cfg_oq_arb_l1_selection.node_10: 0x" << pbc_csr.port_4.cfg_oq_arb_l1_selection.node_10() << endl;
data <<"pbc_csr.port_4.cfg_oq_arb_l1_selection.node_15: 0x" << pbc_csr.port_4.cfg_oq_arb_l1_selection.node_15() << endl;
data <<"pbc_csr.port_4.cfg_oq_arb_l1_selection.node_12: 0x" << pbc_csr.port_4.cfg_oq_arb_l1_selection.node_12() << endl;

    pbc_csr.port_4.cfg_oq_arb_l1_strict.read();
    data <<"pbc_csr.port_4.cfg_oq_arb_l1_strict.all: 0x" << pbc_csr.port_4.cfg_oq_arb_l1_strict.all() << endl;
data <<"pbc_csr.port_4.cfg_oq_arb_l1_strict.priority: 0x" << pbc_csr.port_4.cfg_oq_arb_l1_strict.priority() << endl;

    pbc_csr.port_4.cfg_oq_arb_l2_selection.read();
    data <<"pbc_csr.port_4.cfg_oq_arb_l2_selection.all: 0x" << pbc_csr.port_4.cfg_oq_arb_l2_selection.all() << endl;
data <<"pbc_csr.port_4.cfg_oq_arb_l2_selection.node_3: 0x" << pbc_csr.port_4.cfg_oq_arb_l2_selection.node_3() << endl;
data <<"pbc_csr.port_4.cfg_oq_arb_l2_selection.node_2: 0x" << pbc_csr.port_4.cfg_oq_arb_l2_selection.node_2() << endl;
data <<"pbc_csr.port_4.cfg_oq_arb_l2_selection.node_1: 0x" << pbc_csr.port_4.cfg_oq_arb_l2_selection.node_1() << endl;
data <<"pbc_csr.port_4.cfg_oq_arb_l2_selection.node_0: 0x" << pbc_csr.port_4.cfg_oq_arb_l2_selection.node_0() << endl;

    pbc_csr.port_4.cfg_oq_arb_l2_strict.read();
    data <<"pbc_csr.port_4.cfg_oq_arb_l2_strict.all: 0x" << pbc_csr.port_4.cfg_oq_arb_l2_strict.all() << endl;
data <<"pbc_csr.port_4.cfg_oq_arb_l2_strict.priority: 0x" << pbc_csr.port_4.cfg_oq_arb_l2_strict.priority() << endl;

    pbc_csr.port_4.cfg_oq_dhs_mem.read();
    data <<"pbc_csr.port_4.cfg_oq_dhs_mem.all: 0x" << pbc_csr.port_4.cfg_oq_dhs_mem.all() << endl;
data <<"pbc_csr.port_4.cfg_oq_dhs_mem.address: 0x" << pbc_csr.port_4.cfg_oq_dhs_mem.address() << endl;

    pbc_csr.port_4.cfg_oq_queue.read();
    data <<"pbc_csr.port_4.cfg_oq_queue.all: 0x" << pbc_csr.port_4.cfg_oq_queue.all() << endl;
data <<"pbc_csr.port_4.cfg_oq_queue.enable: 0x" << pbc_csr.port_4.cfg_oq_queue.enable() << endl;
data <<"pbc_csr.port_4.cfg_oq_queue.flush: 0x" << pbc_csr.port_4.cfg_oq_queue.flush() << endl;

    pbc_csr.port_4.cfg_oq_queue_dynamic.read();
    data <<"pbc_csr.port_4.cfg_oq_queue_dynamic.all: 0x" << pbc_csr.port_4.cfg_oq_queue_dynamic.all() << endl;
data <<"pbc_csr.port_4.cfg_oq_queue_dynamic.threshold: 0x" << pbc_csr.port_4.cfg_oq_queue_dynamic.threshold() << endl;

    pbc_csr.port_4.cfg_oq_queue_write_one.read();
    data <<"pbc_csr.port_4.cfg_oq_queue_write_one.all: 0x" << pbc_csr.port_4.cfg_oq_queue_write_one.all() << endl;
data <<"pbc_csr.port_4.cfg_oq_queue_write_one.enable: 0x" << pbc_csr.port_4.cfg_oq_queue_write_one.enable() << endl;
data <<"pbc_csr.port_4.cfg_oq_queue_write_one.disable: 0x" << pbc_csr.port_4.cfg_oq_queue_write_one.disable() << endl;

    pbc_csr.port_4.cfg_oq_xoff2oq.read();
    data <<"pbc_csr.port_4.cfg_oq_xoff2oq.all: 0x" << pbc_csr.port_4.cfg_oq_xoff2oq.all() << endl;
data <<"pbc_csr.port_4.cfg_oq_xoff2oq.map: 0x" << pbc_csr.port_4.cfg_oq_xoff2oq.map() << endl;

    pbc_csr.port_4.cfg_tail_drop.read();
    data <<"pbc_csr.port_4.cfg_tail_drop.all: 0x" << pbc_csr.port_4.cfg_tail_drop.all() << endl;
data <<"pbc_csr.port_4.cfg_tail_drop.cpu_threshold: 0x" << pbc_csr.port_4.cfg_tail_drop.cpu_threshold() << endl;
data <<"pbc_csr.port_4.cfg_tail_drop.span_threshold: 0x" << pbc_csr.port_4.cfg_tail_drop.span_threshold() << endl;

    pbc_csr.port_4.cfg_write_control.read();
    data <<"pbc_csr.port_4.cfg_write_control.all: 0x" << pbc_csr.port_4.cfg_write_control.all() << endl;
data <<"pbc_csr.port_4.cfg_write_control.release_cells: 0x" << pbc_csr.port_4.cfg_write_control.release_cells() << endl;
data <<"pbc_csr.port_4.cfg_write_control.enable: 0x" << pbc_csr.port_4.cfg_write_control.enable() << endl;
data <<"pbc_csr.port_4.cfg_write_control.drop_on_error: 0x" << pbc_csr.port_4.cfg_write_control.drop_on_error() << endl;
data <<"pbc_csr.port_4.cfg_write_control.rewrite: 0x" << pbc_csr.port_4.cfg_write_control.rewrite() << endl;
data <<"pbc_csr.port_4.cfg_write_control.span_pg_enable: 0x" << pbc_csr.port_4.cfg_write_control.span_pg_enable() << endl;
data <<"pbc_csr.port_4.cfg_write_control.span_pg_index: 0x" << pbc_csr.port_4.cfg_write_control.span_pg_index() << endl;
data <<"pbc_csr.port_4.cfg_write_control.enforce_oq24_31: 0x" << pbc_csr.port_4.cfg_write_control.enforce_oq24_31() << endl;
data <<"pbc_csr.port_4.cfg_write_control.packing_timeout_cycles: 0x" << pbc_csr.port_4.cfg_write_control.packing_timeout_cycles() << endl;
data <<"pbc_csr.port_4.cfg_write_control.min_size: 0x" << pbc_csr.port_4.cfg_write_control.min_size() << endl;
data <<"pbc_csr.port_4.cfg_write_control.packing_same_pg: 0x" << pbc_csr.port_4.cfg_write_control.packing_same_pg() << endl;
data <<"pbc_csr.port_4.cfg_write_control.packing: 0x" << pbc_csr.port_4.cfg_write_control.packing() << endl;
data <<"pbc_csr.port_4.cfg_write_control.enqueue_mux_bypass: 0x" << pbc_csr.port_4.cfg_write_control.enqueue_mux_bypass() << endl;
data <<"pbc_csr.port_4.cfg_write_control.enable_drdy: 0x" << pbc_csr.port_4.cfg_write_control.enable_drdy() << endl;
data <<"pbc_csr.port_4.cfg_write_control.rate_limiter: 0x" << pbc_csr.port_4.cfg_write_control.rate_limiter() << endl;
data <<"pbc_csr.port_4.cfg_write_control.cut_thru: 0x" << pbc_csr.port_4.cfg_write_control.cut_thru() << endl;
data <<"pbc_csr.port_4.cfg_write_control.enable_mnic_multicast: 0x" << pbc_csr.port_4.cfg_write_control.enable_mnic_multicast() << endl;
data <<"pbc_csr.port_4.cfg_write_control.enable_bmc_multicast: 0x" << pbc_csr.port_4.cfg_write_control.enable_bmc_multicast() << endl;

    pbc_csr.port_5.cfg_account_clear_xoff.read();
    data <<"pbc_csr.port_5.cfg_account_clear_xoff.all: 0x" << pbc_csr.port_5.cfg_account_clear_xoff.all() << endl;
data <<"pbc_csr.port_5.cfg_account_clear_xoff.bitmap: 0x" << pbc_csr.port_5.cfg_account_clear_xoff.bitmap() << endl;

    pbc_csr.port_5.cfg_account_control.read();
    data <<"pbc_csr.port_5.cfg_account_control.all: 0x" << pbc_csr.port_5.cfg_account_control.all() << endl;
data <<"pbc_csr.port_5.cfg_account_control.sp_enable: 0x" << pbc_csr.port_5.cfg_account_control.sp_enable() << endl;
data <<"pbc_csr.port_5.cfg_account_control.clear_wm_index: 0x" << pbc_csr.port_5.cfg_account_control.clear_wm_index() << endl;
data <<"pbc_csr.port_5.cfg_account_control.clear_wm_enable: 0x" << pbc_csr.port_5.cfg_account_control.clear_wm_enable() << endl;
data <<"pbc_csr.port_5.cfg_account_control.use_sp_as_wm: 0x" << pbc_csr.port_5.cfg_account_control.use_sp_as_wm() << endl;

    pbc_csr.port_5.cfg_account_credit_return.read();
    data <<"pbc_csr.port_5.cfg_account_credit_return.all: 0x" << pbc_csr.port_5.cfg_account_credit_return.all() << endl;
data <<"pbc_csr.port_5.cfg_account_credit_return.enable: 0x" << pbc_csr.port_5.cfg_account_credit_return.enable() << endl;

    pbc_csr.port_5.cfg_account_mtu_table.read();
    data <<"pbc_csr.port_5.cfg_account_mtu_table.all: 0x" << pbc_csr.port_5.cfg_account_mtu_table.all() << endl;
data <<"pbc_csr.port_5.cfg_account_mtu_table.pg10: 0x" << pbc_csr.port_5.cfg_account_mtu_table.pg10() << endl;
data <<"pbc_csr.port_5.cfg_account_mtu_table.pg11: 0x" << pbc_csr.port_5.cfg_account_mtu_table.pg11() << endl;
data <<"pbc_csr.port_5.cfg_account_mtu_table.pg12: 0x" << pbc_csr.port_5.cfg_account_mtu_table.pg12() << endl;
data <<"pbc_csr.port_5.cfg_account_mtu_table.pg13: 0x" << pbc_csr.port_5.cfg_account_mtu_table.pg13() << endl;
data <<"pbc_csr.port_5.cfg_account_mtu_table.pg14: 0x" << pbc_csr.port_5.cfg_account_mtu_table.pg14() << endl;
data <<"pbc_csr.port_5.cfg_account_mtu_table.pg15: 0x" << pbc_csr.port_5.cfg_account_mtu_table.pg15() << endl;
data <<"pbc_csr.port_5.cfg_account_mtu_table.pg6: 0x" << pbc_csr.port_5.cfg_account_mtu_table.pg6() << endl;
data <<"pbc_csr.port_5.cfg_account_mtu_table.pg7: 0x" << pbc_csr.port_5.cfg_account_mtu_table.pg7() << endl;
data <<"pbc_csr.port_5.cfg_account_mtu_table.pg4: 0x" << pbc_csr.port_5.cfg_account_mtu_table.pg4() << endl;
data <<"pbc_csr.port_5.cfg_account_mtu_table.pg5: 0x" << pbc_csr.port_5.cfg_account_mtu_table.pg5() << endl;
data <<"pbc_csr.port_5.cfg_account_mtu_table.pg2: 0x" << pbc_csr.port_5.cfg_account_mtu_table.pg2() << endl;
data <<"pbc_csr.port_5.cfg_account_mtu_table.pg3: 0x" << pbc_csr.port_5.cfg_account_mtu_table.pg3() << endl;
data <<"pbc_csr.port_5.cfg_account_mtu_table.pg0: 0x" << pbc_csr.port_5.cfg_account_mtu_table.pg0() << endl;
data <<"pbc_csr.port_5.cfg_account_mtu_table.pg1: 0x" << pbc_csr.port_5.cfg_account_mtu_table.pg1() << endl;
data <<"pbc_csr.port_5.cfg_account_mtu_table.pg8: 0x" << pbc_csr.port_5.cfg_account_mtu_table.pg8() << endl;
data <<"pbc_csr.port_5.cfg_account_mtu_table.pg9: 0x" << pbc_csr.port_5.cfg_account_mtu_table.pg9() << endl;

    pbc_csr.port_5.cfg_account_pause_timer.read();
    data <<"pbc_csr.port_5.cfg_account_pause_timer.all: 0x" << pbc_csr.port_5.cfg_account_pause_timer.all() << endl;
data <<"pbc_csr.port_5.cfg_account_pause_timer.xoff_timeout_auto_clear: 0x" << pbc_csr.port_5.cfg_account_pause_timer.xoff_timeout_auto_clear() << endl;
data <<"pbc_csr.port_5.cfg_account_pause_timer.xoff_timeout_auto_xon: 0x" << pbc_csr.port_5.cfg_account_pause_timer.xoff_timeout_auto_xon() << endl;
data <<"pbc_csr.port_5.cfg_account_pause_timer.xoff_timeout: 0x" << pbc_csr.port_5.cfg_account_pause_timer.xoff_timeout() << endl;

    pbc_csr.port_5.cfg_account_pg_0.read();
    data <<"pbc_csr.port_5.cfg_account_pg_0.all: 0x" << pbc_csr.port_5.cfg_account_pg_0.all() << endl;
data <<"pbc_csr.port_5.cfg_account_pg_0.alpha: 0x" << pbc_csr.port_5.cfg_account_pg_0.alpha() << endl;
data <<"pbc_csr.port_5.cfg_account_pg_0.xon_threshold: 0x" << pbc_csr.port_5.cfg_account_pg_0.xon_threshold() << endl;
data <<"pbc_csr.port_5.cfg_account_pg_0.headroom: 0x" << pbc_csr.port_5.cfg_account_pg_0.headroom() << endl;
data <<"pbc_csr.port_5.cfg_account_pg_0.reserved_min: 0x" << pbc_csr.port_5.cfg_account_pg_0.reserved_min() << endl;
data <<"pbc_csr.port_5.cfg_account_pg_0.low_limit: 0x" << pbc_csr.port_5.cfg_account_pg_0.low_limit() << endl;

    pbc_csr.port_5.cfg_account_pg_1.read();
    data <<"pbc_csr.port_5.cfg_account_pg_1.all: 0x" << pbc_csr.port_5.cfg_account_pg_1.all() << endl;
data <<"pbc_csr.port_5.cfg_account_pg_1.alpha: 0x" << pbc_csr.port_5.cfg_account_pg_1.alpha() << endl;
data <<"pbc_csr.port_5.cfg_account_pg_1.xon_threshold: 0x" << pbc_csr.port_5.cfg_account_pg_1.xon_threshold() << endl;
data <<"pbc_csr.port_5.cfg_account_pg_1.headroom: 0x" << pbc_csr.port_5.cfg_account_pg_1.headroom() << endl;
data <<"pbc_csr.port_5.cfg_account_pg_1.reserved_min: 0x" << pbc_csr.port_5.cfg_account_pg_1.reserved_min() << endl;
data <<"pbc_csr.port_5.cfg_account_pg_1.low_limit: 0x" << pbc_csr.port_5.cfg_account_pg_1.low_limit() << endl;

    pbc_csr.port_5.cfg_account_pg_10.read();
    data <<"pbc_csr.port_5.cfg_account_pg_10.all: 0x" << pbc_csr.port_5.cfg_account_pg_10.all() << endl;
data <<"pbc_csr.port_5.cfg_account_pg_10.alpha: 0x" << pbc_csr.port_5.cfg_account_pg_10.alpha() << endl;
data <<"pbc_csr.port_5.cfg_account_pg_10.xon_threshold: 0x" << pbc_csr.port_5.cfg_account_pg_10.xon_threshold() << endl;
data <<"pbc_csr.port_5.cfg_account_pg_10.headroom: 0x" << pbc_csr.port_5.cfg_account_pg_10.headroom() << endl;
data <<"pbc_csr.port_5.cfg_account_pg_10.reserved_min: 0x" << pbc_csr.port_5.cfg_account_pg_10.reserved_min() << endl;
data <<"pbc_csr.port_5.cfg_account_pg_10.low_limit: 0x" << pbc_csr.port_5.cfg_account_pg_10.low_limit() << endl;

    pbc_csr.port_5.cfg_account_pg_11.read();
    data <<"pbc_csr.port_5.cfg_account_pg_11.all: 0x" << pbc_csr.port_5.cfg_account_pg_11.all() << endl;
data <<"pbc_csr.port_5.cfg_account_pg_11.alpha: 0x" << pbc_csr.port_5.cfg_account_pg_11.alpha() << endl;
data <<"pbc_csr.port_5.cfg_account_pg_11.xon_threshold: 0x" << pbc_csr.port_5.cfg_account_pg_11.xon_threshold() << endl;
data <<"pbc_csr.port_5.cfg_account_pg_11.headroom: 0x" << pbc_csr.port_5.cfg_account_pg_11.headroom() << endl;
data <<"pbc_csr.port_5.cfg_account_pg_11.reserved_min: 0x" << pbc_csr.port_5.cfg_account_pg_11.reserved_min() << endl;
data <<"pbc_csr.port_5.cfg_account_pg_11.low_limit: 0x" << pbc_csr.port_5.cfg_account_pg_11.low_limit() << endl;

    pbc_csr.port_5.cfg_account_pg_12.read();
    data <<"pbc_csr.port_5.cfg_account_pg_12.all: 0x" << pbc_csr.port_5.cfg_account_pg_12.all() << endl;
data <<"pbc_csr.port_5.cfg_account_pg_12.alpha: 0x" << pbc_csr.port_5.cfg_account_pg_12.alpha() << endl;
data <<"pbc_csr.port_5.cfg_account_pg_12.xon_threshold: 0x" << pbc_csr.port_5.cfg_account_pg_12.xon_threshold() << endl;
data <<"pbc_csr.port_5.cfg_account_pg_12.headroom: 0x" << pbc_csr.port_5.cfg_account_pg_12.headroom() << endl;
data <<"pbc_csr.port_5.cfg_account_pg_12.reserved_min: 0x" << pbc_csr.port_5.cfg_account_pg_12.reserved_min() << endl;
data <<"pbc_csr.port_5.cfg_account_pg_12.low_limit: 0x" << pbc_csr.port_5.cfg_account_pg_12.low_limit() << endl;

    pbc_csr.port_5.cfg_account_pg_13.read();
    data <<"pbc_csr.port_5.cfg_account_pg_13.all: 0x" << pbc_csr.port_5.cfg_account_pg_13.all() << endl;
data <<"pbc_csr.port_5.cfg_account_pg_13.alpha: 0x" << pbc_csr.port_5.cfg_account_pg_13.alpha() << endl;
data <<"pbc_csr.port_5.cfg_account_pg_13.xon_threshold: 0x" << pbc_csr.port_5.cfg_account_pg_13.xon_threshold() << endl;
data <<"pbc_csr.port_5.cfg_account_pg_13.headroom: 0x" << pbc_csr.port_5.cfg_account_pg_13.headroom() << endl;
data <<"pbc_csr.port_5.cfg_account_pg_13.reserved_min: 0x" << pbc_csr.port_5.cfg_account_pg_13.reserved_min() << endl;
data <<"pbc_csr.port_5.cfg_account_pg_13.low_limit: 0x" << pbc_csr.port_5.cfg_account_pg_13.low_limit() << endl;

    pbc_csr.port_5.cfg_account_pg_14.read();
    data <<"pbc_csr.port_5.cfg_account_pg_14.all: 0x" << pbc_csr.port_5.cfg_account_pg_14.all() << endl;
data <<"pbc_csr.port_5.cfg_account_pg_14.alpha: 0x" << pbc_csr.port_5.cfg_account_pg_14.alpha() << endl;
data <<"pbc_csr.port_5.cfg_account_pg_14.xon_threshold: 0x" << pbc_csr.port_5.cfg_account_pg_14.xon_threshold() << endl;
data <<"pbc_csr.port_5.cfg_account_pg_14.headroom: 0x" << pbc_csr.port_5.cfg_account_pg_14.headroom() << endl;
data <<"pbc_csr.port_5.cfg_account_pg_14.reserved_min: 0x" << pbc_csr.port_5.cfg_account_pg_14.reserved_min() << endl;
data <<"pbc_csr.port_5.cfg_account_pg_14.low_limit: 0x" << pbc_csr.port_5.cfg_account_pg_14.low_limit() << endl;

    pbc_csr.port_5.cfg_account_pg_15.read();
    data <<"pbc_csr.port_5.cfg_account_pg_15.all: 0x" << pbc_csr.port_5.cfg_account_pg_15.all() << endl;
data <<"pbc_csr.port_5.cfg_account_pg_15.alpha: 0x" << pbc_csr.port_5.cfg_account_pg_15.alpha() << endl;
data <<"pbc_csr.port_5.cfg_account_pg_15.xon_threshold: 0x" << pbc_csr.port_5.cfg_account_pg_15.xon_threshold() << endl;
data <<"pbc_csr.port_5.cfg_account_pg_15.headroom: 0x" << pbc_csr.port_5.cfg_account_pg_15.headroom() << endl;
data <<"pbc_csr.port_5.cfg_account_pg_15.reserved_min: 0x" << pbc_csr.port_5.cfg_account_pg_15.reserved_min() << endl;
data <<"pbc_csr.port_5.cfg_account_pg_15.low_limit: 0x" << pbc_csr.port_5.cfg_account_pg_15.low_limit() << endl;

    pbc_csr.port_5.cfg_account_pg_2.read();
    data <<"pbc_csr.port_5.cfg_account_pg_2.all: 0x" << pbc_csr.port_5.cfg_account_pg_2.all() << endl;
data <<"pbc_csr.port_5.cfg_account_pg_2.alpha: 0x" << pbc_csr.port_5.cfg_account_pg_2.alpha() << endl;
data <<"pbc_csr.port_5.cfg_account_pg_2.xon_threshold: 0x" << pbc_csr.port_5.cfg_account_pg_2.xon_threshold() << endl;
data <<"pbc_csr.port_5.cfg_account_pg_2.headroom: 0x" << pbc_csr.port_5.cfg_account_pg_2.headroom() << endl;
data <<"pbc_csr.port_5.cfg_account_pg_2.reserved_min: 0x" << pbc_csr.port_5.cfg_account_pg_2.reserved_min() << endl;
data <<"pbc_csr.port_5.cfg_account_pg_2.low_limit: 0x" << pbc_csr.port_5.cfg_account_pg_2.low_limit() << endl;

    pbc_csr.port_5.cfg_account_pg_3.read();
    data <<"pbc_csr.port_5.cfg_account_pg_3.all: 0x" << pbc_csr.port_5.cfg_account_pg_3.all() << endl;
data <<"pbc_csr.port_5.cfg_account_pg_3.alpha: 0x" << pbc_csr.port_5.cfg_account_pg_3.alpha() << endl;
data <<"pbc_csr.port_5.cfg_account_pg_3.xon_threshold: 0x" << pbc_csr.port_5.cfg_account_pg_3.xon_threshold() << endl;
data <<"pbc_csr.port_5.cfg_account_pg_3.headroom: 0x" << pbc_csr.port_5.cfg_account_pg_3.headroom() << endl;
data <<"pbc_csr.port_5.cfg_account_pg_3.reserved_min: 0x" << pbc_csr.port_5.cfg_account_pg_3.reserved_min() << endl;
data <<"pbc_csr.port_5.cfg_account_pg_3.low_limit: 0x" << pbc_csr.port_5.cfg_account_pg_3.low_limit() << endl;

    pbc_csr.port_5.cfg_account_pg_4.read();
    data <<"pbc_csr.port_5.cfg_account_pg_4.all: 0x" << pbc_csr.port_5.cfg_account_pg_4.all() << endl;
data <<"pbc_csr.port_5.cfg_account_pg_4.alpha: 0x" << pbc_csr.port_5.cfg_account_pg_4.alpha() << endl;
data <<"pbc_csr.port_5.cfg_account_pg_4.xon_threshold: 0x" << pbc_csr.port_5.cfg_account_pg_4.xon_threshold() << endl;
data <<"pbc_csr.port_5.cfg_account_pg_4.headroom: 0x" << pbc_csr.port_5.cfg_account_pg_4.headroom() << endl;
data <<"pbc_csr.port_5.cfg_account_pg_4.reserved_min: 0x" << pbc_csr.port_5.cfg_account_pg_4.reserved_min() << endl;
data <<"pbc_csr.port_5.cfg_account_pg_4.low_limit: 0x" << pbc_csr.port_5.cfg_account_pg_4.low_limit() << endl;

    pbc_csr.port_5.cfg_account_pg_5.read();
    data <<"pbc_csr.port_5.cfg_account_pg_5.all: 0x" << pbc_csr.port_5.cfg_account_pg_5.all() << endl;
data <<"pbc_csr.port_5.cfg_account_pg_5.alpha: 0x" << pbc_csr.port_5.cfg_account_pg_5.alpha() << endl;
data <<"pbc_csr.port_5.cfg_account_pg_5.xon_threshold: 0x" << pbc_csr.port_5.cfg_account_pg_5.xon_threshold() << endl;
data <<"pbc_csr.port_5.cfg_account_pg_5.headroom: 0x" << pbc_csr.port_5.cfg_account_pg_5.headroom() << endl;
data <<"pbc_csr.port_5.cfg_account_pg_5.reserved_min: 0x" << pbc_csr.port_5.cfg_account_pg_5.reserved_min() << endl;
data <<"pbc_csr.port_5.cfg_account_pg_5.low_limit: 0x" << pbc_csr.port_5.cfg_account_pg_5.low_limit() << endl;

    pbc_csr.port_5.cfg_account_pg_6.read();
    data <<"pbc_csr.port_5.cfg_account_pg_6.all: 0x" << pbc_csr.port_5.cfg_account_pg_6.all() << endl;
data <<"pbc_csr.port_5.cfg_account_pg_6.alpha: 0x" << pbc_csr.port_5.cfg_account_pg_6.alpha() << endl;
data <<"pbc_csr.port_5.cfg_account_pg_6.xon_threshold: 0x" << pbc_csr.port_5.cfg_account_pg_6.xon_threshold() << endl;
data <<"pbc_csr.port_5.cfg_account_pg_6.headroom: 0x" << pbc_csr.port_5.cfg_account_pg_6.headroom() << endl;
data <<"pbc_csr.port_5.cfg_account_pg_6.reserved_min: 0x" << pbc_csr.port_5.cfg_account_pg_6.reserved_min() << endl;
data <<"pbc_csr.port_5.cfg_account_pg_6.low_limit: 0x" << pbc_csr.port_5.cfg_account_pg_6.low_limit() << endl;

    pbc_csr.port_5.cfg_account_pg_7.read();
    data <<"pbc_csr.port_5.cfg_account_pg_7.all: 0x" << pbc_csr.port_5.cfg_account_pg_7.all() << endl;
data <<"pbc_csr.port_5.cfg_account_pg_7.alpha: 0x" << pbc_csr.port_5.cfg_account_pg_7.alpha() << endl;
data <<"pbc_csr.port_5.cfg_account_pg_7.xon_threshold: 0x" << pbc_csr.port_5.cfg_account_pg_7.xon_threshold() << endl;
data <<"pbc_csr.port_5.cfg_account_pg_7.headroom: 0x" << pbc_csr.port_5.cfg_account_pg_7.headroom() << endl;
data <<"pbc_csr.port_5.cfg_account_pg_7.reserved_min: 0x" << pbc_csr.port_5.cfg_account_pg_7.reserved_min() << endl;
data <<"pbc_csr.port_5.cfg_account_pg_7.low_limit: 0x" << pbc_csr.port_5.cfg_account_pg_7.low_limit() << endl;

    pbc_csr.port_5.cfg_account_pg_8.read();
    data <<"pbc_csr.port_5.cfg_account_pg_8.all: 0x" << pbc_csr.port_5.cfg_account_pg_8.all() << endl;
data <<"pbc_csr.port_5.cfg_account_pg_8.alpha: 0x" << pbc_csr.port_5.cfg_account_pg_8.alpha() << endl;
data <<"pbc_csr.port_5.cfg_account_pg_8.xon_threshold: 0x" << pbc_csr.port_5.cfg_account_pg_8.xon_threshold() << endl;
data <<"pbc_csr.port_5.cfg_account_pg_8.headroom: 0x" << pbc_csr.port_5.cfg_account_pg_8.headroom() << endl;
data <<"pbc_csr.port_5.cfg_account_pg_8.reserved_min: 0x" << pbc_csr.port_5.cfg_account_pg_8.reserved_min() << endl;
data <<"pbc_csr.port_5.cfg_account_pg_8.low_limit: 0x" << pbc_csr.port_5.cfg_account_pg_8.low_limit() << endl;

    pbc_csr.port_5.cfg_account_pg_9.read();
    data <<"pbc_csr.port_5.cfg_account_pg_9.all: 0x" << pbc_csr.port_5.cfg_account_pg_9.all() << endl;
data <<"pbc_csr.port_5.cfg_account_pg_9.alpha: 0x" << pbc_csr.port_5.cfg_account_pg_9.alpha() << endl;
data <<"pbc_csr.port_5.cfg_account_pg_9.xon_threshold: 0x" << pbc_csr.port_5.cfg_account_pg_9.xon_threshold() << endl;
data <<"pbc_csr.port_5.cfg_account_pg_9.headroom: 0x" << pbc_csr.port_5.cfg_account_pg_9.headroom() << endl;
data <<"pbc_csr.port_5.cfg_account_pg_9.reserved_min: 0x" << pbc_csr.port_5.cfg_account_pg_9.reserved_min() << endl;
data <<"pbc_csr.port_5.cfg_account_pg_9.low_limit: 0x" << pbc_csr.port_5.cfg_account_pg_9.low_limit() << endl;

    pbc_csr.port_5.cfg_account_pg_to_sp.read();
    data <<"pbc_csr.port_5.cfg_account_pg_to_sp.all: 0x" << pbc_csr.port_5.cfg_account_pg_to_sp.all() << endl;
data <<"pbc_csr.port_5.cfg_account_pg_to_sp.map: 0x" << pbc_csr.port_5.cfg_account_pg_to_sp.map() << endl;

    pbc_csr.port_5.cfg_account_sp_fetch_quanta.read();
    data <<"pbc_csr.port_5.cfg_account_sp_fetch_quanta.all: 0x" << pbc_csr.port_5.cfg_account_sp_fetch_quanta.all() << endl;
data <<"pbc_csr.port_5.cfg_account_sp_fetch_quanta.pg10: 0x" << pbc_csr.port_5.cfg_account_sp_fetch_quanta.pg10() << endl;
data <<"pbc_csr.port_5.cfg_account_sp_fetch_quanta.pg11: 0x" << pbc_csr.port_5.cfg_account_sp_fetch_quanta.pg11() << endl;
data <<"pbc_csr.port_5.cfg_account_sp_fetch_quanta.pg12: 0x" << pbc_csr.port_5.cfg_account_sp_fetch_quanta.pg12() << endl;
data <<"pbc_csr.port_5.cfg_account_sp_fetch_quanta.pg13: 0x" << pbc_csr.port_5.cfg_account_sp_fetch_quanta.pg13() << endl;
data <<"pbc_csr.port_5.cfg_account_sp_fetch_quanta.pg14: 0x" << pbc_csr.port_5.cfg_account_sp_fetch_quanta.pg14() << endl;
data <<"pbc_csr.port_5.cfg_account_sp_fetch_quanta.pg15: 0x" << pbc_csr.port_5.cfg_account_sp_fetch_quanta.pg15() << endl;
data <<"pbc_csr.port_5.cfg_account_sp_fetch_quanta.pg6: 0x" << pbc_csr.port_5.cfg_account_sp_fetch_quanta.pg6() << endl;
data <<"pbc_csr.port_5.cfg_account_sp_fetch_quanta.pg7: 0x" << pbc_csr.port_5.cfg_account_sp_fetch_quanta.pg7() << endl;
data <<"pbc_csr.port_5.cfg_account_sp_fetch_quanta.pg4: 0x" << pbc_csr.port_5.cfg_account_sp_fetch_quanta.pg4() << endl;
data <<"pbc_csr.port_5.cfg_account_sp_fetch_quanta.pg5: 0x" << pbc_csr.port_5.cfg_account_sp_fetch_quanta.pg5() << endl;
data <<"pbc_csr.port_5.cfg_account_sp_fetch_quanta.pg2: 0x" << pbc_csr.port_5.cfg_account_sp_fetch_quanta.pg2() << endl;
data <<"pbc_csr.port_5.cfg_account_sp_fetch_quanta.pg3: 0x" << pbc_csr.port_5.cfg_account_sp_fetch_quanta.pg3() << endl;
data <<"pbc_csr.port_5.cfg_account_sp_fetch_quanta.pg0: 0x" << pbc_csr.port_5.cfg_account_sp_fetch_quanta.pg0() << endl;
data <<"pbc_csr.port_5.cfg_account_sp_fetch_quanta.pg1: 0x" << pbc_csr.port_5.cfg_account_sp_fetch_quanta.pg1() << endl;
data <<"pbc_csr.port_5.cfg_account_sp_fetch_quanta.pg8: 0x" << pbc_csr.port_5.cfg_account_sp_fetch_quanta.pg8() << endl;
data <<"pbc_csr.port_5.cfg_account_sp_fetch_quanta.pg9: 0x" << pbc_csr.port_5.cfg_account_sp_fetch_quanta.pg9() << endl;

    pbc_csr.port_5.cfg_account_sp_fetch_threshold.read();
    data <<"pbc_csr.port_5.cfg_account_sp_fetch_threshold.all: 0x" << pbc_csr.port_5.cfg_account_sp_fetch_threshold.all() << endl;
data <<"pbc_csr.port_5.cfg_account_sp_fetch_threshold.pg10: 0x" << pbc_csr.port_5.cfg_account_sp_fetch_threshold.pg10() << endl;
data <<"pbc_csr.port_5.cfg_account_sp_fetch_threshold.pg11: 0x" << pbc_csr.port_5.cfg_account_sp_fetch_threshold.pg11() << endl;
data <<"pbc_csr.port_5.cfg_account_sp_fetch_threshold.pg12: 0x" << pbc_csr.port_5.cfg_account_sp_fetch_threshold.pg12() << endl;
data <<"pbc_csr.port_5.cfg_account_sp_fetch_threshold.pg13: 0x" << pbc_csr.port_5.cfg_account_sp_fetch_threshold.pg13() << endl;
data <<"pbc_csr.port_5.cfg_account_sp_fetch_threshold.pg14: 0x" << pbc_csr.port_5.cfg_account_sp_fetch_threshold.pg14() << endl;
data <<"pbc_csr.port_5.cfg_account_sp_fetch_threshold.pg15: 0x" << pbc_csr.port_5.cfg_account_sp_fetch_threshold.pg15() << endl;
data <<"pbc_csr.port_5.cfg_account_sp_fetch_threshold.pg6: 0x" << pbc_csr.port_5.cfg_account_sp_fetch_threshold.pg6() << endl;
data <<"pbc_csr.port_5.cfg_account_sp_fetch_threshold.pg7: 0x" << pbc_csr.port_5.cfg_account_sp_fetch_threshold.pg7() << endl;
data <<"pbc_csr.port_5.cfg_account_sp_fetch_threshold.pg4: 0x" << pbc_csr.port_5.cfg_account_sp_fetch_threshold.pg4() << endl;
data <<"pbc_csr.port_5.cfg_account_sp_fetch_threshold.pg5: 0x" << pbc_csr.port_5.cfg_account_sp_fetch_threshold.pg5() << endl;
data <<"pbc_csr.port_5.cfg_account_sp_fetch_threshold.pg2: 0x" << pbc_csr.port_5.cfg_account_sp_fetch_threshold.pg2() << endl;
data <<"pbc_csr.port_5.cfg_account_sp_fetch_threshold.pg3: 0x" << pbc_csr.port_5.cfg_account_sp_fetch_threshold.pg3() << endl;
data <<"pbc_csr.port_5.cfg_account_sp_fetch_threshold.pg0: 0x" << pbc_csr.port_5.cfg_account_sp_fetch_threshold.pg0() << endl;
data <<"pbc_csr.port_5.cfg_account_sp_fetch_threshold.pg1: 0x" << pbc_csr.port_5.cfg_account_sp_fetch_threshold.pg1() << endl;
data <<"pbc_csr.port_5.cfg_account_sp_fetch_threshold.pg8: 0x" << pbc_csr.port_5.cfg_account_sp_fetch_threshold.pg8() << endl;
data <<"pbc_csr.port_5.cfg_account_sp_fetch_threshold.pg9: 0x" << pbc_csr.port_5.cfg_account_sp_fetch_threshold.pg9() << endl;

    pbc_csr.port_5.cfg_account_tc_to_pg.read();
    data <<"pbc_csr.port_5.cfg_account_tc_to_pg.all: 0x" << pbc_csr.port_5.cfg_account_tc_to_pg.all() << endl;
data <<"pbc_csr.port_5.cfg_account_tc_to_pg.table: 0x" << pbc_csr.port_5.cfg_account_tc_to_pg.table() << endl;

    pbc_csr.port_5.cfg_latency_cache.read();
    data <<"pbc_csr.port_5.cfg_latency_cache.all: 0x" << pbc_csr.port_5.cfg_latency_cache.all() << endl;
data <<"pbc_csr.port_5.cfg_latency_cache.enable: 0x" << pbc_csr.port_5.cfg_latency_cache.enable() << endl;

    pbc_csr.port_5.cfg_oq.read();
    data <<"pbc_csr.port_5.cfg_oq.all: 0x" << pbc_csr.port_5.cfg_oq.all() << endl;
data <<"pbc_csr.port_5.cfg_oq.enable: 0x" << pbc_csr.port_5.cfg_oq.enable() << endl;
data <<"pbc_csr.port_5.cfg_oq.span_cpu_num_cells: 0x" << pbc_csr.port_5.cfg_oq.span_cpu_num_cells() << endl;
data <<"pbc_csr.port_5.cfg_oq.ipg_bytes: 0x" << pbc_csr.port_5.cfg_oq.ipg_bytes() << endl;
data <<"pbc_csr.port_5.cfg_oq.flow_control_enable_xoff: 0x" << pbc_csr.port_5.cfg_oq.flow_control_enable_xoff() << endl;
data <<"pbc_csr.port_5.cfg_oq.flush: 0x" << pbc_csr.port_5.cfg_oq.flush() << endl;
data <<"pbc_csr.port_5.cfg_oq.flow_control_enable_credits: 0x" << pbc_csr.port_5.cfg_oq.flow_control_enable_credits() << endl;
data <<"pbc_csr.port_5.cfg_oq.rewrite_enable: 0x" << pbc_csr.port_5.cfg_oq.rewrite_enable() << endl;

    pbc_csr.port_5.cfg_oq_arb_l1_selection.read();
    data <<"pbc_csr.port_5.cfg_oq_arb_l1_selection.all: 0x" << pbc_csr.port_5.cfg_oq_arb_l1_selection.all() << endl;
data <<"pbc_csr.port_5.cfg_oq_arb_l1_selection.node_13: 0x" << pbc_csr.port_5.cfg_oq_arb_l1_selection.node_13() << endl;
data <<"pbc_csr.port_5.cfg_oq_arb_l1_selection.node_14: 0x" << pbc_csr.port_5.cfg_oq_arb_l1_selection.node_14() << endl;
data <<"pbc_csr.port_5.cfg_oq_arb_l1_selection.node_9: 0x" << pbc_csr.port_5.cfg_oq_arb_l1_selection.node_9() << endl;
data <<"pbc_csr.port_5.cfg_oq_arb_l1_selection.node_8: 0x" << pbc_csr.port_5.cfg_oq_arb_l1_selection.node_8() << endl;
data <<"pbc_csr.port_5.cfg_oq_arb_l1_selection.node_7: 0x" << pbc_csr.port_5.cfg_oq_arb_l1_selection.node_7() << endl;
data <<"pbc_csr.port_5.cfg_oq_arb_l1_selection.node_6: 0x" << pbc_csr.port_5.cfg_oq_arb_l1_selection.node_6() << endl;
data <<"pbc_csr.port_5.cfg_oq_arb_l1_selection.node_5: 0x" << pbc_csr.port_5.cfg_oq_arb_l1_selection.node_5() << endl;
data <<"pbc_csr.port_5.cfg_oq_arb_l1_selection.node_4: 0x" << pbc_csr.port_5.cfg_oq_arb_l1_selection.node_4() << endl;
data <<"pbc_csr.port_5.cfg_oq_arb_l1_selection.node_3: 0x" << pbc_csr.port_5.cfg_oq_arb_l1_selection.node_3() << endl;
data <<"pbc_csr.port_5.cfg_oq_arb_l1_selection.node_2: 0x" << pbc_csr.port_5.cfg_oq_arb_l1_selection.node_2() << endl;
data <<"pbc_csr.port_5.cfg_oq_arb_l1_selection.node_1: 0x" << pbc_csr.port_5.cfg_oq_arb_l1_selection.node_1() << endl;
data <<"pbc_csr.port_5.cfg_oq_arb_l1_selection.node_0: 0x" << pbc_csr.port_5.cfg_oq_arb_l1_selection.node_0() << endl;
data <<"pbc_csr.port_5.cfg_oq_arb_l1_selection.node_11: 0x" << pbc_csr.port_5.cfg_oq_arb_l1_selection.node_11() << endl;
data <<"pbc_csr.port_5.cfg_oq_arb_l1_selection.node_10: 0x" << pbc_csr.port_5.cfg_oq_arb_l1_selection.node_10() << endl;
data <<"pbc_csr.port_5.cfg_oq_arb_l1_selection.node_15: 0x" << pbc_csr.port_5.cfg_oq_arb_l1_selection.node_15() << endl;
data <<"pbc_csr.port_5.cfg_oq_arb_l1_selection.node_12: 0x" << pbc_csr.port_5.cfg_oq_arb_l1_selection.node_12() << endl;

    pbc_csr.port_5.cfg_oq_arb_l1_strict.read();
    data <<"pbc_csr.port_5.cfg_oq_arb_l1_strict.all: 0x" << pbc_csr.port_5.cfg_oq_arb_l1_strict.all() << endl;
data <<"pbc_csr.port_5.cfg_oq_arb_l1_strict.priority: 0x" << pbc_csr.port_5.cfg_oq_arb_l1_strict.priority() << endl;

    pbc_csr.port_5.cfg_oq_arb_l2_selection.read();
    data <<"pbc_csr.port_5.cfg_oq_arb_l2_selection.all: 0x" << pbc_csr.port_5.cfg_oq_arb_l2_selection.all() << endl;
data <<"pbc_csr.port_5.cfg_oq_arb_l2_selection.node_3: 0x" << pbc_csr.port_5.cfg_oq_arb_l2_selection.node_3() << endl;
data <<"pbc_csr.port_5.cfg_oq_arb_l2_selection.node_2: 0x" << pbc_csr.port_5.cfg_oq_arb_l2_selection.node_2() << endl;
data <<"pbc_csr.port_5.cfg_oq_arb_l2_selection.node_1: 0x" << pbc_csr.port_5.cfg_oq_arb_l2_selection.node_1() << endl;
data <<"pbc_csr.port_5.cfg_oq_arb_l2_selection.node_0: 0x" << pbc_csr.port_5.cfg_oq_arb_l2_selection.node_0() << endl;

    pbc_csr.port_5.cfg_oq_arb_l2_strict.read();
    data <<"pbc_csr.port_5.cfg_oq_arb_l2_strict.all: 0x" << pbc_csr.port_5.cfg_oq_arb_l2_strict.all() << endl;
data <<"pbc_csr.port_5.cfg_oq_arb_l2_strict.priority: 0x" << pbc_csr.port_5.cfg_oq_arb_l2_strict.priority() << endl;

    pbc_csr.port_5.cfg_oq_arb_l3_selection.read();
    data <<"pbc_csr.port_5.cfg_oq_arb_l3_selection.all: 0x" << pbc_csr.port_5.cfg_oq_arb_l3_selection.all() << endl;
data <<"pbc_csr.port_5.cfg_oq_arb_l3_selection.node_0: 0x" << pbc_csr.port_5.cfg_oq_arb_l3_selection.node_0() << endl;

    pbc_csr.port_5.cfg_oq_arb_l3_strict.read();
    data <<"pbc_csr.port_5.cfg_oq_arb_l3_strict.all: 0x" << pbc_csr.port_5.cfg_oq_arb_l3_strict.all() << endl;
data <<"pbc_csr.port_5.cfg_oq_arb_l3_strict.priority: 0x" << pbc_csr.port_5.cfg_oq_arb_l3_strict.priority() << endl;

    pbc_csr.port_5.cfg_oq_dhs_mem.read();
    data <<"pbc_csr.port_5.cfg_oq_dhs_mem.all: 0x" << pbc_csr.port_5.cfg_oq_dhs_mem.all() << endl;
data <<"pbc_csr.port_5.cfg_oq_dhs_mem.address: 0x" << pbc_csr.port_5.cfg_oq_dhs_mem.address() << endl;

    pbc_csr.port_5.cfg_oq_queue.read();
    data <<"pbc_csr.port_5.cfg_oq_queue.all: 0x" << pbc_csr.port_5.cfg_oq_queue.all() << endl;
data <<"pbc_csr.port_5.cfg_oq_queue.enable: 0x" << pbc_csr.port_5.cfg_oq_queue.enable() << endl;
data <<"pbc_csr.port_5.cfg_oq_queue.flush: 0x" << pbc_csr.port_5.cfg_oq_queue.flush() << endl;

    pbc_csr.port_5.cfg_oq_queue_dynamic.read();
    data <<"pbc_csr.port_5.cfg_oq_queue_dynamic.all: 0x" << pbc_csr.port_5.cfg_oq_queue_dynamic.all() << endl;
data <<"pbc_csr.port_5.cfg_oq_queue_dynamic.threshold: 0x" << pbc_csr.port_5.cfg_oq_queue_dynamic.threshold() << endl;

    pbc_csr.port_5.cfg_oq_queue_write_one.read();
    data <<"pbc_csr.port_5.cfg_oq_queue_write_one.all: 0x" << pbc_csr.port_5.cfg_oq_queue_write_one.all() << endl;
data <<"pbc_csr.port_5.cfg_oq_queue_write_one.enable: 0x" << pbc_csr.port_5.cfg_oq_queue_write_one.enable() << endl;
data <<"pbc_csr.port_5.cfg_oq_queue_write_one.disable: 0x" << pbc_csr.port_5.cfg_oq_queue_write_one.disable() << endl;

    pbc_csr.port_5.cfg_oq_shared_pool.read();
    data <<"pbc_csr.port_5.cfg_oq_shared_pool.all: 0x" << pbc_csr.port_5.cfg_oq_shared_pool.all() << endl;
data <<"pbc_csr.port_5.cfg_oq_shared_pool.max_decrement: 0x" << pbc_csr.port_5.cfg_oq_shared_pool.max_decrement() << endl;
data <<"pbc_csr.port_5.cfg_oq_shared_pool.alpha_1: 0x" << pbc_csr.port_5.cfg_oq_shared_pool.alpha_1() << endl;
data <<"pbc_csr.port_5.cfg_oq_shared_pool.alpha_0: 0x" << pbc_csr.port_5.cfg_oq_shared_pool.alpha_0() << endl;
data <<"pbc_csr.port_5.cfg_oq_shared_pool.credits_0: 0x" << pbc_csr.port_5.cfg_oq_shared_pool.credits_0() << endl;
data <<"pbc_csr.port_5.cfg_oq_shared_pool.credits_1: 0x" << pbc_csr.port_5.cfg_oq_shared_pool.credits_1() << endl;

    pbc_csr.port_5.cfg_oq_xoff2oq.read();
    data <<"pbc_csr.port_5.cfg_oq_xoff2oq.all: 0x" << pbc_csr.port_5.cfg_oq_xoff2oq.all() << endl;
data <<"pbc_csr.port_5.cfg_oq_xoff2oq.map: 0x" << pbc_csr.port_5.cfg_oq_xoff2oq.map() << endl;

    pbc_csr.port_5.cfg_tail_drop.read();
    data <<"pbc_csr.port_5.cfg_tail_drop.all: 0x" << pbc_csr.port_5.cfg_tail_drop.all() << endl;
data <<"pbc_csr.port_5.cfg_tail_drop.cpu_threshold: 0x" << pbc_csr.port_5.cfg_tail_drop.cpu_threshold() << endl;
data <<"pbc_csr.port_5.cfg_tail_drop.span_threshold: 0x" << pbc_csr.port_5.cfg_tail_drop.span_threshold() << endl;

    pbc_csr.port_5.cfg_write_control.read();
    data <<"pbc_csr.port_5.cfg_write_control.all: 0x" << pbc_csr.port_5.cfg_write_control.all() << endl;
data <<"pbc_csr.port_5.cfg_write_control.release_cells: 0x" << pbc_csr.port_5.cfg_write_control.release_cells() << endl;
data <<"pbc_csr.port_5.cfg_write_control.rate_limiter_adj_time: 0x" << pbc_csr.port_5.cfg_write_control.rate_limiter_adj_time() << endl;
data <<"pbc_csr.port_5.cfg_write_control.enable: 0x" << pbc_csr.port_5.cfg_write_control.enable() << endl;
data <<"pbc_csr.port_5.cfg_write_control.enable_bmc_multicast: 0x" << pbc_csr.port_5.cfg_write_control.enable_bmc_multicast() << endl;
data <<"pbc_csr.port_5.cfg_write_control.rewrite: 0x" << pbc_csr.port_5.cfg_write_control.rewrite() << endl;
data <<"pbc_csr.port_5.cfg_write_control.span_pg_enable: 0x" << pbc_csr.port_5.cfg_write_control.span_pg_enable() << endl;
data <<"pbc_csr.port_5.cfg_write_control.span_pg_index: 0x" << pbc_csr.port_5.cfg_write_control.span_pg_index() << endl;
data <<"pbc_csr.port_5.cfg_write_control.enforce_oq24_31: 0x" << pbc_csr.port_5.cfg_write_control.enforce_oq24_31() << endl;
data <<"pbc_csr.port_5.cfg_write_control.drop_on_error: 0x" << pbc_csr.port_5.cfg_write_control.drop_on_error() << endl;
data <<"pbc_csr.port_5.cfg_write_control.min_size: 0x" << pbc_csr.port_5.cfg_write_control.min_size() << endl;
data <<"pbc_csr.port_5.cfg_write_control.rate_limiter_enable_force_emergency_stop: 0x" << pbc_csr.port_5.cfg_write_control.rate_limiter_enable_force_emergency_stop() << endl;
data <<"pbc_csr.port_5.cfg_write_control.cut_thru_select: 0x" << pbc_csr.port_5.cfg_write_control.cut_thru_select() << endl;
data <<"pbc_csr.port_5.cfg_write_control.enqueue_mux_bypass: 0x" << pbc_csr.port_5.cfg_write_control.enqueue_mux_bypass() << endl;
data <<"pbc_csr.port_5.cfg_write_control.use_intrinsic_credits: 0x" << pbc_csr.port_5.cfg_write_control.use_intrinsic_credits() << endl;
data <<"pbc_csr.port_5.cfg_write_control.enable_drdy: 0x" << pbc_csr.port_5.cfg_write_control.enable_drdy() << endl;
data <<"pbc_csr.port_5.cfg_write_control.rate_limiter_enable_emergency_stop: 0x" << pbc_csr.port_5.cfg_write_control.rate_limiter_enable_emergency_stop() << endl;
data <<"pbc_csr.port_5.cfg_write_control.rate_limiter: 0x" << pbc_csr.port_5.cfg_write_control.rate_limiter() << endl;
data <<"pbc_csr.port_5.cfg_write_control.cut_thru: 0x" << pbc_csr.port_5.cfg_write_control.cut_thru() << endl;
data <<"pbc_csr.port_5.cfg_write_control.enable_mnic_multicast: 0x" << pbc_csr.port_5.cfg_write_control.enable_mnic_multicast() << endl;

    pbc_csr.port_5.cfg_write_global.read();
    data <<"pbc_csr.port_5.cfg_write_global.all: 0x" << pbc_csr.port_5.cfg_write_global.all() << endl;
data <<"pbc_csr.port_5.cfg_write_global.occupancy_clear_threshold: 0x" << pbc_csr.port_5.cfg_write_global.occupancy_clear_threshold() << endl;
data <<"pbc_csr.port_5.cfg_write_global.occupancy_set_threshold: 0x" << pbc_csr.port_5.cfg_write_global.occupancy_set_threshold() << endl;

    pbc_csr.port_6.cfg_account_clear_xoff.read();
    data <<"pbc_csr.port_6.cfg_account_clear_xoff.all: 0x" << pbc_csr.port_6.cfg_account_clear_xoff.all() << endl;
data <<"pbc_csr.port_6.cfg_account_clear_xoff.bitmap: 0x" << pbc_csr.port_6.cfg_account_clear_xoff.bitmap() << endl;

    pbc_csr.port_6.cfg_account_control.read();
    data <<"pbc_csr.port_6.cfg_account_control.all: 0x" << pbc_csr.port_6.cfg_account_control.all() << endl;
data <<"pbc_csr.port_6.cfg_account_control.sp_enable: 0x" << pbc_csr.port_6.cfg_account_control.sp_enable() << endl;
data <<"pbc_csr.port_6.cfg_account_control.clear_wm_index: 0x" << pbc_csr.port_6.cfg_account_control.clear_wm_index() << endl;
data <<"pbc_csr.port_6.cfg_account_control.clear_wm_enable: 0x" << pbc_csr.port_6.cfg_account_control.clear_wm_enable() << endl;
data <<"pbc_csr.port_6.cfg_account_control.use_sp_as_wm: 0x" << pbc_csr.port_6.cfg_account_control.use_sp_as_wm() << endl;

    pbc_csr.port_6.cfg_account_credit_return.read();
    data <<"pbc_csr.port_6.cfg_account_credit_return.all: 0x" << pbc_csr.port_6.cfg_account_credit_return.all() << endl;
data <<"pbc_csr.port_6.cfg_account_credit_return.enable: 0x" << pbc_csr.port_6.cfg_account_credit_return.enable() << endl;

    pbc_csr.port_6.cfg_account_mtu_table.read();
    data <<"pbc_csr.port_6.cfg_account_mtu_table.all: 0x" << pbc_csr.port_6.cfg_account_mtu_table.all() << endl;
data <<"pbc_csr.port_6.cfg_account_mtu_table.pg30: 0x" << pbc_csr.port_6.cfg_account_mtu_table.pg30() << endl;
data <<"pbc_csr.port_6.cfg_account_mtu_table.pg31: 0x" << pbc_csr.port_6.cfg_account_mtu_table.pg31() << endl;
data <<"pbc_csr.port_6.cfg_account_mtu_table.pg18: 0x" << pbc_csr.port_6.cfg_account_mtu_table.pg18() << endl;
data <<"pbc_csr.port_6.cfg_account_mtu_table.pg19: 0x" << pbc_csr.port_6.cfg_account_mtu_table.pg19() << endl;
data <<"pbc_csr.port_6.cfg_account_mtu_table.pg10: 0x" << pbc_csr.port_6.cfg_account_mtu_table.pg10() << endl;
data <<"pbc_csr.port_6.cfg_account_mtu_table.pg11: 0x" << pbc_csr.port_6.cfg_account_mtu_table.pg11() << endl;
data <<"pbc_csr.port_6.cfg_account_mtu_table.pg12: 0x" << pbc_csr.port_6.cfg_account_mtu_table.pg12() << endl;
data <<"pbc_csr.port_6.cfg_account_mtu_table.pg13: 0x" << pbc_csr.port_6.cfg_account_mtu_table.pg13() << endl;
data <<"pbc_csr.port_6.cfg_account_mtu_table.pg14: 0x" << pbc_csr.port_6.cfg_account_mtu_table.pg14() << endl;
data <<"pbc_csr.port_6.cfg_account_mtu_table.pg15: 0x" << pbc_csr.port_6.cfg_account_mtu_table.pg15() << endl;
data <<"pbc_csr.port_6.cfg_account_mtu_table.pg16: 0x" << pbc_csr.port_6.cfg_account_mtu_table.pg16() << endl;
data <<"pbc_csr.port_6.cfg_account_mtu_table.pg17: 0x" << pbc_csr.port_6.cfg_account_mtu_table.pg17() << endl;
data <<"pbc_csr.port_6.cfg_account_mtu_table.pg6: 0x" << pbc_csr.port_6.cfg_account_mtu_table.pg6() << endl;
data <<"pbc_csr.port_6.cfg_account_mtu_table.pg7: 0x" << pbc_csr.port_6.cfg_account_mtu_table.pg7() << endl;
data <<"pbc_csr.port_6.cfg_account_mtu_table.pg4: 0x" << pbc_csr.port_6.cfg_account_mtu_table.pg4() << endl;
data <<"pbc_csr.port_6.cfg_account_mtu_table.pg5: 0x" << pbc_csr.port_6.cfg_account_mtu_table.pg5() << endl;
data <<"pbc_csr.port_6.cfg_account_mtu_table.pg2: 0x" << pbc_csr.port_6.cfg_account_mtu_table.pg2() << endl;
data <<"pbc_csr.port_6.cfg_account_mtu_table.pg3: 0x" << pbc_csr.port_6.cfg_account_mtu_table.pg3() << endl;
data <<"pbc_csr.port_6.cfg_account_mtu_table.pg0: 0x" << pbc_csr.port_6.cfg_account_mtu_table.pg0() << endl;
data <<"pbc_csr.port_6.cfg_account_mtu_table.pg1: 0x" << pbc_csr.port_6.cfg_account_mtu_table.pg1() << endl;
data <<"pbc_csr.port_6.cfg_account_mtu_table.pg8: 0x" << pbc_csr.port_6.cfg_account_mtu_table.pg8() << endl;
data <<"pbc_csr.port_6.cfg_account_mtu_table.pg9: 0x" << pbc_csr.port_6.cfg_account_mtu_table.pg9() << endl;
data <<"pbc_csr.port_6.cfg_account_mtu_table.pg21: 0x" << pbc_csr.port_6.cfg_account_mtu_table.pg21() << endl;
data <<"pbc_csr.port_6.cfg_account_mtu_table.pg20: 0x" << pbc_csr.port_6.cfg_account_mtu_table.pg20() << endl;
data <<"pbc_csr.port_6.cfg_account_mtu_table.pg23: 0x" << pbc_csr.port_6.cfg_account_mtu_table.pg23() << endl;
data <<"pbc_csr.port_6.cfg_account_mtu_table.pg22: 0x" << pbc_csr.port_6.cfg_account_mtu_table.pg22() << endl;
data <<"pbc_csr.port_6.cfg_account_mtu_table.pg25: 0x" << pbc_csr.port_6.cfg_account_mtu_table.pg25() << endl;
data <<"pbc_csr.port_6.cfg_account_mtu_table.pg24: 0x" << pbc_csr.port_6.cfg_account_mtu_table.pg24() << endl;
data <<"pbc_csr.port_6.cfg_account_mtu_table.pg27: 0x" << pbc_csr.port_6.cfg_account_mtu_table.pg27() << endl;
data <<"pbc_csr.port_6.cfg_account_mtu_table.pg26: 0x" << pbc_csr.port_6.cfg_account_mtu_table.pg26() << endl;
data <<"pbc_csr.port_6.cfg_account_mtu_table.pg29: 0x" << pbc_csr.port_6.cfg_account_mtu_table.pg29() << endl;
data <<"pbc_csr.port_6.cfg_account_mtu_table.pg28: 0x" << pbc_csr.port_6.cfg_account_mtu_table.pg28() << endl;

    pbc_csr.port_6.cfg_account_pause_timer.read();
    data <<"pbc_csr.port_6.cfg_account_pause_timer.all: 0x" << pbc_csr.port_6.cfg_account_pause_timer.all() << endl;
data <<"pbc_csr.port_6.cfg_account_pause_timer.xoff_timeout_auto_clear: 0x" << pbc_csr.port_6.cfg_account_pause_timer.xoff_timeout_auto_clear() << endl;
data <<"pbc_csr.port_6.cfg_account_pause_timer.xoff_timeout_auto_xon: 0x" << pbc_csr.port_6.cfg_account_pause_timer.xoff_timeout_auto_xon() << endl;
data <<"pbc_csr.port_6.cfg_account_pause_timer.xoff_timeout: 0x" << pbc_csr.port_6.cfg_account_pause_timer.xoff_timeout() << endl;

    pbc_csr.port_6.cfg_account_pg_0.read();
    data <<"pbc_csr.port_6.cfg_account_pg_0.all: 0x" << pbc_csr.port_6.cfg_account_pg_0.all() << endl;
data <<"pbc_csr.port_6.cfg_account_pg_0.alpha: 0x" << pbc_csr.port_6.cfg_account_pg_0.alpha() << endl;
data <<"pbc_csr.port_6.cfg_account_pg_0.xon_threshold: 0x" << pbc_csr.port_6.cfg_account_pg_0.xon_threshold() << endl;
data <<"pbc_csr.port_6.cfg_account_pg_0.headroom: 0x" << pbc_csr.port_6.cfg_account_pg_0.headroom() << endl;
data <<"pbc_csr.port_6.cfg_account_pg_0.reserved_min: 0x" << pbc_csr.port_6.cfg_account_pg_0.reserved_min() << endl;
data <<"pbc_csr.port_6.cfg_account_pg_0.low_limit: 0x" << pbc_csr.port_6.cfg_account_pg_0.low_limit() << endl;

    pbc_csr.port_6.cfg_account_pg_1.read();
    data <<"pbc_csr.port_6.cfg_account_pg_1.all: 0x" << pbc_csr.port_6.cfg_account_pg_1.all() << endl;
data <<"pbc_csr.port_6.cfg_account_pg_1.alpha: 0x" << pbc_csr.port_6.cfg_account_pg_1.alpha() << endl;
data <<"pbc_csr.port_6.cfg_account_pg_1.xon_threshold: 0x" << pbc_csr.port_6.cfg_account_pg_1.xon_threshold() << endl;
data <<"pbc_csr.port_6.cfg_account_pg_1.headroom: 0x" << pbc_csr.port_6.cfg_account_pg_1.headroom() << endl;
data <<"pbc_csr.port_6.cfg_account_pg_1.reserved_min: 0x" << pbc_csr.port_6.cfg_account_pg_1.reserved_min() << endl;
data <<"pbc_csr.port_6.cfg_account_pg_1.low_limit: 0x" << pbc_csr.port_6.cfg_account_pg_1.low_limit() << endl;

    pbc_csr.port_6.cfg_account_pg_10.read();
    data <<"pbc_csr.port_6.cfg_account_pg_10.all: 0x" << pbc_csr.port_6.cfg_account_pg_10.all() << endl;
data <<"pbc_csr.port_6.cfg_account_pg_10.alpha: 0x" << pbc_csr.port_6.cfg_account_pg_10.alpha() << endl;
data <<"pbc_csr.port_6.cfg_account_pg_10.xon_threshold: 0x" << pbc_csr.port_6.cfg_account_pg_10.xon_threshold() << endl;
data <<"pbc_csr.port_6.cfg_account_pg_10.headroom: 0x" << pbc_csr.port_6.cfg_account_pg_10.headroom() << endl;
data <<"pbc_csr.port_6.cfg_account_pg_10.reserved_min: 0x" << pbc_csr.port_6.cfg_account_pg_10.reserved_min() << endl;
data <<"pbc_csr.port_6.cfg_account_pg_10.low_limit: 0x" << pbc_csr.port_6.cfg_account_pg_10.low_limit() << endl;

    pbc_csr.port_6.cfg_account_pg_11.read();
    data <<"pbc_csr.port_6.cfg_account_pg_11.all: 0x" << pbc_csr.port_6.cfg_account_pg_11.all() << endl;
data <<"pbc_csr.port_6.cfg_account_pg_11.alpha: 0x" << pbc_csr.port_6.cfg_account_pg_11.alpha() << endl;
data <<"pbc_csr.port_6.cfg_account_pg_11.xon_threshold: 0x" << pbc_csr.port_6.cfg_account_pg_11.xon_threshold() << endl;
data <<"pbc_csr.port_6.cfg_account_pg_11.headroom: 0x" << pbc_csr.port_6.cfg_account_pg_11.headroom() << endl;
data <<"pbc_csr.port_6.cfg_account_pg_11.reserved_min: 0x" << pbc_csr.port_6.cfg_account_pg_11.reserved_min() << endl;
data <<"pbc_csr.port_6.cfg_account_pg_11.low_limit: 0x" << pbc_csr.port_6.cfg_account_pg_11.low_limit() << endl;

    pbc_csr.port_6.cfg_account_pg_12.read();
    data <<"pbc_csr.port_6.cfg_account_pg_12.all: 0x" << pbc_csr.port_6.cfg_account_pg_12.all() << endl;
data <<"pbc_csr.port_6.cfg_account_pg_12.alpha: 0x" << pbc_csr.port_6.cfg_account_pg_12.alpha() << endl;
data <<"pbc_csr.port_6.cfg_account_pg_12.xon_threshold: 0x" << pbc_csr.port_6.cfg_account_pg_12.xon_threshold() << endl;
data <<"pbc_csr.port_6.cfg_account_pg_12.headroom: 0x" << pbc_csr.port_6.cfg_account_pg_12.headroom() << endl;
data <<"pbc_csr.port_6.cfg_account_pg_12.reserved_min: 0x" << pbc_csr.port_6.cfg_account_pg_12.reserved_min() << endl;
data <<"pbc_csr.port_6.cfg_account_pg_12.low_limit: 0x" << pbc_csr.port_6.cfg_account_pg_12.low_limit() << endl;

    pbc_csr.port_6.cfg_account_pg_13.read();
    data <<"pbc_csr.port_6.cfg_account_pg_13.all: 0x" << pbc_csr.port_6.cfg_account_pg_13.all() << endl;
data <<"pbc_csr.port_6.cfg_account_pg_13.alpha: 0x" << pbc_csr.port_6.cfg_account_pg_13.alpha() << endl;
data <<"pbc_csr.port_6.cfg_account_pg_13.xon_threshold: 0x" << pbc_csr.port_6.cfg_account_pg_13.xon_threshold() << endl;
data <<"pbc_csr.port_6.cfg_account_pg_13.headroom: 0x" << pbc_csr.port_6.cfg_account_pg_13.headroom() << endl;
data <<"pbc_csr.port_6.cfg_account_pg_13.reserved_min: 0x" << pbc_csr.port_6.cfg_account_pg_13.reserved_min() << endl;
data <<"pbc_csr.port_6.cfg_account_pg_13.low_limit: 0x" << pbc_csr.port_6.cfg_account_pg_13.low_limit() << endl;

    pbc_csr.port_6.cfg_account_pg_14.read();
    data <<"pbc_csr.port_6.cfg_account_pg_14.all: 0x" << pbc_csr.port_6.cfg_account_pg_14.all() << endl;
data <<"pbc_csr.port_6.cfg_account_pg_14.alpha: 0x" << pbc_csr.port_6.cfg_account_pg_14.alpha() << endl;
data <<"pbc_csr.port_6.cfg_account_pg_14.xon_threshold: 0x" << pbc_csr.port_6.cfg_account_pg_14.xon_threshold() << endl;
data <<"pbc_csr.port_6.cfg_account_pg_14.headroom: 0x" << pbc_csr.port_6.cfg_account_pg_14.headroom() << endl;
data <<"pbc_csr.port_6.cfg_account_pg_14.reserved_min: 0x" << pbc_csr.port_6.cfg_account_pg_14.reserved_min() << endl;
data <<"pbc_csr.port_6.cfg_account_pg_14.low_limit: 0x" << pbc_csr.port_6.cfg_account_pg_14.low_limit() << endl;

    pbc_csr.port_6.cfg_account_pg_15.read();
    data <<"pbc_csr.port_6.cfg_account_pg_15.all: 0x" << pbc_csr.port_6.cfg_account_pg_15.all() << endl;
data <<"pbc_csr.port_6.cfg_account_pg_15.alpha: 0x" << pbc_csr.port_6.cfg_account_pg_15.alpha() << endl;
data <<"pbc_csr.port_6.cfg_account_pg_15.xon_threshold: 0x" << pbc_csr.port_6.cfg_account_pg_15.xon_threshold() << endl;
data <<"pbc_csr.port_6.cfg_account_pg_15.headroom: 0x" << pbc_csr.port_6.cfg_account_pg_15.headroom() << endl;
data <<"pbc_csr.port_6.cfg_account_pg_15.reserved_min: 0x" << pbc_csr.port_6.cfg_account_pg_15.reserved_min() << endl;
data <<"pbc_csr.port_6.cfg_account_pg_15.low_limit: 0x" << pbc_csr.port_6.cfg_account_pg_15.low_limit() << endl;

    pbc_csr.port_6.cfg_account_pg_16.read();
    data <<"pbc_csr.port_6.cfg_account_pg_16.all: 0x" << pbc_csr.port_6.cfg_account_pg_16.all() << endl;
data <<"pbc_csr.port_6.cfg_account_pg_16.alpha: 0x" << pbc_csr.port_6.cfg_account_pg_16.alpha() << endl;
data <<"pbc_csr.port_6.cfg_account_pg_16.xon_threshold: 0x" << pbc_csr.port_6.cfg_account_pg_16.xon_threshold() << endl;
data <<"pbc_csr.port_6.cfg_account_pg_16.headroom: 0x" << pbc_csr.port_6.cfg_account_pg_16.headroom() << endl;
data <<"pbc_csr.port_6.cfg_account_pg_16.reserved_min: 0x" << pbc_csr.port_6.cfg_account_pg_16.reserved_min() << endl;
data <<"pbc_csr.port_6.cfg_account_pg_16.low_limit: 0x" << pbc_csr.port_6.cfg_account_pg_16.low_limit() << endl;

    pbc_csr.port_6.cfg_account_pg_17.read();
    data <<"pbc_csr.port_6.cfg_account_pg_17.all: 0x" << pbc_csr.port_6.cfg_account_pg_17.all() << endl;
data <<"pbc_csr.port_6.cfg_account_pg_17.alpha: 0x" << pbc_csr.port_6.cfg_account_pg_17.alpha() << endl;
data <<"pbc_csr.port_6.cfg_account_pg_17.xon_threshold: 0x" << pbc_csr.port_6.cfg_account_pg_17.xon_threshold() << endl;
data <<"pbc_csr.port_6.cfg_account_pg_17.headroom: 0x" << pbc_csr.port_6.cfg_account_pg_17.headroom() << endl;
data <<"pbc_csr.port_6.cfg_account_pg_17.reserved_min: 0x" << pbc_csr.port_6.cfg_account_pg_17.reserved_min() << endl;
data <<"pbc_csr.port_6.cfg_account_pg_17.low_limit: 0x" << pbc_csr.port_6.cfg_account_pg_17.low_limit() << endl;

    pbc_csr.port_6.cfg_account_pg_18.read();
    data <<"pbc_csr.port_6.cfg_account_pg_18.all: 0x" << pbc_csr.port_6.cfg_account_pg_18.all() << endl;
data <<"pbc_csr.port_6.cfg_account_pg_18.alpha: 0x" << pbc_csr.port_6.cfg_account_pg_18.alpha() << endl;
data <<"pbc_csr.port_6.cfg_account_pg_18.xon_threshold: 0x" << pbc_csr.port_6.cfg_account_pg_18.xon_threshold() << endl;
data <<"pbc_csr.port_6.cfg_account_pg_18.headroom: 0x" << pbc_csr.port_6.cfg_account_pg_18.headroom() << endl;
data <<"pbc_csr.port_6.cfg_account_pg_18.reserved_min: 0x" << pbc_csr.port_6.cfg_account_pg_18.reserved_min() << endl;
data <<"pbc_csr.port_6.cfg_account_pg_18.low_limit: 0x" << pbc_csr.port_6.cfg_account_pg_18.low_limit() << endl;

    pbc_csr.port_6.cfg_account_pg_19.read();
    data <<"pbc_csr.port_6.cfg_account_pg_19.all: 0x" << pbc_csr.port_6.cfg_account_pg_19.all() << endl;
data <<"pbc_csr.port_6.cfg_account_pg_19.alpha: 0x" << pbc_csr.port_6.cfg_account_pg_19.alpha() << endl;
data <<"pbc_csr.port_6.cfg_account_pg_19.xon_threshold: 0x" << pbc_csr.port_6.cfg_account_pg_19.xon_threshold() << endl;
data <<"pbc_csr.port_6.cfg_account_pg_19.headroom: 0x" << pbc_csr.port_6.cfg_account_pg_19.headroom() << endl;
data <<"pbc_csr.port_6.cfg_account_pg_19.reserved_min: 0x" << pbc_csr.port_6.cfg_account_pg_19.reserved_min() << endl;
data <<"pbc_csr.port_6.cfg_account_pg_19.low_limit: 0x" << pbc_csr.port_6.cfg_account_pg_19.low_limit() << endl;

    pbc_csr.port_6.cfg_account_pg_2.read();
    data <<"pbc_csr.port_6.cfg_account_pg_2.all: 0x" << pbc_csr.port_6.cfg_account_pg_2.all() << endl;
data <<"pbc_csr.port_6.cfg_account_pg_2.alpha: 0x" << pbc_csr.port_6.cfg_account_pg_2.alpha() << endl;
data <<"pbc_csr.port_6.cfg_account_pg_2.xon_threshold: 0x" << pbc_csr.port_6.cfg_account_pg_2.xon_threshold() << endl;
data <<"pbc_csr.port_6.cfg_account_pg_2.headroom: 0x" << pbc_csr.port_6.cfg_account_pg_2.headroom() << endl;
data <<"pbc_csr.port_6.cfg_account_pg_2.reserved_min: 0x" << pbc_csr.port_6.cfg_account_pg_2.reserved_min() << endl;
data <<"pbc_csr.port_6.cfg_account_pg_2.low_limit: 0x" << pbc_csr.port_6.cfg_account_pg_2.low_limit() << endl;

    pbc_csr.port_6.cfg_account_pg_20.read();
    data <<"pbc_csr.port_6.cfg_account_pg_20.all: 0x" << pbc_csr.port_6.cfg_account_pg_20.all() << endl;
data <<"pbc_csr.port_6.cfg_account_pg_20.alpha: 0x" << pbc_csr.port_6.cfg_account_pg_20.alpha() << endl;
data <<"pbc_csr.port_6.cfg_account_pg_20.xon_threshold: 0x" << pbc_csr.port_6.cfg_account_pg_20.xon_threshold() << endl;
data <<"pbc_csr.port_6.cfg_account_pg_20.headroom: 0x" << pbc_csr.port_6.cfg_account_pg_20.headroom() << endl;
data <<"pbc_csr.port_6.cfg_account_pg_20.reserved_min: 0x" << pbc_csr.port_6.cfg_account_pg_20.reserved_min() << endl;
data <<"pbc_csr.port_6.cfg_account_pg_20.low_limit: 0x" << pbc_csr.port_6.cfg_account_pg_20.low_limit() << endl;

    pbc_csr.port_6.cfg_account_pg_21.read();
    data <<"pbc_csr.port_6.cfg_account_pg_21.all: 0x" << pbc_csr.port_6.cfg_account_pg_21.all() << endl;
data <<"pbc_csr.port_6.cfg_account_pg_21.alpha: 0x" << pbc_csr.port_6.cfg_account_pg_21.alpha() << endl;
data <<"pbc_csr.port_6.cfg_account_pg_21.xon_threshold: 0x" << pbc_csr.port_6.cfg_account_pg_21.xon_threshold() << endl;
data <<"pbc_csr.port_6.cfg_account_pg_21.headroom: 0x" << pbc_csr.port_6.cfg_account_pg_21.headroom() << endl;
data <<"pbc_csr.port_6.cfg_account_pg_21.reserved_min: 0x" << pbc_csr.port_6.cfg_account_pg_21.reserved_min() << endl;
data <<"pbc_csr.port_6.cfg_account_pg_21.low_limit: 0x" << pbc_csr.port_6.cfg_account_pg_21.low_limit() << endl;

    pbc_csr.port_6.cfg_account_pg_22.read();
    data <<"pbc_csr.port_6.cfg_account_pg_22.all: 0x" << pbc_csr.port_6.cfg_account_pg_22.all() << endl;
data <<"pbc_csr.port_6.cfg_account_pg_22.alpha: 0x" << pbc_csr.port_6.cfg_account_pg_22.alpha() << endl;
data <<"pbc_csr.port_6.cfg_account_pg_22.xon_threshold: 0x" << pbc_csr.port_6.cfg_account_pg_22.xon_threshold() << endl;
data <<"pbc_csr.port_6.cfg_account_pg_22.headroom: 0x" << pbc_csr.port_6.cfg_account_pg_22.headroom() << endl;
data <<"pbc_csr.port_6.cfg_account_pg_22.reserved_min: 0x" << pbc_csr.port_6.cfg_account_pg_22.reserved_min() << endl;
data <<"pbc_csr.port_6.cfg_account_pg_22.low_limit: 0x" << pbc_csr.port_6.cfg_account_pg_22.low_limit() << endl;

    pbc_csr.port_6.cfg_account_pg_23.read();
    data <<"pbc_csr.port_6.cfg_account_pg_23.all: 0x" << pbc_csr.port_6.cfg_account_pg_23.all() << endl;
data <<"pbc_csr.port_6.cfg_account_pg_23.alpha: 0x" << pbc_csr.port_6.cfg_account_pg_23.alpha() << endl;
data <<"pbc_csr.port_6.cfg_account_pg_23.xon_threshold: 0x" << pbc_csr.port_6.cfg_account_pg_23.xon_threshold() << endl;
data <<"pbc_csr.port_6.cfg_account_pg_23.headroom: 0x" << pbc_csr.port_6.cfg_account_pg_23.headroom() << endl;
data <<"pbc_csr.port_6.cfg_account_pg_23.reserved_min: 0x" << pbc_csr.port_6.cfg_account_pg_23.reserved_min() << endl;
data <<"pbc_csr.port_6.cfg_account_pg_23.low_limit: 0x" << pbc_csr.port_6.cfg_account_pg_23.low_limit() << endl;

    pbc_csr.port_6.cfg_account_pg_24.read();
    data <<"pbc_csr.port_6.cfg_account_pg_24.all: 0x" << pbc_csr.port_6.cfg_account_pg_24.all() << endl;
data <<"pbc_csr.port_6.cfg_account_pg_24.alpha: 0x" << pbc_csr.port_6.cfg_account_pg_24.alpha() << endl;
data <<"pbc_csr.port_6.cfg_account_pg_24.xon_threshold: 0x" << pbc_csr.port_6.cfg_account_pg_24.xon_threshold() << endl;
data <<"pbc_csr.port_6.cfg_account_pg_24.headroom: 0x" << pbc_csr.port_6.cfg_account_pg_24.headroom() << endl;
data <<"pbc_csr.port_6.cfg_account_pg_24.reserved_min: 0x" << pbc_csr.port_6.cfg_account_pg_24.reserved_min() << endl;
data <<"pbc_csr.port_6.cfg_account_pg_24.low_limit: 0x" << pbc_csr.port_6.cfg_account_pg_24.low_limit() << endl;

    pbc_csr.port_6.cfg_account_pg_25.read();
    data <<"pbc_csr.port_6.cfg_account_pg_25.all: 0x" << pbc_csr.port_6.cfg_account_pg_25.all() << endl;
data <<"pbc_csr.port_6.cfg_account_pg_25.alpha: 0x" << pbc_csr.port_6.cfg_account_pg_25.alpha() << endl;
data <<"pbc_csr.port_6.cfg_account_pg_25.xon_threshold: 0x" << pbc_csr.port_6.cfg_account_pg_25.xon_threshold() << endl;
data <<"pbc_csr.port_6.cfg_account_pg_25.headroom: 0x" << pbc_csr.port_6.cfg_account_pg_25.headroom() << endl;
data <<"pbc_csr.port_6.cfg_account_pg_25.reserved_min: 0x" << pbc_csr.port_6.cfg_account_pg_25.reserved_min() << endl;
data <<"pbc_csr.port_6.cfg_account_pg_25.low_limit: 0x" << pbc_csr.port_6.cfg_account_pg_25.low_limit() << endl;

    pbc_csr.port_6.cfg_account_pg_26.read();
    data <<"pbc_csr.port_6.cfg_account_pg_26.all: 0x" << pbc_csr.port_6.cfg_account_pg_26.all() << endl;
data <<"pbc_csr.port_6.cfg_account_pg_26.alpha: 0x" << pbc_csr.port_6.cfg_account_pg_26.alpha() << endl;
data <<"pbc_csr.port_6.cfg_account_pg_26.xon_threshold: 0x" << pbc_csr.port_6.cfg_account_pg_26.xon_threshold() << endl;
data <<"pbc_csr.port_6.cfg_account_pg_26.headroom: 0x" << pbc_csr.port_6.cfg_account_pg_26.headroom() << endl;
data <<"pbc_csr.port_6.cfg_account_pg_26.reserved_min: 0x" << pbc_csr.port_6.cfg_account_pg_26.reserved_min() << endl;
data <<"pbc_csr.port_6.cfg_account_pg_26.low_limit: 0x" << pbc_csr.port_6.cfg_account_pg_26.low_limit() << endl;

    pbc_csr.port_6.cfg_account_pg_27.read();
    data <<"pbc_csr.port_6.cfg_account_pg_27.all: 0x" << pbc_csr.port_6.cfg_account_pg_27.all() << endl;
data <<"pbc_csr.port_6.cfg_account_pg_27.alpha: 0x" << pbc_csr.port_6.cfg_account_pg_27.alpha() << endl;
data <<"pbc_csr.port_6.cfg_account_pg_27.xon_threshold: 0x" << pbc_csr.port_6.cfg_account_pg_27.xon_threshold() << endl;
data <<"pbc_csr.port_6.cfg_account_pg_27.headroom: 0x" << pbc_csr.port_6.cfg_account_pg_27.headroom() << endl;
data <<"pbc_csr.port_6.cfg_account_pg_27.reserved_min: 0x" << pbc_csr.port_6.cfg_account_pg_27.reserved_min() << endl;
data <<"pbc_csr.port_6.cfg_account_pg_27.low_limit: 0x" << pbc_csr.port_6.cfg_account_pg_27.low_limit() << endl;

    pbc_csr.port_6.cfg_account_pg_28.read();
    data <<"pbc_csr.port_6.cfg_account_pg_28.all: 0x" << pbc_csr.port_6.cfg_account_pg_28.all() << endl;
data <<"pbc_csr.port_6.cfg_account_pg_28.alpha: 0x" << pbc_csr.port_6.cfg_account_pg_28.alpha() << endl;
data <<"pbc_csr.port_6.cfg_account_pg_28.xon_threshold: 0x" << pbc_csr.port_6.cfg_account_pg_28.xon_threshold() << endl;
data <<"pbc_csr.port_6.cfg_account_pg_28.headroom: 0x" << pbc_csr.port_6.cfg_account_pg_28.headroom() << endl;
data <<"pbc_csr.port_6.cfg_account_pg_28.reserved_min: 0x" << pbc_csr.port_6.cfg_account_pg_28.reserved_min() << endl;
data <<"pbc_csr.port_6.cfg_account_pg_28.low_limit: 0x" << pbc_csr.port_6.cfg_account_pg_28.low_limit() << endl;

    pbc_csr.port_6.cfg_account_pg_29.read();
    data <<"pbc_csr.port_6.cfg_account_pg_29.all: 0x" << pbc_csr.port_6.cfg_account_pg_29.all() << endl;
data <<"pbc_csr.port_6.cfg_account_pg_29.alpha: 0x" << pbc_csr.port_6.cfg_account_pg_29.alpha() << endl;
data <<"pbc_csr.port_6.cfg_account_pg_29.xon_threshold: 0x" << pbc_csr.port_6.cfg_account_pg_29.xon_threshold() << endl;
data <<"pbc_csr.port_6.cfg_account_pg_29.headroom: 0x" << pbc_csr.port_6.cfg_account_pg_29.headroom() << endl;
data <<"pbc_csr.port_6.cfg_account_pg_29.reserved_min: 0x" << pbc_csr.port_6.cfg_account_pg_29.reserved_min() << endl;
data <<"pbc_csr.port_6.cfg_account_pg_29.low_limit: 0x" << pbc_csr.port_6.cfg_account_pg_29.low_limit() << endl;

    pbc_csr.port_6.cfg_account_pg_3.read();
    data <<"pbc_csr.port_6.cfg_account_pg_3.all: 0x" << pbc_csr.port_6.cfg_account_pg_3.all() << endl;
data <<"pbc_csr.port_6.cfg_account_pg_3.alpha: 0x" << pbc_csr.port_6.cfg_account_pg_3.alpha() << endl;
data <<"pbc_csr.port_6.cfg_account_pg_3.xon_threshold: 0x" << pbc_csr.port_6.cfg_account_pg_3.xon_threshold() << endl;
data <<"pbc_csr.port_6.cfg_account_pg_3.headroom: 0x" << pbc_csr.port_6.cfg_account_pg_3.headroom() << endl;
data <<"pbc_csr.port_6.cfg_account_pg_3.reserved_min: 0x" << pbc_csr.port_6.cfg_account_pg_3.reserved_min() << endl;
data <<"pbc_csr.port_6.cfg_account_pg_3.low_limit: 0x" << pbc_csr.port_6.cfg_account_pg_3.low_limit() << endl;

    pbc_csr.port_6.cfg_account_pg_30.read();
    data <<"pbc_csr.port_6.cfg_account_pg_30.all: 0x" << pbc_csr.port_6.cfg_account_pg_30.all() << endl;
data <<"pbc_csr.port_6.cfg_account_pg_30.alpha: 0x" << pbc_csr.port_6.cfg_account_pg_30.alpha() << endl;
data <<"pbc_csr.port_6.cfg_account_pg_30.xon_threshold: 0x" << pbc_csr.port_6.cfg_account_pg_30.xon_threshold() << endl;
data <<"pbc_csr.port_6.cfg_account_pg_30.headroom: 0x" << pbc_csr.port_6.cfg_account_pg_30.headroom() << endl;
data <<"pbc_csr.port_6.cfg_account_pg_30.reserved_min: 0x" << pbc_csr.port_6.cfg_account_pg_30.reserved_min() << endl;
data <<"pbc_csr.port_6.cfg_account_pg_30.low_limit: 0x" << pbc_csr.port_6.cfg_account_pg_30.low_limit() << endl;

    pbc_csr.port_6.cfg_account_pg_31.read();
    data <<"pbc_csr.port_6.cfg_account_pg_31.all: 0x" << pbc_csr.port_6.cfg_account_pg_31.all() << endl;
data <<"pbc_csr.port_6.cfg_account_pg_31.alpha: 0x" << pbc_csr.port_6.cfg_account_pg_31.alpha() << endl;
data <<"pbc_csr.port_6.cfg_account_pg_31.xon_threshold: 0x" << pbc_csr.port_6.cfg_account_pg_31.xon_threshold() << endl;
data <<"pbc_csr.port_6.cfg_account_pg_31.headroom: 0x" << pbc_csr.port_6.cfg_account_pg_31.headroom() << endl;
data <<"pbc_csr.port_6.cfg_account_pg_31.reserved_min: 0x" << pbc_csr.port_6.cfg_account_pg_31.reserved_min() << endl;
data <<"pbc_csr.port_6.cfg_account_pg_31.low_limit: 0x" << pbc_csr.port_6.cfg_account_pg_31.low_limit() << endl;

    pbc_csr.port_6.cfg_account_pg_4.read();
    data <<"pbc_csr.port_6.cfg_account_pg_4.all: 0x" << pbc_csr.port_6.cfg_account_pg_4.all() << endl;
data <<"pbc_csr.port_6.cfg_account_pg_4.alpha: 0x" << pbc_csr.port_6.cfg_account_pg_4.alpha() << endl;
data <<"pbc_csr.port_6.cfg_account_pg_4.xon_threshold: 0x" << pbc_csr.port_6.cfg_account_pg_4.xon_threshold() << endl;
data <<"pbc_csr.port_6.cfg_account_pg_4.headroom: 0x" << pbc_csr.port_6.cfg_account_pg_4.headroom() << endl;
data <<"pbc_csr.port_6.cfg_account_pg_4.reserved_min: 0x" << pbc_csr.port_6.cfg_account_pg_4.reserved_min() << endl;
data <<"pbc_csr.port_6.cfg_account_pg_4.low_limit: 0x" << pbc_csr.port_6.cfg_account_pg_4.low_limit() << endl;

    pbc_csr.port_6.cfg_account_pg_5.read();
    data <<"pbc_csr.port_6.cfg_account_pg_5.all: 0x" << pbc_csr.port_6.cfg_account_pg_5.all() << endl;
data <<"pbc_csr.port_6.cfg_account_pg_5.alpha: 0x" << pbc_csr.port_6.cfg_account_pg_5.alpha() << endl;
data <<"pbc_csr.port_6.cfg_account_pg_5.xon_threshold: 0x" << pbc_csr.port_6.cfg_account_pg_5.xon_threshold() << endl;
data <<"pbc_csr.port_6.cfg_account_pg_5.headroom: 0x" << pbc_csr.port_6.cfg_account_pg_5.headroom() << endl;
data <<"pbc_csr.port_6.cfg_account_pg_5.reserved_min: 0x" << pbc_csr.port_6.cfg_account_pg_5.reserved_min() << endl;
data <<"pbc_csr.port_6.cfg_account_pg_5.low_limit: 0x" << pbc_csr.port_6.cfg_account_pg_5.low_limit() << endl;

    pbc_csr.port_6.cfg_account_pg_6.read();
    data <<"pbc_csr.port_6.cfg_account_pg_6.all: 0x" << pbc_csr.port_6.cfg_account_pg_6.all() << endl;
data <<"pbc_csr.port_6.cfg_account_pg_6.alpha: 0x" << pbc_csr.port_6.cfg_account_pg_6.alpha() << endl;
data <<"pbc_csr.port_6.cfg_account_pg_6.xon_threshold: 0x" << pbc_csr.port_6.cfg_account_pg_6.xon_threshold() << endl;
data <<"pbc_csr.port_6.cfg_account_pg_6.headroom: 0x" << pbc_csr.port_6.cfg_account_pg_6.headroom() << endl;
data <<"pbc_csr.port_6.cfg_account_pg_6.reserved_min: 0x" << pbc_csr.port_6.cfg_account_pg_6.reserved_min() << endl;
data <<"pbc_csr.port_6.cfg_account_pg_6.low_limit: 0x" << pbc_csr.port_6.cfg_account_pg_6.low_limit() << endl;

    pbc_csr.port_6.cfg_account_pg_7.read();
    data <<"pbc_csr.port_6.cfg_account_pg_7.all: 0x" << pbc_csr.port_6.cfg_account_pg_7.all() << endl;
data <<"pbc_csr.port_6.cfg_account_pg_7.alpha: 0x" << pbc_csr.port_6.cfg_account_pg_7.alpha() << endl;
data <<"pbc_csr.port_6.cfg_account_pg_7.xon_threshold: 0x" << pbc_csr.port_6.cfg_account_pg_7.xon_threshold() << endl;
data <<"pbc_csr.port_6.cfg_account_pg_7.headroom: 0x" << pbc_csr.port_6.cfg_account_pg_7.headroom() << endl;
data <<"pbc_csr.port_6.cfg_account_pg_7.reserved_min: 0x" << pbc_csr.port_6.cfg_account_pg_7.reserved_min() << endl;
data <<"pbc_csr.port_6.cfg_account_pg_7.low_limit: 0x" << pbc_csr.port_6.cfg_account_pg_7.low_limit() << endl;

    pbc_csr.port_6.cfg_account_pg_8.read();
    data <<"pbc_csr.port_6.cfg_account_pg_8.all: 0x" << pbc_csr.port_6.cfg_account_pg_8.all() << endl;
data <<"pbc_csr.port_6.cfg_account_pg_8.alpha: 0x" << pbc_csr.port_6.cfg_account_pg_8.alpha() << endl;
data <<"pbc_csr.port_6.cfg_account_pg_8.xon_threshold: 0x" << pbc_csr.port_6.cfg_account_pg_8.xon_threshold() << endl;
data <<"pbc_csr.port_6.cfg_account_pg_8.headroom: 0x" << pbc_csr.port_6.cfg_account_pg_8.headroom() << endl;
data <<"pbc_csr.port_6.cfg_account_pg_8.reserved_min: 0x" << pbc_csr.port_6.cfg_account_pg_8.reserved_min() << endl;
data <<"pbc_csr.port_6.cfg_account_pg_8.low_limit: 0x" << pbc_csr.port_6.cfg_account_pg_8.low_limit() << endl;

    pbc_csr.port_6.cfg_account_pg_9.read();
    data <<"pbc_csr.port_6.cfg_account_pg_9.all: 0x" << pbc_csr.port_6.cfg_account_pg_9.all() << endl;
data <<"pbc_csr.port_6.cfg_account_pg_9.alpha: 0x" << pbc_csr.port_6.cfg_account_pg_9.alpha() << endl;
data <<"pbc_csr.port_6.cfg_account_pg_9.xon_threshold: 0x" << pbc_csr.port_6.cfg_account_pg_9.xon_threshold() << endl;
data <<"pbc_csr.port_6.cfg_account_pg_9.headroom: 0x" << pbc_csr.port_6.cfg_account_pg_9.headroom() << endl;
data <<"pbc_csr.port_6.cfg_account_pg_9.reserved_min: 0x" << pbc_csr.port_6.cfg_account_pg_9.reserved_min() << endl;
data <<"pbc_csr.port_6.cfg_account_pg_9.low_limit: 0x" << pbc_csr.port_6.cfg_account_pg_9.low_limit() << endl;

    pbc_csr.port_6.cfg_account_pg_to_sp.read();
    data <<"pbc_csr.port_6.cfg_account_pg_to_sp.all: 0x" << pbc_csr.port_6.cfg_account_pg_to_sp.all() << endl;
data <<"pbc_csr.port_6.cfg_account_pg_to_sp.map: 0x" << pbc_csr.port_6.cfg_account_pg_to_sp.map() << endl;

    pbc_csr.port_6.cfg_account_sp_fetch_quanta.read();
    data <<"pbc_csr.port_6.cfg_account_sp_fetch_quanta.all: 0x" << pbc_csr.port_6.cfg_account_sp_fetch_quanta.all() << endl;
data <<"pbc_csr.port_6.cfg_account_sp_fetch_quanta.pg30: 0x" << pbc_csr.port_6.cfg_account_sp_fetch_quanta.pg30() << endl;
data <<"pbc_csr.port_6.cfg_account_sp_fetch_quanta.pg31: 0x" << pbc_csr.port_6.cfg_account_sp_fetch_quanta.pg31() << endl;
data <<"pbc_csr.port_6.cfg_account_sp_fetch_quanta.pg18: 0x" << pbc_csr.port_6.cfg_account_sp_fetch_quanta.pg18() << endl;
data <<"pbc_csr.port_6.cfg_account_sp_fetch_quanta.pg19: 0x" << pbc_csr.port_6.cfg_account_sp_fetch_quanta.pg19() << endl;
data <<"pbc_csr.port_6.cfg_account_sp_fetch_quanta.pg10: 0x" << pbc_csr.port_6.cfg_account_sp_fetch_quanta.pg10() << endl;
data <<"pbc_csr.port_6.cfg_account_sp_fetch_quanta.pg11: 0x" << pbc_csr.port_6.cfg_account_sp_fetch_quanta.pg11() << endl;
data <<"pbc_csr.port_6.cfg_account_sp_fetch_quanta.pg12: 0x" << pbc_csr.port_6.cfg_account_sp_fetch_quanta.pg12() << endl;
data <<"pbc_csr.port_6.cfg_account_sp_fetch_quanta.pg13: 0x" << pbc_csr.port_6.cfg_account_sp_fetch_quanta.pg13() << endl;
data <<"pbc_csr.port_6.cfg_account_sp_fetch_quanta.pg14: 0x" << pbc_csr.port_6.cfg_account_sp_fetch_quanta.pg14() << endl;
data <<"pbc_csr.port_6.cfg_account_sp_fetch_quanta.pg15: 0x" << pbc_csr.port_6.cfg_account_sp_fetch_quanta.pg15() << endl;
data <<"pbc_csr.port_6.cfg_account_sp_fetch_quanta.pg16: 0x" << pbc_csr.port_6.cfg_account_sp_fetch_quanta.pg16() << endl;
data <<"pbc_csr.port_6.cfg_account_sp_fetch_quanta.pg17: 0x" << pbc_csr.port_6.cfg_account_sp_fetch_quanta.pg17() << endl;
data <<"pbc_csr.port_6.cfg_account_sp_fetch_quanta.pg6: 0x" << pbc_csr.port_6.cfg_account_sp_fetch_quanta.pg6() << endl;
data <<"pbc_csr.port_6.cfg_account_sp_fetch_quanta.pg7: 0x" << pbc_csr.port_6.cfg_account_sp_fetch_quanta.pg7() << endl;
data <<"pbc_csr.port_6.cfg_account_sp_fetch_quanta.pg4: 0x" << pbc_csr.port_6.cfg_account_sp_fetch_quanta.pg4() << endl;
data <<"pbc_csr.port_6.cfg_account_sp_fetch_quanta.pg5: 0x" << pbc_csr.port_6.cfg_account_sp_fetch_quanta.pg5() << endl;
data <<"pbc_csr.port_6.cfg_account_sp_fetch_quanta.pg2: 0x" << pbc_csr.port_6.cfg_account_sp_fetch_quanta.pg2() << endl;
data <<"pbc_csr.port_6.cfg_account_sp_fetch_quanta.pg3: 0x" << pbc_csr.port_6.cfg_account_sp_fetch_quanta.pg3() << endl;
data <<"pbc_csr.port_6.cfg_account_sp_fetch_quanta.pg0: 0x" << pbc_csr.port_6.cfg_account_sp_fetch_quanta.pg0() << endl;
data <<"pbc_csr.port_6.cfg_account_sp_fetch_quanta.pg1: 0x" << pbc_csr.port_6.cfg_account_sp_fetch_quanta.pg1() << endl;
data <<"pbc_csr.port_6.cfg_account_sp_fetch_quanta.pg8: 0x" << pbc_csr.port_6.cfg_account_sp_fetch_quanta.pg8() << endl;
data <<"pbc_csr.port_6.cfg_account_sp_fetch_quanta.pg9: 0x" << pbc_csr.port_6.cfg_account_sp_fetch_quanta.pg9() << endl;
data <<"pbc_csr.port_6.cfg_account_sp_fetch_quanta.pg21: 0x" << pbc_csr.port_6.cfg_account_sp_fetch_quanta.pg21() << endl;
data <<"pbc_csr.port_6.cfg_account_sp_fetch_quanta.pg20: 0x" << pbc_csr.port_6.cfg_account_sp_fetch_quanta.pg20() << endl;
data <<"pbc_csr.port_6.cfg_account_sp_fetch_quanta.pg23: 0x" << pbc_csr.port_6.cfg_account_sp_fetch_quanta.pg23() << endl;
data <<"pbc_csr.port_6.cfg_account_sp_fetch_quanta.pg22: 0x" << pbc_csr.port_6.cfg_account_sp_fetch_quanta.pg22() << endl;
data <<"pbc_csr.port_6.cfg_account_sp_fetch_quanta.pg25: 0x" << pbc_csr.port_6.cfg_account_sp_fetch_quanta.pg25() << endl;
data <<"pbc_csr.port_6.cfg_account_sp_fetch_quanta.pg24: 0x" << pbc_csr.port_6.cfg_account_sp_fetch_quanta.pg24() << endl;
data <<"pbc_csr.port_6.cfg_account_sp_fetch_quanta.pg27: 0x" << pbc_csr.port_6.cfg_account_sp_fetch_quanta.pg27() << endl;
data <<"pbc_csr.port_6.cfg_account_sp_fetch_quanta.pg26: 0x" << pbc_csr.port_6.cfg_account_sp_fetch_quanta.pg26() << endl;
data <<"pbc_csr.port_6.cfg_account_sp_fetch_quanta.pg29: 0x" << pbc_csr.port_6.cfg_account_sp_fetch_quanta.pg29() << endl;
data <<"pbc_csr.port_6.cfg_account_sp_fetch_quanta.pg28: 0x" << pbc_csr.port_6.cfg_account_sp_fetch_quanta.pg28() << endl;

    pbc_csr.port_6.cfg_account_sp_fetch_threshold.read();
    data <<"pbc_csr.port_6.cfg_account_sp_fetch_threshold.all: 0x" << pbc_csr.port_6.cfg_account_sp_fetch_threshold.all() << endl;
data <<"pbc_csr.port_6.cfg_account_sp_fetch_threshold.pg30: 0x" << pbc_csr.port_6.cfg_account_sp_fetch_threshold.pg30() << endl;
data <<"pbc_csr.port_6.cfg_account_sp_fetch_threshold.pg31: 0x" << pbc_csr.port_6.cfg_account_sp_fetch_threshold.pg31() << endl;
data <<"pbc_csr.port_6.cfg_account_sp_fetch_threshold.pg18: 0x" << pbc_csr.port_6.cfg_account_sp_fetch_threshold.pg18() << endl;
data <<"pbc_csr.port_6.cfg_account_sp_fetch_threshold.pg19: 0x" << pbc_csr.port_6.cfg_account_sp_fetch_threshold.pg19() << endl;
data <<"pbc_csr.port_6.cfg_account_sp_fetch_threshold.pg10: 0x" << pbc_csr.port_6.cfg_account_sp_fetch_threshold.pg10() << endl;
data <<"pbc_csr.port_6.cfg_account_sp_fetch_threshold.pg11: 0x" << pbc_csr.port_6.cfg_account_sp_fetch_threshold.pg11() << endl;
data <<"pbc_csr.port_6.cfg_account_sp_fetch_threshold.pg12: 0x" << pbc_csr.port_6.cfg_account_sp_fetch_threshold.pg12() << endl;
data <<"pbc_csr.port_6.cfg_account_sp_fetch_threshold.pg13: 0x" << pbc_csr.port_6.cfg_account_sp_fetch_threshold.pg13() << endl;
data <<"pbc_csr.port_6.cfg_account_sp_fetch_threshold.pg14: 0x" << pbc_csr.port_6.cfg_account_sp_fetch_threshold.pg14() << endl;
data <<"pbc_csr.port_6.cfg_account_sp_fetch_threshold.pg15: 0x" << pbc_csr.port_6.cfg_account_sp_fetch_threshold.pg15() << endl;
data <<"pbc_csr.port_6.cfg_account_sp_fetch_threshold.pg16: 0x" << pbc_csr.port_6.cfg_account_sp_fetch_threshold.pg16() << endl;
data <<"pbc_csr.port_6.cfg_account_sp_fetch_threshold.pg17: 0x" << pbc_csr.port_6.cfg_account_sp_fetch_threshold.pg17() << endl;
data <<"pbc_csr.port_6.cfg_account_sp_fetch_threshold.pg6: 0x" << pbc_csr.port_6.cfg_account_sp_fetch_threshold.pg6() << endl;
data <<"pbc_csr.port_6.cfg_account_sp_fetch_threshold.pg7: 0x" << pbc_csr.port_6.cfg_account_sp_fetch_threshold.pg7() << endl;
data <<"pbc_csr.port_6.cfg_account_sp_fetch_threshold.pg4: 0x" << pbc_csr.port_6.cfg_account_sp_fetch_threshold.pg4() << endl;
data <<"pbc_csr.port_6.cfg_account_sp_fetch_threshold.pg5: 0x" << pbc_csr.port_6.cfg_account_sp_fetch_threshold.pg5() << endl;
data <<"pbc_csr.port_6.cfg_account_sp_fetch_threshold.pg2: 0x" << pbc_csr.port_6.cfg_account_sp_fetch_threshold.pg2() << endl;
data <<"pbc_csr.port_6.cfg_account_sp_fetch_threshold.pg3: 0x" << pbc_csr.port_6.cfg_account_sp_fetch_threshold.pg3() << endl;
data <<"pbc_csr.port_6.cfg_account_sp_fetch_threshold.pg0: 0x" << pbc_csr.port_6.cfg_account_sp_fetch_threshold.pg0() << endl;
data <<"pbc_csr.port_6.cfg_account_sp_fetch_threshold.pg1: 0x" << pbc_csr.port_6.cfg_account_sp_fetch_threshold.pg1() << endl;
data <<"pbc_csr.port_6.cfg_account_sp_fetch_threshold.pg8: 0x" << pbc_csr.port_6.cfg_account_sp_fetch_threshold.pg8() << endl;
data <<"pbc_csr.port_6.cfg_account_sp_fetch_threshold.pg9: 0x" << pbc_csr.port_6.cfg_account_sp_fetch_threshold.pg9() << endl;
data <<"pbc_csr.port_6.cfg_account_sp_fetch_threshold.pg21: 0x" << pbc_csr.port_6.cfg_account_sp_fetch_threshold.pg21() << endl;
data <<"pbc_csr.port_6.cfg_account_sp_fetch_threshold.pg20: 0x" << pbc_csr.port_6.cfg_account_sp_fetch_threshold.pg20() << endl;
data <<"pbc_csr.port_6.cfg_account_sp_fetch_threshold.pg23: 0x" << pbc_csr.port_6.cfg_account_sp_fetch_threshold.pg23() << endl;
data <<"pbc_csr.port_6.cfg_account_sp_fetch_threshold.pg22: 0x" << pbc_csr.port_6.cfg_account_sp_fetch_threshold.pg22() << endl;
data <<"pbc_csr.port_6.cfg_account_sp_fetch_threshold.pg25: 0x" << pbc_csr.port_6.cfg_account_sp_fetch_threshold.pg25() << endl;
data <<"pbc_csr.port_6.cfg_account_sp_fetch_threshold.pg24: 0x" << pbc_csr.port_6.cfg_account_sp_fetch_threshold.pg24() << endl;
data <<"pbc_csr.port_6.cfg_account_sp_fetch_threshold.pg27: 0x" << pbc_csr.port_6.cfg_account_sp_fetch_threshold.pg27() << endl;
data <<"pbc_csr.port_6.cfg_account_sp_fetch_threshold.pg26: 0x" << pbc_csr.port_6.cfg_account_sp_fetch_threshold.pg26() << endl;
data <<"pbc_csr.port_6.cfg_account_sp_fetch_threshold.pg29: 0x" << pbc_csr.port_6.cfg_account_sp_fetch_threshold.pg29() << endl;
data <<"pbc_csr.port_6.cfg_account_sp_fetch_threshold.pg28: 0x" << pbc_csr.port_6.cfg_account_sp_fetch_threshold.pg28() << endl;

    pbc_csr.port_6.cfg_account_tc_to_pg.read();
    data <<"pbc_csr.port_6.cfg_account_tc_to_pg.all: 0x" << pbc_csr.port_6.cfg_account_tc_to_pg.all() << endl;
data <<"pbc_csr.port_6.cfg_account_tc_to_pg.table: 0x" << pbc_csr.port_6.cfg_account_tc_to_pg.table() << endl;

    pbc_csr.port_6.cfg_latency_cache.read();
    data <<"pbc_csr.port_6.cfg_latency_cache.all: 0x" << pbc_csr.port_6.cfg_latency_cache.all() << endl;
data <<"pbc_csr.port_6.cfg_latency_cache.enable: 0x" << pbc_csr.port_6.cfg_latency_cache.enable() << endl;

    pbc_csr.port_6.cfg_oq.read();
    data <<"pbc_csr.port_6.cfg_oq.all: 0x" << pbc_csr.port_6.cfg_oq.all() << endl;
data <<"pbc_csr.port_6.cfg_oq.enable: 0x" << pbc_csr.port_6.cfg_oq.enable() << endl;
data <<"pbc_csr.port_6.cfg_oq.ecc_disable_cor: 0x" << pbc_csr.port_6.cfg_oq.ecc_disable_cor() << endl;
data <<"pbc_csr.port_6.cfg_oq.span_cpu_num_cells: 0x" << pbc_csr.port_6.cfg_oq.span_cpu_num_cells() << endl;
data <<"pbc_csr.port_6.cfg_oq.dhs_eccbypass: 0x" << pbc_csr.port_6.cfg_oq.dhs_eccbypass() << endl;
data <<"pbc_csr.port_6.cfg_oq.ipg_bytes: 0x" << pbc_csr.port_6.cfg_oq.ipg_bytes() << endl;
data <<"pbc_csr.port_6.cfg_oq.bist_run: 0x" << pbc_csr.port_6.cfg_oq.bist_run() << endl;
data <<"pbc_csr.port_6.cfg_oq.flow_control_enable_xoff: 0x" << pbc_csr.port_6.cfg_oq.flow_control_enable_xoff() << endl;
data <<"pbc_csr.port_6.cfg_oq.ecc_disable_det: 0x" << pbc_csr.port_6.cfg_oq.ecc_disable_det() << endl;
data <<"pbc_csr.port_6.cfg_oq.replication_enable: 0x" << pbc_csr.port_6.cfg_oq.replication_enable() << endl;
data <<"pbc_csr.port_6.cfg_oq.flush: 0x" << pbc_csr.port_6.cfg_oq.flush() << endl;
data <<"pbc_csr.port_6.cfg_oq.flow_control_enable_credits: 0x" << pbc_csr.port_6.cfg_oq.flow_control_enable_credits() << endl;
data <<"pbc_csr.port_6.cfg_oq.rewrite_enable: 0x" << pbc_csr.port_6.cfg_oq.rewrite_enable() << endl;
data <<"pbc_csr.port_6.cfg_oq.dhs_selection: 0x" << pbc_csr.port_6.cfg_oq.dhs_selection() << endl;

    pbc_csr.port_6.cfg_oq_arb_l1_selection.read();
    data <<"pbc_csr.port_6.cfg_oq_arb_l1_selection.all: 0x" << pbc_csr.port_6.cfg_oq_arb_l1_selection.all() << endl;
data <<"pbc_csr.port_6.cfg_oq_arb_l1_selection.node_13: 0x" << pbc_csr.port_6.cfg_oq_arb_l1_selection.node_13() << endl;
data <<"pbc_csr.port_6.cfg_oq_arb_l1_selection.node_14: 0x" << pbc_csr.port_6.cfg_oq_arb_l1_selection.node_14() << endl;
data <<"pbc_csr.port_6.cfg_oq_arb_l1_selection.node_9: 0x" << pbc_csr.port_6.cfg_oq_arb_l1_selection.node_9() << endl;
data <<"pbc_csr.port_6.cfg_oq_arb_l1_selection.node_8: 0x" << pbc_csr.port_6.cfg_oq_arb_l1_selection.node_8() << endl;
data <<"pbc_csr.port_6.cfg_oq_arb_l1_selection.node_7: 0x" << pbc_csr.port_6.cfg_oq_arb_l1_selection.node_7() << endl;
data <<"pbc_csr.port_6.cfg_oq_arb_l1_selection.node_6: 0x" << pbc_csr.port_6.cfg_oq_arb_l1_selection.node_6() << endl;
data <<"pbc_csr.port_6.cfg_oq_arb_l1_selection.node_5: 0x" << pbc_csr.port_6.cfg_oq_arb_l1_selection.node_5() << endl;
data <<"pbc_csr.port_6.cfg_oq_arb_l1_selection.node_4: 0x" << pbc_csr.port_6.cfg_oq_arb_l1_selection.node_4() << endl;
data <<"pbc_csr.port_6.cfg_oq_arb_l1_selection.node_3: 0x" << pbc_csr.port_6.cfg_oq_arb_l1_selection.node_3() << endl;
data <<"pbc_csr.port_6.cfg_oq_arb_l1_selection.node_2: 0x" << pbc_csr.port_6.cfg_oq_arb_l1_selection.node_2() << endl;
data <<"pbc_csr.port_6.cfg_oq_arb_l1_selection.node_1: 0x" << pbc_csr.port_6.cfg_oq_arb_l1_selection.node_1() << endl;
data <<"pbc_csr.port_6.cfg_oq_arb_l1_selection.node_0: 0x" << pbc_csr.port_6.cfg_oq_arb_l1_selection.node_0() << endl;
data <<"pbc_csr.port_6.cfg_oq_arb_l1_selection.node_11: 0x" << pbc_csr.port_6.cfg_oq_arb_l1_selection.node_11() << endl;
data <<"pbc_csr.port_6.cfg_oq_arb_l1_selection.node_10: 0x" << pbc_csr.port_6.cfg_oq_arb_l1_selection.node_10() << endl;
data <<"pbc_csr.port_6.cfg_oq_arb_l1_selection.node_15: 0x" << pbc_csr.port_6.cfg_oq_arb_l1_selection.node_15() << endl;
data <<"pbc_csr.port_6.cfg_oq_arb_l1_selection.node_12: 0x" << pbc_csr.port_6.cfg_oq_arb_l1_selection.node_12() << endl;

    pbc_csr.port_6.cfg_oq_arb_l1_strict.read();
    data <<"pbc_csr.port_6.cfg_oq_arb_l1_strict.all: 0x" << pbc_csr.port_6.cfg_oq_arb_l1_strict.all() << endl;
data <<"pbc_csr.port_6.cfg_oq_arb_l1_strict.priority: 0x" << pbc_csr.port_6.cfg_oq_arb_l1_strict.priority() << endl;
data <<"pbc_csr.port_6.cfg_oq_arb_l1_strict.priority_bypass_timer: 0x" << pbc_csr.port_6.cfg_oq_arb_l1_strict.priority_bypass_timer() << endl;

    pbc_csr.port_6.cfg_oq_arb_l2_selection.read();
    data <<"pbc_csr.port_6.cfg_oq_arb_l2_selection.all: 0x" << pbc_csr.port_6.cfg_oq_arb_l2_selection.all() << endl;
data <<"pbc_csr.port_6.cfg_oq_arb_l2_selection.node_3: 0x" << pbc_csr.port_6.cfg_oq_arb_l2_selection.node_3() << endl;
data <<"pbc_csr.port_6.cfg_oq_arb_l2_selection.node_2: 0x" << pbc_csr.port_6.cfg_oq_arb_l2_selection.node_2() << endl;
data <<"pbc_csr.port_6.cfg_oq_arb_l2_selection.node_1: 0x" << pbc_csr.port_6.cfg_oq_arb_l2_selection.node_1() << endl;
data <<"pbc_csr.port_6.cfg_oq_arb_l2_selection.node_0: 0x" << pbc_csr.port_6.cfg_oq_arb_l2_selection.node_0() << endl;

    pbc_csr.port_6.cfg_oq_arb_l2_strict.read();
    data <<"pbc_csr.port_6.cfg_oq_arb_l2_strict.all: 0x" << pbc_csr.port_6.cfg_oq_arb_l2_strict.all() << endl;
data <<"pbc_csr.port_6.cfg_oq_arb_l2_strict.priority: 0x" << pbc_csr.port_6.cfg_oq_arb_l2_strict.priority() << endl;
data <<"pbc_csr.port_6.cfg_oq_arb_l2_strict.priority_bypass_timer: 0x" << pbc_csr.port_6.cfg_oq_arb_l2_strict.priority_bypass_timer() << endl;

    pbc_csr.port_6.cfg_oq_arb_l3_selection.read();
    data <<"pbc_csr.port_6.cfg_oq_arb_l3_selection.all: 0x" << pbc_csr.port_6.cfg_oq_arb_l3_selection.all() << endl;
data <<"pbc_csr.port_6.cfg_oq_arb_l3_selection.node_0: 0x" << pbc_csr.port_6.cfg_oq_arb_l3_selection.node_0() << endl;

    pbc_csr.port_6.cfg_oq_arb_l3_strict.read();
    data <<"pbc_csr.port_6.cfg_oq_arb_l3_strict.all: 0x" << pbc_csr.port_6.cfg_oq_arb_l3_strict.all() << endl;
data <<"pbc_csr.port_6.cfg_oq_arb_l3_strict.priority: 0x" << pbc_csr.port_6.cfg_oq_arb_l3_strict.priority() << endl;
data <<"pbc_csr.port_6.cfg_oq_arb_l3_strict.priority_bypass_timer: 0x" << pbc_csr.port_6.cfg_oq_arb_l3_strict.priority_bypass_timer() << endl;

    pbc_csr.port_6.cfg_oq_dhs_mem.read();
    data <<"pbc_csr.port_6.cfg_oq_dhs_mem.all: 0x" << pbc_csr.port_6.cfg_oq_dhs_mem.all() << endl;
data <<"pbc_csr.port_6.cfg_oq_dhs_mem.address: 0x" << pbc_csr.port_6.cfg_oq_dhs_mem.address() << endl;

    pbc_csr.port_6.cfg_oq_queue.read();
    data <<"pbc_csr.port_6.cfg_oq_queue.all: 0x" << pbc_csr.port_6.cfg_oq_queue.all() << endl;
data <<"pbc_csr.port_6.cfg_oq_queue.recirc: 0x" << pbc_csr.port_6.cfg_oq_queue.recirc() << endl;
data <<"pbc_csr.port_6.cfg_oq_queue.enable: 0x" << pbc_csr.port_6.cfg_oq_queue.enable() << endl;
data <<"pbc_csr.port_6.cfg_oq_queue.flush: 0x" << pbc_csr.port_6.cfg_oq_queue.flush() << endl;

    pbc_csr.port_6.cfg_oq_queue_dynamic.read();
    data <<"pbc_csr.port_6.cfg_oq_queue_dynamic.all: 0x" << pbc_csr.port_6.cfg_oq_queue_dynamic.all() << endl;
data <<"pbc_csr.port_6.cfg_oq_queue_dynamic.threshold: 0x" << pbc_csr.port_6.cfg_oq_queue_dynamic.threshold() << endl;

    pbc_csr.port_6.cfg_oq_queue_write_one.read();
    data <<"pbc_csr.port_6.cfg_oq_queue_write_one.all: 0x" << pbc_csr.port_6.cfg_oq_queue_write_one.all() << endl;
data <<"pbc_csr.port_6.cfg_oq_queue_write_one.enable: 0x" << pbc_csr.port_6.cfg_oq_queue_write_one.enable() << endl;
data <<"pbc_csr.port_6.cfg_oq_queue_write_one.disable: 0x" << pbc_csr.port_6.cfg_oq_queue_write_one.disable() << endl;

    pbc_csr.port_6.cfg_oq_shared_pool.read();
    data <<"pbc_csr.port_6.cfg_oq_shared_pool.all: 0x" << pbc_csr.port_6.cfg_oq_shared_pool.all() << endl;
data <<"pbc_csr.port_6.cfg_oq_shared_pool.max_decrement: 0x" << pbc_csr.port_6.cfg_oq_shared_pool.max_decrement() << endl;
data <<"pbc_csr.port_6.cfg_oq_shared_pool.alpha_1: 0x" << pbc_csr.port_6.cfg_oq_shared_pool.alpha_1() << endl;
data <<"pbc_csr.port_6.cfg_oq_shared_pool.alpha_0: 0x" << pbc_csr.port_6.cfg_oq_shared_pool.alpha_0() << endl;
data <<"pbc_csr.port_6.cfg_oq_shared_pool.credits_0: 0x" << pbc_csr.port_6.cfg_oq_shared_pool.credits_0() << endl;
data <<"pbc_csr.port_6.cfg_oq_shared_pool.credits_1: 0x" << pbc_csr.port_6.cfg_oq_shared_pool.credits_1() << endl;

    pbc_csr.port_6.cfg_oq_xoff2oq.read();
    data <<"pbc_csr.port_6.cfg_oq_xoff2oq.all: 0x" << pbc_csr.port_6.cfg_oq_xoff2oq.all() << endl;
data <<"pbc_csr.port_6.cfg_oq_xoff2oq.map: 0x" << pbc_csr.port_6.cfg_oq_xoff2oq.map() << endl;

    pbc_csr.port_6.cfg_tail_drop.read();
    data <<"pbc_csr.port_6.cfg_tail_drop.all: 0x" << pbc_csr.port_6.cfg_tail_drop.all() << endl;
data <<"pbc_csr.port_6.cfg_tail_drop.cpu_threshold: 0x" << pbc_csr.port_6.cfg_tail_drop.cpu_threshold() << endl;
data <<"pbc_csr.port_6.cfg_tail_drop.span_threshold: 0x" << pbc_csr.port_6.cfg_tail_drop.span_threshold() << endl;

    pbc_csr.port_6.cfg_write_control.read();
    data <<"pbc_csr.port_6.cfg_write_control.all: 0x" << pbc_csr.port_6.cfg_write_control.all() << endl;
data <<"pbc_csr.port_6.cfg_write_control.release_cells: 0x" << pbc_csr.port_6.cfg_write_control.release_cells() << endl;
data <<"pbc_csr.port_6.cfg_write_control.rate_limiter_adj_time: 0x" << pbc_csr.port_6.cfg_write_control.rate_limiter_adj_time() << endl;
data <<"pbc_csr.port_6.cfg_write_control.enable: 0x" << pbc_csr.port_6.cfg_write_control.enable() << endl;
data <<"pbc_csr.port_6.cfg_write_control.enable_bmc_multicast: 0x" << pbc_csr.port_6.cfg_write_control.enable_bmc_multicast() << endl;
data <<"pbc_csr.port_6.cfg_write_control.rewrite: 0x" << pbc_csr.port_6.cfg_write_control.rewrite() << endl;
data <<"pbc_csr.port_6.cfg_write_control.span_pg_enable: 0x" << pbc_csr.port_6.cfg_write_control.span_pg_enable() << endl;
data <<"pbc_csr.port_6.cfg_write_control.span_pg_index: 0x" << pbc_csr.port_6.cfg_write_control.span_pg_index() << endl;
data <<"pbc_csr.port_6.cfg_write_control.enforce_oq24_31: 0x" << pbc_csr.port_6.cfg_write_control.enforce_oq24_31() << endl;
data <<"pbc_csr.port_6.cfg_write_control.drop_on_error: 0x" << pbc_csr.port_6.cfg_write_control.drop_on_error() << endl;
data <<"pbc_csr.port_6.cfg_write_control.min_size: 0x" << pbc_csr.port_6.cfg_write_control.min_size() << endl;
data <<"pbc_csr.port_6.cfg_write_control.rate_limiter_enable_force_emergency_stop: 0x" << pbc_csr.port_6.cfg_write_control.rate_limiter_enable_force_emergency_stop() << endl;
data <<"pbc_csr.port_6.cfg_write_control.cut_thru_select: 0x" << pbc_csr.port_6.cfg_write_control.cut_thru_select() << endl;
data <<"pbc_csr.port_6.cfg_write_control.rate_limiter_enable_emergency_stop: 0x" << pbc_csr.port_6.cfg_write_control.rate_limiter_enable_emergency_stop() << endl;
data <<"pbc_csr.port_6.cfg_write_control.enqueue_mux_bypass: 0x" << pbc_csr.port_6.cfg_write_control.enqueue_mux_bypass() << endl;
data <<"pbc_csr.port_6.cfg_write_control.use_intrinsic_credits: 0x" << pbc_csr.port_6.cfg_write_control.use_intrinsic_credits() << endl;
data <<"pbc_csr.port_6.cfg_write_control.enable_drdy: 0x" << pbc_csr.port_6.cfg_write_control.enable_drdy() << endl;
data <<"pbc_csr.port_6.cfg_write_control.recirc_oq: 0x" << pbc_csr.port_6.cfg_write_control.recirc_oq() << endl;
data <<"pbc_csr.port_6.cfg_write_control.rate_limiter: 0x" << pbc_csr.port_6.cfg_write_control.rate_limiter() << endl;
data <<"pbc_csr.port_6.cfg_write_control.cut_thru: 0x" << pbc_csr.port_6.cfg_write_control.cut_thru() << endl;
data <<"pbc_csr.port_6.cfg_write_control.recirc_enable: 0x" << pbc_csr.port_6.cfg_write_control.recirc_enable() << endl;
data <<"pbc_csr.port_6.cfg_write_control.enable_mnic_multicast: 0x" << pbc_csr.port_6.cfg_write_control.enable_mnic_multicast() << endl;

    pbc_csr.port_6.cfg_write_global.read();
    data <<"pbc_csr.port_6.cfg_write_global.all: 0x" << pbc_csr.port_6.cfg_write_global.all() << endl;
data <<"pbc_csr.port_6.cfg_write_global.occupancy_clear_threshold: 0x" << pbc_csr.port_6.cfg_write_global.occupancy_clear_threshold() << endl;
data <<"pbc_csr.port_6.cfg_write_global.occupancy_set_threshold: 0x" << pbc_csr.port_6.cfg_write_global.occupancy_set_threshold() << endl;

    pbc_csr.port_7.cfg_account_clear_xoff.read();
    data <<"pbc_csr.port_7.cfg_account_clear_xoff.all: 0x" << pbc_csr.port_7.cfg_account_clear_xoff.all() << endl;
data <<"pbc_csr.port_7.cfg_account_clear_xoff.bitmap: 0x" << pbc_csr.port_7.cfg_account_clear_xoff.bitmap() << endl;

    pbc_csr.port_7.cfg_account_control.read();
    data <<"pbc_csr.port_7.cfg_account_control.all: 0x" << pbc_csr.port_7.cfg_account_control.all() << endl;
data <<"pbc_csr.port_7.cfg_account_control.sp_enable: 0x" << pbc_csr.port_7.cfg_account_control.sp_enable() << endl;
data <<"pbc_csr.port_7.cfg_account_control.clear_wm_index: 0x" << pbc_csr.port_7.cfg_account_control.clear_wm_index() << endl;
data <<"pbc_csr.port_7.cfg_account_control.clear_wm_enable: 0x" << pbc_csr.port_7.cfg_account_control.clear_wm_enable() << endl;
data <<"pbc_csr.port_7.cfg_account_control.use_sp_as_wm: 0x" << pbc_csr.port_7.cfg_account_control.use_sp_as_wm() << endl;

    pbc_csr.port_7.cfg_account_credit_return.read();
    data <<"pbc_csr.port_7.cfg_account_credit_return.all: 0x" << pbc_csr.port_7.cfg_account_credit_return.all() << endl;
data <<"pbc_csr.port_7.cfg_account_credit_return.enable: 0x" << pbc_csr.port_7.cfg_account_credit_return.enable() << endl;

    pbc_csr.port_7.cfg_account_mtu_table.read();
    data <<"pbc_csr.port_7.cfg_account_mtu_table.all: 0x" << pbc_csr.port_7.cfg_account_mtu_table.all() << endl;
data <<"pbc_csr.port_7.cfg_account_mtu_table.pg30: 0x" << pbc_csr.port_7.cfg_account_mtu_table.pg30() << endl;
data <<"pbc_csr.port_7.cfg_account_mtu_table.pg31: 0x" << pbc_csr.port_7.cfg_account_mtu_table.pg31() << endl;
data <<"pbc_csr.port_7.cfg_account_mtu_table.pg18: 0x" << pbc_csr.port_7.cfg_account_mtu_table.pg18() << endl;
data <<"pbc_csr.port_7.cfg_account_mtu_table.pg19: 0x" << pbc_csr.port_7.cfg_account_mtu_table.pg19() << endl;
data <<"pbc_csr.port_7.cfg_account_mtu_table.pg10: 0x" << pbc_csr.port_7.cfg_account_mtu_table.pg10() << endl;
data <<"pbc_csr.port_7.cfg_account_mtu_table.pg11: 0x" << pbc_csr.port_7.cfg_account_mtu_table.pg11() << endl;
data <<"pbc_csr.port_7.cfg_account_mtu_table.pg12: 0x" << pbc_csr.port_7.cfg_account_mtu_table.pg12() << endl;
data <<"pbc_csr.port_7.cfg_account_mtu_table.pg13: 0x" << pbc_csr.port_7.cfg_account_mtu_table.pg13() << endl;
data <<"pbc_csr.port_7.cfg_account_mtu_table.pg14: 0x" << pbc_csr.port_7.cfg_account_mtu_table.pg14() << endl;
data <<"pbc_csr.port_7.cfg_account_mtu_table.pg15: 0x" << pbc_csr.port_7.cfg_account_mtu_table.pg15() << endl;
data <<"pbc_csr.port_7.cfg_account_mtu_table.pg16: 0x" << pbc_csr.port_7.cfg_account_mtu_table.pg16() << endl;
data <<"pbc_csr.port_7.cfg_account_mtu_table.pg17: 0x" << pbc_csr.port_7.cfg_account_mtu_table.pg17() << endl;
data <<"pbc_csr.port_7.cfg_account_mtu_table.pg6: 0x" << pbc_csr.port_7.cfg_account_mtu_table.pg6() << endl;
data <<"pbc_csr.port_7.cfg_account_mtu_table.pg7: 0x" << pbc_csr.port_7.cfg_account_mtu_table.pg7() << endl;
data <<"pbc_csr.port_7.cfg_account_mtu_table.pg4: 0x" << pbc_csr.port_7.cfg_account_mtu_table.pg4() << endl;
data <<"pbc_csr.port_7.cfg_account_mtu_table.pg5: 0x" << pbc_csr.port_7.cfg_account_mtu_table.pg5() << endl;
data <<"pbc_csr.port_7.cfg_account_mtu_table.pg2: 0x" << pbc_csr.port_7.cfg_account_mtu_table.pg2() << endl;
data <<"pbc_csr.port_7.cfg_account_mtu_table.pg3: 0x" << pbc_csr.port_7.cfg_account_mtu_table.pg3() << endl;
data <<"pbc_csr.port_7.cfg_account_mtu_table.pg0: 0x" << pbc_csr.port_7.cfg_account_mtu_table.pg0() << endl;
data <<"pbc_csr.port_7.cfg_account_mtu_table.pg1: 0x" << pbc_csr.port_7.cfg_account_mtu_table.pg1() << endl;
data <<"pbc_csr.port_7.cfg_account_mtu_table.pg8: 0x" << pbc_csr.port_7.cfg_account_mtu_table.pg8() << endl;
data <<"pbc_csr.port_7.cfg_account_mtu_table.pg9: 0x" << pbc_csr.port_7.cfg_account_mtu_table.pg9() << endl;
data <<"pbc_csr.port_7.cfg_account_mtu_table.pg21: 0x" << pbc_csr.port_7.cfg_account_mtu_table.pg21() << endl;
data <<"pbc_csr.port_7.cfg_account_mtu_table.pg20: 0x" << pbc_csr.port_7.cfg_account_mtu_table.pg20() << endl;
data <<"pbc_csr.port_7.cfg_account_mtu_table.pg23: 0x" << pbc_csr.port_7.cfg_account_mtu_table.pg23() << endl;
data <<"pbc_csr.port_7.cfg_account_mtu_table.pg22: 0x" << pbc_csr.port_7.cfg_account_mtu_table.pg22() << endl;
data <<"pbc_csr.port_7.cfg_account_mtu_table.pg25: 0x" << pbc_csr.port_7.cfg_account_mtu_table.pg25() << endl;
data <<"pbc_csr.port_7.cfg_account_mtu_table.pg24: 0x" << pbc_csr.port_7.cfg_account_mtu_table.pg24() << endl;
data <<"pbc_csr.port_7.cfg_account_mtu_table.pg27: 0x" << pbc_csr.port_7.cfg_account_mtu_table.pg27() << endl;
data <<"pbc_csr.port_7.cfg_account_mtu_table.pg26: 0x" << pbc_csr.port_7.cfg_account_mtu_table.pg26() << endl;
data <<"pbc_csr.port_7.cfg_account_mtu_table.pg29: 0x" << pbc_csr.port_7.cfg_account_mtu_table.pg29() << endl;
data <<"pbc_csr.port_7.cfg_account_mtu_table.pg28: 0x" << pbc_csr.port_7.cfg_account_mtu_table.pg28() << endl;

    pbc_csr.port_7.cfg_account_pause_timer.read();
    data <<"pbc_csr.port_7.cfg_account_pause_timer.all: 0x" << pbc_csr.port_7.cfg_account_pause_timer.all() << endl;
data <<"pbc_csr.port_7.cfg_account_pause_timer.xoff_timeout_auto_clear: 0x" << pbc_csr.port_7.cfg_account_pause_timer.xoff_timeout_auto_clear() << endl;
data <<"pbc_csr.port_7.cfg_account_pause_timer.xoff_timeout_auto_xon: 0x" << pbc_csr.port_7.cfg_account_pause_timer.xoff_timeout_auto_xon() << endl;
data <<"pbc_csr.port_7.cfg_account_pause_timer.xoff_timeout: 0x" << pbc_csr.port_7.cfg_account_pause_timer.xoff_timeout() << endl;

    pbc_csr.port_7.cfg_account_pg_0.read();
    data <<"pbc_csr.port_7.cfg_account_pg_0.all: 0x" << pbc_csr.port_7.cfg_account_pg_0.all() << endl;
data <<"pbc_csr.port_7.cfg_account_pg_0.alpha: 0x" << pbc_csr.port_7.cfg_account_pg_0.alpha() << endl;
data <<"pbc_csr.port_7.cfg_account_pg_0.xon_threshold: 0x" << pbc_csr.port_7.cfg_account_pg_0.xon_threshold() << endl;
data <<"pbc_csr.port_7.cfg_account_pg_0.headroom: 0x" << pbc_csr.port_7.cfg_account_pg_0.headroom() << endl;
data <<"pbc_csr.port_7.cfg_account_pg_0.reserved_min: 0x" << pbc_csr.port_7.cfg_account_pg_0.reserved_min() << endl;
data <<"pbc_csr.port_7.cfg_account_pg_0.low_limit: 0x" << pbc_csr.port_7.cfg_account_pg_0.low_limit() << endl;

    pbc_csr.port_7.cfg_account_pg_1.read();
    data <<"pbc_csr.port_7.cfg_account_pg_1.all: 0x" << pbc_csr.port_7.cfg_account_pg_1.all() << endl;
data <<"pbc_csr.port_7.cfg_account_pg_1.alpha: 0x" << pbc_csr.port_7.cfg_account_pg_1.alpha() << endl;
data <<"pbc_csr.port_7.cfg_account_pg_1.xon_threshold: 0x" << pbc_csr.port_7.cfg_account_pg_1.xon_threshold() << endl;
data <<"pbc_csr.port_7.cfg_account_pg_1.headroom: 0x" << pbc_csr.port_7.cfg_account_pg_1.headroom() << endl;
data <<"pbc_csr.port_7.cfg_account_pg_1.reserved_min: 0x" << pbc_csr.port_7.cfg_account_pg_1.reserved_min() << endl;
data <<"pbc_csr.port_7.cfg_account_pg_1.low_limit: 0x" << pbc_csr.port_7.cfg_account_pg_1.low_limit() << endl;

    pbc_csr.port_7.cfg_account_pg_10.read();
    data <<"pbc_csr.port_7.cfg_account_pg_10.all: 0x" << pbc_csr.port_7.cfg_account_pg_10.all() << endl;
data <<"pbc_csr.port_7.cfg_account_pg_10.alpha: 0x" << pbc_csr.port_7.cfg_account_pg_10.alpha() << endl;
data <<"pbc_csr.port_7.cfg_account_pg_10.xon_threshold: 0x" << pbc_csr.port_7.cfg_account_pg_10.xon_threshold() << endl;
data <<"pbc_csr.port_7.cfg_account_pg_10.headroom: 0x" << pbc_csr.port_7.cfg_account_pg_10.headroom() << endl;
data <<"pbc_csr.port_7.cfg_account_pg_10.reserved_min: 0x" << pbc_csr.port_7.cfg_account_pg_10.reserved_min() << endl;
data <<"pbc_csr.port_7.cfg_account_pg_10.low_limit: 0x" << pbc_csr.port_7.cfg_account_pg_10.low_limit() << endl;

    pbc_csr.port_7.cfg_account_pg_11.read();
    data <<"pbc_csr.port_7.cfg_account_pg_11.all: 0x" << pbc_csr.port_7.cfg_account_pg_11.all() << endl;
data <<"pbc_csr.port_7.cfg_account_pg_11.alpha: 0x" << pbc_csr.port_7.cfg_account_pg_11.alpha() << endl;
data <<"pbc_csr.port_7.cfg_account_pg_11.xon_threshold: 0x" << pbc_csr.port_7.cfg_account_pg_11.xon_threshold() << endl;
data <<"pbc_csr.port_7.cfg_account_pg_11.headroom: 0x" << pbc_csr.port_7.cfg_account_pg_11.headroom() << endl;
data <<"pbc_csr.port_7.cfg_account_pg_11.reserved_min: 0x" << pbc_csr.port_7.cfg_account_pg_11.reserved_min() << endl;
data <<"pbc_csr.port_7.cfg_account_pg_11.low_limit: 0x" << pbc_csr.port_7.cfg_account_pg_11.low_limit() << endl;

    pbc_csr.port_7.cfg_account_pg_12.read();
    data <<"pbc_csr.port_7.cfg_account_pg_12.all: 0x" << pbc_csr.port_7.cfg_account_pg_12.all() << endl;
data <<"pbc_csr.port_7.cfg_account_pg_12.alpha: 0x" << pbc_csr.port_7.cfg_account_pg_12.alpha() << endl;
data <<"pbc_csr.port_7.cfg_account_pg_12.xon_threshold: 0x" << pbc_csr.port_7.cfg_account_pg_12.xon_threshold() << endl;
data <<"pbc_csr.port_7.cfg_account_pg_12.headroom: 0x" << pbc_csr.port_7.cfg_account_pg_12.headroom() << endl;
data <<"pbc_csr.port_7.cfg_account_pg_12.reserved_min: 0x" << pbc_csr.port_7.cfg_account_pg_12.reserved_min() << endl;
data <<"pbc_csr.port_7.cfg_account_pg_12.low_limit: 0x" << pbc_csr.port_7.cfg_account_pg_12.low_limit() << endl;

    pbc_csr.port_7.cfg_account_pg_13.read();
    data <<"pbc_csr.port_7.cfg_account_pg_13.all: 0x" << pbc_csr.port_7.cfg_account_pg_13.all() << endl;
data <<"pbc_csr.port_7.cfg_account_pg_13.alpha: 0x" << pbc_csr.port_7.cfg_account_pg_13.alpha() << endl;
data <<"pbc_csr.port_7.cfg_account_pg_13.xon_threshold: 0x" << pbc_csr.port_7.cfg_account_pg_13.xon_threshold() << endl;
data <<"pbc_csr.port_7.cfg_account_pg_13.headroom: 0x" << pbc_csr.port_7.cfg_account_pg_13.headroom() << endl;
data <<"pbc_csr.port_7.cfg_account_pg_13.reserved_min: 0x" << pbc_csr.port_7.cfg_account_pg_13.reserved_min() << endl;
data <<"pbc_csr.port_7.cfg_account_pg_13.low_limit: 0x" << pbc_csr.port_7.cfg_account_pg_13.low_limit() << endl;

    pbc_csr.port_7.cfg_account_pg_14.read();
    data <<"pbc_csr.port_7.cfg_account_pg_14.all: 0x" << pbc_csr.port_7.cfg_account_pg_14.all() << endl;
data <<"pbc_csr.port_7.cfg_account_pg_14.alpha: 0x" << pbc_csr.port_7.cfg_account_pg_14.alpha() << endl;
data <<"pbc_csr.port_7.cfg_account_pg_14.xon_threshold: 0x" << pbc_csr.port_7.cfg_account_pg_14.xon_threshold() << endl;
data <<"pbc_csr.port_7.cfg_account_pg_14.headroom: 0x" << pbc_csr.port_7.cfg_account_pg_14.headroom() << endl;
data <<"pbc_csr.port_7.cfg_account_pg_14.reserved_min: 0x" << pbc_csr.port_7.cfg_account_pg_14.reserved_min() << endl;
data <<"pbc_csr.port_7.cfg_account_pg_14.low_limit: 0x" << pbc_csr.port_7.cfg_account_pg_14.low_limit() << endl;

    pbc_csr.port_7.cfg_account_pg_15.read();
    data <<"pbc_csr.port_7.cfg_account_pg_15.all: 0x" << pbc_csr.port_7.cfg_account_pg_15.all() << endl;
data <<"pbc_csr.port_7.cfg_account_pg_15.alpha: 0x" << pbc_csr.port_7.cfg_account_pg_15.alpha() << endl;
data <<"pbc_csr.port_7.cfg_account_pg_15.xon_threshold: 0x" << pbc_csr.port_7.cfg_account_pg_15.xon_threshold() << endl;
data <<"pbc_csr.port_7.cfg_account_pg_15.headroom: 0x" << pbc_csr.port_7.cfg_account_pg_15.headroom() << endl;
data <<"pbc_csr.port_7.cfg_account_pg_15.reserved_min: 0x" << pbc_csr.port_7.cfg_account_pg_15.reserved_min() << endl;
data <<"pbc_csr.port_7.cfg_account_pg_15.low_limit: 0x" << pbc_csr.port_7.cfg_account_pg_15.low_limit() << endl;

    pbc_csr.port_7.cfg_account_pg_16.read();
    data <<"pbc_csr.port_7.cfg_account_pg_16.all: 0x" << pbc_csr.port_7.cfg_account_pg_16.all() << endl;
data <<"pbc_csr.port_7.cfg_account_pg_16.alpha: 0x" << pbc_csr.port_7.cfg_account_pg_16.alpha() << endl;
data <<"pbc_csr.port_7.cfg_account_pg_16.xon_threshold: 0x" << pbc_csr.port_7.cfg_account_pg_16.xon_threshold() << endl;
data <<"pbc_csr.port_7.cfg_account_pg_16.headroom: 0x" << pbc_csr.port_7.cfg_account_pg_16.headroom() << endl;
data <<"pbc_csr.port_7.cfg_account_pg_16.reserved_min: 0x" << pbc_csr.port_7.cfg_account_pg_16.reserved_min() << endl;
data <<"pbc_csr.port_7.cfg_account_pg_16.low_limit: 0x" << pbc_csr.port_7.cfg_account_pg_16.low_limit() << endl;

    pbc_csr.port_7.cfg_account_pg_17.read();
    data <<"pbc_csr.port_7.cfg_account_pg_17.all: 0x" << pbc_csr.port_7.cfg_account_pg_17.all() << endl;
data <<"pbc_csr.port_7.cfg_account_pg_17.alpha: 0x" << pbc_csr.port_7.cfg_account_pg_17.alpha() << endl;
data <<"pbc_csr.port_7.cfg_account_pg_17.xon_threshold: 0x" << pbc_csr.port_7.cfg_account_pg_17.xon_threshold() << endl;
data <<"pbc_csr.port_7.cfg_account_pg_17.headroom: 0x" << pbc_csr.port_7.cfg_account_pg_17.headroom() << endl;
data <<"pbc_csr.port_7.cfg_account_pg_17.reserved_min: 0x" << pbc_csr.port_7.cfg_account_pg_17.reserved_min() << endl;
data <<"pbc_csr.port_7.cfg_account_pg_17.low_limit: 0x" << pbc_csr.port_7.cfg_account_pg_17.low_limit() << endl;

    pbc_csr.port_7.cfg_account_pg_18.read();
    data <<"pbc_csr.port_7.cfg_account_pg_18.all: 0x" << pbc_csr.port_7.cfg_account_pg_18.all() << endl;
data <<"pbc_csr.port_7.cfg_account_pg_18.alpha: 0x" << pbc_csr.port_7.cfg_account_pg_18.alpha() << endl;
data <<"pbc_csr.port_7.cfg_account_pg_18.xon_threshold: 0x" << pbc_csr.port_7.cfg_account_pg_18.xon_threshold() << endl;
data <<"pbc_csr.port_7.cfg_account_pg_18.headroom: 0x" << pbc_csr.port_7.cfg_account_pg_18.headroom() << endl;
data <<"pbc_csr.port_7.cfg_account_pg_18.reserved_min: 0x" << pbc_csr.port_7.cfg_account_pg_18.reserved_min() << endl;
data <<"pbc_csr.port_7.cfg_account_pg_18.low_limit: 0x" << pbc_csr.port_7.cfg_account_pg_18.low_limit() << endl;

    pbc_csr.port_7.cfg_account_pg_19.read();
    data <<"pbc_csr.port_7.cfg_account_pg_19.all: 0x" << pbc_csr.port_7.cfg_account_pg_19.all() << endl;
data <<"pbc_csr.port_7.cfg_account_pg_19.alpha: 0x" << pbc_csr.port_7.cfg_account_pg_19.alpha() << endl;
data <<"pbc_csr.port_7.cfg_account_pg_19.xon_threshold: 0x" << pbc_csr.port_7.cfg_account_pg_19.xon_threshold() << endl;
data <<"pbc_csr.port_7.cfg_account_pg_19.headroom: 0x" << pbc_csr.port_7.cfg_account_pg_19.headroom() << endl;
data <<"pbc_csr.port_7.cfg_account_pg_19.reserved_min: 0x" << pbc_csr.port_7.cfg_account_pg_19.reserved_min() << endl;
data <<"pbc_csr.port_7.cfg_account_pg_19.low_limit: 0x" << pbc_csr.port_7.cfg_account_pg_19.low_limit() << endl;

    pbc_csr.port_7.cfg_account_pg_2.read();
    data <<"pbc_csr.port_7.cfg_account_pg_2.all: 0x" << pbc_csr.port_7.cfg_account_pg_2.all() << endl;
data <<"pbc_csr.port_7.cfg_account_pg_2.alpha: 0x" << pbc_csr.port_7.cfg_account_pg_2.alpha() << endl;
data <<"pbc_csr.port_7.cfg_account_pg_2.xon_threshold: 0x" << pbc_csr.port_7.cfg_account_pg_2.xon_threshold() << endl;
data <<"pbc_csr.port_7.cfg_account_pg_2.headroom: 0x" << pbc_csr.port_7.cfg_account_pg_2.headroom() << endl;
data <<"pbc_csr.port_7.cfg_account_pg_2.reserved_min: 0x" << pbc_csr.port_7.cfg_account_pg_2.reserved_min() << endl;
data <<"pbc_csr.port_7.cfg_account_pg_2.low_limit: 0x" << pbc_csr.port_7.cfg_account_pg_2.low_limit() << endl;

    pbc_csr.port_7.cfg_account_pg_20.read();
    data <<"pbc_csr.port_7.cfg_account_pg_20.all: 0x" << pbc_csr.port_7.cfg_account_pg_20.all() << endl;
data <<"pbc_csr.port_7.cfg_account_pg_20.alpha: 0x" << pbc_csr.port_7.cfg_account_pg_20.alpha() << endl;
data <<"pbc_csr.port_7.cfg_account_pg_20.xon_threshold: 0x" << pbc_csr.port_7.cfg_account_pg_20.xon_threshold() << endl;
data <<"pbc_csr.port_7.cfg_account_pg_20.headroom: 0x" << pbc_csr.port_7.cfg_account_pg_20.headroom() << endl;
data <<"pbc_csr.port_7.cfg_account_pg_20.reserved_min: 0x" << pbc_csr.port_7.cfg_account_pg_20.reserved_min() << endl;
data <<"pbc_csr.port_7.cfg_account_pg_20.low_limit: 0x" << pbc_csr.port_7.cfg_account_pg_20.low_limit() << endl;

    pbc_csr.port_7.cfg_account_pg_21.read();
    data <<"pbc_csr.port_7.cfg_account_pg_21.all: 0x" << pbc_csr.port_7.cfg_account_pg_21.all() << endl;
data <<"pbc_csr.port_7.cfg_account_pg_21.alpha: 0x" << pbc_csr.port_7.cfg_account_pg_21.alpha() << endl;
data <<"pbc_csr.port_7.cfg_account_pg_21.xon_threshold: 0x" << pbc_csr.port_7.cfg_account_pg_21.xon_threshold() << endl;
data <<"pbc_csr.port_7.cfg_account_pg_21.headroom: 0x" << pbc_csr.port_7.cfg_account_pg_21.headroom() << endl;
data <<"pbc_csr.port_7.cfg_account_pg_21.reserved_min: 0x" << pbc_csr.port_7.cfg_account_pg_21.reserved_min() << endl;
data <<"pbc_csr.port_7.cfg_account_pg_21.low_limit: 0x" << pbc_csr.port_7.cfg_account_pg_21.low_limit() << endl;

    pbc_csr.port_7.cfg_account_pg_22.read();
    data <<"pbc_csr.port_7.cfg_account_pg_22.all: 0x" << pbc_csr.port_7.cfg_account_pg_22.all() << endl;
data <<"pbc_csr.port_7.cfg_account_pg_22.alpha: 0x" << pbc_csr.port_7.cfg_account_pg_22.alpha() << endl;
data <<"pbc_csr.port_7.cfg_account_pg_22.xon_threshold: 0x" << pbc_csr.port_7.cfg_account_pg_22.xon_threshold() << endl;
data <<"pbc_csr.port_7.cfg_account_pg_22.headroom: 0x" << pbc_csr.port_7.cfg_account_pg_22.headroom() << endl;
data <<"pbc_csr.port_7.cfg_account_pg_22.reserved_min: 0x" << pbc_csr.port_7.cfg_account_pg_22.reserved_min() << endl;
data <<"pbc_csr.port_7.cfg_account_pg_22.low_limit: 0x" << pbc_csr.port_7.cfg_account_pg_22.low_limit() << endl;

    pbc_csr.port_7.cfg_account_pg_23.read();
    data <<"pbc_csr.port_7.cfg_account_pg_23.all: 0x" << pbc_csr.port_7.cfg_account_pg_23.all() << endl;
data <<"pbc_csr.port_7.cfg_account_pg_23.alpha: 0x" << pbc_csr.port_7.cfg_account_pg_23.alpha() << endl;
data <<"pbc_csr.port_7.cfg_account_pg_23.xon_threshold: 0x" << pbc_csr.port_7.cfg_account_pg_23.xon_threshold() << endl;
data <<"pbc_csr.port_7.cfg_account_pg_23.headroom: 0x" << pbc_csr.port_7.cfg_account_pg_23.headroom() << endl;
data <<"pbc_csr.port_7.cfg_account_pg_23.reserved_min: 0x" << pbc_csr.port_7.cfg_account_pg_23.reserved_min() << endl;
data <<"pbc_csr.port_7.cfg_account_pg_23.low_limit: 0x" << pbc_csr.port_7.cfg_account_pg_23.low_limit() << endl;

    pbc_csr.port_7.cfg_account_pg_24.read();
    data <<"pbc_csr.port_7.cfg_account_pg_24.all: 0x" << pbc_csr.port_7.cfg_account_pg_24.all() << endl;
data <<"pbc_csr.port_7.cfg_account_pg_24.alpha: 0x" << pbc_csr.port_7.cfg_account_pg_24.alpha() << endl;
data <<"pbc_csr.port_7.cfg_account_pg_24.xon_threshold: 0x" << pbc_csr.port_7.cfg_account_pg_24.xon_threshold() << endl;
data <<"pbc_csr.port_7.cfg_account_pg_24.headroom: 0x" << pbc_csr.port_7.cfg_account_pg_24.headroom() << endl;
data <<"pbc_csr.port_7.cfg_account_pg_24.reserved_min: 0x" << pbc_csr.port_7.cfg_account_pg_24.reserved_min() << endl;
data <<"pbc_csr.port_7.cfg_account_pg_24.low_limit: 0x" << pbc_csr.port_7.cfg_account_pg_24.low_limit() << endl;

    pbc_csr.port_7.cfg_account_pg_25.read();
    data <<"pbc_csr.port_7.cfg_account_pg_25.all: 0x" << pbc_csr.port_7.cfg_account_pg_25.all() << endl;
data <<"pbc_csr.port_7.cfg_account_pg_25.alpha: 0x" << pbc_csr.port_7.cfg_account_pg_25.alpha() << endl;
data <<"pbc_csr.port_7.cfg_account_pg_25.xon_threshold: 0x" << pbc_csr.port_7.cfg_account_pg_25.xon_threshold() << endl;
data <<"pbc_csr.port_7.cfg_account_pg_25.headroom: 0x" << pbc_csr.port_7.cfg_account_pg_25.headroom() << endl;
data <<"pbc_csr.port_7.cfg_account_pg_25.reserved_min: 0x" << pbc_csr.port_7.cfg_account_pg_25.reserved_min() << endl;
data <<"pbc_csr.port_7.cfg_account_pg_25.low_limit: 0x" << pbc_csr.port_7.cfg_account_pg_25.low_limit() << endl;

    pbc_csr.port_7.cfg_account_pg_26.read();
    data <<"pbc_csr.port_7.cfg_account_pg_26.all: 0x" << pbc_csr.port_7.cfg_account_pg_26.all() << endl;
data <<"pbc_csr.port_7.cfg_account_pg_26.alpha: 0x" << pbc_csr.port_7.cfg_account_pg_26.alpha() << endl;
data <<"pbc_csr.port_7.cfg_account_pg_26.xon_threshold: 0x" << pbc_csr.port_7.cfg_account_pg_26.xon_threshold() << endl;
data <<"pbc_csr.port_7.cfg_account_pg_26.headroom: 0x" << pbc_csr.port_7.cfg_account_pg_26.headroom() << endl;
data <<"pbc_csr.port_7.cfg_account_pg_26.reserved_min: 0x" << pbc_csr.port_7.cfg_account_pg_26.reserved_min() << endl;
data <<"pbc_csr.port_7.cfg_account_pg_26.low_limit: 0x" << pbc_csr.port_7.cfg_account_pg_26.low_limit() << endl;

    pbc_csr.port_7.cfg_account_pg_27.read();
    data <<"pbc_csr.port_7.cfg_account_pg_27.all: 0x" << pbc_csr.port_7.cfg_account_pg_27.all() << endl;
data <<"pbc_csr.port_7.cfg_account_pg_27.alpha: 0x" << pbc_csr.port_7.cfg_account_pg_27.alpha() << endl;
data <<"pbc_csr.port_7.cfg_account_pg_27.xon_threshold: 0x" << pbc_csr.port_7.cfg_account_pg_27.xon_threshold() << endl;
data <<"pbc_csr.port_7.cfg_account_pg_27.headroom: 0x" << pbc_csr.port_7.cfg_account_pg_27.headroom() << endl;
data <<"pbc_csr.port_7.cfg_account_pg_27.reserved_min: 0x" << pbc_csr.port_7.cfg_account_pg_27.reserved_min() << endl;
data <<"pbc_csr.port_7.cfg_account_pg_27.low_limit: 0x" << pbc_csr.port_7.cfg_account_pg_27.low_limit() << endl;

    pbc_csr.port_7.cfg_account_pg_28.read();
    data <<"pbc_csr.port_7.cfg_account_pg_28.all: 0x" << pbc_csr.port_7.cfg_account_pg_28.all() << endl;
data <<"pbc_csr.port_7.cfg_account_pg_28.alpha: 0x" << pbc_csr.port_7.cfg_account_pg_28.alpha() << endl;
data <<"pbc_csr.port_7.cfg_account_pg_28.xon_threshold: 0x" << pbc_csr.port_7.cfg_account_pg_28.xon_threshold() << endl;
data <<"pbc_csr.port_7.cfg_account_pg_28.headroom: 0x" << pbc_csr.port_7.cfg_account_pg_28.headroom() << endl;
data <<"pbc_csr.port_7.cfg_account_pg_28.reserved_min: 0x" << pbc_csr.port_7.cfg_account_pg_28.reserved_min() << endl;
data <<"pbc_csr.port_7.cfg_account_pg_28.low_limit: 0x" << pbc_csr.port_7.cfg_account_pg_28.low_limit() << endl;

    pbc_csr.port_7.cfg_account_pg_29.read();
    data <<"pbc_csr.port_7.cfg_account_pg_29.all: 0x" << pbc_csr.port_7.cfg_account_pg_29.all() << endl;
data <<"pbc_csr.port_7.cfg_account_pg_29.alpha: 0x" << pbc_csr.port_7.cfg_account_pg_29.alpha() << endl;
data <<"pbc_csr.port_7.cfg_account_pg_29.xon_threshold: 0x" << pbc_csr.port_7.cfg_account_pg_29.xon_threshold() << endl;
data <<"pbc_csr.port_7.cfg_account_pg_29.headroom: 0x" << pbc_csr.port_7.cfg_account_pg_29.headroom() << endl;
data <<"pbc_csr.port_7.cfg_account_pg_29.reserved_min: 0x" << pbc_csr.port_7.cfg_account_pg_29.reserved_min() << endl;
data <<"pbc_csr.port_7.cfg_account_pg_29.low_limit: 0x" << pbc_csr.port_7.cfg_account_pg_29.low_limit() << endl;

    pbc_csr.port_7.cfg_account_pg_3.read();
    data <<"pbc_csr.port_7.cfg_account_pg_3.all: 0x" << pbc_csr.port_7.cfg_account_pg_3.all() << endl;
data <<"pbc_csr.port_7.cfg_account_pg_3.alpha: 0x" << pbc_csr.port_7.cfg_account_pg_3.alpha() << endl;
data <<"pbc_csr.port_7.cfg_account_pg_3.xon_threshold: 0x" << pbc_csr.port_7.cfg_account_pg_3.xon_threshold() << endl;
data <<"pbc_csr.port_7.cfg_account_pg_3.headroom: 0x" << pbc_csr.port_7.cfg_account_pg_3.headroom() << endl;
data <<"pbc_csr.port_7.cfg_account_pg_3.reserved_min: 0x" << pbc_csr.port_7.cfg_account_pg_3.reserved_min() << endl;
data <<"pbc_csr.port_7.cfg_account_pg_3.low_limit: 0x" << pbc_csr.port_7.cfg_account_pg_3.low_limit() << endl;

    pbc_csr.port_7.cfg_account_pg_30.read();
    data <<"pbc_csr.port_7.cfg_account_pg_30.all: 0x" << pbc_csr.port_7.cfg_account_pg_30.all() << endl;
data <<"pbc_csr.port_7.cfg_account_pg_30.alpha: 0x" << pbc_csr.port_7.cfg_account_pg_30.alpha() << endl;
data <<"pbc_csr.port_7.cfg_account_pg_30.xon_threshold: 0x" << pbc_csr.port_7.cfg_account_pg_30.xon_threshold() << endl;
data <<"pbc_csr.port_7.cfg_account_pg_30.headroom: 0x" << pbc_csr.port_7.cfg_account_pg_30.headroom() << endl;
data <<"pbc_csr.port_7.cfg_account_pg_30.reserved_min: 0x" << pbc_csr.port_7.cfg_account_pg_30.reserved_min() << endl;
data <<"pbc_csr.port_7.cfg_account_pg_30.low_limit: 0x" << pbc_csr.port_7.cfg_account_pg_30.low_limit() << endl;

    pbc_csr.port_7.cfg_account_pg_31.read();
    data <<"pbc_csr.port_7.cfg_account_pg_31.all: 0x" << pbc_csr.port_7.cfg_account_pg_31.all() << endl;
data <<"pbc_csr.port_7.cfg_account_pg_31.alpha: 0x" << pbc_csr.port_7.cfg_account_pg_31.alpha() << endl;
data <<"pbc_csr.port_7.cfg_account_pg_31.xon_threshold: 0x" << pbc_csr.port_7.cfg_account_pg_31.xon_threshold() << endl;
data <<"pbc_csr.port_7.cfg_account_pg_31.headroom: 0x" << pbc_csr.port_7.cfg_account_pg_31.headroom() << endl;
data <<"pbc_csr.port_7.cfg_account_pg_31.reserved_min: 0x" << pbc_csr.port_7.cfg_account_pg_31.reserved_min() << endl;
data <<"pbc_csr.port_7.cfg_account_pg_31.low_limit: 0x" << pbc_csr.port_7.cfg_account_pg_31.low_limit() << endl;

    pbc_csr.port_7.cfg_account_pg_4.read();
    data <<"pbc_csr.port_7.cfg_account_pg_4.all: 0x" << pbc_csr.port_7.cfg_account_pg_4.all() << endl;
data <<"pbc_csr.port_7.cfg_account_pg_4.alpha: 0x" << pbc_csr.port_7.cfg_account_pg_4.alpha() << endl;
data <<"pbc_csr.port_7.cfg_account_pg_4.xon_threshold: 0x" << pbc_csr.port_7.cfg_account_pg_4.xon_threshold() << endl;
data <<"pbc_csr.port_7.cfg_account_pg_4.headroom: 0x" << pbc_csr.port_7.cfg_account_pg_4.headroom() << endl;
data <<"pbc_csr.port_7.cfg_account_pg_4.reserved_min: 0x" << pbc_csr.port_7.cfg_account_pg_4.reserved_min() << endl;
data <<"pbc_csr.port_7.cfg_account_pg_4.low_limit: 0x" << pbc_csr.port_7.cfg_account_pg_4.low_limit() << endl;

    pbc_csr.port_7.cfg_account_pg_5.read();
    data <<"pbc_csr.port_7.cfg_account_pg_5.all: 0x" << pbc_csr.port_7.cfg_account_pg_5.all() << endl;
data <<"pbc_csr.port_7.cfg_account_pg_5.alpha: 0x" << pbc_csr.port_7.cfg_account_pg_5.alpha() << endl;
data <<"pbc_csr.port_7.cfg_account_pg_5.xon_threshold: 0x" << pbc_csr.port_7.cfg_account_pg_5.xon_threshold() << endl;
data <<"pbc_csr.port_7.cfg_account_pg_5.headroom: 0x" << pbc_csr.port_7.cfg_account_pg_5.headroom() << endl;
data <<"pbc_csr.port_7.cfg_account_pg_5.reserved_min: 0x" << pbc_csr.port_7.cfg_account_pg_5.reserved_min() << endl;
data <<"pbc_csr.port_7.cfg_account_pg_5.low_limit: 0x" << pbc_csr.port_7.cfg_account_pg_5.low_limit() << endl;

    pbc_csr.port_7.cfg_account_pg_6.read();
    data <<"pbc_csr.port_7.cfg_account_pg_6.all: 0x" << pbc_csr.port_7.cfg_account_pg_6.all() << endl;
data <<"pbc_csr.port_7.cfg_account_pg_6.alpha: 0x" << pbc_csr.port_7.cfg_account_pg_6.alpha() << endl;
data <<"pbc_csr.port_7.cfg_account_pg_6.xon_threshold: 0x" << pbc_csr.port_7.cfg_account_pg_6.xon_threshold() << endl;
data <<"pbc_csr.port_7.cfg_account_pg_6.headroom: 0x" << pbc_csr.port_7.cfg_account_pg_6.headroom() << endl;
data <<"pbc_csr.port_7.cfg_account_pg_6.reserved_min: 0x" << pbc_csr.port_7.cfg_account_pg_6.reserved_min() << endl;
data <<"pbc_csr.port_7.cfg_account_pg_6.low_limit: 0x" << pbc_csr.port_7.cfg_account_pg_6.low_limit() << endl;

    pbc_csr.port_7.cfg_account_pg_7.read();
    data <<"pbc_csr.port_7.cfg_account_pg_7.all: 0x" << pbc_csr.port_7.cfg_account_pg_7.all() << endl;
data <<"pbc_csr.port_7.cfg_account_pg_7.alpha: 0x" << pbc_csr.port_7.cfg_account_pg_7.alpha() << endl;
data <<"pbc_csr.port_7.cfg_account_pg_7.xon_threshold: 0x" << pbc_csr.port_7.cfg_account_pg_7.xon_threshold() << endl;
data <<"pbc_csr.port_7.cfg_account_pg_7.headroom: 0x" << pbc_csr.port_7.cfg_account_pg_7.headroom() << endl;
data <<"pbc_csr.port_7.cfg_account_pg_7.reserved_min: 0x" << pbc_csr.port_7.cfg_account_pg_7.reserved_min() << endl;
data <<"pbc_csr.port_7.cfg_account_pg_7.low_limit: 0x" << pbc_csr.port_7.cfg_account_pg_7.low_limit() << endl;

    pbc_csr.port_7.cfg_account_pg_8.read();
    data <<"pbc_csr.port_7.cfg_account_pg_8.all: 0x" << pbc_csr.port_7.cfg_account_pg_8.all() << endl;
data <<"pbc_csr.port_7.cfg_account_pg_8.alpha: 0x" << pbc_csr.port_7.cfg_account_pg_8.alpha() << endl;
data <<"pbc_csr.port_7.cfg_account_pg_8.xon_threshold: 0x" << pbc_csr.port_7.cfg_account_pg_8.xon_threshold() << endl;
data <<"pbc_csr.port_7.cfg_account_pg_8.headroom: 0x" << pbc_csr.port_7.cfg_account_pg_8.headroom() << endl;
data <<"pbc_csr.port_7.cfg_account_pg_8.reserved_min: 0x" << pbc_csr.port_7.cfg_account_pg_8.reserved_min() << endl;
data <<"pbc_csr.port_7.cfg_account_pg_8.low_limit: 0x" << pbc_csr.port_7.cfg_account_pg_8.low_limit() << endl;

    pbc_csr.port_7.cfg_account_pg_9.read();
    data <<"pbc_csr.port_7.cfg_account_pg_9.all: 0x" << pbc_csr.port_7.cfg_account_pg_9.all() << endl;
data <<"pbc_csr.port_7.cfg_account_pg_9.alpha: 0x" << pbc_csr.port_7.cfg_account_pg_9.alpha() << endl;
data <<"pbc_csr.port_7.cfg_account_pg_9.xon_threshold: 0x" << pbc_csr.port_7.cfg_account_pg_9.xon_threshold() << endl;
data <<"pbc_csr.port_7.cfg_account_pg_9.headroom: 0x" << pbc_csr.port_7.cfg_account_pg_9.headroom() << endl;
data <<"pbc_csr.port_7.cfg_account_pg_9.reserved_min: 0x" << pbc_csr.port_7.cfg_account_pg_9.reserved_min() << endl;
data <<"pbc_csr.port_7.cfg_account_pg_9.low_limit: 0x" << pbc_csr.port_7.cfg_account_pg_9.low_limit() << endl;

    pbc_csr.port_7.cfg_account_pg_to_sp.read();
    data <<"pbc_csr.port_7.cfg_account_pg_to_sp.all: 0x" << pbc_csr.port_7.cfg_account_pg_to_sp.all() << endl;
data <<"pbc_csr.port_7.cfg_account_pg_to_sp.map: 0x" << pbc_csr.port_7.cfg_account_pg_to_sp.map() << endl;

    pbc_csr.port_7.cfg_account_sp_fetch_quanta.read();
    data <<"pbc_csr.port_7.cfg_account_sp_fetch_quanta.all: 0x" << pbc_csr.port_7.cfg_account_sp_fetch_quanta.all() << endl;
data <<"pbc_csr.port_7.cfg_account_sp_fetch_quanta.pg30: 0x" << pbc_csr.port_7.cfg_account_sp_fetch_quanta.pg30() << endl;
data <<"pbc_csr.port_7.cfg_account_sp_fetch_quanta.pg31: 0x" << pbc_csr.port_7.cfg_account_sp_fetch_quanta.pg31() << endl;
data <<"pbc_csr.port_7.cfg_account_sp_fetch_quanta.pg18: 0x" << pbc_csr.port_7.cfg_account_sp_fetch_quanta.pg18() << endl;
data <<"pbc_csr.port_7.cfg_account_sp_fetch_quanta.pg19: 0x" << pbc_csr.port_7.cfg_account_sp_fetch_quanta.pg19() << endl;
data <<"pbc_csr.port_7.cfg_account_sp_fetch_quanta.pg10: 0x" << pbc_csr.port_7.cfg_account_sp_fetch_quanta.pg10() << endl;
data <<"pbc_csr.port_7.cfg_account_sp_fetch_quanta.pg11: 0x" << pbc_csr.port_7.cfg_account_sp_fetch_quanta.pg11() << endl;
data <<"pbc_csr.port_7.cfg_account_sp_fetch_quanta.pg12: 0x" << pbc_csr.port_7.cfg_account_sp_fetch_quanta.pg12() << endl;
data <<"pbc_csr.port_7.cfg_account_sp_fetch_quanta.pg13: 0x" << pbc_csr.port_7.cfg_account_sp_fetch_quanta.pg13() << endl;
data <<"pbc_csr.port_7.cfg_account_sp_fetch_quanta.pg14: 0x" << pbc_csr.port_7.cfg_account_sp_fetch_quanta.pg14() << endl;
data <<"pbc_csr.port_7.cfg_account_sp_fetch_quanta.pg15: 0x" << pbc_csr.port_7.cfg_account_sp_fetch_quanta.pg15() << endl;
data <<"pbc_csr.port_7.cfg_account_sp_fetch_quanta.pg16: 0x" << pbc_csr.port_7.cfg_account_sp_fetch_quanta.pg16() << endl;
data <<"pbc_csr.port_7.cfg_account_sp_fetch_quanta.pg17: 0x" << pbc_csr.port_7.cfg_account_sp_fetch_quanta.pg17() << endl;
data <<"pbc_csr.port_7.cfg_account_sp_fetch_quanta.pg6: 0x" << pbc_csr.port_7.cfg_account_sp_fetch_quanta.pg6() << endl;
data <<"pbc_csr.port_7.cfg_account_sp_fetch_quanta.pg7: 0x" << pbc_csr.port_7.cfg_account_sp_fetch_quanta.pg7() << endl;
data <<"pbc_csr.port_7.cfg_account_sp_fetch_quanta.pg4: 0x" << pbc_csr.port_7.cfg_account_sp_fetch_quanta.pg4() << endl;
data <<"pbc_csr.port_7.cfg_account_sp_fetch_quanta.pg5: 0x" << pbc_csr.port_7.cfg_account_sp_fetch_quanta.pg5() << endl;
data <<"pbc_csr.port_7.cfg_account_sp_fetch_quanta.pg2: 0x" << pbc_csr.port_7.cfg_account_sp_fetch_quanta.pg2() << endl;
data <<"pbc_csr.port_7.cfg_account_sp_fetch_quanta.pg3: 0x" << pbc_csr.port_7.cfg_account_sp_fetch_quanta.pg3() << endl;
data <<"pbc_csr.port_7.cfg_account_sp_fetch_quanta.pg0: 0x" << pbc_csr.port_7.cfg_account_sp_fetch_quanta.pg0() << endl;
data <<"pbc_csr.port_7.cfg_account_sp_fetch_quanta.pg1: 0x" << pbc_csr.port_7.cfg_account_sp_fetch_quanta.pg1() << endl;
data <<"pbc_csr.port_7.cfg_account_sp_fetch_quanta.pg8: 0x" << pbc_csr.port_7.cfg_account_sp_fetch_quanta.pg8() << endl;
data <<"pbc_csr.port_7.cfg_account_sp_fetch_quanta.pg9: 0x" << pbc_csr.port_7.cfg_account_sp_fetch_quanta.pg9() << endl;
data <<"pbc_csr.port_7.cfg_account_sp_fetch_quanta.pg21: 0x" << pbc_csr.port_7.cfg_account_sp_fetch_quanta.pg21() << endl;
data <<"pbc_csr.port_7.cfg_account_sp_fetch_quanta.pg20: 0x" << pbc_csr.port_7.cfg_account_sp_fetch_quanta.pg20() << endl;
data <<"pbc_csr.port_7.cfg_account_sp_fetch_quanta.pg23: 0x" << pbc_csr.port_7.cfg_account_sp_fetch_quanta.pg23() << endl;
data <<"pbc_csr.port_7.cfg_account_sp_fetch_quanta.pg22: 0x" << pbc_csr.port_7.cfg_account_sp_fetch_quanta.pg22() << endl;
data <<"pbc_csr.port_7.cfg_account_sp_fetch_quanta.pg25: 0x" << pbc_csr.port_7.cfg_account_sp_fetch_quanta.pg25() << endl;
data <<"pbc_csr.port_7.cfg_account_sp_fetch_quanta.pg24: 0x" << pbc_csr.port_7.cfg_account_sp_fetch_quanta.pg24() << endl;
data <<"pbc_csr.port_7.cfg_account_sp_fetch_quanta.pg27: 0x" << pbc_csr.port_7.cfg_account_sp_fetch_quanta.pg27() << endl;
data <<"pbc_csr.port_7.cfg_account_sp_fetch_quanta.pg26: 0x" << pbc_csr.port_7.cfg_account_sp_fetch_quanta.pg26() << endl;
data <<"pbc_csr.port_7.cfg_account_sp_fetch_quanta.pg29: 0x" << pbc_csr.port_7.cfg_account_sp_fetch_quanta.pg29() << endl;
data <<"pbc_csr.port_7.cfg_account_sp_fetch_quanta.pg28: 0x" << pbc_csr.port_7.cfg_account_sp_fetch_quanta.pg28() << endl;

    pbc_csr.port_7.cfg_account_sp_fetch_threshold.read();
    data <<"pbc_csr.port_7.cfg_account_sp_fetch_threshold.all: 0x" << pbc_csr.port_7.cfg_account_sp_fetch_threshold.all() << endl;
data <<"pbc_csr.port_7.cfg_account_sp_fetch_threshold.pg30: 0x" << pbc_csr.port_7.cfg_account_sp_fetch_threshold.pg30() << endl;
data <<"pbc_csr.port_7.cfg_account_sp_fetch_threshold.pg31: 0x" << pbc_csr.port_7.cfg_account_sp_fetch_threshold.pg31() << endl;
data <<"pbc_csr.port_7.cfg_account_sp_fetch_threshold.pg18: 0x" << pbc_csr.port_7.cfg_account_sp_fetch_threshold.pg18() << endl;
data <<"pbc_csr.port_7.cfg_account_sp_fetch_threshold.pg19: 0x" << pbc_csr.port_7.cfg_account_sp_fetch_threshold.pg19() << endl;
data <<"pbc_csr.port_7.cfg_account_sp_fetch_threshold.pg10: 0x" << pbc_csr.port_7.cfg_account_sp_fetch_threshold.pg10() << endl;
data <<"pbc_csr.port_7.cfg_account_sp_fetch_threshold.pg11: 0x" << pbc_csr.port_7.cfg_account_sp_fetch_threshold.pg11() << endl;
data <<"pbc_csr.port_7.cfg_account_sp_fetch_threshold.pg12: 0x" << pbc_csr.port_7.cfg_account_sp_fetch_threshold.pg12() << endl;
data <<"pbc_csr.port_7.cfg_account_sp_fetch_threshold.pg13: 0x" << pbc_csr.port_7.cfg_account_sp_fetch_threshold.pg13() << endl;
data <<"pbc_csr.port_7.cfg_account_sp_fetch_threshold.pg14: 0x" << pbc_csr.port_7.cfg_account_sp_fetch_threshold.pg14() << endl;
data <<"pbc_csr.port_7.cfg_account_sp_fetch_threshold.pg15: 0x" << pbc_csr.port_7.cfg_account_sp_fetch_threshold.pg15() << endl;
data <<"pbc_csr.port_7.cfg_account_sp_fetch_threshold.pg16: 0x" << pbc_csr.port_7.cfg_account_sp_fetch_threshold.pg16() << endl;
data <<"pbc_csr.port_7.cfg_account_sp_fetch_threshold.pg17: 0x" << pbc_csr.port_7.cfg_account_sp_fetch_threshold.pg17() << endl;
data <<"pbc_csr.port_7.cfg_account_sp_fetch_threshold.pg6: 0x" << pbc_csr.port_7.cfg_account_sp_fetch_threshold.pg6() << endl;
data <<"pbc_csr.port_7.cfg_account_sp_fetch_threshold.pg7: 0x" << pbc_csr.port_7.cfg_account_sp_fetch_threshold.pg7() << endl;
data <<"pbc_csr.port_7.cfg_account_sp_fetch_threshold.pg4: 0x" << pbc_csr.port_7.cfg_account_sp_fetch_threshold.pg4() << endl;
data <<"pbc_csr.port_7.cfg_account_sp_fetch_threshold.pg5: 0x" << pbc_csr.port_7.cfg_account_sp_fetch_threshold.pg5() << endl;
data <<"pbc_csr.port_7.cfg_account_sp_fetch_threshold.pg2: 0x" << pbc_csr.port_7.cfg_account_sp_fetch_threshold.pg2() << endl;
data <<"pbc_csr.port_7.cfg_account_sp_fetch_threshold.pg3: 0x" << pbc_csr.port_7.cfg_account_sp_fetch_threshold.pg3() << endl;
data <<"pbc_csr.port_7.cfg_account_sp_fetch_threshold.pg0: 0x" << pbc_csr.port_7.cfg_account_sp_fetch_threshold.pg0() << endl;
data <<"pbc_csr.port_7.cfg_account_sp_fetch_threshold.pg1: 0x" << pbc_csr.port_7.cfg_account_sp_fetch_threshold.pg1() << endl;
data <<"pbc_csr.port_7.cfg_account_sp_fetch_threshold.pg8: 0x" << pbc_csr.port_7.cfg_account_sp_fetch_threshold.pg8() << endl;
data <<"pbc_csr.port_7.cfg_account_sp_fetch_threshold.pg9: 0x" << pbc_csr.port_7.cfg_account_sp_fetch_threshold.pg9() << endl;
data <<"pbc_csr.port_7.cfg_account_sp_fetch_threshold.pg21: 0x" << pbc_csr.port_7.cfg_account_sp_fetch_threshold.pg21() << endl;
data <<"pbc_csr.port_7.cfg_account_sp_fetch_threshold.pg20: 0x" << pbc_csr.port_7.cfg_account_sp_fetch_threshold.pg20() << endl;
data <<"pbc_csr.port_7.cfg_account_sp_fetch_threshold.pg23: 0x" << pbc_csr.port_7.cfg_account_sp_fetch_threshold.pg23() << endl;
data <<"pbc_csr.port_7.cfg_account_sp_fetch_threshold.pg22: 0x" << pbc_csr.port_7.cfg_account_sp_fetch_threshold.pg22() << endl;
data <<"pbc_csr.port_7.cfg_account_sp_fetch_threshold.pg25: 0x" << pbc_csr.port_7.cfg_account_sp_fetch_threshold.pg25() << endl;
data <<"pbc_csr.port_7.cfg_account_sp_fetch_threshold.pg24: 0x" << pbc_csr.port_7.cfg_account_sp_fetch_threshold.pg24() << endl;
data <<"pbc_csr.port_7.cfg_account_sp_fetch_threshold.pg27: 0x" << pbc_csr.port_7.cfg_account_sp_fetch_threshold.pg27() << endl;
data <<"pbc_csr.port_7.cfg_account_sp_fetch_threshold.pg26: 0x" << pbc_csr.port_7.cfg_account_sp_fetch_threshold.pg26() << endl;
data <<"pbc_csr.port_7.cfg_account_sp_fetch_threshold.pg29: 0x" << pbc_csr.port_7.cfg_account_sp_fetch_threshold.pg29() << endl;
data <<"pbc_csr.port_7.cfg_account_sp_fetch_threshold.pg28: 0x" << pbc_csr.port_7.cfg_account_sp_fetch_threshold.pg28() << endl;

    pbc_csr.port_7.cfg_account_tc_to_pg.read();
    data <<"pbc_csr.port_7.cfg_account_tc_to_pg.all: 0x" << pbc_csr.port_7.cfg_account_tc_to_pg.all() << endl;
data <<"pbc_csr.port_7.cfg_account_tc_to_pg.table: 0x" << pbc_csr.port_7.cfg_account_tc_to_pg.table() << endl;

    pbc_csr.port_7.cfg_latency_cache.read();
    data <<"pbc_csr.port_7.cfg_latency_cache.all: 0x" << pbc_csr.port_7.cfg_latency_cache.all() << endl;
data <<"pbc_csr.port_7.cfg_latency_cache.enable: 0x" << pbc_csr.port_7.cfg_latency_cache.enable() << endl;

    pbc_csr.port_7.cfg_oq.read();
    data <<"pbc_csr.port_7.cfg_oq.all: 0x" << pbc_csr.port_7.cfg_oq.all() << endl;
data <<"pbc_csr.port_7.cfg_oq.enable: 0x" << pbc_csr.port_7.cfg_oq.enable() << endl;
data <<"pbc_csr.port_7.cfg_oq.ecc_disable_cor: 0x" << pbc_csr.port_7.cfg_oq.ecc_disable_cor() << endl;
data <<"pbc_csr.port_7.cfg_oq.span_cpu_num_cells: 0x" << pbc_csr.port_7.cfg_oq.span_cpu_num_cells() << endl;
data <<"pbc_csr.port_7.cfg_oq.packing_enable: 0x" << pbc_csr.port_7.cfg_oq.packing_enable() << endl;
data <<"pbc_csr.port_7.cfg_oq.dhs_eccbypass: 0x" << pbc_csr.port_7.cfg_oq.dhs_eccbypass() << endl;
data <<"pbc_csr.port_7.cfg_oq.ipg_bytes: 0x" << pbc_csr.port_7.cfg_oq.ipg_bytes() << endl;
data <<"pbc_csr.port_7.cfg_oq.bist_run: 0x" << pbc_csr.port_7.cfg_oq.bist_run() << endl;
data <<"pbc_csr.port_7.cfg_oq.flow_control_enable_xoff: 0x" << pbc_csr.port_7.cfg_oq.flow_control_enable_xoff() << endl;
data <<"pbc_csr.port_7.cfg_oq.ecc_disable_det: 0x" << pbc_csr.port_7.cfg_oq.ecc_disable_det() << endl;
data <<"pbc_csr.port_7.cfg_oq.packing_msb: 0x" << pbc_csr.port_7.cfg_oq.packing_msb() << endl;
data <<"pbc_csr.port_7.cfg_oq.replication_enable: 0x" << pbc_csr.port_7.cfg_oq.replication_enable() << endl;
data <<"pbc_csr.port_7.cfg_oq.flush: 0x" << pbc_csr.port_7.cfg_oq.flush() << endl;
data <<"pbc_csr.port_7.cfg_oq.flow_control_enable_credits: 0x" << pbc_csr.port_7.cfg_oq.flow_control_enable_credits() << endl;
data <<"pbc_csr.port_7.cfg_oq.mux_bypass: 0x" << pbc_csr.port_7.cfg_oq.mux_bypass() << endl;
data <<"pbc_csr.port_7.cfg_oq.rewrite_enable: 0x" << pbc_csr.port_7.cfg_oq.rewrite_enable() << endl;
data <<"pbc_csr.port_7.cfg_oq.dhs_selection: 0x" << pbc_csr.port_7.cfg_oq.dhs_selection() << endl;

    pbc_csr.port_7.cfg_oq_arb_l1_selection.read();
    data <<"pbc_csr.port_7.cfg_oq_arb_l1_selection.all: 0x" << pbc_csr.port_7.cfg_oq_arb_l1_selection.all() << endl;
data <<"pbc_csr.port_7.cfg_oq_arb_l1_selection.node_13: 0x" << pbc_csr.port_7.cfg_oq_arb_l1_selection.node_13() << endl;
data <<"pbc_csr.port_7.cfg_oq_arb_l1_selection.node_14: 0x" << pbc_csr.port_7.cfg_oq_arb_l1_selection.node_14() << endl;
data <<"pbc_csr.port_7.cfg_oq_arb_l1_selection.node_9: 0x" << pbc_csr.port_7.cfg_oq_arb_l1_selection.node_9() << endl;
data <<"pbc_csr.port_7.cfg_oq_arb_l1_selection.node_8: 0x" << pbc_csr.port_7.cfg_oq_arb_l1_selection.node_8() << endl;
data <<"pbc_csr.port_7.cfg_oq_arb_l1_selection.node_7: 0x" << pbc_csr.port_7.cfg_oq_arb_l1_selection.node_7() << endl;
data <<"pbc_csr.port_7.cfg_oq_arb_l1_selection.node_6: 0x" << pbc_csr.port_7.cfg_oq_arb_l1_selection.node_6() << endl;
data <<"pbc_csr.port_7.cfg_oq_arb_l1_selection.node_5: 0x" << pbc_csr.port_7.cfg_oq_arb_l1_selection.node_5() << endl;
data <<"pbc_csr.port_7.cfg_oq_arb_l1_selection.node_4: 0x" << pbc_csr.port_7.cfg_oq_arb_l1_selection.node_4() << endl;
data <<"pbc_csr.port_7.cfg_oq_arb_l1_selection.node_3: 0x" << pbc_csr.port_7.cfg_oq_arb_l1_selection.node_3() << endl;
data <<"pbc_csr.port_7.cfg_oq_arb_l1_selection.node_2: 0x" << pbc_csr.port_7.cfg_oq_arb_l1_selection.node_2() << endl;
data <<"pbc_csr.port_7.cfg_oq_arb_l1_selection.node_1: 0x" << pbc_csr.port_7.cfg_oq_arb_l1_selection.node_1() << endl;
data <<"pbc_csr.port_7.cfg_oq_arb_l1_selection.node_0: 0x" << pbc_csr.port_7.cfg_oq_arb_l1_selection.node_0() << endl;
data <<"pbc_csr.port_7.cfg_oq_arb_l1_selection.node_11: 0x" << pbc_csr.port_7.cfg_oq_arb_l1_selection.node_11() << endl;
data <<"pbc_csr.port_7.cfg_oq_arb_l1_selection.node_10: 0x" << pbc_csr.port_7.cfg_oq_arb_l1_selection.node_10() << endl;
data <<"pbc_csr.port_7.cfg_oq_arb_l1_selection.node_15: 0x" << pbc_csr.port_7.cfg_oq_arb_l1_selection.node_15() << endl;
data <<"pbc_csr.port_7.cfg_oq_arb_l1_selection.node_12: 0x" << pbc_csr.port_7.cfg_oq_arb_l1_selection.node_12() << endl;

    pbc_csr.port_7.cfg_oq_arb_l1_strict.read();
    data <<"pbc_csr.port_7.cfg_oq_arb_l1_strict.all: 0x" << pbc_csr.port_7.cfg_oq_arb_l1_strict.all() << endl;
data <<"pbc_csr.port_7.cfg_oq_arb_l1_strict.priority: 0x" << pbc_csr.port_7.cfg_oq_arb_l1_strict.priority() << endl;
data <<"pbc_csr.port_7.cfg_oq_arb_l1_strict.priority_bypass_timer: 0x" << pbc_csr.port_7.cfg_oq_arb_l1_strict.priority_bypass_timer() << endl;

    pbc_csr.port_7.cfg_oq_arb_l2_selection.read();
    data <<"pbc_csr.port_7.cfg_oq_arb_l2_selection.all: 0x" << pbc_csr.port_7.cfg_oq_arb_l2_selection.all() << endl;
data <<"pbc_csr.port_7.cfg_oq_arb_l2_selection.node_3: 0x" << pbc_csr.port_7.cfg_oq_arb_l2_selection.node_3() << endl;
data <<"pbc_csr.port_7.cfg_oq_arb_l2_selection.node_2: 0x" << pbc_csr.port_7.cfg_oq_arb_l2_selection.node_2() << endl;
data <<"pbc_csr.port_7.cfg_oq_arb_l2_selection.node_1: 0x" << pbc_csr.port_7.cfg_oq_arb_l2_selection.node_1() << endl;
data <<"pbc_csr.port_7.cfg_oq_arb_l2_selection.node_0: 0x" << pbc_csr.port_7.cfg_oq_arb_l2_selection.node_0() << endl;

    pbc_csr.port_7.cfg_oq_arb_l2_strict.read();
    data <<"pbc_csr.port_7.cfg_oq_arb_l2_strict.all: 0x" << pbc_csr.port_7.cfg_oq_arb_l2_strict.all() << endl;
data <<"pbc_csr.port_7.cfg_oq_arb_l2_strict.priority: 0x" << pbc_csr.port_7.cfg_oq_arb_l2_strict.priority() << endl;
data <<"pbc_csr.port_7.cfg_oq_arb_l2_strict.priority_bypass_timer: 0x" << pbc_csr.port_7.cfg_oq_arb_l2_strict.priority_bypass_timer() << endl;

    pbc_csr.port_7.cfg_oq_arb_l3_selection.read();
    data <<"pbc_csr.port_7.cfg_oq_arb_l3_selection.all: 0x" << pbc_csr.port_7.cfg_oq_arb_l3_selection.all() << endl;
data <<"pbc_csr.port_7.cfg_oq_arb_l3_selection.node_0: 0x" << pbc_csr.port_7.cfg_oq_arb_l3_selection.node_0() << endl;

    pbc_csr.port_7.cfg_oq_arb_l3_strict.read();
    data <<"pbc_csr.port_7.cfg_oq_arb_l3_strict.all: 0x" << pbc_csr.port_7.cfg_oq_arb_l3_strict.all() << endl;
data <<"pbc_csr.port_7.cfg_oq_arb_l3_strict.priority: 0x" << pbc_csr.port_7.cfg_oq_arb_l3_strict.priority() << endl;
data <<"pbc_csr.port_7.cfg_oq_arb_l3_strict.priority_bypass_timer: 0x" << pbc_csr.port_7.cfg_oq_arb_l3_strict.priority_bypass_timer() << endl;

    pbc_csr.port_7.cfg_oq_dhs_mem.read();
    data <<"pbc_csr.port_7.cfg_oq_dhs_mem.all: 0x" << pbc_csr.port_7.cfg_oq_dhs_mem.all() << endl;
data <<"pbc_csr.port_7.cfg_oq_dhs_mem.address: 0x" << pbc_csr.port_7.cfg_oq_dhs_mem.address() << endl;

    pbc_csr.port_7.cfg_oq_queue.read();
    data <<"pbc_csr.port_7.cfg_oq_queue.all: 0x" << pbc_csr.port_7.cfg_oq_queue.all() << endl;
data <<"pbc_csr.port_7.cfg_oq_queue.recirc: 0x" << pbc_csr.port_7.cfg_oq_queue.recirc() << endl;
data <<"pbc_csr.port_7.cfg_oq_queue.enable: 0x" << pbc_csr.port_7.cfg_oq_queue.enable() << endl;
data <<"pbc_csr.port_7.cfg_oq_queue.flush: 0x" << pbc_csr.port_7.cfg_oq_queue.flush() << endl;

    pbc_csr.port_7.cfg_oq_queue_dynamic.read();
    data <<"pbc_csr.port_7.cfg_oq_queue_dynamic.all: 0x" << pbc_csr.port_7.cfg_oq_queue_dynamic.all() << endl;
data <<"pbc_csr.port_7.cfg_oq_queue_dynamic.threshold: 0x" << pbc_csr.port_7.cfg_oq_queue_dynamic.threshold() << endl;

    pbc_csr.port_7.cfg_oq_queue_write_one.read();
    data <<"pbc_csr.port_7.cfg_oq_queue_write_one.all: 0x" << pbc_csr.port_7.cfg_oq_queue_write_one.all() << endl;
data <<"pbc_csr.port_7.cfg_oq_queue_write_one.enable: 0x" << pbc_csr.port_7.cfg_oq_queue_write_one.enable() << endl;
data <<"pbc_csr.port_7.cfg_oq_queue_write_one.disable: 0x" << pbc_csr.port_7.cfg_oq_queue_write_one.disable() << endl;

    pbc_csr.port_7.cfg_oq_shared_pool.read();
    data <<"pbc_csr.port_7.cfg_oq_shared_pool.all: 0x" << pbc_csr.port_7.cfg_oq_shared_pool.all() << endl;
data <<"pbc_csr.port_7.cfg_oq_shared_pool.max_decrement: 0x" << pbc_csr.port_7.cfg_oq_shared_pool.max_decrement() << endl;
data <<"pbc_csr.port_7.cfg_oq_shared_pool.alpha_1: 0x" << pbc_csr.port_7.cfg_oq_shared_pool.alpha_1() << endl;
data <<"pbc_csr.port_7.cfg_oq_shared_pool.alpha_0: 0x" << pbc_csr.port_7.cfg_oq_shared_pool.alpha_0() << endl;
data <<"pbc_csr.port_7.cfg_oq_shared_pool.credits_0: 0x" << pbc_csr.port_7.cfg_oq_shared_pool.credits_0() << endl;
data <<"pbc_csr.port_7.cfg_oq_shared_pool.credits_1: 0x" << pbc_csr.port_7.cfg_oq_shared_pool.credits_1() << endl;

    pbc_csr.port_7.cfg_oq_xoff2oq.read();
    data <<"pbc_csr.port_7.cfg_oq_xoff2oq.all: 0x" << pbc_csr.port_7.cfg_oq_xoff2oq.all() << endl;
data <<"pbc_csr.port_7.cfg_oq_xoff2oq.map: 0x" << pbc_csr.port_7.cfg_oq_xoff2oq.map() << endl;

    pbc_csr.port_7.cfg_tail_drop.read();
    data <<"pbc_csr.port_7.cfg_tail_drop.all: 0x" << pbc_csr.port_7.cfg_tail_drop.all() << endl;
data <<"pbc_csr.port_7.cfg_tail_drop.cpu_threshold: 0x" << pbc_csr.port_7.cfg_tail_drop.cpu_threshold() << endl;
data <<"pbc_csr.port_7.cfg_tail_drop.span_threshold: 0x" << pbc_csr.port_7.cfg_tail_drop.span_threshold() << endl;

    pbc_csr.port_7.cfg_write_control.read();
    data <<"pbc_csr.port_7.cfg_write_control.all: 0x" << pbc_csr.port_7.cfg_write_control.all() << endl;
data <<"pbc_csr.port_7.cfg_write_control.release_cells: 0x" << pbc_csr.port_7.cfg_write_control.release_cells() << endl;
data <<"pbc_csr.port_7.cfg_write_control.rate_limiter_adj_time: 0x" << pbc_csr.port_7.cfg_write_control.rate_limiter_adj_time() << endl;
data <<"pbc_csr.port_7.cfg_write_control.enable: 0x" << pbc_csr.port_7.cfg_write_control.enable() << endl;
data <<"pbc_csr.port_7.cfg_write_control.enable_bmc_multicast: 0x" << pbc_csr.port_7.cfg_write_control.enable_bmc_multicast() << endl;
data <<"pbc_csr.port_7.cfg_write_control.rewrite: 0x" << pbc_csr.port_7.cfg_write_control.rewrite() << endl;
data <<"pbc_csr.port_7.cfg_write_control.span_pg_enable: 0x" << pbc_csr.port_7.cfg_write_control.span_pg_enable() << endl;
data <<"pbc_csr.port_7.cfg_write_control.span_pg_index: 0x" << pbc_csr.port_7.cfg_write_control.span_pg_index() << endl;
data <<"pbc_csr.port_7.cfg_write_control.enforce_oq24_31: 0x" << pbc_csr.port_7.cfg_write_control.enforce_oq24_31() << endl;
data <<"pbc_csr.port_7.cfg_write_control.drop_on_error: 0x" << pbc_csr.port_7.cfg_write_control.drop_on_error() << endl;
data <<"pbc_csr.port_7.cfg_write_control.min_size: 0x" << pbc_csr.port_7.cfg_write_control.min_size() << endl;
data <<"pbc_csr.port_7.cfg_write_control.rate_limiter_enable_force_emergency_stop: 0x" << pbc_csr.port_7.cfg_write_control.rate_limiter_enable_force_emergency_stop() << endl;
data <<"pbc_csr.port_7.cfg_write_control.cut_thru_select: 0x" << pbc_csr.port_7.cfg_write_control.cut_thru_select() << endl;
data <<"pbc_csr.port_7.cfg_write_control.rate_limiter_enable_emergency_stop: 0x" << pbc_csr.port_7.cfg_write_control.rate_limiter_enable_emergency_stop() << endl;
data <<"pbc_csr.port_7.cfg_write_control.enqueue_mux_bypass: 0x" << pbc_csr.port_7.cfg_write_control.enqueue_mux_bypass() << endl;
data <<"pbc_csr.port_7.cfg_write_control.use_intrinsic_credits: 0x" << pbc_csr.port_7.cfg_write_control.use_intrinsic_credits() << endl;
data <<"pbc_csr.port_7.cfg_write_control.enable_drdy: 0x" << pbc_csr.port_7.cfg_write_control.enable_drdy() << endl;
data <<"pbc_csr.port_7.cfg_write_control.recirc_oq: 0x" << pbc_csr.port_7.cfg_write_control.recirc_oq() << endl;
data <<"pbc_csr.port_7.cfg_write_control.rate_limiter: 0x" << pbc_csr.port_7.cfg_write_control.rate_limiter() << endl;
data <<"pbc_csr.port_7.cfg_write_control.cut_thru: 0x" << pbc_csr.port_7.cfg_write_control.cut_thru() << endl;
data <<"pbc_csr.port_7.cfg_write_control.recirc_enable: 0x" << pbc_csr.port_7.cfg_write_control.recirc_enable() << endl;
data <<"pbc_csr.port_7.cfg_write_control.enable_mnic_multicast: 0x" << pbc_csr.port_7.cfg_write_control.enable_mnic_multicast() << endl;

    pbc_csr.port_7.cfg_write_global.read();
    data <<"pbc_csr.port_7.cfg_write_global.all: 0x" << pbc_csr.port_7.cfg_write_global.all() << endl;
data <<"pbc_csr.port_7.cfg_write_global.occupancy_clear_threshold: 0x" << pbc_csr.port_7.cfg_write_global.occupancy_clear_threshold() << endl;
data <<"pbc_csr.port_7.cfg_write_global.occupancy_set_threshold: 0x" << pbc_csr.port_7.cfg_write_global.occupancy_set_threshold() << endl;

    SDK_TRACE_DEBUG("%s", data.str().c_str());
}

bool
elba_tm_port_is_uplink_port (uint32_t port)
{
    return ((port >= ELBA_TM_UPLINK_PORT_BEGIN) && (port <= ELBA_TM_UPLINK_PORT_END));
}

bool
elba_tm_port_is_dma_port (uint32_t port)
{
    return ((port >= ELBA_TM_DMA_PORT_BEGIN) && (port <= ELBA_TM_DMA_PORT_END));
}

static bool
elba_tm_is_valid_port (uint32_t port)
{
    return ((port >= ELBA_TM_PORT_UPLINK_0) && (port <= ELBA_TM_PORT_INGRESS));
}

sdk_ret_t
elba_tm_uplink_iq_params_update (tm_port_t             port,
                                 tm_q_t                iq,
                                 tm_uplink_iq_params_t *iq_params)
{
    /* Do some sanity checks for port and iq */
    if (!elba_tm_port_is_uplink_port(port)) {
        SDK_TRACE_ERR("%u is not a valid TM uplink port",
                      port);
        return sdk::SDK_RET_INVALID_ARG;
    }

    tm_port_type_e port_type = elba_tm_get_port_type(port);
    if (iq_params->mtu > tm_cfg_profile()->jumbo_mtu[port_type]) {
        SDK_TRACE_ERR("Invalid mtu %u larger than the jumbo %u",
                      iq_params->mtu, tm_cfg_profile()->jumbo_mtu[port_type]);
        return sdk::SDK_RET_INVALID_ARG;
    }

    if ((iq_params->p4_q < (signed)ELBA_TM_P4_UPLINK_IQ_OFFSET) ||
        (iq_params->p4_q >= (signed)elba_tm_get_num_iqs_for_port_type(TM_PORT_TYPE_P4IG))) {
        SDK_TRACE_ERR("Invalid P4 Oq %u for uplink port %u",
                      iq_params->p4_q, port);
        return sdk::SDK_RET_INVALID_ARG;
    }

    stringstream data;
    data << hex << endl;
    elb_top_csr_t &elb0 = ELB_BLK_REG_MODEL_ACCESS(elb_top_csr_t, 0, 0);
    elb_pbc_csr_t &pbc_csr = elb0.pb.pbc;
    elb_pf_csr_t &pf_csr = elb0.pf.pf;
    cpp_int xoff_val;
    cpp_int xon_val;
    cpp_int oq_map_val;
    cpp_int port_payload_occupancy_val;
    uint32_t payload_occupancy;
    uint32_t payload_occupancy_bytes;
    uint32_t xoff_threshold;
    uint32_t xon_threshold;
    uint32_t hbm_context;
    uint32_t num_hbm_contexts_per_port;
    elb_pbc_oq_map_t oq_map_decoder;
    oq_map_decoder.init();

    num_hbm_contexts_per_port = tm_cfg_profile()->num_qs[TM_PORT_TYPE_UPLINK];
    switch(port) {
        case ELBA_TM_PORT_UPLINK_0:
            {
                // P4 oq derivation register
                pbc_csr.cfg_parser0.read();
                oq_map_val = pbc_csr.cfg_parser0.oq_map();

                cpp_helper.set_slc(oq_map_val, iq_params->p4_q,
                                    iq * 5,
                                    ((iq + 1) * 5) - 1);

                pbc_csr.cfg_parser0.oq_map(oq_map_val);

                oq_map_decoder.all(pbc_csr.cfg_parser0.oq_map());
                oq_map_decoder.set_name("elb0.pb.pbc.cfg_parser0.decoder");
                if (tm_sw_cfg_write_enabled()) {
                    oq_map_decoder.show();

                    data <<"pbc_csr.cfg_parser0.all: 0x" << pbc_csr.cfg_parser0.all() << endl;
data <<"pbc_csr.cfg_parser0.default_cos: 0x" << pbc_csr.cfg_parser0.default_cos() << endl;
data <<"pbc_csr.cfg_parser0.oq_map: 0x" << pbc_csr.cfg_parser0.oq_map() << endl;
data <<"pbc_csr.cfg_parser0.default_port: 0x" << pbc_csr.cfg_parser0.default_port() << endl;

                    pbc_csr.cfg_parser0.write();
                }

                // MTU
                pbc_csr.port_0.cfg_account_mtu_table.read();
                switch (iq) {
                    case 0:
                        {
                            /* Update the MTU in the MTU register */
                            pbc_csr.port_0.cfg_account_mtu_table.pg0(
                                bytes_to_cells(iq_params->mtu));
                            break;
                        }
                    case 1:
                        {
                            /* Update the MTU in the MTU register */
                            pbc_csr.port_0.cfg_account_mtu_table.pg1(
                                bytes_to_cells(iq_params->mtu));
                            break;
                        }
                    case 2:
                        {
                            /* Update the MTU in the MTU register */
                            pbc_csr.port_0.cfg_account_mtu_table.pg2(
                                bytes_to_cells(iq_params->mtu));
                            break;
                        }
                    case 3:
                        {
                            /* Update the MTU in the MTU register */
                            pbc_csr.port_0.cfg_account_mtu_table.pg3(
                                bytes_to_cells(iq_params->mtu));
                            break;
                        }
                    case 4:
                        {
                            /* Update the MTU in the MTU register */
                            pbc_csr.port_0.cfg_account_mtu_table.pg4(
                                bytes_to_cells(iq_params->mtu));
                            break;
                        }
                    case 5:
                        {
                            /* Update the MTU in the MTU register */
                            pbc_csr.port_0.cfg_account_mtu_table.pg5(
                                bytes_to_cells(iq_params->mtu));
                            break;
                        }
                    case 6:
                        {
                            /* Update the MTU in the MTU register */
                            pbc_csr.port_0.cfg_account_mtu_table.pg6(
                                bytes_to_cells(iq_params->mtu));
                            break;
                        }
                    case 7:
                        {
                            /* Update the MTU in the MTU register */
                            pbc_csr.port_0.cfg_account_mtu_table.pg7(
                                bytes_to_cells(iq_params->mtu));
                            break;
                        }
                    default:
                        return sdk::SDK_RET_ERR;
                }

                if (tm_sw_cfg_write_enabled()) {
                    data <<"pbc_csr.port_0.cfg_account_mtu_table.all: 0x" << pbc_csr.port_0.cfg_account_mtu_table.all() << endl;
data <<"pbc_csr.port_0.cfg_account_mtu_table.pg6: 0x" << pbc_csr.port_0.cfg_account_mtu_table.pg6() << endl;
data <<"pbc_csr.port_0.cfg_account_mtu_table.pg7: 0x" << pbc_csr.port_0.cfg_account_mtu_table.pg7() << endl;
data <<"pbc_csr.port_0.cfg_account_mtu_table.pg4: 0x" << pbc_csr.port_0.cfg_account_mtu_table.pg4() << endl;
data <<"pbc_csr.port_0.cfg_account_mtu_table.pg5: 0x" << pbc_csr.port_0.cfg_account_mtu_table.pg5() << endl;
data <<"pbc_csr.port_0.cfg_account_mtu_table.pg2: 0x" << pbc_csr.port_0.cfg_account_mtu_table.pg2() << endl;
data <<"pbc_csr.port_0.cfg_account_mtu_table.pg3: 0x" << pbc_csr.port_0.cfg_account_mtu_table.pg3() << endl;
data <<"pbc_csr.port_0.cfg_account_mtu_table.pg0: 0x" << pbc_csr.port_0.cfg_account_mtu_table.pg0() << endl;
data <<"pbc_csr.port_0.cfg_account_mtu_table.pg1: 0x" << pbc_csr.port_0.cfg_account_mtu_table.pg1() << endl;

                    pbc_csr.port_0.cfg_account_mtu_table.write();
                }

                if (port_supports_hbm_contexts(port)) {
                    // HBM Thresholds
                    pf_csr.cfg_hbm_threshold.read();
                    pf_csr.hbm_port_0.cfg_hbm_eth_payload_occupancy.read();

                    port_payload_occupancy_val = pf_csr.hbm_port_0.cfg_hbm_eth_payload_occupancy.threshold();

                    xoff_val = pf_csr.cfg_hbm_threshold.xoff();
                    xon_val = pf_csr.cfg_hbm_threshold.xon();
                    hbm_context = iq + (num_hbm_contexts_per_port * ELBA_TM_PORT_UPLINK_0);

                    payload_occupancy = cpp_helper.get_slc(
                        port_payload_occupancy_val,
                        iq*19, ((iq + 1) * 19) - 1).convert_to<uint32_t>();

                    payload_occupancy_bytes = payload_occupancy << 10;

                    // xoff threshold is the value from the payload occupancy
                    // threshold.
                    // But in the csr we need to write the value from the base.
                    // So subtract it from the payload occupancy threshold

                    xoff_threshold = iq_params->xoff_threshold;
                    if (xoff_threshold && (payload_occupancy_bytes > xoff_threshold)) {
                        xoff_threshold = payload_occupancy_bytes - xoff_threshold;
                    } else {
                        xoff_threshold = 0;
                    }

                    // xoff and xon thresholds are in 512B units in register.
                    // So right shift by 9 (using ceil value for xon and floor
                    // for xoff)
                    xoff_threshold >>= 9;
                    xon_threshold = (iq_params->xon_threshold + (1<<9) - 1) >> 9;

                    // 20 bits per hbm_context
                    cpp_helper.set_slc(xoff_val, xoff_threshold,
                                        hbm_context * 20, ((hbm_context + 1) * 20) - 1);
                    // 20 bits per hbm_context
                    cpp_helper.set_slc(xon_val, xon_threshold,
                                        hbm_context * 20, ((hbm_context + 1) * 20) - 1);

                    // TODO:write the user provided threshold values 
                    //pf_csr.cfg_hbm_threshold.xoff(xoff_val);
                    //pf_csr.cfg_hbm_threshold.xon(xon_val);
                    // Write all the registers
                    if (tm_sw_cfg_write_enabled()) {
                        data <<"pf_csr.cfg_hbm_threshold.all: 0x" << pf_csr.cfg_hbm_threshold.all() << endl;

                        pf_csr.cfg_hbm_threshold.write();
                    }
                }
                break;
            }
        case ELBA_TM_PORT_UPLINK_1:
            {
                // P4 oq derivation register
                pbc_csr.cfg_parser1.read();
                oq_map_val = pbc_csr.cfg_parser1.oq_map();

                cpp_helper.set_slc(oq_map_val, iq_params->p4_q,
                                    iq * 5,
                                    ((iq + 1) * 5) - 1);

                pbc_csr.cfg_parser1.oq_map(oq_map_val);

                oq_map_decoder.all(pbc_csr.cfg_parser1.oq_map());
                oq_map_decoder.set_name("elb0.pb.pbc.cfg_parser1.decoder");
                if (tm_sw_cfg_write_enabled()) {
                    oq_map_decoder.show();

                    data <<"pbc_csr.cfg_parser1.all: 0x" << pbc_csr.cfg_parser1.all() << endl;
data <<"pbc_csr.cfg_parser1.default_cos: 0x" << pbc_csr.cfg_parser1.default_cos() << endl;
data <<"pbc_csr.cfg_parser1.oq_map: 0x" << pbc_csr.cfg_parser1.oq_map() << endl;
data <<"pbc_csr.cfg_parser1.default_port: 0x" << pbc_csr.cfg_parser1.default_port() << endl;

                    pbc_csr.cfg_parser1.write();
                }

                // MTU
                pbc_csr.port_1.cfg_account_mtu_table.read();
                switch (iq) {
                    case 0:
                        {
                            /* Update the MTU in the MTU register */
                            pbc_csr.port_1.cfg_account_mtu_table.pg0(
                                bytes_to_cells(iq_params->mtu));
                            break;
                        }
                    case 1:
                        {
                            /* Update the MTU in the MTU register */
                            pbc_csr.port_1.cfg_account_mtu_table.pg1(
                                bytes_to_cells(iq_params->mtu));
                            break;
                        }
                    case 2:
                        {
                            /* Update the MTU in the MTU register */
                            pbc_csr.port_1.cfg_account_mtu_table.pg2(
                                bytes_to_cells(iq_params->mtu));
                            break;
                        }
                    case 3:
                        {
                            /* Update the MTU in the MTU register */
                            pbc_csr.port_1.cfg_account_mtu_table.pg3(
                                bytes_to_cells(iq_params->mtu));
                            break;
                        }
                    case 4:
                        {
                            /* Update the MTU in the MTU register */
                            pbc_csr.port_1.cfg_account_mtu_table.pg4(
                                bytes_to_cells(iq_params->mtu));
                            break;
                        }
                    case 5:
                        {
                            /* Update the MTU in the MTU register */
                            pbc_csr.port_1.cfg_account_mtu_table.pg5(
                                bytes_to_cells(iq_params->mtu));
                            break;
                        }
                    case 6:
                        {
                            /* Update the MTU in the MTU register */
                            pbc_csr.port_1.cfg_account_mtu_table.pg6(
                                bytes_to_cells(iq_params->mtu));
                            break;
                        }
                    case 7:
                        {
                            /* Update the MTU in the MTU register */
                            pbc_csr.port_1.cfg_account_mtu_table.pg7(
                                bytes_to_cells(iq_params->mtu));
                            break;
                        }
                    default:
                        return sdk::SDK_RET_ERR;
                }

                if (tm_sw_cfg_write_enabled()) {
                    data <<"pbc_csr.port_1.cfg_account_mtu_table.all: 0x" << pbc_csr.port_1.cfg_account_mtu_table.all() << endl;
data <<"pbc_csr.port_1.cfg_account_mtu_table.pg6: 0x" << pbc_csr.port_1.cfg_account_mtu_table.pg6() << endl;
data <<"pbc_csr.port_1.cfg_account_mtu_table.pg7: 0x" << pbc_csr.port_1.cfg_account_mtu_table.pg7() << endl;
data <<"pbc_csr.port_1.cfg_account_mtu_table.pg4: 0x" << pbc_csr.port_1.cfg_account_mtu_table.pg4() << endl;
data <<"pbc_csr.port_1.cfg_account_mtu_table.pg5: 0x" << pbc_csr.port_1.cfg_account_mtu_table.pg5() << endl;
data <<"pbc_csr.port_1.cfg_account_mtu_table.pg2: 0x" << pbc_csr.port_1.cfg_account_mtu_table.pg2() << endl;
data <<"pbc_csr.port_1.cfg_account_mtu_table.pg3: 0x" << pbc_csr.port_1.cfg_account_mtu_table.pg3() << endl;
data <<"pbc_csr.port_1.cfg_account_mtu_table.pg0: 0x" << pbc_csr.port_1.cfg_account_mtu_table.pg0() << endl;
data <<"pbc_csr.port_1.cfg_account_mtu_table.pg1: 0x" << pbc_csr.port_1.cfg_account_mtu_table.pg1() << endl;

                    pbc_csr.port_1.cfg_account_mtu_table.write();
                }

                if (port_supports_hbm_contexts(port)) {
                    // HBM Thresholds
                    pf_csr.cfg_hbm_threshold.read();
                    pf_csr.hbm_port_1.cfg_hbm_eth_payload_occupancy.read();

                    port_payload_occupancy_val = pf_csr.hbm_port_1.cfg_hbm_eth_payload_occupancy.threshold();

                    xoff_val = pf_csr.cfg_hbm_threshold.xoff();
                    xon_val = pf_csr.cfg_hbm_threshold.xon();
                    hbm_context = iq + (num_hbm_contexts_per_port * ELBA_TM_PORT_UPLINK_1);

                    payload_occupancy = cpp_helper.get_slc(
                        port_payload_occupancy_val,
                        iq*19, ((iq + 1) * 19) - 1).convert_to<uint32_t>();

                    payload_occupancy_bytes = payload_occupancy << 10;

                    // xoff threshold is the value from the payload occupancy
                    // threshold.
                    // But in the csr we need to write the value from the base.
                    // So subtract it from the payload occupancy threshold

                    xoff_threshold = iq_params->xoff_threshold;
                    if (xoff_threshold && (payload_occupancy_bytes > xoff_threshold)) {
                        xoff_threshold = payload_occupancy_bytes - xoff_threshold;
                    } else {
                        xoff_threshold = 0;
                    }

                    // xoff and xon thresholds are in 512B units in register.
                    // So right shift by 9 (using ceil value for xon and floor
                    // for xoff)
                    xoff_threshold >>= 9;
                    xon_threshold = (iq_params->xon_threshold + (1<<9) - 1) >> 9;

                    // 20 bits per hbm_context
                    cpp_helper.set_slc(xoff_val, xoff_threshold,
                                        hbm_context * 20, ((hbm_context + 1) * 20) - 1);
                    // 20 bits per hbm_context
                    cpp_helper.set_slc(xon_val, xon_threshold,
                                        hbm_context * 20, ((hbm_context + 1) * 20) - 1);

                    // TODO:write the user provided threshold values 
                    //pf_csr.cfg_hbm_threshold.xoff(xoff_val);
                    //pf_csr.cfg_hbm_threshold.xon(xon_val);
                    // Write all the registers
                    if (tm_sw_cfg_write_enabled()) {
                        data <<"pf_csr.cfg_hbm_threshold.all: 0x" << pf_csr.cfg_hbm_threshold.all() << endl;

                        pf_csr.cfg_hbm_threshold.write();
                    }
                }
                break;
            }
        case ELBA_TM_PORT_UPLINK_2:
            {
                // P4 oq derivation register
                pbc_csr.cfg_parser2.read();
                oq_map_val = pbc_csr.cfg_parser2.oq_map();

                cpp_helper.set_slc(oq_map_val, iq_params->p4_q,
                                    iq * 5,
                                    ((iq + 1) * 5) - 1);

                pbc_csr.cfg_parser2.oq_map(oq_map_val);

                oq_map_decoder.all(pbc_csr.cfg_parser2.oq_map());
                oq_map_decoder.set_name("elb0.pb.pbc.cfg_parser2.decoder");
                if (tm_sw_cfg_write_enabled()) {
                    oq_map_decoder.show();

                    data <<"pbc_csr.cfg_parser2.all: 0x" << pbc_csr.cfg_parser2.all() << endl;
data <<"pbc_csr.cfg_parser2.default_cos: 0x" << pbc_csr.cfg_parser2.default_cos() << endl;
data <<"pbc_csr.cfg_parser2.oq_map: 0x" << pbc_csr.cfg_parser2.oq_map() << endl;
data <<"pbc_csr.cfg_parser2.default_port: 0x" << pbc_csr.cfg_parser2.default_port() << endl;

                    pbc_csr.cfg_parser2.write();
                }

                // MTU
                pbc_csr.port_2.cfg_account_mtu_table.read();
                switch (iq) {
                    case 0:
                        {
                            /* Update the MTU in the MTU register */
                            pbc_csr.port_2.cfg_account_mtu_table.pg0(
                                bytes_to_cells(iq_params->mtu));
                            break;
                        }
                    case 1:
                        {
                            /* Update the MTU in the MTU register */
                            pbc_csr.port_2.cfg_account_mtu_table.pg1(
                                bytes_to_cells(iq_params->mtu));
                            break;
                        }
                    case 2:
                        {
                            /* Update the MTU in the MTU register */
                            pbc_csr.port_2.cfg_account_mtu_table.pg2(
                                bytes_to_cells(iq_params->mtu));
                            break;
                        }
                    case 3:
                        {
                            /* Update the MTU in the MTU register */
                            pbc_csr.port_2.cfg_account_mtu_table.pg3(
                                bytes_to_cells(iq_params->mtu));
                            break;
                        }
                    case 4:
                        {
                            /* Update the MTU in the MTU register */
                            pbc_csr.port_2.cfg_account_mtu_table.pg4(
                                bytes_to_cells(iq_params->mtu));
                            break;
                        }
                    case 5:
                        {
                            /* Update the MTU in the MTU register */
                            pbc_csr.port_2.cfg_account_mtu_table.pg5(
                                bytes_to_cells(iq_params->mtu));
                            break;
                        }
                    case 6:
                        {
                            /* Update the MTU in the MTU register */
                            pbc_csr.port_2.cfg_account_mtu_table.pg6(
                                bytes_to_cells(iq_params->mtu));
                            break;
                        }
                    case 7:
                        {
                            /* Update the MTU in the MTU register */
                            pbc_csr.port_2.cfg_account_mtu_table.pg7(
                                bytes_to_cells(iq_params->mtu));
                            break;
                        }
                    default:
                        return sdk::SDK_RET_ERR;
                }

                if (tm_sw_cfg_write_enabled()) {
                    data <<"pbc_csr.port_2.cfg_account_mtu_table.all: 0x" << pbc_csr.port_2.cfg_account_mtu_table.all() << endl;
data <<"pbc_csr.port_2.cfg_account_mtu_table.pg6: 0x" << pbc_csr.port_2.cfg_account_mtu_table.pg6() << endl;
data <<"pbc_csr.port_2.cfg_account_mtu_table.pg7: 0x" << pbc_csr.port_2.cfg_account_mtu_table.pg7() << endl;
data <<"pbc_csr.port_2.cfg_account_mtu_table.pg4: 0x" << pbc_csr.port_2.cfg_account_mtu_table.pg4() << endl;
data <<"pbc_csr.port_2.cfg_account_mtu_table.pg5: 0x" << pbc_csr.port_2.cfg_account_mtu_table.pg5() << endl;
data <<"pbc_csr.port_2.cfg_account_mtu_table.pg2: 0x" << pbc_csr.port_2.cfg_account_mtu_table.pg2() << endl;
data <<"pbc_csr.port_2.cfg_account_mtu_table.pg3: 0x" << pbc_csr.port_2.cfg_account_mtu_table.pg3() << endl;
data <<"pbc_csr.port_2.cfg_account_mtu_table.pg0: 0x" << pbc_csr.port_2.cfg_account_mtu_table.pg0() << endl;
data <<"pbc_csr.port_2.cfg_account_mtu_table.pg1: 0x" << pbc_csr.port_2.cfg_account_mtu_table.pg1() << endl;

                    pbc_csr.port_2.cfg_account_mtu_table.write();
                }

                if (port_supports_hbm_contexts(port)) {
                    // HBM Thresholds
                    pf_csr.cfg_hbm_threshold.read();
                    pf_csr.hbm_port_2.cfg_hbm_eth_payload_occupancy.read();

                    port_payload_occupancy_val = pf_csr.hbm_port_2.cfg_hbm_eth_payload_occupancy.threshold();

                    xoff_val = pf_csr.cfg_hbm_threshold.xoff();
                    xon_val = pf_csr.cfg_hbm_threshold.xon();
                    hbm_context = iq + (num_hbm_contexts_per_port * ELBA_TM_PORT_UPLINK_2);

                    payload_occupancy = cpp_helper.get_slc(
                        port_payload_occupancy_val,
                        iq*19, ((iq + 1) * 19) - 1).convert_to<uint32_t>();

                    payload_occupancy_bytes = payload_occupancy << 10;

                    // xoff threshold is the value from the payload occupancy
                    // threshold.
                    // But in the csr we need to write the value from the base.
                    // So subtract it from the payload occupancy threshold

                    xoff_threshold = iq_params->xoff_threshold;
                    if (xoff_threshold && (payload_occupancy_bytes > xoff_threshold)) {
                        xoff_threshold = payload_occupancy_bytes - xoff_threshold;
                    } else {
                        xoff_threshold = 0;
                    }

                    // xoff and xon thresholds are in 512B units in register.
                    // So right shift by 9 (using ceil value for xon and floor
                    // for xoff)
                    xoff_threshold >>= 9;
                    xon_threshold = (iq_params->xon_threshold + (1<<9) - 1) >> 9;

                    // 20 bits per hbm_context
                    cpp_helper.set_slc(xoff_val, xoff_threshold,
                                        hbm_context * 20, ((hbm_context + 1) * 20) - 1);
                    // 20 bits per hbm_context
                    cpp_helper.set_slc(xon_val, xon_threshold,
                                        hbm_context * 20, ((hbm_context + 1) * 20) - 1);

                    // TODO:write the user provided threshold values 
                    //pf_csr.cfg_hbm_threshold.xoff(xoff_val);
                    //pf_csr.cfg_hbm_threshold.xon(xon_val);
                    // Write all the registers
                    if (tm_sw_cfg_write_enabled()) {
                        data <<"pf_csr.cfg_hbm_threshold.all: 0x" << pf_csr.cfg_hbm_threshold.all() << endl;

                        pf_csr.cfg_hbm_threshold.write();
                    }
                }
                break;
            }
        case ELBA_TM_PORT_UPLINK_3:
            {
                // P4 oq derivation register
                pbc_csr.cfg_parser3.read();
                oq_map_val = pbc_csr.cfg_parser3.oq_map();

                cpp_helper.set_slc(oq_map_val, iq_params->p4_q,
                                    iq * 5,
                                    ((iq + 1) * 5) - 1);

                pbc_csr.cfg_parser3.oq_map(oq_map_val);

                oq_map_decoder.all(pbc_csr.cfg_parser3.oq_map());
                oq_map_decoder.set_name("elb0.pb.pbc.cfg_parser3.decoder");
                if (tm_sw_cfg_write_enabled()) {
                    oq_map_decoder.show();

                    data <<"pbc_csr.cfg_parser3.all: 0x" << pbc_csr.cfg_parser3.all() << endl;
data <<"pbc_csr.cfg_parser3.default_cos: 0x" << pbc_csr.cfg_parser3.default_cos() << endl;
data <<"pbc_csr.cfg_parser3.oq_map: 0x" << pbc_csr.cfg_parser3.oq_map() << endl;
data <<"pbc_csr.cfg_parser3.default_port: 0x" << pbc_csr.cfg_parser3.default_port() << endl;

                    pbc_csr.cfg_parser3.write();
                }

                // MTU
                pbc_csr.port_3.cfg_account_mtu_table.read();
                switch (iq) {
                    case 0:
                        {
                            /* Update the MTU in the MTU register */
                            pbc_csr.port_3.cfg_account_mtu_table.pg0(
                                bytes_to_cells(iq_params->mtu));
                            break;
                        }
                    case 1:
                        {
                            /* Update the MTU in the MTU register */
                            pbc_csr.port_3.cfg_account_mtu_table.pg1(
                                bytes_to_cells(iq_params->mtu));
                            break;
                        }
                    case 2:
                        {
                            /* Update the MTU in the MTU register */
                            pbc_csr.port_3.cfg_account_mtu_table.pg2(
                                bytes_to_cells(iq_params->mtu));
                            break;
                        }
                    case 3:
                        {
                            /* Update the MTU in the MTU register */
                            pbc_csr.port_3.cfg_account_mtu_table.pg3(
                                bytes_to_cells(iq_params->mtu));
                            break;
                        }
                    case 4:
                        {
                            /* Update the MTU in the MTU register */
                            pbc_csr.port_3.cfg_account_mtu_table.pg4(
                                bytes_to_cells(iq_params->mtu));
                            break;
                        }
                    case 5:
                        {
                            /* Update the MTU in the MTU register */
                            pbc_csr.port_3.cfg_account_mtu_table.pg5(
                                bytes_to_cells(iq_params->mtu));
                            break;
                        }
                    case 6:
                        {
                            /* Update the MTU in the MTU register */
                            pbc_csr.port_3.cfg_account_mtu_table.pg6(
                                bytes_to_cells(iq_params->mtu));
                            break;
                        }
                    case 7:
                        {
                            /* Update the MTU in the MTU register */
                            pbc_csr.port_3.cfg_account_mtu_table.pg7(
                                bytes_to_cells(iq_params->mtu));
                            break;
                        }
                    default:
                        return sdk::SDK_RET_ERR;
                }

                if (tm_sw_cfg_write_enabled()) {
                    data <<"pbc_csr.port_3.cfg_account_mtu_table.all: 0x" << pbc_csr.port_3.cfg_account_mtu_table.all() << endl;
data <<"pbc_csr.port_3.cfg_account_mtu_table.pg6: 0x" << pbc_csr.port_3.cfg_account_mtu_table.pg6() << endl;
data <<"pbc_csr.port_3.cfg_account_mtu_table.pg7: 0x" << pbc_csr.port_3.cfg_account_mtu_table.pg7() << endl;
data <<"pbc_csr.port_3.cfg_account_mtu_table.pg4: 0x" << pbc_csr.port_3.cfg_account_mtu_table.pg4() << endl;
data <<"pbc_csr.port_3.cfg_account_mtu_table.pg5: 0x" << pbc_csr.port_3.cfg_account_mtu_table.pg5() << endl;
data <<"pbc_csr.port_3.cfg_account_mtu_table.pg2: 0x" << pbc_csr.port_3.cfg_account_mtu_table.pg2() << endl;
data <<"pbc_csr.port_3.cfg_account_mtu_table.pg3: 0x" << pbc_csr.port_3.cfg_account_mtu_table.pg3() << endl;
data <<"pbc_csr.port_3.cfg_account_mtu_table.pg0: 0x" << pbc_csr.port_3.cfg_account_mtu_table.pg0() << endl;
data <<"pbc_csr.port_3.cfg_account_mtu_table.pg1: 0x" << pbc_csr.port_3.cfg_account_mtu_table.pg1() << endl;

                    pbc_csr.port_3.cfg_account_mtu_table.write();
                }

                if (port_supports_hbm_contexts(port)) {
                    // HBM Thresholds
                    pf_csr.cfg_hbm_threshold.read();
                    pf_csr.hbm_port_3.cfg_hbm_eth_payload_occupancy.read();

                    port_payload_occupancy_val = pf_csr.hbm_port_3.cfg_hbm_eth_payload_occupancy.threshold();

                    xoff_val = pf_csr.cfg_hbm_threshold.xoff();
                    xon_val = pf_csr.cfg_hbm_threshold.xon();
                    hbm_context = iq + (num_hbm_contexts_per_port * ELBA_TM_PORT_UPLINK_3);

                    payload_occupancy = cpp_helper.get_slc(
                        port_payload_occupancy_val,
                        iq*19, ((iq + 1) * 19) - 1).convert_to<uint32_t>();

                    payload_occupancy_bytes = payload_occupancy << 10;

                    // xoff threshold is the value from the payload occupancy
                    // threshold.
                    // But in the csr we need to write the value from the base.
                    // So subtract it from the payload occupancy threshold

                    xoff_threshold = iq_params->xoff_threshold;
                    if (xoff_threshold && (payload_occupancy_bytes > xoff_threshold)) {
                        xoff_threshold = payload_occupancy_bytes - xoff_threshold;
                    } else {
                        xoff_threshold = 0;
                    }

                    // xoff and xon thresholds are in 512B units in register.
                    // So right shift by 9 (using ceil value for xon and floor
                    // for xoff)
                    xoff_threshold >>= 9;
                    xon_threshold = (iq_params->xon_threshold + (1<<9) - 1) >> 9;

                    // 20 bits per hbm_context
                    cpp_helper.set_slc(xoff_val, xoff_threshold,
                                        hbm_context * 20, ((hbm_context + 1) * 20) - 1);
                    // 20 bits per hbm_context
                    cpp_helper.set_slc(xon_val, xon_threshold,
                                        hbm_context * 20, ((hbm_context + 1) * 20) - 1);

                    // TODO:write the user provided threshold values 
                    //pf_csr.cfg_hbm_threshold.xoff(xoff_val);
                    //pf_csr.cfg_hbm_threshold.xon(xon_val);
                    // Write all the registers
                    if (tm_sw_cfg_write_enabled()) {
                        data <<"pf_csr.cfg_hbm_threshold.all: 0x" << pf_csr.cfg_hbm_threshold.all() << endl;

                        pf_csr.cfg_hbm_threshold.write();
                    }
                }
                break;
            }
        default:
            return sdk::SDK_RET_ERR;
    }

    SDK_TRACE_DEBUG("%s", data.str().c_str());
    SDK_TRACE_DEBUG("Updated the iq %u on port %u",
                    iq, port);

    return sdk::SDK_RET_OK;
}

sdk_ret_t
elba_tm_uplink_input_map_update (tm_port_t port,
                                 uint32_t dot1q_pcp,
                                 tm_q_t iq)
{
    if (!elba_tm_port_is_uplink_port(port)) {
        SDK_TRACE_ERR("%u is not a valid TM uplink port",
                      port);
        return sdk::SDK_RET_INVALID_ARG;
    }
    stringstream data;
    data << hex << endl;
    elb_top_csr_t &elb0 = ELB_BLK_REG_MODEL_ACCESS(elb_top_csr_t, 0, 0);
    elb_pf_csr_t &pf_csr = elb0.pf.pf;
    cpp_int tc_map_reg_val;
    uint32_t tc;
    uint32_t nbits;

    tc = dot1q_pcp;

    switch(port) {
        case ELBA_TM_PORT_UPLINK_0:
            {
                nbits = 3;
                pf_csr.hbm_port_0.cfg_hbm_tc_to_q.read();
                tc_map_reg_val = pf_csr.hbm_port_0.cfg_hbm_tc_to_q.table();

                cpp_helper.set_slc(tc_map_reg_val, iq,
                                    tc * nbits,
                                    ((tc+1) * nbits) - 1);

                /* Update and write the tc to PG mapping */
                pf_csr.hbm_port_0.cfg_hbm_tc_to_q.table(tc_map_reg_val);
                if (tm_sw_cfg_write_enabled()) {
                    data <<"pf_csr.hbm_port_0.cfg_hbm_tc_to_q.all: 0x" << pf_csr.hbm_port_0.cfg_hbm_tc_to_q.all() << endl;

                    pf_csr.hbm_port_0.cfg_hbm_tc_to_q.write();
                }
                break;
            }
        case ELBA_TM_PORT_UPLINK_1:
            {
                nbits = 3;
                pf_csr.hbm_port_1.cfg_hbm_tc_to_q.read();
                tc_map_reg_val = pf_csr.hbm_port_1.cfg_hbm_tc_to_q.table();

                cpp_helper.set_slc(tc_map_reg_val, iq,
                                    tc * nbits,
                                    ((tc+1) * nbits) - 1);

                /* Update and write the tc to PG mapping */
                pf_csr.hbm_port_1.cfg_hbm_tc_to_q.table(tc_map_reg_val);
                if (tm_sw_cfg_write_enabled()) {
                    data <<"pf_csr.hbm_port_1.cfg_hbm_tc_to_q.all: 0x" << pf_csr.hbm_port_1.cfg_hbm_tc_to_q.all() << endl;

                    pf_csr.hbm_port_1.cfg_hbm_tc_to_q.write();
                }
                break;
            }
        case ELBA_TM_PORT_UPLINK_2:
            {
                nbits = 3;
                pf_csr.hbm_port_2.cfg_hbm_tc_to_q.read();
                tc_map_reg_val = pf_csr.hbm_port_2.cfg_hbm_tc_to_q.table();

                cpp_helper.set_slc(tc_map_reg_val, iq,
                                    tc * nbits,
                                    ((tc+1) * nbits) - 1);

                /* Update and write the tc to PG mapping */
                pf_csr.hbm_port_2.cfg_hbm_tc_to_q.table(tc_map_reg_val);
                if (tm_sw_cfg_write_enabled()) {
                    data <<"pf_csr.hbm_port_2.cfg_hbm_tc_to_q.all: 0x" << pf_csr.hbm_port_2.cfg_hbm_tc_to_q.all() << endl;

                    pf_csr.hbm_port_2.cfg_hbm_tc_to_q.write();
                }
                break;
            }
        case ELBA_TM_PORT_UPLINK_3:
            {
                nbits = 3;
                pf_csr.hbm_port_3.cfg_hbm_tc_to_q.read();
                tc_map_reg_val = pf_csr.hbm_port_3.cfg_hbm_tc_to_q.table();

                cpp_helper.set_slc(tc_map_reg_val, iq,
                                    tc * nbits,
                                    ((tc+1) * nbits) - 1);

                /* Update and write the tc to PG mapping */
                pf_csr.hbm_port_3.cfg_hbm_tc_to_q.table(tc_map_reg_val);
                if (tm_sw_cfg_write_enabled()) {
                    data <<"pf_csr.hbm_port_3.cfg_hbm_tc_to_q.all: 0x" << pf_csr.hbm_port_3.cfg_hbm_tc_to_q.all() << endl;

                    pf_csr.hbm_port_3.cfg_hbm_tc_to_q.write();
                }
                break;
            }
        default:
            return sdk::SDK_RET_INVALID_ARG;
    }
    SDK_TRACE_DEBUG("%s", data.str().c_str());
    return sdk::SDK_RET_OK;
}

sdk_ret_t
elba_tm_uplink_input_dscp_map_update (tm_port_t port,
                                      tm_uplink_input_dscp_map_t *dscp_map)
{
    if (!elba_tm_port_is_uplink_port(port)) {
        SDK_TRACE_ERR("%u is not a valid TM uplink port",
                      port);
        return sdk::SDK_RET_INVALID_ARG;
    }
    stringstream data;
    data << hex << endl;
    #if 0 /* TODO_ELBA */
    elb_top_csr_t &elb0 = ELB_BLK_REG_MODEL_ACCESS(elb_top_csr_t, 0, 0);
    elb_pbc_csr_t &pbc_csr = elb0.pb.pbc;
    elb_pf_csr_t &pf_csr = elb0.pf.pf;
    cpp_int dscp_map_val;
    uint32_t tc;
    int use_ip = 0;

    tc = dscp_map->dot1q_pcp;
    switch(port) {
        case ELBA_TM_PORT_UPLINK_0:
            {
                pf_csr.hbm_port_0.cfg_hbm_parser.read();
                dscp_map_val = pf_csr.hbm_port_0.cfg_hbm_parser.dscp_map();

                for (unsigned i = 0; i < SDK_ARRAY_SIZE(dscp_map->ip_dscp); i++) {
                    if (dscp_map->ip_dscp[i]) {
                        cpp_helper.set_slc(dscp_map_val,
                                           tc,
                                           i * 3,
                                           ((i+1) * 3) - 1);
                    }
                }

                use_ip = (dscp_map_val ? 1 : 0);

                pf_csr.hbm_port_0.cfg_hbm_parser.use_dot1q(1);
                pf_csr.hbm_port_0.cfg_hbm_parser.use_ip(use_ip);
                pf_csr.hbm_port_0.cfg_hbm_parser.dscp_map(dscp_map_val);
                if (tm_sw_cfg_write_enabled()) {
                    data <<"pf_csr.hbm_port_0.cfg_hbm_parser.all: 0x" << pf_csr.hbm_port_0.cfg_hbm_parser.all() << endl;

                    pf_csr.hbm_port_0.cfg_hbm_parser.write();
                }
                break;
            }
        case ELBA_TM_PORT_UPLINK_1:
            {
                pf_csr.hbm_port_1.cfg_hbm_parser.read();
                dscp_map_val = pf_csr.hbm_port_1.cfg_hbm_parser.dscp_map();

                for (unsigned i = 0; i < SDK_ARRAY_SIZE(dscp_map->ip_dscp); i++) {
                    if (dscp_map->ip_dscp[i]) {
                        cpp_helper.set_slc(dscp_map_val,
                                           tc,
                                           i * 3,
                                           ((i+1) * 3) - 1);
                    }
                }

                use_ip = (dscp_map_val ? 1 : 0);

                pf_csr.hbm_port_1.cfg_hbm_parser.use_dot1q(1);
                pf_csr.hbm_port_1.cfg_hbm_parser.use_ip(use_ip);
                pf_csr.hbm_port_1.cfg_hbm_parser.dscp_map(dscp_map_val);
                if (tm_sw_cfg_write_enabled()) {
                    data <<"pf_csr.hbm_port_1.cfg_hbm_parser.all: 0x" << pf_csr.hbm_port_1.cfg_hbm_parser.all() << endl;

                    pf_csr.hbm_port_1.cfg_hbm_parser.write();
                }
                break;
            }
        case ELBA_TM_PORT_UPLINK_2:
            {
                pf_csr.hbm_port_2.cfg_hbm_parser.read();
                dscp_map_val = pf_csr.hbm_port_2.cfg_hbm_parser.dscp_map();

                for (unsigned i = 0; i < SDK_ARRAY_SIZE(dscp_map->ip_dscp); i++) {
                    if (dscp_map->ip_dscp[i]) {
                        cpp_helper.set_slc(dscp_map_val,
                                           tc,
                                           i * 3,
                                           ((i+1) * 3) - 1);
                    }
                }

                use_ip = (dscp_map_val ? 1 : 0);

                pf_csr.hbm_port_2.cfg_hbm_parser.use_dot1q(1);
                pf_csr.hbm_port_2.cfg_hbm_parser.use_ip(use_ip);
                pf_csr.hbm_port_2.cfg_hbm_parser.dscp_map(dscp_map_val);
                if (tm_sw_cfg_write_enabled()) {
                    data <<"pf_csr.hbm_port_2.cfg_hbm_parser.all: 0x" << pf_csr.hbm_port_2.cfg_hbm_parser.all() << endl;

                    pf_csr.hbm_port_2.cfg_hbm_parser.write();
                }
                break;
            }
        case ELBA_TM_PORT_UPLINK_3:
            {
                pf_csr.hbm_port_3.cfg_hbm_parser.read();
                dscp_map_val = pf_csr.hbm_port_3.cfg_hbm_parser.dscp_map();

                for (unsigned i = 0; i < SDK_ARRAY_SIZE(dscp_map->ip_dscp); i++) {
                    if (dscp_map->ip_dscp[i]) {
                        cpp_helper.set_slc(dscp_map_val,
                                           tc,
                                           i * 3,
                                           ((i+1) * 3) - 1);
                    }
                }

                use_ip = (dscp_map_val ? 1 : 0);

                pf_csr.hbm_port_3.cfg_hbm_parser.use_dot1q(1);
                pf_csr.hbm_port_3.cfg_hbm_parser.use_ip(use_ip);
                pf_csr.hbm_port_3.cfg_hbm_parser.dscp_map(dscp_map_val);
                if (tm_sw_cfg_write_enabled()) {
                    data <<"pf_csr.hbm_port_3.cfg_hbm_parser.all: 0x" << pf_csr.hbm_port_3.cfg_hbm_parser.all() << endl;

                    pf_csr.hbm_port_3.cfg_hbm_parser.write();
                }
                break;
            }
        default:
            return sdk::SDK_RET_INVALID_ARG;
    }
    #endif 
    SDK_TRACE_DEBUG("%s", data.str().c_str());
    
    return sdk::SDK_RET_OK;
}


sdk_ret_t
elba_tm_uplink_oq_update (tm_port_t port,
                          tm_q_t oq,
                          bool xoff_enable,
                          uint32_t xoff_cos)
{
    elb_top_csr_t &elb0 = ELB_BLK_REG_MODEL_ACCESS(elb_top_csr_t, 0, 0);
    elb_pbc_csr_t &pbc_csr = elb0.pb.pbc;
    uint32_t xoff_enable_val;
    cpp_int xoff2oq_map_val;
    elb_pbc_eth_oq_xoff_map_t xoff2oq_map_decoder;
    xoff2oq_map_decoder.init();

    if (!elba_tm_port_is_uplink_port(port)) {
        SDK_TRACE_ERR("%u is not a valid TM uplink port",
                      port);
        return sdk::SDK_RET_INVALID_ARG;
    }

    stringstream data;
    data << hex << endl;
    switch(port) {
        case ELBA_TM_PORT_UPLINK_0:
            {
                pbc_csr.port_0.cfg_oq_xoff2oq.read();
                xoff2oq_map_val = pbc_csr.port_0.cfg_oq_xoff2oq.map();

                cpp_helper.set_slc(xoff2oq_map_val, xoff_cos, oq*3, ((oq+1)*3)-1);

                // TODO: write to disable xoff by default
                // pbc_csr.port_0.cfg_oq_xoff2oq.map(xoff2oq_map_val);

                pbc_csr.port_0.cfg_mac_xoff.read();
                xoff_enable_val = pbc_csr.port_0.cfg_mac_xoff.enable().convert_to<uint32_t>();
                if (xoff_enable) {
                    xoff_enable_val |= 1<<xoff_cos;
                } else {
                    xoff_enable_val &= ~(1<<xoff_cos);
                }
                // TODO: write to disable xoff by default
                // pbc_csr.port_0.cfg_mac_xoff.enable(xoff_enable_val);

                xoff2oq_map_decoder.all(pbc_csr.port_0.cfg_oq_xoff2oq.map());
                xoff2oq_map_decoder.set_name("elb0.pb.pbc.port_0.cfg_oq_xoff2oq.decoder");
                if (tm_sw_cfg_write_enabled()) {
                    xoff2oq_map_decoder.show();

                    data <<"pbc_csr.port_0.cfg_oq_xoff2oq.all: 0x" << pbc_csr.port_0.cfg_oq_xoff2oq.all() << endl;
data <<"pbc_csr.port_0.cfg_oq_xoff2oq.map: 0x" << pbc_csr.port_0.cfg_oq_xoff2oq.map() << endl;

                    data <<"pbc_csr.port_0.cfg_mac_xoff.all: 0x" << pbc_csr.port_0.cfg_mac_xoff.all() << endl;
data <<"pbc_csr.port_0.cfg_mac_xoff.enable: 0x" << pbc_csr.port_0.cfg_mac_xoff.enable() << endl;


                    pbc_csr.port_0.cfg_oq_xoff2oq.write();
                    pbc_csr.port_0.cfg_mac_xoff.write();
                }
                break;
            }
        case ELBA_TM_PORT_UPLINK_1:
            {
                pbc_csr.port_1.cfg_oq_xoff2oq.read();
                xoff2oq_map_val = pbc_csr.port_1.cfg_oq_xoff2oq.map();

                cpp_helper.set_slc(xoff2oq_map_val, xoff_cos, oq*3, ((oq+1)*3)-1);

                // TODO: write to disable xoff by default
                // pbc_csr.port_1.cfg_oq_xoff2oq.map(xoff2oq_map_val);

                pbc_csr.port_1.cfg_mac_xoff.read();
                xoff_enable_val = pbc_csr.port_1.cfg_mac_xoff.enable().convert_to<uint32_t>();
                if (xoff_enable) {
                    xoff_enable_val |= 1<<xoff_cos;
                } else {
                    xoff_enable_val &= ~(1<<xoff_cos);
                }
                // TODO: write to disable xoff by default
                // pbc_csr.port_1.cfg_mac_xoff.enable(xoff_enable_val);

                xoff2oq_map_decoder.all(pbc_csr.port_1.cfg_oq_xoff2oq.map());
                xoff2oq_map_decoder.set_name("elb0.pb.pbc.port_1.cfg_oq_xoff2oq.decoder");
                if (tm_sw_cfg_write_enabled()) {
                    xoff2oq_map_decoder.show();

                    data <<"pbc_csr.port_1.cfg_oq_xoff2oq.all: 0x" << pbc_csr.port_1.cfg_oq_xoff2oq.all() << endl;
data <<"pbc_csr.port_1.cfg_oq_xoff2oq.map: 0x" << pbc_csr.port_1.cfg_oq_xoff2oq.map() << endl;

                    data <<"pbc_csr.port_1.cfg_mac_xoff.all: 0x" << pbc_csr.port_1.cfg_mac_xoff.all() << endl;
data <<"pbc_csr.port_1.cfg_mac_xoff.enable: 0x" << pbc_csr.port_1.cfg_mac_xoff.enable() << endl;


                    pbc_csr.port_1.cfg_oq_xoff2oq.write();
                    pbc_csr.port_1.cfg_mac_xoff.write();
                }
                break;
            }
        case ELBA_TM_PORT_UPLINK_2:
            {
                pbc_csr.port_2.cfg_oq_xoff2oq.read();
                xoff2oq_map_val = pbc_csr.port_2.cfg_oq_xoff2oq.map();

                cpp_helper.set_slc(xoff2oq_map_val, xoff_cos, oq*3, ((oq+1)*3)-1);

                // TODO: write to disable xoff by default
                // pbc_csr.port_2.cfg_oq_xoff2oq.map(xoff2oq_map_val);

                pbc_csr.port_2.cfg_mac_xoff.read();
                xoff_enable_val = pbc_csr.port_2.cfg_mac_xoff.enable().convert_to<uint32_t>();
                if (xoff_enable) {
                    xoff_enable_val |= 1<<xoff_cos;
                } else {
                    xoff_enable_val &= ~(1<<xoff_cos);
                }
                // TODO: write to disable xoff by default
                // pbc_csr.port_2.cfg_mac_xoff.enable(xoff_enable_val);

                xoff2oq_map_decoder.all(pbc_csr.port_2.cfg_oq_xoff2oq.map());
                xoff2oq_map_decoder.set_name("elb0.pb.pbc.port_2.cfg_oq_xoff2oq.decoder");
                if (tm_sw_cfg_write_enabled()) {
                    xoff2oq_map_decoder.show();

                    data <<"pbc_csr.port_2.cfg_oq_xoff2oq.all: 0x" << pbc_csr.port_2.cfg_oq_xoff2oq.all() << endl;
data <<"pbc_csr.port_2.cfg_oq_xoff2oq.map: 0x" << pbc_csr.port_2.cfg_oq_xoff2oq.map() << endl;

                    data <<"pbc_csr.port_2.cfg_mac_xoff.all: 0x" << pbc_csr.port_2.cfg_mac_xoff.all() << endl;
data <<"pbc_csr.port_2.cfg_mac_xoff.enable: 0x" << pbc_csr.port_2.cfg_mac_xoff.enable() << endl;


                    pbc_csr.port_2.cfg_oq_xoff2oq.write();
                    pbc_csr.port_2.cfg_mac_xoff.write();
                }
                break;
            }
        case ELBA_TM_PORT_UPLINK_3:
            {
                pbc_csr.port_3.cfg_oq_xoff2oq.read();
                xoff2oq_map_val = pbc_csr.port_3.cfg_oq_xoff2oq.map();

                cpp_helper.set_slc(xoff2oq_map_val, xoff_cos, oq*3, ((oq+1)*3)-1);

                // TODO: write to disable xoff by default
                // pbc_csr.port_3.cfg_oq_xoff2oq.map(xoff2oq_map_val);

                pbc_csr.port_3.cfg_mac_xoff.read();
                xoff_enable_val = pbc_csr.port_3.cfg_mac_xoff.enable().convert_to<uint32_t>();
                if (xoff_enable) {
                    xoff_enable_val |= 1<<xoff_cos;
                } else {
                    xoff_enable_val &= ~(1<<xoff_cos);
                }
                // TODO: write to disable xoff by default
                // pbc_csr.port_3.cfg_mac_xoff.enable(xoff_enable_val);

                xoff2oq_map_decoder.all(pbc_csr.port_3.cfg_oq_xoff2oq.map());
                xoff2oq_map_decoder.set_name("elb0.pb.pbc.port_3.cfg_oq_xoff2oq.decoder");
                if (tm_sw_cfg_write_enabled()) {
                    xoff2oq_map_decoder.show();

                    data <<"pbc_csr.port_3.cfg_oq_xoff2oq.all: 0x" << pbc_csr.port_3.cfg_oq_xoff2oq.all() << endl;
data <<"pbc_csr.port_3.cfg_oq_xoff2oq.map: 0x" << pbc_csr.port_3.cfg_oq_xoff2oq.map() << endl;

                    data <<"pbc_csr.port_3.cfg_mac_xoff.all: 0x" << pbc_csr.port_3.cfg_mac_xoff.all() << endl;
data <<"pbc_csr.port_3.cfg_mac_xoff.enable: 0x" << pbc_csr.port_3.cfg_mac_xoff.enable() << endl;


                    pbc_csr.port_3.cfg_oq_xoff2oq.write();
                    pbc_csr.port_3.cfg_mac_xoff.write();
                }
                break;
            }
        default:
            return sdk::SDK_RET_INVALID_ARG;
    }
    SDK_TRACE_DEBUG("%s", data.str().c_str());

    return sdk::SDK_RET_OK;
}

sdk_ret_t
elba_tm_scheduler_map_update_l0 (uint32_t port,
                                        uint32_t node,
                                        tm_queue_node_params_t *node_params)
{
#ifdef TODO_ELBA
    stringstream data;
    data << hex << endl;
    elb_top_csr_t &elb0 = ELB_BLK_REG_MODEL_ACCESS(elb_top_csr_t, 0, 0);
    elb_pbc_csr_t &pbc_csr = elb0.pb.pbc;
    (void)pbc_csr;
    cpp_int node_val;
    cpp_int strict_val;

    uint32_t max_nodes = ELBA_TM_COUNT_L0_NODES;

    if (node >= max_nodes) {
        SDK_TRACE_ERR("node %u exceeds the number of valid level "
                      "0 nodes in port %u",
                      node, port);
        return sdk::SDK_RET_INVALID_ARG;
    }

    switch(port) {
        case ELBA_TM_PORT_UPLINK_0:
            {
                pbc_csr.port_0.cfg_oq_arb_l1_selection.read();
                pbc_csr.port_0.cfg_oq_arb_l1_strict.read();

                strict_val = pbc_csr.port_0.cfg_oq_arb_l1_strict.priority();
                cpp_helper.set_slc(strict_val,
                                    node_params->sched_type == TM_SCHED_TYPE_STRICT ? 1 : 0,
                                    node,
                                    node);

                // Reset the current node's association in all the parent level
                // nodes
                // 0
                node_val = pbc_csr.port_0.cfg_oq_arb_l1_selection.node_0();
                // Associate/disassociate the current node with the parent node
                cpp_helper.set_slc(node_val,
                                    node_params->parent_node == 0 ? 1 : 0,
                                    node,
                                    node);
                pbc_csr.port_0.cfg_oq_arb_l1_selection.node_0(node_val);
                // 1
                node_val = pbc_csr.port_0.cfg_oq_arb_l1_selection.node_1();
                // Associate/disassociate the current node with the parent node
                cpp_helper.set_slc(node_val,
                                    node_params->parent_node == 1 ? 1 : 0,
                                    node,
                                    node);
                pbc_csr.port_0.cfg_oq_arb_l1_selection.node_1(node_val);
                // 2
                node_val = pbc_csr.port_0.cfg_oq_arb_l1_selection.node_2();
                // Associate/disassociate the current node with the parent node
                cpp_helper.set_slc(node_val,
                                    node_params->parent_node == 2 ? 1 : 0,
                                    node,
                                    node);
                pbc_csr.port_0.cfg_oq_arb_l1_selection.node_2(node_val);
                // 3
                node_val = pbc_csr.port_0.cfg_oq_arb_l1_selection.node_3();
                // Associate/disassociate the current node with the parent node
                cpp_helper.set_slc(node_val,
                                    node_params->parent_node == 3 ? 1 : 0,
                                    node,
                                    node);
                pbc_csr.port_0.cfg_oq_arb_l1_selection.node_3(node_val);
                // 4
                node_val = pbc_csr.port_0.cfg_oq_arb_l1_selection.node_4();
                // Associate/disassociate the current node with the parent node
                cpp_helper.set_slc(node_val,
                                    node_params->parent_node == 4 ? 1 : 0,
                                    node,
                                    node);
                pbc_csr.port_0.cfg_oq_arb_l1_selection.node_4(node_val);
                // 5
                node_val = pbc_csr.port_0.cfg_oq_arb_l1_selection.node_5();
                // Associate/disassociate the current node with the parent node
                cpp_helper.set_slc(node_val,
                                    node_params->parent_node == 5 ? 1 : 0,
                                    node,
                                    node);
                pbc_csr.port_0.cfg_oq_arb_l1_selection.node_5(node_val);
                // 6
                node_val = pbc_csr.port_0.cfg_oq_arb_l1_selection.node_6();
                // Associate/disassociate the current node with the parent node
                cpp_helper.set_slc(node_val,
                                    node_params->parent_node == 6 ? 1 : 0,
                                    node,
                                    node);
                pbc_csr.port_0.cfg_oq_arb_l1_selection.node_6(node_val);
                // 7
                node_val = pbc_csr.port_0.cfg_oq_arb_l1_selection.node_7();
                // Associate/disassociate the current node with the parent node
                cpp_helper.set_slc(node_val,
                                    node_params->parent_node == 7 ? 1 : 0,
                                    node,
                                    node);
                pbc_csr.port_0.cfg_oq_arb_l1_selection.node_7(node_val);
                // 8
                node_val = pbc_csr.port_0.cfg_oq_arb_l1_selection.node_8();
                // Associate/disassociate the current node with the parent node
                cpp_helper.set_slc(node_val,
                                    node_params->parent_node == 8 ? 1 : 0,
                                    node,
                                    node);
                pbc_csr.port_0.cfg_oq_arb_l1_selection.node_8(node_val);
                // 9
                node_val = pbc_csr.port_0.cfg_oq_arb_l1_selection.node_9();
                // Associate/disassociate the current node with the parent node
                cpp_helper.set_slc(node_val,
                                    node_params->parent_node == 9 ? 1 : 0,
                                    node,
                                    node);
                pbc_csr.port_0.cfg_oq_arb_l1_selection.node_9(node_val);
                // 10
                node_val = pbc_csr.port_0.cfg_oq_arb_l1_selection.node_10();
                // Associate/disassociate the current node with the parent node
                cpp_helper.set_slc(node_val,
                                    node_params->parent_node == 10 ? 1 : 0,
                                    node,
                                    node);
                pbc_csr.port_0.cfg_oq_arb_l1_selection.node_10(node_val);
                // 11
                node_val = pbc_csr.port_0.cfg_oq_arb_l1_selection.node_11();
                // Associate/disassociate the current node with the parent node
                cpp_helper.set_slc(node_val,
                                    node_params->parent_node == 11 ? 1 : 0,
                                    node,
                                    node);
                pbc_csr.port_0.cfg_oq_arb_l1_selection.node_11(node_val);
                // 12
                node_val = pbc_csr.port_0.cfg_oq_arb_l1_selection.node_12();
                // Associate/disassociate the current node with the parent node
                cpp_helper.set_slc(node_val,
                                    node_params->parent_node == 12 ? 1 : 0,
                                    node,
                                    node);
                pbc_csr.port_0.cfg_oq_arb_l1_selection.node_12(node_val);
                // 13
                node_val = pbc_csr.port_0.cfg_oq_arb_l1_selection.node_13();
                // Associate/disassociate the current node with the parent node
                cpp_helper.set_slc(node_val,
                                    node_params->parent_node == 13 ? 1 : 0,
                                    node,
                                    node);
                pbc_csr.port_0.cfg_oq_arb_l1_selection.node_13(node_val);
                // 14
                node_val = pbc_csr.port_0.cfg_oq_arb_l1_selection.node_14();
                // Associate/disassociate the current node with the parent node
                cpp_helper.set_slc(node_val,
                                    node_params->parent_node == 14 ? 1 : 0,
                                    node,
                                    node);
                pbc_csr.port_0.cfg_oq_arb_l1_selection.node_14(node_val);
                // 15
                node_val = pbc_csr.port_0.cfg_oq_arb_l1_selection.node_15();
                // Associate/disassociate the current node with the parent node
                cpp_helper.set_slc(node_val,
                                    node_params->parent_node == 15 ? 1 : 0,
                                    node,
                                    node);
                pbc_csr.port_0.cfg_oq_arb_l1_selection.node_15(node_val);
                pbc_csr.port_0.cfg_oq_arb_l1_strict.priority(strict_val);

                /* Write the registers */
                if (tm_sw_cfg_write_enabled()) {
                    data <<"pbc_csr.port_0.cfg_oq_arb_l1_selection.all: 0x" << pbc_csr.port_0.cfg_oq_arb_l1_selection.all() << endl;
data <<"pbc_csr.port_0.cfg_oq_arb_l1_selection.node_13: 0x" << pbc_csr.port_0.cfg_oq_arb_l1_selection.node_13() << endl;
data <<"pbc_csr.port_0.cfg_oq_arb_l1_selection.node_14: 0x" << pbc_csr.port_0.cfg_oq_arb_l1_selection.node_14() << endl;
data <<"pbc_csr.port_0.cfg_oq_arb_l1_selection.node_9: 0x" << pbc_csr.port_0.cfg_oq_arb_l1_selection.node_9() << endl;
data <<"pbc_csr.port_0.cfg_oq_arb_l1_selection.node_8: 0x" << pbc_csr.port_0.cfg_oq_arb_l1_selection.node_8() << endl;
data <<"pbc_csr.port_0.cfg_oq_arb_l1_selection.node_7: 0x" << pbc_csr.port_0.cfg_oq_arb_l1_selection.node_7() << endl;
data <<"pbc_csr.port_0.cfg_oq_arb_l1_selection.node_6: 0x" << pbc_csr.port_0.cfg_oq_arb_l1_selection.node_6() << endl;
data <<"pbc_csr.port_0.cfg_oq_arb_l1_selection.node_5: 0x" << pbc_csr.port_0.cfg_oq_arb_l1_selection.node_5() << endl;
data <<"pbc_csr.port_0.cfg_oq_arb_l1_selection.node_4: 0x" << pbc_csr.port_0.cfg_oq_arb_l1_selection.node_4() << endl;
data <<"pbc_csr.port_0.cfg_oq_arb_l1_selection.node_3: 0x" << pbc_csr.port_0.cfg_oq_arb_l1_selection.node_3() << endl;
data <<"pbc_csr.port_0.cfg_oq_arb_l1_selection.node_2: 0x" << pbc_csr.port_0.cfg_oq_arb_l1_selection.node_2() << endl;
data <<"pbc_csr.port_0.cfg_oq_arb_l1_selection.node_1: 0x" << pbc_csr.port_0.cfg_oq_arb_l1_selection.node_1() << endl;
data <<"pbc_csr.port_0.cfg_oq_arb_l1_selection.node_0: 0x" << pbc_csr.port_0.cfg_oq_arb_l1_selection.node_0() << endl;
data <<"pbc_csr.port_0.cfg_oq_arb_l1_selection.node_11: 0x" << pbc_csr.port_0.cfg_oq_arb_l1_selection.node_11() << endl;
data <<"pbc_csr.port_0.cfg_oq_arb_l1_selection.node_10: 0x" << pbc_csr.port_0.cfg_oq_arb_l1_selection.node_10() << endl;
data <<"pbc_csr.port_0.cfg_oq_arb_l1_selection.node_15: 0x" << pbc_csr.port_0.cfg_oq_arb_l1_selection.node_15() << endl;
data <<"pbc_csr.port_0.cfg_oq_arb_l1_selection.node_12: 0x" << pbc_csr.port_0.cfg_oq_arb_l1_selection.node_12() << endl;

                    data <<"pbc_csr.port_0.cfg_oq_arb_l1_strict.all: 0x" << pbc_csr.port_0.cfg_oq_arb_l1_strict.all() << endl;
data <<"pbc_csr.port_0.cfg_oq_arb_l1_strict.priority: 0x" << pbc_csr.port_0.cfg_oq_arb_l1_strict.priority() << endl;


                    pbc_csr.port_0.cfg_oq_arb_l1_selection.write();
                    pbc_csr.port_0.cfg_oq_arb_l1_strict.write();
                }
                break;
            }
        case ELBA_TM_PORT_UPLINK_1:
            {
                pbc_csr.port_1.cfg_oq_arb_l1_selection.read();
                pbc_csr.port_1.cfg_oq_arb_l1_strict.read();

                strict_val = pbc_csr.port_1.cfg_oq_arb_l1_strict.priority();
                cpp_helper.set_slc(strict_val,
                                    node_params->sched_type == TM_SCHED_TYPE_STRICT ? 1 : 0,
                                    node,
                                    node);

                // Reset the current node's association in all the parent level
                // nodes
                // 0
                node_val = pbc_csr.port_1.cfg_oq_arb_l1_selection.node_0();
                // Associate/disassociate the current node with the parent node
                cpp_helper.set_slc(node_val,
                                    node_params->parent_node == 0 ? 1 : 0,
                                    node,
                                    node);
                pbc_csr.port_1.cfg_oq_arb_l1_selection.node_0(node_val);
                // 1
                node_val = pbc_csr.port_1.cfg_oq_arb_l1_selection.node_1();
                // Associate/disassociate the current node with the parent node
                cpp_helper.set_slc(node_val,
                                    node_params->parent_node == 1 ? 1 : 0,
                                    node,
                                    node);
                pbc_csr.port_1.cfg_oq_arb_l1_selection.node_1(node_val);
                // 2
                node_val = pbc_csr.port_1.cfg_oq_arb_l1_selection.node_2();
                // Associate/disassociate the current node with the parent node
                cpp_helper.set_slc(node_val,
                                    node_params->parent_node == 2 ? 1 : 0,
                                    node,
                                    node);
                pbc_csr.port_1.cfg_oq_arb_l1_selection.node_2(node_val);
                // 3
                node_val = pbc_csr.port_1.cfg_oq_arb_l1_selection.node_3();
                // Associate/disassociate the current node with the parent node
                cpp_helper.set_slc(node_val,
                                    node_params->parent_node == 3 ? 1 : 0,
                                    node,
                                    node);
                pbc_csr.port_1.cfg_oq_arb_l1_selection.node_3(node_val);
                // 4
                node_val = pbc_csr.port_1.cfg_oq_arb_l1_selection.node_4();
                // Associate/disassociate the current node with the parent node
                cpp_helper.set_slc(node_val,
                                    node_params->parent_node == 4 ? 1 : 0,
                                    node,
                                    node);
                pbc_csr.port_1.cfg_oq_arb_l1_selection.node_4(node_val);
                // 5
                node_val = pbc_csr.port_1.cfg_oq_arb_l1_selection.node_5();
                // Associate/disassociate the current node with the parent node
                cpp_helper.set_slc(node_val,
                                    node_params->parent_node == 5 ? 1 : 0,
                                    node,
                                    node);
                pbc_csr.port_1.cfg_oq_arb_l1_selection.node_5(node_val);
                // 6
                node_val = pbc_csr.port_1.cfg_oq_arb_l1_selection.node_6();
                // Associate/disassociate the current node with the parent node
                cpp_helper.set_slc(node_val,
                                    node_params->parent_node == 6 ? 1 : 0,
                                    node,
                                    node);
                pbc_csr.port_1.cfg_oq_arb_l1_selection.node_6(node_val);
                // 7
                node_val = pbc_csr.port_1.cfg_oq_arb_l1_selection.node_7();
                // Associate/disassociate the current node with the parent node
                cpp_helper.set_slc(node_val,
                                    node_params->parent_node == 7 ? 1 : 0,
                                    node,
                                    node);
                pbc_csr.port_1.cfg_oq_arb_l1_selection.node_7(node_val);
                // 8
                node_val = pbc_csr.port_1.cfg_oq_arb_l1_selection.node_8();
                // Associate/disassociate the current node with the parent node
                cpp_helper.set_slc(node_val,
                                    node_params->parent_node == 8 ? 1 : 0,
                                    node,
                                    node);
                pbc_csr.port_1.cfg_oq_arb_l1_selection.node_8(node_val);
                // 9
                node_val = pbc_csr.port_1.cfg_oq_arb_l1_selection.node_9();
                // Associate/disassociate the current node with the parent node
                cpp_helper.set_slc(node_val,
                                    node_params->parent_node == 9 ? 1 : 0,
                                    node,
                                    node);
                pbc_csr.port_1.cfg_oq_arb_l1_selection.node_9(node_val);
                // 10
                node_val = pbc_csr.port_1.cfg_oq_arb_l1_selection.node_10();
                // Associate/disassociate the current node with the parent node
                cpp_helper.set_slc(node_val,
                                    node_params->parent_node == 10 ? 1 : 0,
                                    node,
                                    node);
                pbc_csr.port_1.cfg_oq_arb_l1_selection.node_10(node_val);
                // 11
                node_val = pbc_csr.port_1.cfg_oq_arb_l1_selection.node_11();
                // Associate/disassociate the current node with the parent node
                cpp_helper.set_slc(node_val,
                                    node_params->parent_node == 11 ? 1 : 0,
                                    node,
                                    node);
                pbc_csr.port_1.cfg_oq_arb_l1_selection.node_11(node_val);
                // 12
                node_val = pbc_csr.port_1.cfg_oq_arb_l1_selection.node_12();
                // Associate/disassociate the current node with the parent node
                cpp_helper.set_slc(node_val,
                                    node_params->parent_node == 12 ? 1 : 0,
                                    node,
                                    node);
                pbc_csr.port_1.cfg_oq_arb_l1_selection.node_12(node_val);
                // 13
                node_val = pbc_csr.port_1.cfg_oq_arb_l1_selection.node_13();
                // Associate/disassociate the current node with the parent node
                cpp_helper.set_slc(node_val,
                                    node_params->parent_node == 13 ? 1 : 0,
                                    node,
                                    node);
                pbc_csr.port_1.cfg_oq_arb_l1_selection.node_13(node_val);
                // 14
                node_val = pbc_csr.port_1.cfg_oq_arb_l1_selection.node_14();
                // Associate/disassociate the current node with the parent node
                cpp_helper.set_slc(node_val,
                                    node_params->parent_node == 14 ? 1 : 0,
                                    node,
                                    node);
                pbc_csr.port_1.cfg_oq_arb_l1_selection.node_14(node_val);
                // 15
                node_val = pbc_csr.port_1.cfg_oq_arb_l1_selection.node_15();
                // Associate/disassociate the current node with the parent node
                cpp_helper.set_slc(node_val,
                                    node_params->parent_node == 15 ? 1 : 0,
                                    node,
                                    node);
                pbc_csr.port_1.cfg_oq_arb_l1_selection.node_15(node_val);
                pbc_csr.port_1.cfg_oq_arb_l1_strict.priority(strict_val);

                /* Write the registers */
                if (tm_sw_cfg_write_enabled()) {
                    data <<"pbc_csr.port_1.cfg_oq_arb_l1_selection.all: 0x" << pbc_csr.port_1.cfg_oq_arb_l1_selection.all() << endl;
data <<"pbc_csr.port_1.cfg_oq_arb_l1_selection.node_13: 0x" << pbc_csr.port_1.cfg_oq_arb_l1_selection.node_13() << endl;
data <<"pbc_csr.port_1.cfg_oq_arb_l1_selection.node_14: 0x" << pbc_csr.port_1.cfg_oq_arb_l1_selection.node_14() << endl;
data <<"pbc_csr.port_1.cfg_oq_arb_l1_selection.node_9: 0x" << pbc_csr.port_1.cfg_oq_arb_l1_selection.node_9() << endl;
data <<"pbc_csr.port_1.cfg_oq_arb_l1_selection.node_8: 0x" << pbc_csr.port_1.cfg_oq_arb_l1_selection.node_8() << endl;
data <<"pbc_csr.port_1.cfg_oq_arb_l1_selection.node_7: 0x" << pbc_csr.port_1.cfg_oq_arb_l1_selection.node_7() << endl;
data <<"pbc_csr.port_1.cfg_oq_arb_l1_selection.node_6: 0x" << pbc_csr.port_1.cfg_oq_arb_l1_selection.node_6() << endl;
data <<"pbc_csr.port_1.cfg_oq_arb_l1_selection.node_5: 0x" << pbc_csr.port_1.cfg_oq_arb_l1_selection.node_5() << endl;
data <<"pbc_csr.port_1.cfg_oq_arb_l1_selection.node_4: 0x" << pbc_csr.port_1.cfg_oq_arb_l1_selection.node_4() << endl;
data <<"pbc_csr.port_1.cfg_oq_arb_l1_selection.node_3: 0x" << pbc_csr.port_1.cfg_oq_arb_l1_selection.node_3() << endl;
data <<"pbc_csr.port_1.cfg_oq_arb_l1_selection.node_2: 0x" << pbc_csr.port_1.cfg_oq_arb_l1_selection.node_2() << endl;
data <<"pbc_csr.port_1.cfg_oq_arb_l1_selection.node_1: 0x" << pbc_csr.port_1.cfg_oq_arb_l1_selection.node_1() << endl;
data <<"pbc_csr.port_1.cfg_oq_arb_l1_selection.node_0: 0x" << pbc_csr.port_1.cfg_oq_arb_l1_selection.node_0() << endl;
data <<"pbc_csr.port_1.cfg_oq_arb_l1_selection.node_11: 0x" << pbc_csr.port_1.cfg_oq_arb_l1_selection.node_11() << endl;
data <<"pbc_csr.port_1.cfg_oq_arb_l1_selection.node_10: 0x" << pbc_csr.port_1.cfg_oq_arb_l1_selection.node_10() << endl;
data <<"pbc_csr.port_1.cfg_oq_arb_l1_selection.node_15: 0x" << pbc_csr.port_1.cfg_oq_arb_l1_selection.node_15() << endl;
data <<"pbc_csr.port_1.cfg_oq_arb_l1_selection.node_12: 0x" << pbc_csr.port_1.cfg_oq_arb_l1_selection.node_12() << endl;

                    data <<"pbc_csr.port_1.cfg_oq_arb_l1_strict.all: 0x" << pbc_csr.port_1.cfg_oq_arb_l1_strict.all() << endl;
data <<"pbc_csr.port_1.cfg_oq_arb_l1_strict.priority: 0x" << pbc_csr.port_1.cfg_oq_arb_l1_strict.priority() << endl;


                    pbc_csr.port_1.cfg_oq_arb_l1_selection.write();
                    pbc_csr.port_1.cfg_oq_arb_l1_strict.write();
                }
                break;
            }
        case ELBA_TM_PORT_UPLINK_2:
            {
                pbc_csr.port_2.cfg_oq_arb_l1_selection.read();
                pbc_csr.port_2.cfg_oq_arb_l1_strict.read();

                strict_val = pbc_csr.port_2.cfg_oq_arb_l1_strict.priority();
                cpp_helper.set_slc(strict_val,
                                    node_params->sched_type == TM_SCHED_TYPE_STRICT ? 1 : 0,
                                    node,
                                    node);

                // Reset the current node's association in all the parent level
                // nodes
                // 0
                node_val = pbc_csr.port_2.cfg_oq_arb_l1_selection.node_0();
                // Associate/disassociate the current node with the parent node
                cpp_helper.set_slc(node_val,
                                    node_params->parent_node == 0 ? 1 : 0,
                                    node,
                                    node);
                pbc_csr.port_2.cfg_oq_arb_l1_selection.node_0(node_val);
                // 1
                node_val = pbc_csr.port_2.cfg_oq_arb_l1_selection.node_1();
                // Associate/disassociate the current node with the parent node
                cpp_helper.set_slc(node_val,
                                    node_params->parent_node == 1 ? 1 : 0,
                                    node,
                                    node);
                pbc_csr.port_2.cfg_oq_arb_l1_selection.node_1(node_val);
                // 2
                node_val = pbc_csr.port_2.cfg_oq_arb_l1_selection.node_2();
                // Associate/disassociate the current node with the parent node
                cpp_helper.set_slc(node_val,
                                    node_params->parent_node == 2 ? 1 : 0,
                                    node,
                                    node);
                pbc_csr.port_2.cfg_oq_arb_l1_selection.node_2(node_val);
                // 3
                node_val = pbc_csr.port_2.cfg_oq_arb_l1_selection.node_3();
                // Associate/disassociate the current node with the parent node
                cpp_helper.set_slc(node_val,
                                    node_params->parent_node == 3 ? 1 : 0,
                                    node,
                                    node);
                pbc_csr.port_2.cfg_oq_arb_l1_selection.node_3(node_val);
                // 4
                node_val = pbc_csr.port_2.cfg_oq_arb_l1_selection.node_4();
                // Associate/disassociate the current node with the parent node
                cpp_helper.set_slc(node_val,
                                    node_params->parent_node == 4 ? 1 : 0,
                                    node,
                                    node);
                pbc_csr.port_2.cfg_oq_arb_l1_selection.node_4(node_val);
                // 5
                node_val = pbc_csr.port_2.cfg_oq_arb_l1_selection.node_5();
                // Associate/disassociate the current node with the parent node
                cpp_helper.set_slc(node_val,
                                    node_params->parent_node == 5 ? 1 : 0,
                                    node,
                                    node);
                pbc_csr.port_2.cfg_oq_arb_l1_selection.node_5(node_val);
                // 6
                node_val = pbc_csr.port_2.cfg_oq_arb_l1_selection.node_6();
                // Associate/disassociate the current node with the parent node
                cpp_helper.set_slc(node_val,
                                    node_params->parent_node == 6 ? 1 : 0,
                                    node,
                                    node);
                pbc_csr.port_2.cfg_oq_arb_l1_selection.node_6(node_val);
                // 7
                node_val = pbc_csr.port_2.cfg_oq_arb_l1_selection.node_7();
                // Associate/disassociate the current node with the parent node
                cpp_helper.set_slc(node_val,
                                    node_params->parent_node == 7 ? 1 : 0,
                                    node,
                                    node);
                pbc_csr.port_2.cfg_oq_arb_l1_selection.node_7(node_val);
                // 8
                node_val = pbc_csr.port_2.cfg_oq_arb_l1_selection.node_8();
                // Associate/disassociate the current node with the parent node
                cpp_helper.set_slc(node_val,
                                    node_params->parent_node == 8 ? 1 : 0,
                                    node,
                                    node);
                pbc_csr.port_2.cfg_oq_arb_l1_selection.node_8(node_val);
                // 9
                node_val = pbc_csr.port_2.cfg_oq_arb_l1_selection.node_9();
                // Associate/disassociate the current node with the parent node
                cpp_helper.set_slc(node_val,
                                    node_params->parent_node == 9 ? 1 : 0,
                                    node,
                                    node);
                pbc_csr.port_2.cfg_oq_arb_l1_selection.node_9(node_val);
                // 10
                node_val = pbc_csr.port_2.cfg_oq_arb_l1_selection.node_10();
                // Associate/disassociate the current node with the parent node
                cpp_helper.set_slc(node_val,
                                    node_params->parent_node == 10 ? 1 : 0,
                                    node,
                                    node);
                pbc_csr.port_2.cfg_oq_arb_l1_selection.node_10(node_val);
                // 11
                node_val = pbc_csr.port_2.cfg_oq_arb_l1_selection.node_11();
                // Associate/disassociate the current node with the parent node
                cpp_helper.set_slc(node_val,
                                    node_params->parent_node == 11 ? 1 : 0,
                                    node,
                                    node);
                pbc_csr.port_2.cfg_oq_arb_l1_selection.node_11(node_val);
                // 12
                node_val = pbc_csr.port_2.cfg_oq_arb_l1_selection.node_12();
                // Associate/disassociate the current node with the parent node
                cpp_helper.set_slc(node_val,
                                    node_params->parent_node == 12 ? 1 : 0,
                                    node,
                                    node);
                pbc_csr.port_2.cfg_oq_arb_l1_selection.node_12(node_val);
                // 13
                node_val = pbc_csr.port_2.cfg_oq_arb_l1_selection.node_13();
                // Associate/disassociate the current node with the parent node
                cpp_helper.set_slc(node_val,
                                    node_params->parent_node == 13 ? 1 : 0,
                                    node,
                                    node);
                pbc_csr.port_2.cfg_oq_arb_l1_selection.node_13(node_val);
                // 14
                node_val = pbc_csr.port_2.cfg_oq_arb_l1_selection.node_14();
                // Associate/disassociate the current node with the parent node
                cpp_helper.set_slc(node_val,
                                    node_params->parent_node == 14 ? 1 : 0,
                                    node,
                                    node);
                pbc_csr.port_2.cfg_oq_arb_l1_selection.node_14(node_val);
                // 15
                node_val = pbc_csr.port_2.cfg_oq_arb_l1_selection.node_15();
                // Associate/disassociate the current node with the parent node
                cpp_helper.set_slc(node_val,
                                    node_params->parent_node == 15 ? 1 : 0,
                                    node,
                                    node);
                pbc_csr.port_2.cfg_oq_arb_l1_selection.node_15(node_val);
                pbc_csr.port_2.cfg_oq_arb_l1_strict.priority(strict_val);

                /* Write the registers */
                if (tm_sw_cfg_write_enabled()) {
                    data <<"pbc_csr.port_2.cfg_oq_arb_l1_selection.all: 0x" << pbc_csr.port_2.cfg_oq_arb_l1_selection.all() << endl;
data <<"pbc_csr.port_2.cfg_oq_arb_l1_selection.node_13: 0x" << pbc_csr.port_2.cfg_oq_arb_l1_selection.node_13() << endl;
data <<"pbc_csr.port_2.cfg_oq_arb_l1_selection.node_14: 0x" << pbc_csr.port_2.cfg_oq_arb_l1_selection.node_14() << endl;
data <<"pbc_csr.port_2.cfg_oq_arb_l1_selection.node_9: 0x" << pbc_csr.port_2.cfg_oq_arb_l1_selection.node_9() << endl;
data <<"pbc_csr.port_2.cfg_oq_arb_l1_selection.node_8: 0x" << pbc_csr.port_2.cfg_oq_arb_l1_selection.node_8() << endl;
data <<"pbc_csr.port_2.cfg_oq_arb_l1_selection.node_7: 0x" << pbc_csr.port_2.cfg_oq_arb_l1_selection.node_7() << endl;
data <<"pbc_csr.port_2.cfg_oq_arb_l1_selection.node_6: 0x" << pbc_csr.port_2.cfg_oq_arb_l1_selection.node_6() << endl;
data <<"pbc_csr.port_2.cfg_oq_arb_l1_selection.node_5: 0x" << pbc_csr.port_2.cfg_oq_arb_l1_selection.node_5() << endl;
data <<"pbc_csr.port_2.cfg_oq_arb_l1_selection.node_4: 0x" << pbc_csr.port_2.cfg_oq_arb_l1_selection.node_4() << endl;
data <<"pbc_csr.port_2.cfg_oq_arb_l1_selection.node_3: 0x" << pbc_csr.port_2.cfg_oq_arb_l1_selection.node_3() << endl;
data <<"pbc_csr.port_2.cfg_oq_arb_l1_selection.node_2: 0x" << pbc_csr.port_2.cfg_oq_arb_l1_selection.node_2() << endl;
data <<"pbc_csr.port_2.cfg_oq_arb_l1_selection.node_1: 0x" << pbc_csr.port_2.cfg_oq_arb_l1_selection.node_1() << endl;
data <<"pbc_csr.port_2.cfg_oq_arb_l1_selection.node_0: 0x" << pbc_csr.port_2.cfg_oq_arb_l1_selection.node_0() << endl;
data <<"pbc_csr.port_2.cfg_oq_arb_l1_selection.node_11: 0x" << pbc_csr.port_2.cfg_oq_arb_l1_selection.node_11() << endl;
data <<"pbc_csr.port_2.cfg_oq_arb_l1_selection.node_10: 0x" << pbc_csr.port_2.cfg_oq_arb_l1_selection.node_10() << endl;
data <<"pbc_csr.port_2.cfg_oq_arb_l1_selection.node_15: 0x" << pbc_csr.port_2.cfg_oq_arb_l1_selection.node_15() << endl;
data <<"pbc_csr.port_2.cfg_oq_arb_l1_selection.node_12: 0x" << pbc_csr.port_2.cfg_oq_arb_l1_selection.node_12() << endl;

                    data <<"pbc_csr.port_2.cfg_oq_arb_l1_strict.all: 0x" << pbc_csr.port_2.cfg_oq_arb_l1_strict.all() << endl;
data <<"pbc_csr.port_2.cfg_oq_arb_l1_strict.priority: 0x" << pbc_csr.port_2.cfg_oq_arb_l1_strict.priority() << endl;


                    pbc_csr.port_2.cfg_oq_arb_l1_selection.write();
                    pbc_csr.port_2.cfg_oq_arb_l1_strict.write();
                }
                break;
            }
        case ELBA_TM_PORT_UPLINK_3:
            {
                pbc_csr.port_3.cfg_oq_arb_l1_selection.read();
                pbc_csr.port_3.cfg_oq_arb_l1_strict.read();

                strict_val = pbc_csr.port_3.cfg_oq_arb_l1_strict.priority();
                cpp_helper.set_slc(strict_val,
                                    node_params->sched_type == TM_SCHED_TYPE_STRICT ? 1 : 0,
                                    node,
                                    node);

                // Reset the current node's association in all the parent level
                // nodes
                // 0
                node_val = pbc_csr.port_3.cfg_oq_arb_l1_selection.node_0();
                // Associate/disassociate the current node with the parent node
                cpp_helper.set_slc(node_val,
                                    node_params->parent_node == 0 ? 1 : 0,
                                    node,
                                    node);
                pbc_csr.port_3.cfg_oq_arb_l1_selection.node_0(node_val);
                // 1
                node_val = pbc_csr.port_3.cfg_oq_arb_l1_selection.node_1();
                // Associate/disassociate the current node with the parent node
                cpp_helper.set_slc(node_val,
                                    node_params->parent_node == 1 ? 1 : 0,
                                    node,
                                    node);
                pbc_csr.port_3.cfg_oq_arb_l1_selection.node_1(node_val);
                // 2
                node_val = pbc_csr.port_3.cfg_oq_arb_l1_selection.node_2();
                // Associate/disassociate the current node with the parent node
                cpp_helper.set_slc(node_val,
                                    node_params->parent_node == 2 ? 1 : 0,
                                    node,
                                    node);
                pbc_csr.port_3.cfg_oq_arb_l1_selection.node_2(node_val);
                // 3
                node_val = pbc_csr.port_3.cfg_oq_arb_l1_selection.node_3();
                // Associate/disassociate the current node with the parent node
                cpp_helper.set_slc(node_val,
                                    node_params->parent_node == 3 ? 1 : 0,
                                    node,
                                    node);
                pbc_csr.port_3.cfg_oq_arb_l1_selection.node_3(node_val);
                // 4
                node_val = pbc_csr.port_3.cfg_oq_arb_l1_selection.node_4();
                // Associate/disassociate the current node with the parent node
                cpp_helper.set_slc(node_val,
                                    node_params->parent_node == 4 ? 1 : 0,
                                    node,
                                    node);
                pbc_csr.port_3.cfg_oq_arb_l1_selection.node_4(node_val);
                // 5
                node_val = pbc_csr.port_3.cfg_oq_arb_l1_selection.node_5();
                // Associate/disassociate the current node with the parent node
                cpp_helper.set_slc(node_val,
                                    node_params->parent_node == 5 ? 1 : 0,
                                    node,
                                    node);
                pbc_csr.port_3.cfg_oq_arb_l1_selection.node_5(node_val);
                // 6
                node_val = pbc_csr.port_3.cfg_oq_arb_l1_selection.node_6();
                // Associate/disassociate the current node with the parent node
                cpp_helper.set_slc(node_val,
                                    node_params->parent_node == 6 ? 1 : 0,
                                    node,
                                    node);
                pbc_csr.port_3.cfg_oq_arb_l1_selection.node_6(node_val);
                // 7
                node_val = pbc_csr.port_3.cfg_oq_arb_l1_selection.node_7();
                // Associate/disassociate the current node with the parent node
                cpp_helper.set_slc(node_val,
                                    node_params->parent_node == 7 ? 1 : 0,
                                    node,
                                    node);
                pbc_csr.port_3.cfg_oq_arb_l1_selection.node_7(node_val);
                // 8
                node_val = pbc_csr.port_3.cfg_oq_arb_l1_selection.node_8();
                // Associate/disassociate the current node with the parent node
                cpp_helper.set_slc(node_val,
                                    node_params->parent_node == 8 ? 1 : 0,
                                    node,
                                    node);
                pbc_csr.port_3.cfg_oq_arb_l1_selection.node_8(node_val);
                // 9
                node_val = pbc_csr.port_3.cfg_oq_arb_l1_selection.node_9();
                // Associate/disassociate the current node with the parent node
                cpp_helper.set_slc(node_val,
                                    node_params->parent_node == 9 ? 1 : 0,
                                    node,
                                    node);
                pbc_csr.port_3.cfg_oq_arb_l1_selection.node_9(node_val);
                // 10
                node_val = pbc_csr.port_3.cfg_oq_arb_l1_selection.node_10();
                // Associate/disassociate the current node with the parent node
                cpp_helper.set_slc(node_val,
                                    node_params->parent_node == 10 ? 1 : 0,
                                    node,
                                    node);
                pbc_csr.port_3.cfg_oq_arb_l1_selection.node_10(node_val);
                // 11
                node_val = pbc_csr.port_3.cfg_oq_arb_l1_selection.node_11();
                // Associate/disassociate the current node with the parent node
                cpp_helper.set_slc(node_val,
                                    node_params->parent_node == 11 ? 1 : 0,
                                    node,
                                    node);
                pbc_csr.port_3.cfg_oq_arb_l1_selection.node_11(node_val);
                // 12
                node_val = pbc_csr.port_3.cfg_oq_arb_l1_selection.node_12();
                // Associate/disassociate the current node with the parent node
                cpp_helper.set_slc(node_val,
                                    node_params->parent_node == 12 ? 1 : 0,
                                    node,
                                    node);
                pbc_csr.port_3.cfg_oq_arb_l1_selection.node_12(node_val);
                // 13
                node_val = pbc_csr.port_3.cfg_oq_arb_l1_selection.node_13();
                // Associate/disassociate the current node with the parent node
                cpp_helper.set_slc(node_val,
                                    node_params->parent_node == 13 ? 1 : 0,
                                    node,
                                    node);
                pbc_csr.port_3.cfg_oq_arb_l1_selection.node_13(node_val);
                // 14
                node_val = pbc_csr.port_3.cfg_oq_arb_l1_selection.node_14();
                // Associate/disassociate the current node with the parent node
                cpp_helper.set_slc(node_val,
                                    node_params->parent_node == 14 ? 1 : 0,
                                    node,
                                    node);
                pbc_csr.port_3.cfg_oq_arb_l1_selection.node_14(node_val);
                // 15
                node_val = pbc_csr.port_3.cfg_oq_arb_l1_selection.node_15();
                // Associate/disassociate the current node with the parent node
                cpp_helper.set_slc(node_val,
                                    node_params->parent_node == 15 ? 1 : 0,
                                    node,
                                    node);
                pbc_csr.port_3.cfg_oq_arb_l1_selection.node_15(node_val);
                pbc_csr.port_3.cfg_oq_arb_l1_strict.priority(strict_val);

                /* Write the registers */
                if (tm_sw_cfg_write_enabled()) {
                    data <<"pbc_csr.port_3.cfg_oq_arb_l1_selection.all: 0x" << pbc_csr.port_3.cfg_oq_arb_l1_selection.all() << endl;
data <<"pbc_csr.port_3.cfg_oq_arb_l1_selection.node_13: 0x" << pbc_csr.port_3.cfg_oq_arb_l1_selection.node_13() << endl;
data <<"pbc_csr.port_3.cfg_oq_arb_l1_selection.node_14: 0x" << pbc_csr.port_3.cfg_oq_arb_l1_selection.node_14() << endl;
data <<"pbc_csr.port_3.cfg_oq_arb_l1_selection.node_9: 0x" << pbc_csr.port_3.cfg_oq_arb_l1_selection.node_9() << endl;
data <<"pbc_csr.port_3.cfg_oq_arb_l1_selection.node_8: 0x" << pbc_csr.port_3.cfg_oq_arb_l1_selection.node_8() << endl;
data <<"pbc_csr.port_3.cfg_oq_arb_l1_selection.node_7: 0x" << pbc_csr.port_3.cfg_oq_arb_l1_selection.node_7() << endl;
data <<"pbc_csr.port_3.cfg_oq_arb_l1_selection.node_6: 0x" << pbc_csr.port_3.cfg_oq_arb_l1_selection.node_6() << endl;
data <<"pbc_csr.port_3.cfg_oq_arb_l1_selection.node_5: 0x" << pbc_csr.port_3.cfg_oq_arb_l1_selection.node_5() << endl;
data <<"pbc_csr.port_3.cfg_oq_arb_l1_selection.node_4: 0x" << pbc_csr.port_3.cfg_oq_arb_l1_selection.node_4() << endl;
data <<"pbc_csr.port_3.cfg_oq_arb_l1_selection.node_3: 0x" << pbc_csr.port_3.cfg_oq_arb_l1_selection.node_3() << endl;
data <<"pbc_csr.port_3.cfg_oq_arb_l1_selection.node_2: 0x" << pbc_csr.port_3.cfg_oq_arb_l1_selection.node_2() << endl;
data <<"pbc_csr.port_3.cfg_oq_arb_l1_selection.node_1: 0x" << pbc_csr.port_3.cfg_oq_arb_l1_selection.node_1() << endl;
data <<"pbc_csr.port_3.cfg_oq_arb_l1_selection.node_0: 0x" << pbc_csr.port_3.cfg_oq_arb_l1_selection.node_0() << endl;
data <<"pbc_csr.port_3.cfg_oq_arb_l1_selection.node_11: 0x" << pbc_csr.port_3.cfg_oq_arb_l1_selection.node_11() << endl;
data <<"pbc_csr.port_3.cfg_oq_arb_l1_selection.node_10: 0x" << pbc_csr.port_3.cfg_oq_arb_l1_selection.node_10() << endl;
data <<"pbc_csr.port_3.cfg_oq_arb_l1_selection.node_15: 0x" << pbc_csr.port_3.cfg_oq_arb_l1_selection.node_15() << endl;
data <<"pbc_csr.port_3.cfg_oq_arb_l1_selection.node_12: 0x" << pbc_csr.port_3.cfg_oq_arb_l1_selection.node_12() << endl;

                    data <<"pbc_csr.port_3.cfg_oq_arb_l1_strict.all: 0x" << pbc_csr.port_3.cfg_oq_arb_l1_strict.all() << endl;
data <<"pbc_csr.port_3.cfg_oq_arb_l1_strict.priority: 0x" << pbc_csr.port_3.cfg_oq_arb_l1_strict.priority() << endl;


                    pbc_csr.port_3.cfg_oq_arb_l1_selection.write();
                    pbc_csr.port_3.cfg_oq_arb_l1_strict.write();
                }
                break;
            }
        default:
            return sdk::SDK_RET_ERR;
    }

    pbc_csr.cfg_sched.read();
    pbc_csr.cfg_sched.dhs_selection( 0*2 );
    if (tm_sw_cfg_write_enabled()) {
        data <<"pbc_csr.cfg_sched.all: 0x" << pbc_csr.cfg_sched.all() << endl;

        pbc_csr.cfg_sched.write();
    }

    pbc_csr.cfg_dhs_mem.address(port*max_nodes + node);
    if (tm_sw_cfg_write_enabled()) {
        data <<"pbc_csr.cfg_dhs_mem.all: 0x" << pbc_csr.cfg_dhs_mem.all() << endl;
data <<"pbc_csr.cfg_dhs_mem.address: 0x" << pbc_csr.cfg_dhs_mem.address() << endl;

        pbc_csr.cfg_dhs_mem.write();
    }

    uint64_t scheduler_rate;
    if (node_params->sched_type == TM_SCHED_TYPE_STRICT) {
        scheduler_rate = (uint64_t)ELBA_TM_SCHEDULER_RATE_REFRESH_INTERVAL_US*node_params->strict.rate;
        scheduler_rate /= 1000000;
        if (scheduler_rate >= (1ull << 32)) {
            // This should never happen it's in 80 Tbps range
            SDK_ASSERT(0);
        }
    }

    uint32_t quota = (node_params->sched_type == TM_SCHED_TYPE_STRICT ?
                      scheduler_rate : node_params->dwrr.weight);

    pbc_csr.dhs_sched.entry[0].command(1);   //1: overwrite quota and credits
    pbc_csr.dhs_sched.entry[0].current_credit(quota);
    pbc_csr.dhs_sched.entry[0].quota(quota);
    if (tm_sw_cfg_write_enabled()) {
        data <<"pbc_csr.dhs_sched.entry[0].all: 0x" << pbc_csr.dhs_sched.entry[0].all() << endl;

        pbc_csr.dhs_sched.entry[0].write();
    }
    SDK_TRACE_DEBUG("%s", data.str().c_str());
    SDK_TRACE_DEBUG("Updated the output queue scheduler on port %u "
                    "level %u, node %u",
                    port, 0, node);

#endif
    return sdk::SDK_RET_OK;
}

sdk_ret_t
elba_tm_scheduler_map_update_l1 (uint32_t port,
                                        uint32_t node,
                                        tm_queue_node_params_t *node_params)
{
#ifdef TODO_ELBA
    stringstream data;
    data << hex << endl;
    elb_top_csr_t &elb0 = ELB_BLK_REG_MODEL_ACCESS(elb_top_csr_t, 0, 0);
    elb_pbc_csr_t &pbc_csr = elb0.pb.pbc;
    (void)pbc_csr;
    cpp_int node_val;
    cpp_int strict_val;

    uint32_t max_nodes = ELBA_TM_COUNT_L1_NODES;

    if (node >= max_nodes) {
        SDK_TRACE_ERR("node %u exceeds the number of valid level "
                      "1 nodes in port %u",
                      node, port);
        return sdk::SDK_RET_INVALID_ARG;
    }

    switch(port) {
        case ELBA_TM_PORT_UPLINK_0:
            {
                pbc_csr.port_0.cfg_oq_arb_l2_selection.read();
                pbc_csr.port_0.cfg_oq_arb_l2_strict.read();

                strict_val = pbc_csr.port_0.cfg_oq_arb_l2_strict.priority();
                cpp_helper.set_slc(strict_val,
                                    node_params->sched_type == TM_SCHED_TYPE_STRICT ? 1 : 0,
                                    node,
                                    node);

                // Reset the current node's association in all the parent level
                // nodes
                // 0
                node_val = pbc_csr.port_0.cfg_oq_arb_l2_selection.node_0();
                // Associate/disassociate the current node with the parent node
                cpp_helper.set_slc(node_val,
                                    node_params->parent_node == 0 ? 1 : 0,
                                    node,
                                    node);
                pbc_csr.port_0.cfg_oq_arb_l2_selection.node_0(node_val);
                // 1
                node_val = pbc_csr.port_0.cfg_oq_arb_l2_selection.node_1();
                // Associate/disassociate the current node with the parent node
                cpp_helper.set_slc(node_val,
                                    node_params->parent_node == 1 ? 1 : 0,
                                    node,
                                    node);
                pbc_csr.port_0.cfg_oq_arb_l2_selection.node_1(node_val);
                // 2
                node_val = pbc_csr.port_0.cfg_oq_arb_l2_selection.node_2();
                // Associate/disassociate the current node with the parent node
                cpp_helper.set_slc(node_val,
                                    node_params->parent_node == 2 ? 1 : 0,
                                    node,
                                    node);
                pbc_csr.port_0.cfg_oq_arb_l2_selection.node_2(node_val);
                // 3
                node_val = pbc_csr.port_0.cfg_oq_arb_l2_selection.node_3();
                // Associate/disassociate the current node with the parent node
                cpp_helper.set_slc(node_val,
                                    node_params->parent_node == 3 ? 1 : 0,
                                    node,
                                    node);
                pbc_csr.port_0.cfg_oq_arb_l2_selection.node_3(node_val);
                pbc_csr.port_0.cfg_oq_arb_l2_strict.priority(strict_val);

                /* Write the registers */
                if (tm_sw_cfg_write_enabled()) {
                    data <<"pbc_csr.port_0.cfg_oq_arb_l2_selection.all: 0x" << pbc_csr.port_0.cfg_oq_arb_l2_selection.all() << endl;
data <<"pbc_csr.port_0.cfg_oq_arb_l2_selection.node_3: 0x" << pbc_csr.port_0.cfg_oq_arb_l2_selection.node_3() << endl;
data <<"pbc_csr.port_0.cfg_oq_arb_l2_selection.node_2: 0x" << pbc_csr.port_0.cfg_oq_arb_l2_selection.node_2() << endl;
data <<"pbc_csr.port_0.cfg_oq_arb_l2_selection.node_1: 0x" << pbc_csr.port_0.cfg_oq_arb_l2_selection.node_1() << endl;
data <<"pbc_csr.port_0.cfg_oq_arb_l2_selection.node_0: 0x" << pbc_csr.port_0.cfg_oq_arb_l2_selection.node_0() << endl;

                    data <<"pbc_csr.port_0.cfg_oq_arb_l2_strict.all: 0x" << pbc_csr.port_0.cfg_oq_arb_l2_strict.all() << endl;
data <<"pbc_csr.port_0.cfg_oq_arb_l2_strict.priority: 0x" << pbc_csr.port_0.cfg_oq_arb_l2_strict.priority() << endl;


                    pbc_csr.port_0.cfg_oq_arb_l2_selection.write();
                    pbc_csr.port_0.cfg_oq_arb_l2_strict.write();
                }
                break;
            }
        case ELBA_TM_PORT_UPLINK_1:
            {
                pbc_csr.port_1.cfg_oq_arb_l2_selection.read();
                pbc_csr.port_1.cfg_oq_arb_l2_strict.read();

                strict_val = pbc_csr.port_1.cfg_oq_arb_l2_strict.priority();
                cpp_helper.set_slc(strict_val,
                                    node_params->sched_type == TM_SCHED_TYPE_STRICT ? 1 : 0,
                                    node,
                                    node);

                // Reset the current node's association in all the parent level
                // nodes
                // 0
                node_val = pbc_csr.port_1.cfg_oq_arb_l2_selection.node_0();
                // Associate/disassociate the current node with the parent node
                cpp_helper.set_slc(node_val,
                                    node_params->parent_node == 0 ? 1 : 0,
                                    node,
                                    node);
                pbc_csr.port_1.cfg_oq_arb_l2_selection.node_0(node_val);
                // 1
                node_val = pbc_csr.port_1.cfg_oq_arb_l2_selection.node_1();
                // Associate/disassociate the current node with the parent node
                cpp_helper.set_slc(node_val,
                                    node_params->parent_node == 1 ? 1 : 0,
                                    node,
                                    node);
                pbc_csr.port_1.cfg_oq_arb_l2_selection.node_1(node_val);
                // 2
                node_val = pbc_csr.port_1.cfg_oq_arb_l2_selection.node_2();
                // Associate/disassociate the current node with the parent node
                cpp_helper.set_slc(node_val,
                                    node_params->parent_node == 2 ? 1 : 0,
                                    node,
                                    node);
                pbc_csr.port_1.cfg_oq_arb_l2_selection.node_2(node_val);
                // 3
                node_val = pbc_csr.port_1.cfg_oq_arb_l2_selection.node_3();
                // Associate/disassociate the current node with the parent node
                cpp_helper.set_slc(node_val,
                                    node_params->parent_node == 3 ? 1 : 0,
                                    node,
                                    node);
                pbc_csr.port_1.cfg_oq_arb_l2_selection.node_3(node_val);
                pbc_csr.port_1.cfg_oq_arb_l2_strict.priority(strict_val);

                /* Write the registers */
                if (tm_sw_cfg_write_enabled()) {
                    data <<"pbc_csr.port_1.cfg_oq_arb_l2_selection.all: 0x" << pbc_csr.port_1.cfg_oq_arb_l2_selection.all() << endl;
data <<"pbc_csr.port_1.cfg_oq_arb_l2_selection.node_3: 0x" << pbc_csr.port_1.cfg_oq_arb_l2_selection.node_3() << endl;
data <<"pbc_csr.port_1.cfg_oq_arb_l2_selection.node_2: 0x" << pbc_csr.port_1.cfg_oq_arb_l2_selection.node_2() << endl;
data <<"pbc_csr.port_1.cfg_oq_arb_l2_selection.node_1: 0x" << pbc_csr.port_1.cfg_oq_arb_l2_selection.node_1() << endl;
data <<"pbc_csr.port_1.cfg_oq_arb_l2_selection.node_0: 0x" << pbc_csr.port_1.cfg_oq_arb_l2_selection.node_0() << endl;

                    data <<"pbc_csr.port_1.cfg_oq_arb_l2_strict.all: 0x" << pbc_csr.port_1.cfg_oq_arb_l2_strict.all() << endl;
data <<"pbc_csr.port_1.cfg_oq_arb_l2_strict.priority: 0x" << pbc_csr.port_1.cfg_oq_arb_l2_strict.priority() << endl;


                    pbc_csr.port_1.cfg_oq_arb_l2_selection.write();
                    pbc_csr.port_1.cfg_oq_arb_l2_strict.write();
                }
                break;
            }
        case ELBA_TM_PORT_UPLINK_2:
            {
                pbc_csr.port_2.cfg_oq_arb_l2_selection.read();
                pbc_csr.port_2.cfg_oq_arb_l2_strict.read();

                strict_val = pbc_csr.port_2.cfg_oq_arb_l2_strict.priority();
                cpp_helper.set_slc(strict_val,
                                    node_params->sched_type == TM_SCHED_TYPE_STRICT ? 1 : 0,
                                    node,
                                    node);

                // Reset the current node's association in all the parent level
                // nodes
                // 0
                node_val = pbc_csr.port_2.cfg_oq_arb_l2_selection.node_0();
                // Associate/disassociate the current node with the parent node
                cpp_helper.set_slc(node_val,
                                    node_params->parent_node == 0 ? 1 : 0,
                                    node,
                                    node);
                pbc_csr.port_2.cfg_oq_arb_l2_selection.node_0(node_val);
                // 1
                node_val = pbc_csr.port_2.cfg_oq_arb_l2_selection.node_1();
                // Associate/disassociate the current node with the parent node
                cpp_helper.set_slc(node_val,
                                    node_params->parent_node == 1 ? 1 : 0,
                                    node,
                                    node);
                pbc_csr.port_2.cfg_oq_arb_l2_selection.node_1(node_val);
                // 2
                node_val = pbc_csr.port_2.cfg_oq_arb_l2_selection.node_2();
                // Associate/disassociate the current node with the parent node
                cpp_helper.set_slc(node_val,
                                    node_params->parent_node == 2 ? 1 : 0,
                                    node,
                                    node);
                pbc_csr.port_2.cfg_oq_arb_l2_selection.node_2(node_val);
                // 3
                node_val = pbc_csr.port_2.cfg_oq_arb_l2_selection.node_3();
                // Associate/disassociate the current node with the parent node
                cpp_helper.set_slc(node_val,
                                    node_params->parent_node == 3 ? 1 : 0,
                                    node,
                                    node);
                pbc_csr.port_2.cfg_oq_arb_l2_selection.node_3(node_val);
                pbc_csr.port_2.cfg_oq_arb_l2_strict.priority(strict_val);

                /* Write the registers */
                if (tm_sw_cfg_write_enabled()) {
                    data <<"pbc_csr.port_2.cfg_oq_arb_l2_selection.all: 0x" << pbc_csr.port_2.cfg_oq_arb_l2_selection.all() << endl;
data <<"pbc_csr.port_2.cfg_oq_arb_l2_selection.node_3: 0x" << pbc_csr.port_2.cfg_oq_arb_l2_selection.node_3() << endl;
data <<"pbc_csr.port_2.cfg_oq_arb_l2_selection.node_2: 0x" << pbc_csr.port_2.cfg_oq_arb_l2_selection.node_2() << endl;
data <<"pbc_csr.port_2.cfg_oq_arb_l2_selection.node_1: 0x" << pbc_csr.port_2.cfg_oq_arb_l2_selection.node_1() << endl;
data <<"pbc_csr.port_2.cfg_oq_arb_l2_selection.node_0: 0x" << pbc_csr.port_2.cfg_oq_arb_l2_selection.node_0() << endl;

                    data <<"pbc_csr.port_2.cfg_oq_arb_l2_strict.all: 0x" << pbc_csr.port_2.cfg_oq_arb_l2_strict.all() << endl;
data <<"pbc_csr.port_2.cfg_oq_arb_l2_strict.priority: 0x" << pbc_csr.port_2.cfg_oq_arb_l2_strict.priority() << endl;


                    pbc_csr.port_2.cfg_oq_arb_l2_selection.write();
                    pbc_csr.port_2.cfg_oq_arb_l2_strict.write();
                }
                break;
            }
        case ELBA_TM_PORT_UPLINK_3:
            {
                pbc_csr.port_3.cfg_oq_arb_l2_selection.read();
                pbc_csr.port_3.cfg_oq_arb_l2_strict.read();

                strict_val = pbc_csr.port_3.cfg_oq_arb_l2_strict.priority();
                cpp_helper.set_slc(strict_val,
                                    node_params->sched_type == TM_SCHED_TYPE_STRICT ? 1 : 0,
                                    node,
                                    node);

                // Reset the current node's association in all the parent level
                // nodes
                // 0
                node_val = pbc_csr.port_3.cfg_oq_arb_l2_selection.node_0();
                // Associate/disassociate the current node with the parent node
                cpp_helper.set_slc(node_val,
                                    node_params->parent_node == 0 ? 1 : 0,
                                    node,
                                    node);
                pbc_csr.port_3.cfg_oq_arb_l2_selection.node_0(node_val);
                // 1
                node_val = pbc_csr.port_3.cfg_oq_arb_l2_selection.node_1();
                // Associate/disassociate the current node with the parent node
                cpp_helper.set_slc(node_val,
                                    node_params->parent_node == 1 ? 1 : 0,
                                    node,
                                    node);
                pbc_csr.port_3.cfg_oq_arb_l2_selection.node_1(node_val);
                // 2
                node_val = pbc_csr.port_3.cfg_oq_arb_l2_selection.node_2();
                // Associate/disassociate the current node with the parent node
                cpp_helper.set_slc(node_val,
                                    node_params->parent_node == 2 ? 1 : 0,
                                    node,
                                    node);
                pbc_csr.port_3.cfg_oq_arb_l2_selection.node_2(node_val);
                // 3
                node_val = pbc_csr.port_3.cfg_oq_arb_l2_selection.node_3();
                // Associate/disassociate the current node with the parent node
                cpp_helper.set_slc(node_val,
                                    node_params->parent_node == 3 ? 1 : 0,
                                    node,
                                    node);
                pbc_csr.port_3.cfg_oq_arb_l2_selection.node_3(node_val);
                pbc_csr.port_3.cfg_oq_arb_l2_strict.priority(strict_val);

                /* Write the registers */
                if (tm_sw_cfg_write_enabled()) {
                    data <<"pbc_csr.port_3.cfg_oq_arb_l2_selection.all: 0x" << pbc_csr.port_3.cfg_oq_arb_l2_selection.all() << endl;
data <<"pbc_csr.port_3.cfg_oq_arb_l2_selection.node_3: 0x" << pbc_csr.port_3.cfg_oq_arb_l2_selection.node_3() << endl;
data <<"pbc_csr.port_3.cfg_oq_arb_l2_selection.node_2: 0x" << pbc_csr.port_3.cfg_oq_arb_l2_selection.node_2() << endl;
data <<"pbc_csr.port_3.cfg_oq_arb_l2_selection.node_1: 0x" << pbc_csr.port_3.cfg_oq_arb_l2_selection.node_1() << endl;
data <<"pbc_csr.port_3.cfg_oq_arb_l2_selection.node_0: 0x" << pbc_csr.port_3.cfg_oq_arb_l2_selection.node_0() << endl;

                    data <<"pbc_csr.port_3.cfg_oq_arb_l2_strict.all: 0x" << pbc_csr.port_3.cfg_oq_arb_l2_strict.all() << endl;
data <<"pbc_csr.port_3.cfg_oq_arb_l2_strict.priority: 0x" << pbc_csr.port_3.cfg_oq_arb_l2_strict.priority() << endl;


                    pbc_csr.port_3.cfg_oq_arb_l2_selection.write();
                    pbc_csr.port_3.cfg_oq_arb_l2_strict.write();
                }
                break;
            }
        default:
            return sdk::SDK_RET_ERR;
    }

    pbc_csr.cfg_sched.read();
    pbc_csr.cfg_sched.dhs_selection( 1*2 );
    if (tm_sw_cfg_write_enabled()) {
        data <<"pbc_csr.cfg_sched.all: 0x" << pbc_csr.cfg_sched.all() << endl;

        pbc_csr.cfg_sched.write();
    }

    pbc_csr.cfg_dhs_mem.address(port*max_nodes + node);
    if (tm_sw_cfg_write_enabled()) {
        data <<"pbc_csr.cfg_dhs_mem.all: 0x" << pbc_csr.cfg_dhs_mem.all() << endl;
data <<"pbc_csr.cfg_dhs_mem.address: 0x" << pbc_csr.cfg_dhs_mem.address() << endl;

        pbc_csr.cfg_dhs_mem.write();
    }

    uint64_t scheduler_rate;
    if (node_params->sched_type == TM_SCHED_TYPE_STRICT) {
        scheduler_rate = (uint64_t)ELBA_TM_SCHEDULER_RATE_REFRESH_INTERVAL_US*node_params->strict.rate;
        scheduler_rate /= 1000000;
        if (scheduler_rate >= (1ull << 32)) {
            // This should never happen it's in 80 Tbps range
            SDK_ASSERT(0);
        }
    }

    uint32_t quota = (node_params->sched_type == TM_SCHED_TYPE_STRICT ?
                      scheduler_rate : node_params->dwrr.weight);

    pbc_csr.dhs_sched.entry[0].command(1);   //1: overwrite quota and credits
    pbc_csr.dhs_sched.entry[0].current_credit(quota);
    pbc_csr.dhs_sched.entry[0].quota(quota);
    if (tm_sw_cfg_write_enabled()) {
        data <<"pbc_csr.dhs_sched.entry[0].all: 0x" << pbc_csr.dhs_sched.entry[0].all() << endl;

        pbc_csr.dhs_sched.entry[0].write();
    }
    SDK_TRACE_DEBUG("%s", data.str().c_str());
    SDK_TRACE_DEBUG("Updated the output queue scheduler on port %u "
                    "level %u, node %u",
                    port, 1, node);

#endif
    return sdk::SDK_RET_OK;
}

sdk_ret_t
elba_tm_scheduler_map_update_l2 (uint32_t port,
                                        uint32_t node,
                                        tm_queue_node_params_t *node_params)
{
#ifdef TODO_ELBA
    stringstream data;
    data << hex << endl;
    elb_top_csr_t &elb0 = ELB_BLK_REG_MODEL_ACCESS(elb_top_csr_t, 0, 0);
    elb_pbc_csr_t &pbc_csr = elb0.pb.pbc;
    (void)pbc_csr;
    cpp_int node_val;
    cpp_int strict_val;

    uint32_t max_nodes = ELBA_TM_COUNT_L2_NODES;

    if (node >= max_nodes) {
        SDK_TRACE_ERR("node %u exceeds the number of valid level "
                      "2 nodes in port %u",
                      node, port);
        return sdk::SDK_RET_INVALID_ARG;
    }

    switch(port) {
        default:
            return sdk::SDK_RET_ERR;
    }

    pbc_csr.cfg_sched.read();
    pbc_csr.cfg_sched.dhs_selection( 2*2 );
    if (tm_sw_cfg_write_enabled()) {
        data <<"pbc_csr.cfg_sched.all: 0x" << pbc_csr.cfg_sched.all() << endl;

        pbc_csr.cfg_sched.write();
    }

    pbc_csr.cfg_dhs_mem.address(port*max_nodes + node);
    if (tm_sw_cfg_write_enabled()) {
        data <<"pbc_csr.cfg_dhs_mem.all: 0x" << pbc_csr.cfg_dhs_mem.all() << endl;
data <<"pbc_csr.cfg_dhs_mem.address: 0x" << pbc_csr.cfg_dhs_mem.address() << endl;

        pbc_csr.cfg_dhs_mem.write();
    }

    uint64_t scheduler_rate;
    if (node_params->sched_type == TM_SCHED_TYPE_STRICT) {
        scheduler_rate = (uint64_t)ELBA_TM_SCHEDULER_RATE_REFRESH_INTERVAL_US*node_params->strict.rate;
        scheduler_rate /= 1000000;
        if (scheduler_rate >= (1ull << 32)) {
            // This should never happen it's in 80 Tbps range
            SDK_ASSERT(0);
        }
    }

    uint32_t quota = (node_params->sched_type == TM_SCHED_TYPE_STRICT ?
                      scheduler_rate : node_params->dwrr.weight);

    pbc_csr.dhs_sched.entry[0].command(1);   //1: overwrite quota and credits
    pbc_csr.dhs_sched.entry[0].current_credit(quota);
    pbc_csr.dhs_sched.entry[0].quota(quota);
    if (tm_sw_cfg_write_enabled()) {
        data <<"pbc_csr.dhs_sched.entry[0].all: 0x" << pbc_csr.dhs_sched.entry[0].all() << endl;

        pbc_csr.dhs_sched.entry[0].write();
    }
    SDK_TRACE_DEBUG("%s", data.str().c_str());
    SDK_TRACE_DEBUG("Updated the output queue scheduler on port %u "
                    "level %u, node %u",
                    port, 2, node);

#endif
    return sdk::SDK_RET_OK;
}


sdk_ret_t
elba_tm_scheduler_map_update (uint32_t port,
                               tm_queue_node_type_e node_type,
                               uint32_t node,
                               tm_queue_node_params_t *node_params)
{
    sdk_ret_t ret = sdk::SDK_RET_OK;

    if (!elba_tm_is_valid_port(port)) {
        SDK_TRACE_ERR("%u is not a valid TM port",
                      port);
        return sdk::SDK_RET_INVALID_ARG;
    }
    switch(node_type) {
        case TM_QUEUE_NODE_TYPE_LEVEL_0:
            ret = elba_tm_scheduler_map_update_l0(port, node, node_params);
            break;
        case TM_QUEUE_NODE_TYPE_LEVEL_1:
            ret = elba_tm_scheduler_map_update_l1(port, node, node_params);
            break;
        case TM_QUEUE_NODE_TYPE_LEVEL_2:
            ret = elba_tm_scheduler_map_update_l2(port, node, node_params);
            break;
    }
    return ret;
}

/* Program the lif value on an uplink port */
sdk_ret_t
elba_tm_uplink_lif_set (uint32_t port,
                         uint32_t lif)
{
    if (!elba_tm_port_is_uplink_port(port)) {
        SDK_TRACE_ERR("%u is not a valid TM uplink port",
                      port);
        return sdk::SDK_RET_INVALID_ARG;
    }

    stringstream data;
    data << hex << endl;
    elb_top_csr_t &elb0 = ELB_BLK_REG_MODEL_ACCESS(elb_top_csr_t, 0, 0);
    elb_pbc_csr_t &pbc_csr = elb0.pb.pbc;

    pbc_csr.cfg_src_port_to_lif_map.read();
    /* Update the value in the csr */
    switch(port) {
        case ELBA_TM_PORT_UPLINK_0:
            {
                pbc_csr.cfg_src_port_to_lif_map.entry_0(lif);
                break;
            }
        case ELBA_TM_PORT_UPLINK_1:
            {
                pbc_csr.cfg_src_port_to_lif_map.entry_1(lif);
                break;
            }
        case ELBA_TM_PORT_UPLINK_2:
            {
                pbc_csr.cfg_src_port_to_lif_map.entry_2(lif);
                break;
            }
        case ELBA_TM_PORT_UPLINK_3:
            {
                pbc_csr.cfg_src_port_to_lif_map.entry_3(lif);
                break;
            }
        default:
            return sdk::SDK_RET_ERR;
    }

    /* Write the csr */
    data <<"pbc_csr.cfg_src_port_to_lif_map.all: 0x" << pbc_csr.cfg_src_port_to_lif_map.all() << endl;
data <<"pbc_csr.cfg_src_port_to_lif_map.entry_4: 0x" << pbc_csr.cfg_src_port_to_lif_map.entry_4() << endl;
data <<"pbc_csr.cfg_src_port_to_lif_map.entry_2: 0x" << pbc_csr.cfg_src_port_to_lif_map.entry_2() << endl;
data <<"pbc_csr.cfg_src_port_to_lif_map.entry_3: 0x" << pbc_csr.cfg_src_port_to_lif_map.entry_3() << endl;
data <<"pbc_csr.cfg_src_port_to_lif_map.entry_0: 0x" << pbc_csr.cfg_src_port_to_lif_map.entry_0() << endl;
data <<"pbc_csr.cfg_src_port_to_lif_map.entry_1: 0x" << pbc_csr.cfg_src_port_to_lif_map.entry_1() << endl;

    pbc_csr.cfg_src_port_to_lif_map.write();
    SDK_TRACE_DEBUG("%s", data.str().c_str());

    SDK_TRACE_DEBUG("Set the lif %u on port %u",
                    lif, port);

    return sdk::SDK_RET_OK;
}

uint32_t
elba_tm_get_hbm_occupancy (tm_hbm_fifo_type_e fifo_type, uint32_t context)
{
    //elb_top_csr_t &elb0 = ELB_BLK_REG_MODEL_ACCESS(elb_top_csr_t, 0, 0);
    //elb_pf_csr_t &pf_csr = elb0.pf.pf;
    uint32_t occupancy = UINT32_MAX;

    if (context >= elba_tm_max_hbm_contexts_for_fifo(fifo_type)) {
        SDK_TRACE_ERR("Invalid context %u for fifo %u", context, fifo_type);
        return sdk::SDK_RET_INVALID_ARG;
    }

    stringstream data;
    data << hex << endl;
    switch(fifo_type) {
        case TM_HBM_FIFO_TYPE_UPLINK:
            switch (context) {
                case 0:
                    {
                        // pf_csr.sta_hbm_eth_context_0.read();
                        // data <<"pf_csr.sta_hbm_eth_context_0.all: 0x" << pf_csr.sta_hbm_eth_context_0.all() << endl;

                        // occupancy =
                        //     pf_csr.sta_hbm_eth_context_0.depth().convert_to<uint32_t>();
                    }
                    break;
                case 1:
                    {
                        // pf_csr.sta_hbm_eth_context_1.read();
                        // data <<"pf_csr.sta_hbm_eth_context_1.all: 0x" << pf_csr.sta_hbm_eth_context_1.all() << endl;

                        // occupancy =
                        //     pf_csr.sta_hbm_eth_context_1.depth().convert_to<uint32_t>();
                    }
                    break;
                case 2:
                    {
                        // pf_csr.sta_hbm_eth_context_2.read();
                        // data <<"pf_csr.sta_hbm_eth_context_2.all: 0x" << pf_csr.sta_hbm_eth_context_2.all() << endl;

                        // occupancy =
                        //     pf_csr.sta_hbm_eth_context_2.depth().convert_to<uint32_t>();
                    }
                    break;
                case 3:
                    {
                        // pf_csr.sta_hbm_eth_context_3.read();
                        // data <<"pf_csr.sta_hbm_eth_context_3.all: 0x" << pf_csr.sta_hbm_eth_context_3.all() << endl;

                        // occupancy =
                        //     pf_csr.sta_hbm_eth_context_3.depth().convert_to<uint32_t>();
                    }
                    break;
                case 4:
                    {
                        // pf_csr.sta_hbm_eth_context_4.read();
                        // data <<"pf_csr.sta_hbm_eth_context_4.all: 0x" << pf_csr.sta_hbm_eth_context_4.all() << endl;

                        // occupancy =
                        //     pf_csr.sta_hbm_eth_context_4.depth().convert_to<uint32_t>();
                    }
                    break;
                case 5:
                    {
                        // pf_csr.sta_hbm_eth_context_5.read();
                        // data <<"pf_csr.sta_hbm_eth_context_5.all: 0x" << pf_csr.sta_hbm_eth_context_5.all() << endl;

                        // occupancy =
                        //     pf_csr.sta_hbm_eth_context_5.depth().convert_to<uint32_t>();
                    }
                    break;
                case 6:
                    {
                        // pf_csr.sta_hbm_eth_context_6.read();
                        // data <<"pf_csr.sta_hbm_eth_context_6.all: 0x" << pf_csr.sta_hbm_eth_context_6.all() << endl;

                        // occupancy =
                        //     pf_csr.sta_hbm_eth_context_6.depth().convert_to<uint32_t>();
                    }
                    break;
                case 7:
                    {
                        // pf_csr.sta_hbm_eth_context_7.read();
                        // data <<"pf_csr.sta_hbm_eth_context_7.all: 0x" << pf_csr.sta_hbm_eth_context_7.all() << endl;

                        // occupancy =
                        //     pf_csr.sta_hbm_eth_context_7.depth().convert_to<uint32_t>();
                    }
                    break;
                case 8:
                    {
                        // pf_csr.sta_hbm_eth_context_8.read();
                        // data <<"pf_csr.sta_hbm_eth_context_8.all: 0x" << pf_csr.sta_hbm_eth_context_8.all() << endl;

                        // occupancy =
                        //     pf_csr.sta_hbm_eth_context_8.depth().convert_to<uint32_t>();
                    }
                    break;
                case 9:
                    {
                        // pf_csr.sta_hbm_eth_context_9.read();
                        // data <<"pf_csr.sta_hbm_eth_context_9.all: 0x" << pf_csr.sta_hbm_eth_context_9.all() << endl;

                        // occupancy =
                        //     pf_csr.sta_hbm_eth_context_9.depth().convert_to<uint32_t>();
                    }
                    break;
                case 10:
                    {
                        // pf_csr.sta_hbm_eth_context_10.read();
                        // data <<"pf_csr.sta_hbm_eth_context_10.all: 0x" << pf_csr.sta_hbm_eth_context_10.all() << endl;

                        // occupancy =
                        //     pf_csr.sta_hbm_eth_context_10.depth().convert_to<uint32_t>();
                    }
                    break;
                case 11:
                    {
                        // pf_csr.sta_hbm_eth_context_11.read();
                        // data <<"pf_csr.sta_hbm_eth_context_11.all: 0x" << pf_csr.sta_hbm_eth_context_11.all() << endl;

                        // occupancy =
                        //     pf_csr.sta_hbm_eth_context_11.depth().convert_to<uint32_t>();
                    }
                    break;
                case 12:
                    {
                        // pf_csr.sta_hbm_eth_context_12.read();
                        // data <<"pf_csr.sta_hbm_eth_context_12.all: 0x" << pf_csr.sta_hbm_eth_context_12.all() << endl;

                        // occupancy =
                        //     pf_csr.sta_hbm_eth_context_12.depth().convert_to<uint32_t>();
                    }
                    break;
                case 13:
                    {
                        // pf_csr.sta_hbm_eth_context_13.read();
                        // data <<"pf_csr.sta_hbm_eth_context_13.all: 0x" << pf_csr.sta_hbm_eth_context_13.all() << endl;

                        // occupancy =
                        //     pf_csr.sta_hbm_eth_context_13.depth().convert_to<uint32_t>();
                    }
                    break;
                case 14:
                    {
                        // pf_csr.sta_hbm_eth_context_14.read();
                        // data <<"pf_csr.sta_hbm_eth_context_14.all: 0x" << pf_csr.sta_hbm_eth_context_14.all() << endl;

                        // occupancy =
                        //     pf_csr.sta_hbm_eth_context_14.depth().convert_to<uint32_t>();
                    }
                    break;
                case 15:
                    {
                        // pf_csr.sta_hbm_eth_context_15.read();
                        // data <<"pf_csr.sta_hbm_eth_context_15.all: 0x" << pf_csr.sta_hbm_eth_context_15.all() << endl;

                        // occupancy =
                        //     pf_csr.sta_hbm_eth_context_15.depth().convert_to<uint32_t>();
                    }
                    break;
                case 16:
                    {
                        // pf_csr.sta_hbm_eth_context_16.read();
                        // data <<"pf_csr.sta_hbm_eth_context_16.all: 0x" << pf_csr.sta_hbm_eth_context_16.all() << endl;

                        // occupancy =
                        //     pf_csr.sta_hbm_eth_context_16.depth().convert_to<uint32_t>();
                    }
                    break;
                case 17:
                    {
                        // pf_csr.sta_hbm_eth_context_17.read();
                        // data <<"pf_csr.sta_hbm_eth_context_17.all: 0x" << pf_csr.sta_hbm_eth_context_17.all() << endl;

                        // occupancy =
                        //     pf_csr.sta_hbm_eth_context_17.depth().convert_to<uint32_t>();
                    }
                    break;
                case 18:
                    {
                        // pf_csr.sta_hbm_eth_context_18.read();
                        // data <<"pf_csr.sta_hbm_eth_context_18.all: 0x" << pf_csr.sta_hbm_eth_context_18.all() << endl;

                        // occupancy =
                        //     pf_csr.sta_hbm_eth_context_18.depth().convert_to<uint32_t>();
                    }
                    break;
                case 19:
                    {
                        // pf_csr.sta_hbm_eth_context_19.read();
                        // data <<"pf_csr.sta_hbm_eth_context_19.all: 0x" << pf_csr.sta_hbm_eth_context_19.all() << endl;

                        // occupancy =
                        //     pf_csr.sta_hbm_eth_context_19.depth().convert_to<uint32_t>();
                    }
                    break;
                case 20:
                    {
                        // pf_csr.sta_hbm_eth_context_20.read();
                        // data <<"pf_csr.sta_hbm_eth_context_20.all: 0x" << pf_csr.sta_hbm_eth_context_20.all() << endl;

                        // occupancy =
                        //     pf_csr.sta_hbm_eth_context_20.depth().convert_to<uint32_t>();
                    }
                    break;
                case 21:
                    {
                        // pf_csr.sta_hbm_eth_context_21.read();
                        // data <<"pf_csr.sta_hbm_eth_context_21.all: 0x" << pf_csr.sta_hbm_eth_context_21.all() << endl;

                        // occupancy =
                        //     pf_csr.sta_hbm_eth_context_21.depth().convert_to<uint32_t>();
                    }
                    break;
                case 22:
                    {
                        // pf_csr.sta_hbm_eth_context_22.read();
                        // data <<"pf_csr.sta_hbm_eth_context_22.all: 0x" << pf_csr.sta_hbm_eth_context_22.all() << endl;

                        // occupancy =
                        //     pf_csr.sta_hbm_eth_context_22.depth().convert_to<uint32_t>();
                    }
                    break;
                case 23:
                    {
                        // pf_csr.sta_hbm_eth_context_23.read();
                        // data <<"pf_csr.sta_hbm_eth_context_23.all: 0x" << pf_csr.sta_hbm_eth_context_23.all() << endl;

                        // occupancy =
                        //     pf_csr.sta_hbm_eth_context_23.depth().convert_to<uint32_t>();
                    }
                    break;
                case 24:
                    {
                        // pf_csr.sta_hbm_eth_context_24.read();
                        // data <<"pf_csr.sta_hbm_eth_context_24.all: 0x" << pf_csr.sta_hbm_eth_context_24.all() << endl;

                        // occupancy =
                        //     pf_csr.sta_hbm_eth_context_24.depth().convert_to<uint32_t>();
                    }
                    break;
                case 25:
                    {
                        // pf_csr.sta_hbm_eth_context_25.read();
                        // data <<"pf_csr.sta_hbm_eth_context_25.all: 0x" << pf_csr.sta_hbm_eth_context_25.all() << endl;

                        // occupancy =
                        //     pf_csr.sta_hbm_eth_context_25.depth().convert_to<uint32_t>();
                    }
                    break;
                case 26:
                    {
                        // pf_csr.sta_hbm_eth_context_26.read();
                        // data <<"pf_csr.sta_hbm_eth_context_26.all: 0x" << pf_csr.sta_hbm_eth_context_26.all() << endl;

                        // occupancy =
                        //     pf_csr.sta_hbm_eth_context_26.depth().convert_to<uint32_t>();
                    }
                    break;
                case 27:
                    {
                        // pf_csr.sta_hbm_eth_context_27.read();
                        // data <<"pf_csr.sta_hbm_eth_context_27.all: 0x" << pf_csr.sta_hbm_eth_context_27.all() << endl;

                        // occupancy =
                        //     pf_csr.sta_hbm_eth_context_27.depth().convert_to<uint32_t>();
                    }
                    break;
                case 28:
                    {
                        // pf_csr.sta_hbm_eth_context_28.read();
                        // data <<"pf_csr.sta_hbm_eth_context_28.all: 0x" << pf_csr.sta_hbm_eth_context_28.all() << endl;

                        // occupancy =
                        //     pf_csr.sta_hbm_eth_context_28.depth().convert_to<uint32_t>();
                    }
                    break;
                case 29:
                    {
                        // pf_csr.sta_hbm_eth_context_29.read();
                        // data <<"pf_csr.sta_hbm_eth_context_29.all: 0x" << pf_csr.sta_hbm_eth_context_29.all() << endl;

                        // occupancy =
                        //     pf_csr.sta_hbm_eth_context_29.depth().convert_to<uint32_t>();
                    }
                    break;
                case 30:
                    {
                        // pf_csr.sta_hbm_eth_context_30.read();
                        // data <<"pf_csr.sta_hbm_eth_context_30.all: 0x" << pf_csr.sta_hbm_eth_context_30.all() << endl;

                        // occupancy =
                        //     pf_csr.sta_hbm_eth_context_30.depth().convert_to<uint32_t>();
                    }
                    break;
                case 31:
                    {
                        // pf_csr.sta_hbm_eth_context_31.read();
                        // data <<"pf_csr.sta_hbm_eth_context_31.all: 0x" << pf_csr.sta_hbm_eth_context_31.all() << endl;

                        // occupancy =
                        //     pf_csr.sta_hbm_eth_context_31.depth().convert_to<uint32_t>();
                    }
                    break;
            }
            break;
        case TM_HBM_FIFO_TYPE_TXDMA:
            switch (context) {
                case 0:
                    {
                        // pf_csr.sta_hbm_tx_context_0.read();
                        // data <<"pf_csr.sta_hbm_tx_context_0.all: 0x" << pf_csr.sta_hbm_tx_context_0.all() << endl;

                        // occupancy =
                        //     pf_csr.sta_hbm_tx_context_0.depth().convert_to<uint32_t>();
                    }
                    break;
                case 1:
                    {
                        // pf_csr.sta_hbm_tx_context_1.read();
                        // data <<"pf_csr.sta_hbm_tx_context_1.all: 0x" << pf_csr.sta_hbm_tx_context_1.all() << endl;

                        // occupancy =
                        //     pf_csr.sta_hbm_tx_context_1.depth().convert_to<uint32_t>();
                    }
                    break;
                case 2:
                    {
                        // pf_csr.sta_hbm_tx_context_2.read();
                        // data <<"pf_csr.sta_hbm_tx_context_2.all: 0x" << pf_csr.sta_hbm_tx_context_2.all() << endl;

                        // occupancy =
                        //     pf_csr.sta_hbm_tx_context_2.depth().convert_to<uint32_t>();
                    }
                    break;
                case 3:
                    {
                        // pf_csr.sta_hbm_tx_context_3.read();
                        // data <<"pf_csr.sta_hbm_tx_context_3.all: 0x" << pf_csr.sta_hbm_tx_context_3.all() << endl;

                        // occupancy =
                        //     pf_csr.sta_hbm_tx_context_3.depth().convert_to<uint32_t>();
                    }
                    break;
                case 4:
                    {
                        // pf_csr.sta_hbm_tx_context_4.read();
                        // data <<"pf_csr.sta_hbm_tx_context_4.all: 0x" << pf_csr.sta_hbm_tx_context_4.all() << endl;

                        // occupancy =
                        //     pf_csr.sta_hbm_tx_context_4.depth().convert_to<uint32_t>();
                    }
                    break;
                case 5:
                    {
                        // pf_csr.sta_hbm_tx_context_5.read();
                        // data <<"pf_csr.sta_hbm_tx_context_5.all: 0x" << pf_csr.sta_hbm_tx_context_5.all() << endl;

                        // occupancy =
                        //     pf_csr.sta_hbm_tx_context_5.depth().convert_to<uint32_t>();
                    }
                    break;
                case 6:
                    {
                        // pf_csr.sta_hbm_tx_context_6.read();
                        // data <<"pf_csr.sta_hbm_tx_context_6.all: 0x" << pf_csr.sta_hbm_tx_context_6.all() << endl;

                        // occupancy =
                        //     pf_csr.sta_hbm_tx_context_6.depth().convert_to<uint32_t>();
                    }
                    break;
                case 7:
                    {
                        // pf_csr.sta_hbm_tx_context_7.read();
                        // data <<"pf_csr.sta_hbm_tx_context_7.all: 0x" << pf_csr.sta_hbm_tx_context_7.all() << endl;

                        // occupancy =
                        //     pf_csr.sta_hbm_tx_context_7.depth().convert_to<uint32_t>();
                    }
                    break;
                case 8:
                    {
                        // pf_csr.sta_hbm_tx_context_8.read();
                        // data <<"pf_csr.sta_hbm_tx_context_8.all: 0x" << pf_csr.sta_hbm_tx_context_8.all() << endl;

                        // occupancy =
                        //     pf_csr.sta_hbm_tx_context_8.depth().convert_to<uint32_t>();
                    }
                    break;
                case 9:
                    {
                        // pf_csr.sta_hbm_tx_context_9.read();
                        // data <<"pf_csr.sta_hbm_tx_context_9.all: 0x" << pf_csr.sta_hbm_tx_context_9.all() << endl;

                        // occupancy =
                        //     pf_csr.sta_hbm_tx_context_9.depth().convert_to<uint32_t>();
                    }
                    break;
                case 10:
                    {
                        // pf_csr.sta_hbm_tx_context_10.read();
                        // data <<"pf_csr.sta_hbm_tx_context_10.all: 0x" << pf_csr.sta_hbm_tx_context_10.all() << endl;

                        // occupancy =
                        //     pf_csr.sta_hbm_tx_context_10.depth().convert_to<uint32_t>();
                    }
                    break;
                case 11:
                    {
                        // pf_csr.sta_hbm_tx_context_11.read();
                        // data <<"pf_csr.sta_hbm_tx_context_11.all: 0x" << pf_csr.sta_hbm_tx_context_11.all() << endl;

                        // occupancy =
                        //     pf_csr.sta_hbm_tx_context_11.depth().convert_to<uint32_t>();
                    }
                    break;
                case 12:
                    {
                        // pf_csr.sta_hbm_tx_context_12.read();
                        // data <<"pf_csr.sta_hbm_tx_context_12.all: 0x" << pf_csr.sta_hbm_tx_context_12.all() << endl;

                        // occupancy =
                        //     pf_csr.sta_hbm_tx_context_12.depth().convert_to<uint32_t>();
                    }
                    break;
                case 13:
                    {
                        // pf_csr.sta_hbm_tx_context_13.read();
                        // data <<"pf_csr.sta_hbm_tx_context_13.all: 0x" << pf_csr.sta_hbm_tx_context_13.all() << endl;

                        // occupancy =
                        //     pf_csr.sta_hbm_tx_context_13.depth().convert_to<uint32_t>();
                    }
                    break;
                case 14:
                    {
                        // pf_csr.sta_hbm_tx_context_14.read();
                        // data <<"pf_csr.sta_hbm_tx_context_14.all: 0x" << pf_csr.sta_hbm_tx_context_14.all() << endl;

                        // occupancy =
                        //     pf_csr.sta_hbm_tx_context_14.depth().convert_to<uint32_t>();
                    }
                    break;
                case 15:
                    {
                        // pf_csr.sta_hbm_tx_context_15.read();
                        // data <<"pf_csr.sta_hbm_tx_context_15.all: 0x" << pf_csr.sta_hbm_tx_context_15.all() << endl;

                        // occupancy =
                        //     pf_csr.sta_hbm_tx_context_15.depth().convert_to<uint32_t>();
                    }
                    break;
            }
            break;
        default:
            return occupancy;
    }
    SDK_TRACE_DEBUG("%s", data.str().c_str());

    return occupancy;
}

static void
elba_tm_get_buffer_occupancy (tm_port_t port, tm_q_t iq, 
                               uint32_t *cell_occupancy, 
                               uint32_t *peak_occupancy)
{
    elb_top_csr_t &elb0 = ELB_BLK_REG_MODEL_ACCESS(elb_top_csr_t, 0, 0);
    elb_pbc_csr_t &pbc_csr = elb0.pb.pbc;

    switch(port) {
        case ELBA_TM_PORT_UPLINK_0:
            {
                // ELBA_TM_PORT_UPLINK_0
                pbc_csr.port_0.sta_account.read();
                switch (iq) {
                    case 0:
                        {
                            *cell_occupancy = pbc_csr.port_0.sta_account.occupancy_0().convert_to<uint32_t>();
                            *peak_occupancy = pbc_csr.port_0.sta_account.sp_held_0().convert_to<uint32_t>();
                            break;
                        }
                    case 1:
                        {
                            *cell_occupancy = pbc_csr.port_0.sta_account.occupancy_1().convert_to<uint32_t>();
                            *peak_occupancy = pbc_csr.port_0.sta_account.sp_held_1().convert_to<uint32_t>();
                            break;
                        }
                    case 2:
                        {
                            *cell_occupancy = pbc_csr.port_0.sta_account.occupancy_2().convert_to<uint32_t>();
                            *peak_occupancy = pbc_csr.port_0.sta_account.sp_held_2().convert_to<uint32_t>();
                            break;
                        }
                    case 3:
                        {
                            *cell_occupancy = pbc_csr.port_0.sta_account.occupancy_3().convert_to<uint32_t>();
                            *peak_occupancy = pbc_csr.port_0.sta_account.sp_held_3().convert_to<uint32_t>();
                            break;
                        }
                    case 4:
                        {
                            *cell_occupancy = pbc_csr.port_0.sta_account.occupancy_4().convert_to<uint32_t>();
                            *peak_occupancy = pbc_csr.port_0.sta_account.sp_held_4().convert_to<uint32_t>();
                            break;
                        }
                    case 5:
                        {
                            *cell_occupancy = pbc_csr.port_0.sta_account.occupancy_5().convert_to<uint32_t>();
                            *peak_occupancy = pbc_csr.port_0.sta_account.sp_held_5().convert_to<uint32_t>();
                            break;
                        }
                    case 6:
                        {
                            *cell_occupancy = pbc_csr.port_0.sta_account.occupancy_6().convert_to<uint32_t>();
                            *peak_occupancy = pbc_csr.port_0.sta_account.sp_held_6().convert_to<uint32_t>();
                            break;
                        }
                    case 7:
                        {
                            *cell_occupancy = pbc_csr.port_0.sta_account.occupancy_7().convert_to<uint32_t>();
                            *peak_occupancy = pbc_csr.port_0.sta_account.sp_held_7().convert_to<uint32_t>();
                            break;
                        }
                    default:
                        return;
                }
                break;
            }
        case ELBA_TM_PORT_UPLINK_1:
            {
                // ELBA_TM_PORT_UPLINK_1
                pbc_csr.port_1.sta_account.read();
                switch (iq) {
                    case 0:
                        {
                            *cell_occupancy = pbc_csr.port_1.sta_account.occupancy_0().convert_to<uint32_t>();
                            *peak_occupancy = pbc_csr.port_1.sta_account.sp_held_0().convert_to<uint32_t>();
                            break;
                        }
                    case 1:
                        {
                            *cell_occupancy = pbc_csr.port_1.sta_account.occupancy_1().convert_to<uint32_t>();
                            *peak_occupancy = pbc_csr.port_1.sta_account.sp_held_1().convert_to<uint32_t>();
                            break;
                        }
                    case 2:
                        {
                            *cell_occupancy = pbc_csr.port_1.sta_account.occupancy_2().convert_to<uint32_t>();
                            *peak_occupancy = pbc_csr.port_1.sta_account.sp_held_2().convert_to<uint32_t>();
                            break;
                        }
                    case 3:
                        {
                            *cell_occupancy = pbc_csr.port_1.sta_account.occupancy_3().convert_to<uint32_t>();
                            *peak_occupancy = pbc_csr.port_1.sta_account.sp_held_3().convert_to<uint32_t>();
                            break;
                        }
                    case 4:
                        {
                            *cell_occupancy = pbc_csr.port_1.sta_account.occupancy_4().convert_to<uint32_t>();
                            *peak_occupancy = pbc_csr.port_1.sta_account.sp_held_4().convert_to<uint32_t>();
                            break;
                        }
                    case 5:
                        {
                            *cell_occupancy = pbc_csr.port_1.sta_account.occupancy_5().convert_to<uint32_t>();
                            *peak_occupancy = pbc_csr.port_1.sta_account.sp_held_5().convert_to<uint32_t>();
                            break;
                        }
                    case 6:
                        {
                            *cell_occupancy = pbc_csr.port_1.sta_account.occupancy_6().convert_to<uint32_t>();
                            *peak_occupancy = pbc_csr.port_1.sta_account.sp_held_6().convert_to<uint32_t>();
                            break;
                        }
                    case 7:
                        {
                            *cell_occupancy = pbc_csr.port_1.sta_account.occupancy_7().convert_to<uint32_t>();
                            *peak_occupancy = pbc_csr.port_1.sta_account.sp_held_7().convert_to<uint32_t>();
                            break;
                        }
                    default:
                        return;
                }
                break;
            }
        case ELBA_TM_PORT_UPLINK_2:
            {
                // ELBA_TM_PORT_UPLINK_2
                pbc_csr.port_2.sta_account.read();
                switch (iq) {
                    case 0:
                        {
                            *cell_occupancy = pbc_csr.port_2.sta_account.occupancy_0().convert_to<uint32_t>();
                            *peak_occupancy = pbc_csr.port_2.sta_account.sp_held_0().convert_to<uint32_t>();
                            break;
                        }
                    case 1:
                        {
                            *cell_occupancy = pbc_csr.port_2.sta_account.occupancy_1().convert_to<uint32_t>();
                            *peak_occupancy = pbc_csr.port_2.sta_account.sp_held_1().convert_to<uint32_t>();
                            break;
                        }
                    case 2:
                        {
                            *cell_occupancy = pbc_csr.port_2.sta_account.occupancy_2().convert_to<uint32_t>();
                            *peak_occupancy = pbc_csr.port_2.sta_account.sp_held_2().convert_to<uint32_t>();
                            break;
                        }
                    case 3:
                        {
                            *cell_occupancy = pbc_csr.port_2.sta_account.occupancy_3().convert_to<uint32_t>();
                            *peak_occupancy = pbc_csr.port_2.sta_account.sp_held_3().convert_to<uint32_t>();
                            break;
                        }
                    case 4:
                        {
                            *cell_occupancy = pbc_csr.port_2.sta_account.occupancy_4().convert_to<uint32_t>();
                            *peak_occupancy = pbc_csr.port_2.sta_account.sp_held_4().convert_to<uint32_t>();
                            break;
                        }
                    case 5:
                        {
                            *cell_occupancy = pbc_csr.port_2.sta_account.occupancy_5().convert_to<uint32_t>();
                            *peak_occupancy = pbc_csr.port_2.sta_account.sp_held_5().convert_to<uint32_t>();
                            break;
                        }
                    case 6:
                        {
                            *cell_occupancy = pbc_csr.port_2.sta_account.occupancy_6().convert_to<uint32_t>();
                            *peak_occupancy = pbc_csr.port_2.sta_account.sp_held_6().convert_to<uint32_t>();
                            break;
                        }
                    case 7:
                        {
                            *cell_occupancy = pbc_csr.port_2.sta_account.occupancy_7().convert_to<uint32_t>();
                            *peak_occupancy = pbc_csr.port_2.sta_account.sp_held_7().convert_to<uint32_t>();
                            break;
                        }
                    default:
                        return;
                }
                break;
            }
        case ELBA_TM_PORT_UPLINK_3:
            {
                // ELBA_TM_PORT_UPLINK_3
                pbc_csr.port_3.sta_account.read();
                switch (iq) {
                    case 0:
                        {
                            *cell_occupancy = pbc_csr.port_3.sta_account.occupancy_0().convert_to<uint32_t>();
                            *peak_occupancy = pbc_csr.port_3.sta_account.sp_held_0().convert_to<uint32_t>();
                            break;
                        }
                    case 1:
                        {
                            *cell_occupancy = pbc_csr.port_3.sta_account.occupancy_1().convert_to<uint32_t>();
                            *peak_occupancy = pbc_csr.port_3.sta_account.sp_held_1().convert_to<uint32_t>();
                            break;
                        }
                    case 2:
                        {
                            *cell_occupancy = pbc_csr.port_3.sta_account.occupancy_2().convert_to<uint32_t>();
                            *peak_occupancy = pbc_csr.port_3.sta_account.sp_held_2().convert_to<uint32_t>();
                            break;
                        }
                    case 3:
                        {
                            *cell_occupancy = pbc_csr.port_3.sta_account.occupancy_3().convert_to<uint32_t>();
                            *peak_occupancy = pbc_csr.port_3.sta_account.sp_held_3().convert_to<uint32_t>();
                            break;
                        }
                    case 4:
                        {
                            *cell_occupancy = pbc_csr.port_3.sta_account.occupancy_4().convert_to<uint32_t>();
                            *peak_occupancy = pbc_csr.port_3.sta_account.sp_held_4().convert_to<uint32_t>();
                            break;
                        }
                    case 5:
                        {
                            *cell_occupancy = pbc_csr.port_3.sta_account.occupancy_5().convert_to<uint32_t>();
                            *peak_occupancy = pbc_csr.port_3.sta_account.sp_held_5().convert_to<uint32_t>();
                            break;
                        }
                    case 6:
                        {
                            *cell_occupancy = pbc_csr.port_3.sta_account.occupancy_6().convert_to<uint32_t>();
                            *peak_occupancy = pbc_csr.port_3.sta_account.sp_held_6().convert_to<uint32_t>();
                            break;
                        }
                    case 7:
                        {
                            *cell_occupancy = pbc_csr.port_3.sta_account.occupancy_7().convert_to<uint32_t>();
                            *peak_occupancy = pbc_csr.port_3.sta_account.sp_held_7().convert_to<uint32_t>();
                            break;
                        }
                    default:
                        return;
                }
                break;
            }
        default:
            break;
    }
}


static sdk_ret_t
elba_tm_drain_uplink_port (tm_port_t port)
{
    bool all_zeroes = false;
    uint32_t tries = 0;
    uint32_t max_tries = 1000;
    uint32_t occupancy;
    uint32_t num_hbm_contexts_per_port;
    uint32_t context;
    uint32_t i;

    num_hbm_contexts_per_port = tm_cfg_profile()->num_qs[TM_PORT_TYPE_UPLINK];
    while (port_supports_hbm_contexts(port) &&
           !all_zeroes && (tries < max_tries)) {
        all_zeroes = true;
        for (i = 0; i < num_hbm_contexts_per_port; i++) {
            context = (port * num_hbm_contexts_per_port) + i;
            occupancy = elba_tm_get_hbm_occupancy(TM_HBM_FIFO_TYPE_UPLINK, context);
            if (occupancy) {
                all_zeroes = false;
            }
        }
        // TODO: Do we need a sleep here ?
        usleep(1000);
        tries++;
    }

    if (!all_zeroes && port_supports_hbm_contexts(port)) {
        SDK_TRACE_ERR("Port %u hbm queues not drained completely after %u tries",
                      port, tries);
        return sdk::SDK_RET_RETRY;
    }

    return sdk::SDK_RET_OK;
}

sdk_ret_t
elba_tm_enable_disable_uplink_port (tm_port_t port, bool enable)
{
    sdk_ret_t ret = sdk::SDK_RET_OK;
    elb_top_csr_t &elb0 = ELB_BLK_REG_MODEL_ACCESS(elb_top_csr_t, 0, 0);
    elb_pbc_csr_t &pbc_csr = elb0.pb.pbc;

    if (!elba_tm_port_is_uplink_port(port)) {
        SDK_TRACE_ERR("%u is not a valid TM uplink port",
                      port);
        return sdk::SDK_RET_INVALID_ARG;
    }

    if (enable) {
        /* Make sure the contexts are free */
        ret = elba_tm_drain_uplink_port(port);
        if (ret != sdk::SDK_RET_OK) {
            SDK_TRACE_ERR("Port %u is not fully drained. Retry later", port);
            return sdk::SDK_RET_RETRY;
        }
    }

    stringstream data;
    data << hex << endl;
    switch(port) {
        case ELBA_TM_PORT_UPLINK_0:
            {
                pbc_csr.port_0.cfg_write_control.read();
                pbc_csr.port_0.cfg_oq.read();

                pbc_csr.port_0.cfg_write_control.enable(enable ? 1 : 0);
                pbc_csr.port_0.cfg_oq.flush(enable ? 0 : 1);

                if (tm_sw_cfg_write_enabled()) {
                    data <<"pbc_csr.port_0.cfg_write_control.all: 0x" << pbc_csr.port_0.cfg_write_control.all() << endl;
data <<"pbc_csr.port_0.cfg_write_control.release_cells: 0x" << pbc_csr.port_0.cfg_write_control.release_cells() << endl;
data <<"pbc_csr.port_0.cfg_write_control.enable: 0x" << pbc_csr.port_0.cfg_write_control.enable() << endl;
data <<"pbc_csr.port_0.cfg_write_control.drop_on_error: 0x" << pbc_csr.port_0.cfg_write_control.drop_on_error() << endl;
data <<"pbc_csr.port_0.cfg_write_control.rewrite: 0x" << pbc_csr.port_0.cfg_write_control.rewrite() << endl;
data <<"pbc_csr.port_0.cfg_write_control.span_pg_enable: 0x" << pbc_csr.port_0.cfg_write_control.span_pg_enable() << endl;
data <<"pbc_csr.port_0.cfg_write_control.span_pg_index: 0x" << pbc_csr.port_0.cfg_write_control.span_pg_index() << endl;
data <<"pbc_csr.port_0.cfg_write_control.enforce_oq24_31: 0x" << pbc_csr.port_0.cfg_write_control.enforce_oq24_31() << endl;
data <<"pbc_csr.port_0.cfg_write_control.packing_timeout_cycles: 0x" << pbc_csr.port_0.cfg_write_control.packing_timeout_cycles() << endl;
data <<"pbc_csr.port_0.cfg_write_control.min_size: 0x" << pbc_csr.port_0.cfg_write_control.min_size() << endl;
data <<"pbc_csr.port_0.cfg_write_control.packing_same_pg: 0x" << pbc_csr.port_0.cfg_write_control.packing_same_pg() << endl;
data <<"pbc_csr.port_0.cfg_write_control.packing: 0x" << pbc_csr.port_0.cfg_write_control.packing() << endl;
data <<"pbc_csr.port_0.cfg_write_control.enqueue_mux_bypass: 0x" << pbc_csr.port_0.cfg_write_control.enqueue_mux_bypass() << endl;
data <<"pbc_csr.port_0.cfg_write_control.enable_drdy: 0x" << pbc_csr.port_0.cfg_write_control.enable_drdy() << endl;
data <<"pbc_csr.port_0.cfg_write_control.rate_limiter: 0x" << pbc_csr.port_0.cfg_write_control.rate_limiter() << endl;
data <<"pbc_csr.port_0.cfg_write_control.cut_thru: 0x" << pbc_csr.port_0.cfg_write_control.cut_thru() << endl;
data <<"pbc_csr.port_0.cfg_write_control.enable_mnic_multicast: 0x" << pbc_csr.port_0.cfg_write_control.enable_mnic_multicast() << endl;
data <<"pbc_csr.port_0.cfg_write_control.enable_bmc_multicast: 0x" << pbc_csr.port_0.cfg_write_control.enable_bmc_multicast() << endl;

                    pbc_csr.port_0.cfg_write_control.write();

                    data <<"pbc_csr.port_0.cfg_oq.all: 0x" << pbc_csr.port_0.cfg_oq.all() << endl;
data <<"pbc_csr.port_0.cfg_oq.enable: 0x" << pbc_csr.port_0.cfg_oq.enable() << endl;
data <<"pbc_csr.port_0.cfg_oq.num_hdr_bytes: 0x" << pbc_csr.port_0.cfg_oq.num_hdr_bytes() << endl;
data <<"pbc_csr.port_0.cfg_oq.num_hdr_bytes_crypto: 0x" << pbc_csr.port_0.cfg_oq.num_hdr_bytes_crypto() << endl;
data <<"pbc_csr.port_0.cfg_oq.span_cpu_num_cells: 0x" << pbc_csr.port_0.cfg_oq.span_cpu_num_cells() << endl;
data <<"pbc_csr.port_0.cfg_oq.flush_hw_error: 0x" << pbc_csr.port_0.cfg_oq.flush_hw_error() << endl;
data <<"pbc_csr.port_0.cfg_oq.eg_ts_enable: 0x" << pbc_csr.port_0.cfg_oq.eg_ts_enable() << endl;
data <<"pbc_csr.port_0.cfg_oq.ipg_bytes: 0x" << pbc_csr.port_0.cfg_oq.ipg_bytes() << endl;
data <<"pbc_csr.port_0.cfg_oq.flush: 0x" << pbc_csr.port_0.cfg_oq.flush() << endl;
data <<"pbc_csr.port_0.cfg_oq.flow_control_enable_xoff: 0x" << pbc_csr.port_0.cfg_oq.flow_control_enable_xoff() << endl;
data <<"pbc_csr.port_0.cfg_oq.hw_error_to_pbus: 0x" << pbc_csr.port_0.cfg_oq.hw_error_to_pbus() << endl;
data <<"pbc_csr.port_0.cfg_oq.rewrite_enable: 0x" << pbc_csr.port_0.cfg_oq.rewrite_enable() << endl;

                    pbc_csr.port_0.cfg_oq.write();
                }
                break;
            }
        case ELBA_TM_PORT_UPLINK_1:
            {
                pbc_csr.port_1.cfg_write_control.read();
                pbc_csr.port_1.cfg_oq.read();

                pbc_csr.port_1.cfg_write_control.enable(enable ? 1 : 0);
                pbc_csr.port_1.cfg_oq.flush(enable ? 0 : 1);

                if (tm_sw_cfg_write_enabled()) {
                    data <<"pbc_csr.port_1.cfg_write_control.all: 0x" << pbc_csr.port_1.cfg_write_control.all() << endl;
data <<"pbc_csr.port_1.cfg_write_control.release_cells: 0x" << pbc_csr.port_1.cfg_write_control.release_cells() << endl;
data <<"pbc_csr.port_1.cfg_write_control.enable: 0x" << pbc_csr.port_1.cfg_write_control.enable() << endl;
data <<"pbc_csr.port_1.cfg_write_control.drop_on_error: 0x" << pbc_csr.port_1.cfg_write_control.drop_on_error() << endl;
data <<"pbc_csr.port_1.cfg_write_control.rewrite: 0x" << pbc_csr.port_1.cfg_write_control.rewrite() << endl;
data <<"pbc_csr.port_1.cfg_write_control.span_pg_enable: 0x" << pbc_csr.port_1.cfg_write_control.span_pg_enable() << endl;
data <<"pbc_csr.port_1.cfg_write_control.span_pg_index: 0x" << pbc_csr.port_1.cfg_write_control.span_pg_index() << endl;
data <<"pbc_csr.port_1.cfg_write_control.enforce_oq24_31: 0x" << pbc_csr.port_1.cfg_write_control.enforce_oq24_31() << endl;
data <<"pbc_csr.port_1.cfg_write_control.packing_timeout_cycles: 0x" << pbc_csr.port_1.cfg_write_control.packing_timeout_cycles() << endl;
data <<"pbc_csr.port_1.cfg_write_control.min_size: 0x" << pbc_csr.port_1.cfg_write_control.min_size() << endl;
data <<"pbc_csr.port_1.cfg_write_control.packing_same_pg: 0x" << pbc_csr.port_1.cfg_write_control.packing_same_pg() << endl;
data <<"pbc_csr.port_1.cfg_write_control.packing: 0x" << pbc_csr.port_1.cfg_write_control.packing() << endl;
data <<"pbc_csr.port_1.cfg_write_control.enqueue_mux_bypass: 0x" << pbc_csr.port_1.cfg_write_control.enqueue_mux_bypass() << endl;
data <<"pbc_csr.port_1.cfg_write_control.enable_drdy: 0x" << pbc_csr.port_1.cfg_write_control.enable_drdy() << endl;
data <<"pbc_csr.port_1.cfg_write_control.rate_limiter: 0x" << pbc_csr.port_1.cfg_write_control.rate_limiter() << endl;
data <<"pbc_csr.port_1.cfg_write_control.cut_thru: 0x" << pbc_csr.port_1.cfg_write_control.cut_thru() << endl;
data <<"pbc_csr.port_1.cfg_write_control.enable_mnic_multicast: 0x" << pbc_csr.port_1.cfg_write_control.enable_mnic_multicast() << endl;
data <<"pbc_csr.port_1.cfg_write_control.enable_bmc_multicast: 0x" << pbc_csr.port_1.cfg_write_control.enable_bmc_multicast() << endl;

                    pbc_csr.port_1.cfg_write_control.write();

                    data <<"pbc_csr.port_1.cfg_oq.all: 0x" << pbc_csr.port_1.cfg_oq.all() << endl;
data <<"pbc_csr.port_1.cfg_oq.enable: 0x" << pbc_csr.port_1.cfg_oq.enable() << endl;
data <<"pbc_csr.port_1.cfg_oq.num_hdr_bytes: 0x" << pbc_csr.port_1.cfg_oq.num_hdr_bytes() << endl;
data <<"pbc_csr.port_1.cfg_oq.num_hdr_bytes_crypto: 0x" << pbc_csr.port_1.cfg_oq.num_hdr_bytes_crypto() << endl;
data <<"pbc_csr.port_1.cfg_oq.span_cpu_num_cells: 0x" << pbc_csr.port_1.cfg_oq.span_cpu_num_cells() << endl;
data <<"pbc_csr.port_1.cfg_oq.flush_hw_error: 0x" << pbc_csr.port_1.cfg_oq.flush_hw_error() << endl;
data <<"pbc_csr.port_1.cfg_oq.eg_ts_enable: 0x" << pbc_csr.port_1.cfg_oq.eg_ts_enable() << endl;
data <<"pbc_csr.port_1.cfg_oq.ipg_bytes: 0x" << pbc_csr.port_1.cfg_oq.ipg_bytes() << endl;
data <<"pbc_csr.port_1.cfg_oq.flush: 0x" << pbc_csr.port_1.cfg_oq.flush() << endl;
data <<"pbc_csr.port_1.cfg_oq.flow_control_enable_xoff: 0x" << pbc_csr.port_1.cfg_oq.flow_control_enable_xoff() << endl;
data <<"pbc_csr.port_1.cfg_oq.hw_error_to_pbus: 0x" << pbc_csr.port_1.cfg_oq.hw_error_to_pbus() << endl;
data <<"pbc_csr.port_1.cfg_oq.rewrite_enable: 0x" << pbc_csr.port_1.cfg_oq.rewrite_enable() << endl;

                    pbc_csr.port_1.cfg_oq.write();
                }
                break;
            }
        case ELBA_TM_PORT_UPLINK_2:
            {
                pbc_csr.port_2.cfg_write_control.read();
                pbc_csr.port_2.cfg_oq.read();

                pbc_csr.port_2.cfg_write_control.enable(enable ? 1 : 0);
                pbc_csr.port_2.cfg_oq.flush(enable ? 0 : 1);

                if (tm_sw_cfg_write_enabled()) {
                    data <<"pbc_csr.port_2.cfg_write_control.all: 0x" << pbc_csr.port_2.cfg_write_control.all() << endl;
data <<"pbc_csr.port_2.cfg_write_control.release_cells: 0x" << pbc_csr.port_2.cfg_write_control.release_cells() << endl;
data <<"pbc_csr.port_2.cfg_write_control.enable: 0x" << pbc_csr.port_2.cfg_write_control.enable() << endl;
data <<"pbc_csr.port_2.cfg_write_control.drop_on_error: 0x" << pbc_csr.port_2.cfg_write_control.drop_on_error() << endl;
data <<"pbc_csr.port_2.cfg_write_control.rewrite: 0x" << pbc_csr.port_2.cfg_write_control.rewrite() << endl;
data <<"pbc_csr.port_2.cfg_write_control.span_pg_enable: 0x" << pbc_csr.port_2.cfg_write_control.span_pg_enable() << endl;
data <<"pbc_csr.port_2.cfg_write_control.span_pg_index: 0x" << pbc_csr.port_2.cfg_write_control.span_pg_index() << endl;
data <<"pbc_csr.port_2.cfg_write_control.enforce_oq24_31: 0x" << pbc_csr.port_2.cfg_write_control.enforce_oq24_31() << endl;
data <<"pbc_csr.port_2.cfg_write_control.packing_timeout_cycles: 0x" << pbc_csr.port_2.cfg_write_control.packing_timeout_cycles() << endl;
data <<"pbc_csr.port_2.cfg_write_control.min_size: 0x" << pbc_csr.port_2.cfg_write_control.min_size() << endl;
data <<"pbc_csr.port_2.cfg_write_control.packing_same_pg: 0x" << pbc_csr.port_2.cfg_write_control.packing_same_pg() << endl;
data <<"pbc_csr.port_2.cfg_write_control.packing: 0x" << pbc_csr.port_2.cfg_write_control.packing() << endl;
data <<"pbc_csr.port_2.cfg_write_control.enqueue_mux_bypass: 0x" << pbc_csr.port_2.cfg_write_control.enqueue_mux_bypass() << endl;
data <<"pbc_csr.port_2.cfg_write_control.enable_drdy: 0x" << pbc_csr.port_2.cfg_write_control.enable_drdy() << endl;
data <<"pbc_csr.port_2.cfg_write_control.rate_limiter: 0x" << pbc_csr.port_2.cfg_write_control.rate_limiter() << endl;
data <<"pbc_csr.port_2.cfg_write_control.cut_thru: 0x" << pbc_csr.port_2.cfg_write_control.cut_thru() << endl;
data <<"pbc_csr.port_2.cfg_write_control.enable_mnic_multicast: 0x" << pbc_csr.port_2.cfg_write_control.enable_mnic_multicast() << endl;
data <<"pbc_csr.port_2.cfg_write_control.enable_bmc_multicast: 0x" << pbc_csr.port_2.cfg_write_control.enable_bmc_multicast() << endl;

                    pbc_csr.port_2.cfg_write_control.write();

                    data <<"pbc_csr.port_2.cfg_oq.all: 0x" << pbc_csr.port_2.cfg_oq.all() << endl;
data <<"pbc_csr.port_2.cfg_oq.enable: 0x" << pbc_csr.port_2.cfg_oq.enable() << endl;
data <<"pbc_csr.port_2.cfg_oq.num_hdr_bytes: 0x" << pbc_csr.port_2.cfg_oq.num_hdr_bytes() << endl;
data <<"pbc_csr.port_2.cfg_oq.num_hdr_bytes_crypto: 0x" << pbc_csr.port_2.cfg_oq.num_hdr_bytes_crypto() << endl;
data <<"pbc_csr.port_2.cfg_oq.span_cpu_num_cells: 0x" << pbc_csr.port_2.cfg_oq.span_cpu_num_cells() << endl;
data <<"pbc_csr.port_2.cfg_oq.flush_hw_error: 0x" << pbc_csr.port_2.cfg_oq.flush_hw_error() << endl;
data <<"pbc_csr.port_2.cfg_oq.eg_ts_enable: 0x" << pbc_csr.port_2.cfg_oq.eg_ts_enable() << endl;
data <<"pbc_csr.port_2.cfg_oq.ipg_bytes: 0x" << pbc_csr.port_2.cfg_oq.ipg_bytes() << endl;
data <<"pbc_csr.port_2.cfg_oq.flush: 0x" << pbc_csr.port_2.cfg_oq.flush() << endl;
data <<"pbc_csr.port_2.cfg_oq.flow_control_enable_xoff: 0x" << pbc_csr.port_2.cfg_oq.flow_control_enable_xoff() << endl;
data <<"pbc_csr.port_2.cfg_oq.hw_error_to_pbus: 0x" << pbc_csr.port_2.cfg_oq.hw_error_to_pbus() << endl;
data <<"pbc_csr.port_2.cfg_oq.rewrite_enable: 0x" << pbc_csr.port_2.cfg_oq.rewrite_enable() << endl;

                    pbc_csr.port_2.cfg_oq.write();
                }
                break;
            }
        case ELBA_TM_PORT_UPLINK_3:
            {
                pbc_csr.port_3.cfg_write_control.read();
                pbc_csr.port_3.cfg_oq.read();

                pbc_csr.port_3.cfg_write_control.enable(enable ? 1 : 0);
                pbc_csr.port_3.cfg_oq.flush(enable ? 0 : 1);

                if (tm_sw_cfg_write_enabled()) {
                    data <<"pbc_csr.port_3.cfg_write_control.all: 0x" << pbc_csr.port_3.cfg_write_control.all() << endl;
data <<"pbc_csr.port_3.cfg_write_control.release_cells: 0x" << pbc_csr.port_3.cfg_write_control.release_cells() << endl;
data <<"pbc_csr.port_3.cfg_write_control.enable: 0x" << pbc_csr.port_3.cfg_write_control.enable() << endl;
data <<"pbc_csr.port_3.cfg_write_control.drop_on_error: 0x" << pbc_csr.port_3.cfg_write_control.drop_on_error() << endl;
data <<"pbc_csr.port_3.cfg_write_control.rewrite: 0x" << pbc_csr.port_3.cfg_write_control.rewrite() << endl;
data <<"pbc_csr.port_3.cfg_write_control.span_pg_enable: 0x" << pbc_csr.port_3.cfg_write_control.span_pg_enable() << endl;
data <<"pbc_csr.port_3.cfg_write_control.span_pg_index: 0x" << pbc_csr.port_3.cfg_write_control.span_pg_index() << endl;
data <<"pbc_csr.port_3.cfg_write_control.enforce_oq24_31: 0x" << pbc_csr.port_3.cfg_write_control.enforce_oq24_31() << endl;
data <<"pbc_csr.port_3.cfg_write_control.packing_timeout_cycles: 0x" << pbc_csr.port_3.cfg_write_control.packing_timeout_cycles() << endl;
data <<"pbc_csr.port_3.cfg_write_control.min_size: 0x" << pbc_csr.port_3.cfg_write_control.min_size() << endl;
data <<"pbc_csr.port_3.cfg_write_control.packing_same_pg: 0x" << pbc_csr.port_3.cfg_write_control.packing_same_pg() << endl;
data <<"pbc_csr.port_3.cfg_write_control.packing: 0x" << pbc_csr.port_3.cfg_write_control.packing() << endl;
data <<"pbc_csr.port_3.cfg_write_control.enqueue_mux_bypass: 0x" << pbc_csr.port_3.cfg_write_control.enqueue_mux_bypass() << endl;
data <<"pbc_csr.port_3.cfg_write_control.enable_drdy: 0x" << pbc_csr.port_3.cfg_write_control.enable_drdy() << endl;
data <<"pbc_csr.port_3.cfg_write_control.rate_limiter: 0x" << pbc_csr.port_3.cfg_write_control.rate_limiter() << endl;
data <<"pbc_csr.port_3.cfg_write_control.cut_thru: 0x" << pbc_csr.port_3.cfg_write_control.cut_thru() << endl;
data <<"pbc_csr.port_3.cfg_write_control.enable_mnic_multicast: 0x" << pbc_csr.port_3.cfg_write_control.enable_mnic_multicast() << endl;
data <<"pbc_csr.port_3.cfg_write_control.enable_bmc_multicast: 0x" << pbc_csr.port_3.cfg_write_control.enable_bmc_multicast() << endl;

                    pbc_csr.port_3.cfg_write_control.write();

                    data <<"pbc_csr.port_3.cfg_oq.all: 0x" << pbc_csr.port_3.cfg_oq.all() << endl;
data <<"pbc_csr.port_3.cfg_oq.enable: 0x" << pbc_csr.port_3.cfg_oq.enable() << endl;
data <<"pbc_csr.port_3.cfg_oq.num_hdr_bytes: 0x" << pbc_csr.port_3.cfg_oq.num_hdr_bytes() << endl;
data <<"pbc_csr.port_3.cfg_oq.num_hdr_bytes_crypto: 0x" << pbc_csr.port_3.cfg_oq.num_hdr_bytes_crypto() << endl;
data <<"pbc_csr.port_3.cfg_oq.span_cpu_num_cells: 0x" << pbc_csr.port_3.cfg_oq.span_cpu_num_cells() << endl;
data <<"pbc_csr.port_3.cfg_oq.flush_hw_error: 0x" << pbc_csr.port_3.cfg_oq.flush_hw_error() << endl;
data <<"pbc_csr.port_3.cfg_oq.eg_ts_enable: 0x" << pbc_csr.port_3.cfg_oq.eg_ts_enable() << endl;
data <<"pbc_csr.port_3.cfg_oq.ipg_bytes: 0x" << pbc_csr.port_3.cfg_oq.ipg_bytes() << endl;
data <<"pbc_csr.port_3.cfg_oq.flush: 0x" << pbc_csr.port_3.cfg_oq.flush() << endl;
data <<"pbc_csr.port_3.cfg_oq.flow_control_enable_xoff: 0x" << pbc_csr.port_3.cfg_oq.flow_control_enable_xoff() << endl;
data <<"pbc_csr.port_3.cfg_oq.hw_error_to_pbus: 0x" << pbc_csr.port_3.cfg_oq.hw_error_to_pbus() << endl;
data <<"pbc_csr.port_3.cfg_oq.rewrite_enable: 0x" << pbc_csr.port_3.cfg_oq.rewrite_enable() << endl;

                    pbc_csr.port_3.cfg_oq.write();
                }
                break;
            }
        default:
            return sdk::SDK_RET_ERR;
    }
    SDK_TRACE_DEBUG("%s", data.str().c_str());

    /* If we're disabling the port, we need to wait until all HBM contexts are
     * free
     */
    if (!enable) {
        ret = elba_tm_drain_uplink_port(port);
        if (ret != sdk::SDK_RET_OK) {
            SDK_TRACE_ERR("Port %u is not fully drained", port);
            // Ignore the return status and continue
            ret = sdk::SDK_RET_OK;
        }
    }

    SDK_TRACE_DEBUG("%s uplink port %u",
                    enable ? "Enable" : "Disable", port);
    return sdk::SDK_RET_OK;
}

sdk_ret_t
elba_tm_hw_config_load_poll (int phase)
{
    if (phase == 0) {
        elb_pb_init_done(0,0);
    }
    return sdk::SDK_RET_OK;
}

sdk_ret_t
elba_tm_asic_init (void)
{
    elb_top_csr_t &elb0 = ELB_BLK_REG_MODEL_ACCESS(elb_top_csr_t, 0, 0);
    elb_pbc_csr_t &pbc_csr = elb0.pb.pbc;
    elb_pf_csr_t &pf_csr = elb0.pf.pf;

    // Do a sw reset prior to reconfiguring the islands etc
    pbc_csr.cfg_pbc_control.sw_reset(1);
    pbc_csr.cfg_pbc_control.write();
    pf_csr.cfg_hbm.control_sw_reset(1);
    pf_csr.cfg_hbm.write();

    pbc_csr.cfg_pbc_control.read();
    pf_csr.cfg_hbm.read();

    pbc_csr.cfg_pbc_control.sw_reset(0);
    pbc_csr.cfg_pbc_control.write();
    pf_csr.cfg_hbm.control_sw_reset(0);
    pf_csr.cfg_hbm.write();

    pbc_csr.cfg_pbc_control.read();
    pf_csr.cfg_hbm.read();
    //    elb_pb_init_start(0,0);
    //    elb_pb_init_done(0,0);
    return sdk::SDK_RET_OK;
}

static sdk_ret_t
alloc_cells (tm_port_type_e port_type, uint32_t *pbc_cell_chunks,
             elba_tm_buf_cfg_t *buf_cfg)
{
    sdk_ret_t ret = sdk::SDK_RET_OK;
    uint32_t reserved_mtus;
    uint32_t reserved_cells;
    uint32_t headroom_cells;
    uint32_t chunks_per_q;
    uint32_t island;
    uint32_t num_qs;
    uint32_t reserved_chunks;
    uint32_t high_perf_reserved_chunks;
    uint32_t headroom_chunks;
    uint32_t chunks_needed;
    uint32_t num_high_perf_qs = tm_cfg_profile()->num_p4_high_perf_qs;

    island = elba_tm_get_island_for_port_type(port_type);

    num_qs = tm_cfg_profile()->num_qs[port_type];
    if (port_type == TM_PORT_TYPE_UPLINK) {
        num_qs =
            1 + (tm_cfg_profile()->num_qs[port_type] * tm_cfg_profile()->num_active_uplink_ports);
    }

    reserved_mtus = tm_asic_profile()->port[port_type].reserved_mtus;
    headroom_cells = tm_asic_profile()->port[port_type].headroom_cells;

    headroom_chunks = cells_to_chunks(headroom_cells);

    if (reserved_mtus) {
        // Allocate reserved_mtus and headroom_cells from the given island
        reserved_cells =
            bytes_to_cells(tm_cfg_profile()->jumbo_mtu[port_type]) * reserved_mtus;

        reserved_chunks = cells_to_chunks(reserved_cells);

        high_perf_reserved_chunks = reserved_chunks * 2;

        chunks_needed = (reserved_chunks + headroom_chunks) * (num_qs - num_high_perf_qs);
        chunks_needed = chunks_needed + 
            (high_perf_reserved_chunks + headroom_chunks) * num_high_perf_qs;

        if (pbc_cell_chunks[island] < chunks_needed) {
            SDK_TRACE_ERR("Error allocating reserved pbc chunks "
                          "island %u port_type %u num_qs %u reserved_cells %u "
                          "headroom_cells %u available %u",
                          island, port_type, num_qs, reserved_cells,
                          headroom_cells, chunks_to_cells(pbc_cell_chunks[island]));
            return sdk::SDK_RET_NO_RESOURCE;
        }
        pbc_cell_chunks[island] -= chunks_needed;
        buf_cfg->chunks_per_q[port_type] = reserved_chunks;
    } else {
        // Allocate the remaining chunks in the island for every queue
        chunks_per_q = pbc_cell_chunks[island]/num_qs;
        if (chunks_per_q < headroom_chunks) {
            SDK_TRACE_ERR("Error allocating remaining pbc chunks "
                          "island %u port_type %u num_qs %u "
                          "headroom_cells %u available %u per_q available %u",
                          island, port_type, num_qs,
                          headroom_cells,
                          chunks_to_cells(pbc_cell_chunks[island]),
                          chunks_to_cells(chunks_per_q));
            return sdk::SDK_RET_NO_RESOURCE;
        }
        pbc_cell_chunks[island] -= chunks_per_q * num_qs;
        buf_cfg->chunks_per_q[port_type] = chunks_per_q - headroom_chunks;
    }
    if (chunks_to_cells(buf_cfg->chunks_per_q[port_type]) <
        bytes_to_cells(tm_cfg_profile()->jumbo_mtu[port_type])) {
        SDK_TRACE_ERR("Error reserved cells %u/bytes %u is less than the "
                      "jumbo mtu  %u",
                      chunks_to_cells(buf_cfg->chunks_per_q[port_type]),
                      cells_to_bytes(chunks_to_cells(buf_cfg->chunks_per_q[port_type])),
                      tm_cfg_profile()->jumbo_mtu[port_type]);
        return sdk::SDK_RET_NO_RESOURCE;
    }
    SDK_TRACE_DEBUG("allocated cells %u port_type %u headroom %u",
                    chunks_to_cells(buf_cfg->chunks_per_q[port_type]),
                    port_type,
                    chunks_to_cells(headroom_chunks));

    return ret;
}

static sdk_ret_t
elba_tm_alloc_pbc_buffers (elba_tm_buf_cfg_t *buf_cfg)
{
    sdk_ret_t ret = sdk::SDK_RET_OK;
    uint32_t pbc_cell_chunks[ELBA_TM_NUM_BUFFER_ISLANDS] = {0};

    for (unsigned i = 0; i < SDK_ARRAY_SIZE(pbc_cell_chunks); i++) {
        pbc_cell_chunks[i] = elba_tm_get_max_cell_chunks_for_island(i);
    }

    /* First allocate buffer cells for the P4 ports and
     * then distribute the remaining equally among the different classes
     */
    ret = alloc_cells(TM_PORT_TYPE_P4IG, pbc_cell_chunks, buf_cfg);
    if (ret != sdk::SDK_RET_OK) {
        return ret;
    }

    ret = alloc_cells(TM_PORT_TYPE_P4EG, pbc_cell_chunks, buf_cfg);
    if (ret != sdk::SDK_RET_OK) {
        return ret;
    }

    /* Now allocate the remaining uniformly */
    ret = alloc_cells(TM_PORT_TYPE_UPLINK, pbc_cell_chunks, buf_cfg);
    if (ret != sdk::SDK_RET_OK) {
        return ret;
    }

    ret = alloc_cells(TM_PORT_TYPE_DMA, pbc_cell_chunks, buf_cfg);
    if (ret != sdk::SDK_RET_OK) {
        return ret;
    }

    for (unsigned i = 0; i < SDK_ARRAY_SIZE(pbc_cell_chunks); i++) {
        SDK_TRACE_DEBUG("unallocated cells island %u cells %u",
                        i, chunks_to_cells(pbc_cell_chunks[i]));
    }

    return sdk::SDK_RET_OK;
}

static sdk_ret_t
elba_tm_alloc_hbm_buffers (elba_tm_buf_cfg_t *buf_cfg)
{
    uint32_t num_hbm_contexts[NUM_TM_HBM_FIFO_TYPES] = {0};
    uint64_t total_hbm_chunks;
    uint32_t fifo_type;

    // HBM allocation
    //
    // Out of the whole available HBM payload and control has to be carved.
    // Control needs to be 1/50th of payload
    num_hbm_contexts[TM_HBM_FIFO_TYPE_UPLINK] =
        tm_cfg_profile()->num_qs[TM_PORT_TYPE_UPLINK] * tm_cfg_profile()->num_active_uplink_ports;
    num_hbm_contexts[TM_HBM_FIFO_TYPE_TXDMA] = tm_cfg_profile()->num_qs[TM_PORT_TYPE_DMA];

    total_hbm_chunks = tm_cfg_profile()->hbm_fifo_size/ELBA_TM_HBM_FIFO_ALLOC_SIZE;

    // This calculation involves taking the floor value for both
    // total_hbm_chunks and control_chunks
    uint64_t control_chunks;
    uint64_t payload_chunks;

    control_chunks = total_hbm_chunks/(1 + tm_asic_profile()->hbm_fifo_control_scale_factor);
    payload_chunks = control_chunks * tm_asic_profile()->hbm_fifo_control_scale_factor;
    SDK_ASSERT((payload_chunks + control_chunks) <= total_hbm_chunks);

    SDK_TRACE_DEBUG("Available hbm chunks total %u payload %u control %u",
                    total_hbm_chunks, payload_chunks, control_chunks);

    uint64_t total_reserved_hbm_chunks = 0;
    uint64_t total_hbm_contexts = 0;
    uint64_t reserved_hbm_chunks_per_context[NUM_TM_HBM_FIFO_TYPES] = {0};
    for (fifo_type = 0; fifo_type < NUM_TM_HBM_FIFO_TYPES; fifo_type++) {
        reserved_hbm_chunks_per_context[fifo_type] =
            hbm_bytes_to_chunks(
                tm_asic_profile()->hbm_fifo_reserved_bytes_per_context[fifo_type]);

        if (num_hbm_contexts[fifo_type] > elba_tm_max_hbm_contexts_for_fifo(fifo_type)) {
            SDK_TRACE_ERR("num hbm contexts %u for fifo %u exceeds "
                          "max available %u",
                          num_hbm_contexts[fifo_type],
                          fifo_type,
                          elba_tm_max_hbm_contexts_for_fifo(fifo_type));
            return sdk::SDK_RET_INVALID_ARG;
        }
        uint64_t reserved_hbm_chunks =
            reserved_hbm_chunks_per_context[fifo_type] * num_hbm_contexts[fifo_type];

        total_reserved_hbm_chunks += reserved_hbm_chunks;
        total_hbm_contexts += num_hbm_contexts[fifo_type];
    }

    if (payload_chunks < total_reserved_hbm_chunks) {
        SDK_TRACE_ERR("Error allocating hbm fifo . Available chunks %u "
                      "total reserved required %u ",
                      payload_chunks, total_reserved_hbm_chunks);
        return sdk::SDK_RET_NO_RESOURCE;
    }
    uint64_t rem_payload_chunks = payload_chunks - total_reserved_hbm_chunks;

    // Allocate the rem_payload_chunks for the total number of queues
    //
    uint64_t payload_chunks_per_context = rem_payload_chunks/total_hbm_contexts;

    SDK_TRACE_DEBUG("HBM fifo allocation total payload %u, reserved %u "
                    "remaining %u total_contexts %u "
                    "payload chunks per context %u",
                    payload_chunks, total_reserved_hbm_chunks,
                    rem_payload_chunks, total_hbm_contexts, payload_chunks_per_context);

    uint64_t offset = 0;
    for (fifo_type = 0; fifo_type < NUM_TM_HBM_FIFO_TYPES; fifo_type++) {
        for (uint32_t context = 0; context < num_hbm_contexts[fifo_type]; context++) {
            uint64_t payload_chunks_needed =
                payload_chunks_per_context + reserved_hbm_chunks_per_context[fifo_type];
            uint64_t control_chunks_needed =
                (payload_chunks_needed + tm_asic_profile()->hbm_fifo_control_scale_factor - 1)/
                tm_asic_profile()->hbm_fifo_control_scale_factor;

            buf_cfg->hbm_fifo[fifo_type][context].valid = true;
            buf_cfg->hbm_fifo[fifo_type][context].payload_offset = offset;
            buf_cfg->hbm_fifo[fifo_type][context].payload_chunks = payload_chunks_needed;
            offset += payload_chunks_needed;
            buf_cfg->hbm_fifo[fifo_type][context].control_offset = offset;
            buf_cfg->hbm_fifo[fifo_type][context].control_chunks = control_chunks_needed;
            offset += control_chunks_needed;
        }
    }
    SDK_ASSERT(offset <= (control_chunks + payload_chunks));
    SDK_TRACE_DEBUG("unallocated hbm chunks %u total %u",
                    (control_chunks + payload_chunks) - offset,
                    total_hbm_chunks);
    return sdk::SDK_RET_OK;
}

static bool
iq_disabled (tm_port_t port, uint32_t iq)
{
    tm_port_type_e port_type;
    bool disabled = false;
    uint32_t max_iqs;
    uint32_t num_iqs;
    uint32_t disabled_iq_start, disabled_iq_end;

    port_type = elba_tm_get_port_type(port);
    num_iqs = tm_cfg_profile()->num_qs[port_type];

    switch(port_type) {
        case TM_PORT_TYPE_UPLINK:
            if (port == ELBA_TM_PORT_NCSI) {
                // On the BMC port, only one pg is supported
                num_iqs = 1;
            } else if (!is_active_uplink_port(port)) {
                disabled = true;
            }
            if (iq >= num_iqs) {
                disabled = true;
            }
            break;
        case TM_PORT_TYPE_P4EG:
        case TM_PORT_TYPE_P4IG:
            max_iqs = elba_tm_get_num_iqs_for_port_type(port_type);
            SDK_ASSERT(max_iqs > num_iqs);
            disabled_iq_start = ELBA_TM_P4_UPLINK_IQ_OFFSET - (max_iqs - num_iqs) - 1;
            disabled_iq_end = ELBA_TM_P4_UPLINK_IQ_OFFSET - 1;
            if ((iq >= disabled_iq_start) && (iq < disabled_iq_end)) {
                disabled = true;
            }
            break;
        case TM_PORT_TYPE_DMA:
            if (iq >= num_iqs) {
                disabled = true;
            }
            break;
        case NUM_TM_PORT_TYPES:
            disabled = true;
            break;
    }

    if (disabled) {
        SDK_TRACE_DEBUG("iq %u on port %u disabled",
                        iq, port);
    }

    return disabled;
}

static sdk_ret_t
elba_tm_program_pbc_buffers (elba_tm_buf_cfg_t *buf_cfg)
{
    sdk_ret_t ret = sdk::SDK_RET_OK;
    elb_top_csr_t &elb0 = ELB_BLK_REG_MODEL_ACCESS(elb_top_csr_t, 0, 0);
    elb_pbc_csr_t &pbc_csr = elb0.pb.pbc;
    uint32_t reserved_min;
    uint32_t headroom;
    uint32_t xon_threshold;
    tm_port_type_e port_type;

    stringstream data;
    data << hex << endl;
    // Program the buffers
    port_type = elba_tm_get_port_type(ELBA_TM_PORT_UPLINK_0);
    reserved_min = 0;
    headroom = 0;
    xon_threshold = 0;
    if (!iq_disabled(ELBA_TM_PORT_UPLINK_0, 0)) {
        if (tm_asic_profile()->port[port_type].uses_credits &&
            tm_asic_profile()->port[port_type].recirc_q != 0) {
            reserved_min = buf_cfg->chunks_per_q[port_type];

            if (tm_is_high_perf_q(0)) {
                reserved_min *= 2;
            }
            reserved_min = reserved_min +
                cells_to_chunks(bytes_to_cells(tm_cfg_profile()->jumbo_mtu[port_type])) + 2;
            headroom = 0;
        } else {
            reserved_min = buf_cfg->chunks_per_q[port_type];
            headroom = cells_to_chunks(tm_asic_profile()->port[port_type].headroom_cells);
        }
    }

    if (headroom) {
        SDK_ASSERT(chunks_to_cells(reserved_min) >=
                   bytes_to_cells(tm_cfg_profile()->jumbo_mtu[port_type]));
        xon_threshold = chunks_to_cells(reserved_min) -
            bytes_to_cells(tm_cfg_profile()->jumbo_mtu[port_type]);
    }

    if (port_type == TM_PORT_TYPE_DMA) {
        // Based on asic config
        xon_threshold = 13;
    }

    pbc_csr.port_0.cfg_account_pg_0.read();
    /* Update the PG parameters */
    pbc_csr.port_0.cfg_account_pg_0.reserved_min(reserved_min);
    pbc_csr.port_0.cfg_account_pg_0.headroom(headroom);
    pbc_csr.port_0.cfg_account_pg_0.xon_threshold(xon_threshold);
    pbc_csr.port_0.cfg_account_pg_0.low_limit(0);
    pbc_csr.port_0.cfg_account_pg_0.alpha(0);

    if (tm_sw_init_enabled()) {
        data <<"pbc_csr.port_0.cfg_account_pg_0.all: 0x" << pbc_csr.port_0.cfg_account_pg_0.all() << endl;
data <<"pbc_csr.port_0.cfg_account_pg_0.alpha: 0x" << pbc_csr.port_0.cfg_account_pg_0.alpha() << endl;
data <<"pbc_csr.port_0.cfg_account_pg_0.xon_threshold: 0x" << pbc_csr.port_0.cfg_account_pg_0.xon_threshold() << endl;
data <<"pbc_csr.port_0.cfg_account_pg_0.headroom: 0x" << pbc_csr.port_0.cfg_account_pg_0.headroom() << endl;
data <<"pbc_csr.port_0.cfg_account_pg_0.reserved_min: 0x" << pbc_csr.port_0.cfg_account_pg_0.reserved_min() << endl;
data <<"pbc_csr.port_0.cfg_account_pg_0.low_limit: 0x" << pbc_csr.port_0.cfg_account_pg_0.low_limit() << endl;

        pbc_csr.port_0.cfg_account_pg_0.write();
    }
    reserved_min = 0;
    headroom = 0;
    xon_threshold = 0;
    if (!iq_disabled(ELBA_TM_PORT_UPLINK_0, 1)) {
        if (tm_asic_profile()->port[port_type].uses_credits &&
            tm_asic_profile()->port[port_type].recirc_q != 1) {
            reserved_min = buf_cfg->chunks_per_q[port_type];

            if (tm_is_high_perf_q(1)) {
                reserved_min *= 2;
            }
            reserved_min = reserved_min +
                cells_to_chunks(bytes_to_cells(tm_cfg_profile()->jumbo_mtu[port_type])) + 2;
            headroom = 0;
        } else {
            reserved_min = buf_cfg->chunks_per_q[port_type];
            headroom = cells_to_chunks(tm_asic_profile()->port[port_type].headroom_cells);
        }
    }

    if (headroom) {
        SDK_ASSERT(chunks_to_cells(reserved_min) >=
                   bytes_to_cells(tm_cfg_profile()->jumbo_mtu[port_type]));
        xon_threshold = chunks_to_cells(reserved_min) -
            bytes_to_cells(tm_cfg_profile()->jumbo_mtu[port_type]);
    }

    if (port_type == TM_PORT_TYPE_DMA) {
        // Based on asic config
        xon_threshold = 13;
    }

    pbc_csr.port_0.cfg_account_pg_1.read();
    /* Update the PG parameters */
    pbc_csr.port_0.cfg_account_pg_1.reserved_min(reserved_min);
    pbc_csr.port_0.cfg_account_pg_1.headroom(headroom);
    pbc_csr.port_0.cfg_account_pg_1.xon_threshold(xon_threshold);
    pbc_csr.port_0.cfg_account_pg_1.low_limit(0);
    pbc_csr.port_0.cfg_account_pg_1.alpha(0);

    if (tm_sw_init_enabled()) {
        data <<"pbc_csr.port_0.cfg_account_pg_1.all: 0x" << pbc_csr.port_0.cfg_account_pg_1.all() << endl;
data <<"pbc_csr.port_0.cfg_account_pg_1.alpha: 0x" << pbc_csr.port_0.cfg_account_pg_1.alpha() << endl;
data <<"pbc_csr.port_0.cfg_account_pg_1.xon_threshold: 0x" << pbc_csr.port_0.cfg_account_pg_1.xon_threshold() << endl;
data <<"pbc_csr.port_0.cfg_account_pg_1.headroom: 0x" << pbc_csr.port_0.cfg_account_pg_1.headroom() << endl;
data <<"pbc_csr.port_0.cfg_account_pg_1.reserved_min: 0x" << pbc_csr.port_0.cfg_account_pg_1.reserved_min() << endl;
data <<"pbc_csr.port_0.cfg_account_pg_1.low_limit: 0x" << pbc_csr.port_0.cfg_account_pg_1.low_limit() << endl;

        pbc_csr.port_0.cfg_account_pg_1.write();
    }
    reserved_min = 0;
    headroom = 0;
    xon_threshold = 0;
    if (!iq_disabled(ELBA_TM_PORT_UPLINK_0, 2)) {
        if (tm_asic_profile()->port[port_type].uses_credits &&
            tm_asic_profile()->port[port_type].recirc_q != 2) {
            reserved_min = buf_cfg->chunks_per_q[port_type];

            if (tm_is_high_perf_q(2)) {
                reserved_min *= 2;
            }
            reserved_min = reserved_min +
                cells_to_chunks(bytes_to_cells(tm_cfg_profile()->jumbo_mtu[port_type])) + 2;
            headroom = 0;
        } else {
            reserved_min = buf_cfg->chunks_per_q[port_type];
            headroom = cells_to_chunks(tm_asic_profile()->port[port_type].headroom_cells);
        }
    }

    if (headroom) {
        SDK_ASSERT(chunks_to_cells(reserved_min) >=
                   bytes_to_cells(tm_cfg_profile()->jumbo_mtu[port_type]));
        xon_threshold = chunks_to_cells(reserved_min) -
            bytes_to_cells(tm_cfg_profile()->jumbo_mtu[port_type]);
    }

    if (port_type == TM_PORT_TYPE_DMA) {
        // Based on asic config
        xon_threshold = 13;
    }

    pbc_csr.port_0.cfg_account_pg_2.read();
    /* Update the PG parameters */
    pbc_csr.port_0.cfg_account_pg_2.reserved_min(reserved_min);
    pbc_csr.port_0.cfg_account_pg_2.headroom(headroom);
    pbc_csr.port_0.cfg_account_pg_2.xon_threshold(xon_threshold);
    pbc_csr.port_0.cfg_account_pg_2.low_limit(0);
    pbc_csr.port_0.cfg_account_pg_2.alpha(0);

    if (tm_sw_init_enabled()) {
        data <<"pbc_csr.port_0.cfg_account_pg_2.all: 0x" << pbc_csr.port_0.cfg_account_pg_2.all() << endl;
data <<"pbc_csr.port_0.cfg_account_pg_2.alpha: 0x" << pbc_csr.port_0.cfg_account_pg_2.alpha() << endl;
data <<"pbc_csr.port_0.cfg_account_pg_2.xon_threshold: 0x" << pbc_csr.port_0.cfg_account_pg_2.xon_threshold() << endl;
data <<"pbc_csr.port_0.cfg_account_pg_2.headroom: 0x" << pbc_csr.port_0.cfg_account_pg_2.headroom() << endl;
data <<"pbc_csr.port_0.cfg_account_pg_2.reserved_min: 0x" << pbc_csr.port_0.cfg_account_pg_2.reserved_min() << endl;
data <<"pbc_csr.port_0.cfg_account_pg_2.low_limit: 0x" << pbc_csr.port_0.cfg_account_pg_2.low_limit() << endl;

        pbc_csr.port_0.cfg_account_pg_2.write();
    }
    reserved_min = 0;
    headroom = 0;
    xon_threshold = 0;
    if (!iq_disabled(ELBA_TM_PORT_UPLINK_0, 3)) {
        if (tm_asic_profile()->port[port_type].uses_credits &&
            tm_asic_profile()->port[port_type].recirc_q != 3) {
            reserved_min = buf_cfg->chunks_per_q[port_type];

            if (tm_is_high_perf_q(3)) {
                reserved_min *= 2;
            }
            reserved_min = reserved_min +
                cells_to_chunks(bytes_to_cells(tm_cfg_profile()->jumbo_mtu[port_type])) + 2;
            headroom = 0;
        } else {
            reserved_min = buf_cfg->chunks_per_q[port_type];
            headroom = cells_to_chunks(tm_asic_profile()->port[port_type].headroom_cells);
        }
    }

    if (headroom) {
        SDK_ASSERT(chunks_to_cells(reserved_min) >=
                   bytes_to_cells(tm_cfg_profile()->jumbo_mtu[port_type]));
        xon_threshold = chunks_to_cells(reserved_min) -
            bytes_to_cells(tm_cfg_profile()->jumbo_mtu[port_type]);
    }

    if (port_type == TM_PORT_TYPE_DMA) {
        // Based on asic config
        xon_threshold = 13;
    }

    pbc_csr.port_0.cfg_account_pg_3.read();
    /* Update the PG parameters */
    pbc_csr.port_0.cfg_account_pg_3.reserved_min(reserved_min);
    pbc_csr.port_0.cfg_account_pg_3.headroom(headroom);
    pbc_csr.port_0.cfg_account_pg_3.xon_threshold(xon_threshold);
    pbc_csr.port_0.cfg_account_pg_3.low_limit(0);
    pbc_csr.port_0.cfg_account_pg_3.alpha(0);

    if (tm_sw_init_enabled()) {
        data <<"pbc_csr.port_0.cfg_account_pg_3.all: 0x" << pbc_csr.port_0.cfg_account_pg_3.all() << endl;
data <<"pbc_csr.port_0.cfg_account_pg_3.alpha: 0x" << pbc_csr.port_0.cfg_account_pg_3.alpha() << endl;
data <<"pbc_csr.port_0.cfg_account_pg_3.xon_threshold: 0x" << pbc_csr.port_0.cfg_account_pg_3.xon_threshold() << endl;
data <<"pbc_csr.port_0.cfg_account_pg_3.headroom: 0x" << pbc_csr.port_0.cfg_account_pg_3.headroom() << endl;
data <<"pbc_csr.port_0.cfg_account_pg_3.reserved_min: 0x" << pbc_csr.port_0.cfg_account_pg_3.reserved_min() << endl;
data <<"pbc_csr.port_0.cfg_account_pg_3.low_limit: 0x" << pbc_csr.port_0.cfg_account_pg_3.low_limit() << endl;

        pbc_csr.port_0.cfg_account_pg_3.write();
    }
    reserved_min = 0;
    headroom = 0;
    xon_threshold = 0;
    if (!iq_disabled(ELBA_TM_PORT_UPLINK_0, 4)) {
        if (tm_asic_profile()->port[port_type].uses_credits &&
            tm_asic_profile()->port[port_type].recirc_q != 4) {
            reserved_min = buf_cfg->chunks_per_q[port_type];

            if (tm_is_high_perf_q(4)) {
                reserved_min *= 2;
            }
            reserved_min = reserved_min +
                cells_to_chunks(bytes_to_cells(tm_cfg_profile()->jumbo_mtu[port_type])) + 2;
            headroom = 0;
        } else {
            reserved_min = buf_cfg->chunks_per_q[port_type];
            headroom = cells_to_chunks(tm_asic_profile()->port[port_type].headroom_cells);
        }
    }

    if (headroom) {
        SDK_ASSERT(chunks_to_cells(reserved_min) >=
                   bytes_to_cells(tm_cfg_profile()->jumbo_mtu[port_type]));
        xon_threshold = chunks_to_cells(reserved_min) -
            bytes_to_cells(tm_cfg_profile()->jumbo_mtu[port_type]);
    }

    if (port_type == TM_PORT_TYPE_DMA) {
        // Based on asic config
        xon_threshold = 13;
    }

    pbc_csr.port_0.cfg_account_pg_4.read();
    /* Update the PG parameters */
    pbc_csr.port_0.cfg_account_pg_4.reserved_min(reserved_min);
    pbc_csr.port_0.cfg_account_pg_4.headroom(headroom);
    pbc_csr.port_0.cfg_account_pg_4.xon_threshold(xon_threshold);
    pbc_csr.port_0.cfg_account_pg_4.low_limit(0);
    pbc_csr.port_0.cfg_account_pg_4.alpha(0);

    if (tm_sw_init_enabled()) {
        data <<"pbc_csr.port_0.cfg_account_pg_4.all: 0x" << pbc_csr.port_0.cfg_account_pg_4.all() << endl;
data <<"pbc_csr.port_0.cfg_account_pg_4.alpha: 0x" << pbc_csr.port_0.cfg_account_pg_4.alpha() << endl;
data <<"pbc_csr.port_0.cfg_account_pg_4.xon_threshold: 0x" << pbc_csr.port_0.cfg_account_pg_4.xon_threshold() << endl;
data <<"pbc_csr.port_0.cfg_account_pg_4.headroom: 0x" << pbc_csr.port_0.cfg_account_pg_4.headroom() << endl;
data <<"pbc_csr.port_0.cfg_account_pg_4.reserved_min: 0x" << pbc_csr.port_0.cfg_account_pg_4.reserved_min() << endl;
data <<"pbc_csr.port_0.cfg_account_pg_4.low_limit: 0x" << pbc_csr.port_0.cfg_account_pg_4.low_limit() << endl;

        pbc_csr.port_0.cfg_account_pg_4.write();
    }
    reserved_min = 0;
    headroom = 0;
    xon_threshold = 0;
    if (!iq_disabled(ELBA_TM_PORT_UPLINK_0, 5)) {
        if (tm_asic_profile()->port[port_type].uses_credits &&
            tm_asic_profile()->port[port_type].recirc_q != 5) {
            reserved_min = buf_cfg->chunks_per_q[port_type];

            if (tm_is_high_perf_q(5)) {
                reserved_min *= 2;
            }
            reserved_min = reserved_min +
                cells_to_chunks(bytes_to_cells(tm_cfg_profile()->jumbo_mtu[port_type])) + 2;
            headroom = 0;
        } else {
            reserved_min = buf_cfg->chunks_per_q[port_type];
            headroom = cells_to_chunks(tm_asic_profile()->port[port_type].headroom_cells);
        }
    }

    if (headroom) {
        SDK_ASSERT(chunks_to_cells(reserved_min) >=
                   bytes_to_cells(tm_cfg_profile()->jumbo_mtu[port_type]));
        xon_threshold = chunks_to_cells(reserved_min) -
            bytes_to_cells(tm_cfg_profile()->jumbo_mtu[port_type]);
    }

    if (port_type == TM_PORT_TYPE_DMA) {
        // Based on asic config
        xon_threshold = 13;
    }

    pbc_csr.port_0.cfg_account_pg_5.read();
    /* Update the PG parameters */
    pbc_csr.port_0.cfg_account_pg_5.reserved_min(reserved_min);
    pbc_csr.port_0.cfg_account_pg_5.headroom(headroom);
    pbc_csr.port_0.cfg_account_pg_5.xon_threshold(xon_threshold);
    pbc_csr.port_0.cfg_account_pg_5.low_limit(0);
    pbc_csr.port_0.cfg_account_pg_5.alpha(0);

    if (tm_sw_init_enabled()) {
        data <<"pbc_csr.port_0.cfg_account_pg_5.all: 0x" << pbc_csr.port_0.cfg_account_pg_5.all() << endl;
data <<"pbc_csr.port_0.cfg_account_pg_5.alpha: 0x" << pbc_csr.port_0.cfg_account_pg_5.alpha() << endl;
data <<"pbc_csr.port_0.cfg_account_pg_5.xon_threshold: 0x" << pbc_csr.port_0.cfg_account_pg_5.xon_threshold() << endl;
data <<"pbc_csr.port_0.cfg_account_pg_5.headroom: 0x" << pbc_csr.port_0.cfg_account_pg_5.headroom() << endl;
data <<"pbc_csr.port_0.cfg_account_pg_5.reserved_min: 0x" << pbc_csr.port_0.cfg_account_pg_5.reserved_min() << endl;
data <<"pbc_csr.port_0.cfg_account_pg_5.low_limit: 0x" << pbc_csr.port_0.cfg_account_pg_5.low_limit() << endl;

        pbc_csr.port_0.cfg_account_pg_5.write();
    }
    reserved_min = 0;
    headroom = 0;
    xon_threshold = 0;
    if (!iq_disabled(ELBA_TM_PORT_UPLINK_0, 6)) {
        if (tm_asic_profile()->port[port_type].uses_credits &&
            tm_asic_profile()->port[port_type].recirc_q != 6) {
            reserved_min = buf_cfg->chunks_per_q[port_type];

            if (tm_is_high_perf_q(6)) {
                reserved_min *= 2;
            }
            reserved_min = reserved_min +
                cells_to_chunks(bytes_to_cells(tm_cfg_profile()->jumbo_mtu[port_type])) + 2;
            headroom = 0;
        } else {
            reserved_min = buf_cfg->chunks_per_q[port_type];
            headroom = cells_to_chunks(tm_asic_profile()->port[port_type].headroom_cells);
        }
    }

    if (headroom) {
        SDK_ASSERT(chunks_to_cells(reserved_min) >=
                   bytes_to_cells(tm_cfg_profile()->jumbo_mtu[port_type]));
        xon_threshold = chunks_to_cells(reserved_min) -
            bytes_to_cells(tm_cfg_profile()->jumbo_mtu[port_type]);
    }

    if (port_type == TM_PORT_TYPE_DMA) {
        // Based on asic config
        xon_threshold = 13;
    }

    pbc_csr.port_0.cfg_account_pg_6.read();
    /* Update the PG parameters */
    pbc_csr.port_0.cfg_account_pg_6.reserved_min(reserved_min);
    pbc_csr.port_0.cfg_account_pg_6.headroom(headroom);
    pbc_csr.port_0.cfg_account_pg_6.xon_threshold(xon_threshold);
    pbc_csr.port_0.cfg_account_pg_6.low_limit(0);
    pbc_csr.port_0.cfg_account_pg_6.alpha(0);

    if (tm_sw_init_enabled()) {
        data <<"pbc_csr.port_0.cfg_account_pg_6.all: 0x" << pbc_csr.port_0.cfg_account_pg_6.all() << endl;
data <<"pbc_csr.port_0.cfg_account_pg_6.alpha: 0x" << pbc_csr.port_0.cfg_account_pg_6.alpha() << endl;
data <<"pbc_csr.port_0.cfg_account_pg_6.xon_threshold: 0x" << pbc_csr.port_0.cfg_account_pg_6.xon_threshold() << endl;
data <<"pbc_csr.port_0.cfg_account_pg_6.headroom: 0x" << pbc_csr.port_0.cfg_account_pg_6.headroom() << endl;
data <<"pbc_csr.port_0.cfg_account_pg_6.reserved_min: 0x" << pbc_csr.port_0.cfg_account_pg_6.reserved_min() << endl;
data <<"pbc_csr.port_0.cfg_account_pg_6.low_limit: 0x" << pbc_csr.port_0.cfg_account_pg_6.low_limit() << endl;

        pbc_csr.port_0.cfg_account_pg_6.write();
    }
    reserved_min = 0;
    headroom = 0;
    xon_threshold = 0;
    if (!iq_disabled(ELBA_TM_PORT_UPLINK_0, 7)) {
        if (tm_asic_profile()->port[port_type].uses_credits &&
            tm_asic_profile()->port[port_type].recirc_q != 7) {
            reserved_min = buf_cfg->chunks_per_q[port_type];

            if (tm_is_high_perf_q(7)) {
                reserved_min *= 2;
            }
            reserved_min = reserved_min +
                cells_to_chunks(bytes_to_cells(tm_cfg_profile()->jumbo_mtu[port_type])) + 2;
            headroom = 0;
        } else {
            reserved_min = buf_cfg->chunks_per_q[port_type];
            headroom = cells_to_chunks(tm_asic_profile()->port[port_type].headroom_cells);
        }
    }

    if (headroom) {
        SDK_ASSERT(chunks_to_cells(reserved_min) >=
                   bytes_to_cells(tm_cfg_profile()->jumbo_mtu[port_type]));
        xon_threshold = chunks_to_cells(reserved_min) -
            bytes_to_cells(tm_cfg_profile()->jumbo_mtu[port_type]);
    }

    if (port_type == TM_PORT_TYPE_DMA) {
        // Based on asic config
        xon_threshold = 13;
    }

    pbc_csr.port_0.cfg_account_pg_7.read();
    /* Update the PG parameters */
    pbc_csr.port_0.cfg_account_pg_7.reserved_min(reserved_min);
    pbc_csr.port_0.cfg_account_pg_7.headroom(headroom);
    pbc_csr.port_0.cfg_account_pg_7.xon_threshold(xon_threshold);
    pbc_csr.port_0.cfg_account_pg_7.low_limit(0);
    pbc_csr.port_0.cfg_account_pg_7.alpha(0);

    if (tm_sw_init_enabled()) {
        data <<"pbc_csr.port_0.cfg_account_pg_7.all: 0x" << pbc_csr.port_0.cfg_account_pg_7.all() << endl;
data <<"pbc_csr.port_0.cfg_account_pg_7.alpha: 0x" << pbc_csr.port_0.cfg_account_pg_7.alpha() << endl;
data <<"pbc_csr.port_0.cfg_account_pg_7.xon_threshold: 0x" << pbc_csr.port_0.cfg_account_pg_7.xon_threshold() << endl;
data <<"pbc_csr.port_0.cfg_account_pg_7.headroom: 0x" << pbc_csr.port_0.cfg_account_pg_7.headroom() << endl;
data <<"pbc_csr.port_0.cfg_account_pg_7.reserved_min: 0x" << pbc_csr.port_0.cfg_account_pg_7.reserved_min() << endl;
data <<"pbc_csr.port_0.cfg_account_pg_7.low_limit: 0x" << pbc_csr.port_0.cfg_account_pg_7.low_limit() << endl;

        pbc_csr.port_0.cfg_account_pg_7.write();
    }
    port_type = elba_tm_get_port_type(ELBA_TM_PORT_UPLINK_1);
    reserved_min = 0;
    headroom = 0;
    xon_threshold = 0;
    if (!iq_disabled(ELBA_TM_PORT_UPLINK_1, 0)) {
        if (tm_asic_profile()->port[port_type].uses_credits &&
            tm_asic_profile()->port[port_type].recirc_q != 0) {
            reserved_min = buf_cfg->chunks_per_q[port_type];

            if (tm_is_high_perf_q(0)) {
                reserved_min *= 2;
            }
            reserved_min = reserved_min +
                cells_to_chunks(bytes_to_cells(tm_cfg_profile()->jumbo_mtu[port_type])) + 2;
            headroom = 0;
        } else {
            reserved_min = buf_cfg->chunks_per_q[port_type];
            headroom = cells_to_chunks(tm_asic_profile()->port[port_type].headroom_cells);
        }
    }

    if (headroom) {
        SDK_ASSERT(chunks_to_cells(reserved_min) >=
                   bytes_to_cells(tm_cfg_profile()->jumbo_mtu[port_type]));
        xon_threshold = chunks_to_cells(reserved_min) -
            bytes_to_cells(tm_cfg_profile()->jumbo_mtu[port_type]);
    }

    if (port_type == TM_PORT_TYPE_DMA) {
        // Based on asic config
        xon_threshold = 13;
    }

    pbc_csr.port_1.cfg_account_pg_0.read();
    /* Update the PG parameters */
    pbc_csr.port_1.cfg_account_pg_0.reserved_min(reserved_min);
    pbc_csr.port_1.cfg_account_pg_0.headroom(headroom);
    pbc_csr.port_1.cfg_account_pg_0.xon_threshold(xon_threshold);
    pbc_csr.port_1.cfg_account_pg_0.low_limit(0);
    pbc_csr.port_1.cfg_account_pg_0.alpha(0);

    if (tm_sw_init_enabled()) {
        data <<"pbc_csr.port_1.cfg_account_pg_0.all: 0x" << pbc_csr.port_1.cfg_account_pg_0.all() << endl;
data <<"pbc_csr.port_1.cfg_account_pg_0.alpha: 0x" << pbc_csr.port_1.cfg_account_pg_0.alpha() << endl;
data <<"pbc_csr.port_1.cfg_account_pg_0.xon_threshold: 0x" << pbc_csr.port_1.cfg_account_pg_0.xon_threshold() << endl;
data <<"pbc_csr.port_1.cfg_account_pg_0.headroom: 0x" << pbc_csr.port_1.cfg_account_pg_0.headroom() << endl;
data <<"pbc_csr.port_1.cfg_account_pg_0.reserved_min: 0x" << pbc_csr.port_1.cfg_account_pg_0.reserved_min() << endl;
data <<"pbc_csr.port_1.cfg_account_pg_0.low_limit: 0x" << pbc_csr.port_1.cfg_account_pg_0.low_limit() << endl;

        pbc_csr.port_1.cfg_account_pg_0.write();
    }
    reserved_min = 0;
    headroom = 0;
    xon_threshold = 0;
    if (!iq_disabled(ELBA_TM_PORT_UPLINK_1, 1)) {
        if (tm_asic_profile()->port[port_type].uses_credits &&
            tm_asic_profile()->port[port_type].recirc_q != 1) {
            reserved_min = buf_cfg->chunks_per_q[port_type];

            if (tm_is_high_perf_q(1)) {
                reserved_min *= 2;
            }
            reserved_min = reserved_min +
                cells_to_chunks(bytes_to_cells(tm_cfg_profile()->jumbo_mtu[port_type])) + 2;
            headroom = 0;
        } else {
            reserved_min = buf_cfg->chunks_per_q[port_type];
            headroom = cells_to_chunks(tm_asic_profile()->port[port_type].headroom_cells);
        }
    }

    if (headroom) {
        SDK_ASSERT(chunks_to_cells(reserved_min) >=
                   bytes_to_cells(tm_cfg_profile()->jumbo_mtu[port_type]));
        xon_threshold = chunks_to_cells(reserved_min) -
            bytes_to_cells(tm_cfg_profile()->jumbo_mtu[port_type]);
    }

    if (port_type == TM_PORT_TYPE_DMA) {
        // Based on asic config
        xon_threshold = 13;
    }

    pbc_csr.port_1.cfg_account_pg_1.read();
    /* Update the PG parameters */
    pbc_csr.port_1.cfg_account_pg_1.reserved_min(reserved_min);
    pbc_csr.port_1.cfg_account_pg_1.headroom(headroom);
    pbc_csr.port_1.cfg_account_pg_1.xon_threshold(xon_threshold);
    pbc_csr.port_1.cfg_account_pg_1.low_limit(0);
    pbc_csr.port_1.cfg_account_pg_1.alpha(0);

    if (tm_sw_init_enabled()) {
        data <<"pbc_csr.port_1.cfg_account_pg_1.all: 0x" << pbc_csr.port_1.cfg_account_pg_1.all() << endl;
data <<"pbc_csr.port_1.cfg_account_pg_1.alpha: 0x" << pbc_csr.port_1.cfg_account_pg_1.alpha() << endl;
data <<"pbc_csr.port_1.cfg_account_pg_1.xon_threshold: 0x" << pbc_csr.port_1.cfg_account_pg_1.xon_threshold() << endl;
data <<"pbc_csr.port_1.cfg_account_pg_1.headroom: 0x" << pbc_csr.port_1.cfg_account_pg_1.headroom() << endl;
data <<"pbc_csr.port_1.cfg_account_pg_1.reserved_min: 0x" << pbc_csr.port_1.cfg_account_pg_1.reserved_min() << endl;
data <<"pbc_csr.port_1.cfg_account_pg_1.low_limit: 0x" << pbc_csr.port_1.cfg_account_pg_1.low_limit() << endl;

        pbc_csr.port_1.cfg_account_pg_1.write();
    }
    reserved_min = 0;
    headroom = 0;
    xon_threshold = 0;
    if (!iq_disabled(ELBA_TM_PORT_UPLINK_1, 2)) {
        if (tm_asic_profile()->port[port_type].uses_credits &&
            tm_asic_profile()->port[port_type].recirc_q != 2) {
            reserved_min = buf_cfg->chunks_per_q[port_type];

            if (tm_is_high_perf_q(2)) {
                reserved_min *= 2;
            }
            reserved_min = reserved_min +
                cells_to_chunks(bytes_to_cells(tm_cfg_profile()->jumbo_mtu[port_type])) + 2;
            headroom = 0;
        } else {
            reserved_min = buf_cfg->chunks_per_q[port_type];
            headroom = cells_to_chunks(tm_asic_profile()->port[port_type].headroom_cells);
        }
    }

    if (headroom) {
        SDK_ASSERT(chunks_to_cells(reserved_min) >=
                   bytes_to_cells(tm_cfg_profile()->jumbo_mtu[port_type]));
        xon_threshold = chunks_to_cells(reserved_min) -
            bytes_to_cells(tm_cfg_profile()->jumbo_mtu[port_type]);
    }

    if (port_type == TM_PORT_TYPE_DMA) {
        // Based on asic config
        xon_threshold = 13;
    }

    pbc_csr.port_1.cfg_account_pg_2.read();
    /* Update the PG parameters */
    pbc_csr.port_1.cfg_account_pg_2.reserved_min(reserved_min);
    pbc_csr.port_1.cfg_account_pg_2.headroom(headroom);
    pbc_csr.port_1.cfg_account_pg_2.xon_threshold(xon_threshold);
    pbc_csr.port_1.cfg_account_pg_2.low_limit(0);
    pbc_csr.port_1.cfg_account_pg_2.alpha(0);

    if (tm_sw_init_enabled()) {
        data <<"pbc_csr.port_1.cfg_account_pg_2.all: 0x" << pbc_csr.port_1.cfg_account_pg_2.all() << endl;
data <<"pbc_csr.port_1.cfg_account_pg_2.alpha: 0x" << pbc_csr.port_1.cfg_account_pg_2.alpha() << endl;
data <<"pbc_csr.port_1.cfg_account_pg_2.xon_threshold: 0x" << pbc_csr.port_1.cfg_account_pg_2.xon_threshold() << endl;
data <<"pbc_csr.port_1.cfg_account_pg_2.headroom: 0x" << pbc_csr.port_1.cfg_account_pg_2.headroom() << endl;
data <<"pbc_csr.port_1.cfg_account_pg_2.reserved_min: 0x" << pbc_csr.port_1.cfg_account_pg_2.reserved_min() << endl;
data <<"pbc_csr.port_1.cfg_account_pg_2.low_limit: 0x" << pbc_csr.port_1.cfg_account_pg_2.low_limit() << endl;

        pbc_csr.port_1.cfg_account_pg_2.write();
    }
    reserved_min = 0;
    headroom = 0;
    xon_threshold = 0;
    if (!iq_disabled(ELBA_TM_PORT_UPLINK_1, 3)) {
        if (tm_asic_profile()->port[port_type].uses_credits &&
            tm_asic_profile()->port[port_type].recirc_q != 3) {
            reserved_min = buf_cfg->chunks_per_q[port_type];

            if (tm_is_high_perf_q(3)) {
                reserved_min *= 2;
            }
            reserved_min = reserved_min +
                cells_to_chunks(bytes_to_cells(tm_cfg_profile()->jumbo_mtu[port_type])) + 2;
            headroom = 0;
        } else {
            reserved_min = buf_cfg->chunks_per_q[port_type];
            headroom = cells_to_chunks(tm_asic_profile()->port[port_type].headroom_cells);
        }
    }

    if (headroom) {
        SDK_ASSERT(chunks_to_cells(reserved_min) >=
                   bytes_to_cells(tm_cfg_profile()->jumbo_mtu[port_type]));
        xon_threshold = chunks_to_cells(reserved_min) -
            bytes_to_cells(tm_cfg_profile()->jumbo_mtu[port_type]);
    }

    if (port_type == TM_PORT_TYPE_DMA) {
        // Based on asic config
        xon_threshold = 13;
    }

    pbc_csr.port_1.cfg_account_pg_3.read();
    /* Update the PG parameters */
    pbc_csr.port_1.cfg_account_pg_3.reserved_min(reserved_min);
    pbc_csr.port_1.cfg_account_pg_3.headroom(headroom);
    pbc_csr.port_1.cfg_account_pg_3.xon_threshold(xon_threshold);
    pbc_csr.port_1.cfg_account_pg_3.low_limit(0);
    pbc_csr.port_1.cfg_account_pg_3.alpha(0);

    if (tm_sw_init_enabled()) {
        data <<"pbc_csr.port_1.cfg_account_pg_3.all: 0x" << pbc_csr.port_1.cfg_account_pg_3.all() << endl;
data <<"pbc_csr.port_1.cfg_account_pg_3.alpha: 0x" << pbc_csr.port_1.cfg_account_pg_3.alpha() << endl;
data <<"pbc_csr.port_1.cfg_account_pg_3.xon_threshold: 0x" << pbc_csr.port_1.cfg_account_pg_3.xon_threshold() << endl;
data <<"pbc_csr.port_1.cfg_account_pg_3.headroom: 0x" << pbc_csr.port_1.cfg_account_pg_3.headroom() << endl;
data <<"pbc_csr.port_1.cfg_account_pg_3.reserved_min: 0x" << pbc_csr.port_1.cfg_account_pg_3.reserved_min() << endl;
data <<"pbc_csr.port_1.cfg_account_pg_3.low_limit: 0x" << pbc_csr.port_1.cfg_account_pg_3.low_limit() << endl;

        pbc_csr.port_1.cfg_account_pg_3.write();
    }
    reserved_min = 0;
    headroom = 0;
    xon_threshold = 0;
    if (!iq_disabled(ELBA_TM_PORT_UPLINK_1, 4)) {
        if (tm_asic_profile()->port[port_type].uses_credits &&
            tm_asic_profile()->port[port_type].recirc_q != 4) {
            reserved_min = buf_cfg->chunks_per_q[port_type];

            if (tm_is_high_perf_q(4)) {
                reserved_min *= 2;
            }
            reserved_min = reserved_min +
                cells_to_chunks(bytes_to_cells(tm_cfg_profile()->jumbo_mtu[port_type])) + 2;
            headroom = 0;
        } else {
            reserved_min = buf_cfg->chunks_per_q[port_type];
            headroom = cells_to_chunks(tm_asic_profile()->port[port_type].headroom_cells);
        }
    }

    if (headroom) {
        SDK_ASSERT(chunks_to_cells(reserved_min) >=
                   bytes_to_cells(tm_cfg_profile()->jumbo_mtu[port_type]));
        xon_threshold = chunks_to_cells(reserved_min) -
            bytes_to_cells(tm_cfg_profile()->jumbo_mtu[port_type]);
    }

    if (port_type == TM_PORT_TYPE_DMA) {
        // Based on asic config
        xon_threshold = 13;
    }

    pbc_csr.port_1.cfg_account_pg_4.read();
    /* Update the PG parameters */
    pbc_csr.port_1.cfg_account_pg_4.reserved_min(reserved_min);
    pbc_csr.port_1.cfg_account_pg_4.headroom(headroom);
    pbc_csr.port_1.cfg_account_pg_4.xon_threshold(xon_threshold);
    pbc_csr.port_1.cfg_account_pg_4.low_limit(0);
    pbc_csr.port_1.cfg_account_pg_4.alpha(0);

    if (tm_sw_init_enabled()) {
        data <<"pbc_csr.port_1.cfg_account_pg_4.all: 0x" << pbc_csr.port_1.cfg_account_pg_4.all() << endl;
data <<"pbc_csr.port_1.cfg_account_pg_4.alpha: 0x" << pbc_csr.port_1.cfg_account_pg_4.alpha() << endl;
data <<"pbc_csr.port_1.cfg_account_pg_4.xon_threshold: 0x" << pbc_csr.port_1.cfg_account_pg_4.xon_threshold() << endl;
data <<"pbc_csr.port_1.cfg_account_pg_4.headroom: 0x" << pbc_csr.port_1.cfg_account_pg_4.headroom() << endl;
data <<"pbc_csr.port_1.cfg_account_pg_4.reserved_min: 0x" << pbc_csr.port_1.cfg_account_pg_4.reserved_min() << endl;
data <<"pbc_csr.port_1.cfg_account_pg_4.low_limit: 0x" << pbc_csr.port_1.cfg_account_pg_4.low_limit() << endl;

        pbc_csr.port_1.cfg_account_pg_4.write();
    }
    reserved_min = 0;
    headroom = 0;
    xon_threshold = 0;
    if (!iq_disabled(ELBA_TM_PORT_UPLINK_1, 5)) {
        if (tm_asic_profile()->port[port_type].uses_credits &&
            tm_asic_profile()->port[port_type].recirc_q != 5) {
            reserved_min = buf_cfg->chunks_per_q[port_type];

            if (tm_is_high_perf_q(5)) {
                reserved_min *= 2;
            }
            reserved_min = reserved_min +
                cells_to_chunks(bytes_to_cells(tm_cfg_profile()->jumbo_mtu[port_type])) + 2;
            headroom = 0;
        } else {
            reserved_min = buf_cfg->chunks_per_q[port_type];
            headroom = cells_to_chunks(tm_asic_profile()->port[port_type].headroom_cells);
        }
    }

    if (headroom) {
        SDK_ASSERT(chunks_to_cells(reserved_min) >=
                   bytes_to_cells(tm_cfg_profile()->jumbo_mtu[port_type]));
        xon_threshold = chunks_to_cells(reserved_min) -
            bytes_to_cells(tm_cfg_profile()->jumbo_mtu[port_type]);
    }

    if (port_type == TM_PORT_TYPE_DMA) {
        // Based on asic config
        xon_threshold = 13;
    }

    pbc_csr.port_1.cfg_account_pg_5.read();
    /* Update the PG parameters */
    pbc_csr.port_1.cfg_account_pg_5.reserved_min(reserved_min);
    pbc_csr.port_1.cfg_account_pg_5.headroom(headroom);
    pbc_csr.port_1.cfg_account_pg_5.xon_threshold(xon_threshold);
    pbc_csr.port_1.cfg_account_pg_5.low_limit(0);
    pbc_csr.port_1.cfg_account_pg_5.alpha(0);

    if (tm_sw_init_enabled()) {
        data <<"pbc_csr.port_1.cfg_account_pg_5.all: 0x" << pbc_csr.port_1.cfg_account_pg_5.all() << endl;
data <<"pbc_csr.port_1.cfg_account_pg_5.alpha: 0x" << pbc_csr.port_1.cfg_account_pg_5.alpha() << endl;
data <<"pbc_csr.port_1.cfg_account_pg_5.xon_threshold: 0x" << pbc_csr.port_1.cfg_account_pg_5.xon_threshold() << endl;
data <<"pbc_csr.port_1.cfg_account_pg_5.headroom: 0x" << pbc_csr.port_1.cfg_account_pg_5.headroom() << endl;
data <<"pbc_csr.port_1.cfg_account_pg_5.reserved_min: 0x" << pbc_csr.port_1.cfg_account_pg_5.reserved_min() << endl;
data <<"pbc_csr.port_1.cfg_account_pg_5.low_limit: 0x" << pbc_csr.port_1.cfg_account_pg_5.low_limit() << endl;

        pbc_csr.port_1.cfg_account_pg_5.write();
    }
    reserved_min = 0;
    headroom = 0;
    xon_threshold = 0;
    if (!iq_disabled(ELBA_TM_PORT_UPLINK_1, 6)) {
        if (tm_asic_profile()->port[port_type].uses_credits &&
            tm_asic_profile()->port[port_type].recirc_q != 6) {
            reserved_min = buf_cfg->chunks_per_q[port_type];

            if (tm_is_high_perf_q(6)) {
                reserved_min *= 2;
            }
            reserved_min = reserved_min +
                cells_to_chunks(bytes_to_cells(tm_cfg_profile()->jumbo_mtu[port_type])) + 2;
            headroom = 0;
        } else {
            reserved_min = buf_cfg->chunks_per_q[port_type];
            headroom = cells_to_chunks(tm_asic_profile()->port[port_type].headroom_cells);
        }
    }

    if (headroom) {
        SDK_ASSERT(chunks_to_cells(reserved_min) >=
                   bytes_to_cells(tm_cfg_profile()->jumbo_mtu[port_type]));
        xon_threshold = chunks_to_cells(reserved_min) -
            bytes_to_cells(tm_cfg_profile()->jumbo_mtu[port_type]);
    }

    if (port_type == TM_PORT_TYPE_DMA) {
        // Based on asic config
        xon_threshold = 13;
    }

    pbc_csr.port_1.cfg_account_pg_6.read();
    /* Update the PG parameters */
    pbc_csr.port_1.cfg_account_pg_6.reserved_min(reserved_min);
    pbc_csr.port_1.cfg_account_pg_6.headroom(headroom);
    pbc_csr.port_1.cfg_account_pg_6.xon_threshold(xon_threshold);
    pbc_csr.port_1.cfg_account_pg_6.low_limit(0);
    pbc_csr.port_1.cfg_account_pg_6.alpha(0);

    if (tm_sw_init_enabled()) {
        data <<"pbc_csr.port_1.cfg_account_pg_6.all: 0x" << pbc_csr.port_1.cfg_account_pg_6.all() << endl;
data <<"pbc_csr.port_1.cfg_account_pg_6.alpha: 0x" << pbc_csr.port_1.cfg_account_pg_6.alpha() << endl;
data <<"pbc_csr.port_1.cfg_account_pg_6.xon_threshold: 0x" << pbc_csr.port_1.cfg_account_pg_6.xon_threshold() << endl;
data <<"pbc_csr.port_1.cfg_account_pg_6.headroom: 0x" << pbc_csr.port_1.cfg_account_pg_6.headroom() << endl;
data <<"pbc_csr.port_1.cfg_account_pg_6.reserved_min: 0x" << pbc_csr.port_1.cfg_account_pg_6.reserved_min() << endl;
data <<"pbc_csr.port_1.cfg_account_pg_6.low_limit: 0x" << pbc_csr.port_1.cfg_account_pg_6.low_limit() << endl;

        pbc_csr.port_1.cfg_account_pg_6.write();
    }
    reserved_min = 0;
    headroom = 0;
    xon_threshold = 0;
    if (!iq_disabled(ELBA_TM_PORT_UPLINK_1, 7)) {
        if (tm_asic_profile()->port[port_type].uses_credits &&
            tm_asic_profile()->port[port_type].recirc_q != 7) {
            reserved_min = buf_cfg->chunks_per_q[port_type];

            if (tm_is_high_perf_q(7)) {
                reserved_min *= 2;
            }
            reserved_min = reserved_min +
                cells_to_chunks(bytes_to_cells(tm_cfg_profile()->jumbo_mtu[port_type])) + 2;
            headroom = 0;
        } else {
            reserved_min = buf_cfg->chunks_per_q[port_type];
            headroom = cells_to_chunks(tm_asic_profile()->port[port_type].headroom_cells);
        }
    }

    if (headroom) {
        SDK_ASSERT(chunks_to_cells(reserved_min) >=
                   bytes_to_cells(tm_cfg_profile()->jumbo_mtu[port_type]));
        xon_threshold = chunks_to_cells(reserved_min) -
            bytes_to_cells(tm_cfg_profile()->jumbo_mtu[port_type]);
    }

    if (port_type == TM_PORT_TYPE_DMA) {
        // Based on asic config
        xon_threshold = 13;
    }

    pbc_csr.port_1.cfg_account_pg_7.read();
    /* Update the PG parameters */
    pbc_csr.port_1.cfg_account_pg_7.reserved_min(reserved_min);
    pbc_csr.port_1.cfg_account_pg_7.headroom(headroom);
    pbc_csr.port_1.cfg_account_pg_7.xon_threshold(xon_threshold);
    pbc_csr.port_1.cfg_account_pg_7.low_limit(0);
    pbc_csr.port_1.cfg_account_pg_7.alpha(0);

    if (tm_sw_init_enabled()) {
        data <<"pbc_csr.port_1.cfg_account_pg_7.all: 0x" << pbc_csr.port_1.cfg_account_pg_7.all() << endl;
data <<"pbc_csr.port_1.cfg_account_pg_7.alpha: 0x" << pbc_csr.port_1.cfg_account_pg_7.alpha() << endl;
data <<"pbc_csr.port_1.cfg_account_pg_7.xon_threshold: 0x" << pbc_csr.port_1.cfg_account_pg_7.xon_threshold() << endl;
data <<"pbc_csr.port_1.cfg_account_pg_7.headroom: 0x" << pbc_csr.port_1.cfg_account_pg_7.headroom() << endl;
data <<"pbc_csr.port_1.cfg_account_pg_7.reserved_min: 0x" << pbc_csr.port_1.cfg_account_pg_7.reserved_min() << endl;
data <<"pbc_csr.port_1.cfg_account_pg_7.low_limit: 0x" << pbc_csr.port_1.cfg_account_pg_7.low_limit() << endl;

        pbc_csr.port_1.cfg_account_pg_7.write();
    }
    port_type = elba_tm_get_port_type(ELBA_TM_PORT_UPLINK_2);
    reserved_min = 0;
    headroom = 0;
    xon_threshold = 0;
    if (!iq_disabled(ELBA_TM_PORT_UPLINK_2, 0)) {
        if (tm_asic_profile()->port[port_type].uses_credits &&
            tm_asic_profile()->port[port_type].recirc_q != 0) {
            reserved_min = buf_cfg->chunks_per_q[port_type];

            if (tm_is_high_perf_q(0)) {
                reserved_min *= 2;
            }
            reserved_min = reserved_min +
                cells_to_chunks(bytes_to_cells(tm_cfg_profile()->jumbo_mtu[port_type])) + 2;
            headroom = 0;
        } else {
            reserved_min = buf_cfg->chunks_per_q[port_type];
            headroom = cells_to_chunks(tm_asic_profile()->port[port_type].headroom_cells);
        }
    }

    if (headroom) {
        SDK_ASSERT(chunks_to_cells(reserved_min) >=
                   bytes_to_cells(tm_cfg_profile()->jumbo_mtu[port_type]));
        xon_threshold = chunks_to_cells(reserved_min) -
            bytes_to_cells(tm_cfg_profile()->jumbo_mtu[port_type]);
    }

    if (port_type == TM_PORT_TYPE_DMA) {
        // Based on asic config
        xon_threshold = 13;
    }

    pbc_csr.port_2.cfg_account_pg_0.read();
    /* Update the PG parameters */
    pbc_csr.port_2.cfg_account_pg_0.reserved_min(reserved_min);
    pbc_csr.port_2.cfg_account_pg_0.headroom(headroom);
    pbc_csr.port_2.cfg_account_pg_0.xon_threshold(xon_threshold);
    pbc_csr.port_2.cfg_account_pg_0.low_limit(0);
    pbc_csr.port_2.cfg_account_pg_0.alpha(0);

    if (tm_sw_init_enabled()) {
        data <<"pbc_csr.port_2.cfg_account_pg_0.all: 0x" << pbc_csr.port_2.cfg_account_pg_0.all() << endl;
data <<"pbc_csr.port_2.cfg_account_pg_0.alpha: 0x" << pbc_csr.port_2.cfg_account_pg_0.alpha() << endl;
data <<"pbc_csr.port_2.cfg_account_pg_0.xon_threshold: 0x" << pbc_csr.port_2.cfg_account_pg_0.xon_threshold() << endl;
data <<"pbc_csr.port_2.cfg_account_pg_0.headroom: 0x" << pbc_csr.port_2.cfg_account_pg_0.headroom() << endl;
data <<"pbc_csr.port_2.cfg_account_pg_0.reserved_min: 0x" << pbc_csr.port_2.cfg_account_pg_0.reserved_min() << endl;
data <<"pbc_csr.port_2.cfg_account_pg_0.low_limit: 0x" << pbc_csr.port_2.cfg_account_pg_0.low_limit() << endl;

        pbc_csr.port_2.cfg_account_pg_0.write();
    }
    reserved_min = 0;
    headroom = 0;
    xon_threshold = 0;
    if (!iq_disabled(ELBA_TM_PORT_UPLINK_2, 1)) {
        if (tm_asic_profile()->port[port_type].uses_credits &&
            tm_asic_profile()->port[port_type].recirc_q != 1) {
            reserved_min = buf_cfg->chunks_per_q[port_type];

            if (tm_is_high_perf_q(1)) {
                reserved_min *= 2;
            }
            reserved_min = reserved_min +
                cells_to_chunks(bytes_to_cells(tm_cfg_profile()->jumbo_mtu[port_type])) + 2;
            headroom = 0;
        } else {
            reserved_min = buf_cfg->chunks_per_q[port_type];
            headroom = cells_to_chunks(tm_asic_profile()->port[port_type].headroom_cells);
        }
    }

    if (headroom) {
        SDK_ASSERT(chunks_to_cells(reserved_min) >=
                   bytes_to_cells(tm_cfg_profile()->jumbo_mtu[port_type]));
        xon_threshold = chunks_to_cells(reserved_min) -
            bytes_to_cells(tm_cfg_profile()->jumbo_mtu[port_type]);
    }

    if (port_type == TM_PORT_TYPE_DMA) {
        // Based on asic config
        xon_threshold = 13;
    }

    pbc_csr.port_2.cfg_account_pg_1.read();
    /* Update the PG parameters */
    pbc_csr.port_2.cfg_account_pg_1.reserved_min(reserved_min);
    pbc_csr.port_2.cfg_account_pg_1.headroom(headroom);
    pbc_csr.port_2.cfg_account_pg_1.xon_threshold(xon_threshold);
    pbc_csr.port_2.cfg_account_pg_1.low_limit(0);
    pbc_csr.port_2.cfg_account_pg_1.alpha(0);

    if (tm_sw_init_enabled()) {
        data <<"pbc_csr.port_2.cfg_account_pg_1.all: 0x" << pbc_csr.port_2.cfg_account_pg_1.all() << endl;
data <<"pbc_csr.port_2.cfg_account_pg_1.alpha: 0x" << pbc_csr.port_2.cfg_account_pg_1.alpha() << endl;
data <<"pbc_csr.port_2.cfg_account_pg_1.xon_threshold: 0x" << pbc_csr.port_2.cfg_account_pg_1.xon_threshold() << endl;
data <<"pbc_csr.port_2.cfg_account_pg_1.headroom: 0x" << pbc_csr.port_2.cfg_account_pg_1.headroom() << endl;
data <<"pbc_csr.port_2.cfg_account_pg_1.reserved_min: 0x" << pbc_csr.port_2.cfg_account_pg_1.reserved_min() << endl;
data <<"pbc_csr.port_2.cfg_account_pg_1.low_limit: 0x" << pbc_csr.port_2.cfg_account_pg_1.low_limit() << endl;

        pbc_csr.port_2.cfg_account_pg_1.write();
    }
    reserved_min = 0;
    headroom = 0;
    xon_threshold = 0;
    if (!iq_disabled(ELBA_TM_PORT_UPLINK_2, 2)) {
        if (tm_asic_profile()->port[port_type].uses_credits &&
            tm_asic_profile()->port[port_type].recirc_q != 2) {
            reserved_min = buf_cfg->chunks_per_q[port_type];

            if (tm_is_high_perf_q(2)) {
                reserved_min *= 2;
            }
            reserved_min = reserved_min +
                cells_to_chunks(bytes_to_cells(tm_cfg_profile()->jumbo_mtu[port_type])) + 2;
            headroom = 0;
        } else {
            reserved_min = buf_cfg->chunks_per_q[port_type];
            headroom = cells_to_chunks(tm_asic_profile()->port[port_type].headroom_cells);
        }
    }

    if (headroom) {
        SDK_ASSERT(chunks_to_cells(reserved_min) >=
                   bytes_to_cells(tm_cfg_profile()->jumbo_mtu[port_type]));
        xon_threshold = chunks_to_cells(reserved_min) -
            bytes_to_cells(tm_cfg_profile()->jumbo_mtu[port_type]);
    }

    if (port_type == TM_PORT_TYPE_DMA) {
        // Based on asic config
        xon_threshold = 13;
    }

    pbc_csr.port_2.cfg_account_pg_2.read();
    /* Update the PG parameters */
    pbc_csr.port_2.cfg_account_pg_2.reserved_min(reserved_min);
    pbc_csr.port_2.cfg_account_pg_2.headroom(headroom);
    pbc_csr.port_2.cfg_account_pg_2.xon_threshold(xon_threshold);
    pbc_csr.port_2.cfg_account_pg_2.low_limit(0);
    pbc_csr.port_2.cfg_account_pg_2.alpha(0);

    if (tm_sw_init_enabled()) {
        data <<"pbc_csr.port_2.cfg_account_pg_2.all: 0x" << pbc_csr.port_2.cfg_account_pg_2.all() << endl;
data <<"pbc_csr.port_2.cfg_account_pg_2.alpha: 0x" << pbc_csr.port_2.cfg_account_pg_2.alpha() << endl;
data <<"pbc_csr.port_2.cfg_account_pg_2.xon_threshold: 0x" << pbc_csr.port_2.cfg_account_pg_2.xon_threshold() << endl;
data <<"pbc_csr.port_2.cfg_account_pg_2.headroom: 0x" << pbc_csr.port_2.cfg_account_pg_2.headroom() << endl;
data <<"pbc_csr.port_2.cfg_account_pg_2.reserved_min: 0x" << pbc_csr.port_2.cfg_account_pg_2.reserved_min() << endl;
data <<"pbc_csr.port_2.cfg_account_pg_2.low_limit: 0x" << pbc_csr.port_2.cfg_account_pg_2.low_limit() << endl;

        pbc_csr.port_2.cfg_account_pg_2.write();
    }
    reserved_min = 0;
    headroom = 0;
    xon_threshold = 0;
    if (!iq_disabled(ELBA_TM_PORT_UPLINK_2, 3)) {
        if (tm_asic_profile()->port[port_type].uses_credits &&
            tm_asic_profile()->port[port_type].recirc_q != 3) {
            reserved_min = buf_cfg->chunks_per_q[port_type];

            if (tm_is_high_perf_q(3)) {
                reserved_min *= 2;
            }
            reserved_min = reserved_min +
                cells_to_chunks(bytes_to_cells(tm_cfg_profile()->jumbo_mtu[port_type])) + 2;
            headroom = 0;
        } else {
            reserved_min = buf_cfg->chunks_per_q[port_type];
            headroom = cells_to_chunks(tm_asic_profile()->port[port_type].headroom_cells);
        }
    }

    if (headroom) {
        SDK_ASSERT(chunks_to_cells(reserved_min) >=
                   bytes_to_cells(tm_cfg_profile()->jumbo_mtu[port_type]));
        xon_threshold = chunks_to_cells(reserved_min) -
            bytes_to_cells(tm_cfg_profile()->jumbo_mtu[port_type]);
    }

    if (port_type == TM_PORT_TYPE_DMA) {
        // Based on asic config
        xon_threshold = 13;
    }

    pbc_csr.port_2.cfg_account_pg_3.read();
    /* Update the PG parameters */
    pbc_csr.port_2.cfg_account_pg_3.reserved_min(reserved_min);
    pbc_csr.port_2.cfg_account_pg_3.headroom(headroom);
    pbc_csr.port_2.cfg_account_pg_3.xon_threshold(xon_threshold);
    pbc_csr.port_2.cfg_account_pg_3.low_limit(0);
    pbc_csr.port_2.cfg_account_pg_3.alpha(0);

    if (tm_sw_init_enabled()) {
        data <<"pbc_csr.port_2.cfg_account_pg_3.all: 0x" << pbc_csr.port_2.cfg_account_pg_3.all() << endl;
data <<"pbc_csr.port_2.cfg_account_pg_3.alpha: 0x" << pbc_csr.port_2.cfg_account_pg_3.alpha() << endl;
data <<"pbc_csr.port_2.cfg_account_pg_3.xon_threshold: 0x" << pbc_csr.port_2.cfg_account_pg_3.xon_threshold() << endl;
data <<"pbc_csr.port_2.cfg_account_pg_3.headroom: 0x" << pbc_csr.port_2.cfg_account_pg_3.headroom() << endl;
data <<"pbc_csr.port_2.cfg_account_pg_3.reserved_min: 0x" << pbc_csr.port_2.cfg_account_pg_3.reserved_min() << endl;
data <<"pbc_csr.port_2.cfg_account_pg_3.low_limit: 0x" << pbc_csr.port_2.cfg_account_pg_3.low_limit() << endl;

        pbc_csr.port_2.cfg_account_pg_3.write();
    }
    reserved_min = 0;
    headroom = 0;
    xon_threshold = 0;
    if (!iq_disabled(ELBA_TM_PORT_UPLINK_2, 4)) {
        if (tm_asic_profile()->port[port_type].uses_credits &&
            tm_asic_profile()->port[port_type].recirc_q != 4) {
            reserved_min = buf_cfg->chunks_per_q[port_type];

            if (tm_is_high_perf_q(4)) {
                reserved_min *= 2;
            }
            reserved_min = reserved_min +
                cells_to_chunks(bytes_to_cells(tm_cfg_profile()->jumbo_mtu[port_type])) + 2;
            headroom = 0;
        } else {
            reserved_min = buf_cfg->chunks_per_q[port_type];
            headroom = cells_to_chunks(tm_asic_profile()->port[port_type].headroom_cells);
        }
    }

    if (headroom) {
        SDK_ASSERT(chunks_to_cells(reserved_min) >=
                   bytes_to_cells(tm_cfg_profile()->jumbo_mtu[port_type]));
        xon_threshold = chunks_to_cells(reserved_min) -
            bytes_to_cells(tm_cfg_profile()->jumbo_mtu[port_type]);
    }

    if (port_type == TM_PORT_TYPE_DMA) {
        // Based on asic config
        xon_threshold = 13;
    }

    pbc_csr.port_2.cfg_account_pg_4.read();
    /* Update the PG parameters */
    pbc_csr.port_2.cfg_account_pg_4.reserved_min(reserved_min);
    pbc_csr.port_2.cfg_account_pg_4.headroom(headroom);
    pbc_csr.port_2.cfg_account_pg_4.xon_threshold(xon_threshold);
    pbc_csr.port_2.cfg_account_pg_4.low_limit(0);
    pbc_csr.port_2.cfg_account_pg_4.alpha(0);

    if (tm_sw_init_enabled()) {
        data <<"pbc_csr.port_2.cfg_account_pg_4.all: 0x" << pbc_csr.port_2.cfg_account_pg_4.all() << endl;
data <<"pbc_csr.port_2.cfg_account_pg_4.alpha: 0x" << pbc_csr.port_2.cfg_account_pg_4.alpha() << endl;
data <<"pbc_csr.port_2.cfg_account_pg_4.xon_threshold: 0x" << pbc_csr.port_2.cfg_account_pg_4.xon_threshold() << endl;
data <<"pbc_csr.port_2.cfg_account_pg_4.headroom: 0x" << pbc_csr.port_2.cfg_account_pg_4.headroom() << endl;
data <<"pbc_csr.port_2.cfg_account_pg_4.reserved_min: 0x" << pbc_csr.port_2.cfg_account_pg_4.reserved_min() << endl;
data <<"pbc_csr.port_2.cfg_account_pg_4.low_limit: 0x" << pbc_csr.port_2.cfg_account_pg_4.low_limit() << endl;

        pbc_csr.port_2.cfg_account_pg_4.write();
    }
    reserved_min = 0;
    headroom = 0;
    xon_threshold = 0;
    if (!iq_disabled(ELBA_TM_PORT_UPLINK_2, 5)) {
        if (tm_asic_profile()->port[port_type].uses_credits &&
            tm_asic_profile()->port[port_type].recirc_q != 5) {
            reserved_min = buf_cfg->chunks_per_q[port_type];

            if (tm_is_high_perf_q(5)) {
                reserved_min *= 2;
            }
            reserved_min = reserved_min +
                cells_to_chunks(bytes_to_cells(tm_cfg_profile()->jumbo_mtu[port_type])) + 2;
            headroom = 0;
        } else {
            reserved_min = buf_cfg->chunks_per_q[port_type];
            headroom = cells_to_chunks(tm_asic_profile()->port[port_type].headroom_cells);
        }
    }

    if (headroom) {
        SDK_ASSERT(chunks_to_cells(reserved_min) >=
                   bytes_to_cells(tm_cfg_profile()->jumbo_mtu[port_type]));
        xon_threshold = chunks_to_cells(reserved_min) -
            bytes_to_cells(tm_cfg_profile()->jumbo_mtu[port_type]);
    }

    if (port_type == TM_PORT_TYPE_DMA) {
        // Based on asic config
        xon_threshold = 13;
    }

    pbc_csr.port_2.cfg_account_pg_5.read();
    /* Update the PG parameters */
    pbc_csr.port_2.cfg_account_pg_5.reserved_min(reserved_min);
    pbc_csr.port_2.cfg_account_pg_5.headroom(headroom);
    pbc_csr.port_2.cfg_account_pg_5.xon_threshold(xon_threshold);
    pbc_csr.port_2.cfg_account_pg_5.low_limit(0);
    pbc_csr.port_2.cfg_account_pg_5.alpha(0);

    if (tm_sw_init_enabled()) {
        data <<"pbc_csr.port_2.cfg_account_pg_5.all: 0x" << pbc_csr.port_2.cfg_account_pg_5.all() << endl;
data <<"pbc_csr.port_2.cfg_account_pg_5.alpha: 0x" << pbc_csr.port_2.cfg_account_pg_5.alpha() << endl;
data <<"pbc_csr.port_2.cfg_account_pg_5.xon_threshold: 0x" << pbc_csr.port_2.cfg_account_pg_5.xon_threshold() << endl;
data <<"pbc_csr.port_2.cfg_account_pg_5.headroom: 0x" << pbc_csr.port_2.cfg_account_pg_5.headroom() << endl;
data <<"pbc_csr.port_2.cfg_account_pg_5.reserved_min: 0x" << pbc_csr.port_2.cfg_account_pg_5.reserved_min() << endl;
data <<"pbc_csr.port_2.cfg_account_pg_5.low_limit: 0x" << pbc_csr.port_2.cfg_account_pg_5.low_limit() << endl;

        pbc_csr.port_2.cfg_account_pg_5.write();
    }
    reserved_min = 0;
    headroom = 0;
    xon_threshold = 0;
    if (!iq_disabled(ELBA_TM_PORT_UPLINK_2, 6)) {
        if (tm_asic_profile()->port[port_type].uses_credits &&
            tm_asic_profile()->port[port_type].recirc_q != 6) {
            reserved_min = buf_cfg->chunks_per_q[port_type];

            if (tm_is_high_perf_q(6)) {
                reserved_min *= 2;
            }
            reserved_min = reserved_min +
                cells_to_chunks(bytes_to_cells(tm_cfg_profile()->jumbo_mtu[port_type])) + 2;
            headroom = 0;
        } else {
            reserved_min = buf_cfg->chunks_per_q[port_type];
            headroom = cells_to_chunks(tm_asic_profile()->port[port_type].headroom_cells);
        }
    }

    if (headroom) {
        SDK_ASSERT(chunks_to_cells(reserved_min) >=
                   bytes_to_cells(tm_cfg_profile()->jumbo_mtu[port_type]));
        xon_threshold = chunks_to_cells(reserved_min) -
            bytes_to_cells(tm_cfg_profile()->jumbo_mtu[port_type]);
    }

    if (port_type == TM_PORT_TYPE_DMA) {
        // Based on asic config
        xon_threshold = 13;
    }

    pbc_csr.port_2.cfg_account_pg_6.read();
    /* Update the PG parameters */
    pbc_csr.port_2.cfg_account_pg_6.reserved_min(reserved_min);
    pbc_csr.port_2.cfg_account_pg_6.headroom(headroom);
    pbc_csr.port_2.cfg_account_pg_6.xon_threshold(xon_threshold);
    pbc_csr.port_2.cfg_account_pg_6.low_limit(0);
    pbc_csr.port_2.cfg_account_pg_6.alpha(0);

    if (tm_sw_init_enabled()) {
        data <<"pbc_csr.port_2.cfg_account_pg_6.all: 0x" << pbc_csr.port_2.cfg_account_pg_6.all() << endl;
data <<"pbc_csr.port_2.cfg_account_pg_6.alpha: 0x" << pbc_csr.port_2.cfg_account_pg_6.alpha() << endl;
data <<"pbc_csr.port_2.cfg_account_pg_6.xon_threshold: 0x" << pbc_csr.port_2.cfg_account_pg_6.xon_threshold() << endl;
data <<"pbc_csr.port_2.cfg_account_pg_6.headroom: 0x" << pbc_csr.port_2.cfg_account_pg_6.headroom() << endl;
data <<"pbc_csr.port_2.cfg_account_pg_6.reserved_min: 0x" << pbc_csr.port_2.cfg_account_pg_6.reserved_min() << endl;
data <<"pbc_csr.port_2.cfg_account_pg_6.low_limit: 0x" << pbc_csr.port_2.cfg_account_pg_6.low_limit() << endl;

        pbc_csr.port_2.cfg_account_pg_6.write();
    }
    reserved_min = 0;
    headroom = 0;
    xon_threshold = 0;
    if (!iq_disabled(ELBA_TM_PORT_UPLINK_2, 7)) {
        if (tm_asic_profile()->port[port_type].uses_credits &&
            tm_asic_profile()->port[port_type].recirc_q != 7) {
            reserved_min = buf_cfg->chunks_per_q[port_type];

            if (tm_is_high_perf_q(7)) {
                reserved_min *= 2;
            }
            reserved_min = reserved_min +
                cells_to_chunks(bytes_to_cells(tm_cfg_profile()->jumbo_mtu[port_type])) + 2;
            headroom = 0;
        } else {
            reserved_min = buf_cfg->chunks_per_q[port_type];
            headroom = cells_to_chunks(tm_asic_profile()->port[port_type].headroom_cells);
        }
    }

    if (headroom) {
        SDK_ASSERT(chunks_to_cells(reserved_min) >=
                   bytes_to_cells(tm_cfg_profile()->jumbo_mtu[port_type]));
        xon_threshold = chunks_to_cells(reserved_min) -
            bytes_to_cells(tm_cfg_profile()->jumbo_mtu[port_type]);
    }

    if (port_type == TM_PORT_TYPE_DMA) {
        // Based on asic config
        xon_threshold = 13;
    }

    pbc_csr.port_2.cfg_account_pg_7.read();
    /* Update the PG parameters */
    pbc_csr.port_2.cfg_account_pg_7.reserved_min(reserved_min);
    pbc_csr.port_2.cfg_account_pg_7.headroom(headroom);
    pbc_csr.port_2.cfg_account_pg_7.xon_threshold(xon_threshold);
    pbc_csr.port_2.cfg_account_pg_7.low_limit(0);
    pbc_csr.port_2.cfg_account_pg_7.alpha(0);

    if (tm_sw_init_enabled()) {
        data <<"pbc_csr.port_2.cfg_account_pg_7.all: 0x" << pbc_csr.port_2.cfg_account_pg_7.all() << endl;
data <<"pbc_csr.port_2.cfg_account_pg_7.alpha: 0x" << pbc_csr.port_2.cfg_account_pg_7.alpha() << endl;
data <<"pbc_csr.port_2.cfg_account_pg_7.xon_threshold: 0x" << pbc_csr.port_2.cfg_account_pg_7.xon_threshold() << endl;
data <<"pbc_csr.port_2.cfg_account_pg_7.headroom: 0x" << pbc_csr.port_2.cfg_account_pg_7.headroom() << endl;
data <<"pbc_csr.port_2.cfg_account_pg_7.reserved_min: 0x" << pbc_csr.port_2.cfg_account_pg_7.reserved_min() << endl;
data <<"pbc_csr.port_2.cfg_account_pg_7.low_limit: 0x" << pbc_csr.port_2.cfg_account_pg_7.low_limit() << endl;

        pbc_csr.port_2.cfg_account_pg_7.write();
    }
    port_type = elba_tm_get_port_type(ELBA_TM_PORT_UPLINK_3);
    reserved_min = 0;
    headroom = 0;
    xon_threshold = 0;
    if (!iq_disabled(ELBA_TM_PORT_UPLINK_3, 0)) {
        if (tm_asic_profile()->port[port_type].uses_credits &&
            tm_asic_profile()->port[port_type].recirc_q != 0) {
            reserved_min = buf_cfg->chunks_per_q[port_type];

            if (tm_is_high_perf_q(0)) {
                reserved_min *= 2;
            }
            reserved_min = reserved_min +
                cells_to_chunks(bytes_to_cells(tm_cfg_profile()->jumbo_mtu[port_type])) + 2;
            headroom = 0;
        } else {
            reserved_min = buf_cfg->chunks_per_q[port_type];
            headroom = cells_to_chunks(tm_asic_profile()->port[port_type].headroom_cells);
        }
    }

    if (headroom) {
        SDK_ASSERT(chunks_to_cells(reserved_min) >=
                   bytes_to_cells(tm_cfg_profile()->jumbo_mtu[port_type]));
        xon_threshold = chunks_to_cells(reserved_min) -
            bytes_to_cells(tm_cfg_profile()->jumbo_mtu[port_type]);
    }

    if (port_type == TM_PORT_TYPE_DMA) {
        // Based on asic config
        xon_threshold = 13;
    }

    pbc_csr.port_3.cfg_account_pg_0.read();
    /* Update the PG parameters */
    pbc_csr.port_3.cfg_account_pg_0.reserved_min(reserved_min);
    pbc_csr.port_3.cfg_account_pg_0.headroom(headroom);
    pbc_csr.port_3.cfg_account_pg_0.xon_threshold(xon_threshold);
    pbc_csr.port_3.cfg_account_pg_0.low_limit(0);
    pbc_csr.port_3.cfg_account_pg_0.alpha(0);

    if (tm_sw_init_enabled()) {
        data <<"pbc_csr.port_3.cfg_account_pg_0.all: 0x" << pbc_csr.port_3.cfg_account_pg_0.all() << endl;
data <<"pbc_csr.port_3.cfg_account_pg_0.alpha: 0x" << pbc_csr.port_3.cfg_account_pg_0.alpha() << endl;
data <<"pbc_csr.port_3.cfg_account_pg_0.xon_threshold: 0x" << pbc_csr.port_3.cfg_account_pg_0.xon_threshold() << endl;
data <<"pbc_csr.port_3.cfg_account_pg_0.headroom: 0x" << pbc_csr.port_3.cfg_account_pg_0.headroom() << endl;
data <<"pbc_csr.port_3.cfg_account_pg_0.reserved_min: 0x" << pbc_csr.port_3.cfg_account_pg_0.reserved_min() << endl;
data <<"pbc_csr.port_3.cfg_account_pg_0.low_limit: 0x" << pbc_csr.port_3.cfg_account_pg_0.low_limit() << endl;

        pbc_csr.port_3.cfg_account_pg_0.write();
    }
    reserved_min = 0;
    headroom = 0;
    xon_threshold = 0;
    if (!iq_disabled(ELBA_TM_PORT_UPLINK_3, 1)) {
        if (tm_asic_profile()->port[port_type].uses_credits &&
            tm_asic_profile()->port[port_type].recirc_q != 1) {
            reserved_min = buf_cfg->chunks_per_q[port_type];

            if (tm_is_high_perf_q(1)) {
                reserved_min *= 2;
            }
            reserved_min = reserved_min +
                cells_to_chunks(bytes_to_cells(tm_cfg_profile()->jumbo_mtu[port_type])) + 2;
            headroom = 0;
        } else {
            reserved_min = buf_cfg->chunks_per_q[port_type];
            headroom = cells_to_chunks(tm_asic_profile()->port[port_type].headroom_cells);
        }
    }

    if (headroom) {
        SDK_ASSERT(chunks_to_cells(reserved_min) >=
                   bytes_to_cells(tm_cfg_profile()->jumbo_mtu[port_type]));
        xon_threshold = chunks_to_cells(reserved_min) -
            bytes_to_cells(tm_cfg_profile()->jumbo_mtu[port_type]);
    }

    if (port_type == TM_PORT_TYPE_DMA) {
        // Based on asic config
        xon_threshold = 13;
    }

    pbc_csr.port_3.cfg_account_pg_1.read();
    /* Update the PG parameters */
    pbc_csr.port_3.cfg_account_pg_1.reserved_min(reserved_min);
    pbc_csr.port_3.cfg_account_pg_1.headroom(headroom);
    pbc_csr.port_3.cfg_account_pg_1.xon_threshold(xon_threshold);
    pbc_csr.port_3.cfg_account_pg_1.low_limit(0);
    pbc_csr.port_3.cfg_account_pg_1.alpha(0);

    if (tm_sw_init_enabled()) {
        data <<"pbc_csr.port_3.cfg_account_pg_1.all: 0x" << pbc_csr.port_3.cfg_account_pg_1.all() << endl;
data <<"pbc_csr.port_3.cfg_account_pg_1.alpha: 0x" << pbc_csr.port_3.cfg_account_pg_1.alpha() << endl;
data <<"pbc_csr.port_3.cfg_account_pg_1.xon_threshold: 0x" << pbc_csr.port_3.cfg_account_pg_1.xon_threshold() << endl;
data <<"pbc_csr.port_3.cfg_account_pg_1.headroom: 0x" << pbc_csr.port_3.cfg_account_pg_1.headroom() << endl;
data <<"pbc_csr.port_3.cfg_account_pg_1.reserved_min: 0x" << pbc_csr.port_3.cfg_account_pg_1.reserved_min() << endl;
data <<"pbc_csr.port_3.cfg_account_pg_1.low_limit: 0x" << pbc_csr.port_3.cfg_account_pg_1.low_limit() << endl;

        pbc_csr.port_3.cfg_account_pg_1.write();
    }
    reserved_min = 0;
    headroom = 0;
    xon_threshold = 0;
    if (!iq_disabled(ELBA_TM_PORT_UPLINK_3, 2)) {
        if (tm_asic_profile()->port[port_type].uses_credits &&
            tm_asic_profile()->port[port_type].recirc_q != 2) {
            reserved_min = buf_cfg->chunks_per_q[port_type];

            if (tm_is_high_perf_q(2)) {
                reserved_min *= 2;
            }
            reserved_min = reserved_min +
                cells_to_chunks(bytes_to_cells(tm_cfg_profile()->jumbo_mtu[port_type])) + 2;
            headroom = 0;
        } else {
            reserved_min = buf_cfg->chunks_per_q[port_type];
            headroom = cells_to_chunks(tm_asic_profile()->port[port_type].headroom_cells);
        }
    }

    if (headroom) {
        SDK_ASSERT(chunks_to_cells(reserved_min) >=
                   bytes_to_cells(tm_cfg_profile()->jumbo_mtu[port_type]));
        xon_threshold = chunks_to_cells(reserved_min) -
            bytes_to_cells(tm_cfg_profile()->jumbo_mtu[port_type]);
    }

    if (port_type == TM_PORT_TYPE_DMA) {
        // Based on asic config
        xon_threshold = 13;
    }

    pbc_csr.port_3.cfg_account_pg_2.read();
    /* Update the PG parameters */
    pbc_csr.port_3.cfg_account_pg_2.reserved_min(reserved_min);
    pbc_csr.port_3.cfg_account_pg_2.headroom(headroom);
    pbc_csr.port_3.cfg_account_pg_2.xon_threshold(xon_threshold);
    pbc_csr.port_3.cfg_account_pg_2.low_limit(0);
    pbc_csr.port_3.cfg_account_pg_2.alpha(0);

    if (tm_sw_init_enabled()) {
        data <<"pbc_csr.port_3.cfg_account_pg_2.all: 0x" << pbc_csr.port_3.cfg_account_pg_2.all() << endl;
data <<"pbc_csr.port_3.cfg_account_pg_2.alpha: 0x" << pbc_csr.port_3.cfg_account_pg_2.alpha() << endl;
data <<"pbc_csr.port_3.cfg_account_pg_2.xon_threshold: 0x" << pbc_csr.port_3.cfg_account_pg_2.xon_threshold() << endl;
data <<"pbc_csr.port_3.cfg_account_pg_2.headroom: 0x" << pbc_csr.port_3.cfg_account_pg_2.headroom() << endl;
data <<"pbc_csr.port_3.cfg_account_pg_2.reserved_min: 0x" << pbc_csr.port_3.cfg_account_pg_2.reserved_min() << endl;
data <<"pbc_csr.port_3.cfg_account_pg_2.low_limit: 0x" << pbc_csr.port_3.cfg_account_pg_2.low_limit() << endl;

        pbc_csr.port_3.cfg_account_pg_2.write();
    }
    reserved_min = 0;
    headroom = 0;
    xon_threshold = 0;
    if (!iq_disabled(ELBA_TM_PORT_UPLINK_3, 3)) {
        if (tm_asic_profile()->port[port_type].uses_credits &&
            tm_asic_profile()->port[port_type].recirc_q != 3) {
            reserved_min = buf_cfg->chunks_per_q[port_type];

            if (tm_is_high_perf_q(3)) {
                reserved_min *= 2;
            }
            reserved_min = reserved_min +
                cells_to_chunks(bytes_to_cells(tm_cfg_profile()->jumbo_mtu[port_type])) + 2;
            headroom = 0;
        } else {
            reserved_min = buf_cfg->chunks_per_q[port_type];
            headroom = cells_to_chunks(tm_asic_profile()->port[port_type].headroom_cells);
        }
    }

    if (headroom) {
        SDK_ASSERT(chunks_to_cells(reserved_min) >=
                   bytes_to_cells(tm_cfg_profile()->jumbo_mtu[port_type]));
        xon_threshold = chunks_to_cells(reserved_min) -
            bytes_to_cells(tm_cfg_profile()->jumbo_mtu[port_type]);
    }

    if (port_type == TM_PORT_TYPE_DMA) {
        // Based on asic config
        xon_threshold = 13;
    }

    pbc_csr.port_3.cfg_account_pg_3.read();
    /* Update the PG parameters */
    pbc_csr.port_3.cfg_account_pg_3.reserved_min(reserved_min);
    pbc_csr.port_3.cfg_account_pg_3.headroom(headroom);
    pbc_csr.port_3.cfg_account_pg_3.xon_threshold(xon_threshold);
    pbc_csr.port_3.cfg_account_pg_3.low_limit(0);
    pbc_csr.port_3.cfg_account_pg_3.alpha(0);

    if (tm_sw_init_enabled()) {
        data <<"pbc_csr.port_3.cfg_account_pg_3.all: 0x" << pbc_csr.port_3.cfg_account_pg_3.all() << endl;
data <<"pbc_csr.port_3.cfg_account_pg_3.alpha: 0x" << pbc_csr.port_3.cfg_account_pg_3.alpha() << endl;
data <<"pbc_csr.port_3.cfg_account_pg_3.xon_threshold: 0x" << pbc_csr.port_3.cfg_account_pg_3.xon_threshold() << endl;
data <<"pbc_csr.port_3.cfg_account_pg_3.headroom: 0x" << pbc_csr.port_3.cfg_account_pg_3.headroom() << endl;
data <<"pbc_csr.port_3.cfg_account_pg_3.reserved_min: 0x" << pbc_csr.port_3.cfg_account_pg_3.reserved_min() << endl;
data <<"pbc_csr.port_3.cfg_account_pg_3.low_limit: 0x" << pbc_csr.port_3.cfg_account_pg_3.low_limit() << endl;

        pbc_csr.port_3.cfg_account_pg_3.write();
    }
    reserved_min = 0;
    headroom = 0;
    xon_threshold = 0;
    if (!iq_disabled(ELBA_TM_PORT_UPLINK_3, 4)) {
        if (tm_asic_profile()->port[port_type].uses_credits &&
            tm_asic_profile()->port[port_type].recirc_q != 4) {
            reserved_min = buf_cfg->chunks_per_q[port_type];

            if (tm_is_high_perf_q(4)) {
                reserved_min *= 2;
            }
            reserved_min = reserved_min +
                cells_to_chunks(bytes_to_cells(tm_cfg_profile()->jumbo_mtu[port_type])) + 2;
            headroom = 0;
        } else {
            reserved_min = buf_cfg->chunks_per_q[port_type];
            headroom = cells_to_chunks(tm_asic_profile()->port[port_type].headroom_cells);
        }
    }

    if (headroom) {
        SDK_ASSERT(chunks_to_cells(reserved_min) >=
                   bytes_to_cells(tm_cfg_profile()->jumbo_mtu[port_type]));
        xon_threshold = chunks_to_cells(reserved_min) -
            bytes_to_cells(tm_cfg_profile()->jumbo_mtu[port_type]);
    }

    if (port_type == TM_PORT_TYPE_DMA) {
        // Based on asic config
        xon_threshold = 13;
    }

    pbc_csr.port_3.cfg_account_pg_4.read();
    /* Update the PG parameters */
    pbc_csr.port_3.cfg_account_pg_4.reserved_min(reserved_min);
    pbc_csr.port_3.cfg_account_pg_4.headroom(headroom);
    pbc_csr.port_3.cfg_account_pg_4.xon_threshold(xon_threshold);
    pbc_csr.port_3.cfg_account_pg_4.low_limit(0);
    pbc_csr.port_3.cfg_account_pg_4.alpha(0);

    if (tm_sw_init_enabled()) {
        data <<"pbc_csr.port_3.cfg_account_pg_4.all: 0x" << pbc_csr.port_3.cfg_account_pg_4.all() << endl;
data <<"pbc_csr.port_3.cfg_account_pg_4.alpha: 0x" << pbc_csr.port_3.cfg_account_pg_4.alpha() << endl;
data <<"pbc_csr.port_3.cfg_account_pg_4.xon_threshold: 0x" << pbc_csr.port_3.cfg_account_pg_4.xon_threshold() << endl;
data <<"pbc_csr.port_3.cfg_account_pg_4.headroom: 0x" << pbc_csr.port_3.cfg_account_pg_4.headroom() << endl;
data <<"pbc_csr.port_3.cfg_account_pg_4.reserved_min: 0x" << pbc_csr.port_3.cfg_account_pg_4.reserved_min() << endl;
data <<"pbc_csr.port_3.cfg_account_pg_4.low_limit: 0x" << pbc_csr.port_3.cfg_account_pg_4.low_limit() << endl;

        pbc_csr.port_3.cfg_account_pg_4.write();
    }
    reserved_min = 0;
    headroom = 0;
    xon_threshold = 0;
    if (!iq_disabled(ELBA_TM_PORT_UPLINK_3, 5)) {
        if (tm_asic_profile()->port[port_type].uses_credits &&
            tm_asic_profile()->port[port_type].recirc_q != 5) {
            reserved_min = buf_cfg->chunks_per_q[port_type];

            if (tm_is_high_perf_q(5)) {
                reserved_min *= 2;
            }
            reserved_min = reserved_min +
                cells_to_chunks(bytes_to_cells(tm_cfg_profile()->jumbo_mtu[port_type])) + 2;
            headroom = 0;
        } else {
            reserved_min = buf_cfg->chunks_per_q[port_type];
            headroom = cells_to_chunks(tm_asic_profile()->port[port_type].headroom_cells);
        }
    }

    if (headroom) {
        SDK_ASSERT(chunks_to_cells(reserved_min) >=
                   bytes_to_cells(tm_cfg_profile()->jumbo_mtu[port_type]));
        xon_threshold = chunks_to_cells(reserved_min) -
            bytes_to_cells(tm_cfg_profile()->jumbo_mtu[port_type]);
    }

    if (port_type == TM_PORT_TYPE_DMA) {
        // Based on asic config
        xon_threshold = 13;
    }

    pbc_csr.port_3.cfg_account_pg_5.read();
    /* Update the PG parameters */
    pbc_csr.port_3.cfg_account_pg_5.reserved_min(reserved_min);
    pbc_csr.port_3.cfg_account_pg_5.headroom(headroom);
    pbc_csr.port_3.cfg_account_pg_5.xon_threshold(xon_threshold);
    pbc_csr.port_3.cfg_account_pg_5.low_limit(0);
    pbc_csr.port_3.cfg_account_pg_5.alpha(0);

    if (tm_sw_init_enabled()) {
        data <<"pbc_csr.port_3.cfg_account_pg_5.all: 0x" << pbc_csr.port_3.cfg_account_pg_5.all() << endl;
data <<"pbc_csr.port_3.cfg_account_pg_5.alpha: 0x" << pbc_csr.port_3.cfg_account_pg_5.alpha() << endl;
data <<"pbc_csr.port_3.cfg_account_pg_5.xon_threshold: 0x" << pbc_csr.port_3.cfg_account_pg_5.xon_threshold() << endl;
data <<"pbc_csr.port_3.cfg_account_pg_5.headroom: 0x" << pbc_csr.port_3.cfg_account_pg_5.headroom() << endl;
data <<"pbc_csr.port_3.cfg_account_pg_5.reserved_min: 0x" << pbc_csr.port_3.cfg_account_pg_5.reserved_min() << endl;
data <<"pbc_csr.port_3.cfg_account_pg_5.low_limit: 0x" << pbc_csr.port_3.cfg_account_pg_5.low_limit() << endl;

        pbc_csr.port_3.cfg_account_pg_5.write();
    }
    reserved_min = 0;
    headroom = 0;
    xon_threshold = 0;
    if (!iq_disabled(ELBA_TM_PORT_UPLINK_3, 6)) {
        if (tm_asic_profile()->port[port_type].uses_credits &&
            tm_asic_profile()->port[port_type].recirc_q != 6) {
            reserved_min = buf_cfg->chunks_per_q[port_type];

            if (tm_is_high_perf_q(6)) {
                reserved_min *= 2;
            }
            reserved_min = reserved_min +
                cells_to_chunks(bytes_to_cells(tm_cfg_profile()->jumbo_mtu[port_type])) + 2;
            headroom = 0;
        } else {
            reserved_min = buf_cfg->chunks_per_q[port_type];
            headroom = cells_to_chunks(tm_asic_profile()->port[port_type].headroom_cells);
        }
    }

    if (headroom) {
        SDK_ASSERT(chunks_to_cells(reserved_min) >=
                   bytes_to_cells(tm_cfg_profile()->jumbo_mtu[port_type]));
        xon_threshold = chunks_to_cells(reserved_min) -
            bytes_to_cells(tm_cfg_profile()->jumbo_mtu[port_type]);
    }

    if (port_type == TM_PORT_TYPE_DMA) {
        // Based on asic config
        xon_threshold = 13;
    }

    pbc_csr.port_3.cfg_account_pg_6.read();
    /* Update the PG parameters */
    pbc_csr.port_3.cfg_account_pg_6.reserved_min(reserved_min);
    pbc_csr.port_3.cfg_account_pg_6.headroom(headroom);
    pbc_csr.port_3.cfg_account_pg_6.xon_threshold(xon_threshold);
    pbc_csr.port_3.cfg_account_pg_6.low_limit(0);
    pbc_csr.port_3.cfg_account_pg_6.alpha(0);

    if (tm_sw_init_enabled()) {
        data <<"pbc_csr.port_3.cfg_account_pg_6.all: 0x" << pbc_csr.port_3.cfg_account_pg_6.all() << endl;
data <<"pbc_csr.port_3.cfg_account_pg_6.alpha: 0x" << pbc_csr.port_3.cfg_account_pg_6.alpha() << endl;
data <<"pbc_csr.port_3.cfg_account_pg_6.xon_threshold: 0x" << pbc_csr.port_3.cfg_account_pg_6.xon_threshold() << endl;
data <<"pbc_csr.port_3.cfg_account_pg_6.headroom: 0x" << pbc_csr.port_3.cfg_account_pg_6.headroom() << endl;
data <<"pbc_csr.port_3.cfg_account_pg_6.reserved_min: 0x" << pbc_csr.port_3.cfg_account_pg_6.reserved_min() << endl;
data <<"pbc_csr.port_3.cfg_account_pg_6.low_limit: 0x" << pbc_csr.port_3.cfg_account_pg_6.low_limit() << endl;

        pbc_csr.port_3.cfg_account_pg_6.write();
    }
    reserved_min = 0;
    headroom = 0;
    xon_threshold = 0;
    if (!iq_disabled(ELBA_TM_PORT_UPLINK_3, 7)) {
        if (tm_asic_profile()->port[port_type].uses_credits &&
            tm_asic_profile()->port[port_type].recirc_q != 7) {
            reserved_min = buf_cfg->chunks_per_q[port_type];

            if (tm_is_high_perf_q(7)) {
                reserved_min *= 2;
            }
            reserved_min = reserved_min +
                cells_to_chunks(bytes_to_cells(tm_cfg_profile()->jumbo_mtu[port_type])) + 2;
            headroom = 0;
        } else {
            reserved_min = buf_cfg->chunks_per_q[port_type];
            headroom = cells_to_chunks(tm_asic_profile()->port[port_type].headroom_cells);
        }
    }

    if (headroom) {
        SDK_ASSERT(chunks_to_cells(reserved_min) >=
                   bytes_to_cells(tm_cfg_profile()->jumbo_mtu[port_type]));
        xon_threshold = chunks_to_cells(reserved_min) -
            bytes_to_cells(tm_cfg_profile()->jumbo_mtu[port_type]);
    }

    if (port_type == TM_PORT_TYPE_DMA) {
        // Based on asic config
        xon_threshold = 13;
    }

    pbc_csr.port_3.cfg_account_pg_7.read();
    /* Update the PG parameters */
    pbc_csr.port_3.cfg_account_pg_7.reserved_min(reserved_min);
    pbc_csr.port_3.cfg_account_pg_7.headroom(headroom);
    pbc_csr.port_3.cfg_account_pg_7.xon_threshold(xon_threshold);
    pbc_csr.port_3.cfg_account_pg_7.low_limit(0);
    pbc_csr.port_3.cfg_account_pg_7.alpha(0);

    if (tm_sw_init_enabled()) {
        data <<"pbc_csr.port_3.cfg_account_pg_7.all: 0x" << pbc_csr.port_3.cfg_account_pg_7.all() << endl;
data <<"pbc_csr.port_3.cfg_account_pg_7.alpha: 0x" << pbc_csr.port_3.cfg_account_pg_7.alpha() << endl;
data <<"pbc_csr.port_3.cfg_account_pg_7.xon_threshold: 0x" << pbc_csr.port_3.cfg_account_pg_7.xon_threshold() << endl;
data <<"pbc_csr.port_3.cfg_account_pg_7.headroom: 0x" << pbc_csr.port_3.cfg_account_pg_7.headroom() << endl;
data <<"pbc_csr.port_3.cfg_account_pg_7.reserved_min: 0x" << pbc_csr.port_3.cfg_account_pg_7.reserved_min() << endl;
data <<"pbc_csr.port_3.cfg_account_pg_7.low_limit: 0x" << pbc_csr.port_3.cfg_account_pg_7.low_limit() << endl;

        pbc_csr.port_3.cfg_account_pg_7.write();
    }
    SDK_TRACE_DEBUG("%s", data.str().c_str());
    return ret;
}

static sdk_ret_t
elba_tm_program_p4_credits (elba_tm_buf_cfg_t *buf_cfg)
{
    sdk_ret_t ret = sdk::SDK_RET_OK;
    //elb_top_csr_t &elb0 = ELB_BLK_REG_MODEL_ACCESS(elb_top_csr_t, 0, 0);
    //elb_pbc_csr_t &pbc_csr = elb0.pb.pbc;
    //tm_port_type_e port_type;
    //uint32_t credit_enable;
    // uint32_t credits;
    // uint32_t qs_to_flush;
    // uint32_t recirc_q_val;
    cpp_int max_growth;
    elb_pbc_max_growth_map_t max_growth_decoder;
    max_growth_decoder.init();

    stringstream data;
    data << hex << endl;

    // Program the buffers
    SDK_TRACE_DEBUG("%s", data.str().c_str());

    return ret;
}

static sdk_ret_t
elba_tm_program_hbm_buffers (elba_tm_buf_cfg_t *buf_cfg)
{
    sdk_ret_t ret = sdk::SDK_RET_OK;
    elb_top_csr_t &elb0 = ELB_BLK_REG_MODEL_ACCESS(elb_top_csr_t, 0, 0);
    elb_pf_csr_t &pf_csr = elb0.pf.pf;
    uint32_t num_hbm_contexts_per_port;
    uint32_t context;
    uint32_t num_contexts;
    uint32_t base_offset;
    cpp_int port_payload_base_val;
    cpp_int port_payload_size_val;
    cpp_int port_payload_occupancy_val;
    cpp_int port_control_base_val;
    cpp_int port_control_size_val;

    cpp_int payload_base_val[NUM_TM_HBM_FIFO_TYPES];
    cpp_int payload_size_val[NUM_TM_HBM_FIFO_TYPES];
    cpp_int control_base_val[NUM_TM_HBM_FIFO_TYPES];
    cpp_int control_size_val[NUM_TM_HBM_FIFO_TYPES];
    cpp_int eth_xoff_val;
    cpp_int eth_xon_val;
    uint64_t payload_offset;
    uint64_t payload_size;
    uint64_t payload_occupancy;
    uint64_t control_offset;
    uint64_t control_size;
    uint64_t xoff_threshold;
    uint64_t xon_threshold;

    uint32_t fifo_type;
    
    tm_port_type_e port_type;

    // On active uplink ports and the DMA port, setup the HBM queues
    num_hbm_contexts_per_port = tm_cfg_profile()->num_qs[TM_PORT_TYPE_UPLINK];

    stringstream data;
    data << hex << endl;

    // ELBA_TM_PORT_UPLINK_0
    if (is_active_uplink_port(ELBA_TM_PORT_UPLINK_0)) {
        // If we are an active port, then setup the HBM fifo contexts
        num_contexts = num_hbm_contexts_per_port;
    } else {
        num_contexts = 0;
    }
    base_offset = 0 * num_contexts;

    port_type = elba_tm_get_port_type(ELBA_TM_PORT_UPLINK_0);
    fifo_type = TM_HBM_FIFO_TYPE_UPLINK;
    port_payload_base_val = 0;
    port_payload_size_val = 0;
    port_payload_occupancy_val = 0;
    port_control_base_val = 0;
    port_control_size_val = 0;

    for (unsigned q = 0; q < num_contexts; q++) {
        context = base_offset + q;

        elba_tm_buf_hbm_cfg_t *hbm_cfg = &buf_cfg->hbm_fifo[fifo_type][context];
        if (hbm_cfg->valid) {
            payload_offset = hbm_cfg->payload_offset;
            payload_size = hbm_cfg->payload_chunks;
            // payload occupancy is in units of 1024 bytes
            // program it to drop when 1 jumbo-mtu worth of bytes still free
            payload_occupancy =
                (hbm_chunks_to_bytes(payload_size) - tm_cfg_profile()->jumbo_mtu[port_type]) >> 10;

            if (fifo_type == TM_HBM_FIFO_TYPE_UPLINK) {
                // xoff_threshold is in units of 512 bytes
                // program it to drop when 1000*64 bytes worth of bytes still free
                xoff_threshold = 
                    (hbm_chunks_to_bytes(payload_size) - 
                     ELBA_TM_DEFAULT_XOFF_THRESHOLD_BYTES) >> 9;

                // xon_threshold is in units of 512 bytes
                // program it to drop when 2000*64 bytes worth of bytes still free
                xon_threshold = 
                    (hbm_chunks_to_bytes(payload_size) -
                     ELBA_TM_DEFAULT_XON_THRESHOLD_BYTES) >> 9;
            } else {
                xoff_threshold = 0;
                xon_threshold = 0;
            }

            control_offset = hbm_cfg->control_offset;
            control_size = hbm_cfg->control_chunks;
        } else {
            payload_offset = 0;
            payload_size = 0;
            payload_occupancy = 0;
            xoff_threshold = 0;
            xon_threshold = 0; 
            control_offset = 0;
            control_size = 0;
        }

        // Per port registers
        // 27 bits per hbm_q
        cpp_helper.set_slc(port_payload_base_val, payload_offset,
                            q * 27, ((q + 1) * 27) - 1);
        // 23 bits per q
        cpp_helper.set_slc(port_payload_size_val, payload_size ? payload_size - 1 : 0,
                            q * 23, ((q + 1) * 23) - 1);
        // 19 bits per q
        cpp_helper.set_slc(port_payload_occupancy_val, payload_occupancy ? payload_occupancy - 1 : 0,
                            q * 19, ((q + 1) * 19) - 1);
        // 27 bits per q
        cpp_helper.set_slc(port_control_base_val, control_offset,
                            q * 27, ((q + 1) * 27) - 1);
        // 23 bits per q
        cpp_helper.set_slc(port_control_size_val, control_size ? control_size - 1 : 0,
                            q * 23, ((q + 1) * 23) - 1);

        // Global registers

        // 27 bits per hbm_q
        cpp_helper.set_slc(payload_base_val[fifo_type], payload_offset,
                            context * 27, ((context + 1) * 27) - 1);
        // 23 bits per context
        cpp_helper.set_slc(payload_size_val[fifo_type], payload_size ? payload_size - 1 : 0,
                            context * 23, ((context + 1) * 23) - 1);
        // 27 bits per context
        cpp_helper.set_slc(control_base_val[fifo_type], control_offset,
                            context * 27, ((context + 1) * 27) - 1);
        // 23 bits per context
        cpp_helper.set_slc(control_size_val[fifo_type], control_size ? control_size - 1 : 0,
                            context * 23, ((context + 1) * 23) - 1);

        if (fifo_type == TM_HBM_FIFO_TYPE_UPLINK) {
            // 20 bits per context
            cpp_helper.set_slc(eth_xoff_val, xoff_threshold, 
                               context * 20, ((context + 1) * 20) - 1);
            // 20 bits per context
            cpp_helper.set_slc(eth_xon_val, xon_threshold, 
                               context * 20, ((context + 1) * 20) - 1);
        }
    }

    pf_csr.hbm_port_0.cfg_hbm_eth_payload.base(port_payload_base_val);
    pf_csr.hbm_port_0.cfg_hbm_eth_payload.mem_sz(port_payload_size_val);
    pf_csr.hbm_port_0.cfg_hbm_eth_payload_occupancy.threshold(port_payload_occupancy_val);

    pf_csr.hbm_port_0.cfg_hbm_eth_ctrl.base(port_control_base_val);
    pf_csr.hbm_port_0.cfg_hbm_eth_ctrl.mem_sz(port_control_size_val);


    pf_csr.hbm_port_0.cfg_hbm_context.read();
    pf_csr.hbm_port_0.cfg_hbm_context.enable((1ull<<num_contexts)-1);

    // enable no_drop by default in PB
    pf_csr.hbm_port_0.cfg_hbm_context.no_drop((1ull << num_contexts) - 1);

    // TODO use tm_sw_init_enabled below
    pf_csr.hbm_port_0.cfg_hbm_context.write();

    pf_csr.hbm_port_0.cfg_hbm_context.base(base_offset);
    if (tm_sw_init_enabled()) {

        data <<"pf_csr.hbm_port_0.cfg_hbm_eth_payload.all: 0x" << pf_csr.hbm_port_0.cfg_hbm_eth_payload.all() << endl;

        pf_csr.hbm_port_0.cfg_hbm_eth_payload.write();

        data <<"pf_csr.hbm_port_0.cfg_hbm_eth_payload_occupancy.all: 0x" << pf_csr.hbm_port_0.cfg_hbm_eth_payload_occupancy.all() << endl;

        pf_csr.hbm_port_0.cfg_hbm_eth_payload_occupancy.write();

        data <<"pf_csr.hbm_port_0.cfg_hbm_eth_ctrl.all: 0x" << pf_csr.hbm_port_0.cfg_hbm_eth_ctrl.all() << endl;

        pf_csr.hbm_port_0.cfg_hbm_eth_ctrl.write();

        data <<"pf_csr.hbm_port_0.cfg_hbm_context.all: 0x" << pf_csr.hbm_port_0.cfg_hbm_context.all() << endl;

        pf_csr.hbm_port_0.cfg_hbm_context.write();
    }
    // ELBA_TM_PORT_UPLINK_1
    if (is_active_uplink_port(ELBA_TM_PORT_UPLINK_1)) {
        // If we are an active port, then setup the HBM fifo contexts
        num_contexts = num_hbm_contexts_per_port;
    } else {
        num_contexts = 0;
    }
    base_offset = 1 * num_contexts;

    port_type = elba_tm_get_port_type(ELBA_TM_PORT_UPLINK_1);
    fifo_type = TM_HBM_FIFO_TYPE_UPLINK;
    port_payload_base_val = 0;
    port_payload_size_val = 0;
    port_payload_occupancy_val = 0;
    port_control_base_val = 0;
    port_control_size_val = 0;

    for (unsigned q = 0; q < num_contexts; q++) {
        context = base_offset + q;

        elba_tm_buf_hbm_cfg_t *hbm_cfg = &buf_cfg->hbm_fifo[fifo_type][context];
        if (hbm_cfg->valid) {
            payload_offset = hbm_cfg->payload_offset;
            payload_size = hbm_cfg->payload_chunks;
            // payload occupancy is in units of 1024 bytes
            // program it to drop when 1 jumbo-mtu worth of bytes still free
            payload_occupancy =
                (hbm_chunks_to_bytes(payload_size) - tm_cfg_profile()->jumbo_mtu[port_type]) >> 10;

            if (fifo_type == TM_HBM_FIFO_TYPE_UPLINK) {
                // xoff_threshold is in units of 512 bytes
                // program it to drop when 1000*64 bytes worth of bytes still free
                xoff_threshold = 
                    (hbm_chunks_to_bytes(payload_size) - 
                     ELBA_TM_DEFAULT_XOFF_THRESHOLD_BYTES) >> 9;

                // xon_threshold is in units of 512 bytes
                // program it to drop when 2000*64 bytes worth of bytes still free
                xon_threshold = 
                    (hbm_chunks_to_bytes(payload_size) -
                     ELBA_TM_DEFAULT_XON_THRESHOLD_BYTES) >> 9;
            } else {
                xoff_threshold = 0;
                xon_threshold = 0;
            }

            control_offset = hbm_cfg->control_offset;
            control_size = hbm_cfg->control_chunks;
        } else {
            payload_offset = 0;
            payload_size = 0;
            payload_occupancy = 0;
            xoff_threshold = 0;
            xon_threshold = 0; 
            control_offset = 0;
            control_size = 0;
        }

        // Per port registers
        // 27 bits per hbm_q
        cpp_helper.set_slc(port_payload_base_val, payload_offset,
                            q * 27, ((q + 1) * 27) - 1);
        // 23 bits per q
        cpp_helper.set_slc(port_payload_size_val, payload_size ? payload_size - 1 : 0,
                            q * 23, ((q + 1) * 23) - 1);
        // 19 bits per q
        cpp_helper.set_slc(port_payload_occupancy_val, payload_occupancy ? payload_occupancy - 1 : 0,
                            q * 19, ((q + 1) * 19) - 1);
        // 27 bits per q
        cpp_helper.set_slc(port_control_base_val, control_offset,
                            q * 27, ((q + 1) * 27) - 1);
        // 23 bits per q
        cpp_helper.set_slc(port_control_size_val, control_size ? control_size - 1 : 0,
                            q * 23, ((q + 1) * 23) - 1);

        // Global registers

        // 27 bits per hbm_q
        cpp_helper.set_slc(payload_base_val[fifo_type], payload_offset,
                            context * 27, ((context + 1) * 27) - 1);
        // 23 bits per context
        cpp_helper.set_slc(payload_size_val[fifo_type], payload_size ? payload_size - 1 : 0,
                            context * 23, ((context + 1) * 23) - 1);
        // 27 bits per context
        cpp_helper.set_slc(control_base_val[fifo_type], control_offset,
                            context * 27, ((context + 1) * 27) - 1);
        // 23 bits per context
        cpp_helper.set_slc(control_size_val[fifo_type], control_size ? control_size - 1 : 0,
                            context * 23, ((context + 1) * 23) - 1);

        if (fifo_type == TM_HBM_FIFO_TYPE_UPLINK) {
            // 20 bits per context
            cpp_helper.set_slc(eth_xoff_val, xoff_threshold, 
                               context * 20, ((context + 1) * 20) - 1);
            // 20 bits per context
            cpp_helper.set_slc(eth_xon_val, xon_threshold, 
                               context * 20, ((context + 1) * 20) - 1);
        }
    }

    pf_csr.hbm_port_1.cfg_hbm_eth_payload.base(port_payload_base_val);
    pf_csr.hbm_port_1.cfg_hbm_eth_payload.mem_sz(port_payload_size_val);
    pf_csr.hbm_port_1.cfg_hbm_eth_payload_occupancy.threshold(port_payload_occupancy_val);

    pf_csr.hbm_port_1.cfg_hbm_eth_ctrl.base(port_control_base_val);
    pf_csr.hbm_port_1.cfg_hbm_eth_ctrl.mem_sz(port_control_size_val);


    pf_csr.hbm_port_1.cfg_hbm_context.read();
    pf_csr.hbm_port_1.cfg_hbm_context.enable((1ull<<num_contexts)-1);

    // enable no_drop by default in PB
    pf_csr.hbm_port_1.cfg_hbm_context.no_drop((1ull << num_contexts) - 1);

    // TODO use tm_sw_init_enabled below
    pf_csr.hbm_port_1.cfg_hbm_context.write();

    pf_csr.hbm_port_1.cfg_hbm_context.base(base_offset);
    if (tm_sw_init_enabled()) {

        data <<"pf_csr.hbm_port_1.cfg_hbm_eth_payload.all: 0x" << pf_csr.hbm_port_1.cfg_hbm_eth_payload.all() << endl;

        pf_csr.hbm_port_1.cfg_hbm_eth_payload.write();

        data <<"pf_csr.hbm_port_1.cfg_hbm_eth_payload_occupancy.all: 0x" << pf_csr.hbm_port_1.cfg_hbm_eth_payload_occupancy.all() << endl;

        pf_csr.hbm_port_1.cfg_hbm_eth_payload_occupancy.write();

        data <<"pf_csr.hbm_port_1.cfg_hbm_eth_ctrl.all: 0x" << pf_csr.hbm_port_1.cfg_hbm_eth_ctrl.all() << endl;

        pf_csr.hbm_port_1.cfg_hbm_eth_ctrl.write();

        data <<"pf_csr.hbm_port_1.cfg_hbm_context.all: 0x" << pf_csr.hbm_port_1.cfg_hbm_context.all() << endl;

        pf_csr.hbm_port_1.cfg_hbm_context.write();
    }
    // ELBA_TM_PORT_UPLINK_2
    if (is_active_uplink_port(ELBA_TM_PORT_UPLINK_2)) {
        // If we are an active port, then setup the HBM fifo contexts
        num_contexts = num_hbm_contexts_per_port;
    } else {
        num_contexts = 0;
    }
    base_offset = 2 * num_contexts;

    port_type = elba_tm_get_port_type(ELBA_TM_PORT_UPLINK_2);
    fifo_type = TM_HBM_FIFO_TYPE_UPLINK;
    port_payload_base_val = 0;
    port_payload_size_val = 0;
    port_payload_occupancy_val = 0;
    port_control_base_val = 0;
    port_control_size_val = 0;

    for (unsigned q = 0; q < num_contexts; q++) {
        context = base_offset + q;

        elba_tm_buf_hbm_cfg_t *hbm_cfg = &buf_cfg->hbm_fifo[fifo_type][context];
        if (hbm_cfg->valid) {
            payload_offset = hbm_cfg->payload_offset;
            payload_size = hbm_cfg->payload_chunks;
            // payload occupancy is in units of 1024 bytes
            // program it to drop when 1 jumbo-mtu worth of bytes still free
            payload_occupancy =
                (hbm_chunks_to_bytes(payload_size) - tm_cfg_profile()->jumbo_mtu[port_type]) >> 10;

            if (fifo_type == TM_HBM_FIFO_TYPE_UPLINK) {
                // xoff_threshold is in units of 512 bytes
                // program it to drop when 1000*64 bytes worth of bytes still free
                xoff_threshold = 
                    (hbm_chunks_to_bytes(payload_size) - 
                     ELBA_TM_DEFAULT_XOFF_THRESHOLD_BYTES) >> 9;

                // xon_threshold is in units of 512 bytes
                // program it to drop when 2000*64 bytes worth of bytes still free
                xon_threshold = 
                    (hbm_chunks_to_bytes(payload_size) -
                     ELBA_TM_DEFAULT_XON_THRESHOLD_BYTES) >> 9;
            } else {
                xoff_threshold = 0;
                xon_threshold = 0;
            }

            control_offset = hbm_cfg->control_offset;
            control_size = hbm_cfg->control_chunks;
        } else {
            payload_offset = 0;
            payload_size = 0;
            payload_occupancy = 0;
            xoff_threshold = 0;
            xon_threshold = 0; 
            control_offset = 0;
            control_size = 0;
        }

        // Per port registers
        // 27 bits per hbm_q
        cpp_helper.set_slc(port_payload_base_val, payload_offset,
                            q * 27, ((q + 1) * 27) - 1);
        // 23 bits per q
        cpp_helper.set_slc(port_payload_size_val, payload_size ? payload_size - 1 : 0,
                            q * 23, ((q + 1) * 23) - 1);
        // 19 bits per q
        cpp_helper.set_slc(port_payload_occupancy_val, payload_occupancy ? payload_occupancy - 1 : 0,
                            q * 19, ((q + 1) * 19) - 1);
        // 27 bits per q
        cpp_helper.set_slc(port_control_base_val, control_offset,
                            q * 27, ((q + 1) * 27) - 1);
        // 23 bits per q
        cpp_helper.set_slc(port_control_size_val, control_size ? control_size - 1 : 0,
                            q * 23, ((q + 1) * 23) - 1);

        // Global registers

        // 27 bits per hbm_q
        cpp_helper.set_slc(payload_base_val[fifo_type], payload_offset,
                            context * 27, ((context + 1) * 27) - 1);
        // 23 bits per context
        cpp_helper.set_slc(payload_size_val[fifo_type], payload_size ? payload_size - 1 : 0,
                            context * 23, ((context + 1) * 23) - 1);
        // 27 bits per context
        cpp_helper.set_slc(control_base_val[fifo_type], control_offset,
                            context * 27, ((context + 1) * 27) - 1);
        // 23 bits per context
        cpp_helper.set_slc(control_size_val[fifo_type], control_size ? control_size - 1 : 0,
                            context * 23, ((context + 1) * 23) - 1);

        if (fifo_type == TM_HBM_FIFO_TYPE_UPLINK) {
            // 20 bits per context
            cpp_helper.set_slc(eth_xoff_val, xoff_threshold, 
                               context * 20, ((context + 1) * 20) - 1);
            // 20 bits per context
            cpp_helper.set_slc(eth_xon_val, xon_threshold, 
                               context * 20, ((context + 1) * 20) - 1);
        }
    }

    pf_csr.hbm_port_2.cfg_hbm_eth_payload.base(port_payload_base_val);
    pf_csr.hbm_port_2.cfg_hbm_eth_payload.mem_sz(port_payload_size_val);
    pf_csr.hbm_port_2.cfg_hbm_eth_payload_occupancy.threshold(port_payload_occupancy_val);

    pf_csr.hbm_port_2.cfg_hbm_eth_ctrl.base(port_control_base_val);
    pf_csr.hbm_port_2.cfg_hbm_eth_ctrl.mem_sz(port_control_size_val);


    pf_csr.hbm_port_2.cfg_hbm_context.read();
    pf_csr.hbm_port_2.cfg_hbm_context.enable((1ull<<num_contexts)-1);

    // enable no_drop by default in PB
    pf_csr.hbm_port_2.cfg_hbm_context.no_drop((1ull << num_contexts) - 1);

    // TODO use tm_sw_init_enabled below
    pf_csr.hbm_port_2.cfg_hbm_context.write();

    pf_csr.hbm_port_2.cfg_hbm_context.base(base_offset);
    if (tm_sw_init_enabled()) {

        data <<"pf_csr.hbm_port_2.cfg_hbm_eth_payload.all: 0x" << pf_csr.hbm_port_2.cfg_hbm_eth_payload.all() << endl;

        pf_csr.hbm_port_2.cfg_hbm_eth_payload.write();

        data <<"pf_csr.hbm_port_2.cfg_hbm_eth_payload_occupancy.all: 0x" << pf_csr.hbm_port_2.cfg_hbm_eth_payload_occupancy.all() << endl;

        pf_csr.hbm_port_2.cfg_hbm_eth_payload_occupancy.write();

        data <<"pf_csr.hbm_port_2.cfg_hbm_eth_ctrl.all: 0x" << pf_csr.hbm_port_2.cfg_hbm_eth_ctrl.all() << endl;

        pf_csr.hbm_port_2.cfg_hbm_eth_ctrl.write();

        data <<"pf_csr.hbm_port_2.cfg_hbm_context.all: 0x" << pf_csr.hbm_port_2.cfg_hbm_context.all() << endl;

        pf_csr.hbm_port_2.cfg_hbm_context.write();
    }
    // ELBA_TM_PORT_UPLINK_3
    if (is_active_uplink_port(ELBA_TM_PORT_UPLINK_3)) {
        // If we are an active port, then setup the HBM fifo contexts
        num_contexts = num_hbm_contexts_per_port;
    } else {
        num_contexts = 0;
    }
    base_offset = 3 * num_contexts;

    port_type = elba_tm_get_port_type(ELBA_TM_PORT_UPLINK_3);
    fifo_type = TM_HBM_FIFO_TYPE_UPLINK;
    port_payload_base_val = 0;
    port_payload_size_val = 0;
    port_payload_occupancy_val = 0;
    port_control_base_val = 0;
    port_control_size_val = 0;

    for (unsigned q = 0; q < num_contexts; q++) {
        context = base_offset + q;

        elba_tm_buf_hbm_cfg_t *hbm_cfg = &buf_cfg->hbm_fifo[fifo_type][context];
        if (hbm_cfg->valid) {
            payload_offset = hbm_cfg->payload_offset;
            payload_size = hbm_cfg->payload_chunks;
            // payload occupancy is in units of 1024 bytes
            // program it to drop when 1 jumbo-mtu worth of bytes still free
            payload_occupancy =
                (hbm_chunks_to_bytes(payload_size) - tm_cfg_profile()->jumbo_mtu[port_type]) >> 10;

            if (fifo_type == TM_HBM_FIFO_TYPE_UPLINK) {
                // xoff_threshold is in units of 512 bytes
                // program it to drop when 1000*64 bytes worth of bytes still free
                xoff_threshold = 
                    (hbm_chunks_to_bytes(payload_size) - 
                     ELBA_TM_DEFAULT_XOFF_THRESHOLD_BYTES) >> 9;

                // xon_threshold is in units of 512 bytes
                // program it to drop when 2000*64 bytes worth of bytes still free
                xon_threshold = 
                    (hbm_chunks_to_bytes(payload_size) -
                     ELBA_TM_DEFAULT_XON_THRESHOLD_BYTES) >> 9;
            } else {
                xoff_threshold = 0;
                xon_threshold = 0;
            }

            control_offset = hbm_cfg->control_offset;
            control_size = hbm_cfg->control_chunks;
        } else {
            payload_offset = 0;
            payload_size = 0;
            payload_occupancy = 0;
            xoff_threshold = 0;
            xon_threshold = 0; 
            control_offset = 0;
            control_size = 0;
        }

        // Per port registers
        // 27 bits per hbm_q
        cpp_helper.set_slc(port_payload_base_val, payload_offset,
                            q * 27, ((q + 1) * 27) - 1);
        // 23 bits per q
        cpp_helper.set_slc(port_payload_size_val, payload_size ? payload_size - 1 : 0,
                            q * 23, ((q + 1) * 23) - 1);
        // 19 bits per q
        cpp_helper.set_slc(port_payload_occupancy_val, payload_occupancy ? payload_occupancy - 1 : 0,
                            q * 19, ((q + 1) * 19) - 1);
        // 27 bits per q
        cpp_helper.set_slc(port_control_base_val, control_offset,
                            q * 27, ((q + 1) * 27) - 1);
        // 23 bits per q
        cpp_helper.set_slc(port_control_size_val, control_size ? control_size - 1 : 0,
                            q * 23, ((q + 1) * 23) - 1);

        // Global registers

        // 27 bits per hbm_q
        cpp_helper.set_slc(payload_base_val[fifo_type], payload_offset,
                            context * 27, ((context + 1) * 27) - 1);
        // 23 bits per context
        cpp_helper.set_slc(payload_size_val[fifo_type], payload_size ? payload_size - 1 : 0,
                            context * 23, ((context + 1) * 23) - 1);
        // 27 bits per context
        cpp_helper.set_slc(control_base_val[fifo_type], control_offset,
                            context * 27, ((context + 1) * 27) - 1);
        // 23 bits per context
        cpp_helper.set_slc(control_size_val[fifo_type], control_size ? control_size - 1 : 0,
                            context * 23, ((context + 1) * 23) - 1);

        if (fifo_type == TM_HBM_FIFO_TYPE_UPLINK) {
            // 20 bits per context
            cpp_helper.set_slc(eth_xoff_val, xoff_threshold, 
                               context * 20, ((context + 1) * 20) - 1);
            // 20 bits per context
            cpp_helper.set_slc(eth_xon_val, xon_threshold, 
                               context * 20, ((context + 1) * 20) - 1);
        }
    }

    pf_csr.hbm_port_3.cfg_hbm_eth_payload.base(port_payload_base_val);
    pf_csr.hbm_port_3.cfg_hbm_eth_payload.mem_sz(port_payload_size_val);
    pf_csr.hbm_port_3.cfg_hbm_eth_payload_occupancy.threshold(port_payload_occupancy_val);

    pf_csr.hbm_port_3.cfg_hbm_eth_ctrl.base(port_control_base_val);
    pf_csr.hbm_port_3.cfg_hbm_eth_ctrl.mem_sz(port_control_size_val);


    pf_csr.hbm_port_3.cfg_hbm_context.read();
    pf_csr.hbm_port_3.cfg_hbm_context.enable((1ull<<num_contexts)-1);

    // enable no_drop by default in PB
    pf_csr.hbm_port_3.cfg_hbm_context.no_drop((1ull << num_contexts) - 1);

    // TODO use tm_sw_init_enabled below
    pf_csr.hbm_port_3.cfg_hbm_context.write();

    pf_csr.hbm_port_3.cfg_hbm_context.base(base_offset);
    if (tm_sw_init_enabled()) {

        data <<"pf_csr.hbm_port_3.cfg_hbm_eth_payload.all: 0x" << pf_csr.hbm_port_3.cfg_hbm_eth_payload.all() << endl;

        pf_csr.hbm_port_3.cfg_hbm_eth_payload.write();

        data <<"pf_csr.hbm_port_3.cfg_hbm_eth_payload_occupancy.all: 0x" << pf_csr.hbm_port_3.cfg_hbm_eth_payload_occupancy.all() << endl;

        pf_csr.hbm_port_3.cfg_hbm_eth_payload_occupancy.write();

        data <<"pf_csr.hbm_port_3.cfg_hbm_eth_ctrl.all: 0x" << pf_csr.hbm_port_3.cfg_hbm_eth_ctrl.all() << endl;

        pf_csr.hbm_port_3.cfg_hbm_eth_ctrl.write();

        data <<"pf_csr.hbm_port_3.cfg_hbm_context.all: 0x" << pf_csr.hbm_port_3.cfg_hbm_context.all() << endl;

        pf_csr.hbm_port_3.cfg_hbm_context.write();
    }


    pf_csr.cfg_hbm_eth_payload.base(payload_base_val[TM_HBM_FIFO_TYPE_UPLINK]);
    pf_csr.cfg_hbm_eth_payload.mem_sz(payload_size_val[TM_HBM_FIFO_TYPE_UPLINK]);
    pf_csr.cfg_hbm_eth_ctrl.base(control_base_val[TM_HBM_FIFO_TYPE_UPLINK]);
    pf_csr.cfg_hbm_eth_ctrl.mem_sz(control_size_val[TM_HBM_FIFO_TYPE_UPLINK]);

    // Write all the registers
    {
        elb_pbc_hbm_eth_ctl_t hbm_ctl_decoder;

        hbm_ctl_decoder.init();
        hbm_ctl_decoder.all(pf_csr.cfg_hbm_eth_payload.all());
        hbm_ctl_decoder.set_name("elb0.pf.pf.cfg_hbm_eth_payload.decoder");
//        hbm_ctl_decoder.show();

        hbm_ctl_decoder.init();
        hbm_ctl_decoder.all(pf_csr.cfg_hbm_eth_ctrl.all());
        hbm_ctl_decoder.set_name("elb0.pf.pf.cfg_hbm_eth_ctrl.decoder");
//        hbm_ctl_decoder.show();

    }

    if (tm_sw_init_enabled()) {
        data <<"pf_csr.cfg_hbm_eth_payload.all: 0x" << pf_csr.cfg_hbm_eth_payload.all() << endl;

        data <<"pf_csr.cfg_hbm_eth_ctrl.all: 0x" << pf_csr.cfg_hbm_eth_ctrl.all() << endl;


        pf_csr.cfg_hbm_eth_payload.write();
        pf_csr.cfg_hbm_eth_ctrl.write();
    }
    
    pf_csr.cfg_hbm_threshold.read();
    pf_csr.cfg_hbm_threshold.xoff(eth_xoff_val);
    pf_csr.cfg_hbm_threshold.xon(eth_xon_val);
    if (tm_sw_init_enabled()) {
        data <<"pf_csr.cfg_hbm_threshold.all: 0x" << pf_csr.cfg_hbm_threshold.all() << endl;
;
        pf_csr.cfg_hbm_threshold.write();
    }

    SDK_TRACE_DEBUG("%s", data.str().c_str());

    return ret;
}

static sdk_ret_t
elba_tm_program_buffers (elba_tm_buf_cfg_t *buf_cfg)
{
    sdk_ret_t ret = sdk::SDK_RET_OK;

    ret = elba_tm_program_pbc_buffers(buf_cfg);
    if (ret != sdk::SDK_RET_OK) {
        SDK_TRACE_ERR("Error programming pbc buffers ret %d",
                      ret);
        return ret;
    }

    ret = elba_tm_program_p4_credits(buf_cfg);
    if (ret != sdk::SDK_RET_OK) {
        SDK_TRACE_ERR("Error programming hbm buffers ret %d",
                      ret);
        return ret;
    }
    return ret;
}

static sdk_ret_t
elba_tm_port_program_defaults (void)
{
    elb_top_csr_t &elb0 = ELB_BLK_REG_MODEL_ACCESS(elb_top_csr_t, 0, 0);
    elb_pbc_csr_t &pbc_csr = elb0.pb.pbc;
    cpp_int tc_to_pg_val;
    cpp_int xoff2oq_map_val;
    tm_port_type_e port_type;
    uint32_t mtu_cells;
    uint32_t nbits;
    uint32_t oq;

    // For every port, program the tc_to_pg mapping
    // and mtu

    stringstream data;
    data << hex << endl;

    // ELBA_TM_PORT_UPLINK_0
    port_type = elba_tm_get_port_type(ELBA_TM_PORT_UPLINK_0);
    tc_to_pg_val = 0;

    pbc_csr.port_0.cfg_account_mtu_table.read();
    pbc_csr.port_0.cfg_account_tc_to_pg.read();
    pbc_csr.port_0.cfg_account_control.read();

    pbc_csr.port_0.cfg_account_control.use_sp_as_wm(1);

    mtu_cells = bytes_to_cells(tm_cfg_profile()->jumbo_mtu[port_type]);
    cpp_helper.set_slc(tc_to_pg_val, 0, 0 * 3, ((0+1) * 3) - 1);
    pbc_csr.port_0.cfg_account_mtu_table.pg0(mtu_cells);
    cpp_helper.set_slc(tc_to_pg_val, 1, 1 * 3, ((1+1) * 3) - 1);
    pbc_csr.port_0.cfg_account_mtu_table.pg1(mtu_cells);
    cpp_helper.set_slc(tc_to_pg_val, 2, 2 * 3, ((2+1) * 3) - 1);
    pbc_csr.port_0.cfg_account_mtu_table.pg2(mtu_cells);
    cpp_helper.set_slc(tc_to_pg_val, 3, 3 * 3, ((3+1) * 3) - 1);
    pbc_csr.port_0.cfg_account_mtu_table.pg3(mtu_cells);
    cpp_helper.set_slc(tc_to_pg_val, 4, 4 * 3, ((4+1) * 3) - 1);
    pbc_csr.port_0.cfg_account_mtu_table.pg4(mtu_cells);
    cpp_helper.set_slc(tc_to_pg_val, 5, 5 * 3, ((5+1) * 3) - 1);
    pbc_csr.port_0.cfg_account_mtu_table.pg5(mtu_cells);
    cpp_helper.set_slc(tc_to_pg_val, 6, 6 * 3, ((6+1) * 3) - 1);
    pbc_csr.port_0.cfg_account_mtu_table.pg6(mtu_cells);
    cpp_helper.set_slc(tc_to_pg_val, 7, 7 * 3, ((7+1) * 3) - 1);
    pbc_csr.port_0.cfg_account_mtu_table.pg7(mtu_cells);
    pbc_csr.port_0.cfg_account_tc_to_pg.table(tc_to_pg_val);

    if (tm_sw_init_enabled()) {
        elb_pbc_pg8_map_t pg_map_decoder;
        pg_map_decoder.init();
        pg_map_decoder.all(pbc_csr.port_0.cfg_account_tc_to_pg.table());
        pg_map_decoder.set_name("elb0.pb.pbc.port_0.cfg_account_tc_to_pg.decoder");
        pg_map_decoder.show();

        data <<"pbc_csr.port_0.cfg_account_mtu_table.all: 0x" << pbc_csr.port_0.cfg_account_mtu_table.all() << endl;
data <<"pbc_csr.port_0.cfg_account_mtu_table.pg6: 0x" << pbc_csr.port_0.cfg_account_mtu_table.pg6() << endl;
data <<"pbc_csr.port_0.cfg_account_mtu_table.pg7: 0x" << pbc_csr.port_0.cfg_account_mtu_table.pg7() << endl;
data <<"pbc_csr.port_0.cfg_account_mtu_table.pg4: 0x" << pbc_csr.port_0.cfg_account_mtu_table.pg4() << endl;
data <<"pbc_csr.port_0.cfg_account_mtu_table.pg5: 0x" << pbc_csr.port_0.cfg_account_mtu_table.pg5() << endl;
data <<"pbc_csr.port_0.cfg_account_mtu_table.pg2: 0x" << pbc_csr.port_0.cfg_account_mtu_table.pg2() << endl;
data <<"pbc_csr.port_0.cfg_account_mtu_table.pg3: 0x" << pbc_csr.port_0.cfg_account_mtu_table.pg3() << endl;
data <<"pbc_csr.port_0.cfg_account_mtu_table.pg0: 0x" << pbc_csr.port_0.cfg_account_mtu_table.pg0() << endl;
data <<"pbc_csr.port_0.cfg_account_mtu_table.pg1: 0x" << pbc_csr.port_0.cfg_account_mtu_table.pg1() << endl;

        data <<"pbc_csr.port_0.cfg_account_tc_to_pg.all: 0x" << pbc_csr.port_0.cfg_account_tc_to_pg.all() << endl;
data <<"pbc_csr.port_0.cfg_account_tc_to_pg.table: 0x" << pbc_csr.port_0.cfg_account_tc_to_pg.table() << endl;

        data <<"pbc_csr.port_0.cfg_account_control.all: 0x" << pbc_csr.port_0.cfg_account_control.all() << endl;
data <<"pbc_csr.port_0.cfg_account_control.sp_enable: 0x" << pbc_csr.port_0.cfg_account_control.sp_enable() << endl;
data <<"pbc_csr.port_0.cfg_account_control.clear_wm_index: 0x" << pbc_csr.port_0.cfg_account_control.clear_wm_index() << endl;
data <<"pbc_csr.port_0.cfg_account_control.clear_wm_enable: 0x" << pbc_csr.port_0.cfg_account_control.clear_wm_enable() << endl;
data <<"pbc_csr.port_0.cfg_account_control.use_sp_as_wm: 0x" << pbc_csr.port_0.cfg_account_control.use_sp_as_wm() << endl;


        pbc_csr.port_0.cfg_account_control.write();
        pbc_csr.port_0.cfg_account_mtu_table.write();
        pbc_csr.port_0.cfg_account_tc_to_pg.write();
    }
    // ELBA_TM_PORT_UPLINK_1
    port_type = elba_tm_get_port_type(ELBA_TM_PORT_UPLINK_1);
    tc_to_pg_val = 0;

    pbc_csr.port_1.cfg_account_mtu_table.read();
    pbc_csr.port_1.cfg_account_tc_to_pg.read();
    pbc_csr.port_1.cfg_account_control.read();

    pbc_csr.port_1.cfg_account_control.use_sp_as_wm(1);

    mtu_cells = bytes_to_cells(tm_cfg_profile()->jumbo_mtu[port_type]);
    cpp_helper.set_slc(tc_to_pg_val, 0, 0 * 3, ((0+1) * 3) - 1);
    pbc_csr.port_1.cfg_account_mtu_table.pg0(mtu_cells);
    cpp_helper.set_slc(tc_to_pg_val, 1, 1 * 3, ((1+1) * 3) - 1);
    pbc_csr.port_1.cfg_account_mtu_table.pg1(mtu_cells);
    cpp_helper.set_slc(tc_to_pg_val, 2, 2 * 3, ((2+1) * 3) - 1);
    pbc_csr.port_1.cfg_account_mtu_table.pg2(mtu_cells);
    cpp_helper.set_slc(tc_to_pg_val, 3, 3 * 3, ((3+1) * 3) - 1);
    pbc_csr.port_1.cfg_account_mtu_table.pg3(mtu_cells);
    cpp_helper.set_slc(tc_to_pg_val, 4, 4 * 3, ((4+1) * 3) - 1);
    pbc_csr.port_1.cfg_account_mtu_table.pg4(mtu_cells);
    cpp_helper.set_slc(tc_to_pg_val, 5, 5 * 3, ((5+1) * 3) - 1);
    pbc_csr.port_1.cfg_account_mtu_table.pg5(mtu_cells);
    cpp_helper.set_slc(tc_to_pg_val, 6, 6 * 3, ((6+1) * 3) - 1);
    pbc_csr.port_1.cfg_account_mtu_table.pg6(mtu_cells);
    cpp_helper.set_slc(tc_to_pg_val, 7, 7 * 3, ((7+1) * 3) - 1);
    pbc_csr.port_1.cfg_account_mtu_table.pg7(mtu_cells);
    pbc_csr.port_1.cfg_account_tc_to_pg.table(tc_to_pg_val);

    if (tm_sw_init_enabled()) {
        elb_pbc_pg8_map_t pg_map_decoder;
        pg_map_decoder.init();
        pg_map_decoder.all(pbc_csr.port_1.cfg_account_tc_to_pg.table());
        pg_map_decoder.set_name("elb0.pb.pbc.port_1.cfg_account_tc_to_pg.decoder");
        pg_map_decoder.show();

        data <<"pbc_csr.port_1.cfg_account_mtu_table.all: 0x" << pbc_csr.port_1.cfg_account_mtu_table.all() << endl;
data <<"pbc_csr.port_1.cfg_account_mtu_table.pg6: 0x" << pbc_csr.port_1.cfg_account_mtu_table.pg6() << endl;
data <<"pbc_csr.port_1.cfg_account_mtu_table.pg7: 0x" << pbc_csr.port_1.cfg_account_mtu_table.pg7() << endl;
data <<"pbc_csr.port_1.cfg_account_mtu_table.pg4: 0x" << pbc_csr.port_1.cfg_account_mtu_table.pg4() << endl;
data <<"pbc_csr.port_1.cfg_account_mtu_table.pg5: 0x" << pbc_csr.port_1.cfg_account_mtu_table.pg5() << endl;
data <<"pbc_csr.port_1.cfg_account_mtu_table.pg2: 0x" << pbc_csr.port_1.cfg_account_mtu_table.pg2() << endl;
data <<"pbc_csr.port_1.cfg_account_mtu_table.pg3: 0x" << pbc_csr.port_1.cfg_account_mtu_table.pg3() << endl;
data <<"pbc_csr.port_1.cfg_account_mtu_table.pg0: 0x" << pbc_csr.port_1.cfg_account_mtu_table.pg0() << endl;
data <<"pbc_csr.port_1.cfg_account_mtu_table.pg1: 0x" << pbc_csr.port_1.cfg_account_mtu_table.pg1() << endl;

        data <<"pbc_csr.port_1.cfg_account_tc_to_pg.all: 0x" << pbc_csr.port_1.cfg_account_tc_to_pg.all() << endl;
data <<"pbc_csr.port_1.cfg_account_tc_to_pg.table: 0x" << pbc_csr.port_1.cfg_account_tc_to_pg.table() << endl;

        data <<"pbc_csr.port_1.cfg_account_control.all: 0x" << pbc_csr.port_1.cfg_account_control.all() << endl;
data <<"pbc_csr.port_1.cfg_account_control.sp_enable: 0x" << pbc_csr.port_1.cfg_account_control.sp_enable() << endl;
data <<"pbc_csr.port_1.cfg_account_control.clear_wm_index: 0x" << pbc_csr.port_1.cfg_account_control.clear_wm_index() << endl;
data <<"pbc_csr.port_1.cfg_account_control.clear_wm_enable: 0x" << pbc_csr.port_1.cfg_account_control.clear_wm_enable() << endl;
data <<"pbc_csr.port_1.cfg_account_control.use_sp_as_wm: 0x" << pbc_csr.port_1.cfg_account_control.use_sp_as_wm() << endl;


        pbc_csr.port_1.cfg_account_control.write();
        pbc_csr.port_1.cfg_account_mtu_table.write();
        pbc_csr.port_1.cfg_account_tc_to_pg.write();
    }
    // ELBA_TM_PORT_UPLINK_2
    port_type = elba_tm_get_port_type(ELBA_TM_PORT_UPLINK_2);
    tc_to_pg_val = 0;

    pbc_csr.port_2.cfg_account_mtu_table.read();
    pbc_csr.port_2.cfg_account_tc_to_pg.read();
    pbc_csr.port_2.cfg_account_control.read();

    pbc_csr.port_2.cfg_account_control.use_sp_as_wm(1);

    mtu_cells = bytes_to_cells(tm_cfg_profile()->jumbo_mtu[port_type]);
    cpp_helper.set_slc(tc_to_pg_val, 0, 0 * 3, ((0+1) * 3) - 1);
    pbc_csr.port_2.cfg_account_mtu_table.pg0(mtu_cells);
    cpp_helper.set_slc(tc_to_pg_val, 1, 1 * 3, ((1+1) * 3) - 1);
    pbc_csr.port_2.cfg_account_mtu_table.pg1(mtu_cells);
    cpp_helper.set_slc(tc_to_pg_val, 2, 2 * 3, ((2+1) * 3) - 1);
    pbc_csr.port_2.cfg_account_mtu_table.pg2(mtu_cells);
    cpp_helper.set_slc(tc_to_pg_val, 3, 3 * 3, ((3+1) * 3) - 1);
    pbc_csr.port_2.cfg_account_mtu_table.pg3(mtu_cells);
    cpp_helper.set_slc(tc_to_pg_val, 4, 4 * 3, ((4+1) * 3) - 1);
    pbc_csr.port_2.cfg_account_mtu_table.pg4(mtu_cells);
    cpp_helper.set_slc(tc_to_pg_val, 5, 5 * 3, ((5+1) * 3) - 1);
    pbc_csr.port_2.cfg_account_mtu_table.pg5(mtu_cells);
    cpp_helper.set_slc(tc_to_pg_val, 6, 6 * 3, ((6+1) * 3) - 1);
    pbc_csr.port_2.cfg_account_mtu_table.pg6(mtu_cells);
    cpp_helper.set_slc(tc_to_pg_val, 7, 7 * 3, ((7+1) * 3) - 1);
    pbc_csr.port_2.cfg_account_mtu_table.pg7(mtu_cells);
    pbc_csr.port_2.cfg_account_tc_to_pg.table(tc_to_pg_val);

    if (tm_sw_init_enabled()) {
        elb_pbc_pg8_map_t pg_map_decoder;
        pg_map_decoder.init();
        pg_map_decoder.all(pbc_csr.port_2.cfg_account_tc_to_pg.table());
        pg_map_decoder.set_name("elb0.pb.pbc.port_2.cfg_account_tc_to_pg.decoder");
        pg_map_decoder.show();

        data <<"pbc_csr.port_2.cfg_account_mtu_table.all: 0x" << pbc_csr.port_2.cfg_account_mtu_table.all() << endl;
data <<"pbc_csr.port_2.cfg_account_mtu_table.pg6: 0x" << pbc_csr.port_2.cfg_account_mtu_table.pg6() << endl;
data <<"pbc_csr.port_2.cfg_account_mtu_table.pg7: 0x" << pbc_csr.port_2.cfg_account_mtu_table.pg7() << endl;
data <<"pbc_csr.port_2.cfg_account_mtu_table.pg4: 0x" << pbc_csr.port_2.cfg_account_mtu_table.pg4() << endl;
data <<"pbc_csr.port_2.cfg_account_mtu_table.pg5: 0x" << pbc_csr.port_2.cfg_account_mtu_table.pg5() << endl;
data <<"pbc_csr.port_2.cfg_account_mtu_table.pg2: 0x" << pbc_csr.port_2.cfg_account_mtu_table.pg2() << endl;
data <<"pbc_csr.port_2.cfg_account_mtu_table.pg3: 0x" << pbc_csr.port_2.cfg_account_mtu_table.pg3() << endl;
data <<"pbc_csr.port_2.cfg_account_mtu_table.pg0: 0x" << pbc_csr.port_2.cfg_account_mtu_table.pg0() << endl;
data <<"pbc_csr.port_2.cfg_account_mtu_table.pg1: 0x" << pbc_csr.port_2.cfg_account_mtu_table.pg1() << endl;

        data <<"pbc_csr.port_2.cfg_account_tc_to_pg.all: 0x" << pbc_csr.port_2.cfg_account_tc_to_pg.all() << endl;
data <<"pbc_csr.port_2.cfg_account_tc_to_pg.table: 0x" << pbc_csr.port_2.cfg_account_tc_to_pg.table() << endl;

        data <<"pbc_csr.port_2.cfg_account_control.all: 0x" << pbc_csr.port_2.cfg_account_control.all() << endl;
data <<"pbc_csr.port_2.cfg_account_control.sp_enable: 0x" << pbc_csr.port_2.cfg_account_control.sp_enable() << endl;
data <<"pbc_csr.port_2.cfg_account_control.clear_wm_index: 0x" << pbc_csr.port_2.cfg_account_control.clear_wm_index() << endl;
data <<"pbc_csr.port_2.cfg_account_control.clear_wm_enable: 0x" << pbc_csr.port_2.cfg_account_control.clear_wm_enable() << endl;
data <<"pbc_csr.port_2.cfg_account_control.use_sp_as_wm: 0x" << pbc_csr.port_2.cfg_account_control.use_sp_as_wm() << endl;


        pbc_csr.port_2.cfg_account_control.write();
        pbc_csr.port_2.cfg_account_mtu_table.write();
        pbc_csr.port_2.cfg_account_tc_to_pg.write();
    }
    // ELBA_TM_PORT_UPLINK_3
    port_type = elba_tm_get_port_type(ELBA_TM_PORT_UPLINK_3);
    tc_to_pg_val = 0;

    pbc_csr.port_3.cfg_account_mtu_table.read();
    pbc_csr.port_3.cfg_account_tc_to_pg.read();
    pbc_csr.port_3.cfg_account_control.read();

    pbc_csr.port_3.cfg_account_control.use_sp_as_wm(1);

    mtu_cells = bytes_to_cells(tm_cfg_profile()->jumbo_mtu[port_type]);
    cpp_helper.set_slc(tc_to_pg_val, 0, 0 * 3, ((0+1) * 3) - 1);
    pbc_csr.port_3.cfg_account_mtu_table.pg0(mtu_cells);
    cpp_helper.set_slc(tc_to_pg_val, 1, 1 * 3, ((1+1) * 3) - 1);
    pbc_csr.port_3.cfg_account_mtu_table.pg1(mtu_cells);
    cpp_helper.set_slc(tc_to_pg_val, 2, 2 * 3, ((2+1) * 3) - 1);
    pbc_csr.port_3.cfg_account_mtu_table.pg2(mtu_cells);
    cpp_helper.set_slc(tc_to_pg_val, 3, 3 * 3, ((3+1) * 3) - 1);
    pbc_csr.port_3.cfg_account_mtu_table.pg3(mtu_cells);
    cpp_helper.set_slc(tc_to_pg_val, 4, 4 * 3, ((4+1) * 3) - 1);
    pbc_csr.port_3.cfg_account_mtu_table.pg4(mtu_cells);
    cpp_helper.set_slc(tc_to_pg_val, 5, 5 * 3, ((5+1) * 3) - 1);
    pbc_csr.port_3.cfg_account_mtu_table.pg5(mtu_cells);
    cpp_helper.set_slc(tc_to_pg_val, 6, 6 * 3, ((6+1) * 3) - 1);
    pbc_csr.port_3.cfg_account_mtu_table.pg6(mtu_cells);
    cpp_helper.set_slc(tc_to_pg_val, 7, 7 * 3, ((7+1) * 3) - 1);
    pbc_csr.port_3.cfg_account_mtu_table.pg7(mtu_cells);
    pbc_csr.port_3.cfg_account_tc_to_pg.table(tc_to_pg_val);

    if (tm_sw_init_enabled()) {
        elb_pbc_pg8_map_t pg_map_decoder;
        pg_map_decoder.init();
        pg_map_decoder.all(pbc_csr.port_3.cfg_account_tc_to_pg.table());
        pg_map_decoder.set_name("elb0.pb.pbc.port_3.cfg_account_tc_to_pg.decoder");
        pg_map_decoder.show();

        data <<"pbc_csr.port_3.cfg_account_mtu_table.all: 0x" << pbc_csr.port_3.cfg_account_mtu_table.all() << endl;
data <<"pbc_csr.port_3.cfg_account_mtu_table.pg6: 0x" << pbc_csr.port_3.cfg_account_mtu_table.pg6() << endl;
data <<"pbc_csr.port_3.cfg_account_mtu_table.pg7: 0x" << pbc_csr.port_3.cfg_account_mtu_table.pg7() << endl;
data <<"pbc_csr.port_3.cfg_account_mtu_table.pg4: 0x" << pbc_csr.port_3.cfg_account_mtu_table.pg4() << endl;
data <<"pbc_csr.port_3.cfg_account_mtu_table.pg5: 0x" << pbc_csr.port_3.cfg_account_mtu_table.pg5() << endl;
data <<"pbc_csr.port_3.cfg_account_mtu_table.pg2: 0x" << pbc_csr.port_3.cfg_account_mtu_table.pg2() << endl;
data <<"pbc_csr.port_3.cfg_account_mtu_table.pg3: 0x" << pbc_csr.port_3.cfg_account_mtu_table.pg3() << endl;
data <<"pbc_csr.port_3.cfg_account_mtu_table.pg0: 0x" << pbc_csr.port_3.cfg_account_mtu_table.pg0() << endl;
data <<"pbc_csr.port_3.cfg_account_mtu_table.pg1: 0x" << pbc_csr.port_3.cfg_account_mtu_table.pg1() << endl;

        data <<"pbc_csr.port_3.cfg_account_tc_to_pg.all: 0x" << pbc_csr.port_3.cfg_account_tc_to_pg.all() << endl;
data <<"pbc_csr.port_3.cfg_account_tc_to_pg.table: 0x" << pbc_csr.port_3.cfg_account_tc_to_pg.table() << endl;

        data <<"pbc_csr.port_3.cfg_account_control.all: 0x" << pbc_csr.port_3.cfg_account_control.all() << endl;
data <<"pbc_csr.port_3.cfg_account_control.sp_enable: 0x" << pbc_csr.port_3.cfg_account_control.sp_enable() << endl;
data <<"pbc_csr.port_3.cfg_account_control.clear_wm_index: 0x" << pbc_csr.port_3.cfg_account_control.clear_wm_index() << endl;
data <<"pbc_csr.port_3.cfg_account_control.clear_wm_enable: 0x" << pbc_csr.port_3.cfg_account_control.clear_wm_enable() << endl;
data <<"pbc_csr.port_3.cfg_account_control.use_sp_as_wm: 0x" << pbc_csr.port_3.cfg_account_control.use_sp_as_wm() << endl;


        pbc_csr.port_3.cfg_account_control.write();
        pbc_csr.port_3.cfg_account_mtu_table.write();
        pbc_csr.port_3.cfg_account_tc_to_pg.write();
    }

    // On dma port, set the xoff to oq for flow control in RxDMA
    // ELBA_TM_PORT_UPLINK_0
    port_type = elba_tm_get_port_type(ELBA_TM_PORT_UPLINK_0);
    xoff2oq_map_val = 0;
    if (port_type == TM_PORT_TYPE_UPLINK) {
        nbits = 3;
    } else {
        nbits = 5;
    }
    for (oq = 0; oq < elba_tm_get_num_oqs_for_port_type(port_type); oq++) {
        cpp_helper.set_slc(xoff2oq_map_val, oq, oq*nbits, ((oq+1)*nbits)-1);
    }

    pbc_csr.port_0.cfg_oq_xoff2oq.read();
    pbc_csr.port_0.cfg_oq_xoff2oq.map(xoff2oq_map_val);
    if (tm_sw_init_enabled()) {
        data <<"pbc_csr.port_0.cfg_oq_xoff2oq.all: 0x" << pbc_csr.port_0.cfg_oq_xoff2oq.all() << endl;
data <<"pbc_csr.port_0.cfg_oq_xoff2oq.map: 0x" << pbc_csr.port_0.cfg_oq_xoff2oq.map() << endl;


        pbc_csr.port_0.cfg_oq_xoff2oq.write();
    }
    // ELBA_TM_PORT_UPLINK_1
    port_type = elba_tm_get_port_type(ELBA_TM_PORT_UPLINK_1);
    xoff2oq_map_val = 0;
    if (port_type == TM_PORT_TYPE_UPLINK) {
        nbits = 3;
    } else {
        nbits = 5;
    }
    for (oq = 0; oq < elba_tm_get_num_oqs_for_port_type(port_type); oq++) {
        cpp_helper.set_slc(xoff2oq_map_val, oq, oq*nbits, ((oq+1)*nbits)-1);
    }

    pbc_csr.port_1.cfg_oq_xoff2oq.read();
    pbc_csr.port_1.cfg_oq_xoff2oq.map(xoff2oq_map_val);
    if (tm_sw_init_enabled()) {
        data <<"pbc_csr.port_1.cfg_oq_xoff2oq.all: 0x" << pbc_csr.port_1.cfg_oq_xoff2oq.all() << endl;
data <<"pbc_csr.port_1.cfg_oq_xoff2oq.map: 0x" << pbc_csr.port_1.cfg_oq_xoff2oq.map() << endl;


        pbc_csr.port_1.cfg_oq_xoff2oq.write();
    }
    // ELBA_TM_PORT_UPLINK_2
    port_type = elba_tm_get_port_type(ELBA_TM_PORT_UPLINK_2);
    xoff2oq_map_val = 0;
    if (port_type == TM_PORT_TYPE_UPLINK) {
        nbits = 3;
    } else {
        nbits = 5;
    }
    for (oq = 0; oq < elba_tm_get_num_oqs_for_port_type(port_type); oq++) {
        cpp_helper.set_slc(xoff2oq_map_val, oq, oq*nbits, ((oq+1)*nbits)-1);
    }

    pbc_csr.port_2.cfg_oq_xoff2oq.read();
    pbc_csr.port_2.cfg_oq_xoff2oq.map(xoff2oq_map_val);
    if (tm_sw_init_enabled()) {
        data <<"pbc_csr.port_2.cfg_oq_xoff2oq.all: 0x" << pbc_csr.port_2.cfg_oq_xoff2oq.all() << endl;
data <<"pbc_csr.port_2.cfg_oq_xoff2oq.map: 0x" << pbc_csr.port_2.cfg_oq_xoff2oq.map() << endl;


        pbc_csr.port_2.cfg_oq_xoff2oq.write();
    }
    // ELBA_TM_PORT_UPLINK_3
    port_type = elba_tm_get_port_type(ELBA_TM_PORT_UPLINK_3);
    xoff2oq_map_val = 0;
    if (port_type == TM_PORT_TYPE_UPLINK) {
        nbits = 3;
    } else {
        nbits = 5;
    }
    for (oq = 0; oq < elba_tm_get_num_oqs_for_port_type(port_type); oq++) {
        cpp_helper.set_slc(xoff2oq_map_val, oq, oq*nbits, ((oq+1)*nbits)-1);
    }

    pbc_csr.port_3.cfg_oq_xoff2oq.read();
    pbc_csr.port_3.cfg_oq_xoff2oq.map(xoff2oq_map_val);
    if (tm_sw_init_enabled()) {
        data <<"pbc_csr.port_3.cfg_oq_xoff2oq.all: 0x" << pbc_csr.port_3.cfg_oq_xoff2oq.all() << endl;
data <<"pbc_csr.port_3.cfg_oq_xoff2oq.map: 0x" << pbc_csr.port_3.cfg_oq_xoff2oq.map() << endl;


        pbc_csr.port_3.cfg_oq_xoff2oq.write();
    }

    // On uplink ports, disable xoff on all oqs
    // ELBA_TM_PORT_UPLINK_0
    // pbc_csr.port_0.cfg_mac_xoff.read();
    // TODO: write to disable xoff by default
    // pbc_csr.port_0.cfg_mac_xoff.enable(0);
    // if (tm_sw_init_enabled()) {
    //    data <<"pbc_csr.port_0.cfg_mac_xoff.all: 0x" << pbc_csr.port_0.cfg_mac_xoff.all() << endl;
data <<"pbc_csr.port_0.cfg_mac_xoff.enable: 0x" << pbc_csr.port_0.cfg_mac_xoff.enable() << endl;

    //     pbc_csr.port_0.cfg_mac_xoff.write();
    //}
    // ELBA_TM_PORT_UPLINK_1
    // pbc_csr.port_1.cfg_mac_xoff.read();
    // TODO: write to disable xoff by default
    // pbc_csr.port_1.cfg_mac_xoff.enable(0);
    // if (tm_sw_init_enabled()) {
    //    data <<"pbc_csr.port_1.cfg_mac_xoff.all: 0x" << pbc_csr.port_1.cfg_mac_xoff.all() << endl;
data <<"pbc_csr.port_1.cfg_mac_xoff.enable: 0x" << pbc_csr.port_1.cfg_mac_xoff.enable() << endl;

    //     pbc_csr.port_1.cfg_mac_xoff.write();
    //}
    // ELBA_TM_PORT_UPLINK_2
    // pbc_csr.port_2.cfg_mac_xoff.read();
    // TODO: write to disable xoff by default
    // pbc_csr.port_2.cfg_mac_xoff.enable(0);
    // if (tm_sw_init_enabled()) {
    //    data <<"pbc_csr.port_2.cfg_mac_xoff.all: 0x" << pbc_csr.port_2.cfg_mac_xoff.all() << endl;
data <<"pbc_csr.port_2.cfg_mac_xoff.enable: 0x" << pbc_csr.port_2.cfg_mac_xoff.enable() << endl;

    //     pbc_csr.port_2.cfg_mac_xoff.write();
    //}
    // ELBA_TM_PORT_UPLINK_3
    // pbc_csr.port_3.cfg_mac_xoff.read();
    // TODO: write to disable xoff by default
    // pbc_csr.port_3.cfg_mac_xoff.enable(0);
    // if (tm_sw_init_enabled()) {
    //    data <<"pbc_csr.port_3.cfg_mac_xoff.all: 0x" << pbc_csr.port_3.cfg_mac_xoff.all() << endl;
data <<"pbc_csr.port_3.cfg_mac_xoff.enable: 0x" << pbc_csr.port_3.cfg_mac_xoff.enable() << endl;

    //     pbc_csr.port_3.cfg_mac_xoff.write();
    //}

    // On uplink ports, set the number of header bytes to remove
    // ELBA_TM_PORT_UPLINK_0
    port_type = elba_tm_get_port_type(ELBA_TM_PORT_UPLINK_0);
    pbc_csr.port_0.cfg_oq.read();
    pbc_csr.port_0.cfg_oq.num_hdr_bytes(
        ELBA_GLOBAL_INTRINSIC_HDR_SZ + ELBA_P4_INTRINSIC_HDR_SZ);
    // Tell MAC to stomp CRC on error
    pbc_csr.port_0.cfg_oq.hw_error_to_pbus(1);

    if (tm_sw_init_enabled()) {
        pbc_csr.port_0.cfg_oq.enable(1);
        pbc_csr.port_0.cfg_oq.rewrite_enable(1);
        pbc_csr.port_0.cfg_oq.flow_control_enable_xoff(1);
    }
    data <<"pbc_csr.port_0.cfg_oq.all: 0x" << pbc_csr.port_0.cfg_oq.all() << endl;
data <<"pbc_csr.port_0.cfg_oq.enable: 0x" << pbc_csr.port_0.cfg_oq.enable() << endl;
data <<"pbc_csr.port_0.cfg_oq.num_hdr_bytes: 0x" << pbc_csr.port_0.cfg_oq.num_hdr_bytes() << endl;
data <<"pbc_csr.port_0.cfg_oq.num_hdr_bytes_crypto: 0x" << pbc_csr.port_0.cfg_oq.num_hdr_bytes_crypto() << endl;
data <<"pbc_csr.port_0.cfg_oq.span_cpu_num_cells: 0x" << pbc_csr.port_0.cfg_oq.span_cpu_num_cells() << endl;
data <<"pbc_csr.port_0.cfg_oq.flush_hw_error: 0x" << pbc_csr.port_0.cfg_oq.flush_hw_error() << endl;
data <<"pbc_csr.port_0.cfg_oq.eg_ts_enable: 0x" << pbc_csr.port_0.cfg_oq.eg_ts_enable() << endl;
data <<"pbc_csr.port_0.cfg_oq.ipg_bytes: 0x" << pbc_csr.port_0.cfg_oq.ipg_bytes() << endl;
data <<"pbc_csr.port_0.cfg_oq.flush: 0x" << pbc_csr.port_0.cfg_oq.flush() << endl;
data <<"pbc_csr.port_0.cfg_oq.flow_control_enable_xoff: 0x" << pbc_csr.port_0.cfg_oq.flow_control_enable_xoff() << endl;
data <<"pbc_csr.port_0.cfg_oq.hw_error_to_pbus: 0x" << pbc_csr.port_0.cfg_oq.hw_error_to_pbus() << endl;
data <<"pbc_csr.port_0.cfg_oq.rewrite_enable: 0x" << pbc_csr.port_0.cfg_oq.rewrite_enable() << endl;

    pbc_csr.port_0.cfg_oq.write();
    // ELBA_TM_PORT_UPLINK_1
    port_type = elba_tm_get_port_type(ELBA_TM_PORT_UPLINK_1);
    pbc_csr.port_1.cfg_oq.read();
    pbc_csr.port_1.cfg_oq.num_hdr_bytes(
        ELBA_GLOBAL_INTRINSIC_HDR_SZ + ELBA_P4_INTRINSIC_HDR_SZ);
    // Tell MAC to stomp CRC on error
    pbc_csr.port_1.cfg_oq.hw_error_to_pbus(1);

    if (tm_sw_init_enabled()) {
        pbc_csr.port_1.cfg_oq.enable(1);
        pbc_csr.port_1.cfg_oq.rewrite_enable(1);
        pbc_csr.port_1.cfg_oq.flow_control_enable_xoff(1);
    }
    data <<"pbc_csr.port_1.cfg_oq.all: 0x" << pbc_csr.port_1.cfg_oq.all() << endl;
data <<"pbc_csr.port_1.cfg_oq.enable: 0x" << pbc_csr.port_1.cfg_oq.enable() << endl;
data <<"pbc_csr.port_1.cfg_oq.num_hdr_bytes: 0x" << pbc_csr.port_1.cfg_oq.num_hdr_bytes() << endl;
data <<"pbc_csr.port_1.cfg_oq.num_hdr_bytes_crypto: 0x" << pbc_csr.port_1.cfg_oq.num_hdr_bytes_crypto() << endl;
data <<"pbc_csr.port_1.cfg_oq.span_cpu_num_cells: 0x" << pbc_csr.port_1.cfg_oq.span_cpu_num_cells() << endl;
data <<"pbc_csr.port_1.cfg_oq.flush_hw_error: 0x" << pbc_csr.port_1.cfg_oq.flush_hw_error() << endl;
data <<"pbc_csr.port_1.cfg_oq.eg_ts_enable: 0x" << pbc_csr.port_1.cfg_oq.eg_ts_enable() << endl;
data <<"pbc_csr.port_1.cfg_oq.ipg_bytes: 0x" << pbc_csr.port_1.cfg_oq.ipg_bytes() << endl;
data <<"pbc_csr.port_1.cfg_oq.flush: 0x" << pbc_csr.port_1.cfg_oq.flush() << endl;
data <<"pbc_csr.port_1.cfg_oq.flow_control_enable_xoff: 0x" << pbc_csr.port_1.cfg_oq.flow_control_enable_xoff() << endl;
data <<"pbc_csr.port_1.cfg_oq.hw_error_to_pbus: 0x" << pbc_csr.port_1.cfg_oq.hw_error_to_pbus() << endl;
data <<"pbc_csr.port_1.cfg_oq.rewrite_enable: 0x" << pbc_csr.port_1.cfg_oq.rewrite_enable() << endl;

    pbc_csr.port_1.cfg_oq.write();
    // ELBA_TM_PORT_UPLINK_2
    port_type = elba_tm_get_port_type(ELBA_TM_PORT_UPLINK_2);
    pbc_csr.port_2.cfg_oq.read();
    pbc_csr.port_2.cfg_oq.num_hdr_bytes(
        ELBA_GLOBAL_INTRINSIC_HDR_SZ + ELBA_P4_INTRINSIC_HDR_SZ);
    // Tell MAC to stomp CRC on error
    pbc_csr.port_2.cfg_oq.hw_error_to_pbus(1);

    if (tm_sw_init_enabled()) {
        pbc_csr.port_2.cfg_oq.enable(1);
        pbc_csr.port_2.cfg_oq.rewrite_enable(1);
        pbc_csr.port_2.cfg_oq.flow_control_enable_xoff(1);
    }
    data <<"pbc_csr.port_2.cfg_oq.all: 0x" << pbc_csr.port_2.cfg_oq.all() << endl;
data <<"pbc_csr.port_2.cfg_oq.enable: 0x" << pbc_csr.port_2.cfg_oq.enable() << endl;
data <<"pbc_csr.port_2.cfg_oq.num_hdr_bytes: 0x" << pbc_csr.port_2.cfg_oq.num_hdr_bytes() << endl;
data <<"pbc_csr.port_2.cfg_oq.num_hdr_bytes_crypto: 0x" << pbc_csr.port_2.cfg_oq.num_hdr_bytes_crypto() << endl;
data <<"pbc_csr.port_2.cfg_oq.span_cpu_num_cells: 0x" << pbc_csr.port_2.cfg_oq.span_cpu_num_cells() << endl;
data <<"pbc_csr.port_2.cfg_oq.flush_hw_error: 0x" << pbc_csr.port_2.cfg_oq.flush_hw_error() << endl;
data <<"pbc_csr.port_2.cfg_oq.eg_ts_enable: 0x" << pbc_csr.port_2.cfg_oq.eg_ts_enable() << endl;
data <<"pbc_csr.port_2.cfg_oq.ipg_bytes: 0x" << pbc_csr.port_2.cfg_oq.ipg_bytes() << endl;
data <<"pbc_csr.port_2.cfg_oq.flush: 0x" << pbc_csr.port_2.cfg_oq.flush() << endl;
data <<"pbc_csr.port_2.cfg_oq.flow_control_enable_xoff: 0x" << pbc_csr.port_2.cfg_oq.flow_control_enable_xoff() << endl;
data <<"pbc_csr.port_2.cfg_oq.hw_error_to_pbus: 0x" << pbc_csr.port_2.cfg_oq.hw_error_to_pbus() << endl;
data <<"pbc_csr.port_2.cfg_oq.rewrite_enable: 0x" << pbc_csr.port_2.cfg_oq.rewrite_enable() << endl;

    pbc_csr.port_2.cfg_oq.write();
    // ELBA_TM_PORT_UPLINK_3
    port_type = elba_tm_get_port_type(ELBA_TM_PORT_UPLINK_3);
    pbc_csr.port_3.cfg_oq.read();
    pbc_csr.port_3.cfg_oq.num_hdr_bytes(
        ELBA_GLOBAL_INTRINSIC_HDR_SZ + ELBA_P4_INTRINSIC_HDR_SZ);
    // Tell MAC to stomp CRC on error
    pbc_csr.port_3.cfg_oq.hw_error_to_pbus(1);

    if (tm_sw_init_enabled()) {
        pbc_csr.port_3.cfg_oq.enable(1);
        pbc_csr.port_3.cfg_oq.rewrite_enable(1);
        pbc_csr.port_3.cfg_oq.flow_control_enable_xoff(1);
    }
    data <<"pbc_csr.port_3.cfg_oq.all: 0x" << pbc_csr.port_3.cfg_oq.all() << endl;
data <<"pbc_csr.port_3.cfg_oq.enable: 0x" << pbc_csr.port_3.cfg_oq.enable() << endl;
data <<"pbc_csr.port_3.cfg_oq.num_hdr_bytes: 0x" << pbc_csr.port_3.cfg_oq.num_hdr_bytes() << endl;
data <<"pbc_csr.port_3.cfg_oq.num_hdr_bytes_crypto: 0x" << pbc_csr.port_3.cfg_oq.num_hdr_bytes_crypto() << endl;
data <<"pbc_csr.port_3.cfg_oq.span_cpu_num_cells: 0x" << pbc_csr.port_3.cfg_oq.span_cpu_num_cells() << endl;
data <<"pbc_csr.port_3.cfg_oq.flush_hw_error: 0x" << pbc_csr.port_3.cfg_oq.flush_hw_error() << endl;
data <<"pbc_csr.port_3.cfg_oq.eg_ts_enable: 0x" << pbc_csr.port_3.cfg_oq.eg_ts_enable() << endl;
data <<"pbc_csr.port_3.cfg_oq.ipg_bytes: 0x" << pbc_csr.port_3.cfg_oq.ipg_bytes() << endl;
data <<"pbc_csr.port_3.cfg_oq.flush: 0x" << pbc_csr.port_3.cfg_oq.flush() << endl;
data <<"pbc_csr.port_3.cfg_oq.flow_control_enable_xoff: 0x" << pbc_csr.port_3.cfg_oq.flow_control_enable_xoff() << endl;
data <<"pbc_csr.port_3.cfg_oq.hw_error_to_pbus: 0x" << pbc_csr.port_3.cfg_oq.hw_error_to_pbus() << endl;
data <<"pbc_csr.port_3.cfg_oq.rewrite_enable: 0x" << pbc_csr.port_3.cfg_oq.rewrite_enable() << endl;

    pbc_csr.port_3.cfg_oq.write();

    SDK_TRACE_DEBUG("%s", data.str().c_str());
    return sdk::SDK_RET_OK;
}

static sdk_ret_t
elba_tm_init_pbc (elba_tm_buf_cfg_t *buf_cfg)
{
    sdk_ret_t ret = sdk::SDK_RET_OK;

    ret = elba_tm_alloc_pbc_buffers(buf_cfg);
    if (ret != sdk::SDK_RET_OK) {
        SDK_TRACE_ERR("Error allocating buffer configs %d", ret);
        return ret;
    }

    ret = elba_tm_port_program_defaults();
    if (ret != sdk::SDK_RET_OK) {
        SDK_TRACE_ERR("Error programming port defaults %d", ret);
        return ret;
    }

    ret = elba_tm_program_buffers(buf_cfg);
    if (ret != sdk::SDK_RET_OK) {
        SDK_TRACE_ERR("Error allocating buffer configs %d", ret);
        return ret;
    }

    return sdk::SDK_RET_OK;
}

static sdk_ret_t
elba_tm_init_hbm_q_map (void)
{
    sdk_ret_t ret = sdk::SDK_RET_OK;
    elb_top_csr_t &elb0 = ELB_BLK_REG_MODEL_ACCESS(elb_top_csr_t, 0, 0);
    elb_pbc_csr_t &pbc_csr = elb0.pb.pbc;
    elb_pf_csr_t &pf_csr = elb0.pf.pf;
    tm_port_type_e port_type;
    uint32_t p4_oq;
    cpp_int oq_map_val;
    cpp_int hbm_tc_to_q_val;
    elb_pbc_oq_map_t oq_map_decoder;
    oq_map_decoder.init();

    // Map traffic to the contexts
    // On uplink by default, map everything to context 0
    // On DMA port, map each iq to each context
    // ELBA_TM_PORT_UPLINK_0
    hbm_tc_to_q_val = 0;
    port_type = elba_tm_get_port_type(ELBA_TM_PORT_UPLINK_0);
    pf_csr.hbm_port_0.cfg_hbm_tc_to_q.read();
    pf_csr.hbm_port_0.cfg_hbm_tc_to_q.table(hbm_tc_to_q_val);

    if (tm_sw_init_enabled()) {
        elb_pbc_pg8_map_t pg_map_decoder;
        pg_map_decoder.init();
        pg_map_decoder.all(pf_csr.hbm_port_0.cfg_hbm_tc_to_q.table());
        pg_map_decoder.set_name("elb0.pb.pbc.hbm.hbm_port_0.cfg_hbm_tc_to_q.decoder");
        pg_map_decoder.show();

        stringstream data;
        data << hex << endl;
        data <<"pf_csr.hbm_port_0.cfg_hbm_tc_to_q.all: 0x" << pf_csr.hbm_port_0.cfg_hbm_tc_to_q.all() << endl;

        SDK_TRACE_DEBUG("%s", data.str().c_str());

        pf_csr.hbm_port_0.cfg_hbm_tc_to_q.write();
    }
    // ELBA_TM_PORT_UPLINK_1
    hbm_tc_to_q_val = 0;
    port_type = elba_tm_get_port_type(ELBA_TM_PORT_UPLINK_1);
    pf_csr.hbm_port_1.cfg_hbm_tc_to_q.read();
    pf_csr.hbm_port_1.cfg_hbm_tc_to_q.table(hbm_tc_to_q_val);

    if (tm_sw_init_enabled()) {
        elb_pbc_pg8_map_t pg_map_decoder;
        pg_map_decoder.init();
        pg_map_decoder.all(pf_csr.hbm_port_1.cfg_hbm_tc_to_q.table());
        pg_map_decoder.set_name("elb0.pb.pbc.hbm.hbm_port_1.cfg_hbm_tc_to_q.decoder");
        pg_map_decoder.show();

        stringstream data;
        data << hex << endl;
        data <<"pf_csr.hbm_port_1.cfg_hbm_tc_to_q.all: 0x" << pf_csr.hbm_port_1.cfg_hbm_tc_to_q.all() << endl;

        SDK_TRACE_DEBUG("%s", data.str().c_str());

        pf_csr.hbm_port_1.cfg_hbm_tc_to_q.write();
    }
    // ELBA_TM_PORT_UPLINK_2
    hbm_tc_to_q_val = 0;
    port_type = elba_tm_get_port_type(ELBA_TM_PORT_UPLINK_2);
    pf_csr.hbm_port_2.cfg_hbm_tc_to_q.read();
    pf_csr.hbm_port_2.cfg_hbm_tc_to_q.table(hbm_tc_to_q_val);

    if (tm_sw_init_enabled()) {
        elb_pbc_pg8_map_t pg_map_decoder;
        pg_map_decoder.init();
        pg_map_decoder.all(pf_csr.hbm_port_2.cfg_hbm_tc_to_q.table());
        pg_map_decoder.set_name("elb0.pb.pbc.hbm.hbm_port_2.cfg_hbm_tc_to_q.decoder");
        pg_map_decoder.show();

        stringstream data;
        data << hex << endl;
        data <<"pf_csr.hbm_port_2.cfg_hbm_tc_to_q.all: 0x" << pf_csr.hbm_port_2.cfg_hbm_tc_to_q.all() << endl;

        SDK_TRACE_DEBUG("%s", data.str().c_str());

        pf_csr.hbm_port_2.cfg_hbm_tc_to_q.write();
    }
    // ELBA_TM_PORT_UPLINK_3
    hbm_tc_to_q_val = 0;
    port_type = elba_tm_get_port_type(ELBA_TM_PORT_UPLINK_3);
    pf_csr.hbm_port_3.cfg_hbm_tc_to_q.read();
    pf_csr.hbm_port_3.cfg_hbm_tc_to_q.table(hbm_tc_to_q_val);

    if (tm_sw_init_enabled()) {
        elb_pbc_pg8_map_t pg_map_decoder;
        pg_map_decoder.init();
        pg_map_decoder.all(pf_csr.hbm_port_3.cfg_hbm_tc_to_q.table());
        pg_map_decoder.set_name("elb0.pb.pbc.hbm.hbm_port_3.cfg_hbm_tc_to_q.decoder");
        pg_map_decoder.show();

        stringstream data;
        data << hex << endl;
        data <<"pf_csr.hbm_port_3.cfg_hbm_tc_to_q.all: 0x" << pf_csr.hbm_port_3.cfg_hbm_tc_to_q.all() << endl;

        SDK_TRACE_DEBUG("%s", data.str().c_str());

        pf_csr.hbm_port_3.cfg_hbm_tc_to_q.write();
    }

    // Configure the parsers
    // ELBA_TM_PORT_UPLINK_0
    oq_map_val = 0;
    p4_oq = ELBA_TM_P4_UPLINK_IQ_OFFSET;
    port_type = elba_tm_get_port_type(ELBA_TM_PORT_UPLINK_0);

    pf_csr.hbm_port_0.cfg_hbm_parser.read();
    pf_csr.hbm_port_0.cfg_hbm_parser.use_dot1q(1);
    pf_csr.hbm_port_0.cfg_hbm_parser.use_ip(0);
    pf_csr.hbm_port_0.cfg_hbm_parser.default_cos(0);
    pf_csr.hbm_port_0.cfg_hbm_parser.dscp_map(0);

    for (unsigned tc = 0; tc < elba_tm_get_num_iqs_for_port_type(port_type); tc++) {
        cpp_helper.set_slc(oq_map_val, p4_oq, tc * 5, ((tc+1)*5)-1);
    }

    pbc_csr.cfg_parser0.default_cos(0);
    pbc_csr.cfg_parser0.oq_map(oq_map_val);

    oq_map_decoder.all(pbc_csr.cfg_parser0.oq_map());
    oq_map_decoder.set_name("elb0.pb.pbc.cfg_parser0.decoder");

    if (tm_sw_init_enabled()) {
        stringstream data;
        data << hex << endl;
        data <<"pf_csr.hbm_port_0.cfg_hbm_parser.all: 0x" << pf_csr.hbm_port_0.cfg_hbm_parser.all() << endl;

        data <<"pbc_csr.cfg_parser0.all: 0x" << pbc_csr.cfg_parser0.all() << endl;
data <<"pbc_csr.cfg_parser0.default_cos: 0x" << pbc_csr.cfg_parser0.default_cos() << endl;
data <<"pbc_csr.cfg_parser0.oq_map: 0x" << pbc_csr.cfg_parser0.oq_map() << endl;
data <<"pbc_csr.cfg_parser0.default_port: 0x" << pbc_csr.cfg_parser0.default_port() << endl;

        SDK_TRACE_DEBUG("%s", data.str().c_str());

        pf_csr.hbm_port_0.cfg_hbm_parser.write();
        pbc_csr.cfg_parser0.write();
    }
    // ELBA_TM_PORT_UPLINK_1
    oq_map_val = 0;
    p4_oq = ELBA_TM_P4_UPLINK_IQ_OFFSET;
    port_type = elba_tm_get_port_type(ELBA_TM_PORT_UPLINK_1);

    pf_csr.hbm_port_1.cfg_hbm_parser.read();
    pf_csr.hbm_port_1.cfg_hbm_parser.use_dot1q(1);
    pf_csr.hbm_port_1.cfg_hbm_parser.use_ip(0);
    pf_csr.hbm_port_1.cfg_hbm_parser.default_cos(0);
    pf_csr.hbm_port_1.cfg_hbm_parser.dscp_map(0);

    for (unsigned tc = 0; tc < elba_tm_get_num_iqs_for_port_type(port_type); tc++) {
        cpp_helper.set_slc(oq_map_val, p4_oq, tc * 5, ((tc+1)*5)-1);
    }

    pbc_csr.cfg_parser1.default_cos(0);
    pbc_csr.cfg_parser1.oq_map(oq_map_val);

    oq_map_decoder.all(pbc_csr.cfg_parser1.oq_map());
    oq_map_decoder.set_name("elb0.pb.pbc.cfg_parser1.decoder");

    if (tm_sw_init_enabled()) {
        stringstream data;
        data << hex << endl;
        data <<"pf_csr.hbm_port_1.cfg_hbm_parser.all: 0x" << pf_csr.hbm_port_1.cfg_hbm_parser.all() << endl;

        data <<"pbc_csr.cfg_parser1.all: 0x" << pbc_csr.cfg_parser1.all() << endl;
data <<"pbc_csr.cfg_parser1.default_cos: 0x" << pbc_csr.cfg_parser1.default_cos() << endl;
data <<"pbc_csr.cfg_parser1.oq_map: 0x" << pbc_csr.cfg_parser1.oq_map() << endl;
data <<"pbc_csr.cfg_parser1.default_port: 0x" << pbc_csr.cfg_parser1.default_port() << endl;

        SDK_TRACE_DEBUG("%s", data.str().c_str());

        pf_csr.hbm_port_1.cfg_hbm_parser.write();
        pbc_csr.cfg_parser1.write();
    }
    // ELBA_TM_PORT_UPLINK_2
    oq_map_val = 0;
    p4_oq = ELBA_TM_P4_UPLINK_IQ_OFFSET;
    port_type = elba_tm_get_port_type(ELBA_TM_PORT_UPLINK_2);

    pf_csr.hbm_port_2.cfg_hbm_parser.read();
    pf_csr.hbm_port_2.cfg_hbm_parser.use_dot1q(1);
    pf_csr.hbm_port_2.cfg_hbm_parser.use_ip(0);
    pf_csr.hbm_port_2.cfg_hbm_parser.default_cos(0);
    pf_csr.hbm_port_2.cfg_hbm_parser.dscp_map(0);

    for (unsigned tc = 0; tc < elba_tm_get_num_iqs_for_port_type(port_type); tc++) {
        cpp_helper.set_slc(oq_map_val, p4_oq, tc * 5, ((tc+1)*5)-1);
    }

    pbc_csr.cfg_parser2.default_cos(0);
    pbc_csr.cfg_parser2.oq_map(oq_map_val);

    oq_map_decoder.all(pbc_csr.cfg_parser2.oq_map());
    oq_map_decoder.set_name("elb0.pb.pbc.cfg_parser2.decoder");

    if (tm_sw_init_enabled()) {
        stringstream data;
        data << hex << endl;
        data <<"pf_csr.hbm_port_2.cfg_hbm_parser.all: 0x" << pf_csr.hbm_port_2.cfg_hbm_parser.all() << endl;

        data <<"pbc_csr.cfg_parser2.all: 0x" << pbc_csr.cfg_parser2.all() << endl;
data <<"pbc_csr.cfg_parser2.default_cos: 0x" << pbc_csr.cfg_parser2.default_cos() << endl;
data <<"pbc_csr.cfg_parser2.oq_map: 0x" << pbc_csr.cfg_parser2.oq_map() << endl;
data <<"pbc_csr.cfg_parser2.default_port: 0x" << pbc_csr.cfg_parser2.default_port() << endl;

        SDK_TRACE_DEBUG("%s", data.str().c_str());

        pf_csr.hbm_port_2.cfg_hbm_parser.write();
        pbc_csr.cfg_parser2.write();
    }
    // ELBA_TM_PORT_UPLINK_3
    oq_map_val = 0;
    p4_oq = ELBA_TM_P4_UPLINK_IQ_OFFSET;
    port_type = elba_tm_get_port_type(ELBA_TM_PORT_UPLINK_3);

    pf_csr.hbm_port_3.cfg_hbm_parser.read();
    pf_csr.hbm_port_3.cfg_hbm_parser.use_dot1q(1);
    pf_csr.hbm_port_3.cfg_hbm_parser.use_ip(0);
    pf_csr.hbm_port_3.cfg_hbm_parser.default_cos(0);
    pf_csr.hbm_port_3.cfg_hbm_parser.dscp_map(0);

    for (unsigned tc = 0; tc < elba_tm_get_num_iqs_for_port_type(port_type); tc++) {
        cpp_helper.set_slc(oq_map_val, p4_oq, tc * 5, ((tc+1)*5)-1);
    }

    pbc_csr.cfg_parser3.default_cos(0);
    pbc_csr.cfg_parser3.oq_map(oq_map_val);

    oq_map_decoder.all(pbc_csr.cfg_parser3.oq_map());
    oq_map_decoder.set_name("elb0.pb.pbc.cfg_parser3.decoder");

    if (tm_sw_init_enabled()) {
        stringstream data;
        data << hex << endl;
        data <<"pf_csr.hbm_port_3.cfg_hbm_parser.all: 0x" << pf_csr.hbm_port_3.cfg_hbm_parser.all() << endl;

        data <<"pbc_csr.cfg_parser3.all: 0x" << pbc_csr.cfg_parser3.all() << endl;
data <<"pbc_csr.cfg_parser3.default_cos: 0x" << pbc_csr.cfg_parser3.default_cos() << endl;
data <<"pbc_csr.cfg_parser3.oq_map: 0x" << pbc_csr.cfg_parser3.oq_map() << endl;
data <<"pbc_csr.cfg_parser3.default_port: 0x" << pbc_csr.cfg_parser3.default_port() << endl;

        SDK_TRACE_DEBUG("%s", data.str().c_str());

        pf_csr.hbm_port_3.cfg_hbm_parser.write();
        pbc_csr.cfg_parser3.write();
    }

    return ret;
}

static sdk_ret_t
elba_tm_init_hbm (elba_tm_buf_cfg_t *buf_cfg)
{
    sdk_ret_t ret = sdk::SDK_RET_OK;
    elb_top_csr_t &elb0 = ELB_BLK_REG_MODEL_ACCESS(elb_top_csr_t, 0, 0);
    elb_pbc_csr_t &pbc_csr = elb0.pb.pbc;
    elb_pf_csr_t &pf_csr = elb0.pf.pf;

    ret = elba_tm_alloc_hbm_buffers(buf_cfg);
    if (ret != sdk::SDK_RET_OK) {
        SDK_TRACE_ERR("Error allocating hbm buffers ret %d",
                      ret);
        return ret;
    }

    // Program the HBM buffers
    ret = elba_tm_program_hbm_buffers(buf_cfg);
    if (ret != sdk::SDK_RET_OK) {
        SDK_TRACE_ERR("Error programming hbm buffers ret %d",
                      ret);
        return ret;
    }

    ret = elba_tm_init_hbm_q_map();
    if (ret != sdk::SDK_RET_OK) {
        SDK_TRACE_ERR("Error programming hbm q mapping ret %d",
                      ret);
        return ret;
    }

    uint32_t rate_limiter;
    stringstream data;
    data << hex << endl;
    // On configure rate-limiter from HBM
    // ELBA_TM_PORT_UPLINK_0
    if (elba_tm_port_is_uplink_port(ELBA_TM_PORT_UPLINK_0)) {
        rate_limiter = (tm_cfg_profile()->num_active_uplink_ports * 2) - 1;
    } else {
        rate_limiter = 3;
    }

    pf_csr.hbm_port_0.cfg_hbm.read();
    pf_csr.hbm_port_0.cfg_hbm.rate_limiter(rate_limiter);

    pf_csr.hbm_port_0.cfg_hbm_read_fifo.read();
    pf_csr.hbm_port_0.cfg_hbm_read_fifo.max_cells(0x78);
    pf_csr.hbm_port_0.cfg_hbm_read_fifo.max_flits(0x78);
    pf_csr.hbm_port_0.cfg_hbm_read_fifo.max_packets(0x28);

    if (tm_sw_init_enabled()) {
        data <<"pf_csr.hbm_port_0.cfg_hbm.all: 0x" << pf_csr.hbm_port_0.cfg_hbm.all() << endl;

        pf_csr.hbm_port_0.cfg_hbm.write();
        // TODO: this is not written in elb_pb_api.cc
        // pf_csr.hbm_port_0.cfg_hbm_read_fifo.write();
    }

    // ELBA_TM_PORT_UPLINK_1
    if (elba_tm_port_is_uplink_port(ELBA_TM_PORT_UPLINK_1)) {
        rate_limiter = (tm_cfg_profile()->num_active_uplink_ports * 2) - 1;
    } else {
        rate_limiter = 3;
    }

    pf_csr.hbm_port_1.cfg_hbm.read();
    pf_csr.hbm_port_1.cfg_hbm.rate_limiter(rate_limiter);

    pf_csr.hbm_port_1.cfg_hbm_read_fifo.read();
    pf_csr.hbm_port_1.cfg_hbm_read_fifo.max_cells(0x78);
    pf_csr.hbm_port_1.cfg_hbm_read_fifo.max_flits(0x78);
    pf_csr.hbm_port_1.cfg_hbm_read_fifo.max_packets(0x28);

    if (tm_sw_init_enabled()) {
        data <<"pf_csr.hbm_port_1.cfg_hbm.all: 0x" << pf_csr.hbm_port_1.cfg_hbm.all() << endl;

        pf_csr.hbm_port_1.cfg_hbm.write();
        // TODO: this is not written in elb_pb_api.cc
        // pf_csr.hbm_port_1.cfg_hbm_read_fifo.write();
    }

    // ELBA_TM_PORT_UPLINK_2
    if (elba_tm_port_is_uplink_port(ELBA_TM_PORT_UPLINK_2)) {
        rate_limiter = (tm_cfg_profile()->num_active_uplink_ports * 2) - 1;
    } else {
        rate_limiter = 3;
    }

    pf_csr.hbm_port_2.cfg_hbm.read();
    pf_csr.hbm_port_2.cfg_hbm.rate_limiter(rate_limiter);

    pf_csr.hbm_port_2.cfg_hbm_read_fifo.read();
    pf_csr.hbm_port_2.cfg_hbm_read_fifo.max_cells(0x78);
    pf_csr.hbm_port_2.cfg_hbm_read_fifo.max_flits(0x78);
    pf_csr.hbm_port_2.cfg_hbm_read_fifo.max_packets(0x28);

    if (tm_sw_init_enabled()) {
        data <<"pf_csr.hbm_port_2.cfg_hbm.all: 0x" << pf_csr.hbm_port_2.cfg_hbm.all() << endl;

        pf_csr.hbm_port_2.cfg_hbm.write();
        // TODO: this is not written in elb_pb_api.cc
        // pf_csr.hbm_port_2.cfg_hbm_read_fifo.write();
    }

    // ELBA_TM_PORT_UPLINK_3
    if (elba_tm_port_is_uplink_port(ELBA_TM_PORT_UPLINK_3)) {
        rate_limiter = (tm_cfg_profile()->num_active_uplink_ports * 2) - 1;
    } else {
        rate_limiter = 3;
    }

    pf_csr.hbm_port_3.cfg_hbm.read();
    pf_csr.hbm_port_3.cfg_hbm.rate_limiter(rate_limiter);

    pf_csr.hbm_port_3.cfg_hbm_read_fifo.read();
    pf_csr.hbm_port_3.cfg_hbm_read_fifo.max_cells(0x78);
    pf_csr.hbm_port_3.cfg_hbm_read_fifo.max_flits(0x78);
    pf_csr.hbm_port_3.cfg_hbm_read_fifo.max_packets(0x28);

    if (tm_sw_init_enabled()) {
        data <<"pf_csr.hbm_port_3.cfg_hbm.all: 0x" << pf_csr.hbm_port_3.cfg_hbm.all() << endl;

        pf_csr.hbm_port_3.cfg_hbm.write();
        // TODO: this is not written in elb_pb_api.cc
        // pf_csr.hbm_port_3.cfg_hbm_read_fifo.write();
    }


    pf_csr.cfg_hbm_wb.read();
    pf_csr.cfg_hbm_rb.read();
    pf_csr.cfg_hbm_cut_thru.read();

    pf_csr.cfg_hbm_rb.enable_wrr(0);

    if (tm_sw_init_enabled()) {
        // TODO: this is not written in elb_pb_api.cc
        // pf_csr.cfg_hbm_wb.write();
        data <<"pf_csr.cfg_hbm_rb.all: 0x" << pf_csr.cfg_hbm_rb.all() << endl;

        pf_csr.cfg_hbm_rb.write();
        // pf_csr.cfg_hbm_cut_thru.write();
    }

    // AXI Base Address
    pbc_csr.cfg_axi.read();
    pbc_csr.cfg_axi.base_addr(g_elba_state_pd->mempartition()->base());
    data <<"pbc_csr.cfg_axi.all: 0x" << pbc_csr.cfg_axi.all() << endl;
data <<"pbc_csr.cfg_axi.base_addr: 0x" << pbc_csr.cfg_axi.base_addr() << endl;

    pbc_csr.cfg_axi.write();

    // AXI Base for buffer FIFOs
    pf_csr.cfg_hbm_axi_base.read();
    pf_csr.cfg_hbm_axi_base.addr(0x1300000000);
    data <<"pf_csr.cfg_hbm_axi_base.all: 0x" << pf_csr.cfg_hbm_axi_base.all() << endl;

    pf_csr.cfg_hbm_axi_base.write();

    SDK_TRACE_DEBUG("%s", data.str().c_str());
    return sdk::SDK_RET_OK;
}

sdk_ret_t
elba_tm_port_program_uplink_byte_count (void)
{
    elb_top_csr_t &elb0 = ELB_BLK_REG_MODEL_ACCESS(elb_top_csr_t, 0, 0);
    elb_pbc_csr_t &pbc_csr = elb0.pb.pbc;

    stringstream data;
    data << hex << endl;
    // ELBA_TM_PORT_UPLINK_0
    if (elba_tm_port_is_uplink_port(ELBA_TM_PORT_UPLINK_0)) {
 
    pbc_csr.port_0.cfg_oq.read();
    pbc_csr.port_0.cfg_oq.num_hdr_bytes(
            ELBA_GLOBAL_INTRINSIC_HDR_SZ + ELBA_P4_INTRINSIC_HDR_SZ);
    data <<"pbc_csr.port_0.cfg_oq.all: 0x" << pbc_csr.port_0.cfg_oq.all() << endl;
    data <<"pbc_csr.port_0.cfg_oq.enable: 0x" << pbc_csr.port_0.cfg_oq.enable() << endl;
    data <<"pbc_csr.port_0.cfg_oq.num_hdr_bytes: 0x" << pbc_csr.port_0.cfg_oq.num_hdr_bytes() << endl;
    data <<"pbc_csr.port_0.cfg_oq.num_hdr_bytes_crypto: 0x" << pbc_csr.port_0.cfg_oq.num_hdr_bytes_crypto() << endl;
    data <<"pbc_csr.port_0.cfg_oq.flush_hw_error: 0x" << pbc_csr.port_0.cfg_oq.flush_hw_error() << endl;
    data <<"pbc_csr.port_0.cfg_oq.eg_ts_enable: 0x" << pbc_csr.port_0.cfg_oq.eg_ts_enable() << endl;
    data <<"pbc_csr.port_0.cfg_oq.ipg_bytes: 0x" << pbc_csr.port_0.cfg_oq.ipg_bytes() << endl;
    data <<"pbc_csr.port_0.cfg_oq.flush: 0x" << pbc_csr.port_0.cfg_oq.flush() << endl;
    data <<"pbc_csr.port_0.cfg_oq.flow_control_enable_xoff: 0x" << pbc_csr.port_0.cfg_oq.flow_control_enable_xoff() << endl;
    data <<"pbc_csr.port_0.cfg_oq.hw_error_to_pbus: 0x" << pbc_csr.port_0.cfg_oq.hw_error_to_pbus() << endl;
    data <<"pbc_csr.port_0.cfg_oq.rewrite_enable: 0x" << pbc_csr.port_0.cfg_oq.rewrite_enable() << endl;

    pbc_csr.port_0.cfg_oq.write();
    }

    // ELBA_TM_PORT_UPLINK_1
    if (elba_tm_port_is_uplink_port(ELBA_TM_PORT_UPLINK_1)) {
 
    pbc_csr.port_1.cfg_oq.read();
    pbc_csr.port_1.cfg_oq.num_hdr_bytes(
            ELBA_GLOBAL_INTRINSIC_HDR_SZ + ELBA_P4_INTRINSIC_HDR_SZ);
    data <<"pbc_csr.port_1.cfg_oq.all: 0x" << pbc_csr.port_1.cfg_oq.all() << endl;
    data <<"pbc_csr.port_1.cfg_oq.enable: 0x" << pbc_csr.port_1.cfg_oq.enable() << endl;
    data <<"pbc_csr.port_1.cfg_oq.num_hdr_bytes: 0x" << pbc_csr.port_1.cfg_oq.num_hdr_bytes() << endl;
    data <<"pbc_csr.port_1.cfg_oq.num_hdr_bytes_crypto: 0x" << pbc_csr.port_1.cfg_oq.num_hdr_bytes_crypto() << endl;
    data <<"pbc_csr.port_1.cfg_oq.flush_hw_error: 0x" << pbc_csr.port_1.cfg_oq.flush_hw_error() << endl;
    data <<"pbc_csr.port_1.cfg_oq.eg_ts_enable: 0x" << pbc_csr.port_1.cfg_oq.eg_ts_enable() << endl;
    data <<"pbc_csr.port_1.cfg_oq.ipg_bytes: 0x" << pbc_csr.port_1.cfg_oq.ipg_bytes() << endl;
    data <<"pbc_csr.port_1.cfg_oq.flush: 0x" << pbc_csr.port_1.cfg_oq.flush() << endl;
    data <<"pbc_csr.port_1.cfg_oq.flow_control_enable_xoff: 0x" << pbc_csr.port_1.cfg_oq.flow_control_enable_xoff() << endl;
    data <<"pbc_csr.port_1.cfg_oq.hw_error_to_pbus: 0x" << pbc_csr.port_1.cfg_oq.hw_error_to_pbus() << endl;
    data <<"pbc_csr.port_1.cfg_oq.rewrite_enable: 0x" << pbc_csr.port_1.cfg_oq.rewrite_enable() << endl;

    pbc_csr.port_1.cfg_oq.write();
    }

    // ELBA_TM_PORT_UPLINK_2
    if (elba_tm_port_is_uplink_port(ELBA_TM_PORT_UPLINK_2)) {
 
    pbc_csr.port_2.cfg_oq.read();
    pbc_csr.port_2.cfg_oq.num_hdr_bytes(
            ELBA_GLOBAL_INTRINSIC_HDR_SZ + ELBA_P4_INTRINSIC_HDR_SZ);
    data <<"pbc_csr.port_2.cfg_oq.all: 0x" << pbc_csr.port_2.cfg_oq.all() << endl;
    data <<"pbc_csr.port_2.cfg_oq.enable: 0x" << pbc_csr.port_2.cfg_oq.enable() << endl;
    data <<"pbc_csr.port_2.cfg_oq.num_hdr_bytes: 0x" << pbc_csr.port_2.cfg_oq.num_hdr_bytes() << endl;
    data <<"pbc_csr.port_2.cfg_oq.num_hdr_bytes_crypto: 0x" << pbc_csr.port_2.cfg_oq.num_hdr_bytes_crypto() << endl;
    data <<"pbc_csr.port_2.cfg_oq.flush_hw_error: 0x" << pbc_csr.port_2.cfg_oq.flush_hw_error() << endl;
    data <<"pbc_csr.port_2.cfg_oq.eg_ts_enable: 0x" << pbc_csr.port_2.cfg_oq.eg_ts_enable() << endl;
    data <<"pbc_csr.port_2.cfg_oq.ipg_bytes: 0x" << pbc_csr.port_2.cfg_oq.ipg_bytes() << endl;
    data <<"pbc_csr.port_2.cfg_oq.flush: 0x" << pbc_csr.port_2.cfg_oq.flush() << endl;
    data <<"pbc_csr.port_2.cfg_oq.flow_control_enable_xoff: 0x" << pbc_csr.port_2.cfg_oq.flow_control_enable_xoff() << endl;
    data <<"pbc_csr.port_2.cfg_oq.hw_error_to_pbus: 0x" << pbc_csr.port_2.cfg_oq.hw_error_to_pbus() << endl;
    data <<"pbc_csr.port_2.cfg_oq.rewrite_enable: 0x" << pbc_csr.port_2.cfg_oq.rewrite_enable() << endl;

    pbc_csr.port_2.cfg_oq.write();
    }

    // ELBA_TM_PORT_UPLINK_3
    if (elba_tm_port_is_uplink_port(ELBA_TM_PORT_UPLINK_3)) {
 
    pbc_csr.port_3.cfg_oq.read();
    pbc_csr.port_3.cfg_oq.num_hdr_bytes(
            ELBA_GLOBAL_INTRINSIC_HDR_SZ + ELBA_P4_INTRINSIC_HDR_SZ);
    data <<"pbc_csr.port_3.cfg_oq.all: 0x" << pbc_csr.port_3.cfg_oq.all() << endl;
    data <<"pbc_csr.port_3.cfg_oq.enable: 0x" << pbc_csr.port_3.cfg_oq.enable() << endl;
    data <<"pbc_csr.port_3.cfg_oq.num_hdr_bytes: 0x" << pbc_csr.port_3.cfg_oq.num_hdr_bytes() << endl;
    data <<"pbc_csr.port_3.cfg_oq.num_hdr_bytes_crypto: 0x" << pbc_csr.port_3.cfg_oq.num_hdr_bytes_crypto() << endl;
    data <<"pbc_csr.port_3.cfg_oq.flush_hw_error: 0x" << pbc_csr.port_3.cfg_oq.flush_hw_error() << endl;
    data <<"pbc_csr.port_3.cfg_oq.eg_ts_enable: 0x" << pbc_csr.port_3.cfg_oq.eg_ts_enable() << endl;
    data <<"pbc_csr.port_3.cfg_oq.ipg_bytes: 0x" << pbc_csr.port_3.cfg_oq.ipg_bytes() << endl;
    data <<"pbc_csr.port_3.cfg_oq.flush: 0x" << pbc_csr.port_3.cfg_oq.flush() << endl;
    data <<"pbc_csr.port_3.cfg_oq.flow_control_enable_xoff: 0x" << pbc_csr.port_3.cfg_oq.flow_control_enable_xoff() << endl;
    data <<"pbc_csr.port_3.cfg_oq.hw_error_to_pbus: 0x" << pbc_csr.port_3.cfg_oq.hw_error_to_pbus() << endl;
    data <<"pbc_csr.port_3.cfg_oq.rewrite_enable: 0x" << pbc_csr.port_3.cfg_oq.rewrite_enable() << endl;

    pbc_csr.port_3.cfg_oq.write();
    }


    SDK_TRACE_DEBUG("%s", data.str().c_str());
    return sdk::SDK_RET_OK;
}

sdk_ret_t
elba_pf_init (void)
{
    elb_top_csr_t &elb0 = ELB_BLK_REG_MODEL_ACCESS(elb_top_csr_t, 0, 0);
    elb_pbc_csr_t &pbc_csr = elb0.pb.pbc;
    elb_pf_csr_t &pf_csr = elb0.pf.pf;
    //uint32_t rate_limiter;
    stringstream data;
    data << hex << endl;

    pf_rslt_decoder_t def_rslt;
    def_rslt.all(0);
    def_rslt.out_pb(1);
    def_rslt.pb_port(7);

    pf_csr.hbm_port_0.cfg_hbm_parser_cam_miss.all(def_rslt.all());
    pf_csr.hbm_port_0.cfg_hbm_parser_cam_miss.write();
    pf_csr.hbm_port_1.cfg_hbm_parser_cam_miss.all(def_rslt.all());
    pf_csr.hbm_port_1.cfg_hbm_parser_cam_miss.write();
    pf_csr.hbm_port_2.cfg_hbm_parser_cam_miss.all(def_rslt.all());
    pf_csr.hbm_port_2.cfg_hbm_parser_cam_miss.write();
    pf_csr.hbm_port_3.cfg_hbm_parser_cam_miss.all(def_rslt.all());
    pf_csr.hbm_port_3.cfg_hbm_parser_cam_miss.write();



    // On configure rate-limiter from HBM

    pf_csr.hbm_port_0.cfg_hbm.read();
    //pf_csr.hbm_port_0.cfg_hbm.rate_limiter(rate_limiter);

    pf_csr.hbm_port_0.cfg_hbm_read_fifo.read();
    pf_csr.hbm_port_0.cfg_hbm_read_fifo.max_cells(0x78);
    pf_csr.hbm_port_0.cfg_hbm_read_fifo.max_flits(0x78);
    pf_csr.hbm_port_0.cfg_hbm_read_fifo.max_packets(0x28);

    pf_csr.hbm_port_1.cfg_hbm.read();
    //pf_csr.hbm_port_1.cfg_hbm.rate_limiter(rate_limiter);

    pf_csr.hbm_port_1.cfg_hbm_read_fifo.read();
    pf_csr.hbm_port_1.cfg_hbm_read_fifo.max_cells(0x78);
    pf_csr.hbm_port_1.cfg_hbm_read_fifo.max_flits(0x78);
    pf_csr.hbm_port_1.cfg_hbm_read_fifo.max_packets(0x28);

    pf_csr.hbm_port_2.cfg_hbm.read();
    //pf_csr.hbm_port_2.cfg_hbm.rate_limiter(rate_limiter);

    pf_csr.hbm_port_2.cfg_hbm_read_fifo.read();
    pf_csr.hbm_port_2.cfg_hbm_read_fifo.max_cells(0x78);
    pf_csr.hbm_port_2.cfg_hbm_read_fifo.max_flits(0x78);
    pf_csr.hbm_port_2.cfg_hbm_read_fifo.max_packets(0x28);

    pf_csr.hbm_port_3.cfg_hbm.read();
    //pf_csr.hbm_port_3.cfg_hbm.rate_limiter(rate_limiter);

    pf_csr.hbm_port_3.cfg_hbm_read_fifo.read();
    pf_csr.hbm_port_3.cfg_hbm_read_fifo.max_cells(0x78);
    pf_csr.hbm_port_3.cfg_hbm_read_fifo.max_flits(0x78);
    pf_csr.hbm_port_3.cfg_hbm_read_fifo.max_packets(0x28);

    pf_csr.cfg_hbm_wb.read();
    pf_csr.cfg_hbm_rb.read();
    pf_csr.cfg_hbm_cut_thru.read();

    pf_csr.cfg_hbm_rb.enable_wrr(0);


    // AXI Base Address
    pbc_csr.cfg_axi.read();
    pbc_csr.cfg_axi.base_addr(g_elba_state_pd->mempartition()->base());
    data <<"pbc_csr.cfg_axi.all: 0x" << pbc_csr.cfg_axi.all() << endl;
data <<"pbc_csr.cfg_axi.base_addr: 0x" << pbc_csr.cfg_axi.base_addr() << endl;

    pbc_csr.cfg_axi.write();

    // AXI Base for buffer FIFOs
    pf_csr.cfg_hbm_axi_base.read();
    pf_csr.cfg_hbm_axi_base.addr(0x1300000000);
    data <<"pf_csr.cfg_hbm_axi_base.all: 0x" << pf_csr.cfg_hbm_axi_base.all() << endl;

    pf_csr.cfg_hbm_axi_base.write();

    SDK_TRACE_DEBUG("%s", data.str().c_str());
    return sdk::SDK_RET_OK;
}


static sdk_ret_t
elba_tm_init_ports (void)
{
    sdk_ret_t ret = sdk::SDK_RET_OK;
    elba_tm_buf_cfg_t buf_cfg = {0};

    ret = elba_tm_init_pbc(&buf_cfg);
    if (ret != sdk::SDK_RET_OK) {
        SDK_TRACE_ERR("Error initing pbc %d", ret);
        return ret;
    }

    ret = elba_tm_init_hbm(&buf_cfg);
    if (ret != sdk::SDK_RET_OK) {
        SDK_TRACE_ERR("Error initing hbm %d", ret);
        return ret;
    }

    tm_ctx()->buf_cfg = buf_cfg;

    return ret;
}

static sdk_ret_t
elba_tm_global_init (void)
{
    // Init the FC mgr and RC
    //
    // There are only 2 combinations valid with min_cell, max_row as either
    // 0, 4095 or 4096, 2559 . Choose one of them based on the island which
    // needs more cells
    uint32_t min_cells[] = { 0, 4096};
    uint32_t max_row[] = {4095, 2559};
//    uint32_t sched_timer;
    elb_top_csr_t &elb0 = ELB_BLK_REG_MODEL_ACCESS(elb_top_csr_t, 0, 0);
    elb_pbc_csr_t &pbc_csr = elb0.pb.pbc;
    elb_pf_csr_t &pf_csr = elb0.pf.pf;

    pf_rslt_decoder_t def_rslt;
    def_rslt.all(0);
    def_rslt.out_pb(1);
    def_rslt.pb_port(7);

    pf_csr.hbm_port_0.cfg_hbm_parser_cam_miss.all(def_rslt.all());
    pf_csr.hbm_port_0.cfg_hbm_parser_cam_miss.write();
    pf_csr.hbm_port_1.cfg_hbm_parser_cam_miss.all(def_rslt.all());
    pf_csr.hbm_port_1.cfg_hbm_parser_cam_miss.write();
    pf_csr.hbm_port_2.cfg_hbm_parser_cam_miss.all(def_rslt.all());
    pf_csr.hbm_port_2.cfg_hbm_parser_cam_miss.write();
    pf_csr.hbm_port_3.cfg_hbm_parser_cam_miss.all(def_rslt.all());
    pf_csr.hbm_port_3.cfg_hbm_parser_cam_miss.write();

    pf_csr.cfg_hbm_eth_ctrl_init.head_start(1);
    pf_csr.cfg_hbm_eth_ctrl_init.tail_start(1);
    if (tm_sw_init_enabled()) {
        stringstream data;
        data << hex << endl;
        data <<"pf_csr.cfg_hbm_eth_ctrl_init.all: 0x" << pf_csr.cfg_hbm_eth_ctrl_init.all() << endl;

        SDK_TRACE_DEBUG("%s", data.str().c_str());
        pf_csr.cfg_hbm_eth_ctrl_init.write();
    }

    pf_csr.cfg_hbm_eth_ctrl_init.head_start(0);
    pf_csr.cfg_hbm_eth_ctrl_init.tail_start(0);
    if (tm_sw_init_enabled()) {
        stringstream data;
        data << hex << endl;
        data <<"pf_csr.cfg_hbm_eth_ctrl_init.all: 0x" << pf_csr.cfg_hbm_eth_ctrl_init.all() << endl;

        SDK_TRACE_DEBUG("%s", data.str().c_str());
        pf_csr.cfg_hbm_eth_ctrl_init.write();
    }


    pbc_csr.cfg_fc_mgr_0.init_start(1);
    pbc_csr.cfg_fc_mgr_0.init_reset(0);
    pbc_csr.cfg_fc_mgr_1.init_start(1);
    pbc_csr.cfg_fc_mgr_1.init_reset(0);
    if (elba_tm_get_max_cell_chunks_for_island(0) >
        elba_tm_get_max_cell_chunks_for_island(1)) {
        pbc_csr.cfg_fc_mgr_0.max_row(max_row[0]);
        pbc_csr.cfg_fc_mgr_0.min_cell(min_cells[0]);
        pbc_csr.cfg_fc_mgr_1.max_row(max_row[1]);
        pbc_csr.cfg_fc_mgr_1.min_cell(min_cells[1]);
        pbc_csr.cfg_island_control.map(0);
    } else {
        pbc_csr.cfg_fc_mgr_0.max_row(max_row[1]);
        pbc_csr.cfg_fc_mgr_0.min_cell(min_cells[1]);
        pbc_csr.cfg_fc_mgr_1.max_row(max_row[0]);
        pbc_csr.cfg_fc_mgr_1.min_cell(min_cells[0]);
        pbc_csr.cfg_island_control.map(1);
    }

    pbc_csr.cfg_rc.init_start(1);
    pbc_csr.cfg_rc.init_reset(0);

    return sdk::SDK_RET_OK;
}

static sdk_ret_t
elba_tm_init_enable_ports (void)
{
    sdk_ret_t ret = sdk::SDK_RET_OK;
    elb_top_csr_t &elb0 = ELB_BLK_REG_MODEL_ACCESS(elb_top_csr_t, 0, 0);
    elb_pbc_csr_t &pbc_csr = elb0.pb.pbc;

    tm_port_t port;
    tm_port_type_e port_type;
    uint32_t enable;
    uint32_t rate_limiter;
    uint32_t cut_thru;
    bool set_rate_limiter;
    bool set_cut_thru;
    bool drop_on_error;

    // ELBA_TM_PORT_UPLINK_0
    port = ELBA_TM_PORT_UPLINK_0;
    port_type = elba_tm_get_port_type(port);
    enable = 0;
    rate_limiter = (tm_cfg_profile()->num_active_uplink_ports * 2) - 1;
    cut_thru = 0;

    set_rate_limiter = false;
    set_cut_thru = false;
    drop_on_error = false;

    if (is_active_port(ELBA_TM_PORT_UPLINK_0)) {
        enable = 1;
    }

    switch(port_type) {
        case TM_PORT_TYPE_UPLINK:
            drop_on_error = true;
            if (port != ELBA_TM_PORT_NCSI) {
                set_rate_limiter = true;
            }
            break;
        case TM_PORT_TYPE_P4IG:
        case TM_PORT_TYPE_P4EG:

            set_cut_thru = true;
            cut_thru = 0;
            break;
        case TM_PORT_TYPE_DMA:
            drop_on_error = true;
            set_cut_thru = true;
            cut_thru = 0x1f;
            break;
        case NUM_TM_PORT_TYPES:
            break;
    }

    pbc_csr.port_0.cfg_write_control.read();

    pbc_csr.port_0.cfg_write_control.enable(enable);

    if (set_rate_limiter) {
        pbc_csr.port_0.cfg_write_control.rate_limiter(rate_limiter);
    }

    if (set_cut_thru) {
        pbc_csr.port_0.cfg_write_control.cut_thru(cut_thru);
    }

    if (drop_on_error) {
        pbc_csr.port_0.cfg_write_control.drop_on_error(1);
    } else {
        pbc_csr.port_0.cfg_write_control.drop_on_error(0);
    }


    if (tm_sw_init_enabled()) {
        stringstream data;
        data << hex << endl;
        data <<"pbc_csr.port_0.cfg_write_control.all: 0x" << pbc_csr.port_0.cfg_write_control.all() << endl;
data <<"pbc_csr.port_0.cfg_write_control.release_cells: 0x" << pbc_csr.port_0.cfg_write_control.release_cells() << endl;
data <<"pbc_csr.port_0.cfg_write_control.enable: 0x" << pbc_csr.port_0.cfg_write_control.enable() << endl;
data <<"pbc_csr.port_0.cfg_write_control.drop_on_error: 0x" << pbc_csr.port_0.cfg_write_control.drop_on_error() << endl;
data <<"pbc_csr.port_0.cfg_write_control.rewrite: 0x" << pbc_csr.port_0.cfg_write_control.rewrite() << endl;
data <<"pbc_csr.port_0.cfg_write_control.span_pg_enable: 0x" << pbc_csr.port_0.cfg_write_control.span_pg_enable() << endl;
data <<"pbc_csr.port_0.cfg_write_control.span_pg_index: 0x" << pbc_csr.port_0.cfg_write_control.span_pg_index() << endl;
data <<"pbc_csr.port_0.cfg_write_control.enforce_oq24_31: 0x" << pbc_csr.port_0.cfg_write_control.enforce_oq24_31() << endl;
data <<"pbc_csr.port_0.cfg_write_control.packing_timeout_cycles: 0x" << pbc_csr.port_0.cfg_write_control.packing_timeout_cycles() << endl;
data <<"pbc_csr.port_0.cfg_write_control.min_size: 0x" << pbc_csr.port_0.cfg_write_control.min_size() << endl;
data <<"pbc_csr.port_0.cfg_write_control.packing_same_pg: 0x" << pbc_csr.port_0.cfg_write_control.packing_same_pg() << endl;
data <<"pbc_csr.port_0.cfg_write_control.packing: 0x" << pbc_csr.port_0.cfg_write_control.packing() << endl;
data <<"pbc_csr.port_0.cfg_write_control.enqueue_mux_bypass: 0x" << pbc_csr.port_0.cfg_write_control.enqueue_mux_bypass() << endl;
data <<"pbc_csr.port_0.cfg_write_control.enable_drdy: 0x" << pbc_csr.port_0.cfg_write_control.enable_drdy() << endl;
data <<"pbc_csr.port_0.cfg_write_control.rate_limiter: 0x" << pbc_csr.port_0.cfg_write_control.rate_limiter() << endl;
data <<"pbc_csr.port_0.cfg_write_control.cut_thru: 0x" << pbc_csr.port_0.cfg_write_control.cut_thru() << endl;
data <<"pbc_csr.port_0.cfg_write_control.enable_mnic_multicast: 0x" << pbc_csr.port_0.cfg_write_control.enable_mnic_multicast() << endl;
data <<"pbc_csr.port_0.cfg_write_control.enable_bmc_multicast: 0x" << pbc_csr.port_0.cfg_write_control.enable_bmc_multicast() << endl;

        SDK_TRACE_DEBUG("%s", data.str().c_str());

        pbc_csr.port_0.cfg_write_control.write();
    }
    // ELBA_TM_PORT_UPLINK_1
    port = ELBA_TM_PORT_UPLINK_1;
    port_type = elba_tm_get_port_type(port);
    enable = 0;
    rate_limiter = (tm_cfg_profile()->num_active_uplink_ports * 2) - 1;
    cut_thru = 0;

    set_rate_limiter = false;
    set_cut_thru = false;
    drop_on_error = false;

    if (is_active_port(ELBA_TM_PORT_UPLINK_1)) {
        enable = 1;
    }

    switch(port_type) {
        case TM_PORT_TYPE_UPLINK:
            drop_on_error = true;
            if (port != ELBA_TM_PORT_NCSI) {
                set_rate_limiter = true;
            }
            break;
        case TM_PORT_TYPE_P4IG:
        case TM_PORT_TYPE_P4EG:

            set_cut_thru = true;
            cut_thru = 0;
            break;
        case TM_PORT_TYPE_DMA:
            drop_on_error = true;
            set_cut_thru = true;
            cut_thru = 0x1f;
            break;
        case NUM_TM_PORT_TYPES:
            break;
    }

    pbc_csr.port_1.cfg_write_control.read();

    pbc_csr.port_1.cfg_write_control.enable(enable);

    if (set_rate_limiter) {
        pbc_csr.port_1.cfg_write_control.rate_limiter(rate_limiter);
    }

    if (set_cut_thru) {
        pbc_csr.port_1.cfg_write_control.cut_thru(cut_thru);
    }

    if (drop_on_error) {
        pbc_csr.port_1.cfg_write_control.drop_on_error(1);
    } else {
        pbc_csr.port_1.cfg_write_control.drop_on_error(0);
    }


    if (tm_sw_init_enabled()) {
        stringstream data;
        data << hex << endl;
        data <<"pbc_csr.port_1.cfg_write_control.all: 0x" << pbc_csr.port_1.cfg_write_control.all() << endl;
data <<"pbc_csr.port_1.cfg_write_control.release_cells: 0x" << pbc_csr.port_1.cfg_write_control.release_cells() << endl;
data <<"pbc_csr.port_1.cfg_write_control.enable: 0x" << pbc_csr.port_1.cfg_write_control.enable() << endl;
data <<"pbc_csr.port_1.cfg_write_control.drop_on_error: 0x" << pbc_csr.port_1.cfg_write_control.drop_on_error() << endl;
data <<"pbc_csr.port_1.cfg_write_control.rewrite: 0x" << pbc_csr.port_1.cfg_write_control.rewrite() << endl;
data <<"pbc_csr.port_1.cfg_write_control.span_pg_enable: 0x" << pbc_csr.port_1.cfg_write_control.span_pg_enable() << endl;
data <<"pbc_csr.port_1.cfg_write_control.span_pg_index: 0x" << pbc_csr.port_1.cfg_write_control.span_pg_index() << endl;
data <<"pbc_csr.port_1.cfg_write_control.enforce_oq24_31: 0x" << pbc_csr.port_1.cfg_write_control.enforce_oq24_31() << endl;
data <<"pbc_csr.port_1.cfg_write_control.packing_timeout_cycles: 0x" << pbc_csr.port_1.cfg_write_control.packing_timeout_cycles() << endl;
data <<"pbc_csr.port_1.cfg_write_control.min_size: 0x" << pbc_csr.port_1.cfg_write_control.min_size() << endl;
data <<"pbc_csr.port_1.cfg_write_control.packing_same_pg: 0x" << pbc_csr.port_1.cfg_write_control.packing_same_pg() << endl;
data <<"pbc_csr.port_1.cfg_write_control.packing: 0x" << pbc_csr.port_1.cfg_write_control.packing() << endl;
data <<"pbc_csr.port_1.cfg_write_control.enqueue_mux_bypass: 0x" << pbc_csr.port_1.cfg_write_control.enqueue_mux_bypass() << endl;
data <<"pbc_csr.port_1.cfg_write_control.enable_drdy: 0x" << pbc_csr.port_1.cfg_write_control.enable_drdy() << endl;
data <<"pbc_csr.port_1.cfg_write_control.rate_limiter: 0x" << pbc_csr.port_1.cfg_write_control.rate_limiter() << endl;
data <<"pbc_csr.port_1.cfg_write_control.cut_thru: 0x" << pbc_csr.port_1.cfg_write_control.cut_thru() << endl;
data <<"pbc_csr.port_1.cfg_write_control.enable_mnic_multicast: 0x" << pbc_csr.port_1.cfg_write_control.enable_mnic_multicast() << endl;
data <<"pbc_csr.port_1.cfg_write_control.enable_bmc_multicast: 0x" << pbc_csr.port_1.cfg_write_control.enable_bmc_multicast() << endl;

        SDK_TRACE_DEBUG("%s", data.str().c_str());

        pbc_csr.port_1.cfg_write_control.write();
    }
    // ELBA_TM_PORT_UPLINK_2
    port = ELBA_TM_PORT_UPLINK_2;
    port_type = elba_tm_get_port_type(port);
    enable = 0;
    rate_limiter = (tm_cfg_profile()->num_active_uplink_ports * 2) - 1;
    cut_thru = 0;

    set_rate_limiter = false;
    set_cut_thru = false;
    drop_on_error = false;

    if (is_active_port(ELBA_TM_PORT_UPLINK_2)) {
        enable = 1;
    }

    switch(port_type) {
        case TM_PORT_TYPE_UPLINK:
            drop_on_error = true;
            if (port != ELBA_TM_PORT_NCSI) {
                set_rate_limiter = true;
            }
            break;
        case TM_PORT_TYPE_P4IG:
        case TM_PORT_TYPE_P4EG:

            set_cut_thru = true;
            cut_thru = 0;
            break;
        case TM_PORT_TYPE_DMA:
            drop_on_error = true;
            set_cut_thru = true;
            cut_thru = 0x1f;
            break;
        case NUM_TM_PORT_TYPES:
            break;
    }

    pbc_csr.port_2.cfg_write_control.read();

    pbc_csr.port_2.cfg_write_control.enable(enable);

    if (set_rate_limiter) {
        pbc_csr.port_2.cfg_write_control.rate_limiter(rate_limiter);
    }

    if (set_cut_thru) {
        pbc_csr.port_2.cfg_write_control.cut_thru(cut_thru);
    }

    if (drop_on_error) {
        pbc_csr.port_2.cfg_write_control.drop_on_error(1);
    } else {
        pbc_csr.port_2.cfg_write_control.drop_on_error(0);
    }


    if (tm_sw_init_enabled()) {
        stringstream data;
        data << hex << endl;
        data <<"pbc_csr.port_2.cfg_write_control.all: 0x" << pbc_csr.port_2.cfg_write_control.all() << endl;
data <<"pbc_csr.port_2.cfg_write_control.release_cells: 0x" << pbc_csr.port_2.cfg_write_control.release_cells() << endl;
data <<"pbc_csr.port_2.cfg_write_control.enable: 0x" << pbc_csr.port_2.cfg_write_control.enable() << endl;
data <<"pbc_csr.port_2.cfg_write_control.drop_on_error: 0x" << pbc_csr.port_2.cfg_write_control.drop_on_error() << endl;
data <<"pbc_csr.port_2.cfg_write_control.rewrite: 0x" << pbc_csr.port_2.cfg_write_control.rewrite() << endl;
data <<"pbc_csr.port_2.cfg_write_control.span_pg_enable: 0x" << pbc_csr.port_2.cfg_write_control.span_pg_enable() << endl;
data <<"pbc_csr.port_2.cfg_write_control.span_pg_index: 0x" << pbc_csr.port_2.cfg_write_control.span_pg_index() << endl;
data <<"pbc_csr.port_2.cfg_write_control.enforce_oq24_31: 0x" << pbc_csr.port_2.cfg_write_control.enforce_oq24_31() << endl;
data <<"pbc_csr.port_2.cfg_write_control.packing_timeout_cycles: 0x" << pbc_csr.port_2.cfg_write_control.packing_timeout_cycles() << endl;
data <<"pbc_csr.port_2.cfg_write_control.min_size: 0x" << pbc_csr.port_2.cfg_write_control.min_size() << endl;
data <<"pbc_csr.port_2.cfg_write_control.packing_same_pg: 0x" << pbc_csr.port_2.cfg_write_control.packing_same_pg() << endl;
data <<"pbc_csr.port_2.cfg_write_control.packing: 0x" << pbc_csr.port_2.cfg_write_control.packing() << endl;
data <<"pbc_csr.port_2.cfg_write_control.enqueue_mux_bypass: 0x" << pbc_csr.port_2.cfg_write_control.enqueue_mux_bypass() << endl;
data <<"pbc_csr.port_2.cfg_write_control.enable_drdy: 0x" << pbc_csr.port_2.cfg_write_control.enable_drdy() << endl;
data <<"pbc_csr.port_2.cfg_write_control.rate_limiter: 0x" << pbc_csr.port_2.cfg_write_control.rate_limiter() << endl;
data <<"pbc_csr.port_2.cfg_write_control.cut_thru: 0x" << pbc_csr.port_2.cfg_write_control.cut_thru() << endl;
data <<"pbc_csr.port_2.cfg_write_control.enable_mnic_multicast: 0x" << pbc_csr.port_2.cfg_write_control.enable_mnic_multicast() << endl;
data <<"pbc_csr.port_2.cfg_write_control.enable_bmc_multicast: 0x" << pbc_csr.port_2.cfg_write_control.enable_bmc_multicast() << endl;

        SDK_TRACE_DEBUG("%s", data.str().c_str());

        pbc_csr.port_2.cfg_write_control.write();
    }
    // ELBA_TM_PORT_UPLINK_3
    port = ELBA_TM_PORT_UPLINK_3;
    port_type = elba_tm_get_port_type(port);
    enable = 0;
    rate_limiter = (tm_cfg_profile()->num_active_uplink_ports * 2) - 1;
    cut_thru = 0;

    set_rate_limiter = false;
    set_cut_thru = false;
    drop_on_error = false;

    if (is_active_port(ELBA_TM_PORT_UPLINK_3)) {
        enable = 1;
    }

    switch(port_type) {
        case TM_PORT_TYPE_UPLINK:
            drop_on_error = true;
            if (port != ELBA_TM_PORT_NCSI) {
                set_rate_limiter = true;
            }
            break;
        case TM_PORT_TYPE_P4IG:
        case TM_PORT_TYPE_P4EG:

            set_cut_thru = true;
            cut_thru = 0;
            break;
        case TM_PORT_TYPE_DMA:
            drop_on_error = true;
            set_cut_thru = true;
            cut_thru = 0x1f;
            break;
        case NUM_TM_PORT_TYPES:
            break;
    }

    pbc_csr.port_3.cfg_write_control.read();

    pbc_csr.port_3.cfg_write_control.enable(enable);

    if (set_rate_limiter) {
        pbc_csr.port_3.cfg_write_control.rate_limiter(rate_limiter);
    }

    if (set_cut_thru) {
        pbc_csr.port_3.cfg_write_control.cut_thru(cut_thru);
    }

    if (drop_on_error) {
        pbc_csr.port_3.cfg_write_control.drop_on_error(1);
    } else {
        pbc_csr.port_3.cfg_write_control.drop_on_error(0);
    }


    if (tm_sw_init_enabled()) {
        stringstream data;
        data << hex << endl;
        data <<"pbc_csr.port_3.cfg_write_control.all: 0x" << pbc_csr.port_3.cfg_write_control.all() << endl;
data <<"pbc_csr.port_3.cfg_write_control.release_cells: 0x" << pbc_csr.port_3.cfg_write_control.release_cells() << endl;
data <<"pbc_csr.port_3.cfg_write_control.enable: 0x" << pbc_csr.port_3.cfg_write_control.enable() << endl;
data <<"pbc_csr.port_3.cfg_write_control.drop_on_error: 0x" << pbc_csr.port_3.cfg_write_control.drop_on_error() << endl;
data <<"pbc_csr.port_3.cfg_write_control.rewrite: 0x" << pbc_csr.port_3.cfg_write_control.rewrite() << endl;
data <<"pbc_csr.port_3.cfg_write_control.span_pg_enable: 0x" << pbc_csr.port_3.cfg_write_control.span_pg_enable() << endl;
data <<"pbc_csr.port_3.cfg_write_control.span_pg_index: 0x" << pbc_csr.port_3.cfg_write_control.span_pg_index() << endl;
data <<"pbc_csr.port_3.cfg_write_control.enforce_oq24_31: 0x" << pbc_csr.port_3.cfg_write_control.enforce_oq24_31() << endl;
data <<"pbc_csr.port_3.cfg_write_control.packing_timeout_cycles: 0x" << pbc_csr.port_3.cfg_write_control.packing_timeout_cycles() << endl;
data <<"pbc_csr.port_3.cfg_write_control.min_size: 0x" << pbc_csr.port_3.cfg_write_control.min_size() << endl;
data <<"pbc_csr.port_3.cfg_write_control.packing_same_pg: 0x" << pbc_csr.port_3.cfg_write_control.packing_same_pg() << endl;
data <<"pbc_csr.port_3.cfg_write_control.packing: 0x" << pbc_csr.port_3.cfg_write_control.packing() << endl;
data <<"pbc_csr.port_3.cfg_write_control.enqueue_mux_bypass: 0x" << pbc_csr.port_3.cfg_write_control.enqueue_mux_bypass() << endl;
data <<"pbc_csr.port_3.cfg_write_control.enable_drdy: 0x" << pbc_csr.port_3.cfg_write_control.enable_drdy() << endl;
data <<"pbc_csr.port_3.cfg_write_control.rate_limiter: 0x" << pbc_csr.port_3.cfg_write_control.rate_limiter() << endl;
data <<"pbc_csr.port_3.cfg_write_control.cut_thru: 0x" << pbc_csr.port_3.cfg_write_control.cut_thru() << endl;
data <<"pbc_csr.port_3.cfg_write_control.enable_mnic_multicast: 0x" << pbc_csr.port_3.cfg_write_control.enable_mnic_multicast() << endl;
data <<"pbc_csr.port_3.cfg_write_control.enable_bmc_multicast: 0x" << pbc_csr.port_3.cfg_write_control.enable_bmc_multicast() << endl;

        SDK_TRACE_DEBUG("%s", data.str().c_str());

        pbc_csr.port_3.cfg_write_control.write();
    }

    return ret;
}

static sdk_ret_t
elba_tm_init_qos_profile (elba_tm_cfg_profile_t *tm_cfg_profile,
                           sdk::lib::catalog *catalog)
{
    uint32_t interpipe_mtu;

    tm_cfg_profile->sw_init_enabled = qos_profile.sw_init_enable;
    tm_cfg_profile->sw_cfg_write_enabled = qos_profile.sw_cfg_write_enable;

    tm_cfg_profile->hbm_fifo_base =
            g_elba_state_pd->mempartition()->start_addr(
                                            MEM_REGION_QOS_HBM_FIFO_NAME);
    tm_cfg_profile->hbm_fifo_size =
        g_elba_state_pd->mempartition()->size(MEM_REGION_QOS_HBM_FIFO_NAME);

    tm_cfg_profile->num_active_uplink_ports = catalog->num_fp_ports();

    if (tm_cfg_profile->num_active_uplink_ports <= 1) {
        tm_cfg_profile->num_active_uplink_ports = 1;
        SDK_TRACE_INFO("num_uplink_ports %u is not valid. resetting it to %u",
                       catalog->num_fp_ports(),
                       tm_cfg_profile->num_active_uplink_ports);
    } else {
        // Do not count the BMC port
        tm_cfg_profile->num_active_uplink_ports -= 1;
    }

    tm_cfg_profile->jumbo_mtu[TM_PORT_TYPE_UPLINK] = qos_profile.jumbo_mtu;
    interpipe_mtu = qos_profile.jumbo_mtu + ELBA_TM_MAX_INTERPIPE_HDR_SZ;
    tm_cfg_profile->jumbo_mtu[TM_PORT_TYPE_P4IG] = interpipe_mtu;
    tm_cfg_profile->jumbo_mtu[TM_PORT_TYPE_P4EG] = interpipe_mtu;
    tm_cfg_profile->jumbo_mtu[TM_PORT_TYPE_DMA] = interpipe_mtu;

    tm_cfg_profile->num_qs[TM_PORT_TYPE_UPLINK] = qos_profile.num_uplink_qs;
    tm_cfg_profile->num_qs[TM_PORT_TYPE_P4IG] = qos_profile.num_p4ig_qs;
    tm_cfg_profile->num_qs[TM_PORT_TYPE_P4EG] = qos_profile.num_p4eg_qs;
    tm_cfg_profile->num_qs[TM_PORT_TYPE_DMA] = qos_profile.num_dma_qs;

    tm_cfg_profile->num_p4_high_perf_qs = qos_profile.num_p4_high_perf_qs;
    if (tm_cfg_profile->num_p4_high_perf_qs) {
        tm_cfg_profile->p4_high_perf_qs =
            (tm_q_t *)SDK_MALLOC(sdk::SDK_MEM_ALLOC_PD,
                                 tm_cfg_profile->num_p4_high_perf_qs * sizeof(tm_q_t));

        memcpy(tm_cfg_profile->p4_high_perf_qs, qos_profile.p4_high_perf_qs,
               tm_cfg_profile->num_p4_high_perf_qs * sizeof(tm_q_t));
    }

    if ((tm_cfg_profile->num_qs[TM_PORT_TYPE_UPLINK] *
         tm_cfg_profile->num_active_uplink_ports) > ELBA_TM_MAX_HBM_ETH_CONTEXTS) {
        SDK_TRACE_INFO("num_uplink_ports %u  with %u qs cannot be supported"
                       " reducing the num of qs to %u",
                       tm_cfg_profile->num_active_uplink_ports,
                       tm_cfg_profile->num_qs[TM_PORT_TYPE_UPLINK],
                       ELBA_TM_MAX_HBM_ETH_CONTEXTS/tm_cfg_profile->num_active_uplink_ports);
        tm_cfg_profile->num_qs[TM_PORT_TYPE_UPLINK] =
            ELBA_TM_MAX_HBM_ETH_CONTEXTS/tm_cfg_profile->num_active_uplink_ports;
    }

    if (!tm_cfg_profile->hbm_fifo_size) {
        SDK_TRACE_ERR("HBM allocation for QOS overflow fifo not available");
        return sdk::SDK_RET_INVALID_ARG;
    }
    return sdk::SDK_RET_OK;
}

static sdk_ret_t
elba_tm_update_perf_run_config (void)
{
    elb_top_csr_t &elb0 = ELB_BLK_REG_MODEL_ACCESS(elb_top_csr_t, 0, 0);
    elb_pf_csr_t &pf_csr = elb0.pf.pf;
    stringstream data;
    data << hex << endl;
    // ELBA_TM_PORT_UPLINK_0
    pf_csr.hbm_port_0.cfg_hbm_parser.read();
    pf_csr.hbm_port_0.cfg_hbm_parser.use_dot1q(0);
    pf_csr.hbm_port_0.cfg_hbm_parser.use_ip(0);
    pf_csr.hbm_port_0.cfg_hbm_parser.default_cos(0);
    pf_csr.hbm_port_0.cfg_hbm_parser.dscp_map(0);
    data <<"pf_csr.hbm_port_0.cfg_hbm_parser.all: 0x" << pf_csr.hbm_port_0.cfg_hbm_parser.all() << endl;

    pf_csr.hbm_port_0.cfg_hbm_parser.write();
    // ELBA_TM_PORT_UPLINK_1
    pf_csr.hbm_port_1.cfg_hbm_parser.read();
    pf_csr.hbm_port_1.cfg_hbm_parser.use_dot1q(0);
    pf_csr.hbm_port_1.cfg_hbm_parser.use_ip(0);
    pf_csr.hbm_port_1.cfg_hbm_parser.default_cos(0);
    pf_csr.hbm_port_1.cfg_hbm_parser.dscp_map(0);
    data <<"pf_csr.hbm_port_1.cfg_hbm_parser.all: 0x" << pf_csr.hbm_port_1.cfg_hbm_parser.all() << endl;

    pf_csr.hbm_port_1.cfg_hbm_parser.write();
    // ELBA_TM_PORT_UPLINK_2
    pf_csr.hbm_port_2.cfg_hbm_parser.read();
    pf_csr.hbm_port_2.cfg_hbm_parser.use_dot1q(0);
    pf_csr.hbm_port_2.cfg_hbm_parser.use_ip(0);
    pf_csr.hbm_port_2.cfg_hbm_parser.default_cos(0);
    pf_csr.hbm_port_2.cfg_hbm_parser.dscp_map(0);
    data <<"pf_csr.hbm_port_2.cfg_hbm_parser.all: 0x" << pf_csr.hbm_port_2.cfg_hbm_parser.all() << endl;

    pf_csr.hbm_port_2.cfg_hbm_parser.write();
    // ELBA_TM_PORT_UPLINK_3
    pf_csr.hbm_port_3.cfg_hbm_parser.read();
    pf_csr.hbm_port_3.cfg_hbm_parser.use_dot1q(0);
    pf_csr.hbm_port_3.cfg_hbm_parser.use_ip(0);
    pf_csr.hbm_port_3.cfg_hbm_parser.default_cos(0);
    pf_csr.hbm_port_3.cfg_hbm_parser.dscp_map(0);
    data <<"pf_csr.hbm_port_3.cfg_hbm_parser.all: 0x" << pf_csr.hbm_port_3.cfg_hbm_parser.all() << endl;

    pf_csr.hbm_port_3.cfg_hbm_parser.write();
    SDK_TRACE_DEBUG("%s", data.str().c_str());
    return sdk::SDK_RET_OK;
}

sdk_ret_t
elba_tm_init (sdk::lib::catalog* catalog)
{
    sdk_ret_t ret = sdk::SDK_RET_OK;
    elba_tm_cfg_profile_t tm_cfg_profile_;
    elba_tm_cfg_profile_t *tm_cfg_profile = &tm_cfg_profile_;

    elba_tm_asic_profile_t asic_profile;

    populate_asic_profile(&asic_profile);

    ret = elba_tm_init_qos_profile(tm_cfg_profile, catalog);
    if (ret != SDK_RET_OK) {
        SDK_TRACE_ERR("Error initing qos profile. ret: %d",
                      ret);
        return ret;
    }

    SDK_TRACE_DEBUG("cfg-profile sw_init_en %u sw_cfg_write_en %u "
                    "num_active_uplink_ports %u hbm_fifo_size %u hbm_fifo_base 0x%x",
                    tm_cfg_profile->sw_init_enabled,
                    tm_cfg_profile->sw_cfg_write_enabled,
                    tm_cfg_profile->num_active_uplink_ports,
                    tm_cfg_profile->hbm_fifo_size,
                    tm_cfg_profile->hbm_fifo_base);
    for (unsigned port_type = 0; port_type < NUM_TM_PORT_TYPES; port_type++) {
        if (tm_cfg_profile->num_qs[port_type] >
            elba_tm_get_num_iqs_for_port_type((tm_port_type_e)port_type)) {
            SDK_TRACE_ERR("Error cfg-profile port_type %u num_qs %u "
                          "supported %u",
                          (tm_port_type_e)port_type, tm_cfg_profile->num_qs[port_type],
                          elba_tm_get_num_iqs_for_port_type((tm_port_type_e)port_type));
            return sdk::SDK_RET_INVALID_ARG;
        }
        SDK_TRACE_DEBUG("cfg-profile port_type %u num_qs %u jumbo_mtu %u ",
                        (tm_port_type_e)port_type, tm_cfg_profile->num_qs[port_type],
                        tm_cfg_profile->jumbo_mtu[port_type]);
    }

    set_tm_ctx(tm_cfg_profile, &asic_profile);

    ret = elba_tm_global_init();
    if (ret != sdk::SDK_RET_OK) {
        SDK_TRACE_ERR("Error initing global config ret %d", ret);
        return ret;
    }
    if (tm_sw_init_enabled()) {
        // Poll for the completion of the inits
        elb_pb_init_done(0,0);
    }

    ret = elba_tm_init_ports();
    if (ret != sdk::SDK_RET_OK) {
        SDK_TRACE_ERR("Error initing ports ret %d", ret);
        return ret;
    }

    ret = elba_tm_init_enable_ports();
    if (ret != sdk::SDK_RET_OK) {
        SDK_TRACE_ERR("Error enabling ports ret %d", ret);
        return ret;
    }

    char *perf_run = getenv("PERF_RUN");
    if (perf_run) {
        SDK_TRACE_DEBUG("perf-run env %s", perf_run);
        if (!strcmp(perf_run, "true")) {
            elba_tm_update_perf_run_config();
        }
    }
    SDK_TRACE_DEBUG("Init completed");
    tm_ctx()->init_complete.store(true);

    return ret;
}

// Programs the base address in HBM for the replication table
sdk_ret_t
elba_tm_repl_table_base_addr_set (uint64_t addr)
{
    elb_top_csr_t &elb0 = ELB_BLK_REG_MODEL_ACCESS(elb_top_csr_t, 0, 0);
    elb_pbc_csr_t &pbc_csr = elb0.pb.pbc;
    pbc_csr.cfg_rpl.read();
    pbc_csr.cfg_rpl.base(addr);
    pbc_csr.cfg_rpl.write();
    return sdk::SDK_RET_OK;
}

// Programs the replication table token size
sdk_ret_t
elba_tm_repl_table_token_size_set (uint32_t size_in_bits)
{
    elb_top_csr_t &elb0 = ELB_BLK_REG_MODEL_ACCESS(elb_top_csr_t, 0, 0);
    elb_pbc_csr_t &pbc_csr = elb0.pb.pbc;
    pbc_csr.cfg_rpl.read();

    // "Size of token in nodes. 0: 32 bits, 1: 48 bits, 2: 64 bits"
    if (size_in_bits == 64) {
        pbc_csr.cfg_rpl.token_size(2);
    } else if (size_in_bits == 48) {
        pbc_csr.cfg_rpl.token_size(1);
    } else if (size_in_bits == 32) {
        pbc_csr.cfg_rpl.token_size(0);
    } else {
        return sdk::SDK_RET_INVALID_ARG;
    }

    pbc_csr.cfg_rpl.write();
    return sdk::SDK_RET_OK;
}

// Get hw clock 
extern "C" sdk_ret_t
elba_tm_get_clock_tick (uint64_t *tick)
{
    elb_top_csr_t &elb0 = ELB_BLK_REG_MODEL_ACCESS(elb_top_csr_t, 0, 0);
    elb_pf_csr_t &pf_csr = elb0.pf.pf;

    pf_csr.sta_hbm_timestamp.read();
    *tick = (uint64_t)pf_csr.sta_hbm_timestamp.value().convert_to<uint64_t>();
    return sdk::SDK_RET_OK;
}

#define CHECK_OVERFLOW_AND_UPDATE(c, p, n)       \
{                                                \
    c += (p > n) ? (UINT32_MAX - p) + n : n - p; \
    p = n;                                       \
}

sdk_ret_t
elba_tm_periodic_stats_update (void)
{
    uint32_t port_in, port_out;
    uint32_t fifo_type;
    uint32_t context;
    elba_tm_hbm_context_stats_t *cur_vals;
    elba_tm_hbm_context_stats_t *new_val;
    elba_tm_hbm_context_stats_t *prev_vals;

    elba_tm_hbm_context_stats_t new_vals[NUM_TM_HBM_FIFO_TYPES][ELBA_TM_MAX_HBM_CONTEXTS] = {};
    cpp_int good_count_in;
    cpp_int errored_count_in;
    cpp_int watermark_in;
    cpp_int good_count_out;
    cpp_int errored_count_out;
    cpp_int watermark_out;

    if (!tm_ctx() || !tm_ctx()->init_complete.load()) {
        return sdk::SDK_RET_OK;
    }

    // Read from asic
    for (fifo_type = 0; fifo_type < NUM_TM_HBM_FIFO_TYPES; fifo_type++) {
        for (context = 0; context < elba_tm_max_hbm_contexts_for_fifo(fifo_type);
             context++) {
            if (fifo_type == TM_HBM_FIFO_TYPE_UPLINK) {
                port_in = 13;
                port_out = 12;
            } else {
                port_in = 15;
                port_out = 14;
            }
            new_val = &new_vals[fifo_type][context];
            elb_pb_read_hbm_ctx_stat(0, 0, port_in, context, good_count_in,
                                     errored_count_in, watermark_in);
            elb_pb_read_hbm_ctx_stat(0, 0, port_out, context, good_count_out,
                                     errored_count_out, watermark_out);
            new_val->good_pkts_in = good_count_in.convert_to<uint32_t>();
            new_val->good_pkts_out = good_count_out.convert_to<uint32_t>();
            new_val->errored_pkts_in = errored_count_in.convert_to<uint32_t>();
            new_val->max_oflow_fifo_depth = watermark_in.convert_to<uint32_t>();
        }
    }
    // Take lock and update
    SDK_SPINLOCK_LOCK(&tm_ctx()->stats.slock);
    for (fifo_type = 0; fifo_type < NUM_TM_HBM_FIFO_TYPES; fifo_type++) {
        for (context = 0; context < elba_tm_max_hbm_contexts_for_fifo(fifo_type);
             context++) {
            cur_vals = &tm_ctx()->stats.cur_vals[fifo_type][context];
            prev_vals = &tm_ctx()->stats.prev_vals[fifo_type][context];
            new_val = &new_vals[fifo_type][context];
            CHECK_OVERFLOW_AND_UPDATE(cur_vals->good_pkts_in,
                                      prev_vals->good_pkts_in,
                                      new_val->good_pkts_in);
            CHECK_OVERFLOW_AND_UPDATE(cur_vals->good_pkts_out,
                                      prev_vals->good_pkts_out,
                                      new_val->good_pkts_out);
            CHECK_OVERFLOW_AND_UPDATE(cur_vals->errored_pkts_in,
                                      prev_vals->errored_pkts_in,
                                      new_val->errored_pkts_in);
            cur_vals->max_oflow_fifo_depth = new_val->max_oflow_fifo_depth;
        }
    }
    SDK_SPINLOCK_UNLOCK(&tm_ctx()->stats.slock);
    return sdk::SDK_RET_OK;
}

static void
elba_tm_get_context_stats (tm_hbm_fifo_type_e fifo_type, uint32_t context,
                            elba_tm_hbm_context_stats_t *context_stats)
{
    if ((fifo_type >= NUM_TM_HBM_FIFO_TYPES) ||
        (context >= elba_tm_max_hbm_contexts_for_fifo(fifo_type)))  {
        *context_stats = {0};
        return;
    }
    SDK_SPINLOCK_LOCK(&tm_ctx()->stats.slock);
    *context_stats = tm_ctx()->stats.cur_vals[fifo_type][context];
    SDK_SPINLOCK_UNLOCK(&tm_ctx()->stats.slock);
}

static void
elba_tm_reset_context_stats (tm_hbm_fifo_type_e fifo_type, uint32_t context)
{
    if ((fifo_type >= NUM_TM_HBM_FIFO_TYPES) ||
        (context >= elba_tm_max_hbm_contexts_for_fifo(fifo_type)))  {
        return;
    }
    SDK_SPINLOCK_LOCK(&tm_ctx()->stats.slock);
    tm_ctx()->stats.cur_vals[fifo_type][context] =
                                    (const elba_tm_hbm_context_stats_t) {0};
    SDK_SPINLOCK_UNLOCK(&tm_ctx()->stats.slock);
}

sdk_ret_t
elba_tm_get_iq_stats (tm_port_t port, tm_q_t iq, tm_iq_stats_t *iq_stats)
{
    uint32_t num_hbm_contexts_per_port;
    uint32_t context;
    tm_port_type_e port_type;
    tm_hbm_fifo_type_e fifo_type;
    elba_tm_hbm_context_stats_t context_stats = {0};
    uint32_t cur_occupancy, peak_occupancy;

    if (!elba_tm_is_valid_port(port)) {
        SDK_TRACE_ERR("%u is not a valid TM port",
                      port);
        return sdk::SDK_RET_INVALID_ARG;
    }
    *iq_stats = (const tm_iq_stats_t) {0};

    // Read the registers to figure out the current stats

    if (port_supports_hbm_contexts(port)) {
        port_type = elba_tm_get_port_type(port);
        if (port_type == TM_PORT_TYPE_UPLINK) {
            num_hbm_contexts_per_port = tm_cfg_profile()->num_qs[TM_PORT_TYPE_UPLINK];
            context = (port * num_hbm_contexts_per_port) + iq;
        } else {
            context = iq;
        }
        fifo_type = elba_tm_get_fifo_type_for_port(port);

        // Get the context stats from shadow
        elba_tm_get_context_stats(fifo_type, context, &context_stats);

        iq_stats->oflow.good_pkts_in = context_stats.good_pkts_in;
        iq_stats->oflow.good_pkts_out = context_stats.good_pkts_out;
        iq_stats->oflow.errored_pkts_in = context_stats.errored_pkts_in;
        iq_stats->oflow.max_fifo_depth = ELBA_TM_HBM_FIFO_ALLOC_SIZE *
                                            context_stats.max_oflow_fifo_depth;

        // Get the current occupancy
        iq_stats->oflow.fifo_depth = ELBA_TM_HBM_FIFO_ALLOC_SIZE *
                                elba_tm_get_hbm_occupancy(fifo_type, context);
    }
    elba_tm_get_buffer_occupancy(port, iq, &cur_occupancy, &peak_occupancy);
    iq_stats->buffer_occupancy = ELBA_TM_CELL_SIZE * cur_occupancy;
    iq_stats->peak_occupancy = ELBA_TM_CELL_SIZE * peak_occupancy;

    return sdk::SDK_RET_OK;
}

sdk_ret_t
elba_tm_reset_iq_stats (tm_port_t port, tm_q_t iq)
{
    uint32_t num_hbm_contexts_per_port;
    uint32_t context;
    tm_port_type_e port_type;
    tm_hbm_fifo_type_e fifo_type;

    if (!elba_tm_is_valid_port(port)) {
        SDK_TRACE_ERR("%u is not a valid TM port",
                      port);
        return sdk::SDK_RET_INVALID_ARG;
    }

    if (port_supports_hbm_contexts(port)) {
        port_type = elba_tm_get_port_type(port);
        if (port_type == TM_PORT_TYPE_UPLINK) {
            num_hbm_contexts_per_port = tm_cfg_profile()->num_qs[TM_PORT_TYPE_UPLINK];
            context = (port * num_hbm_contexts_per_port) + iq;
        } else {
            context = iq;
        }
        fifo_type = elba_tm_get_fifo_type_for_port(port);

        // Reset the context stats in shadow
        elba_tm_reset_context_stats(fifo_type, context);

    }
    return sdk::SDK_RET_OK;
}

sdk_ret_t
elba_tm_get_oq_stats (tm_port_t port, tm_q_t oq, tm_oq_stats_t *oq_stats)
{
    elb_top_csr_t &elb0 = ELB_BLK_REG_MODEL_ACCESS(elb_top_csr_t, 0, 0);
    elb_pbc_csr_t &pbc_csr = elb0.pb.pbc;

    if (!elba_tm_is_valid_port(port)) {
        SDK_TRACE_ERR("%u is not a valid TM port",
                      port);
        return sdk::SDK_RET_INVALID_ARG;
    }

    pbc_csr.sta_oq[port].read();

    // 16 bits per oq
    oq_stats->queue_depth = cpp_helper.get_slc(pbc_csr.sta_oq[port].depth_value(),
                                              oq*16,
                                              (((oq+1)*16)-1)).convert_to<uint32_t>();
    return sdk::SDK_RET_OK;
}

extern "C" sdk_ret_t
elba_tm_get_pb_debug_stats (tm_port_t port, 
                            tm_pb_debug_stats_t *debug_stats, 
                            bool reset)
{
    return sdk::SDK_RET_OK;
}

}    // namespace elba
}    // namespace platform
}    // namespace sdk
