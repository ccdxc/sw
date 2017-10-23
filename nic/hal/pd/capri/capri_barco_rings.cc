#include "nic/hal/hal.hpp"
#include "nic/gen/proto/hal/types.pb.h"
#include "nic/hal/pd/capri/capri_hbm.hpp"
#include "nic/hal/pd/capri/capri_barco_rings.hpp"
#include "nic/hal/pd/iris/hal_state_pd.hpp"
#include "nic/asic/capri/model/cap_top/cap_top_csr.h"
#include "nic/hal/pd/capri/capri_barco_res.hpp"

namespace hal {

namespace pd {

hal_ret_t capri_barco_asym_init(capri_barco_ring_t *barco_ring);
bool capri_barco_asym_poller(capri_barco_ring_t *barco_ring);

capri_barco_ring_t  barco_rings[] = {
    {
        BARCO_RING_ASYM,
        CAPRI_HBM_REG_BARCO_RING_ASYM,
        0,
        32,
        1024,
        32,
        0,
        0,
        capri_barco_asym_init,
        capri_barco_asym_poller,
    },
    {
        "",
        "",
        0,
        0,
        0,
        0,
        0,
        0,
        NULL,
        NULL,
    },
};

hal_ret_t capri_barco_ring_common_init(capri_barco_ring_t *barco_ring)
{
    uint64_t                            ring_base = 0;
    uint16_t                            ring_size = 0;

    ring_base = get_start_offset(barco_ring->hbm_region);
    if (!ring_base) {
        HAL_TRACE_ERR("Failed to retrieve Barco Ring memory region for {}", barco_rings->ring_name);
        return HAL_RET_ERR;
    }

    if (ring_base & (barco_ring->ring_alignment - 1)) {
        HAL_TRACE_ERR("Failed to retrieve aligned Barco Ring memory region for {}", barco_rings->ring_name);
        return HAL_RET_ERR;
    }
    
    ring_size = get_size_kb(barco_ring->hbm_region) * 1024;
    if (ring_size < (barco_ring->ring_size * barco_ring->descriptor_size)) {
        HAL_TRACE_ERR("Not enough memory for Barco Ring memory region {}", barco_rings->ring_name);
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

    hens.dhs_crypto_ctl.pk_producer_idx.fld(barco_ring->producer_idx);
    hens.dhs_crypto_ctl.pk_producer_idx.write();

    hens.dhs_crypto_ctl.pk_consumer_idx.fld(barco_ring->consumer_idx);
    hens.dhs_crypto_ctl.pk_consumer_idx.write();

    HAL_TRACE_DEBUG("Barco ring \"{}\" base setup @ {:x}, descriptor count {}",
            barco_ring->ring_name, barco_ring->ring_base, barco_ring->ring_size);

    return capri_barco_asym_key_array_init();
}

bool capri_barco_asym_poller(capri_barco_ring_t *barco_ring)
{
    /* TBD */
    return FALSE;
}


hal_ret_t capri_barco_rings_init(void)
{
    uint16_t        idx;
    hal_ret_t       ret = HAL_RET_OK;

    for (idx = 0; idx < types::BarcoRings_MAX; idx++) {
        if (barco_rings[idx].init) {
            ret = barco_rings[idx].init(&barco_rings[idx]);
            if (ret != HAL_RET_OK) {
                HAL_TRACE_ERR("Failed to initialize Barco Ring {}", barco_rings[idx].ring_name);
                return ret;
            }
        }
        else
            break;
    }
    return ret;
}




} // namespace pd

} // namespace hal
