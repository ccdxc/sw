#include <assert.h>
#include <iostream>
#include "nic/include/base.h"
#include "nic/utils/shm/shmseg.hpp"

namespace utils {
namespace shm {

shmseg::shmseg(char *name, const std::size_t size, shm_mode_e mode, void *baseaddr):name_(name)
{
    // basic validation(s)
    if ((name == NULL) || size <= 16 || baseaddr == NULL) {
        assert(FALSE);
    }

    // remove shared memory segment on construction/destruction
    struct shm_remove {
        char *name_;
        shm_remove(char *name) { name_ = name; }
        ~shm_remove() { shared_memory_object::remove(name_); }
    } remover(name);

    if (mode == CREATE_ONLY) {
        fixed_mgr_shm_ = new fixed_managed_shared_memory(create_only, name, size, baseaddr);
    } else if (mode == OPEN_ONLY) {
        fixed_mgr_shm_ = new fixed_managed_shared_memory(open_only, name, baseaddr);
    } else if (mode == OPEN_OR_CREATE) {
        fixed_mgr_shm_ = new fixed_managed_shared_memory(open_or_create, name, size, baseaddr);
    } else if (mode == OPEN_READ_ONLY) {
        fixed_mgr_shm_ = new fixed_managed_shared_memory(open_read_only, name, baseaddr);
    } else {
        assert(FALSE);
    }

    assert(fixed_mgr_shm_ != NULL);
}

shmseg::~shmseg()
{
    std::cout << "Deleting " << name_.c_str();
    shared_memory_object::remove(name_.c_str());
}

void *
shmseg::allocate(const std::size_t size, const std::size_t alignment)
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

void
shmseg::deallocate(void *mem)
{
    assert(mem != NULL);
    fixed_mgr_shm_->deallocate(mem);
}

std::size_t
shmseg::size(void) const
{
    return fixed_mgr_shm_->get_size();
}

std::size_t
shmseg::free_size(void) const
{
    return fixed_mgr_shm_->get_free_memory();
}

}    // namespace shm
}    // namespace utils
