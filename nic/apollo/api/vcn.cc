/**
 * Copyright (c) 2018 Pensando Systems, Inc.
 *
 * @file    oci_vcn.cc
 *
 * @brief   This file deals with OCI VCN API handling
 */

namespace api {

/**
 * @brief Global internal VCN state
 */
typedef struct oci_int_vcn_gstate_s
{
  /**< slab */
  /**< hash table */
} oci_vcn_gstate_t;

/**
 * @brief Internal VCN structure
 */
typedef struct oci_int_vcn_s
{
    oci_vcn_key_t key;    /**< VCN Key */
    uint16_t id;          /**< Internal ID */

} PACKED oci_vcn_t;

oci_int_vcn_gstate_t g_int_vcn_state;

/**
 * @brief Initialize global internal VCN state
 */
void
oci_int_vcn_global_state_init (void)
{
}


/**
 * @brief Allocate internal VCN structure
 *
 * @return Pointer to the allocated VCN struture, NULL if no memory
 */
static inline oci_int_vcn_t *
oci_int_vcn_alloc (void)
{
    return ((oci_int_vcn_t *)g_vcn_state->int_vcn_slab()->alloc());
}

/**
 * @brief Free internal VCN structure
 *
 * @param[in] vcn VCN information
 */
static inline void
oci_int_vcn_free (oci_int_vcn_t *int_vcn)
{
    sdk::delay_delete_to_slab(OCI_SLAB_INT_VCN, int_vcn);
}

static inline void
oci_int_vcn_init (oci_int_vnc_t *vcn)
{
    PSD_SDK_SPINLOCK_INIT(&pol->slock, PTHREAD_PROCESS_SHARED);
}

static inline void
nat_cfg_pol_uninit (nat_cfg_pol_t *pol)
{
    PSD_SDK_SPINLOCK_DESTROY(&pol->slock);
}

static inline nat_cfg_pol_t *
nat_cfg_pol_alloc_init (void)
{
    nat_cfg_pol_t *pol;

    if ((pol = nat_cfg_pol_alloc()) ==  NULL)
        return NULL;

    nat_cfg_pol_init(pol);
    return pol;
}

static inline void
nat_cfg_pol_uninit_free (nat_cfg_pol_t *pol)
{
    nat_cfg_pol_uninit(pol);
    nat_cfg_pol_free(pol);
}

static inline void
nat_cfg_pol_cleanup (nat_cfg_pol_t *pol)
{
    if (pol) {
        nat_cfg_rule_list_cleanup(&pol->rule_list);
        nat_cfg_pol_uninit_free(pol);
    }
}


/**
 * @brief Allocate and initialize internal VCN structure
 *
 * @param[in] vcn VCN information
 * @return #PSD_SDK_STATUS_SUCCESS on success, failure status code on error
 */
static inline psd_sdk_status_t
oci_vcn_create_handle (_In_ oci_vcn_t *vcn)
{
    oci_int_vcn_t *int_vcn;

    return PSD_SDK_STATUS_SUCCESS;
}

/**
 * @brief Validate VNC API data
 *
 * @param[in] vcn VCN information
 * @return #PSD_SDK_STATUS_SUCCESS on success, failure status code on error
 */
static inline psd_sdk_status_t
oci_vcn_create_validate (_In_ oci_vcn_t *vcn)
{
    return PSD_SDK_STATUS_SUCCESS;
}

/**
 * @brief Create VCN
 *
 * @param[in] vcn VCN information
 * @return #PSD_SDK_STATUS_SUCCESS on success, failure status code on error
 */
psd_sdk_status_t
oci_vcn_create (_In_ oci_vcn_t *vcn)
{
    psd_sdk_status_t st;

    if ((rv = oci_vcn_create_validate(vcn) != PSD_SDK_STATUS_SUCCESS))
        return st;

    if ((rv = oci_vcn_create_handle(vcn)) != PSD_SDK_STATUS_SUCCESS)
        return st;

    return PSD_SDK_STATUS_SUCCESS;
}

/**
 * @brief Handle VCN delete API
 *
 * @param[in] vcn VCN information
 * @return #PSD_SDK_STATUS_SUCCESS on success, failure status code on error
 */
static inline psd_sdk_status_t
oci_vcn_delete_handle (_In_ oci_vcn_t *vcn)
{
    oci_int_vcn_t *int_vcn;

    return PSD_SDK_STATUS_SUCCESS;
}

/**
 * @brief Validate VNC delete API
 *
 * @param[in] vcn_key VCN key information
 * @return #PSD_SDK_STATUS_SUCCESS on success, failure status code on error
 */
static inline psd_sdk_status_t
oci_vcn_delete_validate (_In_ oci_vcn_key_t *vcn_key)
{
    return PSD_SDK_STATUS_SUCCESS;
}

/**
 * @brief Delete VCN
 *
 * @param[in] vcn_key VCN key
 * @return #PSD_SDK_STATUS_SUCCESS on success, failure status code on error
 */
psd_sdk_status_t
oci_vcn_delete (_In_ oci_vcn_key_t *vcn_key)
{
    psd_sdk_status_t rv;
    oci_vcn_t *vcn;

    if ((rv = oci_vcn_delete_validate(vcn_key) != PSD_SDK_STATUS_SUCCESS)
        return rv;
    
    if ((int_vcn = oci_int_vcn_lookup(vcn_key)) == NULL)
        return PSD_SDK_STATUS_NOT_FOUND;

    if ((rv = oci_vcn_delete_handle(vcn_key) != OCIS_TAoci_vcn
    return PSD_SDK_STATUS_SUCCESS;
}


}  // namespace api 
