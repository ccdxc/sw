#!/usr/local/bin/python
#
# Capri-Non-Compiler-Compiler (capri-ncc)

import argparse
import os
import sys
import re
import string
import pdb, json
import logging
from collections import OrderedDict
from enum import IntEnum
from tenjin import *
from tenjin_wrapper import *

tenjin_prefix = "//::"

def p4pd_generate_code(pd_dict, template_dir, output_h_dir, output_c_dir, cli_outputdir_map, prog_name, gen_dir, pipeline):

    #print(template_dir, output_h_dir, output_c_dir, prog_name, gen_dir, pipeline)
    if output_h_dir and not os.path.exists(output_h_dir):
        try:
            os.makedirs(output_h_dir)
        except OSError as e:
            if e.errno != errno.EEXIST:
                raise
    if output_c_dir and not os.path.exists(output_c_dir):
        try:
            os.makedirs(output_c_dir)
        except OSError as e:
            if e.errno != errno.EEXIST:
                raise

    if cli_outputdir_map is not None:
        for output_dir in cli_outputdir_map.values():
            if output_dir and not os.path.exists(output_dir):
                try:
                    os.makedirs(output_dir)
                except OSError as e:
                    if e.errno != errno.EEXIST:
                        raise
    cli_name = pd_dict['cli-name']
    templates_outfiles = make_templates_outfiles(template_dir, output_h_dir,
                                                 output_c_dir,
                                                 cli_outputdir_map, prog_name,
                                                 cli_name, pipeline)
    _prog_name = ''
    if prog_name != '':
        prog_name = prog_name + '_'
        _prog_name = '_' + prog_name
    for templatefile, outfile in templates_outfiles:
        outputfile_path = os.path.dirname(outfile)
        pdd = {}
        with open(outfile, "w") as of:
            pdd['pddict'] = pd_dict
            p4tbl_types = render_template(of, templatefile, pdd, os.path.dirname(templatefile), \
                                          prefix=tenjin_prefix)
            of.close()
            if p4tbl_types:
                outfile = output_h_dir + '/' + prog_name + 'p4pd_table.h'
                with open(outfile, "w") as of:
                    file_prologue =  \
                            '/* ' + prog_name + 'p4pd_table.h\n'+\
                            ' * Pensando Systems\n'  +\
                            ' */\n' + \
                            '/*\n'  + \
                            ' * This file is generated from P4 program. Any changes made to this file will\n' + \
                            ' * be lost.\n' + \
                            ' */\n\n' + \
                            '#ifndef __' + prog_name.upper() + 'P4PD_TABLE_H__\n' + \
                            '#define __' + prog_name.upper() + 'P4PD_TABLE_H__\n'
                    of.write(file_prologue)
                    of.write('\n')
                    of.write('\n')
                    code_str = 'typedef enum '+ prog_name + 'p4pd_table_range_ {\n'
                    of.write(code_str)
                    for k, v in p4tbl_types.items():
                        code_str =  '    ' + k + ' = ' + str(v) + ','  + '\n'
                        of.write(code_str)
                    code_str = '} ' + prog_name + 'p4pd_table_range_en;\n\n'
                    of.write(code_str)
                    ####
                    code_str = '#endif\n\n'
                    of.write(code_str)
                    of.close()


def p4pd_generate_asm_code(pd_dict, template_dir, output_h_dir, output_c_dir, cli_outputdir_map, prog_name, pipeline):

    if output_h_dir and not os.path.exists(output_h_dir):
        try:
            os.mkdir(output_h_dir)
        except OSError as e:
            if e.errno != errno.EEXIST:
                raise
    if output_c_dir and not os.path.exists(output_c_dir):
        try:
            os.mkdir(output_c_dir)
        except OSError as e:
            if e.errno != errno.EEXIST:
                raise

    if cli_outputdir_map is not None:
        for output_dir in cli_outputdir_map.values():
            if output_dir and not os.path.exists(output_dir):
                try:
                    os.mkdir(output_dir)
                except OSError as e:
                    if e.errno != errno.EEXIST:
                        raise

    templates_outfiles = make_templates_outfiles(template_dir, output_h_dir, output_c_dir, cli_outputdir_map, prog_name, pd_dict['cli-name'], pipeline)
    kd_json = {}
    for templatefile, outfile in templates_outfiles:
        outputfile_path = os.path.dirname(outfile)
        pdd = {}
        with open(outfile, "w") as of:
            pdd['pddict'] = pd_dict
            kd_dict = render_template(of, templatefile, pdd, os.path.dirname(templatefile), \
                            prefix=tenjin_prefix)
            for k, v in kd_dict.items():
                kd_json[k] = v
            of.close()
    return kd_json

def capri_p4pd_create_swig_makefile_click(be):
    name = be.prog_name
    out_dir = be.args.gen_dir + '/%s/cli/' % (name)

    if not os.path.exists(out_dir):
        try:
            os.makedirs(out_dir)
        except OSError as e:
            if e.errno != errno.EEXIST:
                raise

    top_dir = capri_get_top_level_path(out_dir)

    content_str = '# This Makefile is auto-generated. Changes will be overwritten!\n'
    content_str += 'SW_DIR         =  ../' + top_dir + '\n'
    content_str += 'NIC_DIR        =  $(SW_DIR)/nic\n'
    content_str += 'TOOLCHAIN_ROOT =  /tool/toolchain/aarch64\n'
    content_str += 'TOOLCHAIN_VER  =  1.1\n'
    content_str += 'TOOLCHAIN_PATH =  $(TOOLCHAIN_ROOT)-$(TOOLCHAIN_VER)\n'
    content_str += '\n'
    content_str += 'ifneq ($(ARCH),aarch64)\n'
    content_str += '    ARCH = x86_64\n'
    content_str += 'endif\n'
    content_str += '\n'
    content_str += 'ifeq ($(ARCH),aarch64)\n'
    content_str += 'CXX            =  aarch64-linux-gnu-g++\n'
    content_str += 'else\n'
    content_str += 'CXX            =  g++\n'
    content_str += 'endif\n'
    content_str += '\n'
    content_str += 'CPPFLAGS       =  -shared -fPIC\n'
    content_str += '\n'
    content_str += 'INC_DIRS       =  -I$(SW_DIR)\n'
    content_str += 'INC_DIRS       += -I$(NIC_DIR)/hal/third-party/grpc/include\n'
    content_str += 'INC_DIRS       += -I$(NIC_DIR)/hal/third-party/google/include\n'
    content_str += 'INC_DIRS	   += -I/usr/include/python3.6m\n'
    content_str += 'INC_DIRS	   += -I/usr/include/python3.4m\n'
    content_str += 'ifeq ($(ARCH),aarch64)\n'
    content_str += 'INC_DIRS       += -I$(TOOLCHAIN_PATH)/aarch64-linux-gnu/usr/include\n'
    content_str += 'endif\n'
    content_str += '\n'
    content_str += 'ARCHIVES       =  -Wl,--allow-multiple-definition\n'
    content_str += 'ARCHIVES       += -Wl,--whole-archive $(NIC_DIR)/hal/third-party/grpc/$(ARCH)/lib/libgrpc*.a -Wl,--no-whole-archive\n'
    content_str += '\n'
    content_str += 'SHARED_LIBS    =  $(NIC_DIR)/hal/third-party/google/$(ARCH)/lib/libprotobuf.so.14\n'
    content_str += 'ifeq ($(ARCH),aarch64)\n'
    content_str += 'SHARED_LIBS    += -L$(TOOLCHAIN_PATH)/aarch64-linux-gnu/usr/lib\n'
    content_str += 'SHARED_LIBS    += -L$(NIC_DIR)/gen/aarch64/lib\n'
    content_str += 'else\n'
    content_str += 'SHARED_LIBS    += -L$(NIC_DIR)/gen/x86_64/lib\n'
    content_str += 'endif\n'
    content_str += 'SHARED_LIBS    += -lhalproto\n'
    content_str += '\n'
    content_str += 'default: ' + name + '\n'
    content_str += '\n'
    content_str += 'swig:\n'
    content_str += '\tswig -c++ -python -I$(NIC_DIR)/gen/'+ name + '/include -I$(NIC_DIR)/hal/pd -o ' + name + '_wrap.cc ' +  name + '.i\n'
    content_str += '\n'
    content_str += 'iris: swig\n'
    content_str += '\t$(CXX) $(CPPFLAGS) $(INC_DIRS) -o _iris.so iris_wrap.cc $(NIC_DIR)/build/iris/gen/datapath/p4/src/p4pd_debug.cc $(ARCHIVES) $(SHARED_LIBS)\n'
    content_str += '\n'
    content_str += 'clean:\n'
    content_str += '\trm -f _iris.so iris_wrap.cc iris.py\n'

    out_file = out_dir + 'Makefile.click'
    with open(out_file, "w") as of:
        of.write(content_str)
        of.close()

def capri_p4pd_create_swig_makefile(be):

    name = be.prog_name
    out_dir = be.args.gen_dir + '/%s/cli/' % (name)

    if not os.path.exists(out_dir):
        try:
            os.makedirs(out_dir)
        except OSError as e:
            if e.errno != errno.EEXIST:
                raise

    top_dir = capri_get_top_level_path(out_dir)

    content_str = '# This Makefile is auto-generated. Changes will be overwritten!\n'
    content_str += 'TOP_DIR = ' + top_dir + '\n'
    content_str += 'CXX = g++\n'
    content_str += 'SWG = swig\n'
    content_str += 'SWGFLAGS = -python -c++\n'
    content_str += 'CPPFLAGS = -g -c -pthread -std=c++11 -fPIC\n'
    content_str += 'LNKFLAGS = -Wl,-rpath,./obj -Wl,-rpath,./sdk/model_sim/build -Wl,-rpath,./sdk/model_sim/libs -Wl,-rpath,./asic/capri/model/capsim-gen/lib\n'
    content_str += 'LNKFLAGS += -Wl,-rpath,$(TOP_DIR)/obj -Wl,-rpath,$(TOP_DIR)/sdk/model_sim/build -Wl,-rpath,$(TOP_DIR)/sdk/model_sim/libs -shared\n'
    content_str += 'BLDFLAGS = -Wl,-rpath,./obj -Wl,-rpath,./sdk/model_sim/build -Wl,-rpath,./sdk/model_sim/libs -Wl,-rpath,./asic/capri/model/capsim-gen/lib\n'
    content_str += 'BLDFLAGS += -Wl,-rpath,$(TOP_DIR)/obj -Wl,-rpath,$(TOP_DIR)/sdk/model_sim/build -Wl,-rpath,$(TOP_DIR)/sdk/model_sim/libs\n'
    content_str += 'INC_DIRS = -I$(TOP_DIR)/include\n'
    content_str += 'INC_DIRS += -I$(TOP_DIR)/third-party/spdlog/include\n'
    content_str += 'INC_DIRS += -I$(TOP_DIR)/build/iris/gen/datapath/p4/include\n'
    content_str += 'INC_DIRS += -I$(TOP_DIR)/gen/common_rxdma_actions/include\n'
    content_str += 'INC_DIRS += -I$(TOP_DIR)/gen/common_txdma_actions/include\n'
    content_str += 'INC_DIRS += -I$(TOP_DIR)/hal/pd -I$(TOP_DIR)/sdk/model_sim/include\n'
    content_str += 'INC_DIRS += -I/usr/include/python2.7\n'
    content_str += 'LIB_DIRS = -L$(TOP_DIR)/obj -L$(TOP_DIR)/sdk/model_sim/build -L$(TOP_DIR)/sdk/model_sim/libs\n'
    content_str += 'LIBS = -lpython2.7 -lmodelclient -lzmq -lcapri -lcapricsr -lp4pluspd_rxdma -lp4pluspd_txdma -lp4pd -ltrace\n'
    content_str += '\n'
    content_str += 'all:\n'
    content_str += '\t$(SWG) $(SWGFLAGS) $(INC_DIRS) -o %s_wrap.c %s.i\n' % (name, name)
    content_str += '\t$(CXX) $(CPPFLAGS) $(INC_DIRS) -o %s_wrap.o %s_wrap.c\n' % (name, name)
    content_str += '\t$(CXX) $(CPPFLAGS) $(INC_DIRS) -o %s_api.o $(TOP_DIR)/hal/pd/iris/p4pd/p4pd_api.cc\n' % (name)
    content_str += '\t$(CXX) $(CPPFLAGS) $(INC_DIRS) -o global_api.o $(TOP_DIR)/hal/pd/iris/p4pd/p4pd_global_api.cc\n'
    content_str += '\t$(CXX) $(CPPFLAGS) $(INC_DIRS) -o rxdma_api.o $(TOP_DIR)/hal/pd/iris/p4pd/p4pluspd_rxdma_api.cc\n'
    content_str += '\t$(CXX) $(CPPFLAGS) $(INC_DIRS) -o txdma_api.o $(TOP_DIR)/hal/pd/iris/p4pd/p4pluspd_txdma_api.cc\n'
    content_str += '\t$(CXX) $(LNKFLAGS) -o _%s.so %s_wrap.o %s_api.o rxdma_api.o txdma_api.o global_api.o $(LIB_DIRS) $(LIBS)\n' % (name, name, name)
    content_str += '\n'
    content_str += 'main:\n'
    content_str += '\t$(CXX) $(CPPFLAGS) -DP4PD_CLI $(INC_DIRS) -o main.o main.cc\n'
    content_str += '\t$(CXX) $(BLDFLAGS) -o main main.o %s_api.o rxdma_api.o txdma_api.o global_api.o $(LIB_DIRS) $(LIBS)\n' % (name)
    content_str += '\n'
    content_str += 'clean:\n'
    content_str += '\trm *.o *.so *.pyc main %s.py %s_wrap.c\n' % (name, name)

    out_file = out_dir + 'Makefile'
    with open(out_file, "w") as of:
        of.write(content_str)
        of.close()

def capri_p4pd_create_swig_custom_hdr(be):

    name = be.prog_name
    if name in ['apollo', 'artemis', 'apulu', 'athena']:
        name = 'p4'
    out_dir = be.args.gen_dir + '/%s/cli/' % (name)

    if not os.path.exists(out_dir):
        try:
            os.makedirs(out_dir)
        except OSError as e:
            if e.errno != errno.EEXIST:
                raise

    if be.args.p4_plus:
        api_prefix = 'p4pd_' + name
    else:
        api_prefix = 'p4pd'

    content_str = """

/* populates the rsp_msg struct with the response obatined from HAL */
p4pd_error_t
""" + api_prefix + """_entry_read(uint32_t  tableid,
                uint32_t  index,
                void      *swkey,
                void      *swkey_mask,
                void      *actiondata,
                void      *rsp_msg,
                int       *size);

p4pd_error_t
""" + api_prefix + """_entry_populate(uint32_t  tableid,
                    void      *swkey,
                    void      *swkey_mask,
                    void      *actiondata,
                    void      *rsp_msg,
                    int       response_index);

void*
allocate_debug_response_msg (void);

void
free_debug_response_msg (void*);

p4pd_error_t
""" + api_prefix + """_entry_write(uint32_t tableid,
                 uint32_t index,
                 void     *swkey,
                 void     *swkey_mask,
                 void     *actiondata);

void
""" + api_prefix + """_register_entry_read(std::string blockname,
                        std::string regname,
                        std::string filename);

void
""" + api_prefix + """_register_list(std::string blockname,
                   std::string regname,
                   std::string filename);
"""

    if be.args.p4_plus:
        content_str += """
p4pd_error_t
""" + api_prefix + """_raw_table_entry_read(uint32_t tableid,
                uint8_t     actionid,
                void        *actiondata,
                uint64_t    address);

void
""" + api_prefix + """_raw_table_entry_write(uint32_t tableid,
                uint8_t     actionid,
                void        *actiondata,
                uint16_t    actiondata_sz,
                uint64_t    address);

"""

    out_file = out_dir + name + '_custom.h'
    with open(out_file, "w") as of:
        of.write(content_str)
        of.close()

def capri_p4pd_create_swig_interface(be):

    name = be.prog_name
    if name in ['apollo', 'artemis', 'apulu', 'athena']:
        name = 'p4'
    out_dir = be.args.gen_dir + '/%s/cli/' % (name)

    if not os.path.exists(out_dir):
        try:
            os.makedirs(out_dir)
        except OSError as e:
            if e.errno != errno.EEXIST:
                raise

    if be.args.p4_plus:
        prefix = 'p4pd_' + name
        caps_p4prog = '_' + name.upper() + '_'
        hdr_name = name + '_'
    else:
        prefix = 'p4pd'
        caps_p4prog = ''
        hdr_name = ''
    dir_str = be.args.gen_dir.split("/gen")[1]
    p4pd_cli_swig_dir = "gen/p4gen" + "/" + name + "/include/" + hdr_name
    p4pd_cust_dir = "gen/p4gen" + "/" + name + "/cli/"

    #if be.args.pipeline != None:
    #    p4pd_cli_swig_dir = "nic/build/%s/gen/datapath/%s/include/%s" %\
    #                         (be.args.pipeline, name, hdr_name)
    # else:
    #    p4pd_cli_swig_dir = "nic/gen/%s/include/%s" % (name, hdr_name) 
    module_name = be.args.pipeline + '_lib' + be.prog_name + '_p4pdcli'
    content_str = \
"""/* This file is auto-generated. Changes will be overwritten! */
/* %s.i */""" %(name) + """
%module """ + """%s""" % (module_name) + """
%include "carrays.i"
%include "cmalloc.i"
%include "cpointer.i"
%include "std_vector.i"
%include "std_string.i"
%{
    #include <thread>
    #include""" + ' "' + p4pd_cli_swig_dir + """p4pd_cli_swig.h"
    #include""" +' "' + p4pd_cust_dir + name + """_custom.h"
    extern int capri_init(void);


    namespace hal {
        thread_local std::thread *t_curr_thread;
    }
%}

#define __attribute__(x)
typedef int p4pd_error_t;
typedef unsigned char uint8_t;
typedef unsigned short uint16_t;
typedef unsigned int uint32_t;
typedef unsigned long long uint64_t;
%template(vector_uint8_t) std::vector<unsigned char>;
%pointer_functions(uint8_t, uint8_ptr_t);
%pointer_functions(uint16_t, uint16_ptr_t);
%pointer_functions(uint32_t, uint32_ptr_t);
%pointer_functions(uint64_t, uint64_ptr_t);
%pointer_functions(int, intp);
%array_functions(uint8_t, uint8_array_t);
%array_functions(uint16_t, uint16_array_t);
%array_functions(uint32_t, uint32_array_t);
%array_functions(uint64_t, uint64_array_t);
%malloc(uint8_t);
%malloc(uint16_t);
%malloc(uint32_t);
%malloc(uint64_t);
%free(uint8_t);
%free(uint16_t);
%free(uint32_t);
%free(uint64_t);
%include"""+' "' + p4pd_cli_swig_dir + """p4pd_cli_swig.h"
%include"""+' "' + p4pd_cust_dir + name + """_custom.h"
"""
    out_file = out_dir + '%s.i' % (name)
    with open(out_file, "w") as of:
        of.write(content_str)
        of.close()

def capri_p4pd_create_swig_main(be):

    name = be.prog_name
    out_dir = be.args.gen_dir + '/%s/cli/' % (name)

    if not os.path.exists(out_dir):
        try:
            os.makedirs(out_dir)
        except OSError as e:
            if e.errno != errno.EEXIST:
                raise

    top_dir = capri_get_top_level_path(out_dir)

    content_str = \
"""
/* This file is auto-generated. Changes will be overwritten! */

#include <stdint.h>
#include <string>
#include <iostream>
#include <thread>
#include "p4pd.h"
#include "common_rxdma_actions_p4pd.h"
#include "common_txdma_actions_p4pd.h"
#include "p4pd_api.hpp"
#include "lib_model_client.h"

extern int capri_init(void);

namespace hal {
    thread_local std::thread *t_curr_thread;
}

int main()
{
    int ret;
    uint32_t tableid = P4TBL_ID_INPUT_PROPERTIES_OTCAM;
    uint32_t index = 11;
    uint32_t hwkey_len;
    uint32_t hwkeymask_len;
    uint32_t hwactiondata_len;
    uint8_t *hwkey_p;
    uint8_t *hwkeymask_p;
    p4pd_table_properties_t tbl_ctx;
    input_properties_otcam_swkey_t sw_key;
    input_properties_otcam_swkey_mask_t swkey_mask;
    input_properties_otcam_actiondata actiondata;
    uint8_t *swkeyp = (uint8_t *)&sw_key;
    uint8_t *swkey_maskp = (uint8_t *)&swkey_mask;
    uint8_t *hwactiondata_p = (uint8_t *)&actiondata;

    """ + \
    """
    setenv("HAL_CONFIG_PATH", "%s", 1);
    """ % ('./conf') + \
    """
    lib_model_connect();
    p4pd_init();
    p4pluspd_rxdma_init();
    p4pluspd_txdma_init();
    capri_init();
    read_reg(0x3408010, hwactiondata_len);

    p4pd_table_properties_get(tableid, &tbl_ctx);
    printf("%s\\n", tbl_ctx.tablename);

    p4pd_hwentry_query(tableid, &hwkey_len, &hwkeymask_len, &hwactiondata_len);

    hwkey_p = (uint8_t *)malloc(((hwkey_len)+7)/8);
    hwkeymask_p = (uint8_t *)malloc(((hwkeymask_len)+7)/8);

    ret = p4pd_hwkey_hwmask_build(tableid, swkeyp, swkey_maskp, hwkey_p, hwkeymask_p);

    if (ret < 0)
    {
        printf("p4pd_hwkey_hwmask_build() returned %d!\\n", (ret));
        return 1;
    }

    actiondata.actionid = INPUT_PROPERTIES_OTCAM_INPUT_PROPERTIES_ID;
    actiondata.input_properties_otcam_action_u.input_properties_otcam_input_properties.vrf = 80;

    ret = p4pd_entry_write(tableid, index, hwkey_p, hwkeymask_p, &actiondata);

    if (ret < 0)
    {
        printf("p4pd_entry_write() returned %d!\\n", (ret));
        return 1;
    }

    printf("Entry was written successfully at index %d\\n", (index));

    ret = p4pd_entry_read(tableid, index, swkeyp, swkey_maskp, &actiondata);

    if (ret < 0)
    {
        printf("Error: p4pd_entry_read() returned %d!\\n", (ret));
        return 0;
    }

    printf("Entry was read successfully at index %d\\n", (index));

    lib_model_conn_close();
    p4pd_cleanup();
    p4pluspd_rxdma_cleanup();
    p4pluspd_txdma_cleanup();
    free(hwkeymask_p);
    free(hwkey_p);
    return 0;
}
"""

    out_file = out_dir + 'main.cc'
    with open(out_file, "w") as of:
        of.write(content_str)
        of.close()

def make_templates_outfiles(template_dir, output_h_dir, output_c_dir, cli_outputdir_map, prog_name, cli_name, pipeline):

    # file-names in template_dir will be used
    # to generate corresponding .c or .h files and
    # output to output_dir
    files = []
    search_dir = template_dir + pipeline

    # walk the pipeline specific dir
    for dir_name, subdir_list, file_list in os.walk(search_dir):
        files.extend([pipeline +'/' + f for f in file_list])

    # walk the template dir
    for dir_name, subdir_list, file_list in os.walk(template_dir):
        if dir_name == template_dir:
            files.extend([os.path.relpath(os.path.join(dir_name, f), template_dir) for f in file_list])

    pdoutfiles = []
    for f in files:
        if f.endswith('.py'):
            output_dir = cli_outputdir_map['default']
            if "p4pd_cli_backend.py" == os.path.basename(f):
                genf = cli_name + '_backend.py'
            elif "p4pd_cli_frontend.py" == f:
                genf = cli_name + '_frontend.py'
            else:
                genf = os.path.basename(f)
        else:
            if f.endswith('.h'):
                output_dir = output_h_dir
            elif f.endswith(".cc"):
                output_dir = output_c_dir
            else:
                continue

            if prog_name != '':
                genf = prog_name + '_' + os.path.basename(f)
            else:
                genf = os.path.basename(f)
        pdoutfiles.append((os.path.join(template_dir, f), \
                           os.path.join(output_dir, genf)))
    return pdoutfiles

class capri_p4pd_generator:
    def __init__(self, capri_be):
        self.be = capri_be
        #self.logger = logging.getLogger('P4PD')
        self.alltables = {}
        self.pddict = {}

    def generate_code(self):
        gen_dir = self.be.args.gen_dir
        h_outputdir = gen_dir + '/%s/include' % (self.be.prog_name)
        c_outputdir = gen_dir + '/%s/src/' % (self.be.prog_name)
        py_outputdir = gen_dir + '/%s/cli' % (self.be.prog_name)

        if not os.path.exists(py_outputdir):
            try:
                os.makedirs(py_outputdir)
            except OSError as e:
                if e.errno != errno.EEXIST:
                    raise

        cur_path = os.path.abspath(__file__)
        cur_path = os.path.split(cur_path)[0]
        templatedir = os.path.join(cur_path, 'pd_templates/')

        if self.be.args.p4_plus:
            prog_name = self.be.prog_name
        else:
            prog_name = ''

        cli_outputdir_map = {}
        cli_outputdir_map['default'] = py_outputdir

        '''
        generate code under : build/$ARCH/$PIPIPELINE/gen/p4gen/$PROG_NAME
        '''
        p4pd_generate_code(self.pddict, templatedir, h_outputdir, c_outputdir,
                           cli_outputdir_map, prog_name, gen_dir,
                           self.be.pipeline)

        '''
        Below ifcondition should be removed when we move apollo code to
        x86_64/apollo/gen/p4gen/apollo to 86_64/apollo/gen/p4gen/p4/ by
        changing the prog name from apollo to p4. generate code under
        build/$ARCH/$PIPIPELINE/gen/p4gen/$PROG_NAME
        '''
        if self.be.prog_name in ['apollo', 'artemis', 'apulu', 'athena']:
            h_outputdir = gen_dir + '/p4/include'
            c_outputdir = gen_dir + '/p4/src/'
            py_outputdir = gen_dir + '/p4/cli/'
            cli_outputdir_map['default'] = py_outputdir
            p4pd_generate_code(self.pddict, templatedir, h_outputdir,
                               c_outputdir, cli_outputdir_map, prog_name,
                               gen_dir, self.be.pipeline)

        outputdir = gen_dir + '/%s/asm_out' % (self.be.prog_name)
        if not os.path.exists(outputdir):
            try:
                os.makedirs(outputdir)
            except OSError as e:
                if e.errno != errno.EEXIST:
                    raise
        cur_path = os.path.abspath(__file__)
        cur_path = os.path.split(cur_path)[0]
        templatedir = os.path.join(cur_path, 'asm_templates/')
        kd_dict = p4pd_generate_asm_code(self.pddict, templatedir, outputdir,
                                         None, None, '',self.be.pipeline)
        return kd_dict

    def generate_swig(self):
        capri_p4pd_create_swig_interface(self.be)
        capri_p4pd_create_swig_custom_hdr(self.be)

    def load_pd_dict(self):
        ingress_file = open(args.ingress_json)
        ingress_str = ingress_file.read()
        ingress_file.close()
        egress_file = open(args.egress_json)
        egress_str = egress_file.read()
        egress_file.close()
        json_data = json.JSONDecoder(object_pairs_hook=OrderedDict).decode(ingress_str)
        json_data["tables"].update((json.JSONDecoder(object_pairs_hook=OrderedDict).decode(egress_str))["tables"])
        for name, table in json_data["tables"].iteritems():
            table['is_toeplitz_hash'] = 0
            table['is_toeplitz'] = False
        self.pddict = json_data
        self.pddict['pipeline'] = args.pipeline
        self.pddict['p4plus_module'] = args.p4_plus_module

    def process_annotations(self):
        # process table and action annotations (pragmas) that influence api generation
        # annotations supported are -
        # - appdatafields
        # - hwfields_access_api

        for t,v in self.pddict['tables'].items():
            # appdatafields and hwfields annotations are internally processed by ncc
            # do not destroy those
            if 'hwfields' in self.pddict['tables'][t]:
                continue
            self.pddict['tables'][t]['hwfields'] = OrderedDict()

            if 'appdatafields' in self.pddict['tables'][t]:
                continue
            self.pddict['tables'][t]['appdatafields'] = OrderedDict()

            for actionname, actiondata, annotations in v['actions']:
                # build adtion data fields dictionary
                adata = OrderedDict()
                for ad in actiondata:
                    fldname = ad['p4_name']
                    if fldname in adata:
                        adata[fldname] = adata[fldname] + ad['len']
                    else:
                        adata[fldname] = ad['len']
                hwfields_acc = False
                # annotations are store as a list where each element is a dictionary that contains
                # only one k,v opair
                for anno in annotations.values()[0]:
                    # each element is a dict with single k,v pair
                    aname, aval = anno.items()[0]
                    if aname == 'appdatafields':
                        actiondatafields = []
                        for adfld in aval:
                            if adfld not in adata:
                                continue
                            actiondatafields.append([adfld, adata[adfld]])

                        self.pddict['tables'][t]['appdatafields'][actionname] = actiondatafields

                    if aname == 'hwfields_access_api':
                        hwfields_acc = True;

                if hwfields_acc:
                    hwfields = []
                    for fldname, fldlen in adata.items():
                        # add all action data fields not specified in the action data to the list
                        add_fld = True
                        if actionname in self.pddict['tables'][t]['appdatafields']:
                            for adf in self.pddict['tables'][t]['appdatafields'][actionname]:
                                if fldname == adf[0]:
                                    add_fld = False
                                    break
                        if add_fld:
                            hwfields.append([fldname, fldlen])

                    self.pddict['tables'][t]['hwfields'][actionname] =  hwfields

def capri_p4pd_code_generate(p4pd_gen):
    p4pd_gen.process_annotations()
    k_plus_d_dict = p4pd_gen.generate_code()
    p4pd_gen.generate_swig()

    return k_plus_d_dict

def get_parser():
    parser = argparse.ArgumentParser(description='C API generator')
    parser.add_argument('--name', dest='program_name', action='store',
                        help='Name of P4/P4+ program',
                        default="")
    parser.add_argument('--ingress-api', dest='ingress_json', action='store',
                        help='Path to IControl.api.json',
                        required=True)
    parser.add_argument('--egress-api', dest='egress_json', action='store',
                        help='Path to EControl.api.json',
                        required=True)
    parser.add_argument('--gen-dir', dest='gen_dir', action='store',
                        help='Directory for all Compiler generated code',
                        default='.', required=False)
    parser.add_argument('--pipeline', dest='pipeline', action='store',
                        help='Pipeline', choices=['iris', 'gft', 'apollo', 'artemis', 'apulu', 'athena'],
                        default=None, required=False)
    parser.add_argument('--p4-plus', dest='p4_plus', action='store_true',
                        help='Compile P4+ program',
                        default=False, required=False)
    parser.add_argument('--p4-plus-module', dest='p4_plus_module', action='store',
                        help='Generate configuration for P4+ program',
                        choices=['rxdma', 'txdma'],
                        default=None, required=False)
    return parser

class _dummy_backend:
    def __init__(self, args):
        self.args = args
        self.prog_name = args.program_name
        self.pipeline = args.pipeline

if __name__ == '__main__':
    args = get_parser().parse_args()
    be = _dummy_backend(args);
    p4pd_gen = capri_p4pd_generator(be)
    p4pd_gen.load_pd_dict()
    capri_p4pd_code_generate(p4pd_gen)

