//------------------------------------------------------------------------------
// {C} Copyright 2020 Pensando Systems Inc. All rights reserved
// -----------------------------------------------------------------------------

#ifndef __AGENT_CORE_EVENT_HPP__
#define __AGENT_CORE_EVENT_HPP__

#include "nic/sdk/include/sdk/base.hpp"
#include "nic/apollo/api/include/pds_event.hpp"
#include "nic/apollo/api/include/pds_upgrade.hpp"

/// \brief event operation
typedef enum pds_event_op_e {
    PDS_EVENT_OP_NONE,
    PDS_EVENT_OP_SUBSCRIBE,
    PDS_EVENT_OP_UNSUBSCRIBE,
} pds_event_op_t;

/// \brief event subscribe/unsubscribe request from app(s)
typedef struct pds_event_spec_s {
    /// event of interest
    pds_event_id_t event_id;
    /// operation - SUBSCRIBE or UNSUBSCRIBE
    pds_event_op_t event_op;
} __PACK__ pds_event_spec_t;

namespace core {

/// \brief handle the incoming event subscribe/unsubscribe event
/// \param[in] spec    pointer to the event spec
/// \param[in] ctxt    client specific context
/// \return    SDK_RET_OK on success, failure status code on error
sdk_ret_t handle_event_request(const pds_event_spec_t *spec, void *ctxt);

/// \brief update the state of the event listener in the event manager
/// \param[in] ctxt    client specific context
/// \return    SDK_RET_OK on success, failure status code on error
sdk_ret_t update_event_listener(void *ctxt);

/// \brief handle event notifications received (from PDS HAL)
/// \param[in] event    event being notified
/// \return    SDK_RET_OK on success, failure status code on error
sdk_ret_t handle_event_ntfn(const pds_event_t *event);

}    // namespace core

#endif    // __AGENT_CORE_EVENT_HPP__
