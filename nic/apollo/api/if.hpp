//
// {C} Copyright 2018 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// interface entry handling
///
//----------------------------------------------------------------------------

#ifndef __API_IF_HPP__
#define __API_IF_HPP__

#include <string.h>
#include "nic/sdk/lib/ht/ht.hpp"
#include "nic/apollo/api/include/pds.hpp"
#include "nic/apollo/framework/api_base.hpp"

#define PDS_MAX_IF    256             ///< Max interfaces

namespace api {

// forward declaration
class if_state;

/// \defgroup PDS_IF_ENTRY - interface entry functionality
/// \ingroup PDS_IF
/// @{

/// \brief    interface entry
class if_entry : public api_base {
public:
    /// \brief    factory method to allocate & initialize a interface entry
    /// \param[in] key    interface key
    /// \return    new instance of interface or NULL, in case of error
    static if_entry *factory(pds_ifindex_t key);

    /// \brief          release all the s/w state associate with the given
    ///                 interface, if any, and free the memory
    /// \param[in]      intf    interface to be freed
    /// \NOTE: h/w entries should have been cleaned up (by calling
    ///        impl->cleanup_hw() before calling this
    static void destroy(if_entry *intf);

    /// \brief          initiate delay deletion of this object
    virtual sdk_ret_t delay_delete(void) override;

    /// \brief          return stringified key of the object (for debugging)
    virtual string key2str(void) const override {
        return "if-" + std::to_string(key_);
    }

    /// \brief          helper function to get key given interface entry
    /// \param          entry    pointer to interface instance
    /// \return         pointer to the interface instance's key
    static void *if_key_func_get(void *entry) {
        if_entry *intf = (if_entry *)entry;
        return (void *)&(intf->key_);
    }

    /// \brief          helper function to compute hash value for given
    ///                 interface id
    /// \param[in]      key        interface's key
    /// \param[in]      ht_size    hash table size
    /// \return         hash value
    static uint32_t if_hash_func_compute(void *key, uint32_t ht_size) {
        return hash_algo::fnv_hash(key, sizeof(pds_ifindex_t)) % ht_size;
    }

    /// \brief          helper function to compare two interface keys
    /// \param[in]      key1        pointer to interface's key
    /// \param[in]      key2        pointer to interface's key
    /// \return         0 if keys are same or else non-zero value
    static bool if_key_func_compare(void *key1, void *key2) {
        SDK_ASSERT((key1 != NULL) && (key2 != NULL));
        if (!memcmp(key1, key2, sizeof(pds_ifindex_t))) {
            return true;
        }
        return false;
    }

    /// \brief    return the interface index
    /// \return interface index
    pds_ifindex_t key(void) const { return key_; }

    /// \brief    set the interface specific information
    /// \param[in] if_info    pointer to the interface specific information
    void set_if_info(void *if_info) { if_info_ = if_info; }

    /// \brief    return interface specific information
    /// \return return pointer to the interface specific information
    void *if_info(void) { return if_info_; }

private:
    /// \brief constructor
    if_entry();

    /// \brief destructor
    ~if_entry();

private:
    pds_ifindex_t key_;      ///< interface key
    void *if_info_;          ///< interface specific information
    ht_ctxt_t ht_ctxt_;       ///< hash table context
    friend class if_state;    ///< if_state is friend of if_entry
} __PACK__;

/// \@}    // end of PDS_IF_ENTRY

}    // namespace api

using api::if_entry;

#endif    // __API_IF_HPP__
