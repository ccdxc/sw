/*
 * Copyright (c) 2019 Pensando Systems. All rights reserved.
 *
 * This software is provided to you under the terms of the GNU
 * General Public License (GPL) Version 2.0, available from the file
 * [ionic_log.h] in the main directory of this source tree.
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

#define IONIC_DRV_NAME        "ionic_en"

extern struct ionic_driver ionic_driver;

enum {
        IONIC_LOG_LEVEL_NOLOG = 0,
        IONIC_LOG_LEVEL_ERR,
        IONIC_LOG_LEVEL_WARN,
        IONIC_LOG_LEVEL_INFO,
        IONIC_LOG_LEVEL_DBG,
};

static const char *const ionic_verbosity_Levels[] = {
        "IONIC_NOLOG",
        "IONIC_ERR",
        "IONIC_WARN",
        "IONIC_INFO",
        "IONIC_DBG"
};

#define ionic_LogLevel(urgency, logLevel, fmt, args...)               \
   vmk_LogLevel(urgency,                                              \
                ionic_driver.log_component,                           \
                (logLevel),                                           \
                "<%s> %s: %s " fmt "\n",                              \
                ionic_verbosity_Levels[logLevel],                     \
                IONIC_DRV_NAME,                                       \
                __FUNCTION__,                                         \
                ##args)

#define ionic_err(fmt, args...)                                                \
   ionic_LogLevel(VMK_LOG_URGENCY_ALERT, IONIC_LOG_LEVEL_ERR, fmt, ##args)

#define ionic_warn(fmt, args...)                                               \
   ionic_LogLevel(VMK_LOG_URGENCY_WARNING, IONIC_LOG_LEVEL_WARN, fmt, ##args)

#define ionic_info(fmt, args...)                                               \
   ionic_LogLevel(VMK_LOG_URGENCY_NORMAL, IONIC_LOG_LEVEL_INFO, fmt, ##args)

#define ionic_dbg(fmt, args...)                                                \
   ionic_LogLevel(VMK_LOG_URGENCY_DEBUG, IONIC_LOG_LEVEL_DBG, fmt, ##args);

#define ionic_print(fmt, args...)                                              \
   ionic_LogLevel(VMK_LOG_URGENCY_DEBUG, IONIC_LOG_LEVEL_DBG, fmt, ##args);    \

#endif /* End of _IONIC_LOG_H_ */
