//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// netagent external header file
///
//----------------------------------------------------------------------------

#ifndef __NETAGENT_HPP__
#define __NETAGENT_HPP__

#include "nic/sdk/platform/evutils/include/evutils.h"
#include "nic/sdk/lib/venice/venice.hpp"

/// \defgroup PDS_NETAGENT
/// @{

namespace netagent {

class netagentapi {
public:
    /// \brief    factory method to netagentapi instance
    /// \return    new instance of netagentapi or NULL, in case of error
    static netagentapi *factory(void);


    /// \brief    destroy netagentapi instance
    /// \param[in]    netagent netagentapi instance
    static void destroy(netagentapi *netagent);

    /// \brief    netagent thread's entry point
    /// \param[in]    ctxt thread start callback context
    /// \return always NULL
    static void *netagent_thread_start(void *ctxt);

    /// \brief    netagent thread's cleanup point
    /// \param[in]    arg object to be destroyed
    static void netagent_thread_cleanup(void *arg=NULL);

    static void netagent_handle_create_modify_venice_coordinates(void *ctxt);
    static void netagent_handle_delete_venice_coordinates(void *ctxt);
    static void netagent_handle_venice_coordinates(sdk::lib::venice* venice, void *ctxt);

    static void netagent_init();

private:
    /// \brief    constructor
    netagentapi() {}

    /// \brief    destructor
    ~netagentapi() {}

};

}    // namespace netagent

/// \@}

#endif    // __NETAGENT_HPP__
