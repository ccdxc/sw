//
// {C} Copyright 2018 Pensando Systems Inc. All rights reserved
//
///----------------------------------------------------------------------------
///
/// \file
/// This module defines Device API
///
///----------------------------------------------------------------------------

#ifndef __INCLUDE_API_DEVICE_HPP__
#define __INCLUDE_API_DEVICE_HPP__

#include "nic/sdk/include/sdk/ip.hpp"
#include "nic/sdk/include/sdk/eth.hpp"
#include "nic/sdk/include/sdk/platform.hpp"
#include "nic/apollo/api/include/pds.hpp"

#define PDS_DROP_REASON_MAX        64    ///< Maximum packet drop reasons
#define PDS_MAX_DROP_NAME_LEN      32    ///< Packet drop reason string length
#define PDS_MAX_DEVICE              1    ///< only one instance of device

using namespace std;

/// \defgroup PDS_DEVICE Device API
/// \@{

///< operational mode of the device
typedef enum pds_device_oper_mode_e {
    PDS_DEV_OPER_MODE_NONE = 0,
    ///< bump-in-the-wire mode with workloads on uplink port(s)
    PDS_DEV_OPER_MODE_BITW = 1,
    ///< host path mode with workloads on pcie
    PDS_DEV_OPER_MODE_HOST = 2,
} pds_device_oper_mode_t;

typedef enum pds_device_profile_e {
    PDS_DEVICE_PROFILE_DEFAULT = 0,
    PDS_DEVICE_PROFILE_2PF     = 1,
    PDS_DEVICE_PROFILE_3PF     = 2,
    PDS_DEVICE_PROFILE_4PF     = 3,
    PDS_DEVICE_PROFILE_5PF     = 4,
    PDS_DEVICE_PROFILE_6PF     = 5,
    PDS_DEVICE_PROFILE_7PF     = 6,
    PDS_DEVICE_PROFILE_8PF     = 7,
} pds_device_profile_t;

typedef enum pds_memory_profile_e {
    PDS_MEMORY_PROFILE_DEFAULT = 0,
} pds_memory_profile_t;

/// \brief device specification
typedef struct pds_device_s {
    /// device IP address
    ip_addr_t              device_ip_addr;
    /// device MAC address
    mac_addr_t             device_mac_addr;
    /// gateway IP address
    ip_addr_t              gateway_ip_addr;
    /// enable or disable forwarding based on L2 entries
    bool                   bridging_en;
    /// enable or disable L2/IP learning
    bool                   learning_en;
    /// MAC, IP aging timeout (in seconds) for learnt entries
    uint32_t               learn_age_timeout;
    /// enable or disable evpn for overlay routing/bridging
    bool                   overlay_routing_en;
    /// device profile
    pds_device_profile_t   device_profile;
    /// memory profile
    pds_memory_profile_t   memory_profile;
    /// device operational mode
    pds_device_oper_mode_t dev_oper_mode;
    /// priority of IP mapping entries (this priority, if configured, will be
    /// used to break the tie in case both LPM/prefix and a mapping entry are
    /// hit in the datapath (i.e., /32 IP mapping entry is also in some LPM
    ///prefix)
    /// NOTE:
    /// 1. by default IP mapping always takes precedence over LPM hit and
    ///    default value of this attribute is 0 (lower the numerical value,
    ///    the higher the priority, hence 0 is the highest priority)
    /// 2. valid priority value range is 0 (highest) to 65535 (lowest)
    uint32_t               ip_mapping_priority;
} __PACK__ pds_device_spec_t;

/// \brief device status
typedef struct pds_device_status_s {
    mac_addr_t  fru_mac;           ///< FRU MAC
    uint8_t     memory_cap;        ///< Memory capacity
    string      mnfg_date;         ///< FRU Manufacturing date
    string      product_name;      ///< FRU Product name
    string      serial_num;        ///< FRU Serial Number
    string      part_num;          ///< FRU Part Number
    string      description;       ///< Description
    string      vendor_id;         ///< Vendor ID
    sdk::platform::asic_type_t chip_type; ///< Chip Type
    string      hardware_revision; ///< Hardware Revision
    string      cpu_vendor;        ///< CPU Vendor
    string      cpu_specification; ///< CPU Specification
    string      fw_version;        ///< Firmware Version
    string      soc_os_version;    ///< SOC OS Version
    string      soc_disk_size;     ///< SOC Disk Size
    string      pcie_specification;///< PCIe Specification
    string      pcie_bus_info;     ///< PCIe Bus Information
    uint32_t    num_pcie_ports;    ///< Number of PCIe ports
    uint32_t    num_ports;         ///< Number of NIC ports
    string      vendor_name;       ///< Vendor Name
    float       pxe_version;       ///< PXE Version
    float       uefi_version;      ///< UEFI Version
    uint32_t    num_host_if;       ///< number of host interfaces
    string      fw_description;    ///< Firmware Description
    string      fw_build_time;     ///< Firmware Build Time
} pds_device_status_t;

/// \brief Drop statistics
typedef struct pds_device_drop_stats_s {
    char     name[PDS_MAX_DROP_NAME_LEN];   ///< drop reason name
    uint64_t count;                         ///< drop count
} __PACK__ pds_device_drop_stats_t;

/// \brief device statistics
typedef struct pds_device_stats_s {
    ///< number of entries in the ingress drop statistics
    uint32_t ing_drop_stats_count;
    ///< number of entries in the egress drop statistics
    uint32_t egr_drop_stats_count;
    ///<< ingress drop statistics
    pds_device_drop_stats_t ing_drop_stats[PDS_DROP_REASON_MAX];
    ///< egress drop statistics
    pds_device_drop_stats_t egr_drop_stats[PDS_DROP_REASON_MAX];
} __PACK__ pds_device_stats_t;

/// \brief device information
typedef struct pds_device_info_s {
    pds_device_spec_t   spec;      ///< specification
    pds_device_status_t status;    ///< status
    pds_device_stats_t  stats;     ///< statistics
} pds_device_info_t;

/// \brief     create device
/// \param[in] spec specification
/// \param[in] bctxt batch context if API is invoked in a batch
/// \return    #SDK_RET_OK on success, failure status code on error
/// \remark    device is a global configuration and can be created only once.
///            Any other validation that is expected on the TEP should be done
///            by the caller
sdk_ret_t pds_device_create(pds_device_spec_t *spec,
                            pds_batch_ctxt_t bctxt = PDS_BATCH_CTXT_INVALID);

/// \brief      read device
/// \param[out] info device information
/// \return     #SDK_RET_OK on success, failure status code on error
sdk_ret_t pds_device_read(pds_device_info_t *info);

/// \brief     update device
/// \param[in] spec specification
/// \param[in] bctxt batch context if API is invoked in a batch
/// \return    #SDK_RET_OK on success, failure status code on error
/// \remark    A valid device specification should be passed
sdk_ret_t pds_device_update(pds_device_spec_t *spec,
                            pds_batch_ctxt_t bctxt = PDS_BATCH_CTXT_INVALID);

/// \brief  delete device
/// \param[in] bctxt batch context if API is invoked in a batch
/// \return #SDK_RET_OK on success, failure status code on error
sdk_ret_t pds_device_delete(pds_batch_ctxt_t bctxt = PDS_BATCH_CTXT_INVALID);

/// @}

#endif    // __INCLUDE_API_PDS_DEVICE_HPP__
