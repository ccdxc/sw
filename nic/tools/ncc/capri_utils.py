#!/usr/bin/python
#
# Capri-Non-Compiler-Compiler (capri-ncc)
# Parag Bhide (Pensando Systems)

import os
import sys
import pdb
import json
from enum import IntEnum
from enum import Enum
from capri_logging import ncc_assert as ncc_assert

from p4_hlir.main import HLIR
import p4_hlir.hlir.p4 as p4

class xgress(IntEnum):
    INGRESS = 0,
    EGRESS = 1,

class meta_op(Enum):
    EXTRACT_FIELD   = 0
    EXTRACT_REG     = 1
    EXTRACT_META    = 2
    EXTRACT_CONST   = 3
    LOAD_REG        = 4
    UPDATE_REG      = 5
    EXTRACT_CURRENT_OFF = 6

class lkp_reg_type(IntEnum):
    LKP_REG_NONE    = 0
    LKP_REG_PKT     = 1
    LKP_REG_STORED  = 2
    LKP_REG_RETAIN  = 3
    LKP_REG_LOAD    = 4
    LKP_REG_UPDATE  = 5

class match_type(Enum):
    NONE                = 0
    MPU_ONLY            = 1
    EXACT_HASH          = 2
    TERNARY_ONLY        = 3     # no sram
    TERNARY             = 4     # w/ sram
    EXACT_HASH_OTCAM    = 5
    EXACT_IDX           = 6

class memory_type(Enum):
    SRAM           = 0
    TCAM           = 1
    HBM            = 2

class lkp_fld_type(IntEnum):
    FLD_NONE        = 0
    FLD_PKT         = 1
    FLD_SAVED_REG   = 2
    FLD_LOCAL       = 3

class memory_base_addr(IntEnum):
    HBM             = 0x0C0000000

class ohi_write_only_type(IntEnum):
    OHI_WR_ONLY_TYPE_HDR_OFFSET     = 0
    OHI_WR_ONLY_TYPE_HDR_LEN        = 1
    OHI_WR_ONLY_TYPE_OTHER          = 2

def get_i2e_fname(cf):
    return '_i2e_meta__' + cf.hfname.replace('.','_')

def cf_get_hname(cfld):
    hname = 'NOHEADER'
    if cfld and cfld.p4_fld and cfld.p4_fld.instance:
        hname = cfld.p4_fld.instance.name
    return hname

def get_hfname(p4fld):
    hname = 'NOHEADER'
    if p4fld.instance:
        hname = p4fld.instance.name
    return hname + '.' + p4fld.name

def get_header_max_size(hdr):
    # Bytes
    if not isinstance(hdr.header_type.length, int):
        return hdr.header_type.max_length
    else:
        return hdr.header_type.length 

def get_header_size(hdr):
    # return fixed len or P4field/expression that represents len
    # Bytes
    return hdr.header_type.length

def header_is_variable_len(hdr):
    return True if not isinstance(hdr.header_type.length, int) else False

def get_header_fixed_size(hdr):
    # return size of the fixed portion of the header i.e. exclude the variable len field at the
    # end
    hsize = 0
    for f in reversed(hdr.fields):
        if f.width == p4.P4_AUTO_WIDTH:
            continue
        hsize = f.offset + f.width
        break
    return (hsize / 8)  # Bytes

def hdr_is_intrinsic(hdr):
    if hdr.name == 'capri_intrinsic' or hdr.name == 'capri_p4_intrinsic':
        return True
    return False

def log2size(size):
    chk_entry_size = 1
    lg2entry_size = 0
    while chk_entry_size < size:
        chk_entry_size <<= 1
        lg2entry_size += 1
    return lg2entry_size

def is_synthetic_header(h):
    if h._parsed_pragmas and 'synthetic_header' in h._parsed_pragmas:
        return True
    return False

def is_parser_local_header(h):
    if h._parsed_pragmas and 'pa_parser_local' in h._parsed_pragmas or \
        h._parsed_pragmas and 'parser_write_only' in h._parsed_pragmas:
        return True
    return False

def is_atomic_header(h):
    if h._parsed_pragmas and 'pa_atomic' in h._parsed_pragmas:
        return True
    return False

def get_header_alignment(h):
    # align header to specified boundary
    supported_alignments = [8, 16, 32, 64, 128, 256, 512, 1024]
    if h._parsed_pragmas and 'pa_align' in h._parsed_pragmas:
        alignment = int(h._parsed_pragmas['pa_align'].keys()[0])
        if alignment in supported_alignments:
            return alignment
    if h._parsed_pragmas and 'pa_atomic' in h._parsed_pragmas: 
        return 8
    return 0 # no alignment restrictions

def get_pragma_param_list(parsed_pragmas):
    # convert recursive dictionaries into a simple list
    param_list = []

    nxt_pdict = parsed_pragmas
    while len(nxt_pdict) != 0:
        nxt_param = nxt_pdict.keys()[0]
        param_list.append(nxt_param)
        nxt_pdict = nxt_pdict[nxt_param]
    return param_list

def is_scratch_header(h):
    return True if 'scratch_metadata' in h._parsed_pragmas else False

def log2(x):
    ncc_assert((x != 0))
    log = log2size(x)
    ncc_assert(x == 1 << log, 'Only log of powers of 2 allowed')
    return log

def pad_to_power2(x):
    return x if x == 0 else (0x01 << log2size(x))

def pad_to_x(value, x):
    return (((value + (x-1)) / x) * x)

def pad_to_64(value):
    return (((value + 63) / 64) * 64)

def pad_to_16(value):
    return (((value + 15) / 16) * 16)

def pad_to_8(value):
    return (((value + 7) / 8) * 8)

def get_block_width(type):
    return 8 if type == 'sram' else \
        8 if type == 'tcam' else \
            1 if type == 'hbm'  else \
                1

def match_type_to_string(type):
    return 'mpu' if type == match_type.MPU_ONLY else \
           'hash' if type == match_type.EXACT_HASH else \
           'tcam' if type == match_type.TERNARY_ONLY else \
           'indexed' if type == match_type.EXACT_IDX else \
           'hash_tcam' if type == match_type.EXACT_HASH_OTCAM else \
           'tcam_sram' if type == match_type.TERNARY else \
           'unknown'

def memory_type_to_string(type):
    return 'hbm'  if type == memory_type.HBM else \
           'sram' if type == memory_type.SRAM else \
           'tcam' if type == memory_type.TCAM else \
           'unknown'

def xgress_to_string(x):
    return 'ingress' if x == xgress.INGRESS else \
           'egress'  if x == xgress.EGRESS else \
           'unknown'

def xgress_from_string(x):
    return xgress.INGRESS if x == 'ingress' else \
           xgress.EGRESS  if x == 'egress' else 0

def table_width_to_allocation_units(type, width):
    return pad_to_16(width) / 16 if type == 'sram' else \
           pad_to_16(width) / 16 if type == 'tcam' else \
           pad_to_16(width) /  8 if type == 'hbm'  else \
           width

def allocation_units_to_table_width(type, units):
    return units * 16 if type == 'sram' else \
           units * 16 if type == 'tcam' else \
           units *  8 if type == 'hbm'  else \
           units

def allocation_units_to_num_blocks(type, width):
    return pad_to_8(width) / 8 if type == 'sram' else \
           pad_to_8(width) / 8 if type == 'tcam' else \
           pad_to_8(width) / 8 if type == 'hbm'  else \
           width

def blocks_to_allocation_units(type, width):
    return width * 8 if type == 'sram' else \
           width * 8 if type == 'tcam' else \
           width * 8 if type == 'hbm'  else \
           width

def depth_to_num_blocks(type, depth):
    return pad_to_x(depth, 4096) / 4096 if type == 'sram' else \
           pad_to_x(depth, 1024) / 1024 if type == 'tcam' else \
           depth

def blocks_to_depth(type, depth):
    return depth * 4096 if type == 'sram' else \
           depth * 1024 if type == 'tcam' else \
           depth

def capri_get_width_from_layout(layout):
    return 0 if not layout else (((layout['bottom_right']['block'] * 8) + layout['bottom_right']['x']) - \
                                 ((layout['top_left']['block'] * 8) + layout['top_left']['x']) + 1)

def capri_get_depth_from_layout(layout):
    return 0 if not layout else (layout['bottom_right']['y'] - layout['top_left']['y'] + 1)

def capri_get_num_bkts_from_layout(layout, entry_width):
    return 0 if entry_width == 0 else (capri_get_width_from_layout(layout) / entry_width)

def capri_get_log2bkts_from_layout(layout, entry_width):
    num_bkts = capri_get_num_bkts_from_layout(layout, entry_width)
    return 0 if num_bkts == 0 else log2(num_bkts)

def capri_get_sram_hw_address_from_coordinate(coord):
    return 0 if not coord else (coord['y'] * 8 * 16) + (coord['block'] * 8) + (coord['x'])

def capri_get_sram_hw_start_address_from_layout(layout):
    return 0 if not layout else capri_get_sram_hw_address_from_coordinate(layout['top_left'])

def capri_get_sram_hw_end_address_from_layout(layout):
    return 0 if not layout else capri_get_sram_hw_address_from_coordinate(layout['bottom_right'])

def capri_get_sram_sw_address_from_coordinate(coord):
    return 0 if not coord else (coord['y'] * 8 * 10) + (coord['block'] * 8) + (coord['x'])

def capri_get_sram_sw_start_address_from_layout(layout):
    return 0 if not layout else capri_get_sram_sw_address_from_coordinate(layout['top_left'])

def capri_get_sram_sw_end_address_from_layout(layout):
    return 0 if not layout else capri_get_sram_sw_address_from_coordinate(layout['bottom_right'])

def capri_get_tcam_address_from_coordinate(coord):
    return 0 if not coord else (coord['y'] * 8 * 8) + (coord['block'] * 8) + (coord['x'])

def capri_get_tcam_start_address_from_layout(layout):
    return 0 if not layout else capri_get_tcam_address_from_coordinate(layout['top_left'])

def capri_get_tcam_end_address_from_layout(layout):
    return 0 if not layout else capri_get_tcam_address_from_coordinate(layout['bottom_right'])

def capri_get_hbm_start_address_from_layout(layout):
    return 0 if not layout else layout['top_left']['x']

def capri_get_hbm_end_address_from_layout(layout):
    return 0 if not layout else layout['bottom_right']['x']
