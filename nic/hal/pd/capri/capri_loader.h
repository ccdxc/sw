/*
 * Copyright (c) 2017, Pensando Systems Inc.
 */

/*
 * capri_loader.h: API definitions for invoking MPU program loader
 */

#ifndef _CAPRI_LOADER_H_
#define _CAPRI_LOADER_H_

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
#include <libcapisa.h>
#include "libmpuobj.h"

/* Definition of maximum number of programs and parameters. 
 * TODO: Remove these artificial limits.
 */
#define MAX_PROGRAMS	64
#define MAX_PARAMS	64

/* Capri loader's MPU program information structure */
typedef struct {
    std::string name;
    uint64_t base_addr;
    uint64_t size;
    MpuProgram prog;
    MpuProgram copy;
    MpuSymbolTable unresolved_params;
    MpuSymbolTable resolved_params;
    MpuSymbolTable labels;
} capri_program_info_t;


/* Capri loader's initialization parameter structure */
typedef struct {
    std::string name;
    uint64_t val;
} capri_param_info_t;

/* Capri loader's initialization program structure for passing an array
 * of parameters. Multiple <programs, parameters> could be resolved by passing 
 * an array of this structure during initialization.
 */
typedef struct {
    std::string name;
    int num_params;
    capri_param_info_t params[MAX_PARAMS];
} capri_prog_param_info_t;


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
int capri_load_mpu_programs(char *pathname, uint64_t hbm_base_addr, 
                            capri_prog_param_info_t *prog_param_info, 
                            int num_prog_params);

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
int capri_program_label_to_offset(char *prog_name, char *label_name,
                                  uint64_t *offset);
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
int capri_program_offset_to_label(char *prog_name, uint64_t offset,
                                  char *label_name, size_t label_size);

/**
 * capri_program_to_base_addr: Resolve a program to its base address in HBM
 * 
 * @prog_name: Program name
 * @base_addr: Pointer to the location where base address is to be filled
 *
 * Return: 0 on success, < 0 on failure
 */
int capri_program_to_base_addr(char *prog_name, uint64_t *base_addr);

#endif   // _CAPRI_LOADER_H_
