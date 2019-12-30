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
    Error(const char* err_str) : std::runtime_error(err_str) {};
    Error(const std::string& err_str) : std::runtime_error(err_str.c_str()) {};
};

}

#endif
