//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//

#ifndef __SCALE_HPP__
#define __SCALE_HPP__

#include "nic/sdk/include/sdk/base.hpp"
#include "nic/apollo/test/api/utils/base.hpp"

#define MHTEST_CHECK_RETURN(_exp, _ret) if (!(_exp)) return (_ret)

namespace test {
namespace api {

pds_ret_t create_helper(uint32_t index, pds_ret_t expret);
pds_ret_t delete_helper(uint32_t index, pds_ret_t expret);
pds_ret_t update_helper(uint32_t index, pds_ret_t expret);
pds_ret_t read_helper(uint32_t index, pds_ret_t expret);

static pds_ret_t create_entries(uint32_t count, pds_ret_t expret) {
    for (auto i = 1; i <= count; i++) {
        auto rs = create_helper(i, expret);
        MHTEST_CHECK_RETURN(rs == expret, PDS_RET_MAX);
    }
    return PDS_RET_OK;
}

static pds_ret_t delete_entries(uint32_t count, pds_ret_t expret) {
    for (auto i = 1; i <= count; i++) {
        auto rs = delete_helper(i, expret);
        MHTEST_CHECK_RETURN(rs == expret, PDS_RET_MAX);
    }
    return PDS_RET_OK;
}

static pds_ret_t update_entries(uint32_t count, pds_ret_t expret) {
    for (auto i = 1; i <= count; i++) {
        auto rs = update_helper(i, expret);
        MHTEST_CHECK_RETURN(rs == expret, PDS_RET_MAX);
    }
    return PDS_RET_OK;
}

static pds_ret_t read_entries(uint32_t count, pds_ret_t expret) {
    for (auto i = 1; i <= count; i++) {
        auto rs = read_helper(i, expret);
        MHTEST_CHECK_RETURN(rs == expret, PDS_RET_MAX);
    }
    return PDS_RET_OK;
}

}    // namespace api
}    // namespace test

#endif // __SCALE_HPP__
