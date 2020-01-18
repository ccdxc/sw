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
#define HAL_LIF_CPU                        33     // cpu lif
#define HAL_LIF_ID_SVC_LIF_MIN             34     // service lif id range min
#define HAL_LIF_ID_SVC_LIF_MAX             64     // service lif id range max
#define HAL_LIF_ID_MIN                     (HAL_LIF_ID_UPLINK_MIN)
#define HAL_LIF_ID_MAX                     (HAL_LIF_ID_SVC_LIF_MAX)

// nicmgr lifs
#define NICMGR_NCSI_LIF_MIN                65     // NCSI LIF
#define NICMGR_NCSI_LIF_MAX                66     // NCSI LIF
#define NICMGR_SVC_LIF                     67     // nicmgr service lif
#define NICMGR_LIF_MAX                     2047   // nicmgr lif id range max

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

// l2segment if ranges
#define NETAGENT_L2SEG_ID_MIN              16

#define SERVICE_LIF_START                       HAL_LIF_ID_SVC_LIF_MIN
#define HAL_PROXY_MAX_ST_LIF_PER_SVC            5
#define HAL_PROXY_MAX_QTYPE_PER_LIF             8

enum {
    SERVICE_LIF_APOLLO = SERVICE_LIF_START,
    SERVICE_LIF_TCP_PROXY,
    SERVICE_LIF_TLS_PROXY,
    SERVICE_LIF_IPSEC_ESP,
    SERVICE_LIF_IPSEC_AH,
    SERVICE_LIF_IPFIX,
    SERVICE_LIF_APP_REDIR,
    SERVICE_LIF_GC,
    SERVICE_LIF_P4PT,
    SERVICE_LIF_CPU_BYPASS,
    SERVICE_LIF_END
};


#endif    // __GLOBALS_HPP__
