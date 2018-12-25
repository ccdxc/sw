/**
 * Copyright (c) 2018 Pensando Systems, Inc.
 *
 * @file    mapping_impl_state.hpp
 *
 * @brief   MAPPING implementation state
 */
#if !defined (__MAPPING_IMPL_STATE_HPP__)
#define __MAPPING_IMPL_STATEHPP__

#include "nic/hal/apollo/framework/api_base.hpp"
#include "nic/sdk/lib/table/directmap/directmap.hpp"

namespace impl {

/**
 * @defgroup OCI_MAPPING_IMPL_STATE - mapping state functionality
 * @ingroup OCI_MAPPING
 * @{
 */

 /**< forward declaration */
class mapping_impl;

/**
 * @brief    state maintained for mappings
 */
class mapping_impl_state : public obj_base {
public:
    /**
     * @brief    constructor
     */
    mapping_impl_state();

    /**
     * @brief    destructor
     */
    ~mapping_impl_state();

private:
    friend class mapping_impl;   /**< mapping_impl class is friend of mapping_impl_state */

private:
};

/** * @} */    // end of OCI_MAPPING_IMPL_STATE

}    // namespace impl

#endif    /** __MAPPING_IMPL_STATE_HPP__ */
