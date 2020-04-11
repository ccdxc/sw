
/*
 * Copyright (c) 2018, Pensando Systems Inc.
 */

#ifndef __CVDB_H__
#define __CVDB_H__

#include "cvdbfile.h"

class CVDBField {
public:
    CVDBField() : field(NULL) {}
    CVDBField(const cvdb_field *_field) : field(_field) {}
    ~CVDBField() {}
    bool operator==(const CVDBField& a) const { return a.field == field; };
    bool operator!=(const CVDBField& a) const { return a.field != field; };
    int hi(void) const { return field->hi; }
    int lo(void) const { return field->lo; }
    const char *name(void) const;
    static CVDBField end(void) { return CVDBField(NULL); }

private:
    const cvdb_field *field;
};

class CVDBReg {
public:
    CVDBReg() : reg(NULL) {}
    CVDBReg(const cvdb_reg *_reg) : reg(_reg) {}
    ~CVDBReg() {}
    bool operator==(const CVDBReg& a) const { return a.reg == reg; };
    bool operator!=(const CVDBReg& a) const { return a.reg != reg; };

    uint32_t addr(void) const { return reg->addr & ~0x1; }
    bool is_secure(void) const { return (reg->addr & 0x1) == 0x1; }
    int nrows(void) const { return reg->nrows; }
    int width(void) const { return reg->width; }
    int stride(void) const { return reg->stride; }
    bool is_array(void) const { return reg->nrows != 0; }
    int nwords(void) const;

    int nfields(void) const;
    const char *name(void) const { return cvdb_regname(reg); }
    CVDBField field(int idx) const;
    CVDBField field_byname(const char *name) const;
    CVDBField field_byname(const std::string& name) const {
        return field_byname(name.c_str());
    }
    CVDBReg next_byname(const char *name) const;
    CVDBReg next_byname(const std::string& name) const {
        return next_byname(name.c_str());
    }

    static int nregs;
    static CVDBReg get_version(void);
    static CVDBReg get_byaddr(uint32_t addr);
    static CVDBReg get_byidx(int idx);
    static CVDBReg get_byname(const char *name);
    static CVDBReg get_byname(const std::string& name) {
        return get_byname(name.c_str());
    }
    static CVDBReg first_byname(const char *name);
    static CVDBReg first_byname(const std::string& name) {
        return first_byname(name.c_str());
    }
    static CVDBReg end(void) { return CVDBReg(NULL); }
    static const char *cvdb_regname(const cvdb_reg *reg) {
        return reg_strtab + reg->name;
    }
    static int loadfile(const char *path);
    static uint32_t dbbuild(void);
    static uint32_t dbver(void);

private:
    const cvdb_reg *reg;

    static int ver_reg_idx;
    static const struct cvdb_reg *reg_tab;
    static const char *reg_strtab;
    static const cvdb_reg **reg_addrtab;
};

#endif
