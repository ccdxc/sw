//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// netagent functionality
///
//----------------------------------------------------------------------------

#include <pthread.h>
#include "include/sdk/base.hpp"
#include "nic/sdk/platform/evutils/include/evutils.h"
#include "nic/apollo/core/trace.hpp"
#include "nic/apollo/netagent/netagent.hpp"
#include "nic/apollo/core/event.hpp"
#include "nic/apollo/core/core.hpp"

/// \defgroup PDS_NETAGENT
/// @{

namespace netagent {

void *
netagentapi::netagent_thread_start(void *ctxt) {
    EV_P;
    char config_file[] = "/sysconfig/config0/naplesStatus.json";

    SDK_THREAD_INIT(ctxt);
    EV_A = evutil_create_loop();

    PDS_TRACE_INFO("Initializing netagent thread...");

    pthread_cleanup_push(netagent::netagentapi::netagent_thread_cleanup, NULL);

#ifdef APOLLO
#ifdef __aarch64__
    // initialize stats timer to update stats to delphi
    PDS_TRACE_INFO("evutil_stat_start...");
    static evutil_stat evu_stat;
    evutil_stat_start(EV_A_
                      &evu_stat,
                      config_file,
                      netagent::netagentapi::netagent_handle_create_modify_venice_coordinates,
                      NULL, 
                      netagent::netagentapi::netagent_handle_delete_venice_coordinates,
                      NULL);
#endif
#endif

    netagent_init();

    PDS_TRACE_INFO("Listening to events ...");
    evutil_run(EV_A);
    pthread_cleanup_pop(1);
   
   return NULL;
}

void
netagentapi::netagent_init() {

}

void
netagentapi::netagent_handle_venice_coordinates(sdk::lib::venice* ven, void *ctxt) {
    if (((ven->venice_db_.naplesMode == "2") ||
        (ven->venice_db_.naplesMode == "3")) &&
        (!ven->venice_db_.controllers.empty())) {
        PDS_TRACE_DEBUG("going to setup connection");
    }
}

void
netagentapi::netagent_handle_create_modify_venice_coordinates(void *ctxt) {
    PDS_TRACE_DEBUG("venice co-ordinates changed!!");
    static sdk::lib::venice* ven = NULL;
    if (ven == NULL) {
        ven = sdk::lib::venice::factory();
    } else {
        string naples_status_file = ven->get_naples_status_file();
        ven->init(naples_status_file);
        ven->dump_venice_coordinates();
    }
    netagent_handle_venice_coordinates(ven, ctxt);
}

void
netagentapi::netagent_handle_delete_venice_coordinates(void *ctxt) {
    PDS_TRACE_DEBUG("venice co-ordinates deleted!!");
}
//------------------------------------------------------------------------------
// netagent thread cleanup
//------------------------------------------------------------------------------
void
netagentapi::netagent_thread_cleanup (void *arg) {
    PDS_TRACE_DEBUG("netagent_thread_cleanup got called");
}


}    // namespace netagent

/// \@}

