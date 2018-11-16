
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
#include <stdexcept>

class CapCSRFile {
public:
    CapCSRFile(const char *path);
    ~CapCSRFile() {};

//private:
    struct csr_sect {
        uint32_t offs;
        uint32_t size;
    };
    struct csr_hdr {
        struct csr_sect sect[4];
    };
    enum {
        SECT_REG_TAB, SECT_FIELD_TAB, SECT_REG_STRTAB, SECT_FIELD_STRTAB
    };
    struct csr_field {
        uint16_t hi;
        uint16_t lo;
        uint32_t name;      // name index from field_strtab
    };
    struct csr_field_tab {
        uint32_t nfields;
        struct csr_field field[0];
    };
    struct csr_reg {
        uint32_t addr;      // address
        uint32_t nrows;     // table row count
        uint32_t name;      // name index from reg_strtab
        uint32_t fldoffs;   // field index from field_tab
        uint16_t width;     // register width;
        uint16_t stride;    // table stride (<= width)
    };

    // Registers
    const struct csr_reg *reg_tab;
    const char *reg_strtab;
    int nregs;

    // Fields
    const char *field_tab;
    const char *field_strtab;
    int nfields;

    union {
        const char *m;
        const struct csr_hdr *hdr;
    };
};

CapCSRFile::CapCSRFile(const char *path)
{
    int fd = open(path, O_RDONLY, 0);
    if (fd < 0) {
        //throw std::system_error(path);
    }

    struct stat st;
    fstat(fd, &st);
    const void *v = mmap(NULL, st.st_size, PROT_READ, MAP_SHARED, fd, 0);
    close(fd);
    if (v == (char *)-1) {
        //throw std::system_error("mmap");
    }
    m = (const char *)v;

    // Registers
    nregs = hdr->sect[SECT_REG_TAB].size / sizeof (csr_reg);
    reg_tab = (csr_reg *)(m + hdr->sect[SECT_REG_TAB].offs);
    reg_strtab = m + hdr->sect[SECT_REG_STRTAB].offs;

    // Fields
    field_tab = m + hdr->sect[SECT_FIELD_TAB].offs;
    field_strtab = m + hdr->sect[SECT_FIELD_STRTAB].offs;
}

int
main(int argc, char *argv[])
{
    CapCSRFile csr("out");
    printf("# Registers:\n");
    const struct CapCSRFile::csr_reg *rp = csr.reg_tab;
    for (int i = 0; i < csr.nregs; i++, rp++) {
        printf("%08x %3u [%5u x %3u] %s\n", rp->addr,
            rp->width, rp->nrows, rp->stride, csr.reg_strtab + rp->name);
        const struct CapCSRFile::csr_field_tab *fp;
        fp = (struct CapCSRFile::csr_field_tab *)(csr.field_tab + rp->fldoffs);
        for (unsigned j = 0; j < fp->nfields; j++) {
            printf("  [%3d:%3d]: %s\n", fp->field[j].hi, fp->field[j].lo,
                csr.field_strtab + fp->field[j].name);
        }
    }
    puts("OK");
    return 0;
}
