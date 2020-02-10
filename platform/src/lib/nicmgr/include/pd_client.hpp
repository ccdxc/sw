/*
* Copyright (c) 2018, Pensando Systems Inc.
*/

#ifndef __PD_CLIENT_HPP__
#define __PD_CLIENT_HPP__

#include "platform/utils/program.hpp"
#include "platform/utils/mpartition.hpp"
#include "platform/utils/lif_mgr/lif_mgr.hpp"
#include "lib/table/directmap/directmap.hpp"
#include "lib/table/common/table.hpp"
#include "lib/bm_allocator/bm_allocator.hpp"
#include "nic/sdk/lib/p4/p4_api.hpp"
#include "platform/capri/capri_common.hpp"
#include "platform/capri/capri_tbl_rw.hpp"
#include "nic/sdk/platform/devapi/devapi_types.hpp"
#include "nic/sdk/platform/rdmamgr/rdmamgr.hpp"
#include "nic/sdk/asic/rw/asicrw.hpp"
#include "platform/utils/mpartition.hpp"
#include "nic/sdk/platform/capri/capri_state.hpp"

// TODO: what was done here was clearly a hack, please clean up !!!
#if defined(APOLLO) || defined(ARTEMIS) || defined(APULU) || defined(ATHENA)
#include "gen/p4gen/p4plus_rxdma/include/p4plus_rxdma_p4pd.h"
#include "gen/p4gen/p4plus_rxdma/include/p4plus_rxdma_p4pd_table.h"
#include "gen/p4gen/p4plus_txdma/include/p4plus_txdma_p4pd.h"
#include "gen/p4gen/p4plus_txdma/include/p4plus_txdma_p4pd_table.h"
#else
#include "gen/p4gen/common_rxdma_actions/include/common_rxdma_actions_p4pd.h"
#include "gen/p4gen/common_rxdma_actions/include/common_rxdma_actions_p4pd_table.h"
#include "gen/p4gen/common_txdma_actions/include/common_txdma_actions_p4pd.h"
#include "gen/p4gen/common_txdma_actions/include/common_txdma_actions_p4pd_table.h"
#endif

// Maximum number of queue per LIF
#define ETH_RSS_MAX_QUEUES                  (128)
// Number of entries in a LIF's indirection table
#define ETH_RSS_LIF_INDIR_TBL_LEN           ETH_RSS_MAX_QUEUES
// Size of each LIF indirection table entry
#define ETH_RSS_LIF_INDIR_TBL_ENTRY_SZ      (sizeof(eth_rx_rss_indir_eth_rx_rss_indir_t))
// Size of a LIF's indirection table
#define ETH_RSS_LIF_INDIR_TBL_SZ            (ETH_RSS_LIF_INDIR_TBL_LEN * ETH_RSS_LIF_INDIR_TBL_ENTRY_SZ)
// Max number of LIFs supported
#define MAX_LIFS                            (2048)
// Size of the entire LIF indirection table
#define ETH_RSS_INDIR_TBL_SZ                (MAX_LIFS * ETH_RSS_LIF_INDIR_TBL_SZ)
// Memory bar should be multiple of 8 MB
#define MEM_BARMAP_SIZE_SHIFT               (23)

typedef enum lif_rss_type_s {
  LIF_RSS_TYPE_NONE = 0,
  LIF_RSS_TYPE_IPV4 = 1,
  LIF_RSS_TYPE_IPV4_TCP = 2,
  LIF_RSS_TYPE_IPV4_UDP = 4,
  LIF_RSS_TYPE_IPV6 = 8,
  LIF_RSS_TYPE_IPV6_TCP = 16,
  LIF_RSS_TYPE_IPV6_UDP = 32,
  LIF_RSS_TYPE_IPV6_EX = 64,
  LIF_RSS_TYPE_IPV6_TCP_EX = 128,
  LIF_RSS_TYPE_IPV6_UDP_EX = 256,
} lif_rss_type_t;

static inline bool
platform_is_hw(sdk::platform::platform_type_t platform)
{
    return (platform == platform_type_t::PLATFORM_TYPE_HW) ||
           (platform == platform_type_t::PLATFORM_TYPE_HAPS);
}

uint8_t *memrev(uint8_t *block, size_t elnum);

/**
 * Queue info structure for LifCreate
 */
struct queue_info {
  uint32_t type_num;           /* HW Queue Type */
  uint32_t size;               /* Qstate Size: 2^size */
  uint32_t entries;            /* Number of Queues: 2^entries */
  lif_qpurpose_t  purpose;     /* Queue Purpose */
  const char* prog;            /* Program File Name */
  const char* label;           /* Program Entry Label */
  const char* qstate;          /* Qstate structure */
};

const uint32_t kNumMaxLIFs = 2048;

class PdClient {
public:
    static PdClient* factory(sdk::platform::platform_type_t platform,
                             std::string mpart_file,
                             std::string cfg_path);
    void update(void);
    void destroy(PdClient *pdc);

    std::string hal_cfg_path_;
    std::string gen_dir_path_;
    std::string mpart_cfg_path_;
    sdk::platform::platform_type_t platform_;
    sdk::platform::utils::program_info *pinfo_;
    sdk::platform::utils::mpartition *mp_;
    sdk::platform::utils::lif_mgr *lm_;
    directmap    **p4plus_rxdma_dm_tables_;
    directmap    **p4plus_txdma_dm_tables_;

    int lif_qstate_map_init(uint64_t hw_lif_id,
                            struct queue_info* queue_info,
                            uint8_t coses);

    int lif_qstate_init(uint64_t hw_lif_id, struct queue_info* queue_info);

    int program_qstate(struct queue_info* queue_info,
                       lif_info_t *lif_info,
                       uint8_t coses);

    uint8_t get_iq(uint8_t pcp_or_dscp, uint8_t pinned_uplink_port_num);

    void set_program_info();
    int32_t get_pc_offset(const char *prog_name, const char *label,
                          uint8_t *offset, uint64_t *base);
    int create_dirs();
    int p4plus_rxdma_init_tables();
    int p4plus_txdma_init_tables();
    int pd_state_init();

    int p4pd_common_p4plus_rxdma_rss_params_table_entry_add(
            uint32_t hw_lif_id, uint8_t rss_type, uint8_t *rss_key);
    int p4pd_common_p4plus_rxdma_rdma_params_table_entry_get(
            uint32_t hw_lif_id, eth_rx_rss_params_actiondata_t *data);
    int p4pd_common_p4plus_rxdma_rss_indir_table_entry_add(
            uint32_t hw_lif_id, uint8_t index, uint8_t qid);
    int p4pd_common_p4plus_rxdma_rss_indir_table_entry_get(
            uint32_t hw_lif_id, uint8_t index, eth_rx_rss_indir_actiondata_t *data);
    int p4pd_common_p4plus_rxdma_rss_params_table_entry_get(
            uint32_t hw_lif_id, eth_rx_rss_params_actiondata_t *data);

    int eth_program_rss(uint32_t hw_lif_id, uint16_t rss_type,
                        uint8_t *rss_key, uint8_t *rss_indir,
                        uint16_t num_queues);

    /* RDMA routines */
    sdk_ret_t rdma_lif_init(uint32_t lif, uint32_t max_keys,
                            uint32_t max_ahs, uint32_t max_ptes,
                            uint64_t mem_bar_addr, uint32_t mem_bar_size,
                            uint32_t max_prefetch_wqes);

    void cmb_mem_init(void);
    uint64_t cmb_mem_alloc(uint64_t size);
    int cmb_mem_reserve (uint64_t addr, uint64_t size);

    void nicmgr_mem_init(void);
    uint64_t nicmgr_mem_alloc(uint64_t size);
    int nicmgr_mem_reserve(uint64_t addr, uint64_t size);

    void devcmd_mem_init(void);
    uint64_t devcmd_mem_alloc(uint64_t size);
    int devcmd_mem_reserve(uint64_t addr, uint64_t size);

    int32_t intr_alloc(uint32_t count);
    int intr_reserve(uint32_t intr_base, uint32_t count);

    uint64_t rdma_get_pt_base_addr(uint32_t lif);
    uint64_t rdma_get_kt_base_addr(uint32_t lif);
#if 0
    uint64_t rdma_get_ah_base_addr(uint32_t lif);
#endif
    bool is_dev_hwinit_done(const char *dev_name);
    bool is_lif_hwinit_done(uint32_t lif_id);
    bool is_queue_hwinit_done(uint32_t lif_id, uint32_t qtype, uint32_t qid);
private:
    PdClient(){}
    ~PdClient(){}
    void init();

    rdmamgr *rdma_mgr_;

    uint64_t nicmgr_hbm_base_;
    std::unique_ptr<sdk::lib::BMAllocator> nicmgr_hbm_allocator_;
    std::map<uint64_t, uint64_t> nicmgr_allocation_sizes_;

    uint64_t devcmd_hbm_base_;
    std::unique_ptr<sdk::lib::BMAllocator> devcmd_hbm_allocator_;
    std::map<uint64_t, uint64_t> devcmd_allocation_sizes_;

    uint64_t cmb_hbm_base_;
    std::unique_ptr<sdk::lib::BMAllocator> cmb_hbm_allocator_;
    std::map<uint64_t, uint64_t> cmb_allocation_sizes_;

    sdk::lib::indexer *intr_allocator;
};

#endif //__PD_CLIENT_HPP__
