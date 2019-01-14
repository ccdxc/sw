
/*
 * Copyright (c) 2018, Pensando Systems Inc.
 */

#include "dtls.h"
#include "editbuf.h"
#include "dev.h"

editbuf_info_s editbuf_info;

void
editbuf_write(void)
{
    editbuf_info_s& e = editbuf_info;

    if (e.valid) {
        uint32_t buf[256];
        memset(buf, 0, sizeof (buf));
        mpz_export(buf, NULL, -1, 4, 0, 0, e.val.get_mpz_t());
        dev_write(buf, e.inf.addr, e.nwords);
    }
}

void
editbuf_read(uint32_t addr, int nbytes)
{
    editbuf_info_s& e = editbuf_info;

    uint32_t buf[256];
    e.nwords = e.inf.reg.nwords();
    dev_read(buf, e.inf.addr, e.nwords);
    mpz_import(e.val.get_mpz_t(), e.nwords, -1, 4, 0, 0, buf);
    e.valid = true;
}

int
editbuf_set_field(int hi, int lo, const mpz_class& val)
{
    editbuf_info_s& e = editbuf_info;

    int width = hi - lo + 1;

    // range check
    if (hi >= e.nwords * 32 || lo > hi || lo < 0) {
        throw std::out_of_range("edit_write_field out of range");
    }
    if (val >= mpz_class(1) << width) {
        return -1;
    }

    mpz_class mask = (mpz_class(1) << (e.nwords * 32)) - 1;
    mask ^= ((mpz_class(1) << width) - 1) << lo;
    e.val = (e.val & mask) | (val << lo);

    return 0;
}

void
editbuf_get_field(mpz_class& res, int hi, int lo)
{
    editbuf_info_s& e = editbuf_info;
    int width = hi - lo + 1;

    // range check
    if (hi >= e.nwords * 32 || lo > hi || lo < 0) {
        throw std::out_of_range("edit_read_field out of range");
    }
    res = (e.val >> lo) & ((mpz_class(1) << width) - 1);
}
