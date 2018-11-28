#ifndef __EVENTS_QUEUE_HPP__
#define __EVENTS_QUEUE_HPP__

#include "nic/utils/ipc/ipc.hpp"
#include <stdio.h>

class events_queue {
public:
    // initializes events queue
    static events_queue* init(const char*, int, int);

    // tear down events queue
    void deinit(void);

    // allocate a buffer of given size in the underlying shared memory
    uint8_t* get_buffer (int size);

    // write given size to buffer header
    int write_msg_size (uint8_t* buf, int size);

    // return the total number of writes performed on the underlying shared memory
    uint64_t get_total_writes();

    // return the name of the underlying shared memory
    const char* get_name();

    void print_queue_stats();
private:
    shm* shm_;  // shared memory
    ipc* ipc_;  // instance of shared memory; events queue will have only one instance

    events_queue (void) {};
};

#endif