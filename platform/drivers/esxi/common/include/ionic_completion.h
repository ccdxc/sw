/*
 * Copyright (c) 2019 Pensando Systems. All rights reserved.
 */

/*
 * ionic_completion.h --
 *
 * Definitions for completion related data structures and functions 
 */

#ifndef _IONIC_COMPLETION_H_
#define _IONIC_COMPLETION_H_

#include <vmkapi.h>

#define IONIC_EN_COMPL_VALIDATION_ID    0xfffffffffffffffeULL

struct ionic_completion {
        vmk_Bool              done;
        vmk_WorldEventID      event_id;
        vmk_Lock              lock;
        vmk_uint64            validation_id;
};

VMK_ReturnStatus
ionic_completion_create(vmk_ModuleID module_id,
                        vmk_HeapID heap_id,
                        vmk_LockDomainID domain,
                        const char *name,
                        struct ionic_completion *completion);

void
ionic_completion_destroy(struct ionic_completion *completion);

void
ionic_completion_init(struct ionic_completion *completion);

void
ionic_complete(struct ionic_completion *completion);

vmk_Bool
ionic_wait_for_completion_timeout(struct ionic_completion *completion,
                                  vmk_uint32 timeout_ms);


#endif /* End of _IONIC_COMPLETION_H_ */
