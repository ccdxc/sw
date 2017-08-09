#ifndef __BITMAP_HPP__
#define __BITMAP_HPP__

#include <base.h>
#include <hal_lock.hpp>

namespace hal {
namespace utils {

#define HAL_INVALID_BIT_POS 0xffffffff

class bitmap {
public:
    static bitmap *factory(uint32_t size, bool thread_safe=false);
    ~bitmap();
    hal_ret_t set(uint32_t posn);
    bool is_set(uint32_t posn);
    hal_ret_t clear(uint32_t posn);
    uint32_t num_set(void) const { return num_set_; }
    // Get the first set index
    hal_ret_t first_set(uint32_t *first_set_p);
    hal_ret_t next_set(uint32_t posn, uint32_t *set_p);

private:
    hal_spinlock_t    slock_;          // lock for thread safety
    bool              thread_safe_;    // TRUE if thread safety is needed
    uint32_t          num_words_;      // size in terms of 32 bit words
    uint32_t          *bits_;          // bits allocated for this
    uint32_t          num_set_;        // number of bits that are set

private:
    bitmap() {};
    hal_ret_t init(uint32_t size, bool thread_safe);
    hal_ret_t first_set_(uint32_t posn, uint32_t *first_set_p);
};

}    // namespace utils
}    // namespace hal

#endif    //__BITMAP_HPP__

