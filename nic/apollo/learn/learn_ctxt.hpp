//
// {C} Copyright 2020 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// learn context derived from learn pkt or API
///
//----------------------------------------------------------------------------

#ifndef __LEARN_LEARN_CTXT_HPP__
#define __LEARN_LEARN_CTXT_HPP__

#include <vector>
#include "nic/sdk/include/sdk/base.hpp"
#include "nic/apollo/framework/api_msg.hpp"
#include "nic/apollo/learn/learn.hpp"
#include "nic/apollo/learn/learn_impl_base.hpp"

namespace learn {

/// \brief type of learn entry object
typedef enum {
    LEARN_ENTRY_TYPE_NONE,
    LEARN_ENTRY_TYPE_MAC,
    LEARN_ENTRY_TYPE_IP,
} learn_entry_type_t;

/// \brief learn entry object
/// learn entries to be deleted are saved in a list and actually
/// deleted only if an API batch succeeds
typedef struct learn_entry_s {
    learn_entry_type_t      obj_type;
    ep_mac_entry            *mac_entry;         ///< needed for both MAC and IP
    ep_ip_entry             *ip_entry;
} learn_entry_t;

typedef std::vector<learn_entry_t> learn_entry_list_t;

/// \brief context specific to learn packets
typedef struct learn_pkt_ctxt_s {
    uint8_t                 pkt_drop_reason;    ///< pkt drop reason
    impl::learn_info_t      impl_info;          ///< learn info provided by impl
    ep_ip_entry             *old_ip_entry;      ///< existing IP entry ptr (L2L)
} learn_pkt_ctxt_t;

/// \brief context specific to mapping API
typedef struct learn_api_ctxt_s {
    pds_remote_mapping_spec_t   *spec;          ///< mapping spec from API
    pds_mapping_key_t           *mkey;          ///< mapping key from API
    api_op_t                    op;             ///< CRUD operation
    learn_entry_list_t          *del_objs;      ///< deleted learn entries
} learn_api_ctxt_t;

typedef enum {
    LEARN_CTXT_TYPE_NONE,
    LEARN_CTXT_TYPE_PKT,
    LEARN_CTXT_TYPE_API,
} learn_ctx_type_t;

/// \brief information passed across learn process functions
typedef struct learn_ctxt_s {
    ep_mac_key_t            mac_key;            ///< MAC key
    ep_ip_key_t             ip_key;             ///< IP key
    ep_mac_entry            *mac_entry;         ///< MAC entry ptr
    ep_ip_entry             *ip_entry;          ///< IP entry ptr
    pds_batch_ctxt_t        bctxt;              ///< API batch context
    ep_learn_type_t         mac_learn_type;     ///< MAC learn type
    ep_learn_type_t         ip_learn_type;      ///< IP learn type
    learn_ctx_type_t        ctxt_type;          ///< learn source
    union {
        learn_pkt_ctxt_t    pkt_ctxt;           ///< learn pkt context
        learn_api_ctxt_t    api_ctxt;           ///< mapping API context
    };

    void reset(void) {
        if (mac_entry && mac_entry->state() == EP_STATE_LEARNING) {
            ep_mac_entry::destroy(mac_entry);
        }
        if (ip_entry && ip_entry->state() == EP_STATE_LEARNING) {
            ep_ip_entry::destroy(ip_entry);
        }
        if (bctxt != PDS_BATCH_CTXT_INVALID) {
            sdk_ret_t ret;
            ret = pds_batch_destroy(bctxt);
            if (ret != SDK_RET_OK) {
                PDS_TRACE_ERR("Failed to destroy pds api batch, error code %u",
                              ret);
            }
        }
    }

    const char *pkt_ctxt_str(void) const {
        static string str;
        const impl::learn_info_t *impl = &pkt_ctxt.impl_info;

        str = "(" +
              std::string(mac_key.subnet.str()) + ", " +
              std::string(macaddr2str(mac_key.mac_addr)) + ", " +
              std::string(ip_key.vpc.str()) + ", " +
              std::string(ipaddr2str(&ip_key.ip_addr)) + ", " +
              "mac learn type " + to_string(mac_learn_type) + ", " +
              "ip learn type " + to_string(ip_learn_type) + ", " +
              "pkt type " + sdk::types::pkttype2str(impl->pkt_type) + ", " +
              "impl hints " + to_string(impl->hints) + ", " +
              "lif " + to_string(impl->lif) +
              ")";
        return str.c_str();
    }

    const char *api_ctxt_str(void) const {
        static string str;
        pds_mapping_key_t *mkey = api_ctxt.mkey;

        if (mkey->type == PDS_MAPPING_TYPE_L3) {
            str = "L3 mapping-(" + std::string(mkey->vpc.str()) + ", " +
                  std::string(ipaddr2str(&mkey->ip_addr)) + ")" +
                  " learn type " + to_string(ip_learn_type) + ", " +
                  " api op " + to_string(api_ctxt.op);
        } else if (mkey->type == PDS_MAPPING_TYPE_L2) {
            str = "L2 mapping-(" + std::string(mkey->subnet.str()) + ", " +
                  std::string(macaddr2str(mkey->mac_addr)) + ")"
                  " learn type " + to_string(mac_learn_type) + ", " +
                  " api op " + to_string(api_ctxt.op);
        }
        return str.c_str();
    }

    const char *str(void) const {
        if (ctxt_type == LEARN_CTXT_TYPE_PKT) {
            return pkt_ctxt_str();
        } else {
            return api_ctxt_str();
        }
    }

} learn_ctxt_t;

static inline void
add_to_delete_list (ep_mac_entry *mac_entry, learn_entry_list_t *del_objects)
{
    learn_entry_t del_obj;

    del_obj.obj_type = LEARN_ENTRY_TYPE_MAC;
    del_obj.mac_entry = mac_entry;
    del_objects->push_back(del_obj);
}

static inline void
add_to_delete_list (ep_ip_entry *ip_entry, ep_mac_entry *mac_entry,
                    learn_entry_list_t *del_objects)
{
    learn_entry_t del_obj;

    del_obj.obj_type = LEARN_ENTRY_TYPE_IP;
    del_obj.ip_entry = ip_entry;
    del_obj.mac_entry = mac_entry;
    del_objects->push_back(del_obj);
}

///< process learn context
sdk_ret_t process_learn(learn_ctxt_t *ctxt);

}    // namespace learn

#endif    // __LEARN_LEARN_CTXT_HPP__
