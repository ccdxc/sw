#include "nic/include/base.h"
#include "nic/include/hal_lock.hpp"
#include "nic/include/pd_api.hpp"
#include "nic/hal/pd/iris/hal_state_pd.hpp"
#include "nic/hal/pd/iris/wring_pd.hpp"
#include "nic/hal/pd/capri/capri_loader.h"
#include "nic/hal/pd/capri/capri_hbm.hpp"
#include "nic/include/capri_common.h"
#include "nic/include/capri_barco.h"
#include "nic/hal/pd/iris/p4plus_pd_api.h"

namespace hal {
namespace pd {

static pd_wring_meta_t g_meta[types::WRingType_MAX + 1];

hal_ret_t brq_gcm_slot_parser(pd_wring_meta_t *meta, wring_t *wring, uint8_t *slot);
hal_ret_t serq_slot_parser(pd_wring_meta_t *meta, wring_t *wring, uint8_t *slot);
hal_ret_t arqrx_get_hw_meta(pd_wring_t* wring_pd);
hal_ret_t barco_gcm0_get_hw_meta(pd_wring_t* wring_pd);

hal_ret_t  
wring_pd_meta_init() {
    /*
     * Add meta info for each ring
     * Format: is Global Ring, region name, # of slots in the ring, slot size in bytes
     *         associate object region name, associated size of object
     */

    g_meta[types::WRING_TYPE_SERQ] = 
        (pd_wring_meta_t) {false, CAPRI_HBM_REG_SERQ, 64, SERQ_WRING_SLOT_SIZE, "", 0, 0, 0, serq_slot_parser, NULL};
 
    g_meta[types::WRING_TYPE_NMDR_TX] = 
        (pd_wring_meta_t) {true, CAPRI_HBM_REG_NMDR_TX, CAPRI_TNMDR_RING_SIZE,
                           DEFAULT_WRING_SLOT_SIZE, CAPRI_HBM_REG_DESCRIPTOR_TX, 128,
                           CAPRI_SEM_TNMDR_ALLOC_RAW_ADDR, CAPRI_SEM_TNMDR_FREE_RAW_ADDR,
                           NULL, NULL};
 
    g_meta[types::WRING_TYPE_NMDR_RX] = 
        (pd_wring_meta_t) {true, CAPRI_HBM_REG_NMDR_RX, CAPRI_RNMDR_RING_SIZE,
                           DEFAULT_WRING_SLOT_SIZE, CAPRI_HBM_REG_DESCRIPTOR_RX, 128,
                           CAPRI_SEM_RNMDR_ALLOC_RAW_ADDR, CAPRI_SEM_RNMDR_FREE_RAW_ADDR,
                           NULL, NULL};
    
    g_meta[types::WRING_TYPE_NMPR_SMALL_TX] = 
        (pd_wring_meta_t) {true, CAPRI_HBM_REG_NMPR_SMALL_TX, CAPRI_TNMPR_SMALL_RING_SIZE,
                           DEFAULT_WRING_SLOT_SIZE, CAPRI_HBM_REG_PAGE_SMALL_TX, 2048,
                           CAPRI_SEM_TNMPR_SMALL_ALLOC_RAW_ADDR, CAPRI_SEM_TNMPR_SMALL_FREE_RAW_ADDR,
                           NULL, NULL};

    g_meta[types::WRING_TYPE_NMPR_SMALL_RX] = 
        (pd_wring_meta_t) {true, CAPRI_HBM_REG_NMPR_SMALL_RX, CAPRI_RNMPR_SMALL_RING_SIZE,
                           DEFAULT_WRING_SLOT_SIZE, CAPRI_HBM_REG_PAGE_SMALL_RX, 2048,
                           CAPRI_SEM_RNMPR_SMALL_ALLOC_RAW_ADDR, CAPRI_SEM_RNMPR_SMALL_FREE_RAW_ADDR,
                           NULL, NULL};

    g_meta[types::WRING_TYPE_NMPR_BIG_TX] = 
        (pd_wring_meta_t) {true, CAPRI_HBM_REG_NMPR_BIG_TX, CAPRI_TNMPR_RING_SIZE,
                           DEFAULT_WRING_SLOT_SIZE, CAPRI_HBM_REG_PAGE_BIG_TX, 9216,
                           CAPRI_SEM_TNMPR_ALLOC_RAW_ADDR, CAPRI_SEM_TNMPR_FREE_RAW_ADDR,
                           NULL, NULL};

    g_meta[types::WRING_TYPE_NMPR_BIG_RX] = 
        (pd_wring_meta_t) {true, CAPRI_HBM_REG_NMPR_BIG_RX, CAPRI_RNMPR_RING_SIZE,
                           DEFAULT_WRING_SLOT_SIZE, CAPRI_HBM_REG_PAGE_BIG_RX, 9216,
                           CAPRI_SEM_RNMPR_ALLOC_RAW_ADDR, CAPRI_SEM_RNMPR_FREE_RAW_ADDR,
                           NULL, NULL};

    g_meta[types::WRING_TYPE_BSQ] =
        (pd_wring_meta_t) {false, CAPRI_HBM_REG_BSQ, 64, DEFAULT_WRING_SLOT_SIZE, "", 0, 0, 0, NULL, NULL};

    g_meta[types::WRING_TYPE_BRQ] =
        (pd_wring_meta_t) {true, CAPRI_HBM_REG_BRQ, 1024, 128, "", 0, 0, 0, brq_gcm_slot_parser, barco_gcm0_get_hw_meta};

    g_meta[types::WRING_TYPE_SESQ] = 
        (pd_wring_meta_t) {false, CAPRI_HBM_REG_SESQ, 64, DEFAULT_WRING_SLOT_SIZE, "", 0, 0, 0, NULL, NULL};

    g_meta[types::WRING_TYPE_IPSECCBQ] =
        (pd_wring_meta_t) {false, CAPRI_HBM_REG_IPSECCB, 64, DEFAULT_WRING_SLOT_SIZE,
                                  "", 0, 0, 0, NULL, NULL};
    g_meta[types::WRING_TYPE_IPSECCBQ_BARCO] =
        (pd_wring_meta_t) {false, CAPRI_HBM_REG_IPSECCB_BARCO, 64, 128,
                                  "", 0, 0, 0, NULL, NULL};
    g_meta[types::WRING_TYPE_ARQRX] =
        (pd_wring_meta_t) {false, CAPRI_HBM_REG_ARQRX, 1024, DEFAULT_WRING_SLOT_SIZE, "", 0, 0, 0, NULL, arqrx_get_hw_meta};

    g_meta[types::WRING_TYPE_ARQTX] =
        (pd_wring_meta_t) {false, CAPRI_HBM_REG_ARQTX, 1024, DEFAULT_WRING_SLOT_SIZE, "", 0, 0, 0, NULL, arqrx_get_hw_meta};

    g_meta[types::WRING_TYPE_ASQ] =
        (pd_wring_meta_t) {true, CAPRI_HBM_REG_ASQ, 1024, DEFAULT_WRING_SLOT_SIZE, "", 0, 0, 0, NULL, NULL};

    g_meta[types::WRING_TYPE_APP_REDIR_RAWC] =
        (pd_wring_meta_t) {false, CAPRI_HBM_REG_APP_REDIR_RAWC, 1024, DEFAULT_WRING_SLOT_SIZE, "", 0, 0};

    g_meta[types::WRING_TYPE_APP_REDIR_PROXYR] =
        (pd_wring_meta_t) {false, CAPRI_HBM_REG_APP_REDIR_PROXYR, 1024, DEFAULT_WRING_SLOT_SIZE, "", 0, 0};

    g_meta[types::WRING_TYPE_APP_REDIR_PROXYC] =
        (pd_wring_meta_t) {false, CAPRI_HBM_REG_APP_REDIR_PROXYC, 1024, DEFAULT_WRING_SLOT_SIZE, "", 0, 0};

    g_meta[types::WRING_TYPE_ASESQ] = 
        (pd_wring_meta_t) {false, CAPRI_HBM_REG_ASESQ, 64, DEFAULT_WRING_SLOT_SIZE, "", 0, 0, 0, NULL};

    g_meta[types::WRING_TYPE_NMDR_TX_GC] = 
        (pd_wring_meta_t) {true, CAPRI_HBM_REG_NMDR_TX_GC, CAPRI_TNMDR_GC_RING_SIZE,
                           DEFAULT_WRING_SLOT_SIZE, "", 0, 0, 0, NULL, NULL};
 
    g_meta[types::WRING_TYPE_NMDR_RX_GC] = 
        (pd_wring_meta_t) {true, CAPRI_HBM_REG_NMDR_RX_GC, CAPRI_RNMDR_GC_RING_SIZE,
                           DEFAULT_WRING_SLOT_SIZE, "", 0, 0, 0, NULL, NULL};

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
    g_hal_state_pd->wring_slab()->free(wring_pd);
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
        wring_hw_id_t obj_addr_base = get_start_offset(meta->obj_hbm_reg_name);
        if(obj_addr_base == 0) {
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
wring_pd_get_base_addr(types::WRingType type, uint32_t wring_id, wring_hw_id_t* wring_base)
{
    pd_wring_meta_t     *meta = &g_meta[type];
    wring_hw_id_t       addr = 0;

    addr = get_start_offset(meta->hbm_reg_name); 
    if(!addr) {
        HAL_TRACE_ERR("Could not find addr for {} region", meta->hbm_reg_name);
        return HAL_RET_ERR;
    }

    if(meta->is_global) {
        // Global rings start at the base of hbm_region
        *wring_base = addr;    
    } else {
        // Flow local ring. Get the offset based on wring id
        uint32_t wring_size = meta->num_slots * meta->slot_size_in_bytes; 
        *wring_base = addr + (wring_id * wring_size);
    }

    return HAL_RET_OK;
}

hal_ret_t
wring_pd_table_init(types::WRingType type, uint32_t wring_id) 
{
    hal_ret_t           ret = HAL_RET_OK;
    pd_wring_meta_t     *meta = &g_meta[type];
    wring_hw_id_t       wring_base;

    ret = wring_pd_get_base_addr(type, wring_id, &wring_base);
    if(ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Could not find the wring base addr");
        return ret;
    }
    
    uint32_t reg_size = get_size_kb(meta->hbm_reg_name);
    if(!reg_size) {
        HAL_TRACE_ERR("Could not find size for {} region", meta->hbm_reg_name);
        return HAL_RET_ERR;
    }

    uint32_t required_size = meta->num_slots * meta->slot_size_in_bytes;
    HAL_ASSERT(reg_size * 1024 >= required_size);

    // Allocate memory for storing value for a slot
    uint8_t value[meta->slot_size_in_bytes];
    for(uint32_t  index = 0; index<meta->num_slots; index++) {

        uint64_t slot_addr = wring_base + (index * meta->slot_size_in_bytes);
        
        get_default_slot_value(type, index, value);
        p4plus_hbm_write(slot_addr, value, meta->slot_size_in_bytes);
    }
    if (meta->alloc_semaphore_addr &&
                    CAPRI_SEM_RAW_IS_PI_CI(meta->alloc_semaphore_addr)) {
        // Set CI = ring size
        uint32_t val32 = meta->num_slots;
        HAL_TRACE_DEBUG("writing {0} to semaphore 0x{1:x}\n",
                                val32, meta->alloc_semaphore_addr +
                                CAPRI_SEM_INC_NOT_FULL_CI_OFFSET);
        p4plus_reg_write(
                meta->alloc_semaphore_addr + CAPRI_SEM_INC_NOT_FULL_CI_OFFSET,
                val32);
    }
    if (meta->free_semaphore_addr &&
                    CAPRI_SEM_RAW_IS_PI_CI(meta->free_semaphore_addr)) {
        // Initialize this ring as
        // FP.PI = AP.CI = meta->num_slots
        uint32_t val32 = meta->num_slots;
        p4plus_reg_write(meta->free_semaphore_addr, val32);
        HAL_TRACE_DEBUG("writing {0} to semaphore 0x{1:x}\n",
                                val32, meta->free_semaphore_addr);

        // FP.CI = FP.PI + 1 (this queue is initially full, until one object is
        // inserted). Note CI is currently unused, since FP is incremented using
        // INC view instead of INF view. This is ok since the only way FP.PI
        // INC can be FULL is if we are trying to free a page, before it
        // has been allocated. Keeping this semaphore here for future need
        val32++;
        p4plus_reg_write(meta->free_semaphore_addr + CAPRI_SEM_INC_NOT_FULL_CI_OFFSET,
                                        val32);
        HAL_TRACE_DEBUG("writing {0} to semaphore 0x{1:x}\n",
                                val32, meta->free_semaphore_addr +
                                CAPRI_SEM_INC_NOT_FULL_CI_OFFSET);
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
    
    if(!p4plus_hbm_read(gcm_desc->iv_address, 
                        (uint8_t*)&wring->slot_info.gcm_desc.salt,
                        sizeof(wring->slot_info.gcm_desc.salt))) {
        HAL_TRACE_ERR("Failed to read the Salt information from HBM");    
    }
    if(!p4plus_hbm_read(gcm_desc->iv_address + 4, 
                        (uint8_t*)&wring->slot_info.gcm_desc.explicit_iv, 
                        sizeof(wring->slot_info.gcm_desc.explicit_iv))) {
        HAL_TRACE_ERR("Failed to read the explicit IV information from HBM");    
    }
    if(!p4plus_hbm_read(gcm_desc->status_address, 
                        (uint8_t*)&wring->slot_info.gcm_desc.barco_status, 
                        sizeof(wring->slot_info.gcm_desc.barco_status))) {
        HAL_TRACE_ERR("Failed to read the Barco Status information from HBM");    
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

    if(!p4plus_hbm_read(slot_addr, 
                        value, 
                        meta->slot_size_in_bytes)) {
        HAL_TRACE_ERR("Failed to read the data from the hw)");    
    }
    if(meta->slot_size_in_bytes == sizeof(uint64_t)) {
        wring->slot_value = ntohll(*value64);
    } else {
        if (meta->slot_parser) {
            meta->slot_parser(meta, wring, value);
        }
        else {
            /* All non basic types need to be supported with a parser */
            assert(0);
        }
    }
    return ret;
}

hal_ret_t
barco_gcm0_get_hw_meta(pd_wring_t* wring_pd)
{
    uint32_t            value;
    hal_ret_t           ret = HAL_RET_OK;

    if(!p4plus_reg_read(CAPRI_BARCO_MD_HENS_REG_GCM0_PRODUCER_IDX,
                        value)) {
        HAL_TRACE_ERR("Failed to read the Barco PIDX value from hw)");
    }
    else {
        HAL_TRACE_DEBUG("Barco GCM0 PIDX 0x{0:x}", value);
        wring_pd->wring->pi = value;
    }

    if(!p4plus_reg_read(CAPRI_BARCO_MD_HENS_REG_GCM0_CONSUMER_IDX,
                        value)) {
        HAL_TRACE_ERR("Failed to read the Barco CIDX value from hw)");
    }
    else {
        HAL_TRACE_DEBUG("Barco GCM0 CIDX 0x{0:x}", value);
        wring_pd->wring->ci = value;
    }
    return ret;
}

hal_ret_t
arqrx_get_hw_meta(pd_wring_t* wring_pd)
{
#ifdef DO_NOT_USE_CPU_SEM
	uint16_t			pindex = 0;
	// get address for ARQRX queue index region
	wring_hw_id_t addr = get_start_offset(CAPRI_HBM_REG_ARQRX_QIDXR);
	if(addr <= 0) {
		HAL_TRACE_ERR("Failed to get ARQ QIDXR region");
		return HAL_RET_QUEUE_NOT_FOUND;
	}

	if(!p4plus_hbm_read(addr, (uint8_t*)&pindex, sizeof(pindex))) {
		HAL_TRACE_ERR("Failed to read pindex value");
		return HAL_RET_HW_FAIL;
	}
    pindex = ntohs(pindex);
#else
	uint32_t			pindex = 0;
    wring_hw_id_t addr = CAPRI_SEM_ARQ_RX_IDX_RAW_ADDR(wring_pd->wring->wring_id);
    if(addr <= 0) {
        HAL_TRACE_ERR("Failed to get semaphore register addr for id: {}",
                    wring_pd->wring->wring_id);
        return HAL_RET_QUEUE_NOT_FOUND;
    }

    if(!p4plus_reg_read(addr, pindex)) {
        HAL_TRACE_ERR("Failed to read pindex value");
		return HAL_RET_HW_FAIL;
    }
#endif
    HAL_TRACE_DEBUG("ARQRX PI ADDR 0x{0:x}", addr);
    HAL_TRACE_DEBUG("ARQRX PI VALUE 0x{0:x}", pindex);
	wring_pd->wring->pi = pindex;
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

    if(meta->get_hw_meta_fn) {
    	return meta->get_hw_meta_fn(wring_pd);
    }

    HAL_TRACE_DEBUG("Reading pi from the addr: 0x{0:x}", sem_addr);

    uint32_t value;
    if(!p4plus_reg_read(sem_addr,
                        value)) {
        HAL_TRACE_ERR("Failed to read the data from the hw)");
    }

    wring->pi = value;

    sem_addr += 4;
    HAL_TRACE_DEBUG("Reading ci from the addr: 0x{0:x}", sem_addr);

    if(!p4plus_reg_read(sem_addr,
                        value)) {
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

    if(meta->get_hw_meta_fn) {
    	return meta->get_hw_meta_fn(wring_pd);
    }

    HAL_TRACE_DEBUG("Writing pi {0} to addr: 0x{1:x}",
            wring->pi, sem_addr);

    if(!p4plus_reg_write(sem_addr,
                        wring->pi)) {
        HAL_TRACE_ERR("Failed to write data to hw)");
    }

    sem_addr += 4;
    HAL_TRACE_DEBUG("Writing ci {0} to addr: 0x{1:x}",
            wring->ci, sem_addr);

    if(!p4plus_reg_write(sem_addr,
                        wring->ci)) {
        HAL_TRACE_ERR("Failed to write data to hw)");
    }

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
    HAL_ASSERT(entry != NULL);
    return (void *)&(((pd_wring_t *)entry)->hw_id);
}

uint32_t
wring_pd_compute_hw_hash_func (void *key, uint32_t ht_size)
{
    return sdk::lib::hash_algo::fnv_hash(key, sizeof(wring_hw_id_t)) % ht_size;
}

bool
wring_pd_compare_hw_key_func (void *key1, void *key2)
{
    HAL_ASSERT((key1 != NULL) && (key2 != NULL));
    if (*(wring_hw_id_t *)key1 == *(wring_hw_id_t *)key2) {
        return true;
    }
    return false;
}

hal_ret_t
pd_wring_create (pd_wring_args_t *args)
{
    hal_ret_t               ret;
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
pd_wring_get_entry (pd_wring_args_t *args)
{
    hal_ret_t               ret;
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
pd_wring_get_meta (pd_wring_args_t *args)
{
    hal_ret_t               ret;
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
pd_wring_set_meta (pd_wring_args_t *args)
{
    hal_ret_t               ret;
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

}    // namespace pd
}    // namespace hal
