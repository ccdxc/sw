/*
 * Copyright (c) 2017, Pensando Systems Inc.
 */

/*
 * capri_loader.cc: Implementation of APIs for MPU program loader
 */

#include "capri_loader.h"
#include "capri_hbm.hpp"
#include <assert.h>
#include <lib_model_client.h>
#include <boost/unordered_map.hpp>

/* TODO: Declaring these as globals for now. Figure out usage and define
 *       these appropriately.
 */
boost::unordered_map<std::string, capri_loader_ctx_t *> loader_instances;


/**
 * read_programs: Read all the programs in a specified directory. For now 
 *                assumes only MPU programs reside in that directory.
 * 
 * @pathname: Fully specified path name of the directory which contains the
 *            MPU programs
 *
 * Return: 0 on success, < 0 on failure
 */
static int
read_programs(const char *handle,
              char *pathname)
{
    DIR *dir;
    struct dirent *ent;
    int i = 0;
    capri_loader_ctx_t *ctx;
    capri_program_info_t *program_info;

    /* Load context */
    ctx = loader_instances[handle];
    if (!ctx) {
        printf("Invalid handle\n");
        return -1;
    }
    program_info = ctx->program_info;

    if ((dir = opendir (pathname)) != NULL) {
        while ((ent = readdir (dir)) != NULL) {
            if (ent->d_name[0] != '.') {
                printf ("%s\n", ent->d_name);
                program_info[i].name = ent->d_name;
                i++;
                assert(i < MAX_PROGRAMS);
            }
        }
        closedir (dir);
    } else {
        printf("Cant open dir %s %s \n", pathname, strerror(errno));
        return -1;
    }
    return i;
}

/**
 * program_check: Check to see if a program (speficied by its name) is present
 *                in a hierarchical input based resolution of <program, param> 
 * 
 * @prog_param_info: Hierarchical data on which <program, param> to resolve
 * @num_prog_params: Number of elements in the prog_param_info array
 * @prog_name: Program name to check
 *
 * Return: Index of program on success, < 0 on failure
 */
static int
program_check(capri_prog_param_info_t *prog_param_info, int num_prog_params, 
              std::string prog_name)
{
    int i;

    if (!prog_param_info || num_prog_params == 0) {
        return -1;
    }

    for (i = 0; i < num_prog_params; i++) {
        if (prog_name == prog_param_info[i].name) {
            return i;
        }
    }
    return -1;
}

/**
 * param_check: Check to see if a param (speficied by its name) is present in
 *              in a input based resolution of the program's parameter list
 * 
 * @prog_param_info: Program's parameter list
 * @prog_name: Parameter name to check
 * @val: Pointer to where the value is to be stored
 *
 * Return: Index of parameter on success, < 0 on failure
 */
static int
param_check(capri_prog_param_info_t *prog_param_ptr, std::string param, uint64_t *val)
{
    int i;

    for (i = 0; i < prog_param_ptr->num_params; i++) {
         if (param == prog_param_ptr->params[i].name) {
             *val = prog_param_ptr->params[i].val;
             return i;
         }
    }
    return -1;
}

/**
 * capri_load_mpu_programs: Load all MPU programs in a given directory. Resolve 
 *                          the parameters defined in the programs using an 
 *                          input list + by checking against labels defined in 
 *                          the programs.  Finally write the programs to HBM 
 *                          memory.  
 *                          NOTE: For now the assumption is that directory 
 *                                specified by "pathname" will contain only MPU 
 *                                binaries.
 * 
 * @pathname: Fully specified path name of the directory which contains the
 *            MPU programs
 * @hbm_base_addr: Base address in HBM to use when loading the MPU programs
 * @prog_param_info: Hierarchical data on which <program, param> to resolve
 * @num_prog_params: Number of elements in the prog_param_info array
 *
 * Return: Index of program on success, < 0 on failure
 */
int
capri_load_mpu_programs(const char *handle,
                        char *pathname, uint64_t hbm_base_addr,
                        capri_prog_param_info_t *prog_param_info, 
                        int num_prog_params)
{
    int i, j, prog_index;
    capri_loader_ctx_t *ctx;
    capri_program_info_t *program_info;
    MpuSymbol *symbol, *param_u, *param_r;
    uint64_t val;
    bool rv;
    MpuSymbolTable global_labels;

    /* ISA library initialization */
    if (libcapisa_init() < 0) {
		printf("Libcapisa initialization failed! \n");
        return -1;
    }

    /* Input check */
    if (!handle || !pathname) {
        printf("Input error \n");
        return -1;
    }

    /* Create a loader instance */
    ctx = loader_instances[handle];
    if (ctx) {
        printf("Programs already loaded!");
        return -1;
    }

    /* Allocate context */
    ctx = new capri_loader_ctx_t;
    ctx->handle = handle;
    ctx->program_info = new capri_program_info_t[MAX_PROGRAMS];
    loader_instances[handle] = ctx;
    program_info = ctx->program_info;

    /* Read all program names */
    if ((ctx->num_programs = read_programs(handle, pathname)) < 0) {
        printf("Cant read programs \n");
        return -1;
    }
    printf("Num programs %d \n", ctx->num_programs);

    /* Other initializations */
    std::string path_str = pathname;
    ctx->prog_hbm_base_addr = (hbm_base_addr + 63) & 0xFFFFFFFFFFFFFFC0L;
    if ((ctx->prog_hbm_base_addr & 63) != 0) {
        printf("Invalid HBM base address\n");
        return -1;
    }

    /* Pass 1: Load all MPU programs into a data structure. Seperate the symbols
     *         in those MPU programs into 3 categories:
     *         1. Resolved labels (maintained in a global list also)
     *         2. Resolved parameters (resolved through the input list)
     *         3. Unresolved parameters
     */
    for (i = 0; i < ctx->num_programs; i++) {
        /* Load the program from the ELF file and check for errors */
        std::string filename = path_str+ "/" + program_info[i].name;
        if (program_info[i].prog.load_from_elf(filename) < 0) {
            printf("Error: %s : %s\n", 
               program_info[i].prog.errmap[program_info[i].prog.err].c_str(),
               filename.c_str());
            return -1;
        }

        /* Save the base address and size */
        program_info[i].base_addr = ctx->prog_hbm_base_addr;
        program_info[i].size = program_info[i].prog.text.size()*sizeof(uint64_t);
        /* Dump program specific info and the symbol table */
        printf("MPU Program file name %s loaded, valid %d, complete %d, "
               "number of symbols %lu, base address %lx, size %lu \n", 
               program_info[i].name.c_str(), 
               program_info[i].prog.valid, program_info[i].prog.complete, 
               program_info[i].prog.symtab.size(),
               program_info[i].base_addr, program_info[i].size);
        program_info[i].prog.symtab.dump();

        /* Check to see if the program that is loaded is present in the
         * list of programs for which parameters are specified via input
         */
        prog_index = program_check(prog_param_info, num_prog_params,
                                   program_info[i].name);

        /* Iterate through the program's symbol table */
        for (j = 0; j < (int) program_info[i].prog.symtab.size(); j++) {
            /* Get each symbol by its id */
            if ((symbol = program_info[i].prog.symtab.get_byid(j)) == NULL) {
                printf("Id: %d, Can't get symbol \n", j);
                return -1;
            } else {
                /* Symbol is a parameter */
                if (symbol->type == MPUSYM_PARAM) {
                    /* If <program, parameter> was passed in the input list =>
                     *    resolve the parameter's value based on the input.
                     * Else
                     *    add the parameter to the list of unresolved list
                     */
                    val = 0;
                    if (prog_index >= 0 &&
                        param_check(&prog_param_info[prog_index], symbol->name, 
                                    &val) >= 0) {
                        printf("resolved param: name %s val %lx\n",
                               symbol->name.c_str(), val);
                        program_info[i].resolved_params.add(
                            MpuSymbol(symbol->name.c_str(), MPUSYM_PARAM, val));
                    } else {
                        printf("unresolved param: name %s \n",
                               symbol->name.c_str());
                        program_info[i].unresolved_params.add(
                            MpuSymbol(symbol->name.c_str(), MPUSYM_PARAM, 0x0));
                    }
                /* Symbol is a label */
                } else if (symbol->type == MPUSYM_LABEL) {
                    /* Add it to the list of program specific and global labels 
                     * which will help resolve unknown parameters in pass 2
                     */
                    printf("label: name %s addr %lx \n", symbol->name.c_str(),
                           program_info[i].base_addr + symbol->val);
                    global_labels.add(
                             MpuSymbol(symbol->name.c_str(),
                                       symbol->type, 
                                       program_info[i].base_addr+ symbol->val));
                    program_info[i].labels.add(
                             MpuSymbol(symbol->name.c_str(),
                                       symbol->type, symbol->val));
                /* Symbol type not known */
                } else {
                    /* Other symbol types are not supported at the moment*/
                    printf("unknown symbol type %d \n", symbol->type);
                    return -1;
                }
            }
        }
        /* Increment the running counter of HBM base address */
        ctx->prog_hbm_base_addr += program_info[i].size;
        ctx->prog_hbm_base_addr = (ctx->prog_hbm_base_addr + 63) & 0xFFFFFFFFFFFFFFC0L;
    }

    /* Pass 2: Resolve all the unresolved parameters based on looking up the
     *         global labels
     */
    for (i = 0; i < ctx->num_programs; i++) {
        for (j = 0; j < (int) program_info[i].unresolved_params.size(); j++) {
            /* Get the unresolved parameter by id */
            if ((param_u = program_info[i].unresolved_params.get_byid(j))
                == NULL) {
                printf("Id: %d, Can't get unresolved param \n", j);
                return -1;
            } else {
                /* Try to resolve it by looking at the global labels */
                if ((param_r = global_labels.get_byname(param_u->name.c_str())) 
                    != NULL) {
                    printf("Resolved param %s to value %lu \n",
                           param_r->name.c_str(), param_r->val);

                    /* Add it to the list of resolved params for that program */
                    program_info[i].resolved_params.add(
                           MpuSymbol(param_u->name.c_str(), MPUSYM_PARAM,
                                     param_r->val));
                } else {
                    printf("Cannot resolve param %s \n", param_u->name.c_str());
                }
            }
        }
    }

    /* Pass 3: Build a copy of the program based on the resolved parameters and 
     *         write it to HBM 
     */
    for (i = 0; i < ctx->num_programs; i++) {
       program_info[i].copy = MpuProgram(program_info[i].prog,
                                         program_info[i].resolved_params);
       printf("Prog details: name %s, base address %lx, size %lu, valid %d, "
              "complete %d \n",
               program_info[i].name.c_str(), program_info[i].base_addr, 
               program_info[i].size, program_info[i].copy.valid,
               program_info[i].copy.complete);

       /* Sanity check the size of the copy, valid and complete flags */
       if (program_info[i].copy.valid != 1 || 
           program_info[i].copy.complete != 1 || 
           program_info[i].copy.text.size()*sizeof(uint64_t) != 
           program_info[i].size) {
           printf("valid = %d, complete = %d copy size %lu size %lu\n",
                   program_info[i].copy.valid, program_info[i].copy.complete,
                   program_info[i].copy.text.size(), program_info[i].size);
           printf("Can't load program: error in validation of size/flags!!!!!!!! \n");
           //return -1;
           continue;
       }
       
       rv = write_mem(program_info[i].base_addr,
                      (uint8_t *) program_info[i].copy.text.data(),
                      program_info[i].size);
       printf("yyy: loading at address 0x%lx, size %ld: \n", program_info[i].base_addr, program_info[i].size);
       int ii;
       for (ii = 0; ii < 16; ii++) {
           printf("0x%x ", (uint8_t)program_info[i].copy.text.data()[ii]);
       }
       printf("\n");
       if (rv != true) {
           printf("HBM P4 program write failed\n");
       }
    }

    return 0;
}

/**
 * capri_program_label_to_offset: Resolve a programs, label to its relative 
 *                                offset
 * 
 * @prog_name: Program name
 * @label_name: Label name
 * @offset: Offset to be filled in if the program, label is found
 *
 * Return: 0 on success, < 0 on failure
 */
int
capri_program_label_to_offset(const char *handle,
                              char *prog_name, char *label_name,
                              uint64_t *offset)
{
    std::string prog_name_str = prog_name;
    int i;
    MpuSymbol *label;
    capri_loader_ctx_t *ctx;
    capri_program_info_t *program_info;

    /* Input check */
    if (!prog_name || !label_name || !offset || !handle) {
        printf("Input error \n");
        return -1;
    }

    /* Load context */
    ctx = loader_instances[handle];
    if (!ctx) {
        printf("Invalid handle \n");
        return -1;
    }
    program_info = ctx->program_info;

    /* Iterate through the list of programs and its labels until the label is
     * found 
     */
    for (i = 0; i < ctx->num_programs; i++) {
        if (program_info[i].name == prog_name_str) {
            if ((label = program_info[i].labels.get_byname(label_name))
                 != NULL) {
                printf("Resolved program name %s label name %s to value %lu \n",
                       program_info[i].name.c_str(), label->name.c_str(), 
                       label->val);
                *offset = label->val;
                return 0;
            }
        }
    }

    printf("Could not resolve program name %s label name %s \n", 
            prog_name, label_name);
           //program_info[i].name.c_str(), label->name.c_str());
    return -1;
}

/**
 * capri_program_offset_to_label: Resolve a program, relative offset to a label
 * 
 * @prog_name: Program name
 * @offset: Offset value
 * @label_name: Pointer to the location where label name is to be filled
 * @label_size: Size allocated by caller for the label_name pointer
 *
 * Return: 0 on success, < 0 on failure
 */
int
capri_program_offset_to_label(const char *handle,
                              char *prog_name, uint64_t offset,
                              char *label_name, size_t label_size)
{
    std::string prog = prog_name;
    int i, j;
    MpuSymbol *label;
    capri_loader_ctx_t *ctx;
    capri_program_info_t *program_info;

    /* Input check */
    if (!prog_name || !label_name || !handle) {
        printf("Input error \n");
        return -1;
    }

    /* Load context */
    ctx = loader_instances[handle];
    if (!ctx) {
        printf("Invalid handle \n");
        return -1;
    }
    program_info = ctx->program_info;

    /* Iterate through the list of programs and all offset associated with its 
     * labels until the offset is found 
     */
    for (i = 0; i < ctx->num_programs; i++) {
        if (program_info[i].name == prog) {
            for (j = 0; j < (int) program_info[i].labels.size(); j++) {
                if ((label = program_info[i].labels.get_byid(j)) != NULL) {
                    if (offset == label->val) {
                        printf("Resolved program name %s offset %lu to "
                               "label name %s \n", program_info[i].name.c_str(),
                               offset, label->name.c_str());
                        strncpy(label_name, label->name.c_str(), label_size);
                        return 0;
                    }
                }
            }
        }
    }
    printf("Could not resolve program name %s offset %lu \n", 
           prog_name, offset);
          //program_info[i].name.c_str(), offset);
    return -1;
}

/**
 * capri_program_to_base_addr: Resolve a program to its base address in HBM
 * 
 * @prog_name: Program name
 * @base_addr: Pointer to the location where base address is to be filled
 *
 * Return: 0 on success, < 0 on failure
 */
int
capri_program_to_base_addr(const char *handle,
                           char *prog_name, uint64_t *base_addr)
{
    std::string prog = prog_name;
    int i;
    capri_loader_ctx_t *ctx;
    capri_program_info_t *program_info;

    /* Input check */
    if (!prog_name || !base_addr || !handle) {
        printf("Input error \n");
        return -1;
    }

    /* Load context */
    ctx = loader_instances[handle];
    if (!ctx) {
        printf("Invalid handle\n");
        return -1;
    }
    program_info = ctx->program_info;

    /* Iterate through the list of programs and if found, return its 
     * base address
     */
    for (i = 0; i < ctx->num_programs; i++) {
        if (program_info[i].name == prog) {
            printf("Resolved program name %s to base_addr %lx \n",
                   program_info[i].name.c_str(), program_info[i].base_addr);
            *base_addr = program_info[i].base_addr;
            return 0;
        }
    }
    printf("Could not resolve program name %s \n",
           prog_name);
           //program_info[i].name.c_str());
    return -1;
}

