#include "nic/include/base.hpp"
#include <arpa/inet.h>
#include "nic/sdk/include/sdk/lock.hpp"
#include "nic/include/pd_api.hpp"
#include "nic/sdk/asic/cmn/asic_hbm.hpp"
#include "nic/hal/pd/libs/wring/wring_pd.hpp"
#include "nic/hal/src/internal/proxy.hpp"
#include "nic/hal/hal.hpp"
#include "gen/p4gen/gc_txdma/include/gc_txdma_p4plus_ingress.h"
#include "nic/hal/pd/iris/internal/p4plus_pd_api.h"
#include "nic/hal/pd/iris/internal/gccb_pd.hpp"

namespace hal {
namespace pd {

/********************************************
 * Garbage collector TxDMA
 * ******************************************/
hal_ret_t
p4pd_get_gc_tx_stage0_prog_offset(int qtype, uint64_t* offset)
{
    char progname[] = "txdma_stage0.bin";
    const char *labelname[ASIC_HBM_GC_NUM_QTYPE] = {
        "gc_tx_rnmdr_stage0",
        "gc_tx_tnmdr_stage0"
    };
    HAL_ABORT(qtype < ASIC_HBM_GC_NUM_QTYPE);

    int ret = sdk::p4::p4_program_label_to_offset("p4plus",
                                            progname,
                                            (char *)labelname[qtype],
                                            offset);
    if(ret != 0) {
        return HAL_RET_HW_FAIL;
    }
    HAL_TRACE_DEBUG("Received offset for stage0 program: {:#x}", *offset);
    return HAL_RET_OK;
}

static hal_ret_t
p4pd_add_or_del_gc_tx_stage0_entry(int qtype, int qid, uint64_t ring_base,
        bool del)
{
    gc_tx_initial_action_d                      data = {0};
    hal_ret_t                                   ret = HAL_RET_OK;
    uint64_t                                    pc_offset = 0;
    uint64_t                                    addr;

    // hardware index for this entry
    // addr = lif_manager()->GetLIFQStateAddr(SERVICE_LIF_GC, qtype, qid);
    addr = lif_manager()->get_lif_qstate_addr(SERVICE_LIF_GC, qtype, qid);

    if(!del) {
        // get pc address
        if(p4pd_get_gc_tx_stage0_prog_offset(qtype, &pc_offset) != HAL_RET_OK) {
            HAL_TRACE_ERR("Failed to get pc address");
            ret = HAL_RET_HW_FAIL;
        }
        pc_offset = (pc_offset >> 6);
        HAL_TRACE_DEBUG("programming action-id: {:#x}", pc_offset);
        data.action_id = pc_offset;
        data.u.initial_action_d.total = 1;
        data.u.initial_action_d.ring_base = ring_base;
    }

    HAL_TRACE_DEBUG("Programming gc tx qtype {} qid {} stage0 at addr: {:#x}",
                    qtype, qid, addr);
    if(!p4plus_hbm_write(addr,  (uint8_t *)&data, sizeof(data),
                P4PLUS_CACHE_INVALIDATE_BOTH)){
        HAL_TRACE_ERR("Failed to create tx: stage0 entry for addr");
        ret = HAL_RET_HW_FAIL;
    }
    return ret;
}

/*
 * Program GC CB for
 *  LIF (SERVICE_LIF_GC)
 *  type (type of ring RNMDR/TNMDR)
 *  qid (producer - tcp, tls, ipsec etc.)
 *
 * CB is programmed with
 *  PC = action for type of ring
 *  ring_base (per producer ring base)
 */
static hal_ret_t
p4pd_init_gc_cb(types::WRingType wring, int qtype,
        int producer_id)
{
    hal_ret_t           ret;
    wring_hw_id_t       ring_base;

    ret = wring_pd_get_base_addr(wring, producer_id, &ring_base);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Failed to get ring base for wring_type {}", wring);
        return ret;
    }

    ret = p4pd_add_or_del_gc_tx_stage0_entry(qtype, producer_id,
            ring_base, false);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Failed to program GC CB for qtype {} qid {}",
                qtype, producer_id);
        return ret;
    }

    return ret;
}

static hal_ret_t
p4pd_init_rnmdr_gc_cbs(void)
{
    hal_ret_t ret;

    /*
     * TCP producer to RNMDR
     */
    ret = p4pd_init_gc_cb(types::WRING_TYPE_NMDR_RX_GC,
            ASIC_HBM_GC_RNMDR_QTYPE, ASIC_RNMDR_GC_TCP_RING_PRODUCER);
    if (ret != HAL_RET_OK) {
        return ret;
    }

    /*
     * TLS producer to RNMDR
     */
    ret = p4pd_init_gc_cb(types::WRING_TYPE_NMDR_RX_GC,
            ASIC_HBM_GC_RNMDR_QTYPE, ASIC_RNMDR_GC_TLS_RING_PRODUCER);
    if (ret != HAL_RET_OK) {
        return ret;
    }

    /*
     * IPSEC producer to RNMDR
     */
    ret = p4pd_init_gc_cb(types::WRING_TYPE_NMDR_RX_GC,
            ASIC_HBM_GC_RNMDR_QTYPE, ASIC_RNMDR_GC_IPSEC_RING_PRODUCER);
    if (ret != HAL_RET_OK) {
        return ret;
    }

    /*
     * CPU producer to RNMDR
     */
    ret = p4pd_init_gc_cb(types::WRING_TYPE_NMDR_RX_GC,
            ASIC_HBM_GC_RNMDR_QTYPE, ASIC_RNMDR_GC_CPU_ARM_RING_PRODUCER);
    if (ret != HAL_RET_OK) {
        return ret;
    }

    return ret;
}

static hal_ret_t
p4pd_init_tnmdr_gc_cbs(void)
{
    hal_ret_t ret;

    /*
     * TCP producer to TNMDR
     */
    ret = p4pd_init_gc_cb(types::WRING_TYPE_NMDR_TX_GC,
            ASIC_HBM_GC_TNMDR_QTYPE, ASIC_TNMDR_GC_TCP_RING_PRODUCER);
    if (ret != HAL_RET_OK) {
        return ret;
    }

    /*
     * TLS producer to TNMDR
     */
    ret = p4pd_init_gc_cb(types::WRING_TYPE_NMDR_TX_GC,
            ASIC_HBM_GC_TNMDR_QTYPE, ASIC_TNMDR_GC_TLS_RING_PRODUCER);
    if (ret != HAL_RET_OK) {
        return ret;
    }

    /*
     * IPSEC producer to TNMDR
     */
    ret = p4pd_init_gc_cb(types::WRING_TYPE_NMDR_TX_GC,
            ASIC_HBM_GC_TNMDR_QTYPE, ASIC_TNMDR_GC_IPSEC_RING_PRODUCER);
    if (ret != HAL_RET_OK) {
        return ret;
    }

    return ret;
}


hal_ret_t
p4pd_init_gc_cbs(void)
{
    hal_ret_t           ret;

    /*
     * Qtype 0
     */
    ret = p4pd_init_rnmdr_gc_cbs();
    if (ret != HAL_RET_OK) {
        return ret;
    }

    /*
     * Qtype 1
     */
    ret = p4pd_init_tnmdr_gc_cbs();
    if (ret != HAL_RET_OK) {
        return ret;
    }

    return ret;
}

}    // namespace pd
}    // namespace hal
