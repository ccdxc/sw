#include <assert.h>
#include <iostream>
#include <boost/interprocess/managed_shared_memory.hpp>
#include "include/sdk/base.hpp"
#include "include/sdk/mem.hpp"
#include "lib/shmmgr/shmmgr.hpp"

using namespace boost::interprocess;
#define TO_FM_SHM(x) ((fixed_managed_shared_memory *)(x))
#define TO_SHM(x)    ((managed_shared_memory *)(x))

namespace sdk {
namespace lib {

// base structure for named segment allocation
class shm_segment {
public:
    // offset of the allocated memory from the named segment
    uint64_t offset;
    // size of the allocated memory
    std::size_t size;
};



//------------------------------------------------------------------------------
// initialization routine
//------------------------------------------------------------------------------
bool
shmmgr::init(const char *name, const std::size_t size,
             shm_mode_e mode, void *baseaddr)
{
    if (baseaddr) {
        fixed_managed_shared_memory    *fixed_mgr_shm;

        if (mode == SHM_CREATE_ONLY) {
            fixed_mgr_shm = new fixed_managed_shared_memory(create_only, name,
                                                            size, baseaddr);
        } else if (mode == SHM_OPEN_ONLY) {
            fixed_mgr_shm = new fixed_managed_shared_memory(open_only, name,
                                                            baseaddr);
        } else if (mode == SHM_OPEN_OR_CREATE) {
            fixed_mgr_shm = new fixed_managed_shared_memory(open_or_create, name,
                                                            size, baseaddr);
        } else if (mode == SHM_OPEN_READ_ONLY) {
            fixed_mgr_shm = new fixed_managed_shared_memory(open_read_only,
                                                            name, baseaddr);
        } else {
            return false;
        }
        mmgr_ = fixed_mgr_shm;
        fixed_ = true;
    } else {
        managed_shared_memory    *mgr_shm;

        if (mode == SHM_CREATE_ONLY) {
            mgr_shm = new managed_shared_memory(create_only, name, size);
        } else if (mode == SHM_OPEN_ONLY) {
            mgr_shm = new managed_shared_memory(open_only, name);
        } else if (mode == SHM_OPEN_OR_CREATE) {
            mgr_shm = new managed_shared_memory(open_or_create, name, size);
        } else if (mode == SHM_OPEN_READ_ONLY) {
            mgr_shm = new managed_shared_memory(open_read_only, name);
        } else {
            return false;
        }
        mmgr_ = mgr_shm;
        fixed_ = false;
    }
    strncpy(name_, name, SHMSEG_NAME_MAX_LEN);
    return true;
}

//------------------------------------------------------------------------------
// constructor
//------------------------------------------------------------------------------
shmmgr::shmmgr()
{
    mmgr_ = NULL;
}

//------------------------------------------------------------------------------
// destructor
//------------------------------------------------------------------------------
shmmgr::~shmmgr()
{
}

//------------------------------------------------------------------------------
// factory method
//------------------------------------------------------------------------------
shmmgr *
shmmgr::factory(const char *name, const std::size_t size,
                shm_mode_e mode, void *baseaddr)
{
    void      *mem;
    shmmgr    *new_shmmgr;

    // basic validation(s)
    if ((name == NULL) || size <= 16) {
        return NULL;
    }

#if 0
    // remove shared memory segment on construction/destruction
    struct shm_remove {
        char *name_;
        shm_remove(char *name) { name_ = name; }
        ~shm_remove() { shared_memory_object::remove(name_); }
    } remover(name);
#endif

    mem = SDK_CALLOC(SDK_MEM_ALLOC_LIB_SHM, sizeof(shmmgr));
    if (mem == NULL) {
        return NULL;
    }

    new_shmmgr = new (mem) shmmgr();
    if (new_shmmgr->init(name, size, mode, baseaddr) == false) {
        new_shmmgr->~shmmgr();
        SDK_FREE(SDK_MEM_ALLOC_LIB_SHM, new_shmmgr);
        return NULL;
    }
    return new_shmmgr;
}

//------------------------------------------------------------------------------
// remove shm segment given its name
//------------------------------------------------------------------------------
void
shmmgr::remove(const char *name)
{
    if (!name) {
        return;
    }
    SDK_TRACE_DEBUG("Deleting segment %s", name);
    shared_memory_object::remove(name);
}

//------------------------------------------------------------------------------
// destroy method
//------------------------------------------------------------------------------
void
shmmgr::destroy(shmmgr *mmgr)
{
    if (!mmgr) {
        return;
    }
    shmmgr::remove(mmgr->name_);
    mmgr->~shmmgr();
    SDK_FREE(SDK_MEM_ALLOC_LIB_SHM, mmgr);
}

//------------------------------------------------------------------------------
// check if shared memory segment exists
//------------------------------------------------------------------------------
bool
shmmgr::exists(const char *name, void *baseaddr)
{
    if (name == NULL) {
        return false;
    }
    try {
        if (baseaddr) {
            fixed_managed_shared_memory seg(open_only, name, baseaddr);
        } else {
            managed_shared_memory seg(open_only, name);
        }
        //return segment.check_sanity();
        return true;
    } catch (const boost::interprocess::interprocess_exception &ex) {
        SDK_TRACE_DEBUG("shmmgr bip exception : ",  ex.what());
    } catch (const std::exception &ex) {
        SDK_TRACE_DEBUG("shmmgr general exception : ",  ex.what());
    }
    return false;
}

//------------------------------------------------------------------------------
// allocate requested amount of memory
//------------------------------------------------------------------------------
void *
shmmgr::alloc(const std::size_t size, const std::size_t alignment,
              bool reset)
{
    void    *ptr = NULL;

    if (alignment == 0) {
        ptr = fixed_ ? TO_FM_SHM(mmgr_)->allocate(size) : TO_SHM(mmgr_)->allocate(size);
    } else {
        assert((alignment & (alignment - 1)) == 0);
        assert(alignment >= 4);
        ptr = fixed_ ? TO_FM_SHM(mmgr_)->allocate_aligned(size, alignment) :
            TO_SHM(mmgr_)->allocate_aligned(size, alignment);
    }
    if (ptr) {
        memset(ptr, 0, size);
    }
    return ptr;
}

//------------------------------------------------------------------------------
// free given memory
//------------------------------------------------------------------------------
void
shmmgr::free(void *mem)
{
    if (mem == NULL) {
        return;
    }
    fixed_ ? TO_FM_SHM(mmgr_)->deallocate(mem) : TO_SHM(mmgr_)->deallocate(mem);
}

//------------------------------------------------------------------------------
// return the size of the shared memory segment
//------------------------------------------------------------------------------
std::size_t
shmmgr::size(void) const
{
    return fixed_ ? TO_FM_SHM(mmgr_)->get_size() : TO_SHM(mmgr_)->get_size();
}

//------------------------------------------------------------------------------
// return the size of the free memory available
//------------------------------------------------------------------------------
std::size_t
shmmgr::free_size(void) const
{
    return fixed_ ? TO_FM_SHM(mmgr_)->get_free_memory() : TO_SHM(mmgr_)->get_free_memory();
}

void *
shmmgr::mmgr(void) const
{
    return mmgr_;
}

void *
shmmgr::segment_alloc(const char *name, std::size_t size, bool create) {
    std::pair<shm_segment*, std::size_t> res;
    shm_segment* state;
    void *addr = NULL;
    managed_shared_memory *shm =  (managed_shared_memory *)mmgr_;

    res = shm->find<shm_segment>(name);
    state = res.first;
    if (create) {
        if (state != NULL) {
            addr = (void *)((uint64_t)state - state->offset);
            shm->deallocate(addr);
            shm->destroy_ptr(state);
        }
        state = shm->construct<shm_segment>(name)();
        if (state) {
            addr = shm->allocate(size);
            if (addr) {
                state->offset = (uint64_t)state - (uint64_t)addr;
                state->size = size;
                return addr;
            } else {
                SDK_TRACE_ERR("Memory alloc failed");
                shm->destroy_ptr(state);
            }
        }
    } else {
        // in upgrade init this should be created.  mmgr will be enabled only in
        // upgrade scenarios. so no need of explicit upgrade check
        SDK_ASSERT(state != NULL);
        addr = (void *)((uint64_t)state - state->offset);
    }
    return addr;
}

std::size_t
shmmgr::get_segment_size(const char *name) {
    std::pair<shm_segment*, std::size_t> res;
    shm_segment* state;
    managed_shared_memory *shm = (managed_shared_memory *)mmgr_;

    res = shm->find<shm_segment>(name);
    state = res.first;
    if (!state) {
        return 0;
    }
    return state->size;
}

}    // namespace lib
}    // namespace sdk
