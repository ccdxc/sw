#include <boost/algorithm/string.hpp>
#include <boost/format.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/property_tree/ptree.hpp>

#include "gen/proto/device.pb.h"

#include "../utils.hpp"

namespace pt = boost::property_tree;

std::string
get_main_config_file (void)
{
    pt::ptree ptree;
    std::string fwd_mode;
    std::string feature;

    if (access(DEVICE_JSON, R_OK) < 0)
    {
        return DEFAULT_SYSMGR_JSON;
    }

    pt::read_json(DEVICE_JSON, ptree);

    fwd_mode = ptree.get<std::string>("forwarding-mode",
        "FORWARDING_MODE_CLASSIC");
    
    if (fwd_mode == "FORWARDING_MODE_CLASSIC" ||
        fwd_mode == std::to_string(device::FORWARDING_MODE_CLASSIC))
    {
        fwd_mode = "classic";
    }
    else if (fwd_mode == "FORWARDING_MODE_HOSTPIN" ||
        fwd_mode == std::to_string(device::FORWARDING_MODE_HOSTPIN))
    {
        fwd_mode = "hostpin";
    }
    else if (fwd_mode == "FORWARDING_MODE_SWITCH" ||
        fwd_mode == std::to_string(device::FORWARDING_MODE_SWITCH))
    {
        fwd_mode = "switch";
    }
    else
    {
        throw std::runtime_error("Unknown forwarding-mode");
    }

    int feature_profile = ptree.get<int>(
        "feature-profile", 
         device::FEATURE_PROFILE_BASE);
    if (feature_profile == device::FEATURE_PROFILE_NONE ||
	feature_profile == device::FEATURE_PROFILE_BASE ||
	feature_profile == device::FEATURE_PROFILE_STORAGE ||
	feature_profile == device::FEATURE_PROFILE_SRIOV ||
	feature_profile == device::FEATURE_PROFILE_VIRTUALIZED ||
	feature_profile == device::FEATURE_PROFILE_PROXY ||
        feature_profile == device::FEATURE_PROFILE_DEVELOPER) {
	// For all feature profiles naples_hostpin is to be selected
	return DEFAULT_SYSMGR_JSON;
    }

    throw std::runtime_error("unkown feature profile");
}
