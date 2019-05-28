/*
 * {C} Copyright 2018 Pensando Systems Inc.
 * All rights reserved.
 *
 */

#ifndef _SONIC_DEV_H_
#define _SONIC_DEV_H_

#define static_assert(...)
#include <linux/spinlock.h>
#include <linux/types.h>
#ifdef __FreeBSD__
#include <linux/list.h>
#endif
#include "osal_sys.h"
#include "accel_ring.h"
#include "storage_seq_common.h"
#include "accel_dev_if.h"

#include "pnso_api.h"
#include "pnso_mpool.h"
#include "pnso_stats.h"
#include "pnso_batch.h"

#pragma pack(push, 1)

#define SONIC_BARS_MAX		6

struct sonic_dev_bar {
	void __iomem *vaddr;
	dma_addr_t bus_addr;
	unsigned long len;
};

#define INTR_INDEX_NOT_ASSIGNED		(-1)
#define INTR_NAME_MAX_SZ		(32)

struct intr {
	char name[INTR_NAME_MAX_SZ];
	unsigned int index;
	unsigned int vector;
	struct intr_ctrl __iomem *ctrl;
};

#pragma pack(pop)

static inline void sonic_struct_size_checks(void)
{
	BUILD_BUG_ON(sizeof(struct doorbell) != 8);
	BUILD_BUG_ON(sizeof(struct intr_ctrl) != 32);
	BUILD_BUG_ON(sizeof(struct intr_status) != 8);
	BUILD_BUG_ON(sizeof(union dev_regs) != 4096);
	BUILD_BUG_ON(sizeof(union dev_info_regs) != 2048);
	BUILD_BUG_ON(sizeof(union dev_cmd_regs) != 2048);
	BUILD_BUG_ON(sizeof(struct admin_cmd) != 64);
	BUILD_BUG_ON(sizeof(struct admin_cpl) != 16);
	BUILD_BUG_ON(sizeof(struct nop_cmd) != 64);
	BUILD_BUG_ON(sizeof(struct nop_cpl) != 16);
	BUILD_BUG_ON(sizeof(struct dev_reset_cmd) != 64);
	BUILD_BUG_ON(sizeof(struct dev_reset_cpl) != 16);
	BUILD_BUG_ON(sizeof(struct hang_notify_cmd) != 64);
	BUILD_BUG_ON(sizeof(struct hang_notify_cpl) != 16);
	BUILD_BUG_ON(sizeof(struct dev_identify_cmd) != 64);
	BUILD_BUG_ON(sizeof(struct dev_identify_cpl) != 16);
	BUILD_BUG_ON(sizeof(struct lif_identify_cmd) != 64);
	BUILD_BUG_ON(sizeof(struct lif_identify_cpl) != 16);
	BUILD_BUG_ON(sizeof(struct lif_init_cmd) != 64);
	BUILD_BUG_ON(sizeof(struct lif_init_cpl) != 16);
	BUILD_BUG_ON(sizeof(struct lif_setattr_cmd) != 64);
	BUILD_BUG_ON(sizeof(struct lif_setattr_cpl) != 16);
	BUILD_BUG_ON(sizeof(struct lif_getattr_cmd) != 64);
	BUILD_BUG_ON(sizeof(struct lif_getattr_cpl) != 16);
	BUILD_BUG_ON(sizeof(struct lif_reset_cmd) != 64);
	BUILD_BUG_ON(sizeof(lif_reset_cpl_t) != 16);
	BUILD_BUG_ON(sizeof(struct seq_queue_init_cmd) != 64);
	BUILD_BUG_ON(sizeof(struct seq_queue_init_cpl) != 16);
	BUILD_BUG_ON(sizeof(struct seq_queue_control_cmd) != 64);
	BUILD_BUG_ON(sizeof(seq_queue_control_cpl_t) != 16);
	BUILD_BUG_ON(sizeof(struct seq_queue_batch_init_cmd) != 64);
	BUILD_BUG_ON(sizeof(struct seq_queue_batch_init_cpl) != 16);
	BUILD_BUG_ON(sizeof(struct seq_queue_batch_control_cmd) != 64);
	BUILD_BUG_ON(sizeof(seq_queue_batch_control_cpl_t) != 16);
	BUILD_BUG_ON(sizeof(struct seq_queue_init_complete_cmd) != 64);
	BUILD_BUG_ON(sizeof(struct seq_queue_init_complete_cpl) != 16);
	BUILD_BUG_ON(sizeof(struct crypto_key_update_cmd) != 64);
	BUILD_BUG_ON(sizeof(struct crypto_key_update_cpl) != 16);
	BUILD_BUG_ON(sizeof(struct adminq_init_cmd) != 64);
	BUILD_BUG_ON(sizeof(struct adminq_init_cpl) != 16);
	BUILD_BUG_ON(sizeof(struct notifyq_init_cmd) != 64);
	BUILD_BUG_ON(sizeof(struct notifyq_init_cpl) != 16);
	BUILD_BUG_ON(sizeof(struct notifyq_event) != 64);
	BUILD_BUG_ON(sizeof(struct reset_event) != 64);
	BUILD_BUG_ON(sizeof(struct heartbeat_event) != 64);
	BUILD_BUG_ON(sizeof(struct log_event) != 64);
	BUILD_BUG_ON(sizeof(struct notifyq_cmd) != 4);
	BUILD_BUG_ON(sizeof(union notifyq_cpl) != 64);
	BUILD_BUG_ON(sizeof(union adminq_cmd) != 64);
	BUILD_BUG_ON(sizeof(union adminq_cpl) != 16);
	BUILD_BUG_ON(sizeof(void *) != sizeof(uint64_t));
}

struct sonic_accel_ring {
	accel_ring_t accel_ring;
	const char *name;
	osal_atomic_int_t descs_inuse;
};

struct sonic_dev {
	union dev_info_regs __iomem *dev_info;
	union dev_cmd_regs __iomem *dev_cmd;
	struct doorbell __iomem *db_pages;
	dma_addr_t phy_db_pages;
	struct intr_ctrl __iomem *intr_ctrl;
	struct intr_status __iomem *intr_status;
	spinlock_t hbm_inuse_lock; /* for hbm_inuse */
	void __iomem *hbm_iomem_vaddr;
	dma_addr_t phy_hbm_pages;
	uint32_t hbm_npages;
	uint32_t hbm_nallocs;
	uint32_t hbm_nfrees;
	struct sonic_accel_ring ring_tbl[ACCEL_RING_ID_MAX];
};

struct cq_info {
	void *cq_desc;
	struct cq_info *next;
	unsigned int index;
	bool last;
};

struct queue;
struct admin_desc_info;
struct sonic_ev_list;
struct qcq;

typedef void (*admin_desc_cb)(struct queue *q, struct admin_desc_info *desc_info,
			struct cq_info *cq_info, void *cb_arg);

struct admin_desc_info {
	void *desc;
	struct admin_desc_info *next;
	unsigned int index;
	unsigned int left;
	admin_desc_cb cb;
	void *cb_arg;
};

#define QUEUE_NAME_MAX_SZ		(32)

struct queue {
	char name[QUEUE_NAME_MAX_SZ];
	struct sonic_dev *idev;
	struct lif *lif;
	struct per_core_resource *pc_res;
	unsigned int qid;
	unsigned int total_size;
	void *base;
	dma_addr_t base_pa;
	struct admin_desc_info *admin_info;
	struct admin_desc_info *admin_tail;
	struct admin_desc_info *admin_head;
	unsigned int num_descs;
	unsigned int desc_size;
	struct doorbell __iomem *db;
	unsigned int pindex;
	unsigned int pid;
	unsigned int qpos;
	unsigned int qtype;
	unsigned int ref_count;
	storage_seq_qgroup_t qgroup;

	/*
	 * Sequencer queue completions are unordered so desc usages
	 * are tracked differently.
	 */
	osal_atomic_int_t descs_inuse;
};

struct seq_queue_batch {
	struct hlist_node node;
	struct lif *lif;
	unsigned int pid;
	unsigned int base_qid;
	unsigned int curr_qid;
	unsigned int num_queues;
	unsigned int base_alloc_size;
	unsigned int curr_alloc_size;
	unsigned int per_q_num_descs;
	unsigned int per_q_desc_size;
	dma_addr_t base_pa;
	void *base_va;
	storage_seq_qgroup_t qgroup;
};

#define MAX_PER_CORE_CPDC_QUEUES 2
#define MAX_PER_CORE_CRYPTO_QUEUES 2
#define MAX_PER_QUEUE_STATUS_ENTRIES 4
#define MAX_PER_QUEUE_SQ_ENTRIES 512
#define MAX_PER_CORE_CPDC_SEQ_STATUS_QUEUES MAX_PER_QUEUE_SQ_ENTRIES
#define MAX_PER_CORE_CRYPTO_SEQ_STATUS_QUEUES MAX_PER_QUEUE_SQ_ENTRIES
#define MAX_PER_CORE_SEQ_STATUS_QUEUES			\
	((MAX_PER_CORE_CPDC_SEQ_STATUS_QUEUES <		\
	  MAX_PER_CORE_CRYPTO_SEQ_STATUS_QUEUES) ?	\
	MAX_PER_CORE_CRYPTO_SEQ_STATUS_QUEUES :		\
	MAX_PER_CORE_CPDC_SEQ_STATUS_QUEUES)
#define SONIC_MAX_CORES (32 < OSAL_MAX_CORES ? 32 : OSAL_MAX_CORES)
#define SONIC_DEFAULT_CORES (20 < OSAL_MAX_CORES ? 20 : OSAL_MAX_CORES)

struct queue_tracking {
	struct queue *queues;
	DECLARE_BITMAP(bmp, MAX_PER_CORE_SEQ_STATUS_QUEUES);
	spinlock_t lock;
	unsigned int num_queues;
	int next_free_id;
};

struct per_core_resource {
	bool initialized;
	bool reserved;
	int core_id;
	int idx;
	struct lif *lif;

	struct queue cp_seq_q;
	struct queue dc_seq_q;
	struct queue cpdc_seq_status_qs[MAX_PER_CORE_CPDC_SEQ_STATUS_QUEUES];
	struct queue_tracking cpdc_statusq_track;

	struct queue crypto_enc_seq_q;
	struct queue crypto_dec_seq_q;
	struct queue crypto_seq_status_qs[MAX_PER_CORE_CRYPTO_SEQ_STATUS_QUEUES];
	struct queue_tracking crypto_statusq_track;

  	struct intr intr;
	struct sonic_event_list *evl; /* top half event list */

	struct mem_pool *mpools[MPOOL_TYPE_MAX];
	struct batch_info *batch_info;
	struct pnso_api_stats api_stats;
};

struct res {
	struct per_core_resource *pc_res[SONIC_MAX_CORES];
	int core_to_res_map[OSAL_MAX_CORES];
	spinlock_t lock;
	DECLARE_BITMAP(pc_res_bmp, OSAL_MAX_CORES);
};

struct cq {
	void *base;
	dma_addr_t base_pa;
	struct lif *lif;
	struct cq_info *info;
	struct cq_info *tail;
	struct queue *bound_q;
	struct intr *bound_intr;
	unsigned int num_descs;
	unsigned int desc_size;
	bool done_color;
};

int sonic_dev_setup(struct sonic_dev *idev, struct sonic_dev_bar bars[],
		    unsigned int num_bars);
void sonic_dev_teardown(struct sonic *ionic);

union dev_cmd; //Need to remove it
void sonic_dev_cmd_go(struct sonic_dev *idev, union dev_cmd *cmd);
u8 sonic_dev_cmd_status(struct sonic_dev *idev);
bool sonic_dev_cmd_done(struct sonic_dev *idev);
void sonic_dev_cmd_cpl(struct sonic_dev *idev, void *mem);
void sonic_dev_cmd_reset(struct sonic_dev *idev);
void sonic_dev_cmd_hang_notify(struct sonic_dev *idev, uint32_t lif_index);
void sonic_dev_cmd_identify(struct sonic_dev *idev, u16 ver);
void sonic_dev_cmd_lif_identify(struct sonic_dev *idev, u16 ver);
void sonic_dev_cmd_lif_init(struct sonic_dev *idev, u32 lif_index);
void sonic_dev_cmd_lif_reset(struct sonic_dev *idev, u32 lif_index);
void sonic_dev_cmd_adminq_init(struct sonic_dev *idev, struct qcq *qcq,
			       unsigned int lif_index, unsigned int intr_index);
int sonic_crypto_key_index_update(const void *key1,
				  const void *key2,
				  uint32_t key_size,
				  uint32_t key_index);

char *sonic_dev_asic_name(u8 asic_type);
struct doorbell __iomem *sonic_db_map(struct sonic_dev *idev, struct queue *q);

int sonic_intr_init(struct sonic_dev *idev, struct intr *intr,
		    unsigned long index);
void sonic_intr_clean(struct intr *intr);
void sonic_intr_mask_on_assertion(struct intr *intr);
void sonic_intr_return_credits(struct intr *intr, unsigned int credits,
			       bool unmask, bool reset_timer);
void sonic_intr_mask(struct intr *intr, bool mask);
void sonic_intr_coal_set(struct intr *intr, u32 coal_usecs);
int sonic_cq_init(struct lif *lif, struct cq *cq, struct intr *intr,
		  unsigned int num_descs, size_t desc_size);
int sonic_cq_reinit(struct cq *cq);
void sonic_cq_map(struct cq *cq, void *base, dma_addr_t base_pa);
void sonic_cq_bind(struct cq *cq, struct queue *q);
typedef bool (*sonic_cq_cb)(struct cq *cq, struct cq_info *cq_info,
			    void *cb_arg);
unsigned int sonic_cq_service(struct cq *cq, unsigned int work_to_do,
			      sonic_cq_cb cb, void *cb_arg);

int sonic_q_alloc(struct lif *lif, struct queue *q,
		  unsigned int num_descs, unsigned int desc_size,
		  bool do_alloc_descs);
void sonic_q_free(struct lif *lif, struct queue *q);
int sonic_q_init(struct lif *lif, struct sonic_dev *idev, struct queue *q,
		 unsigned int index, const char *base, unsigned int num_descs,
		 size_t desc_size, unsigned int pid);
int sonic_q_reinit(struct queue *q);
void sonic_q_map(struct queue *q, void *base, dma_addr_t base_pa);
struct admin_desc_info *sonic_q_post(struct queue *q, bool ring_doorbell,
		admin_desc_cb cb, void *cb_arg);
void sonic_q_rewind(struct queue *q, struct admin_desc_info *start);
unsigned int sonic_q_space_avail(struct queue *q);
bool sonic_q_has_space(struct queue *q, unsigned int want);
void sonic_q_service(struct queue *q, struct cq_info *cq_info,
		     unsigned int stop_index);
void *sonic_q_consume_entry(struct queue *q, uint32_t *index);
int sonic_q_unconsume(struct queue *q, uint32_t count);
void sonic_q_ringdb(struct queue *q, uint32_t index);
u64 sonic_q_ringdb_data(struct queue *q, uint32_t index);

#endif /* _SONIC_DEV_H_ */
