/*
* Copyright (c) 2018, Pensando Systems Inc.
*/

#ifndef __NICMGR_DEVICE_HPP__
#define __NICMGR_DEVICE_HPP__

#include <string>

#ifndef __FTL_DEV_IMPL_HPP__
#include "pal_compat.hpp"
#endif
#include "logger.hpp"
#include "nicmgr_utils.hpp"

#define VERSION_FILE        "/nic/etc/VERSION.json"

/**
 * Device Types
 */
enum DeviceType
{
    INVALID,
    DEBUG,
    ETH,
    ACCEL,
    NVME,
    VIRTIO,
    PCIESTRESS,
#ifdef ATHENA
    FTL,
#endif
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

std::string oprom_type_to_str(OpromType type);
OpromType str_to_oprom_type(std::string const& s);

/**
 * Base Class for devices
 */
class Device {
    public:
        virtual ~Device() {}
        virtual enum DeviceType GetType() final { return type; }
        virtual void SetType(enum DeviceType type) final { this->type = type; }
    private:
        enum DeviceType type;
};

#endif /* __NICMGR_DEVICE_HPP__ */
