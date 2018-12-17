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
#include <linux/interrupt.h>
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
static int ionic_ioctl(struct ifnet *ifp, u_long command, caddr_t data);
static SYSCTL_NODE(_hw, OID_AUTO, ionic, CTLFLAG_RD, 0,
                   "Pensando Ethernet parameters");

int ionic_max_queues = 0;
TUNABLE_INT("hw.ionic.max_queues", &ionic_max_queues);
SYSCTL_INT(_hw_ionic, OID_AUTO, max_queues, CTLFLAG_RDTUN,
    &ionic_max_queues, 0, "Number of Queues");

/* XXX: 40 seconds for now. */
int ionic_devcmd_timeout = 40;
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

int ntxq_descs = 16384;
TUNABLE_INT("hw.ionic.tx_descs", &ntxq_descs);
SYSCTL_INT(_hw_ionic, OID_AUTO, tx_descs, CTLFLAG_RDTUN,
    &ntxq_descs, 0, "Number of Tx descriptors");

int nrxq_descs = 16384;
TUNABLE_INT("hw.ionic.rx_descs", &nrxq_descs);
SYSCTL_INT(_hw_ionic, OID_AUTO, rx_descs, CTLFLAG_RDTUN,
    &nrxq_descs, 0, "Number of Rx descriptors");

int ionic_rx_stride = 32;
TUNABLE_INT("hw.ionic.rx_stride", &ionic_rx_stride);
SYSCTL_INT(_hw_ionic, OID_AUTO, rx_stride, CTLFLAG_RWTUN,
    &ionic_rx_stride, 0, "Rx side doorbell ring stride");

int ionic_rx_fill_threshold = 128;
TUNABLE_INT("hw.ionic.rx_fill_threshold", &ionic_rx_fill_threshold);
SYSCTL_INT(_hw_ionic, OID_AUTO, rx_fill_threshold, CTLFLAG_RWTUN,
    &ionic_rx_fill_threshold, 0, "Rx fill threshold");

int ionic_tx_clean_threshold = 128;
TUNABLE_INT("hw.ionic.tx_clean_threshold", &ionic_tx_clean_threshold);
SYSCTL_INT(_hw_ionic, OID_AUTO, tx_clean_threshold, CTLFLAG_RWTUN,
    &ionic_tx_clean_threshold, 0, "Tx clean threshold");

/* Number of packets processed by ISR, rest is handled by task handler. */
int ionic_rx_process_limit = 128;
TUNABLE_INT("hw.ionic.rx_fill_threshold", &ionic_rx_process_limit);
SYSCTL_INT(_hw_ionic, OID_AUTO, rx_process_limit, CTLFLAG_RWTUN,
    &ionic_rx_process_limit, 0, "Rx can process maximum number of descriptors.");

u32 ionic_tx_coalesce_usecs = 64;
TUNABLE_INT("hw.ionic.tx_coalesce_usecs", &ionic_tx_coalesce_usecs);
SYSCTL_INT(_hw_ionic, OID_AUTO, tx_coalesce_usecs, CTLFLAG_RWTUN,
    &ionic_tx_coalesce_usecs, 0, "Tx coal in usescs.");

u32 ionic_rx_coalesce_usecs = 64;
TUNABLE_INT("hw.ionic.rx_coalesce_usecs", &ionic_rx_coalesce_usecs);
SYSCTL_INT(_hw_ionic, OID_AUTO, rx_coalesce_usecs, CTLFLAG_RWTUN,
    &ionic_rx_coalesce_usecs, 0, "Rx coal in usescs.");

/* XXX: To deal with TSO/SG list bug. */
u32 ionic_max_sg = 0;
TUNABLE_INT("hw.ionic.max_sg", &ionic_max_sg);
SYSCTL_INT(_hw_ionic, OID_AUTO, max_sg, CTLFLAG_RWTUN,
    &ionic_max_sg, 0, "Maximum number of scatter gather list");

static void ionic_dump_mbuf(struct mbuf* m)
{
	IONIC_INFO("len %u\n", m->m_len);

	IONIC_INFO(
		  "data %02hhx %02hhx %02hhx %02hhx %02hhx %02hhx %02hhx %02hhx\n",
		  m->m_data[0], m->m_data[1],
		  m->m_data[2], m->m_data[3],
		  m->m_data[4], m->m_data[5],
		  m->m_data[6], m->m_data[7]);
	IONIC_INFO(
		  "data end %02hhx %02hhx %02hhx %02hhx %02hhx %02hhx %02hhx %02hhx\n",
		  m->m_data[m->m_len - 8], m->m_data[m->m_len - 7],
		  m->m_data[m->m_len - 6], m->m_data[m->m_len - 5],
		  m->m_data[m->m_len - 4], m->m_data[m->m_len - 3],
		  m->m_data[m->m_len - 2], m->m_data[m->m_len - 1]);
}

/* Update mbuf with correct checksum etc. */
static void ionic_rx_checksum(struct mbuf *m, struct rxq_comp *comp, struct rx_stats *stats)
{

	m->m_pkthdr.csum_flags = 0;
	if (comp->csum_ip_ok) {
		m->m_pkthdr.csum_flags = CSUM_IP_CHECKED | CSUM_IP_VALID;
		stats->csum_ip_ok++;
		m->m_pkthdr.csum_data = htons(0xffff);
	}

	if (m->m_pkthdr.csum_flags && (comp->csum_tcp_ok || comp->csum_udp_ok)) {
		m->m_pkthdr.csum_flags |= CSUM_DATA_VALID | CSUM_PSEUDO_HDR;
		stats->csum_l4_ok++;
	}

	if (comp->csum_ip_bad) {
		stats->csum_ip_bad++;
	}

	if (comp->csum_tcp_bad || comp->csum_udp_bad) {
		stats->csum_l4_bad++;
	}
}

/*
 * Set RSS packet type.
 * NOTE: Use h/w logic to determine if packet is TCP type to enable
 * LRO etc.
 */
static bool ionic_rx_rss(struct mbuf *m, struct rxq_comp *comp, int qnum, 
	struct rx_stats *stats)
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

void ionic_rx_input(struct rxque *rxq, struct ionic_rx_buf *rxbuf,
	struct rxq_comp *comp, 	struct rxq_desc *desc)
{
	struct mbuf *m = rxbuf->m;
	struct rx_stats *stats = &rxq->stats;
	struct ifnet *ifp = rxq->lif->netdev;
	int error;
	bool is_tcp;

	KASSERT(IONIC_RX_LOCK_OWNED(rxq), ("%s is not locked", rxq->name));

	/*
	 * No-op case.
	 */
	if (m == NULL) {
		stats->mem_err++;
		return;
	}

	IONIC_RX_TRACE(rxq, "input called for @%d\n", comp->comp_index);
	if (comp->status) {
		IONIC_QUE_WARN(rxq, "RX Status %d\n", comp->status);
		stats->comp_err++;
		m_freem(m);
		return;
	}

#ifdef HAPS
	if (comp->len > ETHER_MAX_FRAME(rxq->lif->netdev, ETHERTYPE_VLAN, 1)) {
		IONIC_QUE_WARN(rxq, "RX PKT TOO LARGE!  comp->len %d\n", comp->len);
		m_freem(m);
	
		return;
	}
#endif

	stats->pkts++;
	stats->bytes += comp->len;

	bus_dmamap_sync(rxq->buf_tag, rxbuf->dma_map, BUS_DMASYNC_POSTREAD);

	prefetch(m->data - NET_IP_ALIGN);
	m->m_pkthdr.rcvif = rxq->lif->netdev;
	m->m_pkthdr.len = comp->len;
	m->m_len = comp->len;

	/* Update the checksum if h/w has calculated. */
	if (ifp->if_capenable & (IFCAP_RXCSUM | IFCAP_RXCSUM_IPV6))
		ionic_rx_checksum(m, comp, stats);

	/* Populate mbuf with h/w RSS hash, type etc. */
	is_tcp = ionic_rx_rss(m, comp, rxq->index, stats);

	if (comp->V) {
		m->m_pkthdr.ether_vtag = le16toh(comp->vlan_tci);
		m->m_flags |= M_VLANTAG;
	}
	/*
	 * Use h/w RSS engine of L4 checksum to determine if its TCP packet.
	 * XXX: add more cases of LRO.
	 * XXX: LRO with VLAN??
	 */
	if ((ifp->if_capenable & IFCAP_LRO) && (is_tcp || comp->csum_tcp_ok)) {
		if (rxq->lro.lro_cnt != 0) {
			if ((error = tcp_lro_rx(&rxq->lro, m, 0)) == 0) {/* third arg -Checksum?? */
				rxbuf->m = NULL;
				return;
			} else {
				IONIC_RX_TRACE(rxq, "lro failed, error: %d\n", error);
			}
		}
	}

#ifdef IONIC_DEBUG_PKT
	IONIC_QUE_INFO(rxq, "len %u\n", m->m_len);

	IONIC_QUE_INFO(rxq,
		  "data %02hhx %02hhx %02hhx %02hhx %02hhx %02hhx %02hhx %02hhx\n",
		  m->m_data[0], m->m_data[1],
		  m->m_data[2], m->m_data[3],
		  m->m_data[4], m->m_data[5],
		  m->m_data[6], m->m_data[7]);
	IONIC_QUE_INFO(rxq,
		  "data end %02hhx %02hhx %02hhx %02hhx %02hhx %02hhx %02hhx %02hhx\n",
		  m->m_data[m->m_len - 8], m->m_data[m->m_len - 7],
		  m->m_data[m->m_len - 6], m->m_data[m->m_len - 5],
		  m->m_data[m->m_len - 4], m->m_data[m->m_len - 3],
		  m->m_data[m->m_len - 2], m->m_data[m->m_len - 1]);
#endif

	/* Send the packet to stack. */
	rxbuf->m = NULL;
	IONIC_RX_TRACE(rxq, "input done for @%d\n", comp->comp_index);
	rxq->lif->netdev->if_input(rxq->lif->netdev, m);
}

int ionic_rx_mbuf_alloc(struct rxque *rxq, int index, int len)
{
	bus_dma_segment_t  seg[1];
	struct ionic_rx_buf *rxbuf;
	struct mbuf *m;
	struct rx_stats *stats = &rxq->stats;
	int nsegs, error;

	KASSERT(IONIC_RX_LOCK_OWNED(rxq), ("%s is not locked", rxq->name));
	rxbuf = &rxq->rxbuf[index];
	m = m_getjcl(M_NOWAIT, MT_DATA, M_PKTHDR, len);
	if (m == NULL) {
		rxbuf->m = NULL;
		stats->alloc_err++;
		return (ENOMEM);
	}

	m->m_pkthdr.len = m->m_len = len;
	rxbuf->m = m;

	error = bus_dmamap_load_mbuf_sg(rxq->buf_tag, rxbuf->dma_map, m, seg, &nsegs, BUS_DMA_NOWAIT);
	if (error) {
		stats->dma_map_err++;
		return (error);
	}

	bus_dmamap_sync(rxq->buf_tag, rxbuf->dma_map, BUS_DMASYNC_PREREAD);
	rxbuf->pa_addr = seg[0].ds_addr;
	rxq->stats.mbuf_alloc++;

	return (0);
}

void ionic_rx_mbuf_free(struct rxque *rxq, struct ionic_rx_buf *rxbuf)
{
	rxq->stats.mbuf_free++;
	bus_dmamap_sync(rxq->buf_tag, rxbuf->dma_map, BUS_DMASYNC_POSTREAD);
	bus_dmamap_unload(rxq->buf_tag, rxbuf->dma_map);

	m_freem(rxbuf->m);
	rxbuf->m = NULL;
}

void ionic_rx_destroy_map(struct rxque *rxq, struct ionic_rx_buf *rxbuf)
{
	bus_dmamap_destroy(rxq->buf_tag, rxbuf->dma_map);
	rxbuf->dma_map = NULL;
}

static irqreturn_t ionic_rx_isr(int irq, void *data)
{
	struct rxque* rxq = data;
#ifdef IONIC_SEPERATE_TX_INTR
	struct txque* txq = rxq->lif->txqs[rxq->index];
#endif
	struct ifnet *ifp = rxq->lif->netdev;
	struct rx_stats* rxstats = &rxq->stats;
	uint64_t status;
	int work_done = 0;

	KASSERT(rxq, ("rxq is NULL"));
	KASSERT(ifp, ("%s ifp == NULL", rxq->name));
	KASSERT((rxq->intr.index != INTR_INDEX_NOT_ASSIGNED),
			("%s has no interrupt resource", rxq->name));
	
	status = *(uint64_t *)rxq->lif->ionic->idev.intr_status;

	/* Protect against spurious interrupts */
	if ((ifp->if_drv_flags & IFF_DRV_RUNNING) == 0)
		return (IRQ_NONE);

	IONIC_RX_LOCK(rxq);

	IONIC_RX_TRACE(rxq, "[%ld]comp index: %d head: %d tail: %d\n",
		rxstats->isr_count, rxq->comp_index, rxq->head_index, rxq->tail_index);
	
	ionic_intr_mask(&rxq->intr, true);

	rxstats->isr_count++;

	work_done = ionic_rx_clean(rxq, ionic_rx_process_limit);
	IONIC_RX_TRACE(rxq, "processed: %d packets, h/w credits: %d\n",
		work_done, rxq->intr.ctrl->int_credits);

	ionic_intr_return_credits(&rxq->intr, work_done, 0, false);
	IONIC_RX_UNLOCK(rxq);
	
	taskqueue_enqueue(rxq->taskq, &rxq->task);

#ifdef IONIC_SEPERATE_TX_INTR
	IONIC_TX_LOCK(txq);
	work_done = ionic_tx_clean(txq, ionic_tx_clean_threshold);
	IONIC_TX_TRACE(txq, "processed: %d packets\n", work_done);
	IONIC_TX_UNLOCK(txq);
#endif

	return (IRQ_HANDLED);
}


static void
ionic_rx_task_handler(void *arg, int pendindg)
{
	struct rxque* rxq = arg;
#ifndef IONIC_SEPERATE_TX_INTR
	struct txque* txq = rxq->lif->txqs[rxq->index];
	int err;
#endif
	int work_done;

	KASSERT(rxq, ("task handler called with rxq == NULL"));

	IONIC_RX_LOCK(rxq);

	rxq->stats.task++;
	IONIC_RX_TRACE(rxq, "comp index: %d head: %d tail :%d\n",
		rxq->comp_index, rxq->head_index, rxq->tail_index);

	/* 
	 * Process all Rx frames.
	 */
	work_done = ionic_rx_clean(rxq, rxq->num_descs);
	IONIC_RX_TRACE(rxq, "processed %d packets\n", work_done);

	ionic_intr_return_credits(&rxq->intr, work_done, 0, true);
	ionic_intr_mask(&rxq->intr, false);

	IONIC_RX_UNLOCK(rxq);

#ifndef IONIC_SEPERATE_TX_INTR
	IONIC_TX_LOCK(txq);
	err = ionic_start_xmit_locked(txq->lif->netdev, txq);
	work_done = ionic_tx_clean(txq, ionic_tx_clean_threshold);
	IONIC_TX_TRACE(txq, "processed %d packets\n", work_done);
	IONIC_TX_UNLOCK(txq);
#endif
}

int ionic_setup_rx_intr(struct rxque* rxq)
{
	int err, bind_cpu;
	struct lif* lif = rxq->lif;
#ifdef RSS
	cpuset_t        cpu_mask;

	bind_cpu = rss_getcpu(rxq->index % rss_getnumbuckets());
	CPU_SETOF(bind_cpu, &cpu_mask);
#else
	bind_cpu = rxq->index;
#endif

	TASK_INIT(&rxq->task, 0, ionic_rx_task_handler, rxq);
    rxq->taskq = taskqueue_create_fast(rxq->intr.name, M_NOWAIT,
	    taskqueue_thread_enqueue, &rxq->taskq);

#ifdef RSS
    err = taskqueue_start_threads_cpuset(&rxq->taskq, 1, PI_NET, &cpu_mask,
	        "%s (que %s)", device_get_nameunit(lif->ionic->dev), rxq->intr.name);
#else
    err = taskqueue_start_threads(&rxq->taskq, 1, PI_NET, NULL,
		"%s (que %s)", device_get_nameunit(lif->ionic->dev), rxq->intr.name);
#endif

	if (err) {
		IONIC_QUE_ERROR(rxq, "failed to create task queue, error: %d\n",
			err);
		taskqueue_free(rxq->taskq);
		return (err);
	}

	/* Legacy interrupt allocation is done once. */
	if (ionic_enable_msix == 0)
		return (0);

	err = request_irq(rxq->intr.vector, ionic_rx_isr, 0, rxq->intr.name, rxq);
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
static bool ionic_tx_avail(struct txque *txq, int want)
{	
	int avail;

	avail = ionic_desc_avail(txq->num_descs, txq->head_index, txq->tail_index);

	return (avail > want);
}

#ifdef IONIC_SEPERATE_TX_INTR
static irqreturn_t ionic_tx_isr(int irq, void *data)
{
	struct txque* txq = data;
	int work_done = 0;
	struct ifnet *ifp = txq->lif->netdev;
	uint64_t status;

	KASSERT(txq, ("txq is NULL"));
	KASSERT((txq->intr.index != INTR_INDEX_NOT_ASSIGNED),
			("%s has no interrupt resource", txq->name));

	if ((ifp->if_drv_flags & IFF_DRV_RUNNING) == 0)
		return (IRQ_NONE);

	IONIC_TX_TRACE(txq, "Enter: head: %d tail :%d\n",
		txq->head_index, txq->tail_index);

	status = *(uint64_t *)txq->lif->ionic->idev.intr_status;

	IONIC_TX_LOCK(txq);

	ionic_intr_mask(&txq->intr, true);
 
	work_done = ionic_tx_clean(txq, txq->num_descs);
	IONIC_TX_TRACE(txq, "processed %d descriptors h/w credits:%d\n", work_done,
		txq->intr.ctrl->int_credits);
	ionic_intr_return_credits(&txq->intr, work_done, 0, false);

	taskqueue_enqueue(txq->taskq, &txq->task);
	IONIC_TX_UNLOCK(txq);

	IONIC_TX_TRACE(txq, "Exit: head: %d tail :%d\n",
		txq->head_index, txq->tail_index);

	return (IRQ_HANDLED);
}

static void
ionic_tx_task_handler(void *arg, int pendindg)
{
	struct txque* txq = arg;
	int err, work_done;

	KASSERT(txq, ("task handler called with txq == NULL"));

	IONIC_TX_TRACE(txq, "Enter: head: %d tail :%d\n",
		txq->head_index, txq->tail_index);

	IONIC_TX_LOCK(txq);

	work_done = ionic_tx_clean(txq, ionic_tx_clean_threshold);
	IONIC_TX_TRACE(txq, "processed %d descriptors h/w credits: %d\n", work_done,
		txq->intr.ctrl->int_credits);

	ionic_intr_return_credits(&txq->intr, work_done, 0, true);

	err = ionic_start_xmit_locked(txq->lif->netdev, txq);
	ionic_intr_mask(&txq->intr, false);
	
	IONIC_TX_UNLOCK(txq);
	IONIC_TX_TRACE(txq, "Exit: head: %d tail :%d\n",
		txq->head_index, txq->tail_index);
}

int ionic_setup_tx_intr(struct txque* txq)
{
	int err, bind_cpu;
	struct lif* lif = txq->lif;
#ifdef RSS
	cpuset_t        cpu_mask;

	bind_cpu = rss_getcpu(txq->index % rss_getnumbuckets());
	CPU_SETOF(bind_cpu, &cpu_mask);
#else
	bind_cpu = txq->index;
#endif

	TASK_INIT(&txq->task, 0, ionic_tx_task_handler, txq);
    txq->taskq = taskqueue_create_fast(txq->name, M_NOWAIT,
	    taskqueue_thread_enqueue, &txq->taskq);

#ifdef RSS
    err = taskqueue_start_threads_cpuset(&txq->taskq, 1, PI_NET, &cpu_mask,
	    "%s:%s [cpu %d]", device_get_nameunit(lif->ionic->dev), txq->name, bind_cpu);
#else
	err = taskqueue_start_threads(&txq->taskq, 1, PI_NET,
	    "%s (que %d)", device_get_nameunit(lif->ionic->dev), txq->index);
#endif

	if (err) {
		IONIC_QUE_ERROR(txq, "failed to create task queue, error: %d\n", err);
		taskqueue_free(txq->taskq);
		return (err);
	}

	if(ionic_enable_msix == 0)
		return (0);

	request_irq(txq->intr.vector, ionic_tx_isr, 0, txq->intr.name, txq);

	err = bind_irq_to_cpu(txq->intr.vector, bind_cpu);
	if (err) {
		IONIC_QUE_WARN(txq, "failed to bind to cpu%d\n", bind_cpu);
	}
	IONIC_QUE_INFO(txq, "bound to cpu%d\n", bind_cpu);
	
	return (0);
}
#endif

static irqreturn_t ionic_legacy_isr(int irq, void *data)
{
	struct lif* lif = data;
	struct ifnet *ifp;
	struct txque *txq;
	struct rxque *rxq;
	uint64_t status;
	int work_done, i;

	ifp = lif->netdev;
	status = *(uint64_t *)lif->ionic->idev.intr_status;

	IONIC_NETDEV_INFO(lif->netdev, "legacy INTR status(%p): 0x%lx\n",
		lif->ionic->idev.intr_status, status);

	if (status == 0) {
		/* Invoked for no reason. */
		lif->spurious++;
		return (IRQ_NONE);
	}

	if ((ifp->if_drv_flags & IFF_DRV_RUNNING) == 0)
		return (IRQ_NONE);

	for (i = 0; i < lif->nrxqs ; i++) {
		rxq = lif->rxqs[i];

		KASSERT((rxq->intr.index != INTR_INDEX_NOT_ASSIGNED),
			("%s has no interrupt resource", rxq->name));
		IONIC_QUE_INFO(rxq, "Interrupt source index:%d credits:%d\n",
			rxq->intr.index, rxq->intr.ctrl->int_credits);

		if ((status & (1 << rxq->intr.index)) == 0)
			continue;

		IONIC_RX_LOCK(rxq);
		ionic_intr_mask(&rxq->intr, true);
		work_done = ionic_rx_clean(rxq, rxq->num_descs);
		ionic_intr_return_credits(&rxq->intr, work_done, 0, true);
		ionic_intr_mask(&rxq->intr, false);
		IONIC_RX_UNLOCK(rxq);
	}

	for (i = 0; i < lif->ntxqs ; i++) {
		txq = lif->txqs[i];
		
		KASSERT((txq->intr.index != INTR_INDEX_NOT_ASSIGNED),
			("%s has no interrupt resource", txq->name));
		IONIC_QUE_INFO(txq, "Interrupt source index: %d credits: %d\n",
			txq->intr.index, txq->intr.ctrl->int_credits);
		
		if ((status & (1 << txq->intr.index)) == 0)
			continue;

		IONIC_TX_LOCK(txq);
		ionic_intr_mask(&txq->intr, true);
		work_done = ionic_tx_clean(txq, txq->num_descs);
		ionic_intr_return_credits(&txq->intr, work_done, 0, true);
		ionic_intr_mask(&txq->intr, false);
		IONIC_TX_UNLOCK(txq);
	}

	return (IRQ_HANDLED);
}

/*
 * Setup legacy interrupt.
 */
int ionic_setup_legacy_intr(struct lif* lif)
{
	int error = 0;

	error = request_irq(lif->ionic->pdev->irq, ionic_legacy_isr, 0, "legacy", lif);

	return (error);
}

static int
ionic_get_header_size(struct txque *txq, struct mbuf *mb, uint16_t *eth_type, 
	int *proto, int *hlen)
{
	struct ether_vlan_header *eh;
	struct tcphdr *th;
	struct ip *ip;
	int ip_hlen, tcp_hlen;
	struct ip6_hdr *ip6;
	struct tx_stats *stats = &txq->stats;
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

/* XXX: Not required. */
static int ionic_load_mbuf_sg(struct txque *txq, bus_dmamap_t dma_map, 
	struct mbuf **m, bus_dma_segment_t  *seg, int *nsegs)
{
	int error, max_sg;
	struct mbuf* newm;
	struct tx_stats *stats = &txq->stats;
 	bus_dma_tag_t buf_tag = txq->buf_tag;

	error = bus_dmamap_load_mbuf_sg(buf_tag, dma_map, *m, seg, nsegs, BUS_DMA_NOWAIT);
	if (error) {
		IONIC_QUE_ERROR(txq, "failed to dma map, error: %d\n", error);
		stats->dma_map_err++;
		return (error);
	}

	max_sg = IONIC_TX_MAX_SG_ELEMS + 1;
	max_sg = ionic_max_sg ? min(max_sg, ionic_max_sg) : max_sg;

	if (*nsegs <= max_sg)
		return (0);

	stats->linearize++;

	/* XXX: use m_collapse for sg > 1 */
	newm = m_defrag(*m, M_NOWAIT);
	if (newm == NULL) {
		IONIC_QUE_ERROR(txq, "failed to defrag\n");
		stats->linearize_err++;
		return (ENOMEM);
	}

	/* Old mbuf was released by collapse. */
	*m = newm;
	
	error = bus_dmamap_load_mbuf_sg(buf_tag, dma_map, *m, seg, nsegs, BUS_DMA_NOWAIT);
	if (error) {
		IONIC_QUE_ERROR(txq, "failed to dma map, error: %d\n", error);
		stats->dma_map_err++;
		return (error);
	}

	KASSERT((*nsegs <= max_sg), ("%s segment more than %d > %d", txq->name, *nsegs, max_sg));

	return (0);
}

static int ionic_tx_setup(struct txque *txq, struct mbuf *m)
{
	struct txq_desc *desc;
	struct txq_sg_desc *sg;
	struct tx_stats *stats = &txq->stats;
	struct ionic_tx_buf *txbuf;
	bool offload = false;
	int i, error, index, nsegs;
	bus_dma_segment_t  seg[IONIC_TX_MAX_SG_ELEMS + 1]; /* Extra for the first segment. */

	IONIC_TX_TRACE(txq, "Enter head: %d tail: %d\n",
		 txq->head_index, txq->tail_index);

	index = txq->head_index;
	desc = &txq->cmd_ring[index];
	txbuf = &txq->txbuf[index];
	sg = &txq->sg_ring[index];

	stats->pkts++;
	stats->bytes += m->m_len;

	error = ionic_load_mbuf_sg(txq, txbuf->dma_map, &m, seg, &nsegs);
	if (error) {
		IONIC_QUE_ERROR(txq, "setting up dma map failed, error: %d\n", error);
		return (error);
	}

	if (!ionic_tx_avail(txq, nsegs)) {
		stats->no_descs++;
		bus_dmamap_unload(txq->buf_tag, txbuf->dma_map);
		IONIC_TX_TRACE(txq, "No space available, head: %u tail: %u nsegs :%d\n",
			txq->head_index, txq->tail_index, nsegs);
		return (ENOBUFS);
	}

	bus_dmamap_sync(txq->buf_tag, txbuf->dma_map, BUS_DMASYNC_PREWRITE);
	txbuf->pa_addr = seg[0].ds_addr;
	txbuf->m = m;
	txbuf->timestamp = rdtsc();
	IONIC_TX_TRACE(txq, "VA: %p DMA addr: 0x%lx nsegs: %d length: 0x%lx\n",
		m, txbuf->pa_addr, nsegs, seg[0].ds_len);

	if (m->m_pkthdr.csum_flags & CSUM_IP) {
		desc->l3_csum = 1;
		offload = true;
	}

	if (m->m_pkthdr.csum_flags & 
		(CSUM_IP | CSUM_TCP | CSUM_UDP | CSUM_UDP_IPV6 | CSUM_TCP_IPV6)) {
		desc->l4_csum = 1;
		offload = true;
	}

	desc->opcode = offload ?
		TXQ_DESC_OPCODE_CALC_CSUM_TCPUDP : TXQ_DESC_OPCODE_CALC_NO_CSUM;
	desc->len = seg[0].ds_len;
	desc->addr = txbuf->pa_addr;
	desc->C = 1;
	desc->csum_offset = 0;
	desc->hdr_len = 0;
	desc->num_sg_elems = nsegs - 1; /* First one is header. */
	desc->O = 0;
	if (m->m_flags & M_VLANTAG) {
			desc->V = 1;
			desc->vlan_tci = htole16(m->m_pkthdr.ether_vtag);;
	}

	/* Populate sg list with rest of segments. */
	for (i = 0 ; i < nsegs - 1 ; i++) {
		sg->elems[i].addr = seg[i + 1].ds_addr;
		sg->elems[i].len = seg[i + 1].ds_len;
	}

	if (offload)
		stats->csum_offload++;
	else
		stats->no_csum_offload++;

	txq->head_index = (txq->head_index + 1) % txq->num_descs;

	/* XXX ping doorbell on 4 rx submission. */
	ionic_tx_ring_doorbell(txq, txq->head_index);

	return 0;
}

#ifdef IONIC_TSO_DEBUG
/*
 * Validate the TSO descriptors.
 */
static void ionic_tx_tso_dump(struct txque *txq, struct mbuf *m,
	bus_dma_segment_t  *seg, int nsegs, int stop_index)
{
	struct txq_desc *desc;
	struct txq_sg_desc *sg;
	struct ionic_tx_buf *txbuf;
	int i, j, len = 0;

	IONIC_TX_TRACE(txq, "TSO: VA: %p nsegs: %d length: %d\n",
		m, nsegs, m->m_pkthdr.len);
	
	for ( i = 0 ; i < nsegs ; i++) {
		IONIC_TX_TRACE(txq, "seg[%d] pa: 0x%lx len:%ld\n",
			i, seg[i].ds_addr, seg[i].ds_len);
	}

	for ( i = txq->head_index ; i < stop_index; i++ ) {
		txbuf = &txq->txbuf[i];
		desc = &txq->cmd_ring[i];
		sg = &txq->sg_ring[i];
		len += desc->len;
		IONIC_TX_TRACE(txq, "TSO Dump desc[%d] pa: 0x%lx length: %d"
			" S:%d E:%d C:%d mss:%d hdr_len:%d mbuf:%p\n",
			i, desc->addr, desc->len, desc->S, desc->E, 
			desc->C, desc->mss, desc->hdr_len, txbuf->m);

		for ( j = 0; j < desc->num_sg_elems; j++ ) {
			len += sg->elems[j].len;
			IONIC_TX_TRACE(txq, "sg[%d] pa: 0x%lx length: %d\n",
				j, sg->elems[j].addr, sg->elems[j].len);
		}

		KASSERT((i == txq->head_index) == desc->S, ("TSO w/o start of frame"));
		KASSERT((i == (stop_index - 1)) == desc->E, ("TSO w/o end of frame"));
	}

	KASSERT(len == m->m_pkthdr.len,
		("TSO packet size mismatch len: %d != actual %d",
		m->m_pkthdr.len, len));
}
#endif

static int ionic_tx_tso_setup(struct txque *txq, struct mbuf *m)
{
	uint32_t mss = m->m_pkthdr.tso_segsz;
	struct tx_stats *stats = &txq->stats;
	struct ifnet* ifp = txq->lif->netdev;
	struct ionic_tx_buf *first_txbuf, *txbuf;
	struct txq_desc *desc;
	struct txq_sg_desc *sg;
	uint16_t eth_type;
	int i, j, index, hdr_len, proto, error, nsegs;
	uint32_t frag_offset, desc_len, remain_len, frag_remain_len, desc_max_size;
	bus_dma_segment_t  seg[IONIC_MAX_TSO_SEG];

	IONIC_TX_TRACE(txq, "Enter head: %d tail: %d\n",
		txq->head_index, txq->tail_index);

	if ((error = ionic_get_header_size(txq, m, &eth_type, &proto, &hdr_len))) {
		IONIC_NETDEV_ERROR(ifp, "mbuf packet discarded, type: %x proto: %x"
			" hdr_len :%u\n", eth_type, proto, hdr_len);
		ionic_dump_mbuf(m);
		stats->bad_ethtype++;
		return (error);
	}

	if (proto != IPPROTO_TCP) {
		return (EINVAL);
	}

	index = txq->head_index;
	first_txbuf = &txq->txbuf[index];

	error = bus_dmamap_load_mbuf_sg(txq->buf_tag, first_txbuf->dma_map, m,
		seg, &nsegs, BUS_DMA_NOWAIT);
	if (error || (nsegs <= 0)) {
		IONIC_QUE_ERROR(txq, "failed to map TSO xmit, error: %d\n", error);
		stats->dma_map_err++;
		return (error);
	}

	if (!ionic_tx_avail(txq, nsegs)) {
		stats->no_descs++;
		IONIC_TX_TRACE(txq, "No space available, head: %u tail: %u nsegs :%d\n",
			txq->head_index, txq->tail_index, nsegs);
		return (ENOBUFS);
	}

	bus_dmamap_sync(txq->buf_tag, first_txbuf->dma_map, BUS_DMASYNC_PREWRITE);
	stats->tso_max_size = max(stats->tso_max_size, m->m_pkthdr.len);

	remain_len = m->m_pkthdr.len;	
	index = txq->head_index;
	frag_offset = 0;
	frag_remain_len = seg[0].ds_len;

	/* 
	 * Loop through all segments of mbuf and create mss size descriptors.
	 * First descriptor points to header.
	 */
	for ( i = 0 ; i < nsegs  && remain_len > 0; )
	{
		desc = &txq->cmd_ring[index];
		txbuf = &txq->txbuf[index];
		sg = &txq->sg_ring[index];
		
		desc->opcode = TXQ_DESC_OPCODE_TSO;
		desc->S = (i == 0) ? 1 : 0;

		desc->hdr_len = desc->S ? hdr_len : 0;
		desc_max_size = desc->S ? (mss + hdr_len) : mss;

		desc_len = min(frag_remain_len, desc_max_size);
		desc->len = desc_len;
		desc->mss = mss;
		desc->addr = seg[i].ds_addr + frag_offset;
		desc->C = 1;

		if (m->m_flags & M_VLANTAG) {
			desc->V = 1;
			desc->vlan_tci = htole16(m->m_pkthdr.ether_vtag);;
		}

		desc->E = (remain_len <= desc_max_size) ? 1 : 0;

		/* Tx completion will use the last descriptor. */
		if (desc->E) {
			txbuf->pa_addr = desc->addr;
			txbuf->m = m;
			txbuf->dma_map = first_txbuf->dma_map;
			txbuf->timestamp = rdtsc();
		} else {
			txbuf->m = NULL;
			txbuf->pa_addr = 0;
		}

		frag_remain_len -= desc_len;

		/* Check if anything left to transmit from this segment. */
		if (frag_remain_len <= 0) {
			i++;
			frag_remain_len = seg[i].ds_len;
			frag_offset = 0;
		} else {
			frag_offset += desc_len;
		}

		/* 
		 * Now populate SG list, with the remaining fragments upto MSS size.
		 */
		for ( j = 0 ; j < IONIC_TX_MAX_SG_ELEMS && (i < nsegs) && desc_len < desc_max_size; j++) {
			sg->elems[j].addr = seg[i].ds_addr + frag_offset;
			sg->elems[j].len = min(frag_remain_len, (desc_max_size - desc_len));
			frag_remain_len -= sg->elems[j].len;
			frag_offset += sg->elems[j].len;
			desc_len += sg->elems[j].len;

			/* End of fragment, jump to next one */
			if (frag_remain_len <= 0) {
				i++;
				frag_offset = 0;
				frag_remain_len = seg[i].ds_len;
			}
		}

		desc->num_sg_elems = j;
		remain_len -= desc_len;

		stats->tso_max_sg = max(stats->tso_max_sg, j);
		stats->pkts++;
		stats->bytes += desc_len;

		index = (index + 1) % txq->num_descs;
	}

	KASSERT(desc->E, ("No end of frame"));
#ifdef IONIC_TSO_DEBUG
	ionic_tx_tso_dump(txq, m, seg, nsegs, index);
#endif

	/* 
	 * Completion will be generated for the last TSO descriptor.
	 * XXX: optimization: ring the doorbell in-between creating descriptors.
	 */ 
	txq->head_index = (index ) % txq->num_descs;
	ionic_tx_ring_doorbell(txq, txq->head_index);
	
	IONIC_TX_TRACE(txq, "Exit head: %d tail: %d\n", txq->head_index, txq->tail_index);

	return 0;
}


static int ionic_xmit(struct ifnet* ifp, struct txque* txq, struct mbuf **head)
{
	struct tx_stats *stats;
	int err;
	struct mbuf* m;

	if ((ifp->if_drv_flags & IFF_DRV_RUNNING) == 0)
	    return (ENETDOWN);

	stats = &txq->stats;
	m = *head;

	/* Send a copy of the frame to the BPF listener */
	ETHER_BPF_MTAP(ifp, m);

	if (m->m_pkthdr.csum_flags & CSUM_TSO)
		err = ionic_tx_tso_setup(txq, m);
	else
		err = ionic_tx_setup(txq, m);

	if (err) {
		stats->retry++;
		return (err);
	}

	return 0;
}

int
ionic_start_xmit_locked(struct ifnet* ifp, 	struct txque* txq)
{
	struct mbuf *m;
	int err, work_done;

	work_done = ionic_tx_clean(txq, txq->num_descs);
#ifdef IONIC_SEPERATE_TX_INTR
	ionic_intr_return_credits(&txq->intr, work_done, 0, false);
#endif
	while ((m = drbr_peek(ifp, txq->br)) != NULL) {
		if ((err = ionic_xmit(ifp, txq, &m)) != 0) {
			if (err) {
				drbr_putback(ifp, txq->br, m);
			}
			break;
		}
		drbr_advance(ifp, txq->br);
	}

	return (err);
}

int
ionic_start_xmit(struct net_device *netdev, struct mbuf *m)
{
	struct lif *lif = netdev_priv(netdev);
	struct ifnet* ifp = lif->netdev;
	struct txque* txq; 
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

	err = drbr_enqueue(ifp, txq->br, m);
	if (err)
		return (err);

	if (IONIC_TX_TRYLOCK(txq)) {
		ionic_start_xmit_locked(ifp, txq);
		IONIC_TX_UNLOCK(txq);
	} else {
		if (txq->taskq)
			taskqueue_enqueue(txq->taskq, &txq->task);
	}

	return (0);
}

static uint64_t
ionic_get_counter(struct ifnet *ifp, ift_counter cnt)
{
	struct lif* lif = if_getsoftc(ifp);
	uint64_t val = 0;
	int i;

	struct tx_stats* txstat;
	struct rx_stats* rxstat;

	switch (cnt) {
	case IFCOUNTER_IPACKETS:
		for ( i = 0 ; i < lif->nrxqs; i++) {
			rxstat = &lif->rxqs[i]->stats;
			val += rxstat->pkts;
		}
		return (val);

	case IFCOUNTER_OPACKETS:
		for ( i = 0 ; i < lif->ntxqs; i++) {
			txstat = &lif->txqs[i]->stats;
			val += txstat->pkts;
		}
		return (val);

	case IFCOUNTER_IBYTES:
		for ( i = 0 ; i < lif->nrxqs; i++) {
			rxstat = &lif->rxqs[i]->stats;
			val += rxstat->bytes;
		}
		return (val);

	case IFCOUNTER_OBYTES:		
		for ( i = 0 ; i < lif->ntxqs; i++) {
			txstat = &lif->txqs[i]->stats;
			val += txstat->bytes;
		}
		return (val);

#ifdef notyet
	case IFCOUNTER_IMCASTS:
		return (ionic->imcasts);
	case IFCOUNTER_OMCASTS:
		return (ionic->omcasts);
	case IFCOUNTER_COLLISIONS:
		return (0);

	case IFCOUNTER_IQDROPS:
		for ( i = 0 ; i < lif->nrxqs; i++) {
			rxstat = &lif->rxqs[i]->stats;
			val += rxstat->bytes;
		}
		return (val);
		return (rxstat->drop);
#endif
	case IFCOUNTER_OQDROPS:
		for ( i = 0 ; i < lif->ntxqs; i++) {
			txstat = &lif->txqs[i]->stats;
			val += lif->txqs[i]->br->br_drops;
		}
		return (val);

#ifdef notyet
	case IFCOUNTER_IERRORS:
		return (ionic->ierrors);
#endif
	default:
		return (if_get_counter_default(ifp, cnt));
	}
}

static void 
ionic_tx_qflush(struct ifnet *ifp)
{
	struct lif *lif = ifp->if_softc;
	struct txque *txq;
	struct mbuf *m;
	unsigned int i;

	for (i = 0; i < lif->ntxqs; i++) {
		txq = lif->txqs[i];
		IONIC_TX_LOCK(txq);
        while ((m = buf_ring_dequeue_sc(txq->br)) != NULL)
                m_freem(m);
		IONIC_TX_UNLOCK(txq);
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
	ifp->if_qflush = ionic_tx_qflush;
	ifp->if_snd.ifq_maxlen = ndescs;
	if_setgetcounterfn(ifp, ionic_get_counter);
	/* Capabilities are set later on. */

	/* Connect lif to ifnet. */
	lif->netdev = ifp;

	IONIC_CORE_LOCK_INIT(lif);

	return (0);
}

static int
ionic_intr_coal_handler(SYSCTL_HANDLER_ARGS)
{
	struct lif* lif = oidp->oid_arg1;
	union identity *ident = lif->ionic->ident;
	u32 tx_coal, rx_coal, coalesce_usecs;
	unsigned int i;
	int error;

	coalesce_usecs = lif->rx_coalesce_usecs;

	IONIC_NETDEV_INFO(lif->netdev, "Enter Intr coal: %d\n", coalesce_usecs);

	error = sysctl_handle_int(oidp, &coalesce_usecs, 0, req);
	if (error || !req->newptr)
		return (EINVAL);

	if (ident->dev.intr_coal_div <= 0) {
		IONIC_NETDEV_ERROR(lif->netdev, "Can't change, divisor is: %d\n",
			ident->dev.intr_coal_div);
		return (EINVAL);
	}

	tx_coal = coalesce_usecs * ident->dev.intr_coal_mult /
		  ident->dev.intr_coal_div;
	rx_coal = coalesce_usecs * ident->dev.intr_coal_mult /
		  ident->dev.intr_coal_div;

	if (tx_coal > INTR_CTRL_COAL_MAX || rx_coal > INTR_CTRL_COAL_MAX)
		return (ERANGE);

#ifdef IONIC_SEPERATE_TX_INTR
	if (coalesce_usecs != lif->tx_coalesce_usecs) {
		lif->tx_coalesce_usecs = coalesce_usecs;
		for (i = 0; i < lif->ntxqs; i++)
			ionic_intr_coal_set(&lif->txqs[i]->intr, tx_coal);
	}
#endif

	if (coalesce_usecs != lif->rx_coalesce_usecs) {
		lif->rx_coalesce_usecs = coalesce_usecs;
		for (i = 0; i < lif->nrxqs; i++)
			ionic_intr_coal_set(&lif->rxqs[i]->intr, rx_coal);
	}

	IONIC_NETDEV_INFO(lif->netdev, "Exit Intr coal: %d\n", coalesce_usecs);

	return (0);
}

static void
ionic_lif_add_rxtstat(struct rxque *rxq, struct sysctl_ctx_list *ctx,
					  struct sysctl_oid_list *queue_list)
{
	struct lro_ctrl *lro = &rxq->lro;
	struct rx_stats *rxstat = &rxq->stats;

	SYSCTL_ADD_UINT(ctx, queue_list, OID_AUTO, "head", CTLFLAG_RD,
					&rxq->head_index, 0, "Head index");
	SYSCTL_ADD_UINT(ctx, queue_list, OID_AUTO, "tail", CTLFLAG_RD,
					&rxq->tail_index, 0, "Tail index");
	SYSCTL_ADD_UINT(ctx, queue_list, OID_AUTO, "comp_index", CTLFLAG_RD,
					&rxq->comp_index, 0, "Completion index");
	SYSCTL_ADD_UINT(ctx, queue_list, OID_AUTO, "num_descs", CTLFLAG_RD,
					&rxq->num_descs, 0, "Number of descriptors");

	SYSCTL_ADD_ULONG(ctx, queue_list, OID_AUTO, "dma_setup_error", CTLFLAG_RD,
					 &rxstat->dma_map_err, "DMA map setup error");
	SYSCTL_ADD_ULONG(ctx, queue_list, OID_AUTO, "alloc_error", CTLFLAG_RD,
					 &rxstat->alloc_err, "Buffer allocation error");
	SYSCTL_ADD_ULONG(ctx, queue_list, OID_AUTO, "comp_err", CTLFLAG_RD,
					 &rxstat->comp_err, "Completion with error");
	SYSCTL_ADD_ULONG(ctx, queue_list, OID_AUTO, "mem_err", CTLFLAG_RD,
					 &rxstat->mem_err, "Completion without mbufs");

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

	SYSCTL_ADD_ULONG(ctx, queue_list, OID_AUTO, "rss_ip6_ex", CTLFLAG_RD,
					 &rxstat->rss_ip6_ex, "RSS IPv6 extension packets");
	SYSCTL_ADD_ULONG(ctx, queue_list, OID_AUTO, "rss_tcp_ip6_ex", CTLFLAG_RD,
					 &rxstat->rss_tcp_ip6_ex, "RSS TCP/IPv6 extension packets");
	SYSCTL_ADD_ULONG(ctx, queue_list, OID_AUTO, "rss_udp_ip6_ex", CTLFLAG_RD,
					 &rxstat->rss_udp_ip6_ex, "RSS UDP/IPv6 extension packets");

	SYSCTL_ADD_ULONG(ctx, queue_list, OID_AUTO, "rss_unknown", CTLFLAG_RD,
					 &rxstat->rss_unknown, "RSS for unknown packets"); // XXX???
}

static void
ionic_lif_add_txtstat(struct txque *txq, struct sysctl_ctx_list *ctx,
					  struct sysctl_oid_list *list)
{
	struct tx_stats *txstat = &txq->stats;

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
	SYSCTL_ADD_ULONG(ctx, list, OID_AUTO, "pkts_retry", CTLFLAG_RD,
					 &txstat->retry, "Packets were retried");
	SYSCTL_ADD_ULONG(ctx, list, OID_AUTO, "no_descs", CTLFLAG_RD,
					 &txstat->no_descs, "Descriptors not available");
	SYSCTL_ADD_ULONG(ctx, list, OID_AUTO, "linearize", CTLFLAG_RD,
					 &txstat->linearize, "Linearize  mbuf");
	SYSCTL_ADD_ULONG(ctx, list, OID_AUTO, "linearize_err", CTLFLAG_RD,
					 &txstat->linearize_err, "Linearize  mbuf failed");
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
	SYSCTL_ADD_ULONG(ctx, list, OID_AUTO, "tso_max_size", CTLFLAG_RD,
					 &txstat->tso_max_size, "TSO maximum packet size");
	/* XXX: remove me . */
	SYSCTL_ADD_ULONG(ctx, list, OID_AUTO, "tso_max_sg", CTLFLAG_RD,
					 &txstat->tso_max_sg, "TSO maximum number of sg");
}

static void
ionic_setup_hw_stats(struct lif *lif, struct sysctl_ctx_list *ctx,
	struct sysctl_oid_list *child)
{
	struct stats_dump *stat = lif->stats_dump;

	if (stat == NULL)
		return;

	SYSCTL_ADD_ULONG(ctx, child, OID_AUTO, "rx_ucast__bytes", CTLFLAG_RD,
       &stat->rx_ucast_bytes, "");
	SYSCTL_ADD_ULONG(ctx, child, OID_AUTO, "rx_ucast_packets", CTLFLAG_RD,
       &stat->rx_ucast_packets, "");
	SYSCTL_ADD_ULONG(ctx, child, OID_AUTO, "rx_mcast_bytes", CTLFLAG_RD,
       &stat->rx_mcast_bytes, "");
	SYSCTL_ADD_ULONG(ctx, child, OID_AUTO, "rx_mcast_packets", CTLFLAG_RD,
       &stat->rx_mcast_packets, "");
	SYSCTL_ADD_ULONG(ctx, child, OID_AUTO, "rx_bcast_bytes", CTLFLAG_RD,
       &stat->rx_bcast_bytes, "");
	SYSCTL_ADD_ULONG(ctx, child, OID_AUTO, "rx_bcast_packets", CTLFLAG_RD,
       &stat->rx_bcast_packets, "");

	SYSCTL_ADD_ULONG(ctx, child, OID_AUTO, "rx_ucast_drop_bytes", CTLFLAG_RD,
       &stat->rx_ucast_drop_bytes, "");
	SYSCTL_ADD_ULONG(ctx, child, OID_AUTO, "rx_ucast_drop_packets", CTLFLAG_RD,
       &stat->rx_ucast_drop_packets, "");
	SYSCTL_ADD_ULONG(ctx, child, OID_AUTO, "rx_mcast_drop_bytes", CTLFLAG_RD,
       &stat->rx_mcast_drop_bytes, "");
	SYSCTL_ADD_ULONG(ctx, child, OID_AUTO, "rx_mcast_drop_packets", CTLFLAG_RD,
       &stat->rx_mcast_drop_packets, "");
	SYSCTL_ADD_ULONG(ctx, child, OID_AUTO, "rx_bcast_drop_bytes", CTLFLAG_RD,
       &stat->rx_bcast_drop_bytes, "");
	SYSCTL_ADD_ULONG(ctx, child, OID_AUTO, "rx_bcast_drop_packets", CTLFLAG_RD,
       &stat->rx_bcast_drop_packets, "");

	SYSCTL_ADD_ULONG(ctx, child, OID_AUTO, "rx_dma_error", CTLFLAG_RD,
       &stat->rx_dma_error, "");

	SYSCTL_ADD_ULONG(ctx, child, OID_AUTO, "tx_ucast__bytes", CTLFLAG_RD,
       &stat->tx_ucast_bytes, "");
	SYSCTL_ADD_ULONG(ctx, child, OID_AUTO, "tx_ucast_packets", CTLFLAG_RD,
       &stat->tx_ucast_packets, "");
	SYSCTL_ADD_ULONG(ctx, child, OID_AUTO, "tx_mcast_bytes", CTLFLAG_RD,
       &stat->tx_mcast_bytes, "");
	SYSCTL_ADD_ULONG(ctx, child, OID_AUTO, "tx_mcast_packets", CTLFLAG_RD,
       &stat->tx_mcast_packets, "");
	SYSCTL_ADD_ULONG(ctx, child, OID_AUTO, "tx_bcast_bytes", CTLFLAG_RD,
       &stat->tx_bcast_bytes, "");
	SYSCTL_ADD_ULONG(ctx, child, OID_AUTO, "tx_bcast_packets", CTLFLAG_RD,
       &stat->tx_bcast_packets, "");

	SYSCTL_ADD_ULONG(ctx, child, OID_AUTO, "tx_ucast_drop_bytes", CTLFLAG_RD,
       &stat->tx_ucast_drop_bytes, "");
	SYSCTL_ADD_ULONG(ctx, child, OID_AUTO, "tx_ucast_drop_packets", CTLFLAG_RD,
       &stat->tx_ucast_drop_packets, "");
	SYSCTL_ADD_ULONG(ctx, child, OID_AUTO, "tx_mcast_drop_bytes", CTLFLAG_RD,
       &stat->tx_mcast_drop_bytes, "");
	SYSCTL_ADD_ULONG(ctx, child, OID_AUTO, "tx_mcast_drop_packets", CTLFLAG_RD,
       &stat->tx_mcast_drop_packets, "");
	SYSCTL_ADD_ULONG(ctx, child, OID_AUTO, "tx_bcast_drop_bytes", CTLFLAG_RD,
       &stat->tx_bcast_drop_bytes, "");
	SYSCTL_ADD_ULONG(ctx, child, OID_AUTO, "tx_bcast_drop_packets", CTLFLAG_RD,
       &stat->tx_bcast_drop_packets, "");

	SYSCTL_ADD_ULONG(ctx, child, OID_AUTO, "tx_dma_error", CTLFLAG_RD,
       &stat->tx_dma_error, "");

	SYSCTL_ADD_ULONG(ctx, child, OID_AUTO, "rx_queue_disabled_drop", CTLFLAG_RD,
       &stat->rx_queue_disabled_drop, "");
	SYSCTL_ADD_ULONG(ctx, child, OID_AUTO, "rx_queue_empty_drop", CTLFLAG_RD,
       &stat->rx_queue_empty_drop, "");
	SYSCTL_ADD_ULONG(ctx, child, OID_AUTO, "rx_queue_scheduled", CTLFLAG_RD,
       &stat->rx_queue_scheduled, "");
	SYSCTL_ADD_ULONG(ctx, child, OID_AUTO, "rx_desc_fetch_error", CTLFLAG_RD,
       &stat->rx_desc_fetch_error, "");
	SYSCTL_ADD_ULONG(ctx, child, OID_AUTO, "rx_desc_data_error", CTLFLAG_RD,
       &stat->rx_desc_data_error, "");

	SYSCTL_ADD_ULONG(ctx, child, OID_AUTO, "tx_queue_disabled", CTLFLAG_RD,
       &stat->tx_queue_disabled, "");
	SYSCTL_ADD_ULONG(ctx, child, OID_AUTO, "tx_queue_scheduled", CTLFLAG_RD,
       &stat->tx_queue_scheduled, "");
	SYSCTL_ADD_ULONG(ctx, child, OID_AUTO, "tx_desc_fetch_error", CTLFLAG_RD,
       &stat->tx_desc_fetch_error, "");
	SYSCTL_ADD_ULONG(ctx, child, OID_AUTO, "tx_desc_data_error", CTLFLAG_RD,
       &stat->tx_desc_data_error, "");

	SYSCTL_ADD_ULONG(ctx, child, OID_AUTO, "rx_rss", CTLFLAG_RD,
       &stat->rx_rss, "");
	SYSCTL_ADD_ULONG(ctx, child, OID_AUTO, "rx_csum_complete", CTLFLAG_RD,
       &stat->rx_csum_complete, "");
	SYSCTL_ADD_ULONG(ctx, child, OID_AUTO, "rx_csum_ip_bad", CTLFLAG_RD,
       &stat->rx_csum_ip_bad, "");
	SYSCTL_ADD_ULONG(ctx, child, OID_AUTO, "rx_csum_tcp_bad", CTLFLAG_RD,
       &stat->rx_csum_tcp_bad, "");
	SYSCTL_ADD_ULONG(ctx, child, OID_AUTO, "rx_csum_udp_bad", CTLFLAG_RD,
       &stat->rx_csum_udp_bad, "");
	SYSCTL_ADD_ULONG(ctx, child, OID_AUTO, "rx_vlan_strip", CTLFLAG_RD,
       &stat->rx_vlan_strip, "");

	SYSCTL_ADD_ULONG(ctx, child, OID_AUTO, "tx_csum_hw", CTLFLAG_RD,
       &stat->tx_csum_hw, "");
	SYSCTL_ADD_ULONG(ctx, child, OID_AUTO, "tx_csum_hw_inner", CTLFLAG_RD,
       &stat->tx_csum_hw_inner, "");
	SYSCTL_ADD_ULONG(ctx, child, OID_AUTO, "tx_vlan_insert", CTLFLAG_RD,
       &stat->tx_vlan_insert, "");
	SYSCTL_ADD_ULONG(ctx, child, OID_AUTO, "tx_sg", CTLFLAG_RD,
       &stat->tx_sg, "");
	SYSCTL_ADD_ULONG(ctx, child, OID_AUTO, "tx_tso_sg", CTLFLAG_RD,
       &stat->tx_tso_sg, "");
	SYSCTL_ADD_ULONG(ctx, child, OID_AUTO, "tx_tso_sop", CTLFLAG_RD,
       &stat->tx_tso_sop, "");
	SYSCTL_ADD_ULONG(ctx, child, OID_AUTO, "tx_tso_eop", CTLFLAG_RD,
       &stat->tx_tso_eop, "");

	SYSCTL_ADD_ULONG(ctx, child, OID_AUTO, "tx_opcode_invalid", CTLFLAG_RD,
       &stat->tx_opcode_invalid, "");
	SYSCTL_ADD_ULONG(ctx, child, OID_AUTO, "tx_opcode_csum_none", CTLFLAG_RD,
       &stat->tx_opcode_csum_none, "");
	SYSCTL_ADD_ULONG(ctx, child, OID_AUTO, "tx_opcode_csum_partial", CTLFLAG_RD,
       &stat->tx_opcode_csum_partial, "");
	SYSCTL_ADD_ULONG(ctx, child, OID_AUTO, "tx_opcode_csum_hw", CTLFLAG_RD,
       &stat->tx_opcode_csum_hw, "");
	SYSCTL_ADD_ULONG(ctx, child, OID_AUTO, "tx_opcode_csum_tso", CTLFLAG_RD,
       &stat->tx_opcode_csum_tso, "");
}

static void
ionic_setup_device_stats(struct lif *lif)
{
	struct sysctl_ctx_list *ctx = &lif->sysctl_ctx;
	struct sysctl_oid *tree = lif->sysctl_ifnet;
	struct sysctl_oid_list *child = SYSCTL_CHILDREN(tree);

	struct sysctl_oid *queue_node;
	struct sysctl_oid_list *queue_list;
	struct adminq* adminq = lif->adminqcq;
	struct notifyq* notifyq = lif->notifyq;
	int i;

#define QUEUE_NAME_LEN 32
	char namebuf[QUEUE_NAME_LEN];

	SYSCTL_ADD_UINT(ctx, child, OID_AUTO, "numq", CTLFLAG_RD,
       &lif->ntxqs, 0, "Number of Tx/Rx queue pairs");
	SYSCTL_ADD_UINT(ctx, child, OID_AUTO, "hw_capabilities", CTLFLAG_RD,
       &lif->hw_features, 0, "Hardware features enabled like checksum, TSO etc");
	SYSCTL_ADD_ULONG(ctx, child, OID_AUTO, "legacy_spurious_interrupts", CTLFLAG_RD,
       &lif->spurious, "Legacy interrupt count");   
	ionic_setup_hw_stats(lif, ctx, child);

	SYSCTL_ADD_PROC(ctx, child, OID_AUTO, "coal_usecs",
	    CTLTYPE_UINT | CTLFLAG_RW, lif, 0,
	    ionic_intr_coal_handler, "IU", "Interrupt coalescing timeout in usecs");

	snprintf(namebuf, QUEUE_NAME_LEN, "adq");
	queue_node = SYSCTL_ADD_NODE(ctx, child, OID_AUTO, namebuf,
				CTLFLAG_RD, NULL, "Queue Name");
	queue_list = SYSCTL_CHILDREN(queue_node);

	SYSCTL_ADD_UINT(ctx, queue_list, OID_AUTO, "num_descs", CTLFLAG_RD,
       &adminq->num_descs, 0, "Number of descriptors");
	SYSCTL_ADD_UINT(ctx, queue_list, OID_AUTO, "head", CTLFLAG_RD,
        &adminq->head_index, 0, "Head index");
	SYSCTL_ADD_UINT(ctx, queue_list, OID_AUTO, "tail", CTLFLAG_RD,
        &adminq->tail_index, 0, "Tail index");
    SYSCTL_ADD_UINT(ctx, queue_list, OID_AUTO, "comp_index", CTLFLAG_RD,
        &adminq->comp_index, 0, "Completion index");

	snprintf(namebuf, QUEUE_NAME_LEN, "nq");
	queue_node = SYSCTL_ADD_NODE(ctx, child, OID_AUTO, namebuf,
				CTLFLAG_RD, NULL, "Queue Name");
	queue_list = SYSCTL_CHILDREN(queue_node);

	SYSCTL_ADD_UINT(ctx, queue_list, OID_AUTO, "num_descs", CTLFLAG_RD,
       &notifyq->num_descs, 0, "Number of descriptors");
	SYSCTL_ADD_ULONG(ctx, queue_list, OID_AUTO, "last_eid", CTLFLAG_RD,
        &lif->last_eid, "Last event Id");
    SYSCTL_ADD_UINT(ctx, queue_list, OID_AUTO, "comp_index", CTLFLAG_RD,
        &adminq->comp_index, 0, "Completion index");

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
ionic_setup_sysctls(struct lif *lif)
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
 * Set features.
 */
int
ionic_set_os_features(struct ifnet* ifp, uint32_t hw_features)
{
    /*
	 * Set software only capabilities.
	 * XXX: read counters from h/w using stats_dump
	 */
	ifp->if_capabilities = IFCAP_JUMBO_MTU | IFCAP_HWSTATS | IFCAP_LRO;

	if (hw_features & ETH_HW_TX_CSUM)
		ifp->if_capabilities |=  IFCAP_TXCSUM | IFCAP_TXCSUM_IPV6;

	if (hw_features & ETH_HW_RX_CSUM)
		ifp->if_capabilities |=  IFCAP_RXCSUM | IFCAP_RXCSUM_IPV6;

	if (hw_features & ETH_HW_TSO)
		ifp->if_capabilities |=  IFCAP_TSO4;

	if (hw_features & ETH_HW_TSO_IPV6)
		ifp->if_capabilities |=  IFCAP_TSO6;

#define ETH_HW_VLAN (ETH_HW_VLAN_TX_TAG | ETH_HW_VLAN_RX_STRIP | ETH_HW_VLAN_RX_FILTER)

	if ((hw_features & ETH_HW_VLAN)) { 
		ifp->if_capabilities |=  IFCAP_VLAN_MTU |
			IFCAP_VLAN_HWTAGGING |
			IFCAP_VLAN_HWCSUM |
			IFCAP_VLAN_HWFILTER;

		if (hw_features & (ETH_HW_TSO | ETH_HW_TSO_IPV6))
			ifp->if_capabilities |= IFCAP_VLAN_HWTSO;
	}

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

 	if(lif->sysctl_ifnet)
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
	struct ifreq *ifr = (struct ifreq *) data;
	int error = 0;
	uint16_t hw_features;

	switch (command) {  
	case SIOCSIFMEDIA:
	case SIOCGIFMEDIA:
	case SIOCGIFXMEDIA:
		IONIC_NETDEV_INFO(ifp, "ioctl: SIOCxIFMEDIA (Get/Set Interface Media)\n");
		error = ifmedia_ioctl(ifp, ifr, &lif->media, command);
		break;

	case SIOCSIFCAP:
	{
		int mask = ifr->ifr_reqcap ^ ifp->if_capenable;

		IONIC_NETDEV_INFO(ifp, "Required: 0x%x enabled: 0x%x\n",
			ifr->ifr_reqcap, ifp->if_capenable);
		if (!mask) {
			IONIC_NETDEV_INFO(ifp, "Nothing to do\n");
			break;
		}

		hw_features = lif->hw_features;

		if (mask & (IFCAP_RXCSUM | IFCAP_RXCSUM_IPV6)) {
			ifp->if_capenable ^= IFCAP_RXCSUM;
			ifp->if_capenable ^= IFCAP_RXCSUM_IPV6;
			hw_features ^= ETH_HW_RX_CSUM;
		}

		if (mask & (IFCAP_TXCSUM | IFCAP_TXCSUM_IPV6)) {
			ifp->if_capenable ^= IFCAP_TXCSUM;
			ifp->if_capenable ^= IFCAP_TXCSUM_IPV6;
			hw_features ^= ETH_HW_TX_CSUM;
		}

		if (mask & IFCAP_TSO4) {
			ifp->if_capenable ^= IFCAP_TSO4;
			/* XXX: clear ETH_HW_TX_SG? */
			hw_features ^= ETH_HW_TSO;
		}

		if (mask & IFCAP_TSO6) {
			ifp->if_capenable ^= IFCAP_TSO6;
			/* XXX: clear ETH_HW_TX_SG? */
			hw_features ^= ETH_HW_TSO_IPV6;
		}

		if (mask & IFCAP_LRO) {
			ifp->if_capenable ^= IFCAP_LRO;
		}

		if (mask & IFCAP_VLAN_HWTAGGING) {
			ifp->if_capenable ^= IFCAP_VLAN_HWTAGGING;
			hw_features ^= ETH_HW_VLAN_TX_TAG;
		}
		if (mask & IFCAP_VLAN_HWFILTER) {
			ifp->if_capenable ^= IFCAP_VLAN_HWFILTER;
			hw_features ^= (ETH_HW_VLAN_RX_STRIP | ETH_HW_VLAN_RX_FILTER);
		}
		if (mask & IFCAP_VLAN_HWTSO) {
			ifp->if_capenable ^= IFCAP_VLAN_HWTSO;
		}

		IONIC_CORE_LOCK(lif);
		error = ionic_set_hw_feature(lif, hw_features);

		if (error) {				
			IONIC_NETDEV_ERROR(lif->netdev, "Failed to set capbilities, err: %d\n\n",
				error);
			IONIC_CORE_UNLOCK(lif);
			break;
		}
		
		IONIC_CORE_UNLOCK(lif);
		VLAN_CAPABILITIES(ifp);
		break;
	}

	case SIOCSIFMTU:
		IONIC_NETDEV_INFO(ifp, "ioctl: SIOCSIFMTU (Set Interface MTU)\n");
		if (ifr->ifr_mtu > IONIC_MAX_MTU) {
			error = EINVAL;
		} else {
			lif->max_frame_size = ifr->ifr_mtu + (ETHER_HDR_LEN + ETHER_CRC_LEN);

			ionic_change_mtu(ifp, ifr->ifr_mtu);
		}
		break;

	case SIOCADDMULTI:
	case SIOCDELMULTI:
		IONIC_CORE_LOCK(lif);
		ionic_set_multi(lif);
		IONIC_CORE_UNLOCK(lif);
		break;

	default:
		error = ether_ioctl(ifp, command, data);
		break;
	}

	return (error);
}

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

int
ionic_lif_rss_setup(struct lif *lif)
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
		IONIC_NETDEV_ERROR(netdev, "failed to allocate RSS table\n");
		return -ENOMEM;
	}

	/* Fill indirection table with 'default' values */

	for (i = 0; i < RSS_IND_TBL_SIZE; i++) {
#ifdef RSS
		lif->rss_ind_tbl[i] = rss_get_indirection_to_bucket(i) % lif->nrxqs;
#else
		lif->rss_ind_tbl[i] = i % lif->nrxqs;
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
#ifdef __FreeBSD__
	if_link_state_change(netdev, LINK_STATE_DOWN);
	netdev->if_drv_flags &= ~IFF_DRV_RUNNING;
#else
	netif_carrier_off(netdev);
	netif_tx_disable(netdev);
#endif
}

void ionic_up_link(struct net_device *netdev)
{
#ifdef __FreeBSD__
	if_link_state_change(netdev, LINK_STATE_UP);
	netdev->if_drv_flags |= IFF_DRV_RUNNING;
#else
	netif_carrier_off(netdev);
	netif_tx_disable(netdev);
#endif
}
