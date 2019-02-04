
/*
 * Copyright (c) 2017, Pensando Systems Inc.
 */

#ifndef __LIBMPUOBJ_H__
#define __LIBMPUOBJ_H__

#include <string>
#include <vector>
#include <map>
#include <elf.h>
#include "mpuobjerr.h"
#include "mpuobjelf.h"
#include "mpuobjcov.h"

/*
 * MpuSymbol
 * This object represents one symbol in a program.  There are two symbol types:
 *      MPUSYM_LABEL        - used for branch references within the program
 *      MPUSYM_PARAM        - used for '.param' instance parameters
 */
#define MPUOBJSYMBOLTYPE_GENERATOR(mac) \
    mac(MPUSYM_LABEL,   "label") \
    mac(MPUSYM_PARAM,   "param")

#define MPUOBJSYMBOLTYPE_ENUM_GEN(a, b) a,

enum MpuSymbolType {
    MPUOBJSYMBOLTYPE_GENERATOR(MPUOBJSYMBOLTYPE_ENUM_GEN)
};

class MpuSymbol {
public:
    MpuSymbol(const std::string& name, MpuSymbolType type, uint64_t val);
    MpuSymbol(const MpuSymbol& osym);
    MpuSymbolType type;                 // symbol type
    std::string name;                   // name
    int id;                             // index in symbol table
    uint64_t val;                       // value
    // pretty print
    void dump(int indent = 0) const;
    static std::map<MpuSymbolType, const std::string> typenamemap;
};

/*
 * MpuSymbolTable
 * An MpuSymbolTable object holds an array of MpuSymbol objects, accessible
 * by array index or name.
 */
class MpuSymbolTable {
public:
    MpuSymbolTable() {}
    MpuSymbolTable(const MpuSymbolTable& src);
    MpuSymbolTable& operator=(const MpuSymbolTable& src);
    ~MpuSymbolTable();
    int add(const MpuSymbol& sym);          // error if already exists
    void set(const MpuSymbol& sym);         // modify if already exists
    MpuSymbol *get_byname(const std::string& name) const;
    MpuSymbol *get_byid(unsigned id) const;
    // pretty print
    void dump(int indent = 0) const;
    size_t size(void) const;
    const std::vector<MpuSymbol *>& tab_ref(void) const;

private:
    std::map<std::string, MpuSymbol *> sym_map;
    std::vector<MpuSymbol *> sym_tab;
};

/*
 * MpuReloc
 * This is a relocation object which holds an external program reference
 * (called 'instance parameters') to an opcode field within a program.
 * The 'hi'/'lo' fields select which portion of the eventual value to write
 * into the opcode field; supporting the capas hiword() and loword()
 * functions.
 */
class MpuReloc {
public:
    MpuReloc(int pc, const std::string& name, int hi, int lo, int fid);
    ~MpuReloc() {}
    int pc;                             // bytewise PC to apply reloc to
    std::string name;                   // instance parameter name
    int hi;                             // bitfield hi
    int lo;                             // bitfield lo
    int fid;                            // ISAField index
    void dump(int indent = 0) const;    // pretty print
};

/*
 * MpuRelocTable
 * This is an array of MpuReloc objects, specifying all the relocations
 * for a program.
 */
class MpuRelocTable {
public:
    MpuRelocTable() {}
    ~MpuRelocTable() {}
    void add(const MpuReloc& reloc);
    const std::vector<MpuReloc>& tab_ref(void) const;
    void dump(int indent = 0) const;    // pretty print

private:
    std::vector<MpuReloc> reloc_tab;
};

/*
 * MpuProgram
 * ----------
 * The MpuProgram is the main interface to the library.
 *
 * To load a program from a file:
 *      MpuProgram prog;
 *      if (prog.load_from_elf(pathname) < 0) {
 *          see prog.err;
 *      }
 *
 * A program is 'valid' (prog.valid == true) if it has been successfully read
 * from the file.
 *
 * A program is 'complete' (prog.complete == true) if there are no undefined
 * instance parameters.  A 'complete' program is ready to use.
 *
 * An incomplete program may be regarded as a program template from which
 * instance-specific concrete programs may be generated.
 *
 * A complete program is constructed by providing an MpuSymbolTable holding
 * concrete values for the unknowns.
 *
 *      MpuSymbolTable instance_syms;
 *      MpuProgram instance_prog = MpuProgram(prog, instance_symtab);
 *
 *      instance_prog.complete will be true if all unknowns were resolved.
 *
 * The text for a program is available as prog.text - a uint64_t vector.
 */
class MpuProgram {
public:
    MpuProgram();
    ~MpuProgram() {}

    /*
     * This constructor creates an MpuProgram from an existing one, filling
     * in unknown Relocs from the symbol table provided 'instab'.
     * The instab symbols should be of type MPUSYM_PARAM.
     * The resulting MpuProgram will be (valid && complete) if all parameters
     * have been resolved and the text is ready to run.
     */
    MpuProgram(const MpuProgram& orig, const MpuSymbolTable& instab);

    /*
     * Symbol Table and Relocation Table
     */
    MpuSymbolTable symtab;
    MpuRelocTable reloctab;

    /*
     * Program text
     * The program text a simple vector of opcodoes.
     */
    std::vector<uint64_t> text;

    /*
     * valid and complete
     * An MpuProgram is 'valid' if it has been correctly read from a file
     * and has been found to be structurally sound.
     *
     * An MpuProgram is 'complete' if it contains no unresolved instance
     * parameters (entries in the reloctab), and therefore represents a
     * fully intact and usable program.
     */
    bool valid;
    bool complete;

    /*
     * Load and save functions.
     * A return of 0 indicates success.  -1 indicates error, and a precise
     * error code is available in 'err', a string version of which is
     * available in 'errmap'.
     */
    mutable MpuObjError err;
    static std::map<MpuObjError, const std::string> errmap;
    int save_to_elf(const std::string& path) const;
    int load_from_elf(const std::string& path);
    int save_to_raw(const std::string& path) const;
    int load_from_raw(const std::string& path);

    // pretty print
    void dump(int indent = 0) const;

private:
    int seterror(MpuObjError e) const;
    int process_raw_file(FILE *fp);
};

#endif
