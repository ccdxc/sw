
/*
 * Copyright (c) 2018, Pensando Systems Inc.
 */

#ifndef __PARSE_H__
#define __PARSE_H__

struct parse_info {
    std::string str;
    std::string regname;
    std::string idx;
    int field_so;
    int mi;
    uint32_t flags;
};
#define PIF_REGNAME     0x1
#define PIF_IDX         0x2
#define PIF_FULLREG     0x4
#define PIF_FULLIDX     0x8

int parse_word(const char *s, parse_info *res);
int parse_init(void);

#endif
