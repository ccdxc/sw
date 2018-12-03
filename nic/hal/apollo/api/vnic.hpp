/**
 * Copyright (c) 2018 Pensando Systems, Inc.
 *
 * @file    vnic.hpp
 *
 * @brief   This file deals with OCI VNIC functionality
 */

#if !defined (__VNIC_HPP__)
#define __VNIC_HPP__

#include "nic/hal/apollo/api/api.hpp"
#include "nic/hal/apollo/include/api/oci_vnic.hpp"
#include "nic/sdk/lib/table/directmap/directmap.hpp"
#include "nic/sdk/lib/table/hash/hash.hpp"

using sdk::table::directmap;

namespace api {

/**
 * @defgroup OCI_VNIC - Internal VNIC
 * @{
 */

/**
 * @brief Internal VNIC structure
 */
typedef struct vnic_s {
    oci_vnic_key_t    key;        /**< VNIC Key */
    ht_ctxt_t         ht_ctxt;    /**< Hash table context */

    /**< P4 datapath specific state */
    uint32_t          hw_id;      /**< hardware ID */
} __PACK__ vnic_t;

/**
 * @brief state maintained for VNICs
 */
class vnic_state {
public:
    /**
     * constructor
     */
    vnic_state();

    /**
     * destructor
     */
    ~vnic_state();

    /**
     * @brief handle VNIC create message
     *
     * @param[in] vnic VNIC information
     * @return #SDK_RET_OK on success, failure status code on error
     */
    sdk_ret_t vnic_create(_In_ oci_vnic_t *oci_vnic);

    /**
     * @brief handle VNIC delete API
     *
     * @param[in] vnic_key VNIC key information
     * @return #SDK_RET_OK on success, failure status code on error
     */
    sdk_ret_t vnic_delete(_In_ oci_vnic_key_t *vnic_key);

private:
    /**
     * @brief add VNIC to database
     *
     * @param[in] vnic VNIC
     */
    sdk_ret_t vnic_add_to_db(vnic_t *vnic);

    /**
     * @brief delete VNIC from database
     *
     * @param[in] vnic_key VNIC key
     */
    vnic_t *vnic_del_from_db(oci_vnic_key_t *vnic_key);

    /**
     * @brief lookup VNIC in database
     *
     * @param[in] vnic_key VNIC key
     */
    vnic_t *vnic_find(oci_vnic_key_t *vnic_key) const;

    /**
     * @brief allocate VNIC structure
     *
     * @return Pointer to the allocated VNIC, NULL if no memory
     */
    vnic_t *vnic_alloc(void);

    /**
     * @brief initialize VNIC structure
     *
     * @param[in] vnic VNIC structure to store the state
     * @param[in] oci_vnic VNIC specific information
     */
    sdk_ret_t vnic_init(vnic_t *vnic, oci_vnic_t *oci_vnic);

    /**
     * @brief allocate and initialize VNIC structure
     *
     * @return Pointer to the allocated and initialized VNIC,
     *         NULL if no memory
     */
    vnic_t *vnic_alloc_init(oci_vnic_t *oci_vnic);

    /**
     * @brief Cleanup state maintained for given VNIC
     *
     * @param[in] vnic VNIC
     */
    void vnic_cleanup(vnic_t *vnic);

    /**
     * @brief free VNIC structure
     *
     * @param[in] vnic VNIC
     */
    void vnic_free(vnic_t *vnic);

    /**
     * @brief uninitialize and free VNIC structure
     *
     * @param[in] vnic VNIC
     */
    void vnic_delete(_In_ vnic_t *vnic);

    /**
     * @brief helper function to get key given VNIC entry
     */
    static void *vnic_key_func_get(void *entry) {
        vnic_t *vnic = (vnic_t *)entry;
        return (void *)&(vnic->key);
    }

    /**
     * @brief helper function to compute hash value for given VNIC id
     */
    static uint32_t vnic_hash_func_compute (void *key, uint32_t ht_size) {
        return hash_algo::fnv_hash(key, sizeof(oci_vnic_key_t)) % ht_size;
    }

    /**
     * @brief helper function to compare two VNIC keys
     */
    static bool vnic_key_func_compare (void *key1, void *key2) {
        SDK_ASSERT((key1 != NULL) && (key2 != NULL));
        if (!memcmp(key1, key2, sizeof(oci_vnic_key_t)))
            return true;

        return false;
    }

    /**
     * @brief program the P4/P4+ datapath for given vnic
     *
     * @param[in] oci_vnic VNIC configuration
     * @param[in] vnic     VNIC information to be stored in the db
     *
     * program_datapath() should be called during batch_commit()
     */
    sdk_ret_t program_datapath(_In_ oci_vnic_t *oci_vnic, _In_ vnic_t *vnic);

private:
    ht      *ht_;      /**< vnic database
                            NOTE: even though VNIC scale is 1K, ids can be in
                                  the range [0, 4095], so to save memory,
                                  instead of 4k index table, we use hash
                                  table */
    indexer *idxr_;    /**< Indexer to allocate hw VNIC id */
    slab    *slab_;    /**< slab for allocating VNIC state */

    /**< P4 datapath tables for vnic
     *   NOTE: there is no explicit table mgmt. for rx and tx stats,
     *         we directly index and bzero out when we create a vnic
     */
    directmap       *local_vnic_by_vlan_tx_;
    sdk_hash        *local_vnic_by_slot_rx_;
    directmap       *egress_local_vnic_info_rx_;
};

/** * @} */ // end of OCI_VNIC

}    // namespace api

#endif    /** __VNIC_HPP__ */
