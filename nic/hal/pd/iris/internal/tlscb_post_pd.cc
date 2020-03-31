#include "nic/include/base.hpp"
#include <arpa/inet.h>
#include "nic/sdk/include/sdk/lock.hpp"
#include "nic/include/pd_api.hpp"
#include "nic/hal/pd/iris/internal/tlscb_pd.hpp"
#include "nic/sdk/asic/cmn/asic_hbm.hpp"
#include "nic/hal/src/internal/proxy.hpp"
#include "nic/hal/hal.hpp"
#include "gen/p4gen/tls_txdma_post_crypto_enc/include/tls_txdma_post_crypto_enc_p4plus_ingress.h"
#include "nic/hal/pd/libs/wring/wring_pd.hpp"
#include "nic/hal/pd/iris/internal/p4plus_pd_api.h"

namespace hal {
namespace pd {

hal_ret_t
p4pd_get_tls_tx_s5_t0_post_crypto_stats_entry(pd_tlscb_t* tlscb_pd)
{
    tx_table_s7_t3_d                   data = {0};
    hal_ret_t                          ret = HAL_RET_OK;

    // hardware index for this entry
    tlscb_hw_id_t hwid = tlscb_pd->hw_id +
        (P4PD_TLSCB_STAGE_ENTRY_OFFSET * P4PD_HWID_TLS_TX_POST_CRYPTO_STATS_U16);

    if(sdk::asic::asic_mem_read(hwid,  (uint8_t *)&data, sizeof(data))){
        HAL_TRACE_ERR("Failed to read tx: s5_t0_post_crypto_stats_entry for TLS CB");
        return HAL_RET_HW_FAIL;
    }

    /*
     * For the stats that could be updated in the pre-crypto stage, we'll add the counters
     * (valid for multiple pass pipelines like AES-CBC-HMAC-SHA2 software chaining).
     */
    tlscb_pd->tlscb->tnmdpr_alloc += ntohs(data.u.tls_post_crypto_stats5_d.tnmdpr_alloc);
    tlscb_pd->tlscb->enc_requests += ntohs(data.u.tls_post_crypto_stats5_d.enc_requests);

    tlscb_pd->tlscb->rnmdpr_free = ntohs(data.u.tls_post_crypto_stats5_d.rnmdpr_free);
    tlscb_pd->tlscb->enc_completions = ntohs(data.u.tls_post_crypto_stats5_d.enc_completions);
    tlscb_pd->tlscb->dec_completions = ntohs(data.u.tls_post_crypto_stats5_d.dec_completions);
    tlscb_pd->tlscb->mac_completions = ntohs(data.u.tls_post_crypto_stats5_d.mac_completions);
    tlscb_pd->tlscb->post_debug_stage0_7_thread =
      (ntohs(data.u.tls_post_crypto_stats5_d.debug_stage4_7_thread) << 16) |
      ntohs(data.u.tls_post_crypto_stats5_d.debug_stage0_3_thread);
    HAL_TRACE_DEBUG("hwid : 0x{:x}", hwid);
    HAL_TRACE_DEBUG("Received tnmdpr alloc: 0x{:x}", tlscb_pd->tlscb->tnmdpr_alloc);
    HAL_TRACE_DEBUG("Received enc requests: 0x{:x}", tlscb_pd->tlscb->enc_requests);

    HAL_TRACE_DEBUG("Received rnmdpr free: 0x{:x}", tlscb_pd->tlscb->rnmdpr_free);
    HAL_TRACE_DEBUG("Received enc completions: 0x{:x}", tlscb_pd->tlscb->enc_completions);
    HAL_TRACE_DEBUG("Received dec completions: 0x{:x}", tlscb_pd->tlscb->dec_completions);
    HAL_TRACE_DEBUG("Received mac completions: 0x{:x}", tlscb_pd->tlscb->mac_completions);
    HAL_TRACE_DEBUG("Received post debug stage0_7 thread: 0x{:x}", tlscb_pd->tlscb->post_debug_stage0_7_thread);
    return ret;
}

}    // namespace pd
}    // namespace hal
