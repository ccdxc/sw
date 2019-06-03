// SPDX-License-Identifier: GPL-2.0
/* Copyright(c) 2017 - 2019 Pensando Systems, Inc */

#include <linux/module.h>
#include <linux/netdevice.h>

#include "ionic.h"
#include "ionic_api.h"
#include "ionic_bus.h"
#include "ionic_lif.h"
#include "ionic_ethtool.h"
#include "ionic_stats.h"

static const char ionic_priv_flags_strings[][ETH_GSTRING_LEN] = {
#define PRIV_F_SW_DBG_STATS		BIT(0)
	"sw-dbg-stats",
};
#define PRIV_FLAGS_COUNT ARRAY_SIZE(ionic_priv_flags_strings)

static void ionic_get_stats_strings(struct lif *lif, u8 *buf)
{
	u32 i;

	for (i = 0; i < ionic_num_stats_grps; i++)
		ionic_stats_groups[i].get_strings(lif, &buf);
}

static void ionic_get_stats(struct net_device *netdev,
			    struct ethtool_stats *stats, u64 *buf)
{
	struct lif *lif;
	u32 i;

	lif = netdev_priv(netdev);

	memset(buf, 0, stats->n_stats * sizeof(*buf));
	for (i = 0; i < ionic_num_stats_grps; i++)
		ionic_stats_groups[i].get_values(lif, &buf);
}

static int ionic_get_stats_count(struct lif *lif)
{
	int i, num_stats = 0;

	for (i = 0; i < ionic_num_stats_grps; i++)
		num_stats += ionic_stats_groups[i].get_count(lif);

	return num_stats;
}

static int ionic_get_sset_count(struct net_device *netdev, int sset)
{
	struct lif *lif = netdev_priv(netdev);
	int count = 0;

	switch (sset) {
	case ETH_SS_STATS:
		count = ionic_get_stats_count(lif);
		break;
	case ETH_SS_TEST:
		break;
	case ETH_SS_PRIV_FLAGS:
		count = PRIV_FLAGS_COUNT;
		break;
	default:
		return -EOPNOTSUPP;
	}
	return count;
}

static void ionic_get_strings(struct net_device *netdev,
			      u32 sset, u8 *buf)
{
	struct lif *lif = netdev_priv(netdev);

	switch (sset) {
	case ETH_SS_STATS:
		ionic_get_stats_strings(lif, buf);
		break;
	case ETH_SS_PRIV_FLAGS:
		memcpy(buf, ionic_priv_flags_strings,
		       PRIV_FLAGS_COUNT * ETH_GSTRING_LEN);
		break;
	case ETH_SS_TEST:
		// IONIC_TODO
	default:
		netdev_err(netdev, "Invalid sset %d\n", sset);
	}
}

static void ionic_get_drvinfo(struct net_device *netdev,
			      struct ethtool_drvinfo *drvinfo)
{
	struct lif *lif = netdev_priv(netdev);
	struct ionic *ionic = lif->ionic;
	struct ionic_dev *idev = &ionic->idev;

	strlcpy(drvinfo->driver, DRV_NAME, sizeof(drvinfo->driver));
	strlcpy(drvinfo->version, DRV_VERSION, sizeof(drvinfo->version));
	strlcpy(drvinfo->fw_version, idev->dev_info.fw_version,
		sizeof(drvinfo->fw_version));
	strlcpy(drvinfo->bus_info, ionic_bus_info(ionic),
		sizeof(drvinfo->bus_info));
}

static int ionic_get_link_ksettings(struct net_device *netdev,
				    struct ethtool_link_ksettings *ks)
{
	struct lif *lif = netdev_priv(netdev);
	struct ionic_dev *idev = &lif->ionic->idev;
	int fake_port_type = 0;

	ethtool_link_ksettings_zero_link_mode(ks, supported);
	ethtool_link_ksettings_zero_link_mode(ks, advertising);

	if (ionic_is_mnic(lif->ionic)) {
		ethtool_link_ksettings_add_link_mode(ks, supported, Backplane);
		ethtool_link_ksettings_add_link_mode(ks, advertising, Backplane);
	} else {
		ethtool_link_ksettings_add_link_mode(ks, supported, FIBRE);
		ethtool_link_ksettings_add_link_mode(ks, advertising, FIBRE);

		if (ionic_is_pf(lif->ionic)) {
			ethtool_link_ksettings_add_link_mode(ks, supported,
							     Autoneg);
			ethtool_link_ksettings_add_link_mode(ks, advertising,
							     Autoneg);
		}
	}

	switch (le16_to_cpu(idev->port_info->status.xcvr.pid)) {
		/* Copper */
	case XCVR_PID_QSFP_100G_CR4:
		ethtool_link_ksettings_add_link_mode(ks, supported,
						     100000baseCR4_Full);
		ethtool_link_ksettings_add_link_mode(ks, advertising,
						     100000baseCR4_Full);
fake_port_type++;
		break;
	case XCVR_PID_QSFP_40GBASE_CR4:
		ethtool_link_ksettings_add_link_mode(ks, supported,
						     40000baseCR4_Full);
		ethtool_link_ksettings_add_link_mode(ks, advertising,
						     40000baseCR4_Full);
fake_port_type++;
		break;
#ifdef HAVE_ETHTOOL_25G_BITS
	case XCVR_PID_SFP_25GBASE_CR_S:
	case XCVR_PID_SFP_25GBASE_CR_L:
	case XCVR_PID_SFP_25GBASE_CR_N:
		ethtool_link_ksettings_add_link_mode(ks, supported,
						     25000baseCR_Full);
		ethtool_link_ksettings_add_link_mode(ks, advertising,
						     25000baseCR_Full);
fake_port_type++;
		break;
#endif
	case XCVR_PID_SFP_10GBASE_AOC:
	case XCVR_PID_SFP_10GBASE_CU:
#ifdef HAVE_ETHTOOL_NEW_10G_BITS
		ethtool_link_ksettings_add_link_mode(ks, supported,
						     10000baseCR_Full);
		ethtool_link_ksettings_add_link_mode(ks, advertising,
						     10000baseCR_Full);
#else
		ethtool_link_ksettings_add_link_mode(ks, supported,
						     10000baseT_Full);
		ethtool_link_ksettings_add_link_mode(ks, advertising,
						     10000baseT_Full);
#endif
fake_port_type++;
		break;

		/* Fibre */
	case XCVR_PID_QSFP_100G_SR4:
	case XCVR_PID_QSFP_100G_AOC:
		ethtool_link_ksettings_add_link_mode(ks, supported,
						     100000baseSR4_Full);
		ethtool_link_ksettings_add_link_mode(ks, advertising,
						     100000baseSR4_Full);
		break;
	case XCVR_PID_QSFP_100G_LR4:
		ethtool_link_ksettings_add_link_mode(ks, supported,
						     100000baseLR4_ER4_Full);
		ethtool_link_ksettings_add_link_mode(ks, advertising,
						     100000baseLR4_ER4_Full);
		break;
	case XCVR_PID_QSFP_100G_ER4:
		ethtool_link_ksettings_add_link_mode(ks, supported,
						     100000baseLR4_ER4_Full);
		ethtool_link_ksettings_add_link_mode(ks, advertising,
						     100000baseLR4_ER4_Full);
		break;
	case XCVR_PID_QSFP_40GBASE_SR4:
	case XCVR_PID_QSFP_40GBASE_AOC:
		ethtool_link_ksettings_add_link_mode(ks, supported,
						     40000baseSR4_Full);
		ethtool_link_ksettings_add_link_mode(ks, advertising,
						     40000baseSR4_Full);
		break;
	case XCVR_PID_QSFP_40GBASE_LR4:
		ethtool_link_ksettings_add_link_mode(ks, supported,
						     40000baseLR4_Full);
		ethtool_link_ksettings_add_link_mode(ks, advertising,
						     40000baseLR4_Full);
		break;
#ifdef HAVE_ETHTOOL_25G_BITS
	case XCVR_PID_SFP_25GBASE_SR:
	case XCVR_PID_SFP_25GBASE_AOC:
		ethtool_link_ksettings_add_link_mode(ks, supported,
						     25000baseSR_Full);
		ethtool_link_ksettings_add_link_mode(ks, advertising,
						     25000baseSR_Full);
		break;
#endif
#ifdef HAVE_ETHTOOL_NEW_10G_BITS
	case XCVR_PID_SFP_10GBASE_SR:
		ethtool_link_ksettings_add_link_mode(ks, supported,
						     10000baseSR_Full);
		ethtool_link_ksettings_add_link_mode(ks, advertising,
						     10000baseSR_Full);
		break;
	case XCVR_PID_SFP_10GBASE_LR:
		ethtool_link_ksettings_add_link_mode(ks, supported,
						     10000baseLR_Full);
		ethtool_link_ksettings_add_link_mode(ks, advertising,
						     10000baseLR_Full);
		break;
	case XCVR_PID_SFP_10GBASE_LRM:
		ethtool_link_ksettings_add_link_mode(ks, supported,
						     10000baseLRM_Full);
		ethtool_link_ksettings_add_link_mode(ks, advertising,
						     10000baseLRM_Full);
		break;
	case XCVR_PID_SFP_10GBASE_ER:
		ethtool_link_ksettings_add_link_mode(ks, supported,
						     10000baseER_Full);
		ethtool_link_ksettings_add_link_mode(ks, advertising,
						     10000baseER_Full);
		break;
#else
	case XCVR_PID_SFP_10GBASE_SR:
	case XCVR_PID_SFP_10GBASE_LR:
	case XCVR_PID_SFP_10GBASE_LRM:
	case XCVR_PID_SFP_10GBASE_ER:
		ethtool_link_ksettings_add_link_mode(ks, supported,
						     10000baseT_Full);
		ethtool_link_ksettings_add_link_mode(ks, advertising,
						     10000baseT_Full);
		break;
#endif
	case XCVR_PID_QSFP_100G_ACC:
	case XCVR_PID_QSFP_40GBASE_ER4:
	case XCVR_PID_SFP_25GBASE_LR:
	case XCVR_PID_SFP_25GBASE_ER:
		dev_info(lif->ionic->dev, "no decode bits for xcvr type pid=%d / 0x%x\n",
			 idev->port_info->status.xcvr.pid,
			 idev->port_info->status.xcvr.pid);
		break;
	case XCVR_PID_UNKNOWN:
		if (ionic_is_mnic(lif->ionic))
			break;
		/* fall through */
	default:
		dev_info(lif->ionic->dev, "unknown xcvr type pid=%d / 0x%x\n",
			 idev->port_info->status.xcvr.pid,
			 idev->port_info->status.xcvr.pid);
		break;
	}

	ethtool_link_ksettings_add_link_mode(ks, supported, Pause);
	if (idev->port_info->config.pause_type)
		ethtool_link_ksettings_add_link_mode(ks, advertising, Pause);

#ifdef ETHTOOL_LINK_MODE_FEC_NONE_BIT
	if (idev->port_info->config.fec_type == PORT_FEC_TYPE_FC)
		ethtool_link_ksettings_add_link_mode(ks, advertising, FEC_BASER);
	else if (idev->port_info->config.fec_type == PORT_FEC_TYPE_RS)
		ethtool_link_ksettings_add_link_mode(ks, advertising, FEC_RS);
	else
		ethtool_link_ksettings_add_link_mode(ks, advertising, FEC_NONE);
#endif

	if (idev->port_info->status.xcvr.phy == PHY_TYPE_COPPER ||
	    fake_port_type) {
		ks->base.port = PORT_DA;
	} else if (idev->port_info->status.xcvr.phy == PHY_TYPE_FIBER) {
		ks->base.port = PORT_FIBRE;
	} else {
		ks->base.port = PORT_OTHER;
	}

	ks->base.speed = le32_to_cpu(lif->info->status.link_speed);

	if (idev->port_info->config.an_enable)
		ks->base.autoneg = AUTONEG_ENABLE;

	if (le16_to_cpu(lif->info->status.link_status))
		ks->base.duplex = DUPLEX_FULL;
	else
		ks->base.duplex = DUPLEX_UNKNOWN;

	return 0;
}

static int ionic_set_link_ksettings(struct net_device *netdev,
				    const struct ethtool_link_ksettings *ks)
{
	struct lif *lif = netdev_priv(netdev);
	struct ionic *ionic = lif->ionic;
	struct ionic_dev *idev = &lif->ionic->idev;
#ifdef ETHTOOL_LINK_MODE_FEC_NONE_BIT
	u32 req_rs, req_b;
	u8 fec_type = PORT_FEC_TYPE_NONE;
#endif
	int err = 0;

	/* set autoneg */
	if (ks->base.autoneg != idev->port_info->config.an_enable) {
		idev->port_info->config.an_enable = ks->base.autoneg;
		mutex_lock(&ionic->dev_cmd_lock);
		ionic_dev_cmd_port_autoneg(idev, ks->base.autoneg);
		err = ionic_dev_cmd_wait(ionic, devcmd_timeout);
		mutex_unlock(&ionic->dev_cmd_lock);
		if (err)
			return err;
	}

	/* set speed */
	if (ks->base.speed != le32_to_cpu(idev->port_info->config.speed)) {
		idev->port_info->config.speed = cpu_to_le32(ks->base.speed);
		mutex_lock(&ionic->dev_cmd_lock);
		ionic_dev_cmd_port_speed(idev, ks->base.speed);
		err = ionic_dev_cmd_wait(ionic, devcmd_timeout);
		mutex_unlock(&ionic->dev_cmd_lock);
		if (err)
			return err;
	}

#ifdef ETHTOOL_LINK_MODE_FEC_NONE_BIT
	/* set FEC */
	req_rs = ethtool_link_ksettings_test_link_mode(ks, advertising, FEC_RS);
	req_b = ethtool_link_ksettings_test_link_mode(ks, advertising, FEC_BASER);
	if (req_rs && req_b) {
		netdev_info(netdev, "Only select one FEC mode at a time\n");
		return -EINVAL;

	} else if (req_b &&
		   idev->port_info->config.fec_type != PORT_FEC_TYPE_FC) {
		fec_type = PORT_FEC_TYPE_FC;
	} else if (req_rs &&
		   idev->port_info->config.fec_type != PORT_FEC_TYPE_RS) {
		fec_type = PORT_FEC_TYPE_RS;
	} else if (!(req_rs | req_b) &&
		 idev->port_info->config.fec_type != PORT_FEC_TYPE_NONE) {
		fec_type = PORT_FEC_TYPE_NONE;
	}

	idev->port_info->config.fec_type = fec_type;
	mutex_lock(&ionic->dev_cmd_lock);
	ionic_dev_cmd_port_fec(idev, PORT_FEC_TYPE_NONE);
	err = ionic_dev_cmd_wait(ionic, devcmd_timeout);
	mutex_unlock(&ionic->dev_cmd_lock);
	if (err)
		return err;
#endif

	return 0;
}

static void ionic_get_pauseparam(struct net_device *netdev,
				 struct ethtool_pauseparam *pause)
{
	struct lif *lif = netdev_priv(netdev);
	struct ionic_dev *idev = &lif->ionic->idev;
	uint8_t pause_type = idev->port_info->config.pause_type;

	pause->autoneg = idev->port_info->config.an_enable;

	if (pause_type) {
		pause->rx_pause = pause_type & IONIC_PAUSE_F_RX ? 1 : 0;
		pause->tx_pause = pause_type & IONIC_PAUSE_F_TX ? 1 : 0;
	}
}

static int ionic_set_pauseparam(struct net_device *netdev,
				struct ethtool_pauseparam *pause)
{
	struct lif *lif = netdev_priv(netdev);
	struct ionic *ionic = lif->ionic;
	struct ionic_dev *idev = &lif->ionic->idev;

	u32 requested_pause;
	u32 cur_autoneg;
	int err;

	cur_autoneg = idev->port_info->config.an_enable ? AUTONEG_ENABLE :
								AUTONEG_DISABLE;
	if (pause->autoneg != cur_autoneg) {
		netdev_info(netdev, "Please use 'ethtool -s ...' to change autoneg\n");
		return -EOPNOTSUPP;
	}

	/* change both at the same time */
	requested_pause = PORT_PAUSE_TYPE_LINK;
	if (pause->rx_pause)
		requested_pause |= IONIC_PAUSE_F_RX;
	if (pause->tx_pause)
		requested_pause |= IONIC_PAUSE_F_TX;

	if (requested_pause == idev->port_info->config.pause_type)
		return 0;

	idev->port_info->config.pause_type = requested_pause;

	mutex_lock(&ionic->dev_cmd_lock);
	ionic_dev_cmd_port_pause(idev, requested_pause);
	err = ionic_dev_cmd_wait(ionic, devcmd_timeout);
	mutex_unlock(&ionic->dev_cmd_lock);
	if (err)
		return err;

	return 0;
}

static int ionic_get_coalesce(struct net_device *netdev,
			      struct ethtool_coalesce *coalesce)
{
	struct lif *lif = netdev_priv(netdev);

	coalesce->tx_coalesce_usecs = lif->tx_coalesce_usecs;
	coalesce->rx_coalesce_usecs = lif->rx_coalesce_usecs;

	return 0;
}

static int ionic_set_coalesce(struct net_device *netdev,
			      struct ethtool_coalesce *coalesce)
{
	struct lif *lif = netdev_priv(netdev);
	struct ionic_dev *idev = &lif->ionic->idev;
	struct identity *ident = &lif->ionic->ident;
	struct qcq *qcq;
	u32 tx_coal, rx_coal;
	unsigned int i;

	if (coalesce->rx_max_coalesced_frames ||
	    coalesce->rx_coalesce_usecs_irq ||
	    coalesce->rx_max_coalesced_frames_irq ||
	    coalesce->tx_max_coalesced_frames ||
	    coalesce->tx_coalesce_usecs_irq ||
	    coalesce->tx_max_coalesced_frames_irq ||
	    coalesce->stats_block_coalesce_usecs ||
	    coalesce->use_adaptive_rx_coalesce ||
	    coalesce->use_adaptive_tx_coalesce ||
	    coalesce->pkt_rate_low ||
	    coalesce->rx_coalesce_usecs_low ||
	    coalesce->rx_max_coalesced_frames_low ||
	    coalesce->tx_coalesce_usecs_low ||
	    coalesce->tx_max_coalesced_frames_low ||
	    coalesce->pkt_rate_high ||
	    coalesce->rx_coalesce_usecs_high ||
	    coalesce->rx_max_coalesced_frames_high ||
	    coalesce->tx_coalesce_usecs_high ||
	    coalesce->tx_max_coalesced_frames_high ||
	    coalesce->rate_sample_interval)
		return -EINVAL;

	if (ident->dev.intr_coal_div == 0)
		return -EIO;

	/* Convert from usecs to device units */
	tx_coal = coalesce->tx_coalesce_usecs *
		  le32_to_cpu(ident->dev.intr_coal_mult) /
		  le32_to_cpu(ident->dev.intr_coal_div);
	rx_coal = coalesce->rx_coalesce_usecs *
		  le32_to_cpu(ident->dev.intr_coal_mult) /
		  le32_to_cpu(ident->dev.intr_coal_div);

	if (tx_coal > INTR_CTRL_COAL_MAX || rx_coal > INTR_CTRL_COAL_MAX)
		return -ERANGE;

	if (coalesce->tx_coalesce_usecs != lif->tx_coalesce_usecs) {
		for (i = 0; i < lif->nxqs; i++) {
			qcq = lif->txqcqs[i].qcq;
			ionic_intr_coal_init(idev->intr_ctrl,
					     qcq->intr.index,
					     tx_coal);
		}
		lif->tx_coalesce_usecs = coalesce->tx_coalesce_usecs;
	}

	if (coalesce->rx_coalesce_usecs != lif->rx_coalesce_usecs) {
		for (i = 0; i < lif->nxqs; i++) {
			qcq = lif->rxqcqs[i].qcq;
			ionic_intr_coal_init(idev->intr_ctrl,
					     qcq->intr.index,
					     rx_coal);
		}
		lif->rx_coalesce_usecs = coalesce->rx_coalesce_usecs;
	}

	return 0;
}

static void ionic_get_ringparam(struct net_device *netdev,
				struct ethtool_ringparam *ring)
{
	struct lif *lif = netdev_priv(netdev);

	ring->tx_max_pending = IONIC_MAX_TXRX_DESC;
	ring->tx_pending = lif->ntxq_descs;
	ring->rx_max_pending = IONIC_MAX_TXRX_DESC;
	ring->rx_pending = lif->nrxq_descs;
}

static int ionic_set_ringparam(struct net_device *netdev,
			       struct ethtool_ringparam *ring)
{
	struct lif *lif = netdev_priv(netdev);
	bool running;
	int i, j;

	if (ring->rx_mini_pending || ring->rx_jumbo_pending) {
		netdev_info(netdev, "Changing jumbo or mini descriptors not supported\n");
		return -EINVAL;
	}

	i = ring->tx_pending & (ring->tx_pending - 1);
	j = ring->rx_pending & (ring->rx_pending - 1);
	if (i || j) {
		netdev_info(netdev, "Descriptor count must be a power of 2\n");
		return -EINVAL;
	}

	if (ring->tx_pending > IONIC_MAX_TXRX_DESC ||
	    ring->tx_pending < IONIC_MIN_TXRX_DESC ||
	    ring->rx_pending > IONIC_MAX_TXRX_DESC ||
	    ring->rx_pending < IONIC_MIN_TXRX_DESC) {
		netdev_info(netdev, "Descriptors count must be in the range [%d-%d]\n",
			    IONIC_MIN_TXRX_DESC, IONIC_MAX_TXRX_DESC);
		return -EINVAL;
	}

	/* if nothing to do return success */
	if (ring->tx_pending == lif->ntxq_descs &&
	    ring->rx_pending == lif->nrxq_descs)
		return 0;

	while (test_and_set_bit(LIF_QUEUE_RESET, lif->state))
		usleep_range(200, 400);

	running = test_bit(LIF_UP, lif->state);
	if (running)
		ionic_stop(netdev);

	lif->ntxq_descs = ring->tx_pending;
	lif->nrxq_descs = ring->rx_pending;

	if (running)
		ionic_open(netdev);
	clear_bit(LIF_QUEUE_RESET, lif->state);

	return 0;
}

static void ionic_get_channels(struct net_device *netdev,
			       struct ethtool_channels *ch)
{
	struct lif *lif = netdev_priv(netdev);

	/* report maximum channels */
	ch->max_combined = lif->ionic->ntxqs_per_lif;

	/* report current channels */
	ch->combined_count = lif->nxqs;
}

static int ionic_set_channels(struct net_device *netdev,
			      struct ethtool_channels *ch)
{
	struct lif *lif = netdev_priv(netdev);
	bool running;

	if (!ch->combined_count || ch->other_count ||
	    ch->rx_count || ch->tx_count)
		return -EINVAL;

	if (ch->combined_count > lif->ionic->ntxqs_per_lif)
		return -EINVAL;

	if (ch->combined_count == lif->nxqs)
		return 0;

	while (test_and_set_bit(LIF_QUEUE_RESET, lif->state))
		usleep_range(200, 400);

	running = test_bit(LIF_UP, lif->state);
	if (running)
		ionic_stop(netdev);

	lif->nxqs = ch->combined_count;

	if (running)
		ionic_open(netdev);
	clear_bit(LIF_QUEUE_RESET, lif->state);

	return 0;
}

static int ionic_get_rxnfc(struct net_device *netdev,
			   struct ethtool_rxnfc *info, u32 *rules)
{
	struct lif *lif = netdev_priv(netdev);
	int err = 0;

	switch (info->cmd) {
	case ETHTOOL_GRXRINGS:
		info->data = lif->nxqs;
		break;
	default:
		netdev_err(netdev, "Command parameter %d is not supported\n",
			   info->cmd);
		err = -EOPNOTSUPP;
	}

	return err;
}

static u32 ionic_get_rxfh_indir_size(struct net_device *netdev)
{
	struct lif *lif = netdev_priv(netdev);

	return le16_to_cpu(lif->ionic->ident.lif.eth.rss_ind_tbl_sz);
}

static u32 ionic_get_rxfh_key_size(struct net_device *netdev)
{
	return IONIC_RSS_HASH_KEY_SIZE;
}

#ifdef HAVE_RXFH_HASHFUNC
static int ionic_get_rxfh(struct net_device *netdev, u32 *indir, u8 *key,
			  u8 *hfunc)
#else
static int ionic_get_rxfh(struct net_device *netdev, u32 *indir, u8 *key)
#endif
{
	struct lif *lif = netdev_priv(netdev);
	unsigned int i, tbl_sz;

	if (indir) {
		tbl_sz = le16_to_cpu(lif->ionic->ident.lif.eth.rss_ind_tbl_sz);
		for (i = 0; i < tbl_sz; i++)
			indir[i] = lif->rss_ind_tbl[i];
	}

	if (key)
		memcpy(key, lif->rss_hash_key, IONIC_RSS_HASH_KEY_SIZE);

#ifdef HAVE_RXFH_HASHFUNC
	if (hfunc)
		*hfunc = ETH_RSS_HASH_TOP;
#endif

	return 0;
}

#ifdef HAVE_RXFH_HASHFUNC
static int ionic_set_rxfh(struct net_device *netdev, const u32 *indir,
			  const u8 *key, const u8 hfunc)
#else
static int ionic_set_rxfh(struct net_device *netdev, const u32 *indir,
			  const u8 *key)
#endif
{
	struct lif *lif = netdev_priv(netdev);
	int err;

#ifdef HAVE_RXFH_HASHFUNC
	if (hfunc != ETH_RSS_HASH_NO_CHANGE && hfunc != ETH_RSS_HASH_TOP)
		return -EOPNOTSUPP;
#endif

	err = ionic_lif_rss_config(lif, lif->rss_types, key, indir);
	if (err)
		return err;

	return 0;
}

static u32 ionic_get_priv_flags(struct net_device *netdev)
{
	u32 priv_flags = 0;
#ifdef IONIC_DEBUG_STATS
	struct lif *lif = netdev_priv(netdev);

	if (test_bit(LIF_SW_DEBUG_STATS, lif->state))
		priv_flags |= PRIV_F_SW_DBG_STATS;
#endif

	return priv_flags;
}

static int ionic_set_priv_flags(struct net_device *netdev, u32 priv_flags)
{
	struct lif *lif = netdev_priv(netdev);
	u32 flags = lif->flags;

#ifdef IONIC_DEBUG_STATS
	clear_bit(LIF_SW_DEBUG_STATS, lif->state);
	if (priv_flags & PRIV_F_SW_DBG_STATS)
		set_bit(LIF_SW_DEBUG_STATS, lif->state);
#endif

	if (flags != lif->flags)
		lif->flags = flags;

	return 0;
}

static int ionic_set_tunable(struct net_device *dev,
			     const struct ethtool_tunable *tuna,
			     const void *data)
{
	struct lif *lif = netdev_priv(dev);

	switch (tuna->id) {
	case ETHTOOL_RX_COPYBREAK:
		lif->rx_copybreak = *(u32 *)data;
		break;
	default:
		return -EOPNOTSUPP;
	}

	return 0;
}

static int ionic_get_tunable(struct net_device *netdev,
			     const struct ethtool_tunable *tuna, void *data)
{
	struct lif *lif = netdev_priv(netdev);

	switch (tuna->id) {
	case ETHTOOL_RX_COPYBREAK:
		*(u32 *)data = lif->rx_copybreak;
		break;
	default:
		return -EOPNOTSUPP;
	}

	return 0;
}

static int ionic_get_module_info(struct net_device *netdev,
				 struct ethtool_modinfo *modinfo)

{
	struct lif *lif = netdev_priv(netdev);
	struct ionic_dev *idev = &lif->ionic->idev;
	struct xcvr_status *xcvr;

	if (ionic_is_mnic(lif->ionic))
		return 0;

	xcvr = &idev->port_info->status.xcvr;

	/* report the module data type and length */
	switch (xcvr->sprom[0]) {
	case 0x03: /* SFP */
		modinfo->type = ETH_MODULE_SFF_8079;
		modinfo->eeprom_len = ETH_MODULE_SFF_8079_LEN;
		break;
	case 0x0D: /* QSFP */
	case 0x11: /* QSFP28 */
		modinfo->type = ETH_MODULE_SFF_8436;
		modinfo->eeprom_len = ETH_MODULE_SFF_8436_LEN;
		break;
	default:
		netdev_info(netdev, "unknown xcvr type 0x%02x\n",
			    xcvr->sprom[0]);
		break;
	}

	return 0;
}

static int ionic_get_module_eeprom(struct net_device *netdev,
				   struct ethtool_eeprom *ee,
				   u8 *data)
{
	struct lif *lif = netdev_priv(netdev);
	struct ionic_dev *idev = &lif->ionic->idev;
	struct xcvr_status *xcvr;
	u32 len;

	/* copy the module bytes into data */
	xcvr = &idev->port_info->status.xcvr;
	len = min_t(u32, sizeof(xcvr->sprom), ee->len);
	memcpy(data, xcvr->sprom, len);

	dev_dbg(&lif->netdev->dev, "notifyblock eid=0x%llx link_status=0x%x link_speed=0x%x link_down_cnt=0x%x\n",
		lif->info->status.eid,
		lif->info->status.link_status,
		lif->info->status.link_speed,
		lif->info->status.link_down_count);
	dev_dbg(&lif->netdev->dev, "  port_status id=0x%x status=0x%x speed=0x%x\n",
		idev->port_info->status.id,
		idev->port_info->status.status,
		idev->port_info->status.speed);
	dev_dbg(&lif->netdev->dev, "    xcvr status state=0x%x phy=0x%x pid=0x%x\n",
		xcvr->state, xcvr->phy, xcvr->pid);
	dev_dbg(&lif->netdev->dev, "  port_config state=0x%x speed=0x%x mtu=0x%x an_enable=0x%x fec_type=0x%x pause_type=0x%x loopback_mode=0x%x\n",
		idev->port_info->config.state,
		idev->port_info->config.speed,
		idev->port_info->config.mtu,
		idev->port_info->config.an_enable,
		idev->port_info->config.fec_type,
		idev->port_info->config.pause_type,
		idev->port_info->config.loopback_mode);

	return 0;
}

static int ionic_nway_reset(struct net_device *netdev)
{
	struct lif *lif = netdev_priv(netdev);
	int err = 0;

	if (netif_running(netdev))
		err = ionic_reset_queues(lif);

	return err;
}

static const struct ethtool_ops ionic_ethtool_ops = {
	.get_drvinfo		= ionic_get_drvinfo,
	.get_link		= ethtool_op_get_link,
	.get_link_ksettings	= ionic_get_link_ksettings,
	.get_coalesce		= ionic_get_coalesce,
	.set_coalesce		= ionic_set_coalesce,
	.get_ringparam		= ionic_get_ringparam,
	.set_ringparam		= ionic_set_ringparam,
	.get_channels		= ionic_get_channels,
	.set_channels		= ionic_set_channels,
	.get_strings		= ionic_get_strings,
	.get_ethtool_stats	= ionic_get_stats,
	.get_sset_count		= ionic_get_sset_count,
	.get_rxnfc		= ionic_get_rxnfc,
	.get_rxfh_indir_size	= ionic_get_rxfh_indir_size,
	.get_rxfh_key_size	= ionic_get_rxfh_key_size,
	.get_rxfh		= ionic_get_rxfh,
	.set_rxfh		= ionic_set_rxfh,
	.get_priv_flags		= ionic_get_priv_flags,
	.set_priv_flags		= ionic_set_priv_flags,
	.get_tunable		= ionic_get_tunable,
	.set_tunable		= ionic_set_tunable,
	.get_module_info	= ionic_get_module_info,
	.get_module_eeprom	= ionic_get_module_eeprom,
	.get_pauseparam		= ionic_get_pauseparam,
	.set_pauseparam		= ionic_set_pauseparam,
	.set_link_ksettings	= ionic_set_link_ksettings,
	.nway_reset		= ionic_nway_reset,
};

void ionic_ethtool_set_ops(struct net_device *netdev)
{
	netdev->ethtool_ops = &ionic_ethtool_ops;
}
