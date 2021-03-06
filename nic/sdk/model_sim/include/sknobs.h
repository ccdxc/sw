#ifndef SKNOBS_INC
#define SKNOBS_INC


#ifdef __cplusplus
extern "C" {
#endif

#define SKNOBS_MAX_LENGTH 8192

extern int sknobs_init(int argc, char **argv);
extern void sknobs_close(void);

extern int sknobs_add(char *pattern, char *value, char *comment);
extern int sknobs_load(int argc, char *argv[], char *comment);
extern int sknobs_load_string(char *name, char *buffer, char *comment);
extern int sknobs_load_file(char *filename);
extern int sknobs_load_file_if_exists(char *filename);

extern int sknobs_exists(char *name);

typedef void *sknobs_iterator_p;
extern sknobs_iterator_p sknobs_iterate(char *name);
extern int sknobs_iterator_next(sknobs_iterator_p iterator);
extern char *sknobs_iterator_get_string(sknobs_iterator_p iterator);

extern char *sknobs_get_string(char *name, char *defaultValue);
extern char *sknobs_find_file(char *filename);
extern char *sknobs_get_filename(char *name, char *defaultValue);

extern unsigned long long sknobs_get_value(char *name, unsigned long long defaultValue);
extern unsigned long long sknobs_get_dynamic_value(char *name, unsigned long long defaultValue);

extern void sknobs_set_string(char *name, char *value);
extern void sknobs_set_value(char *name, unsigned long long value);
extern void sknobs_set_seed(unsigned value);
extern int sknobs_add_string(char *s, char *comment);
extern unsigned long long sknobs_eval(char *expr);

extern void sknobs_dump(void);
extern void sknobs_save(char *filename);
extern void sknobs_set_seed(unsigned value);
extern void * sknobs_range_begin(const char * key);
extern int sknobs_get_next_value (void * iter_v, char * buf, char * value);
extern void sknobs_range_end(void *);

// extern int sknobs_prefix_exists(const char * key);

static inline int sknobs_prefix_exists(const char *name) {
    return 0;
}

extern void sknobs_do_printf(const char * format, ...);// __attribute__((weak, format(printf, 1, 2)));
#ifdef __cplusplus
}
#endif

#endif
