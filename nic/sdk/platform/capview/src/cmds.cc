
/*
 * Copyright (c) 2018, Pensando Systems Inc.
 */

#include "dtls.h"
#include <regex.h>
#include "editbuf.h"
#include "parse.h"
#include "cli.h"

static bool permit_secure = false;

static int
access_info_byaddr(const char *s, access_info_s *res, bool read_or_write)
{
    uint32_t addr;

    addr = strtoul(s, NULL, 0);
    CVDBReg reg = CVDBReg::get_byaddr(addr);
    if (reg == CVDBReg::end()) {
        printf("%% register does not exist\n");
        return -1;
    }
    int idx = -1;
    if (reg.is_array()) {
        addr &= -reg.stride();
        idx = (addr - reg.addr()) / reg.stride();
    } else {
        addr = reg.addr();
    }
    if (read_or_write) {
        if (reg.is_secure() && !permit_secure) {
            printf("%% cannot access secure register\n");
            return -1;
        }
    }
    res->reg = reg;
    res->idx = idx;
    res->addr = addr;
    return 0;
}


static int
access_info_byname(const char *s, access_info_s *res, bool read_or_write)
{
    parse_info inf;
    if (parse_word(s, &inf) < 0) {
        printf("%% register does not exist\n");
        return -1;
    }
    CVDBReg reg = CVDBReg::get_byname(inf.regname.c_str());
    if (reg == CVDBReg::end()) {
        printf("%% register does not exist\n");
        return -1;
    }
    uint32_t addr = reg.addr();
    int idx = -1;
    if (inf.flags & PIF_IDX) {
        if (!reg.is_array()) {
            printf("%% not an array\n");
            return -1;
        }
        idx = strtol(inf.idx.c_str(), NULL, 0);
        if (idx < 0 || idx >= reg.nrows()) {
            printf("%% index out of range (0-%d)\n", reg.nrows() - 1);
            return -1;
        }
        addr += reg.stride() * idx;
    }
    if (read_or_write) {
        if (reg.is_secure() && !permit_secure) {
            printf("%% cannot access secure register\n");
            return -1;
        }
        if (reg.is_array() && idx == -1) {
            printf("%% array index required\n");
            return -1;
        }
    }
    res->reg = reg;
    res->idx = idx;
    res->addr = addr;
    return 0;
}

static int
parse_access_info(const char *s, access_info_s *res, bool read_or_write)
{
    if (isdigit(s[0])) {
        return access_info_byaddr(s, res, read_or_write);
    } else {
        return access_info_byname(s, res, read_or_write);
    }
}

static void
display_register(CVDBReg reg, uint32_t addr, int idx)
{
    printf("0x%08x: %s", addr, reg.name());
    if (reg.is_array()) {
        printf("[%u/0x%x]", idx, idx);
    }
    putchar('\n');

    mpz_class field_val;

    gmp_printf("  val: 0x%Zx\n", editbuf_info.val);
    printf("  Fields:\n");
    size_t max_len = 0;
    for (int pass = 1; pass <= 2; pass++) {
        for (int i = 0; i < reg.nfields(); i++) {
            CVDBField field = reg.field(i);
            if (pass == 1) {
                if (strlen(field.name()) > max_len) {
                    max_len = strlen(field.name());
                }
            } else {
                editbuf_get_field(field_val, field.hi(), field.lo());
                std::string label = std::string(field.name()) + ":";
                char bbuf[32];
                if (field.hi() == field.lo()) {
                    snprintf(bbuf, sizeof (bbuf), "%d", field.lo());
                } else {
                    snprintf(bbuf, sizeof (bbuf), "%d:%d",
                            field.hi(), field.lo());
                }
                int lhs = (7 - strlen(bbuf)) / 2;
                int rhs = 7 - strlen(bbuf) - lhs;
                printf("    [%*s%s%*s] %-*s ", lhs, "", bbuf, rhs, "",
                        (int)max_len + 1, label.c_str());
                gmp_printf("0x%Zx\n", field_val);
            }
        }
    }
}

static void
info_register(CVDBReg reg, uint32_t addr, int idx)
{
    printf("0x%08x: %s", addr, reg.name());
    if (reg.is_array()) {
        if (idx == -1) {
            printf("[] %d rows", reg.nrows());
        } else {
            printf("[%u/0x%x]", idx, idx);
        }
    }
    putchar('\n');

    printf("  Fields:\n");
    size_t max_len = 0;
    for (int pass = 1; pass <= 2; pass++) {
        for (int i = 0; i < reg.nfields(); i++) {
            CVDBField field = reg.field(i);
            if (pass == 1) {
                if (strlen(field.name()) > max_len) {
                    max_len = strlen(field.name());
                }
            } else {
                std::string label = std::string(field.name());
                char bbuf[32];
                if (field.hi() == field.lo()) {
                    snprintf(bbuf, sizeof (bbuf), "%d", field.lo());
                } else {
                    snprintf(bbuf, sizeof (bbuf), "%d:%d",
                            field.hi(), field.lo());
                }
                int lhs = (7 - strlen(bbuf)) / 2;
                int rhs = 7 - strlen(bbuf) - lhs;
                printf("    [%*s%s%*s] %-*s\n", lhs, "", bbuf, rhs, "",
                        (int)max_len + 1, label.c_str());
            }
        }
    }
}

DEFUN_H(sbrk, 1, "", "Show brk use")
{
    printf("%lukB\n", ((intptr_t)sbrk(0) - (intptr_t)init_sbrk) >> 10);
    return CMD_SUCCESS;
}

DEFUN_H(secure, 1, "[on|off]", "Secure access enable")
{
    if (argc == 1) {
        printf("secure access is %s\n", permit_secure ? "on" : "off");
    } else if (strcmp(argv[1], "on") == 0) {
        permit_secure = true;
    } else if (strcmp(argv[1], "off") == 0) {
        permit_secure = false;
    } else {
        return CMD_USAGE;
    }
    return CMD_SUCCESS;
}

DEFUN(info, 2, "addr|regname", "Info register")
{
    access_info_s inf;
    if (parse_access_info(argv[1], &inf, false) < 0) {
        return CMD_FAILED;
    }

    info_register(inf.reg, inf.addr, inf.idx);

    return CMD_SUCCESS;
}

DEFUN(read, 1, "[addr|regname]", "Read register")
{
    editbuf_info_s& e = editbuf_info;

    if (argc < 2) {
        if (!e.valid) {
            printf("%% edit buffer empty\n");
            return CMD_FAILED;
        }
        editbuf_read(e.inf.addr, e.nwords);
    } else {
        access_info_s& inf = editbuf_info.inf;
        if (parse_access_info(argv[1], &inf, true) < 0) {
            return CMD_FAILED;
        }
        editbuf_read(inf.addr, inf.reg.nwords());
    }
    display_register(e.inf.reg, e.inf.addr, e.inf.idx);

    return CMD_SUCCESS;
}

DEFUN(td, 2, "addr|regname [count]", "Table dump")
{
    editbuf_info_s& e = editbuf_info;
    access_info_s& inf = editbuf_info.inf;
    int count;

    if (parse_access_info(argv[1], &inf, true) < 0) {
        return CMD_FAILED;
    }
    if (!inf.reg.is_array()) {
        printf("%% not a table\n");
        return CMD_FAILED;
    }
    if (argc > 2) {
        count = strtoul(argv[2], NULL, 0);
        if (inf.idx + count > inf.reg.nrows()) {
            count = inf.reg.nrows() - inf.idx;
        }
    } else {
        count = inf.reg.nrows() - inf.idx;
    }
    while (count--) {
        editbuf_read(inf.addr, inf.reg.nwords());
        display_register(e.inf.reg, e.inf.addr, e.inf.idx);
        inf.addr += inf.reg.stride();
        ++inf.idx;
    }
    return CMD_SUCCESS;
}

DEFUN(write, 1, "", "Write register")
{
    editbuf_info_s& e = editbuf_info;
 
    // writeback edit buffer
    if (!e.valid) {
        printf("%% edit buffer empty\n");
        return CMD_SUCCESS;
    }
    editbuf_write();
    return CMD_SUCCESS;
}

DEFUN(show, 1, "", "Show edit buffer")
{
    editbuf_info_s& e = editbuf_info;

    if (!e.valid) {
        printf("%% edit buffer empty\n");
        return CMD_SUCCESS;
    }
    display_register(e.inf.reg, e.inf.addr, e.inf.idx);
    return CMD_SUCCESS;
}

static cmd_res_e
do_set(int argc, char *argv[])
{
    editbuf_info_s& e = editbuf_info;

    for (int i = 0; i < argc; i++) {
        char *fn = argv[i];
        char *fv;
        int hi, lo;

        if (fn[0] == '=') {
            fv = fn;
            lo = 0;
            hi = e.inf.reg.field(0).hi();
        } else {
            if (fn[0] == '.') {
                ++fn;
                if (fn[0] == '\0') {
                    return CMD_USAGE;
                }
            }
            fv = strchr(fn, '=');
            if (fv == NULL) {
                return CMD_USAGE;
            }
            std::string fns(fn, fv - fn);
            CVDBField fld = e.inf.reg.field_byname(fns);
            if (fld == CVDBField::end()) {
                printf("%% field does not exist\n");
                return CMD_FAILED;
            }
            hi = fld.hi();
            lo = fld.lo();
        }
        if (fv == NULL || fv[1] == '\0') {
            return CMD_USAGE;
        }
        try {
            mpz_class val(fv + 1);
            if (editbuf_set_field(hi, lo, val) < 0) {
                printf("%% value out of range\n");
                return CMD_FAILED;
            }
        } catch (const std::exception& e) {
            return CMD_USAGE;
        }
    }
    return CMD_SUCCESS;
}

DEFUN(set, 2, "(=val|.field=val) [.field=val]", "Set field in edit buffer")
{
    editbuf_info_s& e = editbuf_info;

    if (!e.valid) {
        printf("%% edit buffer empty\n");
        return CMD_SUCCESS;
    }
    return do_set(argc - 1, argv + 1);
}

DEFUN(fset, 3, "(=val|.field=val) [.field=val]", "Read/Modifiy Write register")
{
    // 'read' followed by 'set' followed by 'write'

    access_info_s& inf = editbuf_info.inf;
    if (parse_access_info(argv[1], &inf, true) < 0) {
        return CMD_FAILED;
    }
    editbuf_read(inf.addr, inf.reg.nwords());
    cmd_res_e res = do_set(argc - 2, argv + 2);
    if (res == CMD_SUCCESS) {
        editbuf_write();
    }
    return res;
}

DEFUN(find, 2, "name", "Find symbol")
{
    regmatch_t match;
    regex_t rex;

    int r = regcomp(&rex, argv[1], REG_EXTENDED | REG_ICASE | REG_NOSUB);
    if (r != 0) {
        char errbuf[128];
        regerror(r, &rex, errbuf, sizeof (errbuf));
        printf("%% %s\n", errbuf);
        return CMD_FAILED;
    }
    int match_count = 0;
    for (int i = 0; i < CVDBReg::nregs; i++) {
        CVDBReg reg = CVDBReg::get_byidx(i);
        std::string rname = reg.name();
        if (regexec(&rex, rname.c_str(), 1, &match, 0) == 0) {
            printf("%s\n", rname.c_str());
            ++match_count;
            continue;
        }
        if (reg.is_array()) {
            rname = rname + "[0]";
        }
        for (int j = 0; j < reg.nfields(); j++) {
            CVDBField field = reg.field(j);
            std::string s = rname + "." + field.name();
            if (regexec(&rex, s.c_str(), 1, &match, 0) == 0) {
                printf("%s\n", s.c_str());
                ++match_count;
            }
        }
    }
    printf("%d match%s\n", match_count, (match_count == 1) ? "" : "es");
    return CMD_SUCCESS;
}
