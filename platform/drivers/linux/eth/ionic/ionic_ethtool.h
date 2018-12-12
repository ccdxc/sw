/* SPDX-License-Identifier: GPL-2.0 */
/* Copyright(c) 2017 - 2019 Pensando Systems, Inc */

#ifndef _IONIC_ETHTOOL_H_
#define _IONIC_ETHTOOL_H_

void ionic_ethtool_set_ops(struct net_device *netdev);
int ionic_rss_ind_tbl_set(struct lif *lif, const u32 *indir);
int ionic_rss_hash_key_set(struct lif *lif, const u8 *key);

#endif /* _IONIC_ETHTOOL_H_ */
