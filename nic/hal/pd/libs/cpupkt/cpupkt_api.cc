#include "nic/hal/pd/libs/wring/wring_pd.hpp"
#include "nic/hal/pd/cpupkt_api.hpp"
#include "nic/include/pd_api.hpp"
#include "nic/hal/pd/iris/hal_state_pd.hpp"
#include "nic/hal/pd/capri/capri_hbm.hpp"
#include "nic/asm/cpu-p4plus/include/cpu-defines.h"
#include "nic/include/capri_common.h"

namespace hal {
namespace pd {

thread_local uint32_t gc_pindex = 0;
thread_local uint32_t cpu_tx_page_pindex = 0;
thread_local uint32_t cpu_tx_page_cindex = 0;
thread_local uint32_t cpu_tx_descr_pindex = 0;
thread_local uint32_t cpu_tx_descr_cindex = 0;
thread_local int      tg_cpu_id = -1;

hal_ret_t cpupkt_descr_free(cpupkt_hw_id_t descr_addr);

/****************************************************************
 * Helper functions
 ***************************************************************/

static inline bool
cpupkt_hbm_write(uint64_t addr, uint8_t* data, uint32_t size_in_bytes)
{
    hal_ret_t rv = hal::pd::asic_mem_write(addr, data, size_in_bytes);
    return rv == HAL_RET_OK ? true : false;
}

static inline bool
cpupkt_hbm_read(uint64_t addr, uint8_t* data, uint32_t size_in_bytes)
{
    hal_ret_t rv = hal::pd::asic_mem_read(addr, data, size_in_bytes);
    return rv == HAL_RET_OK ? true : false;
}

static inline bool
cpupkt_reg_read(uint64_t addr, uint32_t* data)
{
    hal_ret_t rv = hal::pd::asic_reg_read(addr, data);
    return rv == HAL_RET_OK ? true : false;
}

static inline bool
cpupkt_reg_write(uint64_t addr, uint32_t* data)
{
    hal_ret_t rv = hal::pd::asic_reg_write(addr, data);
    return rv == HAL_RET_OK ? true : false;
}

static inline cpupkt_ctxt_t*
cpupkt_ctxt_alloc(void)
{
    return (cpupkt_ctxt_t *)g_hal_state_pd->cpupkt_slab()->alloc();
}

static inline cpupkt_qinst_info_t*
cpupkt_ctxt_qinst_info_alloc(void)
{
    return (cpupkt_qinst_info_t*) g_hal_state_pd->cpupkt_qinst_info_slab()->alloc();
}

static inline void
cpupkt_ctxt_qinst_info_free(cpupkt_qinst_info_t* qinst_info) {
    hal::pd::delay_delete_to_slab(HAL_SLAB_CPUPKT_QINST_INFO_PD, qinst_info);
}

bool is_cpu_send_comp_queue(types::WRingType type)
{
    return(type == types::WRING_TYPE_ASCQ);
}

bool is_cpu_rx_queue(types::WRingType type)
{
    return (type == types::WRING_TYPE_ARQRX || type == types::WRING_TYPE_ARQTX || is_cpu_send_comp_queue(type));
}

bool is_cpu_tx_queue(types::WRingType type)
{
    return ((type == types::WRING_TYPE_ASQ) || (type == types::WRING_TYPE_ASESQ) ||
            (type == types::WRING_TYPE_APP_REDIR_RAWC) ||
            (type == types::WRING_TYPE_APP_REDIR_PROXYC));
}

void
cpupkt_update_slot_addr(cpupkt_qinst_info_t* qinst_info)
{
    if (!qinst_info || !qinst_info->queue_info || !qinst_info->queue_info->wring_meta) {
        HAL_TRACE_ERR("Invalid queue info");
        return;
    }

    uint32_t slot_index = qinst_info->pc_index % qinst_info->queue_info->wring_meta->num_slots;
    /* we initialize pc_index to num_slots. Subtract num_slots before calculating valid bit value for the PI */
    uint32_t queue_tbl_shift = log2(qinst_info->queue_info->wring_meta->num_slots);
    qinst_info->valid_bit_value = (qinst_info->pc_index - qinst_info->queue_info->wring_meta->num_slots) \
        & (0x1 << queue_tbl_shift);
    qinst_info->valid_bit_value = qinst_info->valid_bit_value << (63 - queue_tbl_shift);

    cpupkt_hw_id_t hw_id = qinst_info->base_addr +
            (slot_index * qinst_info->queue_info->wring_meta->slot_size_in_bytes);
    qinst_info->pc_index_addr = hw_id;
    HAL_TRACE_DEBUG("updated pc_index queue: type: {} id: {}, index: {} addr: {:#x}: valid_bit_value: {:#x}",
                    qinst_info->queue_info->type, qinst_info->queue_id,
                    qinst_info->pc_index, hw_id, qinst_info->valid_bit_value);
    return;
}

hal_ret_t
pd_cpupkt_get_slot_addr(types::WRingType type, uint32_t wring_id, uint32_t index, cpupkt_hw_id_t *slot_addr)
{
    hal_ret_t      ret = HAL_RET_OK;
    cpupkt_hw_id_t base_addr = 0;
    pd_wring_meta_t *wring_meta = wring_pd_get_meta(type);
    if(!wring_meta) {
        HAL_TRACE_ERR("Failed to get wring meta for type: {}", type);
        return HAL_RET_WRING_NOT_FOUND;
    }

    ret = wring_pd_get_base_addr(type, wring_id, &base_addr);
    if(ret != HAL_RET_OK) {
        HAL_TRACE_ERR("failed to get base address for the wring: {}", type);
        return HAL_RET_ERR;
    }

    *slot_addr = base_addr + ((index % wring_meta->num_slots) * wring_meta->slot_size_in_bytes);
    return HAL_RET_OK;
}

hal_ret_t
pd_cpupkt_free_tx_descr(cpupkt_hw_id_t descr_addr,
                        pd_descr_aol_t *descr)
{
    hal_ret_t         ret = HAL_RET_OK;
    cpupkt_hw_id_t    slot_addr = 0;
    uint64_t          value = 0;

    if(!descr) {
        HAL_TRACE_ERR("invalid arg");
        return HAL_RET_INVALID_ARG;
    }

    // Free page
    ret = pd_cpupkt_get_slot_addr(types::WRING_TYPE_CPUPR, tg_cpu_id,
                                  cpu_tx_page_cindex, &slot_addr);
    if(ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Failed to get to slot addr for page ci: {}",
                      cpu_tx_page_cindex);
        return ret;
    }

    value = htonll(descr->a0);
    HAL_TRACE_DEBUG("cpu-id: {} update cpupr: slot: {:#x} ci: {} page: {:#x}, value: {:#x}",
                    tg_cpu_id, slot_addr, cpu_tx_page_cindex, descr->a0, value);
    if(!cpupkt_hbm_write(slot_addr, (uint8_t *)&value, sizeof(uint64_t))) {
        HAL_TRACE_ERR("Failed to program page to slot");
        return HAL_RET_HW_FAIL;
    }
    cpu_tx_page_cindex++;

    // Free descr
    ret = pd_cpupkt_get_slot_addr(types::WRING_TYPE_CPUDR, tg_cpu_id,
                                  cpu_tx_descr_cindex, &slot_addr);
    if(ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Failed to get to slot addr for descr ci: {}",
                      cpu_tx_descr_cindex);
        return ret;
    }

    value = htonll(descr_addr);
    HAL_TRACE_DEBUG("cpu-id: {} update cpudr: slot: {:#x} ci: {} descr: {:#x}, value: {#x}",
        tg_cpu_id, slot_addr, cpu_tx_descr_cindex, descr_addr, value);
    if(!cpupkt_hbm_write(slot_addr, (uint8_t *)&value, sizeof(uint64_t))) {
        HAL_TRACE_ERR("Failed to program descr to slot");
        return HAL_RET_HW_FAIL;
    }
    cpu_tx_descr_cindex++;

    return HAL_RET_OK;
}
hal_ret_t
cpupkt_rx_upd_sem_index(cpupkt_qinst_info_t& qinst_info, bool init_pindex)
{
    cpupkt_hw_id_t pi_sem_addr = 0;
    cpupkt_hw_id_t ci_sem_addr = 0;
    switch(qinst_info.queue_info->type) {
    case types::WRING_TYPE_ARQRX:
        ci_sem_addr = CAPRI_SEM_ARQ_CI_RAW_ADDR(tg_cpu_id);
        break;
    case types::WRING_TYPE_ASCQ:
        ci_sem_addr = CAPRI_SEM_ASCQ_CI_RAW_ADDR(tg_cpu_id);
        break;
    default:
        return HAL_RET_OK;
    }

    HAL_TRACE_DEBUG("updating CI: type: {}, addr {:#x}, ci: {}",
                    qinst_info.queue_info->type, ci_sem_addr,
                    qinst_info.pc_index);

    if(!cpupkt_reg_write(ci_sem_addr, &qinst_info.pc_index)) {
        HAL_TRACE_ERR("Failed to program CI semaphore");
        return HAL_RET_HW_FAIL;
    }
    if(init_pindex) {
        pi_sem_addr = ci_sem_addr - 4;
        uint32_t value = 0;
        HAL_TRACE_DEBUG("updating PI: type: {}, addr {:#x}, pi: {}",
                        qinst_info.queue_info->type, pi_sem_addr,
                        value);

        if(!cpupkt_reg_write(pi_sem_addr, &value)) {
        HAL_TRACE_ERR("Failed to program PI semaphore");
        return HAL_RET_HW_FAIL;
        }
    }
    return HAL_RET_OK;
}

/****************************************************************
 * Common Packet send/receive APIs
 ***************************************************************/

hal_ret_t
pd_cpupkt_ctxt_alloc_init(pd_func_args_t *pd_func_args)
{
    pd_cpupkt_ctxt_alloc_init_args_t *args = pd_func_args->pd_cpupkt_ctxt_alloc_init;
    args->ctxt = cpupkt_ctxt_init(cpupkt_ctxt_alloc());
    return HAL_RET_OK;
}

hal_ret_t
cpupkt_register_qinst(cpupkt_queue_info_t* ctxt_qinfo, int qinst_index, types::WRingType type, uint32_t queue_id)
{
    HAL_TRACE_DEBUG("creating qinst for type: {}, id: {}", type, queue_id);
    hal_ret_t           ret = HAL_RET_OK;
    if(!ctxt_qinfo || !ctxt_qinfo->wring_meta) {
        HAL_TRACE_ERR("Invalid ARGs to register_qinst");
        return HAL_RET_INVALID_ARG;
    }

    // Verify that the queue inst is not already registered
    if(ctxt_qinfo->qinst_info[queue_id] != NULL) {
        HAL_TRACE_ERR("queue inst is already registered: type: {}, inst: {}", type, queue_id);
        return HAL_RET_OK;
    }

    cpupkt_qinst_info_t* qinst_info = cpupkt_ctxt_qinst_info_alloc();
    if(!qinst_info) {
        HAL_TRACE_ERR("Failed to allocate qinst_info");
        return HAL_RET_NO_RESOURCE;
    }

    // Initialize Queue
    ret = wring_pd_table_init(type, queue_id);
    if(ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Failed to initialize queue: {}, id: {}, ret: {}",
                    type, queue_id, ret);
        return ret;
    }

    // Get queue base
    wring_hw_id_t base_addr = 0;
    ret = wring_pd_get_base_addr(type, queue_id, &base_addr);
    if(ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Failed to get base addr for queue: {}: ret: {}", type, ret);
        return ret;
    }

    qinst_info->base_addr = base_addr;
    qinst_info->queue_id = queue_id;
    qinst_info->queue_info = ctxt_qinfo;

    if(is_cpu_tx_queue(type)) {
        qinst_info->pc_index = 0;
    } else {
        // For rx queues, initialize CI to wring slot, to detect Queue full condition.
        qinst_info->pc_index = qinst_info->queue_info->wring_meta->num_slots;
        cpupkt_rx_upd_sem_index(*qinst_info, true);
    }

    cpupkt_update_slot_addr(qinst_info);
    ctxt_qinfo->qinst_info[qinst_index] = qinst_info;
    ctxt_qinfo->num_qinst++;

    return HAL_RET_OK;
}

hal_ret_t
pd_cpupkt_register_tx_queue(pd_func_args_t *pd_func_args)
{
    pd_cpupkt_register_tx_queue_args_t *args = pd_func_args->pd_cpupkt_register_tx_queue;
    cpupkt_ctxt_t* ctxt = args->ctxt;
    types::WRingType type = args->type;
    uint32_t queue_id = args->queue_id;

    HAL_TRACE_DEBUG("register Tx Queue: type:{} id:{}", type, queue_id);
    if(!ctxt) {
        HAL_TRACE_ERR("ctxt is null");
        return HAL_RET_INVALID_ARG;
    }

    if(!is_cpu_tx_queue(type)) {
        HAL_TRACE_ERR("Queue is not a valid cpu queue: {}", type);
        return HAL_RET_INVALID_ARG;
    }

    pd_wring_meta_t* meta = wring_pd_get_meta(type);
    if(!meta) {
        HAL_TRACE_ERR("Failed to find meta for the queue: {}", type);
        return HAL_RET_WRING_NOT_FOUND;
    }

    // in Tx case, queue info is indexed based on type for faster lookup.
    int index = type;
    ctxt->tx.queue[index].type = type;
    ctxt->tx.queue[index].wring_meta = meta;
    cpupkt_register_qinst(&(ctxt->tx.queue[index]), queue_id, type, queue_id);
    return HAL_RET_OK;
}

hal_ret_t
pd_cpupkt_register_rx_queue(pd_func_args_t *pd_func_args)
{
    pd_cpupkt_register_rx_queue_args_t *args = pd_func_args->pd_cpupkt_register_rx_queue;
    cpupkt_ctxt_t* ctxt = args->ctxt;
    types::WRingType type = args->type;
    uint32_t queue_id = args->queue_id;

    HAL_TRACE_DEBUG("register Rx Queue: type:{} id:{}", type, queue_id);

    if(!ctxt) {
        HAL_TRACE_ERR("cpupkt:Ctxt is null");
        return HAL_RET_INVALID_ARG;
    }

    if(!is_cpu_rx_queue(type)) {
        HAL_TRACE_ERR("Queue is not a valid cpu queue: {}", type);
        return HAL_RET_INVALID_ARG;
    }

    // Verify if the queeue is already registered
    for(uint32_t i = 0; i< ctxt->rx.num_queues; i++) {
        if(ctxt->rx.queue[i].type == type) {
            HAL_TRACE_DEBUG("cpupkt:Queue is already added: {}", type);
            return HAL_RET_OK;
        }
    }

    if(ctxt->rx.num_queues == MAX_CPU_PKT_QUEUES) {
        HAL_TRACE_ERR("cpupkt:Max queues registered");
        return HAL_RET_NO_RESOURCE;
    }

    pd_wring_meta_t* meta = wring_pd_get_meta(type);
    if(!meta) {
        HAL_TRACE_ERR("Failed to find meta for the queue: {}", type);
        return HAL_RET_WRING_NOT_FOUND;
    }

    if(is_cpu_send_comp_queue(type) && tg_cpu_id < 0) {
        tg_cpu_id = args->queue_id;
        HAL_TRACE_DEBUG("Updated cpu_id to: {}", tg_cpu_id);
    }

    int index = ctxt->rx.num_queues;
    ctxt->rx.queue[index].type = type;
    ctxt->rx.queue[index].wring_meta = meta;
    cpupkt_register_qinst(&(ctxt->rx.queue[index]), ctxt->rx.queue[index].num_qinst, type, queue_id);
    ctxt->rx.num_queues++;
    return HAL_RET_OK;
}

hal_ret_t
pd_cpupkt_unregister_tx_queue(pd_func_args_t *pd_func_args)
{
    pd_cpupkt_unregister_tx_queue_args_t *args = pd_func_args->pd_cpupkt_unregister_tx_queue;
    cpupkt_ctxt_t* ctxt = args->ctxt;
    types::WRingType type = args->type;
    uint32_t queue_id = args->queue_id;

    HAL_TRACE_DEBUG("unregister Tx Queue: type:{} id:{}", type, queue_id);

    if(!ctxt) {
        HAL_TRACE_ERR("Ctxt is null");
        return HAL_RET_INVALID_ARG;
    }

    // in Tx case, queue info is indexed based on type for faster lookup.
    int index = type;
    ctxt->tx.queue[index].type = types::WRING_TYPE_NONE;
    ctxt->tx.queue[index].wring_meta = NULL;
    if(ctxt->tx.queue[index].qinst_info[queue_id]) {
        cpupkt_ctxt_qinst_info_free(ctxt->tx.queue[index].qinst_info[queue_id]);
        ctxt->tx.queue[index].qinst_info[queue_id] = NULL;
    }
    return HAL_RET_OK;
}

/****************************************************************
 * Packet Receive APIs
 ***************************************************************/

static inline void
cpupkt_inc_queue_index(cpupkt_qinst_info_t& qinst_info)
{
    qinst_info.pc_index++;
    cpupkt_update_slot_addr(&qinst_info);
    //HAL_TRACE_DEBUG("incremented  pc_index queue: {} to index: {} addr: {:#x}",
    //                    qinst_info.queue_id,  qinst_info.pc_index, qinst_info.pc_index_addr);
}

static inline hal_ret_t
cpupkt_free_and_inc_queue_index(cpupkt_qinst_info_t& qinst_info)
{
    // Set the slot back to 0
    uint64_t value = 0;
    if(!cpupkt_hbm_write(qinst_info.pc_index_addr,
                         (uint8_t *)&value,
                         sizeof(uint64_t))) {
        HAL_TRACE_ERR("Failed to reset pc_index_addr");
        return HAL_RET_HW_FAIL;
    }
    cpupkt_inc_queue_index(qinst_info);
    //HAL_TRACE_DEBUG("freed and inc pc_index queue: {} index: {} addr: {:#x}",
    //                    qinst_info.queue_id,  qinst_info.pc_index, qinst_info.pc_index_addr);

    return HAL_RET_OK;
}

bool
is_valid_slot_value(cpupkt_qinst_info_t* qinst_info, uint64_t slot_value, uint64_t* descr_addr)
{
    if((slot_value > 0) && ((slot_value & CPU_PKT_VALID_BIT_MASK) == qinst_info->valid_bit_value)) {
        *descr_addr = (qinst_info->valid_bit_value == 0) ? slot_value : (slot_value & ~(uint64_t)CPU_PKT_VALID_BIT_MASK);
        //HAL_TRACE_DEBUG("descr_addr: {:#x}", *descr_addr);
        return true;
    }
    *descr_addr = 0;
    return false;
}


hal_ret_t
cpupkt_descr_to_headers(pd_descr_aol_t& descr,
                        p4_to_p4plus_cpu_pkt_t** flow_miss_hdr,
                        uint8_t** data,
                        size_t* data_len)
{
    if(!flow_miss_hdr || !data || !data_len) {
        return HAL_RET_INVALID_ARG;
    }

    HAL_TRACE_DEBUG("Descriptor0: a: {:#x}, o: {}, l: {}",
                     descr.a0, descr.o0, descr.l0);
    HAL_TRACE_DEBUG("Descriptor1: a: {:#x}, o: {}, l: {}",
                     descr.a1, descr.o1, descr.l1);
    HAL_TRACE_DEBUG("Descriptor2: a: {:#x}, o: {}, l: {}",
                     descr.a2, descr.o2, descr.l2);

    /*
     * App redirect proxied packets (from TCP/TLS) may use more than one
     * page (currently up to 2) to hold meta headers and payload.
     */
    if((descr.l0 > JUMBO_FRAME_SIZE) ||
       (descr.l1 > JUMBO_FRAME_SIZE) ) {
        HAL_TRACE_DEBUG("corrupted packet");
        return HAL_RET_HW_FAIL;
    }

    if((descr.a0 == 0) ||
       (descr.l0 + descr.l1) < sizeof(p4_to_p4plus_cpu_pkt_t)) {
        // Packet should always have a valid address and
        // packet length should be minimum of cpupkt header.
        HAL_TRACE_DEBUG("Received packet with invalid address/length");
        return HAL_RET_HW_FAIL;
    }

    //hard-coding to 9K buffer for now. needs to be replaced with page.
    uint8_t* buffer = (uint8_t* ) malloc(9216);

    uint64_t pktaddr = descr.a0 + descr.o0;

    if(!cpupkt_hbm_read(pktaddr, buffer, descr.l0)) {
        HAL_TRACE_ERR("Failed to read hbm page");
        free(buffer);
        return HAL_RET_HW_FAIL;
    }
    if(descr.l1) {
        uint64_t pktaddr = descr.a1 + descr.o1;
        if (!cpupkt_hbm_read(pktaddr, buffer + descr.l0, descr.l1)) {
            HAL_TRACE_ERR("Failed to read hbm page 1");
            free(buffer);
            return HAL_RET_HW_FAIL;
        }
    }

    *flow_miss_hdr = (p4_to_p4plus_cpu_pkt_t*)buffer;
    *data = buffer + sizeof(p4_to_p4plus_cpu_pkt_t);
    *data_len = (descr.l0 + descr.l1) - sizeof(p4_to_p4plus_cpu_pkt_t);
    return HAL_RET_OK;
}

hal_ret_t
pd_cpupkt_poll_receive(pd_func_args_t *pd_func_args)
{
    hal_ret_t   ret = HAL_RET_OK;
    pd_cpupkt_poll_receive_args_t *args = pd_func_args->pd_cpupkt_poll_receive;
    cpupkt_ctxt_t* ctxt = args->ctxt;
    p4_to_p4plus_cpu_pkt_t** flow_miss_hdr = args->flow_miss_hdr;
    uint8_t** data = args->data;
    size_t* data_len = args->data_len;

    if(!ctxt) {
        return HAL_RET_INVALID_ARG;
    }
    //HAL_TRACE_DEBUG("cpupkt:Starting packet poll for queue: {}", ctxt->rx.num_queues);
    uint64_t value, descr_addr;
    cpupkt_qinst_info_t* qinst_info = NULL;

    for(uint32_t i=0; i< ctxt->rx.num_queues; i++) {
        value = 0;
        qinst_info = ctxt->rx.queue[i].qinst_info[0];
        if(!cpupkt_hbm_read(qinst_info->pc_index_addr,
                            (uint8_t *)&value,
                            sizeof(uint64_t))) {
            HAL_TRACE_ERR("Failed to read the data from the hw");
            return HAL_RET_HW_FAIL;
        }
        value = ntohll(value);
        if(!is_valid_slot_value(qinst_info, value, &descr_addr)) {
            continue;
        }

        HAL_TRACE_DEBUG("Received valid data: queue: {}, qid: {} pc_index: {}, addr: {:#x}, value: {:#x}, descr_addr: {:#x}",
                        ctxt->rx.queue[i].type, qinst_info->queue_id, qinst_info->pc_index, qinst_info->pc_index_addr, value, descr_addr);
        // get the descriptor
        pd_descr_aol_t  descr = {0};
        if(!cpupkt_hbm_read(descr_addr, (uint8_t*)&descr, sizeof(pd_descr_aol_t))) {
            HAL_TRACE_ERR("Failed to read the descr from hw");
            return HAL_RET_HW_FAIL;
        }

        if(is_cpu_send_comp_queue(ctxt->rx.queue[i].type)) {
            ret = pd_cpupkt_free_tx_descr(descr_addr, &descr);
            if(ret != HAL_RET_OK) {
                HAL_TRACE_ERR("Failed to free tx descr: {}", ret);
            }
            ret = HAL_RET_RETRY;
        } else {
            ret = cpupkt_descr_to_headers(descr, flow_miss_hdr, data, data_len);
            if(ret != HAL_RET_OK) {
                HAL_TRACE_ERR("Failed to convert descr to headers");
            } else {
                ret = cpupkt_descr_free(descr_addr);
                if(ret != HAL_RET_OK) {
                    HAL_TRACE_ERR("Failed to free descr");
                }
            }
        }
        cpupkt_inc_queue_index(*qinst_info);

        cpupkt_rx_upd_sem_index(*qinst_info, false);

        return ret;
    }

    return HAL_RET_RETRY;
}

hal_ret_t
pd_cpupkt_free(pd_func_args_t *pd_func_args)
{
    pd_cpupkt_free_args_t *args = pd_func_args->pd_cpupkt_free;
    p4_to_p4plus_cpu_pkt_t* flow_miss_hdr = args->flow_miss_hdr;
    free(flow_miss_hdr);
    return HAL_RET_OK;
}

hal_ret_t
cpupkt_descr_free(cpupkt_hw_id_t descr_addr)
{
    hal_ret_t       ret = HAL_RET_OK;
    wring_hw_id_t   gc_slot_addr=0;
    pd_func_args_t  pd_func_args = {0};

    // program GC queue
    ret = wring_pd_get_base_addr(types::WRING_TYPE_NMDR_RX_GC,
                                 CAPRI_RNMDR_GC_CPU_ARM_RING_PRODUCER,
                                 &gc_slot_addr);
    gc_slot_addr += (gc_pindex * CAPRI_HBM_RNMDR_ENTRY_SIZE);

    uint64_t value = htonll(descr_addr);
    HAL_TRACE_DEBUG("Programming GC queue: {:#x}, descr: {:#x}, gc_pindex: {}, value: {:#x}",
                        gc_slot_addr, descr_addr, gc_pindex, value);
    if(!cpupkt_hbm_write(gc_slot_addr, (uint8_t*)&value,
                         sizeof(cpupkt_hw_id_t))) {
        HAL_TRACE_ERR("Failed to program gc queue");
        return HAL_RET_HW_FAIL;
    }

    // ring doorbell
    // Ring doorbell
    pd_cpupkt_program_send_ring_doorbell_args_t d_args = {0};
    d_args.dest_lif = SERVICE_LIF_GC;
    d_args.qtype = CAPRI_HBM_GC_RNMDR_QTYPE;
    d_args.qid = CAPRI_RNMDR_GC_CPU_ARM_RING_PRODUCER;
    d_args.ring_number = 0;
    d_args.flags = DB_IDX_UPD_PIDX_SET | DB_SCHED_UPD_SET;
    d_args.pidx = (gc_pindex + 1) & CAPRI_HBM_GC_PER_PRODUCER_RING_MASK;
    pd_func_args.pd_cpupkt_program_send_ring_doorbell = &d_args;
    ret = pd_cpupkt_program_send_ring_doorbell(&pd_func_args);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Failed to ring doorbell");
        return HAL_RET_HW_FAIL;
    }

    gc_pindex = (gc_pindex + 1) & CAPRI_HBM_GC_PER_PRODUCER_RING_MASK;

    return HAL_RET_OK;
}

/****************************************************************
 * Packet Send APIs
 ***************************************************************/

hal_ret_t
pd_cpupkt_descr_alloc(pd_func_args_t *pd_func_args)
{
    hal_ret_t           ret = HAL_RET_OK;
    pd_cpupkt_descr_alloc_args_t *args = pd_func_args->pd_cpupkt_descr_alloc;
    cpupkt_hw_id_t      slot_addr;
    cpupkt_hw_id_t      *descr_addr = args->descr_addr;

    if(!descr_addr) {
        return HAL_RET_INVALID_ARG;
    }

    // PI/CI check
    if((cpu_tx_descr_pindex + 1) == cpu_tx_descr_cindex) {
        HAL_TRACE_ERR("No free descr entries available: pi: {}, ci: {}",
                      cpu_tx_descr_pindex, cpu_tx_descr_cindex);
        return HAL_RET_NO_RESOURCE;
    }

    ret = pd_cpupkt_get_slot_addr(types::WRING_TYPE_CPUDR, tg_cpu_id,
                                  cpu_tx_descr_pindex, &slot_addr);
    if(ret != HAL_RET_OK) {
        HAL_TRACE_ERR("failed to get to slot addr for descr pi: {}",
                      cpu_tx_descr_pindex);
        return ret;
    }

    if(!cpupkt_hbm_read(slot_addr, (uint8_t *)descr_addr, sizeof(cpupkt_hw_id_t))) {
        HAL_TRACE_ERR("Failed to read descr addr from CPUDR PI: {}, slot: {:#x}",
                      cpu_tx_descr_pindex, slot_addr);
        return HAL_RET_HW_FAIL;
    }
    *descr_addr = ntohll(*descr_addr);
    HAL_TRACE_DEBUG("cpupdr allocated pi: {} slot_addr: {:#x} descr: {:#x}",
                    cpu_tx_descr_pindex, slot_addr, *descr_addr);
    cpu_tx_descr_pindex++;

    return HAL_RET_OK;
}

hal_ret_t
pd_cpupkt_page_alloc(pd_func_args_t *pd_func_args)
{
    hal_ret_t           ret = HAL_RET_OK;
    pd_cpupkt_page_alloc_args_t *args = pd_func_args->pd_cpupkt_page_alloc;
    cpupkt_hw_id_t      slot_addr;
    cpupkt_hw_id_t* page_addr = args->page_addr;

    if(!page_addr) {
        return HAL_RET_INVALID_ARG;
    }

    // PI/CI check
    if((cpu_tx_page_pindex + 1) == cpu_tx_page_cindex) {
        HAL_TRACE_ERR("No free page entries available: pi: {}, ci: {}",
                      cpu_tx_page_pindex, cpu_tx_page_cindex);
        return HAL_RET_NO_RESOURCE;
    }

    ret = pd_cpupkt_get_slot_addr(types::WRING_TYPE_CPUPR, tg_cpu_id,
                                  cpu_tx_page_pindex, &slot_addr);
    if(ret != HAL_RET_OK) {
        HAL_TRACE_ERR("failed to get to slot addr for page pi: {}",
                      cpu_tx_page_pindex);
        return ret;
    }

    if(!cpupkt_hbm_read(slot_addr, (uint8_t *)page_addr, sizeof(cpupkt_hw_id_t))) {
        HAL_TRACE_ERR("Failed to read page address from CPUPR PI: {} slot: {:#x}",
                      cpu_tx_page_pindex, slot_addr);
        return HAL_RET_HW_FAIL;
    }
    *page_addr = ntohll(*page_addr);
    HAL_TRACE_DEBUG("cpupr allocated pi: {}, slot_addr: {:#x} page: {:#x}",
                    cpu_tx_page_pindex, slot_addr, *page_addr);
    cpu_tx_page_pindex++;
    return HAL_RET_OK;
}

hal_ret_t
cpupkt_program_descr(uint32_t qid, cpupkt_hw_id_t page_addr, size_t len, cpupkt_hw_id_t* descr_addr)
{
    hal_ret_t       ret = HAL_RET_OK;
    pd_descr_aol_t  descr = {0};
    pd_cpupkt_descr_alloc_args_t args;
    pd_func_args_t pd_func_args = {0};

    args.descr_addr = descr_addr;
    pd_func_args.pd_cpupkt_descr_alloc = &args;
    ret = pd_cpupkt_descr_alloc(&pd_func_args);
    if(ret != HAL_RET_OK) {

        HAL_TRACE_ERR("failed to allocate descr for the packet, err: {}", ret);
        goto cleanup;
    }

    descr.a0 = page_addr;
    descr.o0 = 0;
    descr.l0 = len;

    HAL_TRACE_DEBUG("programming descr: descr_addr: {:#x}", *descr_addr);
    if(!cpupkt_hbm_write(*descr_addr, (uint8_t*)&descr, sizeof(pd_descr_aol_t))) {
        HAL_TRACE_ERR("failed to program descr");
        ret = HAL_RET_HW_FAIL;
        goto cleanup;
    }

    ret = HAL_RET_OK;

cleanup:
    return ret;

}

hal_ret_t
cpupkt_program_send_queue(cpupkt_ctxt_t* ctxt, types::WRingType type, uint32_t queue_id, cpupkt_hw_id_t descr_addr)
{
    if(!ctxt) {
        return HAL_RET_INVALID_ARG;
    }
    cpupkt_qinst_info_t* qinst_info = ctxt->tx.queue[type].qinst_info[queue_id];
    if(!qinst_info) {
        HAL_TRACE_ERR("qinst for type: {} qid: {} is not registered", type, queue_id);
        return HAL_RET_QUEUE_NOT_FOUND;
    }
    HAL_TRACE_DEBUG("Programming send queue: addr: {:#x} value: {:#x}",
                        qinst_info->pc_index_addr, descr_addr);
    if(!cpupkt_hbm_write(qinst_info->pc_index_addr, (uint8_t*) &descr_addr, sizeof(cpupkt_hw_id_t))) {
        HAL_TRACE_ERR("Failed to program send queue");
        return HAL_RET_HW_FAIL;
    }

    return HAL_RET_OK;
}

hal_ret_t
pd_cpupkt_program_send_ring_doorbell(pd_func_args_t *pd_func_args)
{
    pd_cpupkt_program_send_ring_doorbell_args_t *args = pd_func_args->pd_cpupkt_program_send_ring_doorbell;
    uint64_t            addr = 0;
    uint64_t            data = 0;
    uint16_t dest_lif = args->dest_lif;
    uint8_t  qtype = args->qtype;
    uint32_t qid = args->qid;
    uint8_t  ring_number = args->ring_number;
    uint64_t            qid64 = qid;


    addr = 0 | args->flags;
    addr = addr << DB_UPD_SHFT;
    addr += (dest_lif << DB_LIF_SHFT);
    addr += (qtype << DB_TYPE_SHFT);
    addr += DB_ADDR_BASE;

    data += ((uint64_t)CPU_ASQ_PID << DB_PID_SHFT);
    data += (qid64 << DB_QID_SHFT);
    data += (ring_number << DB_RING_SHFT);
    data += (args->pidx);

    HAL_TRACE_DEBUG("ringing Doorbell with addr: {:#x} data: {:#x}",
                    addr, data);
    hal::pd::asic_ring_doorbell(addr, data);
    return HAL_RET_OK;
}

void
pd_swizzle_header (uint8_t *hdr, uint32_t hdr_len)
{
#if __BYTE_ORDER == __BIG_ENDIAN

#else /* __BYTE_ORDER == __BIG_ENDIAN */
    for (uint32_t i = 0; i < (hdr_len >> 1); i++) {
        uint8_t temp = hdr[i];
        hdr[i] = hdr[hdr_len - i - 1];
        hdr[hdr_len - i - 1] = temp;
    }
#endif /* __BYTE_ORDER == __BIG_ENDIAN */
}

hal_ret_t
pd_cpupkt_send(pd_func_args_t *pd_func_args)
{
    hal_ret_t           ret = HAL_RET_OK;
    pd_cpupkt_send_args_t *s_args = pd_func_args->pd_cpupkt_send;
    cpupkt_hw_id_t      page_addr = 0;
    cpupkt_hw_id_t      descr_addr = 0;
    cpupkt_hw_id_t      write_addr = 0;
    size_t              write_len = 0;
    size_t              total_len = 0;
    cpupkt_ctxt_t* ctxt = s_args->ctxt;
    types::WRingType type = s_args->type;
    uint32_t queue_id = s_args->queue_id;
    cpu_to_p4plus_header_t* cpu_header = s_args->cpu_header;
    p4plus_to_p4_header_t* p4_header = s_args->p4_header;
    uint8_t* data = s_args->data;
    size_t data_len = s_args->data_len;
    uint16_t dest_lif = s_args->dest_lif;
    uint8_t  qtype = s_args->qtype;
    uint32_t qid = s_args->qid;
    uint8_t ring_number = s_args->ring_number;
    pd_func_args_t pd_func_args1 = {0};

    if(!ctxt || !data) {
        return HAL_RET_INVALID_ARG;
    }

    // Allocate a page and descr for the pkt
    pd_cpupkt_page_alloc_args_t args;
    args.page_addr = &page_addr;
    pd_func_args1.pd_cpupkt_page_alloc = &args;
    ret = pd_cpupkt_page_alloc(&pd_func_args1);
    if(ret != HAL_RET_OK) {
        HAL_TRACE_ERR("failed to allocate page for the packet, err: {}", ret);
        goto cleanup;
    }

    // copy the packet to hbm page.
    // TODO: REMOVE once direct hbm access is available

    write_addr = page_addr;
    if (cpu_header != NULL) {
        // CPU header
        // update l2 header offset to include headers
        cpu_header->l2_offset += (sizeof(cpu_to_p4plus_header_t) +
                                  sizeof(p4plus_to_p4_header_t) +
                                  L2HDR_DOT1Q_OFFSET);

        cpu_header->flags |= CPU_TO_P4PLUS_FLAGS_FREE_BUFFER;

        write_len = sizeof(cpu_to_p4plus_header_t);
        total_len += write_len;
        HAL_TRACE_DEBUG("Copying CPU header of len: {} to addr: {:#x}, l2offset: {:#x}",
                        write_len, write_addr, cpu_header->l2_offset);
        if(!cpupkt_hbm_write(write_addr, (uint8_t *)cpu_header, write_len)) {
            HAL_TRACE_ERR("Failed to copy packet to the page");
            ret = HAL_RET_HW_FAIL;
            goto cleanup;
        }

        // P4plus_to_p4_header_t
        write_addr += write_len; // shift address
        write_len = sizeof(p4plus_to_p4_header_t);
        pd_swizzle_header ((uint8_t *)p4_header, sizeof(p4plus_to_p4_header_t));
        total_len += write_len;
        HAL_TRACE_DEBUG("Copying P4Plus to P4 header of len: {} to addr: {:#x}",
                        write_len, write_addr);
        if(!cpupkt_hbm_write(write_addr, (uint8_t *)p4_header, write_len)) {
            HAL_TRACE_ERR("Failed to copy packet to the page");
            ret = HAL_RET_HW_FAIL;
            goto cleanup;
        }

        write_addr += write_len;
    }
    // Data
    write_len = data_len;
    total_len += write_len;
    HAL_TRACE_DEBUG("copying data of len: {} to page at addr: {:#x}", write_len, write_addr);
    if(!cpupkt_hbm_write(write_addr, data, write_len)) {
        HAL_TRACE_ERR("Failed to copy packet to the page");
        ret = HAL_RET_HW_FAIL;
        goto cleanup;
    }

    HAL_TRACE_DEBUG("total pkt len: {} to page at addr: {:#x}", total_len, page_addr);

    ret = cpupkt_program_descr(qid, page_addr, total_len, &descr_addr);
    if(ret != HAL_RET_OK) {
        goto cleanup;
    }

    // Program Queue
    ret = cpupkt_program_send_queue(ctxt, type, queue_id, descr_addr);
    if(ret != HAL_RET_OK) {
        goto cleanup;
    }

    // Ring doorbell
    pd_cpupkt_program_send_ring_doorbell_args_t d_args;
    d_args.dest_lif = dest_lif;
    d_args.qtype = qtype;
    d_args.qid = qid;
    d_args.ring_number = ring_number;
    d_args.flags = DB_IDX_UPD_PIDX_INC | DB_SCHED_UPD_SET;
    pd_func_args1.pd_cpupkt_program_send_ring_doorbell = &d_args;
    ret = pd_cpupkt_program_send_ring_doorbell(&pd_func_args1);
    if(ret != HAL_RET_OK) {
        goto cleanup;
    }
    cpupkt_inc_queue_index(*(ctxt->tx.queue[type].qinst_info[queue_id]));
    return HAL_RET_OK;
cleanup:
    // FIXME
    return ret;
}


} // namespace pd
} // namespace hal
