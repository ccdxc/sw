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

#ifndef _IONIC_TXRX_H_
#define _IONIC_TXRX_H_

#define NON_TSO_MAX_DESC	16

void ionic_rx_refill(struct queue *q);
void ionic_rx_fill(struct queue *q);
void ionic_rx_empty(struct queue *q);
void ionic_rx_flush(struct cq *cq);
int ionic_tx_napi(struct napi_struct *napi, int budget);
int ionic_rx_napi(struct napi_struct *napi, int budget);
netdev_tx_t ionic_start_xmit(struct sk_buff *skb, struct net_device *netdev);

#endif /* _IONIC_TXRX_H_ */
