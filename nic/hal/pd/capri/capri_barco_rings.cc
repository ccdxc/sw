#include "nic/gen/proto/hal/types.pb.h"
#include "nic/hal/pd/capri/capri_hbm.hpp"
#include "nic/hal/pd/capri/capri_barco_rings.hpp"
//#include "nic/hal/pd/iris/hal_state_pd.hpp"
#include "nic/asic/capri/model/cap_top/cap_top_csr.h"
#include "nic/hal/pd/capri/capri_barco_res.hpp"
#include "nic/hal/pd/capri/capri_barco_crypto.hpp"
#include "nic/hal/src/aclqos/barco_rings.hpp"

namespace hal {

namespace pd {

hal_ret_t capri_barco_asym_init(capri_barco_ring_t *barco_ring);
bool capri_barco_asym_poller(capri_barco_ring_t *barco_ring, uint32_t req_tag);
hal_ret_t capri_barco_asym_queue_request(struct capri_barco_ring_s *barco_ring,
        void *req, uint32_t *req_tag, bool);
hal_ret_t capri_barco_gcm0_init(capri_barco_ring_t *barco_ring);
hal_ret_t capri_barco_gcm1_init(capri_barco_ring_t *barco_ring);
bool capri_barco_gcm0_poller(capri_barco_ring_t *barco_ring, uint32_t req_tag);
bool capri_barco_gcm1_poller(capri_barco_ring_t *barco_ring, uint32_t req_tag);
hal_ret_t capri_barco_gcm0_queue_request(struct capri_barco_ring_s *barco_ring,
                                       void *req, uint32_t *req_tag, bool);
hal_ret_t capri_barco_gcm1_queue_request(struct capri_barco_ring_s *barco_ring,
                                       void *req, uint32_t *req_tag, bool);
hal_ret_t capri_barco_xts0_init(capri_barco_ring_t *barco_ring);
bool capri_barco_xts0_poller(capri_barco_ring_t *barco_ring, uint32_t req_tag);
hal_ret_t capri_barco_xts1_init(capri_barco_ring_t *barco_ring);
bool capri_barco_xts1_poller(capri_barco_ring_t *barco_ring, uint32_t req_tag);
hal_ret_t capri_barco_mpp_queue_request(struct capri_barco_ring_s *barco_ring,
					void *req, uint32_t *req_tag, bool);
hal_ret_t capri_barco_mpp0_init(capri_barco_ring_t *barco_ring);
hal_ret_t capri_barco_mpp1_init(capri_barco_ring_t *barco_ring);
hal_ret_t capri_barco_mpp2_init(capri_barco_ring_t *barco_ring);
hal_ret_t capri_barco_mpp3_init(capri_barco_ring_t *barco_ring);
hal_ret_t capri_barco_cp_init(capri_barco_ring_t *barco_ring);
bool capri_barco_cp_poller(capri_barco_ring_t *barco_ring, uint32_t req_tag);
hal_ret_t capri_barco_cp_hot_init(capri_barco_ring_t *barco_ring);
bool capri_barco_cp_hot_poller(capri_barco_ring_t *barco_ring, uint32_t req_tag);
hal_ret_t capri_barco_dc_init(capri_barco_ring_t *barco_ring);
bool capri_barco_dc_poller(capri_barco_ring_t *barco_ring, uint32_t req_tag);
hal_ret_t capri_barco_dc_hot_init(capri_barco_ring_t *barco_ring);
bool capri_barco_dc_hot_poller(capri_barco_ring_t *barco_ring, uint32_t req_tag);
bool capri_barco_mpp_poller(capri_barco_ring_t *barco_ring, uint32_t req_tag);

capri_barco_ring_t  barco_rings[] = {
    {   // BARCO_RING_ASYM
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
    {   // BARCO_RING_GCM0
        BARCO_RING_GCM0_STR,
        CAPRI_HBM_REG_BRQ,
        0,
        BARCO_CRYPTO_DESC_ALIGN_BYTES,
        1024,
        BARCO_CRYPTO_DESC_SZ,
        0,
        0,
        0,
        0,
        capri_barco_gcm0_init,
        capri_barco_gcm0_poller,
        capri_barco_gcm0_queue_request,
    },
    {   // BARCO_RING_GCM1
        BARCO_RING_GCM1_STR,
        CAPRI_HBM_REG_BARCO_RING_GCM1,
        0,
        BARCO_CRYPTO_DESC_ALIGN_BYTES,
        1024,
        BARCO_CRYPTO_DESC_SZ,
        0,
        0,
        0,
        0,
        capri_barco_gcm1_init,
        capri_barco_gcm1_poller,
        capri_barco_gcm1_queue_request,
    },
    {   // BARCO_RING_XTS0
        BARCO_RING_XTS0_STR,
        CAPRI_HBM_REG_BARCO_RING_XTS0,
        0,
        32,
        1024,
        BARCO_CRYPTO_DESC_SZ,
        0,
        0,
        0,
        0,
        capri_barco_xts0_init,
        capri_barco_xts0_poller,
        NULL,
    },
    {   // BARCO_RING_XTS1
        BARCO_RING_XTS1_STR,
        CAPRI_HBM_REG_BARCO_RING_XTS1,
        0,
        32,
        1024,
        BARCO_CRYPTO_DESC_SZ,
        0,
        0,
        0,
        0,
        capri_barco_xts1_init,
        capri_barco_xts1_poller,
        NULL,
    },
    {   // BARCO_RING_MPP0
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
        capri_barco_mpp0_init,
        capri_barco_mpp_poller,
        capri_barco_mpp_queue_request,
    },
    {   // BARCO_RING_MPP1
        BARCO_RING_MPP1_STR,
        CAPRI_HBM_REG_BARCO_RING_MPP1,
        0,
        128,
        1024,
        128,
        0,
        0,
        0,
        0,
        capri_barco_mpp1_init,
        NULL,
        NULL,
    },
    {   // BARCO_RING_MPP2
        BARCO_RING_MPP2_STR,
        CAPRI_HBM_REG_BARCO_RING_MPP2,
        0,
        128,
        1024,
        128,
        0,
        0,
        0,
        0,
        capri_barco_mpp2_init,
        NULL,
        NULL,
    },
    {   // BARCO_RING_MPP3
        BARCO_RING_MPP3_STR,
        CAPRI_HBM_REG_BARCO_RING_MPP3,
        0,
        128,
        1024,
        128,
        0,
        0,
        0,
        0,
        capri_barco_mpp3_init,
        NULL,
        NULL,
    },
    {   // BARCO_RING_MPP4
    },
    {   // BARCO_RING_MPP5
    },
    {   // BARCO_RING_MPP6
    },
    {   // BARCO_RING_MPP7
    },
    {   // BARCO_RING_CP
        BARCO_RING_CP_STR,
        CAPRI_HBM_REG_BARCO_RING_CP,
        0,
        64,
        BARCO_CRYPTO_CP_RING_SIZE,
        64,
        0,
        0,
        0,
        0,
        capri_barco_cp_init,
        capri_barco_cp_poller,
        NULL,
    },
    {   // BARCO_RING_CP_HOT
        BARCO_RING_CP_HOT_STR,
        CAPRI_HBM_REG_BARCO_RING_CP_HOT,
        0,
        64,
        BARCO_CRYPTO_CP_HOT_RING_SIZE,
        64,
        0,
        0,
        0,
        0,
        capri_barco_cp_hot_init,
        capri_barco_cp_hot_poller,
        NULL,
    },
    {   // BARCO_RING_DC
        BARCO_RING_DC_STR,
        CAPRI_HBM_REG_BARCO_RING_DC,
        0,
        64,
        BARCO_CRYPTO_DC_RING_SIZE,
        64,
        0,
        0,
        0,
        0,
        capri_barco_dc_init,
        capri_barco_dc_poller,
        NULL,
    },
    {   // BARCO_RING_DC_HOT
        BARCO_RING_DC_HOT_STR,
        CAPRI_HBM_REG_BARCO_RING_DC_HOT,
        0,
        64,
        BARCO_CRYPTO_DC_HOT_RING_SIZE,
        64,
        0,
        0,
        0,
        0,
        capri_barco_dc_hot_init,
        capri_barco_dc_hot_poller,
        NULL,
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

    /* Reset Asym ring */
    hens.dhs_crypto_ctl.pk_soft_rst.fld(0xffffffff);
    hens.dhs_crypto_ctl.pk_soft_rst.write();
    /* Bring out of reset */
    hens.dhs_crypto_ctl.pk_soft_rst.fld(0);
    hens.dhs_crypto_ctl.pk_soft_rst.write();

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

    // CI is read-only
    //hens.dhs_crypto_ctl.pk_consumer_idx.fld(barco_ring->consumer_idx);
    //hens.dhs_crypto_ctl.pk_consumer_idx.write();

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
					 void *req, uint32_t *req_tag, bool schedule_barco)
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
 * Only encryption works on xts0 and decryption on xts1
 */
hal_ret_t capri_barco_xts0_key_array_init(void)
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

    hese.dhs_crypto_ctl.xts_enc_key_array_base_w0.fld(key_array_base & 0xffffffff);
    hese.dhs_crypto_ctl.xts_enc_key_array_base_w0.write();
    hese.dhs_crypto_ctl.xts_enc_key_array_base_w1.fld(key_array_base >> 32);
    hese.dhs_crypto_ctl.xts_enc_key_array_base_w1.write();

    hese.dhs_crypto_ctl.xts_enc_key_array_size.fld(key_array_key_count);
    hese.dhs_crypto_ctl.xts_enc_key_array_size.write();
    HAL_TRACE_DEBUG("Barco xts Key Descriptor Array of count {} setup @ {:x}",
            key_array_key_count, key_array_base);

    return ret;
}


hal_ret_t capri_barco_xts0_init(capri_barco_ring_t *barco_ring)
{
    cap_top_csr_t &                     cap0 = CAP_BLK_REG_MODEL_ACCESS(cap_top_csr_t, 0, 0);
    cap_hens_csr_t &                    hens = cap0.md.hens;
    //cap_hese_csr_t &                    hese = cap0.md.hese;
    hal_ret_t                           ret = HAL_RET_OK;

    ret = capri_barco_ring_common_init(barco_ring);
    if (ret != HAL_RET_OK) {
        return ret;
    }

    /* Reset XTS0 ring */
    hens.dhs_crypto_ctl.xts_enc_soft_rst.fld(0xffffffff);
    hens.dhs_crypto_ctl.xts_enc_soft_rst.write();
    /* Bring out of reset */
    hens.dhs_crypto_ctl.xts_enc_soft_rst.fld(0);
    hens.dhs_crypto_ctl.xts_enc_soft_rst.write();

    hens.dhs_crypto_ctl.xts_enc_ring_base_w0.fld((uint32_t)(barco_ring->ring_base & 0xffffffff));
    hens.dhs_crypto_ctl.xts_enc_ring_base_w0.write();
    hens.dhs_crypto_ctl.xts_enc_ring_base_w1.fld((uint32_t)(barco_ring->ring_base >> 32));
    hens.dhs_crypto_ctl.xts_enc_ring_base_w1.write();

    hens.dhs_crypto_ctl.xts_enc_ring_size.fld(barco_ring->ring_size);
    hens.dhs_crypto_ctl.xts_enc_ring_size.write();

    hens.dhs_crypto_ctl.xts_enc_opa_tag_addr_w0.fld((uint32_t)(barco_ring->opaque_tag_addr & 0xffffffff));
    hens.dhs_crypto_ctl.xts_enc_opa_tag_addr_w0.write();
    hens.dhs_crypto_ctl.xts_enc_opa_tag_addr_w1.fld((uint32_t)(barco_ring->opaque_tag_addr >> 32));
    hens.dhs_crypto_ctl.xts_enc_opa_tag_addr_w1.write();

    hens.dhs_crypto_ctl.xts_enc_producer_idx.fld(barco_ring->producer_idx);
    hens.dhs_crypto_ctl.xts_enc_producer_idx.write();

    // CI is read-only
    //hens.dhs_crypto_ctl.xts_enc_consumer_idx.fld(barco_ring->consumer_idx);
    //hens.dhs_crypto_ctl.xts_enc_consumer_idx.write();

    HAL_TRACE_DEBUG("Barco ring \"{}\" base setup @ {:x}, descriptor count {}",
            barco_ring->ring_name, barco_ring->ring_base, barco_ring->ring_size);

    return capri_barco_xts0_key_array_init();
}

bool capri_barco_xts0_poller(capri_barco_ring_t *barco_ring, uint32_t req_tag)
{
    /* TBD */
    return FALSE;
}

hal_ret_t capri_barco_xts1_key_array_init(void)
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


hal_ret_t capri_barco_xts1_init(capri_barco_ring_t *barco_ring)
{
    cap_top_csr_t &                     cap0 = CAP_BLK_REG_MODEL_ACCESS(cap_top_csr_t, 0, 0);
    cap_hens_csr_t &                    hens = cap0.md.hens;
    //cap_hese_csr_t &                    hese = cap0.md.hese;
    hal_ret_t                           ret = HAL_RET_OK;

    ret = capri_barco_ring_common_init(barco_ring);
    if (ret != HAL_RET_OK) {
        return ret;
    }

    /* Reset XTS1 ring */
    hens.dhs_crypto_ctl.xts_soft_rst.fld(0xffffffff);
    hens.dhs_crypto_ctl.xts_soft_rst.write();
    /* Bring out of reset */
    hens.dhs_crypto_ctl.xts_soft_rst.fld(0);
    hens.dhs_crypto_ctl.xts_soft_rst.write();

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

    // CI is read-only
    //hens.dhs_crypto_ctl.xts_consumer_idx.fld(barco_ring->consumer_idx);
    //hens.dhs_crypto_ctl.xts_consumer_idx.write();

    HAL_TRACE_DEBUG("Barco ring \"{}\" base setup @ {:x}, descriptor count {}",
            barco_ring->ring_name, barco_ring->ring_base, barco_ring->ring_size);

    return capri_barco_xts1_key_array_init();
}

bool capri_barco_xts1_poller(capri_barco_ring_t *barco_ring, uint32_t req_tag)
{
    /* TBD */
    return FALSE;
}

hal_ret_t capri_barco_mpp0_key_array_init(void)
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
    HAL_TRACE_DEBUG("Barco MPP0 Key Descriptor Array of count {} setup @ {:x}",
            key_array_key_count, key_array_base);

    return ret;
}

hal_ret_t capri_barco_mpp0_init(capri_barco_ring_t *barco_ring)
{
    cap_top_csr_t &                     cap0 = CAP_BLK_REG_MODEL_ACCESS(cap_top_csr_t, 0, 0);
    cap_mpns_csr_t &                    mpns = cap0.mp.mpns;
    hal_ret_t                           ret = HAL_RET_OK;

    ret = capri_barco_ring_common_init(barco_ring);
    if (ret != HAL_RET_OK) {
        return ret;
    }

    /* Reset MPP0 ring */
    mpns.dhs_crypto_ctl.mpp0_soft_rst.fld(0xffffffff);
    mpns.dhs_crypto_ctl.mpp0_soft_rst.write();
    /* Bring out of reset */
    mpns.dhs_crypto_ctl.mpp0_soft_rst.fld(0);
    mpns.dhs_crypto_ctl.mpp0_soft_rst.write();

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

    // CI is read-only
    //mpns.dhs_crypto_ctl.mpp0_consumer_idx.fld(barco_ring->consumer_idx);
    //mpns.dhs_crypto_ctl.mpp0_consumer_idx.write();

    HAL_TRACE_DEBUG("Barco ring \"{}\" base setup @ {:x}, descriptor count {}",
            barco_ring->ring_name, barco_ring->ring_base, barco_ring->ring_size);

    return capri_barco_mpp0_key_array_init();
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
					uint32_t *req_tag, bool schedule_barco)
{
    cap_top_csr_t &                     cap0 = CAP_BLK_REG_MODEL_ACCESS(cap_top_csr_t, 0, 0);
    cap_mpns_csr_t &                    mpns = cap0.mp.mpns;
    uint64_t                            slot_addr = 0;
    hal_ret_t                           ret = HAL_RET_OK;
    barco_symm_req_descriptor_t         *sym_req_descr = NULL;

    /*
     * We'll use the opaque-tag address to track response from
     * barco.
     */
    sym_req_descr = (barco_symm_req_descriptor_t*) req;

    sym_req_descr->opaque_tag_value = barco_ring->opaqe_tag_value;
    sym_req_descr->opaque_tag_wr_en = 1;

#if 1
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

hal_ret_t capri_barco_mpp1_key_array_init(void)
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

    mpse.dhs_crypto_ctl.mpp1_key_array_base_w0.fld(key_array_base & 0xffffffff);
    mpse.dhs_crypto_ctl.mpp1_key_array_base_w0.write();
    mpse.dhs_crypto_ctl.mpp1_key_array_base_w1.fld(key_array_base >> 32);
    mpse.dhs_crypto_ctl.mpp1_key_array_base_w1.write();

    mpse.dhs_crypto_ctl.mpp1_key_array_size.fld(key_array_key_count);
    mpse.dhs_crypto_ctl.mpp1_key_array_size.write();
    HAL_TRACE_DEBUG("Barco MPP1 Key Descriptor Array of count {} setup @ {:x}",
            key_array_key_count, key_array_base);

    return ret;
}

hal_ret_t capri_barco_mpp1_init(capri_barco_ring_t *barco_ring)
{
    cap_top_csr_t &                     cap0 = CAP_BLK_REG_MODEL_ACCESS(cap_top_csr_t, 0, 0);
    cap_mpns_csr_t &                    mpns = cap0.mp.mpns;
    hal_ret_t                           ret = HAL_RET_OK;

    ret = capri_barco_ring_common_init(barco_ring);
    if (ret != HAL_RET_OK) {
        return ret;
    }

    /* Reset MPP1 ring */
    mpns.dhs_crypto_ctl.mpp1_soft_rst.fld(0xffffffff);
    mpns.dhs_crypto_ctl.mpp1_soft_rst.write();
    /* Bring out of reset */
    mpns.dhs_crypto_ctl.mpp1_soft_rst.fld(0);
    mpns.dhs_crypto_ctl.mpp1_soft_rst.write();

    mpns.dhs_crypto_ctl.mpp1_ring_base_w0.fld((uint32_t)(barco_ring->ring_base & 0xffffffff));
    mpns.dhs_crypto_ctl.mpp1_ring_base_w0.write();
    mpns.dhs_crypto_ctl.mpp1_ring_base_w1.fld((uint32_t)(barco_ring->ring_base >> 32));
    mpns.dhs_crypto_ctl.mpp1_ring_base_w1.write();

    mpns.dhs_crypto_ctl.mpp1_ring_size.fld(barco_ring->ring_size);
    mpns.dhs_crypto_ctl.mpp1_ring_size.write();

    mpns.dhs_crypto_ctl.mpp1_opa_tag_addr_w0.fld((uint32_t)(barco_ring->opaque_tag_addr & 0xffffffff));
    mpns.dhs_crypto_ctl.mpp1_opa_tag_addr_w0.write();
    mpns.dhs_crypto_ctl.mpp1_opa_tag_addr_w1.fld((uint32_t)(barco_ring->opaque_tag_addr >> 32));
    mpns.dhs_crypto_ctl.mpp1_opa_tag_addr_w1.write();

    mpns.dhs_crypto_ctl.mpp1_producer_idx.fld(barco_ring->producer_idx);
    mpns.dhs_crypto_ctl.mpp1_producer_idx.write();

    // CI is read-only
    //mpns.dhs_crypto_ctl.mpp1_consumer_idx.fld(barco_ring->consumer_idx);
    //mpns.dhs_crypto_ctl.mpp1_consumer_idx.write();

    HAL_TRACE_DEBUG("Barco ring \"{}\" base setup @ {:x}, descriptor count {}",
            barco_ring->ring_name, barco_ring->ring_base, barco_ring->ring_size);

    return capri_barco_mpp1_key_array_init();
}

hal_ret_t capri_barco_mpp2_key_array_init(void)
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

    mpse.dhs_crypto_ctl.mpp2_key_array_base_w0.fld(key_array_base & 0xffffffff);
    mpse.dhs_crypto_ctl.mpp2_key_array_base_w0.write();
    mpse.dhs_crypto_ctl.mpp2_key_array_base_w1.fld(key_array_base >> 32);
    mpse.dhs_crypto_ctl.mpp2_key_array_base_w1.write();

    mpse.dhs_crypto_ctl.mpp2_key_array_size.fld(key_array_key_count);
    mpse.dhs_crypto_ctl.mpp2_key_array_size.write();
    HAL_TRACE_DEBUG("Barco MPP2 Key Descriptor Array of count {} setup @ {:x}",
            key_array_key_count, key_array_base);

    return ret;
}

hal_ret_t capri_barco_mpp2_init(capri_barco_ring_t *barco_ring)
{
    cap_top_csr_t &                     cap0 = CAP_BLK_REG_MODEL_ACCESS(cap_top_csr_t, 0, 0);
    cap_mpns_csr_t &                    mpns = cap0.mp.mpns;
    hal_ret_t                           ret = HAL_RET_OK;

    ret = capri_barco_ring_common_init(barco_ring);
    if (ret != HAL_RET_OK) {
        return ret;
    }

    /* Reset MPP2 ring */
    mpns.dhs_crypto_ctl.mpp2_soft_rst.fld(0xffffffff);
    mpns.dhs_crypto_ctl.mpp2_soft_rst.write();
    /* Bring out of reset */
    mpns.dhs_crypto_ctl.mpp2_soft_rst.fld(0);
    mpns.dhs_crypto_ctl.mpp2_soft_rst.write();

    mpns.dhs_crypto_ctl.mpp2_ring_base_w0.fld((uint32_t)(barco_ring->ring_base & 0xffffffff));
    mpns.dhs_crypto_ctl.mpp2_ring_base_w0.write();
    mpns.dhs_crypto_ctl.mpp2_ring_base_w1.fld((uint32_t)(barco_ring->ring_base >> 32));
    mpns.dhs_crypto_ctl.mpp2_ring_base_w1.write();

    mpns.dhs_crypto_ctl.mpp2_ring_size.fld(barco_ring->ring_size);
    mpns.dhs_crypto_ctl.mpp2_ring_size.write();

    mpns.dhs_crypto_ctl.mpp2_opa_tag_addr_w0.fld((uint32_t)(barco_ring->opaque_tag_addr & 0xffffffff));
    mpns.dhs_crypto_ctl.mpp2_opa_tag_addr_w0.write();
    mpns.dhs_crypto_ctl.mpp2_opa_tag_addr_w1.fld((uint32_t)(barco_ring->opaque_tag_addr >> 32));
    mpns.dhs_crypto_ctl.mpp2_opa_tag_addr_w1.write();

    mpns.dhs_crypto_ctl.mpp2_producer_idx.fld(barco_ring->producer_idx);
    mpns.dhs_crypto_ctl.mpp2_producer_idx.write();

    // CI is read-only
    //mpns.dhs_crypto_ctl.mpp2_consumer_idx.fld(barco_ring->consumer_idx);
    //mpns.dhs_crypto_ctl.mpp2_consumer_idx.write();

    HAL_TRACE_DEBUG("Barco ring \"{}\" base setup @ {:x}, descriptor count {}",
            barco_ring->ring_name, barco_ring->ring_base, barco_ring->ring_size);

    return capri_barco_mpp2_key_array_init();
}

hal_ret_t capri_barco_mpp3_key_array_init(void)
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

    mpse.dhs_crypto_ctl.mpp3_key_array_base_w0.fld(key_array_base & 0xffffffff);
    mpse.dhs_crypto_ctl.mpp3_key_array_base_w0.write();
    mpse.dhs_crypto_ctl.mpp3_key_array_base_w1.fld(key_array_base >> 32);
    mpse.dhs_crypto_ctl.mpp3_key_array_base_w1.write();

    mpse.dhs_crypto_ctl.mpp3_key_array_size.fld(key_array_key_count);
    mpse.dhs_crypto_ctl.mpp3_key_array_size.write();
    HAL_TRACE_DEBUG("Barco MPP3 Key Descriptor Array of count {} setup @ {:x}",
            key_array_key_count, key_array_base);

    return ret;
}

hal_ret_t capri_barco_mpp3_init(capri_barco_ring_t *barco_ring)
{
    cap_top_csr_t &                     cap0 = CAP_BLK_REG_MODEL_ACCESS(cap_top_csr_t, 0, 0);
    cap_mpns_csr_t &                    mpns = cap0.mp.mpns;
    hal_ret_t                           ret = HAL_RET_OK;

    ret = capri_barco_ring_common_init(barco_ring);
    if (ret != HAL_RET_OK) {
        return ret;
    }

    /* Reset MPP3 ring */
    mpns.dhs_crypto_ctl.mpp3_soft_rst.fld(0xffffffff);
    mpns.dhs_crypto_ctl.mpp3_soft_rst.write();
    /* Bring out of reset */
    mpns.dhs_crypto_ctl.mpp3_soft_rst.fld(0);
    mpns.dhs_crypto_ctl.mpp3_soft_rst.write();

    mpns.dhs_crypto_ctl.mpp3_ring_base_w0.fld((uint32_t)(barco_ring->ring_base & 0xffffffff));
    mpns.dhs_crypto_ctl.mpp3_ring_base_w0.write();
    mpns.dhs_crypto_ctl.mpp3_ring_base_w1.fld((uint32_t)(barco_ring->ring_base >> 32));
    mpns.dhs_crypto_ctl.mpp3_ring_base_w1.write();

    mpns.dhs_crypto_ctl.mpp3_ring_size.fld(barco_ring->ring_size);
    mpns.dhs_crypto_ctl.mpp3_ring_size.write();

    mpns.dhs_crypto_ctl.mpp3_opa_tag_addr_w0.fld((uint32_t)(barco_ring->opaque_tag_addr & 0xffffffff));
    mpns.dhs_crypto_ctl.mpp3_opa_tag_addr_w0.write();
    mpns.dhs_crypto_ctl.mpp3_opa_tag_addr_w1.fld((uint32_t)(barco_ring->opaque_tag_addr >> 32));
    mpns.dhs_crypto_ctl.mpp3_opa_tag_addr_w1.write();

    mpns.dhs_crypto_ctl.mpp3_producer_idx.fld(barco_ring->producer_idx);
    mpns.dhs_crypto_ctl.mpp3_producer_idx.write();

    // CI is read-only
    //mpns.dhs_crypto_ctl.mpp3_consumer_idx.fld(barco_ring->consumer_idx);
    //mpns.dhs_crypto_ctl.mpp3_consumer_idx.write();

    HAL_TRACE_DEBUG("Barco ring \"{}\" base setup @ {:x}, descriptor count {}",
            barco_ring->ring_name, barco_ring->ring_base, barco_ring->ring_size);

    return capri_barco_mpp3_key_array_init();
}

hal_ret_t capri_barco_gcm0_key_array_init(void)
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
    /* Sanity check that we have enough memory to support the keys scale needed */
    assert(key_array_key_count >= CRYPTO_KEY_COUNT_MAX);

    hese.dhs_crypto_ctl.gcm0_key_array_base_w0.fld(key_array_base & 0xffffffff);
    hese.dhs_crypto_ctl.gcm0_key_array_base_w0.write();
    hese.dhs_crypto_ctl.gcm0_key_array_base_w1.fld(key_array_base >> 32);
    hese.dhs_crypto_ctl.gcm0_key_array_base_w1.write();

    hese.dhs_crypto_ctl.gcm0_key_array_size.fld(key_array_key_count);
    hese.dhs_crypto_ctl.gcm0_key_array_size.write();
    HAL_TRACE_DEBUG("Barco gcm0 Key Descriptor Array of count {} setup @ {:x}",
            key_array_key_count, key_array_base);

    return ret;
}

hal_ret_t capri_barco_gcm0_init(capri_barco_ring_t *barco_ring)
{

    cap_top_csr_t &                     cap0 = CAP_BLK_REG_MODEL_ACCESS(cap_top_csr_t, 0, 0);
    cap_hens_csr_t &                    hens = cap0.md.hens;
    hal_ret_t                           ret = HAL_RET_OK;


    ret = capri_barco_ring_common_init(barco_ring);
    if (ret != HAL_RET_OK) {
        return ret;
    }

    /* Reset GCM0 ring */
    hens.dhs_crypto_ctl.gcm0_soft_rst.fld(0xffffffff);
    hens.dhs_crypto_ctl.gcm0_soft_rst.write();
    /* Bring out of reset */
    hens.dhs_crypto_ctl.gcm0_soft_rst.fld(0);
    hens.dhs_crypto_ctl.gcm0_soft_rst.write();

    hens.dhs_crypto_ctl.gcm0_ring_base_w0.fld((uint32_t)(barco_ring->ring_base & 0xffffffff));
    hens.dhs_crypto_ctl.gcm0_ring_base_w0.write();
    hens.dhs_crypto_ctl.gcm0_ring_base_w1.fld((uint32_t)(barco_ring->ring_base >> 32));
    hens.dhs_crypto_ctl.gcm0_ring_base_w1.write();

    hens.dhs_crypto_ctl.gcm0_ring_size.fld(barco_ring->ring_size);
    hens.dhs_crypto_ctl.gcm0_ring_size.write();

    hens.dhs_crypto_ctl.gcm0_opa_tag_addr_w0.fld((uint32_t)(barco_ring->opaque_tag_addr & 0xffffffff));
    hens.dhs_crypto_ctl.gcm0_opa_tag_addr_w0.write();
    hens.dhs_crypto_ctl.gcm0_opa_tag_addr_w1.fld((uint32_t)(barco_ring->opaque_tag_addr >> 32));
    hens.dhs_crypto_ctl.gcm0_opa_tag_addr_w1.write();

    hens.dhs_crypto_ctl.gcm0_producer_idx.fld(barco_ring->producer_idx);
    hens.dhs_crypto_ctl.gcm0_producer_idx.write();

    return capri_barco_gcm0_key_array_init();
}

bool capri_barco_gcm0_poller(capri_barco_ring_t *barco_ring, uint32_t req_tag)
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
    cap_hens_csr_t &                    hens = cap0.md.hens;

    /* TBD - use Doorbell address in req descriptor  to track request completions */
    if (hens.dhs_crypto_ctl.gcm0_consumer_idx.fld() != barco_ring->consumer_idx) {
        /* New responses available */
        return TRUE;
    }

    return FALSE;
#endif
}

hal_ret_t capri_barco_gcm0_queue_request(struct capri_barco_ring_s *barco_ring, void *req,
			  		uint32_t *req_tag, bool schedule_barco)
{
    cap_top_csr_t &                     cap0 = CAP_BLK_REG_MODEL_ACCESS(cap_top_csr_t, 0, 0);
    cap_hens_csr_t &                    hens = cap0.md.hens;
    uint64_t                            slot_addr = 0;
    hal_ret_t                           ret = HAL_RET_OK;
    barco_symm_req_descriptor_t         *sym_req_descr = NULL;

    /*
     * We'll use the opaque-tag address to track response from
     * barco.
     */
    sym_req_descr = (barco_symm_req_descriptor_t*) req;

    sym_req_descr->opaque_tag_value = barco_ring->opaqe_tag_value;
    sym_req_descr->opaque_tag_wr_en = 1;

#if 1
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

    barco_ring->producer_idx = (barco_ring->producer_idx + 1) & (barco_ring->ring_size - 1);

    if (schedule_barco) {

        /* Barco doorbell */
        hens.dhs_crypto_ctl.gcm0_producer_idx.fld(barco_ring->producer_idx);
        hens.dhs_crypto_ctl.gcm0_producer_idx.write();
    }
    *req_tag = barco_ring->opaqe_tag_value++;

    return ret;
}

hal_ret_t capri_barco_gcm1_key_array_init(void)
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
    /* Sanity check that we have enough memory to support the keys scale needed */
    assert(key_array_key_count >= CRYPTO_KEY_COUNT_MAX);

    hese.dhs_crypto_ctl.gcm1_key_array_base_w0.fld(key_array_base & 0xffffffff);
    hese.dhs_crypto_ctl.gcm1_key_array_base_w0.write();
    hese.dhs_crypto_ctl.gcm1_key_array_base_w1.fld(key_array_base >> 32);
    hese.dhs_crypto_ctl.gcm1_key_array_base_w1.write();

    hese.dhs_crypto_ctl.gcm1_key_array_size.fld(key_array_key_count);
    hese.dhs_crypto_ctl.gcm1_key_array_size.write();
    HAL_TRACE_DEBUG("Barco gcm1 Key Descriptor Array of count {} setup @ {:x}",
            key_array_key_count, key_array_base);

    return ret;
}

hal_ret_t capri_barco_gcm1_init(capri_barco_ring_t *barco_ring)
{

    cap_top_csr_t &                     cap0 = CAP_BLK_REG_MODEL_ACCESS(cap_top_csr_t, 0, 0);
    cap_hens_csr_t &                    hens = cap0.md.hens;
    hal_ret_t                           ret = HAL_RET_OK;


    ret = capri_barco_ring_common_init(barco_ring);
    if (ret != HAL_RET_OK) {
        return ret;
    }

    /* Reset gcm1 ring */
    hens.dhs_crypto_ctl.gcm1_soft_rst.fld(0xffffffff);
    hens.dhs_crypto_ctl.gcm1_soft_rst.write();
    /* Bring out of reset */
    hens.dhs_crypto_ctl.gcm1_soft_rst.fld(0);
    hens.dhs_crypto_ctl.gcm1_soft_rst.write();

    hens.dhs_crypto_ctl.gcm1_ring_base_w0.fld((uint32_t)(barco_ring->ring_base & 0xffffffff));
    hens.dhs_crypto_ctl.gcm1_ring_base_w0.write();
    hens.dhs_crypto_ctl.gcm1_ring_base_w1.fld((uint32_t)(barco_ring->ring_base >> 32));
    hens.dhs_crypto_ctl.gcm1_ring_base_w1.write();

    hens.dhs_crypto_ctl.gcm1_ring_size.fld(barco_ring->ring_size);
    hens.dhs_crypto_ctl.gcm1_ring_size.write();

    hens.dhs_crypto_ctl.gcm1_opa_tag_addr_w0.fld((uint32_t)(barco_ring->opaque_tag_addr & 0xffffffff));
    hens.dhs_crypto_ctl.gcm1_opa_tag_addr_w0.write();
    hens.dhs_crypto_ctl.gcm1_opa_tag_addr_w1.fld((uint32_t)(barco_ring->opaque_tag_addr >> 32));
    hens.dhs_crypto_ctl.gcm1_opa_tag_addr_w1.write();

    hens.dhs_crypto_ctl.gcm1_producer_idx.fld(barco_ring->producer_idx);
    hens.dhs_crypto_ctl.gcm1_producer_idx.write();

    return capri_barco_gcm1_key_array_init();
}


hal_ret_t capri_barco_cp_init(capri_barco_ring_t *barco_ring)
{
    cap_top_csr_t &                     cap0 = CAP_BLK_REG_MODEL_ACCESS(cap_top_csr_t, 0, 0);
    cap_hens_csr_t &                    hens = cap0.md.hens;
    hal_ret_t                           ret = HAL_RET_OK;
    uint32_t                            reg_lo;
    uint32_t                            reg_hi;

    ret = capri_barco_ring_common_init(barco_ring);
    if (ret != HAL_RET_OK) {
        return ret;
    }

    /*
     * Soft reset entire compression block;
     * also set the header version
     */
    hens.dhs_crypto_ctl.cp_cfg_glb.read();
    reg_lo = (hens.dhs_crypto_ctl.cp_cfg_glb.fld().convert_to<uint32_t>() &
              ~BARCO_CRYPTO_CP_CFG_GLB_HDR_VER_MASK)    |
             BARCO_CRYPTO_CP_CFG_GLB_SOFT_RESET         |
             BARCO_CRYPTO_CP_CFG_GLB_HDR_VER;
    hens.dhs_crypto_ctl.cp_cfg_glb.fld(reg_lo);
    hens.dhs_crypto_ctl.cp_cfg_glb.write();

    /*
     * Bring out of reset
     */
    hens.dhs_crypto_ctl.cp_cfg_glb.fld(reg_lo & ~BARCO_CRYPTO_CP_CFG_GLB_SOFT_RESET);
    hens.dhs_crypto_ctl.cp_cfg_glb.write();

    hens.dhs_crypto_ctl.cp_cfg_q_base_adr_w0.fld((uint32_t)(barco_ring->ring_base & 0xffffffff));
    hens.dhs_crypto_ctl.cp_cfg_q_base_adr_w0.write();
    hens.dhs_crypto_ctl.cp_cfg_q_base_adr_w1.fld((uint32_t)(barco_ring->ring_base >> 32));
    hens.dhs_crypto_ctl.cp_cfg_q_base_adr_w1.write();

    /*
     * Enable all 16 cp engines
     */
    hens.dhs_crypto_ctl.cp_cfg_ueng_w0.read();
    reg_lo = hens.dhs_crypto_ctl.cp_cfg_ueng_w0.fld().convert_to<uint32_t>()    |
             BARCO_CRYPTO_CP_UENG_LO_EN_ALL;
    hens.dhs_crypto_ctl.cp_cfg_ueng_w1.read();
    reg_hi = (hens.dhs_crypto_ctl.cp_cfg_ueng_w1.fld().convert_to<uint32_t>()   &
              ~(BARCO_CRYPTO_CP_UENG_HI_CSUM_ON_CP | BARCO_CRYPTO_CP_UENG_HI_HMEM_FILL_ZERO)) |
             BARCO_CRYPTO_CP_UENG_HI_INTEG_APP_STATUS;
    hens.dhs_crypto_ctl.cp_cfg_ueng_w0.fld(reg_lo);
    hens.dhs_crypto_ctl.cp_cfg_ueng_w0.write();
    hens.dhs_crypto_ctl.cp_cfg_ueng_w1.fld(reg_hi);
    hens.dhs_crypto_ctl.cp_cfg_ueng_w1.write();

    /*
     * Enable warm queue
     */
    hens.dhs_crypto_ctl.cp_cfg_dist.read();
    reg_lo = (hens.dhs_crypto_ctl.cp_cfg_dist.fld().convert_to<uint32_t>()  &
              ~BARCO_CRYPTO_CP_DIST_DESC_Q_SIZE(BARCO_CRYPTO_CP_DIST_DESC_Q_SIZE_MASK)) |
             BARCO_CRYPTO_CP_DIST_DESC_Q_EN                                             |
             BARCO_CRYPTO_CP_DIST_DESC_Q_SIZE(BARCO_CRYPTO_CP_RING_SIZE);
    hens.dhs_crypto_ctl.cp_cfg_dist.fld(reg_lo);
    hens.dhs_crypto_ctl.cp_cfg_dist.write();

    hens.dhs_crypto_ctl.cp_cfg_host_opaque_tag_adr_w0.fld((uint32_t)(barco_ring->opaque_tag_addr & 0xffffffff));
    hens.dhs_crypto_ctl.cp_cfg_host_opaque_tag_adr_w0.write();
    hens.dhs_crypto_ctl.cp_cfg_host_opaque_tag_adr_w1.fld((uint32_t)(barco_ring->opaque_tag_addr >> 32));
    hens.dhs_crypto_ctl.cp_cfg_host_opaque_tag_adr_w1.write();
    hens.dhs_crypto_ctl.cp_cfg_host_opaque_tag_data.fld(barco_ring->opaqe_tag_value);
    hens.dhs_crypto_ctl.cp_cfg_host_opaque_tag_data.write();

    hens.dhs_crypto_ctl.cp_cfg_q_pd_idx.fld(barco_ring->producer_idx);
    hens.dhs_crypto_ctl.cp_cfg_q_pd_idx.write();

    HAL_TRACE_DEBUG("Barco compression ring \"{}\" base setup @ {:x}, descriptor count {}",
            barco_ring->ring_name, barco_ring->ring_base, barco_ring->ring_size);

    return HAL_RET_OK;
}

bool capri_barco_cp_poller(capri_barco_ring_t *barco_ring, uint32_t req_tag)
{
    /* TBD */
    return FALSE;
}

hal_ret_t capri_barco_cp_hot_init(capri_barco_ring_t *barco_ring)
{
    cap_top_csr_t &                     cap0 = CAP_BLK_REG_MODEL_ACCESS(cap_top_csr_t, 0, 0);
    cap_hens_csr_t &                    hens = cap0.md.hens;
    hal_ret_t                           ret = HAL_RET_OK;
    uint32_t                            reg_lo;

    ret = capri_barco_ring_common_init(barco_ring);
    if (ret != HAL_RET_OK) {
        return ret;
    }

    hens.dhs_crypto_ctl.cp_cfg_hotq_base_adr_w0.fld((uint32_t)(barco_ring->ring_base & 0xffffffff));
    hens.dhs_crypto_ctl.cp_cfg_hotq_base_adr_w0.write();
    hens.dhs_crypto_ctl.cp_cfg_hotq_base_adr_w1.fld((uint32_t)(barco_ring->ring_base >> 32));
    hens.dhs_crypto_ctl.cp_cfg_hotq_base_adr_w1.write();

    /*
     * Enable hot queue
     */
    hens.dhs_crypto_ctl.cp_cfg_dist.read();
    reg_lo = (hens.dhs_crypto_ctl.cp_cfg_dist.fld().convert_to<uint32_t>()  &
              ~BARCO_CRYPTO_CP_DIST_DESC_HOTQ_SIZE(BARCO_CRYPTO_CP_DIST_DESC_HOTQ_SIZE_MASK))   |
             BARCO_CRYPTO_CP_DIST_DESC_HOTQ_EN                                                  |
             BARCO_CRYPTO_CP_DIST_DESC_HOTQ_SIZE(BARCO_CRYPTO_CP_HOT_RING_SIZE);
    hens.dhs_crypto_ctl.cp_cfg_dist.fld(reg_lo);
    hens.dhs_crypto_ctl.cp_cfg_dist.write();

    hens.dhs_crypto_ctl.cp_cfg_hotq_pd_idx.fld(barco_ring->producer_idx);
    hens.dhs_crypto_ctl.cp_cfg_hotq_pd_idx.write();

    HAL_TRACE_DEBUG("Barco compression hot ring \"{}\" base setup @ {:x}, descriptor count {}",
            barco_ring->ring_name, barco_ring->ring_base, barco_ring->ring_size);

    return HAL_RET_OK;
}

bool capri_barco_cp_hot_poller(capri_barco_ring_t *barco_ring, uint32_t req_tag)
{
    /* TBD */
    return FALSE;
}

hal_ret_t capri_barco_dc_init(capri_barco_ring_t *barco_ring)
{
    cap_top_csr_t &                     cap0 = CAP_BLK_REG_MODEL_ACCESS(cap_top_csr_t, 0, 0);
    cap_hens_csr_t &                    hens = cap0.md.hens;
    hal_ret_t                           ret = HAL_RET_OK;
    uint32_t                            reg_lo;
    uint32_t                            reg_hi;

    ret = capri_barco_ring_common_init(barco_ring);
    if (ret != HAL_RET_OK) {
        return ret;
    }

    /*
     * Soft reset entire decompression block;
     * also set the header version
     */
    hens.dhs_crypto_ctl.dc_cfg_glb.read();
    reg_lo = (hens.dhs_crypto_ctl.dc_cfg_glb.fld().convert_to<uint32_t>() &
              ~BARCO_CRYPTO_DC_CFG_GLB_HDR_VER_MASK)    |
             BARCO_CRYPTO_DC_CFG_GLB_SOFT_RESET         |
             BARCO_CRYPTO_DC_CFG_GLB_HDR_VER;
    hens.dhs_crypto_ctl.dc_cfg_glb.fld(reg_lo);
    hens.dhs_crypto_ctl.dc_cfg_glb.write();

    /*
     * Bring out of reset
     */
    hens.dhs_crypto_ctl.dc_cfg_glb.fld(reg_lo & ~BARCO_CRYPTO_DC_CFG_GLB_SOFT_RESET);
    hens.dhs_crypto_ctl.dc_cfg_glb.write();

    hens.dhs_crypto_ctl.dc_cfg_q_base_adr_w0.fld((uint32_t)(barco_ring->ring_base & 0xffffffff));
    hens.dhs_crypto_ctl.dc_cfg_q_base_adr_w0.write();
    hens.dhs_crypto_ctl.dc_cfg_q_base_adr_w1.fld((uint32_t)(barco_ring->ring_base >> 32));
    hens.dhs_crypto_ctl.dc_cfg_q_base_adr_w1.write();

    /*
     * Enable all 2 dc engines
     */
    hens.dhs_crypto_ctl.dc_cfg_ueng_w0.read();
    reg_lo = hens.dhs_crypto_ctl.dc_cfg_ueng_w0.fld().convert_to<uint32_t>()    |
             BARCO_CRYPTO_DC_UENG_LO_EN_ALL;
    hens.dhs_crypto_ctl.dc_cfg_ueng_w1.read();
    reg_hi = (hens.dhs_crypto_ctl.dc_cfg_ueng_w1.fld().convert_to<uint32_t>()   &
              ~(BARCO_CRYPTO_DC_UENG_HI_CSUM_ON_CP | BARCO_CRYPTO_DC_UENG_HI_HMEM_FILL_ZERO)) |
             BARCO_CRYPTO_DC_UENG_HI_INTEG_APP_STATUS;
    hens.dhs_crypto_ctl.dc_cfg_ueng_w0.fld(reg_lo);
    hens.dhs_crypto_ctl.dc_cfg_ueng_w0.write();
    hens.dhs_crypto_ctl.dc_cfg_ueng_w1.fld(reg_hi);
    hens.dhs_crypto_ctl.dc_cfg_ueng_w1.write();

    /*
     * Enable warm queue
     */
    hens.dhs_crypto_ctl.dc_cfg_dist.read();
    reg_lo = (hens.dhs_crypto_ctl.dc_cfg_dist.fld().convert_to<uint32_t>()  &
              ~BARCO_CRYPTO_DC_DIST_DESC_Q_SIZE(BARCO_CRYPTO_DC_DIST_DESC_Q_SIZE_MASK))   |
             BARCO_CRYPTO_DC_DIST_DESC_Q_EN                                               |
             BARCO_CRYPTO_DC_DIST_DESC_Q_SIZE(BARCO_CRYPTO_DC_RING_SIZE);
    hens.dhs_crypto_ctl.dc_cfg_dist.fld(reg_lo);
    hens.dhs_crypto_ctl.dc_cfg_dist.write();

    hens.dhs_crypto_ctl.dc_cfg_host_opaque_tag_adr_w0.fld((uint32_t)(barco_ring->opaque_tag_addr & 0xffffffff));
    hens.dhs_crypto_ctl.dc_cfg_host_opaque_tag_adr_w0.write();
    hens.dhs_crypto_ctl.dc_cfg_host_opaque_tag_adr_w1.fld((uint32_t)(barco_ring->opaque_tag_addr >> 32));
    hens.dhs_crypto_ctl.dc_cfg_host_opaque_tag_adr_w1.write();
    hens.dhs_crypto_ctl.dc_cfg_host_opaque_tag_data.fld(barco_ring->opaqe_tag_value);
    hens.dhs_crypto_ctl.dc_cfg_host_opaque_tag_data.write();

    hens.dhs_crypto_ctl.dc_cfg_q_pd_idx.fld(barco_ring->producer_idx);
    hens.dhs_crypto_ctl.dc_cfg_q_pd_idx.write();

    HAL_TRACE_DEBUG("Barco decompression ring \"{}\" base setup @ {:x}, descriptor count {}",
            barco_ring->ring_name, barco_ring->ring_base, barco_ring->ring_size);

    return HAL_RET_OK;
}

bool capri_barco_dc_poller(capri_barco_ring_t *barco_ring, uint32_t req_tag)
{
    /* TBD */
    return FALSE;
}

hal_ret_t capri_barco_dc_hot_init(capri_barco_ring_t *barco_ring)
{
    cap_top_csr_t &                     cap0 = CAP_BLK_REG_MODEL_ACCESS(cap_top_csr_t, 0, 0);
    cap_hens_csr_t &                    hens = cap0.md.hens;
    hal_ret_t                           ret = HAL_RET_OK;
    uint32_t                            reg_lo;

    ret = capri_barco_ring_common_init(barco_ring);
    if (ret != HAL_RET_OK) {
        return ret;
    }

    hens.dhs_crypto_ctl.dc_cfg_hotq_base_adr_w0.fld((uint32_t)(barco_ring->ring_base & 0xffffffff));
    hens.dhs_crypto_ctl.dc_cfg_hotq_base_adr_w0.write();
    hens.dhs_crypto_ctl.dc_cfg_hotq_base_adr_w1.fld((uint32_t)(barco_ring->ring_base >> 32));
    hens.dhs_crypto_ctl.dc_cfg_hotq_base_adr_w1.write();

    /*
     * Enable hot queue
     */
    hens.dhs_crypto_ctl.dc_cfg_dist.read();
    reg_lo = (hens.dhs_crypto_ctl.dc_cfg_dist.fld().convert_to<uint32_t>()  &
             ~BARCO_CRYPTO_DC_DIST_DESC_HOTQ_SIZE(BARCO_CRYPTO_DC_DIST_DESC_HOTQ_SIZE_MASK))    |
             BARCO_CRYPTO_DC_DIST_DESC_HOTQ_EN                                                  |
             BARCO_CRYPTO_DC_DIST_DESC_HOTQ_SIZE(BARCO_CRYPTO_DC_HOT_RING_SIZE);
    hens.dhs_crypto_ctl.dc_cfg_dist.fld(reg_lo);
    hens.dhs_crypto_ctl.dc_cfg_dist.write();

    hens.dhs_crypto_ctl.dc_cfg_hotq_pd_idx.fld(barco_ring->producer_idx);
    hens.dhs_crypto_ctl.dc_cfg_hotq_pd_idx.write();

    HAL_TRACE_DEBUG("Barco decompression hot ring \"{}\" base setup @ {:x}, descriptor count {}",
            barco_ring->ring_name, barco_ring->ring_base, barco_ring->ring_size);

    return HAL_RET_OK;
}

bool capri_barco_dc_hot_poller(capri_barco_ring_t *barco_ring, uint32_t req_tag)
{
    /* TBD */
    return FALSE;
}

bool capri_barco_gcm1_poller(capri_barco_ring_t *barco_ring, uint32_t req_tag)
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
    cap_hens_csr_t &                    hens = cap0.md.hens;

    /* TBD - use Doorbell address in req descriptor  to track request completions */
    if (hens.dhs_crypto_ctl.gcm1_consumer_idx.fld() != barco_ring->consumer_idx) {
        /* New responses available */
        return TRUE;
    }

    return FALSE;
#endif
}

hal_ret_t capri_barco_gcm1_queue_request(struct capri_barco_ring_s *barco_ring, void *req,
			  		uint32_t *req_tag, bool schedule_barco)
{
    cap_top_csr_t &                     cap0 = CAP_BLK_REG_MODEL_ACCESS(cap_top_csr_t, 0, 0);
    cap_hens_csr_t &                    hens = cap0.md.hens;
    uint64_t                            slot_addr = 0;
    hal_ret_t                           ret = HAL_RET_OK;
    barco_symm_req_descriptor_t         *sym_req_descr = NULL;

    /*
     * We'll use the opaque-tag address to track response from
     * barco.
     */
    sym_req_descr = (barco_symm_req_descriptor_t*) req;

    sym_req_descr->opaque_tag_value = barco_ring->opaqe_tag_value;
    sym_req_descr->opaque_tag_wr_en = 1;

#if 1
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

    barco_ring->producer_idx = (barco_ring->producer_idx + 1) & (barco_ring->ring_size - 1);

    if (schedule_barco) {

        /* Barco doorbell */
        hens.dhs_crypto_ctl.gcm1_producer_idx.fld(barco_ring->producer_idx);
        hens.dhs_crypto_ctl.gcm1_producer_idx.write();
    }
    *req_tag = barco_ring->opaqe_tag_value++;

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

hal_ret_t capri_barco_ring_queue_request(types::BarcoRings barco_ring_type, void *req, uint32_t *req_tag, bool schedule_barco)
{
    capri_barco_ring_t          *barco_ring;

    /* TODO:
     *  - Locking when queue is shared across multiple CPUs
     *  - Queue full check in the common API
     */
    barco_ring = &barco_rings[barco_ring_type];
    return barco_ring->queue_request(barco_ring, req, req_tag, schedule_barco);
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

    for (idx = 0; idx < types::BarcoRings_ARRAYSIZE; idx++) {
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

    }
    return ret;
}

hal_ret_t get_opaque_tag_addr(types::BarcoRings ring_type, uint64_t* addr)
{
    *addr = barco_rings[ring_type].opaque_tag_addr;
    return HAL_RET_OK;
}

hal_ret_t capri_barco_asym_req_descr_get(uint32_t slot_index, hal::barco_asym_descr_t *asym_req_descr)
{
  /* To be implemented */

  return HAL_RET_OK;
}

hal_ret_t capri_barco_symm_req_descr_get(types::BarcoRings ring_type, uint32_t slot_index,
					 hal::barco_symm_descr_t *symm_req_descr)
{
    barco_symm_req_descriptor_t *req_descr;
    capri_barco_ring_t          *barco_ring;
    uint64_t                    slot_addr;

    barco_ring = &barco_rings[ring_type];
    if (!barco_ring->ring_size || !barco_ring->ring_base) return(HAL_RET_OK);

    uint8_t  value[barco_ring->descriptor_size];
    uint32_t index = (slot_index % barco_ring->ring_size);
    slot_addr = barco_ring->ring_base + (index * barco_ring->descriptor_size);
    HAL_TRACE_DEBUG("{}@{:x}: Ring base {:x}, slot_addr {:x}, read size {:x}",
		    barco_ring->ring_name, barco_ring->ring_size,
            barco_ring->ring_base, (uint64_t) slot_addr,
		    barco_ring->descriptor_size);

    if (capri_hbm_read_mem(slot_addr, value, sizeof(value))) {
        HAL_TRACE_ERR("{}@{:x}: Failed to read Symmetric request descriptor entry",
                barco_ring->ring_name,
                (uint64_t) slot_addr);
        return HAL_RET_INVALID_ARG;
    }

    req_descr = (barco_symm_req_descriptor_t *) value;

    /* Fields already in Little-endian */
    symm_req_descr->ilist_addr = req_descr->input_list_addr;
    symm_req_descr->olist_addr = req_descr->output_list_addr;
    symm_req_descr->command = req_descr->command;
    symm_req_descr->key_desc_index = req_descr->key_descr_idx;
    symm_req_descr->iv_addr = req_descr->iv_address;
    symm_req_descr->status_addr = req_descr->status_addr;
    symm_req_descr->doorbell_addr = req_descr->doorbell_addr;
    symm_req_descr->doorbell_data = req_descr->doorbell_data;
    symm_req_descr->header_size = req_descr->header_size;
    symm_req_descr->second_key_desc_index = req_descr->second_key_descr_idx;

    /* IV is not directly located in the ring, hence dereference it */

    if (req_descr->iv_address) {
        if(capri_hbm_read_mem(req_descr->iv_address,
	    		   (uint8_t*)&symm_req_descr->salt,
	    		   sizeof(symm_req_descr->salt))) {
           HAL_TRACE_ERR("{}@{:x}: Failed to read the Salt information from HBM",
	    	     barco_ring->ring_name, (uint64_t) req_descr->iv_address);
        }
        if(capri_hbm_read_mem(req_descr->iv_address + 4,
	    		   (uint8_t*)&symm_req_descr->explicit_iv,
	    		   sizeof(symm_req_descr->explicit_iv))) {
            HAL_TRACE_ERR("{}@{:x}: Failed to read the explicit IV information from HBM",
	    	      barco_ring->ring_name, (uint64_t) (req_descr->iv_address + 4));
        }
    }
    if (req_descr->status_addr) {
        if(capri_hbm_read_mem(req_descr->status_addr,
	    		   (uint8_t*)&symm_req_descr->barco_status,
	   		   sizeof(symm_req_descr->barco_status))) {
           HAL_TRACE_ERR("{}@{:x}: Failed to read the Barco Status information from HBM",
	    	     barco_ring->ring_name, (uint64_t) req_descr->status_addr);
        }
    }
    return HAL_RET_OK;
}

hal_ret_t capri_barco_ring_meta_get(types::BarcoRings ring_type, uint32_t *pi, uint32_t *ci)
{
    cap_top_csr_t &             cap0 = CAP_BLK_REG_MODEL_ACCESS(cap_top_csr_t, 0, 0);
    cap_mpns_csr_t &            mpns = cap0.mp.mpns;

    /*
     * Read the PI/CI registers for the corresponding barco ring.
     */
    switch(ring_type) {
    case types::BarcoRings::BARCO_RING_MPP0:
      mpns.dhs_crypto_ctl.mpp0_producer_idx.read();
      *pi = mpns.dhs_crypto_ctl.mpp0_producer_idx.fld().convert_to<uint32_t>();
      mpns.dhs_crypto_ctl.mpp0_consumer_idx.read();
      *ci = mpns.dhs_crypto_ctl.mpp0_consumer_idx.fld().convert_to<uint32_t>();
      break;
    case types::BarcoRings::BARCO_RING_MPP1:
      mpns.dhs_crypto_ctl.mpp1_producer_idx.read();
      *pi = mpns.dhs_crypto_ctl.mpp1_producer_idx.fld().convert_to<uint32_t>();
      mpns.dhs_crypto_ctl.mpp1_consumer_idx.read();
      *ci = mpns.dhs_crypto_ctl.mpp1_consumer_idx.fld().convert_to<uint32_t>();
      break;
    case types::BarcoRings::BARCO_RING_MPP2:
      mpns.dhs_crypto_ctl.mpp2_producer_idx.read();
      *pi = mpns.dhs_crypto_ctl.mpp2_producer_idx.fld().convert_to<uint32_t>();
      mpns.dhs_crypto_ctl.mpp2_consumer_idx.read();
      *ci = mpns.dhs_crypto_ctl.mpp2_consumer_idx.fld().convert_to<uint32_t>();
      break;
    case types::BarcoRings::BARCO_RING_MPP3:
      mpns.dhs_crypto_ctl.mpp3_producer_idx.read();
      *pi = mpns.dhs_crypto_ctl.mpp3_producer_idx.fld().convert_to<uint32_t>();
      mpns.dhs_crypto_ctl.mpp3_consumer_idx.read();
      *ci = mpns.dhs_crypto_ctl.mpp3_consumer_idx.fld().convert_to<uint32_t>();
      break;
    default:
      HAL_TRACE_ERR("{}: Ring type not supported yet",
		    barco_rings[ring_type].ring_name);
      return HAL_RET_INVALID_ARG;
      break;
    }
    HAL_TRACE_DEBUG("{}: PI {:x}, CI {:x}",
		    barco_rings[ring_type].ring_name, *pi, *ci);
    return HAL_RET_OK;
}

} // namespace pd

} // namespace hal
