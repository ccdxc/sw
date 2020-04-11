
/*
 * Copyright (c) 2018, Pensando Systems Inc.
 */

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <unistd.h>
#include "dtls.h"
#include "cvdbfile.h"

// mmaped data
const char *mem;

// Globals
int CVDBReg::ver_reg_idx;

// Registers
const cvdb_reg *CVDBReg::reg_tab;
const cvdb_reg **CVDBReg::reg_addrtab;
const char *CVDBReg::reg_strtab;
int CVDBReg::nregs;

// Fields
static const char *field_tab;
static const char *field_strtab;

static int
cmp_byaddr(const void *v1, const void *v2)
{
    auto r1 = *(cvdb_reg **)v1;
    auto r2 = *(cvdb_reg **)v2;

    return (r1->addr > r2->addr) ? 1 :
           (r1->addr == r2->addr) ? 0 : -1;
}

int
CVDBReg::loadfile(const char *path)
{
    int fd = open(path, O_RDONLY, 0);
    if (fd < 0) {
        perror(path);
        return -1;
    }

    struct stat st;
    fstat(fd, &st);
    const void *v = mmap(NULL, st.st_size, PROT_READ, MAP_SHARED, fd, 0);
    close(fd);
    if (v == (char *)-1) {
        perror("mmap");
        return -1;
    }
    mem = (const char *)v;
    auto *hdr = (cvdb_hdr *)mem;

    // Verify header
    if (hdr->magic != CVDB_MAGIC) {
        fprintf(stderr, "%s: bad magic\n", path);
        return -1;
    }
    ver_reg_idx = hdr->ver_reg_idx;

    // Registers
    nregs = hdr->sect[SECT_REG_TAB].size / sizeof (cvdb_reg);
    reg_tab = (cvdb_reg *)(mem + hdr->sect[SECT_REG_TAB].offs);
    reg_strtab = mem + hdr->sect[SECT_REG_STRTAB].offs;
    if (ver_reg_idx >= nregs) {
        fprintf(stderr, "%s: invalid version register index\n", path);
        return -1;
    }

    // Fields
    field_tab = mem + hdr->sect[SECT_FIELD_TAB].offs;
    field_strtab = mem + hdr->sect[SECT_FIELD_STRTAB].offs;
    
    // Sorted address table
    reg_addrtab = (const cvdb_reg **)malloc(nregs * sizeof (cvdb_reg *));
    for (int i = 0; i < nregs; i++) {
        reg_addrtab[i] = &reg_tab[i];
    }
    qsort(reg_addrtab, nregs, sizeof (reg_addrtab[0]), cmp_byaddr);

    return 0;
}

uint32_t
CVDBReg::dbbuild(void)
{
    auto *hdr = (cvdb_hdr *)mem;
    return hdr->build;
}

uint32_t
CVDBReg::dbver(void)
{
    auto *hdr = (cvdb_hdr *)mem;
    return hdr->ver;
}

CVDBReg
CVDBReg::get_byidx(int idx)
{
    if (idx < 0 || idx >= nregs) {
        throw std::out_of_range("invalid register index");
    }
    return CVDBReg(&reg_tab[idx]);
}

CVDBReg
CVDBReg::get_version(void)
{
    return CVDBReg(&reg_tab[ver_reg_idx]);
}

static int
cmp_regaddrs(const void *v1, const void *v2)
{
    auto addr = (uint32_t)(intptr_t)v1;
    auto *r = *(cvdb_reg **)v2;
    uint32_t raddr = r->addr & ~0x1;

    if (addr < raddr) {
        return -1;
    }
    uint32_t eaddr;
    if (r->nrows == 0) {
        eaddr = raddr + r->width;
    } else {
        eaddr = raddr + r->stride * r->nrows;
    }
    return (addr >= eaddr);
}

CVDBReg
CVDBReg::get_byaddr(uint32_t addr)
{
    auto rr = (cvdb_reg **)bsearch((void *)(intptr_t)addr, reg_addrtab, nregs,
            sizeof (cvdb_reg *), cmp_regaddrs);
    if (rr == NULL) {
        return end();
    }
    return CVDBReg(*rr);
}

static int
cmp_regnames(const void *v1, const void *v2)
{
    auto *k = (char *)v1;
    auto *r = (cvdb_reg *)v2;
    return strcmp(k, CVDBReg::cvdb_regname(r));
}

CVDBReg
CVDBReg::get_byname(const char *name)
{
    auto r = (cvdb_reg *)bsearch(name, reg_tab, nregs,
            sizeof (cvdb_reg), cmp_regnames);
    if (r == NULL) {
        return end();
    }
    return CVDBReg(r);
}

static int
cmp_regnamepref(const void *v1, const void *v2)
{
    auto *k = (char *)v1;
    auto *r = (cvdb_reg *)v2;
    return strncmp(k, CVDBReg::cvdb_regname(r), strlen(k));
}

CVDBReg
CVDBReg::first_byname(const char *name)
{
    const cvdb_reg *r;
    r = (cvdb_reg *)bsearch(name, reg_tab, nregs,
            sizeof (cvdb_reg), cmp_regnamepref);
    if (r == NULL) {
        return end();
    }
    while (r > reg_tab) {
        if (strncmp(name, reg_strtab + (r - 1)->name, strlen(name)) != 0) {
            break;
        }
        --r;
    }
    return CVDBReg(r);
}

CVDBReg
CVDBReg::next_byname(const char *name) const
{
    if (reg + 1 == &reg_tab[nregs] ||
        strncmp(name, reg_strtab + (reg + 1)->name, strlen(name)) != 0) {
        return end();
    }
    return CVDBReg(reg + 1);
}

int
CVDBReg::nwords(void) const
{
    return (field(0).hi() + 32) / 32;
}

int CVDBReg::nfields(void) const
{
    auto ftab = (cvdb_field_tab *)(field_tab + reg->fldoffs);
    return ftab->nfields;
}

CVDBField CVDBReg::field(int idx) const
{
    auto ftab = (cvdb_field_tab *)(field_tab + reg->fldoffs);
    if (idx < 0 || idx >= (int)ftab->nfields) {
        throw std::out_of_range("invalid field index");
    }
    return CVDBField(&ftab->field[idx]);
}

CVDBField CVDBReg::field_byname(const char *name) const
{
    auto ftab = (cvdb_field_tab *)(field_tab + reg->fldoffs);
    for (int i = 0; i < (int)ftab->nfields; i++) {
        CVDBField fld(&ftab->field[i]);
        if (strcmp (fld.name(), name) == 0) {
            return fld;
        }
    }
    return CVDBField::end();
}

const char *
CVDBField::name(void) const
{
    return field_strtab + field->name;
}
