
#include <string>

#include "device.hpp"


std::string
oprom_type_to_str(OpromType type) {
    switch (type) {
        CASE(OPROM_UNKNOWN);
        CASE(OPROM_LEGACY);
        CASE(OPROM_UEFI);
        CASE(OPROM_UNIFIED);
    default:
        return "unknown";
    }
}

OpromType
str_to_oprom_type(std::string const& s) {
    if (s == "legacy") {
        return OPROM_LEGACY;
    } else if (s == "uefi") {
        return OPROM_UEFI;
    } else if (s == "unified") {
        return OPROM_UNIFIED;
    } else {
        NIC_LOG_ERR("Unknown OPROM type: {}", s);
        return OPROM_UNKNOWN;
    }
}
