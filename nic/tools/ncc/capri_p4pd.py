#!/usr/bin/python
#
# Capri-Non-Compiler-Compiler (capri-ncc)
# Mahesh Shirshyad (Pensando Systems)

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

from p4_hlir.main import HLIR
import p4_hlir.hlir.p4 as p4
import p4_hlir.hlir.table_dependency as table_dependency
from p4_hlir.graphs.dependency_graph import *
import p4_hlir.hlir.analysis_utils as hlir_utils

import capri_logging
from capri_utils import *
from capri_pa import capri_field as capri_field
from capri_output import _get_output_name as _get_output_name
from capri_output import capri_p4pd_create_swig_makefile as capri_p4pd_create_swig_makefile
from capri_output import capri_p4pd_create_swig_makefile_click as capri_p4pd_create_swig_makefile_click
from capri_output import capri_p4pd_create_bazel_build as capri_p4pd_create_bazel_build
from capri_output import capri_p4pd_create_swig_custom_hdr as capri_p4pd_create_swig_custom_hdr
from capri_output import capri_p4pd_create_swig_interface as capri_p4pd_create_swig_interface
from capri_output import capri_p4pd_create_swig_main as capri_p4pd_create_swig_main

tenjin_prefix = "//::"

CHECK_INVALID_C_VARIABLE = re.compile(r'[^a-zA-Z0-9_]')

def make_templates_outfiles(template_dir, output_h_dir, output_c_dir, cli_outputdir_map, prog_name, cli_name):

    # file-names in template_dir will be used
    # to generate corresponding .c or .h files and
    # output to output_dir
    files = [f for f in os.listdir(template_dir) \
              if os.path.isfile(os.path.join(template_dir,f))]

    pdoutfiles = []
    for f in files:
        if f.endswith('.py'):
            output_dir = cli_outputdir_map['default']
            if "p4pd_cli_backend.py" == f:
                genf = cli_name + '_backend.py'
            elif "p4pd_cli_frontend.py" == f:
                genf = cli_name + '_frontend.py'
            elif "p4pd_cli_frontend_common.py" == f:
                output_dir = cli_outputdir_map['common']
                genf = 'cli_frontend.py'
            else:
                genf = f
        else:
            if f.endswith('.h'):
                output_dir = output_h_dir
            elif f.endswith(".cc"):
                output_dir = output_c_dir
            else:
                continue

            if prog_name != '':
                genf = prog_name + '_' + f
            else:
                genf = f

        pdoutfiles.append((os.path.join(template_dir, f), \
                           os.path.join(output_dir, genf)))
    return pdoutfiles

def p4pd_generate_code(pd_dict, template_dir, output_h_dir, output_c_dir, cli_outputdir_map, prog_name, gen_dir):

    if output_h_dir and not os.path.exists(output_h_dir):
        os.mkdir(output_h_dir)
    if output_c_dir and not os.path.exists(output_c_dir):
        os.mkdir(output_c_dir)

    if cli_outputdir_map is not None:
        for output_dir in cli_outputdir_map.values():
            if output_dir and not os.path.exists(output_dir):
                os.mkdir(output_dir)

    templates_outfiles = make_templates_outfiles(template_dir, output_h_dir, output_c_dir, cli_outputdir_map, prog_name, pd_dict['cli-name'])
    _prog_name = ''
    if prog_name != '':
        prog_name = prog_name + '_'
        _prog_name = '_' + prog_name
    for templatefile, outfile in templates_outfiles:
        outputfile_path = os.path.dirname(outfile)
        pdd = {}
        with open(outfile, "w") as of:
            pdd['pddict'] = pd_dict
            p4tbl_types = render_template(of, templatefile, pdd, template_dir, \
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


def p4pd_generate_asm_code(pd_dict, template_dir, output_h_dir, output_c_dir, cli_outputdir_map, prog_name):

    if output_h_dir and not os.path.exists(output_h_dir):
        os.mkdir(output_h_dir)
    if output_c_dir and not os.path.exists(output_c_dir):
        os.mkdir(output_c_dir)

    if cli_outputdir_map is not None:
        for output_dir in cli_outputdir_map.values():
            if output_dir and not os.path.exists(output_dir):
                os.mkdir(output_dir)

    templates_outfiles = make_templates_outfiles(template_dir, output_h_dir, output_c_dir, cli_outputdir_map, prog_name, pd_dict['cli-name'])
    kd_json = {}
    for templatefile, outfile in templates_outfiles:
        outputfile_path = os.path.dirname(outfile)
        pdd = {}
        with open(outfile, "w") as of:
            pdd['pddict'] = pd_dict
            kd_dict = render_template(of, templatefile, pdd, template_dir, \
                            prefix=tenjin_prefix)
            for k, v in kd_dict.items():
                kd_json[k] = v
            of.close()
    return kd_json

class capri_p4pd:
    def __init__(self, capri_be):
        self.be = capri_be
        self.logger = logging.getLogger('P4PD')
        self.alltables = {}
        self.pddict = {}

    def convert_p4fldname_to_valid_c_variable(self, p4fldname):
        allowed = string.letters + string.digits + '_'
        vstr = ''
        for c in p4fldname:
            if c not in allowed:
                if c != ']':
                    vstr += '_'
            else:
                vstr += c
        return vstr

    def cfield_get_p4name_width_mask(self, cf, ctype):
        hname = cf.get_p4_hdr().name
        p4fldname = hname + '_' + cf.p4_fld.name
        p4fldwidth = cf.width
        if ctype == p4.p4_match_type.P4_MATCH_EXACT:
            mask = (1 << p4fldwidth) - 1
        elif ctype == p4.p4_match_type.P4_MATCH_VALID:
            mask = (1 << p4fldwidth) - 1
            #ctable.k_hv_flds
        elif ctype == p4.p4_match_type.P4_MATCH_TERNARY or \
             ctype == match_type.EXACT_HASH_OTCAM:
            mask = 0    # Run time API when installing entry
                        # into tcam table will specify mask for
                        # ternary match. Hence set to 0
        else:
            pass
        return p4fldname, p4fldwidth, mask

    def cfield_get_p4name_width(self, cf):
        if cf.get_p4_hdr():
            hname = cf.get_p4_hdr().name
            p4fldname = hname + '_' + cf.p4_fld.name
            p4fldwidth = cf.width
        else:
            p4fldname = cf.hfname
            p4fldwidth = cf.width
        return p4fldname, p4fldwidth

    def build_table_keys_from_hdr_union(self, ctable, cf_table_keylist, \
                                        tbl_km_byte_format, \
                                        tbl_km_bit_format):
        processedhdrunion = []
        hdr_unions = {}
        for cfield, ctype in cf_table_keylist.items():
            if cfield.is_hdr_union \
               or (cfield.is_union_storage() and not cfield.is_fld_union):
                # Get list of other headers that this table key-field's
                # header is unionized with.
                # Check if table key is header field from any unionizedheaders
                hdrs = self.be.pa.gress_pa[ctable.d].hdr_unions.values()
                cfieldhdr = cfield.get_p4_hdr()

                if cfieldhdr not in self.be.pa.gress_pa[ctable.d].hdr_unions:
                    continue

                _ , unionizedhdrlist, hdrcontainer = \
                    self.be.pa.gress_pa[ctable.d].hdr_unions[cfieldhdr]

                for h in unionizedhdrlist:
                    if h in processedhdrunion:
                        continue
                    hfields = self.be.pa.gress_pa[ctable.d].get_header_all_cfields(h)
                    unionkeys = []
                    for kcf , kcftype in cf_table_keylist.items():
                        if kcf in hfields and kcf not in unionkeys:
                            unionkeys.append((kcf, kcftype))
                    if len(unionkeys):
                        hdr_unions[h] = unionkeys
                    processedhdrunion.append(h)

        # None of table keys are hdr unions
        if not hdr_unions:
            return []

        hdr_u_keys = []
        for k, values in hdr_unions.items():
            u_keys = []
            for cf, ctype in values:
                p4fldname, p4fldwidth, mask = self.cfield_get_p4name_width_mask(cf, ctype)
                if CHECK_INVALID_C_VARIABLE.search(p4fldname):
                    p4fldname = \
                        self.convert_p4fldname_to_valid_c_variable(p4fldname)
                byteformat = [] # Entry formatting that goes into HW table
                                # at byte extraction boundary
                bitformat = []  # Entry formatting that goes into HW table
                                # at bit extraction boundary
                if cf in tbl_km_byte_format.keys():
                    byteformat = tbl_km_byte_format[cf]
                if cf in tbl_km_bit_format.keys():
                    bitformat = tbl_km_bit_format[cf]
                u_keys.append((p4fldname, p4fldwidth, mask, byteformat, bitformat))
            union_name = k.name
            if CHECK_INVALID_C_VARIABLE.search(union_name):
                union_name = \
                        self.convert_p4fldname_to_valid_c_variable(unionname)
            hdr_u_keys.append((union_name, u_keys))

        return hdr_u_keys

    def build_table_keys_from_fld_union(self, ctable, cf_table_keylist, \
                                        tbl_km_byte_format, \
                                        tbl_km_bit_format):
        processedfldcontainers = []
        fldunions = {}
        for cfield, ctype in cf_table_keylist.items():
            if cfield.is_fld_union:
                unionizedfieldlist, fieldcontainer = \
                    self.be.pa.gress_pa[ctable.d].fld_unions[cfield]
                if fieldcontainer not in processedfldcontainers:
                    unionkeys = []
                    for cf in unionizedfieldlist:
                        if cf in cf_table_keylist.keys():
                            unionkeys.append((cf, cf_table_keylist[cf]))
                    if len(unionkeys):
                        fldunions[fieldcontainer] = unionkeys
                        processedfldcontainers.append(fieldcontainer)

        # None of table keys are field unions
        if not fldunions:
            return []

        fld_u_keys = []
        for k, values in fldunions.items():
            u_keys = []
            for cf, ctype in values:
                p4fldname, p4fldwidth, mask = self.cfield_get_p4name_width_mask(cf, ctype)
                if CHECK_INVALID_C_VARIABLE.search(p4fldname):
                    p4fldname = \
                        self.convert_p4fldname_to_valid_c_variable(p4fldname)
                byteformat = [] # Entry formatting that goes into HW table
                                # at byte extraction boundary
                bitformat = []  # Entry formatting that goes into HW table
                                # at bit extraction boundary
                if cf in tbl_km_byte_format.keys():
                    byteformat = tbl_km_byte_format[cf]
                if cf in tbl_km_bit_format.keys():
                    bitformat = tbl_km_bit_format[cf]
                u_keys.append((p4fldname, p4fldwidth, mask, byteformat, bitformat))

            hname = k.get_p4_hdr().name
            fldunionname = hname + '_' + k.p4_fld.name
            if CHECK_INVALID_C_VARIABLE.search(fldunionname):
                    fldunionname = \
                        self.convert_p4fldname_to_valid_c_variable(fldunionname)
            fld_u_keys.append((fldunionname, u_keys))

        return fld_u_keys




    def build_table_key_hwformat(self, ctable, cf_table_keylist):
        '''
            This function builds dictionary of
            - KM byte --> (table match key p4 field, byte position in the field).
            - table match key p4 field --> List[KM byte, byte of P4field].
        '''
        # +--------------------------------------------------------------+
        # | I1-Bytes | Not my Table | My Table | K-bits| I-bits| I2-Bytes|
        # |          | K-Bytes (only| K-Bytes  |       |       |         |
        # |          | TCAM case)   |          |       |       |         |
        # +--------------------------------------------------------------+

        # km_byte_format dict is built where key is byte position in key-maker
        # and dict value is (carpi-field object, byte within capri field.
        km_byte_to_cf_map   = OrderedDict() # maintains mapping of KM byte# to
                                            # (table-key, byte within key,
                                            #  width=8)
        km_bit_to_cf_map    = OrderedDict() # maintains mapping of KM bit# to
                                            # (table-key, bit within key,
                                            #  width=1)
        match_key_start_byte = 0 # Locates Byte position in KM where
                                 # match-key starts. This is same as
                                 # length of I1 bytes + not my key bytes.
        km_bit = 0
        km_byte = 0
        match_key_len = 0
        not_my_key_bytes = []
        not_my_key_bits = []
        cf_table_ilist = ctable.input_fields
        km_width = self.be.hw_model['match_action']['key_maker_width']
        match_key_start_byte = ctable.start_key_off / 8
        match_key_start_bit = ctable.start_key_off
        if ctable.is_wide_key:
            match_key_start_byte = ctable.last_flit_start_key_off / 8
            match_key_start_bit = ctable.last_flit_start_key_off

        for km_inst, km in enumerate(ctable.key_makers):
            km_start_byte = km_inst * (km_width/8) #Each Key maker has 256bits

            table_km_cprofile = km.combined_profile
            if not km.combined_profile:
                continue # key-less table

            # If there is shared KM for this table, use it; else use
            # regular table km profile. (when KM is exclusive to this table)
            if km.shared_km:
                km_cprofile = km.shared_km.combined_profile
            else:
                km_cprofile = table_km_cprofile

            # Each km profile has atmost 32 bytes populated using k / k,i
            # km_byte tracks KM byte which is final/correct position in
            # HW table

            # When TCAM table ahd hash/idx table share KM, its possible to
            # to push Key in I (for tcam table). In such cases, we might need
            # to mask...!!

            for km_byte in range(len(km_cprofile.byte_sel)):
                km_phv_byte = km_cprofile.byte_sel[km_byte]

                if km_phv_byte in km_cprofile.i1_byte_sel:
                    if km_phv_byte not in table_km_cprofile.k_byte_sel:
                        if ctable.match_type == match_type.TERNARY or \
                           ctable.match_type == match_type.EXACT_HASH_OTCAM:
                            # In case of TCAM Key bytes that belong to another
                            # table but are in the same KM as this table,
                            # (KM is shared by 2 Tcam tables) they will be
                            # part of this table's key that need to be
                            # masked out.
                            if len(km_byte_to_cf_map) > 0 or len(km_bit_to_cf_map) > 0:
                                not_my_key_bytes.append(km_start_byte + km_byte)

                    else: #if km_phv_byte in table_km_cprofile.k_byte_sel:
                        # Collect all 'k' phv bytes and map each of those
                        # KM byte position to byte location within every key field
                        # that table uses as match key.
                        for cf in cf_table_keylist:
                            # cf bits could span more than a byte with
                            # less than 8 bits spilled to another byte.
                            # Example:  PhvB-0 = field-bits 0 - 2
                            #           PhvB-1 = field-bits 3 - 10
                            # remaning 5bits in PhvB-0 could be filled by
                            # different p4 field.
                            for phvc in cf.phcs:
                                # Find phvc that matches km_phv_byte loaded into KM
                                # Using phvc find p4field startbit
                                if phvc != km_phv_byte:
                                    continue
                                containerstart, cf_startbit, width = \
                                    self.be.pa.gress_pa[ctable.d].phcs[phvc].fields[cf.hfname]
                                assert(width <= 8), pdb.set_trace()
                                # When key comes from field union and more than
                                # one field from the same union is used as keys
                                # to table, then we need to collect for all keys
                                # that are field unionized.
                                if km_start_byte + km_byte not in km_byte_to_cf_map.keys():
                                    km_byte_to_cf_map[km_start_byte + km_byte] = [(cf, cf_startbit, width, "K", containerstart)]
                                else:
                                    # Incase of field union, 2 different k bytes can map
                                    # same km byte position or K and I fields share same byte
                                    km_byte_to_cf_map[km_start_byte + km_byte].append((cf, cf_startbit, width, "K", containerstart))
                                break

                        # Traverse input list of the table and check for presence
                        # of input fields in Key-bytes (KM can place input bits
                        # along with Keybits)
                        for cf in cf_table_ilist:
                            for phvc in cf.phcs:
                                # Find phvc that matches km_phv_byte loaded into KM
                                # Using phvc find p4field startbit
                                if phvc != km_phv_byte:
                                    continue
                                containerstart, cf_startbit, width = \
                                    self.be.pa.gress_pa[ctable.d].phcs[phvc].fields[cf.hfname]
                                assert(width <= 8), pdb.set_trace()
                                if km_start_byte + km_byte not in km_byte_to_cf_map.keys():
                                    km_byte_to_cf_map[km_start_byte + km_byte] = [(cf, cf_startbit, width, "I", containerstart)]
                                else:
                                    # Incase of field union, 2 different k bytes can map
                                    # same km byte position or K and I fields share same byte
                                    km_byte_to_cf_map[km_start_byte + km_byte].append((cf, cf_startbit, width,"I", containerstart))
                                break

                if km_phv_byte in km_cprofile.k_byte_sel:
                    if km_phv_byte not in table_km_cprofile.k_byte_sel:
                        if ctable.match_type == match_type.TERNARY or \
                           ctable.match_type == match_type.EXACT_HASH_OTCAM:
                            # In case of TCAM Key bytes that belong to another
                            # table but are in the same KM as this table,
                            # (KM is shared by 2 Tcam tables) they will be
                            # part of this table's key that need to be
                            # masked out.
                            if len(km_byte_to_cf_map) > 0 or len(km_bit_to_cf_map) > 0:
                                not_my_key_bytes.append(km_start_byte + km_byte)

                    else: #if km_phv_byte in table_km_cprofile.k_byte_sel:
                        # Collect all 'k' phv bytes and map each of those
                        # KM byte position to byte location within every key field
                        # that table uses as match key.
                        for cf in cf_table_keylist:
                            # cf bits could span more than a byte with
                            # less than 8 bits spilled to another byte.
                            # Example:  PhvB-0 = field-bits 0 - 2
                            #           PhvB-1 = field-bits 3 - 10
                            # remaning 5bits in PhvB-0 could be filled by
                            # different p4 field.
                            for phvc in cf.phcs:
                                # Find phvc that matches km_phv_byte loaded into KM
                                # Using phvc find p4field startbit
                                if phvc != km_phv_byte:
                                    continue
                                containerstart, cf_startbit, width = \
                                    self.be.pa.gress_pa[ctable.d].phcs[phvc].fields[cf.hfname]
                                assert(width <= 8), pdb.set_trace()
                                # When key comes from field union and more than
                                # one field from the same union is used as keys
                                # to table, then we need to collect for all keys
                                # that are field unionized.
                                if km_start_byte + km_byte not in km_byte_to_cf_map.keys():
                                    km_byte_to_cf_map[km_start_byte + km_byte] = [(cf, cf_startbit, width, "K", containerstart)]
                                else:
                                    # Incase of field union, 2 different k bytes can map
                                    # same km byte position or K and I fields share same byte
                                    km_byte_to_cf_map[km_start_byte + km_byte].append((cf, cf_startbit, width, "K", containerstart))
                                break

                        # Traverse input list of the table and check for presence
                        # of input fields in Key-bytes (KM can place input bits
                        # along with Keybits)
                        for cf in cf_table_ilist:
                            for phvc in cf.phcs:
                                # Find phvc that matches km_phv_byte loaded into KM
                                # Using phvc find p4field startbit
                                if phvc != km_phv_byte:
                                    continue
                                containerstart, cf_startbit, width = \
                                    self.be.pa.gress_pa[ctable.d].phcs[phvc].fields[cf.hfname]
                                assert(width <= 8), pdb.set_trace()
                                if km_start_byte + km_byte not in km_byte_to_cf_map.keys():
                                    km_byte_to_cf_map[km_start_byte + km_byte] = [(cf, cf_startbit, width, "I", containerstart)]
                                else:
                                    # Incase of field union, 2 different k bytes can map
                                    # same km byte position or K and I fields share same byte
                                    km_byte_to_cf_map[km_start_byte + km_byte].append((cf, cf_startbit, width,"I", containerstart))
                                break


            start_kbit = km_cprofile.bit_loc * 8 + (km_inst * 32 * 8)
            if len(km_cprofile.k_bit_sel) > 0:
                for km_bit in range(len(km_cprofile.k_bit_sel)):
                    phv_bit = km_cprofile.k_bit_sel[km_bit]
                    cf_bit_found = False
                    for cf in cf_table_keylist:
                        if phv_bit <= cf.phv_bit + cf.width:
                            for cf_bit in range(0, cf.width):
                                if (cf.phv_bit + cf_bit) == phv_bit:
                                    if start_kbit + km_bit not in km_bit_to_cf_map.keys():
                                        km_bit_to_cf_map[start_kbit + km_bit] = [(cf, cf_bit, 1, "K")]
                                    else:
                                        # Incase of field union, 2 different k bytes can map
                                        # same km byte position.
                                        km_bit_to_cf_map[start_kbit + km_bit].append[(cf, cf_bit, 1, "K")]
                                    cf_bit_found = True
                                    break
                    if not cf_bit_found:
                        # Phv bit that is not my table key bit.
                        # Belongs to key of another table and part
                        # of shared key maker.
                        if ctable.match_type == match_type.TERNARY or \
                           ctable.match_type == match_type.EXACT_HASH_OTCAM:
                            # In case of TCAM Key bits that belong to another
                            # table but are in the same KM as this table,
                            # (KM is shared by 2 Tcam tables) they will be
                            # part of this table's key that need to be
                            # masked out.
                            if len(km_byte_to_cf_map) > 0 or len(km_bit_to_cf_map) > 0:
                                not_my_key_bits.append(start_kbit + km_bit)


        match_key_len =  ctable.end_key_off - ctable.start_key_off
        # km_format dict is built where key is byte position in key-maker
        # Build inverse dict where key is capri-field and value is list of
        # of key-maker bytes and bits
        tbl_cf_to_km_byte_map = {} # Maps Table-Key capri-field-object to list of
                                   # [(KM-byte-location, table-key-field-byte#)]
        for km_byte, cf_value_list in km_byte_to_cf_map.items():
            for cf_value in cf_value_list:
                cf, cf_startbit, width, _, containerstart  = cf_value
                if cf not in tbl_cf_to_km_byte_map.keys():
                    tbl_cf_to_km_byte_map[cf] = [(km_byte, cf_startbit, width, containerstart)]
                else:
                    tbl_cf_to_km_byte_map[cf].append((km_byte, cf_startbit, width, containerstart))

        tbl_cf_to_km_bit_map = {} # Maps Table-Key capri-field-object to list of
                                  # [(KM-bit-location, table-key-field-bit#)]
        for km_bit, cf_value_list in km_bit_to_cf_map.items():
            for cf_value in cf_value_list:
                cf, cf_bit, _, _  = cf_value
                if cf not in tbl_cf_to_km_bit_map.keys():
                    tbl_cf_to_km_bit_map[cf] = [(km_bit, cf_bit)]
                else:
                    tbl_cf_to_km_bit_map[cf].append((km_bit, cf_bit))

        kdict = {}
        kdict['cf_to_km_byte']          = tbl_cf_to_km_byte_map
        kdict['cf_to_km_bit']           = tbl_cf_to_km_bit_map
        kdict['match_key_start_byte']   = match_key_start_byte
        kdict['match_key_start_bit']    = match_key_start_bit
        kdict['match_key_len']          = match_key_len
        kdict['not_my_key_bytes']       = not_my_key_bytes
        kdict['not_my_key_bits']        = not_my_key_bits
        kdict['km_byte_to_cf']          = km_byte_to_cf_map
        kdict['km_bit_to_cf']           = km_bit_to_cf_map
        kdict['wide_key_len']           = 0
        if ctable.is_wide_key:
            kdict['wide_key_len']       = ctable.last_flit_end_key_off - ctable.last_flit_start_key_off
        return kdict


    def build_table_k_fields(self, ctable):
        '''
        '''
        if ctable.match_type != match_type.MPU_ONLY:
            #All capri fields that are keys of the table.
            cf_keylist = {}
            for k in ctable.keys:
                cfield = k[0]
                ctype = k[1]
                cf_keylist[cfield] = ctype


            # Build KeyFormat as done in KM.
            kdict = self.build_table_key_hwformat(ctable, cf_keylist)
            # process all fld unions
            fld_u_keys = \
                self.build_table_keys_from_fld_union(ctable,\
                                                     cf_keylist, \
                                                     kdict['cf_to_km_byte'], \
                                                     kdict['cf_to_km_bit'])
            # process all hdr_unions
            hdr_u_keys = \
                self.build_table_keys_from_hdr_union(ctable,\
                                                     cf_keylist,
                                                     kdict['cf_to_km_byte'], \
                                                     kdict['cf_to_km_bit'])
            # process all table keys that are not fld or header union
            tblkeys = []
            for k in ctable.keys:
                cfield = k[0]
                ctype = k[1]

                if cfield.is_hv:
                    hfname = cfield.hfname.split('.')
                    p4fldname = hfname[0] + '_' + hfname[1]
                    p4fldwidth = 1
                    mask = (1 << p4fldwidth) - 1
                elif cfield.is_key or cfield.is_meta or cfield.is_intrinsic:
                    p4fldname, p4fldwidth, mask = \
                        self.cfield_get_p4name_width_mask(cfield, ctype)
                else:
                    assert(0), "Invalid header field"

                if not cfield.is_fld_union and not cfield.is_hdr_union \
                    and not cfield.is_union_storage():
                    if CHECK_INVALID_C_VARIABLE.search(p4fldname):
                        p4fldname = \
                            self.convert_p4fldname_to_valid_c_variable(p4fldname)

                    # Every table key field when installed in HW RAM/TCAM line
                    # need match key maker format. All bytes of key are extracted
                    # by byte-extractor; hence the array 'byteformat[]'
                    # specifies location of table key bytes to location in
                    # memory.
                    # When key length is not multiple of 8bits, remaining bits
                    # (after extracting all bytes) are extraced by bit-extractors.
                    # The array 'bitformat[]' specifies location of table key
                    # bits to bit location in memory.
                    byteformat = [] # Entry formatting that goes into HW table
                                    # at byte extraction boundary
                    bitformat = []  # Entry formatting that goes into HW table
                                    # at bit extraction boundary
                    if cfield in kdict['cf_to_km_byte'].keys():
                        byteformat = kdict['cf_to_km_byte'][cfield]
                    if cfield in kdict['cf_to_km_bit'].keys():
                        bitformat = kdict['cf_to_km_bit'][cfield]

                    tblkeys.append((p4fldname, p4fldwidth, mask, \
                                    byteformat, bitformat))
            kdict['fld_u_keys'] = fld_u_keys
            kdict['hdr_u_keys'] = hdr_u_keys
            kdict['keys']       = tblkeys
            kdict['keysize'] = ctable.final_key_size
        else:
            kdict = {}
            kdict['not_my_key_bytes']       = []
            kdict['not_my_key_bits']        = []
            kdict['match_key_start_byte']   = 0
            kdict['match_key_start_bit']    = 0
            kdict['match_key_len']          = 0
            kdict['km_byte_to_cf']          = []
            kdict['km_bit_to_cf']           = []
            kdict['fld_u_keys']             = []
            kdict['hdr_u_keys']             = []
            kdict['keys']                   = []
            kdict['keysize']                = 0
            kdict['wide_key_len']           = 0
        # Build all table action data fields
        tblactions = []
        if not ctable.is_hbm and ctable.is_writeback and not ctable.is_raw and not ctable.is_raw_index:
            #Pad actiondata so that sum of all action data fields and action-id
            #is next multiple of 128b
            action_pc_size = 8 if len(ctable.action_data) > 1 else 0
            for actionname,adatafields in ctable.action_data.items():
                adata_size = action_pc_size
                actiondata_flds = []
                for actiondata_fld, actiondata_fld_width in adatafields:
                    actiondata_flds.append((actiondata_fld, actiondata_fld_width))
                    adata_size += actiondata_fld_width
                if (adata_size % 128) > 0:
                    pad_size = 128 - (adata_size % 128)
                    actiondata_flds.append(('__' + actionname + '_wb_pad', pad_size))
                tblactions.append((actionname, actiondata_flds))
        else:
            for actionname,adatafields in ctable.action_data.items():
                tblactions.append((actionname, adatafields))

        kdict['actions'] = tblactions

        return kdict

    def remove_duplicate_cfs(self, ctable, ki_or_kd_to_cf_map, bit_extractors, is_tcam):
        if len(bit_extractors):
            pad_list = []
            for kbit in bit_extractors:
                cflist = ki_or_kd_to_cf_map[kbit]
                for cfs in cflist:
                    if cfs[0] == None:
                        (cf, cf_startbit, width, ftype, hdr) = cfs
                    else:
                        (cf, cf_startbit, width, ftype) = cfs
                    # check if cf
                    for k, v  in ki_or_kd_to_cf_map.items():
                        if k == kbit:
                            continue
                        for dict_cfs in v:
                            if dict_cfs[0] == None:
                                (cf_, cf_startbit_ , width_, ftype_, hdr)  =  dict_cfs
                            else:
                                (cf_, cf_startbit_ , width_, ftype_)  =  dict_cfs
                            if cf == cf_:
                                if cf_startbit >= cf_startbit_ and cf_startbit < (cf_startbit_ + width_):
                                    # Duplicate; Convert to pad
                                    # In case of hash, k-bits can also appear as I bit.
                                    # Hence pad them and no need to make sure duplicate
                                    # should appear only in tcam case.
                                    #assert(is_tcam), pdb.set_trace()
                                    # Convert to Pad
                                    if ftype == 'K':
                                        #In case of duplicate key, remove one that
                                        #is outside the key-range.
                                        if k < ctable.start_key_off or \
                                            k > ctable.end_key_off:
                                            pad_list.append(kbit)
                                    else:
                                        pad_list.append(kbit)
            if len(pad_list):
                for kbit in pad_list:
                    ki_or_kd_to_cf_map[kbit] = [(None, kbit, 1, "P", "__NoHdr")]

        #remove duplicate cfs that are sourced into KM byte without
        #using bit extractors.
        pad_dup_cf = OrderedDict()
        for k, v  in ki_or_kd_to_cf_map.items():
            if len(v) == 1:
                continue
            for dict_cfs in v:
                if dict_cfs[0] == None:
                    (cf, cf_startbit, width, ftype, hdr) = dict_cfs
                else:
                    (cf, cf_startbit, width, ftype) = dict_cfs
                if cf == None:
                    continue
                for k1, v1  in ki_or_kd_to_cf_map.items():
                    if len(v1) == 1:
                        continue
                    if k == k1:
                        continue
                    for cfs in v1:
                        if cfs[0] == None:
                            (cf_, cf_startbit_ , width_, ftype_, hdr)  =  cfs
                        else:
                            (cf_, cf_startbit_ , width_, ftype_)  =  cfs
                        if cf == cf_ and cf_startbit_ == cf_startbit:
                            if (cf_, cf_startbit, width_) not in pad_dup_cf.values():
                                pad_dup_cf[k1] = (cf_, cf_startbit, width_)
        if len(pad_dup_cf):
            for k, dict_cf_ in pad_dup_cf.items():
                (cf, startbit, width) = dict_cf_
                cf_list = ki_or_kd_to_cf_map[k]
                for idx, elem in enumerate(cf_list):
                    if elem[0] == None:
                        (cf_, cf_startbit_ , width_, ftype_, hdr)  =  elem
                    else:
                        (cf_, cf_startbit_ , width_, ftype_)  =  elem
                    if cf == cf_:
                        cf_list[idx] = (None, k+cf_startbit_, width_, "P", cf_get_hname(cf))

    def purge_duplicate_pad(self, ki_or_kd_to_cf_map):
        covered_bits = 0
        skipped_fields_k = []
        for k, v  in ki_or_kd_to_cf_map.items():
            if len(v) > 1:
                max_width = 0
                for dict_cfs in v:
                    if dict_cfs[0] == None:
                        (cf_, cf_startbit_ , width_, ftype_, hdr)  =  dict_cfs
                    else:
                        (cf_, cf_startbit_ , width_, ftype_)  =  dict_cfs
                    if cf_startbit_ != k + max_width:
                        skipped_fields_k.append(k + max_width)
                    max_width += width_
                if max_width > 8:
                    max_width = 8
                covered_bits = k + max_width
            else:
                # check if this field is already covered
                if k < covered_bits and k not in skipped_fields_k:
                    for dict_cfs in v:
                        if dict_cfs[0] == None:
                            (cf_, cf_startbit_ , width_, ftype_, hdr)  =  dict_cfs
                        else:
                            (cf_, cf_startbit_ , width_, ftype_)  =  dict_cfs
                        if cf_ == None:
                            del ki_or_kd_to_cf_map[k]

    def spurious_ki_fix(self, ki_or_kd_to_cf_map):
        ki_names = []
        spurious_ki_cfs = set()
        for k, v  in ki_or_kd_to_cf_map.items():
            for dict_cfs in v:
                if dict_cfs[0] != None:
                    (cf_, cf_startbit_ , width_, ftype_)  =  dict_cfs
                    if (cf_.hfname, cf_startbit_) not in ki_names:
                        ki_names.append((cf_.hfname, cf_startbit_))
                    else:
                        spurious_ki_cfs.add((dict_cfs, k))

        while len(spurious_ki_cfs) > 0:
            #Need to convert duplicate K/I into spurious_<key/input>
            elem = spurious_ki_cfs.pop()
            dict_cfs, index  = elem
            v = ki_or_kd_to_cf_map[index]
            new_v = []
            for dcf in v:
                if dcf == dict_cfs:
                    new_v.append((None, dcf[1],  dcf[2], dcf[3], 'spurious_' + dcf[0].hfname.replace('.', '_')))
                else:
                    new_v.append(dcf)
            ki_or_kd_to_cf_map[index] = new_v

    def build_table_asm_hwformat(self, ctable, kd=0):
        '''
            This function builds dictionary of KiM byte to
            (action input p4 field, byte position in the field).
        '''
        # +--------------------------------------------------------------+
        # | I1-Bytes | Not my Table | My Table | K-bits| I-bits| I2-Bytes|
        # |          | K-Bytes (only| K-Bytes  |       |       |         |
        # |          | TCAM case)   |          |       |       |         |
        # +--------------------------------------------------------------+

        ki_or_kd_to_cf_map    = OrderedDict() # maintains mapping of KM byte/bit# to
                                        # cf, cf_bit/cf_start_bit, width
        km_width = self.be.hw_model['match_action']['key_maker_width']

        key_encountered = False
        if ctable.match_type == match_type.TERNARY \
           or ctable.match_type == match_type.TERNARY_ONLY:
            is_tcam = 1
        else:
            is_tcam = 0
        bit_extractors = []

        start_km_inst = 0
        wide_key_width = 0
        if ctable.is_wide_key:
            #In case of wide key table build KI using only last two KM.
            start_km_inst = len(ctable.key_makers) - 2

        for km_inst, km in enumerate(ctable.key_makers):
            if km_inst < start_km_inst:
                continue
            km_start_byte = km_inst * (km_width/8)
            if not km.combined_profile:
                continue # key-less table
            table_km_cprofile = km.combined_profile
            # If there is shared KM for this table, use it; else use
            # regular table km profile. (when KM is exclusive to this table)
            if km.shared_km:
                km_cprofile = km.shared_km.combined_profile
            else:
                km_cprofile = table_km_cprofile

            nbytes = (len(km_cprofile.bit_sel) + 7) / 8

            for km_byte_pos in range(len(km_cprofile.byte_sel)):
                phv_byte = km_cprofile.byte_sel[km_byte_pos]
                phv_byte_found = False
                kbit = (km_start_byte + km_byte_pos) * 8
                if phv_byte == -1:
                    if km_byte_pos != km_cprofile.bit_loc and \
                       km_byte_pos != km_cprofile.bit_loc1:
                        put_pad = False
                        if is_tcam:
                            put_pad = True
                        elif not kd:
                            put_pad = True
                        if put_pad:
                            # PAD unused byte
                            ki_or_kd_to_cf_map[kbit] = [(None, kbit, 8, "P", "__NoHdr")]
                        continue


                    # This KM byte holds bit. When there are more
                    # than a 8 bits extracted all bits in bit_sel is processed.
                    for km_bit_pos, phv_bit in enumerate(km_cprofile.bit_sel):
                        if km_byte_pos == km_cprofile.bit_loc1:
                            if km_bit_pos < 8:
                                continue
                        if km_byte_pos == km_cprofile.bit_loc:
                            if km_bit_pos >= 8:
                                continue
                        phv_bit_found = False
                        for cf in ctable.input_fields:
                            if phv_bit <= cf.phv_bit + cf.width:
                                #ignore i when building KD if key not encountered
                                if not is_tcam and kd:
                                    continue
                                if is_tcam and kd and not key_encountered:
                                    continue
                                for cf_bit in range(0, cf.width):
                                    if (cf.phv_bit + cf_bit) == phv_bit:
                                        if kbit not in ki_or_kd_to_cf_map.keys():
                                            ki_or_kd_to_cf_map[kbit] = [(cf, cf_bit, 1, "I")]
                                            bit_extractors.append(kbit)
                                        else:
                                            # Incase of field union, 2 different input bits can map
                                            # same km byte position.
                                            if (cf, cf_bit, 1, "I") not in ki_or_kd_to_cf_map[kbit]:
                                                # When sharing KMs, there can be duplicate p4_fld
                                                ki_or_kd_to_cf_map[kbit].append((cf, cf_bit, 1, "I"))
                                                bit_extractors.append(kbit)
                                        phv_bit_found = True
                                        break

                        if phv_bit_found:
                            # There can be cases where cf bit is both key and input. For such cases
                            # continue to next KM field bit.
                            kbit += 1
                            continue

                        for _cf in ctable.keys:
                            cf = _cf[0]
                            if phv_bit <= cf.phv_bit + cf.width:
                                if not key_encountered:
                                    key_encountered = True
                                for cf_bit in range(0, cf.width):
                                    if (cf.phv_bit + cf_bit) == phv_bit:
                                        if kbit not in ki_or_kd_to_cf_map.keys():
                                            ki_or_kd_to_cf_map[kbit] = [(cf, cf_bit, 1, "K")]
                                            bit_extractors.append(kbit)
                                        else:
                                            # Incase of field union, 2 different input bits can map
                                            # same km byte position.
                                            if (cf, cf_bit, 1, "K") not in ki_or_kd_to_cf_map[kbit]:
                                                ki_or_kd_to_cf_map[kbit].append((cf, cf_bit, 1, "K"))
                                                bit_extractors.append(kbit)
                                        phv_bit_found = True
                                        break
                        if not phv_bit_found and is_tcam:
                            ki_or_kd_to_cf_map[kbit] = [(None, kbit, 1, "P", "__NoHdr")]
                        elif not phv_bit_found and not kd:
                            # In case of KI, pad unused bit; However in case of KD
                            # unused bit when packing as KD will be used by D bit
                            # in entry packing.
                            ki_or_kd_to_cf_map[kbit] = [(None, kbit, 1, "P", "__NoHdr")]

                        kbit += 1

                    if kbit % 8 > 0:
                        # IF bit extractor does not completely fill 8 bits
                        # In case of KD unfilled bits (trailing or leading)
                        # will be used by data.
                        if not kd or is_tcam:
                            ki_or_kd_to_cf_map[kbit] = [(None, kbit, 8 - kbit % 8, "P", "__NoHdr")]

                elif phv_byte != -1 and len(self.be.pa.gress_pa[ctable.d].phcs[phv_byte].fields) == 1:
                    phv_byte_found = False
                    for cf in ctable.input_fields:
                        if phv_byte in cf.phcs:
                            phv_byte_found = True
                            if not is_tcam and kd:
                               continue
                            if is_tcam and kd and not key_encountered:
                                continue
                            containerstart, cf_startbit, width = \
                                self.be.pa.gress_pa[ctable.d].phcs[phv_byte].fields[cf.hfname]
                            if kbit not in ki_or_kd_to_cf_map.keys():
                                ki_or_kd_to_cf_map[kbit] = [(cf, cf_startbit, min(8, cf.width), "I")]
                            else:
                                # Incase of field union, 2 action inputs can map
                                # same km byte position.
                                if (cf, cf_startbit, min(8, cf.width), "I") not in ki_or_kd_to_cf_map[kbit]:
                                    ki_or_kd_to_cf_map[kbit].append((cf, cf_startbit, min(8, cf.width), "I"))
                    for _cf in ctable.keys:
                        cf = _cf[0]
                        if phv_byte in cf.phcs:
                            phv_byte_found = True
                            if not key_encountered:
                                key_encountered = True
                            containerstart, cf_startbit, width = \
                                self.be.pa.gress_pa[ctable.d].phcs[phv_byte].fields[cf.hfname]
                            if kbit not in ki_or_kd_to_cf_map.keys():
                                #Check if same key byte that is in second KM instance was also in first KM inst...
                                if km_inst:
                                    if  [(cf, cf_startbit, min(8, cf.width), "K")] \
                                               not in ki_or_kd_to_cf_map.values():
                                        ki_or_kd_to_cf_map[kbit] = [(cf, cf_startbit, min(8, cf.width), "K")]
                                    else:
                                        # Duplicate key bits.. Pad it
                                        phv_byte_found = False
                                else:
                                    ki_or_kd_to_cf_map[kbit] = [(cf, cf_startbit, min(8, cf.width), "K")]
                            else:
                                # Incase of field union, 2 key bytes can map
                                # same km byte position.
                                if (cf, cf_startbit, min(8, cf.width), "K") not in ki_or_kd_to_cf_map[kbit]:
                                    #Check if same key byte that is in second KM instance was also in first KM inst...
                                    if km_inst:
                                        if (cf, cf_startbit, min(8, cf.width), "K") not in ki_or_kd_to_cf_map[kbit]:
                                            ki_or_kd_to_cf_map[kbit].append((cf, cf_startbit, min(8, cf.width), "K"))
                                        else:
                                            # Duplicate key bits.. Pad it
                                            phv_byte_found = False
                                    else:
                                        ki_or_kd_to_cf_map[kbit].append((cf, cf_startbit, min(8, cf.width), "K"))
                    if not phv_byte_found and is_tcam:
                        ki_or_kd_to_cf_map[kbit] = [(None, kbit, 8, "P", "__NoHdr")]
                    elif not phv_byte_found and not kd:
                        ki_or_kd_to_cf_map[kbit] = [(None, kbit, 8, "P", "__NoHdr")]
                elif phv_byte != -1:
                    #Check if  single phv byte is sourced from multiple headers and
                    #fields from different headers are inter mixed...like example below.
                    # Byte X has following fields {
                    # Hdr1.field1[1bit]
                    # Hdr1.field2[1bit]
                    # Hdr2.field1[1bit]
                    # Hdr1.field3[1bit]
                    # Hdr2.field2[1bit]
                    #   :
                    # In this case, do not group header fields on per header basis. This will
                    # result in reordering of fields in final K+I
                    hdrs_seen = {}
                    phv_byte_sourced_with_intermixed_multihdr = False
                    intermixed_multihdr = False
                    for k, v  in self.be.pa.gress_pa[ctable.d].phcs[phv_byte].fields.items():
                        containerstart, cf_startbit, width = v
                        if k.split(".")[0] not in hdrs_seen.keys():
                            hdrs_seen[k.split(".")[0]] = width
                            hdr = k.split(".")[0]
                            cur_hdr = hdr
                        else:
                            hdrs_seen[k.split(".")[0]] += width
                            #if same as cur_hdr, then move on
                            if cur_hdr == k.split(".")[0]:
                                continue
                            else:
                                #headers fields are intermixed.
                                intermixed_multihdr = True

                    if len(hdrs_seen) > 1 and intermixed_multihdr:
                        for k, v in hdrs_seen.items():
                            if v  <  8:
                                #Phv byte is not completely filled by every header that sources its
                                #content. Phv byte is filled by eclectic fields from eclectic hdrs.
                                phv_byte_sourced_with_intermixed_multihdr = True
                                break

                    if phv_byte_sourced_with_intermixed_multihdr:
                        for k, v  in self.be.pa.gress_pa[ctable.d].phcs[phv_byte].fields.items():
                            containerstart, cf_startbit, width = v
                            cf = self.be.pa.get_field(k, ctable.d)
                            if cf == None:
                                ki_or_kd_to_cf_map[kbit + containerstart] = [(cf, cf_startbit, width, "D", "__Pad")]
                            else:
                                ki_or_kd_to_cf_map[kbit + containerstart] = [(cf, cf_startbit, width, "D")]
                        continue

                    fields_of_same_header_in_byte = {}
                    total_p4_fld_width = 0
                    for k, v  in self.be.pa.gress_pa[ctable.d].phcs[phv_byte].fields.items():
                        field_found = False
                        for cf in ctable.input_fields:
                            if cf.hfname == k:
                                field_found = True
                                if not is_tcam and kd:
                                    continue
                                if is_tcam and kd and not key_encountered:
                                    continue
                                containerstart, cf_startbit, width = \
                                    self.be.pa.gress_pa[ctable.d].phcs[phv_byte].fields[cf.hfname]
                                if cf.is_hv:
                                    cf_hname = cf.hfname
                                else:
                                    cf_hname = cf_get_hname(cf)
                                if cf_hname not in fields_of_same_header_in_byte.keys():
                                    fields_of_same_header_in_byte[cf_hname] = containerstart
                                cs = fields_of_same_header_in_byte[cf_hname]
                                if kbit+cs not in ki_or_kd_to_cf_map.keys():
                                    ki_or_kd_to_cf_map[kbit+cs] = [(cf, cf_startbit, width, "I")]
                                    total_p4_fld_width += width
                                else:
                                    # Incase of field union, 2 action input sub-bits can map
                                    # same km sub-bit position.
                                    if (cf, cf_startbit, width, "I") not in ki_or_kd_to_cf_map[kbit+cs]:
                                        ki_or_kd_to_cf_map[kbit+cs].append((cf, cf_startbit, width, "I"))
                                        total_p4_fld_width += width


                        if field_found:
                            # There can be cases where cf is both key and input. For such cases
                            # continue to next KM field.
                            continue

                        for _cf in ctable.keys:
                            cf = _cf[0]
                            if cf.hfname == k:
                                field_found = True
                                if not key_encountered:
                                    key_encountered = True
                                containerstart, cf_startbit, width = \
                                    self.be.pa.gress_pa[ctable.d].phcs[phv_byte].fields[cf.hfname]
                                if cf.is_hv:
                                    cf_hname = cf.hfname
                                else:
                                    cf_hname = cf_get_hname(cf)
                                if cf_hname not in fields_of_same_header_in_byte.keys():
                                    fields_of_same_header_in_byte[cf_hname] = containerstart
                                cs = fields_of_same_header_in_byte[cf_hname]
                                if kbit+cs not in ki_or_kd_to_cf_map.keys():
                                    ki_or_kd_to_cf_map[kbit+cs] = [(cf, cf_startbit, width, "K")]
                                    total_p4_fld_width += width
                                else:
                                    # Incase of field union, 2 key sub-bits can map
                                    # same km sub-bit position.
                                    if (cf, cf_startbit, width, "K") not in ki_or_kd_to_cf_map[kbit+cs]:
                                        ki_or_kd_to_cf_map[kbit+cs].append((cf, cf_startbit, width, "K"))
                                        total_p4_fld_width += width

                        put_pad = False
                        if not field_found and is_tcam:
                            put_pad = True
                        elif not field_found and not kd:
                            put_pad = True
                        if put_pad:
                            containerstart, cf_startbit, width = v
                            #cf_hname = "__Other"
                            cf_hname = k.split('.')[0]
                            if cf_hname not in fields_of_same_header_in_byte.keys():
                                fields_of_same_header_in_byte[cf_hname] = containerstart
                            cs = fields_of_same_header_in_byte[cf_hname]
                            if kbit+cs not in ki_or_kd_to_cf_map.keys():
                                if cs >= total_p4_fld_width:
                                    ki_or_kd_to_cf_map[kbit+cs] = [(None, kbit+containerstart, width, "P", cf_hname)]
                                    total_p4_fld_width += width
                            else:
                                if (None, kbit+containerstart, width, "P", cf_hname) not in ki_or_kd_to_cf_map[kbit+cs]:
                                    ki_or_kd_to_cf_map[kbit+cs].append((None, kbit+containerstart, width, "P", cf_hname))
                                    total_p4_fld_width += width

                    # Since a list of p4fields shared same phv byte, get max field width and
                    # if it is less than 8 bits, then pad remaining bits
                    if total_p4_fld_width < 8:
                        put_pad = False
                        if is_tcam or not kd:
                            put_pad = True
                        if put_pad:
                            cs = total_p4_fld_width # Set container start bit from where padding is needed
                            if kbit+cs not in ki_or_kd_to_cf_map.keys():
                                ki_or_kd_to_cf_map[kbit+cs] = [(None, kbit+cs, (8 - total_p4_fld_width), "P", "__NoHdr")]

            # Pad each KM instance so as to cover unused bytes
            for km_byte_pos in range(len(km_cprofile.byte_sel), 32):
                kbit_ = (km_inst * (km_width/8) + km_byte_pos) * 8
                if is_tcam:
                    ki_or_kd_to_cf_map[kbit_] = [(None, kbit_, 8, "P", "__NoHdr")]
                elif not kd:
                    ki_or_kd_to_cf_map[kbit_] = [(None, kbit_, 8, "P", "__NoHdr")]

        # remove any duplicate cf that are extracted as bit as well as
        # byte or multibit in a byte
        self.remove_duplicate_cfs(ctable, ki_or_kd_to_cf_map, bit_extractors, is_tcam)
        # purge pad bits that appeared with in a union and outside union
        self.purge_duplicate_pad(ki_or_kd_to_cf_map)


        #Before returning ki_kd map, prepend duplicate K or I appearing in different
        #bit position in KM with "spurious".. This is needed for assembler to not
        #error out. At the same time, KM logic has freedom to populate same field
        #more than once in KM if needed to save/share KM etc.
        self.spurious_ki_fix(ki_or_kd_to_cf_map)

        return ki_or_kd_to_cf_map



    def asm_contiguous_field_chunk_get(self, kidict, index, field_elem=0, fields=1):
        k, v = kidict.items()[index]
        p4f_size = v[field_elem][2]
        done = False
        i = index
        while not done:
            if (i + 1) == len(kidict):
                done = True
            else:
                k1, v1 = kidict.items()[i]
                k2, v2 = kidict.items()[i + 1]
                if len(v2) != fields:
                    done = True
                    break
                if field_elem >= len(v2):
                    done = True
                else:
                    for z in range(fields):
                        #if both fields are valid
                        if v1[z][0] != None and v2[z][0] != None:
                            if v1[z][0] != v2[z][0]:
                                done = True
                                break
                        elif (v1[z][0] != None and v2[z][0] == None) \
                             or (v1[z][0] == None and v2[z][0] != None):
                            done = True
                            break
                        elif (v1[z][0] == None and v2[z][0] == None):
                            if (v1[z][4] != v2[z][4]):
                                done = True
                                break
                    if done:
                        break
                    else:
                        i += 1
                        p4f_size += v2[field_elem][2]

        return p4f_size, (i + 1 - index)

    def build_table_asm_fields_helper(self, cf_value_list, p4f_size, asm_field_info, key_position, ctable):
        flit_sz = self.be.hw_model['phv']['flit_size']
        pad = ''
        if len(cf_value_list) == 1:

            if cf_value_list[0][0] == None:
                cf, cf_startbit, width, ftype, hdr = cf_value_list[0]
                # Pad byte/bit
                hdr = hdr.replace('[', '_').replace(']', '_')
                asm_field_info.append(("__pad_" + hdr + "_" + str(cf_startbit), "__pad_" + hdr + "_" + str(cf_startbit), \
                                      p4f_size, 0, 0, 0, ftype, hdr))
                return

            cf, cf_startbit, width, ftype = cf_value_list[0]
            if p4f_size == -1:
                p4f_size = width

            p4fldname, _ = self.cfield_get_p4name_width(cf)
            if CHECK_INVALID_C_VARIABLE.search(p4fldname):
                p4fldname = \
                        self.convert_p4fldname_to_valid_c_variable(p4fldname)

            if p4f_size != cf.width:
                # Not entire p4 input field is contiguous.
                # More than one input field name will have to be created
                # Such p4fields created will be named following the pattern
                # <p4fldname>_<startbit>_<endbit>
                p4fldname = p4fldname + '_sbit' + str(cf_startbit) + \
                            '_ebit' + str(cf_startbit + p4f_size - 1)

            # Duplicate cf is possible in shared KM when
            # same key is used in 2 tables and KM1 and KM0
            # has key
            # We can same field in KM as I (Other) and K for this field
            # Explictly check for both I and K
            if (p4fldname, _get_output_name(cf.hfname), \
                                 p4f_size, cf.phv_bit, \
                                 cf.phv_bit / flit_sz, cf.phv_bit % flit_sz,\
                                 "I", cf_get_hname(cf)) in asm_field_info \
                or \
                (p4fldname, _get_output_name(cf.hfname), \
                                 p4f_size, cf.phv_bit, \
                                 cf.phv_bit / flit_sz, cf.phv_bit % flit_sz,\
                                 "K", cf_get_hname(cf)) in asm_field_info:
                if key_position < ctable.start_key_off or \
                            	key_position > ctable.end_key_off or self.be.args.p4_plus:
                    return
                else:
                    pad = '__pad_'
            asm_field_info.append((pad + p4fldname, pad + _get_output_name(cf.hfname), \
                                 p4f_size, cf.phv_bit, \
                                 cf.phv_bit / flit_sz, cf.phv_bit % flit_sz,\
                                 ftype, cf_get_hname(cf)))

    def build_table_ki_asm_fields(self, ctable):
        '''
        '''
        kidict = self.build_table_asm_hwformat(ctable)
        ki_field_info = []
        i = 0
        while i < len(kidict):
            # Contiguous chunk of bytes in KM could be sourced
            # from same p4 input field.
            k, cf_value_list = kidict.items()[i]
            if len(cf_value_list) == 1:
                k_p4f_size, cb = self.asm_contiguous_field_chunk_get(kidict, i, field_elem=0, fields=1)
                self.build_table_asm_fields_helper(cf_value_list, k_p4f_size, ki_field_info, k, ctable)
                i += cb
            else:
                #handle union
                num_fields_in_each_byte = len(cf_value_list)
                k_p4f_size = {}
                dict_lines = {}
                for f in range(num_fields_in_each_byte):
                    k_p4f_size[f], dict_lines[f] = \
                        self.asm_contiguous_field_chunk_get(kidict, i, field_elem=f, fields=len(cf_value_list))

                un_ki_field_info = []
                for u in range(len(cf_value_list)):
                    cflist = []
                    if cf_value_list[u][0] == None:
                        _cf, _cf_start, _width, _t, hdr = cf_value_list[u]
                        cflist.append((_cf, _cf_start, _width, _t, hdr))
                    else:
                        _cf, _cf_start, _width, _t = cf_value_list[u]
                        cflist.append((_cf, _cf_start, _width, _t))
                    self.build_table_asm_fields_helper(cflist, k_p4f_size[u], un_ki_field_info, k, ctable)
                    dict_lines[f] = -1
                ki_field_info.append(("unionized", un_ki_field_info))
                i += dict_lines[0]


        return ki_field_info

    def build_table_kd_asm_fields(self, ctable):
        '''
        '''
        kddict = self.build_table_asm_hwformat(ctable, kd=1)
        kd_field_info = []
        i = 0
        while i < len(kddict):
            # Contiguous chunk of bytes in KM could be sourced
            # from same p4 input field.
            k_p4f_size, cb = self.asm_contiguous_field_chunk_get(kddict, i)
            k, cf_value_list = kddict.items()[i]
            self.build_table_asm_fields_helper(cf_value_list, k_p4f_size, kd_field_info, k, ctable)
            i += cb
        return kd_field_info

    def build_pd_dict(self):
        '''
        For each table in p4 build a dict of
        {table-name: [List of Table-Keys]}
           Each Table-Key is (p4_field_name, length of field in bits)
        '''
        tbl_mgr = self.be.tables

        #Collect all tables in both ingress/egress direction
        for d in xgress:
            for table_name in tbl_mgr.gress_tm[d].tables:
                if table_name not in self.alltables.keys():
                    ctable = tbl_mgr.gress_tm[d].tables[table_name]
                    self.alltables[table_name] = ctable

        alltables = OrderedDict()
        for table_name, ctable in self.alltables.items():
            tdict = {}
            tdict['include_k_in_d'] = True if 'include_k_in_d' in ctable.p4_table._parsed_pragmas else False
            # Build input field list needed to generate
            # MPU ASM structures. For ASM KI / K , K will
            # be built using tdict[keys/not_my_key_bytes/bits]
            tdict['asm_ki_fields'] = self.build_table_ki_asm_fields(ctable)
            tdict['asm_kd_fields'] = self.build_table_kd_asm_fields(ctable)
            # Build information to generate p4pd APIs
            keydict = self.build_table_k_fields(ctable)
            # Table keys that are not unionized
            tdict['keys'] = keydict['keys']
            # Table keys that are field unionized. When more than one table keys
            # are unionized, such table keys are exclusive in use and will be
            # part of union data structure.
            tdict['fldunion_keys'] = keydict['fld_u_keys']
            # Table keys that are header unionized. When more than one table keys
            # are unionized, such table keys are exclusive in use and will be
            # part of union data structure.
            tdict['hdrunion_keys'] = keydict['hdr_u_keys']
            tdict['keysize'] = keydict['keysize']
            tdict['match_key_start_byte'] = keydict['match_key_start_byte']
            tdict['match_key_start_bit'] = keydict['match_key_start_bit']
            tdict['match_key_bit_length'] = keydict['match_key_len']
            tdict['is_wide_key'] = 1 if ctable.is_wide_key else 0
            tdict['wide_key_len'] = keydict['wide_key_len']
            # Not my key bytes and bits will NOT be NULL when
            # KeyMaker is shared by 2 TCAM tables. KM logic
            # can interperse Key bytes/bits to adhere to Banyan
            # n/w restriction.
            tdict['not_my_key_bytes'] = keydict['not_my_key_bytes']
            tdict['not_my_key_bits'] = keydict['not_my_key_bits']

            # List of (action-name, list of action-data-fields)
            # one per p4 action.
            tdict['actions'] = keydict['actions']
            tdict['le_action_params'] = ctable.le_action_params

            # Indicates where table resides.
            tdict['location'] = 'HBM' if ctable.is_hbm else 'P4Pipe'

            # Table match lookup type.
            if ctable.match_type == match_type.TERNARY \
               or ctable.match_type == match_type.TERNARY_ONLY:
                tdict['type'] = 'Ternary'
            elif ctable.match_type == match_type.EXACT_HASH_OTCAM:
                tdict['type'] = 'Hash_OTcam'
            elif ctable.match_type == match_type.EXACT_IDX:
                tdict['type'] = 'Index'
            elif ctable.match_type == match_type.MPU_ONLY:
                tdict['type'] = 'Mpu'
            elif ctable.match_type == match_type.EXACT_HASH:
                tdict['type'] = 'Hash'
            if ctable.is_toeplitz_hash():
                tdict['is_toeplitz'] = True
            else:
                tdict['is_toeplitz'] = False

            if ctable.collision_ct:
                tdict['hash_overflow_tbl'] = ctable.collision_ct.p4_table.name
            else:
                tdict['hash_overflow_tbl'] = ''
            if ctable.is_overflow and not ctable.is_otcam:
                tdict['hash_overflow'] = True
            else:
                tdict['hash_overflow'] = False
            tdict['otcam'] = ctable.is_otcam
            if ctable.is_otcam:
                tdict['parent_hash_table_size'] = ctable.hash_ct.num_entries

            tdict['direction'] = "INGRESS" if ctable.d == xgress.INGRESS else "EGRESS"
            if self.be.args.p4_plus:
                tdict['is_raw'] = ctable.is_raw
            else:
                tdict['is_raw'] = False

            alltables[table_name] = tdict

        self.pddict['tables'] = alltables
        self.pddict['p4plus'] = 1 if self.be.args.p4_plus else 0
        if self.be.args.p4_plus:
            self.pddict['p4plus'] = 1
            self.pddict['p4program'] = self.be.prog_name
        else:
            self.pddict['p4plus'] = 0
            self.pddict['p4program'] = self.be.prog_name

        self.pddict['cli-name'] = self.be.prog_name
        self.pddict['egr-hdrs'] = self.be.parsers[xgress.EGRESS].headers

    def verify_table_ki(self):
        '''
        Iterate over all p4 tables and check if every table key/input is in pddict
        '''
        for table_name, ctable in self.alltables.items():
            ki = self.pddict['tables'][table_name]['asm_ki_fields']
            # Check for duplicates within table key/input
            kidict = {}
            for fields in ki:
                if len(fields) == 2:
                    ustr, uflds = fields
                    for _fields in uflds:
                        split_p4fldname, p4fldname, p4fldwidth, \
                        phvbit, flit, flitoffset, typestr, hdrname = _fields
                        if split_p4fldname not in kidict.keys():
                            kidict[split_p4fldname] = 1
                        else:
                            assert(0), pdb.set_trace()
                            print ctable, split_p4fldname
                else:
                    split_p4fldname, p4fldname, p4fldwidth, \
                    phvbit, flit, flitoffset, typestr, hdrname = fields
                    if split_p4fldname not in kidict.keys():
                        kidict[split_p4fldname] = 1
                    else:
                        assert(0), pdb.set_trace()
                        print ctable, split_p4fldname

        # Check every table key and input is in ASM KI
        verified_cfk= []
        verified_cfi= []
        notfound_cfk= []
        notfound_cfi= []
        for table_name, ctable in self.alltables.items():
            ki = self.pddict['tables'][table_name]['asm_ki_fields']
            for cfkey in ctable.keys:
                if CHECK_INVALID_C_VARIABLE.search(cfkey[0].hfname):
                    cfkname = \
                            self.convert_p4fldname_to_valid_c_variable(cfkey[0].hfname)
                found = False
                for fields in ki:
                    if len(fields) == 2:
                        ustr, uflds = fields
                        for _fields in uflds:
                            split_p4fldname, p4fldname, p4fldwidth, \
                            phvbit, flit, flitoffset, typestr, hdrname = _fields
                            if cfkname == p4fldname:
                                found = True
                                break
                        if found:
                            break
                    else:
                        split_p4fldname, p4fldname, p4fldwidth, \
                        phvbit, flit, flitoffset, typestr, hdrname = fields
                        if cfkname == p4fldname:
                            found = True
                            break
                if not found:
                    if not ctable.is_wide_key:
                        notfound_cfk.append((ctable.d, table_name, cfkname))
                else:
                    verified_cfk.append(cfkname)

            for cfi in ctable.input_fields:
                if CHECK_INVALID_C_VARIABLE.search(cfi.hfname):
                    cfiname = \
                            self.convert_p4fldname_to_valid_c_variable(cfi.hfname)
                found = False
                for fields in ki:
                    if len(fields) == 2:
                        ustr, uflds = fields
                        for fields in uflds:
                            split_p4fldname, p4fldname, p4fldwidth, \
                            phvbit, flit, flitoffset, typestr, hdrname = fields
                            if cfiname == p4fldname:
                                found = True
                                break
                        if found:
                            break
                    else:
                        split_p4fldname, p4fldname, p4fldwidth, \
                        phvbit, flit, flitoffset, typestr, hdrname = fields
                        if cfiname == p4fldname:
                            found = True
                            break
                if not found:
                    if not ctable.is_wide_key:
                        notfound_cfi.append((ctable.d, table_name, cfiname))
                else:
                    verified_cfi.append(cfiname)

        if len(notfound_cfk) or len(notfound_cfi):
            assert(0), pdb.set_trace()
            pass

    def generate_code(self):
        gen_dir = self.be.args.gen_dir
        h_outputdir = gen_dir + '/%s/include' % (self.be.prog_name)
        c_outputdir = gen_dir + '/%s/src/' % (self.be.prog_name)
        py_outputdir = gen_dir + '/%s/cli/' % (self.be.prog_name)
        cli_outputdir = gen_dir + '/common/cli/'

        if not os.path.exists(h_outputdir):
            os.makedirs(h_outputdir)
        if not os.path.exists(c_outputdir):
            os.makedirs(c_outputdir)
        if not os.path.exists(py_outputdir):
            os.makedirs(py_outputdir)
        if not os.path.exists(cli_outputdir):
            os.makedirs(cli_outputdir)

        cur_path = os.path.abspath(__file__)
        cur_path = os.path.split(cur_path)[0]
        templatedir = os.path.join(cur_path, 'pd_templates/')

        if self.be.args.p4_plus:
            prog_name = self.be.prog_name
        else:
            prog_name = ''

        cli_outputdir_map = {}
        cli_outputdir_map['default'] = py_outputdir
        cli_outputdir_map['common']  = cli_outputdir

        p4pd_generate_code(self.pddict, templatedir, h_outputdir, c_outputdir, cli_outputdir_map, prog_name, gen_dir)

        outputdir = gen_dir + '/%s/asm_out' % (self.be.prog_name)
        if not os.path.exists(outputdir):
            os.makedirs(outputdir)
        cur_path = os.path.abspath(__file__)
        cur_path = os.path.split(cur_path)[0]
        templatedir = os.path.join(cur_path, 'asm_templates/')
        kd_dict = p4pd_generate_asm_code(self.pddict, templatedir, outputdir, None, None, '')
        return kd_dict

    def generate_swig(self):
        capri_p4pd_create_swig_interface(self.be)
        capri_p4pd_create_bazel_build(self.be)
        capri_p4pd_create_swig_custom_hdr(self.be)

def capri_p4pd_code_generate(capri_be):
    p4pd = capri_p4pd(capri_be)
    p4pd.build_pd_dict()
    p4pd.verify_table_ki()
    #with open('/tmp/pddict.json', "w") as of:
    #    json.dump(p4pd.pddict, of, indent=2)
    #    of.close()
    k_plus_d_dict = p4pd.generate_code()
    p4pd.generate_swig()

    return k_plus_d_dict
