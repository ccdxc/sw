//
// {C} Copyright 2020 Pensando Systems Inc. All rights reserved
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
#include "platform/elbtrace/elbtrace.hpp"

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
std::map<int, int> max_stages = {{TXDMA, 7}, {RXDMA, 7}, {P4IG, 7}, {P4EG, 7}};
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

std::string
dmatrace_pipeline_str_get (int pipeline_num) {
    switch (pipeline_num) {
    case TXPDMA:
        return std::string("txdma");
    case RXPDMA:
        return std::string("rxdma");
    default:
        return std::string("");
    }
}

static inline void
mputrace_cfg_inst_init (mputrace_cfg_inst_t *cfg_inst)
{
    if (cfg_inst == NULL) {
        cout << "Error: elbtrace cfg instance is NULL." << endl;
        assert(0);
    }
    memset(cfg_inst, 0, sizeof(mputrace_cfg_inst_t));
    cfg_inst->pipeline_str.assign(MPUTRACE_MATCH_ALL);
    cfg_inst->stage_str.assign(MPUTRACE_MATCH_ALL);
    cfg_inst->mpu_str.assign(MPUTRACE_MATCH_ALL);
    cfg_inst->ctrl.trace_enable = false;
    cfg_inst->ctrl.phv_debug = false;
    cfg_inst->ctrl.phv_error = false;
    //    cfg_inst->ctrl.watch_pc = 0;
    cfg_inst->ctrl.watch_pc.trace = false;
    cfg_inst->ctrl.watch_pc.count = false;
    cfg_inst->ctrl.watch_pc.intr = false;
    cfg_inst->ctrl.watch_pc.stop = false;
    cfg_inst->ctrl.watch_pc.exception = false;
    cfg_inst->ctrl.watch_pc.addr_lo = 0;
    cfg_inst->ctrl.watch_pc.addr_hi = 0;

    cfg_inst->ctrl.watch_data.rtrace = false;
    cfg_inst->ctrl.watch_data.wtrace = false;
    cfg_inst->ctrl.watch_data.rintr = false;
    cfg_inst->ctrl.watch_data.wintr = false;
    cfg_inst->ctrl.watch_data.rstop = false;
    cfg_inst->ctrl.watch_data.wstop = false;
    cfg_inst->ctrl.watch_data.rexception = false;
    cfg_inst->ctrl.watch_data.wexception = false;
    cfg_inst->ctrl.watch_data.addr_lo = 0;
    cfg_inst->ctrl.watch_data.addr_hi = 0;

    cfg_inst->capture.table_key = false;
    cfg_inst->capture.instructions = false;
    cfg_inst->settings.reset = false;
    cfg_inst->settings.wrap = false;
    cfg_inst->settings.trace_addr = 0;
    cfg_inst->settings.trace_size = 4096; //todo:
}

static inline void
sdptrace_cfg_inst_init (sdptrace_cfg_inst_t *cfg_inst)
{
    if (cfg_inst == NULL) {
        cout << "Error: elbtrace SDP cfg instance is NULL." << endl;
        assert(0);
    }
    memset(cfg_inst, 0, sizeof(sdptrace_cfg_inst_t));
    cfg_inst->pipeline_str.assign(MPUTRACE_MATCH_ALL);
    cfg_inst->stage_str.assign(MPUTRACE_MATCH_ALL);

    cfg_inst->ctrl.enable = false;
    cfg_inst->ctrl.trace_trigger_enable = false;

    //    for (uint32_t i = 0; i < 8; i++) {
      //      cfg_inst->capture.trigger_data[i] = i;
      //cfg_inst->capture.trigger_mask[i] = i;
    //}
    cfg_inst->capture.trigger_data_p4 = 0;
    cfg_inst->capture.trigger_mask_p4 = 0;
    cfg_inst->capture.trigger_data_tx = 0;
    cfg_inst->capture.trigger_mask_tx = 0;
    cfg_inst->capture.trigger_data_rx = 0;
    cfg_inst->capture.trigger_mask_rx = 0;
    
    cfg_inst->settings.sw_reset_enable = false;
    cfg_inst->settings.no_trace_when_full = false;
    cfg_inst->settings.stop_when_full = false;
    cfg_inst->settings.ring_size = 4096; //todo
    cfg_inst->settings.phv_base_addr = 0;
    cfg_inst->settings.ctl_base_addr = 0;

}

static inline void
dmatrace_cfg_inst_init (dmatrace_cfg_inst_t *cfg_inst)
{
    if (cfg_inst == NULL) {
        cout << "Error: elbtrace DMA cfg instance is NULL." << endl;
        assert(0);
    }
    memset(cfg_inst, 0, sizeof(dmatrace_cfg_inst_t));
    cfg_inst->pipeline_str.assign(MPUTRACE_MATCH_ALL);

    cfg_inst->ctrl.enable = false;
    cfg_inst->ctrl.phv_enable = false;
    cfg_inst->ctrl.capture_all = false;
    cfg_inst->ctrl.axi_err_enable = false;
    cfg_inst->ctrl.pkt_phv_sync_err_enable = false;

    cfg_inst->settings.wrap = false;
    cfg_inst->settings.reset = false;
    cfg_inst->settings.buf_size  = 4096; //todo
    cfg_inst->settings.base_addr = 0;
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

static inline uint64_t
mputrace_json_get_pc_addr (std::string pgm_or_lbl, int hi_lo)
{
    boost::property_tree::ptree pt;
    bool is_pgm = (pgm_or_lbl.find(".bin") != std::string::npos);
    uint64_t pgm_base = 0;

    boost::property_tree::read_json("conf/gen/mpu_prog_info.json", pt);
    for (auto pgm : pt.get_child("programs")) {
      if (hi_lo == 0) {
        pgm_base = strtoul(pgm.second.get<std::string>("base_addr_hex").c_str(), NULL, 16);
      } else {
        pgm_base = strtoul(pgm.second.get<std::string>("end_addr_hex").c_str(), NULL, 16);
      }
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
mputrace_json_ctrl_watch_pc_parse (ptree &pt, mputrace_cfg_inst_t *cfg_inst)
{
    boost::optional<bool> val;
    boost::optional<std::string> str;

    val = pt.get_optional<bool>("trace");
    if (val) {
        cfg_inst->ctrl.watch_pc.trace = val.get();
    }
    val = pt.get_optional<bool>("count");
    if (val) {
        cfg_inst->ctrl.watch_pc.count = val.get();
    }
    val = pt.get_optional<bool>("intr");
    if (val) {
        cfg_inst->ctrl.watch_pc.intr = val.get();
    }
    val = pt.get_optional<bool>("stop");
    if (val) {
        cfg_inst->ctrl.watch_pc.stop = val.get();
    }
    val = pt.get_optional<bool>("exception");
    if (val) {
        cfg_inst->ctrl.watch_pc.exception = val.get();
    }

    str = pt.get_optional<std::string>("addr_lo");
    if (str) {
      cfg_inst->ctrl.watch_pc.addr_lo = mputrace_json_get_pc_addr(str.get(), 0);
    }

    str = pt.get_optional<std::string>("addr_hi");
    if (str) {
      cfg_inst->ctrl.watch_pc.addr_hi = mputrace_json_get_pc_addr(str.get(), 1);
    }
}

static inline void
mputrace_json_ctrl_watch_data_parse (ptree &pt, mputrace_cfg_inst_t *cfg_inst)
{
    boost::optional<bool> val;
    boost::optional<std::string> str;

    val = pt.get_optional<bool>("rtrace");
    if (val) {
        cfg_inst->ctrl.watch_data.rtrace = val.get();
    }
    val = pt.get_optional<bool>("wtrace");
    if (val) {
        cfg_inst->ctrl.watch_data.wtrace = val.get();
    }
    val = pt.get_optional<bool>("rintr");
    if (val) {
        cfg_inst->ctrl.watch_data.rintr = val.get();
    }
    val = pt.get_optional<bool>("wintr");
    if (val) {
        cfg_inst->ctrl.watch_data.wintr = val.get();
    }
    val = pt.get_optional<bool>("rstop");
    if (val) {
        cfg_inst->ctrl.watch_data.rstop = val.get();
    }
    val = pt.get_optional<bool>("wstop");
    if (val) {
        cfg_inst->ctrl.watch_data.wstop = val.get();
    }
    val = pt.get_optional<bool>("rexception");
    if (val) {
        cfg_inst->ctrl.watch_data.rexception = val.get();
    }
    val = pt.get_optional<bool>("wexception");
    if (val) {
        cfg_inst->ctrl.watch_data.wexception = val.get();
    }

    str = pt.get_optional<std::string>("addr_lo");
    if (str) {

      cfg_inst->ctrl.watch_data.addr_lo = strtoul(str.get().data(), NULL, 16);
    }

    str = pt.get_optional<std::string>("addr_hi");
    if (str) {
      cfg_inst->ctrl.watch_data.addr_hi = strtoul(str.get().data(), NULL, 16);
    }

}

static inline void
mputrace_json_ctrl_parse (ptree &pt, mputrace_cfg_inst_t *cfg_inst)
{
    boost::optional<bool> val;
    boost::optional<std::string> str;

    //    for (ptree::iterator pos = pt.begin(); pos != pt.end();) {
    //  cout << "justina test " << pos->first.data() << endl;
    //  val = pt.get_optional<bool>(pos->first.data());
    //  //call a macro
    //  JUSTINA_MACRO(pos->first.data(), val);
    //  pos++;
    // }

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
    //    str = pt.get_optional<std::string>("watch-pc");
    //    if (str) {
    //    cfg_inst->ctrl.watch_pc = mputrace_json_get_pc(str.get());
    //}


    for (ptree::iterator pos = pt.begin(); pos != pt.end();) {
        if (strcmp(pos->first.data(), "watch_pc") == 0) {
            mputrace_json_ctrl_watch_pc_parse(pos->second, cfg_inst);
        }
        else if (strcmp(pos->first.data(), "watch_data") == 0) {
            mputrace_json_ctrl_watch_data_parse(pos->second, cfg_inst);
        }
        pos++;
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

  //////// SDP  ////////////

static inline void
sdptrace_json_ctrl_parse (ptree &pt, sdptrace_cfg_inst_t *cfg_inst)
{
    boost::optional<bool> val;
    boost::optional<std::string> str;

    val = pt.get_optional<bool>("trace");
    if (val) {
        cfg_inst->ctrl.enable = val.get();
    }
    val = pt.get_optional<bool>("trace_trigger_enable");
    if (val) {
        cfg_inst->ctrl.trace_trigger_enable = val.get();
    }
}


static inline void
sdptrace_json_settings_parse (ptree &pt, sdptrace_cfg_inst_t *cfg_inst)
{
    boost::optional<std::string> str =
        pt.get_optional<std::string>("trace-size");
    boost::optional<bool> val;

    if (str) {
        cfg_inst->settings.ring_size = atoi(str.get().c_str());
    }
    //    val = pt.get_optional<bool>("sw_reset_enable");   //todo: shd this be part of reset and not json??
    //    if (val) {
    //        cfg_inst->settings.sw_reset_enable = val.get();
    //    }
    val = pt.get_optional<bool>("no_trace_when_full");
    if (val) {
        cfg_inst->settings.no_trace_when_full = val.get();
    }
    val = pt.get_optional<bool>("stop_when_full");
    if (val) {
        cfg_inst->settings.stop_when_full = val.get();
    }
}

  //////// DMA  ////////////

static inline void
dmatrace_json_ctrl_parse (ptree &pt, dmatrace_cfg_inst_t *cfg_inst)
{
    boost::optional<bool> val;
    boost::optional<std::string> str;

    val = pt.get_optional<bool>("trace");
    if (val) {
        cfg_inst->ctrl.enable = val.get();
	cout << "ctrl parse enable " << val.get() << endl;
    }
    val = pt.get_optional<bool>("phv_enable");
    if (val) {
        cfg_inst->ctrl.phv_enable = val.get();
    }
    val = pt.get_optional<bool>("capture_all");
    if (val) {
        cfg_inst->ctrl.capture_all = val.get();
    }
    val = pt.get_optional<bool>("axi_err_enable");
    if (val) {
        cfg_inst->ctrl.axi_err_enable = val.get();
    }
    val = pt.get_optional<bool>("pkt_phv_sync_err_enable");
    if (val) {
        cfg_inst->ctrl.pkt_phv_sync_err_enable = val.get();
    }
}

static inline void
dmatrace_json_settings_parse (ptree &pt, dmatrace_cfg_inst_t *cfg_inst)
{
    boost::optional<std::string> str =
        pt.get_optional<std::string>("trace-size");
    boost::optional<bool> val;

    if (str) {
        cfg_inst->settings.buf_size = atoi(str.get().c_str());
    }
    val = pt.get_optional<bool>("wrap");
    if (val) {
      cfg_inst->settings.wrap = val.get();
    }
    //program rst bit directly in register for dmatrace reset
}



static inline bool
elbtrace_is_match (std::string num_str, std::string str)
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

static inline void
sdptrace_json_cfg_inst_parse (ptree &pt, sdptrace_cfg_inst_t *cfg_inst)
{
    sdptrace_cfg_inst_init(cfg_inst);
    for (ptree::iterator pos = pt.begin(); pos != pt.end();) {
        if (strcmp(pos->first.data(), "pipeline") == 0) {
            cfg_inst->pipeline_str = pos->second.data();
        } else if (strcmp(pos->first.data(), "stage") == 0) {
            cfg_inst->stage_str = pos->second.data();
        } else if (strcmp(pos->first.data(), "control") == 0) {
            sdptrace_json_ctrl_parse(pos->second, cfg_inst);
        } else if (strcmp(pos->first.data(), "capture") == 0) {
	  //todo: change the function below to return structs for all pipes
            sdptrace_json_capture_parse(pos->second, cfg_inst);
        } else if (strcmp(pos->first.data(), "settings") == 0) {
            sdptrace_json_settings_parse(pos->second, cfg_inst);
        }
        pos++;
    }
    
}

static inline void
dmatrace_json_cfg_inst_parse (ptree &pt, dmatrace_cfg_inst_t *cfg_inst)
{
    dmatrace_cfg_inst_init(cfg_inst);
    for (ptree::iterator pos = pt.begin(); pos != pt.end();) {
        if (strcmp(pos->first.data(), "pipeline") == 0) {
            cfg_inst->pipeline_str = pos->second.data();
        } else if (strcmp(pos->first.data(), "control") == 0) {
            dmatrace_json_ctrl_parse(pos->second, cfg_inst);
        } else if (strcmp(pos->first.data(), "settings") == 0) {
            dmatrace_json_settings_parse(pos->second, cfg_inst);
        }
        pos++;
    }

    cout << "\n dma_cfg_inst_parse begin" << endl;
    cout << "buf_size " << dec << cfg_inst->settings.buf_size  << endl;
    cout << "wrap "           <<  cfg_inst->settings.wrap	  << endl;
    cout << "reset "          <<  cfg_inst->settings.reset	  << endl;
    cout << "phv_enable "     <<  cfg_inst->ctrl.phv_enable	  << endl;
    cout << "capture_all "    <<  cfg_inst->ctrl.capture_all	  << endl;
    cout << "axi_err_enable " <<  cfg_inst->ctrl.axi_err_enable   << endl;
    cout << "dma_cfg_inst_parse end \n" << endl;
    



}

// Read each pipeline object from the json file
void
elbtrace_json_cfg_inst_program (ptree &pt, std::string mod_name)
{

  
    mputrace_cfg_inst_t cfg_inst;
    sdptrace_cfg_inst_t cfg_inst_sdp;
    dmatrace_cfg_inst_t cfg_inst_dma;

    if (mod_name == "mpu") {
      mputrace_json_cfg_inst_parse(pt, &cfg_inst);
      
      for (int p = 0; p < PIPE_CNT; p++) {
        if (elbtrace_is_match(mputrace_pipeline_str_get(p),
                              cfg_inst.pipeline_str)) {
	  for (int s = 0; s <= max_stages[p]; s++) {
	    if (elbtrace_is_match(std::to_string(s), cfg_inst.stage_str)) {
	      for (int m = 0; m < MPUTRACE_MAX_MPU; m++) {
		if (elbtrace_is_match(std::to_string(m),
				      cfg_inst.mpu_str)) {
		  mputrace_cfg_trace(p, s, m, &cfg_inst);
		}
	      }
	    }
	  }
        }
      }
    }
    else if (mod_name == "sdp") {
      //      cout << "before sdptrace_json_cfg_inst_parse" << endl;
      sdptrace_json_cfg_inst_parse(pt, &cfg_inst_sdp);
      
      for (int p = 0; p < PIPE_CNT; p++) {
        if (elbtrace_is_match(mputrace_pipeline_str_get(p),
                              cfg_inst_sdp.pipeline_str)) {
	  for (int s = 0; s <= max_stages[p]; s++) {
	    if (elbtrace_is_match(std::to_string(s), cfg_inst_sdp.stage_str)) {
	      //	      cout << "before sdptrace_cfg_trace" << endl;
	      sdptrace_cfg_trace(p, s, &cfg_inst_sdp);
	    }
	  }
        }
      }
    }
    else if (mod_name == "dma") {
      dmatrace_json_cfg_inst_parse(pt, &cfg_inst_dma);
      cout << "json_cfg_inst_program" << endl;
      
      for (int p = 0; p < DMA_PIPE_CNT; p++) {
	cout << "json_cfg_inst_program pipe " << p << endl;
	
	cout << "pipeline str is " << cfg_inst_dma.pipeline_str << endl;
	cout << "pipeline str get is " << dmatrace_pipeline_str_get(p) << endl;
        if (elbtrace_is_match(dmatrace_pipeline_str_get(p),
                              cfg_inst_dma.pipeline_str)) {
	  cout << "json_cfg_inst_program inside if " << endl;
	  dmatrace_cfg_trace(p, &cfg_inst_dma);
	  cout << "json_cfg_inst_program end if " << endl;
        }
      }
    }

}

static inline void
elbtrace_json_parse_and_program (const char *cfg_file, std::string mod_name)
{
    ptree pt, iter;
    ptree::iterator pos;
    std::ifstream json_cfg(cfg_file);

    read_json(json_cfg, pt);
    pos = pt.begin();
    if (strcmp(pos->first.data(), "instances") != 0) {
        cout << "Please check /nic/conf/elbtrace/ for sample config json files."
             << endl;
        exit(EXIT_FAILURE);
    }

    iter = pos->second;
    for (pos = iter.begin(); pos != iter.end();) {
        if (strcmp(pos->first.data(), "") == 0) {
	  //	  cout << "justina" << pos->first.data() << endl;
	  //cout << "inside elbtrace_json_parse_and_program" << endl;
	  elbtrace_json_cfg_inst_program(pos->second, mod_name);
        }
        ++pos;
    }
}

void
elbtrace_cfg (const char *cfg_file, std::string mod_name)
{

  if ( !((mod_name == "mpu") || 
	 (mod_name == "sdp") || 
	 (mod_name == "dma")) ) {
	 cout << "Module name did not match mpu, sdp or dma. Incorrect argument." << endl;
       }
  else {
     if (mod_name == "mpu") {
      cout << "Initiating MPU config from file " << cfg_file << endl;
     }
     else if (mod_name == "sdp") {
       cout << "Initiating SDP config from file " << cfg_file << endl;
     }
     else if (mod_name == "dma") {
       cout << "Initiating DMA config from file " << cfg_file << endl;
     }
     elbtrace_json_parse_and_program(cfg_file, mod_name);
     cout << "'elbtrace conf " << cfg_file << "' success!" << endl;
  }
  
}
  
         
}    // end namespace platform
}    // end namespace sdk
