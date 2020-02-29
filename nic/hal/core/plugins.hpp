//-----------------------------------------------------------------------------
// {C} Copyright 2018 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------

#pragma once

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <dlfcn.h>
#include <sys/queue.h>
#include "nic/include/base.hpp"
#include "nic/include/hal_cfg.hpp"
#include "nic/fte/fte_core.hpp"
#include "gen/proto/system.pb.h"

namespace hal {
namespace plugins {

struct feature_t;
struct plugin_t;
struct pipeline_t;

namespace pt = boost::property_tree;

class plugin_manager_t {
public:
    // Singleton instance
    static plugin_manager_t& get() {
        static plugin_manager_t instance_;
        return instance_;
    }

    // Parses the config
    void parse(const std::string& config, const std::string& plugin_path);

    // Loads/exits plugins with auto-load enabled
    void load(hal_cfg_t *hal_cfg);
    void exit();

    // Per-thread initialization and destruction
    void thread_init(int tid);
    void thread_exit(int tid);

    // delete default copy contructor and assignment operator
    plugin_manager_t(plugin_manager_t const&) = delete;
    void operator=(plugin_manager_t const&)  = delete;

private:
    std::unordered_map<std::string, plugin_t*> plugins_;
    STAILQ_HEAD(pipelines_head_, pipeline_t) pipelines_;

    // private constructor
    plugin_manager_t() {
        STAILQ_INIT(&pipelines_);
    };

    static std::string plugin_name(std::string& vendor, std::string& name) {
        return vendor + "/" + name;
    }

    static std::string feature_name(std::string& vendor,
                                    std::string& plugin_name,
                                    std::string& feature_name) {
        return vendor + "/" + plugin_name + ":" + feature_name;
    }

    bool parse_plugin(const pt::ptree &tree, plugin_t *plugin,
                      const std::string &plugin_path);
    bool parse_pipeline(const pt::ptree &tree, pipeline_t *pipeline);
    void parse_plugins(const pt::ptree &tree, const std::string &plugin_path);
    void parse_pipelines(const pt::ptree &tree);
    sys::ForwardMode parse_fwdmode_string_to_enum(const std::string& fwdmode);

    void register_features(plugin_t *plugin);
    void register_plugin(plugin_t *plugin);
    void register_pipeline(pipeline_t *pipeline);

    bool load_symbol(void *so, std::string name, void **symbol);
    bool load_symbols(void *so, plugin_t *plugin);
    bool load_plugin(hal_cfg_t *hal_cfg, plugin_t *plugin);
    void exit_plugin(plugin_t *plugin);

    void thread_init_plugin(plugin_t *plugin, int tid);
    void thread_exit_plugin(plugin_t *plugin, int tid);
};
}    // namepsace plugins

inline hal_ret_t init_plugins(hal_cfg_t *hal_cfg) {
    fte::init();
    // bool classic_nic = hal_cfg->device_cfg.forwarding_mode == sdk::lib::FORWARDING_MODE_CLASSIC;
    std::string plugin_file;

#if 0
    if (classic_nic) {
        plugin_file =  hal_cfg->cfg_path + std::string("/") +
                           std::string(hal_cfg->feature_set) +
                           std::string("/classic-plugins.json");
    } else {
        plugin_file =  hal_cfg->cfg_path + std::string("/") +
                           std::string(hal_cfg->feature_set) +
                           std::string("/plugins.json");
    }
#endif
    plugin_file =  hal_cfg->cfg_path + std::string("/") +
        std::string(hal_cfg->feature_set) +
        std::string("/plugins.json");
    std::string plugin_path = hal_cfg->cfg_path + std::string("/plugins");;

    plugins::plugin_manager_t &pluginmgr = plugins::plugin_manager_t::get();
    pluginmgr.parse(plugin_file, plugin_path);
    pluginmgr.load(hal_cfg);

    return HAL_RET_OK;
}

inline void exit_plugins() {
    plugins::plugin_manager_t &pluginmgr = plugins::plugin_manager_t::get();

    pluginmgr.exit();
}

inline void thread_init_plugins(int tid) {
    plugins::plugin_manager_t &pluginmgr = plugins::plugin_manager_t::get();

    pluginmgr.thread_init(tid);
}

inline void thread_exit_plugins(int tid) {
    plugins::plugin_manager_t &pluginmgr = plugins::plugin_manager_t::get();

    pluginmgr.thread_exit(tid);
}

}    // namespace hal
