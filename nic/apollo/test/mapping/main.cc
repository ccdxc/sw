//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// This file contains the mapping test cases
///
//----------------------------------------------------------------------------

#include "nic/sdk/include/sdk/eth.hpp"
#include "nic/sdk/include/sdk/ip.hpp"
#include "nic/apollo/api/include/pds_subnet.hpp"
#include "nic/apollo/api/include/pds_vnic.hpp"
#include "nic/apollo/test/utils/api_base.hpp"
#include "nic/apollo/test/utils/base.hpp"
#include "nic/apollo/test/utils/batch.hpp"
#include "nic/apollo/test/utils/device.hpp"
#include "nic/apollo/test/utils/local_mapping.hpp"
#include "nic/apollo/test/utils/remote_mapping.hpp"
#include "nic/apollo/test/utils/route.hpp"
#include "nic/apollo/test/utils/subnet.hpp"
#include "nic/apollo/test/utils/tep.hpp"
#include "nic/apollo/test/utils/utils.hpp"
#include "nic/apollo/test/utils/vnic.hpp"
#include "nic/apollo/test/utils/vpc.hpp"
#include "nic/apollo/test/utils/workflow.hpp"

namespace api_test {
/// \cond
// Globals
static pds_epoch_t g_batch_epoch = PDS_EPOCH_INVALID;

// Config for VPC 1
uint32_t g_vpc_id = 1;
uint32_t g_subnet_id = 1;
uint16_t g_vnic_id = 1;
uint32_t g_encap_val = 1;
uint64_t g_base_vnic_mac = 0x000000030b020a01;
uint32_t g_mytep_id = 1;
uint32_t g_tep_id = g_mytep_id + 1;
std::string g_vpc_cidr_v4("5.5.0.0/16");
std::string g_vnic_cidr_v4("5.5.0.2/16");
std::string g_subnet_cidr_v4("5.5.0.0/16");
std::string g_public_ip_v4("192.168.0.2/16");
std::string g_tep_cidr_v4("1.0.0.1");
std::string g_rt_cidr_v4("1.0.0.1");
std::string vnic_test_ip = "5.5.0.0";
std::string tep_test_ip = "1.0.0.1";
std::string remote_test_ip = "5.5.0.10";
uint64_t g_remote_vnic_mac = 0x000000030b020a02;
std::string g_vpc_rmac("00:03:01:00:00:01");
std::string g_subnet_rmac("00:04:01:00:00:01");

//----------------------------------------------------------------------------
// Mapping test class
//----------------------------------------------------------------------------

class mapping_test : public pds_test_base {
protected:
    mapping_test() {}
    virtual ~mapping_test() {}
    virtual void SetUp() {}
    virtual void TearDown() {}
    static void SetUpTestCase() {
        if (!agent_mode())
            pds_test_base::SetUpTestCase(g_tc_params);
        uint16_t vpc_id = api_test::g_vpc_id;
        uint16_t vnic_stepper = api_test::g_vpc_id;
        uint32_t num_vnics = k_max_vnic;
        uint32_t rt_id_v4 = api_test::g_subnet_id;
        uint32_t rt_id_v6 = api_test::g_subnet_id + 1024;
        uint32_t num_teps = PDS_MAX_TEP - 1;
        uint64_t vnic_stepper_mac = api_test::g_base_vnic_mac;
        pds_vpc_key_t vpc_key = {0};
        pds_subnet_key_t subnet_key = {0};
        pds_vnic_info_t vnic_info = {0};
        pds_vpc_info_t vpc_info = {0};
        pds_subnet_info_t sub_info = {0};
        pds_batch_params_t batch_params = {0};
        std::string subnet_cidr = api_test::g_subnet_cidr_v4;
        std::string nr_cidr = "100.0.0.1/16";
        ip_prefix_t ip_pfx, rt_pfx, nr_pfx;
        ip_addr_t ipaddr, rt_addr, nr_addr;

        vpc_key.id = api_test::g_vpc_id;
        subnet_key.id = api_test::g_subnet_id;
        extract_ip_pfx((char *)subnet_cidr.c_str(), &ip_pfx);
        extract_ip_pfx((char *)api_test::g_rt_cidr_v4.c_str(), &rt_pfx);
        extract_ip_pfx((char *)nr_cidr.c_str(), &nr_pfx);

        rt_addr = rt_pfx.addr;

        pds_batch_ctxt_t bctxt = batch_start();
        sample_device_setup(bctxt);

        vpc_feeder vpc_feeder;
        vpc_feeder.init(vpc_key, PDS_VPC_TYPE_TENANT, g_vpc_cidr_v4,
                        g_vpc_rmac, PDS_MAX_VPC);
        many_create(bctxt, vpc_feeder);

        // sample_tep_setup(g_mytep_id, k_device_ip, 1);
        sample_tep_setup(bctxt, g_tep_id, api_test::g_tep_cidr_v4, num_teps);
        for (uint16_t idx = 0; idx < num_teps; idx++) {
            sample_route_table_setup(
                bctxt, nr_pfx, rt_addr, IP_AF_IPV4, 1, 1, rt_id_v4+idx);
            ip_prefix_ip_next(&rt_pfx, &rt_addr);
            rt_pfx.addr = rt_addr;

            ip_prefix_ip_next(&nr_pfx, &nr_addr);
            nr_pfx.addr = nr_addr;
        }

        subnet_feeder subnet_feeder;
        subnet_key.id = api_test::g_subnet_id;
        vpc_key.id = api_test::g_vpc_id;
        for (uint16_t idx = 0; idx < PDS_MAX_VPC; idx++) {
            subnet_feeder.init(subnet_key, vpc_key, subnet_cidr,
                               g_subnet_rmac, 1);
            many_create(bctxt, subnet_feeder);
            subnet_key.id += 1;
            vpc_key.id += 1;
            ip_prefix_ip_next(&ip_pfx, &ipaddr);
            ip_pfx.addr = ipaddr;
            subnet_cidr = ippfx2str(&ip_pfx);
        }

        vnic_feeder vnic_feeder;
        vnic_feeder.init(1, num_vnics, vnic_stepper_mac);
        many_create(bctxt, vnic_feeder);
        batch_commit(bctxt);

        vpc_key.id = api_test::g_vpc_id;
        vpc_feeder.init(vpc_key, PDS_VPC_TYPE_TENANT, g_vpc_cidr_v4,
                        g_vpc_rmac, PDS_MAX_VPC);
        many_read(vpc_feeder);

        subnet_key.id = api_test::g_subnet_id;
        for (uint16_t idx = 0; idx < PDS_MAX_VPC; idx++) {
            subnet_feeder.init(subnet_key, vpc_key, subnet_cidr,
                               g_subnet_rmac, 1);
            many_read(subnet_feeder);
            subnet_key.id += 1;
            vpc_key.id += 1;
        }

        sample_device_setup_validate();
        // sample_tep_validate(g_mytep_id, k_device_ip, 1);
        sample_tep_validate(g_tep_id, api_test::g_tep_cidr_v4, num_teps);
        many_read(vnic_feeder);
    }
    static void TearDownTestCase() {
#if 0
        uint16_t vpc_id = api_test::g_vpc_id;
        uint16_t vnic_stepper = api_test::g_vpc_id;
        uint32_t num_vnics = k_max_vnic;
        uint32_t rt_id_v4 = api_test::g_subnet_id;
        uint32_t rt_id_v6 = api_test::g_subnet_id + 1024;
        uint32_t num_teps = PDS_MAX_TEP - 1;
        uint64_t vnic_stepper_mac = api_test::g_base_vnic_mac;
        std::string subnet_cidr = api_test::g_subnet_cidr_v4;
        std::string rt_ip = api_test::g_rt_cidr_v4;
        std::string nr_cidr = "100.0.0.1/16";
        ip_prefix_t ip_pfx, rt_pfx, nr_pfx;
        ip_addr_t ipaddr, rt_addr, nr_addr;
        device_stepper_seed_t device_seed;
        pds_vpc_key_t vpc_key = {0};
        pds_subnet_key_t subnet_key = {0};
        vnic_stepper_seed_t seed;
        pds_batch_params_t batch_params = {0};
        pds_encap_t encap = {PDS_ENCAP_TYPE_MPLSoUDP, 0};

        DEVICE_SEED_INIT(&device_seed, g_device_ip, g_device_macaddr, g_gateway_ip);

        vpc_key.id = api_test::g_vpc_id;
        subnet_key.id = api_test::g_subnet_id;
        extract_ip_pfx((char *)subnet_cidr.c_str(), &ip_pfx);
        extract_ip_pfx((char *)api_test::g_rt_cidr_v4.c_str(), &rt_pfx);
        extract_ip_pfx((char *)nr_cidr.c_str(), &nr_pfx);

        tep_util tep_obj(api_test::g_device_ip, PDS_TEP_TYPE_WORKLOAD, encap);

        batch_params.epoch = ++api_test::g_batch_epoch;
        ASSERT_TRUE(pds_batch_start(&batch_params) == sdk::SDK_RET_OK);
        ASSERT_TRUE(vnic_util::many_delete(&seed, num_vnics) ==
                    sdk::SDK_RET_OK);
        for (uint16_t idx = 0; idx < num_teps; idx++) {
            route_table_util rt_obj(rt_id_v4 + idx, nr_cidr, rt_ip, IP_AF_IPV4,
                                    1);
            ip_prefix_ip_next(&rt_pfx, &rt_addr);
            rt_pfx.addr = rt_addr;
            rt_ip = ipv4addr2str(rt_addr.addr.v4_addr);

            ip_prefix_ip_next(&nr_pfx, &nr_addr);
            nr_pfx.addr = nr_addr;
            nr_cidr = ippfx2str(&nr_pfx);

            ASSERT_TRUE(rt_obj.del() == sdk::SDK_RET_OK);
        }
        ASSERT_TRUE(tep_util::many_delete(num_teps, api_test::g_tep_cidr_v4,
                                          PDS_TEP_TYPE_WORKLOAD,
                                          encap) == sdk::SDK_RET_OK);
        ASSERT_TRUE(tep_obj.del() == sdk::SDK_RET_OK);
        subnet_key.id = api_test::g_subnet_id;
        vpc_key.id = api_test::g_vpc_id;
        for (uint16_t idx = 0; idx < PDS_MAX_VPC; idx++) {
            subnet_util sub_obj(vpc_key.id, subnet_key.id, subnet_cidr,
                                rt_id_v4++, rt_id_v6++);
            ASSERT_TRUE(sub_obj.del() == sdk::SDK_RET_OK);
            subnet_key.id += 1;
            vpc_key.id += 1;
            ip_prefix_ip_next(&ip_pfx, &ipaddr);
            ip_pfx.addr = ipaddr;
            subnet_cidr = ippfx2str(&ip_pfx);
        }
        ASSERT_TRUE(vpc_util::many_delete(vpc_key, PDS_MAX_VPC) ==
                    sdk::SDK_RET_OK);
        DEVICE_DELETE(&device_seed);
        ASSERT_TRUE(pds_batch_commit() == sdk::SDK_RET_OK);
#endif
        if (!agent_mode())
            pds_test_base::TearDownTestCase();
    }
};
/// \endcond
//----------------------------------------------------------------------------
// Mapping test cases implementation
//----------------------------------------------------------------------------

/// \defgroup MAPPING_TEST Mapping tests
/// @{

// --------------------------- IPv4 MAPPINGS -----------------------

static void
remote_mapping_stepper_seed_init (remote_mapping_stepper_seed_t *seed,
                                  uint32_t base_encap_val, uint64_t base_mac_64,
                                  std::string vnic_ip_cidr,
                                  std::string tep_ip_cidr)
{
    seed->encap_val_stepper = base_encap_val;
    seed->vnic_mac_stepper = base_mac_64;
    seed->vnic_ip_stepper = vnic_ip_cidr;
    seed->tep_ip_stepper = tep_ip_cidr;
}

static void
step_up_ip (ip_prefix_t *pfx, uint16_t num_steps)
{
    if (pfx->addr.af == IP_AF_IPV4) {
        pfx->addr.addr.v4_addr += num_steps;
    }
}

static void
step_up_ip_next (ip_prefix_t *pfx, uint16_t num_steps)
{
    ip_addr_t ipaddr_next;
    for (uint8_t i = 0; i < num_steps; i++) {
        ip_prefix_ip_next(pfx, &ipaddr_next);
        pfx->addr = ipaddr_next;
    }
}

// --------------------------- LOCAL MAPPINGS -----------------------

/// \brief local mappings WF_1
/// \ref WF_1
TEST_F(mapping_test, DISABLED_v4_local_mapping_workflow_1) {
    local_mapping_stepper_seed_t seed = {0};

    LOCAL_MAPPING_SEED_INIT(&seed, g_vpc_id, g_subnet_id, g_vnic_id,
                            PDS_ENCAP_TYPE_MPLSoUDP, g_encap_val,
                            g_base_vnic_mac, g_vnic_cidr_v4.c_str(), true,
                            g_public_ip_v4.c_str());
    seed.num_vnics = k_max_vnic;
    seed.num_ip_per_vnic = PDS_MAX_VNIC_IP;
    workflow_1<local_mapping_util, local_mapping_stepper_seed_t>(&seed);
}

/// \brief local mappings WF_2
/// \ref WF_2
TEST_F(mapping_test, DISABLED_v4_local_mapping_workflow_2) {
    local_mapping_stepper_seed_t seed = {0};

    LOCAL_MAPPING_SEED_INIT(&seed, g_vpc_id, g_subnet_id, g_vnic_id,
                            PDS_ENCAP_TYPE_MPLSoUDP, g_encap_val,
                            g_base_vnic_mac, g_vnic_cidr_v4.c_str(), true,
                            g_public_ip_v4.c_str());
    seed.num_vnics = k_max_vnic;
    seed.num_ip_per_vnic = PDS_MAX_VNIC_IP;
    workflow_2<local_mapping_util, local_mapping_stepper_seed_t>(&seed);

}

/// \brief local mappings WF_3
/// \ref WF_3
TEST_F(mapping_test, DISABLED_local_mapping_workflow_3) {
    uint32_t num_ip_per_vnic = PDS_MAX_VNIC_IP;
    uint32_t num_vnics_per_set = 341;
    uint32_t ip_stepper = num_ip_per_vnic * num_vnics_per_set;
    std::string vnic_ip_cidr = "0.0.0.0/8";
    std::string pub_ip_cidr = "0.0.0.0/8";
    ip_prefix_t vip_pfx;
    ip_prefix_t pip_pfx;
    local_mapping_stepper_seed_t s1;
    local_mapping_stepper_seed_t s2;
    local_mapping_stepper_seed_t s3;

    LOCAL_MAPPING_SEED_INIT(&s1, g_vpc_id, g_subnet_id, g_vnic_id,
                            PDS_ENCAP_TYPE_MPLSoUDP, g_encap_val,
                            g_base_vnic_mac, g_vnic_cidr_v4.c_str(), true,
                            g_public_ip_v4.c_str());
    s1.num_vnics = num_vnics_per_set;
    s1.num_ip_per_vnic = num_ip_per_vnic;

    str2ipv4pfx((char *)g_vnic_cidr_v4.c_str(), &vip_pfx);
    step_up_ip(&vip_pfx, ip_stepper);
    vnic_ip_cidr = ippfx2str(&vip_pfx);
    str2ipv4pfx((char *)g_public_ip_v4.c_str(), &pip_pfx);
    step_up_ip(&pip_pfx, ip_stepper);
    pub_ip_cidr = ippfx2str(&pip_pfx);

    LOCAL_MAPPING_SEED_INIT(&s2, g_vpc_id, g_subnet_id,
                            g_vnic_id + num_vnics_per_set,
                            PDS_ENCAP_TYPE_MPLSoUDP,
                            g_encap_val + num_vnics_per_set,
                            g_base_vnic_mac + num_vnics_per_set,
                            vnic_ip_cidr.c_str(), true, pub_ip_cidr.c_str());
    s2.num_vnics = num_vnics_per_set;
    s2.num_ip_per_vnic = num_ip_per_vnic;

    step_up_ip(&vip_pfx, ip_stepper);
    step_up_ip(&pip_pfx, ip_stepper);
    LOCAL_MAPPING_SEED_INIT(&s3, g_vpc_id, g_subnet_id,
                            g_vnic_id + (num_vnics_per_set * 2),
                            PDS_ENCAP_TYPE_MPLSoUDP,
                            g_encap_val + (num_vnics_per_set * 2),
                            g_base_vnic_mac + (num_vnics_per_set * 2),
                            ippfx2str(&vip_pfx), true,
                            ippfx2str(&pip_pfx));
    s3.num_vnics = num_vnics_per_set + 1; // 341 + 341 + 342 = 1024
    s3.num_ip_per_vnic = num_ip_per_vnic;
    workflow_3<local_mapping_util, local_mapping_stepper_seed_t>(&s1, &s2, &s3);
}

/// \brief local mappings WF_4
/// \ref WF_4
TEST_F(mapping_test, v4_local_mapping_workflow_4) {
    local_mapping_stepper_seed_t seed = {0};

    LOCAL_MAPPING_SEED_INIT(&seed, g_vpc_id, g_subnet_id, g_vnic_id,
                            PDS_ENCAP_TYPE_MPLSoUDP, g_encap_val,
                            g_base_vnic_mac, g_vnic_cidr_v4.c_str(), true,
                            g_public_ip_v4.c_str());
    seed.num_vnics = k_max_vnic;
    seed.num_ip_per_vnic = PDS_MAX_VNIC_IP;
    workflow_4<local_mapping_util, local_mapping_stepper_seed_t>(&seed);
}

/// \brief local mappings WF_5
/// \ref WF_5
TEST_F(mapping_test, DISABLED_v4_local_mapping_workflow_5) {
    uint32_t num_ip_per_vnic = PDS_MAX_VNIC_IP;
    uint32_t num_vnics_per_set = k_max_vnic/3;
    uint32_t ip_stepper = num_ip_per_vnic * num_vnics_per_set;
    std::string vnic_ip_cidr = "0.0.0.0/8";
    std::string pub_ip_cidr = "0.0.0.0/8";
    ip_prefix_t vip_pfx;
    ip_prefix_t pip_pfx;
    local_mapping_stepper_seed_t s1;
    local_mapping_stepper_seed_t s2;
    local_mapping_stepper_seed_t s3;

    LOCAL_MAPPING_SEED_INIT(&s1, g_vpc_id, g_subnet_id, g_vnic_id,
                            PDS_ENCAP_TYPE_MPLSoUDP, g_encap_val,
                            g_base_vnic_mac, g_vnic_cidr_v4.c_str(), true,
                            g_public_ip_v4.c_str());
    s1.num_vnics = num_vnics_per_set;
    s1.num_ip_per_vnic = num_ip_per_vnic;

    str2ipv4pfx((char *)g_vnic_cidr_v4.c_str(), &vip_pfx);
    step_up_ip(&vip_pfx, ip_stepper);
    vnic_ip_cidr = ippfx2str(&vip_pfx);
    str2ipv4pfx((char *)g_public_ip_v4.c_str(), &pip_pfx);
    step_up_ip(&pip_pfx, ip_stepper);
    pub_ip_cidr = ippfx2str(&pip_pfx);

    LOCAL_MAPPING_SEED_INIT(&s2, g_vpc_id, g_subnet_id,
                            g_vnic_id + num_vnics_per_set,
                            PDS_ENCAP_TYPE_MPLSoUDP,
                            g_encap_val + num_vnics_per_set,
                            g_base_vnic_mac + num_vnics_per_set,
                            vnic_ip_cidr.c_str(), true, pub_ip_cidr.c_str());
    s2.num_vnics = num_vnics_per_set;
    s2.num_ip_per_vnic = num_ip_per_vnic;

    step_up_ip(&vip_pfx, ip_stepper);
    step_up_ip(&pip_pfx, ip_stepper);
    LOCAL_MAPPING_SEED_INIT(&s3, g_vpc_id, g_subnet_id,
                            g_vnic_id + (num_vnics_per_set * 2),
                            PDS_ENCAP_TYPE_MPLSoUDP,
                            g_encap_val + (num_vnics_per_set * 2),
                            g_base_vnic_mac + (num_vnics_per_set * 2),
                            ippfx2str(&vip_pfx), true,
                            ippfx2str(&pip_pfx));
    s3.num_vnics = num_vnics_per_set + 1;
    s3.num_ip_per_vnic = num_ip_per_vnic;
    workflow_5<local_mapping_util, local_mapping_stepper_seed_t>(&s1, &s2, &s3);
}

/// \brief local mappings WF_6
/// \ref WF_6
TEST_F(mapping_test, DISABLED_v4_local_mapping_workflow_6) {
    local_mapping_stepper_seed_t seed1 = {0};
    local_mapping_stepper_seed_t seed1A = {0};
    local_mapping_stepper_seed_t seed1B = {0};

    LOCAL_MAPPING_SEED_INIT(&seed1, g_vpc_id, g_subnet_id, g_vnic_id,
                            PDS_ENCAP_TYPE_MPLSoUDP, g_encap_val,
                            g_base_vnic_mac, g_vnic_cidr_v4.c_str(), true,
                            g_public_ip_v4.c_str());
    seed1.num_vnics = k_max_vnic;
    seed1.num_ip_per_vnic = PDS_MAX_VNIC_IP;

    LOCAL_MAPPING_SEED_INIT(&seed1A, g_vpc_id, g_subnet_id + 10, g_vnic_id,
                            PDS_ENCAP_TYPE_VXLAN, g_encap_val + 1024,
                            g_base_vnic_mac, g_vnic_cidr_v4.c_str(), true,
                            g_public_ip_v4.c_str());
    seed1A.num_vnics = k_max_vnic;
    seed1A.num_ip_per_vnic = PDS_MAX_VNIC_IP;

    LOCAL_MAPPING_SEED_INIT(&seed1B, g_vpc_id, g_subnet_id + 20, g_vnic_id,
                            PDS_ENCAP_TYPE_VXLAN, g_encap_val + 2048,
                            g_base_vnic_mac, g_vnic_cidr_v4.c_str(), false, "");
    seed1B.num_vnics = k_max_vnic;
    seed1B.num_ip_per_vnic = PDS_MAX_VNIC_IP;

    workflow_6<local_mapping_util, local_mapping_stepper_seed_t>(&seed1, &seed1A, &seed1B);
}

/// \brief local mappings WF_7
/// \ref WF_7
TEST_F(mapping_test, DISABLED_v4_local_mapping_workflow_7) {
    local_mapping_stepper_seed_t seed1 = {0};
    local_mapping_stepper_seed_t seed1A = {0};
    local_mapping_stepper_seed_t seed1B = {0};

    LOCAL_MAPPING_SEED_INIT(&seed1, g_vpc_id, g_subnet_id, g_vnic_id,
                            PDS_ENCAP_TYPE_MPLSoUDP, g_encap_val,
                            g_base_vnic_mac, g_vnic_cidr_v4.c_str(), true,
                            g_public_ip_v4.c_str());
    seed1.num_vnics = k_max_vnic;
    seed1.num_ip_per_vnic = PDS_MAX_VNIC_IP;

    LOCAL_MAPPING_SEED_INIT(&seed1A, g_vpc_id, g_subnet_id + 10, g_vnic_id,
                            PDS_ENCAP_TYPE_VXLAN, g_encap_val + 1024,
                            g_base_vnic_mac, g_vnic_cidr_v4.c_str(), true,
                            g_public_ip_v4.c_str());
    seed1A.num_vnics = k_max_vnic;
    seed1A.num_ip_per_vnic = PDS_MAX_VNIC_IP;

    LOCAL_MAPPING_SEED_INIT(&seed1B, g_vpc_id, g_subnet_id + 20, g_vnic_id,
                            PDS_ENCAP_TYPE_VXLAN, g_encap_val + 20,
                            g_base_vnic_mac, g_vnic_cidr_v4.c_str(), false, "");
    seed1B.num_vnics = k_max_vnic;
    seed1B.num_ip_per_vnic = PDS_MAX_VNIC_IP;

    workflow_7<local_mapping_util, local_mapping_stepper_seed_t>(&seed1, &seed1A, &seed1B);
}

/// \brief local mappings WF_8
/// \ref WF_8
TEST_F(mapping_test, DISABLED_v4_local_mapping_workflow_8) {
    local_mapping_stepper_seed_t seed1 = {0};
    local_mapping_stepper_seed_t seed1A = {0};
    local_mapping_stepper_seed_t seed1B = {0};

    LOCAL_MAPPING_SEED_INIT(&seed1, g_vpc_id, g_subnet_id, g_vnic_id,
                            PDS_ENCAP_TYPE_MPLSoUDP, g_encap_val,
                            g_base_vnic_mac, g_vnic_cidr_v4.c_str(), true,
                            g_public_ip_v4.c_str());
    seed1.num_vnics = k_max_vnic;
    seed1.num_ip_per_vnic = PDS_MAX_VNIC_IP;

    LOCAL_MAPPING_SEED_INIT(&seed1A, g_vpc_id, g_subnet_id + 10, g_vnic_id,
                            PDS_ENCAP_TYPE_VXLAN, g_encap_val + 1024,
                            g_base_vnic_mac, g_vnic_cidr_v4.c_str(), true,
                            g_public_ip_v4.c_str());
    seed1A.num_vnics = k_max_vnic;
    seed1A.num_ip_per_vnic = PDS_MAX_VNIC_IP;

    LOCAL_MAPPING_SEED_INIT(&seed1B, g_vpc_id, g_subnet_id + 20, g_vnic_id,
                            PDS_ENCAP_TYPE_VXLAN, g_encap_val + 2048,
                            g_base_vnic_mac, g_vnic_cidr_v4.c_str(), false, "");
    seed1B.num_vnics = k_max_vnic;
    seed1B.num_ip_per_vnic = PDS_MAX_VNIC_IP;

    workflow_8<local_mapping_util, local_mapping_stepper_seed_t>(&seed1, &seed1A, &seed1B);
}

/// \brief local mappings WF_9
/// \ref WF_9
TEST_F(mapping_test, DISABLED_v4_local_mapping_workflow_9) {
    local_mapping_stepper_seed_t seed1 = {0};
    local_mapping_stepper_seed_t seed1A = {0};

    LOCAL_MAPPING_SEED_INIT(&seed1, g_vpc_id, g_subnet_id, g_vnic_id,
                            PDS_ENCAP_TYPE_MPLSoUDP, g_encap_val,
                            g_base_vnic_mac, g_vnic_cidr_v4.c_str(), true,
                            g_public_ip_v4.c_str());
    seed1.num_vnics = k_max_vnic;
    seed1.num_ip_per_vnic = PDS_MAX_VNIC_IP;

    LOCAL_MAPPING_SEED_INIT(&seed1A, g_vpc_id, g_subnet_id, g_vnic_id,
                            PDS_ENCAP_TYPE_MPLSoUDP, g_encap_val + 2048,
                            g_base_vnic_mac + 1024, g_vnic_cidr_v4.c_str(), true,
                            g_public_ip_v4.c_str());
    seed1A.num_vnics = k_max_vnic;
    seed1A.num_ip_per_vnic = PDS_MAX_VNIC_IP;

    workflow_9<local_mapping_util, local_mapping_stepper_seed_t>(&seed1, &seed1A);
}

/// \brief local mappings WF_10
/// \ref WF_10
TEST_F(mapping_test, DISABLED_v4_local_mapping_workflow_10) {
    uint32_t per_set_vnics = 256;
    uint32_t ip_stepper = per_set_vnics * PDS_MAX_VNIC_IP;
    local_mapping_stepper_seed_t seed1 = {0};
    local_mapping_stepper_seed_t seed2 = {0};
    local_mapping_stepper_seed_t seed2A = {0};
    local_mapping_stepper_seed_t seed3 = {0};
    local_mapping_stepper_seed_t seed3A = {0};
    local_mapping_stepper_seed_t seed4 = {0};
    std::string vnic_ip_cidr = "0.0.0.0/8";
    std::string pub_ip_cidr = "0.0.0.0/8";
    ip_prefix_t vip_pfx;
    ip_prefix_t pip_pfx;


    LOCAL_MAPPING_SEED_INIT(&seed1, g_vpc_id, g_subnet_id, g_vnic_id,
                            PDS_ENCAP_TYPE_MPLSoUDP, g_encap_val,
                            g_base_vnic_mac, g_vnic_cidr_v4.c_str(), true,
                            g_public_ip_v4.c_str());
    seed1.num_vnics = per_set_vnics;
    seed1.num_ip_per_vnic = PDS_MAX_VNIC_IP;

    str2ipv4pfx((char *)g_vnic_cidr_v4.c_str(), &vip_pfx);
    step_up_ip(&vip_pfx, ip_stepper);
    vnic_ip_cidr = ippfx2str(&vip_pfx);
    str2ipv4pfx((char *)g_public_ip_v4.c_str(), &pip_pfx);
    step_up_ip(&pip_pfx, ip_stepper);
    pub_ip_cidr = ippfx2str(&pip_pfx);

    LOCAL_MAPPING_SEED_INIT(&seed2, g_vpc_id, g_subnet_id, g_vnic_id + per_set_vnics,
                            PDS_ENCAP_TYPE_MPLSoUDP, g_encap_val + per_set_vnics,
                            g_base_vnic_mac + per_set_vnics, vnic_ip_cidr.c_str(), true,
                            pub_ip_cidr.c_str());
    seed2.num_vnics = per_set_vnics;
    seed2.num_ip_per_vnic = PDS_MAX_VNIC_IP;

    LOCAL_MAPPING_SEED_INIT(&seed2A, g_vpc_id, g_subnet_id + 10, g_vnic_id + per_set_vnics,
                            PDS_ENCAP_TYPE_VXLAN, g_encap_val + per_set_vnics,
                            g_base_vnic_mac + per_set_vnics, vnic_ip_cidr.c_str(), true,
                            pub_ip_cidr.c_str());
    seed2.num_vnics = per_set_vnics;
    seed2.num_ip_per_vnic = PDS_MAX_VNIC_IP;

    step_up_ip(&vip_pfx, ip_stepper);
    vnic_ip_cidr = ippfx2str(&vip_pfx);
    LOCAL_MAPPING_SEED_INIT(&seed3, g_vpc_id, g_subnet_id, g_vnic_id + (per_set_vnics * 2),
                            PDS_ENCAP_TYPE_MPLSoUDP, g_encap_val + (per_set_vnics * 2),
                            g_base_vnic_mac + (per_set_vnics * 2), vnic_ip_cidr.c_str(), false, "");
    seed3.num_vnics = per_set_vnics;
    seed3.num_ip_per_vnic = PDS_MAX_VNIC_IP;

    LOCAL_MAPPING_SEED_INIT(&seed3A, g_vpc_id, g_subnet_id + 20, g_vnic_id + (per_set_vnics * 2),
                            PDS_ENCAP_TYPE_VXLAN, g_encap_val + (per_set_vnics * 2),
                            g_base_vnic_mac + (per_set_vnics * 2), vnic_ip_cidr.c_str(), false, "");
    seed3.num_vnics = per_set_vnics;
    seed3.num_ip_per_vnic = PDS_MAX_VNIC_IP;

    step_up_ip(&vip_pfx, ip_stepper);
    vnic_ip_cidr = ippfx2str(&vip_pfx);
    LOCAL_MAPPING_SEED_INIT(&seed4, g_vpc_id, g_subnet_id, g_vnic_id + (per_set_vnics * 3),
                            PDS_ENCAP_TYPE_MPLSoUDP, g_encap_val + (per_set_vnics * 3),
                            g_base_vnic_mac + (per_set_vnics * 3), vnic_ip_cidr.c_str(), false, "");
    seed4.num_vnics = per_set_vnics;
    seed4.num_ip_per_vnic = PDS_MAX_VNIC_IP;

    workflow_10<local_mapping_util, local_mapping_stepper_seed_t>(&seed1, &seed2, &seed2A,
                                                                  &seed3, &seed3A, &seed4);
}

/// \brief local mappings WF_N_1
/// \ref WF_N_1
TEST_F(mapping_test, DISABLED_v4_local_mapping_workflow_neg_1) {
    local_mapping_stepper_seed_t seed = {0};

    LOCAL_MAPPING_SEED_INIT(&seed, g_vpc_id, g_subnet_id, g_vnic_id,
                            PDS_ENCAP_TYPE_MPLSoUDP, g_encap_val,
                            g_base_vnic_mac, g_vnic_cidr_v4, true,
                            g_public_ip_v4);
    seed.num_vnics = k_max_vnic;
    seed.num_ip_per_vnic = PDS_MAX_VNIC_IP;
    workflow_neg_1<local_mapping_util, local_mapping_stepper_seed_t>(&seed);
}

/// \brief local mappings WF_N_2
/// \ref WF_N_2
TEST_F(mapping_test, DISABLED_v4_local_mapping_workflow_neg_2) {
    local_mapping_stepper_seed_t seed = {0};

    LOCAL_MAPPING_SEED_INIT(&seed, g_vpc_id, g_subnet_id, g_vnic_id,
                            PDS_ENCAP_TYPE_MPLSoUDP, g_encap_val,
                            g_base_vnic_mac, g_vnic_cidr_v4, true,
                            g_public_ip_v4);

    pds_batch_ctxt_t bctxt = batch_start();
    seed.num_vnics = 1;
    seed.num_ip_per_vnic = 1;
    LOCAL_MAPPING_MANY_CREATE(&seed);
    batch_commit(bctxt);

    seed.num_vnics = k_max_vnic;
    seed.num_ip_per_vnic = PDS_MAX_VNIC_IP;
    workflow_neg_2<local_mapping_util, local_mapping_stepper_seed_t>(&seed);
}

/// \brief local mappings WF_N_3
/// \ref WF_N_3
TEST_F(mapping_test, DISABLED_v4_local_mapping_workflow_neg_3) {
    uint32_t vnic_id = 9999;
    uint64_t vnic_mac = 0x0000000040302010;
    std::string vnic_cidr = "100.99.98.97";
    std::string public_ip_cidr = "200.97.98.99";
    local_mapping_stepper_seed_t seed = {0};

    LOCAL_MAPPING_SEED_INIT(&seed, g_vpc_id, g_subnet_id, vnic_id,
                            PDS_ENCAP_TYPE_MPLSoUDP, 4000,
                            vnic_mac, vnic_cidr, true,
                            public_ip_cidr);
    seed.num_vnics = 1;
    seed.num_ip_per_vnic = 1;
    workflow_neg_3<local_mapping_util, local_mapping_stepper_seed_t>(&seed);
}

/// \brief local mappings WF_N_4
/// \ref WF_N_4
TEST_F(mapping_test, DISABLED_v4_local_mapping_workflow_neg_4) {
    uint32_t num_ip_per_vnic = PDS_MAX_VNIC_IP;
    uint32_t num_vnics_per_set = 512;
    uint32_t ip_stepper = num_ip_per_vnic * num_vnics_per_set;
    std::string vnic_ip_cidr = "0.0.0.0/8";
    std::string pub_ip_cidr = "0.0.0.0/8";
    ip_prefix_t vip_pfx;
    ip_prefix_t pip_pfx;
    local_mapping_stepper_seed_t s1;
    local_mapping_stepper_seed_t s2;

    LOCAL_MAPPING_SEED_INIT(&s1, g_vpc_id, g_subnet_id, g_vnic_id,
                            PDS_ENCAP_TYPE_MPLSoUDP, g_encap_val,
                            g_base_vnic_mac, g_vnic_cidr_v4.c_str(), true,
                            g_public_ip_v4.c_str());
    s1.num_vnics = num_vnics_per_set;
    s1.num_ip_per_vnic = num_ip_per_vnic;

    str2ipv4pfx((char *)g_vnic_cidr_v4.c_str(), &vip_pfx);
    step_up_ip(&vip_pfx, ip_stepper);
    vnic_ip_cidr = ippfx2str(&vip_pfx);
    str2ipv4pfx((char *)g_public_ip_v4.c_str(), &pip_pfx);
    step_up_ip(&pip_pfx, ip_stepper);
    pub_ip_cidr = ippfx2str(&pip_pfx);

    LOCAL_MAPPING_SEED_INIT(&s2, g_vpc_id, g_subnet_id,
                            g_vnic_id + num_vnics_per_set,
                            PDS_ENCAP_TYPE_MPLSoUDP,
                            g_encap_val + num_vnics_per_set,
                            g_base_vnic_mac + num_vnics_per_set,
                            vnic_ip_cidr.c_str(), true, pub_ip_cidr.c_str());
    s2.num_vnics = num_vnics_per_set;
    s2.num_ip_per_vnic = num_ip_per_vnic;

    workflow_neg_4<local_mapping_util, local_mapping_stepper_seed_t>(&s1, &s2);
}

/// \brief local mappings WF_N_5
/// \ref WF_N_5
TEST_F(mapping_test, DISABLED_v4_local_mapping_workflow_neg_5) {
    local_mapping_stepper_seed_t seed1 = {0};
    local_mapping_stepper_seed_t seed1A = {0};

    LOCAL_MAPPING_SEED_INIT(&seed1, g_vpc_id, g_subnet_id, g_vnic_id,
                            PDS_ENCAP_TYPE_MPLSoUDP, g_encap_val,
                            g_base_vnic_mac, g_vnic_cidr_v4, true,
                            g_public_ip_v4);
    seed1.num_vnics = k_max_vnic;
    seed1.num_ip_per_vnic = PDS_MAX_VNIC_IP;

    LOCAL_MAPPING_SEED_INIT(&seed1A, g_vpc_id, g_subnet_id, g_vnic_id,
                            PDS_ENCAP_TYPE_MPLSoUDP, g_encap_val + 1024,
                            g_base_vnic_mac + 2048, g_vnic_cidr_v4, false, "");
    seed1A.num_vnics = k_max_vnic;
    seed1A.num_ip_per_vnic = PDS_MAX_VNIC_IP;

    workflow_neg_5<local_mapping_util, local_mapping_stepper_seed_t>(&seed1, &seed1A);
}

/// \brief local mappings WF_N_6
/// \ref WF_N_6
TEST_F(mapping_test, DISABLED_v4_local_mapping_workflow_neg_6) {
    local_mapping_stepper_seed_t seed1 = {0};
    local_mapping_stepper_seed_t seed1A = {0};

    LOCAL_MAPPING_SEED_INIT(&seed1, g_vpc_id, g_subnet_id, g_vnic_id,
                            PDS_ENCAP_TYPE_MPLSoUDP, g_encap_val,
                            g_base_vnic_mac, g_vnic_cidr_v4, true,
                            g_public_ip_v4);
    seed1.num_vnics = k_max_vnic;
    seed1.num_ip_per_vnic = PDS_MAX_VNIC_IP;

    LOCAL_MAPPING_SEED_INIT(&seed1A, g_vpc_id, g_subnet_id, g_vnic_id,
                            PDS_ENCAP_TYPE_MPLSoUDP, g_encap_val + 2048,
                            g_base_vnic_mac + 1024, g_vnic_cidr_v4, false, "");
    seed1A.num_vnics =k_max_vnic + 1;
    seed1A.num_ip_per_vnic = PDS_MAX_VNIC_IP;

    workflow_neg_6<local_mapping_util, local_mapping_stepper_seed_t>(&seed1, &seed1A);
}

/// \brief local mappings WF_N_7
/// \ref WF_N_7
TEST_F(mapping_test, DISABLED_v4_local_mapping_workflow_neg_7) {
    uint32_t num_vnics_per_set = 512;
    uint32_t ip_stepper = num_vnics_per_set * PDS_MAX_VNIC_IP;
    local_mapping_stepper_seed_t seed1 = {0};
    local_mapping_stepper_seed_t seed1A = {0};
    local_mapping_stepper_seed_t seed2 = {0};
    std::string vnic_ip_cidr = "0.0.0.0/8";
    std::string pub_ip_cidr = "0.0.0.0/8";
    ip_prefix_t vip_pfx;
    ip_prefix_t pip_pfx;

    LOCAL_MAPPING_SEED_INIT(&seed1, g_vpc_id, g_subnet_id, g_vnic_id,
                            PDS_ENCAP_TYPE_MPLSoUDP, g_encap_val,
                            g_base_vnic_mac, g_vnic_cidr_v4, true,
                            g_public_ip_v4);
    seed1.num_vnics = num_vnics_per_set;
    seed1.num_ip_per_vnic = PDS_MAX_VNIC_IP;

    LOCAL_MAPPING_SEED_INIT(&seed1A, g_vpc_id, g_subnet_id, g_vnic_id,
                            PDS_ENCAP_TYPE_MPLSoUDP, g_encap_val + 2048,
                            g_base_vnic_mac + 1024, g_vnic_cidr_v4, false, "");
    seed1A.num_vnics = num_vnics_per_set;
    seed1A.num_ip_per_vnic = PDS_MAX_VNIC_IP;

    str2ipv4pfx((char *)g_vnic_cidr_v4.c_str(), &vip_pfx);
    step_up_ip(&vip_pfx, ip_stepper);
    vnic_ip_cidr = ippfx2str(&vip_pfx);
    str2ipv4pfx((char *)g_public_ip_v4.c_str(), &pip_pfx);
    step_up_ip(&pip_pfx, ip_stepper);
    pub_ip_cidr = ippfx2str(&pip_pfx);

    LOCAL_MAPPING_SEED_INIT(&seed2, g_vpc_id, g_subnet_id, g_vnic_id + num_vnics_per_set,
                            PDS_ENCAP_TYPE_MPLSoUDP, g_encap_val + num_vnics_per_set,
                            g_base_vnic_mac + num_vnics_per_set, vnic_ip_cidr, true,
                            pub_ip_cidr);
    seed1.num_vnics = num_vnics_per_set;
    seed1.num_ip_per_vnic = PDS_MAX_VNIC_IP;

    workflow_neg_7<local_mapping_util, local_mapping_stepper_seed_t>(&seed1, &seed1A, &seed2);
}

/// \brief local mappings WF_N_8
/// \ref WF_N_8
TEST_F(mapping_test, DISABLED_v4_local_mapping_workflow_neg_8) {
    uint32_t num_vnics_per_set = 512;
    uint32_t ip_stepper = num_vnics_per_set * PDS_MAX_VNIC_IP;
    local_mapping_stepper_seed_t seed1 = {0};
    local_mapping_stepper_seed_t seed2 = {0};
    std::string vnic_ip_cidr = "0.0.0.0/8";
    std::string pub_ip_cidr = "0.0.0.0/8";
    ip_prefix_t vip_pfx;
    ip_prefix_t pip_pfx;

    LOCAL_MAPPING_SEED_INIT(&seed1, g_vpc_id, g_subnet_id, g_vnic_id,
                            PDS_ENCAP_TYPE_MPLSoUDP, g_encap_val,
                            g_base_vnic_mac, g_vnic_cidr_v4, true,
                            g_public_ip_v4);
    seed1.num_vnics = num_vnics_per_set;
    seed1.num_ip_per_vnic = PDS_MAX_VNIC_IP;

    str2ipv4pfx((char *)g_vnic_cidr_v4.c_str(), &vip_pfx);
    step_up_ip(&vip_pfx, ip_stepper);
    vnic_ip_cidr = ippfx2str(&vip_pfx);
    str2ipv4pfx((char *)g_public_ip_v4.c_str(), &pip_pfx);
    step_up_ip(&pip_pfx, ip_stepper);
    pub_ip_cidr = ippfx2str(&pip_pfx);

    LOCAL_MAPPING_SEED_INIT(&seed2, g_vpc_id, g_subnet_id, g_vnic_id + num_vnics_per_set,
                            PDS_ENCAP_TYPE_MPLSoUDP, g_encap_val + num_vnics_per_set,
                            g_base_vnic_mac + num_vnics_per_set, vnic_ip_cidr, true,
                            pub_ip_cidr);
    seed1.num_vnics = num_vnics_per_set;
    seed1.num_ip_per_vnic = PDS_MAX_VNIC_IP;

    workflow_neg_8<local_mapping_util, local_mapping_stepper_seed_t>(&seed1, &seed2);
}

// --------------------------- END LOCAL MAPPINGS -----------------------

// --------------------------- REMOTE MAPPINGS -----------------------

/// \brief remote mappings WF_1
/// \ref WF_1
TEST_F(mapping_test, DISABLED_v4_remote_mapping_workflow_1) {
    remote_mapping_stepper_seed_t seed = {0};

    REMOTE_MAPPING_SEED_INIT(&seed, g_vpc_id, g_subnet_id, g_vnic_cidr_v4.c_str(),
                            PDS_ENCAP_TYPE_MPLSoUDP, g_encap_val,
                            g_base_vnic_mac, g_tep_id, g_tep_cidr_v4.c_str());
    seed.num_vnics = k_max_vnic;
    seed.num_teps = PDS_MAX_TEP - 1 -1;
    workflow_1<remote_mapping_util, remote_mapping_stepper_seed_t>(&seed);
}

/// \brief remote mappings WF_2
/// \ref WF_2
TEST_F(mapping_test, DISABLED_v4_remote_mapping_workflow_2) {
    remote_mapping_stepper_seed_t seed = {0};

    REMOTE_MAPPING_SEED_INIT(&seed, g_vpc_id, g_subnet_id, g_vnic_cidr_v4.c_str(),
                            PDS_ENCAP_TYPE_MPLSoUDP, g_encap_val,
                            g_base_vnic_mac, g_tep_id, g_tep_cidr_v4.c_str());
    seed.num_vnics = k_max_vnic;
    seed.num_teps = PDS_MAX_TEP - 1 -1;
    workflow_2<remote_mapping_util, remote_mapping_stepper_seed_t>(&seed);
}

/// \brief remote mappings WF_3
/// \ref WF_3
TEST_F(mapping_test, DISABLED_v4_remote_mapping_workflow_3) {
    uint32_t num_vnics_per_set = k_max_vnic;
    uint32_t num_teps_per_set = 341;
    uint32_t stepper = num_teps_per_set * num_vnics_per_set;
    ip_prefix_t vip_pfx;
    ip_prefix_t tip_pfx;
    remote_mapping_stepper_seed_t s1 = {0};
    remote_mapping_stepper_seed_t s2 = {0};
    remote_mapping_stepper_seed_t s3 = {0};

    REMOTE_MAPPING_SEED_INIT(&s1, g_vpc_id, g_subnet_id, g_vnic_cidr_v4.c_str(),
                            PDS_ENCAP_TYPE_MPLSoUDP, g_encap_val,
                            g_base_vnic_mac, g_tep_id, g_tep_cidr_v4.c_str());
    s1.num_vnics = num_vnics_per_set;
    s1.num_teps = num_teps_per_set;

    str2ipv4pfx((char *)g_vnic_cidr_v4.c_str(), &vip_pfx);
    step_up_ip_next(&vip_pfx, num_teps_per_set);
    str2ipv4pfx((char *)g_tep_cidr_v4.c_str(), &tip_pfx);
    step_up_ip_next(&tip_pfx, num_teps_per_set);
    REMOTE_MAPPING_SEED_INIT(&s2, g_vpc_id + num_teps_per_set,
                             g_subnet_id + num_teps_per_set,
                             ippfx2str(&vip_pfx),
                             PDS_ENCAP_TYPE_MPLSoUDP, g_encap_val + stepper,
                             g_base_vnic_mac + stepper,
                             g_tep_id + num_teps_per_set,
                             ippfx2str(&tip_pfx));
    s2.num_vnics = num_vnics_per_set;
    s2.num_teps = num_teps_per_set;

    step_up_ip_next(&vip_pfx, num_teps_per_set);
    step_up_ip_next(&tip_pfx, num_teps_per_set);
    REMOTE_MAPPING_SEED_INIT(&s3, g_vpc_id + (num_teps_per_set * 2),
                             g_subnet_id + (num_teps_per_set * 2),
                             ippfx2str(&vip_pfx),
                             PDS_ENCAP_TYPE_MPLSoUDP, g_encap_val + (stepper * 2),
                             g_base_vnic_mac + (stepper * 2),
                             g_tep_id + (2 * num_teps_per_set),
                             ippfx2str(&tip_pfx));
    s3.num_vnics = num_vnics_per_set;
    s3.num_teps = num_teps_per_set - 1;

    workflow_3<remote_mapping_util, remote_mapping_stepper_seed_t>(&s1, &s2, &s3);
}

/// \brief remote mappings WF_4
/// \ref WF_4
TEST_F(mapping_test, v4_remote_mapping_workflow_4) {
    remote_mapping_stepper_seed_t seed = {0};

    REMOTE_MAPPING_SEED_INIT(&seed, g_vpc_id, g_subnet_id, g_vnic_cidr_v4.c_str(),
                            PDS_ENCAP_TYPE_MPLSoUDP, g_encap_val,
                            g_base_vnic_mac, g_tep_id, g_tep_cidr_v4.c_str());
    seed.num_vnics = k_max_vnic;
    seed.num_teps = PDS_MAX_TEP - 1 - 1;
    workflow_4<remote_mapping_util, remote_mapping_stepper_seed_t>(&seed);
}

/// \brief remote mappings WF_5
/// \ref WF_5
TEST_F(mapping_test, DISABLED_v4_remote_mapping_workflow_5) {
    uint32_t num_vnics_per_set = k_max_vnic;
    uint32_t num_teps_per_set = PDS_MAX_TEP/3;
    uint32_t stepper = num_teps_per_set * num_vnics_per_set;
    ip_prefix_t vip_pfx;
    ip_prefix_t tip_pfx;
    remote_mapping_stepper_seed_t s1 = {0};
    remote_mapping_stepper_seed_t s2 = {0};
    remote_mapping_stepper_seed_t s3 = {0};

    REMOTE_MAPPING_SEED_INIT(&s1, g_vpc_id, g_subnet_id, g_vnic_cidr_v4.c_str(),
                            PDS_ENCAP_TYPE_MPLSoUDP, g_encap_val,
                            g_base_vnic_mac, g_tep_id, g_tep_cidr_v4.c_str());
    s1.num_vnics = num_vnics_per_set;
    s1.num_teps = num_teps_per_set;

    str2ipv4pfx((char *)g_vnic_cidr_v4.c_str(), &vip_pfx);
    step_up_ip_next(&vip_pfx, num_teps_per_set);
    str2ipv4pfx((char *)g_tep_cidr_v4.c_str(), &tip_pfx);
    step_up_ip_next(&tip_pfx, num_teps_per_set);
    REMOTE_MAPPING_SEED_INIT(&s2, g_vpc_id + num_teps_per_set,
                             g_subnet_id + num_teps_per_set,
                             ippfx2str(&vip_pfx),
                             PDS_ENCAP_TYPE_MPLSoUDP, g_encap_val + stepper,
                             g_base_vnic_mac + stepper, 
                             g_tep_id + num_teps_per_set,
                             ippfx2str(&tip_pfx));
    s2.num_vnics = num_vnics_per_set;
    s2.num_teps = num_teps_per_set;

    step_up_ip_next(&vip_pfx, num_teps_per_set);
    step_up_ip_next(&tip_pfx, num_teps_per_set);
    REMOTE_MAPPING_SEED_INIT(&s3, g_vpc_id + (num_teps_per_set * 2),
                             g_subnet_id + (num_teps_per_set * 2),
                             ippfx2str(&vip_pfx),
                             PDS_ENCAP_TYPE_MPLSoUDP, g_encap_val + (stepper * 2),
                             g_base_vnic_mac + (stepper * 2),
                             g_tep_id + (2 * num_teps_per_set),
                             ippfx2str(&tip_pfx));
    s3.num_vnics = num_vnics_per_set;
    s3.num_teps = num_teps_per_set;

    workflow_3<remote_mapping_util, remote_mapping_stepper_seed_t>(&s1, &s2, &s3);
}

/// \brief remote mappings WF_6
/// \ref WF_6
TEST_F(mapping_test, DISABLED_v4_remote_mapping_workflow_6) {
    remote_mapping_stepper_seed_t seed1 = {0};
    remote_mapping_stepper_seed_t seed1A = {0};
    remote_mapping_stepper_seed_t seed1B = {0};

    REMOTE_MAPPING_SEED_INIT(&seed1, g_vpc_id, g_subnet_id, g_vnic_cidr_v4.c_str(),
                            PDS_ENCAP_TYPE_MPLSoUDP, g_encap_val,
                            g_base_vnic_mac, g_tep_id, g_tep_cidr_v4.c_str());
    seed1.num_vnics = k_max_vnic;
    seed1.num_teps = PDS_MAX_TEP - 1 -1;

    REMOTE_MAPPING_SEED_INIT(&seed1A, g_vpc_id, g_subnet_id + 10, g_vnic_cidr_v4.c_str(),
                            PDS_ENCAP_TYPE_MPLSoUDP, g_encap_val + 100,
                            g_base_vnic_mac + 200, g_tep_id, g_tep_cidr_v4.c_str());
    seed1A.num_vnics = k_max_vnic;
    seed1A.num_teps = PDS_MAX_TEP - 1 -1;

    REMOTE_MAPPING_SEED_INIT(&seed1B, g_vpc_id, g_subnet_id, g_vnic_cidr_v4.c_str(),
                            PDS_ENCAP_TYPE_VXLAN, g_encap_val + 200,
                            g_base_vnic_mac, g_tep_id, g_tep_cidr_v4.c_str());
    seed1B.num_vnics = k_max_vnic;
    seed1B.num_teps = PDS_MAX_TEP - 1 -1;

    workflow_6<remote_mapping_util, remote_mapping_stepper_seed_t>(&seed1, &seed1A, &seed1B);
}

/// \brief remote mappings WF_7
/// \ref WF_7
TEST_F(mapping_test, DISABLED_v4_remote_mapping_workflow_7) {
    remote_mapping_stepper_seed_t seed1 = {0};
    remote_mapping_stepper_seed_t seed1A = {0};
    remote_mapping_stepper_seed_t seed1B = {0};

    REMOTE_MAPPING_SEED_INIT(&seed1, g_vpc_id, g_subnet_id, g_vnic_cidr_v4.c_str(),
                            PDS_ENCAP_TYPE_MPLSoUDP, g_encap_val,
                            g_base_vnic_mac, g_tep_id, g_tep_cidr_v4.c_str());
    seed1.num_vnics = k_max_vnic;
    seed1.num_teps = PDS_MAX_TEP - 1 -1;

    REMOTE_MAPPING_SEED_INIT(&seed1A, g_vpc_id, g_subnet_id + 10, g_vnic_cidr_v4.c_str(),
                            PDS_ENCAP_TYPE_MPLSoUDP, g_encap_val + 100,
                            g_base_vnic_mac + 200, g_tep_id, g_tep_cidr_v4.c_str());
    seed1A.num_vnics = k_max_vnic;
    seed1A.num_teps = PDS_MAX_TEP - 1 -1;

    REMOTE_MAPPING_SEED_INIT(&seed1B, g_vpc_id, g_subnet_id, g_vnic_cidr_v4.c_str(),
                            PDS_ENCAP_TYPE_VXLAN, g_encap_val + 200,
                            g_base_vnic_mac, g_tep_id, g_tep_cidr_v4.c_str());
    seed1B.num_vnics = k_max_vnic;
    seed1B.num_teps = PDS_MAX_TEP - 1 -1;

    workflow_7<remote_mapping_util, remote_mapping_stepper_seed_t>(&seed1, &seed1A, &seed1B);
}

/// \brief remote mappings WF_8
/// \ref WF_8
TEST_F(mapping_test, DISABLED_v4_remote_mapping_workflow_8) {
    remote_mapping_stepper_seed_t seed1 = {0};
    remote_mapping_stepper_seed_t seed1A = {0};
    remote_mapping_stepper_seed_t seed1B = {0};

    REMOTE_MAPPING_SEED_INIT(&seed1, g_vpc_id, g_subnet_id, g_vnic_cidr_v4.c_str(),
                            PDS_ENCAP_TYPE_MPLSoUDP, g_encap_val,
                            g_base_vnic_mac, g_tep_id, g_tep_cidr_v4.c_str());
    seed1.num_vnics = k_max_vnic;
    seed1.num_teps = PDS_MAX_TEP - 1 -1;

    REMOTE_MAPPING_SEED_INIT(&seed1A, g_vpc_id, g_subnet_id + 10, g_vnic_cidr_v4.c_str(),
                            PDS_ENCAP_TYPE_MPLSoUDP, g_encap_val + 100,
                            g_base_vnic_mac + 200, g_tep_id, g_tep_cidr_v4.c_str());
    seed1A.num_vnics = k_max_vnic;
    seed1A.num_teps = PDS_MAX_TEP - 1 -1;

    REMOTE_MAPPING_SEED_INIT(&seed1B, g_vpc_id, g_subnet_id + 20, g_vnic_cidr_v4.c_str(),
                            PDS_ENCAP_TYPE_VXLAN, g_encap_val + 20,
                            g_base_vnic_mac + 20, g_tep_id, g_tep_cidr_v4.c_str());
    seed1B.num_vnics = k_max_vnic;
    seed1B.num_teps = PDS_MAX_TEP - 1 -1;

    workflow_8<remote_mapping_util, remote_mapping_stepper_seed_t>(&seed1, &seed1A, &seed1B);
}

/// \brief remote mappings WF_9
/// \ref WF_9
TEST_F(mapping_test, DISABLED_v4_remote_mapping_workflow_9) {
    remote_mapping_stepper_seed_t seed1 = {0};
    remote_mapping_stepper_seed_t seed1A = {0};

    REMOTE_MAPPING_SEED_INIT(&seed1, g_vpc_id, g_subnet_id, g_vnic_cidr_v4.c_str(),
                            PDS_ENCAP_TYPE_MPLSoUDP, g_encap_val,
                            g_base_vnic_mac, g_tep_id, g_tep_cidr_v4.c_str());
    seed1.num_vnics = k_max_vnic;
    seed1.num_teps = PDS_MAX_TEP - 1 -1;

    REMOTE_MAPPING_SEED_INIT(&seed1A, g_vpc_id, g_subnet_id + 10, g_vnic_cidr_v4.c_str(),
                            PDS_ENCAP_TYPE_MPLSoUDP, g_encap_val + 100,
                            g_base_vnic_mac + 1000, g_tep_id, g_tep_cidr_v4.c_str());
    seed1A.num_vnics = k_max_vnic;
    seed1A.num_teps = PDS_MAX_TEP - 1 -1;

    workflow_9<remote_mapping_util, remote_mapping_stepper_seed_t>(&seed1, &seed1A);
}

/// \brief remote mappings WF_10
/// \ref WF_10
TEST_F(mapping_test, DISABLED_v4_remote_mapping_workflow_10) {
    uint32_t num_vnics_per_set = k_max_vnic;
    uint32_t num_teps_per_set = 256;
    uint32_t stepper = num_teps_per_set * num_vnics_per_set;
    ip_prefix_t vip_pfx;
    ip_prefix_t tip_pfx;
    remote_mapping_stepper_seed_t s1 = {0};
    remote_mapping_stepper_seed_t s2 = {0};
    remote_mapping_stepper_seed_t s2A = {0};
    remote_mapping_stepper_seed_t s3 = {0};
    remote_mapping_stepper_seed_t s3A = {0};
    remote_mapping_stepper_seed_t s4 = {0};

    REMOTE_MAPPING_SEED_INIT(&s1, g_vpc_id, g_subnet_id, g_vnic_cidr_v4.c_str(),
                            PDS_ENCAP_TYPE_MPLSoUDP, g_encap_val,
                            g_base_vnic_mac, g_tep_id, g_tep_cidr_v4.c_str());
    s1.num_vnics = num_vnics_per_set;
    s1.num_teps = num_teps_per_set;

    str2ipv4pfx((char *)g_vnic_cidr_v4.c_str(), &vip_pfx);
    step_up_ip_next(&vip_pfx, num_teps_per_set);
    str2ipv4pfx((char *)g_tep_cidr_v4.c_str(), &tip_pfx);
    step_up_ip_next(&tip_pfx, num_teps_per_set);
    REMOTE_MAPPING_SEED_INIT(&s2, g_vpc_id + num_teps_per_set,
                             g_subnet_id + num_teps_per_set,
                             ippfx2str(&vip_pfx),
                             PDS_ENCAP_TYPE_MPLSoUDP, g_encap_val + stepper,
                             g_base_vnic_mac + stepper,
                             g_tep_id + num_teps_per_set,
                             ippfx2str(&tip_pfx));
    s2.num_vnics = num_vnics_per_set;
    s2.num_teps = num_teps_per_set;

    REMOTE_MAPPING_SEED_INIT(&s2A, g_vpc_id + num_teps_per_set,
                             g_subnet_id + num_teps_per_set,
                             ippfx2str(&vip_pfx),
                             PDS_ENCAP_TYPE_VXLAN, g_encap_val + 100,
                             g_base_vnic_mac + 100,
                             g_tep_id + num_teps_per_set,
                             ippfx2str(&tip_pfx));
    s2A.num_vnics = num_vnics_per_set;
    s2A.num_teps = num_teps_per_set;

    step_up_ip_next(&vip_pfx, num_teps_per_set);
    step_up_ip_next(&tip_pfx, num_teps_per_set);
    REMOTE_MAPPING_SEED_INIT(&s3, g_vpc_id + (num_teps_per_set * 2),
                             g_subnet_id + (num_teps_per_set * 2),
                             ippfx2str(&vip_pfx),
                             PDS_ENCAP_TYPE_MPLSoUDP, g_encap_val + (stepper * 2),
                             g_base_vnic_mac + (stepper * 2),
                             g_tep_id + (2 * num_teps_per_set),
                             ippfx2str(&tip_pfx));
    s3.num_vnics = num_vnics_per_set;
    s3.num_teps = num_teps_per_set - 1;

    REMOTE_MAPPING_SEED_INIT(&s3A, g_vpc_id + (num_teps_per_set * 2),
                             g_subnet_id + (num_teps_per_set * 2),
                             ippfx2str(&vip_pfx),
                             PDS_ENCAP_TYPE_VXLAN, g_encap_val + stepper,
                             g_base_vnic_mac + stepper,
                             g_tep_id + (2 * num_teps_per_set),
                             ippfx2str(&tip_pfx));
    s3A.num_vnics = num_vnics_per_set;
    s3A.num_teps = num_teps_per_set - 1;

    step_up_ip_next(&vip_pfx, num_teps_per_set);
    step_up_ip_next(&tip_pfx, num_teps_per_set);
    REMOTE_MAPPING_SEED_INIT(&s4, g_vpc_id + (num_teps_per_set * 3),
                             g_subnet_id + (num_teps_per_set * 3),
                             ippfx2str(&vip_pfx),
                             PDS_ENCAP_TYPE_MPLSoUDP, g_encap_val + (stepper * 3),
                             g_base_vnic_mac + (stepper * 3),
                             g_tep_id + (3 * num_teps_per_set),
                             ippfx2str(&tip_pfx));
    s4.num_vnics = num_vnics_per_set;
    s4.num_teps = num_teps_per_set - 2; // total = 256 + 256 + 256 + 254 = 1022

    workflow_10<remote_mapping_util, remote_mapping_stepper_seed_t>(&s1, &s2, &s2A,
                                                                    &s3, &s3A, &s4);
}

/// \brief remote mappings WF_N_1
/// \ref WF_N_1
TEST_F(mapping_test, DISABLED_v4_remote_mapping_workflow_neg_1) {
    remote_mapping_stepper_seed_t seed = {0};

    REMOTE_MAPPING_SEED_INIT(&seed, g_vpc_id, g_subnet_id, g_vnic_cidr_v4.c_str(),
                            PDS_ENCAP_TYPE_MPLSoUDP, g_encap_val,
                            g_base_vnic_mac, g_tep_id, g_tep_cidr_v4.c_str());
    seed.num_vnics = k_max_vnic;
    seed.num_teps = PDS_MAX_TEP - 1 -1;
    workflow_neg_1<remote_mapping_util, remote_mapping_stepper_seed_t>(&seed);
}

/// \brief remote mappings WF_N_2
/// \ref WF_N_2
TEST_F(mapping_test, DISABLED_v4_remote_mapping_workflow_neg_2) {
    remote_mapping_stepper_seed_t seed = {0};
    remote_mapping_stepper_seed_t dummy_seed = {0};
    std::string dummy_vnic_ip("9.6.8.1/16");
    REMOTE_MAPPING_SEED_INIT(&seed, g_vpc_id, g_subnet_id, g_vnic_cidr_v4.c_str(),
                            PDS_ENCAP_TYPE_MPLSoUDP, g_encap_val,
                            g_base_vnic_mac, g_tep_id, g_tep_cidr_v4.c_str());

    REMOTE_MAPPING_SEED_INIT(&dummy_seed, g_vpc_id, g_subnet_id, dummy_vnic_ip.c_str(),
                            PDS_ENCAP_TYPE_MPLSoUDP, g_encap_val,
                            g_base_vnic_mac, g_tep_id, g_tep_cidr_v4.c_str());

    pds_batch_ctxt_t bctxt = batch_start();
    dummy_seed.num_vnics = 1;
    dummy_seed.num_teps = 1;
    REMOTE_MAPPING_MANY_CREATE(&dummy_seed);
    batch_commit(bctxt);

    seed.num_vnics = k_max_vnic;
    seed.num_teps = PDS_MAX_TEP - 1 -1;
    workflow_neg_2<remote_mapping_util, remote_mapping_stepper_seed_t>(&seed);
}

/// \brief remote mappings WF_N_3
/// \ref WF_N_3
TEST_F(mapping_test, DISABLED_v4_remote_mapping_workflow_neg_3) {
    uint64_t vnic_mac = 0x0000000040302010;
    std::string vnic_cidr = "100.99.98.97";
    std::string tep_cidr = "200.97.98.99";
    remote_mapping_stepper_seed_t seed = {0};


    REMOTE_MAPPING_SEED_INIT(&seed, g_vpc_id, g_subnet_id, vnic_cidr,
                            PDS_ENCAP_TYPE_MPLSoUDP, 4000,
                            vnic_mac, g_tep_id, tep_cidr);
    seed.num_vnics = 1;
    seed.num_teps = 1;
    workflow_neg_3<remote_mapping_util, remote_mapping_stepper_seed_t>(&seed);
}

/// \brief remote mappings WF_N_4
/// \ref WF_N_4
TEST_F(mapping_test, DISABLED_remote_mapping_workflow_neg_4) {
    uint32_t num_vnics_per_set = k_max_vnic;
    uint32_t num_teps_per_set = 511;
    uint32_t stepper = num_teps_per_set * num_vnics_per_set;
    ip_prefix_t vip_pfx;
    ip_prefix_t tip_pfx;
    remote_mapping_stepper_seed_t s1 = {0};
    remote_mapping_stepper_seed_t s2 = {0};

    REMOTE_MAPPING_SEED_INIT(&s1, g_vpc_id, g_subnet_id, g_vnic_cidr_v4.c_str(),
                            PDS_ENCAP_TYPE_MPLSoUDP, g_encap_val,
                            g_base_vnic_mac, g_tep_id, g_tep_cidr_v4.c_str());
    s1.num_vnics = num_vnics_per_set;
    s1.num_teps = num_teps_per_set;

    str2ipv4pfx((char *)g_vnic_cidr_v4.c_str(), &vip_pfx);
    step_up_ip_next(&vip_pfx, num_teps_per_set);
    str2ipv4pfx((char *)g_tep_cidr_v4.c_str(), &tip_pfx);
    step_up_ip_next(&tip_pfx, num_teps_per_set);
    REMOTE_MAPPING_SEED_INIT(&s2, g_vpc_id + num_teps_per_set,
                             g_subnet_id + num_teps_per_set,
                             ippfx2str(&vip_pfx),
                             PDS_ENCAP_TYPE_MPLSoUDP, g_encap_val + stepper,
                             g_base_vnic_mac + stepper,
                             g_tep_id + num_teps_per_set,
                             ippfx2str(&tip_pfx));
    s2.num_vnics = num_vnics_per_set;
    s2.num_teps = num_teps_per_set;

    workflow_neg_4<remote_mapping_util, remote_mapping_stepper_seed_t>(&s1, &s2);
}

/// \brief remote mappings WF_N_5
/// \ref WF_N_5
TEST_F(mapping_test, DISABLED_v4_remote_mapping_workflow_neg_5) {
    remote_mapping_stepper_seed_t seed1 = {0};
    remote_mapping_stepper_seed_t seed1A = {0};

    REMOTE_MAPPING_SEED_INIT(&seed1, g_vpc_id, g_subnet_id, g_vnic_cidr_v4.c_str(),
                            PDS_ENCAP_TYPE_MPLSoUDP, g_encap_val,
                            g_base_vnic_mac, g_tep_id, g_tep_cidr_v4.c_str());
    seed1.num_vnics = k_max_vnic;
    seed1.num_teps = PDS_MAX_TEP - 1 -1;

    REMOTE_MAPPING_SEED_INIT(&seed1A, g_vpc_id, g_subnet_id, g_vnic_cidr_v4.c_str(),
                            PDS_ENCAP_TYPE_VXLAN, g_encap_val + 100,
                            g_base_vnic_mac + 200, g_tep_id, g_tep_cidr_v4.c_str());
    seed1A.num_vnics = k_max_vnic;
    seed1A.num_teps = PDS_MAX_TEP - 1 -1;

    workflow_neg_5<remote_mapping_util, remote_mapping_stepper_seed_t>(&seed1, &seed1A);
}

/// \brief remote mappings WF_N_6
/// \ref WF_N_6
TEST_F(mapping_test, DISABLED_v4_remote_mapping_workflow_neg_6) {
    remote_mapping_stepper_seed_t seed1 = {0};
    remote_mapping_stepper_seed_t seed1A = {0};

    REMOTE_MAPPING_SEED_INIT(&seed1, g_vpc_id, g_subnet_id, g_vnic_cidr_v4.c_str(),
                            PDS_ENCAP_TYPE_MPLSoUDP, g_encap_val,
                            g_base_vnic_mac, g_tep_id, g_tep_cidr_v4.c_str());
    seed1.num_vnics = k_max_vnic;
    seed1.num_teps = PDS_MAX_TEP - 1 -1;

    REMOTE_MAPPING_SEED_INIT(&seed1A, g_vpc_id, g_subnet_id, g_vnic_cidr_v4.c_str(),
                            PDS_ENCAP_TYPE_VXLAN, g_encap_val + 100,
                            g_base_vnic_mac + 200, g_tep_id, g_tep_cidr_v4.c_str());
    seed1A.num_vnics = k_max_vnic;
    seed1A.num_teps = PDS_MAX_TEP;

    workflow_neg_6<remote_mapping_util, remote_mapping_stepper_seed_t>(&seed1, &seed1A);
}

/// \brief remote mappings WF_N_7
/// \ref WF_N_7
TEST_F(mapping_test, DISABLED_v4_remote_mapping_workflow_neg_7) {
    uint32_t num_vnics_per_set = k_max_vnic;
    uint32_t num_teps_per_set = 511;
    uint32_t stepper = num_teps_per_set * num_vnics_per_set;
    ip_prefix_t vip_pfx;
    ip_prefix_t tip_pfx;
    remote_mapping_stepper_seed_t seed1 = {0};
    remote_mapping_stepper_seed_t seed1A = {0};
    remote_mapping_stepper_seed_t seed2 = {0};

    REMOTE_MAPPING_SEED_INIT(&seed1, g_vpc_id, g_subnet_id, g_vnic_cidr_v4.c_str(),
                            PDS_ENCAP_TYPE_MPLSoUDP, g_encap_val,
                            g_base_vnic_mac, g_tep_id, g_tep_cidr_v4.c_str());
    seed1.num_vnics = num_vnics_per_set;
    seed1.num_teps = num_teps_per_set;

    REMOTE_MAPPING_SEED_INIT(&seed1A, g_vpc_id, g_subnet_id, g_vnic_cidr_v4.c_str(),
                            PDS_ENCAP_TYPE_VXLAN, g_encap_val + 100,
                            g_base_vnic_mac + 200, g_tep_id, g_tep_cidr_v4.c_str());
    seed1A.num_vnics = num_vnics_per_set;
    seed1A.num_teps = num_teps_per_set;

    str2ipv4pfx((char *)g_vnic_cidr_v4.c_str(), &vip_pfx);
    step_up_ip_next(&vip_pfx, num_teps_per_set);
    str2ipv4pfx((char *)g_tep_cidr_v4.c_str(), &tip_pfx);
    step_up_ip_next(&tip_pfx, num_teps_per_set);
    REMOTE_MAPPING_SEED_INIT(&seed2, g_vpc_id + num_teps_per_set,
                             g_subnet_id + num_teps_per_set,
                             ippfx2str(&vip_pfx),
                             PDS_ENCAP_TYPE_MPLSoUDP, g_encap_val + stepper,
                             g_base_vnic_mac + stepper,
                             g_tep_id + num_teps_per_set,
                             ippfx2str(&tip_pfx));
    seed2.num_vnics = num_vnics_per_set;
    seed2.num_teps = num_teps_per_set;

    workflow_neg_7<remote_mapping_util, remote_mapping_stepper_seed_t>(&seed1, &seed1A, &seed2);
}

/// \brief remote mappings WF_N_8
/// \ref WF_N_8
TEST_F(mapping_test, DISABLED_v4_remote_mapping_workflow_neg_8) {
    uint32_t num_vnics_per_set = k_max_vnic;
    uint32_t num_teps_per_set = 511;
    uint32_t stepper = num_teps_per_set * num_vnics_per_set;
    ip_prefix_t vip_pfx;
    ip_prefix_t tip_pfx;
    remote_mapping_stepper_seed_t seed1 = {0};
    remote_mapping_stepper_seed_t seed2 = {0};

    REMOTE_MAPPING_SEED_INIT(&seed1, g_vpc_id, g_subnet_id, g_vnic_cidr_v4.c_str(),
                            PDS_ENCAP_TYPE_MPLSoUDP, g_encap_val,
                            g_base_vnic_mac, g_tep_id, g_tep_cidr_v4.c_str());
    seed1.num_vnics = num_vnics_per_set;
    seed1.num_teps = num_teps_per_set;

    str2ipv4pfx((char *)g_vnic_cidr_v4.c_str(), &vip_pfx);
    step_up_ip_next(&vip_pfx, num_teps_per_set);
    str2ipv4pfx((char *)g_tep_cidr_v4.c_str(), &tip_pfx);
    step_up_ip_next(&tip_pfx, num_teps_per_set);
    REMOTE_MAPPING_SEED_INIT(&seed2, g_vpc_id + num_teps_per_set,
                             g_subnet_id + num_teps_per_set,
                             ippfx2str(&vip_pfx),
                             PDS_ENCAP_TYPE_MPLSoUDP, g_encap_val + stepper,
                             g_base_vnic_mac + stepper,
                             g_tep_id + num_teps_per_set,
                             ippfx2str(&tip_pfx));
    seed2.num_vnics = num_vnics_per_set;
    seed2.num_teps = num_teps_per_set;

    workflow_neg_8<remote_mapping_util, remote_mapping_stepper_seed_t>(&seed1, &seed2);
}

/// --------------------------END REMOTE MAPPINGS -----------------------

/// --------------------------END IPv4 MAPPINGS -----------------------
// @}

}    // namespace api_test

//----------------------------------------------------------------------------
// Entry point
//----------------------------------------------------------------------------

/// @private
int
main (int argc, char **argv)
{
    return api_test_program_run(argc, argv);
}
