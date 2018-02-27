/*
 * Copyright (c) 2017, Pensando Systems Inc.
 */

/*
 * capri_loader.cc: Implementation of APIs for MPU program loader
 */

#include "nic/hal/pd/capri/capri_loader.h"
#include "nic/hal/pd/capri/capri_hbm.hpp"
#include <assert.h>
#include "nic/include/asic_pd.hpp"
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
read_programs(const char *handle, char *pathname)
{
    DIR *dir;
    struct dirent *ent;
    int i = 0;
    capri_loader_ctx_t *ctx;
    capri_program_info_t *program_info;

    /* Load context */
    ctx = loader_instances[handle];
    if (!ctx) {
        HAL_TRACE_ERR("Invalid handle");
        return -1;
    }
    program_info = ctx->program_info;

    if ((dir = opendir (pathname)) != NULL) {
        while ((ent = readdir (dir)) != NULL) {
            if (ent->d_name[0] != '.') {
                program_info[i].name = ent->d_name;
                i++;
                assert(i < MAX_PROGRAMS);
            }
        }
        closedir (dir);
    } else {
        HAL_TRACE_ERR("Cannot open dir {} {}", pathname, strerror(errno));
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
capri_load_mpu_programs (const char *handle,
                         char *pathname, uint64_t hbm_base_addr,
                         capri_prog_param_info_t *prog_param_info,
                         int num_prog_params)
{
    int i, j, prog_index;
    capri_loader_ctx_t *ctx;
    capri_program_info_t *program_info;
    MpuSymbol *symbol, *param_u, *param_r;
    uint64_t val;
    hal_ret_t rv;
    MpuSymbolTable global_labels;

    /* ISA library initialization */
    if (libcapisa_init() < 0) {
	    HAL_TRACE_ERR("Libcapisa initialization failed!");
        HAL_ASSERT_RETURN(0, HAL_RET_ERR);
    }

    /* Input check */
    if (!handle || !pathname) {
        HAL_TRACE_ERR("Input error");
        HAL_ASSERT_RETURN(0, HAL_RET_ERR);
    }

    /* Create a loader instance */
    ctx = loader_instances[handle];
    if (ctx) {
        HAL_TRACE_ERR("Programs already loaded!");
        HAL_ASSERT_RETURN(0, HAL_RET_ERR);
    }

    /* Allocate context */
    ctx = new capri_loader_ctx_t;
    ctx->handle = handle;
    ctx->program_info = new capri_program_info_t[MAX_PROGRAMS];
    loader_instances[handle] = ctx;
    program_info = ctx->program_info;

    /* Read all program names */
    if ((ctx->num_programs = read_programs(handle, pathname)) < 0) {
        HAL_TRACE_ERR("Cannot read programs");
        HAL_ASSERT_RETURN(0, HAL_RET_ERR);
    }
    HAL_TRACE_DEBUG("Num programs {}", ctx->num_programs);

    /* Other initializations */
    std::string path_str = pathname;
    ctx->prog_hbm_base_addr = (hbm_base_addr + 63) & 0xFFFFFFFFFFFFFFC0L;
    if ((ctx->prog_hbm_base_addr & 63) != 0) {
        HAL_TRACE_ERR("Invalid HBM base address");
        HAL_ASSERT_RETURN(0, HAL_RET_ERR);
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
            HAL_TRACE_ERR("Error: {} : {}",
               program_info[i].prog.errmap[program_info[i].prog.err].c_str(),
               filename.c_str());
            HAL_ASSERT_RETURN(0, HAL_RET_ERR);
        }

        /* Save the base address and size */
        program_info[i].base_addr = ctx->prog_hbm_base_addr;
        program_info[i].size = program_info[i].prog.text.size()*sizeof(uint64_t);
        /* Dump program specific info and the symbol table */
        HAL_TRACE_DEBUG("MPU Program file name {} loaded, valid {}, "
                        "complete {}, number of symbols {}, "
                        "base address {:#x}, size {}",
                        program_info[i].name.c_str(),
                        program_info[i].prog.valid,
                        program_info[i].prog.complete,
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
                HAL_TRACE_ERR("Id: {}, Cannot get symbol", j);
                HAL_ASSERT_RETURN(0, HAL_RET_ERR);
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
                        prog_index < num_prog_params &&
                        prog_param_info &&
                        param_check(&prog_param_info[prog_index], symbol->name,
                                    &val) >= 0) {
                        HAL_TRACE_DEBUG("resolved param: name {} val {:#x}",
                                        symbol->name.c_str(), val);
                        program_info[i].resolved_params.add(
                            MpuSymbol(symbol->name.c_str(), MPUSYM_PARAM, val));
                    } else {
                        HAL_TRACE_DEBUG("unresolved param: name {}",
                                        symbol->name.c_str());
                        program_info[i].unresolved_params.add(
                            MpuSymbol(symbol->name.c_str(), MPUSYM_PARAM, 0x0));
                    }
                /* Symbol is a label */
                } else if (symbol->type == MPUSYM_LABEL) {
                    /* Add it to the list of program specific and global labels
                     * which will help resolve unknown parameters in pass 2
                     */
                    HAL_TRACE_DEBUG("label: name {} addr {:#x}",
                                    symbol->name.c_str(),
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
                    HAL_TRACE_ERR("unknown symbol type {}", symbol->type);
                    HAL_ASSERT_RETURN(0, HAL_RET_ERR);
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
                HAL_TRACE_ERR("Id: {}, Cannot get unresolved param", j);
                HAL_ASSERT_RETURN(0, HAL_RET_ERR);
            } else {
                /* Try to resolve it by looking at the global labels */
                if ((param_r = global_labels.get_byname(param_u->name.c_str()))
                    != NULL) {
                    HAL_TRACE_DEBUG("Resolved param {} to value {:#x}",
                                    param_r->name.c_str(), param_r->val);

                    /* Add it to the list of resolved params for that program */
                    program_info[i].resolved_params.add(
                           MpuSymbol(param_u->name.c_str(), MPUSYM_PARAM,
                                     param_r->val));
                } else {
                    HAL_TRACE_ERR("Cannot resolve param {}",
                                  param_u->name.c_str());
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
       /* Sanity check the size of the copy, valid and complete flags */
       if (program_info[i].copy.valid != 1 ||
           program_info[i].copy.complete != 1 ||
           program_info[i].copy.text.size()*sizeof(uint64_t) !=
           program_info[i].size) {
           HAL_TRACE_DEBUG("Failed to resolve program: name {}, "
                           "base address {:#x}, size {}, valid {}, complete {}",
                           program_info[i].name.c_str(),
                           program_info[i].base_addr,
                           program_info[i].size, 
                           program_info[i].copy.valid,
                           program_info[i].copy.complete);

           HAL_TRACE_DEBUG("MPU symbol table: ");
           program_info[i].copy.symtab.dump();
           HAL_TRACE_DEBUG("MPU reloctab: ");
           program_info[i].copy.reloctab.dump();

           HAL_ASSERT_RETURN(0, HAL_RET_ERR);
       } else {
           HAL_TRACE_DEBUG("Successfully resolved program: name {}, "
                           "base address {:#x}, size {}, valid {}, complete {}",
                           program_info[i].name.c_str(),
                           program_info[i].base_addr,
                           program_info[i].size, 
                           program_info[i].copy.valid,
                           program_info[i].copy.complete);
       }

       /* Write program to HBM */
       rv = hal::pd::asic_mem_write(program_info[i].base_addr,
                                    (uint8_t *) program_info[i].copy.text.data(),
                                    program_info[i].size);
       if (rv != HAL_RET_OK) {
           HAL_TRACE_ERR("HBM program write failed");
           HAL_ASSERT_RETURN(0, HAL_RET_ERR);
       }
    }

    return HAL_RET_OK;
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
        HAL_TRACE_ERR("Input error");
        return -1;
    }

    /* Load context */
    ctx = loader_instances[handle];
    if (!ctx) {
        HAL_TRACE_ERR("Invalid handle");
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
                HAL_TRACE_DEBUG("Resolved program name {} label name {} to "
                                "value {:#x}", program_info[i].name.c_str(),
                                label->name.c_str(), label->val);
                *offset = label->val;
                return 0;
            }
        }
    }

    HAL_TRACE_ERR("Could not resolve program name {} label name {}",
                  prog_name, label_name);
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
        HAL_TRACE_ERR("Input error");
        return -1;
    }

    /* Load context */
    ctx = loader_instances[handle];
    if (!ctx) {
        HAL_TRACE_ERR("Invalid handle");
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
                        HAL_TRACE_DEBUG("Resolved program name {} offset {} to "
                                        "label name {}",
                                        program_info[i].name.c_str(),
                                        offset, label->name.c_str());
                        strncpy(label_name, label->name.c_str(), label_size);
                        return 0;
                    }
                }
            }
        }
    }
    HAL_TRACE_ERR("Could not resolve program name {} offset {}",
                  prog_name, offset);
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
        HAL_TRACE_ERR("Input error");
        return -1;
    }

    /* Load context */
    ctx = loader_instances[handle];
    if (!ctx) {
        HAL_TRACE_ERR("Invalid handle");
        return -1;
    }
    program_info = ctx->program_info;

    /* Iterate through the list of programs and if found, return its
     * base address
     */
    for (i = 0; i < ctx->num_programs; i++) {
        if (program_info[i].name == prog) {
            HAL_TRACE_DEBUG("Resolved program name {} to base_addr {:#x}",
                            program_info[i].name.c_str(),
                            program_info[i].base_addr);
            *base_addr = program_info[i].base_addr;
            return 0;
        }
    }
    HAL_TRACE_ERR("Could not resolve program name {}", prog_name);
    return -1;
}

/**
 * capri_list_program_addr: List each program's name, start address and end address
 *
 * @filename: The filename where the output is to be stored
 *
 * Return: 0 on success, < 0 on failure
 */
int
capri_list_program_addr(const char *filename) 
{
    capri_loader_ctx_t *ctx;
    capri_program_info_t *program_info;
    FILE *fp = NULL;

    /* Input check  */
    if ((!filename) || (!(fp = fopen(filename, "w+")))) {
        HAL_TRACE_ERR("Cannot open input file for listing program addresses");
        return -1;
    }

    /* Iterate through the loader instances, programs and list the valid ones */
    for (auto it = loader_instances.begin(); it != loader_instances.end(); it++) {
        if ((ctx = loader_instances[it->first]) != NULL) {
            HAL_TRACE_DEBUG("Listing programs for handle name {}", it->first);
            program_info = ctx->program_info;
            for (int i = 0; i < ctx->num_programs; i++) {
                fprintf(fp, "%s,%lx,%lx\n", program_info[i].name.c_str(),
                        program_info[i].base_addr,
                        ((program_info[i].base_addr + 
                          program_info[i].size + 63) & 0xFFFFFFFFFFFFFFC0L) - 1);
                fflush(fp);
            }
        } else {
            HAL_TRACE_DEBUG("Cannot listing programs for handle name {}", 
                            it->first);
        }
    }
  return 0;
}
