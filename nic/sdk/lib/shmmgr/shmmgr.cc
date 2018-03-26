#include <assert.h>
#include <iostream>
#include <boost/interprocess/managed_shared_memory.hpp>
#include "sdk/base.hpp"
#include "sdk/mem.hpp"
#include "lib/shmmgr/shmmgr.hpp"

using namespace boost::interprocess;
#define TO_FM_SHM(x)    ((fixed_managed_shared_memory *)(x))

namespace sdk {
namespace lib {

//------------------------------------------------------------------------------
// initialization routine
//------------------------------------------------------------------------------
bool
shmmgr::init(const char *name, const std::size_t size,
             shm_mode_e mode, void *baseaddr)
{
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

    strncpy(name_, name, SHMSEG_NAME_MAX_LEN);
    mmgr_ = fixed_mgr_shm;
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
    if ((name == NULL) || size <= 16 || baseaddr == NULL) {
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

    mem = SDK_CALLOC(HAL_MEM_ALLOC_LIB_SHM, sizeof(shmmgr));
    if (mem == NULL) {
        return NULL;
    }

    new_shmmgr = new (mem) shmmgr();
    if (new_shmmgr->init(name, size, mode, baseaddr) == false) {
        new_shmmgr->~shmmgr();
        SDK_FREE(HAL_MEM_ALLOC_LIB_SHM, new_shmmgr);
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
    SDK_FREE(HAL_MEM_ALLOC_LIB_SHM, mmgr);
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
        fixed_managed_shared_memory seg(open_only, name, baseaddr);
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
    void    *ptr;

    if (alignment == 0) {
        ptr = TO_FM_SHM(mmgr_)->allocate(size);
    } else {
        assert((alignment & (alignment - 1)) == 0);
        assert(alignment >= 4);
        ptr = TO_FM_SHM(mmgr_)->allocate_aligned(size, alignment);
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
    TO_FM_SHM(mmgr_)->deallocate(mem);
}

//------------------------------------------------------------------------------
// return the size of the shared memory segment
//------------------------------------------------------------------------------
std::size_t
shmmgr::size(void) const
{
    return TO_FM_SHM(mmgr_)->get_size();
}

//------------------------------------------------------------------------------
// return the size of the free memory available
//------------------------------------------------------------------------------
std::size_t
shmmgr::free_size(void) const
{
    return TO_FM_SHM(mmgr_)->get_free_memory();
}

void *
shmmgr::mmgr(void) const
{
    return mmgr_;
}


}    // namespace lib
}    // namespace sdk
