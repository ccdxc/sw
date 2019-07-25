// {C} Copyright 2019 Pensando Systems Inc. All rights reserved

#include "lib/device/device.hpp"
#include "include/sdk/mem.hpp"
#include "lib/utils/utils.hpp"

namespace sdk {
namespace lib {

SDK_DEFINE_MAP(dev_forwarding_mode_t, DEV_FORWARDING_MODE)
SDK_DEFINE_MAP(dev_feature_profile_t, DEV_FEATURE_PROFILE)
SDK_DEFINE_MAP(dev_port_state_t, DEV_PORT_STATE)

#define FORWARDING_MODE_KEY_STR "forwarding-mode"
#define FEATURE_PROFILE_KEY_STR "feature-profile"
#define PORT_ADMIN_STATE_KEY_STR "port-admin-state"
#define MGMT_IF_MAC_STR "mgmt-if-mac"

device *
device::factory(std::string device_file)
{
    void       *mem;
    device     *new_device;
    sdk_ret_t  ret = SDK_RET_OK;

    if (device_file.empty()) {
        device_file = DEFAULT_DEVICE_FILE;
    }

    SDK_TRACE_DEBUG("device file: %s", device_file.c_str());

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

    device_db_.device_file = device_file;
    ret = get_ptree_(device_file, prop_tree);
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
        SDK_TRACE_ERR("device file %s has no read permissions",
                      device_file.c_str());
        return SDK_RET_ERR;
    }

    boost::property_tree::read_json(device_file, prop_tree);
    return SDK_RET_OK;
}

sdk_ret_t
device::populate_device(ptree &pt)
{

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
            if (is_integer_(it->second.get_value<std::string>())) {
                device_db_.feature_profile = (dev_feature_profile_t)pt.get<int>(FEATURE_PROFILE_KEY_STR);
            } else {
                std::string feature_profile = pt.get<std::string>(FEATURE_PROFILE_KEY_STR);
                if (DEV_FEATURE_PROFILE_map.find(feature_profile) == DEV_FEATURE_PROFILE_map.end()) {
                    SDK_TRACE_ERR("Unable to find %s: %s ... setting default: %s", 
                                  FEATURE_PROFILE_KEY_STR, feature_profile.c_str(), 
                                  DEV_FEATURE_PROFILE_str(FEATURE_PROFILE_CLASSIC_DEFAULT));

                } else {
                    device_db_.feature_profile = 
                        DEV_FEATURE_PROFILE_map[pt.get<std::string>(FEATURE_PROFILE_KEY_STR)];
                }
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
            device_db_.mgmt_if_mac = pt.get<std::uint64_t>("mgmt-if-mac");
        }
    }
    return SDK_RET_OK;
}

void
device::populate_device_defaults_(void)
{
    device_db_.fwd_mode = FORWARDING_MODE_CLASSIC;
    device_db_.feature_profile = FEATURE_PROFILE_CLASSIC_DEFAULT;
    device_db_.port_admin_state = PORT_ADMIN_STATE_ENABLE;
    device_db_.mgmt_if_mac = 0;
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
