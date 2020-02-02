// {C} Copyright 2018 Pensando Systems Inc. All rights reserved

#ifndef __SDK_PLATFORM_PROGRAM_HPP__
#define __SDK_PLATFORM_PROGRAM_HPP__

#include <string>
#include <map>
#include "include/sdk/base.hpp"
#include "include/sdk/mem.hpp"
#include "lib/shmmgr/shmmgr.hpp"
#include "include/sdk/types.hpp"
#include "boost/foreach.hpp"
#include "boost/optional.hpp"
#include "boost/property_tree/ptree.hpp"
#include "boost/property_tree/json_parser.hpp"
#include "include/sdk/platform.hpp"

namespace sdk {
namespace platform {
namespace utils {

typedef struct prog_info_s {
    mem_addr_t    base_addr;
    mem_addr_t    end_addr;
} __PACK__ prog_info_t;

typedef struct symbol_info_s {
    mem_addr_t    addr;
} __PACK__ symbol_info_t;

class program_info {
public:
    // constructor to build program info instance given full path of the json
    // file containing loader generated information
    static program_info *factory(const char *program_info_file, shmmgr *mmgr = NULL);
    static void destroy(program_info *program_info);
    mem_addr_t program_base_address(const char *program) const;
    mem_addr_t program_end_address(const char *program) const;
    mem_addr_t symbol_address(const char *program, char *symbol) const;
    static program_info *get_program_info(void) { return pinfo_; }

private:
    shmmgr                                  *mmgr_;
    std::map<std::string, prog_info_t>      program_map_;
    std::map<std::string, symbol_info_t>    symbol_map_;
    static program_info *pinfo_;

private:
    program_info() {};
    ~program_info();
    bool init(const char *program_info_file, shmmgr *mmgr = NULL);

};

}    // namespace utils
}    // namespace platform
}    // namespace sdk

using sdk::platform::utils::program_info;

#endif // __SDK_PLATFORM_PROGRAM_HPP__
