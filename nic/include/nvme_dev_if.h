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

#ifndef _NVME_DEV_IF_H_
#define _NVME_DEV_IF_H_

#ifdef __FreeBSD__
#include <linux/types.h>
#endif

#define NVME_DEV_STRINGIFY(x)          #x
#define NVME_DEV_INDEX_STRINGIFY(x)    [x] = NVME_DEV_STRINGIFY(x)
#define NVME_DEV_CASE_STRINGIFY(x)     case x: return NVME_DEV_STRINGIFY(x)


enum nvme_status_code {
	NVME_RC_SUCCESS	= 0,	/* Success */
	NVME_RC_EVERSION	= 1,	/* Incorrect version for request */
	NVME_RC_EOPCODE	= 2,	/* Invalid cmd opcode */
	NVME_RC_EIO		= 3,	/* I/O error */
	NVME_RC_EPERM		= 4,	/* Permission denied */
	NVME_RC_EQID		= 5, 	/* Bad qid */
	NVME_RC_EQTYPE		= 6, 	/* Bad qtype */
	NVME_RC_ENOENT		= 7,	/* No such element */
	NVME_RC_EINTR		= 8,	/* operation interrupted */
	NVME_RC_EAGAIN		= 9,	/* Try again */
	NVME_RC_ENOMEM		= 10,	/* Out of memory */
	NVME_RC_EFAULT		= 11,	/* Bad address */
	NVME_RC_EBUSY		= 12,	/* Device or resource busy */
	NVME_RC_EEXIST		= 13,	/* object already exists */
	NVME_RC_EINVAL		= 14,	/* Invalid argument */
	NVME_RC_ENOSPC		= 15,	/* No space left or alloc failure */
	NVME_RC_ERANGE		= 16,	/* Parameter out of range */
	NVME_RC_BAD_ADDR	= 17,	/* Descriptor contains a bad ptr */
	NVME_RC_DEV_CMD	= 18,	/* Device cmd attempted on AdminQ */
	NVME_RC_ERROR		= 19,	/* Generic error */
};

#define NVME_RC_STR_TABLE                              \
    NVME_DEV_INDEX_STRINGIFY(NVME_RC_SUCCESS),        \
    NVME_DEV_INDEX_STRINGIFY(NVME_RC_EVERSION),       \
    NVME_DEV_INDEX_STRINGIFY(NVME_RC_EOPCODE),        \
    NVME_DEV_INDEX_STRINGIFY(NVME_RC_EIO),            \
    NVME_DEV_INDEX_STRINGIFY(NVME_RC_EPERM),          \
    NVME_DEV_INDEX_STRINGIFY(NVME_RC_EQID),           \
    NVME_DEV_INDEX_STRINGIFY(NVME_RC_EQTYPE),         \
    NVME_DEV_INDEX_STRINGIFY(NVME_RC_ENOENT),         \
    NVME_DEV_INDEX_STRINGIFY(NVME_RC_EINTR),          \
    NVME_DEV_INDEX_STRINGIFY(NVME_RC_EAGAIN),         \
    NVME_DEV_INDEX_STRINGIFY(NVME_RC_ENOMEM),         \
    NVME_DEV_INDEX_STRINGIFY(NVME_RC_EFAULT),         \
    NVME_DEV_INDEX_STRINGIFY(NVME_RC_EBUSY),          \
    NVME_DEV_INDEX_STRINGIFY(NVME_RC_EEXIST),         \
    NVME_DEV_INDEX_STRINGIFY(NVME_RC_EINVAL),         \
    NVME_DEV_INDEX_STRINGIFY(NVME_RC_ENOSPC),         \
    NVME_DEV_INDEX_STRINGIFY(NVME_RC_ERANGE),         \
    NVME_DEV_INDEX_STRINGIFY(NVME_RC_BAD_ADDR),       \
    NVME_DEV_INDEX_STRINGIFY(NVME_RC_DEV_CMD),        \
    NVME_DEV_INDEX_STRINGIFY(NVME_RC_ERROR),          \

typedef int nvme_status_code_t;

#endif /* _NVME_DEV_IF_H_ */
