//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// This file handles NAT port block configuration
///
//----------------------------------------------------------------------------

#ifndef __API_NAT_HPP__
#define __API_NAT_HPP__

#include "nic/sdk/lib/ht/ht.hpp"
#include "nic/apollo/framework/api_base.hpp"
#include "nic/apollo/framework/api_stooge.hpp"
#include "nic/apollo/api/include/pds_nat.hpp"

namespace api {

// forward declaration
class nat_port_block_state;

/// \defgroup PDS_NAT_PORT_BLOCK - NAT port block functionality
/// \ingroup PDS_NAT
/// @{

/// \brief    NAT port block entry
class nat_port_block : public api_stooge {
public:
    /// \brief          factory method to allocate & initialize a NAT port block
    /// \param[in]      spec    NAT port block information
    /// \return         new instance of NAT port block or NULL, in case of error
    static nat_port_block *factory(pds_nat_port_block_spec_t *port_block);

    /// \brief          release all the s/w state associate with the given
    ///                 NAT port block, if any, and free the memory
    /// \param[in]      port_block NAT port block to be freed
    /// \NOTE: h/w entries should have been cleaned up (by calling
    ///        impl->cleanup_hw() before calling this
    static void destroy(nat_port_block *port_block);

    /// \brief    clone this object and return cloned object
    /// \param[in]    api_ctxt API context carrying object related configuration
    /// \return       new object instance of current object
    virtual api_base *clone(api_ctxt_t *api_ctxt) override;

    /// \brief    free all the memory associated with this object without
    ///           touching any of the databases or h/w etc.
    /// \param[in] port_block    NAT port block to be freed
    /// \return   sdk_ret_ok or error code
    static sdk_ret_t free(nat_port_block *port_block);

    /// \brief    build object given its key from the (sw and/or hw state we
    ///           have) and return an instance of the object (this is useful for
    ///           stateless objects to be operated on by framework during DELETE
    ///           or UPDATE operations)
    /// \param[in] key    key of object instance of interest
    /// \return    NAT port block instance corresponding to the key
    static nat_port_block *build(pds_nat_port_block_key_t *key);

    /// \brief    free a stateless entry's temporary s/w only resources like
    ///           memory etc., for a stateless entry calling destroy() will
    ///           remove resources from h/w, which can't be done during ADD/UPD
    ///           etc. operations esp. when object is constructed on the fly
    /// \param[in] port_block    NAT port block instance to be freed
    static void soft_delete(nat_port_block *port_block);

    /// \brief          initialize NAT port block entry with the given config
    /// \param[in]      api_ctxt API context carrying the configuration
    /// \return         SDK_RET_OK on success, failure status code on error
    virtual sdk_ret_t init_config(api_ctxt_t *api_ctxt) override;

    /// \brief populate the IPC msg with object specific information
    ///        so it can be sent to other components
    /// \param[in] msg         IPC message to be filled in
    /// \param[in] obj_ctxt    transient state associated with this API
    /// \return #SDK_RET_OK on success, failure status code on error
    virtual sdk_ret_t populate_msg(pds_msg_t *msg,
                                   api_obj_ctxt_t *obj_ctxt) override;

    /// \brief          read config
    /// \param[out]     info pointer to the info object
    /// \return         SDK_RET_OK on success, failure status code on error
    sdk_ret_t read(pds_nat_port_block_info_t *info);

    /// \brief          add given NAT port block to the database
    /// \return         SDK_RET_OK on success, failure status code on error
    virtual sdk_ret_t add_to_db(void) override;

    /// \brief          delete given NAT port block from the database
    /// \return         SDK_RET_OK on success, failure status code on error
    virtual sdk_ret_t del_from_db(void) override;

    /// \brief          this method is called on new object that needs to
    ///                 replace the old version of the object in the DBs
    /// \param[in]      orig_obj    old version of the object being swapped out
    /// \param[in]      obj_ctxt    transient state associated with this API
    /// \return         SDK_RET_OK on success, failure status code on error
    virtual sdk_ret_t update_db(api_base *orig_obj,
                                api_obj_ctxt_t *obj_ctxt) override;

    /// \brief          initiate delay deletion of this object
    /// \return         SDK_RET_OK on success, failure status code on error
    virtual sdk_ret_t delay_delete(void) override;

    /// \brief          return stringified key of the object (for debugging)
    virtual string key2str(void) const override {
        return "nat-port-block-" + std::string(key_.str());
    }

    /// \brief          helper function to get key given NAT port block
    /// \param[in]      entry    pointer to NAT port block instance
    /// \return         pointer to the NAT port block instance's key
    static void *nat_port_block_key_func_get(void *entry) {
        nat_port_block *port_block = (nat_port_block *)entry;
        return (void *)&(port_block->key_);
    }

private:
    /// \brief constructor
    nat_port_block();

    /// \brief destructor
    ~nat_port_block();

    /// \brief      fill the NAT port block sw spec
    /// \param[out] spec specification
    void fill_spec_(pds_nat_port_block_spec_t *spec);

private:
    /// NAT port block key
    pds_nat_port_block_key_t key_;

    /// hash table context
    ht_ctxt_t ht_ctxt_;

    /// nat_state is friend of nat_port_block
    friend class nat_state;
} __PACK__;

/// \@}    // end of PDS_NAT

}    // namespace api

using api::nat_port_block;

#endif    // __API_NAT_HPP__
