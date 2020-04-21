// {C} Copyright 2017 Pensando Systems Inc. All rights reserved

#ifndef __ELBA_TOEPLITZ_HPP__
#define __ELBA_TOEPLITZ_HPP__

namespace sdk {
namespace platform {
namespace elba {

void elba_rss_table_config(uint32_t stage, uint32_t stage_tableid,
                             uint64_t tbl_base, uint64_t pc);
sdk_ret_t elba_toeplitz_init(const char *handle, int stage, int stage_tableid,
                              uint32_t rss_indir_tbl_size);
sdk_ret_t elba_rss_table_base_pc_get(const char *handle, uint64_t *tbl_base,
                                      uint64_t *pc);

}    // namespace elba
}    // namespace platform
}    // namespace sdk

#endif    // __ELBA_TOEPLITZ_HPP__
