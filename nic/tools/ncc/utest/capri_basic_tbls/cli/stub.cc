/* This file is auto-generated. Changes will be overwritten! */
#include <stdint.h>
#include <string>
#include <iostream>
#include <thread>
#include "p4pd.h"
#include "p4pd_api.hpp"

std::thread *g_hal_threads[100];

namespace hal {
    thread_local std::thread *t_curr_thread;
}

typedef int p4pd_error_t;
p4pd_error_t p4pd_table_properties_get(uint32_t                       tableid,
                                                                      p4pd_table_properties_t       *tbl_ctx)
{
    return 0;
}
