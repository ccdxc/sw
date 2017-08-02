#ifndef __SHMSEG_HPP__
#define __SHMSEG_HPP__

#include <boost/interprocess/managed_shared_memory.hpp>

using namespace boost::interprocess;

namespace utils {
namespace shm {

//------------------------------------------------------------------------------
// modes in which a shared memory segment can be opened/created
//------------------------------------------------------------------------------
enum shm_mode_e {
    CREATE_ONLY,
    OPEN_ONLY,
    OPEN_OR_CREATE,
    OPEN_READ_ONLY,
};

//------------------------------------------------------------------------------
// shmseg is the shared memory and it expected to be instantiated once
// on each core in process/core's private heap memory, this will manage all the
// shared memory between 
// name - name of the shared memory segment being created
// size - size of the shared memory segment being instantiated/opened
// mode - mode in which shared memory segment is being opened/created
// baseaddr - address in the caller's memory address space this shared memory
//            will be mapped to
//------------------------------------------------------------------------------
class shmseg {
public:
    // even shared
    shmseg(char *name, const std::size_t size, shm_mode_e mode, void *baseaddr);
    ~shmseg();

    // allocate memory of give size (in bytes)from underlying system shared
    // memory.  if alignment is 0, caller doesn't care if memory allocated is
    // aligned or not. if the alignment is non-zero, it should be a power of 2
    // and >= 4
    void *allocate(const std::size_t size, const std::size_t alignment = 4);

    // free the given memory back to system shared memory
    void deallocate(void *mem);

    // returns the size of this shared memory segment
    std::size_t size(void) const;

    // returns the size of the free memory in this segment
    std::size_t free_size(void) const;

private:
    fixed_managed_shared_memory    *fixed_mgr_shm_;
    std::string                    name_;
};

}    // namespace shm
}    // namespace utils

#endif    // __SHMSEG_HPP__

