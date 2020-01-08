/*
 * Copyright (c) 2018-2019 Pensando Systems, Inc.  All rights reserved.
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

#ifndef __IONIC_ABI_H__
#define __IONIC_ABI_H__

#include <infiniband/kern-abi.h>
#include <infiniband/verbs.h>
#include <rdma/ionic-abi.h>
#include <kernel-abi/ionic-abi.h>
#include "ionic_fw.h"

DECLARE_DRV_CMD(uionic_ctx, IB_USER_VERBS_CMD_GET_CONTEXT,
		ionic_ctx_req, ionic_ctx_resp);
DECLARE_DRV_CMD(uionic_ah, IB_USER_VERBS_CMD_CREATE_AH,
		empty, ionic_ah_resp);
DECLARE_DRV_CMD(uionic_cq, IB_USER_VERBS_CMD_CREATE_CQ,
		ionic_cq_req, ionic_cq_resp);
DECLARE_DRV_CMD(uionic_qp, IB_USER_VERBS_EX_CMD_CREATE_QP,
		ionic_qp_req, ionic_qp_resp);
DECLARE_DRV_CMD(uionic_srq, IB_USER_VERBS_CMD_CREATE_XSRQ,
		ionic_srq_req, ionic_srq_resp);

#endif /* __IONIC_ABI_H__ */
