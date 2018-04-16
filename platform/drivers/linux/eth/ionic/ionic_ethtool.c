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

#include <linux/module.h>
#include <linux/netdevice.h>

#include "ionic.h"
#include "ionic_if.h"
#include "ionic_api.h"
#include "ionic_bus.h"
#include "ionic_lif.h"
#include "ionic_ethtool.h"

static void ionic_get_drvinfo(struct net_device *netdev,
			      struct ethtool_drvinfo *drvinfo)
{
	struct lif *lif = netdev_priv(netdev);
	struct ionic *ionic = lif->ionic;

	strlcpy(drvinfo->driver, DRV_NAME, sizeof(drvinfo->driver));
	strlcpy(drvinfo->version, DRV_VERSION, sizeof(drvinfo->version));
	strlcpy(drvinfo->fw_version, ionic->ident->dev.fw_version,
		sizeof(drvinfo->fw_version));
	strlcpy(drvinfo->bus_info, ionic_bus_info(ionic),
		sizeof(drvinfo->bus_info));
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
	union identity *ident = lif->ionic->ident;
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

	tx_coal = coalesce->tx_coalesce_usecs * ident->dev.intr_coal_mult /
		  ident->dev.intr_coal_div;
	rx_coal = coalesce->rx_coalesce_usecs * ident->dev.intr_coal_mult /
		  ident->dev.intr_coal_div;

	if (tx_coal > INTR_CTRL_COAL_MAX || rx_coal > INTR_CTRL_COAL_MAX)
		return -ERANGE;

	if (coalesce->tx_coalesce_usecs != lif->tx_coalesce_usecs) {
		for (i = 0; i < lif->ntxqcqs; i++)
			ionic_intr_coal_set(&lif->txqcqs[i]->intr, tx_coal);
		lif->tx_coalesce_usecs = coalesce->tx_coalesce_usecs;
	}

	if (coalesce->rx_coalesce_usecs != lif->rx_coalesce_usecs) {
		for (i = 0; i < lif->nrxqcqs; i++)
			ionic_intr_coal_set(&lif->rxqcqs[i]->intr, rx_coal);
		lif->rx_coalesce_usecs = coalesce->rx_coalesce_usecs;
	}

	return 0;
}

static void ionic_get_ringparam(struct net_device *netdev,
				struct ethtool_ringparam *ring)
{
	ring->tx_max_pending = 1 << 16;
	ring->tx_pending = ntxq_descs;
	ring->rx_max_pending = 1 << 16;
	ring->rx_pending = nrxq_descs;
}

static int ionic_get_rxnfc(struct net_device *netdev,
			   struct ethtool_rxnfc *info, u32 *rules)
{
	struct lif *lif = netdev_priv(netdev);
	int err = 0;

	switch (info->cmd) {
	case ETHTOOL_GRXRINGS:
		info->data = lif->nrxqcqs;
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
	return RSS_IND_TBL_SIZE;
}

static u32 ionic_get_rxfh_key_size(struct net_device *netdev)
{
	return RSS_HASH_KEY_SIZE;
}

static int ionic_get_rxfh(struct net_device *netdev, u32 *indir, u8 *key,
			  u8 *hfunc)
{
	struct lif *lif = netdev_priv(netdev);
	unsigned int i;

	if (indir)
		for (i = 0; i < RSS_IND_TBL_SIZE; i++)
			indir[i] = lif->rss_ind_tbl[i];

	if (key)
		memcpy(key, lif->rss_hash_key, RSS_HASH_KEY_SIZE);

	if (hfunc)
		*hfunc = ETH_RSS_HASH_TOP;

	return 0;
}

int ionic_rss_ind_tbl_set(struct lif *lif, const u32 *indir)
{
	struct ionic_admin_ctx ctx = {
		.work = COMPLETION_INITIALIZER_ONSTACK(ctx.work),
		.cmd.rss_indir_set = {
			.opcode = CMD_OPCODE_RSS_INDIR_SET,
			.addr = lif->rss_ind_tbl_pa,
		},
#ifdef HAPS
		.side_data = lif->rss_ind_tbl,
		.side_data_len = RSS_IND_TBL_SIZE,
#endif
	};
	unsigned int i;

	if (indir)
		for (i = 0; i < RSS_IND_TBL_SIZE; i++)
			lif->rss_ind_tbl[i] = indir[i];

	netdev_info(lif->netdev, "rss_ind_tbl_set\n");

	return ionic_adminq_post_wait(lif, &ctx);
}

int ionic_rss_hash_key_set(struct lif *lif, const u8 *key)
{
	struct ionic_admin_ctx ctx = {
		.work = COMPLETION_INITIALIZER_ONSTACK(ctx.work),
		.cmd.rss_hash_set = {
			.opcode = CMD_OPCODE_RSS_HASH_SET,
			.types = RSS_TYPE_IPV4
			       | RSS_TYPE_IPV4_TCP
			       | RSS_TYPE_IPV4_UDP
			       | RSS_TYPE_IPV6
			       | RSS_TYPE_IPV6_TCP
			       | RSS_TYPE_IPV6_UDP
			       | RSS_TYPE_IPV6_EX
			       | RSS_TYPE_IPV6_TCP_EX
			       | RSS_TYPE_IPV6_UDP_EX,
		},
	};

	memcpy(lif->rss_hash_key, key, RSS_HASH_KEY_SIZE);

	memcpy(ctx.cmd.rss_hash_set.key, lif->rss_hash_key,
	       RSS_HASH_KEY_SIZE);

	netdev_info(lif->netdev, "rss_hash_key_set\n");

	return ionic_adminq_post_wait(lif, &ctx);
}

static int ionic_set_rxfh(struct net_device *netdev, const u32 *indir,
			  const u8 *key, const u8 hfunc)
{
	struct lif *lif = netdev_priv(netdev);
	int err;

	if (hfunc != ETH_RSS_HASH_NO_CHANGE && hfunc != ETH_RSS_HASH_TOP)
		return -EOPNOTSUPP;

	if (indir) {
		err = ionic_rss_ind_tbl_set(lif, indir);
		if (err)
			return err;
	}

	if (key) {
		err = ionic_rss_hash_key_set(lif, key);
		if (err)
			return err;
	}

	return 0;
}

static const struct ethtool_ops ionic_ethtool_ops = {
	.get_drvinfo		= ionic_get_drvinfo,
	.get_link		= ethtool_op_get_link,
	.get_coalesce		= ionic_get_coalesce,
	.set_coalesce		= ionic_set_coalesce,
	.get_ringparam		= ionic_get_ringparam,
	.get_rxnfc		= ionic_get_rxnfc,
	.get_rxfh_indir_size    = ionic_get_rxfh_indir_size,
	.get_rxfh_key_size	= ionic_get_rxfh_key_size,
	.get_rxfh		= ionic_get_rxfh,
	.set_rxfh		= ionic_set_rxfh,
};

void ionic_ethtool_set_ops(struct net_device *netdev)
{
	netdev->ethtool_ops = &ionic_ethtool_ops;
}
