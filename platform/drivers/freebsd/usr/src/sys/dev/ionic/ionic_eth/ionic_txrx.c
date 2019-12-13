/*
 * Copyright (c) 2017-2019 Pensando Systems, Inc.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#include <linux/kernel.h>
#include <linux/interrupt.h>
#include <linux/if_vlan.h>

#include "opt_inet.h"
#include "opt_inet6.h"

#include <netinet/in_systm.h>
#include <netinet/in.h>
#include <netinet/if_ether.h>
#include <netinet/ip.h>
#include <netinet/ip6.h>
#include <netinet6/ip6_var.h>
#include <netinet/tcp.h>
#include <netinet/tcp_lro.h>
#include <netinet/udp.h>
#include <net/ethernet.h>
#include <sys/buf_ring.h>
#include <net/sff8472.h>
#include <net/sff8436.h>

#include <sys/sockio.h>
#include <sys/kdb.h>
#include <sys/param.h>
#include <sys/systm.h>
#include <sys/linker.h>
#include <sys/firmware.h>

#include "ionic.h"
#include "ionic_lif.h"
#include "ionic_txrx.h"

#include "opt_rss.h"

#ifdef	RSS
#include <net/rss_config.h>
#include <netinet/in_rss.h>
#endif

#define QUEUE_NAME_LEN 32

MALLOC_DEFINE(M_IONIC, "ionic", "Pensando IONIC Ethernet adapter");
static int ionic_ioctl(struct ifnet *ifp, u_long command, caddr_t data);
static SYSCTL_NODE(_hw, OID_AUTO, ionic, CTLFLAG_RD, 0,
    "Pensando Ethernet parameters");

int ionic_max_queues = 0;
TUNABLE_INT("hw.ionic.max_queues", &ionic_max_queues);
SYSCTL_INT(_hw_ionic, OID_AUTO, max_queues, CTLFLAG_RDTUN,
    &ionic_max_queues, 0, "Number of Queues");

/* XXX: 60 seconds for firmware update */
int ionic_devcmd_timeout = 60;
TUNABLE_INT("hw.ionic.devcmd_timeout", &ionic_devcmd_timeout);
SYSCTL_INT(_hw_ionic, OID_AUTO, devcmd_timeout, CTLFLAG_RWTUN,
    &ionic_devcmd_timeout, 0, "Timeout in seconds for devcmd");

int ionic_enable_msix = 1;
TUNABLE_INT("hw.ionic.enable_msix", &ionic_enable_msix);
SYSCTL_INT(_hw_ionic, OID_AUTO, enable_msix, CTLFLAG_RWTUN,
    &ionic_enable_msix, 0, "Enable MSI/X");

int adminq_descs = 16;
TUNABLE_INT("hw.ionic.adminq_descs", &adminq_descs);
SYSCTL_INT(_hw_ionic, OID_AUTO, adminq_descs, CTLFLAG_RDTUN,
    &adminq_descs, 0, "Number of Admin descriptors");

int ionic_notifyq_descs = 64;
TUNABLE_INT("hw.ionic.notifyq_descs", &ionic_notifyq_descs);
SYSCTL_INT(_hw_ionic, OID_AUTO, notifyq_descs, CTLFLAG_RDTUN,
    &ionic_notifyq_descs, 0, "Number of notifyq descriptors");

int ionic_tx_descs = 2048;
TUNABLE_INT("hw.ionic.tx_descs", &ionic_tx_descs);
SYSCTL_INT(_hw_ionic, OID_AUTO, tx_descs, CTLFLAG_RDTUN,
    &ionic_tx_descs, 0, "Number of Tx descriptors");

int ionic_rx_descs = 2048;
TUNABLE_INT("hw.ionic.rx_descs", &ionic_rx_descs);
SYSCTL_INT(_hw_ionic, OID_AUTO, rx_descs, CTLFLAG_RDTUN,
    &ionic_rx_descs, 0, "Number of Rx descriptors");

int ionic_rx_stride = 4;
TUNABLE_INT("hw.ionic.rx_stride", &ionic_rx_stride);
SYSCTL_INT(_hw_ionic, OID_AUTO, rx_stride, CTLFLAG_RWTUN,
    &ionic_rx_stride, 0, "Rx side doorbell ring stride");

int ionic_tx_stride = 4;
TUNABLE_INT("hw.ionic.tx_stride", &ionic_tx_stride);
SYSCTL_INT(_hw_ionic, OID_AUTO, tx_stride, CTLFLAG_RWTUN,
	&ionic_tx_stride, 0, "Tx side doorbell ring stride");

int ionic_rx_fill_threshold = 128;
TUNABLE_INT("hw.ionic.rx_fill_threshold", &ionic_rx_fill_threshold);
SYSCTL_INT(_hw_ionic, OID_AUTO, rx_fill_threshold, CTLFLAG_RWTUN,
    &ionic_rx_fill_threshold, 0, "Rx fill threshold");

int ionic_tx_clean_limit = 128;
TUNABLE_INT("hw.ionic.tx_clean_limit", &ionic_tx_clean_limit);
SYSCTL_INT(_hw_ionic, OID_AUTO, tx_clean_threshold, CTLFLAG_RWTUN,
    &ionic_tx_clean_limit, 0, "Tx clean threshold");

/* Number of packets processed by ISR, rest is handled by task handler. */
int ionic_rx_clean_limit = 128;
TUNABLE_INT("hw.ionic.rx_clean_limit", &ionic_rx_clean_limit);
SYSCTL_INT(_hw_ionic, OID_AUTO, rx_clean_limit, CTLFLAG_RWTUN,
    &ionic_rx_clean_limit, 0, "Rx can process maximum number of descriptors.");

int ionic_intr_coalesce = 3;
TUNABLE_INT("hw.ionic.intr_coalesce", &ionic_intr_coalesce);
SYSCTL_INT(_hw_ionic, OID_AUTO, intr_coalesce, CTLFLAG_RWTUN,
    &ionic_intr_coalesce, 0, "Initial interrupt coal value in us.");

/* Size of Rx scatter gather buffers, disabled by default. */
int ionic_rx_sg_size = 0;
TUNABLE_INT("hw.ionic.rx_sg_size", &ionic_rx_sg_size);
SYSCTL_INT(_hw_ionic, OID_AUTO, rx_sg_size, CTLFLAG_RDTUN,
    &ionic_rx_sg_size, 0, "Rx scatter-gather buffer size, disabled by default.");

int ionic_dev_cmd_auto_disable = true;
TUNABLE_INT("hw.ionic.dev_cmd_auto_disable", &ionic_dev_cmd_auto_disable);
SYSCTL_INT(_hw_ionic, OID_AUTO, dev_cmd_auto_disable, CTLFLAG_RWTUN,
    &ionic_dev_cmd_auto_disable, 0, "Enable device self-disable after error.");

int ionic_wdog_error_trigger = 0;
TUNABLE_INT("hw.ionic.wdog_error_trigger", &ionic_wdog_error_trigger);
SYSCTL_INT(_hw_ionic, OID_AUTO, wdog_error_trigger, CTLFLAG_RWTUN,
    &ionic_wdog_error_trigger, 0, "Trigger error via watchdog (TEST).");

int ionic_adminq_hb_interval = 0; /* Disabled by default */
TUNABLE_INT("hw.ionic.adminq_hb_interval", &ionic_adminq_hb_interval);
SYSCTL_INT(_hw_ionic, OID_AUTO, adminq_hb_interval, CTLFLAG_RDTUN,
    &ionic_adminq_hb_interval, 0, "AdminQ heartbeat interval in msecs.");

int ionic_cmd_hb_interval = 0;    /* Disabled by default */
TUNABLE_INT("hw.ionic.cmd_hb_interval", &ionic_cmd_hb_interval);
SYSCTL_INT(_hw_ionic, OID_AUTO, cmd_hb_interval, CTLFLAG_RDTUN,
    &ionic_cmd_hb_interval, 0, "Command heartbeat interval in msecs.");

int ionic_fw_hb_interval = IONIC_WDOG_HB_DEFAULT_MS;
TUNABLE_INT("hw.ionic.fw_hb_interval", &ionic_fw_hb_interval);
SYSCTL_INT(_hw_ionic, OID_AUTO, fw_hb_interval, CTLFLAG_RDTUN,
    &ionic_fw_hb_interval, 0, "Firmware heartbeat interval in msecs.");

int ionic_txq_wdog_timeout = IONIC_WDOG_TX_DEFAULT_MS;
TUNABLE_INT("hw.ionic.txq_wdog_timeout", &ionic_txq_wdog_timeout);
SYSCTL_INT(_hw_ionic, OID_AUTO, txq_wdog_timeout, CTLFLAG_RDTUN,
    &ionic_txq_wdog_timeout, 0, "Tx queue watchdog timeout in msecs.");

/*
 * Firmware version for update.
 */
static char ionic_fw_update_ver[IONIC_DEVINFO_FWVERS_BUFLEN + 1];
TUNABLE_STR("hw.ionic.fw_update_ver", ionic_fw_update_ver,
    sizeof(ionic_fw_update_ver));
SYSCTL_STRING(_hw_ionic, OID_AUTO, fw_update_ver, CTLFLAG_RWTUN,
    ionic_fw_update_ver, 0, "Firmware version that needs to be programmed.");

int ionic_cdp_vlan = 0;
TUNABLE_INT("hw.ionic.cdp_vlan", &ionic_cdp_vlan);
SYSCTL_INT(_hw_ionic, OID_AUTO, cdp_vlan, CTLFLAG_RDTUN,
    &ionic_cdp_vlan, 0, "Receive CDP tagged with this vlan");

int ionic_lldp_vlan = 0;
TUNABLE_INT("hw.ionic.lldp_vlan", &ionic_lldp_vlan);
SYSCTL_INT(_hw_ionic, OID_AUTO, lldp_vlan, CTLFLAG_RDTUN,
    &ionic_lldp_vlan, 0, "Receive LLDP tagged with this vlan");

#ifndef IONIC_NDEBUG
int ionic_debug = 0;
TUNABLE_INT("hw.ionic.debug", &ionic_debug);
SYSCTL_INT(_hw_ionic, OID_AUTO, debug, CTLFLAG_RWTUN,
    &ionic_debug, 0, "Enable IONIC_INFO debug messages");

int ionic_trace = 0;
TUNABLE_INT("hw.ionic.trace", &ionic_trace);
SYSCTL_INT(_hw_ionic, OID_AUTO, trace, CTLFLAG_RWTUN,
    &ionic_trace, 0, "Enable IONIC_TRACE debug messages");

int ionic_tso_debug = 0;
TUNABLE_INT("hw.ionic.tso_debug", &ionic_tso_debug);
SYSCTL_INT(_hw_ionic, OID_AUTO, tso_debug, CTLFLAG_RWTUN,
    &ionic_tso_debug, 0, "Enable IONIC_TSO_DEBUG messages");
#endif

static void ionic_start_xmit_locked(struct ifnet *ifp, struct ionic_txque *txq);

static inline bool
ionic_is_rx_tcp(uint8_t pkt_type)
{

	return ((pkt_type == PKT_TYPE_IPV4_TCP) ||
		(pkt_type == PKT_TYPE_IPV6_TCP));
}

static inline bool
ionic_is_rx_ipv6(uint8_t pkt_type)
{

	return ((pkt_type == PKT_TYPE_IPV6) ||
		(pkt_type == PKT_TYPE_IPV6_TCP) ||
		(pkt_type == PKT_TYPE_IPV6_UDP));
}

static void
ionic_dump_mbuf(struct mbuf* m)
{
	IONIC_INFO("len %u\n", m->m_len);

	IONIC_INFO(
	    "data %02hhx %02hhx %02hhx %02hhx %02hhx %02hhx %02hhx %02hhx\n",
	    m->m_data[0], m->m_data[1], m->m_data[2], m->m_data[3],
	    m->m_data[4], m->m_data[5], m->m_data[6], m->m_data[7]);
	IONIC_INFO(
	    "data end %02hhx %02hhx %02hhx %02hhx %02hhx %02hhx %02hhx %02hhx\n",
	    m->m_data[m->m_len - 8], m->m_data[m->m_len - 7],
	    m->m_data[m->m_len - 6], m->m_data[m->m_len - 5],
	    m->m_data[m->m_len - 4], m->m_data[m->m_len - 3],
	    m->m_data[m->m_len - 2], m->m_data[m->m_len - 1]);
}

/*
 * Set mbuf checksum flag based on hardware.
 */
static void
ionic_rx_checksum(struct ifnet *ifp, struct mbuf *m,
    struct rxq_comp *comp, struct ionic_rx_stats *stats)
{
	bool ipv6;

	ipv6 = ionic_is_rx_ipv6(comp->pkt_type_color & IONIC_RXQ_COMP_PKT_TYPE_MASK);
	m->m_pkthdr.csum_flags = 0;

	if ((if_getcapenable(ifp) & (IFCAP_RXCSUM | IFCAP_RXCSUM_IPV6)) == 0)
		return;

	if (ipv6 || (comp->csum_flags & IONIC_RXQ_COMP_CSUM_F_IP_OK)) {
		m->m_pkthdr.csum_flags = CSUM_IP_CHECKED | CSUM_IP_VALID;
		m->m_pkthdr.csum_data = htons(0xffff);
		if ((comp->csum_flags & IONIC_RXQ_COMP_CSUM_F_TCP_OK) ||
			(comp->csum_flags & IONIC_RXQ_COMP_CSUM_F_UDP_OK)) {
			m->m_pkthdr.csum_flags |= CSUM_DATA_VALID | CSUM_PSEUDO_HDR;
		}
	}

	if (comp->csum_flags & IONIC_RXQ_COMP_CSUM_F_IP_OK)
		stats->csum_ip_ok++;

	if ((comp->csum_flags & IONIC_RXQ_COMP_CSUM_F_TCP_OK) ||
		(comp->csum_flags & IONIC_RXQ_COMP_CSUM_F_UDP_OK))
		stats->csum_l4_ok++;

	if (comp->csum_flags & IONIC_RXQ_COMP_CSUM_F_IP_BAD)
		stats->csum_ip_bad++;

	if ((comp->csum_flags & IONIC_RXQ_COMP_CSUM_F_TCP_BAD) ||
		(comp->csum_flags & IONIC_RXQ_COMP_CSUM_F_UDP_BAD))
		stats->csum_l4_bad++;
}

/*
 * Set RSS packet type.
 */
static void
ionic_rx_rss(struct mbuf *m, struct rxq_comp *comp, int qnum,
    struct ionic_rx_stats *stats, uint16_t rss_hash)
{

	if (!(comp->pkt_type_color & IONIC_RXQ_COMP_PKT_TYPE_MASK)) {
		m->m_pkthdr.flowid = qnum;
		M_HASHTYPE_SET(m, M_HASHTYPE_OPAQUE_HASH);
		stats->rss_unknown++;
		return;
	}

	m->m_pkthdr.flowid = comp->rss_hash;
#ifdef RSS
	/*
	 * Set the correct RSS type based on RSS hash config. If RSS
	 * is not enabled for that particular type, e.g. TCP/IPv4 but
	 * enabled for IPv4, set RSS type to IPv4.
	 */
	switch (comp->pkt_type_color & IONIC_RXQ_COMP_PKT_TYPE_MASK) {
	case PKT_TYPE_IPV4:
		if (rss_hash & IONIC_RSS_TYPE_IPV4) {
			M_HASHTYPE_SET(m, M_HASHTYPE_RSS_IPV4);
			stats->rss_ip4++;
		}
		break;
	case PKT_TYPE_IPV4_TCP:
		if (rss_hash & IONIC_RSS_TYPE_IPV4_TCP) {
			M_HASHTYPE_SET(m, M_HASHTYPE_RSS_TCP_IPV4);
			stats->rss_tcp_ip4++;
		} else if (rss_hash & IONIC_RSS_TYPE_IPV4) {
			M_HASHTYPE_SET(m, M_HASHTYPE_RSS_IPV4);
			stats->rss_ip4++;
		}
		break;
	case PKT_TYPE_IPV4_UDP:
		if (rss_hash & IONIC_RSS_TYPE_IPV4_UDP) {
			M_HASHTYPE_SET(m, M_HASHTYPE_RSS_UDP_IPV4);
			stats->rss_udp_ip4++;
		} else if (rss_hash & IONIC_RSS_TYPE_IPV4) {
			M_HASHTYPE_SET(m, M_HASHTYPE_RSS_IPV4);
			stats->rss_ip4++;
		}
		break;
	case PKT_TYPE_IPV6:
		if (rss_hash & IONIC_RSS_TYPE_IPV6) {
			M_HASHTYPE_SET(m, M_HASHTYPE_RSS_IPV6);
			stats->rss_ip6++;
		}
		break;
	case PKT_TYPE_IPV6_TCP:
		if (rss_hash & IONIC_RSS_TYPE_IPV6_TCP) {
			M_HASHTYPE_SET(m, M_HASHTYPE_RSS_TCP_IPV6);
			stats->rss_tcp_ip6++;
		} else if (rss_hash & IONIC_RSS_TYPE_IPV6) {
			M_HASHTYPE_SET(m, M_HASHTYPE_RSS_IPV6);
			stats->rss_ip6++;
		}
		break;
	case PKT_TYPE_IPV6_UDP:
		if (rss_hash & IONIC_RSS_TYPE_IPV6_UDP) {
			M_HASHTYPE_SET(m, M_HASHTYPE_RSS_UDP_IPV6);
			stats->rss_udp_ip6++;
		} else if (rss_hash & IONIC_RSS_TYPE_IPV6) {
			M_HASHTYPE_SET(m, M_HASHTYPE_RSS_IPV6);
			stats->rss_ip6++;
		}
		break;
	default:
		M_HASHTYPE_SET(m, M_HASHTYPE_OPAQUE_HASH);
		stats->rss_unknown++;
		break;
	}

	/*
	 * RSS hash type was not configured for these packet type,
	 * use OPAQUE_HASH.
	 */
	if (M_HASHTYPE_GET(m) == M_HASHTYPE_NONE) {
		M_HASHTYPE_SET(m, M_HASHTYPE_OPAQUE_HASH);
		stats->rss_unknown++;
	}
#else
	M_HASHTYPE_SET(m, M_HASHTYPE_OPAQUE_HASH);
	stats->rss_unknown++;
#endif
}

void
ionic_rx_input(struct ionic_rxque *rxq, struct ionic_rx_buf *rxbuf,
    struct rxq_comp *comp, struct rxq_desc *desc)
{
	struct mbuf *mb, *m = rxbuf->m;
	struct ionic_rx_stats *stats = &rxq->stats;
	struct ifnet *ifp = rxq->lif->netdev;
	int left, error;

	KASSERT(IONIC_RX_LOCK_OWNED(rxq), ("%s is not locked", rxq->name));

	if (comp->status) {
		IONIC_QUE_INFO(rxq, "RX Status %d\n", comp->status);
		stats->comp_err++;
		m_freem(m);
		rxbuf->m = NULL;
		return;
	}

	if (__IONIC_DEBUG) {
		if (comp->len > ETHER_MAX_FRAME(ifp, ETHERTYPE_VLAN, 1)) {
			IONIC_QUE_INFO(rxq,
			    "RX PKT TOO LARGE!  comp->len %d\n", comp->len);
			stats->length_err++;
			m_freem(m);
			rxbuf->m = NULL;
			return;
		}
		if (comp->len < ETHER_HDR_LEN + ETHER_CRC_LEN) {
			IONIC_QUE_INFO(rxq,
			    "Malformed ethernet packet!  comp->len %d\n",
			    comp->len);
			stats->length_err++;
			m_freem(m);
			rxbuf->m = NULL;
			return;
		}
	}

	stats->pkts++;
	stats->bytes += comp->len;

	bus_dmamap_sync(rxq->buf_tag, rxbuf->dma_map, BUS_DMASYNC_POSTREAD);

	prefetch(m->data - NET_IP_ALIGN);
	m->m_pkthdr.rcvif = ifp;
	/*
	 * Write the length here, if its chained mbufs for SG list,
	 * it will be overwritten.
	 */
	m->m_pkthdr.len = comp->len;
	m->m_len = comp->len;
	left = comp->len;
	/*
	 * Go through mbuf chain and adjust the length of chained mbufs
	 * depending on data length.
	 */
	if (rxbuf->sg_buf_len) {
		for (mb = m; mb != NULL; mb = mb->m_next) {
			mb->m_len = min(rxbuf->sg_buf_len, left);
			left -= mb->m_len;
			if (left == 0) {
				m_freem(mb->m_next);
				mb->m_next = NULL;
				break;
			}
		}
	}

	ionic_rx_checksum(ifp, m, comp, stats);

	/* Populate mbuf with h/w RSS hash, type etc. */
	ionic_rx_rss(m, comp, rxq->index, stats, rxq->lif->rss_types);

	if ((if_getcapenable(ifp) & IFCAP_VLAN_HWTAGGING) &&
		(comp->csum_flags & IONIC_RXQ_COMP_CSUM_F_VLAN)) {
		m->m_pkthdr.ether_vtag = le16toh(comp->vlan_tci);
		m->m_flags |= M_VLANTAG;
	}

	if ((if_getcapenable(ifp) & IFCAP_LRO) &&
		ionic_is_rx_tcp(comp->pkt_type_color & IONIC_RXQ_COMP_PKT_TYPE_MASK)) {
		if (rxq->lro.lro_cnt != 0) {
			if ((error = tcp_lro_rx(&rxq->lro, m, 0)) == 0) {
				rxbuf->m = NULL;
				return;
			}
		}
	}

	/* Send the packet to stack. */
	rxbuf->m = NULL;
	ifp->if_input(ifp, m);
}

/*
 * Allocate mbuf for receive path.
 */
int
ionic_rx_mbuf_alloc(struct ionic_rxque *rxq, int index, int len)
{
	bus_dma_segment_t *pseg, seg[IONIC_RX_MAX_SG_ELEMS + 1];
	struct ionic_rx_buf *rxbuf;
	struct rxq_desc *desc;
	struct rxq_sg_desc *sg;
	struct mbuf *m, *mb;
	struct ionic_rx_stats *stats = &rxq->stats;
	int i, nsegs, error, size;

	KASSERT(IONIC_RX_LOCK_OWNED(rxq), ("%s is not locked", rxq->name));
	rxbuf = &rxq->rxbuf[index];
	desc = &rxq->cmd_ring[index];
	sg = &rxq->sg_ring[index];

	bzero(desc, sizeof(*desc));
	bzero(sg, sizeof(*sg));
	KASSERT(rxbuf->m == NULL, ("rxuf %d is not empty", index));

	size = ionic_rx_sg_size ? ionic_rx_sg_size : len;
	m = m_getjcl(M_NOWAIT, MT_DATA, M_PKTHDR, size);
	if (m == NULL) {
		rxbuf->m = NULL;
		stats->alloc_err++;
		return (ENOMEM);
	}
	/*
	 * Set the size for:
	 * 1. Non-SGL
	 * 2. First mbuf of SGL.
	 */
	m->m_pkthdr.len = m->m_len = size;
	/*
	 * Set the size of rest of mbuf for SGL path.
	 */
	if (ionic_rx_sg_size) {
		rxbuf->sg_buf_len = ionic_rx_sg_size;
		mb = m;
		while (m->m_pkthdr.len < len) {
			mb = mb->m_next = m_getjcl(M_NOWAIT, MT_DATA, 0, size);
			if (mb == NULL) {
				rxbuf->m = NULL;
				m_freem(m);
				stats->alloc_err++;
				return (ENOMEM);
			}
			mb->m_len = size;
			m->m_pkthdr.len += size;
		}
	}

	error = bus_dmamap_load_mbuf_sg(rxq->buf_tag, rxbuf->dma_map, m,
	    seg, &nsegs, BUS_DMA_NOWAIT);
	if (error) {
		rxbuf->m = NULL;
		m_freem(m);
		stats->dma_map_err++;
		return (error);
	}

	bus_dmamap_sync(rxq->buf_tag, rxbuf->dma_map, BUS_DMASYNC_PREREAD);
	rxq->stats.mbuf_alloc++;
	rxbuf->m = m;

	desc->addr = seg[0].ds_addr;
	desc->len = seg[0].ds_len;
	desc->opcode = nsegs ? RXQ_DESC_OPCODE_SG : RXQ_DESC_OPCODE_SIMPLE;

	size = desc->len;
	for (i = 0; i < nsegs - 1; i++) {
		pseg = &seg[i + 1];
		if (!pseg->ds_len || pseg->ds_len > ionic_rx_sg_size)
			panic("seg[%d] 0x%lx %lu > %u\n", i, pseg->ds_addr,
			    pseg->ds_len, ionic_rx_sg_size);
		sg->elems[i].addr = pseg->ds_addr;
		sg->elems[i].len = pseg->ds_len;
		size += sg->elems[i].len;
	}

	if (ionic_rx_sg_size && size < len)
		panic("Rx SG size is not sufficient(%d) != %d", size, len);

	return (0);
}

/*
 * Free receive path mbuf.
 */
void
ionic_rx_mbuf_free(struct ionic_rxque *rxq, struct ionic_rx_buf *rxbuf)
{

	rxq->stats.mbuf_free++;
	bus_dmamap_sync(rxq->buf_tag, rxbuf->dma_map, BUS_DMASYNC_POSTREAD);
	bus_dmamap_unload(rxq->buf_tag, rxbuf->dma_map);
	bus_dmamap_destroy(rxq->buf_tag, rxbuf->dma_map);
	rxbuf->dma_map = NULL;

	m_freem(rxbuf->m);
	rxbuf->m = NULL;
}

/*
 * PerQ interrupt handler.
 */
static irqreturn_t
ionic_queue_isr(int irq, void *data)
{
	struct ionic_rxque *rxq = data;
	struct ionic_txque *txq = rxq->lif->txqs[rxq->index];
	struct ionic_dev *idev = &rxq->lif->ionic->idev;
	struct ionic_rx_stats* rxstats = &rxq->stats;
	int work_done, tx_work;

	KASSERT(rxq, ("rxq is NULL"));
	KASSERT((rxq->intr.index != INTR_INDEX_NOT_ASSIGNED),
	    ("%s has no interrupt resource", rxq->name));

	ionic_intr_mask(idev->intr_ctrl, rxq->intr.index,
	    IONIC_INTR_MASK_SET);
	IONIC_RX_LOCK(rxq);
	IONIC_RX_TRACE(rxq, "[%ld]comp index: %d head: %d tail: %d\n",
	    rxstats->isr_count, rxq->comp_index, rxq->head_index,
	    rxq->tail_index);

	rxstats->isr_count++;
	work_done = ionic_rx_clean(rxq, ionic_rx_clean_limit);
	/* Fill the receive ring. */
	if (IONIC_Q_REMAINING(rxq) >= ionic_rx_fill_threshold)
		ionic_rx_fill(rxq);
	/* Task handler will not be scheduled, flush LRO now. */
	if (work_done < ionic_rx_clean_limit) {
		tcp_lro_flush_all(&rxq->lro);
	}
	IONIC_RX_TRACE(rxq, "processed: %d packets\n", work_done);
	IONIC_RX_UNLOCK(rxq);

	IONIC_TX_LOCK(txq);
	tx_work = ionic_tx_clean(txq, ionic_tx_clean_limit);
	IONIC_TX_TRACE(txq, "processed: %d packets\n", tx_work);
	if (tx_work < ionic_tx_clean_limit) {
		ionic_start_xmit_locked(txq->lif->netdev, txq);
	}
	IONIC_TX_UNLOCK(txq);

	/* Not enough work items for task, unmask interrupt. */
	if ((work_done < ionic_rx_clean_limit) &&
	    (tx_work < ionic_tx_clean_limit)) {
		ionic_intr_credits(idev->intr_ctrl, rxq->intr.index,
		    (work_done + tx_work), IONIC_INTR_CRED_REARM);
	} else {
		ionic_intr_credits(idev->intr_ctrl, rxq->intr.index,
		    work_done + tx_work, 0);
		taskqueue_enqueue(rxq->taskq, &rxq->task);
	}

	return (IRQ_HANDLED);
}

/*
 * Interrupt task handler.
 */
static void
ionic_queue_task_handler(void *arg, int pendindg)
{
	struct ionic_rxque *rxq = arg;
	struct ionic_txque *txq = rxq->lif->txqs[rxq->index];
	struct ionic_dev *idev = &rxq->lif->ionic->idev;
	int work_done, tx_work;

	KASSERT(rxq, ("task handler called with rxq == NULL"));

	IONIC_RX_LOCK(rxq);
	if (rxq->lif->stop) {
		/*
		 * The task should return early if the interface should be
		 * stopped.  The rxq lock will guarantee that the stopped state
		 * is observed to change here when needed.
		 *
		 * When the stopped state is cleared, after that in the same
		 * thread the driver will do a final clean of the rxq, which
		 * involves taking and dropping the rxq lock.  When rxq the
		 * lock is acquired here after the final clean, we will also
		 * observe that the stopped state is cleared.
		 *
		 * Returning early from the task does not change the rxq or txq
		 * state, and does not unmask the isr.
		 */
		IONIC_RX_UNLOCK(rxq);
		return;
	}

	rxq->stats.task++;
	IONIC_RX_TRACE(rxq, "comp index: %d head: %d tail: %d\n",
	    rxq->comp_index, rxq->head_index, rxq->tail_index);

	/*
	 * Process all Rx frames.
	 */
	work_done = ionic_rx_clean(rxq, rxq->num_descs);
	/* Fill the receive ring. */
	if (IONIC_Q_REMAINING(rxq) >= ionic_rx_fill_threshold)
		ionic_rx_fill(rxq);
	IONIC_RX_TRACE(rxq, "processed %d packets\n", work_done);
	tcp_lro_flush_all(&rxq->lro);
	IONIC_RX_UNLOCK(rxq);

	IONIC_TX_LOCK(txq);
	ionic_start_xmit_locked(txq->lif->netdev, txq);
	tx_work = ionic_tx_clean(txq, txq->num_descs);
	IONIC_TX_TRACE(txq, "processed %d packets\n", tx_work);
	IONIC_TX_UNLOCK(txq);

	ionic_intr_credits(idev->intr_ctrl, rxq->intr.index,
	    (work_done + tx_work), IONIC_INTR_CRED_REARM);
}

static void
ionic_deferred_xmit_task(void *arg, int pending)
{
	struct ionic_txque *txq = arg;
	struct ifnet *ifp = txq->lif->netdev;

	IONIC_TX_LOCK(txq);
	if (!drbr_empty(ifp, txq->br))
		ionic_start_xmit_locked(ifp, txq);
	IONIC_TX_UNLOCK(txq);
}
/*
 * Setup queue interrupt handler.
 */
int
ionic_setup_rx_intr(struct ionic_rxque *rxq)
{
	int err, bind_cpu;
	struct ionic_lif *lif = rxq->lif;
	struct ionic_txque *txq = lif->txqs[rxq->index];
	char namebuf[16];
#ifdef RSS
	cpuset_t        cpu_mask;

	bind_cpu = rss_getcpu(rxq->index % rss_getnumbuckets());
	CPU_SETOF(bind_cpu, &cpu_mask);
#else
	bind_cpu = rxq->index;
#endif

	TASK_INIT(&rxq->tx_task, 0, ionic_deferred_xmit_task, txq);
	TASK_INIT(&rxq->task, 0, ionic_queue_task_handler, rxq);
	snprintf(namebuf, sizeof(namebuf), "task-%s", rxq->name);
	rxq->taskq = taskqueue_create(namebuf, M_NOWAIT,
	    taskqueue_thread_enqueue, &rxq->taskq);

#ifdef RSS
	err = taskqueue_start_threads_cpuset(&rxq->taskq, 1, PI_NET, &cpu_mask,
	    "%s (que %s)", device_get_nameunit(lif->ionic->dev),
	    rxq->intr.name);
#else
	err = taskqueue_start_threads(&rxq->taskq, 1, PI_NET, NULL,
	    "%s (que %s)", device_get_nameunit(lif->ionic->dev),
	    rxq->intr.name);
#endif

	if (err) {
		IONIC_QUE_ERROR(rxq, "failed to create taskqueue, error: %d\n",
			err);
		taskqueue_free(rxq->taskq);
		return (err);
	}

	/* Legacy interrupt allocation is done once. */
	if (ionic_enable_msix == 0)
		return (0);

	err = request_irq(rxq->intr.vector, ionic_queue_isr, 0, rxq->intr.name, rxq);
	if (err) {
		IONIC_QUE_ERROR(rxq, "request_irq failed, error: %d\n", err);
		taskqueue_free(rxq->taskq);
		return (err);
	}

	err = bind_irq_to_cpu(rxq->intr.vector, bind_cpu);
	if (err) {
		IONIC_QUE_WARN(rxq, "failed to bind to cpu%d\n", bind_cpu);
	}
	IONIC_QUE_INFO(rxq, "bound to cpu%d\n", bind_cpu);

	return (0);
}

/**************************** Tx handling **********************/
static bool
ionic_tx_avail(struct ionic_txque *txq, int want)
{
	int avail;

	avail = ionic_desc_avail(txq->num_descs, txq->head_index,
	    txq->tail_index);

	return (avail > want);
}

/*
 * Legacy interrupt handler for device.
 */
static irqreturn_t
ionic_legacy_isr(int irq, void *data)
{
	struct ionic_lif *lif = data;
	struct ionic_dev *idev = &lif->ionic->idev;
	struct ifnet *ifp;
	struct ionic_adminq* adminq;
	struct ionic_notifyq* notifyq;
	struct ionic_txque *txq;
	struct ionic_rxque *rxq;
	uint64_t status;
	int work_done, i, tx_work;

	ifp = lif->netdev;
	adminq = lif->adminq;
	notifyq = lif->notifyq;
	status = readq(idev->intr_status);

	IONIC_NETDEV_INFO(ifp, "legacy INTR status(%p): 0x%lx\n",
	    idev->intr_status, status);

	if (status == 0) {
		/* Invoked for no reason. */
		lif->spurious++;
		return (IRQ_NONE);
	}

	if (status & BIT_ULL(adminq->intr.index)) {
		IONIC_ADMIN_LOCK(adminq);
		ionic_intr_mask(idev->intr_ctrl, adminq->intr.index,
		    IONIC_INTR_MASK_SET);
		work_done = ionic_adminq_clean(adminq, adminq->num_descs);
		ionic_intr_credits(idev->intr_ctrl, adminq->intr.index,
		    work_done, IONIC_INTR_CRED_REARM);
		IONIC_ADMIN_UNLOCK(adminq);
	}

	if (status & BIT_ULL(notifyq->intr.index)) {
		ionic_intr_mask(idev->intr_ctrl, notifyq->intr.index,
		    IONIC_INTR_MASK_SET);
		work_done = ionic_notifyq_clean(notifyq);
		ionic_intr_credits(idev->intr_ctrl, notifyq->intr.index,
		    work_done, IONIC_INTR_CRED_REARM);
	}

	for (i = 0; i < lif->nrxqs; i++) {
		rxq = lif->rxqs[i];
		txq = lif->txqs[i];

		KASSERT((rxq->intr.index != INTR_INDEX_NOT_ASSIGNED),
		    ("%s has no interrupt resource", rxq->name));
		IONIC_QUE_INFO(rxq, "Interrupt source index: %d\n",
		    rxq->intr.index);

		if ((status & BIT_ULL(rxq->intr.index)) == 0)
			continue;

		IONIC_RX_LOCK(rxq);
		ionic_intr_mask(idev->intr_ctrl, rxq->intr.index,
		    IONIC_INTR_MASK_SET);
		work_done = ionic_rx_clean(rxq, rxq->num_descs);
		tcp_lro_flush_all(&rxq->lro);
		ionic_rx_fill(rxq);
		IONIC_RX_UNLOCK(rxq);

		IONIC_TX_LOCK(txq);
		tx_work = ionic_tx_clean(txq, txq->num_descs);
		IONIC_TX_UNLOCK(txq);

		ionic_intr_credits(idev->intr_ctrl, rxq->intr.index,
		    (work_done + tx_work), IONIC_INTR_CRED_REARM);
	}

	return (IRQ_HANDLED);
}

/*
 * Setup legacy interrupt.
 */
int
ionic_setup_legacy_intr(struct ionic_lif *lif)
{
	int error = 0;

	error = request_irq(lif->ionic->pdev->irq, ionic_legacy_isr, 0,
	    "legacy", lif);
	return (error);
}

static int
ionic_get_header_size(struct ionic_txque *txq, struct mbuf *mb,
    uint16_t *eth_type, int *proto, int *hlen)
{
	struct ether_vlan_header *eh;
	struct tcphdr *th;
	struct ip *ip;
	int ip_hlen, tcp_hlen;
	struct ip6_hdr *ip6;
	struct ionic_tx_stats *stats = &txq->stats;
	int eth_hdr_len;

	eh = mtod(mb, struct ether_vlan_header *);
	if (mb->m_len < ETHER_HDR_LEN)
		return (EINVAL);

	if (eh->evl_encap_proto == htons(ETHERTYPE_VLAN)) {
		*eth_type = ntohs(eh->evl_proto);
		eth_hdr_len = ETHER_HDR_LEN + ETHER_VLAN_ENCAP_LEN;
	} else {
		*eth_type = ntohs(eh->evl_encap_proto);
		eth_hdr_len = ETHER_HDR_LEN;
	}

	if (mb->m_len < eth_hdr_len)
		return (EINVAL);

	switch (*eth_type) {
#if defined(INET)
	case ETHERTYPE_IP:
		ip = (struct ip *)(mb->m_data + eth_hdr_len);
		if (mb->m_len < eth_hdr_len + sizeof(*ip))
			return (EINVAL);
		*proto = ip->ip_p;
		ip_hlen = ip->ip_hl << 2;
		eth_hdr_len += ip_hlen;
		stats->tso_ipv4++;
		break;
#endif

#if defined(INET6)
	case ETHERTYPE_IPV6:
		ip6 = (struct ip6_hdr *)(mb->m_data + eth_hdr_len);
		if (mb->m_len < eth_hdr_len + sizeof(*ip6))
			return (EINVAL);
		*proto = ip6->ip6_nxt;
		eth_hdr_len += sizeof(*ip6);
		stats->tso_ipv6++;
		break;
#endif

	default:
		return (EINVAL);
	}

	if (mb->m_len < eth_hdr_len + sizeof(*th))
		return (EINVAL);

	th = (struct tcphdr *)(mb->m_data + eth_hdr_len);
	tcp_hlen = th->th_off << 2;
	eth_hdr_len += tcp_hlen;

	if (mb->m_len < eth_hdr_len)
		return (EINVAL);

	*hlen = eth_hdr_len;

	return (0);
}

/*
 * Non-TSO transmit path.
 */
static int
ionic_tx_setup(struct ionic_txque *txq, struct mbuf **m_headp)
{
	struct txq_desc *desc;
	struct txq_sg_elem *sg;
	struct ionic_tx_stats *stats = &txq->stats;
	struct ionic_tx_buf *txbuf;
	struct mbuf *m, *newm;
	bool offload = false;
	int i, error, index, nsegs;
	bus_dma_segment_t  seg[IONIC_MAX_SEGMENTS];
	uint8_t opcode, flags = 0;

	IONIC_TX_TRACE(txq, "Enter head: %d tail: %d\n", txq->head_index,
	    txq->tail_index);

	index = txq->head_index;
	desc = &txq->cmd_ring[index];
	txbuf = &txq->txbuf[index];
	sg = &txq->sg_ring[index * txq->sg_desc_stride];
	bzero(sg, sizeof(*sg) * txq->sg_desc_stride);

	error = bus_dmamap_load_mbuf_sg(txq->buf_tag, txbuf->dma_map, *m_headp,
	    seg, &nsegs, BUS_DMA_NOWAIT);
	if (!error && nsegs > txq->max_sg_elems + 1) {
		bus_dmamap_unload(txq->buf_tag, txbuf->dma_map);
		stats->pkt_sparse++;
		error = EFBIG;
	}

	if (error == EFBIG) {
		stats->mbuf_defrag++;
		newm = m_defrag(*m_headp, M_NOWAIT);
		if (newm == NULL) {
			stats->mbuf_defrag_err++;
			m_freem(*m_headp);
			*m_headp = NULL;
			IONIC_QUE_ERROR(txq, "failed to defrag mbuf\n");
			return (ENOBUFS);
		}
		*m_headp = newm;

		error = bus_dmamap_load_mbuf_sg(txq->buf_tag, txbuf->dma_map, newm,
		    seg, &nsegs, BUS_DMA_NOWAIT);
		if (!error && nsegs > txq->max_sg_elems + 1) {
			bus_dmamap_unload(txq->buf_tag, txbuf->dma_map);
			stats->pkt_defrag_sparse++;
			error = EFBIG;
		}
	}

	if (error) {
		m_freem(*m_headp);
		*m_headp = NULL;
		stats->dma_map_err++;
		IONIC_QUE_ERROR(txq, "setting up dma map failed, segs %d/%d, error: %d\n",
		    nsegs, txq->max_sg_elems, error);
		return (error);
	}

	if (!ionic_tx_avail(txq, nsegs)) {
		stats->no_descs++;
		bus_dmamap_unload(txq->buf_tag, txbuf->dma_map);
		IONIC_TX_TRACE(txq, "No space available, head: %u tail: %u nsegs: %d\n",
		    txq->head_index, txq->tail_index, nsegs);
		return (ENOSPC);
	}

	m = *m_headp;
	bus_dmamap_sync(txq->buf_tag, txbuf->dma_map, BUS_DMASYNC_PREWRITE);

	txbuf->pa_addr = seg[0].ds_addr;
	txbuf->m = m;
	txbuf->timestamp = rdtsc();

	if (m->m_pkthdr.csum_flags & CSUM_IP) {
		flags |= IONIC_TXQ_DESC_FLAG_CSUM_L3;
		offload = true;
	}

	if (m->m_pkthdr.csum_flags &
	    (CSUM_IP | CSUM_TCP | CSUM_UDP | CSUM_UDP_IPV6 | CSUM_TCP_IPV6)) {
		flags |= IONIC_TXQ_DESC_FLAG_CSUM_L4;
		offload = true;
	}

	if (m->m_flags & M_VLANTAG) {
		flags |= IONIC_TXQ_DESC_FLAG_VLAN;
		desc->vlan_tci = htole16(m->m_pkthdr.ether_vtag);
	}

	if (offload) {
		opcode = IONIC_TXQ_DESC_OPCODE_CSUM_HW;
		stats->csum_offload++;
	} else {
		opcode = IONIC_TXQ_DESC_OPCODE_CSUM_NONE;
		stats->no_csum_offload++;
	}

	desc->cmd = encode_txq_desc_cmd(opcode, flags, nsegs - 1, txbuf->pa_addr);
	desc->len = seg[0].ds_len;
	/* Populate sg list with rest of segments. */
	for (i = 0; i < nsegs - 1; i++, sg++) {
		sg->addr = seg[i + 1].ds_addr;
		sg->len = seg[i + 1].ds_len;
	}

	stats->pkts++;
	stats->bytes += m->m_len;

	txq->head_index = IONIC_MOD_INC(txq, head_index);

	ionic_tx_ring_doorbell(txq, txq->head_index);

	return (0);
}

/*
 * Validate the TSO descriptors.
 */
static void
ionic_tx_tso_dump(struct ionic_txque *txq, struct mbuf *m,
    bus_dma_segment_t  *seg, int nsegs, int stop_index)
{
	struct txq_desc *desc;
	struct txq_sg_elem *sg;
	struct ionic_tx_buf *txbuf;
	int i, j, len;

	IONIC_TX_TRACE(txq, "--------------------------------------\n");
	IONIC_TX_TRACE(txq, "TSO: mbuf VA: %p nsegs: %d length: %d\n",
	    m, nsegs, m->m_pkthdr.len);

	for (i = 0; i < nsegs; i++) {
		IONIC_TX_TRACE(txq, "mbuf seg[%d] pa: 0x%lx len: %ld\n",
		    i, seg[i].ds_addr, seg[i].ds_len);
	}

	IONIC_TX_TRACE(txq, "start: %d stop: %d\n", txq->head_index, stop_index);
	len = 0;
	for (i = txq->head_index; i != stop_index; i = (i + 1) % txq->num_descs) {
		txbuf = &txq->txbuf[i];
		desc = &txq->cmd_ring[i];
		sg = &txq->sg_ring[i * txq->sg_desc_stride];
		len += desc->len;
		/* Expected SOF */
		if ((i == txq->head_index) &&
		    ((desc->cmd & IONIC_TXQ_DESC_FLAG_TSO_SOT) == 0)) {
			IONIC_QUE_ERROR(txq, "TSO w/o start of frame\n");
		}
		/* Unexpected SOF */
		if ((i != txq->head_index) &&
		    (desc->cmd & IONIC_TXQ_DESC_FLAG_TSO_SOT)) {
			IONIC_QUE_ERROR(txq, "TSO unexpected start of frame\n");
		}
		/* Expected EOF */
		if ((((i + 1) % txq->num_descs) == stop_index) &&
		    ((desc->cmd & IONIC_TXQ_DESC_FLAG_TSO_EOT) == 0)) {
			IONIC_QUE_ERROR(txq, "TSO w/o end of frame\n");
		}
		/* Unexpected EOF */
		if ((((i + 1) % txq->num_descs) != stop_index) &&
		    (desc->cmd & IONIC_TXQ_DESC_FLAG_TSO_EOT)) {
			IONIC_QUE_ERROR(txq, "TSO unexpected end of frame\n");
		}

		IONIC_TX_TRACE(txq, "TSO desc[%d] length: %d" " cmd:%lx mss:%d hdr_len:%d mbuf:%p\n",
		    i, desc->len, desc->cmd, desc->mss, desc->hdr_len, txbuf->m);

		for (j = 0; j < txq->max_sg_elems && sg->len; j++, sg++) {
			len += sg->len;
			IONIC_TX_TRACE(txq, "  TSO sg[%d] pa: 0x%lx length: %d\n",
			    j, sg->addr, sg->len);
		}
	}
	if (len != m->m_pkthdr.len)
		IONIC_QUE_ERROR(txq, "descriptor length mismatch %d(actual: %d)\n",
		    len, m->m_pkthdr.len);
}

/* Is any MSS size chunk spanning more than the supported number of SGLs? */
static inline int
ionic_tso_is_sparse(struct ionic_txque *txq, struct mbuf *m,
    bus_dma_segment_t *segs, int nsegs, int hdr_len)
{
	int frag_cnt = 0, frag_len = 0;
	int seg_rem = 0, seg_idx = 0;
	int tso_rem = 0, tcp_seg_rem = 0;

	if (nsegs <= txq->max_sg_elems + 1)
		return (0);

	tso_rem = m->m_pkthdr.len;
	/* first tcp segment descriptor has header + mss_payload */
	tcp_seg_rem = m->m_pkthdr.tso_segsz + hdr_len;
	seg_rem = segs[seg_idx].ds_len;
	/* until all data in tso is exhausted */
	while (tso_rem > 0) {
		/* until a full tcp segment can be created */
		while (tcp_seg_rem > 0 && tso_rem > 0) {
			frag_cnt++;
			/* is this tcp segment too fragmented? */
			if (frag_cnt > txq->max_sg_elems + 1)
				return (EFBIG);
			/* is the mbuf segment is exhausted? */
			if (seg_rem == 0) {
				seg_idx++;
				/* we ran out of mbufs before all tso data is exhausted?
				   may be length is wrong in mbuf */
				if (seg_idx == nsegs)
					return (EIO);
				/* use the next mbuf segment */
				seg_rem = segs[seg_idx].ds_len;
			}
			frag_len = min(seg_rem, tcp_seg_rem);
			seg_rem -= frag_len;
			tcp_seg_rem -= frag_len;
			tso_rem -= frag_len;
		}
		/* move to the next tcp segment */
		frag_cnt = 0;
		/* subsequent tcp segment descriptor only has mss_payload */
		tcp_seg_rem = m->m_pkthdr.tso_segsz;
	}

	return (0);
}

static int
ionic_tx_tso_setup(struct ionic_txque *txq, struct mbuf **m_headp)
{
	struct mbuf *m, *newm;
	struct ionic_tx_stats *stats = &txq->stats;
	struct ionic_tx_buf *first_txbuf, *txbuf;
	struct txq_desc *desc;
	struct txq_sg_elem *sg;
	bus_dma_segment_t seg[IONIC_MAX_TSO_SEGMENTS];
	dma_addr_t addr;
	int i, j, index, hdr_len, proto, error, nsegs, num_descs;
	uint32_t mss, frag_offset, desc_len, remain_len, frag_remain_len, desc_max_size;
	uint16_t len, eth_type;
	uint8_t flags;
	bool start, end, has_vlan;

	IONIC_TX_TRACE(txq, "Enter head: %d tail: %d\n",
	    txq->head_index, txq->tail_index);

	if ((error = ionic_get_header_size(txq, *m_headp, &eth_type, &proto,
	    &hdr_len))) {
		IONIC_QUE_ERROR(txq,
		    "mbuf pkt discarded, type: %x proto: %x hdr_len: %u\n",
		    eth_type, proto, hdr_len);
		ionic_dump_mbuf(*m_headp);
		stats->bad_ethtype++;
		m_freem(*m_headp);
		*m_headp = NULL;
		return (error);
	}

	if (proto != IPPROTO_TCP) {
		return (EINVAL);
	}

	index = txq->head_index;
	first_txbuf = &txq->txbuf[index];

	error = bus_dmamap_load_mbuf_sg(txq->tso_buf_tag,
	    first_txbuf->tso_dma_map, *m_headp, seg, &nsegs, BUS_DMA_NOWAIT);
	if (!error && ionic_tso_is_sparse(txq, *m_headp, seg, nsegs, hdr_len)) {
		bus_dmamap_unload(txq->tso_buf_tag, first_txbuf->tso_dma_map);
		stats->tso_sparse++;
		error = EFBIG;
	}

	if (error == EFBIG) {
		stats->mbuf_defrag++;
		newm = m_defrag(*m_headp, M_NOWAIT);
		if (newm == NULL) {
			stats->mbuf_defrag_err++;
			m_freem(*m_headp);
			*m_headp = NULL;
			IONIC_QUE_ERROR(txq, "mbuf_defrag returned NULL\n");
			return (ENOBUFS);
		}
		*m_headp = newm;
		error = bus_dmamap_load_mbuf_sg(txq->tso_buf_tag,
		    first_txbuf->tso_dma_map, newm, seg, &nsegs,
		    BUS_DMA_NOWAIT);
		if (!error &&
		    ionic_tso_is_sparse(txq, *m_headp, seg, nsegs, hdr_len)) {
			bus_dmamap_unload(txq->tso_buf_tag,
			    first_txbuf->tso_dma_map);
			stats->tso_defrag_sparse++;
			error = EFBIG;
		}
	}

	if (error) {
		m_freem(*m_headp);
		*m_headp = NULL;
		stats->dma_map_err++;
		IONIC_QUE_ERROR(txq, "dma map failed, seg %d/%d , error: %d\n",
		    nsegs, txq->max_sg_elems, error);
		return (error);
	}

	m = *m_headp;
	remain_len = m->m_pkthdr.len;
	mss = m->m_pkthdr.tso_segsz;

	/* Calculate the number of descriptors needed for the whole TSO */
	num_descs = (remain_len - hdr_len + mss - 1) / mss;
	if (num_descs > stats->tso_max_descs)
		stats->tso_max_descs = num_descs;
	if (num_descs > txq->num_descs) {
		stats->tso_too_big++;
		bus_dmamap_unload(txq->tso_buf_tag, first_txbuf->tso_dma_map);
		IONIC_TX_TRACE(txq, "TSO too big, num_descs: %d request: %d\n",
		    txq->num_descs, num_descs);
		return (ENOSPC);
	}
	if (!ionic_tx_avail(txq, num_descs)) {
		stats->tso_no_descs++;
		bus_dmamap_unload(txq->tso_buf_tag, first_txbuf->tso_dma_map);
		IONIC_TX_TRACE(txq,
		    "No space avail, head: %u tail: %u num_descs: %d\n",
		    txq->head_index, txq->tail_index, num_descs);
		return (ENOSPC);
	}

	bus_dmamap_sync(txq->tso_buf_tag, first_txbuf->tso_dma_map,
	    BUS_DMASYNC_PREWRITE);
	stats->tso_max_size = max(stats->tso_max_size, m->m_pkthdr.len);
	index = txq->head_index;
	frag_offset = 0;
	frag_remain_len = seg[0].ds_len;

	has_vlan = (m->m_flags & M_VLANTAG) ? 1 : 0;
	start = true;
	/*
	 * Loop through all segments of mbuf and create mss size descriptors.
	 * First descriptor points to header.
	 */
	for (i = 0; i < nsegs && remain_len > 0;) {
		desc = &txq->cmd_ring[index];
		txbuf = &txq->txbuf[index];
		sg = &txq->sg_ring[index * txq->sg_desc_stride];
		bzero(sg, sizeof(*sg) * txq->sg_desc_stride);

		desc_max_size = start ? (mss + hdr_len) : mss;
		desc_len = min(frag_remain_len, desc_max_size);
		frag_remain_len -= desc_len;
		addr = seg[i].ds_addr + frag_offset;
		len = desc_len;

		/* Check if anything left to transmit from this fragment. */
		if (frag_remain_len <= 0) {
			/* This fragment is used up. Use the next one. */
			i++;
			frag_remain_len = seg[i].ds_len;
			frag_offset = 0;
		} else {
			/* There are bytes left over in this fragment. */
			frag_offset += desc_len;
		}

		/*
		 * Now populate SG list, with the remaining fragments
		 * upto MSS size.
		 */
		for (j = 0; j < txq->max_sg_elems && (i < nsegs) &&
		    desc_len < desc_max_size; j++, sg++) {
			sg->addr = seg[i].ds_addr + frag_offset;
			sg->len = min(frag_remain_len,
			    (desc_max_size - desc_len));
			frag_remain_len -= sg->len;
			frag_offset += sg->len;
			desc_len += sg->len;

			/*
			 * Check if anything left to transmit from
			 * this fragment.
			 */
			if (frag_remain_len <= 0) {
				/* Use the next fragment. */
				i++;
				frag_offset = 0;
				frag_remain_len = seg[i].ds_len;
			}
		}

		remain_len -= desc_len;
		end = remain_len ? 0 : 1;
		/* Tx completion will use the last descriptor. */
		if (end) {
			txbuf->pa_addr = addr;
			txbuf->m = m;
			txbuf->tso_dma_map = first_txbuf->tso_dma_map;
			txbuf->timestamp = rdtsc();
			txbuf->is_tso = 1;
		} else {
			txbuf->m = NULL;
			txbuf->pa_addr = 0;
		}

		flags = 0;
		flags |= has_vlan ? IONIC_TXQ_DESC_FLAG_VLAN : 0;
		flags |= start ? IONIC_TXQ_DESC_FLAG_TSO_SOT : 0;
		flags |= end ? IONIC_TXQ_DESC_FLAG_TSO_EOT : 0;

		desc->cmd = encode_txq_desc_cmd(IONIC_TXQ_DESC_OPCODE_TSO,
		     flags, j, addr);
		desc->len = len;
		desc->vlan_tci = htole16(m->m_pkthdr.ether_vtag);
		desc->hdr_len = hdr_len;
		desc->mss = mss;

		/* Toggle the start flag. */
		start = false;

		stats->tso_max_sg = max(stats->tso_max_sg, j);
		stats->pkts++;
		stats->bytes += desc_len;

		num_descs--;
		index = (index + 1) % txq->num_descs;
		if (index % ionic_tx_stride == 0)
			ionic_tx_ring_doorbell(txq, index);
	}

	KASSERT(end, ("No end of frame"));
	KASSERT(num_descs == 0, ("unexpected num_descs %d", num_descs));
	if (__IONIC_TSO_DEBUG)
		ionic_tx_tso_dump(txq, m, seg, nsegs, index);
	txq->head_index = index;
	if (txq->head_index % ionic_tx_stride)
		ionic_tx_ring_doorbell(txq, txq->head_index);

	IONIC_TX_TRACE(txq, "Exit head: %d tail: %d\n",
		txq->head_index, txq->tail_index);

	return (0);
}


static int
ionic_xmit(struct ifnet *ifp, struct ionic_txque *txq, struct mbuf **m)
{
	struct ionic_tx_stats *stats;
	int err;

	if ((ifp->if_drv_flags & IFF_DRV_RUNNING) == 0)
		return (ENETDOWN);

	stats = &txq->stats;

	/* Send a copy of the frame to the BPF listener */
	ETHER_BPF_MTAP(ifp, *m);

	if ((*m)->m_pkthdr.csum_flags & CSUM_TSO)
		err = ionic_tx_tso_setup(txq, m);
	else
		err = ionic_tx_setup(txq, m);

	txq->full = (err == ENOSPC);
	if (txq->full && !txq->wdog_start)
		txq->wdog_start = ticks;

	/*
	 * Reset the timer if queue is not full,
	 * we might be recovering from TxQ full condition.
	 */	
	if (!txq->full)
		txq->wdog_start = 0;

	return (err);
}

static void
ionic_start_xmit_locked(struct ifnet *ifp, struct ionic_txque *txq)
{
	struct mbuf *m;
	struct ionic_tx_stats *stats;
	int err;

	stats = &txq->stats;

	while ((m = drbr_peek(ifp, txq->br)) != NULL) {
		if ((err = ionic_xmit(ifp, txq, &m)) != 0) {
			if (m == NULL) {
				drbr_advance(ifp, txq->br);
			} else {
				stats->re_queue++;
				drbr_putback(ifp, txq->br, m);
			}
			break;
		}
		drbr_advance(ifp, txq->br);
	}

	/* On successful ionic_xmit, set the watchdog timer */
	if (!err)
		txq->wdog_start = ticks;
}

int
ionic_start_xmit(struct ifnet *ifp, struct mbuf *m)
{
	struct ionic_lif *lif = if_getsoftc(ifp);
	struct ionic_txque *txq;
	struct ionic_rxque *rxq;
	int  err, qid = 0;
#ifdef RSS
	int bucket;
#endif

	if (M_HASHTYPE_GET(m) != M_HASHTYPE_NONE) {
#ifdef RSS
		if (rss_hash2bucket(m->m_pkthdr.flowid,
			M_HASHTYPE_GET(m), &bucket) == 0)
			qid = bucket % lif->ntxqs;
	 	else
#endif
		qid = (m->m_pkthdr.flowid % 128) % lif->ntxqs;
	}

	txq = lif->txqs[qid];
	rxq = lif->rxqs[qid];

	err = drbr_enqueue(ifp, txq->br, m);
	if (err)
		return (err);

	if (IONIC_TX_TRYLOCK(txq)) {
		ionic_start_xmit_locked(ifp, txq);
		IONIC_TX_UNLOCK(txq);
	} else
		taskqueue_enqueue(rxq->taskq, &rxq->tx_task);

	return (0);
}

/*
 * Provide various statistics to stack.
 */
static uint64_t
ionic_get_counter(struct ifnet *ifp, ift_counter cnt)
{
	struct ionic_lif *lif = if_getsoftc(ifp);
	struct lif_stats *hwstat = &lif->info->stats;

	switch (cnt) {
	case IFCOUNTER_IPACKETS:
		return (hwstat->rx_ucast_packets +
			hwstat->rx_mcast_packets +
			hwstat->rx_bcast_packets);

	case IFCOUNTER_IERRORS:
		return (hwstat->rx_queue_disabled +
			hwstat->rx_queue_empty +
			hwstat->rx_desc_fetch_error +
			hwstat->rx_desc_data_error);

	case IFCOUNTER_OPACKETS:
		return (hwstat->tx_ucast_packets +
			hwstat->tx_mcast_packets +
			hwstat->tx_bcast_packets);

	case IFCOUNTER_OERRORS:
		return (hwstat->tx_queue_disabled +
			hwstat->tx_desc_fetch_error +
			hwstat->tx_desc_data_error);

	case IFCOUNTER_COLLISIONS:
		return (0);

	case IFCOUNTER_IBYTES:
		return (hwstat->rx_ucast_bytes +
			hwstat->rx_mcast_bytes +
			hwstat->rx_bcast_bytes);

	case IFCOUNTER_OBYTES:
		return (hwstat->tx_ucast_bytes +
			hwstat->tx_mcast_bytes +
			hwstat->tx_bcast_bytes);

	case IFCOUNTER_IMCASTS:
		return (hwstat->rx_mcast_packets +
			hwstat->rx_bcast_packets);

	case IFCOUNTER_OMCASTS:
		return (hwstat->tx_mcast_packets +
			hwstat->tx_bcast_packets);

	case IFCOUNTER_IQDROPS:
		return (hwstat->rx_ucast_drop_packets +
			hwstat->rx_mcast_drop_packets +
			hwstat->rx_bcast_drop_packets);

	case IFCOUNTER_OQDROPS:
		return (hwstat->tx_ucast_drop_packets +
			hwstat->tx_mcast_drop_packets +
			hwstat->tx_bcast_drop_packets);

	default:
		return (if_get_counter_default(ifp, cnt));
	}

}

static void
ionic_tx_qflush(struct ifnet *ifp)
{
	struct ionic_lif *lif = if_getsoftc(ifp);
	struct ionic_txque *txq;
	struct mbuf *m;
	int i;

	for (i = 0; i < lif->ntxqs; i++) {
		txq = lif->txqs[i];
		IONIC_TX_LOCK(txq);
		while ((m = buf_ring_dequeue_sc(txq->br)) != NULL)
			m_freem(m);
		IONIC_TX_UNLOCK(txq);
	}

	if_qflush(ifp);
}

static void
ionic_if_init(void *arg)
{
	struct ionic_lif *lif = arg;

	IONIC_LIF_LOCK(lif);
	ionic_open_or_stop(lif);
	IONIC_LIF_UNLOCK(lif);
}

int
ionic_lif_netdev_alloc(struct ionic_lif *lif, int ndescs)
{
	struct ifnet *ifp;

	KASSERT(lif->ionic, ("lif: %s ionic == NULL", lif->name));

	ifp = if_alloc(IFT_ETHER);
	if (ifp == NULL) {
		dev_err(lif->ionic->dev,
		    "Cannot allocate ifnet, aborting\n");
		return (-ENOMEM);
	}

	if_initname(ifp, "ionic", device_get_unit(lif->ionic->dev->bsddev));

	ifp->if_softc = lif;
	ifp->if_mtu = ETHERMTU;
	ifp->if_init = ionic_if_init;
	ifp->if_flags = IFF_BROADCAST | IFF_SIMPLEX | IFF_MULTICAST;
	ifp->if_ioctl = ionic_ioctl;
	ifp->if_transmit = ionic_start_xmit;
	ifp->if_qflush = ionic_tx_qflush;
	ifp->if_snd.ifq_maxlen = ndescs;

	if (lif->ionic->is_mgmt_nic)
		ifp->if_baudrate = IF_Gbps(1);
	else
		ifp->if_baudrate = IF_Gbps(100);

	if_setgetcounterfn(ifp, ionic_get_counter);
	/* Capabilities are set later on. */

	/* Connect lif to ifnet, taking a long-lived reference */
	if_ref(ifp);
	lif->netdev = ifp;
#ifdef NETAPP_PATCH
	lif->iff_up = (ifp->if_flags & IFF_UP) != 0;
#endif
	return (0);
}

#define IONIC_IFP_MAX_SLEEPS 40
void
ionic_lif_netdev_free(struct ionic_lif *lif)
{
	struct ifnet *ifp = lif->netdev;
	uint32_t sleeps = 0;
	int chan;

	/* if_free() drops second-to-last reference and sets IFF_DYING flag */
	if_free(ifp);

	/* wait until ifp callbacks complete */
	while (ifp->if_refcount > 1 && sleeps < IONIC_IFP_MAX_SLEEPS) {
		sleeps++;
		tsleep(&chan, 0, "ifpdrn", HZ / 10);
	}
	if (ifp->if_refcount > 1) {
		IONIC_NETDEV_ERROR(ifp, "slept %u times, %u refs leaked\n",
				   sleeps, ifp->if_refcount - 1);
	} else if (sleeps > 0) {
		IONIC_NETDEV_INFO(ifp, "slept %u times\n", sleeps);
	}

	/* Finally, drop last reference */
	if_rele(ifp);
}

/*
 * Sysctl to control interrupt coalescing timer.
 */
static int
ionic_intr_coal_handler(SYSCTL_HANDLER_ARGS)
{
	struct ionic_lif *lif = oidp->oid_arg1;
	int coal, error;

	error = sysctl_handle_int(oidp, &coal, 0, req);
	if (error || !req->newptr)
		return (EINVAL);

	return (ionic_setup_intr_coal(lif, coal));
}

/*
 * Dump MAC filter list.
 */
static int
ionic_filter_sysctl(SYSCTL_HANDLER_ARGS)
{
	struct ionic_lif *lif;
	struct sbuf *sb;
	struct ionic_mc_addr *mc;
	struct rx_filter *f;
	int i, err;

	lif = oidp->oid_arg1;
	err = sysctl_wire_old_buffer(req, 0);
	if (err)
		return (err);

	sb = sbuf_new_for_sysctl(NULL, NULL, 4096, req);
	if (sb == NULL)
		return (ENOMEM);

	for (i = 0; i < lif->num_mc_addrs; i++) {
		mc = &lif->mc_addrs[i];
		f = ionic_rx_filter_by_addr(lif, mc->addr);
		sbuf_printf(sb, "\nMAC[%d](%d) %02x:%02x:%02x:%02x:%02x:%02x",
		    i, f ? f->filter_id : -1, mc->addr[0], mc->addr[1],
		    mc->addr[2], mc->addr[3], mc->addr[4], mc->addr[5]);
	}

	err = sbuf_finish(sb);
	sbuf_delete(sb);

	return (err);
}

static int
ionic_vlan_sysctl(SYSCTL_HANDLER_ARGS)
{
	struct ionic_lif *lif;
	struct sbuf *sb;
	struct ionic_mc_addr *mc;
	struct rx_filter *f;
	int i, err;

	lif = oidp->oid_arg1;
	err = sysctl_wire_old_buffer(req, 0);
	if (err)
		return (err);

	sb = sbuf_new_for_sysctl(NULL, NULL, 4096, req);
	if (sb == NULL)
		return (ENOMEM);

	for (i = 0; i < MAX_VLAN_TAG; i++) {
		mc = &lif->mc_addrs[i];
		f = ionic_rx_filter_by_vlan(lif, i);
		if (f != NULL)
			sbuf_printf(sb, "\nVLAN[%d](%d)", i, f->filter_id);
	}

	err = sbuf_finish(sb);
	sbuf_delete(sb);

	return (err);
}

/*
 * Dump perQ interrupt status.
 */
static int
ionic_intr_sysctl(SYSCTL_HANDLER_ARGS)
{
	struct ionic_dev *idev;
	struct ionic_lif *lif;
	struct ionic_rxque *rxq;
	struct ionic_adminq *adminq;
	struct ionic_notifyq* notifyq;
	struct ionic_intr __iomem *intr;
	struct sbuf *sb;
	uint32_t mask_val, cred_val;
	int i, err;

	lif = oidp->oid_arg1;
	idev = &lif->ionic->idev;
	adminq = lif->adminq;
	notifyq = lif->notifyq;

        err = sysctl_wire_old_buffer(req, 0);
        if (err)
                return (err);

	sb = sbuf_new_for_sysctl(NULL, NULL, 4096, req);
        if (sb == NULL)
                return (ENOMEM);

	intr = &idev->intr_ctrl[adminq->intr.index];
	mask_val = ioread32(&intr->mask);
	cred_val = ioread32(&intr->credits);

	sbuf_printf(sb, "\n%s intr: %s credits: %d\n",
	    adminq->name, mask_val ? "masked" : "unmasked",
	    cred_val & IONIC_INTR_CRED_COUNT);

	intr = &idev->intr_ctrl[notifyq->intr.index];
	mask_val = ioread32(&intr->mask);
	cred_val = ioread32(&intr->credits);

	sbuf_printf(sb, "%s intr: %s credits: %d\n",
	    notifyq->name, mask_val ? "masked" : "unmasked",
	    cred_val & IONIC_INTR_CRED_COUNT);

	for (i = 0; i < lif->ntxqs; i++) {
		rxq = lif->rxqs[i];
		intr = &idev->intr_ctrl[rxq->intr.index];
		mask_val = ioread32(&intr->mask);
		cred_val = ioread32(&intr->credits);

		sbuf_printf(sb, "%s intr: %s credits: %d\n",
		    rxq->name, mask_val ? "masked" : "unmasked",
		    cred_val & IONIC_INTR_CRED_COUNT);
	}

        err = sbuf_finish(sb);
        sbuf_delete(sb);

        return (err);
}

/*
 * Allow user to set flow control:
 * 0 - No flow control
 * 1 - Link level
 * 2 - Priority Flow Control
 */
static int
ionic_flow_ctrl_sysctl(SYSCTL_HANDLER_ARGS)
{
	struct ionic_lif *lif = oidp->oid_arg1;
	struct ionic *ionic = lif->ionic;
	struct ionic_dev *idev = &ionic->idev;
	uint8_t pause_type = idev->port_info->config.pause_type & 0xf0;
	int err, fc;

	/* Not allowed to change for mgmt interface. */
	if (ionic->is_mgmt_nic)
		return (EINVAL);

	err = sysctl_handle_int(oidp, &fc, 0, req);
	if ((err) || (req->newptr == NULL))
		return (err);

	switch (fc) {
	case 0:
		pause_type |= PORT_PAUSE_TYPE_NONE;
		break;
	case 1:
		pause_type |= PORT_PAUSE_TYPE_LINK;
		break;
	case 2:
		pause_type |= PORT_PAUSE_TYPE_PFC;
		break;
	default:
		if_printf(lif->netdev,
		    "Out of range flow control: %d value\n", fc);
		return (EIO);
	}

	if (idev->port_info->config.pause_type != pause_type) {
		IONIC_DEV_LOCK(ionic);
		ionic_dev_cmd_port_pause(idev, pause_type);
		err = ionic_dev_cmd_wait_check(idev, ionic_devcmd_timeout * HZ);
		IONIC_DEV_UNLOCK(ionic);
		if (err) {
			IONIC_NETDEV_ERROR(lif->netdev,
			    "failed to set pause, error = %d\n", err);
			return (err);
		}
		idev->port_info->config.pause_type = pause_type;
	}

	return (0);
}

static int
ionic_link_pause_sysctl(SYSCTL_HANDLER_ARGS)
{
	struct ionic_lif *lif = oidp->oid_arg1;
	struct ionic *ionic = lif->ionic;
	struct ionic_dev *idev = &ionic->idev;
	struct ifnet *ifp = lif->netdev;
	uint8_t pause_type;
	int err, type;

	/* Not allowed to change for mgmt interface. */
	if (ionic->is_mgmt_nic)
		return (EINVAL);

	err = sysctl_handle_int(oidp, &type, 0, req);
	if ((err) || (req->newptr == NULL))
		return (err);

	pause_type = idev->port_info->config.pause_type & IONIC_PAUSE_TYPE_MASK;
	if (pause_type != PORT_PAUSE_TYPE_LINK) {
		if_printf(ifp, "first set pause type to link\n");
		return (ENXIO);
	}
	if (type < 1 || type > 3) {
		if_printf(ifp, "value out of range: 1(rx), 2(tx) or 3(both)\n");
		return (ENXIO);
	}

	if (type & 1)
		pause_type |= IONIC_PAUSE_F_RX;
	if (type & 2)
		pause_type |= IONIC_PAUSE_F_TX;

	IONIC_DEV_LOCK(ionic);
	ionic_dev_cmd_port_pause(idev, pause_type);
	err = ionic_dev_cmd_wait_check(idev, ionic_devcmd_timeout * HZ);
	IONIC_DEV_UNLOCK(ionic);
	if (err)
		IONIC_NETDEV_ERROR(ifp,
		    "failed to set pause type, error = %d\n", err);

	return (err);
}

static int
ionic_rdma_sniffer_sysctl(SYSCTL_HANDLER_ARGS)
{
	struct ionic_lif *lif = oidp->oid_arg1;
	unsigned int rx_mode = lif->rx_mode;
	int value, err;

	rx_mode = lif->rx_mode;
	value = (rx_mode & RX_MODE_F_RDMA_SNIFFER) ? 1 : 0;

	err = sysctl_handle_int(oidp, &value, 0, req);
	if (err || (req->newptr == NULL))
		return (err);

	if (value)
		rx_mode |= RX_MODE_F_RDMA_SNIFFER;
	else
		rx_mode &= ~RX_MODE_F_RDMA_SNIFFER;
	
	ionic_lif_rx_mode(lif, rx_mode);

	return (err);
}

static int
ionic_reset_stats_sysctl(SYSCTL_HANDLER_ARGS)
{
	struct ionic_lif *lif = oidp->oid_arg1;
	struct ionic_adminq *adminq = lif->adminq;
	struct ionic *ionic = lif->ionic;
	struct ionic_dev *idev = &ionic->idev;
	struct ifnet *ifp = lif->netdev;
	struct ionic_rxque *rxq;
	struct ionic_txque *txq;
	struct ionic_rx_stats *rxstat;
	struct ionic_tx_stats *txstat;
	int i, err, value;

	err = sysctl_handle_int(oidp, &value, 0, req);
	if ((err) || (req->newptr == NULL))
		return (err);

	if (value == 0) {
		if_printf(ifp, "use non-zero value to reset\n");
		return (EINVAL);
	}

	IONIC_LIF_LOCK(lif);
	/* Reset the driver stats. */
	bzero(&adminq->stats, sizeof(adminq->stats));
	for (i = 0; i < lif->nrxqs; i++) {
		rxq = lif->rxqs[i];
		rxstat = &rxq->stats;
		bzero(rxstat, sizeof(*rxstat));
		txq = lif->txqs[i];
		txstat = &txq->stats;
		bzero(txstat, sizeof(*txstat));
	}

	/* Reset firmware stats. */
	err = ionic_lif_reset_stats(lif);
	IONIC_LIF_UNLOCK(lif);
	if (err) {
		if (err == IONIC_RC_ENOSUPP)
			IONIC_NETDEV_ERROR(ifp,
			    "lif stats reset not supported, error: %d\n", err);
		else
			IONIC_NETDEV_ERROR(ifp,
			    "lif stats reset failed, error:  %d\n", err);
	}

	/* Reset port stats. */
	IONIC_DEV_LOCK(ionic);
	ionic_dev_cmd_port_reset_stats(idev);
	err = ionic_dev_cmd_wait_check(idev, ionic_devcmd_timeout * HZ);
	IONIC_DEV_UNLOCK(ionic);
	if (err) {
		if (err == IONIC_RC_ENOSUPP)
			IONIC_NETDEV_ERROR(ifp,
			    "port stats reset not supported, error: %d\n", err);
		else
			IONIC_NETDEV_ERROR(ifp,
			    "port stats reset failed, error: %d\n", err);
	}

	return (0);
}
/*
 * Print various media details.
 */
static int
ionic_media_status_sysctl(SYSCTL_HANDLER_ARGS)
{
	struct ionic_lif *lif;
	struct lif_status *lif_status;
	struct port_status *port_status;
	union port_config *port_config;
	struct xcvr_status *xcvr;
	struct sbuf *sb;
	struct ionic *ionic;
	struct ionic_dev *idev;
	int err;

	lif = oidp->oid_arg1;
	ionic = lif->ionic;
	idev = &ionic->idev;
	lif_status = &lif->info->status;
	port_status = &idev->port_info->status;
	port_config = &idev->port_info->config;
	xcvr = &port_status->xcvr;

	err = sysctl_wire_old_buffer(req, 0);
	if (err)
		return (err);

	sb = sbuf_new_for_sysctl(NULL, NULL, 4096, req);
	if (sb == NULL)
		return (ENOMEM);

	sbuf_printf(sb, "\n");

	sbuf_printf(sb, " lif_status eid=%ld status=%s"
	    " speed=%dMbps lif_down_count=%d\n",
	    lif_status->eid,
	    ionic_port_oper_status_str(lif_status->link_status),
	    lif_status->link_speed, lif_status->link_down_count);

	sbuf_printf(sb, "  port_status id=%d status=%s"
	    " speed=%dMbps link_down_count=%d\n",
	    port_status->id,
	    ionic_port_oper_status_str(port_status->status),
	    port_status->speed, port_status->link_down_count);

	sbuf_printf(sb, "  port_config state=%s speed=%dMbps mtu=%d AN %s"
	    " fec_type=%s pause_type=%s loopback_mode=%s\n",
	    ionic_port_admin_state_str(port_config->state),
	    port_config->speed, port_config->mtu,
	    port_config->an_enable ? "enabled" : "disabled",
	    ionic_port_fec_type_str(port_config->fec_type),
	    ionic_port_pause_type_str(port_config->pause_type & IONIC_PAUSE_TYPE_MASK),
	    ionic_port_loopback_mode_str(port_config->loopback_mode));

	sbuf_printf(sb, "  xcvr_status state=%s phy=%s pid=0x%x\n",
	    ionic_xcvr_state_str(xcvr->state), ionic_phy_type_str(xcvr->phy),
	    xcvr->pid);

	switch (xcvr->pid) {
	case XCVR_PID_QSFP_100G_CR4:
	case XCVR_PID_QSFP_40GBASE_CR4:
	case XCVR_PID_QSFP_100G_AOC:
	case XCVR_PID_QSFP_100G_ACC:
	case XCVR_PID_QSFP_100G_SR4:
	case XCVR_PID_QSFP_100G_LR4:
	case XCVR_PID_QSFP_100G_ER4:
	case XCVR_PID_QSFP_100G_CWDM4:
	case XCVR_PID_QSFP_100G_PSM4:
	case XCVR_PID_QSFP_40GBASE_ER4:
	case XCVR_PID_QSFP_40GBASE_SR4:
	case XCVR_PID_QSFP_40GBASE_LR4:
	case XCVR_PID_QSFP_40GBASE_AOC:
		if (xcvr->sprom[SFF_8436_ID] == 0) {
			/* Older firmware sends page1 info in page0 space */
			sbuf_printf(sb,
			    "    QSFP Vendor: %-.*s P/N: %-.*s S/N: %-.*s\n",
			    16, &xcvr->sprom[SFF_8472_VENDOR_START],
			    16, &xcvr->sprom[SFF_8472_PN_START],
			    16, &xcvr->sprom[SFF_8472_SN_START]);
		} else {
			sbuf_printf(sb,
			    "    QSFP Vendor: %-.*s P/N: %-.*s S/N: %-.*s\n",
			    16, &xcvr->sprom[SFF_8436_VENDOR_START],
			    16, &xcvr->sprom[SFF_8436_PN_START],
			    16, &xcvr->sprom[SFF_8436_SN_START]);
		}
		break;

	case XCVR_PID_SFP_25GBASE_CR_S:
	case XCVR_PID_SFP_25GBASE_CR_L:
	case XCVR_PID_SFP_25GBASE_CR_N:
	case XCVR_PID_SFP_25GBASE_SR:
	case XCVR_PID_SFP_25GBASE_LR:
	case XCVR_PID_SFP_25GBASE_ER:
	case XCVR_PID_SFP_25GBASE_AOC:
	case XCVR_PID_SFP_25GBASE_ACC:
	case XCVR_PID_SFP_10GBASE_SR:
	case XCVR_PID_SFP_10GBASE_LR:
	case XCVR_PID_SFP_10GBASE_LRM:
	case XCVR_PID_SFP_10GBASE_ER:
	case XCVR_PID_SFP_10GBASE_AOC:
	case XCVR_PID_SFP_10GBASE_CU:
		sbuf_printf(sb, "    SFP Vendor: %-.*s P/N: %-.*s S/N: %-.*s\n",
		    16, &xcvr->sprom[SFF_8472_VENDOR_START],
		    16, &xcvr->sprom[SFF_8472_PN_START],
		    16, &xcvr->sprom[SFF_8472_SN_START]);
		break;

	default:
		sbuf_printf(sb, "    unknown media\n");
		break;
	}

	err = sbuf_finish(sb);
	sbuf_delete(sb);

	return (err);
}

static int
ionic_lif_reset_sysctl(SYSCTL_HANDLER_ARGS)
{
	struct ionic_lif *lif;
	int error, reset;

	lif = oidp->oid_arg1;

	error = sysctl_handle_int(oidp, &reset, 0, req);
	if ((error) || (req->newptr == NULL))
		return (error);

	if (reset == 0)
		return (EINVAL);

	return (ionic_lif_reinit(lif, false));
}

/*
 * Sysctl to control Firmware heartbeat interval timer.
 */
static int
ionic_fw_hb_handler(SYSCTL_HANDLER_ARGS)
{
	struct ionic_lif *lif = oidp->oid_arg1;
	struct ionic_dev *idev = &lif->ionic->idev;
	uint32_t old_hb_msecs, new_hb_msecs;
	int error;

	old_hb_msecs = (uint32_t)(idev->fw_hb_interval * 1000 / HZ);

	error = SYSCTL_OUT(req, &old_hb_msecs, sizeof(old_hb_msecs));
	if (error || !req->newptr)
		return (error);

	error = SYSCTL_IN(req, &new_hb_msecs, sizeof(new_hb_msecs));
	if (error)
		return (error);

	if (idev->fw_hb_state == IONIC_FW_HB_UNSUPPORTED) {
		IONIC_DEV_WARN(lif->ionic->dev,
		    "fw heartbeat not supported on this build\n");
		return (0);
	}

	if (new_hb_msecs > 0 && new_hb_msecs < IONIC_WDOG_MIN_MS)
		return (EINVAL);
	if (new_hb_msecs == old_hb_msecs)
		return (0);

	if (new_hb_msecs > 0 &&
	    new_hb_msecs < IONIC_WDOG_FW_WARN_MS) {
		IONIC_DEV_WARN(lif->ionic->dev,
		    "setting fw_hb_interval below %ums will "
		    "cause spurious timeouts\n",
		    IONIC_WDOG_FW_WARN_MS);
	}

	/* Update stored value and restart the workqueue */
	idev->fw_hb_interval = (unsigned long)new_hb_msecs * HZ / 1000;
	idev->fw_hb_state = new_hb_msecs ?
		IONIC_FW_HB_INIT : IONIC_FW_HB_DISABLED;
	ionic_fw_hb_resched(idev);

	return (0);
}

/*
 * Sysctl to control Command heartbeat interval timer.
 */
static int
ionic_cmd_hb_handler(SYSCTL_HANDLER_ARGS)
{
	struct ionic_lif *lif = oidp->oid_arg1;
	struct ionic_dev *idev = &lif->ionic->idev;
	uint32_t old_hb_msecs, new_hb_msecs;
	int error;

	old_hb_msecs = (uint32_t)(idev->cmd_hb_interval * 1000 / HZ);

	error = SYSCTL_OUT(req, &old_hb_msecs, sizeof(old_hb_msecs));
	if (error || !req->newptr)
		return (error);

	error = SYSCTL_IN(req, &new_hb_msecs, sizeof(new_hb_msecs));
	if (error)
		return (error);
	if (new_hb_msecs > 0 && new_hb_msecs < IONIC_WDOG_MIN_MS)
		return (EINVAL);
	if (new_hb_msecs == old_hb_msecs)
		return (0);

	/* Update stored value and restart the workqueue */
	idev->cmd_hb_interval = (unsigned long)new_hb_msecs * HZ / 1000;
	ionic_cmd_hb_resched(idev);

	return (0);
}

/*
 * Sysctl to control TxQ watchdog timeouts.
 */
static int
ionic_txq_wdog_handler(SYSCTL_HANDLER_ARGS)
{
	struct ionic_lif *lif = oidp->oid_arg1;
	uint32_t old_to_msecs, new_to_msecs;
	int error;

	old_to_msecs = (uint32_t)(lif->txq_wdog_timeout * 1000 / HZ);

	error = SYSCTL_OUT(req, &old_to_msecs, sizeof(old_to_msecs));
	if (error || !req->newptr)
		return (error);

	error = SYSCTL_IN(req, &new_to_msecs, sizeof(new_to_msecs));
	if (error)
		return (error);
	if (new_to_msecs > 0 && new_to_msecs < IONIC_WDOG_MIN_MS)
		return (EINVAL);
	if (new_to_msecs == old_to_msecs)
		return (0);

	/* Update stored value and restart the workqueue */
	lif->txq_wdog_timeout = (unsigned long)new_to_msecs * HZ / 1000;
	ionic_txq_wdog_resched(lif);

	return (0);
}

static void
ionic_lif_add_rxtstat(struct ionic_rxque *rxq, struct sysctl_ctx_list *ctx,
    struct sysctl_oid_list *queue_list)
{
	struct lro_ctrl *lro = &rxq->lro;
	struct ionic_rx_stats *rxstat = &rxq->stats;

	SYSCTL_ADD_UINT(ctx, queue_list, OID_AUTO, "head", CTLFLAG_RD,
			&rxq->head_index, 0, "Head index");
	SYSCTL_ADD_UINT(ctx, queue_list, OID_AUTO, "tail", CTLFLAG_RD,
			&rxq->tail_index, 0, "Tail index");
	SYSCTL_ADD_UINT(ctx, queue_list, OID_AUTO, "comp_index", CTLFLAG_RD,
			&rxq->comp_index, 0, "Completion index");
	SYSCTL_ADD_UINT(ctx, queue_list, OID_AUTO, "num_descs", CTLFLAG_RD,
			&rxq->num_descs, 0, "Number of descriptors");
	SYSCTL_ADD_UINT(ctx, queue_list, OID_AUTO, "irq", CTLFLAG_RD,
			&rxq->intr.vector, 0, "Interrupt vector");

	SYSCTL_ADD_ULONG(ctx, queue_list, OID_AUTO, "dma_setup_error", CTLFLAG_RD,
			 &rxstat->dma_map_err, "DMA map setup error");
	SYSCTL_ADD_ULONG(ctx, queue_list, OID_AUTO, "alloc_error", CTLFLAG_RD,
			 &rxstat->alloc_err, "Buffer allocation error");
	SYSCTL_ADD_ULONG(ctx, queue_list, OID_AUTO, "comp_err", CTLFLAG_RD,
			 &rxstat->comp_err, "Completion with error");
	SYSCTL_ADD_ULONG(ctx, queue_list, OID_AUTO, "length_err", CTLFLAG_RD,
			 &rxstat->length_err, "Too short or too long packets");

	SYSCTL_ADD_ULONG(ctx, queue_list, OID_AUTO, "pkts", CTLFLAG_RD,
			 &rxstat->pkts, "Rx Packets");
	SYSCTL_ADD_ULONG(ctx, queue_list, OID_AUTO, "bytes", CTLFLAG_RD,
			 &rxstat->bytes, "Rx bytes");
	SYSCTL_ADD_ULONG(ctx, queue_list, OID_AUTO, "csum_ip_ok", CTLFLAG_RD,
			 &rxstat->csum_ip_ok, "IP checksum OK");
	SYSCTL_ADD_ULONG(ctx, queue_list, OID_AUTO, "csum_ip_bad", CTLFLAG_RD,
			 &rxstat->csum_ip_bad, "IP checksum bad");
	SYSCTL_ADD_ULONG(ctx, queue_list, OID_AUTO, "csum_l4_ok", CTLFLAG_RD,
			 &rxstat->csum_l4_ok, "L4 checksum OK");
	SYSCTL_ADD_ULONG(ctx, queue_list, OID_AUTO, "csum_l4_bad", CTLFLAG_RD,
			 &rxstat->csum_l4_bad, "L4 checksum bad");
	SYSCTL_ADD_ULONG(ctx, queue_list, OID_AUTO, "isr_count", CTLFLAG_RD,
			 &rxstat->isr_count, "ISR count");
	SYSCTL_ADD_ULONG(ctx, queue_list, OID_AUTO, "clean_count", CTLFLAG_RD,
			 &rxstat->task, "Rx clean count");
	SYSCTL_ADD_ULONG(ctx, queue_list, OID_AUTO, "mbuf_alloc", CTLFLAG_RD,
			&rxstat->mbuf_alloc, "Number of mbufs allocated");
	SYSCTL_ADD_ULONG(ctx, queue_list, OID_AUTO, "mbuf_free", CTLFLAG_RD,
			&rxstat->mbuf_free, "Number of mbufs free");

	/* LRO related. */
	SYSCTL_ADD_ULONG(ctx, queue_list, OID_AUTO, "lro_queued", CTLFLAG_RD,
			 &lro->lro_queued, "LRO packets queued");
	SYSCTL_ADD_ULONG(ctx, queue_list, OID_AUTO, "lro_flushed", CTLFLAG_RD,
			 &lro->lro_flushed, "LRO packets flushed");
	SYSCTL_ADD_ULONG(ctx, queue_list, OID_AUTO, "lro_bad_csum", CTLFLAG_RD,
			 &lro->lro_bad_csum, "LRO" );

	/* RSS related. */
	SYSCTL_ADD_ULONG(ctx, queue_list, OID_AUTO, "rss_ip4", CTLFLAG_RD,
			 &rxstat->rss_ip4, "RSS IPv4 packets");
	SYSCTL_ADD_ULONG(ctx, queue_list, OID_AUTO, "rss_tcp_ip4", CTLFLAG_RD,
			 &rxstat->rss_tcp_ip4, "RSS TCP/IPv4 packets");
	SYSCTL_ADD_ULONG(ctx, queue_list, OID_AUTO, "rss_udp_ip4", CTLFLAG_RD,
			 &rxstat->rss_udp_ip4, "RSS UDP/IPv4 packets");

	SYSCTL_ADD_ULONG(ctx, queue_list, OID_AUTO, "rss_ip6", CTLFLAG_RD,
			 &rxstat->rss_ip6, "RSS IPv6 packets");
	SYSCTL_ADD_ULONG(ctx, queue_list, OID_AUTO, "rss_tcp_ip6", CTLFLAG_RD,
			 &rxstat->rss_tcp_ip6, "RSS TCP/IPv6 packets");
	SYSCTL_ADD_ULONG(ctx, queue_list, OID_AUTO, "rss_udp_ip6", CTLFLAG_RD,
			 &rxstat->rss_udp_ip6, "RSS UDP/IPv6 packets");

	SYSCTL_ADD_ULONG(ctx, queue_list, OID_AUTO, "rss_unknown", CTLFLAG_RD,
			 &rxstat->rss_unknown, "RSS for unknown packets");
}

static void
ionic_lif_add_txtstat(struct ionic_txque *txq, struct sysctl_ctx_list *ctx,
    struct sysctl_oid_list *list)
{
	struct ionic_tx_stats *txstat = &txq->stats;

	SYSCTL_ADD_UINT(ctx, list, OID_AUTO, "head", CTLFLAG_RD,
			&txq->head_index, 0, "Head index");
	SYSCTL_ADD_UINT(ctx, list, OID_AUTO, "tail", CTLFLAG_RD,
			&txq->tail_index, 0, "Tail index");
	SYSCTL_ADD_UINT(ctx, list, OID_AUTO, "comp_index", CTLFLAG_RD,
			&txq->comp_index, 0, "Completion index");
	SYSCTL_ADD_UINT(ctx, list, OID_AUTO, "num_descs", CTLFLAG_RD,
			&txq->num_descs, 0, "Number of descriptors");

	SYSCTL_ADD_ULONG(ctx, list, OID_AUTO, "dma_map_error", CTLFLAG_RD,
			 &txstat->dma_map_err, "DMA mapping error");
	SYSCTL_ADD_ULONG(ctx, list, OID_AUTO, "comp_err", CTLFLAG_RD,
			 &txstat->comp_err, "Completion with error");
	SYSCTL_ADD_ULONG(ctx, list, OID_AUTO, "tx_clean", CTLFLAG_RD,
			 &txstat->clean, "Tx clean");
	SYSCTL_ADD_ULONG(ctx, list, OID_AUTO, "tx_requeued", CTLFLAG_RD,
			 &txstat->re_queue, "Packets were requeued");
	SYSCTL_ADD_ULONG(ctx, list, OID_AUTO, "no_descs", CTLFLAG_RD,
			 &txstat->no_descs, "Descriptors not available");
	SYSCTL_ADD_ULONG(ctx, list, OID_AUTO, "mbuf_defrag", CTLFLAG_RD,
			 &txstat->mbuf_defrag, "Linearize  mbuf");
	SYSCTL_ADD_ULONG(ctx, list, OID_AUTO, "mbuf_defrag_err", CTLFLAG_RD,
			 &txstat->mbuf_defrag_err, "Linearize  mbuf failed");
	SYSCTL_ADD_ULONG(ctx, list, OID_AUTO, "bad_ethtype", CTLFLAG_RD,
			 &txstat->bad_ethtype, "Tx malformed Ethernet");
	SYSCTL_ADD_ULONG(ctx, list, OID_AUTO, "wdog_expired", CTLFLAG_RD,
			 &txstat->wdog_expired, "Tx watchdog expired");

	SYSCTL_ADD_ULONG(ctx, list, OID_AUTO, "pkts", CTLFLAG_RD,
			 &txstat->pkts, "Tx Packets");
	SYSCTL_ADD_ULONG(ctx, list, OID_AUTO, "bytes", CTLFLAG_RD,
			 &txstat->bytes, "Tx Bytes");
	SYSCTL_ADD_ULONG(ctx, list, OID_AUTO, "csum_offload", CTLFLAG_RD,
			 &txstat->csum_offload, "Tx h/w checksum");
	SYSCTL_ADD_ULONG(ctx, list, OID_AUTO, "no_csum_offload", CTLFLAG_RD,
			 &txstat->no_csum_offload, "Tx checksum");
	SYSCTL_ADD_ULONG(ctx, list, OID_AUTO, "tso_ipv4", CTLFLAG_RD,
			 &txstat->tso_ipv4, "TSO for IPv4");
	SYSCTL_ADD_ULONG(ctx, list, OID_AUTO, "tso_ipv6", CTLFLAG_RD,
			 &txstat->tso_ipv6, "TSO for IPv6");
	SYSCTL_ADD_ULONG(ctx, list, OID_AUTO, "tso_max_size", CTLFLAG_RD,
			 &txstat->tso_max_size, "TSO maximum packet size");
	SYSCTL_ADD_ULONG(ctx, list, OID_AUTO, "tso_max_sg", CTLFLAG_RD,
			 &txstat->tso_max_sg, "TSO maximum number of sg");
	SYSCTL_ADD_ULONG(ctx, list, OID_AUTO, "tso_max_descs", CTLFLAG_RD,
			 &txstat->tso_max_descs, "TSO max number of descs");
	SYSCTL_ADD_ULONG(ctx, list, OID_AUTO, "tso_too_big", CTLFLAG_RD,
			 &txstat->tso_too_big, "TSO too big for ring");
	SYSCTL_ADD_ULONG(ctx, list, OID_AUTO, "tso_no_descs", CTLFLAG_RD,
			 &txstat->tso_no_descs, "TSO descriptors not avail");
	SYSCTL_ADD_ULONG(ctx, list, OID_AUTO, "tso_sparse", CTLFLAG_RD,
			 &txstat->tso_sparse, "TSO is sparse");
	SYSCTL_ADD_ULONG(ctx, list, OID_AUTO, "tso_defrag_sparse", CTLFLAG_RD,
			 &txstat->tso_defrag_sparse, "TSO is sparse after defragging");

	SYSCTL_ADD_ULONG(ctx, list, OID_AUTO, "pkt_sparse", CTLFLAG_RD,
			 &txstat->pkt_sparse, "Packet is sparse");
	SYSCTL_ADD_ULONG(ctx, list, OID_AUTO, "pkt_defrag_sparse", CTLFLAG_RD,
			 &txstat->pkt_defrag_sparse, "Packet is sparse after defragging");
}

/*
 * Stats provided by firmware.
 */
static void
ionic_setup_fw_stats(struct ionic_lif *lif, struct sysctl_ctx_list *ctx,
	struct sysctl_oid_list *child)
{
	struct lif_stats *stat = &lif->info->stats;
	struct sysctl_oid *queue_node;
	struct sysctl_oid_list *queue_list;
	char namebuf[QUEUE_NAME_LEN];

	snprintf(namebuf, QUEUE_NAME_LEN, "fw");
	queue_node = SYSCTL_ADD_NODE(ctx, child, OID_AUTO, namebuf,
				CTLFLAG_RD, NULL, "Firmware generated counters");
	queue_list = SYSCTL_CHILDREN(queue_node);
	SYSCTL_ADD_ULONG(ctx, queue_list, OID_AUTO, "rx_ucast_bytes", CTLFLAG_RD,
			&stat->rx_ucast_bytes, "");
	SYSCTL_ADD_ULONG(ctx, queue_list, OID_AUTO, "rx_ucast_packets", CTLFLAG_RD,
			&stat->rx_ucast_packets, "");
	SYSCTL_ADD_ULONG(ctx, queue_list, OID_AUTO, "rx_mcast_bytes", CTLFLAG_RD,
			&stat->rx_mcast_bytes, "");
	SYSCTL_ADD_ULONG(ctx, queue_list, OID_AUTO, "rx_mcast_packets", CTLFLAG_RD,
			&stat->rx_mcast_packets, "");
	SYSCTL_ADD_ULONG(ctx, queue_list, OID_AUTO, "rx_bcast_bytes", CTLFLAG_RD,
			&stat->rx_bcast_bytes, "");
	SYSCTL_ADD_ULONG(ctx, queue_list, OID_AUTO, "rx_bcast_packets", CTLFLAG_RD,
			&stat->rx_bcast_packets, "");

	SYSCTL_ADD_ULONG(ctx, queue_list, OID_AUTO, "rx_ucast_drop_bytes", CTLFLAG_RD,
			&stat->rx_ucast_drop_bytes, "");
	SYSCTL_ADD_ULONG(ctx, queue_list, OID_AUTO, "rx_ucast_drop_packets", CTLFLAG_RD,
			&stat->rx_ucast_drop_packets, "");
	SYSCTL_ADD_ULONG(ctx, queue_list, OID_AUTO, "rx_mcast_drop_bytes", CTLFLAG_RD,
			&stat->rx_mcast_drop_bytes, "");
	SYSCTL_ADD_ULONG(ctx, queue_list, OID_AUTO, "rx_mcast_drop_packets", CTLFLAG_RD,
			&stat->rx_mcast_drop_packets, "");
	SYSCTL_ADD_ULONG(ctx, queue_list, OID_AUTO, "rx_bcast_drop_bytes", CTLFLAG_RD,
			&stat->rx_bcast_drop_bytes, "");
	SYSCTL_ADD_ULONG(ctx, queue_list, OID_AUTO, "rx_bcast_drop_packets", CTLFLAG_RD,
			&stat->rx_bcast_drop_packets, "");
	SYSCTL_ADD_ULONG(ctx, queue_list, OID_AUTO, "rx_dma_error", CTLFLAG_RD,
			&stat->rx_dma_error, "");

	SYSCTL_ADD_ULONG(ctx, queue_list, OID_AUTO, "tx_ucast_bytes", CTLFLAG_RD,
			&stat->tx_ucast_bytes, "");
	SYSCTL_ADD_ULONG(ctx, queue_list, OID_AUTO, "tx_ucast_packets", CTLFLAG_RD,
			&stat->tx_ucast_packets, "");
	SYSCTL_ADD_ULONG(ctx, queue_list, OID_AUTO, "tx_mcast_bytes", CTLFLAG_RD,
			&stat->tx_mcast_bytes, "");
	SYSCTL_ADD_ULONG(ctx, queue_list, OID_AUTO, "tx_mcast_packets", CTLFLAG_RD,
			&stat->tx_mcast_packets, "");
	SYSCTL_ADD_ULONG(ctx, queue_list, OID_AUTO, "tx_bcast_bytes", CTLFLAG_RD,
			&stat->tx_bcast_bytes, "");
	SYSCTL_ADD_ULONG(ctx, queue_list, OID_AUTO, "tx_bcast_packets", CTLFLAG_RD,
			&stat->tx_bcast_packets, "");

	SYSCTL_ADD_ULONG(ctx, queue_list, OID_AUTO, "tx_ucast_drop_bytes", CTLFLAG_RD,
			&stat->tx_ucast_drop_bytes, "");
	SYSCTL_ADD_ULONG(ctx, queue_list, OID_AUTO, "tx_ucast_drop_packets", CTLFLAG_RD,
			&stat->tx_ucast_drop_packets, "");
	SYSCTL_ADD_ULONG(ctx, queue_list, OID_AUTO, "tx_mcast_drop_bytes", CTLFLAG_RD,
			&stat->tx_mcast_drop_bytes, "");
	SYSCTL_ADD_ULONG(ctx, queue_list, OID_AUTO, "tx_mcast_drop_packets", CTLFLAG_RD,
			&stat->tx_mcast_drop_packets, "");
	SYSCTL_ADD_ULONG(ctx, queue_list, OID_AUTO, "tx_bcast_drop_bytes", CTLFLAG_RD,
			&stat->tx_bcast_drop_bytes, "");
	SYSCTL_ADD_ULONG(ctx, queue_list, OID_AUTO, "tx_bcast_drop_packets", CTLFLAG_RD,
			&stat->tx_bcast_drop_packets, "");
	SYSCTL_ADD_ULONG(ctx, queue_list, OID_AUTO, "tx_dma_error", CTLFLAG_RD,
			&stat->tx_dma_error, "");

	SYSCTL_ADD_ULONG(ctx, queue_list, OID_AUTO, "rx_queue_disabled", CTLFLAG_RD,
			&stat->rx_queue_disabled, "");
	SYSCTL_ADD_ULONG(ctx, queue_list, OID_AUTO, "rx_queue_empty", CTLFLAG_RD,
			&stat->rx_queue_empty, "");
	SYSCTL_ADD_ULONG(ctx, queue_list, OID_AUTO, "rx_queue_error", CTLFLAG_RD,
			&stat->rx_queue_error, "");
	SYSCTL_ADD_ULONG(ctx, queue_list, OID_AUTO, "rx_desc_fetch_error", CTLFLAG_RD,
			&stat->rx_desc_fetch_error, "");
	SYSCTL_ADD_ULONG(ctx, queue_list, OID_AUTO, "rx_desc_data_error", CTLFLAG_RD,
			&stat->rx_desc_data_error, "");

	SYSCTL_ADD_ULONG(ctx, queue_list, OID_AUTO, "tx_queue_disabled", CTLFLAG_RD,
			&stat->tx_queue_disabled, "");
	SYSCTL_ADD_ULONG(ctx, queue_list, OID_AUTO, "tx_queue_error", CTLFLAG_RD,
			&stat->tx_queue_error, "");
	SYSCTL_ADD_ULONG(ctx, queue_list, OID_AUTO, "tx_desc_fetch_error", CTLFLAG_RD,
			&stat->tx_desc_fetch_error, "");
	SYSCTL_ADD_ULONG(ctx, queue_list, OID_AUTO, "tx_desc_data_error", CTLFLAG_RD,
			&stat->tx_desc_data_error, "");

	/* H/w stats for RoCE devices. */
	SYSCTL_ADD_ULONG(ctx, queue_list, OID_AUTO, "tx_rdma_ucast_bytes", CTLFLAG_RD,
			&stat->tx_rdma_ucast_bytes, "");
	SYSCTL_ADD_ULONG(ctx, queue_list, OID_AUTO, "tx_rdma_ucast_packets", CTLFLAG_RD,
			&stat->tx_rdma_ucast_packets, "");
	SYSCTL_ADD_ULONG(ctx, queue_list, OID_AUTO, "tx_rdma_mcast_bytes", CTLFLAG_RD,
			&stat->tx_rdma_mcast_bytes, "");
	SYSCTL_ADD_ULONG(ctx, queue_list, OID_AUTO, "tx_rdma_mcast_packets", CTLFLAG_RD,
			&stat->tx_rdma_mcast_packets, "");
	SYSCTL_ADD_ULONG(ctx, queue_list, OID_AUTO, "tx_rdma_cnp_packets", CTLFLAG_RD,
			&stat->tx_rdma_cnp_packets, "");

	SYSCTL_ADD_ULONG(ctx, queue_list, OID_AUTO, "rx_rdma_ucast_bytes", CTLFLAG_RD,
			&stat->rx_rdma_ucast_bytes, "");
	SYSCTL_ADD_ULONG(ctx, queue_list, OID_AUTO, "rx_rdma_ucast_packets", CTLFLAG_RD,
			&stat->rx_rdma_ucast_packets, "");
	SYSCTL_ADD_ULONG(ctx, queue_list, OID_AUTO, "rx_rdma_mcast_bytes", CTLFLAG_RD,
			&stat->rx_rdma_mcast_bytes, "");
	SYSCTL_ADD_ULONG(ctx, queue_list, OID_AUTO, "rx_rdma_mcast_packets", CTLFLAG_RD,
			&stat->rx_rdma_mcast_packets, "");
	SYSCTL_ADD_ULONG(ctx, queue_list, OID_AUTO, "rx_rdma_cnp_packets", CTLFLAG_RD,
			&stat->rx_rdma_cnp_packets, "");
	SYSCTL_ADD_ULONG(ctx, queue_list, OID_AUTO, "rx_rdma_ecn_packets", CTLFLAG_RD,
			&stat->rx_rdma_ecn_packets, "");
}

/*
 * MAC statistics.
 */
static void
ionic_setup_mac_stats(struct ionic_lif *lif, struct sysctl_ctx_list *ctx,
    struct sysctl_oid_list *child)
{
	struct port_stats *stats;
	struct sysctl_oid *queue_node;
	struct sysctl_oid_list *queue_list;
	char namebuf[QUEUE_NAME_LEN];

	snprintf(namebuf, QUEUE_NAME_LEN, "mac");
	queue_node = SYSCTL_ADD_NODE(ctx, child, OID_AUTO, namebuf,
				CTLFLAG_RD, NULL, "MAC provided counters");
	queue_list = SYSCTL_CHILDREN(queue_node);
	stats = &lif->ionic->idev.port_info->stats;

	SYSCTL_ADD_ULONG(ctx, queue_list, OID_AUTO, "frames_rx_ok", CTLFLAG_RD,
			&stats->frames_rx_ok, "");
	SYSCTL_ADD_ULONG(ctx, queue_list, OID_AUTO, "frames_rx_all", CTLFLAG_RD,
			&stats->frames_rx_all, "");
	SYSCTL_ADD_ULONG(ctx, queue_list, OID_AUTO, "frames_rx_bad_fcs", CTLFLAG_RD,
			&stats->frames_rx_bad_fcs, "");
	SYSCTL_ADD_ULONG(ctx, queue_list, OID_AUTO, "frames_rx_bad_all", CTLFLAG_RD,
			&stats->frames_rx_bad_all, "");

	SYSCTL_ADD_ULONG(ctx, queue_list, OID_AUTO, "octets_rx_ok", CTLFLAG_RD,
			&stats->octets_rx_ok, "");
	SYSCTL_ADD_ULONG(ctx, queue_list, OID_AUTO, "octets_rx_all", CTLFLAG_RD,
			&stats->octets_rx_all, "");

	SYSCTL_ADD_ULONG(ctx, queue_list, OID_AUTO, "frames_rx_unicast", CTLFLAG_RD,
			&stats->frames_rx_unicast, "");
	SYSCTL_ADD_ULONG(ctx, queue_list, OID_AUTO, "frames_rx_multicast", CTLFLAG_RD,
			&stats->frames_rx_multicast, "");
	SYSCTL_ADD_ULONG(ctx, queue_list, OID_AUTO, "frames_rx_broadcast", CTLFLAG_RD,
			&stats->frames_rx_broadcast, "");
	SYSCTL_ADD_ULONG(ctx, queue_list, OID_AUTO, "frames_rx_pause", CTLFLAG_RD,
			&stats->frames_rx_pause, "");

	SYSCTL_ADD_ULONG(ctx, queue_list, OID_AUTO, "frames_rx_bad_length", CTLFLAG_RD,
			&stats->frames_rx_bad_length, "");
	SYSCTL_ADD_ULONG(ctx, queue_list, OID_AUTO, "frames_rx_undersized", CTLFLAG_RD,
			&stats->frames_rx_undersized, "");
	SYSCTL_ADD_ULONG(ctx, queue_list, OID_AUTO, "frames_rx_oversized", CTLFLAG_RD,
			&stats->frames_rx_oversized, "");
	SYSCTL_ADD_ULONG(ctx, queue_list, OID_AUTO, "frames_rx_fragments", CTLFLAG_RD,
			&stats->frames_rx_fragments, "");
	SYSCTL_ADD_ULONG(ctx, queue_list, OID_AUTO, "frames_rx_jabber", CTLFLAG_RD,
			&stats->frames_rx_jabber, "");
	SYSCTL_ADD_ULONG(ctx, queue_list, OID_AUTO, "frames_rx_pripause", CTLFLAG_RD,
			&stats->frames_rx_pripause, "");
	SYSCTL_ADD_ULONG(ctx, queue_list, OID_AUTO, "frames_rx_stomped_crc", CTLFLAG_RD,
			&stats->frames_rx_stomped_crc, "");
	SYSCTL_ADD_ULONG(ctx, queue_list, OID_AUTO, "frames_rx_too_long", CTLFLAG_RD,
			&stats->frames_rx_too_long, "");

	SYSCTL_ADD_ULONG(ctx, queue_list, OID_AUTO, "frames_rx_vlan_good", CTLFLAG_RD,
			&stats->frames_rx_vlan_good, "");
	SYSCTL_ADD_ULONG(ctx, queue_list, OID_AUTO, "frames_rx_dropped", CTLFLAG_RD,
			&stats->frames_rx_dropped, "");
	SYSCTL_ADD_ULONG(ctx, queue_list, OID_AUTO, "frames_rx_less_than_64b", CTLFLAG_RD,
			&stats->frames_rx_less_than_64b, "");
	SYSCTL_ADD_ULONG(ctx, queue_list, OID_AUTO, "frames_rx_64b", CTLFLAG_RD,
			&stats->frames_rx_64b, "");
	SYSCTL_ADD_ULONG(ctx, queue_list, OID_AUTO, "frames_rx_65b_127b", CTLFLAG_RD,
			&stats->frames_rx_65b_127b, "");
	SYSCTL_ADD_ULONG(ctx, queue_list, OID_AUTO, "frames_rx_128b_255b", CTLFLAG_RD,
			&stats->frames_rx_128b_255b, "");
	SYSCTL_ADD_ULONG(ctx, queue_list, OID_AUTO, "frames_rx_256b_511b", CTLFLAG_RD,
			&stats->frames_rx_256b_511b, "");
	SYSCTL_ADD_ULONG(ctx, queue_list, OID_AUTO, "frames_rx_512b_1023b", CTLFLAG_RD,
			&stats->frames_rx_512b_1023b, "");
	SYSCTL_ADD_ULONG(ctx, queue_list, OID_AUTO, "frames_rx_1024b_1518b", CTLFLAG_RD,
			&stats->frames_rx_1024b_1518b, "");
	SYSCTL_ADD_ULONG(ctx, queue_list, OID_AUTO, "frames_rx_1519b_2047b", CTLFLAG_RD,
			&stats->frames_rx_1519b_2047b, "");
	SYSCTL_ADD_ULONG(ctx, queue_list, OID_AUTO, "frames_rx_2048b_4095b", CTLFLAG_RD,
			&stats->frames_rx_2048b_4095b, "");
	SYSCTL_ADD_ULONG(ctx, queue_list, OID_AUTO, "frames_rx_4096b_8191b", CTLFLAG_RD,
			&stats->frames_rx_4096b_8191b, "");
	SYSCTL_ADD_ULONG(ctx, queue_list, OID_AUTO, "frames_rx_8192b_9215b", CTLFLAG_RD,
			&stats->frames_rx_8192b_9215b, "");
	SYSCTL_ADD_ULONG(ctx, queue_list, OID_AUTO, "frames_rx_other", CTLFLAG_RD,
			&stats->frames_rx_other, "");
	SYSCTL_ADD_ULONG(ctx, queue_list, OID_AUTO, "rx_pause_1us_count", CTLFLAG_RD,
			&stats->rx_pause_1us_count, "");
	/* Transmit stats. */
	SYSCTL_ADD_ULONG(ctx, queue_list, OID_AUTO, "frames_tx_ok", CTLFLAG_RD,
			&stats->frames_tx_ok, "");
	SYSCTL_ADD_ULONG(ctx, queue_list, OID_AUTO, "frames_tx_all", CTLFLAG_RD,
			&stats->frames_tx_all, "");
	SYSCTL_ADD_ULONG(ctx, queue_list, OID_AUTO, "frames_tx_bad", CTLFLAG_RD,
			&stats->frames_tx_bad, "");

	SYSCTL_ADD_ULONG(ctx, queue_list, OID_AUTO, "octets_tx_ok", CTLFLAG_RD,
			&stats->octets_tx_ok, "");
	SYSCTL_ADD_ULONG(ctx, queue_list, OID_AUTO, "octets_tx_total", CTLFLAG_RD,
			&stats->octets_tx_total, "");

	SYSCTL_ADD_ULONG(ctx, queue_list, OID_AUTO, "frames_tx_unicast", CTLFLAG_RD,
			&stats->frames_tx_unicast, "");
	SYSCTL_ADD_ULONG(ctx, queue_list, OID_AUTO, "frames_tx_multicast", CTLFLAG_RD,
			&stats->frames_tx_multicast, "");
	SYSCTL_ADD_ULONG(ctx, queue_list, OID_AUTO, "frames_tx_broadcast", CTLFLAG_RD,
			&stats->frames_tx_broadcast, "");
	SYSCTL_ADD_ULONG(ctx, queue_list, OID_AUTO, "frames_tx_pause", CTLFLAG_RD,
			&stats->frames_tx_pause, "");

	SYSCTL_ADD_ULONG(ctx, queue_list, OID_AUTO, "frames_tx_pripause", CTLFLAG_RD,
			&stats->frames_tx_pripause, "");
	SYSCTL_ADD_ULONG(ctx, queue_list, OID_AUTO, "frames_tx_vlan", CTLFLAG_RD,
			&stats->frames_tx_vlan, "");

	SYSCTL_ADD_ULONG(ctx, queue_list, OID_AUTO, "frames_tx_less_than_64b", CTLFLAG_RD,
			&stats->frames_tx_less_than_64b, "");
	SYSCTL_ADD_ULONG(ctx, queue_list, OID_AUTO, "frames_tx_64b", CTLFLAG_RD,
			&stats->frames_tx_64b, "");
	SYSCTL_ADD_ULONG(ctx, queue_list, OID_AUTO, "frames_tx_65b_127b", CTLFLAG_RD,
			&stats->frames_tx_65b_127b, "");
	SYSCTL_ADD_ULONG(ctx, queue_list, OID_AUTO, "frames_tx_128b_255b", CTLFLAG_RD,
			&stats->frames_tx_128b_255b, "");
	SYSCTL_ADD_ULONG(ctx, queue_list, OID_AUTO, "frames_tx_256b_511b", CTLFLAG_RD,
			&stats->frames_tx_256b_511b, "");
	SYSCTL_ADD_ULONG(ctx, queue_list, OID_AUTO, "frames_tx_512b_1023b", CTLFLAG_RD,
			&stats->frames_tx_512b_1023b, "");
	SYSCTL_ADD_ULONG(ctx, queue_list, OID_AUTO, "frames_tx_1024b_1518b", CTLFLAG_RD,
			&stats->frames_tx_1024b_1518b, "");
	SYSCTL_ADD_ULONG(ctx, queue_list, OID_AUTO, "frames_tx_1519b_2047b", CTLFLAG_RD,
			&stats->frames_tx_1519b_2047b, "");
	SYSCTL_ADD_ULONG(ctx, queue_list, OID_AUTO, "frames_tx_2048b_4095b", CTLFLAG_RD,
			&stats->frames_tx_2048b_4095b, "");
	SYSCTL_ADD_ULONG(ctx, queue_list, OID_AUTO, "frames_tx_4096b_8191b", CTLFLAG_RD,
			&stats->frames_tx_4096b_8191b, "");
	SYSCTL_ADD_ULONG(ctx, queue_list, OID_AUTO, "frames_tx_8192b_9215b", CTLFLAG_RD,
			&stats->frames_tx_8192b_9215b, "");

	SYSCTL_ADD_ULONG(ctx, queue_list, OID_AUTO, "frames_tx_other", CTLFLAG_RD,
			&stats->frames_tx_other, "");
	SYSCTL_ADD_ULONG(ctx, queue_list, OID_AUTO, "frames_tx_truncated", CTLFLAG_RD,
			&stats->frames_tx_truncated, "");

	SYSCTL_ADD_ULONG(ctx, queue_list, OID_AUTO, "frames_rx_pri_0", CTLFLAG_RD,
			&stats->frames_rx_pri_0, "");
	SYSCTL_ADD_ULONG(ctx, queue_list, OID_AUTO, "frames_tx_pri_0", CTLFLAG_RD,
			&stats->frames_tx_pri_0, "");
	SYSCTL_ADD_ULONG(ctx, queue_list, OID_AUTO, "frames_rx_pri_1", CTLFLAG_RD,
			&stats->frames_rx_pri_1, "");
	SYSCTL_ADD_ULONG(ctx, queue_list, OID_AUTO, "frames_tx_pri_1", CTLFLAG_RD,
			&stats->frames_tx_pri_1, "");
	SYSCTL_ADD_ULONG(ctx, queue_list, OID_AUTO, "frames_rx_pri_2", CTLFLAG_RD,
			&stats->frames_rx_pri_2, "");
	SYSCTL_ADD_ULONG(ctx, queue_list, OID_AUTO, "frames_tx_pri_2", CTLFLAG_RD,
			&stats->frames_tx_pri_2, "");
	SYSCTL_ADD_ULONG(ctx, queue_list, OID_AUTO, "frames_rx_pri_3", CTLFLAG_RD,
			&stats->frames_rx_pri_3, "");
	SYSCTL_ADD_ULONG(ctx, queue_list, OID_AUTO, "frames_tx_pri_3", CTLFLAG_RD,
			&stats->frames_tx_pri_3, "");
	SYSCTL_ADD_ULONG(ctx, queue_list, OID_AUTO, "frames_rx_pri_4", CTLFLAG_RD,
			&stats->frames_rx_pri_4, "");
	SYSCTL_ADD_ULONG(ctx, queue_list, OID_AUTO, "frames_tx_pri_4", CTLFLAG_RD,
			&stats->frames_tx_pri_4, "");
	SYSCTL_ADD_ULONG(ctx, queue_list, OID_AUTO, "frames_rx_pri_5", CTLFLAG_RD,
			&stats->frames_rx_pri_5, "");
	SYSCTL_ADD_ULONG(ctx, queue_list, OID_AUTO, "frames_tx_pri_5", CTLFLAG_RD,
			&stats->frames_tx_pri_5, "");
	SYSCTL_ADD_ULONG(ctx, queue_list, OID_AUTO, "frames_rx_pri_6", CTLFLAG_RD,
			&stats->frames_rx_pri_6, "");
	SYSCTL_ADD_ULONG(ctx, queue_list, OID_AUTO, "frames_tx_pri_6", CTLFLAG_RD,
			&stats->frames_tx_pri_6, "");
	SYSCTL_ADD_ULONG(ctx, queue_list, OID_AUTO, "frames_rx_pri_7", CTLFLAG_RD,
			&stats->frames_rx_pri_7, "");
	SYSCTL_ADD_ULONG(ctx, queue_list, OID_AUTO, "frames_tx_pri_7", CTLFLAG_RD,
			&stats->frames_tx_pri_7, "");

	SYSCTL_ADD_ULONG(ctx, queue_list, OID_AUTO, "rx_pripause_0_1us_count", CTLFLAG_RD,
			&stats->rx_pripause_0_1us_count, "");
	SYSCTL_ADD_ULONG(ctx, queue_list, OID_AUTO, "tx_pripause_0_1us_count", CTLFLAG_RD,
			&stats->tx_pripause_0_1us_count, "");
	SYSCTL_ADD_ULONG(ctx, queue_list, OID_AUTO, "rx_pripause_1_1us_count", CTLFLAG_RD,
			&stats->rx_pripause_1_1us_count, "");
	SYSCTL_ADD_ULONG(ctx, queue_list, OID_AUTO, "tx_pripause_1_1us_count", CTLFLAG_RD,
			&stats->tx_pripause_1_1us_count, "");
	SYSCTL_ADD_ULONG(ctx, queue_list, OID_AUTO, "rx_pripause_2_1us_count", CTLFLAG_RD,
			&stats->rx_pripause_2_1us_count, "");
	SYSCTL_ADD_ULONG(ctx, queue_list, OID_AUTO, "tx_pripause_2_1us_count", CTLFLAG_RD,
			&stats->tx_pripause_2_1us_count, "");
	SYSCTL_ADD_ULONG(ctx, queue_list, OID_AUTO, "rx_pripause_3_1us_count", CTLFLAG_RD,
			&stats->rx_pripause_3_1us_count, "");
	SYSCTL_ADD_ULONG(ctx, queue_list, OID_AUTO, "tx_pripause_3_1us_count", CTLFLAG_RD,
			&stats->tx_pripause_3_1us_count, "");
	SYSCTL_ADD_ULONG(ctx, queue_list, OID_AUTO, "rx_pripause_4_1us_count", CTLFLAG_RD,
			&stats->rx_pripause_4_1us_count, "");
	SYSCTL_ADD_ULONG(ctx, queue_list, OID_AUTO, "tx_pripause_4_1us_count", CTLFLAG_RD,
			&stats->tx_pripause_4_1us_count, "");
	SYSCTL_ADD_ULONG(ctx, queue_list, OID_AUTO, "rx_pripause_5_1us_count", CTLFLAG_RD,
			&stats->rx_pripause_5_1us_count, "");
	SYSCTL_ADD_ULONG(ctx, queue_list, OID_AUTO, "tx_pripause_5_1us_count", CTLFLAG_RD,
			&stats->tx_pripause_5_1us_count, "");
	SYSCTL_ADD_ULONG(ctx, queue_list, OID_AUTO, "rx_pripause_6_1us_count", CTLFLAG_RD,
			&stats->rx_pripause_6_1us_count, "");
	SYSCTL_ADD_ULONG(ctx, queue_list, OID_AUTO, "tx_pripause_6_1us_count", CTLFLAG_RD,
			&stats->tx_pripause_6_1us_count, "");
	SYSCTL_ADD_ULONG(ctx, queue_list, OID_AUTO, "rx_pripause_7_1us_count", CTLFLAG_RD,
			&stats->rx_pripause_7_1us_count, "");
	SYSCTL_ADD_ULONG(ctx, queue_list, OID_AUTO, "tx_pripause_7_1us_count", CTLFLAG_RD,
			&stats->tx_pripause_7_1us_count, "");
}

static void
ionic_notifyq_sysctl(struct ionic_lif *lif, struct sysctl_ctx_list *ctx,
    struct sysctl_oid_list *child)
{
	struct ionic_notifyq* notifyq = lif->notifyq;
	struct sysctl_oid *queue_node;
	struct sysctl_oid_list *queue_list;
	char namebuf[QUEUE_NAME_LEN];

	if (notifyq == NULL)
		return;

	snprintf(namebuf, QUEUE_NAME_LEN, "nq");
	queue_node = SYSCTL_ADD_NODE(ctx, child, OID_AUTO, namebuf,
	    CTLFLAG_RD, NULL, "Queue Name");
	queue_list = SYSCTL_CHILDREN(queue_node);

	SYSCTL_ADD_UINT(ctx, queue_list, OID_AUTO, "num_descs", CTLFLAG_RD,
			&notifyq->num_descs, 0, "Number of descriptors");
	SYSCTL_ADD_UINT(ctx, queue_list, OID_AUTO, "comp_index", CTLFLAG_RD,
			&notifyq->comp_index, 0, "Completion index");
	SYSCTL_ADD_UINT(ctx, queue_list, OID_AUTO, "irq", CTLFLAG_RD,
			&notifyq->intr.vector, 0, "Interrupt vector");
	SYSCTL_ADD_ULONG(ctx, queue_list, OID_AUTO, "last_eid", CTLFLAG_RD,
			&lif->last_eid, "Last event Id");
	SYSCTL_ADD_ULONG(ctx, queue_list, OID_AUTO, "isr_count", CTLFLAG_RD,
			&notifyq->isr_count, "ISR count");
	SYSCTL_ADD_ULONG(ctx, queue_list, OID_AUTO, "comp_count", CTLFLAG_RD,
			&notifyq->comp_count, "NQ completions processed");
}

static void
ionic_qos_config_sysctl(struct ionic_qos_tc *tc, int num, struct sysctl_ctx_list *ctx,
    struct sysctl_oid_list *queue_list)
{
	if (num == 0) {
		SYSCTL_ADD_BOOL(ctx, queue_list, OID_AUTO, "enable", CTLFLAG_RD,
				&tc->enable, 0, "Enable policy");
		SYSCTL_ADD_BOOL(ctx, queue_list, OID_AUTO, "drop", CTLFLAG_RD,
				&tc->drop, 0, "Enable drop");
	} else {
		SYSCTL_ADD_BOOL(ctx, queue_list, OID_AUTO, "enable", CTLFLAG_RW,
				&tc->enable, 0, "Enable policy");
		SYSCTL_ADD_BOOL(ctx, queue_list, OID_AUTO, "drop", CTLFLAG_RW,
				&tc->drop, 0, "Enable drop");
	}

	SYSCTL_ADD_UINT(ctx, queue_list, OID_AUTO, "mtu", CTLFLAG_RW,
			&tc->mtu, 1500, "MTU");
	SYSCTL_ADD_U8(ctx, queue_list, OID_AUTO, "pcp", CTLFLAG_RW,
			&tc->dot1q_pcp, 0, "802.1q TCI pcp value");
	SYSCTL_ADD_U8(ctx, queue_list, OID_AUTO, "weight", CTLFLAG_RW,
			&tc->dwrr_weight, 0, "DWRR weight");
	SYSCTL_ADD_U8(ctx, queue_list, OID_AUTO, "pfc_cos", CTLFLAG_RW,
			&tc->pfc_cos, 0, "pfc cos value");
}

static int
ionic_qos_validate(struct ionic_lif *lif, int qnum)
{	
	struct ionic_qos_tc *swtc;
	struct ifnet *ifp = lif->netdev;
	swtc = &lif->ionic->qos_tc[qnum];
	if (swtc->dot1q_pcp >= IONIC_QOS_CLASS_MAX) {
		IONIC_NETDEV_ERROR(ifp, "tc%d pcp value: %d invalid\n",
		    qnum, swtc->dot1q_pcp);
			return (EINVAL);
	}

	if (swtc->dwrr_weight > 100) {
		IONIC_NETDEV_ERROR(ifp, "tc%d weight value: %d invalid\n",
		    qnum, swtc->dwrr_weight);
		return (EINVAL);
	}

	if (swtc->mtu < IONIC_MIN_MTU || swtc->mtu > IONIC_MAX_MTU) {
		IONIC_NETDEV_ERROR(lif->netdev, "tc%d mtu value: %d invalid\n",
		    qnum, swtc->mtu);
		return (EINVAL);
	}

	if (swtc->pfc_cos >= IONIC_QOS_CLASS_MAX) {
		IONIC_NETDEV_ERROR(ifp, "tc%d pfc_cos value: %d invalid\n",
		    qnum, swtc->pfc_cos);
			return (EINVAL);
	}

	return (0);
}

/*
 * Send the QoS config to the card.
 */
static int
ionic_qos_apply_sysctl(SYSCTL_HANDLER_ARGS)
{
	struct ionic_lif *lif = oidp->oid_arg1;
	struct ionic *ionic = lif->ionic;
	struct identity *ident = &ionic->ident;
	union qos_config *qos;
	struct ionic_qos_tc *swtc;
	int i, err, val, total_wt;

	/* Not allowed to change for mgmt interface. */
	if (ionic->is_mgmt_nic)
		return (EINVAL);

	err = sysctl_handle_int(oidp, &val, 0, req);
	if ((err) || (req->newptr == NULL))
		return (err);

	total_wt = 0;
	for (i = 1; i < IONIC_QOS_CLASS_MAX; i++) {
		swtc = &ionic->qos_tc[i];
		if (swtc->enable)
			total_wt += swtc->dwrr_weight;
	}

	if (total_wt > 100) {
		IONIC_NETDEV_ERROR(lif->netdev,
		    "total weight %d > 100\n", total_wt);
		return (EINVAL);
	}

	ionic_qos_class_identify(ionic);

	for (i = 1; i < IONIC_QOS_CLASS_MAX; i++) {
		qos = &ident->qos.config[i];
		swtc = &ionic->qos_tc[i];

		/* Validate user provided parameters. */
		if (ionic_qos_validate(lif, i))
			continue;

		/* Delete the class if it already exists */
		if (qos->flags & IONIC_QOS_CONFIG_F_ENABLE)
			ionic_qos_class_reset(ionic, i);

		/* No need to init the class if the user does not want to enable it */
		if (!swtc->enable) {
			IONIC_NETDEV_DEBUG(lif->netdev,
			    "Class %d not enabled. Skipped!\n", i);
			continue;
		}

		qos->flags |= IONIC_QOS_CONFIG_F_ENABLE;

		if (swtc->drop)
			qos->flags |= IONIC_QOS_CONFIG_F_DROP;
		else
			qos->flags &= ~IONIC_QOS_CONFIG_F_DROP;

		qos->mtu = swtc->mtu;
		qos->dot1q_pcp = swtc->dot1q_pcp;
		qos->dwrr_weight = swtc->dwrr_weight;

		if ((ionic->idev.port_info->config.pause_type & IONIC_PAUSE_TYPE_MASK) != PORT_PAUSE_TYPE_PFC)
			qos->pause_type = PORT_PAUSE_TYPE_LINK;  // default
		else
			qos->pause_type = (ionic->idev.port_info->config.pause_type & IONIC_PAUSE_TYPE_MASK);

		qos->class_type = QOS_CLASS_TYPE_PCP;
		qos->sched_type = QOS_SCHED_TYPE_DWRR;

		qos->pfc_cos = swtc->pfc_cos;

		IONIC_NETDEV_DEBUG(lif->netdev,
		    "TC: %d pcp: %d weight: %d drop: %s class: %d pause_type:%d pfc_cos: %d "
		    "flags: %#x\n",
		    i, qos->dot1q_pcp, qos->dwrr_weight,
		    swtc->drop ? "true" : "false", qos->class_type, qos->pause_type,
		    qos->pfc_cos, qos->flags);

		err = ionic_qos_class_init(ionic, i, qos);
		if (err) {
			IONIC_NETDEV_ERROR(lif->netdev,
			    "failed to create class %d\n", i);
			ionic_qos_class_identify(ionic);
			return (err);
		}
	}

	return (0);
}

static void
ionic_qos_sysctl(struct ionic_lif *lif, struct sysctl_ctx_list *ctx,
    struct sysctl_oid_list *child)
{
	struct sysctl_oid *queue_node, *queue_node1;
	struct sysctl_oid_list *queue_list, *queue_list1;
	char namebuf[QUEUE_NAME_LEN];
	int i;

	snprintf(namebuf, QUEUE_NAME_LEN, "qos");
	queue_node = SYSCTL_ADD_NODE(ctx, child, OID_AUTO, namebuf,
	    CTLFLAG_RD, NULL, "QoS for controlling CoS/PFC etc, shared between ports");
	queue_list = SYSCTL_CHILDREN(queue_node);

	SYSCTL_ADD_PROC(ctx, queue_list, OID_AUTO, "apply",
	    CTLTYPE_INT | CTLFLAG_RW | CTLFLAG_SKIP, lif, 0,
	    ionic_qos_apply_sysctl, "I",
	    "Apply the newly configured QoS params to hardware");

	for (i = 0; i < IONIC_QOS_CLASS_MAX; i++) {
		snprintf(namebuf, QUEUE_NAME_LEN, "tc%d", i);
		queue_node1 = SYSCTL_ADD_NODE(ctx, queue_list, OID_AUTO,
		    namebuf, CTLFLAG_RD, NULL, "QoS traffic class");
		queue_list1 = SYSCTL_CHILDREN(queue_node1);
		ionic_qos_config_sysctl(&lif->ionic->qos_tc[i], i, ctx, queue_list1);
	}
}

/*
 * Sysctl to control AdminQ heartbeat interval timer.
 */
static int
ionic_adminq_hb_handler(SYSCTL_HANDLER_ARGS)
{
	struct ionic_lif *lif = oidp->oid_arg1;
	uint32_t old_hb_msecs, new_hb_msecs;
	int error;

	old_hb_msecs = (uint32_t)(lif->adq_hb_interval * 1000 / HZ);

	error = SYSCTL_OUT(req, &old_hb_msecs, sizeof(old_hb_msecs));
	if (error || !req->newptr)
		return (error);

	error = SYSCTL_IN(req, &new_hb_msecs, sizeof(new_hb_msecs));
	if (error)
		return (error);
	if (new_hb_msecs > 0 && new_hb_msecs < IONIC_WDOG_MIN_MS)
		return (EINVAL);
	if (new_hb_msecs == old_hb_msecs)
		return (0);

	/* Update stored value and restart the workqueue */
	lif->adq_hb_interval = (unsigned long)new_hb_msecs * HZ / 1000;
	ionic_adminq_hb_resched(lif);

	return (0);
}

static void
ionic_adminq_sysctl(struct ionic_lif *lif, struct sysctl_ctx_list *ctx,
		struct sysctl_oid_list *child)
{
	struct ionic_adminq* adminq = lif->adminq;
	struct sysctl_oid *queue_node;
	struct sysctl_oid_list *queue_list;
	char namebuf[QUEUE_NAME_LEN];

	if(adminq == NULL)
		return;

	snprintf(namebuf, QUEUE_NAME_LEN, "adq");
	queue_node = SYSCTL_ADD_NODE(ctx, child, OID_AUTO, namebuf,
				CTLFLAG_RD, NULL, "Queue Name");
	queue_list = SYSCTL_CHILDREN(queue_node);

	SYSCTL_ADD_UINT(ctx, queue_list, OID_AUTO, "num_descs", CTLFLAG_RD,
			&adminq->num_descs, 0, "Number of descriptors");
	SYSCTL_ADD_UINT(ctx, queue_list, OID_AUTO, "irq", CTLFLAG_RD,
			&adminq->intr.vector, 0, "Interrupt vector");
	SYSCTL_ADD_UINT(ctx, queue_list, OID_AUTO, "head", CTLFLAG_RD,
			&adminq->head_index, 0, "Head index");
	SYSCTL_ADD_UINT(ctx, queue_list, OID_AUTO, "tail", CTLFLAG_RD,
			&adminq->tail_index, 0, "Tail index");
	SYSCTL_ADD_UINT(ctx, queue_list, OID_AUTO, "comp_index", CTLFLAG_RD,
			&adminq->comp_index, 0, "Completion index");
	SYSCTL_ADD_ULONG(ctx, queue_list, OID_AUTO, "comp_err", CTLFLAG_RD,
			&adminq->stats.comp_err, "Completions with error");
	SYSCTL_ADD_PROC(ctx, queue_list, OID_AUTO, "hb_interval",
			CTLTYPE_UINT | CTLFLAG_RW, lif, 0,
			ionic_adminq_hb_handler, "IU",
			"AdminQ heartbeat interval in msecs");
}

/*
 * Firmware update
 */
static int
ionic_firmware_update2(struct ionic_lif *lif)
{
	const struct firmware *fw;
	struct ifnet *ifp = lif->netdev;
	struct ionic_dev *idev = &lif->ionic->idev;
	unsigned long time;
	int err;

	if (strcmp(ionic_fw_update_ver, "") == 0) {
		if_printf(ifp, "Firmware version number not provided\n");
		return (0);
	}

	if (strncmp(ionic_fw_update_ver, idev->dev_info.fw_version,
	    sizeof(ionic_fw_update_ver)) == 0) {
		if_printf(ifp,
		    "No need to update firmware, its already on %s\n",
		    ionic_fw_update_ver);
		return (0);
	}

	time = jiffies;
	if ((fw = firmware_get("ionic_fw")) == NULL) {
		IONIC_NETDEV_ERROR(ifp, "Could not find firmware image\n");
		return (ENOENT);
	} else {
		if_printf(ifp, "Updating firmware %s ->%s\n",
		    idev->dev_info.fw_version, ionic_fw_update_ver);
	}

	err = ionic_firmware_update(lif, fw->data, fw->datasize);
	if (err) {
		IONIC_NETDEV_ERROR(ifp,
		    "Firmware update version: %s failed\n",
		    ionic_fw_update_ver);
	}

	if_printf(ifp, "firmware update took %ld secs\n", (jiffies - time)/HZ);
	firmware_put(fw, FIRMWARE_UNLOAD);

	return (0);
}

static int
ionic_firmware_update_sysctl(SYSCTL_HANDLER_ARGS)
{
	struct ionic_lif *lif;
	int error, reset;

	lif = oidp->oid_arg1;
	error = sysctl_handle_int(oidp, &reset, 0, req);
	if ((error) || (req->newptr == NULL))
		return (error);

	return (ionic_firmware_update2(lif));
}

static void
ionic_setup_device_stats(struct ionic_lif *lif)
{
	struct sysctl_ctx_list *ctx = &lif->sysctl_ctx;
	struct sysctl_oid *tree = lif->sysctl_ifnet;
	struct sysctl_oid_list *child = SYSCTL_CHILDREN(tree);
	struct ionic_dev *idev = &lif->ionic->idev;
	struct sysctl_oid *queue_node;
	struct sysctl_oid_list *queue_list;
	char namebuf[QUEUE_NAME_LEN];
	int i;

	SYSCTL_ADD_STRING(ctx, child, OID_AUTO, "fw_version", CTLFLAG_RD,
			idev->dev_info.fw_version, sizeof(idev->dev_info.fw_version),
			"Running firmware version");
	SYSCTL_ADD_STRING(ctx, child, OID_AUTO, "serial_no", CTLFLAG_RD,
			idev->dev_info.serial_num, sizeof(idev->dev_info.serial_num),
			"Card serial number");
	SYSCTL_ADD_UINT(ctx, child, OID_AUTO, "numq", CTLFLAG_RD,
			&lif->ntxqs, 0, "Number of Tx/Rx queue pairs");
	SYSCTL_ADD_UINT(ctx, child, OID_AUTO, "hw_capabilities", CTLFLAG_RD,
			&lif->hw_features, 0, "Hardware features enabled like checksum, TSO etc");
	SYSCTL_ADD_ULONG(ctx, child, OID_AUTO, "dev_cmds", CTLFLAG_RD,
			&lif->num_dev_cmds, "Number of dev commands used");
	SYSCTL_ADD_ULONG(ctx, child, OID_AUTO, "lif_resets", CTLFLAG_RD,
			&lif->num_resets, "Number of resets attempted on this LIF");
	SYSCTL_ADD_ULONG(ctx, child, OID_AUTO, "legacy_spurious_isr", CTLFLAG_RD,
			&lif->spurious, "Legacy spurious interrupts");
	SYSCTL_ADD_UINT(ctx, child, OID_AUTO, "wdog_error_trigger", CTLFLAG_RW,
			&lif->wdog_error_trigger, 0, "Inject watchdog error");
	SYSCTL_ADD_UINT(ctx, child, OID_AUTO, "mac_filter_count", CTLFLAG_RD,
			&lif->num_mc_addrs, 0, "Number of MAC filters");
	SYSCTL_ADD_UINT(ctx, child, OID_AUTO, "rx_mbuf_sz", CTLFLAG_RD,
			&lif->rx_mbuf_size, 0, "Size of receive buffers");
	SYSCTL_ADD_UINT(ctx, child, OID_AUTO, "rx_mode", CTLFLAG_RD,
			&lif->rx_mode, 0, "Current receive mode");
	SYSCTL_ADD_UINT(ctx, child, OID_AUTO, "curr_coal_us", CTLFLAG_RD,
			&lif->intr_coalesce_us, 0, "Get current interrupt coalescing value(us)");
	SYSCTL_ADD_UINT(ctx, child, OID_AUTO, "coal_max_us", CTLFLAG_RD,
			&lif->intr_coalesce_max_us, 0, "Maximum interrupt coalescing value(us)");
	SYSCTL_ADD_PROC(ctx, child, OID_AUTO, "intr_coal", CTLTYPE_UINT | CTLFLAG_RW, lif, 0,
			ionic_intr_coal_handler, "IU", "Set interrupt coalescing value");
	SYSCTL_ADD_PROC(ctx, child, OID_AUTO, "filters",
			CTLTYPE_STRING | CTLFLAG_RD | CTLFLAG_SKIP, lif, 0,
			ionic_filter_sysctl, "A", "Print MAC filter list");
	SYSCTL_ADD_PROC(ctx, child, OID_AUTO, "vlans",
			CTLTYPE_STRING | CTLFLAG_RD | CTLFLAG_SKIP, lif, 0,
			ionic_vlan_sysctl, "A", "Print vlan filter list");
	SYSCTL_ADD_PROC(ctx, child, OID_AUTO, "media_status",
			CTLTYPE_STRING | CTLFLAG_RD | CTLFLAG_SKIP, lif, 0,
			ionic_media_status_sysctl, "A", "Miscellaneous media details");
	SYSCTL_ADD_PROC(ctx, child, OID_AUTO, "intr_status",
			CTLTYPE_STRING | CTLFLAG_RD | CTLFLAG_SKIP, lif, 0,
			ionic_intr_sysctl, "A", "Interrupt details");
	SYSCTL_ADD_PROC(ctx, child, OID_AUTO, "flow_ctrl",
			CTLTYPE_INT | CTLFLAG_RW | CTLFLAG_SKIP, lif, 0,
			ionic_flow_ctrl_sysctl, "I",
			"Set flow control - 0(off), 1(link), 2(pfc)");
	SYSCTL_ADD_PROC(ctx, child, OID_AUTO, "link_pause",
			CTLTYPE_INT | CTLFLAG_RW | CTLFLAG_SKIP, lif, 0,
			ionic_link_pause_sysctl, "I",
			"Set link pause - 1(rx), 2(tx) or 3(both)");
	SYSCTL_ADD_PROC(ctx, child, OID_AUTO, "cmd_hb_interval",
			CTLTYPE_UINT | CTLFLAG_RW, lif, 0,
			ionic_cmd_hb_handler, "IU",
			"Command heartbeat interval in msecs");

	/* Fw heartbeat. */
	SYSCTL_ADD_UINT(ctx, child, OID_AUTO, "fw_hb_counter", CTLFLAG_RD,
			&idev->fw_hb_last, 0, "Firmware heartbeat counter");
	SYSCTL_ADD_UINT(ctx, child, OID_AUTO, "fw_hb_state", CTLFLAG_RD,
			&idev->fw_hb_state, 0, "Firmware heartbeat state");
	SYSCTL_ADD_PROC(ctx, child, OID_AUTO, "fw_hb_interval",
			CTLTYPE_INT | CTLFLAG_RW, lif, 0,
			ionic_fw_hb_handler, "I",
			"Firmware heartbeat interval in msecs");

	SYSCTL_ADD_PROC(ctx, child, OID_AUTO, "txq_wdog_timeout",
			CTLTYPE_INT | CTLFLAG_RW, lif, 0,
			ionic_txq_wdog_handler, "I",
			"Tx queue watchdog timeout in msecs");
	SYSCTL_ADD_PROC(ctx, child, OID_AUTO, "reset_stats",
			CTLTYPE_INT | CTLFLAG_RW | CTLFLAG_SKIP, lif, 0,
			ionic_reset_stats_sysctl, "I",
			"Reset driver, firmware and port statistics");
	SYSCTL_ADD_PROC(ctx, child, OID_AUTO, "rdma_sniffer",
			CTLTYPE_INT | CTLFLAG_RW, lif, 0,
			ionic_rdma_sniffer_sysctl, "I",
			"Enable/Disable RDMA sniffer mode");
	SYSCTL_ADD_PROC(ctx, child, OID_AUTO, "reset",
			CTLTYPE_INT | CTLFLAG_RW | CTLFLAG_SKIP, lif, 0,
			ionic_lif_reset_sysctl, "I", "Reinit lif");
	SYSCTL_ADD_PROC(ctx, child, OID_AUTO, "fw_update",
			CTLTYPE_INT | CTLFLAG_RW | CTLFLAG_SKIP, lif, 0,
			ionic_firmware_update_sysctl, "I", "Firmware update");

	ionic_setup_fw_stats(lif, ctx, child);
	ionic_setup_mac_stats(lif, ctx, child);
	ionic_adminq_sysctl(lif, ctx, child);
	ionic_notifyq_sysctl(lif, ctx, child);
	ionic_qos_sysctl(lif, ctx, child);

	for (i = 0; i < lif->nrxqs; i++) {
		snprintf(namebuf, QUEUE_NAME_LEN, "rxq%d", i);
		queue_node = SYSCTL_ADD_NODE(ctx, child, OID_AUTO, namebuf,
					CTLFLAG_RD, NULL, "Queue Name");
		queue_list = SYSCTL_CHILDREN(queue_node);
		ionic_lif_add_rxtstat(lif->rxqs[i], ctx, queue_list);
	}

	for (i = 0; i < lif->ntxqs; i++) {
		snprintf(namebuf, QUEUE_NAME_LEN, "txq%d", i);
		queue_node = SYSCTL_ADD_NODE(ctx, child, OID_AUTO, namebuf,
					CTLFLAG_RD, NULL, "Queue Name");
		queue_list = SYSCTL_CHILDREN(queue_node);
		ionic_lif_add_txtstat(lif->txqs[i], ctx, queue_list);
	}
}

void
ionic_setup_sysctls(struct ionic_lif *lif)
{
	device_t dev = lif->ionic->dev;
	struct ifnet *ifp = lif->netdev;
	char buf[16];

	/* ifnet sysctl tree */
	sysctl_ctx_init(&lif->sysctl_ctx);
	lif->sysctl_ifnet = SYSCTL_ADD_NODE(&lif->sysctl_ctx, SYSCTL_STATIC_CHILDREN(_dev),
				OID_AUTO, ifp->if_dname, CTLFLAG_RD, 0, "Pensando I/O NIC");

	if (lif->sysctl_ifnet == NULL) {
		dev_err(dev, "SYSCTL_ADD_NODE() failed\n");

		return;
	}

	snprintf(buf, sizeof(buf), "%d", ifp->if_dunit);
	lif->sysctl_ifnet = SYSCTL_ADD_NODE(&lif->sysctl_ctx, SYSCTL_CHILDREN(lif->sysctl_ifnet),
					OID_AUTO, buf, CTLFLAG_RD, 0, "Pensando NIC unit");

	if (lif->sysctl_ifnet == NULL) {
		dev_err(dev, "SYSCTL_ADD_NODE() failed\n");
		sysctl_ctx_free(&lif->sysctl_ctx);

		return;
	}

	ionic_setup_device_stats(lif);
}


/*
 * Set netdev capabilities
 */
int
ionic_set_os_features(struct ifnet *ifp, uint32_t hw_features)
{
	if_setcapabilitiesbit(ifp, (IFCAP_VLAN_MTU | IFCAP_JUMBO_MTU |
		IFCAP_HWSTATS | IFCAP_LRO), 0);

	if (hw_features & ETH_HW_TX_CSUM)
		if_setcapabilitiesbit(ifp,
			(IFCAP_TXCSUM | IFCAP_TXCSUM_IPV6 | IFCAP_VLAN_HWCSUM), 0);

	if (hw_features & ETH_HW_RX_CSUM)
		if_setcapabilitiesbit(ifp,
			(IFCAP_RXCSUM | IFCAP_RXCSUM_IPV6 | IFCAP_VLAN_HWCSUM), 0);

	if (hw_features & ETH_HW_TSO)
		if_setcapabilitiesbit(ifp, IFCAP_TSO4, 0);

	if (hw_features & ETH_HW_TSO_IPV6)
		if_setcapabilitiesbit(ifp, IFCAP_TSO6, 0);

	if (hw_features & (ETH_HW_VLAN_TX_TAG | ETH_HW_VLAN_RX_STRIP))
		if_setcapabilitiesbit(ifp, IFCAP_VLAN_HWTAGGING, 0);

	if (hw_features & ETH_HW_VLAN_RX_FILTER)
		if_setcapabilitiesbit(ifp, IFCAP_VLAN_HWFILTER, 0);

	if (hw_features & (ETH_HW_TSO | ETH_HW_TSO_IPV6))
		if_setcapabilitiesbit(ifp, IFCAP_VLAN_HWTSO, 0);

	// Enable all capabilities
	if_setcapenable(ifp, if_getcapabilities(ifp));

	ifp->if_hwassist = 0;

	if ((if_getcapenable(ifp) & IFCAP_TSO))
		if_sethwassistbits(ifp, CSUM_TSO, 0);

	if ((if_getcapenable(ifp) & IFCAP_TXCSUM))
		if_sethwassistbits(ifp, (CSUM_IP | CSUM_TCP | CSUM_UDP), 0);

	if ((if_getcapenable(ifp) & IFCAP_TXCSUM_IPV6))
		if_sethwassistbits(ifp, (CSUM_TCP_IPV6 | CSUM_UDP_IPV6), 0);

	return (0);
}

void
ionic_lif_sysctl_free(struct ionic_lif *lif)
{
	if (lif->sysctl_ifnet) {
		sysctl_ctx_free(&lif->sysctl_ctx);
		lif->sysctl_ifnet = NULL;
	}
}

static void
ionic_iff_up(struct ionic_lif *lif)
{
#ifdef NETAPP_PATCH
	bool iff_up;
#endif

	/*
	 * These internally check if the respective value has changed, before
	 * issuing a command to the device.
	 */
	ionic_set_rx_mode(lif->netdev);
	ionic_set_mac(lif->netdev);

#ifdef NETAPP_PATCH
	/*
	 * Control the port admin state: allow or disallow link to come up.
	 *
	 * Only update the admin state when IFF_UP is observed to change.
	 * Initially, the port admin state may be UP on the nic, but not IFF_UP
	 * in the network stack.  When changing other flags (eg: promiscuous),
	 * do not to disturb the port admin state.
	 */
	iff_up = (lif->netdev->if_flags & IFF_UP) != 0;
	if (lif->iff_up == iff_up)
		return;

	lif->iff_up = iff_up;

	if (iff_up) {
		ionic_set_port_state(lif->ionic, PORT_ADMIN_STATE_UP);
	} else {
		/* Report link down until it is set by the next link event */
		lif->link_up = false;
		ionic_set_port_state(lif->ionic, PORT_ADMIN_STATE_DOWN);
	}
#endif
}

static int
ionic_ioctl(struct ifnet *ifp, u_long command, caddr_t data)
{
	struct ionic_lif *lif = if_getsoftc(ifp);
	struct ifreq *ifr = (struct ifreq *) data;
	int error = 0;
	int mask;
	uint32_t hw_features;

	switch (command) {
	case SIOCSIFMEDIA:
	case SIOCGIFMEDIA:
	case SIOCGIFXMEDIA:
		IONIC_NETDEV_INFO(ifp, "ioctl: SIOCxIFMEDIA (Get/Set Interface Media)\n");
		if (lif->registered) {
			error = ifmedia_ioctl(ifp, ifr, &lif->media, command);
		}
		break;

	case SIOCSIFCAP:
		mask = ifr->ifr_reqcap ^ if_getcapenable(ifp);

		IONIC_LIF_LOCK(lif);

		hw_features = lif->hw_features;

		if ((mask & IFCAP_LRO) &&
		    (if_getcapabilities(ifp) & IFCAP_LRO)) {
			if_togglecapenable(ifp, IFCAP_LRO);
		}

		if ((mask & IFCAP_TXCSUM) &&
		    (if_getcapabilities(ifp) & IFCAP_TXCSUM)) {
			if_togglecapenable(ifp, IFCAP_TXCSUM);
		}

		if ((mask & IFCAP_TXCSUM_IPV6) &&
		    (if_getcapabilities(ifp) & IFCAP_TXCSUM_IPV6)) {
			if_togglecapenable(ifp, IFCAP_TXCSUM_IPV6);
		}

		if ((mask & (IFCAP_TXCSUM | IFCAP_TXCSUM_IPV6)) &&
		    (if_getcapabilities(ifp) & (IFCAP_TXCSUM | IFCAP_TXCSUM_IPV6))) {
			hw_features ^= ETH_HW_TX_CSUM;
		}

		if ((if_getcapenable(ifp) & IFCAP_TXCSUM))
			if_sethwassistbits(ifp, (CSUM_IP | CSUM_TCP | CSUM_UDP), 0);
		else
			if_sethwassistbits(ifp, 0, (CSUM_IP | CSUM_TCP | CSUM_UDP));

		if ((if_getcapenable(ifp) & IFCAP_TXCSUM_IPV6))
			if_sethwassistbits(ifp, (CSUM_TCP_IPV6 | CSUM_UDP_IPV6), 0);
		else
			if_sethwassistbits(ifp, 0, (CSUM_TCP_IPV6 | CSUM_UDP_IPV6));

		if ((mask & IFCAP_RXCSUM) &&
		    (if_getcapabilities(ifp) & IFCAP_RXCSUM)) {
			if_togglecapenable(ifp, IFCAP_RXCSUM);
		}

		if ((mask & IFCAP_RXCSUM_IPV6) &&
		    (if_getcapabilities(ifp) & IFCAP_RXCSUM_IPV6)) {
			if_togglecapenable(ifp, IFCAP_RXCSUM_IPV6);
		}

		if ((mask & (IFCAP_RXCSUM | IFCAP_RXCSUM_IPV6)) &&
		    (if_getcapabilities(ifp) & (IFCAP_RXCSUM | IFCAP_RXCSUM_IPV6))) {
			hw_features ^= ETH_HW_RX_CSUM;
		}

		/* Checksum offload for vlan tagged packets */
		if ((mask & IFCAP_VLAN_HWCSUM) &&
		    (if_getcapabilities(ifp) & IFCAP_VLAN_HWCSUM)) {
			if_togglecapenable(ifp, IFCAP_VLAN_HWCSUM);
		}

		/* Rx vlan strip & tx vlan insert offloads */
		if ((mask & IFCAP_VLAN_HWTAGGING) &&
		    (if_getcapabilities(ifp) & IFCAP_VLAN_HWTAGGING)) {
			if_togglecapenable(ifp, IFCAP_VLAN_HWTAGGING);
			hw_features ^= (ETH_HW_VLAN_TX_TAG | ETH_HW_VLAN_RX_STRIP);
		}

		if ((mask & IFCAP_VLAN_HWFILTER) &&
		    (if_getcapabilities(ifp) & IFCAP_VLAN_HWFILTER)) {
			if_togglecapenable(ifp, IFCAP_VLAN_HWFILTER);
			hw_features ^= IFCAP_VLAN_HWFILTER;
		}

		// tso offloads
		if ((mask & IFCAP_TSO4) &&
		    (if_getcapabilities(ifp) & IFCAP_TSO4)) {
			if_togglecapenable(ifp, IFCAP_TSO4);
			hw_features ^= ETH_HW_TSO;
		}

		if ((mask & IFCAP_TSO6) &&
		    (if_getcapabilities(ifp) & IFCAP_TSO6)) {
			if_togglecapenable(ifp, IFCAP_TSO6);
			hw_features ^= ETH_HW_TSO_IPV6;
		}

		if ((mask & IFCAP_VLAN_HWTSO))
			if_togglecapenable(ifp, IFCAP_VLAN_HWTSO);

		if ((if_getcapenable(ifp) & IFCAP_TSO))
			if_sethwassistbits(ifp, CSUM_TSO, 0);
		else
			if_sethwassistbits(ifp, 0, CSUM_TSO);

		/* Enable offloads on hardware */
		error = ionic_set_hw_features(lif, hw_features);
		if (error) {
			IONIC_NETDEV_ERROR(ifp, "Failed to set capabilities, err: %d\n",
					   error);
			IONIC_LIF_UNLOCK(lif);
			break;
		}

		IONIC_LIF_UNLOCK(lif);
		VLAN_CAPABILITIES(ifp);
		break;

	case SIOCSIFMTU:
		IONIC_NETDEV_INFO(ifp, "ioctl: SIOCSIFMTU (Set Interface MTU)\n");
		if (lif->ionic->is_mgmt_nic) {
			if_printf(ifp, "MTU change not allowed\n");
			error = EINVAL;
			break;
		}
		IONIC_LIF_LOCK(lif);
		error = ionic_change_mtu(ifp, ifr->ifr_mtu);
		if (error)
			IONIC_NETDEV_ERROR(ifp, "Failed to set mtu %d, err: %d\n",
					   ifr->ifr_mtu, error);
		IONIC_LIF_UNLOCK(lif);
		break;

	case SIOCADDMULTI:
	case SIOCDELMULTI:
		IONIC_NETDEV_INFO(ifp, "ioctl: %s (Add/Del Multicast Filter)\n",
				  (command == SIOCADDMULTI) ? "SIOCADDMULTI" : "SIOCDELMULTI");
		error =  ionic_set_multi(lif);
		break;

	case SIOCSIFADDR:
		IONIC_NETDEV_INFO(ifp, "ioctl: SIOCSIFADDR (Set interface address)\n");
		/* bringup the interface when ip address is set */
		error = ether_ioctl(ifp, command, data);
		if (error) {
			IONIC_NETDEV_ERROR(ifp, "Failed to set ip, err: %d\n", error);
			break;
		}

		/*
		 * Freebsd network stack can change the admin link state of a
		 * netdev while handling the SIOCSIFADDR ioctl. It will not
		 * call the SIOCSIFFLAGS ioctl. So we need to apply if_flags in
		 * the SIOCSIFADDR ioctl handler.
		 */
		IONIC_LIF_LOCK(lif);
		ionic_iff_up(lif);
		IONIC_LIF_UNLOCK(lif);
		break;

	case SIOCSIFFLAGS:
		IONIC_NETDEV_INFO(ifp, "ioctl: SIOCSIFFLAGS (Set interface flags)\n");
		IONIC_LIF_LOCK(lif);
		ionic_open_or_stop(lif);
		ionic_iff_up(lif);
		IONIC_LIF_UNLOCK(lif);
		break;

	default:
		error = ether_ioctl(ifp, command, data);
		break;
	}

	return (error);
}

uint16_t
ionic_set_rss_type(void)
{
#ifdef RSS
	uint32_t rss_hash_config;
	uint16_t rss_types;

	rss_hash_config = rss_gethashconfig();
	rss_types = 0;
	if (rss_hash_config & RSS_HASHTYPE_RSS_IPV4)
		rss_types |= IONIC_RSS_TYPE_IPV4;
	if (rss_hash_config & RSS_HASHTYPE_RSS_TCP_IPV4)
		rss_types |= IONIC_RSS_TYPE_IPV4_TCP;
	if (rss_hash_config & RSS_HASHTYPE_RSS_UDP_IPV4)
		rss_types |= IONIC_RSS_TYPE_IPV4_UDP;
	if (rss_hash_config & RSS_HASHTYPE_RSS_IPV6)
		rss_types |= IONIC_RSS_TYPE_IPV6;
	if (rss_hash_config & RSS_HASHTYPE_RSS_TCP_IPV6)
		rss_types |= IONIC_RSS_TYPE_IPV6_TCP;
	if (rss_hash_config & RSS_HASHTYPE_RSS_UDP_IPV6)
		rss_types |= IONIC_RSS_TYPE_IPV6_UDP;
#else
	uint16_t rss_types = IONIC_RSS_TYPE_IPV4
			       | IONIC_RSS_TYPE_IPV4_TCP
			       | IONIC_RSS_TYPE_IPV4_UDP
			       | IONIC_RSS_TYPE_IPV6
			       | IONIC_RSS_TYPE_IPV6_TCP
			       | IONIC_RSS_TYPE_IPV6_UDP;
#endif

	return (rss_types);
}
