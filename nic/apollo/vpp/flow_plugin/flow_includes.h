//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//

#ifndef __VPP_FLOW_PLUGIN_FLOW_INCLUDES_H__
#define __VPP_FLOW_PLUGIN_FLOW_INCLUDES_H__

#include <arpa/inet.h>
#include "flow.h"
#include <nic/utils/ftl/ftl_structs.hpp>

#ifdef APOLLO

#include <nic/apollo/p4/include/defines.h>
#include "p4_cpu_hdr_apollo.h"
#include "flow_apollo.h"

#define VPP_CPU_FLAGS_VLAN_VALID           APOLLO_CPU_FLAGS_VLAN_VALID
#define VPP_CPU_FLAGS_IPV4_1_VALID         APOLLO_CPU_FLAGS_IPV4_1_VALID
#define VPP_CPU_FLAGS_IPV6_1_VALID         APOLLO_CPU_FLAGS_IPV6_1_VALID
#define VPP_CPU_FLAGS_ETH_2_VALID          APOLLO_CPU_FLAGS_ETH_2_VALID
#define VPP_CPU_FLAGS_IPV4_2_VALID         APOLLO_CPU_FLAGS_IPV4_2_VALID
#define VPP_CPU_FLAGS_IPV6_2_VALID         APOLLO_CPU_FLAGS_IPV6_2_VALID
#define VPP_CPU_FLAGS_DIRECTION            APOLLO_CPU_FLAGS_DIRECTION
#define VPP_PREDICATE_HDR_SZ               APOLLO_PREDICATE_HDR_SZ
#define VPP_P4_TO_ARM_HDR_SZ               APOLLO_P4_TO_ARM_HDR_SZ

#elif ARTEMIS

#include <nic/apollo/p4/include/artemis_defines.h>
#include "p4_cpu_hdr_artemis.h"
#include "flow_artemis.h"

#define VPP_CPU_FLAGS_VLAN_VALID           ARTEMIS_CPU_FLAGS_VLAN_VALID
#define VPP_CPU_FLAGS_IPV4_1_VALID         ARTEMIS_CPU_FLAGS_IPV4_1_VALID
#define VPP_CPU_FLAGS_IPV6_1_VALID         ARTEMIS_CPU_FLAGS_IPV6_1_VALID
#define VPP_CPU_FLAGS_ETH_2_VALID          ARTEMIS_CPU_FLAGS_ETH_2_VALID
#define VPP_CPU_FLAGS_IPV4_2_VALID         ARTEMIS_CPU_FLAGS_IPV4_2_VALID
#define VPP_CPU_FLAGS_IPV6_2_VALID         ARTEMIS_CPU_FLAGS_IPV6_2_VALID
#define VPP_CPU_FLAGS_DIRECTION            ARTEMIS_CPU_FLAGS_DIRECTION
#define VPP_PREDICATE_HDR_SZ               ARTEMIS_PREDICATE_HDR_SZ
#define VPP_P4_TO_ARM_HDR_SZ               ARTEMIS_P4_TO_ARM_HDR_SZ

#endif

#endif  //__VPP_FLOW_PLUGIN_FLOW_INCLUDES_H__
