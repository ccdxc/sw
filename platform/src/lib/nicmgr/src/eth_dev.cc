/*
* Copyright (c) 2018, Pensando Systems Inc.
*/

#include <cstdio>
#include <iostream>
#include <iomanip>
#include <algorithm>
#include <cmath>
#include <endian.h>
#include <sstream>

#include "cap_top_csr_defines.h"
#include "cap_pics_c_hdr.h"
#include "cap_wa_c_hdr.h"

#include "nic/include/base.hpp"
#include "nic/sdk/lib/thread/thread.hpp"
#include "nic/p4/common/defines.h"

#include "gen/platform/mem_regions.hpp"
#include "gen/proto/nicmgr/nicmgr.pb.h"
#include "gen/proto/nicmgr/metrics.delphi.hpp"
#include "gen/proto/common/nicmgr_status_msgs.pb.h"
#include "gen/proto/common/nicmgr_status_msgs.delphi.hpp"

#include "platform/src/lib/misc/include/misc.h"
#include "platform/src/lib/intrutils/include/intrutils.h"
#include "platform/src/lib/pciemgr_if/include/pciemgr_if.hpp"
#include "platform/src/lib/hal_api/include/print.hpp"
#include "platform/src/app/nicmgrd/src/delphic.hpp"

#include "logger.hpp"
#include "eth_dev.hpp"
#include "rdma_dev.hpp"
#include "pd_client.hpp"
#include "hal_client.hpp"


using namespace nicmgr;
using namespace nicmgr_status_msgs;

using nicmgr::EthDeviceInfo;
using nicmgr::MacAddrVlans;
using nicmgr::MacAddrs;
using nicmgr::Vlans;
using nicmgr::LifInfo;
using nicmgr::QstateAddr;
using nicmgr_status_msgs::EthDeviceHostDownStatusMsg;
using nicmgr_status_msgs::EthDeviceHostUpStatusMsg;

extern class pciemgr *pciemgr;

static uint8_t *
memrev (uint8_t *block, size_t elnum)
{
    uint8_t *s, *t, tmp;

    for (s = block, t = s + (elnum - 1); s < t; s++, t--) {
        tmp = *s;
        *s = *t;
        *t = tmp;
    }
    return block;
}

template<typename ... Args>
string string_format( const std::string& format, Args ... args )
{
    size_t size = snprintf( nullptr, 0, format.c_str(), args ... ) + 1 /* for '\0' */;
    unique_ptr<char[]> buf( new char[ size ] );
    snprintf( buf.get(), size, format.c_str(), args ... );
    return string( buf.get(), buf.get() + size - 1 ); // ignore '\0'
}

sdk::lib::indexer *Eth::fltr_allocator = sdk::lib::indexer::factory(4096);

Eth::Eth(HalClient *hal_client,
         HalCommonClient *hal_common_client,
         void *dev_spec,
         hal_lif_info_t *nicmgr_lif_info,
         PdClient *pd_client)
{
    hal = hal_client;
    hal_common_client = hal_common_client;
    spec = (struct eth_devspec *)dev_spec;
    Eth::nicmgr_lif_info = nicmgr_lif_info;
    pd = pd_client;

    // Allocate lifs
    lif_base = pd->lm_->LIFRangeAlloc(-1, spec->lif_count);
    if (lif_base < 0) {
        NIC_LOG_ERR("{}: Failed to allocate lifs", spec->name);
        throw;
    }
    NIC_LOG_DEBUG("{}: lif_base {} lif_count {}", spec->name, lif_base, spec->lif_count);

    // Allocate interrupts
    intr_base = pd->intr_alloc(spec->intr_count);
    if (intr_base < 0) {
        NIC_LOG_ERR("{}: Failed to allocate interrupts", spec->name);
        throw;
    }
    NIC_LOG_DEBUG("{}: intr_base {} intr_count {}", spec->name, intr_base, spec->intr_count);

    // Allocate & Init Devcmd Region
    devcmd_mem_addr = pd->devcmd_mem_alloc(4096);
    if (devcmd_mem_addr == 0) {
        NIC_LOG_ERR("{}: Failed to allocate devcmd region", spec->name);
        throw;
    }
    MEM_SET(devcmd_mem_addr, 0, 4096, 0);
    // TODO: mmap instead of calloc after porting to real pal
    devcmd = (struct dev_cmd_regs *)calloc(1, sizeof(struct dev_cmd_regs));
    if (devcmd == NULL) {
        NIC_LOG_ERR("{}: Failed to map devcmd region", spec->name);
        throw;
    }
    devcmd->signature = DEV_CMD_SIGNATURE;
    WRITE_MEM(devcmd_mem_addr, (uint8_t *)devcmd, sizeof(*devcmd), 0);

    devcmddb_mem_addr = pd->devcmd_mem_alloc(4096);
    if (devcmddb_mem_addr == 0) {
        NIC_LOG_ERR("{}: Failed to allocate devcmddb region", spec->name);
        throw;
    }
    MEM_SET(devcmddb_mem_addr, 0, 4096, 0);

    NIC_LOG_DEBUG("{}: devcmd_addr {:#x} devcmddb_addr {:#x}",
        spec->name, devcmd_mem_addr, devcmddb_mem_addr);

    // Allocate CMB region
    if (spec->enable_rdma && spec->barmap_size) {
        cmb_mem_size = (spec->barmap_size << MEM_BARMAP_SIZE_SHIFT);
        assert (cmb_mem_size <= (8 * 1024 * 1024));

        cmb_mem_addr = pd->rdma_mem_bar_alloc(cmb_mem_size);
        assert (cmb_mem_addr != 0);
        // bar address must be aligned to bar size
        assert ((cmb_mem_size % cmb_mem_size) == 0);

        NIC_LOG_DEBUG("{}: cmb_mem_addr: {:#x}, cmb_mem_size: {}, ",
                      spec->name, cmb_mem_addr, cmb_mem_size);
    } else {
        cmb_mem_addr = 0;
        cmb_mem_size = 0;
    }

    // Create the device
    if (spec->eth_type == ETH_HOST_MGMT || spec->eth_type == ETH_HOST) {
        if (!CreateHostDevice()) {
            NIC_LOG_ERR("{}: Failed to create device", spec->name);
            throw;
        }
    } else {
        NIC_LOG_DEBUG("{}: Skipped creating device", spec->name);
    }

    // Create LIF
    lif_state = LIF_STATE_CREATING;

    memset(qinfo, 0, sizeof(qinfo));

    qinfo[ETH_QTYPE_RX] = {
        .type_num = ETH_QTYPE_RX,
        .size = 1,
        .entries = (uint32_t)log2(spec->rxq_count),
    };

    qinfo[ETH_QTYPE_TX] = {
        .type_num = ETH_QTYPE_TX,
        .size = 1,
        .entries = (uint32_t)log2(spec->txq_count),
    };

    qinfo[ETH_QTYPE_ADMIN] = {
        .type_num = ETH_QTYPE_ADMIN,
        .size = 2,
        .entries = (uint32_t)log2(spec->adminq_count + spec->rdma_adminq_count),
    };

    qinfo[ETH_QTYPE_SQ] = {
        .type_num = ETH_QTYPE_SQ,
        .size = 4,
        .entries = (uint32_t)log2(spec->rdma_sq_count),
    };

    qinfo[ETH_QTYPE_RQ] = {
        .type_num = ETH_QTYPE_RQ,
        .size = 4,
        .entries = (uint32_t)log2(spec->rdma_rq_count),
    };

    qinfo[ETH_QTYPE_CQ] = {
        .type_num = ETH_QTYPE_CQ,
        .size = 1,
        .entries = (uint32_t)log2(spec->rdma_cq_count),
    };

    qinfo[ETH_QTYPE_EQ] = {
        .type_num = ETH_QTYPE_EQ,
        .size = 1,
        .entries = (uint32_t)log2(spec->eq_count + spec->rdma_eq_count),
    };

    qinfo[ETH_QTYPE_SVC] = {
        .type_num = ETH_QTYPE_SVC,
        .size = 1,
        .entries = 1,
    };

    memset(&hal_lif_info_, 0, sizeof(hal_lif_info_t));
    hal_lif_info_.hw_lif_id = lif_base;
    hal_lif_info_.name = spec->name;
    hal_lif_info_.type = ConvertDevTypeToLifType(spec->eth_type);
    hal_lif_info_.pinned_uplink_port_num = spec->uplink_port_num;
    hal_lif_info_.enable_rdma = spec->enable_rdma;
    hal_lif_info_.pushed_to_hal = false;
    memcpy(hal_lif_info_.queue_info, qinfo, sizeof(hal_lif_info_.queue_info));

    NIC_LOG_INFO("lif created: name {} hw_lif_id {}"
                 " mac {} uplink {}",
                 hal_lif_info_.name,
                 hal_lif_info_.hw_lif_id,
                 macaddr2str(spec->mac_addr),
                 spec->uplink_port_num);

    // Stats
    stats_mem_addr = pd->mem_start_addr(CAPRI_HBM_REG_LIF_STATS);
    if (stats_mem_addr == INVALID_MEM_ADDRESS) {
        NIC_LOG_ERR("{}: Failed to allocate stats region for lif {}", spec->name, lif_base);
        throw;
    }
    stats_mem_addr += (hal_lif_info_.hw_lif_id << LG2_LIF_STATS_SIZE);
    host_stats_mem_addr = 0;

    NIC_LOG_INFO("lif-{}: stats_mem_addr: {:#x}",
        hal_lif_info_.hw_lif_id, stats_mem_addr);

    auto lif_stats =
        delphi::objects::LifMetrics::NewLifMetrics(lif_base, stats_mem_addr);
    if (lif_stats == NULL) {
        NIC_LOG_ERR("lif-{}: Failed lif metrics registration with delphi",
                    hal_lif_info_.hw_lif_id);
        throw;
    }

    // Notify Queue
    notify_block_addr = pd->nicmgr_mem_alloc(sizeof(struct notify_block));
    host_notify_block_addr = 0;
    // TODO: mmap instead of calloc
    notify_block = (struct notify_block *)calloc(1, sizeof(struct notify_block));
    // notify_block = (struct notify_block *)pal_mem_map(notify_block_addr, sizeof(struct notify_block), 0);
    if (notify_block == NULL) {
        NIC_LOG_ERR("lif-{}: Failed to map notify block!", hal_lif_info_.hw_lif_id);
        throw;
    }
    MEM_SET(notify_block_addr, 0, sizeof(struct notify_block), 0);
    // memset(notify_block, 0, sizeof(struct notify_block));

    NIC_LOG_INFO("lif-{}: notify_block_addr {:#x}",
        hal_lif_info_.hw_lif_id, notify_block_addr);

    // Notify Block
    notify_ring_head = 0;
    notify_ring_base = pd->nicmgr_mem_alloc(4096 + (sizeof(union notifyq_comp) * ETH_NOTIFYQ_RING_SIZE));
    if (notify_ring_base == 0) {
        NIC_LOG_ERR("lif-{}: Failed to allocate notify ring!",
            hal_lif_info_.hw_lif_id);
        return;
    }
    MEM_SET(notify_ring_base, 0, 4096 + (sizeof(union notifyq_comp) * ETH_NOTIFYQ_RING_SIZE), 0);

    NIC_LOG_INFO("lif-{}: notify_ring_base {:#x}",
        hal_lif_info_.hw_lif_id, notify_ring_base);

    // Edma Queue
    edma_ring_head = 0;
    edma_ring_base = pd->nicmgr_mem_alloc(4096 + (sizeof(struct edma_cmd_desc) * ETH_EDMAQ_RING_SIZE));
    if (edma_ring_base == 0) {
        NIC_LOG_ERR("lif-{}: Failed to allocate edma ring!",
            hal_lif_info_.hw_lif_id);
        return;
    }
    MEM_SET(edma_ring_base, 0, 4096 + (sizeof(struct edma_cmd_desc) * ETH_EDMAQ_RING_SIZE), 0);

    edma_comp_tail = 0;
    edma_exp_color = 1;
    edma_comp_base = pd->nicmgr_mem_alloc(4096 + (sizeof(struct edma_comp_desc) * ETH_EDMAQ_RING_SIZE));
    if (edma_comp_base == 0) {
        NIC_LOG_ERR("lif-{}: Failed to allocate edma completion ring!",
            hal_lif_info_.hw_lif_id);
        return;
    }
    MEM_SET(edma_comp_base, 0, 4096 + (sizeof(struct edma_comp_desc) * ETH_EDMAQ_RING_SIZE), 0);

    NIC_LOG_INFO("lif-{}: edma_ring_base {:#x} edma_comp_base {:#x}",
        hal_lif_info_.hw_lif_id, edma_ring_base, edma_comp_base);

    evutil_timer_start(&devcmd_timer, Eth::DevcmdPoll, this, 0.0, 0.01);

    lif_state = LIF_STATE_CREATED;
}

static void *
create_mnet(void *obj)
{
    struct mnet_dev_create_req_t *req = (struct mnet_dev_create_req_t *)obj;

    NIC_LOG_INFO("{}: started mnet thread", req->iface_name);

    int ret = create_mnet(req);
    if (ret) {
        NIC_LOG_ERR("{}: Failed to create MNIC device. ret: {}",
            req->iface_name, ret);
        free(obj);
        return NULL;
    }

    free(obj);
    return NULL;
}

void
Eth::HalEventHandler(bool status)
{
    if (!status) {
        return;
    }

    // Create the MNIC devices
    if (spec->eth_type == ETH_MNIC_OOB_MGMT ||
        spec->eth_type == ETH_MNIC_INTERNAL_MGMT ||
        spec->eth_type == ETH_MNIC_INBAND_MGMT) {
        if (!CreateLocalDevice()) {
            NIC_LOG_ERR("{}: Failed to create device", spec->name);
        }
    }
}

bool
Eth::CreateLocalDevice()
{
    NIC_LOG_DEBUG("{}: Creating MNIC device", spec->name);

    struct mnet_dev_create_req_t *mnet_req = NULL;

    mnet_req = (struct mnet_dev_create_req_t *)calloc(1, sizeof(*mnet_req));
    if (mnet_req == NULL) {
        NIC_LOG_ERR("{}: Failed to allocate mnet request", spec->name);
        return false;
    }

    mnet_req->devcmd_pa = devcmd_mem_addr;
    mnet_req->devcmd_db_pa = devcmddb_mem_addr;
    mnet_req->doorbell_pa = DOORBELL_ADDR(lif_base);
    mnet_req->drvcfg_pa = intr_drvcfg_addr(intr_base);
    mnet_req->msixcfg_pa = intr_msixcfg_addr(intr_base);
    strcpy(mnet_req->iface_name, spec->name.c_str());

    NIC_LOG_DEBUG("{}: devcmd_pa: {:#x}, devcmddb_pa: {:#x}, doorbell_pa: {:#x},"
                    " drvcfg_pa: {:#x}, msixcfg_pa: {:#x}",
                    mnet_req->iface_name,
                    mnet_req->devcmd_pa,
                    mnet_req->devcmd_db_pa,
                    mnet_req->doorbell_pa,
                    mnet_req->drvcfg_pa,
                    mnet_req->msixcfg_pa);

    for (uint32_t intr = 0; intr < spec->intr_count; intr++) {
        intr_fwcfg(intr_base + intr, lif_base, 0, 0, 0, 0);
        intr_fwcfg_local(intr_base + intr, 1);
    }

#define NICMGRD_THREAD_ID_MNET 0
    sdk::lib::thread *mnet_thread = NULL;

    sdk::lib::thread::control_cores_mask_set(0x8);
    mnet_thread = sdk::lib::thread::factory(spec->name.c_str(),
                                            NICMGRD_THREAD_ID_MNET,
                                            sdk::lib::THREAD_ROLE_CONTROL,
                                            0x8,
                                            create_mnet,
                                            sched_get_priority_max(SCHED_OTHER),
                                            SCHED_OTHER,
                                            false); // yield
    if (mnet_thread == NULL) {
        NIC_LOG_ERR("{}: Unable to start mnet thread. Exiting!!", spec->name);
        return false;
    }

    mnet_thread->start(mnet_req);

    return true;
}

bool
Eth::LoadOprom()
{
    std::string rom_file_path;
    uint64_t rom_file_size, rom_bar_size;

    rom_mem_addr = 0;
    rom_mem_size = 0;

    // FIXME: Get the filepaths from catalog
#ifdef __aarch64__
    rom_file_path = "/platform/oprom/";
#else
    rom_file_path = "/sw/platform/gen/";
#endif
    switch (spec->oprom) {
        case OPROM_LEGACY:
            rom_file_path += "ionic.rom";
            break;
        case OPROM_UEFI:
            rom_file_path += "ionic.efirom";
            break;
        case OPROM_UNIFIED:
            NIC_LOG_ERR("Not implemented!");
            return true;
        case OPROM_UNKNOWN:
            NIC_LOG_DEBUG("{}: No oprom configured", spec->name);
            return true;
    };

    // FIXME: The same ROM bar can be shared by all ethernet devices. We
    // can make this a static method and call it once.
    NIC_LOG_DEBUG("{}: Opening oprom {}", spec->name, rom_file_path);
    FILE *rom_file = fopen(rom_file_path.c_str(), "rb");
    if (rom_file == NULL) {
        NIC_LOG_ERR("{}: Failed to open oprom : {}", spec->name, strerror(errno));
        return false;
    }

    // Get file size
    fseek(rom_file, 0L, SEEK_END);
    rom_file_size = ftell(rom_file);
    rewind(rom_file);

    // FIXME: Create a new memory region and allocator for OPROMs.
    rom_mem_size = roundup_power2(rom_file_size);
    rom_bar_size = roundup_power2(rom_file_size);
    rom_mem_addr = roundup(pd->nicmgr_mem_alloc(2*rom_bar_size), rom_bar_size);
    NIC_LOG_INFO("{}: rom_mem_addr {:#x} rom_mem_size {}"
                " rom_file_size {} rom_bar_size {}",
                spec->name, rom_mem_addr, rom_mem_size,
                rom_file_size, rom_bar_size);
    // Must be naturally aligned
    if ((rom_mem_addr % rom_bar_size) != 0) {
        NIC_LOG_ERR("{}: rom_mem_addr is not naturally aligned", spec->name);
        fclose(rom_file);
        rom_mem_addr = 0;
        rom_mem_size = 0;
        return false;
    }

    NIC_LOG_INFO("{}: Writing oprom", spec->name);
    uint64_t rom_addr = rom_mem_addr;
    uint8_t buf[4096] = {0};
    uint32_t bytes_read = 0;
    while (!feof(rom_file)) {
        bytes_read = fread(buf, sizeof(buf[0]), sizeof(buf), rom_file);
        WRITE_MEM(rom_addr, buf, bytes_read, 0);
        rom_addr += bytes_read;
    }
    // zero-out rest of the bar
    MEM_SET(rom_addr, 0, rom_mem_size - rom_file_size, 0);
    NIC_LOG_INFO("{}: Finished writing oprom", spec->name);

    fclose(rom_file);

    return true;
}

bool
Eth::CreateHostDevice()
{
    pciehdevice_resources_t pci_resources = {0};

    if (!LoadOprom()) {
        NIC_LOG_ERR("{}: Failed to load oprom", spec->name);
        // FIXME: error out after oproms are pacakged in the image
        // return false;
    }

    pci_resources.lif_valid = 1;
    pci_resources.lif = lif_base;
    pci_resources.intrb = intr_base;
    pci_resources.intrc = spec->intr_count;
    pci_resources.port = spec->pcie_port;
    pci_resources.npids = spec->rdma_pid_count;
    pci_resources.devcmdpa = devcmd_mem_addr;
    pci_resources.devcmddbpa = devcmddb_mem_addr;
    pci_resources.cmbpa = cmb_mem_addr;
    pci_resources.cmbsz = cmb_mem_size;
    pci_resources.rompa = rom_mem_addr;
    pci_resources.romsz = rom_mem_size;

    // Create PCI device
    if (spec->eth_type == ETH_HOST) {
        NIC_LOG_DEBUG("{}: Creating Host device", spec->name);
        pdev = pciehdev_eth_new(spec->name.c_str(), &pci_resources);
    } else if (spec->eth_type == ETH_HOST_MGMT) {
        NIC_LOG_DEBUG("{}: Creating Host Management device", spec->name);
        pdev = pciehdev_mgmteth_new(spec->name.c_str(), &pci_resources);
    } else {
        assert(0); // NOT REACHABLE
    }

    if (pdev == NULL) {
        NIC_LOG_ERR("{}: Failed to create PCI device", spec->name);
        return false;
    }
    pciehdev_set_priv(pdev, (void *)this);

    // Add device to PCI topology
    if (pciemgr) {
        int ret = pciemgr->add_device(pdev);
        if (ret != 0) {
            NIC_LOG_ERR("{}: Failed to add PCI device to topology", spec->name);
            return false;
        }
    }

    return true;
}

void
Eth::DevcmdPoll(void *obj)
{
    Eth             *dev = (Eth *)obj;
    dev_cmd_db_t    db = {0};
    dev_cmd_db_t    db_clear = {0};

    READ_MEM(dev->devcmddb_mem_addr, (uint8_t *)&db, sizeof(db), 0);
    if (db.v) {
        NIC_LOG_INFO("{}: Devcmd doorbell", dev->spec->name);
        WRITE_MEM(dev->devcmddb_mem_addr, (uint8_t *)&db_clear, sizeof(db_clear), 0);
        dev->DevcmdHandler();
    }
}

void
Eth::StatsUpdate(void *obj)
{
    Eth *eth = (Eth *)obj;

    if (eth->host_stats_mem_addr == 0) {
        NIC_LOG_DEBUG("lif-{}: Host stats region is not created!",
            eth->hal_lif_info_.hw_lif_id);
        return;
    }

    uint64_t addr, req_db_addr;

    // NIC_LOG_DEBUG("lif-{}: stats_mem_addr local {:#x} host {:#x}",
    //     eth->hal_lif_info_.hw_lif_id,
    //     eth->stats_mem_addr, eth->host_stats_mem_addr);

    struct edma_cmd_desc cmd = {
        .opcode = EDMA_OPCODE_LOCAL_TO_HOST,
        .len = sizeof(struct ionic_lif_stats),
        .src_lif = (uint16_t)eth->hal_lif_info_.hw_lif_id,
        .src_addr = eth->stats_mem_addr,
        .dst_lif = (uint16_t)eth->hal_lif_info_.hw_lif_id,
        .dst_addr = eth->host_stats_mem_addr,
    };

    // Update stats
    addr = eth->edma_ring_base + eth->edma_ring_head * sizeof(struct edma_cmd_desc);
    WRITE_MEM(addr, (uint8_t *)&cmd, sizeof(struct edma_cmd_desc), 0);
    req_db_addr =
#ifdef __aarch64__
                CAP_ADDR_BASE_DB_WA_OFFSET +
#endif
                CAP_WA_CSR_DHS_LOCAL_DOORBELL_BYTE_ADDRESS +
                (0b1011 /* PI_UPD + SCHED_SET */ << 17) +
                (eth->hal_lif_info_.hw_lif_id << 6) +
                (ETH_QTYPE_SVC << 3);

    // NIC_LOG_DEBUG("lif-{}: Updating stats, edma_idx {} edma_desc_addr {:#x}",
    //     eth->hal_lif_info_.hw_lif_id, eth->edma_ring_head, addr);
    eth->edma_ring_head = (eth->edma_ring_head + 1) % ETH_EDMAQ_RING_SIZE;
    WRITE_DB64(req_db_addr, (ETH_EDMAQ_ID << 24) | eth->edma_ring_head);

    // Wait for EDMA completion
    struct edma_comp_desc comp = {0};
    uint8_t npolls = 0;
    addr = eth->edma_comp_base + eth->edma_comp_tail * sizeof(struct edma_comp_desc);
    do {
        READ_MEM(addr, (uint8_t *)&comp, sizeof(struct edma_comp_desc), 0);
        usleep(ETH_EDMAQ_COMP_POLL_US);
    } while (comp.color != eth->edma_exp_color && ++npolls < ETH_EDMAQ_COMP_POLL_MAX);

    if (npolls == ETH_EDMAQ_COMP_POLL_MAX) {
        NIC_LOG_ERR("lif-{}: Stats update timeout", eth->hal_lif_info_.hw_lif_id);
    } else {
        eth->edma_comp_tail = (eth->edma_comp_tail + 1) % ETH_EDMAQ_RING_SIZE;
        if (eth->edma_comp_tail == 0) {
            eth->edma_exp_color = eth->edma_exp_color ? 0 : 1;
        }
    }
}

void
Eth::NotifyBlockUpdate(void *obj)
{
    Eth *eth = (Eth *)obj;

    if (eth->host_notify_block_addr == 0) {
        NIC_LOG_DEBUG("lif-{}: Host stats region is not created!",
            eth->hal_lif_info_.hw_lif_id);
        return;
    }

    uint64_t addr, req_db_addr;

    // NIC_LOG_DEBUG("lif-{}: notify_block_addr local {:#x} host {:#x}",
    //     eth->hal_lif_info_.hw_lif_id,
    //     eth->notify_block_addr, eth->host_notify_block_addr);

    struct edma_cmd_desc cmd = {
        .opcode = EDMA_OPCODE_LOCAL_TO_HOST,
        .len = sizeof(struct notify_block),
        .src_lif = (uint16_t)eth->hal_lif_info_.hw_lif_id,
        .src_addr = eth->notify_block_addr,
        .dst_lif = (uint16_t)eth->hal_lif_info_.hw_lif_id,
        .dst_addr = eth->host_notify_block_addr,
    };

    addr = eth->edma_ring_base + eth->edma_ring_head * sizeof(struct edma_cmd_desc);
    WRITE_MEM(addr, (uint8_t *)&cmd, sizeof(struct edma_cmd_desc), 0);
    req_db_addr =
#ifdef __aarch64__
                CAP_ADDR_BASE_DB_WA_OFFSET +
#endif
                CAP_WA_CSR_DHS_LOCAL_DOORBELL_BYTE_ADDRESS +
                (0b1011 /* PI_UPD + SCHED_SET */ << 17) +
                (eth->hal_lif_info_.hw_lif_id << 6) +
                (ETH_QTYPE_SVC << 3);

    // NIC_LOG_DEBUG("lif-{}: Updating notify block, eid {} edma_idx {} edma_desc_addr {:#x}",
    //     hal_lif_info_.hw_lif_id, notify_block->eid, edma_ring_head, addr);
    eth->edma_ring_head = (eth->edma_ring_head + 1) % ETH_EDMAQ_RING_SIZE;
    WRITE_DB64(req_db_addr, (ETH_EDMAQ_ID << 24) | eth->edma_ring_head);

    // Wait for EDMA completion
    struct edma_comp_desc comp = {0};
    uint8_t npolls = 0;
    addr = eth->edma_comp_base + eth->edma_comp_tail * sizeof(struct edma_comp_desc);
    do {
        READ_MEM(addr, (uint8_t *)&comp, sizeof(struct edma_comp_desc), 0);
        usleep(ETH_EDMAQ_COMP_POLL_US);
    } while (comp.color != eth->edma_exp_color && ++npolls < ETH_EDMAQ_COMP_POLL_MAX);

    if (npolls == ETH_EDMAQ_COMP_POLL_MAX) {
        NIC_LOG_ERR("lif-{}: Notify block update timeout", eth->hal_lif_info_.hw_lif_id);
    } else {
        eth->edma_comp_tail = (eth->edma_comp_tail + 1) % ETH_EDMAQ_RING_SIZE;
        if (eth->edma_comp_tail == 0) {
            eth->edma_exp_color = eth->edma_exp_color ? 0 : 1;
        }
    }
}

void
Eth::LinkEventHandler(port_status_t *evd)
{
    if (spec->uplink_port_num != evd->port_id) {
        return;
    }

    if (lif_state != LIF_STATE_INITED &&
        lif_state != LIF_STATE_UP &&
        lif_state != LIF_STATE_DOWN) {
        NIC_LOG_INFO("lif-{}: {} + {} => {}",
            hal_lif_info_.hw_lif_id,
            lif_state_to_str(lif_state),
            evd->oper_status ? "LINK_UP" : "LINK_DN",
            lif_state_to_str(lif_state));
        return;
    }

    // Update the local notify block
    ++notify_block->eid;
    notify_block->link_status = evd->oper_status;
    notify_block->link_error_bits = 0;
    notify_block->phy_type = 0;
    notify_block->link_speed = evd->oper_status ? evd->port_speed : 0;
    notify_block->autoneg_status = 0;
    ++notify_block->link_flap_count;
    WRITE_MEM(notify_block_addr, (uint8_t *)notify_block, sizeof(struct notify_block), 0);

    if (host_notify_block_addr == 0) {
        NIC_LOG_WARN("lif-{}: Host notify block is not created!", hal_lif_info_.hw_lif_id);
        return;
    }

    uint64_t addr, req_db_addr;

    Eth::NotifyBlockUpdate(this);

    // Send the link event notification
    struct link_change_event msg = {
        .eid = notify_block->eid,
        .ecode = EVENT_OPCODE_LINK_CHANGE,
        .link_status = evd->oper_status,
        .link_error_bits = 0,
        .phy_type = 0,
        .link_speed = evd->port_speed,
        .autoneg_status = 0,
    };

    addr = notify_ring_base + notify_ring_head * sizeof(union notifyq_comp);
    WRITE_MEM(addr, (uint8_t *)&msg, sizeof(union notifyq_comp), 0);
    req_db_addr =
#ifdef __aarch64__
                CAP_ADDR_BASE_DB_WA_OFFSET +
#endif
                CAP_WA_CSR_DHS_LOCAL_DOORBELL_BYTE_ADDRESS +
                (0b1011 /* PI_UPD + SCHED_SET */ << 17) +
                (hal_lif_info_.hw_lif_id << 6) +
                (ETH_QTYPE_SVC << 3);

    // NIC_LOG_DEBUG("lif-{}: Sending notify event, eid {} notify_idx {} notify_desc_addr {:#x}",
    //     hal_lif_info_.hw_lif_id, notify_block->eid, notify_ring_head, addr);
    notify_ring_head = (notify_ring_head + 1) % ETH_NOTIFYQ_RING_SIZE;
    WRITE_DB64(req_db_addr, (ETH_NOTIFYQ_ID << 24) | notify_ring_head);

    // FIXME: Wait for completion

    NIC_LOG_INFO("lif-{}: {} + {} => {}",
        hal_lif_info_.hw_lif_id,
        lif_state_to_str(lif_state),
        evd->oper_status ? "LINK_UP" : "LINK_DN",
        evd->oper_status ? lif_state_to_str(LIF_STATE_UP) : lif_state_to_str(LIF_STATE_DOWN));

    lif_state = evd->oper_status ? LIF_STATE_UP : LIF_STATE_DOWN;
}

void Eth::DevObjSave() {
    auto eth_dev_obj_ptr = make_shared<delphi::objects::EthDeviceInfo>();

    uint64_t mac_addr;
    uint16_t vlan;

    eth_dev_obj_ptr->set_key(spec->dev_uuid);
    eth_dev_obj_ptr->mutable_lif()->set_hw_lif_id(hal_lif_info_.hw_lif_id);
    for (int i = 0; i < NUM_QUEUE_TYPES; i++) {
        auto qstate_addr = eth_dev_obj_ptr->mutable_lif()->add_qstate_addr();
        qstate_addr->set_qstate_addr(hal_lif_info_.qstate_addr[i]);
    }
    eth_dev_obj_ptr->set_rss_type(rss_type);
    eth_dev_obj_ptr->set_rss_key((const char*) rss_key);
    eth_dev_obj_ptr->set_rss_indir((const char*) rss_indir);
    for (auto it = vlans.cbegin(); it != vlans.cend(); it++) {
        vlan = it->second;
        auto vlans = eth_dev_obj_ptr->add_vlans();
        vlans->set_vlan(vlan);
    }
    for (auto it = mac_addrs.cbegin(); it != mac_addrs.cend(); it++) {
        mac_addr = it->second;
        auto mac_addresses = eth_dev_obj_ptr->add_mac_addrs();
        mac_addresses->set_mac_addr(mac_addr);
    }
    g_nicmgr_svc->sdk()->SetObject(eth_dev_obj_ptr);
}

void
Eth::DevcmdHandler()
{
    enum DevcmdStatus status;

    NIC_HEADER_TRACE("Devcmd");

    // read devcmd region
    READ_MEM(devcmd_mem_addr, (uint8_t *)devcmd,
             sizeof(struct dev_cmd_regs), 0);

    if (devcmd->done != 0) {
        NIC_LOG_ERR("{}: Devcmd done is set before processing command, opcode {}",
            spec->name,
            opcode_to_str((enum cmd_opcode)devcmd->cmd.cmd.opcode));
        status = DEVCMD_ERROR;
        goto devcmd_done;
    }

    if (devcmd->signature != DEV_CMD_SIGNATURE) {
        NIC_LOG_ERR("{}: Devcmd signature mismatch, opcode {}",
            spec->name,
            opcode_to_str((enum cmd_opcode)devcmd->cmd.cmd.opcode));
        status = DEVCMD_ERROR;
        goto devcmd_done;
    }

    status = CmdHandler(&devcmd->cmd, &devcmd->data, &devcmd->comp, &devcmd->data);

    // write data
    if (status == DEVCMD_SUCCESS) {
        WRITE_MEM(devcmd_mem_addr + offsetof(struct dev_cmd_regs, data),
                  (uint8_t *)devcmd + offsetof(struct dev_cmd_regs, data),
                  sizeof(devcmd->data), 0);
    }

devcmd_done:
    devcmd->comp.comp.status = status;
    devcmd->done = 1;

    // write completion
    WRITE_MEM(devcmd_mem_addr + offsetof(struct dev_cmd_regs, comp),
              (uint8_t *)devcmd + offsetof(struct dev_cmd_regs, comp),
              sizeof(devcmd->comp), 0);

    // write done
    WRITE_MEM(devcmd_mem_addr + offsetof(struct dev_cmd_regs, done),
              (uint8_t *)devcmd + offsetof(struct dev_cmd_regs, done),
              sizeof(devcmd->done), 0);

    NIC_HEADER_TRACE("Devcmd End");
}

#define CASE(opcode) case opcode: return #opcode

const char*
Eth::opcode_to_str(enum cmd_opcode opcode)
{
    switch(opcode) {
        CASE(CMD_OPCODE_NOP);
        CASE(CMD_OPCODE_RESET);
        CASE(CMD_OPCODE_IDENTIFY);
        CASE(CMD_OPCODE_LIF_INIT);
        CASE(CMD_OPCODE_ADMINQ_INIT);
        CASE(CMD_OPCODE_TXQ_INIT);
        CASE(CMD_OPCODE_RXQ_INIT);
        CASE(CMD_OPCODE_FEATURES);
        CASE(CMD_OPCODE_SET_NETDEV_INFO);
        CASE(CMD_OPCODE_HANG_NOTIFY);
        CASE(CMD_OPCODE_Q_ENABLE);
        CASE(CMD_OPCODE_Q_DISABLE);
        CASE(CMD_OPCODE_NOTIFYQ_INIT);
        CASE(CMD_OPCODE_STATION_MAC_ADDR_GET);
        CASE(CMD_OPCODE_MTU_SET);
        CASE(CMD_OPCODE_RX_MODE_SET);
        CASE(CMD_OPCODE_RX_FILTER_ADD);
        CASE(CMD_OPCODE_RX_FILTER_DEL);
        CASE(CMD_OPCODE_STATS_DUMP_START);
        CASE(CMD_OPCODE_STATS_DUMP_STOP);
        CASE(CMD_OPCODE_DEBUG_Q_DUMP);
        CASE(CMD_OPCODE_RSS_HASH_SET);
        CASE(CMD_OPCODE_RSS_INDIR_SET);
        CASE(CMD_OPCODE_RDMA_RESET_LIF);
        CASE(CMD_OPCODE_RDMA_CREATE_EQ);
        CASE(CMD_OPCODE_RDMA_CREATE_CQ);
        CASE(CMD_OPCODE_RDMA_CREATE_ADMINQ);
        default: return "DEVCMD_UNKNOWN";
    }
}

const char*
Eth::lif_state_to_str(enum lif_state state)
{
    switch(state) {
        CASE(LIF_STATE_RESET);
        CASE(LIF_STATE_CREATING);
        CASE(LIF_STATE_CREATED);
        CASE(LIF_STATE_INITING);
        CASE(LIF_STATE_INITED);
        CASE(LIF_STATE_UP);
        CASE(LIF_STATE_DOWN);
        default: return "LIF_STATE_INVALID";
    }
}

enum DevcmdStatus
Eth::CmdHandler(void *req, void *req_data,
    void *resp, void *resp_data)
{
    union dev_cmd *cmd = (union dev_cmd *)req;
    union dev_cmd_comp *comp = (union dev_cmd_comp *)resp;
    enum DevcmdStatus status;

    NIC_LOG_DEBUG("{}: Handling cmd: {}", spec->name,
        opcode_to_str((enum cmd_opcode)cmd->cmd.opcode));

    switch (cmd->cmd.opcode) {

    // Device Commands
    case CMD_OPCODE_NOP:
        status = DEVCMD_SUCCESS;
        break;

    case CMD_OPCODE_RESET:
        status = this->_CmdReset(req, req_data, resp, resp_data);
        break;

    case CMD_OPCODE_IDENTIFY:
        status = this->_CmdIdentify(req, req_data, resp, resp_data);
        break;

    case CMD_OPCODE_LIF_INIT:
        status = this->_CmdLifInit(req, req_data, resp, resp_data);
        break;

    case CMD_OPCODE_LIF_RESET:
        status = this->_CmdLifReset(req, req_data, resp, resp_data);
        break;

    case CMD_OPCODE_ADMINQ_INIT:
        status = this->_CmdAdminQInit(req, req_data, resp, resp_data);
        break;

    // Admin Commands
    case CMD_OPCODE_TXQ_INIT:
        status = this->_CmdTxQInit(req, req_data, resp, resp_data);
        break;

    case CMD_OPCODE_RXQ_INIT:
        status = this->_CmdRxQInit(req, req_data, resp, resp_data);
        break;

    case CMD_OPCODE_FEATURES:
        status = this->_CmdFeatures(req, req_data, resp, resp_data);
        break;

    case CMD_OPCODE_SET_NETDEV_INFO:
        status = this->_CmdSetNetdevInfo(req, req_data, resp, resp_data);
        break;

    case CMD_OPCODE_HANG_NOTIFY:
        status = this->_CmdHangNotify(req, req_data, resp, resp_data);
        status = DEVCMD_SUCCESS;
        break;

    case CMD_OPCODE_Q_ENABLE:
        status = this->_CmdQEnable(req, req_data, resp, resp_data);
        break;

    case CMD_OPCODE_Q_DISABLE:
        status = this->_CmdQDisable(req, req_data, resp, resp_data);
        break;

    case CMD_OPCODE_NOTIFYQ_INIT:
        status = this->_CmdNotifyQInit(req, req_data, resp, resp_data);
        break;

    case CMD_OPCODE_STATION_MAC_ADDR_GET:
        status = this->_CmdMacAddrGet(req, req_data, resp, resp_data);
        break;

    case CMD_OPCODE_MTU_SET:
        status = DEVCMD_SUCCESS;
        break;

    case CMD_OPCODE_RX_MODE_SET:
        status = this->_CmdSetMode(req, req_data, resp, resp_data);
        break;

    case CMD_OPCODE_RX_FILTER_ADD:
        status = this->_CmdRxFilterAdd(req, req_data, resp, resp_data);
        break;

    case CMD_OPCODE_RX_FILTER_DEL:
        status = this->_CmdRxFilterDel(req, req_data, resp, resp_data);
        break;

    case CMD_OPCODE_STATS_DUMP_START:
        status = this->_CmdStatsDumpStart(req, req_data, resp, resp_data);
        break;

    case CMD_OPCODE_STATS_DUMP_STOP:
        status = this->_CmdStatsDumpStop(req, req_data, resp, resp_data);
        break;

    case CMD_OPCODE_DEBUG_Q_DUMP:
        status = DEVCMD_SUCCESS;
        break;

    case CMD_OPCODE_RSS_HASH_SET:
        status = this->_CmdRssHashSet(req, req_data, resp, resp_data);
        break;

    case CMD_OPCODE_RSS_INDIR_SET:
        status = this->_CmdRssIndirSet(req, req_data, resp, resp_data);
        break;

    case CMD_OPCODE_RDMA_RESET_LIF:
        status = DEVCMD_SUCCESS;
        break;

    case CMD_OPCODE_RDMA_CREATE_EQ:
        status = this->_CmdRDMACreateEQ(req, req_data, resp, resp_data);
        status = DEVCMD_SUCCESS;
        break;

    case CMD_OPCODE_RDMA_CREATE_CQ:
        status = this->_CmdRDMACreateCQ(req, req_data, resp, resp_data);
        status = DEVCMD_SUCCESS;
        break;

    case CMD_OPCODE_RDMA_CREATE_ADMINQ:
        status = this->_CmdRDMACreateAdminQ(req, req_data, resp, resp_data);
        status = DEVCMD_SUCCESS;
        break;

    default:
        NIC_LOG_ERR("{}: Unknown Opcode {}", spec->name, cmd->cmd.opcode);
        status = DEVCMD_UNKNOWN;
        break;
    }

    comp->comp.status = status;
    comp->comp.rsvd = 0xff;
    NIC_LOG_DEBUG("{}: Done cmd: {}, status: {}", spec->name,
        opcode_to_str((enum cmd_opcode)cmd->cmd.opcode), status);

    return (status);
}

enum DevcmdStatus
Eth::_CmdIdentify(void *req, void *req_data, void *resp, void *resp_data)
{
    union identity *rsp = (union identity *)resp_data;
    struct identify_comp *comp = (struct identify_comp *)resp;

    NIC_LOG_DEBUG("{}: CMD_OPCODE_IDENTIFY", spec->name);

    // TODO: Get these from hw
    rsp->dev.asic_type = 0x00;
    rsp->dev.asic_rev = 0xA0;
    sprintf((char *)&rsp->dev.serial_num, "naples");
    // TODO: Get this from sw
    sprintf((char *)&rsp->dev.fw_version, "v0.0.1");
    rsp->dev.nlifs = 1;
    // At least one ndbpgs_per_lif
    rsp->dev.nintrs = spec->intr_count;
    rsp->dev.ndbpgs_per_lif = MAX(spec->rdma_pid_count, 1);
    rsp->dev.nucasts_per_lif = 32;
    rsp->dev.nmcasts_per_lif = 32;
    rsp->dev.intr_coal_mult = 1;
    rsp->dev.intr_coal_div = 10;

    rsp->dev.rdma_version = 1;
    rsp->dev.rdma_qp_opcodes = 27;
    rsp->dev.rdma_admin_opcodes = 50;
    rsp->dev.nrdma_pts_per_lif = spec->pte_count;
    rsp->dev.nrdma_mrs_per_lif = spec->key_count;
    rsp->dev.nrdma_ahs_per_lif = spec->ah_count;
    rsp->dev.rdma_max_stride = 11;
    rsp->dev.rdma_cl_stride = 6;
    rsp->dev.rdma_pte_stride = 3;
    rsp->dev.rdma_rrq_stride = 5;
    rsp->dev.rdma_rsq_stride = 5;

    rsp->dev.tx_qtype.qtype = 0;
    rsp->dev.tx_qtype.qid_count = spec->txq_count;
    rsp->dev.tx_qtype.qid_base = 0;

    rsp->dev.rx_qtype.qtype = 0;
    rsp->dev.rx_qtype.qid_count = spec->rxq_count;
    rsp->dev.rx_qtype.qid_base = 0;

    rsp->dev.admin_qtype.qtype = 2;
    rsp->dev.admin_qtype.qid_count = spec->adminq_count;
    rsp->dev.admin_qtype.qid_base = 0;

    rsp->dev.notify_qtype.qtype = 6;
    rsp->dev.notify_qtype.qid_count = spec->eq_count;
    rsp->dev.notify_qtype.qid_base = 0;

    rsp->dev.rdma_aq_qtype.qtype = 2;
    rsp->dev.rdma_aq_qtype.qid_count = spec->rdma_adminq_count;
    rsp->dev.rdma_aq_qtype.qid_base = spec->adminq_count;

    rsp->dev.rdma_sq_qtype.qtype = 3;
    rsp->dev.rdma_sq_qtype.qid_count = spec->rdma_sq_count;
    rsp->dev.rdma_sq_qtype.qid_base = 0;

    rsp->dev.rdma_rq_qtype.qtype = 4;
    rsp->dev.rdma_rq_qtype.qid_count = spec->rdma_rq_count;
    rsp->dev.rdma_rq_qtype.qid_base = 0;

    rsp->dev.rdma_cq_qtype.qtype = 5;
    rsp->dev.rdma_cq_qtype.qid_count = spec->rdma_cq_count;
    rsp->dev.rdma_cq_qtype.qid_base = 0;

    rsp->dev.rdma_eq_qtype.qtype = 6;
    rsp->dev.rdma_eq_qtype.qid_count = spec->rdma_eq_count;
    rsp->dev.rdma_eq_qtype.qid_base = spec->eq_count;

    // XXX RDMA data path requires that RDMA AdminQ is QID 1 for now.
    rsp->dev.admin_qtype.qid_base = 0;
    rsp->dev.admin_qtype.qid_count = 1;
    rsp->dev.rdma_aq_qtype.qid_base = 1;
    rsp->dev.rdma_aq_qtype.qid_count = 1;
    // XXX Remove hardcode when flexibility is added.

    comp->ver = IDENTITY_VERSION_1;

    NIC_LOG_DEBUG("{}: asic_type {} asic_rev {} serial_num {} fw_version {} "
                 "ndbpgs_per_lif {} nintrs {} nucasts_per_lif {} nmcasts_per_lif {} "
                 "intr_coal_mult {} intr_coal_div {} "
                 "rdma_version {} rdma_qp_opcodes {} rdma_admin_opcodes {} "
                 "nrdma_pts_per_lif {} nrdma_mrs_per_lif {} "
                 "nrdma_ahs_per_lif {} rdma_max_stride {} "
                 "rdma_cl_stride {} rdma_pte_stride {} "
                 "rdma_rrq_stride {} rdma_rsq_stride {} "
                 "tx_qtype {} tx_qid_count {} tx_qid_base {} "
                 "rx_qtype {} rx_qid_count {} rx_qid_base {} "
                 "admin_qtype {} admin_qid_count {} admin_qid_base {} "
                 "notify_qtype {} notify_qid_count {} notify_qid_base {} "
                 "rdma_aq_qtype {} rdma_aq_qid_count {} rdma_aq_qid_base {} "
                 "rdma_sq_qtype {} rdma_sq_qid_count {} rdma_sq_qid_base {} "
                 "rdma_rq_qtype {} rdma_rq_qid_count {} rdma_rq_qid_base {} "
                 "rdma_cq_qtype {} rdma_cq_qid_count {} rdma_cq_qid_base {} "
                 "rdma_eq_qtype {} rdma_eq_qid_count {} rdma_eq_qid_base {}",
                 spec->name,
                 rsp->dev.asic_type, rsp->dev.asic_rev, rsp->dev.serial_num,
                 rsp->dev.fw_version, rsp->dev.ndbpgs_per_lif, rsp->dev.nintrs,
                 rsp->dev.nucasts_per_lif, rsp->dev.nmcasts_per_lif,
                 rsp->dev.intr_coal_mult, rsp->dev.intr_coal_div,
                 rsp->dev.rdma_version, rsp->dev.rdma_qp_opcodes, rsp->dev.rdma_admin_opcodes,
                 rsp->dev.nrdma_pts_per_lif, rsp->dev.nrdma_mrs_per_lif, rsp->dev.nrdma_ahs_per_lif,
                 rsp->dev.rdma_max_stride, rsp->dev.rdma_cl_stride, rsp->dev.rdma_pte_stride,
                 rsp->dev.rdma_rrq_stride, rsp->dev.rdma_rsq_stride,
                 rsp->dev.tx_qtype.qtype, rsp->dev.tx_qtype.qid_count, rsp->dev.tx_qtype.qid_base,
                 rsp->dev.rx_qtype.qtype, rsp->dev.rx_qtype.qid_count, rsp->dev.rx_qtype.qid_base,
                 rsp->dev.admin_qtype.qtype, rsp->dev.admin_qtype.qid_count, rsp->dev.admin_qtype.qid_base,
                 rsp->dev.notify_qtype.qtype, rsp->dev.notify_qtype.qid_count, rsp->dev.notify_qtype.qid_base,
                 rsp->dev.rdma_aq_qtype.qtype, rsp->dev.rdma_aq_qtype.qid_count, rsp->dev.rdma_aq_qtype.qid_base,
                 rsp->dev.rdma_sq_qtype.qtype, rsp->dev.rdma_sq_qtype.qid_count, rsp->dev.rdma_sq_qtype.qid_base,
                 rsp->dev.rdma_rq_qtype.qtype, rsp->dev.rdma_rq_qtype.qid_count, rsp->dev.rdma_rq_qtype.qid_base,
                 rsp->dev.rdma_cq_qtype.qtype, rsp->dev.rdma_cq_qtype.qid_count, rsp->dev.rdma_cq_qtype.qid_base,
                 rsp->dev.rdma_eq_qtype.qtype, rsp->dev.rdma_eq_qtype.qid_count, rsp->dev.rdma_eq_qtype.qid_base);

    return (DEVCMD_SUCCESS);
}

enum DevcmdStatus
Eth::_CmdReset(void *req, void *req_data, void *resp, void *resp_data)
{
    EthLif *eth_lif = NULL;

    NIC_LOG_DEBUG("{}: CMD_OPCODE_RESET", spec->name);

    for (uint32_t intr = 0; intr < spec->intr_count; intr++) {
        intr_pba_clear(intr_base + intr);
        intr_drvcfg(intr_base + intr);
    }

    for (auto it = lif_map.cbegin(); it != lif_map.cend(); it++) {
        eth_lif = it->second;
        eth_lif->Reset();
        FreeUpMacFilters();
        FreeUpVlanFilters();
        FreeUpMacVlanFilters();
    }

    // RSS configuration
    rss_type = LifRssType::RSS_TYPE_NONE;
    memset(rss_key, 0x00, RSS_HASH_KEY_SIZE);
    memset(rss_indir, 0x00, RSS_IND_TBL_SIZE);

    lif_state = LIF_STATE_RESET;

    return (DEVCMD_SUCCESS);
}

enum DevcmdStatus
Eth::_CmdHangNotify(void *req, void *req_data, void *resp, void *resp_data)
{
    int64_t addr;
    eth_rx_qstate_t rx_qstate;
    eth_tx_qstate_t tx_qstate;
    admin_qstate_t adminq_qstate;
    intr_state_t intr_st;

    NIC_LOG_DEBUG("lif-{}: CMD_OPCODE_HANG_NOTIFY", hal_lif_info_.hw_lif_id);

    for (uint32_t qid = 0; qid < spec->rxq_count; qid++) {
        addr = pd->lm_->GetLIFQStateAddr(hal_lif_info_.hw_lif_id, ETH_QTYPE_RX, qid);
        if (addr < 0) {
            NIC_LOG_ERR("lif-{}: Failed to get qstate address for RX qid {}",
                hal_lif_info_.hw_lif_id, qid);
            return (DEVCMD_ERROR);
        }
        READ_MEM(addr, (uint8_t *)(&rx_qstate), sizeof(rx_qstate), 0);
        NIC_LOG_DEBUG("lif-{}: rxq{}: p_index0 {:#x} c_index0 {:#x} comp {:#x} intr {}",
            hal_lif_info_.hw_lif_id, qid,
            rx_qstate.p_index0, rx_qstate.c_index0, rx_qstate.comp_index,
            rx_qstate.intr_assert_index);
    }

    for (uint32_t qid = 0; qid < spec->txq_count; qid++) {
        addr = pd->lm_->GetLIFQStateAddr(hal_lif_info_.hw_lif_id, ETH_QTYPE_TX, qid);
        if (addr < 0) {
            NIC_LOG_ERR("lif-{}: Failed to get qstate address for TX qid {}",
                hal_lif_info_.hw_lif_id, qid);
            return (DEVCMD_ERROR);
        }
        READ_MEM(addr, (uint8_t *)(&tx_qstate), sizeof(tx_qstate), 0);
        NIC_LOG_DEBUG("lif-{}: txq{}: p_index0 {:#x} c_index0 {:#x} comp {:#x} intr {}",
            hal_lif_info_.hw_lif_id, qid,
            tx_qstate.p_index0, tx_qstate.c_index0, tx_qstate.comp_index,
            tx_qstate.intr_assert_index);
    }

    for (uint32_t qid = 0; qid < spec->adminq_count; qid++) {
        addr = pd->lm_->GetLIFQStateAddr(hal_lif_info_.hw_lif_id, ETH_QTYPE_ADMIN, qid);
        if (addr < 0) {
            NIC_LOG_ERR("lif-{}: Failed to get qstate address for ADMIN qid {}",
                hal_lif_info_.hw_lif_id, qid);
            return (DEVCMD_ERROR);
        }
        READ_MEM(addr, (uint8_t *)(&adminq_qstate), sizeof(adminq_qstate), 0);
        NIC_LOG_DEBUG("lif-{}: adminq{}: p_index0 {:#x} c_index0 {:#x} comp {:#x} intr {}",
            hal_lif_info_.hw_lif_id, qid,
            adminq_qstate.p_index0, adminq_qstate.c_index0, adminq_qstate.comp_index,
            adminq_qstate.intr_assert_index);
    }

    for (uint32_t intr = 0; intr < spec->intr_count; intr++) {
        intr_state(intr_base + intr, &intr_st);
        NIC_LOG_DEBUG("lif-{}: intr{}: fwcfg_lif {} fwcfg_function_mask {}"
            " drvcfg_mask {} drvcfg_int_credits {} drvcfg_mask_on_assert {}",
            hal_lif_info_.hw_lif_id, intr_base + intr,
            intr_st.fwcfg_lif, intr_st.fwcfg_function_mask,
            intr_st.drvcfg_mask, intr_st.drvcfg_int_credits,
            intr_st.drvcfg_mask_on_assert);
    }

    return (DEVCMD_SUCCESS);
}

enum DevcmdStatus
Eth::_CmdLifInit(void *req, void *req_data, void *resp, void *resp_data)
{
    struct lif_init_cmd *cmd = (struct lif_init_cmd *)req;
    int64_t addr;
    edma_qstate_t qstate;
    EthLif *eth_lif = NULL;
    uint64_t lif_id = lif_base + cmd->index;

    lif_state = LIF_STATE_INITING;

    NIC_LOG_DEBUG("{}: CMD_OPCODE_LIF_INIT: lif_index {}", spec->name, cmd->index);

    if (lif_map.find(lif_id) == lif_map.cend()) {

        // Create the LIF
        eth_lif = EthLif::Factory(&hal_lif_info_);
        if (eth_lif == NULL) {
            NIC_LOG_ERR("{}: Failed to create LIF {}",
                spec->name, hal_lif_info_.hw_lif_id);
            return (DEVCMD_ERROR);
        }

        lif_map[lif_id] = eth_lif;
        NIC_LOG_INFO("lif-{}: created", hal_lif_info_.hw_lif_id);

        cosA = 1;
        cosB = QosClass::GetTxTrafficClassCos(spec->qos_group, spec->uplink_port_num);
        if (cosB < 0) {
            NIC_LOG_ERR("lif-{}: Failed to get cosB for group {}, uplink {}",
                        hal_lif_info_.hw_lif_id, spec->qos_group,
                        spec->uplink_port_num);
            return (DEVCMD_ERROR);
        }
        uint8_t coses = (((cosB & 0x0f) << 4) | (cosA & 0x0f));

        pd->program_qstate((struct queue_info*)hal_lif_info_.queue_info,
            &hal_lif_info_, coses);

        if (spec->enable_rdma) {
            pd->rdma_lif_init(hal_lif_info_.hw_lif_id, spec->key_count,
                            spec->ah_count, spec->pte_count,
                            cmb_mem_addr, cmb_mem_size);
            // TODO: Handle error
        }
    }

    // Clear all non-intrinsic fields
    for (uint32_t qid = 0; qid < spec->rxq_count; qid++) {
        addr = pd->lm_->GetLIFQStateAddr(lif_id, ETH_QTYPE_RX, qid);
        if (addr < 0) {
            NIC_LOG_ERR("lif-{}: Failed to get qstate address for RX qid {}",
                lif_id, qid);
            return (DEVCMD_ERROR);
        }
        WRITE_MEM(addr + offsetof(eth_rx_qstate_t, p_index0),
                  (uint8_t *)(&qstate) + offsetof(eth_rx_qstate_t, p_index0),
                  sizeof(qstate) - offsetof(eth_rx_qstate_t, p_index0), 0);
        invalidate_rxdma_cacheline(addr);
    }

    for (uint32_t qid = 0; qid < spec->txq_count; qid++) {
        addr = pd->lm_->GetLIFQStateAddr(lif_id, ETH_QTYPE_TX, qid);
        if (addr < 0) {
            NIC_LOG_ERR("lif-{}: Failed to get qstate address for TX qid {}",
                lif_id, qid);
            return (DEVCMD_ERROR);
        }
        WRITE_MEM(addr + offsetof(eth_tx_qstate_t, p_index0),
                  (uint8_t *)(&qstate) + offsetof(eth_tx_qstate_t, p_index0),
                  sizeof(qstate) - offsetof(eth_tx_qstate_t, p_index0), 0);
        invalidate_txdma_cacheline(addr);
    }

    for (uint32_t qid = 0; qid < spec->adminq_count; qid++) {
        addr = pd->lm_->GetLIFQStateAddr(lif_id, ETH_QTYPE_ADMIN, qid);
        if (addr < 0) {
            NIC_LOG_ERR("lif-{}: Failed to get qstate address for ADMIN qid {}",
                lif_id, qid);
            return (DEVCMD_ERROR);
        }
        WRITE_MEM(addr + offsetof(admin_qstate_t, p_index0),
                  (uint8_t *)(&qstate) + offsetof(admin_qstate_t, p_index0),
                  sizeof(qstate) - offsetof(admin_qstate_t, p_index0), 0);
        invalidate_txdma_cacheline(addr);
    }

    addr = pd->lm_->GetLIFQStateAddr(lif_id, ETH_QTYPE_SVC, ETH_EDMAQ_ID);
    if (addr < 0) {
        NIC_LOG_ERR("lif-{}: Failed to get qstate address for SVC qid {}",
            lif_id, ETH_EDMAQ_ID);
        return (DEVCMD_ERROR);
    }

    MEM_SET(notify_ring_base, 0, 4096 + (sizeof(union notifyq_comp) * ETH_NOTIFYQ_RING_SIZE), 0);
    MEM_SET(edma_ring_base, 0, 4096 + (sizeof(struct edma_cmd_desc) * ETH_EDMAQ_RING_SIZE), 0);
    MEM_SET(edma_comp_base, 0, 4096 + (sizeof(struct edma_comp_desc) * ETH_EDMAQ_RING_SIZE), 0);

    edma_ring_head = 0;
    edma_comp_tail = 0;
    edma_exp_color = 1;

    // Initialize the EDMA queue
    uint8_t off;
    if (pd->lm_->GetPCOffset("p4plus", "txdma_stage0.bin", "edma_stage0", &off) < 0) {
        NIC_LOG_ERR("Failed to get PC offset of program: txdma_stage0.bin label: edma_stage0");
        return (DEVCMD_ERROR);
    }
    qstate.pc_offset = off;
    qstate.cos_sel = 0;
    qstate.cosA = 0;
    qstate.cosB = cosB;
    qstate.host = 0;
    qstate.total = 1;
    qstate.pid = 0;
    qstate.p_index0 = edma_ring_head;
    qstate.c_index0 = 0;
    qstate.comp_index = edma_comp_tail;
    qstate.sta.color = edma_exp_color;
    qstate.cfg.enable = 1;
    qstate.ring_base = edma_ring_base;
    qstate.ring_size = LG2_ETH_EDMAQ_RING_SIZE;
    qstate.cq_ring_base = edma_comp_base;
    qstate.cfg.intr_enable = 0;
    qstate.intr_assert_index = 0;
    WRITE_MEM(addr, (uint8_t *)&qstate, sizeof(qstate), 0);

    invalidate_txdma_cacheline(addr);

    lif_state = LIF_STATE_INITED;

    return (DEVCMD_SUCCESS);
}

enum DevcmdStatus
Eth::_CmdLifReset(void *req, void *req_data, void *resp, void *resp_data)
{
    struct lif_reset_cmd *cmd = (struct lif_reset_cmd *)req;
    EthLif *eth_lif = NULL;
    uint64_t lif_id = lif_base + cmd->index;

    NIC_LOG_DEBUG("{}: CMD_OPCODE_LIF_RESET: lif_index {}",
                spec->name,
                cmd->index);

    auto it = lif_map.find(lif_id);
    if (it == lif_map.cend()) {
        NIC_FUNC_ERR("{}: Unable to find lif {}", spec->name, lif_id);
        return (DEVCMD_ERROR);
    }
    eth_lif = it->second;
    eth_lif->Reset();
    FreeUpMacFilters();
    FreeUpVlanFilters();
    FreeUpMacVlanFilters();

    lif_state = LIF_STATE_RESET;

    return (DEVCMD_SUCCESS);
}

enum DevcmdStatus
Eth::_CmdAdminQInit(void *req, void *req_data, void *resp, void *resp_data)
{
    int64_t addr;
    struct adminq_init_cmd *cmd = (struct adminq_init_cmd *)req;
    struct adminq_init_comp *comp = (struct adminq_init_comp *)resp;
    admin_qstate_t qstate;
    uint64_t lif_id = lif_base + cmd->lif_index;

    NIC_LOG_DEBUG("{}: CMD_OPCODE_ADMINQ_INIT: "
        "lif_index {} queue_index {} ring_base {:#x} ring_size {} intr_index {}",
        spec->name,
        cmd->lif_index,
        cmd->index,
        cmd->ring_base,
        cmd->ring_size,
        cmd->intr_index);

    if (cmd->lif_index >= spec->lif_count) {
        NIC_LOG_ERR("{}: bad lif index {}", spec->name, cmd->lif_index);
        return (DEVCMD_ERROR);
    }

    if (cmd->index >= spec->adminq_count) {
        NIC_LOG_ERR("lif-{}: bad qid {}", lif_id, cmd->index);
        return (DEVCMD_ERROR);
    }

    if (cmd->intr_index >= spec->intr_count) {
        NIC_LOG_ERR("lif-{}: bad intr {}", lif_id, cmd->intr_index);
        return (DEVCMD_ERROR);
    }

    if (cmd->ring_size < 2 || cmd->ring_size > 16) {
        NIC_LOG_ERR("lif-{}: bad ring size {}", lif_id, cmd->ring_size);
        return (DEVCMD_ERROR);
    }

    addr = pd->lm_->GetLIFQStateAddr(lif_id, ETH_QTYPE_ADMIN, cmd->index);
    if (addr < 0) {
        NIC_LOG_ERR("lif-{}: Failed to get qstate address for ADMIN qid {}",
            lif_id, cmd->index);
        return (DEVCMD_ERROR);
    }

    uint8_t off;
    if (pd->lm_->GetPCOffset("p4plus", "txdma_stage0.bin", "adminq_stage0", &off) < 0) {
        NIC_LOG_ERR("Failed to get PC offset of program: txdma_stage0.bin label: adminq_stage0");
        return (DEVCMD_ERROR);
    }
    qstate.pc_offset = off;
    qstate.cos_sel = 0;
    qstate.cosA = 0;
    qstate.cosB = cosB;
    qstate.host = 1;
    qstate.total = 1;
    qstate.pid = cmd->pid;
    qstate.p_index0 = 0;
    qstate.c_index0 = 0;
    qstate.comp_index = 0;
    qstate.ci_fetch = 0;
    qstate.sta.color = 1;
    qstate.cfg.enable = 1;
    qstate.cfg.host_queue = spec->host_dev;
    qstate.cfg.intr_enable = 1;
    if (spec->host_dev)
        qstate.ring_base = (1ULL << 63) | (hal_lif_info_.hw_lif_id << 52) | cmd->ring_base;
    else
        qstate.ring_base = cmd->ring_base;
    qstate.ring_size = cmd->ring_size;
    qstate.cq_ring_base = roundup(qstate.ring_base + (sizeof(union adminq_cmd) << cmd->ring_size), 4096);
    qstate.intr_assert_index = intr_base + cmd->intr_index;
    if (nicmgr_lif_info) {
        qstate.nicmgr_qstate_addr = nicmgr_lif_info->qstate_addr[NICMGR_QTYPE_REQ];
    }
    WRITE_MEM(addr, (uint8_t *)&qstate, sizeof(qstate), 0);

    invalidate_txdma_cacheline(addr);

    comp->qid = cmd->index;
    comp->qtype = ETH_QTYPE_ADMIN;

    NIC_LOG_DEBUG("lif-{}: qid {} qtype {}", lif_id, comp->qid, comp->qtype);
    return (DEVCMD_SUCCESS);
}

enum DevcmdStatus
Eth::_CmdTxQInit(void *req, void *req_data, void *resp, void *resp_data)
{
    int64_t addr;
    struct txq_init_cmd *cmd = (struct txq_init_cmd *)req;
    struct txq_init_comp *comp = (struct txq_init_comp *)resp;
    eth_tx_qstate_t qstate;

    NIC_LOG_DEBUG("lif-{}: CMD_OPCODE_TXQ_INIT: "
        "queue_index {} cos {} ring_base {:#x} ring_size {} intr_index {} {}{}",
        hal_lif_info_.hw_lif_id,
        cmd->index,
        cmd->cos,
        cmd->ring_base,
        cmd->ring_size,
        cmd->intr_index,
        cmd->I ? 'I' : '-',
        cmd->E ? 'E' : '-');

    if (cmd->index >= spec->txq_count) {
        NIC_LOG_ERR("lif-{}: bad qid {}", hal_lif_info_.hw_lif_id, cmd->index);
        return (DEVCMD_ERROR);
    }

    if (cmd->intr_index >= spec->intr_count) {
        NIC_LOG_ERR("lif-{}: bad intr {}", hal_lif_info_.hw_lif_id, cmd->intr_index);
        return (DEVCMD_ERROR);
    }

    if (cmd->ring_size < 2 || cmd->ring_size > 16) {
        NIC_LOG_ERR("lif-{}: bad ring_size {}", hal_lif_info_.hw_lif_id, cmd->ring_size);
        return (DEVCMD_ERROR);
    }

    addr = pd->lm_->GetLIFQStateAddr(hal_lif_info_.hw_lif_id, ETH_QTYPE_TX, cmd->index);
    if (addr < 0) {
        NIC_LOG_ERR("lif-{}: Failed to get qstate address for TX qid {}",
            hal_lif_info_.hw_lif_id, cmd->index);
        return (DEVCMD_ERROR);
    }

    uint8_t off;
    if (pd->lm_->GetPCOffset("p4plus", "txdma_stage0.bin", "eth_tx_stage0", &off) < 0) {
        NIC_LOG_ERR("Failed to get PC offset of program: txdma_stage0.bin label: eth_tx_stage0");
        return (DEVCMD_ERROR);
    }
    qstate.pc_offset = off;
    qstate.cos_sel = 0;
    qstate.cosA = 0;
    qstate.cosB = cmd->cos;
    qstate.host = 1;
    qstate.total = 1;
    qstate.pid = cmd->pid;
    qstate.p_index0 = 0;
    qstate.c_index0 = 0;
    qstate.comp_index = 0;
    qstate.ci_fetch = 0;
    qstate.ci_miss = 0;
    qstate.sta.color = 1;
    qstate.sta.spec_miss = 0;
    qstate.cfg.enable = cmd->E;
    qstate.cfg.host_queue = spec->host_dev;
    qstate.cfg.intr_enable = cmd->E;
    if (spec->host_dev)
        qstate.ring_base = (1ULL << 63) | (hal_lif_info_.hw_lif_id << 52) | cmd->ring_base;
    else
        qstate.ring_base = cmd->ring_base;
    qstate.ring_size = cmd->ring_size;
    qstate.cq_ring_base = roundup(qstate.ring_base + (sizeof(struct txq_desc) << cmd->ring_size), 4096);
    qstate.intr_assert_index = intr_base + cmd->intr_index;
    qstate.sg_ring_base = roundup(qstate.cq_ring_base + (sizeof(struct txq_comp) << cmd->ring_size), 4096);
    qstate.spurious_db_cnt = 0;
    WRITE_MEM(addr, (uint8_t *)&qstate, sizeof(qstate), 0);

    invalidate_txdma_cacheline(addr);

    comp->qid = cmd->index;
    comp->qtype = ETH_QTYPE_TX;

    NIC_LOG_DEBUG("lif-{}: qid {} qtype {}",
                 hal_lif_info_.hw_lif_id, comp->qid, comp->qtype);
    return (DEVCMD_SUCCESS);
}

enum DevcmdStatus
Eth::_CmdRxQInit(void *req, void *req_data, void *resp, void *resp_data)
{
    int64_t addr;
    struct rxq_init_cmd *cmd = (struct rxq_init_cmd *)req;
    struct rxq_init_comp *comp = (struct rxq_init_comp *)resp;
    eth_rx_qstate_t qstate;

    NIC_LOG_DEBUG("lif-{}: CMD_OPCODE_RXQ_INIT: "
        "queue_index {} ring_base {:#x} ring_size {} intr_index {} {}{}",
        hal_lif_info_.hw_lif_id,
        cmd->index,
        cmd->ring_base,
        cmd->ring_size,
        cmd->intr_index,
        cmd->I ? 'I' : '-',
        cmd->E ? 'E' : '-');

    if (cmd->index >= spec->rxq_count) {
        NIC_LOG_ERR("lif-{}: bad qid {}", hal_lif_info_.hw_lif_id, cmd->index);
        return (DEVCMD_ERROR);
    }

    if (cmd->intr_index >= spec->intr_count) {
        NIC_LOG_ERR("lif-{}: bad intr {}", hal_lif_info_.hw_lif_id, cmd->intr_index);
        return (DEVCMD_ERROR);
    }

    if (cmd->ring_size < 2 || cmd->ring_size > 16) {
        NIC_LOG_ERR("lif-{}: bad ring_size {}", hal_lif_info_.hw_lif_id, cmd->ring_size);
        return (DEVCMD_ERROR);
    }

    addr = pd->lm_->GetLIFQStateAddr(hal_lif_info_.hw_lif_id, ETH_QTYPE_RX, cmd->index);
    if (addr < 0) {
        NIC_LOG_ERR("lif-{}: Failed to get qstate address for RX qid {}",
            hal_lif_info_.hw_lif_id, cmd->index);
        return (DEVCMD_ERROR);
    }

    uint8_t off;
    if (pd->lm_->GetPCOffset("p4plus", "rxdma_stage0.bin", "eth_rx_stage0", &off) < 0) {
        NIC_LOG_ERR("Failed to get PC offset of program: rxdma_stage0.bin label: eth_rx_stage0");
        return (DEVCMD_ERROR);
    }
    qstate.pc_offset = off;
    qstate.cos_sel = 0;
    qstate.cosA = 0;
    qstate.cosB = 0;
    qstate.host = 1;
    qstate.total = 1;
    qstate.pid = cmd->pid;
    qstate.p_index0 = 0;
    qstate.c_index0 = 0;
    qstate.comp_index = 0;
    qstate.sta.color = 1;
    qstate.cfg.enable = cmd->E;
    qstate.cfg.host_queue = spec->host_dev;
    qstate.cfg.intr_enable = cmd->E;
    if (spec->host_dev)
        qstate.ring_base = (1ULL << 63) | (hal_lif_info_.hw_lif_id << 52) | cmd->ring_base;
    else
        qstate.ring_base = cmd->ring_base;
    qstate.ring_size = cmd->ring_size;
    qstate.cq_ring_base = roundup(qstate.ring_base + (sizeof(struct rxq_desc) << cmd->ring_size), 4096);
    qstate.intr_assert_index = intr_base + cmd->intr_index;
    WRITE_MEM(addr, (uint8_t *)&qstate, sizeof(qstate), 0);

    invalidate_rxdma_cacheline(addr);

    comp->qid = cmd->index;
    comp->qtype = ETH_QTYPE_RX;

    NIC_LOG_DEBUG("lif-{}: qid {} qtype {}",
                 hal_lif_info_.hw_lif_id, comp->qid, comp->qtype);
    return (DEVCMD_SUCCESS);
}

enum DevcmdStatus
Eth::_CmdNotifyQInit(void *req, void *req_data, void *resp, void *resp_data)
{
    int64_t addr;
    uint64_t host_ring_base;
    notify_qstate_t qstate;
    struct notifyq_init_cmd *cmd = (struct notifyq_init_cmd *)req;
    struct notifyq_init_comp *comp = (struct notifyq_init_comp *)resp;

    NIC_LOG_INFO("lif-{}: CMD_OPCODE_NOTIFYQ_INIT: "
        "queue_index {} ring_base {:#x} ring_size {} intr_index {} notify_base {:#x}",
        hal_lif_info_.hw_lif_id,
        cmd->index,
        cmd->ring_base,
        cmd->ring_size,
        cmd->intr_index,
        cmd->notify_base);

    if (cmd->index >= spec->eq_count) {
        NIC_LOG_ERR("lif-{}: bad qid {}", hal_lif_info_.hw_lif_id, cmd->index);
        return (DEVCMD_ERROR);
    }

    if (cmd->intr_index >= spec->intr_count) {
        NIC_LOG_ERR("lif-{}: bad intr {}", hal_lif_info_.hw_lif_id, cmd->intr_index);
        return (DEVCMD_ERROR);
    }

    if (cmd->ring_size < 2 || cmd->ring_size > 16) {
        NIC_LOG_ERR("lif-{}: bad ring_size {}", hal_lif_info_.hw_lif_id, cmd->ring_size);
        return (DEVCMD_ERROR);
    }

    addr = pd->lm_->GetLIFQStateAddr(hal_lif_info_.hw_lif_id, ETH_QTYPE_SVC, cmd->index);
    if (addr < 0) {
        NIC_LOG_ERR("lif-{}: Failed to get qstate address for SVC qid {}",
            hal_lif_info_.hw_lif_id, cmd->index);
        return (DEVCMD_ERROR);
    }

    notify_ring_head = 0;

    uint8_t off;
    if (pd->lm_->GetPCOffset("p4plus", "txdma_stage0.bin", "notify_stage0", &off) < 0) {
        NIC_LOG_ERR("Failed to resolve program: txdma_stage0.bin label: notify_stage0");
        return (DEVCMD_ERROR);
    }
    qstate.pc_offset = off;
    qstate.cos_sel = 0;
    qstate.cosA = 0;
    qstate.cosB = cosB;
    qstate.host = 0;
    qstate.total = 1;
    qstate.pid = cmd->pid;
    qstate.p_index0 = 0;
    qstate.c_index0 = 0;
    qstate.host_pindex = 0;
    qstate.sta = {0};
    qstate.cfg.enable = 1;
    qstate.cfg.host_queue = spec->host_dev;
    qstate.cfg.intr_enable = 1;
    qstate.ring_base = notify_ring_base;
    qstate.ring_size = LG2_ETH_NOTIFYQ_RING_SIZE;
    if (spec->host_dev)
        host_ring_base = (1ULL << 63) | (hal_lif_info_.hw_lif_id << 52) | cmd->ring_base;
    else
        host_ring_base = cmd->ring_base;
    qstate.host_ring_base = roundup(host_ring_base + (sizeof(notifyq_comp) << cmd->ring_size), 4096);
    qstate.host_ring_size = cmd->ring_size;
    qstate.host_intr_assert_index = intr_base + cmd->intr_index;
    WRITE_MEM(addr, (uint8_t *)&qstate, sizeof(qstate), 0);

    host_notify_block_addr = cmd->notify_base;
    NIC_LOG_INFO("lif-{}: host_notify_block_addr {:#x}",
                 hal_lif_info_.hw_lif_id, host_notify_block_addr);

    invalidate_txdma_cacheline(addr);

    // Init the notify block
    notify_block->eid = 1;
    if (spec->uplink_port_num) {
        port_status_t port_status;
        hal->PortStatusGet(spec->uplink_port_num, port_status);
        notify_block->link_status = port_status.oper_status;
        notify_block->link_speed = port_status.oper_status ? port_status.port_speed : 0;
    } else {
        notify_block->link_status = true;
        notify_block->link_speed = 1000; // 1 Gbps
    }
    notify_block->link_flap_count = 0;
    WRITE_MEM(notify_block_addr, (uint8_t *)notify_block, sizeof(struct notify_block), 0);

    Eth::NotifyBlockUpdate(this);

    comp->qid = cmd->index;
    comp->qtype = ETH_QTYPE_SVC;

    NIC_LOG_INFO("lif-{}: qid {} qtype {}", hal_lif_info_.hw_lif_id,
        comp->qid, comp->qtype);

    return (DEVCMD_SUCCESS);
}

enum DevcmdStatus
Eth::_CmdFeatures(void *req, void *req_data, void *resp, void *resp_data)
{
    struct features_cmd *cmd = (struct features_cmd *)req;
    struct features_comp *comp = (struct features_comp *)resp;
    EthLif *eth_lif = NULL;
    hal_irisc_ret_t ret = HAL_IRISC_RET_SUCCESS;

    NIC_LOG_DEBUG("lif-{}: CMD_OPCODE_FEATURES: wanted "
        "vlan_strip {} vlan_insert {} rx_csum {} tx_csum {} rx_hash {} sg {}",
        hal_lif_info_.hw_lif_id,
        (cmd->wanted & ETH_HW_VLAN_RX_STRIP) ? 1 : 0,
        (cmd->wanted & ETH_HW_VLAN_TX_TAG) ? 1 : 0,
        (cmd->wanted & ETH_HW_RX_CSUM) ? 1 : 0,
        (cmd->wanted & ETH_HW_TX_CSUM) ? 1 : 0,
        (cmd->wanted & ETH_HW_RX_HASH) ? 1 : 0,
        (cmd->wanted & ETH_HW_TX_SG)  ? 1 : 0
    );

    comp->status = 0;
    if (hal->fwd_mode == FWD_MODE_SMART_NIC) {
        comp->supported = (
            //ETH_HW_VLAN_RX_STRIP |
            //ETH_HW_VLAN_TX_TAG |
            ETH_HW_RX_CSUM |
            ETH_HW_TX_CSUM |
            ETH_HW_RX_HASH |
            ETH_HW_TX_SG |
            ETH_HW_TSO |
            ETH_HW_TSO_IPV6
        );
    } else {
        comp->supported = (
            ETH_HW_VLAN_RX_STRIP |
            ETH_HW_VLAN_TX_TAG |
            ETH_HW_VLAN_RX_FILTER |
            ETH_HW_RX_CSUM |
            ETH_HW_TX_CSUM |
            ETH_HW_RX_HASH |
            ETH_HW_TX_SG |
            ETH_HW_TSO |
            ETH_HW_TSO_IPV6
        );
    }

    auto it = lif_map.find(hal_lif_info_.hw_lif_id);
    if (it == lif_map.cend()) {
        NIC_FUNC_ERR("{}: Unable to find lif {}", spec->name, hal_lif_info_.hw_lif_id);
        return (DEVCMD_ERROR);
    }
    eth_lif = it->second;

    ret = eth_lif->UpdateVlanStripEn(cmd->wanted & comp->supported & ETH_HW_VLAN_RX_STRIP);
    if (ret != HAL_IRISC_RET_SUCCESS) {
        NIC_LOG_ERR("lif-{}: Failed to update VlanStrip offload",
            hal_lif_info_.hw_lif_id);
        return (DEVCMD_ERROR);
    }

    ret = eth_lif->UpdateVlanInsertEn(cmd->wanted & comp->supported & ETH_HW_VLAN_TX_TAG);
    if (ret != HAL_IRISC_RET_SUCCESS) {
        NIC_LOG_ERR("lif-{}: Failed to update VlanInsert offload",
            hal_lif_info_.hw_lif_id);
        return (DEVCMD_ERROR);
    }

    NIC_LOG_DEBUG("lif-{}: supported {}", hal_lif_info_.hw_lif_id, comp->supported);

    return (DEVCMD_SUCCESS);
}

enum DevcmdStatus
Eth::_CmdSetNetdevInfo(void *req, void *req_data, void *resp, void *resp_data)
{
    struct set_netdev_info_cmd *cmd = (struct set_netdev_info_cmd *)req;
    // set_netdev_info_comp *comp = (set_netdev_info_comp *)resp;

    NIC_LOG_DEBUG("{}: CMD_OPCODE_SET_NETDEV_INFO: nd_name {} dev_name {}",
        spec->name, cmd->nd_name, cmd->dev_name);

    nd_name = std::string(cmd->nd_name);
    dev_name = std::string(cmd->dev_name);

    return (DEVCMD_SUCCESS);
}

enum DevcmdStatus
Eth::_CmdQEnable(void *req, void *req_data, void *resp, void *resp_data)
{
    int64_t addr;
    struct q_enable_cmd *cmd = (struct q_enable_cmd *)req;
    // q_enable_comp *comp = (q_enable_comp *)resp;
    struct eth_rx_cfg_qstate rx_cfg = {0};
    struct eth_tx_cfg_qstate tx_cfg = {0};
    struct admin_cfg_qstate admin_cfg = {0};

    if (cmd->qtype >= 8) {
        NIC_LOG_ERR("lif-{}: CMD_OPCODE_Q_ENABLE: bad qtype {}",
        hal_lif_info_.hw_lif_id, cmd->qtype);
        return (DEVCMD_ERROR);
    }

    NIC_LOG_DEBUG("lif-{}: CMD_OPCODE_Q_ENABLE: type {} qid {}",
        hal_lif_info_.hw_lif_id, cmd->qtype, cmd->qid);

    switch (cmd->qtype) {
    case ETH_QTYPE_RX:
        if (cmd->qid >= spec->rxq_count) {
            NIC_LOG_ERR("lif-{}: CMD_OPCODE_Q_ENABLE: bad qid {}",
            hal_lif_info_.hw_lif_id, cmd->qid);
            return (DEVCMD_ERROR);
        }
        addr = pd->lm_->GetLIFQStateAddr(hal_lif_info_.hw_lif_id, cmd->qtype, cmd->qid);
        if (addr < 0) {
            NIC_LOG_ERR("lif-{}: Failed to get qstate address for RX qid {}",
                hal_lif_info_.hw_lif_id, cmd->qid);
            return (DEVCMD_ERROR);
        }
        rx_cfg.enable = 0x1;
        rx_cfg.host_queue = spec->host_dev;
        rx_cfg.intr_enable = 0x1;
        WRITE_MEM(addr + offsetof(eth_rx_qstate_t, cfg), (uint8_t *)&rx_cfg, sizeof(rx_cfg), 0);
        invalidate_rxdma_cacheline(addr);
        break;
    case ETH_QTYPE_TX:
        if (cmd->qid >= spec->txq_count) {
            NIC_LOG_ERR("lif-{}: CMD_OPCODE_Q_ENABLE: bad qid {}",
                   hal_lif_info_.hw_lif_id, cmd->qid);
            return (DEVCMD_ERROR);
        }
        addr = pd->lm_->GetLIFQStateAddr(hal_lif_info_.hw_lif_id, cmd->qtype, cmd->qid);
        if (addr < 0) {
            NIC_LOG_ERR("lif-{}: Failed to get qstate address for TX qid {}",
                hal_lif_info_.hw_lif_id, cmd->qid);
            return (DEVCMD_ERROR);
        }
        tx_cfg.enable = 0x1;
        tx_cfg.host_queue = spec->host_dev;
        tx_cfg.intr_enable = 0x1;
        WRITE_MEM(addr + offsetof(eth_tx_qstate_t, cfg), (uint8_t *)&tx_cfg, sizeof(tx_cfg), 0);
        invalidate_txdma_cacheline(addr);
        break;
    case ETH_QTYPE_ADMIN:
        if (cmd->qid >= spec->adminq_count) {
            NIC_LOG_ERR("lif-{}: CMD_OPCODE_Q_ENABLE: bad qid {}",
                   hal_lif_info_.hw_lif_id, cmd->qid);
            return (DEVCMD_ERROR);
        }
        addr = pd->lm_->GetLIFQStateAddr(hal_lif_info_.hw_lif_id, cmd->qtype, cmd->qid);
        if (addr < 0) {
            NIC_LOG_ERR("lif-{}: Failed to get qstate address for ADMIN qid {}",
                hal_lif_info_.hw_lif_id, cmd->qid);
            return (DEVCMD_ERROR);
        }
        admin_cfg.enable = 0x1;
        admin_cfg.host_queue = spec->host_dev;
        admin_cfg.intr_enable = 0x1;
        WRITE_MEM(addr + offsetof(admin_qstate_t, cfg), (uint8_t *)&admin_cfg, sizeof(admin_cfg), 0);
        invalidate_txdma_cacheline(addr);
        break;
    default:
        return (DEVCMD_ERROR);
        break;
    }

    return (DEVCMD_SUCCESS);
}

enum DevcmdStatus
Eth::_CmdQDisable(void *req, void *req_data, void *resp, void *resp_data)
{
    int64_t addr;
    struct q_disable_cmd *cmd = (struct q_disable_cmd *)req;
    // q_disable_comp *comp = (q_disable_comp *)resp;
    struct eth_rx_cfg_qstate rx_cfg = {0};
    struct eth_tx_cfg_qstate tx_cfg = {0};
    struct admin_cfg_qstate admin_cfg = {0};

    if (cmd->qtype >= 8) {
        NIC_LOG_ERR("lif-{}: CMD_OPCODE_Q_DISABLE: bad qtype {}",
        hal_lif_info_.hw_lif_id, cmd->qtype);
        return (DEVCMD_ERROR);
    }

    NIC_LOG_DEBUG("lif-{}: CMD_OPCODE_Q_DISABLE: type {} qid {}",
        hal_lif_info_.hw_lif_id, cmd->qtype, cmd->qid);

    switch (cmd->qtype) {
    case ETH_QTYPE_RX:
        if (cmd->qid >= spec->rxq_count) {
            NIC_LOG_ERR("lif-{}: CMD_OPCODE_Q_ENABLE: bad qid {}",
            hal_lif_info_.hw_lif_id, cmd->qid);
            return (DEVCMD_ERROR);
        }
        addr = pd->lm_->GetLIFQStateAddr(hal_lif_info_.hw_lif_id, cmd->qtype, cmd->qid);
        if (addr < 0) {
            NIC_LOG_ERR("lif-{}: Failed to get qstate address for RX qid {}",
                hal_lif_info_.hw_lif_id, cmd->qid);
            return (DEVCMD_ERROR);
        }
        rx_cfg.enable = 0x0;
        rx_cfg.host_queue = spec->host_dev;
        rx_cfg.intr_enable = 0x0;
        WRITE_MEM(addr + offsetof(eth_rx_qstate_t, cfg), (uint8_t *)&rx_cfg, sizeof(rx_cfg), 0);
        invalidate_rxdma_cacheline(addr);
        break;
    case ETH_QTYPE_TX:
        if (cmd->qid >= spec->txq_count) {
            NIC_LOG_ERR("lif-{}: CMD_OPCODE_Q_ENABLE: bad qid {}",
                   hal_lif_info_.hw_lif_id, cmd->qid);
            return (DEVCMD_ERROR);
        }
        addr = pd->lm_->GetLIFQStateAddr(hal_lif_info_.hw_lif_id, cmd->qtype, cmd->qid);
        if (addr < 0) {
            NIC_LOG_ERR("lif-{}: Failed to get qstate address for TX qid {}",
                hal_lif_info_.hw_lif_id, cmd->qid);
            return (DEVCMD_ERROR);
        }
        tx_cfg.enable = 0x0;
        tx_cfg.host_queue = spec->host_dev;
        tx_cfg.intr_enable = 0x0;
        WRITE_MEM(addr + offsetof(eth_tx_qstate_t, cfg), (uint8_t *)&tx_cfg, sizeof(tx_cfg), 0);
        invalidate_txdma_cacheline(addr);
        break;
    case ETH_QTYPE_ADMIN:
        if (cmd->qid >= spec->adminq_count) {
            NIC_LOG_ERR("lif-{}: CMD_OPCODE_Q_ENABLE: bad qid {}",
                   hal_lif_info_.hw_lif_id, cmd->qid);
            return (DEVCMD_ERROR);
        }
        addr = pd->lm_->GetLIFQStateAddr(hal_lif_info_.hw_lif_id, cmd->qtype, cmd->qid);
        if (addr < 0) {
            NIC_LOG_ERR("lif-{}: Failed to get qstate address for ADMIN qid {}",
                hal_lif_info_.hw_lif_id, cmd->qid);
            return (DEVCMD_ERROR);
        }
        admin_cfg.enable = 0x0;
        admin_cfg.host_queue = spec->host_dev;
        admin_cfg.intr_enable = 0x0;
        WRITE_MEM(addr + offsetof(admin_qstate_t, cfg), (uint8_t *)&admin_cfg, sizeof(admin_cfg), 0);
        invalidate_txdma_cacheline(addr);
        break;
    default:
        return (DEVCMD_ERROR);
        break;
    }

    return (DEVCMD_SUCCESS);
}

enum DevcmdStatus
Eth::_CmdSetMode(void *req, void *req_data, void *resp, void *resp_data)
{
    struct rx_mode_set_cmd *cmd = (struct rx_mode_set_cmd *)req;
    // rx_mode_set_comp *comp = (rx_mode_set_comp *)resp;
    EthLif *eth_lif = NULL;
    hal_irisc_ret_t ret = HAL_IRISC_RET_SUCCESS;

    NIC_LOG_DEBUG("lif-{}: CMD_OPCODE_RX_MODE_SET: rx_mode {} {}{}{}{}{}",
            hal_lif_info_.hw_lif_id,
            cmd->rx_mode,
            cmd->rx_mode & RX_MODE_F_UNICAST   ? 'u' : '-',
            cmd->rx_mode & RX_MODE_F_MULTICAST ? 'm' : '-',
            cmd->rx_mode & RX_MODE_F_BROADCAST ? 'b' : '-',
            cmd->rx_mode & RX_MODE_F_PROMISC   ? 'p' : '-',
            cmd->rx_mode & RX_MODE_F_ALLMULTI  ? 'a' : '-');

    auto it = lif_map.find(hal_lif_info_.hw_lif_id);
    if (it == lif_map.cend()) {
        NIC_FUNC_ERR("{}: Unable to find lif {}", spec->name, hal_lif_info_.hw_lif_id);
        return (DEVCMD_ERROR);
    }
    eth_lif = it->second;

    ret = eth_lif->UpdateReceiveBroadcast(cmd->rx_mode & RX_MODE_F_BROADCAST);
    if (ret != HAL_IRISC_RET_SUCCESS) {
        NIC_LOG_ERR("lif-{}: Failed to update BROADCAST mode",
            hal_lif_info_.hw_lif_id);
        return (DEVCMD_ERROR);
    }

    ret = eth_lif->UpdateReceiveAllMulticast(cmd->rx_mode & RX_MODE_F_ALLMULTI);
    if (ret != HAL_IRISC_RET_SUCCESS) {
        NIC_LOG_ERR("lif-{}: Failed to update ALL_MULTICAST mode",
            hal_lif_info_.hw_lif_id);
        return (DEVCMD_ERROR);
    }

    ret = eth_lif->UpdateReceivePromiscuous(cmd->rx_mode & RX_MODE_F_PROMISC);
    if (ret != HAL_IRISC_RET_SUCCESS) {
        NIC_LOG_ERR("lif-{}: Failed to update PROMISCUOUS mode",
            hal_lif_info_.hw_lif_id);
        return (DEVCMD_ERROR);
    }

    return (DEVCMD_SUCCESS);
}

enum DevcmdStatus
Eth::_CmdRxFilterAdd(void *req, void *req_data, void *resp, void *resp_data)
{
    //int status;
    uint64_t mac_addr;
    uint16_t vlan;
    uint32_t filter_id = 0;
    struct rx_filter_add_cmd *cmd = (struct rx_filter_add_cmd *)req;
    struct rx_filter_add_comp *comp = (struct rx_filter_add_comp *)resp;
    EthLif *eth_lif = NULL;
    hal_irisc_ret_t ret = HAL_IRISC_RET_SUCCESS;

    auto it = lif_map.find(hal_lif_info_.hw_lif_id);
    if (it == lif_map.cend()) {
        NIC_FUNC_ERR("{}: Unable to find lif {}", spec->name, hal_lif_info_.hw_lif_id);
        return (DEVCMD_ERROR);
    }
    eth_lif = it->second;

    if (cmd->match == RX_FILTER_MATCH_MAC) {

        memcpy((uint8_t *)&mac_addr, (uint8_t *)&cmd->mac.addr, sizeof(cmd->mac.addr));
        mac_addr = be64toh(mac_addr) >> (8 * sizeof(mac_addr) - 8 * sizeof(cmd->mac.addr));

        NIC_LOG_DEBUG("lif-{}: Add RX_FILTER_MATCH_MAC mac {}",
                hal_lif_info_.hw_lif_id, macaddr2str(mac_addr));

        ret = eth_lif->AddMac(mac_addr);

        if (ret != HAL_IRISC_RET_SUCCESS) {
            NIC_LOG_WARN("lif-{}: Duplicate Add. Return devcmd error: {}", hal_lif_info_.hw_lif_id, DEVCMD_ERROR);
            return (DEVCMD_ERROR);
        }

        // Store filter
        if (fltr_allocator->alloc(&filter_id) != sdk::lib::indexer::SUCCESS) {
            NIC_LOG_ERR("Failed to allocate MAC address filter");
            return (DEVCMD_ERROR);
        }
        mac_addrs[filter_id] = mac_addr;
    } else if (cmd->match == RX_FILTER_MATCH_VLAN) {
        vlan = cmd->vlan.vlan;

        NIC_LOG_DEBUG("lif-{}: Add RX_FILTER_MATCH_VLAN vlan {}",
                      hal_lif_info_.hw_lif_id, vlan);
        eth_lif->AddVlan(vlan);

        // Store filter
        if (fltr_allocator->alloc(&filter_id) != sdk::lib::indexer::SUCCESS) {
            NIC_LOG_ERR("Failed to allocate VLAN filter");
            return (DEVCMD_ERROR);
        }
        vlans[filter_id] = vlan;
    } else {
        memcpy((uint8_t *)&mac_addr, (uint8_t *)&cmd->mac_vlan.addr, sizeof(cmd->mac_vlan.addr));
        mac_addr = be64toh(mac_addr) >> (8 * sizeof(mac_addr) - 8 * sizeof(cmd->mac_vlan.addr));
        vlan = cmd->mac_vlan.vlan;

        NIC_LOG_DEBUG("lif-{}: Add RX_FILTER_MATCH_MAC_VLAN mac {} vlan {}",
                      hal_lif_info_.hw_lif_id, macaddr2str(mac_addr), vlan);

        eth_lif->AddMacVlan(mac_addr, vlan);

        // Store filter
        if (fltr_allocator->alloc(&filter_id) != sdk::lib::indexer::SUCCESS) {
            NIC_LOG_ERR("Failed to allocate VLAN filter");
            return (DEVCMD_ERROR);
        }
        mac_vlans[filter_id] = std::make_tuple(mac_addr, vlan);
    }

    comp->filter_id = filter_id;
    NIC_LOG_DEBUG("lif-{}: filter_id {}",
                 hal_lif_info_.hw_lif_id, comp->filter_id);
    return (DEVCMD_SUCCESS);
}

enum DevcmdStatus
Eth::_CmdRxFilterDel(void *req, void *req_data, void *resp, void *resp_data)
{
    //int status;
    uint64_t mac_addr;
    uint16_t vlan;
    struct rx_filter_del_cmd *cmd = (struct rx_filter_del_cmd *)req;
    //struct rx_filter_del_comp *comp = (struct rx_filter_del_comp *)resp;
    EthLif *eth_lif = NULL;
    indexer::status rs;

    auto it = lif_map.find(hal_lif_info_.hw_lif_id);
    if (it == lif_map.cend()) {
        NIC_FUNC_ERR("{}: Unable to find lif {}", spec->name, hal_lif_info_.hw_lif_id);
        return (DEVCMD_ERROR);
    }
    eth_lif = it->second;

    if (mac_addrs.find(cmd->filter_id) != mac_addrs.end()) {
        mac_addr = mac_addrs[cmd->filter_id];
        NIC_LOG_DEBUG("lif-{}: Del RX_FILTER_MATCH_MAC mac:{}",
                      hal_lif_info_.hw_lif_id,
                      macaddr2str(mac_addr));
        eth_lif->DelMac(mac_addr);
        mac_addrs.erase(cmd->filter_id);
    } else if (vlans.find(cmd->filter_id) != vlans.end()) {
        vlan = vlans[cmd->filter_id];
        NIC_LOG_DEBUG("lif-{}: Del RX_FILTER_MATCH_VLAN vlan {}",
                     hal_lif_info_.hw_lif_id, vlan);
        eth_lif->DelVlan(vlan);
        vlans.erase(cmd->filter_id);
    } else if (mac_vlans.find(cmd->filter_id) != mac_vlans.end()) {
        auto mac_vlan = mac_vlans[cmd->filter_id];
        mac_addr = std::get<0>(mac_vlan);
        vlan = std::get<1>(mac_vlan);
        NIC_LOG_DEBUG("lif-{}: Del RX_FILTER_MATCH_MAC_VLAN mac: {}, vlan: {}",
                     hal_lif_info_.hw_lif_id, macaddr2str(mac_addr), vlan);
        eth_lif->DelMacVlan(mac_addr, vlan);
        mac_vlans.erase(cmd->filter_id);
    } else {
        NIC_LOG_ERR("Invalid filter id {}", cmd->filter_id);
        return (DEVCMD_ERROR);
    }

    rs = fltr_allocator->free(cmd->filter_id);
    if (rs != indexer::SUCCESS) {
        HAL_TRACE_ERR("Failed to free filter_id: {}, err: {}",
                      cmd->filter_id, rs);
        return (DEVCMD_ERROR);
    }
    NIC_LOG_DEBUG("Freed filter_id: {}", cmd->filter_id);

    return (DEVCMD_SUCCESS);
}

enum DevcmdStatus
Eth::_CmdMacAddrGet(void *req, void *req_data, void *resp, void *resp_data)
{
    uint64_t mac_addr;

    //struct station_mac_addr_get_cmd *cmd = (struct station_mac_addr_get_cmd *)req;
    struct station_mac_addr_get_comp *comp = (struct station_mac_addr_get_comp *)resp;

    NIC_LOG_DEBUG("lif-{}: CMD_OPCODE_STATION_MAC_ADDR_GET", hal_lif_info_.hw_lif_id);

    mac_addr = be64toh(spec->mac_addr) >> (8 * sizeof(spec->mac_addr) - 8 * sizeof(uint8_t[6]));
    memcpy((uint8_t *)comp->addr, (uint8_t *)&mac_addr, sizeof(comp->addr));

    NIC_LOG_DEBUG("lif-{}: station mac address {}", hal_lif_info_.hw_lif_id,
        macaddr2str(mac_addr));

    return (DEVCMD_SUCCESS);
}

enum DevcmdStatus
Eth::_CmdStatsDumpStart(void *req, void *req_data, void *resp, void *resp_data)
{
    struct stats_dump_cmd *cmd = (struct stats_dump_cmd *)req;

    NIC_LOG_DEBUG("lif-{}: CMD_OPCODE_STATS_DUMP_START: host_stats_mem_addr {:#x}",
        hal_lif_info_.hw_lif_id, cmd->addr);

    if (cmd->addr == 0) {
        return (DEVCMD_SUCCESS);
    }

    host_stats_mem_addr = cmd->addr;

    MEM_SET(stats_mem_addr, 0, LIF_STATS_SIZE, 0);

    evutil_timer_start(&stats_timer, &Eth::StatsUpdate, this, 0.0, 0.2);

    return (DEVCMD_SUCCESS);
}

enum DevcmdStatus
Eth::_CmdStatsDumpStop(void *req, void *req_data, void *resp, void *resp_data)
{
    NIC_LOG_DEBUG("lif-{}: CMD_OPCODE_STATS_DUMP_STOP: host_stats_mem_addr {:#x}",
        hal_lif_info_.hw_lif_id, host_stats_mem_addr);

    if (host_stats_mem_addr == 0) {
        return (DEVCMD_SUCCESS);
    }

    evutil_timer_stop(&stats_timer);

    host_stats_mem_addr = 0;

    return (DEVCMD_SUCCESS);
}

enum DevcmdStatus
Eth::_CmdRssHashSet(void *req, void *req_data, void *resp, void *resp_data)
{
    int ret;
    struct rss_hash_set_cmd *cmd = (struct rss_hash_set_cmd *)req;
    //rss_hash_set_comp *comp = (struct rss_hash_set_comp *)resp;

    rss_type = cmd->types;
    memcpy(rss_key, cmd->key, RSS_HASH_KEY_SIZE);

    NIC_LOG_DEBUG("lif-{}: CMD_OPCODE_RSS_HASH_SET: type {:#x} key {} table {}",
        hal_lif_info_.hw_lif_id, rss_type, rss_key, rss_indir);

    ret = pd->eth_program_rss(hal_lif_info_.hw_lif_id, rss_type, rss_key, rss_indir,
                              spec->rxq_count);
    if (ret != 0) {
        NIC_LOG_DEBUG("_CmdRssHashSet:{}: Unable to program hw for RSS HASH", ret);
        return (DEVCMD_ERROR);
    }

    return DEVCMD_SUCCESS;
}

enum DevcmdStatus
Eth::_CmdRssIndirSet(void *req, void *req_data, void *resp, void *resp_data)
{
    int ret;
    //struct rss_indir_set_cmd *cmd = (struct rss_indir_set_cmd *)req;
    //rss_indir_set_comp *comp = (struct rss_indir_set_comp *)resp;

    memcpy(rss_indir, req_data, RSS_IND_TBL_SIZE);
    NIC_LOG_DEBUG("lif-{}: CMD_OPCODE_RSS_INDIR_SET: type {:#x} key {} table {}",
        hal_lif_info_.hw_lif_id, rss_type, rss_key, rss_indir);

    for (int i = 0; i < RSS_IND_TBL_SIZE; i++) {
        if (((uint8_t *)req_data)[i] > spec->rxq_count) {
            NIC_LOG_ERR("lif-{}: Invalid indirection table entry index %d qid %d",
                i, ((uint8_t *)req_data)[i]);
            return (DEVCMD_ERROR);
        }
    }

    ret = pd->eth_program_rss(hal_lif_info_.hw_lif_id, rss_type, rss_key, rss_indir,
                          spec->rxq_count);
    if (ret != 0) {
        NIC_LOG_ERR("_CmdRssIndirSet:{}: Unable to program hw for RSS INDIR", ret);
        return (DEVCMD_ERROR);
    }

    return (DEVCMD_SUCCESS);
}

/*
 * CMD_OPCODE_RDMA_* ops
 */
enum DevcmdStatus
Eth::_CmdRDMACreateEQ(void *req, void *req_data, void *resp, void *resp_data)
{
    struct rdma_queue_cmd  *cmd = (struct rdma_queue_cmd  *) req;
    eqcb_t      eqcb;
    int64_t     addr;

    NIC_LOG_DEBUG("lif-{}: CMD_OPCODE_RDMA_CREATE_EQ "
                 "qid {} depth_log2 {} "
                 "stride_log2 {} dma_addr {} "
                 "cid {}", hal_lif_info_.hw_lif_id + cmd->lif_id, cmd->qid_ver,
                 1u << cmd->depth_log2, 1u << cmd->stride_log2,
                 cmd->dma_addr, cmd->cid);

    memset(&eqcb, 0, sizeof(eqcb_t));
    // EQ does not need scheduling, so set one less (meaning #rings as zero)
    eqcb.ring_header.total_rings = MAX_EQ_RINGS - 1;
    eqcb.eqe_base_addr = cmd->dma_addr | (1UL << 63) | ((uint64_t)(hal_lif_info_.hw_lif_id + cmd->lif_id) << 52);
    eqcb.log_wqe_size = cmd->stride_log2;
    eqcb.log_num_wqes = cmd->depth_log2;
    eqcb.int_enabled = 1;
    //eqcb.int_num = spec.int_num();
    eqcb.eq_id = cmd->cid;
    eqcb.color = 0;

    eqcb.int_assert_addr = intr_assert_addr(intr_base + cmd->cid);

    memrev((uint8_t*)&eqcb, sizeof(eqcb_t));

    addr = pd->lm_->GetLIFQStateAddr(hal_lif_info_.hw_lif_id, ETH_QTYPE_EQ, cmd->qid_ver);
    if (addr < 0) {
        NIC_LOG_ERR("lif-{}: Failed to get qstate address for EQ qid {}",
                    hal_lif_info_.hw_lif_id+ cmd->lif_id, cmd->qid_ver);
        return (DEVCMD_ERROR);
    }
    WRITE_MEM(addr, (uint8_t *)&eqcb, sizeof(eqcb), 0);
    invalidate_rxdma_cacheline(addr);
    invalidate_txdma_cacheline(addr);
    return (DEVCMD_SUCCESS);
}

enum DevcmdStatus
Eth::_CmdRDMACreateCQ(void *req, void *req_data, void *resp, void *resp_data)
{
    struct rdma_queue_cmd *cmd = (struct rdma_queue_cmd *) req;
    uint32_t               lif = hal_lif_info_.hw_lif_id + cmd->lif_id;
    uint32_t               num_cq_wqes, cqwqe_size;
    cqcb_t                 cqcb;
    uint8_t                offset;
    int                    ret;
    int64_t                addr;

    NIC_LOG_DEBUG("lif-{}: RDMA_CREATE_CQ: cq_num: {} cq_wqe_size: {} num_cq_wqes: {} "
                  "eq_id: {} hostmem_pg_size: {} ",
                  lif, cmd->qid_ver,
                  1u << cmd->stride_log2, 1u << cmd->depth_log2,
                  cmd->cid, 1ull << (cmd->stride_log2 + cmd->depth_log2));

    cqwqe_size = 1u << cmd->stride_log2;
    num_cq_wqes = 1u << cmd->depth_log2;

    NIC_LOG_DEBUG("cqwqe_size: {} num_cq_wqes: {}", cqwqe_size, num_cq_wqes);

    memset(&cqcb, 0, sizeof(cqcb_t));
    cqcb.ring_header.total_rings = MAX_CQ_RINGS;
    cqcb.ring_header.host_rings = MAX_CQ_HOST_RINGS;

    int32_t cq_pt_index = cmd->xxx_table_index;
    uint64_t  pt_table_base_addr = pd->rdma_get_pt_base_addr(lif);

    cqcb.pt_base_addr = pt_table_base_addr >> PT_BASE_ADDR_SHIFT;
    cqcb.log_cq_page_size = cmd->stride_log2 + cmd->depth_log2;
    cqcb.log_wqe_size = log2(cqwqe_size);
    cqcb.log_num_wqes = log2(num_cq_wqes);
    cqcb.cq_id = cmd->qid_ver;
    cqcb.eq_id = cmd->cid;
    cqcb.host_addr = 1;

    cqcb.pt_pa = cmd->dma_addr;
    if (cqcb.host_addr) {
        cqcb.pt_pa |= ((1UL << 63)  | (uint64_t)lif << 52);
    }

    cqcb.pt_pg_index = 0;
    cqcb.pt_next_pg_index = 0x1FF;

    int log_num_pages = cqcb.log_num_wqes + cqcb.log_wqe_size - cqcb.log_cq_page_size;
    NIC_LOG_DEBUG("lif-{}: pt_pa: {:#x}: pt_next_pa: {:#x}: pt_pa_index: {}: pt_next_pa_index: {}: log_num_pages: {}",
        lif, cqcb.pt_pa, cqcb.pt_next_pa, cqcb.pt_pg_index, cqcb.pt_next_pg_index, log_num_pages);

    /* store  pt_pa & pt_next_pa in little endian. So need an extra memrev */
    memrev((uint8_t*)&cqcb.pt_pa, sizeof(uint64_t));

    ret = pd->lm_->GetPCOffset("p4plus", "rxdma_stage0.bin", "rdma_cq_rx_stage0", &offset);
    if (ret < 0) {
        NIC_LOG_ERR("Failed to get PC offset : {} for prog: {}, label: {}", ret, "rxdma_stage0.bin", "rdma_cq_rx_stage0");
        return (DEVCMD_ERROR);
    }

    cqcb.ring_header.pc = offset;

    // write to hardware
    NIC_LOG_DEBUG("lif-{}: Writting initial CQCB State, "
                  "CQCB->PT: {:#x} cqcb_size: {}",
                  lif, cqcb.pt_base_addr, sizeof(cqcb_t));
    // Convert data before writting to HBM
    memrev((uint8_t*)&cqcb, sizeof(cqcb_t));

    addr = pd->lm_->GetLIFQStateAddr(hal_lif_info_.hw_lif_id, ETH_QTYPE_CQ, cmd->qid_ver);
    if (addr < 0) {
        NIC_LOG_ERR("lif-{}: Failed to get qstate address for CQ qid {}",
                    lif, cmd->qid_ver);
        return DEVCMD_ERROR;
    }
    WRITE_MEM(addr, (uint8_t *)&cqcb, sizeof(cqcb), 0);

    uint64_t pt_table_addr = pt_table_base_addr+cq_pt_index*sizeof(uint64_t);

    // There is only one page table entry for adminq CQ
    WRITE_MEM(pt_table_addr, (uint8_t *)&cmd->dma_addr, sizeof(uint64_t), 0);
    NIC_LOG_DEBUG("PT Entry Write: Lif {}: CQ PT Idx: {} PhyAddr: {:#x}",
                  lif, cq_pt_index, cmd->dma_addr);
    invalidate_rxdma_cacheline(pt_table_addr);
    invalidate_txdma_cacheline(pt_table_addr);

    return (DEVCMD_SUCCESS);
}

enum DevcmdStatus
Eth::_CmdRDMACreateAdminQ(void *req, void *req_data, void *resp, void *resp_data)
{
    struct rdma_queue_cmd  *cmd = (struct rdma_queue_cmd  *) req;
    int                     ret;
    uint32_t                lif = hal_lif_info_.hw_lif_id + cmd->lif_id;
    aqcb_t                  aqcb;
    uint8_t                 offset;
    int64_t                 addr;

    NIC_LOG_DEBUG("lif-{}: RDMA_CREATE_ADMINQ aq_num: {} aq_log_wqe_size: {} "
                    "aq_log_num_wqes: {} "
                    "cq_num: {} phy_base_addr: {}",
                    lif, cmd->qid_ver,
                    cmd->stride_log2, cmd->depth_log2, cmd->cid,
                    cmd->dma_addr);

    memset(&aqcb, 0, sizeof(aqcb_t));
    aqcb.aqcb0.ring_header.total_rings = MAX_AQ_RINGS;
    aqcb.aqcb0.ring_header.host_rings = MAX_AQ_HOST_RINGS;

    aqcb.aqcb0.log_wqe_size = cmd->stride_log2;
    aqcb.aqcb0.log_num_wqes = cmd->depth_log2;
    aqcb.aqcb0.aq_id = cmd->qid_ver;
    aqcb.aqcb0.phy_base_addr = cmd->dma_addr | (1UL << 63) | ((uint64_t)lif << 52);
    aqcb.aqcb0.cq_id = cmd->cid;
    addr = pd->lm_->GetLIFQStateAddr(hal_lif_info_.hw_lif_id, ETH_QTYPE_CQ, cmd->cid);
    if (addr < 0) {
        NIC_LOG_ERR("lif-{}: Failed to get qstate address for CQ qid {}",
                    lif, cmd->cid);
        return DEVCMD_ERROR;
    }
    aqcb.aqcb0.cqcb_addr = addr;

    aqcb.aqcb0.first_pass = 1;

    ret = pd->lm_->GetPCOffset("p4plus", "txdma_stage0.bin", "rdma_aq_tx_stage0", &offset);
    if (ret < 0) {
        NIC_LOG_ERR("Failed to get PC offset : {} for prog: {}, label: {}", ret, "txdma_stage0.bin", "rdma_aq_tx_stage0");
        return DEVCMD_ERROR;
    }

    aqcb.aqcb0.ring_header.pc = offset;

    // write to hardware
    NIC_LOG_DEBUG("lif-{}: Writting initial AQCB State, "
                    "AQCB->phy_addr: {:#x} "
                    "aqcb_size: {}",
                    lif, aqcb.aqcb0.phy_base_addr, sizeof(aqcb_t));

    // Convert data before writting to HBM
    memrev((uint8_t*)&aqcb.aqcb0, sizeof(aqcb0_t));
    memrev((uint8_t*)&aqcb.aqcb1, sizeof(aqcb1_t));

    addr = pd->lm_->GetLIFQStateAddr(hal_lif_info_.hw_lif_id, ETH_QTYPE_ADMIN, cmd->qid_ver);;
    if (addr < 0) {
        NIC_LOG_ERR("lif-{}: Failed to get qstate address for AQ qid {}",
                    lif, cmd->qid_ver);
        return DEVCMD_ERROR;
    }
    WRITE_MEM(addr, (uint8_t *)&aqcb, sizeof(aqcb), 0);

    return (DEVCMD_SUCCESS);
}

int
Eth::GenerateQstateInfoJson(pt::ptree &lifs)
{
    pt::ptree lif;
    pt::ptree qstates;

    NIC_LOG_DEBUG("lif-{}: Qstate Info to Json", hal_lif_info_.hw_lif_id);

    lif.put("hw_lif_id", hal_lif_info_.hw_lif_id);

    for (int j = 0; j < NUM_QUEUE_TYPES; j++) {
        pt::ptree qs;
        char numbuf[32];

        if (qinfo[j].size < 1) continue;

        qs.put("qtype", qinfo[j].type_num);
        qs.put("qsize", qinfo[j].size);
        qs.put("qaddr", hal_lif_info_.qstate_addr[qinfo[j].type_num]);
        snprintf(numbuf, sizeof(numbuf), "0x%lx", hal_lif_info_.qstate_addr[qinfo[j].type_num]);
        qs.put("qaddr_hex", numbuf);
        qstates.push_back(std::make_pair("", qs));
        qs.clear();
    }

    lif.add_child("qstates", qstates);
    lifs.push_back(std::make_pair("", lif));
    qstates.clear();
    lif.clear();
    return 0;
}

void
Eth::FreeUpMacFilters()
{
    uint64_t filter_id;
    indexer::status rs;

    for (auto it = mac_addrs.cbegin(); it != mac_addrs.cend();) {
        filter_id = it->first;
        rs = fltr_allocator->free(filter_id);
        if (rs != indexer::SUCCESS) {
            HAL_TRACE_ERR("Failed to free filter_id: {}, err: {}",
                          filter_id, rs);
            // return (DEVCMD_ERROR);
        }
        NIC_LOG_DEBUG("Freed filter_id: {}", filter_id);
        it = mac_addrs.erase(it);
    }
}

void
Eth::FreeUpVlanFilters()
{
    uint64_t filter_id;
    indexer::status rs;

    for (auto it = vlans.cbegin(); it != vlans.cend();) {
        filter_id = it->first;
        rs = fltr_allocator->free(filter_id);
        if (rs != indexer::SUCCESS) {
            HAL_TRACE_ERR("Failed to free filter_id: {}, err: {}",
                          filter_id, rs);
            // return (DEVCMD_ERROR);
        }
        NIC_LOG_DEBUG("Freed filter_id: {}", filter_id);
        it = vlans.erase(it);
    }
}

void
Eth::FreeUpMacVlanFilters()
{
    uint64_t filter_id;
    indexer::status rs;

    for (auto it = mac_vlans.cbegin(); it != mac_vlans.cend();) {
        filter_id = it->first;
        rs = fltr_allocator->free(filter_id);
        if (rs != indexer::SUCCESS) {
            HAL_TRACE_ERR("Failed to free filter_id: {}, err: {}",
                          filter_id, rs);
            // return (DEVCMD_ERROR);
        }
        NIC_LOG_DEBUG("Freed filter_id: {}", filter_id);
        it = mac_vlans.erase(it);
    }
}

types::LifType
Eth::ConvertDevTypeToLifType(EthDevType dev_type)
{
    switch(dev_type) {
        case ETH_HOST: return types::LIF_TYPE_HOST;
        case ETH_HOST_MGMT: return types::LIF_TYPE_HOST_MANAGEMENT;
        case ETH_MNIC_OOB_MGMT: return types::LIF_TYPE_MNIC_OOB_MANAGEMENT;
        case ETH_MNIC_INTERNAL_MGMT: return types::LIF_TYPE_MNIC_INTERNAL_MANAGEMENT;
        case ETH_MNIC_INBAND_MGMT: return types::LIF_TYPE_MNIC_INBAND_MANAGEMENT;
        default: return types::LIF_TYPE_NONE;
    }
}

void
Eth::SetHalClient(HalClient *hal_client, HalCommonClient *hal_cmn_client)
{
    hal = hal_client;
    hal_common_client = hal_cmn_client;
}
