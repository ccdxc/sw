#include <assert.h>
#include <boost/algorithm/string.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/property_tree/ptree.hpp>
#include <dlfcn.h>
#include <map>
#include <memory>
#include <unistd.h>

#include "lib/operd/operd.hpp"

#include "output.hpp"
#include "rotated_file.hpp"

namespace pt = boost::property_tree;

const int SLEEP_DURATION = 250000;


class library : public output {
public:
    static std::shared_ptr<library> factory(std::string path);
    library(std::string path);
    ~library();
    void handle(sdk::operd::log_ptr entry) override;
private:
    bool try_load_(void);
private:
    std::string path_;
    void *dlhandle_;
    void (*handler_)(sdk::operd::log_ptr entry);
};
typedef std::shared_ptr<library> library_ptr;

class input {
public:
    input(std::string name);
    void add_output(output_ptr);
    // returns true if there are more to read
    bool read(void);
private:
    sdk::operd::consumer_ptr consumer_;
    std::vector<output_ptr> outputs_;
    std::string name_;
};
typedef std::shared_ptr<input> input_ptr;

void
library::handle(sdk::operd::log_ptr entry) {
    this->handler_(entry);
}

library::library(std::string path) : path_(path) {
    this->dlhandle_ = NULL;
    this->handler_ = nullptr;
}

library::~library() {
    if (this->dlhandle_ != NULL) {
        dlclose(this->dlhandle_);
    }
}

library_ptr
library::factory(std::string path) {
    library_ptr lib = std::make_shared<library>(path);
    assert(lib != nullptr);
    
    if (!lib->try_load_()) {
        return nullptr;
    }

    return lib;
}

bool
library::try_load_(void) {
    assert(this->handler_ == nullptr);
    assert(this->dlhandle_ == NULL);

    if (access(this->path_.c_str(), F_OK) != 0) {
        fprintf(stderr, "Can't access %s", this->path_.c_str());
        return false;
    }
    
    this->dlhandle_ = dlopen(this->path_.c_str(), RTLD_LAZY);
    if (this->dlhandle_ == NULL) {
        fprintf(stderr, "%s\n", dlerror());
        return false;
    }

    dlerror();
    
    *(void **) (&this->handler_) = dlsym(this->dlhandle_, "handler");
    if (this->handler_ == NULL) {
        fprintf(stderr, "%s\n", dlerror());
        return false;
    }
    return true;
}

input::input(std::string name) : name_(name) {
    this->consumer_ = sdk::operd::consumer::create(name);
    this->consumer_->reset();
}

void
input::add_output(output_ptr output) {
    this->outputs_.push_back(output);
}

bool
input::read(void) {
    sdk::operd::log_ptr e;
    
    e = this->consumer_->read();

    if (e == nullptr) {
        return false;
    }
    for (auto out: this->outputs_) {
        out->handle(e);
    }
    return true;
}

output_ptr
parse_output (pt::ptree obj)
{
    std::string type = obj.get<std::string>("type");
    if (type == "") {
        fprintf(stderr, "output needs a type\n");
        return nullptr;
    } else if (type == "library") {
        std::string path = obj.get<std::string>("location");
        if (path == "") {
            fprintf(stderr, "output needs location property\n");
            return nullptr;
        }
        
        return library::factory(path);
    } else if (type == "file") {
        std::string path = obj.get<std::string>("location");
        if (path == "") {
            fprintf(stderr, "output needs location property\n");
            return nullptr;
        }
        
        int max_size = atoi(obj.get<std::string>("max-size", "0").c_str());
        if (max_size <= 0) {
            fprintf(stderr, "warning: max-size is %i\n", max_size);
            max_size = 0;
        }
        max_size = max_size * (1024 * 1024); // convert from MB to bytes

        int max_files = atoi(obj.get<std::string>("max-files", "0").c_str());
        if (max_files < 0) {
            fprintf(stderr, "warning: max-files is %i\n", max_size);
            max_files = 0;
        }

        int level = atoi(obj.get<std::string>("level", "0").c_str());
        if (level < 0 || level > UINT8_MAX) {
            fprintf(stderr, "warning: level is %i\n", level);
            level = UINT8_MAX;
        }

        return rotated_file::factory(path, (uint8_t)level, max_size,
                                     max_files);
    } else {
        fprintf(stderr, "output type not supported: %s\n", type.c_str());
        return nullptr;
    }
    return nullptr;
}

std::map<std::string, input_ptr>
load_config (std::string config_file)
{
    pt::ptree root;
    std::map<std::string, input_ptr> inputs;

    if (access(config_file.c_str(), F_OK) != 0) {
        fprintf(stderr, "Warning: config file not found: %s\n",
                config_file.c_str());
        return inputs; 
    }

    read_json(config_file, root);

    for (auto obj: root) {
        output_ptr output = parse_output(obj.second.get_child("output"));
        if (output == nullptr) {
            continue;
        }
        for (auto el: obj.second.get_child("input")) {
            std::string region = el.second.data();
            if (inputs.count(region) == 0) {
                inputs[region] = std::make_shared<input>(region);
            }
            inputs[region]->add_output(output);
        }
    }

    return inputs;
}

int
main (int argc, const char *argv[])
{
    std::map<std::string, input_ptr> inputs;
    std::string config;

    if (argc != 2) {
        fprintf(stderr, "Usage: %s <config.json>", argv[0]);
        exit(-1);
    }
    config = argv[1];
    inputs = load_config(config);

    while (true) {
        bool more = false;
        for (auto pair: inputs) {
            more = pair.second->read() || more;
        }
        if (more == false) {
            usleep(SLEEP_DURATION);
        }
    }

    return 0;
}
