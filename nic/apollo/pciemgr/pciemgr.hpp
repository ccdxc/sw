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

/// \@}

#endif    // __PCIEMGR_HPP__
