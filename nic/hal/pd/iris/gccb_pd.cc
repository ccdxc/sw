#include "nic/include/base.h"
#include <arpa/inet.h>
#include "nic/include/hal_lock.hpp"
#include "nic/include/pd_api.hpp"
#include "nic/hal/pd/capri/capri_loader.h"
#include "nic/hal/pd/capri/capri_hbm.hpp"
#include "nic/hal/pd/libs/wring/wring_pd.hpp"
#include "nic/hal/src/proxy.hpp"
#include "nic/hal/hal.hpp"
#include "nic/hal/src/lif_manager.hpp"
#include "nic/gen/gc_txdma/include/gc_txdma_p4plus_ingress.h"
#include "nic/hal/pd/iris/p4plus_pd_api.h"
#include "nic/hal/pd/iris/gccb_pd.hpp"

namespace hal {
namespace pd {

/********************************************
 * Garbage collector TxDMA
 * ******************************************/
hal_ret_t
p4pd_get_gc_tx_stage0_prog_offset(int qid, uint64_t* offset)
{
    char progname[] = "txdma_stage0.bin";
    const char *labelname[CAPRI_HBM_GC_NUM_QID] = {
        "gc_tx_rnmdr_stage0",
        "gc_tx_tnmdr_stage0"
    };
    HAL_ABORT(qid < CAPRI_HBM_GC_NUM_QID);

    int ret = capri_program_label_to_offset("p4plus",
                                            progname,
                                            (char *)labelname[qid],
                                            offset);
    if(ret < 0) {
        return HAL_RET_HW_FAIL;
    }
    HAL_TRACE_DEBUG("Received offset for stage0 program: {:#x}", *offset);
    return HAL_RET_OK;
}

static hal_ret_t
p4pd_add_or_del_gc_tx_stage0_entry(int qid, uint64_t ring_base, int ring_shift, bool del)
{
    gc_tx_initial_action_d                      data = {0};
    hal_ret_t                                   ret = HAL_RET_OK;
    uint64_t                                    pc_offset = 0;
    uint64_t                                    addr;

    // hardware index for this entry
    addr = g_lif_manager->GetLIFQStateAddr(SERVICE_LIF_GC, 0, qid);

    if(!del) {
        // get pc address
        if(p4pd_get_gc_tx_stage0_prog_offset(qid, &pc_offset) != HAL_RET_OK) {
            HAL_TRACE_ERR("Failed to get pc address");
            ret = HAL_RET_HW_FAIL;
        }
        pc_offset = (pc_offset >> 6);
        HAL_TRACE_DEBUG("programming action-id: {:#x}", pc_offset);
        data.action_id = pc_offset;
        data.u.initial_action_d.total = 5;
        data.u.initial_action_d.ring_base = ring_base;
        data.u.initial_action_d.ring_shift = ring_shift;
    }

    HAL_TRACE_DEBUG("Programming gc tx qid {0} stage0 at addr: 0x{1:x}",
            qid, addr);
    HAL_TRACE_DEBUG("ring_base 0x{0:x}, ring_shift 0x{1:x}",
            ring_base, ring_shift);
    if(!p4plus_hbm_write(addr,  (uint8_t *)&data, sizeof(data))){
        HAL_TRACE_ERR("Failed to create tx: stage0 entry for addr");
        ret = HAL_RET_HW_FAIL;
    }
    return ret;
}

hal_ret_t
p4pd_init_gc_cbs(void)
{
    wring_hw_id_t       ring_base;
    hal_ret_t           ret;

    /*
     * QID 0 is for RNMDR
     */
    ret = wring_pd_get_base_addr(types::WRING_TYPE_NMDR_RX_GC, 0, &ring_base);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Failed to get ring base for NMDR_RX");
        return ret;
    }
    p4pd_add_or_del_gc_tx_stage0_entry(P4PD_HBM_GC_RNMDR_QID, 
            ring_base, P4PD_HBM_GC_PER_PRODUCER_RING_SHIFT, false);

    /*
     * QID 1 is for TNMDR
     */
    ret = wring_pd_get_base_addr(types::WRING_TYPE_NMDR_TX_GC, 0, &ring_base);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Failed to get ring base for NMDR_RX");
        return ret;
    }
    p4pd_add_or_del_gc_tx_stage0_entry(P4PD_HBM_GC_TNMDR_QID, 
            ring_base, P4PD_HBM_GC_PER_PRODUCER_RING_SHIFT, false);

    return HAL_RET_OK;
}

}    // namespace pd
}    // namespace hal
