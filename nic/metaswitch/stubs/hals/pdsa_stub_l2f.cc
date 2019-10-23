// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
// Implementation of Metaswitch L2F stub integration

#include "pdsa_stub_l2f.hpp"

namespace pdsa_stub {

// Class that implements the L2F Integration subcomponent interface 
void L2fIntegSubcomp::add_upd_bd (ATG_BDPI_UPDATE_BD *update_bd)
{
}

void L2fIntegSubcomp::delete_bd (const ATG_L2_BD_ID *bd_id,
                                         NBB_CORRELATOR dp_bd_correlator)
{
}

void L2fIntegSubcomp::add_upd_bd_if (const ATG_L2_BD_ID *bd_id,
                                             NBB_CORRELATOR dp_bd_correlator,
                                             NBB_LONG bd_service_type,
                                             ATG_BDPI_INTERFACE_BIND *if_bind,
                                             const l2f::BdpiSlaveJoin &join)
{
}

void L2fIntegSubcomp::delete_bd_if (const ATG_L2_BD_ID *bd_id,
                                            NBB_CORRELATOR dp_bd_correlator,
                                            ATG_BDPI_INTERFACE_BIND *if_bind)
{
}

}
