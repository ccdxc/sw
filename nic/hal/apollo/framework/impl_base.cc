/**
 * Copyright (c) 2018 Pensando Systems, Inc.
 *
 * @file    impl_base.cc
 *
 * @brief   base object definition for all impl objects
 */

#include "nic/hal/apollo/framework/impl.hpp"
#include "nic/hal/apollo/framework/impl_base.hpp"
#include "nic/hal/apollo/include/api/oci_switchport.hpp"
#include "nic/hal/apollo/api/impl/switchport_impl.hpp"    // TODO: think how to accomodate a new impl
#include "nic/hal/apollo/api/impl/tep_impl.hpp"

namespace impl {

/**
 * @brief        factory method to instantiate an impl object
 * @param[in]    impl    object id
 * @param[in]    args    args (not interpreted by this class)
 */
impl_base *
impl_base::factory(impl_obj_id_t obj_id, void *args) {
    switch (obj_id) {
    case IMPL_OBJ_ID_SWITCHPORT:
        return switchport_impl::factory((oci_switchport_t *)args);

    default:
        break;
    }
    return NULL;
}

/**
 * @brief    release all the resources associated with this object
 *           and free the memory
 * @param[in] impl_obj    impl instance to be freed
 */
void
impl_base::destroy(impl_obj_id_t obj_id, impl_base *impl) {
    switch (obj_id) {
    case IMPL_OBJ_ID_SWITCHPORT:
        return switchport_impl::destroy((switchport_impl *)impl);

    case IMPL_OBJ_ID_TEP:
        return tep_impl::destroy((tep_impl *)impl);

    default:
        break;
    }
}

}    // namespace impl
