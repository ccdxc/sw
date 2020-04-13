// {C} Copyright 2020 Pensando Systems Inc. All rights reserved

#ifndef __SDK_ASIC_PD_INTERNAL_HPP__
#define __SDK_ASIC_PD_INTERNAL_HPP__

namespace sdk {
namespace asic {
namespace pd {

sdk_ret_t asic_program_hbm_table_base_addr(int tableid, int stage_tableid,
                                           char *tablename, int stage,
                                           int pipe);

}    // namespace pd
}    // namespace asic
}    // namespace sdk

#endif	// __SDK_ASIC_PD_INTERNAL_HPP__
