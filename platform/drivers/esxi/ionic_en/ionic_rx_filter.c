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

#include "ionic.h"
#include "ionic_lif.h"
#include "ionic_rx_filter.h"
#include "ionic_locks.h"
#include "ionic_log.h"

void
ionic_rx_filter_free(struct rx_filter *f)
{
        ionic_heap_free(ionic_driver.heap_id, f);
}


VMK_ReturnStatus
ionic_rx_filter_del(struct lif *lif, struct rx_filter *f)
{
        VMK_ReturnStatus status;
        struct ionic_admin_ctx ctx = {
//                .work = COMPLETION_INITIALIZER_ONSTACK(ctx.work),
                .cmd.rx_filter_del = {
                        .opcode = CMD_OPCODE_RX_FILTER_DEL,
                        .filter_id = f->filter_id,
                },
        };

        status = ionic_completion_create(ionic_driver.module_id,
					 ionic_driver.heap_id,
					 ionic_driver.lock_domain,
					 "ionic_admin_ctx.work",
					 &ctx.work);
	if (status != VMK_OK) {
		ionic_err("ionic_completion_create() failed, status: %s",
			  vmk_StatusToString(status));
		return status;
	}

        status  = ionic_adminq_post_wait(lif, &ctx);
        ionic_completion_destroy(&ctx.work);
	if (status != VMK_OK) {
                ionic_err("ionic_adminq_post_wait() failed, status: %s",
                          vmk_StatusToString(status));
                return status;
        }

	ionic_info("rx_filter del (id %d)\n",
		   ctx.cmd.rx_filter_del.filter_id);

        return status;
}

        
static inline vmk_HashKeyIteratorCmd
ionic_hash_table_free_filter_iterator(vmk_HashTable hash_table,
                                      vmk_HashKey key,
                                      vmk_HashValue value,
                                      vmk_AddrCookie addr_cookie)
{
        vmk_uint32 *cnt = (vmk_uint32 *) addr_cookie.ptr;
        struct rx_filter *f = (struct rx_filter *) value;

//        status = ionic_rx_filter_del(f->lif, f);
//        VMK_ASSERT(status == VMK_OK);

        ionic_rx_filter_free(f);

        (*cnt)++; 

        return VMK_HASH_KEY_ITER_CMD_CONTINUE;
}


VMK_ReturnStatus
ionic_rx_filters_init(struct lif *lif)
{
        VMK_ReturnStatus status;
        vmk_HashProperties hash_props;

//        spin_lock_init(&lif->rx_filters.lock);
        status = ionic_spinlock_create("lif->rx_filters.lock",
                                       ionic_driver.module_id,
                                       ionic_driver.heap_id ,
                                       ionic_driver.lock_domain,
                                       VMK_SPINLOCK, 
                                       IONIC_LOCK_RANK_HIGH,
                                       &lif->rx_filters.lock);
        if (status != VMK_OK) {
                ionic_err("ionic_spinlock_create() failed, status: %s",
                          vmk_StatusToString(status));
                return status;
        }

        vmk_Memset(&hash_props, 0, sizeof(vmk_HashProperties));

        hash_props.moduleID  = ionic_driver.module_id;
        hash_props.heapID    = ionic_driver.heap_id;
        hash_props.keyType   = VMK_HASH_KEY_TYPE_INT;
        hash_props.keyFlags  = VMK_HASH_KEY_FLAGS_NONE;
        hash_props.keySize   = 0;
        hash_props.nbEntries = RX_FILTER_HLISTS;
        hash_props.acquire   = NULL;
        hash_props.release   = NULL;

        status = vmk_HashAlloc(&hash_props,
                               &lif->rx_filters.by_hash);
        if (status != VMK_OK) {
                ionic_err("vmk_HashAlloc() failed, status: %s",
                          vmk_StatusToString(status));
                goto by_hash_err;
        }

        status = vmk_HashAlloc(&hash_props,
                               &lif->rx_filters.by_id);
        if (status != VMK_OK) {
                ionic_err("vmk_HashAlloc() failed, status: %s",
                          vmk_StatusToString(status));
                goto by_id_err;
        }
       
        return status;
/*
        for (i = 0; i < RX_FILTER_HLISTS; i++) {
                INIT_HLIST_HEAD(&lif->rx_filters.by_hash[i]);
                INIT_HLIST_HEAD(&lif->rx_filters.by_id[i]);
        }
*/

by_id_err:
        vmk_HashRelease(lif->rx_filters.by_hash);         

by_hash_err:
        ionic_spinlock_destroy(lif->rx_filters.lock);

        return status;
}

static void
ionic_rx_filters_hash_tables_destroy(struct rx_filters *rx_filters)     // IN
{
        VMK_ReturnStatus status;

        vmk_SpinlockLock(rx_filters->lock);
        status = vmk_HashDeleteAll(rx_filters->by_id);
        if (status != VMK_OK) {
                ionic_err("vmk_HashDeleteAll() failed, status: %s",
                          vmk_StatusToString(status));
        }
        vmk_SpinlockUnlock(rx_filters->lock);

        status = vmk_HashRelease(rx_filters->by_id);
        if (status != VMK_OK) {
                ionic_err("vmk_HashRelease() failed, status: %s",
                          vmk_StatusToString(status));
        }

        vmk_SpinlockLock(rx_filters->lock);
        status = vmk_HashDeleteAll(rx_filters->by_hash);
        if (status != VMK_OK) {
                ionic_err("vmk_HashDeleteAll() failed, status: %s",
                          vmk_StatusToString(status));
        }
        vmk_SpinlockUnlock(rx_filters->lock);

        status = vmk_HashRelease(rx_filters->by_hash);;
        if (status != VMK_OK) {
                ionic_err("vmk_HashRelease() failed, status: %s",
                          vmk_StatusToString(status));
        }

}



void ionic_rx_filters_deinit(struct lif *lif)
{
        VMK_ReturnStatus status;
        vmk_AddrCookie addr_cookie;
        vmk_uint32 cnt = 0;

        addr_cookie.ptr = &cnt;

        status = vmk_HashKeyIterate(lif->rx_filters.by_hash,
                                    ionic_hash_table_free_filter_iterator,
                                    addr_cookie);
        if (status == VMK_OK) {
                ionic_info("vmk_HashKeyIterate() returns %d elements", cnt);
        } else {
                ionic_err("vmk_HashKeyIterate() failed, status: %s",
                          vmk_StatusToString(status));
        }

        ionic_rx_filters_hash_tables_destroy(&lif->rx_filters);

        ionic_spinlock_destroy(lif->rx_filters.lock);
/*
        for (i = 0; i < RX_FILTER_HLISTS; i++) {
                head = &lif->rx_filters.by_id[i];
                hlist_for_each_entry_safe(f, tmp, head, by_id)
                        ionic_rx_filter_free(lif, f);
        }
*/
}


VMK_ReturnStatus
ionic_rx_filter_save(struct lif *lif, u32 flow_id, u16 rxq_index,
                     u32 hash, struct ionic_admin_ctx *ctx)
{
//        struct device *dev = lif->ionic->dev;
//        struct rx_filter *f = devm_kzalloc(dev, sizeof(*f), GFP_KERNEL);
//        struct hlist_head *head;
        VMK_ReturnStatus status;
        unsigned int key;

//        if (!f)
//                return -ENOMEM;
        struct rx_filter *f;

        f = ionic_heap_zalloc(ionic_driver.heap_id,
                              sizeof(struct rx_filter));
        if (VMK_UNLIKELY(f == NULL)) {
                status = VMK_NO_MEMORY;
                ionic_err("ionic_heap_zalloc() failed, status: %s",
                          vmk_StatusToString(status));
                return status;
        }

        f->flow_id = flow_id;
        f->filter_id = ctx->comp.rx_filter_add.filter_id;
        f->rxq_index = rxq_index;
        f->lif = lif;
        vmk_Memcpy(&f->cmd, &ctx->cmd, sizeof(f->cmd));

//        INIT_HLIST_NODE(&f->by_hash);
//        INIT_HLIST_NODE(&f->by_id);

        switch (f->cmd.match) {
                case RX_FILTER_MATCH_VLAN:
                        key = f->cmd.vlan.vlan & RX_FILTER_HLISTS_MASK;
                        break;
                case RX_FILTER_MATCH_MAC:
                        key = *(u32 *)f->cmd.mac.addr & RX_FILTER_HLISTS_MASK;
                        break;
                case RX_FILTER_MATCH_MAC_VLAN:
                        key = f->cmd.mac_vlan.vlan & RX_FILTER_HLISTS_MASK;
                        break;
                default:
                        ionic_err("Unsupported filter command, %d", f->cmd.match);
                        return VMK_NOT_SUPPORTED;
        }

        vmk_SpinlockLock(lif->rx_filters.lock);

        status = vmk_HashKeyInsert(lif->rx_filters.by_hash,
                                   (vmk_HashKey) (vmk_uint64) key,
                                   (vmk_HashValue) f);
        if (status != VMK_OK) {
                ionic_err("vmk_HashKeyInsert() failed, status: %s",
                          vmk_StatusToString(status));
                goto by_hash_insert_err;
        }
//        head = &lif->rx_filters.by_hash[key];
//        hlist_add_head(&f->by_hash, head);

        key = f->filter_id & RX_FILTER_HLISTS_MASK;
//        head = &lif->rx_filters.by_id[key];
//        hlist_add_head(&f->by_id, head);
 
        status = vmk_HashKeyInsert(lif->rx_filters.by_id,
                                   (vmk_HashKey) (vmk_uint64) key,
                                   (vmk_HashValue) f);
        if (status != VMK_OK) {
                ionic_err("vmk_HashKeyInsert() failed, status: %s",
                          vmk_StatusToString(status));
                // TODO: DO WE NEED TO REMOVE previous element that inserted into by_hash?
                goto by_hash_insert_err;
        }

        vmk_SpinlockUnlock(lif->rx_filters.lock);

        return status;

by_hash_insert_err:
        ionic_heap_free(ionic_driver.heap_id, f);

        vmk_SpinlockUnlock(lif->rx_filters.lock);

        return status;
}

struct rx_filter *ionic_rx_filter_by_vlan(struct lif *lif, u16 vid)
{
        VMK_ReturnStatus status;
        unsigned int key = vid & RX_FILTER_HLISTS_MASK;
//        struct hlist_head *head = &lif->rx_filters.by_hash[key];
        struct rx_filter *f = NULL;

        status = vmk_HashKeyDelete(lif->rx_filters.by_hash,
                                   (vmk_HashKey) (vmk_uint64) key,
                                   (vmk_HashValue *) &f);
        if (status == VMK_OK) {
                return f;
        } else {
                return NULL;
        }
/*
        hlist_for_each_entry(f, head, by_hash) {
                if (f->cmd.match != RX_FILTER_MATCH_VLAN)
                        continue;
                if (f->cmd.vlan.vlan == vid)
                        return f;
        }
*/
//        return NULL;
}

struct rx_filter *ionic_rx_filter_by_addr(struct lif *lif, const u8 *addr)
{
        VMK_ReturnStatus status;
        unsigned int key = *(u32 *)addr & RX_FILTER_HLISTS_MASK;
//        struct hlist_head *head = &lif->rx_filters.by_hash[key];
        struct rx_filter *f = NULL;

        status = vmk_HashKeyDelete(lif->rx_filters.by_hash,
                                   (vmk_HashKey) (vmk_uint64) key,
                                   (vmk_HashValue *) &f); 
        if (status == VMK_OK) {
                return f;
        } else {
                return NULL;
        }
/*
        hlist_for_each_entry(f, head, by_hash) {
                if (f->cmd.match != RX_FILTER_MATCH_MAC)
                        continue;
                if (memcmp(addr, f->cmd.mac.addr, ETH_ALEN) == 0)
                        return f;
        }

        return NULL;
*/
}

