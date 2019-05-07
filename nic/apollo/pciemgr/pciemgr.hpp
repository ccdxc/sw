//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// pciemgr external header file
///
//----------------------------------------------------------------------------

#ifndef __PCIEMGR_HPP__
#define __PCIEMGR_HPP__

#include "nic/sdk/platform/evutils/include/evutils.h"
#include "nic/sdk/platform/pciemgr/include/pciemgr.h"

/// \defgroup PDS_PCIEMGR
/// @{

namespace pdspciemgr {

class pciemgrapi {
public:
    /// \brief    factory method to pciemgrapi instance
    /// \return    new instance of pciemgrapi or NULL, in case of error
    static pciemgrapi *factory(void);


    /// \brief    destroy pciemgrapi instance
    /// \param[in]    pciemgr pciemgrapi instance
    static void destroy(pciemgrapi *pciemgr);

    /// \brief    pciemgr thread's entry point
    /// \param[in]    ctxt thread start callback context
    /// \return always NULL
    static void *pciemgr_thread_start(void *ctxt);

private:
    /// \brief    constructor
    pciemgrapi() {}

    /// \brief    destructor
    ~pciemgrapi() {}

};

}    // namespace pciemgr

#if 0
typedef struct pciemgrenv_s {
    u_int8_t interactive:1;
    u_int8_t gold:1;
    u_int8_t reboot_on_hostdn:1;
    u_int8_t poll_port:1;
    u_int8_t poll_dev:1;
    u_int8_t enabled_ports;
    pciemgr_params_t params;
} pciemgrenv_t;

int intr_init(void);
#endif


/// \@}

#endif    // __PCIEMGR_HPP__
