/*
 * plugin.cc
 */

#include "core.hpp"
#include "utils.hpp"
#include "sdk/slab.hpp"
#include "nic/hal/plugins/alg_utils/core.hpp"
#include "nic/include/hal_mem.hpp"

namespace hal {
namespace plugins {
namespace alg_ftp {

using namespace hal::plugins::alg_utils;

alg_state_t *g_ftp_state;

extern "C" hal_ret_t alg_ftp_init() {
    slab *appsess_slab_ = NULL;
    slab *l4sess_slab_ = NULL;
    slab *ftpinfo_slab_ = NULL;

    fte::register_feature(FTE_FEATURE_ALG_FTP, alg_ftp_exec);
   
    appsess_slab_ = slab::factory("ftp_alg_appsess", HAL_SLAB_FTP_ALG_APPSESS,
                                  sizeof(app_session_t), 64,
                                  true, true, true);
    HAL_ASSERT_RETURN((appsess_slab_ != NULL), HAL_RET_OOM);

    l4sess_slab_ = slab::factory("ftp_alg_l4sess", HAL_SLAB_FTP_ALG_L4SESS,
                                 sizeof(l4_alg_status_t), 64,
                                 true, true, true);
    HAL_ASSERT_RETURN((l4sess_slab_ != NULL), HAL_RET_OOM);

    ftpinfo_slab_  = slab::factory("ftp_alg_ftpinfo", HAL_SLAB_FTP_ALG_FTPINFO,
                                     sizeof(ftp_info_t), 64,
                                    true, true, true);
    HAL_ASSERT_RETURN((ftpinfo_slab_ != NULL), HAL_RET_OOM);
 
    g_ftp_state = alg_state_t::factory(FTE_FEATURE_ALG_FTP.c_str(), 
                                  appsess_slab_, l4sess_slab_, ftpinfo_slab_, NULL, NULL);

    return HAL_RET_OK;
}

extern "C" void alg_ftp_exit() {
    fte::unregister_feature(FTE_FEATURE_ALG_FTP);
}

}  // namespace alg_ftp
}  // namespace plugins
}  // namespace hal
