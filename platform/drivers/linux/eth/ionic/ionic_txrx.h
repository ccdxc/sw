/* SPDX-License-Identifier: GPL-2.0 */
/* Copyright(c) 2017 - 2019 Pensando Systems, Inc */

#ifndef _IONIC_TXRX_H_
#define _IONIC_TXRX_H_

void ionic_rx_refill(struct queue *q);
void ionic_rx_fill(struct queue *q);
void ionic_rx_empty(struct queue *q);
void ionic_rx_flush(struct cq *cq);
int ionic_tx_napi(struct napi_struct *napi, int budget);
int ionic_rx_napi(struct napi_struct *napi, int budget);
netdev_tx_t ionic_start_xmit(struct sk_buff *skb, struct net_device *netdev);

#endif /* _IONIC_TXRX_H_ */
