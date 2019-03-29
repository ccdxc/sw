/*
 * Copyright (c) 2019 Pensando Systems. All rights reserved.
 */

#ifndef _IONIC_API_H_
#define _IONIC_API_H_

#include "ionic.h"

#define FAKE_ADMINQ     1

#include "ionic_if.h"
#include "ionic_completion.h"

/** IONIC_API_VERSION - Version number of this interface.
 *
 * Any interface changes to this interface must also change the version.
 */
#define IONIC_API_VERSION "1"


/** ionic_admin_ctx - Admin command context.
 * @work:               Work completion wait queue element.
 * @cmd:                Admin command (64B) to be copied to the queue.
 * @comp:               Admin completion (16B) copied from the queue.
 *
 * @side_data:          Additional data to be copied to the doorbell page,
 *                        if the command is issued as a dev cmd.
 * @side_data_len:      Length of additional data to be copied.
 *
 * TODO:
 * The side_data and side_data_len are temporary and will be removed.  For now,
 * they are used when admin commands referring to side-band data are posted as
 * dev commands instead.  Only single-indirect side-band data is supported.
 * Only 2K of data is supported, because first half of page is for registers.
 */
struct ionic_admin_ctx {
//      struct completion work;
        struct ionic_completion work;
        union adminq_cmd cmd;
        union adminq_comp comp;

#ifdef FAKE_ADMINQ
        //struct list_head list;
        vmk_ListLinks list;
        void *side_data;
        size_t side_data_len;
#endif
};

VMK_ReturnStatus
ionic_api_adminq_post(struct lif *lif, struct ionic_admin_ctx *ctx);

#endif /* End of _IONIC_API_H_ */
