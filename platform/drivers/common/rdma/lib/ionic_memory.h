/* SPDX-License-Identifier: GPL-2.0 OR Linux-OpenIB */
#ifdef __FreeBSD__
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
#else
/*
 * Copyright (c) 2018-2020 Pensando Systems, Inc.  All rights reserved.
 */
#endif /* __FreeBSD__ */

#ifndef IONIC_MEMORY_H
#define IONIC_MEMORY_H

#include <stdlib.h>

/**
 * ionic_map_anon() - Allocate page-aligned anonymous memory
 * @size:	Size to allocate
 *
 * Return: pointer to first page of memory, or NULL
 */
void *ionic_map_anon(size_t size);

/**
 * ionic_map_device() - Map device memory via fd and offset
 * @size:	Size to map
 * @fd:		File descriptor representing the device context
 * @offset:	Offset provided by the kernel as a handle for the mapping
 *
 * The mapping may be write-only, and may use write-combining.
 *
 * Return: pointer to first page of the mapping, or NULL
 */
void *ionic_map_device(size_t size, int fd, size_t offset);

/**
 * ionic_unmap() - Unmap anonymous or device memory
 * @ptr:	Pointer to the first page, or NULL (do nothing)
 * @size:	Size of the map
 */
void ionic_unmap(void *ptr, size_t size);

#endif /* IONIC_MEMORY_H */
