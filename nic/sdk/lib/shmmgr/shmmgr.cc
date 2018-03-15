#include <assert.h>
#include <iostream>
#include "sdk/base.hpp"
#include "sdk/mem.hpp"
#include "lib/shmmgr/shmmgr.hpp"

namespace sdk {
namespace lib {

//------------------------------------------------------------------------------
// initialization routine
//------------------------------------------------------------------------------
bool
shmmgr::init(const char *name, const std::size_t size,
             shm_mode_e mode, void *baseaddr)
{
    if (mode == CREATE_ONLY) {
        fixed_mgr_shm_ = new fixed_managed_shared_memory(create_only, name,
                                                         size, baseaddr);
    } else if (mode == OPEN_ONLY) {
        fixed_mgr_shm_ = new fixed_managed_shared_memory(open_only, name,
                                                         baseaddr);
    } else if (mode == OPEN_OR_CREATE) {
        fixed_mgr_shm_ = new fixed_managed_shared_memory(open_or_create, name,
                                                         size, baseaddr);
    } else if (mode == OPEN_READ_ONLY) {
        fixed_mgr_shm_ = new fixed_managed_shared_memory(open_read_only,
                                                         name, baseaddr);
    } else {
        return false;
    }

    if (fixed_mgr_shm_ == NULL) {
        return false;
    }
    strncpy(this->name_, name, SHMSEG_NAME_MAX_LEN);
    return true;
}

//------------------------------------------------------------------------------
// factory method
//------------------------------------------------------------------------------
shmmgr *
shmmgr::factory(const char *name, const std::size_t size,
                shm_mode_e mode, void *baseaddr)
{
    void      *mem;
    shmmgr    *new_seg;

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

    new_seg = new (mem) shmmgr();
    if (new_seg->init(name, size, mode, baseaddr) == false) {
        new_seg->~shmmgr();
        SDK_FREE(HAL_MEM_ALLOC_LIB_SHM, new_seg);
        return NULL;
    }
    return new_seg;
}

//------------------------------------------------------------------------------
// destroy method
//------------------------------------------------------------------------------
void
shmmgr::destroy(shmmgr *seg)
{
    if (!seg) {
        return;
    }
    SDK_TRACE_DEBUG("Deleting segment {}", seg->name_);
    shared_memory_object::remove(seg->name_);
    seg->~shmmgr();
    SDK_FREE(HAL_MEM_ALLOC_LIB_SHM, seg);
}

//------------------------------------------------------------------------------
// allocate requested amount of memory
//------------------------------------------------------------------------------
void *
shmmgr::allocate(const std::size_t size, const std::size_t alignment)
{
    void    *ptr;

    if (alignment == 0) {
        ptr = fixed_mgr_shm_->allocate(size);
    } else {
        assert((alignment & (alignment - 1)) == 0);
        assert(alignment >= 4);
        ptr = fixed_mgr_shm_->allocate_aligned(size, alignment);
    }
    assert(ptr != NULL);
    return ptr;
}

//------------------------------------------------------------------------------
// free given memory
//------------------------------------------------------------------------------
void
shmmgr::deallocate(void *mem)
{
    assert(mem != NULL);
    fixed_mgr_shm_->deallocate(mem);
}

//------------------------------------------------------------------------------
// return the size of the shared memory segment
//------------------------------------------------------------------------------
std::size_t
shmmgr::size(void) const
{
    return fixed_mgr_shm_->get_size();
}

//------------------------------------------------------------------------------
// return the size of the free memory available
//------------------------------------------------------------------------------
std::size_t
shmmgr::free_size(void) const
{
    return fixed_mgr_shm_->get_free_memory();
}

}    // namespace lib
}    // namespace sdk
