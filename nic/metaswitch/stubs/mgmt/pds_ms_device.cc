// {C} Copyright 2019 Pensando Systems Inc. All rights reserved

#include "nic/metaswitch/stubs/mgmt/pds_ms_device.hpp"
#include "nic/metaswitch/stubs/mgmt/pds_ms_mgmt_state.hpp"
#include "nic/metaswitch/stubs/mgmt/pds_ms_stubs_utils.hpp"

namespace pds_ms {
void device_create (pds_device_spec_t *spec)
{
    PDS_TRACE_INFO("Device Config Overlay Routing %s",
                    (spec->overlay_routing_en) ? "Enabled" : "Disabled");
    {
        auto mgmt_ctxt = mgmt_state_t::thread_context();
        mgmt_ctxt.state()->set_overlay_routing_en(
                       spec->overlay_routing_en);
    }
    pds_ms_hals_stub_init();
}

} // End namespace
