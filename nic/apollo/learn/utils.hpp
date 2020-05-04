//
// {C} Copyright 2020 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// packet manipulation and rx tx utilties
///
//----------------------------------------------------------------------------

#ifndef __LEARN_UTILS_HPP__
#define __LEARN_UTILS_HPP__

#include "nic/sdk/lib/dpdk/dpdk.hpp"
#include "nic/apollo/learn/learn_state.hpp"
#include "nic/apollo/learn/learn_impl_base.hpp"

namespace learn {

#define learn_lif learn_db()->learn_lif()

/// \brief allocate mbuf from learn lif's pkt buffer pool
static inline void *
learn_lif_alloc_mbuf (void)
{
    void *mbuf;

    mbuf = (void *)learn_lif->alloc_mbuf();
    if (unlikely(mbuf == nullptr)) {
        LEARN_COUNTER_INCR(pkt_buf_alloc_err);
    } else {
        LEARN_COUNTER_INCR(pkt_buf_alloc_ok);
    }
    return mbuf;
}

/// \brief get pointer to pkt data given mbuf
static inline char *
learn_lif_mbuf_data_start (void *mbuf)
{
    return dpdk_device::get_data_ptr((dpdk_mbuf *)mbuf);
}

/// \brief adjust pkt data pointer of rx mbuf to make room for arm to p4 tx hdr
/// returns pointer to pkt data start
static inline char *
learn_lif_mbuf_rx_to_tx (void *mbuf)
{
    // TODO: cache these constants or make them constexpr
    uint16_t p4_to_arm_hdr_sz = impl::p4_to_arm_hdr_sz();
    uint16_t arm_to_p4_hdr_sz = impl::arm_to_p4_hdr_sz();
    dpdk_mbuf *pkt = (dpdk_mbuf *)mbuf;

    // remove rx header and insert tx header keeping packet data location intact
    if (dpdk_device::remove_header(pkt, p4_to_arm_hdr_sz) != nullptr) {
        return dpdk_device::add_header(pkt, arm_to_p4_hdr_sz);
    }
    return nullptr;
}

/// \brief drop packet by freeing mbuf to learn lif's pkt buffer pool
static inline void
learn_lif_drop_pkt (void *mbuf, uint8_t reason)
{
    dpdk_mbuf *pkt = (dpdk_mbuf *)mbuf;

    learn_lif->drop_packets(&pkt, 1);
    SDK_ASSERT((reason < PKT_DROP_REASON_MAX));
    LEARN_COUNTER_INCR(pkt_drop_reason[reason]);

    // force driver cq cleanup
    (void) learn_lif->transmit_packets(0, nullptr, 0);
}

/// \brief transmit pkt from learn lif
/// this expects that packet data has correct arm to p4 tx header
static inline void
learn_lif_send_pkt (void *mbuf)
{
    dpdk_mbuf *pkt = (dpdk_mbuf *)mbuf;
    uint16_t tx_fail;

    tx_fail = learn_lif->transmit_packets(0, &pkt, 1);
    if (tx_fail) {
        LEARN_COUNTER_INCR(tx_pkts_err);
        learn_lif_drop_pkt(mbuf, PKT_DROP_REASON_TX_FAIL);
        return;
    }
    LEARN_COUNTER_INCR(tx_pkts_ok);
}

/// \brief add data to mbuf
/// increment pkt data length and return pointer to pkt data
static inline char *
learn_lif_mbuf_append_data (void *mbuf, uint16_t len)
{
    dpdk_mbuf *pkt = (dpdk_mbuf *)mbuf;

    return dpdk_device::append_data(pkt, len);
}

/// \brief get count of available mbufs in the pool
static inline uint32_t
learn_lif_avail_mbuf_count (void)
{
    // statistics could be fetched before we initialize learn lif
    if (learn_lif) {
        return learn_lif->avail_mbuf_count();
    }
    return 0;
}

}    // namespace learn

#endif    // __LEARN_UTILS_HPP__
