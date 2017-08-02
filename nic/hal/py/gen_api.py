#!/usr/bin/env python
# -*- coding: utf-8 -*-

from __future__ import print_function
import argparse
import json
import os
import sys
import time
import subprocess
import pdb

line_seperator = "// " + "-" * 80 + "\n"

def create_file(obj):
    f = open(obj['name'] + ".h", 'w')
    return f

def make_common_header(obj, f):

    macro = '__' + obj['name'].upper() + '_H__'

    # Auto generation Warning
    f.write(line_seperator)
    f.write("// " + "Warning: This is an Auto-generated file.\n")
    f.write(line_seperator)
    f.write("\n")


    # Header Lines Comment
    f.write(line_seperator)
    f.write("// " + obj['name'].upper() + " HAL API related public data structures and APIs" + "\n")
    f.write(line_seperator)

    #
    f.write("#ifndef " + macro + "\n")
    f.write("#define " + macro + "\n")
    f.write("\n")
    f.write("#ifdef __cplusplus\n")
    f.write("extern \"C\" {\n")
    f.write("#endif\n")
    f.write("\n")
    f.write("#include <base.h>\n")
    f.write("#include <eth.h>\n")
    f.write("\n")


    pass

def make_common_trailer(obj, f):
    macro = '__' + obj['name'].upper() + '_H__'

    f.write("#ifdef __cplusplus\n")
    f.write("}\n")
    f.write("#endif\n")
    f.write("\n")

    f.write("#endif // " + macro)

    pass

def make_key_ds(obj, f):
    # Form Key structure
    f.write(line_seperator)
    f.write("// " + obj['name'].upper() + " key\n")
    f.write(line_seperator)

    f.write("typedef struct " + obj['name'] + "_key_s { \n")
    for key in obj['key_fields']:
        if 'num_bits' not in  key.keys():
            f.write("    " + key['type'] + " " + key['name'] + ";")
        else:
            f.write("    " + key['type'] + " " + key['name'] + ":" + key['num_bits'] + ";")
        f.write("\n")

    f.write("} __PACK__ " + obj['name'] + "_key_t; \n")
    f.write("\n")

    # Form Key Handle
    f.write(line_seperator)
    f.write("// " + obj['name'].upper() + " Valid bits for key/handle\n")
    f.write(line_seperator)

    f.write("typedef struct " + obj['name'] + "_key_handle_s { \n")
    f.write("    " + obj['name'] + "_key_t " + "key;\n")
    f.write("    hal_handle_t hndl;\n")
    f.write("    uint8_t __key_valid:1;\n")
    f.write("    uint8_t __hndl_valid:1;\n")

    f.write("} __PACK__ " + obj['name'] + "_key_handle_t; \n")
    f.write("\n")

    # Form Multiple Key Handles
    f.write(line_seperator)
    f.write("// " + obj['name'].upper() + " Multiple Key Handles \n")
    f.write(line_seperator)

    f.write("typedef struct " + obj['name'] + "_key_handles_s { \n")
    f.write("    uint32_t num_khs;\n")
    f.write("    " + obj['name'] + "_key_handle_t " + "khs[0];\n")

    f.write("} __PACK__ " + obj['name'] + "_key_handles_t; \n")
    f.write("\n")

def make_data_ds(obj, f):
    # Making "Data" structure
    f.write(line_seperator)
    f.write("// " + obj['name'].upper() + " data. Valid bits for each field \n")
    f.write(line_seperator)

    f.write("typedef struct " + obj['name'] + "_data_s { \n")
    for data in obj['data_fields']:
        if 'num_bits' not in  data.keys():
            f.write("    " + data['type'] + " " + data['name'] + ";")
        else:
            f.write("    " + data['type'] + " " + data['name'] + ":" + data['num_bits'] + ";")
        f.write("\n")
        f.write("    uint8_t __" + data['name'] + "_valid:1;\n")
        f.write("\n")

    f.write("} __PACK__ " + obj['name'] + "_data_t; \n")
    f.write("\n")

    # Making Oper State structure
    f.write(line_seperator)
    f.write("// " + obj['name'].upper() + " operational state \n")
    f.write(line_seperator)

    f.write("typedef struct " + obj['name'] + "_oper_s { \n")
    f.write("    hal_ret_t      reason_code;\n")
    f.write("} __PACK__ " + obj['name'] + "_oper_t; \n")
    f.write("\n")

    # Make Full entry
    f.write(line_seperator)
    f.write("// " + obj['name'].upper() + " Full Entry \n")
    f.write(line_seperator)

    f.write("typedef struct " + obj['name'] + "_s { \n")
    f.write("    " + obj['name'] + "_key_handle_t kh;\n")
    f.write("    " + obj['name'] + "_data_t data;\n")
    f.write("    " + obj['name'] + "_oper_t oper;\n")

    f.write("} __PACK__ " + obj['name'] + "_t; \n")
    f.write("\n")

    # Make Full entries
    f.write(line_seperator)
    f.write("// " + obj['name'].upper() + " Full Entries \n")
    f.write(line_seperator)

    f.write("typedef struct " + obj['name'] + "s_s { \n")
    f.write("    uint32_t num_entries;\n")
    f.write("    " + obj['name'] + "_t entries[0];\n")

    f.write("} __PACK__ " + obj['name'] + "s_t; \n")
    f.write("\n")

    # Make filter structure
    f.write(line_seperator)
    f.write("// " + obj['name'].upper() + " Filter structure\n")
    f.write(line_seperator)

    f.write("typedef struct " + obj['name'] + "_filter_s { \n")
    # Key fields
    for key in obj['key_fields']:
        if 'num_bits' not in  key.keys():
            f.write("    " + key['type'] + " " + key['name'] + ";")
        else:
            f.write("    " + key['type'] + " " + key['name'] + ":" + key['num_bits'] + ";")
        f.write("\n")
        f.write("    uint8_t __" + key['name'] + "_valid:1;\n")
        f.write("\n")
    # Data fields
    for data in obj['data_fields']:
        if 'num_bits' not in  data.keys():
            f.write("    " + data['type'] + " " + data['name'] + ";")
        else:
            f.write("    " + data['type'] + " " + data['name'] + ":" + data['num_bits'] + ";")
        f.write("\n")
        f.write("    uint8_t __" + data['name'] + "_valid:1;\n")
        f.write("\n")

    f.write("} __PACK__ " + obj['name'] + "_filter_t; \n")
    f.write("\n")

def make_CRUD_apis(obj, f):
    # CRUD APIs
    f.write(line_seperator)
    f.write("// " + obj['name'].upper() + " CRUD APIs\n")
    f.write(line_seperator)

    # Create API
    f.write("hal_ret_t hal_create_" + obj['name'] + "(" + obj['name'] + "s_t *" + obj['name'] + "s __INOUT__);\n")

    # Retrieve API
    f.write("hal_ret_t hal_read_" + obj['name'] + "(" + obj['name'] + "_key_handles_t *khs __IN__,\n")
    f.write("    " + obj['name'] + "_filter_t *filter __IN__,\n")
    f.write("    " + obj['name'] + "s_t **" + obj['name'] + "s __OUT__);\n")

    # Update API
    f.write("hal_ret_t hal_update_" + obj['name'] + "(" + obj['name'] + "s_t *" + obj['name'] + "s __IN__);\n")
    f.write("hal_ret_t hal_update_" + obj['name'] + "_by_key(" + obj['name'] + "_key_handles_t *khs,\n")
    f.write("    " + obj['name'] + "_data_t *data __IN__);\n")

    # Delete API
    f.write("hal_ret_t hal_delete_" + obj['name'] + "(" + obj['name'] + "_key_handles_t *khs __IN__);\n")

    f.write("\n")


def make_init_apis(obj, f):
    # Key Handle Init
    f.write(line_seperator)
    f.write("// " + obj['name'].upper() + " Key Handle Init\n")
    f.write(line_seperator)
    f.write("static inline void\n")
    f.write("hal_init_" + obj['name'] + "_key_handle (" + obj['name'] +"_key_handle_t *kh __INOUT__)\n")
    f.write("{\n")
    f.write("    kh->__key_valid = FALSE;\n")
    f.write("    kh->__hndl_valid = FALSE;\n")
    f.write("}\n")
    f.write("\n")

    # Key Handles Init
    f.write(line_seperator)
    f.write("// " + obj['name'].upper() + " Key Handles Init\n")
    f.write(line_seperator)
    f.write("static inline void\n")
    f.write("hal_init_" + obj['name'] + "_key_handles (" + obj['name'] +"_key_handles_t *kh __INOUT__)\n")
    f.write("{\n")
    f.write("    khs->num_khs = 0;\n")
    f.write("}\n")
    f.write("\n")

    # Data Init
    f.write(line_seperator)
    f.write("// " + obj['name'].upper() + " Data Fields Init\n")
    f.write(line_seperator)
    f.write("static inline void\n")
    f.write("hal_init_" + obj['name'] + "_data (" + obj['name'] +"_data_t *data __INOUT__)\n")
    f.write("{\n")
    for data in obj['data_fields']:
        f.write("    data->__" + data['name'] + "_valid" + " = FALSE;\n")
    f.write("}\n")
    f.write("\n")

    # Filter Init
    f.write(line_seperator)
    f.write("// " + obj['name'].upper() + " Filter Fields Init\n")
    f.write(line_seperator)
    f.write("static inline void\n")
    f.write("hal_init_" + obj['name'] + "_filter (" + obj['name'] +"_filter_t *filter __INOUT__)\n")
    f.write("{\n")
    # Key fields
    for key in obj['key_fields']:
        f.write("    " + "filter->__" + key['name'] + "_valid = FALSE;\n")
    for data in obj['data_fields']:
        f.write("    " + "filter->__" + data['name'] + "_valid = FALSE;\n")
    f.write("}\n")
    f.write("\n")

    # Full Entries Init
    f.write(line_seperator)
    f.write("// " + obj['name'].upper() + " Full Entries Init\n")
    f.write(line_seperator)
    f.write("static inline void\n")
    f.write("hal_init_" + obj['name'] + "s (" + obj['name'] +"s_t *entries __INOUT__)\n")
    f.write("{\n")
    f.write("    entries->num_entries = 0;\n")
    f.write("}\n")
    f.write("\n")

def make_key_set_apis(obj, f):
    # Setting Key
    f.write(line_seperator)
    f.write("// " + obj['name'].upper() + " Setting Key\n")
    f.write(line_seperator)
    f.write("static inline void\n")
    f.write("hal_set_" + obj['name'] + "_key_handle_key (" + obj['name'] +"_key_handle_t *kh __INOUT__,\n")
    for key in obj['key_fields'][:-1]:
        f.write("    " + key['type'] + " " + key['name'] + " __IN__,\n")
    key = obj['key_fields'][-1]
    f.write("    " + key['type'] + " " + key['name'] + " __IN__)\n")
    f.write("{\n")
    for key in obj['key_fields']:
        if key['type'] == 'mac_addr_t':
            f.write("    memcpy(kh->key." + key['name'] + ", " + key['name'] + ", ETH_ADDR_LEN);\n")
        else:
            f.write("    kh->key." + key['name'] + " = " + key['name'] + ";\n")
    f.write("    kh->__key_valid = TRUE;\n")
    f.write("}\n")
    f.write("\n")

    # Setting Handle
    f.write(line_seperator)
    f.write("// " + obj['name'].upper() + " Setting Key Handle\n")
    f.write(line_seperator)
    f.write("static inline void\n")
    f.write("hal_set_" + obj['name'] + "_key_handle_handle (" + obj['name'] +"_key_handle_t *kh __INOUT__,\n")
    f.write("    hal_handle_t hndl __IN__)\n")
    f.write("{\n")
    f.write("    kh->hndl = hndl;\n")
    f.write("    kh->__hndl_valid = TRUE;\n")
    f.write("}\n")
    f.write("\n")

def make_data_set_apis(obj, f):
    for data in obj['data_fields']:
        f.write(line_seperator)
        f.write("// " + obj['name'].upper() + " Setting Data Field " + data['name'] + "\n")
        f.write(line_seperator)
        f.write("static inline void\n")
        f.write("hal_set_" + obj['name'] + "_data_" + data['name'] + " (" + obj['name'] +"_data_t *data __INOUT__,\n")
        f.write("    " + data['type'] + " " + data['name'] + " __IN__)\n")
        f.write("{\n")
        f.write("    data->" + data['name'] + " = " + data['name'] + ";\n")
        f.write("    data->__" + data['name'] + "_valid = TRUE;\n")
        f.write("}\n")
        f.write("\n")


def make_filter_set_apis(obj, f):
    # Key fields
    for key in obj['key_fields']:
        f.write(line_seperator)
        f.write("// " + obj['name'].upper() + " Setting Filter Field " + key['name'] + "\n")
        f.write(line_seperator)
        f.write("static inline void\n")
        f.write("hal_set_" + obj['name'] + "_filter_" + key['name'] + " (" + obj['name'] +"_filter_t *filter __INOUT__,\n")
        f.write("    " + key['type'] + " " + key['name'] + " __IN__)\n")
        f.write("{\n")
        if key['type'] == 'mac_addr_t':
            f.write("    memcpy(filter->" + key['name'] + ", " + key['name'] + ", ETH_ADDR_LEN);\n")
        else:
            f.write("    filter->" + key['name'] + " = " + key['name'] + ";\n")
        f.write("    filter->__" + key['name'] + "_valid = TRUE;\n")
        f.write("}\n")
        f.write("\n")

    # Data fields
    for data in obj['data_fields']:
        f.write(line_seperator)
        f.write("// " + obj['name'].upper() + " Setting Filter Field " + data['name'] + "\n")
        f.write(line_seperator)
        f.write("static inline void\n")
        f.write("hal_set_" + obj['name'] + "_filter_" + data['name'] + " (" + obj['name'] +"_filter_t *filter __INOUT__,\n")
        f.write("    " + data['type'] + " " + data['name'] + " __IN__)\n")
        f.write("{\n")
        if data['type'] == 'mac_addr_t':
            f.write("    memcpy(filter->" + data['name'] + ", " + data['name'] + ", ETH_ADDR_LEN);\n")
        else:
            f.write("    filter->" + data['name'] + " = " + data['name'] + ";\n")
        f.write("    filter->__" + data['name'] + "_valid = TRUE;\n")
        f.write("}\n")
        f.write("\n")


def make_hdr_file(obj):
    print ("Making Header file for obj: %s" % obj['name'])

    # Create the file
    f = create_file(obj)

    # Make Common Code
    make_common_header(obj, f)

    # Make Data Structures
    make_key_ds(obj, f)
    make_data_ds(obj, f)

    # Make CRUD APIs and set APIs for fields
    make_CRUD_apis(obj, f)
    make_init_apis(obj, f)
    make_key_set_apis(obj, f)
    make_data_set_apis(obj, f)
    make_filter_set_apis(obj, f)


    # Make Common Trailer
    make_common_trailer(obj, f)
    pass


def prepare_parser():
    parser = argparse.ArgumentParser(description="Generates HAL APIs for objects in Json")
    parser.add_argument("-c", "--cfg", type=str, required=True, help = "Json file with HAL Objects ")

    return parser

if __name__ == "__main__":
    parser = prepare_parser()
    args = parser.parse_args()

    json_file = args.cfg

    with open(json_file) as json_file:
        json_data = json.load(json_file)


        for obj in json_data['objects']:
            make_hdr_file(obj)

   	# Open Cfg File for SIM and Model
   	# f = open('conn.cfg', 'w')



