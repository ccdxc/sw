/**
 * Copyright (c) 2018 Pensando Systems, Inc.
 *
 * @file    capri_impl.cc
 *
 * @brief   CAPRI asic implementation
 */

#include "nic/sdk/include/sdk/mem.hpp"
#include "nic/sdk/lib/pal/pal.hpp"
#include "nic/apollo/api/impl/capri_impl.hpp"
// TODO: vijay please cleanup this along with asicpd move to sdk
#include "nic/hal/pd/asic_pd.hpp"

namespace impl {

/**
 * @defgroup OCI_ASIC_IMPL - asic wrapper implementation
 * @ingroup OCI_ASIC
 * @{
 */

/*
 * @brief    initialize an instance of capri impl class
 * @param[in] asic_cfg    asic information
 * @return    SDK_RET_OK on success, failure status code on error
 */
sdk_ret_t
capri_impl::init_(asic_cfg_t *asic_cfg) {
    asic_cfg_ = *asic_cfg;
    return SDK_RET_OK;
}

/**
 * @brief    factory method to asic impl instance
 * @param[in] asic_cfg    asic information
 * @return    new instance of capri asic impl or NULL, in case of error
 */
capri_impl *
capri_impl::factory(asic_cfg_t *asic_cfg) {
    capri_impl    *impl;

    impl = (capri_impl *)SDK_CALLOC(SDK_MEM_ALLOC_OCI_ASIC_IMPL,
                                    sizeof(capri_impl));
    new (impl) capri_impl();
    if (impl->init_(asic_cfg) != SDK_RET_OK) {
        impl->~capri_impl();
        SDK_FREE(SDK_MEM_ALLOC_OCI_ASIC_IMPL, impl);
        return NULL;
    }
    return impl;
}

/**
 * @brief    init routine to initialize the asic
 * @return    SDK_RET_OK on success, failure status code on error
 */
sdk_ret_t
capri_impl::asic_init(void) {
    sdk::lib::pal_ret_t    pal_ret;
    sdk_ret_t              ret;

    pal_ret = sdk::lib::pal_init(asic_cfg_.platform);
    SDK_ASSERT(pal_ret == sdk::lib::PAL_RET_OK);
    ret = sdk::asic::asic_init(&asic_cfg_);
    SDK_ASSERT(ret == SDK_RET_OK);
    return SDK_RET_OK;
}

/**
 * @brief    dump per TM port stats
 * @param[in] fp       file handle
 * @param[in] stats    pointer to the stats
 */
void
capri_impl::dump_tm_debug_stats_(FILE *fp, tm_pb_debug_stats_t *debug_stats)
{
    fprintf(fp, "    in  : %u\n", debug_stats->buffer_stats.sop_count_in);
    fprintf(fp, "    out : %u\n", debug_stats->buffer_stats.sop_count_out);
    fprintf(fp, "    intrinsic drop : %u\n",
            debug_stats->buffer_stats.drop_counts[sdk::platform::capri::BUFFER_INTRINSIC_DROP]);
    fprintf(fp, "    discarded : %u\n",
            debug_stats->buffer_stats.drop_counts[sdk::platform::capri::BUFFER_DISCARDED]);
    fprintf(fp, "    admitted : %u\n",
            debug_stats->buffer_stats.drop_counts[sdk::platform::capri::BUFFER_ADMITTED]);
    fprintf(fp, "    out of cells drop : %u\n",
            debug_stats->buffer_stats.drop_counts[sdk::platform::capri::BUFFER_OUT_OF_CELLS_DROP]);
    fprintf(fp, "    out of cells drop 2 : %u\n",
            debug_stats->buffer_stats.drop_counts[sdk::platform::capri::BUFFER_OUT_OF_CELLS_DROP_2]);
    fprintf(fp, "    out of credit drop : %u\n",
            debug_stats->buffer_stats.drop_counts[sdk::platform::capri::BUFFER_OUT_OF_CREDIT_DROP]);
    fprintf(fp, "    truncation drop : %u\n",
            debug_stats->buffer_stats.drop_counts[sdk::platform::capri::BUFFER_TRUNCATION_DROP]);
    fprintf(fp, "    port disabled drop : %u\n",
            debug_stats->buffer_stats.drop_counts[sdk::platform::capri::BUFFER_PORT_DISABLED_DROP]);
    fprintf(fp, "    copy to cpu tail drop : %u\n",
            debug_stats->buffer_stats.drop_counts[sdk::platform::capri::BUFFER_COPY_TO_CPU_TAIL_DROP]);
    fprintf(fp, "    span tail drop : %u\n",
            debug_stats->buffer_stats.drop_counts[sdk::platform::capri::BUFFER_SPAN_TAIL_DROP]);
    fprintf(fp, "    min size violation drop : %u\n",
            debug_stats->buffer_stats.drop_counts[sdk::platform::capri::BUFFER_MIN_SIZE_VIOLATION_DROP]);
    fprintf(fp, "    enqueue error drop : %u\n",
            debug_stats->buffer_stats.drop_counts[sdk::platform::capri::BUFFER_ENQUEUE_ERROR_DROP]);
    fprintf(fp, "    invalid port drop : %u\n",
            debug_stats->buffer_stats.drop_counts[sdk::platform::capri::BUFFER_INVALID_PORT_DROP]);
    fprintf(fp, "    invalid output queue drop : %u\n",
            debug_stats->buffer_stats.drop_counts[sdk::platform::capri::BUFFER_INVALID_OUTPUT_QUEUE_DROP]);
}

/**
 * @brief    dump all the debug information to given file
 * @param[in] fp    file handle
 */
void
capri_impl::debug_dump(FILE *fp) {
    sdk_ret_t              ret;
    bool                   reset = true;
    tm_pb_debug_stats_t    debug_stats;

    fprintf(fp, "PB/TM statistics\n");
    for (uint32_t port = 0; port < TM_NUM_PORTS; port++) {
        memset(&debug_stats, 0, sizeof(debug_stats));
        ret = capri_tm_get_pb_debug_stats(port, &debug_stats, reset);
        if ((port >= TM_UPLINK_PORT_BEGIN) && (port <= TM_UPLINK_PORT_END)) {
            fprintf(fp, "  TM Uplink Port %u\n",
                    sdk::platform::capri::capri_tm_port_to_fp_port(port));
            dump_tm_debug_stats_(fp, &debug_stats);
        } else if (port == TM_DMA_PORT_BEGIN) {
            fprintf(fp, "  DMA\n");
            dump_tm_debug_stats_(fp, &debug_stats);
        } else if (port == TM_PORT_INGRESS) {
            fprintf(fp, "  P4 IG\n");
            dump_tm_debug_stats_(fp, &debug_stats);
        } else if (port == TM_PORT_EGRESS) {
            fprintf(fp, "  P4 EG\n");
            dump_tm_debug_stats_(fp, &debug_stats);
        }
    }
    fprintf(fp, "\n");
}

/** @} */    // end of OCI_ASIC_IMPL

}    // namespace impl
