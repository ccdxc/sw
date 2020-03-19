#!/usr/bin/python
#
# Invariant CRC p4 object processing for RDMA.
# Capri-Non-Compiler-Compiler (capri-ncc)
#
# Oct 2017, Mahesh Shirshyad (Pensando Systems)
#
#

'''
  iCRC : High Level Infomation
  ----------------------------

     ROCEv2
      +-------+-----------+--------+---------+---------+---------+------+-----+
      | EthL2 | EtherType | IP-Hdr | UDP-Hdr | BTH-HDR | Payload | ICRC | FCS |
      +-------+-----------+--------+---------+---------+---------+------+-----+
      optionally : UDP trailer options will be after iCRC bytes and before FCS.

    (a) The ICRC calculation starts with 64 bits of 1
    (b) The ICRC calculation continues with the entire IP datagram starting with
        the first byte of the IP header up until and including the last IB
        Pay-load byte right before the ICRC field itself.
    (c) The variant fields in the IP header are replaced with 1s for the purpose
        of the ICRC calculation/check so that changes to these fields along
        the way dont affect the calculated ICRC value.
    (d) UDP Checksum field is replaced with 1s for the purpose of the ICRC
        calculation/check
    (e) Fields (BTH.fr, BTH.br, BTH.reserved1) which is a byte is replaced with 1s

    For RoCEv2 over IPv4 the fields replaced with 1s for the purpose of ICRC
    calculation are:
        * Time to Live
        * Type of Service (DSCP and ECN).
        * Header Checksum
    For RoCEv2 over IPv6 the fields replaced with 1s for the purpose of ICRC
    calculation are:
        * Traffic Class (DSCP and ECN)
        * Flow Label
        * Hop Limit.

  Capri Requirement:
  ------------------
    (1) TxDMA (Pkt from host) should always contain 4bytes of iCRC. These bytes
        could be all zeros.
    (2) Deparser in Pipeline will not auto compute packet payload length stick
        iCRC at the end of it. It requires MPU to assist with payload len calcualtion.

  How iCRC expressed in P4
  ------------------------
    1. P4 construct calculated_field is used to express icrc
    2. Parser local variable is used specify destination of icrc computed value.
       icrc destination field doesn't use PHV space.
    3. Input list contains either ipv4.version or ipv6.version to indicate
       L3 header is v4 / v6. No other fields are specifiedin P4. Any ipv4/ipv6
       can be used instead of version field.
    4. Algorithm specified should be "icrc"

  - iCRC verification done in IG pipe parser. [for pkt from uplink, icrc error bit
    should be honored. For pkt from host, icrc error bit should be ignored]
  - iCRC update is done in EG pipe deparser.

  iCRC Logical Output
  --------------------
  Pipeline configuration to setup icrc verifcation and computation in
  <gen-dir>/logs/icrc.out


  Capri Deparser Challenges for insert 4B iCRC
  --------------------------------------------
   1. Today, deparser can add bytes to construct outgoing packet based on
      header valid bit and associated header field info.
   2. In order to add 4B of iCRC, (if original packet from host) did not have it
      pipeline will have to create a HV bit meant to add iCRC bytes. Since to parser
      these bytes are transparent, programming header field details for icrc HV bit
      will be difficult.
   3. Current deparser model is to assume RDMA packet from host/network will have
      4 bytes of iCRC. In case of pkt from host, icrc 4B could be place holder bytes
      and host need not compute it.

'''

'''                 iCRC Engine
                -------------------

    Capri Parser Verfying iCRC
    --------------------------
    1. For capri parser to verify iCRC, parser block needs to be programmed
       with the start offset of L3 Hdr covering UDP and  RDMA-BTH.
    2. Program mask profile to include 64b'1s and make L3 fields invariant
    3. In parse state where roce-bth header is extracted, enable icrc

    Capri DeParser iCRC Computation
    -------------------------------
    REQUIREMENT:  RDMA Packet from host and from n/w should always have 4B of
                  iCRC. In case of packet from host, TxDMA can append 4Bytes of
                  zeros.
    1. Deparser need to be programmed to  include headers and paylaod  that
       are part of iCRC computation.
    2. In order to selectively turn on/off iCRC computation, apart from regular
       header valid bits, roce_bth.icrc HV bit will be allocated. This .icrc bit
       will be used to enable iCRC computation and OVERWRITE last  4B of payload.
    3. MPU/Pipe should set roce.icrc HV bits and should also keep track of
       L2 Payload Len without UDP options, so that location of icrc (4bytes)
       can be specified to deparser.
'''


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



# If P4 has icrc calculated_fields, process and build objects
# There can be 2 types of objects. One for verfication
# and one for update.


#                   ICRC verification In parser
#               -----------------------------------

class IcrcParserProfile:
    '''
    Icrc profile values.
    '''
    def __init__(self):
        self.icrc_profile       = 0 #-1
        self.len_mask           = 0x3FFF
        self.len_shift_left     = 0
        self.len_shift_val      = 0
        self.addsub_start       = 0
        self.start_adj          = 0
        self.addsub_end         = 0
        self.end_adj            = 0
        self.addsub_mask        = 0
        self.mask_adj           = 0
        self.end_eop            = 0

        self.mask_profile       = 0
        #Mask profile fields.
        self.mask_fields        = {} #Key is one of 5 mask fld instances
        self.l4_mask_fields     = {} #Key is one of 5 mask fld instances
        self.l5_mask_fields     = {} #Key is one of 5 mask fld instances

    def IcrcProfileNumGet(self):
        return self.icrc_profile

    def IcrcProfileNumSet(self, icrc_profile):
        self.icrc_profile = icrc_profile

    def IcrcMaskProfileNumGet(self):
        return self.mask_profile

    def IcrcMaskProfileNumSet(self, mask_profile):
        self.mask_profile = mask_profile

    def IcrcProfileShiftLeftSet(self, shift_left, shift_val):
        self.len_shift_left = shift_left
        self.len_shift_val = shift_val

    def IcrcProfileStartAdjSet(self, addsub_start, start_adj):
        self.addsub_start = addsub_start
        self.start_adj = start_adj

    def IcrcProfileEndAdjSet(self, addsub_end, end_adj):
        self.addsub_end = addsub_end
        self.end_adj = end_adj

    def IcrcProfileMaskAdjSet(self, addsub_mask, mask_adj):
        self.addsub_mask = addsub_mask
        self.mask_adj = mask_adj

    def IcrcProfileEndEopSet(self, end_eop):
        self.end_eop = end_eop

    def IcrcMaskProfileMaskFieldAdd(self, fld_inst, masked_field):
        self.mask_fields[fld_inst] = masked_field

    def IcrcL4MaskProfileMaskFieldAdd(self, fld_inst, masked_field):
        self.l4_mask_fields[fld_inst] = masked_field

    def IcrcL5MaskProfileMaskFieldAdd(self, fld_inst, masked_field):
        self.l5_mask_fields[fld_inst] = masked_field

    def IcrcMaskProfileMaskFieldLenGet(self):
        return len(self.mask_fields)

    def IcrcL4MaskProfileMaskFieldLenGet(self):
        return len(self.l4_mask_fields)

    def IcrcL5MaskProfileMaskFieldLenGet(self):
        return len(self.l5_mask_fields)

    def ConfigGenerate(self, profile):
        profile['len_mask']['value']       = str(self.len_mask)
        profile['len_shift_left']['value'] = str(self.len_shift_left)
        profile['len_shift_val']['value']  = str(self.len_shift_val)
        profile['addsub_start']['value']   = str(self.addsub_start)
        profile['start_adj']['value']      = str(self.start_adj)
        profile['addsub_end']['value']     = str(self.addsub_end)
        profile['end_adj']['value']        = str(self.end_adj)
        profile['addsub_mask']['value']    = str(self.addsub_mask)
        profile['mask_adj']['value']       = str(self.mask_adj)
        profile['end_eop']['value']        = str(self.end_eop)
        profile['mask_prof_sel']['value']  = str(self.mask_profile)
        profile['_modified']               = True

    def LogGenerate(self):
        log_str = ''
        log_str += 'Icrc Profile\n'
        log_str += '    Profile#  %d\n'             % (self.icrc_profile)
        log_str += '        len_mask        = 0x%x\n'% (self.len_mask)
        log_str += '        len_shift_left  = %d\n' % (self.len_shift_left)
        log_str += '        len_shift_val   = %d\n' % (self.len_shift_val)
        log_str += '        addsub_start    = %d\n' % (self.addsub_start)
        log_str += '        start_adj       = %d\n' % (self.start_adj)
        log_str += '        addsub_end      = %d\n' % (self.addsub_end)
        log_str += '        end_adj         = %d\n' % (self.end_adj)
        log_str += '        addsub_mask     = %d\n' % (self.addsub_mask)
        log_str += '        mask_adj        = %d\n' % (self.mask_adj)
        log_str += '        end_eop         = %d\n' % (self.end_eop)
        log_str += '        mask profile    = %d\n' % (self.mask_profile)
        log_str += '\n'
        return log_str

    def MaskProfileConfigGenerate(self, mask_profile):
        for fld_inst, fld_mask_cfg in self.mask_fields.items():
            prefix = 'fld%d_' % fld_inst
            for k, v in fld_mask_cfg.items():
                mask_profile[prefix+k]['value'] = str(v)
        for fld_inst, fld_mask_cfg in self.l4_mask_fields.items():
            prefix = 'fld%d_' % fld_inst
            for k, v in fld_mask_cfg.items():
                mask_profile[prefix+k]['value'] = str(v)
        for fld_inst, fld_mask_cfg in self.l5_mask_fields.items():
            prefix = 'fld%d_' % fld_inst
            for k, v in fld_mask_cfg.items():
                mask_profile[prefix+k]['value'] = str(v)

    def MaskProfileLogGenerate(self):
        log_str = ''
        log_str += 'Parser Icrc Mask Profile:\n'
        log_str += '____________________________\n\n'
        for fld_inst, fld_mask_cfg in self.mask_fields.items():
            prefix = 'fld%d_' % fld_inst
            for k, v in fld_mask_cfg.items():
                pstr = '{:20s}'.format(prefix+k)
                _log_str = '        ' + pstr + '         = 0x%x\n'% (v)
                log_str += _log_str
        for fld_inst, fld_mask_cfg in self.l4_mask_fields.items():
            prefix = 'fld%d_' % fld_inst
            for k, v in fld_mask_cfg.items():
                pstr = '{:20s}'.format(prefix+k)
                _log_str = '        ' + pstr + '         = 0x%x\n'% (v)
                log_str += _log_str
        for fld_inst, fld_mask_cfg in self.l5_mask_fields.items():
            prefix = 'fld%d_' % fld_inst
            for k, v in fld_mask_cfg.items():
                pstr = '{:20s}'.format(prefix+k)
                _log_str = '        ' + pstr + '         = 0x%x\n'% (v)
                log_str += _log_str
        log_str += '\n'
        return log_str

# Representation for P4 ICRC Calulated Field.
class IcrcParserCalField:
    '''
    '''
    def __init__(self, capri_be, dstField, VerifyOrUpdateFunc, roce_hdr):
        self.be                         = capri_be
        self.logstr_tbl                 = []
        self.dstField                   = dstField
        self.icrc_prsr_profile_obj      = None # Icrc L3 parser profile obj
        self.l3_ohi_start_sel           = -1
        self.l3_ohi_len_sel             = -1
        self.ohi_mask_sel               = -1
        self.l3hdr_parse_states         = None # Reference to List of Parser
                                               # states where L3 hdr fields
                                               # are extracted/built.
        self.l3hdr_name                 = ''
        self.roce_hdr                   = roce_hdr

        self.P4FieldListCalculation     = self.be.h.\
                                            p4_field_list_calculations\
                                            [VerifyOrUpdateFunc]
        if 'icrc' in \
            self.P4FieldListCalculation._parsed_pragmas.keys():
            if 'verify_len' in \
                self.P4FieldListCalculation._parsed_pragmas['icrc']:
                self.icrc_verify_len_field = self.P4FieldListCalculation._parsed_pragmas\
                               ['icrc']['verify_len'].keys()[0]
            else:
                self.icrc_verify_len_field = ''
        #P4 code should have atleast one input field list.
        ncc_assert(self.P4FieldListCalculation.input[0].fields[0] != None)
        ncc_assert(self.P4FieldListCalculation != None)
        ncc_assert(self.P4FieldListCalculation.algorithm == 'icrc')
        ncc_assert(self.P4FieldListCalculation.output_width == 32)

        self.l3hdr_name, self.l3hdr_invariant_fields, self.l4_hdr_name, \
        self.l4_hdr_invariant_fields, self.l5_hdr_name, self.l5_hdr_invariant_fields = \
            self.ProcessIcrcCalFields(self.P4FieldListCalculation, dstField)

    def ProcessIcrcCalFields(self, field_list_calculation, icrc_field):
        '''
        '''
        icrc_p4_field = None
        icrc_hdr = icrc_field.split(".")[0]
        icrc_field = icrc_field.split(".")[1]
        l3hdr_ifields = []
        for idx, field in enumerate(field_list_calculation.input[0].fields[0:-2]):
            if idx == 0:
                ncc_assert(field.instance.name != icrc_hdr)
                l3hdr_name = field.instance.name
            l3hdr_ifields.append(field)
        #Last invariant field in the list is udp.checksum field.
        l4_hdr_ifields = []
        l4_hdr_name = ''
        field = field_list_calculation.input[0].fields[-2]
        l4_hdr_name = field.instance.name
        l4_hdr_ifields.append(field)
        l5_hdr_ifields = []
        l5_hdr_name = ''
        field = field_list_calculation.input[0].fields[-1]
        l5_hdr_name = field.instance.name
        l5_hdr_ifields.append(field)

        return l3hdr_name, l3hdr_ifields, l4_hdr_name, l4_hdr_ifields,\
               l5_hdr_name, l5_hdr_ifields

    def CalculatedFieldHdrGet(self):
        hdrinst = self.dstField.split(".")[0]
        return hdrinst

    def L3HdrInvariantFieldsGet(self):
        return self.l3hdr_invariant_fields

    def L4HdrInvariantFieldsGet(self):
        return self.l4_hdr_invariant_fields

    def L5HdrInvariantFieldsGet(self):
        return self.l5_hdr_invariant_fields

    def IcrcParserProfileObjSet(self, IcrcParserProfileObj):
        self.icrc_prsr_profile_obj = IcrcParserProfileObj

    def IcrcParserProfileObjGet(self):
        return self.icrc_prsr_profile_obj

    def IcrcL3HdrNameGet(self):
        return self.l3hdr_name

    def IcrcL4HdrNameGet(self):
        return self.l4_hdr_name

    def IcrcL5HdrNameGet(self):
        return self.l5_hdr_name

    def IcrcL3hdrParseStateSet(self, l3hdr_parse_states):
        self.l3hdr_parse_states = l3hdr_parse_states

    def IcrcL3hdrParseStateGet(self):
        return self.l3hdr_parse_states

    def IcrcOhiStartSelSet(self, ohiId):
        self.l3_ohi_start_sel = ohiId

    def IcrcOhiStartSelGet(self):
        return self.l3_ohi_start_sel

    def IcrcOhiLenSelSet(self, ohiId):
        self.l3_ohi_len_sel = ohiId

    def IcrcOhiLenSelGet(self):
        return self.l3_ohi_len_sel

    def IcrcOhiMaskSelSet(self, ohiId):
        self.ohi_mask_sel = ohiId

    def IcrcOhiMaskSelGet(self):
        return self.ohi_mask_sel

    def IcrcLogStrTableGet(self):
        return self.logstr_tbl

    def IcrcAddLog(self, logstr):
        self.logstr_tbl.append(logstr)

    @staticmethod
    def _build_icrc_instr(sram, calfldobj, enable, prof_sel_en,\
                          icrc_profile, hdr_ohi_id, len_ohi_id,\
                          mask_ohi_id):

        sram['crc_inst']['en']['value']        = str(enable)
        sram['crc_inst']['prof_sel_en']['value']  = str(prof_sel_en)
        sram['crc_inst']['prof_sel']['value']  = str(icrc_profile)
        sram['crc_inst']['ohi_start_sel']['value'] = str(hdr_ohi_id)
        sram['crc_inst']['ohi_len_sel']['value'] = str(len_ohi_id)
        sram['crc_inst']['ohi_mask_sel']['value'] = str(mask_ohi_id)

        log_str = ''
        log_str += 'Icrc Instruction\n'
        log_str += '        enable          = %d\n' % (enable)
        log_str += '        prof_sel_en     = %d\n' % (prof_sel_en)
        if prof_sel_en:
            log_str += '        icrc_profile    = %d\n' % (icrc_profile)
            log_str += '        ohiID_start_sel = %d\n' % (hdr_ohi_id)
            log_str += '        ohiID_len_sel   = %d\n' % (len_ohi_id)
            log_str += '        ohiID_mask_sel  = %d\n' % (mask_ohi_id)
        else:
            log_str += '        icrc_profile    Ignored; Latched prof used from previous state\n'
            log_str += '        ohiID_start_sel Ignored; Latched prof used from previous state\n'
            log_str += '        ohiID_len_sel   Ignored; Latched prof used from previous state\n'
            log_str += '        ohiID_mask_sel  Ignored; Latched prof used from previous state\n'
        log_str += '\n'

        return log_str

#                   ICRC computation In Deparser
#               -----------------------------------
#
# Example Config to understand Deparser icrc block.
# L3 Hdr is ipv4/ipv6 and
#   - Lets say hv bit for roce_bth.icrc is 10
#   - Configuration for computing icrc is
#
#       - csr_cfg_crc_hdrs[0]
#           .hdr_num = 10
#           .crc_vld = 1
#           .crc_unit = 0
#           .crc_profile = 0
#           .mask_vld = 1
#           .mask_unit = ??????????
#           .mask_profile = 0
#           .hdrfld_start = cap_dpphdr_csr_cfg_hdr_info[L3].fld_start
#           .hdrfld_end =  0  // since length used comes from PHV
#
#       - csr_cfg_crc_profile[0]
#           .use_phv_len = 1
#           .phv_len_sel = X
#           .len_mask = 0xFFFF
#           .len_shift_left = 0
#           .len_shift_val = 0
#           .start_adj = 0
#           .end_adj = 0
#           .loc_adj = 0



class IcrcDeParserProfile:
    '''
        Deparser Icrc profile values.
    '''
    def __init__(self):
        self.icrc_profile       = -1
        self.use_phv_len        = 1
        self.phv_len_sel        = -1
        self.len_mask           = 0xFFFF
        self.len_shift_left     = 0
        self.len_shift_val      = 0
        self.start_sop          = -1
        self.start_eop          = -1
        self.start_adj          = 0
        self.start_adj_sub      = 0
        self.end_adj            = 0
        self.end_adj_sub        = 0
        self.icrc_loc_adj       = 0
        self.icrc_loc_adj_sub   = 0

        #Mask profile fields.
        self.mask_profile       = 0
        self.l4_mask_profile    = 0
        self.mask_fields        = {} #Key is one of 6 mask fld instances
        self.l4_mask_fields     = {} #Key is one of 6 mask fld instances
        self.l5_mask_profile    = 0
        self.l5_mask_fields     = {} #Key is one of 6 mask fld instances

    def IcrcProfileNumSet(self, profile):
        self.icrc_profile = profile

    def IcrcProfileNumGet(self):
        return self.icrc_profile

    def IcrcMaskProfileNumSet(self, mask_profile):
        self.mask_profile = mask_profile

    def IcrcMaskProfileNumGet(self):
        return self.mask_profile

    def IcrcL4MaskProfileNumSet(self, mask_profile):
        self.l4_mask_profile = mask_profile

    def IcrcL4MaskProfileNumGet(self):
        return self.l4_mask_profile

    def IcrcL5MaskProfileNumSet(self, mask_profile):
        self.l5_mask_profile = mask_profile

    def IcrcL5MaskProfileNumGet(self):
        return self.l5_mask_profile

    def IcrcProfilePhvLenSelSet(self, use_phv_len, phv_len_sel):
        self.use_phv_len = use_phv_len
        self.phv_len_sel = phv_len_sel

    def IcrcProfilePhvLenSelGet(self):
        return self.phv_len_sel

    def IcrcProfileUsePhvLenGet(self):
        return self.use_phv_len

    def IcrcProfileShiftLeftSet(self, shift_left, shift_val):
        self.len_shift_left = shift_left
        self.len_shift_val  = shift_val

    def IcrcProfileStartAdjSet(self, start_adj, sub_adj=0):
        self.start_adj = start_adj
        self.start_adj_sub = sub_adj #if sub_adj == 1, start_adj is subtracted

    def IcrcProfileEndAdjSet(self, end_adj, sub_adj=0):
        self.end_adj        = end_adj
        self.end_adj_sub    = sub_adj

    def IcrcProfileLocAdjSet(self, icrc_loc_adj, icrc_loc_adj_sub=0):
        self.icrc_loc_adj       = icrc_loc_adj
        self.icrc_loc_adj_sub   = icrc_loc_adj_sub

    def IcrcMaskProfileMaskFieldAdd(self, fld_inst, masked_field):
        self.mask_fields[fld_inst] = masked_field

    def IcrcL4MaskProfileMaskFieldAdd(self, fld_inst, masked_field):
        self.l4_mask_fields[fld_inst] = masked_field

    def IcrcL5MaskProfileMaskFieldAdd(self, fld_inst, masked_field):
        self.l5_mask_fields[fld_inst] = masked_field

    def IcrcMaskProfileMaskFieldLenGet(self):
        return len(self.mask_fields)

    def IcrcL4MaskProfileMaskFieldLenGet(self):
        return len(self.l4_mask_fields)

    def IcrcL5MaskProfileMaskFieldLenGet(self):
        return len(self.l5_mask_fields)

    def ConfigGenerate(self, icrc_profile):
        icrc_profile['use_phv_len']   ['value']=str(self.use_phv_len)
        icrc_profile['phv_len_sel']   ['value']=str(self.phv_len_sel)
        icrc_profile['len_mask']      ['value']=str(self.len_mask)
        icrc_profile['len_shift_left']['value']=str(self.len_shift_left)
        icrc_profile['len_shift_val'] ['value']=str(self.len_shift_val)
        icrc_profile['start_adj']     ['value']=str(self.start_adj)

        icrc_profile['start_adj_sub'] ['value']=str(self.start_adj_sub)
        icrc_profile['end_adj']       ['value']=str(self.end_adj)
        icrc_profile['end_adj_sub']   ['value']=str(self.end_adj_sub)
        icrc_profile['loc_adj']       ['value']=str(self.icrc_loc_adj)
        icrc_profile['loc_adj_sub']   ['value']=str(self.icrc_loc_adj_sub)
        #Deparser now provides one bit knob to add 64 1'b transparently before
        #start of L3 header. Set this knob for all iCRC cases.
        icrc_profile['add_fix_mask']  ['value']=str(1)
        icrc_profile['_modified']              = True

    def LogGenerate(self):
        log_str = ''
        log_str += 'DeParser Icrc Profile:\n'
        log_str += '_________________________\n\n'
        log_str += '    use_phv_len     = %d\n' % self.use_phv_len
        log_str += '    phv_len_sel     = %d\n' % self.phv_len_sel
        log_str += '    len_mask        = 0x%x\n' % self.len_mask
        log_str += '    len_shift_left  = %d\n' % self.len_shift_left
        log_str += '    len_shift_val   = %d\n' % self.len_shift_val
        log_str += '    start_adj       = %d\n' % self.start_adj
        log_str += '    start_adj_sub   = %d\n' % self.start_adj_sub
        log_str += '    end_adj         = %d\n' % self.end_adj
        log_str += '    end_adj_sub     = %d\n' % self.end_adj_sub
        log_str += '    loc_adj         = %d\n' % self.end_adj
        log_str += '    loc_adj_sub     = %d\n' % self.end_adj_sub
        return log_str

    def MaskProfileConfigGenerate(self, mask_profile):
        prefix = 'fld_'
        for fld_inst, fld_mask_cfg in self.mask_fields.items():
            suffix = '_%d' % fld_inst
            for k, v in fld_mask_cfg.items():
                if 'skip_first_nibble' in k:
                    mask_profile[k+suffix]['value'] = str(v)
                elif 'fill' in k:
                    mask_profile[k]['value'] = str(v)
                else:
                    mask_profile[prefix+k+suffix]['value'] = str(v)
        mask_profile['_modified'] = True

    def L4MaskProfileConfigGenerate(self, mask_profile):
        prefix = 'fld_'
        for fld_inst, fld_mask_cfg in self.l4_mask_fields.items():
            suffix = '_%d' % fld_inst
            for k, v in fld_mask_cfg.items():
                if 'skip_first_nibble' in k:
                    mask_profile[k+suffix]['value'] = str(v)
                elif 'fill' in k:
                    mask_profile[k]['value'] = str(v)
                else:
                    mask_profile[prefix+k+suffix]['value'] = str(v)
        mask_profile['_modified'] = True

    def L5MaskProfileConfigGenerate(self, mask_profile):
        prefix = 'fld_'
        for fld_inst, fld_mask_cfg in self.l5_mask_fields.items():
            suffix = '_%d' % fld_inst
            for k, v in fld_mask_cfg.items():
                if 'skip_first_nibble' in k:
                    mask_profile[k+suffix]['value'] = str(v)
                elif 'fill' in k:
                    mask_profile[k]['value'] = str(v)
                else:
                    mask_profile[prefix+k+suffix]['value'] = str(v)
        mask_profile['_modified'] = True

    def MaskProfileLogGenerate(self):
        prefix = 'fld_'
        log_str = ''
        log_str += 'DeParser Icrc Mask Profile:\n'
        log_str += '____________________________\n\n'
        for fld_inst, fld_mask_cfg in self.mask_fields.items():
            suffix = '_%d' % fld_inst
            for k, v in fld_mask_cfg.items():
                if 'skip_first_nibble' in k:
                    pstr = '{:20s}'.format(k+suffix)
                elif 'fill' in k:
                    pstr = '{:20s}'.format(k)
                else:
                    pstr = '{:20s}'.format(prefix+k+suffix)
                _log_str = '        ' + pstr + '         = 0x%x\n'% (v)
                log_str += _log_str
        log_str += '\n'
        return log_str

    def L4MaskProfileLogGenerate(self):
        prefix = 'fld_'
        log_str = ''
        log_str += 'DeParser L4 Icrc Mask Profile:\n'
        log_str += '____________________________\n\n'
        for fld_inst, fld_mask_cfg in self.l4_mask_fields.items():
            suffix = '_%d' % fld_inst
            for k, v in fld_mask_cfg.items():
                if 'skip_first_nibble' in k:
                    pstr = '{:20s}'.format(k+suffix)
                elif 'fill' in k:
                    pstr = '{:20s}'.format(k)
                else:
                    pstr = '{:20s}'.format(prefix+k+suffix)
                _log_str = '        ' + pstr + '         = 0x%x\n'% (v)
                log_str += _log_str
        log_str += '\n'
        return log_str

    def L5MaskProfileLogGenerate(self):
        prefix = 'fld_'
        log_str = ''
        log_str += 'DeParser L5 Icrc Mask Profile:\n'
        log_str += '____________________________\n\n'
        for fld_inst, fld_mask_cfg in self.l5_mask_fields.items():
            suffix = '_%d' % fld_inst
            for k, v in fld_mask_cfg.items():
                if 'skip_first_nibble' in k:
                    pstr = '{:20s}'.format(k+suffix)
                elif 'fill' in k:
                    pstr = '{:20s}'.format(k)
                else:
                    pstr = '{:20s}'.format(prefix+k+suffix)
                _log_str = '        ' + pstr + '         = 0x%x\n'% (v)
                log_str += _log_str
        log_str += '\n'
        return log_str


class IcrcDeParserCalField:
    '''
    '''
    def __init__(self, capri_be, dstField, VerifyOrUpdateFunc, roce_hdr):
        self.be                    = capri_be
        self.logstr_tbl            = []
        self.dstField              = dstField
        self.unit                  =  0 # Use crc unit zero for icrc computation
        self.hv                    = -1
        self.icrc_hv               = -1
        self.icrc_l4_hv            = -1 #udp.icrc / inner_udp.icrc HV bit to include
                                        #udp.checksum as invariant in icrc
        self.hdrfld_start          = -1
        self.hdrfld_end            = -1
        self.icrc_profile_obj      = None # Icrc profile obj that
                                          # icrc unit will use
        self.P4FieldListCalculation= self.be.h.\
                                              p4_field_list_calculations\
                                                 [VerifyOrUpdateFunc]
        #P4 code should have atleast one input field list.
        ncc_assert(self.P4FieldListCalculation.input[0].fields[0] != None)

        if 'icrc' in \
            self.P4FieldListCalculation._parsed_pragmas.keys():
            if 'update_len' in \
                self.P4FieldListCalculation._parsed_pragmas['icrc']:
                self.icrc_update_len_field = self.P4FieldListCalculation._parsed_pragmas\
                                   ['icrc']['update_len'].keys()[0]
        else:
            self.icrc_update_len_field = ''

        ncc_assert(self.P4FieldListCalculation.input[0].fields[0] != None)
        ncc_assert(self.P4FieldListCalculation != None)
        ncc_assert(self.P4FieldListCalculation.algorithm == 'icrc')
        ncc_assert(self.P4FieldListCalculation.output_width == 32)
        self.l3hdr_name, self.l3hdr_invariant_fields, self.l4_hdr_name, \
        self.l4_hdr_invariant_fields, self.l5_hdr_name, self.l5_hdr_invariant_fields  = \
            self.ProcessIcrcCalFields(self.P4FieldListCalculation, dstField)
        self.roce_hdr = roce_hdr

    def __getitem__(self, index):
        return self

    def ProcessIcrcCalFields(self, field_list_calculation, icrc_field):
        '''
        '''
        icrc_p4_field = None
        icrc_hdr = icrc_field.split(".")[0]
        icrc_field = icrc_field.split(".")[1]
        l3hdr_ifields = []
        for idx, field in enumerate(field_list_calculation.input[0].fields[0:-2]):
            if idx == 0:
                ncc_assert(field.instance.name != icrc_hdr)
                l3hdr_name = field.instance.name
            l3hdr_ifields.append(field)
        l4_hdr_ifields = []
        l4_hdr_name = ''
        field = field_list_calculation.input[0].fields[-2]
        l4_hdr_name = field.instance.name
        l4_hdr_ifields.append(field)
        l5_hdr_ifields = []
        l5_hdr_name = ''
        field = field_list_calculation.input[0].fields[-1]
        l5_hdr_name = field.instance.name
        l5_hdr_ifields.append(field)

        return l3hdr_name, l3hdr_ifields, l4_hdr_name, l4_hdr_ifields,\
               l5_hdr_name, l5_hdr_ifields

    def L3HdrInvariantFieldsGet(self):
        return self.l3hdr_invariant_fields

    def L4HdrInvariantFieldsGet(self):
        return self.l4_hdr_invariant_fields

    def L5HdrInvariantFieldsGet(self):
        return self.l5_hdr_invariant_fields

    def IcrcL3HdrNameGet(self):
        return self.l3hdr_name

    def IcrcL4HdrNameGet(self):
        return self.l4_hdr_name

    def IcrcL5HdrNameGet(self):
        return self.l5_hdr_name

    def CalculatedFieldHdrGet(self):
        hdrinst = self.dstField.split(".")[0]
        return hdrinst

    def IcrcUnitNumGet(self):
        return self.unit

    def IcrcUnitNumSet(self, unit):
        self.unit = unit

    def IcrcHvBitNumSet(self, icrc_hv):
        self.icrc_hv = icrc_hv

    def IcrcHvBitNumGet(self):
        return self.icrc_hv

    def IcrcL4HvBitNumSet(self, icrc_l4_hv):
        self.icrc_l4_hv = icrc_l4_hv

    def IcrcL4HvBitNumGet(self):
        return self.icrc_l4_hv

    def IcrcL5HvBitNumSet(self, icrc_l5_hv):
        self.icrc_l5_hv = icrc_l5_hv

    def IcrcL5HvBitNumGet(self):
        return self.icrc_l5_hv

    def HvBitNumSet(self, hv):
        self.hv = hv

    def HvBitNumGet(self):
        return self.hv

    def HdrFldStartEndSet(self, hdrfld_start, hdrfld_end):
        self.hdrfld_start   = hdrfld_start
        self.hdrfld_end     = hdrfld_end

    def HdrFldStartGet(self):
        return self.hdrfld_start

    def IcrcDeParserProfileObjSet(self, DeParserProfileObj):
        self.icrc_profile_obj = DeParserProfileObj

    def IcrcDeParserProfileObjGet(self):
        return self.icrc_profile_obj

    def ConfigGenerate(self, icrc_hdr_cfg, hdr):
        max_hv_bit_idx = self.be.hw_model['parser']['max_hv_bits'] - 1
        if hdr == 'l3hdr':
            icrc_hdr_cfg['hdr_num']     ['value']   = str(max_hv_bit_idx - self.icrc_hv)
            icrc_hdr_cfg['crc_vld']     ['value']   = str(1)
            icrc_hdr_cfg['crc_unit']    ['value']   = str(self.unit)\
                                                  if self.unit != -1 else str(0)
            icrc_hdr_cfg['hdrfld_start']['value']   = str(self.hdrfld_start)
            icrc_hdr_cfg['hdrfld_end']  ['value']   = str(self.hdrfld_end)
            icrc_hdr_cfg['crc_profile'] ['value']   = str(self.IcrcDeParserProfileObjGet().\
                                                                IcrcProfileNumGet())
            icrc_hdr_cfg['mask_profile']['value']   = str(self.IcrcDeParserProfileObjGet().\
                                                                IcrcMaskProfileNumGet())
            icrc_hdr_cfg['mask_vld']    ['value']   = str(1)
            icrc_hdr_cfg['mask_unit']   ['value']   = str(0) #Mask unit ????
        if hdr == 'l4hdr' or hdr == 'l5hdr':
            if hdr == 'l4hdr':
                #Config to add udp.checksum as invariant/ bth.reserved1 as invariant
                icrc_hdr_cfg['hdr_num']     ['value']   = str(max_hv_bit_idx - self.icrc_l4_hv)
                icrc_hdr_cfg['mask_profile']['value']   = str(self.IcrcDeParserProfileObjGet().\
                                                                IcrcL4MaskProfileNumGet())
            if hdr == 'l5hdr':
                icrc_hdr_cfg['hdr_num']     ['value']   = str(max_hv_bit_idx - self.icrc_l5_hv)
                icrc_hdr_cfg['mask_profile']['value']   = str(self.IcrcDeParserProfileObjGet().\
                                                                IcrcL5MaskProfileNumGet())
            icrc_hdr_cfg['crc_vld']     ['value']   = str(0)
            icrc_hdr_cfg['crc_unit']    ['value']   = str(self.unit)\
                                                  if self.unit != -1 else str(0)
            icrc_hdr_cfg['hdrfld_start']['value']   = str(self.hdrfld_start)
            icrc_hdr_cfg['hdrfld_end']  ['value']   = str(self.hdrfld_end)
            icrc_hdr_cfg['crc_profile'] ['value']   = str(self.IcrcDeParserProfileObjGet().\
                                                                IcrcProfileNumGet())
            icrc_hdr_cfg['mask_vld']    ['value']   = str(1)
            icrc_hdr_cfg['mask_unit']   ['value']   = str(0) #Mask unit ????

        icrc_hdr_cfg['_modified']               = True

    def LogGenerate(self, icrc_hdr):
        max_hv_bit_idx = self.be.hw_model['parser']['max_hv_bits'] - 1
        log_str = ''
        log_str += 'DeParser IcrcConfig: Hdr %s\n' % icrc_hdr
        log_str += '_____________________________________\n\n'
        log_str += '    Icrc Unit %d\n' % self.unit
        log_str += '    crc vld %d\n' % 1
        log_str += '    crc unit %d\n' % self.unit
        log_str += '    HvBit %d\n' % self.hv
        log_str += '    Icrc HvBit %d\n' % (max_hv_bit_idx - self.icrc_hv)
        log_str += '    Icrc profile# %d\n' % self.IcrcDeParserProfileObjGet().IcrcProfileNumGet()
        log_str += '    Icrc Mask profile# %d\n' % self.IcrcDeParserProfileObjGet().IcrcMaskProfileNumGet()
        log_str += '    mask vld %d\n' % 1
        log_str += '    mask unit %d\n' % 0 # ???
        log_str += '    HdrFld Start %d\n' % self.hdrfld_start
        log_str += '    HdrFld End %d\n' % self.hdrfld_end

        return log_str

    def L4LogGenerate(self, icrc_hdr):
        max_hv_bit_idx = self.be.hw_model['parser']['max_hv_bits'] - 1
        log_str = ''
        log_str += 'DeParser IcrcConfig: Hdr %s\n' % icrc_hdr
        log_str += '_____________________________________\n\n'
        log_str += '    Icrc Unit %d\n' % self.unit
        log_str += '    crc vld %d\n' % 1
        log_str += '    crc unit %d\n' % self.unit
        log_str += '    HvBit %d\n' % self.hv
        log_str += '    Icrc HvBit %d\n' % (max_hv_bit_idx - self.icrc_hv)
        log_str += '    Icrc profile# %d\n' % self.IcrcDeParserProfileObjGet().IcrcProfileNumGet()
        log_str += '    Icrc Mask profile# %d\n' % self.IcrcDeParserProfileObjGet().IcrcL4MaskProfileNumGet()
        log_str += '    mask vld %d\n' % 1
        log_str += '    mask unit %d\n' % 0 # ???
        log_str += '    HdrFld Start %d\n' % self.hdrfld_start
        log_str += '    HdrFld End %d\n' % self.hdrfld_end

        return log_str

    def L5LogGenerate(self, icrc_hdr):
        max_hv_bit_idx = self.be.hw_model['parser']['max_hv_bits'] - 1
        log_str = ''
        log_str += 'DeParser IcrcConfig: Hdr %s\n' % icrc_hdr
        log_str += '_____________________________________\n\n'
        log_str += '    Icrc Unit %d\n' % self.unit
        log_str += '    crc vld %d\n' % 1
        log_str += '    crc unit %d\n' % self.unit
        log_str += '    HvBit %d\n' % self.hv
        log_str += '    Icrc HvBit %d\n' % (max_hv_bit_idx - self.icrc_hv)
        log_str += '    Icrc profile# %d\n' % self.IcrcDeParserProfileObjGet().IcrcProfileNumGet()
        log_str += '    Icrc Mask profile# %d\n' % self.IcrcDeParserProfileObjGet().IcrcL5MaskProfileNumGet()
        log_str += '    mask vld %d\n' % 1
        log_str += '    mask unit %d\n' % 0 # ???
        log_str += '    HdrFld Start %d\n' % self.hdrfld_start
        log_str += '    HdrFld End %d\n' % self.hdrfld_end

        return log_str


    def IcrcLogStrTableGet(self):
        return self.logstr_tbl

    def IcrcAddLog(self, logstr):
        self.logstr_tbl.append(logstr)

    def IcrcDeParserConfigMatrixRowLog(self):
        pstr = '{:<12s}{:<5d}{:<7d}{:<7d}{:<8d}{:<8d}{:<8d}{:<6d}{:<9d}{:<8d}'\
               '{:<8d}{:<10d}{:<5d}{:<5d}{:<5d}{:<d}\n'.format(self.l3hdr_name,
                                       self.unit,
                                       self.icrc_hv,
                                       384 + (127 - self.icrc_hv),
                                       self.icrc_profile_obj.icrc_profile,
                                       self.icrc_profile_obj.mask_profile,
                                       self.hdrfld_start,
                                       self.hdrfld_end,
                                       self.icrc_profile_obj.use_phv_len,
                                       self.icrc_profile_obj.phv_len_sel,
                                       self.icrc_profile_obj.start_adj,
                                       self.icrc_profile_obj.start_adj_sub,
                                       self.icrc_profile_obj.end_adj,
                                       self.icrc_profile_obj.end_adj_sub,
                                       self.icrc_profile_obj.icrc_loc_adj,
                                       self.icrc_profile_obj.icrc_loc_adj_sub)

        return pstr

    def IcrcDeParserL4ConfigMatrixRowLog(self):
        pstr = '{:<12s}{:<5d}{:<7d}{:<7d}{:<8s}{:<8d}{:<8d}{:<d}\n'.format(self.l4_hdr_name,
                                       self.unit,
                                       self.icrc_l4_hv,
                                       384 + (127 - self.icrc_l4_hv),
                                       'UnUsed',
                                       self.icrc_profile_obj.l4_mask_profile,
                                       self.hdrfld_start,
                                       self.hdrfld_end)

        return pstr

    def IcrcDeParserL5ConfigMatrixRowLog(self):
        pstr = '{:<12s}{:<5d}{:<7d}{:<7d}{:<8s}{:<8d}{:<8d}{:<d}\n'.format(self.l5_hdr_name,
                                       self.unit,
                                       self.icrc_l5_hv,
                                       384 + (127 - self.icrc_l5_hv),
                                       'UnUsed',
                                       self.icrc_profile_obj.l5_mask_profile,
                                       self.hdrfld_start,
                                       self.hdrfld_end)

        return pstr



class Icrc:
    '''
     NB:  Inorder to efficiently use parser OHI resources, process ICRC p4 objects
          after processing p4 csum objects. This will help in reusing resources.

     Code to facilitate ICRC computation in parser to verify incoming packets icrc
     is correct or not. For outgoing packet, enable pipeline to insert icrc.

     icrc calculation starts from entire IP datagram (including IP Hdr) until end
     of payload. In case of encap pkt, icrc is calculated from inner L3 hdr onwards.
    '''

    def __init__(self, capri_be):
        self.be                     = capri_be
        self.logstr_tbl             = []
        self.icrc_verify_logger     = logging.getLogger('ICRC_V')
        self.icrc_compute_logger    = logging.getLogger('ICRC_C')
        self.verify_cal_fieldlist   = [] #List of CalField Objects; icrc verified in ingress pipeline
        self.eg_verify_cal_fieldlist= [] #List of CalField Objects; icrc verified in egress pipeline
        self.update_cal_fieldlist   = [] #List of CalField Objects; icrc computed in ingress pipeline
        self.eg_update_cal_fieldlist= [] #List of CalField Objects; icrc computed in egress pipeline
        self.icrc_profiles_allocated        = 0
        self.icrc_mask_profiles_allocated   = 0
        self.icrc_dp_profiles_allocated     = 0
        self.icrc_dp_mask_profiles_allocated= 0
        self.l3hdr_to_profile_map           = {}
        self.l3hdr_to_profile_map_dp        = {}
        self.l4_hdr_to_profile_map_dp       = {}
        self.l5_hdr_to_profile_map_dp       = {}
        self.dpr_hw_icrc_obj                = [] #Sorted list of calfldobj; sorted by fldstart used in ingress pipeline
        self.eg_dpr_hw_icrc_obj             = [] #Sorted list of calfldobj; sorted by fldstart used in egress pipeline

    def icrc_direction_get(self, field_list_cal):
        p4_calfld_obj = self.be.h.p4_field_list_calculations[field_list_cal]
        if 'gress' in p4_calfld_obj._parsed_pragmas['icrc']:
            gress = p4_calfld_obj._parsed_pragmas['icrc']['gress'].keys()[0]
            return xgress.EGRESS if gress == 'egress' else xgress.INGRESS
        return None

    def initialize(self):
        '''
        '''
        for cal_fld in self.be.h.calculated_fields:
            field_dst, fld_ops, _, _  = cal_fld
            for ops in fld_ops:
                if self.be.h.p4_field_list_calculations[ops[1]].algorithm != 'icrc':
                    #calculated field objects are not icrc(skip checksum related obj)
                    continue
                d = self.icrc_direction_get(ops[1])
                if ops[0] == 'verify':
                    if d == xgress.INGRESS or d == None or d == 'XGRESS':
                        self.verify_cal_fieldlist.append(IcrcParserCalField(\
                                                     self.be, \
                                                     field_dst, ops[1], ops[2].right))
                    if d == xgress.EGRESS or d == 'XGRESS':
                        self.eg_verify_cal_fieldlist.append(IcrcParserCalField(\
                                                     self.be, \
                                                     field_dst, ops[1], ops[2].right))
                else:
                    if d == xgress.INGRESS or d == 'XGRESS':
                        self.update_cal_fieldlist.append(IcrcDeParserCalField(\
                                                     self.be, \
                                                     field_dst, ops[1], ops[2].right))
                    if d == xgress.EGRESS or d == 'XGRESS' or d == None:
                        self.eg_update_cal_fieldlist.append(IcrcDeParserCalField(\
                                                     self.be, \
                                                     field_dst, ops[1], ops[2].right))

    def ProcessIcrcObjects(self, d):
        self.ProcessIcrcVerificationCalFldList(self.be.parsers[d])
        self.ProcessIcrcUpdateCalFldList(self.be.parsers[d])

    def AllocateIcrcObjects(self, d):
        self.AllocateParserIcrcResources(self.be.parsers[d])
        self.AllocateDeParserIcrcResources(self.be.parsers[d])

    def IsHdrRoceV2(self, hdr_name, d):
        verify_cal_fieldlist = self.verify_cal_fieldlist if d == xgress.INGRESS \
                                                else self.eg_verify_cal_fieldlist
        update_cal_fieldlist = self.update_cal_fieldlist if d == xgress.INGRESS \
                                                else self.eg_update_cal_fieldlist
        for calfldobj in verify_cal_fieldlist:
            if calfldobj.roce_hdr.name == hdr_name:
                return True
        for calfldobj in update_cal_fieldlist:
            if calfldobj.roce_hdr.name == hdr_name:
                return True
        return False


    #   --------  iCRC verification related Code --------


    def VerifyIcrcCalFieldObjGet(self, l3hdr, l4hdr, l5hdr, d):
        # Given l3hdr or l4/l5 hdr return Calculated Field Obj that is verified
        verify_cal_fieldlist = self.verify_cal_fieldlist if d == xgress.INGRESS \
                                                     else self.eg_verify_cal_fieldlist
        if l3hdr == '' or l4hdr == '' or l5hdr == '':
            return None
        for calflistobj in verify_cal_fieldlist:
            if calflistobj.IcrcL3HdrNameGet() == l3hdr and \
               calflistobj.IcrcL4HdrNameGet() == l4hdr and \
               calflistobj.IcrcL5HdrNameGet() == l5hdr:
                return calflistobj
        return None


    def IsHdrInIcrcVerify(self, hdrname, d):
        verify_cal_fieldlist = self.verify_cal_fieldlist if d == xgress.INGRESS \
                                                     else self.eg_verify_cal_fieldlist
        if hdrname == '':
            return False
        for calflistobj in verify_cal_fieldlist:
            if calflistobj.IcrcL3HdrNameGet() == hdrname or\
               calflistobj.IcrcL4HdrNameGet() == hdrname or\
               calflistobj.IcrcL5HdrNameGet() == hdrname:
                return True
        return False

    def IcrcParserL3HdrIFldProfileBuild(self, calfldobj):
        if calfldobj.l3hdr_name not in self.l3hdr_to_profile_map.keys():
            self.l3hdr_to_profile_map[calfldobj.l3hdr_name] = \
                (self.icrc_profiles_allocated, self.icrc_mask_profiles_allocated)
            self.icrc_profiles_allocated += 1
            self.icrc_mask_profiles_allocated += 1

        profile_num, mask_profile_num = self.l3hdr_to_profile_map[calfldobj.l3hdr_name]

        prof_obj = calfldobj.IcrcParserProfileObjGet()
        prof_obj.IcrcProfileNumSet(profile_num)
        prof_obj.IcrcMaskProfileNumSet(mask_profile_num)
        #Subtract 8 bytes from the start of L3 hdr so that
        #64 1bits are added to icrc computation.
        prof_obj.IcrcProfileStartAdjSet(0, 8)
        prof_obj.IcrcProfileShiftLeftSet(0, 0)
        prof_obj.IcrcProfileEndAdjSet(0, 0)
        prof_obj.IcrcProfileMaskAdjSet(0, 0)
        #Icrc calculation till end of packet.
        if calfldobj.icrc_verify_len_field == '':
            #Icrc calculation till end of packet; verify_len option not used.
            prof_obj.IcrcProfileEndEopSet(1)

        #Build mask profile for invariant fields.

        #Fill 64bit 1's before L3 hdr.
        #Parser has no knob to add 64 1's. Use mask field.
        #The total mask_field count is now 6 in parser.
        fld_inst                = 0
        mask_field              = {}
        mask_field['mask_en']   = 1
        mask_field['use_ohi']   = 0
        mask_field['start_adj'] = 0
        mask_field['end_adj']   = 7  #End is inclusive in HW
        mask_field['fill']  = 1
        mask_field['skip_first_nibble']  = 0
        prof_obj.IcrcMaskProfileMaskFieldAdd(fld_inst, mask_field)

        leading_64b_byte_len    = 8
        l3hdr_iflds = calfldobj.L3HdrInvariantFieldsGet()
        fld_inst = 1
        for l3hdr_ifld in l3hdr_iflds:
            mask_field              = {}
            mask_field['mask_en']   = 1
            #For L3 Iflds, use crc-start-offset to program mask profile.
            #Use mask-ohi for udp.csum field.
            mask_field['use_ohi']   = 0
            mask_field['start_adj'] = l3hdr_ifld.offset / 8
            mask_field['end_adj']   = (l3hdr_ifld.offset + l3hdr_ifld.width) / 8 - 1 #End is inclusive in HW
            mask_field['fill']      = 1
            mask_field['skip_first_nibble']  = 0
            if l3hdr_ifld.offset % 8 == 4:
                #field starts on nibble
                mask_field['skip_first_nibble']  = 1
                if not l3hdr_ifld.width % 8:
                    #start in middle of byte and ends in middle of byte
                    #hence move end_adj by one more byte
                    mask_field['end_adj']   += 1
                #NB:
                # TC in ipv6 hdr starts  @bit 4 and ends @bit 11. Since there is no way
                # to skip end nibble, bit12 to bit15 are also marked invariant. This
                # works because FLow-Label starts at bit12 and ends @bit31 and is also
                # invariant field.
            mask_field['start_adj'] += leading_64b_byte_len
            mask_field['end_adj']   += leading_64b_byte_len
            prof_obj.IcrcMaskProfileMaskFieldAdd(fld_inst, mask_field)
            fld_inst += 1

    def IcrcParserL4HdrIFldProfileBuild(self, calfldobj):
        l4_hdr_iflds = calfldobj.L4HdrInvariantFieldsGet()
        prof_obj = calfldobj.IcrcParserProfileObjGet()
        fld_inst = prof_obj.IcrcMaskProfileMaskFieldLenGet()
        for hdr_ifld in l4_hdr_iflds:
            mask_field              = {}
            mask_field['mask_en']   = 1
            #For L3 Iflds, use crc-start-offset to program mask profile.
            #Use mask-ohi for udp.csum field.
            mask_field['use_ohi']   = 1
            mask_field['start_adj'] = hdr_ifld.offset / 8
            mask_field['end_adj']   = (hdr_ifld.offset + hdr_ifld.width) / 8 - 1 #End is inclusive in HW
            mask_field['fill']      = 1
            mask_field['skip_first_nibble']  = 0
            if hdr_ifld.offset % 8 == 4:
                #field starts on nibble
                mask_field['skip_first_nibble']  = 1
                if not hdr_ifld.width % 8:
                    #start in middle of byte and ends in middle of byte
                    #hence move end_adj by one more byte
                    mask_field['end_adj']   += 1
            prof_obj.IcrcL4MaskProfileMaskFieldAdd(fld_inst, mask_field)

    def IcrcParserL5HdrIFldProfileBuild(self, calfldobj):
        l5_hdr_iflds = calfldobj.L5HdrInvariantFieldsGet()
        prof_obj = calfldobj.IcrcParserProfileObjGet()
        fld_inst = prof_obj.IcrcMaskProfileMaskFieldLenGet() + \
                   prof_obj.IcrcL4MaskProfileMaskFieldLenGet()
        for hdr_ifld in l5_hdr_iflds:
            mask_field              = {}
            mask_field['mask_en']   = 1
            #For L3 Iflds, use crc-start-offset to program mask profile.
            #Use mask-ohi for udp.csum field / bth.reserved1.
            mask_field['use_ohi']   = 1
            mask_field['start_adj'] = (hdr_ifld.offset / 8) + 8 #TODO: Get size of UDP header 
            mask_field['end_adj']   = ((hdr_ifld.offset + hdr_ifld.width) / 8) - 1  + 8 #End is inclusive in HW
            mask_field['fill']      = 1
            mask_field['skip_first_nibble']  = 0
            if hdr_ifld.offset % 8 == 4:
                #field starts on nibble
                mask_field['skip_first_nibble']  = 1
                if not hdr_ifld.width % 8:
                    #start in middle of byte and ends in middle of byte
                    #hence move end_adj by one more byte
                    mask_field['end_adj']   += 1
            prof_obj.IcrcL5MaskProfileMaskFieldAdd(fld_inst, mask_field)

    def ProcessIcrcVerificationCalFldList(self, parser):
        '''
        This function will process all verifiable calculated fields
        and creates icrc objects
        '''
        verify_cal_fieldlist = self.verify_cal_fieldlist if parser.d == xgress.INGRESS \
                                                     else self.eg_verify_cal_fieldlist
        icrc_l3_hdrs = []
        for calfldobj in verify_cal_fieldlist:
            ncc_assert(calfldobj != None)
            calfldhdr = calfldobj.CalculatedFieldHdrGet()
            ncc_assert(calfldhdr != None)
            l3_name = calfldobj.IcrcL3HdrNameGet()
            ncc_assert(l3_name != '')
            ncc_assert(l3_name in self.be.h.p4_header_instances)
            #Also allocate l3 profile obj
            calfldobj.IcrcParserProfileObjSet(IcrcParserProfile())
            icrc_l3_hdrs.append(l3_name)

        self.icrc_verify_logger.debug('Icrc L3 Hdrs =  %s' % (str(icrc_l3_hdrs)))

        # Ensure/CrossCheck that every calculated field that needs to
        # be verified has been created and also build profile.
        for calfldobj in verify_cal_fieldlist:
            if calfldobj.IcrcParserProfileObjGet() == None:
                ncc_assert(0)
            #L4HdrIFldProfileBuild should be invoked after calling L3hdrProfileBuild
            self.IcrcParserL3HdrIFldProfileBuild(calfldobj)
            self.IcrcParserL4HdrIFldProfileBuild(calfldobj)
            self.IcrcParserL5HdrIFldProfileBuild(calfldobj)

    def InsertIcrcObjReferenceInParseState(self, parser):
        '''
            In parse states where icrc L3 hdrs are extracted, and where
            roce header is extracted, insert reference to p4 cal fld objects
            so that Parser block can be programmed to trigger icrc verification.
        '''
        icrc_l3hdrs = set()
        icrc_l4_hdrs = set()
        icrc_l5_hdrs = set()
        all_icrc_l3hdrs = set()
        all_icrc_l4_hdrs = set()
        all_icrc_l5_hdrs = set()
        verify_cal_fieldlist = self.verify_cal_fieldlist if parser.d == xgress.INGRESS \
                                                     else self.eg_verify_cal_fieldlist
        for calfldobj in verify_cal_fieldlist:
            icrc_l3hdr = calfldobj.IcrcL3HdrNameGet()
            icrc_l4_hdr = calfldobj.IcrcL4HdrNameGet()
            icrc_l5_hdr = calfldobj.IcrcL5HdrNameGet()
            if icrc_l3hdr != '' and icrc_l3hdr not in icrc_l3hdrs:
                icrc_l3hdrs.add(icrc_l3hdr)
                all_icrc_l3hdrs.add(icrc_l3hdr)
            if icrc_l4_hdr != '' and icrc_l4_hdr not in icrc_l4_hdrs:
                icrc_l4_hdrs.add(icrc_l4_hdr)
                all_icrc_l4_hdrs.add(icrc_l4_hdr)
            if icrc_l5_hdr != '' and icrc_l5_hdr not in icrc_l5_hdrs:
                icrc_l5_hdrs.add(icrc_l5_hdr)
                all_icrc_l5_hdrs.add(icrc_l5_hdr)
        #Find parse states where reference to icrc calfldobj should be added.
        for parsepath in parser.paths:
            l3hdr = None
            l4hdr = None
            l5hdr = None
            for hdr in parsepath:
                if hdr.name in all_icrc_l3hdrs:
                    l3hdr = hdr
                if hdr.name in all_icrc_l4_hdrs:
                    l4hdr = hdr
                if hdr.name in all_icrc_l5_hdrs:
                    l5hdr = hdr


            if l3hdr and l4hdr and l5hdr:
                calfldobj = self.VerifyIcrcCalFieldObjGet(l3hdr.name, l4hdr.name, l5hdr.name, parser.d)
            else:
                continue

            if calfldobj == None:
                continue

            for parsestate in parser.get_ext_cstates(l3hdr):
                if ('L3_IFLD', calfldobj) not in parsestate.icrc_verify_cal_field_objs:
                    parsestate.icrc_verify_cal_field_objs.\
                                            append(("L3_IFLD", calfldobj))
                    if l3hdr.name in icrc_l3hdrs:
                        icrc_l3hdrs.remove(l3hdr.name)

            for parsestate in parser.get_ext_cstates(l4hdr):
                if ('L4_IFLD', calfldobj) not in parsestate.icrc_verify_cal_field_objs:
                    parsestate.icrc_verify_cal_field_objs.\
                                            append(("L4_IFLD", calfldobj))
                    if l4hdr.name in icrc_l4_hdrs:
                        icrc_l4_hdrs.remove(l4hdr.name)
                #Also add L3 calfldobj in this parse state. Depending upon parse branches
                #correct calfldobj will be used in IcrcParserConfigGenerate()
                if ('L3_IFLD', calfldobj) not in parsestate.icrc_verify_cal_field_objs:
                    parsestate.icrc_verify_cal_field_objs.\
                                            append(("L3_IFLD", calfldobj))

            for parsestate in parser.get_ext_cstates(l5hdr):
                if ('L5_IFLD', calfldobj) not in parsestate.icrc_verify_cal_field_objs:
                    parsestate.icrc_verify_cal_field_objs.\
                                            append(("L5_IFLD", calfldobj))
                    if l5hdr.name in icrc_l5_hdrs:
                        icrc_l5_hdrs.remove(l5hdr.name)
                #Also add L3 calfldobj in this parse state. Depending upon parse branches
                #correct calfldobj will be used in IcrcParserConfigGenerate()
                if ('L3_IFLD', calfldobj) not in parsestate.icrc_verify_cal_field_objs:
                    parsestate.icrc_verify_cal_field_objs.\
                                            append(("L3_IFLD", calfldobj))

        ncc_assert((len(icrc_l5_hdrs) == 0 and len(icrc_l4_hdrs) == 0 and \
               len(icrc_l3hdrs) == 0))


    def AllocateParserIcrcResources(self, parser):
        icrc_l3hdrs = set()
        verify_cal_fieldlist = self.verify_cal_fieldlist if parser.d == xgress.INGRESS \
                                                     else self.eg_verify_cal_fieldlist
        for calfldobj in verify_cal_fieldlist:
            icrc_l3hdr = calfldobj.IcrcL3HdrNameGet()
            if icrc_l3hdr != '' and icrc_l3hdr not in icrc_l3hdrs:
                icrc_l3hdrs.add(icrc_l3hdr)
        if not len(icrc_l3hdrs):
            return

        icrc_objects = set(calfldobj for calfldobj in verify_cal_fieldlist)
        all_roce_hdr_names = set(calfldobj.roce_hdr.name for calfldobj in verify_cal_fieldlist)
        all_parse_paths = sorted(parser.paths, key=lambda p: len(p), reverse=True)
        for parse_path in all_parse_paths:
            program_icrc = False
            parse_path_hdrs = set(hdr.name for hdr in parse_path)

            rhdr = parse_path_hdrs.intersection(all_roce_hdr_names)
            if len(rhdr):
                ncc_assert(len(rhdr) == 1)
                roce_hdr_name = list(rhdr)[0]
                program_icrc = True
            if program_icrc:
                l3_hdrs = None
                _s = parse_path_hdrs.intersection(icrc_l3hdrs)
                if len(_s):
                    l3_hdrs = _s
                else:
                    #roce hdr in parse path; but no associated L3 hdr is in the path.
                    ncc_assert(0)

                for _i, hdr in enumerate(parse_path):
                    if hdr.name != roce_hdr_name:
                        continue
                    #Associate correct l3hdr and l4hdr that is the covering roce_hdr (if multiple layers of tunnels)
                    l4hdr = parse_path[_i-1].name
                    l3hdr = parse_path[_i-2].name
                    icrc_calfldobj = \
                       self.VerifyIcrcCalFieldObjGet(l3hdr, l4hdr, roce_hdr_name, parser.d)

                    if icrc_calfldobj in icrc_objects:
                        #In all the L3hdr Parse States, 2 things are needed
                        #Capture start of L3 hdr in OHI and Load ip.TotalLen/PayLoadLen
                        #in OHI. This can be accomplished via reusing L3 start
                        #captured for csum if done (P4 has csum objects) or
                        #generated parser instructions to perform these in L3Hdr
                        #parse states
                        #Another option to provide length to parser is to
                        #not use len ohi and instead use EOP config. This
                        #option makes it easy to use versus using any payload
                        #len calculation parser local variable (or reusing
                        #ones that are in used for csum)
                        for parsestate in parser.get_ext_cstates(self.be.h.\
                                                p4_header_instances[l3hdr]):
                            if len(parsestate.verify_cal_field_objs) or \
                               parsestate.phdr_offset_ohi_id != -1:

                                #reuse csum phdr ohi-id
                                if parsestate.phdr_offset_ohi_id != -1:
                                    ohi_start_id = parsestate.phdr_offset_ohi_id
                                else:
                                    #reuse csum hdr ohi-id
                                    ohi_start_id = parsestate.\
                                        verify_cal_field_objs[0].\
                                        ParserCsumObjGet().CsumOhiStartSelGet()

                                #When processing csum constructs, OHI ID with L3
                                #offset is already loaded. Reusing same OHI
                                icrc_calfldobj.IcrcOhiStartSelSet(ohi_start_id)

                                if icrc_calfldobj.icrc_verify_len_field != '':
                                    ohi_id = parser.get_ohi_slot_wr_only_field_name(\
                                      icrc_calfldobj.icrc_verify_len_field.split('.')[1])
                                    icrc_calfldobj.IcrcOhiLenSelSet(ohi_id)
                                else:
                                    #when verify length is not computed in parser,
                                    #icrc computation is from L3 to till end of packet
                                    icrc_calfldobj.IcrcOhiLenSelSet(0)
                            else:
                                ohi_start_id = parser.get_ohi_hdr_start_off(\
                                            self.be.h.p4_header_instances[l3hdr])
                                ncc_assert(ohi_start_id != None)
                                icrc_calfldobj.IcrcOhiStartSelSet(ohi_start_id)

                                if icrc_calfldobj.icrc_verify_len_field != '':
                                    ohi_len_id = parser.get_ohi_slot_wr_only_field_name(\
                                      icrc_calfldobj.icrc_verify_len_field.split('.')[1])
                                    ncc_assert(ohi_len_id != None)
                                    icrc_calfldobj.IcrcOhiLenSelSet(ohi_len_id)
                                else:
                                    #when verify length is not computed in parser,
                                    #icrc computation is from L3 to till end of packet
                                    icrc_calfldobj.IcrcOhiLenSelSet(0)

                            self.icrc_verify_logger.debug(\
                             'Icrc Assignment along path %s' % (str(parse_path)))
                            self.icrc_verify_logger.debug(\
                             'Icrc %s Profile# %d, MaskProfile %d, OhiStart %d \
                              OhiLen %d OhiMask %d' % \
                             (icrc_calfldobj.dstField, \
                              icrc_calfldobj.IcrcParserProfileObjGet().IcrcProfileNumGet(),
                              icrc_calfldobj.IcrcParserProfileObjGet().IcrcMaskProfileNumGet(),
                              icrc_calfldobj.IcrcOhiStartSelGet(),
                              icrc_calfldobj.IcrcOhiLenSelGet(),
                              icrc_calfldobj.IcrcOhiMaskSelGet()))
                            self.icrc_verify_logger.debug('\n')

                        l4_hdr_name = icrc_calfldobj.IcrcL4HdrNameGet()
                        for parsestate in parser.get_ext_cstates(self.be.h.\
                                                p4_header_instances[l4_hdr_name]):
                            if len(parsestate.verify_cal_field_objs):
                                ohi_start_id = parsestate.verify_cal_field_objs[0].\
                                           ParserCsumObjGet().CsumOhiStartSelGet()

                                #When processing csum constructs, OHI ID with L3
                                #offset is already loaded. Reusing same OHI
                                #For masking out invariant fields in UDP, use OHI
                                #slot# that is used for capturing UDP hdr start.
                                icrc_calfldobj.IcrcOhiMaskSelSet(ohi_start_id)
                            else:
                                ohi_start_id = parser.get_ohi_hdr_start_off(\
                                            self.be.h.p4_header_instances[l4_hdr_name])
                                ncc_assert(ohi_start_id != None)
                                icrc_calfldobj.IcrcOhiMaskSelSet(ohi_start_id)

                            self.icrc_verify_logger.debug(\
                             'Icrc Assignment along path %s' % (str(parse_path)))
                            self.icrc_verify_logger.debug(\
                             'Icrc %s Profile# %d, MaskProfile %d, OhiStart %d \
                              OhiLen %d OhiMask %d' % \
                             (icrc_calfldobj.dstField, \
                              icrc_calfldobj.IcrcParserProfileObjGet().IcrcProfileNumGet(),
                              icrc_calfldobj.IcrcParserProfileObjGet().IcrcMaskProfileNumGet(),
                              icrc_calfldobj.IcrcOhiStartSelGet(),
                              icrc_calfldobj.IcrcOhiLenSelGet(),
                              icrc_calfldobj.IcrcOhiMaskSelGet()))
                            self.icrc_verify_logger.debug('\n')
                        # add bth.reserved1 as invariant is added related to UDP ohi.

                        icrc_objects.remove(icrc_calfldobj)
                    #There is no need to loop through headers that are beyond roce_hdr in the parse_path.
                    break

                if not len(icrc_objects):
                    break

        #Assert if all calfld objects are allocated resources
        ncc_assert(len(icrc_objects) == 0)

        #In parse states where L3,L4 hdrs are extracted, and where roce_bth
        #is extracted, insert reference to calculated fld objects so that
        #Parser block can be programmed to trigger icrc verification.
        self.InsertIcrcObjReferenceInParseState(parser)

    def IcrcFindCalFldObjMatchingL3hdr(self, parse_state, parse_states_in_path):
        headers_in_parse_path = []
        for _parse_state in parse_states_in_path:
            headers_in_parse_path += _parse_state.headers
        headers_in_parse_path.reverse()
        icrc_l3hdrs = set()

        for hdr_type, calfldobj in parse_state.icrc_verify_cal_field_objs:
            if hdr_type == 'L3_IFLD':
                icrc_l3hdr = calfldobj.IcrcL3HdrNameGet()
                if icrc_l3hdr != '' and icrc_l3hdr not in icrc_l3hdrs:
                    icrc_l3hdrs.add(icrc_l3hdr)

        ncc_assert(len(icrc_l3hdrs) > 0)

        matched_calobj = False
        matched_l3hdr = False
        for l3hdr in headers_in_parse_path:
            if l3hdr.name in icrc_l3hdrs:
                matched_l3hdr = True
                break
        for hdr_type, calfldobj in parse_state.icrc_verify_cal_field_objs:
            if hdr_type == 'L3_IFLD':
                if calfldobj.IcrcL3HdrNameGet() == l3hdr.name:
                    matched_calobj = True
                    break
        ncc_assert(matched_calobj and matched_l3hdr)

        return hdr_type, calfldobj

    def IcrcParserConfigGenerate(self, parser, parse_states_in_path,\
                                 parse_state, sram):

        from_parse_state = parse_states_in_path[-2]
        headers_in_parse_path = []
        for _parse_state in parse_states_in_path:
            headers_in_parse_path += _parse_state.headers

        log_str = ''
        log_str += 'IcrcConfig: %s --> %s\n' \
                    % (from_parse_state.name, parse_state.name)
        log_str += '_____________________________________\n\n'

        log_str += 'Headers in the parse path %s\n\n\n' % (headers_in_parse_path)

        if len(parse_state.icrc_verify_cal_field_objs) > 1:
            hdr_type, calfldobj = self.IcrcFindCalFldObjMatchingL3hdr(\
                                              parse_state, parse_states_in_path)
        else:
            hdr_type, calfldobj = parse_state.icrc_verify_cal_field_objs[0]

        if calfldobj == None:
            ncc_assert(0)

        hdr_ohi_id = calfldobj.IcrcOhiStartSelGet()
        mask_ohi_id = calfldobj.IcrcOhiMaskSelGet()
        len_ohi_id = calfldobj.IcrcOhiLenSelGet()

        ncc_assert(hdr_ohi_id != -1)
        ncc_assert(len_ohi_id != -1)
        ncc_assert(mask_ohi_id != -1)

        extracted_hdrs = [hdr for hdr in parse_state.headers]
        if calfldobj.roce_hdr in extracted_hdrs:
            icrc_enable = True
            use_latched_profile_from_l3_state = True
        elif hdr_type == 'L4_IFLD' or hdr_type == 'L5_IFLD':
            icrc_enable = False
            use_latched_profile_from_l3_state = True
        else:
            icrc_enable = False
            use_latched_profile_from_l3_state = False

        #there is only one parser instruction for icrc cal.
        icrc_instr = 0

        log_str += IcrcParserCalField._build_icrc_instr(sram, 
                      calfldobj, 1 if icrc_enable else 0,\
                      0 if use_latched_profile_from_l3_state else 1,\
                      calfldobj.IcrcParserProfileObjGet().IcrcProfileNumGet(),\
                      hdr_ohi_id, len_ohi_id, mask_ohi_id)

        calfldobj.IcrcAddLog(log_str)


    def ParserIcrcProfileGenerate(self, parser, parse_states_in_path,\
                                  parse_state, icrc_t):
        profile = None
        profile_obj = None
        p = -1


        # Since this function is called on parse state where
        # calculated field may not be extracted, check for
        # existence of calfld obj
        if not len(parse_state.icrc_verify_cal_field_objs):
            return profile, p

        hdr_type, calfldobj = parse_state.icrc_verify_cal_field_objs[0]
        profile_obj = calfldobj.IcrcParserProfileObjGet()
        if profile_obj != None and hdr_type == 'L3_IFLD':
            profile = copy.deepcopy(icrc_t)
            p = profile_obj.icrc_profile
            if p == -1:
                ncc_assert(0)
            profile_obj.ConfigGenerate(profile)
            calfldobj.IcrcAddLog(profile_obj.LogGenerate())
            #Only one calFld processed in any parse state
            return profile, p
        elif profile_obj == None:
            ncc_assert(0)

        return profile, p

    def ParserIcrcMaskProfileGenerate(self, parser, parse_states_in_path,\
                                      parse_state, icrc_mask_t):
        profile = None
        profile_obj = None
        p = -1

        # Since this function is called on parse state where
        # calculated field may not be extracted, check for
        # existence of calfld obj
        if not len(parse_state.icrc_verify_cal_field_objs):
            return profile, p

        hdr_type, calfldobj = parse_state.icrc_verify_cal_field_objs[0]
        profile_obj = calfldobj.IcrcParserProfileObjGet()
        if profile_obj != None and hdr_type == 'L3_IFLD':
            profile = copy.deepcopy(icrc_mask_t)
            p = profile_obj.mask_profile
            if p == -1:
                ncc_assert(0)
            profile_obj.MaskProfileConfigGenerate(profile)
            calfldobj.IcrcAddLog(profile_obj.MaskProfileLogGenerate())
            #Only one calFld processed in any parse state
            return profile, p
        elif profile_obj == None:
            ncc_assert(0)

        return profile, p



    #   --------  iCRC computation related Code --------

    def UpdateIcrcCalFieldObjGet(self, hdr, d):
        # Given l3 hdr or l4 or l5 header name, return icrc Calculated Field Obj that is updated
        update_cal_fieldlist = self.update_cal_fieldlist if d == xgress.INGRESS else self.eg_update_cal_fieldlist
        for calflistobj in update_cal_fieldlist:
            if hdr != '' and (calflistobj.IcrcL3HdrNameGet() == hdr        \
                                or calflistobj.IcrcL4HdrNameGet() == hdr   \
                                or calflistobj.IcrcL5HdrNameGet() == hdr):
                return calflistobj
        return None

    def IsHdrInIcrcCompute(self, hdrname, d):
        return True if self.UpdateIcrcCalFieldObjGet(hdrname, d) else False

    def DeParserIcrcPayLoadLenSlotGet(self, calfldobj, parser):
        ncc_assert(calfldobj.icrc_update_len_field != '')
        cf_icrc_update_len = self.be.pa.get_field(calfldobj.icrc_update_len_field, parser.d)
        dpr_variable_len_phv_start = self.be.hw_model['phv']['flit_size']
        pl_slot = (cf_icrc_update_len.phv_bit - dpr_variable_len_phv_start) / 16

        return pl_slot

    def IcrcDeParserProfileBuild(self, calfldobj, parser):
        if calfldobj.l3hdr_name not in self.l3hdr_to_profile_map_dp.keys():
            self.l3hdr_to_profile_map_dp[calfldobj.l3hdr_name] = \
                (self.icrc_dp_profiles_allocated, self.icrc_dp_mask_profiles_allocated)
            self.icrc_dp_profiles_allocated += 1
            self.icrc_dp_mask_profiles_allocated += 1

        profile_num, mask_profile_num = self.l3hdr_to_profile_map_dp[calfldobj.l3hdr_name]

        prof_obj = calfldobj.IcrcDeParserProfileObjGet()
        prof_obj.IcrcProfileNumSet(profile_num)
        prof_obj.IcrcMaskProfileNumSet(mask_profile_num)
        phv_len_slot = self.DeParserIcrcPayLoadLenSlotGet(calfldobj, parser)
        prof_obj.IcrcProfilePhvLenSelSet(1, phv_len_slot)
        #Subtract 8 bytes from the start of L3 hdr so that
        #64 1bits are added to icrc computation.
        prof_obj.IcrcProfileShiftLeftSet(0, 0)
        prof_obj.IcrcProfileEndAdjSet(4, 1)
        prof_obj.IcrcProfileLocAdjSet(4, 1)
        prof_obj.IcrcProfileStartAdjSet(8, 1)

        #Build mask profile for invariant fields.
        leading_64b_byte_len = 0
        l3hdr_iflds = calfldobj.L3HdrInvariantFieldsGet()
        fld_inst = 0
        span_into_next_byte = 0
        for l3hdr_ifld in l3hdr_iflds:
            mask_field              = {}
            mask_field['en']        = 1
            mask_field['start']     = (l3hdr_ifld.offset / 8) + span_into_next_byte
            mask_field['end']       = (l3hdr_ifld.offset + l3hdr_ifld.width) / 8  - 1 #End is inclusive in HW
            mask_field['fill']      = 1
            mask_field['skip_first_nibble']  = 0
            if not span_into_next_byte and l3hdr_ifld.offset % 8 == 4:
                #field starts on nibble
                mask_field['skip_first_nibble']  = 1
                if not l3hdr_ifld.width % 8:
                    #start in middle of byte and ends in middle of byte
                    #hence move end_adj by one more byte
                    mask_field['end'] += 1
                    span_into_next_byte = 1
                #NB:
                # TC in ipv6 hdr starts  @bit 4 and ends @bit 11. Since there is no way
                # to skip end nibble, bit12 to bit15 are also marked invariant. This
                # works because FLow-Label starts at bit12 and ends @bit31 and is also
                # invariant field.
            elif span_into_next_byte:
                span_into_next_byte = 0
            mask_field['start'] += leading_64b_byte_len
            mask_field['end']   += leading_64b_byte_len
            prof_obj.IcrcMaskProfileMaskFieldAdd(fld_inst, mask_field)
            fld_inst += 1

    def IcrcDeParserL4HdrIFldProfileBuild(self, calfldobj):
        if calfldobj.l4_hdr_name not in self.l4_hdr_to_profile_map_dp.keys():
            self.l4_hdr_to_profile_map_dp[calfldobj.l4_hdr_name] = \
                                    self.icrc_dp_mask_profiles_allocated
            self.icrc_dp_mask_profiles_allocated += 1

        mask_profile_num = self.l4_hdr_to_profile_map_dp[calfldobj.l4_hdr_name]
        l4_hdr_iflds = calfldobj.L4HdrInvariantFieldsGet()
        prof_obj = calfldobj.IcrcDeParserProfileObjGet()
        prof_obj.IcrcL4MaskProfileNumSet(mask_profile_num)
        fld_inst = prof_obj.IcrcMaskProfileMaskFieldLenGet()
        for hdr_ifld in l4_hdr_iflds:
            mask_field              = {}
            mask_field['en']        = 1
            mask_field['start']     = hdr_ifld.offset / 8
            mask_field['end']       = (hdr_ifld.offset + hdr_ifld.width) / 8  - 1 #End is inclusive in HW
            mask_field['fill']      = 1
            mask_field['skip_first_nibble']  = 0
            if hdr_ifld.offset % 8 == 4:
                #field starts on nibble
                mask_field['skip_first_nibble']  = 1
                if not hdr_ifld.width % 8:
                    #start in middle of byte and ends in middle of byte
                    #hence move end_adj by one more byte
                    mask_field['end'] += 1
            prof_obj.IcrcL4MaskProfileMaskFieldAdd(fld_inst, mask_field)
            fld_inst += 1

    def IcrcDeParserL5HdrIFldProfileBuild(self, calfldobj):
        if calfldobj.l5_hdr_name not in self.l5_hdr_to_profile_map_dp.keys():
            self.l5_hdr_to_profile_map_dp[calfldobj.l5_hdr_name] = \
                                    self.icrc_dp_mask_profiles_allocated
            self.icrc_dp_mask_profiles_allocated += 1

        mask_profile_num = self.l5_hdr_to_profile_map_dp[calfldobj.l5_hdr_name]
        l5_hdr_iflds = calfldobj.L5HdrInvariantFieldsGet()
        prof_obj = calfldobj.IcrcDeParserProfileObjGet()
        prof_obj.IcrcL5MaskProfileNumSet(mask_profile_num)
        fld_inst = prof_obj.IcrcMaskProfileMaskFieldLenGet() + \
                   prof_obj.IcrcL4MaskProfileMaskFieldLenGet()
        for hdr_ifld in l5_hdr_iflds:
            mask_field              = {}
            mask_field['en']        = 1
            mask_field['start']     = hdr_ifld.offset / 8
            mask_field['end']       = (hdr_ifld.offset + hdr_ifld.width) / 8  - 1 #End is inclusive in HW
            mask_field['fill']      = 1
            mask_field['skip_first_nibble']  = 0
            if hdr_ifld.offset % 8 == 4:
                #field starts on nibble
                mask_field['skip_first_nibble']  = 1
                if not hdr_ifld.width % 8:
                    #start in middle of byte and ends in middle of byte
                    #hence move end_adj by one more byte
                    mask_field['end'] += 1
            prof_obj.IcrcL5MaskProfileMaskFieldAdd(fld_inst, mask_field)
            fld_inst += 1


    def ProcessIcrcUpdateCalFldList(self, parser):
        '''
         Process all update calculated fields
         and creates icrc objects
        '''
        icrc_l3_hdrs = []
        update_cal_fieldlist = self.update_cal_fieldlist if parser.d == xgress.INGRESS else self.eg_update_cal_fieldlist
        for calfldobj in update_cal_fieldlist:
            ncc_assert(calfldobj != None)
            calfldhdr = calfldobj.CalculatedFieldHdrGet()
            ncc_assert(calfldhdr != None)
            l3_name = calfldobj.IcrcL3HdrNameGet()
            ncc_assert(l3_name != '')
            ncc_assert(l3_name in self.be.h.p4_header_instances)
            #Also allocate l3 profile obj
            calfldobj.IcrcDeParserProfileObjSet(IcrcDeParserProfile())
            icrc_l3_hdrs.append(l3_name)

        self.icrc_compute_logger.debug('Icrc L3 Hdrs =  %s' % (str(icrc_l3_hdrs)))

        # Ensure/CrossCheck that every calculated field that needs to
        # be verified has been created and also build profile.
        for calfldobj in update_cal_fieldlist:
            if calfldobj.IcrcDeParserProfileObjGet() == None:
                ncc_assert(0)
            #L4HdrIFldProfileBuild should be invoked after calling L3hdrProfileBuild
            self.IcrcDeParserProfileBuild(calfldobj, parser)
            self.IcrcDeParserL4HdrIFldProfileBuild(calfldobj)
            self.IcrcDeParserL5HdrIFldProfileBuild(calfldobj)

    def AllocateDeParserIcrcResources(self, parser):
        update_cal_fieldlist = self.update_cal_fieldlist if parser.d == xgress.INGRESS else self.eg_update_cal_fieldlist
        for calfldobj in update_cal_fieldlist:
            icrc_l3hdr = calfldobj.IcrcL3HdrNameGet()
            icrc_hv_and_hf = parser.icrc_hdr_hv_bit[self.be.h.\
                                                       p4_header_instances[icrc_l3hdr]]
            icrc_hv, phv_bit, hfname = icrc_hv_and_hf[0]
            calfldobj.IcrcHvBitNumSet(icrc_hv)
            hdr_valid_phv_bit = parser.hdr_hv_bit[self.be.h.\
                                                 p4_header_instances[icrc_l3hdr]]
            calfldobj.HvBitNumSet(511 - hdr_valid_phv_bit)

            icrc_l4_hdr = calfldobj.IcrcL4HdrNameGet()
            icrc_hv_and_hf = parser.icrc_hdr_hv_bit[self.be.h.\
                                                       p4_header_instances[icrc_l4_hdr]]
            icrc_l4_hv, phv_bit, hfname = icrc_hv_and_hf[0]
            calfldobj.IcrcL4HvBitNumSet(icrc_l4_hv)

            icrc_l5_hdr = calfldobj.IcrcL5HdrNameGet()
            icrc_hv_and_hf = parser.icrc_hdr_hv_bit[self.be.h.\
                                                       p4_header_instances[icrc_l5_hdr]]
            icrc_l5_hv, phv_bit, hfname = icrc_hv_and_hf[0]
            calfldobj.IcrcL5HvBitNumSet(icrc_l5_hv)

    def IcrcDeParserConfigGenerate(self, deparser, hv_fld_slots, dpp_json):
        '''
            Configure HdrFldStart,End and also generate JSON config output.
        '''
        self.icrc_compute_logger.debug('%s' % ("HVB, StartFld, EndFld  HdrName:"))
        self.icrc_compute_logger.debug('%s' % ("-------------------------------"))
        for hvb, hv_info in hv_fld_slots.items():
            self.icrc_compute_logger.debug('%d %d %d %s' % \
            (deparser.be.hw_model['parser']['max_hv_bits'] - 1 - hvb, \
             hv_info[0], hv_info[1], hv_info[2]))
        self.icrc_compute_logger.debug('\n')

        hw_icrcobj = [] # list of icrcobj that need to be programmed in HW
                        # without repeatation and maintaining Banyan contrainst.
        update_cal_fieldlist = self.update_cal_fieldlist if deparser.d == xgress.INGRESS else self.eg_update_cal_fieldlist
        for calfldobj in update_cal_fieldlist:
            l3hdr = calfldobj.IcrcL3HdrNameGet()
            icrc_profile_obj = calfldobj.IcrcDeParserProfileObjGet()
            ncc_assert(icrc_profile_obj != None)
            ncc_assert(calfldobj.hv != -1)
            ncc_assert(calfldobj.icrc_hv != -1)
            fldstart, fldend, _ = hv_fld_slots[calfldobj.icrc_hv]
            calfldobj.HdrFldStartEndSet(fldstart,fldend)

            #Generate Logical Output
            calfldobj.IcrcAddLog(calfldobj.LogGenerate(l3hdr))
            calfldobj.IcrcAddLog(icrc_profile_obj.LogGenerate())
            calfldobj.IcrcAddLog(icrc_profile_obj.MaskProfileLogGenerate())
            hw_icrcobj.append(calfldobj)

        l4_hdrs = []
        self.l4_calfldobjs = []
        for calfldobj in update_cal_fieldlist:
            l4_hdr = calfldobj.IcrcL4HdrNameGet()
            if l4_hdr not in l4_hdrs:
                l4_hdrs.append(l4_hdr)
                fldstart, fldend, _ = hv_fld_slots[calfldobj.icrc_l4_hv]
                new_calfldobj = copy.copy(calfldobj) #deep copy not needed as
                                                     #only fldstart and fldend
                                                     #are updated in copied
                                                     #instance which are object
                                                     #variables.
                new_calfldobj.HdrFldStartEndSet(fldstart,fldend)
                new_calfldobj.logstr_tbl = []
                new_calfldobj.IcrcAddLog(new_calfldobj.L4LogGenerate(l4_hdr))
                icrc_profile_obj = new_calfldobj.IcrcDeParserProfileObjGet()
                new_calfldobj.IcrcAddLog(icrc_profile_obj.L4MaskProfileLogGenerate())
                hw_icrcobj.append(new_calfldobj)
                self.l4_calfldobjs.append(new_calfldobj)

        l5_hdrs = []
        self.l5_calfldobjs = []
        for calfldobj in update_cal_fieldlist:
            l5_hdr = calfldobj.IcrcL5HdrNameGet()
            if l5_hdr not in l5_hdrs:
                l5_hdrs.append(l5_hdr)
                fldstart, fldend, _ = hv_fld_slots[calfldobj.icrc_l5_hv]
                new_calfldobj = copy.copy(calfldobj) #deep copy not needed as
                                                     #only fldstart and fldend
                                                     #are updated in copied
                                                     #instance which are object
                                                     #variables.
                new_calfldobj.HdrFldStartEndSet(fldstart,fldend)
                new_calfldobj.logstr_tbl = []
                new_calfldobj.IcrcAddLog(new_calfldobj.L5LogGenerate(l5_hdr))
                icrc_profile_obj = new_calfldobj.IcrcDeParserProfileObjGet()
                new_calfldobj.IcrcAddLog(icrc_profile_obj.L5MaskProfileLogGenerate())
                hw_icrcobj.append(new_calfldobj)
                self.l5_calfldobjs.append(new_calfldobj)


        #Before generating HW config, sort based on StartFld Value.
        dpr_hw_icrc_obj = sorted(hw_icrcobj, key=lambda obj: obj[0].HdrFldStartGet())
        if deparser.d == xgress.INGRESS:
            self.dpr_hw_icrc_obj =  dpr_hw_icrc_obj
        else:
            self.eg_dpr_hw_icrc_obj =  dpr_hw_icrc_obj
        #Generate ASIC Config
        if deparser.asic == "capri":
            self.IcrcDeParserConfigGenerateCapri(dpr_hw_icrc_obj, deparser, dpp_json)
        elif deparser.asic == "elba":
            self.IcrcDeParserConfigGenerateElba(dpr_hw_icrc_obj, deparser, dpp_json)
        #Json is dumped in the caller to cfg-file.

    def IcrcDeParserConfigGenerateCapri(self, dpr_hw_icrc_obj, deparser, dpp_json):
        icrc_hdr_index = 0
        for _calfldobj in dpr_hw_icrc_obj:
            _icrc_profile_obj = _calfldobj.IcrcDeParserProfileObjGet()
            if _calfldobj not in self.l4_calfldobjs and \
               _calfldobj not in self.l5_calfldobjs:
                icrc_hdr_cfg_name = 'cap_dppcsum_csr_cfg_crc_hdrs[%d]' %\
                                    (icrc_hdr_index)
                _calfldobj.ConfigGenerate(dpp_json['cap_dpp']\
                                    ['registers'][icrc_hdr_cfg_name], 'l3hdr')
                icrc_profile_cfg_name = 'cap_dppcsum_csr_cfg_crc_profile[%d]' %\
                                       _icrc_profile_obj.IcrcProfileNumGet()
                _icrc_profile_obj.ConfigGenerate(dpp_json['cap_dpp']\
                                       ['registers'][icrc_profile_cfg_name])
                icrc_profile_cfg_name = 'cap_dppcsum_csr_cfg_crc_mask_profile[%d]' %\
                                       _icrc_profile_obj.IcrcMaskProfileNumGet()
                _icrc_profile_obj.MaskProfileConfigGenerate(dpp_json['cap_dpp']\
                                       ['registers'][icrc_profile_cfg_name])
            elif _calfldobj in self.l4_calfldobjs:
                icrc_hdr_cfg_name = 'cap_dppcsum_csr_cfg_crc_hdrs[%d]' %\
                                    (icrc_hdr_index)
                _calfldobj.ConfigGenerate(dpp_json['cap_dpp']\
                                    ['registers'][icrc_hdr_cfg_name], 'l4hdr')
                icrc_profile_cfg_name = 'cap_dppcsum_csr_cfg_crc_mask_profile[%d]' %\
                                       _icrc_profile_obj.IcrcL4MaskProfileNumGet()
                _icrc_profile_obj.L4MaskProfileConfigGenerate(dpp_json['cap_dpp']\
                                       ['registers'][icrc_profile_cfg_name])
            elif _calfldobj in self.l5_calfldobjs:
                icrc_hdr_cfg_name = 'cap_dppcsum_csr_cfg_crc_hdrs[%d]' %\
                                    (icrc_hdr_index)
                _calfldobj.ConfigGenerate(dpp_json['cap_dpp']\
                                    ['registers'][icrc_hdr_cfg_name], 'l5hdr')
                icrc_profile_cfg_name = 'cap_dppcsum_csr_cfg_crc_mask_profile[%d]' %\
                                       _icrc_profile_obj.IcrcL5MaskProfileNumGet()
                _icrc_profile_obj.L5MaskProfileConfigGenerate(dpp_json['cap_dpp']\
                                       ['registers'][icrc_profile_cfg_name])

            icrc_hdr_index += 1

        #Deparser expects unused icrc Hdr Slots to be programmed with start fld
        #in increasing order.
        if len(dpr_hw_icrc_obj):
            last_start_fld = _calfldobj.HdrFldStartGet()
            for unfilled_index in  range(icrc_hdr_index, deparser.be.hw_model['deparser']['max_crc_hdrs']):
                icrc_hdr_cfg_name = 'cap_dppcsum_csr_cfg_crc_hdrs[%d]' % unfilled_index
                dpp_json['cap_dpp']['registers'][icrc_hdr_cfg_name]['hdrfld_start']['value'] = \
                                                                                  str(last_start_fld + 1)
                dpp_json['cap_dpp']['registers'][icrc_hdr_cfg_name]['hdrfld_end']['value'] = \
                                                                                  str(last_start_fld + 2)
                dpp_json['cap_dpp']['registers'][icrc_hdr_cfg_name]['_modified'] = True
                last_start_fld += 1

        #Json is dumped in the caller to cfg-file.

    def IcrcDeParserConfigGenerateElba(self, dpr_hw_icrc_obj, deparser, dpp_json):
        icrc_hdr_index = 0
        for _calfldobj in dpr_hw_icrc_obj:
            _icrc_profile_obj = _calfldobj.IcrcDeParserProfileObjGet()
            if _calfldobj not in self.l4_calfldobjs and \
               _calfldobj not in self.l5_calfldobjs:
                icrc_hdr_cfg_name = 'elb_dppcsum_csr_cfg_crc_hdrs[%d]' %\
                                    (icrc_hdr_index)
                _calfldobj.ConfigGenerate(dpp_json['elb_dpp']\
                                    ['registers'][icrc_hdr_cfg_name], 'l3hdr')
                icrc_profile_cfg_name = 'elb_dppcsum_csr_cfg_crc_profile[%d]' %\
                                       _icrc_profile_obj.IcrcProfileNumGet()
                _icrc_profile_obj.ConfigGenerate(dpp_json['elb_dpp']\
                                       ['registers'][icrc_profile_cfg_name])
                icrc_profile_cfg_name = 'elb_dppcsum_csr_cfg_crc_mask_profile[%d]' %\
                                       _icrc_profile_obj.IcrcMaskProfileNumGet()
                _icrc_profile_obj.MaskProfileConfigGenerate(dpp_json['elb_dpp']\
                                       ['registers'][icrc_profile_cfg_name])
            elif _calfldobj in self.l4_calfldobjs:
                icrc_hdr_cfg_name = 'elb_dppcsum_csr_cfg_crc_hdrs[%d]' %\
                                    (icrc_hdr_index)
                _calfldobj.ConfigGenerate(dpp_json['elb_dpp']\
                                    ['registers'][icrc_hdr_cfg_name], 'l4hdr')
                icrc_profile_cfg_name = 'elb_dppcsum_csr_cfg_crc_mask_profile[%d]' %\
                                       _icrc_profile_obj.IcrcL4MaskProfileNumGet()
                _icrc_profile_obj.L4MaskProfileConfigGenerate(dpp_json['elb_dpp']\
                                       ['registers'][icrc_profile_cfg_name])
            elif _calfldobj in self.l5_calfldobjs:
                icrc_hdr_cfg_name = 'elb_dppcsum_csr_cfg_crc_hdrs[%d]' %\
                                    (icrc_hdr_index)
                _calfldobj.ConfigGenerate(dpp_json['elb_dpp']\
                                    ['registers'][icrc_hdr_cfg_name], 'l5hdr')
                icrc_profile_cfg_name = 'elb_dppcsum_csr_cfg_crc_mask_profile[%d]' %\
                                       _icrc_profile_obj.IcrcL5MaskProfileNumGet()
                _icrc_profile_obj.L5MaskProfileConfigGenerate(dpp_json['elb_dpp']\
                                       ['registers'][icrc_profile_cfg_name])

            icrc_hdr_index += 1

        #Deparser expects unused icrc Hdr Slots to be programmed with start fld
        #in increasing order.
        if len(dpr_hw_icrc_obj):
            last_start_fld = _calfldobj.HdrFldStartGet()
            for unfilled_index in  range(icrc_hdr_index, deparser.be.hw_model['deparser']['max_crc_hdrs']):
                icrc_hdr_cfg_name = 'elb_dppcsum_csr_cfg_crc_hdrs[%d]' % unfilled_index
                dpp_json['elb_dpp']['registers'][icrc_hdr_cfg_name]['hdrfld_start']['value'] = \
                                                                                  str(last_start_fld + 1)
                dpp_json['elb_dpp']['registers'][icrc_hdr_cfg_name]['hdrfld_end']['value'] = \
                                                                                  str(last_start_fld + 2)
                dpp_json['elb_dpp']['registers'][icrc_hdr_cfg_name]['_modified'] = True
                last_start_fld += 1


    def IcrcLogicalOutputCreate(self):
        out_dir = self.be.args.gen_dir + '/%s/logs' % (self.be.prog_name)
        if not os.path.exists(out_dir):
            try:
                os.makedirs(out_dir)
            except OSError as e:
                if e.errno != errno.EEXIST:
                    raise

        for d in xgress:
            if d == xgress.INGRESS:
                verify_cal_fieldlist = self.verify_cal_fieldlist
                dpr_hw_icrc_obj      = self.dpr_hw_icrc_obj
            else:
                verify_cal_fieldlist = self.eg_verify_cal_fieldlist
                dpr_hw_icrc_obj      = self.eg_dpr_hw_icrc_obj

            ofile = open('%s/icrc_%s.out' % (out_dir, d.name), "w")
            if len(verify_cal_fieldlist):
                ofile.write("Icrc Verification Config in parser\n")
                ofile.write("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n\n")
                for calfldobj in verify_cal_fieldlist:
                    for log_str in calfldobj.IcrcLogStrTableGet():
                        ofile.write(log_str)

            if len(dpr_hw_icrc_obj):
                ofile.write("Icrc Compute Config in Deparser\n")
                ofile.write("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n\n")
                for calfldobj in dpr_hw_icrc_obj:
                    if calfldobj not in self.l4_calfldobjs and \
                       calfldobj not in self.l5_calfldobjs:
                        for log_str in calfldobj.IcrcLogStrTableGet():
                            ofile.write(log_str)
                    elif calfldobj in self.l4_calfldobjs:
                        ofile.write("L4 Instance Config\n")
                        ofile.write("-----------------\n")
                        for log_str in calfldobj.IcrcLogStrTableGet():
                            ofile.write(log_str)
                    elif calfldobj in self.l5_calfldobjs:
                        ofile.write("L5 Instance Config\n")
                        ofile.write("-----------------\n")
                        for log_str in calfldobj.IcrcLogStrTableGet():
                            ofile.write(log_str)

                ofile.write("Summary: Icrc Compute Config in Deparser\n")
                ofile.write("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n\n")

                pstr = '{:<12s}{:<5s}{:<7s}{:<7s}{:<8s}{:<8s}{:<8s}{:<5s}{:<8s}{:<7s}{:<7s}'\
                       '{:<10s}{:<5s}{:<s}{:<5s}{:<5s}\n'.format("L3/L4/L5 ", "icrc ", "icrc ",
                                                                 "PHV ", "Icrc   ", "Mask   ",
                                                                 "HdrFld ", "HdrFld ", "use ",
                                                                 "phv ", "start ", "startsub ",
                                                                 "end ", "endsub ", "loc ",
                                                                 "locsub")
                pstr += '{:<12s}{:<5s}{:<7s}{:<7s}{:<8s}{:<8s}{:<8s}{:<5s}{:<8s}{:<7s}{:<7s}'\
                        '{:<10s}{:<5s}{:<s}{:<5s}{:<5s}\n'.format("Hdr", "unit ", "HV  ",
                                                                  "    ", "Profile","Profile",
                                                                  "Start ", "End   ", "phv  ",
                                                                  " len ", " adj   ", " adj     ",
                                                                  " adj ", " adj    ", " adj ",
                                                                  "adj   ", " adj ")
                ofile.write(pstr)
                ofile.write("\n")

            for calfldobj in dpr_hw_icrc_obj:
                if calfldobj not in self.l4_calfldobjs and \
                   calfldobj not in self.l5_calfldobjs:
                    ofile.write(calfldobj.IcrcDeParserConfigMatrixRowLog())
                elif calfldobj in self.l4_calfldobjs:
                    ofile.write(calfldobj.IcrcDeParserL4ConfigMatrixRowLog())
                elif calfldobj in self.l5_calfldobjs:
                    ofile.write(calfldobj.IcrcDeParserL5ConfigMatrixRowLog())

            ofile.write("\n\n")
            ofile.close()

