/*
* Copyright (c) 2018, Pensando Systems Inc.
*/

#ifndef __NICMGR_DEVICE_HPP__
#define __NICMGR_DEVICE_HPP__

#include <string>

#include "pal_compat.hpp"
#include "logger.hpp"
#include "nicmgr_utils.hpp"

#define VERSION_FILE        "/nic/etc/VERSION.json"

/**
 * Device Types
 */
typedef enum DeviceType
{
    INVALID,
    DEBUG,
    ETH,
    ACCEL,
    NVME,
    VIRTIO,
    PCIESTRESS,
    FTL,
    DEVICETYPE_MAX,
    DEVICETYPE_FIRST = DEBUG
} DeviceType;

typedef enum BusType_s
{
    BUS_TYPE_PCIE_PF,
    BUS_TYPE_PCIE_VF,
    BUS_TYPE_PLATFORM,
} BusType;

typedef enum DevTrustType {
    DEV_UNTRUSTED = 0,
    DEV_TRUSTED = 1,
} DevTrustType;

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
        
        virtual DeviceType GetType() final { return type; }
        virtual void SetType(DeviceType type) final { this->type = type; }
    
        BusType GetBusType() { return bus_type; }
        void SetBusType(BusType bus_type) { this->bus_type = bus_type; }

        DevTrustType GetTrustType() { return trust_type; }
        void SetTrustType(DevTrustType trust_type) { this->trust_type = trust_type; }

    protected:
        DeviceType type;
	    BusType bus_type;
	    DevTrustType trust_type;
};

#endif /* __NICMGR_DEVICE_HPP__ */
