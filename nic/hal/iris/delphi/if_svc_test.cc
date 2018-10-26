//-----------------------------------------------------------------------------
// {C} Copyright 2018 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------

#include <iostream>
#include "grpc++/grpc++.h"
#include "nic/hal/iris/delphi/if_svc.hpp"
#include "gen/proto/interface.delphi_utest.hpp"
#include "nic/hal/test/utils/hal_test_utils.hpp"
#include "nic/hal/test/utils/hal_base_test.hpp"
#include "nic/hal/plugins/cfg/lif/lif.hpp"

namespace {

using namespace std;
using namespace hal::svc;
using intf::LifSpec;
using intf::LifResponse;

// INTERFACESPEC_REACTOR_TEST(if_svcTest, if_svc);

class InterfaceSpecService : public delphi::Service, public enable_shared_from_this<InterfaceSpecService> {
public:
    InterfaceSpecService(delphi::SdkPtr sk) {
        this->sdk_ = sk;
        delphi::objects::InterfaceSpec::Mount(sdk_, delphi::ReadWriteMode);
        if_svc_ = make_shared<if_svc>(sdk_);
        delphi::objects::InterfaceSpec::Watch(sdk_, if_svc_);
    }
    void OnMountComplete() {
        vector<delphi::objects::InterfaceSpecPtr> olist = delphi::objects::InterfaceSpec::List(sdk_);
        for (vector<delphi::objects::InterfaceSpecPtr>::iterator obj=olist.begin(); obj!=olist.end(); ++obj) {
            if_svc_->OnInterfaceSpecCreate(*obj);
        }
    }

    std::shared_ptr<if_svc>    if_svc_;
    delphi::SdkPtr             sdk_;
};

class if_svcTest : public hal_base_test {
protected:
    pthread_t ev_thread_id = 0;
    delphi::SdkPtr sdk_; \
    delphi::shm::DelphiShmPtr srv_shm_;
    shared_ptr<InterfaceSpecService> exsvc_;
public:
    virtual void SetUp() {
        // setup HAL stuff
        hal_base_test::SetUp();

        // setup delphi
        sdk_ = make_shared<delphi::Sdk>();
        srv_shm_ = make_shared<delphi::shm::DelphiShm>();
        delphi::error err = srv_shm_->MemMap(DELPHI_SHM_NAME, DELPHI_SHM_SIZE, true);
        assert(err.IsOK());
        exsvc_ = make_shared<InterfaceSpecService>(sdk_);
        assert(exsvc_ != NULL);
        sdk_->RegisterService(exsvc_);
        pthread_create(&ev_thread_id, 0, &StartTestLoop, (void*)&sdk_);
        usleep(10 * 1000);
    }
    virtual void TearDown() {
        sdk_->Stop();
        srv_shm_->MemUnmap();
        pthread_cancel(ev_thread_id);
        pthread_join(ev_thread_id, NULL);
        usleep(1000);
    }
    static void SetUpTestCase() {
      hal_base_test::SetUpTestCase();
      hal_test_utils_slab_disable_delete();
    }
};

TEST_F(if_svcTest, InterfaceSpecTest) {
    LifSpec        lif_spec;
    LifResponse    lif_rsp;

    // create the lif
    lif_spec.mutable_key_or_handle()->set_lif_id(1);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    hal_ret_t ret = hal::lif_create(lif_spec, &lif_rsp, NULL);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);

    // port spec parameters
    delphi::objects::InterfaceSpecPtr spec = std::make_shared<delphi::objects::InterfaceSpec>();
    spec->set_type(intf::IF_TYPE_ENIC);
    spec->mutable_if_enic_info()->mutable_lif_key_or_handle()->set_lif_id(1);
    spec->mutable_key_or_handle()->set_interface_id(IF_ID_OFFSET + 1);
    spec->mutable_if_enic_info()->set_enic_type(intf::IF_ENIC_TYPE_USEG);
    spec->mutable_if_enic_info()->mutable_enic_info()->mutable_l2segment_key_handle()->set_segment_id(1);
    spec->mutable_if_enic_info()->mutable_enic_info()->set_encap_vlan_id(20);

    // create interface spec
    sdk_->QueueUpdate(spec);

    // verify port spec got created
    ASSERT_EQ_EVENTUALLY(sdk_->ListKind("InterfaceSpec").size(), (uint32_t)1) << "Interface spec object was not created";
    ASSERT_EQ_EVENTUALLY(delphi::objects::InterfaceSpec::List(sdk_)[0]->type(), intf::IF_TYPE_ENIC) << "invalid intf type";
    ASSERT_EQ_EVENTUALLY(delphi::objects::InterfaceSpec::List(sdk_)[0]->if_enic_info().enic_info().encap_vlan_id(), (uint32_t)20) << "invalid encap vlan";

    // update the encap vlan
    spec->mutable_if_enic_info()->mutable_enic_info()->set_encap_vlan_id(30);
    sdk_->QueueUpdate(spec);

    // verify the objetc got updated
    ASSERT_EQ_EVENTUALLY(delphi::objects::InterfaceSpec::List(sdk_)[0]->if_enic_info().enic_info().encap_vlan_id(), (uint32_t)30) << "invalid encap vlan";

    // delete the interface spec
    sdk_->QueueDelete(spec);

    // verify object was removed
    ASSERT_EQ_EVENTUALLY(sdk_->ListKind("InterfaceSpec").size(), (uint32_t)0) << "Interface spec object was not deleted";
}

} // namespace

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
