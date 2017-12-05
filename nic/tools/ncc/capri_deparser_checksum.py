#!/usr/bin/python
#
# Header and Payload Checksum computation in deparser block.
# Capri-Non-Compiler-Compiler (capri-ncc)
#
# Sept 2017, Mahesh Shirshyad (Pensando Systems)
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
from capri_utils import *



# Example Config to understand Deparser Checksum block.
#Case 1
# ipv4 header checksum
#   - Lets say hv bit for ipv4 hdr 5
#   - Lets say hv bit for ipv4 hdr checksum is 10
#   - Configuration for computing hdr checksum is
#       - csr_cfg_csum_hdrs[0]
#           .hdr_num = 10
#           .csum_vld = 1
#           .csum_unit = 1
#           .csum_profile = 0
#           .phdr_vld = 1
#           .phdr_unit = 2 (if TCP/UDP checksum unit is 2)
#           .phdr_profile = 0
#           .csum_unit_include_bm = 0
#           .crc_include_bm = 0
#           .hdrfld_start = cap_dpphdr_csr_cfg_hdr_info[5].fld_start
#           .hdrfld_end = cap_dpphdr_csr_cfg_hdr_info[5].fld_end
#       - csr_cfg_csum_profile[0]
#           .use_phv_len = 0
#           .phv_len_sel = X
#           .len_mask = 0xFFFF
#           .len_shift_left = 0
#           .len_shift_val = 0
#           .start_adj = 0
#           .end_adj = 0
#           .loc_adj = 10 (10th byte in ipv4 hdr is checksum)
#           .add_len = 0
#           .csum_unit_include_bm = 0
#       - csr_cfg_phdr_profile[0]
#           .fld_en_0 = 1
#           .fld_start_0 = start of v4 ipsa = 12
#           .fld_end_0 = end of v4 ipsa = 16
#           .fld_align_0 = 0
#           .fld_en_1 = 1
#           .fld_start_1 = start of v4 ipda = 16
#           .fld_end_1 = start of v4 ipda = 20
#           .fld_align_1 = 0
#           .fld_en_2 = 1
#           .fld_start_2 =  start of protocol offset = 9
#           .fld_end_2 = end of protocol offset = 10
#           .fld_align_2 =  1 (to align protocol to odd byte)
#           .fld_en_3 = X
#           .fld_start_3= X
#           .fld_end_3 = X
#           .fld_align_3 = X
#           .add_len = 1
#
#Case 2
# TCP checksum with V4 as pseudo header
#   - Lets say hv bit for tcp hdr 8
#   - Lets say hv bit for tcp checksum is 12
#   - Configuration for computing tcp checksum is
#       - csr_cfg_csum_hdrs[1]
#           .hdr_num = 12
#           .csum_vld = 1
#           .csum_unit = 2
#           .csum_profile = 1
#           .phdr_vld = 0
#           .phdr_unit = X
#           .phdr_profile = X
#           .csum_unit_include_bm = 0
#           .crc_include_bm = 0
#           .hdrfld_start = cap_dpphdr_csr_cfg_hdr_info[8].fld_start
#           .hdrfld_end = cap_dpphdr_csr_cfg_hdr_info[8].fld_end
#               // how is hdrfld_end used in case of TCP checksum ?
#
#       - csr_cfg_csum_profile[1]
#           .use_phv_len = 1
#           .phv_len_sel = 1 (payload len in phvbit (512 - 528)
#           .len_mask = 0xFFFF
#           .len_shift_left = 0
#           .len_shift_val = 0
#           .start_adj = 0
#           .end_adj = 0
#           .loc_adj = 16 (16th byte in TCP hdr is checksum)
#           .add_len = 1
#           .csum_unit_include_bm = 0


class DeParserPhdrProfile:
    '''
    Csum Pseudo header field profile. 
    '''
    def __init__(self, phdr_profile, phdr_type, add_len):
        self.phdr_profile   = phdr_profile
        self.phdr_type      = phdr_type
        if phdr_type == 'v4':
            self.fld0_en       = 1
            self.fld0_start    = 12 # SA offset from the start of phdr
            self.fld0_end      = 15 # Size of IPSA 
            self.fld0_align    = 0

            self.fld1_en       = 1
            self.fld1_start    = 16 # DA offset from the start of phdr
            self.fld1_end      = 19 # end offset IPDA 
            self.fld1_align    = 0
            
            self.fld2_en       = 1
            self.fld2_start    = 9 # start of zeros/protocol
            self.fld2_end      = 9
            self.fld2_align    = 1 # Aligns protocol 8b value as bottom
            self.add_len       = add_len# Add 16b payload len to computed checksum
                                        # only in case of TCP payload csum (TCP hdr
                                        # has no payload len field)
        elif phdr_type == 'v6':
            self.fld0_en       = 1
            self.fld0_start    = 8  # SA offset from the start of phdr
            self.fld0_end      = 23 # Size of IPSA 
            self.fld0_add_len  = add_len# Add 16b payload len to computed checksum
                                        # only in case of TCP payload csum (TCP hdr
                                        # has no payload len field)
            self.fld0_align    = 0

            self.fld1_en       = 1
            self.fld1_start    = 24 # DA offset from the start of phdr
            self.fld1_end      = 39 # end offset IPDA 
            self.fld1_add_len  = 0 
            self.fld1_align    = 0
            
            self.fld2_en       = 1
            self.fld2_start    = 6 # start of next_header
            self.fld2_end      = 6 # 
            self.fld2_add_len  = 0 
            self.fld2_align    = 1 # Aligns next_header
                                   # 8b value as bottom
                                   # 8b in 16b value
            self.add_len       = add_len
        else:
            assert(0), pdb.set_trace()

    def CsumPhdrProfileUnitNumGet(self):
        return self.phdr_profile

    def ConfigGenerate(self, phdr_profile):
        phdr_profile['fld_en_0']   ['value']=str(self.fld0_en)
        phdr_profile['fld_align_0']['value']=str(self.fld0_align)
        phdr_profile['fld_start_0']['value']=str(self.fld0_start)
        phdr_profile['fld_end_0']  ['value']=str(self.fld0_end)
        phdr_profile['fld_en_1']   ['value']=str(self.fld1_en)
        phdr_profile['fld_align_1']['value']=str(self.fld1_align)
        phdr_profile['fld_start_1']['value']=str(self.fld1_start)
        phdr_profile['fld_end_1']  ['value']=str(self.fld1_end)
        phdr_profile['fld_en_2']   ['value']=str(self.fld2_en)
        phdr_profile['fld_align_2']['value']=str(self.fld2_align)
        phdr_profile['fld_start_2']['value']=str(self.fld2_start)
        phdr_profile['fld_end_2']  ['value']=str(self.fld2_end)
        phdr_profile['add_len']    ['value']=str(self.add_len)
        phdr_profile['_modified']           = True

    def LogGenerate(self):
        log_str = ''
        log_str += ' Deparser Checksum Phdr Profile\n'
        log_str += '-------------------------------\n\n'
        log_str += '        Phdr Type    %s\n' % self.phdr_type
        log_str += '        Profile#     %d\n' % self.phdr_profile
        log_str += '        fld0_en    = %d\n' % self.fld0_en
        log_str += '        fld0_start = %d\n' % self.fld0_start
        log_str += '        fld0_end   = %d\n' % self.fld0_end
        log_str += '        fld0_align = %d\n' % self.fld0_align
        log_str += '        fld1_en    = %d\n' % self.fld1_en
        log_str += '        fld1_start = %d\n' % self.fld1_start
        log_str += '        fld1_end   = %d\n' % self.fld1_end
        log_str += '        fld1_align = %d\n' % self.fld1_align
        log_str += '        fld2_en    = %d\n' % self.fld2_en
        log_str += '        fld2_start = %d\n' % self.fld2_start
        log_str += '        fld2_end   = %d\n' % self.fld2_end
        log_str += '        fld2_align = %d\n' % self.fld2_align
        log_str += '        add_len    = %d\n' % self.add_len
        log_str += '\n'
        return log_str


class DeParserCsumProfile:
    '''
        Deparser Csum profile values.
    '''
    def __init__(self):
        self.csum_profile       = -1 
        self.use_phv_len        = 0
        self.phv_len_sel        = -1 
        self.len_mask           = 0xFFFF
        self.add_len            = 0
        self.csum_unit_include_bm = 0
        self.len_shift_left     = 0
        self.len_shift_val      = 0
        self.start_adj          = 0
        self.end_adj            = 0
        self.csum_loc_adj       = 0
        
    def CsumProfileNumSet(self, profile):
        self.csum_profile = profile

    def CsumProfileNumGet(self):
        return self.csum_profile

    def CsumProfilePhvLenSelSet(self, use_phv_len, phv_len_sel):
        self.use_phv_len = use_phv_len
        self.phv_len_sel = phv_len_sel

    def CsumProfilePhvLenSelGet(self):
        return self.phv_len_sel

    def CsumProfileUsePhvLenGet(self):
        return self.use_phv_len

    def CsumProfileAddLenSet(self, add_len):
        self.add_len = add_len

    def CsumProfileAddLenGet(self):
        return self.use_phv_len

    def CsumProfileShiftLeftSet(self, shift_left, shift_val):
        self.len_shift_left = shift_left
        self.len_shift_val = shift_val

    def CsumProfileStartAdjSet(self, start_adj):
        self.start_adj = start_adj

    def CsumProfileEndAdjSet(self, end_adj):
        self.end_adj = end_adj

    def CsumProfileCsumLocSet(self, csum_loc_adj):
        self.csum_loc_adj = csum_loc_adj

    def ConfigGenerate(self, csum_profile):
        csum_profile['use_phv_len']   ['value']=str(self.use_phv_len)
        csum_profile['phv_len_sel']   ['value']=str(self.phv_len_sel)
        csum_profile['len_mask']      ['value']=str(self.len_mask)
        csum_profile['len_shift_left']['value']=str(self.len_shift_left)
        csum_profile['len_shift_val'] ['value']=str(self.len_shift_val)
        csum_profile['start_adj']     ['value']=str(self.start_adj)
        csum_profile['end_adj']       ['value']=str(self.end_adj)
        csum_profile['loc_adj']       ['value']=str(self.csum_loc_adj)
        csum_profile['add_len']       ['value']=str(self.add_len)
        #csum_profile['csum_unit_include_bm']['value'] = \
        #                        str(self.csum_unit_include_bm)
        csum_profile['_modified']           = True
    def LogGenerate(self):
        log_str = ''
        log_str += 'DeParser Csum Profile:\n'
        log_str += '_________________________\n\n'
        log_str += '    use_phv_len     = %d\n' % self.use_phv_len
        log_str += '    phv_len_sel     = %d\n' % self.phv_len_sel
        log_str += '    len_mask        = 0x%x\n' % self.len_mask
        log_str += '    len_shift_left  = %d\n' % self.len_shift_left
        log_str += '    len_shift_val   = %d\n' % self.len_shift_val
        log_str += '    start_adj       = %d\n' % self.start_adj
        log_str += '    end_adj         = %d\n' % self.end_adj
        log_str += '    loc_adj         = %d\n' % self.csum_loc_adj
        log_str += '    add_len         = %d\n' % self.add_len
        log_str += '    csum_unit_include_bm 0x%x\n' % self.csum_unit_include_bm
        return log_str


class DeParserCsumObj:
    def __init__(self):
        self.unit                   = -1
        self.hv                     = -1
        self.csum_hv                = -1 
        self.profile                = -1
        self.phdr_vld               = 0
        self.phdr_unit              = -1
        self.phdr_profile           = -1
        self.csum_unit_include_bm   = 0
        self.crc_include_bm         = 0
        self.hdrfld_start           = -1
        self.hdrfld_end             = -1

        self.phdr_only              = False # Set to True in case of
                                            # ipv6/ipv4 w/o hdr-checksum

    def CsumUnitNumGet(self):
        return self.unit

    def CsumUnitNumSet(self, unit):
        self.unit = unit

    def CsumInnerCsumResultInclude(self, inner_csum_unit):
        self.csum_unit_include_bm |= (1 << inner_csum_unit)

    def CsumIncludeInnerCsumsGet(self):
        return self.csum_unit_include_bm

    def CsumHvBitNumSet(self, csum_hv):
        self.csum_hv = csum_hv

    def CsumHvBitNumGet(self):
        return self.csum_hv

    def HvBitNumSet(self, hv):
        self.hv = hv

    def HvBitNumGet(self):
        return self.hv

    def CsumProfileNumSet(self, csum_profile):
        self.profile = csum_profile

    def CsumProfileNumGet(self):
        return self.profile

    def PhdrValidSet(self, v):
        #Before setting obj as phdr valid, make sure its invalid with assert check.
        assert self.phdr_vld == 0, pdb.set_trace()
        self.phdr_vld = v
        if self.phdr_vld:
            self.phdr_only = True
        else:
            self.phdr_only = False

    def PhdrValidGet(self):
        return self.phdr_vld

    def PhdrProfileNumSet(self, p):
        assert self.phdr_profile == -1, pdb.set_trace()
        assert p != -1, pdb.set_trace()
        self.phdr_profile = p

    def PhdrProfileNumGet(self):
        return self.phdr_profile

    def PhdrUnitSet(self, csum_unit):
        '''
            Csum Engine Unit which uses this pseudo hdr
        '''
        assert self.phdr_unit == -1 or \
               self.phdr_unit == csum_unit, pdb.set_trace()
        self.phdr_unit = csum_unit

    def PhdrUnitGet(self):
        return self.phdr_unit

    def HdrFldStartEndSet(self, hdrfld_start, hdrfld_end):
        self.hdrfld_start   = hdrfld_start
        self.hdrfld_end     = hdrfld_end

    def HdrFldStartGet(self):
        return self.hdrfld_start

    def ConfigGenerate(self, csum_hdr_cfg):
        #max_hv_bit_idx = self.be.hw_model['parser']['max_hv_bits'] - 1
        max_hv_bit_idx = 127 #Add code to get BE reference in this obj
        csum_hdr_cfg['hdr_num']['value'] = str(max_hv_bit_idx - self.csum_hv)
        if self.phdr_only:
            csum_hdr_cfg['csum_vld']['value']=str(0)
        else:
            csum_hdr_cfg['csum_vld']['value']=str(1)
        csum_hdr_cfg['csum_unit']   ['value']=str(self.unit)\
                                              if self.unit != -1 else str(0)
        csum_hdr_cfg['csum_profile']['value']=str(self.profile)\
                                              if self.profile != -1 else str(0)
        csum_hdr_cfg['phdr_vld']    ['value']=str(self.phdr_vld)
        csum_hdr_cfg['phdr_unit']   ['value']=str(self.phdr_unit)\
                                              if self.phdr_unit != -1 else str(0)
        csum_hdr_cfg['phdr_profile']['value']=str(self.phdr_profile)\
                                              if self.phdr_profile != -1 else str(0)
        csum_hdr_cfg['hdrfld_start']['value']=str(self.hdrfld_start)
        csum_hdr_cfg['hdrfld_end']  ['value']=str(self.hdrfld_end)
        csum_hdr_cfg['crc_include_bm']['value']=str(self.crc_include_bm)
        csum_hdr_cfg['csum_unit_include_bm']['value'] = \
                                      str(self.csum_unit_include_bm)
        csum_hdr_cfg['_modified']            = True

    def LogGenerate(self, csum_hdr):
        log_str = ''
        log_str += 'DeParser CsumConfig: Hdr %s\n' % csum_hdr
        log_str += '_____________________________________\n\n'
        log_str += '    Csum Unit %d\n' % self.unit
        log_str += '    HvBit %d\n' % self.hv
        log_str += '    Csum HvBit %d\n' % self.csum_hv
        log_str += '    Csum profile# %d\n' % self.profile
        log_str += '    Phdr Valid %d\n' % self.phdr_vld
        log_str += '    Phdr Unit %d\n' % self.phdr_unit
        log_str += '    Phdr Profile# %d\n' % self.phdr_profile
        log_str += '    HdrFld Start %d\n' % self.hdrfld_start
        log_str += '    HdrFld End %d\n' % self.hdrfld_end
        log_str += '    crc include BM 0x%x\n' % self.crc_include_bm
        log_str += '    csum unit include BM 0x%x\n' % self.csum_unit_include_bm
        log_str += '    Phdr only %s\n' % ("True" if self.phdr_only else "False")

        return log_str



# Representation for P4 Calulated Field.
class DeParserCalField:
    '''
        Implements header and payload checksums (Ipv4, tcp,udp, inner_udp, inner_v4)
    '''
    def __init__(self, capri_be, dstField, VerifyOrUpdateFunc):
        self.be                    = capri_be
        self.logstr_tbl            = []
        self.dstField              = dstField
        self.csum_hdr_obj          = None # Csum hdr obj which will
                                          # update cal fld
        self.csum_profile_obj      = None # Csum profile obj that
                                          # csum unit will use
        self.phdr_profile_obj      = None # In case of payload csum,
                                          # obj to build Phdr profile.
        self.phdr_csum_hdr_obj     = None # Reference to phdr csum hdr obj
                                          # in case of payload checksum
        self.P4FieldListCalculation= self.be.h.\
                                              p4_field_list_calculations\
                                                 [VerifyOrUpdateFunc]
        #P4 code should have atleast one input field list.
        assert(self.P4FieldListCalculation.input[0].fields[0] != None)
        #Check last input field and last field within the last input field
        #to determine 'payload' keyword is part of field list.
        self.payload_checksum      = True\
                                       if isinstance(\
                                           self.P4FieldListCalculation.\
                                           input[-1].fields[-1],\
                                           p4.p4_header_keywords)\
                                       else\
                                           False
        #Find pseudo header associated with payload checksum
        if self.payload_checksum and 'checksum' in \
            self.P4FieldListCalculation._parsed_pragmas.keys():
            if 'update_len' in \
                self.P4FieldListCalculation._parsed_pragmas['checksum']:
                self.l4_update_len_field = self.P4FieldListCalculation._parsed_pragmas\
                                   ['checksum']['update_len'].keys()[0]

            self.phdr_name, self.phdr_type, self.payload_hdr_type, self.phdr_fields = \
                self.be.checksum.ProcessCalFields(\
                                            self.P4FieldListCalculation,\
                                            self.dstField)

        else:
            self.phdr_name = ''
            self.phdr_type = ''
            self.payload_hdr_type = ''
            self.phdr_fields = None
            self.l4_update_len_field = ''

        assert(self.P4FieldListCalculation != None)
        assert(self.P4FieldListCalculation.algorithm == 'csum16')
        assert(self.P4FieldListCalculation.output_width == 16)


    def CalculatedFieldHdrGet(self):
        hdrinst = self.dstField.split(".")[0]
        return hdrinst

    def DeParserCsumObjSet(self, DeParserCsumObj):
        self.csum_hdr_obj = DeParserCsumObj

    def DeParserCsumObjGet(self):
        return self.csum_hdr_obj

    def DeParserCsumProfileObjSet(self, DeParserProfileObj):
        self.csum_profile_obj = DeParserProfileObj

    def DeParserCsumProfileObjGet(self):
        return self.csum_profile_obj

    def DeParserPhdrCsumObjSet(self, DeParserPhdrCsumObj):
        self.phdr_csum_hdr_obj = DeParserPhdrCsumObj

    def DeParserPhdrCsumObjGet(self):
        return self.phdr_csum_hdr_obj

    def DeParserPhdrProfileObjSet(self, DeParserPhdrProfileObj):
        self.phdr_profile_obj = DeParserPhdrProfileObj

    def DeParserPhdrProfileObjGet(self):
        return self.phdr_profile_obj

    def CsumPhdrNameGet(self):
        return self.phdr_name

    def CsumPhdrTypeGet(self):
        return self.phdr_type

    def CsumPayloadHdrTypeGet(self):
        return self.payload_hdr_type

    def DeParserCsumObjLogStrTableGet(self):
        return self.logstr_tbl

    def DeParserCsumObjAddLog(self, logstr):
        self.logstr_tbl.append(logstr)

    def DeparserCsumConfigMatrixRowLog(self, is_phdr):
        if not is_phdr:
            pstr = '{:12s}{:5d}{:7d}{:7d}{:5d}{:8d}{:6d}{:5d}{:8d}{:7d}{:7d}{:6d}{:5d}'\
                   '{:5d}{:5d}{:5d}\n'.format(self.dstField.split(".")[0],
                                       self.csum_hdr_obj.CsumUnitNumGet(),
                                       self.csum_hdr_obj.CsumHvBitNumGet(),
                                       384 + (127 - self.csum_hdr_obj.CsumHvBitNumGet()),
                                       self.csum_hdr_obj.HvBitNumGet(),
                                       self.csum_hdr_obj.profile,
                                       self.csum_hdr_obj.phdr_vld,
                                       self.csum_hdr_obj.phdr_unit,
                                       self.csum_hdr_obj.phdr_profile,
                                       self.csum_hdr_obj.hdrfld_start,
                                       self.csum_hdr_obj.hdrfld_end,
                                       self.csum_hdr_obj.csum_unit_include_bm,
                                       self.csum_profile_obj.use_phv_len,
                                       self.csum_profile_obj.phv_len_sel,
                                       self.csum_profile_obj.csum_loc_adj,
                                       self.csum_profile_obj.add_len)
        if is_phdr:
            pstr = '{:12s}{:5d}{:7d}{:7d}{:5d}{:8d}{:6d}{:5d}{:8d}{:7d}{:7d}{:6d}'\
                    '\n'.format(self.phdr_name,
                                self.phdr_csum_hdr_obj.CsumUnitNumGet(),
                                self.phdr_csum_hdr_obj.CsumHvBitNumGet(),
                                384 + (127 - self.phdr_csum_hdr_obj.CsumHvBitNumGet()),
                                self.phdr_csum_hdr_obj.HvBitNumGet(),
                                self.phdr_csum_hdr_obj.profile,
                                self.phdr_csum_hdr_obj.phdr_vld,
                                self.phdr_csum_hdr_obj.phdr_unit,
                                self.phdr_csum_hdr_obj.phdr_profile,
                                self.phdr_csum_hdr_obj.hdrfld_start,
                                self.phdr_csum_hdr_obj.hdrfld_end,
                                self.phdr_csum_hdr_obj.csum_unit_include_bm)
        return pstr

class DeParserGsoCalField:
    '''
        Implements GSO csum that includes into final checksum result any partially
        computed checksum result by host/kernel.
    '''
    def __init__(self, capri_be, dstField, UpdateFunc):
        self.be                    = capri_be
        self.logstr_tbl            = []
        self.dstField              = dstField
        self.hdr_valid             = -1
        self.hdrfld_slot           = -1
        self.csum_field_name       = '' # p4 Csum field name.
        self.csum_field_ohi_slot   = -1 # Csum field location inside packet.
        self.gso_csum_result_fld_name = '' # Csum result variable name located in PHV
        self.gso_csum_result_phv   = -1 # phv_bit# of parser computed csum result
        self.field_size            = 2  # two bytes to replace (csum width)
        self.P4FieldListCalculation= self.be.h.\
                                              p4_field_list_calculations\
                                                 [UpdateFunc]
        #P4 code should have atleast one input field list.
        assert(self.P4FieldListCalculation.input[0].fields[0] != None)
        #Check last input field and last field within the last input field
        #to determine 'payload' keyword is part of field list.
        self.payload_checksum      = True\
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

        #Check all headers and header with pragma 'gso_checksum_offset' will indicate
        #p4 field name that specifies csum field location inside packet.

        for hdr_name, hdr in self.be.h.p4_header_instances.items():
            if 'gso_checksum_offset' in hdr._parsed_pragmas:
                self.csum_field_name = hdr._parsed_pragmas['gso_checksum_offset'].keys()[0]
                break

        assert(self.P4FieldListCalculation != None)
        assert(self.P4FieldListCalculation.algorithm == 'gso')
        assert(self.P4FieldListCalculation.output_width == 16)
        assert(self.csum_field_name != '')
        assert(self.payload_checksum)


    def GsoCalculatedFieldHdrGet(self):
        hdrinst = self.dstField.split(".")[0]
        return hdrinst

    def DeParserGsoCsumObjLogStrTableGet(self):
        return self.logstr_tbl

    def DeParserGsoCsumHdrFldSlotSet(self, hdrfld_slot):
        self.hdrfld_slot = hdrfld_slot

    def DeParserGsoCsumHdrFldSlotGet(self):
        return self.hdrfld_slot

    def DeParserGsoCsumHdrValidSet(self, hv):
        self.hdr_valid = hv

    def DeParserGsoCsumHdrValidGet(self):
        return self.hdr_valid

    def DeParserGsoCsumObjAddLog(self, logstr):
        self.logstr_tbl.append(logstr)


    def GsoCfgLogGenerate(self):
        log_str = ''
        log_str += 'DeParser Gso Csum Config: Hdr %s\n' % self.dstField
        log_str += '_____________________________________\n\n'
        log_str += '    HvBit %d\n' % self.hdr_valid
        log_str += '    HdrFld %d\n' % self.hdrfld_slot
        log_str += '    Gso Offset ( %s ) Loaded into Ohi %d\n' % \
                        (self.csum_field_name, self.csum_field_ohi_slot)
        log_str += '    Gso Csum result ( %s ) captured @ phv %d\n' % \
                        (self.gso_csum_result_fld_name, self.gso_csum_result_phv)
        return log_str

    def GsoConfigGenerate(self, dpp_json, dpr_json):
        self.DeParserGsoCsumObjAddLog(self.GsoCfgLogGenerate())
        dpp_rstr_name = 'cap_dpphdrfld_csr_cfg_hdrfld_info[%d]' % (self.hdrfld_slot)
        dpr_rstr_name = 'cap_dprhdrfld_csr_cfg_hdrfld_info[%d]' % (self.hdrfld_slot)
        dpp_rstr = dpp_json['cap_dpp']['registers'][dpp_rstr_name]
        dpr_rstr = dpr_json['cap_dpr']['registers'][dpr_rstr_name]
        dpp_rstr['size_sel']['value'] = str(self.be.hw_model['deparser']['dpa_src_ohi'])
        dpp_rstr['size_val']['value'] = str(self.csum_field_ohi_slot)
        dpr_rstr['source_sel']['value'] = str(self.be.hw_model['deparser']['dpa_src_ohi'])
        dpr_rstr['source_oft']['value'] = str(self.csum_field_ohi_slot)
        dpp_rstr['_modified'] = True
        dpr_rstr['_modified'] = True
        dpr_rstr_name = 'cap_dprhdrfld_csr_cfg_ingress_rw_phv_info[%d]' % (self.hdr_valid)
        dpr_rstr = dpr_json['cap_dpr']['registers'][dpr_rstr_name]
        dpr_rstr['enable']['value'] = str(1)
        dpr_rstr['start_loc']['value'] = str(self.gso_csum_result_phv)
        dpr_rstr['fld_size']['value'] = str(self.field_size)
        dpp_rstr['_modified'] = True
        dpr_rstr['_modified'] = True

