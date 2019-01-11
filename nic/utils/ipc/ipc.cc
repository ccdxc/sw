#include <sys/mman.h>
#include <sys/stat.h>        /* For mode constants */
#include <fcntl.h>           /* For O_* constants */
#include <unistd.h>
#include <sys/types.h>
#include <stdio.h>
#include <string.h>
#include <inttypes.h>
#include <errno.h>
#include "nic/utils/ipc/ipc.hpp"
#include "nic/utils/ipc/constants.h"
#include "lib/logger/logger.hpp"

#define __STDC_FORMAT_MACROS
#define ALIGN_64_MASK 0xfffffffffffffff8
#define ROUND_DOWN4(x) ((uint32_t)(x) & 0xfffffffc)

// Shared memory implemented as a circular buffer. Messages will be discarded once the buffer is full.

// sets up shared memory of given size, name, buffer size, etc.
shm *shm::setup_shm(const char* name, int size, int num_inst, int buf_size, Logger logger)
{
    if (num_inst == 0) {
        logger->error("{}: number of ipc instances must be >0", name);
        return nullptr;
    }

    if (buf_size >= size) {
        logger->error("{}: buffer size should be less than the shared memory size", name);
        return nullptr;
    }

    logger->info("{}: setting up shared memory with size: {}", name, size);

    int fd = shm_open(name, O_RDWR | O_CREAT, 0666);
    if (fd < 0) {
        logger->error("{}: failed to create shared memory, errno: {}", name, errno);
        return nullptr;
    }

    int ret = ftruncate(fd, size);
    if (ret != 0) {
        close(fd);
        return nullptr;
    }

    void *mmap_addr = mmap(0, size, PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0);
    if (mmap_addr == MAP_FAILED) {
        logger->error("{}: mmap failed", name);
        close(fd);
        return nullptr;
    }

    shm *inst = new(shm);
    inst->fd_         = fd;
    inst->mmap_addr_  = mmap_addr;
    inst->mmap_size_  = size;
    inst->inst_count_ = 0;
    inst->max_inst_   = num_inst;
    inst->name_       = std::string(name);
    inst->buf_size_   = buf_size;
    inst->logger_     = logger;

    return inst;
}

// unmaps and unlinks the underlying shared memory
void shm::tear_down_shm(void)
{
    if (this->mmap_addr_ == NULL) {
        return;
    }

    // unmap, release shm.
    munmap(this->mmap_addr_, this->mmap_size_);
    close(this->fd_);
    shm_unlink(this->name_.c_str());
}

// initializes/creates an instance of IPC from the shared memory. Assumes single thread for now.
ipc *shm::factory(void)
{
    if (this->inst_count_ == this->max_inst_) {
        this->logger_->error("{}: 0 instances left on the shared memory", this->name_);
        return NULL;
    }

    uint8_t *addr      = (uint8_t *)this->mmap_addr_;
    uint32_t ipc_size  = ROUND_DOWN4(this->mmap_size_/this->max_inst_);

    ipc *inst =  new(ipc);
    inst->init(addr, this->inst_count_, ipc_size, this->buf_size_, this->logger_);
    this->inst_count_++;
    return inst;
}

// returns the shared memory name
const char *shm::get_name(void)
{
    return this->name_.c_str();
}

// initializes the ipc instance with defaults
void ipc::init(uint8_t *addr, int inst_count, uint32_t ipc_size, int buf_size, Logger logger)
{
    // initialize IPC attributes
    this->buf_size_     = buf_size;
    this->size_         = ipc_size;
    this->base_addr_    = &addr[inst_count * this->size_];
    this->control_out_  = (uint32_t *)this->base_addr_;
    this->control_in_   = (uint32_t *)&this->base_addr_[sizeof(uint32_t)];
    this->put_count_    = (uint64_t *)&this->base_addr_[2 * sizeof(uint32_t)];
    this->err_count_    = (uint64_t *)&this->base_addr_[4 * sizeof(uint32_t)];
    this->write_index_  = (uint32_t *)&this->base_addr_[6 * sizeof(uint32_t)];
    this->read_index_   = (uint32_t *)&this->base_addr_[7 * sizeof(uint32_t)];
    this->num_buffers_  = (this->size_ - IPC_OVH_SIZE)/this->buf_size_;
    memset(this->base_addr_, 0, IPC_OVH_SIZE);
    this->base_addr_   = &this->base_addr_[IPC_OVH_SIZE];
    this->logger_      = logger;
}

// Note: buffer could include wrap-around.
// Serializer needs to handle it.
uint8_t *ipc::get_buffer(int size)
{
    uint32_t offset;

    if (get_avail_size(*this->write_index_) == 0) {
        *this->err_count_ = *this->err_count_ + 1;
        return NULL;
    }

    if ((size + IPC_HDR_SIZE) > this->buf_size_) {
        return NULL;
    }

    offset = *this->write_index_ * this->buf_size_;

    offset += IPC_HDR_SIZE; // account for overhead
    return (uint8_t *)(this->base_addr_ + offset);
}

// writes the given size to buffer header
int ipc::put_buffer(uint8_t *buf, int size)
{
    uint32_t *hdrPtr = (uint32_t *)(buf - IPC_HDR_SIZE);

    if (hdrPtr != (uint32_t *)(*this->write_index_ * this->buf_size_ + this->base_addr_)) {
        *this->err_count_ = *this->err_count_ + 1;
        return -1;
    }

    if (size > (int)(this->buf_size_ - IPC_HDR_SIZE)) {
        *this->err_count_ = *this->err_count_ + 1;
        return -1;
    }

    *hdrPtr = size; // update the buffer header with size of the data; will be used by the reader;
    inc_write_index();
    *this->put_count_ = *this->put_count_ + 1;
    return 0;
}

// returns the available number of empty buffers
int ipc::get_avail_size(uint32_t wIndex)
{
    // mem. is assumed to be full when there is one empty slot left.
    return (this->num_buffers_ - 1 + *this->read_index_ - wIndex) % this->num_buffers_;
}

// increments the write index
void ipc::inc_write_index()
{
    uint32_t wIndex = *this->write_index_;

    wIndex = (wIndex + 1) % this->num_buffers_;
    *this->write_index_ = wIndex;
}

// returns the total number of writes on this IPC segment
uint64_t ipc::get_total_writes()
{
    return *this->put_count_;
}

// print IPC header details
void ipc::print_OVH_data()
{
    printf("++++++ OVH(meta) data ++++++ \n");
    printf("base_addr   : %p\n", this->base_addr_);
    printf("num_buffers : %d\n", this->num_buffers_-1);
    printf("put_count   : %p, %" PRIu64 "\n", this->put_count_, *this->put_count_);
    printf("err_count   : %p, %" PRIu64 "\n", this->err_count_, *this->err_count_);
    printf("read_index  : %p, %d\n", this->read_index_, *this->read_index_);
    printf("write_index : %p, %d\n", this->write_index_, *this->write_index_);
    printf("\n\n");
}
