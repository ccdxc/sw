// SPDX-License-Identifier: GPL-2.0 OR Linux-OpenIB
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

#include <infiniband/driver.h>
#include <sys/mman.h>

#include "ionic_memory.h"

#define IONIC_ANON_MFLAGS	(MAP_PRIVATE | MAP_ANONYMOUS)
#define IONIC_ANON_MPROT	(PROT_READ | PROT_WRITE)

#define IONIC_DEV_MFLAGS	MAP_SHARED
#define IONIC_DEV_MPROT		PROT_WRITE

void *ionic_map_anon(size_t size)
{
	void *ptr;
	int rc;

	/* TODO: READ only CQ, WRITE only Send/RecvQ */
	ptr = mmap(NULL, size, IONIC_ANON_MPROT, IONIC_ANON_MFLAGS, -1, 0);
	if (ptr == MAP_FAILED)
		return NULL;

	rc = ibv_dontfork_range(ptr, size);
	if (rc) {
		munmap(ptr, size);
		errno = rc;
		return NULL;
	}

	return ptr;
}

void *ionic_map_device(size_t size, int fd, size_t offset)
{
	void *ptr;
	int rc;

	ptr = mmap(NULL, size, IONIC_DEV_MPROT, IONIC_DEV_MFLAGS, fd, offset);
	if (ptr == MAP_FAILED)
		return NULL;

	rc = ibv_dontfork_range(ptr, size);
	if (rc) {
		munmap(ptr, size);
		errno = rc;
		return NULL;
	}

	return ptr;
}

void ionic_unmap(void *ptr, size_t size)
{
	if (ptr) {
		ibv_dofork_range(ptr, size);
		munmap(ptr, size);
	}
}

