/**
 * Copyright (c) 2018 Pensando Systems, Inc.
 *
 * @file    mem.hpp
 *
 * @brief   This file captures OCI memory related helpers
 */

#if !defined (__MEM_HPP_)
#define __MEM_HPP_

#include "nic/sdk/include/sdk/base.hpp"

namespace api {

/** unique slab identifiers */
enum {
    OCI_SLAB_ID_MIN = 0,
    OCI_SLAB_ID_API_PARAMS = OCI_SLAB_ID_MIN,
    OCI_SLAB_ID_SWITCHPORT,
    OCI_SLAB_ID_TEP,
    OCI_SLAB_ID_VCN,
    OCI_SLAB_ID_SUBNET,
    OCI_SLAB_ID_VNIC,
    OCI_SLAB_ID_MAPPING,
    OCI_SLAB_ID_ROUTE_TABLE,
    OCI_SLAB_ID_MAX = 16383
};

enum {
    OCI_MEM_ALLOC_SWITCHPORT,
    OCI_MEM_ALLOC_MAX = 16383,
};

#define OCI_DELAY_DELETE_MSECS       (TIME_MSECS_PER_SEC << 1)

/**
 * @brief wrapper function to delay delete slab elements
 *
 * @param[in]     slab_id    identifier of the slab to free the element to
 * @param[int]    elem       element to free to the given slab
 * @return #SDK_RET_OK on success, failure status code on error
 *
 * NOTE: currently delay delete timeout is OCI_DELAY_DELETE_MSECS, it is
 *       expected that other thread(s) using (a pointer to) this object should
 *       be done using this object within this timeout or else this memory can
 *       be freed and allocated for other objects and can result in corruptions.
 *       Essentially, OCI_DELAY_DELETE is assumed to be infinite
 */
sdk_ret_t delay_delete_to_slab(uint32_t slab_id, void *elem);

}    // namespace api

#endif /** __MEM_HPP_ */
