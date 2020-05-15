import sys, os
import pdb
import csv
import string

chip_name = "capri"
chip_prefix = 'cap'

sys.path.append(os.environ['ASIC_SRC']+ '/' + chip_name + "/verif/common/csr_gen")
sys.path.append(os.environ['ASIC_SRC']+"/common/tools/bin")
sys.path.append(os.environ['ASIC_SRC']+"/" + chip_name + "/design/common")

from gen_decoders import OrderedDictYAMLLoader, component_info, recurse
from collections import namedtuple
from string import Template

import yaml
if chip_name == 'capri':
    import cap_addr as pen_addr
else:    
    import elb_addr as pen_addr
import pprint, re, math

# globals
desc_db = {}
reg_id = 1
message_count = 0
max_msg_per_proto = 120

delphi_metric_objects_def_str = ""
delphi_metric_objects_publish_str = ""
delphi_metric_objects_create_table_str = ""

delphiprotofile = "asicerrord.proto"
protofile = open(delphiprotofile, "w+")

delphiprotofile_1 = "asicerrord_1.proto"
protofile_1 = open(delphiprotofile_1, "w+")

delphiprotofile_2 = "asicerrord_2.proto"
protofile_2 = open(delphiprotofile_2, "w+")

headerindent = 0
delphipublishfile = "asicerrord_delphi.cc"
publishfile = open(delphipublishfile, "w+")

default_header_indent = 0
default_publish_filename = "asicerrord_default.cc"
default_publish_file = open(default_publish_filename, "w+")

## Helper functions for this script

def get_csr_name(csr_name):
    """ get_csr_name: this helper function gets csr_name
    as argument and returns mapped name
    for example, 'cap_emmc_csr' will return 'emmc_SDHOST_Memory_Map'
    """
    if chip_name == 'capri':
        if csr_name == "cap_emmc_csr":
            return "emmc_SDHOST_Memory_Map"
        else :
            return csr_name    
    else :
        raise Exception("Unknown chip_name")


def get_csr_intr_inst_name(csr_name):
    """ converts csr_name into name that can be used to instantiate
    under c/c++
    """
    csr_name = csr_name.replace('.','_')
    csr_name = csr_name.replace('[','_')
    csr_name = csr_name.replace(']','')
    return csr_name




### Extending gen_decoders


def find_all_blocks(self, Registry, block_list):
    """ return a list of sub-blocks/mini CSRs"""
    cur_fields = self.fields
    if len(cur_fields) > 0:
        for field in cur_fields:
            if field.decoder != "":
                if Registry[field.decoder].subtype == 'block':
                    block_list = Registry[field.decoder].find_all_blocks(Registry, block_list)
                    block_list.append(field.decoder)                        

    return block_list




def get_csr_base_addr ( self, search_name, base_addr):
    """ search through fields and determine offset for name"""
    for field in self.fields:
        if field.name == search_name:
            return base_addr + field.offset
    raise Exception("Could not find addr for " + search_name) 

def set_end_addr(self, Registry, indent_level=0):
    """ iterate through all CSRs and fields to determine end_addr for 
    given instance
    """


    if len(self.fields) > 0:

        for field in self.fields:
            if field.decoder != "":
                Registry[field.decoder].set_end_addr(Registry, indent_level+1)
                if field.array > 1:
                    if field.end_addr < field.offset + (Registry[field.decoder].end_addr * field.array) :
                        field.end_addr = field.offset + (Registry[field.decoder].end_addr * field.array)-1
                else:
                    if field.end_addr < field.offset + Registry[field.decoder].end_addr :
                        field.end_addr = field.offset + Registry[field.decoder].end_addr -1
            else:
                field.set_end_addr(Registry, indent_level+1)

            if self.end_addr < field.end_addr:
                self.end_addr = field.end_addr
    else:
       if self.size != 0:

            act_size = (self.size+7)/8
            act_size = next_power_of_2(act_size)
            if act_size < 4: 
                act_size = 4

            if self.array > 1:
               act_size = act_size * self.array                   

            if self.end_addr < (self.offset + act_size):
                self.end_addr = self.offset + act_size -1

       else:
          raise Exception("field size is 0 %s"%(self.name))

    self.end_addr = next_power_of_2(self.end_addr) -1

component_info.get_csr_base_addr = get_csr_base_addr
component_info.find_all_blocks =find_all_blocks 
component_info.set_end_addr = set_end_addr

intreg_declare_template = Template("""intr_reg_t ${inst_name} = {  .name="${name}", .addr=${addr}, .reg_type=${reg_type}, .id=${id}, .field_count=${field_count}, .fields=\n\t{\n${fields}\n\t} };\n""")
nested_intreg_declare_template = Template("""\t\t{  .name="${name}", .next_ptr=${next_ptr}, .id=0, .severity=INTR_SEV_TYPE_INFO, .desc="", .flags=INTR_FLAGS_NONE },\n""");
leaf_int_declare_template = Template("""\t\t{  .name="${name}", .next_ptr=nullptr, .id=${id}, .severity=${severity}, .desc="${desc}", .flags=${flags} },\n""");

delphi_metric_objects_create_template = Template(
"""\
    delphi::objects::${delphi_msg_name}::CreateTable();
""")

delphi_metric_objects_def_template = Template(
"""\
delphi::objects::${delphi_msg_name}_t ${delphi_msg_name};
""")

delphi_metric_objects_publish_template_1 = Template(
"""\
    case ${reg_id}:
    {
        delphi::objects::${delphi_msg_name}_t *reg = &${delphi_msg_name};
        switch (field_id) {
""")

delphi_metric_objects_publish_template_2 = Template(
"""\
        case ${field_num}:
            reg->${field}++;
            break;
""")

delphi_metric_objects_publish_template_3 = Template(
"""\
        default:
            break;
        }
        delphi::objects::${delphi_msg_name}::Publish(key, reg);
        break;
    }
""")

delphi_metric_objects_proto_template_1 = Template(
"""\
message ${delphi_msg_name} {
  fixed64 Key = 1;
""")

delphi_metric_objects_proto_template_2 = Template(
"""\
  delphi.Counter ${field} = ${field_num} [ (venice.metricsField) = { DisplayName: "${display_name}" Description: "${description}" Units: Count } ];
""")


# generate default interrupt notify
def generate_default_interrupt_notify(intr_inst_name, interrupt_fields, reg_id):
    global default_header_indent

    default_header_indent = 4
    default_publish_file.write(' ' * default_header_indent + 'case ' + str(reg_id) + ':\n')
    default_publish_file.write(' ' * default_header_indent + '{\n')
    default_header_indent += 4
    default_publish_file.write(' ' * default_header_indent + 'switch (field_id) {\n')

    # delphi proto fields start from 1
    field_num = 2

    for field, val_list in interrupt_fields:
        default_publish_file.write(' ' * default_header_indent + 'case ' + str(field_num) + ':\n')
        default_header_indent += 4
        default_publish_file.write(' ' * default_header_indent + 'printf("%s_%s\\n", "' + intr_inst_name + '", "' + field + '");\n')
        default_publish_file.write(' ' * default_header_indent + 'break;\n')
        default_header_indent -= 4

        field_num += 1

    # add default case
    default_publish_file.write(' ' * default_header_indent + 'default:\n')
    default_header_indent += 4
    default_publish_file.write(' ' * default_header_indent + 'break;\n')
    default_header_indent -= 4

    default_publish_file.write(' ' * default_header_indent + '}\n')
    default_publish_file.write(' ' * default_header_indent + 'break;\n')

    default_header_indent -= 4
    default_publish_file.write(' ' * default_header_indent + '}\n')


# generate delphi message proto
def generate_delphi_proto_msg(intr_inst_name, interrupt_fields, reg_id):
    global protofile, protofile_1, protofile_2
    global delphi_metric_objects_def_str, delphi_metric_objects_publish_str, delphi_metric_objects_create_table_str
    global message_count, max_msg_per_proto
    global headerindent

    # dont generate message proto if it does have any fields
    if len(interrupt_fields) == 0:
        return

    delphi_msg_name = intr_inst_name.replace('_','').capitalize() + 'Metrics'

    headerindent = 4

    delphi_metric_objects_publish_str += delphi_metric_objects_publish_template_1.substitute(reg_id=reg_id, delphi_msg_name=delphi_msg_name.lower())

    delphi_metric_objects_def_str += delphi_metric_objects_def_template.substitute(delphi_msg_name=delphi_msg_name.lower())

    delphi_metric_objects_create_table_str += delphi_metric_objects_create_template.substitute(delphi_msg_name=delphi_msg_name)

    # delphi proto fields start from 1
    field_num = 1

    # proto indent is 2
    protoindent = 2

    proto_file = protofile
    message_count += 1

    if message_count <= max_msg_per_proto:
        proto_file = protofile
    elif message_count > max_msg_per_proto and message_count <= (2*max_msg_per_proto):
        proto_file = protofile_1
    else:
        proto_file = protofile_2

    proto_file.write(delphi_metric_objects_proto_template_1.substitute(delphi_msg_name=delphi_msg_name))

    # start the field number from 2 since first field is reserved for Key
    field_num += 1

    for field, val_list in interrupt_fields:
        if val_list is None:
            desc = ''
            severity = ''
            display_name = ''
        else:
            desc = val_list[0]
            severity = val_list[1]
            display_name = val_list[2]

        if severity != 'INTR_SEV_TYPE_INFO':
            delphi_metric_objects_publish_str += delphi_metric_objects_publish_template_2.substitute(field_num=field_num, field=field)

            display_name = intr_inst_name + '_' + field
            proto_file.write(delphi_metric_objects_proto_template_2.substitute(field=field, field_num=field_num, display_name=display_name, description=desc))
        field_num += 1

    # add default case
    delphi_metric_objects_publish_str += delphi_metric_objects_publish_template_3.substitute(delphi_msg_name=delphi_msg_name)

    proto_file.write('}\n\n')


# start the function with int_groups
#
def pen_populate_csr_intr_nodes(self, all_csr_nodes, block_itr_str, block_name, block_inst_name, block_base_addr, csr_name, inst_name, inst_offset, Registry, indent_level=0, reverse_fields=0):
    """ this function walks through all fields under CSR
    it detects for interrupt related registers and creates hierachy 
    to walk-through till the source

    This function is recursive

    arguments
    all_csr_nodes   : its a ref to dict which holds all CSRs including mini-CSRs
    block_itr_str   : string that will be added to all_csr_nodes, it is populated recursive way
    block_name      : block name of current call
    block_inst_name : block instant name of current call
    block_base_addr : block base address, offset will be added to this value
    csr_name        : current csr under process
    inst_name       : current inst name under process
    inst_offset     : current instance offset
    Registry        : Registry database - read only
    indent_level    : for print purpose
    reverse_fields  : unused for now
    """

    global reg_id

    # detect error case for non-exist of csr_name in Registry
    if csr_name not in Registry:
        print "Unknown csr_name", csr_name
        raise Exception("Unknown csr_name")

    # if current csr_name is of block type then start with intreg_status
    # iterate over mini-CSRs if any
    if Registry[csr_name].subtype == 'block':
        search_csr_name = csr_name + "_intreg_status"
        new_block_str = ""    

        for field in self.fields:
            if field.name == "int_groups":
                new_offset = field.offset

        if search_csr_name in Registry:
            new_block_str = Registry[search_csr_name].pen_populate_csr_intr_nodes( all_csr_nodes, new_block_str, block_name, block_inst_name, block_base_addr , search_csr_name, "int_groups.intreg" , new_offset, Registry, indent_level+1, reverse_fields=reverse_fields)
            all_csr_nodes[block_inst_name + ".int_groups.intreg"] =new_block_str 
        else :
            print "// could not find any " + search_csr_name + " under " + block_inst_name 

        # this logic looks for mini-csr
        for field in self.fields:
            if field.decoder != "":
                l_is_int_related_reg = 0 
                if field.decoder in Registry and Registry[field.decoder].subtype == 'block':
                    l_is_int_related_reg = 1
                    decoder_name = get_csr_name(field.decoder)

                    if field.array > 1: 
                        for inst_idx in range(field.array):
                            new_block_itr_str = ""
                            new_inst_name = block_inst_name + "." + field.name + '[' + str(inst_idx) + ']' 
                            new_base_addr = block_base_addr + field.offset + ((Registry[decoder_name].end_addr+1)*inst_idx)
                            new_block_itr_str = Registry[decoder_name].pen_populate_csr_intr_nodes( all_csr_nodes, new_block_itr_str, decoder_name, new_inst_name , new_base_addr, decoder_name , "", 0, Registry, indent_level+1, reverse_fields=reverse_fields)
                    else:
                        new_block_itr_str = ""
                        new_inst_name = block_inst_name + "." + field.name
                        new_base_addr = block_base_addr + field.offset 
                        new_block_itr_str = Registry[decoder_name].pen_populate_csr_intr_nodes( all_csr_nodes, new_block_itr_str, decoder_name, new_inst_name , new_base_addr, decoder_name, "", 0, Registry, indent_level+1, reverse_fields=reverse_fields)


    else :
        search_csr_name = csr_name

        reg_defined = 0
        intreg_intr_str = ""

        # block_inst_base_name, block_inst_name: block    name without/with instance
        # inst_base_name,       inst_name      : register name without/with instance
        # intr_inst_name: (block_inst_name + inst_name) with brackets replaced by underscores
        intr_inst_name = get_csr_intr_inst_name(str(block_inst_name + "." + inst_name))
        inst_base_name = re.sub('\d', '', inst_name)
        block_inst_base_name = re.sub('\[\d\]', '', block_inst_name)

        # print (block_inst_name + " " + inst_name)
        # print (block_inst_base_name + " " + inst_base_name)

        interrupt_fields = []

        # start field_id from 2 to match with delphi proto number assignment
        field_id = 2

        # track if all the fields are of type 'INTR_SEV_TYPE_INFO' to avoid
        # generating delphi proto msgs for them
        severity_info = True

        # at this point, we can iterate over all fields and look for interrupt related registers
        # For interrupt fields, we need to determine whether its a nested interrupt or leaf one
        for field in self.fields:
            csr_reg_name = field.name

            matchObj = re.search(r'^int_(.*)_interrupt$|(.*)_intgrp_status$', csr_reg_name)
            nested_reg_found = 0
            if matchObj: 
                if matchObj.group(1):
                    local_search_name = 'int_' + matchObj.group(1) 
                    for search_field in Registry[block_name].fields:
                        if search_field.name == local_search_name:
                            for local_field in Registry[search_field.decoder].fields:
                                if local_field.name == "intreg":
                                    new_csr_reg_name = local_field.decoder
                                    new_csr_inst_name = local_search_name
                                    nested_reg_found =1 
                    if nested_reg_found == 0:
                        field_name = field.name
                        if field_name.endswith('_interrupt'):
                            field_name = field_name[:-10]

                        field_base_name = re.sub('\d', '', field.name.replace('_interrupt', ''))
                        val_list = get_desc_db(block_inst_base_name, inst_base_name, field_base_name)
                        if val_list == None:
                            print("// No desc for " + block_inst_base_name + " " + inst_base_name + " " + field_base_name)
                            desc = ""
                            severity = 'INTR_SEV_TYPE_INFO'
                            flags = 'INTR_FLAGS_NONE'
                        else:
                            desc = val_list[0]
                            severity = val_list[1]
                            flags = val_list[3]

                        if severity != 'INTR_SEV_TYPE_INFO':
                            severity_info = False

                        intreg_intr_str = intreg_intr_str + leaf_int_declare_template.substitute(name=field_name, id=field_id, severity=severity, desc=desc, flags=flags)
                        field_id += 1
                        interrupt_fields.append((field.name, val_list))

                elif matchObj.group(2):
                    new_csr_reg_name = block_name + "_" + matchObj.group(2) + "_intgrp_status" 
                    new_csr_inst_name = "int_" + matchObj.group(1)
                    nested_reg_found = 1
                else :
                    raise Exception("Unknown csr_name")

                if nested_reg_found :
                    new_base_addr = Registry[block_name].get_csr_base_addr(new_csr_inst_name, 0)

                    if new_csr_reg_name in Registry and csr_reg_name != csr_name :
                        new_block_itr_str = "" 
                        new_block_itr_str = Registry[new_csr_reg_name].pen_populate_csr_intr_nodes( all_csr_nodes, new_block_itr_str, block_name, block_inst_name, block_base_addr, new_csr_reg_name , new_csr_inst_name , new_base_addr, Registry, indent_level+1, reverse_fields=reverse_fields)
                        block_itr_str = block_itr_str + new_block_itr_str
                        intreg_intr_str = intreg_intr_str + nested_intreg_declare_template.substitute(name=get_csr_intr_inst_name(new_csr_inst_name), next_ptr="&" + get_csr_intr_inst_name(str(block_inst_name + "." + new_csr_inst_name)) )
                    else :
                        raise Exception("Unwanted match")
            else :
                field_name = field.name
                if field_name.endswith('_interrupt'):
                    field_name = field_name[:-10]

                field_base_name = re.sub('\d', '', field.name.replace('_interrupt', ''))
                val_list = get_desc_db(block_inst_base_name, inst_base_name, field_base_name)
                if val_list == None:
                    print("// No desc for " + block_inst_base_name + " " + inst_base_name + " " + field_base_name)
                    desc = ""
                    severity = 'INTR_SEV_TYPE_INFO'
                    flags = 'INTR_FLAGS_NONE'
                else:
                    desc = val_list[0]
                    severity = val_list[1]
                    flags = val_list[3]

                if severity != 'INTR_SEV_TYPE_INFO':
                    severity_info = False

                intreg_intr_str = intreg_intr_str + leaf_int_declare_template.substitute(name=field_name, id=field_id, severity=severity, desc=desc, flags=flags)
                field_id += 1
                interrupt_fields.append((field_name, val_list))

        reg_type = 0
        if inst_name.endswith('int_groups.intreg'):
            reg_type = 1
        block_itr_str = block_itr_str + intreg_declare_template.substitute(inst_name = get_csr_intr_inst_name(str(block_inst_name + "." + inst_name))  , name=block_inst_name + "." + inst_name , addr=str(hex(block_base_addr+inst_offset)), reg_type=reg_type, id=reg_id, field_count=len(self.fields), fields=intreg_intr_str)

        if severity_info == False:
            generate_delphi_proto_msg(intr_inst_name, interrupt_fields, reg_id)

        generate_default_interrupt_notify(intr_inst_name, interrupt_fields, reg_id)
        reg_id += 1

    return block_itr_str


component_info.pen_populate_csr_intr_nodes = pen_populate_csr_intr_nodes




def next_power_of_2(n):
    p = 1
    if (n and not(n & (n - 1))): 
        return n 
    while (p < n) : 
        p <<= 1
    return p


entryTuple = namedtuple('entryTuple', 'l_name inst_name num start_addr size ring')
def load_all_yaml_db(entry_list):

    logical_names = []
    for item in AddrMapList:
        logical_names.append(item.tla)
    logical_names = list(set(logical_names))

    for l_name in logical_names:
        match = re.search(r'\w', l_name)
        if not match:
            continue;            



        for item in AddrMapList:
            if item.tla != l_name:
                continue
            if not pen_addr.gen_cpp_filter_list(item.iname, 'yaml'):
                continue
            if(item.rng != ""):
                num = int(item.num)
                start = int(item.start, 16)
                msize = int(item.msize, 16)
                entry_list.append(entryTuple(l_name=l_name, inst_name=item.iname, num=num, start_addr=start, size=msize, ring=item.rng))


def get_desc_db(block, register, field):
    global desc_db

    if block not in desc_db:
        return None

    if register not in desc_db[block]:
        return None

    if field not in desc_db[block][register]:
        return None

    val_list = desc_db[block][register][field]

    # print("Get " + block + " " + register + " " + field + " " + val_list[0] + " " + val_list[1])
    return val_list

def insert_desc_db(block, register, field, desc, severity, display_name, flags):
    global desc_db

    if block not in desc_db:
        desc_db[block] = {}

    if register not in desc_db[block]:
        desc_db[block][register] = {}

    # print ("Insert " + " " + block + " " + register + " " + field)
    desc_db[block][register][field] = [desc, severity, display_name, flags]


def parse_severity(severity_str):
    severity_str = string.lower(severity_str)

    if 'fatal' in severity_str:
        return 'INTR_SEV_TYPE_FATAL'

    if 'error' in severity_str:
        return 'INTR_SEV_TYPE_ERR'

    if 'hw-rma' in severity_str:
        return 'INTR_SEV_TYPE_HW_RMA'

    return 'INTR_SEV_TYPE_INFO'

def parse_flags(flags_str):
    flags_str = string.lower(flags_str)

    if 'ignore_all' in flags_str:
        return 'INTR_FLAGS_IGNORE_ALL'

    if 'ignore_once' in flags_str:
        return 'INTR_FLAGS_IGNORE_ONCE'

    return 'INTR_FLAGS_NONE'

def populate_desc_db():
    csv_file = open("intr_list.csv", 'r')

    fields = ( "group","count","owner","parent_int_groups", "register", "field", "path", "name", "type", "allow_port_up", "flags", "severity", "blank1", "info_action", "info_err_count", "blank2", "err_action", "err_count", "desc", "blank3", "fatal_action")

    dict_reader = csv.DictReader(csv_file, fields)

    for row in dict_reader:
        # skip invalid rows
        if row['group'] == "Group":
           continue

        parent_int_groups = row['parent_int_groups']

        register = row['register']
        register = register.replace('#', '')

        field = row['field']
        field = field.replace('#', '')

        flags = parse_flags(row['flags'])

        #if field == 'int_credit_positive_after_return':
         #   pdb.set_trace()

        desc = row['desc']
        severity = parse_severity(row['severity'])

        display_name = parent_int_groups + '_' + register + '_' + field

        # split if multiple blocks
        if '/' in parent_int_groups:
            for block in parent_int_groups.split('/'):
                # strip all spaces
                block = re.sub(' ', '', block)

                block = block.replace('[#]', '')
                insert_desc_db(block, register, field, desc, severity, display_name, flags)
        else:
            parent_int_groups = parent_int_groups.replace('[#]', '')
            insert_desc_db(parent_int_groups, register, field, desc, severity, display_name, flags)

# inits
def init():
    global protofile, protofile_1, protofile_2
    global delphi_metric_objects_publish_str
    global publishfile, default_publish_file

    init_str = \
'''
/**
 * Copyright (c) 2019 Pensando Systems, Inc.
 *
 * @file    interrupts.cc
 *
 * @brief   This file is autogenerated and populates interrupt structures
 */

#include "interrupts.hpp"
'''
    print init_str

    delphi_import_str = \
'''
syntax = "proto3";
package asicerrord;

import "delphi.proto";
import "metrics_annotations.proto";

'''
    protofile.write(delphi_import_str)
    protofile_1.write(delphi_import_str)
    protofile_2.write(delphi_import_str)

    init_str = \
'''
/**
 * Copyright (c) 2019 Pensando Systems, Inc.
 *
 * @file    asicerrord_delphi.cc
 *
 * @brief   This file is autogenerated and handles delphi events
 */

'''
    publishfile.write(init_str)
    publishfile.write('#include <stdio.h>\n')
    publishfile.write('#include <stdlib.h>\n')
    publishfile.write('#include <stdint.h>\n')
    publishfile.write('#include "gen/proto/asicerrord.delphi.hpp"\n')
    publishfile.write('#include "gen/proto/asicerrord_1.delphi.hpp"\n')
    publishfile.write('#include "gen/proto/asicerrord_2.delphi.hpp"\n\n')

    delphi_metric_objects_publish_str += 'sdk_ret_t interrupt_notify (uint64_t reg_id, uint64_t field_id)\n'
    delphi_metric_objects_publish_str += '{\n'
    delphi_metric_objects_publish_str += '    uint64_t key = 0;\n'
    delphi_metric_objects_publish_str += '    switch (reg_id) {\n'

    default_publish_file.write('#include <stdio.h>\n')
    default_publish_file.write('#include <stdlib.h>\n')
    default_publish_file.write('#include <stdint.h>\n\n')
    default_publish_file.write('int interrupt_notify (uint64_t reg_id, uint64_t field_id)\n')
    default_publish_file.write('{\n')
    default_publish_file.write('    switch (reg_id) {\n')

    populate_desc_db()

# end
def end():
    global publishfile
    global headerindent, default_header_indent
    global delphi_metric_objects_def_str, delphi_metric_objects_publish_str, delphi_metric_objects_create_table_str

    publishfile.write(delphi_metric_objects_def_str)
    publishfile.write('\n\n')
    publishfile.write(delphi_metric_objects_publish_str)

    # add default case
    publishfile.write(' ' * headerindent + 'default:\n')
    headerindent += 4
    publishfile.write(' ' * headerindent + 'break;\n')
    headerindent -= 4

    publishfile.write(' ' * headerindent + '}\n')
    publishfile.write(' ' * default_header_indent + 'return SDK_RET_OK;\n')
    headerindent -= 4
    publishfile.write(' ' * headerindent + '}\n')

    publishfile.write('\nvoid create_tables (void)\n{\n')
    publishfile.write(delphi_metric_objects_create_table_str);
    publishfile.write('}')

    default_publish_file.write(' ' * default_header_indent + 'default:\n')
    default_header_indent += 4
    default_publish_file.write(' ' * default_header_indent + 'break;\n')
    default_header_indent -= 4

    default_publish_file.write(' ' * default_header_indent + '}\n')
    default_publish_file.write(' ' * default_header_indent + 'return 0;\n')
    default_header_indent -= 4
    default_publish_file.write(' ' * default_header_indent + '}')


if __name__ == "__main__":
    """ this logic iterates through all YAML files and generates
    database to use later
    """

    init()

    AddrMapList = pen_addr.AddrMapList
    data_map = {}

    
    entry_list = []
    Registry={}
    load_all_yaml_db(entry_list)
    asic_src = os.environ['ASIC_SRC']
    asic_gen = os.environ['ASIC_GEN']
    csr_gen_path = asic_src + "/" + chip_name + "/verif/common/csr_gen/"
    csr_define_path = asic_src + "/" + chip_name + "/model/" + chip_prefix + "_top/csr_defines/"
    parent_info = component_info(name="root", blockname="root", chip_prefix=chip_prefix)
    pending_block_csr = []
    for entry in entry_list:
        if entry.inst_name in data_map:
            continue
        real_path = ""
        if not os.path.isfile(csr_gen_path+entry.inst_name+".yaml"):
            if not os.path.isfile(csr_gen_path+ chip_prefix + "_"+entry.inst_name+".yaml"):
                print csr_gen_path+entry.inst_name+".yaml doesn't exist"
                continue
            else:
                real_path = csr_gen_path + chip_prefix + "_" + entry.inst_name +".yaml"
        else:                
            real_path = csr_gen_path+entry.inst_name+".yaml"
        with open( real_path) as f:
            data_map[entry.inst_name] = yaml.load(f, OrderedDictYAMLLoader)
            recurse(data_map[entry.inst_name], parent_info, Registry, chip_prefix, 0, 1)
            csr_name = chip_prefix + "_"+entry.inst_name + "_csr"
            csr_name = get_csr_name(csr_name)
            pending_block_csr = []
            pending_block_csr = Registry[csr_name].find_all_blocks(Registry, pending_block_csr)
            pending_block_csr.append(csr_name)
            pending_block_csr = list(set(pending_block_csr))

            for block_name in pending_block_csr:
                csr_name = get_csr_name(block_name)
        
                if csr_name in Registry:                
                    Registry[csr_name].set_tot_size(Registry)
                else :
                    raise Exception("Can not find %s" % csr_name )
               
        
            real_path = asic_gen + "/" + chip_name + "/model/" + chip_prefix + "_top/"


    """
    This is related to interrupt
    iterate over all top level PRP nodes and populate dict named 'all_csr_nodes'
    """


    all_csr_nodes = {}

    chip_inst_str = []

    for entry in entry_list:
        csr_name = chip_prefix + "_"+entry.inst_name + "_csr"
        decoder_name = get_csr_name(csr_name)
        populate_intr_str = ""
        Registry[decoder_name].set_end_addr(Registry)
        if entry.num > 1 :
            for inst_id in range(entry.num):
                inst_name = entry.l_name + "." + entry.inst_name + "[" + str(inst_id) + "]"
                base_addr = entry.start_addr + (entry.size * inst_id)
                populate_intr_str = Registry[decoder_name].pen_populate_csr_intr_nodes(all_csr_nodes, populate_intr_str, decoder_name, inst_name, base_addr, decoder_name, entry.inst_name , 0, Registry)
                chip_inst_str.append({get_csr_intr_inst_name(inst_name) : get_csr_intr_inst_name(inst_name) + "_int_groups_intreg"})
        else :
            inst_name = entry.l_name + "." + entry.inst_name 
            base_addr = entry.start_addr 
            populate_intr_str = Registry[decoder_name].pen_populate_csr_intr_nodes(all_csr_nodes, populate_intr_str, decoder_name, inst_name, base_addr, decoder_name, entry.inst_name , 0, Registry)
            chip_inst_str.append({get_csr_intr_inst_name(inst_name) : get_csr_intr_inst_name(inst_name) + "_int_groups_intreg"})

    """
    following logic implements all CSRs including mini-CSRs
    """
    intreg_field_str = ""
    l_field_count = 0
    for key,val in all_csr_nodes.items():
        print val
        intreg_field_str = intreg_field_str + nested_intreg_declare_template.substitute(name=get_csr_intr_inst_name(key), next_ptr="&"+get_csr_intr_inst_name(str(key)))
        l_field_count = l_field_count + 1
    print intreg_declare_template.substitute(inst_name = get_csr_intr_inst_name(str('all_csrs')) , name='all_csrs', addr=str(hex(0)), reg_type=2, id=0, field_count=l_field_count , fields=intreg_field_str)


    """
    Following logic takes care of <inst>0 - top level interrupt service
    """
    intreg_field_str = ""
    l_field_count = 0
    for inst in chip_inst_str:
        for key,val in inst.items():
            for csr_key,csr_val in all_csr_nodes.items():
                csr_new = get_csr_intr_inst_name(csr_key)
                if csr_new == val:
                    intreg_field_str = intreg_field_str + nested_intreg_declare_template.substitute(name=key, next_ptr="&"+val)
                    l_field_count = l_field_count + 1
                    break
    print intreg_declare_template.substitute(inst_name = get_csr_intr_inst_name(str(chip_prefix + '0')) , name=chip_prefix + '0', addr=str(hex(0)), reg_type=2, id=0, field_count=l_field_count, fields=intreg_field_str)

    end()
