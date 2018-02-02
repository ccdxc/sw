
#include "nic/include/base.h"
#include "nic/hal/hal.hpp"
#include "nic/include/hal_state.hpp"
#include "nic/hal/src/interface.hpp"
#include "nic/hal/src/lif_manager.hpp"
#include "nic/include/pd.hpp"
#include "nic/include/pd_api.hpp"
#include "nic/hal/src/eth.hpp"
#ifndef GFT
// #include "nic/hal/pd/iris/eth_pd.hpp"      // <== really ?? iris here ??
#endif
#include "nic/utils/host_mem/host_mem.hpp"
// #include "nic/hal/pd/capri/capri_hbm.hpp"
#ifndef GFT
// #include "nic/hal/pd/iris/if_pd_utils.hpp"
// #include "nic/hal/pd/iris/hal_state_pd.hpp"  // <== why would u need this here ??
#endif
// #include "nic/p4/include/common_defines.h"

namespace hal {

#define MAX_LIFS (2048)
#define ETH_RSS_MAX_QUEUES (128)
#define ETH_RSS_LIF_INDIR_TBL_LEN ETH_RSS_MAX_QUEUES

hal_ret_t
eth_rss_init (uint32_t hw_lif_id, lif_rss_info_t *rss, lif_queue_info_t *qinfo)
{
#ifndef GFT
    uint32_t num_queues;
#endif

    HAL_TRACE_DEBUG("{}: Entered\n", __FUNCTION__);

    //HAL_ASSERT(hw_lif_id < MAX_LIFS);
    HAL_ASSERT(rss != NULL);
    HAL_ASSERT(qinfo != NULL);

#ifndef GFT
    pd::pd_rss_params_table_entry_add_args_t args;
    args.hw_lif_id = hw_lif_id;
    args.rss_type = rss->type;
    args.rss_key = (uint8_t *)&rss->key;
    pd::hal_pd_call(pd::PD_FUNC_ID_RSS_PARAMS_TABLE_ADD, (void *)&args);
#if 0
    hal::pd::p4pd_common_p4plus_rxdma_rss_params_table_entry_add(
        hw_lif_id, rss->type, (uint8_t *)&rss->key);
#endif

    num_queues = qinfo[intf::LIF_QUEUE_PURPOSE_RX].num_queues;
    HAL_ASSERT(num_queues < ETH_RSS_MAX_QUEUES);

    if (num_queues > 0) {
        // TODO: please seriously stop hacking the code ... follow the layering,
        // it is pretty simple apis all over
        for (unsigned int index = 0; index < ETH_RSS_LIF_INDIR_TBL_LEN; index++) {
            pd::pd_rss_indir_table_entry_add_args_t args;
            args.hw_lif_id = hw_lif_id;
            args.index = index;
            args.enable = rss->enable;
            args.qid = index % num_queues;
            pd::hal_pd_call(pd::PD_FUNC_ID_RSS_INDIR_TABLE_ADD, (void *)&args);
#if 0
            hal::pd::p4pd_common_p4plus_rxdma_rss_indir_table_entry_add(
                hw_lif_id, index, rss->enable, index % num_queues);
#endif
        }
    }
#endif

    HAL_TRACE_DEBUG("{}: Leaving\n", __FUNCTION__);
    return HAL_RET_OK;
}

} // namespace hal
