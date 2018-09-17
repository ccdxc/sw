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

#ifndef napi_struct
#define napi_struct work_struct
#endif
struct napi_struct;

void ionic_tx_napi(struct napi_struct *napi);

int ionic_start_xmit(struct net_device *netdev, struct mbuf *m);
int ionic_start_xmit_locked(struct ifnet* ifp, 	struct txque* txq);

int ionic_lif_netdev_alloc(struct lif* lif, int ndescs);
void ionic_lif_netdev_free(struct lif* lifs);

int ionic_set_os_features(struct ifnet* ifp, uint32_t hw_features);
void ionic_setup_sysctls(struct lif *lif);
int ionic_lif_rss_setup(struct lif *lif);
void ionic_lif_rss_teardown(struct lif *lif);

void ionic_down_link(struct net_device *netdev);
void ionic_up_link(struct net_device *netdev);

int ionic_rx_mbuf_alloc(struct rxque *rxq, int index, int len);
void ionic_rx_mbuf_free(struct rxque *rxq, struct ionic_rx_buf *rxbuf);
void ionic_rx_destroy_map(struct rxque *rxq, struct ionic_rx_buf *rxbuf);

/* sysctl variables. */
extern int ionic_rx_stride;
extern int ionic_rx_fill_threshold;
extern int ionic_rx_process_limit;
#endif /* _IONIC_TXRX_H_ */
