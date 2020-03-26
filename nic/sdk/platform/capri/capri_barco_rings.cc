#include "asic/cmn/asic_common.hpp"
#include "platform/capri/capri_barco_rings.hpp"
#include "third-party/asic/capri/model/cap_top/cap_top_csr.h"
#include "platform/capri/capri_barco_res.hpp"
#include "platform/capri/capri_barco_crypto.hpp"
#include "platform/capri/capri_common.hpp"
#include "platform/capri/capri_hbm_rw.hpp"
#include "asic/rw/asicrw.hpp"
#include "asic/cmn/asic_hbm.hpp"

namespace sdk {
namespace platform {
namespace capri {

sdk_ret_t capri_barco_asym_init(capri_barco_ring_t *barco_ring);
bool capri_barco_asym_poller(capri_barco_ring_t *barco_ring, uint32_t req_tag);
sdk_ret_t capri_barco_asym_queue_request(struct capri_barco_ring_s *barco_ring,
        void *req, uint32_t *req_tag, bool);
sdk_ret_t capri_barco_gcm0_init(capri_barco_ring_t *barco_ring);
sdk_ret_t capri_barco_gcm1_init(capri_barco_ring_t *barco_ring);
bool capri_barco_gcm0_poller(capri_barco_ring_t *barco_ring, uint32_t req_tag);
bool capri_barco_gcm1_poller(capri_barco_ring_t *barco_ring, uint32_t req_tag);
sdk_ret_t capri_barco_gcm0_queue_request(struct capri_barco_ring_s *barco_ring,
                                       void *req, uint32_t *req_tag, bool);
sdk_ret_t capri_barco_gcm1_queue_request(struct capri_barco_ring_s *barco_ring,
                                       void *req, uint32_t *req_tag, bool);
sdk_ret_t capri_barco_xts0_init(capri_barco_ring_t *barco_ring);
bool capri_barco_xts0_poller(capri_barco_ring_t *barco_ring, uint32_t req_tag);
sdk_ret_t capri_barco_xts1_init(capri_barco_ring_t *barco_ring);
bool capri_barco_xts1_poller(capri_barco_ring_t *barco_ring, uint32_t req_tag);
sdk_ret_t capri_barco_mpp_queue_request(struct capri_barco_ring_s *barco_ring,
					void *req, uint32_t *req_tag, bool);
sdk_ret_t capri_barco_mpp0_init(capri_barco_ring_t *barco_ring);
sdk_ret_t capri_barco_mpp1_init(capri_barco_ring_t *barco_ring);
sdk_ret_t capri_barco_mpp2_init(capri_barco_ring_t *barco_ring);
sdk_ret_t capri_barco_mpp3_init(capri_barco_ring_t *barco_ring);
sdk_ret_t capri_barco_cp_init(capri_barco_ring_t *barco_ring);
bool capri_barco_cp_poller(capri_barco_ring_t *barco_ring, uint32_t req_tag);
sdk_ret_t capri_barco_cp_hot_init(capri_barco_ring_t *barco_ring);
bool capri_barco_cp_hot_poller(capri_barco_ring_t *barco_ring, uint32_t req_tag);
sdk_ret_t capri_barco_dc_init(capri_barco_ring_t *barco_ring);
bool capri_barco_dc_poller(capri_barco_ring_t *barco_ring, uint32_t req_tag);
sdk_ret_t capri_barco_dc_hot_init(capri_barco_ring_t *barco_ring);
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
        sizeof(uint32_t),
        sizeof(uint32_t),
        0,
        0,
        true,   // run-time modified based on platform
        false,
        capri_barco_asym_init,
        capri_barco_asym_poller,
        capri_barco_asym_queue_request,
    },
    {   // BARCO_RING_GCM0
        BARCO_RING_GCM0_STR,
        CAPRI_HBM_REG_BARCO_RING_GCM0,
        0,
        BARCO_CRYPTO_DESC_ALIGN_BYTES,
        CAPRI_BARCO_RING_SLOTS,
        BARCO_CRYPTO_DESC_SZ,
        0,
        0,
        0,
        0,
        sizeof(uint32_t),
        sizeof(uint32_t),
        0,
        0,
        true,   // run-time modified based on platform
        false,
        capri_barco_gcm0_init,
        capri_barco_gcm0_poller,
        capri_barco_gcm0_queue_request,
    },
    {   // BARCO_RING_GCM1
        BARCO_RING_GCM1_STR,
        CAPRI_HBM_REG_BARCO_RING_GCM1,
        0,
        BARCO_CRYPTO_DESC_ALIGN_BYTES,
        CAPRI_BARCO_RING_SLOTS,
        BARCO_CRYPTO_DESC_SZ,
        0,
        0,
        0,
        0,
        sizeof(uint32_t),
        sizeof(uint32_t),
        0,
        0,
        true,   // run-time modified based on platform
        false,
        capri_barco_gcm1_init,
        capri_barco_gcm1_poller,
        capri_barco_gcm1_queue_request,
    },
    {   // BARCO_RING_XTS0
        BARCO_RING_XTS0_STR,
        CAPRI_HBM_REG_BARCO_RING_XTS0,
        0,
        32,
        CAPRI_BARCO_XTS_RING_SLOTS,
        BARCO_CRYPTO_DESC_SZ,
        0,
        0,
        0,
        0,
        sizeof(uint32_t),
        sizeof(uint32_t),
        0,
        0,
        true,   // run-time modified based on platform
        false,
        capri_barco_xts0_init,
        capri_barco_xts0_poller,
        NULL,
    },
    {   // BARCO_RING_XTS1
        BARCO_RING_XTS1_STR,
        CAPRI_HBM_REG_BARCO_RING_XTS1,
        0,
        32,
        CAPRI_BARCO_XTS_RING_SLOTS,
        BARCO_CRYPTO_DESC_SZ,
        0,
        0,
        0,
        0,
        sizeof(uint32_t),
        sizeof(uint32_t),
        0,
        0,
        true,   // run-time modified based on platform
        false,
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
        sizeof(uint32_t),
        sizeof(uint32_t),
        0,
        0,
        true,   // run-time modified based on platform
        false,
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
        sizeof(uint32_t),
        sizeof(uint32_t),
        0,
        0,
        true,   // run-time modified based on platform
        false,
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
        sizeof(uint32_t),
        sizeof(uint32_t),
        0,
        0,
        true,   // run-time modified based on platform
        false,
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
        sizeof(uint32_t),
        sizeof(uint32_t),
        0,
        0,
        true,   // run-time modified based on platform
        false,
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
        sizeof(uint32_t),
        sizeof(uint32_t),
        0,
        0,
        true,   // run-time modified based on platform
        true,   //   "        "
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
        sizeof(uint32_t),
        sizeof(uint32_t),
        0,
        0,
        true,   // run-time modified based on platform
        true,   //   "        "
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
        sizeof(uint32_t),
        sizeof(uint32_t),
        0,
        0,
        true,   // run-time modified based on platform
        true,   //   "        "
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
        sizeof(uint32_t),
        sizeof(uint32_t),
        0,
        0,
        true,   // run-time modified based on platform
        true,   //   "        "
        capri_barco_dc_hot_init,
        capri_barco_dc_hot_poller,
        NULL,
    },
};

sdk_ret_t capri_barco_ring_common_init(capri_barco_ring_t *barco_ring)
{
    uint64_t                            ring_base = 0;
    uint32_t                            ring_size = 0;

    ring_base = sdk::asic::asic_get_mem_addr(barco_ring->hbm_region);
    if (ring_base == INVALID_MEM_ADDRESS) {
        SDK_TRACE_ERR("Failed to retrieve Barco Ring memory region for %s", barco_ring->ring_name);
        return SDK_RET_ERR;
    }

    if (ring_base & (barco_ring->ring_alignment - 1)) {
        SDK_TRACE_ERR("Failed to retrieve aligned Barco Ring memory region for %s", barco_ring->ring_name);
        return SDK_RET_ERR;
    }

    ring_size = sdk::asic::asic_get_mem_size_kb(barco_ring->hbm_region) * 1024;
    if (ring_size < (uint32_t)(barco_ring->ring_size * barco_ring->descriptor_size)) {
        SDK_TRACE_ERR("Not enough memory for Barco Ring memory region %s", barco_ring->ring_name);
        return SDK_RET_ERR;
    }

    barco_ring->ring_base = ring_base;

    return SDK_RET_OK;
}

sdk_ret_t capri_barco_asym_key_array_init(void)
{
    cap_top_csr_t &                     cap0 = CAP_BLK_REG_MODEL_ACCESS(cap_top_csr_t, 0, 0);
    cap_hese_csr_t &                    hese = cap0.md.hese;
    sdk_ret_t                           ret = SDK_RET_OK;
    uint64_t                            asym_key_array_base;
    uint32_t                            asym_key_array_key_count;

    ret = capri_barco_res_region_get(CRYPTO_BARCO_RES_ASYM_KEY_DESCR, &asym_key_array_base);
    if (ret != SDK_RET_OK) {
        SDK_TRACE_ERR("Could not retrieve the Asym Crypto Key Descr region");
        return ret;
    }

    ret = capri_barco_res_obj_count_get(CRYPTO_BARCO_RES_ASYM_KEY_DESCR, &asym_key_array_key_count);
    if (ret != SDK_RET_OK) {
        SDK_TRACE_ERR("Could not retrieve the Asym Crypto Key Descr count");
        return ret;
    }

    hese.dhs_crypto_ctl.pk_key_array_base_w0.fld(asym_key_array_base & 0xffffffff);
    hese.dhs_crypto_ctl.pk_key_array_base_w0.write();
    hese.dhs_crypto_ctl.pk_key_array_base_w1.fld(asym_key_array_base >> 32);
    hese.dhs_crypto_ctl.pk_key_array_base_w1.write();

    hese.dhs_crypto_ctl.pk_key_array_size.fld(asym_key_array_key_count);
    hese.dhs_crypto_ctl.pk_key_array_size.write();

    /*
     * Enable the read/write cache for AXI memory accesses for Barco descriptors, data-read,
     * data-write and status/opaque-tag/doorbell writes.
     */
    hese.dhs_crypto_ctl.pk_axi_desc.fld(0x3c0);
    hese.dhs_crypto_ctl.pk_axi_desc.write();
    hese.dhs_crypto_ctl.pk_axi_data_read.fld(0x3c0);
    hese.dhs_crypto_ctl.pk_axi_data_read.write();
    hese.dhs_crypto_ctl.pk_axi_data_write.fld(0x3c0);
    hese.dhs_crypto_ctl.pk_axi_data_write.write();
    hese.dhs_crypto_ctl.pk_axi_status.fld(0x3c0);
    hese.dhs_crypto_ctl.pk_axi_status.write();

    SDK_TRACE_DEBUG("Barco Asym Key Descriptor Array of count %d setup @ 0x%llx",
            asym_key_array_key_count, asym_key_array_base);

    return ret;
}


sdk_ret_t capri_barco_asym_init(capri_barco_ring_t *barco_ring)
{
    cap_top_csr_t &                     cap0 = CAP_BLK_REG_MODEL_ACCESS(cap_top_csr_t, 0, 0);
    cap_hens_csr_t &                    hens = cap0.md.hens;
    //cap_hese_csr_t &                    hese = cap0.md.hese;
    sdk_ret_t                           ret = SDK_RET_OK;

    ret = capri_barco_ring_common_init(barco_ring);
    if (ret != SDK_RET_OK) {
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

    hens.dhs_crypto_ctl.pk_ci_addr_w0.fld((uint32_t)(barco_ring->opaque_tag_addr & 0xffffffff));
    hens.dhs_crypto_ctl.pk_ci_addr_w0.write();
    hens.dhs_crypto_ctl.pk_ci_addr_w1.fld((uint32_t)(barco_ring->opaque_tag_addr >> 32));
    hens.dhs_crypto_ctl.pk_ci_addr_w1.write();

    hens.dhs_crypto_ctl.pk_producer_idx.fld(barco_ring->producer_idx);
    hens.dhs_crypto_ctl.pk_producer_idx.write();
    barco_ring->producer_idx_addr = hens.dhs_crypto_ctl.pk_producer_idx.get_offset();

    // CI is read-only
    //hens.dhs_crypto_ctl.pk_consumer_idx.fld(barco_ring->consumer_idx);
    //hens.dhs_crypto_ctl.pk_consumer_idx.write();

    SDK_TRACE_DEBUG("Barco ring \"%s\" base setup @ 0x%llx, descriptor count %d",
            barco_ring->ring_name, barco_ring->ring_base, barco_ring->ring_size);

    return capri_barco_asym_key_array_init();
}


bool capri_barco_asym_poller(capri_barco_ring_t *barco_ring, uint32_t req_tag)
{
    bool                                ret = FALSE;
    uint32_t                            curr_opaque_tag = 0;

    /* The opaque tag address is used as the CI address for the Asym ring */
    if (sdk::asic::asic_mem_read(barco_ring->opaque_tag_addr, (uint8_t*)&curr_opaque_tag, sizeof(curr_opaque_tag))) {
        SDK_TRACE_ERR("Poll:%s: Failed to retrieve current opaque tag value @ 0x%llx",
                barco_ring->ring_name, (uint64_t) barco_ring->opaque_tag_addr);
        return FALSE;
    }
    else {
        if ((int32_t(curr_opaque_tag - req_tag)) > 0) {
            SDK_TRACE_DEBUG("Poll:%s: Check for req: %d: Retrieved opaque tag addr: 0x%llx value: %d", 
                    barco_ring->ring_name, req_tag, barco_ring->opaque_tag_addr, curr_opaque_tag);

            ret = TRUE;
        }
    }

    return ret;
}

sdk_ret_t capri_barco_asym_queue_request(struct capri_barco_ring_s *barco_ring,
					 void *req, uint32_t *req_tag, bool schedule_barco)
{
    cap_top_csr_t &                     cap0 = CAP_BLK_REG_MODEL_ACCESS(cap_top_csr_t, 0, 0);
    cap_hens_csr_t &                    hens = cap0.md.hens;
    uint64_t                            slot_addr = 0;
    sdk_ret_t                           ret = SDK_RET_OK;

    slot_addr = barco_ring->ring_base + (barco_ring->producer_idx * barco_ring->descriptor_size);

    if (sdk::asic::asic_mem_write(slot_addr, (uint8_t*)req, barco_ring->descriptor_size)) {
        SDK_TRACE_ERR("Failed to write descriptor entry for %s  @ 0x%llx",
                barco_ring->ring_name,
                (uint64_t) slot_addr);
        ret = SDK_RET_INVALID_ARG;
    }
    else {
        *req_tag = barco_ring->producer_idx;;
        barco_ring->producer_idx = (barco_ring->producer_idx + 1) & (barco_ring->ring_size - 1);

        /* Barco doorbell */
        hens.dhs_crypto_ctl.pk_producer_idx.fld(barco_ring->producer_idx);
        hens.dhs_crypto_ctl.pk_producer_idx.write();

    }

    return ret;
}

/*
 * Only encryption works on xts0 and decryption on xts1
 */
sdk_ret_t capri_barco_xts0_key_array_init(void)
{
    cap_top_csr_t &                     cap0 = CAP_BLK_REG_MODEL_ACCESS(cap_top_csr_t, 0, 0);
    cap_hese_csr_t &                    hese = cap0.md.hese;
    sdk_ret_t                           ret = SDK_RET_OK;
    uint64_t                            key_array_base;
    uint32_t                            key_array_key_count;
    char                                key_desc_array[] = CAPRI_BARCO_KEY_DESC;
    uint32_t                            region_sz = 0;

    // Currently sharing the same key descriptor array as GCM
    // Eventually all symmetric protocols will share one large key array
    key_array_base = sdk::asic::asic_get_mem_addr(key_desc_array);
    SDK_ASSERT(key_array_base != INVALID_MEM_ADDRESS);
    /* All regions in hbm_mem.json are in multiples of 1kb and hence should already be aligned to 16byte
     * but confirm
     */
    assert((key_array_base & (BARCO_CRYPTO_KEY_DESC_ALIGN_BYTES - 1)) == 0);
    region_sz = sdk::asic::asic_get_mem_size_kb(key_desc_array) * 1024;
    key_array_key_count = region_sz / BARCO_CRYPTO_KEY_DESC_SZ;

    hese.dhs_crypto_ctl.xts_enc_key_array_base_w0.fld(key_array_base & 0xffffffff);
    hese.dhs_crypto_ctl.xts_enc_key_array_base_w0.write();
    hese.dhs_crypto_ctl.xts_enc_key_array_base_w1.fld(key_array_base >> 32);
    hese.dhs_crypto_ctl.xts_enc_key_array_base_w1.write();

    hese.dhs_crypto_ctl.xts_enc_key_array_size.fld(key_array_key_count);
    hese.dhs_crypto_ctl.xts_enc_key_array_size.write();
    SDK_TRACE_DEBUG("Barco xts Key Descriptor Array of count %d setup @ 0x%llx",
            key_array_key_count, key_array_base);

    return ret;
}


sdk_ret_t capri_barco_xts0_init(capri_barco_ring_t *barco_ring)
{
    cap_top_csr_t &                     cap0 = CAP_BLK_REG_MODEL_ACCESS(cap_top_csr_t, 0, 0);
    cap_hens_csr_t &                    hens = cap0.md.hens;
    //cap_hese_csr_t &                    hese = cap0.md.hese;
    sdk_ret_t                           ret = SDK_RET_OK;

    ret = capri_barco_ring_common_init(barco_ring);
    if (ret != SDK_RET_OK) {
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
    barco_ring->producer_idx_addr = hens.dhs_crypto_ctl.xts_enc_producer_idx.get_offset();

    // CI is read-only
    //hens.dhs_crypto_ctl.xts_enc_consumer_idx.fld(barco_ring->consumer_idx);
    //hens.dhs_crypto_ctl.xts_enc_consumer_idx.write();

    SDK_TRACE_DEBUG("Barco ring \"%s\" base setup @ 0x%llx, descriptor count %d",
            barco_ring->ring_name, barco_ring->ring_base, barco_ring->ring_size);

    return capri_barco_xts0_key_array_init();
}

bool capri_barco_xts0_poller(capri_barco_ring_t *barco_ring, uint32_t req_tag)
{
    /* TBD */
    return FALSE;
}

sdk_ret_t capri_barco_xts1_key_array_init(void)
{
    cap_top_csr_t &                     cap0 = CAP_BLK_REG_MODEL_ACCESS(cap_top_csr_t, 0, 0);
    cap_hese_csr_t &                    hese = cap0.md.hese;
    sdk_ret_t                           ret = SDK_RET_OK;
    uint64_t                            key_array_base;
    uint32_t                            key_array_key_count;
    char                                key_desc_array[] = CAPRI_BARCO_KEY_DESC;
    uint32_t                            region_sz = 0;

    // Currently sharing the same key descriptor array as GCM
    // Eventually all symmetric protocols will share one large key array
    key_array_base = sdk::asic::asic_get_mem_addr(key_desc_array);
    SDK_ASSERT(key_array_base != INVALID_MEM_ADDRESS);
    /* All regions in hbm_mem.json are in multiples of 1kb and hence should already be aligned to 16byte
     * but confirm
     */
    assert((key_array_base & (BARCO_CRYPTO_KEY_DESC_ALIGN_BYTES - 1)) == 0);
    region_sz = sdk::asic::asic_get_mem_size_kb(key_desc_array) * 1024;
    key_array_key_count = region_sz / BARCO_CRYPTO_KEY_DESC_SZ;

    hese.dhs_crypto_ctl.xts_key_array_base_w0.fld(key_array_base & 0xffffffff);
    hese.dhs_crypto_ctl.xts_key_array_base_w0.write();
    hese.dhs_crypto_ctl.xts_key_array_base_w1.fld(key_array_base >> 32);
    hese.dhs_crypto_ctl.xts_key_array_base_w1.write();

    hese.dhs_crypto_ctl.xts_key_array_size.fld(key_array_key_count);
    hese.dhs_crypto_ctl.xts_key_array_size.write();
    SDK_TRACE_DEBUG("Barco xts Key Descriptor Array of count %d setup @ 0x%llx",
            key_array_key_count, key_array_base);

    return ret;
}


sdk_ret_t capri_barco_xts1_init(capri_barco_ring_t *barco_ring)
{
    cap_top_csr_t &                     cap0 = CAP_BLK_REG_MODEL_ACCESS(cap_top_csr_t, 0, 0);
    cap_hens_csr_t &                    hens = cap0.md.hens;
    //cap_hese_csr_t &                    hese = cap0.md.hese;
    sdk_ret_t                           ret = SDK_RET_OK;

    ret = capri_barco_ring_common_init(barco_ring);
    if (ret != SDK_RET_OK) {
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
    barco_ring->producer_idx_addr = hens.dhs_crypto_ctl.xts_producer_idx.get_offset();

    // CI is read-only
    //hens.dhs_crypto_ctl.xts_consumer_idx.fld(barco_ring->consumer_idx);
    //hens.dhs_crypto_ctl.xts_consumer_idx.write();

    SDK_TRACE_DEBUG("Barco ring \"%s\" base setup @ 0x%llx, descriptor count %d",
            barco_ring->ring_name, barco_ring->ring_base, barco_ring->ring_size);

    return capri_barco_xts1_key_array_init();
}

bool capri_barco_xts1_poller(capri_barco_ring_t *barco_ring, uint32_t req_tag)
{
    /* TBD */
    return FALSE;
}

sdk_ret_t capri_barco_mpp0_key_array_init(void)
{
    cap_top_csr_t &                     cap0 = CAP_BLK_REG_MODEL_ACCESS(cap_top_csr_t, 0, 0);
    cap_mpse_csr_t &                    mpse = cap0.mp.mpse;
    sdk_ret_t                           ret = SDK_RET_OK;
    uint64_t                            key_array_base;
    uint32_t                            key_array_key_count;
    char                                key_desc_array[] = CAPRI_BARCO_KEY_DESC;
    uint32_t                            region_sz = 0;

    // Currently sharing the same key descriptor array as GCM
    // Eventually all symmetric protocols will share one large key array
    key_array_base = sdk::asic::asic_get_mem_addr(key_desc_array);
    SDK_ASSERT(key_array_base != INVALID_MEM_ADDRESS);
    /* All regions in hbm_mem.json are in multiples of 1kb and hence should already be aligned to 16byte
     * but confirm
     */
    assert((key_array_base & (BARCO_CRYPTO_KEY_DESC_ALIGN_BYTES - 1)) == 0);
    region_sz = sdk::asic::asic_get_mem_size_kb(key_desc_array) * 1024;
    key_array_key_count = region_sz / BARCO_CRYPTO_KEY_DESC_SZ;

    mpse.dhs_crypto_ctl.mpp0_key_array_base_w0.fld(key_array_base & 0xffffffff);
    mpse.dhs_crypto_ctl.mpp0_key_array_base_w0.write();
    mpse.dhs_crypto_ctl.mpp0_key_array_base_w1.fld(key_array_base >> 32);
    mpse.dhs_crypto_ctl.mpp0_key_array_base_w1.write();

    mpse.dhs_crypto_ctl.mpp0_key_array_size.fld(key_array_key_count);
    mpse.dhs_crypto_ctl.mpp0_key_array_size.write();

    /*
     * Enable the read/write cache for AXI memory accesses for Barco descriptors, data-read,
     * data-write and status/opaque-tag/doorbell writes.
     */
    mpse.dhs_crypto_ctl.mpp0_axi_desc.fld(0x3c0);
    mpse.dhs_crypto_ctl.mpp0_axi_desc.write();
    mpse.dhs_crypto_ctl.mpp0_axi_data_read.fld(0x3c0);
    mpse.dhs_crypto_ctl.mpp0_axi_data_read.write();
    mpse.dhs_crypto_ctl.mpp0_axi_data_write.fld(0x3c0);
    mpse.dhs_crypto_ctl.mpp0_axi_data_write.write();
    mpse.dhs_crypto_ctl.mpp0_axi_status.fld(0x3c0);
    mpse.dhs_crypto_ctl.mpp0_axi_status.write();

    SDK_TRACE_DEBUG("Barco MPP0 Key Descriptor Array of count %d setup @ 0x%llx",
            key_array_key_count, key_array_base);

    return ret;
}

sdk_ret_t capri_barco_mpp0_init(capri_barco_ring_t *barco_ring)
{
    cap_top_csr_t &                     cap0 = CAP_BLK_REG_MODEL_ACCESS(cap_top_csr_t, 0, 0);
    cap_mpns_csr_t &                    mpns = cap0.mp.mpns;
    sdk_ret_t                           ret = SDK_RET_OK;

    ret = capri_barco_ring_common_init(barco_ring);
    if (ret != SDK_RET_OK) {
        return ret;
    }

    /* FIXME: This should be done prior to per ring initialization */
    mpns.cfg_mp_ctl.clk_en(1);
    mpns.cfg_mp_ctl.sw_rst(0);
    mpns.cfg_mp_ctl.write();


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
    barco_ring->producer_idx_addr = mpns.dhs_crypto_ctl.mpp0_producer_idx.get_offset();

    // CI is read-only
    //mpns.dhs_crypto_ctl.mpp0_consumer_idx.fld(barco_ring->consumer_idx);
    //mpns.dhs_crypto_ctl.mpp0_consumer_idx.write();

    SDK_TRACE_DEBUG("Barco ring \"%s\" base setup @ 0x%llx, descriptor count %d",
            barco_ring->ring_name, barco_ring->ring_base, barco_ring->ring_size);

    return capri_barco_mpp0_key_array_init();
}

bool capri_barco_mpp_poller(capri_barco_ring_t *barco_ring, uint32_t req_tag)
{
    bool                                ret = FALSE;
    uint32_t                            curr_opaque_tag = 0;

    if (sdk::asic::asic_mem_read(barco_ring->opaque_tag_addr, (uint8_t*)&curr_opaque_tag,
			   sizeof(curr_opaque_tag))) {
        SDK_TRACE_ERR("Poll:%s: Failed to retrieve current opaque tag value @ 0x%llx",
                barco_ring->ring_name, (uint64_t) barco_ring->opaque_tag_addr);
        return FALSE;
    }
    else {
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

sdk_ret_t capri_barco_mpp_queue_request(struct capri_barco_ring_s *barco_ring, void *req,
					uint32_t *req_tag, bool schedule_barco)
{
    cap_top_csr_t &                     cap0 = CAP_BLK_REG_MODEL_ACCESS(cap_top_csr_t, 0, 0);
    cap_mpns_csr_t &                    mpns = cap0.mp.mpns;
    uint64_t                            slot_addr = 0;
    sdk_ret_t                           ret = SDK_RET_OK;
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

    if (sdk::asic::asic_mem_write(slot_addr, (uint8_t*)req, barco_ring->descriptor_size)) {
        SDK_TRACE_ERR("Failed to write MPP Req descriptor entry for %s  @ 0x%llx",
                barco_ring->ring_name,
                (uint64_t) slot_addr);
        ret = SDK_RET_INVALID_ARG;
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

sdk_ret_t capri_barco_mpp1_key_array_init(void)
{
    cap_top_csr_t &                     cap0 = CAP_BLK_REG_MODEL_ACCESS(cap_top_csr_t, 0, 0);
    cap_mpse_csr_t &                    mpse = cap0.mp.mpse;
    sdk_ret_t                           ret = SDK_RET_OK;
    uint64_t                            key_array_base;
    uint32_t                            key_array_key_count;
    char                                key_desc_array[] = CAPRI_BARCO_KEY_DESC;
    uint32_t                            region_sz = 0;

    // Currently sharing the same key descriptor array as GCM
    // Eventually all symmetric protocols will share one large key array
    key_array_base = sdk::asic::asic_get_mem_addr(key_desc_array);
    SDK_ASSERT(key_array_base != INVALID_MEM_ADDRESS);
    /* All regions in hbm_mem.json are in multiples of 1kb and hence should already be aligned to 16byte
     * but confirm
     */
    assert((key_array_base & (BARCO_CRYPTO_KEY_DESC_ALIGN_BYTES - 1)) == 0);
    region_sz = sdk::asic::asic_get_mem_size_kb(key_desc_array) * 1024;
    key_array_key_count = region_sz / BARCO_CRYPTO_KEY_DESC_SZ;

    mpse.dhs_crypto_ctl.mpp1_key_array_base_w0.fld(key_array_base & 0xffffffff);
    mpse.dhs_crypto_ctl.mpp1_key_array_base_w0.write();
    mpse.dhs_crypto_ctl.mpp1_key_array_base_w1.fld(key_array_base >> 32);
    mpse.dhs_crypto_ctl.mpp1_key_array_base_w1.write();

    mpse.dhs_crypto_ctl.mpp1_key_array_size.fld(key_array_key_count);
    mpse.dhs_crypto_ctl.mpp1_key_array_size.write();

    /*
     * Enable the read/write cache for AXI memory accesses for Barco descriptors, data-read,
     * data-write and status/opaque-tag/doorbell writes.
     */
    mpse.dhs_crypto_ctl.mpp1_axi_desc.fld(0x3c0);
    mpse.dhs_crypto_ctl.mpp1_axi_desc.write();
    mpse.dhs_crypto_ctl.mpp1_axi_data_read.fld(0x3c0);
    mpse.dhs_crypto_ctl.mpp1_axi_data_read.write();
    mpse.dhs_crypto_ctl.mpp1_axi_data_write.fld(0x3c0);
    mpse.dhs_crypto_ctl.mpp1_axi_data_write.write();
    mpse.dhs_crypto_ctl.mpp1_axi_status.fld(0x3c0);
    mpse.dhs_crypto_ctl.mpp1_axi_status.write();

    SDK_TRACE_DEBUG("Barco MPP1 Key Descriptor Array of count %d setup @ 0x%llx",
            key_array_key_count, key_array_base);

    return ret;
}

sdk_ret_t capri_barco_mpp1_init(capri_barco_ring_t *barco_ring)
{
    cap_top_csr_t &                     cap0 = CAP_BLK_REG_MODEL_ACCESS(cap_top_csr_t, 0, 0);
    cap_mpns_csr_t &                    mpns = cap0.mp.mpns;
    sdk_ret_t                           ret = SDK_RET_OK;

    ret = capri_barco_ring_common_init(barco_ring);
    if (ret != SDK_RET_OK) {
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
    barco_ring->producer_idx_addr = mpns.dhs_crypto_ctl.mpp1_producer_idx.get_offset();

    // CI is read-only
    //mpns.dhs_crypto_ctl.mpp1_consumer_idx.fld(barco_ring->consumer_idx);
    //mpns.dhs_crypto_ctl.mpp1_consumer_idx.write();

    SDK_TRACE_DEBUG("Barco ring \"%s\" base setup @ 0x%llx, descriptor count %d",
            barco_ring->ring_name, barco_ring->ring_base, barco_ring->ring_size);

    return capri_barco_mpp1_key_array_init();
}

sdk_ret_t capri_barco_mpp2_key_array_init(void)
{
    cap_top_csr_t &                     cap0 = CAP_BLK_REG_MODEL_ACCESS(cap_top_csr_t, 0, 0);
    cap_mpse_csr_t &                    mpse = cap0.mp.mpse;
    sdk_ret_t                           ret = SDK_RET_OK;
    uint64_t                            key_array_base;
    uint32_t                            key_array_key_count;
    char                                key_desc_array[] = CAPRI_BARCO_KEY_DESC;
    uint32_t                            region_sz = 0;

    // Currently sharing the same key descriptor array as GCM
    // Eventually all symmetric protocols will share one large key array
    key_array_base = sdk::asic::asic_get_mem_addr(key_desc_array);
    SDK_ASSERT(key_array_base != INVALID_MEM_ADDRESS);
    /* All regions in hbm_mem.json are in multiples of 1kb and hence should already be aligned to 16byte
     * but confirm
     */
    assert((key_array_base & (BARCO_CRYPTO_KEY_DESC_ALIGN_BYTES - 1)) == 0);
    region_sz = sdk::asic::asic_get_mem_size_kb(key_desc_array) * 1024;
    key_array_key_count = region_sz / BARCO_CRYPTO_KEY_DESC_SZ;

    mpse.dhs_crypto_ctl.mpp2_key_array_base_w0.fld(key_array_base & 0xffffffff);
    mpse.dhs_crypto_ctl.mpp2_key_array_base_w0.write();
    mpse.dhs_crypto_ctl.mpp2_key_array_base_w1.fld(key_array_base >> 32);
    mpse.dhs_crypto_ctl.mpp2_key_array_base_w1.write();

    mpse.dhs_crypto_ctl.mpp2_key_array_size.fld(key_array_key_count);
    mpse.dhs_crypto_ctl.mpp2_key_array_size.write();

    /*
     * Enable the read/write cache for AXI memory accesses for Barco descriptors, data-read,
     * data-write and status/opaque-tag/doorbell writes.
     */
    mpse.dhs_crypto_ctl.mpp2_axi_desc.fld(0x3c0);
    mpse.dhs_crypto_ctl.mpp2_axi_desc.write();
    mpse.dhs_crypto_ctl.mpp2_axi_data_read.fld(0x3c0);
    mpse.dhs_crypto_ctl.mpp2_axi_data_read.write();
    mpse.dhs_crypto_ctl.mpp2_axi_data_write.fld(0x3c0);
    mpse.dhs_crypto_ctl.mpp2_axi_data_write.write();
    mpse.dhs_crypto_ctl.mpp2_axi_status.fld(0x3c0);
    mpse.dhs_crypto_ctl.mpp2_axi_status.write();

    SDK_TRACE_DEBUG("Barco MPP2 Key Descriptor Array of count %d setup @ 0x%llx",
            key_array_key_count, key_array_base);

    return ret;
}

sdk_ret_t capri_barco_mpp2_init(capri_barco_ring_t *barco_ring)
{
    cap_top_csr_t &                     cap0 = CAP_BLK_REG_MODEL_ACCESS(cap_top_csr_t, 0, 0);
    cap_mpns_csr_t &                    mpns = cap0.mp.mpns;
    sdk_ret_t                           ret = SDK_RET_OK;

    ret = capri_barco_ring_common_init(barco_ring);
    if (ret != SDK_RET_OK) {
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
    barco_ring->producer_idx_addr = mpns.dhs_crypto_ctl.mpp2_producer_idx.get_offset();

    // CI is read-only
    //mpns.dhs_crypto_ctl.mpp2_consumer_idx.fld(barco_ring->consumer_idx);
    //mpns.dhs_crypto_ctl.mpp2_consumer_idx.write();

    SDK_TRACE_DEBUG("Barco ring \"%s\" base setup @ 0x%llx, descriptor count %d",
            barco_ring->ring_name, barco_ring->ring_base, barco_ring->ring_size);

    return capri_barco_mpp2_key_array_init();
}

sdk_ret_t capri_barco_mpp3_key_array_init(void)
{
    cap_top_csr_t &                     cap0 = CAP_BLK_REG_MODEL_ACCESS(cap_top_csr_t, 0, 0);
    cap_mpse_csr_t &                    mpse = cap0.mp.mpse;
    sdk_ret_t                           ret = SDK_RET_OK;
    uint64_t                            key_array_base;
    uint32_t                            key_array_key_count;
    char                                key_desc_array[] = CAPRI_BARCO_KEY_DESC;
    uint32_t                            region_sz = 0;

    // Currently sharing the same key descriptor array as GCM
    // Eventually all symmetric protocols will share one large key array
    key_array_base = sdk::asic::asic_get_mem_addr(key_desc_array);
    SDK_ASSERT(key_array_base != INVALID_MEM_ADDRESS);
    /* All regions in hbm_mem.json are in multiples of 1kb and hence should already be aligned to 16byte
     * but confirm
     */
    assert((key_array_base & (BARCO_CRYPTO_KEY_DESC_ALIGN_BYTES - 1)) == 0);
    region_sz = sdk::asic::asic_get_mem_size_kb(key_desc_array) * 1024;
    key_array_key_count = region_sz / BARCO_CRYPTO_KEY_DESC_SZ;

    mpse.dhs_crypto_ctl.mpp3_key_array_base_w0.fld(key_array_base & 0xffffffff);
    mpse.dhs_crypto_ctl.mpp3_key_array_base_w0.write();
    mpse.dhs_crypto_ctl.mpp3_key_array_base_w1.fld(key_array_base >> 32);
    mpse.dhs_crypto_ctl.mpp3_key_array_base_w1.write();

    mpse.dhs_crypto_ctl.mpp3_key_array_size.fld(key_array_key_count);
    mpse.dhs_crypto_ctl.mpp3_key_array_size.write();

    /*
     * Enable the read/write cache for AXI memory accesses for Barco descriptors, data-read,
     * data-write and status/opaque-tag/doorbell writes.
     */
    mpse.dhs_crypto_ctl.mpp3_axi_desc.fld(0x3c0);
    mpse.dhs_crypto_ctl.mpp3_axi_desc.write();
    mpse.dhs_crypto_ctl.mpp3_axi_data_read.fld(0x3c0);
    mpse.dhs_crypto_ctl.mpp3_axi_data_read.write();
    mpse.dhs_crypto_ctl.mpp3_axi_data_write.fld(0x3c0);
    mpse.dhs_crypto_ctl.mpp3_axi_data_write.write();
    mpse.dhs_crypto_ctl.mpp3_axi_status.fld(0x3c0);
    mpse.dhs_crypto_ctl.mpp3_axi_status.write();

    SDK_TRACE_DEBUG("Barco MPP3 Key Descriptor Array of count %d setup @ 0x%llx",
            key_array_key_count, key_array_base);

    return ret;
}

sdk_ret_t capri_barco_mpp3_init(capri_barco_ring_t *barco_ring)
{
    cap_top_csr_t &                     cap0 = CAP_BLK_REG_MODEL_ACCESS(cap_top_csr_t, 0, 0);
    cap_mpns_csr_t &                    mpns = cap0.mp.mpns;
    sdk_ret_t                           ret = SDK_RET_OK;

    ret = capri_barco_ring_common_init(barco_ring);
    if (ret != SDK_RET_OK) {
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
    barco_ring->producer_idx_addr = mpns.dhs_crypto_ctl.mpp3_producer_idx.get_offset();

    // CI is read-only
    //mpns.dhs_crypto_ctl.mpp3_consumer_idx.fld(barco_ring->consumer_idx);
    //mpns.dhs_crypto_ctl.mpp3_consumer_idx.write();

    SDK_TRACE_DEBUG("Barco ring \"%s\" base setup @ 0x%llx, descriptor count %d",
            barco_ring->ring_name, barco_ring->ring_base, barco_ring->ring_size);

    return capri_barco_mpp3_key_array_init();
}

sdk_ret_t capri_barco_gcm0_key_array_init(void)
{
    cap_top_csr_t &                     cap0 = CAP_BLK_REG_MODEL_ACCESS(cap_top_csr_t, 0, 0);
    cap_hese_csr_t &                    hese = cap0.md.hese;
    sdk_ret_t                           ret = SDK_RET_OK;
    uint64_t                            key_array_base;
    uint32_t                            key_array_key_count;
    char                                key_desc_array[] = CAPRI_BARCO_KEY_DESC;
    uint32_t                            region_sz = 0;

    // Currently sharing the same key descriptor array as GCM
    // Eventually all symmetric protocols will share one large key array
    key_array_base = sdk::asic::asic_get_mem_addr(key_desc_array);
    SDK_ASSERT(key_array_base != INVALID_MEM_ADDRESS);
    /* All regions in hbm_mem.json are in multiples of 1kb and hence should already be aligned to 16byte
     * but confirm
     */
    assert((key_array_base & (BARCO_CRYPTO_KEY_DESC_ALIGN_BYTES - 1)) == 0);
    region_sz = sdk::asic::asic_get_mem_size_kb(key_desc_array) * 1024;
    key_array_key_count = region_sz / BARCO_CRYPTO_KEY_DESC_SZ;
    /* Sanity check that we have enough memory to support the keys scale needed */
    assert(key_array_key_count >= CRYPTO_KEY_COUNT_MAX);

    hese.dhs_crypto_ctl.gcm0_key_array_base_w0.fld(key_array_base & 0xffffffff);
    hese.dhs_crypto_ctl.gcm0_key_array_base_w0.write();
    hese.dhs_crypto_ctl.gcm0_key_array_base_w1.fld(key_array_base >> 32);
    hese.dhs_crypto_ctl.gcm0_key_array_base_w1.write();

    hese.dhs_crypto_ctl.gcm0_key_array_size.fld(key_array_key_count);
    hese.dhs_crypto_ctl.gcm0_key_array_size.write();

    /*
     * Enable the read/write cache for AXI memory accesses for Barco descriptors, data-read,
     * data-write and status/opaque-tag/doorbell writes.
     */
    hese.dhs_crypto_ctl.gcm0_axi_desc.fld(0x3c0);
    hese.dhs_crypto_ctl.gcm0_axi_desc.write();
    hese.dhs_crypto_ctl.gcm0_axi_data_read.fld(0x3c0);
    hese.dhs_crypto_ctl.gcm0_axi_data_read.write();
    hese.dhs_crypto_ctl.gcm0_axi_data_write.fld(0x3c0);
    hese.dhs_crypto_ctl.gcm0_axi_data_write.write();
    hese.dhs_crypto_ctl.gcm0_axi_status.fld(0x3c0);
    hese.dhs_crypto_ctl.gcm0_axi_status.write();

    SDK_TRACE_DEBUG("Barco gcm0 Key Descriptor Array of count %d setup @ 0x%llx",
            key_array_key_count, key_array_base);

    return ret;
}

sdk_ret_t capri_barco_gcm0_init(capri_barco_ring_t *barco_ring)
{

    cap_top_csr_t &                     cap0 = CAP_BLK_REG_MODEL_ACCESS(cap_top_csr_t, 0, 0);
    cap_hens_csr_t &                    hens = cap0.md.hens;
    sdk_ret_t                           ret = SDK_RET_OK;
    uint32_t                            gcm0_pi, gcm0_ci;


    ret = capri_barco_ring_common_init(barco_ring);
    if (ret != SDK_RET_OK) {
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

    gcm0_pi = hens.dhs_crypto_ctl.gcm0_producer_idx.fld().convert_to<uint32_t>();
    gcm0_ci = hens.dhs_crypto_ctl.gcm0_consumer_idx.fld().convert_to<uint32_t>();
    barco_ring->producer_idx_addr = hens.dhs_crypto_ctl.gcm0_producer_idx.get_offset();

    assert((gcm0_pi == 0) && (gcm0_ci == 0));

    return capri_barco_gcm0_key_array_init();
}

bool capri_barco_gcm0_poller(capri_barco_ring_t *barco_ring, uint32_t req_tag)
{
    bool                                ret = FALSE;
    uint32_t                            curr_opaque_tag = 0;

    if (sdk::asic::asic_mem_read(barco_ring->opaque_tag_addr, (uint8_t*)&curr_opaque_tag,
			   sizeof(curr_opaque_tag))) {
        SDK_TRACE_ERR("Poll:%s: Failed to retrieve current opaque tag value @ 0x%llx",
                barco_ring->ring_name, (uint64_t) barco_ring->opaque_tag_addr);
        return FALSE;
    }
    else {
        SDK_TRACE_DEBUG("Poll:%s: Retrievd opaque tag value: %d", barco_ring->ring_name,
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

sdk_ret_t capri_barco_gcm0_queue_request(struct capri_barco_ring_s *barco_ring, void *req,
			  		uint32_t *req_tag, bool schedule_barco)
{
    cap_top_csr_t &                     cap0 = CAP_BLK_REG_MODEL_ACCESS(cap_top_csr_t, 0, 0);
    cap_hens_csr_t &                    hens = cap0.md.hens;
    uint64_t                            slot_addr = 0;
    sdk_ret_t                           ret = SDK_RET_OK;
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

    if (sdk::asic::asic_mem_write(slot_addr, (uint8_t*)req, barco_ring->descriptor_size)) {
        SDK_TRACE_ERR("Failed to write MPP Req descriptor entry for %s  @ 0x%llx",
                barco_ring->ring_name,
                (uint64_t) slot_addr);
        ret = SDK_RET_INVALID_ARG;
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

sdk_ret_t capri_barco_gcm1_key_array_init(void)
{
    cap_top_csr_t &                     cap0 = CAP_BLK_REG_MODEL_ACCESS(cap_top_csr_t, 0, 0);
    cap_hese_csr_t &                    hese = cap0.md.hese;
    sdk_ret_t                           ret = SDK_RET_OK;
    uint64_t                            key_array_base;
    uint32_t                            key_array_key_count;
    char                                key_desc_array[] = CAPRI_BARCO_KEY_DESC;
    uint32_t                            region_sz = 0;

    // Currently sharing the same key descriptor array as GCM
    // Eventually all symmetric protocols will share one large key array
    key_array_base = sdk::asic::asic_get_mem_addr(key_desc_array);
    SDK_ASSERT(key_array_base != INVALID_MEM_ADDRESS);
    /* All regions in hbm_mem.json are in multiples of 1kb and hence should already be aligned to 16byte
     * but confirm
     */
    assert((key_array_base & (BARCO_CRYPTO_KEY_DESC_ALIGN_BYTES - 1)) == 0);
    region_sz = sdk::asic::asic_get_mem_size_kb(key_desc_array) * 1024;
    key_array_key_count = region_sz / BARCO_CRYPTO_KEY_DESC_SZ;
    /* Sanity check that we have enough memory to support the keys scale needed */
    assert(key_array_key_count >= CRYPTO_KEY_COUNT_MAX);

    hese.dhs_crypto_ctl.gcm1_key_array_base_w0.fld(key_array_base & 0xffffffff);
    hese.dhs_crypto_ctl.gcm1_key_array_base_w0.write();
    hese.dhs_crypto_ctl.gcm1_key_array_base_w1.fld(key_array_base >> 32);
    hese.dhs_crypto_ctl.gcm1_key_array_base_w1.write();

    hese.dhs_crypto_ctl.gcm1_key_array_size.fld(key_array_key_count);
    hese.dhs_crypto_ctl.gcm1_key_array_size.write();

    /*
     * Enable the read/write cache for AXI memory accesses for Barco descriptors, data-read,
     * data-write and status/opaque-tag/doorbell writes.
     */
    hese.dhs_crypto_ctl.gcm1_axi_desc.fld(0x3c0);
    hese.dhs_crypto_ctl.gcm1_axi_desc.write();
    hese.dhs_crypto_ctl.gcm1_axi_data_read.fld(0x3c0);
    hese.dhs_crypto_ctl.gcm1_axi_data_read.write();
    hese.dhs_crypto_ctl.gcm1_axi_data_write.fld(0x3c0);
    hese.dhs_crypto_ctl.gcm1_axi_data_write.write();
    hese.dhs_crypto_ctl.gcm1_axi_status.fld(0x3c0);
    hese.dhs_crypto_ctl.gcm1_axi_status.write();

    SDK_TRACE_DEBUG("Barco gcm1 Key Descriptor Array of count %d setup @ 0x%llx",
            key_array_key_count, key_array_base);

    return ret;
}

sdk_ret_t capri_barco_gcm1_init(capri_barco_ring_t *barco_ring)
{

    cap_top_csr_t &                     cap0 = CAP_BLK_REG_MODEL_ACCESS(cap_top_csr_t, 0, 0);
    cap_hens_csr_t &                    hens = cap0.md.hens;
    sdk_ret_t                           ret = SDK_RET_OK;
    uint32_t                            gcm1_pi, gcm1_ci;


    ret = capri_barco_ring_common_init(barco_ring);
    if (ret != SDK_RET_OK) {
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

    gcm1_pi = hens.dhs_crypto_ctl.gcm1_producer_idx.fld().convert_to<uint32_t>();
    gcm1_ci = hens.dhs_crypto_ctl.gcm1_consumer_idx.fld().convert_to<uint32_t>();
    barco_ring->producer_idx_addr = hens.dhs_crypto_ctl.gcm1_producer_idx.get_offset();

    assert((gcm1_pi == 0) && (gcm1_ci == 0));


    return capri_barco_gcm1_key_array_init();
}


sdk_ret_t capri_barco_cp_init(capri_barco_ring_t *barco_ring)
{
    cap_top_csr_t &                     cap0 = CAP_BLK_REG_MODEL_ACCESS(cap_top_csr_t, 0, 0);
    cap_hens_csr_t &                    hens = cap0.md.hens;
    sdk_ret_t                           ret = SDK_RET_OK;
    uint32_t                            reg_lo;
    uint32_t                            reg_hi;

    ret = capri_barco_ring_common_init(barco_ring);
    if (ret != SDK_RET_OK) {
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
              ~BARCO_CRYPTO_CP_UENG_HI_HMEM_FILL_ZERO) | 
             (BARCO_CRYPTO_CP_UENG_HI_SHA_DATA_UNCOMP  | BARCO_CRYPTO_CP_UENG_HI_CSUM_ON_UNCOMP |
              BARCO_CRYPTO_CP_UENG_HI_INTEG_APP_STATUS);
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
    barco_ring->producer_idx_addr = hens.dhs_crypto_ctl.cp_cfg_q_pd_idx.get_offset();

    SDK_TRACE_DEBUG("Barco compression ring \"%s\" base setup @ 0x%llx, descriptor count %d",
            barco_ring->ring_name, barco_ring->ring_base, barco_ring->ring_size);

    return SDK_RET_OK;
}

bool capri_barco_cp_poller(capri_barco_ring_t *barco_ring, uint32_t req_tag)
{
    /* TBD */
    return FALSE;
}

sdk_ret_t capri_barco_cp_hot_init(capri_barco_ring_t *barco_ring)
{
    cap_top_csr_t &                     cap0 = CAP_BLK_REG_MODEL_ACCESS(cap_top_csr_t, 0, 0);
    cap_hens_csr_t &                    hens = cap0.md.hens;
    sdk_ret_t                           ret = SDK_RET_OK;
    uint32_t                            reg_lo;

    ret = capri_barco_ring_common_init(barco_ring);
    if (ret != SDK_RET_OK) {
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
    barco_ring->producer_idx_addr = hens.dhs_crypto_ctl.cp_cfg_hotq_pd_idx.get_offset();

    SDK_TRACE_DEBUG("Barco compression hot ring \"%s\" base setup @ 0x%llx, descriptor count %d",
            barco_ring->ring_name, barco_ring->ring_base, barco_ring->ring_size);

    return SDK_RET_OK;
}

bool capri_barco_cp_hot_poller(capri_barco_ring_t *barco_ring, uint32_t req_tag)
{
    /* TBD */
    return FALSE;
}

sdk_ret_t capri_barco_dc_init(capri_barco_ring_t *barco_ring)
{
    cap_top_csr_t &                     cap0 = CAP_BLK_REG_MODEL_ACCESS(cap_top_csr_t, 0, 0);
    cap_hens_csr_t &                    hens = cap0.md.hens;
    sdk_ret_t                           ret = SDK_RET_OK;
    uint32_t                            reg_lo;
    uint32_t                            reg_hi;

    ret = capri_barco_ring_common_init(barco_ring);
    if (ret != SDK_RET_OK) {
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
              ~BARCO_CRYPTO_DC_UENG_HI_HMEM_FILL_ZERO) |
             (BARCO_CRYPTO_DC_UENG_HI_SHA_DATA_UNCOMP  | BARCO_CRYPTO_DC_UENG_HI_CSUM_ON_UNCOMP |
              BARCO_CRYPTO_DC_UENG_HI_INTEG_APP_STATUS);
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
    barco_ring->producer_idx_addr = hens.dhs_crypto_ctl.dc_cfg_q_pd_idx.get_offset();

    SDK_TRACE_DEBUG("Barco decompression ring \"%s\" base setup @ 0x%llx, descriptor count %d",
            barco_ring->ring_name, barco_ring->ring_base, barco_ring->ring_size);

    return SDK_RET_OK;
}

bool capri_barco_dc_poller(capri_barco_ring_t *barco_ring, uint32_t req_tag)
{
    /* TBD */
    return FALSE;
}

sdk_ret_t capri_barco_dc_hot_init(capri_barco_ring_t *barco_ring)
{
    cap_top_csr_t &                     cap0 = CAP_BLK_REG_MODEL_ACCESS(cap_top_csr_t, 0, 0);
    cap_hens_csr_t &                    hens = cap0.md.hens;
    sdk_ret_t                           ret = SDK_RET_OK;
    uint32_t                            reg_lo;

    ret = capri_barco_ring_common_init(barco_ring);
    if (ret != SDK_RET_OK) {
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
    barco_ring->producer_idx_addr = hens.dhs_crypto_ctl.dc_cfg_hotq_pd_idx.get_offset();

    SDK_TRACE_DEBUG("Barco decompression hot ring \"%s\" base setup @ 0x%llx, descriptor count %d",
            barco_ring->ring_name, barco_ring->ring_base, barco_ring->ring_size);

    return SDK_RET_OK;
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

    if (sdk::asic::asic_mem_read(barco_ring->opaque_tag_addr, (uint8_t*)&curr_opaque_tag,
			   sizeof(curr_opaque_tag))) {
        SDK_TRACE_ERR("Poll:%s: Failed to retrieve current opaque tag value @ 0x%llx",
                barco_ring->ring_name, (uint64_t) barco_ring->opaque_tag_addr);
        return FALSE;
    }
    else {
        SDK_TRACE_DEBUG("Poll:%s: Retrievd opaque tag value: %s", barco_ring->ring_name,
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

sdk_ret_t capri_barco_gcm1_queue_request(struct capri_barco_ring_s *barco_ring, void *req,
			  		uint32_t *req_tag, bool schedule_barco)
{
    cap_top_csr_t &                     cap0 = CAP_BLK_REG_MODEL_ACCESS(cap_top_csr_t, 0, 0);
    cap_hens_csr_t &                    hens = cap0.md.hens;
    uint64_t                            slot_addr = 0;
    sdk_ret_t                           ret = SDK_RET_OK;
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

    if (sdk::asic::asic_mem_write(slot_addr, (uint8_t*)req, barco_ring->descriptor_size)) {
        SDK_TRACE_ERR("Failed to write MPP Req descriptor entry for %s  @ 0x%llx",
                barco_ring->ring_name,
                (uint64_t) slot_addr);
        ret = SDK_RET_INVALID_ARG;
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

bool capri_barco_ring_poll(barco_rings_t barco_ring_type, uint32_t req_tag)
{
    capri_barco_ring_t          *barco_ring;

    barco_ring = &barco_rings[barco_ring_type];
    if (barco_ring->poller) {
        return barco_ring->poller(barco_ring, req_tag);
    }

    return FALSE;
}

sdk_ret_t capri_barco_ring_queue_request(barco_rings_t barco_ring_type, void *req, uint32_t *req_tag, bool schedule_barco)
{
    capri_barco_ring_t          *barco_ring;

    /* TODO:
     *  - Locking when queue is shared across multiple CPUs
     *  - Queue full check in the common API
     */
    barco_ring = &barco_rings[barco_ring_type];
    return barco_ring->queue_request(barco_ring, req, req_tag, schedule_barco);
}

sdk_ret_t capri_barco_ring_consume(barco_rings_t barco_ring_type)
{
    capri_barco_ring_t          *barco_ring;

    barco_ring = &barco_rings[barco_ring_type];
    barco_ring->consumer_idx = (barco_ring->consumer_idx + 1) & (barco_ring->ring_size - 1);
    return SDK_RET_OK;
}


sdk_ret_t capri_barco_rings_init(platform_type_t platform)
{
    uint16_t        idx;
    uint64_t        opa_tag_addr = 0;
    uint32_t        opa_tag_def_val = 0;
    uint64_t        shadow_pndx_addr = 0;
    uint64_t        shadow_pndx_def_val = 0;
    uint64_t        shadow_pndx_total = 0;
    uint32_t        shadow_pndx_size = 0;
    sdk_ret_t       ret = SDK_RET_OK;

    shadow_pndx_total = CRYPTO_BARCO_RES_HBM_MEM_512B_SIZE;
    ret = capri_barco_res_alloc(CRYPTO_BARCO_RES_HBM_MEM_512B, NULL, &shadow_pndx_addr);
    if (ret != SDK_RET_OK) {
        SDK_TRACE_ERR("Failed to allocate shadow pndx storage for rings");
        return ret;
    }
    for (idx = 0; idx < BARCO_RING_MAX; idx++) {
        barco_rings[idx].producer_idx = 0;
        barco_rings[idx].consumer_idx = 0;

        if (barco_rings[idx].ring_size != 0) {
            barco_rings[idx].opaqe_tag_value = 0x1;
            
            opa_tag_addr = sdk::asic::asic_get_mem_addr(CAPRI_HBM_REG_OPAQUE_TAG) + get_opaque_tag_offset((barco_rings_t)idx);

            SDK_TRACE_DEBUG("Ring: %s: Allocated opaque tag @ 0x%llx",
                barco_rings[idx].ring_name, opa_tag_addr);
            if(sdk::asic::asic_mem_write(opa_tag_addr, (uint8_t *)&opa_tag_def_val, sizeof(opa_tag_def_val))) {
                SDK_TRACE_ERR("Ring: %s: Failed to initialized opaque tag @ 0x%llx",
                    barco_rings[idx].ring_name, opa_tag_addr);
                return SDK_RET_HW_PROGRAM_ERR;
            } 
            SDK_TRACE_DEBUG("Ring: %s: initialized opaque tag to 0 @ 0x%llx",
                barco_rings[idx].ring_name, opa_tag_addr);

            barco_rings[idx].opaque_tag_addr = opa_tag_addr;
        }
        shadow_pndx_size = barco_rings[idx].shadow_pndx_size;
        if (shadow_pndx_size) {
            assert(shadow_pndx_size <= sizeof(shadow_pndx_def_val));
            if (shadow_pndx_total < shadow_pndx_size) {
                SDK_TRACE_ERR("Out of shadow pndx storage for ring %s",
                              barco_rings[idx].ring_name);
                return SDK_RET_HW_PROGRAM_ERR;
            }
            barco_rings[idx].shadow_pndx_addr = shadow_pndx_addr;
            if(sdk::asic::asic_mem_write(shadow_pndx_addr, (uint8_t *)&shadow_pndx_def_val,
                                   shadow_pndx_size)) {
                SDK_TRACE_ERR("Ring: %s: Failed to initialize shadow pndx @ 0x%llx",
                              barco_rings[idx].ring_name, shadow_pndx_addr);
                return SDK_RET_HW_PROGRAM_ERR;
            } 
            SDK_TRACE_DEBUG("Ring: %s: initialized shadow pndx to 0 @ 0x%llx",
                            barco_rings[idx].ring_name, shadow_pndx_addr);
            shadow_pndx_addr += shadow_pndx_size;
            shadow_pndx_total -= shadow_pndx_size;
        }

        /* Model does not support sw_reset */
        if (barco_rings[idx].sw_reset_capable) {
            if ((platform != platform_type_t::PLATFORM_TYPE_HAPS) &&
                (platform != platform_type_t::PLATFORM_TYPE_HW)) {
                barco_rings[idx].sw_reset_capable = false;
            }
        }
        if (barco_rings[idx].init) {
            ret = barco_rings[idx].init(&barco_rings[idx]);
            if (ret != SDK_RET_OK) {
                SDK_TRACE_ERR("Failed to initialize Barco Ring %s", barco_rings[idx].ring_name);
                return ret;
            }
        }

    }
    return ret;
}

sdk_ret_t get_opaque_tag_addr(barco_rings_t ring_type, uint64_t* addr)
{
    *addr = barco_rings[ring_type].opaque_tag_addr;
    return SDK_RET_OK;
}

sdk_ret_t capri_barco_get_meta_config_info(barco_rings_t ring_type,
                                           barco_ring_meta_config_t *meta)
{
    meta->shadow_pndx_addr = barco_rings[ring_type].shadow_pndx_addr;
    meta->desc_size = barco_rings[ring_type].descriptor_size;
    meta->pndx_size = barco_rings[ring_type].shadow_pndx_size;
    meta->opaque_tag_size = barco_rings[ring_type].opaque_tag_size;
    meta->ring_base = barco_rings[ring_type].ring_base;
    meta->producer_idx_addr = barco_rings[ring_type].producer_idx_addr;
    meta->opaque_tag_addr = barco_rings[ring_type].opaque_tag_addr;
    meta->ring_size = barco_rings[ring_type].ring_size;
    return SDK_RET_OK;
}
sdk_ret_t capri_barco_get_capabilities(barco_rings_t ring_type,
                                       bool *sw_reset_capable, bool *sw_enable_capable)
{
    *sw_reset_capable = barco_rings[ring_type].sw_reset_capable;
    *sw_enable_capable = barco_rings[ring_type].sw_enable_capable;
    return SDK_RET_OK;
}

sdk_ret_t capri_barco_asym_req_descr_get(uint32_t slot_index, barco_asym_descr_t *asym_req_descr)
{
  /* To be implemented */

  return SDK_RET_OK;
}

sdk_ret_t capri_barco_symm_req_descr_get(barco_rings_t ring_type, uint32_t slot_index,
					 barco_symm_descr_t *symm_req_descr)
{
    barco_symm_req_descriptor_t *req_descr;
    capri_barco_ring_t          *barco_ring;
    uint64_t                    slot_addr;

    barco_ring = &barco_rings[ring_type];
    if (!barco_ring->ring_size || !barco_ring->ring_base) return(SDK_RET_OK);

    uint8_t  value[barco_ring->descriptor_size];
    uint32_t index = (slot_index % barco_ring->ring_size);
    slot_addr = barco_ring->ring_base + (index * barco_ring->descriptor_size);
    SDK_TRACE_DEBUG("%s@%d: Ring base 0x%llx, slot_addr 0x%llx, read size %d",
		    barco_ring->ring_name, barco_ring->ring_size,
            barco_ring->ring_base, (uint64_t) slot_addr,
		    barco_ring->descriptor_size);

    if (sdk::asic::asic_mem_read(slot_addr, value, sizeof(value))) {
        SDK_TRACE_ERR("%s@0x%llx: Failed to read Symmetric request descriptor entry",
                barco_ring->ring_name,
                (uint64_t) slot_addr);
        return SDK_RET_INVALID_ARG;
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
        if(sdk::asic::asic_mem_read(req_descr->iv_address,
	    		   (uint8_t*)&symm_req_descr->salt,
	    		   sizeof(symm_req_descr->salt))) {
           SDK_TRACE_ERR("%s@0x%llx: Failed to read the Salt information from HBM",
	    	     barco_ring->ring_name, (uint64_t) req_descr->iv_address);
        }
        if(sdk::asic::asic_mem_read(req_descr->iv_address + 4,
	    		   (uint8_t*)&symm_req_descr->explicit_iv,
	    		   sizeof(symm_req_descr->explicit_iv))) {
            SDK_TRACE_ERR("%s@0x%llx: Failed to read the explicit IV information from HBM",
	    	      barco_ring->ring_name, (uint64_t) (req_descr->iv_address + 4));
        }
    }
    if (req_descr->status_addr) {
        if(sdk::asic::asic_mem_read(req_descr->status_addr,
	    		   (uint8_t*)&symm_req_descr->barco_status,
	   		   sizeof(symm_req_descr->barco_status))) {
           SDK_TRACE_ERR("%s@0x%llx: Failed to read the Barco Status information from HBM",
	    	     barco_ring->ring_name, (uint64_t) req_descr->status_addr);
        }
    }
    return SDK_RET_OK;
}

sdk_ret_t capri_barco_ring_meta_get(barco_rings_t ring_type, uint32_t *pi, uint32_t *ci)
{
    cap_top_csr_t &             cap0 = CAP_BLK_REG_MODEL_ACCESS(cap_top_csr_t, 0, 0);
    cap_mpns_csr_t &            mpns = cap0.mp.mpns;
    cap_hens_csr_t &            hens = cap0.md.hens;

    /*
     * Read the PI/CI registers for the corresponding barco ring.
     */
    switch(ring_type) {
    case BARCO_RING_GCM0:
        hens.dhs_crypto_ctl.gcm0_producer_idx.read();
        *pi = hens.dhs_crypto_ctl.gcm0_producer_idx.fld().convert_to<uint32_t>();
        hens.dhs_crypto_ctl.gcm0_consumer_idx.read();
        *ci = hens.dhs_crypto_ctl.gcm0_consumer_idx.fld().convert_to<uint32_t>();
        break;
    case BARCO_RING_GCM1:
        hens.dhs_crypto_ctl.gcm1_producer_idx.read();
        *pi = hens.dhs_crypto_ctl.gcm1_producer_idx.fld().convert_to<uint32_t>();
        hens.dhs_crypto_ctl.gcm1_consumer_idx.read();
        *ci = hens.dhs_crypto_ctl.gcm1_consumer_idx.fld().convert_to<uint32_t>();
        break;
    case BARCO_RING_MPP0:
      mpns.dhs_crypto_ctl.mpp0_producer_idx.read();
      *pi = mpns.dhs_crypto_ctl.mpp0_producer_idx.fld().convert_to<uint32_t>();
      mpns.dhs_crypto_ctl.mpp0_consumer_idx.read();
      *ci = mpns.dhs_crypto_ctl.mpp0_consumer_idx.fld().convert_to<uint32_t>();
      break;
    case BARCO_RING_MPP1:
      mpns.dhs_crypto_ctl.mpp1_producer_idx.read();
      *pi = mpns.dhs_crypto_ctl.mpp1_producer_idx.fld().convert_to<uint32_t>();
      mpns.dhs_crypto_ctl.mpp1_consumer_idx.read();
      *ci = mpns.dhs_crypto_ctl.mpp1_consumer_idx.fld().convert_to<uint32_t>();
      break;
    case BARCO_RING_MPP2:
      mpns.dhs_crypto_ctl.mpp2_producer_idx.read();
      *pi = mpns.dhs_crypto_ctl.mpp2_producer_idx.fld().convert_to<uint32_t>();
      mpns.dhs_crypto_ctl.mpp2_consumer_idx.read();
      *ci = mpns.dhs_crypto_ctl.mpp2_consumer_idx.fld().convert_to<uint32_t>();
      break;
    case BARCO_RING_MPP3:
      mpns.dhs_crypto_ctl.mpp3_producer_idx.read();
      *pi = mpns.dhs_crypto_ctl.mpp3_producer_idx.fld().convert_to<uint32_t>();
      mpns.dhs_crypto_ctl.mpp3_consumer_idx.read();
      *ci = mpns.dhs_crypto_ctl.mpp3_consumer_idx.fld().convert_to<uint32_t>();
      break;
    default:
      SDK_TRACE_ERR("%s: Ring type not supported yet",
		    barco_rings[ring_type].ring_name);
      return SDK_RET_INVALID_ARG;
      break;
    }
    SDK_TRACE_DEBUG("%s: PI %d, CI %d",
		    barco_rings[ring_type].ring_name, *pi, *ci);
    return SDK_RET_OK;
}

}    // namespace capri
}    // namespace platform
}    // namespace sdk
