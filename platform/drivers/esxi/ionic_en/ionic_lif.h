/*
 * Copyright (c) 2019 Pensando Systems. All rights reserved.
 */

#ifndef _IONIC_LIF_H_
#define _IONIC_LIF_H_

#define NOTIFYQ_LENGTH  64      /* must be a power of two */

#include "ionic_rx_filter.h"
#include "ionic_en_uplink.h"

struct tx_stats {
        u64 dma_map_err;
        u64 pkts;
        u64 bytes;
        u64 clean;
        u64 drop;
        u64 linearize;
        u64 stop;
        u64 wake;
        u64 no_csum;
        u64 csum;
        u64 tso;
        u64 frags;
        u64 encap;
        u64 tx_queue_waken;
        u64 busy;
};

struct rx_stats {
        u64 dma_map_err;
        u64 alloc_err;
        u64 pkts;
        u64 bytes;
        u64 csum_err;
        u64 csum_complete;
        u64 no_csum;
        u64 encap;
};

#define QCQ_F_INITED            BIT(0)
#define QCQ_F_SG                BIT(1)
#define QCQ_F_INTR              BIT(2)
#define QCQ_F_TX_STATS          BIT(3)
#define QCQ_F_RX_STATS          BIT(4)
#define QCQ_F_NOTIFYQ           BIT(5)

struct qcq {
        vmk_uint32 ring_idx;

        /* tx/rx_ring that associates with this qcq */
        void *ring;

        void *base;
        dma_addr_t base_pa;
        unsigned int total_size;
        struct queue q;
        struct cq cq;
        struct intr intr;
//        struct napi_struct napi;
        vmk_NetPoll netpoll;
        vmk_Bool is_netpoll_created;
        vmk_Bool is_netpoll_enabled;
        union {
                struct tx_stats tx;
                struct rx_stats rx;
        } stats;

        unsigned int flags;
};

#define q_to_qcq(q)             IONIC_CONTAINER_OF(q, struct qcq, q)
#define q_to_tx_stats(q)        (&q_to_qcq(q)->stats.tx)
#define q_to_rx_stats(q)        (&q_to_qcq(q)->stats.rx)
#define napi_to_qcq(napi)       container_of(napi, struct qcq, napi)
#define napi_to_cq(napi)        (&napi_to_qcq(napi)->cq)

enum deferred_work_type {
        DW_TYPE_RX_MODE,
        DW_TYPE_RX_ADDR_ADD,
        DW_TYPE_RX_ADDR_DEL,
};

struct deferred_work {
//        struct list_head list;
        vmk_ListLinks list;
        enum deferred_work_type type;
        union {
                unsigned int rx_mode;
                u8 addr[VMK_ETH_ADDR_LENGTH];
        };
};

struct deferred {
        spinlock_t lock;
        //struct list_head list;
        vmk_ListLinks list;
//        struct work_struct work;
        struct ionic_work work;
};

#define LIF_F_INITED            BIT(0)
#define LIF_NAME_MAX_SZ         (32)
struct lif {
        char name[LIF_NAME_MAX_SZ];
        vmk_ListLinks list;
        struct ionic_en_uplink_handle *uplink_handle;
        struct ionic *ionic;
        bool registered;
        unsigned int index;
        unsigned int hw_index;
        unsigned int kern_pid;
        spinlock_t adminq_lock;
        struct qcq *adminqcq;
        struct qcq *notifyqcq;
        struct qcq **txqcqs;
        struct qcq **rxqcqs;
        u64 last_eid;
        unsigned int neqs;
        unsigned int ntxqcqs;
        unsigned int nrxqcqs;
        unsigned int rx_mode;
        u32 hw_features;

        u16 rss_types;
        u8 rss_hash_key[IONIC_RSS_HASH_KEY_SIZE];
        u8 *rss_ind_tbl;
        u32 rss_ind_tbl_size;
        u32 rss_key_size;
        dma_addr_t rss_ind_tbl_pa;

        struct rx_filters rx_filters;
        struct ionic_work_queue *def_work_queue;///////
        struct deferred deferred;
        u32 tx_coalesce_usecs;
        u32 rx_coalesce_usecs;
        void *api_private;
        u32 flags;

        u32 info_sz;
        struct ionic_lif_info *info;
        dma_addr_t info_pa;
        vmk_Bool is_skip_res_alloc_after_fw;
};

#define lif_to_txq(lif, i)      (&lif->txqcqs[i]->q)
#define lif_to_rxq(lif, i)      (&lif->rxqcqs[i]->q)

VMK_ReturnStatus
ionic_dev_recover_world(void *data);

VMK_ReturnStatus
ionic_lif_identify(struct ionic *ionic);

VMK_ReturnStatus
ionic_lifs_alloc(struct ionic *ionic);

void
ionic_lifs_free(struct ionic *ionic);

void
ionic_lifs_deinit(struct ionic *ionic);

VMK_ReturnStatus
ionic_lifs_init(struct ionic *ionic);

int
ionic_lifs_register(struct ionic *ionic);

void
ionic_lifs_unregister(struct ionic *ionic);

VMK_ReturnStatus
ionic_station_set(struct lif *lif);

VMK_ReturnStatus
ionic_lifs_size(struct ionic *ionic);

VMK_ReturnStatus
ionic_set_rx_mode(struct lif *lif,
                  vmk_UplinkState new_state);
VMK_ReturnStatus
ionic_open(struct lif *lif);

VMK_ReturnStatus
ionic_stop(struct lif *lif);

void
ionic_lifs_size_undo(struct ionic_en_priv_data *priv_data);

void
ionic_notifyq_flush(struct cq *cq);

VMK_ReturnStatus
ionic_intr_alloc(struct lif *lif, struct intr *intr);
void IONIC_QINIT_Free(struct lif *lif, struct intr *intr);

VMK_ReturnStatus
ionic_lif_addr_add(struct lif *lif, const u8 *addr);

VMK_ReturnStatus
ionic_lif_addr_del(struct lif *lif, const u8 *addr);

VMK_ReturnStatus
ionic_qcq_enable(struct qcq *qcq);

VMK_ReturnStatus
ionic_qcq_disable(struct qcq *qcq);

VMK_ReturnStatus
ionic_vlan_rx_add_vid(struct lif *lif,
                      u16 vid);
VMK_ReturnStatus
ionic_vlan_rx_kill_vid(struct lif *lif,
                       u16 vid);

void
ionic_adminq_flush(struct lif *lif);

VMK_ReturnStatus
ionic_lif_rss_config(struct lif *lif, const u16 types,
                     const u8 *key, const u32 *indir);

VMK_ReturnStatus
ionic_lif_set_uplink_info(struct lif *lif);

VMK_ReturnStatus
ionic_lif_quiesce(struct lif *lif);

#endif /* _IONIC_LIF_H_ */

