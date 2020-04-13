#include "nic/sdk/asic/rw/asicrw.hpp"
#include "nic/hal/pd/libs/wring/wring_pd.hpp"
#include "nic/hal/pd/cpupkt_api.hpp"
#include "nic/include/pd_api.hpp"
#include "nic/hal/pd/iris/hal_state_pd.hpp"
#include "nic/sdk/asic/cmn/asic_hbm.hpp"
#include "nic/asm/cpu-p4plus/include/cpu-defines.h"
#include "asic/cmn/asic_common.hpp"
#include "platform/pal/include/pal_mem.h"
#include "nic/sdk/lib/pal/pal.hpp"

namespace hal {
namespace pd {

//#define HAL_TRACE_DEBUG2(fmt, ...)
//#define HAL_TRACE_ERR2(fmt, ...)

#define HAL_TRACE_DEBUG2 HAL_TRACE_VERBOSE
#define HAL_TRACE_ERR2   HAL_TRACE_ERR

thread_local uint32_t gc_pindex = 0;
thread_local uint32_t cpu_tx_page_pindex = 0;
thread_local uint32_t cpu_tx_page_cindex = 0;
thread_local uint32_t cpu_tx_page_full_err = 0;
thread_local uint32_t cpu_tx_descr_pindex = 0;
thread_local uint32_t cpu_tx_descr_cindex = 0;
thread_local uint32_t cpu_tx_descr_full_err = 0;
thread_local uint32_t cpu_rx_dpr_cindex = 0;
thread_local uint32_t cpu_rx_dpr_sem_cindex = 0;
thread_local uint32_t cpu_rx_dpr_sem_free_err = 0;
thread_local uint32_t cpu_rx_dpr_descr_free_err = 0;
thread_local uint32_t cpu_rx_dpr_descr_invalid_free_err = 0;
thread_local int      tg_cpu_id = -1;
static bool cpu_do_zero_copy = true;

hal_ret_t cpupkt_descr_free(cpupkt_hw_id_t descr_addr);

#define CPU_PINDEX_MAX 65536

/****************************************************************
 * Helper functions
 ***************************************************************/
static inline cpupkt_ctxt_t*
cpupkt_ctxt_alloc (void)
{
    return (cpupkt_ctxt_t *)g_hal_state_pd->cpupkt_slab()->alloc();
}

static inline cpupkt_qinst_info_t*
cpupkt_ctxt_qinst_info_alloc (void)
{
    return (cpupkt_qinst_info_t*)g_hal_state_pd->cpupkt_qinst_info_slab()->alloc();
}

static inline void
cpupkt_ctxt_qinst_info_free (cpupkt_qinst_info_t* qinst_info)
{
    hal::pd::delay_delete_to_slab(HAL_SLAB_CPUPKT_QINST_INFO_PD, qinst_info);
}

static inline bool is_cpu_send_comp_queue (types::WRingType type)
{
    return(type == types::WRING_TYPE_ASCQ);
}

static inline bool is_cpu_rx_queue (types::WRingType type)
{
    return (type == types::WRING_TYPE_ARQRX || type == types::WRING_TYPE_ARQTX || is_cpu_send_comp_queue(type));
}

static inline bool is_cpu_tx_queue (types::WRingType type)
{
    return ((type == types::WRING_TYPE_ASQ) || (type == types::WRING_TYPE_ASESQ) ||
            (type == types::WRING_TYPE_APP_REDIR_RAWC) ||
            (type == types::WRING_TYPE_APP_REDIR_PROXYC));
}

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
    return (is_hw && cpu_do_zero_copy);
}

/*
 * Given a physical-address of the descriptor, check whether its an Rx descriptor (descriptor
 * allocated by P4 and received by us), using the address comparison with the regions configured
 * for the descriptor pools, and return the corresponding virtual address that we've mapped to
 * this process.
 */
static inline bool
is_phyaddr_cpu_rx_dpr_descr (cpupkt_hw_id_t descr_addr, int tg_cpu_id, pd_descr_aol_t **virt_addr)
{
    pd_wring_meta_t *meta = wring_pd_get_meta(types::WRING_TYPE_CPU_RX_DPR);

    /*
     * Check if the descriptor could be a "CPU-RX-DPR" descriptor  (an Rx-packet from
     * CPU-P4+ Rx program), based on the address-range of the descriptor-address, and
     * get the corresponding virtual-address of the descriptor which we can access
     * directly.
     */
    HAL_TRACE_DEBUG2("CPU-RX-DPR obj-base-addr {:#x} size {} KB",
            meta->obj_base_addr[0], (meta->num_slots * meta->obj_size)/1024);
    if (descr_addr >= meta->obj_base_addr[0] &&
    descr_addr <= (meta->obj_base_addr[0] + (meta->num_slots * meta->obj_size))) {

        if (is_cpu_zero_copy_enabled() && virt_addr) {
        *virt_addr = (pd_descr_aol_t *) ((uint64_t)meta->virt_obj_base_addr[0] +
                                             (uint64_t)(descr_addr - meta->obj_base_addr[0]));
    }
    return(true);
    }
    return(false);
}

/*
 * Given a physical-address of the descriptor, check whether its an Tx descriptor (descriptor
 * allocated by us to transmit originated packets), using the address comparison with the
 * regions configured for the descriptor pools, and return the corresponding virtual address
 * that we've mapped to this process.
 */
static inline bool
is_phyaddr_cpu_tx_dr_descr (cpupkt_hw_id_t descr_addr, int tg_cpu_id, pd_descr_aol_t **virt_addr)
{
    pd_wring_meta_t *meta = wring_pd_get_meta(types::WRING_TYPE_CPU_TX_DR);

    /*
     * Check if the descriptor could be a "CPU-TX-DR" descriptor (a Tx-packet
     * originated from ARM which is being sent back on completion-queue for free'ing),
     * based on the address-range of the descriptor-address, and get the corresponding
     * virtual-address of the descriptor which we can access directly.
     */
    HAL_TRACE_DEBUG2("CPU-TX-DR({}) obj-base-addr {:#x} size {} KB", tg_cpu_id,
            meta->obj_base_addr[tg_cpu_id], (meta->num_slots * meta->obj_size)/1024);

    if (descr_addr >= meta->obj_base_addr[tg_cpu_id] &&
    descr_addr <= (meta->obj_base_addr[tg_cpu_id] + (meta->num_slots * meta->obj_size))) {

        if (is_cpu_zero_copy_enabled()) {
        *virt_addr = (pd_descr_aol_t *) ((uint64_t)meta->virt_obj_base_addr[tg_cpu_id] +
                         (uint64_t)(descr_addr - meta->obj_base_addr[tg_cpu_id]));
    }
    return(true);
    }
    return(false);
}

/*
 * Given a virtual-address of the page, check whether its an CPU-RX-DPR page (descriptor+page
 * allocated by P4 and received by us), using the address comparison with the regions configured
 * for the descriptor pools, and return the corresponding physical address of that page.
 */
static inline bool
is_virtaddr_cpu_rx_dpr_page (cpupkt_hw_id_t page_addr, int tg_cpu_id, cpupkt_hw_id_t *phy_page_addr)
{
    pd_wring_meta_t *meta = wring_pd_get_meta(types::WRING_TYPE_CPU_RX_DPR);

    /*
     * Check if the page could be a "CPU-RX-DPR" page  (an Rx-packet from
     * CPU-P4+ Rx program), based on the address-range of the page-address, and
     * get the corresponding virtual-address of the page which we can access
     * directly.
     */
    HAL_TRACE_VERBOSE("CPU-RX-DPR virt-obj-base-addr {:p} size {} KB",
                      meta->virt_obj_base_addr[0],
                      (meta->num_slots * meta->obj_size)/1024);
    if (page_addr >= (uint64_t)meta->virt_obj_base_addr[0] &&
    page_addr <= ((uint64_t) meta->virt_obj_base_addr[0] + (meta->num_slots * meta->obj_size))) {

        *phy_page_addr = (cpupkt_hw_id_t) ((uint64_t)meta->obj_base_addr[0] +
                       (uint64_t)(page_addr - (uint64_t)meta->virt_obj_base_addr[0]));
    return(true);
    }
    return(false);
}

void
cpupkt_update_slot_addr (cpupkt_qinst_info_t* qinst_info)
{
    if (!qinst_info || !qinst_info->queue_info || !qinst_info->queue_info->wring_meta) {
        HAL_TRACE_ERR2("Invalid queue info");
        return;
    }

    uint32_t slot_index = qinst_info->pc_index % qinst_info->queue_info->wring_meta->num_slots;
    /* we initialize pc_index to num_slots. Subtract num_slots before calculating valid bit value for the PI */
    //uint32_t queue_tbl_shift = log2(qinst_info->queue_info->wring_meta->num_slots);
    qinst_info->valid_bit_value = (qinst_info->pc_index - qinst_info->queue_info->wring_meta->num_slots) \
        & (0x1 << qinst_info->queue_tbl_shift);
    qinst_info->valid_bit_value = qinst_info->valid_bit_value << (63 - qinst_info->queue_tbl_shift);

    cpupkt_hw_id_t hw_id = qinst_info->base_addr +
            (slot_index * qinst_info->queue_info->wring_meta->slot_size_in_bytes);
    qinst_info->pc_index_addr = hw_id;

    /*
     * Update the virtual PC-index addr for direct access into the memory.
     */
    if (qinst_info->virt_base_addr) {
        qinst_info->virt_pc_index_addr = qinst_info->virt_base_addr +
                (slot_index * qinst_info->queue_info->wring_meta->slot_size_in_bytes);
    }

    HAL_TRACE_DEBUG2("updated pc_index queue: type: {} id: {}, index: {} virt-idx-addr: {:#x} addr: {:#x}: valid_bit_value: {:#x}",
                    qinst_info->queue_info->type, qinst_info->queue_id,
                    qinst_info->pc_index, (uint64_t) qinst_info->virt_pc_index_addr, hw_id, qinst_info->valid_bit_value);
    return;
}

hal_ret_t
pd_cpupkt_get_slot_addr (types::WRingType type, uint32_t wring_id,
                         uint32_t index, cpupkt_hw_id_t *slot_addr, uint64_t **slot_virt_addr, uint32_t *num_slots_p)
{
    hal_ret_t      ret = HAL_RET_OK;
    cpupkt_hw_id_t base_addr = 0;
    pd_wring_meta_t *wring_meta = wring_pd_get_meta(type);
    uint8_t *virt_base_addr;

    if(!wring_meta) {
        HAL_TRACE_ERR2("Failed to get wring meta for type: {}", type);
        return HAL_RET_WRING_NOT_FOUND;
    }

    if (num_slots_p) *num_slots_p = wring_meta->num_slots;

    ret = wring_pd_get_base_addr(type, wring_id, &base_addr);
    if(ret != HAL_RET_OK) {
        HAL_TRACE_ERR2("failed to get base address for the wring: {}", type);
        return HAL_RET_ERR;
    }

    *slot_addr = base_addr + ((index % wring_meta->num_slots) * wring_meta->slot_size_in_bytes);

    /*
     * Get the virtual-address of the slot from the memory region we've mapped to our address-space.
     */
    virt_base_addr = wring_pd_get_meta_virt_base_addr(wring_meta, wring_id);
    if (virt_base_addr) {
        virt_base_addr += (index % wring_meta->num_slots) * wring_meta->slot_size_in_bytes;
    }

    *slot_virt_addr = (uint64_t *)virt_base_addr;
    return HAL_RET_OK;
}

static inline hal_ret_t
pd_cpupkt_get_slot_addr_for_ring (pd_wring_meta_t *wring_meta, cpupkt_hw_id_t base_addr, uint32_t wring_id,
                  uint32_t index, cpupkt_hw_id_t *slot_addr, uint64_t **slot_virt_addr)
{
    uint8_t *virt_base_addr;

    *slot_addr = base_addr + ((index % wring_meta->num_slots) * wring_meta->slot_size_in_bytes);

    /*
     * Get the virtual-address of the slot from the memory region we've mapped to our address-space.
     */
    virt_base_addr = wring_pd_get_meta_virt_base_addr(wring_meta, wring_id);
    if (virt_base_addr) {
        virt_base_addr += (index % wring_meta->num_slots) * wring_meta->slot_size_in_bytes;
    }
    *slot_virt_addr = (uint64_t *)virt_base_addr;
    return HAL_RET_OK;
}


/*
 * Free the descriptor back to the CPU-RX-DPR pool.
 */
hal_ret_t
pd_cpupkt_free_rx_descrs (uint64_t *descr_addrs, pd_descr_aol_t **virt_addrs,
              uint16_t pkt_count)
{
    hal_ret_t         ret = HAL_RET_OK;
    cpupkt_hw_id_t    slot_addr = 0;
    uint64_t          value = 0, *slot_virt_addr = NULL;
    static thread_local uint64_t dpr_ring_base_addr = 0;
    static thread_local pd_wring_meta_t *dpr_wring_meta = NULL;

    if (dpr_ring_base_addr == 0) {
        dpr_wring_meta = wring_pd_get_meta(types::WRING_TYPE_CPU_RX_DPR);
    if(!dpr_wring_meta) {
        HAL_TRACE_ERR2("Failed to get wring meta for type: {}", types::WRING_TYPE_CPU_RX_DPR);
            cpu_rx_dpr_descr_invalid_free_err++;
        return HAL_RET_WRING_NOT_FOUND;
    }

    ret = wring_pd_get_base_addr(types::WRING_TYPE_CPU_RX_DPR, 0, &dpr_ring_base_addr);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR2("failed to get base address for the wring: {}", types::WRING_TYPE_CPU_RX_DPR);
            cpu_rx_dpr_descr_invalid_free_err++;
        return HAL_RET_ERR;
    }
    }

    for (uint16_t npkt = 0; npkt < pkt_count; npkt++) {

        // Free page
        ret = pd_cpupkt_get_slot_addr_for_ring(dpr_wring_meta, dpr_ring_base_addr, 0,
                           cpu_rx_dpr_cindex, &slot_addr, &slot_virt_addr);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR2("Failed to get to slot addr for cpu-rx-dpr ci: {}",
              cpu_rx_dpr_cindex);
            cpu_rx_dpr_descr_invalid_free_err++;
        return ret;
    }

    value = htonll(descr_addrs[npkt]);

    /*
     * Directly free to the slot virtual-address if zero-copy is enabled.
     */
    if (is_cpu_zero_copy_enabled() && slot_virt_addr) {
        *slot_virt_addr = value;

        HAL_TRACE_DEBUG2("cpu-id: {} update cpudr: slot: {:#x} virt-slot: {:#x} ci: {} descr: {:#x}, value: {:#x}",
                tg_cpu_id, slot_addr, *slot_virt_addr,
                cpu_rx_dpr_cindex, descr_addrs[npkt], value);
    } else {
        if (asic_mem_write(slot_addr, (uint8_t *)&value, sizeof(uint64_t),
                   ASIC_WRITE_MODE_WRITE_THRU) != sdk::SDK_RET_OK) {
            HAL_TRACE_ERR2("Failed to program descr to slot");
                cpu_rx_dpr_descr_invalid_free_err++;
        return HAL_RET_HW_FAIL;
        }
        HAL_TRACE_DEBUG2("cpu-id: {} update cpudr: slot: {:#x} ci: {} descr: {:#x}, value: {:#x}",
                tg_cpu_id, slot_addr, cpu_rx_dpr_cindex, descr_addrs[npkt], value);
    }
    cpu_rx_dpr_cindex++;
    cpu_rx_dpr_sem_cindex++;
    }

    /*
     * Also update the CI Semaphore for the CPU-RX-DPR ring to return
     * the descriptor/page. We'll do this in a batch, to reduce CSR-write
     * overheads.
     */
    cpupkt_hw_id_t ci_sem_addr = ASIC_SEM_CPU_RX_DPR_ALLOC_CI_RAW_ADDR;
    if (asic_reg_write(ci_sem_addr, &cpu_rx_dpr_sem_cindex, 1,
               ASIC_WRITE_MODE_WRITE_THRU) != sdk::SDK_RET_OK) {
            HAL_TRACE_ERR2("Failed to program CPU-RX-DPR CI semaphore to {}",
              cpu_rx_dpr_sem_cindex);
            cpu_rx_dpr_sem_free_err++;
        return HAL_RET_HW_FAIL;
    }

    HAL_TRACE_DEBUG2("Updated CPU-RX-DPR CI semaphore to {}", cpu_rx_dpr_sem_cindex);

    return HAL_RET_OK;
}

/*
 * Free the descriptor back to the CPU-RX-DPR pool.
 */
hal_ret_t
pd_cpupkt_free_rx_descr (cpupkt_hw_id_t descr_addr)
{
    hal_ret_t         ret = HAL_RET_OK;
    cpupkt_hw_id_t    slot_addr = 0;
    uint64_t          value = 0, *slot_virt_addr = NULL;
    static thread_local int sem_batch = 0;

    if(!descr_addr) {
        HAL_TRACE_ERR2("invalid arg");
        cpu_rx_dpr_descr_invalid_free_err++;
        return HAL_RET_INVALID_ARG;
    }

    // Free page
    ret = pd_cpupkt_get_slot_addr(types::WRING_TYPE_CPU_RX_DPR, 0,
                                  cpu_rx_dpr_cindex, &slot_addr, &slot_virt_addr, NULL);
    if(ret != HAL_RET_OK) {
        HAL_TRACE_ERR2("Failed to get to slot addr for cpu-rx-dpr ci: {}",
                      cpu_rx_dpr_cindex);
        cpu_rx_dpr_descr_invalid_free_err++;
        return ret;
    }

    value = htonll(descr_addr);

    /*
     * Directly free to the slot virtual-address if zero-copy is enabled.
     */
    if (is_cpu_zero_copy_enabled() && slot_virt_addr) {
        *slot_virt_addr = value;

    HAL_TRACE_DEBUG2("cpu-id: {} update cpudr: slot: {:#x} virt-slot: {:#x} ci: {} descr: {:#x}, value: {:#x}",
             tg_cpu_id, slot_addr, *slot_virt_addr,
             cpu_rx_dpr_cindex, descr_addr, value);

    } else {
        if (asic_mem_write(slot_addr, (uint8_t *)&value, sizeof(uint64_t),
               ASIC_WRITE_MODE_WRITE_THRU) != sdk::SDK_RET_OK) {
            HAL_TRACE_ERR2("Failed to program descr to slot");
            cpu_rx_dpr_descr_invalid_free_err++;
        return HAL_RET_HW_FAIL;
    }
    HAL_TRACE_DEBUG2("cpu-id: {} update cpudr: slot: {:#x} ci: {} descr: {:#x}, value: {:#x}",
             tg_cpu_id, slot_addr, cpu_rx_dpr_cindex, descr_addr, value);
    }
    cpu_rx_dpr_cindex++;
    cpu_rx_dpr_sem_cindex++;

    /*
     * Ensure the programming of the descriptor/senq-queue is completed with a
     * memory barrier, before ringing the txdma doorbell.
     */
    PAL_barrier();

    /*
     * Also update the CI Semaphore for the CPU-RX-DPR ring to return
     * the descriptor/page. We'll do this in a batch, to reduce CSR-write
     * overheads.
     */
    if (!(++sem_batch % CPU_PKT_SEM_CI_BATCH_SIZE)) {
        cpupkt_hw_id_t ci_sem_addr = ASIC_SEM_CPU_RX_DPR_ALLOC_CI_RAW_ADDR;
    if (asic_reg_write(ci_sem_addr, &cpu_rx_dpr_sem_cindex, 1,
               ASIC_WRITE_MODE_WRITE_THRU) != sdk::SDK_RET_OK) {
            HAL_TRACE_ERR2("Failed to program CPU-RX-DPR CI semaphore to {}",
              cpu_rx_dpr_sem_cindex);
            cpu_rx_dpr_sem_free_err++;
        return HAL_RET_HW_FAIL;
    }
    }

    HAL_TRACE_DEBUG2("Updated CPU-RX-DPR CI semaphore to {}", cpu_rx_dpr_sem_cindex);

    return HAL_RET_OK;
}

/*
 * Free the descriptor/page back to the CPU-TX-DR and CPU-TX-PR pools.
 */
hal_ret_t
pd_cpupkt_free_tx_descr (cpupkt_hw_id_t descr_addr, pd_descr_aol_t *descr)
{
    hal_ret_t         ret = HAL_RET_OK;
    cpupkt_hw_id_t    slot_addr = 0;
    uint64_t          value = 0, *slot_virt_addr = NULL;
    uint32_t          num_slots;

    if(!descr) {
        HAL_TRACE_ERR2("invalid arg");
        return HAL_RET_INVALID_ARG;
    }

    // Free page
    ret = pd_cpupkt_get_slot_addr(types::WRING_TYPE_CPU_TX_PR, tg_cpu_id,
                                  cpu_tx_page_cindex, &slot_addr, &slot_virt_addr, &num_slots);
    if(ret != HAL_RET_OK) {
        HAL_TRACE_ERR2("Failed to get to slot addr for page ci: {}",
                      cpu_tx_page_cindex);
        return ret;
    }

    value = htonll(descr->a0);
    HAL_TRACE_DEBUG2("cpu-id: {} update cpupr: slot: {:#x} ci: {} page: {:#x}, value: {:#x}",
                    tg_cpu_id, slot_addr, cpu_tx_page_cindex, descr->a0, value);
    if (asic_mem_write(slot_addr, (uint8_t *)&value, sizeof(uint64_t),
                       ASIC_WRITE_MODE_WRITE_THRU) != sdk::SDK_RET_OK) {
        HAL_TRACE_ERR2("Failed to program page to slot");
        return HAL_RET_HW_FAIL;
    }
    cpu_tx_page_cindex = ((cpu_tx_page_cindex + 1) % num_slots);

    // Free descr
    ret = pd_cpupkt_get_slot_addr(types::WRING_TYPE_CPU_TX_DR, tg_cpu_id,
                                  cpu_tx_descr_cindex, &slot_addr, &slot_virt_addr, &num_slots);
    if(ret != HAL_RET_OK) {
        HAL_TRACE_ERR2("Failed to get to slot addr for descr ci: {}",
                      cpu_tx_descr_cindex);
        return ret;
    }

    value = htonll(descr_addr);
    HAL_TRACE_DEBUG2("cpu-id: {} update cpudr: slot: {:#x} ci: {} descr: {:#x}, value: {:#x}",
        tg_cpu_id, slot_addr, cpu_tx_descr_cindex, descr_addr, value);
    if (asic_mem_write(slot_addr, (uint8_t *)&value, sizeof(uint64_t),
                       ASIC_WRITE_MODE_WRITE_THRU) != sdk::SDK_RET_OK) {
        HAL_TRACE_ERR2("Failed to program descr to slot");
        return HAL_RET_HW_FAIL;
    }

    /*
     * Ensure the programming of the descriptor/senq-queue is completed with a
     * memory barrier, before ringing the txdma doorbell.
     */
    PAL_barrier();

    cpu_tx_descr_cindex = ((cpu_tx_descr_cindex + 1) % num_slots);

    return HAL_RET_OK;
}

/*
 * Free the descriptor/page back to the CPU-TX-DR and CPU-TX-PR pools.
 */
hal_ret_t
pd_cpupkt_free_tx_descrs (uint64_t *descr_addrs, pd_descr_aol_t **virt_addrs,
              uint16_t pkt_count)
{
    hal_ret_t         ret = HAL_RET_OK;
    cpupkt_hw_id_t    slot_addr = 0;
    uint64_t          value = 0, *slot_virt_addr = NULL;
    cpupkt_hw_id_t    descr_addr;
    pd_descr_aol_t    *descr;
    uint32_t          num_slots;

    for (uint16_t npkt = 0; npkt < pkt_count; npkt++) {

        descr_addr = descr_addrs[npkt];
    descr = virt_addrs[npkt];

        // Free page
        ret = pd_cpupkt_get_slot_addr(types::WRING_TYPE_CPU_TX_PR, tg_cpu_id,
                      cpu_tx_page_cindex, &slot_addr, &slot_virt_addr, &num_slots);
    if (ret != HAL_RET_OK) {
            HAL_TRACE_ERR2("Failed to get to slot addr for page ci: {}",
              cpu_tx_page_cindex);
        return ret;
    }

    value = htonll(descr->a0);
    HAL_TRACE_DEBUG2("cpu-id: {} update cpupr: slot: {:#x} ci: {} page: {:#x}, value: {:#x}",
            tg_cpu_id, slot_addr, cpu_tx_page_cindex, descr->a0, value);
    if (asic_mem_write(slot_addr, (uint8_t *)&value, sizeof(uint64_t),
                           ASIC_WRITE_MODE_WRITE_THRU) != sdk::SDK_RET_OK) {
        HAL_TRACE_ERR2("Failed to program page to slot");
        return HAL_RET_HW_FAIL;
    }
        cpu_tx_page_cindex = ((cpu_tx_page_cindex + 1) % num_slots);

        // Free descr
    ret = pd_cpupkt_get_slot_addr(types::WRING_TYPE_CPU_TX_DR, tg_cpu_id,
                      cpu_tx_descr_cindex, &slot_addr, &slot_virt_addr, &num_slots);
    if (ret != HAL_RET_OK) {
            HAL_TRACE_ERR2("Failed to get to slot addr for descr ci: {}",
              cpu_tx_descr_cindex);
        return ret;
    }

    value = htonll(descr_addr);
    HAL_TRACE_DEBUG2("cpu-id: {} update cpudr: slot: {:#x} ci: {} descr: {:#x}, value: {:#x}",
            tg_cpu_id, slot_addr, cpu_tx_descr_cindex, descr_addr, value);
    if (asic_mem_write(slot_addr, (uint8_t *)&value, sizeof(uint64_t),
               ASIC_WRITE_MODE_WRITE_THRU) != sdk::SDK_RET_OK) {
        HAL_TRACE_ERR2("Failed to program descr to slot");
        return HAL_RET_HW_FAIL;
    }
        cpu_tx_descr_cindex = ((cpu_tx_descr_cindex + 1) % num_slots);
    }
    return HAL_RET_OK;
}

hal_ret_t
cpupkt_rx_upd_sem_index (cpupkt_qinst_info_t& qinst_info, bool init_pindex)
{
    cpupkt_hw_id_t pi_sem_addr = 0;
    cpupkt_hw_id_t ci_sem_addr = 0;
    static thread_local int sem_batch = 0;

    HAL_ABORT_TRACE(qinst_info.queue_id >= 0 && qinst_info.queue_id < 4,
                    "qinst_info->queue_id {} not initialized",
                    qinst_info.queue_id);
    switch(qinst_info.queue_info->type) {
    case types::WRING_TYPE_ARQRX:
        ci_sem_addr = ASIC_SEM_ARQ_CI_RAW_ADDR(qinst_info.queue_id);
        break;
    case types::WRING_TYPE_ASCQ:
        ci_sem_addr = ASIC_SEM_ASCQ_CI_RAW_ADDR(qinst_info.queue_id);
        break;
    default:
        return HAL_RET_OK;
    }

    /*
     * Update the ARQ/ASCQ semaphore CI values. We do this in a batch to
     * reduce CSR-write overheads.
     */
    if (!(++sem_batch % CPU_PKT_SEM_CI_BATCH_SIZE)) {
        HAL_TRACE_DEBUG2("updating CI: type: {}, addr {:#x}, ci: {}",
            qinst_info.queue_info->type, ci_sem_addr,
            qinst_info.pc_index);

    if (asic_reg_write(ci_sem_addr, &qinst_info.pc_index, 1,
               ASIC_WRITE_MODE_WRITE_THRU) != sdk::SDK_RET_OK) {
            qinst_info.ctr.rx_sem_wr_err++;
        HAL_TRACE_ERR2("Failed to program CI semaphore");
        return HAL_RET_HW_FAIL;
    }
    }

    /*
     * Initialize the semaphore to reset PI to zero.
     */
    if (init_pindex) {
        pi_sem_addr = ci_sem_addr - 4;
    uint32_t value = 0;
    HAL_TRACE_DEBUG2("updating PI: type: {}, addr {:#x}, pi: {}",
            qinst_info.queue_info->type, pi_sem_addr,
            value);
    if (asic_reg_write(pi_sem_addr, &value, 1,
               ASIC_WRITE_MODE_WRITE_THRU) != sdk::SDK_RET_OK) {
            HAL_TRACE_ERR2("Failed to program PI semaphore");
        return HAL_RET_HW_FAIL;
    }
    }

    return HAL_RET_OK;
}

/****************************************************************
 * Common Packet send/receive APIs
 ***************************************************************/

hal_ret_t
pd_cpupkt_ctxt_alloc_init (pd_func_args_t *pd_func_args)
{
    pd_cpupkt_ctxt_alloc_init_args_t *args = pd_func_args->pd_cpupkt_ctxt_alloc_init;
    args->ctxt = cpupkt_ctxt_init(cpupkt_ctxt_alloc());
    return HAL_RET_OK;
}

hal_ret_t
cpupkt_register_qinst (cpupkt_queue_info_t* ctxt_qinfo, int qinst_index,
                       types::WRingType type, uint32_t queue_id)
{
    HAL_TRACE_DEBUG2("creating qinst for type: {}, id: {}", type, queue_id);
    hal_ret_t           ret = HAL_RET_OK;
    if(!ctxt_qinfo || !ctxt_qinfo->wring_meta) {
        HAL_TRACE_ERR2("Invalid ARGs to register_qinst");
        return HAL_RET_INVALID_ARG;
    }

    // Verify that the queue inst is not already registered
    if(ctxt_qinfo->qinst_info[qinst_index] != NULL) {
        HAL_TRACE_ERR2("queue inst is already registered: type: {}, inst: {}", type, queue_id);
        return HAL_RET_OK;
    }

    cpupkt_qinst_info_t* qinst_info = cpupkt_ctxt_qinst_info_alloc();
    if(!qinst_info) {
        HAL_TRACE_ERR2("Failed to allocate qinst_info");
        return HAL_RET_NO_RESOURCE;
    }

    // Initialize Queue
    ret = wring_pd_table_init(type, queue_id);
    if(ret != HAL_RET_OK) {
        HAL_TRACE_ERR2("Failed to initialize queue: {}, id: {}, ret: {}",
                    type, queue_id, ret);
        return ret;
    }

    // Get queue base
    wring_hw_id_t base_addr = 0;
    ret = wring_pd_get_base_addr(type, queue_id, &base_addr);
    if(ret != HAL_RET_OK) {
        HAL_TRACE_ERR2("Failed to get base addr for queue: {}: ret: {}", type, ret);
        return ret;
    }

    qinst_info->base_addr = base_addr;
    qinst_info->queue_id = queue_id;
    qinst_info->queue_info = ctxt_qinfo;
    qinst_info->queue_tbl_shift = log2(qinst_info->queue_info->wring_meta->num_slots);
    qinst_info->ctr = {};

    /*
     * The base address of memory mapped WRING memory region is already memory mapped is
     * stored in wring_meta, we'll cache the base virtual address for our queue_id from
     * the wring_meta, so that we can do direct memory access instead of asic_read/write
     * API overheads.
     */
    if (is_cpu_zero_copy_enabled() &&
        wring_pd_get_meta_virt_base_addr(ctxt_qinfo->wring_meta, queue_id)) {

        qinst_info->virt_base_addr =
              wring_pd_get_meta_virt_base_addr(ctxt_qinfo->wring_meta, queue_id);
        HAL_TRACE_DEBUG2("mmap the CPU RX Ring(T:{}, Q:{}) phy {:#x} @ virt {:#x}",
            type, queue_id, (uint64_t)qinst_info->base_addr,
            (uint64_t)qinst_info->virt_base_addr);
    }

    if(is_cpu_tx_queue(type)) {
        qinst_info->pc_index = 0;
    } else {
        // For rx queues, initialize CI to wring slot, to detect Queue full condition.
        qinst_info->pc_index = qinst_info->queue_info->wring_meta->num_slots;
        cpupkt_rx_upd_sem_index(*qinst_info, true);

    /*
     * We'll do a one-time initialization of the CPU-RX-DPR CI semaphore here
     * to the size of the ring. This is the software CI for the semaphore that
     * we maintain in order to free back the CPU-RX-DPR descriptor-page entry
     * to the pool.
     */
    pd_wring_meta_t *meta = wring_pd_get_meta(types::WRING_TYPE_CPU_RX_DPR);
    if (!cpu_rx_dpr_sem_cindex) cpu_rx_dpr_sem_cindex = meta->num_slots;
    }

    cpupkt_update_slot_addr(qinst_info);
    ctxt_qinfo->qinst_info[qinst_index] = qinst_info;
    ctxt_qinfo->num_qinst++;

    return HAL_RET_OK;
}

hal_ret_t
pd_cpupkt_register_tx_queue (pd_func_args_t *pd_func_args)
{
    pd_cpupkt_register_tx_queue_args_t *args = pd_func_args->pd_cpupkt_register_tx_queue;
    cpupkt_ctxt_t* ctxt = args->ctxt;
    types::WRingType type = args->type;
    uint32_t queue_id = args->queue_id;

    HAL_TRACE_DEBUG2("register Tx Queue: type:{} id:{}", type, queue_id);
    if(!ctxt) {
        HAL_TRACE_ERR2("ctxt is null");
        return HAL_RET_INVALID_ARG;
    }

    if(!is_cpu_tx_queue(type)) {
        HAL_TRACE_ERR2("Queue is not a valid cpu queue: {}", type);
        return HAL_RET_INVALID_ARG;
    }

    pd_wring_meta_t* meta = wring_pd_get_meta(type);
    if(!meta) {
        HAL_TRACE_ERR2("Failed to find meta for the queue: {}", type);
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
pd_cpupkt_register_rx_queue (pd_func_args_t *pd_func_args)
{
    pd_cpupkt_register_rx_queue_args_t *args = pd_func_args->pd_cpupkt_register_rx_queue;
    cpupkt_ctxt_t* ctxt = args->ctxt;
    types::WRingType type = args->type;
    uint32_t queue_id = args->queue_id;

    HAL_TRACE_DEBUG2("register Rx Queue: type:{} id:{}", type, queue_id);

    if(!ctxt) {
        HAL_TRACE_ERR2("cpupkt:Ctxt is null");
        return HAL_RET_INVALID_ARG;
    }

    if(!is_cpu_rx_queue(type)) {
        HAL_TRACE_ERR2("Queue is not a valid cpu queue: {}", type);
        return HAL_RET_INVALID_ARG;
    }

    // Verify if the queeue is already registered
    for(uint32_t i = 0; i< ctxt->rx.num_queues; i++) {
        if(ctxt->rx.queue[i].type == type) {
            HAL_TRACE_DEBUG2("cpupkt:Queue is already added: {}", type);
            return HAL_RET_OK;
        }
    }

    if(ctxt->rx.num_queues == MAX_CPU_PKT_QUEUES) {
        HAL_TRACE_ERR2("cpupkt:Max queues registered");
        return HAL_RET_NO_RESOURCE;
    }

    pd_wring_meta_t* meta = wring_pd_get_meta(type);
    if(!meta) {
        HAL_TRACE_ERR2("Failed to find meta for the queue: {}", type);
        return HAL_RET_WRING_NOT_FOUND;
    }

    if(is_cpu_send_comp_queue(type) && tg_cpu_id < 0) {
        tg_cpu_id = args->queue_id;
        HAL_TRACE_DEBUG2("Updated cpu_id to: {}", tg_cpu_id);
    }

    int index = ctxt->rx.num_queues;
    ctxt->rx.queue[index].type = type;
    ctxt->rx.queue[index].wring_meta = meta;
    cpupkt_register_qinst(&(ctxt->rx.queue[index]), ctxt->rx.queue[index].num_qinst, type, queue_id);
    ctxt->rx.num_queues++;
    return HAL_RET_OK;
}

hal_ret_t
pd_cpupkt_unregister_tx_queue (pd_func_args_t *pd_func_args)
{
    pd_cpupkt_unregister_tx_queue_args_t *args = pd_func_args->pd_cpupkt_unregister_tx_queue;
    cpupkt_ctxt_t* ctxt = args->ctxt;
    types::WRingType type = args->type;
    uint32_t queue_id = args->queue_id;

    HAL_TRACE_DEBUG2("unregister Tx Queue: type:{} id:{}", type, queue_id);

    if(!ctxt) {
        HAL_TRACE_ERR2("Ctxt is null");
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
cpupkt_inc_queue_index (cpupkt_qinst_info_t& qinst_info)
{
    qinst_info.pc_index++;
    cpupkt_update_slot_addr(&qinst_info);
    //HAL_TRACE_DEBUG2("incremented  pc_index queue: {} to index: {} addr: {:#x}",
    //                    qinst_info.queue_id,  qinst_info.pc_index, qinst_info.pc_index_addr);
}

static inline hal_ret_t
cpupkt_free_and_inc_queue_index (cpupkt_qinst_info_t& qinst_info)
{
    // Set the slot back to 0
    uint64_t value = 0;
    if (asic_mem_write(qinst_info.pc_index_addr, (uint8_t *)&value,
                       sizeof(uint64_t),
                       ASIC_WRITE_MODE_WRITE_THRU) != sdk::SDK_RET_OK) {
        HAL_TRACE_ERR2("Failed to reset pc_index_addr");
        return HAL_RET_HW_FAIL;
    }
    cpupkt_inc_queue_index(qinst_info);
    //HAL_TRACE_DEBUG2("freed and inc pc_index queue: {} index: {} addr: {:#x}",
    //                    qinst_info.queue_id,  qinst_info.pc_index, qinst_info.pc_index_addr);

    return HAL_RET_OK;
}

bool
is_valid_slot_value (cpupkt_qinst_info_t* qinst_info, uint64_t slot_value,
                     uint64_t* descr_addr)
{
    if ((slot_value > 0) &&
        ((slot_value & CPU_PKT_VALID_BIT_MASK) == qinst_info->valid_bit_value)) {
        *descr_addr = (qinst_info->valid_bit_value == 0) ? slot_value : (slot_value & ~(uint64_t)CPU_PKT_VALID_BIT_MASK);
        //HAL_TRACE_DEBUG2("descr_addr: {:#x}", *descr_addr);
        return true;
    }
    *descr_addr = 0;
    return false;
}

/*
 * Given a descriptor address, read the AOL fields and return the packet data
 * fields. This routine handles both the zero-copy case where a pointer to the
 * original packet page is returned, and the packet copy case.
 */
hal_ret_t
cpupkt_descr_to_headers (pd_descr_aol_t *descr,
                         p4_to_p4plus_cpu_pkt_t** flow_miss_hdr,
                         uint8_t** data, size_t* data_len, bool *copied_pkt, bool no_copy)
{
    if(!flow_miss_hdr || !data || !data_len) {
        return HAL_RET_INVALID_ARG;
    }

    HAL_TRACE_DEBUG2("Descriptor0: a: {:#x}, o: {}, l: {}",
                     descr->a0, descr->o0, descr->l0);
    HAL_TRACE_DEBUG2("Descriptor1: a: {:#x}, o: {}, l: {}",
                     descr->a1, descr->o1, descr->l1);
    HAL_TRACE_DEBUG2("Descriptor2: a: {:#x}, o: {}, l: {}",
                     descr->a2, descr->o2, descr->l2);

    /*
     * App redirect proxied packets (from TCP/TLS) may use more than one
     * page (currently up to 2) to hold meta headers and payload.
     */
    if((descr->l0 > JUMBO_FRAME_SIZE) ||
       (descr->l1 > JUMBO_FRAME_SIZE) ) {
        HAL_TRACE_DEBUG2("corrupted packet");
        return HAL_RET_HW_FAIL;
    }

    if((descr->a0 == 0) ||
       (descr->l0 + descr->l1) < sizeof(p4_to_p4plus_cpu_pkt_t)) {
        // Packet should always have a valid address and
        // packet length should be minimum of cpupkt header.
        HAL_TRACE_DEBUG2("Received packet with invalid address/length");
        return HAL_RET_HW_FAIL;
    }

    //hard-coding to 9K buffer for now. needs to be replaced with page.
    //uint8_t* buffer = (uint8_t* ) malloc(9216);
    uint8_t *buffer;

    if (likely(is_cpu_zero_copy_enabled() && no_copy)) {

        /*
     * Packet page is at a fixed offset from the descriptor, as we use a
     * single descriptor+page contiguous entry for packets.
     * Also set the 'copied_pkt' flag to false so that the
     * caller knows this is a zero-copy buffer and not a local slab-allocated
     * buffer, so that it doesn't try to free this page.
         */
        buffer = (uint8_t *)descr + ASIC_CPU_RX_DPR_PAGE_OFFSET;
    *flow_miss_hdr = (p4_to_p4plus_cpu_pkt_t*)buffer;
    *copied_pkt = false;
    } else {
        buffer = (uint8_t *) g_hal_state->cpu_pkt_slab()->alloc();

    uint64_t pktaddr = descr->a0 + descr->o0;

    HAL_TRACE_DEBUG2("read buffer of length {}", descr->l0 + descr->l1);

    if (sdk::asic::asic_mem_read(pktaddr, buffer, descr->l0, true) !=
            sdk::SDK_RET_OK) {
        HAL_TRACE_ERR2("Failed to read hbm page");
        free(buffer);
        return HAL_RET_HW_FAIL;
    }
    if (descr->l1) {
            uint64_t pktaddr = descr->a1 + descr->o1;
        if (sdk::asic::asic_mem_read(pktaddr, buffer + descr->l0,
                     descr->l1, true) != SDK_RET_OK) {
                HAL_TRACE_ERR2("Failed to read hbm page 1");
        free(buffer);
        return HAL_RET_HW_FAIL;
        }
    }
    *flow_miss_hdr = (p4_to_p4plus_cpu_pkt_t*)buffer;

        /*
     * Also set the 'copied_pkt' flag to true here so that the
     * caller knows this is a local slab-allocated buffer,
         * so that caller will free it after packet processing is done.
         */
    *copied_pkt = true;
    }

    *data = buffer + sizeof(p4_to_p4plus_cpu_pkt_t);
    *data_len = (descr->l0 + descr->l1) - sizeof(p4_to_p4plus_cpu_pkt_t);
    return HAL_RET_OK;
}

hal_ret_t
pd_cpupkt_poll_receive (pd_func_args_t *pd_func_args)
{
    hal_ret_t   ret = HAL_RET_OK;
    pd_cpupkt_poll_receive_args_t *args = pd_func_args->pd_cpupkt_poll_receive;
    cpupkt_ctxt_t* ctxt = args->ctxt;
    p4_to_p4plus_cpu_pkt_t** flow_miss_hdr = args->flow_miss_hdr;
    uint8_t** data = args->data;
    size_t* data_len = args->data_len;
    bool    *copied_pkt = args->copied_pkt;

    if (!ctxt) {
        return HAL_RET_INVALID_ARG;
    }

    // HAL_TRACE_DEBUG2("cpupkt:Starting packet poll for queue: {}", ctxt->rx.num_queues);
    uint64_t value, descr_addr;
    cpupkt_qinst_info_t* qinst_info = NULL;

    for(uint32_t i=0; i< ctxt->rx.num_queues; i++) {
        value = 0;
        qinst_info = ctxt->rx.queue[i].qinst_info[0];
        qinst_info->ctr.poll_count++;

    /*
     * If zero-copy enabled, we'll read the slot directly without the
     * asic_mem_read() API overhead.
     */
    if (is_cpu_zero_copy_enabled() && qinst_info->virt_pc_index_addr) {
            value = *(uint64_t*)qinst_info->virt_pc_index_addr;
    } else {
        if (sdk::asic::asic_mem_read(qinst_info->pc_index_addr,
                     (uint8_t *)&value,
                     sizeof(uint64_t), true) != sdk::SDK_RET_OK) {
                qinst_info->ctr.rx_slot_value_read_err++;
        HAL_TRACE_ERR2("Failed to read the slot value from the hw");
        return HAL_RET_HW_FAIL;
        }
    }

    value = ntohll(value);
        if (!is_valid_slot_value(qinst_info, value, &descr_addr)) {
            continue;
        }

        HAL_TRACE_DEBUG2("Rcvd valid data: queue: {}, qid: {} pc_index: {}, "
                        "addr: {:#x}, value: {:#x}, descr_addr: {:#x}",
                        ctxt->rx.queue[i].type, qinst_info->queue_id,
                        qinst_info->pc_index, qinst_info->pc_index_addr,
                        value, descr_addr);

        pd_descr_aol_t *descr_p = NULL;
        bool           cpu_rx_descr = false, cpu_tx_descr = false;
        bool           is_comp_queue = is_cpu_send_comp_queue(ctxt->rx.queue[i].type);

    /*
     * We've received a packet descriptor on one of our queues. Let us determine
     * if it an RX-descriptor (CPU-RX-DPR pool) or a TX-descriptor (CPU-TX-DR pool)
     * so we know where to free them to. These APIs also return the virtual-address
     * of the descriptor if this region is memory mapped to this process, for direct
     * memory access.
     */
    cpu_rx_descr = is_phyaddr_cpu_rx_dpr_descr(descr_addr, tg_cpu_id, &descr_p);
    if (!cpu_rx_descr) cpu_tx_descr = is_phyaddr_cpu_tx_dr_descr(descr_addr, tg_cpu_id, &descr_p);

    if (is_comp_queue) {
            cpu_tx_descr ? qinst_info->ctr.send_pkts++ : qinst_info->ctr.recv_pkts++;
    } else {
            qinst_info->ctr.recv_pkts++;
    }

        if (!descr_p) {
        // get the descriptor
        pd_descr_aol_t  descr = {0};
        if (sdk::asic::asic_mem_read(descr_addr, (uint8_t *)&descr,
                         sizeof(pd_descr_aol_t), true) != sdk::SDK_RET_OK) {
                qinst_info->ctr.rx_descr_read_err++;
        HAL_TRACE_ERR2("Failed to read the descr from hw");
        return HAL_RET_HW_FAIL;
        }
        descr_p = &descr;
        HAL_TRACE_DEBUG2("Received packet descriptor {:#x} from {} memory pool",
                descr_addr, cpu_rx_descr ? "CPU-Rx" : (cpu_tx_descr ? "CPU-Tx" : "RNMDPR"));
    } else {
        HAL_TRACE_DEBUG2("Received packet descriptor {:#x} from {} memory pool, virtual-addr {:#x}",
                descr_addr, cpu_rx_descr ? "CPU-Rx" : (cpu_tx_descr ? "CPU-Tx" : "RNMDPR"),
                descr_p ? (uint64_t) descr_p : (uint64_t)-1);
    }

    /*
     * If we received the descriptor on the completion-queue (one of the ASCQ's), we
     * will free the descriptor to its corresponding pool.
     */
        if (is_comp_queue) {
        if (cpu_rx_descr) {
                ret = pd_cpupkt_free_rx_descr(descr_addr);
                if (ret != HAL_RET_OK) {
            cpu_rx_dpr_descr_free_err++;
                    HAL_TRACE_ERR2("Failed to free tx descr: {}", ret);
                }
                ret = HAL_RET_RETRY;
        } else if (cpu_tx_descr) {
                ret = pd_cpupkt_free_tx_descr(descr_addr, descr_p);
                if (ret != HAL_RET_OK) {
                    qinst_info->ctr.tx_descr_free_err++;
                    HAL_TRACE_ERR2("Failed to free tx descr: {}", ret);
                }
                ret = HAL_RET_RETRY;
        } else {
            HAL_TRACE_ERR2("Packet descriptor from Invalid pool, unable to free {:#x}",
                  (uint64_t) descr_p);
        abort();
        }
        } else {

            /*
         * Lets get the packet header/data from the descriptor we've received.
         */
        ret = cpupkt_descr_to_headers(descr_p, flow_miss_hdr, data, data_len, copied_pkt,
                      cpu_rx_descr);
            if(ret != HAL_RET_OK) {
                qinst_info->ctr.rx_descr_to_hdr_err++;
                HAL_TRACE_ERR2("Failed to convert descr to headers");
            } else {
            if (cpu_rx_descr) {

                    /*
             * If we're not doing zero-copy and this was a CPU-RX-DPR
             * descriptor, then we will free this descriptor directly
             * to the pool, and not go through the Garbage-collector (GC)
             * P4+ program.
             */
            if (!is_cpu_zero_copy_enabled()) {
                ret = pd_cpupkt_free_rx_descr(descr_addr);
            if (ret != HAL_RET_OK) {
                cpu_rx_dpr_descr_free_err++;
                HAL_TRACE_ERR2("Failed to free tx descr: {}", ret);
            }
            }
        } else {

            /*
             * Free the descriptor to GC.
             */
                    ret = cpupkt_descr_free(descr_addr);
            if (ret != HAL_RET_OK) {
                qinst_info->ctr.rx_descr_free_err++;
            HAL_TRACE_ERR2("Failed to free rx descr");
            }
        }
            }
        }

    /*
     * Increment the queue's slot index/addr to poll on next.
     */
        cpupkt_inc_queue_index(*qinst_info);

    /*
     * Update the CI semaphore of the ARQ/ASCQ where we received the above
     * packet, to indicate that the slot is free to be used by P4+ for
     * the next packet to be enqueued.
     */
        cpupkt_rx_upd_sem_index(*qinst_info, false);

        return ret;
    }

    return HAL_RET_RETRY;
}

/*
 * Free packet resources allocated from data-path.
 */
hal_ret_t
pd_cpupkt_free_pkt_resources (pd_func_args_t *pd_func_args)
{
    pd_cpupkt_free_pkt_resources_args_t *args = pd_func_args->pd_cpupkt_free_pkt_resources;
    cpupkt_hw_id_t phy_page_addr = 0;
    hal_ret_t       ret = HAL_RET_OK;

    uint8_t *pkt = args->pkt;

    if (!pkt) {
        cpu_rx_dpr_descr_invalid_free_err++;
        return(HAL_RET_INVALID_ARG);
    }

    /*
     * Only if this is a zero-copy page from CPU-RX-DPR pool, we have to free it back
     * to the pool, else we dont have anything to do.
     */
    bool no_copy = (is_cpu_zero_copy_enabled() &&
            is_virtaddr_cpu_rx_dpr_page((cpupkt_hw_id_t) pkt, 0, &phy_page_addr));

    if (no_copy) {
        cpupkt_hw_id_t descr_addr = (cpupkt_hw_id_t)(phy_page_addr - ASIC_CPU_RX_DPR_PAGE_OFFSET);

    ret = pd_cpupkt_free_rx_descr(descr_addr);
    if (ret != HAL_RET_OK) {
        cpu_rx_dpr_descr_free_err++;
        HAL_TRACE_ERR2("Failed to free tx descr: {}", ret);
    }
    } else {
    HAL_TRACE_DEBUG2("Unknown packet page-addr: {:#x}, page-virt-addr {:#x}", phy_page_addr,
             (uint64_t) pkt);
        cpu_rx_dpr_descr_invalid_free_err++;
    }

    return(ret);
}

static thread_local uint64_t descr_addrs[CPUPKT_MAX_BATCH_SIZE];
static thread_local uint64_t rxq_descr_addrs[CPUPKT_MAX_BATCH_SIZE];
static thread_local uint64_t compq_tx_descr_addrs[CPUPKT_MAX_BATCH_SIZE];
static thread_local uint64_t compq_rx_descr_addrs[CPUPKT_MAX_BATCH_SIZE];
static thread_local pd_descr_aol_t *rxq_descr_virt_ptrs[CPUPKT_MAX_BATCH_SIZE];
static thread_local pd_descr_aol_t *compq_rx_descr_virt_ptrs[CPUPKT_MAX_BATCH_SIZE];
static thread_local pd_descr_aol_t *compq_tx_descr_virt_ptrs[CPUPKT_MAX_BATCH_SIZE];
static thread_local bool cpu_rx_descrs[CPUPKT_MAX_BATCH_SIZE];
static thread_local pd_descr_aol_t rxq_descr_copy[CPUPKT_MAX_BATCH_SIZE]; // copy for non-HW platforms

hal_ret_t
pd_cpupkt_poll_receive_new (pd_func_args_t *pd_func_args)
{
    hal_ret_t   ret = HAL_RET_OK;
    pd_cpupkt_poll_receive_new_args_t *args = pd_func_args->pd_cpupkt_poll_receive_new;
    cpupkt_pkt_batch_t *pkt_batch = args->pkt_batch;
    cpupkt_ctxt_t      *ctxt = args->ctxt;
    uint16_t           *app_pkt_count = &(pkt_batch->pktcount);
    bool               cpu_rx_descr, cpu_tx_descr;
    cpupkt_queue_info_t *compq, *rxq;
    uint16_t pktcount, qpkt_count, npkt, compq_rx_pkt_count, compq_tx_pkt_count, rxq_pkt_count;

    if (!ctxt) {
        return HAL_RET_INVALID_ARG;
    }

    // HAL_TRACE_DEBUG2("cpupkt:Starting packet poll for queue: {}", ctxt->rx.num_queues);
    uint64_t value;
    cpupkt_qinst_info_t *qinst_info = NULL;
    bool more_pkt;
    pktcount = compq_rx_pkt_count = compq_tx_pkt_count = rxq_pkt_count = 0;
    bool comp_queue = false;

    if (!ctxt->rx.num_queues) return(HAL_RET_RETRY);

    for(uint32_t i = 0; i < ctxt->rx.num_queues && pktcount < CPUPKT_MAX_BATCH_SIZE; i++) {
        value = 0;
        qinst_info = ctxt->rx.queue[i].qinst_info[0];
    qpkt_count = 0;
        comp_queue = is_cpu_send_comp_queue(ctxt->rx.queue[i].type);

    if (comp_queue) {
        compq = &ctxt->rx.queue[i];
    } else {
        rxq = &ctxt->rx.queue[i];
    }
    do {
        qinst_info->ctr.poll_count++;
        more_pkt = false;

        /*
         * If zero-copy enabled, we'll read the slot directly without the
         * asic_mem_read() API overhead.
         */
        if (likely(is_cpu_zero_copy_enabled() && qinst_info->virt_pc_index_addr)) {
                value = *(uint64_t*)qinst_info->virt_pc_index_addr;
        } else {
            if (sdk::asic::asic_mem_read(qinst_info->pc_index_addr,
                         (uint8_t *)&value,
                         sizeof(uint64_t), true) != sdk::SDK_RET_OK) {
            qinst_info->ctr.rx_slot_value_read_err++;
            HAL_TRACE_ERR2("Failed to read the slot value from the hw");
            return HAL_RET_HW_FAIL;
        }
        }

        value = ntohll(value);
        if (!is_valid_slot_value(qinst_info, value, &descr_addrs[pktcount])) {
            continue;
        }

        //HAL_TRACE_DEBUG2("Rcvd valid data (count {}): queue: {}, qid: {} pc_index: {}, "
        //      "addr: {:#x}, value: {:#x}, descr_addr: {:#x}", pktcount,
        //      ctxt->rx.queue[i].type, qinst_info->queue_id,
        //      qinst_info->pc_index, qinst_info->pc_index_addr,
        //      value, descr_addrs[pktcount]);

        /*
         * Increment the queue's slot index/addr to poll on next.
         */
        cpupkt_inc_queue_index(*qinst_info);

        pd_descr_aol_t *descr_p = NULL;

        /*
         * We've received a packet descriptor on one of our queues. Let us determine
         * if it an RX-descriptor (CPU-RX-DPR pool) or a TX-descriptor (CPU-TX-DR pool)
         * or some other descriptor (like RNMDPR/IPsec-RNMDPR etc for service-lif packets)
         * so we know where to free them to. These APIs also return the virtual-address
         * of the descriptor if this region is memory mapped to this process, for direct
         * memory access.
         */
        cpu_rx_descr = is_phyaddr_cpu_rx_dpr_descr(descr_addrs[pktcount], tg_cpu_id, &descr_p);
        if (!cpu_rx_descr) {
            cpu_tx_descr = is_phyaddr_cpu_tx_dr_descr(descr_addrs[pktcount], tg_cpu_id, &descr_p);
            }

            /*
             * If 'descr_p' is NULL, its either a platform where the descriptor memory is not mapped
             * to this process or its not a CPU Rx/Tx descriptor.
             */
            if (!descr_p) {

                 /*
                  * get the descriptor
                  * We'll make a copy of the descriptor here, as we can't do a direct memory access
                  * to the descriptor memory (This is only for the non-HW platforms/where descriptor
                  * memory is not directly mapped to the process).
                  */
            if (sdk::asic::asic_mem_read(descr_addrs[pktcount], (uint8_t *)&rxq_descr_copy[pktcount],
                         sizeof(pd_descr_aol_t), true) != sdk::SDK_RET_OK) {
            qinst_info->ctr.rx_descr_read_err++;
            HAL_TRACE_ERR2("Failed to read the descr from hw");
            return HAL_RET_HW_FAIL;
        }
        descr_p = &rxq_descr_copy[pktcount];
        }

        if (comp_queue) {
        if (cpu_rx_descr) {
            compq_rx_descr_addrs[compq_rx_pkt_count] = descr_addrs[pktcount];
            compq_rx_descr_virt_ptrs[compq_rx_pkt_count] = descr_p;
            compq_rx_pkt_count++;
        } else if (cpu_tx_descr) {
            compq_tx_descr_addrs[compq_tx_pkt_count] = descr_addrs[pktcount];
            compq_tx_descr_virt_ptrs[compq_tx_pkt_count] = descr_p;
            compq_tx_pkt_count++;
        } else {
                HAL_TRACE_ERR2("Packet descriptor from Invalid pool, unable to free {:#x}",
                  (uint64_t) descr_p);
            abort();
        }
        } else {
            rxq_descr_addrs[rxq_pkt_count] = descr_addrs[pktcount];
        rxq_descr_virt_ptrs[rxq_pkt_count] = descr_p;
        cpu_rx_descrs[rxq_pkt_count] = cpu_rx_descr;
        rxq_pkt_count++;
        }

        qinst_info->ctr.recv_pkts++;
        pktcount++;
        qpkt_count++;
        more_pkt = true;

    } while (more_pkt && pktcount < CPUPKT_MAX_BATCH_SIZE);

    /*
     * Update the CI semaphore of the ARQ/ASCQ where we received the above
     * packet, to indicate that the slot is free to be used by P4+ for
     * the next packet to be enqueued.
     */
        if (qpkt_count) cpupkt_rx_upd_sem_index(*qinst_info, false);
    }

    (*app_pkt_count) = 0;
    if (!pktcount) {
    return(HAL_RET_RETRY);
    }
    bzero(pkt_batch->pkts, (sizeof(cpupkt_pktinfo_t)*rxq_pkt_count));

    /*
     * If we received the descriptor on the completion-queue (one of the ASCQ's), we
     * will free the descriptor to its corresponding pool.
     */
    if (compq_rx_pkt_count) {
        ret = pd_cpupkt_free_rx_descrs(compq_rx_descr_addrs, compq_rx_descr_virt_ptrs, compq_rx_pkt_count);
    if (ret != HAL_RET_OK) {
        compq->qinst_info[0]->ctr.rx_descr_free_err++;
        HAL_TRACE_ERR2("Failed to free rx descrs: {}", ret);
    }
    }

    if (compq_tx_pkt_count) {
        ret = pd_cpupkt_free_tx_descrs(compq_tx_descr_addrs, compq_tx_descr_virt_ptrs, compq_tx_pkt_count);
    if (ret != HAL_RET_OK) {
        compq->qinst_info[0]->ctr.tx_descr_free_err++;
        HAL_TRACE_ERR2("Failed to free tx descrs: {}", ret);
    }
    }

    for (npkt = 0; npkt < rxq_pkt_count; npkt++) {
        /*
         * Lets get the packet header/data from the descriptor we've received.
         */
        ret = cpupkt_descr_to_headers(rxq_descr_virt_ptrs[npkt], &(pkt_batch->pkts[*app_pkt_count].cpu_rxhdr),
                &(pkt_batch->pkts[*app_pkt_count].pkt), &(pkt_batch->pkts[*app_pkt_count].pkt_len),
                &(pkt_batch->pkts[*app_pkt_count].copied_pkt), cpu_rx_descrs[npkt]);

        if(ret != HAL_RET_OK) {
            rxq->qinst_info[0]->ctr.rx_descr_to_hdr_err++;
            HAL_TRACE_ERR2("Failed to convert descr to headers");
            if (cpu_rx_descrs[npkt] && is_cpu_zero_copy_enabled()) {
                ret = pd_cpupkt_free_rx_descr(rxq_descr_addrs[npkt]);
                if (ret != HAL_RET_OK) {
                    rxq->qinst_info[0]->ctr.tx_descr_free_err++;
                    HAL_TRACE_ERR2("Failed to free rx descr: {}", ret);
                }
            }
        } else {

            (*app_pkt_count)++;

            if (cpu_rx_descrs[npkt]) {

                /*
                 * If we're not doing zero-copy and this was a CPU-RX-DPR
                 * descriptor, then we will free this descriptor directly
                 * to the pool, and not go through the Garbage-collector (GC)
                 * P4+ program.
                 */
                if (unlikely(!is_cpu_zero_copy_enabled())) {
                    ret = pd_cpupkt_free_rx_descr(rxq_descr_addrs[npkt]);
                    if (ret != HAL_RET_OK) {
                        rxq->qinst_info[0]->ctr.tx_descr_free_err++;
                        HAL_TRACE_ERR2("Failed to free rx descr: {}", ret);
                    }
                }
            } else {

                /*
                 * Free the descriptor to GC.
                 */
                ret = cpupkt_descr_free(rxq_descr_addrs[npkt]);
                if (ret != HAL_RET_OK) {
                    rxq->qinst_info[0]->ctr.rx_descr_free_err++;
                    HAL_TRACE_ERR2("Failed to free tx descr");
                }
            }
        }
    }

    return (*app_pkt_count) ? ret : HAL_RET_RETRY;
}

hal_ret_t
cpupkt_descr_free (cpupkt_hw_id_t descr_addr)
{
    hal_ret_t       ret = HAL_RET_OK;
    wring_hw_id_t   gc_slot_addr=0;
    pd_func_args_t  pd_func_args = {0};

    // program GC queue
    ret = wring_pd_get_base_addr(types::WRING_TYPE_NMDR_RX_GC,
                                 ASIC_RNMDR_GC_CPU_ARM_RING_PRODUCER,
                                 &gc_slot_addr);
    gc_slot_addr += (gc_pindex * ASIC_HBM_RNMDR_ENTRY_SIZE);

    uint64_t value = htonll(descr_addr);
    HAL_TRACE_DEBUG2("Programming GC queue: {:#x}, descr: {:#x}, gc_pindex: {}, value: {:#x}",
                        gc_slot_addr, descr_addr, gc_pindex, value);
    if (asic_mem_write(gc_slot_addr, (uint8_t *)&value,
                       sizeof(cpupkt_hw_id_t),
                       ASIC_WRITE_MODE_WRITE_THRU) != sdk::SDK_RET_OK) {
        HAL_TRACE_ERR2("Failed to program gc queue");
        return HAL_RET_HW_FAIL;
    }

    /*
     * Ensure the programming of the descriptor/senq-queue is completed with a
     * memory barrier, before ringing the txdma doorbell.
     */
    PAL_barrier();

    // ring doorbell
    pd_cpupkt_program_send_ring_doorbell_args_t d_args = {0};
    d_args.dest_lif = SERVICE_LIF_GC;
    d_args.qtype = ASIC_HBM_GC_RNMDR_QTYPE;
    d_args.qid = ASIC_RNMDR_GC_CPU_ARM_RING_PRODUCER;
    d_args.ring_number = 0;
    d_args.flags = DB_IDX_UPD_PIDX_SET | DB_SCHED_UPD_SET;
    d_args.pidx = (gc_pindex + 1) & ASIC_HBM_GC_PER_PRODUCER_RING_MASK;
    pd_func_args.pd_cpupkt_program_send_ring_doorbell = &d_args;
    ret = pd_cpupkt_program_send_ring_doorbell(&pd_func_args);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR2("Failed to ring doorbell");
        return HAL_RET_HW_FAIL;
    }

    gc_pindex = (gc_pindex + 1) & ASIC_HBM_GC_PER_PRODUCER_RING_MASK;

    return HAL_RET_OK;
}

/****************************************************************
 * Packet Send APIs
 ***************************************************************/

hal_ret_t
pd_cpupkt_descr_alloc (pd_func_args_t *pd_func_args)
{
    hal_ret_t           ret = HAL_RET_OK;
    pd_cpupkt_descr_alloc_args_t *args = pd_func_args->pd_cpupkt_descr_alloc;
    cpupkt_hw_id_t      slot_addr;
    cpupkt_hw_id_t      *descr_addr = args->descr_addr;
    uint64_t            *slot_virt_addr = NULL;
    cpupkt_hw_id_t base_addr = 0;
    pd_wring_meta_t *wring_meta = wring_pd_get_meta(types::WRING_TYPE_CPU_TX_DR);

    if(!descr_addr) {
        return HAL_RET_INVALID_ARG;
    }

    if(!wring_meta) {
      HAL_TRACE_ERR2("Failed to get wring meta for type: {}", types::WRING_TYPE_CPU_TX_DR);
        return HAL_RET_WRING_NOT_FOUND;
    }

    ret = wring_pd_get_base_addr(types::WRING_TYPE_CPU_TX_DR, tg_cpu_id, &base_addr);
    if(ret != HAL_RET_OK) {
        HAL_TRACE_ERR2("failed to get base address for the wring: {}", types::WRING_TYPE_CPU_TX_DR);
        return HAL_RET_ERR;
    }

    // PI/CI check
    if(((cpu_tx_descr_pindex + 1) % wring_meta->num_slots) == cpu_tx_descr_cindex) {
        HAL_TRACE_ERR2("No free descr entries available: pi: {}, ci: {}",
                      cpu_tx_descr_pindex, cpu_tx_descr_cindex);
    cpu_tx_descr_full_err++;
        return HAL_RET_NO_RESOURCE;
    }

    ret = pd_cpupkt_get_slot_addr_for_ring(wring_meta, base_addr, tg_cpu_id,
                                           cpu_tx_descr_pindex, &slot_addr, &slot_virt_addr);
    if(ret != HAL_RET_OK) {
        HAL_TRACE_ERR2("failed to get to slot addr for descr pi: {}",
                      cpu_tx_descr_pindex);
        return ret;
    }

    if (sdk::asic::asic_mem_read(slot_addr, (uint8_t *)descr_addr,
                                 sizeof(cpupkt_hw_id_t), true) != sdk::SDK_RET_OK) {
        HAL_TRACE_ERR2("Failed to read descr addr from CPUDR PI: {}, slot: {:#x}",
                      cpu_tx_descr_pindex, slot_addr);
        return HAL_RET_HW_FAIL;
    }
    *descr_addr = ntohll(*descr_addr);
    HAL_TRACE_DEBUG2("cpupdr allocated pi: {} slot_addr: {:#x} descr: {:#x}",
                    cpu_tx_descr_pindex, slot_addr, *descr_addr);
    cpu_tx_descr_pindex++;

    return HAL_RET_OK;
}

hal_ret_t
pd_cpupkt_page_alloc (pd_func_args_t *pd_func_args)
{
    hal_ret_t           ret = HAL_RET_OK;
    pd_cpupkt_page_alloc_args_t *args = pd_func_args->pd_cpupkt_page_alloc;
    cpupkt_hw_id_t      slot_addr;
    cpupkt_hw_id_t* page_addr = args->page_addr;
    uint64_t        *slot_virt_addr = NULL;
    cpupkt_hw_id_t base_addr = 0;
    pd_wring_meta_t *wring_meta = wring_pd_get_meta(types::WRING_TYPE_CPU_TX_PR);

    if(!page_addr) {
        return HAL_RET_INVALID_ARG;
    }

    if(!wring_meta) {
      HAL_TRACE_ERR2("Failed to get wring meta for type: {}", types::WRING_TYPE_CPU_TX_PR);
        return HAL_RET_WRING_NOT_FOUND;
    }

    ret = wring_pd_get_base_addr(types::WRING_TYPE_CPU_TX_PR, tg_cpu_id, &base_addr);
    if(ret != HAL_RET_OK) {
        HAL_TRACE_ERR2("failed to get base address for the wring: {}", types::WRING_TYPE_CPU_TX_PR);
        return HAL_RET_ERR;
    }

    // PI/CI check
    if(((cpu_tx_page_pindex + 1) % wring_meta->num_slots) == cpu_tx_page_cindex) {
        HAL_TRACE_ERR2("No free page entries available: pi: {}, ci: {}",
                      cpu_tx_page_pindex, cpu_tx_page_cindex);
    cpu_tx_page_full_err++;
        return HAL_RET_NO_RESOURCE;
    }

    ret = pd_cpupkt_get_slot_addr_for_ring(wring_meta, base_addr, tg_cpu_id,
                                           cpu_tx_page_pindex, &slot_addr, &slot_virt_addr);
    if(ret != HAL_RET_OK) {
        HAL_TRACE_ERR2("failed to get to slot addr for page pi: {}",
                      cpu_tx_page_pindex);
        return ret;
    }

    if (sdk::asic::asic_mem_read(slot_addr, (uint8_t *)page_addr,
                                 sizeof(cpupkt_hw_id_t),
                                 true) != sdk::SDK_RET_OK) {
        HAL_TRACE_ERR2("Failed to read page address from CPUPR PI: {} slot: {:#x}",
                      cpu_tx_page_pindex, slot_addr);
        return HAL_RET_HW_FAIL;
    }
    *page_addr = ntohll(*page_addr);
    HAL_TRACE_DEBUG2("cpupr allocated pi: {}, slot_addr: {:#x} page: {:#x}",
                    cpu_tx_page_pindex, slot_addr, *page_addr);
    cpu_tx_page_pindex++;
    return HAL_RET_OK;
}

hal_ret_t
cpupkt_program_descr (uint32_t qid, cpupkt_hw_id_t page_addr, int offset, size_t len,
                      cpupkt_hw_id_t* descr_addr, bool no_copy,
              cpupkt_hw_id_t phy_page_addr)
{
    hal_ret_t       ret = HAL_RET_OK;
    pd_descr_aol_t  descr = {0}, *descr_p;
    pd_cpupkt_descr_alloc_args_t args;
    pd_func_args_t pd_func_args = {0};

    /*
     * If in zero-copy mode, the page-address we have is the virtual page-address of
     * the packet from CPU-RX-DPR entry, from which the descriptor-address is at a
     * fixed offset above it. We will update the AOL fields in that descriptor
     * to the page offset/lengths we want to transmit out now.
     */
    if (no_copy) {
        descr_p = (pd_descr_aol_t *) (page_addr - ASIC_CPU_RX_DPR_PAGE_OFFSET);
    *descr_addr = (cpupkt_hw_id_t)(phy_page_addr - ASIC_CPU_RX_DPR_PAGE_OFFSET);
    HAL_TRACE_DEBUG2("programming zero-copy descr: descr_addr: {:#x}, virt-addr {:#x}, offset {:#x}, len {}", *descr_addr,
            (uint64_t) descr_p, offset, len);

    } else {
        args.descr_addr = descr_addr;
    pd_func_args.pd_cpupkt_descr_alloc = &args;
    ret = pd_cpupkt_descr_alloc(&pd_func_args);
    if(ret != HAL_RET_OK) {
        HAL_TRACE_ERR2("failed to allocate descr for the packet, err: {}", ret);
        goto cleanup;
    }
    descr_p = &descr;
    HAL_TRACE_DEBUG2("programming descr: descr_addr: {:#x}, offset {:#x}, len {}", *descr_addr, offset, len);
    }

    descr_p->a0 = no_copy ? phy_page_addr : page_addr;
    descr_p->o0 = offset;
    descr_p->l0 = len;

    /*
     * If not in zero-copy mode, we'll write the descriptor to the physical address using the
     * asic_mem_write() API.
     */
    if (!no_copy) {
        if (asic_mem_write(*descr_addr, (uint8_t *)&descr,
               sizeof(pd_descr_aol_t),
               ASIC_WRITE_MODE_WRITE_THRU) != sdk::SDK_RET_OK) {
            HAL_TRACE_ERR2("failed to program descr");
        ret = HAL_RET_HW_FAIL;
        goto cleanup;
    }
    } else {

        /*
     * Ensure the programming of the descriptor/senq-queue is completed with a
     * memory barrier, before ringing the txdma doorbell.
     */
        PAL_barrier();
    }

    ret = HAL_RET_OK;

cleanup:
    return ret;

}

hal_ret_t
cpupkt_program_send_queue (cpupkt_ctxt_t* ctxt, types::WRingType type,
                           uint32_t queue_id, cpupkt_hw_id_t descr_addr, bool no_copy)
{
    if (!ctxt) {
        return HAL_RET_INVALID_ARG;
    }
    cpupkt_qinst_info_t* qinst_info = ctxt->tx.queue[type].qinst_info[queue_id];
    if(!qinst_info) {
        HAL_TRACE_ERR2("qinst for type: {} qid: {} is not registered", type, queue_id);
        return HAL_RET_QUEUE_NOT_FOUND;
    }

    /*
     * If zero-copy mode, we'll write the descriptor-address to the send-queue(ASQ)'s slot
     * using its virtual address.
     */
    if (no_copy && qinst_info->virt_pc_index_addr) {
        *((uint64_t *)qinst_info->virt_pc_index_addr) = descr_addr;
    HAL_TRACE_DEBUG2("Programming zero-copy send queue: addr: {:#x} virt-addr: {:#x} value: {:#x}",
            qinst_info->pc_index_addr, (uint64_t) qinst_info->virt_pc_index_addr, descr_addr);
    } else {
        if (asic_mem_write(qinst_info->pc_index_addr,
               (uint8_t *)&descr_addr,
               sizeof(cpupkt_hw_id_t),
               ASIC_WRITE_MODE_WRITE_THRU) != sdk::SDK_RET_OK) {
            HAL_TRACE_ERR2("Failed to program send queue");
        return HAL_RET_HW_FAIL;
    }
    HAL_TRACE_VERBOSE("Programming send queue: addr: {:#x} value: {:#x}",
                      qinst_info->pc_index_addr, descr_addr);
    }

    return HAL_RET_OK;
}

hal_ret_t
pd_cpupkt_program_send_ring_doorbell (pd_func_args_t *pd_func_args)
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

    HAL_TRACE_DEBUG2("ringing Doorbell with addr: {:#x} data: {:#x} lif: {} qtype: {} qid: {} ring: {} pidx: {}",
                    addr, data, dest_lif, qtype, qid, ring_number, args->pidx);
    asic_ring_doorbell(addr, data, ASIC_WRITE_MODE_WRITE_THRU);
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
pd_cpupkt_get_global (pd_func_args_t *pd_func_args)
{
    pd_cpupkt_get_global_args_t *args = pd_func_args->pd_cpupkt_get_global;
    args->gc_pindex = gc_pindex;
    args->cpu_tx_page_pindex = cpu_tx_page_pindex;
    args->cpu_tx_page_cindex = cpu_tx_page_cindex;
    args->cpu_tx_page_full_err = cpu_tx_page_full_err;
    args->cpu_tx_descr_pindex = cpu_tx_descr_pindex;
    args->cpu_tx_descr_cindex = cpu_tx_descr_cindex;
    args->cpu_tx_descr_full_err = cpu_tx_descr_full_err;
    args->cpu_rx_dpr_cindex = cpu_rx_dpr_cindex;
    args->cpu_rx_dpr_sem_cindex = cpu_rx_dpr_sem_cindex;
    args->cpu_rx_dpr_sem_free_err = cpu_rx_dpr_sem_free_err;
    args->cpu_rx_dpr_descr_free_err = cpu_rx_dpr_descr_free_err;
    args->cpu_rx_dpr_descr_invalid_free_err = cpu_rx_dpr_descr_invalid_free_err;
    return HAL_RET_OK;
}

hal_ret_t
pd_cpupkt_send (pd_func_args_t *pd_func_args)
{
    hal_ret_t               ret = HAL_RET_OK;
    pd_cpupkt_send_args_t  *s_args = pd_func_args->pd_cpupkt_send;
    cpupkt_hw_id_t          page_addr = 0;
    cpupkt_hw_id_t          descr_addr = 0;
    cpupkt_hw_id_t          write_addr = 0;
    size_t                  write_len = 0;
    size_t                  total_len = 0;
    cpupkt_ctxt_t*          ctxt = s_args->ctxt;
    types::WRingType        type = s_args->type;
    uint32_t                queue_id = s_args->queue_id;
    cpu_to_p4plus_header_t *cpu_header = s_args->cpu_header;
    p4plus_to_p4_header_t  *p4_header = s_args->p4_header;
    uint8_t                *data = s_args->data;
    size_t                  data_len = s_args->data_len;
    uint16_t                dest_lif = s_args->dest_lif;
    uint8_t                 qtype = s_args->qtype;
    uint32_t                qid = s_args->qid;
    uint8_t                 ring_number = s_args->ring_number;
    pd_func_args_t          pd_func_args1 = {0};
    cpupkt_hw_id_t          phy_page_addr = 0;
    bool                    no_copy;
    int                     rx_dpr_offset = 0;

    if(!ctxt || !data) {
        return HAL_RET_INVALID_ARG;
    }
    cpupkt_qinst_info_t &qinst_info = *(ctxt->tx.queue[type].qinst_info[queue_id]);

    /*
     * We'll do a zero-copy send if the platform has zero-copy enabled (HW/Haps), and
     * the packet page we have is an CPU-RX-DPR entry which is a combined descriptor-page
     * entry we received from P4+ in cpupkt_poll_receive() and we want to transmit the
     * same received packet back out.
     * If it is a packet originated at the CPU, we'll have to allocate a TX descriptor/page
     * here and copy the contents accordingly before we send it out.
     */
    no_copy = (is_cpu_zero_copy_enabled() &&
           is_virtaddr_cpu_rx_dpr_page((cpupkt_hw_id_t) data, queue_id, &phy_page_addr));

    HAL_TRACE_VERBOSE("Doing {} Tx of packet on LIF/Qtype/ID/ring {}/{}/{}/{}",
                      no_copy ? "zero-copy" : "",
                      dest_lif,
                      qtype, qid, ring_number);

    if (no_copy) {

        /*
     * If it is an RX-DPR packet, the packet actually contains a 'p4_to_p4plus_cpu_pkt_t'
     * header before it, which we'll rewind to to get the page-address.
     * Also, when we're transmitting this packet out, we want to write the
     * 'cpu_to_p4plus_header_t' and 'p4plus_to_p4_header_t' headers before the payload,
     * so we'll adjust the offset of where to write accordingly.
         */
        page_addr = (cpupkt_hw_id_t) (data - sizeof(p4_to_p4plus_cpu_pkt_t));
        phy_page_addr -= sizeof(p4_to_p4plus_cpu_pkt_t);
        rx_dpr_offset = (uint64_t)(sizeof(p4_to_p4plus_cpu_pkt_t) - (sizeof(cpu_to_p4plus_header_t)
                                     + sizeof(p4plus_to_p4_header_t)));
        write_addr = page_addr + rx_dpr_offset;
    }

    if (!page_addr) {

        // This is a CPU-originated packet. Allocate a page and descr for the pkt
        pd_cpupkt_page_alloc_args_t args;
    args.page_addr = &page_addr;
    pd_func_args1.pd_cpupkt_page_alloc = &args;
    ret = pd_cpupkt_page_alloc(&pd_func_args1);
    if (ret != HAL_RET_OK) {
        qinst_info.ctr.tx_page_alloc_err++;
        HAL_TRACE_ERR2("failed to allocate page for the packet, err: {}", ret);
        goto cleanup;
    }
    HAL_TRACE_DEBUG2("Allocated page at address {:#x}", page_addr);
    write_addr = page_addr;
    }

    HAL_TRACE_VERBOSE("CPU page-addr {:#x}, phy-page-addr {:#x} write-addr {:#x}",
            page_addr, phy_page_addr, write_addr);
    if (cpu_header != NULL) {
        // CPU header
        // update l2 header offset to include headers
        cpu_header->l2_offset += (sizeof(cpu_to_p4plus_header_t) +
                                  sizeof(p4plus_to_p4_header_t) +
                                  L2HDR_DOT1Q_OFFSET);

        cpu_header->flags |= CPU_TO_P4PLUS_FLAGS_FREE_BUFFER;

        write_len = sizeof(cpu_to_p4plus_header_t);
        total_len += write_len;

    if (no_copy) {

        /*
         * The 'write_addr' is the virtual address of the page offset where
         * we want to Write the cpu-to-p4plus header directly at.
         */
            cpu_to_p4plus_header_t *cpu_hdr_p = (cpu_to_p4plus_header_t *) write_addr;
        *cpu_hdr_p = *cpu_header;
    } else {
        HAL_TRACE_DEBUG2("Copying CPU header of len: {} to addr: {:#x}, l2offset: {:#x}",
                write_len, write_addr, cpu_header->l2_offset);
        if (asic_mem_write(write_addr, (uint8_t *)cpu_header, write_len,
                   ASIC_WRITE_MODE_WRITE_THRU) != sdk::SDK_RET_OK) {
                qinst_info.ctr.tx_page_copy_err++;
        HAL_TRACE_ERR2("Failed to copy packet to the page");
        ret = HAL_RET_HW_FAIL;
        goto cleanup;
        }
    }

    // P4plus_to_p4_header_t
    write_addr += write_len; // shift address
    write_len = sizeof(p4plus_to_p4_header_t);
    pd_swizzle_header ((uint8_t *)p4_header, sizeof(p4plus_to_p4_header_t));
    total_len += write_len;

    if (no_copy) {

        /*
         * The 'write_addr' is the virtual address of the page offset where
         * we want to Write the p4plus-to-p4 header directly at.
         */
            p4plus_to_p4_header_t *p4_hdr_p = (p4plus_to_p4_header_t *) write_addr;
        *p4_hdr_p = *p4_header;
    } else {

        HAL_TRACE_DEBUG2("Copying P4Plus to P4 header of len: {} to addr: {:#x}",
                        write_len, write_addr);
        if (asic_mem_write(write_addr, (uint8_t *)p4_header, write_len,
                   ASIC_WRITE_MODE_WRITE_THRU) != sdk::SDK_RET_OK) {
                HAL_TRACE_ERR2("Failed to copy packet to the page");
        ret = HAL_RET_HW_FAIL;
        goto cleanup;
        }
    }

        write_addr += write_len;
    }

    if (no_copy) {

        /*
     * Zero-copy case, data is as-is that came in the Rx path. We just update the length.
         */
        total_len += data_len;
    } else {

        // Data
        write_len = data_len;
    total_len += write_len;
    HAL_TRACE_DEBUG2("copying data of len: {} to page at addr: {:#x}", write_len, write_addr);
    if (asic_mem_write(write_addr, data, write_len,
               ASIC_WRITE_MODE_WRITE_THRU) != sdk::SDK_RET_OK) {
        HAL_TRACE_ERR2("Failed to copy packet to the page");
        ret = HAL_RET_HW_FAIL;
        goto cleanup;
    }
    }

    HAL_TRACE_DEBUG2("total pkt len: {} to page at addr: {:#x}", total_len, page_addr);

    ret = cpupkt_program_descr(qid, page_addr, no_copy ? rx_dpr_offset : 0, total_len,
                   &descr_addr, no_copy, phy_page_addr);
    if(ret != HAL_RET_OK) {
        qinst_info.ctr.tx_descr_pgm_err++;
        HAL_TRACE_ERR2("Failed to program tx descr");
        goto cleanup;
    }

    // Program Queue
    ret = cpupkt_program_send_queue(ctxt, type, queue_id, descr_addr, no_copy);
    if(ret != HAL_RET_OK) {
        qinst_info.ctr.tx_send_err++;
        goto cleanup;
    }

    /*
     * Ensure the programming of the descriptor/senq-queue is completed with a
     * memory barrier, before ringing the txdma doorbell.
     */
    PAL_barrier();

    // Ring doorbell
    pd_cpupkt_program_send_ring_doorbell_args_t d_args;
    d_args.dest_lif = dest_lif;
    d_args.qtype = qtype;
    d_args.qid = qid;
    d_args.ring_number = ring_number;
    d_args.flags = DB_IDX_UPD_PIDX_SET | DB_SCHED_UPD_SET;
    d_args.pidx = (qinst_info.ctr.send_pkts + 1) % CPU_PINDEX_MAX;
    pd_func_args1.pd_cpupkt_program_send_ring_doorbell = &d_args;
    ret = pd_cpupkt_program_send_ring_doorbell(&pd_func_args1);
    if(ret != HAL_RET_OK) {
        qinst_info.ctr.tx_db_err++;
        goto cleanup;
    }
    cpupkt_inc_queue_index(*(ctxt->tx.queue[type].qinst_info[queue_id]));
    qinst_info.ctr.send_pkts++;
    return HAL_RET_OK;

cleanup:

    // FIXME
    return ret;
}

hal_ret_t
pd_cpupkt_send_new (pd_func_args_t *pd_func_args)
{
    hal_ret_t               ret = HAL_RET_OK;
    pd_cpupkt_send_new_args_t  *s_args = pd_func_args->pd_cpupkt_send_new;
    cpupkt_ctxt_t*          ctxt;
    types::WRingType        type;
    uint32_t                queue_id;
    cpu_to_p4plus_header_t *cpu_header;
    p4plus_to_p4_header_t  *p4_header;
    uint8_t                *data;
    size_t                  data_len;
    uint16_t                dest_lif;
    uint8_t                 qtype;
    uint32_t                qid;
    uint8_t                 ring_number;
    pd_func_args_t          pd_func_args1 = {0};
    cpupkt_hw_id_t          phy_page_addr;
    bool                    no_copy;
    int                     rx_dpr_offset;
    cpupkt_qinst_info_t     *qinst_info;
    cpupkt_hw_id_t          page_addr;
    cpupkt_hw_id_t          descr_addr;
    cpupkt_hw_id_t          write_addr;
    size_t                  write_len;
    size_t                  total_len;
    uint16_t                send_pkts = 0;

    HAL_TRACE_DEBUG2("Batch send: {} packets", s_args->pkt_batch.pktcount);

    for (uint16_t npkt = 0; npkt < s_args->pkt_batch.pktcount; npkt++) {

        ctxt = s_args->pkt_batch.pkts[npkt].ctxt;
    type = s_args->pkt_batch.pkts[npkt].type;
    queue_id = s_args->pkt_batch.pkts[npkt].queue_id;
    cpu_header = s_args->pkt_batch.pkts[npkt].cpu_header;
    p4_header = s_args->pkt_batch.pkts[npkt].p4_header;
    data = s_args->pkt_batch.pkts[npkt].data;
    data_len = s_args->pkt_batch.pkts[npkt].data_len;
    dest_lif = s_args->pkt_batch.pkts[npkt].dest_lif;
    qtype = s_args->pkt_batch.pkts[npkt].qtype;
    qid = s_args->pkt_batch.pkts[npkt].qid;
    ring_number = s_args->pkt_batch.pkts[npkt].ring_number;

    phy_page_addr = 0;
    rx_dpr_offset = 0;
        page_addr = descr_addr = write_addr = 0;
    write_len = total_len = 0;

    if(!ctxt || !data) {
       return HAL_RET_INVALID_ARG;
    }
    qinst_info = ctxt->tx.queue[type].qinst_info[queue_id];

    /*
     * We'll do a zero-copy send if the platform has zero-copy enabled (HW/Haps), and
     * the packet page we have is an CPU-RX-DPR entry which is a combined descriptor-page
     * entry we received from P4+ in cpupkt_poll_receive() and we want to transmit the
     * same received packet back out.
     * If it is a packet originated at the CPU, we'll have to allocate a TX descriptor/page
     * here and copy the contents accordingly before we send it out.
     */
    no_copy = (is_cpu_zero_copy_enabled() &&
           is_virtaddr_cpu_rx_dpr_page((cpupkt_hw_id_t) data, queue_id, &phy_page_addr));

    HAL_TRACE_DEBUG2("Doing {} Tx of packet on LIF/Qtype/ID/ring {}/{}/{}/{}",
            no_copy ? "zero-copy" : "", dest_lif, qtype, qid, ring_number);

    if (no_copy) {

        /*
         * If it is an RX-DPR packet, the packet actually contains a 'p4_to_p4plus_cpu_pkt_t'
         * header before it, which we'll rewind to to get the page-address.
         * Also, when we're transmitting this packet out, we want to write the
         * 'cpu_to_p4plus_header_t' and 'p4plus_to_p4_header_t' headers before the payload,
         * so we'll adjust the offset of where to write accordingly.
         */
        page_addr = (cpupkt_hw_id_t) (data - sizeof(p4_to_p4plus_cpu_pkt_t));
        phy_page_addr -= sizeof(p4_to_p4plus_cpu_pkt_t);
        rx_dpr_offset = (uint64_t)(sizeof(p4_to_p4plus_cpu_pkt_t) - (sizeof(cpu_to_p4plus_header_t)
                                     + sizeof(p4plus_to_p4_header_t)));
        write_addr = page_addr + rx_dpr_offset;
    }

    if (!page_addr) {

        // This is a CPU-originated packet. Allocate a page and descr for the pkt
        pd_cpupkt_page_alloc_args_t args;
        args.page_addr = &page_addr;
        pd_func_args1.pd_cpupkt_page_alloc = &args;
        ret = pd_cpupkt_page_alloc(&pd_func_args1);
        if (ret != HAL_RET_OK) {
            qinst_info->ctr.tx_page_alloc_err++;
        HAL_TRACE_ERR2("failed to allocate page for the packet, err: {}", ret);
        goto cleanup;
        }
        HAL_TRACE_DEBUG2("Allocated page at address {:#x}", page_addr);
        write_addr = page_addr;
    }

    HAL_TRACE_DEBUG2("CPU page-addr {:#x}, phy-page-addr {:#x} write-addr {:#x}",
            page_addr, phy_page_addr, write_addr);
    if (cpu_header != NULL) {
            // CPU header
        // update l2 header offset to include headers
        cpu_header->l2_offset += (sizeof(cpu_to_p4plus_header_t) +
                      sizeof(p4plus_to_p4_header_t) +
                      L2HDR_DOT1Q_OFFSET);

        cpu_header->flags |= CPU_TO_P4PLUS_FLAGS_FREE_BUFFER;

        write_len = sizeof(cpu_to_p4plus_header_t);
        total_len += write_len;

        if (no_copy) {

            /*
         * The 'write_addr' is the virtual address of the page offset where
         * we want to Write the cpu-to-p4plus header directly at.
         */
            cpu_to_p4plus_header_t *cpu_hdr_p = (cpu_to_p4plus_header_t *) write_addr;
        *cpu_hdr_p = *cpu_header;
        } else {
            HAL_TRACE_DEBUG2("Copying CPU header of len: {} to addr: {:#x}, l2offset: {:#x}",
                write_len, write_addr, cpu_header->l2_offset);
        if (asic_mem_write(write_addr, (uint8_t *)cpu_header, write_len,
                   ASIC_WRITE_MODE_WRITE_THRU) != sdk::SDK_RET_OK) {
            qinst_info->ctr.tx_page_copy_err++;
            HAL_TRACE_ERR2("Failed to copy packet to the page");
            ret = HAL_RET_HW_FAIL;
            goto cleanup;
        }
        }

        // P4plus_to_p4_header_t
        write_addr += write_len; // shift address
        write_len = sizeof(p4plus_to_p4_header_t);
        pd_swizzle_header ((uint8_t *)p4_header, sizeof(p4plus_to_p4_header_t));
        total_len += write_len;

        if (no_copy) {

            /*
         * The 'write_addr' is the virtual address of the page offset where
         * we want to Write the p4plus-to-p4 header directly at.
         */
            p4plus_to_p4_header_t *p4_hdr_p = (p4plus_to_p4_header_t *) write_addr;
        *p4_hdr_p = *p4_header;
        } else {

            HAL_TRACE_DEBUG2("Copying P4Plus to P4 header of len: {} to addr: {:#x}",
                write_len, write_addr);
        if (asic_mem_write(write_addr, (uint8_t *)p4_header, write_len,
                   ASIC_WRITE_MODE_WRITE_THRU) != sdk::SDK_RET_OK) {
          HAL_TRACE_ERR2("Failed to copy packet to the page");
          ret = HAL_RET_HW_FAIL;
          goto cleanup;
        }
        }

        write_addr += write_len;
    }

    if (no_copy) {

        /*
         * Zero-copy case, data is as-is that came in the Rx path. We just update the length.
         */
        total_len += data_len;
    } else {

        // Data
        write_len = data_len;
        total_len += write_len;
        HAL_TRACE_DEBUG2("copying data of len: {} to page at addr: {:#x}", write_len, write_addr);
        if (asic_mem_write(write_addr, data, write_len,
                   ASIC_WRITE_MODE_WRITE_THRU) != sdk::SDK_RET_OK) {
            HAL_TRACE_ERR2("Failed to copy packet to the page");
        ret = HAL_RET_HW_FAIL;
        goto cleanup;
        }
    }

    HAL_TRACE_DEBUG2("total pkt len: {} to page at addr: {:#x}", total_len, page_addr);

    ret = cpupkt_program_descr(qid, page_addr, no_copy ? rx_dpr_offset : 0, total_len,
                   &descr_addr, no_copy, phy_page_addr);
    if(ret != HAL_RET_OK) {
       qinst_info->ctr.tx_descr_pgm_err++;
       HAL_TRACE_ERR2("Failed to program tx descr");
       goto cleanup;
    }

    // Program Queue
    ret = cpupkt_program_send_queue(ctxt, type, queue_id, descr_addr, no_copy);
    if (ret != HAL_RET_OK) {
       qinst_info->ctr.tx_send_err++;
       goto cleanup;
    }

    cpupkt_inc_queue_index(*(ctxt->tx.queue[type].qinst_info[queue_id]));
    send_pkts++;

    } // for loop

    /*
     * Ensure the programming of the descriptor/senq-queue is completed with a
     * memory barrier, before ringing the txdma doorbell.
     */
    PAL_barrier();

    // Ring doorbell
    pd_cpupkt_program_send_ring_doorbell_args_t d_args;
    d_args.dest_lif = dest_lif;
    d_args.qtype = qtype;
    d_args.qid = qid;
    d_args.ring_number = ring_number;
    d_args.flags = DB_IDX_UPD_PIDX_SET | DB_SCHED_UPD_SET;
    d_args.pidx = (qinst_info->ctr.send_pkts + send_pkts) % CPU_PINDEX_MAX;
    pd_func_args1.pd_cpupkt_program_send_ring_doorbell = &d_args;
    ret = pd_cpupkt_program_send_ring_doorbell(&pd_func_args1);
    if(ret != HAL_RET_OK) {
        qinst_info->ctr.tx_db_err++;
        goto cleanup;
    }
    qinst_info->ctr.send_pkts += send_pkts;

    return HAL_RET_OK;

cleanup:

    // FIXME
    return ret;
}

} // namespace pd
} // namespace hal
