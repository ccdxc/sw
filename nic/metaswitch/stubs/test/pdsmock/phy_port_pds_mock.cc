//------------------------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//------------------------------------------------------------------------------

#include "nic/metaswitch/stubs/test/pdsmock/phy_port_pds_mock.hpp"
#include "nic/metaswitch/stubs/hals/pds_ms_hal_init.hpp"
#include "nic/metaswitch/stubs/hals/pds_ms_li.hpp"
#include "nic/metaswitch/stubs/common/pdsa_cookie.hpp"
#include "nic/metaswitch/stubs/common/pdsa_state.hpp"
#include "nic/apollo/test/base/utils.hpp"
#include <cstdlib>
#include <cstdio>

namespace pdsa_test {

void load_phy_port_test_output ()
{
    static phy_port_pds_mock_t g_phy_port_pds_mock;
    test_params()->test_output = &g_phy_port_pds_mock;
}

void phy_port_pds_mock_t::generate_addupd_specs(const phy_port_input_params_t& input,
                                                batch_spec_t& pds_batch) 
{
    auto op = (op_create_)?API_OP_CREATE : API_OP_UPDATE;
    test::api::if_feeder if_feeder;
    pds_if_spec_t spec = {0};
    spec.key.id = IFINDEX(IF_TYPE_L3, 1,input.phy_port,0);
   // spec.key.id = IFINDEX(IF_TYPE_L3, 1,1,0);
    spec.type = PDS_IF_TYPE_L3;
    spec.admin_state = input.admin_state ? PDS_IF_STATE_UP:PDS_IF_STATE_DOWN;

    // Get Linux MAC address
    auto fp = popen ("ifconfig eth0 | grep ether | awk '{print $2}'", "r");
    if (!fp) {
        throw std::runtime_error ("ERROR Fetching Linux MAC");
    }
    char buf[100];
    while (std::fgets(buf, sizeof buf, fp) != NULL) {
        std::cout << '"' << buf << '"' << '\n';
    }
    pclose (fp);    

    mac_str_to_addr(buf, spec.l3_if_info.mac_addr);
    // TODO: Change to EthIfIndex when HAL supports it 
    // spec.l3_if_info.port_num = ETH_IFINDEX (1,1,1);
    spec.l3_if_info.port_num = input.phy_port-1;
    spec.l3_if_info.encap.type = PDS_ENCAP_TYPE_NONE; 
    spec.l3_if_info.encap.val.vnid = 0;
    spec.l3_if_info.vpc.id = 0;
    pds_batch.emplace_back (OBJ_ID_IF, op);
    auto& if_spec = pds_batch.back().intf;
    if_spec = spec; 
}

void phy_port_pds_mock_t::generate_del_specs(const phy_port_input_params_t& input,
                                          batch_spec_t& pds_batch) 
{
    pds_batch.emplace_back (OBJ_ID_IF, API_OP_DELETE);
    pds_batch.back().intf.key.id = IFINDEX(IF_TYPE_L3, 1,1,0);
}

void phy_port_pds_mock_t::validate_()
{    
    if (mock_pds_spec_op_fail_ ||
        mock_pds_batch_commit_fail_) {
        // Verify all temporary objects and cookies are freed
        auto state_ctxt = pdsa_stub::state_t::thread_context();
        auto state = state_ctxt.state();
        ASSERT_TRUE (state->get_slab_in_use (pdsa_stub::PDSA_IF_SLAB_ID) == (num_if_objs_));
        ASSERT_TRUE (state->get_slab_in_use (pdsa_stub::PDSA_COOKIE_SLAB_ID) == 0);
        return;
    }

     // Verify temporary objects and cookie are created as expected

    { // Enter state thread context
        auto state_ctxt = pdsa_stub::state_t::thread_context();
        auto state = state_ctxt.state();
        if (op_delete_) {
            // Object is removed from store synchronously for deletes
            ASSERT_TRUE (state->get_slab_in_use (pdsa_stub::PDSA_IF_SLAB_ID) == (num_if_objs_-1));
        } else {
            ASSERT_TRUE (state->get_slab_in_use (pdsa_stub::PDSA_IF_SLAB_ID) == (num_if_objs_+1));
        }
        ASSERT_TRUE (state->get_slab_in_use (pdsa_stub::PDSA_COOKIE_SLAB_ID) == 1);
    }

    ASSERT_TRUE (pds_ret_status != mock_pds_batch_async_fail_);
    
    // Mock callback
    auto pds_mock = dynamic_cast<pds_mock_t*>(test_params()->test_output);
    auto cookie = (pdsa_stub::cookie_t*) pds_mock->cookie;
    pdsa_stub::hal_callback((mock_pds_batch_async_fail_) ? SDK_RET_ERR : SDK_RET_OK,
                            cookie);

    if (mock_pds_batch_async_fail_) {
        // Verify no change to slab - all temporary objects released
        auto state_ctxt = pdsa_stub::state_t::thread_context();
        auto state = state_ctxt.state();
        ASSERT_TRUE (state->get_slab_in_use (pdsa_stub::PDSA_IF_SLAB_ID) == num_if_objs_);
        ASSERT_TRUE (state->get_slab_in_use (pdsa_stub::PDSA_COOKIE_SLAB_ID) == 0);
        return;
    }

    if (op_create_) { ++num_if_objs_;}
    if (op_delete_) { --num_if_objs_;}
    auto state_ctxt = pdsa_stub::state_t::thread_context();
    auto state = state_ctxt.state();
    ASSERT_TRUE (state->get_slab_in_use (pdsa_stub::PDSA_IF_SLAB_ID) == num_if_objs_);
    ASSERT_TRUE (state->get_slab_in_use (pdsa_stub::PDSA_COOKIE_SLAB_ID) == 0);
}

} // End namespace pdsa_test
