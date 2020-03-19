/*
 * Copyright (c) 2017-2019, Pensando Systems Inc.
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <assert.h>

#include "pciesys.h"

void
pciesys_init(void)
{
}

void *
pciesys_zalloc(const size_t size)
{
    void *p = calloc(1, size);
    assert(p != NULL);
    return p;
}

void
pciesys_free(void *p)
{
    free(p);
}

void *
pciesys_realloc(void *p, const size_t size)
{
    p = realloc(p, size);
    assert(p != NULL);
    return p;
}

static void
logvprintf(const char *fmt, va_list ap)
{
    vprintf(fmt, ap);
}

static void
logflush(void)
{
    fflush(stdout);
}

static pciesys_logger_t default_logger = {
    .logdebug = logvprintf,
    .loginfo  = logvprintf,
    .logwarn  = logvprintf,
    .logerror = logvprintf,
    .logflush = logflush,
};

static pciesys_logger_t *current_logger = &default_logger;

void
pciesys_set_logger(pciesys_logger_t *logger)
{
    current_logger = logger;
}

void
pciesys_logdebug(const char *fmt, ...)
{
    va_list ap;

    va_start(ap, fmt);
    current_logger->logdebug(fmt, ap);
    va_end(ap);
}

void
pciesys_loginfo(const char *fmt, ...)
{
    va_list ap;

    va_start(ap, fmt);
    current_logger->loginfo(fmt, ap);
    va_end(ap);
}

void
pciesys_logwarn(const char *fmt, ...)
{
    va_list ap;

    va_start(ap, fmt);
    current_logger->logwarn(fmt, ap);
    va_end(ap);
}

void
pciesys_logerror(const char *fmt, ...)
{
    va_list ap;

    va_start(ap, fmt);
    current_logger->logerror(fmt, ap);
    va_end(ap);
}

void
pciesys_logflush(void)
{
    current_logger->logflush();
}

static void
pciesys_sbus_nop(void)
{
}

static pciesys_sbus_locker_t default_sbus_locker = {
    .sbus_lock   = pciesys_sbus_nop,
    .sbus_unlock = pciesys_sbus_nop,
};

static pciesys_sbus_locker_t *sbus_locker = &default_sbus_locker;

void
pciesys_set_sbus_locker(pciesys_sbus_locker_t *locker)
{
    sbus_locker = locker;
}

void
pciesys_sbus_lock(void)
{
    sbus_locker->sbus_lock();
}

void
pciesys_sbus_unlock(void)
{
    sbus_locker->sbus_unlock();
}
