/*
 * Copyright (c) 2019 Pensando Systems. All rights reserved.
 */

#include "ionic.h"

static void ionic_api_adminq_cb(struct queue *q, struct desc_info *desc_info,
				struct cq_info *cq_info, void *cb_arg)
{
	struct ionic_admin_ctx *ctx = cb_arg;
	struct admin_comp *comp = cq_info->cq_desc;

        if (VMK_UNLIKELY(!ctx)) {
                return;
        }

        if (IONIC_WARN_ON(comp->comp_index != desc_info->index))
		return;

	vmk_Memcpy(&ctx->comp, comp, sizeof(*comp));

        ionic_hex_dump("comp admin queue command",
                       &ctx->comp,
                       sizeof(ctx->comp),
                       ionic_driver.log_component);

        ionic_complete(&ctx->work);
}

VMK_ReturnStatus
ionic_api_adminq_post(void *handle, struct ionic_admin_ctx *ctx)
{
	VMK_ReturnStatus status = VMK_OK;
	struct lif *lif = handle;
	struct queue *adminq = &lif->adminqcq->q;

	vmk_SpinlockLock(lif->adminq_lock);
	if (!ionic_q_has_space(adminq, 1)) {
                status = VMK_NO_MEMORY;
		goto err_out;
	}

	vmk_Memcpy(adminq->head->desc, &ctx->cmd, sizeof(ctx->cmd));

        ionic_hex_dump("post admin queue command",
                       &ctx->cmd,
                       sizeof(ctx->cmd),
                       ionic_driver.log_component);

        ionic_q_post(adminq, true, ionic_api_adminq_cb, ctx);

err_out:
	vmk_SpinlockUnlock(lif->adminq_lock);
	return status;
}

