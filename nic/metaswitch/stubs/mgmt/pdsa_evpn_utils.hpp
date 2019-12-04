// {C} Copyright 2019 Pensando Systems Inc. All rights reserved

#ifndef __PDSA_EVPN_UTILS_HPP__
#define __PDSA_EVPN_UTILS_HPP__

#include "nic/metaswitch/stubs/mgmt/pdsa_mgmt_utils.hpp"
#include "nic/metaswitch/stubs/mgmt/pds_ms_vpc.hpp"

namespace pdsa_stub {

NBB_VOID pdsa_row_update_evpn_ip_vrf (pds_vpc_id_t   vpc_id, 
                                      pds_vnid_id_t  vni, 
                                      NBB_LONG       row_status, 
                                      NBB_ULONG      correlator);
NBB_VOID pdsa_row_update_evpn_evi (pds_subnet_id_t   evi,
                                   pds_encap_type_t encap_type,
                                   NBB_LONG          row_status,
                                   NBB_ULONG         correlator);
NBB_VOID pdsa_row_update_evpn_bd (pds_subnet_id_t    evi,
                                  pds_vnid_id_t      vni,
                                  NBB_LONG           row_status,
                                  NBB_ULONG          correlator);
NBB_VOID pdsa_row_update_evpn_if_bind_cfg (pds_subnet_id_t   evi,
                                           pds_vnic_id_t     vnic_id,
                                           NBB_LONG          row_status,
                                           NBB_ULONG         correlator);
}
#endif /*__PDSA_EVPN_UTILS_HPP__*/
