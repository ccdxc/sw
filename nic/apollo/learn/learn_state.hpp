//
// {C} Copyright 2020 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// global state of learn module
///
//----------------------------------------------------------------------------

#ifndef __LEARN_LEARN_STATE_HPP__
#define __LEARN_LEARN_STATE_HPP__

#include "nic/sdk/include/sdk/base.hpp"
#include "nic/sdk/lib/slab/slab.hpp"
#include "nic/sdk/lib/dpdk/dpdk.hpp"
#include "nic/sdk/lib/rte_indexer/rte_indexer.hpp"
#include "nic/apollo/framework/state_base.hpp"
#include "nic/apollo/learn/ep_mac_state.hpp"
#include "nic/apollo/learn/ep_ip_state.hpp"

using api::state_walk_cb_t;

namespace learn {

/// \brief packet drop reasons
enum {
    PKT_DROP_REASON_NONE = 0,       ///< unknown drop reason
    PKT_DROP_REASON_PARSE_ERR,      ///< unable to parse the packet
    PKT_DROP_REASON_RES_ALLOC_FAIL, ///< resource allocation failed
    PKT_DROP_REASON_LEARNING_FAIL,  ///< failed to learn from this pkt
    PKT_DROP_REASON_MBUF_ERR,       ///< mbuf manipulation error
    PKT_DROP_REASON_TX_FAIL,        ///< unable to tx the packet
    PKT_DROP_REASON_ARP_REPLY,      ///< arp reply for probe
    PKT_DROP_REASON_MAX,
};

/// \brief CRUD API operations tracked by learn counters
/// these are defined to decouple counter definitions from api_op_t enumeration
/// and also to make zero based indexing into counters easier
enum {
    OP_CREATE,
    OP_DELETE,
    OP_UPDATE,
    OP_MAX,
};

// local learn validation errors
enum {
    UNTAGGED_MAC_LIMIT,
    MAC_LIMIT,
    IP_LIMIT,
    IP_ADDR_SUBNET_MISMATCH,
    LEARN_VALIDATION_MAX,
};

/// \brief learn related debug counters
typedef struct learn_counters_s {
    // packet counters
    uint64_t    rx_pkts;                        ///< pkts rcvd on learn lif
    uint64_t    tx_pkts_ok;                     ///< pkts sent on learn lif
    uint64_t    tx_pkts_err;                    ///< pkt send failures
    uint64_t    arp_probes_ok;                  ///< ARP probes sent
    uint64_t    arp_probes_err;                 ///< ARP probe send failures
    uint64_t    pkt_buf_alloc_ok;               ///< mbuf allocations
    uint64_t    pkt_buf_alloc_err;              ///< mbuf allocation failures
    uint64_t    pkt_drop_reason[PKT_DROP_REASON_MAX];   ///< pkt drop reason

    // ageout counters
    uint64_t    ip_ageout_ok;                   ///< IP ageouts
    uint64_t    ip_ageout_err;                  ///< IP ageout failures
    uint64_t    mac_ageout_ok;                  ///< MAC ageouts
    uint64_t    mac_ageout_err;                 ///< MAC ageout failures

    // learn and endpoint move events
#define NUM_CTRS            learn_type_ctr_sz()
    uint64_t   mac_learns_ok [NUM_CTRS];        ///< mac learns and moves
    uint64_t   ip_learns_ok  [NUM_CTRS];        ///< ip learns and moves
    uint64_t   mac_learns_err[NUM_CTRS];        ///< mac learns and moves
    uint64_t   ip_learns_err [NUM_CTRS];        ///< ip learns and moves
#undef NUM_CTRS

    // learn validation errors
    uint64_t   validation_err[LEARN_VALIDATION_MAX];

    // api counters
    uint64_t   vnic_ok           [OP_MAX];      ///< VNIC API ops
    uint64_t   vnic_err          [OP_MAX];      ///< VNIC API ops failed
    uint64_t   remote_mac_map_ok [OP_MAX];      ///< remote l2 mapping API ops
    uint64_t   remote_mac_map_err[OP_MAX];      ///< remote l2 map API errors
    uint64_t   local_ip_map_ok   [OP_MAX];      ///< local l3 mapping API ops
    uint64_t   local_ip_map_err  [OP_MAX];      ///< local l3 mapping API error
    uint64_t   remote_ip_map_ok  [OP_MAX];      ///< remote l3 mapping API ops
    uint64_t   remote_ip_map_err [OP_MAX];      ///< remote l3 mapping API error
} learn_counters_t;

#define LEARN_COUNTER_INCR(name)        (learn_db()->counters()->name)++
#define LEARN_COUNTER_ADD(name, val)    (learn_db()->counters()->name) += (val)
#define LEARN_COUNTER_SUB(name, val)    (learn_db()->counters()->name) -= (val)
#define LEARN_COUNTER_DECR(name)        (learn_db()->counters()->name)--
#define LEARN_COUNTER_CLEAR(name)       learn_db()->counters()->name = 0
#define LEARN_COUNTER_GET(name)         learn_db()->counters()->name

/// \brief defualt timeout/age
#define LEARN_EP_DEFAULT_AGE_SEC                300
#define LEARN_EP_ARP_PROBE_TIMEOUT_SEC          30
#define LEARN_PKT_POLL_INTERVAL_MSEC            50

/// \brief packet receive parameters
#define LEARN_LIF_PKT_BUF_SZ                    2048
#define LEARN_LIF_RECV_BURST_SZ                 32
#define LEARN_LIF_RX_DESC_COUNT                 512
#define LEARN_LIF_TX_DESC_COUNT                 256

/// \brief packet buffer count for local use
#define LEARN_LIF_APP_MBUF_COUNT                32

/// \brief total packet buffer count
#define LEARN_LIF_MBUF_COUNT                    (LEARN_LIF_RX_DESC_COUNT + \
                                                 LEARN_LIF_TX_DESC_COUNT + \
                                                 LEARN_LIF_RECV_BURST_SZ + \
                                                 LEARN_LIF_APP_MBUF_COUNT)


/// \brief start epoch for API calls
// TODO: define a generic internal epoch id that can be used for batching only
// and not to advance the epoch
#define LEARN_API_EPOCH_START           0xdeaddead


/// \brief singleton to hold the global state of learn module
class learn_state : public state_base {
public:
    /// \brief      constructor
    learn_state();

    /// \brief      destructor
    ~learn_state();

    /// \brief      initialize the global state
    /// \return     #SDK_RET_OK on success, error code on failure
    sdk_ret_t init(void);

    /// \brief      return epoch to be used for API batch
    /// \return     epoch value
    pds_epoch_t epoch_next(void) { return ++epoch_; }

    /// \brief      allocate vnic id for new vnic creation
    /// \param[out] pointer to copy allocated index
    /// \return     #SDK_RET_OK on success, error code on failure
    sdk_ret_t vnic_obj_id_alloc(uint32_t *idx_ptr) {
        return vnic_objid_idxr_->alloc(idx_ptr);
    }

    /// \brief      free vnic index
    /// \param[in]  index to free
    /// \return     #SDK_RET_OK on success, error code on failure
    sdk_ret_t vnic_obj_id_free(uint32_t idx) {
        return vnic_objid_idxr_->free(idx);
    }

    /// \brief      return learn lif device
    /// \return     pointer to learn lif device
    dpdk_device *learn_lif(void) const { return learn_lif_; }

    /// \brief      return learn counters
    /// \return     pointer to learn counter struct
    learn_counters_t *counters(void) { return &counters_; }

    /// \brief      reset learn counters
    void reset_counters(void) {
        memset(&counters_, 0, sizeof(learn_counters_t));
    }

    /// \brief      return configured endpoint age
    /// \return     age in seconds
    uint32_t ep_timeout(void) const;

    /// \brief      return arp probe reply timeout
    /// \return     timeout value in seconds
    uint16_t arp_probe_timeout(void) const { return arp_probe_timeout_secs_; }

    /// \brief      return learn packer poll frequency
    /// \return     poll interval in seconds
    uint16_t pkt_poll_interval_msecs(void) const {
        return pkt_poll_interval_msecs_;
    }

    /// \brief      return endpoint MAC state
    /// \return     pointer to endpoint MAC state
    ep_mac_state *ep_mac_db(void) { return ep_mac_state_; }

    /// \brief      return endpoint MAC state
    /// \return     pointer to endpoint MAC state
    ep_ip_state *ep_ip_db(void) { return ep_ip_state_; }

    /// \brief API to walk MAC and IP entires
    /// \param[in] walk_cb    callback to be invoked for every entry
    /// \param[in] ctxt       opaque context passed back to the callback
    /// \return   SDK_RET_OK on success, failure status code on error
    sdk_ret_t walk(state_walk_cb_t walk_cb, void *ctxt) override;

    /// \brief API to walk the slabs in learn db
    /// \param[in] walk_cb    callback to be invoked for every slab
    /// \param[in] ctxt       opaque context passed back to the callback
    /// \return   SDK_RET_OK on success, failure status code on error
    sdk_ret_t slab_walk(state_walk_cb_t walk_cb, void *ctxt) override;

private:
    /// \brief      intialize learn lif device
    /// \return     #SDK_RET_OK on success, error code on failure
    sdk_ret_t lif_init_(void);
private:
    pds_epoch_t epoch_;                 ///< epoch for api batch
    rte_indexer *vnic_objid_idxr_;      ///< vnic object id
    dpdk_device *learn_lif_;            ///< learn lif device
    uint16_t arp_probe_timeout_secs_;   ///< arp probe timeout
    uint16_t pkt_poll_interval_msecs_;  ///< learn pkt poll frequency
    learn_counters_t counters_;         ///< debug counters
    ep_mac_state *ep_mac_state_;        ///< endpoint MAC state
    ep_ip_state *ep_ip_state_;          ///< endpoint IP state
};

}    // namepsace learn
using learn::learn_state;

#endif    // __LEARN_LEARN_STATE_HPP__
