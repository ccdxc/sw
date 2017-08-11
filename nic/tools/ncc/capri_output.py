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
from collections import OrderedDict
from enum import IntEnum
from capri_utils import *

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
    pstr = ''
    pstr += 'action = %s, ' % se['action']['value']
    pstr += 'nxt_state = %s, ' % se['nxt_state']['value']
    pstr += 'offset_inst: sel %s, mux_sel %s, val %s ' % \
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
            pstr += 'lkp_val_inst[%s] = retain_old_val\n' % (i)
        elif lkp_val_inst['sel']['value'] == '3':
            pstr += 'lkp_val_inst[%s] = stored_lkp_val\n' % (i)
        else:
            pass
    if used:
        pstr += '\nTotal lkp_val_inst %s\n' % used

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

    if used:
        pstr += 'Total mux_inst %s\n' % used

    return pstr

def _parser_init_profile_print(parser, cfg_init_profile):
    pstr = 'Parser init_profile\n'
    pstr += 'curr_offset = %s, ' % cfg_init_profile['curr_offset']['value']
    pstr += 'state = %s, ' % cfg_init_profile['state']['value']
    pstr += 'lkp_val_pkt_idx0 = %s, ' % cfg_init_profile['lkp_val_pkt_idx0']['value']
    pstr += 'lkp_val_pkt_idx1 = %s, ' % cfg_init_profile['lkp_val_pkt_idx1']['value']
    pstr += 'lkp_val_pkt_idx2 = %s\n' % cfg_init_profile['lkp_val_pkt_idx2']['value']
    return pstr

def _build_mux_inst(parser, cs, rid, mux_inst, mux_id, _capri_expr):
    # DO NOT modify _capri_expr, this function is called multiple times for the same state
    capri_expr = copy.copy(_capri_expr)
    if capri_expr.op2:
        assert rid != -1, pdb.set_trace()
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
        off = cs.fld_off[capri_expr.src1]
        eoff = off + capri_expr.src1.width
        # lookup fields > 16 bits are not currently supported. That requires concatenation
        # of multiple lkp regs - TBD
        assert capri_expr.src1.width <= 16
        c_soff = (off / 8) * 8
        c_eoff = ((eoff+7)/8) * 8
        # since the lkp regs are 16bits each, end offset must be aligned to register
        c_eoff = max(c_eoff, (c_soff + 16))
        if c_soff != off:
            # not aligned at the start of the container - nothing to do
            pass

        if capri_expr.mask:
            mask = capri_expr.mask
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

        assert (capri_expr.shft >= 0 and capri_expr.shft < 16), pdb.set_trace()
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

def capri_asm_output_pa(gress_pa):
    gen_dir = gress_pa.pa.be.args.gen_dir
    cur_path = gen_dir + '/%s/asm_out' % gress_pa.pa.be.prog_name
    if not os.path.exists(cur_path):
        os.makedirs(cur_path)
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
            if cf.is_union_storage:
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
                        max_size_in_first_union = 0
                        for uh in uh_list:
                            if gress_pa.get_header_phv_size(uh) == 0:
                                pstr += indent2 + '// skip header %s - ' \
                                        'Nothing in PHV\n' % uh.name
                                continue
                            add_pad = 0
                            last_ucf_width = 0
                            this_hdr_broken_size = 0
                            uhfields_str = ''
                            for f in uh.fields:
                                ucf = gress_pa.get_field(get_hfname(f))
                                last_ucf = ucf
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
                            if this_hdr_broken_size > max_size_in_first_union:
                                max_size_in_first_union = this_hdr_broken_size

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
                            pad_size = (max_unioned_hdr_size * 8) - max_size_in_first_union
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
                    assert cf in gress_pa.fld_unions
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

                    pstr += indent + ('%s : %d; // %s, Crossed Flits ' \
                            '(from flit %d to %d). Split into two fields\n' % \
                            (_get_output_name(cf.hfname) + '_1',  this_flit_bits,
                                _get_phv_loc(flit_inst, cf.phv_bit, this_flit_bits),
                                flit_inst, flit_inst+1))

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
                assert(0), pdb.set_trace()

    for i in range (num_flits):
        comment = '/* --------------- Phv Flit %d ------------ */\n\n' % i
        pstr_flit[i] = comment + pstr_flit[i]
    for i in range (num_flits-1, -1, -1):
        p_str += pstr_flit[i]
    p_str += '};\n'

    hfile.write(p_str)
    hfile.close()


def capri_asm_output_table(be, ctable):

    # Table KI, KD asm code is not generated
    # using tenjin template. This code is not
    # used any more. For now putting a return
    # here. This code is potential candidate 
    # for cleanup -- TBD done once asm code
    # stabilizes.
    return

    hw_model = be.hw_model
    gen_dir = be.args.gen_dir
    cur_path = gen_dir + '/%s/asm_out' % be.prog_name
    flit_sz = hw_model['phv']['flit_size']
    if not os.path.exists(cur_path):
        os.makedirs(cur_path)
    fname = cur_path + '/%s_%s_k.h' % (ctable.d.name, ctable.p4_table.name)
    hfile = open(fname, 'w')

    pstr = 'struct %s_k {\n' % ctable.p4_table.name
    key_cfs = [cf for cf,_,_ in ctable.keys] 
    cfields = key_cfs + ctable.input_fields
    ki_fields = sorted(cfields, key=lambda f: f.phv_bit)

    indent = '    '
    indent2=indent+indent
    # handle unions
    #k_off = ctable.key_size + ctable.d_size
    k_off = sum([cf.width for cf in ki_fields])
    for cf in ki_fields:
        pstr += indent + '%s : %d; // %d : phv[%d:%d]\n' % \
            (_get_output_name(cf.hfname), cf.width, k_off-cf.width,
             cf.phv_bit / flit_sz, cf.phv_bit % flit_sz)
        k_off -= cf.width
    pstr += '};\n'
    hfile.write(pstr)
    hfile.close()

    fname = cur_path + '/%s_%s_d.h' % (ctable.d.name, ctable.p4_table.name)
    hfile = open(fname, 'w')

    for a,ad in ctable.action_data.items():
        pstr = ''
        # Generate k+d if hash table.
        pstr += 'struct %s_%s_d {\n' % (ctable.p4_table.name, a)
        if ctable.match_type == match_type.EXACT_HASH:
            key_cfs = [cf for cf,_,_ in ctable.keys] 
            cfields = key_cfs
            k_fields = sorted(cfields, key=lambda f: f.phv_bit)
            k_off = sum([cf.width for cf in key_cfs])
            for cf in k_fields:
                pstr += indent + '%s : %d; // %d : phv[%d:%d]\n' % \
                    (_get_output_name(cf.hfname), cf.width, k_off-cf.width,
                     cf.phv_bit / flit_sz, cf.phv_bit % flit_sz)
                
                k_off -= cf.width
        # no ifdefs are needed by the assembler
        # pstr += '#ifdef %s\n' % a.upper()
        if not len(ad):
            continue
        # print each action parameter for this action
        for ap in ad:
            pstr += indent + '%s : %d;\n' % (ap[0], ap[1])
        pstr += '};\n'
        # pstr += '#endif /* %s */\n' % a.upper()
        hfile.write(pstr)

    if len(ctable.action_data):
        pstr = 'struct %s_d {\n' % (ctable.p4_table.name)
        pstr += indent + 'union {\n'
        for a,ad in ctable.action_data.items():
            if not len(ad):
                continue
            pstr += indent2 + 'struct %s_%s_d %s_d;\n' % (ctable.p4_table.name, a, a)
        pstr += indent + '} u;\n'
        pstr += '};\n'
        hfile.write(pstr)
    hfile.close()


def capri_parser_logical_output(parser):
    gen_dir = parser.be.args.gen_dir
    out_dir = gen_dir + '/%s/logs/' % (parser.be.prog_name)
    if not os.path.exists(out_dir):
        os.makedirs(out_dir)
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
        os.makedirs(out_dir)
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

        assert h in deparser.topo_ordered_phv_ohi_chunks, pdb.set_trace()
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
        os.makedirs(cfg_out_dir)

    dpp_cfg_output_reg = os.path.join(cfg_out_dir, 'cap_dpp_%s_cfg_reg.json' \
                         % deparser.d.name)
    dpp_cfg_file_reg = open(dpp_cfg_output_reg, 'w+')

    dpr_cfg_output_reg = os.path.join(cfg_out_dir, 'cap_dpr_%s_cfg_reg.json' \
                         % deparser.d.name)
    dpr_cfg_file_reg = open(dpr_cfg_output_reg, 'w+')

    deparser.logger.info('%s:DeParser Dpp & Dpr Block Output Generation:' \
                         % deparser.d.name)

    used_hdr_fld_info_slots = 0
    max_hdr_flds = deparser.be.hw_model['deparser']['max_hdr_flds']
    phv_sel = deparser.be.hw_model['deparser']['dpa_src_phv']
    ohi_sel = deparser.be.hw_model['deparser']['dpa_src_ohi']
    fixed_sel = deparser.be.hw_model['deparser']['dpa_src_fixed']
    cfg_sel = deparser.be.hw_model['deparser']['dpa_src_cfg']
    pkt_sel = deparser.be.hw_model['deparser']['dpa_src_pkt']
    max_hv_bit_idx = deparser.be.hw_model['parser']['max_hv_bits'] - 1

    # Last header field slot is reserved for payload len. 
    # Hence max available hdr-field slot is one less.

    # Fill in payload len related information.
    cf = deparser.be.pa.get_field("capri_p4_intrinsic.frame_size", deparser.d)
    if cf:
        payload_offset_len_ohi_id = deparser.be.hw_model['parser']['ohi_threshold']
        payload_offset_ohi_bit = 1<<payload_offset_len_ohi_id
        dpp_json['cap_dpp']['registers']['cap_dpp_csr_cfg_ohi_payload']\
            ['ohi_slot_payload_ptr_bm']['value'] = str('0x%x' % payload_offset_ohi_bit)

        # Because header bit 127 is used for payload, use hdrfld_info slot 255 (last slot)
        # for specifying packet payload ohi information. OHI slot contains payload offset,
        # len comes from fram_len phv
        rstr = 'cap_dpphdr_csr_cfg_hdr_info[%d]' % (max_hv_bit_idx)
        dpp_json['cap_dpp']['registers'][rstr]['fld_start']['value'] = str(max_hdr_flds-1)
        dpp_json['cap_dpp']['registers'][rstr]['fld_end']['value'] = str(0)
        dpp_rstr_name = 'cap_dpphdrfld_csr_cfg_hdrfld_info[%d]' % (max_hdr_flds-1)
        dpr_rstr_name = 'cap_dprhdrfld_csr_cfg_hdrfld_info[%d]' % (max_hdr_flds-1)
        dpp_rstr = dpp_json['cap_dpp']['registers'][dpp_rstr_name]
        dpr_rstr = dpr_json['cap_dpr']['registers'][dpr_rstr_name]
        # hw gets total frame len from frame_size phv, needs the ohi_slot carrying payload_offset
        # to subtract it from total len
        dpp_rstr['size_sel']['value'] = str(ohi_sel)
        dpp_rstr['size_val']['value'] = str(payload_offset_len_ohi_id)
        # get pkt_offset from last(sw) ohi slot
        dpr_rstr['source_sel']['value'] = str(ohi_sel)
        dpr_rstr['source_oft']['value'] = str(payload_offset_len_ohi_id)


    # Fill in all header fields information.
    for hvb in range(max_hv_bit_idx, -1, -1):
        h = deparser.be.parsers[deparser.d].hv_bit_header[hvb]
        if (h == None):
            continue

        assert h in deparser.topo_ordered_phv_ohi_chunks, pdb.set_trace()
        dp_hdr_fields = deparser.topo_ordered_phv_ohi_chunks[h]

        #compute number of header field info slots this header needs.
        # which is sum of PHV chunks and OHI slots
        phvchunks = 0 
        ohis = 0 

        #Generate DPP block configurations
        rstr = 'cap_dpphdr_csr_cfg_hdr_info[%d]' % (max_hv_bit_idx - hvb)
        # Logic = all_1_mask >> fld_end  & (all_1_mask << fld_start)
        
        first_ohi = False
        start_fld = used_hdr_fld_info_slots
        for i, chunks in enumerate(dp_hdr_fields):
            assert used_hdr_fld_info_slots < (max_hdr_flds-1), "No hdr fld slots avaialble"
            dpp_rstr_name = 'cap_dpphdrfld_csr_cfg_hdrfld_info[%d]' % (used_hdr_fld_info_slots)
            dpr_rstr_name = 'cap_dprhdrfld_csr_cfg_hdrfld_info[%d]' % (used_hdr_fld_info_slots)
            dpp_rstr = dpp_json['cap_dpp']['registers'][dpp_rstr_name]
            dpr_rstr = dpr_json['cap_dpr']['registers'][dpr_rstr_name]
            (phv_ohi, chunk_type, _) = chunks
            if (chunk_type == deparser.field_type_phv):
                dpp_rstr['size_sel']['value'] = str(fixed_sel)
                dpp_rstr['size_val']['value'] = str(phv_ohi[1]/8) # in bytes

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
                    dpp_rstr['size_val']['value'] = str(phv_ohi.length)
                else:
                    dpp_rstr['size_sel']['value'] = str(ohi_sel)
                    dpp_rstr['size_val']['value'] = str(phv_ohi.var_id)
            used_hdr_fld_info_slots += 1

        dpp_json['cap_dpp']['registers'][rstr]['fld_start']['value'] = str(start_fld)
        dpp_json['cap_dpp']['registers'][rstr]['fld_end']['value'] = \
            str(max_hdr_flds - used_hdr_fld_info_slots)

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
        os.makedirs(dbg_out_dir)

    dbg_out_fname = os.path.join(dbg_out_dir, 'model_debug.json')
    dbgfile = open(dbg_out_fname, 'w+')
    json.dump(dbg_info, dbgfile, indent=4, sort_keys=False, separators=(',', ': '))
    dbgfile.close()

def capri_output_i2e_meta_header(be, i2e_fields, hsize):
    gen_dir = be.args.gen_dir
    cur_path = gen_dir + '/%s/asm_out' % be.prog_name
    if not os.path.exists(cur_path):
        os.makedirs(cur_path)
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
    assert ename in reg_json
    elem = reg_json[ename]
    if 'decoder' in elem.keys():
        decoder_name = elem['decoder']
        return _expand_decoder(decoder_json, decoder_name), decoder_name
    else:
        return copy.deepcopy(elem), None

def _fill_parser_sram_entry(sram_t, parser, bi, add_cs = None):
    parser.logger.debug("%s:fill_sram_entry for %s + %s" % \
        (parser.d.name, bi.nxt_state, add_cs))
    sram = copy.deepcopy(sram_t)

    def _mux_idx_alloc(pkt_off):
        for i, used in enumerate(mux_idx_allocator):
            # check for sharing a slot
            if used != None and mux_idx_allocator[i] == pkt_off:
                return i
        for i, used in enumerate(mux_idx_allocator):
            if used == None:
                mux_idx_allocator[i] = pkt_off
                return i
        assert 0, pdb.set_trace()

    def _mux_inst_alloc():
        for i, used in enumerate(mux_inst_allocator):
            if used == None:
                mux_inst_allocator[i] = 1
                return i
        assert 0, pdb.set_trace()
    # add_cs is valid when this is the first state that h/w enters via cfg registers
    # add extractions, setops, hv bits etc from add_cs to this
    add_off = 0 # Bytes
    if add_cs:
        add_off = add_cs.extract_len    # in Bytes
        assert isinstance(add_off, int), \
            "Variable size extraction not allowed on start state %s" % \
                (add_cs.name)

    mux_idx_allocator = [None for _ in sram['mux_idx']]
    mux_inst_allocator = [None for _ in sram['mux_inst']]

    nxt_cs = bi.nxt_state

    sram['action']['value'] = str(1) if nxt_cs.is_end else str(0)
    sram['nxt_state']['value'] = str(nxt_cs.id)

    current_flit = None
    # for pre extraction, use info from next state for offset
    # offset_inst
    offset_inst = sram['offset_inst']
    if isinstance(nxt_cs.extract_len, int):
        offset_inst['sel']['value'] = str(0)
        offset_inst['muxsel']['value'] = str(0)
        offset_inst['val']['value'] = str(nxt_cs.extract_len + add_off)
    else: 
        #assert isinstance(nxt_cs.extract_len, capri_parser_expr)
        mux_inst_id = _mux_inst_alloc()
        off = 0
        if nxt_cs.extract_len.src1:
            off = nxt_cs.fld_off[nxt_cs.extract_len.src1]

        # use add_off to access pkt field from nxt_cs
        mux_id = _mux_idx_alloc((off/8)+add_off)
        sram['mux_idx'][mux_id]['sel']['value'] = str(0)
        sram['mux_idx'][mux_id]['lkpsel']['value'] = str(0)   # NA
        sram['mux_idx'][mux_id]['idx']['value'] = str((off/8)+add_off)
        _build_mux_inst(parser, nxt_cs, -1, 
            sram['mux_inst'][mux_inst_id], mux_id, nxt_cs.extract_len)
        offset_inst['sel']['value'] = str(1)
        offset_inst['muxsel']['value'] = str(mux_inst_id)
        # add_off is not added into expr, instead it is added by h/w using 'value'
        offset_inst['val']['value'] = str(add_off)

    # lkp_val_inst
    for r,lkp_reg in enumerate(nxt_cs.lkp_regs):
        if lkp_reg.type == lkp_reg_type.LKP_REG_NONE:
            continue
        elif lkp_reg.type == lkp_reg_type.LKP_REG_PKT:
            mux_id = _mux_idx_alloc((lkp_reg.pkt_off/8) + add_off)
            sram['mux_idx'][mux_id]['sel']['value'] = str(0)
            sram['mux_idx'][mux_id]['lkpsel']['value'] = str(0)   # NA
            sram['mux_idx'][mux_id]['idx']['value'] = str((lkp_reg.pkt_off/8) + add_off)

            sram['lkp_val_inst'][r]['sel']['value'] = str(0)
            sram['lkp_val_inst'][r]['muxsel']['value'] = str(mux_id)

        elif lkp_reg.type == lkp_reg_type.LKP_REG_RETAIN:
            sram['lkp_val_inst'][r]['sel']['value'] = str(2)
            sram['lkp_val_inst'][r]['muxsel']['value'] = str(0)  # NA

        elif lkp_reg.type == lkp_reg_type.LKP_REG_STORED:
            sram['lkp_val_inst'][r]['sel']['value'] = str(3)
            sram['lkp_val_inst'][r]['muxsel']['value'] = str(0)  # NA
        else: # REG_LOAD/UPDATE
            if lkp_reg.capri_expr:
                if lkp_reg.capri_expr.op1:
                    assert lkp_reg.capri_expr.src1, pdb.set_trace()
                    assert lkp_reg.pkt_off != -1
                    off = lkp_reg.pkt_off
                    mux_id = _mux_idx_alloc((off/8) + add_off)
                    sram['mux_idx'][mux_id]['sel']['value'] = str(0)
                    sram['mux_idx'][mux_id]['lkpsel']['value'] = str(0)   # NA
                    sram['mux_idx'][mux_id]['idx']['value'] = str((off/8) + add_off)
                else:
                    mux_id = 0    # NA
                mux_inst_id = _mux_inst_alloc()
                _build_mux_inst(parser, nxt_cs, r, sram['mux_inst'][mux_inst_id], 
                    mux_id, lkp_reg.capri_expr)
                sram['lkp_val_inst'][r]['sel']['value'] = str(1)
                sram['lkp_val_inst'][r]['muxsel']['value'] = str(mux_inst_id)
            else:
                # local var load from pkt
                if lkp_reg.first_pkt_fld:
                    assert lkp_reg.pkt_off != -1
                    off = lkp_reg.pkt_off
                    mux_id = _mux_idx_alloc((lkp_reg.pkt_off/8) + add_off)
                    sram['mux_idx'][mux_id]['sel']['value'] = str(0)
                    sram['mux_idx'][mux_id]['lkpsel']['value'] = str(0)   # NA
                    sram['mux_idx'][mux_id]['idx']['value'] = str((lkp_reg.pkt_off/8) + add_off)

                    sram['lkp_val_inst'][r]['sel']['value'] = str(0)
                    sram['lkp_val_inst'][r]['muxsel']['value'] = str(mux_id)
            
        if lkp_reg.store_en:
            sram['lkp_val_inst'][r]['store_en']['value'] = str(1)
        else:
            sram['lkp_val_inst'][r]['store_en']['value'] = str(0)

    # extract_inst
    # For all fields that go to phv, check if fields can be combined to extract
    # 4 bytes at a time (Needed only if more than 16 fields are extracted)
    # ASSUME pre-extraction i.e. fields from a state are extracted *before* entering the
    # state
    parser_num_flits = parser.be.hw_model['parser']['parser_num_flits']
    flit_size = parser.be.hw_model['parser']['flit_size']
    flit_sizeB = flit_size / 8

    insts = parser.generate_extract_instructions(nxt_cs, add_cs)
    if (len(insts)):
        current_flit = insts[0][2] / flit_sizeB
    #assert len(insts) <= len(sram['extract_inst']), "%s:Too many(%d) extractions" % \
    #    (nxt_cs.name, len(insts))
    if len(insts) > len(sram['extract_inst']):
        parser.logger.critical("Violation:%s:Too many(%d) extractions" % \
            (nxt_cs.name, len(insts)))
        assert 0, pdb.set_trace()   # possibly internal error, may need state splitting

    for e, ext_ins in enumerate(insts):
        sram['extract_inst'][e]['pkt_idx']['value'] = str(ext_ins[0])
        sram['extract_inst'][e]['len']['value']     = str(ext_ins[1])
        sram['extract_inst'][e]['phv_idx']['value'] = str(ext_ins[2] & 0x7F) # remove flit bits

    #pdb.set_trace()
    # ohi_inst
    hw_max_ohi_per_state = len(sram['ohi_inst'])
    s = 0
    headers = []
    if add_cs:
        headers += add_cs.headers
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

            assert s < hw_max_ohi_per_state, pdb.set_trace() # 'Too many OHIs in a state %s' % cs.name

            if ohi.id == -1:
                assert len(parser.ohi[hdr]) > 1, pdb.set_trace() # ERROR
            else:
                # slot[0] : pkt offset
                sram['ohi_inst'][s]['sel']['value'] = str(1)    # provide pkt_offset
                sram['ohi_inst'][s]['muxsel']['value'] = str(0) # NA
                sram['ohi_inst'][s]['idx_val']['value'] = str(hdr_off + ohi.start)
                sram['ohi_inst'][s]['slot_num']['value'] = str(ohi.id)
                s += 1

            if not isinstance(ohi.length, int):
                #pdb.set_trace()
                # ohi.length is a capri expression, need to allocate mux_idx and inst
                mux_inst_id = _mux_inst_alloc()
                op_off = 0
                assert ohi.length.src1, "No oprand for ohi.length expression %s" % ohi.length
                op_off = ohi.length.src1.p4_fld.offset
                # pkt_mux provides correct pkt field for calculation
                mux_id = _mux_idx_alloc((op_off/8) + hdr_off)
                sram['mux_idx'][mux_id]['sel']['value'] = str(0)
                sram['mux_idx'][mux_id]['lkpsel']['value'] = str(0)   # NA
                sram['mux_idx'][mux_id]['idx']['value'] = str((op_off/8) + hdr_off)
                _build_mux_inst(parser, cs, -1, 
                    sram['mux_inst'][mux_inst_id], mux_id, ohi.length)
                # slot[1] : ohi_len
                sram['ohi_inst'][s]['sel']['value'] = str(3)  # len using mux_inst_data
                sram['ohi_inst'][s]['muxsel']['value'] = str(mux_inst_id)
                sram['ohi_inst'][s]['idx_val']['value'] = str(0) # NA
                sram['ohi_inst'][s]['slot_num']['value'] = str(ohi.var_id)
                s += 1
            else:
                # for fixed size ohi, deparser is programmed with fixed length
                pass
                
            parser.logger.debug('OHI instruction[%d]: off %d, len %s' % \
                (ohi.id, ohi.start + hdr_off, ohi.length))

    if nxt_cs.is_end:
        # need to capture current_offset where parser stops parsing. This is needed for the
        # deparser, use offset instruction for computing ohi. When explicit END state
        # is used, offset_instruction will be 0
        ohi_payload_slot = parser.be.hw_model['parser']['ohi_threshold']
        assert s < hw_max_ohi_per_state, pdb.set_trace()# 'No OHI instr available in end state %s' % cs.name
        if isinstance(nxt_cs.extract_len, int):
            sram['ohi_inst'][s]['sel']['value'] = str(1)    # integer extraction
            sram['ohi_inst'][s]['muxsel']['value'] = str(0) # NA
            sram['ohi_inst'][s]['idx_val']['value'] = offset_inst['val']['value']
            sram['ohi_inst'][s]['slot_num']['value'] = str((ohi_payload_slot))
        else: 
            assert add_off == 0, "XXX add_off and exit is not a common case, need to support this"
            offset_inst['val']['value'] = str(add_off)
            sram['ohi_inst'][s]['sel']['value'] = str(3)    # use mux inst
            sram['ohi_inst'][s]['muxsel']['value'] = offset_inst['muxsel']['value']
            sram['ohi_inst'][s]['idx_val']['value'] = str(0)
            sram['ohi_inst'][s]['slot_num']['value'] = str((ohi_payload_slot))
        parser.logger.debug('%s:%s:End State: OHI instruction[%d] %d len %d' % \
                            (parser.d.name, nxt_cs.name, s, ohi_payload_slot, nxt_cs.extract_len))
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
        max_hv_bit_idx = 0 # Use last bit in BE order for payload len hv_en
        hv_byte = max_hv_bit_idx / 8
        boff = max_hv_bit_idx % 8
        hv_bits[hv_byte] = (1 << (7 - boff))

    mid = 0
    max_mid = len(sram['meta_inst'])
    # select values
    meta_ops = parser.be.hw_model['parser']['parser_consts']['meta_ops']
    for hv_byte,hv in hv_bits.items():
        assert mid < max_mid
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
        assert mid < max_mid
        if op.op_type == meta_op.EXTRACT_REG:
            #pdb.set_trace() # un-tested so far
            assert op.rid
            mux_inst_id = _mux_inst_alloc()
            _build_mux_inst(parser, op.cstate, op.rid, sram['mux_inst'][mux_inst_id], 
                0, op.capri_expr)
            dst_phv = op.dst.phv_bit
        elif op.op_type == meta_op.EXTRACT_META:
            #pdb.set_trace()
            assert op.src1
            mux_inst_id = _mux_inst_alloc()
            op_off = op.cstate.fld_off[op.src1]
            if op.cstate == nxt_cs:
                fld_off = (op_off/8) + add_off
            else:
                fld_off = (op_off/8)
                
            mux_id = _mux_idx_alloc((fld_off))
            sram['mux_idx'][mux_id]['sel']['value'] = str(0)
            sram['mux_idx'][mux_id]['lkpsel']['value'] = str(0)   # NA
            sram['mux_idx'][mux_id]['idx']['value'] = str(fld_off)
            _build_mux_inst(parser, op.cstate, None, sram['mux_inst'][mux_inst_id], 
                mux_id, op.capri_expr)
            dst_phv = op.dst.phv_bit
        elif op.op_type == meta_op.EXTRACT_CONST:
            #pdb.set_trace()
            dst_phv = op.dst.phv_bit
            assert (dst_phv % 8) == 0, "Destination phv %s must be byte aligned" % op.dst.hfname
            val = op.const
        else:
            continue

        flit = dst_phv / flit_size
        if not current_flit:
            current_flit = flit
        assert current_flit == flit, pdb.set_trace()

        sram['meta_inst'][mid]['phv_idx']['value'] = str((dst_phv / 8) & 0x7F)
        if mux_inst_id == None:
            sram['meta_inst'][mid]['sel']['value'] = meta_ops['set_val']
            sram['meta_inst'][mid]['val']['value'] = str(val)
        else:
            sram['meta_inst'][mid]['sel']['value'] = meta_ops['set_mux_data']
            sram['meta_inst'][mid]['val']['value'] = str(mux_inst_id)
        mid += 1

    # fill unused instructions to do NOP - XXX
    # set up the unused meta instr to perform |= 0
    for m in range(mid, max_mid):
        sram['meta_inst'][mid]['sel']['value'] = meta_ops['nop']
        sram['meta_inst'][mid]['phv_idx']['value'] = str(0)
        sram['meta_inst'][mid]['val']['value'] = str(0)

    if current_flit != None:
        parser.logger.debug("%s:%s:Flit # %d" % (parser.d.name, nxt_cs.name, current_flit))
        sram['phv_idx_upr']['value'] = str(current_flit)
    else:
        parser.logger.debug("%s:%s:No PHV writes in this state" % (parser.d.name, nxt_cs.name))

    return sram

def _fill_parser_sram_catch_all(sram_t):
    sram = copy.deepcopy(sram_t)
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
        os.makedirs(cfg_out_dir)
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
    tcam0 = []
    sram0 = []

    for cs in parser.states:
        # create a match entry for {state_id, lkp_flds, lkp_fld_mask}
        if not parser.be.args.post_extract and cs.is_end:
            # Terminal state
            parser.logger.debug('Skip transition from %s -> __END__, Terminate' % (cs.name))
            continue
        for bi in cs.branches:
            parser.logger.debug('%s:%s[%d]->%s[%d]' % 
                    (parser.d.name, cs.name, cs.id, bi.nxt_state.name, \
                     bi.nxt_state.id))
            te = _fill_parser_tcam_entry(tcam_t, parser, cs, bi)
            if cs.is_hw_start:
                add_cs = cs
            else:
                add_cs = None
            se = _fill_parser_sram_entry(sram_t, parser, bi, add_cs)

            # Allow smaller json definition file and add entries
            te['entry_idx'] = str(idx)  # debug aid
            se['entry_idx'] = str(idx)  # debug aid
            if idx < len(tcam0):
                tcam0[idx] = te
                sram0[idx] = se
            else:
                tcam0.append(te)
                sram0.append(se)
            parser.logger.debug('TCAM-decoder[%d] - \n%s' % (idx, _parser_tcam_print(tcam0[idx])))
            parser.logger.debug('SRAM-decoder[%d] - \n%s' % \
                (idx, _parser_sram_print(parser,sram0[idx])))
            idx += 1
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
    # program catch all entry register
    ppa_json['cap_ppa']['registers']['cap_ppa_csr_cfg_ctrl']['pe_enable']['value'] = str(0x3ff)
    ppa_json['cap_ppa']['registers']['cap_ppa_csr_cfg_ctrl']['parse_loop_cnt']['value'] = str(64)
    ppa_json['cap_ppa']['registers']['cap_ppa_csr_cfg_ctrl']['num_phv_flit']['value'] = str(6)
    ppa_json['cap_ppa']['registers']['cap_ppa_csr_cfg_ctrl']\
        ['state_lkp_catchall_entry']['value'] = str(idx)
    idx += 1
    parser.logger.info('%s:Tcam states used %d' % (parser.d.name, idx))
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

def _fill_te_tcam_catch_all_run(tcam_t):
    # create an entry that will always be a 'hit'
    te = copy.deepcopy(tcam_t)
    te['valid']['value'] = str(1)
    te['value']['value'] = str(0)
    te['mask']['value'] = str(0)
    return te

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
        os.makedirs(cfg_out_dir)
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
    num_cycles = stage.gtm.tm.be.hw_model['match_action']['num_cycles']
    te_consts = stage.gtm.tm.be.hw_model['match_action']['te_consts']
    no_load_byte = te_consts['no_load_byte']
    no_load_bit = te_consts['no_load_bit']

    json_regs = te_json['cap_te']['registers']
    json_mems = te_json['cap_te']['memories']

    for prof in stage.hw_km_profiles:
        if prof == None:
            continue
        hw_id = prof.hw_id
        assert hw_id < max_km_profiles, pdb.set_trace()
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
            sel_id += 1
        # load rest of the bytes to not load the km
        for b in range(sel_id, ((hw_id+1) * max_km_wB)):
            if sel_id >= 256: pdb.set_trace()
            json_km_profile = json_regs['cap_te_csr_cfg_km_profile_byte_sel[%d]' % sel_id]
            json_km_profile['byte_sel']['value'] = no_load_byte
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
            bit_sel_id -= 1
            bits_left -= 1
        
        for b in range(bits_left):
            json_km_profile = json_regs['cap_te_csr_cfg_km_profile_bit_sel[%d]' % bit_sel_id]
            json_km_profile['bit_sel']['value'] = no_load_bit
            bit_sel_id -= 1

        # bit_loc1 is bit0-7, bit_loc0 is bit8-15
        # since bit_sels are written from 15 down to 0, no need to switch bit_loc here
        # XXX check for <= 8 can be removed or turned into assert, just look at bit_locX >= 0
        if len(prof.bit_sel) <= 8:
            json_km_profile = json_regs['cap_te_csr_cfg_km_profile_bit_loc[%d]' % (bit_loc_id)]
            if prof.bit_loc >= 0:
                json_km_profile['valid']['value'] = str(1)
                json_km_profile['bit_loc']['value'] = str(prof.bit_loc)
            else:
                json_km_profile['valid']['value'] = str(0)
            json_km_profile = json_regs['cap_te_csr_cfg_km_profile_bit_loc[%d]' % (bit_loc_id+1)]
            json_km_profile['valid']['value'] = str(0)
        else:
            json_km_profile = json_regs['cap_te_csr_cfg_km_profile_bit_loc[%d]' % (bit_loc_id)]
            if prof.bit_loc >= 0:
                json_km_profile['valid']['value'] = str(1)
                json_km_profile['bit_loc']['value'] = str(prof.bit_loc)
            else:
                json_km_profile['valid']['value'] = str(0)
            json_km_profile = json_regs['cap_te_csr_cfg_km_profile_bit_loc[%d]' % (bit_loc_id+1)]
            if prof.bit_loc1 >= 0:
                json_km_profile['valid']['value'] = str(1)
                json_km_profile['bit_loc']['value'] = str(prof.bit_loc1)
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
        te = _fill_te_tcam_catch_all_run(tcam_t)
        json_regs['cap_te_csr_cfg_table_profile_cam[%d]' % idx] = te
        # profile_id 0 is used to populate all the tables by the compiler
        run_all_tables = True

    else:
        # ASIC does not launch any lookup on TCAM miss.. so need to program any catchall

        w, cf_key_sel = stage.stg_get_tbl_profile_key()
        k = 0
        # program the key selector
        for cf in cf_key_sel:
            phv_bit = cf.phv_bit
            stage.gtm.tm.logger.debug("%s:Stage %d:Table profile key field %s phv %d, w %d" % \
                (stage.gtm.d.name, stage.id, cf.hfname, cf.phv_bit, cf.width))
            for b in range(cf.width):
                json_tbl_prof_key['sel%d' % k]['value'] = str(phv_bit)
                phv_bit += 1
                k += 1

        assert k == w, pdb.set_trace()
        for k in range(w, max_key_sel):
            json_tbl_prof_key['sel%d' % k]['value'] = str(0)

    prof_idx = 0

    profile_vals = sorted(stage.table_profiles.keys(), reverse=True)
    tcam_entries = OrderedDict()
    for prof_val in profile_vals:
        ctg = stage.table_profiles[prof_val]
        if not run_all_tables:
            # program tcam entries for predicate values
            tcam_vms = stage.stg_create_tbl_profile_tcam_val_mask(prof_val)
            # XXX for now max one tcam entry per prof_val is supported
            if len(tcam_vms) == 0:
                stage.gtm.tm.logger.warning( \
                        "%s:Stage %d: Table profile TCAM: profile_val %d Skip Invalid condition" % \
                        (stage.gtm.d.name, stage.id, prof_val))
                continue
            assert len(tcam_vms) == 1, pdb.set_trace()
            (val, mask) = tcam_vms[0]
            if (val, mask) in tcam_entries:
                if ctg != tcam_entries[(val,mask)]:
                    stage.gtm.tm.logger.warning( \
                        "%s:Stage %d:Table profile TCAM: Skip entry (0x%x, 0x%x) : %d, %s" % \
                        (stage.gtm.d.name, stage.id, val, mask, prof_val, ctg))
                continue

            tcam_entries[(val, mask)] = ctg
            te = json_regs['cap_te_csr_cfg_table_profile_cam[%d]' % prof_idx]
            te['valid']['value'] = str(1)
            te['value']['value'] = "0x%x" % val
            te['mask']['value'] = "0x%x" % mask

        te = json_regs['cap_te_csr_cfg_table_profile_cam[%d]' % prof_idx] # for printing
        # do use overflow tcam when programming
        active_ctg = [act for act in stage.table_profiles[prof_val] if not act.is_otcam]

        json_tbl_prof = json_regs['cap_te_csr_cfg_table_profile[%d]' % prof_idx]
        json_tbl_prof['mpu_results']['value'] = str(len(active_ctg))

        stage.gtm.tm.logger.debug( \
            "%s:Stage %d:Table profile TCAM[%d]:(val %s, mask %s): prof_val %d, %s, mpu_res %d" % \
                (stage.gtm.d.name, stage.id, prof_idx, te['value']['value'], 
                te['mask']['value'], prof_val, active_ctg, len(active_ctg)))

        # h/w allows a flexible partitioning of the ctrl_sram entries per profile
        # for now keep it fixed allocation of 12 cycles per profile
        sidx_base = (prof_idx * num_cycles)
        json_tbl_prof['seq_base']['value'] = str(sidx_base)

        flit_kms = [[] for _ in range(num_flits)]
        for ct in active_ctg:
            assert not ct.is_otcam
            for _km in ct.key_makers:
                if _km.shared_km:
                    km = _km.shared_km
                else:
                    km = _km
                for fid in km.flits_used:
                    if km not in flit_kms[fid]:
                        flit_kms[fid].append(km)
        fid = 0
        cyc_done = False
        for cyc in range(num_cycles):
            # fill control_sram entry
            sidx = sidx_base + cyc
            se = json_regs['cap_te_csr_dhs_table_profile_ctrl_sram_entry[%d]' % sidx]

            if not stage.table_sequencer[prof_val][cyc].is_used:
                # no need to do much, key-maker values can be modified as they are not used anymore
                # XXX can set new_key = 1 for all key maker and keep resetting it if needed
                pass
            else:
                assert fid <= num_flits, pdb.set_trace()    # error in advancing flits
                for km in flit_kms[fid]:
                    km_prof = km.combined_profile
                    kmid = km.hw_id
                    if not km_prof:
                        continue # key-less tables
                    assert se['km_mode%d' % kmid]['value'] == '-1', pdb.set_trace()
                    se['km_mode%d' % kmid]['value'] = str(km_prof.mode)
                    se['km_profile%d' % kmid]['value'] = str(km_prof.hw_id)
                    if fid == km.flits_used[0]:
                        se['km_new_key%d' % kmid]['value'] = str(1)
                    else:
                        se['km_new_key%d' % kmid]['value'] = str(0)

            ct = stage.table_sequencer[prof_val][cyc].tbl
            if ct:
                assert se['tableid']['value'] == '-1', pdb.set_trace()
                se['tableid']['value'] = str(ct.tbl_id)
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
                    assert 0, pdb.set_trace()
            else:
                se['lkup']['value'] = te_consts['no_op']

            # fill sram_extention
            json_sram_ext = json_regs['cap_te_csr_cfg_table_profile_ctrl_sram_ext[%d]' % sidx]
            if stage.table_sequencer[prof_val][cyc].adv_flit:
                json_sram_ext['adv_phv_flit']['value'] = str(1)
                fid += 1
            else:
                json_sram_ext['adv_phv_flit']['value'] = str(0)

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

        prof_idx += 1

    for ct in stage.ct_list:
        if ct.is_otcam:
            continue
        json_tbl_ = json_regs['cap_te_csr_cfg_table_property[%d]' % ct.tbl_id]
        # update to asic doc - axi=1 => SRAM
        if ct.is_hbm:
            json_tbl_['axi']['value'] = str(0)
        else:
            json_tbl_['axi']['value'] = str(1)

        # key mask programming - 
        # hw bit numbering is 511:0 - little endian like
        # which is opposite on ncc ordering
        # ncc creates end_key_off such that it points to a bit after the key
        # if only one km is used, it is at msb (??) so still adjust it using 512 bit
        # hardware expects hi, lo mask as -
        # (hi, lo] => hi=512 means we need bit 511, lo=496 means we need bit 496
        # XXX hw does not have enough bits to store 512 - will be fixed soon
        key_mask_hi = 512 - ct.start_key_off
        key_mask_lo = 512 - ct.end_key_off

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
            json_tbl_['addr_sz']['value'] = str(ct.key_size)
        elif ct.is_hash_table():
            lg2_size = log2size(ct.num_entries)
            json_tbl_['addr_sz']['value'] = str(lg2_size)
            

        # XXX Check w/ asic km0=>lo, km1=>hi (from te.cc)
        # ncc uses km0 as high key and km1 as lo key bytes, so flip it
        k = 1
        for _km in ct.key_makers:
            if _km.shared_km:
                km = _km.shared_km
            else:
                km = _km
            
            json_tbl_['fullkey_km_sel%d' % k]['value'] = str(km.hw_id)
            k -= 1

        if ct.is_writeback:
            json_tbl_['lock_en']['value'] = str(1)
        else:
            json_tbl_['lock_en']['value'] = str(0)

        if ct.num_actions() > 1:
            json_tbl_['mpu_pc_dyn']['value'] = str(1)
        else:
            json_tbl_['mpu_pc_dyn']['value'] = str(0)
        # set a fixed value for model testing XXX
        json_tbl_['mpu_pc']['value'] = '0x2FEED00'

        if ct.is_raw:
            json_tbl_['mpu_pc_raw']['value'] = str(1)
        else:
            json_tbl_['mpu_pc_raw']['value'] = str(0)
        json_tbl_['mpu_pc_ofst_err']['value'] = str(0)
        json_tbl_['mpu_vec']['value'] = '0xF'    # all mpus for scheduling
        json_tbl_['addr_base']['value'] = str(0)
        json_tbl_['addr_vf_id_en']['value'] = str(0)
        json_tbl_['addr_vf_id_loc']['value'] = str(0)

        entry_size = ct.d_size
        if ct.is_hash_table():
            entry_size += ct.key_phv_size
        entry_sizeB = (entry_size + 7) / 8   # convert to bytes

        if ct.is_hbm and not ct.is_raw and not ct.is_raw_index:
            lg2entry_size = log2size(entry_sizeB)
            json_tbl_['addr_shift']['value'] = str(lg2entry_size)
            json_tbl_['lg2_entry_size']['value'] = str(lg2entry_size)
        elif ct.is_raw_index:
            # special handling, don't shift addr, but read entry_size bytes
            lg2entry_size = log2size(entry_sizeB)
            json_tbl_['addr_shift']['value'] = str(0)
            json_tbl_['lg2_entry_size']['value'] = str(lg2entry_size)
        else:
            json_tbl_['addr_shift']['value'] = str(0)
            json_tbl_['lg2_entry_size']['value'] = str(0)

        json_tbl_['chain_shift']['value'] = str(0)
        stage.gtm.tm.logger.debug("%s:Stage[%d]:Table %s:cap_te_csr_cfg_table_property[%d]:\n%s" % \
            (stage.gtm.d.name, stage.id, ct.p4_table.name, ct.tbl_id,
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

def capri_dump_registers(cfg_out_dir, prog_name, cap_mod, cap_inst, regs, mems):
    if cfg_out_dir is None or cap_mod is None or prog_name is None:
        return
    if not os.path.exists(cfg_out_dir):
        os.makedirs(cfg_out_dir)
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
        width = 0
        skip = True

        if 'decoder' in conf:
            skip = False
            m = re.search('(\w+)_entry\[(\d+)\]', name)
            decoder = m.group(1)
            idx = int(m.group(2))
            if idx > (len(mems[decoder]['entries']) - 1):
                pass
            else:
                result = [data, 0]
                _decode_mem(mems[decoder]['entries'][int(idx)], result)
                data = result[0]

            # find the correct width of the entry
            for field, attrib in conf.iteritems():
                if ((field == 'word_size') or (field == 'inst_name') or
                    (field == 'addr_offset') or (field == 'decoder') or
                    (field == 'is_array')):
                    continue
                lsb  = int(attrib['field_lsb'])
                msb  = int(attrib['field_msb'])
                width += msb - lsb + 1
        else:
            for field, attrib in conf.iteritems():
                if ((field == 'word_size') or (field == 'inst_name') or
                    (field == 'addr_offset') or (field == 'decoder') or
                    (field == 'is_array')):
                    continue
                lsb  = int(attrib['field_lsb'])
                msb  = int(attrib['field_msb'])
                mask = int(attrib['field_mask'], 16)
                val  = int(attrib['value'], 0)
                if val == -1:
                    val = 0
                else:
                    skip = False
                width += msb - lsb + 1
#               data = data | ((val & mask) << lsb)
                data = data | (val << lsb)
        if skip == True:
            continue

        while width > 0:
#           print '0x%08x 0x%08x %s\n' % \
#                       (addr_offset, data & 0xFFFFFFFF, name)
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

def capri_table_memory_spec_load():
    cur_path = os.path.abspath(__file__)
    cur_path = os.path.split(cur_path)[0]
    spec_file_path = os.path.join(cur_path, 'csr_json/cap_memory_spec.json')
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
        os.makedirs(out_dir)

    for mem_type in out_pic:
        for direction in out_pic[mem_type]:
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
        os.makedirs(out_dir)

    out_file = out_dir + 'capri_p4_table_spec.json'
    with open(out_file, "w") as of:
        json.dump(out_dict, of, indent=2, sort_keys=True)
        of.close()

def capri_p4_table_map_output(be, map_dict):

    out_dir = be.args.gen_dir + '/%s/p4pd/' % (be.prog_name)

    if not os.path.exists(out_dir):
        os.makedirs(out_dir)

    out_file = out_dir + 'capri_p4_table_map.json'
    with open(out_file, "w") as of:
        json.dump(map_dict, of, indent=2, sort_keys=True)
        of.close()

def capri_dump_table_memory(be, memory, tables, mem_type, region):

    out_dir = be.args.gen_dir + '/%s/p4pd/' % (be.prog_name)

    if not os.path.exists(out_dir):
        os.makedirs(out_dir)

    out_file = out_dir + 'capri_' + region + '_p4_' + mem_type + '_dump.txt'
    with open(out_file, "w") as of:
        free  = 0
        total = 0
        for row in range(memory[mem_type][region]['depth']):
            if row != 0 and (row % memory[mem_type][region]['blk_d']) == 0:
                of.write('\n')
            for col in range(memory[mem_type][region]['width']):
                if col != 0 and (col % memory[mem_type][region]['blk_w']) == 0:
                    of.write(' ')
                value = memory[mem_type][region]['space'][row][col]
                if value == 0:
                    free += 1
                total += 1
                of.write('%c' % value if value else '0')
            of.write('\n')

        of.write('%d cells free out of %d (%d%% utilization).\n' % (free, total, ((total - free) * 100 / (total if total else 1))))
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

            if width == 1 and depth == 1:
                layout_string = 'width: None, depth: None'
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

def capri_p4pd_cli_create_makefile(be):

    out_dir = be.args.gen_dir + '/%s/cli/' % (be.prog_name)

    if not os.path.exists(out_dir):
        os.makedirs(out_dir)

    content_str = '# This Makefile is auto-generated. Changes will be overwritten!\n'
    content_str += 'CXX = g++\n'
    content_str += 'SWG = swig\n'
    content_str += 'SWGFLAGS = -c++ -python\n'
    content_str += 'CPPFLAGS = -g -c -pthread -std=c++11 -fPIC\n'
    content_str += 'LNKFLAGS = -Wl,-rpath,../../../../../../obj -shared\n'
    content_str += 'INC_DIRS = -I.\n'
    content_str += 'INC_DIRS += -I../include\n'
    content_str += 'INC_DIRS += -I../../../../../../hal/pd/\n'
    content_str += 'INC_DIRS += -I/usr/local/include -I/usr/include/boost/\n'
    content_str += 'INC_DIRS += -I/usr/include/python2.7 -I/usr/include/x86_64-linux-gnu/python2.7\n'
    content_str += 'LIB_DIRS = -L/usr/local/lib -L../../../../../../obj -L/usr/lib/python2.7/dist-packages -L.\n'
    content_str += 'LIBS = -lpython2.7 -lcapri -lcaprimrl -ltrace\n'
    content_str += '\n'
    content_str += 'all:\n'
    content_str += '\t$(SWG) $(SWGFLAGS) -o p4pd_wrap.c p4pd.i\n'
    content_str += '\t$(CXX) $(CPPFLAGS) $(INC_DIRS) -o p4pd.o ../src/p4pd.c\n'
    content_str += '\t$(CXX) $(CPPFLAGS) $(INC_DIRS) -o stub.o stub.cc\n'
    content_str += '\t$(CXX) $(CPPFLAGS) $(INC_DIRS) -o p4pd_wrap.o p4pd_wrap.c\n'
    content_str += '\t$(CXX) $(LNKFLAGS) -o _p4pd.so *.o $(LIB_DIRS) $(LIBS)\n'
    content_str += 'clean:\n'
    content_str += '\trm *.o *.so p4pd.py p4pd_wrap.c\n'

    out_file = out_dir + 'Makefile'
    with open(out_file, "w") as of:
        of.write(content_str)
        of.close()

def capri_p4pd_cli_create_swig_interface(be):

    out_dir = be.args.gen_dir + '/%s/cli/' % (be.prog_name)

    if not os.path.exists(out_dir):
        os.makedirs(out_dir)

    content_str = '/* This file is auto-generated. Changes will be overwritten! */\n'
    content_str += '/* p4pd.i */\n'
    content_str += '%module p4pd\n'
    content_str += '%{\n'
    content_str += '\t#include "p4pd.h"\n'
    content_str += '\ttypedef int p4pd_error_t;\n'
    content_str += '\tp4pd_error_t p4pd_entry_read(uint32_t tableid, uint32_t index, void *swkey, void *swkey_mask, void *actiondata);\n'
    content_str += '%}\n'
    content_str += '\n'
    content_str += 'typedef int p4pd_error_t;\n'
    content_str += 'p4pd_error_t p4pd_entry_read(uint32_t tableid, uint32_t index, void *swkey, void *swkey_mask, void *actiondata);\n'

    out_file = out_dir + 'p4pd.i'
    with open(out_file, "w") as of:
        of.write(content_str)
        of.close()

def capri_p4pd_cli_create_stub_file(be):

    out_dir = be.args.gen_dir + '/%s/cli/' % (be.prog_name)

    if not os.path.exists(out_dir):
        os.makedirs(out_dir)

    content_str = '/* This file is auto-generated. Changes will be overwritten! */\n'
    content_str += '#include <stdint.h>\n'
    content_str += '#include <string>\n'
    content_str += '#include <iostream>\n'
    content_str += '#include <thread>\n'
    content_str += '#include "p4pd.h"\n'
    content_str += '#include "p4pd_api.hpp"\n'
    content_str += '\n'
    content_str += 'std::thread *g_hal_threads[100];\n'
    content_str += '\n'
    content_str += 'namespace hal {\n'
    content_str += '    thread_local std::thread *t_curr_thread;\n'
    content_str += '}\n'
    content_str += '\n'
    content_str += 'typedef int p4pd_error_t;\n'
    content_str += 'p4pd_error_t p4pd_table_properties_get(uint32_t                       tableid,\n'
    content_str += '                                                                      p4pd_table_properties_t       *tbl_ctx)\n'
    content_str += '{\n'
    content_str += '    return 0;\n'
    content_str += '}\n'

    out_file = out_dir + 'stub.cc'
    with open(out_file, "w") as of:
        of.write(content_str)
        of.close()


def capri_p4pd_cli_create_shell(be):

    out_dir = be.args.gen_dir + '/%s/cli/' % (be.prog_name)

    if not os.path.exists(out_dir):
        os.makedirs(out_dir)

    content_str  = '#!/usr/bin/python\n'
    content_str += '# This file is auto-generated. Changes will be overwritten! \n'
    content_str += '#\n'
    content_str += '# p4pd CLI\n'
    content_str += '#\n'
    content_str += 'from cmd2 import Cmd\n'
    content_str += 'import p4pd\n'
    content_str += '\n'
    content_str += 'class level1Cmd(Cmd):\n'
    content_str += '    prompt = "p4pd/level1> "\n'
    content_str += '\n'
    content_str += '    def __init__(self):\n'
    content_str += '        Cmd.__init__(self)\n'
    content_str += '\n'
    content_str += '    def do_main(self, args):\n'
    content_str += '        """Calls p4pd unit test."""\n'
    content_str += '        p4pd.main()\n'
    content_str += '\n'
    content_str += '    def do_back(self, args):\n'
    content_str += '        """Go back one level."""\n'
    content_str += '        return True\n'
    content_str += '\n'
    content_str += '    do_EOF = do_back\n'
    content_str += '\n'
    content_str += 'class rootCmd(Cmd):\n'
    content_str += '\n'
    content_str += '    prompt = "p4pd> "\n'
    content_str += '    intro = "This is a debug CLI. Theres no protection against bad arguments!"\n'
    content_str += '\n'
    content_str += '    def __init__(self):\n'
    content_str += '        Cmd.__init__(self)\n'
    content_str += '\n'
    content_str += '    def do_level1(self, args):\n'
    content_str += '        """Go to level1."""\n'
    content_str += '        cmd = level1Cmd()\n'
    content_str += '        cmd.cmdloop()\n'
    content_str += '\n'
    content_str += 'if __name__ == \'__main__\':\n'
    content_str += '    cmd = rootCmd()\n'
    content_str += '    cmd.cmdloop()\n'

    out_file = out_dir + 'cli.py'
    with open(out_file, "w") as of:
        of.write(content_str)
        of.close()
