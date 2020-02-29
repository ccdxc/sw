// {C} Copyright 2020 Pensando Systems Inc. All rights reserved.
// This file was forked from delphi

#include <fcntl.h>
#include <iostream>
#include <map>
#include <memory>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string>
#include <sys/mman.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "shm.hpp"

namespace sdk {
namespace metrics {

const int MAX_OPEN_TRIES = 5;
const int OPEN_FAIL_SLEEP_USEC = 250000; // 250 ms

// a global variable that keeps track of all memory mapped shared memory
std::map<std::string, void *> g_mapped_region;

// Shm constructor
Shm::Shm() {
    my_id_ = ::getpid();
    mapped_ = false;
}

error Shm::createMemMap(std::string name, int32_t size) {
    int shm_fd;
    void *ptr;
    
    shm_fd = shm_open(name.c_str(), O_EXCL | O_CREAT | O_RDWR, 0666);
    if (shm_fd < 0) {
        return error::OK();;
    }

    // configure the shared memory size
    int err = ftruncate(shm_fd, size);
    if (err < 0) {
        //LogError("Error setting shared memory file. Err {}/{}", err, errno);
        close(shm_fd);
        return error::New("Error setting shared memory size");
    }

    // memory map the shared memory
    ptr = mmap(0, size, PROT_WRITE, MAP_SHARED, shm_fd, 0);
    if (ptr == NULL) {
        //LogError("Error memory mapping the file {}. Err {}", name, errno);
        close(shm_fd);
        return error::New("Error memory mapping shared memory");
    }

    // find the shm meta
    shm_meta_ = (ShmMeta_t *)ptr;

    // save the mapped region
    g_mapped_region[name] = ptr;

    // check if the shared memory is already initialized
    if ((shm_meta_->hdr.magic != kShmMagic) ||
        (shm_meta_->hdr.version != kShmVersion)) {
        error err = this->initSharedMemory(size);
        if (err.IsNotOK()) {
            //LogError("Error initializing shared memory. Err {}", err);
            close(shm_fd);
            return err;
        }
    }

    // verify size of shared memory is same as we're expecting
    if (shm_meta_->hdr.size != size) {
        //LogError("Shared memory size {} does not match expected size {}",
        // shm_meta_->hdr.size != size);
        close(shm_fd);
        return error::New("Shared memory size does not match");
    }

    close(shm_fd);
    return error::OK();
}

// MemMap memory maps the shared memory
error Shm::MemMap(std::string name, int32_t size) {
    std::map<std::string, void *>::iterator it;
    
    // find the mapped region by name
    it = g_mapped_region.find(name);
    if (it != g_mapped_region.end()) {
        shm_meta_ = (ShmMeta_t *)it->second;
    } else {
        int      shm_fd;
        void*    ptr;
        int      tries = 0;

        if(access(("/dev/shm/" + name).c_str(), F_OK) == -1 ) {
            this->createMemMap(name, size);
        }

        // open the shared memory file
        // First to memmap creates the file and takes a lock. The rest
        // spin until init is done.
        do {
            shm_fd = shm_open(name.c_str(), O_RDWR, 0666);
            if (shm_fd != -1) {
                break;
            }
            tries += 1;
            usleep(OPEN_FAIL_SLEEP_USEC);
        } while (tries < MAX_OPEN_TRIES);
        if (shm_fd < 0) {
            //LogError("Error opening shared memory err: {}/{}", shm_fd, errno);
            return error::New("Error opening shared memory");
        }
            

        // configure the shared memory size
        int err = ftruncate(shm_fd, size);
        if (err < 0) {
            //LogError("Error setting shared memory file. Err {}/{}", err, errno);
            return error::New("Error setting shared memory size");
        }

        // memory map the shared memory
        ptr = mmap(0, size, PROT_WRITE, MAP_SHARED, shm_fd, 0);
        if (ptr == NULL) {
            //LogError("Error memory mapping the file {}. Err {}", name, errno);
            return error::New("Error memory mapping shared memory");
        }

        // find the shm meta
        shm_meta_ = (ShmMeta_t *)ptr;

        // save the mapped region
        g_mapped_region[name] = ptr;
    }

    // verify the shared memory contains the magic string
    if ((shm_meta_->hdr.magic != kShmMagic) || (shm_meta_->hdr.version != kShmVersion)) {
        //LogError("Delphi magic string not found in shared memory(found {}/{})",
        // shm_meta_->hdr.magic, shm_meta_->hdr.version);
        return error::New("Shared memory magic string not found");
    }

    // verify size of shared memory is same as we're expecting
    if (shm_meta_->hdr.size != size) {
        //LogError("Shared memory size {} does not match expected size {}",
        // shm_meta_->hdr.size != size);
        return error::New("Shared memory size does not match");
    }

    // set the mapped flag and store shared memory name
    mapped_ = true;
    shm_name_ = name;

    // instantiate the slab allocator
    slab_mgr_ = std::unique_ptr<SlabAllocator>(
        new SlabAllocator(&(shm_meta_->mempool), (uint8_t *)shm_meta_, my_id_));
    assert(slab_mgr_ != NULL);

    // initialize kvstore manager
    kvstore_ = std::make_shared<KvstoreMgr>(&shm_meta_->kvstore_root,
                                            shared_from_this());
    assert(kvstore_ != NULL);

    return error::OK();
}

// initSharedMemory initializes a newly created shared memory
error Shm::initSharedMemory(int32_t size) {
    // create and init the slab mgr
    slab_mgr_ = std::unique_ptr<SlabAllocator>(new SlabAllocator(&(shm_meta_->mempool), (uint8_t *)shm_meta_, my_id_));
    assert(slab_mgr_ != NULL);
    error err = slab_mgr_->InitMem(size);
    if (err.IsNotOK()) {
        //LogError("Error initializing slab allocator. Err: {}", err);
        return err;
    }

    // set the mapped flag
    mapped_ = true;

    // initialize the kvstore
    err = KvstoreMgr::Init(&shm_meta_->kvstore_root, shared_from_this());
    if (err.IsNotOK()) {
        //LogError("Error initializing kvstore. Err: {}", err);
        return err;
    }

    // init the metadata header
    shm_meta_->hdr.size    = size;
    shm_meta_->hdr.version = kShmVersion;
    shm_meta_->hdr.magic   = kShmMagic;

    return error::OK();
}

// MemUnmap unmaps the delphi library
error Shm::MemUnmap() {
    // if memory is not mapped, we are done
    if (!mapped_) {
        return 0;
    }

    int err = shm_unlink(shm_name_.c_str());
    if ((err < 0) && (errno != ENOENT)) {
        //LogError("Error unlinking shared memory {}. Err {}/{}", shm_name_, err, errno);
        return error::New("Error unmapping shared memory");
    }
    mapped_ = false;

    return 0;
}

// Alloc allocates some memory
void * Shm::Alloc(int len) {
    if (!mapped_) {
        //LogError("Shared memory is not mapped");
        return NULL;
    }

    return (void *)slab_mgr_->Alloc(len);
}

// Free frees memory
error  Shm::Free(void *ptr) {
    if (!mapped_) {
        return error::New("Shared memory is not mapped");
    }

    // check the pointer is in shared memory range
    uint8_t *uptr = (uint8_t *)ptr;
    if ((uptr < (uint8_t *)shm_meta_) || (uptr > (((uint8_t *)shm_meta_) + shm_meta_->hdr.size))) {
        return error::New("Pointer is not in shared memory range");
    }

    return slab_mgr_->Free(uptr);
}

// DumpMeta dumps shm meta into shared memory
void Shm::DumpMeta() {
    printf("Shared memory mapnped at: %p\n", (void *)shm_meta_);
    printf("Magic: 0x%lx, Version: 0x%x, Size: %d\n", shm_meta_->hdr.magic, shm_meta_->hdr.version, shm_meta_->hdr.size);
    slab_mgr_->Print();
    printf("Kvstore root: 0x%x\n", shm_meta_->kvstore_root);
    kvstore_->DumpInfo();
}

}  // namespace shm
} // namespace delphi
