#ifndef _IONIC_STATS_H_
#define _IONIC_STATS_H_

#define IONIC_STAT_TO_OFFSET(type, stat_name) (offsetof(type, stat_name))

#define IONIC_STAT_DESC(type, stat_name) { \
	.name = #stat_name, \
	.offset = IONIC_STAT_TO_OFFSET(type, stat_name) \
}

#define IONIC_LIF_STAT_DESC(stat_name) \
	IONIC_STAT_DESC(struct lif_stats, stat_name)

#define IONIC_TX_STAT_DESC(stat_name) \
	IONIC_STAT_DESC(struct tx_stats, stat_name)

#define IONIC_TX_Q_STAT_DESC(stat_name) \
	IONIC_STAT_DESC(struct queue, stat_name)

#define IONIC_CQ_STAT_DESC(stat_name) \
	IONIC_STAT_DESC(struct cq, stat_name)

#define IONIC_INTR_STAT_DESC(stat_name) \
	IONIC_STAT_DESC(struct intr, stat_name)

#define IONIC_NAPI_STAT_DESC(stat_name) \
	IONIC_STAT_DESC(struct napi_stats, stat_name)

/* Interface structure for a particalar stats group */
struct ionic_stats_group_intf {
	void (*get_strings)(struct lif *lif, u8 **buf);
	void (*get_values)(struct lif *lif, u64 **buf);
	u64 (*get_count)(struct lif *lif);
};

extern const struct ionic_stats_group_intf ionic_stats_groups[];
extern const int ionic_num_stats_grps;

#define IONIC_READ_STAT64(base_ptr, desc_ptr) \
	(*((u64 *)(((u8 *)(base_ptr)) + (desc_ptr)->offset)))

struct ionic_stat_desc {
	char name[ETH_GSTRING_LEN];
	u64 offset;
};

#endif // _IONIC_STATS_H_
