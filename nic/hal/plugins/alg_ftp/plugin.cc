//-----------------------------------------------------------------------------
// {C} Copyright 2017 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------

#include "core.hpp"
#include "utils.hpp"
#include "lib/slab/slab.hpp"
#include "nic/include/hal_mem.hpp"
#include "nic/hal/iris/include/hal_state.hpp" 

namespace hal {
namespace plugins {
namespace alg_ftp {

using namespace hal::plugins::alg_utils;

alg_state_t *g_ftp_state;
tcp_buffer_slab_t g_ftp_tcp_buffer_slabs; 

extern "C" hal_ret_t alg_ftp_init(hal_cfg_t *hal_cfg) {
    slab *appsess_slab_ = NULL;
    slab *l4sess_slab_ = NULL;
    slab *ftpinfo_slab_ = NULL;
    fte::feature_info_t info = {
        state_size:  0,
        state_init_fn: NULL,
        sess_del_cb: alg_ftp_session_delete_cb,
        sess_get_cb: alg_ftp_session_get_cb,
    };
    hal_slab_args_t slab_args;

    fte::register_feature(FTE_FEATURE_ALG_FTP, alg_ftp_exec, info);

    appsess_slab_ =  g_hal_state->register_slab(HAL_SLAB_FTP_ALG_APPSESS,
                        slab_args={.name="ftp_alg_appsess",
                       .size=sizeof(app_session_t), .num_elements=64,
                       .thread_safe=true, .grow_on_demand=true, .zero_on_alloc=true});
    SDK_ASSERT_RETURN((appsess_slab_ != NULL), HAL_RET_OOM);

    l4sess_slab_ =  g_hal_state->register_slab(HAL_SLAB_FTP_ALG_L4SESS,
                       slab_args={.name="ftp_alg_l4sess",
                       .size=sizeof(l4_alg_status_t), .num_elements=64,
                       .thread_safe=true, .grow_on_demand=true, .zero_on_alloc=true});
    SDK_ASSERT_RETURN((l4sess_slab_ != NULL), HAL_RET_OOM);

    ftpinfo_slab_  =  g_hal_state->register_slab(HAL_SLAB_FTP_ALG_FTPINFO,
                       slab_args={.name="ftp_alg_ftpinfo",
                       .size=sizeof(ftp_info_t), .num_elements=64,
                       .thread_safe=true, .grow_on_demand=true, .zero_on_alloc=true});
    SDK_ASSERT_RETURN((ftpinfo_slab_ != NULL), HAL_RET_OOM);

    g_ftp_state = alg_state_t::factory(FTE_FEATURE_ALG_FTP.c_str(),
                                       appsess_slab_, l4sess_slab_,
                                       ftpinfo_slab_, NULL,
                                       ftpinfo_cleanup_hdlr);

    g_ftp_tcp_buffer_slabs[0] = g_hal_state->register_slab(HAL_SLAB_FTP_TCP_BUFFER_2048,
                       slab_args={.name="ftp_tcp_buffer_2k",
                       .size=2048, .num_elements=8,
                       .thread_safe=true, .grow_on_demand=true, .zero_on_alloc=true});
    SDK_ASSERT_RETURN((g_ftp_tcp_buffer_slabs[0] != NULL), HAL_RET_OOM);

    g_ftp_tcp_buffer_slabs[1] = g_hal_state->register_slab(HAL_SLAB_FTP_TCP_BUFFER_4096,
                          slab_args={.name="ftp_tcp_buffer_4k",
                          .size=4096, .num_elements=8,
                          .thread_safe=true, .grow_on_demand=true, .zero_on_alloc=true});
    SDK_ASSERT_RETURN((g_ftp_tcp_buffer_slabs[1] != NULL), HAL_RET_OOM);

    g_ftp_tcp_buffer_slabs[2] = g_hal_state->register_slab(HAL_SLAB_FTP_TCP_BUFFER_8192,
                       slab_args={.name="ftp_tcp_buffer_8k",
                       .size=8192, .num_elements=8,
                       .thread_safe=true, .grow_on_demand=true, .zero_on_alloc=true});
    SDK_ASSERT_RETURN((g_ftp_tcp_buffer_slabs[2] != NULL), HAL_RET_OOM);

    g_ftp_tcp_buffer_slabs[3] = g_hal_state->register_slab(HAL_SLAB_FTP_TCP_BUFFER_16384,
                       slab_args={.name="ftp_tcp_buffer_16k",
                       .size=16384, .num_elements=8,
                       .thread_safe=true, .grow_on_demand=true, .zero_on_alloc=true});
    SDK_ASSERT_RETURN((g_ftp_tcp_buffer_slabs[3] != NULL), HAL_RET_OOM);

    return HAL_RET_OK;
}

extern "C" void alg_ftp_exit() {
    fte::unregister_feature(FTE_FEATURE_ALG_FTP);
}

}  // namespace alg_ftp
}  // namespace plugins
}  // namespace hal
