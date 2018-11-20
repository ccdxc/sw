#include "nic/utils/ipc/ipc.hpp"
#include "nic/utils/events/queue/queue.hpp"
#include <pthread.h>
#include <stdio.h>

#define SHM_INSTANCES 1 // events queue will always have one instance

// events queue implemented on top of nic/utils/ipc/ipc.hpp

// events_recorder sets up shared memory for use by the events recorder
events_queue* events_queue::init(const char* name, int size, int buff_size)
{
    // create shared memory
    shm *shm_ = shm::setup_shm(name, size, SHM_INSTANCES, buff_size);
    if (!shm_) {
        return nullptr;
    }

    // create an IPC instance
    ipc *ipc_ = shm_->factory();
    if (!ipc_) {
        return nullptr;
    }

    events_queue *evtsQ_ = new(events_queue);
    evtsQ_->shm_ = shm_;
    evtsQ_->ipc_ = ipc_;
    return evtsQ_;
}

// tear down the underlying shared memory
void events_queue::deinit (void)
{
    this->shm_->tear_down_shm();
    this->shm_ = nullptr;
    this->ipc_ = nullptr;
}

// get buffer of given size or SHM_BUF_SIZE
uint8_t* events_queue::get_buffer(int size)
{
    return this->ipc_->get_buffer(size);
}

// write given message size to buffer header
int events_queue::write_msg_size(uint8_t *buf, int size)
{
    return this->ipc_->put_buffer(buf, size);
}

// return total number of writes performed on the queue
uint64_t events_queue::get_total_writes()
{
    return this->ipc_->get_total_writes();
}

// print queue stats (reader, writer indices, num buffers, etc.)
void events_queue::print_queue_stats()
{
    if (this->ipc_) {
        this->ipc_->print_OVH_data();
    }
}