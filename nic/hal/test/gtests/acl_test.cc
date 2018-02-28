#include "nic/include/base.h"
#include "nic/hal/src/acl.hpp"
#include "nic/gen/proto/hal/types.pb.h"
#include "nic/gen/proto/hal/acl.pb.h"
#include "nic/hal/hal.hpp"
#include <gtest/gtest.h>
#include <stdio.h>
#include <stdlib.h>
#include "nic/p4/nw/include/defines.h"
#include "nic/hal/test/utils/hal_base_test.hpp"
#include <google/protobuf/util/message_differencer.h>
#include <google/protobuf/text_format.h>
#include "nic/hal/src/qos.hpp"
#include "nic/gen/proto/hal/qos.pb.h"

using google::protobuf::util::MessageDifferencer;
using acl::AclSpec;
using acl::AclResponse;
using kh::AclKeyHandle;
using acl::AclSelector;
using acl::AclActionInfo;
using acl::AclGetRequest;
using acl::AclGetResponse;
using acl::AclDeleteRequest;
using acl::AclDeleteResponse;

class acl_test : public hal_base_test {
protected:
  acl_test() {
  }

  virtual ~acl_test() {
  }

  // will be called immediately after the constructor before each test
  virtual void SetUp() {
  }

  // will be called immediately after each test before the destructor
  virtual void TearDown() {
  }

};

// Creating acls
TEST_F(acl_test, test1)
{
    hal_ret_t     ret;
    AclSpec       spec;
    AclResponse   rsp;

    AclSelector   *match;
    AclActionInfo *action;

    match = spec.mutable_match();
    match->mutable_ip_selector()->set_ip_af(types::IPAddressFamily::IP_AF_INET);
    match->mutable_ip_selector()->mutable_src_prefix()->mutable_address()->set_ip_af(types::IPAddressFamily::IP_AF_INET);
    match->mutable_ip_selector()->mutable_src_prefix()->mutable_address()->set_v4_addr(1);
    match->mutable_ip_selector()->mutable_src_prefix()->set_prefix_len(32);

    action = spec.mutable_action();
    action->set_action(acl::AclAction::ACL_ACTION_DENY);

    spec.mutable_key_or_handle()->set_acl_id(1000);
    spec.set_priority(1000);

    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::acl_create(spec, &rsp);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);
    spec.Clear();
}

TEST_F(acl_test, test2)
{
    hal_ret_t     ret;
    AclSpec       spec;
    AclResponse   rsp;

    AclSelector   *match; 
    AclActionInfo *action;

    for (int i = 100; i < 120; i++) {
        match = spec.mutable_match();
        match->mutable_ip_selector()->set_ip_af(types::IPAddressFamily::IP_AF_INET);
        match->mutable_ip_selector()->mutable_src_prefix()->mutable_address()->set_ip_af(types::IPAddressFamily::IP_AF_INET);
        match->mutable_ip_selector()->mutable_src_prefix()->mutable_address()->set_v4_addr(1);
        match->mutable_ip_selector()->mutable_src_prefix()->set_prefix_len(32);

        action = spec.mutable_action();
        action->set_action(acl::AclAction::ACL_ACTION_DENY);

        spec.mutable_key_or_handle()->set_acl_id(i);
        spec.set_priority(i);

        hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
        ret = hal::acl_create(spec, &rsp);
        hal::hal_cfg_db_close();
        ASSERT_TRUE(ret == HAL_RET_OK);
        spec.Clear();
    }
}

// Create and Delete ACLs
TEST_F(acl_test, test3)
{
    hal_ret_t     ret;
    AclSpec       spec;
    AclResponse   rsp;
    AclDeleteRequest del_req;
    AclDeleteResponse del_rsp;

    AclSelector   *match; 
    AclActionInfo *action;

    std::vector<hal_handle_t> entries;

    for (int i = 200; i < 210; i++) {
        match = spec.mutable_match();
        match->mutable_ip_selector()->set_ip_af(types::IPAddressFamily::IP_AF_INET6);
        match->mutable_ip_selector()->mutable_src_prefix()->mutable_address()->set_ip_af(types::IPAddressFamily::IP_AF_INET6);
        match->mutable_ip_selector()->mutable_src_prefix()->mutable_address()->set_v6_addr("00010001000100010001000100010001");
        match->mutable_ip_selector()->mutable_src_prefix()->set_prefix_len(64);

        action = spec.mutable_action();
        action->set_action(acl::AclAction::ACL_ACTION_DENY);

        spec.mutable_key_or_handle()->set_acl_id(i);
        spec.set_priority(i);

        hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
        ret = hal::acl_create(spec, &rsp);
        hal::hal_cfg_db_close();
        ASSERT_TRUE(ret == HAL_RET_OK);

        entries.push_back(rsp.status().acl_handle());
        spec.Clear();
        rsp.Clear();
    }

    for (auto &entry : entries) {
        del_req.mutable_key_or_handle()->set_acl_handle(entry);
        hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
        ret = hal::acl_delete(del_req, &del_rsp);
        hal::hal_cfg_db_close();
        ASSERT_EQ(ret, HAL_RET_OK);
        ASSERT_EQ(del_rsp.api_status(), types::API_STATUS_OK);
        del_req.Clear();
        del_rsp.Clear();
    }

}

// Create, Update and Delete ACLs
TEST_F(acl_test, test4)
{
    hal_ret_t     ret;
    AclSpec       spec;
    AclResponse   rsp;
    AclGetRequest get_req;
    AclGetResponseMsg get_rsp_msg;
    AclDeleteRequest del_req;
    AclDeleteResponse del_rsp;

    AclSelector   *match; 
    AclActionInfo *action;
    hal_handle_t acl_handle;

    std::vector<std::pair<hal_handle_t,uint32_t>> entries;

    for (int i = 300; i < 310; i++) {
        spec.Clear();
        rsp.Clear();

        match = spec.mutable_match();
        match->mutable_ip_selector()->set_ip_af(types::IPAddressFamily::IP_AF_INET6);
        match->mutable_ip_selector()->mutable_src_prefix()->mutable_address()->set_ip_af(types::IPAddressFamily::IP_AF_INET6);
        match->mutable_ip_selector()->mutable_src_prefix()->mutable_address()->set_v6_addr(std::string("\x00\x01\x00\x01\x00\x01\x00\x01\x00\x01\x00\x01\x00\x01\x00\x01", 16));
        match->mutable_ip_selector()->mutable_src_prefix()->set_prefix_len(35);

        action = spec.mutable_action();
        action->set_action(acl::AclAction::ACL_ACTION_DENY);

        spec.mutable_key_or_handle()->set_acl_id(i);
        spec.set_priority(i);

        hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
        ret = hal::acl_create(spec, &rsp);
        hal::hal_cfg_db_close();
        ASSERT_EQ(ret, HAL_RET_OK);
        ASSERT_EQ(rsp.api_status(), types::API_STATUS_OK);

        acl_handle = rsp.status().acl_handle();
        entries.push_back(std::make_pair(acl_handle, i));

        // Get
        get_req.Clear();
        get_rsp_msg.Clear();

        get_req.mutable_key_or_handle()->set_acl_handle(acl_handle);
        hal::hal_cfg_db_open(hal::CFG_OP_READ);
        ret = hal::acl_get(get_req, &get_rsp_msg);
        hal::hal_cfg_db_close();
        ASSERT_EQ(ret, HAL_RET_OK);

        ASSERT_TRUE(get_rsp_msg.response(0).spec().priority() == spec.priority());
        if (!MessageDifferencer::Equivalent(get_rsp_msg.response(0).spec().match(), spec.match())) {
            std::string str;
            google::protobuf::TextFormat::PrintToString(get_rsp_msg.response(0).spec().match(), &str);
            std::cout << str << std::endl;
            google::protobuf::TextFormat::PrintToString(spec.match(), &str);
            std::cout << str << std::endl;
            ASSERT_TRUE(MessageDifferencer::Equivalent(get_rsp_msg.response(0).spec().match(), spec.match()));
        }
        if (!MessageDifferencer::Equivalent(get_rsp_msg.response(0).spec().action(), spec.action())) {
            std::string str;
            google::protobuf::TextFormat::PrintToString(get_rsp_msg.response(0).spec().action(), &str);
            std::cout << str << std::endl;
            google::protobuf::TextFormat::PrintToString(spec.action(), &str);
            std::cout << str << std::endl;
            ASSERT_TRUE(MessageDifferencer::Equivalent(get_rsp_msg.response(0).spec().action(), spec.action()));
        }
    }


    CoppGetRequest copp_get_req;
    CoppGetResponse copp_get_rsp;
    CoppGetResponseMsg copp_resp_msg;

    copp_get_req.Clear();
    copp_get_rsp.Clear();
    copp_resp_msg.Clear();

    copp_get_req.mutable_key_or_handle()->set_copp_type(kh::COPP_TYPE_FLOW_MISS);
    hal::hal_cfg_db_open(hal::CFG_OP_READ);
    ret = hal::copp_get(copp_get_req, &copp_resp_msg);
    copp_get_rsp = copp_resp_msg.response(0);
    hal::hal_cfg_db_close();
    ASSERT_EQ(ret, HAL_RET_OK);
    uint64_t copp_handle = copp_get_rsp.status().copp_handle();

    bool change_prio = true;
    for (auto &entry : entries) {
        spec.Clear();
        rsp.Clear();

        match = spec.mutable_match();
        match->mutable_ip_selector()->set_ip_af(types::IPAddressFamily::IP_AF_INET);
        match->mutable_ip_selector()->mutable_dst_prefix()->mutable_address()->set_ip_af(types::IPAddressFamily::IP_AF_INET);
        match->mutable_ip_selector()->mutable_dst_prefix()->mutable_address()->set_v4_addr(0x12345678);
        match->mutable_ip_selector()->mutable_dst_prefix()->set_prefix_len(20);

        action = spec.mutable_action();
        action->set_action(acl::AclAction::ACL_ACTION_LOG);
        action->mutable_copp_key_handle()->set_copp_handle(copp_handle);

        acl_handle = entry.first;
        spec.mutable_key_or_handle()->set_acl_handle(acl_handle);
        if (change_prio) {
            spec.set_priority(entry.second + 1);
        } else {
            spec.set_priority(entry.second);
        }

        hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
        ret = hal::acl_update(spec, &rsp);
        hal::hal_cfg_db_close();
        if (change_prio) {
            // Changing of priority is not allowed
            ASSERT_NE(ret, HAL_RET_OK);
            ASSERT_NE(rsp.api_status(), types::API_STATUS_OK);
        } else {
            ASSERT_EQ(ret, HAL_RET_OK);
            ASSERT_EQ(rsp.api_status(), types::API_STATUS_OK);
        }

        // Get
        get_req.Clear();
        get_rsp_msg.Clear();

        get_req.mutable_key_or_handle()->set_acl_handle(acl_handle);
        hal::hal_cfg_db_open(hal::CFG_OP_READ);
        ret = hal::acl_get(get_req, &get_rsp_msg);
        hal::hal_cfg_db_close();
        ASSERT_EQ(ret, HAL_RET_OK);

        if (change_prio) {
            // Request didn't succeed. So it should match original spec
        } else {
            ASSERT_TRUE(get_rsp_msg.response(0).spec().priority() == spec.priority());
            if (!MessageDifferencer::Equivalent(get_rsp_msg.response(0).spec().match(), spec.match())) {
                std::string str;
                google::protobuf::TextFormat::PrintToString(get_rsp_msg.response(0).spec().match(), &str);
                std::cout << str << std::endl;
                google::protobuf::TextFormat::PrintToString(spec.match(), &str);
                std::cout << str << std::endl;
                ASSERT_TRUE(MessageDifferencer::Equivalent(get_rsp_msg.response(0).spec().match(), spec.match()));
            }
            if (!MessageDifferencer::Equivalent(get_rsp_msg.response(0).spec().action(), spec.action())) {
                std::string str;
                google::protobuf::TextFormat::PrintToString(get_rsp_msg.response(0).spec().action(), &str);
                std::cout << str << std::endl;
                google::protobuf::TextFormat::PrintToString(spec.action(), &str);
                std::cout << str << std::endl;
                ASSERT_TRUE(MessageDifferencer::Equivalent(get_rsp_msg.response(0).spec().action(), spec.action()));
            }
        }
        change_prio = false;
    }

    for (auto &entry : entries) {
        del_req.mutable_key_or_handle()->set_acl_handle(entry.first);
        hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
        ret = hal::acl_delete(del_req, &del_rsp);
        hal::hal_cfg_db_close();
        ASSERT_EQ(ret, HAL_RET_OK);
        ASSERT_EQ(del_rsp.api_status(), types::API_STATUS_OK);
        del_req.Clear();
        del_rsp.Clear();
    }
}


int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
