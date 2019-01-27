/**
 * Copyright (c) 2018 Pensando Systems, Inc.
 *
 * @file    mem.cc
 *
 * @brief   This file contains generic memory handling for OCI
 */

#include "nic/sdk/lib/logger/logger.hpp"
#include "nic/sdk/lib/periodic/periodic.hpp"
#include "nic/apollo/core/mem.hpp"
#include "nic/apollo/core/trace.hpp"
#include "nic/apollo/api/vcn.hpp"
#include "nic/apollo/api/subnet.hpp"
#include "nic/apollo/api/vnic.hpp"
#include "nic/apollo/api/oci_state.hpp"

sdk_logger::trace_cb_t    g_trace_cb;

// TODO: move all the calls to xxx_free() to inside xxx_entry()::destroy()

namespace api {

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
    switch (slab_id) {
    case OCI_SLAB_ID_SWITCHPORT:
        switchport_entry::destroy((switchport_entry *)elem);
        switchport_db()->switchport_free((switchport_entry *)elem);
        break;

    case OCI_SLAB_ID_TEP:
        tep_entry::destroy((tep_entry *)elem);
        tep_db()->tep_free((tep_entry *)elem);
        break;

    case OCI_SLAB_ID_VCN:
        vcn_entry::destroy((vcn_entry *)elem);
        vcn_db()->vcn_free((vcn_entry *)elem);
        break;

    case OCI_SLAB_ID_SUBNET:
        subnet_entry::destroy((subnet_entry *)elem);
        subnet_db()->subnet_free((subnet_entry *)elem);
        break;

    case OCI_SLAB_ID_VNIC:
        vnic_entry::destroy((vnic_entry *)elem);
        vnic_db()->vnic_free((vnic_entry *)elem);
        break;

    case OCI_SLAB_ID_MAPPING:
        mapping_entry::destroy((mapping_entry *)elem);
        mapping_db()->mapping_free((mapping_entry *)elem);
        break;

    case OCI_SLAB_ID_ROUTE_TABLE:
        route_table::destroy((route_table *)elem);
        route_table_db()->route_table_free((route_table *)elem);
        break;

    default:
        OCI_TRACE_ERR("Unknown slab id {}", slab_id);
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
 * NOTE: currently delay delete timeout is OCI_DELAY_DELETE_MSECS, it is
 *       expected that other thread(s) using (a pointer to) this object should
 *       be done using this object within this timeout or else this memory can
 *       be freed and allocated for other objects and can result in corruptions.
 *       Essentially, OCI_DELAY_DELETE is assumed to be infinite
 */
sdk_ret_t
delay_delete_to_slab (uint32_t slab_id, void *elem)
{
    void    *timer_ctxt;

    if (slab_id >= OCI_SLAB_ID_MAX) {
        OCI_TRACE_ERR("Unexpected slab id {}", slab_id);
        return sdk::SDK_RET_INVALID_ARG;
    }
    if (sdk::lib::periodic_thread_is_running()) {
        timer_ctxt =
            sdk::lib::timer_schedule(slab_id, OCI_DELAY_DELETE_MSECS, elem,
                                     (sdk::lib::twheel_cb_t)slab_delay_delete_cb,
                                     false);
        if (!timer_ctxt) {
            return sdk::SDK_RET_ERR;
        }
    }
    return SDK_RET_OK;
}

}    // namespace api
