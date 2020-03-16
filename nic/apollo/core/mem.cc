/**
 * Copyright (c) 2018 Pensando Systems, Inc.
 *
 * @file    mem.cc
 *
 * @brief   This file contains generic memory handling for PDS
 */

#include "nic/sdk/lib/logger/logger.hpp"
#include "nic/sdk/lib/periodic/periodic.hpp"
#include "nic/apollo/core/mem.hpp"
#include "nic/apollo/core/trace.hpp"
#include "nic/apollo/api/vpc.hpp"
#include "nic/apollo/api/subnet.hpp"
#include "nic/apollo/api/vnic.hpp"
#include "nic/apollo/api/pds_state.hpp"
#include "nic/apollo/learn/ep_ip_entry.hpp"
#include "nic/apollo/learn/ep_mac_entry.hpp"

sdk_logger::trace_cb_t    g_trace_cb;

// TODO: move all the calls to xxx_free() to inside xxx_entry()::destroy()

namespace api {

// TODO: provide an API to enable/disable this ?
static bool g_delay_delete = false;

/**
 * @brief callback invoked by the timerwheel to release an object to its slab
 *
 * @param[in]    timer      timer context
 * @param[in]    slab_id    identifier of the slab to free the element to
 * @param[in]    elem       element to free to the given slab
 */
void
slab_delay_delete_cb (void *timer, uint32_t slab_id, void *elem)
{
    PDS_TRACE_VERBOSE("timer %p, slab %u, elem %p", timer, slab_id, elem);
    switch (slab_id) {
    case PDS_SLAB_ID_DEVICE:
        device_entry::destroy((device_entry *)elem);
        break;

    case PDS_SLAB_ID_TEP:
        tep_entry::destroy((tep_entry *)elem);
        break;

    case PDS_SLAB_ID_IF:
        if_entry::destroy((if_entry *)elem);
        break;

    case PDS_SLAB_ID_VPC:
        vpc_entry::destroy((vpc_entry *)elem);
        break;

    case PDS_SLAB_ID_SUBNET:
        subnet_entry::destroy((subnet_entry *)elem);
        break;

    case PDS_SLAB_ID_VNIC:
        vnic_entry::destroy((vnic_entry *)elem);
        break;

    case PDS_SLAB_ID_MAPPING:
        mapping_entry::destroy((mapping_entry *)elem);
        break;

    case PDS_SLAB_ID_ROUTE_TABLE:
        route_table::destroy((route_table *)elem);
        break;

    case PDS_SLAB_ID_POLICY:
        policy::destroy((policy *)elem);
        break;

    case PDS_SLAB_ID_MIRROR_SESSION:
        mirror_session::destroy((mirror_session *)elem);
        break;

    case PDS_SLAB_ID_METER:
        meter_entry::destroy((meter_entry *)elem);
        break;

    case PDS_SLAB_ID_TAG:
        tag_entry::destroy((tag_entry *)elem);
        break;

    case PDS_SLAB_ID_SVC_MAPPING:
        svc_mapping::destroy((svc_mapping *)elem);
        break;

    case PDS_SLAB_ID_VPC_PEER:
        vpc_peer_entry::destroy((vpc_peer_entry *)elem);
        break;

    case PDS_SLAB_ID_NEXTHOP:
        nexthop::destroy((nexthop *)elem);
        break;

    case PDS_SLAB_ID_NEXTHOP_GROUP:
        nexthop_group::destroy((nexthop_group *)elem);
        break;

    case PDS_SLAB_ID_POLICER:
        policer_entry::destroy((policer_entry *)elem);
        break;

    case PDS_SLAB_ID_NAT_PORT_BLOCK:
        nat_port_block::destroy((nat_port_block *)elem);
        break;

    case PDS_SLAB_ID_DHCP_POLICY:
        dhcp_policy::destroy((dhcp_policy *)elem);
        break;

    case PDS_SLAB_ID_SECURITY_PROFILE:
        security_profile::destroy((security_profile *)elem);
        break;

    case PDS_SLAB_ID_MAC_ENTRY:
        ep_mac_entry::destroy((ep_mac_entry *)elem);
        break;

    case PDS_SLAB_ID_IP_ENTRY:
        ep_ip_entry::destroy((ep_ip_entry *)elem);
        break;

    default:
        PDS_TRACE_ERR("Unknown slab id {}", slab_id);
        SDK_ASSERT(false);
    }
    return;
}

/**
 * @brief wrapper function to delay delete slab elements
 *
 * @param[in]     slab_id    identifier of the slab to free the element to
 * @param[int]    elem       element to free to the given slab
 * @return #SDK_RET_OK on success, failure status code on error
 *
 * NOTE: currently delay delete timeout is PDS_DELAY_DELETE_MSECS, it is
 *       expected that other thread(s) using (a pointer to) this object should
 *       be done using this object within this timeout or else this memory can
 *       be freed and allocated for other objects and can result in corruptions.
 *       Essentially, PDS_DELAY_DELETE is assumed to be infinite
 */
sdk_ret_t
delay_delete_to_slab (uint32_t slab_id, void *elem)
{
    void    *timer_ctxt;

    if (slab_id >= PDS_SLAB_ID_MAX) {
        PDS_TRACE_ERR("Unexpected slab id {}", slab_id);
        return sdk::SDK_RET_INVALID_ARG;
    }
    if (g_delay_delete && sdk::lib::periodic_thread_is_running()) {
        timer_ctxt =
            sdk::lib::timer_schedule(slab_id, PDS_DELAY_DELETE_MSECS, elem,
                          (sdk::lib::twheel_cb_t)slab_delay_delete_cb, false);
        if (!timer_ctxt) {
            return sdk::SDK_RET_ERR;
        }
    } else {
        slab_delay_delete_cb(NULL, slab_id, elem);
    }
    return SDK_RET_OK;
}

}    // namespace api
