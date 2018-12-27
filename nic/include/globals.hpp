// {C} Copyright 2018 Pensando Systems Inc. All rights reserved

#ifndef __GLOBALS_HPP__
#define __GLOBALS_HPP__

// interface id ranges
#define HAL_IF_ID_MIN                      1      // HAL interface id range min
#define HAL_IF_ID_MAX                      127    // HAL interface id range max
#define NETAGENT_IF_ID_UPLINK_MIN          128    // netagent uplink interface id range min
#define NETAGENT_IF_ID_UPLINK_MAX          256    // netagent uplink interface id range max
#define IF_ID_ENIC_MIN                     257    // enic interface id range min

// lif hw id ranges
#define HAL_LIF_ID_UPLINK_MIN              1      // uplink lif id range min
#define HAL_LIF_ID_UPLINK_MAX              32     // uplink lif id range max
#define HAL_LIF_ID_SVC_LIF_MIN             33     // service lif id range min
#define HAL_LIF_ID_SVC_LIF_MAX             64     // service lif id range max
#define HAL_LIF_ID_NICMGR_MIN              65     // nicmgr lif id range min
#define HAL_LIF_ID_NICMGR_ADMINQ           HAL_LIF_ID_NICMGR_MIN    // nicmgr's adminq

// vrf id ranges
#define HAL_VRF_ID_MIN                     1      // HAL vrf id range min
#define HAL_VRF_ID_MAX                     16     // HAL vrf id range max
#define NICMGR_VRF_ID_MIN                  17     // nicmgr vrf id range min
#define NICMGR_VRF_ID_MAX                  64     // nicmgr vrf id range max
#define NETAGENT_VRF_ID_MIN                65     // netagent vrf id range min

// network security profile id ranges
#define HAL_NWSEC_PROFILE_ID_MIN           0      // HAL security profile id min
#define HAL_NWSEC_PROFILE_ID_MAX           15     // HAL security profile id max
#define NETAGENT_NWSEC_PROFILE_ID_MIN      16     // netagent security profile id min
#define NETAGENT_NWSEC_PROFILE_ID_MAX      255    // netagent security profile id max

#endif    // __GLOBALS_HPP__
