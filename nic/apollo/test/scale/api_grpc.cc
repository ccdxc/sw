//------------------------------------------------------------------------------
// Copyright (c) 2019 Pensando Systems, Inc.
//------------------------------------------------------------------------------

#include <iostream>
#include <math.h>
#include "nic/apollo/agent/client/app.hpp"
#include "nic/apollo/test/scale/test_common.hpp"

static int g_epoch = 1;
static pds_batch_ctxt_t g_bctxt = PDS_BATCH_CTXT_INVALID;

static inline bool
pds_batching_enabled (void)
{
    if (getenv("BATCHING_DISABLED")) {
        return FALSE;
    }
    return TRUE;
}

sdk_ret_t
create_route_table (pds_route_table_spec_t *route_table)
{
    sdk_ret_t ret;

    ret = create_route_table_grpc(route_table);
    SDK_ASSERT_TRACE_RETURN((ret == SDK_RET_OK), ret,
                            "create route table failed!, ret %u", ret);
    // Batching: push leftover objects
    ret = create_route_table_grpc(NULL);
    SDK_ASSERT_TRACE_RETURN((ret == SDK_RET_OK), ret,
                            "create route table failed!, ret %u", ret);
    if (pds_batching_enabled()) {
        ret = batch_commit_grpc(g_bctxt);
        SDK_ASSERT_TRACE_RETURN((ret == SDK_RET_OK), ret,
                                "Batch commit failed!, ret %u", ret);
        g_bctxt = batch_start_grpc(g_epoch++);
        SDK_ASSERT_TRACE_RETURN((g_bctxt != PDS_BATCH_CTXT_INVALID),
                                SDK_RET_ERR,
                                "Batch start failed!");
    }
    return ret;
}

sdk_ret_t
create_svc_mapping (pds_svc_mapping_spec_t *svc_mapping)
{
    return create_svc_mapping_grpc(svc_mapping);
}

sdk_ret_t
create_local_mapping (pds_local_mapping_spec_t *pds_local_mapping)
{
    return create_local_mapping_grpc(pds_local_mapping);
}

sdk_ret_t
create_remote_mapping (pds_remote_mapping_spec_t *pds_remote_mapping)
{
    return create_remote_mapping_grpc(pds_remote_mapping);
}

sdk_ret_t
create_vnic (pds_vnic_spec_t *pds_vnic)
{
    return create_vnic_grpc(pds_vnic);
}

sdk_ret_t
create_vpc (pds_vpc_spec_t *pds_vpc)
{
    return create_vpc_grpc(pds_vpc);
}

sdk_ret_t
read_vpc (pds_vpc_key_t *key, pds_vpc_info_t *info)
{
    return read_vpc_grpc(key, info);
}

sdk_ret_t
update_vpc (pds_vpc_spec_t *pds_vpc)
{
    return update_vpc_grpc(pds_vpc);
}

sdk_ret_t
delete_vpc (pds_vpc_key_t *key)
{
    return delete_vpc_grpc(key);
}

sdk_ret_t
create_vpc_peer (pds_vpc_peer_spec_t *pds_vpc_peer)
{
    return create_vpc_peer_grpc(pds_vpc_peer);
}

sdk_ret_t
create_l3_intf (pds_if_spec_t *pds_if)
{
    return create_l3_intf_grpc(pds_if);
}

sdk_ret_t
create_tag (pds_tag_spec_t *pds_tag)
{
    return create_tag_grpc(pds_tag);
}

sdk_ret_t
create_meter (pds_meter_spec_t *pds_meter)
{
    return create_meter_grpc(pds_meter);
}

sdk_ret_t
create_nexthop (pds_nexthop_spec_t *pds_nh)
{
    return create_nexthop_grpc(pds_nh);
}

sdk_ret_t
create_tunnel (uint32_t id, pds_tep_spec_t *pds_tep)
{
    return create_tunnel_grpc(id, pds_tep);
}

sdk_ret_t
create_subnet (pds_subnet_spec_t *pds_subnet)
{
    return create_subnet_grpc(pds_subnet);
}

sdk_ret_t
create_device (pds_device_spec_t *device)
{
    return create_device_grpc(device);
}

sdk_ret_t
create_policy (pds_policy_spec_t *policy)
{
    sdk_ret_t ret;
    ret = create_policy_grpc(policy);
    SDK_ASSERT_TRACE_RETURN((ret == SDK_RET_OK), ret,
                            "create policy failed!, ret %u", ret);
    ret = create_policy_grpc(NULL);
    SDK_ASSERT_TRACE_RETURN((ret == SDK_RET_OK), ret,
                            "create policy failed!, ret %u", ret);
    return ret;
}

sdk_ret_t
create_mirror_session (pds_mirror_session_spec_t *ms)
{
    sdk_ret_t ret;
    ret = create_mirror_session_grpc(ms);
    SDK_ASSERT_TRACE_RETURN((ret == SDK_RET_OK), ret,
                            "create mirror failed!, ret %u", ret);
    // push leftover objects
    ret = create_mirror_session_grpc(NULL);
    SDK_ASSERT_TRACE_RETURN((ret == SDK_RET_OK), ret,
                            "create mirror failed!, ret %u", ret);
    return ret;
}

sdk_ret_t
create_objects_init (test_params_t *test_params)
{
    if (pds_batching_enabled()) {
        g_bctxt = batch_start_grpc(g_epoch++);
        SDK_ASSERT_TRACE_RETURN((g_bctxt != PDS_BATCH_CTXT_INVALID),
                                SDK_RET_ERR,
                                "Batch start failed!");
    }
    return SDK_RET_OK;
}

sdk_ret_t
create_objects_end (void)
{
    sdk_ret_t ret;

    if (pds_batching_enabled()) {
        // TODO: hack until hooks is cleaned up
        (void)batch_start_grpc(PDS_EPOCH_INVALID);
        ret = batch_commit_grpc(g_bctxt);
        SDK_ASSERT_TRACE_RETURN((ret == SDK_RET_OK), ret,
                                "Batch commit failed!, ret %u", ret);
    }
    return SDK_RET_OK;
}
