#include "nic/include/base.h"
#include "nic/include/fte.hpp"
#include "nic/gen/proto/hal/types.pb.h"
#include "nic/hal/src/proxy.hpp"

#include "plugins.hpp"

#include <dlfcn.h>

namespace hal {
namespace plugins {

typedef hal_ret_t (*init_handler_t)();
typedef void (*exit_handler_t)();

enum plugin_state_t {
    PLUGIN_STATE_NONE = 0,
    PLUGIN_STATE_REGISTERED,
    PLUGIN_STATE_LOADING_DEPS,
    PLUGIN_STATE_LOADED,
    PLUGIN_STATE_MISSING_DEPS,
    PLUGIN_STATE_DEPS_FAILED,
    PLUGIN_STATE_LOAD_FAILED,
    PLUGIN_STATE_INIT_FAILED,
};

//------------------------------------------------------------------------------
// HAL plugin info
//------------------------------------------------------------------------------
struct plugin_t {
    std::string name;
    std::string provider;
    std::string lib;
    std::string init_func;
    std::string exit_func;
    bool        auto_load;
    std::vector<std::string> deps;
    std::vector<std::string> features;

    plugin_state_t state;
    init_handler_t init_handler;
    exit_handler_t exit_handler;
};

static inline plugin_t* plugin_alloc()
{
    plugin_t* plugin =  (plugin_t *)HAL_CALLOC(hal::HAL_MEM_ALLOC_PLUGIN_MANAGER,
                                               sizeof(plugin_t));
    plugin->state = PLUGIN_STATE_NONE;

    return plugin;
}

std::ostream& operator<<(std::ostream& os, const plugin_t& val)
{
    os << "{ name=" << val.name;
    os << ", provider=" << val.provider;
    os << ", lib=" << val.lib;
    os << ", init_func=" << val.init_func;
    os << ", exit_func=" << val.exit_func;
    os << ", auto_load=" << val.auto_load;

    os << ", deps=[";
    for (const std::string &dep :  val.deps) {
        os << " " << dep;
    }
    os << " ]";

    os << ", features=[";
    for (const std::string &name :  val.features) {
        os << " " << name;
    }
    os << " ]";

    return os << "}";
}
//------------------------------------------------------------------------------
// FTE pipeline info
//------------------------------------------------------------------------------
struct pipeline_t {
    std::string name;
    bool wildcard_qid;
    fte::lifqid_t lifq;
    std::vector<std::string> outbound_features;
    std::vector<std::string> inbound_features;

    STAILQ_ENTRY(pipeline_t) entries;
};

std::ostream& operator<<(std::ostream& os, const pipeline_t& val)
{
    os << "{ name=" << val.name;
    os << ", wildcard_qid=" << val.wildcard_qid;
    os << ", lifq=" << val.lifq;
    os << ", outbound_features=[";
    for (const std::string &name :  val.outbound_features) {
        os << " " << name;
    }
    os << " ]";

    os << ", inbound_features=[";
    for (const std::string &name :  val.inbound_features) {
        os << " " << name;
    }
    os << " ]";
    
    return os << "}";                   
}

static pipeline_t* pipeline_alloc()
{
    return (pipeline_t*) HAL_CALLOC(hal::HAL_MEM_ALLOC_PLUGIN_MANAGER,
                                    sizeof(pipeline_t));
}


//------------------------------------------------------------------------------
// Parse plugin info
//------------------------------------------------------------------------------
bool plugin_manager_t::parse_plugin(const pt::ptree &tree, plugin_t *plugin,
                                    const std::string& plugin_path)
{
    if (auto name = tree.get_optional<std::string>("name")) {
        plugin->name = *name;
    } else {
        HAL_TRACE_ERR("plugins::parse_plugin missing name");
        return false;
    }

    if (auto provider = tree.get_optional<std::string>("provider")) {
        plugin->provider = *provider;
    } else {
        HAL_TRACE_ERR("plugins::parse_plugin missing provider for plugin {}", plugin->name);
        return false;
    }

    if (auto lib = tree.get_optional<std::string>("lib")) {
        if ((*lib)[0] == '/') {
            plugin->lib =  *lib;
        } else {
            plugin->lib = plugin_path + "/" + *lib;
        }
    } else {
        HAL_TRACE_ERR("plugins::parse_plugin missing lib for plugin {}", plugin->name);
        return false;
    }

    plugin->init_func = tree.get<std::string>("init_func", "");
    plugin->exit_func = tree.get<std::string>("exit_func", "");
    plugin->auto_load = tree.get<bool>("auto_load", false);

    // Parese deps
    auto deps = tree.get_child_optional("deps");
    if (deps) {
        for (auto &node : *deps) {
            plugin->deps.push_back(node.second.data());
        }
    }

    // Parse features
    auto features = tree.get_child_optional("features");
    if (features){
        for (auto &node : *features) {
            plugin->features.push_back(node.second.data());
        }
    }

    HAL_TRACE_DEBUG("plugins::parse_plugin plugin={}", *plugin);

    return true;
}

//------------------------------------------------------------------------------
// Parse plugins 
//------------------------------------------------------------------------------
void plugin_manager_t::parse_plugins(const pt::ptree &tree,
                                     const std::string& plugin_path)
{
    auto plugins = tree.get_child_optional("plugins");
    if (!plugins) {
        return;
    }

    for (auto &node : *plugins) {
        plugin_t *plugin = plugin_alloc();
        if (parse_plugin(node.second, plugin, plugin_path)) {
            plugins_[plugin_name(plugin->provider, plugin->name)] = plugin;
        }
    }
}


//------------------------------------------------------------------------------
// Parse pipeline info
//------------------------------------------------------------------------------
bool plugin_manager_t::parse_pipeline(const pt::ptree &tree, pipeline_t *pipeline)
{
    if (auto name = tree.get_optional<std::string>("name")) {
        pipeline->name = *name;
    } else {
        HAL_TRACE_ERR("plugins::parse_pipeline missing name");
        return false;
    }

    // parse selector
    if (auto lif = tree.get_optional<std::string>("selector.lif")) {
        pipeline->lifq.lif = hal::parse_service_lif(lif->c_str());
    } else {
        HAL_TRACE_ERR("plugins::parse_pipeline missing selector.lif for pipeline {}", pipeline->name);
        return false;
    }

    if (auto qid = tree.get_optional<std::string>("selector.qid")) {
        if (pipeline->lifq.lif == hal::SERVICE_LIF_CPU) {
            types::CpucbId id;
            if (types::CpucbId_Parse(*qid, &id) == false) {
                HAL_TRACE_ERR("plugins::parse_pipeline invalid qid {}", *qid);
                return false;
            }
            pipeline->lifq.qid = id;
        } else {
            pipeline->lifq.qid = atoi(qid->c_str());
        }
    } else {
        pipeline->wildcard_qid = true;
    }

    // parse outbound-features
    auto outbound_features = tree.get_child_optional("outbound_features");
    if (outbound_features) {
        for (auto &node : *outbound_features) {
            pipeline->outbound_features.push_back(node.second.data());
        }        
    }


    // parse inbound-features
    auto inbound_features = tree.get_child_optional("inbound_features");
    if (inbound_features) {
        for (auto &node : *inbound_features) {
            pipeline->inbound_features.push_back(node.second.data());
        }
    }

    HAL_TRACE_DEBUG("plugins::parse_pipeline pipeline={}", *pipeline);
    return true;
}


//------------------------------------------------------------------------------
// Parse pipelines
//------------------------------------------------------------------------------
void plugin_manager_t::parse_pipelines(const pt::ptree &tree)
{
    auto pipelines =  tree.get_child_optional("pipelines");
    if (!pipelines) {
        return;
    }

    for (auto &node : *pipelines) {
        pipeline_t *pipeline = pipeline_alloc();
        if (parse_pipeline(node.second, pipeline)) {
            STAILQ_INSERT_TAIL(&pipelines_, pipeline, entries);
        }
    }
}


//------------------------------------------------------------------------------
// Register plugin features
//------------------------------------------------------------------------------
void plugin_manager_t::register_features(plugin_t *plugin)
{
    for (std::string &name : plugin->features) {
        fte::add_feature(feature_name(plugin->provider,plugin->name, name));
    }
}

//------------------------------------------------------------------------------
// Register plugins
//------------------------------------------------------------------------------
void plugin_manager_t::register_plugin(plugin_t *plugin)
{
    HAL_TRACE_DEBUG("plugins::register_plugin plugin={}", plugin->name);
    if (plugin->state == PLUGIN_STATE_NONE) {
        register_features(plugin);
        plugin->state = PLUGIN_STATE_REGISTERED;
    }
}

//------------------------------------------------------------------------------
// Register pipeline
//------------------------------------------------------------------------------
void plugin_manager_t::register_pipeline(pipeline_t *pipeline)
{
    fte::lifqid_t mask = { 0x7FF, 0x7, 0xFFFFFF };
    mask.qid = pipeline->wildcard_qid ? 0: 0xFFFFFF;
    fte::register_pipeline(pipeline->name, pipeline->lifq,
                           pipeline->outbound_features,
                           pipeline->inbound_features,
                           mask);
}

//------------------------------------------------------------------------------
// Load a symbol from so
//------------------------------------------------------------------------------
bool plugin_manager_t::load_symbol(void *so, std::string name, void **symbol)
{
    if (name == "") {
        return true;
    }

    *symbol = dlsym(so, name.c_str());
    const char *dlsym_error = dlerror();
    if (dlsym_error) {
        HAL_TRACE_ERR("plugins: cannot load symbol {}: {}", name, dlsym_error);
        return false;
    }

    return true;
}

//------------------------------------------------------------------------------
// Load plugin symbols
//------------------------------------------------------------------------------
bool plugin_manager_t::load_symbols(void *so, plugin_t *plugin)
{

    if (!load_symbol(so, plugin->init_func, (void **)&plugin->init_handler)) {
        return false;
    }

    if (!load_symbol(so, plugin->exit_func, (void **)&plugin->exit_handler)) {
        return false;
    }

    return true;
}


//------------------------------------------------------------------------------
// Load plugin
//------------------------------------------------------------------------------
bool plugin_manager_t::load_plugin(plugin_t *plugin)
{
    hal_ret_t ret;

    // return if plugin is already loaded
    if (plugin->state == PLUGIN_STATE_LOADED) {
        return true;
    }

    if (plugin->state == PLUGIN_STATE_LOADING_DEPS) {
        HAL_TRACE_ERR("plugins::load_plugin {} - detected circular dependency",
                      plugin->name);
        return false;
    }

    // return if plugin already failed to load
    if (plugin->state != PLUGIN_STATE_REGISTERED) {
        return false;
    }

    HAL_TRACE_INFO("plugins::load_plugin {}", plugin->name);

    // Load all dependent plugins
    for (std::string dep : plugin->deps) {
        auto entry = plugins_.find(dep);
        if (entry == plugins_.end()) {
            HAL_TRACE_ERR("plugins::load_plugin {} missing dependent plugin {}",
                          plugin->name, dep);
            plugin->state = PLUGIN_STATE_MISSING_DEPS;
            return false;
        }

        if (load_plugin(entry->second) == false) {
            HAL_TRACE_ERR("plugins::load_plugin {} dependent plugin {} failed",
                          plugin->name, dep);
            plugin->state = PLUGIN_STATE_DEPS_FAILED;
            return false;
        }
    }

    // open so and load symbols
    HAL_TRACE_INFO("plugins::load_plugin {} loading so {}", plugin->name, plugin->lib);

    void *so = dlopen(plugin->lib.c_str(), RTLD_NOW|RTLD_GLOBAL|RTLD_DEEPBIND);
    if (!so) {
        HAL_TRACE_ERR("plugins: {} dlopen failed {}", plugin->lib, dlerror());
        plugin->state = PLUGIN_STATE_LOAD_FAILED;
        return false;
    }

    if (!load_symbols(so, plugin)) {
        HAL_TRACE_ERR("plugins::load_plugin {} load symbols failed", plugin->name);
        plugin->state = PLUGIN_STATE_LOAD_FAILED;
        dlclose(so);
        return false;
    }

    // init plugin
    if (plugin->init_handler) {
        HAL_TRACE_DEBUG("plugins::load_plugin {} initializing...", plugin->name);
        ret = plugin->init_handler();
        if (ret != HAL_RET_OK) {
            HAL_TRACE_ERR("plugins::load_plugin {} init failed ret={}",
                          plugin->name, ret);
            plugin->state = PLUGIN_STATE_INIT_FAILED;
            dlclose(so);
            return false;
        }
    }

    plugin->state = PLUGIN_STATE_LOADED;

    return true;
}


//------------------------------------------------------------------------------
//  parse plugin config
//------------------------------------------------------------------------------
void plugin_manager_t::parse(const std::string& json_path,
                             const std::string &plugin_path)
{
    pt::ptree root;

    pt::read_json(json_path, root);
    parse_plugins(root, plugin_path);
    parse_pipelines(root);
}

//------------------------------------------------------------------------------
//   load/init plugins and pipelines
//------------------------------------------------------------------------------
void plugin_manager_t::load()
{
    // register plugins
    for (auto &kv : plugins_) {
        register_plugin(kv.second);
    }

    // register pipelines
    pipeline_t *pipeline;
    STAILQ_FOREACH(pipeline, &pipelines_, entries) {
        register_pipeline(pipeline);
    }

    // Load all plugins with auto load
    for (auto &kv : plugins_) {
        plugin_t *plugin = kv.second;
        if (plugin->auto_load) {
            load_plugin(plugin);
        }
    }
}

} // namespace plugins
} // namespace hal

