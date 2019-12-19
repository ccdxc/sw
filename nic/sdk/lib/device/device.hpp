// {C} Copyright 2019 Pensando Systems Inc. All rights reserved

#ifndef __SDK_DEVICE_HPP__
#define __SDK_DEVICE_HPP__

#include <map>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include "include/sdk/base.hpp"
#include "include/sdk/types.hpp"

#define DEVICE_CFG_FNAME "device.conf"

namespace sdk {
namespace lib {

using boost::property_tree::ptree;

#define DEFAULT_DEVICE_FILE "/nic/conf/" DEVICE_CFG_FNAME

#define DEV_FORWARDING_MODE(ENTRY)                                          \
    ENTRY(FORWARDING_MODE_NONE,         0, "FORWARDING_MODE_NONE")          \
    ENTRY(FORWARDING_MODE_CLASSIC,      1, "FORWARDING_MODE_CLASSIC")       \
    ENTRY(FORWARDING_MODE_HOSTPIN,      2, "FORWARDING_MODE_HOSTPIN")       \
    ENTRY(FORWARDING_MODE_SWITCH,       3, "FORWARDING_MODE_SWITCH")

SDK_DEFINE_ENUM(dev_forwarding_mode_t, DEV_FORWARDING_MODE)
SDK_DEFINE_ENUM_TO_STR(dev_forwarding_mode_t, DEV_FORWARDING_MODE)
SDK_DEFINE_MAP_EXTERN(dev_forwarding_mode_t, DEV_FORWARDING_MODE)
// #undef DEV_FORWARDING_MODE

#define DEV_FEATURE_PROFILE(ENTRY)                                                           \
    ENTRY(FEATURE_PROFILE_NONE,                  0, "FEATURE_PROFILE_NONE")                  \
    ENTRY(FEATURE_PROFILE_CLASSIC_DEFAULT,       1, "FEATURE_PROFILE_CLASSIC_DEFAULT")       \
    ENTRY(FEATURE_PROFILE_CLASSIC_ETH_DEV_SCALE, 2, "FEATURE_PROFILE_CLASSIC_ETH_DEV_SCALE")

SDK_DEFINE_ENUM(dev_feature_profile_t, DEV_FEATURE_PROFILE)
SDK_DEFINE_ENUM_TO_STR(dev_feature_profile_t, DEV_FEATURE_PROFILE)
SDK_DEFINE_MAP_EXTERN(dev_feature_profile_t, DEV_FEATURE_PROFILE)
// #undef DEV_FEATURE_PROFILE

#define DEV_PORT_STATE(ENTRY)                                           \
    ENTRY(PORT_ADMIN_STATE_ENABLE,      0, "PORT_ADMIN_STATE_ENABLE")   \
    ENTRY(PORT_ADMIN_STATE_DISABLE,     1, "PORT_ADMIN_STATE_DISABLE")

SDK_DEFINE_ENUM(dev_port_state_t, DEV_PORT_STATE)
SDK_DEFINE_ENUM_TO_STR(dev_port_state_t, DEV_PORT_STATE)
SDK_DEFINE_MAP_EXTERN(dev_port_state_t, DEV_PORT_STATE)
// #undef DEV_PORT_STATE

#define DEV_MICRO_SEG(ENTRY)                              \
    ENTRY(MICRO_SEG_DISABLE,    0, "MICRO_SEG_DISABLE")   \
    ENTRY(MICRO_SEG_ENABLE,     1, "MICRO_SEG_ENABLE")

SDK_DEFINE_ENUM(dev_micro_seg_t, DEV_MICRO_SEG)
SDK_DEFINE_ENUM_TO_STR(dev_micro_seg_t, DEV_MICRO_SEG)
SDK_DEFINE_MAP_EXTERN(dev_micro_seg_t, DEV_MICRO_SEG)
// #undef DEV_PORT_STATE

typedef struct qos_profile_s {
    uint32_t jumbo_mtu;
    uint32_t num_uplink_qs;
    uint32_t num_p4ig_qs;
    uint32_t num_p4eg_qs;
    uint32_t num_dma_qs;
    uint32_t num_p4_high_perf_qs;
    int32_t  p4_high_perf_qs[2];
} qos_profile_t;

typedef struct device_profile_s {
    qos_profile_t qos_profile;
} device_profile_t;

typedef struct device_s {
    std::string            device_file;            // device.conf file with absolute path
    dev_forwarding_mode_t  fwd_mode;               // forwarding mode
    dev_feature_profile_t  feature_profile;        // feature profile
    dev_port_state_t       port_admin_state;       // ports' default admin state
    uint64_t               mgmt_if_mac;            // mgmt if's mac. used only for telemetry
    uint32_t               mgmt_vlan;              // vlan encap on uplinks for mgmt
    device_profile_t       device_profile;         // device config profile
    std::string            device_cfg_path;        // device config path
    dev_micro_seg_t        micro_seg_en;
} device_t;

class device {
public:
    static device *factory(std::string device_file="");
    static void destroy(device *dev);

    dev_forwarding_mode_t get_forwarding_mode(void) { return device_db_.fwd_mode; }
    dev_feature_profile_t get_feature_profile(void) { return device_db_.feature_profile; }
    dev_port_state_t get_port_admin_state(void) { return device_db_.port_admin_state; }
    uint64_t get_mgmt_if_mac(void) { return device_db_.mgmt_if_mac; }
    uint32_t get_mgmt_vlan(void) { return device_db_.mgmt_vlan; }
    device_profile_t *device_profile(void) {
        return &device_db_.device_profile;
    }
    bool get_micro_seg_en(void) { return device_db_.micro_seg_en; }

private:
    device_t device_db_;    // device database

private:
    device() {};
    ~device();

    sdk_ret_t init(std::string &device_file);
    static sdk_ret_t get_ptree_(std::string& device_file, ptree& prop_tree);
    bool is_integer_(const std::string &s);
    void populate_device_defaults_();

    // populate device
    sdk_ret_t populate_device(ptree &pt);

    // populate qos profile
    sdk_ret_t populate_qos_profile(std::string qos_profile_name);
};


}    // namespace lib
}    // namespace sdk

#endif    //__SDK_DEVICE_HPP__
