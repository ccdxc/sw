//---------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
// Exception used by all PDSA stub components
//---------------------------------------------------------------

#ifndef __PDSA_ERR_HPP__
#define __PDSA_ERR_HPP__

#include <stdexcept>

namespace pdsa_stub {

class Error : public std::runtime_error {
public:
    Error(const char* err_str) : std::runtime_error(err_str) {};
    Error(const std::string& err_str) : std::runtime_error(err_str.c_str()) {};
};

}

#endif
