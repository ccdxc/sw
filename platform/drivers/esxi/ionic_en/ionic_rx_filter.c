/*
 * Copyright (c) 2019 Pensando Systems. All rights reserved.
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
                .cmd.rx_filter_del = {
                        .opcode = IONIC_CMD_RX_FILTER_DEL,
                        .filter_id = f->filter_id,
                },
        };

        status = ionic_completion_create(ionic_driver.module_id,
					 ionic_driver.heap_id,
					 ionic_driver.lock_domain,
					 "ionic_admin_ctx.work",
					 &ctx.work);
	if (status != VMK_OK) {
		ionic_en_err("ionic_completion_create() failed, status: %s",
			  vmk_StatusToString(status));
		return status;
	}

        status  = ionic_adminq_post_wait(lif, &ctx);
        ionic_completion_destroy(&ctx.work);
	if (status != VMK_OK) {
                ionic_en_err("ionic_adminq_post_wait() failed, status: %s",
                          vmk_StatusToString(status));
                return status;
        }

	ionic_en_info("rx_filter del (id %d)\n",
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

        ionic_rx_filter_free(f);

        (*cnt)++; 

        return VMK_HASH_KEY_ITER_CMD_CONTINUE;
}


VMK_ReturnStatus
ionic_rx_filters_init(struct lif *lif)
{
        VMK_ReturnStatus status;
        vmk_HashProperties hash_props;

        /* After live FW upgrade, we don't need to reinitialize
         * these resources */
        if (lif->is_skip_res_alloc_after_fw) {
                return VMK_OK;
        }

        status = ionic_spinlock_create("lif->rx_filters.lock",
                                       ionic_driver.module_id,
                                       ionic_driver.heap_id ,
                                       ionic_driver.lock_domain,
                                       VMK_SPINLOCK, 
                                       IONIC_LOCK_RANK_HIGH,
                                       &lif->rx_filters.lock);
        if (status != VMK_OK) {
                ionic_en_err("ionic_spinlock_create() failed, status: %s",
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
                ionic_en_err("vmk_HashAlloc() failed, status: %s",
                          vmk_StatusToString(status));
                goto by_hash_err;
        }

        return status;

by_hash_err:
        ionic_spinlock_destroy(lif->rx_filters.lock);

        return status;
}

static void
ionic_rx_filters_hash_tables_destroy(struct rx_filters *rx_filters)     // IN
{
        VMK_ReturnStatus status;

        vmk_SpinlockLock(rx_filters->lock);
        status = vmk_HashDeleteAll(rx_filters->by_hash);
        if (status != VMK_OK) {
                ionic_en_err("vmk_HashDeleteAll() failed, status: %s",
                          vmk_StatusToString(status));
        }
        vmk_SpinlockUnlock(rx_filters->lock);

        status = vmk_HashRelease(rx_filters->by_hash);
        if (status != VMK_OK) {
                ionic_en_err("vmk_HashRelease() failed, status: %s",
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
                ionic_en_info("vmk_HashKeyIterate() returns %d elements", cnt);
        } else {
                ionic_en_err("vmk_HashKeyIterate() failed, status: %s",
                          vmk_StatusToString(status));
        }

        ionic_rx_filters_hash_tables_destroy(&lif->rx_filters);

        ionic_spinlock_destroy(lif->rx_filters.lock);
}


VMK_ReturnStatus
ionic_rx_filter_save(struct lif *lif, u32 flow_id, u16 rxq_index,
                     u32 hash, struct ionic_admin_ctx *ctx)
{
        VMK_ReturnStatus status;
        unsigned int key;

        struct rx_filter *f;

        f = ionic_heap_zalloc(ionic_driver.heap_id,
                              sizeof(struct rx_filter));
        if (VMK_UNLIKELY(f == NULL)) {
                status = VMK_NO_MEMORY;
                ionic_en_err("ionic_heap_zalloc() failed, status: %s",
                          vmk_StatusToString(status));
                return status;
        }

        f->flow_id = flow_id;
        f->filter_id = ctx->comp.rx_filter_add.filter_id;
        f->rxq_index = rxq_index;
        f->lif = lif;
        vmk_Memcpy(&f->cmd, &ctx->cmd, sizeof(f->cmd));

        switch (f->cmd.match) {
                case IONIC_RX_FILTER_MATCH_VLAN:
                        key = f->cmd.vlan.vlan & RX_FILTER_HLISTS_MASK;
                        break;
                case IONIC_RX_FILTER_MATCH_MAC:
                        key = *(u32 *)f->cmd.mac.addr & RX_FILTER_MAC_MASK;
                        break;
                case IONIC_RX_FILTER_MATCH_MAC_VLAN:
                        key = f->cmd.mac_vlan.vlan & RX_FILTER_HLISTS_MASK;
                        break;
                default:
                        ionic_en_err("Unsupported filter command, %d", f->cmd.match);
                        return VMK_NOT_SUPPORTED;
        }

        vmk_SpinlockLock(lif->rx_filters.lock);

        status = vmk_HashKeyInsert(lif->rx_filters.by_hash,
                                   (vmk_HashKey) (vmk_uint64) key,
                                   (vmk_HashValue) f);
        if (status != VMK_OK) {
                ionic_en_err("vmk_HashKeyInsert() failed, status: %s",
                          vmk_StatusToString(status));
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
        struct rx_filter *f = NULL;

        status = vmk_HashKeyDelete(lif->rx_filters.by_hash,
                                   (vmk_HashKey) (vmk_uint64) key,
                                   (vmk_HashValue *) &f);
        if (status == VMK_OK) {
                return f;
        } else {
                return NULL;
        }
}

struct rx_filter *ionic_rx_filter_by_addr(struct lif *lif, const u8 *addr)
{
        VMK_ReturnStatus status;
        unsigned int key = *(u32 *)addr & RX_FILTER_MAC_MASK;
        struct rx_filter *f = NULL;

        status = vmk_HashKeyDelete(lif->rx_filters.by_hash,
                                   (vmk_HashKey) (vmk_uint64) key,
                                   (vmk_HashValue *) &f); 
        if (status == VMK_OK) {
                return f;
        } else {
                return NULL;
        }
}

