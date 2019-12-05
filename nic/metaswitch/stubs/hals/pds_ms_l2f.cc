//---------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
// PDS Implementation of Metaswitch L2F stub integration
//---------------------------------------------------------------

#include <l2f_c_includes.hpp>
#include "nic/metaswitch/stubs/hals/pds_ms_l2f.hpp"

namespace pds_ms {

// Class that implements the L2F Integration subcomponent interface 
void l2f_integ_subcomp_t::add_upd_bd(ATG_BDPI_UPDATE_BD *update_bd_ips)
{
}

void l2f_integ_subcomp_t::delete_bd(const ATG_L2_BD_ID *bd_id,
                                    NBB_CORRELATOR dp_bd_correlator)
{
}

void l2f_integ_subcomp_t::add_upd_bd_if(const ATG_L2_BD_ID *bd_id,
                                        NBB_CORRELATOR dp_bd_correlator,
                                        NBB_LONG bd_service_type,
                                        ATG_BDPI_INTERFACE_BIND *if_bind,
                                        const l2f::BdpiSlaveJoin &join)
{
}

void l2f_integ_subcomp_t::delete_bd_if(const ATG_L2_BD_ID *bd_id,
                                       NBB_CORRELATOR dp_bd_correlator,
                                       ATG_BDPI_INTERFACE_BIND *if_bind)
{
}

} // End namespace
