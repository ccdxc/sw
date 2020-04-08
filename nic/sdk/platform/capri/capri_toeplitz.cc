// {C} Copyright 2018 Pensando Systems Inc. All rights reserved

#include "asic/cmn/asic_hbm.hpp"
#include "platform/capri/capri_tbl_rw.hpp"
#include "platform/capri/capri_hbm_rw.hpp"
#include "platform/capri/capri_state.hpp"
#include "platform/utils/mpartition.hpp"
#include "third-party/asic/capri/model/utils/cap_blk_reg_model.h"
#include "third-party/asic/capri/model/cap_top/cap_top_csr.h"

namespace sdk {
namespace platform {
namespace capri {

// RSS Topelitz Table
#define ETH_RSS_INDIR_PROGRAM               "eth_rx_rss_indir.bin"
// Maximum number of queue per LIF
#define ETH_RSS_MAX_QUEUES                  (128)
// Number of entries in a LIF's indirection table
#define ETH_RSS_LIF_INDIR_TBL_LEN           ETH_RSS_MAX_QUEUES
// Size of each LIF indirection table entry
#define ETH_RSS_LIF_INDIR_TBL_ENTRY_SZ      (2)
// Size of a LIF's indirection table
#define ETH_RSS_LIF_INDIR_TBL_SZ            (ETH_RSS_LIF_INDIR_TBL_LEN * ETH_RSS_LIF_INDIR_TBL_ENTRY_SZ)
// Max number of LIFs supported
#define MAX_LIFS                            (2048)
// Size of the entire LIF indirection table
#define ETH_RSS_INDIR_TBL_SZ                (MAX_LIFS * ETH_RSS_LIF_INDIR_TBL_SZ)

void
capri_rss_table_config (uint32_t stage, uint32_t stage_tableid,
                        uint64_t tbl_base, uint64_t pc)
{
    cap_top_csr_t & cap0 = sdk::platform::capri::g_capri_state_pd->cap_top();
    cap_te_csr_t *te_csr;
    int tbl_id = stage_tableid;

    SDK_TRACE_DEBUG("rss_indir_table stage %u stage-tableid %u table_base %x\n",
                    stage, tbl_id, tbl_base);

    te_csr = &cap0.pcr.te[stage];
    te_csr->cfg_table_property[tbl_id].read();
    te_csr->cfg_table_property[tbl_id].mpu_pc(pc >> 6);
    te_csr->cfg_table_property[tbl_id].mpu_pc_dyn(0);
    // HBM Table
    te_csr->cfg_table_property[tbl_id].axi(0); //1==table in SRAM, 0== table in HBM
    // TE addr = hash
    // TE mask = (1 << addr_sz) - 1
    te_csr->cfg_table_property[tbl_id].addr_sz((uint8_t)log2(ETH_RSS_LIF_INDIR_TBL_LEN));
    // TE addr <<= addr_shift
    te_csr->cfg_table_property[tbl_id].addr_shift((uint8_t)log2(ETH_RSS_LIF_INDIR_TBL_ENTRY_SZ));
    // TE addr = (hash & mask) + addr_base
    te_csr->cfg_table_property[tbl_id].addr_base(tbl_base);
    // TE lif_shift_en
    te_csr->cfg_table_property[tbl_id].addr_vf_id_en(1);
    // TE lif_shift
    te_csr->cfg_table_property[tbl_id].addr_vf_id_loc((uint8_t)log2(ETH_RSS_LIF_INDIR_TBL_SZ));
    // addr |= (lif << lif_shift)
    // TE addr = addr & ((1 << chain_shift) - 1) if 0 <= cycle_id < 63 else addr
    te_csr->cfg_table_property[tbl_id].chain_shift(0x3f);
    // size of each indirection table entry
    te_csr->cfg_table_property[tbl_id].lg2_entry_size((uint8_t)log2(ETH_RSS_LIF_INDIR_TBL_ENTRY_SZ));
    te_csr->cfg_table_property[tbl_id].write();
}

static sdk_ret_t
capri_rss_table_config_ex (uint32_t stage, uint32_t stage_tableid,
                           uint32_t rss_indir_tbl_entry_size)
{
    int tbl_id;
    uint64_t pc, tbl_base, rss_indir_tbl_size, rss_indir_tbl_total_size;
    cap_top_csr_t & cap0 = sdk::platform::capri::g_capri_state_pd->cap_top();
    cap_te_csr_t *te_csr = NULL;

    rss_indir_tbl_size = (ETH_RSS_LIF_INDIR_TBL_LEN * rss_indir_tbl_entry_size);
    rss_indir_tbl_total_size = (MAX_LIFS * rss_indir_tbl_size);

#define CAPRI_P4PLUS_HANDLE         "p4plus"
    if (sdk::p4::p4_program_to_base_addr((char *) CAPRI_P4PLUS_HANDLE,
                                   (char *) ETH_RSS_INDIR_PROGRAM,
                                   &pc) != 0) {
        SDK_TRACE_DEBUG("Could not resolve handle %s program %s",
                        (char *) CAPRI_P4PLUS_HANDLE,
                        (char *) ETH_RSS_INDIR_PROGRAM);
        return SDK_RET_ERR;
    }
    SDK_TRACE_DEBUG("Resolved handle %s program %s to PC 0x%x",
                    (char *) CAPRI_P4PLUS_HANDLE,
                    (char *) ETH_RSS_INDIR_PROGRAM,
                    pc);

    tbl_id = stage_tableid;

#ifdef MEM_REGION_RSS_INDIR_TABLE_NAME
    tbl_base = asicpd_get_mem_addr(MEM_REGION_RSS_INDIR_TABLE_NAME);
    SDK_ASSERT(tbl_base != INVALID_MEM_ADDRESS);
#else
    SDK_ASSERT(0);
#endif
    // Align the table address because while calculating the read address TE shifts the LIF
    // value by LOG2 of size of the per lif indirection table.
    tbl_base = (tbl_base + rss_indir_tbl_total_size) &
        ~(rss_indir_tbl_total_size - 1);

    SDK_TRACE_DEBUG("rss_indir_table stage %u stage-tableid %u table_base 0x%x rss_indir_tbl_entry_size %u rss_indir_tbl_size %u rss_indir_tbl_total_size %u\n",
                    stage, tbl_id, tbl_base, rss_indir_tbl_entry_size,
                    rss_indir_tbl_size, rss_indir_tbl_total_size);

    // Program rss params table with the PC
    te_csr = &cap0.pcr.te[stage];
    te_csr->cfg_table_property[tbl_id].read();
    te_csr->cfg_table_property[tbl_id].mpu_pc(pc >> 6);
    te_csr->cfg_table_property[tbl_id].mpu_pc_dyn(0);
    // HBM Table
    te_csr->cfg_table_property[tbl_id].axi(0); //1==table in SRAM, 0== table in HBM
    // TE addr = hash
    // TE mask = (1 << addr_sz) - 1
    te_csr->cfg_table_property[tbl_id].addr_sz((uint8_t)log2(ETH_RSS_LIF_INDIR_TBL_LEN));
    // TE addr <<= addr_shift
    te_csr->cfg_table_property[tbl_id].addr_shift((uint8_t)log2(rss_indir_tbl_entry_size));
    // TE addr = (hash & mask) + addr_base
    te_csr->cfg_table_property[tbl_id].addr_base(tbl_base);
    // TE lif_shift_en
    te_csr->cfg_table_property[tbl_id].addr_vf_id_en(1);
    // TE lif_shift
    te_csr->cfg_table_property[tbl_id].addr_vf_id_loc((uint8_t)log2(rss_indir_tbl_size));
    // addr |= (lif << lif_shift)
    // TE addr = addr & ((1 << chain_shift) - 1) if 0 <= cycle_id < 63 else addr
    te_csr->cfg_table_property[tbl_id].chain_shift(0x3f);
    // size of each indirection table entry
    te_csr->cfg_table_property[tbl_id].lg2_entry_size((uint8_t)log2(rss_indir_tbl_entry_size));
    te_csr->cfg_table_property[tbl_id].write();

    return SDK_RET_OK;
}

sdk_ret_t
capri_rss_table_base_pc_get (const char *handle, uint64_t *tbl_base,
                             uint64_t *pc)
{
    if (sdk::p4::p4_program_to_base_addr(handle,
                                         (char *)ETH_RSS_INDIR_PROGRAM,
                                         pc) != 0) {
        SDK_TRACE_DEBUG("Could not resolve handle %s program %s",
                        handle, (char *) ETH_RSS_INDIR_PROGRAM);
        return SDK_RET_ERR;
    }
    SDK_TRACE_DEBUG("Resolved handle %s program %s to PC 0x%x",
                    handle, (char *)ETH_RSS_INDIR_PROGRAM, *pc);


#ifdef MEM_REGION_RSS_INDIR_TABLE_NAME
    *tbl_base = sdk::asic::asic_get_mem_addr(MEM_REGION_RSS_INDIR_TABLE_NAME);
    SDK_ASSERT(*tbl_base != INVALID_MEM_ADDRESS);
#else
    SDK_ASSERT(0);
#endif

    // Align the table address because while calculating the read address TE shifts the LIF
    // value by LOG2 of size of the per lif indirection table.
    *tbl_base = (*tbl_base + ETH_RSS_INDIR_TBL_SZ) & ~(ETH_RSS_INDIR_TBL_SZ - 1);

    return SDK_RET_OK;
}

sdk_ret_t
capri_toeplitz_init (const char *handle, int stage, int stage_tableid,
                     uint32_t rss_indir_tbl_entry_size)
{
    uint64_t pc, tbl_base;
    sdk_ret_t rv;

    if (rss_indir_tbl_entry_size == 0) {
        rv = capri_rss_table_base_pc_get(handle, &tbl_base, &pc);
        if (rv == SDK_RET_OK)
            capri_rss_table_config(stage, stage_tableid, tbl_base, pc);
    } else
        rv = capri_rss_table_config_ex(stage, stage_tableid,
                                       rss_indir_tbl_entry_size);
    return rv;
}

}    // namespace capri
}    // namespace platform
}    // namespace sdk
