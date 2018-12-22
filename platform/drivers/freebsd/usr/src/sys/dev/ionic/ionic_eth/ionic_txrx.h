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

#ifndef _IONIC_TXRX_H_
#define _IONIC_TXRX_H_

#define NON_TSO_MAX_DESC	16

int ionic_setup_rx_intr(struct rxque* rxq);
int ionic_setup_tx_intr(struct txque* txq);
int ionic_setup_legacy_intr(struct lif* lif);

int ionic_start_xmit(struct net_device *netdev, struct mbuf *m);
int ionic_start_xmit_locked(struct ifnet* ifp, 	struct txque* txq);

int ionic_lif_netdev_alloc(struct lif* lif, int ndescs);
void ionic_lif_netdev_free(struct lif* lifs);

int ionic_set_os_features(struct ifnet* ifp, uint32_t hw_features);
int ionic_enable_os_features(struct ifnet* ifp);

void ionic_setup_sysctls(struct lif *lif);
int ionic_lif_rss_setup(struct lif *lif);
void ionic_lif_rss_teardown(struct lif *lif);

void ionic_down_link(struct net_device *netdev);
void ionic_up_link(struct net_device *netdev);

int ionic_rx_mbuf_alloc(struct rxque *rxq, int index, int len);
void ionic_rx_mbuf_free(struct rxque *rxq, struct ionic_rx_buf *rxbuf);
void ionic_rx_destroy_map(struct rxque *rxq, struct ionic_rx_buf *rxbuf);

/* sysctl variables. */
extern int ionic_use_adminq;
extern int ionic_enable_msix;
extern int ionic_rx_stride;
extern int ionic_rx_fill_threshold;
extern int ionic_rx_process_limit;
extern u32 ionic_tx_coalesce_usecs;
extern u32 ionic_rx_coalesce_usecs;
extern int ionic_max_queues;
extern int ntxq_descs;
extern int nrxq_descs;
extern int adminq_descs;
extern int ionic_notifyq_descs;

#endif /* _IONIC_TXRX_H_ */
