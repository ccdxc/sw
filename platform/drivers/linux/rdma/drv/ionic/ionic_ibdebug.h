#ifndef IONIC_IBDEBUG_H
#define IONIC_IBDEBUG_H

/* Ionic rdma debugfs directory contents:
 *
 * The debugfs contents are an informative resoure for debugging, only.  They
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
struct dentry;

void ionic_dbgfs_add_dev(struct ionic_ibdev *dev, struct dentry *parent);
void ionic_dbgfs_rm_dev(struct ionic_ibdev *dev);

void ionic_dbgfs_add_eq(struct ionic_ibdev *dev, struct ionic_eq *eq);
void ionic_dbgfs_rm_eq(struct ionic_eq *eq);

#endif /* IONIC_IBDEBUG_H */
