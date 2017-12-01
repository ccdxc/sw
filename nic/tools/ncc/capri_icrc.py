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

    def IcrcMaskProfileMaskFieldLenGet(self):
        return len(self.mask_fields)

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
        log_str += '\n'
        return log_str

# Representation for P4 ICRC Calulated Field.
class IcrcParserCalField:
    '''
    '''
    def __init__(self, capri_be, dstField, VerifyOrUpdateFunc):
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

        self.P4FieldListCalculation     = self.be.h.\
                                            p4_field_list_calculations\
                                            [VerifyOrUpdateFunc]
        #P4 code should have atleast one input field list.
        assert(self.P4FieldListCalculation.input[0].fields[0] != None)
        assert(self.P4FieldListCalculation != None)
        assert(self.P4FieldListCalculation.algorithm == 'icrc')
        assert(self.P4FieldListCalculation.output_width == 32)

        self.l3hdr_name, self.l3hdr_invariant_fields, self.l4_hdr_name, \
        self.l4_hdr_invariant_fields = \
            self.ProcessIcrcCalFields(self.P4FieldListCalculation, dstField)

    def ProcessIcrcCalFields(self, field_list_calculation, icrc_field):
        '''
        '''
        icrc_p4_field = None
        icrc_hdr = icrc_field.split(".")[0]
        icrc_field = icrc_field.split(".")[1]
        l3hdr_ifields = []
        for idx, field in enumerate(field_list_calculation.input[0].fields[0:-1]):
            if idx == 0:
                assert field.instance.name != icrc_hdr, pdb.set_trace()
                l3hdr_name = field.instance.name
            l3hdr_ifields.append(field)
        #Last invariant field in the list is udp.checksum field.
        l4_hdr_ifields = []
        l4_hdr_name = ''
        field = field_list_calculation.input[0].fields[-1]
        l4_hdr_name = field.instance.name
        l4_hdr_ifields.append(field)

        return l3hdr_name, l3hdr_ifields, l4_hdr_name, l4_hdr_ifields


    def CalculatedFieldHdrGet(self):
        hdrinst = self.dstField.split(".")[0]
        return hdrinst

    def L3HdrInvariantFieldsGet(self):
        return self.l3hdr_invariant_fields

    def L4HdrInvariantFieldsGet(self):
        return self.l4_hdr_invariant_fields

    def IcrcParserProfileObjSet(self, IcrcParserProfileObj):
        self.icrc_prsr_profile_obj = IcrcParserProfileObj

    def IcrcParserProfileObjGet(self):
        return self.icrc_prsr_profile_obj

    def IcrcL3HdrNameGet(self):
        return self.l3hdr_name

    def IcrcL4HdrNameGet(self):
        return self.l4_hdr_name

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

        self.mask_profile       = 0
        self.l4_mask_profile   = 0
        #Mask profile fields.
        self.mask_fields        = {} #Key is one of 5 mask fld instances
        self.l4_mask_fields        = {} #Key is one of 5 mask fld instances

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

    def IcrcMaskProfileMaskFieldLenGet(self):
        return len(self.mask_fields)

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


class IcrcDeParserCalField:
    '''
    '''
    def __init__(self, capri_be, dstField, VerifyOrUpdateFunc):
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
        assert(self.P4FieldListCalculation.input[0].fields[0] != None)

        if 'icrc' in \
            self.P4FieldListCalculation._parsed_pragmas.keys():
            if 'update_len' in \
                self.P4FieldListCalculation._parsed_pragmas['icrc']:
                self.icrc_update_len_field = self.P4FieldListCalculation._parsed_pragmas\
                                   ['icrc']['update_len'].keys()[0]
        else:
            self.icrc_update_len_field = ''

        assert(self.P4FieldListCalculation.input[0].fields[0] != None)
        assert(self.P4FieldListCalculation != None)
        assert(self.P4FieldListCalculation.algorithm == 'icrc')
        assert(self.P4FieldListCalculation.output_width == 32)
        self.l3hdr_name, self.l3hdr_invariant_fields, self.l4_hdr_name, \
        self.l4_hdr_invariant_fields = \
            self.ProcessIcrcCalFields(self.P4FieldListCalculation, dstField)

    def __getitem__(self, index):
        return self

    def ProcessIcrcCalFields(self, field_list_calculation, icrc_field):
        '''
        '''
        icrc_p4_field = None
        icrc_hdr = icrc_field.split(".")[0]
        icrc_field = icrc_field.split(".")[1]
        l3hdr_ifields = []
        for idx, field in enumerate(field_list_calculation.input[0].fields[0:-1]):
            if idx == 0:
                assert field.instance.name != icrc_hdr, pdb.set_trace()
                l3hdr_name = field.instance.name
            l3hdr_ifields.append(field)
        #Last invariant field in the list is udp.checksum field.
        l4_hdr_ifields = []
        l4_hdr_name = ''
        field = field_list_calculation.input[0].fields[-1]
        l4_hdr_name = field.instance.name
        l4_hdr_ifields.append(field)

        return l3hdr_name, l3hdr_ifields, l4_hdr_name, l4_hdr_ifields

    def L3HdrInvariantFieldsGet(self):
        return self.l3hdr_invariant_fields

    def L4HdrInvariantFieldsGet(self):
        return self.l4_hdr_invariant_fields

    def IcrcL3HdrNameGet(self):
        return self.l3hdr_name

    def IcrcL4HdrNameGet(self):
        return self.l4_hdr_name

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

    def ConfigGenerate(self, icrc_hdr_cfg, l3hdr):
        max_hv_bit_idx = self.be.hw_model['parser']['max_hv_bits'] - 1
        if l3hdr:
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
        else:
            #Config to add udp.checksum as invariant
            icrc_hdr_cfg['hdr_num']     ['value']   = str(max_hv_bit_idx - self.icrc_l4_hv)
            icrc_hdr_cfg['crc_vld']     ['value']   = str(0)
            icrc_hdr_cfg['crc_unit']    ['value']   = str(self.unit)\
                                                  if self.unit != -1 else str(0)
            icrc_hdr_cfg['hdrfld_start']['value']   = str(self.hdrfld_start)
            icrc_hdr_cfg['hdrfld_end']  ['value']   = str(self.hdrfld_end)
            icrc_hdr_cfg['crc_profile'] ['value']   = str(self.IcrcDeParserProfileObjGet().\
                                                                IcrcProfileNumGet())
            icrc_hdr_cfg['mask_profile']['value']   = str(self.IcrcDeParserProfileObjGet().\
                                                                IcrcL4MaskProfileNumGet())
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
        self.verify_cal_fieldlist   = [] #List of CalField Objects
        self.update_cal_fieldlist   = [] #List of CalField Objects
        self.icrc_profiles_allocated        = 0
        self.icrc_mask_profiles_allocated   = 0
        self.icrc_dp_profiles_allocated     = 0
        self.icrc_dp_mask_profiles_allocated= 0
        self.l3hdr_to_profile_map           = {}
        self.l3hdr_to_profile_map_dp        = {}
        self.l4_hdr_to_profile_map_dp      = {}
        self.roce_hdr                       = None
        self.dpr_hw_icrc_obj                = [] #Sorted list of calfldobj; sorted by fldstart

    def initialize(self):
        '''
        '''
        for cal_fld in self.be.h.calculated_fields:
            field_dst, fld_ops, _, _  = cal_fld
            for ops in fld_ops:
                if self.be.h.p4_field_list_calculations[ops[1]].algorithm != 'icrc':
                    #calculated field objects are not icrc(skip checksum related obj)
                    continue
                if ops[2] and ops[2].op == 'valid':
                    if not self.roce_hdr:
                        self.roce_hdr = ops[2].right
                    else:
                        #Make sure there is no mistake in P4 expressing icrc with
                        #different header instances in icrc calculation.
                        assert self.roce_hdr == ops[2].right, pdb.set_trace()
                if ops[0] == 'verify':
                    self.verify_cal_fieldlist.append(IcrcParserCalField(\
                                                     self.be, \
                                                     field_dst, ops[1]))
                else:
                    self.update_cal_fieldlist.append(IcrcDeParserCalField(\
                                                     self.be, \
                                                     field_dst, ops[1]))

    def IsHdrRoceV2(self, hdr_name):
        return True if self.roce_hdr == hdr_name else False


    #   --------  iCRC verification related Code --------


    def VerifyIcrcCalFieldObjGet(self, hdr):
        # Given l3hdr or l4 hdr return Calculated Field Obj that is verified
        for calflistobj in self.verify_cal_fieldlist:
            if hdr != '' and (calflistobj.IcrcL3HdrNameGet() == hdr or \
                                calflistobj.IcrcL4HdrNameGet() == hdr):
                return calflistobj
        return None


    def IsHdrInIcrcVerify(self, hdrname):
        return True if VerifyIcrcCalFieldObjGet(hdrname) else False

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
        prof_obj.IcrcProfileEndEopSet(1)

        #Build mask profile for invariant fields.

        #Fill 64bit 1's before L3 hdr.
        fld_inst                = 0
        mask_field              = {}
        mask_field['mask_en']   = 1
        mask_field['use_ohi']   = 0
        mask_field['start_adj'] = 0
        mask_field['end_adj']   = 8
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
            mask_field['end_adj']   = (l3hdr_ifld.offset + l3hdr_ifld.width) / 8
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
            mask_field['end_adj']   = (hdr_ifld.offset + hdr_ifld.width) / 8
            mask_field['fill']      = 1
            mask_field['skip_first_nibble']  = 0
            if hdr_ifld.offset % 8 == 4:
                #field starts on nibble
                mask_field['skip_first_nibble']  = 1
                if not hdr_ifld.width % 8:
                    #start in middle of byte and ends in middle of byte
                    #hence move end_adj by one more byte
                    mask_field['end_adj']   += 1
            prof_obj.IcrcMaskProfileMaskFieldAdd(fld_inst, mask_field)

    def ProcessIcrcVerificationCalFldList(self, parser):
        '''
        This function will process all verifiable calculated fields
        and creates icrc objects
        '''
        icrc_l3_hdrs = []
        for calfldobj in self.verify_cal_fieldlist:
            assert calfldobj != None, pdb.set_trace()
            calfldhdr = calfldobj.CalculatedFieldHdrGet()
            assert calfldhdr != None, pdb.set_trace()
            l3_name = calfldobj.IcrcL3HdrNameGet()
            assert(l3_name != ''), pdb.set_trace()
            assert(l3_name in self.be.h.p4_header_instances),\
                                                    pdb.set_trace()
            #Also allocate l3 profile obj
            calfldobj.IcrcParserProfileObjSet(IcrcParserProfile())
            icrc_l3_hdrs.append(l3_name)

        self.icrc_verify_logger.debug('Icrc L3 Hdrs =  %s' % (str(icrc_l3_hdrs)))

        # Ensure/CrossCheck that every calculated field that needs to
        # be verified has been created and also build profile.
        for calfldobj in self.verify_cal_fieldlist:
            if calfldobj.IcrcParserProfileObjGet() == None:
                assert(0), pdb.set_trace()
            #L4HdrIFldProfileBuild should be invoked after calling L3hdrProfileBuild
            self.IcrcParserL3HdrIFldProfileBuild(calfldobj)
            self.IcrcParserL4HdrIFldProfileBuild(calfldobj)

    def InsertIcrcObjReferenceInParseState(self, parser):
        '''
            In parse states where icrc L3 hdrs are extracted, and where
            roce header is extracted, insert reference to p4 cal fld objects
            so that Parser block can be programmed to trigger icrc verification.
        '''
        icrc_l3hdrs = set()
        icrc_l4_hdrs = set()
        icrc_calfldobjs = set(calfldobj for calfldobj in self.verify_cal_fieldlist)
        for calfldobj in self.verify_cal_fieldlist:
            icrc_l3hdr = calfldobj.IcrcL3HdrNameGet()
            icrc_l4_hdr = calfldobj.IcrcL4HdrNameGet()
            if icrc_l3hdr != '' and icrc_l3hdr not in icrc_l3hdrs:
                icrc_l3hdrs.add(icrc_l3hdr)
            if icrc_l4_hdr != '' and icrc_l4_hdr not in icrc_l4_hdrs:
                icrc_l4_hdrs.add(icrc_l4_hdr)
        #Find parse states where reference to icrc calfldobj should be added.
        l3_calfldlist = []
        for parsepath in parser.paths:
            if not len(icrc_calfldobjs):
                break
            for hdr in parsepath:
                if hdr.name in icrc_l3hdrs:
                    calfldobj = self.VerifyIcrcCalFieldObjGet(hdr.name)
                    for parsestate in parser.get_ext_cstates(hdr):
                        if calfldobj not in parsestate.icrc_verify_cal_field_objs:
                            parsestate.icrc_verify_cal_field_objs.\
                                            append(("L3_IFLD", calfldobj))
                            l3_calfldlist.append(calfldobj)
                    icrc_l3hdrs.remove(hdr.name)
                    icrc_calfldobjs.remove(calfldobj)
                if hdr.name in icrc_l4_hdrs:
                    calfldobj = self.VerifyIcrcCalFieldObjGet(hdr.name)
                    for parsestate in parser.get_ext_cstates(hdr):
                        if calfldobj not in parsestate.icrc_verify_cal_field_objs:
                            parsestate.icrc_verify_cal_field_objs.\
                                            append(("L4_IFLD", calfldobj))
                    icrc_l4_hdrs.remove(hdr.name)
        assert(len(icrc_l4_hdrs) == 0 and len(icrc_l3hdrs) == 0), pdb.set_trace()

        #Insert all calfldobj in roce_bth parse state. Depending on parse path
        #one of the calfldobj will be used to program parser when extracting
        #roce_bth header that corresponds to L3hdr in the parser path.
        all_calflds = [("L3_IFLD", calfldobj) for calfldobj in self.verify_cal_fieldlist]
        for parsestate in parser.get_ext_cstates(self.roce_hdr):
            parsestate.icrc_verify_cal_field_objs =  all_calflds


    def AllocateParserIcrcResources(self, parser):
        icrc_l3hdrs = set()
        for calfldobj in self.verify_cal_fieldlist:
            icrc_l3hdr = calfldobj.IcrcL3HdrNameGet()
            if icrc_l3hdr != '' and icrc_l3hdr not in icrc_l3hdrs:
                icrc_l3hdrs.add(icrc_l3hdr)
        if not len(icrc_l3hdrs):
            return

        icrc_objects = set(calfldobj for calfldobj in self.verify_cal_fieldlist)
        all_parse_paths = sorted(parser.paths, key=lambda p: len(p), reverse=True)
        for parse_path in all_parse_paths:
            program_icrc = False
            parse_path_hdrs = set(hdr.name for hdr in parse_path)
            if self.roce_hdr.name in parse_path_hdrs:
                program_icrc = True
            if program_icrc:
                l3_hdrs = None
                _s = parse_path_hdrs.intersection(icrc_l3hdrs)
                if len(_s):
                    assert(len(_s) <= 2), pdb.set_trace()
                    l3_hdrs = _s
                else:
                    #roce hdr in parse path; but no associated L3 hdr is in the path.
                    assert(0), pdb.set_trace()

                for hdr in parse_path:
                    if hdr.name != self.roce_hdr.name:
                        continue
                    #In order to associate right l3hdr, maintain
                    #l3hdrs in the same order as they are in parse path.
                    topo_l3hdrs_in_parse_path = []
                    for _hdr in parse_path:
                        if _hdr.name in l3_hdrs:
                            topo_l3hdrs_in_parse_path.append(_hdr.name)
                        if _hdr.name == hdr.name: break
                    topo_l3hdrs_in_parse_path.reverse()
                    l3hdr = topo_l3hdrs_in_parse_path[0]
                    icrc_calfldobj = \
                       self.VerifyIcrcCalFieldObjGet(l3hdr)

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

                                #Since icrc is from L3 to till end of packet, OhiLen
                                #is not put to use for now. However keeping
                                #a place holder code in the icrc obj incase
                                #in future Len is needed.
                                icrc_calfldobj.IcrcOhiLenSelSet(0)
                            else:
                                #TODO : using allocated OHI, parser instructions
                                # need to be generated in respective parse states.
                                #Until this case is handled, compiler should assert
                                #when compiling P4 that has icrc but no csum constructs.
                                assert(0), pdb.set_trace()
                                '''
                                icrc_calfldobj.IcrcOhiStartSelSet(parser.\
                                                assign_ohi_slots_for_icrc(l3hdr))
                                icrc_calfldobj.IcrcOhiLenSelSet(parser.\
                                                assign_ohi_slots_for_icrc(l3hdr))
                                '''

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
                                ohi_start_id = parsestate.\
                                    verify_cal_field_objs[0].\
                                    ParserCsumObjGet().CsumOhiStartSelGet()

                                #When processing csum constructs, OHI ID with L3
                                #offset is already loaded. Reusing same OHI
                                #For masking out invariant fields in UDP, use OHI
                                #slot# that is used for capturing UDP hdr start.
                                icrc_calfldobj.IcrcOhiMaskSelSet(ohi_start_id)
                            else:
                                #TODO : using allocated OHI, parser instructions
                                # need to be generated in respective parse states.
                                #Until this case is handled, compiler should assert
                                #when compiling P4 that has icrc but no csum constructs.
                                assert(0), pdb.set_trace()
                                '''
                                icrc_calfldobj.IcrcOhiMaskSelSet(parser.\
                                               assign_ohi_slots_for_icrc(l3hdr))
                                '''

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

                        icrc_objects.remove(icrc_calfldobj)
                    break

                if not len(icrc_objects):
                    break

        #Assert if all calfld objects are allocated resources
        assert(len(icrc_objects) == 0), pdb.set_trace()

        #In parse states where L3 hdrs are extracted, and where roce_bth
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

        assert len(icrc_l3hdrs) > 0, pdb.set_trace()

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
        assert matched_calobj and matched_l3hdr, pdb.set_trace()

        return calfldobj

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
            calfldobj = self.IcrcFindCalFldObjMatchingL3hdr(\
                                              parse_state, parse_states_in_path)
        else:
            hdr_type, calfldobj = parse_state.icrc_verify_cal_field_objs[0]

        if calfldobj == None:
            assert(0), pdb.set_trace()

        hdr_ohi_id = calfldobj.IcrcOhiStartSelGet()
        mask_ohi_id = calfldobj.IcrcOhiMaskSelGet()
        len_ohi_id = calfldobj.IcrcOhiLenSelGet()

        assert hdr_ohi_id != -1, pdb.set_trace()
        assert len_ohi_id != -1, pdb.set_trace()
        assert mask_ohi_id != -1, pdb.set_trace()

        extracted_hdrs = [hdr for hdr in parse_state.headers]
        if self.roce_hdr in extracted_hdrs:
            icrc_enable = True
            use_latched_profile_from_l3_state = True
        elif hdr_type == 'L4_IFLD':
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
                assert(0), pdb.set_trace()
            profile_obj.ConfigGenerate(profile)
            calfldobj.IcrcAddLog(profile_obj.LogGenerate())
            #Only one calFld processed in any parse state
            return profile, p
        elif profile_obj == None:
            assert(0), pdb.set_trace()

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
                assert(0), pdb.set_trace()
            profile_obj.MaskProfileConfigGenerate(profile)
            calfldobj.IcrcAddLog(profile_obj.MaskProfileLogGenerate())
            #Only one calFld processed in any parse state
            return profile, p
        elif profile_obj == None:
            assert(0), pdb.set_trace()

        return profile, p



    #   --------  iCRC computation related Code --------

    def UpdateIcrcCalFieldObjGet(self, hdr):
        # Given l3 hdr or l4 header name, return icrc Calculated Field Obj that is updated
        for calflistobj in self.update_cal_fieldlist:
            if hdr != '' and (calflistobj.IcrcL3HdrNameGet() == hdr
                                or calflistobj.IcrcL4HdrNameGet() == hdr):
                return calflistobj
        return None

    def IsHdrInIcrcCompute(self, hdrname):
        return True if self.UpdateIcrcCalFieldObjGet(hdrname) else False

    def DeParserIcrcPayLoadLenSlotGet(self, calfldobj, eg_parser):
        assert calfldobj.icrc_update_len_field != '', pdb.set_trace()
        cf_icrc_update_len = self.be.pa.get_field(calfldobj.icrc_update_len_field, eg_parser.d)
        pl_slot = (cf_icrc_update_len.phv_bit - 
                   self.be.hw_model['deparser']['len_phv_start']) / 16

        return pl_slot

    def IcrcDeParserProfileBuild(self, calfldobj, eg_parser):
        if calfldobj.l3hdr_name not in self.l3hdr_to_profile_map_dp.keys():
            self.l3hdr_to_profile_map_dp[calfldobj.l3hdr_name] = \
                (self.icrc_dp_profiles_allocated, self.icrc_dp_mask_profiles_allocated)
            self.icrc_dp_profiles_allocated += 1
            self.icrc_dp_mask_profiles_allocated += 1

        profile_num, mask_profile_num = self.l3hdr_to_profile_map_dp[calfldobj.l3hdr_name]

        prof_obj = calfldobj.IcrcDeParserProfileObjGet()
        prof_obj.IcrcProfileNumSet(profile_num)
        prof_obj.IcrcMaskProfileNumSet(mask_profile_num)
        phv_len_slot = self.DeParserIcrcPayLoadLenSlotGet(calfldobj, eg_parser)
        prof_obj.IcrcProfilePhvLenSelSet(1, phv_len_slot)
        #Subtract 8 bytes from the start of L3 hdr so that
        #64 1bits are added to icrc computation.
        prof_obj.IcrcProfileStartAdjSet(8, 1)
        prof_obj.IcrcProfileShiftLeftSet(0, 0)
        prof_obj.IcrcProfileEndAdjSet(0, 0)
        prof_obj.IcrcProfileLocAdjSet(4, 1)

        #Build mask profile for invariant fields.

        #Fill 64bit 1's before L3 hdr.
        fld_inst = 0
        mask_field              = {}
        mask_field['en']   = 1
        mask_field['start'] = 0
        mask_field['end']   = 8
        mask_field['fill']  = 1
        mask_field['skip_first_nibble']  = 0
        prof_obj.IcrcMaskProfileMaskFieldAdd(fld_inst, mask_field)

        leading_64b_byte_len = 8

        l3hdr_iflds = calfldobj.L3HdrInvariantFieldsGet()
        fld_inst = 1
        for l3hdr_ifld in l3hdr_iflds:
            mask_field              = {}
            mask_field['en']        = 1
            mask_field['start']     = l3hdr_ifld.offset / 8
            mask_field['end']       = (l3hdr_ifld.offset + l3hdr_ifld.width) / 8
            mask_field['fill']      = 1
            mask_field['skip_first_nibble']  = 0
            if l3hdr_ifld.offset % 8 == 4:
                #field starts on nibble
                mask_field['skip_first_nibble']  = 1
                if not l3hdr_ifld.width % 8:
                    #start in middle of byte and ends in middle of byte
                    #hence move end_adj by one more byte
                    mask_field['end'] += 1
                #NB:
                # TC in ipv6 hdr starts  @bit 4 and ends @bit 11. Since there is no way
                # to skip end nibble, bit12 to bit15 are also marked invariant. This
                # works because FLow-Label starts at bit12 and ends @bit31 and is also
                # invariant field.
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
            mask_field['end']       = (hdr_ifld.offset + hdr_ifld.width) / 8
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


    def ProcessIcrcUpdateCalFldList(self, egress_parser):
        '''
         Process all update calculated fields
         and creates icrc objects
        '''
        icrc_l3_hdrs = []
        for calfldobj in self.update_cal_fieldlist:
            assert calfldobj != None, pdb.set_trace()
            calfldhdr = calfldobj.CalculatedFieldHdrGet()
            assert calfldhdr != None, pdb.set_trace()
            l3_name = calfldobj.IcrcL3HdrNameGet()
            assert(l3_name != ''), pdb.set_trace()
            assert(l3_name in self.be.h.p4_header_instances),\
                                                    pdb.set_trace()
            #Also allocate l3 profile obj
            calfldobj.IcrcDeParserProfileObjSet(IcrcDeParserProfile())
            icrc_l3_hdrs.append(l3_name)

        self.icrc_compute_logger.debug('Icrc L3 Hdrs =  %s' % (str(icrc_l3_hdrs)))

        # Ensure/CrossCheck that every calculated field that needs to
        # be verified has been created and also build profile.
        for calfldobj in self.update_cal_fieldlist:
            if calfldobj.IcrcDeParserProfileObjGet() == None:
                assert(0), pdb.set_trace()
            #L4HdrIFldProfileBuild should be invoked after calling L3hdrProfileBuild
            self.IcrcDeParserProfileBuild(calfldobj, egress_parser)
            self.IcrcDeParserL4HdrIFldProfileBuild(calfldobj)

    def AllocateDeParserIcrcResources(self, eg_parser):
        for calfldobj in self.update_cal_fieldlist:
            icrc_l3hdr = calfldobj.IcrcL3HdrNameGet()
            icrc_hv_and_hf = eg_parser.icrc_hdr_hv_bit[self.be.h.\
                                                       p4_header_instances[icrc_l3hdr]]
            icrc_hv, phv_bit, hfname = icrc_hv_and_hf[0]
            calfldobj.IcrcHvBitNumSet(icrc_hv)
            hdr_valid_phv_bit = eg_parser.hdr_hv_bit[self.be.h.\
                                                 p4_header_instances[icrc_l3hdr]]
            calfldobj.HvBitNumSet(511 - hdr_valid_phv_bit)

            icrc_l4_hdr = calfldobj.IcrcL4HdrNameGet()
            icrc_hv_and_hf = eg_parser.icrc_hdr_hv_bit[self.be.h.\
                                                       p4_header_instances[icrc_l4_hdr]]
            icrc_l4_hv, phv_bit, hfname = icrc_hv_and_hf[0]
            calfldobj.IcrcL4HvBitNumSet(icrc_l4_hv)

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
        for calfldobj in self.update_cal_fieldlist:
            l3hdr = calfldobj.IcrcL3HdrNameGet()
            icrc_profile_obj = calfldobj.IcrcDeParserProfileObjGet()
            assert icrc_profile_obj != None, pdb.set_trace()
            assert calfldobj.hv != -1, pdb.set_trace()
            assert calfldobj.icrc_hv != -1, pdb.set_trace()
            fldstart, fldend, _ = hv_fld_slots[calfldobj.icrc_hv]
            calfldobj.HdrFldStartEndSet(fldstart,fldend)

            #Generate Logical Output
            calfldobj.IcrcAddLog(calfldobj.LogGenerate(l3hdr))
            calfldobj.IcrcAddLog(icrc_profile_obj.LogGenerate())
            calfldobj.IcrcAddLog(icrc_profile_obj.MaskProfileLogGenerate())
            hw_icrcobj.append(calfldobj)

        l4_hdrs = []
        self.l4_calfldobjs = []
        for calfldobj in self.update_cal_fieldlist:
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

        #Before generating HW config, sort based on StartFld Value.
        self.dpr_hw_icrc_obj = sorted(hw_icrcobj, key=lambda obj: obj[0].HdrFldStartGet())
        #Generate ASIC Config
        icrc_hdr_index = 0

        for _calfldobj in self.dpr_hw_icrc_obj:
            _icrc_profile_obj = _calfldobj.IcrcDeParserProfileObjGet()
            if _calfldobj not in self.l4_calfldobjs:
                icrc_hdr_cfg_name = 'cap_dppcsum_csr_cfg_crc_hdrs[%d]' %\
                                    (icrc_hdr_index)
                _calfldobj.ConfigGenerate(dpp_json['cap_dpp']\
                                    ['registers'][icrc_hdr_cfg_name], True)
                icrc_profile_cfg_name = 'cap_dppcsum_csr_cfg_crc_profile[%d]' %\
                                       _icrc_profile_obj.IcrcProfileNumGet()
                _icrc_profile_obj.ConfigGenerate(dpp_json['cap_dpp']\
                                       ['registers'][icrc_profile_cfg_name])
                icrc_profile_cfg_name = 'cap_dppcsum_csr_cfg_crc_mask_profile[%d]' %\
                                       _icrc_profile_obj.IcrcMaskProfileNumGet()
                _icrc_profile_obj.MaskProfileConfigGenerate(dpp_json['cap_dpp']\
                                       ['registers'][icrc_profile_cfg_name])
            else:
                icrc_hdr_cfg_name = 'cap_dppcsum_csr_cfg_crc_hdrs[%d]' %\
                                    (icrc_hdr_index)
                _calfldobj.ConfigGenerate(dpp_json['cap_dpp']\
                                    ['registers'][icrc_hdr_cfg_name], False)
                icrc_profile_cfg_name = 'cap_dppcsum_csr_cfg_crc_mask_profile[%d]' %\
                                       _icrc_profile_obj.IcrcL4MaskProfileNumGet()
                _icrc_profile_obj.L4MaskProfileConfigGenerate(dpp_json['cap_dpp']\
                                       ['registers'][icrc_profile_cfg_name])

            icrc_hdr_index += 1

        #Json is dumped in the caller to cfg-file.


    def IcrcLogicalOutputCreate(self):
        out_dir = self.be.args.gen_dir + '/%s/logs' % (self.be.prog_name)
        if not os.path.exists(out_dir):
            os.makedirs(out_dir)
        ofile = open('%s/icrc.out' % (out_dir), "w")
        ofile.write("Icrc Verification Config in Ingress parser\n")
        ofile.write("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n\n")
        for calfldobj in self.verify_cal_fieldlist:
            for log_str in calfldobj.IcrcLogStrTableGet():
                ofile.write(log_str)

        ofile.write("Icrc Compute Config in Egress Deparser\n")
        ofile.write("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n\n")
        #for calfldobj in self.update_cal_fieldlist:
        for calfldobj in self.dpr_hw_icrc_obj:
            if calfldobj not in self.l4_calfldobjs:
                for log_str in calfldobj.IcrcLogStrTableGet():
                    ofile.write(log_str)
            else:
                ofile.write("L4 Instance Config\n")
                ofile.write("-----------------\n")
                for log_str in calfldobj.IcrcLogStrTableGet():
                    ofile.write(log_str)

        ofile.write("Summary: Icrc Compute Config in Egress Deparser\n")
        ofile.write("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n\n")

        pstr = '{:<12s}{:<5s}{:<7s}{:<7s}{:<8s}{:<8s}{:<8s}{:<5s}{:<8s}{:<7s}{:<7s}'\
               '{:<10s}{:<5s}{:<s}{:<5s}{:<5s}\n'.format("L3 ", "icrc ", "icrc ",
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

        for calfldobj in self.dpr_hw_icrc_obj:
            if calfldobj not in self.l4_calfldobjs:
                ofile.write(calfldobj.IcrcDeParserConfigMatrixRowLog())
            else:
                ofile.write(calfldobj.IcrcDeParserL4ConfigMatrixRowLog())

        ofile.write("\n\n")
        ofile.close()

