#include <assert.h>
#include <boost/algorithm/string.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/property_tree/ptree.hpp>
#include <fcntl.h>
#include <memory>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "region.hpp"

namespace pt = boost::property_tree;

namespace sdk {
namespace operd {

const std::string CONFIG_LOCATION = "/nic/conf/operd-regions.json";
const std::string CONFIG_OVERRIDE = "OPERD_REGIONS";
const int OPEN_TRIES = 10;

region::region(std::string name) {
    this->name_ = name;
    this->load_config_();
    this->create_shm_();
    this->open_shm_();
}

void
region::load_config_(void) {
    pt::ptree root;

    std::string config_file = CONFIG_LOCATION;
    if (getenv(CONFIG_OVERRIDE.c_str())) {
        config_file = getenv(CONFIG_OVERRIDE.c_str());
    }

    if (access(config_file.c_str(), F_OK) != 0) {
        fprintf(stderr, "operd config file not found: %s\n",
                config_file.c_str());
        this->chunk_count_ = 0;
        this->severity_ = 0;
        return; 
    }

    read_json(config_file, root);

    if (root.count(this->name_) == 0) {
        this->chunk_count_ = 0;
        this->severity_ = 0;
        return;
    }
    
    auto obj = root.get_child(this->name_);

    this->chunk_count_ = std::stoul(obj.get<std::string>("chunk-count", "0"),
                                    nullptr, 10);
    this->severity_ = std::stoi(obj.get<std::string>("severity", "0"),
                                nullptr, 10);
}

void
region::create_shm_(void) {
    int fd;
    int rc;
    region_t *region;

    if (this->chunk_count_ == 0) {
        return;
    };

    fd = shm_open(this->name_.c_str(), O_CREAT | O_RDWR | O_EXCL | O_TRUNC,
                  0666);
    if (fd == -1) {
        return;
    }

    rc = ftruncate(fd, sizeof_region_t(this->chunk_count_));
    assert(rc == 0);

    region = (region_t *)mmap(0, sizeof_region_t(this->chunk_count_),
                               PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    assert(region != NULL);

    region->chunk_count = this->chunk_count_;
    region->severity = this->severity_;

    close(fd);
}

void
region::open_shm_(void) {
    int rc;
    struct stat st;

    if (this->chunk_count_ == 0) {
        return;
    };
    
    this->create_shm_();

    // we try to open multiple times to avoid race conditions with create
    for (int i = 0; i < OPEN_TRIES; i++) {
        this->fd_ = shm_open(this->name_.c_str(), O_RDWR, 0666);
        if (this->fd_ != -1) {
            break;
        }
        usleep(100);
    }
    assert(this->fd_ != -1);

    rc = fstat(this->fd_, &st);
    assert(rc == 0);

    assert(st.st_size == sizeof_region_t(this->chunk_count_));
       
    this->region_ = (region_t *)mmap(0, st.st_size, PROT_READ | PROT_WRITE,
                                     MAP_SHARED, this->fd_, 0);

    assert(this->region_->chunk_count == this->chunk_count_);

    this->read_next_ = this->region_->serial;
}

serial_t
region::reserve_(int count) {
    serial_t starting_serial;
    
    assert(count > 0 && count < CHUNK_COUNT_MAX);

    starting_serial = __atomic_fetch_add(&this->region_->serial, count,
                                         __ATOMIC_SEQ_CST);

    for (int i = 0; i < count; i++) {
        chunk_t *chunk;

        chunk = this->get_chunk_(starting_serial + i);

        chunk->serial = starting_serial + i;
        chunk->part_of = starting_serial;
    }

    return starting_serial;   
}

serial_t
region::get_chunk_index_(serial_t serial) {
    return serial % this->region_->chunk_count;
}

chunk_t *
region::get_chunk_(serial_t serial) {
    return &this->region_->chunks[get_chunk_index_(serial)];
}

void
region::write(uint8_t encoder, uint8_t severity, const void *data,
              size_t data_length) {
    int chunk_count;
    serial_t starting_serial;
    chunk_t *starting_chunk;
    
    if (this->chunk_count_ == 0 || this->region_->severity < severity) {
        return;
    };

    chunk_count = (data_length / CHUNK_SIZE) + 1;

    starting_serial = this->reserve_(chunk_count);

    starting_chunk = this->get_chunk_(starting_serial);
    starting_chunk->encoder = encoder;
    starting_chunk->severity = severity;
    starting_chunk->pid = getpid();
    gettimeofday(&starting_chunk->timestamp, NULL);

    for (int i = 0; i < chunk_count; i++) {
        int copy_size;
        char *copy_start; // the start point in the data buffer to
                          // start copying from
        chunk_t *chunk;

        chunk = this->get_chunk_(starting_serial + i);

        copy_start = ((char *)data) + (i * CHUNK_SIZE);
        
        if ((i + 1) == chunk_count)
            // last chunk, copy whatever left
            copy_size = data_length - ((chunk_count - 1) * CHUNK_SIZE); 
        else 
            copy_size = CHUNK_SIZE;

        memcpy(chunk->data, copy_start, copy_size);
        chunk->size = data_length - (i * CHUNK_SIZE);
    }
}

std::shared_ptr<log>
region::read(void) {
    entry_ptr e = nullptr;

    if (this->chunk_count_ == 0) {
        return nullptr;
    }
    
    while (this->read_next_ < this->region_->serial) {
        e = this->read_(this->read_next_);
        if (e != nullptr) {
            this->read_next_ += (e->data_length_ / CHUNK_SIZE) + 1;
            return e;
        }
        this->read_next_ += 1;
    }

    return nullptr;
}

entry_ptr
region::read_(serial_t serial)
{
    chunk_t *chunk;
    int chunk_index;
    entry_ptr e = nullptr;

    if (this->chunk_count_ == 0) {
        return nullptr;
    };

    chunk_index = 0;
    do {
        int copy_amount = CHUNK_SIZE;
        if (serial + chunk_index >= this->region_->serial) {
            // todo
            // Serial not created yet
            return nullptr;
        }
        chunk = this->get_chunk_(serial + chunk_index);
        if (chunk_index == 0) {
            e = std::make_shared<entry>(chunk->size);
            e->pid_ = chunk->pid;
            e->severity_ = chunk->severity;
            e->encoder_ = chunk->encoder;
            memcpy(&e->timestamp_, &chunk->timestamp, sizeof(e->timestamp_));
        }

        if (chunk->size < CHUNK_SIZE)            
            copy_amount = chunk->size; 
        memcpy(e->data_ + (chunk_index * CHUNK_SIZE), chunk->data, copy_amount);

        if (chunk->part_of != serial) {
            if (chunk_index == 0) {
                // todo
                // It's not a starting serial
            } else {
                // todo
                // Roll over while reading record
            }
            return nullptr;
        }

        chunk_index += 1;
    } while (chunk->size > CHUNK_SIZE);

    return e;
}

void
region::reset(void) {
    if (this->region_ == NULL) {
        return;
    }
    if (this->region_->serial > this->region_->chunk_count) {
        this->read_next_ = this->region_->serial - this->region_->chunk_count;
    } else {
        this->read_next_ = 0;
    }
}

void
region::set_severity(uint8_t new_severity) {
    if (this->region_ == NULL) {
        return;
    }
    this->region_->severity = new_severity;
}

} // namespace operd
} // namespace sdk
