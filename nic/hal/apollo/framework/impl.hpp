/**
 * Copyright (c) 2018 Pensando Systems, Inc.
 *
 * @file    impl.hpp
 *
 * @brief   basic types for impl classes
 */

#if !defined (__IMPL_HPP__)
#define __IMPL_HPP__

namespace impl {

/**
 * @brief    impl object identifiers
 */
typedef enum impl_obj_id_e {
    IMPL_OBJ_ID_NONE,
    IMPL_OBJ_ID_SWITCHPORT,
    IMPL_OBJ_ID_VCN,
    IMPL_OBJ_ID_SUBNET,
    IMPL_OBJ_ID_TEP,
    IMPL_OBJ_ID_VNIC,
    IMPL_OBJ_ID_ROUTE_TABLE,
    IMPL_OBJ_ID_SECURITY_RULES,
    IMPL_OBJ_ID_MAPPING,
    IMPL_OBJ_ID_MAX,
} impl_obj_id_t;

}    // namespace impl

using impl::impl_obj_id_t;

#endif    /** __IMPL_HPP__ */
