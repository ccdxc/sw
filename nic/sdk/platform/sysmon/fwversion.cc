/*
 * Copyright (c) 2019, Pensando Systems Inc.
 */

#include "sysmon_internal.hpp"

#define FW_VERSION_FILE "/nic/etc/VERSION.json"
#define SW_VERSION_KEY "sw.version"
#define VERSION_PATTERN "%d %*[.] %d %*[.] %d %*[-] %c"

void
updatefwversion(void)
{
    boost::property_tree::ptree input;
    std::string version;
    uint32_t major_version;
    uint32_t minor_version;
    uint32_t maintenance_version;
    char pipeline;
    uint32_t pipeline_version;
    int ret;

    try {
        boost::property_tree::read_json(FW_VERSION_FILE, input);
    }
    catch (std::exception const &ex) {
        SDK_TRACE_DEBUG("%s",ex.what());
        return;
    }

    if (input.get_optional<std::string>(SW_VERSION_KEY)) {
        version = input.get<std::string>(SW_VERSION_KEY);
    } else {
        SDK_TRACE_ERR("sw version not found in %s", FW_VERSION_FILE);
        return;
    }
    SDK_TRACE_INFO("version is %s", version.c_str());
    ret = sscanf(version.c_str(), VERSION_PATTERN, &major_version,
                 &minor_version, &maintenance_version, &pipeline);
    if (ret == 4) {
        // Encode pipeline
        if (isalpha(pipeline)) {
            pipeline_version = toupper(pipeline) - 'A' + 1;
        } else {
            // Special case - dev builds
            pipeline_version = 0;
        }
        pal_write_fw_ver_to_cpld(major_version, minor_version,
                                 maintenance_version, pipeline_version);
    } else {
        SDK_TRACE_ERR("Error getting firmware version");
        return;
    }
}
