/**
 * Copyright (c) 2018 Pensando Systems, Inc.
 *
 * @file    vcn.hpp
 *
 * @brief   This file deals with OCI VCN functionality
 */

#if !defined (__VCN_HPP_)
#define __VCN_HPP_

#include "nic/hal/apollo/api/api.hpp"
#include "nic/hal/apollo/include/api/oci_vcn.hpp"

namespace api {

/**
 * @defgroup OCI_VCN - Internal VCN
 * @{
 */

/**
 * @brief Internal VCN structure
 */
typedef struct vcn_s {
    oci_vcn_key_t    key;        /**< VCN Key */
    ht_ctxt_t        ht_ctxt;    /**< Hash table context */

    /**< P4 datapath specific state */
    uint16_t         hw_id;      /**< hardware ID */
} __PACK__ vcn_t;

/**
 * @brief state maintained for VCNs
 */
class vcn_state {
public:
    /**
     * constructor
     */
    vcn_state();

    /**
     * destructor
     */
    ~vcn_state();

    /**
     * @brief Handle VCN create message
     *
     * @param[in] vcn VCN information
     * @return #SDK_RET_OK on success, failure status code on error
     */
    sdk_ret_t vcn_create(_In_ oci_vcn_t *oci_vcn);

    /**
     * @brief Handle VCN delete API
     *
     * @param[in] vcn_key VCN key information
     * @return #SDK_RET_OK on success, failure status code on error
     */
    sdk_ret_t vcn_delete(_In_ oci_vcn_key_t *key);

    vcn_t *vcn_get(_In_ oci_vcn_key_t *key);

    /**
     * @brief   return vcn slab
     */
    slab *vcn_slab(void) const { return vcn_slab_; }

private:
    /**
     * @brief Add VCN to database
     *
     * @param[in] vcn VCN
     */ 
    sdk_ret_t vcn_add_to_db(vcn_t *vcn);

    /**
     * @brief Delete VCN from database
     *
     * @param[in] vcn_key VCN key
     */
    vcn_t *vcn_del_from_db(oci_vcn_key_t *vcn_key);

    /**
     * @brief Lookup VCN in database
     *
     * @param[in] vcn_key VCN key
     */
    vcn_t *vcn_find(oci_vcn_key_t *vcn_key) const;

    /**
     * @brief Allocate VCN structure
     *
     * @return Pointer to the allocated VCN, NULL if no memory
     */
    vcn_t *vcn_alloc(void);

    /**
     * @brief Initialize VCN structure
     *
     * @param[in] vcn VCN structure to store the state
     * @param[in] oci_vcn VCN specific information
     */
    sdk_ret_t vcn_init(vcn_t *vcn, oci_vcn_t *oci_vcn);

    /**
     * @brief Allocate and initialize VCN structure
     *
     * @return Pointer to the allocated and initialized VCN,
     *         NULL if no memory
     */
    vcn_t *vcn_alloc_init(oci_vcn_t *oci_vcn);

    /**
     * @brief Cleanup state maintained for given VCN
     *
     * @param[in] vcn VCN
     */
    void vcn_cleanup(vcn_t *vcn);

    /**
     * @brief Free VCN structure
     *
     * @param[in] vcn VCN
     */
    void vcn_free(vcn_t *vcn);

    /**
     * @brief Uninitialize and free VCN structure
     *
     * @param[in] vcn VCN
     */
    void vcn_delete(_In_ vcn_t *vcn);

    /**
     * @brief helper function to get key given VCN entry
     */
    static void *vcn_key_func_get(void *entry) {
        vcn_t *vcn = (vcn_t *)entry;
        return (void *)&(vcn->key);
    }

    /**
     * @brief helper function to compute hash value for given VCN id
     */
    static uint32_t vcn_hash_func_compute (void *key, uint32_t ht_size) {
        return hash_algo::fnv_hash(key, sizeof(oci_vcn_key_t)) % ht_size;
    }

    /**
     * @brief helper function to compare two VCN keys
     */
    static bool vcn_key_func_compare (void *key1, void *key2) {
        SDK_ASSERT((key1 != NULL) && (key2 != NULL));
        if (!memcmp(key1, key2, sizeof(oci_vcn_key_t)))
            return true;

        return false;
    }

private:
    ht      *vcn_ht_;      /**< Hash table root */
    indexer *vcn_idxr_;    /**< Indexer to allocate hw VCN id */
    slab    *vcn_slab_;    /**< slab for allocating VCN state */
};
extern vcn_state g_vcn_state;

/** * @} */ // end of OCI_VCN

}    // namespace api

#endif /** __VCN_HPP_ */
