/* SPDX-License-Identifier: GPL-2.0 */
/* Copyright(c) 2017 - 2019 Pensando Systems, Inc */

#ifndef _IONIC_DEBUGFS_H_
#define _IONIC_DEBUGFS_H_

#include <linux/debugfs.h>

struct ionic;
struct qcq;

#ifdef CONFIG_DEBUG_FS

#ifdef DEBUGFS_TEST_API
#define NUM_SCRATCH_BUFS	16
#endif

void ionic_debugfs_create(void);
void ionic_debugfs_destroy(void);
int ionic_debugfs_add_dev(struct ionic *ionic);
void ionic_debugfs_del_dev(struct ionic *ionic);
int ionic_debugfs_add_bars(struct ionic *ionic);
int ionic_debugfs_add_dev_cmd(struct ionic *ionic);
int ionic_debugfs_add_ident(struct ionic *ionic);
int ionic_debugfs_add_sizes(struct ionic *ionic);
int ionic_debugfs_add_lif(struct lif *lif);
int ionic_debugfs_add_qcq(struct lif *lif, struct qcq *qcq);
void ionic_debugfs_del_lif(struct lif *lif);
void ionic_debugfs_del_qcq(struct qcq *qcq);
#else
static inline void ionic_debugfs_create(void) { }
static inline void ionic_debugfs_destroy(void) { }
static inline int ionic_debugfs_add_dev(struct ionic *ionic) { return 0; }
static inline void ionic_debugfs_del_dev(struct ionic *ionic) { }
static inline int ionic_debugfs_add_bars(struct ionic *ionic) { return 0; }
static inline int ionic_debugfs_add_dev_cmd(struct ionic *ionic) { return 0; }
static inline int ionic_debugfs_add_ident(struct ionic *ionic) { return 0; }
static inline int ionic_debugfs_add_sizes(struct ionic *ionic) { return 0; }
static inline int ionic_debugfs_add_lif(struct lif *lif) { return 0; }
static inline int ionic_debugfs_add_qcq(struct lif *lif, struct qcq *qcq) { return 0; }
static inline void ionic_debugfs_del_lif(struct lif *lif) { return 0; }
static inline void ionic_debugfs_del_qcq(struct qcq *qcq) { return 0; }
#endif

#endif /* _IONIC_DEBUGFS_H_ */
