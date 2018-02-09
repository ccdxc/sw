#pragma once

#include "nic/include/base.h"
#include "nic/include/fte.hpp"
#include "nic/hal/plugins/proxy/proxy_plugin.hpp"
#include <dlfcn.h>
#include <sys/queue.h>

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>

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
    void load();
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

    void register_features(plugin_t *plugin);
    void register_plugin(plugin_t *plugin);
    void register_pipeline(pipeline_t *pipeline);

    bool load_symbol(void *so, std::string name, void **symbol);
    bool load_symbols(void *so, plugin_t *plugin);
    bool load_plugin(plugin_t *plugin);
    void exit_plugin(plugin_t *plugin);

    void thread_init_plugin(plugin_t *plugin, int tid);
    void thread_exit_plugin(plugin_t *plugin, int tid);
};
} // namepsace plugins

inline hal_ret_t init_plugins(bool classic_nic) {
    fte::init();

    const char* cfg_path =  std::getenv("HAL_CONFIG_PATH");
    HAL_ASSERT(cfg_path);

    std::string plugin_file;

    if (classic_nic) {
        plugin_file =  std::string(cfg_path) + std::string("/classic-plugins.json");
    } else {
        plugin_file =  std::string(cfg_path) + std::string("/plugins.json");
    }

    std::string plugin_path = std::string(cfg_path) + std::string("/plugins");;

    plugins::plugin_manager_t &pluginmgr = plugins::plugin_manager_t::get();

    pluginmgr.parse(plugin_file, plugin_path);
    pluginmgr.load();
    hal::proxy::proxy_plugin_init();

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


} // namespace hal
