// {C} Copyright 2018 Pensando Systems Inc. All rights reserved

#ifndef __REPL_DECODE_HPP__
#define __REPL_DECODE_HPP__

#include "nic/include/base.hpp"

namespace hal {
namespace pd {
uint32_t repl_entry_data_to_str(void *repl_entry_data,
                                char *buff, uint32_t buff_size);
}
}

#endif
