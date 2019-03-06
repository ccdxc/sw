/*
 * Copyright (c) 2019 Pensando Systems. All rights reserved.
 *
 * This software is provided to you under the terms of the GNU
 * General Public License (GPL) Version 2.0, available from the file
 * [ionic_api.c] in the main directory of this source tree.
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

#include "ionic.h"

static void ionic_api_adminq_cb(struct queue *q, struct desc_info *desc_info,
				struct cq_info *cq_info, void *cb_arg)
{
	struct ionic_admin_ctx *ctx = cb_arg;
	struct admin_comp *comp = cq_info->cq_desc;

	if (IONIC_WARN_ON(comp->comp_index != desc_info->index))
		return;

	vmk_Memcpy(&ctx->comp, comp, sizeof(*comp));

        ionic_hex_dump("comp admin queue command:",
                       &ctx->comp,
                       sizeof(ctx->comp));

        ionic_complete(&ctx->work);
}

VMK_ReturnStatus
ionic_api_adminq_post(struct lif *lif, struct ionic_admin_ctx *ctx)
{
	VMK_ReturnStatus status = VMK_OK;	
	struct queue *adminq = &lif->adminqcq->q;

#ifdef FAKE_ADMINQ
        struct ionic *ionic = lif->ionic;

        if (!use_AQ) {
                goto fake_adminq;
        }
#endif

	vmk_SpinlockLock(lif->adminq_lock);
	if (!ionic_q_has_space(adminq, 1)) {
                status = VMK_NO_MEMORY;
		goto err_out;
	}

	vmk_Memcpy(adminq->head->desc, &ctx->cmd, sizeof(ctx->cmd));

        ionic_hex_dump("post admin queue command:",
                       &ctx->cmd,
                       sizeof(ctx->cmd));

        ionic_q_post(adminq, true, ionic_api_adminq_cb, ctx);

err_out:
	vmk_SpinlockUnlock(lif->adminq_lock);
	return status;

#ifdef FAKE_ADMINQ
fake_adminq:
        vmk_SpinlockLock(ionic->cmd_lock);
        vmk_ListInsert(&ctx->list, &ionic->cmd_list);
	vmk_SpinlockUnlock(ionic->cmd_lock);

        status = ionic_work_queue_submit(ionic->cmd_work_queue,
                                         &ionic->cmd_work,
                                         0);
        if (status != VMK_OK) {
                ionic_err("ionic_work_queue_submit() failed, status: %s",
                          vmk_StatusToString(status));
        }

	return status;
#endif
}

