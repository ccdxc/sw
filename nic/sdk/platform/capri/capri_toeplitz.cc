// {C} Copyright 2018 Pensando Systems Inc. All rights reserved

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

#define CAPRI_P4PLUS_HANDLE                 "apollo_rxdma"

int
capri_toeplitz_init (int stage, int stage_tableid)
{
    int tbl_id;
    uint64_t pc;
    uint64_t tbl_base;
    cap_top_csr_t & cap0 = sdk::platform::capri::g_capri_state_pd->cap_top();
    cap_te_csr_t *te_csr = NULL;

    if (sdk::p4::p4_program_to_base_addr((char *) CAPRI_P4PLUS_HANDLE,
                                   (char *) ETH_RSS_INDIR_PROGRAM,
                                   &pc) != 0) {
        SDK_TRACE_DEBUG("Could not resolve handle %s program %s",
                        (char *) CAPRI_P4PLUS_HANDLE,
                        (char *) ETH_RSS_INDIR_PROGRAM);
        return CAPRI_FAIL;
    }
    SDK_TRACE_DEBUG("Resolved handle %s program %s to PC 0x%x",
                    (char *) CAPRI_P4PLUS_HANDLE,
                    (char *) ETH_RSS_INDIR_PROGRAM,
                    pc);

    // Program rss params table with the PC
    te_csr = &cap0.pcr.te[stage];

    tbl_id = stage_tableid;

#ifdef MEM_REGION_RSS_INDIR_TABLE_NAME
    tbl_base = get_mem_addr(MEM_REGION_RSS_INDIR_TABLE_NAME);
    SDK_ASSERT(tbl_base != INVALID_MEM_ADDRESS);
#else
    SDK_ASSERT(0);
#endif
    // Align the table address because while calculating the read address TE shifts the LIF
    // value by LOG2 of size of the per lif indirection table.
    tbl_base = (tbl_base + ETH_RSS_INDIR_TBL_SZ) & ~(ETH_RSS_INDIR_TBL_SZ - 1);

    SDK_TRACE_DEBUG("rss_indir_table id %u table_base %x\n", tbl_id, tbl_base);

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

    return CAPRI_OK;
}

}    // namespace capri
}    // namespace platform
}    // namespace sdk
