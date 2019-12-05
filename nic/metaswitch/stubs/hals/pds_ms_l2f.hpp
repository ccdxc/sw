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
};

} // End namespace

#endif
