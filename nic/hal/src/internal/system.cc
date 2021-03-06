//-----------------------------------------------------------------------------
// {C} Copyright 2017 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------

#include "nic/include/base.hpp"
#include "nic/hal/hal.hpp"
#include "nic/sdk/include/sdk/lock.hpp"
#include "nic/sdk/platform/fru/fru.hpp"
#include "nic/hal/iris/include/hal_state.hpp"
#include "gen/hal/include/hal_api_stats.hpp"
#include "nic/hal/src/internal/system.hpp"
#include "nic/include/pd_api.hpp"
#include "nic/hal/plugins/cfg/nw/session.hpp"
#include "gen/proto/system.pb.h"
#include "nic/hal/iris/delphi/delphic.hpp"
#include "nic/hal/plugins/cfg/aclqos/acl_api.hpp"
#include "nic/hal/plugins/sfw/cfg/nwsec.hpp"
#include "nic/hal/plugins/cfg/nw/interface.hpp"
#include "nic/hal/plugins/cfg/nw/microseg.hpp"
#include "nic/hal/vmotion/vmotion.hpp"

namespace hal {

#define HAL_MICRO_SEG_ENF_FLAP_TIME (60 * 1000)

void
api_stats_fill_entry (ApiStatsEntry *entry, ApiCounter type)
{
    entry->set_api_type(type);
    switch (type) {
    case sys::API_COUNTER_VRF_CREATE:
        entry->set_num_api_call(g_hal_state->api_stats(HAL_API_VRF_CREATE));
        entry->set_num_api_success(g_hal_state->api_stats(HAL_API_VRF_CREATE_SUCCESS));
        entry->set_num_api_fail(g_hal_state->api_stats(HAL_API_VRF_CREATE_FAIL));
        break;
    case sys::API_COUNTER_VRF_UPDATE:
        entry->set_num_api_call(g_hal_state->api_stats(HAL_API_VRF_UPDATE));
        entry->set_num_api_success(g_hal_state->api_stats(HAL_API_VRF_UPDATE_SUCCESS));
        entry->set_num_api_fail(g_hal_state->api_stats(HAL_API_VRF_UPDATE_FAIL));
        break;
    case sys::API_COUNTER_VRF_DELETE:
        entry->set_num_api_call(g_hal_state->api_stats(HAL_API_VRF_DELETE));
        entry->set_num_api_success(g_hal_state->api_stats(HAL_API_VRF_DELETE_SUCCESS));
        entry->set_num_api_fail(g_hal_state->api_stats(HAL_API_VRF_DELETE_FAIL));
        break;
    case sys::API_COUNTER_VRF_GET:
        entry->set_num_api_call(g_hal_state->api_stats(HAL_API_VRF_GET));
        entry->set_num_api_success(g_hal_state->api_stats(HAL_API_VRF_GET_SUCCESS));
        entry->set_num_api_fail(g_hal_state->api_stats(HAL_API_VRF_GET_FAIL));
        break;
    case sys::API_COUNTER_ACL_CREATE:
        entry->set_num_api_call(g_hal_state->api_stats(HAL_API_ACL_CREATE));
        entry->set_num_api_success(g_hal_state->api_stats(HAL_API_ACL_CREATE_SUCCESS));
        entry->set_num_api_fail(g_hal_state->api_stats(HAL_API_ACL_CREATE_FAIL));
        break;
    case sys::API_COUNTER_ACL_UPDATE:
        entry->set_num_api_call(g_hal_state->api_stats(HAL_API_ACL_UPDATE));
        entry->set_num_api_success(g_hal_state->api_stats(HAL_API_ACL_UPDATE_SUCCESS));
        entry->set_num_api_fail(g_hal_state->api_stats(HAL_API_ACL_UPDATE_FAIL));
        break;
    case sys::API_COUNTER_ACL_DELETE:
        entry->set_num_api_call(g_hal_state->api_stats(HAL_API_ACL_DELETE));
        entry->set_num_api_success(g_hal_state->api_stats(HAL_API_ACL_DELETE_SUCCESS));
        entry->set_num_api_fail(g_hal_state->api_stats(HAL_API_ACL_DELETE_FAIL));
        break;
    case sys::API_COUNTER_ACL_GET:
        entry->set_num_api_call(g_hal_state->api_stats(HAL_API_ACL_GET));
        entry->set_num_api_success(g_hal_state->api_stats(HAL_API_ACL_GET_SUCCESS));
        entry->set_num_api_fail(g_hal_state->api_stats(HAL_API_ACL_GET_FAIL));
        break;
#ifdef __x86_64__
    case sys::API_COUNTER_CPUCB_CREATE:
        entry->set_num_api_call(g_hal_state->api_stats(HAL_API_CPUCB_CREATE));
        entry->set_num_api_success(g_hal_state->api_stats(HAL_API_CPUCB_CREATE_SUCCESS));
        entry->set_num_api_fail(g_hal_state->api_stats(HAL_API_CPUCB_CREATE_FAIL));
        break;
    case sys::API_COUNTER_CPUCB_UPDATE:
        entry->set_num_api_call(g_hal_state->api_stats(HAL_API_CPUCB_UPDATE));
        entry->set_num_api_success(g_hal_state->api_stats(HAL_API_CPUCB_UPDATE_SUCCESS));
        entry->set_num_api_fail(g_hal_state->api_stats(HAL_API_CPUCB_UPDATE_FAIL));
        break;
    case sys::API_COUNTER_CPUCB_DELETE:
        entry->set_num_api_call(g_hal_state->api_stats(HAL_API_CPUCB_DELETE));
        entry->set_num_api_success(g_hal_state->api_stats(HAL_API_CPUCB_DELETE_SUCCESS));
        entry->set_num_api_fail(g_hal_state->api_stats(HAL_API_CPUCB_DELETE_FAIL));
        break;
    case sys::API_COUNTER_CPUCB_GET:
        entry->set_num_api_call(g_hal_state->api_stats(HAL_API_CPUCB_GET));
        entry->set_num_api_success(g_hal_state->api_stats(HAL_API_CPUCB_GET_SUCCESS));
        entry->set_num_api_fail(g_hal_state->api_stats(HAL_API_CPUCB_GET_FAIL));
        break;
#endif
    case sys::API_COUNTER_CRYPTOAPI_INVOKE:
        entry->set_num_api_call(g_hal_state->api_stats(HAL_API_CRYPTOAPI_INVOKE));
        entry->set_num_api_success(g_hal_state->api_stats(HAL_API_CRYPTOAPI_INVOKE_SUCCESS));
        entry->set_num_api_fail(g_hal_state->api_stats(HAL_API_CRYPTOAPI_INVOKE_FAIL));
        break;
    case sys::API_COUNTER_CRYPTOKEY_CREATE:
        entry->set_num_api_call(g_hal_state->api_stats(HAL_API_CRYPTOKEY_CREATE));
        entry->set_num_api_success(g_hal_state->api_stats(HAL_API_CRYPTOKEY_CREATE_SUCCESS));
        entry->set_num_api_fail(g_hal_state->api_stats(HAL_API_CRYPTOKEY_CREATE_FAIL));
        break;
    case sys::API_COUNTER_CRYPTOKEY_UPDATE:
        entry->set_num_api_call(g_hal_state->api_stats(HAL_API_CRYPTOKEY_UPDATE));
        entry->set_num_api_success(g_hal_state->api_stats(HAL_API_CRYPTOKEY_UPDATE_SUCCESS));
        entry->set_num_api_fail(g_hal_state->api_stats(HAL_API_CRYPTOKEY_UPDATE_FAIL));
        break;
    case sys::API_COUNTER_CRYPTOKEY_DELETE:
        entry->set_num_api_call(g_hal_state->api_stats(HAL_API_CRYPTOKEY_DELETE));
        entry->set_num_api_success(g_hal_state->api_stats(HAL_API_CRYPTOKEY_DELETE_SUCCESS));
        entry->set_num_api_fail(g_hal_state->api_stats(HAL_API_CRYPTOKEY_DELETE_FAIL));
        break;
    case sys::API_COUNTER_CRYPTOKEY_READ:
        entry->set_num_api_call(g_hal_state->api_stats(HAL_API_CRYPTOKEY_READ));
        entry->set_num_api_success(g_hal_state->api_stats(HAL_API_CRYPTOKEY_READ_SUCCESS));
        entry->set_num_api_fail(g_hal_state->api_stats(HAL_API_CRYPTOKEY_READ_FAIL));
        break;
    case sys::API_COUNTER_DESCRAOL_GET:
        entry->set_num_api_call(g_hal_state->api_stats(HAL_API_DESCRAOL_GET));
        entry->set_num_api_success(g_hal_state->api_stats(HAL_API_DESCRAOL_GET_SUCCESS));
        entry->set_num_api_fail(g_hal_state->api_stats(HAL_API_DESCRAOL_GET_FAIL));
        break;
    case sys::API_COUNTER_ENDPOINT_CREATE:
        entry->set_num_api_call(g_hal_state->api_stats(HAL_API_ENDPOINT_CREATE));
        entry->set_num_api_success(g_hal_state->api_stats(HAL_API_ENDPOINT_CREATE_SUCCESS));
        entry->set_num_api_fail(g_hal_state->api_stats(HAL_API_ENDPOINT_CREATE_FAIL));
        break;
    case sys::API_COUNTER_ENDPOINT_UPDATE:
        entry->set_num_api_call(g_hal_state->api_stats(HAL_API_ENDPOINT_UPDATE));
        entry->set_num_api_success(g_hal_state->api_stats(HAL_API_ENDPOINT_UPDATE_SUCCESS));
        entry->set_num_api_fail(g_hal_state->api_stats(HAL_API_ENDPOINT_UPDATE_FAIL));
        break;
    case sys::API_COUNTER_ENDPOINT_DELETE:
        entry->set_num_api_call(g_hal_state->api_stats(HAL_API_ENDPOINT_DELETE));
        entry->set_num_api_success(g_hal_state->api_stats(HAL_API_ENDPOINT_DELETE_SUCCESS));
        entry->set_num_api_fail(g_hal_state->api_stats(HAL_API_ENDPOINT_DELETE_FAIL));
        break;
    case sys::API_COUNTER_ENDPOINT_GET:
        entry->set_num_api_call(g_hal_state->api_stats(HAL_API_ENDPOINT_GET));
        entry->set_num_api_success(g_hal_state->api_stats(HAL_API_ENDPOINT_GET_SUCCESS));
        entry->set_num_api_fail(g_hal_state->api_stats(HAL_API_ENDPOINT_GET_FAIL));
        break;
#ifdef GFT
    case sys::API_COUNTER_GFT_EXACT_MATCH_PROFILE_CREATE:
        entry->set_num_api_call(g_hal_state->api_stats(HAL_API_GFT_EXACT_MATCH_PROFILE_CREATE));
        entry->set_num_api_success(g_hal_state->api_stats(HAL_API_GFT_EXACT_MATCH_PROFILE_CREATE_SUCCESS));
        entry->set_num_api_fail(g_hal_state->api_stats(HAL_API_GFT_EXACT_MATCH_PROFILE_CREATE_FAIL));
        break;
    case sys::API_COUNTER_GFT_HEADER_TRANSPOSITION_PROFILE_CREATE:
        entry->set_num_api_call(g_hal_state->api_stats(HAL_API_GFT_HEADER_TRANSPOSITION_PROFILE_CREATE));
        entry->set_num_api_success(g_hal_state->api_stats(HAL_API_GFT_HEADER_TRANSPOSITION_PROFILE_CREATE_SUCCESS));
        entry->set_num_api_fail(g_hal_state->api_stats(HAL_API_GFT_HEADER_TRANSPOSITION_PROFILE_CREATE_FAIL));
        break;
    case sys::API_COUNTER_GFT_EXACT_MATCH_FLOW_ENTRY_CREATE:
        entry->set_num_api_call(g_hal_state->api_stats(HAL_API_GFT_EXACT_MATCH_FLOW_ENTRY_CREATE));
        entry->set_num_api_success(g_hal_state->api_stats(HAL_API_GFT_EXACT_MATCH_FLOW_ENTRY_CREATE_SUCCESS));
        entry->set_num_api_fail(g_hal_state->api_stats(HAL_API_GFT_EXACT_MATCH_FLOW_ENTRY_CREATE_FAIL));
        break;
#endif
    case sys::API_COUNTER_GETPROGRAM_ADDRESS:
        entry->set_num_api_call(g_hal_state->api_stats(HAL_API_GETPROGRAM_ADDRESS));
        entry->set_num_api_success(g_hal_state->api_stats(HAL_API_GETPROGRAM_ADDRESS_SUCCESS));
        entry->set_num_api_fail(g_hal_state->api_stats(HAL_API_GETPROGRAM_ADDRESS_FAIL));
        break;
    case sys::API_COUNTER_ALLOCHBM_ADDRESS:
        entry->set_num_api_call(g_hal_state->api_stats(HAL_API_ALLOCHBM_ADDRESS));
        entry->set_num_api_success(g_hal_state->api_stats(HAL_API_ALLOCHBM_ADDRESS_SUCCESS));
        entry->set_num_api_fail(g_hal_state->api_stats(HAL_API_ALLOCHBM_ADDRESS_FAIL));
        break;
    case sys::API_COUNTER_CONFIGURELIF_BDF:
        entry->set_num_api_call(g_hal_state->api_stats(HAL_API_CONFIGURELIF_BDF));
        entry->set_num_api_success(g_hal_state->api_stats(HAL_API_CONFIGURELIF_BDF_SUCCESS));
        entry->set_num_api_fail(g_hal_state->api_stats(HAL_API_CONFIGURELIF_BDF_FAIL));
        break;
#ifdef __x86_64__
    case sys::API_COUNTER_IPSECCB_CREATE:
        entry->set_num_api_call(g_hal_state->api_stats(HAL_API_IPSECCB_CREATE));
        entry->set_num_api_success(g_hal_state->api_stats(HAL_API_IPSECCB_CREATE_SUCCESS));
        entry->set_num_api_fail(g_hal_state->api_stats(HAL_API_IPSECCB_CREATE_FAIL));
        break;
    case sys::API_COUNTER_IPSECCB_UPDATE:
        entry->set_num_api_call(g_hal_state->api_stats(HAL_API_IPSECCB_UPDATE));
        entry->set_num_api_success(g_hal_state->api_stats(HAL_API_IPSECCB_UPDATE_SUCCESS));
        entry->set_num_api_fail(g_hal_state->api_stats(HAL_API_IPSECCB_UPDATE_FAIL));
        break;
    case sys::API_COUNTER_IPSECCB_DELETE:
        entry->set_num_api_call(g_hal_state->api_stats(HAL_API_IPSECCB_DELETE));
        entry->set_num_api_success(g_hal_state->api_stats(HAL_API_IPSECCB_DELETE_SUCCESS));
        entry->set_num_api_fail(g_hal_state->api_stats(HAL_API_IPSECCB_DELETE_FAIL));
        break;
    case sys::API_COUNTER_IPSECCB_GET:
        entry->set_num_api_call(g_hal_state->api_stats(HAL_API_IPSECCB_GET));
        entry->set_num_api_success(g_hal_state->api_stats(HAL_API_IPSECCB_GET_SUCCESS));
        entry->set_num_api_fail(g_hal_state->api_stats(HAL_API_IPSECCB_GET_FAIL));
        break;
#endif
    case sys::API_COUNTER_L2SEGMENT_CREATE:
        entry->set_num_api_call(g_hal_state->api_stats(HAL_API_L2SEGMENT_CREATE));
        entry->set_num_api_success(g_hal_state->api_stats(HAL_API_L2SEGMENT_CREATE_SUCCESS));
        entry->set_num_api_fail(g_hal_state->api_stats(HAL_API_L2SEGMENT_CREATE_FAIL));
        break;
    case sys::API_COUNTER_L2SEGMENT_UPDATE:
        entry->set_num_api_call(g_hal_state->api_stats(HAL_API_L2SEGMENT_UPDATE));
        entry->set_num_api_success(g_hal_state->api_stats(HAL_API_L2SEGMENT_UPDATE_SUCCESS));
        entry->set_num_api_fail(g_hal_state->api_stats(HAL_API_L2SEGMENT_UPDATE_FAIL));
        break;
    case sys::API_COUNTER_L2SEGMENT_DELETE:
        entry->set_num_api_call(g_hal_state->api_stats(HAL_API_L2SEGMENT_DELETE));
        entry->set_num_api_success(g_hal_state->api_stats(HAL_API_L2SEGMENT_DELETE_SUCCESS));
        entry->set_num_api_fail(g_hal_state->api_stats(HAL_API_L2SEGMENT_DELETE_FAIL));
        break;
    case sys::API_COUNTER_L2SEGMENT_GET:
        entry->set_num_api_call(g_hal_state->api_stats(HAL_API_L2SEGMENT_GET));
        entry->set_num_api_success(g_hal_state->api_stats(HAL_API_L2SEGMENT_GET_SUCCESS));
        entry->set_num_api_fail(g_hal_state->api_stats(HAL_API_L2SEGMENT_GET_FAIL));
        break;
#ifdef __x86_64__
    case sys::API_COUNTER_L4LBSERVICE_CREATE:
        entry->set_num_api_call(g_hal_state->api_stats(HAL_API_L4LBSERVICE_CREATE));
        entry->set_num_api_success(g_hal_state->api_stats(HAL_API_L4LBSERVICE_CREATE_SUCCESS));
        entry->set_num_api_fail(g_hal_state->api_stats(HAL_API_L4LBSERVICE_CREATE_FAIL));
        break;
#endif
    case sys::API_COUNTER_MULTICASTENTRY_CREATE:
        entry->set_num_api_call(g_hal_state->api_stats(HAL_API_MULTICASTENTRY_CREATE));
        entry->set_num_api_success(g_hal_state->api_stats(HAL_API_MULTICASTENTRY_CREATE_SUCCESS));
        entry->set_num_api_fail(g_hal_state->api_stats(HAL_API_MULTICASTENTRY_CREATE_FAIL));
        break;
    case sys::API_COUNTER_MULTICASTENTRY_UPDATE:
        entry->set_num_api_call(g_hal_state->api_stats(HAL_API_MULTICASTENTRY_UPDATE));
        entry->set_num_api_success(g_hal_state->api_stats(HAL_API_MULTICASTENTRY_UPDATE_SUCCESS));
        entry->set_num_api_fail(g_hal_state->api_stats(HAL_API_MULTICASTENTRY_UPDATE_FAIL));
        break;
    case sys::API_COUNTER_MULTICASTENTRY_DELETE:
        entry->set_num_api_call(g_hal_state->api_stats(HAL_API_MULTICASTENTRY_DELETE));
        entry->set_num_api_success(g_hal_state->api_stats(HAL_API_MULTICASTENTRY_DELETE_SUCCESS));
        entry->set_num_api_fail(g_hal_state->api_stats(HAL_API_MULTICASTENTRY_DELETE_FAIL));
        break;
    case sys::API_COUNTER_MULTICASTENTRY_GET:
        entry->set_num_api_call(g_hal_state->api_stats(HAL_API_MULTICASTENTRY_GET));
        entry->set_num_api_success(g_hal_state->api_stats(HAL_API_MULTICASTENTRY_GET_SUCCESS));
        entry->set_num_api_fail(g_hal_state->api_stats(HAL_API_MULTICASTENTRY_GET_FAIL));
        break;
    case sys::API_COUNTER_SECURITYPROFILE_CREATE:
        entry->set_num_api_call(g_hal_state->api_stats(HAL_API_SECURITYPROFILE_CREATE));
        entry->set_num_api_success(g_hal_state->api_stats(HAL_API_SECURITYPROFILE_CREATE_SUCCESS));
        entry->set_num_api_fail(g_hal_state->api_stats(HAL_API_SECURITYPROFILE_CREATE_FAIL));
        break;
    case sys::API_COUNTER_SECURITYPROFILE_UPDATE:
        entry->set_num_api_call(g_hal_state->api_stats(HAL_API_SECURITYPROFILE_UPDATE));
        entry->set_num_api_success(g_hal_state->api_stats(HAL_API_SECURITYPROFILE_UPDATE_SUCCESS));
        entry->set_num_api_fail(g_hal_state->api_stats(HAL_API_SECURITYPROFILE_UPDATE_FAIL));
        break;
    case sys::API_COUNTER_SECURITYPROFILE_DELETE:
        entry->set_num_api_call(g_hal_state->api_stats(HAL_API_SECURITYPROFILE_DELETE));
        entry->set_num_api_success(g_hal_state->api_stats(HAL_API_SECURITYPROFILE_DELETE_SUCCESS));
        entry->set_num_api_fail(g_hal_state->api_stats(HAL_API_SECURITYPROFILE_DELETE_FAIL));
        break;
    case sys::API_COUNTER_SECURITYPROFILE_GET:
        entry->set_num_api_call(g_hal_state->api_stats(HAL_API_SECURITYPROFILE_GET));
        entry->set_num_api_success(g_hal_state->api_stats(HAL_API_SECURITYPROFILE_GET_SUCCESS));
        entry->set_num_api_fail(g_hal_state->api_stats(HAL_API_SECURITYPROFILE_GET_FAIL));
        break;
    case sys::API_COUNTER_SECURITYGROUPPOLICY_CREATE:
        entry->set_num_api_call(g_hal_state->api_stats(HAL_API_SECURITYGROUPPOLICY_CREATE));
        entry->set_num_api_success(g_hal_state->api_stats(HAL_API_SECURITYGROUPPOLICY_CREATE_SUCCESS));
        entry->set_num_api_fail(g_hal_state->api_stats(HAL_API_SECURITYGROUPPOLICY_CREATE_FAIL));
        break;
    case sys::API_COUNTER_SECURITYGROUPPOLICY_UPDATE:
        entry->set_num_api_call(g_hal_state->api_stats(HAL_API_SECURITYGROUPPOLICY_UPDATE));
        entry->set_num_api_success(g_hal_state->api_stats(HAL_API_SECURITYGROUPPOLICY_UPDATE_SUCCESS));
        entry->set_num_api_fail(g_hal_state->api_stats(HAL_API_SECURITYGROUPPOLICY_UPDATE_FAIL));
        break;
    case sys::API_COUNTER_SECURITYGROUPPOLICY_DELETE:
        entry->set_num_api_call(g_hal_state->api_stats(HAL_API_SECURITYGROUPPOLICY_DELETE));
        entry->set_num_api_success(g_hal_state->api_stats(HAL_API_SECURITYGROUPPOLICY_DELETE_SUCCESS));
        entry->set_num_api_fail(g_hal_state->api_stats(HAL_API_SECURITYGROUPPOLICY_DELETE_FAIL));
        break;
    case sys::API_COUNTER_SECURITYGROUPPOLICY_GET:
        entry->set_num_api_call(g_hal_state->api_stats(HAL_API_SECURITYGROUPPOLICY_GET));
        entry->set_num_api_success(g_hal_state->api_stats(HAL_API_SECURITYGROUPPOLICY_GET_SUCCESS));
        entry->set_num_api_fail(g_hal_state->api_stats(HAL_API_SECURITYGROUPPOLICY_GET_FAIL));
        break;
    case sys::API_COUNTER_SECURITYGROUP_CREATE:
        entry->set_num_api_call(g_hal_state->api_stats(HAL_API_SECURITYGROUP_CREATE));
        entry->set_num_api_success(g_hal_state->api_stats(HAL_API_SECURITYGROUP_CREATE_SUCCESS));
        entry->set_num_api_fail(g_hal_state->api_stats(HAL_API_SECURITYGROUP_CREATE_FAIL));
        break;
    case sys::API_COUNTER_SECURITYGROUP_UPDATE:
        entry->set_num_api_call(g_hal_state->api_stats(HAL_API_SECURITYGROUP_UPDATE));
        entry->set_num_api_success(g_hal_state->api_stats(HAL_API_SECURITYGROUP_UPDATE_SUCCESS));
        entry->set_num_api_fail(g_hal_state->api_stats(HAL_API_SECURITYGROUP_UPDATE_FAIL));
        break;
    case sys::API_COUNTER_SECURITYGROUP_DELETE:
        entry->set_num_api_call(g_hal_state->api_stats(HAL_API_SECURITYGROUP_DELETE));
        entry->set_num_api_success(g_hal_state->api_stats(HAL_API_SECURITYGROUP_DELETE_SUCCESS));
        entry->set_num_api_fail(g_hal_state->api_stats(HAL_API_SECURITYGROUP_DELETE_FAIL));
        break;
    case sys::API_COUNTER_SECURITYGROUP_GET:
        entry->set_num_api_call(g_hal_state->api_stats(HAL_API_SECURITYGROUP_GET));
        entry->set_num_api_success(g_hal_state->api_stats(HAL_API_SECURITYGROUP_GET_SUCCESS));
        entry->set_num_api_fail(g_hal_state->api_stats(HAL_API_SECURITYGROUP_GET_FAIL));
        break;
#if 0
    case sys::API_COUNTER_DOSPOLICY_CREATE:
        entry->set_num_api_call(g_hal_state->api_stats(HAL_API_DOSPOLICY_CREATE));
        entry->set_num_api_success(g_hal_state->api_stats(HAL_API_DOSPOLICY_CREATE_SUCCESS));
        entry->set_num_api_fail(g_hal_state->api_stats(HAL_API_DOSPOLICY_CREATE_FAIL));
        break;
    case sys::API_COUNTER_DOSPOLICY_UPDATE:
        entry->set_num_api_call(g_hal_state->api_stats(HAL_API_DOSPOLICY_UPDATE));
        entry->set_num_api_success(g_hal_state->api_stats(HAL_API_DOSPOLICY_UPDATE_SUCCESS));
        entry->set_num_api_fail(g_hal_state->api_stats(HAL_API_DOSPOLICY_UPDATE_FAIL));
        break;
    case sys::API_COUNTER_DOSPOLICY_DELETE:
        entry->set_num_api_call(g_hal_state->api_stats(HAL_API_DOSPOLICY_DELETE));
        entry->set_num_api_success(g_hal_state->api_stats(HAL_API_DOSPOLICY_DELETE_SUCCESS));
        entry->set_num_api_fail(g_hal_state->api_stats(HAL_API_DOSPOLICY_DELETE_FAIL));
        break;
    case sys::API_COUNTER_DOSPOLICY_GET:
        entry->set_num_api_call(g_hal_state->api_stats(HAL_API_DOSPOLICY_GET));
        entry->set_num_api_success(g_hal_state->api_stats(HAL_API_DOSPOLICY_GET_SUCCESS));
        entry->set_num_api_fail(g_hal_state->api_stats(HAL_API_DOSPOLICY_GET_FAIL));
        break;
#endif
    case sys::API_COUNTER_PROXYCCB_CREATE:
        entry->set_num_api_call(g_hal_state->api_stats(HAL_API_PROXYCCB_CREATE));
        entry->set_num_api_success(g_hal_state->api_stats(HAL_API_PROXYCCB_CREATE_SUCCESS));
        entry->set_num_api_fail(g_hal_state->api_stats(HAL_API_PROXYCCB_CREATE_FAIL));
        break;
    case sys::API_COUNTER_PROXYCCB_UPDATE:
        entry->set_num_api_call(g_hal_state->api_stats(HAL_API_PROXYCCB_UPDATE));
        entry->set_num_api_success(g_hal_state->api_stats(HAL_API_PROXYCCB_UPDATE_SUCCESS));
        entry->set_num_api_fail(g_hal_state->api_stats(HAL_API_PROXYCCB_UPDATE_FAIL));
        break;
    case sys::API_COUNTER_PROXYCCB_DELETE:
        entry->set_num_api_call(g_hal_state->api_stats(HAL_API_PROXYCCB_DELETE));
        entry->set_num_api_success(g_hal_state->api_stats(HAL_API_PROXYCCB_DELETE_SUCCESS));
        entry->set_num_api_fail(g_hal_state->api_stats(HAL_API_PROXYCCB_DELETE_FAIL));
        break;
    case sys::API_COUNTER_PROXYCCB_GET:
        entry->set_num_api_call(g_hal_state->api_stats(HAL_API_PROXYCCB_GET));
        entry->set_num_api_success(g_hal_state->api_stats(HAL_API_PROXYCCB_GET_SUCCESS));
        entry->set_num_api_fail(g_hal_state->api_stats(HAL_API_PROXYCCB_GET_FAIL));
        break;
    case sys::API_COUNTER_PROXYRCB_CREATE:
        entry->set_num_api_call(g_hal_state->api_stats(HAL_API_PROXYRCB_CREATE));
        entry->set_num_api_success(g_hal_state->api_stats(HAL_API_PROXYRCB_CREATE_SUCCESS));
        entry->set_num_api_fail(g_hal_state->api_stats(HAL_API_PROXYRCB_CREATE_FAIL));
        break;
    case sys::API_COUNTER_PROXYRCB_UPDATE:
        entry->set_num_api_call(g_hal_state->api_stats(HAL_API_PROXYRCB_UPDATE));
        entry->set_num_api_success(g_hal_state->api_stats(HAL_API_PROXYRCB_UPDATE_SUCCESS));
        entry->set_num_api_fail(g_hal_state->api_stats(HAL_API_PROXYRCB_UPDATE_FAIL));
        break;
    case sys::API_COUNTER_PROXYRCB_DELETE:
        entry->set_num_api_call(g_hal_state->api_stats(HAL_API_PROXYRCB_DELETE));
        entry->set_num_api_success(g_hal_state->api_stats(HAL_API_PROXYRCB_DELETE_SUCCESS));
        entry->set_num_api_fail(g_hal_state->api_stats(HAL_API_PROXYRCB_DELETE_FAIL));
        break;
    case sys::API_COUNTER_PROXYRCB_GET:
        entry->set_num_api_call(g_hal_state->api_stats(HAL_API_PROXYRCB_GET));
        entry->set_num_api_success(g_hal_state->api_stats(HAL_API_PROXYRCB_GET_SUCCESS));
        entry->set_num_api_fail(g_hal_state->api_stats(HAL_API_PROXYRCB_GET_FAIL));
        break;
    case sys::API_COUNTER_QOSCLASS_CREATE:
        entry->set_num_api_call(g_hal_state->api_stats(HAL_API_QOSCLASS_CREATE));
        entry->set_num_api_success(g_hal_state->api_stats(HAL_API_QOSCLASS_CREATE_SUCCESS));
        entry->set_num_api_fail(g_hal_state->api_stats(HAL_API_QOSCLASS_CREATE_FAIL));
        break;
    case sys::API_COUNTER_QOSCLASS_UPDATE:
        entry->set_num_api_call(g_hal_state->api_stats(HAL_API_QOSCLASS_UPDATE));
        entry->set_num_api_success(g_hal_state->api_stats(HAL_API_QOSCLASS_UPDATE_SUCCESS));
        entry->set_num_api_fail(g_hal_state->api_stats(HAL_API_QOSCLASS_UPDATE_FAIL));
        break;
    case sys::API_COUNTER_QOSCLASS_DELETE:
        entry->set_num_api_call(g_hal_state->api_stats(HAL_API_QOSCLASS_DELETE));
        entry->set_num_api_success(g_hal_state->api_stats(HAL_API_QOSCLASS_DELETE_SUCCESS));
        entry->set_num_api_fail(g_hal_state->api_stats(HAL_API_QOSCLASS_DELETE_FAIL));
        break;
    case sys::API_COUNTER_QOSCLASS_GET:
        entry->set_num_api_call(g_hal_state->api_stats(HAL_API_QOSCLASS_GET));
        entry->set_num_api_success(g_hal_state->api_stats(HAL_API_QOSCLASS_GET_SUCCESS));
        entry->set_num_api_fail(g_hal_state->api_stats(HAL_API_QOSCLASS_GET_FAIL));
        break;
    case sys::API_COUNTER_COPP_UPDATE:
        entry->set_num_api_call(g_hal_state->api_stats(HAL_API_COPP_UPDATE));
        entry->set_num_api_success(g_hal_state->api_stats(HAL_API_COPP_UPDATE_SUCCESS));
        entry->set_num_api_fail(g_hal_state->api_stats(HAL_API_COPP_UPDATE_FAIL));
        break;
    case sys::API_COUNTER_COPP_GET:
        entry->set_num_api_call(g_hal_state->api_stats(HAL_API_COPP_GET));
        entry->set_num_api_success(g_hal_state->api_stats(HAL_API_COPP_GET_SUCCESS));
        entry->set_num_api_fail(g_hal_state->api_stats(HAL_API_COPP_GET_FAIL));
        break;
    case sys::API_COUNTER_RAWCCB_CREATE:
        entry->set_num_api_call(g_hal_state->api_stats(HAL_API_RAWCCB_CREATE));
        entry->set_num_api_success(g_hal_state->api_stats(HAL_API_RAWCCB_CREATE_SUCCESS));
        entry->set_num_api_fail(g_hal_state->api_stats(HAL_API_RAWCCB_CREATE_FAIL));
        break;
    case sys::API_COUNTER_RAWCCB_UPDATE:
        entry->set_num_api_call(g_hal_state->api_stats(HAL_API_RAWCCB_UPDATE));
        entry->set_num_api_success(g_hal_state->api_stats(HAL_API_RAWCCB_UPDATE_SUCCESS));
        entry->set_num_api_fail(g_hal_state->api_stats(HAL_API_RAWCCB_UPDATE_FAIL));
        break;
    case sys::API_COUNTER_RAWCCB_DELETE:
        entry->set_num_api_call(g_hal_state->api_stats(HAL_API_RAWCCB_DELETE));
        entry->set_num_api_success(g_hal_state->api_stats(HAL_API_RAWCCB_DELETE_SUCCESS));
        entry->set_num_api_fail(g_hal_state->api_stats(HAL_API_RAWCCB_DELETE_FAIL));
        break;
    case sys::API_COUNTER_RAWCCB_GET:
        entry->set_num_api_call(g_hal_state->api_stats(HAL_API_RAWCCB_GET));
        entry->set_num_api_success(g_hal_state->api_stats(HAL_API_RAWCCB_GET_SUCCESS));
        entry->set_num_api_fail(g_hal_state->api_stats(HAL_API_RAWCCB_GET_FAIL));
        break;
    case sys::API_COUNTER_TCPCB_CREATE:
        entry->set_num_api_call(g_hal_state->api_stats(HAL_API_TCPCB_CREATE));
        entry->set_num_api_success(g_hal_state->api_stats(HAL_API_TCPCB_CREATE_SUCCESS));
        entry->set_num_api_fail(g_hal_state->api_stats(HAL_API_TCPCB_CREATE_FAIL));
        break;
    case sys::API_COUNTER_TCPCB_UPDATE:
        entry->set_num_api_call(g_hal_state->api_stats(HAL_API_TCPCB_UPDATE));
        entry->set_num_api_success(g_hal_state->api_stats(HAL_API_TCPCB_UPDATE_SUCCESS));
        entry->set_num_api_fail(g_hal_state->api_stats(HAL_API_TCPCB_UPDATE_FAIL));
        break;
    case sys::API_COUNTER_TCPCB_DELETE:
        entry->set_num_api_call(g_hal_state->api_stats(HAL_API_TCPCB_DELETE));
        entry->set_num_api_success(g_hal_state->api_stats(HAL_API_TCPCB_DELETE_SUCCESS));
        entry->set_num_api_fail(g_hal_state->api_stats(HAL_API_TCPCB_DELETE_FAIL));
        break;
    case sys::API_COUNTER_TCPCB_GET:
        entry->set_num_api_call(g_hal_state->api_stats(HAL_API_TCPCB_GET));
        entry->set_num_api_success(g_hal_state->api_stats(HAL_API_TCPCB_GET_SUCCESS));
        entry->set_num_api_fail(g_hal_state->api_stats(HAL_API_TCPCB_GET_FAIL));
        break;
    case sys::API_COUNTER_TLSCB_CREATE:
        entry->set_num_api_call(g_hal_state->api_stats(HAL_API_TLSCB_CREATE));
        entry->set_num_api_success(g_hal_state->api_stats(HAL_API_TLSCB_CREATE_SUCCESS));
        entry->set_num_api_fail(g_hal_state->api_stats(HAL_API_TLSCB_CREATE_FAIL));
        break;
    case sys::API_COUNTER_TLSCB_UPDATE:
        entry->set_num_api_call(g_hal_state->api_stats(HAL_API_TLSCB_UPDATE));
        entry->set_num_api_success(g_hal_state->api_stats(HAL_API_TLSCB_UPDATE_SUCCESS));
        entry->set_num_api_fail(g_hal_state->api_stats(HAL_API_TLSCB_UPDATE_FAIL));
        break;
    case sys::API_COUNTER_TLSCB_DELETE:
        entry->set_num_api_call(g_hal_state->api_stats(HAL_API_TLSCB_DELETE));
        entry->set_num_api_success(g_hal_state->api_stats(HAL_API_TLSCB_DELETE_SUCCESS));
        entry->set_num_api_fail(g_hal_state->api_stats(HAL_API_TLSCB_DELETE_FAIL));
        break;
    case sys::API_COUNTER_TLSCB_GET:
        entry->set_num_api_call(g_hal_state->api_stats(HAL_API_TLSCB_GET));
        entry->set_num_api_success(g_hal_state->api_stats(HAL_API_TLSCB_GET_SUCCESS));
        entry->set_num_api_fail(g_hal_state->api_stats(HAL_API_TLSCB_GET_FAIL));
        break;
    default:
        entry->set_num_api_call(0);
        entry->set_num_api_success(0);
        entry->set_num_api_fail(0);
        break;
    }
}

//------------------------------------------------------------------------------
// process a API stats get request
//------------------------------------------------------------------------------
hal_ret_t
api_stats_get (ApiStatsResponse *rsp)
{
    ApiStatsEntry    *stats_entry;

    for (int i = sys::API_COUNTER_VRF_CREATE; i < sys::API_COUNTER_MAX; i ++) {
        stats_entry = rsp->add_api_entries();
        api_stats_fill_entry(stats_entry, (sys::ApiCounter) i);
    }

    return HAL_RET_OK;
}
//------------------------------------------------------------------------------
// process a system uuid get request
//------------------------------------------------------------------------------
hal_ret_t
system_uuid_get (SystemResponse *rsp)
{
    hal_ret_t   ret = HAL_RET_OK;
    char *uuid = NULL;
    std::string mac_str;

    rsp->set_api_status(types::API_STATUS_OK);
    if (hal::g_hal_cfg.platform == platform_type_t::PLATFORM_TYPE_HW) {
        sdk::platform::readfrukey(BOARD_MACADDRESS_KEY, mac_str);
        rsp->set_uuid(mac_str);
    } else {
        uuid = getenv("SYSUUID");
        if (uuid) {
            rsp->set_uuid(uuid);
        } else {
            rsp->set_uuid(PENSANDO_NIC_MAC_STR);
        }
    }
    return ret;
}

//------------------------------------------------------------------------------
// process a system get request
//------------------------------------------------------------------------------
hal_ret_t
system_get (const SystemGetRequest *req, SystemResponse *rsp)
{
    hal_ret_t                           ret = HAL_RET_OK;
    pd::pd_system_args_t                pd_system_args = { 0 };
    pd::pd_drop_stats_get_args_t        d_args;
    pd::pd_egress_drop_stats_get_args_t ed_args;
    pd::pd_table_stats_get_args_t       t_args;
    pd::pd_pb_stats_get_args_t          pb_args;
    pd::pd_func_args_t                  pd_func_args = {0};
    auto                                req_type = req->request();
    if_t                                *hal_if = NULL;

    // HAL_TRACE_DEBUG("--------------------- API Start ------------------------");
    // HAL_TRACE_DEBUG("Querying Drop Stats:");


    pd::pd_system_args_init(&pd_system_args);
    pd_system_args.rsp = rsp;

    if (req_type == sys::SYSTEM_GET_INB_MGMT_IF) {
        hal_if = find_if_by_handle(g_hal_state->inb_bond_active_uplink());
        rsp->set_inb_mgmt_if_id(hal_if ? hal_if->if_id : 0);
        rsp->set_api_status(types::API_STATUS_OK);
        goto end;
    }

    if ((req_type == sys::SYSTEM_GET_DROP_STATS) ||
        (req_type == sys::SYSTEM_GET_ALL_STATS)) {
        d_args.pd_sys_args = &pd_system_args;
        pd_func_args.pd_drop_stats_get = &d_args;
        ret = pd::hal_pd_call(pd::PD_FUNC_ID_DROP_STATS_GET, &pd_func_args);
        if (ret != HAL_RET_OK) {
            HAL_TRACE_ERR("Failed to get drop stats, err : {}", ret);
            rsp->set_api_status(types::API_STATUS_HW_PROG_ERR);
            goto end;
        }
        ed_args.pd_sys_args = &pd_system_args;
        pd_func_args.pd_egress_drop_stats_get = &ed_args;
        ret = pd::hal_pd_call(pd::PD_FUNC_ID_EGRESS_DROP_STATS_GET, &pd_func_args);
        if (ret != HAL_RET_OK) {
            HAL_TRACE_ERR("Failed to get egress drop stats, err : {}", ret);
            rsp->set_api_status(types::API_STATUS_HW_PROG_ERR);
            goto end;
        }
    }

    if ((req_type == sys::SYSTEM_GET_TABLE_STATS) ||
        (req_type == sys::SYSTEM_GET_ALL_STATS)) {
        t_args.pd_sys_args = &pd_system_args;
        pd_func_args.pd_table_stats_get = &t_args;
        ret = pd::hal_pd_call(pd::PD_FUNC_ID_TABLE_STATS_GET, &pd_func_args);
        if (ret != HAL_RET_OK) {
            HAL_TRACE_ERR("Failed to get drop stats, err : {}", ret);
            rsp->set_api_status(types::API_STATUS_HW_PROG_ERR);
            goto end;
        }
    }

    if ((req_type == sys::SYSTEM_GET_PB_STATS) ||
        (req_type == sys::SYSTEM_GET_DROP_STATS) ||
        (req_type == sys::SYSTEM_GET_ALL_STATS)) {
        pb_args.pd_sys_args = &pd_system_args;
        pd_func_args.pd_pb_stats_get = &pb_args;
        ret = pd::hal_pd_call(pd::PD_FUNC_ID_PB_STATS_GET, &pd_func_args);
        if (ret != HAL_RET_OK) {
            HAL_TRACE_ERR("Failed to get packet buffer stats, err : {}", ret);
            rsp->set_api_status(types::API_STATUS_HW_PROG_ERR);
            goto end;
        }
    }

    //unlock the cfg_db as fte dont need it, if needed fte has to take the lock again
    hal::hal_cfg_db_close();

    if (hal::g_hal_cfg.features != hal::HAL_FEATURE_SET_GFT) {

        if ((req_type == sys::SYSTEM_GET_FTE_STATS) ||
            (req_type == sys::SYSTEM_GET_ALL_STATS)) {
            // FTE stats get
            system_fte_stats_get(rsp);
            if (ret != HAL_RET_OK) {
                HAL_TRACE_ERR("Failed to get drop stats, err : {}", ret);
                rsp->set_api_status(types::API_STATUS_ERR);
                goto end;
            }

            // Session Summary get
            system_session_summary_get(rsp);
            if (ret != HAL_RET_OK) {
                HAL_TRACE_ERR("Failed to get session summary get, err : {}", ret);
                rsp->set_api_status(types::API_STATUS_ERR);
                goto end;
            }
        }

        if ((req_type == sys::SYSTEM_GET_FTE_TXRX_STATS) ||
            (req_type == sys::SYSTEM_GET_ALL_STATS)) {
            system_fte_txrx_stats_get(rsp);
            if (ret != HAL_RET_OK) {
                HAL_TRACE_ERR("Failed to get drop stats, err : {}", ret);
                rsp->set_api_status(types::API_STATUS_ERR);
                goto end;
            }
        }
    }

    rsp->set_api_status(types::API_STATUS_OK);

end:
    return HAL_RET_OK;
}

hal_ret_t
clear_ingress_drop_stats (void)
{
    hal_ret_t ret;
    pd::pd_func_args_t pd_func_args = {0};

    ret = pd::hal_pd_call(pd::PD_FUNC_ID_DROP_STATS_CLEAR, &pd_func_args);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Failed to clear drop stats, err : {}", ret);
        goto end;
    }
end:
    return ret;
}

hal_ret_t
clear_egress_drop_stats (void)
{
    hal_ret_t ret;
    pd::pd_func_args_t pd_func_args = {0};

    ret = pd::hal_pd_call(pd::PD_FUNC_ID_EGRESS_DROP_STATS_CLEAR, &pd_func_args);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Failed to clear egress drop stats, err : {}", ret);
        goto end;
    }
end:
    return ret;
}

hal_ret_t
clear_pb_stats (void)
{
    hal_ret_t          ret = HAL_RET_OK;
    pd::pd_func_args_t pd_func_args = {0};

    HAL_TRACE_DEBUG("--------------------- API Start ------------------------");
    HAL_TRACE_DEBUG("Clearing PB Stats:");

    ret = pd::hal_pd_call(pd::PD_FUNC_ID_PB_STATS_CLEAR, &pd_func_args);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Failed to clear pd stats, err : {}", ret);
        goto end;
    }
end:
    return ret;
}

hal_ret_t
upgrade_table_reset (void)
{
    hal_ret_t          ret = HAL_RET_OK;
    pd::pd_func_args_t pd_func_args = {0};

    HAL_TRACE_DEBUG("Clearing Table entries:");
    ret = pd::hal_pd_call(pd::PD_FUNC_ID_UPG_TABLE_RESET, &pd_func_args);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Failed to clear pd stats, err : {}", ret);
        goto end;
    }
end:
    return ret;
}

//------------------------------------------------------------------------------
// process a feature profile get request
//------------------------------------------------------------------------------
hal_ret_t
feature_profile_get (FeatureProfileResponse *rsp)
{
    switch (hal::g_hal_cfg.device_cfg.feature_profile) {
    case sdk::lib::FEATURE_PROFILE_NONE:
        rsp->set_feature_profile(sys::FEATURE_PROFILE_NONE);
        break;
    case sdk::lib::FEATURE_PROFILE_BASE:
        rsp->set_feature_profile(sys::FEATURE_PROFILE_BASE);
        break;
    case sdk::lib::FEATURE_PROFILE_STORAGE:
        rsp->set_feature_profile(sys::FEATURE_PROFILE_STORAGE);
        break;
    case sdk::lib::FEATURE_PROFILE_SRIOV:
        rsp->set_feature_profile(sys::FEATURE_PROFILE_SRIOV);
        break;
    case sdk::lib::FEATURE_PROFILE_VIRTUALIZED:
        rsp->set_feature_profile(sys::FEATURE_PROFILE_VIRTUALIZED);
        break;
    case sdk::lib::FEATURE_PROFILE_PROXY:
        rsp->set_feature_profile(sys::FEATURE_PROFILE_PROXY);
        break;
    case sdk::lib::FEATURE_PROFILE_DEVELOPER:
        rsp->set_feature_profile(sys::FEATURE_PROFILE_DEVELOPER);
        break;
    default:
        rsp->set_feature_profile(sys::FEATURE_PROFILE_NONE);
    }
    rsp->set_api_status(types::API_STATUS_OK);

    return HAL_RET_OK;
}

hal_ret_t
micro_seg_update(MicroSegUpdateRequest &req,
                 MicroSegUpdateResponse *rsp)
{
    proto_msg_dump(req);

    if ((hal::g_hal_cfg.device_cfg.micro_seg_en &&
        req.micro_seg_mode() == sys::MICRO_SEG_ENABLE) ||
        (!hal::g_hal_cfg.device_cfg.micro_seg_en &&
         req.micro_seg_mode() == sys::MICRO_SEG_DISABLE)) {
        HAL_TRACE_DEBUG("No change in Micro segmentation mode: {}. No-op",
                        MicroSegMode_Name(req.micro_seg_mode()));
        goto end;
    }

    HAL_TRACE_DEBUG("Micro segmentation mode change. {} -> {}",
                    hal::g_hal_cfg.device_cfg.micro_seg_en ?
                    MicroSegMode_Name(sys::MICRO_SEG_ENABLE) : MicroSegMode_Name(sys::MICRO_SEG_DISABLE),
                    MicroSegMode_Name(req.micro_seg_mode()));

    hal_handle_microseg_enable();

    // Send micro seg mode change to Nicmgr
    hal::svc::micro_seg_mode_notify(req.micro_seg_mode());

    hal::g_hal_cfg.device_cfg.micro_seg_en =
        (req.micro_seg_mode() == sys::MICRO_SEG_ENABLE) ? true : false;
end:
    rsp->set_api_status(types::API_STATUS_OK);
    return HAL_RET_OK;
}

hal_ret_t
micro_seg_status_update(MicroSegSpec &req,
                        MicroSegResponse *rsp)
{
    // TODO: Do we have to consider if HAL is done with cleanup.

    auto walk_cb = [](uint32_t event_id, void *entry, void *ctxt) {
        grpc::ServerWriter<EventResponse> *stream =
            (grpc::ServerWriter<EventResponse> *)ctxt;
        MicroSegSpec    *req  = (MicroSegSpec *)entry;
        EventResponse   evtresponse;
        MicroSegEvent   *event = evtresponse.mutable_micro_seg_event();

        HAL_TRACE_DEBUG("Sending Micro-Segmentation status: {}:{} to agent",
                        req->status(), MicroSegStatus_Name(req->status()));
        evtresponse.set_event_id(::event::EVENT_ID_MICRO_SEG_STATE);
        event->set_status(req->status());
        stream->Write(evtresponse);
        return true;
    };
    g_hal_state->event_mgr()->notify_event(::event::EVENT_ID_MICRO_SEG_STATE, &req, walk_cb);

    rsp->set_api_status(types::API_STATUS_OK);
    return HAL_RET_OK;
}

hal_ret_t
system_mseg_enf_to_transp (const SysSpec *spec)
{
    hal_ret_t ret = HAL_RET_OK;

    // Remove host enics from mgmt prom list
    ret = enicif_update_host_prom(false);

    // Remove Learn ACLs
    ret = hal_acl_micro_seg_deinit();

    // Stop FTE processing packets
    fte::fte_set_quiesce(0 /* FTE ID */, true);

    // Clear sessions
    hal::session_delete_all();

    // vMotion DeInit
    ret = vmotion_deinit();

    // Change mode in hal
    hal::g_hal_state->set_fwd_mode(spec->fwd_mode());
    hal::g_hal_state->set_policy_mode(spec->policy_mode());

    // Disable micro seg in nicmgr. Will create transp config
    hal::svc::micro_seg_mode_notify(sys::MICRO_SEG_DISABLE);

    // Add host enics in mseg/WL prom list
    ret = enicif_update_host_prom(true);

    return ret;
}

hal_ret_t
system_mseg_enf_to_transp_enf_flaware (const SysSpec *spec)
{
    hal_ret_t ret = HAL_RET_OK;

    ret = system_mseg_enf_to_transp(spec);

    // Start FTE process packets
    fte::fte_set_quiesce(0 /* FTE ID */, false);

    // Make host traffic WL traffic
    ret = hal::plugins::sfw::
        sfw_update_default_security_profile(L4_PROFILE_HOST_DEFAULT,true);

    return ret;
}

hal_ret_t
system_mseg_enf_to_transp_bnet (const SysSpec *spec)
{
    hal_ret_t ret = HAL_RET_OK;

    ret = system_mseg_enf_to_transp(spec);

    return ret;
}

#define IS_MODE(fwd_mode, fwd_mode_val, policy_mode, policy_mode_val) \
    ((fwd_mode == fwd_mode_val) && (policy_mode == policy_mode_val))
//------------------------------------------------------------------------------
//  Handle system mode changes (fwd & policy)
//------------------------------------------------------------------------------
hal_ret_t
system_handle_fwd_policy_updates(const SysSpec *spec,
                                 SysSpecResponse *rsp)
{
    hal_ret_t ret = HAL_RET_OK;
    auto spec_ref = *spec;

    proto_msg_dump(spec_ref);

    if (hal::g_hal_state->fwd_mode() == spec->fwd_mode() &&
        hal::g_hal_state->policy_mode() == spec->policy_mode()) {
        HAL_TRACE_WARN("No change in sys spec Noop");
        goto end;
    }

    HAL_TRACE_DEBUG("Mode change: {},{} => {},{}",
                    ForwardMode_Name(hal::g_hal_state->fwd_mode()),
                    PolicyMode_Name(hal::g_hal_state->policy_mode()),
                    ForwardMode_Name(spec->fwd_mode()),
                    PolicyMode_Name(spec->policy_mode()));

    // (Transparent, Basenet) => ...
    if (IS_MODE(hal::g_hal_state->fwd_mode(), sys::FWD_MODE_TRANSPARENT,
                hal::g_hal_state->policy_mode(), sys::POLICY_MODE_BASE_NET)) {

        if (IS_MODE(spec->fwd_mode(), sys::FWD_MODE_TRANSPARENT,
                    spec->policy_mode(), sys::POLICY_MODE_FLOW_AWARE) ||
            IS_MODE(spec->fwd_mode(), sys::FWD_MODE_TRANSPARENT,
                    spec->policy_mode(), sys::POLICY_MODE_ENFORCE)) {
             // 1. Change l4 profile to enable policy_enf_cfg_en to pull packets to FTE
            ret = hal::plugins::sfw::
                sfw_update_default_security_profile(L4_PROFILE_HOST_DEFAULT,true);

            // 2. Set FTE to stop Quiescing
            fte::fte_set_quiesce(0 /* FTE ID */, false);

            hal::g_hal_state->set_policy_mode(spec->policy_mode());
        }

        if (IS_MODE(spec->fwd_mode(), sys::FWD_MODE_MICROSEG,
                    spec->policy_mode(), sys::POLICY_MODE_FLOW_AWARE)) {
            // Not supported yet.
            hal::g_hal_state->set_fwd_mode(spec->fwd_mode());
            hal::g_hal_state->set_policy_mode(spec->policy_mode());
        }

        if (IS_MODE(spec->fwd_mode(), sys::FWD_MODE_MICROSEG,
                    spec->policy_mode(), sys::POLICY_MODE_ENFORCE)) {
            /*
             * 1. Change l4 profile to disable policy_enf_cfg_en to not pull packets to FTE
             *    To prevent shared mgmt pkts from uplink or host to not come to FTE
             */
            ret = hal::plugins::sfw::
                sfw_update_default_security_profile(L4_PROFILE_HOST_DEFAULT, false);

            // 1. Cleanup config from nicmgr.
            hal::svc::micro_seg_mode_notify(sys::MICRO_SEG_ENABLE);

            // 2. Remove host enics from mseg prom list
            ret = enicif_update_host_prom(false);

            // 3. Set up mode in hal state
            hal::g_hal_state->set_fwd_mode(spec->fwd_mode());

            // 4. Add host enics to mgmt prom list
            ret = enicif_update_host_prom(true);

            // 5. Install ACLs for micro seg mode.
            ret = hal_acl_micro_seg_init();

            // 6. vMotion Init
            ret = vmotion_init(stoi(hal::g_hal_cfg.vmotion_port));

            // 7. Set FTE to stop Quiescing
            fte::fte_set_quiesce(0 /* FTE ID */, false);

            hal::g_hal_state->set_policy_mode(spec->policy_mode());

            // Flap ports
            ret = system_sched_to_flap_ports();
            if (ret != HAL_RET_OK) {
                HAL_TRACE_ERR("Unable to sched for ports flap. err {}", ret);
            }
        }
    }

    // (Transparent, Flowaware) => ...
    if (IS_MODE(hal::g_hal_state->fwd_mode(), sys::FWD_MODE_TRANSPARENT,
                hal::g_hal_state->policy_mode(), sys::POLICY_MODE_FLOW_AWARE)) {

        // => (Transparent, Basenet)
        if (IS_MODE(spec->fwd_mode(), sys::FWD_MODE_TRANSPARENT,
                    spec->policy_mode(), sys::POLICY_MODE_BASE_NET)) {

            // Stop FTE processing of packets
            fte::fte_set_quiesce(0 /* FTE ID */, true);

            // Change l4 profile to disable policy_enf_cfg_en to
            // stop pulling packets to FTE
            ret = hal::plugins::sfw::
                sfw_update_default_security_profile(L4_PROFILE_HOST_DEFAULT,
                                                    false);

            // Clear sessions
            hal::session_delete_all();

            hal::g_hal_state->set_policy_mode(spec->policy_mode());
        }

        // => (Microseg, Enforce)
        if (IS_MODE(spec->fwd_mode(), sys::FWD_MODE_TRANSPARENT,
                    spec->policy_mode(), sys::POLICY_MODE_ENFORCE)) {
            hal::g_hal_state->set_policy_mode(spec->policy_mode());
        }

        if (IS_MODE(spec->fwd_mode(), sys::FWD_MODE_MICROSEG,
                    spec->policy_mode(), sys::POLICY_MODE_ENFORCE)) {

            // 1. Make host traffic management
            ret = hal::plugins::sfw::
                sfw_update_default_security_profile(L4_PROFILE_HOST_DEFAULT, false);

            // 2. Cleanup config from nicmgr.
            hal::svc::micro_seg_mode_notify(sys::MICRO_SEG_ENABLE);

            // 3. Remove host enics from mseg prom list
            ret = enicif_update_host_prom(false);

            // 4. Mark Quiesce on in FTE
            fte::fte_set_quiesce(0 /* FTE ID */, true);

            // 5. Clear sessions
            hal::session_delete_all();

            // 6. Change mode
            hal::g_hal_state->set_fwd_mode(spec->fwd_mode());
            hal::g_hal_state->set_policy_mode(spec->policy_mode());

            // 7. Add host enics to mgmt prom list
            ret = enicif_update_host_prom(true);

            // 8. Install ACLs for micro seg mode.
            ret = hal_acl_micro_seg_init();

            // 9. vMotion Init
            ret = vmotion_init(stoi(hal::g_hal_cfg.vmotion_port));

            // 10. Mark Quiesce off in FTE
            fte::fte_set_quiesce(0 /* FTE ID */, false);

            // Flap ports
            ret = system_sched_to_flap_ports();
            if (ret != HAL_RET_OK) {
                HAL_TRACE_ERR("Unable to sched for ports flap. err {}", ret);
            }
        }
    }

    // (Transparent, Enforce) => ...
    if (IS_MODE(hal::g_hal_state->fwd_mode(), sys::FWD_MODE_TRANSPARENT,
                hal::g_hal_state->policy_mode(), sys::POLICY_MODE_ENFORCE)) {

        if (IS_MODE(spec->fwd_mode(), sys::FWD_MODE_TRANSPARENT,
                    spec->policy_mode(), sys::POLICY_MODE_BASE_NET)) {
            // Stop FTE processing packets
            fte::fte_set_quiesce(0 /* FTE ID */, true);

            // Change l4 profile to not send packets to FTE
            ret = hal::plugins::sfw::
                sfw_update_default_security_profile(L4_PROFILE_HOST_DEFAULT,
                                                    false);

            // Clear sessions
            hal::session_delete_all();
            
            // Update policy
            hal::g_hal_state->set_policy_mode(spec->policy_mode());
        }

        if (IS_MODE(spec->fwd_mode(), sys::FWD_MODE_TRANSPARENT,
                    spec->policy_mode(), sys::POLICY_MODE_FLOW_AWARE)) {
            // Stop FTE processing packets
            fte::fte_set_quiesce(0 /* FTE ID */, true);

            // Clear sessions
            hal::session_delete_all();

            // Start FTE processing packets
            fte::fte_set_quiesce(0, false);

            // Update policy
            hal::g_hal_state->set_policy_mode(spec->policy_mode());
        }

        if (IS_MODE(spec->fwd_mode(), sys::FWD_MODE_MICROSEG,
                    spec->policy_mode(), sys::POLICY_MODE_ENFORCE)) {

            // 1. Make host traffic management
            ret = hal::plugins::sfw::
                sfw_update_default_security_profile(L4_PROFILE_HOST_DEFAULT, false);

            // 2. Cleanup config from nicmgr.
            hal::svc::micro_seg_mode_notify(sys::MICRO_SEG_ENABLE);

            // 3. Remove host enics from mseg prom list
            ret = enicif_update_host_prom(false);

            // 4. Mark Quiesce on in FTE
            fte::fte_set_quiesce(0 /* FTE ID */, true);

            // 5. Clear sessions
            hal::session_delete_all();

            // 6. Change mode
            hal::g_hal_state->set_fwd_mode(spec->fwd_mode());
            hal::g_hal_state->set_policy_mode(spec->policy_mode());

            // 7. Add host enics to mgmt prom list
            ret = enicif_update_host_prom(true);

            // 8. Install ACLs for micro seg mode.
            ret = hal_acl_micro_seg_init();

            // 9. vMotion Init
            ret = vmotion_init(stoi(hal::g_hal_cfg.vmotion_port));

            // 10. Mark Quiesce off in FTE
            fte::fte_set_quiesce(0 /* FTE ID */, false);

            // Flap ports
            ret = system_sched_to_flap_ports();
            if (ret != HAL_RET_OK) {
                HAL_TRACE_ERR("Unable to sched for ports flap. err {}", ret);
            }
        }
    }

    // (Microseg, Enforce) => ...
    if (IS_MODE(hal::g_hal_state->fwd_mode(), sys::FWD_MODE_MICROSEG,
                hal::g_hal_state->policy_mode(), sys::POLICY_MODE_ENFORCE)) {

        // (Microseg, Enforce) => (Transparent, Enforce) ||
        //                        (Transparent, Flowaware)
        if (IS_MODE(spec->fwd_mode(), sys::FWD_MODE_TRANSPARENT,
                    spec->policy_mode(), sys::POLICY_MODE_ENFORCE) ||
            IS_MODE(spec->fwd_mode(), sys::FWD_MODE_TRANSPARENT,
                    spec->policy_mode(), sys::POLICY_MODE_FLOW_AWARE)) {
            ret = system_mseg_enf_to_transp_enf_flaware(spec);
        }

        // (Microseg, Enforce) => (Transparent, Basenet)
        if (IS_MODE(spec->fwd_mode(), sys::FWD_MODE_TRANSPARENT,
                    spec->policy_mode(), sys::POLICY_MODE_BASE_NET)) {
            ret = system_mseg_enf_to_transp_bnet(spec);
        }
    }

end:
    rsp->set_api_status(types::API_STATUS_OK);
    return ret;
}

//----------------------------------------------------------------------------
// Schedule to flap ports
//----------------------------------------------------------------------------
hal_ret_t
system_sched_to_flap_ports (void)
{
    auto flap_cb = [](void *timer, uint32_t timer_id, void *ctxt) {
        HAL_TRACE_DEBUG("Flapping ports for VMs to trigger RARPs.");

        port_update_type_admin_state(port_type_t::PORT_TYPE_ETH,
                                     port_admin_state_t::PORT_ADMIN_STATE_DOWN);

        port_update_type_admin_state(port_type_t::PORT_TYPE_ETH,
                                     port_admin_state_t::PORT_ADMIN_STATE_UP);

    };

    HAL_TRACE_DEBUG("Scheduling timer to flap ports");
    sdk::lib::timer_schedule(HAL_TIMER_ID_MICRO_SEG_ENF_FLAP,
                             HAL_MICRO_SEG_ENF_FLAP_TIME,
                             NULL, flap_cb, false);

    return HAL_RET_OK;
}

//----------------------------------------------------------------------------
// Handle system policy & forward mode get
//----------------------------------------------------------------------------
hal_ret_t
system_get_fwd_policy_mode(SysSpecGetResponse *rsp) {
   rsp->mutable_spec()->set_fwd_mode(hal::g_hal_state->fwd_mode());
   rsp->mutable_spec()->set_policy_mode(hal::g_hal_state->policy_mode());

   rsp->set_api_status(types::API_STATUS_OK);
   return HAL_RET_OK;
}

//----------------------------------------------------------------------------
// Upgrade A => B. If A is in hostpin, keep B in micro-seg enforce.
//----------------------------------------------------------------------------
hal_ret_t
system_handle_a_to_b (void)
{
    hal_ret_t       ret = HAL_RET_OK;
    SysSpec         sys_spec;
    SysSpecResponse sys_rsp;

    if (g_hal_state->forwarding_mode() == sdk::lib::FORWARDING_MODE_HOSTPIN) {
        HAL_TRACE_DEBUG("Upgrade A(Hostpin) -> B. Moving to microseg-enforce");
        sys_spec.set_fwd_mode(sys::FWD_MODE_MICROSEG);
        sys_spec.set_policy_mode(sys::POLICY_MODE_ENFORCE);

        ret = system_handle_fwd_policy_updates(&sys_spec, &sys_rsp);
        if (ret != HAL_RET_OK) {
            HAL_TRACE_ERR("Upgrade A(Hostpin) -> B. Failed to move "
                          "to microseg-enforce");
            goto end;
        }
    }

end:
    return ret;
}

}    // namespace hal
