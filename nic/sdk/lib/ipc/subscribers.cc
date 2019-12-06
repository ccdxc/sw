#include <assert.h>
#include <fcntl.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "subscribers.hpp"

const char *SUBS_SHM_FILE = "/ipc_subs";

using namespace sdk::ipc;

subscribers *subscribers::instance_ = new subscribers();

off_t
subs_size (void)
{
    return sizeof(uint8_t) * (IPC_MAX_CLIENT + 1) * (IPC_MAX_ID + 1);
}

subscribers *
subscribers::instance(void) {
    return subscribers::instance_;
}

subscribers::subscribers() {
    shmopen();
}

subscribers::~subscribers() {
}

void
subscribers::shmopen(void) {
    int fd;
    struct stat st;

    fd = shm_open(SUBS_SHM_FILE, O_RDWR | O_CREAT, 0600);
    assert(fd != -1);

    // Make sure no two binaries are trying to use the same shared
    // memory with different size of subs. This still leaves the door
    // open for problems where somebody modifies both MAX_ID and
    // MAX_CLIENT but their product is still the same
    fstat(fd, &st);
    assert(st.st_size == 0 || st.st_size == subs_size());

    ftruncate(fd, subs_size());
    
    this->subs_ = (uint8_t *)mmap(0, subs_size(), PROT_READ | PROT_WRITE,
                                  MAP_SHARED, fd, 0);
    assert(this->subs_ != NULL);
}

void
subscribers::set(uint32_t code, uint32_t client) {
    this->subs_[(code * IPC_MAX_CLIENT) + client] = 1;
}

void
subscribers::clear(uint32_t code, uint32_t client) {
    this->subs_[(code * IPC_MAX_CLIENT) + client] = 0;
}

std::vector<uint32_t>
subscribers::get(uint32_t code) {
    std::vector<uint32_t> subs;

    for (int i = 0; i <= IPC_MAX_CLIENT; i++) {
        if (this->subs_[(code * IPC_MAX_CLIENT) + i] == 1) {
            subs.push_back(i);
        }
    }

    return subs;
}
