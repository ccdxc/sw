/*
 * Copyright (c) 2019 Pensando Systems. All rights reserved.
 */

#ifndef _IONIC_API_H_
#define _IONIC_API_H_

#include "ionic.h"
#include "ionic_types.h"
#include "ionic_if.h"
#include "ionic_completion.h"

/** IONIC_API_VERSION - Version number of this interface.
 *
 * Any interface changes to this interface must also change the version.
 */
#define IONIC_API_VERSION "2"


/** ionic_admin_ctx - Admin command context.
 * @work:               Work completion wait queue element.
 * @cmd:                Admin command (64B) to be copied to the queue.
 * @comp:               Admin completion (16B) copied from the queue.
 */
struct ionic_admin_ctx {
        struct ionic_completion work;
        union ionic_adminq_cmd cmd;
        union ionic_adminq_comp comp;
};

VMK_ReturnStatus
ionic_api_adminq_post(void *handle, struct ionic_admin_ctx *ctx);

#endif /* End of _IONIC_API_H_ */
