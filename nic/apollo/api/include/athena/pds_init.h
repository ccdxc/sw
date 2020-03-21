//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// This file defines the global and per thread initialisation APIs
///
//----------------------------------------------------------------------------


#ifndef __PDS_INIT_H__
#define __PDS_INIT_H__

#include "nic/apollo/api/include/pds_init.hpp"

#ifdef __cplusplus
extern "C" {
#endif

/// \brief     Global init
//  \param[in] params pds init parameters
/// \return    #SDK_RET_OK on success, failure status code on error
/// \remark    This needs to be called precisely once by application
///            from the control core
sdk_ret_t pds_global_init(pds_init_params_t *params);

/// \brief     Per thread init
/// \return    #SDK_RET_OK on success, failure status code on error
/// \remark    A valid core id should be passed
///            This needs to be called on every data core of the application
sdk_ret_t pds_thread_init(uint32_t core_id);

/// \brief     Global teardown
/// \remark    This needs to be called precisely once by application
///            from the control core after threads are stopped
void pds_global_teardown(void);

#ifdef __cplusplus
}
#endif

#endif // __PDS_INIT_H__
