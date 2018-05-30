
/*
 * Copyright (c) 2018, Pensando Systems Inc.
 */

#ifndef __EDITBUF_H__
#define __EDITBUF_H__

struct editbuf_info_s {
    mpz_class val;
    int nwords;
    access_info_s inf;
    bool valid;
};
extern editbuf_info_s editbuf_info;

void editbuf_write(void);
void editbuf_read(uint32_t addr, int nbytes);
int editbuf_set_field(int hi, int lo, const mpz_class& val);
void editbuf_get_field(mpz_class& res, int hi, int lo);

#endif
