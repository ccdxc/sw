#include "nic/include/hal_mem.hpp"
#include "nic/utils/bitmap/bitmap.hpp"

namespace hal {
namespace utils {

/* DEFINES BELOW ARE FOR 64 BIT WORD SIZE */
#define WORD_SIZE               64
#define WORD_SIZE_SHIFT         6
#define WORD_SIZE_MASK          0x3F
#define ALL_ONES                UINT64_MAX

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

    size_ = size;
    num_words_ = (size + WORD_SIZE - 1) >> WORD_SIZE_SHIFT;
    num_set_ = 0;

    // allocate the memory
    bits_ = (uint64_t *)HAL_CALLOC(HAL_MEM_ALLOC_LIB_BITMAP,
                                   sizeof(uint64_t) * num_words_);
    if (bits_ == NULL) {
        return HAL_RET_OOM;
    }

    return HAL_RET_OK;
}

//------------------------------------------------------------------------------
// factory method for this class
//------------------------------------------------------------------------------
bitmap *
bitmap::factory(uint32_t size, bool thread_safe)
{
    hal_ret_t    ret;
    void         *mem;
    bitmap       *new_bmap;

    new_bmap = new bitmap();
    mem = HAL_CALLOC(HAL_MEM_ALLOC_LIB_BITMAP, sizeof(bitmap));
    if (mem == NULL) {
        return NULL;
    }
    new_bmap = new (mem) bitmap();

    ret = new_bmap->init(size, thread_safe);
    if (ret != HAL_RET_OK) {
        new_bmap->~bitmap();
        HAL_FREE(HAL_MEM_ALLOC_LIB_BITMAP, mem);
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

void
bitmap::destroy(bitmap *bmap)
{
    if (!bmap) {
        return;
    }
    bmap->~bitmap();
    HAL_FREE(HAL_MEM_ALLOC_LIB_BITMAP, bmap);
}

//------------------------------------------------------------------------------
// set the bit at given position
//------------------------------------------------------------------------------
hal_ret_t
bitmap::set(uint32_t posn)
{
    uint32_t    word = posn >> WORD_SIZE_SHIFT;
    uint8_t     off = posn & WORD_SIZE_MASK;

    if (posn >= size_) {
        return HAL_RET_NO_RESOURCE;
    }

    if (thread_safe_) {
        HAL_SPINLOCK_LOCK(&slock_);
    }
    bits_[word] |= (1ull << off);
    num_set_++;
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
    uint32_t    word = posn >> WORD_SIZE_SHIFT;
    uint8_t     off = posn & WORD_SIZE_MASK;

    if (posn >= size_) {
        return HAL_RET_NO_RESOURCE;
    }

    return (bits_[word] & (1ull << off));
}

//------------------------------------------------------------------------------
// clear the bit at given position
//------------------------------------------------------------------------------
hal_ret_t
bitmap::clear(uint32_t posn)
{
    uint32_t    word = posn >> WORD_SIZE_SHIFT;
    uint8_t     off = posn & WORD_SIZE_MASK;

    if (posn >= size_) {
        return HAL_RET_NO_RESOURCE;
    }

    if (thread_safe_) {
        HAL_SPINLOCK_LOCK(&slock_);
    }
    bits_[word] &= ~(1ull << off);
    num_set_--;
    if (thread_safe_) {
        HAL_SPINLOCK_UNLOCK(&slock_);
    }

    return HAL_RET_OK;
}

hal_ret_t
bitmap::first_set_(uint32_t posn, uint32_t *first_set_p)
{
    uint32_t    start_word = posn >> WORD_SIZE_SHIFT;
    uint8_t     start_off = posn & WORD_SIZE_MASK;
    uint32_t    word;
    uint8_t     off;
    uint64_t    mask = ALL_ONES;

    mask <<= start_off;

    lock_();
    // Find the word with the first set bit. For the current word,
    // apply a mask so that all bits less than the start_off are treated
    // to be 0
    for (word = start_word; 
         (word < num_words_) && ((bits_[word] & mask)  == 0); 
         word++) {
        mask = ALL_ONES;
    }

    if (word >= num_words_) {
        unlock_();
        return HAL_RET_NO_RESOURCE;
    }

    off = ffs_(bits_[word] & mask);
    unlock_();

    HAL_ASSERT(off);
    off--;
    *first_set_p = (word << WORD_SIZE_SHIFT) | off;

    if (*first_set_p >= size_) {
        return HAL_RET_NO_RESOURCE;
    }
    return HAL_RET_OK;
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

hal_ret_t
bitmap::last_set_(uint32_t posn, uint32_t *last_set_p)
{
    uint32_t    start_word = posn >> WORD_SIZE_SHIFT;
    uint8_t     start_off = posn & WORD_SIZE_MASK;
    uint32_t    word;
    uint8_t     off;
    uint64_t    mask = ALL_ONES;

    if (start_word >= num_words_) {
        return HAL_RET_NO_RESOURCE;
    }

    if (start_off < (WORD_SIZE-1)) {
        mask = (1ull << (start_off+1)) - 1;
    }

    lock_();
    // Find the word with the first set bit in reverse. For the current word,
    // apply a mask so that all bits greater than the start_off are treated
    // to be already free 
    for (word = start_word+1; 
         (word != 0) && ((bits_[word-1] & mask) == 0); 
         word--) {
        mask = ALL_ONES;
    }

    if (word == 0) {
        unlock_();
        return HAL_RET_NO_RESOURCE;
    }

    off = log2_floor_(bits_[word-1] & mask);
    unlock_();

    HAL_ASSERT(off != 64);
    *last_set_p = ((word-1) << WORD_SIZE_SHIFT) | off;
    if (*last_set_p >= size_) {
        return HAL_RET_NO_RESOURCE;
    }
    return HAL_RET_OK;
}
//------------------------------------------------------------------------------
// find the prev bit set before posn
//------------------------------------------------------------------------------
hal_ret_t
bitmap::prev_set(uint32_t posn, uint32_t *set_p) 
{
    if (posn == 0) {
        return HAL_RET_NO_RESOURCE;
    }
    return last_set_(posn-1, set_p);
}

hal_ret_t
bitmap::first_free_(uint32_t posn, uint32_t *first_free_p)
{
    uint32_t    start_word = posn >> WORD_SIZE_SHIFT;
    uint8_t     start_off = posn & WORD_SIZE_MASK;
    uint32_t    word;
    uint8_t     off;
    uint64_t    mask = 0;

    mask = (1ull<<start_off) - 1;

    lock_();
    // Find the word with the first free bit. For the current word,
    // apply a mask so that all bits less than the start_off are treated
    // to be already set 
    for (word = start_word; 
         (word < num_words_) && (~(bits_[word] | mask) == 0); 
         word++) {
        mask = 0;
    }

    if (word >= num_words_) {
        unlock_();
        return HAL_RET_NO_RESOURCE;
    }

    off = ffs_(~(bits_[word] | mask));
    unlock_();

    HAL_ASSERT(off);
    off--;
    *first_free_p = (word << WORD_SIZE_SHIFT) | off;
    if (*first_free_p >= size_) {
        return HAL_RET_NO_RESOURCE;
    }
    return HAL_RET_OK;
}

//------------------------------------------------------------------------------
// find the first bit free
//------------------------------------------------------------------------------
hal_ret_t
bitmap::first_free(uint32_t *first_free_p)
{
    return first_free_(0, first_free_p);
}

//------------------------------------------------------------------------------
// find the next bit free after posn
//------------------------------------------------------------------------------
hal_ret_t
bitmap::next_free(uint32_t posn, uint32_t *free_p) 
{
    return first_free_(posn+1, free_p);
}

hal_ret_t
bitmap::last_free_(uint32_t posn, uint32_t *last_free_p)
{
    uint32_t    start_word = posn >> WORD_SIZE_SHIFT;
    uint8_t     start_off = posn & WORD_SIZE_MASK;
    uint32_t    word;
    uint8_t     off;
    uint64_t    mask = 0;

    if (start_word >= num_words_) {
        return HAL_RET_NO_RESOURCE;
    }

    if (start_off < (WORD_SIZE - 1)) {
        mask = ALL_ONES << (start_off+1);
    }

    lock_();
    // Find the word with the first free bit in reverse. For the current word,
    // apply a mask so that all bits greater than the start_off are treated
    // to be already set 
    for (word = start_word+1; 
         (word != 0) && (~(bits_[word-1] | mask) == 0); 
         word--) {
        mask = 0;
    }

    if (word == 0) {
        unlock_();
        return HAL_RET_NO_RESOURCE;
    }

    off = log2_floor_(~(bits_[word-1] | mask));
    unlock_();

    HAL_ASSERT(off < 64);
    *last_free_p = ((word-1) << WORD_SIZE_SHIFT) | off;
    if (*last_free_p >= size_) {
        return HAL_RET_NO_RESOURCE;
    }
    return HAL_RET_OK;
}

//------------------------------------------------------------------------------
// find the prev bit free before posn
//------------------------------------------------------------------------------
hal_ret_t
bitmap::prev_free(uint32_t posn, uint32_t *free_p) 
{
    if (posn == 0) {
        return HAL_RET_NO_RESOURCE;
    }
    return last_free_(posn-1, free_p);
}


uint32_t
bitmap::ffs_(uint64_t v)
{
#ifdef __GNUC__
    return __builtin_ffsl(v);
#else
    uint64_t        debruijn64_ = 0x022FDD63CC95386D;
    /* Convert debruijn idx to standard idx */
    const unsigned int index64_[64] =
    {
        0,  1,  2, 53,  3,  7, 54, 27,
        4, 38, 41,  8, 34, 55, 48, 28,
       62,  5, 39, 46, 44, 42, 22,  9,
       24, 35, 59, 56, 49, 18, 29, 11,
       63, 52,  6, 26, 37, 40, 33, 47,
       61, 45, 43, 21, 23, 58, 17, 10,
       51, 25, 36, 32, 60, 20, 57, 16,
       50, 31, 19, 15, 30, 14, 13, 12,
    };
    return index64_[((v & (-v)) * debruijn64_) >> 58]+1;
#endif
}

uint32_t
bitmap::log2_floor_(uint64_t v)
{
#ifdef __GNUC__
    return v ? 64 - __builtin_clzl(v) - 1: 64;
#else
    // https://graphics.stanford.edu/~seander/bithacks.html#IntegerLogLookup
    static const char LogTable256[256] = 
    {
#define LT(n) n, n, n, n, n, n, n, n, n, n, n, n, n, n, n, n
        0, 0, 1, 1, 2, 2, 2, 2, 3, 3, 3, 3, 3, 3, 3, 3,
        LT(4), LT(5), LT(5), LT(6), LT(6), LT(6), LT(6),
        LT(7), LT(7), LT(7), LT(7), LT(7), LT(7), LT(7), LT(7)
    };

    unsigned r;     // r will be lg(v)
    uint64_t t, tt; // temporaries

    if ((tt = (v >> 48)) != 0) {
        r = (t = tt >> 8) ? 56 + LogTable256[t] : 48 + LogTable256[tt];
    } else if ((tt = (v >> 32)) != 0) {
        r = (t = tt >> 8) ? 40 + LogTable256[t] : 32 + LogTable256[tt];
    } else if ((tt = (v >> 16)) != 0) {
        r = (t = tt >> 8) ? 24 + LogTable256[t] : 16 + LogTable256[tt];
    } else {
        r = (t = v >> 8) ? 8 + LogTable256[t] : LogTable256[v];
    }
    return r;
#endif
}


}    // namespace utils
}    // namespace hal
