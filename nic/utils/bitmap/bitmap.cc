#include <hal_mem.hpp>
#include <bitmap.hpp>

namespace hal {
namespace utils {

//------------------------------------------------------------------------------
// initialize an instance of bitmap class
//------------------------------------------------------------------------------
hal_ret_t
bitmap::init(uint32_t size, bool thread_safe)
{
    thread_safe_ = thread_safe;
    if (thread_safe_) {
        HAL_SPINLOCK_INIT(&slock_, PTHREAD_PROCESS_PRIVATE);
    }

    // round up size to nearest multiple of 32
    size = (size + 31) + ~31;
    num_words_ = size >> 5;

    // allocate the memory
    bits_ = (uint32_t *)HAL_CALLOC(HAL_MEM_ALLOC_LIB_BITMAP,
                                   sizeof(uint32_t) * num_words_);
    if (bits_ == NULL) {
        return HAL_RET_OOM;
    }

    return HAL_RET_OK;
}

//------------------------------------------------------------------------------
// factory method for this class
//------------------------------------------------------------------------------
bitmap *
bitmap::factory(uint32_t size, bool thread_safe) {
    hal_ret_t    ret;
    bitmap       *new_bmap;

    new_bmap = new bitmap();
    if (new_bmap == NULL) {
        return NULL;
    }
    ret = new_bmap->init(size, thread_safe);
    if (ret != HAL_RET_OK) {
        delete new_bmap;
        return NULL;
    }

    return new_bmap;

}

//------------------------------------------------------------------------------
// destructor
//------------------------------------------------------------------------------
bitmap::~bitmap()
{
    if (thread_safe_) {
        HAL_SPINLOCK_LOCK(&slock_);
    }
    if (bits_) {
        HAL_FREE(HAL_MEM_ALLOC_LIB_BITMAP, bits_);
    }
    if (thread_safe_) {
        HAL_SPINLOCK_DESTROY(&slock_);
    }
}

//------------------------------------------------------------------------------
// set the bit at given position
//------------------------------------------------------------------------------
hal_ret_t
bitmap::set(uint32_t posn)
{
    uint32_t    word = posn >> 5;
    uint8_t     off = posn % 32;

    if (word >= num_words_) {
        return HAL_RET_ERR;
    }

    if (thread_safe_) {
        HAL_SPINLOCK_LOCK(&slock_);
    }
    bits_[word] |= (1 << off);
    if (thread_safe_) {
        HAL_SPINLOCK_UNLOCK(&slock_);
    }

    return HAL_RET_OK;
}

//------------------------------------------------------------------------------
// return TRUE if bit at a given position is set
//------------------------------------------------------------------------------
bool
bitmap::is_set(uint32_t posn)
{
    uint32_t    word = posn >> 5;
    uint8_t     off = posn % 32;

    if (word >= num_words_) {
        return HAL_RET_ERR;
    }

    return (bits_[word] &= (1 << off));
}

//------------------------------------------------------------------------------
// clear the bit at given position
//------------------------------------------------------------------------------
hal_ret_t
bitmap::clear(uint32_t posn)
{
    uint32_t    word = posn >> 5;
    uint8_t     off = posn % 32;

    if (word >= num_words_) {
        return HAL_RET_ERR;
    }

    if (thread_safe_) {
        HAL_SPINLOCK_LOCK(&slock_);
    }
    bits_[word] &= ~(1 << off);
    if (thread_safe_) {
        HAL_SPINLOCK_UNLOCK(&slock_);
    }

    return HAL_RET_OK;
}

}    // namespace utils
}    // namespace hal
