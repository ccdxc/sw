//-----------------------------------------------------------------------------
// {C} Copyright 2017 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------

#include "nic/include/base.h"
#include "nic/hal/hal.hpp"
#include "nic/include/hal_lock.hpp"
#include "nic/include/hal_state.hpp"
#include "nic/gen/hal/include/hal_api_stats.hpp"
#include "nic/hal/src/internal/system.hpp"
#include "nic/include/pd_api.hpp"

namespace hal {

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
    case sys::API_COUNTER_L4LBSERVICE_CREATE:
        entry->set_num_api_call(g_hal_state->api_stats(HAL_API_L4LBSERVICE_CREATE));
        entry->set_num_api_success(g_hal_state->api_stats(HAL_API_L4LBSERVICE_CREATE_SUCCESS));
        entry->set_num_api_fail(g_hal_state->api_stats(HAL_API_L4LBSERVICE_CREATE_FAIL));
        break;
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
// process a vrf get request
//------------------------------------------------------------------------------
hal_ret_t
system_get (SystemResponse *rsp)
{
    hal_ret_t                           ret = HAL_RET_OK;
    pd::pd_system_args_t                pd_system_args = { 0 };
    pd::pd_drop_stats_get_args_t        d_args;
    pd::pd_egress_drop_stats_get_args_t ed_args;
    pd::pd_table_stats_get_args_t       t_args;
    pd::pd_func_args_t                  pd_func_args = {0};

    HAL_TRACE_DEBUG("--------------------- API Start ------------------------");
    HAL_TRACE_DEBUG("Querying Drop Stats:");


    pd::pd_system_args_init(&pd_system_args);
    pd_system_args.rsp = rsp;


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

    t_args.pd_sys_args = &pd_system_args;
    pd_func_args.pd_table_stats_get = &t_args;
    ret = pd::hal_pd_call(pd::PD_FUNC_ID_TABLE_STATS_GET, &pd_func_args);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Failed to get drop stats, err : {}", ret);
        rsp->set_api_status(types::API_STATUS_HW_PROG_ERR);
        goto end;
    }

    rsp->set_api_status(types::API_STATUS_OK);

end:

    return HAL_RET_OK;
}

}    // namespace hal
