#include <wring_pd.hpp>
#include <cpupkt_api.hpp>
#include <p4plus_pd_api.h>
#include <pd_api.hpp>
#include <hal_state_pd.hpp>

namespace hal {
namespace pd {

static inline cpupkt_rx_ctxt_t *
cpupkt_rx_ctxt_alloc(void) 
{
    return (cpupkt_rx_ctxt_t *)g_hal_state_pd->cpupkt_rx_slab()->alloc();    
}

bool is_cpu_queue(types::WRingType type)
{
    return (type == types::WRING_TYPE_ARQRX);
}

void
cpupkt_update_slot_addr(cpupkt_queue_info_t* queue_info)
{
    if(!queue_info || !queue_info->wring_meta) {
        HAL_TRACE_ERR("Invalid queue info");
        return;
    }

    uint32_t slot_index = queue_info->cindex % queue_info->wring_meta->num_slots;
    cpupkt_hw_id_t hw_id = queue_info->base_addr + 
            (slot_index * queue_info->wring_meta->slot_size_in_bytes);
    queue_info->cindex_addr = hw_id;
    HAL_TRACE_DEBUG("Updated cindex addr: {:#x}", hw_id);
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
                       uint64_t* data_offset, 
                       uint32_t* data_len)
{
    if(!flow_miss_hdr || !data_offset ||!data_len) {
        return HAL_RET_INVALID_ARG;    
    }

    HAL_TRACE_DEBUG("Descriptor0: a: {:#x}, o: {}, l: {}", 
                     descr.a0, descr.o0, descr.l0);
    HAL_TRACE_DEBUG("Descriptor1: a: {:#x}, o: {}, l: {}", 
                     descr.a1, descr.o1, descr.l1);
    HAL_TRACE_DEBUG("Descriptor2: a: {:#x}, o: {}, l: {}", 
                     descr.a2, descr.o2, descr.l2);
 
    *flow_miss_hdr = (p4_to_p4plus_cpu_pkt_t*)(descr.a0 + descr.o0);
    *data_offset = (descr.a0 + descr.o0 + sizeof(p4_to_p4plus_cpu_pkt_t));
    *data_len = descr.l0 - sizeof(p4_to_p4plus_cpu_pkt_t);
    uint64_t flow_miss_hdr_offset = (uint64_t)*flow_miss_hdr;
    HAL_TRACE_DEBUG("flow_miss_hdr {:#x}, data: {:#x}, len: {}",
                        flow_miss_hdr_offset, *data_offset, *data_len);
    return HAL_RET_OK;
}

hal_ret_t 
cpupkt_free_queue_index(cpupkt_queue_info_t& queue_info)
{
    // Set ASQ back to 0
    uint64_t value = 0;
    if(!p4plus_hbm_write(queue_info.cindex_addr,
                         (uint8_t *)&value,
                         sizeof(uint64_t))) {
        HAL_TRACE_ERR("Failed to reset cindex_addr");
        return HAL_RET_HW_FAIL;  
    }
    return HAL_RET_OK;
}

/****************************************************************
 * Packet send/receive APIs
 ***************************************************************/
 
cpupkt_rx_ctxt_t* 
cpupkt_rx_ctxt_alloc_init(void)
{
    return cpupkt_rx_ctxt_init(cpupkt_rx_ctxt_alloc());    
}

hal_ret_t 
cpupkt_register_rx_queue(cpupkt_rx_ctxt_t* ctxt, types::WRingType type)
{
    hal_ret_t   ret = HAL_RET_OK;
    
    if(!ctxt) {
        HAL_TRACE_ERR("Ctxt is null");
        return HAL_RET_INVALID_ARG;    
    }
    
    if(!is_cpu_queue(type)) {
        HAL_TRACE_ERR("Queue is not a valid cpu queue: {}", type);
        return HAL_RET_INVALID_ARG;    
    }

    // Verify if the queeue is already registered
    for(uint32_t i = 0; i< ctxt->num_queues; i++) {
        if(ctxt->queue[i].type == type) {
            HAL_TRACE_DEBUG("Queue is already added: {}", type);
            return HAL_RET_OK;
        }
    }
    
    if(ctxt->num_queues == MAX_CPU_PKT_QUEUES) {
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

    int index = ctxt->num_queues;
    ctxt->queue[index].type = type;
    ctxt->queue[index].cindex = 0;
    ctxt->queue[index].base_addr = base_addr;
    ctxt->queue[index].wring_meta = meta;
    cpupkt_update_slot_addr(&ctxt->queue[index]);
    HAL_TRACE_DEBUG("cindex addr: {:#x}", ctxt->queue[index].cindex_addr);
    ctxt->num_queues = index + 1;
    return HAL_RET_OK;
}

hal_ret_t 
cpupkt_poll_receive(cpupkt_rx_ctxt_t* ctxt,
                    p4_to_p4plus_cpu_pkt_t** flow_miss_hdr,
                    uint64_t* data_offset, 
                    uint32_t* data_len)
{
    hal_ret_t   ret = HAL_RET_OK;
    if(!ctxt) {
        return HAL_RET_INVALID_ARG;    
    }
    HAL_TRACE_DEBUG("Starting packet poll for queue: {}", ctxt->num_queues);

    while(true) {
        sleep(10);
        for(uint32_t i=0; i< ctxt->num_queues; i++) {
            uint64_t value = 0;
            HAL_TRACE_DEBUG("Checking queue: {} at address: {:#x}", i, ctxt->queue[i].cindex_addr);
            if(!p4plus_hbm_read(ctxt->queue[i].cindex_addr,
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
            value = value + 64;
            
            HAL_TRACE_DEBUG("Received valid data: queue: {}, cindex: {}, addr: {:#x}, value: {:#x}", 
                                ctxt->queue[i].type, ctxt->queue[i].cindex, ctxt->queue[i].cindex_addr, value);
            // get the descriptor
            pd_descr_aol_t  descr = {0};
           if(!p4plus_hbm_read(value, (uint8_t*)&descr, sizeof(pd_descr_aol_t))) {
                HAL_TRACE_ERR("Failed to read the descr from hw");
                return HAL_RET_HW_FAIL;
            }
            
            cpupkt_descr_to_skbuff(descr, flow_miss_hdr, data_offset, data_len);
            cpupkt_free_queue_index(ctxt->queue[i]);
        }
    }
    return ret;    
}

void* cpupkt_test_pkt_receive(void * thread_context) 
{
    p4_to_p4plus_cpu_pkt_t* flow_miss_hdr_offset = NULL;
    uint64_t data_offset = 0;
    uint32_t data_len = 0;

    HAL_TRACE_DEBUG("CPU Thread {} initializing ... ");
    cpupkt_rx_ctxt_t* ctxt = cpupkt_rx_ctxt_alloc_init();

    cpupkt_register_rx_queue(ctxt, types::WRING_TYPE_ARQRX);

    cpupkt_poll_receive(ctxt, 
                        &flow_miss_hdr_offset,
                        &data_offset,
                        &data_len);

    return NULL;
}


} // namespace pd
} // namespace hal
