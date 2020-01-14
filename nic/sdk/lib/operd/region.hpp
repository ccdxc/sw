#ifndef __OPERD_REGION_H__
#define __OPERD_REGION_H__

//
// A region is a shared memory circular buffer where multiple
// processes write to and multiple processes read from.
//
// It is optmized? for writes, so we don't make take any locks(does
// __atomic_fetch_add take lock?) or make any checks at the time of
// writing.
//
// This means we can have two processes or threads writing at the same
// location at the same time. And it is ok, as long as we can detect
// it at reading time.
//

#include <assert.h>
#include <map>
#include <memory>
#include <stdint.h>
#include <string>
#include <sys/time.h>

#include "operd.hpp"
#include "entry.hpp"

namespace sdk {
namespace operd {

using serial_t = uint64_t;

const int CHUNK_SIZE = 128;
const int CHUNK_COUNT_MAX = 16384;

struct chunk_ {
    serial_t serial;
    serial_t part_of;
    // if there are multiple chunks in an entry, size contains
    // the size starting with this fragment until the end
    size_t size;
    uint8_t encoder;
    pid_t pid;
    uint8_t severity;
    struct timeval timestamp;
    char data[CHUNK_SIZE];
};
typedef struct chunk_ chunk_t;

struct region_ {
    uint8_t severity;
    uint32_t chunk_count;
    serial_t serial;
    chunk_t chunks[];
};
typedef struct region_ region_t;

static inline ssize_t
sizeof_region_t (int chunk_count)
{
    assert(chunk_count > 0 && chunk_count <= CHUNK_COUNT_MAX);
    return sizeof(region_t) + sizeof(chunk_t) * chunk_count;
}

class region : public producer,
               public consumer {
public:
    region(std::string name);
    void write(uint8_t encoder, uint8_t severity, const char *data,
               size_t data_length) override;
    // Read is NOT thread safe
    std::shared_ptr<log> read(void) override;
    // reset reader
    void reset(void) override;
    void set_severity(uint8_t new_severity);
private:
    void load_config_(void);
    void create_shm_(void);
    void open_shm_(void);
    serial_t reserve_(int count);
    serial_t get_chunk_index_(serial_t serial);
    chunk_t *get_chunk_(serial_t serial);
    entry_ptr read_(serial_t serial);
private:
    int fd_;
    std::string name_;
    uint32_t chunk_count_;
    uint8_t severity_;
    region_t *region_;
    serial_t read_next_;
};
typedef std::shared_ptr<region> region_ptr;

} // namespace operd
} // namespace sdk

#endif // __OPERD_REGION_H__
