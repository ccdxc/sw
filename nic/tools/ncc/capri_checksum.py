#!/usr/bin/python
#
# Header and Payload Checksum p4 object processing.
# Capri-Non-Compiler-Compiler (capri-ncc)
#
# Sept 2017, Mahesh Shirshyad (Pensando Systems)
#

#           Checksum : High Level Infomation
#           ---------------------------------
# Checksums:
#   Header CheckSum:
#       - IPv4 Header Checksum
#   UDP CheckSum:
#       Calculated over IP pseudo header and UDP data.
#           - IP source address: taken from IP header (v4/v6)
#           - IP destination address: taken from IP header (v4/v6)
#           - resevered: 8 bits/1 byte, all zeros
#           - protocol: 8 bits/1 byte, taken from IP header.
#           - UDP Length: The length including UDP header and data.
#   TCP CheckSum:
#       Calculated over IP pseudo header and TCP data.
#           - IP source address: taken from IP header (v4/v6)
#           - IP destination address: taken from IP header (v4/v6)
#           - resevered: 8 bits/1 byte, all zeros
#           - protocol: 8 bits/1 byte, taken from IP header.
#           - TCP Length: The length of the TCP segment, including TCP header
#             and TCP data. This field is not available in TCP header, therefore
#             is computed on the fly.
#
#
# Processing of P4 Calculated Field Objects
# ------------------------------------------
#   - For each object in hlir.field_list_calculation[p4-name]
#       - Assign checksum engine.(Keep a monotonically incrementing counter)
#       - If more than 4 needed, assert
#       - For each input field in hlir.field_list_calculation[p4-name]
#           - In the parser state where the input field is extracted,
#             use one of the two csum_inst. Also increment in_use_csum_inst
#             count for that state. If more than 2 csum_inst needed in a
#             parser state, then that is a voilation that can handled by
#             splitting header extraction into 2 states.
#
# - Checksum verification done in parser
# - Checksum computation done in deparser

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
from capri_parser_checksum import *
from capri_deparser_checksum import *


# ----   P4 Calculated Field Verification using Checksum engines in Parser --- #

class ParserCsumEngine:
    '''
    Resource allocation done here. Allocating CsumEngine for header/payload
    checksum verification, allocation of csum profile and csum pseudo header
    profile is also done here.
    '''
    def __init__(self, direction, capri_be):
        self.be                     = capri_be
        self.direction              = direction
        self.max_csum_units         = self.be.hw_model['parser']['max_csum_engines']
        self.max_cprofiles          = 8
        self.max_phdr_profiles      = 8
        self.allocated_csum_units   = [False, False, False, False, False]
        self.allocated_csum_is_l3   = [False, False, False, False, False]
        self.allocated_cprofile     = 0
        self.allocated_phdr_profile = 0
        self.csum_units             = {} # k = checksumfield, v = csum-unit#
        self.csum_profile           = {} # k = checksumfield, v = csum-profile#
        self.csum_phdr_profile      = {} # k = checksumfield, v = csum-phdr-profile#
        self.csum_units_in_use      = 0

    def ResetCsumResources(self):
        #self.allocated_cprofile     = 0
        #self.allocated_phdr_cprofile= 0
        self.csum_units_in_use   = 0
        for i in range(self.max_csum_units):
            self.allocated_csum_units[i] = False
            #self.allocated_csum_is_l3[i] = False

    def AllocateCsumUnit(self, csumField, is_l3=False):
        if csumField in self.csum_units.keys():
            if self.allocated_csum_units[self.csum_units[csumField]] == False:
                self.allocated_csum_units[self.csum_units[csumField]] = True
                self.csum_units_in_use += 1
            return self.csum_units[csumField]
        ncc_assert(self.csum_units_in_use < self.max_csum_units)
        if not is_l3:
            self.csum_units[csumField] = self.allocated_csum_units.index(False)
            self.allocated_csum_units[self.csum_units[csumField]] = True
            self.csum_units_in_use += 1
        else:
            for i, used in enumerate(self.allocated_csum_units):
                if used == False and not self.allocated_csum_is_l3[i]:
                    self.csum_units[csumField] = i
                    self.allocated_csum_units[self.csum_units[csumField]] = True
                    self.csum_units_in_use += 1
                    break
            if i >= len(self.allocated_csum_units):
                ncc_assert(0)
        self.allocated_csum_is_l3[self.csum_units[csumField]] = is_l3
        return (self.csum_units[csumField])

    def DeAllocateCsumUnit(self, csumField):
        if csumField in self.csum_units.keys():
            self.allocated_csum_units[self.csum_units[csumField]] = False
            self.allocated_csum_is_l3[self.csum_units[csumField]] = False
            self.csum_units_in_use -= 1
            del self.csum_units[csumField]

    def AlreadyAllocatedCsumUnitSet(self, csumField, csumUnit, is_l3):
        ncc_assert(csumUnit != -1)
        if not is_l3:
            if self.allocated_csum_units[csumUnit] == False:
                self.csum_units_in_use += 1
        else:
            self.allocated_csum_is_l3[csumUnit] = is_l3
            if self.allocated_csum_units[csumUnit] == False and \
            self.allocated_csum_is_l3[csumUnit] == True:
                self.csum_units_in_use += 1
        self.allocated_csum_units[csumUnit] = True

    def CsumUnitAllocationFinalize(self):
        self.csum_units_in_use = 0
        for i in range(self.max_csum_units):
            self.allocated_csum_units[i] = False
        for csumField, csum_unit in self.csum_units.items():
            if self.allocated_csum_units[csum_unit] == False:
                self.allocated_csum_units[csum_unit] = True
                self.csum_units_in_use += 1

    def AllocateCsumProfile(self, csumField):
        if csumField in self.csum_profile.keys():
            return self.csum_profile[csumField]
        ncc_assert(self.allocated_cprofile < self.max_cprofiles)
        self.csum_profile[csumField] = self.allocated_cprofile
        self.allocated_cprofile += 1
        return (self.allocated_cprofile - 1)

    def AllocatePhdrProfile(self, csumField, phdr):
        if (csumField,phdr) in self.csum_phdr_profile.keys():
            return self.csum_phdr_profile[(csumField, phdr)]
        ncc_assert(self.allocated_phdr_profile < self.max_phdr_profiles)
        self.csum_phdr_profile[(csumField, phdr)] = self.allocated_phdr_profile
        self.allocated_phdr_profile += 1
        return (self.allocated_phdr_profile - 1)

class DeParserCsumEngine:
    '''
    Resource allocation done here. Allocating CsumEngine for header/payload
    checksum verification, allocation of csum profile and csum pseudo header
    profile is also done here.
    '''
    def __init__(self, direction, capri_be):
        self.be                     = capri_be
        self.direction              = direction
        self.max_csum_units         = self.be.hw_model['deparser']['max_csum_engines']
        self.max_cprofiles          = 16
        self.max_phdr_profiles      = 16
        self.allocated_csum_units   = [False, False, False, False, False]
        self.allocated_csum_is_l3   = [False, False, False, False, False]
        self.allocated_cprofile     = 0
        self.allocated_phdr_profile = 0
        self.csum_units             = {} # k = checksumfield, v = csum-unit#
        self.csum_profile           = {} # k = checksumfield, v = csum-profile#
        self.csum_phdr_profile      = {} # k = checksumfield, v = csum-phdr-profile#
        self.csum_units_in_use      = 0

    def ResetCsumResources(self):
        self.csum_units_in_use   = 0
        for i in range(self.max_csum_units):
            self.allocated_csum_units[i] = False
            #self.allocated_csum_is_l3[i] = False

    def AllocateCsumUnit(self, csumField, calfldobj, is_l3=False):
        allocate_csum_unit = True
        if len(calfldobj.share_csum_engine_with):
            #This csum object intends to share checksum engine with
            #another csum object. P4 programmer decided to share
            #based on exclusivity of l4 csum in rx path. Similar
            #sharing can also be applied in Tx path.
            for csum_dstfield in calfldobj.share_csum_engine_with:
                field = csum_dstfield.split('.')[0]
                if field in self.csum_units.keys():
                    return (self.csum_units[field])

        if csumField in self.csum_units.keys():
            if self.allocated_csum_units[self.csum_units[csumField]] == False:
                self.allocated_csum_units[self.csum_units[csumField]] = True
                self.csum_units_in_use += 1
            return self.csum_units[csumField]
        ncc_assert(self.csum_units_in_use < self.max_csum_units)
        if not is_l3:
            self.csum_units[csumField] = self.allocated_csum_units.index(False)
            self.allocated_csum_units[self.csum_units[csumField]] = True
            self.csum_units_in_use += 1
        else:
            for i, used in enumerate(self.allocated_csum_units):
                if used == False and not self.allocated_csum_is_l3[i]:
                    self.csum_units[csumField] = i
                    self.allocated_csum_units[self.csum_units[csumField]] = True
                    self.csum_units_in_use += 1
                    break
            ncc_assert(i < len(self.allocated_csum_units))
        self.allocated_csum_is_l3[self.csum_units[csumField]] = is_l3

        return (self.csum_units[csumField])

    def DeAllocateCsumUnit(self, csumField):
        if csumField in self.csum_units.keys():
            self.allocated_csum_units[self.csum_units[csumField]] = False
            self.allocated_csum_is_l3[self.csum_units[csumField]] = False
            self.csum_units_in_use -= 1
            del self.csum_units[csumField]

    def AlreadyAllocatedCsumUnitSet(self, csumField, csumUnit, is_l3):
        ncc_assert(csumUnit != -1)
        if not is_l3:
            if self.allocated_csum_units[csumUnit] == False:
                self.csum_units_in_use += 1
        else:
            self.allocated_csum_is_l3[csumUnit] = is_l3
            if self.allocated_csum_units[csumUnit] == False and \
            self.allocated_csum_is_l3[csumUnit] == True:
                self.csum_units_in_use += 1
        self.allocated_csum_units[csumUnit] = True

    def CsumUnitAllocationFinalize(self):
        self.csum_units_in_use = 0
        for i in range(self.max_csum_units):
            self.allocated_csum_units[i] = False
        for csumField, csum_unit in self.csum_units.items():
            if self.allocated_csum_units[csum_unit] == False:
                self.allocated_csum_units[csum_unit] = True
                self.csum_units_in_use += 1

    def AllocateCsumProfile(self, csumField):
        if csumField in self.csum_profile.keys():
            return self.csum_profile[csumField]
        ncc_assert(self.allocated_cprofile < self.max_cprofiles)
        self.csum_profile[csumField] = self.allocated_cprofile
        self.allocated_cprofile += 1
        return (self.allocated_cprofile - 1)

    def AllocatePhdrProfile(self, csumField, phdr):
        if (csumField, phdr) in self.csum_phdr_profile.keys():
            return self.csum_phdr_profile[(csumField, phdr)]
        ncc_assert(self.allocated_phdr_profile < self.max_phdr_profiles)
        self.csum_phdr_profile[(csumField, phdr)] = self.allocated_phdr_profile
        self.allocated_phdr_profile += 1
        return (self.allocated_phdr_profile - 1)


# If P4 has calculated_fields, process and build objects
# There can be 2 types of objects. One for verfication
# and one for update.
class Checksum:
    def __init__(self, capri_be):
        self.be                     = capri_be
        self.logstr_tbl             = []
        self.csum_verify_logger     = logging.getLogger('CSUM_V')
        self.csum_compute_logger    = logging.getLogger('CSUM_C')
        self.IngParserCsumEngineObj = ParserCsumEngine(xgress.INGRESS, capri_be)
        self.EgParserCsumEngineObj  = ParserCsumEngine(xgress.EGRESS, capri_be)
        self.IngDeParserCsumEngineObj = DeParserCsumEngine(xgress.INGRESS, capri_be)
        self.EgDeParserCsumEngineObj = DeParserCsumEngine(xgress.EGRESS, capri_be)

        self.verify_cal_fieldlist   = [] #List of CalField Objects verfified on ingress pipeline parser
        self.eg_verify_cal_fieldlist= [] #List of CalField Objects verified on egress pipeline parser
        self.update_cal_fieldlist   = [] #List of CalField Objects computed on ingress pipeline deparser
        self.eg_update_cal_fieldlist= [] #List of CalField Objects computed on egress pipeline deparser

        self.gso_cal_fieldlist_compute    = [] #GSO csum computation done in ingress parser
        self.eg_gso_cal_fieldlist_compute = [] #GSO csum computation done in egress parser
        self.gso_cal_fieldlist_update     = [] #GSO csum rewrite into pkt done in ingress deparser
        self.eg_gso_cal_fieldlist_update  = [] #GSO csum rewrite into pkt done in egress deparser

        self.l2_csum_cal_fieldlist_update   = [] #L2 csum computed in ingress pipeline deparser
        self.eg_l2_csum_cal_fieldlist_update= [] #L2 csum computed in egress pipeline deparser

        self.dpr_phdr_csum_obj      = {} #List of phdr objects in ingress pipe; k = hdr-name, v = obj
        self.eg_dpr_phdr_csum_obj   = {} #List of phdr objects in egress pipe; k = hdr-name, v = obj
        self.dpr_hw_csum_obj        = [] #Sorted list of (csum_obj, is_phdr, calfldobj)
                                         #sorted based on fldstart value. (Ingress pipe)
        self.eg_dpr_hw_csum_obj     = [] #Sorted list of (csum_obj, is_phdr, calfldobj)
                                         #sorted based on fldstart value. (Egress pipe)

    def csum_direction_get(self, op, field_list_cal):
        p4_calfld_obj = self.be.h.p4_field_list_calculations[field_list_cal]
        if 'gress' in p4_calfld_obj._parsed_pragmas['checksum']:
            gress = p4_calfld_obj._parsed_pragmas['checksum']['gress'].keys()[0]
            if gress == 'xgress':
                return 'XGRESS'
            return xgress.EGRESS if gress == 'egress' else xgress.INGRESS
        if op in p4_calfld_obj._parsed_pragmas['checksum']:
            gress = p4_calfld_obj._parsed_pragmas['checksum'][op].keys()[0]
            if gress == 'xgress':
                return 'XGRESS'
            return xgress.EGRESS if gress == 'egress' else xgress.INGRESS
        return None

    def initialize(self):
        '''
         - Use verify_cal_fieldlist and program parser blocks.
         - Use update_cal_fieldlist and program deparser blocks.
        '''
        for cal_fld in self.be.h.calculated_fields:
            field_dst, fld_ops, _, _  = cal_fld
            for ops in fld_ops:
                if self.be.h.p4_field_list_calculations[ops[1]].algorithm == 'csum16' \
                  or self.be.h.p4_field_list_calculations[ops[1]].algorithm == 'csum8':
                    d = self.csum_direction_get(ops[0], ops[1])
                    if ops[0] == 'verify':
                        if d == xgress.INGRESS or d == None or d == 'XGRESS':
                            self.verify_cal_fieldlist.append(ParserCalField(\
                                                         self.be, \
                                                         field_dst, ops[1]))
                        if d == xgress.EGRESS or d == 'XGRESS':
                            self.eg_verify_cal_fieldlist.append(ParserCalField(\
                                                         self.be, \
                                                         field_dst, ops[1]))
                    else:
                        if d == xgress.INGRESS or d == 'XGRESS':
                            self.update_cal_fieldlist.append(DeParserCalField(\
                                                         self.be, \
                                                         field_dst, ops[1]))
                        if d == xgress.EGRESS or d == 'XGRESS' or d == None:
                            self.eg_update_cal_fieldlist.append(DeParserCalField(\
                                                         self.be, \
                                                         field_dst, ops[1]))
                elif self.be.h.p4_field_list_calculations[ops[1]].algorithm == 'gso':
                    d = self.csum_direction_get(ops[0], ops[1])
                    if d == xgress.INGRESS or d == 'XGRESS':
                        gso_cal_fieldlist_compute = self.gso_cal_fieldlist_compute
                        gso_cal_fieldlist_update = self.gso_cal_fieldlist_update
                    if d == xgress.EGRESS or d == 'XGRESS' or d == None:
                        gso_cal_fieldlist_compute = self.eg_gso_cal_fieldlist_compute
                        gso_cal_fieldlist_update = self.eg_gso_cal_fieldlist_update

                    if ops[0] == 'update':
                        #Since GSO csum is computed in parser
                        #create parser csum obj for csum update
                        gso_cal_fieldlist_compute.append(ParserGsoCalField(\
                                                         self.be, \
                                                         field_dst, ops[1]))
                        gso_cal_fieldlist_update.append(DeParserGsoCalField(\
                                                        self.be, \
                                                        field_dst, ops[1]))
                elif self.be.h.p4_field_list_calculations[ops[1]].algorithm == 'l2_complete_csum':
                    d = self.csum_direction_get(ops[0], ops[1])
                    if ops[0] == 'update':
                        if d == xgress.INGRESS or d == 'XGRESS':
                            self.l2_csum_cal_fieldlist_update.append(DeParserCalField(\
                                                                     self.be, \
                                                                     field_dst, ops[1]))
                        if d == xgress.EGRESS or d == 'XGRESS' or d == None:
                            self.eg_l2_csum_cal_fieldlist_update.append(DeParserCalField(\
                                                                     self.be, \
                                                                     field_dst, ops[1]))
                else:
                    #calculated field objects are not csum constructs.
                    continue

    def ProcessAllCsumObjects(self, d):
        self.ProcessVerificationCalFldList(self.be.parsers[d])
        self.ProcessGsoCsumComputeObj(d)
        self.ProcessUpdateCalFldList(d)

    def AllocateAllCsumResources(self, d):
        self.AllocateParserCsumResources(self.be.parsers[d])
        self.AllocateParserGsoCsumResources(self.be.parsers[d])
        self.AllocateDeParserGsoCsumResources(self.be.parsers[d])
        self.AllocateDeParserCsumResources(self.be.parsers[d])
        self.AllocateL2CompleteCsumDeParserResources(self.be.parsers[d])

    def ProcessCalFields(self, field_list_calculation, csum_hfield):
        '''
            Process Single input list.
        '''
        phdr_fields = []
        csum_p4_field = None
        csum_hdr = csum_hfield.split(".")[0]
        csum_field = csum_hfield.split(".")[1]
        for idx, field in enumerate(field_list_calculation.input[0].fields):
            if idx == 0:
                phdr_name = field.instance.name
                phdr_type = 'v4' if field.offset == 96 else 'v6'
            if idx >= 4:
                # 4 fields from pseudo hdr are used.
                break
            phdr_fields.append(field)

        for field in self.be.h.p4_header_instances[csum_hdr].fields:
            if field.name == csum_field:
                csum_p4_field = field
                break
        ncc_assert(csum_p4_field != None )
        if csum_p4_field.offset == 128:
            payload_hdr_type = 'tcp'
        elif csum_p4_field.offset == 48:
            payload_hdr_type = 'udp'
        elif csum_p4_field.offset == 16:
            payload_hdr_type = 'icmp'
        else:
            payload_hdr_type = ''

        return phdr_name, phdr_type, payload_hdr_type, phdr_fields 

    def ProcessVerifyLenFieldComputedParseStates(self, parser, dest_parse_states,\
                                                 verify_len_field, ohi_id, \
                                                 calfldobj):
        '''
            Get all parse states that appear before dest_parse_state in topo
            sorted parse states and where verify_len_field is adjusted. In all
            such states, add csum object to so that the verify_len_field value
            is stored in OHI.
            One usage of this function is to collect all parse states where
            v6 option length is decremented using a parse local field. Those
            parse states that branch to dest_parse_state is where final V6 option
            is processed. In such states, load final option-len decremented
            from payload-len into OHI.
        '''
        for dest_parse_state in dest_parse_states:
            for parse_state in parser.states:
                if parse_state == dest_parse_state:
                    break
                for setop in parse_state.set_ops:
                    if setop.dst.hfname == verify_len_field:
                        #Indicate that the parse state should generate OHI instr
                        #by setting True and OHI# to use is ohi_id
                        parse_state.csum_payloadlen_ohi_instr_gen = (True, ohi_id, calfldobj)
                for setop in parse_state.no_reg_set_ops:
                    if setop.dst.hfname == verify_len_field:
                        #Indicate that the parse state should generate OHI instr
                        #by setting True and OHI# to use is ohi_id
                        parse_state.csum_payloadlen_ohi_instr_gen = (True, ohi_id, calfldobj)

    def CalFldListLogStrTableGet(self):
        return self.logstr_tbl

    def CalFldListAddLog(self, logstr):
        self.logstr_tbl.append(logstr)

    def VerifyCalFieldObjGet(self, header, phdr, d):
        # Given header name and pseudo hdr,
        # return Calculated Field Obj that is verified
        calfldlist = []
        if d == xgress.INGRESS:
            verify_cal_fieldlist = self.verify_cal_fieldlist
        else:
            verify_cal_fieldlist = self.eg_verify_cal_fieldlist
        for calflistobj in verify_cal_fieldlist:
            if header == calflistobj.CalculatedFieldHdrGet():
                if phdr != '' and calflistobj.CsumPhdrNameGet() == phdr:
                    return calflistobj
                elif phdr == '':
                    calfldlist.append(calflistobj)
        if phdr == '':
            if len(calfldlist):
                return calfldlist
            else:
                return None

        return None

    def UpdateCalFieldObjGet(self, d, header, phdr):
        # Given header name and pseudo hdr,
        # return Calculated Field Obj that is updated
        calfldlist = []
        update_cal_fieldlist = self.update_cal_fieldlist if d == xgress.INGRESS \
                                                     else self.eg_update_cal_fieldlist
        for calflistobj in update_cal_fieldlist:
            if header == calflistobj.CalculatedFieldHdrGet():
                if phdr != '' and calflistobj.CsumPhdrNameGet() == phdr:
                    return calflistobj
                elif phdr == '':
                    calfldlist.append(calflistobj)
        if phdr == '':
            if len(calfldlist):
                return calfldlist
            else:
                return None

        return None


    def ProcessVerificationCalFldList(self, parser):
        '''
        This function will process all verifiable calculated fields
        and uses parse state graph to create csum objects
        '''
        if parser.d == xgress.INGRESS:
            verify_cal_fieldlist = self.verify_cal_fieldlist
        else:
            verify_cal_fieldlist = self.eg_verify_cal_fieldlist

        # For the extracted header, check if the header
        # has verify calculated field / checksum.
        # If so, get obj of calculated field.
        for calfldobj in verify_cal_fieldlist:
            calfldhdr = calfldobj.CalculatedFieldHdrGet()
            for parsepath in parser.paths:
                for hdr in parsepath:
                    if hdr.name == calfldhdr:
                        #Allocate checksum and checksum profile to
                        #verify calcu field the first time header
                        #is encountered in parse path.
                        if calfldobj.ParserCsumObjGet() == None:
                            calfldobj.ParserCsumObjSet(ParserCsumObj())
                            calfldobj.ParserCsumProfileSet(ParserCsumProfile())
                            if calfldobj.payload_checksum:
                                phdr_name = calfldobj.CsumPhdrNameGet()
                                ncc_assert(phdr_name != '')
                                ncc_assert(phdr_name in self.be.h.p4_header_instances)
                                #Also allocate phdr profile obj
                                calfldobj.ParserPhdrProfileSet(ParserPhdrProfile())

        # Ensure/CrossCheck that every calculated field that needs to
        # be verified has been allocated checksum unit and checksumprofile.
        for calfldobj in verify_cal_fieldlist:
            if calfldobj.ParserCsumObjGet() == None:
                ncc_assert(0)


    def InsertCsumObjReferenceInParseState(self, parser):
        '''
            In parse states where csum related hdrs are extracted,
            insert reference to calculated fld objects so that
            Parser block can be programmed to trigger csum verification.
        '''
        if parser.d == xgress.INGRESS:
            verify_cal_fieldlist = self.verify_cal_fieldlist
        else:
            verify_cal_fieldlist = self.eg_verify_cal_fieldlist

        for calfldobj in verify_cal_fieldlist:
            calfldhdr = calfldobj.CalculatedFieldHdrGet()
            for parsepath in parser.paths:
                for hdr in parsepath:
                    if hdr.name == calfldhdr:

                        if calfldobj.payload_checksum:
                            phdr_name = calfldobj.CsumPhdrNameGet()
                            ncc_assert(phdr_name != '')
                            ncc_assert(phdr_name in self.be.h.p4_header_instances)
                            #Using phdr_name, ncc_assert(there is parser)
                            #state where the phdr hdr is extracted. In
                            #working case it will be either ipv4 or
                            #ipv6 parser state.
                            phdr_parse_states = parser.get_ext_cstates(\
                               self.be.h.p4_header_instances[phdr_name])
                            calfldobj.CsumPhdrParseStateSet(phdr_parse_states)
                            #Check if phdr is also a calculated fld obj.
                            #ipv4 hdr checksum will create ipv4 hdr calfld
                            #object that is also used for building phdr.
                            #However in ipv6 case there will be no calfldobj
                            #for ipv6.
                            phdr_offset_ohi_id = -1
                            total_len_ohi_id = -1

                            csum_unit = calfldobj.ParserCsumObjGet().\
                                                        CsumUnitNumGet()
                            for phdr_parse_state in phdr_parse_states:
                                #if not len(phdr_parse_state.\
                                #            verify_cal_field_objs):
                                if phdr_parse_state.phdr_type == 'v6':
                                    phdr_parse_state.phdr_type = \
                                        calfldobj.CsumPhdrTypeGet()
                                    if phdr_offset_ohi_id == -1 and \
                                       phdr_parse_state.phdr_offset_ohi_id == -1:
                                        phdr_offset_ohi_id = \
                                            parser.assign_ohi_slots_for_checksum(   \
                                                       csum_unit, -1, phdr_name,    \
                                                       ohi_write_only_type.         \
                                                        OHI_WR_ONLY_TYPE_HDR_OFFSET)
                                    if total_len_ohi_id == -1 and \
                                       phdr_parse_state.totalLen_ohi_id == -1:
                                        total_len_ohi_id = \
                                            parser.assign_ohi_slots_for_checksum(   \
                                                      csum_unit, 1,                 \
                                                      calfldobj.l4_verify_len_field,\
                                                        ohi_write_only_type.        \
                                                         OHI_WR_ONLY_TYPE_OTHER)
                                    if phdr_parse_state.phdr_offset_ohi_id == -1:
                                        phdr_parse_state.phdr_offset_ohi_id = \
                                                            phdr_offset_ohi_id
                                    if phdr_parse_state.totalLen_ohi_id == -1:
                                        phdr_parse_state.totalLen_ohi_id = \
                                                                total_len_ohi_id
                                else:
                                    #phdr is also cal fld obj (hdr checksum case).
                                    #Use same ohi_slot that is used for storing
                                    #hdr offset star to calculate hdr checksum.
                                    phdr_parse_state.phdr_type = \
                                        calfldobj.CsumPhdrTypeGet()

                            #Handle v6 option parsing case.
                            if phdr_parse_state.phdr_type == 'v6':
                                # Find all parse states where option len is
                                # decremented from payload Len
                                # In those parse states, since there is need
                                # store the new payload len in OHI, indicate
                                # the need for generating OHI instruction to
                                # store result of l4_verify_len parser local.
                                # variable into OHI
                                #pdb.set_trace()

                                l4hdr_parse_states = parser.get_ext_cstates(\
                                    self.be.h.p4_header_instances[hdr.name])

                                l4_verify_len_adjusted_last_parse_states =  \
                                self.ProcessVerifyLenFieldComputedParseStates(
                                                                   parser, \
                                                        l4hdr_parse_states,\
                                             calfldobj.l4_verify_len_field,\
                                          phdr_parse_state.totalLen_ohi_id,\
                                                                  calfldobj)

                        #In the parse states where the header is extracted,
                        #put reference to verify checksum obj
                        for parsestate in parser.get_ext_cstates(hdr):
                            if calfldobj not in parsestate.verify_cal_field_objs:
                                parsestate.verify_cal_field_objs.append(calfldobj)
                                if calfldobj.CsumPayloadHdrTypeGet() == 'udp':
                                    parsestate.enable_udp_zero_csum_error = 1



    def AllocateParserCsumResources(self, parser):
        '''
            Walk all parse paths and assign checksum resources
            to verify checksum.
            For each calculatedfieldobject assign header start offset OHI slot
            and len ohi slot. Slot for Phdr needed in case of payload
            checksum is retrieved from IP parse state where IP hdr start offset
            is captured and stored in the OHI slot.
        '''

        if parser.d == xgress.INGRESS:
            verify_cal_fieldlist = self.verify_cal_fieldlist
            ParserCsumEngineObj = self.IngParserCsumEngineObj
        else:
            verify_cal_fieldlist = self.eg_verify_cal_fieldlist
            ParserCsumEngineObj = self.EgParserCsumEngineObj

        #Log write only Ohi allocation
        logstr = ''
        for k, v in parser.wr_only_ohi.items():
            logstr += '{:25s}{:35s}{:s}\n'.format('Write Only Ohi Field: ', k, str(v))
        logstr += '\n\n'
        self.CalFldListAddLog(logstr)

        csum_hdrs = set()
        csum_phdrs = set()
        for calfldobj in verify_cal_fieldlist:
            csum_hdr = calfldobj.CalculatedFieldHdrGet()
            if csum_hdr not in csum_hdrs:
                csum_hdrs.add(csum_hdr)
        for calfldobj in verify_cal_fieldlist:
            csum_phdr = calfldobj.CsumPhdrNameGet()
            if csum_phdr != '' and csum_hdr not in csum_phdrs:
                csum_phdrs.add(csum_phdr)
        all_csum_hdrs = csum_hdrs.union(csum_phdrs)
        csum_objects = set(calfldobj for calfldobj in verify_cal_fieldlist)
        all_parse_paths = sorted(parser.paths, key=lambda p: len(p), reverse=True)
        for parse_path in all_parse_paths:
            csum_assign = False
            parse_path_hdrs = set(hdr.name for hdr in parse_path)
            if len(parse_path_hdrs.intersection(all_csum_hdrs)):
                csum_assign = True
            if csum_assign:
                # Before assigning csum units along parse path, start
                # assigning from zeroth csum unit. However assign same
                # csum unit, csum profile if an header was seen
                # along a parse a path that was already processed.
                ParserCsumEngineObj.ResetCsumResources()
                #Find which phdrs are in the path.
                pseudo_hdrs = None
                _s = parse_path_hdrs.intersection(csum_phdrs)
                if len(_s):
                    ncc_assert(len(_s) <= 2)
                    pseudo_hdrs = _s

                #After resetting csum unit allocation to start from start,
                #ensure same csum unit is assigned if the hdr is also
                #in another parse path.
                for hdr in parse_path:
                    if hdr.name not in all_csum_hdrs:
                        continue
                    calfldobjlist = self.VerifyCalFieldObjGet(hdr.name, '', parser.d)
                    if calfldobjlist == None:
                        continue
                    for calfldobj in calfldobjlist:
                        if calfldobj not in csum_objects:
                            #This header was allocated Csum unit
                            #while processing parse path previously.
                            #Provide same csum unit.
                            ParserCsumEngineObj.AlreadyAllocatedCsumUnitSet(\
                                    calfldobj.CalculatedFieldHdrGet(),\
                                    calfldobj.ParserCsumObjGet().CsumUnitNumGet(), calfldobj.payload_checksum)


                for hdr in parse_path:
                    if hdr.name not in all_csum_hdrs:
                        continue
                    calfldobjlist = self.VerifyCalFieldObjGet(hdr.name, '', parser.d)
                    if calfldobjlist == None:
                        # ipv6 doesn't have calculated field object..
                        # continue
                        continue
                    #Pick first calfldobj from the list is fine. In case of hdr
                    #checksum only one elem will be in the list. In case of
                    #payload checksum elem# = #phdrs. For payload checksum
                    #calfldobjlist is not used.
                    calfldobj = calfldobjlist[0]
                    if not calfldobj.payload_checksum and \
                                    calfldobj in csum_objects:
                        calfldhdr = calfldobj.CalculatedFieldHdrGet()
                        calfldobj.ParserCsumObjGet().CsumUnitNumSet(\
                            ParserCsumEngineObj.
                                            AllocateCsumUnit(calfldhdr, True))
                        calfldobj.ParserCsumProfileGet().CsumProfileUnitNumSet(\
                            ParserCsumEngineObj.\
                                            AllocateCsumProfile(calfldhdr))
                        #Allocate OHI slots
                        csum_unit = calfldobj.ParserCsumObjGet().CsumUnitNumGet()
                        calfldobj.ParserCsumObjGet().\
                            CsumOhiStartSelSet(parser.assign_ohi_slots_for_checksum(\
                                                        csum_unit, 0, hdr.name,     \
                                                        ohi_write_only_type.        \
                                                        OHI_WR_ONLY_TYPE_HDR_OFFSET))
                        if calfldobj.hdrlen_verify_field != '':
                            calfldobj.ParserCsumObjGet().\
                                CsumOhiLenSelSet(parser.assign_ohi_slots_for_checksum(  \
                                                        csum_unit, 1,                   \
                                                        calfldobj.hdrlen_verify_field,  \
                                                        ohi_write_only_type.            \
                                                        OHI_WR_ONLY_TYPE_OTHER))
                        #Since pseudo header start and IPv4 header start are the same,
                        #set phdr_ohi_sel ID same as OHI id used for capturing
                        #hdr start to compute header checksum.
                        calfldobj.ParserCsumObjGet().CsumOhiPhdrSelSet(calfldobj.\
                                               ParserCsumObjGet().CsumOhiStartSelGet())
                        if calfldobj.hdrlen_verify_field != '':
                            calfldobj.ParserCsumObjGet().\
                             CsumOhiTotalLenSet(parser.assign_ohi_slots_for_checksum(   \
                                                        csum_unit, -1,                  \
                                                        calfldobj.l4_verify_len_field,  \
                                                        ohi_write_only_type.            \
                                                        OHI_WR_ONLY_TYPE_OTHER))
                        if calfldobj.hdrlen_verify_field == '' and \
                           calfldobj.l4_verify_len_field != '':
                            #Csum calfld obj without payload and not header csum is
                            #treated as option csum over multiple option headers
                            calfldobj.ParserCsumObjGet().\
                                CsumOhiLenSelSet(parser.assign_ohi_slots_for_checksum(  \
                                                        csum_unit, 1,                   \
                                                        calfldobj.l4_verify_len_field,  \
                                                        ohi_write_only_type.            \
                                                        OHI_WR_ONLY_TYPE_OTHER))

                        self.csum_verify_logger.debug(\
                            'Checksum Assignment along path %s' % (str(parse_path)))
                        self.csum_verify_logger.debug(\
                            'Checksum %s Csum Profile# %d, Csum# %d ' \
                              'OhiStart %d OhiLen %d' % \
                            (calfldobj.dstField,
                             calfldobj.ParserCsumProfileGet().CsumProfileUnitNumGet(),
                             calfldobj.ParserCsumObjGet().CsumUnitNumGet(),
                             calfldobj.ParserCsumObjGet().CsumOhiStartSelGet(),
                             calfldobj.ParserCsumObjGet().CsumOhiLenSelGet()))
                        self.csum_verify_logger.debug('')
                        csum_objects.remove(calfldobj)
                    elif calfldobj.payload_checksum:
                        #In order to associate right phdr, maintain
                        #phdrs in the same order as they are in parse path.
                        #This is needed is because in case of v4, v6, tcp pkt
                        #phdr of tcp is v6 not v4.
                        topo_phdrs_in_parse_path = []
                        for _hdr in parse_path:
                            if _hdr.name in pseudo_hdrs:
                                topo_phdrs_in_parse_path.append(_hdr.name)
                            # Collect phdrs until payload hdr only.
                            if _hdr.name == hdr.name: break
                        topo_phdrs_in_parse_path.reverse()
                        phdr = topo_phdrs_in_parse_path[0]
                        pl_calfldobj = self.VerifyCalFieldObjGet(hdr.name,
                                                                 phdr, parser.d)
                        if pl_calfldobj and pl_calfldobj in csum_objects:
                            calfldhdr = pl_calfldobj.CalculatedFieldHdrGet()

                            # Payload checksum node needs 1 csum engine
                            # and 1 csum profile but more csum phdr profiles
                            # depending on which pseudo hdr is in the packet
                            # /parse path.
                            pl_calfldobj.ParserCsumObjGet().CsumUnitNumSet(\
                                ParserCsumEngineObj.
                                               AllocateCsumUnit(calfldhdr))
                            pl_calfldobj.ParserCsumProfileGet().\
                                            CsumProfileUnitNumSet(\
                                ParserCsumEngineObj.\
                                            AllocateCsumProfile(calfldhdr))
                            ncc_assert((pl_calfldobj.ParserPhdrProfileGet().\
                                 CsumPhdrProfileUnitNumGet() == -1), "")
                            pl_calfldobj.ParserPhdrProfileGet().\
                                            CsumPhdrProfileUnitNumSet(\
                                ParserCsumEngineObj.
                                            AllocatePhdrProfile(calfldhdr, phdr))
                            #Allocate OHI slots
                            csum_unit = pl_calfldobj.ParserCsumObjGet().CsumUnitNumGet()
                            pl_calfldobj.ParserCsumObjGet().\
                                CsumOhiStartSelSet(parser.assign_ohi_slots_for_checksum(\
                                                          csum_unit, 0,                 \
                                                          hdr.name,                     \
                                                          ohi_write_only_type.          \
                                                            OHI_WR_ONLY_TYPE_HDR_OFFSET))
                            pl_calfldobj.ParserCsumObjGet().\
                                CsumOhiLenSelSet(parser.assign_ohi_slots_for_checksum(  \
                                                        csum_unit, 1,                   \
                                                        calfldobj.l4_verify_len_field,  \
                                                        ohi_write_only_type.            \
                                                            OHI_WR_ONLY_TYPE_OTHER))
                            self.csum_verify_logger.debug(\
                                'Checksum Assignment along path %s' \
                                % (str(parse_path)))
                            self.csum_verify_logger.debug(\
                                'Checksum %s Phdr %s Phdr Profile#' \
                                ' %d, CsumProfile# %d Csum# %d ' \
                                'OhiStart %d OhiLen %d TotalLen/PayloadLen %d' % \
                                (pl_calfldobj.dstField,
                                 pl_calfldobj.phdr_name,
                                 pl_calfldobj.ParserPhdrProfileGet().CsumPhdrProfileUnitNumGet(),
                                 pl_calfldobj.ParserCsumProfileGet().CsumProfileUnitNumGet(),
                                 pl_calfldobj.ParserCsumObjGet().CsumUnitNumGet(),
                                 pl_calfldobj.ParserCsumObjGet().CsumOhiStartSelGet(),
                                 pl_calfldobj.ParserCsumObjGet().CsumOhiLenSelGet(),
                                 pl_calfldobj.ParserCsumObjGet().CsumOhiTotalLenGet()))
                            self.csum_verify_logger.debug('')
                            csum_objects.remove(pl_calfldobj)
                if not len(csum_objects):
                    break
        #Assert if all calfld objects are allocated resources
        ncc_assert(len(csum_objects) == 0)

        ParserCsumEngineObj.CsumUnitAllocationFinalize()

        #In parse states where csum related hdrs are extracted,
        #insert reference to calculated fld objects so that
        #Parser block can be programmed to trigger csum verification.
        self.InsertCsumObjReferenceInParseState(parser)


    def CsumParserPhdrOffsetInstrGenerate(self, parse_state, sram, 
                                          ohi_instr_inst, ohi_inst_allocator,\
                                          mux_idx_allocator, \
                                          mux_inst_allocator):
        '''
        Use OHI instr to capture start of the pseudo header offset.
        used in case of IP hdr either is v6 or when v4 is not included
        in header checksum verification in p4.
        '''
        instruction_already_generated = False
        for _, v in enumerate(ohi_inst_allocator):
            if  v != None and parse_state.phdr_offset_ohi_id == v[1]:
                instruction_already_generated = True
                break
        if not instruction_already_generated:
            select          = 1 # Load current offset + index
            na              = 0 # NA
            index           = 0
            log_str = ParserCalField._build_ohi_instr(sram, ohi_instr_inst, select, na,\
                              index, parse_state.phdr_offset_ohi_id)
            self.csum_verify_logger.debug("%s" % (log_str))
            ohi_instr_inst += 1
        return ohi_instr_inst


    def _CsumFindCalFldObjMatchingPhdr(self, parse_state, parse_states_in_path):
        '''
         Multiple calculated objs to be processed in a parse state.
         This is only possible in case of payload checksum.
         Transition into TCP/UDP state where payload checksum
         is computed can be from v4/inner_v4/v6/inner_v6
         TCP checksum should only process using one of the
         four possible phdr state depending on what parse
         path contains. If branching happened from parse state
         where ipv4 hdr was extracted then process calfldobj whose
         pseudo header is ipv4; If branching happened from parse state
         where inner_ipv4 hdr was extracted then process calfldobj
         whose pseudo header is inner_ipv4.
        '''
        headers_in_parse_path = []
        for _parse_state in parse_states_in_path:
            headers_in_parse_path += _parse_state.headers
        headers_in_parse_path.reverse()
        csum_phdrs = set()
        for calfldobj in parse_state.verify_cal_field_objs:
            csum_phdr = calfldobj.CsumPhdrNameGet()
            if csum_phdr != '' and csum_phdr not in csum_phdrs:
                csum_phdrs.add(csum_phdr)

        if not len(csum_phdrs):
            return None, None

        matched_calobj = False
        matched_phdr = False
        for phdr in headers_in_parse_path:
            if phdr.name in csum_phdrs:
                matched_phdr = True
                break
        for calfldobj in parse_state.verify_cal_field_objs:
            if calfldobj.CsumPhdrNameGet() == phdr.name:
                matched_calobj = True
                break
        ncc_assert(matched_calobj and matched_phdr)
        #Find the pseudo hdr parse state that is in the parse path.
        phdr_parse_state_found = False
        for i, phdr_parse_state in enumerate(calfldobj.CsumPhdrParseStateGet()):
            if phdr_parse_state in parse_states_in_path:
                phdr_parse_state_found = True
                break
        ncc_assert(phdr_parse_state_found == True)

        return calfldobj, phdr_parse_state


    def CsumParserConfigGenerate(self, parser, parse_states_in_path,    \
                                 parse_state, sram, ohi_instr_inst,     \
                                 ohi_inst_allocator, mux_idx_allocator, \
                                 mux_inst_allocator, l4len_mux_instr_id,\
                                 l4len_mux_idx_id, hdrlen_mux_idx_id,   \
                                 hdrlen_expr, l4len_expr):
        '''
         The parser state (branch to) where header is extracted, fill in
         parser sram with ohi instructions so as to build checksum related
         data, program/Enable Checksum unit needed to verify calculated field
        '''

        # Currently if more than one header is extracted in a parser state,
        # code to capture header offset of the calculated field will be a bit
        # complicated. For now working with an approach that atmost one header
        # is extracted is in parse state.
        #ncc_assert(len(parse_state.verify_cal_field_objs) == 1)

        from_parse_state = parse_states_in_path[-2]
        headers_in_parse_path = []
        for _parse_state in parse_states_in_path:
            headers_in_parse_path += _parse_state.headers

        log_str = ''
        log_str += 'CsumConfig: %s --> %s\n' \
                    % (from_parse_state.name, parse_state.name)
        log_str += '_____________________________________\n\n'

        processed_calfld_objs = []
        phdr_parse_state = None
        l4_calfldobj, phdr_parse_state = self._CsumFindCalFldObjMatchingPhdr(\
                                              parse_state, parse_states_in_path)
        if l4_calfldobj != None:
            ncc_assert(phdr_parse_state != None)
            phdr_calfldobj = None
            for calfldobj in phdr_parse_state.verify_cal_field_objs:
                if l4_calfldobj.phdr_name == calfldobj.CalculatedFieldHdrGet():
                    phdr_calfldobj = calfldobj
                    break
            ohi_instr_inst = self.CalFldCsumParserConfigGenerate(parser,      \
                                       parse_states_in_path,                  \
                                       parse_state, sram, ohi_instr_inst,     \
                                       ohi_inst_allocator, mux_idx_allocator, \
                                       mux_inst_allocator, l4len_mux_instr_id,\
                                       l4len_mux_idx_id, hdrlen_mux_idx_id,   \
                                       log_str, l4_calfldobj, phdr_parse_state,\
                                       phdr_calfldobj, hdrlen_expr, l4len_expr)
            processed_calfld_objs.append(l4_calfldobj)

        #When multiple headers are extracted in a parse state and more than
        #one such extracted headers is part of csum, it is required to loop
        #through all those headers.
        for calfldobj in parse_state.verify_cal_field_objs:
            if  not calfldobj.payload_checksum and calfldobj not in processed_calfld_objs:
                ohi_instr_inst = self.CalFldCsumParserConfigGenerate(parser,      \
                                       parse_states_in_path,                      \
                                       parse_state, sram, ohi_instr_inst,         \
                                       ohi_inst_allocator, mux_idx_allocator,     \
                                       mux_inst_allocator, l4len_mux_instr_id,    \
                                       l4len_mux_idx_id, hdrlen_mux_idx_id,       \
                                       log_str, calfldobj, None, None,            \
                                       hdrlen_expr, l4len_expr)

        return ohi_instr_inst


    def CalFldCsumParserConfigGenerate(self, parser, parse_states_in_path,    \
                                       parse_state, sram, ohi_instr_inst,     \
                                       ohi_inst_allocator, mux_idx_allocator, \
                                       mux_inst_allocator, l4len_mux_instr_id,\
                                       l4len_mux_idx_id, hdrlen_mux_idx_id,   \
                                       log_str, calfldobj, phdr_parse_state,  \
                                       phdr_calfldobj, hdrlen_expr, l4len_expr):

        if calfldobj == None:
            ncc_assert(0)
        phdr_ohi_id = -1
        if calfldobj.payload_checksum and not calfldobj.option_checksum:
            ncc_assert(phdr_parse_state != None)
            if calfldobj.payload_checksum and \
                phdr_parse_state.phdr_offset_ohi_id != -1:
                phdr_ohi_id = phdr_parse_state.phdr_offset_ohi_id
                totalLen_ohi_id = phdr_parse_state.totalLen_ohi_id
            elif calfldobj.payload_checksum and \
                 phdr_parse_state.phdr_offset_ohi_id == -1:
                #Go back to calfld obj where ipv4 hdr is processed to compute
                #header checksum; Use the same ohi slot where start of ipv4 hdr
                #is captured and where TotalLen-ihl*4 is captured as PayloadLen.
                ncc_assert(phdr_calfldobj != None)
                phdr_ohi_id = phdr_calfldobj.ParserCsumObjGet().\
                                CsumOhiPhdrSelGet()
                totalLen_ohi_id = phdr_calfldobj.ParserCsumObjGet().\
                                CsumOhiTotalLenGet()
            elif calfldobj.payload_checksum:
                ncc_assert(0)

        hdr_ohi_id = calfldobj.ParserCsumObjGet().CsumOhiStartSelGet()
        len_ohi_id = calfldobj.ParserCsumObjGet().CsumOhiLenSelGet()
        ncc_assert(hdr_ohi_id != -1)
        ncc_assert(len_ohi_id != -1)

        if calfldobj.payload_checksum and not calfldobj.option_checksum:
            ncc_assert(phdr_ohi_id != -1)
            ncc_assert(totalLen_ohi_id != -1)
            # Save phdr_ohi_id, totalLen_ohi_id,
            # in calfldobj where payload checksum is verified (udp/tcp checksum)
            calfldobj.ParserCsumObjGet().CsumOhiPhdrSelSet(phdr_ohi_id)
            calfldobj.ParserCsumObjGet().CsumOhiTotalLenSet(totalLen_ohi_id)

        ohi_id = parser.get_ohi_hdr_start_off(self.be.h.p4_header_instances[\
                                              calfldobj.CalculatedFieldHdrGet()])
        if ohi_id != None:
            ncc_assert(hdr_ohi_id == ohi_id)
            ohi_instr_generated = False
            for allocated_ohi_instr in ohi_inst_allocator:
                if allocated_ohi_instr:
                    _expr, _ohi_id = allocated_ohi_instr
                    if _ohi_id == ohi_id:
                        ohi_instr_generated = True
                        break
            if not ohi_instr_generated:
                ncc_assert(ohi_instr_inst < len(sram['ohi_inst']))
                # Ohi Instr to capture HdrStart Offset
                select          = 1 # Load current offset + index
                na              = 0 # NA
                index           = 0
                log_str += ParserCalField._build_ohi_instr(sram, ohi_instr_inst, select, \
                                                           na, index, hdr_ohi_id)
                ohi_instr_inst += 1

        if not calfldobj.payload_checksum and not calfldobj.option_checksum:
            # When no payload, assume its always ipv4 hdr checksum. Its not
            # expected to have cal fld on any collection of p4 fields.
            # Target doesn't support it; Instead target only takes
            # (start-offset, size) to compute checksum.

            mux_instr_generated = False
            for allocated_mux_instr in mux_inst_allocator:
                if allocated_mux_instr:
                    _, _, _expr = allocated_mux_instr
                    if _expr == hdrlen_expr:
                        mux_instr_generated = True
                        break

            if not mux_instr_generated and (l4len_mux_instr_id != -1) and \
               (l4len_mux_idx_id != -1) and \
               (hdrlen_mux_idx_id != -1) :
                index       = 0
                mask        = 0x0F00
                shift_left  = 0
                shift_val   = 6
                add_sub     = 0
                add_sub_val = 0
                load_mux_pkt = 1
                log_str += ParserCalField._build_mux_instr(sram, 1,                 \
                                                 l4len_mux_instr_id,                \
                                                 hdrlen_mux_idx_id, mask, add_sub,  \
                                                 add_sub_val, shift_val,            \
                                                 shift_left, load_mux_pkt,          \
                                                 l4len_mux_idx_id)

            #Configure checksum profile.
            csum_profile_obj = calfldobj.ParserCsumProfileGet()
            csum_profile_obj.CsumProfileShiftLeftSet(shift_left=0,\
                                                     shift_val=0)
            csum_profile_obj.CsumProfileStartAdjSet(addsub_start=0,\
                                                    start_adj=0)
            csum_profile_obj.CsumProfileEndAdjSet(1, calfldobj.end_adj_const)
            csum_profile_obj.CsumProfilePhdrSet(addsub_phdr=0,\
                                                phdr_adj=0)
            # 10th byte offset in ipv4 hdr where hdr checksum field is.
            csum_profile_obj.CsumProfileCsumLocSet(addsub_csum_loc=1,\
                                                   csum_loc_adj=10)
        if calfldobj.payload_checksum:
            # UDP/TCP case
            #  - Get PhdrOffset OHI# from IP parse state
            #  - Get PayLoadLen OHI# if parser moved from v4 to TCP
            #    else for v6 to TCP, read stored lkp_reg where PayLoadLen
            #    is stored (v6 options is already decremented) and use it
            #    as payloadLen value for checksum.
            #
            # In case of transitioning into TCP state from ipv6 state,
            # calculated PayloadLen when in ipv6 parse state also includes
            # ipv6 options as well. Size of all ipv6 options needs to be
            # decremented. Decrementing option size should be specified
            # in P4.
            csum_profile_obj = calfldobj.ParserCsumProfileGet()
            phdr_profile_obj = calfldobj.ParserPhdrProfileGet()
            addsub_csum_loc = 1
            if calfldobj.CsumPayloadHdrTypeGet() == 'tcp':
                addlen = 1
                # checksum location byte offset in TCP=16/UDP=6 hdr
                csum_loc_adj = 16
            elif calfldobj.CsumPayloadHdrTypeGet() == 'udp':
                addlen = 1
                csum_loc_adj = 6
            else:
                ncc_assert(0)
            log_str += calfldobj.PhdrProfileBuild(phdr_parse_state,\
                                              phdr_profile_obj, addlen)
            log_str += calfldobj.PayloadCsumProfileBuild(phdr_parse_state,                  \
                                                         csum_profile_obj,                  \
                                                         calfldobj.CsumPayloadHdrTypeGet())
            csum_profile_obj.CsumProfileCsumLocSet(addsub_csum_loc,\
                                                   csum_loc_adj)
        if calfldobj.option_checksum:
            addsub_csum_loc = 1
            csum_profile_obj = calfldobj.ParserCsumProfileGet()
            #Udp option checksum is 8b value.
            csum_profile_obj.CsumProfileCsum8bSet(1)
            csum_loc_adj = 1 #TODO:
            log_str += calfldobj.PayloadCsumProfileBuild(None,                  \
                                                         csum_profile_obj, None)
            csum_profile_obj.CsumProfileCsumLocSet(addsub_csum_loc,\
                                                   csum_loc_adj)

        calfldobj.ParserCsumObjAddLog(log_str)

        #Program Csum unit
        csum_instr = 0
        log_str = ''
        csum_unit = calfldobj.ParserCsumObjGet().CsumUnitNumGet()
        csum_profile = calfldobj.ParserCsumProfileGet().\
                                    CsumProfileUnitNumGet()
        hdr_ohi_id = calfldobj.ParserCsumObjGet().CsumOhiStartSelGet()
        if calfldobj.payload_checksum:
            phdr_ohi_id = calfldobj.ParserCsumObjGet().CsumOhiPhdrSelGet()
            phdr_profile = calfldobj.ParserPhdrProfileGet().\
                                        CsumPhdrProfileUnitNumGet()
            ncc_assert(phdr_profile != None)
            #Get ohi id that has totalLen/PayloadLen - v6-option-size or ihl * 4
            len_ohi_id = calfldobj.ParserCsumObjGet().CsumOhiTotalLenGet()
        else:
            phdr_ohi_id = -1
            phdr_profile = -1
            len_ohi_id = calfldobj.ParserCsumObjGet().CsumOhiLenSelGet()

        log_str += ParserCalField._build_csum_instr(sram, calfldobj, csum_instr,
                                          1, csum_unit,csum_profile,\
                                          hdr_ohi_id, len_ohi_id,\
                                          phdr_profile, phdr_ohi_id, \
                                          parse_state.enable_udp_zero_csum_error)
        csum_instr += 1
        calfldobj.ParserCsumObjAddLog(log_str)
        ncc_assert(csum_instr < len(sram['csum_inst']))

        return ohi_instr_inst


    def ParserCsumProfileGenerate(self, parser, parse_states_in_path,\
                                  parse_state, csum_t):
        profile = None
        profile_obj = None
        p = -1
        phdr_in_path = False
        profile_list = []

        # Since this function is called on parse state where
        # calculated field may not be extracted, check for
        # existence of calfld obj
        if not len(parse_state.verify_cal_field_objs):
            return profile_list

        for calfldobj in parse_state.verify_cal_field_objs:
            if calfldobj.payload_checksum:
                calfldobj, phdr_parse_state = self._CsumFindCalFldObjMatchingPhdr(\
                                                  parse_state, parse_states_in_path)

            profile_obj = calfldobj.ParserCsumProfileGet()
            if profile_obj != None:
                profile = copy.deepcopy(csum_t)
                p = profile_obj.csum_profile
                if p == -1:
                    ncc_assert(0)
                log_str = profile_obj.ConfigGenerate(profile)
                calfldobj.ParserCsumObjAddLog(log_str)
                #Only one calFld processed in any parse state
                profile_list.append((profile, p))

        return profile_list

    def ParserCsumPhdrProfileGenerate(self, parser, parse_states_in_path,\
                                      parse_state, csum_phdr_t):
        phdr_profile = None
        p = -1
        phdr_in_path = False
        profile_list = []

        if not len(parse_state.verify_cal_field_objs):
            return profile_list
        from_parse_state = parse_states_in_path[-2]
        headers_in_parse_path = []
        for _parse_state in parse_states_in_path:
            headers_in_parse_path += _parse_state.headers

        for calfldobj in parse_state.verify_cal_field_objs:
            if calfldobj.payload_checksum:
                calfldobj, phdr_parse_state = self._CsumFindCalFldObjMatchingPhdr(\
                                                  parse_state, parse_states_in_path)
                phdr_profile_obj = calfldobj.ParserPhdrProfileGet()
                if phdr_profile_obj != None:
                    phdr_profile = copy.deepcopy(csum_phdr_t)
                    p = phdr_profile_obj.phdr_profile
                    phdr_profile_obj.ConfigGenerate(phdr_profile)
                    profile_list.append((phdr_profile, p))

        return profile_list

    def ParserCsumUnitAllocationCodeGenerate(self):
        '''
            Generates Csum Engine Allocation to CsumHeader mapping.
            This mapping is needed for MPU microcode to check for
            checksum error
        '''
        gen_dir = self.be.args.gen_dir
        cur_path = gen_dir + '/%s/asm_out' % self.be.prog_name
        if not os.path.exists(cur_path):
            try:
                os.makedirs(cur_path)
            except OSError as e:
                if e.errno != errno.EEXIST:
                    raise
        for d in xgress:
            parser = self.be.parsers[d]
            fname = cur_path + '/CSUM_%s.h' % parser.d.name
            hfile = open(fname, 'w')
            HdrCsumD = {}

            if parser.d == xgress.INGRESS:
                verify_cal_fieldlist = self.verify_cal_fieldlist
                ParserCsumEngineObj = self.IngParserCsumEngineObj
            else:
                verify_cal_fieldlist = self.eg_verify_cal_fieldlist
                ParserCsumEngineObj = self.EgParserCsumEngineObj

            for calfldobj in verify_cal_fieldlist:
                csum_hdr = calfldobj.CalculatedFieldHdrGet()
                csum_unit = calfldobj.ParserCsumObjGet().CsumUnitNumGet()
                if csum_hdr not in HdrCsumD.keys():
                    HdrCsumD[csum_hdr] = csum_unit
                else:
                    ncc_assert(HdrCsumD[csum_hdr] == csum_unit)
            csum_str = '//The file contaiins mapping of checksum header to checksum '\
                       'engine allocation.\n'
            csum_str += '//Checksum Engine allcation number can be used to check '\
                        'checksum error status bit.\n'
            csum_str += "\n\n\n\n\n"
            hfile.write(csum_str)
            for k, v in HdrCsumD.items():
                csum_str = "#define csum_hdr_{:12s}{:8d}\n".format(k, v)
                hfile.write(csum_str)
            csum_str += "\n"
            hfile.close()

    # -- Methods to compute checksum in parser : GSO csum --

    def ProcessGsoCsumComputeObj(self, d):
        '''
        This function will process GSO csum objs
        '''
        if d == xgress.INGRESS:
            gso_cal_fieldlist_compute = self.gso_cal_fieldlist_compute
        else:
            gso_cal_fieldlist_compute = self.eg_gso_cal_fieldlist_compute

        for calfldobj in gso_cal_fieldlist_compute:
            calfldhdr = calfldobj.CalculatedFieldHdrGet()
            if calfldobj.ParserCsumObjGet() == None:
                calfldobj.ParserCsumObjSet(ParserCsumObj())
                calfldobj.ParserCsumProfileSet(ParserCsumProfile())

    def InsertGsoCsumObjReferenceInParseState(self, parser):
        '''
            All parse states, that are annotated with pargma
            'generic_checksum_start', insert GSO CsumObj reference.
        '''
        if parser.d == xgress.INGRESS:
            gso_cal_fieldlist_compute = self.gso_cal_fieldlist_compute
        else:
            gso_cal_fieldlist_compute = self.eg_gso_cal_fieldlist_compute
        for parse_state in parser.states:
            if parse_state.generic_checksum_start() != None:
                gso_csum_hfname = parse_state.generic_checksum_start()
                for calfldobj in gso_cal_fieldlist_compute:
                    if calfldobj.dstField == gso_csum_hfname:
                        parse_state.gso_csum_calfldobj = calfldobj
                        break

    def AllocateParserGsoCsumResources(self, parser):
        self.InsertGsoCsumObjReferenceInParseState(parser)
        if parser.d == xgress.INGRESS:
            gso_cal_fieldlist_compute = self.gso_cal_fieldlist_compute
            ParserCsumEngineObj = self.IngParserCsumEngineObj
        else:
            gso_cal_fieldlist_compute = self.eg_gso_cal_fieldlist_compute
            ParserCsumEngineObj = self.EgParserCsumEngineObj
        for calfldobj in gso_cal_fieldlist_compute:
            #Allocate csum unit
            #Get ohi_start id and save it in
            csum_obj = calfldobj.ParserCsumObjGet()
            csum_profile_obj = calfldobj.ParserCsumProfileGet()
            csum_obj.CsumUnitNumSet(ParserCsumEngineObj.AllocateCsumUnit(calfldobj.dstField))
            csum_profile_obj.CsumProfileUnitNumSet(ParserCsumEngineObj.\
                                AllocateCsumProfile(calfldobj.dstField))
            csum_obj.CsumOhiStartSelSet(parser.\
                    get_ohi_slot_wr_only_field_name(calfldobj.gso_start_field))
            cf = self.be.pa.get_field(calfldobj.gso_csum_result_fld_name, parser.d)
            ncc_assert(cf != None)
            csum_profile_obj.CsumProfileCsumPhvFlitSet(cf.phv_bit / self.be.hw_model['phv']['flit_size'])
            for field in self.be.h.p4_header_instances[calfldobj.csum_hfield_name.split(".")[0]].fields:
                if field.name == calfldobj.csum_hfield_name.split(".")[1] and field.width <= 8:
                    csum_profile_obj.CsumProfileCsum8bSet(1)

    def GsoCsumParserConfigGenerate(self, parser, parse_states_in_path,\
                                    parse_state, sram):
        '''
         The parse state where GSO csum branching happens.
        '''
        from_parse_state = parse_states_in_path[-2]
        headers_in_parse_path = []
        for _parse_state in parse_states_in_path:
            headers_in_parse_path += _parse_state.headers

        log_str = ''
        log_str += 'Gso CsumConfig: %s --> %s\n' \
                    % (from_parse_state.name, parse_state.name)
        log_str += '_____________________________________\n\n'

        calfldobj = parse_state.gso_csum_calfldobj
        if calfldobj == None:
            ncc_assert(0)
        gso_start_ohi_id = calfldobj.ParserCsumObjGet().CsumOhiStartSelGet()
        ncc_assert(gso_start_ohi_id != -1)
        csum_obj = calfldobj.ParserCsumObjGet()
        csum_profile_obj = calfldobj.ParserCsumProfileGet()
        calfldobj.GsoCsumProfileBuild(parse_state, csum_profile_obj)

        #Program Csum unit
        csum_instr = 0
        csum_unit = csum_obj.CsumUnitNumGet()
        csum_profile = csum_profile_obj.CsumProfileUnitNumGet()
        start_ohi_id = calfldobj.ParserCsumObjGet().CsumOhiStartSelGet()
        log_str += ParserGsoCalField._build_csum_instr(sram, calfldobj, csum_instr,
                                                       1, csum_unit,csum_profile,\
                                                       start_ohi_id, 0)
        csum_instr += 1
        calfldobj.GsoCsumObjAddLog(log_str)
        ncc_assert(csum_instr < len(sram['csum_inst']))

    def GsoCsumParserProfileGenerate(self, parser, parse_states_in_path,\
                                     parse_state, csum_t):
        profile = None
        profile_obj = None
        p = -1
        phdr_in_path = False
        if not parse_state.gso_csum_calfldobj:
            return profile, p
        calfldobj = parse_state.gso_csum_calfldobj
        profile_obj = calfldobj.ParserCsumProfileGet()
        if profile_obj != None:
            profile = copy.deepcopy(csum_t)
            p = profile_obj.csum_profile
            if p == -1:
                ncc_assert(0)
            log_str = profile_obj.ConfigGenerate(profile)
            calfldobj.GsoCsumObjAddLog(log_str)
            #Only one calFld processed in any parse state
            return profile, p
        else:
            ncc_assert(0)
        return profile, p

    # -- Methods to process update calculated field list to compute checksum --

    def DeParserPhdrCsumObjSet(self, phdr_name, dpr_csum_obj, csum_unit, hdr, d):
        dpr_phdr_csum_obj = self.dpr_phdr_csum_obj if d == xgress.INGRESS \
                                              else self.eg_dpr_phdr_csum_obj
        if (phdr_name, hdr) not in dpr_phdr_csum_obj.keys():
            dpr_phdr_csum_obj[(phdr_name, hdr)] = [(dpr_csum_obj, csum_unit, hdr)]
        else:
            dpr_phdr_csum_obj[(phdr_name, hdr)].append((dpr_csum_obj, csum_unit, hdr))

    def DeParserPhdrCsumObjGet(self, phdr_name, csum_unit, hdr, d):
        dpr_phdr_csum_obj = self.dpr_phdr_csum_obj if d == xgress.INGRESS \
                                              else self.eg_dpr_phdr_csum_obj
        if (phdr_name, hdr) in dpr_phdr_csum_obj.keys():
            for csum_obj, _csum_unit, hdr in dpr_phdr_csum_obj[(phdr_name, hdr)]:
                if csum_unit == _csum_unit:
                    return csum_obj
            return None
        return None

    def IsHdrInCsumCompute(self, hdrname, d):
        update_cal_fieldlist = self.update_cal_fieldlist if d == xgress.INGRESS \
                                              else self.eg_update_cal_fieldlist
        for calfldobj in update_cal_fieldlist:
            if calfldobj.CalculatedFieldHdrGet() == hdrname:
                return True
        return False

    def IsHdrInPayLoadCsumCompute(self, hdrname, d):
        update_cal_fieldlist = self.update_cal_fieldlist if d == xgress.INGRESS \
                                              else self.eg_update_cal_fieldlist
        for calfldobj in update_cal_fieldlist:
            if calfldobj.payload_checksum and \
               calfldobj.CalculatedFieldHdrGet() == hdrname:
                return True
        return False

    def IsHdrInCsumComputePhdr(self, hdrname, d):
        update_cal_fieldlist = self.update_cal_fieldlist if d == xgress.INGRESS \
                                              else self.eg_update_cal_fieldlist
        for calfldobj in update_cal_fieldlist:
            if calfldobj.payload_checksum and \
               calfldobj.phdr_name == hdrname:
                return True
        return False

    def IsHdrInGsoCsumCompute(self, hdrname, d):
        if d == xgress.INGRESS:
            gso_cal_fieldlist_update = self.gso_cal_fieldlist_update
        else:
            gso_cal_fieldlist_update = self.eg_gso_cal_fieldlist_update
        for calfldobj in gso_cal_fieldlist_update:
            if calfldobj.payload_checksum and \
               calfldobj.GsoCalculatedFieldHdrGet() == hdrname:
                return True
        return False

    def IsHdrInL2CompleteCsumCompute(self, hdrname, d):
        l2_csum_cal_fieldlist_update = self.l2_csum_cal_fieldlist_update if d == xgress.INGRESS \
                                              else self.eg_l2_csum_cal_fieldlist_update
        for calfldobj in l2_csum_cal_fieldlist_update:
            if calfldobj.CalculatedFieldHdrGet() == hdrname:
                return True
        return False

    def IsL2HdrInL2CompleteCsumCompute(self, hdrname, d):
        l2_csum_cal_fieldlist_update = self.l2_csum_cal_fieldlist_update if d == xgress.INGRESS \
                                              else self.eg_l2_csum_cal_fieldlist_update
        for calfldobj in l2_csum_cal_fieldlist_update:
            if calfldobj.phdr_name == hdrname:
                return True
        return False

    def IsHdrInOptionCsumCompute(self, hdrname, d):
        update_cal_fieldlist = self.update_cal_fieldlist if d == xgress.INGRESS \
                                              else self.eg_update_cal_fieldlist
        for calfldobj in update_cal_fieldlist:
            if calfldobj.CalculatedFieldHdrGet() == hdrname and \
               calfldobj.option_checksum:
                return True
        return False

    #If checksum header is computed over payload, and without pseudo hdr fiedls
    #return TRUE;  case: ICMP header and with ipv4
    def IsPayloadCsumComputeWithNoPhdr(self, hdrname, d):
        update_cal_fieldlist = self.update_cal_fieldlist if d == xgress.INGRESS \
                                              else self.eg_update_cal_fieldlist
        for calfldobj in update_cal_fieldlist:
            if calfldobj.CalculatedFieldHdrGet() == hdrname and \
               calfldobj.no_phdr_in_checksum:
                return True
        return False

    #Return count of the L3 hdr as pseudo hdr in payload checksum.
    def L3HdrAsPseudoHdr(self, hdrname, d):
        update_cal_fieldlist = self.update_cal_fieldlist if d == xgress.INGRESS \
                                              else self.eg_update_cal_fieldlist
        count = 0
        for calfldobj in update_cal_fieldlist:
            if calfldobj.CsumPhdrNameGet() == hdrname:
                count += 1
        return count

    def DeParserPayLoadLenSlotGet(self, calfldobj, parser):
        ncc_assert(calfldobj.payload_update_len_field != '')
        cf_pl_update_len = self.be.pa.get_field(calfldobj.payload_update_len_field, parser.d)
        pl_slot = (cf_pl_update_len.phv_bit - 
                   self.be.hw_model['phv']['flit_size']) / 16

        return pl_slot

    def DeParserInnerPayLoadCsumInclude(self, outer_csum_objs, csum_hdr,\
                                        csum_obj):
        if not len(outer_csum_objs):
            return

        csum_unit = csum_obj.CsumUnitNumGet()

        self.csum_compute_logger.debug('    Including Csum result of %s in '\
                                       'computation of following headers' % \
                                       (csum_hdr))
        #Include csum_unit in all outer payload checksum computation.
        #This is provided in csum_profile as csum-engine bit mask
        for outer_csum_obj, _csum_hdr in outer_csum_objs:
            if outer_csum_obj.CsumUnitNumGet() != csum_unit:
                outer_csum_obj.CsumInnerCsumResultInclude(csum_unit)
                self.csum_compute_logger.debug('    Payload Csum %s value includes '\
                                               ' payload csum result of %s' % \
                                               (_csum_hdr, csum_hdr))

    def ProcessUpdateCalFldList(self, d):
        '''
         Process all update calculated fields
         and creates checksum objects
        '''
        update_cal_fieldlist = self.update_cal_fieldlist if d == xgress.INGRESS \
                                              else self.eg_update_cal_fieldlist
        l2_csum_cal_fieldlist_update = self.l2_csum_cal_fieldlist_update if d == xgress.INGRESS \
                                              else self.eg_l2_csum_cal_fieldlist_update
        for calfldobj in update_cal_fieldlist:
            #Allocate checksum and checksum profile
            if calfldobj.DeParserCsumObjGet() == None:
                calfldobj.DeParserCsumObjSet(DeParserCsumObj())
                calfldobj.DeParserCsumProfileObjSet(DeParserCsumProfile())
                if calfldobj.payload_checksum:
                    phdr_name = calfldobj.CsumPhdrNameGet()
                    ncc_assert(phdr_name != '')
                    ncc_assert(phdr_name in self.be.h.p4_header_instances)

        # Ensure/CrossCheck that every calculated field that needs to
        # be updated has been allocated necessary sw objects.
        for calfldobj in update_cal_fieldlist:
            if calfldobj.DeParserCsumObjGet() == None:
                ncc_assert(0)

        # In case of deparser, PseudoHdrs also need object representation
        # because deparser csum computation works using HV bits.
        csum_phdrs = set()
        for calfldobj in update_cal_fieldlist:
            csum_phdr = calfldobj.CsumPhdrNameGet()
            if csum_phdr != '' and csum_phdr not in csum_phdrs:
                csum_phdrs.add(csum_phdr)
                self.DeParserPhdrCsumObjSet(csum_phdr, DeParserCsumObj(), -1, '', d)

        #L2 complete csum objects. Each object in l2_csum_cal_fieldlist_udpate
        #represents L3 hdr encapped in outer L2.
        for calfldobj in l2_csum_cal_fieldlist_update:
            #Allocate checksum and checksum profile
            if calfldobj.DeParserCsumObjGet() == None:
                calfldobj.DeParserCsumObjSet(DeParserCsumObj())
                calfldobj.DeParserCsumProfileObjSet(DeParserCsumProfile())
        if len(l2_csum_cal_fieldlist_update):
            #Inorder to copy L2 csum into internal header, lets also
            #create an calfld object for internal header.
            calfldobj = copy.copy(l2_csum_cal_fieldlist_update[0])
            calfldobj.phdr_name = '' #This identifies as internal hdr
            calfldobj.DeParserCsumObjSet(DeParserCsumObj())
            calfldobj.DeParserCsumProfileObjSet(DeParserCsumProfile())
            l2_csum_cal_fieldlist_update.append(calfldobj)

    def BuildDeParserCsumNonPayloadObject(self, parser, hdr, calfldobj):
        DeParserCsumEngineObj = self.IngDeParserCsumEngineObj if parser.d == xgress.INGRESS \
                                               else self.EgDeParserCsumEngineObj
        csum_obj = calfldobj.DeParserCsumObjGet()
        csum_obj.HvBitNumSet(parser.hdr_hv_bit[hdr])
        csum_obj.CsumHvBitNumSet(parser.csum_hdr_hv_bit[hdr][0][0])
        csum_obj.CsumHvBitStrSet(parser.csum_hdr_hv_bit[hdr][0][2])
        calfldhdr = calfldobj.CalculatedFieldHdrGet()
        csum_obj.CsumUnitNumSet(DeParserCsumEngineObj.\
                                 AllocateCsumUnit(calfldhdr, calfldobj, True))
        csum_obj.PhdrValidSet(0)
        csum_profile = DeParserCsumEngineObj.AllocateCsumProfile(calfldhdr)
        #Config Csum Profile
        csum_obj.CsumProfileNumSet(csum_profile)
        csum_profile_obj = calfldobj.DeParserCsumProfileObjGet()
        csum_profile_obj.CsumProfileNumSet(csum_profile)
        csum_profile_obj.CsumProfilePhvLenSelSet(0,0)
        csum_field_offset = -1
        for field in hdr.fields:
            if field.name == calfldobj.dstField.split(".")[1]:
                csum_field_offset = field.offset/8 #bit to byte offset
                break
        ncc_assert(csum_field_offset != -1)
        csum_profile_obj.CsumProfileCsumLocSet(csum_field_offset)
        if calfldobj.option_checksum:
            csum_profile_obj.CsumProfileCsumLocSet(1)
            csum_profile_obj.CsumEightBitSet(1)
        if calfldobj.payload_update_len_field != '':
            dprsr_payload_len_slot = \
                    self.DeParserPayLoadLenSlotGet(calfldobj, parser)
            csum_profile_obj.CsumProfilePhvLenSelSet(1,dprsr_payload_len_slot)
        self.csum_compute_logger.debug("%s" % csum_obj.LogGenerate(hdr.name))
        self.csum_compute_logger.debug(" %s" %
                         csum_profile_obj.LogGenerate())
        self.csum_compute_logger.debug('\n')


    def BuildDeParserCsumPayloadObject(self, parser, hdr, phdr, pl_calfldobj, payload_csum_obj_list):
        DeParserCsumEngineObj = self.IngDeParserCsumEngineObj if parser.d == xgress.INGRESS \
                                               else self.EgDeParserCsumEngineObj
        calfldhdr = pl_calfldobj.CalculatedFieldHdrGet()
        # Payload checksum node needs 1 csum engine
        # and 1 csum profile but more csum phdr profiles
        # depending on which pseudo hdr is in the packet
        # /parse path.
        csum_unit = DeParserCsumEngineObj.AllocateCsumUnit(calfldhdr, pl_calfldobj)
        csum_profile = DeParserCsumEngineObj.AllocateCsumProfile(calfldhdr)
        csum_obj = pl_calfldobj.DeParserCsumObjGet()
        csum_obj.CsumUnitNumSet(csum_unit)
        #Include result of inner payload checksum into
        #outer payload checksum computation
        self.DeParserInnerPayLoadCsumInclude(payload_csum_obj_list, hdr.name, csum_obj)
        payload_csum_obj_list.append((csum_obj, hdr.name))
        csum_obj.HvBitNumSet(parser.hdr_hv_bit[hdr])
        csumhv = -1
        l3hf_name = calfldhdr + '.csum'
        for elem in parser.csum_hdr_hv_bit[hdr]:
            if elem[2] == l3hf_name:
                csumhv = elem[0]
                break
        ncc_assert(csumhv != -1)
        csum_obj.CsumHvBitNumSet(csumhv)
        csum_obj.CsumHvBitStrSet(l3hf_name)
        csum_obj.CsumProfileNumSet(csum_profile)
        csum_obj.PhdrValidSet(0)
        csum_profile_obj = pl_calfldobj.DeParserCsumProfileObjGet()
        csum_profile_obj.CsumProfileNumSet(csum_profile)
        dprsr_payload_len_slot = \
                self.DeParserPayLoadLenSlotGet(pl_calfldobj, parser)
        csum_profile_obj.CsumProfilePhvLenSelSet(1, dprsr_payload_len_slot)

        #TODO use p4field hlir-offset to set checksum location
        if pl_calfldobj.CsumPayloadHdrTypeGet() == 'tcp':
            csum_profile_obj.CsumProfileCsumLocSet(16)
            csum_profile_obj.CsumProfilePhdrNextHdrSet(6)
            add_len = 1
        elif pl_calfldobj.CsumPayloadHdrTypeGet() == 'udp':
            csum_profile_obj.CsumProfileCsumLocSet(6)
            csum_profile_obj.CsumProfilePhdrNextHdrSet(17)
            add_len = 1
        elif pl_calfldobj.CsumPayloadHdrTypeGet() == 'icmp':
            csum_profile_obj.CsumProfileCsumLocSet(2)
            #next header field is used from p4 code (listed as part
            #of field-list.)Hence icmp csum with ipv6 can be supported
            #without ipv6 option fields. When ipv6 options also
            #supported in icmp packet, then NextHdr should be set to 58 
            csum_profile_obj.CsumProfilePhdrNextHdrSet(0)
            #For icmp csum w/ v6, ipv6.payloadLenis specified in p4 as
            #pseudo hdr field. Hence do not specify add-len
            add_len = 0
        csum_profile_obj.CsumProfileAddLenSet(add_len)

        if self.DeParserPhdrCsumObjGet(phdr, csum_unit,hdr.name, parser.d) == None:
            #Set Phdr profile#, in csum hdr object of Phdr
            #Phdr that this payload csum uses.
            phdr_csum_obj = \
                    self.DeParserPhdrCsumObjGet(phdr, -1, '', parser.d)
            ncc_assert(phdr_csum_obj != None)
            phdr_csum_obj = copy.copy(phdr_csum_obj)
            phdr_inst = self.be.h.p4_header_instances[phdr]
            phdr_csum_obj.HvBitNumSet(parser.hdr_hv_bit[phdr_inst])
            if pl_calfldobj.CsumPayloadHdrTypeGet() == 'tcp':
                csumhv = -1
                l3hf_name = pl_calfldobj.phdr_name + '.tcp_csum'
                for elem in parser.csum_hdr_hv_bit[phdr_inst]:
                    if elem[2] == l3hf_name:
                        csumhv = elem[0]
                        break
                ncc_assert(csumhv != -1)
                phdr_csum_obj.CsumHvBitNumSet(csumhv)
                phdr_csum_obj.CsumHvBitStrSet(l3hf_name)
            elif pl_calfldobj.CsumPayloadHdrTypeGet() == 'udp':
                csumhv = -1
                l3hf_name = pl_calfldobj.phdr_name + '.udp_csum'
                for elem in parser.csum_hdr_hv_bit[phdr_inst]:
                    if elem[2] == l3hf_name:
                        csumhv = elem[0]
                        break
                ncc_assert(csumhv != -1)
                phdr_csum_obj.CsumHvBitNumSet(csumhv)
                phdr_csum_obj.CsumHvBitStrSet(l3hf_name)
            elif pl_calfldobj.CsumPayloadHdrTypeGet() == 'icmp':
                #pdb.set_trace()
                csumhv = -1
                l3hf_name = pl_calfldobj.phdr_name + '.icmp_csum'
                for elem in parser.csum_hdr_hv_bit[phdr_inst]:
                    if elem[2] == l3hf_name:
                        csumhv = elem[0]
                        break
                ncc_assert(csumhv != -1)
                phdr_csum_obj.CsumHvBitNumSet(csumhv)
                phdr_csum_obj.CsumHvBitStrSet(l3hf_name)
            else:
                pdb.set_trace()

            phdr_profile = DeParserCsumEngineObj.AllocatePhdrProfile(calfldhdr, phdr)
            phdr_csum_obj.PhdrProfileNumSet(phdr_profile)
            self.DeParserPhdrCsumObjSet(phdr, phdr_csum_obj, csum_unit, hdr.name, parser.d)
            pl_calfldobj.DeParserPhdrCsumObjSet(phdr_csum_obj)
            ncc_assert(pl_calfldobj.DeParserPhdrProfileObjGet() == None)
            use_payload_len_from_l3_hdr = 1 if pl_calfldobj.CsumPayloadHdrTypeGet() == 'icmp' else 0
            pl_calfldobj.DeParserPhdrProfileObjSet(\
                          DeParserPhdrProfile(phdr_profile,  \
                                              pl_calfldobj.CsumPhdrTypeGet(), \
                                              pl_calfldobj.phdr_fields, 0, \
                                              use_payload_len_from_l3_hdr))
        else:
            ncc_assert(0)

        phdr_csum_obj.PhdrValidSet(1)
        phdr_csum_obj.PhdrUnitSet(csum_unit)
        self.csum_compute_logger.debug('%s' % csum_obj.LogGenerate(hdr.name))
        self.csum_compute_logger.debug('%s' % csum_profile_obj.LogGenerate())
        self.csum_compute_logger.debug('%s' % phdr_csum_obj.LogGenerate(pl_calfldobj.phdr_name))
        self.csum_compute_logger.debug('\n')


    def AllocateDeParserCsumResources(self, parser):
        '''
         Walk all parse paths and assign resources
         to update checksum.
        '''
        DeParserCsumEngineObj = self.IngDeParserCsumEngineObj if parser.d == xgress.INGRESS \
                                              else self.EgDeParserCsumEngineObj 
        csum_hdrs = set()
        csum_phdrs = set()
        update_cal_fieldlist = self.update_cal_fieldlist if parser.d == xgress.INGRESS \
                                              else self.eg_update_cal_fieldlist
        for calfldobj in update_cal_fieldlist:
            csum_hdr = calfldobj.CalculatedFieldHdrGet()
            if csum_hdr not in csum_hdrs:
                csum_hdrs.add(csum_hdr)
            csum_phdr = calfldobj.CsumPhdrNameGet()
            if csum_phdr != '' and csum_phdr not in csum_phdrs:
                csum_phdrs.add(csum_phdr)

        all_csum_hdrs = csum_hdrs.union(csum_phdrs)
        csum_objects = set(calfldobj for calfldobj in update_cal_fieldlist)

        all_parse_paths = sorted(parser.paths, key=lambda p: len(p), reverse=True)
        for parse_path in all_parse_paths:
            csum_assign = False
            parse_path_hdrs = set(hdr.name for hdr in parse_path)
            if len(parse_path_hdrs.intersection(all_csum_hdrs)):
                csum_assign = True

            if csum_assign:
                # Before assigning csum units along parse path, start
                # assigning from zeroth csum unit. However assign same
                # csum unit, csum profile if an header was already
                # along a parse a path that was already processed.
                DeParserCsumEngineObj.ResetCsumResources()
                #Find which phdrs are in the path.
                pseudo_hdrs = None
                _s = parse_path_hdrs.intersection(csum_phdrs)
                if len(_s):
                    ncc_assert(len(_s) <= 3)
                    pseudo_hdrs = _s

                #After resetting csum unit allocation to start from start,
                #ensure same csum unit is assigned if the hdr is also
                #in another parse path.
                for hdr in parse_path:
                    if hdr.name not in all_csum_hdrs:
                        continue
                    calfldobjlist = self.UpdateCalFieldObjGet(parser.d, hdr.name, '')
                    if calfldobjlist == None:
                        continue
                    for calfldobj in calfldobjlist:
                        if calfldobj not in csum_objects \
                           and not calfldobj.option_checksum:
                            #This header was allocated Csum unit
                            #while processing parse path previously.
                            #Provide same csum unit.
                            DeParserCsumEngineObj.AlreadyAllocatedCsumUnitSet(\
                                    calfldobj.CalculatedFieldHdrGet(),\
                                    calfldobj.DeParserCsumObjGet().CsumUnitNumGet(), calfldobj.payload_checksum)

                current_phdr_obj = None
                payload_csum_obj_list = [] #used to include inner payload csum
                                           #result into outer PL csum calculation
                for hdr in parse_path:
                    if hdr.name not in all_csum_hdrs:
                        continue
                    calfldobjlist = self.UpdateCalFieldObjGet(parser.d, hdr.name, '')
                    if calfldobjlist == None:
                        # ipv6 doesn't have calculated field object..
                        phdr_csum_obj = self.DeParserPhdrCsumObjGet(hdr.name, -1, '', parser.d)
                        ncc_assert(phdr_csum_obj != None)
                        #CsumHV bit for phdrs that are not calfldobjs (ipv6) is
                        #same as HV bit.
                        continue
                    for calfldobj in calfldobjlist:
                        if not calfldobj.payload_checksum and calfldobj in csum_objects:
                            self.csum_compute_logger.debug(\
                            '%s: Checksum Assignment along path %s' % (xgress_to_string(parser.d), str(parse_path)))
                            self.BuildDeParserCsumNonPayloadObject(parser, hdr, calfldobj)
                            csum_objects.remove(calfldobj)

                        elif calfldobj.payload_checksum:
                            #In order to associate right phdr, maintain
                            #phdrs in the same order as they are in parse path.
                            #This is needed is because in case of v4, v6, tcp pkt
                            #phdr of tcp is v6 not v4.
                            topo_phdrs_in_parse_path = []
                            for _hdr in parse_path:
                                if _hdr.name in pseudo_hdrs:
                                    topo_phdrs_in_parse_path.append(_hdr.name)
                                # Collect phdrs until payload hdr only.
                                if _hdr.name == hdr.name: break
                            topo_phdrs_in_parse_path.reverse()
                            phdr = topo_phdrs_in_parse_path[0]
                            pl_calfldobj = self.UpdateCalFieldObjGet(parser.d, hdr.name,\
                                                                     phdr)
                            if pl_calfldobj and pl_calfldobj not in csum_objects:
                                # This payload object hash already been processed.
                                # However to include result of inner csum into
                                # outer csum, process it.
                                _csum_obj = pl_calfldobj.DeParserCsumObjGet()
                                self.DeParserInnerPayLoadCsumInclude(\
                                            payload_csum_obj_list, hdr.name, _csum_obj)
                                payload_csum_obj_list.append((_csum_obj, hdr.name))

                            if pl_calfldobj and pl_calfldobj in csum_objects:
                                self.csum_compute_logger.debug('%s Checksum Assignment'\
                                                               ' along path %s' %
                                                               (xgress_to_string(parser.d), str(parse_path)))
                                #Inner L4 csum result is included in outer L4 csum
                                self.BuildDeParserCsumPayloadObject(parser, hdr, phdr, pl_calfldobj, payload_csum_obj_list)
                                csum_objects.remove(pl_calfldobj)

                    #Include inner L3 csum result into outer L4 csum
                    if not calfldobj.payload_checksum and not calfldobj.option_checksum:
                        _csum_obj = calfldobj.DeParserCsumObjGet()
                        self.DeParserInnerPayLoadCsumInclude(\
                                        payload_csum_obj_list, hdr.name, _csum_obj)

                if not len(csum_objects):
                    break

        if len(csum_objects):
            #Not all csum update objects processed. This could happen if one/more
            #csum headers are extracted in deparser-only states. Such headers will
            #not appear in parse_path. This can be done for csum update objects
            #only or objects that are in checksum.ig/eg_update_cal_fieldlist.
            #Add logic to walk through deparser only headers and if those headers
            # are also csum objects, then assign csum resources.
            deparse_only_hdrs = set(hdr.name for hdr in parser.deparse_only_hdrs)
            for calfldobj in update_cal_fieldlist:
                csum_hdr = calfldobj.CalculatedFieldHdrGet()
                if csum_hdr in deparse_only_hdrs:
                    if not calfldobj.payload_checksum and calfldobj in csum_objects:
                        self.csum_compute_logger.debug(\
                        '%s: Checksum Assignment along path %s' % (xgress_to_string(parser.d),
                         str(parse_path)))
                        self.BuildDeParserCsumNonPayloadObject(parser, \
                        parser.be.h.p4_header_instances[csum_hdr], calfldobj)
                        csum_objects.remove(calfldobj)
                    elif calfldobj.payload_checksum and calfldobj in csum_objects:
                        csum_phdr = calfldobj.CsumPhdrNameGet()
                        if csum_phdr != '' and parser.be.h.p4_header_instances[csum_phdr] \
                                               in parser.deparse_only_hdrs:
                            self.csum_compute_logger.debug('%s Checksum Assignment'\
                                                           ' along path %s' % \
                                                           (xgress_to_string(parser.d),\
                                                           str(parse_path)))
                            self.BuildDeParserCsumPayloadObject(parser, \
                                 parser.be.h.p4_header_instances[csum_hdr], \
                                 csum_phdr, calfldobj, payload_csum_obj_list)

                            csum_objects.remove(calfldobj)

        ncc_assert(len(csum_objects) == 0)
        DeParserCsumEngineObj.CsumUnitAllocationFinalize()


    def CsumDeParserHwConfigObjs(self, deparser, hv_fld_slots):
        self.csum_compute_logger.debug('%s' % ("HVB, StartFld, EndFld  HdrName:"))
        self.csum_compute_logger.debug('%s' % ("-------------------------------"))
        for hvb, hv_info in hv_fld_slots.items():
            self.csum_compute_logger.debug('%d %d %d %s' % \
            (deparser.be.hw_model['parser']['max_hv_bits'] - 1 - hvb, \
             hv_info[0], hv_info[1], hv_info[2]))
        self.csum_compute_logger.debug('\n')

        csum_hdrs = []
        hw_csum_obj = [] # list of csumobj that need to be programmed in HW
                         # without repeatation and maintaining Banyan contrainst.
        update_cal_fieldlist = self.update_cal_fieldlist if deparser.d == xgress.INGRESS \
                                              else self.eg_update_cal_fieldlist
        for calfldobj in update_cal_fieldlist:
            csum_hdr = calfldobj.CalculatedFieldHdrGet()
            _csum_obj = calfldobj.DeParserCsumObjGet()
            _csum_profile_obj = calfldobj.DeParserCsumProfileObjGet()
            ncc_assert(_csum_obj != None)
            ncc_assert(_csum_profile_obj != None)
            ncc_assert(_csum_obj.hv != -1)
            ncc_assert(_csum_obj.csum_hv != -1)
            fldstart, fldend, _ = hv_fld_slots[_csum_obj.csum_hv]
            _csum_obj.HdrFldStartEndSet(fldstart,fldend)
            if calfldobj.payload_checksum:
                _phdr_csum_obj = calfldobj.DeParserPhdrCsumObjGet()
                _phdr_profile_obj = calfldobj.DeParserPhdrProfileObjGet()
                ncc_assert(_phdr_csum_obj != None)
                ncc_assert(_phdr_profile_obj != None)
                ncc_assert(_phdr_csum_obj.csum_hv != None)
                fldstart, fldend, _ = hv_fld_slots[_phdr_csum_obj.csum_hv]
                _phdr_csum_obj.HdrFldStartEndSet(fldstart,fldend)

            #Generate Logical Output
            calfldobj.DeParserCsumObjAddLog(_csum_obj.LogGenerate(csum_hdr))
            calfldobj.DeParserCsumObjAddLog(_csum_profile_obj.LogGenerate())
            if calfldobj.payload_checksum:
                calfldobj.DeParserCsumObjAddLog(_phdr_csum_obj.\
                                            LogGenerate(calfldobj.phdr_name))
                calfldobj.DeParserCsumObjAddLog(_phdr_profile_obj.\
                                            LogGenerate(calfldobj.phdr_fields))
            if csum_hdr not in csum_hdrs:
                csum_hdrs.append(csum_hdr)
                hw_csum_obj.append((_csum_obj, False, calfldobj))
            #hw_csum_obj.append((_csum_obj, False, calfldobj))
            if calfldobj.payload_checksum:
                hw_csum_obj.append((_phdr_csum_obj, True, calfldobj))

        return  hw_csum_obj

    def CsumDeParserConfigGenerate(self, deparser, hv_fld_slots, dpp_json):
        '''
            Configure HdrFldStart,End and also generate JSON config output.
        '''
        hw_csum_obj = self.CsumDeParserHwConfigObjs(deparser, hv_fld_slots)
        hw_csum_obj += self.L2CompleteCsumDeParserHwConfigObjs(deparser, hv_fld_slots)

        #Before generating HW config, sort based on StartFld Value.
        dpr_hw_csum_obj = sorted(hw_csum_obj, key=lambda obj: obj[0].HdrFldStartGet())
        if deparser.d == xgress.INGRESS:
            self.dpr_hw_csum_obj = dpr_hw_csum_obj
        if deparser.d == xgress.EGRESS:
            self.eg_dpr_hw_csum_obj = dpr_hw_csum_obj

        #Generate ASIC Config
        if deparser.asic == "capri":
            self.CsumDeParserConfigGenerateCapri(dpr_hw_csum_obj, deparser, dpp_json)
        elif deparser.asic == "elba":
            self.CsumDeParserConfigGenerateElba(dpr_hw_csum_obj, deparser, dpp_json)
        #Json is dumped in the caller to cfg-file.

    def CsumDeParserConfigGenerateCapri(self, dpr_hw_csum_obj, deparser, dpp_json):
        csum_hdr_index = 0
        _calfldobj = None
        for _, is_phdr, _calfldobj in dpr_hw_csum_obj:
            if not is_phdr:
                _csum_obj = _calfldobj.DeParserCsumObjGet()
                _csum_profile_obj = _calfldobj.DeParserCsumProfileObjGet()
                csum_hdr_cfg_name = 'cap_dppcsum_csr_cfg_csum_hdrs[%d]' %\
                                        (csum_hdr_index)
                csum_hdr_index += 1
                _csum_obj.ConfigGenerate(dpp_json['cap_dpp']\
                                        ['registers'][csum_hdr_cfg_name])
                csum_profile_cfg_name = 'cap_dppcsum_csr_cfg_csum_profile[%d]' %\
                                           _csum_profile_obj.CsumProfileNumGet()
                _csum_profile_obj.ConfigGenerate(dpp_json['cap_dpp']\
                                           ['registers'][csum_profile_cfg_name])
            if is_phdr:
                _phdr_csum_obj = _calfldobj.DeParserPhdrCsumObjGet()
                _phdr_profile_obj = _calfldobj.DeParserPhdrProfileObjGet()
                csum_hdr_cfg_name = 'cap_dppcsum_csr_cfg_csum_hdrs[%d]' %\
                                    (csum_hdr_index)
                csum_hdr_index += 1
                _phdr_csum_obj.ConfigGenerate(dpp_json['cap_dpp']\
                                    ['registers'][csum_hdr_cfg_name])
                phdr_cfg_name = 'cap_dppcsum_csr_cfg_csum_phdr_profile[%d]' %\
                                     _phdr_csum_obj.PhdrProfileNumGet()
                _phdr_profile_obj.ConfigGenerate(dpp_json['cap_dpp']\
                                             ['registers'][phdr_cfg_name], _calfldobj.phdr_fields)

        #Deparser expects unused Csum Hdr Slots to be programmed with start fld
        #in increasing order.
        if _calfldobj:
            last_end_fld = _calfldobj.DeParserCsumObjGet().HdrFldEndGet()
            for unfilled_index in  range(csum_hdr_index, deparser.be.hw_model['deparser']['max_csum_hdrs']):
                csum_hdr_cfg_name = 'cap_dppcsum_csr_cfg_csum_hdrs[%d]' % unfilled_index
                dpp_json['cap_dpp']['registers'][csum_hdr_cfg_name]['hdrfld_start']['value'] = str(last_end_fld + 1)
                dpp_json['cap_dpp']['registers'][csum_hdr_cfg_name]['hdrfld_end']['value'] = str(last_end_fld + 2)
                dpp_json['cap_dpp']['registers'][csum_hdr_cfg_name]['_modified'] = True
                last_end_fld += 1

    def CsumDeParserConfigGenerateElba(self, dpr_hw_csum_obj, deparser, dpp_json):
        csum_hdr_index = 0
        _calfldobj = None
        for _, is_phdr, _calfldobj in dpr_hw_csum_obj:
            if not is_phdr:
                _csum_obj = _calfldobj.DeParserCsumObjGet()
                _csum_profile_obj = _calfldobj.DeParserCsumProfileObjGet()
                csum_hdr_cfg_name = 'elb_dppcsum_csr_cfg_csum_hdrs[%d]' %\
                                        (csum_hdr_index)
                csum_hdr_index += 1
                _csum_obj.ConfigGenerate(dpp_json['elb_dpp']\
                                        ['registers'][csum_hdr_cfg_name])
                csum_profile_cfg_name = 'elb_dppcsum_csr_cfg_csum_profile[%d]' %\
                                           _csum_profile_obj.CsumProfileNumGet()
                _csum_profile_obj.ConfigGenerate(dpp_json['elb_dpp']\
                                           ['registers'][csum_profile_cfg_name])
            if is_phdr:
                _phdr_csum_obj = _calfldobj.DeParserPhdrCsumObjGet()
                _phdr_profile_obj = _calfldobj.DeParserPhdrProfileObjGet()
                csum_hdr_cfg_name = 'elb_dppcsum_csr_cfg_csum_hdrs[%d]' %\
                                    (csum_hdr_index)
                csum_hdr_index += 1
                _phdr_csum_obj.ConfigGenerate(dpp_json['elb_dpp']\
                                    ['registers'][csum_hdr_cfg_name])
                phdr_cfg_name = 'elb_dppcsum_csr_cfg_csum_phdr_profile[%d]' %\
                                     _phdr_csum_obj.PhdrProfileNumGet()
                _phdr_profile_obj.ConfigGenerate(dpp_json['elb_dpp']\
                                             ['registers'][phdr_cfg_name], _calfldobj.phdr_fields)

        #Deparser expects unused Csum Hdr Slots to be programmed with start fld
        #in increasing order.
        if _calfldobj:
            last_end_fld = _calfldobj.DeParserCsumObjGet().HdrFldEndGet()
            for unfilled_index in  range(csum_hdr_index, deparser.be.hw_model['deparser']['max_csum_hdrs']):
                csum_hdr_cfg_name = 'elb_dppcsum_csr_cfg_csum_hdrs[%d]' % unfilled_index
                dpp_json['elb_dpp']['registers'][csum_hdr_cfg_name]['hdrfld_start']['value'] = str(last_end_fld + 1)
                dpp_json['elb_dpp']['registers'][csum_hdr_cfg_name]['hdrfld_end']['value'] = str(last_end_fld + 2)
                dpp_json['elb_dpp']['registers'][csum_hdr_cfg_name]['_modified'] = True
                last_end_fld += 1

    def AllocateDeParserGsoCsumResources(self, parser):
        #Doesn't need csum unit.
        #Need to use one of first 4 HV bit
        #Need to use one of first 4 HdrFld Info slots
        hv_location = self.be.hw_model['parser']['hv_location']
        max_hv_bits = self.be.hw_model['parser']['max_hv_bits']

        if parser.d == xgress.INGRESS:
            gso_cal_fieldlist_update = self.gso_cal_fieldlist_update
        else:
            gso_cal_fieldlist_update = self.eg_gso_cal_fieldlist_update

        for calfldobj in gso_cal_fieldlist_update:
            calfldhdr = calfldobj.GsoCalculatedFieldHdrGet()
            hf_name = calfldhdr + '.gso'
            cf = self.be.pa.get_field(hf_name, parser.d)
            ncc_assert(cf and cf.is_hv)
            calfldobj.hdr_valid = (hv_location + max_hv_bits - 1) - cf.phv_bit
            calfldobj.hdrfld_slot = calfldobj.hdr_valid
            cf = self.be.pa.get_field(calfldobj.gso_csum_result_fld_name, parser.d)
            ncc_assert(cf != None)
            calfldobj.gso_csum_result_phv = cf.phv_bit
            calfldobj.csum_field_ohi_slot = \
                parser.get_ohi_slot_wr_only_field_name(calfldobj.csum_hfield_name.split(".")[1])
            ncc_assert(calfldobj.csum_field_ohi_slot != None)

    def GsoCsumDeParserConfigGenerate(self, deparser, dpp_json, dpr_json):
        if deparser.d == xgress.INGRESS:
            gso_cal_fieldlist_update = self.gso_cal_fieldlist_update
        else:
            gso_cal_fieldlist_update = self.eg_gso_cal_fieldlist_update

        for calfldobj in gso_cal_fieldlist_update:
            if deparser.asic == "capri":
                calfldobj.GsoConfigGenerateCapri(dpp_json, dpr_json)
            elif deparser.asic == "elba":
                calfldobj.GsoConfigGenerateElba(dpp_json, dpr_json)

    def AllocateL2CompleteCsumDeParserResources(self, parser):
        '''
            Assign csum unit, profiles for L2 complete csum calculation
        '''
        l2_csum_cal_fieldlist_update = self.l2_csum_cal_fieldlist_update if parser.d == xgress.INGRESS \
                                              else self.eg_l2_csum_cal_fieldlist_update
        DeParserCsumEngineObj = self.IngDeParserCsumEngineObj if parser.d == xgress.INGRESS \
                                              else self.EgDeParserCsumEngineObj 
        for calfldobj in l2_csum_cal_fieldlist_update:
            calfldhdr = calfldobj.CalculatedFieldHdrGet()
            _csum_obj = calfldobj.DeParserCsumObjGet()
            l2hdr_name  = calfldobj.CsumPhdrNameGet()

            if l2hdr_name == '':
                hf_name = calfldhdr + '.l2csum'
                hdr_inst = self.be.h.p4_header_instances[calfldhdr]
            else:
                hf_name = l2hdr_name + '.l2csum'
                hdr_inst = self.be.h.p4_header_instances[l2hdr_name]

            _csum_obj.HvBitNumSet(parser.hdr_hv_bit[hdr_inst])
            csumhv = -1
            for elem in parser.csum_hdr_hv_bit[hdr_inst]:
                if elem[2] == hf_name:
                    csumhv = elem[0]
                    break
            ncc_assert(csumhv != -1)
            _csum_obj.CsumHvBitNumSet(csumhv)
            _csum_obj.CsumHvBitStrSet(hf_name)
            _csum_obj.CsumUnitNumSet(DeParserCsumEngineObj.\
                                        AllocateCsumUnit(calfldhdr, calfldobj, True))
            _csum_obj.PhdrValidSet(0)
            csum_profile = DeParserCsumEngineObj.\
                                     AllocateCsumProfile(hf_name)
            #Config Csum Profile
            _csum_obj.CsumProfileNumSet(csum_profile)
            _csum_profile_obj = calfldobj.DeParserCsumProfileObjGet()
            _csum_profile_obj.CsumProfileNumSet(csum_profile)

            if l2hdr_name == '':
                #case : Header/Container of l2 csum result field
                _csum_obj.CsumCopyVldSet(1)
                locadj = -1
                for p4f in hdr_inst.fields:
                    if p4f.name == calfldobj.dstField.split(".")[1]:
                        locadj = p4f.offset / 8
                        break
                ncc_assert(locadj != -1)
                _csum_profile_obj.CsumProfileCsumLocSet(locadj)
                _csum_profile_obj.CsumProfilePhvLenSelSet(0, 0)
            else:
                #case: L3 Header or L2Payload Header
                dprsr_payload_len_slot = \
                            self.DeParserPayLoadLenSlotGet(calfldobj, parser)
                _csum_profile_obj.CsumProfilePhvLenSelSet(1, dprsr_payload_len_slot)
                #l2 csum result goes in internal header not in packet.Set no-rewrite
                _csum_profile_obj.CsumProfileNoCsumRewriteSet(1)
                _csum_profile_obj.CsumProfileCsumLocSet(0)

        DeParserCsumEngineObj.CsumUnitAllocationFinalize()

        covered_csum_units = set()
        update_cal_fieldlist = self.update_cal_fieldlist if parser.d == xgress.INGRESS \
                                              else self.eg_update_cal_fieldlist
        for calfldobj in update_cal_fieldlist:
            covered_csum_units.add(calfldobj.DeParserCsumObjGet().CsumUnitNumGet())
        for calfldobj in l2_csum_cal_fieldlist_update:
            l2csum_unit = calfldobj.DeParserCsumObjGet().CsumUnitNumGet()
            for cunit in covered_csum_units:
                if cunit != l2csum_unit:
                    calfldobj.DeParserCsumObjGet().CsumInnerCsumResultInclude(cunit)

    def L2CompleteCsumDeParserHwConfigObjs(self, deparser, hv_fld_slots):
        hw_csum_obj = [] # list of csumobj that need to be programmed in HW
                         # without repeatation and maintaining Banyan contrainst.
        l2_csum_cal_fieldlist_update = self.l2_csum_cal_fieldlist_update if deparser.d == xgress.INGRESS \
                                              else self.eg_l2_csum_cal_fieldlist_update
        for calfldobj in l2_csum_cal_fieldlist_update:
            csum_hdr = calfldobj.CalculatedFieldHdrGet()
            phdr_name = calfldobj.phdr_name
            _csum_obj = calfldobj.DeParserCsumObjGet()
            _csum_profile_obj = calfldobj.DeParserCsumProfileObjGet()
            ncc_assert(_csum_obj != None)
            ncc_assert(_csum_profile_obj != None)
            ncc_assert(_csum_obj.hv != -1)
            ncc_assert(_csum_obj.csum_hv != -1)
            fldstart, fldend, _ = hv_fld_slots[_csum_obj.csum_hv]
            _csum_obj.HdrFldStartEndSet(fldstart,fldend)

            #Generate Logical Output
            calfldobj.DeParserCsumObjAddLog(_csum_obj.LogGenerate(csum_hdr + " , " + phdr_name))
            calfldobj.DeParserCsumObjAddLog(_csum_profile_obj.LogGenerate())
            hw_csum_obj.append((_csum_obj, False, calfldobj))

        return  hw_csum_obj


    def CsumLogicalOutputCreate(self):
        '''
            Generates csum.out file containing configuration pushed to HW.
        '''
        out_dir = self.be.args.gen_dir + '/%s/logs' % (self.be.prog_name)
        if not os.path.exists(out_dir):
            try:
                os.makedirs(out_dir)
            except OSError as e:
                if e.errno != errno.EEXIST:
                    raise

        for d in xgress:
            if d == xgress.INGRESS:
                verify_cal_fieldlist        = self.verify_cal_fieldlist
                gso_cal_fieldlist_compute   = self.gso_cal_fieldlist_compute
                gso_cal_fieldlist_update    = self.gso_cal_fieldlist_update
                update_cal_fieldlist        = self.update_cal_fieldlist
                dpr_hw_csum_obj             = self.dpr_hw_csum_obj
                l2_csum_cal_fieldlist_update= self.l2_csum_cal_fieldlist_update
            else:
                verify_cal_fieldlist        = self.eg_verify_cal_fieldlist
                gso_cal_fieldlist_compute   = self.eg_gso_cal_fieldlist_compute
                update_cal_fieldlist        = self.eg_update_cal_fieldlist
                dpr_hw_csum_obj             = self.eg_dpr_hw_csum_obj
                gso_cal_fieldlist_update    = self.eg_gso_cal_fieldlist_update
                l2_csum_cal_fieldlist_update= self.eg_l2_csum_cal_fieldlist_update

            ofile = open('%s/csum_%s.out' % (out_dir, d.name), "w")
            ofile.write("   Ohi Slot Allocation\n")
            ofile.write("   ~~~~~~~~~~~~~~~~~~~\n\n")
            for log_str in self.CalFldListLogStrTableGet():
                ofile.write(log_str)

            if len(verify_cal_fieldlist):
                ofile.write("Checksum Verification Config in parser\n")
                ofile.write("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n\n")
                for calfldobj in verify_cal_fieldlist:
                    for log_str in calfldobj.ParserCsumObjLogStrTableGet():
                        ofile.write(log_str)

                for calfldobj in gso_cal_fieldlist_compute:
                    for log_str in calfldobj.GsoCsumObjLogStrTableGet():
                        ofile.write(log_str)

            if len(gso_cal_fieldlist_update) or len(l2_csum_cal_fieldlist_update) \
                or len(update_cal_fieldlist):
                ofile.write("Checksum Config in Deparser\n")
                ofile.write("~~~~~~~~~~~~~~~~~~~~~~~~~~\n\n")

                if gso_cal_fieldlist_update:
                    ofile.write("       Gso Config \n")
                    ofile.write("       ~~~~~~~~~~\n\n")
                    for calfldobj in gso_cal_fieldlist_update:
                        for log_str in calfldobj.DeParserGsoCsumObjLogStrTableGet():
                            ofile.write(log_str)
                    ofile.write("\n\n")

                if l2_csum_cal_fieldlist_update:
                    ofile.write("       L2 Complete Csum Config \n")
                    ofile.write("       ~~~~~~~~~~~~~~~~~~~~~~~~\n\n")
                    for calfldobj in l2_csum_cal_fieldlist_update:
                        for log_str in calfldobj.DeParserCsumObjLogStrTableGet():
                            ofile.write(log_str)
                ofile.write("\n\n")

                if len(update_cal_fieldlist):
                    ofile.write("  Checksum Compute Config in Deparser\n")
                    ofile.write("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n\n")
                    for calfldobj in update_cal_fieldlist:
                        for log_str in calfldobj.DeParserCsumObjLogStrTableGet():
                            ofile.write(log_str)

                    ofile.write("Summary: Checksum Compute Config in Deparser\n")
                    ofile.write("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n\n")


                    pstr = '{:32s}{:5s}{:7s}{:7s}{:7s}{:8s}{:6s}{:5s}{:8s}{:7s}{:7s}{:6s}{:5s}'\
                           '{:5s}{:5s}{:5s}\n'.format("Hdr", "csum#", "  csum", "  csum", "  HV", \
                                                           "Csum", "Phdr", "Phdr", "Phdr",\
                                                           "HdrFld", "HdrFld", "Csum", "use",\
                                                           "phv", "loc", "add")
                    pstr += '{:32s}{:5s}{:7s}{:7s}{:7s}{:8s}{:6s}{:5s}{:8s}{:7s}{:7s}{:6s}{:5s}'\
                           '{:5s}{:5s}{:5s}\n'.format("   ", "    ", "  Hv", "Flit", "   ", \
                                                           "Profile", "Valid", "Unit", "Profile",\
                                                           "Start", "End", "incl", "phv",\
                                                           "len", "adj", "len")
                    pstr += '{:32s}{:5s}{:7s}{:7s}{:7s}{:8s}{:6s}{:5s}{:8s}{:7s}{:7s}{:6s}{:5s}'\
                           '{:5s}{:5s}{:5s}\n'.format("   ", "    ", " ", "bit", " ", \
                                                           "       ", "     ", "    ", "       ",\
                                                           "     ", "   ", "BM", "len",\
                                                           "sel", "   ", "   ")
                    pstr += '{:32s}{:5s}{:7s}{:7s}{:7s}{:8s}{:6s}{:5s}{:8s}{:7s}{:7s}{:6s}{:5s}'\
                           '{:5s}{:5s}{:5s}\n'.format("   ", "    ", " ", " ", " ", \
                                                           "       ", "     ", "    ", "       ",\
                                                           "     ", "   ", "  ", "   ",\
                                                           "   ", "   ", "   ")
                    ofile.write(pstr)
                    for _, is_phdr, _calfldobj in dpr_hw_csum_obj:
                        ofile.write(_calfldobj.DeparserCsumConfigMatrixRowLog(is_phdr))
                    ofile.write("\n\n")


            ofile.close()
