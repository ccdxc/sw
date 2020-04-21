//-----------------------------------------------------------------------------
// {C} Copyright 2020 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------

#ifndef __PDSCTL_PDS_AGENT_INIT_H__
#define __PDSCTL_PDS_AGENT_INIT_H__

#include "nic/apollo/api/include/athena/pds_base.h"

#ifdef __cplusplus
extern "C" {
#endif

/// \brief     pds agent init
/// \return    #PDS_RET_OK on success, failure status code on error
/// \remark    This needs to be called when pdsctl is needed
pds_ret_t pds_agent_init();

#ifdef __cplusplus
}
#endif

#endif    // __PDSCTL_PDS_AGENT_INIT_H__
