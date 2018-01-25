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

#ifndef _IONIC_ADMINQ_H_
#define _IONIC_ADMINQ_H_

int ionic_adminq_txq_init(struct queue *adminq, struct queue *txq,
			  struct cq *cq, unsigned int cos);
int ionic_adminq_rxq_init(struct queue *adminq, struct queue *rxq,
			  struct cq *cq);
int ionic_adminq_q_enable(struct queue *adminq, struct queue *q, desc_cb cb,
			  void *cb_arg);
int ionic_adminq_q_disable(struct queue *adminq, struct queue *q, desc_cb cb,
			   void *cb_arg);
int ionic_adminq_station_get(struct queue *adminq, desc_cb cb, void *cb_arg);
int ionic_adminq_mtu_set(struct queue *adminq, int new_mtu, desc_cb cb,
			 void *cb_arg);
int ionic_adminq_rx_mode_set(struct queue *adminq, unsigned int rx_mode,
			     desc_cb cb, void *cb_arg);
int ionic_adminq_rx_filter_mac(struct queue *adminq, const u8 *addr, bool add,
			       desc_cb cb, void *cb_arg);
int ionic_adminq_rx_filter_vlan(struct queue *adminq, u16 vid, bool add,
				desc_cb cb, void *cb_arg);
int ionic_adminq_features(struct queue *adminq, u16 set, desc_cb cb,
			  void *cb_arg);
int ionic_adminq_rdma_cmd(struct queue *adminq, struct admin_cmd *cmd,
			  desc_cb cb, void *cb_arg);
bool ionic_adminq_service(struct cq *cq, struct cq_info *cq_info,
			  void *cb_arg);

#endif /* _IONIC_ADMINQ_H_ */
