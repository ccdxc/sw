/*
* Copyright (c) 2018, Pensando Systems Inc.
*/

#ifndef __DEV_HPP__
#define __DEV_HPP__

#include <string>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>

#include "nic/sdk/include/sdk/pal.hpp"
#include "nic/include/globals.hpp"
#include "nic/include/adminq.h"
#include "platform/src/lib/hal_api/include/hal_common_client.hpp"
#include "platform/src/lib/hal_api/include/hal_grpc_client.hpp"

#include "hal_client.hpp"

#define HAL_HW_LIF_ID_MAX (HAL_LIF_ID_NICMGR_MIN - 1)

#ifdef __x86_64__

#define READ_MEM        sdk::lib::pal_mem_read
#define WRITE_MEM       sdk::lib::pal_mem_write
#define MEM_SET(pa, val, sz, flags) { \
    uint8_t v = val; \
    for (size_t i = 0; i < sz; i += sizeof(v)) { \
        sdk::lib::pal_mem_write(pa + i, &v, sizeof(v)); \
    } \
}

#define READ_REG        sdk::lib::pal_reg_read
#define WRITE_REG       sdk::lib::pal_reg_write
static inline uint32_t READ_REG32(uint64_t addr)
{
    uint32_t val;
    sdk::lib::pal_reg_write(addr, &val);
    return val;
}
#define WRITE_REG32(addr, val) { \
    uint32_t v = val; \
    sdk::lib::pal_reg_write(addr, &v); \
}
static inline uint64_t READ_REG64(uint64_t addr)
{
    uint64_t val;
    sdk::lib::pal_reg_read(addr, (uint32_t *)&val, 2);
    return val;
}
#define WRITE_REG64(addr, val) { \
    uint64_t v = val; \
    sdk::lib::pal_reg_write(addr, (uint32_t *)&v, 2); \
}

#define WRITE_DB64      sdk::lib::pal_ring_doorbell

#else
#include "platform/src/lib/pal/include/pal.h"
#define READ_MEM        pal_mem_rd
#define WRITE_MEM       pal_mem_wr
#define MEM_SET         pal_memset

#define READ_REG        pal_reg_rd32w
#define WRITE_REG       pal_reg_wr32w
#define READ_REG32      pal_reg_rd32
#define WRITE_REG32     pal_reg_wr32
#define READ_REG64      pal_reg_rd64
#define WRITE_REG64     pal_reg_wr64

#define WRITE_DB64      pal_reg_wr64
#endif

#ifdef __aarch64__
#include "platform/src/lib/pciemgr/include/pciemgr.h"
#endif
#include "platform/src/lib/pciemgrutils/include/pciemgrutils.h"
#include "platform/src/lib/pciehdevices/include/pciehdevices.h"

/**
 * Memory Regions
 */
#define DEVCMD_BASE             roundup(0x0138096c00, 4096)
#define NICMGR_BASE             roundup(0x013a096c00, 4096)

/**
 * ADMINQ
 */
#define NICMGR_QTYPE_REQ        0
#define NICMGR_QTYPE_RESP       1

enum {
    NICMGR_THREAD_ID_MIN           = 0,
    NICMGR_THREAD_ID_DELPHI_CLIENT = 1,
    NICMGR_THREAD_ID_MAX           = 2,
};

enum {
    NICMGR_TIMER_ID_NONE                     = 0,
    NICMGR_TIMER_ID_MIN                      = 1,
    NICMGR_TIMER_ID_HEARTBEAT                = NICMGR_TIMER_ID_MIN,
    NICMGR_TIMER_ID_MAX                      = 2,
};

#pragma pack(push, 1)
struct nicmgr_req_desc {
    uint16_t lif;
    uint8_t qtype;
    uint32_t qid;
    uint16_t comp_index;
    uint64_t adminq_qstate_addr;
    uint8_t pad[47];
    uint8_t cmd[64];
};

struct nicmgr_resp_desc {
    uint16_t lif;
    uint8_t qtype;
    uint32_t qid;
    uint16_t comp_index;
    uint64_t adminq_qstate_addr;
    uint8_t pad[47];
    uint8_t comp[64];
};
#pragma pack(pop)

static_assert(sizeof(struct nicmgr_req_desc) == 128, "");
static_assert(sizeof(struct nicmgr_resp_desc) == 128, "");

/**
 * Device Types
 */
enum DeviceType
{
    INVALID,
    MNIC,
    DEBUG,
    ETH,
    ACCEL,
    NVME,
    VIRTIO,
};

/**
 * Device Spec structures
 */

/**
 * ETH Device type
 */
typedef enum EthDevType_s {
    ETH_UNKNOWN,
    ETH_HOST,
    ETH_HOST_MGMT,
    ETH_MNIC_OOB_MGMT,
    ETH_MNIC_INTERNAL_MGMT,
    ETH_MNIC_INBAND_MGMT,
} EthDevType;

const char *eth_dev_type_to_str(EthDevType type);

/**
 * Eth Device Spec
 */
struct eth_devspec {
    // Delphi Object Key
    uint64_t dev_uuid;
    // FWD
    EthDevType  eth_type;
    std::string if_name;
    uint32_t    uplink_id;
    Uplink      *uplink;
    uint64_t    lif_id;
    uint32_t    hw_lif_id;
    // RES
    uint32_t rxq_count;
    uint32_t txq_count;
    uint32_t eq_count;
    uint32_t adminq_count;
    uint32_t intr_base;
    uint32_t intr_count;
    uint64_t mac_addr;
    // DEV
    bool     host_dev;
    uint8_t  pcie_port;
    // RDMA
    bool     enable_rdma;
    uint32_t pte_count;
    uint32_t key_count;
    uint32_t ah_count;
    uint32_t rdma_sq_count;
    uint32_t rdma_rq_count;
    uint32_t rdma_cq_count;
    uint32_t rdma_eq_count;
    uint32_t rdma_adminq_count;
    uint32_t rdma_pid_count;
    //in 8MB units
    uint32_t barmap_size;
};

typedef struct dev_cmd_db {
    uint32_t    v;
} dev_cmd_db_t;

/**
 * Devcmd Status codes
 */
enum DevcmdStatus
{
    DEVCMD_SUCCESS,
    DEVCMD_BUSY,
    DEVCMD_ERROR,
    DEVCMD_UNKNOWN,
};

/**
 * Utils
 */
void invalidate_rxdma_cacheline(uint64_t addr);
void invalidate_txdma_cacheline(uint64_t addr);

class PdClient;

/**
 * Base Class for devices
 */
class Device {
public:
    virtual void DevcmdPoll() = 0;
    virtual void DevcmdHandler() = 0;
    virtual enum DevcmdStatus CmdHandler(
        void *req, void *req_data,
        void *resp, void *resp_data) = 0;
    enum DeviceType GetType() { return type; }
    void SetType(enum DeviceType type) { this->type = type;}
private:
    enum DeviceType type;

};

/**
 * Device Manager
 */
class DeviceManager {
public:
    DeviceManager(std::string config_file, enum ForwardingMode fwd_mode, platform_t platform,
                  bool dol_integ);
    ~DeviceManager();
    int LoadConfig(std::string path);
    Device *AddDevice(enum DeviceType type, void *dev_spec);
    static DeviceManager *GetInstance() { return instance; }

#ifdef __aarch64__
    static void PcieEventHandler(const pciehdev_eventdata_t *evd);
#endif
    void Update();
    void CreateMnets();
    void DevcmdPoll();
    void AdminQPoll();
    void DevLinkDownHandler(uint32_t port_num);
    void DevLinkUpHandler(uint32_t port_num);
    Device *GetDevice(uint64_t id);
    void CreateUplinkVRFs();
    void SetHalClient(HalClient *hal_client, HalCommonClient *hal_cmn_client);

private:
    static DeviceManager *instance;
    int lifs_reservation(platform_t platform);

    boost::property_tree::ptree spec;
    std::map<uint64_t, Device*> devices; // lif -> device
    std::map<uint64_t, Uplink*> uplinks; // uplink_id -> Uplink

    // Service Lif Info
    hal_lif_info_t hal_lif_info_;
    static struct queue_info qinfo[NUM_QUEUE_TYPES];
    int GenerateQstateInfoJson(std::string qstate_info_file);

    // HAL Info
    HalClient *hal;
    HalCommonClient *hal_common_client;
    uint32_t lif_id;
    // Bharat TODO: Not needed anymore as its being used for non-eth lif
    uint64_t lif_handle;
    bool dol_integ;
    bool init_done;
    std::string config_file;

    PdClient *pd;

    // AdminQ
    uint64_t req_ring_base;
    uint64_t resp_ring_base;
    uint16_t ring_size;
    uint16_t req_head;
    uint16_t req_tail;
    uint16_t resp_head;
    uint16_t resp_tail;
    ForwardingMode fwd_mode;
};
void devicemanager_init(void);


#endif /* __DEV_HPP__ */
