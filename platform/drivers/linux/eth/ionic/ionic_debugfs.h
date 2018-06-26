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

#ifndef _IONIC_DEBUGFS_H_
#define _IONIC_DEBUGFS_H_

#include <linux/debugfs.h>

struct ionic;

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
#endif

#endif /* _IONIC_DEBUGFS_H_ */
