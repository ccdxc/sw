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
    size = (size + 31) & ~31;
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

    return (bits_[word] & (1 << off));
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

hal_ret_t
bitmap::first_set_(uint32_t posn, uint32_t *first_set_p)
{
    uint32_t    start_word = posn >> 5;
    uint8_t     start_off = posn % 32;
    uint32_t    word;
    uint8_t     off;

    for (word = start_word; 
         !bits_[word] && (word < num_words_); 
         word++);

    if (word >= num_words_) {
        return HAL_RET_ERR;
    }

    if (word != start_word) {
        start_off = 0;
    }

    for (off = start_off; off < 32; off++) {
        /* TODO:Lazy search. Could be optimized with gcc builtins
         * or lookup tables
         */
        if (bits_[word] & (1<<off)) {
            *first_set_p = (word << 5) | off;
            return HAL_RET_OK;
        }
    }
    // Shouldn't come here 
    return HAL_RET_ERR;
}

//------------------------------------------------------------------------------
// find the first bit set
//------------------------------------------------------------------------------
hal_ret_t
bitmap::first_set(uint32_t *first_set_p)
{
    return first_set_(0, first_set_p);
}

//------------------------------------------------------------------------------
// find the next bit set after posn
//------------------------------------------------------------------------------
hal_ret_t
bitmap::next_set(uint32_t posn, uint32_t *set_p) 
{
    return first_set_(posn+1, set_p);
}

}    // namespace utils
}    // namespace hal
