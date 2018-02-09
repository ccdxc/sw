#pragma once

namespace acl {

#ifndef container_of
#define container_of(ptr, type, member)                     \
    (type *)((char *)(ptr) - (char *) &((type *)0)->member)
#endif

struct ref_t {
    std::atomic<uint32_t> count;
    void (*free)(const ref_t *ref_count);
};

static inline void
ref_inc(const ref_t *ref_count)
{
    if (ref_count) 
        ((ref_t *)ref_count)->count++;
}

static  inline const ref_t *
ref_clone(const ref_t *ref_count)
{
    ref_inc(ref_count);
    return ref_count;
}

static inline void
ref_dec(const ref_t *ref_count)
{
    if (!ref_count) {
        return;
    }

    SDK_ASSERT(ref_count->count > 0);

    if (--((ref_t *)ref_count)->count == 0) {
        if (ref_count->free) {
            ref_count->free(ref_count);
        }
    }
}

static inline void
ref_init(ref_t * ref_count, void (*free)(const ref_t *ref_count))
{
    ref_count->count = 1;
    ref_count->free = free;
}

static inline bool
ref_is_shared(const ref_t *ref_count) {
    return ref_count ? ref_count->count > 1 : false;
}

} // namespace acl
