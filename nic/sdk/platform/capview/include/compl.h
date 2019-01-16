
/*
 * Copyright (c) 2018, Pensando Systems Inc.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <unistd.h>
#include <ctype.h>
#include <readline/readline.h>
#include <readline/history.h>
#include <regex.h>
#include <map>
#include "dtls.h"

static regex_t *cli_rex;
static int cli_nrex;

struct rexdesc_s {
    int regname_mi;
    int fieldname_mi;
    int idx_mi;
    uint32_t flags;
    const char *pat;
};
#define RDF_REG     0x1
#define RDF_IDX     0x2

#define VAR "([^.[]*)"
#define NUM "([^]]*)"
static const struct rexdesc_s cli_rexdesc[] = {
    {  1,  3,  2, RDF_REG | RDF_IDX, "^" VAR "\\[" NUM "\\]\\." VAR "$" },
    {  1, -1,  2, RDF_REG | RDF_IDX, "^" VAR "\\[" NUM "\\]$" },
    {  1, -1,  2, RDF_REG,           "^" VAR "\\[" NUM "$" },
    {  1,  2, -1, RDF_REG,           "^" VAR "\\." VAR "$" },
    {  1, -1, -1, 0,                 "^" VAR "$" },
};

static int
cli_compile_rex(void)
{
    cli_nrex = sizeof (cli_rexdesc) / sizeof (cli_rexdesc[0]);
    cli_rex = (regex_t *)calloc(sizeof (cli_rex[0]), cli_nrex);
    
    const int rflags = REG_EXTENDED | REG_ICASE;
    int r = 0;
    for (int i = 0; i < cli_nrex; i++) {
        r |= regcomp(&cli_rex[i], cli_rexdesc[i].pat, rflags);
    }
    if (r) {
        fprintf(stderr, "regcomp failed\n");
        return -1;
    }
    return 0;
}

struct parse_info {
    std::string str;
    std::string regname;
    std::string fieldname;
    std::string idx;
    int field_so;
    int mi;
    uint32_t flags;
};
#define PIF_REGNAME     0x1
#define PIF_FIELDNAME   0x2
#define PIF_IDX         0x4
#define PIF_FULLREG     0x8
#define PIF_FULLIDX     0x10

#if 0
static std::map<uint32_t, std::string> parse_info_flagsmap = {
    { PIF_REGNAME,      "REGNAME"   },
    { PIF_FIELDNAME,    "FIELDNAME" },
    { PIF_IDX,          "IDX"       },
    { PIF_FULLREG,      "FULLREG"   },
    { PIF_FULLIDX,      "FULLIDX"   },
};

static std::string
cli_flags2string(std::map<uint32_t, std::string> xmap, uint32_t flags)
{
    std::string res = "";
    while (flags) {
        uint32_t bit = flags ^ (flags & (flags - 1));
        flags &= ~bit;
        const std::string& fstr = xmap[bit];
        res = fstr + " " + res;
    }
    res.insert(0, 1, '[');
    if (res.back() == ' ') {
        res.pop_back();
    }
    res.push_back(']');
    return res;
}
#endif

static std::string
cli_match_string(const char *s, const regmatch_t *mp)
{
    if (mp->rm_so == -1) {
        throw std::runtime_error("rm_so == -1");
    }
    return std::string(s + mp->rm_so, mp->rm_eo - mp->rm_so);
}

static int
cli_parse_word(const char *s, parse_info *res)
{
    regmatch_t match[4];
    int mi;

    res->str = "";
    res->regname = "";
    res->fieldname = "";
    res->idx = "";
    res->field_so = -1;
    res->mi = -1;
    res->flags = 0;

    for (mi = 0; mi < cli_nrex; mi++) {
        if (regexec(&cli_rex[mi], s, 4, match, 0) == 0) {
            break;
        }
    }
    if (mi == cli_nrex) {
        return -1;
    }
#if 0
    printf("match:");
    for (int i = 0; i < 4; i++) {
        printf("  [%d]: %2d, %2d\n", i, match[i].rm_so, match[i].rm_eo);
    }
#endif

    res->str = cli_match_string(s, &match[0]);
    res->mi = mi;

    const rexdesc_s *rp = &cli_rexdesc[mi];
#if 0
    printf("rexdesc[%d]:\n", mi);
    printf("  %-14s %d\n", "regname_mi:", rp->regname_mi);
    printf("  %-14s %d\n", "fieldname_mi:", rp->fieldname_mi);
    printf("  %-14s %d\n", "idx_mi:", rp->idx_mi);
    printf("  %-14s %d\n", "flags:", rp->flags);
    printf("  %-14s \"%s\"\n", "pat:", rp->pat);
#endif
    if (rp->regname_mi != -1) {
        res->regname = cli_match_string(s, &match[rp->regname_mi]);
        res->flags |= PIF_REGNAME;
        if (rp->flags & RDF_REG) {
            res->flags |= PIF_FULLREG;
        }
    }
    if (rp->fieldname_mi != -1) {
        res->fieldname = cli_match_string(s, &match[rp->fieldname_mi]);
        res->field_so = match[rp->fieldname_mi].rm_so;
        res->flags |= PIF_FIELDNAME;
    }
    if (rp->idx_mi != -1) {
        res->idx = cli_match_string(s, &match[rp->idx_mi]);
        res->flags |= PIF_IDX;
        if (rp->flags & RDF_IDX) {
            res->flags |= PIF_FULLIDX;
        }
    }
    return 0;
}

#if 1
static char *
cli_register_generator(const char *s, int state)
{
    static struct parse_info inf;
    static CVDBReg reg;
    static enum { IT_REGNAMES, IT_FIELDNAMES } mode;
    static int fidx;
    char *match = NULL;

    if (state == 0) {
        if (cli_parse_word(s, &inf) < 0) {
            return NULL;
        }
        if (inf.flags & PIF_FULLREG) {
            if (!(inf.flags & PIF_FIELDNAME)) {
                return NULL;
            }
            reg = CVDBReg::get_byname(inf.regname.c_str());
            if (reg == CVDBReg::end()) {
                return NULL;
            }
            mode = IT_FIELDNAMES;
            fidx = 0;
        } else {
            reg = CVDBReg::first_byname(inf.regname.c_str());
            mode = IT_REGNAMES;
        }
    }

    if (mode == IT_REGNAMES) {
        if (reg == CVDBReg::end()) {
            return NULL;
        }
        match = strdup(reg.name());
        reg = reg.next_byname(inf.regname.c_str());
    } else {
        while (fidx < reg.nfields()) {
            CVDBField f = reg.field(fidx++);
            if (strncmp(f.name(), inf.fieldname.c_str(),
                    inf.fieldname.length()) == 0) {
                match = strdup((std::string(s, inf.field_so) +
                        f.name()).c_str());
                break;
            }
        }
    }
    return match;
}
#else
static char *
cli_register_generator(const char *s, int state)
{
    static struct parse_info inf;

    printf("\ngenerate \"%s\"\n", s);

    if (cli_parse_word(s, &inf) < 0) {
        printf("  NULL\n");
        rl_forced_update_display();
        return NULL;
    }
    printf("res:\n");
    printf("  str:       \"%s\"\n", inf.str.c_str());
    printf("  regname:   \"%s\"\n", inf.regname.c_str());
    printf("  fieldname: \"%s\"\n", inf.fieldname.c_str());
    printf("  idx:       \"%s\"\n", inf.idx.c_str());
    printf("  field_so:  %d\n", inf.field_so);
    printf("  mi:        %d\n", inf.mi);
    printf("  flags:     %s\n",
            cli_flags2string(parse_info_flagsmap, inf.flags).c_str());
    rl_forced_update_display();
    return NULL;
}
#endif

static char *
cli_command_generator(const char *s, int state)
{
    static int idx;

    if (state == 0) {
        idx = 0;
    }
    switch (idx++) {
    case 0:
        return strdup("cmd_zero");
    case 1:
        return strdup("cmd_one");
    case 2:
        return strdup("cmd_two");
    default:
        return NULL;
    }
}

static char **
cli_generator(const char *text, int start, int end)
{
    char **matches = NULL;

#if 0
    printf("\n");
    printf(": %s, %d, %d\n", text, start, end);
    printf(": %s\n", rl_line_buffer);
    if (end == start) {
        printf(": %*s\"\n", start, "");
    } else {
        printf(": %*s'%*s'\n", start, "", (end - start - 1), "");
    }
#endif
    if (start == 0) {
        matches = rl_completion_matches(text, cli_command_generator);
        rl_completion_append_character = ' ';
    } else {
        matches = rl_completion_matches(text, cli_register_generator);
        rl_completion_append_character = '\0';
    }
    rl_attempted_completion_over = 1;
    return matches;
}
