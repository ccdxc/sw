#!/usr/bin/python
#
# Capri-Non-Compiler-Compiler (capri-ncc)
# Parag Bhide (Pensando Systems)

import os
import sys
import pdb
import logging
import copy
from collections import OrderedDict
from enum import IntEnum

from p4_hlir.main import HLIR
import p4_hlir.hlir.p4 as p4

from capri_logging import ncc_assert as ncc_assert

from capri_utils import *

from capri_model import capri_model as capri_model
from capri_output import capri_parser_logical_output as capri_parser_logical_output
from capri_output import capri_parser_output_decoders as capri_parser_output_decoders
from capri_output import elba_parser_output_decoders as elba_parser_output_decoders
from capri_pa import capri_field as capri_field

def _get_p4_headers(s):
    if not isinstance(s, p4.p4_parse_state):
        return []
    return [call[1] for call in s.call_sequence if call[0] == p4.parse_call.extract]

def _get_p4_virtual_headers(s):
    vhdrs = []
    hdrs = _get_p4_headers(s)
    for h in hdrs:
        if h.virtual:
            vhdrs.append(h)
    return vhdrs

def _get_header_order_node_list(parser, node, node_list):
    # TBD (see recent fix in hlir for this) - done it here in a different way
    # need to do this as egress header order is not the same as specified
    hdr_order = get_pragma_param_list(node.p4_state._parsed_pragmas['header_ordering'])
    p4h_hdr_order = []
    ordered_node_list = []
    for h in hdr_order:
        if h not in parser.be.h.p4_header_instances:
            continue
        p4h = parser.be.h.p4_header_instances[h]
        for cs in node_list:
            if not isinstance(cs, capri_parse_state):
                continue
            if _find_header(node, cs, p4h):
                if cs not in ordered_node_list:
                    ordered_node_list.insert(0,cs)

    return ordered_node_list

def _resolve_len_expr(parser, hdr, l_exp, hlen_fld_name, hlen_dummy):
    # change all variable names to corresponding p4_field objects
    if isinstance(l_exp.left, str):
        if l_exp.left == hlen_dummy:
            l_exp.left = parser.be.h.p4_fields[hlen_fld_name]
        else:
            l_exp.left = parser.be.h.p4_fields[hdr.name+'.'+l_exp.left]
    if isinstance(l_exp.right, str):
        if l_exp.right == hlen_dummy:
            l_exp.right = parser.be.h.p4_fields[hlen_fld_name]
        else:
            l_exp.right = parser.be.h.p4_fields[hdr.name+'.'+l_exp.right]
    if isinstance(l_exp.left, p4.p4_expression):
        _resolve_len_expr(parser, hdr, l_exp.left, hlen_fld_name, hlen_dummy)
    if isinstance(l_exp.right, p4.p4_expression):
        _resolve_len_expr(parser, hdr, l_exp.right, hlen_fld_name, hlen_dummy)

def _topo_sort_headers(parser, node, sorted_list, marker):
    if node in marker:
        if marker[node] == 1:
            parser.logger.debug("Loop at %s" % (node.name()))
            ncc_assert(0);
            return True #cycles
        if marker[node] == 2:
            return False
    else:
        marker[node] = 1
        for next_node in node.branches:
            if _topo_sort_headers(parser, next_node, sorted_list, marker):
                return True
        #print "Confirm node %s" % node.name
        marker[node] = 2
        sorted_list.insert(0, node)
        return False


# Only two types of loops are supported -
# 1. loops with header stack (mpls) where only one header stack is supported per loop
#       new state is created for each header instance in a stack
# 2. loops without header stack (tcp options) with header_ordering pragma
# XXX what is really needed is topological header ordering (not just states)
# and un-rolling of any loops to handle header-stacks
# we can support only one header-stack per loop
def _parser_topo_sort(parser, d, parse_states, node, marker, sorted_list, depth=0):
    # Toposort algorithm from wikipedia
    #print "Check node %s" % str(node)
    if node in marker:
        if marker[node] == 1:
            #print "Cyclic visit node %s" % (str(node))
            return True #cycles
        if marker[node] == 2:
            return False
    else:
        marker[node] = 1
        node_list = node.branch_to.values()
        if node.p4_state and 'header_ordering' in node.p4_state._parsed_pragmas:
            # handle header ordering pragma to enforce the specified order
            # create a state list in the reverse order so that topo-sort will
            # build the header order correctly by visiting last header state first
            extracted_hdrs = parser._prune_next_states(node, node_list)
            looping_branches = [nxt for nxt in node.branch_to.values() if nxt not in node_list]
            traversed_nodes = parser._traverse_loops(node, looping_branches)
            ordered_node_list = _get_header_order_node_list(parser, node, traversed_nodes)
            node_list += ordered_node_list

        for next_node in node_list:
            if not next_node:
                continue
            if not isinstance(next_node, capri_parse_state):
                continue
            if next_node.p4_state and 'xgress' in next_node.p4_state._parsed_pragmas:
                if next_node.p4_state._parsed_pragmas['xgress'].keys()[0].upper() != d.name:
                    continue # skip - specified only for parser in other direction
            if _parser_topo_sort(parser, d, parse_states, next_node, marker, sorted_list, depth+1):
                if 'header_ordering' in next_node.p4_state._parsed_pragmas:
                    continue # allow looping
                vhdr = _get_p4_virtual_headers(next_node.p4_state)
                if len(vhdr) != 0:
                    continue
                #print "Cyclic visit node %s->%s - %s\n%s" % \
                #            (str(node), str(next_node), node_list, sorted_list)
                return True

        #print "Confirm node %s" % node.name
        marker[node] = 2
        if depth > node.depth:
            node.depth = depth
        sorted_list.insert(0, node)

        return False

def _find_header(parent, _state, hdr):
    # This function is used at differnet times, it can be called with parent and _state as
    # p4_parse_state objects or capri_parse_state objects
    state = _state
    if not isinstance(state, capri_parse_state):
        if not isinstance(state, p4.p4_parse_state):
            return False
    else:
        state = _state.p4_state

    if isinstance(parent, capri_parse_state):
        parent = parent.p4_state

    if state == parent:
        return False
    if hdr in _get_p4_headers(state):
        return True
    if 'header_ordering' in state._parsed_pragmas:
        # terminate search if we find another pragma.. Need to find a better soln
        return False
    #print "looking for %s in state %s" % (hdr.name, state.name)
    for nxt in state.branch_to.values():
        if _find_header(parent, nxt, hdr):
            return True
    return False

class capri_lkp_fld:
    def __init__(self, cf, src, offset=-1, l=-1):
        self.src_type = src     # LKP_PKT | LOCAL | SAVED_REG
        self.cf = cf            # when src_type is LKP_PKT
        self.set_op = None      # p4_expr when src_type is LOAD/UPDATE_REG
        self.is_key = False
        self.store_en = False   # per register

        self.pkt_off = offset   # when loading from pkt
        self.len = l
        self.key_off = -1       # offset of this field within the key 0=msb
        self.reg_id = -1        # lkp_reg allocated to this field

        self.hfname = cf.hfname if cf else '__current_x_y'

    def __repr__(self):
        return "\n%s %s pkt_off %d len %d key_off %d" % \
            (self.hfname, self.src_type.name, self.pkt_off, self.len, self.key_off)

class capri_parser_set_op:
    def __init__(self, cstate, dst, src):
        self.cstate = cstate
        self.op_type = None # LOAD/UPDATE REG, EXTRACT_FIELD/REG
        self.dst = dst      # capri field
        # if src is not a p4_expression, derive the required information directly
        # else the same information is obtained from capri_expr
        self.src1 = None    # capri field
        self.src_reg = None
        self.const = 0
        self.capri_expr = None
        # hw info
        self.rid = None # store rid allocated to this operation - required for register programming

        # dst is a capri_field, could be parser_only or in phv
        ncc_assert(dst.is_meta, "%s: %s must be metadata field" % \
            (self.cstate.name, dst.hfname))

        if isinstance(src, int):
            # loading a const into register(local_var) has to be done indirectly
            # using a expression - cookup a simple expr for this
            if dst.parser_local:
                # reg <-  const : Load
                self.op_type = meta_op.LOAD_REG
                self.capri_expr = capri_parser_expr(cstate.parser, None)
                # op1 == None indicates a dummy load from pkt and masked off to 0
                self.capri_expr.op3 = '+'
                self.capri_expr.const = src
            else:
                # metaPhv <- const
                self.op_type = meta_op.EXTRACT_CONST
                self.const = src
                if (dst.width % 8):
                    ncc_assert(dst.is_meta)
                    if (dst.width % 8) < 4:
                        # don;t pad below 4 bits.
                        cstate.parser.logger.debug("%s:%s:set_metadata to %s will use OR instn" % \
                            (cstate.parser.d.name, cstate.name, dst.hfname))
                    else:
                        # padding flds >= 4b seems to help key maker sharing for ???
                        pad = 8-(dst.width % 8)
                        dst.width += pad
                        cstate.parser.logger.warning("%s:%s:Pad Metadata %s to %d" % \
                                (cstate.parser.d.name, cstate.name, dst.hfname, dst.width))
                        cstate.parser.logger.debug("%s:%s:set_metadata to %s will use OR instn" % \
                            (cstate.parser.d.name, cstate.name, dst.hfname))

            return

        if isinstance(src, p4.p4_field):
            hf_name = get_hfname(src)
            cf = cstate.parser.be.pa.get_field(hf_name, cstate.parser.d)
            if cf.is_meta:
                # phv <- reg : must use expression to address lkp_reg
                ncc_assert(cf.parser_local, "%s:Unsupported source %s for set_metadata" % \
                    (self.cstate.parser.d.name, cf.hfname))
                ncc_assert(not dst.parser_local, "%s:Unsupported local reg dst %s for set_metadata" % \
                    (self.cstate.parser.d.name, dst.hfname))
                self.op_type = meta_op.EXTRACT_REG
                self.src_reg = cf
                self.capri_expr = capri_parser_expr(cstate.parser, None)
                self.capri_expr.op2 = '+'
                self.const = 0
                self.capri_expr.src_reg = cf
            else:
                if dst.parser_local:
                    # reg <- pkt_field: Load
                    self.op_type = meta_op.LOAD_REG
                    # byte alignment check for cf
                    # fields loaded in reg are expected to be right justified in the register
                    # mask is computed by output genertion code
                    src_boff = cf.p4_fld.offset % 8
                    src_eoff = (src_boff + cf.width) % 16
                    if src_eoff:
                        self.capri_expr = capri_parser_expr(cstate.parser, None)
                        self.capri_expr.op1 = '>>'
                        self.capri_expr.shft = 0    # output function handles this 16 - src_eoff
                        self.capri_expr.src1 = cf
                else:
                    # metaPhv <- pkt_field : ExtractCopy
                    # check byte alignment and size - alignment matters when it is assigned a
                    # phv
                    # Since parser can extract on byte boundary from packet to phv, pkt_fld is
                    # checked for byte alignment. Any bits on both sides of the field are added
                    # to the field's width and dst width is set to that.
                    # E.g. if a fld occupies bits 3-11 (w=9), the dst width is set to 16. (9+3+4)
                    # If dst.width > new_width, it is only aligned to byte boundary. This is
                    # needed when larger containers are used. (e.g. lkp_src)
                    self.op_type = meta_op.EXTRACT_FIELD
                    src_boff = cf.p4_fld.offset % 8
                    src_eoff = (cf.p4_fld.offset+cf.width) % 8
                    # increase the dst field's width by all misaligned amounts on both sides
                    # XXX when eoff is not byte aligned, mux_inst must be used for extraction
                    # using meta_instruction mechanism. Currently this mechanism is used only
                    # when there is a P4 expression used to compute the src, for now "force"
                    # the programmer to create a dummy expression
                    if src_eoff:
                        cstate.parser.logger.critical("%s:Misaligned extraction of %s" % \
                            (cstate.name, cf.hfname))
                        cstate.parser.logger.critical("Use an expression like %s + 0" % \
                            (cf.hfname))
                        ncc_assert(0, "Un-supported metadata extraction")
                    if src_eoff:
                        dst_w = cf.width + (src_boff + 8 - src_eoff)
                    else:
                        dst_w = cf.width + src_boff

                    if dst.width < dst_w:
                        dst.pad = (dst_w - dst.width)
                        dst.width = dst_w
                        cstate.parser.logger.warning("%s:%s:Pad Metadata %s to %d" % \
                            (cstate.parser.d.name, cstate.name, dst.hfname, dst.width))

                    if dst.width % 8:
                        dst.pad = (8-(dst.width % 8))
                        dst.width += (8-(dst.width % 8))
                        cstate.parser.logger.warning("%s:%s:Pad Metadata %s to %d" % \
                            (cstate.parser.d.name, cstate.name, dst.hfname, dst.width))

                self.src1 = cf
        elif isinstance(src, tuple):
            # current(0,0) is used to indicate current_offset itself is loaded into reg or phv
            # LOAD_REG always uses mux_idx instruction which allows for loading current_offset
            # In case of extracting current_offset to phv, create a new op_type
            if dst.parser_local:
                # reg <- (pkt_off, size) : Load
                self.op_type = meta_op.LOAD_REG
            else:
                if src[1] == 0:
                    # extract current offset
                    pdb.set_trace() # test case
                    self.op_type = meta_op.EXTRACT_CURRENT_OFF
                # metaPhv <- (pkt_off, size) : ExtractCopy
                elif (dst.width % 8):
                    ncc_assert(dst.is_meta)
                    pad = 8-(dst.width % 8)
                    dst.width += pad
                    cstate.parser.logger.warning("%s:%s:Pad Metadata %s to %d" % \
                            (cstate.parser.d.name, cstate.name, dst.hfname, dst.width))
                    self.op_type = meta_op.EXTRACT_FIELD
                else:
                    pass
            self.src1 = src
        elif isinstance(src, p4.p4_expression):
            # src is p4_expr
            self.capri_expr = capri_parser_expr(cstate.parser, src)
            if dst.parser_local:
                self.op_type = meta_op.LOAD_REG
                if self.capri_expr.src_reg and self.capri_expr.src_reg == dst:
                    self.op_type = meta_op.UPDATE_REG
                elif self.capri_expr.src1 and self.capri_expr.src1 == dst:
                    self.op_type = meta_op.UPDATE_REG
                elif self.capri_expr.const:
                    self.op_type = meta_op.LOAD_REG
                else:
                    #ncc_assert(0)
                    pass
            else:
                # EXTRACT_REG uses meta_instruction to perform the operation
                # Also any time an expression is involved, use EXTRACT_META
                # to indicate that meta_inst must be used
                if (dst.width % 8):
                    ncc_assert(dst.is_meta)
                    pad = 8-(dst.width % 8)
                    dst.width += pad
                    cstate.parser.logger.warning("%s:%s:Pad Metadata %s to %d" % \
                            (cstate.parser.d.name, cstate.name, dst.hfname, dst.width))
                if self.capri_expr.src_reg:
                    self.op_type = meta_op.EXTRACT_REG
                    self.src_reg = self.capri_expr.src_reg
                else:
                    if isinstance(self.capri_expr.src1, tuple) and self.capri_expr.src1[1] == 0:
                        self.op_type = meta_op.EXTRACT_CURRENT_OFF
                    else:
                        self.op_type = meta_op.EXTRACT_META
                        ncc_assert((self.capri_expr.src1), "Need pkt data oprand for set_metadata()")
                    self.src1 = self.capri_expr.src1
        else:
            ncc_assert(0, "unknown source for set_metadata operation %s" % src)

        if self.op_type == meta_op.LOAD_REG:
            ncc_assert(self.dst.parser_local, \
                "Field %s must be defined as parser_local" % (self.dst.hfname))

    def __repr__(self):
        pstr =''
        pstr += '%s: Op=%s, dst=%s src_reg=%s src1=%s const=%d\ncapri_expr %s' % \
            (self.cstate.name, self.op_type.name, self.dst.hfname, \
             self.src_reg.hfname if self.src_reg else None,
             self.src1.hfname if self.src1 and isinstance(self.src1, capri_field) else self.src1,
             self.const, self.capri_expr)
        return pstr

class capri_branch_info:
    def __init__(self):
        self.val = 0
        self.mask = None
        self.nxt_state = None

class capri_lkp_reg:
    def __init__(self, sz):
        self.type = lkp_reg_type.LKP_REG_NONE
        self.first_pkt_fld = None   # First pkt field that is loaded into this reg
        self.flds = OrderedDict()   # flds in this register {reg_off : (lkp_fld, off, used)}
        self.pkt_off = -1
        self.pkt_off2 = -1
        self.store_en = False
        self.capri_expr = None
        # book-keeping
        self.avail = sz             # tracking variables when packing pkt flds
        self.size = sz
        self.is_key = False         # True if register is used for lookup

    def reset(self):
        self.type = lkp_reg_type.LKP_REG_NONE
        self.first_pkt_fld = None   # First pkt field that is loaded into this reg
        self.flds = OrderedDict()   # flds in this register {reg_off : (lkp_fld, off, used)}
        self.pkt_off = -1
        self.store_en = False
        self.capri_expr = None
        # book-keeping
        self.avail = self.size
        self.is_key = False         # True if register is used for lookup

    def __repr__(self):
        return "\n%s pkt_off %d, pkt_off2 %d, is_key %s, store_en %s, expr %s, first_pkt_fld: %s flds: %s" % \
                (self.type.name, self.pkt_off, self.pkt_off2, self.is_key, self.store_en, self.capri_expr,
                 self.first_pkt_fld.hfname if self.first_pkt_fld else None,
                 [(f[0],f[1][0].hfname) for f in self.flds.items()])

class capri_parse_state:
    def __init__(self, parser, p4_state, name=None):
        self.id = -1
        self.parser = parser
        self.p4_state = p4_state
        self.depth = 0
        # copy info from p4 state that may be modified later (merge/split/unroll)
        self.name = p4_state.name[:] if p4_state else name
        self.branch_on = [] # p4_state.branch_on[:] if p4_state else []
        self.branch_to = OrderedDict()
        self.headers = []
        self.extracted_fields = []      # all header fields extracted, can go to phv or ohi
        self.set_ops = []
        self.no_reg_set_ops = []        # HACK XXX revisit
        self.local_flds = OrderedDict()
        self.meta_extracted_fields = [] # extractions to metadata phvs
        # utility flags/variables
        self.unrolled_state = False
        self.unrolled_idx = 0
        self.internal_state = False if p4_state else True
        self.is_virtual = False
        self.is_end = False
        self.is_hw_start = False
        self.is_split = False
        self.variable_hdr = False
        self.deparse_only = False

        self.fld_off = OrderedDict()    # pkt offset(bit) of each fld relative to this state
        self.extract_len = -1
        self.max_extract_len = -1
        self.lkp_flds = OrderedDict()   # {cf.hfname : list of capri_lkp_fld objects}
        self.reserved_lfs = []
        self.load_saved_lkp = []        # load lkp save register for future use
        self.saved_lkp = []             # lkp registers reserved
        self.key_len = 0
        self.branches = []              # processed branch information - maintain the order

        self.active_lkp_lfs = [None for _ in self.parser.be.hw_model['parser']['lkp_regs']]
        self.len_chk_profiles = []
        self.payload_offset_expr = None

        # hardware information
        self.lkp_regs = [capri_lkp_reg(v) for v in self.parser.be.hw_model['parser']['lkp_regs']]
        self.hw_lkp_size = sum([v for v in self.parser.be.hw_model['parser']['lkp_regs']])
        self.key_l2p_map = None

        # Checksum / Calculated Field List related values stored in parse-state
        self.csum_payloadlen_ohi_instr_gen = (False, -1, None)
                                                #When true, in the parse-state
                                                #also generate OHI instruction
                                                #to load OHI slot
                                                #(totalLen_ohi_id) with
                                                #l4_verify_len value
        self.verify_cal_field_objs = [] # List of objects (At most 2 supported
                                        # in a parse state)
        self.enable_udp_zero_csum_error = 0
        self.phdr_offset_ohi_id = -1    # In case of phdr capture IP hdr offset
                                        # start in this ohi_sel.
        self.totalLen_ohi_id = -1       # used to store ohi# where
                                        # totaLen-ihl*4 is captured or
                                        #v6.payloadLen - Sum(all v6_options)
        self.phdr_type = ''             # Used to indicate phdr is v4/v6

        # RoCE icrc related reference objs stored in parse states
        self.icrc_verify_cal_field_objs = [] #In roce_hdr state
                                           #at most 2 calfldobjs
                                           #can be present
        self.gso_csum_calfldobj = None


    def active_reg_find(self, lf):
        if lf in self.active_lkp_lfs:
            return self.active_lkp_lfs.index(lf)
        return None

    def active_reg_alloc(self, lf, rid=None):
        if rid == None:
            for i,_lf in enumerate(self.active_lkp_lfs):
                if _lf == None:
                    self.active_lkp_lfs[i] = lf
                    self.parser.logger.debug("%s:state %s:Reserve LF %s reg %d" % \
                            (self.parser.d.name, self.name, lf.hfname, i))
                    return i
            ncc_assert(0 )# out of lkp registers
        else:
            if self.active_lkp_lfs[rid] == None:
                self.active_lkp_lfs[rid] = lf
                self.parser.logger.debug("%s:state %s:Assign LF %s reg %d" % \
                            (self.parser.d.name, self.name, lf.hfname, rid))
                return rid
            else:
                ncc_assert(self.active_lkp_lfs[rid] == lf )# duplicate assignment
                return rid

    def active_reg_alloc_for_saved_lkp(self, lf):
        # just look for a register from back of the list
        # hoping that the free register is also free in all states that need to access the saved reg
        # if register is not free, then we are out of registers anyway
        for i in range(len(self.active_lkp_lfs)-1, -1, -1):
            if self.active_lkp_lfs[i] == None:
                self.active_lkp_lfs[i] = lf
                self.parser.logger.debug("%s:state %s:Assign Stored LF %s reg %d" % \
                            (self.parser.d.name, self.name, lf.hfname, i))
                return i
        ncc_assert(0, "%s:state %s:Ran out of lookup registers" %  (self.parser.d.name, self.name))
        return None

    def lkp_reg_alloc(self):
        for i,lr in enumerate(self.lkp_regs):
            if lr.type == lkp_reg_type.LKP_REG_NONE:
                return i
        self.parser.logger.critical("%s:%s:No free lkp register - regs used: %s" % \
                        (self.parser.d.name, self.name, self.lkp_regs))
        ncc_assert(0)

    def get_hdr_off(self, hdr):
        for cf in self.extracted_fields:
            h = cf.get_p4_hdr()
            if h == hdr:
                return self.fld_off[cf]
        ncc_assert(0)
        return -1

    def dont_advance_packet(self):
        if self.p4_state and 'dont_advance_packet' in self.p4_state._parsed_pragmas:
            return True
        else:
            return False

    def capture_payload_offset(self):
        if self.p4_state and 'capture_payload_offset' in self.p4_state._parsed_pragmas:
            return True
        if self.p4_state and 'dont_capture_payload_offset' in self.p4_state._parsed_pragmas:
            return False
        if self.is_end:
            return True
        return False

    def no_extract(self):
        if self.p4_state and 'no_extract' in self.p4_state._parsed_pragmas:
            return True
        else:
            return False

    def generic_checksum_start(self):
        if self.p4_state and 'generic_checksum_start' in self.p4_state._parsed_pragmas:
            return self.p4_state._parsed_pragmas['generic_checksum_start'].keys()[0]
        else:
            return None

    def create_payload_offset_expr(self):
        ncc_assert(isinstance(self.extract_len, capri_parser_expr))
        # need one of the terms free to use it for current offset
        ncc_assert(not self.extract_len.op2 or not self.extract_len.op3)
        # payload_offset = current_offset + self.extract_len
        payload_expr = copy.copy(self.extract_len)
        # [cf2 OP2] [(cf1 [OP1 shft]) OP3] [const]
        if not payload_expr.op2:
            payload_expr.op2 = '+'
            payload_expr.src_reg = (0, 0)
        elif not payload_expr.op3:
            payload_expr.op3 = '+'
            payload_expr.src1 = (0, 0)
            payload_expr.op1 = '>>'

        self.payload_offset_expr = payload_expr
        return

    def __repr__(self):
        return self.name

class capri_ohi:
    def __init__(self, start, l):
        self.start = start  # byte offset
        self.length = l # in bytes
        self.id = -1
        self.var_id = -1 # id for variable len ohi size

    def __repr__(self):
        return "(%d,%d):(%d, %s)" % (self.id, self.var_id, self.start, self.length)

class _scope:
    start_cs = None
    end_cs = None
    reg_id = -1

    def __init__(self, s, e):
        self.start_cs = s
        self.end_cs = e
        self.reg_id = -1

class _saved_lkp_reg:
    # Due to byte alignment and fix size of 16 bits in hardware -
    # - multiple fields can occupy a given register
    # - a given field may span multiple registers
    # XXX The cases above are not handled yet.. for now each field can occupy only
    # one register and one register can contain only one field XXX
    def __init__(self, cf):
        self.flds = [cf] # list of flds in this reg

    def add_fld(self, cf):
        self.flds.append(cf)

class capri_parser_expr:
        # Possible operations (supported by capri-parser hw):
        # reg = reg +/- ((src1 & mask) <</>> shft) +/- c
        # phv = reg +/- ((src1 & mask) <</>> shft) +/- c
        # New:
        # reg = src2 +/- ((src1 & mask) <</>> shft) +/- c
        # reg = reg1 +/- ((reg2 & mask) <</>> shft) +/- c (TBD)
        # The generic expression is defined as -
        # dst = src_reg OP2 (src1 OP1 shft) OP3 c
        # NOTE:
        # 1. src1 & mask is not supported as user programmble,
        #   it is computed and used internally to handle field alignements in h/w
        # 2. src1 is expected to be pkt_data (not register)
        #
    def __init__(self, parser, p4_expr):
        self.parser = parser
        self.p4_expr = p4_expr     # original expr

        self.op1 = None
        self.src1 = None    # cfield or tuple or register
        self.shft = 0

        self.op2 = None
        self.src_reg = None

        self.op3 = None
        self.const = 0

        # fields programmed later, based on usage and field alignment
        self.mask = None        # applicable for op1, user supplied mask is not supported

        if p4_expr:
            #self.process_p4_expr(copy.deepcopy(p4_expr))
            self.process_p4_expr(p4_expr)
            # op fixup
            if self.src1 and not self.op1:
                self.op1 = '<<'
                self.shft = 0
            if self.src_reg and not self.op2:
                self.op2 = self.op3
            self.parser.logger.debug("%s:Initialized parser expr %s" % (self.parser.d.name, self))

    def __copy__(self):
        new_obj = capri_parser_expr(self.parser, None)
        new_obj.p4_expr = self.p4_expr
        new_obj.op1 = copy.copy(self.op1)
        new_obj.src1 = (self.src1)
        new_obj.shft = copy.copy(self.shft)
        new_obj.op2 = copy.copy(self.op2)
        new_obj.src_reg = (self.src_reg)
        new_obj.op3 = copy.copy(self.op3)
        new_obj.const = copy.copy(self.const)
        new_obj.mask = copy.copy(self.mask)
        return new_obj

    def process_p4_expr(self, p4_expr):
        # New format:
        # [cf2 +-] [(cf1 [<<>> shft])+-] [const]
        # [cf2 OP2] [(cf1 [OP1 shft]) OP3] [const]
        supported_ops1 = ['<<', '>>']
        supported_ops2 = ['+', '-',]    # op2 and op3

        #pdb.set_trace()
        # convert p4_fields to capri_fields
        # do not modify the original p4_expr same object is used on ingress and egress by hlir
        left_cf = None; right_cf = None
        if isinstance(p4_expr.left, p4.p4_field):
            hf_name = get_hfname(p4_expr.left)
            cf = self.parser.be.pa.get_field(hf_name, self.parser.d)
            #p4_expr.left = cf
            left_cf = cf

        if isinstance(p4_expr.right, p4.p4_field):
            hf_name = get_hfname(p4_expr.right)
            cf = self.parser.be.pa.get_field(hf_name, self.parser.d)
            #p4_expr.right = cf
            right_cf = cf

        if isinstance(p4_expr.left, p4.p4_expression):
            self.process_p4_expr(p4_expr.left)
        if isinstance(p4_expr.right, p4.p4_expression):
            self.process_p4_expr(p4_expr.right)

        # case: cf/tuple OP expr (expr is already processed)
        if isinstance(p4_expr.left, p4.p4_expression):
            # if right is expr/cf/tuple
            if not isinstance(p4_expr.right, int):
                if not self.op2:
                    self.op2 = p4_expr.op
                else:
                    ncc_assert(not self.op3)
                    self.op3 = p4_expr.op
                if not isinstance(p4_expr.right, p4.p4_expression):
                    if not self.src_reg:
                        self.src_reg = right_cf if right_cf else p4_expr.right
                    else:
                        ncc_assert(not self.src1)
                        self.src1 = right_cf if right_cf else p4_expr.right
            # elif right is const
            elif isinstance(p4_expr.right, int):
                ncc_assert(not self.op3)
                self.op3 = p4_expr.op
                self.const = p4_expr.right
            else:
                ncc_assert(0)
            return
        elif isinstance(p4_expr.right, p4.p4_expression):
            # if left is expr/cf/tuple
            if not isinstance(p4_expr.left, int):
                if not self.op2:
                    self.op2 = p4_expr.op
                else:
                    ncc_assert(not self.op3)
                    self.op3 = p4_expr.op
                if not isinstance(p4_expr.left, p4.p4_expression):
                    if not self.src_reg:
                        self.src_reg = left_cf if left_cf else p4_expr.left
                    else:
                        ncc_assert(not self.src1)
                        self.src1 = left_cf if left_cf else p4_expr.left
            # elif left is const
            elif isinstance(p4_expr.left, int):
                ncc_assert(not self.op3)
                self.op3 = p4_expr.op
                self.const = p4_expr.left
            else:
                ncc_assert(0)
            return
        # cases:
        # cf/tuple +- cf/tuple
        # cf/tuple +- const
        # cf/tuple <<>> shift_val
        # src_reg OP2 ((src1 OP1 shft) OP3 c
        if p4_expr.op in supported_ops1:
            # left must be pkt_field/reg and right must be int
            if self.op1:
                # set due to dummy expression creation
                ncc_assert(self.shft == 0 and not self.src_reg)
                self.src_reg = self.src1
            self.op1 = p4_expr.op
            ncc_assert(isinstance(p4_expr.left, p4.p4_field) or \
                   isinstance(p4_expr.left, tuple), \
                   "%s:unsupported left oprand - Must refer to packet data (found %s)" % \
                   (self.parser.d.name, p4_expr.left))
            if left_cf:
                self.src1 = left_cf #p4_expr.left
            else:
                self.src1 = p4_expr.left # tuple
            ncc_assert(isinstance(p4_expr.right, int), "invalid operand %s for as shift val" % \
                (p4_expr.right))
            self.shft = p4_expr.right
        else:
            # top level is +-, left or right can be expr involving << >>
            if left_cf:
                cf1 = left_cf
            elif isinstance(p4_expr.left, tuple):
                cf1 = p4_expr.left
            else:
                cf1 = None
            if right_cf:
                cf2 = right_cf
            elif isinstance(p4_expr.right, tuple):
                cf2 = p4_expr.right
            else:
                cf2 = None

            # handle : reg/pkt_fld +- reg/pkt_fld
            if cf1 and cf2:
                ncc_assert(not self.src_reg)
                ncc_assert(not self.src1)

                if isinstance(cf1, capri_field) and cf1.parser_local:
                    self.src_reg = cf1
                    self.src1 = cf2
                else:
                    self.src_reg = cf2
                    self.src1 = cf1
                if not self.op1:
                    self.op1 = '<<'
                    self.shft = 0
                self.op2 = p4_expr.op

            elif cf1:
                ncc_assert(isinstance(p4_expr.right, int))
                ncc_assert(not self.op3)
                self.op3 = p4_expr.op
                self.const = p4_expr.right

                # if cf1 is tuple or pkt fld, first choice is to use src1
                # if src1 is already used, use src_reg
                if (isinstance(cf1, tuple) or not cf1.parser_local) and not self.src1:
                    self.src1 = cf1
                else:
                    self.src_reg = cf1
            elif cf2:
                ncc_assert(isinstance(p4_expr.left, int))
                ncc_assert(not self.op3)
                self.op3 = p4_expr.op
                self.const = p4_expr.left

                # if cf2 is tuple or pkt fld, first choice is to use src1
                # if src1 is already used, use src_reg
                if (isinstance(cf2, tuple) or not cf2.parser_local) and not self.src1:
                    self.src1 = cf2
                else:
                    self.src_reg = cf2

    def add_signed_const(self, s_val):
        # s_val is a signed value
        if self.op3:
            if self.op3 == '-' and (s_val < 0):
                self.const -= s_val  # add fixed size (-ve)
            elif self.op3 == '-' and (s_val > 0):
                if self.const > s_val:
                    self.const -= s_val
                else:
                    self.op3 = '+'
                    self.const = (s_val - self.const)
            elif self.op3 == '+' and (s_val > 0):
                self.const += s_val
            elif self.op3 == '+' and (s_val < 0):
                delta = self.const + s_val # signed addition
                if delta < 0:
                    self.op3 = '-'
                    self.const = delta * (-1)
                else:
                    self.const = delta
        else:
            if s_val > 0:
                self.op3 = '+'
                self.const = s_val
            else:
                self.op3 = '-'
                self.const = s_val * (-1)

    def flatten_capri_expr(self, no_const=False):
        if no_const:
            # Keep operator used with constant so that expr has
            # no numerical value but all other part.
            pstr = 'EXPR:%s %s (%s %s %s) %s\n' % \
                (self.src_reg.hfname if isinstance(self.src_reg, capri_field) else self.src_reg, self.op2,
                 self.src1.hfname if isinstance(self.src1, capri_field) else self.src1,
                 self.op1, self.shft, self.op3 if self.const else 'None')

        else:
            pstr = 'EXPR:%s %s (%s %s %s) %s %s\n' % \
                (self.src_reg.hfname if isinstance(self.src_reg, capri_field) else self.src_reg, self.op2,
                 self.src1.hfname if isinstance(self.src1, capri_field) else self.src1,
                 self.op1, self.shft,
                 self.op3 if self.const else 'None', self.const)
        return pstr


    def __repr__(self):
        pstr = 'EXPR:%s %s (%s %s %s) %s %s\n' % \
                (self.src_reg.hfname if isinstance(self.src_reg, capri_field) else self.src_reg, self.op2,
                 self.src1.hfname if isinstance(self.src1, capri_field) else self.src1,
                 self.op1, self.shft,
                 self.op3, self.const)
        return pstr

class capri_parser_len_chk_profile:
    def __init__(self, parser, name):
        self.parser = parser
        self.name = name
        self.start_hfname = None
        self.len_hfname = None
        self.start_offset = 0
        self.offset_op = '+'
        self.cmp_op = 'gt'
        self.prof_id = None

class capri_header:
    def __init__(self, p4_header):
        self.p4_header = p4_header
        self.branches = []

    def add_branch(self, chdr):
        if chdr == self: return # not sure why headers are repeated on paths - check
        if chdr not in self.branches:
            self.branches.append(chdr)

    def name(self):
        return self.p4_header.name if self.p4_header else 'Root'

    def __repr__(self):
        return "Hdr:%s => %s" % \
            (self.name(), \
            [h.name() for h in self.branches])

class capri_parser:
    # one per direction
    def __init__(self, capri_be, d):
        self.be = capri_be
        self.d = d
        self.logger = logging.getLogger('Parser')
        self.start_state = None
        self.hw_start_state = None  # special state that is used to boot-strap hw
        self.states = []
        self.headers = []
        self.deparse_only_hdrs = []
        self.extracted_fields = []
        self.paths = []                 # headers extracted along a parse path
        self.path_states = []           # states visited along a parse path
        self.longest_path_size = 0
        self.ohi = None
        self.ohi_used = 0
        self.wr_only_ohi = OrderedDict()    # {wr_only_fld_name : ohi_slot}
        self.wr_only_hdrs = [] #All headers with parser_write_only directive.
        self.hdr_ext_states = OrderedDict() # {hdr : [ states where it is extracted ] }
        self.meta_ext_states = OrderedDict() # {meta_fld : [ states where it is extracted ] }
        self.hdr_order_groups = []  # header groups specified using header_ordering pragma
        parser_flits = self.be.hw_model['parser']['parser_num_flits']
        hv_per_flit = self.be.hw_model['parser']['flit_hv_bits']
        self.hdr_hv_bit = OrderedDict() # ordered so it is easy to debug when printed
        self.csum_hdr_hv_bit = OrderedDict()
        self.icrc_hdr_hv_bit = OrderedDict()
        self.hv_bit_header = [ None for _ in range(self.be.hw_model['parser']['max_hv_bits']) ]

        self.var_len_headers = OrderedDict() # {hdr_name : var_len_exp|str}
        self.asic            = capri_be.asic

        self.flit_hv_idx_start = [0 for _ in range(parser_flits)]
        hv_start_bit = 0
        for fl in range(parser_flits):
            self.flit_hv_idx_start[fl] = hv_start_bit
            hv_start_bit += hv_per_flit[fl]

        self.saved_lkp_scope = OrderedDict()  # { cf : _scope() } - scope of a saved_lkp_field
        self.saved_lkp_reg_allocation = [None for _ in self.be.hw_model['parser']['lkp_regs']]
        self.saved_lkp_fld = OrderedDict() # {cf : saved_lkp_reg} XXX multiple regs per field

        self.free_chksum_ohi_slots = [True for i in \
                range(self.be.hw_model['parser']['ohi_threshold']\
                + self.be.hw_model['parser']['max_csum_engines'] * 2, \
                self.be.hw_model['parser']['num_ohi'])]

        self.free_ohi_slots = [True for i in \
                range(self.be.hw_model['parser']['ohi_threshold'])]

        self.len_chk_profiles = \
            [None for _ in range(self.be.hw_model['parser']['num_len_chk_profiles'])]

        self.hdr_graph = None

    def get_header_size(self, hdr):
        # return fixed len or P4field/expression that represents len
        # Bytes
        if hdr.name not in self.var_len_headers:
            return hdr.header_type.length
        else:
            return self.var_len_headers[hdr.name]

    def initialize(self):
        #Collect write_only ohi fields
        for hdr_name, hdr in self.be.h.p4_header_instances.items():
            if 'parser_write_only' in hdr._parsed_pragmas:
                self.wr_only_hdrs.append(hdr)
                for field in hdr.fields:
                    self.wr_only_ohi[field.name] = None

        # create the state dictionary using capri_parse_state class
        capri_parse_states = OrderedDict()
        for k,v in self.be.h.p4_parse_states.items():
            if 'xgress' in v._parsed_pragmas:
                if v._parsed_pragmas['xgress'].keys()[0].upper() != self.d.name:
                    # skip this state, since it is specified for other direction only
                    self.logger.debug("%s:%s is excluded due to xgress pragma" % \
                        (self.d.name, k))
                    continue
            capri_parse_states[k] = capri_parse_state(self, v)

        # init branch_to dictionaries to point to capri_parse_states
        end_state = capri_parse_state(self, None, "__END__")
        end_state.is_end = True
        for k,v in capri_parse_states.items():
            for bkey,bv in v.p4_state.branch_to.items():
                if isinstance(bv, p4.p4_parse_state):
                    if bv.name not in capri_parse_states:
                        # this state was not included (could be because xgress pragma)
                        # Need to replace it with END state if this was a default transition
                        if bkey == p4.P4_DEFAULT:
                            v.branch_to[bkey] = end_state
                        continue
                    v.branch_to[bkey] = capri_parse_states[bv.name]
                else:
                    # table or conditional node - replace it with __END__ state
                    v.branch_to[bkey] = bv

        if self.d == xgress.INGRESS:
            self.start_state = capri_parse_states['start']
        elif 'egress_start' in capri_parse_states:
            self.start_state = capri_parse_states['egress_start']
        else:
            self.start_state = capri_parse_states['start']
        self.start_state.is_hw_start = True

        # create topo-sorted list of parser states
        marker = {} # {state: mark}
        sorted_states = []
        if _parser_topo_sort(self, self.d, capri_parse_states, self.start_state, \
                                marker, sorted_states):
            self.logger.critical("Loops in parse graph are not supported")
            ncc_assert(0)

        self.logger.info("%s States: %d" % (self.d.name, len(sorted_states)))
        # un-roll the states that use virtual headers (aka header stacks)
        for s in sorted_states:
            vhdrs = _get_p4_virtual_headers(s.p4_state)
            if len(vhdrs) == 0:
                self.states.append(s)
                continue
            ncc_assert((len(vhdrs) == 1) )# support only one header-stack
            vhdr = vhdrs[0]
            unrolled_states = []
            next_unrolled_state = None
            for i in range(vhdr.max_index+1 ,0, -1):
                cs = capri_parse_state(self, s.p4_state)
                cs.name += '__%d' % (i-1)
                cs.unrolled_state = True
                cs.unrolled_idx = int(i-1)
                for k,v in s.branch_to.items():
                    if v == s:
                        if not next_unrolled_state:
                            # remove branch to itself from the last node
                            continue
                        # link to the next node
                        cs.branch_to[k] = next_unrolled_state
                    else:
                        # copy other branches as-is (exit branch)
                        cs.branch_to[k] = v
                next_unrolled_state = cs
                unrolled_states.insert(0,cs)
            # connect un-rolled state sequence to branch_to for the original state.prev XXX
            # or keep it for now as virtual
            ncc_assert(len(unrolled_states), "Header Stack of 0 size?? Not allowed")
            # remove other branches and add a branch to unrolled states
            # XXX Need to change all incoming branches to skip this virtual state and
            # point to unrolled states below it
            for b in s.branch_to.keys()[:]:
                del s.branch_to[b]
            s.branch_to[p4.P4_DEFAULT] = unrolled_states[0]
            s.is_virtual = True
            self.states.append(s)
            for us in unrolled_states:
                self.states.append(us)

        # TBD - Same header is not expected to appear multiple times in a parse graph
        # accept it for now - add ncc_assert(later)
        hdr_processed = {}
        syn_headers = []
        # initialize headers and fields extracted in each state and in a global list
        # A field is added to extracted_fields only if it is used in the pipeline
        ohi_flds = OrderedDict()

        for s in self.states:
            if s.is_virtual:
                continue
            if not s.p4_state:
                continue
            s_ohi_flds = OrderedDict()
            for c in s.p4_state.call_sequence:
                if c[0] == p4.parse_call.extract:
                    hdr = c[1]
                    if hdr.virtual:
                        if not s.unrolled_state:
                            self.logger.critical("Header stack not allowed in this state - %s" % \
                                                    s.name)
                            ncc_assert(0)
                        unrolled_hdr_name = hdr.base_name + '[%d]' % s.unrolled_idx
                        # get the real header instance for the virtual header
                        hdr = self.be.h.p4_header_instances[unrolled_hdr_name]

                    if hdr.name == 'capri_i2e_metadata':
                        if self.d == xgress.INGRESS:
                            self.logger.warning(\
                                "i2e_metadata cannot be extracted by ingress parser, IGNORED")
                            continue

                    hdr_no_ohi = False
                    if not self.be.args.no_ohi:
                        # check if header has no-ohi pragma
                        if 'no_ohi' in hdr._parsed_pragmas:
                            ncc_assert(len(hdr._parsed_pragmas['no_ohi'].keys()), \
                            "Provide direction as no_ohi [xgress/ingress/egress]")
                            pdirection = hdr._parsed_pragmas['no_ohi'].keys()[0]
                            if pdirection.upper() == 'XGRESS' or \
                                pdirection.upper() == self.d.name:
                                hdr_no_ohi = True

                    for fld in hdr.fields:
                        if is_synthetic_header(hdr):
                            if hdr not in syn_headers:
                                syn_headers.append(hdr)
                            continue # do not add fields of syn header, add them at the end
                        hf_name = get_hfname(fld)
                        if ((self.d == xgress.INGRESS) and \
                                (self.be.h.p4_fields[hf_name].ingress_read or \
                                 self.be.h.p4_fields[hf_name].ingress_write)) or \
                            ((self.d == xgress.EGRESS) and \
                                (self.be.h.p4_fields[hf_name].egress_read or \
                                 self.be.h.p4_fields[hf_name].egress_write)):
                            cfield = self.be.pa.get_field(hf_name, self.d)
                            ncc_assert(cfield)
                            if hdr not in hdr_processed:
                                # many roce headers are extracted from multiple states
                                self.extracted_fields.append(cfield)
                            s.extracted_fields.append(cfield)
                            # check if this is variable len fld
                            ncc_assert(cfield.width != 0, \
                                "%s:Variable len fld %s cannot be used in the pipeline" % \
                                (self.d.name, s.name))
                        else:
                            cfield = self.be.pa.get_field(hf_name, self.d)
                            if not cfield:
                                cfield = self.be.pa.allocate_field(fld, self.d)

                            if hdr not in hdr_processed:
                                self.extracted_fields.append(cfield)
                            s.extracted_fields.append(cfield)

                            if not hdr_is_intrinsic(hdr) and not hdr_no_ohi:
                                cfield.set_ohi()
                                if hdr not in s_ohi_flds:
                                    s_ohi_flds[hdr] = [cfield]
                                else:
                                    s_ohi_flds[hdr].append(cfield)
                            else:
                                # if --no-ohi is set, cannot allow variable len flds
                                ncc_assert(not cfield.is_ohi, \
                                    "Cannot allow %s without ohi support" % cfield.hfname)

                    # common processing for i2e_meta header and regular headers
                    s.headers.append(hdr)
                    if hdr not in hdr_processed:
                        hdr_processed[hdr]=1
                        self.headers.append(hdr)

                    if hdr not in self.hdr_ext_states:
                        self.hdr_ext_states[hdr] = [s]
                    else:
                        self.hdr_ext_states[hdr].append(s)

                    # add hdr and ohi info in a global list
                    if hdr in s_ohi_flds and hdr not in ohi_flds:
                        ohi_flds[hdr] = s_ohi_flds[hdr]

                    if header_is_variable_len(hdr):
                        s.variable_hdr = True

                if c[0] == p4.parse_call.set:
                    cfield = self.be.pa.get_field(get_hfname(c[1]), self.d)
                    if not cfield:
                        if self.d == xgress.INGRESS:
                            # issue a wanrning only for ingress, for egress set_meta operations
                            # are disabled
                            self.logger.warning("%s:%s:Ignoring set_metadata() to unused field %s" % \
                            (self.d.name, s.name, get_hfname(c[1])))
                        continue
                    elif not cfield.parser_local:
                        # This field is used in pipeline, extract it to phv
                        # Egress parser is NOT allowed to use set_metadata -> phv
                        # Any required fields in egress pipeline will be populated via
                        # bridged metadata. Exceptions to this must be specified by the
                        # programmer via a special pragma on the state
                        if self.d == xgress.EGRESS:
                            if 'allow_set_meta' not in s.p4_state._parsed_pragmas or \
                                cfield.hfname not in s.p4_state._parsed_pragmas['allow_set_meta']:
                                self.logger.warning("%s:%s:Ignoring set_metadata() to field %s" % \
                                    (self.d.name, s.name, cfield.hfname))
                                continue
                            self.logger.info("%s:%s:Allow set_metadata() to %s" % \
                                (self.d.name, s.name, cfield.hfname))

                        cfield.is_parser_extracted = True
                        if cfield in self.extracted_fields:
                            # move it to the last occurance of this field
                            cidx = self.extracted_fields.index(cfield)
                            self.extracted_fields.pop(cidx)
                        self.extracted_fields.append(cfield)
                        if cfield not in s.meta_extracted_fields:
                            self.logger.debug("%s:%s:meta_field %s" % \
                                (self.d.name, s.name, cfield.hfname))
                            s.meta_extracted_fields.append(cfield)
                        if cfield not in self.meta_ext_states:
                            self.meta_ext_states[cfield] = [s]
                        else:
                            self.meta_ext_states[cfield].append(s)
                    else:
                        pass

                    set_op = capri_parser_set_op(s, cfield, c[2])
                    if cfield.no_register():
                        s.no_reg_set_ops.append(set_op)
                        self.logger.debug("%s:%s No_Reg set_op = %s" % \
                            (self.d.name, s.name, set_op));
                    else:
                        s.set_ops.append(set_op)
                    hf_name = cfield.hfname
                    if cfield.parser_local:
                        if set_op.op_type == meta_op.LOAD_REG:
                            s.local_flds[hf_name] = (cfield, 0, 1) # rd=0 wr=1
                        elif set_op.op_type == meta_op.UPDATE_REG:
                            s.local_flds[hf_name] = (cfield, 1, 1) # rd=1 wr=1
                        self.logger.debug("%s:%s Add local field %s  %s:%d:%d" % \
                            (self.d.name, s.name, hf_name, s.local_flds[hf_name][0].hfname, \
                             s.local_flds[hf_name][1], \
                             s.local_flds[hf_name][2]))

                    if set_op.op_type == meta_op.EXTRACT_REG and set_op.src_reg and \
                        set_op.src_reg.is_meta:
                        hf_name = set_op.src_reg.hfname
                        if hf_name not in s.local_flds:
                            s.local_flds[hf_name] = (set_op.src_reg, 1, 0) # rd=1 wr=0
                            self.logger.debug("%s:%s Add local field(Ext) %s: %d:%d" % \
                                (self.d.name, s.name, hf_name, \
                                 s.local_flds[hf_name][1], \
                                 s.local_flds[hf_name][2]))

                    # Add any RHS variables to local_flds
                    if set_op.capri_expr:
                        set_op_expr = set_op.capri_expr
                        if set_op_expr.src_reg and isinstance(set_op_expr.src_reg, capri_field):
                            if set_op_expr.src_reg.is_meta and \
                                set_op_expr.src_reg.hfname not in s.local_flds:
                                hf_name = set_op_expr.src_reg.hfname
                                s.local_flds[hf_name] = (set_op_expr.src_reg, 1, 0) # rd=1 wr=0
                                self.logger.debug("%s:%s Add local field (RHS)%s: %d:%d" % \
                                    (self.d.name, s.name, hf_name, \
                                     s.local_flds[hf_name][1], \
                                     s.local_flds[hf_name][2]))
                        if set_op_expr.src1 and isinstance(set_op_expr.src1, capri_field):
                            if set_op_expr.src1.is_meta and set_op_expr.src1.hfname not in s.local_flds:
                                hf_name = set_op_expr.src1.hfname
                                s.local_flds[hf_name] = (set_op_expr.src1, 1, 0) # rd=1 wr=0
                                self.logger.debug("%s:%s Add local field(RHS) %s: %d:%d" % \
                                    (self.d.name, s.name, hf_name, \
                                     s.local_flds[hf_name][1], \
                                     s.local_flds[hf_name][2]))
                            # XXX - what if src1 is pkt fld not extracted in this state (reserve lkp?)
                            # not handled currently, do it by forcing a local variable
                            # Add it to upstream variables and make reservations

        for hdr in syn_headers:
            for f in hdr.fields:
                cf = self.be.pa.get_field(get_hfname(f), self.d)
                if not cf:
                    cf = self.be.pa.allocate_field(f, self.d)
                if cf not in self.extracted_fields:
                    self.extracted_fields.append(cf)

        deparse_only_hdrs = []
        # change branch_on fields to capri_fields
        for cs in self.states:
            if not cs.p4_state:
                continue
            # checke deparser-only states
            if 'deparse_only' in cs.p4_state._parsed_pragmas:
                cs.deparse_only = True
                for hdr in cs.headers:
                    if hdr not in deparse_only_hdrs:
                        deparse_only_hdrs.append(hdr)

            # create header_ordering groups (to be placed together in a flit)
            if 'header_ordering' in cs.p4_state._parsed_pragmas:
                hdr_group = \
                    get_pragma_param_list(cs.p4_state._parsed_pragmas['header_ordering'])
                p4_hdr_group = []
                for h in hdr_group:
                    if h not in self.be.h.p4_header_instances:
                        continue
                    p4h = self.be.h.p4_header_instances[h]
                    if p4h not in self.headers:
                        # header not used
                        continue
                    p4_hdr_group.append(p4h)
                if len(p4_hdr_group) != 0:
                    self.hdr_order_groups.append(p4_hdr_group)

            if 'packet_len_check' in cs.p4_state._parsed_pragmas:
                len_chk_pragma = cs.p4_state._parsed_pragmas['packet_len_check']
                for _name in len_chk_pragma.keys():
                    _name = len_chk_pragma.keys()[0]
                    len_chk_profile = self.len_chk_profile_get(_name)
                    if not len_chk_profile:
                        len_chk_profile = capri_parser_len_chk_profile(self, _name)
                        if self.len_chk_profile_alloc(len_chk_profile) == None:
                            ncc_assert(0, "No len check profile avaialble for %s" % _name)
                    if 'start' in len_chk_pragma[_name].keys():
                        ncc_assert(len_chk_profile.start_hfname == None)
                        len_chk_profile.start_hfname = len_chk_pragma[_name]['start'].keys()[0]
                    if 'len' in len_chk_pragma[_name].keys():
                        len_params = get_pragma_param_list(len_chk_pragma[_name]['len'])
                        ncc_assert(len(len_params) > 1)
                        ncc_assert(len_params[0] == 'eq' or len_params[0] == 'gt')
                        len_chk_profile.cmp_op = len_params[0]
                        ncc_assert(len_chk_profile.len_hfname == None)
                        len_chk_profile.len_hfname = len_params[1]
                        if len(len_params) > 3:
                            ncc_assert(len_params[2] == '+' or len_params[2] == '-')
                            len_chk_profile.offset_op = len_params[2]
                            len_chk_profile.start_offset = int(len_params[3])
                    cs.len_chk_profiles.append(len_chk_profile)

            if not cs.p4_state or cs.is_virtual:
                continue
            for b_on in cs.p4_state.branch_on:
                if isinstance(b_on, p4.p4_field):
                    hdr = b_on.instance
                    if hdr.virtual:
                        ncc_assert(cs.unrolled_state)
                        # used un-rolled header
                        unrolled_hdr_name = hdr.base_name + '[%d]' % cs.unrolled_idx
                        hf_name = unrolled_hdr_name + '.' + b_on.name
                        cf = self.be.pa.get_field(hf_name, self.d)
                    else:
                        hf_name = get_hfname(b_on)
                        cf = self.be.pa.get_field(hf_name, self.d)
                    if not cf:
                        cf = self.be.pa.allocate_field(b_on, self.d)
                    ncc_assert(cf)
                    if cf.is_meta:
                        ncc_assert(cf.parser_local, "%s:%s %s must be parser_local for using as select" % \
                            (self.d.name, cs.name, cf.hfname))
                    if cf.parser_local:
                        # if metadata is used for select(), it must be parser local
                        if hf_name not in cs.local_flds:
                            cs.local_flds[hf_name] = (cf, 1, 0)     # rd=1, wr=0
                            self.logger.debug("%s:%s Add lkp local field %s - %d:%d" % \
                                (self.d.name, cs.name, cf.hfname, cs.local_flds[hf_name][1], \
                                 cs.local_flds[hf_name][2]))
                    cs.branch_on.append(cf)
                else:
                    cs.branch_on.append(b_on)

        # fix deparse-only header list - remove any header that is also extracted in
        # non-deparse-only state
        for hdr in deparse_only_hdrs:
            # get_ext_cstates only report non-deparse-only states, so a non zero list
            # indicates that hdr is extracted in non-deparse-only state
            ext_css = self.get_ext_cstates(hdr)
            if len(ext_css):
                continue
            if hdr not in self.deparse_only_hdrs:
                self.deparse_only_hdrs.append(hdr)

        # fix the len expression in variable size headers
        for hdr in self.headers:
            if not header_is_variable_len(hdr):
                continue
            if hdr.name in self.var_len_headers:
                continue
            # The header len is specified on per instance and not per type.
            # Ideally we now need capri_header... (may be add it later), for now create a
            # different dictionary for it
            v_size_exp = None
            hlen_fld_name = None
            hlen_dummy = None
            if 'hdr_len'in hdr._parsed_pragmas:
                # hdr_len pragma is used to substitute hdr_len fld with another
                # field from any other header
                hlen_fld_name = hdr._parsed_pragmas['hdr_len'].keys()[0]
                hlen_fld = self.be.h.p4_fields[hlen_fld_name]
                ncc_assert(hlen_fld, "hdr_len specified %s does not exists" % hlen_fld_name)
                # find p4_fld 'hdr_len' in the header
                p4f = None
                for p4f in hdr.fields:
                    if p4f.name == 'hdr_len':
                        hlen_dummy = p4f.name
                        break
                ncc_assert(hlen_dummy, "Header %s must have a field \'hdr_len\'" % hdr.name)

                # remove the dummy fields from various lists
                p4cf = self.be.pa.get_field(hdr.name + '.' + hlen_dummy, self.d)
                ncc_assert(p4cf in self.extracted_fields)
                self.extracted_fields.remove(p4cf)
                for cs in self.hdr_ext_states[hdr]:
                    ncc_assert(p4cf in cs.extracted_fields)
                    cs.extracted_fields.remove(p4cf)
                # HACK - hlir seems to keep a single instance (copy) of a header for both
                # ingress and egrss.. for now assume that we process ingress before egress
                # and remove the hdr field only on egress (XXX this will be a problem in msft
                # mode where each pipeline is used by itself.) Best to have capri_hdr
                # Since build ohi is called before egress processing it is a problem, currently
                # build_ohi is called multiple times which fixes the problem
                if self.d == xgress.EGRESS or (self.d == xgress.INGRESS and self.be.args.single_pipe):
                    hdr.fields.remove(p4f)
                    ncc_assert(len(hdr.fields) == 1 )# HACK only 2 flds allowed, 1 removed
                    hdr.fields[0].offset = 0

            if isinstance(hdr.header_type.length, p4.p4_expression):
                v_size_exp = copy.deepcopy(hdr.header_type.length)
                _resolve_len_expr(self, hdr, v_size_exp, hlen_fld_name, hlen_dummy)
            elif isinstance(hdr.header_type.length, str):
                if not hlen_fld_name:
                    hlen_fld_name = hdr.name + '.' + hdr.header_type.length
                v_size_exp = hlen_fld_name
            else:
                ncc_assert(0, "Invalid len spec %s for variable len header" % hlen_fld_name)

            self.logger.debug("%s:Variable hdr %s len %s" % (self.d.name, hdr.name, v_size_exp))
            self.var_len_headers[hdr.name] = v_size_exp

        self.logger.debug("%s: States %s" % (self.d.name, [s.name for s in self.states]))
        self.logger.info("%s: Total Headers : %d" % (self.d.name, len(self.headers)))
        self.logger.info("Total Fixed Headers size: %d Bytes" % \
            sum([get_header_fixed_size(h) for h in self.headers]))
        self.logger.debug('Headers: %s' % self.headers)

        self.logger.info("%s: Find parse paths.." % (self.d.name))
        self.paths, self.path_states = self._find_parser_paths(self.start_state)
        self.logger.info("%s: Total parse_state paths %d" % (self.d.name, len(self.path_states)))
        state_count = 0
        lp = None
        for i, ps in enumerate(self.path_states):
            if len(ps) > state_count:
                state_count = len(ps)
                lp = ps
        self.logger.info("%s: Longest parse path = %d states" % (self.d.name, len(lp)))
        self.logger.debug("%s: Longest parse path(%d)=%s" % (self.d.name, len(lp), lp))
        self.longest_path_size = len(lp)
        # validate if all headers got covered
        hdrs_covered = {}
        for path in self.paths:
            for hdr in path:
                hdrs_covered[hdr] = True
        for h in self.headers:
            ncc_assert(h in hdrs_covered, "Missed header %s in path calculations" % h.name)

        # create correct header order and replace self.headers with it.
        # header order created based on topo-sorted states is incorrect when the same header
        # is extracted from multiple states
        # XXX PHV allocation is still broken (can get the code from Sorrento where it is fixed)
        self.create_header_graph()
        # Remove impossible branches - these are added to create deparser parse-graph
        # Removing them will reduce processing and resource requirements like saved registers
        # 'impossible' branch is the one where mask is set to mask off non-zero bits in case val
        # also remove all branches after 'default'
        # remove the deparse-only branches as well
        remove_branch = False
        for cs in self.states:
            found_default = False
            for bkey,bval in cs.branch_to.items():
                if bkey == p4.P4_DEFAULT:
                    found_default = True
                    continue

                if found_default:
                    remove_branch = True
                    del cs.branch_to[bkey]
                    continue

                if isinstance(bval, capri_parse_state) and bval.deparse_only:
                    pdb.set_trace()
                    remove_branch = True
                    del cs.branch_to[bkey]
                    continue

                if isinstance(bkey, tuple):
                    cval = int(bkey[0])
                    mask = int(bkey[1])
                    if cval and (cval & mask) == 0:
                        # remove this branch
                        self.logger.debug("%s:Removing impossible branch (0x%x & 0x%x)->%s from %s" %\
                            (self.d.name, cval, mask, cs.branch_to[bkey], cs.name))
                        del cs.branch_to[bkey]
                        remove_branch = True

        # recompute the paths after removing the dummy(impossible) branches
        if remove_branch:
            self.logger.info("%s:Find parse paths (after removing dummy).." % (self.d.name))
            #dbg_on = True if self.d == xgress.EGRESS else False
            dbg_on = False
            self.paths, self.path_states = self._find_parser_paths(self.start_state, dbg_on)
            self.logger.info("%s: Total parse_state paths %d" % (self.d.name, len(self.path_states)))

        self.path_states = sorted(self.path_states, key=lambda p: len(p), reverse=True)

    def init_ohi(self):
        # Do this after both ingress and egress inits are done
        self.ohi = self._build_ohi(None)

    def create_hw_start_state(self):
        # NOT USED
        # this is a dummy state to boot-strap the hw parser
        hw_state = capri_parse_state(self, None, name='hw_start')
        hw_state.branch_to[p4.P4_DEFAULT] = self.start_state
        hw_state.is_hw_start = True
        self.states.insert(0, hw_state)

    def _hdr_ohi_to_phv(self, hdr, ohi):
        # convert all header flds to phv for specified ohi(off, len)
        result = False
        for fld in hdr.fields:
            hf_name = get_hfname(fld)
            cf = self.be.pa.get_field(hf_name, self.d)
            # Since headers are allowed to be extracted from multiple states (topo-sort???)
            # flds could be already converted to phv
            if cf.width == 0:
                continue

            ohi_start = ohi.start * 8 # byte->bit
            if not isinstance(ohi.length, int):
                # can be called for a variable len header, which cannot move to phv
                return False
            ohi_length = ohi.length * 8
            if fld.offset >= ohi_start and (fld.offset+cf.width) <= (ohi_start+ohi_length):
                cf.reset_ohi()
                result = True

            if fld.offset >= (ohi_start+ohi_length):
                break

        return result

    def update_ohi_per_state(self):
        num_ohi_per_state = self.be.hw_model['parser']['num_ohi_per_state']
        for cs in self.states:
            if cs.is_hw_start:
                # No ohi on first state - so it causes no problems when merged with next state
                # with pre-extraction
                max_ohi_per_state = 0
            else:
                max_ohi_per_state = num_ohi_per_state
            if cs.no_extract():
                continue
            num_ohis = sum([len(self.ohi[hdr]) for hdr in cs.headers if hdr in self.ohi])
            # each ohi requires two slots
            if num_ohis <= (max_ohi_per_state/2):
                continue
            # reduce the ohi usage
            for hdr in cs.headers:
                if hdr not in self.ohi:
                    continue
                if num_ohis <= (max_ohi_per_state/2):
                    break
                for ohi in self.ohi[hdr]:
                    if self._hdr_ohi_to_phv(hdr, ohi):
                        num_ohis -= 1
                    if num_ohis <= (max_ohi_per_state/2):
                        break
            ncc_assert(num_ohis <= (max_ohi_per_state/2), "Too many OHIs in state %s\n" % cs.name)
        # rebuild ohi
        self.ohi = self._build_ohi(None)

    def _build_ohi(self, fld_ohi):
        # create offset-len pairs for all the consecutive OHI entries
        # OHI entry must start and end on byte boundary
        if not fld_ohi:
            fld_ohi = OrderedDict()
            # build fld_ohi dictionary, if not done by the caller
            for hdr in self.headers:
                for fld in hdr.fields:
                    hf_name = get_hfname(fld)
                    cfield = self.be.pa.get_field(hf_name, self.d)
                    if cfield.is_ohi:
                        if hdr not in fld_ohi:
                            fld_ohi[hdr] = [cfield]
                        else:
                            fld_ohi[hdr].append(cfield)
        ohi = OrderedDict()
        for hdr in fld_ohi.keys():
            # TBD - it is possible to create OHI entries for union-headers
            # They can share OHI entries. #ohi = max(ohi for headers in union)
            # Also #phv = max(phv for headers in union)
            ohi_bits = 0
            nxt = 0
            vcf = None
            vl = None
            if header_is_variable_len(hdr):
                vcf = fld_ohi[hdr][-1] # last field has to be the only variable len fld
                ncc_assert(vcf.width == 0)
                v_size_exp = self.get_header_size(hdr)
                if isinstance(v_size_exp, p4.p4_expression):
                    vl = capri_parser_expr(self, v_size_exp)
                else:
                    ncc_assert(isinstance(v_size_exp, str))
                    # no real expression, convert str to p4_field
                    vl = capri_parser_expr(self, None)
                    # get the name of the len field and find capri_field obj
                    hfname = v_size_exp
                    cf = self.be.pa.get_field(hfname, self.d)
                    ncc_assert(cf)
                    if cf.is_meta:
                        vl.src_reg = cf
                        vl.op2 = '+'
                    else:
                        vl.src1 = cf
                        vl.op1 = '<<'
                        vl.shft = 0

                # Adjust expr for fixed portion of the header
                fixed_size = get_header_fixed_size(hdr) * (-1)  # subtract fixed portion
                if fixed_size:
                    vl.add_signed_const(fixed_size)

            for i, cf in enumerate(fld_ohi[hdr]):
                if ohi_bits == 0:
                    # can start new ohi segment
                    if cf.p4_fld.offset % 8:
                        self.logger.debug("exclude mis-aligned ohi start field %s" % cf.hfname)
                        cf.reset_ohi()
                        continue;
                    else:
                        # start new segment
                        ohi_start = cf.p4_fld.offset
                        last_ohiB = i
                        ohi_bits = cf.width
                        nxt = cf.p4_fld.offset + cf.width
                else:
                    ncc_assert(nxt)
                    if cf.p4_fld.offset == nxt:
                        # continue the ohi segment if the fld is contiguous
                        ohi_bits += cf.width
                        if (nxt % 8) == 0:
                            last_ohiB = i
                        nxt += cf.width
                    else:
                        # terminate the ohi segment on byte boundary
                        if ohi_bits % 8:
                            # truncate-remove all flds from last byte-aligned fld until current fld
                            # for r in range(i-1, last_ohiB-1, -1):
                            for r in range(last_ohiB, i):
                                ohi_bits -= fld_ohi[hdr][r].width
                                self.logger.debug("terminate - remove mis-aligned ohi field %s" % \
                                    fld_ohi[hdr][r].hfname)
                                fld_ohi[hdr][r].reset_ohi()
                        if ohi_bits:
                            ncc_assert((ohi_bits % 8) == 0)
                            if hdr in ohi:
                                ohi[hdr].append(capri_ohi(ohi_start/8, ohi_bits/8))
                            else:
                                ohi[hdr] = [capri_ohi(ohi_start/8, ohi_bits/8)]

                        ohi_bits = 0
                        nxt = 0
                        # check if we can start a new segment
                        if cf.p4_fld.offset % 8:
                            cf.reset_ohi()
                            self.logger.debug("remove mis-aligned ohi field %s" % cf.hfname)
                        else:
                            # start new segment
                            ohi_start = cf.p4_fld.offset
                            last_ohiB = i
                            ohi_bits = cf.width
                            nxt = cf.p4_fld.offset + cf.width

            if ohi_bits % 8:
                # last chunk is not byte aligned
                # truncate
                for r in range(last_ohiB, i+1):
                    ohi_bits -= fld_ohi[hdr][r].width
                    self.logger.debug("terminate - remove last mis-aligned ohi field %s" % \
                        fld_ohi[hdr][r].hfname)
                    fld_ohi[hdr][r].reset_ohi()
                ncc_assert((ohi_bits % 8) == 0)

            if ohi_bits:
                if hdr in ohi:
                    ohi[hdr].append(capri_ohi(ohi_start/8, ohi_bits/8))
                else:
                    ohi[hdr] = [capri_ohi(ohi_start/8, ohi_bits/8)]

            if vcf:
                ncc_assert((vcf.p4_fld.offset % 8) == 0)
                if hdr in ohi:
                    ohi[hdr].append(capri_ohi(vcf.p4_fld.offset/8, vl))
                else:
                    ohi[hdr] = [capri_ohi(vcf.p4_fld.offset/8, vl)]
            #self.logger.debug("OHI hdr %s: fields %s : %s" % \
            #    (hdr.name, [f.hfname for f in fld_ohi[hdr] \
            #    if f.is_ohi], ohi[hdr] if hdr in ohi else '[]'))
        self.logger.debug("%s:OHI %s" % (self.d.name, ohi))
        return ohi


    def assign_ohi_slots_for_checksum(self, csum_unit, instance, ohi_field, ohi_type):
        '''
        Every Csum engine needs atleast 2 OHIs. One for storing offset and
        second for storing csum  len. Hence for 5 csum engines first 10
        OHI slots are used.
        In some cases a csum unit needs more than 2 slots. Storing phdr offset
        tcp offset, tcp len.
        '''

        ohi_id = None
        if ohi_field != '':
            #Lookup  in write-only OHIs matching write_only_var
            if ohi_type == ohi_write_only_type.OHI_WR_ONLY_TYPE_HDR_OFFSET:
                #Get OHI ID of write only header start offset variable
                ohi_id = self.get_ohi_hdr_start_off(self.be.h.p4_header_instances[ohi_field])
            elif ohi_type == ohi_write_only_type.OHI_WR_ONLY_TYPE_HDR_LEN:
                #Get OHI ID of write only length variable
                ohi_id = self.get_ohi_hdr_len(self.be.h.p4_header_instances[ohi_field])
            else:
                ohi_id = self.get_ohi_slot_wr_only_field_name(ohi_field.split('.')[1])

        if ohi_id == None or ohi_field == '':
            if instance != -1:
                ohi_id = self.be.hw_model['parser']['ohi_threshold'] + 1 + \
                       (csum_unit * 2) + instance
            else:
                try:
                    ohi_id = self.free_chksum_ohi_slots.index(True)
                    self.free_chksum_ohi_slots[ohi_id] = False
                    ohi_id += self.be.hw_model['parser']['ohi_threshold'] + 1 +\
                            + (self.be.hw_model['parser']['max_csum_engines'] * 2)
                except:
                    ncc_assert(0)

        return ohi_id

    def assign_ohi_slots(self):
        self.assign_wr_only_ohi_slots()
        # go thru' longest parse_path and assign ohi slots to headers as needed
        # keep doing it until all headers are done
        paths = sorted(self.paths, key=lambda p: len(p), reverse=True)

        max_ohi = self.be.hw_model['parser']['ohi_threshold']
        max_ohi_used = 0
        max_ohi_path = None

        for path in paths:
            # ohi slots must be allocated for each path
            # make sure a given header gets the same slot on multiple paths
            free_slots = [True for i in range(max_ohi)]
            slot_overflow = False
            ohi_used = 0
            # collect all pre-allocated slots
            # a given header must use the same slot on all paths
            # also collect all globally allocated slots
            for v in self.wr_only_ohi.values():
                if v != None:
                    free_slots[v] = False
                    ohi_used += 1
            for hdr in path:
                if hdr not in self.ohi:
                    # header is in phv no ohi needed
                    continue
                for oh in self.ohi[hdr]:
                    if oh.id != -1:
                        free_slots[oh.id] = False
                        ohi_used += 1
                    if oh.var_id != -1:
                        free_slots[oh.var_id] = False
                        ohi_used += 1

            headers_in_path = set()
            for hdr in path:
                headers_in_path.add(hdr.name)
                if hdr not in self.ohi:
                    # header is in phv no ohi needed
                    continue
                # allocate 1 ohi slot per header for storing offset
                # allocate additional slot for variable len ohi
                fixed_ohi_id = self.ohi[hdr][0].id
                if fixed_ohi_id == -1:
                    # check if this is globally allocated
                    wr_only_ohi_name = hdr.name + '___start_off'
                    if self.ohi[hdr][0].start == 0 and wr_only_ohi_name in self.wr_only_ohi:
                        ohid = self.wr_only_ohi[wr_only_ohi_name]
                        ncc_assert(ohid != None)
                        self.logger.debug("%s:%s use global ohi slot %d" % \
                                (self.d.name, wr_only_ohi_name, ohid))
                    else:
                        try:
                            ohid = free_slots.index(True)
                            free_slots[ohid] = False
                            ohi_used += 1
                        except:
                            slot_overflow = True
                            break
                    self.ohi[hdr][0].id = ohid
                    fixed_ohi_id = ohid

                for i,oh in enumerate(self.ohi[hdr]):
                    if isinstance(oh.length, int):
                        continue
                    # XXX for var len, same offset slot can be used.. but expression
                    # needs to be adjusted based on first offset - TBD
                    # for now use an additional slot
                    if oh.var_id == -1:
                        # check if this is globally allocated
                        wr_only_ohi_name = hdr.name + '___hdr_len'
                        if wr_only_ohi_name in self.wr_only_ohi:
                            ohid = self.wr_only_ohi[wr_only_ohi_name]
                            self.logger.debug("%s:%s use global ohi slot %d" % \
                                (self.d.name, wr_only_ohi_name, ohid))
                            ncc_assert(ohid != None)
                        else:
                            try:
                                ohid = free_slots.index(True)
                                free_slots[ohid] = False
                                ohi_used += 1
                            except:
                                slot_overflow = True
                                break
                        oh.var_id = ohid

                    if i==0:
                        # var len slot is the only slot
                        oh.id = fixed_ohi_id

                if slot_overflow:
                    break

            # For deparse only headers, that alias to parsed header, capture their OHI
            # information for deparser to build packet correctly. Aliasing is supported
            # using write-only variables and by capturing header start and len in OHI.
            # Aliasing and aliased header are expected to be in OHI.
            for hdr in self.headers:
                if hdr.name not in headers_in_path:
                    if hdr not in self.ohi:
                        continue
                    fixed_ohi_id = self.ohi[hdr][0].id
                    if fixed_ohi_id == -1:
                        # check if this is globally allocated
                        wr_only_ohi_name = hdr.name + '___start_off'
                        if self.ohi[hdr][0].start == 0 and wr_only_ohi_name in self.wr_only_ohi:
                            ohid = self.wr_only_ohi[wr_only_ohi_name]
                            ncc_assert(ohid != None)
                            self.logger.debug("%s:%s use global ohi slot %d" % \
                                    (self.d.name, wr_only_ohi_name, ohid))
                            self.ohi[hdr][0].id = ohid
                            fixed_ohi_id = ohid
                    for i,oh in enumerate(self.ohi[hdr]):
                        if isinstance(oh.length, int):
                            continue
                        if oh.var_id == -1:
                            # check if this is globally allocated
                            wr_only_ohi_name = hdr.name + '___hdr_len'
                            if wr_only_ohi_name in self.wr_only_ohi:
                                ohid = self.wr_only_ohi[wr_only_ohi_name]
                                self.logger.debug("%s:%s use global ohi slot %d" % \
                                    (self.d.name, wr_only_ohi_name, ohid))
                                ncc_assert(ohid != None)
                                oh.var_id = ohid

            if ohi_used > max_ohi_used:
                max_ohi_used = ohi_used
                max_ohi_path = path

            #self.logger.debug("%s: OHI used %d" % (self.d.name, ohi_used))
            if slot_overflow:
                pdb.set_trace()
                break

        max_ohi = self.be.hw_model['parser']['ohi_threshold']
        for i in range(max_ohi):
            self.free_ohi_slots[i] = free_slots[i]

        self.logger.info("%s:Max ohi used %d" % (self.d.name, max_ohi_used))
        self.logger.debug("Max ohi path %s" % max_ohi_path)
        self.logger.debug("%s: OHI allocation %s" % (self.d.name, self.ohi))
        self.ohi_used = max_ohi_used
        return (max_ohi_used, max_ohi_path)

    def get_ohi_slot(self):
        ohi_id = self.free_ohi_slots.index(True)
        self.free_ohi_slots[ohi_id] = False
        return ohi_id

    def assign_wr_only_ohi_slots(self):
        # since wr_only variable will not appear in the path-headers, there is a small
        # chance that ohi-slot used for a wr-only variable will conflict with another header
        # on that path (same ohi slot can be used by different headers on different mutually
        # exclusive paths), but that cannot be checked for wr_only variable as it does not appear
        # on a path.. since wr_only variables are not visible on path-headers
        # for now do a global allocation for wr_only variables
        for k,v in self.wr_only_ohi.items():
            # reset
            self.wr_only_ohi[k] = None

        groups = 0
        ohi_groups = {}
        for wr_only_hdr in self.wr_only_hdrs:
            if 'parser_share_ohi' in wr_only_hdr._parsed_pragmas:
                for  shared_ohi_field_group in wr_only_hdr._parsed_pragmas['parser_share_ohi']:
                    shared_fields = get_pragma_param_list(\
                    wr_only_hdr._parsed_pragmas['parser_share_ohi'][shared_ohi_field_group])
                    ohi_groups[groups] = shared_fields
                    groups += 1
        ohi_slot = groups
        for k in self.wr_only_ohi.keys():
            shared = False
            for grp in range(groups):
                if k in ohi_groups[grp]:
                    self.wr_only_ohi[k] = grp
                    shared = True
                    break
            if not shared:
                self.wr_only_ohi[k] = ohi_slot
                ohi_slot += 1

    def get_ohi_hdr_start_off(self, hdr):
        # return ohi slot assigned to capture header start offset (if allocated)
        ohi_name = hdr.name + '___start_off'
        if hdr in self.ohi:
            if self.ohi[hdr][0].start == 0:
                return self.ohi[hdr][0].id
            elif ohi_name in self.wr_only_ohi:
                return self.wr_only_ohi[ohi_name]
            else:
                return None
        elif ohi_name in self.wr_only_ohi:
            return self.wr_only_ohi[ohi_name]
        else:
            return None

    def get_ohi_hdr_len(self, hdr):
        # return ohi slot assigned to capture header len. (if allocated)
        ohi_name = hdr.name + '___hdr_len'
        if hdr in self.ohi:
            return self.ohi[hdr].var_id
        elif ohi_name in self.wr_only_ohi:
            return self.wr_only_ohi[ohi_name]
        else:
            return None

    def get_ohi_slot_wr_only_cf(self, cf):
        ohi_name = cf.hfname.split('.')[1]
        ohi_names = ohi_name.split('___')
        if len(ohi_names) > 1:
            hdr_name = ohi_names[0]
            hdr = self.be.h.p4_header_instances[hdr_name]
            if ohi_names[1] == 'start_off':
                return self.get_ohi_hdr_start_off(hdr)
            elif ohi_names[1] == 'hdr_len':
                return self.get_ohi_hdr_len(hdr)
            else:
                return None
        else:
            if ohi_names[0] in self.wr_only_ohi:
                return self.wr_only_ohi[ohi_names[0]]
        return None

    def get_ohi_slot_wr_only_field_name(self, ohi_field):
        if ohi_field in self.wr_only_ohi:
            return self.wr_only_ohi[ohi_field]
        return None

    def _find_parser_paths(self, start_state, dbg_on=False):
        paths=[]
        path_states=[]
        current_path=[]
        path_hdrs=[]
        self._find_paths(start_state, paths, path_states, current_path, path_hdrs, dbg_on)
        return paths, path_states

    def _traverse_loops(self, pnode, loops):
        def _traverse_node(pnode, cnode, traversed_states):
            if cnode == pnode:
                return
            traversed_states.append(cnode)
            for nxt in cnode.branch_to.values():
                if not nxt or not isinstance(nxt, capri_parse_state):
                    return
                _traverse_node(pnode, nxt, traversed_states)

        traversed_states = []
        for cnode in loops:
            _traverse_node(pnode, cnode, traversed_states)
        return traversed_states

    def _find_paths(self, node, paths, path_states, current_path, path_hdrs, dbg_on):
        if node in current_path:
            return True # loop
        #print "find path at node %s" % node.name
        node_list = []
        for n in node.branch_to.values():
            # multiple case statements can go to the same branch
            # This greatly reduced the # of paths and hence compilation time
            if n not in node_list:
                node_list.append(n)
        extracted_hdrs = []
        traversed_nodes = [node]
        if 'header_ordering' in node.p4_state._parsed_pragmas:
            # unfortunately need to make a special case for this
            extracted_hdrs = self._prune_next_states(node, node_list)
            # collect all looping states as traversed states
            looping_branches = [nxt for nxt in node.branch_to.values() if nxt not in node_list]
            loop_traversed_nodes = self._traverse_loops(node, looping_branches)
            for n in loop_traversed_nodes:
                if n not in traversed_nodes:
                    traversed_nodes.append(n)

        extracted_hdrs += node.headers
        '''
        if dbg_on and len(path_states) >= 36 and len(path_states) <= 45:
            self.logger.debug("Path[%d]... %s" % (len(path_states), current_path+traversed_nodes))
            pdb.set_trace()
        '''
        for next_node in node_list:
            if not isinstance(next_node, capri_parse_state) or not next_node.p4_state:
                # terminate
                # check duplicate headers

                # do not terminate if there are other exit paths (this path will be a subset)
                if len(node_list) > 1:
                    continue

                new_path_hdrs = []
                for h in path_hdrs:
                    if h not in new_path_hdrs:
                        new_path_hdrs.append(h)

                for h in extracted_hdrs:
                    if h not in new_path_hdrs:
                        new_path_hdrs.append(h)
                #paths.append(path_hdrs+extracted_hdrs)
                paths.append(new_path_hdrs)
                path_states.append(current_path+traversed_nodes)
                continue
            if 'xgress' in next_node.p4_state._parsed_pragmas:
                if next_node.p4_state._parsed_pragmas['xgress'].keys()[0].upper() != \
                    self.d.name:
                    continue # skip - specified only for parser in other direction
            if next_node.is_virtual:
                # go the the unrolled chain
                next_node = next_node.branch_to.values()[0]
            #print "find path from %s -> %s" % (node.name, next_node.name)
            if next_node == node:
                continue; # loop to itself, ideally shoud use header stack.. but not mandatory
            if self._find_paths(next_node, paths, path_states, current_path+traversed_nodes, \
                    path_hdrs+extracted_hdrs, dbg_on):
                ncc_assert(0, "Unhandled parser loop at %s - Path %s??" % \
                    (next_node.name, current_path+traversed_nodes))
        return False

    def _prune_next_states(self, state, next_states):
        # find and remove all the states that extract headers in a given header_ordering
        nxt_hdr_list = get_pragma_param_list(state.p4_state._parsed_pragmas['header_ordering'])
        #print "prune states from %s Headers %s" % (state.name, nxt_hdr_list)
        p4_hdr_list = []
        for hname in nxt_hdr_list:
            if hname not in self.be.h.p4_header_instances:
                self.logger.warning("Unknown header in header_ordering pragma %s" % hname)
                continue;
            hdr = self.be.h.p4_header_instances[hname]
            p4_hdr_list.append(hdr)

            for nxt in next_states[:]:
                #print "find hdr %s in state %s" % (hdr.name, nxt.name)
                if _find_header(state, nxt, hdr):
                    # XXX do not remove this branch.. create branches_to_remove dict and do it
                    # later at the end
                    # this can cause problem when all specified headers are not extracted
                    next_states.remove(nxt)
                    # check other states too, may extract same header

        return p4_hdr_list

    def ohi_to_phv(self, max_ohi_path, n_ohi):
        self.logger.info("%s:Try to reduce ohi pairs by %d" % (self.d.name, n_ohi))
        xohi = sorted(self.ohi.items(), key=lambda (k,v): len(v), reverse=True)
        for hdr,oh in xohi:
            if hdr not in max_ohi_path:
                continue
            h_ohi = 0
            for ohi in oh:
                if self._hdr_ohi_to_phv(hdr, ohi):
                    h_ohi += 1

            if h_ohi == len(oh):
                self.logger.debug("%s:Hdr %s from will be removed from ohi, count %d" % \
                    (self.d.name, hdr.name, h_ohi))
            if h_ohi >= n_ohi:
                n_ohi = 0
                break
            n_ohi -= h_ohi
        ncc_assert(n_ohi == 0, "Could not reduce ohi count")
        # rebuild the ohi information
        self.ohi = self._build_ohi(None)

    def align_hv_bit(self, h):
        '''
        # XXX Enable this code if needed - not fully working
        # align hv bit to byte boundary if
        # - this is the first extracted header in a state and multiple headers are extracted
        # in that state
        if h not in self.hdr_ext_states:
            return False
        # XXX this is still not a correct solution (need to group the hv bits)
        for cs in self.hdr_ext_states[h]:
            if not cs.deparse_only and len(cs.headers) > 1 and h == cs.headers[0]:
                return True
        '''
        return False

    def assign_hv_bits(self):
        # this function is changed to just follow the hv allocation information created
        # while building each flit. This now only replaces the hv_bits in flit0 with actual names
        hv_location = self.be.hw_model['parser']['hv_location']
        max_hv_bits = self.be.hw_model['parser']['max_hv_bits']

        # fill it backwards as needed by deparser
        hv_bit = hv_location + max_hv_bits - 1
        hv_start_offset = self.be.hw_model['parser']['hv_start_offset']

        #Don't use leading HV bits which are meant to be used for PHV rewrite purposes
        #in deparser.
        hv_bit -= hv_start_offset

        hv_headers = copy.copy(self.headers)
        if self.d == xgress.INGRESS and self.be.pa.gress_pa[xgress.INGRESS].i2e_hdr:
            # ingress deparser needs to insert i2e header as expected by egress parser
            # It is assumed to be in this order intrinsic -> [tm_replication_data] -> i2e -> ..
            # tm_replication data is only seen by egress parser so insert i2e right after
            # intrinsic header
            # New: Add i2e header after p4_intr
            if self.be.pa.gress_pa[xgress.INGRESS].capri_p4_intr_hdr in hv_headers:
                hvidx = hv_headers.index(self.be.pa.gress_pa[xgress.INGRESS].capri_p4_intr_hdr)
            elif self.be.pa.gress_pa[xgress.INGRESS].capri_intr_hdr in hv_headers:
                hvidx = hv_headers.index(self.be.pa.gress_pa[xgress.INGRESS].capri_intr_hdr)
            else:
                hvidx = -1
            hv_headers.insert(hvidx+1, self.be.pa.gress_pa[xgress.INGRESS].i2e_hdr)

        hidx = hv_start_offset
        for _hidx in range(len(hv_headers)):
            h = hv_headers[_hidx]

            #For Deparser Csum to work correctly, CSUM HV bits should be co-located
            #with hdr valid bit.  Also HV bit for csum should be allocated before
            #header valid bit.

            if (self.be.checksum.IsHdrInCsumCompute(h.name, self.d) or
                self.be.checksum.IsHdrInCsumComputePhdr(h.name, self.d) or
                self.be.checksum.IsL2HdrInL2CompleteCsumCompute(h.name, self.d) or
                self.be.checksum.IsHdrInL2CompleteCsumCompute(h.name, self.d)):
                csum_hv_names = []
                if self.be.checksum.IsHdrInCsumCompute(h.name, self.d):
                    hfname = h.name + '.csum'
                    csum_hv_names.append(hfname)
                if not self.be.checksum.IsHdrInL2CompleteCsumCompute(h.name, self.d) and \
                   not self.be.checksum.IsHdrInPayLoadCsumCompute(h.name, self.d) and \
                   not self.be.checksum.IsHdrInOptionCsumCompute(h.name, self.d) and \
                   not self.be.checksum.IsPayloadCsumComputeWithNoPhdr(h.name, self.d):
                    hfname = h.name + '.tcp_csum'
                    csum_hv_names.append(hfname)
                    hfname = h.name + '.udp_csum'
                    csum_hv_names.append(hfname)
                    if self.be.checksum.L3HdrAsPseudoHdr(h.name, self.d) == 3:
                        hfname = h.name + '.icmp_csum'
                        csum_hv_names.append(hfname)
                    '''
                    #start HV bit at byte boundary so that parser-meta instuction
                    #can be used optimally.
                    if hidx % 8:
                        byte_align_hv_skip = 8 - (hidx % 8)
                        hidx += byte_align_hv_skip
                        hv_bit -= byte_align_hv_skip
                    '''
                if self.be.checksum.IsL2HdrInL2CompleteCsumCompute(h.name, self.d) \
                   or self.be.checksum.IsHdrInL2CompleteCsumCompute(h.name, self.d):
                    #allocates hv bit for ethernet.l2_csum
                    # or allocates p4_2_p4plus.l2_csum
                    hfname = h.name + '.l2csum'
                    csum_hv_names.append(hfname)
                #CSUM: Build csum hv bit dict
                csum_hv_bit_and_hf = []
                for hf_name in csum_hv_names:
                    csum_cf = self.be.pa.get_field(hf_name, self.d)
                    ncc_assert(csum_cf and cf.is_hv)
                    csum_cf.phv_bit = hv_bit
                    self.be.pa.replace_hv_field(hv_bit, csum_cf, self.d)
                    csum_hv_bit_and_hf.append((max_hv_bits - hidx - 1, hv_bit, hf_name))
                    self.hv_bit_header[max_hv_bits - hidx - 1] = h
                    hv_bit -= 1
                    hidx += 1
                self.csum_hdr_hv_bit[h] = csum_hv_bit_and_hf

            #Allocate HV bit for ICRC along with l3 header
            if self.be.icrc.IsHdrInIcrcCompute(h.name, self.d):
                '''
                if self.be.icrc.IsHdrRoceV2(h.name, self.d):
                    #start HV bit at byte boundary so that parser-meta instuction
                    #can be used optimally.
                    if hidx % 8:
                        byte_align_hv_skip = 8 - (hidx % 8)
                        hidx += byte_align_hv_skip
                        hv_bit -= byte_align_hv_skip
                '''
                icrc_hv_bit_and_hf = []
                hf_name = h.name + '.icrc'
                icrc_cf = self.be.pa.get_field(hf_name, self.d)
                icrc_cf.phv_bit = hv_bit
                self.be.pa.replace_hv_field(hv_bit, icrc_cf, self.d)
                icrc_hv_bit_and_hf.append((max_hv_bits - hidx - 1, hv_bit, hf_name))
                self.hv_bit_header[max_hv_bits - hidx - 1] = h
                hv_bit -= 1
                hidx += 1
                self.icrc_hdr_hv_bit[h] = icrc_hv_bit_and_hf

            #Allocate HV for header.valid
            hf_name = h.name + '.valid'
            cf = self.be.pa.get_field(hf_name, self.d)
            ncc_assert(cf and cf.is_hv)
            # point to copy of hv_bits in flit0
            hv_align_off = 0
            if self.align_hv_bit(h):
                if hv_bit % 8:
                    hv_align_off += 8 - (hv_bit % 8)
            hv_bit += hv_align_off
            hidx += hv_align_off
            cf.phv_bit = hv_bit
            self.be.pa.replace_hv_field(hv_bit, cf, self.d)
            self.hdr_hv_bit[h] = hv_bit
            self.hv_bit_header[max_hv_bits - hidx - 1] = h
            hv_bit -= 1
            hidx += 1


        #Assign special HV bits (Headers not seen by Parser)
        for name, hdr in self.be.h.p4_header_instances.items():
            if hdr.metadata:
                if 'deparser_variable_length_header' in hdr._parsed_pragmas:
                    hf_name = hdr.name + '.trunc'
                    cf = self.be.pa.get_field(hf_name, self.d)
                    ncc_assert(cf and cf.is_hv)
                    # point to copy of hv_bits in flit0
                    cf.phv_bit = self.be.hw_model['parser']['phv_pkt_trunc_location']
                    self.be.pa.replace_hv_field(cf.phv_bit, cf, self.d)
                    self.hdr_hv_bit[hdr] =  self.be.hw_model['parser']['hv_pkt_trunc_location']
                    _hvb = max_hv_bits - 1 - self.be.hw_model['parser']['hv_pkt_trunc_location']
                    self.hv_bit_header[_hvb] = None

                if 'deparser_pad_header' in hdr._parsed_pragmas:
                    hf_name = hdr.name + '.pad'
                    cf = self.be.pa.get_field(hf_name, self.d)
                    ncc_assert(cf and cf.is_hv)
                    # point to copy of hv_bits in flit0
                    cf.phv_bit = self.be.hw_model['parser']['phv_pad_hdr_location']
                    self.be.pa.replace_hv_field(cf.phv_bit, cf, self.d)
                    self.hdr_hv_bit[hdr] =  self.be.hw_model['parser']['hv_pad_hdr_location']
                    _hvb = max_hv_bits - 1 - self.be.hw_model['parser']['hv_pad_hdr_location']
                    self.hv_bit_header[_hvb] = None

        if not self.be.args.p4_plus:
            #Assign special payload len HV bits
            hf_name = 'capri_intrinsic' + '.payload'
            cf = self.be.pa.get_field(hf_name, self.d)
            ncc_assert(cf and cf.is_hv)
            # point to copy of hv_bits in flit0
            cf.phv_bit = self.be.hw_model['parser']['phv_pkt_len_location']
            self.be.pa.replace_hv_field(cf.phv_bit, cf, self.d)
            _hvb = max_hv_bits - 1 - self.be.hw_model['parser']['hv_pkt_len_location']
            self.hv_bit_header[_hvb] = None

    def assign_rw_phv_hv_bits(self):
        hv_location = self.be.hw_model['parser']['hv_location']
        max_hv_bits = self.be.hw_model['parser']['max_hv_bits']
        hv_bit = hv_location + max_hv_bits - 1
        hv_start_offset = self.be.hw_model['parser']['rw_phv_hv_start_offset']
        max_rw_phv_hv_bits = self.be.hw_model['parser']['max_rw_phv_hv_bits']
        hv_bit -= hv_start_offset
        hidx = hv_start_offset

        #Assign special HV bits (Headers used to rewrite PHV before 
        #deparser constructs packet. Case: GSO)
        for name, hdr in self.be.h.p4_header_instances.items():
            if hdr.metadata:
                if 'gso_csum_header' in hdr._parsed_pragmas and \
                    self.be.checksum.IsHdrInGsoCsumCompute(name, self.d):
                    hf_name = hdr.name + '.gso'
                    cf = self.be.pa.get_field(hf_name, self.d)
                    ncc_assert(cf and cf.is_hv)
                    ncc_assert(hidx < max_rw_phv_hv_bits)
                    # point to copy of hv_bits in flit0
                    cf.phv_bit = hv_bit
                    self.be.pa.replace_hv_field(cf.phv_bit, cf, self.d)
                    self.hdr_hv_bit[hdr] = hv_bit
                    self.hv_bit_header[max_hv_bits - hidx - 1] = None
                    hv_bit -= 1
                    hidx  += 1

    def assign_state_ids(self):
        # for now just assign sequential ids
        # optimization to save parser state can be done by assigning the same logical ids
        # to the same states etc. TBD
        for id, cs in enumerate(self.states):
            if cs.deparse_only:
                continue
            cs.id = id+1 # zero is not used - easy to debug

    def setup_saved_lkp(self):
        # Algorithm is not same as setup_local_regs anymore
        # register saving is still done based on topo sort. That means some saved regs will
        # be occupied on states where they are not used/needed.. but it is faster and we are
        # not yet running out of saved regs.
        # Also only lkp fields are moved to saved regs (not parser local vars that are used
        # for computation)
        reversed_topo_states = self.states[:]
        reversed_topo_states.reverse()
        for cs in reversed_topo_states:
            for b_on in cs.branch_on:
                if not isinstance(b_on, capri_field):
                    continue    # branch using current()
                hdr = b_on.get_p4_hdr()
                if not hdr.metadata and hdr not in cs.headers:
                    # saved lkp
                    if b_on not in self.saved_lkp_scope:
                        self.saved_lkp_scope[b_on] = _scope(None, cs)
                elif hdr.metadata and b_on.hfname not in cs.local_flds:
                    ncc_assert(0)

        for cf in self.saved_lkp_scope.keys():
            hdr = cf.get_p4_hdr()
            hdr_found = False
            for cs in self.states:
                if not hdr_found and hdr not in cs.headers:
                    continue
                # allow multiple states to load a given value in a saved register
                if hdr in cs.headers:
                    cs.load_saved_lkp.append(cf)
                    if hdr_found:
                        # also add to saved lkp list for states between scope start and end
                        cs.saved_lkp.append(cf)
                    else:
                        self.saved_lkp_scope[cf].start_cs = cs
                        hdr_found = True;
                else:
                    cs.saved_lkp.append(cf)

                if cs == self.saved_lkp_scope[cf].end_cs:
                    break

            self.logger.debug("%s:saved_lkp_scope %s: %s[%d] -> %s[%d]" % \
                                (self.d.name, cf.hfname,
                                 self.saved_lkp_scope[cf].start_cs.name,
                                 self.states.index(self.saved_lkp_scope[cf].start_cs),
                                 self.saved_lkp_scope[cf].end_cs.name,
                                 self.states.index(self.saved_lkp_scope[cf].end_cs)))

    def setup_local_regs(self):
        # path_states = sorted(self.path_states, key=lambda p: len(p), reverse=True)
        path_states = self.path_states
        # start with longest path
        # do data flow ananlysis per path and create in(down) and out(up) variables for each state
        # do not share in, out objects between states (multiple down stream states can clobber each
        # others data
        # ALGO: Register allocation is done by doing liveness calculation and creating
        # up(out)/down(in) variables for each state. Since a given parse state can be on multiple
        # paths, collect the preallocated register information while performing the liveness
        # analysis on each path.
        # Perform register allocation on each path top->bottom, allocate register for any
        # un-allocated variables
        # This code does not create RIG (register interference graph).. need to see if that is
        # a better way to handle this.
        # There is one case that is NOT covered as,
        # Path1: A-B-C-D, Path2: P-Q-R-S, Path3: A-B-R-S. In this case it is possible that
        # same varialble used on path1 and path2 will be assigned a different register and
        # will result in conflicting assignments on path3 (which may be processed later)
        # In this case program must be modified to create a common state between path1,2,3.

        cs_lfs = OrderedDict() # {cs: {in_lfs, out_lfs]}
        cs_all_reg_allocated = OrderedDict()
        n_states = len([s for s in self.states if s.name != '__END__' and not s.deparse_only \
            and not s.is_virtual])
        cs_covered = 0
        self.logger.info("%s:Build local variable in/out map" % (self.d.name))
        pnum = -1
        paths_traversed = 0
        # build in/out_lfs for all paths up-front
        for pnum,path in enumerate(path_states):
            out_lfs = OrderedDict() # {lf: reg}
            for cs in reversed(path):
                if cs.deparse_only or cs.name == '__END' or cs.is_virtual:
                    continue
                if cs not in cs_lfs:
                    cs_lfs[cs] = [OrderedDict(), OrderedDict()]

                in_lfs = cs_lfs[cs][0]
                # previous (downstream out_lf is now in_lf) copy it
                for lf,r in out_lfs.items():
                    if lf not in in_lfs:
                        in_lfs[lf] = r
                        ncc_assert(r == None)

                out_lfs = cs_lfs[cs][1]
                sol_lfs = []    # start of life for this variable
                for (lf,rd,wr) in cs.local_flds.values():
                    if lf in cs.saved_lkp:
                        # saved_lkp and load_saved_lkp are handled separately
                        continue
                    if wr and not rd:
                        sol_lfs.append(lf)
                    elif rd:
                        out_lfs[lf] = None
                    else:
                        ncc_assert(0, pdb.set_trace )# must be either wr or rd

                for lf in in_lfs.keys():
                    if lf not in out_lfs and lf not in sol_lfs and lf not in cs.load_saved_lkp:
                        # pass thru' variable
                        out_lfs[lf] = in_lfs[lf]

            if len(out_lfs):
                # unassigned/un-initialized lfs.. remove them
                for lf in out_lfs.keys():
                    self.logger.critical("%s:Un-initialized variable %s on path %s" % \
                        (self.d.name, lf.hfname, path))
                ncc_assert(0)

        for cs in self.states:
            if cs not in cs_lfs:
                continue
            self.logger.debug("%s:state %s:In LFs %s, Out LFs %s" % \
                            (self.d.name, cs.name, [(k.hfname,t) for k,t in cs_lfs[cs][0].items()],
                            [(k.hfname,t) for k,t in cs_lfs[cs][1].items()]))

        self.logger.info("%s:Start local variable allocation" % (self.d.name))
        for pnum,path in enumerate(path_states):
            # once all state are covered.. we can stop the loop, # of paths is of the order of
            # 250k for iris.p4 program
            if cs_covered == n_states:
                #pdb.set_trace()
                break;
            out_lfs = OrderedDict() # {lf: reg}
            # check if all states on this path are already covered
            path_cs_covered = 0
            for cs in path:
                if cs in cs_all_reg_allocated or cs.deparse_only or cs.name == '__END' or cs.is_virtual:
                    path_cs_covered += 1
                else:
                    self.logger.debug("%s:state %s not done on path[%d]" % \
                        (self.d.name, cs.name, pnum))
                    break
            if path_cs_covered == len(path):
                continue
            self.logger.debug("%s:Local variable allocation on path[%d]" % \
                (self.d.name, pnum))
            paths_traversed += 1
            path_lfs = OrderedDict()
            for cs in reversed(path):
                if cs.deparse_only or cs.name == '__END' or cs.is_virtual:
                    continue

                in_lfs = cs_lfs[cs][0]
                # previous (downstream out_lf is now in_lf) copy it
                for lf,r in out_lfs.items():
                    if lf in in_lfs:
                        ncc_assert(in_lfs[lf] == None or r == None or in_lfs[lf] == r)
                        pass
                    else:
                        in_lfs[lf] = r
                        if r != None:
                            cs.active_reg_alloc(lf, r)
                out_lfs = cs_lfs[cs][1]
                sol_lfs = []    # start of life for this variable
                # Don't allocate new.. just collect previously allocated
                for rid,lf in enumerate(cs.active_lkp_lfs):
                    if lf != None:
                        if lf not in path_lfs:
                            path_lfs[lf] = rid

                for (lf,rd,wr) in cs.local_flds.values():
                    if lf in cs.saved_lkp:
                        # saved_lkp and load_saved_lkp are handled separately
                        continue
                    if lf in in_lfs:
                        rid = in_lfs[lf]
                    else:
                        rid = cs.active_reg_find(lf)

                    if wr and not rd:
                        sol_lfs.append(lf)
                        if lf not in path_lfs and rid != None:
                            path_lfs[lf] = rid
                    elif rd:
                        out_lfs[lf] = rid
                        if lf not in path_lfs and rid != None:
                            path_lfs[lf] = rid
                    else:
                        ncc_assert(0, pdb.set_trace )# must be either wr or rd

                for lf in in_lfs.keys():
                    if lf not in out_lfs and lf not in sol_lfs and lf not in cs.load_saved_lkp:
                        # pass thru' variable
                        out_lfs[lf] = in_lfs[lf]

            if len(out_lfs):
                # unassigned/un-initialized lfs.. remove them
                for lf in out_lfs.keys():
                    self.logger.critical("%s:Un-initialized variable %s on path %s" % \
                        (self.d.name, lf.hfname, path))
                ncc_assert(0)
                pass

            # allocate reg ids for new lfs
            upstream_lfs = OrderedDict() #out_lfs
            for cs in path:
                if cs.deparse_only:
                    continue
                downstream_lfs = cs_lfs[cs][0]
                # take the reg assignments from upstream node for local and downstream lfs
                for lf,r in upstream_lfs.items():
                    if r == None:
                        ncc_assert(0)
                        continue
                    if lf in downstream_lfs or lf.hfname in cs.local_flds:
                        r_used = cs.active_reg_find(lf)
                        if r_used == None:
                            cs.active_reg_alloc(lf, r)
                        else:
                            ncc_assert(r_used == r)
                            pass
                        if lf in downstream_lfs:
                            downstream_lfs[lf] = r

                # reuse the rids that are allocated upstream but were EOL before reaching
                # this state
                for lf,rd,wr in cs.local_flds.values():
                    if lf in cs.saved_lkp:
                        continue
                    if lf in upstream_lfs:
                        continue
                    if lf in path_lfs:
                        rid = path_lfs[lf]
                        ncc_assert(rid != None)
                        self.logger.debug("%s:%s Reuse rid %d for LF %s" % \
                                (self.d.name, cs.name, rid, lf.hfname))
                        _ = cs.active_reg_alloc(lf, rid)

                for lf,rd,wr in cs.local_flds.values():
                    if lf in cs.saved_lkp:
                        continue

                    if lf in upstream_lfs:
                        # reg should come from top, if not ignore
                        rid = upstream_lfs[lf]
                        if rid == None:
                            continue
                        if not lf.no_register():    # ??? check, why no_reg lfs are reported up
                            _ = cs.active_reg_alloc(lf, rid)
                    else:
                        rid = cs.active_reg_find(lf)
                        if rid == None:
                            # allocate only if downstream needs it or locally used for lkp
                            if not lf.no_register() and \
                                (lf in downstream_lfs or lf in cs.branch_on or \
                                 lf in cs.load_saved_lkp):
                                if lf in path_lfs:
                                    rid = path_lfs[lf]
                                    if rid:
                                        self.logger.debug("%s:%s Reuse rid %d for LF %s" % \
                                            (self.d.name, cs.name, rid, lf.hfname))
                                if rid == None:
                                    rid = cs.active_reg_alloc(lf)
                                    if lf not in path_lfs:
                                        path_lfs[lf] = rid
                                else:
                                    # reserve
                                    _ = cs.active_reg_alloc(lf, rid)

                    if rid != None and lf not in cs.reserved_lfs:
                        cs.reserved_lfs.append(lf)

                    if lf in downstream_lfs:
                        downstream_lfs[lf] = rid

                for lf,r in downstream_lfs.items()[:]:
                    if r == None:
                        #self.logger.debug("%s:%s Prune LF %s" % \
                        #    (self.d.name, cs.name, lf.hfname))
                        del downstream_lfs[lf]
                    else:
                        # Rest of the code uses cs.reserved_lfs list.. update it
                        if lf not in cs.reserved_lfs:
                            cs.reserved_lfs.append(lf)

                # see if registers for all local flds of this state are allocated
                if cs not in cs_all_reg_allocated:
                    reg_allocated = 0
                    for lf,rd,wr in cs.local_flds.values():
                        if cs.active_reg_find(lf) == None and not lf.no_register():
                            continue
                        reg_allocated += 1

                    if reg_allocated == len(cs.local_flds.keys()):
                        cs_all_reg_allocated[cs] = 1
                        cs_covered += 1
                        self.logger.debug("%s:%s LF allocation complete %d/%d" % \
                            (self.d.name, cs.name, cs_covered, n_states))

                upstream_lfs = OrderedDict()
                upstream_lfs = copy.copy(downstream_lfs)

        #pdb.set_trace()
        self.logger.info( \
            "%s:Done local variable allocation (paths traversed %d) states covered %d/%d" % \
            (self.d.name, paths_traversed, cs_covered, n_states))

    def saved_lkp_reg_find(self, lf):
        if lf in self.saved_lkp_fld:
            saved_reg = self.saved_lkp_fld[lf]
            for i, sr in enumerate(self.saved_lkp_reg_allocation):
                if sr == saved_reg:
                    return i, sr
        ncc_assert(0)
        return None, None

    def saved_lkp_reg_alloc(self, lf, rid):
        self.logger.debug("%s:saved_lkp_reg_alloc(), lf= %s, rid= %d" % \
            (self.d.name, lf.hfname, rid))
        ncc_assert(lf not in self.saved_lkp_fld)
        ncc_assert(self.saved_lkp_reg_allocation[rid] == None)
        lkp_reg = _saved_lkp_reg(lf)
        self.saved_lkp_fld[lf] = lkp_reg
        self.saved_lkp_reg_allocation[rid] = lkp_reg

    def saved_lkp_reg_free(self, lf):
        i, sr = self.saved_lkp_reg_find(lf)
        self.saved_lkp_fld[lf] = None
        # check if other flds sharing this reg are still active
        for saved_reg in self.saved_lkp_fld.values():
            if saved_reg == sr:
                self.logger.debug("%s:saved_lkp_reg_free() still in use! lf= %s" % \
                            (self.d.name, lf.hfname))
                return i
        # no other fld is active, free it
        self.logger.debug("%s:saved_lkp_reg_free(), lf= %s, rid= %d" % \
            (self.d.name, lf.hfname, i))
        self.saved_lkp_reg_allocation[i] = None
        return i

    def reserve_lkp_regs(self, cs):
        # Reserve all the lkp_registers that are dictated by downstream reservations
        # 1. local_flds
        # 2. saved_lkp (if field is used for lookup)
        # 3. Then allocate registers to save future lkp.
        # After all reservations are done, registers for for lookups in current state
        # are allocated in program_capri_state()
        for lf in cs.reserved_lfs:
            reg_type = lkp_reg_type.LKP_REG_RETAIN
            if lf.hfname in cs.local_flds:
                lf,r,w = cs.local_flds[lf.hfname]
                if w:
                    reg_type = lkp_reg_type.LKP_REG_LOAD
            rid = cs.active_reg_find(lf)
            if rid == None and lf.no_register():
                continue

            ncc_assert(rid != None)

            if not lf.is_meta:
                cs.lkp_regs[rid].first_pkt_fld = lf

            cs.lkp_regs[rid].type = reg_type
            # create lkp_fld - used for l2p key mapping
            lkp_fld = capri_lkp_fld(lf, lkp_fld_type.FLD_LOCAL)
            lkp_fld.reg_id = rid
            cs.lkp_flds[lf.hfname] = lkp_fld

            if lf.is_meta:
                roff = cs.lkp_regs[rid].size - lf.width
                self.logger.debug("%s:roff for REG fld %s is set to %d" % \
                    (cs.name, lf.hfname, roff))
            else:
                roff = (lf.p4_fld.offset % 8)

            cs.lkp_regs[rid].flds[roff] = (lkp_fld, 0, lf.width)

        for cf in cs.saved_lkp:
            if cf not in cs.branch_on:
                # lkp field is not used in this state, pass-thru
                continue
            rid, sr = self.saved_lkp_reg_find(cf)
            ncc_assert(cs.lkp_regs[rid].type == lkp_reg_type.LKP_REG_NONE, \
                "Lkp register is double-booked")
            if cf in cs.load_saved_lkp:
                cs.lkp_regs[rid].type = lkp_reg_type.LKP_REG_LOAD
                cs.lkp_regs[rid].store_en = True
                continue

            cs.lkp_regs[rid].type = lkp_reg_type.LKP_REG_STORED
            # create lkp_fld - used for l2p key mapping
            lkp_fld = capri_lkp_fld(cf, lkp_fld_type.FLD_SAVED_REG)
            lkp_fld.reg_id = rid

            cs.lkp_flds[cf.hfname] = lkp_fld
            if cf.is_meta:
                roff = cs.lkp_regs[rid].size - cf.width
                self.logger.debug("%s:roff for REG fld %s is set to %d" % \
                    (cs.name, cf.hfname, roff))
            else:
                roff = (cf.p4_fld.offset % 8)
            cs.lkp_regs[rid].flds[roff] = (lkp_fld, 0, cf.width)

        for sf in cs.load_saved_lkp:
            # allow multiple states to save the same value. This happens when a given
            # header is extracted in multiple states
            if sf in cs.saved_lkp:
                rid, _ = self.saved_lkp_reg_find(sf)
            else:
                rid = cs.active_reg_alloc_for_saved_lkp(sf)
                self.saved_lkp_reg_alloc(sf, rid)
            cs.lkp_regs[rid].type = lkp_reg_type.LKP_REG_LOAD
            cs.lkp_regs[rid].store_en = True
            # create lkp_fld - used for l2p key mapping
            if sf.is_meta:
                lkp_fld = capri_lkp_fld(sf, lkp_fld_type.FLD_LOCAL)
            else:
                lkp_fld = capri_lkp_fld(sf, lkp_fld_type.FLD_PKT)
                cs.lkp_regs[rid].first_pkt_fld = sf
            lkp_fld.reg_id = rid
            cs.lkp_flds[sf.hfname] = lkp_fld
            if sf.is_meta:
                roff = cs.lkp_regs[rid].size - sf.width
                self.logger.debug("%s:roff for REG fld %s is set to %d" % \
                    (cs.name, sf.hfname, roff))
            else:
                roff = (sf.p4_fld.offset % 8)
            cs.lkp_regs[rid].flds[roff] = (lkp_fld, 0, sf.width)

    def process_state_set_ops(self, cs):
        # assign lkp registers to set_metadata operations
        for op in cs.set_ops:
            self.logger.debug("set_metadata: %s" % op)

            if op.op_type == meta_op.EXTRACT_FIELD:
                ncc_assert(op.src1)
                pass
            elif op.op_type == meta_op.EXTRACT_REG:
                # meta instruction - find lkp_reg holding this local variable
                cf = op.src_reg
                rid = cs.active_reg_find(cf)
                ncc_assert(rid != None, "Invalid src reg  %s for EXTRACT_REG" % (cf.hfname))
                op.rid = rid
            elif op.op_type == meta_op.EXTRACT_META:
                # meta instruction - phv <- expr(pkt_data) or parser_local (reg)
                cf = op.src1
                if cf.parser_local:
                    rid = cs.active_reg_find(cf)
                    ncc_assert(rid != None, "Invalid src %s for EXTRACT_META" % (cf.hfname))
                    op.rid = rid
            elif op.op_type == meta_op.EXTRACT_CONST:
                # meta instruction - handled by output generation
                pass
            elif op.op_type == meta_op.LOAD_REG:
                # find lkp_reg, set capri-expression
                # allocation is done up-front
                # create lkp_reg object for this state
                if op.dst in cs.reserved_lfs:
                    rid = cs.active_reg_find(op.dst)
                    ncc_assert(rid != None)
                    # parser local variables can be updated (reloaded)
                    if cs.lkp_regs[rid].type == lkp_reg_type.LKP_REG_RETAIN:
                        self.logger.debug("%s:%s:Reg %d changed from RETAIN to LOAD- %s" % \
                            (self.d.name, cs.name, rid, 
                             op.capri_expr if op.capri_expr else op.src1))

                        cs.lkp_regs[rid].type = lkp_reg_type.LKP_REG_LOAD

                    if op.capri_expr:
                        cs.lkp_regs[rid].capri_expr = op.capri_expr
                    elif isinstance(op.src1, capri_field):
                        cs.lkp_regs[rid].first_pkt_fld = op.src1
                    else:
                        cs.lkp_regs[rid].first_pkt_fld = None
                else:
                    self.logger.warning("%s:%s:Removed unused field %s" % \
                        (self.d.name, cs.name, op.dst.hfname))

            elif op.op_type == meta_op.UPDATE_REG:
                # allocate/find lkp_reg, set capri-expression
                # allocation is done up-front
                rid = cs.active_reg_find(op.dst)
                ncc_assert(rid != None)
                cs.lkp_regs[rid].type = lkp_reg_type.LKP_REG_UPDATE
                cs.lkp_regs[rid].capri_expr = op.capri_expr
                if op.capri_expr:
                    cs.lkp_regs[rid].capri_expr = op.capri_expr
                elif isinstance(op.src1, capri_field):
                    cs.lkp_regs[rid].first_pkt_fld = op.src1
                else:
                    cs.lkp_regs[rid].first_pkt_fld = None
            elif op.op_type == meta_op.EXTRACT_CURRENT_OFF:
                pass
            else:
                ncc_assert(0)

    def split_one_state(self, cs, split_off):
        new_cs = capri_parse_state(self, None, cs.name + '__split')
        split_cf = None
        # adjust the split_offset to a field boundary
        for cf in cs.extracted_fields:
            if ((cs.fld_off[cf] + cf.width - 1)/8) > split_off:
                if (cs.fld_off[cf]/8) < split_off:
                    split_cf = cf
                break
            # field below split
            split_cf = cf

        split_off = cs.fld_off[split_cf] / 8
        ncc_assert(split_cf)
        split_idx = cs.extracted_fields.index(split_cf)
        for i in range(split_idx, len(cs.extracted_fields)):
            new_cs.extracted_fields.append(cs.extracted_fields[i])

        for cf in new_cs.extracted_fields:
            cs.extracted_fields.remove(cf)

        split_reg_found = False
        for r,lkp_reg in enumerate(cs.lkp_regs):
            # XXX Need a test case for saved lkp_regs and reg loading on split state
            if lkp_reg.pkt_off == -1:   # covers cases of NONE, RETAIN, STORED regs
                new_cs.lkp_regs[r] = copy.copy(lkp_reg)
                continue
            if not split_reg_found:
                if (lkp_reg.pkt_off/8) < split_off and ((lkp_reg.pkt_off + 16)/8) <= split_off:
                    # since the register is loaded in the first half of the split state
                    # add it to new state to RETAIN the value
                    new_cs.lkp_regs[r].type = lkp_reg_type.LKP_REG_RETAIN
                    continue

                if (lkp_reg.pkt_off/8) < split_off:
                    # adjust split offset (field should be ok)
                    split_off = lkp_reg.pkt_off/8

                split_reg_found = True

            # split register is found, add to new state and reset it from parent
            new_cs.lkp_regs[r] = copy.copy(lkp_reg)
            new_cs.lkp_regs[r].pkt_off -= (split_off * 8)
            # no need to update lkp flds within lkp reg ?? used for debug printing
            new_cs.lkp_flds = cs.lkp_flds
            cs.lkp_flds = OrderedDict()
            lkp_reg.reset()

        for op in cs.set_ops:
            if op.op_type == meta_op.EXTRACT_REG or \
                op.op_type == meta_op.EXTRACT_META or \
                op.op_type == meta_op.EXTRACT_CURRENT_OFF or \
                op.op_type == meta_op.EXTRACT_CONST:
                continue

            off = 0
            if op.src1:
                hdr = op.src1.get_p4_hdr()
                off = cs.fld_off[op.src1]
            elif op.capri_expr and op.capri_expr.src1:
                hdr = op.capri_expr.src1.get_p4_hdr()
                off = cs.fld_off[op.capri_expr.src1]
            if (off/8) < split_off:
                continue
            new_cs.set_ops.append(op)

        #XXX fix fld offsets in new state
        for op in new_cs.set_ops:
            cs.set_ops.remove(op)

        # move branch_on/branch_to - not used for programming - just for sanity and debug o/p
        new_cs.branch_on = copy.copy(cs.branch_on)
        cs.branch_on = []
        new_cs.branch_to = copy.copy(cs.branch_to)
        cs.branch_to = OrderedDict()
        # add a default branch from cs->new_cs
        cs.branch_to[p4.P4_DEFAULT] = new_cs

        # branches[] is used by capri_output
        new_cs.key_len = cs.key_len
        new_cs.branches = copy.copy(cs.branches)
        cs.branches = []
        br_info = capri_branch_info()
        br_info.val = 0
        br_info.mask = [0 for i in range(cs.key_len)]
        br_info.nxt_state = new_cs
        cs.branches.append(br_info)
        cs.key_len = 0      # no lookup on first state

        # adjust extract_len
        ncc_assert(isinstance(cs.extract_len, int))
        new_cs.extract_len = cs.extract_len - split_off
        cs.extract_len = split_off
        # rebuild the fld offset dictionary
        cs.fld_off = OrderedDict()
        s_off = 0
        for cf in cs.extracted_fields:
            cs.fld_off[cf] = s_off
            s_off += cf.width

        s_off = 0
        for cf in new_cs.extracted_fields:
            new_cs.fld_off[cf] = s_off
            s_off += cf.width

        # rebuild headers info for each state
        for cf in new_cs.extracted_fields:
            hdr = cf.get_p4_hdr()
            if hdr not in new_cs.headers:
                new_cs.headers.append(hdr)
        cs.headers = []
        for cf in cs.extracted_fields:
            hdr = cf.get_p4_hdr()
            if hdr not in cs.headers:
                cs.headers.append(hdr)

        # fix OHI slots
        if len(new_cs.extracted_fields):
            cf = new_cs.extracted_fields[0]
            # no need to fix ohi slot if first field is not ohi
            if cf.is_ohi:
                hdr = cf.get_p4_hdr()
                for i, ohi in enumerate(self.ohi[hdr]):
                    if ohi.start == (cf.p4_fld.offset/8):
                        break # ohi chunk start with this field
                    if ohi.start > ((cf.p4_fld.offset + cf.width) / 8):
                        break
                    if (ohi.start+ohi.length) <= (cf.p4_fld.offset/8):
                        continue
                    # a cf cannot straddle ohi chunk
                    ncc_assert(ohi.start < (cf.p4_fld.offset/8))
                    max_ohi = self.be.hw_model['parser']['num_ohi']
                    ncc_assert(self.ohi_used < max_ohi)
                    self.logger.info("Allocate new ohi slots for %s %s" % (new_cs.name, cf.hfname))
                    new_ohi = capri_ohi(cf.p4_fld.offset/8,
                                        ohi.start + ohi.length - (cf.p4_fld.offset / 8))
                    new_ohi.id = self.ohi_used
                    self.ohi[hdr].insert(i+1, new_ohi)
                    self.ohi_used += 1
                    ohi.length = (cf.p4_fld.offset/8) - ohi.start
                    break

        new_cs.local_flds = cs.local_flds # FIX
        # XXX meta_extracted_fields not needed any more ??? remove?
        new_cs.meta_extracted_fields = cs.meta_extracted_fields
        # utility flags/variables
        new_cs.unrolled_state = cs.unrolled_state
        new_cs.unrolled_idx = cs.unrolled_idx
        new_cs.internal_state = cs.internal_state
        new_cs.is_virtual = cs.is_virtual
        new_cs.is_end = False
        cs.is_end = False

        new_cs.max_extract_len = new_cs.extract_len
        cs.max_extract_len = cs.extract_len
        new_cs.reserved_lfs = cs.reserved_lfs   # FIX

        # hardware information
        new_cs.hw_lkp_size = cs.hw_lkp_size
        new_cs.key_l2p_map = copy.copy(cs.key_l2p_map)
        cs.key_l2p_map = None
        cs.is_split = True
        new_cs.is_split = True
        return new_cs

    def split_states(self, cs, add_cs):
        # call generate_extract_instructions()
        # get split_offset from the insts[16]
        # rest of the code is based on split_offset
        if cs.variable_hdr or (add_cs and add_cs.variable_hdr):
            self.logger.debug( \
                "No split for %s, %s due to variable len header extraction (goes to OHI)" %\
                (cs.name, add_cs.name if add_cs else None))
            return

        num_extracts = self.be.hw_model['parser']['num_extracts']
        max_extract = self.be.hw_model['parser']['max_extract']
        total_extract = (add_cs.extract_len if add_cs else 0) + cs.extract_len
        insts = self.generate_extract_instructions(cs, add_cs)

        # No need to check extract_len, if number of extract inst are less than
        # allowed we are within the budget. This can happen when headers go to ohi
        # and therefore do not consume extract instructions
        if len(insts) <= num_extracts:
            return

        split_off = 0
        '''
        if len(insts) > num_extracts:
        '''
        # find the inst that takes over the limit
        for i, inst in enumerate(insts):
            # check if reached extraction limit
            if (i+1) == num_extracts:
                split_off = inst[0]
                break
            # check inst crosses parser limit
            if inst[0] + inst[1] > max_extract:
                split_off = inst[0]
                break
        if not split_off:
            # this split is due to extract instuction limit, not len
            # use the offset from last instruction
            split_off = insts[num_extracts-1][0]
        '''
        else:
            # split is due to extract len limit
            split_off = max_extract
        '''
        ncc_assert(split_off)
        # XXX Now no need to split state if ohi extraction crosses 64B (hw allows upto 14b
        # increment to offset - added to skip udp payload
        self.logger.warning("%s:Splitting states %s, %s, split_off %d" % \
            (self.d.name, cs.name, add_cs.name if add_cs else None, split_off))
        split_cf = None
        if add_cs and split_off < add_cs.extract_len:
            pdb.set_trace() # XXX rare - catch when it happens
            new_cs = self.split_one_state(add_cs, split_off)
            cs_idx = self.states.index(add_cs)
            self.states.insert(cs_idx+1, new_cs)
            # combine with the current state
            self.split_states(new_cs, cs)
        else:
            if add_cs:
                split_off -= add_cs.extract_len
            new_cs = self.split_one_state(cs, split_off)
            cs_idx = self.states.index(cs)
            self.states.insert(cs_idx+1, new_cs)
            # recursively split until it fits
            self.split_states(new_cs, None)
        return

    def split_large_states(self):
        # A state needs to be split into smaller states due to several reasons-
        # - extractions instructions > 64 due to large headers and/or metadata extraction
        # - variable-len header whose max size can exceed 64B (Ok if OHI?)
        # - num ohi slots when multiple headers are extracted
        # - parser flit crossing (this is currently handled when while doing phv allocation)
        # XXX currently only checks done are for the extraction size rest TBD
        max_extract = self.be.hw_model['parser']['max_extract']
        second_states = [bi.nxt_state for bi in self.start_state.branches]
        for cs in self.states[:]:
            if cs == self.start_state or cs.deparse_only:
                continue
            # breakup the state (recursive)
            if cs in second_states:
                self.split_states(cs, self.start_state)
            else:
                self.split_states(cs, None)

    def program_capri_states(self):
        # XXX Add internal states to handle any hw headers and intrinsic metadata to the front
        # Create and add END state
        # replace all the branches to P4_table to END state while programming the h/w
        end_state = capri_parse_state(self, None, "__END__")
        end_state.is_end = True
        self.states.append(end_state)
        # init branch_to dictionaries to point to end state
        # branch to deparse_only state is also replaced with END so these will not be programmed
        for cs in self.states:
            for bkey,bv in cs.branch_to.items():
                if not isinstance(bv, capri_parse_state) or \
                    bv.deparse_only:
                    cs.branch_to[bkey] = end_state
                    if len(cs.branch_to) == 1:
                        self.logger.debug("%s:%s Marked as END" % (self.d.name, cs.name))
                        cs.is_end = True

        self.setup_saved_lkp()
        self.setup_local_regs()

        for id, cs in enumerate(self.states):   #{
            if cs.deparse_only:
                continue
            s_off = 0
            for cf in cs.extracted_fields:
                cs.fld_off[cf] = s_off
                s_off += cf.width

            self.reserve_lkp_regs(cs)

            # process fields used for branching in this state
            koff = 0
            for b_on in cs.branch_on:
                if isinstance(b_on, capri_field):
                    hdr = b_on.get_p4_hdr()
                    if hdr.metadata:
                        ncc_assert(b_on.parser_local, \
                            "Metadata fld %s used for lookup must be parser_local" % \
                            (b_on.hfname))
                        lkp_fld = cs.lkp_flds[b_on.hfname]
                        lkp_fld.key_off = koff
                        lkp_fld.is_key = True
                        koff += b_on.width
                        continue
                    elif hdr not in cs.headers:
                        # use saved lookup - must be the same lkp register that was
                        # used earlier state to save the value.. also align it on 16b
                        ncc_assert(b_on in cs.saved_lkp, "%s:saved lookup error for %s in %s" % \
                            (self.d.name, b_on.hfname, cs.name))
                        lkp_fld = cs.lkp_flds[b_on.hfname]
                        lkp_fld.key_off = koff
                        lkp_fld.is_key = True
                        koff += b_on.width
                        continue

                    # normal packet field
                    off = cs.fld_off[b_on]
                    l = b_on.width
                    ncc_assert(l != p4.P4_AUTO_WIDTH, "Cannot support variable len match field")
                    lkp_fld = capri_lkp_fld(b_on, lkp_fld_type.FLD_PKT, off, l)
                    lkp_fld.is_key = True
                    lkp_fld.key_off = koff
                    cs.lkp_flds[b_on.hfname] = lkp_fld
                    koff += b_on.width
                elif isinstance(b_on, tuple):
                    off = b_on[0]
                    l = b_on[1]
                    fld_name = '_current.%d_%d' % (off, l)
                    lkp_fld = capri_lkp_fld(None, lkp_fld_type.FLD_PKT, off, l)
                    lkp_fld.is_key = True
                    lkp_fld.key_off = koff
                    cs.lkp_flds[fld_name] = lkp_fld
                    koff += l
                else:
                    pdb.set_trace()
                    pass

            cs.key_len = koff
            self.logger.debug("State: %s: key_size %d" % (cs.name, cs.key_len))

            self.process_state_set_ops(cs)

            found_default = False
            # Initialize branch values and associated masks
            for k,v in cs.branch_to.items():
                # maintain user provided order
                br_info = capri_branch_info()
                ncc_assert(isinstance(v, capri_parse_state))
                br_info.nxt_state = v
                if isinstance(k, int):
                    br_info.val = k
                    br_info.mask = [1 for i in range(cs.key_len)]
                elif isinstance(k, tuple):
                    br_info.val = k[0]
                    br_info.mask = []
                    for i in range(cs.key_len):
                        if k[1] & (1<<i):
                            br_info.mask.insert(0,1)
                        else:
                            br_info.mask.insert(0,0)
                else:
                    # P4_DEFAULT
                    br_info.val = 0
                    br_info.mask = [0 for i in range(cs.key_len)]
                    found_default = True

                cs.branches.append(br_info)

            if not found_default and not cs.is_end:
                self.logger.warning("%s:%s has no DEFAULT transition" % \
                    (self.d.name, cs.name))

            # map key to hw registers
            # allocate lkp_registers for the keys in pkt_offset order
            # pkt_offset order is needed for collecting pkt flds < 16b and combining
            # them into a single lkp_reg
            pkt_lkp_fld_names = [k for k in cs.lkp_flds.keys() if \
                                    cs.lkp_flds[k].src_type == lkp_fld_type.FLD_PKT]
            sorted_lkp = sorted(pkt_lkp_fld_names, key=lambda k: cs.lkp_flds[k].pkt_off)
            reg_in_use = None
            for lkp_name in sorted_lkp:
                lkp_fld = cs.lkp_flds[lkp_name]
                if lkp_fld.reg_id != -1:
                    # register is already allocated (could be due to load_saved_lkp)
                    continue
                off = lkp_fld.pkt_off
                l = lkp_fld.len
                while l:
                    # find reg in use
                    if not reg_in_use:
                        rid = cs.lkp_reg_alloc()
                        if rid == None:
                            break
                        reg_in_use = cs.lkp_regs[rid]
                        reg_in_use.type = lkp_reg_type.LKP_REG_PKT
                        reg_in_use.pkt_off = (off/8) * 8

                    if reg_in_use.pkt_off <= off and off < (reg_in_use.pkt_off + reg_in_use.size):
                        avail = (reg_in_use.pkt_off+reg_in_use.size) - off
                        used = avail if avail < l else l
                        reg_in_use.flds[off - reg_in_use.pkt_off] = \
                            (lkp_fld, off-lkp_fld.pkt_off, used)
                        l -= used
                        off += used
                        avail -= used
                        if avail == 0:
                            reg_in_use = None
                    else:
                        reg_in_use = None

                ncc_assert(l==0, "Not enough registers for lkp_val")

            # fix pkt_off for all lkp_regs that load from pkt
            for lkp_reg in cs.lkp_regs:
                if lkp_reg.type == lkp_reg_type.LKP_REG_NONE or \
                    lkp_reg.type == lkp_reg_type.LKP_REG_STORED or \
                    lkp_reg.type == lkp_reg_type.LKP_REG_RETAIN or \
                    lkp_reg.type == lkp_reg_type.LKP_REG_PKT:
                    continue

                if lkp_reg.capri_expr:
                    if lkp_reg.capri_expr.src_reg and \
                            (not isinstance(lkp_reg.capri_expr.src_reg, capri_field) or \
                             not lkp_reg.capri_expr.src_reg.is_meta):
                        # pkt fld used as src2
                        if isinstance(lkp_reg.capri_expr.src_reg, tuple):
                            off = lkp_reg.capri_expr.src_reg[0]
                        else:
                            ncc_assert(lkp_reg.capri_expr.src_reg in cs.fld_off)
                            off = cs.fld_off[lkp_reg.capri_expr.src_reg]
                        lkp_reg.pkt_off2 = off

                    if lkp_reg.capri_expr.src1 and \
                            (not isinstance(lkp_reg.capri_expr.src_reg, capri_field) or \
                             not lkp_reg.capri_expr.src1.is_meta):
                        # pkt fld used as src1 - could be tuple
                        if isinstance(lkp_reg.capri_expr.src1, tuple):
                            off = lkp_reg.capri_expr.src1[0]
                        else:
                            ncc_assert(lkp_reg.capri_expr.src1 in cs.fld_off)
                            off = cs.fld_off[lkp_reg.capri_expr.src1]
                        lkp_reg.pkt_off = off
                else:
                    # local var load from pkt
                    if lkp_reg.first_pkt_fld:
                        off = cs.fld_off[lkp_reg.first_pkt_fld]
                        lkp_reg.pkt_off = off

            # create logical->physical key bit mapping based on how key flds were placed
            # into lkp registers
            # phys key comprises of registers key_val0-2..=>48 bits
            # p4 key bits are mapped to bits in phys key. Due to 16bit reg size, some
            # bits in phv key can be un-used, mask bits for those are set to 0 to ignore those)
            self.logger.debug("%s:%s Build l2p keymap size %d" % (self.d.name, cs.name, cs.key_len))
            cs.key_l2p_map = [-1 for i in range(cs.key_len)]
            phys_off = 0
            for r,reg in enumerate(cs.lkp_regs):
                for roff, t in reg.flds.items():
                    if not t[0].is_key:
                        self.logger.debug("%s:%s Reg %d not used for lookup" % \
                            (self.d.name, cs.name, r))
                        continue
                    koff = t[0].key_off + t[1] # bit pos in logical key
                    klen = t[2]
                    self.logger.debug("%s:%s Create key map for %s koff %d size %d phys_off %d roff %d" % \
                        (self.d.name, cs.name, t[0].hfname, koff, klen, phys_off, roff))
                    for k in range(klen):
                        cs.key_l2p_map[koff+k] = phys_off+roff+k
                phys_off += reg.size

            ncc_assert(cs.key_l2p_map.count(-1) == 0)
            # "Internal Error unmapped lkp bits"

            self.logger.debug("State LKP_FLDS: %s: lkp_flds %s" % (cs.name, cs.lkp_flds.values()))
            self.logger.debug("State LKP_REGS: %s: lkp_regs %s" % (cs.name, cs.lkp_regs))

            # compute no. of bytes extracted in this state
            cs.max_extract_len = sum([get_header_max_size(h) for h in cs.headers])
            if not cs.variable_hdr:
                cs.extract_len = sum([self.get_header_size(h) for h in cs.headers])
            else:
                # only last header can be variable len. More than 1 variable len headers
                # cannot be extracted in a state -
                # cannot specify offset inst with multiple pkt fields
                num_var_hdrs = sum([1 if header_is_variable_len(h) else 0 for h in cs.headers])
                ncc_assert(num_var_hdrs == 1, "Too many var len headers extracted in state %s" % \
                    cs.name)
                ncc_assert(header_is_variable_len(cs.headers[-1]), "Only last header can be variable")
                if len(cs.headers) > 1:
                    # fixed size of the last (variable len) hdr needs to be subtracted.
                    fixed_size = sum([get_header_fixed_size(h) for h in cs.headers])
                    v_fixed_size = get_header_fixed_size(cs.headers[-1])
                    fixed_size -= v_fixed_size # signed value
                else:
                    fixed_size = 0
                v_size_exp = self.get_header_size(cs.headers[-1])

                if isinstance(v_size_exp, p4.p4_expression):
                    cs.extract_len = capri_parser_expr(cs.parser, v_size_exp)
                else:
                    ncc_assert(isinstance(v_size_exp, str))
                    # no real expression, convert str to p4_field
                    capri_expr = capri_parser_expr(cs.parser, None)
                    hfname = v_size_exp
                    cf = cs.parser.be.pa.get_field(hfname, cs.parser.d)
                    if not cf:
                        p4_fld = cs.parser.be.h.p4_fields[hfname]
                        cf = self.be.pa.allocate_field(p4_fld, cs.parser.d)
                    if cf.is_meta:
                        capri_expr.src_reg = cf
                        capri_expr.op2 = '+'
                    else:
                        capri_expr.src1 = cf
                        capri_expr.op1 = '<<'
                        capri_expr.shft = 0
                    cs.extract_len = capri_expr

                if fixed_size:
                    cs.extract_len.add_signed_const(fixed_size)

            self.logger.debug('%s:%s extract_len %s' % (self.d.name, cs.name, cs.extract_len))

            # free lkp_reg whose scope ends here
            for lf, scope in self.saved_lkp_scope.items():
                if cs == scope.end_cs:
                    self.saved_lkp_reg_free(lf)
        #}
        # it is better to split states at the hw level so that partial headers, TLV
        # headers can be handled easily
        self.split_large_states()
        self.assign_state_ids()

    def get_ext_cstates(self, hdr):
        if hdr not in self.hdr_ext_states:
            ncc_assert(hdr.metadata)
            # not a real header, but atomic meta header
            return []
        return [cs for cs in self.hdr_ext_states[hdr] if not cs.deparse_only]

    def get_meta_ext_cstates(self, mf):
        if mf not in self.meta_ext_states:
            return []
        return copy.copy(self.meta_ext_states[mf])

    def generate_output(self):
        # capri_parser_logical_output(self)
        if (self.asic == 'capri'):
            capri_parser_output_decoders(self)
        elif (self.asic == 'elba'):
            elba_parser_output_decoders(self)
 
    def create_extraction_chunks(self, cs, add_off, extracted_fields):
        # extracted_field entry is a tuple (src, dst). For pkt header both src and dst are
        # pkt header fields. For meta extraction, src can be pkt_field, dst is
        # meta field
        # Need to check that dst fields is also contiguous when extracting multiple fields using
        # a single instruction (generally true.. but may not be true when unions are used)
        extractions = []    # [(pkt_byte, len, phv_byte)]
        curr_off = 0
        pkt_off = 0
        dst_off = 0
        ext_len = 0
        start_phv_bit = None
        start_ext_off = 0

        for src_cf,dst_cf in extracted_fields:
            # dst_cf can be ohi when both src and dst are pkt header flds. If dst is meta
            # if cannot be ohi, so check dst_cf, if ohi, no extraction needed
            if dst_cf.is_ohi:
                continue
            pkt_off = cs.fld_off[src_cf]

            if ext_len == 0:
                # start new chunk (first time)
                start_phv_bit = dst_cf.phv_bit    # phv_bit of the first field in the chunk
                start_ext_off = pkt_off
                curr_off = pkt_off
                dst_off = dst_cf.phv_bit

            if pkt_off == curr_off and dst_off == dst_cf.phv_bit:
                ext_len += src_cf.width
                curr_off += src_cf.width
                dst_off += dst_cf.width
                last_cf = dst_cf # for logging
            else:
                if ext_len:
                    ncc_assert(start_phv_bit != None)
                    if (start_ext_off % 8) or (ext_len % 8):
                        self.logger.warning('Padding mis-alingned extraction at %s, start %d, len %d' % \
                            (last_cf.hfname, start_ext_off, ext_len))

                    start_ext_off = (start_ext_off / 8) * 8
                    ext_len = ((ext_len+7)/ 8) * 8
                    extractions.append(((start_ext_off/8)+add_off, ext_len/8, start_phv_bit/8))
                # start new chunk
                start_phv_bit = dst_cf.phv_bit    # phv_bit of the first field in the chunk
                start_ext_off = pkt_off
                ext_len = src_cf.width
                curr_off = pkt_off + ext_len
                dst_off = dst_cf.phv_bit + dst_cf.width

        if ext_len:
            # add the last open chunk
            extractions.append(((start_ext_off/8)+add_off, (ext_len+7)/8, start_phv_bit/8))

        return extractions

    def create_extract_instructions(self, extractions):
        insts = []
        for ext_off, ext_len, phv_byte in extractions:
            num_bytes = ext_len
            pkt_off = ext_off
            phv_off = phv_byte
            while num_bytes:
                if num_bytes <= 4:
                    insts.append((pkt_off, num_bytes, phv_off))
                    num_bytes = 0
                else:
                    # more than 4 bytes
                    insts.append((pkt_off, 4, phv_off))
                    num_bytes -= 4
                    pkt_off += 4
                    phv_off += 4

        self.logger.debug('Extract instructions : %s' % \
            (['pkt_off %d, bytes %d, phc %d' % (v[0], v[1], v[2]) for v in insts]))
        return insts

    def generate_extract_instructions(self, nxt_cs, add_cs):
        extractions = []
        add_off = 0
        if add_cs:
            add_off += add_cs.extract_len
            extracted_fields = zip(add_cs.extracted_fields, add_cs.extracted_fields)
            # create tuples of contiguous extractions to phv
            extractions += self.create_extraction_chunks(add_cs, 0, extracted_fields)

        extracted_fields = zip(nxt_cs.extracted_fields, nxt_cs.extracted_fields)
        extractions += self.create_extraction_chunks(nxt_cs, add_off, extracted_fields)

        extracted_fields = []
        # set_metadata ops may use pkt byte extraction to populate metadata phvs
        # add them to the list
        # handle set_metadata(dst, current()) if any
        if add_cs:
            for op in add_cs.set_ops:
                if op.op_type != meta_op.EXTRACT_FIELD:
                    continue
                if isinstance(op.src1, tuple):
                    continue
                extracted_fields.append((op.src1, op.dst))
            # sort set_meta fields based on src (pkt) offset)
            extracted_fields = sorted(extracted_fields, key=lambda k: add_cs.fld_off[k[0]])
            extractions += self.create_extraction_chunks(add_cs, 0, extracted_fields)

        extracted_fields = []
        for op in nxt_cs.set_ops:
            if op.op_type != meta_op.EXTRACT_FIELD:
                continue
            if isinstance(op.src1, tuple):
                continue
            extracted_fields.append((op.src1, op.dst))
        # sort set_meta fields based on src (pkt) offset)
        extracted_fields = sorted(extracted_fields, key=lambda k: nxt_cs.fld_off[k[0]])
        extractions += self.create_extraction_chunks(nxt_cs, add_off, extracted_fields)
        # Add tuples from add_cs and nxt_cs
        if add_cs:
            for op in add_cs.set_ops:
                if op.op_type != meta_op.EXTRACT_FIELD:
                    continue
                if not isinstance(op.src1, tuple):
                    continue
                extractions.append((op.src1[0]/8, op.src1[1]/8, op.dst.phv_bit/8))
        for op in nxt_cs.set_ops:
            if op.op_type != meta_op.EXTRACT_FIELD:
                continue
            if not isinstance(op.src1, tuple):
                continue
            extractions.append(((op.src1[0]/8)+add_off, op.src1[1]/8, op.dst.phv_bit/8))
        extractions = sorted(extractions, key=lambda k: k[0])
        # check if any extractions can be combined
        ext_copy = copy.copy(extractions)
        extractions = []
        for t in ext_copy:
            if len(extractions) == 0:
                extractions.append(t)
                continue
            p = extractions[-1]
            if ((p[0]+p[1]) == t[0]) and ((p[2]+p[1]) == t[2]):
                # contigous chunks
                extractions[-1] = (p[0], p[1]+t[1], p[2])
            else:
                extractions.append(t)

        self.logger.debug("%s:extraction_tuples=%s" % (nxt_cs.name, extractions))

        flit_size = self.be.hw_model['parser']['flit_size']
        flit_sizeB = flit_size / 8
        if len(extractions):
            fl1 = extractions[0][2] / flit_sizeB
            fl2 = extractions[-1][2] / flit_sizeB
            if fl1 != fl2:
                self.logger.critical("%s:%s Extractions crossing flits %d -> %d" % \
                    (self.d.name, nxt_cs.name, fl1, fl2))
                self.logger.debug("%s:extracted flds=%s, meta_flds %s" % \
                    (nxt_cs.name, nxt_cs.extracted_fields, nxt_cs.meta_extracted_fields))
                ncc_assert(0, "Fix the critical problems and try again")
            current_flit = fl1

        insts = self.create_extract_instructions(extractions)
        return insts

    def len_chk_profile_get(self, name):
        for p in self.len_chk_profiles:
            if p and p.name == name:
                return p
        return None

    def len_chk_profile_alloc(self, len_chk_profile):
        for p in self.len_chk_profiles:
            if p and p == len_chk_profile:
                ncc_assert(0 )# duplicate allocation
        for i,p in enumerate(self.len_chk_profiles):
            if p == None:
                self.len_chk_profiles[i] = len_chk_profile
                len_chk_profile.prof_id = i
                return i
        return None

    def print_path_histogram(self, bin_size):
        num_bins = (self.longest_path_size + bin_size) / bin_size
        bin_count = {b : 0 for b in range(num_bins)}
        #paths = sorted(self.path_states, key=lambda p: len(p), reverse=True)
        paths = self.path_states
        exclude_names = ['parse_recirc', 'parse_mpls__0', 'parse_tcp_options', \
                         'parse_p4egress', 'parse_fake_intrinsic', 'parse_llc_header', \
                         'parse_ipv6_extn_hdrs', 'parse_ipv4_options', \
                         'parse_inner_ipv4_options', 'parse_icmp', \
                         'parse_geneve', 'parse_nvgre', 'parse_erspan_t3',]
        exclude_set_names = [ \
            ['parse_ipv6', 'parse_vxlan'],
        ]
        '''
        exclude_names = []
        exclude_set_names = [[]]
        '''
        exclude_cs = []
        exclude_lists = [[] for _ in range(len(exclude_set_names))]
        for cs in self.states:
            if cs.name in exclude_names:
                exclude_cs.append(cs)
            for i,ex_list in enumerate(exclude_set_names):
                if cs.name in ex_list:
                    exclude_lists[i].append(cs)

        paths = self.path_states
        for lp in paths:
            exclude = False
            for cs in exclude_cs:
                if cs in lp:
                    exclude = True
                    break
            for ex_list in exclude_lists:
                ex_set = set(ex_list)
                path_set = set(lp)
                if len(ex_set) and ex_set.issubset(path_set):
                    exclude = True
                    break
            if exclude:
                continue

            if len(lp) > 8 and len(lp) <= 14:
                self.logger.debug("%s: Good Path len %02d, %s" % \
                    (self.d.name, len(lp), lp))

            if bin_count[len(lp)/bin_size] == 0:
                self.logger.debug("%s: Sample Path len %02d, %s" % \
                    (self.d.name, len(lp), lp))
            bin_count[len(lp)/bin_size] += 1

        self.logger.debug("%s: path len histogram %s" % \
            (self.d.name, [((i*bin_size), s) for i,s in bin_count.items()]))

    def create_header_graph(self):
        # go thru all parse paths, 
        # create capri_header object if not created already
        # create branch from current node to new header if not already
        hdrs_added = OrderedDict()
        paths = sorted(self.paths, key=lambda p: len(p), reverse=True)
        self.hdr_graph = capri_header(None)
        hdrs_added['Root'] = self.hdr_graph
        for path in paths:
            current_node = self.hdr_graph
            for hdr in path:
                if hdr.name not in hdrs_added:
                    chdr = capri_header(hdr)
                    hdrs_added[hdr.name] = chdr
                else:
                    chdr = hdrs_added[hdr.name]
                    
                current_node.add_branch(chdr)
                current_node = chdr
        # print
        self.logger.debug("Header graph")
        for h in hdrs_added.values():
            self.logger.debug("%s" % h)

        # toposort header graph
        marker = {}
        sorted_list = []
        if _topo_sort_headers(self, self.hdr_graph, sorted_list, marker):
            ncc_assert(0, 'loops in topo sort of headers')

        self.logger.debug("Toposorted headers[%d]: %s" % \
                            (len(sorted_list), [h.name() for h in sorted_list]))
        # replace headers list with new topo-sorted list
        ncc_assert(len(sorted_list)-1 == len(self.headers), "Incorrect topo-sorted header list")
        self.headers = []
        for h in sorted_list:
            if h.p4_header:
                self.headers.append(h.p4_header)


    def print_long_paths(self, path_len):
        # print all paths above path_len
        num_long_paths = 0
        for lp in self.path_states:
            if len(lp) > path_len:
                num_long_paths += 1
                #self.logger.debug("%s: Long parse path(%d)=%s" % (self.d.name, len(lp), lp))
        self.logger.debug("%s: Long paths %d" % (self.d.name, num_long_paths))

    def print_short_paths(self, path_len):
        # print all paths below path_len
        num_paths = 0
        for lp in self.path_states:
            if len(lp) < path_len:
                num_paths += 1
                #self.logger.debug("%s: Shorter parse path(%d)=%s" % (self.d.name, len(lp), lp))
        self.logger.debug("%s: Shorter paths %d" % (self.d.name, num_paths))

    def parser_dbg_output(self):
        parser_state_ids = OrderedDict()
        for cs in self.states:
            parser_state_ids[cs.id] = cs.name
        return parser_state_ids

    def parser_check_flit_violation(self):
        flit_sz = self.be.hw_model['parser']['flit_size']
        path_states = self.path_states
        for path in path_states:
            last_flit = 0
            last_cs = None
            cs_flit = -1
            for cs in path:
                if cs.deparse_only:
                    continue
                for hdr in cs.headers:
                    hdr_phv_bit = self.be.pa.get_hdr_phv_start(hdr, self.d)
                    if hdr_phv_bit < 0:
                        # ohi header
                        continue
                    cs_flit = hdr_phv_bit/flit_sz
                    if cs_flit < last_flit:
                        self.logger.critical( \
                        "%s:Flit violation at %s phv %d, cs %s last_cs %s last_flit %d on path:\n %s" % \
                            (self.d.name, hdr.name, hdr_phv_bit, cs.name, last_cs.name, last_flit, path))
                        ncc_assert(0)
                        return True
                    last_cs = cs

                for cf in cs.meta_extracted_fields:
                    cs_flit = cf.phv_bit / flit_sz
                    if cs_flit < last_flit:
                        self.logger.critical( \
                        "%s:Flit violation at %s phv %d, cs %s last_cs %s last_flit %d on path:\n %s" % \
                            (self.d.name, cf.hfname, cf.phv_bit, cs.name, last_cs.name, last_flit, path))
                        ncc_assert(0)
                        return True
                    last_cs = cs

                if cs_flit >= 0:
                    last_flit = cs_flit

        self.logger.info("%s:No parser flit violations detected" % (self.d.name))
        return False

    def get_hdr_loop(self, h):
        # return all headers of a loop if h is part of loop
        for hdr_group in self.hdr_order_groups:
                if h in hdr_group:
                    # All headers in hdr order group must fit in the same flit as parser
                    # can loop thru' them in any order
                    return hdr_group
        return None
