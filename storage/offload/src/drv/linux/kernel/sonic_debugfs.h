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

#ifndef _SONIC_DEBUGFS_H_
#define _SONIC_DEBUGFS_H_

#ifndef __FreeBSD__
#include <linux/debugfs.h>
#endif

struct sonic;
struct qcq;

//#ifdef CONFIG_DEBUG_FS
#if 0

#ifdef DEBUGFS_TEST_API
#define NUM_SCRATCH_BUFS	16
#endif

void sonic_debugfs_create(void);
void sonic_debugfs_destroy(void);
int sonic_debugfs_add_dev(struct sonic *sonic);
void sonic_debugfs_del_dev(struct sonic *sonic);
int sonic_debugfs_add_bars(struct sonic *sonic);
int sonic_debugfs_add_dev_cmd(struct sonic *sonic);
int sonic_debugfs_add_ident(struct sonic *sonic);
int sonic_debugfs_add_sizes(struct sonic *sonic);
int sonic_debugfs_add_lif(struct lif *lif);
int sonic_debugfs_add_qcq(struct lif *lif, struct qcq *qcq);
#else
static inline void sonic_debugfs_create(void) { }
static inline void sonic_debugfs_destroy(void) { }
static inline int sonic_debugfs_add_dev(struct sonic *sonic) { return 0; }
static inline void sonic_debugfs_del_dev(struct sonic *sonic) { }
static inline int sonic_debugfs_add_bars(struct sonic *sonic) { return 0; }
static inline int sonic_debugfs_add_dev_cmd(struct sonic *sonic) { return 0; }
static inline int sonic_debugfs_add_ident(struct sonic *sonic) { return 0; }
static inline int sonic_debugfs_add_sizes(struct sonic *sonic) { return 0; }
static inline int sonic_debugfs_add_lif(struct lif *lif) { return 0; }
static inline int sonic_debugfs_add_qcq(struct lif *lif, struct qcq *qcq) { return 0; }
#endif

#endif /* _SONIC_DEBUGFS_H_ */
