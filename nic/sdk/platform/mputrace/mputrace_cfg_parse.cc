//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//
//===----------------------------------------------------------------------===//
///
/// \file
/// This file contains implementation to parse mputrace cfg from a json file
///
//===----------------------------------------------------------------------===//

#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <regex>
#include <string>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include "platform/mputrace/mputrace.hpp"

using boost::property_tree::ptree;
using std::string;
using namespace std;

namespace sdk {
namespace platform {

#define MPUTRACE_MATCH_ALL ".*"

std::map<std::string, int> pipeline_str_to_id = {
    {"txdma", TXDMA},
    {"rxdma", RXDMA},
    {"p4ig", P4IG},
    {"p4eg", P4EG},
};
std::map<int, int> max_stages = {{TXDMA, 7}, {RXDMA, 7}, {P4IG, 5}, {P4EG, 5}};
std::map<std::string, int> trace_options = {
    {"trace_enable", MPUTRACE_TRACE_ENABLE},
    {"phv_debug", MPUTRACE_PHV_DEBUG},
    {"phv_error", MPUTRACE_PHV_ERROR},
    {"table_key_enable", MPUTRACE_TBL_KEY_ENABLE},
    {"instr_enable", MPUTRACE_INSTR_ENABLE},
    {"wrap", MPUTRACE_WRAP},
    {"watch_pc", MPUTRACE_WATCH_PC},
    {"trace_size", MPUTRACE_TRACE_SIZE},
};

std::string
mputrace_pipeline_str_get (int pipeline_num) {
    switch (pipeline_num) {
    case TXDMA:
        return std::string("txdma");
    case RXDMA:
        return std::string("rxdma");
    case P4IG:
        return std::string("p4ig");
    case P4EG:
        return std::string("p4eg");
    default:
        return std::string("");
    }
}

static inline void
mputrace_cfg_inst_init (mputrace_cfg_inst_t *cfg_inst)
{
    if (cfg_inst == NULL) {
        cout << "Error: captrace cfg instance is NULL." << endl;
        assert(0);
    }
    memset(cfg_inst, 0, sizeof(mputrace_cfg_inst_t));
    cfg_inst->pipeline_str.assign(MPUTRACE_MATCH_ALL);
    cfg_inst->stage_str.assign(MPUTRACE_MATCH_ALL);
    cfg_inst->mpu_str.assign(MPUTRACE_MATCH_ALL);
    cfg_inst->ctrl.trace_enable = false;
    cfg_inst->ctrl.phv_debug = false;
    cfg_inst->ctrl.phv_error = false;
    cfg_inst->ctrl.watch_pc = 0;
    cfg_inst->capture.table_key = false;
    cfg_inst->capture.instructions = false;
    cfg_inst->settings.reset = false;
    cfg_inst->settings.wrap = false;
    cfg_inst->settings.trace_addr = 0;
    cfg_inst->settings.trace_size = 4096;
}

static inline uint64_t
mputrace_json_get_pc (std::string pgm_or_lbl)
{
    boost::property_tree::ptree pt;
    bool is_pgm = (pgm_or_lbl.find(".bin") != std::string::npos);
    uint64_t pgm_base = 0;

    boost::property_tree::read_json("/nic/conf/gen/mpu_prog_info.json", pt);
    for (auto pgm : pt.get_child("programs")) {
        pgm_base = strtoul(pgm.second.get<std::string>("base_addr_hex").c_str(), NULL, 16);
        if (is_pgm && pgm.second.get<std::string>("name") == pgm_or_lbl) {
            return pgm_base;
        } else {
            for (auto lbl : pgm.second.get_child("symbols")) {
                if (lbl.second.get<std::string>("name") == pgm_or_lbl) {
                    return pgm_base + strtoul(lbl.second.get<std::string>("addr_hex").c_str(), NULL, 16);
                }
            }
        }
    }

    return 0;
}

static inline void
mputrace_json_ctrl_parse (ptree &pt, mputrace_cfg_inst_t *cfg_inst)
{
    boost::optional<bool> val;
    boost::optional<std::string> str;

    val = pt.get_optional<bool>("trace");
    if (val) {
        cfg_inst->ctrl.trace_enable = val.get();
    }
    val = pt.get_optional<bool>("phv-debug");
    if (val) {
        cfg_inst->ctrl.phv_debug = val.get();
    }
    val = pt.get_optional<bool>("phv-error");
    if (val) {
        cfg_inst->ctrl.phv_error = val.get();
    }
    str = pt.get_optional<std::string>("watch-pc");
    if (str) {
        cfg_inst->ctrl.watch_pc = mputrace_json_get_pc(str.get());
    }
}

static inline void
mputrace_json_capture_parse (ptree &pt, mputrace_cfg_inst_t *cfg_inst)
{
    boost::optional<bool> val;

    val = pt.get_optional<bool>("key-data");
    if (val) {
        cfg_inst->capture.table_key = val.get();
    }
    val = pt.get_optional<bool>("instructions");
    if (val) {
        cfg_inst->capture.instructions = val.get();
    }
}

static inline void
mputrace_json_settings_parse (ptree &pt, mputrace_cfg_inst_t *cfg_inst)
{
    boost::optional<std::string> str =
        pt.get_optional<std::string>("trace-size");
    boost::optional<bool> val;

    if (str) {
        cfg_inst->settings.trace_size = atoi(str.get().c_str());
    }
    val = pt.get_optional<bool>("wrap");
    if (val) {
        cfg_inst->settings.wrap = val.get();
    }
}

static inline bool
mputrace_is_match (std::string num_str, std::string str)
{
    regex reg_exp(str);
    if (regex_match(num_str, reg_exp)) {
        return true;
    }
    return false;
}

static inline void
mputrace_json_cfg_inst_parse (ptree &pt, mputrace_cfg_inst_t *cfg_inst)
{
    mputrace_cfg_inst_init(cfg_inst);
    for (ptree::iterator pos = pt.begin(); pos != pt.end();) {
        if (strcmp(pos->first.data(), "pipeline") == 0) {
            cfg_inst->pipeline_str = pos->second.data();
        } else if (strcmp(pos->first.data(), "stage") == 0) {
            cfg_inst->stage_str = pos->second.data();
        } else if (strcmp(pos->first.data(), "mpu") == 0) {
            cfg_inst->mpu_str = pos->second.data();
        } else if (strcmp(pos->first.data(), "control") == 0) {
            mputrace_json_ctrl_parse(pos->second, cfg_inst);
        } else if (strcmp(pos->first.data(), "capture") == 0) {
            mputrace_json_capture_parse(pos->second, cfg_inst);
        } else if (strcmp(pos->first.data(), "settings") == 0) {
            mputrace_json_settings_parse(pos->second, cfg_inst);
        }
        pos++;
    }
}

// Read each pipeline object from the json file
void
mputrace_json_cfg_inst_program (ptree &pt)
{
    mputrace_cfg_inst_t cfg_inst;
    mputrace_json_cfg_inst_parse(pt, &cfg_inst);

    for (int p = 0; p < PIPE_CNT; p++) {
        if (mputrace_is_match(mputrace_pipeline_str_get(p),
                              cfg_inst.pipeline_str)) {
            for (int s = 0; s <= max_stages[p]; s++) {
                if (mputrace_is_match(std::to_string(s), cfg_inst.stage_str)) {
                    for (int m = 0; m < MPUTRACE_MAX_MPU; m++) {
                        if (mputrace_is_match(std::to_string(m),
                                              cfg_inst.mpu_str)) {
                            mputrace_cfg_trace(p, s, m, &cfg_inst);
                        }
                    }
                }
            }
        }
    }
}

static inline void
mputrace_json_parse_and_program (const char *cfg_file)
{
    ptree pt, iter;
    ptree::iterator pos;
    std::ifstream json_cfg(cfg_file);

    read_json(json_cfg, pt);
    pos = pt.begin();
    if (strcmp(pos->first.data(), "instances") != 0) {
        cout << "Please check /nic/conf/captrace/ for sample config json files."
             << endl;
        exit(EXIT_FAILURE);
    }

    iter = pos->second;
    for (pos = iter.begin(); pos != iter.end();) {
        if (strcmp(pos->first.data(), "") == 0) {
            mputrace_json_cfg_inst_program(pos->second);
        }
        ++pos;
    }
}

void
mputrace_cfg (const char *cfg_file)
{
    mputrace_json_parse_and_program(cfg_file);
    cout << "'captrace conf " << cfg_file << "' success!" << endl;
}

}    // end namespace platform
}    // end namespace sdk
