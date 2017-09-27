#define ASSERT_PHVWR(d, d_start, d_end, s, s_start, s_end) \
    .assert((offsetof(d, d_start) - offsetof(d, d_end)) == \
            (offsetof(s, s_start) - offsetof(s, s_end)))
