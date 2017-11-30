#include "nic/hal/pd/iris/wring_pd.hpp"
#include "nic/hal/pd/common/cpupkt_api.hpp"
#include "nic/hal/pd/iris/p4plus_pd_api.h"
#include "nic/include/pd_api.hpp"
#include "nic/hal/pd/iris/hal_state_pd.hpp"
#include "nic/hal/pd/capri/capri_hbm.hpp"
#include "nic/asm/cpu-p4plus/include/cpu-defines.h"

namespace hal {
namespace pd {

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
    g_hal_state_pd->cpupkt_qinst_info_slab()->free(qinst_info);
}

bool is_cpu_rx_queue(types::WRingType type)
{
    return (type == types::WRING_TYPE_ARQRX || type == types::WRING_TYPE_ARQTX);
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
    if(!qinst_info || !qinst_info->queue_info || !qinst_info->queue_info->wring_meta) {
        HAL_TRACE_ERR("cpupkt: Invalid queue info");
        return;
    }

    uint32_t slot_index = qinst_info->pc_index % qinst_info->queue_info->wring_meta->num_slots;
    cpupkt_hw_id_t hw_id = qinst_info->base_addr +
            (slot_index * qinst_info->queue_info->wring_meta->slot_size_in_bytes);
    qinst_info->pc_index_addr = hw_id;
    HAL_TRACE_DEBUG("Updated pc_index addr: {:#x}", hw_id);
    return;
}

static inline void
cpupkt_inc_queue_index(cpupkt_qinst_info_t& qinst_info)
{
    qinst_info.pc_index++;
    cpupkt_update_slot_addr(&qinst_info);
}

static inline hal_ret_t
cpupkt_free_and_inc_queue_index(cpupkt_qinst_info_t& qinst_info)
{
    // Set the slot back to 0
    uint64_t value = 0;
    if(!p4plus_hbm_write(qinst_info.pc_index_addr,
                         (uint8_t *)&value,
                         sizeof(uint64_t))) {
        HAL_TRACE_ERR("Failed to reset pc_index_addr");
        return HAL_RET_HW_FAIL;
    }
    cpupkt_inc_queue_index(qinst_info);
    return HAL_RET_OK;
}

bool
is_valid_slot_value(uint64_t slot_value, uint64_t* descr_addr)
{
    if((slot_value > 0) && (slot_value & CPU_PKT_VALID_BIT_MASK)) {
        *descr_addr = (slot_value & ~(uint64_t)CPU_PKT_VALID_BIT_MASK);
        HAL_TRACE_DEBUG("CPUPKT descr_addr: {:#x}", *descr_addr);
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
    
    if(descr.l0 > 9216) {
        HAL_TRACE_DEBUG("corrupted packet");
        return HAL_RET_HW_FAIL;
    }
    uint8_t* buffer = (uint8_t* ) malloc(descr.l0);

    uint64_t pktaddr = descr.a0 + descr.o0;

    if(!p4plus_hbm_read(pktaddr, buffer, descr.l0)) {
        HAL_TRACE_ERR("Failed to read hbm page");
        return HAL_RET_HW_FAIL;
    }

    *flow_miss_hdr = (p4_to_p4plus_cpu_pkt_t*)buffer;
    *data = buffer + sizeof(p4_to_p4plus_cpu_pkt_t);
    *data_len = descr.l0 - sizeof(p4_to_p4plus_cpu_pkt_t);
    return HAL_RET_OK;
}


/****************************************************************
 * Packet send/receive APIs
 ***************************************************************/
 
cpupkt_ctxt_t* 
cpupkt_ctxt_alloc_init(void)
{
    return cpupkt_ctxt_init(cpupkt_ctxt_alloc());    
}

hal_ret_t
cpupkt_register_qinst(cpupkt_queue_info_t* ctxt_qinfo, types::WRingType type, uint32_t queue_id)
{
    hal_ret_t           ret = HAL_RET_OK;
    if(!ctxt_qinfo || !ctxt_qinfo->wring_meta) {
        HAL_TRACE_ERR("cpupkt: Invalid ARGs to register_qinst");
        return HAL_RET_INVALID_ARG;
    }

    // Verify that the queue inst is not already registered
    if(ctxt_qinfo->qinst_info[queue_id] != NULL) {
        HAL_TRACE_ERR("cpupkt: queue inst is already registered: type: {}, inst: {}", type, queue_id);
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
    qinst_info->pc_index = 0;
    qinst_info->queue_info = ctxt_qinfo;
    cpupkt_update_slot_addr(qinst_info);
    ctxt_qinfo->qinst_info[queue_id] = qinst_info;

    return HAL_RET_OK;
}

hal_ret_t 
cpupkt_register_tx_queue(cpupkt_ctxt_t* ctxt, types::WRingType type, uint32_t queue_id)
{
    if(!ctxt) {
        HAL_TRACE_ERR("Ctxt is null");
        return HAL_RET_INVALID_ARG;    
    }
    
    if(!is_cpu_tx_queue(type)) {
        HAL_TRACE_ERR("Queue is not a valid cpu queue: {}", type);
        return HAL_RET_INVALID_ARG;    
    }

    pd_wring_meta_t* meta = wring_pd_get_meta(type);
    if(!meta) {
        HAL_TRACE_ERR("cpupkt: Failed to find meta for the queue: {}", type);
        return HAL_RET_WRING_NOT_FOUND;
    }

    // in Tx case, queue info is indexed based on type for faster lookup.
    int index = type;
    ctxt->tx.queue[index].type = type;
    ctxt->tx.queue[index].wring_meta = meta;
    cpupkt_register_qinst(&(ctxt->tx.queue[index]), type, queue_id);
    return HAL_RET_OK;
}


hal_ret_t 
cpupkt_register_rx_queue(cpupkt_ctxt_t* ctxt, types::WRingType type, uint32_t queue_id)
{
    if(!ctxt) {
        HAL_TRACE_ERR("Ctxt is null");
        return HAL_RET_INVALID_ARG;    
    }
    
    if(!is_cpu_rx_queue(type)) {
        HAL_TRACE_ERR("Queue is not a valid cpu queue: {}", type);
        return HAL_RET_INVALID_ARG;    
    }

    // Verify if the queeue is already registered
    for(uint32_t i = 0; i< ctxt->rx.num_queues; i++) {
        if(ctxt->rx.queue[i].type == type) {
            HAL_TRACE_DEBUG("Queue is already added: {}", type);
            return HAL_RET_OK;
        }
    }
    
    if(ctxt->rx.num_queues == MAX_CPU_PKT_QUEUES) {
        HAL_TRACE_ERR("Max queues registered");
        return HAL_RET_NO_RESOURCE;
    }

    pd_wring_meta_t* meta = wring_pd_get_meta(type);
    if(!meta) {
        HAL_TRACE_ERR("cpupkt: Failed to find meta for the queue: {}", type);
        return HAL_RET_WRING_NOT_FOUND;
    }

    int index = ctxt->rx.num_queues;
    ctxt->rx.queue[index].type = type;
    ctxt->rx.queue[index].wring_meta = meta;
    cpupkt_register_qinst(&(ctxt->rx.queue[index]), type, queue_id);
    ctxt->rx.num_queues++;
    return HAL_RET_OK;
}

hal_ret_t 
cpupkt_unregister_tx_queue(cpupkt_ctxt_t* ctxt, types::WRingType type, uint32_t queue_id)
{
    if(!ctxt) {
        HAL_TRACE_ERR("Ctxt is null");
        return HAL_RET_INVALID_ARG;
    }

    HAL_TRACE_DEBUG("cpupkt: Unregister TX queue type: {} inst: {}", type, queue_id);
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


hal_ret_t
cpupkt_poll_receive(cpupkt_ctxt_t* ctxt,
                    p4_to_p4plus_cpu_pkt_t** flow_miss_hdr,
                    uint8_t** data, 
                    size_t* data_len)
{
    hal_ret_t   ret = HAL_RET_OK;
    if(!ctxt) {
        return HAL_RET_INVALID_ARG;    
    }
    //HAL_TRACE_DEBUG("cpupkt:Starting packet poll for queue: {}", ctxt->rx.num_queues);
    uint64_t value, descr_addr;
    cpupkt_qinst_info_t* qinst_info = NULL;

    for(uint32_t i=0; i< ctxt->rx.num_queues; i++) {
        value = 0;
        qinst_info = ctxt->rx.queue[i].qinst_info[0];
        //HAL_TRACE_DEBUG("cpupkt rx: checking queue at address: {:#x}", ctxt->rx.queue[i].pc_index_addr);
        if(!p4plus_hbm_read(qinst_info->pc_index_addr,
                            (uint8_t *)&value, 
                            sizeof(uint64_t))) {
            HAL_TRACE_ERR("Failed to read the data from the hw");
            return HAL_RET_HW_FAIL;
        }
        value = ntohll(value);
        if(!is_valid_slot_value(value, &descr_addr)) {
            if(value > 0) {
                // With DEBUG DOL, P4+ writes the queue without setting VALID BIT.
                // Increase CI to keep it in sync with hw
                cpupkt_inc_queue_index(*qinst_info);
            }
            continue;
        }
        
        HAL_TRACE_DEBUG("cpupkt: Received valid data: queue: {}, pc_index: {}, addr: {:#x}, value: {:#x}, descr_addr: {:#x}",
                        ctxt->rx.queue[i].type, qinst_info->pc_index, qinst_info->pc_index_addr, value, descr_addr);
        // get the descriptor
        pd_descr_aol_t  descr = {0};
        if(!p4plus_hbm_read(descr_addr, (uint8_t*)&descr, sizeof(pd_descr_aol_t))) {
            HAL_TRACE_ERR("Failed to read the descr from hw");
            return HAL_RET_HW_FAIL;
        }
        
        ret = cpupkt_descr_to_headers(descr, flow_miss_hdr, data, data_len);
        if(ret != HAL_RET_OK) {
            HAL_TRACE_ERR("Failed to create skbuff");
        }
        cpupkt_free_and_inc_queue_index(*qinst_info);
        return ret;
    }

    return HAL_RET_RETRY;    
}

hal_ret_t
cpupkt_free(p4_to_p4plus_cpu_pkt_t* flow_miss_hdr, uint8_t* data)
{
    free(flow_miss_hdr);
    return HAL_RET_OK;
}

hal_ret_t
cpupkt_descr_alloc(cpupkt_hw_id_t* descr_addr)
{
    indexer::status     rs;
    uint32_t            descr_index;
    cpupkt_hw_id_t      base_addr;

    if(!descr_addr) {
        return HAL_RET_INVALID_ARG;    
    }

    base_addr = get_start_offset(CAPRI_HBM_REG_CPU_DESCR);
    if(!base_addr) {
        HAL_TRACE_ERR("Failed to get the base addr for the page region");
        return HAL_RET_ERR;
    }
    
    rs = g_hal_state_pd->cpupkt_descr_hw_id_idxr()->alloc(&descr_index);
    if(rs != indexer::SUCCESS) {
        HAL_TRACE_ERR("Failed to allocate descr index, err {}", rs);
        return HAL_RET_NO_RESOURCE;
    }

    *descr_addr = base_addr + (descr_index * CPU_PKT_DESCR_SIZE);
    return HAL_RET_OK;
}

hal_ret_t
cpupkt_page_alloc(cpupkt_hw_id_t* page_addr)
{
    indexer::status     rs;
    uint32_t            page_index;
    cpupkt_hw_id_t      base_addr;

    if(!page_addr) {
        return HAL_RET_INVALID_ARG;    
    }

    base_addr = get_start_offset(CAPRI_HBM_REG_CPU_PAGE);
    if(!base_addr) {
        HAL_TRACE_ERR("Failed to get the base addr for the page region");
        return HAL_RET_ERR;
    }
    
    rs = g_hal_state_pd->cpupkt_page_hw_id_idxr()->alloc(&page_index);
    if(rs != indexer::SUCCESS) {
        HAL_TRACE_ERR("Failed to allocate page index, err {}", rs);
        return HAL_RET_NO_RESOURCE;
    }

    *page_addr = base_addr + (page_index * CPU_PKT_PAGE_SIZE);
    return HAL_RET_OK;
}

hal_ret_t
cpupkt_program_descr(cpupkt_hw_id_t page_addr, size_t len, cpupkt_hw_id_t* descr_addr)
{
    hal_ret_t       ret = HAL_RET_OK;
    pd_descr_aol_t  descr = {0};

    ret = cpupkt_descr_alloc(descr_addr);
    if(ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Failed to allocate descr for the packet, err: {}", ret);
        goto cleanup;
    }
    
    descr.a0 = page_addr;
    descr.o0 = 0;
    descr.l0 = len;

    HAL_TRACE_DEBUG("Programming descr: descr_addr: {:#x}", *descr_addr);
    if(!p4plus_hbm_write(*descr_addr, (uint8_t*)&descr, sizeof(pd_descr_aol_t))) {
        HAL_TRACE_ERR("Failed to program descr");
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
        HAL_TRACE_ERR("cpupkt: qinst is not registered");
        return HAL_RET_QUEUE_NOT_FOUND;
    }
    HAL_TRACE_DEBUG("Programming send queue: addr: {:#x} value: {:#x}",
                        qinst_info->pc_index_addr, descr_addr);
    if(!p4plus_hbm_write(qinst_info->pc_index_addr, (uint8_t*) &descr_addr, sizeof(cpupkt_hw_id_t))) {
        HAL_TRACE_ERR("Failed to program send queue");
        return HAL_RET_HW_FAIL;
    }
    
    return HAL_RET_OK;
}

hal_ret_t
cpupkt_program_send_ring_doorbell(uint16_t dest_lif,
                                  uint8_t  qtype,
                                  uint32_t qid,
                                  uint8_t  ring_number)
{
    uint64_t            addr = 0;
    uint64_t            data = 0;
    uint64_t            qid64 = qid;

    
    addr = 0 | DB_IDX_UPD_PIDX_INC | DB_SCHED_UPD_SET;
    addr = addr << DB_UPD_SHFT;
    addr += (dest_lif << DB_LIF_SHFT);
    addr += (qtype << DB_TYPE_SHFT);
    addr += DB_ADDR_BASE;

    data += ((uint64_t)CPU_ASQ_PID << DB_PID_SHFT);
    data += (qid64 << DB_QID_SHFT);
    data += (ring_number << DB_RING_SHFT);

    HAL_TRACE_DEBUG("Ringing Doorbell with addr: {:#x} data: {:#x}",
                    addr, data);
    hal::pd::asic_ring_doorbell(addr, data);
    return HAL_RET_OK;
}


hal_ret_t
cpupkt_send(cpupkt_ctxt_t* ctxt,
            types::WRingType type,
            uint32_t queue_id,
            cpu_to_p4plus_header_t* cpu_header,
            p4plus_to_p4_header_t* p4_header,
            uint8_t* data, 
            size_t data_len,
            uint16_t dest_lif,
            uint8_t  qtype,
            uint32_t qid,
            uint8_t ring_number)
{
    hal_ret_t           ret = HAL_RET_OK;
    cpupkt_hw_id_t      page_addr = 0;
    cpupkt_hw_id_t      descr_addr = 0;
    cpupkt_hw_id_t      write_addr = 0;
    size_t              write_len = 0;
    size_t              total_len = 0;

    if(!ctxt || !data) {
        return HAL_RET_INVALID_ARG;    
    }
    
    // Allocate a page and descr for the pkt
    ret = cpupkt_page_alloc(&page_addr);
    if(ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Failed to allocate page for the packet, err: {}", ret);
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

        write_len = sizeof(cpu_to_p4plus_header_t);
        total_len += write_len;
        HAL_TRACE_DEBUG("Copying CPU header of len: {} to addr: {:#x}, l2offset: {:#x}",
                        write_len, write_addr, cpu_header->l2_offset);
        if(!p4plus_hbm_write(write_addr, (uint8_t *)cpu_header, write_len)) {
            HAL_TRACE_ERR("Failed to copy packet to the page");
            ret = HAL_RET_HW_FAIL;
            goto cleanup;
        }

        // P4plus_to_p4_header_t
        write_addr += write_len; // shift address
        write_len = sizeof(p4plus_to_p4_header_t);
        total_len += write_len;
        HAL_TRACE_DEBUG("Copying P4Plus to P4 header of len: {} to addr: {:#x}",
                        write_len, write_addr);
        if(!p4plus_hbm_write(write_addr, (uint8_t *)p4_header, write_len)) {
            HAL_TRACE_ERR("Failed to copy packet to the page");
            ret = HAL_RET_HW_FAIL;
            goto cleanup;
        }
    
        write_addr += write_len;
    }
    // Data
    write_len = data_len;
    total_len += write_len;
    HAL_TRACE_DEBUG("Copying data of len: {} to page at addr: {:#x}", write_len, write_addr);
    if(!p4plus_hbm_write(write_addr, data, write_len)) {
        HAL_TRACE_ERR("Failed to copy packet to the page");
        ret = HAL_RET_HW_FAIL;
        goto cleanup;
    }

    HAL_TRACE_DEBUG("Total pkt len: {} to page at addr: {:#x}", total_len, page_addr);

    ret = cpupkt_program_descr(page_addr, total_len, &descr_addr);
    if(ret != HAL_RET_OK) {
        goto cleanup;    
    }
    
    // Program Queue
    ret = cpupkt_program_send_queue(ctxt, type, queue_id, descr_addr);
    if(ret != HAL_RET_OK) {
        goto cleanup;    
    }

    // Ring doorbell
    ret = cpupkt_program_send_ring_doorbell(dest_lif, qtype, qid, ring_number);
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
