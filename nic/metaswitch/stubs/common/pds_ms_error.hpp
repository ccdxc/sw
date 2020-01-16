//---------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
// Exception used by all PDS-MS stub components
//---------------------------------------------------------------

#ifndef __PDS_MS_ERR_HPP__
#define __PDS_MS_ERR_HPP__

#include <stdexcept>

namespace pds_ms {

class Error : public std::runtime_error {
public:
    Error(const char* err_str, sdk_ret_t rc=SDK_RET_ERR)
        : std::runtime_error(err_str), rc_(rc) {};
    Error(const std::string& err_str, sdk_ret_t rc=SDK_RET_ERR)
        : std::runtime_error(err_str.c_str()), rc_(rc) {};
    sdk_ret_t rc() const {return rc_;}
private:
    sdk_ret_t rc_;
};

}

#endif
