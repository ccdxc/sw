// {C} Copyright 2020 Pensando Systems Inc. All rights reserved
// Metaswitch Table Index constants

#ifndef __PDS_MS_TBL_IDX__
#define __PDS_MS_TBL_IDX__


#define PDS_MS_DEFAULT_VRF_ID     0
#define PDS_MS_BGP_RM_ENT_INDEX   1
#define PDS_MS_BGP_NM_ENT_INDEX   1
#define PDS_MS_SCK_ENT_INDEX      1
#define PDS_MS_I3_ENT_INDEX       1
#define PDS_MS_LI_ENT_INDEX       1
#define PDS_MS_LIM_ENT_INDEX      1
#define PDS_MS_L2F_ENT_INDEX      1
#define PDS_MS_NAR_ENT_INDEX      1
#define PDS_MS_NRM_ENT_INDEX      1
#define PDS_MS_RTM_DEF_ENT_INDEX  1 // Default VRF
#define PDS_MS_PSM_ENT_INDEX      1
#define PDS_MS_SMI_ENT_INDEX      1
#define PDS_MS_FTM_ENT_INDEX      1
#define PDS_MS_FT_ENT_INDEX       1
#define PDS_MS_HALS_ENT_INDEX     1
#define PDS_MS_EVPN_ENT_INDEX     2 // Use a different entity id than BGP
                                    // Required to redistribute routes to EVPN
                                    // (Type 5 test) 
#define PDS_MS_BGP_ROUTE_MAP_DEF_INDEX  1
#define PDS_MS_BGP_ROUTE_MAP_DEF_NUBMER 1

#endif
