/* SPDX-License-Identifier: GPL-2.0 OR Linux-OpenIB */
/*
 * Copyright (c) 2018-2020 Pensando Systems, Inc.  All rights reserved.
 *
 * This software is available to you under a choice of one of two
 * licenses.  You may choose to be licensed under the terms of the GNU
 * General Public License (GPL) Version 2, available from the file
 * COPYING in the main directory of this source tree, or the
 * OpenIB.org BSD license below:
 *
 *     Redistribution and use in source and binary forms, with or
 *     without modification, are permitted provided that the following
 *     conditions are met:
 *
 *      - Redistributions of source code must retain the above
 *        copyright notice, this list of conditions and the following
 *        disclaimer.
 *
 *      - Redistributions in binary form must reproduce the above
 *        copyright notice, this list of conditions and the following
 *        disclaimer in the documentation and/or other materials
 *        provided with the distribution.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
 * BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
 * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#ifndef IONIC_SYSCTL_H
#define IONIC_SYSCTL_H

struct sysctl_oid;

struct ionic_ibdev;
struct ionic_eq;
struct ionic_cq;
struct ionic_aq;
struct ionic_mr;
struct ionic_qp;

void ionic_dbg_add_dev(struct ionic_ibdev *dev, struct sysctl_oid *oidp);
void ionic_dbg_rm_dev(struct ionic_ibdev *dev);

void ionic_dbg_add_eq(struct ionic_ibdev *dev, struct ionic_eq *eq);
void ionic_dbg_rm_eq(struct ionic_eq *eq);

void ionic_dbg_add_cq(struct ionic_ibdev *dev, struct ionic_cq *cq);
void ionic_dbg_rm_cq(struct ionic_cq *cq);

void ionic_dbg_add_aq(struct ionic_ibdev *dev, struct ionic_aq *aq);
void ionic_dbg_rm_aq(struct ionic_aq *aq);

void ionic_dbg_add_mr(struct ionic_ibdev *dev, struct ionic_mr *mr);
void ionic_dbg_rm_mr(struct ionic_mr *mr);

void ionic_dbg_add_qp(struct ionic_ibdev *dev, struct ionic_qp *qp);
void ionic_dbg_rm_qp(struct ionic_qp *qp);

/* make pr_debug, print_hex_dump_debug useful */

#include <ionic_kpicompat.h>

extern bool ionic_dyndbg_enable;

#undef pr_debug
#define pr_debug(fmt, ...) do {						\
	if (ionic_dyndbg_enable)					\
		pr_info("%s:%d: " #fmt, __func__, __LINE__, ##__VA_ARGS__); \
} while (0)

#undef print_hex_dump_debug
#define print_hex_dump_debug(...) do {					\
	if (ionic_dyndbg_enable)					\
		print_hex_dump(NULL, ##__VA_ARGS__);			\
} while (0)

#endif /* IONIC_SYSCTL_H */
