/*
 * Copyright 2017-2018 Pensando Systems, Inc.  All rights reserved.
 *
 * This program is free software; you may redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 2 of the License.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
 * BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
 * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 */

#include <linux/kernel.h>
#include <linux/if_vlan.h>
#include <linux/dma-mapping.h>

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


#include <sys/sockio.h>
#include <sys/kdb.h>

#include "ionic.h"
#include "ionic_lif.h"
#include "ionic_txrx.h"

#include "opt_rss.h"

#ifdef	RSS
#include <net/rss_config.h>
#include <netinet/in_rss.h>
#endif

MALLOC_DEFINE(M_IONIC, "ionic", "Pensando IONIC Ethernet adapter");
static int
ionic_ioctl(struct ifnet *ifp, u_long command, caddr_t data);

static SYSCTL_NODE(_hw, OID_AUTO, ionic, CTLFLAG_RD, 0,
                   "Pensando Ethernet parameters");

int ionic_max_queues = 0;
TUNABLE_INT("hw.ionic.max_queues", &ionic_max_queues);
SYSCTL_INT(_hw_ionic, OID_AUTO, max_queues, CTLFLAG_RDTUN,
    &ionic_max_queues, 0, "Number of Queues");

int adminq_descs = 16;
TUNABLE_INT("hw.ionic.adminq_descs", &adminq_descs);
SYSCTL_INT(_hw_ionic, OID_AUTO, adminq_descs, CTLFLAG_RDTUN,
    &adminq_descs, 0, "Number of Admin descriptors");

int ntxq_descs = 128;
TUNABLE_INT("hw.ionic.tx_descs", &ntxq_descs);
SYSCTL_INT(_hw_ionic, OID_AUTO, tx_descs, CTLFLAG_RDTUN,
    &ntxq_descs, 0, "Number of Tx descriptors");

int nrxq_descs = 128;
TUNABLE_INT("hw.ionic.rx_descs", &nrxq_descs);
SYSCTL_INT(_hw_ionic, OID_AUTO, rx_descs, CTLFLAG_RDTUN,
    &nrxq_descs, 0, "Number of Rx descriptors");


#ifdef notyet //Why do we need to recycle?
static void ionic_rx_recycle(struct queue *q, struct desc_info *desc_info,
			     struct mbuf *m)
{
	struct rxq_desc *old = desc_info->desc;
	struct rxq_desc *new = q->head->desc;

	//IONIC_DEV_TRACE(q->lif->ionic->dev, "\n");

	new->addr = old->addr;
	new->len = old->len;

	ionic_q_post(q, true, ionic_rx_input, m);
}
#endif

/* Update mbuf with correct checksum etc. */
static void ionic_rx_checksum(struct mbuf *m, struct rxq_comp *comp, struct rx_stats *stats)
{

	m->m_pkthdr.csum_flags = 0;
	if (comp->csum_ip_ok) {
		m->m_pkthdr.csum_flags = CSUM_IP_CHECKED | CSUM_IP_VALID;
		stats->checksum_ip_ok++;
		m->m_pkthdr.csum_data = htons(0xffff);
	}

	if (m->m_pkthdr.csum_flags && (comp->csum_tcp_ok || comp->csum_udp_ok)) {
		m->m_pkthdr.csum_flags |= CSUM_DATA_VALID | CSUM_PSEUDO_HDR;
		stats->checksum_l4_ok++;
	}

	if (comp->csum_ip_bad) {
		stats->checksum_ip_bad++;
	}

	if (comp->csum_tcp_bad || comp->csum_udp_bad) {
		stats->checksum_l4_bad++;
	}
}

/*
 * Return true if its TCP for LRO.
 */
static bool ionic_rx_rss(struct mbuf *m, struct rxq_comp *comp, int qnum, struct rx_stats *stats)
{
	bool tcp = false;

	if (comp->rss_type != 0) {
		m->m_pkthdr.flowid = comp->rss_hash;
#ifdef RSS
		switch (comp->rss_type) {
		case RXQ_COMP_RSS_TYPE_IPV4:
			M_HASHTYPE_SET(m, M_HASHTYPE_RSS_IPV4);
			stats->rss_ip4++;
			break;
		case RXQ_COMP_RSS_TYPE_IPV4_TCP:
			M_HASHTYPE_SET(m, M_HASHTYPE_RSS_TCP_IPV4);
			stats->rss_tcp_ip4++;
			tcp = true;
			break;
		case RXQ_COMP_RSS_TYPE_IPV4_UDP:
			M_HASHTYPE_SET(m, M_HASHTYPE_RSS_UDP_IPV4);
			stats->rss_udp_ip4++;
			break;
		case RXQ_COMP_RSS_TYPE_IPV6:
			M_HASHTYPE_SET(m, M_HASHTYPE_RSS_IPV6);
			stats->rss_ip6++;
			break;
		case RXQ_COMP_RSS_TYPE_IPV6_TCP:
			M_HASHTYPE_SET(m, M_HASHTYPE_RSS_TCP_IPV6);
			stats->rss_tcp_ip6++;
			tcp = true;
			break;
		case RXQ_COMP_RSS_TYPE_IPV6_UDP:
			M_HASHTYPE_SET(m, M_HASHTYPE_RSS_UDP_IPV6);
			stats->rss_udp_ip6++;
			break;
		case RXQ_COMP_RSS_TYPE_IPV6_EX:
			M_HASHTYPE_SET(m, M_HASHTYPE_RSS_IPV6_EX);
			stats->rss_ip6_ex++;
			break;
		case RXQ_COMP_RSS_TYPE_IPV6_TCP_EX:
			M_HASHTYPE_SET(m, M_HASHTYPE_RSS_TCP_IPV6_EX);
			stats->rss_tcp_ip6_ex++;
			tcp = true;
			break;
		case RXQ_COMP_RSS_TYPE_IPV6_UDP_EX:
			M_HASHTYPE_SET(m, M_HASHTYPE_RSS_UDP_IPV6_EX);
			stats->rss_udp_ip6_ex++;
			break;
		default:	/* XXX: Not used */
			M_HASHTYPE_SET(m, M_HASHTYPE_OPAQUE_HASH);
			stats->rss_unknown++;
			break;
		}
#else
		M_HASHTYPE_SET(m, M_HASHTYPE_OPAQUE_HASH);
#endif

	} else {
		m->m_pkthdr.flowid = qnum;
		M_HASHTYPE_SET(m, M_HASHTYPE_OPAQUE_HASH);
	}

	return(tcp);
}

void ionic_rx_input(struct rx_qcq *rxqcq, struct ionic_rx_buf *rxbuf,
			   struct rxq_comp *comp, 	struct rxq_desc *desc)
{
	struct mbuf *m = rxbuf->m;
	struct rx_stats *stats = &rxqcq->stats;
	dma_addr_t dma_addr;
	int error;
	bool use_lro;

	KASSERT(m, ("mbuf is NULL"));

	if (comp->status) {
		// TODO record errors
		IONIC_NETDEV_QWARN(rxqcq, "RX Status %d\n", comp->status);
		//Why do we need this? just free mbuf
		//ionic_rx_recycle(rxqcq, desc_info, m);

		m_freem(m);
		return;
	}

#ifdef HAPS
	if (comp->len > ETHER_MAX_FRAME(netdev, ETHERTYPE_VLAN, 1)) {
		IONIC_NETDEV_QWARN(q, "RX PKT TOO LARGE!  comp->len %d\n", comp->len);
		ionic_rx_recycle(q, desc_info, m);
		return;
	}
#endif

	stats->pkts++;
	stats->bytes += comp->len;

	// TODO add copybreak to avoid allocating a new m for small receive

	dma_addr = rxbuf->pa_addr;

	dma_addr -= ETHER_ALIGN;

	bus_dmamap_sync(rxqcq->buf_tag, rxbuf->dma_map, BUS_DMASYNC_POSTREAD);

	prefetch(m->data - NET_IP_ALIGN);
	m->m_pkthdr.rcvif = rxqcq->lif->netdev;
	m->m_pkthdr.len = comp->len;
	m->m_len = comp->len;

	/* Update the checksum if h/w has calculated. */
	ionic_rx_checksum(m, comp, stats);

	/* Populate mbuf with h/w RSS hash, type etc. */
	use_lro = ionic_rx_rss(m, comp, rxqcq->index, stats);

	/*
	 * Use h/w RSS engine of L4 checksum to determine if its TCP packet.
	 * XXX: add more cases of LRO.
	 * XXX: Looks like BSD 11.0 LRO is broken
	 */
	if ((rxqcq->lro.ifp != NULL) && (use_lro || comp->csum_tcp_ok)) {
		if (rxqcq->lro.lro_cnt != 0) {
			if ((error = tcp_lro_rx(&rxqcq->lro, m, 0)) == 0) {/* third arg -Checksum?? */
				IONIC_NETDEV_RX_TRACE(rxqcq, "sending to lro\n");
				return;
			}
			IONIC_NETDEV_QWARN(rxqcq, "lro failed, error: %d\n", error);
		}
	}

#if 0
	IONIC_NETDEV_QINFO(rxqcq, "len %u\n", m->m_len);

	IONIC_NETDEV_QINFO(rxqcq,
		  "data %02hhx %02hhx %02hhx %02hhx %02hhx %02hhx %02hhx %02hhx\n",
		  m->m_data[0], m->m_data[1],
		  m->m_data[2], m->m_data[3],
		  m->m_data[4], m->m_data[5],
		  m->m_data[6], m->m_data[7]);
	IONIC_NETDEV_QINFO(rxqcq,
		  "data end %02hhx %02hhx %02hhx %02hhx %02hhx %02hhx %02hhx %02hhx\n",
		  m->m_data[m->m_len - 8], m->m_data[m->m_len - 7],
		  m->m_data[m->m_len - 6], m->m_data[m->m_len - 5],
		  m->m_data[m->m_len - 4], m->m_data[m->m_len - 3],
		  m->m_data[m->m_len - 2], m->m_data[m->m_len - 1]);
#endif

	rxqcq->lif->netdev->if_input(rxqcq->lif->netdev, m);
}

/* XXX: this should be named alloc and map */
static int ionic_rx_mbuf_alloc(struct rx_qcq *rxqcq, int index, int len)
{
	bus_dma_segment_t  seg[1];
	struct ionic_rx_buf *rxbuf;
	struct mbuf *m;
	struct rx_stats *stats = &rxqcq->stats;
	int nsegs, error;

	rxbuf = &rxqcq->rxbuf[index];

	m = m_getjcl(M_NOWAIT, MT_DATA, M_PKTHDR, len);
	if (m == NULL) {
		stats->alloc_err++;
		return (ENOMEM);
	}

	m->m_pkthdr.len = m->m_len = len;
	rxbuf->m = m;

	error = bus_dmamap_load_mbuf_sg(rxqcq->buf_tag, rxbuf->dma_map, m, seg, &nsegs, BUS_DMA_NOWAIT);
	if (error) {
		stats->dma_map_err++;
		return (error);
	}

	bus_dmamap_sync(rxqcq->buf_tag, rxbuf->dma_map, BUS_DMASYNC_PREREAD);
	rxbuf->pa_addr = seg[0].ds_addr;

	return (0);
}

static void ionic_rx_mbuf_free(struct rx_qcq *rxqcq, struct ionic_rx_buf *rxbuf)
{
	bus_dmamap_sync(rxqcq->buf_tag, rxbuf->dma_map, BUS_DMASYNC_POSTREAD);
	m_freem(rxbuf->m);

	rxbuf->m = NULL;
}

void ionic_rx_fill(struct rx_qcq *rxqcq)
{
	struct rxq_desc *desc;
	struct ionic_rx_buf *rxbuf;
	int error, i, index;

	for ( i = 0 ; i < rxqcq->num_descs ; i++) {
		index = rxqcq->cmd_head_index;
		rxbuf = &rxqcq->rxbuf[index];
		desc = &rxqcq->cmd_ring[index];

		if ((error = ionic_rx_mbuf_alloc(rxqcq, index, rxqcq->lif->buf_len))) {
			IONIC_NETDEV_QERR(rxqcq, "rx_fill mbuf alloc failed for p_index :%d, error: %d\n",
				index, error);
			break;
		}

		desc->addr = rxbuf->pa_addr;
		desc->len = rxqcq->lif->buf_len;
		desc->opcode = RXQ_DESC_OPCODE_SIMPLE;

		/* XXX ping doorbell on 4 rx submission. */
		ionic_ring_doorbell(rxqcq->db, rxqcq->qid, rxqcq->cmd_head_index);

		//IONIC_NETDEV_QINFO(rxqcq, "rx_fill index :%d mbuf pa: 0x%lx \n", index, rxbuf->pa_addr);
		/* Q full condition. */
		if (rxqcq->cmd_head_index + 1 == rxqcq->cmd_tail_index)
			break;

		rxqcq->cmd_head_index = (rxqcq->cmd_head_index + 1) % rxqcq->num_descs;
	 }
	 
	IONIC_NETDEV_RX_TRACE(rxqcq, "rx_fill head: %d tail: %d descs: %d filled %d\n",  
		rxqcq->cmd_head_index, rxqcq->cmd_tail_index, rxqcq->num_descs, i);

}

/* XXX: where is the doorbell ping for refill ? */
void ionic_rx_refill(struct rx_qcq *rxqcq)
{
	struct ionic_rx_buf *rxbuf;
	int error, i, index;

	IONIC_NETDEV_RX_TRACE(rxqcq, "rx_refill producer: %d consumer :%d num_descs: %d\n",
		rxqcq->cmd_head_index, rxqcq->cmd_tail_index, rxqcq->num_descs);

	for ( i = 0 ; i < rxqcq->num_descs && rxqcq->cmd_head_index != rxqcq->cmd_tail_index ; i++) {
		index = rxqcq->cmd_tail_index;
		rxbuf = &rxqcq->rxbuf[index];

		if (rxbuf->m != NULL) {
			ionic_rx_mbuf_free(rxqcq, rxbuf);
		}

		if ((error = ionic_rx_mbuf_alloc(rxqcq, index, rxqcq->lif->buf_len))) {
			IONIC_NETDEV_QERR(rxqcq, "rx_refill mbuf alloc failed for p_index :%d, error: %d\n",
				index, error);
			break;
		}

		IONIC_NETDEV_RX_TRACE(rxqcq, "rx_refill index :%d mbuf pa: 0x%lx \n", index, rxbuf->pa_addr);

		rxqcq->cmd_tail_index = (rxqcq->cmd_tail_index + 1) % rxqcq->num_descs;
	 }

	IONIC_NETDEV_RX_TRACE(rxqcq, "filled  %d rxbufs\n", i);
}


void ionic_rx_empty(struct rx_qcq *rxqcq)
{
	struct ionic_rx_buf *rxbuf;
	int i, index;

	IONIC_NETDEV_RX_TRACE(rxqcq, "\n");

	 for (i = 0 ; i < rxqcq->num_descs && (rxqcq->cmd_head_index != rxqcq->cmd_tail_index) ; i++) {
		index = rxqcq->cmd_tail_index;
		rxbuf = &rxqcq->rxbuf[index];

		ionic_rx_mbuf_free(rxqcq, rxbuf);
		bus_dmamap_unload(rxqcq->buf_tag, rxbuf->dma_map);
		bus_dmamap_destroy(rxqcq->buf_tag, rxbuf->dma_map);

		rxqcq->cmd_tail_index = (rxqcq->cmd_tail_index + 1) % rxqcq->num_descs;
	}
}


static int
ionic_get_header_size(struct tx_stats *stats, struct mbuf *mb, uint16_t *eth_type, int *proto, int *hlen)
{
	struct ether_vlan_header *eh;
	struct tcphdr *th;
	struct ip *ip;
	int ip_hlen, tcp_hlen;
	struct ip6_hdr *ip6;
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


static int ionic_tx_setup(struct tx_qcq *txqcq, struct mbuf *m)
{
	struct txq_desc *desc;
	struct tx_stats *stats = &txqcq->stats;
	struct ionic_tx_buf *txbuf;
	bool offload = false;
	int error, index, len, nsegs;
	bus_dma_segment_t  seg[1];

	index = txqcq->cmd_head_index;
	desc = &txqcq->cmd_ring[index];
	txbuf = &txqcq->txbuf[index];
	len = m->m_len;

	error = bus_dmamap_load_mbuf_sg(txqcq->buf_tag, txbuf->dma_map, m, seg, &nsegs, BUS_DMA_NOWAIT);
	if (error) {
		IONIC_NETDEV_QERR(txqcq, "failed to map xmit, error: %d\n", error);
		stats->dma_map_err++;
		return (error);
	}

	if (!ionic_tx_avail(txqcq, nsegs)) {
		stats->no_descs++;
		bus_dmamap_unload(txqcq->buf_tag, txbuf->dma_map);
		/* This is a hard error, log it */
		IONIC_NETDEV_QERR(txqcq, "BUG! Tx ring full when queue awake!\n");
		return (ENOBUFS);
	}

	bus_dmamap_sync(txqcq->buf_tag, txbuf->dma_map, BUS_DMASYNC_PREWRITE);
	txbuf->pa_addr = seg[0].ds_addr;
	txbuf->m = m;
	IONIC_NETDEV_TX_TRACE(txqcq, "VA: %p DMA addr: 0x%lx nsegs: %d length: 0x%lx\n", m, txbuf->pa_addr, nsegs, seg[0].ds_len);

	if (m->m_pkthdr.csum_flags & CSUM_IP) {
		desc->l3_csum = 1;
		offload = true;
	}
	if (m->m_pkthdr.csum_flags & (CSUM_IP | CSUM_TCP | CSUM_UDP | CSUM_UDP_IPV6 | CSUM_TCP_IPV6)) {
		desc->l4_csum = 1;
		offload = true;
	}
	desc->opcode = offload ? TXQ_DESC_OPCODE_CALC_CSUM_TCPUDP : TXQ_DESC_OPCODE_CALC_NO_CSUM;
	desc->num_sg_elems = 0;
	desc->len = m->m_len;
	desc->addr = txbuf->pa_addr;
	desc->vlan_tci = 0;
	desc->hdr_len = 0;
	desc->csum_offset = 0;
	desc->V = 0;
	desc->C = 1;
	desc->O = 0;

//	IONIC_NETDEV_ERROR(q->lif->netdev, "checksum offload hdr_len: %d csum_offset: %d\n",
//		start, m->m_pkthdr.csum_data);
	if (offload)
		stats->csum_offload++;
	else
		stats->no_csum_offload++;

	return 0;
}


/*
 * TSO
 */
static int ionic_tx_tso_setup(struct tx_qcq *txqcq, struct mbuf *m)
{
	uint32_t mss = m->m_pkthdr.tso_segsz;
	struct tx_stats *stats = &txqcq->stats;
#ifdef notyet
	struct desc_info *abort = q->head;
	struct desc_info *rewind = abort;
#endif
	struct ifnet* ifp = txqcq->lif->netdev;
	struct ionic_tx_buf *txbuf;
	struct txq_desc *desc;
	struct txq_sg_desc *sg;
	uint16_t eth_type;
	int i, index, hdr_len, proto, error, nsegs;

	bus_dma_segment_t  seg[IONIC_TX_MAX_SG_ELEMS + 1]; /* Extra for the first segment. */

	if ((error = ionic_get_header_size(stats, m, &eth_type, &proto, &hdr_len))) {
		IONIC_NETDEV_ERROR(ifp, "mbuf packet disacarded\n");
		stats->bad_ethtype++;
		return (error);
	}

	if (proto != IPPROTO_TCP) {
		return (EINVAL);
	}

	index = txqcq->cmd_head_index;
	desc = &txqcq->cmd_ring[index];
	txbuf = &txqcq->txbuf[index];
	sg = &txqcq->sg_ring[index];

	txbuf->m = m;

	error = bus_dmamap_load_mbuf_sg(txqcq->buf_tag, txbuf->dma_map, m, seg, &nsegs, BUS_DMA_NOWAIT);
	if (error) {
		IONIC_NETDEV_QERR(txqcq, "failed to map TSO xmit, error: %d\n", error);
		stats->dma_map_err++;
		return (error);
	}

	if (nsegs > IONIC_TX_MAX_SG_ELEMS +1) {
		bus_dmamap_unload(txqcq->buf_tag, txbuf->dma_map);
		IONIC_NETDEV_QERR(txqcq, "too many fragments: %d\n", nsegs);
//		stats->tso_map_err++;
		return (EFBIG);
	}

	if (!ionic_tx_avail(txqcq, nsegs)) {
		stats->no_descs++;
		/* This is a hard error, log it */
		bus_dmamap_unload(txqcq->buf_tag, txbuf->dma_map);
		IONIC_NETDEV_QERR(txqcq, "BUG! Tx ring full when queue awake!\n");
		return (ENOBUFS);
	}

	bus_dmamap_sync(txqcq->buf_tag, txbuf->dma_map, BUS_DMASYNC_PREWRITE);

	desc->opcode = TXQ_DESC_OPCODE_TSO;
//	desc->vlan_tci = vlan_tci;
	desc->hdr_len = hdr_len;
//	desc->V = has_vlan;
	desc->C = 1;
//	desc->O = outer_csum;
	desc->S = 1;
	desc->E = (nsegs == 1) ? 1 : 0;
	desc->mss = mss;

	desc->num_sg_elems = nsegs;
	desc->len = seg[0].ds_len;
	desc->addr = txbuf->pa_addr = seg[0].ds_addr;

	IONIC_NETDEV_TX_TRACE(txqcq, "TSO: VA: %p DMA addr: 0x%lx nsegs: %d length: 0x%x\n",
		m, txbuf->pa_addr, nsegs, m->m_pkthdr.len);
	IONIC_NETDEV_TX_TRACE(txqcq, "sg[0] pa: 0x%lx length: 0x%hx\n",  sg->elems[0].addr, sg->elems[0].len);
	/* Now populate SG list, past the first fragment. */
	for ( i = 0 ; i < nsegs -1 ; i++) {
		sg->elems[i].addr = seg[i + 1].ds_addr;
		sg->elems[i].len = seg[i + 1].ds_len;
		IONIC_NETDEV_TX_TRACE(txqcq, "sg[%d] pa: 0x%lx length: 0x%hx\n", i, sg->elems[i].addr, sg->elems[i].len);
	}

	txqcq->cmd_head_index = (txqcq->cmd_head_index + 1) % txqcq->num_descs;

	return 0;

//err_out_abort:
#ifdef notyet
	while (rewind->desc != q->head->desc) {
		ionic_tx_clean(q, rewind, NULL, NULL);
		rewind = rewind->next;
	}
	q->head = abort;
#endif

	return -ENOMEM;
}


static int ionic_xmit(struct tx_qcq* txqcq, struct mbuf **head)
{
	struct tx_stats *stats;
	int err;
	struct mbuf* m;

	stats = &txqcq->stats;
	m = *head;

	if (m->m_pkthdr.csum_flags & CSUM_TSO)
		err = ionic_tx_tso_setup(txqcq, m);
	else
		err = ionic_tx_setup(txqcq, m);

	if (err)
		goto err_out_drop;

	stats->pkts++;
	stats->bytes += m->m_len;

	/* XXX ping doorbell on 4 rx submission. */
	ionic_ring_doorbell(txqcq->db, txqcq->qid, txqcq->cmd_head_index);
	IONIC_NETDEV_TX_TRACE(txqcq, "db: %p Qid: %d index: %d\n",
		 txqcq->db, txqcq->qid, txqcq->cmd_head_index);

	txqcq->cmd_head_index = (txqcq->cmd_head_index + 1) % txqcq->num_descs;
	return 0;

err_out_drop:
	stats->drop++;
	m_freem(m);
	return 0;
}


int ionic_start_xmit_locked(struct ifnet* ifp, 	struct tx_qcq* txqcq)
{
	struct mbuf *m;
	int err;
	int processed = 0;

	IONIC_NETDEV_TX_TRACE(txqcq, "head: %d tail: %d\n",
		 txqcq->cmd_head_index, txqcq->cmd_tail_index);

	if ((ifp->if_drv_flags & IFF_DRV_RUNNING) == 0)
	    return (ENETDOWN);


	while ((m = drbr_peek(ifp, txqcq->br)) != NULL) {
		processed++;
		if ((err = ionic_xmit(txqcq, &m)) != 0) {
			if (m == NULL)
				drbr_advance(ifp, txqcq->br);
			else
				drbr_putback(ifp, txqcq->br, m);
			break;
		}
		drbr_advance(ifp, txqcq->br);
		/* Send a copy of the frame to the BPF listener */
		ETHER_BPF_MTAP(ifp, m);
		if ((ifp->if_drv_flags & IFF_DRV_RUNNING) == 0)
			break;
	}

	if (processed > 8)
		ionic_tx_clean(txqcq, 100);

	return (err);
}

int ionic_start_xmit(struct net_device *netdev, struct mbuf *m)
{
	struct lif *lif = netdev_priv(netdev);
	struct ifnet* ifp = lif->netdev;
	struct tx_qcq* txqcq; 
	int err, qid = 0;
	int bucket;

	if (M_HASHTYPE_GET(m) != M_HASHTYPE_NONE) {
#ifdef RSS
		if (rss_hash2bucket(m->m_pkthdr.flowid,
			M_HASHTYPE_GET(m), &bucket) == 0)
			qid = bucket % lif->ntxqcqs;
	 	else
#endif
		qid = (m->m_pkthdr.flowid % 128) % lif->ntxqcqs;
	}

	txqcq = lif->txqcqs[qid];

	err = drbr_enqueue(ifp, txqcq->br, m);
	if (err)
		return (err);

	if (mtx_trylock(&txqcq->mtx)) {
		ionic_start_xmit_locked(ifp, txqcq);
		mtx_unlock(&txqcq->mtx);
	} else
		taskqueue_enqueue(txqcq->taskq, &txqcq->task);

	return (0);
}


static uint64_t
ionic_get_counter(struct ifnet *ifp, ift_counter cnt)
{
	struct lif* lif = if_getsoftc(ifp);
	/* XXX : fix for all Qs. */
	struct tx_stats* txstat = &lif->txqcqs[0]->stats;
	struct rx_stats* rxstat = &lif->rxqcqs[0]->stats;

	switch (cnt) {
	case IFCOUNTER_IPACKETS:
		return (rxstat->pkts);
	case IFCOUNTER_OPACKETS:
		return (txstat->pkts);
	case IFCOUNTER_IBYTES:
		return (rxstat->bytes);
	case IFCOUNTER_OBYTES:
		return (txstat->bytes);
#ifdef notyet
	case IFCOUNTER_IMCASTS:
		return (ionic->imcasts);
	case IFCOUNTER_OMCASTS:
		return (ionic->omcasts);
	case IFCOUNTER_COLLISIONS:
		return (0);
	case IFCOUNTER_IQDROPS:
		return (rxstat->drop);
	case IFCOUNTER_OQDROPS:
		rv = 0;
		txr = ionic->tx_rings;
		for (int i = 0; i < ionic->num_queues; i++, txr++)
			rv += txr->br->br_drops;
		return (rv);
	case IFCOUNTER_IERRORS:
		return (ionic->ierrors);
#endif
	default:
		return (if_get_counter_default(ifp, cnt));
	}
}

int
ionic_lif_netdev_alloc(struct lif* lif, int ndescs)
{
	struct ifnet* ifp;

	KASSERT(lif->ionic, ("lif: %s ionic == NULL", lif->name));

	ifp = if_alloc(IFT_ETHER);
	if (ifp == NULL) {
		dev_err(lif->ionic->dev, "Cannot allocate ifnet, aborting\n");
		return -ENOMEM;
	}

	if_initname(ifp, "ionic", device_get_unit(lif->ionic->dev->bsddev));

	ifp->if_softc = lif;
	ifp->if_mtu = ETHERMTU;
	ifp->if_init = ionic_open;
	ifp->if_flags = IFF_BROADCAST | IFF_SIMPLEX | IFF_MULTICAST;
	ifp->if_ioctl = ionic_ioctl;
	ifp->if_transmit = ionic_start_xmit;
	ifp->if_qflush = if_qflush;
	ifp->if_snd.ifq_maxlen = ndescs;
	if_setgetcounterfn(ifp, ionic_get_counter);
	/* Capabilities are set later on. */

	/* Connect lif to ifnet. */
	lif->netdev = ifp;

	mtx_init(&lif->mtx, "ionic lif lock", NULL, MTX_DEF);

	return (0);
}

static void
ionic_lif_add_rxtstat(struct rx_qcq *rxqcq, struct sysctl_ctx_list *ctx,
    struct sysctl_oid_list *queue_list)
{
	struct lro_ctrl* lro = &rxqcq->lro;
	struct rx_stats* rxstat = &rxqcq->stats;

    SYSCTL_ADD_UINT(ctx, queue_list, OID_AUTO, "head", CTLFLAG_RD,
            &rxqcq->cmd_head_index, 0, "Head index");
       SYSCTL_ADD_UINT(ctx, queue_list, OID_AUTO, "tail", CTLFLAG_RD,
            &rxqcq->cmd_tail_index, 0, "Tail index");
       SYSCTL_ADD_UINT(ctx, queue_list, OID_AUTO, "comp_index", CTLFLAG_RD,
            &rxqcq->comp_index, 0, "Completion index");
    	SYSCTL_ADD_UINT(ctx, queue_list, OID_AUTO, "num_descs", CTLFLAG_RD,
        	&rxqcq->num_descs, 0, "Number of descriptors");

		SYSCTL_ADD_ULONG(ctx, queue_list, OID_AUTO, "dma_setup_error", CTLFLAG_RD,
            &rxstat->dma_map_err,"DMA map setup error");
		SYSCTL_ADD_ULONG(ctx, queue_list, OID_AUTO, "alloc_error", CTLFLAG_RD,
            &rxstat->alloc_err, "Buffer allocation error");

        SYSCTL_ADD_ULONG(ctx, queue_list, OID_AUTO, "pkts", CTLFLAG_RD,
            &rxstat->pkts, "Rx Packets");
        SYSCTL_ADD_ULONG(ctx, queue_list, OID_AUTO, "bytes", CTLFLAG_RD,
            &rxstat->bytes, "Rx bytes");
        SYSCTL_ADD_ULONG(ctx, queue_list, OID_AUTO, "ip_checksum_ok", CTLFLAG_RD,
            &rxstat->checksum_ip_ok, "IP checksum OK");
        SYSCTL_ADD_ULONG(ctx, queue_list, OID_AUTO, "ip_checksum_bad", CTLFLAG_RD,
            &rxstat->checksum_ip_bad, "IP checksum bad");
        SYSCTL_ADD_ULONG(ctx, queue_list, OID_AUTO, "L4_checksum_ok", CTLFLAG_RD,
            &rxstat->checksum_l4_ok, "L4 checksum OK");
        SYSCTL_ADD_ULONG(ctx, queue_list, OID_AUTO, "L4_checksum_bad", CTLFLAG_RD,
            &rxstat->checksum_l4_bad, "L4 checksum bad");
        SYSCTL_ADD_ULONG(ctx, queue_list, OID_AUTO, "isr_count", CTLFLAG_RD,
            &rxstat->isr_count, "ISR count");

		/* LRO related. */
		SYSCTL_ADD_ULONG(ctx, queue_list, OID_AUTO, "lro_queued", CTLFLAG_RD,
            &lro->lro_queued, "LRO packets queued");
		SYSCTL_ADD_ULONG(ctx, queue_list, OID_AUTO, "lro_flushed", CTLFLAG_RD,
            &lro->lro_flushed, "LRO packets flushed");

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

		SYSCTL_ADD_ULONG(ctx, queue_list, OID_AUTO, "rss_ip6_ex", CTLFLAG_RD,
			&rxstat->rss_ip6_ex, "RSS IPv6 extension packets");
		SYSCTL_ADD_ULONG(ctx, queue_list, OID_AUTO, "rss_tcp_ip6_ex", CTLFLAG_RD,
			&rxstat->rss_tcp_ip6_ex, "RSS TCP/IPv6 extension packets");
		SYSCTL_ADD_ULONG(ctx, queue_list, OID_AUTO, "rss_udp_ip6_ex", CTLFLAG_RD,
			&rxstat->rss_udp_ip6_ex, "RSS UDP/IPv6 extension packets");
}

static void
ionic_lif_add_txtstat(struct tx_qcq* txqcq, struct sysctl_ctx_list *ctx,
    struct sysctl_oid_list *list)
{
	struct tx_stats* txstat = &txqcq->stats;

	SYSCTL_ADD_UINT(ctx, list, OID_AUTO, "head", CTLFLAG_RD,
        &txqcq->cmd_head_index, 0, "Head index");
    SYSCTL_ADD_UINT(ctx, list, OID_AUTO, "tail", CTLFLAG_RD,
        &txqcq->cmd_tail_index, 0, "Tail index");
    SYSCTL_ADD_UINT(ctx, list, OID_AUTO, "comp_index", CTLFLAG_RD,
        &txqcq->comp_index, 0, "Completion index");
    SYSCTL_ADD_UINT(ctx, list, OID_AUTO, "num_descs", CTLFLAG_RD,
        &txqcq->num_descs, 0, "Number of descriptors");

	SYSCTL_ADD_ULONG(ctx, list, OID_AUTO, "dma_map_error", CTLFLAG_RD,
        &txstat->dma_map_err, "DMA mapping error");
	SYSCTL_ADD_ULONG(ctx, list, OID_AUTO, "tx_clean", CTLFLAG_RD,
        &txstat->clean, "Tx clean");
	SYSCTL_ADD_ULONG(ctx, list, OID_AUTO, "pkts_drop", CTLFLAG_RD,
        &txstat->drop, "Packets were dropped");
	SYSCTL_ADD_ULONG(ctx, list, OID_AUTO, "no_descs", CTLFLAG_RD,
        &txstat->no_descs, "Descriptors not available");
	SYSCTL_ADD_ULONG(ctx, list, OID_AUTO, "linearize", CTLFLAG_RD,
        &txstat->linearize, "Linearize  mbuf");
	SYSCTL_ADD_ULONG(ctx, list, OID_AUTO, "bad_ethtype", CTLFLAG_RD,
        &txstat->bad_ethtype, "Tx malformed Ethernet");

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

}

static void
ionic_setup_device_stats(struct lif *lif)
{
	//device_t dev = lif->ionic->dev;
	struct sysctl_ctx_list *ctx = &lif->sysctl_ctx;
	struct sysctl_oid *tree = lif->sysctl_ifnet;
	struct sysctl_oid_list *child = SYSCTL_CHILDREN(tree);

	struct sysctl_oid *queue_node;
	struct sysctl_oid_list *queue_list;
	struct adminq* adminq = lif->adminqcq;
	int i;

#define QUEUE_NAME_LEN 32
	char namebuf[QUEUE_NAME_LEN];

	SYSCTL_ADD_UINT(ctx, child, OID_AUTO, "numq", CTLFLAG_RD,
       &lif->ntxqcqs, 0, "Number of Tx/Rx queue pairs");

	snprintf(namebuf, QUEUE_NAME_LEN, "adq");
	queue_node = SYSCTL_ADD_NODE(ctx, child, OID_AUTO, namebuf,
					    CTLFLAG_RD, NULL, "Queue Name");
	queue_list = SYSCTL_CHILDREN(queue_node);

	SYSCTL_ADD_UINT(ctx, queue_list, OID_AUTO, "num_descs", CTLFLAG_RD,
       &adminq->num_descs, 0, "Number of descriptors");
	SYSCTL_ADD_UINT(ctx, queue_list, OID_AUTO, "head", CTLFLAG_RD,
        &adminq->cmd_head_index, 0, "Head index");
	SYSCTL_ADD_UINT(ctx, queue_list, OID_AUTO, "tail", CTLFLAG_RD,
        &adminq->cmd_tail_index, 0, "Tail index");
    SYSCTL_ADD_UINT(ctx, queue_list, OID_AUTO, "comp_index", CTLFLAG_RD,
        &adminq->comp_index, 0, "Completion index");


	for (i = 0; i < lif->nrxqcqs; i++) {
		snprintf(namebuf, QUEUE_NAME_LEN, "rxq%d", i);
		queue_node = SYSCTL_ADD_NODE(ctx, child, OID_AUTO, namebuf,
					    CTLFLAG_RD, NULL, "Queue Name");
		queue_list = SYSCTL_CHILDREN(queue_node);
        ionic_lif_add_rxtstat(lif->rxqcqs[i], ctx, queue_list);
    }

    for (i = 0; i < lif->ntxqcqs; i++) {
		snprintf(namebuf, QUEUE_NAME_LEN, "txq%d", i);
		queue_node = SYSCTL_ADD_NODE(ctx, child, OID_AUTO, namebuf,
					    CTLFLAG_RD, NULL, "Queue Name");
		queue_list = SYSCTL_CHILDREN(queue_node);

		ionic_lif_add_txtstat(lif->txqcqs[i], ctx, queue_list);
    }
}

void
ionic_setup_sysctls(struct lif *lif)
{
    device_t dev = lif->ionic->dev;
    struct ifnet *ifp = lif->netdev;
    char buf[16];

	/* ifnet sysctl tree */
	sysctl_ctx_init(&lif->sysctl_ctx);
	lif->sysctl_ifnet = SYSCTL_ADD_NODE(&lif->sysctl_ctx, SYSCTL_STATIC_CHILDREN(_dev),
	    OID_AUTO, ifp->if_dname, CTLFLAG_RD, 0, "Pwensando I/O NIC");
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
 * Set features.
 */
int
ionic_set_os_features(struct ifnet* ifp, uint32_t hw_features)
{
    /*
	 * Set software only capabilities.
	 * XXX: read counters from h/w using stats_dump
	 */
	ifp->if_capabilities = IFCAP_HWSTATS | IFCAP_LRO;
	if (hw_features & ETH_HW_TX_CSUM)
		ifp->if_capabilities |=  IFCAP_TXCSUM | IFCAP_TXCSUM_IPV6;

	if (hw_features & ETH_HW_RX_CSUM)
		ifp->if_capabilities |=  IFCAP_RXCSUM | IFCAP_RXCSUM_IPV6;

	if (hw_features & ETH_HW_TSO)
		ifp->if_capabilities |=  IFCAP_TSO4;

	if (hw_features & ETH_HW_TSO_IPV6)
		ifp->if_capabilities |=  IFCAP_TSO6;
	/* XXX: add more capabilities. */

	ifp->if_capenable = ifp->if_capabilities;
	IONIC_NETDEV_INFO(ifp, "if_capenable: 0x%x\n", ifp->if_capenable);
	ifp->if_hwassist = 0;

	if (ifp->if_capenable & IFCAP_TSO)
		ifp->if_hwassist |= CSUM_TSO;
	if (ifp->if_capenable & IFCAP_TXCSUM)
		ifp->if_hwassist |= (CSUM_TCP | CSUM_UDP | CSUM_IP);
	if (ifp->if_capenable & IFCAP_TXCSUM_IPV6)
		ifp->if_hwassist |= (CSUM_UDP_IPV6 | CSUM_TCP_IPV6);

	return (0);
}

void
ionic_lif_netdev_free(struct lif* lif)
{

    sysctl_ctx_free(&lif->sysctl_ctx);

	if (lif->netdev) {
		if_free(lif->netdev);
		lif->netdev = NULL;
	}
}

static int
ionic_ioctl(struct ifnet *ifp, u_long command, caddr_t data)
{
	struct lif* lif = if_getsoftc(ifp);
	struct ifreq	*ifr = (struct ifreq *) data;
#if defined(INET) || defined(INET6)
	//struct ifaddr *ifa = (struct ifaddr *)data;
#endif
	int error = 0;
	uint16_t hw_features;
	//bool		avoid_reset = FALSE;

	switch (command) {  
	case SIOCSIFCAP:
	{
		int mask = ifr->ifr_reqcap ^ ifp->if_capenable;
		IONIC_NETDEV_INFO(ifp, "Required: 0x%x enabled: 0x%x\n",  ifr->ifr_reqcap, ifp->if_capenable);
		if (!mask) {
			IONIC_NETDEV_INFO(ifp, "Nothing to do\n");
			break;
		}

		hw_features = lif->hw_features;

		if (mask & (IFCAP_RXCSUM | IFCAP_RXCSUM_IPV6)) {
		//	ifp->if_capenable ^= IFCAP_RXCSUM;
		//	ifp->if_capenable ^= IFCAP_RXCSUM_IPV6;
			hw_features ^= ETH_HW_RX_CSUM;
		}

		if (mask & (IFCAP_TXCSUM | IFCAP_TXCSUM_IPV6)) {
		//	ifp->if_capenable ^= IFCAP_TXCSUM;
		//	ifp->if_capenable ^= IFCAP_TXCSUM_IPV6;
			hw_features ^= ETH_HW_TX_CSUM;
		}

		if (mask & IFCAP_TSO4)
			ifp->if_capenable ^= IFCAP_TSO4;
		if (mask & IFCAP_TSO6)
			ifp->if_capenable ^= IFCAP_TSO6;
		if (mask & IFCAP_LRO)
			ifp->if_capenable ^= IFCAP_LRO;
		if (mask & IFCAP_VLAN_HWTAGGING)
			ifp->if_capenable ^= IFCAP_VLAN_HWTAGGING;
		if (mask & IFCAP_VLAN_HWFILTER)
			ifp->if_capenable ^= IFCAP_VLAN_HWFILTER;
		if (mask & IFCAP_VLAN_HWTSO)
			ifp->if_capenable ^= IFCAP_VLAN_HWTSO;

	//	if (ifp->if_drv_flags & IFF_DRV_RUNNING) {
			IONIC_NETDEV_INFO(lif->netdev, "Updating features 0x%x -> 0x%x", lif->hw_features, hw_features);
			mtx_lock(&lif->mtx);
			error = ionic_set_features(lif, hw_features);
			if (error) {
				IONIC_NETDEV_ERROR(lif->netdev, "Failed to set capbilities, err: %d\n\n", error);
			}
			ionic_reinit_unlock(ifp);
			mtx_unlock(&lif->mtx);
	//	}
	//	VLAN_CAPABILITIES(ifp);
		break;
	}

	default:
			error = ether_ioctl(ifp, command, data);
			break;
	}

	return (error);
}


/* Should move to somewhere else. */
static uint16_t
ionic_set_rss_type(void)
{
#ifdef RSS
	uint32_t rss_hash_config = rss_gethashconfig();
	uint16_t rss_types = 0;

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
	if (rss_hash_config & RSS_HASHTYPE_RSS_IPV6_EX)
		rss_types |= IONIC_RSS_TYPE_IPV6_EX;
	if (rss_hash_config & RSS_HASHTYPE_RSS_TCP_IPV6_EX)
		rss_types |= IONIC_RSS_TYPE_IPV6_TCP_EX;
	if (rss_hash_config & RSS_HASHTYPE_RSS_UDP_IPV4_EX)
		rss_types |= IONIC_RSS_TYPE_IPV6_UDP_EX;
#else
	uint16_t rss_types = IONIC_RSS_TYPE_IPV4
			       | IONIC_RSS_TYPE_IPV4_TCP
			       | IONIC_RSS_TYPE_IPV4_UDP
			       | IONIC_RSS_TYPE_IPV6
			       | IONIC_RSS_TYPE_IPV6_TCP
			       | IONIC_RSS_TYPE_IPV6_UDP
			       | IONIC_RSS_TYPE_IPV6_EX
			       | IONIC_RSS_TYPE_IPV6_TCP_EX
			       | IONIC_RSS_TYPE_IPV6_UDP_EX;
#endif

	return (rss_types);
}

int ionic_lif_rss_setup(struct lif *lif)
{
	struct net_device *netdev = lif->netdev;
	struct device *dev = lif->ionic->dev;
	size_t tbl_size = sizeof(*lif->rss_ind_tbl) * RSS_IND_TBL_SIZE;
	unsigned int i;
	int err;

#ifdef	RSS
	u8 toeplitz_symmetric_key[40];
	rss_getkey((uint8_t *) &toeplitz_symmetric_key);
#else
	static const u8 toeplitz_symmetric_key[] = {
		0x6D, 0x5A, 0x6D, 0x5A, 0x6D, 0x5A, 0x6D, 0x5A,
		0x6D, 0x5A, 0x6D, 0x5A, 0x6D, 0x5A, 0x6D, 0x5A,
		0x6D, 0x5A, 0x6D, 0x5A, 0x6D, 0x5A, 0x6D, 0x5A,
		0x6D, 0x5A, 0x6D, 0x5A, 0x6D, 0x5A, 0x6D, 0x5A,
		0x6D, 0x5A, 0x6D, 0x5A, 0x6D, 0x5A, 0x6D, 0x5A,
	};
#endif


	lif->rss_ind_tbl = dma_zalloc_coherent(dev, tbl_size, &lif->rss_ind_tbl_pa,
					      GFP_KERNEL);

	if (!lif->rss_ind_tbl) {
		IONIC_NETDEV_ERROR(netdev, "%s OOM\n", __func__);
		return -ENOMEM;
	}

	/* Fill indirection table with 'default' values */

	for (i = 0; i < RSS_IND_TBL_SIZE; i++) {
#ifdef RSS
		lif->rss_ind_tbl[i] = rss_get_indirection_to_bucket(i) % lif->nrxqcqs;
#else
		lif->rss_ind_tbl[i] = i % lif->nrxqcqs;
#endif
	}

	err = ionic_rss_ind_tbl_set(lif, NULL);
	if (err)
		goto err_out_free;

	err = ionic_rss_hash_key_set(lif, toeplitz_symmetric_key, ionic_set_rss_type());
	if (err)
		goto err_out_free;

	return 0;

err_out_free:
	dma_free_coherent(dev, tbl_size, lif->rss_ind_tbl,
			  lif->rss_ind_tbl_pa);
	return err;
}

void ionic_lif_rss_teardown(struct lif *lif)
{
	struct device *dev = lif->ionic->dev;
	size_t tbl_size = sizeof(*lif->rss_ind_tbl) * RSS_IND_TBL_SIZE;

	if (!lif->rss_ind_tbl)
		return;

	dma_free_coherent(dev, tbl_size, lif->rss_ind_tbl,
			  lif->rss_ind_tbl_pa);

	lif->rss_ind_tbl = NULL;
}


void ionic_down_link(struct net_device *netdev)
{
#ifdef FREEBSD
	if_link_state_change(netdev, LINK_STATE_DOWN);
	netdev->if_drv_flags &= ~IFF_DRV_RUNNING;
#else
	netif_carrier_off(netdev);
	netif_tx_disable(netdev);
#endif


}

void ionic_up_link(struct net_device *netdev)
{
#ifdef FREEBSD
	if_link_state_change(netdev, LINK_STATE_UP);
	netdev->if_drv_flags |= IFF_DRV_RUNNING;
#else
	netif_carrier_off(netdev);
	netif_tx_disable(netdev);
#endif
}