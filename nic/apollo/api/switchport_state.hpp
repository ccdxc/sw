/**
 * Copyright (c) 2019 Pensando Systems, Inc.
 *
 * @file    switchport_state.hpp
 *
 * @brief   switchport database maintenance
 */

#if !defined (__SWITCHPORT_STATE_HPP__)
#define __SWITCHPORT_STATE_HPP__

#include "nic/apollo/framework/api_base.hpp"
#include "nic/apollo/framework/impl_base.hpp"
#include "nic/apollo/api/switchport.hpp"

namespace api {

/**
 * @defgroup OCI_SWITCHPORT_STATE - switchport functionality
 * @ingroup OCI_SWITCHPORT
 * @{
 */

/**
 * @brief    state maintained for switchports
 */
class switchport_state : public obj_base {
public:
    /**
     * @brief    constructor
     */
    switchport_state() {
        switchport_cfg_ = NULL;
    }

    /**
     * @brief    destructor
     */
    ~switchport_state() {}

    /**
     * @brief    allocate memory required for switchport object
     * @return pointer to the allocated switchport, NULL if no memory
     */
    switchport_entry *switchport_alloc(void);

    /**
     * @brief
     * @return   SDK_RET_OK on success, failure status code on error
     */
    sdk_ret_t insert(switchport_entry *switchport) {
        if (switchport_cfg_) {
            return sdk::SDK_RET_ENTRY_EXISTS;
        }
        switchport_cfg_ = switchport;
        return SDK_RET_OK;
    }

    /**
     * @brief      free switchport instance back to slab
     * @param[in]  switchport   pointer to the allocated switchport
     */
    void switchport_free(switchport_entry *switchport);

    /**
     * @brief     lookup a switchport in database given the key
     * @param[in] switchport_key switchport key
     */
    switchport_entry *switchport_find(void) {
        return switchport_cfg_;
    }

private:
    switchport_entry    *switchport_cfg_;    /**< user provided config */
};

/** * @} */    // end of OCI_SWITHCPORT_STATE

}    // namespace api

using api::switchport_state;

#endif    /** __SWITCHPORT_STATE_HPP__ */
