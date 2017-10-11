#ifndef _SKNOBS_H_
#define _SKNOBS_H_

static inline int sknobs_exists(char *name) {
    return 0;
}

static inline int sknobs_prefix_exists(const char *name) {
    return 0;
}
static inline char *sknobs_get_string(char *name, char *defaultValue) {
    return NULL;
}
static inline unsigned long long sknobs_get_value(char *name, unsigned long long defaultValue) {
    return 0;
}
#endif // _SKNOBS_H_
