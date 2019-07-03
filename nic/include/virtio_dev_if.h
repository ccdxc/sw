/*
 * Copyright 2018-2019 Pensando Systems, Inc.  All rights reserved.
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

#ifndef _VIRTIO_DEV_IF_H_
#define _VIRTIO_DEV_IF_H_

#ifdef __FreeBSD__
#include <linux/types.h>
#endif

#define VIRTIO_DEV_STRINGIFY(x)          #x
#define VIRTIO_DEV_INDEX_STRINGIFY(x)    [x] = VIRTIO_DEV_STRINGIFY(x)
#define VIRTIO_DEV_CASE_STRINGIFY(x)     case x: return VIRTIO_DEV_STRINGIFY(x)


enum virtio_status_code {
	VIRTIO_RC_SUCCESS	= 0,	/* Success */
	VIRTIO_RC_EVERSION	= 1,	/* Incorrect version for request */
	VIRTIO_RC_EOPCODE	= 2,	/* Invalid cmd opcode */
	VIRTIO_RC_EIO		= 3,	/* I/O error */
	VIRTIO_RC_EPERM		= 4,	/* Permission denied */
	VIRTIO_RC_EQID		= 5,	/* Bad qid */
	VIRTIO_RC_EQTYPE	= 6,	/* Bad qtype */
	VIRTIO_RC_ENOENT	= 7,	/* No such element */
	VIRTIO_RC_EINTR		= 8,	/* operation interrupted */
	VIRTIO_RC_EAGAIN	= 9,	/* Try again */
	VIRTIO_RC_ENOMEM	= 10,	/* Out of memory */
	VIRTIO_RC_EFAULT	= 11,	/* Bad address */
	VIRTIO_RC_EBUSY		= 12,	/* Device or resource busy */
	VIRTIO_RC_EEXIST	= 13,	/* object already exists */
	VIRTIO_RC_EINVAL	= 14,	/* Invalid argument */
	VIRTIO_RC_ENOSPC	= 15,	/* No space left or alloc failure */
	VIRTIO_RC_ERANGE	= 16,	/* Parameter out of range */
	VIRTIO_RC_BAD_ADDR	= 17,	/* Descriptor contains a bad ptr */
	VIRTIO_RC_DEV_CMD	= 18,	/* Device cmd attempted on AdminQ */
	VIRTIO_RC_ERROR		= 19,	/* Generic error */
};

#define VIRTIO_RC_STR_TABLE                              \
    VIRTIO_DEV_INDEX_STRINGIFY(VIRTIO_RC_SUCCESS),        \
    VIRTIO_DEV_INDEX_STRINGIFY(VIRTIO_RC_EVERSION),       \
    VIRTIO_DEV_INDEX_STRINGIFY(VIRTIO_RC_EOPCODE),        \
    VIRTIO_DEV_INDEX_STRINGIFY(VIRTIO_RC_EIO),            \
    VIRTIO_DEV_INDEX_STRINGIFY(VIRTIO_RC_EPERM),          \
    VIRTIO_DEV_INDEX_STRINGIFY(VIRTIO_RC_EQID),           \
    VIRTIO_DEV_INDEX_STRINGIFY(VIRTIO_RC_EQTYPE),         \
    VIRTIO_DEV_INDEX_STRINGIFY(VIRTIO_RC_ENOENT),         \
    VIRTIO_DEV_INDEX_STRINGIFY(VIRTIO_RC_EINTR),          \
    VIRTIO_DEV_INDEX_STRINGIFY(VIRTIO_RC_EAGAIN),         \
    VIRTIO_DEV_INDEX_STRINGIFY(VIRTIO_RC_ENOMEM),         \
    VIRTIO_DEV_INDEX_STRINGIFY(VIRTIO_RC_EFAULT),         \
    VIRTIO_DEV_INDEX_STRINGIFY(VIRTIO_RC_EBUSY),          \
    VIRTIO_DEV_INDEX_STRINGIFY(VIRTIO_RC_EEXIST),         \
    VIRTIO_DEV_INDEX_STRINGIFY(VIRTIO_RC_EINVAL),         \
    VIRTIO_DEV_INDEX_STRINGIFY(VIRTIO_RC_ENOSPC),         \
    VIRTIO_DEV_INDEX_STRINGIFY(VIRTIO_RC_ERANGE),         \
    VIRTIO_DEV_INDEX_STRINGIFY(VIRTIO_RC_BAD_ADDR),       \
    VIRTIO_DEV_INDEX_STRINGIFY(VIRTIO_RC_DEV_CMD),        \
    VIRTIO_DEV_INDEX_STRINGIFY(VIRTIO_RC_ERROR),          \

typedef int virtio_status_code_t;

#endif /* _VIRTIO_DEV_IF_H_ */
