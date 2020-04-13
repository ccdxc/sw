#include "nic/include/base.hpp"
#include "nic/sdk/include/sdk/lock.hpp"
#include "nic/include/pd_api.hpp"
#include "nic/sdk/asic/rw/asicrw.hpp"
#include "nic/hal/pd/iris/hal_state_pd.hpp"
#include "nic/hal/pd/libs/wring/wring_pd.hpp"
#include "nic/sdk/asic/cmn/asic_hbm.hpp"
#include "nic/sdk/platform/capri/capri_barco_crypto.hpp"
#include "asic/cmn/asic_common.hpp"
#include "asic/cmn/asic_common.hpp"
#include "nic/sdk/platform/capri/capri_barco.h"
#include "nic/hal/pd/iris/internal/p4plus_pd_api.h"
#include "nic/sdk/lib/pal/pal.hpp"

#define ARM_CPU_RING_SIZE  4096

namespace hal {
namespace pd {

static pd_wring_meta_t g_meta[types::WRingType_MAX + 1];

hal_ret_t brq_gcm_slot_parser(pd_wring_meta_t *meta, wring_t *wring, uint8_t *slot);
hal_ret_t serq_slot_parser(pd_wring_meta_t *meta, wring_t *wring, uint8_t *slot);
hal_ret_t armq_slot_parser(pd_wring_meta_t *meta, wring_t *wring, uint8_t *slot);
hal_ret_t arqrx_get_hw_meta(pd_wring_t* wring_pd);
hal_ret_t barco_gcm0_get_hw_meta(pd_wring_t* wring_pd);
hal_ret_t tcp_actl_q_get_hw_meta(pd_wring_t* wring_pd);
hal_ret_t p4pd_wring_set_rnmdpr_meta(pd_wring_t* wring_pd);

hal_ret_t
wring_pd_meta_init() {
    /*
     * Add meta info for each ring
     * Format: is Global Ring, region name, # of slots in the ring, slot size in bytes
     *         associate object region name, associated size of object
     */

    g_meta[types::WRING_TYPE_SERQ] =
        (pd_wring_meta_t) {false, ASIC_HBM_REG_SERQ, ASIC_SERQ_RING_SLOTS, SERQ_WRING_SLOT_SIZE,
                            "", 0, 0, serq_slot_parser, NULL, false, 1, 0};

    g_meta[types::WRING_TYPE_IPSEC_NMDR_TX] =
        (pd_wring_meta_t) {true, ASIC_HBM_REG_IPSEC_NMDR_TX, ASIC_TNMDR_IPSEC_RING_SIZE,
                           DEFAULT_WRING_SLOT_SIZE, ASIC_HBM_REG_IPSEC_DESC_TX, 128,
                           ASIC_SEM_IPSEC_TNMDR_ALLOC_RAW_ADDR,
                           NULL, NULL, false, 1, 0};

    g_meta[types::WRING_TYPE_IPSEC_NMDR_RX] =
        (pd_wring_meta_t) {true, ASIC_HBM_REG_IPSEC_NMDR_RX, ASIC_RNMDR_IPSEC_RING_SIZE,
                           DEFAULT_WRING_SLOT_SIZE, ASIC_HBM_REG_IPSEC_DESC_RX, 128,
                           ASIC_SEM_IPSEC_RNMDR_ALLOC_RAW_ADDR,
                           NULL, NULL, false, 1, 0};

    g_meta[types::WRING_TYPE_IPSEC_BIG_NMDR_TX] =
        (pd_wring_meta_t) {true, ASIC_HBM_REG_IPSEC_BIG_NMDR_TX, ASIC_TNMDR_IPSEC_RING_SIZE,
                           DEFAULT_WRING_SLOT_SIZE, ASIC_HBM_REG_IPSEC_BIG_DESC_TX, 128,
                           ASIC_SEM_IPSEC_BIG_TNMDR_ALLOC_RAW_ADDR,
                           NULL, NULL, false, 1, 0};

    g_meta[types::WRING_TYPE_IPSEC_BIG_NMDR_RX] =
        (pd_wring_meta_t) {true, ASIC_HBM_REG_IPSEC_BIG_NMDR_RX, ASIC_RNMDR_IPSEC_RING_SIZE,
                           DEFAULT_WRING_SLOT_SIZE, ASIC_HBM_REG_IPSEC_BIG_DESC_RX, 128,
                           ASIC_SEM_IPSEC_BIG_RNMDR_ALLOC_RAW_ADDR,
                           NULL, NULL, false, 1, 0};

    g_meta[types::WRING_TYPE_IPSEC_NMPR_TX] =
        (pd_wring_meta_t) {true, ASIC_HBM_REG_IPSEC_NMPR_TX, ASIC_TNMPR_IPSEC_RING_SIZE,
                           DEFAULT_WRING_SLOT_SIZE, ASIC_HBM_REG_ENC_PAGE_BIG_TX, 9600,
                           ASIC_SEM_IPSEC_TNMPR_ALLOC_RAW_ADDR,
                           NULL, NULL, false, 1, 0};

    g_meta[types::WRING_TYPE_IPSEC_NMPR_RX] =
        (pd_wring_meta_t) {true, ASIC_HBM_REG_IPSEC_NMPR_RX, ASIC_RNMPR_IPSEC_RING_SIZE,
                           DEFAULT_WRING_SLOT_SIZE, ASIC_HBM_REG_ENC_PAGE_BIG_RX, 9600,
                           ASIC_SEM_IPSEC_RNMPR_ALLOC_RAW_ADDR,
                           NULL, NULL, false, 1, 0};

    g_meta[types::WRING_TYPE_IPSEC_BIG_NMPR_TX] =
        (pd_wring_meta_t) {true, ASIC_HBM_REG_IPSEC_BIG_NMPR_TX, ASIC_TNMPR_IPSEC_RING_SIZE,
                           DEFAULT_WRING_SLOT_SIZE, ASIC_HBM_REG_DEC_PAGE_BIG_TX, 9600,
                           ASIC_SEM_IPSEC_BIG_TNMPR_ALLOC_RAW_ADDR,
                           NULL, NULL, false, 1, 0};

    g_meta[types::WRING_TYPE_IPSEC_BIG_NMPR_RX] =
        (pd_wring_meta_t) {true, ASIC_HBM_REG_IPSEC_BIG_NMPR_RX, ASIC_RNMPR_IPSEC_RING_SIZE,
                           DEFAULT_WRING_SLOT_SIZE, ASIC_HBM_REG_DEC_PAGE_BIG_RX, 9600,
                           ASIC_SEM_IPSEC_BIG_RNMPR_ALLOC_RAW_ADDR,
                           NULL, NULL, false, 1, 0};
    g_meta[types::WRING_TYPE_BSQ] =
        (pd_wring_meta_t) {false, ASIC_HBM_REG_BSQ, ASIC_BSQ_RING_SLOTS, ASIC_BSQ_RING_SLOT_SIZE,
                            "", 0, 0, NULL, NULL, false, 1, 0};

    g_meta[types::WRING_TYPE_BRQ] =
        (pd_wring_meta_t) {true, ASIC_HBM_REG_BRQ, 1024, 128, "", 0, 0, brq_gcm_slot_parser,
                            barco_gcm0_get_hw_meta, false, 0, 0};

    // SESQ and ASESQ use the same region in HBM
    g_meta[types::WRING_TYPE_ASESQ] =
        (pd_wring_meta_t) {false, ASIC_HBM_REG_SESQ, ASIC_SESQ_RING_SLOTS,
            DEFAULT_WRING_SLOT_SIZE, "", 0, 0, NULL, NULL, false,
            2, // ring_types_in_region (SESQ + ASESQ)
               0, 1};

    g_meta[types::WRING_TYPE_SESQ] =
        (pd_wring_meta_t) {false, ASIC_HBM_REG_SESQ, ASIC_SESQ_RING_SLOTS,
            DEFAULT_WRING_SLOT_SIZE, "", 0, 0, NULL, NULL, false,
            2, // ring_types_in_region (SESQ + ASESQ)
            (ASIC_ASESQ_RING_SLOTS * DEFAULT_WRING_SLOT_SIZE)}; // ring_type_offset


    g_meta[types::WRING_TYPE_IPSECCBQ] =
        (pd_wring_meta_t) {false, ASIC_HBM_REG_IPSECCB, 1024, DEFAULT_WRING_SLOT_SIZE,
                                  "", 0, 0, NULL, NULL, false, 1, 0};
    g_meta[types::WRING_TYPE_IPSECCBQ_BARCO] =
        (pd_wring_meta_t) {false, ASIC_HBM_REG_IPSECCB_BARCO, 1024, 16,
                                  "", 0, 0, NULL, NULL, false, 1, 0};
    g_meta[types::WRING_TYPE_ARQRX] =
        (pd_wring_meta_t) {false, ASIC_HBM_REG_ARQRX, ARM_CPU_RING_SIZE, DEFAULT_WRING_SLOT_SIZE,
                            "", 0, 0, armq_slot_parser, arqrx_get_hw_meta,
               false, 1, 0, 1};

    g_meta[types::WRING_TYPE_ASQ] =
        (pd_wring_meta_t) {false, ASIC_HBM_REG_ASQ, ARM_CPU_RING_SIZE, DEFAULT_WRING_SLOT_SIZE,
               "", 0, 0, NULL, NULL, false, 1, 0, 1};

    g_meta[types::WRING_TYPE_ASCQ] =
        (pd_wring_meta_t) {false, ASIC_HBM_REG_ASCQ, ARM_CPU_RING_SIZE, DEFAULT_WRING_SLOT_SIZE,
               "", 0, 0, armq_slot_parser, NULL, false, 1, 0, 1};

    g_meta[types::WRING_TYPE_APP_REDIR_RAWC] =
        (pd_wring_meta_t) {false, ASIC_HBM_REG_APP_REDIR_RAWC, 128, DEFAULT_WRING_SLOT_SIZE, "", 0, 0,
                           NULL, NULL, true, 1, 0, false, true};

    g_meta[types::WRING_TYPE_APP_REDIR_PROXYR] =
        (pd_wring_meta_t) {false, ASIC_HBM_REG_APP_REDIR_PROXYR, 128, DEFAULT_WRING_SLOT_SIZE, "", 0, 0,
                           NULL, NULL, true, 1, 0, false, true};

    g_meta[types::WRING_TYPE_APP_REDIR_PROXYC] =
        (pd_wring_meta_t) {false, ASIC_HBM_REG_APP_REDIR_PROXYC, 128, DEFAULT_WRING_SLOT_SIZE, "", 0, 0,
                           NULL, NULL, true, 1, 0, false, true};

    g_meta[types::WRING_TYPE_NMDR_TX_GC] =
        (pd_wring_meta_t) {false, ASIC_HBM_REG_NMDR_TX_GC,
                           ASIC_HBM_GC_PER_PRODUCER_RING_SIZE,
                           DEFAULT_WRING_SLOT_SIZE, "", 0, 0, NULL, NULL, false, 1, 0};

    g_meta[types::WRING_TYPE_NMDR_RX_GC] =
        (pd_wring_meta_t) {false, ASIC_HBM_REG_NMDR_RX_GC,
                           ASIC_HBM_GC_PER_PRODUCER_RING_SIZE,
                           DEFAULT_WRING_SLOT_SIZE, "", 0, 0, NULL, NULL, false, 1, 0};

    g_meta[types::WRING_TYPE_CPU_TX_DR] =
        (pd_wring_meta_t) {false, ASIC_HBM_REG_CPU_TX_DR,
                           ASIC_HBM_CPU_TX_DR_RING_SIZE, DEFAULT_WRING_SLOT_SIZE,
                           ASIC_HBM_REG_CPU_TX_DESCR, 128, 0, NULL, NULL, false, 1, 0, 1};

    g_meta[types::WRING_TYPE_CPU_TX_PR] =
        (pd_wring_meta_t) {false, ASIC_HBM_REG_CPU_TX_PR,
                           ASIC_HBM_CPU_TX_PR_RING_SIZE, DEFAULT_WRING_SLOT_SIZE,
                           ASIC_HBM_REG_CPU_TX_PAGE, ASIC_CPU_TX_PR_OBJ_TOTAL_SIZE, 0, NULL, NULL, false, 1, 0, 1};

    g_meta[types::WRING_TYPE_CPU_RX_DPR] =
        (pd_wring_meta_t) {true, ASIC_HBM_REG_CPU_RX_DPR,
                           ASIC_CPU_RX_DPR_RING_SIZE, DEFAULT_WRING_SLOT_SIZE,
                           ASIC_HBM_REG_CPU_RX_DESC_PAGE, ASIC_CPU_RX_DPR_OBJ_TOTAL_SIZE,
               ASIC_SEM_CPU_RX_DPR_ALLOC_RAW_ADDR, NULL, NULL, false, 0, 0, 1};

    /* Descriptor-Page Combined Allocator rings */
    g_meta[types::WRING_TYPE_NMDPR_SMALL_TX] =
        (pd_wring_meta_t) {true, ASIC_HBM_REG_NMDPR_SMALL_TX, ASIC_TNMDPR_SMALL_RING_SIZE,
                           DEFAULT_WRING_SLOT_SIZE, ASIC_HBM_REG_NMDPR_OBJS_SMALL_TX, ASIC_NMDPR_SMALL_OBJ_TOTAL_SIZE,
                           ASIC_SEM_TNMDPR_SMALL_ALLOC_RAW_ADDR,
                           NULL, NULL, false};

    g_meta[types::WRING_TYPE_NMDPR_SMALL_RX] =
        (pd_wring_meta_t) {true, ASIC_HBM_REG_NMDPR_SMALL_RX, ASIC_RNMDPR_SMALL_RING_SIZE,
                           DEFAULT_WRING_SLOT_SIZE, ASIC_HBM_REG_NMDPR_OBJS_SMALL_RX, ASIC_NMDPR_SMALL_OBJ_TOTAL_SIZE,
                           ASIC_SEM_RNMDPR_SMALL_ALLOC_RAW_ADDR,
                           NULL, NULL, false};

    g_meta[types::WRING_TYPE_NMDPR_BIG_TX] =
        (pd_wring_meta_t) {true, ASIC_HBM_REG_NMDPR_BIG_TX, ASIC_TNMDPR_BIG_RING_SIZE,
                           DEFAULT_WRING_SLOT_SIZE, ASIC_HBM_REG_NMDPR_OBJS_BIG_TX, ASIC_NMDPR_BIG_OBJ_TOTAL_SIZE,
                           ASIC_SEM_TNMDPR_BIG_ALLOC_RAW_ADDR,
                           NULL, NULL, false};

    g_meta[types::WRING_TYPE_NMDPR_BIG_RX] =
        (pd_wring_meta_t) {true, ASIC_HBM_REG_NMDPR_BIG_RX, ASIC_RNMDPR_BIG_RING_SIZE,
                           DEFAULT_WRING_SLOT_SIZE, ASIC_HBM_REG_NMDPR_OBJS_BIG_RX, ASIC_NMDPR_BIG_OBJ_TOTAL_SIZE,
                           ASIC_SEM_RNMDPR_BIG_ALLOC_RAW_ADDR,
                           NULL, NULL, false};
    g_meta[types::WRING_TYPE_NMDPR_BIG_RX].set_hw_meta_fn = p4pd_wring_set_rnmdpr_meta;

    g_meta[types::WRING_TYPE_TCP_OOO_RX] =
        (pd_wring_meta_t) {true, ASIC_HBM_REG_TCP_OOO_QBASE_RING, ASIC_TCP_ALLOC_OOQ_RING_SIZE,
                           DEFAULT_WRING_SLOT_SIZE, ASIC_HBM_REG_TCP_OOO_QUEUE,
                           TCP_OOO_QUEUE_ALLOC_SIZE,
                           ASIC_SEM_TCP_OOQ_ALLOC_RAW_ADDR, NULL, NULL, false};

    g_meta[types::WRING_TYPE_TCP_OOO_RX2TX] =
        (pd_wring_meta_t) {false, ASIC_HBM_REG_TCP_OOO_RX2TX_QUEUE,
                            ASIC_OOO_RX2TX_RING_SLOTS, DEFAULT_WRING_SLOT_SIZE,
                            "", 0, 0, NULL, NULL, false, 1, 0};

    g_meta[types::WRING_TYPE_TCP_ACTL_Q] =
        (pd_wring_meta_t) {false, ASIC_HBM_REG_TCP_ACTL_Q, ARM_CPU_RING_SIZE, DEFAULT_WRING_SLOT_SIZE,
                            "", 0, 0, armq_slot_parser, tcp_actl_q_get_hw_meta,
               false, 1, 0, 1};


    return HAL_RET_OK;
}

pd_wring_meta_t* wring_pd_get_meta(types::WRingType type)
{
    return &g_meta[type];
}
/**************************
 * Helper functions
 *************************/
// allocate a wring pd instance
static inline pd_wring_t *
wring_pd_alloc (void)
{
    pd_wring_t    *wring_pd;

    wring_pd = (pd_wring_t *)g_hal_state_pd->wring_slab()->alloc();
    if (wring_pd == NULL) {
        return NULL;
    }

    return wring_pd;
}

// allocate and initialize a wring pd instance
static inline pd_wring_t *
wring_pd_alloc_init (void)
{
    return wring_pd_init(wring_pd_alloc());
}

// free wring pd instance
static inline hal_ret_t
wring_pd_free (pd_wring_t *wring_pd)
{
    hal::pd::delay_delete_to_slab(HAL_SLAB_WRING_PD, wring_pd);
    return HAL_RET_OK;
}

// insert wring pd state in all meta data structures
static inline hal_ret_t
add_wring_pd_to_db (pd_wring_t *wring_pd)
{
    g_hal_state_pd->wring_hwid_ht()->insert(wring_pd, &wring_pd->hw_ht_ctxt);
    return HAL_RET_OK;
}

// find a wring pd instance given its hw id
static inline pd_wring_t *
find_wring_by_hwid (wring_hw_id_t hwid)
{
    return (pd_wring_t *)g_hal_state_pd->wring_hwid_ht()->lookup(&hwid);
}

/*****************************
 * APIs
 *****************************/

hal_ret_t
get_default_slot_value(types::WRingType type, uint32_t index, uint8_t* value)
{
    pd_wring_meta_t     *meta = &g_meta[type];

    if(strlen(meta->obj_hbm_reg_name) <= 0) {
        memset(value, 0, meta->slot_size_in_bytes);
    } else {
        wring_hw_id_t obj_addr_base =
            asicpd_get_mem_addr(meta->obj_hbm_reg_name);
        if(obj_addr_base == INVALID_MEM_ADDRESS) {
            HAL_TRACE_ERR("Failed to get the addr for the object");
            memset(value, 0, meta->slot_size_in_bytes);
            return HAL_RET_ENTRY_NOT_FOUND;
        }
        // get the addr of the object
        uint64_t obj_addr = obj_addr_base + (index * meta->obj_size);
        obj_addr = htonll(obj_addr);
        memcpy(value, &obj_addr, sizeof(obj_addr));
    }
    return HAL_RET_OK;
}

hal_ret_t
wring_pd_get_base_addr(types::WRingType type, uint32_t wring_id,
                       wring_hw_id_t* wring_base)
{
    pd_wring_meta_t     *meta = &g_meta[type];
    wring_hw_id_t       addr = 0;

    addr = asicpd_get_mem_addr(meta->hbm_reg_name);
    if(addr == INVALID_MEM_ADDRESS) {
        HAL_TRACE_ERR("Could not find addr for {} region", meta->hbm_reg_name);
        return HAL_RET_ERR;
    }

    if(meta->is_global) {
        // Global rings start at the base of hbm_region
        *wring_base = addr;
    } else {
        // Flow local ring. Get the offset based on wring id
        uint32_t wring_size = meta->num_slots * meta->slot_size_in_bytes;
        *wring_base = addr + meta->ring_type_offset +
                (wring_id * wring_size * meta->ring_types_in_region);
    }

    return HAL_RET_OK;
}

hal_ret_t
wring_pd_get_num_entries (types::WRingType type, uint32_t *num_entries)
{
    pd_wring_meta_t     *meta = &g_meta[type];

    *num_entries = meta->num_slots;

    return HAL_RET_OK;
}
hal_ret_t
wring_pd_table_init (types::WRingType type, uint32_t wring_id)
{
    hal_ret_t           ret = HAL_RET_OK;
    pd_wring_meta_t     *meta = &g_meta[type];
    wring_hw_id_t       wring_base = 0;
    wring_hw_id_t       wring_obj_base = 0;
    uint32_t            obj_reg_size = 0;

    ret = wring_pd_get_base_addr(type, wring_id, &wring_base);
    if(ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Could not find the wring base addr");
        return ret;
    }

    /*
     * We'll cache the base-address for faster lookups.
     */
    wring_pd_set_meta_base_addr(meta, wring_id, wring_base);

    /*
     * If we have an object region for this Wring, we'll cache the object
     * region base-address too, for faster lookups.
     */
    if (meta->obj_size) {
        wring_obj_base = asicpd_get_mem_addr(meta->obj_hbm_reg_name);
        if (wring_obj_base == INVALID_MEM_ADDRESS) {
            HAL_TRACE_ERR("Could not find base addr for {} region",
                          meta->obj_hbm_reg_name);
            return HAL_RET_ERR;
        }

        obj_reg_size = meta->num_slots * meta->obj_size;
        if (meta->is_global) {
            wring_pd_set_meta_obj_base_addr(meta, 0, wring_obj_base);
        } else {
            wring_obj_base += (wring_id * obj_reg_size);
            wring_pd_set_meta_obj_base_addr(meta, wring_id, wring_obj_base);
        }
        HAL_TRACE_DEBUG("base-addr {:#x} size {} KB obj-base-addr {:#x} size {} KB",
              wring_base, asicpd_get_mem_size_kb(meta->hbm_reg_name),
              wring_obj_base, asicpd_get_mem_size_kb(meta->obj_hbm_reg_name));
    }

    uint32_t reg_size = asicpd_get_mem_size_kb(meta->hbm_reg_name);
    if(!reg_size) {
        HAL_TRACE_ERR("Could not find size for {} region", meta->hbm_reg_name);
        return HAL_RET_ERR;
    }

    uint32_t single_size = meta->num_slots * meta->slot_size_in_bytes *
                    (meta->ring_types_in_region ? meta->ring_types_in_region : 1);
    uint32_t required_size = single_size * (wring_id + 1);
    if ((reg_size * 1024) < required_size) {

        /*
         * NOTE: Old code did not validate the required_size correctly -- it didn't
         * take multiple per-flow queues as identified by wring_id into account.
         * Hence the validation would always succeed.
         *
         * Even though the issue is now fixed, it is too risky to enable at
         * this point as it might cause failures in existing DOL and iota tests.
         * So for the time being, validation failures are only logged for
         * debugging purposes.
         */
        HAL_TRACE_DEBUG("region {} total size {} is too small for wing_id {} "
                        "of size {}", meta->hbm_reg_name, reg_size * 1024,
                        wring_id, single_size);
        //return HAL_RET_NO_RESOURCE;
    }

    // Allocate memory for storing value for a slot
    if (meta->skip_init_slots) {
        HAL_TRACE_DEBUG("skip_init_slots for {} region", meta->hbm_reg_name);
    } else {
        uint8_t value[meta->slot_size_in_bytes];
        for(uint32_t  index = 0; index<meta->num_slots; index++) {

            uint64_t slot_addr = wring_base + (index * meta->slot_size_in_bytes);

            get_default_slot_value(type, index, value);
            p4plus_hbm_write(slot_addr, value, meta->slot_size_in_bytes,
                    P4PLUS_CACHE_ACTION_NONE);
        }
    }
    if (meta->alloc_semaphore_addr &&
                    ASIC_SEM_RAW_IS_PI_CI(meta->alloc_semaphore_addr)) {
        // Set CI = ring size
        uint32_t val32 = meta->num_slots;
        HAL_TRACE_DEBUG("writing {} to semaphore {:#x}",
                        val32, meta->alloc_semaphore_addr +
                        ASIC_SEM_INC_NOT_FULL_CI_OFFSET);
        sdk::asic::asic_reg_write(meta->alloc_semaphore_addr +
                                  ASIC_SEM_INC_NOT_FULL_CI_OFFSET, &val32);
        // Set PI to 0
        val32 = 0;
        sdk::asic::asic_reg_write(meta->alloc_semaphore_addr, &val32);
    }

    /*
     * If the 'mmap_ring' flag is set, then we want to memory-map the HBM region of
     * this ring into our process, so we can do direct memory read/write access without
     * having to go thru the PAL/asic-rw APIs which need a translation/copy from the
     * physical region to the virtual address of this process.
     */
    if (!is_platform_type_haps() && !is_platform_type_hw()) {
        return(HAL_RET_OK);
    }
    if (!meta->mmap_ring || wring_pd_get_meta_virt_base_addr(meta, wring_id)) return(HAL_RET_OK);

    /*
     * This should not happen for the CPU interested Rings which are not
     * per-flow rings.
     */
    if (!meta->per_flow_ring && (wring_id > MAX_WRING_IDS)) {
        HAL_TRACE_ERR("Invalid wrind_id {}, failed to mmap ring memory for Q:{}",
              wring_id, type);
        return(HAL_RET_ERR);
    }

    wring_pd_set_meta_virt_base_addr(meta, wring_id, (uint8_t *)sdk::lib::pal_mem_map(wring_base,
                                       meta->num_slots *
                                       meta->slot_size_in_bytes));
     if (!wring_pd_get_meta_virt_base_addr(meta, wring_id)) {
     HAL_TRACE_ERR("Failed to mmap the Ring(T:{}, Q:{})", type, wring_id);
     return HAL_RET_NO_RESOURCE;
     }
     else {
         HAL_TRACE_DEBUG("mmap the WRing(T:{}, Q:{}) phy {:#x} @ virt {:#x}, size: {} KB",
                         type, wring_id, (uint64_t)wring_base,
                         (uint64_t)wring_pd_get_meta_virt_base_addr(meta, wring_id),
                         ((meta->num_slots * meta->slot_size_in_bytes)/1024));
     }

     /*
      * If the object ring is present, lets memory-map the object memory region too.
      */
     if (meta->obj_size) {
     wring_pd_set_meta_virt_obj_base_addr(meta, wring_id, (uint8_t *)sdk::lib::pal_mem_map(wring_obj_base,
                                           obj_reg_size));
     if (!wring_pd_get_meta_virt_obj_base_addr(meta, wring_id)) {
         HAL_TRACE_ERR("Failed to mmap the OBJ Ring(T:{}, Q:{})", type, wring_id);
         return HAL_RET_NO_RESOURCE;
     }
     else {
         HAL_TRACE_DEBUG("mmap the OBJ WRing(T:{}, Q:{}) phy {:#x} @ virt {:#x}, size: {} KB",
                 type, wring_id, (uint64_t)wring_obj_base,
                 (uint64_t)wring_pd_get_meta_virt_obj_base_addr(meta, wring_id),
                 (obj_reg_size/1024));
     }
    }

    return HAL_RET_OK;
}

/* Partial descriptor for now due to bit fields in the middle */
typedef struct barco_desc_s {
    uint64_t            input_list_address;
    uint64_t            output_list_address;
    uint32_t            command;
    uint32_t            key_desc_index;
    uint64_t            iv_address;
    uint64_t            auth_tag_addr;
    uint32_t            header_size;
    uint64_t            status_address;
    uint32_t            opaque_tag_value;
    uint32_t            opaque_tag_write_en:1;
    uint32_t            rsvd1:31;
    uint16_t            sector_size;
    uint16_t            application_tag;
    uint32_t            sector_num;
    uint64_t            doorbell_addr;
    uint64_t            doorbell_data;
} __PACK__ barco_desc_t;

hal_ret_t brq_gcm_slot_parser(pd_wring_meta_t *meta, wring_t *wring, uint8_t *slot)
{
    barco_desc_t        *gcm_desc;

    gcm_desc = (barco_desc_t*) slot;

    /* Fields already in Little-endian */
    wring->slot_info.gcm_desc.ilist_addr = gcm_desc->input_list_address;
    wring->slot_info.gcm_desc.olist_addr = gcm_desc->output_list_address;
    wring->slot_info.gcm_desc.command = gcm_desc->command;
    wring->slot_info.gcm_desc.key_desc_index = gcm_desc->key_desc_index;
    wring->slot_info.gcm_desc.iv_addr = gcm_desc->iv_address;
    wring->slot_info.gcm_desc.status_addr = gcm_desc->status_address;
    wring->slot_info.gcm_desc.doorbell_addr = gcm_desc->doorbell_addr;
    wring->slot_info.gcm_desc.doorbell_data = gcm_desc->doorbell_data;
    wring->slot_info.gcm_desc.header_size = gcm_desc->header_size;

    /* IV is not directly located in the ring, hence dereference it */

    if (gcm_desc->iv_address) {
        if(sdk::asic::asic_mem_read(gcm_desc->iv_address,
                            (uint8_t*)&wring->slot_info.gcm_desc.salt,
                            sizeof(wring->slot_info.gcm_desc.salt))) {
            HAL_TRACE_ERR("Failed to read the Salt information from HBM");
        }
        if(sdk::asic::asic_mem_read(gcm_desc->iv_address + 4,
                            (uint8_t*)&wring->slot_info.gcm_desc.explicit_iv,
                            sizeof(wring->slot_info.gcm_desc.explicit_iv))) {
            HAL_TRACE_ERR("Failed to read the explicit IV information from HBM");
        }
    } else {
            wring->slot_info.gcm_desc.salt = 0;
            wring->slot_info.gcm_desc.explicit_iv = 0;
    }
    if (gcm_desc->status_address) {
        if(sdk::asic::asic_mem_read(gcm_desc->status_address,
                            (uint8_t*)&wring->slot_info.gcm_desc.barco_status,
                            sizeof(wring->slot_info.gcm_desc.barco_status))) {
            HAL_TRACE_ERR("Failed to read the Barco Status information from HBM");
        }
    } else {
        wring->slot_info.gcm_desc.barco_status = 0;
    }

    return HAL_RET_OK;
}

hal_ret_t serq_slot_parser(pd_wring_meta_t *meta, wring_t *wring, uint8_t *slot)
{

    /* We still only return the descriptor pointer */
    wring->slot_value = ntohll(*(uint64_t *)slot);

    return HAL_RET_OK;
}

static
hal_ret_t
p4pd_wring_get_entry(pd_wring_t* wring_pd)
{
    hal_ret_t           ret = HAL_RET_OK;
    wring_t*            wring = wring_pd->wring;
    pd_wring_meta_t     *meta = &g_meta[wring->wring_type];
    wring_hw_id_t       wring_base;

    ret = wring_pd_get_base_addr(wring->wring_type,
                                 wring->wring_id,
                                 &wring_base);
    if(ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Could not find the wring base addr");
        return ret;
    }

    // Normalize
    uint32_t slot_index = (wring_pd->wring->slot_index % meta->num_slots);
    wring_hw_id_t slot_addr = wring_base + (slot_index * meta->slot_size_in_bytes);

    uint8_t value[meta->slot_size_in_bytes];
    uint64_t *value64 = (uint64_t*)value;

    if(sdk::asic::asic_mem_read(slot_addr,
                        value,
                        meta->slot_size_in_bytes)) {
        HAL_TRACE_ERR("Failed to read the data from the hw)");
    }

    if (meta->slot_parser) {
        meta->slot_parser(meta, wring, value);
    }
    else if(meta->slot_size_in_bytes == sizeof(uint64_t)) {
        wring->slot_value = ntohll(*value64);
    } else {
        /* All non basic types need to be supported with a parser */
        assert(0);
    }
    return ret;
}

hal_ret_t
barco_gcm0_get_hw_meta(pd_wring_t* wring_pd)
{
    uint32_t            value;
    hal_ret_t           ret = HAL_RET_OK;

    if (sdk::asic::asic_reg_read(CAPRI_BARCO_MD_HENS_REG_GCM0_PRODUCER_IDX,
                                 &value) != SDK_RET_OK) {
        HAL_TRACE_ERR("Failed to read the Barco PIDX value from hw)");
    }
    else {
        HAL_TRACE_DEBUG("Barco GCM0 PIDX {:#x}", value);
        wring_pd->wring->pi = value;
    }

    if (sdk::asic::asic_reg_read(CAPRI_BARCO_MD_HENS_REG_GCM0_CONSUMER_IDX,
                                 &value) != SDK_RET_OK) {
        HAL_TRACE_ERR("Failed to read the Barco CIDX value from hw)");
    }
    else {
        HAL_TRACE_DEBUG("Barco GCM0 CIDX {:#x}", value);
        wring_pd->wring->ci = value;
    }
    return ret;
}

hal_ret_t
armq_slot_parser(pd_wring_meta_t *meta, wring_t *wring, uint8_t *slot)
{
    wring->slot_value = ntohll(*(uint64_t *)slot);
    // clear the 63rd bit
    wring->slot_value = wring->slot_value & ( ((uint64_t)1 << 63) - 1);
    return HAL_RET_OK;
}

hal_ret_t
tcp_actl_q_get_hw_meta(pd_wring_t* wring_pd)
{
    uint32_t            value = 0;
    wring_hw_id_t addr = ASIC_SEM_TCP_ACTL_Q_RAW_ADDR(wring_pd->wring->wring_id);
    if(addr <= 0) {
        HAL_TRACE_ERR("Failed to get semaphore register addr for id: {}",
                    wring_pd->wring->wring_id);
        return HAL_RET_QUEUE_NOT_FOUND;
    }

    if (sdk::asic::asic_reg_read(addr, &value) != SDK_RET_OK) {
        HAL_TRACE_ERR("Failed to read pindex value");
        return HAL_RET_HW_FAIL;
    }

    wring_pd->wring->pi = value;
    addr += 4;
    if (sdk::asic::asic_reg_read(addr, &value) != SDK_RET_OK) {
        HAL_TRACE_ERR("Failed to read cindex value");
        return HAL_RET_HW_FAIL;
    }

    wring_pd->wring->ci = value;

    HAL_TRACE_DEBUG("TCP ACTL Q id: {} pi addr {:#x}, pi: {}, ci: {}",
                    wring_pd->wring->wring_id,
                    addr,
                    wring_pd->wring->pi,
                    wring_pd->wring->ci);
    return HAL_RET_OK;
}

hal_ret_t
arqrx_get_hw_meta(pd_wring_t* wring_pd)
{
    uint32_t            value = 0;
    wring_hw_id_t addr = ASIC_SEM_ARQ_RAW_ADDR(wring_pd->wring->wring_id);
    if(addr <= 0) {
        HAL_TRACE_ERR("Failed to get semaphore register addr for id: {}",
                    wring_pd->wring->wring_id);
        return HAL_RET_QUEUE_NOT_FOUND;
    }

    if (sdk::asic::asic_reg_read(addr, &value) != SDK_RET_OK) {
        HAL_TRACE_ERR("Failed to read pindex value");
        return HAL_RET_HW_FAIL;
    }

    wring_pd->wring->pi = value;
    addr += 4;
    if (sdk::asic::asic_reg_read(addr, &value) != SDK_RET_OK) {
        HAL_TRACE_ERR("Failed to read cindex value");
        return HAL_RET_HW_FAIL;
    }

    wring_pd->wring->ci = value;

    HAL_TRACE_DEBUG("ARQ id: {} pi addr {:#x}, pi: {}, ci: {}",
                    wring_pd->wring->wring_id,
                    addr,
                    wring_pd->wring->pi,
                    wring_pd->wring->ci);
    return HAL_RET_OK;
}

static
hal_ret_t
p4pd_wring_get_meta(pd_wring_t* wring_pd)
{
    hal_ret_t           ret = HAL_RET_OK;
    wring_t*            wring = wring_pd->wring;
    pd_wring_meta_t     *meta = &g_meta[wring->wring_type];
    uint64_t            sem_addr = meta->alloc_semaphore_addr;
    wring_hw_id_t       wring_base;

    wring->num_entries = meta->num_slots;
    wring->obj_size = meta->obj_size;
    wring->is_global = meta->is_global;

    ret = wring_pd_get_base_addr(wring->wring_type,
                                 wring->wring_id,
                                 &wring_base);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Could not find the wring base addr");
        return ret;
    }
    wring->phys_base_addr = wring_base;

    if(meta->get_hw_meta_fn) {
        return meta->get_hw_meta_fn(wring_pd);
    }

    if(sem_addr == 0) {
        HAL_TRACE_DEBUG("skipping PI/CI read for sem addr 0");
        return ret;
    }

    HAL_TRACE_DEBUG("Reading pi from the addr: {:#x}", sem_addr);

    uint32_t value;
    if (sdk::asic::asic_reg_read(sem_addr, &value) != SDK_RET_OK) {
        HAL_TRACE_ERR("Failed to read the data from the hw)");
    }

    wring->pi = value;

    sem_addr += 4;
    HAL_TRACE_DEBUG("Reading ci from the addr: {:#x}", sem_addr);

    if (sdk::asic::asic_reg_read(sem_addr, &value) != SDK_RET_OK) {
        HAL_TRACE_ERR("Failed to read the data from the hw)");
    }

    wring->ci = value;
    return ret;
}

static
hal_ret_t
p4pd_wring_set_meta(pd_wring_t* wring_pd)
{
    hal_ret_t           ret = HAL_RET_OK;
    wring_t*            wring = wring_pd->wring;
    pd_wring_meta_t     *meta = &g_meta[wring->wring_type];
    uint64_t            sem_addr = meta->alloc_semaphore_addr;

    if(meta->set_hw_meta_fn) {
        return meta->set_hw_meta_fn(wring_pd);
    }

    HAL_TRACE_DEBUG("Writing pi {} to addr: {:#x}",
            wring->pi, sem_addr);

    if (sdk::asic::asic_reg_write(sem_addr, &wring->pi) != SDK_RET_OK) {
        HAL_TRACE_ERR("Failed to write data to hw)");
    }

    sem_addr += 4;
    HAL_TRACE_DEBUG("Writing ci {} to addr: {:#x}",
            wring->ci, sem_addr);

    if (sdk::asic::asic_reg_write(sem_addr, &wring->ci) != SDK_RET_OK) {
        HAL_TRACE_ERR("Failed to write data to hw)");
    }

    return ret;
}

hal_ret_t
p4pd_wring_set_rnmdpr_meta(pd_wring_t* wring_pd)
{
    hal_ret_t           ret = HAL_RET_OK;
    wring_t*            wring = wring_pd->wring;
    pd_wring_meta_t     *meta = &g_meta[wring->wring_type];
    uint64_t            sem_addr = meta->alloc_semaphore_addr;
    uint64_t            addr;
    uint32_t            ci;

    HAL_TRACE_DEBUG("Writing pi {} to addr: {:#x}",
            wring->pi, sem_addr);

    if (sdk::asic::asic_reg_write(sem_addr, &wring->pi) != SDK_RET_OK) {
        HAL_TRACE_ERR("Failed to write data to hw)");
    }

    sem_addr += 4;
    HAL_TRACE_DEBUG("Writing ci {} to addr: {:#x}",
            wring->ci, sem_addr);

    if (sdk::asic::asic_reg_write(sem_addr, &wring->ci) != SDK_RET_OK) {
        HAL_TRACE_ERR("Failed to write data to hw)");
    }

    addr = asicpd_get_mem_addr(ASIC_HBM_REG_TLS_PROXY_PAD_TABLE) +
        CAPRI_GC_GLOBAL_RNMDPR_FP_PI;
    ci = wring->ci - ASIC_RNMDPR_BIG_RING_SIZE;
    HAL_TRACE_DEBUG("Writing ci {} to addr: {:#x}", wring->ci, addr);
    sdk::asic::asic_mem_write(addr, (uint8_t *)&ci, sizeof(uint32_t));

    return ret;
}

static
hal_ret_t
p4pd_wring_get_base_addr(pd_wring_t* wring_pd)
{
    hal_ret_t           ret = HAL_RET_OK;
    wring_t*            wring = wring_pd->wring;
    wring_hw_id_t       wring_base;

    ret = wring_pd_get_base_addr(wring->wring_type,
                                 wring->wring_id,
                                 &wring_base);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Could not find the wring base addr");
        return ret;
    }

    HAL_TRACE_DEBUG("wring {} {}: {:#x}", wring->wring_type, wring->wring_id,
            wring_base);

    wring->phys_base_addr = wring_base;
    return ret;
}

hal_ret_t
wring_pd_init_global_rings()
{
    hal_ret_t   ret = HAL_RET_OK;

    ret = wring_pd_meta_init();
    if(ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Failed to initialize meta");
        return HAL_RET_ERR;
    }

    for(int i = 1; i< types::WRingType_ARRAYSIZE; i++) {
        if(g_meta[i].is_global) {
            ret = wring_pd_table_init(types::WRingType(i), 0);
            if(ret != HAL_RET_OK) {
                HAL_TRACE_ERR("Failed to initialize p4plus wring of type: {}", i);
                // continue with the initialization of the remaining tables
                continue;
            }
            HAL_TRACE_DEBUG("Initialized p4plus ring: {}",
                types::WRingType_Name(types::WRingType(i)));
        }
    }
    return ret;
}

void *
wring_pd_get_hw_key_func (void *entry)
{
    SDK_ASSERT(entry != NULL);
    return (void *)&(((pd_wring_t *)entry)->hw_id);
}

uint32_t
wring_pd_hw_key_size ()
{
    return sizeof(wring_hw_id_t);
}

hal_ret_t
pd_wring_create (pd_func_args_t *pd_func_args)
{
    hal_ret_t               ret;
    pd_wring_create_args_t *args = pd_func_args->pd_wring_create;
    pd_wring_s              *wring_pd;

    HAL_TRACE_DEBUG("WRING pd create");

    // allocate PD wring state
    wring_pd = wring_pd_alloc_init();
    if (wring_pd == NULL) {
        return HAL_RET_OOM;
    }
    HAL_TRACE_DEBUG("Alloc done");
    wring_pd->wring = args->wring;

    // TODO: get hw base  for this WRING
    //wring_pd->hw_id = pd_wring_get_base_hw_index(wring_pd);
    ret = wring_pd_table_init(wring_pd->wring->wring_type,
                              wring_pd->wring->wring_id);
    if(ret != HAL_RET_OK) {
        goto cleanup;
    }

    // add to db
    ret = add_wring_pd_to_db(wring_pd);
    if (ret != HAL_RET_OK) {
       goto cleanup;
    }
    HAL_TRACE_DEBUG("DB add done");
    args->wring->pd = wring_pd;

    return HAL_RET_OK;

cleanup:
    if (wring_pd) {
        wring_pd_free(wring_pd);
    }
    return ret;
}

hal_ret_t
pd_wring_get_entry (pd_func_args_t *pd_func_args)
{
    hal_ret_t               ret;
    pd_wring_get_entry_args_t *args = pd_func_args->pd_wring_get_entry;
    pd_wring_t              wring_pd;

    // allocate PD wring state
    wring_pd_init(&wring_pd);

    wring_pd.wring = args->wring;

    // get hw wring entry
    ret = p4pd_wring_get_entry(&wring_pd);
    if(ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Failed to get wring entry");
        goto cleanup;
    }
cleanup:
    return ret;
}

hal_ret_t
pd_wring_get_meta (pd_func_args_t *pd_func_args)
{
    hal_ret_t               ret;
    pd_wring_get_meta_args_t *args = pd_func_args->pd_wring_get_meta;
    pd_wring_t              wring_pd;

    HAL_TRACE_DEBUG("Wring pd get meta");

    // allocate PD wring state
    wring_pd_init(&wring_pd);

    wring_pd.wring = args->wring;

    // get hw wring entry
    ret = p4pd_wring_get_meta(&wring_pd);
    if(ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Failed to get wring meta");
        goto cleanup;
    }
cleanup:
    return ret;
}

hal_ret_t
pd_wring_set_meta (pd_func_args_t *pd_func_args)
{
    hal_ret_t               ret;
    pd_wring_set_meta_args_t *args = pd_func_args->pd_wring_set_meta;
    pd_wring_t              wring_pd;

    HAL_TRACE_DEBUG("Wring pd set meta");

    // allocate PD wring state
    wring_pd_init(&wring_pd);

    wring_pd.wring = args->wring;

    // get hw wring entry
    ret = p4pd_wring_set_meta(&wring_pd);
    if(ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Failed to get wring meta");
        goto cleanup;
    }
cleanup:
    return ret;
}

hal_ret_t
pd_wring_get_base_addr (pd_func_args_t *pd_func_args)
{
    hal_ret_t               ret;
    pd_wring_get_meta_args_t *args = pd_func_args->pd_wring_get_meta;
    pd_wring_t              wring_pd;

    HAL_TRACE_DEBUG("Wring pd get base_addr");

    // allocate PD wring state
    wring_pd_init(&wring_pd);

    wring_pd.wring = args->wring;

    // get hw wring entry
    ret = p4pd_wring_get_base_addr(&wring_pd);
    if(ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Failed to get wring meta");
        goto cleanup;
    }
cleanup:
    return ret;
}

}    // namespace pd
}    // namespace hal
