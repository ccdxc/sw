// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
// Initialize Stubs that drive the dataplane

#include "pdsa_hal_init.hpp"
#include "pdsa_li.hpp"
#include "pdsa_l2f.hpp"

void pdsa_hal_init ()
{
    pdsa_stub::LiIntegSubcomponent pli;
    /* LI stub implementation instance needs to be registered with Metaswitch*/
    li_pen::is_initialize (&pli);
}
