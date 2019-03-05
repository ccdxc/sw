#include <math.h>
#include "tests.hpp"
#include "utils.hpp"
#include "hal_if.hpp"
#include "nicmgr_if.hpp"
#include "qstate_if.hpp"
#include "logger.hpp"
#include "nic/sdk/platform/utils/qstate_mgr.hpp"
#include "nic/sdk/platform/capri/capri_state.hpp"
#include "platform/src/app/nicmgrd/src/delphic.hpp"

#include <assert.h>
#include <stdint.h>
#include <unistd.h>
#include <stdio.h>

#define ACCEL_DEV_CMD_ENUMERATE
#include "accel_dev.hpp"

/*
 * Size of dev_cmd_regs_t's cmd/cpl area without the data portion.
 */
#define DEV_CMDREGS_CMDCPL_SIZE         \
    (offsetof(dev_cmd_regs_t, cpl) + sizeof(dev_cmd_cpl_t))

using namespace sdk::platform::utils;
using namespace utils;

/*
 * Public symbols for libnicmgr to silence linker warnings
 */
class pciemgr *pciemgr;
namespace nicmgr {
    shared_ptr<nicmgr::NicMgrService> g_nicmgr_svc;
}

namespace nicmgr_if {

static DeviceManager *devmgr;
static Accel_PF     *accel_dev;
static dp_mem_t     *accel_devcmdpa_buf;
static dp_mem_t     *accel_devcmddbpa_buf;
static dp_mem_t     *accel_devcmdpa_cmdcpl;
static uint64_t     nicmgr_accel_lif_id;

static dev_cmd_regs_t *nicmgr_if_push(dev_cmd_t& cmd);


static int
sdk_trace_cb (sdk_trace_level_e trace_level,
              const char *format, ...)
{
    char       logbuf[1024];
    va_list    args;

    va_start(args, format);
    vsnprintf(logbuf, sizeof(logbuf), format, args);
    printf("%s", logbuf);
    va_end(args);
    return 0;
}

static void
sdk_init (void)
{
    sdk::lib::logger::init(sdk_trace_cb);
}

int
nicmgr_if_init(void)
{
    /*
     * Initializations needed by libnicmgr
     */
    utils::logger::init(true);
    sdk_init();
    sdk::platform::capri::capri_state_pd_init(NULL);

    /*
     * Interface with the Accel_PF device directly
     */
    devmgr = new DeviceManager(FLAGS_nicmgr_config_file,
                               FWD_MODE_CLASSIC_NIC, PLATFORM_SIM);
    devmgr->LoadConfig(FLAGS_nicmgr_config_file);
    devmgr->HalEventHandler(true);
    accel_dev = (Accel_PF *)devmgr->GetDevice("accel");
    if (!accel_dev) {
        printf("Failed to locate Accel_PF device\n");
        return -1;
    }
    qstate_if::set_nicmgr_pd_client(devmgr->GetPdClient());

    accel_devcmdpa_buf = new dp_mem_t((uint8_t *)accel_dev->devcmd_page_get(),
                                      1, ACCEL_DEV_PAGE_SIZE,
                                      DP_MEM_TYPE_HBM, DP_MEM_ALLOC_NO_FILL);
    accel_devcmddbpa_buf = new dp_mem_t((uint8_t *)accel_dev->devcmddb_page_get(),
                                        1, sizeof(dev_cmd_db_t),
                                        DP_MEM_TYPE_HBM, DP_MEM_ALLOC_NO_FILL);
    accel_devcmdpa_cmdcpl =
           accel_devcmdpa_buf->fragment_find(0, DEV_CMDREGS_CMDCPL_SIZE);

    printf("%s accel_devcmd_page 0x%lx accel_devcmddb_page 0x%lx\n",
           __FUNCTION__, accel_devcmdpa_buf->pa(), accel_devcmddbpa_buf->pa());
    return 0;
}


void
nicmgr_if_fini(void)
{
    if (devmgr) {
        devmgr->ThreadsWaitJoin();
    }
    if (utils::logger::logger()) {
        utils::logger::logger()->flush();
    }
}


int
nicmgr_if_identify(uint64_t *ret_seq_lif,
                   uint32_t *ret_seq_queues_per_lif,
                   accel_ring_t *ret_accel_ring_tbl,
                   uint32_t accel_ring_tbl_size)
{
    dev_cmd_regs_t  *dev_cmd;
    identity_t      *identity;
    identify_cpl_t  *cpl;
    dev_cmd_t       cmd = {0};

    *ret_seq_lif = 0;
    *ret_seq_queues_per_lif = 0;

    cmd.identify.opcode = CMD_OPCODE_IDENTIFY;
    cmd.identify.ver = IDENTITY_VERSION_1;
    dev_cmd = nicmgr_if_push(cmd);
    if (dev_cmd) {

        /*
         * Read the full result buffer including the data portion.
         */
        dev_cmd = (dev_cmd_regs_t *)accel_devcmdpa_buf->read_thru();
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

        nicmgr_accel_lif_id = identity->dev.lif_tbl[0].hw_lif_id;
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

        *ret_seq_lif = nicmgr_accel_lif_id;
        *ret_seq_queues_per_lif = identity->dev.seq_queues_per_lif;
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
    dev_cmd_regs_t  *dev_cmd;

    accel_devcmdpa_buf->clear();
    dev_cmd = (dev_cmd_regs_t *)accel_devcmdpa_buf->read();

    dev_cmd->signature = DEV_CMD_SIGNATURE;
    memcpy(&dev_cmd->cmd, &cmd, sizeof(dev_cmd->cmd));
    accel_devcmdpa_buf->fragment_find(0, DEV_CMDREGS_CMDCPL_SIZE)->write_thru();

    /*
     * Sending command thru devcmdpa so ring its doorbell.
     *
     * Note: we don't have a way to trigger Accel_PF timer so just go
     * directly to its DevcmdHandler.
    dev_cmd_db = (dev_cmd_db_t *)accel_devcmddbpa_buf->read();
    dev_cmd_db->v = true;
    accel_devcmddbpa_buf->write_thru();
     */
    accel_dev->DevcmdHandler();

    dev_cmd = (dev_cmd_regs_t *)accel_devcmdpa_cmdcpl->read_thru();
    if ((dev_cmd->cpl.status != 0) || !dev_cmd->done) {
        printf("%s command %u failed status %u done %u\n", __FUNCTION__,
               dev_cmd->cmd.nop.opcode, dev_cmd->cpl.status, dev_cmd->done);
        return nullptr;
    }

    return dev_cmd;
}

}  // namespace nicmgr_if



