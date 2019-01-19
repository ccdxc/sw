
#ifndef __ETH_LIF_H__
#define __ETH_LIF_H__

/* Supply these for ionic_if.h */
#define BIT(n)                  (1 << n)
#define TEST_BIT(x, n)          ((x) & (1 << n))
#define u8 uint8_t
#define u16 uint16_t
#define u32 uint32_t
#define u64 uint64_t
#define dma_addr_t uint64_t

#include "platform/drivers/common/ionic_if.h"

#pragma pack(push, 1)

/**
 * ETH PF Devcmd Region
 */

union dev_cmd {
    u32 words[16];
    struct admin_cmd cmd;
    struct nop_cmd nop;
    struct reset_cmd reset;
    struct identify_cmd identify;
    struct lif_init_cmd lif_init;
    struct adminq_init_cmd adminq_init;
    struct txq_init_cmd txq_init;
    struct rxq_init_cmd rxq_init;
    struct notifyq_init_cmd notifyq_init;
    struct features_cmd features;
    struct q_enable_cmd q_enable;
    struct q_disable_cmd q_disable;
    struct station_mac_addr_get_cmd station_mac_addr_get;
    struct mtu_set_cmd mtu_set;
    struct rx_mode_set_cmd rx_mode_set;
    struct rx_filter_add_cmd rx_filter_add;
    struct rx_filter_del_cmd rx_filter_del;
    struct rss_hash_set_cmd rss_hash_set;
    struct rss_indir_set_cmd rss_indir_set;
};

union dev_cmd_comp {
    u32 words[4];
    u8 status;
    struct admin_comp comp;
    struct nop_comp nop;
    struct reset_comp reset;
    struct identify_comp identify;
    struct lif_init_comp lif_init;
    struct adminq_init_comp adminq_init;
    struct txq_init_comp txq_init;
    struct rxq_init_comp rxq_init;
    struct notifyq_init_comp notifyq_init;
    struct features_comp features;
    q_enable_comp q_enable;
    q_disable_comp q_disable;
    struct station_mac_addr_get_comp station_mac_addr_get;
    mtu_set_comp mtu_set;
    rx_mode_set_comp rx_mode_set;
    struct rx_filter_add_comp rx_filter_add;
    rx_filter_del_comp rx_filter_del;
    rss_hash_set_comp rss_hash_set;
    rss_indir_set_comp rss_indir_set;
};

#pragma pack(pop)

struct dev_cmd_regs {
    u32 signature;
    u32 done;
    union dev_cmd cmd;
    union dev_cmd_comp comp;
    uint8_t data[2048] __attribute__((aligned (2048)));
};

static_assert(sizeof(struct dev_cmd_regs) == 4096);
static_assert((offsetof(struct dev_cmd_regs, cmd)  % 4) == 0);
static_assert(sizeof(((struct dev_cmd_regs*)0)->cmd) == 64);
static_assert((offsetof(struct dev_cmd_regs, comp) % 4) == 0);
static_assert(sizeof(((struct dev_cmd_regs*)0)->comp) == 16);
static_assert((offsetof(struct dev_cmd_regs, data) % 4) == 0);

#endif  /* __ETH_LIF_H__ */
