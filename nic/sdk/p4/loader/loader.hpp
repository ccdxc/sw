/*
 * Copyright (c) 2017, Pensando Systems Inc.
 */

/*
 * loader.hpp: API definitions for invoking MPU program loader
 */

#ifndef __SDK_P4_LOADER_HPP__
#define __SDK_P4_LOADER_HPP__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <string>
#include <vector>
#include <map>
#include <iostream>
#include <fstream>
#include <dirent.h>

#include "include/sdk/platform.hpp"
#include "include/sdk/types.hpp"
#include "include/sdk/base.hpp"

namespace sdk {
namespace p4 {

#define LDD_INFO_FILE_RPATH    "gen" //Relative path from main
#define LDD_INFO_FILE_NAME    "mpu_prog_info.json"

/* Definition of maximum number of programs and parameters.
 * TODO: Remove these artificial limits.
 */
// #define MAX_PROGRAMS	512
#define MAX_PARAMS	512

/* P4 loader's initialization parameter structure */
typedef struct {
    std::string name;
    uint64_t val;
} p4_param_info_t;

typedef void (*mpu_pgm_sort_t)(std::vector <std::string> &);
typedef uint32_t (*mpu_pgm_symbols_t)(void **, platform_type_t);

/**
 * p4_load_mpu_programs: Load all MPU programs in a given directory. Resolve
 *                          the parameters defined in the programs using an
 *                          input list + by checking against labels defined in
 *                          the programs.  Finally write the programs to HBM
 *                          memory.
 *                          NOTE: For now the assumption is that directory
 *                                specified by "pathname" will contain only MPU
 *                                binaries.
 *
 * @handle: Handle for loader context
 * @pathname: Fully specified path name of the directory which contains the
 *            MPU programs
 * @hbm_base_addr: Base address in HBM to use when loading the MPU programs
 * @prog_param_info: Hierarchical data on which <program, param> to resolve
 * @num_prog_params: Number of elements in the prog_param_info array
 *
 * Return: sdk_ret_t
 */
sdk_ret_t p4_load_mpu_programs(const char *handle,
                            char *pathname, uint64_t hbm_base_addr,
                            p4_param_info_t *prog_param_info,
                            int num_prog_params, mpu_pgm_sort_t sort_func,
                            bool is_slave);

/**
 * p4_program_label_to_offset: Resolve a programs, label to its relative
 *                                offset
 *
 * @handle: Handle for loader context
 * @prog_name: Program name
 * @label_name: Label name
 * @offset: Offset to be filled in if the program, label is found
 *
 * Return: sdk_ret_t
 */
sdk_ret_t p4_program_label_to_offset(const char *handle,
                                  char *prog_name, char *label_name,
                                  uint64_t *offset);
/**
 * p4_program_offset_to_label: Resolve a program, relative offset to a label
 *
 * @handle: Handle for the loader context
 * @prog_name: Program name
 * @offset: Offset value
 * @label_name: Pointer to the location where label name is to be filled
 * @label_size: Size allocated by caller for the label_name pointer
 *
 * Return: sdk_ret_t
 */
sdk_ret_t p4_program_offset_to_label(const char *handle,
                                  char *prog_name, uint64_t offset,
                                  char *label_name, size_t label_size);

/**
 * p4_program_to_base_addr: Resolve a program to its base address in HBM
 *
 * @handle: Handle for the loader context
 * @prog_name: Program name
 * @base_addr: Pointer to the location where base address is to be filled
 *
 * Return: sdk_ret_t
 */
sdk_ret_t p4_program_to_base_addr(const char *handle,
                               char *prog_name, uint64_t *base_addr);

/**
 * p4_dump_program_info: Save the MPU program information to mpu_prog_info.json
 *
 * @cfg_path: Config path
 *
 * Return sdk_ret_t
 */
sdk_ret_t p4_dump_program_info (const char *cfg_path);

/**
 * p4_cleanup: Cleanup the memories and variables allocated/used during init
 */
void p4_cleanup(void);

}    // namespace p4
}    // namnespace sdk

using sdk::p4::mpu_pgm_sort_t;
using sdk::p4::mpu_pgm_symbols_t;

#endif   // _SDK_P4_LOADER_HPP_
