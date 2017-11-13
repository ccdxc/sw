#include "nic/hal/hal.hpp"
#include "nic/gen/proto/hal/types.pb.h"
#include "nic/hal/pd/capri/capri_hbm.hpp"
#include "nic/hal/pd/capri/capri_barco_rings.hpp"
#include "nic/hal/pd/iris/hal_state_pd.hpp"
#include "nic/asic/capri/model/cap_top/cap_top_csr.h"
#include "nic/hal/pd/capri/capri_barco_res.hpp"
#include "nic/hal/pd/capri/capri_barco_crypto.hpp"
#include "nic/hal/src/barco_rings.hpp"

namespace hal {

namespace pd {

hal_ret_t capri_barco_asym_init(capri_barco_ring_t *barco_ring);
bool capri_barco_asym_poller(capri_barco_ring_t *barco_ring, uint32_t req_tag);
hal_ret_t capri_barco_asym_queue_request(struct capri_barco_ring_s *barco_ring,
        void *req, uint32_t *req_tag);
hal_ret_t capri_barco_xts_init(capri_barco_ring_t *barco_ring);
bool capri_barco_xts_poller(capri_barco_ring_t *barco_ring, uint32_t req_tag);
hal_ret_t capri_barco_mpp_queue_request(struct capri_barco_ring_s *barco_ring,
					void *req, uint32_t *req_tag);
hal_ret_t capri_barco_mpp_init(capri_barco_ring_t *barco_ring);
bool capri_barco_mpp_poller(capri_barco_ring_t *barco_ring, uint32_t req_tag);

capri_barco_ring_t  barco_rings[] = {
    {   /* types::BARCO_RING_ASYM */
        BARCO_RING_ASYM_STR,
        CAPRI_HBM_REG_BARCO_RING_ASYM,
        0,
        32,
        1024,
        32,
        0,
        0,
        0,
        0,
        capri_barco_asym_init,
        capri_barco_asym_poller,
        capri_barco_asym_queue_request,
    },
    {
    },
    {
    },
    {
    },
    {
        BARCO_RING_XTS_STR,
        CAPRI_HBM_REG_BARCO_RING_XTS,
        0,
        32,
        1024,
        BARCO_CRYPTO_DESC_SZ,
        0,
        0,
        0,
        0,
        capri_barco_xts_init,
        capri_barco_xts_poller,
        NULL,
    },
    {
        BARCO_RING_MPP0_STR,
        CAPRI_HBM_REG_BARCO_RING_MPP0,
        0,
        128,
        1024,
        128,
        0,
        0,
        0,
        0,
        capri_barco_mpp_init,
        capri_barco_mpp_poller,
        capri_barco_mpp_queue_request,
    },
    {
    },
    {
    },
    {
    },
    {
    },
    {
    },
    {
    },
    {
    },
};

hal_ret_t capri_barco_ring_common_init(capri_barco_ring_t *barco_ring)
{
    uint64_t                            ring_base = 0;
    uint32_t                            ring_size = 0;

    ring_base = get_start_offset(barco_ring->hbm_region);
    if (!ring_base) {
        HAL_TRACE_ERR("Failed to retrieve Barco Ring memory region for {}", barco_ring->ring_name);
        return HAL_RET_ERR;
    }

    if (ring_base & (barco_ring->ring_alignment - 1)) {
        HAL_TRACE_ERR("Failed to retrieve aligned Barco Ring memory region for {}", barco_ring->ring_name);
        return HAL_RET_ERR;
    }
    
    ring_size = get_size_kb(barco_ring->hbm_region) * 1024;
    if (ring_size < (uint32_t)(barco_ring->ring_size * barco_ring->descriptor_size)) {
        HAL_TRACE_ERR("Not enough memory for Barco Ring memory region {}", barco_ring->ring_name);
        return HAL_RET_ERR;
    }

    barco_ring->ring_base = ring_base;

    return HAL_RET_OK;
}

hal_ret_t capri_barco_asym_key_array_init(void)
{
    cap_top_csr_t &                     cap0 = CAP_BLK_REG_MODEL_ACCESS(cap_top_csr_t, 0, 0);
    cap_hese_csr_t &                    hese = cap0.md.hese;
    hal_ret_t                           ret = HAL_RET_OK;
    uint64_t                            asym_key_array_base;
    uint32_t                            asym_key_array_key_count;

    ret = capri_barco_res_region_get(CRYPTO_BARCO_RES_ASYM_KEY_DESCR, &asym_key_array_base);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Could not retrieve the Asym Crypto Key Descr region");
        return ret;
    }
    
    ret = capri_barco_res_obj_count_get(CRYPTO_BARCO_RES_ASYM_KEY_DESCR, &asym_key_array_key_count);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Could not retrieve the Asym Crypto Key Descr count");
        return ret;
    }

    hese.dhs_crypto_ctl.pk_key_array_base_w0.fld(asym_key_array_base & 0xffffffff);
    hese.dhs_crypto_ctl.pk_key_array_base_w0.write();
    hese.dhs_crypto_ctl.pk_key_array_base_w1.fld(asym_key_array_base >> 32);
    hese.dhs_crypto_ctl.pk_key_array_base_w1.write();

    hese.dhs_crypto_ctl.pk_key_array_size.fld(asym_key_array_key_count);
    hese.dhs_crypto_ctl.pk_key_array_size.write();
    HAL_TRACE_DEBUG("Barco Asym Key Descriptor Array of count {} setup @ {:x}",
            asym_key_array_key_count, asym_key_array_base);

    return ret;
}


hal_ret_t capri_barco_asym_init(capri_barco_ring_t *barco_ring)
{
    cap_top_csr_t &                     cap0 = CAP_BLK_REG_MODEL_ACCESS(cap_top_csr_t, 0, 0);
    cap_hens_csr_t &                    hens = cap0.md.hens;
    //cap_hese_csr_t &                    hese = cap0.md.hese;
    hal_ret_t                           ret = HAL_RET_OK;

    ret = capri_barco_ring_common_init(barco_ring);
    if (ret != HAL_RET_OK) {
        return ret;
    }

    hens.dhs_crypto_ctl.pk_ring_base_w0.fld((uint32_t)(barco_ring->ring_base & 0xffffffff));
    hens.dhs_crypto_ctl.pk_ring_base_w0.write();
    hens.dhs_crypto_ctl.pk_ring_base_w1.fld((uint32_t)(barco_ring->ring_base >> 32));
    hens.dhs_crypto_ctl.pk_ring_base_w1.write();

    hens.dhs_crypto_ctl.pk_ring_size.fld(barco_ring->ring_size);
    hens.dhs_crypto_ctl.pk_ring_size.write();

    hens.dhs_crypto_ctl.pk_opa_tag_addr_w0.fld((uint32_t)(barco_ring->opaque_tag_addr & 0xffffffff));
    hens.dhs_crypto_ctl.pk_opa_tag_addr_w0.write();
    hens.dhs_crypto_ctl.pk_opa_tag_addr_w1.fld((uint32_t)(barco_ring->opaque_tag_addr >> 32));
    hens.dhs_crypto_ctl.pk_opa_tag_addr_w1.write();

    hens.dhs_crypto_ctl.pk_producer_idx.fld(barco_ring->producer_idx);
    hens.dhs_crypto_ctl.pk_producer_idx.write();

    hens.dhs_crypto_ctl.pk_consumer_idx.fld(barco_ring->consumer_idx);
    hens.dhs_crypto_ctl.pk_consumer_idx.write();

    HAL_TRACE_DEBUG("Barco ring \"{}\" base setup @ {:x}, descriptor count {}",
            barco_ring->ring_name, barco_ring->ring_base, barco_ring->ring_size);

    return capri_barco_asym_key_array_init();
}

bool capri_barco_asym_poller(capri_barco_ring_t *barco_ring, uint32_t req_tag)
{
    bool                                ret = FALSE;
    uint32_t                            curr_opaque_tag = 0;

    if (capri_hbm_read_mem(barco_ring->opaque_tag_addr, (uint8_t*)&curr_opaque_tag, sizeof(curr_opaque_tag))) {
        HAL_TRACE_ERR("Poll:{}: Failed to retrieve current opaque tag value @ {:x}",
                barco_ring->ring_name, (uint64_t) barco_ring->opaque_tag_addr); 
        return FALSE;
    }
    else {
        HAL_TRACE_DEBUG("Poll:{}: Retrieved opaque tag value: {}", barco_ring->ring_name, curr_opaque_tag);
        /* TODO: Handle wraparounds */
        if (curr_opaque_tag >= req_tag)
            ret = TRUE;
    }

    return ret;
}

hal_ret_t capri_barco_asym_queue_request(struct capri_barco_ring_s *barco_ring,
        void *req, uint32_t *req_tag)
{
    cap_top_csr_t &                     cap0 = CAP_BLK_REG_MODEL_ACCESS(cap_top_csr_t, 0, 0);
    cap_hens_csr_t &                    hens = cap0.md.hens;
    uint64_t                            slot_addr = 0;
    hal_ret_t                           ret = HAL_RET_OK;
    barco_asym_descriptor_t             *asym_req_descr = NULL;

    asym_req_descr = (barco_asym_descriptor_t*) req;

    asym_req_descr->opaque_tag_value = barco_ring->opaqe_tag_value;
    asym_req_descr->opage_tag_wr_en = 1;

    slot_addr = barco_ring->ring_base + (barco_ring->producer_idx * barco_ring->descriptor_size);
    
    if (capri_hbm_write_mem(slot_addr, (uint8_t*)req, barco_ring->descriptor_size)) {
        HAL_TRACE_ERR("Failed to write descriptor entry for {}  @ {:x}",
                barco_ring->ring_name,
                (uint64_t) slot_addr); 
        ret = HAL_RET_INVALID_ARG;
    }
    else {
        barco_ring->producer_idx = (barco_ring->producer_idx + 1) & (barco_ring->ring_size - 1);

        /* Barco doorbell */
        hens.dhs_crypto_ctl.pk_producer_idx.fld(barco_ring->producer_idx);
        hens.dhs_crypto_ctl.pk_producer_idx.write();

        *req_tag = barco_ring->opaqe_tag_value++;
    }

    return ret;
}

/*
 * TODO: Decryption currently does not work on xts0. Only initializing and using xts1 for now
 */
hal_ret_t capri_barco_xts_key_array_init(void)
{
    cap_top_csr_t &                     cap0 = CAP_BLK_REG_MODEL_ACCESS(cap_top_csr_t, 0, 0);
    cap_hese_csr_t &                    hese = cap0.md.hese;
    hal_ret_t                           ret = HAL_RET_OK;
    uint64_t                            key_array_base;
    uint32_t                            key_array_key_count;
    char                                key_desc_array[] = CAPRI_BARCO_KEY_DESC;
    uint32_t                            region_sz = 0;

    // Currently sharing the same key descriptor array as GCM
    // Eventually all symmetric protocols will share one large key array
    key_array_base = get_start_offset(key_desc_array);
    /* All regions in hbm_mem.json are in multiples of 1kb and hence should already be aligned to 16byte
     * but confirm
     */
    assert((key_array_base & (BARCO_CRYPTO_KEY_DESC_ALIGN_BYTES - 1)) == 0);
    region_sz = get_size_kb(key_desc_array) * 1024;
    key_array_key_count = region_sz / BARCO_CRYPTO_KEY_DESC_SZ;

    hese.dhs_crypto_ctl.xts_key_array_base_w0.fld(key_array_base & 0xffffffff);
    hese.dhs_crypto_ctl.xts_key_array_base_w0.write();
    hese.dhs_crypto_ctl.xts_key_array_base_w1.fld(key_array_base >> 32);
    hese.dhs_crypto_ctl.xts_key_array_base_w1.write();

    hese.dhs_crypto_ctl.xts_key_array_size.fld(key_array_key_count);
    hese.dhs_crypto_ctl.xts_key_array_size.write();
    HAL_TRACE_DEBUG("Barco xts Key Descriptor Array of count {} setup @ {:x}",
            key_array_key_count, key_array_base);

    return ret;
}


hal_ret_t capri_barco_xts_init(capri_barco_ring_t *barco_ring)
{
    cap_top_csr_t &                     cap0 = CAP_BLK_REG_MODEL_ACCESS(cap_top_csr_t, 0, 0);
    cap_hens_csr_t &                    hens = cap0.md.hens;
    //cap_hese_csr_t &                    hese = cap0.md.hese;
    hal_ret_t                           ret = HAL_RET_OK;

    ret = capri_barco_ring_common_init(barco_ring);
    if (ret != HAL_RET_OK) {
        return ret;
    }

    hens.dhs_crypto_ctl.xts_ring_base_w0.fld((uint32_t)(barco_ring->ring_base & 0xffffffff));
    hens.dhs_crypto_ctl.xts_ring_base_w0.write();
    hens.dhs_crypto_ctl.xts_ring_base_w1.fld((uint32_t)(barco_ring->ring_base >> 32));
    hens.dhs_crypto_ctl.xts_ring_base_w1.write();

    hens.dhs_crypto_ctl.xts_ring_size.fld(barco_ring->ring_size);
    hens.dhs_crypto_ctl.xts_ring_size.write();

    hens.dhs_crypto_ctl.xts_opa_tag_addr_w0.fld((uint32_t)(barco_ring->opaque_tag_addr & 0xffffffff));
    hens.dhs_crypto_ctl.xts_opa_tag_addr_w0.write();
    hens.dhs_crypto_ctl.xts_opa_tag_addr_w1.fld((uint32_t)(barco_ring->opaque_tag_addr >> 32));
    hens.dhs_crypto_ctl.xts_opa_tag_addr_w1.write();

    hens.dhs_crypto_ctl.xts_producer_idx.fld(barco_ring->producer_idx);
    hens.dhs_crypto_ctl.xts_producer_idx.write();

    hens.dhs_crypto_ctl.xts_consumer_idx.fld(barco_ring->consumer_idx);
    hens.dhs_crypto_ctl.xts_consumer_idx.write();

    HAL_TRACE_DEBUG("Barco ring \"{}\" base setup @ {:x}, descriptor count {}",
            barco_ring->ring_name, barco_ring->ring_base, barco_ring->ring_size);

    return capri_barco_xts_key_array_init();
}

bool capri_barco_xts_poller(capri_barco_ring_t *barco_ring, uint32_t req_tag)
{
    /* TBD */
    return FALSE;
}

hal_ret_t capri_barco_mpp_key_array_init(void)
{
    cap_top_csr_t &                     cap0 = CAP_BLK_REG_MODEL_ACCESS(cap_top_csr_t, 0, 0);
    cap_mpse_csr_t &                    mpse = cap0.mp.mpse;
    hal_ret_t                           ret = HAL_RET_OK;
    uint64_t                            key_array_base;
    uint32_t                            key_array_key_count;
    char                                key_desc_array[] = CAPRI_BARCO_KEY_DESC;
    uint32_t                            region_sz = 0;

    // Currently sharing the same key descriptor array as GCM
    // Eventually all symmetric protocols will share one large key array
    key_array_base = get_start_offset(key_desc_array);
    /* All regions in hbm_mem.json are in multiples of 1kb and hence should already be aligned to 16byte
     * but confirm
     */
    assert((key_array_base & (BARCO_CRYPTO_KEY_DESC_ALIGN_BYTES - 1)) == 0);
    region_sz = get_size_kb(key_desc_array) * 1024;
    key_array_key_count = region_sz / BARCO_CRYPTO_KEY_DESC_SZ;

    mpse.dhs_crypto_ctl.mpp0_key_array_base_w0.fld(key_array_base & 0xffffffff);
    mpse.dhs_crypto_ctl.mpp0_key_array_base_w0.write();
    mpse.dhs_crypto_ctl.mpp0_key_array_base_w1.fld(key_array_base >> 32);
    mpse.dhs_crypto_ctl.mpp0_key_array_base_w1.write();

    mpse.dhs_crypto_ctl.mpp0_key_array_size.fld(key_array_key_count);
    mpse.dhs_crypto_ctl.mpp0_key_array_size.write();
    HAL_TRACE_DEBUG("Barco xts Key Descriptor Array of count {} setup @ {:x}",
            key_array_key_count, key_array_base);

    return ret;
}

hal_ret_t capri_barco_mpp_init(capri_barco_ring_t *barco_ring)
{
    cap_top_csr_t &                     cap0 = CAP_BLK_REG_MODEL_ACCESS(cap_top_csr_t, 0, 0);
    cap_mpns_csr_t &                    mpns = cap0.mp.mpns;
    hal_ret_t                           ret = HAL_RET_OK;

    ret = capri_barco_ring_common_init(barco_ring);
    if (ret != HAL_RET_OK) {
        return ret;
    }

    mpns.dhs_crypto_ctl.mpp0_ring_base_w0.fld((uint32_t)(barco_ring->ring_base & 0xffffffff));
    mpns.dhs_crypto_ctl.mpp0_ring_base_w0.write();
    mpns.dhs_crypto_ctl.mpp0_ring_base_w1.fld((uint32_t)(barco_ring->ring_base >> 32));
    mpns.dhs_crypto_ctl.mpp0_ring_base_w1.write();

    mpns.dhs_crypto_ctl.mpp0_ring_size.fld(barco_ring->ring_size);
    mpns.dhs_crypto_ctl.mpp0_ring_size.write();

    mpns.dhs_crypto_ctl.mpp0_opa_tag_addr_w0.fld((uint32_t)(barco_ring->opaque_tag_addr & 0xffffffff));
    mpns.dhs_crypto_ctl.mpp0_opa_tag_addr_w0.write();
    mpns.dhs_crypto_ctl.mpp0_opa_tag_addr_w1.fld((uint32_t)(barco_ring->opaque_tag_addr >> 32));
    mpns.dhs_crypto_ctl.mpp0_opa_tag_addr_w1.write();

    mpns.dhs_crypto_ctl.mpp0_producer_idx.fld(barco_ring->producer_idx);
    mpns.dhs_crypto_ctl.mpp0_producer_idx.write();

    mpns.dhs_crypto_ctl.mpp0_consumer_idx.fld(barco_ring->consumer_idx);
    mpns.dhs_crypto_ctl.mpp0_consumer_idx.write();

    HAL_TRACE_DEBUG("Barco ring \"{}\" base setup @ {:x}, descriptor count {}",
            barco_ring->ring_name, barco_ring->ring_base, barco_ring->ring_size);

    return capri_barco_mpp_key_array_init();
}

bool capri_barco_mpp_poller(capri_barco_ring_t *barco_ring, uint32_t req_tag)
{
    bool                                ret = FALSE;
    uint32_t                            curr_opaque_tag = 0;

    if (capri_hbm_read_mem(barco_ring->opaque_tag_addr, (uint8_t*)&curr_opaque_tag,
			   sizeof(curr_opaque_tag))) {
        HAL_TRACE_ERR("Poll:{}: Failed to retrieve current opaque tag value @ {:x}",
                barco_ring->ring_name, (uint64_t) barco_ring->opaque_tag_addr); 
        return FALSE;
    }
    else {
        HAL_TRACE_DEBUG("Poll:{}: Retrievd opaque tag value: {}", barco_ring->ring_name,
			curr_opaque_tag);
        /* TODO: Handle wraparounds */
        if (curr_opaque_tag >= req_tag)
            ret = TRUE;
    }

    return ret;

#if 0
    cap_top_csr_t &                     cap0 = CAP_BLK_REG_MODEL_ACCESS(cap_top_csr_t, 0, 0);
    cap_mpns_csr_t &                    mpns = cap0.mp.mpns;

    /* TBD - use Doorbell address in req descriptor  to track request completions */
    if (mpns.dhs_crypto_ctl.mpp0_consumer_idx.fld() != barco_ring->consumer_idx) {
        /* New responses available */
        return TRUE;
    }

    return FALSE;
#endif
}

hal_ret_t capri_barco_mpp_queue_request(struct capri_barco_ring_s *barco_ring, void *req,
					uint32_t *req_tag)
{
    cap_top_csr_t &                     cap0 = CAP_BLK_REG_MODEL_ACCESS(cap_top_csr_t, 0, 0);
    cap_mpns_csr_t &                    mpns = cap0.mp.mpns;
    uint64_t                            slot_addr = 0;
    hal_ret_t                           ret = HAL_RET_OK;
    barco_mpp_req_descriptor_t          *sym_req_descr = NULL;

    /*
     * We'll use the opaque-tag address to track response from
     * barco.
     */
    sym_req_descr = (barco_mpp_req_descriptor_t*) req;

    sym_req_descr->opaque_tag_value = barco_ring->opaqe_tag_value;
    sym_req_descr->opaque_tag_wr_en = 1;

#if 0
    /*
     * Use doorbell-address in the symm request descriptor to
     * track response from barco.
     */
    sym_req_descr->doorbell_addr = barco_ring->opaque_tag_addr;
    sym_req_descr->doorbell_data = barco_ring->opaqe_tag_value;
#endif

    slot_addr = barco_ring->ring_base + (barco_ring->producer_idx * barco_ring->descriptor_size);
    
    if (capri_hbm_write_mem(slot_addr, (uint8_t*)req, barco_ring->descriptor_size)) {
        HAL_TRACE_ERR("Failed to write MPP Req descriptor entry for {}  @ {:x}",
                barco_ring->ring_name,
                (uint64_t) slot_addr); 
        ret = HAL_RET_INVALID_ARG;
    }
    else {
        barco_ring->producer_idx = (barco_ring->producer_idx + 1) & (barco_ring->ring_size - 1);

        /* Barco doorbell */
        mpns.dhs_crypto_ctl.mpp0_producer_idx.fld(barco_ring->producer_idx);
        mpns.dhs_crypto_ctl.mpp0_producer_idx.write();

        *req_tag = barco_ring->opaqe_tag_value++;
    }

    return ret;
}

bool capri_barco_ring_poll(types::BarcoRings barco_ring_type, uint32_t req_tag)
{
    capri_barco_ring_t          *barco_ring;

    barco_ring = &barco_rings[barco_ring_type];
    if (barco_ring->poller) {
        return barco_ring->poller(barco_ring, req_tag);
    }
    
    return FALSE;
}

hal_ret_t capri_barco_ring_queue_request(types::BarcoRings barco_ring_type, void *req, uint32_t *req_tag)
{
    capri_barco_ring_t          *barco_ring;

    /* TODO:
     *  - Locking when queue is shared across multiple CPUs
     *  - Queue full check in the common API
     */
    barco_ring = &barco_rings[barco_ring_type];
    return barco_ring->queue_request(barco_ring, req, req_tag);
}

hal_ret_t capri_barco_ring_consume(types::BarcoRings barco_ring_type)
{
    capri_barco_ring_t          *barco_ring;

    barco_ring = &barco_rings[barco_ring_type];
    barco_ring->consumer_idx = (barco_ring->consumer_idx + 1) & (barco_ring->ring_size - 1);
    return HAL_RET_OK;
}


hal_ret_t capri_barco_rings_init(void)
{
    uint16_t        idx;
    uint64_t        opa_tag_addr = 0;
    hal_ret_t       ret = HAL_RET_OK;

    for (idx = 0; idx < types::BarcoRings_MAX; idx++) {
        barco_rings[idx].producer_idx = 0;
        barco_rings[idx].consumer_idx = 0;

	if (barco_rings[idx].ring_size != 0) {
	    barco_rings[idx].opaqe_tag_value = 0x1;
	    /* FIXME: 512 byte is an overkill, on the real target use storage in the ring structure */
	    ret = capri_barco_res_alloc(CRYPTO_BARCO_RES_HBM_MEM_512B, NULL, &opa_tag_addr);
	    if (ret != HAL_RET_OK) {
	        HAL_TRACE_ERR("Failed to allocate opaque tag storage for ring {}",
			      barco_rings[idx].ring_name);
		return ret;
	    }
	    HAL_TRACE_DEBUG("Ring: {}: Allocated opaque tag @ {:x}",
			    barco_rings[idx].ring_name, opa_tag_addr);
	    barco_rings[idx].opaque_tag_addr = opa_tag_addr;
	}
        if (barco_rings[idx].init) {
            ret = barco_rings[idx].init(&barco_rings[idx]);
            if (ret != HAL_RET_OK) {
                HAL_TRACE_ERR("Failed to initialize Barco Ring {}", barco_rings[idx].ring_name);
                return ret;
            }
        }
        //else
        //    break;
    }
    return ret;
}

hal_ret_t get_opaque_tag_addr(types::BarcoRings ring_type, uint64_t* addr)
{
    hal_ret_t       ret = HAL_RET_OK;
    std::string ring_type_str;
    switch(ring_type) {
    case(types::BARCO_RING_ASYM): {
        ring_type_str = BARCO_RING_ASYM_STR;
    } break;
    case(types::BARCO_RING_XTS1):
        ring_type_str = BARCO_RING_XTS_STR;
    break;
    default:
        return HAL_RET_ERR;
    break;
    }

    for (uint32_t idx = 0; idx < types::BarcoRings_MAX; idx++) {
      if(ring_type_str == std::string(barco_rings[idx].ring_name)) {
        *addr = barco_rings[idx].opaque_tag_addr;
        return ret;
      }
    }

  return HAL_RET_ERR;
}



} // namespace pd

} // namespace hal
