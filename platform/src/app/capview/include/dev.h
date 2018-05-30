
/*
 * Copyright (c) 2018, Pensando Systems Inc.
 */

#ifndef __DEV_H__
#define __DEV_H__

class dev_io_exception : public std::exception {
public:
    dev_io_exception() {}
    ~dev_io_exception() {}
};

void dev_read(uint32_t *wp, uint64_t addr, int count);
void dev_write(const uint32_t *rp, uint64_t addr, int count);

#endif
