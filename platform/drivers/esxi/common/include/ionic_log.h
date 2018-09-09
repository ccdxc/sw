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
 * ionic_log.h --
 *
 * Implement ionic logging
 */

#ifndef _IONIC_LOG_H_
#define _IONIC_LOG_H_

#include <vmkapi.h>

enum {
   IONIC_LOG_LEVEL_INFO,
   IONIC_LOG_LEVEL_WARN,
   IONIC_LOG_LEVEL_ERR,
   IONIC_LOG_LEVEL_DBG,
};

static const char *const ionic_verbosity_Levels[] = {
   "IONIC_INFO",
   "IONIC_WARN",
   "IONIC_ERR",
   "IONIC_DBG"
};

#define ionic_LogNoLevel(urgency, logLevel, fmt, args...)                      \
   vmk_LogNoLevel(urgency,                                                     \
                  "<%s> " fmt "\n",                                            \
                  ionic_verbosity_Levels[logLevel],                            \
                  ##args)

#define ionic_LogLevel(urgency, logComp, logLevel, fmt, args...)               \
   vmk_LogLevel(urgency,                                                       \
                (logComp),                                                     \
                (logLevel),                                                    \
                "<%s> " fmt "\n",                                              \
                ionic_verbosity_Levels[logLevel],                              \
                __FUNCTION__,                                                  \
                __FILE__,                                                      \
                __LINE__,                                                      \
                ##args)

#define ionic_info(fmt, args...)                                               \
   ionic_LogNoLevel(VMK_LOG_URGENCY_NORMAL, IONIC_LOG_LEVEL_INFO, fmt, ##args)

#define ionic_warn(fmt, args...)                                               \
   ionic_LogNoLevel(VMK_LOG_URGENCY_WARNING, IONIC_LOG_LEVEL_WARN, fmt, ##args)

#define ionic_err(fmt, args...)                                                \
   ionic_LogNoLevel(VMK_LOG_URGENCY_ALERT, IONIC_LOG_LEVEL_ERR, fmt, ##args)

#define ionic_dbg(fmt, args...)                                                \
   ionic_LogNoLevel(VMK_LOG_URGENCY_DEBUG, IONIC_LOG_LEVEL_DBG, fmt, ##args);

#define ionic_print(fmt, args...)                                              \
   vmk_LogNoLevel(VMK_LOG_URGENCY_NORMAL, fmt, ##args)


#endif /* End of _IONIC_LOG_H_ */
