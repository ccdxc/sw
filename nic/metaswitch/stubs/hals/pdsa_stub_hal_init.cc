// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
// Initialize Stubs that drive the dataplane

#include "pdsa_stub_hal_init.hpp"
#include "pdsa_stub_li.hpp"
#include "pdsa_stub_l2f.hpp"

void pdsa_stub_hal_init ()
{
    pdsa_stub::LiIntegSubcomp pli;
    /* LI stub implementation instance needs to be registered with Metaswitch*/
    li_pen::is_initialize (&pli);
}
