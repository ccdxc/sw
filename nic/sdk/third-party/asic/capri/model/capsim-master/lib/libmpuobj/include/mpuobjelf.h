
/*
 * Copyright (c) 2017, Pensando Systems Inc.
 */

#ifndef __MPUOBJELF_H__
#define __MPUOBJELF_H__

#define EM_PENSANDO_CAPRI_MPU       0xdac2
#define ELF64_R_CAPINFO(hi, lo, fid) (((hi) << 16) | ((lo) << 8) | (fid))
#define ELF64_R_CAPINFO_HI(x)       (((x) >> 16) & 0x3f)
#define ELF64_R_CAPINFO_LO(x)       (((x) >> 8) & 0x3f)
#define ELF64_R_CAPINFO_FID(x)      ((x) & 0xff)

class ElfSection;

class ElfStringTable {
public:
    ElfStringTable();
    ~ElfStringTable() {}
    std::vector<std::string> tab;
    std::map<std::string, int> map;
    int nbytes;
    int get(const std::string& str);
    int load_from_buf(const uint8_t *buf, size_t size);
    int load_from_section(const ElfSection *sec);
    int get_byid(int id, std::string *strp);
};

class ElfSection {
public:
    ElfSection(const std::string& name,
            Elf64_Word sh_type, Elf64_Xword sh_flags);
    ~ElfSection() {}
    void append(const void *d, size_t size);
    void dump(void);

    std::string name;
    int idx;
    Elf64_Shdr shdr;
    std::vector<uint8_t> data;

private:
    ElfSection() {}
    friend class ElfFile;
};

class ElfFile {
public:
    ElfFile();
    ~ElfFile();
    bool valid;
    MpuObjError err;
    ElfSection *add_section(const std::string& name,
            Elf64_Word sh_type, Elf64_Xword sh_flags);
    void finalize(void);
    int save(const std::string& path);
    int load(const std::string& path);
    void dump(void);
    std::vector<ElfSection *> sec_tab;
    std::map<std::string, ElfSection *> sec_byname;
    Elf64_Ehdr ehdr;
    ElfStringTable shstrtab;

private:
    int process_file(FILE *fp);
    int seterror(MpuObjError e);
};

#endif
