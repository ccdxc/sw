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
#include "nic/apollo/api/pds_state.hpp"
#include "nic/apollo/framework/api_msg.hpp"
#include "nic/apollo/learn/ep_mac_entry.hpp"
#include "nic/apollo/learn/ep_ip_entry.hpp"
#include "nic/apollo/learn/learn.hpp"
#include "nic/apollo/learn/learn_impl_base.hpp"
#include "nic/apollo/learn/learn_state.hpp"

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
} learn_api_ctxt_t;

/// \brief per batch counters
/// counters are updated locally as each pkt or API is processed, learn
/// counters are updated based on whether the batch commit succeeds or fails
typedef struct batch_counters_s {
    uint32_t mac_learns    [NUM_LEARN_TYPE_CTRS];
    uint32_t ip_learns     [NUM_LEARN_TYPE_CTRS];
    uint32_t vnics         [OP_MAX];
    uint32_t remote_mac_map[OP_MAX];
    uint64_t local_ip_map  [OP_MAX];
    uint32_t remote_ip_map [OP_MAX];
} batch_counters_t;

/// \brief per batch context common across all pkts or APIs
typedef struct learn_batch_ctxt_s {
    learn_entry_list_t      del_objs;           ///< deleted entries
    batch_counters_t        counters;           ///< batch counters
} learn_batch_ctxt_t;

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
    ep_learn_type_t         mac_learn_type;     ///< MAC learn type
    ep_learn_type_t         ip_learn_type;      ///< IP learn type
    learn_ctx_type_t        ctxt_type;          ///< learn source
    pds_batch_ctxt_t        bctxt;              ///< API batch context
    learn_batch_ctxt_t      *lbctxt;            ///< API batch context
    union {
        learn_pkt_ctxt_t    pkt_ctxt;           ///< learn pkt context
        learn_api_ctxt_t    api_ctxt;           ///< mapping API context
    };
    std::string             mac_move_log;       ///< MAC move details for log
    std::string             ip_move_log;        ///< IP move details for log

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

    const char *pkt_ctxt_log_str(pds_mapping_type_t mtype) const {
        static string str;

        if (mtype == PDS_MAPPING_TYPE_L2) {
            str = "learn type " +
                  std::string(ep_learn_type_str(mac_learn_type)) + ", MAC-(" +
                  std::string(mac_key.subnet.str()) + ", " +
                  std::string(macaddr2str(mac_key.mac_addr)) + ")" +
                  mac_move_log;
        } else {
            str = "learn type " +
                  std::string(ep_learn_type_str(ip_learn_type)) + ", IP-(" +
                  std::string(ip_key.vpc.str()) + ", " +
                  std::string(ipaddr2str(&ip_key.ip_addr)) + ")" + ip_move_log;
        }
        return str.c_str();
    }

    const char *api_ctx_log_str(void) const {
        static string str;
        pds_mapping_key_t *mkey = api_ctxt.mkey;

        if (mkey->type == PDS_MAPPING_TYPE_L2) {
            str = "learn type " +
                  std::string(ep_learn_type_str(mac_learn_type)) + ", MAC-(" +
                  std::string(mkey->subnet.str()) + ", " +
                  std::string(macaddr2str(mkey->mac_addr)) + ")" + mac_move_log;
        } else {
            str = "learn type " +
                  std::string(ep_learn_type_str(ip_learn_type)) + ", IP-(" +
                  std::string(mkey->vpc.str()) + ", " +
                  std::string(ipaddr2str(&mkey->ip_addr)) + ")" + ip_move_log;
        }
        return str.c_str();
    }

    const char *log_str(pds_mapping_type_t mtype) const {
        if (ctxt_type == LEARN_CTXT_TYPE_PKT) {
            return pkt_ctxt_log_str(mtype);
        } else {
            return api_ctx_log_str();
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

static inline void
update_batch_counters(learn_batch_ctxt_t *lbctxt, bool ok)
{
    batch_counters_t *ctrs = &lbctxt->counters;

    // learn/move counters
    for (int i = 0; i < learn_type_ctr_sz(); i++) {
        if (ok) {
            LEARN_COUNTER_ADD(mac_learns_ok[i], ctrs->mac_learns[i]);
            LEARN_COUNTER_ADD(ip_learns_ok[i], ctrs->ip_learns[i]);
        } else {
            LEARN_COUNTER_ADD(mac_learns_err[i], ctrs->mac_learns[i]);
            LEARN_COUNTER_ADD(ip_learns_err[i], ctrs->ip_learns[i]);
        }
    }

    // API counters
    for (int i = 0; i < OP_MAX; i++) {
        if (ok) {
            LEARN_COUNTER_ADD(vnic_ok[i], ctrs->vnics[i]);
            LEARN_COUNTER_ADD(remote_mac_map_ok[i], ctrs->remote_mac_map[i]);
            LEARN_COUNTER_ADD(local_ip_map_ok[i], ctrs->local_ip_map[i]);
            LEARN_COUNTER_ADD(remote_ip_map_ok[i], ctrs->remote_ip_map[i]);
        } else {
            LEARN_COUNTER_ADD(vnic_err[i], ctrs->vnics[i]);
            LEARN_COUNTER_ADD(remote_mac_map_err[i], ctrs->remote_mac_map[i]);
            LEARN_COUNTER_ADD(local_ip_map_err[i], ctrs->local_ip_map[i]);
            LEARN_COUNTER_ADD(remote_ip_map_err[i], ctrs->remote_ip_map[i]);
        }
    }
}

///< process learn context
sdk_ret_t process_learn(learn_ctxt_t *ctxt);

}    // namespace learn

#endif    // __LEARN_LEARN_CTXT_HPP__
