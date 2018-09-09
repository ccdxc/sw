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

/*
 * ionic_completion.h --
 *
 * Definitions for completion related data structures and functions 
 */

#ifndef _IONIC_COMPLETION_H_
#define _IONIC_COMPLETION_H_

#include <vmkapi.h>

struct ionic_completion {
        vmk_Bool              done;
        vmk_WorldEventID      event_id;
        vmk_Lock              lock;
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

void
ionic_wait_for_completion(struct ionic_completion *completion);


#endif /* End of _IONIC_COMPLETION_H_ */
