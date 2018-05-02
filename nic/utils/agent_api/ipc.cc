#include <sys/mman.h>
#include <sys/stat.h>        /* For mode constants */
#include <fcntl.h>           /* For O_* constants */
#include <unistd.h>
#include <sys/types.h>
#include <string.h>
#include "nic/utils/agent_api/ipc.hpp"
#include "nic/utils/agent_api/shared_constants.h"
#define FWLOG_SHM "/fwlog_ipc_shm"
#define ALIGN_64_MASK 0xfffffffffffffff8
#define ROUND_DOWN4(x) ((uint32_t)(x) & 0xfffffffc)

int ipc::shm_fd_;
void *ipc::mmap_addr_;
int ipc::mmap_size_;
int ipc::inst_count_;
int ipc::max_inst_;
// Sets up shared memory for IPC.
int ipc::setup_shm(int size, int numInst)
{
    int fd;
    int ret;
    void *p;

    if (numInst == 0) {
        return -1;
    }

    fd = shm_open(FWLOG_SHM, O_RDWR|O_CREAT, 0666);
    if (fd < 0) {
        return fd;
    }

    ret = ftruncate(fd, size);
    if (ret != 0) {
        close(fd);
        return ret;
    }

    p = mmap(0, size, PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0);
    if (p == MAP_FAILED) {
        close(fd);
        return -1;
    }

    shm_fd_ = fd;
    mmap_addr_ = p;
    mmap_size_ = size;
    inst_count_ = 0;
    max_inst_ = numInst;

    return 0;
}

// Creates an instance of ipc. Assumes single thread for
// now.
ipc *ipc::factory(void)
{
    uint8_t *addr;

    if (inst_count_ == max_inst_) {
        return NULL;
    }

    ipc *inst = new(ipc);

    inst->size_ = ROUND_DOWN4(mmap_size_/max_inst_);
    addr = (uint8_t *)mmap_addr_;
    inst->base_addr_ = &addr[inst_count_*inst->size_];
    inst->control_out_ = (uint32_t *)inst->base_addr_;
    inst->control_in_ = (uint32_t *)&inst->base_addr_[sizeof(uint32_t)];
    inst->put_count_ = (uint64_t *)&inst->base_addr_[2*sizeof(uint32_t)];
    inst->err_count_ = (uint64_t *)&inst->base_addr_[4*sizeof(uint32_t)];
    inst->write_index_ = (uint32_t *)&inst->base_addr_[6*sizeof(uint32_t)];
    inst->read_index_ = (uint32_t *)&inst->base_addr_[7*sizeof(uint32_t)];
    inst->num_buffers_ = (inst->size_ - IPC_OVH_SIZE)/IPC_BUF_SIZE;
    memset(inst->base_addr_, 0, IPC_OVH_SIZE);
    inst->base_addr_ = &inst->base_addr_[IPC_OVH_SIZE];

    inst_count_++;
    return inst;
}

void ipc::tear_down_shm(void)
{
    if (mmap_addr_ == NULL) {
        return;
    }

    // unmap, release shm.
    munmap(mmap_addr_, mmap_size_);
    close(shm_fd_);
    shm_unlink(FWLOG_SHM);
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

    if ((size + IPC_HDR_SIZE) > IPC_BUF_SIZE) {
        return NULL;
    }

    offset = *this->write_index_ * IPC_BUF_SIZE;

    offset += IPC_HDR_SIZE; // account for overhead
    return (uint8_t *)(this->base_addr_ + offset);
}

int ipc::put_buffer(uint8_t *buf, int size)
{
    uint32_t *hdrPtr = (uint32_t *)(buf - IPC_HDR_SIZE);

    if (hdrPtr != (uint32_t *)(*this->write_index_ * IPC_BUF_SIZE + this->base_addr_)) {
        *this->err_count_++;
        return -1;
    }

    if (size > (int)(IPC_BUF_SIZE - IPC_HDR_SIZE)) {
        *this->err_count_++;
        return -1;
    }

    *hdrPtr = size;
    inc_write_index();
    *this->put_count_++;
    return 0;
}

int ipc::get_avail_size(uint32_t wIndex)
{
    return (this->num_buffers_ - 1 + *this->read_index_ - wIndex) % this->num_buffers_;
}

void ipc::inc_write_index()
{
    uint32_t wIndex = *this->write_index_;

    wIndex = (wIndex + 1) % this->num_buffers_;
    *this->write_index_ = wIndex;
}
