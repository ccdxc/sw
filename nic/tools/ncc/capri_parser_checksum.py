#!/usr/bin/python
#
# Header and Payload Checksum verification in parser block.
# Capri-Non-Compiler-Compiler (capri-ncc)
#
# Sept 2017, Mahesh Shirshyad (Pensando Systems)
#

#
# Checksum Verification HW resources (Done in parser)
# ---------------------------------------------------
# Checksum Engine Allocation for CSUM verfication
#   - 5 csum checkers in Parser
#
#   - In any given parser state, only 2 csum checker can be enabled
#     (csum_inst0, 1).
#
#   - HW Prgm for csum verification: 
#       - CheckSum Unit
#           1b csum_enable, 3b csum_unit_sel, 3b prof_sel
#           6b ohi_start_sel, 6b ohi_len_sel,
#           1b phdr_en, 3b phdr_sel, 6b phdr_ohi_sel,
#           1b dis_zero
#
#       - CheckSum Profile
#           len_mask[14], 
#           len_shift_left[1], len_shift_val[3], 
#           addsub_start[1], start_adj[6]
#           addsub_end[1], end_adj[6]
#           addsub_phdr[1], phdr_adj[6]
#           addsub_csum_loc[1], csum_loc_adj[6]
#           align[1]
#   
# Checksum on V4 Packets
# ----------------------
#   @ Ipv4 parse State
#       - V4 Hdr Checksum and TCP Len Computation in Ipv4 state:
#       - Total Instructions Needed:
#           3 ohi_inst, 2 mux_idx, 2 mux_inst
#           =================================
#           - Instruction to load start of IPv4 header into OHI slot. (ohi_start)
#               * ohi_inst = (3b select = 1, 16b idx_value = 0, 6b ohi_slot_num = X)
#           - Instructions to calculate header length (ihl << 2) (ohi_len)
#               * mux_idx1 = (2b sel = 0, 6b idx = 0) [ Load IHL ]
#               * mux_inst1 = (1b sel = 0, 2b mux_sel = mux_idx#, 
#                             16b mask_val = 0x0F00, 1b 
#                             shift_left = 0, shift_val = 6)
#           - Instruction to load length of IPv4 header into OHI slot. 
#               * ohi_inst = (3b select = 3, 2b mux_sel = mux_inst#, 6b ohi_slot_num = Y)
#           - Compute TCP Len : 
#               * mux_idx2 = (2b sel = 0, 6b idx = 2) [ Load Total Len ]
#               * mux_inst2 = (1b sel = 1, 1b load_mux_pkt = 1,
#                             2b lkpsel = mux_idx2, 2b muxsel = mux_idx1,
#                             16b mask_val = 0x0F00, 1b shift_left = 0,
#                             shift_val = 6)
#           - Save computed TCP Len in OHI slot D : 
#               * ohi_inst = (3b select = 3, 2b muxsel = mux_inst#, 6b ohi_slot_num = D)
#           - Checksum destination = ohi_start + csum_loc_adj = 10byte
#
#
#   @ UDP parse State
#       - Total Instructions Needed:
#           2 ohi_inst
#           ==========
#           - Instruction to load start of UDP header into OHI slot. 
#               * ohi_inst = (3b select = 1, 16b idx_value = 0, 6b ohi_slot_num = A)
#           - Instructions to load payload length from UDP header
#               * ohi_inst = (3b select = 1, 16b idx_value = 4, 6b ohi_slot_num = B)
#           - Phdr start OHI = ohi slot num X (loaded in ipv4 state)
#
#
#   @ TCP parse State
#       - Total Instructions Needed:
#           1 ohi_inst
#           ==========
#           - Instruction to load start of TCP header into OHI slot. 
#               * ohi_inst = (3b select = 1, 16b idx_value = 0, 6b ohi_slot_num = C)
#           - Use computed payload length already loaded in OHI slot
#             (ohi_slot_num = D) in csum profile.
#           - Phdr start OHI = ohi slot num X (loaded in ipv4 state)
#
# Checksum on V6 Packets
# ----------------------
#
#   @ IPV6 Parse State
#       - Save start of v6 header offset in OHI
#       - Save ipv6.paylodLen
#       - Start ipv6 options len computation
#       - Total Instructions Needed: 
#           1 ohi_inst, 1 mux_idx, 1 lkp_val_inst
#           =====================================
#           - Instruction to load start of IPv6 header into OHI slot. (ohi_start)
#             (used for building Phdr in UDP,TCP state)
#               * ohi_inst = (3b select = 1, 16b idx_value = 0, 6b ohi_slot_num = X)
#           - Instruction to load Payload length from IPv6 header into saved lkp_val register
#               * muxidx1  = (2b sel = 0, 6b idx = 4) [ Loads paylad len ]
#               * lkp_val1 = (2b select = 0, 2b mux_sel = muxidx1, 1b store_en = 1)
#                            [ Saves PayloadLen in lkp_val]
#
#   @ IPV6 Option Parsing State
#       - Decrement Option size from Ipv6.payloadLen
#       - DO NOT KNOW HOW TO DECREMENT OPTION HDR SIZE from stored_lkp_val.
#
#   - V6 UDP Checksum:
#       - Total Instructions Needed:
#           2 ohi_inst
#           ==========
#           - Instruction to load start of UDP header into OHI slot. 
#               * ohi_inst = (3b select = 1, 16b idx_value = 0, 6b ohi_slot_num = A)
#           - Instructions to load payload length from UDP header
#               * ohi_inst = (3b select = 1, 16b idx_value = 4, 6b ohi_slot_num = B)
#           - Phdr start OHI = ohi slot num X (loaded in ipv6 state)
#           - Parser processing branch : branch_to udp header
#
#
#   - V4 TCP Checksum:
#       - Need to resolve ipv6 option size calculation and how it can be
#         subtracted from ipv6.payloadLen
#


import os
import sys
import pdb
import logging
import copy
from collections import OrderedDict
from enum import IntEnum
from p4_hlir.main import HLIR
import p4_hlir.hlir.p4 as p4
import capri_logging
from capri_output import *
from capri_utils import *


class ParserPhdrProfile:
    '''
    Csum Pseudo header field profile. 
    '''
    def __init__(self):
        self.phdr_profile   = -1 
        self.fld0_en        = False
        self.fld0_align     = 0
        self.fld0_start     = 0
        self.fld0_end       = 0
        self.fld0_add_len   = 0
        self.fld1_en        = False
        self.fld1_align     = 0
        self.fld1_start     = 0
        self.fld1_end       = 0
        self.fld1_add_len   = 0
        self.fld2_en        = False
        self.fld2_align     = 0
        self.fld2_start     = 0
        self.fld2_end       = 0
        self.fld2_add_len   = 0
        self.fld3_en        = False
        self.fld3_align     = 0
        self.fld3_start     = 0
        self.fld3_end       = 0
        self.fld3_add_len   = 0

    def CsumPhdrProfileUnitNumGet(self):
        return self.phdr_profile

    def CsumPhdrProfileUnitNumSet(self, phdr_profile):
        self.phdr_profile = phdr_profile

    def ConfigGenerate(self, phdr_profile):
        phdr_profile['fld0_fld_en']['value']    = str(1 if self.fld0_en else 0)
        phdr_profile['fld0_fld_align']['value'] = str(self.fld0_align)
        phdr_profile['fld0_fld_start']['value'] = str(self.fld0_start)
        phdr_profile['fld0_fld_end']['value']   = str(self.fld0_end)
        phdr_profile['fld0_add_len']['value']   = str(self.fld0_add_len)

        phdr_profile['fld1_fld_en']['value']    = str(1 if self.fld1_en else 0)
        phdr_profile['fld1_fld_align']['value'] = str(self.fld1_align)
        phdr_profile['fld1_fld_start']['value'] = str(self.fld1_start)
        phdr_profile['fld1_fld_end']['value']   = str(self.fld1_end)
        phdr_profile['fld1_add_len']['value']   = str(self.fld1_add_len)

        phdr_profile['fld2_fld_en']['value']    = str(1 if self.fld2_en else 0)
        phdr_profile['fld2_fld_align']['value'] = str(self.fld2_align)
        phdr_profile['fld2_fld_start']['value'] = str(self.fld2_start)
        phdr_profile['fld2_fld_end']['value']   = str(self.fld2_end)
        phdr_profile['fld2_add_len']['value']   = str(self.fld2_add_len)

        phdr_profile['fld3_fld_en']['value']    = str(1 if self.fld3_en else 0)
        phdr_profile['fld3_fld_align']['value'] = str(self.fld3_align)
        phdr_profile['fld3_fld_start']['value'] = str(self.fld3_start)
        phdr_profile['fld3_fld_end']['value']   = str(self.fld3_end)
        phdr_profile['fld3_add_len']['value']   = str(self.fld3_add_len)

        phdr_profile['_modified'] = True


class ParserCsumProfile:
    '''
    Csum profile values.
    '''
    def __init__(self):
        self.csum_profile       = -1 
        self.len_mask           = 0x3FFF
        self.len_shift_left     = 0
        self.len_shift_val      = 0
        self.addsub_start       = 0
        self.start_adj          = 0
        self.addsub_end         = 0
        self.end_adj            = 0
        self.addsub_phdr        = 0
        self.phdr_adj           = 0
        self.phdr_constant      = 0
        self.addsub_csum_loc    = 0
        self.csum_loc_adj       = 0
        self.align              = 0
        self.csum_8b            = 0
        self.phv_csum_flit_num  = 0
        self.end_eop            = 0

    def CsumProfileUnitNumGet(self):
        return self.csum_profile

    def CsumProfileUnitNumSet(self, csum_profile):
        self.csum_profile = csum_profile

    def CsumProfileShiftLeftSet(self, shift_left, shift_val):
        self.len_shift_left = shift_left
        self.len_shift_val = shift_val

    def CsumProfileStartAdjSet(self, addsub_start, start_adj):
        self.addsub_start = addsub_start
        self.start_adj = start_adj

    def CsumProfileEndAdjSet(self, addsub_end, end_adj):
        self.addsub_end = addsub_end
        self.end_adj = end_adj

    def CsumProfilePhdrSet(self, addsub_phdr, phdr_adj):
        self.addsub_phdr = addsub_phdr
        self.phdr_adj = phdr_adj

    def CsumProfileConstantSet(self, l4hdr_type):
        if l4hdr_type == 'tcp':
            self.phdr_constant = 6  # Tcp proto value
        elif l4hdr_type == 'udp':
            self.phdr_constant = 17 # udp proto value
        else:
            self.phdr_constant = 0    #In all other case make sure its zero.

    def CsumProfileConstantGet(self):
        return self.phdr_constant

    def CsumProfileCsumLocSet(self, addsub_csum_loc, csum_loc_adj):
        self.addsub_csum_loc = addsub_csum_loc
        self.csum_loc_adj = csum_loc_adj

    def CsumProfileCsumPhvFlitSet(self, phv_csum_flit_num):
        self.phv_csum_flit_num  = phv_csum_flit_num
        self.end_eop            = 1

    def CsumProfileCsumPhvFlitGet(self):
        return self.phv_csum_flit_num

    def CsumProfileCsum8bSet(self, csum_8b):
        self.csum_8b = csum_8b

    def CsumProfileCsum8bGet(self):
        return self.csum_8b

    def ConfigGenerate(self, profile):
        profile['len_mask']['value']       = str(self.len_mask)
        profile['len_shift_left']['value'] = str(self.len_shift_left)
        profile['len_shift_val']['value']  = str(self.len_shift_val)
        profile['addsub_start']['value']   = str(self.addsub_start)
        profile['start_adj']['value']      = str(self.start_adj)
        profile['addsub_end']['value']     = str(self.addsub_end)
        profile['end_adj']['value']        = str(self.end_adj)
        profile['addsub_phdr']['value']    = str(self.addsub_phdr)
        profile['phdr_adj']['value']       = str(self.phdr_adj)
        profile['addsub_csum_loc']['value']= str(self.addsub_csum_loc)
        profile['csum_loc_adj']['value']   = str(self.csum_loc_adj)
        profile['align']['value']          = str(self.align)
        profile['phv_csum_flit_num']['value']   = str(self.phv_csum_flit_num)
        profile['end_eop']['value']             = str(self.end_eop)
        profile['csum_8b']['value']             = str(self.csum_8b)
        profile['add_val']['value']             = str(self.phdr_constant)
        profile['_modified']               = True

        log_str = ''
        log_str += 'Csum Profile\n'
        log_str += '    Profile#  %d\n'             % (self.csum_profile)
        log_str += '        len_mask        = 0x%x\n'% (self.len_mask)
        log_str += '        len_shift_left  = %d\n' % (self.len_shift_left)
        log_str += '        len_shift_val   = %d\n' % (self.len_shift_val)
        log_str += '        addsub_start    = %d\n' % (self.addsub_start)
        log_str += '        start_adj       = %d\n' % (self.start_adj)
        log_str += '        addsub_end      = %d\n' % (self.addsub_end)
        log_str += '        end_adj         = %d\n' % (self.end_adj)
        log_str += '        addsub_phdr     = %d\n' % (self.addsub_phdr)
        log_str += '        phdr_adj        = %d\n' % (self.phdr_adj)
        log_str += '        addsub_csum_loc = %d\n' % (self.addsub_csum_loc)
        log_str += '        csum_loc_adj    = %d\n' % (self.csum_loc_adj)
        log_str += '        align           = %d\n' % (self.align)
        log_str += '        phv_flit_num    = %d\n' % (self.phv_csum_flit_num)
        log_str += '        end_eop         = %d\n' % (self.end_eop)
        log_str += '        csum_8b         = %d\n' % (self.csum_8b)
        log_str += '        add_val         = %d\n' % (self.phdr_constant)
        log_str += '\n'

        return log_str

class ParserCsumObj:
    def __init__(self):
        self.unit           = -1    # HW : csum_unit_sel[3b] - init to invalid
        self.ohi_start_sel  = -1    # ohi slot id where csum start offset is
                                    # populated.
        self.ohi_len_sel    = -1    # ohi slot id where csum len is populated.
        self.phdr_ohi_sel   = -1    # ohi slot that has start of IP hdr offest
                                    # In case of ipv4, phdr_ohi_sel and
                                    # ohi_start_sel should be same.
        self.phdr_sel       = -1    # profile for building phdr used in payload
                                    # checksum
        self.payloadLen_ohi_sel = -1# Used only when parser transitions to ipv4.
                                    # ohi slot id used to capture PayloadLen
                                    #  -in ipv4 state, (totalLen - ihl*4) is 
                                    #   stored in ohi and this value used as is
                                    #   in tcp checksum
                                    #  -in ipv6 state, v6.PayloadLen is stored
                                    #   in lookup reg so that v6 options can be
                                    #   decremented.
    def CsumUnitNumGet(self):
        return self.unit

    def CsumUnitNumSet(self, unit):
        self.unit = unit

    def CsumOhiStartSelSet(self, ohiId):
        self.ohi_start_sel = ohiId

    def CsumOhiStartSelGet(self):
        return self.ohi_start_sel

    def CsumOhiLenSelSet(self, ohiId):
        self.ohi_len_sel = ohiId

    def CsumOhiLenSelGet(self):
        return self.ohi_len_sel

    def CsumOhiPhdrSelSet(self, ohiId):
        self.phdr_ohi_sel = ohiId

    def CsumOhiPhdrSelGet(self):
        return self.phdr_ohi_sel

    def CsumOhiTotalLenSet(self, ohiId):
        self.payloadLen_ohi_sel = ohiId

    def CsumOhiTotalLenGet(self):
        return self.payloadLen_ohi_sel


# Representation for P4 Calulated Field.
class ParserCalField:
    '''
    Since capri doesn't support individual header field population
    and instead goes by standard checksum calculation (implicitly
    adds fields needed for checksum based on header start offset and
    length over which checksum in calculated), input fields listed in P4
    will be ignored. Using only checksum destination field,
    identify kind of checksum to be computed or verified.
    '''
    def __init__(self, capri_be, dstField, VerifyOrUpdateFunc):
        self.be                         = capri_be
        self.logstr_tbl                 = []
        self.dstField                   = dstField
        self.parser_csum_obj            = None # Csum unit obj which will
                                               # verify cal fld
        self.parser_csum_profile_obj    = None # Csum profile obj that
                                               # csum unit will use
        self.parser_phdr_profile_obj    = None # In case of payload csum,
                                               # obj to build Phdr
        self.phdr_ohi_sel               = -1   # This value will be set to valid
                                               # value only in the case where
                                               # payload checksum is computed but
                                               # not IP hdr checksum.
        self.phdr_parse_states          = None # Reference to List of Parser
                                               # states where phdr fields
                                               # are extracted/built.
        self.phdr_name                  = ''
        self.phdr_type                  = ''
        self.payload_hdr_type           = ''
        self.l4_verify_len_field        = ''
        self.hdrlen_verify_field        = ''

        self.P4FieldListCalculation       = self.be.h.\
                                            p4_field_list_calculations\
                                            [VerifyOrUpdateFunc]
        #P4 code should have atleast one input field list.
        assert(self.P4FieldListCalculation.input[0].fields[0] != None)

        #Check last input field and last field within the last input field
        #to determine 'payload' keyword is part of field list.
        self.payload_checksum           = True\
                                             if isinstance(\
                                                  self.P4FieldListCalculation.\
                                                  input[-1].fields[-1],\
                                                  p4.p4_header_keywords)\
                                             else\
                                          False

        if 'checksum' in self.P4FieldListCalculation._parsed_pragmas.keys() and \
           'udp_option' in self.P4FieldListCalculation._parsed_pragmas['checksum']:
            self.payload_checksum = False
            self.option_checksum = True
        else:
            self.option_checksum = False

        #Find pseudo header associated with payload checksum
        if self.payload_checksum and 'checksum' in \
            self.P4FieldListCalculation._parsed_pragmas.keys():
            if 'verify_len' in \
                self.P4FieldListCalculation._parsed_pragmas['checksum']:
                self.l4_verify_len_field = self.P4FieldListCalculation._parsed_pragmas\
                                   ['checksum']['verify_len'].keys()[0]
            self.phdr_name, self.phdr_type, self.payload_hdr_type, self.phdr_fields = \
                self.be.checksum.ProcessCalFields(\
                                            self.P4FieldListCalculation,\
                                            self.dstField)
            assert self.l4_verify_len_field  != '', pdb.set_trace()
        elif 'checksum' in self.P4FieldListCalculation._parsed_pragmas.keys():
            if 'hdr_len_expr' in self.P4FieldListCalculation.\
                                    _parsed_pragmas['checksum'].keys():
                self.hdrlen_verify_field = \
                   self.P4FieldListCalculation.\
                    _parsed_pragmas['checksum']['hdr_len_expr'].keys()[0]
                param_list = get_pragma_param_list(self.P4FieldListCalculation.\
                                                   _parsed_pragmas['checksum']['hdr_len_expr'])
                self.end_adj_const = int(param_list[2])
                assert param_list[1] == '+', pdb.set_trace()
            if 'verify_len' in \
                self.P4FieldListCalculation._parsed_pragmas['checksum']:
                self.l4_verify_len_field = self.P4FieldListCalculation._parsed_pragmas\
                                   ['checksum']['verify_len'].keys()[0]
            #assert self.hdrlen_verify_field != '', pdb.set_trace()
            assert self.l4_verify_len_field  != '', pdb.set_trace()

        assert(self.P4FieldListCalculation != None)
        assert((self.P4FieldListCalculation.algorithm == 'csum16') \
               or (self.P4FieldListCalculation.algorithm == 'csum8'))


    def CalculatedFieldHdrGet(self):
        hdrinst = self.dstField.split(".")[0]
        return hdrinst

    def ParserCsumObjSet(self, ParserCsumObj):
        self.parser_csum_obj = ParserCsumObj

    def ParserCsumObjGet(self):
        return self.parser_csum_obj

    def ParserCsumProfileSet(self, ParserProfileObj):
        self.parser_csum_profile_obj = ParserProfileObj

    def ParserCsumProfileGet(self):
        return self.parser_csum_profile_obj

    def ParserPhdrProfileSet(self, ParserPhdrProfileObj):
        self.parser_phdr_profile_obj = ParserPhdrProfileObj

    def ParserPhdrProfileGet(self):
        return self.parser_phdr_profile_obj

    def CsumPhdrNameGet(self):
        return self.phdr_name

    def CsumPhdrParseStateSet(self, phdr_parse_states):
        self.phdr_parse_states = phdr_parse_states

    def CsumPhdrParseStateGet(self):
        return self.phdr_parse_states

    def CsumPhdrTypeGet(self):
        return self.phdr_type

    def CsumPayloadHdrTypeGet(self):
        return self.payload_hdr_type

    def ParserCsumObjLogStrTableGet(self):
        return self.logstr_tbl

    def ParserCsumObjAddLog(self, logstr):
        self.logstr_tbl.append(logstr)

    @staticmethod
    def mux_idx_allocate(mux_idx_allocator, pkt_off):
        return mux_idx_alloc(mux_idx_allocator, pkt_off)

    @staticmethod
    def mux_inst_allocate(mux_inst_allocator, expr):
        instr, _ = mux_inst_alloc(mux_inst_allocator, expr)
        return instr

    @staticmethod
    def _build_mux_idx(sram, mux_sel, index):
        # Load 16b from current offset + idx(in bytes)
        sram['mux_idx'][mux_sel]['sel']['value']    = str(0)
        sram['mux_idx'][mux_sel]['lkpsel']['value'] = str(0)
        sram['mux_idx'][mux_sel]['idx']['value']    = str(index)

        log_str = ''
        log_str += 'Instruction: mux_idx\n'
        log_str += '    Instruction#  %d\n' % (mux_sel)
        log_str += '        sel     = %d [ Load Current offset + Index ]\n' % (0)
        log_str += '        lkpsel  = %d\n' % (0)
        log_str += '        index   = %d\n' % (index)
        log_str += '\n'

        return log_str

    @staticmethod
    def _build_mux_instr(sram, sel, mux_instr_sel, mux_sel, mask, add_sub,\
                         add_sub_val, shift_val, shift_left, \
                         load_mux_pkt, lkpsel):
        sram['mux_inst'][mux_instr_sel]['sel']['value']         = str(sel)
        sram['mux_inst'][mux_instr_sel]['muxsel']['value']      = str(mux_sel)
        sram['mux_inst'][mux_instr_sel]['mask_val']['value']    = str(mask)
        sram['mux_inst'][mux_instr_sel]['addsub_val']['value']  = str(add_sub_val) 
        sram['mux_inst'][mux_instr_sel]['addsub']['value']      = str(add_sub)
        sram['mux_inst'][mux_instr_sel]['shift_left']['value']  = str(shift_left)
        sram['mux_inst'][mux_instr_sel]['shift_val']['value']   = str(shift_val) 
        sram['mux_inst'][mux_instr_sel]['load_mux_pkt']['value']= str(load_mux_pkt)
        sram['mux_inst'][mux_instr_sel]['lkpsel']['value']      = str(lkpsel)

        log_str = ''
        log_str += 'Instruction: mux_Instr\n'
        log_str += '    Instruction#  %d\n' % (mux_instr_sel)
        log_str += '        sel         = %d\n' % (sel)
        log_str += '        muxsel      = %d\n' % (mux_sel)
        log_str += '        mask_val    = 0x%x\n'% (mask)
        log_str += '        addsub      = %d\n' % (add_sub)
        log_str += '        addsub_val  = %d\n' % (add_sub_val)
        log_str += '        shiftleft   = %d\n' % (shift_left)
        log_str += '        shift_val   = %d\n' % (shift_val)
        log_str += '        shift_val   = %d\n' % (shift_val)
        log_str += '        load_mux_pkt= %d\n' % (load_mux_pkt)
        log_str += '        lkp_sel     = %d\n' % (lkpsel)
        log_str += '\n'

        return log_str

    @staticmethod
    def _build_ohi_instr(sram, ohi_instr_inst, select, mux_instr_sel,\
                         index, ohi_slot_num):

        try:
            sram['ohi_inst'][ohi_instr_inst]['sel']['value']        = str(select)
            sram['ohi_inst'][ohi_instr_inst]['muxsel']['value']     = str(mux_instr_sel)
            sram['ohi_inst'][ohi_instr_inst]['idx_val']['value']    = str(index)
            sram['ohi_inst'][ohi_instr_inst]['slot_num']['value']   = str(ohi_slot_num)

            log_str = ''
            log_str += 'Instruction: Ohi_Instr\n'
            log_str += '    Instruction#  %d\n'     % (ohi_instr_inst)
            log_str += '        sel         = %d\n' % (select)
            log_str += '        muxsel      = %d\n' % (mux_instr_sel)
            log_str += '        idx_value   = %d\n' % (index)
            log_str += '        ohiSlot     = %d\n' % (ohi_slot_num)
            log_str += '\n'
            return log_str
        except:
            pdb.set_trace()

    @staticmethod
    def _build_csum_instr(sram, calfldobj, csum_instr, enable, csum_unit,\
                          csum_profile, hdr_ohi_id, len_ohi_id, phdr_profile, \
                          phdr_ohi_id, enable_csum_zero_error):

        sram['csum_inst'][csum_instr]['en']['value']        = str(enable)
        sram['csum_inst'][csum_instr]['unit_sel']['value']  = str(csum_unit)
        sram['csum_inst'][csum_instr]['prof_sel']['value']  = str(csum_profile)
        sram['csum_inst'][csum_instr]['ohi_start_sel']['value'] = str(hdr_ohi_id)
        sram['csum_inst'][csum_instr]['ohi_len_sel']['value'] = str(len_ohi_id)

        phdr_en = 0
        if calfldobj.payload_checksum:
            phdr_en = 1
            sram['csum_inst'][csum_instr]['phdr_en']['value']   = str(phdr_en)
            sram['csum_inst'][csum_instr]['phdr_sel']['value']  = str(phdr_profile)
            sram['csum_inst'][csum_instr]['phdr_ohi_sel']['value'] = str(phdr_ohi_id)
            sram['csum_inst'][csum_instr]['load_phdr_prof_en']['value'] = str(1)
        else:
            sram['csum_inst'][csum_instr]['phdr_en']['value'] = str(phdr_en)

        sram['csum_inst'][csum_instr]['dis_zero']['value'] = str(enable_csum_zero_error)

        log_str = ''
        log_str += 'Csum Instruction\n'
        log_str += '    Instruction#  %d\n'         % (csum_instr)
        log_str += '        enable          = %d\n' % (enable)
        log_str += '        unit_sel        = %d\n' % (csum_unit)
        log_str += '        prof_sel        = %d\n' % (csum_profile)
        log_str += '        ohiID_start_sel = %d\n' % (hdr_ohi_id)
        log_str += '        ohiID_len_sel   = %d\n' % (len_ohi_id)
        log_str += '        phdr_en         = %d\n' % (phdr_en)
        log_str += '        phdr_sel        = %d\n' % (phdr_profile)
        log_str += '        phdr_ohiID_sel  = %d\n' % (phdr_ohi_id)
        log_str += '        dis_zero        = %d\n' % (enable_csum_zero_error)
        log_str += '\n'

        return log_str


    def PhdrProfileBuild(self, parse_state, phdr_profile_obj, add_len):
        '''
            Pseudo header of type based config for pulling phdr fields
        '''
        log_str = ''
        log_str += 'Pseudo Header Profile Values\n'
        assert(len(self.phdr_fields) >= 3), pdb.set_trace()

        if not isinstance(self.phdr_fields[0], int):
            phdr_field                     = self.phdr_fields[0]
            phdr_profile_obj.fld0_en       = 1
            phdr_profile_obj.fld0_start    = (phdr_field.offset + 7) / 8
            phdr_profile_obj.fld0_end      = ((phdr_field.offset + phdr_field.width + 7) / 8) - 1
            phdr_profile_obj.fld0_add_len  = add_len  # Adds 16b len field
                                                      # to phdr only in case of TCP
                                                      # (TCP hdr no payload len field)
            phdr_profile_obj.fld0_align    = 0 if (phdr_field.offset % 16 == 0 and \
                                                   (phdr_field.offset + phdr_field.width) % 16 == 0) else 1
        if not isinstance(self.phdr_fields[1], int):
            phdr_field                     = self.phdr_fields[1]
            phdr_profile_obj.fld1_en       = 1
            phdr_profile_obj.fld1_start    = (phdr_field.offset + 7) / 8
            phdr_profile_obj.fld1_end      = ((phdr_field.offset + phdr_field.width + 7) / 8) - 1
            phdr_profile_obj.fld1_add_len  = 0
            phdr_profile_obj.fld1_align    = 0 if (phdr_field.offset % 16 == 0 and \
                                                   (phdr_field.offset + phdr_field.width) % 16 == 0) else 1

        if not isinstance(self.phdr_fields[2], int):
            phdr_field                     = self.phdr_fields[2]
            phdr_profile_obj.fld2_en       = 1
            phdr_profile_obj.fld2_start    = (phdr_field.offset  + 7) / 8
            phdr_profile_obj.fld2_end      = ((phdr_field.offset + phdr_field.width + 7) / 8) - 1
            phdr_profile_obj.fld2_add_len  = 0
            phdr_profile_obj.fld2_align    = 0 if (phdr_field.offset % 16 == 0 and \
                                                       (phdr_field.offset + phdr_field.width) % 16 == 0) else 1

        log_str += '        Profile#     %d\n' % phdr_profile_obj.phdr_profile
        log_str += '        fld0_en    = %d\n' % phdr_profile_obj.fld0_en
        log_str += '        fld0_start = %d\n' % phdr_profile_obj.fld0_start
        log_str += '        fld0_end   = %d\n' % phdr_profile_obj.fld0_end
        log_str += '        fld0_add_en= %d\n' % phdr_profile_obj.fld0_add_len
        log_str += '        fld0_align = %d\n' % phdr_profile_obj.fld0_align
        log_str += '        fld1_en    = %d\n' % phdr_profile_obj.fld1_en
        log_str += '        fld1_start = %d\n' % phdr_profile_obj.fld1_start
        log_str += '        fld1_end   = %d\n' % phdr_profile_obj.fld1_end
        log_str += '        fld1_add_en= %d\n' % phdr_profile_obj.fld1_add_len
        log_str += '        fld1_align = %d\n' % phdr_profile_obj.fld1_align
        log_str += '        fld2_en    = %d\n' % phdr_profile_obj.fld2_en
        log_str += '        fld2_start = %d\n' % phdr_profile_obj.fld2_start
        log_str += '        fld2_end   = %d\n' % phdr_profile_obj.fld2_end
        log_str += '        fld2_add_en= %d\n' % phdr_profile_obj.fld2_add_len
        log_str += '        fld2_align = %d\n' % phdr_profile_obj.fld2_align
        log_str += '\n'

        return log_str


    def PayloadCsumProfileBuild(self, parse_state, csum_profile_obj, L4HdrType):
        log_str     = ''
        log_str     += 'Csum Profile Values\n'
        shift_left  = 0
        shift_val   = 0
        addsub_start= 0
        start_adj   = 0
        addsub_phdr = 0
        phdr_adj    = 0
        addsub_end  = 0
        end_adj     = 0
        if parse_state and parse_state.phdr_type == 'v4':
            log_str     += '    PseudoHdr Type V4\n'
            addsub_end  = 1
            csum_profile_obj.CsumProfileShiftLeftSet(shift_left, shift_val)
            csum_profile_obj.CsumProfileStartAdjSet(addsub_start, start_adj)
            csum_profile_obj.CsumProfileEndAdjSet(addsub_end, end_adj)
            # Phdr start offset is same as IP hdr start offset
            csum_profile_obj.CsumProfilePhdrSet(addsub_phdr, phdr_adj)
        elif parse_state and parse_state.phdr_type == 'v6':
            log_str     += '    PseudoHdr Type V6\n'
            addsub_end  = 0
            csum_profile_obj.CsumProfileShiftLeftSet(shift_left, shift_val)
            csum_profile_obj.CsumProfileStartAdjSet(addsub_start, start_adj)
            csum_profile_obj.CsumProfileEndAdjSet(addsub_end, end_adj)
            # Phdr start offset is same as IP hdr start offset
            csum_profile_obj.CsumProfilePhdrSet(addsub_phdr, phdr_adj)
        elif parse_state:
            assert(0), pdb.set_trace()

        #One of the phdr fields (ipv4.protocol / ipv6.nextHdr) are programmed as constants.
        #The reason being when ipv6 options are present, protocol value ipv6.nextHdr is 
        #in the last option's header. Since such phdr field is at variable length from
        #the start of IP header, this field is added as constant value depending on
        #L4 header is UDP/TCP.
        csum_profile_obj.CsumProfileConstantSet(L4HdrType)

        log_str += '        Shift Left = %d\n' % shift_left
        log_str += '        Shift Val  = %d\n' % shift_val
        log_str += '        AddSubStart= %d\n' % addsub_start
        log_str += '        StartAdj   = %d\n' % start_adj
        log_str += '        AddSubEnd  = %d\n' % addsub_end
        log_str += '        EndAdj     = %d\n' % end_adj
        log_str += '        AddSubPhdr = %d\n' % addsub_phdr
        log_str += '        PhdrAdj    = %d\n' % phdr_adj
        log_str += '        Addvalue   = 0x%x\n' % csum_profile_obj.CsumProfileConstantGet()
        log_str += '\n'

        return log_str


#           GSO Checksum Details
#           --------------------
#
#   Host to Device communication for GSO
#   -----------------------------------
#       - Host delivers packet to Capri where in checksum is computed partially
#         (ex: only psuedo hdr fields used to compute checksum but none of the
#          payload bytes are yet added to final checksum)
#       - Host indicates Capri Pipeline to foldin partially computed csum and
#         compute final checksum including packet data from gso_start  to EOP
#       - Host via interal headers (p4plus_to_p4) provides following
#            - gso_start [ Offset in packet from where all the packet bytes
#                          should be added to csum computation till EOP ]
#            - gso_offset [Byte Offset inside packet starting from gso_start
#                          where checksum result should be stored ]
#            - gso_valid [ When set indicates to device pipeline that GSO
#                          checksum need to be computed ]
#
#   P4 Representation for GSO
#   -------------------------
#       - P4 calculated construct will be used to express GSO.
#       - CalFld Input field list provides gso_start and payload keyword
#       - Algorithm should provide 'gso'

class ParserGsoCalField:
    '''
        Implements GSO checksum
    '''
    def __init__(self, capri_be, dstField, UpdateFunc):
        self.be                         = capri_be
        self.logstr_tbl                 = []
        self.dstField                   = dstField
        self.parser_csum_obj            = None # GSO Csum unit obj which will
                                               # verify cal fld
        self.parser_csum_profile_obj    = None # GSO Csum profile obj that
                                               # csum unit will use
        self.gso_parse_states           = None # Reference to List of Parser
                                               # states gso_header is extracted

        self.P4FieldListCalculation       = self.be.h.\
                                            p4_field_list_calculations\
                                            [UpdateFunc]
        #P4 code should have atleast one input field list.
        assert(self.P4FieldListCalculation.input[0].fields[0] != None)
        self.gso_start_field = self.P4FieldListCalculation.input[0].fields[0].name

        #Check last input field and last field within the last input field
        #to determine 'payload' keyword is part of field list.
        self.payload_checksum           = True\
                                             if isinstance(\
                                                  self.P4FieldListCalculation.\
                                                  input[-1].fields[-1],\
                                                  p4.p4_header_keywords)\
                                             else\
                                          False

        if self.payload_checksum and 'checksum' in \
            self.P4FieldListCalculation._parsed_pragmas.keys():
            if 'update_len' in \
                self.P4FieldListCalculation._parsed_pragmas['checksum']:
                self.gso_csum_result_fld_name  = self.P4FieldListCalculation._parsed_pragmas\
                                                 ['checksum']['update_len'].keys()[0]
        else:
            assert(0), pdb.set_trace()

        if 'gso_checksum_offset' in \
            self.P4FieldListCalculation._parsed_pragmas['checksum']:
            self.csum_hfield_name = self.P4FieldListCalculation._parsed_pragmas\
                                                 ['checksum']['gso_checksum_offset'].keys()[0]

        assert(self.P4FieldListCalculation != None)
        assert(self.P4FieldListCalculation.algorithm == 'gso')
        assert(self.P4FieldListCalculation.output_width == 16)
        assert(self.csum_hfield_name != '')
        assert(self.payload_checksum)

    def CalculatedFieldHdrGet(self):
        hdrinst = self.dstField.split(".")[0]
        return hdrinst

    def ParserCsumObjSet(self, ParserCsumObj):
        self.parser_csum_obj = ParserCsumObj

    def ParserCsumObjGet(self):
        return self.parser_csum_obj

    def ParserCsumProfileSet(self, ParserProfileObj):
        self.parser_csum_profile_obj = ParserProfileObj

    def ParserCsumProfileGet(self):
        return self.parser_csum_profile_obj

    def GsoCsumObjLogStrTableGet(self):
        return self.logstr_tbl

    def GsoCsumObjAddLog(self, logstr):
        self.logstr_tbl.append(logstr)


    @staticmethod
    def _build_csum_instr(sram, calfldobj, csum_instr, enable, csum_unit,\
                          csum_profile, start_ohi_id, enable_csum_zero_error):
        sram['csum_inst'][csum_instr]['en']['value']        = str(enable)
        sram['csum_inst'][csum_instr]['unit_sel']['value']  = str(csum_unit)
        sram['csum_inst'][csum_instr]['prof_sel']['value']  = str(csum_profile)
        sram['csum_inst'][csum_instr]['ohi_start_sel']['value'] = str(start_ohi_id)
        phdr_en = 0 #No phdr in case of GSO
        sram['csum_inst'][csum_instr]['phdr_en']['value'] = str(phdr_en)
        sram['csum_inst'][csum_instr]['dis_zero']['value'] = str(enable_csum_zero_error)

        log_str = ''
        log_str += 'Csum Instruction\n'
        log_str += '    Instruction#  %d\n'         % (csum_instr)
        log_str += '        enable          = %d\n' % (enable)
        log_str += '        unit_sel        = %d\n' % (csum_unit)
        log_str += '        prof_sel        = %d\n' % (csum_profile)
        log_str += '        ohiID_start_sel = %d\n' % (start_ohi_id)
        log_str += '        phdr_en         = %d\n' % (phdr_en)
        log_str += '\n'

        return log_str


    def GsoCsumProfileBuild(self, parse_state, csum_profile_obj):
        shift_left  = 0
        shift_val   = 0
        addsub_start= 0
        start_adj   = 0
        addsub_phdr = 0
        phdr_adj    = 0
        addsub_end  = 0
        end_adj     = 0
        csum_profile_obj.CsumProfileShiftLeftSet(shift_left, shift_val)
        csum_profile_obj.CsumProfileStartAdjSet(addsub_start, start_adj)
        csum_profile_obj.CsumProfileEndAdjSet(addsub_end, end_adj)
        csum_profile_obj.CsumProfilePhdrSet(addsub_phdr, phdr_adj)

