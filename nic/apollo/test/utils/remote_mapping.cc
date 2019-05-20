//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// This file contains the local IP map test utility routines implementation
///
//----------------------------------------------------------------------------

#include "iostream"
#include "nic/sdk/include/sdk/ip.hpp"
#include "nic/apollo/test/utils/utils.hpp"
#include "nic/apollo/api/include/pds_vnic.hpp"
#include "nic/apollo/test/utils/remote_mapping.hpp"

namespace api_test {

remote_mapping_util::remote_mapping_util() {}

remote_mapping_util::remote_mapping_util(pds_vpc_id_t vpc_id,
                                         pds_subnet_id_t sub_id,
                                         std::string vnic_ip,
                                         std::string tep_ip, uint64_t vnic_mac,
                                         pds_encap_type_t encap_type,
                                         uint32_t encap_val) {
    mac_addr_t mac;
    this->vpc_id = vpc_id;
    this->vnic_ip = vnic_ip;
    this->sub_id = sub_id;
    this->tep_ip = tep_ip;
    this->encap_type = encap_type;
    switch (encap_type) {
    case PDS_ENCAP_TYPE_MPLSoUDP:
        this->mpls_tag = encap_val;
        break;
    case PDS_ENCAP_TYPE_VXLAN:
        this->vxlan_id = encap_val;
        break;
    default:
        break;
    }
    MAC_UINT64_TO_ADDR(mac, vnic_mac);
    this->vnic_mac = macaddr2str(mac);
}

remote_mapping_util::~remote_mapping_util() {}

sdk::sdk_ret_t
remote_mapping_util::create(void) {
    pds_remote_mapping_spec_t remote_spec = {0};
    struct in_addr ipaddr;

    remote_spec.key.vpc.id = this->vpc_id;
    extract_ip_addr(this->vnic_ip.c_str(), &remote_spec.key.ip_addr);
    remote_spec.subnet.id = this->sub_id;

    switch (this->encap_type) {
    case PDS_ENCAP_TYPE_MPLSoUDP:
        remote_spec.fabric_encap.type = PDS_ENCAP_TYPE_MPLSoUDP;
        remote_spec.fabric_encap.val.mpls_tag = this->mpls_tag;
        break;
    case PDS_ENCAP_TYPE_VXLAN:
        remote_spec.fabric_encap.type = PDS_ENCAP_TYPE_VXLAN;
        remote_spec.fabric_encap.val.vnid = this->vxlan_id;
        break;
    default:
        break;
    }

    inet_aton(this->tep_ip.c_str(), &ipaddr);
    remote_spec.tep.ip_addr = ntohl(ipaddr.s_addr);
    mac_str_to_addr((char *)this->vnic_mac.c_str(), remote_spec.vnic_mac);

    return pds_remote_mapping_create(&remote_spec);
}

sdk::sdk_ret_t
remote_mapping_util::read(pds_remote_mapping_info_t *info) {
    // uint64_t mac1, mac2;
    sdk_ret_t rv = sdk::SDK_RET_OK;
    pds_mapping_key_t key = {0};
    // mac_addr_t mac;

    // mac_str_to_addr((char *)this->vnic_mac.c_str(), mac);
    key.vpc.id = this->vpc_id;
    extract_ip_addr(this->vnic_ip.c_str(), &key.ip_addr);
    memset(info, 0, sizeof(*info));
    if ((rv = pds_remote_mapping_read(&key, info) )!= sdk::SDK_RET_OK)
        return rv;

    if (capri_mock_mode())
        return SDK_RET_OK;

    std::cout << "Mapping info:"
              << "\n";
    std::cout << "HW: vpc id: " << info->spec.key.vpc.id
              << " SW: vpc_id: " << this->vpc_id << "\n";
    std::cout << "HW: vnic ip: " << ipaddr2str(&info->spec.key.ip_addr)
              << " SW: vpc_id: " << this->vnic_ip.c_str() << "\n";
    std::cout << "HW: tep ip: " << ipv4addr2str(info->spec.tep.ip_addr)
              << " SW: tep ip: " << this->tep_ip.c_str() << "\n";
    std::cout << "HW: mpls_tag: " << info->spec.fabric_encap.type
              << " SW: mpls_tag: " << this->mpls_tag << "\n";

    // mac1 = MAC_TO_UINT64(mac);
    // mac2 = MAC_TO_UINT64(info->spec.vnic_mac);
    SDK_ASSERT(this->vpc_id == info->spec.key.vpc.id);
    // SDK_ASSERT(this->sub_id == info->spec.subnet.id); // not stored in hw
    // table
    SDK_ASSERT(strcmp(this->vnic_ip.c_str(),
                      ipaddr2str(&info->spec.key.ip_addr)) == 0);
    if (this->encap_type == PDS_ENCAP_TYPE_MPLSoUDP) {
        SDK_ASSERT(PDS_ENCAP_TYPE_MPLSoUDP == info->spec.fabric_encap.type);
        SDK_ASSERT(this->mpls_tag == info->spec.fabric_encap.val.mpls_tag);
    } else if (this->encap_type == PDS_ENCAP_TYPE_VXLAN) {
        SDK_ASSERT(PDS_ENCAP_TYPE_VXLAN == info->spec.fabric_encap.type);
        SDK_ASSERT(this->vxlan_id == info->spec.fabric_encap.val.vnid);
    }
    // SDK_ASSERT((mac1 == mac2) == 0); // not stored in hw table
    SDK_ASSERT(strcmp(this->tep_ip.c_str(),
                      ipv4addr2str(info->spec.tep.ip_addr)) == 0);

    return rv;
}

sdk::sdk_ret_t
remote_mapping_util::del(void) {
    pds_mapping_key_t key = {0};

    key.vpc.id = vpc_id;
    extract_ip_addr(vnic_ip.c_str(), &key.ip_addr);
    return pds_remote_mapping_delete(&key);
}

/// \brief get <num_vnics> per vpc
/// and create/delete/read mappings for
/// given <vpc_id, sub_id, tep_ip>
/// max mappings = num_vnics * num_teps
/// eg. tep ip = 10.10.0.1,
/// then vnic_ip = 10.10.0.2
static inline sdk::sdk_ret_t
mapping_util_object_stepper (
    utils_op_t op, uint32_t num_vnics, pds_vpc_id_t vpc_id,
    pds_subnet_id_t sub_id, uint32_t num_teps,
    remote_mapping_stepper_seed_t *seed,
    pds_encap_type_t encap_type = PDS_ENCAP_TYPE_MPLSoUDP,
    sdk::sdk_ret_t expected_result = sdk::SDK_RET_OK) {
    uint32_t curr_encap_val;
    uint64_t curr_vnic_mac;
    sdk::sdk_ret_t rv = sdk::SDK_RET_OK;
    pds_remote_mapping_info_t info = {0};
    ip_prefix_t ippfx;
    ip_addr_t ipaddr;
    ip_addr_t ipaddr_next;
    std::string vnic_ip = "0.0.0.0";
    ip_prefix_t tep_ippfx;
    ip_addr_t tep_ipaddr;
    std::string tep_ip = "0.0.0.0";

    // Calculate vnic ip from vnic ip string
    extract_ip_pfx((char *)seed->vnic_ip_stepper.c_str(), &ippfx);
    ipaddr = ippfx.addr;
    if (ip_version(seed->vnic_ip_stepper.c_str()) == IP_AF_IPV6) {
        vnic_ip = ipv6addr2str(ipaddr.addr.v6_addr);
    } else {
        // ipaddr.addr.v4_addr += 1;
        vnic_ip = ipv4addr2str(ipaddr.addr.v4_addr);
    }

    // Max mapping limit is reached when reaches
    // this ip address i.e. <ipaddr_next>
    // for given <vnic_ip_cidr_str>
    ip_prefix_ip_high(&ippfx, &ipaddr_next);

    // Get tep ip for all vnic_ip in this <vpc, subnet>
    extract_ip_pfx((char *)seed->tep_ip_stepper.c_str(), &tep_ippfx);
    tep_ipaddr = tep_ippfx.addr;
    if (ip_version(seed->tep_ip_stepper.c_str()) == IP_AF_IPV6) {
        tep_ip = ipv6addr2str(tep_ipaddr.addr.v6_addr);
    } else {
        tep_ip = ipv4addr2str(tep_ipaddr.addr.v4_addr);
    }

    curr_vnic_mac = seed->vnic_mac_stepper;
    curr_encap_val = seed->encap_val_stepper;

    SDK_ASSERT(num_vnics * num_teps <= (1 << 20));
    for (uint32_t tep_indx = 0; tep_indx < num_teps; tep_indx++) {
        for (uint32_t vnic_indx = 1; vnic_indx <= num_vnics; vnic_indx++) {
            remote_mapping_util mapping_obj(vpc_id, sub_id, vnic_ip, tep_ip,
                                            curr_vnic_mac++, encap_type,
                                            curr_encap_val++);

            std::cout << "vpc_id: " << vpc_id << " and vnic_indx:" << vnic_indx
                      << "\n ";
            std::cout << "vnic_ip:" << vnic_ip << " and tep_ip: " << tep_ip
                      << "\n";
            // std::cout << "encap_val:" << curr_encap_val << "and " <<
            // "vnic_mac:" << curr_vnic_mac << "\n";
            switch (op) {
            case OP_MANY_CREATE:
                rv = mapping_obj.create();
                break;
            case OP_MANY_DELETE:
                rv = mapping_obj.del();
                break;
            case OP_MANY_READ:
                rv = mapping_obj.read(&info);
                break;
            default:
                return sdk::SDK_RET_INVALID_OP;
            }

            if (rv != expected_result) {
                return sdk::SDK_RET_ERR;
            }

            // Increment to next mapping i.e. next vnic ip
            if (ip_version(vnic_ip.c_str()) == IP_AF_IPV6) {
                // ippfx.addr.addr.v6_addr += 1;
            } else {
                ipaddr.addr.v4_addr += 1;
                vnic_ip = ipv4addr2str(ipaddr.addr.v4_addr);
            }

            // Reached max mapping limit as per ip_prefix len
            if (ipaddr.addr.v4_addr == ipaddr_next.addr.v4_addr) {
                printf("reached limit for vnics. Added: %d\n", vnic_indx);
                SDK_ASSERT(0);
            }
        }

        vpc_id++;
        sub_id++;
        ip_prefix_ip_next(&ippfx, &ipaddr);
        if (ip_version(vnic_ip.c_str()) == IP_AF_IPV6) {
            vnic_ip = ipv6addr2str(ipaddr.addr.v6_addr);
        } else {
            ippfx.addr = ipaddr;
            vnic_ip = ipv4addr2str(ipaddr.addr.v4_addr);
        }

        if (ip_version(tep_ip.c_str()) == IP_AF_IPV6) {
            // TODO: Increment IPv6
            tep_ip = ipv6addr2str(tep_ipaddr.addr.v6_addr);
        } else {
            tep_ipaddr.addr.v4_addr += 1;
            tep_ip = ipv4addr2str(tep_ipaddr.addr.v4_addr);
        }
    }
    return rv;
}

sdk::sdk_ret_t
remote_mapping_util::many_create(uint16_t num_vnics, uint16_t num_teps,
                                 pds_vpc_id_t vpc_id, pds_subnet_id_t sub_id,
                                 remote_mapping_stepper_seed_t *seed,
                                 pds_encap_type_t encap_type) {
    return (mapping_util_object_stepper(OP_MANY_CREATE, num_vnics, vpc_id,
                                        sub_id, num_teps, seed, encap_type));
}

sdk::sdk_ret_t
remote_mapping_util::many_delete(uint16_t num_vnics, uint16_t num_teps,
                                 pds_vpc_id_t vpc_id,
                                 remote_mapping_stepper_seed_t *seed) {
    return (mapping_util_object_stepper(OP_MANY_DELETE, num_vnics, vpc_id, 0, 0,
                                        seed));
}

sdk::sdk_ret_t
remote_mapping_util::many_read(uint16_t num_vnics, uint16_t num_teps,
                               pds_vpc_id_t vpc_id, pds_subnet_id_t sub_id,
                               remote_mapping_stepper_seed_t *seed,
                               pds_encap_type_t encap_type,
                               sdk::sdk_ret_t expected_result) {
    return (mapping_util_object_stepper(OP_MANY_READ, num_vnics, vpc_id, sub_id,
                                        num_teps, seed, encap_type,
                                        expected_result));
}

}    // namespace api_test
