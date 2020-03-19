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
import p4_hlir.hlir.table_dependency as table_dependency
from p4_hlir.graphs.dependency_graph import *
import p4_hlir.hlir.analysis_utils as hlir_utils

from capri_logging import ncc_assert as ncc_assert
from capri_utils import *
from capri_model import capri_model as capri_model
from capri_pa import capri_field as capri_field
from capri_output import capri_deparser_logical_output as capri_deparser_logical_output
from capri_output import capri_deparser_cfg_output as capri_deparser_cfg_output
from capri_output import elba_deparser_cfg_output as elba_deparser_cfg_output

class capri_deparser:
    def __init__(self, capri_be, d):
        self.d    = d
        self.asic = capri_be.asic
        self.be   = capri_be
        self.field_type_none = 0
        self.field_type_phv = 1
        self.field_type_ohi = 2
        self.logger = logging.getLogger('DP')
        # Topological Order of PHV chunks and OHI of a header. Key = header
        self.topo_ordered_phv_ohi_chunks = OrderedDict()

        #self.hv_fld_slots = {} # Key = HVbit, Value = (fld_start, fld_end)

    def initialize(self):
        pass

    def generate_output(self):
        capri_deparser_logical_output(self)
        if (self.asic == 'capri'):
            capri_deparser_cfg_output(self)#, self.hv_fld_slots)
        elif (self.asic == 'elba'):
            elba_deparser_cfg_output(self)#, self.hv_fld_slots)

    def build_field_dictionary(self):
        headers = self.be.parsers[self.d].headers
        for hdr in headers:
            current_field_type = self.field_type_none
            dp_hdr_fields = [] # (phv_chunk|ohi, type=field_type_phv|field_type_ohi, cf)
            cfields = []
            ohi_fields = []
            if hdr in self.be.parsers[self.d].ohi:
                ohi_list = list(self.be.parsers[self.d].ohi[hdr])
            else:
                ohi_list = []

            hdr_flds = self.be.pa.get_header_all_cfields(hdr, self.d)
            phv_chunks = self.be.pa.get_hdr_phv_chunks(hdr_flds, self.d, check_ohi=True)

            #check for byte alignment
            for pc in phv_chunks:
                if pc[0] % 8:
                    ncc_assert(0)
                if (pc[1] % 8):
                    ncc_assert(0)

            cur_offset = 0

            for f in hdr.fields:
                cf = self.be.pa.gress_pa[self.d].get_field(get_hfname(f))
                ncc_assert(cf, "unknown field %s" % (hdr.name + f.name))
                field_byte_offset = cf.get_field_offset() / 8
                if field_byte_offset < cur_offset:
                    continue
                if cf.is_ohi:
                    ohi = ohi_list.pop(0)
                    ncc_assert(field_byte_offset == ohi.start)
                    dp_hdr_fields.append((ohi, self.field_type_ohi, cf))
                    if (isinstance(ohi.length, int)):
                        cur_offset += ohi.length
                else:
                    pc = phv_chunks.pop(0)
                    ncc_assert(pc[0] == cf.phv_bit)
                    dp_hdr_fields.append((pc, self.field_type_phv, cf))
                    cur_offset += pc[1]/8
            self.topo_ordered_phv_ohi_chunks[hdr] = dp_hdr_fields

        # Some metadata like capri_i2e_metadata may or may not be present. Check for presence
        if self.be.pa.gress_pa[self.d].i2e_hdr:
            phv_chunks = self.be.pa.get_hdr_phv_chunks(self.be.pa.gress_pa[self.d].i2e_fields,
                            self.d, check_ohi=False)
            #check for byte alignment
            for pc in phv_chunks:
                if pc[0] % 8:
                    ncc_assert(0)
                if (pc[1] % 8):
                    ncc_assert(0)
            phvchunklist = []
            for chunks in phv_chunks:
                phvchunklist.append((chunks, self.field_type_phv, None))
            self.topo_ordered_phv_ohi_chunks[self.be.pa.gress_pa[self.d].i2e_hdr] = phvchunklist

    def print_deparser_info(self):
        # For now get all info from parser and pa
        num_headers = len(self.be.parsers[self.d].headers)
        num_flds = 0
        self.logger.debug('%s,Hdr,#PhvChunks,#Ohi' % (self.d.name))
        for hdr, phv_ohi_chunks in self.topo_ordered_phv_ohi_chunks.items():
            num_phv = 0
            num_ohi = 0
            for (phv_ohi_list, chunk_type, _) in phv_ohi_chunks:
                if (chunk_type == self.field_type_phv):
                    num_phv += len(phv_ohi_list)
                if (chunk_type == self.field_type_ohi):
                    num_ohi += 1
            num_flds += (num_phv + num_ohi)
            self.logger.debug('%s,%s,%d,%d' %(self.d.name, hdr.name, num_phv, num_ohi))

        self.logger.info('%s:Deparser Total Headers %d Total Fields %d\n' % \
            (self.d.name, num_headers, num_flds))

        # Details
        for hdr, phv_ohi_chunks in self.topo_ordered_phv_ohi_chunks.items():
            for (phv_ohi, chunk_type, _) in phv_ohi_chunks:
                if (chunk_type == self.field_type_phv):
                    self.logger.debug('%s:%s Phv ChunkStart %d ChunkLen %d' % (self.d.name, hdr.name, phv_ohi[0], phv_ohi[1]))
                if (chunk_type == self.field_type_ohi):
                        self.logger.debug('%s:%s Ohi %s' % (self.d.name, hdr.name, phv_ohi))
