//---------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
// PDS Metaswitch L2F stub integration subcomponent APIs
//---------------------------------------------------------------

#ifndef __PDS_MS_L2F_HPP__
#define __PDS_MS_L2F_HPP__

#include <l2f_integ_subcomp.hpp>

namespace pds_ms {

// PDSA Class that implements the L2F Integration subcomponent interface 
class l2f_integ_subcomp_t: public l2f::IntegrationSubcomponent {
public:
    void add_upd_bd(ATG_BDPI_UPDATE_BD *update_bd_ips);

    void delete_bd(const ATG_L2_BD_ID *bd_id,
                   NBB_CORRELATOR dp_bd_correlator);

    void add_upd_bd_if(const ATG_L2_BD_ID *bd_id,
                       NBB_CORRELATOR dp_bd_correlator,
                       NBB_LONG bd_service_type,
                       ATG_BDPI_INTERFACE_BIND *if_bind,
                       const l2f::BdpiSlaveJoin &join);

    void delete_bd_if(const ATG_L2_BD_ID *bd_id,
                      NBB_CORRELATOR dp_bd_correlator,
                      ATG_BDPI_INTERFACE_BIND *if_bind);

    void add_upd_fdb_mac(ATG_BDPI_UPDATE_FDB_MAC *update_fdb_mac); 
    void delete_fdb_mac(l2f::FdbMacKey *key) ;
    NBB_ULONG add_upd_mac_ip(ATG_MAI_MAC_IP_ID *mac_ip_id,
                                     NBB_BYTE sticky);

    void delete_mac_ip(const ATG_MAI_MAC_IP_ID *mac_ip_id,
                               bool programmed);
    NBB_BYTE add_upd_vrf_arp_entry(const ATG_MAI_MAC_IP_ID *mac_ip_id,
                                           const char *vrf_name);

    void delete_vrf_arp_entry(const ATG_MAI_MAC_IP_ID *mac_ip_id,
                                      const char *vrf_name);


};

} // End namespace

#endif
