//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// lif datapath implementation
///
//----------------------------------------------------------------------------

#ifndef __LIF_IMPL_HPP__
#define __LIF_IMPL_HPP__

#include "nic/sdk/lib/ht/ht.hpp"
#include "nic/sdk/include/sdk/qos.hpp"
#include "nic/sdk/platform/devapi/devapi_types.hpp"
#include "nic/apollo/framework/impl_base.hpp"

namespace api {
namespace impl {

// forward declaration
class lif_impl_state;

typedef struct pds_lif_spec_s {
    pds_lif_key_t    key;
    pds_port_id_t    pinned_port_id;
} pds_lif_spec_t;

/// \defgroup PDS_LIF_IMPL - lif entry datapath implementation
/// \ingroup PDS_LIF
/// \@{
/// \brief LIF implementation

class lif_impl : public impl_base {
public:
    /// \brief  factory method to allocate and initialize a lif entry
    /// \param[in] spec    lif configuration parameters
    /// \return    new instance of lif or NULL, in case of error
    static lif_impl *factory(pds_lif_spec_t *spec);

    /// \brief  release all the s/w state associate with the given lif,
    ///         if any, and free the memory
    /// \param[in] impl    lif impl instance to be freed
    ///                  NOTE: h/w entries should have been cleaned up (by
    ///                  calling impl->cleanup_hw() before calling this
    static void destroy(lif_impl *impl);

    /// \brief     helper function to get key given lif entry
    /// \param[in] entry    pointer to lif instance
    /// \return    pointer to the lif instance's key
    static void *lif_key_func_get(void *entry) {
        lif_impl *lif = (lif_impl *)entry;
        return (void *)&(lif->key_);
    }

    /// \brief     helper function to compute hash value for given lif id
    /// \param[in] key        lif's key
    /// \param[in] ht_size    hash table size
    /// \return    hash value
    static uint32_t lif_hash_func_compute(void *key, uint32_t ht_size) {
        return hash_algo::fnv_hash(key, sizeof(pds_lif_key_t)) % ht_size;
    }

    /// \brief     helper function to compare two lif keys
    /// \param[in] key1    pointer to lif's key
    /// \param[in] key2    pointer to lif's key
    /// \return    0 if keys are same or else non-zero value
    static bool lif_key_func_compare(void *key1, void *key2) {
        SDK_ASSERT((key1 != NULL) && (key2 != NULL));
        if (!memcmp(key1, key2, sizeof(pds_lif_key_t))) {
            return true;
        }
        return false;
    }

    ///< \brief    program lif tx policer for given lif 
    ///< param[in] lif_id     h/w lif id
    ///< param[in] policer    policer parameters
    static sdk_ret_t program_tx_policer(uint32_t lif_id,
                                        sdk::policer_t *policer);

    ///< \brief  return the pinned port id of the lif
    /// \return  port id, lif is pinned to
    pds_port_id_t pinned_port_id(void) const {
        return pinned_port_id_;
    }

    ///< \brief    program necessary filter entries for this lif
    ///< \param[in] lif_params    lif configuration parameters
    sdk_ret_t program_filters(lif_info_t *lif_params);

private:
    ///< constructor
    lif_impl(pds_lif_spec_t *spec);

    ///< destructor
    ~lif_impl() {}

private:
    pds_lif_key_t    key_;               ///< (s/w & h/w) lif id
    pds_port_id_t    pinned_port_id_;    ///< pinnned port id, if any
    ht_ctxt_t        ht_ctxt_;           ///< hash table context

    friend class lif_impl_state;         ///< lif_impl_state is friend of lif_impl
} __PACK__;

/// \@}

}    // namespace impl
}    // namespace api

#endif    /** __LIF_IMPL_HPP__ */
