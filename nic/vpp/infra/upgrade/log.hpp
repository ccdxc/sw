//
// {C} Copyright 2020 Pensando Systems Inc. All rights reserved
//

#ifndef __VPP_INFRA_IPC_UPGRADE_LOG_HPP__
#define __VPP_INFRA_IPC_UPGRADE_LOG_HPP__

#include <stdint.h>
#include <string.h>

extern "C" {

// upgrade log, to be used by CB functions
int upg_log_notice(const char *fmt, ...);
int upg_log_error(const char *fmt, ...);
void upg_log_facility_init();
}

#endif /* __VPP_INFRA_IPC_UPGRADE_LOG_HPP__ */

