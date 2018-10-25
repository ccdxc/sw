#include <math.h>
#include "tests.hpp"
#include "utils.hpp"
#include "hal_if.hpp"
#include "nicmgr_if.hpp"
#include "nic/e2etests/driver/lib_driver.hpp"

#include <assert.h>
#include <stdint.h>
#include <unistd.h>
#include <stdio.h>

#define ACCEL_DEV_CMD_ENUMERATE
#include "accel_dev.hpp"

/*
 * Interface to nicmgr admin queue
 */

#define NICMGR_ADMIN_QID                0
#define NICMGR_ADMIN_QSIZE              16
#define NICMGR_ADMIN_QPOLL_RETRIES      1000
#define NICMGR_ADMIN_QCNDX_UNKNOWN      0xffff

/*
 * Size of dev_cmd_regs_t's cmd/cpl area without the data portion.
 */
#define DEV_CMDREGS_CMDCPL_SIZE         \
    (offsetof(dev_cmd_regs_t, cpl) + sizeof(dev_cmd_cpl_t))

using namespace utils;

namespace nicmgr_if {

static dp_mem_t     *dev_cmd_buf;
static dp_mem_t     *nicmgr_devcmdpa_buf;
static dp_mem_t     *nicmgr_devcmddbpa_buf;
static dp_mem_t     *nicmgr_devcmdpa_cmdcpl;
static uint64_t     nicmgr_hw_lif_id;
static uint64_t     seq_hw_lif_id;
static uint64_t     nicmgr_devcmddbpa;
static uint64_t     nicmgr_devcmdpa;
static uint16_t     nicmgr_admin_qcndx;

static dev_cmd_regs_t *nicmgr_if_push(dev_cmd_t& cmd);

static void
nicmgr_if_devcmdpa_init(void)
{
    uint32_t    hbm_size;

    // Allocatge HBM address for storage
    assert(hal_if::alloc_hbm_address(ACCEL_DEVCMD_HBM_HANDLE, &nicmgr_devcmdpa,
                                     &hbm_size) == 0);
    hbm_size *= 1024;
    assert(hbm_size >= (ACCEL_DEV_BAR0_NUM_PAGES_MAX * ACCEL_DEV_PAGE_SIZE));
    nicmgr_devcmdpa = ACCEL_DEV_PAGE_ALIGN(nicmgr_devcmdpa);

    nicmgr_devcmddbpa = nicmgr_devcmdpa + ACCEL_DEV_PAGE_SIZE;
    printf("%s nicmgr_devcmdpa 0x%lx nicmgr_devcmddbpa 0x%lx total_size %u\n",
           __FUNCTION__, nicmgr_devcmdpa, nicmgr_devcmddbpa, hbm_size);

    nicmgr_devcmdpa_buf = new dp_mem_t((uint8_t *)nicmgr_devcmdpa,
                                       1, ACCEL_DEV_PAGE_SIZE,
                                       DP_MEM_TYPE_HBM, DP_MEM_ALLOC_NO_FILL);
    nicmgr_devcmddbpa_buf = new dp_mem_t((uint8_t *)nicmgr_devcmddbpa,
                                         1, sizeof(dev_cmd_db_t),
                                         DP_MEM_TYPE_HBM, DP_MEM_ALLOC_NO_FILL);
    nicmgr_devcmdpa_cmdcpl = 
           nicmgr_devcmdpa_buf->fragment_find(0, DEV_CMDREGS_CMDCPL_SIZE);
}


int
nicmgr_if_init(void)
{
    tests::Poller   poll(FLAGS_long_poll_interval);
    enum queue_type nicmgr_qtype = RX;
    dev_cmd_regs_t  *dev_cmd;

    nicmgr_if_devcmdpa_init();

    /*
     * Wait until nicmgr has finished creating its LIF
     */
    auto nicmgr_lif_poll = [] () -> int
    {
        return hal_if::get_lif_info(FLAGS_nicmgr_lif, &nicmgr_hw_lif_id);
    };

    if (poll(nicmgr_lif_poll)) {
        printf("Failed to locate nicmgr LIF %lu\n", FLAGS_nicmgr_lif);
        return -1;
    }
    printf("Sucessfully located nicmgr hardware LIF ID %lu\n", nicmgr_hw_lif_id);

    /*
     * Wait until nicmgr has finished initializing its Rx/Tx qstate
     */
    auto nicmgr_qstate_poll = [&nicmgr_qtype] () -> int
    {
        queue_info_t    qi = {0};
        admin_qstate_t *qstate;

        qi.qstate_addr = get_qstate_addr(nicmgr_hw_lif_id, nicmgr_qtype, 0);
        qi.qstate = calloc(1, sizeof(admin_qstate_t));
        qstate = (admin_qstate_t *)qi.qstate;
        set_queue_info(nicmgr_hw_lif_id, nicmgr_qtype, 0, qi);
        read_queue(nicmgr_hw_lif_id, nicmgr_qtype, 0);

        return qstate->ring_base && qstate->ring_size ? 0 : -1;
    };

    usleep(10000);
    nicmgr_qtype = RX;
    if (poll(nicmgr_qstate_poll)) {
        printf("nicmgr_qstate_poll Rx timeout\n");
        return -1;
    }
    nicmgr_qtype = TX;
    if (poll(nicmgr_qstate_poll)) {
        printf("nicmgr_qstate_poll Tx timeout\n");
        return -1;
    }

    /*
     * Wait until Accel_PF is ready
     */
    auto devcmd_ready_poll = [&dev_cmd] () -> int
    {
        dev_cmd = (dev_cmd_regs_t *)nicmgr_devcmdpa_cmdcpl->read_thru();
        return dev_cmd->signature == DEV_CMD_SIGNATURE ? 0 : -1;
    };

    if (poll(devcmd_ready_poll)) {
        printf("Accel_PF devcmd_ready_poll timeout\n");
        return -1;
    }

    dev_cmd_buf = new dp_mem_t(1, sizeof(dev_cmd_regs_t),
                               DP_MEM_ALIGN_PAGE, DP_MEM_TYPE_HOST_MEM,
                               0, DP_MEM_ALLOC_NO_FILL);
    return 0;
}


int
nicmgr_if_identify(uint64_t *ret_seq_lif,
                   uint32_t *ret_seq_queues_per_lif,
                   accel_ring_t *ret_accel_ring_tbl,
                   uint32_t accel_ring_tbl_size)
{
    tests::Poller   poll(FLAGS_long_poll_interval);
    dev_cmd_regs_t  *dev_cmd;
    identity_t      *identity;
    identify_cpl_t  *cpl;
    dev_cmd_t       cmd = {0};
    queue_info_t    qinfo;

    *ret_seq_lif = 0;
    *ret_seq_queues_per_lif = 0;

    cmd.identify.opcode = CMD_OPCODE_IDENTIFY;
    cmd.identify.ver = IDENTITY_VERSION_1;
    dev_cmd = nicmgr_if_push(cmd);
    if (dev_cmd) {

        /*
         * Read the full result buffer including the data portion.
         */
        dev_cmd = (dev_cmd_regs_t *)nicmgr_devcmdpa_buf->read_thru();
        cpl = &dev_cmd->cpl.identify;
        if (cpl->ver != IDENTITY_VERSION_1) {
            printf("%s unsupported identity version %u\n",
                   __FUNCTION__, cpl->ver);
            return -1;
        }
        identity = (identity_t *)dev_cmd->data;
        printf("%s num_lifs %u hw_lif_id 0x%lx db_pages_per_lif %u "
               "admin_queues_per_lif %u seq_queues_per_lif %u\n", __FUNCTION__,
               identity->dev.num_lifs, identity->dev.lif_tbl[0].hw_lif_id, 
               identity->dev.db_pages_per_lif, identity->dev.admin_queues_per_lif,
               identity->dev.seq_queues_per_lif);
        if (!identity->dev.num_lifs || !identity->dev.db_pages_per_lif) {
            printf("%s num_lifs or db_pages_per_lif ERROR\n", __FUNCTION__);
            return -1;
        }

        seq_hw_lif_id = identity->dev.lif_tbl[0].hw_lif_id;
        if (ret_accel_ring_tbl) {
            if (accel_ring_tbl_size < sizeof(identity->dev.accel_ring_tbl)) {
                printf("%s accel_ring_tbl_size %u less than required size %d\n",
                       __FUNCTION__, accel_ring_tbl_size,
                      (int) sizeof(identity->dev.accel_ring_tbl));
                return -1;
            }
            memcpy(ret_accel_ring_tbl, identity->dev.accel_ring_tbl,
                   accel_ring_tbl_size);
        }

        *ret_seq_lif = seq_hw_lif_id;
        *ret_seq_queues_per_lif = identity->dev.seq_queues_per_lif;

        /*
         * We'd like to use lib_driver to interface to the adminQ so we
         * must set it up in the way lib_driver expects.
         */
        alloc_queue(seq_hw_lif_id, ADMIN, NICMGR_ADMIN_QID, NICMGR_ADMIN_QSIZE);
        read_queue(seq_hw_lif_id, ADMIN, NICMGR_ADMIN_QID);
        qinfo = get_queue_info(seq_hw_lif_id, ADMIN, NICMGR_ADMIN_QID); 
        ((admin_qstate_t *)qinfo.qstate)->nicmgr_qstate_addr =
                                    get_qstate_addr(nicmgr_hw_lif_id, RX, 0);
        write_queue(seq_hw_lif_id, ADMIN, NICMGR_ADMIN_QID);
        nicmgr_admin_qcndx = NICMGR_ADMIN_QCNDX_UNKNOWN;
        return 0;
    }

    return -1;
}


int
nicmgr_if_reset(void)
{
    dev_cmd_t   cmd = {0};

    cmd.reset.opcode = CMD_OPCODE_RESET;
    return nicmgr_if_push(cmd) ? 0 : -1;
}


int
nicmgr_if_lif_init(uint64_t seq_lif)
{
    dev_cmd_t   cmd = {0};

    cmd.lif_init.opcode = CMD_OPCODE_LIF_INIT;
    cmd.lif_init.index = seq_lif;
    return nicmgr_if_push(cmd) ? 0 : -1;
}


int
nicmgr_if_admin_queue_init(uint64_t seq_lif,
                           uint16_t log2_num_entries,
                           uint64_t base_addr)
{
    dev_cmd_t   cmd = {0};

    cmd.adminq_init.opcode = CMD_OPCODE_ADMINQ_INIT;
    cmd.adminq_init.index = 0;
    cmd.adminq_init.lif_index = seq_lif;
    cmd.adminq_init.ring_size = log2_num_entries;
    cmd.adminq_init.ring_base = base_addr;
    return nicmgr_if_push(cmd) ? 0 : -1;
}


int
nicmgr_if_seq_queue_init(uint64_t lif,
                         storage_seq_qgroup_t qgroup,
                         uint32_t qid,
                         uint8_t total_wrings,
                         uint8_t host_wrings,
                         uint16_t log2_num_entries,
                         uint64_t base_addr,
                         uint16_t log2_entry_size)
{
    dev_cmd_t   cmd = {0};

    cmd.seq_q_init.opcode = CMD_OPCODE_SEQ_QUEUE_INIT;
    cmd.seq_q_init.index = qid;
    cmd.seq_q_init.qgroup = qgroup;
    cmd.seq_q_init.enable = true;
    cmd.seq_q_init.total_wrings = total_wrings;
    cmd.seq_q_init.host_wrings = host_wrings;
    cmd.seq_q_init.entry_size = log2_entry_size;
    cmd.seq_q_init.wring_size = log2_num_entries;
    cmd.seq_q_init.wring_base = base_addr;
    cmd.seq_q_init.dol_req_devcmd_done = true;
    return nicmgr_if_push(cmd) ? 0 : -1;
}


static dev_cmd_regs_t *
nicmgr_if_push(dev_cmd_t& cmd)
{
    dp_mem_t        *dev_cmd_curr;
    dev_cmd_regs_t  *dev_cmd;
    dev_cmd_db_t    *dev_cmd_db;
    uint16_t        count = 0;
    tests::Poller   poll(FLAGS_long_poll_interval);

    switch (cmd.nop.opcode) {

    case CMD_OPCODE_RESET:
    case CMD_OPCODE_IDENTIFY:
    case CMD_OPCODE_LIF_INIT:
    case CMD_OPCODE_ADMINQ_INIT:

        /*
         * These commands are issued before the lif adminQ is initialized.
         * To emulate the same behavior as the sonic driver, we issue them
         * to the devcmdpa directly.
         */
        dev_cmd_curr = nicmgr_devcmdpa_buf;
        break;

    default:

        /*
         * All other commands will use the sequencer lif adminQ
         */
        dev_cmd_curr = dev_cmd_buf;
        nicmgr_devcmdpa_cmdcpl->clear_thru();
        break;
    }

    dev_cmd_curr->clear();
    dev_cmd = (dev_cmd_regs_t *)dev_cmd_curr->read();

    dev_cmd->signature = DEV_CMD_SIGNATURE;
    memcpy(&dev_cmd->cmd, &cmd, sizeof(dev_cmd->cmd));
    dev_cmd_curr->fragment_find(0, DEV_CMDREGS_CMDCPL_SIZE)->write_thru();

    if (dev_cmd_curr == nicmgr_devcmdpa_buf) {
        /*
         * Sending command thru devcmdpa so ring its doorbell
         */
        dev_cmd_db = (dev_cmd_db_t *)nicmgr_devcmddbpa_buf->read();
        dev_cmd_db->v = true;
        nicmgr_devcmddbpa_buf->write_thru();
    } else {
        post_buffer(seq_hw_lif_id, ADMIN, NICMGR_ADMIN_QID,
                    (uint8_t *)&dev_cmd->cmd, sizeof(dev_cmd->cmd));
        count = 0;
        while (!poll_queue(seq_hw_lif_id, ADMIN, NICMGR_ADMIN_QID)) {
            if (count == NICMGR_ADMIN_QPOLL_RETRIES) {
                printf("%s poll_queue timeout\n", __FUNCTION__);
                return nullptr;
            }
            usleep(10000);  // 10 ms
            count++;
        }
        consume_buffer(seq_hw_lif_id, ADMIN, NICMGR_ADMIN_QID,
            NULL, NULL);
    }

    auto cmd_done_poll = [&dev_cmd] () -> int
    {
        dev_cmd = (dev_cmd_regs_t *)nicmgr_devcmdpa_cmdcpl->read_thru();
        return dev_cmd->done ? 0 : -1;
    };

    poll(cmd_done_poll);
    dev_cmd = (dev_cmd_regs_t *)nicmgr_devcmdpa_cmdcpl->read_thru();
    if ((dev_cmd->cpl.status != DEVCMD_SUCCESS) || !dev_cmd->done) {
        printf("%s command %u failed status %u done %u\n", __FUNCTION__,
               dev_cmd->cmd.nop.opcode, dev_cmd->cpl.status, dev_cmd->done);
        return nullptr;
    }

    return dev_cmd;
}

}  // namespace nicmgr_if



