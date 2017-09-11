#include <wring_pd.hpp>
#include <cpupkt_api.hpp>
#include <p4plus_pd_api.h>
#include <pd_api.hpp>
#include <hal_state_pd.hpp>
#include <capri_hbm.hpp>
#include <proxy.hpp>

namespace hal {
namespace pd {

static inline cpupkt_ctxt_t *
cpupkt_ctxt_alloc(void) 
{
    return (cpupkt_ctxt_t *)g_hal_state_pd->cpupkt_slab()->alloc();    
}

bool is_cpu_rx_queue(types::WRingType type)
{
    return (type == types::WRING_TYPE_ARQRX);
}

bool is_cpu_tx_queue(types::WRingType type)
{
    return (type == types::WRING_TYPE_ASQ);
}


void
cpupkt_update_slot_addr(cpupkt_queue_info_t* queue_info)
{
    if(!queue_info || !queue_info->wring_meta) {
        HAL_TRACE_ERR("Invalid queue info");
        return;
    }

    uint32_t slot_index = queue_info->pc_index % queue_info->wring_meta->num_slots;
    cpupkt_hw_id_t hw_id = queue_info->base_addr + 
            (slot_index * queue_info->wring_meta->slot_size_in_bytes);
    queue_info->pc_index_addr = hw_id;
    HAL_TRACE_DEBUG("Updated pc_index addr: {:#x}", hw_id);
    return;
}

bool
is_valid_slot_value(uint64_t slot_value) 
{
    return (slot_value > 0);
}

hal_ret_t
cpupkt_descr_to_skbuff(pd_descr_aol_t& descr,
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

hal_ret_t 
cpupkt_free_queue_index(cpupkt_queue_info_t& queue_info)
{
    // Set ASQ back to 0
    uint64_t value = 0;
    if(!p4plus_hbm_write(queue_info.pc_index_addr,
                         (uint8_t *)&value,
                         sizeof(uint64_t))) {
        HAL_TRACE_ERR("Failed to reset pc_index_addr");
        return HAL_RET_HW_FAIL;  
    }
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
cpupkt_register_tx_queue(cpupkt_ctxt_t* ctxt, types::WRingType type)
{
    hal_ret_t   ret = HAL_RET_OK;
    
    if(!ctxt) {
        HAL_TRACE_ERR("Ctxt is null");
        return HAL_RET_INVALID_ARG;    
    }
    
    if(!is_cpu_tx_queue(type)) {
        HAL_TRACE_ERR("Queue is not a valid cpu queue: {}", type);
        return HAL_RET_INVALID_ARG;    
    }

    // Verify if the queeue is already registered
    if(ctxt->tx.queue.type == type) {
        HAL_TRACE_DEBUG("Queue is already added: {}", type);
        return HAL_RET_OK;
    }
    
    wring_hw_id_t base_addr = 0;
    ret = wring_pd_get_base_addr(type, 0, &base_addr);
    if(ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Failed to get base addr for queue: {}: ret: {}", type, ret); 
        return ret;
    }
  
    pd_wring_meta_t* meta = wring_pd_get_meta(type);

    ctxt->tx.queue.type = type;
    ctxt->tx.queue.pc_index = 0;
    ctxt->tx.queue.base_addr = base_addr;
    ctxt->tx.queue.wring_meta = meta;
    cpupkt_update_slot_addr(&ctxt->tx.queue);
    HAL_TRACE_DEBUG("pindex addr: {:#x}", ctxt->tx.queue.pc_index_addr);
    return HAL_RET_OK;
}


hal_ret_t 
cpupkt_register_rx_queue(cpupkt_ctxt_t* ctxt, types::WRingType type)
{
    hal_ret_t   ret = HAL_RET_OK;
    
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
 
    wring_hw_id_t base_addr = 0;
    ret = wring_pd_get_base_addr(type, 0, &base_addr);
    if(ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Failed to get base addr for queue: {}: ret: {}", type, ret); 
        return ret;
    }
  
    pd_wring_meta_t* meta = wring_pd_get_meta(type);

    int index = ctxt->rx.num_queues;
    ctxt->rx.queue[index].type = type;
    ctxt->rx.queue[index].pc_index = 0;
    ctxt->rx.queue[index].base_addr = base_addr;
    ctxt->rx.queue[index].wring_meta = meta;
    cpupkt_update_slot_addr(&ctxt->rx.queue[index]);
    HAL_TRACE_DEBUG("pc_index addr: {:#x}", ctxt->rx.queue[index].pc_index_addr);
    ctxt->rx.num_queues = index + 1;
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
    HAL_TRACE_DEBUG("Starting packet poll for queue: {}", ctxt->rx.num_queues);

    while(true) {
        sleep(10);
        for(uint32_t i=0; i< ctxt->rx.num_queues; i++) {
            uint64_t value = 0;
            HAL_TRACE_DEBUG("Checking queue: {} at address: {:#x}", i, ctxt->rx.queue[i].pc_index_addr);
            if(!p4plus_hbm_read(ctxt->rx.queue[i].pc_index_addr,
                                (uint8_t *)&value, 
                                sizeof(uint64_t))) {
                HAL_TRACE_ERR("Failed to read the data from the hw");
                return HAL_RET_HW_FAIL;
            }
            value = ntohll(value);
            if(!is_valid_slot_value(value)) {
                continue;
            }
             // offset to take care of scratch
            value = value + CPU_PKT_DESCR_OFFSET;
            
            HAL_TRACE_DEBUG("Received valid data: queue: {}, pc_index: {}, addr: {:#x}, value: {:#x}", 
                                ctxt->rx.queue[i].type, ctxt->rx.queue[i].pc_index, ctxt->rx.queue[i].pc_index_addr, value);
            // get the descriptor
            pd_descr_aol_t  descr = {0};
            if(!p4plus_hbm_read(value, (uint8_t*)&descr, sizeof(pd_descr_aol_t))) {
                HAL_TRACE_ERR("Failed to read the descr from hw");
                return HAL_RET_HW_FAIL;
            }
            
            ret = cpupkt_descr_to_skbuff(descr, flow_miss_hdr, data, data_len);
            if(ret != HAL_RET_OK) 
            {
                HAL_TRACE_ERR("Failed to create skbuff");
            }
            cpupkt_free_queue_index(ctxt->rx.queue[i]);
            return ret;
        }
    }
    return ret;    
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
    cpupkt_hw_id_t     addr = 0;
    pd_descr_aol_t  descr = {0};

    ret = cpupkt_descr_alloc(&addr);
    if(ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Failed to allocate descr for the packet, err: {}", ret);
        goto cleanup;
    }
    
    descr.a0 = page_addr;
    descr.o0 = 0;
    descr.l0 = len;

    *descr_addr = addr;
    // shift addr to take care of the scratch
    addr = addr + CPU_PKT_DESCR_OFFSET;
    HAL_TRACE_DEBUG("Programming descr: descr_addr: {:#x} addr: {:#x}", *descr_addr, addr);
    if(!p4plus_hbm_write(addr, (uint8_t*)&descr, sizeof(pd_descr_aol_t))) {
        HAL_TRACE_ERR("Failed to program descr");
        ret = HAL_RET_HW_FAIL;
        goto cleanup;
    }

    ret = HAL_RET_OK;

cleanup:
    return ret;
        
}

hal_ret_t
cpupkt_program_send_queue(cpupkt_ctxt_t* ctxt, cpupkt_hw_id_t descr_addr) 
{
    if(!ctxt) {
        return HAL_RET_INVALID_ARG;    
    }
    
    HAL_TRACE_DEBUG("Programming send queue: addr: {:#x} value: {:#x}",
                            ctxt->tx.queue.pc_index_addr, descr_addr);
    if(!p4plus_hbm_write(ctxt->tx.queue.pc_index_addr, (uint8_t*) &descr_addr, sizeof(cpupkt_hw_id_t))) {
        HAL_TRACE_ERR("Failed to program send queue");
        return HAL_RET_HW_FAIL;
    }
    
    return HAL_RET_OK;
}

hal_ret_t
cpupkt_program_send_ring_doorbell(cpupkt_ctxt_t* ctxt) 
{
    uint64_t            addr = 0;
    uint64_t            data = 0;

    if(!ctxt) {
        return HAL_RET_INVALID_ARG;    
    }
    
    addr = 0 | DB_IDX_UPD_PIDX_INC | DB_SCHED_UPD_SET;
    addr = addr << DB_UPD_SHFT;
    addr += (SERVICE_LIF_CPU << DB_LIF_SHFT);
    addr += DB_ADDR_BASE;

    data += ((uint64_t)CPU_ASQ_PID << DB_PID_SHFT);
    data += (CPU_ASQ_QID << DB_QID_SHFT);
    data += (CPU_SCHED_RING_ASQ << DB_RING_SHFT);

    HAL_TRACE_DEBUG("Ringing Doorbell with addr: {:#x} data: {:#x}",
                    addr, data);
    step_doorbell(addr, data);
    ctxt->tx.queue.pc_index++;
    HAL_TRACE_DEBUG("Ring doorbell done. Set pc_index: {}", ctxt->tx.queue.pc_index++);
    return HAL_RET_OK;
}

hal_ret_t
cpupkt_send(cpupkt_ctxt_t* ctxt,
            p4plus_to_p4_header_t* header,
            uint8_t* data, 
            size_t data_len) 
{
    hal_ret_t           ret = HAL_RET_OK;
    cpupkt_hw_id_t      page_addr = 0;
    cpupkt_hw_id_t      descr_addr = 0;
    cpupkt_hw_id_t      data_addr = 0;
    size_t              header_len = sizeof(p4plus_to_p4_header_t);
    size_t              total_len = header_len + data_len;

    if(!ctxt || !header || !data) {
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
    HAL_TRACE_DEBUG("Copying header of len: {} to page at addr: {:#x}", 
                          header_len, page_addr);
    if(!p4plus_hbm_write(page_addr, (uint8_t *)header, header_len)) {
        HAL_TRACE_ERR("Failed to copy packet to the page");
        ret = HAL_RET_HW_FAIL;
        goto cleanup;
    }
    
    data_addr = page_addr + header_len; 
    HAL_TRACE_DEBUG("Copying data of len: {} to page at addr: {:#x}", data_len, data_addr);
    if(!p4plus_hbm_write(data_addr, data, data_len)) {
        HAL_TRACE_ERR("Failed to copy packet to the page");
        ret = HAL_RET_HW_FAIL;
        goto cleanup;
    }

    total_len = header_len + data_len;
    HAL_TRACE_DEBUG("Total pkt len: {} to page at addr: {:#x}", total_len, data_addr);

    ret = cpupkt_program_descr(page_addr, total_len, &descr_addr);
    if(ret != HAL_RET_OK) {
        goto cleanup;    
    }
    
    // Program Queue
    ret = cpupkt_program_send_queue(ctxt, descr_addr); 
    if(ret != HAL_RET_OK) {
        goto cleanup;    
    }

    // Ring doorbell
    ret = cpupkt_program_send_ring_doorbell(ctxt); 
    if(ret != HAL_RET_OK) {
        goto cleanup;    
    }
    
    return HAL_RET_OK;
cleanup:
    // FIXME
    return ret;
}


/*****************************************************
 * Dummy Test API TODO: REMOVE
 * ***************************************************/
void* cpupkt_test_pkt_receive(void * thread_context) 
{
    p4_to_p4plus_cpu_pkt_t* flow_miss_hdr = NULL;
    uint8_t* data = NULL;
    size_t data_len = 0;
    p4plus_to_p4_header_t hdr = {};

    HAL_TRACE_DEBUG("CPU Thread {} initializing ... ");
    cpupkt_ctxt_t* ctxt = cpupkt_ctxt_alloc_init();

    cpupkt_register_rx_queue(ctxt, types::WRING_TYPE_ARQRX);
    cpupkt_register_tx_queue(ctxt, types::WRING_TYPE_ASQ);
    
    while(true) {
        cpupkt_poll_receive(ctxt, 
                            &flow_miss_hdr,
                            &data,
                            &data_len);
        HAL_TRACE_DEBUG("Received packet.....");
        cpupkt_send(ctxt, &hdr, data, data_len); 
        cpupkt_free(flow_miss_hdr, data);
    }

    return NULL;
}


} // namespace pd
} // namespace hal
