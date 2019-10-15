//-----------------------------------------------------------------------------
// {C} Copyright 2017 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------

#include "nic/include/fte.hpp"
#include "core.hpp"
#include "alg_dns/core.hpp"
#include "alg_ftp/core.hpp"
#include "alg_rpc/core.hpp"
#include "alg_rtsp/core.hpp"
#include "alg_sip/core.hpp"
#include "alg_tftp/core.hpp"

namespace hal {
namespace plugins {
namespace sfw {

using namespace hal::plugins::alg_dns;
using namespace hal::plugins::alg_ftp;
using namespace hal::plugins::alg_rpc;
using namespace hal::plugins::alg_rtsp;
using namespace hal::plugins::alg_sip;
using namespace hal::plugins::alg_tftp;

fte::pipeline_action_t sfw_exec(fte::ctx_t &ctx);
fte::pipeline_action_t conntrack_exec(fte::ctx_t &ctx);

hal_ret_t sfwcfg_init(hal_cfg_t *hal_cfg);
hal_ret_t sfwcfg_exit(void);

extern "C" hal_ret_t sfw_init(hal_cfg_t *hal_cfg) {
    hal_ret_t hal_ret = HAL_RET_OK;

    fte::feature_info_t info = {
        state_size: sizeof(sfw_info_t),
        state_init_fn: NULL,
        sess_del_cb: sfw_session_delete_cb,
        sess_get_cb: NULL,
        sess_upd_cb: NULL,
    };

    fte::register_feature(FTE_FEATURE_SFW, sfw_exec, info);
    fte::register_feature(FTE_FEATURE_CONNTRACK, conntrack_exec);

    hal_ret = sfwcfg_init(hal_cfg);
    SDK_ASSERT_RETURN((hal_ret == HAL_RET_OK), hal_ret);

    hal_ret = alg_dns_init(hal_cfg);
    SDK_ASSERT_RETURN((hal_ret == HAL_RET_OK), hal_ret);

    hal_ret = alg_ftp_init(hal_cfg);
    SDK_ASSERT_RETURN((hal_ret == HAL_RET_OK), hal_ret);

    hal_ret = alg_rpc_init(hal_cfg);
    SDK_ASSERT_RETURN((hal_ret == HAL_RET_OK), hal_ret);

    hal_ret = alg_rtsp_init(hal_cfg);
    SDK_ASSERT_RETURN((hal_ret == HAL_RET_OK), hal_ret);

    hal_ret = alg_sip_init(hal_cfg);
    SDK_ASSERT_RETURN((hal_ret == HAL_RET_OK), hal_ret);

    hal_ret = alg_tftp_init(hal_cfg);
    SDK_ASSERT_RETURN((hal_ret == HAL_RET_OK), hal_ret);

    return hal_ret;
}

extern "C" void sfw_exit() {
    fte::unregister_feature(FTE_FEATURE_SFW);
    fte::unregister_feature(FTE_FEATURE_CONNTRACK);
    sfwcfg_exit();

    alg_dns_exit();
    alg_ftp_exit();
    alg_rpc_exit();
    alg_rtsp_exit();
    alg_sip_exit();
    alg_tftp_exit();
}

}  // namespace sfw
}  // namespace plugins
}  // namespace hal
