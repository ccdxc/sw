/*
 * Copyright (c) 2017-2018, Pensando Systems Inc.
 */

#ifndef __PCIESYS_H__
#define __PCIESYS_H__

#ifdef __cplusplus
extern "C" {
#if 0
} /* close to calm emacs autoindent */
#endif
#endif

void pciesys_init(void);

void *pciesys_zalloc(const size_t size);
void pciesys_free(void *p);
void *pciesys_realloc(void *p, const size_t size);

void pciesys_logdebug(const char *fmt, ...)
    __attribute__((format (printf, 1, 2)));
void pciesys_loginfo(const char *fmt, ...)
    __attribute__((format (printf, 1, 2)));
void pciesys_logwarn(const char *fmt, ...)
    __attribute__((format (printf, 1, 2)));
void pciesys_logerror(const char *fmt, ...)
    __attribute__((format (printf, 1, 2)));

typedef struct pciesys_logger_s {
    void (*logdebug)(const char *fmt, va_list ap);
    void (*loginfo)(const char *fmt, va_list ap);
    void (*logwarn)(const char *fmt, va_list ap);
    void (*logerror)(const char *fmt, va_list ap);
} pciesys_logger_t;

void pciesys_set_logger(pciesys_logger_t *logger);

#ifdef __cplusplus
}
#endif

#endif /* __PCIESYS_H__ */
