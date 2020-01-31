//-----------------------------------------------------------------------------
// {C} Copyright 2018 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------

#include <iostream>
#include "grpc++/grpc++.h"
#include "nic/linkmgr/delphi/linkmgr_delphi.hpp"
#include "gen/proto/port.delphi_utest.hpp"
#include "nic/linkmgr/test/utils/linkmgr_base_test.hpp"

namespace {

using namespace std;
using namespace linkmgr;

PORTSPEC_REACTOR_TEST(port_svc_test, port_svc);

TEST_F(port_svc_test, PortSpecTest) {
    linkmgr_initialize("");

    // port spec parameters
    delphi::objects::PortSpecPtr spec = std::make_shared<delphi::objects::PortSpec>();
    spec->mutable_key_or_handle()->set_port_id(1);
    spec->set_port_speed(port::PORT_SPEED_100G);
    spec->set_num_lanes(1);
    spec->set_port_type(::port::PORT_TYPE_ETH);
    spec->set_admin_state(port::PORT_ADMIN_STATE_UP);

    // create port spec
    sdk_->QueueUpdate(spec);

    // verify port spec got created
    ASSERT_EQ_EVENTUALLY(sdk_->ListKind("PortSpec").size(), 1) << "Port spec object was not created";
    ASSERT_EQ_EVENTUALLY(delphi::objects::PortSpec::List(sdk_)[0]->admin_state(), port::PORT_ADMIN_STATE_UP) << "invalid port state";

    // update the port admin state
    spec->set_admin_state(port::PORT_ADMIN_STATE_DOWN);
    sdk_->QueueUpdate(spec);

    // verify the objetc got updated
    ASSERT_EQ_EVENTUALLY(delphi::objects::PortSpec::List(sdk_)[0]->admin_state(), port::PORT_ADMIN_STATE_DOWN) << "invalid port state";

    // delete the port spec
    sdk_->QueueDelete(spec);

    // verify object was removed
    ASSERT_EQ_EVENTUALLY(sdk_->ListKind("PortSpec").size(), 0) << "Port spec object was not deleted";
}

} // namespace

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
