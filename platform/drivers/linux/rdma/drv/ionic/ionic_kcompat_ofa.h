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

#ifndef IONIC_KCOMPAT_OFA
#define IONIC_KCOMPAT_OFA

/* There is no semantic versioning for OFED kernel api.
 *
 * Instead, our makefile detects the presence of an ofed stack, and if present
 * it defines OFA_KERNEL with hash from the compat_version file at the top of
 * the ofa kernel, eg, /usr/src/ofa_kernel/default/compat_version.  That
 * compat_version is just a unique hash for that particular version of ofed,
 * not a semantic version number.
 *
 * Then below, we encode compatibility for each _specific_ version of ofed that
 * we support.  We will support a minimal number of ofed stacks as an
 * out-of-ofed driver.
 *
 *
 * This compatibility support is fragile, because ofed can be bugfixed by
 * vendor with no minor version chage, but then ofed compat_version hash can
 * still change, breaking compatibility.  To resolve the new hash to compat
 * ofed configuration, add one more line like following, to this file:
 *
 *	#define OFA_COMPAT_xxxxyyy		MLNX_OFED_4_5
 */

/* MLNX_OFED_LINUX-4.2-1.2.0.0-debian9.0-x86_64 */
#define OFA_COMPAT_f8de107		MLNX_OFED_4_2

#define MLNX_OFED_4_2__support		1
#define MLNX_OFED_4_2__umem_get		1
#define MLNX_OFED_4_2__4_10b		1
#define MLNX_OFED_4_2__4_14a		1
#define MLNX_OFED_4_2__4_14c		1
#define MLNX_OFED_4_2__4_17		1
#define MLNX_OFED_4_2__4_19a		1
#define MLNX_OFED_4_2__4_19b		1
#define MLNX_OFED_4_2__4_20		1
#define MLNX_OFED_4_2__5_0		1

/* MLNX_OFED_LINUX-4.5-1.0.1.0-debian9.0-x86_64 */
#define OFA_COMPAT_b4fdfac		MLNX_OFED_4_5

#define MLNX_OFED_4_5__support		1
#define MLNX_OFED_4_5__umem_get		1
#define MLNX_OFED_4_5__reg_user_mr	1
#define MLNX_OFED_4_5__4_10b		1
#define MLNX_OFED_4_5__4_19b		1
#define MLNX_OFED_4_5__4_20		1
#define MLNX_OFED_4_5__5_0		1

/* macro magic...
 * OFA_COMPAT_CHECK(OFA_KERNEL, OFA)
 * -> _OFA_COMPAT_CHECK(OFA_COMPAT_ ## f8de107, support)
 * -> __OFA_COMPAT_CHECK(OFA_COMPAT_f8de107, support)
 * -> ___OFA_COMPAT_CHECK(MLNX_OFED_4_2, support)
 * -> MLNX_OFED_4_2__support
 */
#define OFA_COMPAT_CHECK(OFA_KERNEL, OFA) \
	_OFA_COMPAT_CHECK(OFA_KERNEL, OFA)
#define _OFA_COMPAT_CHECK(OFA_KERNEL, OFA) \
	__OFA_COMPAT_CHECK(OFA_COMPAT_ ## OFA_KERNEL, OFA)
#define __OFA_COMPAT_CHECK(OFA_COMPAT, OFA) \
	___OFA_COMPAT_CHECK(OFA_COMPAT, OFA)
#define ___OFA_COMPAT_CHECK(OFA_COMPAT, OFA) \
	OFA_COMPAT ## __ ## OFA

#if ! OFA_COMPAT_CHECK(OFA_KERNEL, support)
#error Missing support for OFED version
#endif

#if OFA_COMPAT_CHECK(OFA_KERNEL, umem_get)
#define ib_umem_get(c,s,l,a,f) ib_umem_get(c,s,l,a,f,0)
#endif

#if OFA_COMPAT_CHECK(OFA_KERNEL, reg_user_mr)
#define HAVE_IB_USER_MR_INIT_ATTR
#endif

#endif
