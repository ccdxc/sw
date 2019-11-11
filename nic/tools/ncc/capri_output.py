#!/usr/bin/python
#
# Capri-Non-Compiler-Compiler (capri-ncc)
# Parag Bhide (Pensando Systems)

import os
import re
import sys
import pdb
import struct
import logging
import copy
import pprint
import json
import math
import errno
from collections import OrderedDict
from enum import IntEnum
from capri_utils import *
from capri_logging import ncc_assert as ncc_assert

def _get_output_name(s):
    # XXX use regex or something better
    s1 = s.replace('].','_')
    s1 = s1.replace('.','_')
    s1 = s1.replace('[', '_')
    s1 = s1.replace(']', '_')
    return s1

def _parser_tcam_print(te):
    # print tcam entry
    pstr = ''
    pstr += 'valid = %s\n' % te['valid']['value']
    pstr += 'x_control(key) = %s, ' % te['key']['control']['value']
    pstr += 'y_control(mask) = %s, ' % te['mask']['control']['value']
    pstr += 'x_state = %s, ' % te['key']['state']['value']
    pstr += 'y_state = %s\n' % te['mask']['state']['value']
    for r,_ in enumerate(te['mask']['lkp_val']):
        pstr += 'x_lkp_val[%d] = %s, ' % (r, te['key']['lkp_val'][r]['value'])
        pstr += '\ty_lkp_val[%d] = %s\n' % (r, te['mask']['lkp_val'][r]['value'])
    return pstr

def _parser_sram_print(parser, se):
    pstr = '\n'
    pstr += 'action = %s, ' % se['action']['value']
    pstr += 'nxt_state = %s, ' % se['nxt_state']['value']
    pstr += 'offset_jump_chk_en = %s, ' % se['offset_jump_chk_en']['value']
    if se['pkt_size_chk_en']['value'] == '1':
        pstr += 'pkt_size_chk = %s, ' % se['pkt_size']['value']
    pstr += '\noffset_inst: sel %s, mux_sel %s, val %s ' % \
        (se['offset_inst']['sel']['value'], se['offset_inst']['muxsel']['value'],
         se['offset_inst']['val']['value'])
    pstr += '\n'

    used = 0
    for i, lkp_val_inst in enumerate(se['lkp_val_inst']):
        if lkp_val_inst['sel']['value'] == 'Default':
            continue
        used += 1
        if lkp_val_inst['sel']['value'] == '0':
            pstr += 'lkp_val_inst[%s] = mux_pkt[%s], store_en = %s\n' % \
                (i, lkp_val_inst['muxsel']['value'],
                lkp_val_inst['store_en']['value'])
        elif lkp_val_inst['sel']['value'] == '1':
            pstr += 'lkp_val_inst[%s] = mux_inst_data[%s], store_en = %s\n' % \
                (i, lkp_val_inst['muxsel']['value'],
                lkp_val_inst['store_en']['value'])
        elif lkp_val_inst['sel']['value'] == '2':
            pstr += 'lkp_val_inst[%s] = retain_old_val, store_en = %s\n' % \
                (i, lkp_val_inst['store_en']['value'])
        elif lkp_val_inst['sel']['value'] == '3':
            pstr += 'lkp_val_inst[%s] = stored_lkp_val, store_en = %s\n' % \
                (i, lkp_val_inst['store_en']['value'])
        else:
            pass
    if used:
        pstr += '\nTotal lkp_val_inst %s\n' % used

    used = 0
    meta_ops = parser.be.hw_model['parser']['parser_consts']['meta_ops']
    for e, meta_inst in enumerate(se['meta_inst']):
        if meta_inst['phv_idx']['value'] == 'Default':
            continue
        if meta_inst['sel']['value'] == meta_ops['set_mux_data'] or \
           meta_inst['sel']['value'] == meta_ops['or_mux_data']:
            pstr += 'meta_inst[%d]: phv[%s] %s mux_inst_data[%s]\n' % \
                    (e,
                    meta_inst['phv_idx']['value'],
                    '|=' if meta_inst['sel']['value'] == meta_ops['or_mux_data'] else '=',
                    meta_inst['val']['value'])
        elif meta_inst['sel']['value'] == meta_ops['set_val'] or \
           meta_inst['sel']['value'] == meta_ops['or_val']:
            pstr += 'meta_inst[%d]: phv[%s] %s %s\n' % \
                    (e,
                    meta_inst['phv_idx']['value'],
                    '|=' if meta_inst['sel']['value'] == meta_ops['or_val'] else '=',
                    meta_inst['val']['value'])
        elif meta_inst['sel']['value'] == meta_ops['set_hv']:
            pstr += 'meta_inst[%d]: hv_en %s |= %s\n' % \
                (e,
                meta_inst['phv_idx']['value'],
                meta_inst['val']['value'])
        else:
            continue
        used += 1
    if used:
        pstr += 'Total meta_inst %s\n' % used

    used = 0
    for e, ohi_inst in enumerate(se['ohi_inst']):
        if ohi_inst['sel']['value'] == '0':
            continue
        used += 1
        if ohi_inst['sel']['value'] == '1':
            pstr += 'ohi_inst[%s]: ohi[%s] = current_off + %s\n' % \
                (e,
                ohi_inst['slot_num']['value'],
                ohi_inst['idx_val']['value'])
        elif ohi_inst['sel']['value'] == '2':
            pstr += 'ohi_inst[%s]: ohi[%s] = %s\n' % \
                (e,
                ohi_inst['slot_num']['value'],
                ohi_inst['idx_val']['value'])
        elif ohi_inst['sel']['value'] == '3':
            pstr += 'ohi_inst[%s]: ohi[%s] = mux_inst[%s]\n' % \
                (e,
                ohi_inst['slot_num']['value'],
                ohi_inst['muxsel']['value'])
    if used:
        pstr += 'Total ohi_inst %s\n' % used

    used = 0
    for e, mux_idx in enumerate(se['mux_idx']):
        if mux_idx['sel']['value'] == 'Default':
            continue
        used += 1
        if mux_idx['sel']['value'] == '0':
            pstr += 'mux_idx[%d] = pkt[current_off + %s]\n' % \
                    (e, mux_idx['idx']['value'])
        elif mux_idx['sel']['value'] == '1':
            pstr += 'mux_idx[%d] = lkp_val[%s]\n' % \
                    (e, mux_idx['lkpsel']['value'])
        elif mux_idx['sel']['value'] == '2':
            pstr += 'mux_idx[%d] = mux_pkt[current_off + lkp_val[%s]]\n' % \
                    (e, mux_idx['lkpsel']['value'])
        elif mux_idx['sel']['value'] == '3':
            pstr += 'mux_idx[%d] = current_offset\n' % (e)
        else:
            pass
    if used:
        pstr += 'Total mux_idx %s\n' % used

    used = 0
    for e, mux_inst in enumerate(se['mux_inst']):
        if mux_inst['sel']['value'] == 'Default':
            continue
        used += 1
        if mux_inst['sel']['value'] == '0':
            pstr += 'mux_inst_data[%d] = ((mux_pkt[%s] & %s) %s %s) %s %s\n' % \
                    (e,
                    mux_inst['muxsel']['value'],
                    mux_inst['mask_val']['value'],
                    '<<' if mux_inst['shift_left']['value'] == '1' else '>>',
                    mux_inst['shift_val']['value'],
                    '+' if mux_inst['addsub']['value'] == '1' else '-',
                    mux_inst['addsub_val']['value'])
        else:
            if mux_inst['load_mux_pkt']['value'] == '0':
                pstr += 'mux_inst_data[%d] = lkp_val[%s] %s ((mux_pkt[%s] & %s) %s %s) %s %s\n' % \
                    (e,
                    mux_inst['lkpsel']['value'],
                    '+' if mux_inst['lkp_addsub']['value'] == '1' else '-',
                    mux_inst['muxsel']['value'],
                    mux_inst['mask_val']['value'],
                    '<<' if mux_inst['shift_left']['value'] == '1' else '>>',
                    mux_inst['shift_val']['value'],
                    '+' if mux_inst['addsub']['value'] == '1' else '-',
                    mux_inst['addsub_val']['value'])
            else:
                pstr += 'mux_inst_data[%d] = mux_pkt2[%s] %s ((mux_pkt[%s] & %s) %s %s) %s %s\n' % \
                    (e,
                    mux_inst['lkpsel']['value'],
                    '+' if mux_inst['lkp_addsub']['value'] == '1' else '-',
                    mux_inst['muxsel']['value'],
                    mux_inst['mask_val']['value'],
                    '<<' if mux_inst['shift_left']['value'] == '1' else '>>',
                    mux_inst['shift_val']['value'],
                    '+' if mux_inst['addsub']['value'] == '1' else '-',
                    mux_inst['addsub_val']['value'])

    if used:
        pstr += 'Total mux_inst %s\n' % used

    used = 0
    for e, extract_inst in enumerate(se['extract_inst']):
        if extract_inst['phv_idx']['value'] == 'Default':
            continue
        used += 1
        pstr += 'extract_inst[%s] = pkt(%s,%s) -> phv(%s), ' % \
                (e, extract_inst['pkt_idx']['value'],
                    extract_inst['len']['value'],
                    extract_inst['phv_idx']['value'])
    if used:
        pstr += '\nTotal extract_inst %s\n' % used

    for e,len_chk_inst in enumerate(se['len_chk_inst']):
        if len_chk_inst['en']['value'] == '0':
            continue
        pstr += 'len_chk_inst[%d] en=%s ' % (e, len_chk_inst['en']['value'])
        pstr += 'unit_sel=%s ' % (len_chk_inst['unit_sel']['value'])
        pstr += 'prof_sel= %s ' % (len_chk_inst['prof_sel']['value'])
        pstr += 'start_ohi= %s ' % (len_chk_inst['ohi_start_sel']['value'])
        pstr += 'len_ohi= %s ' % (len_chk_inst['ohi_len_sel']['value'])
        pstr += 'exact= %s\n' % (len_chk_inst['exact']['value'])
    return pstr

def _parser_init_profile_print(parser, cfg_init_profile):
    pstr = 'Parser init_profile\n'
    pstr += 'curr_offset = %s, ' % cfg_init_profile['curr_offset']['value']
    pstr += 'state = %s, ' % cfg_init_profile['state']['value']
    pstr += 'lkp_val_pkt_idx0 = %s, ' % cfg_init_profile['lkp_val_pkt_idx0']['value']
    pstr += 'lkp_val_pkt_idx1 = %s, ' % cfg_init_profile['lkp_val_pkt_idx1']['value']
    pstr += 'lkp_val_pkt_idx2 = %s\n' % cfg_init_profile['lkp_val_pkt_idx2']['value']
    return pstr

def _build_mux_inst2(parser, cs, mux_inst, rid1, rid2, mux_id1, mux_id2, _capri_expr, add_off=0):
    #capri_expr = copy.copy(_capri_expr)
    capri_expr = _capri_expr
    use_current = False # for special handling of add_offset with current_offset
    if capri_expr.op2:
        mux_inst['sel']['value'] = str(1)
        mux_inst['lkp_addsub']['value'] = str(1) if capri_expr.op2 == '+' else str(0)
        if mux_id2 != None:
            # when 'current' is used as src_reg operand
            # mux_id2 is already programmed to src current_offset
            if isinstance(capri_expr.src_reg, tuple) and \
                capri_expr.src_reg[1] == 0:
                use_current = True
            mux_inst['load_mux_pkt']['value'] = str(1)
            mux_inst['lkpsel']['value'] = str(mux_id2)
        else:
            ncc_assert(rid1 != None)
            mux_inst['load_mux_pkt']['value'] = str(0)
            mux_inst['lkpsel']['value'] = str(rid1)
    else:
        mux_inst['sel']['value'] = str(0)
        mux_inst['lkpsel']['value'] = str(0)         # NA
        mux_inst['lkp_addsub']['value'] = str(0)     # NA

    op1 = capri_expr.op1
    if capri_expr.op1:
        if rid2 != None:
            ncc_assert(mux_id1 != None)
            mux_inst['muxsel']['value'] = str(mux_id1)
            ncc_assert(not isinstance(capri_expr.src1, tuple) )# Cannot have tuple w/ rid2
            mask = (1<<capri_expr.src1.width) - 1
            mux_inst['mask_val']['value'] = str(mask)
            mux_inst['shift_val']['value'] = str(capri_expr.shft)
        else:
            ncc_assert(mux_id1 != None)
            mux_inst['muxsel']['value'] = str(mux_id1)
            # need to adjust mask and shift vals based on alignment
            # hw pull 16b from a specified byte boundary
            if isinstance(capri_expr.src1, tuple):
                # XXX is this offset correct or need to get it from lkp_reg??
                off = capri_expr.src1[0]
                eoff = off + capri_expr.src1[1]
            else:
                off = cs.fld_off[capri_expr.src1]
                eoff = off + capri_expr.src1.width
                ncc_assert(capri_expr.src1.width <= 16)
            # lookup fields > 16 bits are not currently supported. That requires concatenation
            # of multiple lkp regs - TBD
            c_soff = (off / 8) * 8
            c_eoff = ((eoff+7)/8) * 8
            # since the lkp regs are 16bits each, end offset must be aligned to register
            if c_soff != c_eoff:
                # soff and eoff are same when 'current' is used
                c_eoff = max(c_eoff, (c_soff + 16))

            if c_soff != off:
                # not aligned at the start of the container - nothing to do
                pass

            if capri_expr.mask:
                mask = capri_expr.mask
            else:
                if isinstance(capri_expr.src1, tuple):
                    if capri_expr.src1[1] == 0:
                        mask = 0xFFFF
                        use_current = True
                    else:
                        mask = (1<<capri_expr.src1[1]) - 1
                else:
                    mask = (1<<capri_expr.src1.width) - 1

            c_shft = c_eoff - eoff          # c_shft indicate >> to right justify the field
            if c_shft:
                # mask is shifted in opposite direction since the value being used is not
                # right justified, c_shft will right justify it.. but mask is applied before
                # the shift, so adjust the mask to the left
                mask <<= c_shft
            # Adjust the mask. Mask is applied before any shifting so it has to be moved to
            # align with the field prior to any shifting
            new_shft = 0
            if capri_expr.op1 == '>>':
                new_shft = capri_expr.shft + c_shft
            elif capri_expr.op1 == '<<':
                if c_shft > capri_expr.shft:
                    # reverse the direction
                    op1 = '>>'
                    new_shft = (c_shft - capri_expr.shft)
                else:
                    new_shft = capri_expr.shft - c_shft
            else:
                op1 = '>>'
                new_shft = capri_expr.shft + c_shft

            ncc_assert(new_shft >= 0 and new_shft < 16)
            mux_inst['mask_val']['value'] = str(mask)
            mux_inst['shift_val']['value'] = str(new_shft)
    else:
        mux_inst['muxsel']['value'] = str(0)
        mux_inst['mask_val']['value'] = str(0)     # mask off all bits
        mux_inst['shift_val']['value'] = str(0)


    addsub_val = capri_expr.const
    op3 = capri_expr.op3
    if use_current and add_off: # and addsub_val:
        # adjust the addsub value (testcase - gft program)
        if capri_expr.op3 == '-':
            delta_const = capri_expr.const - add_off
            if delta_const < 0:
                op3 = '+'
                addsub_val = (-1) * delta_const
            else:
                addsub_val = delta_const
        else:
            addsub_val += add_off
            op3 = '+'

    mux_inst['addsub_val']['value'] = str(addsub_val)
    mux_inst['addsub']['value'] = str(1) if op3 == '+' else str(0)

    # XXX h/w register name changed - remove this once move to new format is complete
    if 'shift_left' in mux_inst.keys():
        mux_inst['shift_left']['value'] = str(1) if op1 == '<<' else str(0)
    else:
        mux_inst['shift']['value'] = str(1) if op1 == '<<' else str(0)

def _build_mux_inst(parser, cs, rid, mux_inst, mux_id, _capri_expr):
    # DO NOT modify _capri_expr, this function is called multiple times for the same state
    capri_expr = copy.copy(_capri_expr)
    if capri_expr.op2:
        ncc_assert(rid != -1)
        mux_inst['sel']['value'] = str(1)
        mux_inst['lkpsel']['value'] = str(rid)
        mux_inst['lkp_addsub']['value'] = str(1) if capri_expr.op2 == '+' else str(0)
    else:
        mux_inst['sel']['value'] = str(0)
        mux_inst['lkpsel']['value'] = str(0)         # NA
        mux_inst['lkp_addsub']['value'] = str(0)     # NA

    if capri_expr.op1:
        # need to adjust mask and shift vals based on alignment
        # hw pull 16b from a specified byte boundary
        if isinstance(capri_expr.src1, tuple) and capri_expr.src1[1] == 0:
            # special case for using current_offset
            ncc_assert(not capri_expr.mask)
            mask = 0xFFFF
            off = 0
            eoff = 16
        else:
            if isinstance(capri_expr.src1, tuple):
                off = capri_expr.src1[0]
                eoff = capri_expr.src1[0] + capri_expr.src1[1]
                if capri_expr.mask:
                    mask = capri_expr.mask
                else:
                    mask = (1<<(eoff-off)) - 1
            else:
                off = cs.fld_off[capri_expr.src1]
                eoff = off + capri_expr.src1.width
                if capri_expr.mask:
                    mask = capri_expr.mask
                else:
                    mask = (1<<capri_expr.src1.width) - 1
            # lookup fields > 16 bits are not currently supported. That requires concatenation
            # of multiple lkp regs - TBD
            ncc_assert(capri_expr.src1.width <= 16)
        c_soff = (off / 8) * 8
        c_eoff = ((eoff+7)/8) * 8
        # since the lkp regs are 16bits each, end offset must be aligned to register
        c_eoff = max(c_eoff, (c_soff + 16))
        if c_soff != off:
            # not aligned at the start of the container - nothing to do
            pass

        c_shft = c_eoff - eoff          # c_shft indicate >> to right justify the field
        if c_shft:
            # mask is shifted in opposite direction since the value being used is not
            # right justified, c_shft will right justify it.. but mask is applied before
            # the shift, so adjust the mask to the left
            mask <<= c_shft
        # Adjust the mask. Mask is applied before any shifting so it has to be moved to
        # align with the field prior to any shifting
        if capri_expr.op1 == '>>':
            capri_expr.shft += c_shft
        elif capri_expr.op1 == '<<':
            if c_shft > capri_expr.shft:
                # reverse the direction
                capri_expr.op1 = '>>'
                capri_expr.shft = (c_shft - capri_expr.shft)
            else:
                capri_expr.shft -= c_shft
        else:
            capri_expr.op1 = '>>'
            capri_expr.shft += c_shft

        ncc_assert(capri_expr.shft >= 0 and capri_expr.shft < 16)
        mux_inst['muxsel']['value'] = str(mux_id)
        mux_inst['mask_val']['value'] = str(mask)
        mux_inst['shift_val']['value'] = str(capri_expr.shft)
    else:
        mux_inst['muxsel']['value'] = str(0)
        mux_inst['mask_val']['value'] = str(0)     # mask off all bits
        mux_inst['shift_val']['value'] = str(0)

    mux_inst['addsub_val']['value'] = str(capri_expr.const)
    # XXX name changed - remove this once move to new format is complete
    if 'shift_left' in mux_inst.keys():
        mux_inst['shift_left']['value'] = str(1) if capri_expr.op1 == '<<' else str(0)
    else:
        mux_inst['shift']['value'] = str(1) if capri_expr.op1 == '<<' else str(0)
    mux_inst['addsub']['value'] = str(1) if capri_expr.op3 == '+' else str(0)


def _get_phv_loc(flit_inst, loc, size):
    start = ((flit_inst * 512) - loc) + ((flit_inst + 1) * 512) - 1
    end = start - size + 1
    return "[" + str(start) + ":" + str(end) + "]"

def capri_cstruct_output_pa(gress_pa):
    capri_asm_output_pa(gress_pa, False)

def capri_asm_output_pa(gress_pa, asm_output=True):
    if gress_pa.pa.be.args.old_phv_allocation:
        return _capri_asm_output_pa(gress_pa)

    def cstruct_data_type_get(asm_output, width, indent):
        if asm_output:
            return indent
        if width < 32:
            return indent + 'uint32_t '
        if width < 64:
            return indent + 'uint64_t '
        return indent + 'uint8_t '

    def phv_width_string_get(asm_output, width):
        if asm_output or width < 64:
            return ' : ' + str(width)
        if width > 64:
            return '[' + str(width / 8) + ']'
        return ''


    class _phv_union:
        def __init__(self, uname, asm_output):
            self.streams = OrderedDict()
            self.name = uname
            self.end_phv = -1
            self.start_phv = -1
            self.asm_output = asm_output

        def add_cfld(self, strm_name, cf):
            if strm_name in self.streams:
                self.streams[strm_name].append(cf)
            else:
                self.streams[strm_name] = [cf]

            if self.end_phv < cf.phv_bit + cf.storage_size():
                self.end_phv = cf.phv_bit + cf.storage_size()

            if self.start_phv < 0:
                self.start_phv = cf.phv_bit

        def print_union(self):
            indent = '    '
            indent2 = indent+indent
            indent3 = indent+indent2
            pstr = indent + 'union {\n'
            for strm, cf_strm in self.streams.items():
                phv_bit = self.start_phv
                if self.asm_output:
                    pstr += indent2 + 'struct {\n'
                else:
                    pstr += indent2 + 'struct  __attribute__ ((__packed__)) {\n'
                cfields = []
                if not asm_output:
                    save_pstr = pstr
                for cf in cf_strm:
                    if phv_bit != cf.phv_bit:
                        # print pad
                        cstr = cstruct_data_type_get(asm_output, cf.phv_bit - phv_bit, indent3)
                        width_str = phv_width_string_get(asm_output, cf.phv_bit - phv_bit)
                        cstr += '_%s_pad_%d_%s; // FLE[%d:%d]\n' % \
                            (strm, phv_bit, width_str,
                            _phv_bit_flit_le_bit(phv_bit), _phv_bit_flit_le_bit(cf.phv_bit-1))
                        pstr += cstr
                        cfields.append(cstr)
                    cstr = cstruct_data_type_get(asm_output, cf.width, indent3)
                    width_str = phv_width_string_get(asm_output, cf.width)
                    cstr += '%s%s; // BE[%d] FLE[%d:%d]\n' % \
                        (_get_output_name(cf.hfname), width_str, cf.phv_bit,
                        _phv_bit_flit_le_bit(cf.phv_bit),
                        _phv_bit_flit_le_bit(cf.phv_bit+cf.width-1))
                    pstr += cstr
                    cfields.append(cstr)
                    phv_bit += cf.storage_size()
                # add pad to bottom-align unions as needed by capas
                bottom_pad = self.end_phv - phv_bit
                if bottom_pad > 0:
                    cstr = cstruct_data_type_get(asm_output, bottom_pad, indent3)
                    width_str = phv_width_string_get(asm_output, bottom_pad)
                    cstr += '%s_bottom_pad_%d_%s; // FLE[%d:%d]\n' % \
                        (_get_output_name(strm), phv_bit, width_str,
                        _phv_bit_flit_le_bit(phv_bit), _phv_bit_flit_le_bit(self.end_phv-1))
                    pstr += cstr
                    cfields.append(cstr)

                if not asm_output:
                    for _pstr in reversed(cfields):
                        save_pstr += _pstr
                    pstr = save_pstr

                pstr += indent2 + '};\n'
            pstr += indent + '};\n'
            #pdb.set_trace()
            return pstr


    gen_dir = gress_pa.pa.be.args.gen_dir
    if not asm_output:
        cur_path = gen_dir + '/%s/include' % gress_pa.pa.be.prog_name
        if not os.path.exists(cur_path):
            try:
                os.makedirs(cur_path)
            except OSError as e:
                if e.errno != errno.EEXIST:
                    raise
        fname = cur_path + '/%s_phv.h' % gress_pa.d.name.lower()
    else:
        cur_path = gen_dir + '/%s/asm_out' % gress_pa.pa.be.prog_name
        if not os.path.exists(cur_path):
            try:
                os.makedirs(cur_path)
            except OSError as e:
                if e.errno != errno.EEXIST:
                    raise
        fname = cur_path + '/%s_p.h' % gress_pa.d.name
    hfile = open(fname, 'w')
    num_flits = gress_pa.pa.be.hw_model['phv']['num_flits']
    pstr_flit = {}
    for i in range (0, num_flits):
        pstr_flit[i] = ''

    indent='    '
    indent2=indent+indent
    indent3=indent2+indent
    output_hdr_unions = {}

    flit_sz = gress_pa.pa.be.hw_model['phv']['flit_size']
    fid = 0
    cfields = []
    phv_bit = 0
    pstr = indent + '//----- Flit %d -----\n' % fid
    flit_cstr = pstr
    active_union = None
    for cf in gress_pa.field_order:
        if cf.width == 0 or cf.is_ohi:
            continue

        if (cf.phv_bit / flit_sz) != fid:
            # it is possible to have a pad at the start of new flit where next cf does not
            # start at the start of the flit, in that case part of pad goes at the end of previous
            # flit and part at the start of new flit
            # XXX completely empty flits are not handled
            flit_start = ((fid+1) * flit_sz)
            flit_end_pad = flit_start - phv_bit
            flit_start_pad = cf.phv_bit - flit_start
            if active_union:
                # print union...
                cstr = active_union.print_union()
                pstr += cstr
                cfields.append(cstr)
                phv_bit = active_union.end_phv
                active_union = None

            ncc_assert(flit_end_pad >= 0)
            if flit_end_pad > 0:
                cstr = cstruct_data_type_get(asm_output, flit_end_pad, indent)
                width_str = phv_width_string_get(asm_output, flit_end_pad)
                cstr += '_flit_%d_pad_%d_%s; // BE[%d] FLE[%d:%d]\n' % \
                    (fid, phv_bit, width_str, phv_bit,
                    _phv_bit_flit_le_bit(phv_bit), _phv_bit_flit_le_bit(cf.phv_bit-1))
                pstr += cstr
                cfields.append(cstr)
            phv_bit = cf.phv_bit
            if not asm_output:
                for _pstr in reversed(cfields):
                    flit_cstr += _pstr
                pstr = flit_cstr
            pstr_flit[fid] = copy.copy(pstr)
            cfields = []
            fid += 1
            #pdb.set_trace()
            pstr = indent + '//----- Flit %d -----\n' % fid
            flit_cstr = pstr
            if flit_start_pad > 0:
                cstr = cstruct_data_type_get(asm_output, flit_start_pad, indent)
                width_str = phv_width_string_get(asm_output, flit_start_pad)
                cstr += '_flit_%d_start_pad_%d_%s; // BE[%d] FLE[%d:%d]\n' % \
                    (fid, phv_bit, width_str, flit_start,
                    _phv_bit_flit_le_bit(flit_start), _phv_bit_flit_le_bit(cf.phv_bit-1))
                pstr += cstr
                cfields.append(cstr)

        if not cf.is_union_storage() and not cf.is_union():
            if active_union:
                # print union
                cstr = active_union.print_union()
                pstr += cstr
                cfields.append(cstr)
                phv_bit = active_union.end_phv
                active_union = None
            # check for gaps
            if phv_bit != cf.phv_bit:
                ncc_assert((cf.phv_bit - phv_bit) >= 0)
                cstr = cstruct_data_type_get(asm_output, cf.phv_bit - phv_bit, indent)
                width_str = phv_width_string_get(asm_output, cf.phv_bit - phv_bit)
                cstr += '_pad_%d_%s; // BE[%d] FLE[%d:%d]\n' % \
                    (phv_bit, width_str, phv_bit,
                    _phv_bit_flit_le_bit(phv_bit),_phv_bit_flit_le_bit(cf.phv_bit-1))
                pstr += cstr
                cfields.append(cstr)
            cstr = cstruct_data_type_get(asm_output, cf.width, indent)
            width_str = phv_width_string_get(asm_output, cf.width)
            cstr += '%s%s; // BE[%d] FLE[%d:%d]\n' % \
                    (_get_output_name(cf.hfname), width_str, cf.phv_bit,
                    _phv_bit_flit_le_bit(cf.phv_bit), _phv_bit_flit_le_bit(cf.phv_bit+cf.width-1))
            pstr += cstr
            cfields.append(cstr)
            phv_bit = cf.phv_bit + cf.storage_size()
            continue

        # union-ed fld or header
        # skip union-ed synthetic header flds (they should be accessed using it src field
        # (otherwise it can be a nested union)
        if is_synthetic_header(cf.get_p4_hdr()) and cf.is_union():
            continue

        if cf.is_hdr_union_storage or cf.is_hdr_union:
            #pdb.set_trace()
            hdr_name = cf.get_p4_hdr().name
            storage_hdr_name = gress_pa.hdr_unions[cf.get_p4_hdr()][2].name
            if active_union and active_union.name != storage_hdr_name:
                cstr = active_union.print_union()
                pstr += cstr
                cfields.append(cstr)
                active_union = None

            if active_union:
                active_union.add_cfld(hdr_name, cf)
            else:
                active_union = _phv_union(storage_hdr_name, asm_output)
                active_union.add_cfld(hdr_name, cf)

                # check for gaps
                if phv_bit != cf.phv_bit:
                    ncc_assert((cf.phv_bit - phv_bit) >= 0)
                    cstr = cstruct_data_type_get(asm_output, cf.phv_bit - phv_bit, indent)
                    width_str = phv_width_string_get(asm_output, cf.phv_bit - phv_bit)
                    cstr += '_pad_%d_%s; // BE[%d] FLE[%d:%d]\n' % \
                        (phv_bit, width_str, phv_bit,
                        _phv_bit_flit_le_bit(phv_bit),_phv_bit_flit_le_bit(cf.phv_bit-1))
                    pstr += cstr
                    cfields.append(cstr)

            phv_bit = active_union.end_phv

        elif cf.is_fld_union_storage or cf.is_fld_union:
            #pdb.set_trace()
            storage_fld_name = gress_pa.fld_unions[cf][1].hfname
            if active_union and active_union.name != storage_fld_name:
                cstr = active_union.print_union()
                pstr += cstr
                cfields.append(cstr)
                phv_bit = active_union.end_phv
                active_union = None

            if active_union:
                active_union.add_cfld(cf.hfname, cf)
                phv_bit = active_union.end_phv
            else:
                active_union = _phv_union(storage_fld_name, asm_output)
                active_union.add_cfld(cf.hfname, cf)
                # check for gaps
                if phv_bit != cf.phv_bit:
                    ncc_assert((cf.phv_bit - phv_bit) >= 0)
                    cstr = cstruct_data_type_get(asm_output, cf.phv_bit - phv_bit, indent)
                    width_str = phv_width_string_get(asm_output, cf.phv_bit - phv_bit)
                    cstr += '_pad_%d_%s; // BE[%d] FLE[%d:%d]\n' % \
                        (phv_bit, width_str, phv_bit,
                        _phv_bit_flit_le_bit(phv_bit),_phv_bit_flit_le_bit(cf.phv_bit-1))
                    pstr += cstr
                    cfields.append(cstr)

                phv_bit = active_union.end_phv
        else:
            pass # covered first

    if active_union:
        cstr = active_union.print_union()
        pstr += cstr
        cfields.append(cstr)
        phv_bit = active_union.end_phv

    last_flit_pad = ((fid+1) * flit_sz) - phv_bit
    if last_flit_pad:
        cstr = cstruct_data_type_get(asm_output, last_flit_pad, indent)
        width_str = phv_width_string_get(asm_output, last_flit_pad)
        cstr += '_pad_%d_ %s; // FLE[%d:%d]\n' % \
                    (phv_bit, width_str,
                    _phv_bit_flit_le_bit(phv_bit),
                    _phv_bit_flit_le_bit(phv_bit+last_flit_pad-1))
        pstr += cstr
        cfields.append(cstr)
    if fid < num_flits and pstr_flit[fid] == '':
        if not asm_output:
            for _pstr in reversed(cfields):
                flit_cstr += _pstr
            pstr = flit_cstr
        pstr_flit[fid] = copy.copy(pstr)
        cfields = []

    if not asm_output:
        fname = '__%s_%s_phv__' % (gress_pa.pa.be.prog_name, gress_pa.d.name)
        hfile.write("#ifndef " +  fname.upper() + '\n\n')
        hfile.write("#define " + fname.upper() + '\n\n\n')
        pstr = 'typedef struct __attribute__ ((__packed__)) ' + gress_pa.pa.be.prog_name + '_' + gress_pa.d.name.lower() + '_phv_ {\n'
        hfile.write(pstr)
        for i in range (0, num_flits):
            hfile.write(pstr_flit[i])
        pstr = '} ' + gress_pa.pa.be.prog_name + '_' + gress_pa.d.name.lower() + '_phv_t;\n\n'
        hfile.write(pstr)
        hfile.write("#endif\n")
    else:
        pstr = 'struct phv_ {\n'
        hfile.write(pstr)
        for i in range (num_flits-1, -1, -1):
            hfile.write(pstr_flit[i])
        pstr = '};\n'
        hfile.write(pstr)
    hfile.close()

    # create a test asm file with phvwr to all flds
    # This can be used for a quick validation that phv_ struct generated can be used
    # by capas
    if asm_output:
        fname = cur_path + '/%s_phv_test.asm' % gress_pa.d.name
        hfile = open(fname, 'w')
        pstr = "#include \"%s_p.h\"\n" % gress_pa.d.name
        pstr += "struct phv_ p;"
        pstr += "\n%%\ntest_start:\n"

        indent = '    '
        for cf in gress_pa.field_order:
            if cf.width == 0 or cf.is_ohi:
                continue
            if cf.get_p4_hdr() and is_synthetic_header(cf.get_p4_hdr()) and cf.is_union():
                # hv bits do not have header associated with them
                continue
            pstr += indent + "phvwr p.%s, 0\n" % _get_output_name(cf.hfname)
        pstr += indent + 'nop.e\n'
        pstr += indent + 'nop\n'
        hfile.write(pstr)
        hfile.close()

def _capri_asm_output_pa(gress_pa):
    gen_dir = gress_pa.pa.be.args.gen_dir
    cur_path = gen_dir + '/%s/asm_out' % gress_pa.pa.be.prog_name
    if not os.path.exists(cur_path):
        try:
            os.makedirs(cur_path)
        except OSError as e:
            if e.errno != errno.EEXIST:
                raise
    fname = cur_path + '/%s_p.h' % gress_pa.d.name
    hfile = open(fname, 'w')
    num_flits = gress_pa.pa.be.hw_model['phv']['num_flits']
    pstr_flit = {}
    for i in range (0, num_flits):
        pstr_flit[i] = ''

    p_str = 'struct phv_ {\n'
    indent='    '
    indent2=indent+indent
    indent3=indent2+indent
    output_hdr_unions = {}

    flit_sz = gress_pa.pa.be.hw_model['phv']['flit_size']

    for flit_inst in range (0, num_flits):
        pstr = ''
        nextflit_pstr = ''
        for cf in gress_pa.field_order:
            # if phv_bit is not in range of flit instance bits
            # continue
            if not ((cf.phv_bit >= (flit_inst * flit_sz)) and
                    (cf.phv_bit < ((flit_inst + 1) * flit_sz))):
                continue
            if cf.is_union() or cf.is_union_pad or cf.width == 0:
            #if cf.is_union() or cf.width == 0:
                continue
            if cf.is_union_storage():
                # union_strage is NOT marked as is_union
                hdr = cf.get_p4_hdr()
                if hdr in gress_pa.hdr_unions:
                    if hdr in output_hdr_unions:
                        continue
                    output_hdr_unions[hdr] = True
                    # check if any header has fields in phv
                    # get all non-ohi fields of all the headers in union
                    _, uh_list, _ = gress_pa.hdr_unions[hdr]
                    zero_phv = True
                    for uh in uh_list:
                        if gress_pa.get_header_phv_size(uh) != 0:
                            zero_phv = False
                    if zero_phv:
                        continue
                    # create union of all header and their non-ohi flds
                    hdr = cf.get_p4_hdr()
                    max_unioned_hdr_size = 0
                    for uh in uh_list:
                        if gress_pa.get_header_phv_size(uh) > max_unioned_hdr_size:
                            max_unioned_hdr_size = gress_pa.get_header_phv_size(uh)

                    last_field_within_flit_phv_bit = 0
                    last_field_within_flit_width = 0
                    for uh in uh_list:
                        if gress_pa.get_header_phv_size(uh) == 0:
                            continue
                        for f in uh.fields:
                            ucf = gress_pa.get_field(get_hfname(f))
                            if ucf.phv_bit > last_field_within_flit_phv_bit:
                                last_field_within_flit_phv_bit = ucf.phv_bit
                                last_field_within_flit_width = ucf.width

                    break_header_union = False
                    if last_field_within_flit_phv_bit + last_field_within_flit_width \
                         > (flit_inst + 1) * flit_sz:
                        # Last header union field ends in next flit. This implies
                        # header union has to be broken into 2. One for this flit
                        # another one for next flit. Assuming atmost header will be
                        # split across 2 flits only.. No recursive solution!!
                        break_header_union = True

                    if not break_header_union:
                        pstr += indent+'union { /* Header Union */\n'
                        for uh in uh_list:
                            if gress_pa.get_header_phv_size(uh) == 0:
                                pstr += indent2 + '// skip header %s - ' \
                                        'Nothing in PHV\n' % uh.name
                                continue
                            pad_size = max_unioned_hdr_size - gress_pa.get_header_phv_size(uh)
                            add_pad = 0
                            last_ucf_width = 0
                            field_pad = 0
                            uhfields_str = ''
                            for f in uh.fields:
                                ucf = gress_pa.get_field(get_hfname(f))
                                if ucf.is_ohi:
                                    continue
                                uhfields_str += indent3+'%s : %d; // %s\n' % \
                                    (_get_output_name(ucf.hfname),  ucf.width,
                                            _get_phv_loc(flit_inst, ucf.phv_bit, ucf.width))
                                add_pad = 1
                                if last_ucf_width > 0:
                                    if last_ucf.phv_bit + last_ucf_width < ucf.phv_bit:
                                        uhfields_str += indent3+'%s : %d; //\n' % \
                                         ('_pad_gap_' + \
                                         str(last_ucf.phv_bit + last_ucf_width),\
                                          ucf.phv_bit - (last_ucf.phv_bit + \
                                                         last_ucf_width))
                                        field_pad += ucf.phv_bit - (last_ucf.phv_bit + last_ucf_width)

                                last_ucf = ucf
                                last_ucf_width = ucf.width
                            if pad_size > 0 and add_pad == 1:
                                uhfields_str += indent3+'%s : %d; //\n' % \
                                        ('_pad' + uh.name,  (pad_size * 8) - field_pad)

                            if len(uhfields_str) > 0:
                                pstr += indent2+'struct {\n'
                                pstr += uhfields_str
                                pstr += indent2+'};\n'
                        pstr += indent+'};\n'

                    if break_header_union:
                        pstr += indent+'union { /* Header Union */\n'
                        # Collect all fields that are in this flit and unionize them
                        max_size_in_union = 0
                        for uh in uh_list:
                            total_field_size_in_this_flit = 0
                            for f in uh.fields:
                                ucf = gress_pa.get_field(get_hfname(f))
                                if ucf.is_ohi:
                                    continue
                                if (ucf.phv_bit + ucf.width) > (flit_inst+1) * flit_sz:
                                    continue
                                total_field_size_in_this_flit += ucf.width

                            if total_field_size_in_this_flit > max_size_in_union:
                                max_size_in_union = total_field_size_in_this_flit

                        for uh in uh_list:
                            if gress_pa.get_header_phv_size(uh) == 0:
                                pstr += indent2 + '// skip header %s - ' \
                                        'Nothing in PHV\n' % uh.name
                                continue
                            add_pad = 0
                            last_ucf_width = 0
                            this_hdr_broken_size = 0
                            uhfields_str = ''
                            last_ucf = gress_pa.get_field(get_hfname(uh.fields[0]))
                            last_ucf_width = last_ucf.width
                            for f in uh.fields:
                                ucf = gress_pa.get_field(get_hfname(f))
                                if ucf.is_ohi:
                                    continue
                                if (ucf.phv_bit + ucf.width) > (flit_inst+1) * flit_sz:
                                    continue
                                add_pad = 1
                                uhfields_str += indent3+'%s : %d; // %s\n' % \
                                    (_get_output_name(ucf.hfname),  ucf.width,
                                            _get_phv_loc(flit_inst, ucf.phv_bit, ucf.width))
                                this_hdr_broken_size += ucf.width
                                if last_ucf_width > 0:
                                    if last_ucf.phv_bit + last_ucf_width < ucf.phv_bit:
                                        uhfields_str += indent3+'%s : %d; //\n' % \
                                         ('_pad_gap_' + \
                                         str(last_ucf.phv_bit + last_ucf_width),\
                                          ucf.phv_bit - (last_ucf.phv_bit + \
                                                         last_ucf_width))
                                        this_hdr_broken_size += (ucf.phv_bit - \
                                             (last_ucf.phv_bit + last_ucf_width))

                                last_ucf = ucf
                                last_ucf_width = ucf.width
                            if this_hdr_broken_size < max_size_in_union:
                                uhfields_str += indent3+'%s : %d; //\n' % \
                                  ('_pad_' + uh.name + '_' + \
                                  str(last_ucf.phv_bit),\
                                  max_size_in_union - this_hdr_broken_size)

                            if len(uhfields_str) > 0:
                                pstr += indent2+'struct {\n'
                                pstr += uhfields_str
                                pstr += indent2+'};\n'
                        pstr += indent+'};\n'

                        # Collect all fields that are in next flit and unionize them
                        empty_union = 1
                        nextflit_pstr = indent+'union { /* Header Union */\n'
                        for uh in uh_list:
                            if gress_pa.get_header_phv_size(uh) == 0:
                                nextflit_pstr += indent2 + '// skip header %s - ' \
                                        'Nothing in PHV\n' % uh.name
                                continue
                            add_pad = 0
                            last_ucf_width = 0
                            pad_size = (max_unioned_hdr_size * 8) - max_size_in_union
                            field_pad = 0
                            fields_nextflit_pstr = ''
                            for f in uh.fields:
                                ucf = gress_pa.get_field(get_hfname(f))
                                last_ucf = ucf
                                if ucf.is_ohi:
                                    continue
                                if (ucf.phv_bit + ucf.width) <= (flit_inst+1) * flit_sz:
                                    continue
                                fields_nextflit_pstr += indent3+'%s : %d; // %s\n' % \
                                    (_get_output_name(ucf.hfname),  ucf.width,
                                            _get_phv_loc(flit_inst, ucf.phv_bit, ucf.width))
                                pad_size -= ucf.width
                                add_pad = 1
                                if last_ucf_width > 0:
                                    if last_ucf.phv_bit + last_ucf_width < ucf.phv_bit:
                                        pstr += indent3+'%s : %d; //\n' % \
                                         ('_pad_gap_' + \
                                         str(last_ucf.phv_bit + last_ucf_width),\
                                          ucf.phv_bit - (last_ucf.phv_bit + \
                                                         last_ucf_width))
                                        pad_size -= ucf.phv_bit - (last_ucf.phv_bit + last_ucf_width)

                                last_ucf = ucf
                                last_ucf_width = ucf.width
                                empty_union = 0
                            if pad_size > 0 and add_pad == 1:
                                if ((pad_size * 8) - field_pad) > 0:
                                    fields_nextflit_pstr += indent3+'%s : %d; //\n' % \
                                            ('_pad' + uh.name,  pad_size)

                            if len(fields_nextflit_pstr) > 0:
                                nextflit_pstr += indent2+'struct {\n'
                                nextflit_pstr += fields_nextflit_pstr
                                nextflit_pstr += indent2+'};\n'

                        nextflit_pstr += indent+'};\n'

                        if empty_union:
                            nextflit_pstr = ''

                else:
                    # fld union
                    ncc_assert(cf in gress_pa.fld_unions)
                    pstr += indent+'union {\n'
                    # create union of all non-ohi flds
                    uf_list, _ = gress_pa.fld_unions[cf]
                    for uf in uf_list:
                        if uf.is_ohi:
                            pstr += '// skip ohi field %s\n' % uf.hfname
                            continue
                        if (uf.phv_bit/flit_sz) != ((uf.phv_bit+uf.width-1) / flit_sz):
                            next_flit_bits = (uf.phv_bit + uf.width) % flit_sz
                            this_flit_bits = uf.width - next_flit_bits
                            pstr += indent2+'%s : %d; // %s; Field' + \
                                    ' union crossed flit. (from flit %d to %d)\n' % \
                                    (_get_output_name(uf.hfname)+'_0', this_flit_bits,
                                            _get_phv_loc(flit_inst, uf.phv_bit, this_flit_bits),
                                            flit_inst, flit_inst+1)

                            pstr += indent+'}; // Remaining fld union bit ' \
                                     + 'of %s split into second union. \n' % \
                                     _get_output_name(uf.hfname)
                            nextflit_pstr = indent+'union { // Split fld ' + \
                                            'union bits of %s continued.\n' \
                                            % _get_output_name(uf.hfname)
                            nextflit_pstr += indent2+'%s : %d; // %s Remaining  \
                                             fld union bit here\n' % \
                                             (_get_output_name(uf.hfname)+'_1',
                                                     next_flit_bits,
                                                     _get_phv_loc(flit_inst+1,
                                                         uf.phv_bit + this_flit_bits, next_flit_bits))
                        else:
                            if not len(nextflit_pstr):
                                pstr += indent2+'%s : %d; // %s\n' % \
                                        (_get_output_name(uf.hfname),  uf.width,
                                                _get_phv_loc(flit_inst, uf.phv_bit, uf.width))
                            else:
                                nextflit_pstr += indent2+'%s : %d; // %s\n' % \
                                        (_get_output_name(uf.hfname), uf.width,
                                            _get_phv_loc(flit_inst, uf.phv_bit, uf.width))

                    if not len(nextflit_pstr):
                        pstr += indent+'};\n'
                    else:
                        nextflit_pstr += indent+'};\n'
            else:
                if cf.is_ohi:
                    continue
                if (cf.phv_bit/flit_sz) != ((cf.phv_bit+cf.width-1) / flit_sz):
                    # this field crosses the phv flit
                    # Split the field into two and populate into next flit.
                    if cf.width > flit_sz:
                        this_flit_bits = cf.width % flit_sz
                        next_flit_bits = cf.width - this_flit_bits
                    else:
                        next_flit_bits = (cf.phv_bit + cf.width) % flit_sz
                        this_flit_bits = cf.width - next_flit_bits

                    if this_flit_bits:
                        pstr += indent + ('%s : %d; // %s, Crossed Flits ' \
                            '(from flit %d to %d). Split into two fields\n' % \
                            (_get_output_name(cf.hfname) + '_1',  this_flit_bits,
                                _get_phv_loc(flit_inst, cf.phv_bit, this_flit_bits),
                                flit_inst, flit_inst+1))

                    if next_flit_bits:
                        nextflit_pstr = indent + ('%s : %d; // %s, Crossed Flits' \
                            '(from flit %d to %d). Split ' \
                            'into two fields\n' % \
                            (_get_output_name(cf.hfname) + '_0', next_flit_bits,
                                _get_phv_loc(flit_inst+1, cf.phv_bit + this_flit_bits, next_flit_bits),
                                flit_inst, flit_inst + 1))
                else:
                    pstr += indent + '%s : %d; // %s\n' % \
                            (_get_output_name(cf.hfname), cf.width,
                                    _get_phv_loc(flit_inst, cf.phv_bit, cf.width))

        pstr_flit[flit_inst] += pstr # += here because in case when processing flit0
                                     # we collected bits for flit1. (Field or header
                                     # that spans across multiple flits.
        if flit_inst + 1 < num_flits:
            if len(nextflit_pstr):
                pstr_flit[flit_inst + 1] = nextflit_pstr
        else:
            if len(nextflit_pstr):
                ncc_assert(0)

    for i in range (num_flits):
        comment = '/* --------------- Phv Flit %d ------------ */\n\n' % i
        pstr_flit[i] = comment + pstr_flit[i]
    for i in range (num_flits-1, -1, -1):
        p_str += pstr_flit[i]
    p_str += '};\n'

    hfile.write(p_str)
    hfile.close()


def _add_cf_to_fld_map(km_off, cf, t, active_cfs, fld_map):
    # t = (km_off, fs, fw)
    if not cf:
        #pdb.set_trace()
        fld_map[km_off+t[0]] = [(None, t[1], t[2], True)]
        return
    if cf in active_cfs:
        t1 = active_cfs[cf]
        active_cfs[cf]= (t1[0], t1[1], t1[2]+t[2], False)
    else:
        active_cfs[cf] = (km_off+t[0], t[1], t[2], False)

    if (t[1]+t[2]) == cf.width:
        t1 = active_cfs.pop(cf)
        if t1[0] in fld_map:
            fld_map[t1[0]].append((cf, t1[1], t1[2], t1[1] == 0))
        else:
            fld_map[t1[0]] = [(cf, t1[1], t1[2], t1[1] == 0)]
    return

def _close_active_cfs(new_cfs, active_cfs, fld_map):
    close_cfs = []
    for cf, t1 in active_cfs.items():
        # if any cf is not continued, close it
        if cf not in new_cfs:
            close_cfs.append(cf)
            if t1[0] in fld_map:
                fld_map[t1[0]].append((cf, t1[1], t1[2], False))
            else:
                fld_map[t1[0]] = [(cf, t1[1], t1[2], False)]

    for cf in close_cfs:
        #pdb.set_trace()
        del active_cfs[cf]

def capri_asm_output_table(be, ctable):

    # Table KI, KD asm code is now generated
    # using tenjin template. This code is not
    # used any more. For now putting a return
    # here. This code is potential candidate
    # for cleanup -- TBD done once asm code
    # stabilizes.
    # return
    class _ki_union:
        def __init__(self, km_off, flist):
            self.ustreams = OrderedDict()
            self.union_end = 0
            self.km_off = km_off    # for printing comments
            for (cf, fs, fw, full_fld) in flist:
                if not cf: pdb.set_trace()
                s_name = cf_get_hname(cf)
                ncc_assert(s_name not in self.ustreams)
                self.ustreams[s_name] = [(cf, fs, fw, full_fld)]
                if (km_off+fw) > self.union_end:
                    self.union_end = km_off+fw

        def fladd(self, km_off, flist):
            for (cf, fs, fw, full_fld) in flist:
                s_name = cf_get_hname(cf)
                ncc_assert(s_name in self.ustreams)
                self.ustreams[s_name].append((cf, fs, fw, full_fld))
                if (km_off+fw) > self.union_end:
                    self.union_end = km_off+fw

        def _print(self, fld_names, indent='    '):
            pstr = indent + 'union {\n'
            indent2 = indent+indent
            for s_name,flist in self.ustreams.items():
                strm_off = 0
                findent = indent2
                #if len(flist) > 1:
                pstr += indent2 + 'struct {\n'
                findent = indent + indent2

                for (cf, fs, fw, full_fld) in flist:
                    fname = _get_output_name(cf.hfname)
                    if not full_fld:
                        fname += '_s%d_e%d' % (fs, fs+fw-1)
                    if fname in fld_names:
                        fname += '_%d' % (self.km_off+strm_off)
                    fld_names[fname] = 1
                    pstr += findent + '%s : %d;    // k[%d:%d]\n' % \
                        (fname, fw,
                         511-(self.km_off+strm_off),
                         511-(self.km_off+strm_off+fw)+1)
                    strm_off += fw
                if self.km_off+strm_off < self.union_end:
                    pstr += findent + '_%s_end_pad_%d : %d; // k[%d:%d]\n' % \
                                    (s_name, self.km_off+strm_off,
                                     self.union_end-strm_off-self.km_off,
                                     511-strm_off-self.km_off, 511-self.union_end+1)
                #if len(flist) > 1:
                pstr += indent2 + '};\n'
            pstr += indent + '};\n'
            return pstr

    hw_model = be.hw_model
    max_km_width = hw_model['match_action']['key_maker_width']
    max_kmB = max_km_width/8

    gen_dir = be.args.gen_dir
    cur_path = gen_dir + '/%s/alt_asm_out' % be.prog_name
    flit_sz = hw_model['phv']['flit_size']
    if not os.path.exists(cur_path):
        try:
            os.makedirs(cur_path)
        except OSError as e:
            if e.errno != errno.EEXIST:
                raise
    fname = cur_path + '/%s_%s_k.h' % (ctable.d.name, ctable.p4_table.name)
    hfile = open(fname, 'w')

    gress_pa = ctable.gtm.tm.be.pa.gress_pa[ctable.gtm.d]

    # create a fld map of all flds in the key-makers
    km_off = 0
    fld_map = OrderedDict() # {km_offset : (cf, f_start, f_size, complete_fld_flag)}
    # fields that are partial, keep collecting them from consecutive bytes/bits
    active_cfs = OrderedDict()
    key_makers = []
    if ctable.is_wide_key:
        key_makers.append(ctable.key_makers[-2])
        key_makers.append(ctable.key_makers[-1])
    else:
        key_makers += ctable.key_makers
    for km in key_makers:    #{
        km_prof = km.shared_km.combined_profile if km.shared_km else km.combined_profile
        i = -1
        if not km_prof:
            # mpu-only table no K or I
            continue
        ncc_assert(km_prof)
        for i,phc in enumerate(km_prof.byte_sel):   #{
            new_cfs = []
            if phc == -1:   #{
                # handle bit extractions
                bit_sel = []
                if i != km_prof.bit_loc and i != km_prof.bit_loc1:
                    # close active_cfs
                    _close_active_cfs(new_cfs, active_cfs, fld_map)

                    # unused byte
                    fld_map[km_off] = [(None, 0, 8, True)]
                    km_off += 8
                    continue
                elif i == km_prof.bit_loc:
                    for b in range(0, 8):
                        if b < len(km_prof.bit_sel):
                            bit_sel.append(km_prof.bit_sel[b])
                        else:
                            bit_sel.append(-1)
                else:
                    ncc_assert(i == km_prof.bit_loc1)
                    for b in range(8, 16):
                        if b < len(km_prof.bit_sel):
                            bit_sel.append(km_prof.bit_sel[b])
                        else:
                            bit_sel.append(-1)

                #pdb.set_trace()
                for b in bit_sel: #{
                    if b < 0:
                        # close all active
                        _close_active_cfs(new_cfs, active_cfs, fld_map)
                        fld_map[km_off] = [(None, 0, 1, True)]
                        km_off += 1
                        continue
                    phc = b/8
                    bflds = gress_pa.phcs[phc].get_flds(b%8,1)
                    for fname,(fs,fw) in bflds.items():
                        new_cfs = []
                        cf = gress_pa.get_field(fname)
                        _add_cf_to_fld_map(km_off, cf, (0, fs, fw), active_cfs, fld_map)
                    km_off += 1
                #}
                # get next byte
                continue
            #}

            # sort based on position in the phv container
            phc_flds = sorted(gress_pa.phcs[phc].fields.items(), key=lambda k: k[1][0])

            for fname, t in phc_flds:   #{
                cf = gress_pa.get_field(fname)
                new_cfs.append(cf)
                _add_cf_to_fld_map(km_off, cf, t, active_cfs, fld_map)
            #}
            _close_active_cfs(new_cfs, active_cfs, fld_map)

            km_off += 8
        #}

        #pdb.set_trace()
        if (i+1) < max_kmB:
            # close active cfs before adding pad bytes
            new_cfs = []
            _close_active_cfs(new_cfs, active_cfs, fld_map)

            # single pad field to fill unused km
            pad_sz = (max_kmB-(i+1)) * 8
            fld_map[km_off] = [(None, 0, pad_sz, True)]
            km_off += pad_sz

    #}

    if len(active_cfs):
        new_cfs = []
        _close_active_cfs(new_cfs, active_cfs, fld_map)

    ncc_assert(len(active_cfs) == 0)

    # mpu-only table with no k+i
    if len(fld_map) == 0:
        return

    # table using single km
    if len(ctable.key_makers) < 2:
        fld_map[max_km_width] = [(None, 0, max_km_width, True)]

    #pdb.set_trace()
    active_union = None
    ki_flds = []
    exp_km_off = 0

    sorted_fld_map = sorted(fld_map.items(), key=lambda k: k[0])
    #pdb.set_trace()
    for km_off,flist in sorted_fld_map:
        # XXX pad flds within union are not expected/handled ??
        if active_union:
            if km_off == active_union.union_end:
                ki_flds.append(copy.copy(active_union))
                active_union = None
                exp_km_off = km_off
            else:
                active_union.fladd(km_off, flist)
                continue

        ncc_assert(active_union == None)

        if km_off > exp_km_off:
            ctable.gtm.tm.logger.debug("%s:Table %s insert km pad of %d at %s" % \
                                (ctable.gtm.d, ctable.p4_table.name, km_off-exp_km_off, flist))
            ki_flds.append((None, 0, km_off-exp_km_off, True))
            exp_km_off = km_off

        if len(flist) > 1:
            active_union = _ki_union(km_off, flist)
        else:
            ki_flds.append(flist[0])
            exp_km_off += flist[0][2]

    #pdb.set_trace()
    if active_union:
        ki_flds.append(copy.copy(active_union))
        active_union = None

    pstr = 'struct %s_k_ {\n' % ctable.p4_table.name
    indent = '    '
    km_off = 0
    fld_names = {}
    for i,ki_f in enumerate(ki_flds):
        if isinstance(ki_f, _ki_union):
            pstr += ki_f._print(fld_names, indent)
            km_off = ki_f.union_end
        else:
            if ki_f[0] == None:
                fname = '_pad_%d' % i
            else:
                fname = _get_output_name(ki_f[0].hfname)
            if not ki_f[3]:
                fname += '_s%d_e%d' % (ki_f[1], ki_f[1]+ki_f[2]-1)
            if fname in fld_names:
                fname += '_%d' % km_off
            fld_names[fname] = 1
            pstr += indent + '%s : %d;    // k[%d:%d]\n' % \
                        (fname, ki_f[2],
                         511-km_off,
                         511-(km_off+ki_f[2])+1)
            km_off += ki_f[2]

    pstr += '};\n'
    #pdb.set_trace()
    hfile.write(pstr)
    hfile.close()
    return


def capri_parser_logical_output(parser):
    gen_dir = parser.be.args.gen_dir
    out_dir = gen_dir + '/%s/logs/' % (parser.be.prog_name)
    if not os.path.exists(out_dir):
        try:
            os.makedirs(out_dir)
        except OSError as e:
            if e.errno != errno.EEXIST:
                raise
    ofile = open('%s/%s_parser.out' % (out_dir, parser.d.name), 'w')
    pstr = ''
    # create initial config register for start state
    pstr += '%s:Init Profile:\nStart state = %d\n' % (parser.d.name, parser.start_state.id)
    for r,lkp_reg in enumerate(parser.start_state.lkp_regs):
        if lkp_reg.pkt_off >= 0:
            pstr += 'LkpReg[%d] = %d\n' % (r, lkp_reg.pkt_off / 8)

    idx = 0    # fill these backwards??
    max_tcam_entries = parser.be.hw_model['parser']['num_states']
    # XXX need array in profile
    for cs in parser.states:
        # create a match entry for {state_id, lkp_flds, lkp_fld_mask}
        for bi in cs.branches:
            if not parser.be.args.post_extract and bi.nxt_state.is_end and (len(cs.branches)==1):
                # if next state is __END and that is the only transition then this is a
                # terminal state. With pre-extraction, there is no need for additional end state
                continue
            pstr += 'Entry[%d]: Transition %s(%d) -> %s(%d):\n------------------\n' % \
                (idx, cs.name, cs.id, bi.nxt_state.name, bi.nxt_state.id)
            pstr += 'TCAM:State_id %d, ' % cs.id
            pstr += 'TCAM:lkp_val 0x%x, mask[] %s\n' % (bi.val, bi.mask)
            # SRAM - ASSUME PRE-EXTRACTION
            pstr += 'SRAM:next_state: %s\n' % bi.nxt_state.id
            if not parser.be.args.post_extract:
                pstr += 'SRAM:offset_inst: %s\n' % bi.nxt_state.extract_len
            else:
                pstr += 'SRAM:offset_inst: %s\n' % cs.extract_len
            pstr += 'SRAM: next_lkp: '
            for lkp in bi.nxt_state.lkp_flds.keys():
                pstr += '%s, ' % lkp
            pstr += '\n'
            if not parser.be.args.post_extract:
                pstr += 'SRAM:(Pre)extracted_fields: %s\n' % \
                    [cf.hfname for cf in bi.nxt_state.extracted_fields if not cf.is_ohi]
                pstr += 'SRAM:ohi_fields: %s\n' % \
                    [cf.hfname for cf in bi.nxt_state.extracted_fields if cf.is_ohi]
                pstr += 'SRAM:set_meta_reg_ops: %s\n' % bi.nxt_state.set_ops
                if bi.nxt_state.is_end:
                    pstr += 'Terminal state %s(%d)\n' % (bi.nxt_state.name, bi.nxt_state.id)
            else:
                pstr += 'SRAM:(Post)extracted_fields: %s\n' % \
                    [cf.hfname for cf in cs.extracted_fields if not cf.is_ohi]
                pstr += 'SRAM:ohi_fields: %s\n' % \
                    [cf.hfname for cf in cs.extracted_fields if cf.is_ohi]
                pstr += 'SRAM:set_meta_reg_ops: %s\n' % cs.set_ops
            pstr += '\n'
            idx += 1

    ofile.write(pstr)
    ofile.close()


def capri_deparser_logical_output(deparser):
    gen_dir = deparser.be.args.gen_dir
    out_dir = gen_dir + '/%s/logs' % (deparser.be.prog_name)
    if not os.path.exists(out_dir):
        try:
            os.makedirs(out_dir)
        except OSError as e:
            if e.errno != errno.EEXIST:
                raise
    ofile = open('%s/%s_deparser.out' % (out_dir, deparser.d.name), 'w')
    pstr = ''
    cf = deparser.be.pa.get_field("capri_p4_intrinsic.frame_size", deparser.d)
    if not cf:
        deparser.logger.warning("!!!! capri p4 instrinsic header in the" \
                                 " p4 program. Payload len Position Offset" \
                                 " not set or left as default value.!!!!")
        pstr += "\n !!!! capri p4 instrinsic header in the p4 program. "\
                "Payload len Position Offset not set or left as default value.!!!!\n"

    for hvb in range(deparser.be.hw_model['parser']['max_hv_bits']-1, -1, -1):
        h = deparser.be.parsers[deparser.d].hv_bit_header[hvb]

        if (h == None): continue

        ncc_assert(h in deparser.topo_ordered_phv_ohi_chunks)
        dp_hdr_fields = deparser.topo_ordered_phv_ohi_chunks[h]
        pstr += "\nHeader: %s \n" % (h.name)
        pstr += " Absolute HV Bit position %d\n" % (hvb)
        ohi_inst = 0
        phvchunk = 0
        for chunks in dp_hdr_fields:
            (phv_ohi, chunk_type, cf) = chunks
            if (chunk_type == deparser.field_type_phv):
                if cf != None:
                    cfstr = 'First header field %s, is at offset %d' % \
                            (cf.hfname, cf.phv_bit/8)
                else:
                    cfstr = ''
                pstr +="  PHV Chunk %d at Offset (phv bit) %d, Size %d. %s\n" \
                       % (phvchunk, phv_ohi[0], phv_ohi[1], cfstr)
                phvchunk += 1
            else:
                if (isinstance(phv_ohi.length, int)):
                    pstr += "  Ohi Inst %d Slot %d, Ohi Offset %d, Ohi size %d \n" \
                            % (ohi_inst, phv_ohi.id, phv_ohi.start, phv_ohi.length)
                else:
                    pstr += "  Ohi Inst %d Slot %d, Ohi Offset %d; size=%s \n" % \
                           (ohi_inst, phv_ohi.id, phv_ohi.start, phv_ohi.length)
                    ohi_inst += 1

    ofile.write(pstr)
    ofile.close()


def capri_deparser_cfg_output(deparser):
    csum_hv_fld_slots = OrderedDict() # Key = HVbit, Value = (fld_start, fld_end)
    icrc_hv_fld_slots = OrderedDict() # Key = HVbit, Value = (fld_start, fld_end)
    # read the register spec json
    cur_path = os.path.abspath(__file__)
    cur_path = os.path.split(cur_path)[0]
    dpr_file_path = os.path.join(cur_path, 'csr_json/cap_dpr_csr.json')
    dpr_file = open(dpr_file_path)
    dpr_json = json.load(dpr_file)

    dpp_file_path = os.path.join(cur_path, 'csr_json/cap_dpp_csr.json')
    dpp_file = open(dpp_file_path)
    dpp_json = json.load(dpp_file)

    gen_dir = deparser.be.args.gen_dir
    cfg_out_dir = os.path.join(gen_dir + '/%s/cfg_out' % deparser.be.prog_name)
    if not os.path.exists(cfg_out_dir):
        try:
            os.makedirs(cfg_out_dir)
        except OSError as e:
            if e.errno != errno.EEXIST:
                raise

    dpp_cfg_output_reg = os.path.join(cfg_out_dir, 'cap_dpp_%s_cfg_reg.json' \
                         % deparser.d.name)
    dpp_cfg_file_reg = open(dpp_cfg_output_reg, 'w+')

    dpr_cfg_output_reg = os.path.join(cfg_out_dir, 'cap_dpr_%s_cfg_reg.json' \
                         % deparser.d.name)
    dpr_cfg_file_reg = open(dpr_cfg_output_reg, 'w+')

    deparser.logger.info('%s:DeParser Dpp & Dpr Block Output Generation:' \
                         % deparser.d.name)

    used_hdr_fld_info_slots = deparser.be.hw_model['deparser']['hdrfld_info_start']
    max_hdr_flds = deparser.be.hw_model['deparser']['max_hdr_flds']
    phv_sel = deparser.be.hw_model['deparser']['dpa_src_phv']
    ohi_sel = deparser.be.hw_model['deparser']['dpa_src_ohi']
    fixed_sel = deparser.be.hw_model['deparser']['dpa_src_fixed']
    cfg_sel = deparser.be.hw_model['deparser']['dpa_src_cfg']
    pkt_sel = deparser.be.hw_model['deparser']['dpa_src_pkt']
    max_hv_bit_idx = deparser.be.hw_model['parser']['max_hv_bits'] - 1
    payload_hv_bit_idx = deparser.be.hw_model['parser']['hv_pkt_len_location']
    trunc_hv_bit_idx = deparser.be.hw_model['parser']['hv_pkt_trunc_location']
    pad_hv_bit_idx = deparser.be.hw_model['parser']['hv_pad_hdr_location']

    # Last header field slot is reserved for payload len.
    # Hence max available hdr-field slot is one less.

    # Fill in payload len related information.
    cf = deparser.be.pa.get_field("capri_p4_intrinsic.frame_size", deparser.d)
    if cf:
        payload_offset_len_ohi_id = deparser.be.hw_model['parser']['ohi_threshold']
        payload_offset_ohi_bit = 1<<payload_offset_len_ohi_id
        dpp_json['cap_dpp']['registers']['cap_dpp_csr_cfg_ohi_payload']\
            ['ohi_slot_payload_ptr_bm']['value'] = str('0x%x' % payload_offset_ohi_bit)
        dpp_json['cap_dpp']['registers']['cap_dpp_csr_cfg_ohi_payload']['_modified'] = True

        # Because header bit 126 is used for payload, use hdrfld_info slot 254 (last slot)
        # for specifying packet payload ohi information. OHI slot contains payload offset,
        # len comes from fram_len phv
        rstr = 'cap_dpphdr_csr_cfg_hdr_info[%d]' % (payload_hv_bit_idx)
        dpp_json['cap_dpp']['registers'][rstr]['fld_start']['value'] = str(max_hdr_flds-2)
        dpp_json['cap_dpp']['registers'][rstr]['fld_end']['value'] = str(1)
        dpp_json['cap_dpp']['registers'][rstr]['_modified'] = True
        dpp_rstr_name = 'cap_dpphdrfld_csr_cfg_hdrfld_info[%d]' % (max_hdr_flds-2)
        dpr_rstr_name = 'cap_dprhdrfld_csr_cfg_hdrfld_info[%d]' % (max_hdr_flds-2)
        dpp_rstr = dpp_json['cap_dpp']['registers'][dpp_rstr_name]
        dpr_rstr = dpr_json['cap_dpr']['registers'][dpr_rstr_name]
        # hw gets total frame len from frame_size phv, needs the ohi_slot carrying payload_offset
        # to subtract it from total len
        dpp_rstr['size_sel']['value'] = str(ohi_sel)
        dpp_rstr['size_val']['value'] = str(payload_offset_len_ohi_id)
        dpp_rstr['_modified'] = True
        # get pkt_offset from last(sw) ohi slot
        dpr_rstr['source_sel']['value'] = str(ohi_sel)
        dpr_rstr['source_oft']['value'] = str(payload_offset_len_ohi_id)
        dpr_rstr['_modified'] = True

    for name, hdr in deparser.be.h.p4_header_instances.items():
        if hdr.metadata:
            if 'deparser_variable_length_header' in hdr._parsed_pragmas:
                hf_name = hdr.name + '.trunc'
                cf = deparser.be.pa.get_field(hf_name, deparser.d)
                ncc_assert(cf)
                rstr = 'cap_dpphdr_csr_cfg_hdr_info[%d]' % (trunc_hv_bit_idx)
                dpp_json['cap_dpp']['registers'][rstr]['fld_start']['value'] = str(max_hdr_flds-3)
                dpp_json['cap_dpp']['registers'][rstr]['fld_end']['value'] = str(2)
                dpp_json['cap_dpp']['registers'][rstr]['_modified'] = True
                dpp_rstr_name = 'cap_dpphdrfld_csr_cfg_hdrfld_info[%d]' % (max_hdr_flds-3)
                dpr_rstr_name = 'cap_dprhdrfld_csr_cfg_hdrfld_info[%d]' % (max_hdr_flds-3)
                dpp_rstr = dpp_json['cap_dpp']['registers'][dpp_rstr_name]
                dpr_rstr = dpr_json['cap_dpr']['registers'][dpr_rstr_name]
                dpp_rstr['size_sel']['value'] = str(phv_sel)
                hf_name = hdr.name + '.trunc_pkt_len'
                cf = deparser.be.pa.get_field(hf_name, deparser.d)
                ncc_assert(cf)
                dpr_slot = cf.phv_bit - deparser.be.hw_model['phv']['flit_size']
                dpr_slot = dpr_slot / 16
                dpp_rstr['size_val']['value'] = str(dpr_slot)
                payload_offset_len_ohi_id = deparser.be.hw_model['parser']['ohi_threshold']
                dpr_rstr['source_sel']['value'] = str(ohi_sel)
                dpr_rstr['source_oft']['value'] = str(payload_offset_len_ohi_id)
                dpr_rstr['_modified'] = True
                dpp_rstr['_modified'] = True

            if 'deparser_pad_header' in hdr._parsed_pragmas:
                hf_name = hdr.name + '.pad'
                cf = deparser.be.pa.get_field(hf_name, deparser.d)
                ncc_assert(cf)
                rstr = 'cap_dpphdr_csr_cfg_hdr_info[%d]' % (pad_hv_bit_idx)
                dpp_json['cap_dpp']['registers'][rstr]['fld_start']['value'] = str(max_hdr_flds-1)
                dpp_json['cap_dpp']['registers'][rstr]['fld_end']['value'] = str(0)
                dpp_json['cap_dpp']['registers'][rstr]['_modified'] = True
                dpp_rstr_name = 'cap_dpphdrfld_csr_cfg_hdrfld_info[%d]' % (max_hdr_flds-1)
                dpr_rstr_name = 'cap_dprhdrfld_csr_cfg_hdrfld_info[%d]' % (max_hdr_flds-1)
                dpp_rstr = dpp_json['cap_dpp']['registers'][dpp_rstr_name]
                dpr_rstr = dpr_json['cap_dpr']['registers'][dpr_rstr_name]
                dpp_rstr['size_sel']['value'] = str(phv_sel)
                hf_name = hdr.name + '.pad_len'
                cf = deparser.be.pa.get_field(hf_name, deparser.d)
                ncc_assert(cf)
                dpr_slot = cf.phv_bit - deparser.be.hw_model['phv']['flit_size']
                dpr_slot = dpr_slot / 16
                dpp_rstr['size_val']['value'] = str(dpr_slot)
                dpr_rstr['source_sel']['value'] = str(cfg_sel)
                dpr_rstr['source_oft']['value'] = str(0) #cap_dprcfg_csr_cfg_static_field
                                                         #register has to be set to zero
                dpr_rstr['_modified'] = True
                dpp_rstr['_modified'] = True

                #Configure config register from which pad bytes are sourced.
                #Setting this register to zero will make pad bytes as zero value.
                for idx in range(0, 64):
                    dpr_rstr_name = 'cap_dprcfg_csr_cfg_static_field[%d]' % (idx)
                    dpr_rstr = dpr_json['cap_dpr']['registers'][dpr_rstr_name]
                    dpr_rstr['data']['value'] = str(0)
                    dpr_rstr['_modified'] = True

    # Fill in all header fields information.
    for hvb in range(max_hv_bit_idx, -1, -1):
        h = deparser.be.parsers[deparser.d].hv_bit_header[hvb]
        if (h == None):
            continue

        ncc_assert(h in deparser.topo_ordered_phv_ohi_chunks)

        dp_hdr_fields = deparser.topo_ordered_phv_ohi_chunks[h]

        phvchunks = 0
        ohis = 0

        first_ohi = False
        start_fld = used_hdr_fld_info_slots

        #Generate DPP block configurations
        rstr = 'cap_dpphdr_csr_cfg_hdr_info[%d]' % (max_hv_bit_idx - hvb)
        # Logic = all_1_mask >> fld_end  & (all_1_mask << fld_start)

        csum_hvb = False
        if h in deparser.be.parsers[deparser.d].csum_hdr_hv_bit.keys():
            #HV bit used for csum purposes. Do not use them to
            #include hdrs into packet.
            for csum_h in \
                deparser.be.parsers[deparser.d].csum_hdr_hv_bit[h]:
                csum_allocated_hv, _, _ = csum_h
                if csum_allocated_hv == hvb:
                    csum_hvb = True
                    break
        icrc_hvb = False
        if h in deparser.be.parsers[deparser.d].icrc_hdr_hv_bit.keys():
            #HV bit used for icrc purposes. Do not use them to
            #include hdrs into packet.
            for icrc_h in \
                deparser.be.parsers[deparser.d].icrc_hdr_hv_bit[h]:
                icrc_allocated_hv, _, _ = icrc_h
                if icrc_allocated_hv == hvb:
                    icrc_hvb = True
                    break
        end_fld_info_adjust = False
        if not csum_hvb and not icrc_hvb:
            for i, chunks in enumerate(dp_hdr_fields):
                ncc_assert(used_hdr_fld_info_slots < (max_hdr_flds-1), "No hdr fld slots avaialble")
                dpp_rstr_name = 'cap_dpphdrfld_csr_cfg_hdrfld_info[%d]' % (used_hdr_fld_info_slots)
                dpr_rstr_name = 'cap_dprhdrfld_csr_cfg_hdrfld_info[%d]' % (used_hdr_fld_info_slots)
                dpp_rstr = dpp_json['cap_dpp']['registers'][dpp_rstr_name]
                dpr_rstr = dpr_json['cap_dpr']['registers'][dpr_rstr_name]
                (phv_ohi, chunk_type, _) = chunks
                if (chunk_type == deparser.field_type_phv):
                    dpp_rstr['size_sel']['value'] = str(fixed_sel)
                    dpp_rstr['size_val']['value'] = str(phv_ohi[1]/8) if not csum_hvb else str(0) # in bytes

                    dpr_rstr['source_sel']['value'] = str(phv_sel)
                    dpr_rstr['source_oft']['value'] = str(phv_ohi[0]/8) # in bytes
                else:
                    if not first_ohi:
                        first_ohi = True
                        fohi = phv_ohi

                    dpr_rstr['source_sel']['value'] = str(ohi_sel)
                    dpr_rstr['source_oft']['value'] = \
                        str((phv_ohi.start - fohi.start) << 6 | fohi.id)
                    if (isinstance(phv_ohi.length, int)):
                        dpp_rstr['size_sel']['value'] = str(fixed_sel)
                        dpp_rstr['size_val']['value'] = str(phv_ohi.length) if not csum_hvb else str(0)
                    else:
                        dpp_rstr['size_sel']['value'] = str(ohi_sel)
                        dpp_rstr['size_val']['value'] = str(phv_ohi.var_id) if not csum_hvb else str(0)
                dpr_rstr['_modified'] = True
                dpp_rstr['_modified'] = True
                used_hdr_fld_info_slots += 1

            #Check if the header has csum hv bits; if so, allocate dummy  end fld slots
            #and adjust end_fld slot associated with csum hv bits.
            if h in deparser.be.parsers[deparser.d].csum_hdr_hv_bit.keys():
                end_fld_info_adjust = True
        elif csum_hvb or icrc_hvb:
            ncc_assert(used_hdr_fld_info_slots < (max_hdr_flds-1), "No hdr fld slots avaialble")
            dpp_rstr_name = 'cap_dpphdrfld_csr_cfg_hdrfld_info[%d]' % (used_hdr_fld_info_slots)
            dpr_rstr_name = 'cap_dprhdrfld_csr_cfg_hdrfld_info[%d]' % (used_hdr_fld_info_slots)
            dpp_rstr = dpp_json['cap_dpp']['registers'][dpp_rstr_name]
            dpr_rstr = dpr_json['cap_dpr']['registers'][dpr_rstr_name]
            dpp_rstr['size_val']['value'] = str(0)
            dpr_rstr['_modified'] = True
            dpp_rstr['_modified'] = True
            used_hdr_fld_info_slots += 1

        dpp_json['cap_dpp']['registers'][rstr]['fld_start']['value'] = str(start_fld)
        dpp_json['cap_dpp']['registers'][rstr]['_modified'] = True
        if not end_fld_info_adjust:
            dpp_json['cap_dpp']['registers'][rstr]['fld_end']['value'] = \
                str(max_hdr_flds - used_hdr_fld_info_slots)
            if csum_hvb:
                #Collect startfld, endfld, hv to program deparser csum
                #endfld will get adjusted once associated hdr.valid is allocated
                #for now start_fld and end_fld will be single slot.
                #Also csum hdrfld configuration to specify fld_start and fld_end
                #is different from hdrfld configuration to build pkt. (In case
                #csum, provide fld_start and fld_end zero based. In case of pkt
                #build, provide fld_start and 255-fld_end.
                csum_hv_fld_slots[hvb] = \
                    (start_fld, used_hdr_fld_info_slots, h.name)
            elif icrc_hvb:
                icrc_hv_fld_slots[hvb] = \
                    (start_fld, used_hdr_fld_info_slots, h.name)
        else:
            if icrc_hvb:
                #This should not happen. Assert
                ncc_assert(0)
            csum_bits_assigned_for_hdr = len(deparser.be.parsers[deparser.d].csum_hdr_hv_bit[h])
            ncc_assert(csum_bits_assigned_for_hdr > 0)
            #Adjust end slot for hdr.valid
            dpp_json['cap_dpp']['registers'][rstr]['fld_end']['value'] = \
                str(max_hdr_flds - (used_hdr_fld_info_slots + csum_bits_assigned_for_hdr))
            #Adjust end slot for all csum related HV
            for csum_h in \
                deparser.be.parsers[deparser.d].csum_hdr_hv_bit[h]:
                csum_allocated_hv, _, _ = csum_h
                start_fld = csum_hv_fld_slots[csum_allocated_hv][0]
                end_fld = used_hdr_fld_info_slots
                dpp_rstr_name = 'cap_dpphdrfld_csr_cfg_hdrfld_info[%d]' % (used_hdr_fld_info_slots)
                dpr_rstr_name = 'cap_dprhdrfld_csr_cfg_hdrfld_info[%d]' % (used_hdr_fld_info_slots)
                dpp_rstr = dpp_json['cap_dpp']['registers'][dpp_rstr_name]
                dpr_rstr = dpr_json['cap_dpr']['registers'][dpr_rstr_name]
                dpp_rstr['size_val']['value'] = str(0)
                dpr_rstr['_modified'] = True
                dpp_rstr['_modified'] = True
                rstr = 'cap_dpphdr_csr_cfg_hdr_info[%d]' % (max_hv_bit_idx - csum_allocated_hv)
                dpp_json['cap_dpp']['registers'][rstr]['fld_end']['value'] = \
                    str(max_hdr_flds - end_fld)
                dpp_json['cap_dpp']['registers'][rstr]['_modified'] = True
                used_hdr_fld_info_slots += 1 #Dummy slot after actual hdr slot
                csum_hv_fld_slots[csum_allocated_hv] = \
                    (start_fld, end_fld - 1, h.name)

    deparser.be.checksum.GsoCsumDeParserConfigGenerate(deparser, \
                                                       dpp_json, \
                                                       dpr_json)
    deparser.be.checksum.CsumDeParserConfigGenerate(deparser, \
                                            csum_hv_fld_slots, dpp_json)
    deparser.be.icrc.IcrcDeParserConfigGenerate(deparser, \
                                            icrc_hv_fld_slots, dpp_json)

    # Configure minimum packet size on all network ports 0 to 7.
    # Pad profile #1 is set to size 88 Bytes  (60B + 22Bytes). 4B CRC is not
    # included in min size calculation. MAC adds 4Bytes.
    # Pad profile #0 is configured to pad upto 0 bytes..which means no padding.
    # For ports 0 to 7, profile 1 is set. For all other ports profile 0
    # is configured.
    dpr_rstr_name = 'cap_dpr_csr_cfg_global_1'
    dpr_rstr = dpr_json['cap_dpr']['registers'][dpr_rstr_name]
    dpr_rstr['padding_en']['value'] = str(1)
    dpr_rstr['_modified'] = True
    dpr_rstr_name = 'cap_dpr_csr_cfg_pkt_padding'
    dpr_rstr = dpr_json['cap_dpr']['registers'][dpr_rstr_name]
    dpr_rstr['padding_profile_sel']['value'] = str(0x5555)
    p4_intrinsic_size = get_header_size(deparser.be.h.p4_header_instances['capri_p4_intrinsic']) if 'capri_p4_intrinsic' in deparser.be.h.p4_header_instances.keys()  else 0
    intrinsic_size =  get_header_size(deparser.be.h.p4_header_instances['capri_intrinsic']) if 'capri_intrinsic' in deparser.be.h.p4_header_instances.keys() else 0
    min_size = (p4_intrinsic_size + intrinsic_size + 60) << 8 # 64B - 4B CRC
    dpr_rstr['min_size']['value'] = str(min_size) #Profile #1 starts from bit 8
    dpr_rstr['_modified'] = True

    json.dump(dpp_json['cap_dpp']['registers'],
                dpp_cfg_file_reg, indent=4, sort_keys=True, separators=(',', ': '))
    dpp_cfg_file_reg.close()
    json.dump(dpr_json['cap_dpr']['registers'],
                dpr_cfg_file_reg, indent=4, sort_keys=True, separators=(',', ': '))
    dpr_cfg_file_reg.close()

    if not deparser.be.args.p4_plus:
        cap_inst = 1 if (deparser.d == xgress.INGRESS) else 0
        capri_dump_registers(deparser.be.args.cfg_dir, deparser.be.prog_name,
                             'cap_dpp', cap_inst,
                             dpp_json['cap_dpp']['registers'], None)

        capri_dump_registers(deparser.be.args.cfg_dir, deparser.be.prog_name,
                             'cap_dpr', cap_inst,
                             dpr_json['cap_dpr']['registers'], None)

def capri_model_dbg_output(be, dbg_info):
    gen_dir = be.args.gen_dir
    dbg_out_dir = os.path.join(gen_dir + '/%s/dbg_out' % be.prog_name)
    if not os.path.exists(dbg_out_dir):
        try:
            os.makedirs(dbg_out_dir)
        except OSError as e:
            if e.errno != errno.EEXIST:
                raise

    dbg_out_fname = os.path.join(dbg_out_dir, 'model_debug.json')
    dbgfile = open(dbg_out_fname, 'w+')
    json.dump(dbg_info, dbgfile, indent=4, sort_keys=False, separators=(',', ': '))
    dbgfile.close()

def capri_output_i2e_meta_header(be, i2e_fields, hsize):
    gen_dir = be.args.gen_dir
    cur_path = gen_dir + '/%s/asm_out' % be.prog_name
    if not os.path.exists(cur_path):
        try:
            os.makedirs(cur_path)
        except OSError as e:
            if e.errno != errno.EEXIST:
                raise
    fname = cur_path + '/i2e_metadata.h'
    hfile = open(fname, 'w')
    pstr = "struct i2e_metadata_header_ {\n"
    indent = '    '
    for cf in i2e_fields:
        i2e_fname = _get_output_name(cf.hfname)
        fname = i2e_fname.split('__')[2]
        str
        pstr += indent + '%s : %d;\n' % (fname, cf.width)
    pstr += "};\n// Total size %d\n" % hsize
    hfile.write(pstr)
    hfile.close()

# new output json format with decoders
def _expand_decoder(decoder_json, dname):
    tmplt = OrderedDict()
    #tmplt['fields'] = []
    for field in decoder_json['cap_ppa_decoders']['decoders'][dname]['fields']:
        tfield = OrderedDict()
        for fname,fattr in field.items():
            #tfield[fname] = []
            f_decoder = None
            n_elem = 1
            for a in fattr:
                for ak, av in a.items():
                    if ak == 'decoder':
                        f_decoder = av
                    elif ak == 'array':
                        n_elem = av
                    elif ak == 'size':
                        elem_sz = av
            if n_elem > 1:
                tfield[fname] = []
                for n in range(n_elem):
                    if f_decoder:
                        tfield[fname].append(_expand_decoder(decoder_json, f_decoder))
                    else:
                        tfield[fname].append({'value':'0', 'size':str(elem_sz)})
            else:
                if f_decoder:
                    tfield[fname] = _expand_decoder(decoder_json, f_decoder)
                else:
                    tfield[fname] = {'value':'0', 'size':str(elem_sz)}
        #tmplt['fields'].append(tfield)
        tmplt[fname] = tfield[fname]
    return tmplt

def _create_template(reg_json, decoder_json, ename):
    ncc_assert(ename in reg_json)
    elem = reg_json[ename]
    if 'decoder' in elem.keys():
        decoder_name = elem['decoder']
        return _expand_decoder(decoder_json, decoder_name), decoder_name
    else:
        return copy.deepcopy(elem), None

def mux_idx_alloc(mux_idx_allocator, pkt_off):
    for i, used in enumerate(mux_idx_allocator):
        # check for sharing a slot
        if used != None and mux_idx_allocator[i] == pkt_off:
            return i
    for i, used in enumerate(mux_idx_allocator):
        if used == None:
            mux_idx_allocator[i] = pkt_off
            return i
    ncc_assert(0)

def mux_inst_alloc(mux_inst_allocator, expr, adjust_const=False):
    if expr:
        flat_expr_str = expr.flatten_capri_expr()
        flat_expr_wo_const_str = expr.flatten_capri_expr(no_const=True)
        if adjust_const:
            #if constant value can be adjusted (only in the case if
            # mux_instr is used in offset_instr), try to share
            # mux_instr by looking for another expression that was
            # already allocated mux-instr
            for i, value in enumerate(mux_inst_allocator):
                _flat_expr_str, _flat_expr_wo_const_str, _expr = value
                if _flat_expr_wo_const_str == flat_expr_wo_const_str:
                    #Found mux instruction that can be shared by adjusting
                    #constant value of offset_instr
                    ncc_assert(_expr.const <= expr.const)

                    # Adjust constant in the expression
                    expr.const = expr.const - _expr.const
                    return i, expr.const

        for i, value in enumerate(mux_inst_allocator):
            try:
                _flat_expr_str, _flat_expr_wo_const_str, _expr = value
                if _flat_expr_str == flat_expr_str:
                    return i, 0
            except:
                pdb.set_trace()
    else:
        flat_expr_str = 'Not shared'
        flat_expr_wo_const_str = 'Not shared'

    #Instruction need to be allocated because either expr has never been
    # allocated instruction or expr = ''
    for i, value in enumerate(mux_inst_allocator):
        _flat_expr_str, _flat_expr_wo_const_str, _expr = value
        if _flat_expr_str == None:
            mux_inst_allocator[i] = (flat_expr_str, flat_expr_wo_const_str, expr)
            return i, 0
    ncc_assert(0)

def _allocate_mux_inst_resoures(capri_expr, nxt_cs, r, pkt_off1, pkt_off2, \
    mux_idx_allocator, sram):
    reg_id1 = None; reg_id2 = None
    mux_id1 = None; mux_id2 = None
    if capri_expr.src_reg:
        if not isinstance(capri_expr.src_reg, tuple) and \
            capri_expr.src_reg.is_meta:
            if capri_expr.src_reg.hfname not in nxt_cs.lkp_flds:
                reg_id1 = r
            else:
                reg_id1 = nxt_cs.lkp_flds[capri_expr.src_reg.hfname].reg_id
        else:
            off = pkt_off2
            mux_id2 = mux_idx_alloc(mux_idx_allocator, off/8)
            sram['mux_idx'][mux_id2]['sel']['value'] = str(0)
            sram['mux_idx'][mux_id2]['lkpsel']['value'] = str(0)   # NA
            sram['mux_idx'][mux_id2]['idx']['value'] = str((off/8))

    if capri_expr.src1:
        if not isinstance(capri_expr.src1, tuple) and capri_expr.src1.is_meta:
            if capri_expr.src1.hfname not in nxt_cs.lkp_flds:
                reg_id2 = r
            else:
                ncc_assert(capri_expr.src1.hfname in nxt_cs.lkp_flds, \
                    "%s is not availabe in state %s" % (capri_expr.src1.hfname, nxt_cs,name))
                reg_id2 = nxt_cs.lkp_flds[capri_expr.src1.hfname].reg_id
                mux_id1 = mux_idx_alloc(mux_idx_allocator, capri_expr.src1.hfname)
                sram['mux_idx'][mux_id1]['sel']['value'] = str(1)
                sram['mux_idx'][mux_id1]['lkpsel']['value'] = str(reg_id2)
                sram['mux_idx'][mux_id1]['load_stored_lkp']['value'] = str(0)
                sram['mux_idx'][mux_id1]['idx']['value'] = str(0)
        else:
            off = pkt_off1
            mux_id1 = mux_idx_alloc(mux_idx_allocator, off/8)
            sram['mux_idx'][mux_id1]['sel']['value'] = str(0)
            sram['mux_idx'][mux_id1]['lkpsel']['value'] = str(0)   # NA
            sram['mux_idx'][mux_id1]['idx']['value'] = str((off/8))

    return reg_id1, reg_id2, mux_id1, mux_id2

def _allocate_mux_inst_resoures2(capri_expr, nxt_cs, add_off, \
    mux_idx_allocator, sram):
    # support src1 and src2 to be either reg, pkt[] or current_offset
    reg_id1 = None; reg_id2 = None
    mux_id1 = None; mux_id2 = None

    if capri_expr.src_reg:
        off = -1
        if isinstance(capri_expr.src_reg, tuple):
            if capri_expr.src_reg[1] == 0:
                mux_id2 = mux_idx_alloc(mux_idx_allocator, 'current')
                sram['mux_idx'][mux_id2]['sel']['value'] = str(3)
                sram['mux_idx'][mux_id2]['lkpsel']['value'] = str(0)   # NA
                sram['mux_idx'][mux_id2]['idx']['value'] = str(0)
            else:
                off = capri_expr.src_reg[0] + (add_off*8)
        else:
            if capri_expr.src_reg.is_meta:
                if capri_expr.src_reg.hfname not in nxt_cs.lkp_flds:
                    reg_id1 = nxt_cs.active_reg_find(capri_expr.src_reg)
                else:
                    reg_id1 = nxt_cs.lkp_flds[capri_expr.src_reg.hfname].reg_id
            else:
                off = nxt_cs.fld_off[capri_expr.src_reg] + (add_off*8)
        if off >= 0:
            mux_id2 = mux_idx_alloc(mux_idx_allocator, off/8)
            sram['mux_idx'][mux_id2]['sel']['value'] = str(0)
            sram['mux_idx'][mux_id2]['lkpsel']['value'] = str(0)   # NA
            sram['mux_idx'][mux_id2]['idx']['value'] = str((off/8))

    if capri_expr.src1:
        off = -1
        if isinstance(capri_expr.src1, tuple):
            if capri_expr.src1[1] == 0:
                mux_id1 = mux_idx_alloc(mux_idx_allocator, 'current')
                sram['mux_idx'][mux_id1]['sel']['value'] = str(3)
                sram['mux_idx'][mux_id1]['lkpsel']['value'] = str(0)   # NA
                sram['mux_idx'][mux_id1]['idx']['value'] = str(0)
            else:
                off = capri_expr.src1[0] + (add_off*8)
        else:
            if capri_expr.src1.is_meta:
                if capri_expr.src1.hfname not in nxt_cs.lkp_flds:
                    reg_id2 = nxt_cs.active_reg_find(capri_expr.src1)
                else:
                    reg_id2 = nxt_cs.lkp_flds[capri_expr.src1.hfname].reg_id
                mux_id1 = mux_idx_alloc(mux_idx_allocator, capri_expr.src1.hfname)
                sram['mux_idx'][mux_id1]['sel']['value'] = str(1)
                sram['mux_idx'][mux_id1]['lkpsel']['value'] = str(reg_id2)
                sram['mux_idx'][mux_id1]['load_stored_lkp']['value'] = str(0)
                sram['mux_idx'][mux_id1]['idx']['value'] = str(0)
            else:
                off = nxt_cs.fld_off[capri_expr.src1] + (add_off*8)
        if off >= 0:
            mux_id1 = mux_idx_alloc(mux_idx_allocator, off/8)
            sram['mux_idx'][mux_id1]['sel']['value'] = str(0)
            sram['mux_idx'][mux_id1]['lkpsel']['value'] = str(0)   # NA
            sram['mux_idx'][mux_id1]['idx']['value'] = str((off/8))

    return reg_id1, reg_id2, mux_id1, mux_id2

def _fill_parser_sram_entry_for_csum(sram, parse_states_in_path, nxt_cs,            \
                                     parser, all_set_ops,                           \
                                     ohi_instr_allocated_count, ohi_inst_allocator, \
                                     mux_idx_allocator, mux_inst_allocator,         \
                                     mux_inst_id_to_mux_index_id_map,               \
                                     mux_inst_id_to_capri_expr_map,                 \
                                     mux_inst_id_to_ohi_id_map):

    csum_hdrlen_expr_found      = 0
    csum_l4len_expr_found       = 0
    csum_hdrlen_expr            = None
    csum_l4len_expr             = None

    for calfldobj in nxt_cs.verify_cal_field_objs:
        if calfldobj.hdrlen_verify_field != '':
            for s_ops, set_ops_field in enumerate(all_set_ops):
                if set_ops_field.dst.hfname == calfldobj.hdrlen_verify_field:
                    csum_hdrlen_expr_found = 1
                    break
            if csum_hdrlen_expr_found:
                hdrlen_const        = set_ops_field.const
                csum_hdrlen_expr    = set_ops_field.capri_expr
                break
    for calfldobj in nxt_cs.verify_cal_field_objs:
        if calfldobj.l4_verify_len_field != '':
            for s_ops, set_ops_field in enumerate(all_set_ops):
                if set_ops_field.dst.hfname == calfldobj.l4_verify_len_field:
                    csum_l4len_expr_found = 1
                    break
            if csum_l4len_expr_found:
                l4len_const         = set_ops_field.const
                csum_l4len_expr     = set_ops_field.capri_expr
                break

    csum_hdrlen_ohi_id          = -1
    csum_hdrlen_mux_instr_id    = -1
    csum_l4len_ohi_id           = -1
    csum_l4len_mux_instr_id     = -1
    if csum_hdrlen_expr:
        csum_hdrlen_mux_instr_id, _ = mux_inst_alloc(mux_inst_allocator, csum_hdrlen_expr)
        if csum_hdrlen_mux_instr_id in mux_inst_id_to_ohi_id_map:
            csum_hdrlen_ohi_id = mux_inst_id_to_ohi_id_map[csum_hdrlen_mux_instr_id]
    if csum_l4len_expr:
        csum_l4len_mux_instr_id, _ = mux_inst_alloc(mux_inst_allocator, csum_l4len_expr)
        if csum_l4len_mux_instr_id in mux_inst_id_to_ohi_id_map:
            csum_l4len_ohi_id = mux_inst_id_to_ohi_id_map[csum_l4len_mux_instr_id]

    csum_l4len_mux_idx_id       = -1
    csum_hdrlen_mux_idx_id      = -1
    if csum_l4len_ohi_id != -1 and csum_hdrlen_ohi_id != -1:
        #parse state updates l4 len and hdr len.
        csum_l4len_mux_idx_id, mux_id2 = mux_inst_id_to_mux_index_id_map[csum_l4len_mux_instr_id]
        csum_hdrlen_mux_idx_id, mux_id2 = mux_inst_id_to_mux_index_id_map[csum_hdrlen_mux_instr_id]
        ncc_assert(csum_l4len_mux_idx_id != -1 and csum_hdrlen_mux_idx_id != -1)

    if len(nxt_cs.verify_cal_field_objs) > 0:
        ohi_instr_allocated_count = parser.be.checksum.CsumParserConfigGenerate(parser, \
                                               parse_states_in_path, nxt_cs, sram,\
                                               ohi_instr_allocated_count,\
                                               ohi_inst_allocator, mux_idx_allocator,\
                                               mux_inst_allocator, \
                                               csum_l4len_mux_instr_id, csum_l4len_mux_idx_id, \
                                               csum_hdrlen_mux_idx_id, csum_hdrlen_expr, csum_l4len_expr)
    elif nxt_cs.phdr_offset_ohi_id != -1:
        # Case where parse state is moving into ipv4/ipv6
        # and ipv4 hdr is not part of header checksum verification
        #   - an ohi_slot is allocated to capture start of IP hdr which
        #     serves as start of phdr
        #   - store payloadLen in OhiSlot used in case of ipv4 -> tcp;
        #     In case of v6 -> TCP, since v6 options need to be decremented
        #     from payloadLen, store v6.payload in stored lookup register.
        ohi_instr_allocated_count = parser.be.checksum.CsumParserPhdrOffsetInstrGenerate(\
                                           nxt_cs, sram, ohi_instr_allocated_count,\
                                           ohi_inst_allocator, mux_idx_allocator,\
                                           mux_inst_allocator)

    #Gso Csum Config
    if nxt_cs.gso_csum_calfldobj:
        parser.be.checksum.GsoCsumParserConfigGenerate(parser, \
                                               parse_states_in_path, nxt_cs, sram)
    return ohi_instr_allocated_count


def _fill_parser_sram_entry(parse_states_in_path, sram_t, parser, bi, add_cs = None):
    parser.logger.debug("%s:fill_sram_entry for %s + %s" % \
        (parser.d.name, bi.nxt_state, add_cs))
    sram = copy.deepcopy(sram_t)
    sram['_modified'] = True

    # add_cs is valid when this is the first state that h/w enters via cfg registers
    # add extractions, setops, hv bits etc from add_cs to this
    add_off = 0 # Bytes
    if add_cs:
        add_off = add_cs.extract_len    # in Bytes
        ncc_assert(isinstance(add_off, int), \
            "Variable size extraction not allowed on start state %s" % \
                (add_cs.name))

    mux_idx_allocator = [None for _ in sram['mux_idx']]
    mux_inst_allocator = [(None, None, None) for _ in sram['mux_inst']]
    mux_inst_id_to_mux_index_id_map = {} # mux_instr_id ->  mux_idx map.
    mux_inst_id_to_capri_expr_map = {}
    mux_inst_id_to_ohi_id_map = {}

    nxt_cs = bi.nxt_state
    sram['action']['value'] = str(1) if nxt_cs.is_end else str(0)
    sram['nxt_state']['value'] = str(nxt_cs.id)

    current_flit = None

    all_set_ops = []
    for elem in nxt_cs.set_ops:
        all_set_ops.append(elem)
    for elem in nxt_cs.no_reg_set_ops:
        all_set_ops.append(elem)

    # lkp_val_inst
    for r,lkp_reg in enumerate(nxt_cs.lkp_regs):
        if add_cs != None and add_cs.lkp_regs[r].store_en:
            # make sure if register is stored in start state then it is not loaded in nxt_cs
            if lkp_reg.type != lkp_reg_type.LKP_REG_NONE and \
                lkp_reg.type != lkp_reg_type.LKP_REG_RETAIN and \
                lkp_reg.type != lkp_reg_type.LKP_REG_STORED:
                parser.logger.critical( \
                        "Cannot store this registers in state %s as it is loaded in next state %s" % \
                        (add_cs.name, nxt_cs.name))
                ncc_assert(0)
                ncc_assert(0, "Add a dummy start state to avoid this problem")

        if lkp_reg.type == lkp_reg_type.LKP_REG_NONE:
            continue
        elif lkp_reg.type == lkp_reg_type.LKP_REG_PKT:
            mux_id = mux_idx_alloc(mux_idx_allocator, (lkp_reg.pkt_off/8) + add_off)
            sram['mux_idx'][mux_id]['sel']['value'] = str(0)
            sram['mux_idx'][mux_id]['lkpsel']['value'] = str(0)   # NA
            sram['mux_idx'][mux_id]['idx']['value'] = str((lkp_reg.pkt_off/8) + add_off)

            sram['lkp_val_inst'][r]['sel']['value'] = str(0)
            sram['lkp_val_inst'][r]['muxsel']['value'] = str(mux_id)

        elif lkp_reg.type == lkp_reg_type.LKP_REG_RETAIN:
            sram['lkp_val_inst'][r]['sel']['value'] = str(2)
            sram['lkp_val_inst'][r]['muxsel']['value'] = str(0)  # NA

        elif lkp_reg.type == lkp_reg_type.LKP_REG_STORED:
            if add_cs != None:
                # convert stored to retain as we did not have a chance to store them
                # retain and save for future use
                sram['lkp_val_inst'][r]['sel']['value'] = str(2)
                sram['lkp_val_inst'][r]['muxsel']['value'] = str(0)  # NA
            else:
                sram['lkp_val_inst'][r]['sel']['value'] = str(3)
                sram['lkp_val_inst'][r]['muxsel']['value'] = str(0)  # NA
        else: # REG_LOAD/UPDATE
            if lkp_reg.capri_expr:
                # with new mux_instruction, we can now supply two mux_idx/reg_ids, depending on
                # the operands src_reg and src1. Each of these operands can be either a lkp_reg or
                # pkt_field
                # mux_inst natively supports one lkpsel (reg) second reg must use mux_sel and vice-
                # versa for pkt field
                reg_id1, reg_id2, mux_id1, mux_id2 = \
                        _allocate_mux_inst_resoures2(lkp_reg.capri_expr, nxt_cs, add_off, \
                            mux_idx_allocator, sram)

                # build mux instruction
                mux_inst_id, _ = mux_inst_alloc(mux_inst_allocator, lkp_reg.capri_expr)
                _build_mux_inst2(parser, nxt_cs, sram['mux_inst'][mux_inst_id], reg_id1, reg_id2,
                        mux_id1, mux_id2, lkp_reg.capri_expr)

                # >>>
                mux_inst_id_to_capri_expr_map[mux_inst_id] = lkp_reg.capri_expr
                mux_inst_id_to_mux_index_id_map[mux_inst_id] = (mux_id1, mux_id2)
                sram['lkp_val_inst'][r]['sel']['value'] = str(1)
                sram['lkp_val_inst'][r]['muxsel']['value'] = str(mux_inst_id)

            else:
                # local var load from pkt
                if lkp_reg.first_pkt_fld:
                    ncc_assert(lkp_reg.pkt_off != -1)
                    off = lkp_reg.pkt_off
                    mux_id = mux_idx_alloc(mux_idx_allocator, (lkp_reg.pkt_off/8) + add_off)
                    sram['mux_idx'][mux_id]['sel']['value'] = str(0)
                    sram['mux_idx'][mux_id]['lkpsel']['value'] = str(0)   # NA
                    sram['mux_idx'][mux_id]['idx']['value'] = str((lkp_reg.pkt_off/8) + add_off)
                else:
                    # Load current_offset
                    ncc_assert(lkp_reg.pkt_off == -1)
                    mux_id = mux_idx_alloc(mux_idx_allocator, 'current')
                    sram['mux_idx'][mux_id]['sel']['value'] = str(3)
                    sram['mux_idx'][mux_id]['lkpsel']['value'] = str(0)   # NA
                    sram['mux_idx'][mux_id]['idx']['value'] = str(0) # NA

                sram['lkp_val_inst'][r]['sel']['value'] = str(0)
                sram['lkp_val_inst'][r]['muxsel']['value'] = str(mux_id)


        if lkp_reg.store_en:
            sram['lkp_val_inst'][r]['store_en']['value'] = str(1)
        elif add_cs != None and add_cs.lkp_regs[r].store_en:
            sram['lkp_val_inst'][r]['store_en']['value'] = str(1)
        else:
            sram['lkp_val_inst'][r]['store_en']['value'] = str(0)

    # special handling for add_cs lkp_regs that need to be stored,
    # since start_state(indicated by add_cs) is programmed via init_profile registers,
    # there was no chance to save these registers. This is handled (somewhat) in the above
    # code, but when register is not used in nxt_cs, it got missed
    if add_cs != None:
        for r,lkp_reg in enumerate(add_cs.lkp_regs):
            if lkp_reg.type == lkp_reg_type.LKP_REG_NONE or not lkp_reg.store_en:
                continue
            # may be create an empty start state ???
            sram['lkp_val_inst'][r]['sel']['value'] = str(2)     # Retain
            sram['lkp_val_inst'][r]['muxsel']['value'] = str(0)  # NA
            sram['lkp_val_inst'][r]['store_en']['value'] = str(1)
    #lkp instructions allocated so far
    lkp_instr_inst = r

    #Generate instructions to evaluate expressions that are not
    #used for lookup purposes but other pursposes (csum, icrc, option len computation etc).
    for set_op in nxt_cs.no_reg_set_ops:
        ncc_assert(set_op.capri_expr)
        if set_op.capri_expr:
            reg_id1, reg_id2, mux_id1, mux_id2 = \
                    _allocate_mux_inst_resoures2(set_op.capri_expr, nxt_cs, add_off, \
                        mux_idx_allocator, sram)

            # build mux instruction
            mux_inst_id, _ = mux_inst_alloc(mux_inst_allocator, set_op.capri_expr)
            _build_mux_inst2(parser, nxt_cs, sram['mux_inst'][mux_inst_id], reg_id1, reg_id2,
                    mux_id1, mux_id2, set_op.capri_expr, add_off)

            mux_inst_id_to_capri_expr_map[mux_inst_id] = set_op.capri_expr
            mux_inst_id_to_mux_index_id_map[mux_inst_id] = (mux_id1, mux_id2)
        else:
            ncc_assert(0, "Must use expression when writing to write-only register")

    # extract_inst
    # For all fields that go to phv, check if fields can be combined to extract
    # 4 bytes at a time (Needed only if more than 16 fields are extracted)
    # ASSUME pre-extraction i.e. fields from a state are extracted *before* entering the
    # state
    parser_num_flits = parser.be.hw_model['parser']['parser_num_flits']
    flit_size = parser.be.hw_model['parser']['flit_size']
    flit_sizeB = flit_size / 8

    insts = []
    if not nxt_cs.no_extract():
        insts = parser.generate_extract_instructions(nxt_cs, add_cs)
    for i in range(len(insts)):
        if current_flit != None:
            ncc_assert(current_flit == (insts[i][2] / flit_sizeB))
        else:
            current_flit = insts[i][2] / flit_sizeB
    #ncc_assert(len(insts) <= len(sram['extract_inst']), "%s:Too many(%d) extractions" % \
    #    (nxt_cs.name, len(insts)))
    if len(insts) > len(sram['extract_inst']):
        parser.logger.critical("Violation:%s:Too many(%d) extractions" % \
            (nxt_cs.name, len(insts)))
        ncc_assert(0   )# possibly internal error, may need state splitting

    for e, ext_ins in enumerate(insts):
        sram['extract_inst'][e]['pkt_idx']['value'] = str(ext_ins[0])
        sram['extract_inst'][e]['len']['value']     = str(ext_ins[1])
        sram['extract_inst'][e]['phv_idx']['value'] = str(ext_ins[2] & 0x7F) # remove flit bits

    #pdb.set_trace()
    # ohi_inst


    hw_max_ohi_per_state = len(sram['ohi_inst'])
    s = 0
    headers = []
    ohi_inst_allocator = [None for _ in range(hw_max_ohi_per_state)] # (fld_name, ohi_id)
    if add_cs and not add_cs.no_extract():
        headers += add_cs.headers
    if not nxt_cs.no_extract():
        headers += nxt_cs.headers

    for hdr in headers:
        if hdr not in parser.ohi:
            continue
        if add_cs and hdr in add_cs.headers:
            cs = add_cs
        else:
            cs = nxt_cs

        for cf in cs.extracted_fields:
            if cf.get_p4_hdr() == hdr:
                hdr_off = (cs.fld_off[cf] - cf.p4_fld.offset) / 8 # can be -ve
                break
        if cs == nxt_cs:
            hdr_off += add_off

        for ohi in parser.ohi[hdr]:
            # If header is split across states, need to split OHI
            # Note that header fields are not split across states
            # check if any ohi field of this header is extracted in this state
            found = False
            for cf in cs.extracted_fields:
                if not cf.is_ohi or cf.get_p4_hdr() != hdr:
                    continue
                if (cf.p4_fld.offset/8) == ohi.start:
                    found = True
                    break

            if not found:
                continue

            if ohi.id == -1 and ohi.var_id == -1:
                # no need to store this offset/len info
                continue

            ncc_assert(s < hw_max_ohi_per_state )# 'Too many OHIs in a state %s' % cs.name

            if ohi.id == -1:
                ncc_assert(len(parser.ohi[hdr]) > 1 )# ERROR
            else:
                # slot[0] : pkt offset
                '''
                if hdr_off != 0:
                    # split state check ???
                    # XXXX looks incorrect code....
                    pdb.set_trace()
                    ohi_id = parser.get_ohi_slot()
                    ohi.id = ohi_id
                '''
                sram['ohi_inst'][s]['sel']['value'] = str(1)    # provide pkt_offset
                sram['ohi_inst'][s]['muxsel']['value'] = str(0) # NA
                sram['ohi_inst'][s]['idx_val']['value'] = str(hdr_off + ohi.start)
                sram['ohi_inst'][s]['slot_num']['value'] = str(ohi.id)
                if ohi.start == 0:
                    ohi_inst_allocator[s] = (hdr.name + '___start_off', ohi.id)
                s += 1

            if not isinstance(ohi.length, int):
                # ohi.length is a capri expression
                mux_inst_id, _ = mux_inst_alloc(mux_inst_allocator, ohi.length)

                #pdb.set_trace()
                # special case for option_blob where ohi len comes from another header field
                if ohi.length.src1:
                    ohi_len_fld_off = (cs.fld_off[ohi.length.src1]/8) + add_off
                    # pkt_mux provides correct pkt field for calculation
                    mux_id = mux_idx_alloc(mux_idx_allocator, ohi_len_fld_off)
                    sram['mux_idx'][mux_id]['sel']['value'] = str(0)
                    sram['mux_idx'][mux_id]['lkpsel']['value'] = str(0)   # NA
                    sram['mux_idx'][mux_id]['idx']['value'] = str(ohi_len_fld_off)
                    reg_id = -1
                elif ohi.length.src_reg:
                    ncc_assert(ohi.length.src_reg.hfname in nxt_cs.lkp_flds, \
                        "%s:%s:ohi len fld %s is not available" % \
                        (parser.d.name, nxt_cs.name, ohi.length.src_reg.hfname))
                    reg_id = nxt_cs.lkp_flds[ohi.length.src_reg.hfname].reg_id
                    mux_id = -1
                else:
                    ncc_assert(0)

                _build_mux_inst(parser, cs, reg_id,
                    sram['mux_inst'][mux_inst_id], mux_id, ohi.length)
                mux_inst_id_to_capri_expr_map[mux_inst_id] = ohi.length
                mux_inst_id_to_mux_index_id_map[mux_inst_id] = (mux_id, None)
                # slot[1] : ohi_len
                sram['ohi_inst'][s]['sel']['value'] = str(3)  # len using mux_inst_data
                sram['ohi_inst'][s]['muxsel']['value'] = str(mux_inst_id)
                sram['ohi_inst'][s]['idx_val']['value'] = str(0) # NA
                sram['ohi_inst'][s]['slot_num']['value'] = str(ohi.var_id)
                mux_inst_id_to_ohi_id_map[mux_inst_id] = s
                ohi_inst_allocator[s] = (hdr.name + '___hdr_len', ohi.var_id)
                parser.logger.debug('OHI instruction[%d]: ohi_slot %d off %d, len %s' % \
                    (s, ohi.var_id, ohi.start + hdr_off, ohi.length))
                flat_expr_str = ohi.length.flatten_capri_expr()
                s += 1
            else:
                # for fixed size ohi, deparser is programmed with fixed length
                parser.logger.debug('OHI instruction[%d]: ohi_slot %d off %d, len %s' % \
                     (s, ohi.id, ohi.start + hdr_off, ohi.length))
                pass

    # add ohi instructions for write-only variables
    for set_op in nxt_cs.no_reg_set_ops:
        ncc_assert(set_op.capri_expr)
        ohi_id = None
        # remove hdr name
        dst_name = set_op.dst.hfname.split('.')[1]
        for ohi_inst,v in enumerate(ohi_inst_allocator):
            if not v:
                continue
            if dst_name == v[0]:
                ohi_id = v[1]
                break
        if ohi_id != None:
            # already programmed
            # pdb.set_trace()
            parser.logger.debug("%s:OHI (wr-only) %s shared ohi_id %d ohi_instruction %d" % \
                (parser.d.name, dst_name, ohi_id, ohi_inst))
            continue
        #pdb.set_trace()
        if dst_name not in parser.wr_only_ohi:
            parser.logger.debug("OHI (wr-only) %s is not programmed here - TBD csum changes" % \
                (set_op.dst))
            continue
        # create new ohi instruction, mux inst may be already allocated
        ncc_assert(s < hw_max_ohi_per_state)
        mux_inst_id, _ = mux_inst_alloc(mux_inst_allocator, set_op.capri_expr)
        mux_inst_id_to_capri_expr_map[mux_inst_id] = set_op.capri_expr
        ohi_id = parser.wr_only_ohi[dst_name]
        sram['ohi_inst'][s]['sel']['value'] = str(3)  # len using mux_inst_data
        sram['ohi_inst'][s]['muxsel']['value'] = str(mux_inst_id)
        sram['ohi_inst'][s]['idx_val']['value'] = str(0) # NA
        sram['ohi_inst'][s]['slot_num']['value'] = str(ohi_id)
        mux_inst_id_to_ohi_id_map[mux_inst_id] = ohi_id
        ohi_inst_allocator[s] = (dst_name, ohi_id)
        parser.logger.debug('OHI (wr-only) instruction[%d]: ohi_slot %d, %s=%s' %
                            (s, ohi_id, dst_name, set_op))
        s += 1

    # Build offset instruction at the end so that if possible
    # mux instructions can be re-used by adjusting constant values
    # for pre extraction, use info from next state for offset
    # offset_inst
    offset_inst = sram['offset_inst']
    if nxt_cs.dont_advance_packet():
        # to be able to extract same header over and over (e.g. tcp_options_blob and individual
        # tcp options), state may be marked with dont_advance_packet
        offset_inst['sel']['value'] = str(0)
        offset_inst['muxsel']['value'] = str(0)
        offset_inst['val']['value'] = str(0)
    else:
        if isinstance(nxt_cs.extract_len, int):
            offset_inst['sel']['value'] = str(0)
            offset_inst['muxsel']['value'] = str(0)
            offset_inst['val']['value'] = str(nxt_cs.extract_len + add_off)
            if (nxt_cs.extract_len + add_off) != 0:
                # enable jump check if moving the offset
                sram['offset_jump_chk_en']['value'] = str(1)

            if (nxt_cs.extract_len + add_off) != 0:
                sram['pkt_size_chk_en']['value'] = str(1)
                if (nxt_cs.extract_len + add_off) == 64:
                    sram['pkt_size']['value'] = str(0)
                else:
                    sram['pkt_size']['value'] = str(nxt_cs.extract_len + add_off)
        else:
            #ncc_assert(isinstance(nxt_cs.extract_len, capri_parser_expr))
            expr_const = nxt_cs.extract_len.const
            if nxt_cs.capture_payload_offset():
                adjust_const = False
            else:
                adjust_const = True
            mux_inst_id, adjusted_const = mux_inst_alloc(mux_inst_allocator,\
                                                         nxt_cs.extract_len,\
                                                         adjust_const=adjust_const)
            mux_inst_id_to_capri_expr_map[mux_inst_id] = nxt_cs.extract_len
            off = 0
            if nxt_cs.extract_len.src1:
                off = nxt_cs.fld_off[nxt_cs.extract_len.src1]
                # use add_off to access pkt field from nxt_cs
                mux_id = mux_idx_alloc(mux_idx_allocator, (off/8)+add_off)
                sram['mux_idx'][mux_id]['sel']['value'] = str(0)
                sram['mux_idx'][mux_id]['lkpsel']['value'] = str(0)   # NA
                sram['mux_idx'][mux_id]['idx']['value'] = str((off/8)+add_off)
                reg_id = -1
            elif nxt_cs.extract_len.src_reg:
                ncc_assert(ohi.length.src_reg.hfname in nxt_cs.lkp_flds, \
                    "%s:%s:ohi len fld %s is not available" % \
                    (parser.d.name, nxt_cs.name, ohi.length.src_reg.hfname))
                reg_id = nxt_cs.lkp_flds[ohi.length.src_reg.hfname].reg_id
                mux_id = -1
            else:
                ncc_assert(0)
            _build_mux_inst(parser, nxt_cs, reg_id,
                sram['mux_inst'][mux_inst_id], mux_id, nxt_cs.extract_len)
            mux_inst_id_to_mux_index_id_map[mux_inst_id] = (mux_id, None)

            offset_inst['sel']['value'] = str(1)
            offset_inst['muxsel']['value'] = str(mux_inst_id)
            # add_off is not added into expr, instead it is added by h/w using 'value'
            offset_inst['val']['value'] = str(add_off + adjusted_const)
            # enable jump check since we are moving the offset
            sram['offset_jump_chk_en']['value'] = str(1)

            if adjusted_const:
                #Reset constant value back to original so that when processing
                #all parser branches from parse-state, original
                #expression is used.
                nxt_cs.extract_len.const = expr_const


    #Generate Checksum related Configuration in parser.
    s = _fill_parser_sram_entry_for_csum(sram, parse_states_in_path, nxt_cs, parser,\
                                         all_set_ops, s, ohi_inst_allocator,        \
                                         mux_idx_allocator, mux_inst_allocator,     \
                                         mux_inst_id_to_mux_index_id_map,           \
                                         mux_inst_id_to_capri_expr_map,             \
                                         mux_inst_id_to_ohi_id_map)

    #Generate ICRC related Configuration in parser.
    if len(nxt_cs.icrc_verify_cal_field_objs):
        parser.be.icrc.IcrcParserConfigGenerate(parser, parse_states_in_path, nxt_cs, sram)

    if nxt_cs.is_end:
        ncc_assert(s < hw_max_ohi_per_state)

    if nxt_cs.capture_payload_offset():
        # need to capture current_offset where parser stops parsing. This is needed for the
        # deparser, 
        # Earlier offset instruction programming was used to reuse the code and expression,
        # but that is incorrect, especially when dont_advance_offset pragma is also used
        # is used, offset_instruction will be 0
        ohi_payload_slot = parser.be.hw_model['parser']['ohi_threshold']
        ncc_assert(s < hw_max_ohi_per_state)# 'No OHI instr available in end state %s' % cs.name
        if isinstance(nxt_cs.extract_len, int):
            sram['ohi_inst'][s]['sel']['value'] = str(1)    # current_offset + idx_val
            sram['ohi_inst'][s]['muxsel']['value'] = str(0) # NA
            sram['ohi_inst'][s]['idx_val']['value'] = str(nxt_cs.extract_len + add_off)
            sram['ohi_inst'][s]['slot_num']['value'] = str((ohi_payload_slot))
        else:
            ncc_assert(add_off == 0, "XXX add_off and exit is not a common case, need to support this")
            # since extract len is an expression
            # create new expression for payload offset as
            # payload_offset = current_offset + extract_len
            # Ideally this should be done in capri_parser.py... this is a quick HACK
            if not nxt_cs.payload_offset_expr:
                nxt_cs.create_payload_offset_expr()

            pyld_expr = nxt_cs.payload_offset_expr

            reg_id1, reg_id2, mux_id1, mux_id2 = \
                    _allocate_mux_inst_resoures2(pyld_expr, nxt_cs, add_off, \
                        mux_idx_allocator, sram)

            # build mux instruction
            mux_inst_id, _ = mux_inst_alloc(mux_inst_allocator, pyld_expr)
            _build_mux_inst2(parser, nxt_cs, sram['mux_inst'][mux_inst_id], reg_id1, reg_id2,
                    mux_id1, mux_id2, pyld_expr)

            mux_inst_id_to_capri_expr_map[mux_inst_id] = pyld_expr
            mux_inst_id_to_mux_index_id_map[mux_inst_id] = (mux_id1, mux_id2)

            sram['ohi_inst'][s]['sel']['value'] = str(3)    # use mux inst
            sram['ohi_inst'][s]['muxsel']['value'] =  str(mux_inst_id)
            sram['ohi_inst'][s]['idx_val']['value'] = str(0)
            sram['ohi_inst'][s]['slot_num']['value'] = str((ohi_payload_slot))

        parser.logger.debug('%s:%s:Payload Offset: OHI instruction[%d] slot %d' % \
                            (parser.d.name, nxt_cs.name, s, ohi_payload_slot))

    # meta_inst
    # meta inst for hv bits -
    # use all headers from add_cs and nxt_cs
    hv_bits = OrderedDict() # {byte: val}
    # special processing for i2e_meta header
    if add_cs:
        i2e_hdr = None
        if parser.d == xgress.INGRESS:
            i2e_hdr = parser.be.pa.gress_pa[parser.d].i2e_hdr
        if i2e_hdr:
            headers.append(i2e_hdr)
    for hdr in headers:
        # special processing of i2e header
        if parser.d == xgress.EGRESS and hdr == parser.be.pa.gress_pa[parser.d].i2e_hdr:
            continue # do not set the hv_bit for meta hdr on egress
        # set hv_bit - collect hv bits set in this state
        # new hw feature does not require hv bits per flit
        hvidx = parser.hdr_hv_bit[hdr]
        hv_byte = hvidx/8
        boff = hvidx % 8
        if hv_byte in hv_bits:
            hv_bits[hv_byte] |= (1 << (7 - boff))  # hv_bit0 -> MSbit
        else:
            hv_bits[hv_byte] = (1 << (7 - boff))

    if nxt_cs.is_end:
        # phv_hv_bits are between 384(hv_location)-511
        hv_location = parser.be.hw_model['parser']['phv_pkt_len_location']
        max_hv_bit_idx = hv_location # Use last bit in BE order for payload len hv_en
        hv_byte = max_hv_bit_idx / 8
        boff = max_hv_bit_idx % 8
        if hv_byte in hv_bits:
            hv_bits[hv_byte] |= (1 << (7 - boff))
        else:
            hv_bits[hv_byte] = (1 << (7 - boff))

    mid = 0
    max_mid = len(sram['meta_inst'])
    # select values
    meta_ops = parser.be.hw_model['parser']['parser_consts']['meta_ops']
    for hv_byte,hv in hv_bits.items():
        ncc_assert(mid < max_mid)
        sram['meta_inst'][mid]['sel']['value'] = meta_ops['set_hv']
        sram['meta_inst'][mid]['phv_idx']['value'] = str(hv_byte)
        sram['meta_inst'][mid]['val']['value'] = str(hv)
        mid += 1

    # meta instructions for register/const -> phv (no OR)
    # add meta instructions from add_cs if specified
    set_ops = []
    if add_cs:
        set_ops += add_cs.set_ops
    set_ops += nxt_cs.set_ops
    for op in set_ops:
        #pdb.set_trace()
        mux_inst_id = None
        dst_phv = None
        val = 0
        use_or = False
        if op.op_type == meta_op.EXTRACT_REG:
            #pdb.set_trace() # un-tested so far
            ncc_assert(op.rid != None)
            mux_inst_id, _ = mux_inst_alloc(mux_inst_allocator, op.capri_expr)
            _build_mux_inst(parser, op.cstate, op.rid, sram['mux_inst'][mux_inst_id],
                0, op.capri_expr)
            dst_phv = op.dst.phv_bit
        elif op.op_type == meta_op.EXTRACT_META:
            #pdb.set_trace()
            ncc_assert(op.src1)
            mux_inst_id, _ = mux_inst_alloc(mux_inst_allocator, op.capri_expr)
            op_off = op.cstate.fld_off[op.src1]
            if op.cstate == nxt_cs:
                fld_off = (op_off/8) + add_off
            else:
                fld_off = (op_off/8)

            mux_id = mux_idx_alloc(mux_idx_allocator, fld_off)
            sram['mux_idx'][mux_id]['sel']['value'] = str(0)
            sram['mux_idx'][mux_id]['lkpsel']['value'] = str(0)   # NA
            sram['mux_idx'][mux_id]['idx']['value'] = str(fld_off)
            _build_mux_inst(parser, op.cstate, None, sram['mux_inst'][mux_inst_id],
                mux_id, op.capri_expr)
            dst_phv = op.dst.phv_bit
        elif op.op_type == meta_op.EXTRACT_CURRENT_OFF:
            ncc_assert(isinstance(op.src1, tuple))
            ncc_assert(op.src1[1] == 0)
            ncc_assert(op.capri_expr, "Must use expression")
            mux_inst_id, _ = mux_inst_alloc(mux_inst_allocator, op.capri_expr)
            mux_id = mux_idx_alloc(mux_idx_allocator, 'current')
            sram['mux_idx'][mux_id]['sel']['value'] = str(3)
            sram['mux_idx'][mux_id]['lkpsel']['value'] = str(0)   # NA
            sram['mux_idx'][mux_id]['idx']['value'] = str(0)
            _build_mux_inst(parser, op.cstate, None, sram['mux_inst'][mux_inst_id],
                mux_id, op.capri_expr)
            dst_phv = op.dst.phv_bit
        elif op.op_type == meta_op.EXTRACT_CONST:
            #pdb.set_trace()
            dst_phv = op.dst.phv_bit
            #ncc_assert((dst_phv % 8) == 0, "Destination phv %s must be byte aligned" % op.dst.hfname)
            val = op.const
            if (dst_phv %8) or (op.dst.width % 8):
                # if not writing integral byte, OR the value
                ncc_assert(op.dst.width < 8, "%s:%s must be <8bit wide to use set_metadata()" % \
                    (nxt_cs, op.dst.hfname))
                use_or = True
        else:
            continue

        flit = dst_phv / flit_size
        if current_flit == None:
            current_flit = flit
        ncc_assert(current_flit == flit)

        ncc_assert(mid < max_mid, "%s:%s:Out of meta instructions %s" % (parser.d, nxt_cs.name, set_ops))

        sram['meta_inst'][mid]['phv_idx']['value'] = str((dst_phv / 8) & 0x7F)
        if mux_inst_id == None:
            if use_or:
                sram['meta_inst'][mid]['sel']['value'] = meta_ops['or_val']
            else:
                sram['meta_inst'][mid]['sel']['value'] = meta_ops['set_val']
            sram['meta_inst'][mid]['val']['value'] = str(val)
        else:
            if use_or:
                sram['meta_inst'][mid]['sel']['value'] = meta_ops['or_mux_data']
            else:
                sram['meta_inst'][mid]['sel']['value'] = meta_ops['set_mux_data']
            sram['meta_inst'][mid]['val']['value'] = str(mux_inst_id)
        mid += 1

    # fill unused instructions to do NOP - XXX
    # set up the unused meta instr to perform |= 0
    for m in range(mid, max_mid):
        sram['meta_inst'][m]['sel']['value'] = meta_ops['nop']
        sram['meta_inst'][m]['phv_idx']['value'] = str(0)
        sram['meta_inst'][m]['val']['value'] = str(0)

    for m,used in enumerate(mux_idx_allocator):
        if used != None:
            continue
        sram['mux_idx'][m]['sel']['value'] = str(3) # load offset - Do not access packet

    if current_flit != None:
        parser.logger.debug("%s:%s:Flit # %d" % (parser.d.name, nxt_cs.name, current_flit))
        sram['phv_idx_upr']['value'] = str(current_flit)
    else:
        parser.logger.debug("%s:%s:No PHV writes in this state" % (parser.d.name, nxt_cs.name))

    # program len_chk instructions if specified
    len_chk_inst_id = 0
    max_len_chk_inst = parser.be.hw_model['parser']['num_len_chk_inst']
    ncc_assert(len(nxt_cs.len_chk_profiles) <= max_len_chk_inst)
    for len_chk_prof in nxt_cs.len_chk_profiles:
        ncc_assert(len_chk_prof.start_hfname)
        ncc_assert(len_chk_prof.len_hfname)
        start_cfname = len_chk_prof.start_hfname.split('.')[1]
        len_cfname = len_chk_prof.len_hfname.split('.')[1]
        ncc_assert(start_cfname in parser.wr_only_ohi)
        ncc_assert(len_cfname in parser.wr_only_ohi)
        start_ohi_slot = parser.wr_only_ohi[start_cfname]
        len_ohi_slot = parser.wr_only_ohi[len_cfname]
        sram['len_chk_inst'][len_chk_inst_id]['en']['value'] = str(1)
        # use same prof and unit ids - TBD if we need more profiles and share units
        sram['len_chk_inst'][len_chk_inst_id]['unit_sel']['value'] = str(len_chk_prof.prof_id)
        sram['len_chk_inst'][len_chk_inst_id]['prof_sel']['value'] = str(len_chk_prof.prof_id)
        sram['len_chk_inst'][len_chk_inst_id]['ohi_start_sel']['value'] = str(start_ohi_slot)
        sram['len_chk_inst'][len_chk_inst_id]['ohi_len_sel']['value'] = str(len_ohi_slot)
        if len_chk_prof.cmp_op == 'eq':
            sram['len_chk_inst'][len_chk_inst_id]['exact']['value'] = str(1)
        else:
            sram['len_chk_inst'][len_chk_inst_id]['exact']['value'] = str(0)
        len_chk_inst_id += 1

    return sram

def _fill_parser_sram_catch_all(sram_t):
    sram = copy.deepcopy(sram_t)
    sram['_modified'] = True
    # mark this as 'end' state
    sram['action']['value'] = str(1)
    sram['nxt_state']['value'] = str(0)
    sram['offset_inst']['value'] = str(0)
    for ext_inst in sram['extract_inst']:
        ext_inst['len']['value'] = str(0)
    for meta_inst in sram['meta_inst']:
        meta_inst['sel']['value'] = str(0) # nop
        meta_inst['phv_idx']['value'] = str(0)
        meta_inst['val']['value'] = str(0)
    for ohi_inst in sram['ohi_inst']:
        ohi_inst['sel']['value'] = str(0)
    return sram

def _fill_parser_tcam_catch_all(tcam_t):
    # Catch-all entry
    te = copy.deepcopy(tcam_t)
    te['_modified'] = True
    te['key']['control']['value'] = str(0)
    te['mask']['control']['value'] = str(0)
    te['key']['state']['value'] = "0"
    te['mask']['state']['value'] = "0"
    for r,_ in enumerate(te['mask']['lkp_val']):
        te['mask']['lkp_val'][r]['value'] = "0"
        te['key']['lkp_val'][r]['value'] = "0"
    te['valid']['value'] = str(1)
    return te

def _fill_parser_tcam_entry(tcam_t, parser, cs, bi):
    # tcam entry is created for lookup (current_state_id, lkp_regs => bi.value, bi.mask
    te = copy.deepcopy(tcam_t)
    te['_modified'] = True
    hw_mask = [0 for i in range(cs.hw_lkp_size)]
    hw_val = [0 for i in range(cs.hw_lkp_size)]
    for i,b in enumerate(bi.mask):
        if b:
            hw_mask[cs.key_l2p_map[i]] = 1

    for i in range(cs.key_len):
        if bi.val & (1<<(cs.key_len-1-i)):
            hw_val[cs.key_l2p_map[i]] = 1

    # convert mask per lkp_reg (lkp regs are only used to get the sizes here)
    boff = 0
    # convert the hardware val and mask to integer

    r_mask = [0 for i in cs.lkp_regs]
    r_key = [0 for i in cs.lkp_regs]
    for r, reg in enumerate(cs.lkp_regs):
        bit_mask = (1<<(reg.size-1))
        for b in range(reg.size):
            if hw_mask[boff+b]:
                r_mask[r] |= (bit_mask >> b)
            if hw_val[boff+b]:
                r_key[r] |= (bit_mask >> b)
        boff += reg.size

    # ignore control bits
    te['key']['control']['value'] = str(0)
    te['mask']['control']['value'] = str(0)
    te['key']['state']['value'] = "0x%x" % cs.id
    te['mask']['state']['value'] = "0x%x" % (~cs.id & 0x1FF)
    for i in range(len(r_mask)):
        # use X, Y values as key and mask respectively
        te['key']['lkp_val'][i]['value'] = "0x%x" % (r_key[i] & r_mask[i])
        te['mask']['lkp_val'][i]['value'] = "0x%x" % ((r_key[i] ^ r_mask[i]) & r_mask[i])
    te['valid']['value'] = str(1)

    return te

def capri_parser_output_decoders(parser):
    # read the register spec json
    cur_path = os.path.abspath(__file__)
    cur_path = os.path.split(cur_path)[0]
    gen_dir = parser.be.args.gen_dir
    ppa_file_path = os.path.join(cur_path, 'csr_json/cap_ppa_csr.json')
    ppa_file = open(ppa_file_path)
    ppa_json = json.load(ppa_file)
    ppa_decoder_file_path = os.path.join(cur_path, 'csr_json/cap_ppa_decoders.json')
    ppa_decoder_file = open(ppa_decoder_file_path)
    ppa_decoder_json = json.load(ppa_decoder_file, object_pairs_hook=OrderedDict)
    cfg_out_dir = os.path.join(gen_dir + '/%s/cfg_out' % parser.be.prog_name)
    if not os.path.exists(cfg_out_dir):
        try:
            os.makedirs(cfg_out_dir)
        except OSError as e:
            if e.errno != errno.EEXIST:
                raise
    ppa_cfg_output_mem = os.path.join(cfg_out_dir, 'cap_ppa_%s_cfg_decoder_mem.json' % \
        parser.d.name)
    ppa_cfg_output_reg = os.path.join(cfg_out_dir, 'cap_ppa_%s_cfg_decoder_reg.json' % \
        parser.d.name)
    ppa_cfg_file_mem = open(ppa_cfg_output_mem, 'w+')
    ppa_cfg_file_reg = open(ppa_cfg_output_reg, 'w+')

    parser.logger.info('%s:Parser Output Generation:' % parser.d.name)

    # create initial config register for start state
    idx = 0
    max_tcam_entries = parser.be.hw_model['parser']['num_states']

    tcam_t, tcam_dname = _create_template(ppa_json['cap_ppa']['registers'], ppa_decoder_json,
                            'cap_ppa_csr_dhs_bndl0_state_lkp_tcam_entry[0]')
    sram_t, sram_dname = _create_template(ppa_json['cap_ppa']['registers'], ppa_decoder_json,
                            'cap_ppa_csr_dhs_bndl0_state_lkp_sram_entry[0]')
    csum_t, csum_dname = _create_template(ppa_json['cap_ppa']['registers'], ppa_decoder_json,
                            'cap_ppa_csr_cfg_csum_profile[1]')
    csum_phdr_t, phdr_dname = _create_template(ppa_json['cap_ppa']['registers'], ppa_decoder_json,
                            'cap_ppa_csr_cfg_csum_phdr_profile[1]')
    icrc_t, icrc_dname = _create_template(ppa_json['cap_ppa']['registers'], ppa_decoder_json,
                            'cap_ppa_csr_cfg_crc_profile[1]')
    icrc_mask_t, icrc_mask_dname = _create_template(ppa_json['cap_ppa']['registers'], ppa_decoder_json,
                            'cap_ppa_csr_cfg_crc_mask_profile[1]')
    tcam0 = []
    sram0 = []

    parse_state_stack = []
    bi_processed_list = [] #(parse-state, [all-parse-states-along-the path])
    parse_state_stack.append((parser.states[0], [parser.states[0]]))
    while len(parse_state_stack):
        cs, parse_states_in_path = parse_state_stack.pop(0)
        ncc_assert(cs != None)
        for bi in cs.branches:
            if bi not in bi_processed_list:
                bi_processed_list.append(bi)

                # create a match entry for {state_id, lkp_flds, lkp_fld_mask}
                if not parser.be.args.post_extract and cs.is_end:
                    # Terminal state
                    parser.logger.debug('Skip transition from %s -> __END__,\
                                        Terminate' % (cs.name))
                    continue

                parse_state_stack.append((bi.nxt_state, parse_states_in_path + \
                                         [bi.nxt_state]))

                parser.logger.debug('%s:%s[%d]->%s[%d]' % \
                        (parser.d.name, cs.name, cs.id, bi.nxt_state.name, \
                         bi.nxt_state.id))
                te = _fill_parser_tcam_entry(tcam_t, parser, cs, bi)
                if cs.is_hw_start:
                    add_cs = cs
                else:
                    add_cs = None
                se = _fill_parser_sram_entry(parse_states_in_path + \
                                             [bi.nxt_state],\
                                             sram_t, parser, bi, add_cs)

                # Allow smaller json definition file and add entries
                te['entry_idx'] = str(idx)  # debug aid
                te['_modified'] = True
                se['entry_idx'] = str(idx)  # debug aid
                se['_modified'] = True
                if idx < len(tcam0):
                    tcam0[idx] = te
                    sram0[idx] = se
                else:
                    tcam0.append(te)
                    sram0.append(se)
                parser.logger.debug('TCAM-decoder[%d] - \n%s' % (idx,
                                                _parser_tcam_print(tcam0[idx])))
                parser.logger.debug('SRAM-decoder[%d] - \n%s' % \
                    (idx, _parser_sram_print(parser,sram0[idx])))

                for r,lf in enumerate(bi.nxt_state.active_lkp_lfs):
                    if lf != None:
                        parser.logger.debug('%s:lkp_reg[%d] = %s' % (parser.d.name, r, lf.hfname))
                idx += 1

                #Generate csum related profile config for parser block
                csum_prof_list = parser.be.checksum.\
                                ParserCsumProfileGenerate(parser, \
                                                          parse_states_in_path+\
                                                          [bi.nxt_state], bi.nxt_state,\
                                                          csum_t)
                csum_phdr_prof_list = parser.be.checksum.\
                           ParserCsumPhdrProfileGenerate(parser, \
                                                         parse_states_in_path +\
                                                         [bi.nxt_state], bi.nxt_state,\
                                                         csum_phdr_t)
                if len(csum_prof_list):
                    for csum_p in csum_prof_list:
                        csum_prof, cprof_inst = csum_p
                        #For handling wide register store word_size  *  profile#
                        csum_prof['word_size'] = str(hex(int(csum_t['word_size'], 16) * cprof_inst))
                        ppa_json['cap_ppa']['registers']\
                            ['cap_ppa_csr_cfg_csum_profile[%d]' % cprof_inst]\
                                = csum_prof
                        ppa_json['cap_ppa']['registers']\
                            ['cap_ppa_csr_cfg_csum_profile[%d]' % cprof_inst]['_modified']\
                                = True
                if len(csum_phdr_prof_list):
                    for csum_p in csum_phdr_prof_list:
                        csum_phdr_prof, phdr_inst = csum_p
                        #For handling wide register store word_size  *  profile#
                        csum_phdr_prof['word_size'] = str(hex(int(csum_phdr_t['word_size'], 16) * phdr_inst))
                        ppa_json['cap_ppa']['registers']\
                            ['cap_ppa_csr_cfg_csum_phdr_profile[%d]' % phdr_inst]\
                                                              = csum_phdr_prof
                        ppa_json['cap_ppa']['registers']\
                            ['cap_ppa_csr_cfg_csum_phdr_profile[%d]' % phdr_inst]['_modified']\
                                = True

                #Generate Gso csum related profile config for parser block
                csum_prof, cprof_inst = parser.be.checksum.\
                                GsoCsumParserProfileGenerate(parser, \
                                                          parse_states_in_path+\
                                                          [bi.nxt_state], bi.nxt_state,\
                                                          csum_t)
                if csum_prof != None:
                    csum_prof['word_size'] = str(hex(int(csum_t['word_size'], 16) * cprof_inst))
                    ppa_json['cap_ppa']['registers']\
                        ['cap_ppa_csr_cfg_csum_profile[%d]' % cprof_inst]\
                            = csum_prof

                #Generate icrc related profile config for parser block
                icrc_prof, prof_inst = parser.be.icrc.\
                                ParserIcrcProfileGenerate(parser, \
                                                          parse_states_in_path+\
                                                          [bi.nxt_state], bi.nxt_state,\
                                                          icrc_t)
                if icrc_prof != None:
                    #For handling wide register store word_size  *  profile#
                    icrc_prof['word_size'] = str(hex(int(icrc_t['word_size'], 16) * prof_inst))
                    ppa_json['cap_ppa']['registers']\
                      ['cap_ppa_csr_cfg_crc_profile[%d]' % prof_inst] = icrc_prof
                    ppa_json['cap_ppa']['registers']\
                      ['cap_ppa_csr_cfg_crc_profile[%d]' % prof_inst]['_modified'] = True

                icrc_mask_prof, prof_inst = parser.be.icrc.\
                                ParserIcrcMaskProfileGenerate(parser, \
                                                          parse_states_in_path+\
                                                          [bi.nxt_state], bi.nxt_state,\
                                                          icrc_mask_t)
                if icrc_mask_prof != None:
                    #For handling wide register store word_size  *  profile#
                    icrc_mask_prof['word_size'] = str(hex(int(icrc_mask_t['word_size'], 16) * prof_inst))
                    ppa_json['cap_ppa']['registers']\
                      ['cap_ppa_csr_cfg_crc_mask_profile[%d]' % prof_inst] = icrc_mask_prof
                    ppa_json['cap_ppa']['registers']\
                      ['cap_ppa_csr_cfg_crc_mask_profile[%d]' % prof_inst]['_modified'] = True


    # XXX add a catch-all end state
    te = _fill_parser_tcam_catch_all(tcam_t)
    se = _fill_parser_sram_catch_all(sram_t)
    if idx < len(tcam0):
        tcam0[idx] = te
        sram0[idx] = se
    else:
        tcam0.append(te)
        sram0.append(se)
    te['entry_idx'] = str(idx)  # debug aid
    se['entry_idx'] = str(idx)  # debug aid
    te['_modified'] = True
    se['_modified'] = True

    idx += 1
    parser.logger.info('%s:Tcam states used (including catch-all) %d' % (parser.d.name, idx))
    ncc_assert(idx <= parser.be.hw_model['parser']['num_states'], "Parser TCAM overflow")

    # program catch all entry register
    ppa_json['cap_ppa']['registers']['cap_ppa_csr_cfg_ctrl']['pe_enable']['value'] = str(0x3ff)
    ppa_json['cap_ppa']['registers']['cap_ppa_csr_cfg_ctrl']['parse_loop_cnt']['value'] = str(64)
    ppa_json['cap_ppa']['registers']['cap_ppa_csr_cfg_ctrl']['num_phv_flit']['value'] = str(6)
    ppa_json['cap_ppa']['registers']['cap_ppa_csr_cfg_ctrl']\
        ['state_lkp_catchall_entry']['value'] = str(idx)

    # gso csum will be written by a separate checksum instruction, enabling it here is ok
    # even if checksum inst is not executed
    ppa_json['cap_ppa']['registers']['cap_ppa_csr_cfg_ctrl']['gso_csum_en']['value'] = str(1)
    if parser.be.pa.gress_pa[parser.d].parser_end_off_cf:
        phv_flit_sz = parser.be.hw_model['phv']['flit_size']
        end_offset_flit = parser.be.pa.gress_pa[parser.d].parser_end_off_cf.phv_bit / phv_flit_sz
        ppa_json['cap_ppa']['registers']['cap_ppa_csr_cfg_ctrl']['end_offset_en']['value'] = str(1)
        ppa_json['cap_ppa']['registers']['cap_ppa_csr_cfg_ctrl']['end_offset_flit_num']['value'] = \
            str(end_offset_flit)

    ppa_json['cap_ppa']['registers']['cap_ppa_csr_cfg_ctrl']['_modified'] = True

    # program len_chk profiles
    for e,len_chk_profile in enumerate(parser.len_chk_profiles):
        if len_chk_profile == None:
            continue
        cap_ppa_csr_cfg_len_chk_profile = \
            ppa_json['cap_ppa']['registers']['cap_ppa_csr_cfg_len_chk_profile[%d]'%e]
        cap_ppa_csr_cfg_len_chk_profile['len_mask']['value'] = "0x%x" % int(0x3FFF)
        cap_ppa_csr_cfg_len_chk_profile['len_shift_left']['value'] = str(0)
        cap_ppa_csr_cfg_len_chk_profile['len_shift_val']['value'] = str(0)
        if len_chk_profile.offset_op == '+':
            cap_ppa_csr_cfg_len_chk_profile['addsub_start']['value'] = str(1)
        else:
            cap_ppa_csr_cfg_len_chk_profile['addsub_start']['value'] = str(0)

        cap_ppa_csr_cfg_len_chk_profile['start_adj']['value'] = "0x%x" % \
            int(len_chk_profile.start_offset)
        cap_ppa_csr_cfg_len_chk_profile['_modified'] = True

    ppa_json['cap_ppa']['memories']['cap_ppa_csr_dhs_bndl0_state_lkp_tcam']['entries'] = tcam0
    ppa_json['cap_ppa']['memories']['cap_ppa_csr_dhs_bndl1_state_lkp_tcam']['entries'] = tcam0
    if tcam_dname:
        ppa_json['cap_ppa']['memories']['cap_ppa_csr_dhs_bndl0_state_lkp_tcam']['decoder'] = \
            tcam_dname
        ppa_json['cap_ppa']['memories']['cap_ppa_csr_dhs_bndl1_state_lkp_tcam']['decoder'] = \
            tcam_dname
    ppa_json['cap_ppa']['memories']['cap_ppa_csr_dhs_bndl0_state_lkp_sram']['entries'] = sram0
    ppa_json['cap_ppa']['memories']['cap_ppa_csr_dhs_bndl1_state_lkp_sram']['entries'] = sram0
    if sram_dname:
        ppa_json['cap_ppa']['memories']['cap_ppa_csr_dhs_bndl0_state_lkp_sram']['decoder'] = \
            sram_dname
        ppa_json['cap_ppa']['memories']['cap_ppa_csr_dhs_bndl1_state_lkp_sram']['decoder'] = \
            sram_dname

    json.dump(ppa_json['cap_ppa']['memories'],
                ppa_cfg_file_mem, indent=4, sort_keys=False, separators=(',', ': '))

    # XXX program all init profiles to use the same info
    # change this when we optimize for separate start state
    num_profiles = parser.be.hw_model['parser']['num_init_profiles']
    for i in range(num_profiles):
        init_profile = ppa_json['cap_ppa']['registers']['cap_ppa_csr_cfg_init_profile[%d]'%i]
        init_profile['curr_offset']['value'] = str(0)
        init_profile['state']['value'] = str(parser.start_state.id)
        init_profile['_modified'] = True
        for r, reg in enumerate(parser.start_state.lkp_regs):
            if reg.pkt_off >= 0:
                #init_profile['mux_sel%d' % r]['value'] =  str(r)
                #init_profile['mux_idx%d' % r]['value'] =  str(reg.pkt_off / 8)
                init_profile['lkp_val_pkt_idx%d' % r]['value'] =  str(reg.pkt_off / 8)
            else:
                #init_profile['mux_sel%d' % r]['value'] =  str(r)
                #init_profile['mux_sel%d' % r]['value'] =  str(0)
                init_profile['lkp_val_pkt_idx%d' % r]['value'] =  str(0)
    parser.logger.debug("%s" % _parser_init_profile_print(parser, init_profile))

    #Enable pre parser on all uplinks and p4-ingress.
    pre_parser = ppa_json['cap_ppa']['registers']['cap_ppa_csr_cfg_preparse']
    pre_parser['tm_iport_enc_en']['value'] = str(0xff) # First 8 bits map to 8 uplink ports.
    pre_parser['bypass']['value'] = str(0)
    pre_parser['udp_dstport_roce_val0']['value'] = str(4791)
    pre_parser['udp_dstport_vxlan_val0']['value'] = str(4789)
    pre_parser['udp_dstport_vxlan_val1']['value'] = str(4790)
    pre_parser['vxlan_flag_mask']['value'] = str(0xfb) # vxlan-flags = |R|R|Ver|I|P|B|O|
    pre_parser['vxlan_flag_val']['value'] = str(0x8)
    pre_parser['_modified'] = True

    json.dump(ppa_json['cap_ppa']['registers'],
                ppa_cfg_file_reg, indent=4, sort_keys=True, separators=(',', ': '))
    ppa_cfg_file_mem.close()
    ppa_cfg_file_reg.close()
    if not parser.be.args.p4_plus:
        cap_inst = 1 if (parser.d == xgress.INGRESS) else 0
        capri_dump_registers(parser.be.args.cfg_dir, parser.be.prog_name,
                             'cap_ppa', cap_inst,
                             ppa_json['cap_ppa']['registers'],
                             ppa_json['cap_ppa']['memories'],)

def _fill_te_tcam_catch_all(tcam_t):
    # create an entry that will always be a 'hit'
    tcam_t['valid']['value'] = str(1)
    tcam_t['value']['value'] = str(0)
    tcam_t['mask']['value'] = str(0)
    tcam_t['_modified'] = True

def _phv_bit_flit_le_bit(phv_bit):
    # ncc numbers everthing from left to right, msb->lsb
    # hw is weired endian where bits are numbered in reverse direction for each flit
    # convert phv_bit nuber 0-->N where 0 is msbit of flit0... to
    # 511..0 - 1023..512
    # 0->511, 3->508, 511->0, 512->1023, 514->1021, 1023->512...
    flit = phv_bit / 512
    flit_base = flit * 512
    phv_off = phv_bit % 512
    return flit_base + (511 - phv_off)

def capri_te_cfg_output(stage):
    # read the register spec json
    cur_path = os.path.abspath(__file__)
    cur_path = os.path.split(cur_path)[0]
    gen_dir = stage.gtm.tm.be.args.gen_dir
    te_file_path = os.path.join(cur_path, 'csr_json/cap_te_csr.json')
    te_file = open(te_file_path)
    te_json = json.load(te_file)
    '''
    # no decoder for TE
    te_decoder_file_path = os.path.join(cur_path, 'csr_json/cap_te_decoders.json')
    te_decoder_file = open(te_decoder_file_path)
    te_decoder_json = json.load(te_decoder_file)
    '''
    cfg_out_dir = os.path.join(gen_dir + '/%s/cfg_out' % stage.gtm.tm.be.prog_name)
    if not os.path.exists(cfg_out_dir):
        try:
            os.makedirs(cfg_out_dir)
        except OSError as e:
            if e.errno != errno.EEXIST:
                raise
    te_cfg_output_mem = os.path.join(cfg_out_dir, 'cap_te_%s_%d_cfg_mem.json' % \
        (stage.gtm.d.name, stage.id))
    te_cfg_output_reg = os.path.join(cfg_out_dir, 'cap_te_%s_%d_cfg_reg.json' % \
        (stage.gtm.d.name, stage.id))
    te_cfg_file_mem = open(te_cfg_output_mem, 'w+')
    te_cfg_file_reg = open(te_cfg_output_reg, 'w+')

    stage.gtm.tm.logger.info('%s:====== Table Engine Output Generation: stage %d ======' % \
        (stage.gtm.d.name, stage.id))
    # program km_profiles
    max_km_width = stage.gtm.tm.be.hw_model['match_action']['key_maker_width']
    max_km_wB = max_km_width/8
    max_km_profiles = stage.gtm.tm.be.hw_model['match_action']['num_km_profiles']
    max_km_bits = stage.gtm.tm.be.hw_model['match_action']['num_bit_extractors']

    num_flits = stage.gtm.tm.be.hw_model['phv']['num_flits']
    max_cycles = stage.gtm.tm.be.hw_model['match_action']['max_te_cycles']
    te_consts = stage.gtm.tm.be.hw_model['match_action']['te_consts']
    no_load_byte = te_consts['no_load_byte']
    no_load_bit = te_consts['no_load_bit']

    json_regs = te_json['cap_te']['registers']
    json_mems = te_json['cap_te']['memories']

    for prof in stage.hw_km_profiles:
        if prof == None:
            continue
        hw_id = prof.hw_id
        ncc_assert(hw_id < max_km_profiles)
        sel_id = (hw_id * max_km_wB)
        bit_sel_id = (hw_id * max_km_bits)
        bit_loc_id = (hw_id * (max_km_bits/8))

        byte_sel_val = []
        stage.gtm.tm.logger.debug("%s:%d:Program km_profile[%d]:" % \
            (stage.gtm.d.name, stage.id, hw_id))

        for b in prof.byte_sel:
            json_km_profile = json_regs['cap_te_csr_cfg_km_profile_byte_sel[%d]' % sel_id]
            if b < 0:
                json_km_profile['byte_sel']['value'] = no_load_byte
            else:
                json_km_profile['byte_sel']['value'] = str(b)
            json_km_profile['_modified'] = True
            sel_id += 1
        # load rest of the bytes to not load the km
        for b in range(sel_id, ((hw_id+1) * max_km_wB)):
            if sel_id >= 256: pdb.set_trace()
            json_km_profile = json_regs['cap_te_csr_cfg_km_profile_byte_sel[%d]' % sel_id]
            json_km_profile['byte_sel']['value'] = no_load_byte
            json_km_profile['_modified'] = True
            sel_id += 1

        byte_sel_val = []
        sel_id = (hw_id * max_km_wB)
        for b in range(sel_id, sel_id+max_km_wB):
            byte_sel_val.append( \
                json_regs['cap_te_csr_cfg_km_profile_byte_sel[%d]' % b]['byte_sel']['value'])

        # Change the bit order and phv_bit numbering as hw wants it
        # For ASIC ms_bit must be programmed in bit_sel[15] and lsb in bit_sel[0]
        bit_sel_id += (max_km_bits - 1)
        bits_left = max_km_bits
        for b in prof.bit_sel:
            json_km_profile = json_regs['cap_te_csr_cfg_km_profile_bit_sel[%d]' % bit_sel_id]
            if b < 0:
                json_km_profile['bit_sel']['value'] = no_load_bit
            else:
                json_km_profile['bit_sel']['value'] = "0x%x" % int(_phv_bit_flit_le_bit(b))
            json_km_profile['_modified'] = True
            bit_sel_id -= 1
            bits_left -= 1

        for b in range(bits_left):
            json_km_profile = json_regs['cap_te_csr_cfg_km_profile_bit_sel[%d]' % bit_sel_id]
            json_km_profile['bit_sel']['value'] = no_load_bit
            json_km_profile['_modified'] = True
            bit_sel_id -= 1

        # bit_loc1 is bit0-7, bit_loc0 is bit8-15
        # since bit_sels are written from 15 down to 0, no need to switch bit_loc here
        # XXX check for <= 8 can be removed or turned into ncc_assert(, just look at bit_locX >= 0)
        if len(prof.bit_sel) <= 8:
            json_km_profile = json_regs['cap_te_csr_cfg_km_profile_bit_loc[%d]' % (bit_loc_id)]
            if prof.bit_loc >= 0:
                json_km_profile['valid']['value'] = str(1)
                json_km_profile['bit_loc']['value'] = str(prof.bit_loc)
                json_km_profile['_modified'] = True
            else:
                # do we need to set _modified flag here?
                json_km_profile['valid']['value'] = str(0)
            json_km_profile = json_regs['cap_te_csr_cfg_km_profile_bit_loc[%d]' % (bit_loc_id+1)]
            json_km_profile['valid']['value'] = str(0)
        else:
            json_km_profile = json_regs['cap_te_csr_cfg_km_profile_bit_loc[%d]' % (bit_loc_id)]
            if prof.bit_loc >= 0:
                json_km_profile['valid']['value'] = str(1)
                json_km_profile['bit_loc']['value'] = str(prof.bit_loc)
                json_km_profile['_modified'] = True
            else:
                json_km_profile['valid']['value'] = str(0)
            json_km_profile = json_regs['cap_te_csr_cfg_km_profile_bit_loc[%d]' % (bit_loc_id+1)]
            if prof.bit_loc1 >= 0:
                json_km_profile['valid']['value'] = str(1)
                json_km_profile['bit_loc']['value'] = str(prof.bit_loc1)
                json_km_profile['_modified'] = True
            else:
                json_km_profile['valid']['value'] = str(0)

        # debug printing
        bit_sel_val = []
        bit_sel_id = (hw_id * max_km_bits)
        for b in range(bit_sel_id, bit_sel_id+max_km_bits):
            bit_sel_val.append( \
                json_regs['cap_te_csr_cfg_km_profile_bit_sel[%d]' % b]['bit_sel']['value'])
        stage.gtm.tm.logger.debug( \
            "byte_sel[%d] %s\nbit_sel[%d] %s\nbit_locs [%d]%s, [%d]%s" % \
            (sel_id, byte_sel_val, bit_sel_id, bit_sel_val, bit_loc_id,
            json_regs['cap_te_csr_cfg_km_profile_bit_loc[%d]' % bit_loc_id]['bit_loc']['value'] \
                if prof.bit_loc >= 0 else "-",
            (bit_loc_id+1),
            json_regs['cap_te_csr_cfg_km_profile_bit_loc[%d]'%(bit_loc_id+1)]['bit_loc']['value'] \
                if prof.bit_loc1 >= 0 else "-",
            ))

    # got thru' predicate combinations and program km
    # program predication tcam/srams for each combination
    max_key_sel = stage.gtm.tm.be.hw_model['match_action']['num_predicate_bits']
    max_tbl_profiles = stage.gtm.tm.be.hw_model['match_action']['num_table_profiles']
    max_hw_flits = stage.gtm.tm.be.hw_model['phv']['max_hw_flits']
    num_km = stage.gtm.tm.be.hw_model['match_action']['num_key_makers']
    json_tbl_prof_key = json_regs['cap_te_csr_cfg_table_profile_key']

    # XXX TCAM programming can be handled in a loop for all predicate values
    # need to set unused entries to map to DO-NOTHING
    run_all_tables = False
    if len(stage.active_predicates) == 0:
        # No predicates - execute all tables
        for k in range(max_key_sel):
            json_tbl_prof_key['sel%d' % k]['value'] = str(0)
        # setup tcam entry#0 as catch-all and prgram all tables in sram..
        idx = 0
        tcam_t = json_regs['cap_te_csr_cfg_table_profile_cam[%d]' % idx]
        _fill_te_tcam_catch_all(tcam_t)
        # profile_id 0 is used to populate all the tables by the compiler
        run_all_tables = True
    else:
        w, cf_key_sel = stage.stg_get_tbl_profile_key()
        k = 0
        # program the key selector
        key_sel_bits = []
        for cf in cf_key_sel:
            phv_bit = cf.phv_bit
            stage.gtm.tm.logger.debug("%s:Stage %d:Table profile key field %s phv %d, w %d" % \
                (stage.gtm.d.name, stage.id, cf.hfname, cf.phv_bit, cf.width))
            for b in range(cf.width):
                key_sel_bits.insert(0, phv_bit)
                phv_bit += 1
                k += 1

        # ASIC uses flittle endian when using bit locations.
        # Keep the active profile bits to lsb since val and mask is computed that way
        k = -1
        for k in range(len(key_sel_bits)):
            json_tbl_prof_key['sel%d' % k]['value'] = "0x%x" % \
                int(_phv_bit_flit_le_bit(key_sel_bits[k]))
            json_tbl_prof_key['_modified'] = True

        for k in range(k+1, max_key_sel):
            json_tbl_prof_key['sel%d' % k]['value'] = str(0)
            #json_tbl_prof_key['_modified'] = True

    prof_idx = 0
    # don't sort the prof_vals, already done while creating profiles
    # sorting must be done based on masks
    tcam_entries = OrderedDict()
    sidx = 0
    for prof_val,ctg in stage.table_profiles.items():
        if not run_all_tables:
            # program tcam entries for predicate values
            tcam_vms = stage.stg_create_tbl_profile_tcam_val_mask(prof_val)
            # XXX for now max one tcam entry per prof_val is supported
            if len(tcam_vms) == 0:
                stage.gtm.tm.logger.warning( \
                        "%s:Stage %d: Table profile TCAM: profile_val %d Skip Invalid condition" % \
                        (stage.gtm.d.name, stage.id, prof_val))
                continue
            ncc_assert(len(tcam_vms) == 1)
            (val, mask) = tcam_vms[0]
            if (val, mask) in tcam_entries:
                stage.gtm.tm.logger.warning( \
                        "%s:Stage %d:Table profile TCAM: Skip entry (0x%x, 0x%x) : %d, %s" % \
                        (stage.gtm.d.name, stage.id, val, mask, prof_val, ctg))
                continue

            tcam_entries[(val, mask)] = ctg
            te = json_regs['cap_te_csr_cfg_table_profile_cam[%d]' % prof_idx]
            te['valid']['value'] = str(1)
            te['value']['value'] = "0x%x" % val
            te['mask']['value'] = "0x%x" % mask
            te['_modified'] = True

        te = json_regs['cap_te_csr_cfg_table_profile_cam[%d]' % prof_idx] # for printing
        # do use overflow tcam when programming
        active_ctg = [act for act in stage.table_profiles[prof_val] if not act.is_otcam]

        json_tbl_prof = json_regs['cap_te_csr_cfg_table_profile[%d]' % prof_idx]
        # compute mpu res based on threads
        mpu_res = sum(x.num_threads for x in active_ctg if not x.is_memory_only)
        json_tbl_prof['mpu_results']['value'] = str(mpu_res)
        json_tbl_prof['_modified'] = True

        stage.gtm.tm.logger.debug( \
            "%s:Stage %d:Table profile TCAM[%d]:(val %s, mask %s): prof_val %d, %s, mpu_res %d" % \
                (stage.gtm.d.name, stage.id, prof_idx, te['value']['value'],
                te['mask']['value'], prof_val, active_ctg, mpu_res))

        # h/w allows a flexible partitioning of the total (192) ctrl_sram entries per profile
        json_tbl_prof['seq_base']['value'] = str(sidx)
        ncc_assert(sidx < max_cycles)

        stage.gtm.tm.logger.debug( \
            "%s:Stage %d:Profile %d SRAM start_idx %d" % \
            (stage.gtm.d.name, stage.id, prof_idx, sidx))

        flit_kms = [[] for _ in range(num_flits)]
        for ct in active_ctg:
            ncc_assert(not ct.is_otcam)
            for _km in ct.key_makers:
                if _km.shared_km:
                    km = _km.shared_km
                else:
                    km = _km
                for fid in km.flits_used:
                    if km not in flit_kms[fid]:
                        flit_kms[fid].append(km)

        fid = 0
        prev_fid = -1
        cyc_done = False
        prev_km_prof = [(-1,-1) for _ in range(num_km)] # preserve the profile_id on unused flits
        for cyc in range(len(stage.table_sequencer[prof_val])):
            cyc_km_used = {}
            # fill control_sram entry
            se = json_regs['cap_te_csr_dhs_table_profile_ctrl_sram_entry[%d]' % sidx]

            # For good measures, hw model checks that key-makers are cleared on very first
            # cycle of the sequencer
            if cyc == 0:
                for kmid in range(num_km):
                    se['km_new_key%d' % kmid]['value'] = str(1)

            # init km_prof with vales from previous cyc, these will be over-written
            # if kms are used/changed, if not just retain the old values
            # table's flits_used are set contiguously from first flits used until the last flit
            # but key makers' flits_used is specific to flits (done for P4+ and sharing)
            for kmid, (prof_id,prof_mode) in enumerate(prev_km_prof):
                if prof_id < 0:
                    continue
                se['km_profile%d' % kmid]['value'] = str(prof_id)
                se['km_mode%d' % kmid]['value'] = str(prof_mode)

            if not stage.table_sequencer[prof_val][cyc].is_used:
                # no need to do much, key-maker values can be modified only if they are not
                # used anymore
                pass
            else:
                ncc_assert(fid <= num_flits    )# error in advancing flits
                for km in flit_kms[fid]:
                    km_prof = km.combined_profile
                    kmid = km.hw_id

                    if kmid in cyc_km_used:
                        ncc_assert(cyc_km_used[kmid] == km_prof.hw_id)
                    else:
                        cyc_km_used[kmid] = km_prof.hw_id

                    if not km_prof:
                        continue # key-less tables
                    se['km_mode%d' % kmid]['value'] = str(km_prof.mode)
                    se['km_profile%d' % kmid]['value'] = str(km_prof.hw_id)
                    # save the values for init of next cycle
                    prev_km_prof[kmid] = (km_prof.hw_id, km_prof.mode)
                    if fid == km.flits_used[0] and fid != prev_fid:
                        # on first flit used by this km, set new_key = 1
                        # asic loads km only on accepting new flit, if flit is not
                        # advanced, km can get reset if new_key is set
                        se['km_new_key%d' % kmid]['value'] = str(1)
                    else:
                        se['km_new_key%d' % kmid]['value'] = str(0)

            ct = stage.table_sequencer[prof_val][cyc].tbl
            if ct:
                ncc_assert(se['tableid']['value'] == '-1')
                if stage.table_sequencer[prof_val][cyc].thread == 0:
                    se['tableid']['value'] = str(ct.tbl_id)
                else:
                    se['tableid']['value'] = str(\
                        ct.thread_tbl_ids[stage.table_sequencer[prof_val][cyc].thread])
                se['hash_sel']['value'] = str(ct.hash_type)
                if ct.match_type == match_type.EXACT_IDX:
                    se['lkup']['value'] = te_consts['direct']
                elif ct.match_type == match_type.TERNARY:
                    se['lkup']['value'] = te_consts['tcam_sram']    # XXX new type for tcam_only
                elif ct.match_type == match_type.EXACT_HASH:
                    se['lkup']['value'] = te_consts['hash_only']
                elif ct.match_type == match_type.EXACT_HASH_OTCAM:
                    se['lkup']['value'] = te_consts['hash_otcam_sram']
                elif ct.match_type == match_type.MPU_ONLY:
                    se['lkup']['value'] = te_consts['mpu_only']
                else:
                    ncc_assert(0)
                if ct.is_wide_key:
                    if fid == ct.flits_used[0]:
                        se['hash_chain']['value'] = str(0)
                        se['hash_store']['value'] = str(1)
                    elif fid == ct.flits_used[-1]:
                        se['hash_chain']['value'] = str(1)
                        se['hash_store']['value'] = str(0)
                    else:
                        se['hash_chain']['value'] = str(1)
                        se['hash_store']['value'] = str(1)

                if ct.num_entries == 0 and ct.is_hash_table():
                    stage.gtm.tm.logger.warning(\
                        '%s: Stage %d:sram[%d]:Table %s change lkp_type %s to HASH-ONLY (no mem access)' % \
                            (stage.gtm.d.name, stage.id, sidx, ct.p4_table.name, se['lkup']['value']))
                    #se['lkup']['value'] = te_consts['mpu_only']
                    se['lkup']['value'] = te_consts['hash_only']
            else:
                se['lkup']['value'] = te_consts['no_op']
                se['hash_chain']['value'] = str(0)
                se['hash_store']['value'] = str(0)

            # fill sram_extention
            json_sram_ext = json_regs['cap_te_csr_cfg_table_profile_ctrl_sram_ext[%d]' % sidx]
            if stage.table_sequencer[prof_val][cyc].adv_flit:
                json_sram_ext['adv_phv_flit']['value'] = str(1)
                prev_fid = fid
                fid += 1
            else:
                json_sram_ext['adv_phv_flit']['value'] = str(0)
                prev_fid = fid

            if stage.table_sequencer[prof_val][cyc].is_last:
                json_sram_ext['done']['value'] = str(1)
            else:
                json_sram_ext['done']['value'] = str(0)

            if not cyc_done:
                stage.gtm.tm.logger.debug(\
                    "%s:Stage[%d]:cap_te_csr_dhs_table_profile_ctrl_sram_entry[%d]:\n%s" % \
                    (stage.gtm.d.name, stage.id, sidx,
                    te_ctrl_sram_print(se, json_sram_ext)))
            if stage.table_sequencer[prof_val][cyc].is_last:
                cyc_done = True

            json_sram_ext['_modified'] = True
            se['_modified'] = True
            sidx += 1

        prof_idx += 1

    if len(stage.active_predicates) > 0 or prof_idx == 0:
        # prof_idx == 0 indicates that no tables to be run in this stage, use catch-all
        if prof_idx == 0:
            stage.gtm.tm.logger.debug(\
                "%s:Stage[%d]:Empty Stage. Use catch-all to skip it" % \
                (stage.gtm.d.name, stage.id))
        if prof_idx < stage.gtm.tm.be.hw_model['match_action']['num_table_profiles']:
            # create a catch-all entry to execute NO tables
            # ASIC seems to use tcam entry 0 on miss (Initally plan was to skip table lookups on miss)
            # If all entries are already programmed.. we can skip this... but need to make sure if there
            # can be a tcam miss - XXX
            te = json_regs['cap_te_csr_cfg_table_profile_cam[%d]' % prof_idx]
            _fill_te_tcam_catch_all(te)

            json_tbl_prof = json_regs['cap_te_csr_cfg_table_profile[%d]' % prof_idx]
            json_tbl_prof['mpu_results']['value'] = str(0)
            json_tbl_prof['seq_base']['value'] = str(sidx)
            json_tbl_prof['_modified'] = True

            json_sram_ext = json_regs['cap_te_csr_cfg_table_profile_ctrl_sram_ext[%d]' % sidx]
            json_sram_ext['adv_phv_flit']['value'] = str(1)
            json_sram_ext['done']['value'] = str(1)
            json_sram_ext['_modified'] = True

            # setup sram entry to launch no lookup
            se = json_regs['cap_te_csr_dhs_table_profile_ctrl_sram_entry[%d]' % sidx]
            se['lkup']['value'] = te_consts['no_op']
            # init a few other values to keep RTL sim happy
            se['tableid']['value'] = str(0)
            se['hash_sel']['value'] = str(0)
            se['hash_chain']['value'] = str(0)
            se['hash_store']['value'] = str(0)
            # For good measures, hw model checks that key-makers are cleared on very first
            # cycle of the sequencer
            for kmid in range(num_km):
                se['km_new_key%d' % kmid]['value'] = str(1)
            se['_modified'] = True

            stage.gtm.tm.logger.debug( \
                "%s:Stage %d:Table profile (catch-all)TCAM[%d]:(val %s, mask %s): prof_val %d, %s, mpu_res %d" % \
                    (stage.gtm.d.name, stage.id, prof_idx, te['value']['value'],
                    te['mask']['value'], 0, [], 0))
            stage.gtm.tm.logger.debug(\
                        "%s:Stage[%d]:cap_te_csr_dhs_table_profile_ctrl_sram_entry[%d]:\n%s" % \
                        (stage.gtm.d.name, stage.id, sidx,
                        te_ctrl_sram_print(se, json_sram_ext)))
        else:
            stage.gtm.tm.logger.warning( \
                "%s:Stage[%d]:No space to create catch-all table profile" % \
                (stage.gtm.d.name, stage.id))

    for ct in stage.ct_list:
        if ct.is_otcam:
            continue
        for thd in range(ct.num_threads):
            if thd == 0:
                tbl_id = ct.tbl_id
                json_tbl_ = json_regs['cap_te_csr_cfg_table_property[%d]' % tbl_id]
            else:
                tbl_id = ct.thread_tbl_ids[thd]
                json_tbl_ = json_regs['cap_te_csr_cfg_table_property[%d]' % tbl_id]

            # update to asic doc - axi=1 => SRAM
            if ct.is_hbm:
                json_tbl_['axi']['value'] = str(0)
                #json_tbl_['hbm']['value'] = str(1)
            else:
                # XXX there can be tables in host mem (not hbm) for which axi should set to 0
                # currently there are no host tables supported (need a pragma)
                json_tbl_['axi']['value'] = str(1)
                #json_tbl_['hbm']['value'] = str(0)

            if ct.is_hash_table() and ct.d_size < ct.start_key_off:
                # Program APC location. APC location is always first byte
                # in the hash table entry. However when axi-shift is programmed,
                # APC byte location moves right by axi-shift number of bytes.
                json_tbl_['mpu_pc_loc']['value'] = (((ct.start_key_off - ct.d_size) >> 4) << 1)

            # key mask programming -
            # hw bit numbering is 511:0 - little endian like
            # which is opposite on ncc ordering
            # ncc creates end_key_off such that it points to a bit after the key
            # if only one km is used, it is at msb (??) so still adjust it using 512 bit
            # hardware expects hi, lo mask as -
            # (hi, lo] => hi=512 means we need bit 511, lo=496 means we need bit 496
            # XXX hw does not have enough bits to store 512 - will be fixed soon
            if ct.is_wide_key:
                key_mask_hi = 512 - ct.last_flit_start_key_off
                key_mask_lo = 512 - ct.last_flit_end_key_off
            elif not ct.is_mpu_only():
                ncc_assert(ct.start_key_off >= 0)
                key_mask_hi = 512 - ct.start_key_off
                key_mask_lo = 512 - ct.end_key_off
            else:
                key_mask_hi = 0
                key_mask_lo = 0

            # for index table the key_mask_lo must be specified in terms of 16bit chunks
            if ct.is_index_table():
                # do it for raw table also.. should not matter
                key_mask_lo = key_mask_lo / 16  # model param for 16??

            json_tbl_['key_mask_hi']['value'] = str(key_mask_hi)
            json_tbl_['key_mask_lo']['value'] = str(key_mask_lo)

            # XXX set mpu_pc_loc to 0
            json_tbl_['mpu_pc_loc']['value'] = str(0)

            # key size (applicable to hash and index tables), num bits to use as index
            if ct.is_index_table():
                #lg2_size = log2size(ct.num_entries)
                # does not work correctly for raw tables
                json_tbl_['addr_sz']['value'] = str(ct.final_key_size)
            elif ct.is_hash_table():
                if ct.is_overflow:
                    lg2_size = log2size(ct.num_entries)
                    json_tbl_['addr_sz']['value'] = str(lg2_size)
                else:
                    lg2_size = log2size(ct.num_entries)
                    json_tbl_['addr_sz']['value'] = str(lg2_size)

            # For asic  km0=>lo, km1=>hi (from te.cc)
            # ncc uses km0 as high key and km1 as lo key bytes, so flip it
            k = 1
            for _km in ct.key_makers:
                if k < 0:
                    break;  # handles wide-key tables
                if _km.shared_km:
                    km = _km.shared_km
                else:
                    km = _km

                json_tbl_['fullkey_km_sel%d' % k]['value'] = str(km.hw_id)
                k -= 1

            if ct.is_writeback:
                if ct.is_raw:
                    json_tbl_['lock_en_raw']['value'] = str(1)
                else:
                    if ct.match_type == match_type.TERNARY:
                        json_tbl_['lock_en']['value'] = str(0)
                    else:
                        json_tbl_['lock_en']['value'] = str(1)
            else:
                json_tbl_['lock_en']['value'] = str(0)

            if ct.num_actions() > 1 and not ct.is_raw:
                json_tbl_['mpu_pc_dyn']['value'] = str(1)
            else:
                json_tbl_['mpu_pc_dyn']['value'] = str(0)
            # set a fixed value for model testing XXX
            json_tbl_['mpu_pc']['value'] = '0x2FEED00'

            if ct.is_raw:
                json_tbl_['mpu_pc_raw']['value'] = str(1)
            else:
                json_tbl_['mpu_pc_raw']['value'] = str(0)

            if ct.is_raw or ct.is_raw_index:
                json_tbl_['max_bypass_cnt']['value'] = str(16)

            json_tbl_['mpu_pc_ofst_err']['value'] = str(0)
            json_tbl_['mpu_vec']['value'] = '0xF'    # all mpus for scheduling
            json_tbl_['addr_base']['value'] = str(0)
            json_tbl_['addr_vf_id_en']['value'] = str(0)
            json_tbl_['addr_vf_id_loc']['value'] = str(0)

            if ct.num_entries and ct.otcam_ct:
                #This value is used by Otcam table to jump to sram area
                #associated with TCAM but present at end of hash-table's
                #sram-area.
                json_tbl_['oflow_base_idx']['value'] = str(ct.num_entries)

            entry_size = ct.d_size
            if ct.is_hash_table():
                if ct.is_overflow:
                    entry_size += ct.hash_ct.key_phv_size
                else:
                    entry_size += ct.key_phv_size

            entry_sizeB = (entry_size + 7) / 8   # convert to bytes
            lg2entry_size = log2size(entry_sizeB)
            json_tbl_['tbl_entry_sz_raw']['value'] = str(0)
            json_tbl_['addr_shift']['value'] = str(0)
            json_tbl_['lg2_entry_size']['value'] = str(0)

            if ct.is_hbm and not ct.is_raw and not ct.is_raw_index:
                json_tbl_['addr_shift']['value'] = str(lg2entry_size)
                json_tbl_['lg2_entry_size']['value'] = str(lg2entry_size)
            elif ct.is_raw_index:
                # special handling, don't shift addr, but read entry_size bytes
                json_tbl_['addr_shift']['value'] = str(0)
                json_tbl_['lg2_entry_size']['value'] = str(lg2entry_size)
            elif ct.is_raw:
                # XXX need parameter to pragma if entry size is fixed - leave it to run-time for now
                json_tbl_['tbl_entry_sz_raw']['value'] = str(1)
                json_tbl_['addr_shift']['value'] = str(0)
                json_tbl_['lg2_entry_size']['value'] = str(0)
            else:
                json_tbl_['addr_shift']['value'] = str(0)
                json_tbl_['lg2_entry_size']['value'] = str(0)

            # need to program chain shift for wide-key table - for toeplitz leave it as 0??
            if ct.is_wide_key and not ct.is_toeplitz_hash():
                json_tbl_['chain_shift']['value'] = str(6)
                json_tbl_['lg2_entry_size']['value'] = str(6)
            else:
                json_tbl_['chain_shift']['value'] = str(0)

            # special case - for hash table w/ no mem access, overwrite log2entry and axi values
            if ct.num_entries == 0 and ct.is_hash_table():
                # this also covers toeplitz hash
                # special values (7) is used by h/w to not launch mem read operation
                json_tbl_['lg2_entry_size']['value'] = str(7)
                json_tbl_['axi']['value'] = str(0) # make it hbm for lg2_Entry_sz to take effect

            if ct.is_memory_only:
                json_tbl_['memory_only']['value'] = '0x1'
            # else leave default value

            json_tbl_['_modified'] = True
            stage.gtm.tm.logger.debug("%s:Stage[%d]:Table %s:cap_te_csr_cfg_table_property[%d]:\n%s" % \
                (stage.gtm.d.name, stage.id, ct.p4_table.name, tbl_id,
                te_tbl_property_print(json_tbl_)))

    #pdb.set_trace()
    json.dump(te_json['cap_te']['registers'],
                te_cfg_file_reg, indent=4, sort_keys=True, separators=(',', ': '))
    te_cfg_file_reg.close()
    te_cfg_file_mem.close()

def _decode_mem(entry, result):
    if isinstance(entry, list):
        for field in entry:
            _decode_mem(field, result)
    elif isinstance(entry, dict):
        if 'value' in entry and 'size' in entry:
            result[0] |= int(entry['value'], 0) << result[1]
            result[1] += int(entry['size'], 0)
        else:
            for field, attrib in entry.iteritems():
                _decode_mem(attrib, result)

def _decode_reg(entry, result):
    if isinstance(entry, list):
        for field in entry:
            _decode_reg(field, result)
    elif isinstance(entry, dict):
        if 'value' in entry and 'size' in entry:
            result[0] |= int(entry['value'], 0) << result[1]
            result[1] += int(entry['size'], 0)
        else:
            for field, attrib in entry.iteritems():
                _decode_reg(attrib, result)

def capri_dump_registers(cfg_out_dir, prog_name, cap_mod, cap_inst, regs, mems):
    if cfg_out_dir is None or cap_mod is None or prog_name is None:
        return
    if not os.path.exists(cfg_out_dir):
        try:
            os.makedirs(cfg_out_dir)
        except OSError as e:
            if e.errno != errno.EEXIST:
                raise
    cfg_out_fname = os.path.join(cfg_out_dir, '%s_%s_%d.bin' % (prog_name, cap_mod, cap_inst))
    cfg_out_fp = open(cfg_out_fname, 'wb')

    # fetch base address
    cur_path = os.path.abspath(__file__)
    cur_path = os.path.split(cur_path)[0]
    base_addr_file = os.path.join(cur_path, 'csr_json/cap_addr.json')
    base_addr_fp = open(base_addr_file)
    base_addr_json = json.load(base_addr_fp)
    addr_map_size = eval(base_addr_json[cap_mod]['amap_size'])
    base_addr = int(base_addr_json[cap_mod]['base_addr'], 16)
    base_addr += addr_map_size * cap_inst

    for name, conf in regs.iteritems():
        addr_offset = int(conf['addr_offset'], 16) + base_addr
        word_size = int(conf['word_size'], 16)
        is_array = int(conf['is_array'])
        if is_array == 1:
            addr_offset += word_size * 4
        data = 0
        width = int(conf['_width'], 0)
        skip = True

        if 'decoder' in conf:
            skip = False
            m = re.search('(\w+)_entry\[(\d+)\]', name)
            if m == None:
                m = re.search('(\w+)_profile\[(\d+)\]', name)
            if m == None:
                continue
            decoder = m.group(1)
            idx = int(m.group(2))
            if decoder in mems:
                if idx > (len(mems[decoder]['entries']) - 1):
                    # entry does not exist in the decode, skip it
                    continue
                else:
                    result = [data, 0]
                    _decode_mem(mems[decoder]['entries'][int(idx)], result)
                    data = result[0]

            elif m.group(0) in regs:
                if decoder not in regs[m.group(0)]:
                    continue
                else:
                    result = [data, 0]
                    _decode_reg(regs[m.group(0)][decoder]['entries'], result)
                    data = result[0]
        else:
            if '_modified' not in conf.keys():
                # skip the register if its contents are not modified
                continue
            for field, attrib in conf.iteritems():
                if not type(attrib) is dict:
                    continue
                lsb  = int(attrib['field_lsb'])
                msb  = int(attrib['field_msb'])
                mask = int(attrib['field_mask'], 16)
                val  = int(attrib['value'], 0)
                if val == -1:
                    val = 0
                else:
                    skip = False
                data = data | (val << lsb)
        if skip == True:
            continue

        while width > 0:
#            print '0x%08x 0x%08x %s' % \
#                (addr_offset, data & 0xFFFFFFFF, name)
            cfg_out_fp.write(struct.pack('II', addr_offset, data & 0xFFFFFFFF))
            data = data >> 32
            width -= 32
            addr_offset += 4
    cfg_out_fp.close()
    base_addr_fp.close()

def te_ctrl_sram_print(se, sram_ext):
    pstr = ''
    for kmid in range(4):
        pstr += '\tkm_mode%d = %s, ' % (kmid, se['km_mode%d' % kmid]['value'])
        pstr += '\tkm_profile%d = %s, ' % (kmid, se['km_profile%d' % kmid]['value'])
        pstr += '\tkm_new_key%d = %s\n' % (kmid, se['km_new_key%d' % kmid]['value'])

    pstr += '\ttableid = %s, ' % se['tableid']['value']
    pstr += '\thash_sel = %s, ' % se['hash_sel']['value']
    pstr += '\thash_chain = %s, ' % se['hash_chain']['value']
    pstr += '\thash_store = %s, ' % se['hash_store']['value']
    pstr += '\tlkup = %s, ' % se['lkup']['value']

    pstr += '\tadv_phv_flit = %s, ' % sram_ext['adv_phv_flit']['value']
    pstr += '\tdone = %s\n' % sram_ext['done']['value']
    return pstr

def te_tbl_property_print(json_tbl_):
    pstr = ''
    pstr += '\taxi = %s\n' % json_tbl_['axi']['value']
    pstr += '\tkey_mask_hi = %s, ' % json_tbl_['key_mask_hi']['value']
    pstr += '\tkey_mask_lo = %s\n' % json_tbl_['key_mask_lo']['value']
    for k in range(2):
        pstr += '\tfullkey_km_sel%d = %s, ' % (k, json_tbl_['fullkey_km_sel%d' % k]['value'])
    pstr += '\n'
    pstr += '\tlock_en = %s\n' % json_tbl_['lock_en']['value']
    pstr += '\tmpu_pc_dyn = %s, ' % json_tbl_['mpu_pc_dyn']['value']
    pstr += '\tmpu_pc_loc = %s, ' % json_tbl_['mpu_pc_loc']['value']
    pstr += '\tmpu_pc = %s, ' % json_tbl_['mpu_pc']['value']
    pstr += '\tmpu_pc_raw = %s\n' % json_tbl_['mpu_pc_raw']['value']
    pstr += '\tmpu_pc_ofst_err = %s, ' % json_tbl_['mpu_pc_ofst_err']['value']
    pstr += '\tmpu_vec = %s\n' % json_tbl_['mpu_vec']['value']
    pstr += '\taddr_base = %s, ' % json_tbl_['addr_base']['value']
    pstr += '\taddr_sz = %s, ' % json_tbl_['addr_sz']['value']
    pstr += '\taddr_shift = %s\n' % json_tbl_['addr_shift']['value']
    pstr += '\tlg2_entry_size = %s\n' % json_tbl_['lg2_entry_size']['value']
    pstr += '\taddr_vf_id_en = %s, ' % json_tbl_['addr_vf_id_en']['value']
    pstr += '\taddr_vf_id_loc = %s\n' % json_tbl_['addr_vf_id_loc']['value']
    pstr += '\tchain_shift = %s\n' % json_tbl_['chain_shift']['value']
    return pstr

def capri_table_memory_spec_load(be):
    cur_path = os.path.abspath(__file__)
    cur_path = os.path.split(cur_path)[0]
    if be.args.target == 'haps':
        if be.args.p4_plus:
            spec_file_path = os.path.join(cur_path, 'specs/cap_memory_spec_p4plus_haps.json')
        else:
            spec_file_path = os.path.join(cur_path, 'specs/cap_memory_spec_haps.json')
            be.hw_model['parser']['num_states'] = 256   # HAPS platform supports 256 entry TCAM
    else:
        if be.args.p4_plus:
            spec_file_path = os.path.join(cur_path, 'specs/cap_memory_spec_p4plus.json')
        else:
            spec_file_path = os.path.join(cur_path, 'specs/cap_memory_spec.json')
    spec_file = open(spec_file_path)
    spec = json.load(spec_file)
    spec_file.close()
    return spec

def capri_pic_csr_load(tmgr):
    pic = {'sram':{'ingress':{}, 'egress':{}},
           'tcam':{'ingress':{}, 'egress':{}},
           'hbm' :{'ingress':[], 'egress':[]}}
    cur_path = os.path.abspath(__file__)
    cur_path = os.path.split(cur_path)[0]
    pics_file_path = os.path.join(cur_path, 'csr_json/cap_pics_csr.json')
    pict_file_path = os.path.join(cur_path, 'csr_json/cap_pict_csr.json')
    pics_file = open(pics_file_path)
    pict_file = open(pict_file_path)
    pic['sram']['ingress'] = json.load(pics_file)
    pic['tcam']['ingress'] = json.load(pict_file)
    pics_file.seek(0)
    pict_file.seek(0)
    # Read the files a second time for egress. Seems to be faster than copy.deepcopy()
    pic['sram']['egress']  = json.load(pics_file)
    pic['tcam']['egress']  = json.load(pict_file)
    pics_file.close()
    pict_file.close()
    cur_path = tmgr.be.args.gen_dir + '/%s/cfg_out/' % (tmgr.be.prog_name)
    for direction in tmgr.gress_tm:
        for stage in sorted(direction.stages):
            te_file_path = cur_path + 'cap_te_' + xgress_to_string(direction.d).upper() + '_' + repr(stage) + '_cfg_reg.json'
            te_file = open(te_file_path)
            pic['hbm'][xgress_to_string(direction.d)].append(json.load(te_file))
            te_file.close()

    return pic

def capri_pic_csr_output(be, out_pic):

    out_dir = be.args.gen_dir + '/%s/cfg_out/' % (be.prog_name)

    if not os.path.exists(out_dir):
        try:
            os.makedirs(out_dir)
        except OSError as e:
            if e.errno != errno.EEXIST:
                raise

    for mem_type in out_pic:
        for direction in out_pic[mem_type]:
            if be.args.p4_plus and (direction != xgress_to_string(xgress.INGRESS)):
                continue
            if mem_type == 'hbm':
                for stage in range(len(out_pic[mem_type][direction])):
                    out_file = out_dir + 'cap_te_' + direction.upper() + '_' + repr(stage) + '_cfg_reg.json'
                    with open(out_file, "w") as of:
                        json.dump(out_pic[mem_type][direction][stage], of, indent=2, sort_keys=True)
                        of.close()

                    prog_name = None
                    if not be.args.p4_plus:
                        prog_name = be.prog_name
                        if (direction == xgress_to_string(xgress.INGRESS)):
                            cap_mod = 'cap_sgi_te'
                        else:
                            cap_mod = 'cap_sge_te'
                    else:
                        prog_name = be.args.p4_plus_module
                        if be.args.p4_plus_module == 'rxdma':
                            cap_mod = 'cap_pcr_te'
                        elif be.args.p4_plus_module == 'txdma':
                            cap_mod = 'cap_pct_te'
                        else:
                            cap_mod = None
                    capri_dump_registers(be.args.cfg_dir, prog_name, cap_mod, stage,
                                         out_pic[mem_type][direction][stage], None)
            else:
                name = 'cap_pics_' if mem_type == 'sram' else 'cap_pict_'
                out_file = out_dir + name + direction.upper() + '.json'
                with open(out_file, "w") as of:
                    json.dump(out_pic[mem_type][direction], of, indent=2, sort_keys=True)
                    of.close()
                if (mem_type == 'sram'):
                    prog_name = None
                    if not be.args.p4_plus:
                        prog_name = be.prog_name
                        if (direction == xgress_to_string(xgress.INGRESS)):
                            cap_mod = 'cap_ssi_pics'
                        else:
                            cap_mod = 'cap_sse_pics'
                    else:
                        prog_name = be.args.p4_plus_module
                        if be.args.p4_plus_module == 'rxdma':
                            cap_mod = 'cap_rpc_pics'
                        elif be.args.p4_plus_module == 'txdma':
                            cap_mod = 'cap_tpc_pics'
                        else:
                            cap_mod = None
                    capri_dump_registers(be.args.cfg_dir, prog_name, cap_mod, 0,
                        out_pic[mem_type][direction]['cap_pics']['registers'], None)
                else:
                    prog_name = None
                    if not be.args.p4_plus:
                        prog_name = be.prog_name
                        if (direction == xgress_to_string(xgress.INGRESS)):
                            cap_mod = 'cap_tsi_pict'
                        else:
                            cap_mod = 'cap_tse_pict'
                    else:
                        prog_name = be.args.p4_plus_module
                        if be.args.p4_plus_module == 'rxdma':
                            cap_mod = 'cap_rpc_pict'
                        elif be.args.p4_plus_module == 'txdma':
                            cap_mod = 'cap_tpc_pict'
                        else:
                            cap_mod = None
                    capri_dump_registers(be.args.cfg_dir, prog_name, cap_mod, 0,
                        out_pic[mem_type][direction]['cap_pict']['registers'], None)

def capri_p4_table_spec_output(be, out_dict):

    out_dir = be.args.gen_dir + '/%s/p4pd/' % (be.prog_name)

    if not os.path.exists(out_dir):
        try:
            os.makedirs(out_dir)
        except OSError as e:
            if e.errno != errno.EEXIST:
                raise

    out_file = out_dir + 'capri_p4_table_spec.json'
    with open(out_file, "w") as of:
        json.dump(out_dict, of, indent=2, sort_keys=True)
        of.close()

def capri_p4_table_map_output(be, map_dict):

    out_dir = be.args.gen_dir + '/%s/p4pd/' % (be.prog_name)

    if not os.path.exists(out_dir):
        try:
            os.makedirs(out_dir)
        except OSError as e:
            if e.errno != errno.EEXIST:
                raise

    out_file = out_dir + 'capri_p4_table_map.json'
    with open(out_file, "w") as of:
        json.dump(map_dict, of, indent=2, sort_keys=True)
        of.close()

def capri_dump_table_memory(be, memory, tables, mem_type, region):

    out_dir = be.args.gen_dir + '/%s/p4pd/' % (be.prog_name)

    if not os.path.exists(out_dir):
        try:
            os.makedirs(out_dir)
        except OSError as e:
            if e.errno != errno.EEXIST:
                raise

    out_file = out_dir + 'capri_' + region + '_p4_' + mem_type + '_dump.txt'
    with open(out_file, "w") as of:
        free  = 0
        total = 0
        macros = OrderedDict()
        for macro in range(memory[mem_type][region]['width'] / memory[mem_type][region]['blk_w']):
            macros[macro] = OrderedDict()
        for row in range(memory[mem_type][region]['depth']):
            if row != 0 and (row % memory[mem_type][region]['blk_d']) == 0:
                of.write('\n')
            for col in range(memory[mem_type][region]['width']):
                if col != 0 and (col % memory[mem_type][region]['blk_w']) == 0:
                    of.write(' ')
                value = memory[mem_type][region]['space'][row][col]
                if value == 0:
                    free += 1
                else:
                    macros[col / memory[mem_type][region]['blk_w']][value] = 1
                total += 1
                of.write('%c' % value if value else '0')
            of.write('\n')

        for macro in macros:
            of.write('%d tables ' % (len(macros[macro])))

        of.write('\n')
        of.write('%d cells free out of %d (%d%% utilization).\n' % (free, total, math.ceil((total - free) * 100.0 / (total if total else 1))))
        of.write('\n')
        of.write('-0s represent free cells. Letters represent cells used by tables.\n')
        of.write('-Each cell is 16b wide and one row deep. Columns represent memory blocks.\n')
        of.write('-Table entry widths have been appended with 8b for PC value and been padded to the next 16b boundary.\n')
        of.write('-To achieve efficient packing of all tables, some tables may have been sliced depth-wise and placed side by side.\n')
        of.write('\n')
        reqd  = 0
        total = 0
        for i in range(len(tables[mem_type][region])):
            width = capri_get_width_from_layout(tables[mem_type][region][i]['layout'])
            depth = capri_get_depth_from_layout(tables[mem_type][region][i]['layout'])
            reqd += tables[mem_type][region][i]['width'] * tables[mem_type][region][i]['depth']

            if width == 0 and depth == 0:
                layout_string = 'width: NONE, depth: NONE'
            else:
                layout_string = 'width: %d, depth: %d' % (width, depth)
                total += width * depth

            of.write('%c: %s (width = %d, depth = %d). Stage = %d. Layout{%s}.\n' % (i + 65,
                                                       tables[mem_type][region][i]['name'],
                                                       tables[mem_type][region][i]['width'],
                                                       tables[mem_type][region][i]['depth'],
                                                       tables[mem_type][region][i]['stage'],
                                                       layout_string))
        of.write('\n')
        of.write('Total %d tables. Total %d cells required' % (len(tables[mem_type][region]), reqd))
        of.close()

def capri_get_top_level_path(cur_dir):
    top_dir = '.'
    subpaths = os.path.abspath(cur_dir).split("nic/")
    del subpaths[0] # Drop everything until the topmost 'nic' and then count directories below it
    for subpath in subpaths:
        dirs = subpath.split("/")
        for dir in dirs:
            top_dir += '/..'

    return top_dir
