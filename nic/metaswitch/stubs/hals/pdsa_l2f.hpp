//---------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
// Class that implements the Metaswitch L2F stub integration interface
//---------------------------------------------------------------

#ifndef __PDSA_L2F_HPP__
#define __PDSA_L2F_HPP__

// This is to avoid including the Metaswitch definition of TestIntegSubcomponent
// class that is supposed to implement the L2F IntegrationSubcomponent interface
// but is actually only a stub.
#define L2F_TEST_INTEG_SUBCOMP_HPP_INC

#include "l2f_integ_subcomp.hpp"
#include "l2f_integ_api.hpp"

namespace pdsa_stub {

// Class that implements the L2F Integration subcomponent interface 
class L2fIntegSubcomponent: public l2f::IntegrationSubcomponent {
public:
    void add_upd_bd(ATG_BDPI_UPDATE_BD *update_bd);

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

}

namespace l2f {
using TestIntegSubcomponent = pdsa_stub::L2fIntegSubcomponent;
}

#endif
