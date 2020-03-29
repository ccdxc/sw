// {C} Copyright 2019 Pensando Systems Inc. All rights reserved

#include <libgen.h>
#include "lib/device/device.hpp"
#include "include/sdk/mem.hpp"
#include "lib/utils/utils.hpp"

namespace sdk {
namespace lib {

SDK_DEFINE_MAP(dev_forwarding_mode_t, DEV_FORWARDING_MODE)
SDK_DEFINE_MAP(dev_feature_profile_t, DEV_FEATURE_PROFILE)
SDK_DEFINE_MAP(dev_port_state_t, DEV_PORT_STATE)
SDK_DEFINE_MAP(dev_micro_seg_t, DEV_MICRO_SEG)

#define FORWARDING_MODE_KEY_STR "forwarding-mode"
#define FEATURE_PROFILE_KEY_STR "feature-profile"
#define PORT_ADMIN_STATE_KEY_STR "port-admin-state"
#define MGMT_IF_MAC_STR "mgmt-if-mac"
#define MGMT_VLAN_STR "mgmt-vlan"
#define DEVICE_QOS_PROFILE_STR "profile.qos"
#define MICRO_SEG_STR "micro-seg-mode"

device *
device::factory(std::string device_file)
{
    void       *mem;
    device     *new_device;
    sdk_ret_t  ret = SDK_RET_OK;

    mem = SDK_CALLOC(sdk::SDK_MEM_ALLOC_DEVICE, sizeof(device));
    if (!mem) {
        SDK_TRACE_ERR("Failed to allocate memory for device");
        return NULL;
    }

    new_device = new (mem) device();
    ret = new_device->init(device_file);
    if (ret != SDK_RET_OK) {
        SDK_TRACE_ERR("Failed to init device");
        new_device->~device();
        SDK_FREE(sdk::SDK_MEM_ALLOC_DEVICE, new_device);
        return NULL;
    }

    return new_device;
}


device::~device()
{
}

void
device::destroy(device *dev)
{
    if (!dev) {
        return;
    }
    dev->~device();
    SDK_FREE(sdk::SDK_MEM_ALLOC_DEVICE, dev);
}

sdk_ret_t
device::init(std::string &device_file)
{
    sdk_ret_t ret;
    ptree prop_tree;

    if (device_file.empty()) {
        device_file = DEFAULT_DEVICE_FILE;
    }

    SDK_TRACE_DEBUG("device file: %s", device_file.c_str());
    device_db_.device_file = device_file;

    // dirname modifies device_file string
    device_db_.device_cfg_path = dirname((char*)device_file.c_str());

    ret = get_ptree_(device_db_.device_file, prop_tree);
    if (ret != SDK_RET_OK) {
        // No device.conf file ... return defaults
        populate_device_defaults_();
        return SDK_RET_OK;
    }

    return populate_device(prop_tree);
}

sdk_ret_t
device::get_ptree_(std::string& device_file, ptree& prop_tree)
{
    if (access(device_file.c_str(), R_OK) < 0) {
        SDK_TRACE_DEBUG("device file %s has no read permissions/not preset",
                        device_file.c_str());
        return SDK_RET_ERR;
    }

    boost::property_tree::read_json(device_file, prop_tree);
    return SDK_RET_OK;
}

sdk_ret_t
device::populate_qos_profile(std::string qos_profile_name) {
    ptree prop_tree;
    uint32_t i = 0;
    qos_profile_t *qos_profile = &device_db_.device_profile.qos_profile;
    std::string profile_fname = device_db_.device_cfg_path + "/profiles/" +
                                "qos_profile_" + qos_profile_name + ".json";

    if (access(profile_fname.c_str(), R_OK) < 0) {
        SDK_TRACE_DEBUG("Device config profile: {} not found",
                        profile_fname.c_str());
        return SDK_RET_ERR;
    }
    boost::property_tree::read_json(profile_fname, prop_tree);
    qos_profile->jumbo_mtu =
                prop_tree.get<uint32_t>("qos.jumbo_mtu", 9216);
    qos_profile->num_uplink_qs =
                prop_tree.get<uint32_t>("qos.num_uplink_qs", 8);
    qos_profile->num_p4ig_qs =
                prop_tree.get<uint32_t>("qos.num_p4ig_qs", 25);
    qos_profile->num_p4eg_qs =
                prop_tree.get<uint32_t>("qos.num_p4eg_qs", 27);
    qos_profile->num_dma_qs =
                prop_tree.get<uint32_t>("qos.num_dma_qs", 16);
    qos_profile->num_p4_high_perf_qs = prop_tree.get<uint32_t>(
                                        "qos.num_p4_high_perf_qs", 2);
    for (ptree::value_type &p4_high_perf_q :
                        prop_tree.get_child("qos.p4_high_perf_qs")) {
        qos_profile->p4_high_perf_qs[i++] =
                p4_high_perf_q.second.get_value<int32_t>();
    }
    return SDK_RET_OK;
}

sdk_ret_t
device::populate_device(ptree &pt)
{
    std::string qos_profile_name;

    populate_device_defaults_();

    ptree::const_iterator end = pt.end();
    for (ptree::const_iterator it = pt.begin(); it != end; ++it) {
        if (it->first == FORWARDING_MODE_KEY_STR) {
            if (is_integer_(it->second.get_value<std::string>())) {
                device_db_.fwd_mode = (dev_forwarding_mode_t)pt.get<int>(FORWARDING_MODE_KEY_STR);
            } else {
                std::string fwd_mode = pt.get<std::string>(FORWARDING_MODE_KEY_STR);
                if (DEV_FORWARDING_MODE_map.find(fwd_mode) == DEV_FORWARDING_MODE_map.end()) {
                    SDK_TRACE_ERR("Unable to find %s: %s ... setting default: %s",
                                  FORWARDING_MODE_KEY_STR, fwd_mode.c_str(),
                                  DEV_FORWARDING_MODE_str(FORWARDING_MODE_CLASSIC));

                } else {
                    device_db_.fwd_mode =
                        DEV_FORWARDING_MODE_map[fwd_mode.c_str()];
                }
            }
        }

        if (it->first == FEATURE_PROFILE_KEY_STR) {
            SDK_TRACE_DEBUG("Assigning feature profile\n");
            if (is_integer_(it->second.get_value<std::string>())) {
                device_db_.feature_profile = (dev_feature_profile_t)pt.get<int>(FEATURE_PROFILE_KEY_STR);
            } else {
                std::string feature_profile = pt.get<std::string>(FEATURE_PROFILE_KEY_STR);
                if (DEV_FEATURE_PROFILE_map.find(feature_profile) == DEV_FEATURE_PROFILE_map.end()) {
                    SDK_TRACE_ERR("Unable to find %s: %s ... setting default: %s",
                                  FEATURE_PROFILE_KEY_STR, feature_profile.c_str(),
                                  DEV_FEATURE_PROFILE_str(FEATURE_PROFILE_BASE));

                } else {
                    device_db_.feature_profile =
                        DEV_FEATURE_PROFILE_map[pt.get<std::string>(FEATURE_PROFILE_KEY_STR)];
                }
            }
            if (device_db_.feature_profile == FEATURE_PROFILE_NONE) {
                device_db_.feature_profile = FEATURE_PROFILE_BASE;
            }
        }

        if (it->first == PORT_ADMIN_STATE_KEY_STR) {
            if (is_integer_(it->second.get_value<std::string>())) {
                device_db_.port_admin_state = (dev_port_state_t)pt.get<int>(PORT_ADMIN_STATE_KEY_STR);
            } else {
                std::string port_state = pt.get<std::string>(PORT_ADMIN_STATE_KEY_STR);
                if (DEV_PORT_STATE_map.find(port_state) == DEV_PORT_STATE_map.end()) {
                    SDK_TRACE_ERR("Unable to find %s: %s ... setting default: %s",
                                  PORT_ADMIN_STATE_KEY_STR, port_state.c_str(),
                                  DEV_PORT_STATE_str(PORT_ADMIN_STATE_ENABLE));

                } else {
                    device_db_.port_admin_state =
                        DEV_PORT_STATE_map[pt.get<std::string>(PORT_ADMIN_STATE_KEY_STR)];
                }
            }
        }

        if (it->first == MGMT_IF_MAC_STR) {
            device_db_.mgmt_if_mac = pt.get<std::uint64_t>(MGMT_IF_MAC_STR);
        }

        if (it->first == MGMT_VLAN_STR) {
            device_db_.mgmt_vlan = pt.get<std::uint32_t>(MGMT_VLAN_STR);
        }

        if (it->first == DEVICE_QOS_PROFILE_STR) {
            qos_profile_name =
                    pt.get<std::string>(DEVICE_QOS_PROFILE_STR, "default");
            if (populate_qos_profile(qos_profile_name) != SDK_RET_OK) {
                return SDK_RET_ERR;
            }
        }

        if (it->first == MICRO_SEG_STR) {
            if (is_integer_(it->second.get_value<std::string>())) {
                device_db_.micro_seg_en = (dev_micro_seg_t)pt.get<int>(MICRO_SEG_STR);
            } else {
                std::string mseg_en = pt.get<std::string>(MICRO_SEG_STR);
                if (DEV_MICRO_SEG_map.find(mseg_en) == DEV_MICRO_SEG_map.end()) {
                    SDK_TRACE_ERR("Unable to find %s: %s ... setting default: %s",
                                  MICRO_SEG_STR, mseg_en.c_str(),
                                  DEV_MICRO_SEG_str(MICRO_SEG_DISABLE));

                } else {
                    device_db_.micro_seg_en =
                        DEV_MICRO_SEG_map[pt.get<std::string>(MICRO_SEG_STR)];
                }
            }
        }
    }
    return SDK_RET_OK;
}

void
device::populate_device_defaults_(void)
{
    device_db_.fwd_mode = FORWARDING_MODE_CLASSIC;
    device_db_.feature_profile = FEATURE_PROFILE_BASE;
    device_db_.port_admin_state = PORT_ADMIN_STATE_ENABLE;
    device_db_.mgmt_if_mac = 0;
    device_db_.mgmt_vlan = 0;
    device_db_.device_profile.qos_profile =
                            {9216, 8, 25, 27, 16, 2, {0, 24}};
}

bool
device::is_integer_(const std::string & s)
{
    if(s.empty() || ((!isdigit(s[0])) && (s[0] != '-') && (s[0] != '+'))) return false;

    char * p;
    strtol(s.c_str(), &p, 10);

    return (*p == 0);
}

}    // namespace lib
}    // namespace sdk
