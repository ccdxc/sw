/*
 * Copyright (c) 2018-2019, Pensando Systems Inc.
 */

#include <cstdio>
#include <iostream>
#include <iomanip>
#include <algorithm>
#include <cmath>
#include <endian.h>
#include <sstream>
#include <string>
#include <sys/time.h>

// Tell nvme_dev.hpp to emumerate definitions of all devcmds
#define NVME_DEV_CMD_ENUMERATE  1

#include "nic/sdk/platform/misc/include/misc.h"
#include "nic/sdk/platform/intrutils/include/intrutils.h"
#include "nic/sdk/platform/pciemgr_if/include/pciemgr_if.hpp"
#include "nic/sdk/asic/pd/pd.hpp"
#include "nic/sdk/nvme/nvme_common.h"
#include "nic/sdk/platform/fru/fru.hpp"
#include "nic/include/edmaq.h"

#include "logger.hpp"
#include "nicmgr_utils.hpp"
#include "nvme_dev.hpp"
#include "nvme_lif.hpp"
#include "pd_client.hpp"
#include "adminq.hpp"

using namespace std;

#define GBPS_TO_BYTES_PER_SEC(gbps)                     \
    ((uint64_t)(gbps) * (1000000000ULL / 8))

#define PCI_VENDOR_ID_PENSANDO          0x1dd8

// Amount of time to wait for sequencer queues to be quiesced
#define NVME_DEV_SEQ_QUEUES_QUIESCE_TIME_US    5000000
#define NVME_DEV_RING_OP_QUIESCE_TIME_US       1000000
#define NVME_DEV_ALL_RINGS_MAX_QUIESCE_TIME_US (10 * NVME_DEV_RING_OP_QUIESCE_TIME_US)

/*
 * rounded up log2
 */
static uint32_t
log_2(uint32_t x)
{
  uint32_t log = 0;

  while ((uint64_t)(1 << log) < (uint64_t)x) {
      log++;
  }
  return log;
}

static inline uint64_t
timestamp(void)
{
    struct timeval tv;

    gettimeofday(&tv, NULL);
    return (tv.tv_sec * 1000000 + tv.tv_usec);
}

static inline void
time_expiry_set(nvme_timestamp_t& ts,
                uint64_t expiry)
{
    ts.timestamp = timestamp();
    ts.expiry = expiry;
}

static inline bool
time_expiry_check(const nvme_timestamp_t& ts)
{
    return (ts.expiry == 0) ||
           ((timestamp() - ts.timestamp) > ts.expiry);
}

static const char              *lif_state_str_table[] = {
    NVME_LIF_STATE_STR_TABLE
};

static const char              *lif_event_str_table[] = {
    NVME_LIF_EVENT_STR_TABLE
};

static nvme_lif_state_event_t  lif_initial_ev_table[] = {
    {
        NVME_LIF_EV_ANY,
        &NvmeLif::nvme_lif_reject_action,
        NVME_LIF_ST_SAME,
    },
    {
        NVME_LIF_EV_CREATE,
        &NvmeLif::nvme_lif_create_action,
        NVME_LIF_ST_WAIT_HAL,
    },
    {
        NVME_LIF_EV_DESTROY,
        &NvmeLif::nvme_lif_null_action,
        NVME_LIF_ST_SAME,
    },
    {
        NVME_LIF_EV_NULL
    },
};

static nvme_lif_state_event_t  lif_wait_hal_ev_table[] = {
    {
        NVME_LIF_EV_ANY,
        &NvmeLif::nvme_lif_eagain_action,
        NVME_LIF_ST_SAME,
    },
    {
        NVME_LIF_EV_HAL_UP,
        &NvmeLif::nvme_lif_hal_up_action,
        NVME_LIF_ST_PRE_INIT,
    },
    {
        NVME_LIF_EV_DESTROY,
        &NvmeLif::nvme_lif_null_action,
        NVME_LIF_ST_SAME,
    },
    {
        NVME_LIF_EV_NULL
    },
};

static nvme_lif_state_event_t  lif_pre_init_ev_table[] = {
    {
        NVME_LIF_EV_ANY,
        &NvmeLif::nvme_lif_reject_action,
        NVME_LIF_ST_SAME,
    },
    {
        NVME_LIF_EV_INIT,
        &NvmeLif::nvme_lif_init_action,
        NVME_LIF_ST_POST_INIT,
    },
    {
        NVME_LIF_EV_DESTROY,
        &NvmeLif::nvme_lif_null_action,
        NVME_LIF_ST_SAME,
    },
    {
        NVME_LIF_EV_NULL
    },
};

static nvme_lif_state_event_t  lif_post_init_ev_table[] = {
    {
        NVME_LIF_EV_ANY,
        &NvmeLif::nvme_lif_reject_action,
        NVME_LIF_ST_SAME,
    },
    {
        NVME_LIF_EV_DESTROY,
        &NvmeLif::nvme_lif_destroy_action,
        NVME_LIF_ST_SAME,
    },
    {
        NVME_LIF_EV_NULL
    },
};

static nvme_lif_state_event_t  *lif_fsm_table[NVME_LIF_ST_MAX] = {
    [NVME_LIF_ST_INITIAL]              = lif_initial_ev_table,
    [NVME_LIF_ST_WAIT_HAL]             = lif_wait_hal_ev_table,
    [NVME_LIF_ST_PRE_INIT]             = lif_pre_init_ev_table,
    [NVME_LIF_ST_POST_INIT]            = lif_post_init_ev_table,
};

static nvme_lif_ordered_event_t lif_ordered_ev_table[NVME_LIF_ST_MAX][NVME_LIF_EV_MAX];


static void nvme_lif_state_machine_build(void);
static const char *lif_state_str(nvme_lif_state_t state);
static const char *lif_event_str(nvme_lif_event_t event);

#define NVME_LIF_FSM_LOG()                                                     \
    NIC_LOG_DEBUG("{}: state {} event {}: ",                                    \
                  LifNameGet(),                                                 \
                  lif_state_str(fsm_ctx.enter_state), lif_event_str(event))

#if 0
static bool nvme_lif_fsm_verbose;

#define NVME_LIF_FSM_VERBOSE_LOG()                                             \
    if (nvme_lif_fsm_verbose) NVME_LIF_FSM_LOG()
#endif

#define NVME_LIF_FSM_ERR_LOG()                                                 \
    NIC_LOG_ERR("{}: state {} invalid event {}: ",                              \
                LifNameGet(),                                                   \
                lif_state_str(fsm_ctx.enter_state), lif_event_str(event))

NvmeLif::NvmeLif(NvmeDev& nvme_dev,
                   nvme_lif_res_t& lif_res,
                   EV_P) :
    nvme_dev(nvme_dev),
    spec(nvme_dev.DevSpecGet()),
    pd(nvme_dev.PdClientGet()),
    dev_api(nvme_dev.DevApiGet())
{
    this->loop = loop;
    nvme_lif_state_machine_build();

    NIC_HEADER_TRACE("Adding NVME LIF");

    memset(&hal_lif_info_, 0, sizeof(hal_lif_info_));
    hal_lif_info_.lif_id = lif_res.lif_id;
    lif_name = spec->name + std::string("/lif") +
               std::to_string(hal_lif_info_.lif_id);
    strncpy0(hal_lif_info_.name, lif_name.c_str(), sizeof(hal_lif_info_.name));
    intr_base = lif_res.intr_base;

    memset(&fsm_ctx, 0, sizeof(fsm_ctx));
    fsm_ctx.state = NVME_LIF_ST_INITIAL;
    nvme_lif_state_machine(NVME_LIF_EV_CREATE);
}

NvmeLif::~NvmeLif()
{
    /*
     * Host driver would have already performed graceful reset-destroy, in
     * which case, the following FSM event would result in very quick work,
     * i.e., no delay.
     */
    fsm_ctx.devcmd.status = NVME_RC_EAGAIN;
    while (fsm_ctx.devcmd.status == NVME_RC_EAGAIN) {
        nvme_lif_state_machine(NVME_LIF_EV_DESTROY);
    }
}

void
NvmeLif::SetHalClient(devapi *dapi)
{
    dev_api = dapi;
}

void
NvmeLif::HalEventHandler(bool status)
{
    if (status) {
        nvme_lif_state_machine(NVME_LIF_EV_HAL_UP);
    }
}

void
NvmeAdminCmdHandler(void *obj,
                void *req,
                void *req_data,
                void *resp,
                void *resp_data)
{
    NvmeLif *lif = (NvmeLif *)obj;
    lif->CmdHandler(req, req_data, resp, resp_data);
}

#define NUM_NS_SUPPORTED 4

nvme_status_code_t
NvmeLif::CmdHandler(void *req,
                     void *req_data,
                     void *resp,
                     void *resp_data)
{
    nvme_cmd_t  *cmd_p;
    nvme_cpl_t cpl = {0};
    uint64_t    src_addr = 0, dst_addr = 0;
    uint16_t    xfer_size;
    bool        cpl_valid = true;

    NIC_LOG_DEBUG("Nvme {}: Handling cmd", LifNameGet());

    cmd_p = (nvme_cmd_t *) req;

    NIC_LOG_DEBUG("Nvme opc: {} cid: {} nsid: {} "
                  "prp1: {:#x} prp2: {:#x} cdw10: {:#x} "
                  "cdw11: {:#x} cdw12: {:#x} cdw13: {:#x} cdw14: {:#x}",
                   cmd_p->opc, cmd_p->cid, cmd_p->nsid,
                   cmd_p->dptr.prp.prp1, cmd_p->dptr.prp.prp2,
                   cmd_p->cdw10, cmd_p->cdw11, cmd_p->cdw12,
                   cmd_p->cdw13, cmd_p->cdw14);

    // cpl default return values
    cpl.status.p = cq_color;
    cpl.cid = cmd_p->cid;
    cpl.sqid = NVME_ASQ_QID;
    //cpl.sqhd = cq_head;
    cpl.sqhd = sq_head;
    cpl.status.sc = NVME_SC_SUCCESS;
    cpl.status.sct = NVME_SCT_GENERIC;

    // assuming cq_ring_size is equal to sq_ring_size
    sq_head = (sq_head + 1) % cq_ring_size;

    // default dst_addr
    dst_addr  = cmd_p->dptr.prp.prp1;
    //dst_addr |= NVME_PHYS_ADDR_LIF_SET(LifIdGet());
    //dst_addr |= NVME_PHYS_ADDR_HOST_SET(1);

    if (cmd_p->opc == NVME_OPC_IDENTIFY) {
        // identify
        uint8_t cns;

        NIC_LOG_DEBUG("Nvme Identify cntid: {} cns: {}",
                      cmd_p->cdw10 >> 16, cmd_p->cdw10 & 0xff);

        cns = cmd_p->cdw10 & 0xff;

        if (cns == NVME_IDENTIFY_CTRLR) {
            //controller
            //PCI_DEVICE_ID_PENSANDO_NVME
            nvme_ctrlr_data_t ctrlr_data = {0};

            ctrlr_data.vid = PCI_VENDOR_ID_PENSANDO;
            ctrlr_data.ssvid = PCI_VENDOR_ID_PENSANDO;

            std::string sn;
            sdk::platform::readfrukey(BOARD_SERIALNUMBER_KEY, sn);
            strncpy0((char *)ctrlr_data.sn, sn.c_str(), sizeof(ctrlr_data.sn));

            strncpy0((char *)ctrlr_data.mn, "PDS-NVMEDEV1", sizeof(ctrlr_data.mn));
            strncpy0((char *)ctrlr_data.fr, "0.90", sizeof(ctrlr_data.fr));

            ctrlr_data.rab = 0;
            ctrlr_data.mdts = 5;
            ctrlr_data.cntlid = LifIdGet();
            ctrlr_data.ver.mjr = 1;
            ctrlr_data.ver.mnr = 2;

            ctrlr_data.oaes.ns_attribute_notices = 1;
            ctrlr_data.acl = 0x3;
            ctrlr_data.aerl = 0x4;
            ctrlr_data.frmw.num_slots = 1;
            ctrlr_data.frmw.slot1_ro = 1;
            ctrlr_data.wctemp = 0x157;
            ctrlr_data.cctemp = 0x157;

            ctrlr_data.sqes.min = 6;
            ctrlr_data.sqes.max = 6;

            ctrlr_data.cqes.min = 4;
            ctrlr_data.cqes.max = 4;

            ctrlr_data.nn = NUM_NS_SUPPORTED;
            ctrlr_data.oncs.write_unc = 1;
            ctrlr_data.oncs.write_zeroes = 1;

            strncpy0((char *)ctrlr_data.subnqn, "pensando.nvme.io", sizeof(ctrlr_data.subnqn));

            xfer_size = sizeof(nvme_ctrlr_data_t);
            src_addr = edma_buf_base;

            WRITE_MEM(src_addr, (uint8_t *) &ctrlr_data, sizeof(nvme_ctrlr_data_t), 0);

        } else if (cns == NVME_IDENTIFY_ACTIVE_NS_LIST) {
            //active ns list
            nvme_ns_list_t ns_list = { 0 };

            for (int i = 0; i < NUM_NS_SUPPORTED; i++) {
                ns_list.ns_list[i] = i + 1;
            }

            xfer_size = sizeof(nvme_ns_list_t);
            src_addr = edma_buf_base;

            WRITE_MEM(src_addr, (uint8_t *) &ns_list, sizeof(nvme_ns_list_t), 0);

        } else if (cns == NVME_IDENTIFY_NS) {
            //namespace
            nvme_ns_data_t ns_data = { 0 };

            ns_data.nsze = cmd_p->nsid * 1024;
            ns_data.ncap = cmd_p->nsid * 1024;
            ns_data.nuse = cmd_p->nsid * 1024;
            ns_data.nlbaf = 1;

            ns_data.lbaf[0].ms = 0;
            ns_data.lbaf[0].lbads = 9; //512B
            ns_data.lbaf[0].rp = 3;

            ns_data.lbaf[1].ms = 0;
            ns_data.lbaf[1].lbads = 12; //4K
            ns_data.lbaf[1].rp = 0;

            xfer_size = sizeof(nvme_ns_data_t);
            src_addr = edma_buf_base;

            WRITE_MEM(src_addr, (uint8_t *) &ns_data, sizeof(nvme_ns_data_t), 0);
        }

    } else if (cmd_p->opc == NVME_OPC_SET_FEATURES) {

        // set_features
        uint8_t fid;

        fid = cmd_p->cdw10 & 0xff;

        NIC_LOG_DEBUG("Nvme Set Features fid: {}", fid);

        if (fid == NVME_FEAT_NUMBER_OF_QUEUES) {

            union nvme_feat_number_of_queues nvme_feat;

            nvme_feat.raw = cmd_p->cdw11;

            NIC_LOG_DEBUG("Nvme Set Features-Num Queues: ncqr: {:#x}, nsqr: {:#x}",
                          nvme_feat.bits.ncqr, nvme_feat.bits.nsqr);

            cpl.cdw0 = nvme_feat.raw;
        }
    } else if (cmd_p->opc == NVME_OPC_DELETE_IO_SQ) {
        // delete I/O SQ

        union nvme_delete_io_sq0 sqw0;

        sqw0.raw = cmd_p->cdw10;

        NIC_LOG_DEBUG("NVME Delete IO SQ qid: {:#x} ",
                       sqw0.bits.qid);

    } else if (cmd_p->opc == NVME_OPC_DELETE_IO_CQ) {
        // delete I/O CQ

        union nvme_delete_io_cq0 cqw0;

        cqw0.raw = cmd_p->cdw10;

        NIC_LOG_DEBUG("NVME Delete IO CQ qid: {:#x} ",
                       cqw0.bits.cqid);

    } else if (cmd_p->opc == NVME_OPC_CREATE_IO_CQ) {
        // create I/O CQ

        union nvme_create_io_cq0 cqw0;
        union nvme_create_io_cq1 cqw1;

        cqw0.raw = cmd_p->cdw10;
        cqw1.raw = cmd_p->cdw11;

        NIC_LOG_DEBUG("NVME Create IO CQ qid: {:#x} qsize: {:#x} "
                      "pc: {} ien: {}, iv: {}",
                       cqw0.bits.qid, cqw0.bits.qsize,
                       cqw1.bits.pc, cqw1.bits.ien, cqw1.bits.iv);

    } else if (cmd_p->opc == NVME_OPC_CREATE_IO_SQ) {
        // create I/O SQ

        union nvme_create_io_sq0 sqw0;
        union nvme_create_io_sq1 sqw1;

        sqw0.raw = cmd_p->cdw10;
        sqw1.raw = cmd_p->cdw11;

        NIC_LOG_DEBUG("NVME Create IO SQ qid: {:#x} qsize: {:#x} "
                      "pc: {} qprio: {}, cqid: {:#x}",
                       sqw0.bits.qid, sqw0.bits.qsize,
                       sqw1.bits.pc, sqw1.bits.qprio, sqw1.bits.cqid);
    } else if (cmd_p->opc == NVME_OPC_ASYNC_EVENT_REQUEST) {
        NIC_LOG_DEBUG("Ignoring ASYNC EVENT REQUEST\n");
        cpl_valid = false;
    } else if (cmd_p->opc == NVME_OPC_ABORT) {
        NIC_LOG_DEBUG("Simply Ignoring ABORT REQUEST\n");
    }

    if (cpl_valid == false) {
        return NVME_RC_SUCCESS;
    }

    uint64_t addr, db_data;
    asic_db_addr_t db_addr;

    if (src_addr != 0) {
        struct edma_cmd_desc edma_cmd = {
            .opcode = EDMA_OPCODE_LOCAL_TO_HOST,
            .len = xfer_size,
            .src_lif = (uint16_t)LifIdGet(),
            .src_addr = src_addr,
            .dst_lif = (uint16_t)LifIdGet(),
            .dst_addr = dst_addr,
        };

        NIC_LOG_DEBUG("edma opc: L2H, src_lif: {}, dst_lif: {}, "
                      "src_addr: {:#x}, dst_addr: {:#x}, len: {}",
                      edma_cmd.src_lif, edma_cmd.dst_lif,
                      edma_cmd.src_addr, edma_cmd.dst_addr,
                      edma_cmd.len);

        addr = edma_ring_base + edma_ring_head * sizeof(struct edma_cmd_desc);
        WRITE_MEM(addr, (uint8_t *)&edma_cmd, sizeof(struct edma_cmd_desc), 0);
        edma_ring_head = (edma_ring_head + 1) % NVME_ARMQ_EDMA_RING_SIZE;

        if (edma_ring_head == 0) {
            edma_exp_color = (edma_exp_color + 1) % 2;
        }

        edma_comp_tail = (edma_comp_tail + 1) % NVME_ARMQ_EDMA_RING_SIZE;
    }


    // post completion

    src_addr = edma_buf_base2;
    WRITE_MEM(src_addr, (uint8_t *) &cpl, sizeof(nvme_cpl_t), 0);


    struct edma_cmd_desc cpl_edma_cmd = {
        .opcode = EDMA_OPCODE_LOCAL_TO_HOST,
        .len = sizeof(nvme_cpl_t),
        .src_lif = (uint16_t)LifIdGet(),
        .src_addr = src_addr,
        .dst_lif = (uint16_t)LifIdGet(),
        .dst_addr = cq_ring_base + cq_head * sizeof(nvme_cpl_t),
    };

    NIC_LOG_DEBUG("cpl edma opc: L2H, src_lif: {}, dst_lif: {}, "
                  "src_addr: {:#x}, dst_addr: {:#x}, len: {}",
                  cpl_edma_cmd.src_lif, cpl_edma_cmd.dst_lif,
                  cpl_edma_cmd.src_addr, cpl_edma_cmd.dst_addr,
                  cpl_edma_cmd.len);

    addr = edma_ring_base + edma_ring_head * sizeof(struct edma_cmd_desc);
    WRITE_MEM(addr, (uint8_t *)&cpl_edma_cmd, sizeof(struct edma_cmd_desc), 0);
    edma_ring_head = (edma_ring_head + 1) % NVME_ARMQ_EDMA_RING_SIZE;

    if (edma_ring_head == 0) {
        edma_exp_color = (edma_exp_color + 1) % 2;
    }

    // don't change the order  with the cq usage above
    cq_head = (cq_head + 1) % cq_ring_size;
    if (cq_head == 0) {
        cq_color = (cq_color + 1)%2;
    }

    // post completion interrupt
    uint32_t intr_assert_data = 1;

    src_addr = edma_buf_base2 + sizeof(nvme_cpl_t);
    WRITE_MEM(src_addr, (uint8_t *) &intr_assert_data, sizeof(uint32_t), 0);

    struct edma_cmd_desc cpl_intr_edma_cmd = {
        .opcode = EDMA_OPCODE_LOCAL_TO_LOCAL,
        .len = sizeof(uint32_t),
        .src_lif = (uint16_t)LifIdGet(),
        .src_addr = src_addr,
        .dst_lif = (uint16_t)LifIdGet(),
        .dst_addr = cq_intr_assert_addr,
    };

    NIC_LOG_DEBUG("cpl intr edma opc: L2L, src_lif: {}, dst_lif: {}, "
                  "src_addr: {:#x}, dst_addr: {:#x}, len: {}",
                  cpl_intr_edma_cmd.src_lif, cpl_intr_edma_cmd.dst_lif,
                  cpl_intr_edma_cmd.src_addr, cpl_intr_edma_cmd.dst_addr,
                  cpl_intr_edma_cmd.len);

    addr = edma_ring_base + edma_ring_head * sizeof(struct edma_cmd_desc);
    WRITE_MEM(addr, (uint8_t *)&cpl_intr_edma_cmd, sizeof(struct edma_cmd_desc), 0);
    edma_ring_head = (edma_ring_head + 1) % NVME_ARMQ_EDMA_RING_SIZE;

    //skip color check to later after polling

    //Ring doorbell at the end
    db_addr.lif_id = LifIdGet();
    db_addr.q_type = NVME_QTYPE_ARMQ;
    db_addr.upd = ASIC_DB_ADDR_UPD_FILL(ASIC_DB_UPD_SCHED_COSB, ASIC_DB_UPD_INDEX_SET_PINDEX, false);

    db_data = NVME_LIF_LOCAL_DBDATA_SET(NVME_ARMQ_EDMAQ_QID, 0 /*ring*/, edma_ring_head);

    NIC_LOG_DEBUG("db lif: {} qtype: {} qid: {} ring: {} pindex: {}",
                  LifIdGet(), NVME_QTYPE_ARMQ, NVME_ARMQ_EDMAQ_QID, 0, edma_ring_head);
    PAL_barrier();
    //free(ctrlr_data_p);
    sdk::asic::pd::asic_ring_db(&db_addr, db_data);

    // skip completion, monitor the last one.
    edma_comp_tail = (edma_comp_tail + 1) % NVME_ARMQ_EDMA_RING_SIZE;

    // Wait for EDMA completion
    struct edma_comp_desc comp = {0};
    uint8_t npolls = 0;
    addr = edma_comp_base + edma_comp_tail * sizeof(struct edma_comp_desc);
    do {
        READ_MEM(addr, (uint8_t *)&comp, sizeof(struct edma_comp_desc), 0);
        usleep(NVME_EDMAQ_COMP_POLL_US);
    } while (comp.color != edma_exp_color && ++npolls < NVME_EDMAQ_COMP_POLL_MAX);

    NIC_LOG_DEBUG("cq_ring_base: {:#x}, cq_head: {}, cq_color: {} "
                  "poll_addr: {:#x} exp_color: {}, color: {}, npolls: {}",
                  cq_ring_base, cq_head, cq_color, addr,
                  edma_exp_color, comp.color, npolls);

    if (edma_ring_head == 0) {
        edma_exp_color = (edma_exp_color + 1) % 2;
    }

    edma_comp_tail = (edma_comp_tail + 1) % NVME_ARMQ_EDMA_RING_SIZE;

    return NVME_RC_SUCCESS;
}

nvme_status_code_t
NvmeLif::Enable(nvme_dev_cmd_regs_t *regs_p)
{
    fsm_ctx.devcmd.req = regs_p;
    nvme_lif_state_machine(NVME_LIF_EV_INIT);
    return fsm_ctx.devcmd.status;
}

nvme_status_code_t
NvmeLif::Disable(nvme_dev_cmd_regs_t *regs_p)
{
    fsm_ctx.devcmd.status = NVME_RC_SUCCESS;
    return fsm_ctx.devcmd.status;
}

/*
 * LIF State Machine Actions
 */
nvme_lif_event_t
NvmeLif::nvme_lif_null_action(nvme_lif_event_t event)
{
    NVME_LIF_FSM_LOG();
    fsm_ctx.devcmd.status = NVME_RC_SUCCESS;
    return NVME_LIF_EV_NULL;
}

nvme_lif_event_t
NvmeLif::nvme_lif_eagain_action(nvme_lif_event_t event)
{
    NVME_LIF_FSM_LOG();
    fsm_ctx.devcmd.status = NVME_RC_EAGAIN;
    return NVME_LIF_EV_NULL;
}

nvme_lif_event_t
NvmeLif::nvme_lif_reject_action(nvme_lif_event_t event)
{
    NVME_LIF_FSM_ERR_LOG();
    fsm_ctx.devcmd.status = NVME_RC_EPERM;
    return NVME_LIF_EV_NULL;
}

nvme_lif_event_t
NvmeLif::nvme_lif_create_action(nvme_lif_event_t event)
{
    NVME_LIF_FSM_LOG();
    memset(qinfo, 0, sizeof(qinfo));

    qinfo[NVME_QTYPE_SQ] = {
        .type_num = NVME_QTYPE_SQ,
        .size = HW_CB_MULTIPLE(NVME_SQ_CB_SIZE_SHIFT),
        .entries = log_2(spec->sq_count),
    };

    qinfo[NVME_QTYPE_CQ] = {
        .type_num = NVME_QTYPE_CQ,
        .size = HW_CB_MULTIPLE(NVME_CQ_CB_SIZE_SHIFT),
        .entries = log_2(spec->cq_count),
    };

    qinfo[NVME_QTYPE_ARMQ] = {
        .type_num = NVME_QTYPE_ARMQ,
        .size = HW_CB_MULTIPLE(NVME_ARMQ_CB_SIZE_SHIFT),
        .entries = log_2(spec->cq_count),
    };

    hal_lif_info_.type = sdk::platform::LIF_TYPE_NONE;
    memcpy(hal_lif_info_.queue_info, qinfo, sizeof(hal_lif_info_.queue_info));

    return NVME_LIF_EV_NULL;
}

nvme_lif_event_t
NvmeLif::nvme_lif_destroy_action(nvme_lif_event_t event)
{
    NVME_LIF_FSM_LOG();
    fsm_ctx.devcmd.status = NVME_RC_SUCCESS;
    return NVME_LIF_EV_NULL;
}

nvme_lif_event_t
NvmeLif::nvme_lif_hal_up_action(nvme_lif_event_t event)
{
    NVME_LIF_FSM_LOG();

    cosA = 0;
    cosB = 0;
    admin_cosA = 1;
    admin_cosB = 1;

    return NVME_LIF_EV_NULL;
}

nvme_lif_event_t
NvmeLif::nvme_lif_init_action(nvme_lif_event_t event)
{
    NVME_LIF_FSM_LOG();

    if (dev_api->lif_create(&hal_lif_info_) != SDK_RET_OK) {
        NIC_LOG_ERR("{}: Failed to create LIF", LifNameGet());
        fsm_ctx.devcmd.status = NVME_RC_ERROR;
    }
    if (dev_api->lif_init(&hal_lif_info_) != SDK_RET_OK) {
        NIC_LOG_ERR("{}: Failed to init LIF", LifNameGet());
        fsm_ctx.devcmd.status = NVME_RC_ERROR;
    }

    // program the queue state
    pd->program_qstate((struct queue_info*)hal_lif_info_.queue_info,
                       &hal_lif_info_, 0x0);
    NIC_LOG_INFO("{}: created", LifNameGet());

    nvme_lif_edmaq_init_action(event);

    return (nvme_lif_adminq_init_action(event)); //XXX

    //return NVME_LIF_EV_NULL;
}

nvme_lif_event_t
NvmeLif::nvme_lif_edmaq_init_action(nvme_lif_event_t event)
{
    uint64_t    addr;

    // Edma Queue
    edma_ring_head = 0;
    edma_ring_base = pd->nicmgr_mem_alloc(4096 + (sizeof(struct edma_cmd_desc) * NVME_ARMQ_EDMA_RING_SIZE));
    if (edma_ring_base == 0) {
        NIC_LOG_ERR("{}: Failed to allocate edma ring!",
            hal_lif_info_.name);
        throw;
    }
    MEM_SET(edma_ring_base, 0, 4096 + (sizeof(struct edma_cmd_desc) * NVME_ARMQ_EDMA_RING_SIZE), 0);

    edma_comp_tail = 0;
    edma_exp_color = 1;
    edma_comp_base = pd->nicmgr_mem_alloc(4096 + (sizeof(struct edma_comp_desc) * NVME_ARMQ_EDMA_RING_SIZE));
    if (edma_comp_base == 0) {
        NIC_LOG_ERR("{}: Failed to allocate edma completion ring!",
            hal_lif_info_.name);
        throw;
    }
    MEM_SET(edma_comp_base, 0, 4096 + (sizeof(struct edma_comp_desc) * NVME_ARMQ_EDMA_RING_SIZE), 0);

    edma_buf_base = pd->nicmgr_mem_alloc(4096);
    if (edma_buf_base == 0) {
        NIC_LOG_ERR("{}: Failed to allocate edma buffer!",
            hal_lif_info_.name);
        throw;
    }

    edma_buf_base2 = pd->nicmgr_mem_alloc(4096);
    if (edma_buf_base2 == 0) {
        NIC_LOG_ERR("{}: Failed to allocate edma buffer2!",
            hal_lif_info_.name);
        throw;
    }

    NIC_LOG_INFO("{}: edma_ring_base {:#x} edma_comp_base {:#x} "
                 "edma_buf_base {:#x} edma_buf_base2 {:#x}",
                 hal_lif_info_.name, edma_ring_base, edma_comp_base,
                 edma_buf_base, edma_buf_base2);

    // Initialize EDMA service
    addr = pd->lm_->get_lif_qstate_addr(hal_lif_info_.lif_id, NVME_QTYPE_ARMQ, NVME_ARMQ_EDMAQ_QID);
    if (addr < 0) {
        NIC_LOG_ERR("{}: Failed to get qstate address for EDMAQ qid {}",
            hal_lif_info_.name, NVME_ARMQ_EDMAQ_QID);
        return (NVME_LIF_EV_NULL);
    }

    // Initialize the EDMA queue
    uint8_t off;
    edma_qstate_t dq_qstate = {0};
    if (pd->get_pc_offset("txdma_stage0.bin", "edma_stage0", &off, NULL) < 0) {
        NIC_LOG_ERR("Failed to get PC offset of program: txdma_stage0.bin label: edma_stage0");
        return (NVME_LIF_EV_NULL);
    }

    dq_qstate.pc_offset = off;
    dq_qstate.cos_sel = 0;
    dq_qstate.cosA = admin_cosA;
    dq_qstate.cosB = admin_cosB;
    dq_qstate.host = 0;
    dq_qstate.total = 1;
    dq_qstate.pid = 0;
    dq_qstate.p_index0 = edma_ring_head;
    dq_qstate.c_index0 = 0;
    dq_qstate.comp_index = edma_comp_tail;
    dq_qstate.sta.color = edma_exp_color;
    dq_qstate.cfg.enable = 1;
    dq_qstate.ring_base = edma_ring_base;
    dq_qstate.ring_size = LG2_NVME_ARMQ_EDMA_RING_SIZE;
    dq_qstate.cq_ring_base = edma_comp_base;
    dq_qstate.cfg.intr_enable = 0;
    dq_qstate.intr_assert_index = 0;
    WRITE_MEM(addr, (uint8_t *)&dq_qstate, sizeof(dq_qstate), 0);

    PAL_barrier();
    sdk::asic::pd::asicpd_p4plus_invalidate_cache(addr, sizeof(edma_qstate_t), P4PLUS_CACHE_INVALIDATE_TXDMA);

    return NVME_LIF_EV_NULL;
}

nvme_lif_event_t
NvmeLif::nvme_lif_adminq_init_action(nvme_lif_event_t event)
{
    admin_qstate_t      qstate;
    int64_t             asq_addr, acq_addr, arm_asq_addr;
    nvme_dev_cmd_regs_t  *regs_p= (nvme_dev_cmd_regs_t *)fsm_ctx.devcmd.req;

    NIC_LOG_DEBUG("aqa_acqs: {} aqa_asqs: {} acq: {:#x} asq: {:#x} "
                  "acqb: {:#x} asqb: {:#x}",
                  regs_p->aqa.acqs, regs_p->aqa.asqs,
                  regs_p->acq.num64, regs_p->asq.num64,
                  regs_p->acq.acqb, regs_p->asq.asqb);

    fsm_ctx.devcmd.status = NVME_RC_ERROR;
    asq_addr = pd->lm_->get_lif_qstate_addr(LifIdGet(),
                            NVME_QTYPE_SQ, NVME_ASQ_QID);
    if (asq_addr < 0) {
        NIC_LOG_ERR("{}: Failed to get qstate address for ASQ qid {}",
                    LifNameGet(), NVME_ASQ_QID);
        return NVME_LIF_EV_NULL;
    }

    acq_addr = pd->lm_->get_lif_qstate_addr(LifIdGet(),
                            NVME_QTYPE_CQ, NVME_ACQ_QID);
    if (acq_addr < 0) {
        NIC_LOG_ERR("{}: Failed to get qstate address for ACQ qid {}",
                    LifNameGet(), NVME_ACQ_QID);
        return NVME_LIF_EV_NULL;
    }

    arm_asq_addr = pd->lm_->get_lif_qstate_addr(LifIdGet(),
                                  NVME_QTYPE_ARMQ, NVME_ARMQ_ASQ_QID);
    if (arm_asq_addr < 0) {
        NIC_LOG_ERR("{}: Failed to get qstate address for ARM ASQ qid {}",
                    LifIdGet(), NVME_ARMQ_ASQ_QID);
        return NVME_LIF_EV_NULL;
    }

    uint8_t off;
    if (pd->get_pc_offset("txdma_stage0.bin", "adminq_stage0", &off, NULL) < 0) {
        NIC_LOG_ERR("Failed to get PC offset of program: txdma_stage0.bin "
                    "label: adminq_stage0");
        return NVME_LIF_EV_NULL;
    }

    memset(&qstate, 0, sizeof(qstate));

    qstate.pc_offset = off;
    qstate.cos_sel = 0;
    qstate.cosA = admin_cosA;
    qstate.cosB = admin_cosB;
    qstate.host = 1;
    qstate.total = 1;
    qstate.pid = 0; //XXX
    qstate.p_index0 = 0;
    qstate.c_index0 = 0;
    qstate.comp_index = 0;
    qstate.ci_fetch = 0;
    qstate.sta.color = 1;
    qstate.cfg.enable = 1;
    qstate.cfg.host_queue = 1;
    qstate.cfg.intr_enable = 1;

    qstate.ring_base = regs_p->asq.num64;
    qstate.ring_base |= NVME_PHYS_ADDR_LIF_SET(LifIdGet());
    qstate.ring_base |= NVME_PHYS_ADDR_HOST_SET(1);
    qstate.ring_size = log2(regs_p->aqa.asqs + 1);

    qstate.cq_ring_base = regs_p->acq.num64;
    qstate.cq_ring_base |= NVME_PHYS_ADDR_LIF_SET(LifIdGet());
    qstate.cq_ring_base |= NVME_PHYS_ADDR_HOST_SET(1);

    qstate.intr_assert_index = intr_base + NVME_ACQ_INTR_NUM;
    qstate.nicmgr_qstate_addr = arm_asq_addr;
    WRITE_MEM(asq_addr, (uint8_t *)&qstate, sizeof(qstate), 0);

    qstate.ring_size = log2(regs_p->aqa.acqs + 1);
    WRITE_MEM(acq_addr, (uint8_t *)&qstate, sizeof(qstate), 0);

    PAL_barrier();
    sdk::asic::pd::asicpd_p4plus_invalidate_cache(asq_addr, sizeof(qstate), P4PLUS_CACHE_INVALIDATE_TXDMA);
    sdk::asic::pd::asicpd_p4plus_invalidate_cache(acq_addr, sizeof(qstate), P4PLUS_CACHE_INVALIDATE_TXDMA);

    adminq = new AdminQ(LifNameGet().c_str(),
                        pd, LifIdGet(),
                        NVME_QTYPE_ARMQ, NVME_ARMQ_ASQ_QID, regs_p->aqa.asqs + 1,
                        NVME_QTYPE_ARMQ, NVME_ARMQ_ACQ_QID, regs_p->aqa.acqs + 1,
                        NvmeAdminCmdHandler, this, loop,
                        false);

    // Initialize AdminQ service
    if (!adminq->Init(0, admin_cosA, admin_cosB)) {
        NIC_LOG_ERR("{}: Failed to initialize AdminQ service",
                    LifNameGet());
        fsm_ctx.devcmd.status = NVME_RC_ERROR;
    }

    NIC_HEADER_TRACE("Config ASQ/ACQ for NVME Lif");

    cq_ring_base = qstate.cq_ring_base;
    aq_ring_base = qstate.ring_base;
    cq_ring_size = 1 << qstate.ring_size;
    sq_head = 0;
    cq_head = 0;
    cq_color = 1;
    cq_intr_assert_addr = intr_assert_addr(qstate.intr_assert_index);

    fsm_ctx.devcmd.status = NVME_RC_SUCCESS;
    return NVME_LIF_EV_NULL;
}

void
NvmeLif::nvme_lif_state_machine(nvme_lif_event_t event)
{
    nvme_lif_ordered_event_t   *ordered_event;
    nvme_lif_action_t          action;

    while (event != NVME_LIF_EV_NULL) {

        if ((fsm_ctx.state < NVME_LIF_ST_MAX) &&
            (event < NVME_LIF_EV_MAX)) {

            ordered_event = &lif_ordered_ev_table[fsm_ctx.state][event];
            fsm_ctx.enter_state = fsm_ctx.state;
            if (ordered_event->next_state != NVME_LIF_ST_SAME) {
                fsm_ctx.state = ordered_event->next_state;
            }
            action = ordered_event->action;
            if (!action) {
                NIC_LOG_ERR("Null action for state {} event {}",
                            lif_state_str(fsm_ctx.enter_state),
                            lif_event_str(event));
                throw;
            }
            event = (this->*action)(event);

        } else {
            NIC_LOG_ERR("Unknown state {} or event {}",
                        fsm_ctx.state, event);
            throw;
        }
    }
}

static void
nvme_lif_state_machine_build(void)
{
    nvme_lif_state_event_t    **fsm_entry;
    nvme_lif_state_event_t    *state_event;
    nvme_lif_state_event_t    *any_event;
    nvme_lif_ordered_event_t  *ordered_event;
    uint32_t                   state;

    static bool lif_ordered_event_table_built;
    if (lif_ordered_event_table_built) {
        return;
    }
    lif_ordered_event_table_built = true;

    for (fsm_entry = &lif_fsm_table[0], state = 0;
         fsm_entry < &lif_fsm_table[NVME_LIF_ST_MAX];
         fsm_entry++, state++) {

        state_event = *fsm_entry;
        if (state_event) {
            any_event = nullptr;
            while (state_event->event != NVME_LIF_EV_NULL) {
                if (state_event->event < NVME_LIF_EV_MAX) {
                    ordered_event = &lif_ordered_ev_table[state]
                                                         [state_event->event];

                    ordered_event->action = state_event->action;
                    ordered_event->next_state = state_event->next_state;

                    if (state_event->event == NVME_LIF_EV_ANY) {
                        any_event = state_event;
                    }

                } else {
                    NIC_LOG_ERR("Unknown event {} for state {}", state_event->event,
                                lif_state_str((nvme_lif_state_t)state));
                    throw;
                }
                state_event++;
            }

            if (!any_event) {
                NIC_LOG_ERR("Missing 'any' event for state {}",
                            lif_state_str((nvme_lif_state_t)state));
                throw;
            }

            for (ordered_event = &lif_ordered_ev_table[state][0];
                 ordered_event < &lif_ordered_ev_table[state][NVME_LIF_EV_MAX];
                 ordered_event++) {

                if (!ordered_event->action) {
                    ordered_event->action  = any_event->action;
                    ordered_event->next_state = any_event->next_state;
                }
            }
        }
    }
}

static const char *
lif_state_str(nvme_lif_state_t state)
{
    if (state < NVME_LIF_ST_MAX) {
        return lif_state_str_table[state];
    }
    return "unknown_state";
}

static const char *
lif_event_str(nvme_lif_event_t event)
{
    if (event < NVME_LIF_EV_MAX) {
        return lif_event_str_table[event];
    }
    return "unknown_event";
}

