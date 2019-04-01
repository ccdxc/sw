/*
* Copyright (c) 2018, Pensando Systems Inc.
*/

#ifndef __DEVICE_HPP__
#define __DEVICE_HPP__

#include "pal_compat.hpp"
#include "logger.hpp"

#define VERSION_FILE        "/nic/etc/VERSION.json"

/**
 * Device Types
 */
enum DeviceType
{
    INVALID,
    MNIC,
    DEBUG,
    ETH,
#ifdef IRIS
    ACCEL,
#endif //IRIS
    NVME,
    VIRTIO,
};

/**
 * OPROM
 */
typedef enum OpromType_s {
    OPROM_UNKNOWN,
    OPROM_LEGACY,
    OPROM_UEFI,
    OPROM_UNIFIED
} OpromType;

typedef struct dev_cmd_db_s {
    uint32_t    v;
} dev_cmd_db_t;

/**
 * Base Class for devices
 */
class Device {
    public:
        virtual ~Device() {}
        enum DeviceType GetType() { return type; }
        void SetType(enum DeviceType type) { this->type = type;}
        static OpromType oprom_type_str_to_type(std::string const& s)
        {
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
        virtual void DelphiMountEventHandler(bool mounted) {}
    private:
        enum DeviceType type;
};

#endif /* __DEVICE_HPP__ */
