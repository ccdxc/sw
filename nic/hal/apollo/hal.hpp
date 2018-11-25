//------------------------------------------------------------------------------
// {C} Copyright 2018 Pensando Systems Inc. All rights reserved
//------------------------------------------------------------------------------

#ifndef __HAL_HPP__
#define __HAL_HPP__

namespace hal {

// top level initialization function for HAL
// returns 0, if successful or else -ve number
int init(char *hal_cfg_file_path);

}    // namespace hal

#endif    // __HAL_HPP__

