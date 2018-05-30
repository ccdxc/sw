
/*
 * Copyright (c) 2018, Pensando Systems Inc.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <unistd.h>
#include <ctype.h>
#if defined(__x86_64__)
#include <readline/readline.h>
#include <readline/history.h>
#endif
#include <regex.h>
#include <map>
#include "dtls.h"
#include "parse.h"

static regex_t *parse_rex;
static int parse_nrex;

struct rexdesc_s {
    int regname_mi;
    int idx_mi;
    uint32_t flags;
    const char *pat;
};
#define RDF_REG     0x1
#define RDF_IDX     0x2

#define VAR "([^.[]*)"
#define NUM "([^]]*)"
static const struct rexdesc_s parse_rexdesc[] = {
    {  1,  2, RDF_REG | RDF_IDX, "^" VAR "\\[" NUM "\\]$" },
    {  1, -1, RDF_REG,           "^" VAR "\\[" NUM "$" },
    {  1, -1, 0,                 "^" VAR "$" },
};

static int
parse_compile_rex(void)
{
    parse_nrex = sizeof (parse_rexdesc) / sizeof (parse_rexdesc[0]);
    parse_rex = (regex_t *)calloc(sizeof (parse_rex[0]), parse_nrex);
    
    const int rflags = REG_EXTENDED | REG_ICASE;
    int r = 0;
    for (int i = 0; i < parse_nrex; i++) {
        r |= regcomp(&parse_rex[i], parse_rexdesc[i].pat, rflags);
    }
    if (r) {
        fprintf(stderr, "regcomp failed\n");
        return -1;
    }
    return 0;
}

static std::string
parse_match_string(const char *s, const regmatch_t *mp)
{
    if (mp->rm_so == -1) {
        throw std::runtime_error("rm_so == -1");
    }
    return std::string(s + mp->rm_so, mp->rm_eo - mp->rm_so);
}

int
parse_word(const char *s, parse_info *res)
{
    regmatch_t match[4];
    int mi;

    res->str = "";
    res->regname = "";
    res->idx = "";
    res->field_so = -1;
    res->mi = -1;
    res->flags = 0;

    for (mi = 0; mi < parse_nrex; mi++) {
        if (regexec(&parse_rex[mi], s, 4, match, 0) == 0) {
            break;
        }
    }
    if (mi == parse_nrex) {
        return -1;
    }

    res->str = parse_match_string(s, &match[0]);
    res->mi = mi;

    const rexdesc_s *rp = &parse_rexdesc[mi];
    if (rp->regname_mi != -1) {
        res->regname = parse_match_string(s, &match[rp->regname_mi]);
        res->flags |= PIF_REGNAME;
        if (rp->flags & RDF_REG) {
            res->flags |= PIF_FULLREG;
        }
    }
    if (rp->idx_mi != -1) {
        res->idx = parse_match_string(s, &match[rp->idx_mi]);
        res->flags |= PIF_IDX;
        if (rp->flags & RDF_IDX) {
            res->flags |= PIF_FULLIDX;
        }
    }
    return 0;
}

int
parse_init(void)
{
    if (parse_compile_rex() < 0) {
        return -1;
    }
    return 0;
}
