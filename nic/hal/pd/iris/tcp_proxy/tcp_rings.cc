#include "nic/sdk/asic/rw/asicrw.hpp"
#include "nic/hal/pd/libs/wring/wring_pd.hpp"
#include "nic/hal/pd/iris/tcp_proxy/tcp_rings.hpp"
#include "nic/include/pd_api.hpp"
#include "nic/hal/pd/iris/hal_state_pd.hpp"
#include "asic/cmn/asic_common.hpp"

namespace hal {
namespace pd {

hal_ret_t
cpupkt_descr_free (cpupkt_hw_id_t descr_addr);

hal_ret_t
cpupkt_descr_to_headers (pd_descr_aol_t *descr,
                         p4_to_p4plus_cpu_pkt_t** flow_miss_hdr,
                         uint8_t** data, size_t* data_len, bool *copied_pkt, bool no_copy);
bool cpu_do_zero_copy = true;
/*
 * Check if zero-copy packet processing is enabled for PMD on this platform.
 */
static inline bool is_cpu_zero_copy_enabled (void)
{
    static bool platform_check_done = false;
    static bool is_hw = false;

    if (!platform_check_done) {
        is_hw = (is_platform_type_haps() || is_platform_type_hw());
        platform_check_done = true;
    }
    return(is_hw && cpu_do_zero_copy);
}

static inline tcp_rings_ctxt_t*
tcp_rings_ctxt_alloc(void)
{
    return (tcp_rings_ctxt_t *)g_hal_state_pd->tcp_rings_slab()->alloc();
}

hal_ret_t
tcp_ring_upd_sem_idx(tcp_ring_info_t *ring_info, bool init_pindex)
{
    uint64_t pi_sem_addr = 0;
    uint64_t ci_sem_addr = 0;
    static thread_local int sem_batch = 0;


    switch(ring_info->type) {
        case types::WRING_TYPE_TCP_ACTL_Q:
            ci_sem_addr = ASIC_SEM_TCP_ACTL_Q_CI_RAW_ADDR(ring_info->queue_id);
            break;
        default:
            HAL_ABORT_TRACE(0,
                  "Invalid TCP ring\n",
                  ring_info->queue_id);
        return HAL_RET_OK;
    }

    /*
     * Update TCP ACTL Q semaphore CI. We do this in a batch to
     * reduce CSR-write overheads.
     */
    if (!(sem_batch++ % TCP_RING_SEM_CI_BATCH_SIZE)) {
        HAL_TRACE_DEBUG("updating CI: type: {}, addr {:#x}, ci: {}",
            ring_info->type, ci_sem_addr,
            ring_info->pc_idx);

        if (asic_reg_write(ci_sem_addr, &ring_info->pc_idx, 1,
              ASIC_WRITE_MODE_WRITE_THRU) != sdk::SDK_RET_OK) {
            HAL_TRACE_ERR("Failed to program CI semaphore");
            return HAL_RET_HW_FAIL;
        }
    }

    /*
     * Initialize the semaphore to reset PI to zero.
     */
    if (init_pindex) {
        uint32_t value = 0;
        pi_sem_addr = ci_sem_addr - 4;
            HAL_TRACE_DEBUG("TCP ACTL updating PI: type: {}, addr {:#x}, pi: {}",
            ring_info->type, pi_sem_addr,
            value);
        if (asic_reg_write(pi_sem_addr, &value, 1,
            ASIC_WRITE_MODE_WRITE_THRU) != sdk::SDK_RET_OK) {
            HAL_TRACE_ERR("Failed to program PI semaphore");
            return HAL_RET_HW_FAIL;
        }
    }

    return HAL_RET_OK;
}


hal_ret_t
pd_tcp_rings_ctxt_init (pd_func_args_t *pd_func_args)
{
    pd_tcp_rings_ctxt_init_args_t *args = pd_func_args->pd_tcp_rings_ctxt_init;
    tcp_rings_ctxt_t *ctxt = tcp_rings_ctxt_alloc();

    if (ctxt == NULL) {
        HAL_TRACE_ERR("Failed to alloc tcp ring ctxt");
        return HAL_RET_NO_RESOURCE;
    }
    memset(ctxt, 0, sizeof(tcp_rings_ctxt_t));

    args->ctxt = ctxt;
    return HAL_RET_OK;
}

/*
 * Note: This function assumes pc_idx is being incremented by one only
 * before updating slot address. This simplifies the function
 */
static inline void
tcp_ring_update_slot_addr(tcp_ring_info_t* ring)
{
    uint32_t slot_index;
    pd_wring_meta_t *wring_meta = ring->wring_meta;

    slot_index = ring->pc_idx % wring_meta->num_slots;
    if (slot_index != 0) {
        ring->pc_idx_addr += wring_meta->slot_size_in_bytes;
        ring->virt_pc_idx_addr += wring_meta->slot_size_in_bytes;
    } else {
        ring->pc_idx_addr = ring->ring_base;
        ring->virt_pc_idx_addr = ring->virt_ring_base;
        /*
         * Ideally, for the first sweep across the ring calid bit should have been 1
         * since the ring is initialized to all zero.
         * However, P4+ computes valid bit from its prod_idx which is initialized
         * to zero. Ideally this should be fixed
         */
#if 1
        ring->valid_bit_val ^= TCP_ACTL_Q_MSG_VALID_BIT_MASK;
#else
        ring->valid_bit_val =
            (ring->pc_idx - ring->wring_meta->num_slots) &
              (0x1 << ring->ring_size_shift);
        ring->valid_bit_val =
            ring->valid_bit_val << (63 - ring->ring_size_shift);
#endif
    }

    /*
     * Update the virtual PC-index addr for direct access into the memory.
     */

    HAL_TRACE_DEBUG("updated pc_index queue: type: {} id: {}, index: {} virt-idx-addr: {:#x} addr: {:#x}: valid_bit: {:#x}",
                    ring->type, ring->queue_id,
                    ring->pc_idx, (uint64_t)ring->virt_pc_idx_addr,
                    ring->pc_idx_addr, ring->valid_bit_val);
    return;
}

hal_ret_t
pd_tcp_rings_register(pd_func_args_t *pd_func_args)
{
    pd_tcp_rings_register_args_t *args = pd_func_args->pd_tcp_rings_register;
    uint32_t queue_id = args->queue_id;
    tcp_rings_ctxt_t *ctxt = (tcp_rings_ctxt_t *)args->ctxt;
    types::WRingType type = args->type;
    pd_wring_meta_t *wring_meta;
    tcp_ring_info_t *ring_info;
    hal_ret_t ret;

    if (ctxt == NULL) {
        return HAL_RET_INVALID_ARG;
    }

    // Max one ring instance per type of ring
    if (ctxt->num_queues >= MAX_TCP_RING_TYPES) {
        return HAL_RET_NO_RESOURCE;
    }

    HAL_ABORT_TRACE(queue_id >= 0 && queue_id < 3,
          "TCP ring queue id {} out of bound", queue_id);

    // TODO: Check if the ring type is already registered

    // Initialize wring meta data fields
    ret = wring_pd_table_init(type, queue_id);
    if(ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Failed to initialize queue: {}, id: {}, ret: {}",
                    type, queue_id, ret);
        return ret;
    }

    wring_meta = wring_pd_get_meta(type);

    ring_info = &ctxt->ring_info[ctxt->num_queues];
    ring_info->ring_base = wring_meta->base_addr[queue_id];
    ring_info->ring_size_shift = log2(wring_meta->num_slots);
    ring_info->queue_id = queue_id;
    ring_info->type = type;
    ring_info->wring_meta = wring_meta;

    if (is_cpu_zero_copy_enabled()) {

        ring_info->virt_ring_base = wring_meta->virt_base_addr[queue_id];
        if (!ring_info->virt_ring_base) {
            HAL_TRACE_ERR("No virtual ring base addr. TCP wring type {} queue {}", type, queue_id);
            return HAL_RET_NO_RESOURCE;
        } else {
            HAL_TRACE_DEBUG("mmap the TCP ring T:{}. Q:{} phy{:#x} virt {:#x}",
                type, queue_id, (uint64_t)ring_info->ring_base,
                (uint64_t)ring_info->virt_ring_base);
        }
    }

    ring_info->pc_idx = wring_meta->num_slots;

    /*
     * Following is a hack for ensure valid_bit_val gets initialized to
     * zero and not 1. Ideally it should have been 1.
     * since the ring is initialized to all zero.
     * However, P4+ computes valid bit from its prod_idx which is initialized
     * to zero. Ideally this should be fixed.
     */
    ring_info->valid_bit_val = TCP_ACTL_Q_MSG_VALID_BIT_MASK;

    tcp_ring_upd_sem_idx(ring_info, true);
    tcp_ring_update_slot_addr(ring_info);

    ctxt->num_queues++;

    return HAL_RET_OK;
}

static inline bool
is_valid_msg_elem(tcp_ring_info_t* ring, uint64_t msg)
{
    if ((msg > 0) && (msg & TCP_ACTL_Q_MSG_VALID_BIT_MASK) == ring->valid_bit_val) {
        return true;
    }
    return false;
}

static inline void
tcp_ring_inc_queue_idx(tcp_ring_info_t& ring)
{
    ring.pc_idx++;
    tcp_ring_update_slot_addr(&ring);
    HAL_TRACE_DEBUG("incremented  pc_index queue: {} to index: {} addr: {:#x}",
                        ring.queue_id,  ring.pc_idx, ring.pc_idx_addr);
}

static thread_local uint64_t rxq_descr_addrs[TCP_MSGS_MAX_BATCH_SIZE];
static thread_local pd_descr_aol_t *rxq_descr_virt_ptrs[TCP_MSGS_MAX_BATCH_SIZE];
static thread_local pd_descr_aol_t rxq_descr_copy[TCP_MSGS_MAX_BATCH_SIZE]; // copy for non-HW platforms

hal_ret_t
pd_tcp_rings_poll(pd_func_args_t *pd_func_args)
{
    pd_tcp_rings_poll_args_t *args = pd_func_args->pd_tcp_rings_poll;
    tcp_msg_batch_t *batch = args->msg_batch;
    tcp_rings_ctxt_t *ctxt = (tcp_rings_ctxt_t *)args->ctxt;
    tcp_ring_info_t *ring;
    uint64_t msg_data, msg_cnt = 0, rx_pkt_cnt = 0;
    pd_descr_aol_t *descr_p = NULL;
    uint64_t npkt;

    if (!ctxt->num_queues) return(HAL_RET_RETRY);

    ring = &ctxt->ring_info[0];

    /*
     * For supporting for more than one rings, we will have to loop through the rings and
     * process them based on their types. Currently supporting only one type: ACTL
     */
    HAL_ABORT_TRACE((ctxt->num_queues == 1 && ring->type == types::WRING_TYPE_TCP_ACTL_Q),
          "Currently supporting only one tcp ring type: ACTL "
          "num_queues {} type {}",
          ctxt->num_queues, ring->type);

    do {
        if (is_cpu_zero_copy_enabled()) {
            msg_data = *(uint64_t*)ring->virt_pc_idx_addr;
        } else {
            if (sdk::asic::asic_mem_read(ring->pc_idx_addr,
                  (uint8_t *)&msg_data,
                  sizeof(uint64_t), true) != sdk::SDK_RET_OK) {
                      HAL_TRACE_ERR("Failed to read the slot msg from the hw");
                return HAL_RET_HW_FAIL;
            }
        }

        msg_data = ntohll(msg_data);
        if (!is_valid_msg_elem(ring, msg_data)) {
            break;
        }

        // Mask off valid bit
        msg_data &= ~TCP_ACTL_Q_MSG_VALID_BIT_MASK;
        HAL_TRACE_DEBUG("desc: {:#x}", msg_data);

        tcp_ring_inc_queue_idx(*ring);

        if (tcp_actl_get_msg_type(msg_data) == TCP_ACTL_MSG_TYPE_PKT) {
            uint64_t descr_addr = tcp_actl_get_pkt_desc(msg_data);
            if (sdk::asic::asic_mem_read(descr_addr, (uint8_t *)&rxq_descr_copy[rx_pkt_cnt],
                  sizeof(pd_descr_aol_t), true) != sdk::SDK_RET_OK) {
                HAL_TRACE_ERR("Failed to read the descr from hw");
                return HAL_RET_HW_FAIL;
            }

            descr_p = &rxq_descr_copy[rx_pkt_cnt];
            rxq_descr_addrs[rx_pkt_cnt] = descr_addr;
            rxq_descr_virt_ptrs[rx_pkt_cnt] = descr_p;
            rx_pkt_cnt++;
        } else {
            HAL_ABORT_TRACE(0,
                  "Only Pkt type TCP ACTL MSG expected. {} ?",
                  ring->queue_id);
        }
        msg_cnt++;
    } while (msg_cnt < TCP_MSGS_MAX_BATCH_SIZE);

    if (msg_cnt) {
        tcp_ring_upd_sem_idx(ring, false);
    } else {
        return(HAL_RET_RETRY);
    }

    HAL_TRACE_DEBUG("Process batch: total-msgs {} rx-pkts {}",
        msg_cnt, rx_pkt_cnt);

    for (npkt = 0; npkt < rx_pkt_cnt; npkt++) {
        hal_ret_t ret;
        batch->msg_info[npkt].msg_type = TCP_ACTL_MSG_TYPE_PKT;
       /*
        * Lets get the packet header/data from the descriptor we've received.
        */
        ret = cpupkt_descr_to_headers(rxq_descr_virt_ptrs[npkt], &(batch->msg_info[npkt].u.pkt.cpu_rxhdr),
                      &(batch->msg_info[npkt].u.pkt.pkt), &(batch->msg_info[npkt].u.pkt.pkt_len),
                      &(batch->msg_info[npkt].u.pkt.copied_pkt), 0);
        if(ret != HAL_RET_OK) {
            HAL_TRACE_ERR("Failed to convert descr to headers");
            break;
        } else {
            p4_to_p4plus_cpu_pkt_t *cpu_rxhdr = batch->msg_info[npkt].u.pkt.cpu_rxhdr;
            HAL_TRACE_DEBUG("Parsed CPU PKT: lif {} qid {} qtype {} t_flags {:#x}",
               cpu_rxhdr->lif, cpu_rxhdr->qid, cpu_rxhdr->qtype, cpu_rxhdr->tcp_flags);

           /*
            * Free the descriptor to GC.
            * We have made the copy
            */
            ret = cpupkt_descr_free(rxq_descr_addrs[npkt]);
            if (ret != HAL_RET_OK) {
                HAL_TRACE_ERR("Failed to free rx descr");
            }
        }
    }
    batch->msg_cnt = npkt;

    return HAL_RET_OK;
}

}
}
