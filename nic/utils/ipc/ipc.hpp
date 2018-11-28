#ifndef __IPC__HPP__
#define __IPC__HPP__
#include <stdint.h>
#include <pthread.h>

// Shared memory can be split into multiple instances/segment.
// IPC represents single segment within shared memory.
class ipc {
public:
    ipc(void){};

    // initialize a shared memory with defaults
    void init (uint8_t*, int, uint32_t, int);

    // allocate the buffer of given size from IPC instance
    uint8_t *get_buffer(int); // size

    // write the given size to buffer header
    int put_buffer(uint8_t*, int); // buf, size

    // return the total number of writes on this IPC instance
    uint64_t get_total_writes();

    // print IPC header details
    void print_OVH_data();
private:
    uint8_t *base_addr_;                // base address of this IPC segment

    // metadata stored in IPC header
    uint32_t *control_out_;             // control out
    uint32_t *control_in_;              // control in
    volatile uint64_t *put_count_;      // put/write count
    volatile uint64_t *err_count_;      // error count
    volatile uint32_t *write_index_;    // write index
    volatile uint32_t *read_index_;     // read index

    uint32_t size_;                     // size of IPC segment
    int num_buffers_;                   // total number of buffer available in this IPC segment
    int buf_size_;                      // size of each buffer

    // return number of empty buffers available on this IPC segment
    int get_avail_size(uint32_t);

    // increment write index
    void inc_write_index();
};

class shm {
public:
    // create and mmap shared memory with the given size
    static shm *setup_shm(const char*, int, int, int);

    // tear down the shared memory
    void tear_down_shm(void);

    // initialize/create an IPC instance
    ipc *factory(void);

    // return the shared memory name
    const char *get_name();
private:
    int fd_;            // file descriptor of shared mem.
    void *mmap_addr_;   // memory mapped address
    int mmap_size_;     // size of shared memory
    int buf_size_;      // size of each buffer allocated from this shared mem.
    int inst_count_;    // total IPC instances allocated on this shared mem.
    int max_inst_;      // total number of allowed/possible IPC instances on this shared mem.
    const char* name_;  // name of the shared mem.
};
#endif