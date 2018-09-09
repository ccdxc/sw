/*
 * Copyright 2017-2018 Pensando Systems, Inc.  All rights reserved.
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

#ifndef _IONIC_API_H_
#define _IONIC_API_H_

#include "ionic.h"

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

#ifndef ADMINQ
        //struct list_head list;
        vmk_ListLinks list;
        void *side_data;
        size_t side_data_len;
#endif
};

VMK_ReturnStatus
ionic_api_adminq_post(struct lif *lif, struct ionic_admin_ctx *ctx);

#endif /* End of _IONIC_API_H_ */
