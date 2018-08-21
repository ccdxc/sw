/*
 * Copyright (c) 2018 Pensando Systems, Inc.  All rights reserved.
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

#ifndef IONIC_IBDEBUG_H
#define IONIC_IBDEBUG_H

/* Ionic rdma debugfs directory contents:
 *
 * The debugfs contents are an informative resource for debugging, only.  They
 * should not be relied on as a stable api from user space.  The location,
 * arrangement, names, internal formats and structures of these files may
 * change without warning.  Any documentation, including this, is very likely
 * to be incorrect or incomplete.  You have been warned.
 *
 * (parent dir: ionic lif)
 * - rdma
 *   |- info		- rdma device info
 *   |
 *   |- ah/N
 *   |  `- info		- address handle info (sgid idx, dgid, qkey, ...)
 *   |  `- hdr		- packet header template (raw data)
 *   |
 *   |- cq/N
 *   |  |- info		- completion queue info (id, prod, cons, mask, ...)
 *   |  `- q		- (*) completion queue content (raw data)
 *   |
 *   |- eq/N
 *   |  |- info		- event queue info (id, prod, cons, mask, ...)
 *   |  `- q		- event queue content (raw data)
 *   |
 *   |- mr/N
 *   |  |- info		- memory region info (lkey, rkey, access, length, ...)
 *   |  `- umem		- (*) page and dma mapping infrmation
 *   |
 *   |- mw/N
 *   |  `- info		- memory key info (lkey, rkey*, access*, length*, ...)
 *   |
 *   |- pd/N
 *   |  `- info		- protection domain info (id)
 *   |
 *   |- qp/N
 *   |  |- info		- queue pair info (id, type, sq/rq prod, cons, ...)
 *   |  |- rq		- (*) receive queue content (raw data)
 *   |  `- sq		- (*) send queue content (raw data)
 *   |
 *   `- srq/N
 *      |- info		- shared receive queue info (id, type, prod, cons, ...)
 *      `- rq		- (*) receive queue content (raw data)
 *
 * (*) - These files are only present if supported for the resource type.
 *       These files are not created for user space resources, only kernel.
 *       Some resources (eg, XRC QP) will not have a send and/or recv queue.
 *       Some memory window attributes are not shown for user space.
 */

struct ionic_ibdev;
struct ionic_eq;
struct ionic_cq;
struct ionic_aq;
struct ionic_mr;
struct ionic_qp;
struct dentry;

void ionic_dbgfs_add_dev(struct ionic_ibdev *dev, struct dentry *parent);
void ionic_dbgfs_rm_dev(struct ionic_ibdev *dev);

void ionic_dbgfs_add_eq(struct ionic_ibdev *dev, struct ionic_eq *eq);
void ionic_dbgfs_rm_eq(struct ionic_eq *eq);

void ionic_dbgfs_add_cq(struct ionic_ibdev *dev, struct ionic_cq *cq);
void ionic_dbgfs_rm_cq(struct ionic_cq *cq);

void ionic_dbgfs_add_aq(struct ionic_ibdev *dev, struct ionic_aq *aq);
void ionic_dbgfs_rm_aq(struct ionic_aq *aq);

void ionic_dbgfs_add_mr(struct ionic_ibdev *dev, struct ionic_mr *mr);
void ionic_dbgfs_rm_mr(struct ionic_mr *mr);

void ionic_dbgfs_add_qp(struct ionic_ibdev *dev, struct ionic_qp *qp);
void ionic_dbgfs_rm_qp(struct ionic_qp *qp);

#endif /* IONIC_IBDEBUG_H */
