// {C} Copyright 2018 Pensando Systems Inc. All rights reserved

#include "platform/utils/program.hpp"

#define JSON_KEY_PROGRAMS        "programs"
#define JSON_KEY_SYMBOLS         "symbols"
#define JSON_KEY_NAME            "name"
#define JSON_KEY_BASE_ADDRESS    "base_addr"
#define JSON_KEY_END_ADDRESS     "end_addr"
#define JSON_KEY_ADDRESS         "addr"

namespace pt = boost::property_tree;

namespace sdk {
namespace platform {
namespace utils {

program_info *program_info::pinfo_ = NULL;

bool
program_info::init(const char *program_info_file, shmmgr *mmgr)
{
    pt::ptree        json_pt;
    prog_info_t      pinfo;
    symbol_info_t    sinfo;

    mmgr_ = mmgr;
    std::ifstream proginfo(program_info_file);
    read_json(proginfo, json_pt);

    BOOST_FOREACH(pt::ptree::value_type& pgm, json_pt.get_child(JSON_KEY_PROGRAMS)) {
        std::string program = pgm.second.get<std::string>(JSON_KEY_NAME);
        std::string base_addr = pgm.second.get<std::string>(JSON_KEY_BASE_ADDRESS);
        std::string end_addr = pgm.second.get<std::string>(JSON_KEY_END_ADDRESS);

        if (program_map_.find(program) != program_map_.end()) {
            SDK_TRACE_ERR("Program %s info exists already !! duplicate ??",
                          program.c_str());
            return false;
        }
        pinfo.base_addr = std::stol(base_addr);
        pinfo.end_addr = std::stol(end_addr);
        program_map_[program] = pinfo;
        // SDK_TRACE_ERR("Added program %s to map", program.c_str());

        // iterate through all the symbols now
        pt::ptree& symbols = pgm.second.get_child(JSON_KEY_SYMBOLS);
        BOOST_FOREACH(pt::ptree::value_type& sym, symbols) {
            std::string symbol = sym.second.get<std::string>(JSON_KEY_NAME);
            std::string addr = sym.second.get<std::string>(JSON_KEY_ADDRESS);

            if (symbol_map_.find(program + ":" + symbol) != symbol_map_.end()) {
                SDK_TRACE_ERR("Symbol %s:%s info exists already !! duplicate ??",
                              program.c_str(), symbol.c_str());
                return false;
            }
            sinfo.addr = std::stol(addr);
            symbol_map_[(program + ":" + symbol)] = sinfo;
            // SDK_TRACE_ERR("Added symbol %s to map with addr: %lu",
            //               (program + ":" + symbol).c_str(),
            //               sinfo.addr);
        }
    }

    return true;
}

program_info *
program_info::factory(shmmgr *mmgr)
{
    void            *mem;
    program_info    *new_program_info;

    if (pinfo_) {
        return pinfo_;
    }

    if (mmgr) {
        mem = mmgr->alloc(sizeof(program_info), 4, true);
    } else {
        mem = SDK_CALLOC(SDK_MEM_ALLOC_LIB_PLATFORM, sizeof(program_info));
    }
    if (mem == NULL) {
        SDK_TRACE_ERR("Failed to create program info instance");
        return NULL;
    }

    new_program_info = new (mem) program_info();
    pinfo_ = new_program_info;
    return new_program_info;
}

program_info *
program_info::factory(const char *program_info_file, shmmgr *mmgr)
{
    void            *mem;
    program_info    *new_program_info;

    if (program_info_file == nullptr) {
        return NULL;
    }

    if (access(program_info_file, R_OK) < 0) {
        SDK_TRACE_ERR("File %s doesn't exist or not accessible",
                      program_info_file);
        return NULL;
    }

    if (pinfo_) {
        return pinfo_;
    }

    if (mmgr) {
        mem = mmgr->alloc(sizeof(program_info), 4, true);
    } else {
        mem = SDK_CALLOC(SDK_MEM_ALLOC_LIB_PLATFORM, sizeof(program_info));
    }
    if (mem == NULL) {
        SDK_TRACE_ERR("Failed to create program info instance for %s",
                      program_info_file);
        return NULL;
    }

    new_program_info = new (mem) program_info();
    if (new_program_info->init(program_info_file) == false) {
        SDK_TRACE_ERR("Failed to initialize program info for %s",
                      program_info_file);
        new_program_info->~program_info();
        if (mmgr) {
            mmgr->free(mem);
        } else {
            SDK_FREE(SDK_MEM_ALLOC_LIB_PLATFORM, mem);
        }
        return NULL;
    }
    pinfo_ = new_program_info;
    return new_program_info;
}

program_info::~program_info()
{
}

void 
program_info::add_program(std::string program, mem_addr_t base_addr, mem_addr_t end_addr)
{
    prog_info_t pinfo;

    if (program_map_.find(program) != program_map_.end()) {
        SDK_TRACE_ERR("Program %s info exists already !! duplicate ??",
                      program.c_str());
        return;
    }

    // SDK_TRACE_DEBUG("Added program %s to map", program.c_str());
    pinfo.base_addr = base_addr;
    pinfo.end_addr = end_addr;
    program_map_[program] = pinfo;
}

void 
program_info::add_symbol(std::string program, std::string symbol, mem_addr_t addr)
{
    symbol_info_t sinfo;

    if (symbol_map_.find(program + ":" + symbol) != symbol_map_.end()) {
        SDK_TRACE_ERR("Symbol %s:%s info exists already !! duplicate ??",
                      program.c_str(), symbol.c_str());
        return;
    }

#if 0
    SDK_TRACE_DEBUG("Added symbol %s to map with addr: %lu",
                    (program + ":" + symbol).c_str(),
                    addr);
#endif
    sinfo.addr = addr;
    symbol_map_[(program + ":" + symbol)] = sinfo;
}

void
program_info::destroy(program_info *proginfo)
{
    shmmgr    *mmgr;

    if (!proginfo) {
        return;
    }

    mmgr = proginfo->mmgr_;
    proginfo->~program_info();
    if (mmgr) {
        mmgr->free(proginfo);
    } else {
        SDK_FREE(SDK_MEM_ALLOC_LIB_PLATFORM, proginfo);
    }
}

uint64_t
program_info::program_base_address(const char *program) const
{
    if (program == NULL) {
        return SDK_INVALID_HBM_ADDRESS;
    }
    auto it = program_map_.find(std::string(program));
    if (it == program_map_.end()) {
        return SDK_INVALID_HBM_ADDRESS;
    }
    return it->second.base_addr;
}

uint64_t
program_info::program_end_address(const char *program) const
{
    if (program == NULL) {
        return SDK_INVALID_HBM_ADDRESS;
    }
    auto it = program_map_.find(std::string(program));
    if (it == program_map_.end()) {
        return SDK_INVALID_HBM_ADDRESS;
    }
    return it->second.end_addr;
}

uint64_t
program_info::symbol_address(const char *program, char *symbol) const
{
    if ((program == NULL) || (symbol == NULL)) {
        return SDK_INVALID_HBM_ADDRESS;
    }

    auto it = symbol_map_.find(std::string(program) + ":" + std::string(symbol));
    if (it == symbol_map_.end()) {
        return SDK_INVALID_HBM_ADDRESS;
    }
    return it->second.addr;
}

}    // namespace utils
}    // namespace platform
}    // namespace sdk
