#include <assert.h>
#include <boost/algorithm/string.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/property_tree/ptree.hpp>
#include <dlfcn.h>
#include <map>
#include <memory>
#include <unistd.h>

#include "lib/operd/operd.hpp"
#include "lib/operd/decoder.h"

#include "output.hpp"
#include "rotated_file.hpp"

namespace pt = boost::property_tree;

const int SLEEP_DURATION = 250000;
const int DECODER_BUFFER = 2048;

std::map<uint8_t, decoder_fn> g_decoders;

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

class decoded_log : public sdk::operd::log {
public:
    static std::shared_ptr<decoded_log> create(sdk::operd::log_ptr log);
    uint8_t severity(void) override;
    pid_t pid(void) override;
    const struct timeval *timestamp(void) override;
    uint8_t encoder(void) override;
    const char *data(void) override;
    size_t data_length(void) override;
private:
    sdk::operd::log_ptr orig_;
    char buffer_[DECODER_BUFFER];
    size_t decoded_length_;
};
typedef std::shared_ptr<decoded_log> decoded_log_ptr;

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
        fprintf(stderr, "Can't access %s\n", this->path_.c_str());
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

    if (e->encoder() != OPERD_DECODER_PLAIN_TEXT) {
        decoded_log_ptr de;
        de = decoded_log::create(e);
        if (de == nullptr) {
            fprintf(stderr, "Failed to decode %hhu\n", e->encoder());
            return true;
        }
        e = de;
    }
    for (auto out: this->outputs_) {
        out->handle(e);
    }
    return true;
}

decoded_log_ptr
decoded_log::create(sdk::operd::log_ptr log) {
    decoded_log_ptr dl;

    if (g_decoders.count(log->encoder()) == 0) {
        return nullptr;
    }

    dl = std::make_shared<decoded_log>();
    dl->orig_ = log;

    dl->decoded_length_ = g_decoders[log->encoder()](
        log->encoder(), log->data(), log->data_length(), dl->buffer_,
        DECODER_BUFFER);

    return dl;
}

uint8_t
decoded_log::severity(void) {
    return this->orig_->severity();
}

pid_t
decoded_log::pid(void) {
    return this->orig_->pid();
}

const struct timeval *
decoded_log::timestamp(void) {
    return this->orig_->timestamp();
}

uint8_t
decoded_log::encoder(void) {
    return this->orig_->encoder();
}

const char *
decoded_log::data(void) {
    return this->buffer_;
}

size_t
decoded_log::data_length(void) {
    return this->decoded_length_;
}

static output_ptr
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

static std::map<std::string, input_ptr>
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
register_decoder(uint8_t encoder_id, decoder_fn fn)
{
    if (g_decoders.count(encoder_id) != 0) {
        fprintf(stderr, "Error: duplicate encoder_id registration for %hhu\n",
                encoder_id);
        return -1;
    }

    g_decoders[encoder_id] = fn;
    return 0;
}

static std::map<uint8_t, decoder_fn>
load_decoders (std::string decoders_file)
{
    pt::ptree root;
    std::map<uint8_t, decoder_fn> decoders;

    if (access(decoders_file.c_str(), F_OK) != 0) {
        fprintf(stderr, "Warning: decoders file not found: %s\n",
                decoders_file.c_str());
        return decoders;
    }

    read_json(decoders_file, root);

    for (auto obj: root) {
        void *handle;
        decoder_lib_init_fn fn;
        const char *path;

        path = obj.second.data().c_str();

        fprintf(stderr, "Loading %s\n", path);
        if (access(path, F_OK) != 0) {
            fprintf(stderr, "Can't access %s\n", path);
            continue;
        }
        handle = dlopen(path, RTLD_LAZY);
        if (handle == NULL) {
            fprintf(stderr, "%s\n", dlerror());
            continue;
        }
        *(void **)&fn = dlsym(handle, "decoder_lib_init");
        if (fn == NULL) {
            fprintf(stderr, "%s\n", dlerror());
            dlclose(handle);
            continue;
        }
        fn(register_decoder);
    }

    return decoders;
}

int
main (int argc, const char *argv[])
{
    std::map<std::string, input_ptr> inputs;
    std::map<uint8_t, decoder_fn> decoders;
    std::string config;

    // Don't buffer stdout and stderr, so we read it in
    // sysmgr log files immediatelly
    setbuf(stdout, NULL);
    setbuf(stderr, NULL);
    
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <outputs.json> <decoders.json>\n", argv[0]);
        exit(-1);
    }
    config = argv[1];
    inputs = load_config(config);
    decoders = load_decoders(argv[2]);
    
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
