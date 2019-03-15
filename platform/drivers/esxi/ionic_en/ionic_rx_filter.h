/*
 * Copyright (c) 2019 Pensando Systems. All rights reserved.
 *
 * This software is provided to you under the terms of the GNU
 * General Public License (GPL) Version 2.0, available from the file
 * [ionic_rx_filter.h] in the main directory of this source tree.
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

#ifndef _IONIC_RX_FILTER_H_
#define _IONIC_RX_FILTER_H_

#include "ionic_api.h"

struct ionic_admin_ctx;

#define RXQ_INDEX_ANY           (0xFFFF)
struct rx_filter {
        u32 flow_id;
        u32 filter_id;
        u16 rxq_index;
        struct rx_filter_add_cmd cmd;
        struct lif *lif;
//        struct hlist_node by_hash;
//        struct hlist_node by_id;
};

#define RX_FILTER_HLISTS        (1 << 10)
#define RX_FILTER_MAC_MASK      0xffffffffffff
#define RX_FILTER_HLISTS_MASK   (RX_FILTER_HLISTS - 1)

struct rx_filters {
        spinlock_t lock;
//        struct hlist_head by_hash[RX_FILTER_HLISTS];    /* by skb hash */
//        struct hlist_head by_id[RX_FILTER_HLISTS];      /* by filter_id */
        vmk_HashTable by_hash;
        vmk_HashTable by_id;
};

VMK_ReturnStatus
ionic_rx_filters_init(struct lif *lif);

void
ionic_rx_filters_deinit(struct lif *lif);

void
ionic_rx_filter_free(struct rx_filter *f);

VMK_ReturnStatus
ionic_rx_filter_save(struct lif *lif, u32 flow_id, u16 rxq_index,
                     u32 hash, struct ionic_admin_ctx *ctx);

struct rx_filter *ionic_rx_filter_by_vlan(struct lif *lif, u16 vid);
struct rx_filter *ionic_rx_filter_by_addr(struct lif *lif, const u8 *addr);

#if 0
int ionic_rx_filter_del(struct lif *lif, struct rx_filter *f);
void ionic_rx_filters_deinit(struct lif *lif);
int ionic_rx_filter_save(struct lif *lif, u32 flow_id, u16 rxq_index,
                         u32 hash, struct ionic_admin_ctx *ctx);
struct rx_filter *ionic_rx_filter_by_vlan(struct lif *lif, u16 vid);
#endif

#endif /* _IONIC_RX_FILTER_H_ */

