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

#ifndef IONIC_DBG_H
#define IONIC_DBG_H

#include <stdio.h>
#include "ionic.h"

#ifndef IONIC_DEBUG
#define IONIC_DEBUG true
#endif

extern FILE *IONIC_DEBUG_FILE;

#define IONIC_DEFAULT_DEBUG_FILE stderr

#define _ionic_dbg(file, fmt, args...)					\
	fprintf(file, "%s:%d: " fmt "\n",				\
		__func__, __LINE__, ##args)

#define ionic_dbg(ctx, fmt, args...) do {				\
	if ((IONIC_DEBUG) && unlikely(ctx->dbg_file))			\
		_ionic_dbg(ctx->dbg_file, fmt, ##args);			\
} while (0)

#define ionic_err(fmt, args...)						\
	_ionic_dbg(IONIC_DEBUG_FILE, fmt, ##args)

static inline void ionic_dbg_xdump(struct ionic_ctx *ctx, const char *str,
				   const void *ptr, size_t size)
{
	const uint8_t *ptr8 = ptr;
	int i;

	if (!(IONIC_DEBUG) || likely(!ctx->dbg_file))
		return;

	for (i = 0; i < size; i += 8)
		_ionic_dbg(ctx->dbg_file,
			   "%s: %02x %02x %02x %02x %02x %02x %02x %02x", str,
			   ptr8[i + 0], ptr8[i + 1], ptr8[i + 2], ptr8[i + 3],
			   ptr8[i + 4], ptr8[i + 5], ptr8[i + 6], ptr8[i + 7]);
}

#endif /* IONIC_DBG_H */
