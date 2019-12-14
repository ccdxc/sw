#!/usr/bin/python
#
# Capri-Non-Compiler-Compiler (capri-ncc)
# Parag Bhide (Pensando Systems)

import os
import sys
import pdb
import json
import gc
import logging
import copy
from collections import OrderedDict
from enum import IntEnum

from p4_hlir.main import HLIR
import p4_hlir.hlir.p4 as p4
import p4_hlir.hlir.table_dependency as table_dependency
from p4_hlir.graphs.dependency_graph import *
import p4_hlir.hlir.analysis_utils as hlir_utils

import capri_logging
from capri_utils import *
from capri_model import capri_model as capri_model
from capri_pa import capri_field as capri_field
from capri_output import capri_asm_output_table as capri_asm_output_table
from capri_output import capri_te_cfg_output as capri_te_cfg_output
from capri_output import capri_p4_table_spec_output as capri_p4_table_spec_output
from capri_output import capri_pic_csr_load as capri_pic_csr_load
from capri_output import capri_pic_csr_output as capri_pic_csr_output
from capri_output import capri_table_memory_spec_load as capri_table_memory_spec_load
from capri_output import capri_p4_table_map_output as capri_p4_table_map_output
from capri_output import capri_dump_table_memory as capri_dump_table_memory

def get_logical_stages(graph):
    if not graph.root:
        # empty control flow
        return []
    has_cycle, sorted_list = graph.topo_sorting()
    ncc_assert(not has_cycle)
    nb_stages = 0
    stage_list = []
    stage_dependencies_list = []
    for table in sorted_list:
        d_type_ = 0
        i = nb_stages - 1
        while i >= 0:
            stage = stage_list[i]
            stage_dependencies = stage_dependencies_list[i]
            if table in stage_dependencies:
                d_type_ = stage_dependencies[table]
                ncc_assert(d_type_ > 0)
                break
            else:
                i -= 1
        if d_type_ == 0:
            i += 1
        elif d_type_ >= Dependency.ACTION:
            i += 1
        if i == nb_stages:
            stage_list.append([])
            stage_dependencies_list.append(defaultdict(int))
            nb_stages += 1

        stage = stage_list[i]
        stage_dependencies = stage_dependencies_list[i]
        stage.append(table)
        for node_to, edge in table.edges.items():
            type_ = edge.type_
            if type_ > 0 and type_ > stage_dependencies[node_to]:
                stage_dependencies[node_to] = type_

    stage_table_list = [[] for _ in stage_list]
    fixed_stage_tables = OrderedDict() #[[] for _ in stage_list]
    for i, table_list in enumerate(stage_list):
        for n in table_list:
            if isinstance(n.p4_node, p4.p4_table) and \
                n.p4_node._parsed_pragmas and 'stage' in n.p4_node._parsed_pragmas:
                stg = int(n.p4_node._parsed_pragmas['stage'].keys()[0])
                if stg in fixed_stage_tables:
                    fixed_stage_tables[stg].append(n.p4_node)
                else:
                    fixed_stage_tables[stg] = [n.p4_node]
            else:
                stage_table_list[i].append(n.p4_node)


    # remove empty stages (i.e. stages that are empty only has condition node)
    # split stages when too many tables in a given stage (done by user?? using stage pragma)
    cond_tables = []
    fixed_stage_table_list = []
    for i, table_list in enumerate(stage_table_list):
        table_present = False
        for n in table_list:
            if isinstance(n, p4.p4_table):
                table_present = True
                break
        if not table_present:
            cond_tables += table_list
        else:
            for c in cond_tables:
                if c not in table_list:
                    table_list.append(c)
            fixed_stage_table_list.append(table_list)
            cond_tables = []

    if len(cond_tables):
        fixed_stage_table_list.append(cond_tables)

    # re-insert the fixed tables
    stages_used = len(fixed_stage_table_list)
    if len(fixed_stage_tables):
        stages_needed = max(fixed_stage_tables.keys()) + 1
    else:
        stages_needed = stages_used

    while stages_used < stages_needed:
        fixed_stage_table_list.append([])
        stages_used += 1

    for k in range(stages_needed):
        if k in fixed_stage_tables:
            fixed_stage_table_list[k] += fixed_stage_tables[k]
        else:
            fixed_stage_table_list[k] += []

    return fixed_stage_table_list

def table_topo_sort(root_node):
    # NOT used... TBD
    def _visit(node, marker, sorted_list):
        if not node:
            return False
        if node in marker:
            if marker[node] == 1:
                return True #cycles
            if marker[node] == 2:
                return False
        else:
            marker[node] = 1
            for nxt_node in node.dependencies_for.keys():
                if _visit(nxt_node, marker, sorted_list):
                    return True
            marker[node] = 2
            sorted_list.insert(0, node)

    marker = OrderedDict()
    sorted_list = []
    has_cycles = _visit(root_node, marker, sorted_list)
    return has_cycles, sorted_list

def _remove_duplicate_chunks(dst_chunks, src_chunks):
    # dst is checked against src and duplicate are removed from dst, src is un-changed
    # assumptions - chunks are byte aligned and are in order
    # XXX this can create large (upto 1K) entry valid bytes array for filt 8... optimize later
    last_byte = max(dst_chunks[-1][0]+dst_chunks[-1][1], src_chunks[-1][0]+src_chunks[-1][1]) / 8
    valid_bytes = [False for _ in range(last_byte)]
    for cs,cw in dst_chunks:
        while cw:
            valid_bytes[cs/8] = True
            cs += 8
            cw -= 8

    for cs, cw in src_chunks:
        while cw:
            ncc_assert(cw > 0)
            # remove src bytes from dst
            valid_bytes[cs/8] = False
            cs += 8
            cw -= 8
    cs = 0
    cw = 0
    new_chunks = []
    for i,v in enumerate(valid_bytes):
        if cw:
            if v:
                cw += 8
            else:
                new_chunks.append((cs,cw))
                cw = 0
        elif v:
            cs = i*8
            cw = 8
    if cw:
        new_chunks.append((cs,cw))

    return new_chunks

def _phv_chunks_to_bytes(phv_chunks):
    # phv chunks are expected to be byte aligned
    # chunk is expressed using (start_bit, num_bits)
    byte_sel = []
    for b, w in phv_chunks:
        while w:
            ncc_assert(w > 0)
            byte_sel.append(b/8)
            b += 8
            w -= 8
    return byte_sel

class capri_km_flit:
    def __init__(self, ct):
        self.ct = ct    # associated capri table
        self.k_phv_chunks = []
        self.i_phv_chunks = []
        self.i_bit_ext = []
        self.k_bit_ext = []
        self.i1_phv_chunks = []
        self.i2_phv_chunks = []
        self.i2k_pad_chunks = []    # i_bits converted to k
        self.km_profile = capri_km_profile(self.ct.gtm)


    def flit_create_km_profile(self):
        i1_bytes = _phv_chunks_to_bytes(self.i1_phv_chunks)
        self.km_profile.byte_sel += i1_bytes
        self.km_profile.i1_byte_sel += i1_bytes

        k_bytes = _phv_chunks_to_bytes(self.k_phv_chunks)
        self.km_profile.byte_sel += k_bytes
        self.km_profile.k_byte_sel += k_bytes

        i2_bytes = _phv_chunks_to_bytes(self.i2_phv_chunks)
        self.km_profile.byte_sel += i2_bytes
        self.km_profile.i2_byte_sel += i2_bytes

        for k_bit, kw in self.k_bit_ext:
            while kw:
                self.km_profile.bit_sel.append(k_bit)
                k_bit += 1
                kw -= 1

        self.km_profile.k_bit_sel += self.km_profile.bit_sel

        for i_bit, w, _ in self.i_bit_ext:
            while w:
                self.km_profile.bit_sel.append(i_bit)
                self.km_profile.i_bit_sel.append(i_bit)
                i_bit += 1
                w -= 1

        # sorted_bytes = sorted(self.km_profile.byte_sel)
        # ncc_assert(sorted_bytes == self.km_profile.byte_sel)

# Table Management
class capri_table:
    def __init__(self, gtm, p4_table, d):
        self.p4_table = p4_table
        self.gtm = gtm
        self.d = d
        self.stage = -1
        self.tbl_id = -1      # within the state (0-15)
        self.thread_tbl_ids = {}    # used for a multi-threaded table
        self.match_type = match_type.NONE    # hash, indexed, tcam
        self.num_entries = -1
        self.is_otcam = False
        self.is_overflow = False
        self.htable_name = None
        self.hash_ct = None         # parent hash table assiciated with this overflow table
        self.otcam_ct = None        # otcam associated with this hash table
        self.collision_ct = None    # collision table associated with this hash table
        self.collision_cf = None    # collision table index
        self.hash_type = 0  # pragma hash_type
        self.is_raw = False # Raw table (used in p4_plus)
        self.is_raw_index = False # Raw index table (used in p4_plus) - slightly different from raw
        self.is_hbm = False # If table resides in HBM, it will be set to true.
                            # A pragma is used in P4 to qualify the table.
        self.is_writeback = False # True when MPU writes to table entry - need lock
        self.is_memory_only = False
        self.is_wide_key = False
        self.is_policer = False
        self.policer_colors = 0 # 2-color/3-color
        self.is_rate_limit_en = False
        self.token_refresh_profile = 0 # Token refresh profile id for policer/rate-limit
        self.token_refresh_rate = 4000 #Token refresh rate, 4,000 times/sec
        self.is_multi_threaded = False
        self.num_threads = 1
        # flit numbers from which key(K+I) is built, Lookup can be
        # launched from the last flit when all the info is avaialble
        self.flits_used = []     # flits that provide K and I values
        num_flits = gtm.tm.be.hw_model['phv']['num_flits']
        self.km_flits = [capri_km_flit(self) for f in range(num_flits)]

        self.launch_flit = -1
        self.key_makers = []

        self.keys = []  # (hdr/capri_field, match_type, mask) => k{}
        self.input_fields = [] # combined list of input fields used by all actions => i{}
        self.toeplitz_key_cfs = []
        self.toeplitz_seed_cfs = []
        # action data => d{} is union of all action data
        self.action_data = OrderedDict()  # {action_name : [(name, size)]}
        self.action_output = [] # not kept per action.. so for no use for this - TBD
        self.meta_fields = [] # all metadata fields used as key and input
        self.key_size = -1
        self.key_phv_size = -1  # can be less than key_size due to unions
        self.final_key_size = -1 # final size may be different when km_profiles makers are shared
        self.i_size = -1
        self.i_phv_size = -1    # can be less than key_size due to unions
        self.d_size = -1

        # hardware information
        self.k_bit_ext = []
        self.k_phv_chunks = []
        self.i_bit_ext = []
        self.i1_phv_chunks = []
        self.i2_phv_chunks = []
        self.i2k_pad_chunks = []    # i_bits converted to k
        self.i_in_key = 0
        self.num_km = 0 # key makers needed
        # typically all flit_profiles will point to the same profile
        # keep separate per flit profiles so that profiles can be modified/shared only
        # in a givne flit (useful for loading new dis-joint data into km while waiting
        # to launch a lookup
        self.combined_profile = None
        self.oo_flits = []    # flits that need separate profiles - XXX not used

        self.start_key_off = -1         # computed based on 1st K byte location in KM
        self.start_key_off_delta = 0    # bit offset within 1st K byte
        self.end_key_off = -1           # computed based on last K bit location in KM
        self.end_key_off_delta = 0
        self.last_flit_start_key_off = -1   # for wide key and toeplitz has tables
        self.last_flit_end_key_off = -1     # for wide key and toeplitz has tables
        self.le_action_params = {}       #Collection of action paramerters that
                                            #are in little endian format.
        self.k_plus_d = {} #Dictionary of K+D fields for each table's action.
        self.action_appfields = {} # SubList of action parameters used by flow table
        self.action_hwfields = {} # List of action parameters that have get/set c-api

    def is_hash_table(self):
        return True if (self.match_type == match_type.EXACT_HASH_OTCAM or \
                        self.match_type == match_type.EXACT_HASH) else False
    def is_index_table(self):
        return True if self.match_type == match_type.EXACT_IDX else False

    def is_tcam_table(self):
        return True if self.match_type == match_type.TERNARY else False

    def is_mpu_only(self):
        return True if self.match_type == match_type.MPU_ONLY else False

    def is_toeplitz_hash(self):
        return self.hash_type == 4

    def num_actions(self):
        return len(self.action_data)
    
    def ct_compute_phv_size(self, cflist):
        # compute the phv size based on phv bits to handle any unions/overlaps
        # sorted_cflist = sorted(cflist, key=lambda k: k.phv_bit) - by caller
        phv_off = -1
        ksize = 0
        for cf in cflist:
            if phv_off <= cf.phv_bit:
                ksize += (cf.width - cf.pad)
                phv_off = cf.phv_bit + cf.width
            elif phv_off < (cf.phv_bit + cf.width):
                new_bits = (cf.phv_bit + cf.width) - phv_off
                ksize += new_bits
                phv_off = cf.phv_bit + cf.width - cf.pad
            else:
                pass
        if len(cflist):
            ncc_assert(ksize, 'Invalid key size computed for %s' % self.p4_table.name)
        return ksize


    def ct_print_table_parameters(self, summary=True):
        n = self.num_entries if self.num_entries else 1
        kd_size = self.d_size
        if self.is_hash_table():
            kd_size += self.final_key_size
        if self.is_tcam_table():
            sram_size = kd_size * n
            tcam_size = self.final_key_size * n
        else:
            sram_size = kd_size * n
            tcam_size = 0

        match_type_name = self.match_type.name if not self.is_raw else "RAW_TABLE"

        pstr = '%s,%d,%s,%d,%s,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d' % \
            (self.d.name, self.stage, self.p4_table.name, self.tbl_id, match_type_name, n,
             self.final_key_size, self.final_key_size+self.i_phv_size, self.d_size,
             kd_size, sram_size, tcam_size,
             sum([i[1] for i in self.i1_phv_chunks]),
             sum([i[1] for i in self.i2_phv_chunks]),
             self.i_in_key
             )
        return pstr

    def ct_print_table_ki(self):
        self.gtm.tm.logger.debug("Table K-I Info(per-flit): %s" % self.p4_table.name)
        for k,kf in enumerate(self.km_flits):
            if len(kf.k_phv_chunks):
                self.gtm.tm.logger.debug("k_phvs[%d]: %s" % (k, kf.k_phv_chunks))
            if len(kf.i1_phv_chunks):
                self.gtm.tm.logger.debug("i1_phvs[%d]: %s" % (k, kf.i1_phv_chunks))
            if len(kf.i2_phv_chunks):
                self.gtm.tm.logger.debug("i2_phvs[%d]: %s" % (k, kf.i2_phv_chunks))
            if len(kf.k_bit_ext):
                self.gtm.tm.logger.debug("k_bits[%d]: %s" % (k, kf.k_bit_ext))
            if len(kf.i_bit_ext):
                self.gtm.tm.logger.debug("i_bits[%d]: %s" % (k, kf.i_bit_ext))

        self.gtm.tm.logger.debug("keys: %s" % self.keys)
        self.gtm.tm.logger.debug("input: %s" % self.input_fields)

    def ct_validate_table_config(self):
        if self.is_overflow and not self.is_otcam:
            return

        max_ki_size = self.gtm.tm.be.hw_model['match_action']['max_ki_size']
        max_kd_size = self.gtm.tm.be.hw_model['match_action']['max_kd_size']
        violation = False

        if self.is_wide_key:
            return

        if self.is_mpu_only():
            final_key_size = 0
        else:
            final_key_size = self.final_key_size

        if ((final_key_size + self.i_phv_size) > max_ki_size):
            self.gtm.tm.logger.critical('%s:Violation Table %s (K+I) %d\n' % \
                (self.d.name, self.p4_table.name, final_key_size + self.i_phv_size))
            violation = True
        kd_size = self.d_size
        if self.is_hash_table():
            kd_size += final_key_size
            # XXX for HBM mem need to add i1 size

        if (kd_size > max_kd_size):
            # XXX for direct and tcam tables - this may be a different calculation
            self.gtm.tm.logger.critical('%s:Violation Table %s (K+D) %d\n' % \
                (self.d.name, self.p4_table.name, kd_size))
            violation = True

        self.gtm.tm.logger.debug('%s:Keys (%d) = %s\n' % \
            (self.p4_table.name, len(self.keys), self.keys))
        self.gtm.tm.logger.debug('%s:Input flds %s\n' % \
            (self.p4_table.name, self.input_fields))

        total_bit_extractions = 0
        i_bits = sum([k[1] for k in self.i_bit_ext])
        total_bit_extractions += i_bits
        k_bits = sum([k[1] for k in self.k_bit_ext])
        total_bit_extractions += k_bits

        # initial check using logical key and data widths
        num_bit_extractors = self.gtm.tm.be.hw_model['match_action']['num_bit_extractors']
        key_maker_width = self.gtm.tm.be.hw_model['match_action']['key_maker_width']
        num_km = (final_key_size + self.i_phv_size + key_maker_width - 1) / key_maker_width
        num_bit_extractors = num_bit_extractors * num_km
        if total_bit_extractions > num_bit_extractors:
            self.gtm.tm.logger.critical("%s:Violation-%s need %d bit extractors(max %d allowed)" % \
                (self.d.name, self.p4_table.name, total_bit_extractions, num_bit_extractors))
            self.gtm.tm.logger.debug("%s:Violation - \n i_bits(%d) %s\n k_bits(%d) %s:\nK:%s \nI:%s\n" % \
                (self.p4_table.name, i_bits, self.i_bit_ext, k_bits, self.k_bit_ext, self.keys,
                 self.input_fields))
            violation = True

        ncc_assert(violation == False )#"Fix violations and try again"
        if len(self.key_makers) == 0:
            self.gtm.tm.logger.critical("%s:Problem:%s has no km created for it" % \
                    (self.gtm.d.name, self.p4_table.name))
        for _km in self.key_makers:
            km = _km.shared_km if _km.shared_km else _km
            hw_id = km.hw_id
            if hw_id == -1:
                self.gtm.tm.logger.critical("%s:Problem:%s has no km allocated to it" % \
                    (self.gtm.d.name, self.p4_table.name))
            if km.combined_profile:
                km.combined_profile.km_profile_validate()

        # check key_size and offset programming
        new_key_size = self.end_key_off - self.start_key_off
        old_key_size = self.key_phv_size - self.start_key_off_delta - self.end_key_off_delta
        if new_key_size != old_key_size:
            self.gtm.tm.logger.debug("%s:Key size changed due to km sharing %d -> %d" % \
                (self.p4_table.name, old_key_size, new_key_size))
            if self.is_hash_table() or self.is_index_table():
                self.gtm.tm.logger.critical("%s:BUG:key_size computation for %s" % \
                    (self.gtm.d.name, self.p4_table.name))
                ncc_assert(new_key_size == old_key_size)

        km_used = [-1 for _ in self.key_makers]
        for _km in self.key_makers:
            hw_id = _km.get_hw_id()
            ncc_assert(hw_id not in km_used)
            km_used[_km.km_id] = hw_id

    def ct_build_tbl_ki_info_from_flits(self):
        # Add info from different flits into a common place
        self.i1_phv_chunks = []
        self.k_phv_chunks = []
        self.k_bit_ext = []
        self.i_bit_ext = []
        self.i2_phv_chunks = []
        for i,kf in enumerate(self.km_flits):
            self.i1_phv_chunks += kf.i1_phv_chunks
            self.k_phv_chunks += kf.k_phv_chunks
            self.k_bit_ext += kf.k_bit_ext
            self.i_bit_ext += kf.i_bit_ext
            self.i2_phv_chunks += kf.i2_phv_chunks

        # validate that everything is correctly accounted for
        key_size = 0
        key_size += sum([k[1] for k in self.k_bit_ext])
        key_size += sum([k[1] for k in self.k_phv_chunks])

        # key size may reduce due to unions
        self.key_phv_size = key_size
        if key_size != self.key_size:
            self.gtm.tm.logger.debug('%s k_size changed %d -> %d' % \
                (self.p4_table.name, self.key_size, key_size))

        i_size = sum([k[1] for k in self.i_bit_ext])
        i_size += sum([k[1] for k in self.i1_phv_chunks])
        i_size += sum([k[1] for k in self.i2_phv_chunks])

        # i_size can change due to union-field phvs shared between k and i
        self.i_phv_size = i_size
        if i_size != self.i_size:
            self.gtm.tm.logger.debug('%s i_size changed %d -> %d' % \
                (self.p4_table.name, self.i_size, i_size))
        self.key_size = key_size
        self.i_size = i_size

    def ct_align_table_k_i_bits(self):
        # adjust table k and i fields if not enough hw resources to perform bit extractions
        # Adjust k and i sizes:
        # If too many bit_extractions and K+I < 512:
        # - If too many i bits - include additional bits to byte align (all tables)
        # - If too many k_bits: If TCAM lkp, include additional bits to byte-align
        #   - need to include them in mask and key size? (apis)
        num_flits = self.gtm.tm.be.hw_model['phv']['num_flits']
        flit_size = self.gtm.tm.be.hw_model['phv']['flit_size']
        key_maker_width = self.gtm.tm.be.hw_model['match_action']['key_maker_width']
        bit_extractors = self.gtm.tm.be.hw_model['match_action']['num_bit_extractors']

        num_km = (self.key_size + self.i_size + key_maker_width - 1) / key_maker_width
        ki_size = self.key_size + self.i_size

        if num_km > 2:
            self.gtm.tm.logger.debug("%s:%s : K= %s\nI= %s" % \
                (self.gtm.d.name, self.p4_table.name, self.keys, self.input_fields))
            if self.is_hash_table() or self.is_otcam:
                # QQQ: what is the k size % 512 check is for ??
                ncc_assert(self.i_size < (2 * key_maker_width) and \
                    (self.key_size % (2 * key_maker_width)))
                self.is_wide_key = True
                return False
        ncc_assert(num_km <= 2, "K+I exceed key maker width for table %s" % (self.p4_table.name))
        max_ki = num_km * key_maker_width
        max_bits = num_km * bit_extractors

        n_k_bits = sum(kw for kb,kw in self.k_bit_ext)
        n_i_bits = sum(kw for kb,kw,_ in self.i_bit_ext)

        if (n_k_bits + n_i_bits) <= max_bits:
            # For a tcam table,
            # if k_size < km_width bit n_k_bits > max_km_bits
            # convert extra k-bits to k-byte if there is space
            if self.is_tcam_table() and self.key_size < key_maker_width and \
                n_k_bits > bit_extractors:

                extra_k_bits = n_k_bits - bit_extractors
                k_bit2byte = []
                # start with largest chunk
                sorted_k_bit_ext = sorted(self.k_bit_ext, key=lambda k: k[1], reverse=True)
                rm_chunks = []
                k = 0
                removed_bits = 0

                while extra_k_bits > 0:
                    kB = sorted_k_bit_ext[k][0] / 8
                    endkB = (sorted_k_bit_ext[k][0]+sorted_k_bit_ext[k][1]-1)/8

                    extra_k_bits -= sorted_k_bit_ext[k][1]
                    removed_bits += sorted_k_bit_ext[k][1]
                    for b in range(kB, endkB+1):
                        k_bit2byte.append(b)
                    rm_chunks.append(sorted_k_bit_ext[k])
                    k += 1

                # The following math is not accurate, but conservative. It does not take
                # into account 'covered' k bits due to bit2byte conversion - XXX
                new_ki_size = ki_size - removed_bits + (len(k_bit2byte) * 8)
                if new_ki_size < max_ki:
                    # XXXX fix i1 and i2 designation due to added k-byte
                    # this can be complex if newly added kbyte falls in-between i1 or i2 bytes
                    # which is avoided while selecting bit2byte chunk
                    # accept changes
                    for b in k_bit2byte:
                        fid = (b*8)/flit_size
                        self.km_flits[fid].k_phv_chunks.append((b*8,8))
                        self.km_flits[fid].k_phv_chunks = sorted(self.km_flits[fid].k_phv_chunks,
                            key=lambda k:k[0])
                        k_start = self.km_flits[fid].k_phv_chunks[0][0]
                        k_end = self.km_flits[fid].k_phv_chunks[-1][0] + \
                                self.km_flits[fid].k_phv_chunks[-1][1] - 1

                        i1_to_i2 = []
                        i1_to_k = []
                        for (cs,cw) in self.km_flits[fid].i1_phv_chunks:
                            if (cs+cw) < k_start:
                                # still i1
                                continue
                            if cs > k_end:
                                i1_to_i2.append((cs,cw))
                                continue
                            i1_to_k.append((cs,cw))

                        i2_to_i1 = []
                        i2_to_k = []
                        for (cs,cw) in self.km_flits[fid].i2_phv_chunks:
                            if cs > k_end:
                                # still i2
                                continue
                            if (cs+cw) < k_start:
                                i2_to_i1.append((cs,cw))
                                continue
                            i2_to_k.append((cs,cw))

                        for cscw in i1_to_i2:
                            self.km_flits[fid].i2_phv_chunks.append(cscw)
                            self.km_flits[fid].i1_phv_chunks.remove(cscw)

                        if len(i1_to_i2):
                            self.km_flits[fid].i2_phv_chunks = \
                                sorted(self.km_flits[fid].i2_phv_chunks, key=lambda k:k[0])

                        for cscw in i1_to_k:
                            self.km_flits[fid].k_phv_chunks.append(cscw)
                            self.km_flits[fid].i1_phv_chunks.remove(cscw)

                        for cscw in i2_to_i1:
                            self.km_flits[fid].i1_phv_chunks.append(cscw)
                            self.km_flits[fid].i2_phv_chunks.remove(cscw)

                        if len(i2_to_i1):
                            self.km_flits[fid].i1_phv_chunks = \
                                sorted(self.km_flits[fid].i1_phv_chunks, key=lambda k:k[0])

                        for cscw in i2_to_k:
                            self.km_flits[fid].k_phv_chunks.append(cscw)
                            self.km_flits[fid].i2_phv_chunks.remove(cscw)

                        if len(i1_to_k) or len(i2_to_k):
                            self.km_flits[fid].k_phv_chunks = \
                                sorted(self.km_flits[fid].k_phv_chunks, key=lambda k:k[0])

                    # adjust/removed any overlapping bit chunks that got covered by bytes
                    for c,rc in enumerate(self.k_bit_ext):
                        if rc in rm_chunks:
                            continue
                        sB = rc[0]/8; eB = (rc[0]+rc[1]-1)/8
                        n_sb = rc[0]; n_eb = rc[0]+rc[1]
                        if sB == eB and sB in k_bit2byte:
                            # entire chunk is covered
                            rm_chunks.append(rc)
                        else:
                            if sB in k_bit2byte:
                                n_sb = (sB+1)*8
                            if eB in k_bit2byte:
                                n_eb = eB*8
                            if n_eb - n_sb:
                                self.k_bit_ext[c] = (n_sb, n_eb-n_sb)
                            else:
                                rm_chunks.append(rc)

                    for rc in rm_chunks:
                        fid = rc[0]/flit_size
                        self.km_flits[fid].k_bit_ext.remove(rc)
                    return True
            return False

        extra_bits = n_k_bits + n_i_bits - max_bits
        add_bits = 0
        rm_bits = 0
        rm_chunks = []
        i2k_bits = []
        # combine multiple consecutive i_bits - XXX already done ???
        sorted_i_bits = sorted(self.i_bit_ext, key=lambda k:k[1], reverse=True)

        for b,w,in_key in sorted_i_bits:
            if rm_bits >= extra_bits:
                break
            if in_key:
                # keep these as bit extraction as much as possible
                # remove them later if it is a must
                continue

            # update the chunk info and record the pad chunk
            if w%8:
                n_add_bits = (8-(w%8))
                if (ki_size + add_bits + n_add_bits) > max_ki:
                    continue
                add_bits += (8-(w%8))
            rm_bits += w
            rm_chunks.append((b,w,in_key))

        # if there are still extra bits try to remove those
        for b,w,in_key in sorted_i_bits:
            if rm_bits >= extra_bits:
                break
            if not in_key:
                continue
            if self.is_tcam_table():
                # this indicates that i bits are in the middle of the key in the same flit
                # these can be converted to key if this is a TCAM lookup
                i2k_bits.append((b,w))
                if w%8:
                    add_bits += (8-(w%8))
                rm_bits += w

        if (ki_size + add_bits) > max_ki:
            # still a problem - needs to change the program
            ncc_assert(0)

        # move i_bits chunk to byte extraction
        for rc in rm_chunks:
            w = rc[1]
            self.gtm.tm.logger.debug('%s:convert %s i_bits to i_bytes' % \
                (self.p4_table.name, rc))
            b = rc[0]
            if rc[0] % 8:
                b = (rc[0]/8) * 8
                w += rc[0] - b
            if w % 8:
                w += (8-(w%8))

            self.i_bit_ext.remove(rc)
            fid = b/flit_size
            self.km_flits[fid].i_bit_ext.remove(rc)
            # add it to i1 or i2
            if len(self.km_flits[fid].k_phv_chunks) and \
                self.km_flits[fid].k_phv_chunks[0][0] > b:
                self.km_flits[fid].i1_phv_chunks.append((b,w))
                self.km_flits[fid].i1_phv_chunks = \
                    sorted(self.km_flits[fid].i1_phv_chunks, key=lambda k:k[0])
            else:
                self.km_flits[fid].i2_phv_chunks.append((b,w))
                self.km_flits[fid].i2_phv_chunks = \
                    sorted(self.km_flits[fid].i2_phv_chunks, key=lambda k:k[0])

        for km_flit in self.km_flits:
            # remove duplicate/overlapping chunks from i1 and i2
            km_flit.i1_phv_chunks = \
                self._remove_overlapping_chunks(km_flit.i1_phv_chunks)
            km_flit.i2_phv_chunks = \
                self._remove_overlapping_chunks(km_flit.i2_phv_chunks)

        if len(i2k_bits) and self.is_tcam_table():
            for b,w in i2k_bits:
                fid = b/flit_size
                # record the bits added to the front and back of this chunk
                # remove this chunk from i_bit_ext to k_phv_chunks
                start = b; width = w; pad = 0
                pad_chunk1 = None; pad_chunk2 = None
                if b % 8:
                    sp = (b/8)*8    # start pad
                    sp_w = b-sp
                    pad_chunk1 = (sp, sp_w)
                    start = sp
                    width += sp_w
                    pad += sp_w
                # start is not byte aligned, check width
                if width % 8:
                    ep_w = (8-(w%8))
                    pad_chunk2 = (start+width, ep_w)
                    width += ep_w
                    pad += ep_w

                if (ki_size + pad) > max_ki:
                    continue
                if pad_chunk1:
                    self.i2k_pad_chunks.append(pad_chunk1)
                    self.km_flits[fid].i2k_pad_chunks.append(pad_chunk1)
                if pad_chunk2:
                    self.i2k_pad_chunks.append(pad_chunk2)
                    self.km_flits[fid].i2k_pad_chunks.append(pad_chunk2)
                if not pad_chunk1 and not pad_chunk2:
                    self.i2k_pad_chunks.append((start, width))

                self.gtm.tm.logger.debug("%s:i2k conversion %s -> %s" % \
                        (self.p4_table.name, (b,w), (start,width)))
                self.i_bit_ext.remove((b,w,True))
                self.km_flits[fid].i_bit_ext.remove((b,w,True))
                self.km_flits[fid].k_phv_chunks.append((start,width))
                self.km_flits[fid].k_phv_chunks = \
                    sorted(self.km_flits[fid].k_phv_chunks, key=lambda k:k[0])
                ki_size += pad

                ## extra_bits -= w

            ## ncc_assert(extra_bits <= 0)
        return True

    def _remove_overlapping_chunks(self, phv_chunks):
        new_phv_chunks = []
        c_offset = -1
        for c, (cs,cw) in enumerate(phv_chunks):
            if cs > c_offset:
                c_offset = cs+cw
                new_phv_chunks.append((cs,cw))
                continue

            # overlap or duplicate
            if (cs+cw) <= c_offset:
                # full overlap
                continue

            # add to last chunk (merge contiguous/overlapping chunks)
            ncc_assert(len(new_phv_chunks) != 0)
            (l_cs, l_cw) = new_phv_chunks.pop()
            add_w = (cs+cw) - c_offset
            new_phv_chunks.append((l_cs, l_cw+add_w))
            c_offset = cs+cw
        return new_phv_chunks

    def ct_update_table_config(self):
        # sort the keys based on phv position
        sorted_keys = sorted(self.keys, key=lambda k: k[0].phv_bit)
        # Remove the invalid k fields (no phv allocated to it)
        # This happens due to mistakes in dummy P4 action routines
        valid_keys = []
        for k in sorted_keys:
            if k[0].phv_bit == -1:
                self.gtm.tm.logger.warning("%s:%s:Removing unused key field %s" % \
                    (self.gtm.d.name, self.p4_table.name, k[0].hfname))
                continue
            valid_keys.append(k)
        
        self.keys = valid_keys
        # fix k-i sizes based on unions
        key_cfs = [cf for cf,_,_ in self.keys]
        if self.key_size < 0:
            self.key_size = self.ct_compute_phv_size(key_cfs)

        # sort the input flds based on phv position
        sorted_i = sorted(self.input_fields, key=lambda k: k.phv_bit)
        valid_i = []
        for k in sorted_i:
            if k.phv_bit == -1:
                self.gtm.tm.logger.warning("%s:%s:Removing unused input field %s" % \
                    (self.gtm.d.name, self.p4_table.name, k.hfname))
                continue
            valid_i.append(k)
        
        self.input_fields = valid_i
        if self.i_size < 0:
            self.i_size = self.ct_compute_phv_size(self.input_fields)

        # create per flit info -
        # move K and I fields into respctive flits
        # create i1, i2, i_bits, k, k_bits within each flit
        # combine them at the end as - i1(all):k(all):k_bits:hv_bits:i_bits:i2(all)
        flit_size = self.gtm.tm.be.hw_model['phv']['flit_size']
        num_flits = self.gtm.tm.be.hw_model['phv']['num_flits']

        for k in self.keys:
            flit = k[0].phv_bit / flit_size
            if flit not in self.flits_used:
                self.flits_used.append(flit)

        for k in self.input_fields:
            flit = k.phv_bit / flit_size
            if flit not in self.flits_used:
                self.flits_used.append(flit)

        if len(self.flits_used):
            self.flits_used = sorted(self.flits_used)
            # launch lookup on the last flit
            # XXX this can change when key-makers are shared XXX
            self.launch_flit = self.flits_used[-1]
        # compute the phv chunks and then distribute them to flits to avoid computation
        # problems w/ hdr_unions
        k_phv_chunks = self.gtm.tm.be.pa.get_phv_chunks([k[0] for k in self.keys], self.gtm.d)
        i_phv_chunks = self.gtm.tm.be.pa.get_phv_chunks(self.input_fields, self.gtm.d)

        self.gtm.tm.logger.debug("%s:%s:%s:Initial K,I: K:%s\nI:%s" % \
                    (self.gtm.d.name, self.p4_table.name, self.match_type.name,
                     k_phv_chunks, i_phv_chunks))

        k_size = sum(cw for (cs, cw) in k_phv_chunks)
        i_size = sum(cw for (cs, cw) in i_phv_chunks)
        k_start_delta = 0
        k_end_delta = 0
        # Fix k and i phv chunks to minimize bit extractions
        # start and end of k chunks can be byte-aligned using key_mask
        # All i chunks can be byte aligned (give extra data to MPU)
        # XXX:Check if this conversion does not exceed byte_sel limits
        # For TCAMs all k chunks can be byte aligned too - (increase in tcam size)
        if not self.gtm.tm.be.args.no_te_bit2byte and not self.is_overflow:
            new_k_phv_chunks = copy.copy(k_phv_chunks)
            new_i_phv_chunks = copy.copy(i_phv_chunks)
            # if index table has >1 k_phv_chunks, it means different bit fields are concatenated
            # into an index. Don't convert them to bytes
            # phv allocator takes care of byte alignment and right_justification of index keys
            # that come from metadata, no need to worry about them here
            if len(new_k_phv_chunks) and \
                (not self.is_index_table() or \
                    (self.is_index_table() and len(new_k_phv_chunks) == 1)):
                cs,cw = new_k_phv_chunks[0]
                ce = cs+cw

                if cs % 8:
                    # check if end is byte-aligned. This is to weed out small bit fields
                    # (like hv bit) that appear at the front
                    # if index table has hv bits or other header fields that cannot be phv aligned,
                    # add padding at start only if end is byte aligned
                    
                    start_pad = cs % 8
                    if (self.is_index_table() and ((ce % 8) == 0)) or \
                        (not self.is_index_table() and (start_pad < 4 and ((cs+cw)%8) == 0)):
                        # more bits than pad
                        cs -= start_pad
                        cw += start_pad
                        new_k_phv_chunks.pop(0)
                        new_k_phv_chunks.insert(0,(cs,cw))
                        k_start_delta = start_pad

                # This can done for TCAM tables. For other tables if there are other k-bits
                # then last partial K-byte cannot be extended into full byte
                # Cannot be done for index tables.. key must be byte aligned in key maker
                # (this restriction was not known when this code was written)
                cs,cw = new_k_phv_chunks[-1]
                ce = cs+cw

                # for tcam table, make sure that the last chunk start is byte aligned
                # so that it is eligible for byte extraction
                if (cs % 8) == 0 and ce % 8 and self.is_tcam_table():
                    new_k_phv_chunks.pop()
                    end_pad = (8-(ce % 8))
                    cw += end_pad
                    new_k_phv_chunks.append((cs,cw))
                    k_end_delta = end_pad

            for c, (cs,cw) in enumerate(new_i_phv_chunks):
                new_i_phv_chunks.pop(c)
                if cs % 8:
                    start_pad = cs % 8
                    cs -= start_pad
                    cw += start_pad
                ce = cs+cw
                if ce % 8:
                    end_pad = (8-(ce % 8))
                    cw += end_pad
                new_i_phv_chunks.insert(c, (cs,cw))

            # above logic can cause overlapping chunks in I phv -fix those
            # these are removed later by _remove_duplicate function - only if there are k-bytes
            # so fix it here as well
            # if self.p4_table.name == 'l4_profile': pdb.set_trace()
            # sort based on cstart
            tmp_i_phv_chunks = sorted(new_i_phv_chunks, key=lambda k:k[0])
            new_i_phv_chunks = self._remove_overlapping_chunks(tmp_i_phv_chunks)

            new_k_size = sum(cw for (cs, cw) in new_k_phv_chunks)
            new_i_size = sum(cw for (cs, cw) in new_i_phv_chunks)

            # accept bit2byte conversion only if increase in K+I is within resource limits
            key_maker_width = self.gtm.tm.be.hw_model['match_action']['key_maker_width']
            new_num_km = (new_k_size + new_i_size + key_maker_width - 1) / key_maker_width
            num_km = (k_size + i_size + key_maker_width - 1) / key_maker_width
            ki_delta = new_k_size + new_i_size - k_size - i_size
            k_delta = new_k_size - k_size
            # accept 50% (arbitrary) increase in k+I
            #allow_increase = 2 # 50%
            allow_increase = 1 # 100%

            new_num_km_k = (new_k_size + key_maker_width - 1) / key_maker_width

            # don't change bit->byte for I if this is table has overflow(collision) table
            # as the byte extraction can lead to banyon violation (ideally check if i-chunks are
            # before k-chunks.. but this is just a HACK
            if (new_num_km <= num_km) and (ki_delta >= 0) and \
                (ki_delta <= ((k_size+i_size) / allow_increase)) and \
                (not self.collision_ct or new_i_size == i_size):

                k_phv_chunks = new_k_phv_chunks
                i_phv_chunks = new_i_phv_chunks

                self.gtm.tm.logger.debug("%s:%s:%s:bit2byte changes to K,I" % \
                    (self.gtm.d.name, self.p4_table.name, self.match_type.name))
                self.gtm.tm.logger.debug("%s:%s:(k,I) new:(%d, %d) old:(%d,%d) k_delta(%d, %d)" % \
                    (self.gtm.d.name, self.p4_table.name, new_k_size, new_i_size, k_size, i_size,
                     k_start_delta, k_end_delta))
                self.start_key_off_delta = k_start_delta
                self.end_key_off_delta = k_end_delta
            elif new_num_km_k <= num_km and k_delta > 0 and \
                k_delta <= ((k_size+i_size) / allow_increase):

                # only k changes can be accepted
                k_phv_chunks = new_k_phv_chunks

                self.gtm.tm.logger.debug("%s:%s:%s:bit2byte changes to K-only" % \
                    (self.gtm.d.name, self.p4_table.name, self.match_type.name))
                self.gtm.tm.logger.debug("%s:%s:(k,I) new:(%d) old:(%d)" % \
                    (self.gtm.d.name, self.p4_table.name, new_k_size, k_size))
                self.start_key_off_delta = k_start_delta
                self.end_key_off_delta = k_end_delta
            else:
                self.gtm.tm.logger.debug("%s:%s:bit2byte changes to K,I are not accepted" % \
                    (self.gtm.d.name, self.p4_table.name))
                self.gtm.tm.logger.debug("%s:%s:(k,I) new:(%d, %d) old:(%d,%d)" % \
                    (self.gtm.d.name, self.p4_table.name, new_k_size, new_i_size, k_size, i_size))

        flit_k_phv_chunks = {f:[] for f in range(num_flits)}
        flit_i_phv_chunks = {f:[] for f in range(num_flits)}

        for cs, cw in k_phv_chunks:
            start_fid = cs/flit_size
            ncc_assert(start_fid >= 0)
            end_fid = (cs+cw-1)/flit_size
            if start_fid == end_fid:
                flit_k_phv_chunks[start_fid].append((cs,cw))
                continue
            # crossing the flit
            w_flit = ((start_fid+1) * flit_size) - cs
            w = min(flit_size, w_flit)
            ncc_assert(w > 0)
            flit_k_phv_chunks[start_fid].append((cs,w))
            new_cs = cs+w
            for fid in range(start_fid+1, end_fid):
                flit_k_phv_chunks[fid].append((new_cs,flit_size))
                new_cs += flit_size
                w += flit_size
            if (cw-w):
                flit_k_phv_chunks[end_fid].append((new_cs,cw-w))

        for cs, cw in i_phv_chunks:
            start_fid = cs/flit_size
            ncc_assert(start_fid >= 0)
            end_fid = (cs+cw-1)/flit_size
            if start_fid == end_fid:
                flit_i_phv_chunks[start_fid].append((cs,cw))
                continue
            # crossing the flit
            w_flit = ((start_fid+1) * flit_size) - cs
            w = min(flit_size, w_flit)
            flit_i_phv_chunks[start_fid].append((cs,w))
            ncc_assert(w > 0)
            new_cs = cs+w
            for fid in range(start_fid+1, end_fid):
                flit_i_phv_chunks[fid].append((new_cs,flit_size))
                new_cs += flit_size
                w += flit_size
            if (cw-w):
                flit_i_phv_chunks[end_fid].append((new_cs,cw-w))

        for i,kf in enumerate(self.km_flits):
            k_phv_chunks = flit_k_phv_chunks[i]
            i_phv_chunks = flit_i_phv_chunks[i]

            # remove non-byte aligned flds from the k vector
            new_phv_chunks = []
            for c, (cs, cw) in enumerate(k_phv_chunks):
                part_bits = cs % 8
                if part_bits:
                    if cw > (8-part_bits):
                        kf.k_bit_ext.append((cs, 8-part_bits))
                        cs += (8-part_bits)
                        cw -= (8-part_bits)
                    else:
                        # entire fld is in this container
                        kf.k_bit_ext.append((cs, cw))
                        cs = 0
                        cw = 0

                ncc_assert((cs % 8) == 0)
                part_bits = cw % 8
                if part_bits:
                    kf.k_bit_ext.append((cs+cw-part_bits, part_bits))
                    cw -= part_bits
                if cw:
                    new_phv_chunks.append((cs, cw))

            k_phv_chunks = new_phv_chunks
            if len(k_phv_chunks):
                k_start = k_phv_chunks[0][0]
                k_end = k_phv_chunks[-1][0] + k_phv_chunks[-1][1]
            else:
                k_start = -1
                k_end = -1
            new_i_phv_chunks = []

            # move non-byte aligned flds from the i vector to a separate list
            # for toeplitz table, key is split between two key-makers as key and seed
            # so i-bytes will apprear to be within key, for now just ignore it, there will
            # an ncc_assert(later.)
            # XXX check portions of seed/key separately to find i_in_key
            i_in_key = 0
            if not self.is_toeplitz_hash():
                for c, (cs, cw) in enumerate(i_phv_chunks):
                    part_start = cs % 8
                    part_end = (cs+cw) % 8
                    if cs >= k_start and cs < k_end:
                        in_key = True
                    else:
                        in_key = False
                    if part_start:
                        part_len = cw if cw < (8-part_start) else (8-part_start)
                        kf.i_bit_ext.append((cs, part_len, in_key))
                        if in_key:
                            i_in_key += part_len
                        cw -= part_len
                        cs += part_len
                        if not cw:
                            continue
                    if part_end:
                        kf.i_bit_ext.append((cs+cw-part_end, part_end, in_key))
                        if in_key:
                            i_in_key += part_end
                        cw -= part_end
                        if not cw:
                            continue
                    if cw:
                        new_i_phv_chunks.append((cs, cw))

            if len(new_i_phv_chunks) and len(k_phv_chunks):
                i_phv_chunks = _remove_duplicate_chunks(new_i_phv_chunks, k_phv_chunks)
            else:
                i_phv_chunks = new_i_phv_chunks

            # convert any bytes that fall within k chunks to bits for now (move them to bytes later)
            for cs, cw in i_phv_chunks:
                part_start = cs % 8
                part_end = (cs+cw) % 8
                is_i1 = False   # default is i2
                is_i_bits = False
                if len(k_phv_chunks):
                    # for toeplitz table treat all i bytes as i2
                    if not self.is_toeplitz_hash():
                        if cs < k_phv_chunks[0][0]:
                            is_i1 = True
                        elif cs < k_phv_chunks[-1][0]:
                            is_i_bits = True
                            i_in_key += cw
                        else:
                            pass
                if is_i_bits:
                    kf.i_bit_ext.append((cs, cw, True if i_in_key else False))
                    continue
                if part_start:
                    part_len = cw if cw < (8-part_start) else (8-part_start)
                    kf.i_bit_ext.append((cs, part_len, False))
                    cw -= part_len
                    cs += part_len
                    if not cw:
                        continue
                if part_end:
                    kf.i_bit_ext.append((cs+cw-part_end, part_end, False))
                    cw -= part_end
                    if not cw:
                        continue
                if is_i1:
                    kf.i1_phv_chunks.append((cs, cw))
                else:
                    kf.i2_phv_chunks.append((cs, cw))

            if i_in_key:
                self.gtm.tm.logger.debug("%s:%s has input fields %d (bits) mixed with key" % \
                    (self.d, self.p4_table.name, i_in_key))
                self.i_in_key = i_in_key # for printing (for analysis)
            kf.k_phv_chunks = k_phv_chunks # [(phv_bit, width)]

        # Add info from different flits into a common place
        self.ct_build_tbl_ki_info_from_flits()
        if self.ct_align_table_k_i_bits():
            # rebuild table info if anyting changed
            self.ct_build_tbl_ki_info_from_flits()

        self.gtm.tm.logger.debug("%s:%s:%s:Final K,I: K:%s, K-bits:%s\nI1:%s, I2:%s, I-bits%s" % \
                    (self.gtm.d.name, self.p4_table.name, self.match_type.name,
                     self.k_phv_chunks, self.k_bit_ext, self.i1_phv_chunks, self.i2_phv_chunks,
                     self.i_bit_ext))

    def ct_print_km_profiles(self):
        tbl_str = "%s:" % self.p4_table.name
        for km in self.key_makers:
            km_hw_id = km.shared_km.hw_id if km.shared_km else km.hw_id
            tbl_str += "km=%d " % km_hw_id
        self.gtm.tm.logger.debug(tbl_str)

    def ct_create_km_profiles(self):
        # QQ what is the value of byte_selector when byte is not used? => MAX_VAL
        # Key+I data is placed into the key-maker as-
        # input data bytes can be placed before and after the key bytes. Any sub-byte
        # extractions are converted to bit-extractions for both k and i fields. Bit
        # extractions can be added to either end of the key. HV bits, if used as key
        # will be extracted using bit-extractors
        # +---------------+-------+--------+---------+--------+--------------+
        # |[i1_data_bytes] k_bytes [k_bits][k_hv_bits][i_bits][i2_data_bytes]|
        # +---------------+-------+--------+---------+--------+--------------+
        # when a table needs multiple key makers, 2nd key maker cannot have i1 data
        #
        # make sure to leave unused byte_sels when profiles are combined
        # What about bit_sel???? Can they be combined?
        combined_profile = capri_km_profile(self.gtm)
        for kf in self.km_flits:
            kf.flit_create_km_profile()

        #all_phv_chunks = self.i1_phv_chunks + self.k_phv_chunks + self.i2_phv_chunks
        for f,kf in enumerate(self.km_flits):
            combined_profile.i1_byte_sel += kf.km_profile.i1_byte_sel
            combined_profile.k_byte_sel += kf.km_profile.k_byte_sel
            combined_profile.i2_byte_sel += kf.km_profile.i2_byte_sel
            combined_profile.k_bit_sel += kf.km_profile.k_bit_sel
            combined_profile.i_bit_sel += kf.km_profile.i_bit_sel

        # XXX need to add bits extractions right after K bytes
        combined_profile.byte_sel = combined_profile.i1_byte_sel + combined_profile.k_byte_sel + \
            combined_profile.i2_byte_sel
        combined_profile.bit_sel = combined_profile.k_bit_sel + combined_profile.i_bit_sel

        self.combined_profile = combined_profile

        # hardware allows having out of order byte loading across flits.
        # so the combined profile will not be in sorted phv order
        key_maker_width = self.gtm.tm.be.hw_model['match_action']['key_maker_width']
        ki_size = ((self.key_phv_size + self.i_phv_size + 7) / 8 ) * 8
        reserve_bytes = 0
        if self.is_hash_table():
            # HACK: for now just add required bytes to total_kB so that we have some
            # space in the key_maker for hbm hash table to align the key in key-maker
            # and HBM memory. H/w needs first 8 bits for action_pc when used
            if self.num_actions() > 1 and not self.collision_ct:
                action_id_size = self.gtm.tm.be.hw_model['match_action']['action_id_size']
                reserve_bytes = (action_id_size+7)/8
            elif self.collision_ct:
                collision_key_size = self.gtm.tm.be.hw_model['match_action']['collision_index_sz']
                reserve_bytes = (collision_key_size / 8)
            else:
                pass
        self.num_km = (ki_size + reserve_bytes + key_maker_width - 1) / key_maker_width
        self.gtm.tm.logger.debug("%s:%s:Table-Logical-Profile: %s" % \
            (self.gtm.d.name, self.p4_table.name, self.combined_profile))

    def create_toeplitz_key_makers(self):
        # Toeplitz table has special requirements for the constucting the key-makers
        # Each cycle uses two key-makers as -
        # cycle0: km0 = secret[319:64] upper 256 bits left justified in km
        #         km1 = key[] upper 256 bits
        # cycle1+:km0 = '32b0 + secret[63:0] left justified in km
        #         km1 = '32b0 + key[] rest of the bits
        # On clycle1+, need to leave 32 msbits in key and seed keymaker as hw puts
        # 32bits from previous cyc in there

        # allocate 2 kms for each flit used.. key1 and seed1 can be loaded across
        # multiple flits.. but rest of the key/seed must come from each subsequent
        # flit.. once the calculation is started each subsequent flit must feed the
        # key-maker until the last flit
        # first 2 flits
        ncc_assert(len(self.flits_used) == 2, "Must use two consecutive flits for Toeplitz Hash")
        self.num_km = len(self.flits_used) * 2
        max_km_width = self.gtm.tm.be.hw_model['match_action']['key_maker_width']
        max_kmB = max_km_width/8
        flit_szB = self.gtm.tm.be.hw_model['phv']['flit_size'] / 8

        k_phv_chunks = self.gtm.tm.be.pa.get_phv_chunks(self.toeplitz_key_cfs, self.gtm.d)
        key_phv_bytes = sorted(_phv_chunks_to_bytes(k_phv_chunks))
        k_len = len(key_phv_bytes)
        s_phv_chunks = self.gtm.tm.be.pa.get_phv_chunks(self.toeplitz_seed_cfs, self.gtm.d)
        seed_phv_bytes = sorted(_phv_chunks_to_bytes(s_phv_chunks))
        s_len = len(seed_phv_bytes)

        b=0
        km_id=0
        for fid in range(self.flits_used[0], self.flits_used[-1]+1):
        #{
            # allocate 2 key makers (512bits total) per flit which has bytes to load
            # it is possible to collect 512b worth key from multiple flits, it is not
            # currently supported by hw - not done. For now there is a km and km_profile
            # per flit
            km0 = capri_key_maker(self, [self])
            km0.km_id = km_id
            km0.flits_used.append(fid)
            self.key_makers.append(km0)
            flit_profile0 = capri_km_profile(self.gtm)
            km0.combined_profile = capri_km_profile(self.gtm)
            km1= capri_key_maker(self, [self])
            km1.km_id = km_id+1
            km1.flits_used.append(fid)
            km_id += 2
            self.key_makers.append(km1)
            flit_profile1 = capri_km_profile(self.gtm)
            km1.combined_profile = capri_km_profile(self.gtm)


            if fid == self.flits_used[0]:
                bytes_avail = max_kmB
            else:
                bytes_avail = max_kmB - 4

            for i in range(bytes_avail):
                if b < s_len:
                    ncc_assert((seed_phv_bytes[b] / flit_szB) == fid)
                    flit_profile0.k_byte_sel.append(seed_phv_bytes[b])
                if b < k_len:
                    ncc_assert((key_phv_bytes[b] / flit_szB) == fid)
                    flit_profile1.k_byte_sel.append(key_phv_bytes[b])
                b += 1

            flit_profile0.byte_sel += flit_profile0.k_byte_sel
            flit_profile1.byte_sel += flit_profile1.k_byte_sel

            km0.flit_km_profiles[fid] = flit_profile0
            km1.flit_km_profiles[fid] = flit_profile1
            # update km0 and km1 combined_profiles
            km0.combined_profile += flit_profile0
            km1.combined_profile += flit_profile1

            if fid != self.flits_used[0]:
                for i in range(4):
                    km0.combined_profile.k_byte_sel.insert(0,-1)
                    km0.combined_profile.byte_sel.insert(0,-1)
                    flit_profile0.k_byte_sel.insert(0,-1)
                    flit_profile0.byte_sel.insert(0,-1)

                    km1.combined_profile.k_byte_sel.insert(0,-1)
                    km1.combined_profile.byte_sel.insert(0,-1)
                    flit_profile1.k_byte_sel.insert(0,-1)
                    flit_profile1.byte_sel.insert(0,-1)

        #}

        # XXX we can allow some i2-bytes in the last flit km1 (key) if there is space
        last_fid = self.flits_used[-1]
        last_key_profile = self.key_makers[-1].combined_profile
        last_key_flit_profile = self.key_makers[-1].flit_km_profiles[last_fid]

        bytes_avail = max_kmB - len(self.key_makers[-1].combined_profile.byte_sel)
        kf = self.km_flits[last_fid]
        i1_bytes = len(kf.km_profile.i1_byte_sel)
        i2_bytes = len(kf.km_profile.i2_byte_sel)
        i_bytes = i1_bytes + i2_bytes

        if i_bytes > bytes_avail:
            ncc_assert(0, "Only %d bytes are available for I for Toeplitz table %s, need %d" % \
                (bytes_avail, self.p4_table.name, i_bytes))

        # last key maker contains last chunk of the key
        last_key_flit_profile.i1_byte_sel += kf.km_profile.i1_byte_sel
        last_key_flit_profile.byte_sel = kf.km_profile.i1_byte_sel + \
                                        last_key_flit_profile.byte_sel
        last_key_profile.i1_byte_sel += kf.km_profile.i1_byte_sel
        last_key_profile.byte_sel = kf.km_profile.i1_byte_sel + \
                                        last_key_profile.byte_sel
        last_key_flit_profile.i2_byte_sel += kf.km_profile.i2_byte_sel
        last_key_flit_profile.byte_sel += kf.km_profile.i2_byte_sel
        last_key_profile.i2_byte_sel += kf.km_profile.i2_byte_sel
        last_key_profile.byte_sel += kf.km_profile.i2_byte_sel

        # setup the key_offsets and last_flit_key_offsets
        # key is in km1 i.e. lower half so add km1 size to the offsets
        # also hw does not want leading '32b0 included in the key_offset
        self.start_key_off = 0; self.end_key_off = self.key_size
        self.last_flit_start_key_off = max_km_width + 32   # 32bits reserved for bits from last flit
        self.last_flit_end_key_off = max_km_width + (len(last_key_profile.k_byte_sel) * 8)

    def create_key_makers(self):
        # Allocate key-maker(s) for the table
        # If >1 kms are needed, split the profiles into each key-maker
        # When >1 km is needed, the K and I bytes and bits are split in different ways
        # based on table type, key size etc.
        # For K > key_maker_width:
        #   KM0: (k bits < bit_ext)     KM1:
        #   +--------+-------+          +---------+--------+------+---------+
        #   |[i_bits] k_bytes|          | k_bytes [k_bits][i_bits] [i_bytes |
        #   +--------+-------+          +---------+--------+------+---------+
        #
        #   KM0: (k bits > bit_ext)     KM1:
        #   +--------+------+           +---------+------+--------+---------+
        #   | k_bytes k-bits|           | k_bytes [k_bits][i_bits] i_bytes  |
        #   +--------+------+           +---------+------+--------+---------+

        # For K < key_maker_width:
        #   KM0:                        KM1:
        #   +-------+--------+--+-----+ +---------+--------+
        #   |k_bytes [k-bits] [i-bits]| | [i_bytes [i-bits]|
        #   +-------+--------+--+-----+ +---------+--------+
        #
        # when KM profiles are shared between hash/idx and TCAM, K bytes/bits are considered
        # only from hash/idx table, as they need to be contiguous

        if self.is_toeplitz_hash():
            self.create_toeplitz_key_makers()
            return

        if self.num_km == 0:
            # always use KM0 for this, it does not matter if km is used for another table
            ncc_assert(self.is_mpu_only())
            km = capri_key_maker(self, [self])
            km.km_id = 0
            self.key_makers.append(km)
            return

        if self.is_otcam:
            # skip key-maker for otcam - h/w uses the km of the parent hash
            # create km for overflow hash table.. it will be merged with its hash_table later
            return

        self.gtm.tm.logger.debug("%s:Create %d key_makers for %s" % \
            (self.gtm.d.name, self.num_km, self.p4_table.name))

        reserve_bytes = 0
        if self.is_hash_table():
            # HACK: for now just add required bytes to total_kB so that we have some
            # space in the key_maker for hbm hash table to align the key in key-maker
            # and HBM memory. H/w needs first 8 bits for action_pc when used
            if self.num_actions() > 1 and not self.collision_ct:
                action_id_size = self.gtm.tm.be.hw_model['match_action']['action_id_size']
                reserve_bytes = (action_id_size+7)/8
            elif self.collision_ct:
                collision_key_size = self.gtm.tm.be.hw_model['match_action']['collision_index_sz']
                reserve_bytes = (collision_key_size / 8)
            else:
                pass

        if not self.is_wide_key:
            for i in range(self.num_km):
                km = capri_key_maker(self, [self])
                km.km_id = i
                self.key_makers.append(km)

        if self.num_km < 2:
            # only one key-maker, copy all info from table
            # XXX check if there is enough space for action_pc or collision idx for hash
            # tables
            km = self.key_makers[0]
            km.combined_profile = copy.deepcopy(self.combined_profile)
            km.combined_profile._update_bit_loc_key_off()
            for fid,kf in enumerate(self.km_flits):
                km.flit_km_profiles[fid] = copy.deepcopy(kf.km_profile)
            for fid in range(self.flits_used[0], self.flits_used[-1]+1):
                km.flits_used.append(fid)
            return

        max_km_width = self.gtm.tm.be.hw_model['match_action']['key_maker_width']
        max_kmB = max_km_width/8
        max_km_bits = self.gtm.tm.be.hw_model['match_action']['num_bit_extractors']

        if self.num_km > 2:
            # wide key table
            # need a pair of kms per flit
            ncc_assert(self.is_wide_key)
            # split the combined profile into profiles of max km width
            # make sure that any 'i' data is only in the last profile
            km_id = 0
            last_fid = self.flits_used[-1]
            for fid in range(self.flits_used[0], self.flits_used[-1]+1): #{
                # allocate 2 key makers (512bits total) per flit which has bytes to load
                # it is possible to collect 512b worth key from multiple flits, it is not
                # currently supported by hw - not done. For now there is a km and km_profile
                # per flit
                km0 = capri_key_maker(self, [self])
                km0.km_id = km_id
                km0.flits_used.append(fid)
                self.key_makers.append(km0)
                flit_profile0 = capri_km_profile(self.gtm)
                km0.combined_profile = capri_km_profile(self.gtm)

                km1= capri_key_maker(self, [self])
                km1.km_id = km_id+1
                km1.flits_used.append(fid)
                km_id += 2
                self.key_makers.append(km1)
                flit_profile1 = capri_km_profile(self.gtm)
                km1.combined_profile = capri_km_profile(self.gtm)

                # copy all available bits and bytes.. everything must fit
                kf = self.km_flits[fid]
                bytes_avail0 = max_kmB
                bytes_avail1 = max_kmB
                if len(kf.km_profile.bit_sel): #{
                    k0_bits = min(max_km_bits, len(kf.km_profile.k_bit_sel))
                    i0_bits = min(max_km_bits-k0_bits, len(kf.km_profile.i_bit_sel))
                    for k in range(k0_bits):
                        flit_profile0.k_bit_sel.append(kf.km_profile.k_bit_sel[k])
                    for i in range(i0_bits):
                        flit_profile0.i_bit_sel.append(kf.km_profile.i_bit_sel[i])
                    flit_profile0.bit_sel = flit_profile0.k_bit_sel + flit_profile0.i_bit_sel

                    if fid != last_fid:
                        ncc_assert(len(flit_profile0.i_bit_sel) == 0)
                        #"Cannot have i-bits until last flit"

                    k1_bits = 0 if len(kf.km_profile.k_bit_sel) < k0_bits else \
                        len(kf.km_profile.k_bit_sel) - k0_bits
                    i1_bits = 0 if len(kf.km_profile.i_bit_sel) < i0_bits else \
                        len(kf.km_profile.i_bit_sel) - i0_bits

                    for k in range(k1_bits):
                        flit_profile1.k_bit_sel.append(kf.km_profile.k_bit_sel[k0_bits+k])

                    for i in range(i1_bits):
                        flit_profile1.i_bit_sel.append(kf.km_profile.i_bit_sel[i0_bits+i])

                    flit_profile1.bit_sel = flit_profile1.k_bit_sel + flit_profile1.i_bit_sel

                    if fid != last_fid:
                        ncc_assert(len(flit_profile1.i_bit_sel) == 0)
                        #"Cannot have i-bits until last flit"

                    bytes_avail0 -= (k0_bits+i0_bits+7)/8
                    bytes_avail1 -= (k1_bits+i1_bits+7)/8
                #}

                # if there is any wastage due to bit extractors not being full, make sure
                # rest of the required byte can be loaded
                # need to add bytes for action_pc/overflow index alignment
                bytes_needed = len(kf.km_profile.byte_sel)
                if fid == last_fid:
                    bytes_avail0 -= reserve_bytes

                ncc_assert(bytes_needed <= (bytes_avail0+bytes_avail1))
                k0_bytes = min(bytes_avail0, len(kf.km_profile.k_byte_sel))

                k1_bytes = len(kf.km_profile.k_byte_sel) - k0_bytes
                ncc_assert(k1_bytes <= bytes_avail1)

                # for wide-key, we can have i1 and i2 bytes only on the last flit as -
                # action_pc/collision_idx - i1bytes - key - i2bytes
                # Since both km0 and km1 are chained,
                # if key is in km0, only km0 can have i1bytes,
                # if key spans km0 and km1, i1bytes must be in km0 and i2bytes must be in km1
                i10_bytes = min(bytes_avail0-k0_bytes, len(kf.km_profile.i1_byte_sel))
                i20_bytes = min(bytes_avail0-i10_bytes, len(kf.km_profile.i2_byte_sel))

                i11_bytes = len(kf.km_profile.i1_byte_sel) - i10_bytes
                i21_bytes = len(kf.km_profile.i2_byte_sel) - i20_bytes

                if fid != last_fid:
                    ncc_assert((i10_bytes+i20_bytes+i11_bytes+i21_bytes) == 0)
                    #"Cannot have i-bytes until last flit"
                else:
                    if k1_bytes:
                        # key spans into km1
                        ncc_assert(i20_bytes == 0)
                        ncc_assert(i11_bytes == 0)

                for i in range(i10_bytes):
                    flit_profile0.i1_byte_sel.append(kf.km_profile.i1_byte_sel[i])
                for k in range(k0_bytes):
                    flit_profile0.k_byte_sel.append(kf.km_profile.k_byte_sel[k])
                for i in range(i20_bytes):
                    flit_profile0.i2_byte_sel.append(kf.km_profile.i2_byte_sel[i])

                flit_profile0.byte_sel = flit_profile0.i1_byte_sel + flit_profile0.k_byte_sel + \
                    flit_profile0.i2_byte_sel

                for i in range(i11_bytes):
                    flit_profile1.i1_byte_sel.append(kf.km_profile.i1_byte_sel[i+i10_bytes])
                for k in range(k1_bytes):
                    flit_profile1.k_byte_sel.append(kf.km_profile.k_byte_sel[k+k0_bytes])
                for i in range(i21_bytes):
                    flit_profile1.i2_byte_sel.append(kf.km_profile.i2_byte_sel[i+i20_bytes])

                flit_profile1.byte_sel = flit_profile1.i1_byte_sel + flit_profile1.k_byte_sel + \
                    flit_profile1.i2_byte_sel

                km0.flit_km_profiles[fid] = flit_profile0
                km1.flit_km_profiles[fid] = flit_profile1
                # update km0 and km1 combined_profiles
                km0.combined_profile += flit_profile0
                km0.combined_profile._update_bit_loc_key_off()
                km1.combined_profile += flit_profile1
                km1.combined_profile._update_bit_loc_key_off()
            #}
            # num_km might change since 2 kms are allocated per flit even if both kms are not
            # loaded - update it here
            self.num_km = len(self.key_makers)
            return

        ncc_assert(self.num_km <= 2 )# max km_per_key
        # multiple kms
        # check how to split
        # prefer if key and i can be separated into two kms
        # Any i1 data can be moved to second key-maker
        # XXX Can we do this while creating the km_profiles ???
        # Allocate key-makers up-front??
        # There is a lot of benefit getting rid on i1 in terms of entry size savings
        total_kB = (self.key_phv_size + 7) / 8
        total_kB += reserve_bytes   # keep space for action_pc
        total_i1B = len(self.combined_profile.i1_byte_sel)
        total_i2B = len(self.combined_profile.i2_byte_sel)
        total_iB = (self.i_phv_size + 7) / 8

        if total_kB <= max_kmB and total_iB <= max_kmB:
            self.gtm.tm.logger.debug("%s:Split key_makers as K and I" % self.p4_table.name)
            # K and I can be split into separate key makers
            # XXX reserved bytes not really added to the key maker... it might cause problem
            # later.. need to fix it (TBD)
            km0 = self.key_makers[0]
            km0_profile = capri_km_profile(self.gtm)
            km0_profile.byte_sel += self.combined_profile.k_byte_sel
            km0_profile.k_byte_sel += self.combined_profile.k_byte_sel
            km0_profile.bit_sel += self.combined_profile.k_bit_sel
            km0_profile.k_bit_sel += self.combined_profile.k_bit_sel
            km0_profile._update_bit_loc_key_off()
            km0.combined_profile = km0_profile
            for fid,kf in enumerate(self.km_flits):
                flit_profile = capri_km_profile(self.gtm)
                #flit_profile.copy_k_only(kf.km_profile)
                flit_profile.k_byte_sel += kf.km_profile.k_byte_sel
                flit_profile.byte_sel += kf.km_profile.k_byte_sel
                flit_profile.k_bit_sel += kf.km_profile.k_bit_sel
                flit_profile.bit_sel += kf.km_profile.k_bit_sel

                km0.flit_km_profiles[fid] = flit_profile

            # create and new profile for km1
            km1 = self.key_makers[1]
            km1_profile = capri_km_profile(self.gtm)
            km1_profile.byte_sel += self.combined_profile.i1_byte_sel
            km1_profile.byte_sel += self.combined_profile.i2_byte_sel
            # all I data is considered I2 here
            km1_profile.i2_byte_sel += km1_profile.byte_sel
            km1_profile.bit_sel += self.combined_profile.i_bit_sel
            km1_profile.i_bit_sel += self.combined_profile.i_bit_sel

            km1_profile._update_bit_loc_key_off()
            km1.combined_profile = km1_profile

            for fid,kf in enumerate(self.km_flits):
                flit_profile = capri_km_profile(self.gtm)
                flit_profile.byte_sel += kf.km_profile.i1_byte_sel
                flit_profile.byte_sel += kf.km_profile.i2_byte_sel
                flit_profile.i2_byte_sel += flit_profile.byte_sel

                flit_profile.i_bit_sel += kf.km_profile.i_bit_sel
                flit_profile.bit_sel += kf.km_profile.i_bit_sel
                km1.flit_km_profiles[fid] = flit_profile

        else:
            self.gtm.tm.logger.debug("%s:%s:Split key_makers (K+I) as it fits" % \
                (self.gtm.d.name, self.p4_table.name))

            ncc_assert(len(self.oo_flits) == 0)

            km0 = self.key_makers[0]
            km0_profile = capri_km_profile(self.gtm)

            km1 = self.key_makers[1]
            km1_profile = capri_km_profile(self.gtm)

            # try to split i1 out to another km and keep k and other i data in km0
            km0_free = max_kmB; km1_free = max_kmB

            num_kbits = len(self.combined_profile.k_bit_sel)
            num_ibits = len(self.combined_profile.i_bit_sel)

            if total_kB > max_kmB:
                self.gtm.tm.logger.debug("%s:%s:Split key_makers K > max_km" % \
                    (self.gtm.d.name, self.p4_table.name))
                km0_free -= reserve_bytes
                # not much can be done. just place in order(i1,k,...) as it fits
                # if k_bits > max_km_bits : split k bits in km0(end) and km1 after k bytes
                # if k_bits < max_km_bits : move them all to km1 after k bytes. In this case
                # any i-bits in km0 will have to be placed before k-bytes
                # if i_bits > max_km_bits : move some i-bits before key in km0
                if total_i1B:
                    ncc_assert(total_i1B < max_kmB)
                    km0_profile.i1_byte_sel += self.combined_profile.i1_byte_sel
                    km0_free -= total_i1B
                if num_kbits > max_km_bits:
                    for b in range(max_km_bits):
                        km0_profile.k_bit_sel.append(self.combined_profile.k_bit_sel[b])
                    km0_free -= (max_km_bits/8)
                    for b in range(max_km_bits, num_kbits):
                        km1_profile.k_bit_sel.append(self.combined_profile.k_bit_sel[b])
                    km1_profile.i_bit_sel = self.combined_profile.i_bit_sel
                    km1_free -= ((num_kbits-max_km_bits+num_ibits+7) / 8)
                else:
                    # put k bits in km1 at the end of K-bytes
                    km1_profile.k_bit_sel += self.combined_profile.k_bit_sel
                    # distribute i_bits evenly
                    km_free_bits = (max_km_bits + max_km_bits - num_kbits - num_ibits)/2
                    km0_bits = max_km_bits - km_free_bits
                    km0_bits = min(km0_bits, num_ibits)
                    for b in range(km0_bits):
                        km0_profile.i_bit_sel.append(self.combined_profile.i_bit_sel[b])
                    km0_free -= ((km0_bits+7)/8)
                    for b in range(km0_bits, num_ibits):
                        km1_profile.i_bit_sel.append(self.combined_profile.i_bit_sel[b])
                    km1_free -= ((num_ibits-km0_bits+7)/8)

                # split k into km0 and km1
                for b in range(km0_free):
                    km0_profile.k_byte_sel.append(self.combined_profile.k_byte_sel[b])
                for b in range(km0_free, len(self.combined_profile.k_byte_sel)):
                    km1_profile.k_byte_sel.append(self.combined_profile.k_byte_sel[b])

                # copy i2
                km1_profile.i2_byte_sel += self.combined_profile.i2_byte_sel

                km0_profile.byte_sel = (km0_profile.i1_byte_sel + km0_profile.k_byte_sel + \
                    km0_profile.i2_byte_sel)
                km0_profile.bit_sel = km0_profile.k_bit_sel + km0_profile.i_bit_sel
                km1_profile.byte_sel = (km1_profile.i1_byte_sel + km1_profile.k_byte_sel + \
                    km1_profile.i2_byte_sel)
                km1_profile.bit_sel = km1_profile.k_bit_sel + km1_profile.i_bit_sel

                km0.combined_profile = km0_profile
                km1.combined_profile = km1_profile

                km0_profile._update_bit_loc_key_off()
                km1_profile._update_bit_loc_key_off()
            else:
                # place key in km0
                # if entire i1 can move to km1, move it and move rest of the i2 evenly
                # if i1 cannot be completely moved to km1, then divide i1+i2 evenly
                self.gtm.tm.logger.debug("%s:%s:Split key_makers k < max_km" % \
                    (self.gtm.d.name, self.p4_table.name))

                # when km are fully used, do not round-up bits to byte separately for
                # K and I
                num_byte_sel = len(self.combined_profile.k_byte_sel) + \
                                len(self.combined_profile.i1_byte_sel) + \
                                len(self.combined_profile.i2_byte_sel)
                num_bitB = (num_ibits + num_kbits + 7) / 8
                keep_free = (km0_free + km1_free - num_byte_sel - num_bitB) / 2
                ncc_assert(keep_free >= 0)

                # copy k bytes and bits to km0
                km0_profile.k_byte_sel += self.combined_profile.k_byte_sel
                km0_free -= len(self.combined_profile.k_byte_sel)
                ncc_assert(num_kbits <= max_km_bits )# XXX TBD
                km0_profile.k_bit_sel += self.combined_profile.k_bit_sel

                byte_avail = (self.num_km * max_kmB) - num_byte_sel
                byte_avail -= ((num_kbits+7)/8)

                if num_ibits > max_km_bits or (byte_avail < (num_ibits+7)/8):
                    if byte_avail < (num_ibits+7)/8:
                        # evenly splitting the bits may require 2B per km and that
                        # will exceed total available bytes, pack all bits as possible
                        km_free_bits = 0
                    else:
                        km_free_bits = (max_km_bits + max_km_bits - num_kbits - num_ibits)/2
                    km0_bits = max_km_bits - num_kbits - km_free_bits
                    km0_bits = min(km0_bits, num_ibits)
                    for b in range(km0_bits):
                        km0_profile.i_bit_sel.append(self.combined_profile.i_bit_sel[b])
                    km0_free -= ((km0_bits+num_kbits+7)/8)
                    for b in range(km0_bits, num_ibits):
                        km1_profile.i_bit_sel.append(self.combined_profile.i_bit_sel[b])
                    km1_free -= ((num_ibits-km0_bits+7)/8)
                else:
                    # move all i bits to km1
                    km1_profile.i_bit_sel += self.combined_profile.i_bit_sel
                    km1_free -= ((num_ibits+7)/8)
                    km0_free -= (num_kbits+7)/8

                km0_iB = km0_free-keep_free
                km1_iB = km1_free-keep_free
                # Try to avoid breaking up a field
                # Copy i1 data to km1 (as i2). A threshold is used so that enough
                # bytes are kept free for sharing key-makers in future.
                # if a field crosses the threshold, but fits within available space, allow it
                # Copy remaining i1 to km0 as i1 and do the same for i2 data bytes
                # NOTE: For P4+ this can cause different compilation output for common program and
                # P4 apps. As apps can define different headers and union them with generic headers
                # differences in header fld widths can cause this logic to pick different bytes for
                # different apps which cannot work
                if self.gtm.tm.be.args.p4_plus:
                    i = -1
                    # copy i1 bytes to km1 (as much as allowed)
                    # XXX check if we can avoid breaking up a field
                    for i in range(min(km1_iB, total_i1B)):
                        km1_profile.i2_byte_sel.append(self.combined_profile.i1_byte_sel[i])
                        km1_free -= 1
                        km1_iB -= 1

                    # rest of i1B to km0
                    for b in range(i+1, total_i1B):
                        km0_profile.i1_byte_sel.append(self.combined_profile.i1_byte_sel[b])
                        km0_free -= 1
                        km0_iB -= 1

                    i = -1
                    # copy i2B in remaining space in km1 and km0
                    for i in range(min(km1_iB, total_i2B)):
                        km1_profile.i2_byte_sel.append(self.combined_profile.i2_byte_sel[i])
                        km1_free -= 1
                        km1_iB -= 1

                    for b in range(i+1, total_i2B):
                        km0_profile.i2_byte_sel.append(self.combined_profile.i2_byte_sel[b])
                        km0_free -= 1
                        km0_iB -= 1
                else:
                    i1_fld_chunks = {} # {phc: sizeB}
                    i2_fld_chunks = {}
                    for cf in self.input_fields:
                        if (cf.phv_bit % 8):
                            continue
                        if cf.phv_bit/8 in self.combined_profile.i1_byte_sel:
                            if (cf.phv_bit/8) in i1_fld_chunks:
                                # keep the larger one (damn unions)
                                i1_fld_chunks[cf.phv_bit/8] = max(i1_fld_chunks[cf.phv_bit/8],
                                                                    (cf.width+7)/8)
                            else:
                                i1_fld_chunks[cf.phv_bit/8] = (cf.width+7)/8
                        elif cf.phv_bit/8 in self.combined_profile.i2_byte_sel:
                            if (cf.phv_bit/8) in i2_fld_chunks:
                                # keep the larger one (damn unions)
                                i2_fld_chunks[cf.phv_bit/8] = max(i2_fld_chunks[cf.phv_bit/8],
                                                                    (cf.width+7)/8)
                            else:
                                i2_fld_chunks[cf.phv_bit/8] = (cf.width+7)/8
                        else:
                            pass
                    iB = 0
                    i1B_remain = total_i1B
                    while i1B_remain:
                        if not km1_free:
                            break
                        b = self.combined_profile.i1_byte_sel[iB]
                        if b in i1_fld_chunks:
                            if not km1_iB:
                                # already crossed the threshold
                                break
                            # make sure entire fld fits
                            w = i1_fld_chunks[b]
                            if w > km1_iB:
                                if w > km1_free:
                                    # need to break it
                                    break
                            for _ in range(w):
                                b = self.combined_profile.i1_byte_sel[iB]
                                km1_profile.i2_byte_sel.append(b)
                                i1B_remain -= 1
                                km1_free -= 1
                                iB += 1
                                if km1_iB > 0:
                                    km1_iB -= 1
                        else:
                            # these are fields that are not byte aligned..
                            # just place in in km if below threshold
                            if not km1_iB:
                                break
                            b = self.combined_profile.i1_byte_sel[iB]
                            km1_profile.i2_byte_sel.append(b)
                            i1B_remain -= 1
                            km1_free -= 1
                            iB += 1
                            if km1_iB > 0:
                                km1_iB -= 1

                    # copy the rest in km0
                    while i1B_remain:
                        b = self.combined_profile.i1_byte_sel[iB]
                        if b in i1_fld_chunks:
                            if w > km0_free:
                                if w < km1_free:
                                    # does not fit in km0 but can in km1
                                    for _ in range(w):
                                        b = self.combined_profile.i1_byte_sel[iB]
                                        km1_profile.i2_byte_sel.append(b)
                                        i1B_remain -= 1
                                        km1_free -= 1
                                        iB += 1
                                        if km1_iB > 0:
                                            km1_iB -= 1
                                    continue
                        # partial fld or cannot fit in km1... just add to km0 as bytes
                        if not km0_free:
                            break
                        km0_profile.i1_byte_sel.append(b)
                        i1B_remain -= 1
                        km0_free -= 1
                        if km0_iB > 0:
                            km0_iB -= 1
                        iB += 1

                    ncc_assert(i1B_remain <= km1_free)
                    while i1B_remain:
                        b = self.combined_profile.i1_byte_sel[iB]
                        km1_profile.i2_byte_sel.append(b)
                        i1B_remain -= 1
                        km1_free -= 1
                        if km1_iB > 0:
                            km1_iB -= 1
                        iB += 1

                    ncc_assert(i1B_remain == 0)
                    iB = 0
                    i2B_remain = total_i2B
                    while i2B_remain:
                        if not km1_free:
                            break
                        b = self.combined_profile.i2_byte_sel[iB]
                        if b in i2_fld_chunks:
                            if not km1_iB:
                                # already crossed the threshold
                                break
                            # make sure entire fld fits
                            w = i2_fld_chunks[b]
                            if w > km1_iB:
                                if w > km1_free:
                                    # need to break it
                                    break
                            for _ in range(w):
                                b = self.combined_profile.i2_byte_sel[iB]
                                km1_profile.i2_byte_sel.append(b)
                                i2B_remain -= 1
                                km1_free -= 1
                                iB += 1
                                if km1_iB > 0:
                                    km1_iB -= 1
                        else:
                            # these are partial fields (can it happen??)
                            # just place in in km if below threshold
                            if not km1_iB:
                                break
                            b = self.combined_profile.i2_byte_sel[iB]
                            km1_profile.i2_byte_sel.append(b)
                            i2B_remain -= 1
                            km1_free -= 1
                            iB += 1
                            if km1_iB > 0:
                                km1_iB -= 1

                    # copy the rest in km0
                    while i2B_remain:
                        b = self.combined_profile.i2_byte_sel[iB]
                        if b in i2_fld_chunks:
                            if w > km0_free:
                                if w < km1_free:
                                    # does not fit in km0 but can in km1
                                    for _ in range(w):
                                        b = self.combined_profile.i2_byte_sel[iB]
                                        km1_profile.i2_byte_sel.append(b)
                                        i2B_remain -= 1
                                        km1_free -= 1
                                        iB += 1
                                        if km1_iB > 0:
                                            km1_iB -= 1
                                    continue
                        # if cannot move the fld to km1.. default is km0 now
                        if not km0_free:
                            break
                        km0_profile.i2_byte_sel.append(b)
                        i2B_remain -= 1
                        km0_free -= 1
                        if km0_iB > 0:
                            km0_iB -= 1
                        iB += 1

                    ncc_assert(i2B_remain <= km1_free)
                    while i2B_remain:
                        b = self.combined_profile.i2_byte_sel[iB]
                        km1_profile.i2_byte_sel.append(b)
                        i2B_remain -= 1
                        km1_free -= 1
                        if km1_iB > 0:
                            km1_iB -= 1
                        iB += 1

                    ncc_assert(i2B_remain == 0)
                    ncc_assert(km1_free >=0 and km0_free >= 0)

                km0_profile.i1_byte_sel = sorted(km0_profile.i1_byte_sel)
                km0_profile.i2_byte_sel = sorted(km0_profile.i2_byte_sel)
                km1_profile.i2_byte_sel = sorted(km1_profile.i2_byte_sel)

                km0_profile.byte_sel = (km0_profile.i1_byte_sel + km0_profile.k_byte_sel + \
                    km0_profile.i2_byte_sel)
                km0_profile.bit_sel = km0_profile.k_bit_sel + km0_profile.i_bit_sel
                km1_profile.byte_sel = (km1_profile.i1_byte_sel + km1_profile.k_byte_sel + \
                    km1_profile.i2_byte_sel)
                km1_profile.bit_sel = km1_profile.k_bit_sel + km1_profile.i_bit_sel

                km0.combined_profile = km0_profile
                km1.combined_profile = km1_profile
                km0_profile._update_bit_loc_key_off()
                km1_profile._update_bit_loc_key_off()

            # common code for split profiles
            # copy new profiles into flits for future use
            for fid,kf in enumerate(self.km_flits):
                km0_flit_profile = capri_km_profile(self.gtm)
                km1_flit_profile = capri_km_profile(self.gtm)
                for b in kf.km_profile.k_byte_sel:
                    if b in km0_profile.byte_sel:
                        km0_flit_profile.k_byte_sel.append(b)
                    if b in km1_profile.byte_sel:
                        km1_flit_profile.k_byte_sel.append(b)

                for b in (kf.km_profile.i1_byte_sel + kf.km_profile.i2_byte_sel):
                    if b in km0_profile.i1_byte_sel:
                        km0_flit_profile.i1_byte_sel.append(b)
                    elif b in km0_profile.i2_byte_sel:
                        km0_flit_profile.i2_byte_sel.append(b)
                    elif b in km1_profile.i1_byte_sel:
                        km1_flit_profile.i1_byte_sel.append(b)
                    elif b in km1_profile.i2_byte_sel:
                        km1_flit_profile.i2_byte_sel.append(b)
                    else:
                        ncc_assert(0)

                for b in kf.km_profile.k_bit_sel:
                    if b in km0_profile.bit_sel:
                        km0_flit_profile.k_bit_sel.append(b)
                    if b in km1_profile.k_bit_sel:
                        km1_flit_profile.k_bit_sel.append(b)

                for b in kf.km_profile.i_bit_sel:
                    if b in km0_profile.i_bit_sel:
                        km0_flit_profile.i_bit_sel.append(b)
                    if b in km1_profile.i_bit_sel:
                        km1_flit_profile.i_bit_sel.append(b)

                km0_flit_profile.byte_sel = km0_flit_profile.i1_byte_sel + \
                    km0_flit_profile.k_byte_sel + km0_flit_profile.i2_byte_sel
                km0_flit_profile.bit_sel = km0_flit_profile.k_bit_sel + \
                    km0_flit_profile.i_bit_sel

                km1_flit_profile.byte_sel = km1_flit_profile.i1_byte_sel + \
                    km1_flit_profile.k_byte_sel + km1_flit_profile.i2_byte_sel
                km1_flit_profile.bit_sel = km1_flit_profile.k_bit_sel + \
                    km1_flit_profile.i_bit_sel

                km0.flit_km_profiles[fid] = km0_flit_profile
                km1.flit_km_profiles[fid] = km1_flit_profile
        km0_start_fid = -1; km1_start_fid = -1
        for fid in range(self.flits_used[0], self.flits_used[-1]+1):
            if km0_start_fid >= 0 or km0.flit_km_profiles[fid].km_prof_size():
                km0.flits_used.append(fid)
                km0_start_fid = fid
            if km1_start_fid >= 0 or km1.flit_km_profiles[fid].km_prof_size():
                km1.flits_used.append(fid)
                km1_start_fid = fid

        # validate that all bytes are covered
        total_k_bytes = sum([len(km.combined_profile.k_byte_sel) for km in self.key_makers])
        total_i1_bytes = sum([len(km.combined_profile.i1_byte_sel) for km in self.key_makers])
        total_i2_bytes = sum([len(km.combined_profile.i2_byte_sel) for km in self.key_makers])
        total_i_bytes = total_i1_bytes + total_i2_bytes
        total_k_bits = sum([len(km.combined_profile.k_bit_sel) for km in self.key_makers])
        total_i_bits = sum([len(km.combined_profile.i_bit_sel) for km in self.key_makers])
        ncc_assert(total_k_bytes == len(self.combined_profile.k_byte_sel))
        ncc_assert(total_i_bytes == len(self.combined_profile.i1_byte_sel) + \
                                len(self.combined_profile.i2_byte_sel))
        ncc_assert(total_k_bits == len(self.combined_profile.k_bit_sel))
        ncc_assert(total_i_bits == len(self.combined_profile.i_bit_sel))

        self.gtm.tm.logger.debug("%s:%s:Split Profiles km0:%s, km1%s" % \
                    (self.gtm.d.name, self.p4_table.name, km0_profile, km1_profile))

    def ct_update_wide_key_size(self):
        if self.is_toeplitz_hash():
            return
        max_km_width = self.gtm.tm.be.hw_model['match_action']['key_maker_width']
        wide_key_km_width = 2 * max_km_width
        # for wide key table, key start/end offsets are valid only for the last flit
        # To get correct key size, keep start_offset to 0 for now and compute real-start
        # address based on last flit and action_pc/collision idx etc while programming
        # the hw
        ncc_assert(self.start_key_off == 0)

        # final key_size is computed as (n-1)*512 + end_key_off, where n=flits used
        self.final_key_size = ((len(self.flits_used)-1) * wide_key_km_width) + \
                        self.last_flit_end_key_off - self.last_flit_start_key_off
        ncc_assert(self.final_key_size >= self.key_size)

    def ct_tcam_get_key_end_bytes(self):
        # get first and last k bytes and bits from key makers
        ncc_assert(self.num_km)
        _km = self.key_makers[0]
        km0 = _km.shared_km if _km.shared_km else _km
        km1 = None
        if self.num_km > 1:
            _km = self.key_makers[-1]
            km1 = _km.shared_km if _km.shared_km else _km


        # for tcams, when km_profiles are shared and common bytes are removed
        # due to that it is possible that first and last k bytes are now different
        # than what is in table's combined profile
        t_fk_byte = -1; t_lk_byte = -1
        if len(self.combined_profile.k_byte_sel):
            for b in km0.combined_profile.byte_sel:
                if b in self.combined_profile.k_byte_sel:
                    t_fk_byte = b
                    break
            if t_fk_byte == -1:
                for b in km1.combined_profile.byte_sel:
                    if b in self.combined_profile.k_byte_sel:
                        t_fk_byte = b
                        break
            ncc_assert(t_fk_byte != -1)

            if km1 != None:
                for b in reversed(km1.combined_profile.byte_sel):
                    if b in self.combined_profile.k_byte_sel:
                        t_lk_byte = b
                        break
            if t_lk_byte == -1:
                for b in reversed(km0.combined_profile.byte_sel):
                    if b in self.combined_profile.k_byte_sel:
                        t_lk_byte = b
                        break
            ncc_assert(t_lk_byte != -1)

        t_fk_bit = -1; t_lk_bit = -1
        if len(self.combined_profile.k_bit_sel):
            for b in km0.combined_profile.bit_sel:
                if b in self.combined_profile.k_bit_sel:
                    t_fk_bit = b
                    break
            if t_fk_bit == -1:
                for b in km1.combined_profile.bit_sel:
                    if b in self.combined_profile.k_bit_sel:
                        t_fk_bit = b
                        break
            ncc_assert(t_fk_bit != -1)
            if km1 != None:
                for b in reversed(km1.combined_profile.bit_sel):
                    if b in self.combined_profile.k_bit_sel:
                        t_lk_bit = b
                        break
            if t_lk_bit == -1:
                for b in reversed(km0.combined_profile.bit_sel):
                    if b in self.combined_profile.k_bit_sel:
                        t_lk_bit = b
                        break
            ncc_assert(t_lk_bit != -1)

        self.gtm.tm.logger.debug("%s:TCAM fk_byte %d lk_byte %d, bits %d, %d" % \
                        (self.p4_table.name, t_fk_byte, t_lk_byte, t_fk_bit, t_lk_bit))
        return (t_fk_byte, t_lk_byte, t_fk_bit, t_lk_bit)

    def ct_update_key_offsets(self):
        # XXX for tables that do not share key-makers this is already computed - merge the two
        # methods ....
        if self.key_phv_size == 0:
            return

        if self.is_overflow and not self.is_otcam:
            # for overflow hash table, fix the key_offset with the parent hash table
            return

        if self.is_toeplitz_hash():
            return

        # Re-init the offsets since this function can be called multiple times
        self.start_key_off = -1; self.end_key_off = -1;
        max_km_width = self.gtm.tm.be.hw_model['match_action']['key_maker_width']
        max_kmB = max_km_width/8

        # for wide_key (>512b), key must start in first km and end in the last km
        # i1 data is not allowed, nor any data with key in intermediate flits
        # i data is only allowed in the last flit
        fk_byte = -1; lk_byte = -1
        if len(self.combined_profile.k_byte_sel):
            fk_byte = self.combined_profile.k_byte_sel[0]
            lk_byte = self.combined_profile.k_byte_sel[-1]

        fk_bit = -1; lk_bit = -1
        if len(self.combined_profile.k_bit_sel):
            fk_bit = self.combined_profile.k_bit_sel[0]
            lk_bit = self.combined_profile.k_bit_sel[-1]

        ncc_assert(self.num_km)
        km0 = None; km1 = None
        _km0 = None; _km1 = None
        if self.is_wide_key:
            # take first and last key byte from last flit
            km0 = self.key_makers[-2]
            _km0 = km0
            km1 = self.key_makers[-1]
            _km1 = km1
            last_fid = self.flits_used[-1]
            if len(self.km_flits[last_fid].km_profile.k_byte_sel):
                fk_byte = self.km_flits[last_fid].km_profile.k_byte_sel[0]
            else:
                fk_byte = -1
            if len(self.km_flits[last_fid].km_profile.k_byte_sel):
                lk_byte = self.km_flits[last_fid].km_profile.k_byte_sel[-1]
            else:
                lk_byte = -1
        else:
            _km0 = self.key_makers[0]
            km0 = _km0.shared_km if _km0.shared_km else _km0
            km1 = None
            if self.num_km > 1:
                _km1 = self.key_makers[-1]
                km1 = _km1.shared_km if _km1.shared_km else _km1

        if self.is_tcam_table():
            fk_byte, lk_byte, fk_bit, lk_bit = self.ct_tcam_get_key_end_bytes()

        # compute start offset
        # since key makers are combined, bits may appear before/after/middle of a key
        # when key-makers are shared same k-bytes may appear in multiple kms. In that case
        # use this tables logical key-maker/profile to decide the km to use for key size
        # calculations
        fk_idx = max_kmB
        if fk_byte > 0 and fk_byte in km0.combined_profile.byte_sel:
            fk_idx = km0.combined_profile.byte_sel.index(fk_byte)
            self.start_key_off = (fk_idx * 8) + self.start_key_off_delta

        if fk_bit > 0 and fk_bit in km0.combined_profile.bit_sel:
            if km0.combined_profile.bit_loc < fk_idx:
                bit_idx = km0.combined_profile.bit_sel.index(fk_bit)
                self.start_key_off = (km0.combined_profile.bit_loc*8) + \
                        self.start_key_off_delta + bit_idx
                self.gtm.tm.logger.debug("%s:Using km0 bit_location %d as k-start" % \
                        (self.p4_table.name, self.start_key_off))

        if self.start_key_off == -1:
            fk_idx = max_kmB
            ncc_assert(km1)
            if fk_byte > 0 and fk_byte in km1.combined_profile.byte_sel:
                fk_idx = km1.combined_profile.byte_sel.index(fk_byte)
                self.start_key_off = (fk_idx * 8) + self.start_key_off_delta + max_km_width
            if km1 and fk_bit > 0 and fk_bit in km1.combined_profile.bit_sel:
                if km1.combined_profile.bit_loc < fk_idx:
                    bit_idx = km1.combined_profile.bit_sel.index(fk_bit)
                    self.start_key_off = (km1.combined_profile.bit_loc*8) + \
                        self.start_key_off_delta + bit_index
                    self.gtm.tm.logger.debug("%s:Using km1 bit_location %d as k-start" % \
                        (self.p4_table.name, self.start_key_off))

        ncc_assert(self.start_key_off != -1)

        # compute end offset
        # check km1 first
        lk_end_byte = -1
        if km1:
            if lk_bit > 0 and lk_bit in km1.combined_profile.bit_sel:
                lk_bit_idx = km1.combined_profile.bit_sel.index(lk_bit)
                if lk_bit_idx < 8:
                    self.end_key_off = (km1.combined_profile.bit_loc*8) + lk_bit_idx + 1 + max_km_width
                    lk_end_byte = km1.combined_profile.bit_loc
                else:
                    self.end_key_off = (km1.combined_profile.bit_loc1*8) + (lk_bit_idx % 8) + 1 + max_km_width
                    lk_end_byte = km1.combined_profile.bit_loc1

            if lk_byte > 0 and lk_byte in km1.combined_profile.byte_sel:
                lk_idx = km1.combined_profile.byte_sel.index(lk_byte)
                if lk_idx > lk_end_byte:
                    self.end_key_off = (lk_idx * 8) + 8 + max_km_width - self.end_key_off_delta
                    self.gtm.tm.logger.debug("%s:Using km1 byte_location %d as k-end" % \
                            (self.p4_table.name, self.end_key_off))

        lk_end_byte = -1
        if self.end_key_off == -1:
            if lk_bit > 0 and lk_bit in km0.combined_profile.bit_sel:
                lk_bit_idx = km0.combined_profile.bit_sel.index(lk_bit)
                if lk_bit_idx < 8:
                    self.end_key_off = (km0.combined_profile.bit_loc*8) + lk_bit_idx + 1
                    lk_end_byte = km0.combined_profile.bit_loc
                else:
                    self.end_key_off = (km0.combined_profile.bit_loc1*8) + (lk_bit_idx%8) + 1
                    lk_end_byte = km0.combined_profile.bit_loc1
            if lk_byte > 0 and lk_byte in km0.combined_profile.byte_sel:
                lk_idx = km0.combined_profile.byte_sel.index(lk_byte)
                if lk_idx > lk_end_byte:
                    self.end_key_off = (lk_idx * 8) + 8 - self.end_key_off_delta
                    self.gtm.tm.logger.debug("%s:Using km0 byte_location %d as k-end" % \
                            (self.p4_table.name, self.end_key_off))
        ncc_assert(self.end_key_off != -1)
        # For index and hash tables that use multiple key makers,
        # if key is split between the two kms and if
        # km0 is not fully used, right justify the key
        if (self.start_key_off / max_km_width) != (self.end_key_off / max_km_width) and \
            not self.is_wide_key and not self.is_tcam_table():
            # key start and end in different key-makers
            if len(km0.combined_profile.byte_sel) < max_kmB:
                front_pad = max_kmB - len(km0.combined_profile.byte_sel)
                for i in range(front_pad):
                    km0.combined_profile.byte_sel.insert(0, -1)
                self.gtm.tm.logger.debug("%s:Add front pad of %d to km_profile" % \
                    (self.p4_table.name, front_pad))
                self.start_key_off += (front_pad * 8)

        self.gtm.tm.logger.debug("%s:start_key_off %d end_key_off %d, key_size %d" % \
            (self.p4_table.name, self.start_key_off, self.end_key_off,
             self.end_key_off-self.start_key_off))

        if self.is_wide_key:
            new_key_size = (self.num_km-2) * max_km_width
            new_key_size += (self.end_key_off - self.start_key_off)
            self.final_key_size = new_key_size
            self.last_flit_start_key_off = self.start_key_off
            self.start_key_off = 0
            self.last_flit_end_key_off = self.end_key_off
            self.end_key_off = new_key_size + self.last_flit_start_key_off
            if self.collision_ct:
                self.collision_ct.last_flit_start_key_off = self.last_flit_start_key_off
                self.collision_ct.last_flit_end_key_off = self.last_flit_end_key_off
                self.collision_ct.start_key_off = self.start_key_off
                self.collision_ct.end_key_off = self.end_key_off
        else:
            new_key_size = self.end_key_off - self.start_key_off
            self.final_key_size = new_key_size

        if self.collision_ct:
            # XXX this does not work for wide key -- need to fix
            # overflow hash table - use same key-maker/profiles of the parent hash table
            # collision table index key is added (earlier by caller) into the parent table's km
            # Only check that the overflow index are the very first bytes in the km of the parent
            # If not... just ncc_assert(for now.. can try to move around bytes in km XXX??)
            cf = self.collision_ct.keys[0][0]
            collision_key_size = self.gtm.tm.be.hw_model['match_action']['collision_index_sz']
            ncc_assert(cf.width == collision_key_size)
            ncc_assert((cf.phv_bit % 8) == 0)
            phvB = cf.phv_bit / 8
            if self.is_wide_key:
                km0 = self.key_makers[-2]
            else:
                km0 = self.key_makers[0].shared_km if self.key_makers[0].shared_km else \
                        self.key_makers[0]

            km_prof = km0.combined_profile

            for i in range(collision_key_size/8):
                ncc_assert(km_prof.byte_sel[i] == phvB + i, \
                    "incorrect placement of the overflow key")
            self.collision_ct.start_key_off = self.start_key_off
            self.collision_ct.end_key_off = self.end_key_off
            self.collision_ct.final_key_size = self.final_key_size
            return

    def _fix_hbm_hash_table_km_profile(self):
        # XXX HACK: Now it is done for non-hbm tables as well, since hw has new config
        # this can be handled differently, need api changes to go with it
        if self.is_overflow:
            return

        action_id_size = self.gtm.tm.be.hw_model['match_action']['action_id_size']
        max_km_width = self.gtm.tm.be.hw_model['match_action']['key_maker_width']
        max_kmB = max_km_width/8
        start_km = self.start_key_off / max_km_width
        km_prof = None
        shared_km = False
        if self.key_makers[start_km].shared_km:
            km_prof = self.key_makers[start_km].shared_km.combined_profile
            shared_km = True
        else:
            km_prof = self.key_makers[start_km].combined_profile

        if self.num_actions() > 1 and self.start_key_off < action_id_size:
            if shared_km:
                # XXX need to fix all shared tables.. if shared table is an idx table, more checks
                # so far does not run into it
                ncc_assert(0, "need test case")
            else:
                ncc_assert(len(km_prof.byte_sel) < max_kmB)
                for _ in range((action_id_size+7)/8):
                    km_prof.byte_sel.insert(0, -1)
                    self.start_key_off += 8
                    self.end_key_off += 8
                    if km_prof.bit_loc >= 0:
                        km_prof.bit_loc += 1
                    if km_prof.bit_loc1 >= 0:
                        km_prof.bit_loc1 += 1

        if self.otcam_ct and (self.start_key_off % 16):
            # if key maker is full - need to move things around.. XXX
            # this does not happen so far, but needs to be done
            ncc_assert(len(km_prof.byte_sel) < max_kmB)
            km_prof.byte_sel.insert(0, -1)
            self.start_key_off += 8
            self.end_key_off += 8
            if km_prof.bit_loc >= 0:
                km_prof.bit_loc += 1
            if km_prof.bit_loc1 >= 0:
                km_prof.bit_loc1 += 1

    def remove_i_in_k(self):
        if len(self.i2k_pad_chunks) == 0:
            return False

        max_km_width = self.gtm.tm.be.hw_model['match_action']['key_maker_width']
        max_kmB = max_km_width/8
        # hw need tcam key to start at 2B boundary to use AXI shift for aligning the key
        k_start = self.start_key_off / 8
        key_szB = (self.final_key_size + 7)/8
        # for tcam do not use start key_off_delta, just include non-k bits into k and mask them
        # this is for axi shift adjustment
        if self.start_key_off_delta:
            self.gtm.tm.logger.debug("%s:Reset start_key_off_delta(%d) for tcam table" % \
                (self.p4_table.name, self.start_key_off_delta))
            self.start_key_off_delta = 0

        if self.is_otcam:
            # This needs to be fixed by fixing its parent hash table key - XXX
            ncc_assert((k_start % 2) == 0)

        km0_prof = self.key_makers[0].shared_km.combined_profile \
            if self.key_makers[0].shared_km else \
            self.key_makers[0].combined_profile

        km1_prof = None
        if self.num_km > 1:
            km1_prof = self.key_makers[1].shared_km.combined_profile \
                        if self.key_makers[1].shared_km \
                        else self.key_makers[1].combined_profile

        fix_km_prof = None
        k_end = (self.end_key_off+7) / 8
        is_shared_km = False
        shared_keys = {}
        shared_tables = []
        if k_start < max_kmB:
            fix_km_prof = km0_prof
            is_shared_km = self.key_makers[0].shared_km != None
            if is_shared_km:
                shared_tables = self.key_makers[0].shared_km.ctables
        else:
            fix_km_prof = km1_prof
            is_shared_km = self.key_makers[1].shared_km != None
            if is_shared_km:
                shared_tables = self.key_makers[1].shared_km.ctables

        for sct in shared_tables:
            if sct == self:
                continue
            for b in sct.combined_profile.k_byte_sel:
                shared_keys[b] = True

        km_kstart = k_start % max_kmB
        km_kend = k_end % max_kmB

        if len(self.i2k_pad_chunks):
            # ignore Banyon and remove i2k chunk out of key and place them rigth after the key
            self.gtm.tm.logger.debug("%s: Remove i-chunks mixed with key for table %s:%s" % \
                                        (self.gtm.d.name, self.p4_table.name, self.i2k_pad_chunks))
            ibytes_in_k = []
            for cstart, clen in self.i2k_pad_chunks:
                ncc_assert(cstart % 8 == 0)
                ibyte = cstart / 8
                for b in range(clen/8):
                    if b in shared_keys:
                        continue
                    ibytes_in_k.append(ibyte+b)

            # if key spans two kms, any i-bytes removed from km0 should be added before the key
            # (i.e shift key to right). If i-bytes are removed from km1, add them after
            # the key. When keys are contained in one km, move i-bytes after the key
            one_km = True if k_start/max_kmB == k_end/max_kmB else False
            moved_bits = 0
            if one_km:
                # list is reversred so that bytes get re-inserted in ascending order 
                ibytes_in_k.sort(reverse=True)
                update_bit_loc = True if (fix_km_prof.bit_loc >= km_kstart and \
                                         fix_km_prof.bit_loc <= km_kend) \
                                      else False
                update_bit_loc1 = True if (fix_km_prof.bit_loc1 >= km_kstart and \
                                           fix_km_prof.bit_loc1 <= km_kend) \
                                       else False
                for b in ibytes_in_k:
                    if not is_shared_km:
                        ncc_assert(b in fix_km_prof.k_byte_sel)
                        fix_km_prof.k_byte_sel.remove(b)
                        fix_km_prof.i2_byte_sel.append(b)
                    rm_index = fix_km_prof.byte_sel.index(b)
                    fix_km_prof.byte_sel.insert(km_kend, b)
                    fix_km_prof.byte_sel.remove(b)
                    moved_bits += 8
                    k_end -= 1
                    km_kend -= 1
                    self.end_key_off -= 8
                    # update bit locations
                    if update_bit_loc and rm_index < fix_km_prof.bit_loc:
                        fix_km_prof.bit_loc -= 1
                    if update_bit_loc1 and rm_index < fix_km_prof.bit_loc1:
                        fix_km_prof.bit_loc1 -= 1
            else:
                ibytes_in_k.sort()
                km0_i2k_bytes = []
                km1_i2k_bytes = []
                for b in ibytes_in_k:
                    if b in km0_prof.k_byte_sel:
                        km0_i2k_bytes.append(b)
                    elif b in km1_prof.k_byte_sel:
                        km1_i2k_bytes.append(b)
                    else:
                        ncc_assert(0)
            
                if len(km0_i2k_bytes) % 2:
                    # keep 1 byte in key so that start ofset stays on 2B alignment
                    use_byte = None
                    for cs, cl in self.i2k_pad_chunks:
                        # ??? not sure what is done here cl == 1 does not make sense
                        if cl == 1 and (cs/8) in km0_i2k_bytes:
                            use_byte = cs / 8
                            break
                    if use_byte != None:
                        km0_i2k_bytes.remove(use_byte)
                        ibytes_in_k.remove(use_byte)
                    else:
                        b = km0_i2k_bytes.pop()
                        ibytes_in_k.remove(b)

                for b in km0_i2k_bytes:
                    rm_index = km0_prof.byte_sel.index(b)
                    km0_prof.byte_sel.remove(b)
                    km0_prof.k_byte_sel.remove(b)
                    km0_prof.i1_byte_sel.append(b)
                    km0_prof.byte_sel.insert(km_kstart, b)
                    moved_bits += 8
                    k_start += 1
                    km_start += 1
                    self.start_key_off += 8
                    bit_loc = km0_prof.bit_loc
                    bit_loc1 = km0_prof.bit_loc1
                    if bit_loc >= km_kstart and bit_loc < rm_index:
                        km0_prof.bit_loc += 1
                    if bit_loc1 >= km_kstart and bit_loc1 < rm_index:
                        km0_prof.bit_loc1 += 1

                for b in km1_i2k_bytes.reverse():
                    ncc_assert(b in fix_km_prof.k_byte_sel)
                    rm_index = km1_prof.byte_sel.index(b)
                    km1_prof.k_byte_sel.remove(b)
                    km1_prof.i2_byte_sel.append(b)
                    km1.byte_sel.insert(km_kend, b)
                    km1.byte_sel.remove(b)
                    moved_bits += 8
                    k_end -= 1
                    km_kend -= 1
                    self.end_key_off -= 8
                    bit_loc = km1_prof.bit_loc
                    bit_loc1 = km1_prof.bit_loc1
                    if bit_loc <= km_kend and bit_loc > rm_index:
                        km1_prof.bit_loc -= 1
                    if bit_loc1 <= km_kstart and bit_lo > rm_index:
                        km1_prof.bit_loc1 -= 1

            self.key_size -= moved_bits
            self.key_phv_size -= moved_bits
            self.final_key_size -= moved_bits
            self.i_size -= moved_bits
            self.i_phv_size -= moved_bits
            # adjust combined profiles for all shared tables
            if self not in shared_tables:
                shared_tables.append(self)
            for sct in shared_tables:
                for b in ibytes_in_k:
                    if b in sct.combined_profile.byte_sel:
                        sct.combined_profile.byte_sel.remove(b)
                    if b in sct.combined_profile.k_byte_sel:
                        sct.combined_profile.k_byte_sel.remove(b)
            return True
        return False

    def _fix_tcam_table_key(self):
        max_km_width = self.gtm.tm.be.hw_model['match_action']['key_maker_width']
        max_kmB = max_km_width/8
        # hw need tcam key to start at 2B boundary to use AXI shift for aligning the key
        k_start = self.start_key_off / 8
        key_szB = (self.final_key_size + 7)/8
        # for tcam do not use start key_off_delta, just include non-k bits into k and mask them
        # this is for axi shift adjustment
        if self.start_key_off_delta:
            self.gtm.tm.logger.debug("%s:Reset start_key_off_delta(%d) for tcam table" % \
                (self.p4_table.name, self.start_key_off_delta))
            self.start_key_off_delta = 0

        if self.is_otcam:
            # This needs to be fixed by fixing its parent hash table key - XXX
            ncc_assert((k_start % 2) == 0)

        km0_prof = self.key_makers[0].shared_km.combined_profile \
            if self.key_makers[0].shared_km else \
            self.key_makers[0].combined_profile

        km1_prof = None
        if self.num_km > 1:
            km1_prof = self.key_makers[1].shared_km.combined_profile \
                        if self.key_makers[1].shared_km \
                        else self.key_makers[1].combined_profile

        fix_km_prof = None
        k_end = self.end_key_off / 8
        is_shared_km = False
        if k_start < max_kmB:
            fix_km_prof = km0_prof
            is_shared_km = self.key_makers[0].shared_km != None
        else:
            fix_km_prof = km1_prof
            is_shared_km = self.key_makers[1].shared_km != None

        km_kstart = k_start % max_kmB
        km_kend = k_end % max_kmB

        if (k_start % 2) == 0:
            return False


        self.gtm.tm.logger.debug("_fix_tcam_table_key:%s: key_offs %d, %d bit_loc %d, %d" % \
            (self.p4_table.name, self.start_key_off, self.end_key_off,
            fix_km_prof.bit_loc, fix_km_prof.bit_loc1))

        if len(fix_km_prof.byte_sel) < max_kmB:
            if is_shared_km:
                # since km is shared, don't insert bytes as it can un-align the shared table
                # increase the key size by 8 bits (hopefully we have space in TCAM)
                self.start_key_off -= 8
                self.final_key_size += 8
                return False
            # add a byte just before the key
            # If this km is shared with index table AND idx table shares key start with tcam
            # key, it can violate the key alignment req.t for idx table XXX
            fix_km_prof.byte_sel.insert(km_kstart, -1)
            self.start_key_off += 8
            self.end_key_off += 8
            if fix_km_prof.bit_loc > km_kstart:
                fix_km_prof.bit_loc += 1
            if fix_km_prof.bit_loc1 > km_kstart:
                fix_km_prof.bit_loc1 += 1
            self.gtm.tm.logger.debug( \
                "_fix_tcam_table_key:%s:EXTRA-BYTE: key_offs %d, %d bit_loc %d, %d" % \
                (self.p4_table.name, self.start_key_off, self.end_key_off,
                fix_km_prof.bit_loc, fix_km_prof.bit_loc1))
            return False
        else:
            # key maker is full
            # if the kstart bytes of tcam are shared with another index
            # table, moving/inserting byte after km_kstart can cause problems for the
            # index table
            # just increase tcam key width by 1 byte by moving start_key off back by 1 byte
            if is_shared_km:
                # since km is shared, don't insert bytes as it can un-align the shared table
                # increase the key size by 8 bits (hopefully we have space in TCAM)
                self.start_key_off -= 8
                self.final_key_size += 8
                return False
            i = km_kstart
            for i in range(km_kstart):
                if i == fix_km_prof.bit_loc or i == fix_km_prof.bit_loc1:
                    continue
                if fix_km_prof.byte_sel[i] < 0:
                    # unused bytes are inserted in tcam's km to reduce the key size by
                    # right justifying key in km0 so that it stays close to k-bytes in km1
                    # move an unused byte before key to after key
                    fix_km_prof.byte_sel.insert(km_kstart+1, -1)
                    fix_km_prof.byte_sel.pop(i)
                    break
            if i < km_kstart:
                # just move the start key offset back and let p4pd treat the
                # leading bytes as non-key and mask it
                self.start_key_off -= 8
                self.final_key_size += 8
                # don't change end off unless it is same as start
                if k_start == k_end:
                    self.end_key_off -= 8
                    self.final_key_size -= 8
                if fix_km_prof.bit_loc > i and fix_km_prof.bit_loc < km_kstart:
                    fix_km_prof.bit_loc -= 1
                if fix_km_prof.bit_loc1 > i and fix_km_prof.bit_loc1 < km_kstart:
                    fix_km_prof.bit_loc1 -= 1
                return False

            ncc_assert(0, "need a test case")
            for i in range(km_kstart, max_kmB):
                if i == fix_km_prof.bit_loc or i == fix_km_prof.bit_loc1:
                    continue
                if fix_km_prof.byte_sel[i] < 0:
                    fix_km_prof.byte_sel.insert(km_kstart,-1) # add to front
                    fix_km_prof.byte_sel.pop(i)
                    break
            ncc_assert(i < max_kmB )# cannot find free byte for alignment
            if fix_km_prof.bit_loc < i and fix_km_prof.bit_loc > km_kstart:
                fix_km_prof.bit_loc += 1
            if fix_km_prof.bit_loc1 < i and fix_km_prof.bit_loc1 > km_kstart:
                fix_km_prof.bit_loc1 += 1
            # complicated start/end offset manipulation.. let it be done by caller
            return True

    def _fix_tcam_table_km_profile(self):
        def _fix_bit_loc(fix_km_prof, max_kmB, right_justify):
            # Internally called from this function (at the end)
            # since this km_prof is not shared with anyone, arrange the bytes and bits
            # as needed, fix the bit_loc and start/end_key_off
            # after removing duplicates the km structure is -
            # | i1_bytes | k_bytes | ki_bits | i2_bytes |
            # right justify the bytes
            # arrange it as -
            # | ...[pad]....i1_bytes | k_bytes | ki_bits | i2_bytes |
            num_bytes = 0
            if len(fix_km_prof.bit_sel):
                num_bytes = (len(fix_km_prof.bit_sel) + 7) / 8
                if len(fix_km_prof.k_byte_sel):
                    bit_loc = fix_km_prof.byte_sel.index(fix_km_prof.k_byte_sel[-1]) + 1
                elif len(fix_km_prof.i1_byte_sel):
                    bit_loc = fix_km_prof.byte_sel.index(fix_km_prof.i1_byte_sel[-1]) + 1
                elif len(fix_km_prof.i2_byte_sel):
                    bit_loc = fix_km_prof.byte_sel.index(fix_km_prof.i2_byte_sel[0])
                else:
                    if right_justify:
                        bit_loc = max_kmB-1
                    else:
                        bit_loc = 0

                fix_km_prof.bit_loc = bit_loc
                if num_bytes > 1:
                    if bit_loc == max_kmB-1:
                        fix_km_prof.bit_loc1 = bit_loc
                        fix_km_prof.bit_loc = bit_loc - 1
                    else:
                        fix_km_prof.bit_loc1 = bit_loc + 1
                else:
                    fix_km_prof.bit_loc1 = -1

                fix_km_prof.byte_sel.insert(fix_km_prof.bit_loc, -1)

                if fix_km_prof.bit_loc1 != -1:
                    fix_km_prof.byte_sel.insert(fix_km_prof.bit_loc1, -1)

            else:
                fix_km_prof.bit_loc = -1
                fix_km_prof.bit_loc1 = -1

            if right_justify:
                for _ in range(len(fix_km_prof.byte_sel), max_kmB):
                    fix_km_prof.byte_sel.insert(0, -1)
                    # move bit_locs as we insert un-used bytes
                    if fix_km_prof.bit_loc > 0:
                        fix_km_prof.bit_loc += 1
                    if fix_km_prof.bit_loc1 > 0:
                        fix_km_prof.bit_loc1 += 1
            return

        #####
        # This function is written to handle a specific case seen while compiling nic.p4
        # where a tcam table needs two key makers and one of the key-maker is shared with another
        # tcam table. In this case a lot of fields were common between two key makers,
        # those fields can be removed from one (non-shared) key maker and key can be compressed
        if self.num_km < 2:
            return False

        if (self.final_key_size - self.key_size) < 64:
            return False  # not worth fixing it ??? XXX

        km0_shared = False; km1_shared = False
        km0_prof = self.key_makers[0].combined_profile
        km1_prof = self.key_makers[1].combined_profile
        if self.key_makers[0].shared_km:
            km0_prof = self.key_makers[0].shared_km.combined_profile
            km0_shared = True

        if self.key_makers[1].shared_km:
            km1_prof = self.key_makers[1].shared_km.combined_profile
            km1_shared = True

        # if both kms are shared or both kms not shared, cannot compress
        if km0_shared == km1_shared:
            return False

        max_km_width = self.gtm.tm.be.hw_model['match_action']['key_maker_width']
        max_kmB = max_km_width/8

        ncc_assert(km0_shared or km1_shared)

        common_byte_sel = set(km0_prof.byte_sel) & set(km1_prof.byte_sel)
        common_bit_sel = set(km0_prof.bit_sel) & set(km1_prof.bit_sel)
        if not km0_shared:
            fix_km_prof = km0_prof
        else:
            fix_km_prof = km1_prof

        if len(common_byte_sel) == 0:
            return False # nothing is common

        # remove the -1 bytes
        if fix_km_prof.bit_loc1 != -1:
            fix_km_prof.byte_sel.pop(fix_km_prof.bit_loc1)
            # bit loc1 is after bit_loc..so bit_loc index is still valid
        if fix_km_prof.bit_loc != -1:
            fix_km_prof.byte_sel.pop(fix_km_prof.bit_loc)

        if -1 in common_byte_sel:
            common_byte_sel.remove(-1)

        if len(common_byte_sel):
            self.gtm.tm.logger.debug("%s:removing common bytes from km0 and km1 %s" % \
                (self.p4_table.name, common_byte_sel))

        for b in common_byte_sel:
            fix_km_prof.byte_sel.remove(b)
            if b in fix_km_prof.k_byte_sel:
                fix_km_prof.k_byte_sel.remove(b)
            elif b in fix_km_prof.i1_byte_sel:
                fix_km_prof.i1_byte_sel.remove(b)
            else:
                fix_km_prof.i2_byte_sel.remove(b)

        for b in common_bit_sel:
            fix_km_prof.bit_sel.remove(b)
            if b in fix_km_prof.k_bit_sel:
                fix_km_prof.k_bit_sel.remove(b)
            else:
                fix_km_prof.i_bit_sel.remove(b)

        if not km0_shared:
            _fix_bit_loc(fix_km_prof, max_kmB, True)
        else:
            _fix_bit_loc(fix_km_prof, max_kmB, False)

        # need to recompute key_offsets - done by caller
        return True

    def _fix_idx_table_km_profile(self):
        # if k_bits: need to move those the the end of bit_sel
        # if no kbits - just need a byte alignement
        if self.is_overflow:
            return;

        max_km_width = self.gtm.tm.be.hw_model['match_action']['key_maker_width']
        max_kmB = max_km_width/8
        start_km = self.start_key_off / max_km_width
        end_km = self.end_key_off / max_km_width
        km_shared = False
        km_off = max_km_width if start_km else 0

        km_prof = None
        if self.key_makers[start_km].shared_km:
            km_prof = self.key_makers[start_km].shared_km.combined_profile
            km_shared = True
        else:
            km_prof = self.key_makers[start_km].combined_profile

        if self.key_makers[end_km].shared_km:
            end_km_prof = self.key_makers[end_km].shared_km.combined_profile
            end_km_shared = True
        else:
            end_km_prof = self.key_makers[end_km].combined_profile

        ncc_assert(km_prof == end_km_prof )# not allowed

        max_km_bits = self.gtm.tm.be.hw_model['match_action']['num_bit_extractors']

        is_shared_idx_table = False

        shared_idx_tables = []
        if km_shared:
            shared_idx_tables = [x for x in self.key_makers[start_km].shared_km.ctables \
                                    if x.is_index_table() and x != self]
            if len(shared_idx_tables):
                is_shared_idx_table = True

        if len(self.combined_profile.k_bit_sel):
            num_k_bits = len(km_prof.k_bit_sel)
            # since k bits are at the start, use bit_loc0 to compute start offset
            k_bit_start_off = (km_prof.bit_loc * 8)
            k_bit_end_off = k_bit_start_off + num_k_bits    # no -1, it is one after the end bit

            # if km is shared with another index table, it may have k_bytes
            # reverse i and k bits and fill up unused bits with -1
            km_prof.bit_sel = km_prof.i_bit_sel + km_prof.k_bit_sel


            num_bytes = (len(km_prof.bit_sel)+7) / 8
            num_bits = num_bytes * 8

            # pad bit_sel with -1
            pad_bits = 0
            for i in range(num_bits-len(km_prof.bit_sel)):
                km_prof.bit_sel.insert(0, -1)
                pad_bits += 1

            if self.start_key_off == k_bit_start_off:
                self.start_key_off += pad_bits
            if self.end_key_off == k_bit_end_off:
                self.end_key_off += pad_bits

            bit_end = (km_prof.bit_loc+num_bytes) * 8

            if bit_end % 16:
                if len(km_prof.byte_sel) < max_kmB:
                    # add a pad byte
                    km_prof.byte_sel.insert(km_prof.bit_loc, -1)
                    self.start_key_off += 8
                    self.end_key_off += 8
                    km_prof.bit_loc += 1
                    if km_prof.bit_loc1 != -1:
                        km_prof.bit_loc1 += 1
                    self.gtm.tm.logger.debug("%s:%s:Fixed index location (add pad at bit_loc) to %d:%d" % \
                        (self.gtm.d.name, self.p4_table.name, self.start_key_off, self.end_key_off))
                else:
                    # move/split bit_loc bytes
                    if len(km_prof.k_byte_sel) == 0:
                        # move to the other end of the km_profile
                        # move both bit extractors as entire key is in bits (???)
                        if km_prof.bit_loc != -1:
                            km_prof.byte_sel.pop(km_prof.bit_loc)
                            km_prof.byte_sel.append(-1)
                        if km_prof.bit_loc1 != -1:
                            km_prof.byte_sel.pop(km_prof.bit_loc1)
                            km_prof.byte_sel.append(-1)

                        if km_prof.bit_loc1 != -1:
                            km_prof.bit_loc1 = max_kmB-1
                            km_prof.bit_loc = max_kmB-2
                        else:
                            km_prof.bit_loc = max_kmB-1

                        self.start_key_off = km_off + (km_prof.bit_loc * 8) + num_bits - num_k_bits
                        self.end_key_off = km_off + max_km_width
                        self.gtm.tm.logger.debug("%s:%s:Fixed index location (split bit_loc) to %d:%d" % \
                            (self.gtm.d.name, self.p4_table.name, self.start_key_off, self.end_key_off))
                    else:
                        # just don't insert middle of another key XXX ???
                        ncc_assert(0 )# TBD
            return

        # no k-bits, but there could be i-bits...
        if self.end_key_off % 16:
            ncc_assert((self.end_key_off % 8) == 0)
            # when two idx tables share a profile, it is possible to mis-align other table's
            # key while fixing alignment for this
            # if tables are processed based on sorted key_offsets order, we can keep adjusting
            # one table at a time w/o affecting the next one
            if len(km_prof.byte_sel) < max_kmB:
                if is_shared_idx_table:
                    k_byte_idx = km_prof.byte_sel.index(self.combined_profile.k_byte_sel[0])
                else:
                    k_byte_idx = km_prof.byte_sel.index(km_prof.k_byte_sel[0])
                km_prof.byte_sel.insert(k_byte_idx, -1)
                self.start_key_off += 8
                self.end_key_off += 8
                if km_prof.bit_loc > k_byte_idx:
                    km_prof.bit_loc += 1
                if km_prof.bit_loc1 != -1 and km_prof.bit_loc1 > k_byte_idx:
                    km_prof.bit_loc1 += 1
                self.gtm.tm.logger.debug("%s:%s:Fixed index location by +8 : %d,%d" % \
                    (self.gtm.d.name, self.p4_table.name, self.start_key_off, self.end_key_off))
                return
            # key maker is full, if there are i-bits we can move it around to align the key
            # if not, need to convert byte to bit_sel and move a byte using it
            if is_shared_idx_table:
                k_byte_start_idx = km_prof.byte_sel.index(self.combined_profile.k_byte_sel[0])
                k_byte_end_idx = km_prof.byte_sel.index(self.combined_profile.k_byte_sel[-1])
            else:
                k_byte_start_idx = km_prof.byte_sel.index(km_prof.k_byte_sel[0])
                k_byte_end_idx = km_prof.byte_sel.index(km_prof.k_byte_sel[-1])

            if len(km_prof.bit_sel) <= 8:
                # when profile is shared, i1 and i2 bytes are kind of messed-up..i.e they are marked
                # as i1 or i2 based on their relative location to k-bytes of the base table
                # pick i1 or i2 byte of this table.. need to check if that is not k byte for other
                # tables sharing this km_profile
                # if tables are shared, when aligning the idx of the second table, make sure not
                # to disturb the first table.. to avoid it use only i bytes *after* the current
                # k_byte_end_idx
                shared_k_bytes = []
                for t in shared_idx_tables:
                    shared_k_bytes += t.combined_profile.k_byte_sel
                # add k bytes of this table
                shared_k_bytes += self.combined_profile.k_byte_sel

                i_byte_chunks = [] #(last_byte, num_contiguous_bytes before it)

                for i in range(k_byte_end_idx+1, max_kmB):
                    b = km_prof.byte_sel[i]
                    if b < 0:
                        continue
                    if b not in shared_k_bytes:
                        if len(i_byte_chunks) and b == i_byte_chunks[-1][0]+1:
                            i_byte_chunks[-1] = (b, i_byte_chunks[-1][1]+1)
                        else:
                            i_byte_chunks.append((b, 1))

                if len(i_byte_chunks) == 0:
                    self.gtm.tm.logger.critical( \
                        "Cannot align the index table key to 16b boundary")
                    self.gtm.tm.logger.critical( \
                        "Need to implement more sophisticated algo to align key for %s" % \
                        (self.p4_table.name))
                    ncc_assert(0)

                # sort based on num contiguous bytes
                i_byte_chunks = sorted(i_byte_chunks, key=lambda t: t[1])
                if i_byte_chunks[0][1] > 1:
                    # pick the last byte
                    byte_to_bit = i_byte_chunks[-1][0]
                else:
                    byte_to_bit = i_byte_chunks[0][0]

                if km_prof.bit_loc < 0:
                    km_prof.bit_loc = k_byte_start_idx
                else:
                    km_prof.bit_loc1 = k_byte_start_idx
                km_prof.byte_sel.insert(k_byte_start_idx, -1)
                for i in range(8):
                    km_prof.bit_sel.append((byte_to_bit * 8) + i)
                    km_prof.i_bit_sel.append((byte_to_bit * 8) + i)

                self.start_key_off += 8
                self.end_key_off += 8
                self.gtm.tm.logger.debug( \
                    "%s:Converted i2 byte %d to bits and located at %d in km before K" % \
                    (self.p4_table.name, byte_to_bit, km_prof.bit_loc))

                # remove the byte converted to bit from various byte_sels
                km_prof.byte_sel.remove(byte_to_bit)
                if byte_to_bit in km_prof.i1_byte_sel:
                    km_prof.i1_byte_sel.remove(byte_to_bit)
                elif byte_to_bit in km_prof.i2_byte_sel:
                    km_prof.i2_byte_sel.remove(byte_to_bit)
                else:
                    ncc_assert(0 )# Bug
                    pass

                if self not in shared_idx_tables:
                    shared_idx_tables.append(self)

                for ct in shared_idx_tables:
                    if byte_to_bit in ct.combined_profile.byte_sel:
                        ct.combined_profile.byte_sel.remove(byte_to_bit)
                    if byte_to_bit in ct.combined_profile.i1_byte_sel:
                        ct.combined_profile.i1_byte_sel.remove(byte_to_bit)
                    elif byte_to_bit in ct.combined_profile.i2_byte_sel:
                        ct.combined_profile.i2_byte_sel.remove(byte_to_bit)

                return

            # more than 8 bits extractions are used and km_profile is full
            # move the i-bits around to shift k by 1 byte
            # XXX - when this is used for shared table, can mis-align first table
            # need to check
            ncc_assert(km_prof.bit_loc != -1)
            if km_prof.bit_loc1 != -1:
                if km_prof.bit_loc1 < k_byte_start_idx:
                    # move it after the k_bytes
                    km_prof.byte_sel.pop(km_prof.bit_loc1)
                    # kbyte index is automatically moved after k byte due to pop
                    km_prof.byte_sel.insert(k_byte_end_idx, -1)
                    km_prof.bit_loc1 = k_byte_end_idx
                    self.start_key_off -= 8
                    self.end_key_off -= 8
                else:
                    # move it before the k_bytes
                    km_prof.byte_sel.pop(km_prof.bit_loc1)
                    km_prof.byte_sel.insert(k_byte_start_idx, -1)
                    km_prof.bit_loc1 = k_byte_start_idx
                    self.start_key_off += 8
                    self.end_key_off += 8
            elif km_prof.bit_loc < k_byte_start_idx:
                # move it after the k_bytes
                km_prof.byte_sel.pop(km_prof.bit_loc)
                # kbyte index is automatically moved after k byte due to pop
                km_prof.byte_sel.insert(k_byte_end_idx, -1)
                km_prof.bit_loc = k_byte_end_idx
                self.start_key_off -= 8
                self.end_key_off -= 8
            else:
                # move it before the k_bytes
                km_prof.byte_sel.pop(km_prof.bit_loc)
                km_prof.byte_sel.insert(k_byte_start_idx, -1)
                km_prof.bit_loc = k_byte_start_idx
                self.start_key_off += 8
                self.end_key_off += 8

            self.gtm.tm.logger.debug("%s:%s:Fixed index location by splitting bits: %d,%d" % \
                (self.gtm.d.name, self.p4_table.name, km_prof.bit_loc, km_prof.bit_loc1))


    def __repr__(self):
        return self.p4_table.name

class capri_predicate:
    def __init__(self, gtm, p4_cond):
        self.p4_cond = p4_cond
        self.gtm = gtm
        # Only the following condition expressions are supported
        #   C1 := (boolean_field == True/False)
        #   C2 := (field == value)
        #   [C1 | C2] && [C1 | C2] && [C1 | C2] ...
        # Not supported:
        # field != value
        # C1 || C2 : No OR conditions supported

        # store each field and expected value for condition to be True
        # Hw implements it using TCAM, all AND conditions are concatenated
        # Total # of bit across all conditions must be < 8
        self.cfield_vals = []   # list of (cf, value) used in p4 condition, when cond == True
        self.cwidth = 0

        def _expand(p4_expr):
            supported_ops = ['and', '==', 'valid']
            ncc_assert(p4_expr.op in supported_ops, \
                "Unsupported op %s in control-flow. Allowed ops %s" % (p4_expr.op, supported_ops))

            if p4_expr.op == 'and':
                ncc_assert(isinstance(p4_expr.left, p4.p4_expression), "Invalid condition")
                ncc_assert(isinstance(p4_expr.right, p4.p4_expression), "Invalid condition")
                # check bits accumulated so far
                ncc_assert(self.cwidth <= 8)
                _expand(p4_expr.left)
                ncc_assert(self.cwidth <= 8)
                _expand(p4_expr.right)
                return

            elif p4_expr.op == '==':
                if isinstance(p4_expr.left, p4.p4_field):
                    hf_name = get_hfname(p4_expr.left)
                    cf = self.gtm.tm.be.pa.get_field(hf_name, self.gtm.d)
                    ncc_assert(cf)
                    ncc_assert(isinstance(p4_expr.right, int))
                    cval = p4_expr.right
                elif isinstance(p4_expr.right, p4.p4_field):
                    hf_name = get_hfname(p4_expr.right)
                    cf = self.gtm.tm.be.pa.get_field(hf_name, self.gtm.d)
                    ncc_assert(cf)
                    ncc_assert(isinstance(p4_expr.left, int))
                    cval = p4_expr.left
                else:
                    ncc_assert(0)
            elif p4_expr.op == 'valid':
                ncc_assert(isinstance(p4_expr.right, p4.p4_header_instance))
                hf_name = p4_expr.right.name + '.valid'
                cf = self.gtm.tm.be.pa.get_field(hf_name, self.gtm.d)
                ncc_assert(cf)
                cval = 1
            else:
                ncc_assert(0)

            self.cfield_vals.append((cf, cval))
            self.cwidth += cf.width

        _expand(self.p4_cond.condition)

    def __repr__(self):
        pstr = '%s:%d [' % (self.p4_cond.name, self.cwidth)
        for cf,val in self.cfield_vals:
            pstr += ' %s:%d' % (cf.hfname, val)
        pstr += ']'
        return pstr

class capri_key_maker:
    def __init__(self, stage, cts):
        # multiple tables can share this key_maker
        # use ctable info to decide lkp_type, launch flit etc
        self.stage = stage
        ncc_assert(isinstance(cts, list))
        self.km_id = -1 # logical id 0, 1 when multiple key-makers used
        self.hw_id = -1
        self.is_shared = False
        self.shared_km = None
        self.ctables = cts
        self.reuse_mutex_cts = []
        self.flits_used = []
        self.combined_profile = None
        # keep flit components of the combined profile.. these are needed while
        # sharing profiles across tables
        self.flit_km_profiles = OrderedDict() # {fid: profile}
        # HACK: there is problem in sharing km which has overflow key 
        self.has_overflow_key = False   # set it when overflow key is added

    def _merge(self, rhs, is_overflow_key_merge=False):
        # merge two key_makers XXX can be __add__
        for ct in rhs.ctables:
            if ct not in self.ctables:
                self.ctables.append(ct)
        self.has_overflow_key = rhs.has_overflow_key
        base_table = None
        idx_tbls = [ct for ct in self.ctables if ct.is_index_table()]
        h_tbls = [ct for ct in self.ctables if ct.is_hash_table()]

        if len(h_tbls):
            ncc_assert(len(idx_tbls) == 0)
            base_table = h_tbls[0]

        elif len(idx_tbls):
            for ct in idx_tbls:
                if len(ct.combined_profile.k_bit_sel):
                    base_table = ct
                    break
        else:
            # for tcam, base table does not matter, if keys of the two tables are
            # intertwined
            pass

        if base_table:
            self.stage.gtm.tm.logger.debug("km_merge: use %s - %s as main table" % \
                (base_table.p4_table.name, base_table.match_type.name))

        if len(self.flits_used) == 0:
            self.flits_used += rhs.flits_used
        else:
            for fid in rhs.flits_used:
                if fid not in self.flits_used:
                    self.flits_used.append(fid)
            self.flits_used = sorted(self.flits_used)

        for fid,fkp in rhs.flit_km_profiles.items():
            if fid not in self.flit_km_profiles:
                self.flit_km_profiles[fid] = copy.deepcopy(fkp)
            else:
                self.flit_km_profiles[fid] += fkp

        # rebuild the combined profile
        if not self.combined_profile:
            self.combined_profile = capri_km_profile(self.stage.gtm)

        # clear existing info

        self.combined_profile.i1_byte_sel = []
        self.combined_profile.i2_byte_sel = []
        self.combined_profile.k_byte_sel = []
        self.combined_profile.k_bit_sel = []
        self.combined_profile.i_bit_sel = []
        k_bit_sel = []
        # merge must take into account table types
        for fid in sorted(self.flit_km_profiles.keys()):
            if base_table:
                # make sure all hash and idx table keys are kept contiguous
                k_byte_sel = []; i1_byte_sel = []; i2_byte_sel = []
                fk_byte = -1
                for b in self.flit_km_profiles[fid].k_byte_sel:
                    if b in base_table.combined_profile.k_byte_sel:
                        k_byte_sel.append(b)

                # it is possible that base table does not load k bytes in a
                # given flit... but it may have more k bytes in following flits
                # so make sure that bytes from this flit do not get inserted between k-byte of the
                # base table
                if len(k_byte_sel):
                    fk_byte = k_byte_sel[0]

                # fk_byte == -1 indicates no k bytes of base table.. it will make all
                # bytes of shared table as i2
                for b in self.flit_km_profiles[fid].i1_byte_sel:
                    if self.flit_km_profiles[fid].has_overflow_key or b < fk_byte:
                        i1_byte_sel.append(b)
                    else:
                        i2_byte_sel.append(b)

                overflow_k_bytes = []
                for b in self.flit_km_profiles[fid].k_byte_sel:
                    # pick up k bytes of shared table (non hash/idx)
                    if b in k_byte_sel:
                        continue
                    if base_table.collision_ct and is_overflow_key_merge:
                        # special case for hash_overflow table k-bytes
                        # keep them as i1 bytes
                        i1_byte_sel.append(b)
                        self.has_overflow_key = True
                        overflow_k_bytes.append(b)
                    elif b < fk_byte:
                        i1_byte_sel.append(b)
                    else:
                        i2_byte_sel.append(b)

                # fix the profile used for the overflow key, move overflow key to i1
                for b in overflow_k_bytes:
                    self.flit_km_profiles[fid].k_byte_sel.remove(b)
                    self.flit_km_profiles[fid].i1_byte_sel.append(b)
                    self.flit_km_profiles[fid].has_overflow_key = True
                    rhs.flit_km_profiles[fid].has_overflow_key = True
                    self.stage.gtm.tm.logger.debug("km_merge: move k to i1 fid %d: %d" % \
                        (fid, b))

                i2_byte_sel += self.flit_km_profiles[fid].i2_byte_sel
                self.combined_profile.i1_byte_sel += sorted(i1_byte_sel)
                self.combined_profile.i2_byte_sel += sorted(i2_byte_sel)
                self.combined_profile.k_byte_sel += sorted(k_byte_sel)
                # Must keep k-bits of the base-table together (don't update k bits into profile)
                k_bit_sel += self.flit_km_profiles[fid].k_bit_sel
                self.combined_profile.i_bit_sel += self.flit_km_profiles[fid].i_bit_sel
                # continue
            else:
                # no base table
                self.combined_profile.i1_byte_sel += self.flit_km_profiles[fid].i1_byte_sel
                self.combined_profile.i2_byte_sel += self.flit_km_profiles[fid].i2_byte_sel
                self.combined_profile.k_byte_sel += self.flit_km_profiles[fid].k_byte_sel
                # Must keep k-bits of the base-table together (don't update k bits)
                if base_table:
                    k_bit_sel += self.flit_km_profiles[fid].k_bit_sel
                else:
                    self.combined_profile.k_bit_sel += self.flit_km_profiles[fid].k_bit_sel

                self.combined_profile.i_bit_sel += self.flit_km_profiles[fid].i_bit_sel

        # create byte and bit_sel from k and i
        self.combined_profile.byte_sel = self.combined_profile.i1_byte_sel + \
            self.combined_profile.k_byte_sel + self.combined_profile.i2_byte_sel

        if base_table:
            # when a table is given a preference (i.e. hash/idx, must keep its k-bits together)
            # add all the k bits of the base table that are in this key-maker
            for b in k_bit_sel:
                if b in base_table.combined_profile.k_bit_sel:
                    self.combined_profile.k_bit_sel.append(b)
            # add other k_bits from this merged key_maker
            for b in k_bit_sel:
                if b not in self.combined_profile.k_bit_sel:
                    self.combined_profile.k_bit_sel.append(b)
            self.combined_profile.bit_sel = self.combined_profile.k_bit_sel + \
                self.combined_profile.i_bit_sel
        else:
            self.combined_profile.bit_sel = self.combined_profile.k_bit_sel + \
                self.combined_profile.i_bit_sel


    def _assign_bit_loc(self):
        # compute bit location for the combined profile based on tables that share this km
        # if there is hash table and hash_tbl has k_bits:
        #   place all bits right after last k_byte of hash table
        # if there is a index_table and index_table has k_bits:
        #   place all bits after last k_byte the index table
        # if k_bits: (both tables are tcams)
        #   place all bits at after the earlier of the last_k_bytes of all tables
        # elif i_bits:
        #   place them at the end of k_bytes (or before i2 bytes if not k_bytes)

        if self.combined_profile.bit_loc > 0:
            return # already assigned

        max_km_width = self.stage.gtm.tm.be.hw_model['match_action']['key_maker_width']
        max_kmB = max_km_width/8
        if len(self.combined_profile.bit_sel) == 0:
            self.combined_profile.bit_loc = -1
            return

        if len(self.combined_profile.k_bit_sel) == 0:
            # only i-bits:
            self.combined_profile._update_bit_loc_key_off()
            return

        hash_ctbls = [ct for ct in self.ctables if ct.is_hash_table()]
        idx_ctbls = [ct for ct in self.ctables if ct.is_index_table()]

        hash_ct = None
        if len(hash_ctbls):
            hash_ct = hash_ctbls[0]
            # ncc_assert(on un-supported km sharing)
            ncc_assert(len(hash_ctbls) == 1)
            ncc_assert(len(idx_ctbls) == 0)

        if len(idx_ctbls) > 1:
            # ncc_assert(on un-supported km sharing)
            # I think multiple idx tables can share km if both do not need k_bits XXX
            self.stage.gtm.tm.logger.debug("Merging multiple index tables %s" % \
                ([ct.p4_table.name for ct in idx_ctbls]))

        if hash_ct:
            k_bits = set(self.combined_profile.k_bit_sel) & set(hash_ct.combined_profile.k_bit_sel)
            k_bytes = set(self.combined_profile.k_byte_sel) & \
                set(hash_ct.combined_profile.k_byte_sel)

            if k_bits:
                num_bytes = 0
                bit_loc = -1
                if len(k_bytes):
                    k_byte = -1
                    for b in reversed(self.combined_profile.k_byte_sel):
                        if b in hash_ct.combined_profile.k_byte_sel:
                            k_byte = b
                            break
                    ncc_assert(k_byte >= 0)
                    bit_loc = self.combined_profile.byte_sel.index(k_byte) + 1
                    num_bytes = (len(self.combined_profile.bit_sel)+7) / 8
                    for i in range(num_bytes):
                        self.combined_profile.byte_sel.insert(bit_loc+i, -1)
                    self.combined_profile.bit_loc = bit_loc
                    if num_bytes > 1:
                        self.combined_profile.bit_loc1 = bit_loc + 1
                else:
                    self.combined_profile._update_bit_loc_key_off()
            else:
                self.combined_profile._update_bit_loc_key_off()
            ncc_assert(len(self.combined_profile.byte_sel) <= max_kmB)
            return

        if len(idx_ctbls):
            idx_ct = None
            for ct in idx_ctbls:
                if ct.combined_profile.k_bit_sel:
                    idx_ct = ct
                    break
            # place bits after idx_ct k_bytes
            if idx_ct:
                k_bytes = set(self.combined_profile.k_byte_sel) & \
                            set(idx_ct.combined_profile.k_byte_sel)
                if len(k_bytes):
                    k_byte = -1
                    for b in reversed(self.combined_profile.k_byte_sel):
                        if b in idx_ct.combined_profile.k_byte_sel:
                            k_byte = b
                            break
                    ncc_assert(k_byte >= 0)
                    bit_loc = self.combined_profile.byte_sel.index(k_byte) + 1
                    num_bytes = (len(self.combined_profile.bit_sel)+7) / 8
                    for i in range(num_bytes):
                        self.combined_profile.byte_sel.insert(bit_loc+i, -1)
                    self.combined_profile.bit_loc = bit_loc
                    if num_bytes > 1:
                        self.combined_profile.bit_loc1 = bit_loc + 1
                else:
                    self.combined_profile._update_bit_loc_key_off()
            else:
                # Bit loc is last k_byte in combine_profile.byte_sel
                if len(self.combined_profile.k_byte_sel):
                    last_k_byte = self.combined_profile.k_byte_sel[-1]
                    bit_loc = self.combined_profile.byte_sel.index(last_k_byte) + 1
                    num_bytes = (len(self.combined_profile.bit_sel)+7) / 8
                    for i in range(num_bytes):
                        self.combined_profile.byte_sel.insert(bit_loc+i, -1)
                    self.combined_profile.bit_loc = bit_loc
                    if num_bytes > 1:
                        self.combined_profile.bit_loc1 = bit_loc + 1
                else:
                    self.combined_profile._update_bit_loc_key_off()
            ncc_assert(len(self.combined_profile.byte_sel) <= max_kmB)
            return
        else:
            # tcam tables - can place bits after K
            cts = sorted(self.ctables, key=lambda k:k.key_phv_size)
            ct = cts[0]
            k_bytes = set(self.combined_profile.k_byte_sel) & \
                        set(ct.combined_profile.k_byte_sel)
            if len(k_bytes):
                k_byte = -1
                for b in reversed(self.combined_profile.k_byte_sel):
                    if b in ct.combined_profile.k_byte_sel:
                        k_byte = b
                        break
                ncc_assert(k_byte >= 0)
                bit_loc = self.combined_profile.byte_sel.index(k_byte) + 1
                num_bytes = (len(self.combined_profile.bit_sel)+7) / 8
                for i in range(num_bytes):
                    self.combined_profile.byte_sel.insert(bit_loc+i, -1)
                self.combined_profile.bit_loc = bit_loc
                if num_bytes > 1:
                    self.combined_profile.bit_loc1 = bit_loc + 1
            else:
                self.combined_profile._update_bit_loc_key_off()

        ncc_assert(len(self.combined_profile.byte_sel) <= max_kmB)
        return

    def get_hw_id(self):
        if self.shared_km:
            return self.shared_km.hw_id
        return self.hw_id

    def copy_km(self):
        new_km = capri_key_maker(self.stage, [])
        new_km.km_id = self.km_id
        new_km.hw_id = self.hw_id
        new_km.is_shared = False
        new_km.shared_km = None
        new_km.flits_used = copy.copy(self.flits_used)
        new_km.combined_profile = copy.deepcopy(self.combined_profile)
        new_km.flit_km_profiles = copy.deepcopy(self.flit_km_profiles)
        new_km.has_overflow_key = self.has_overflow_key
        return new_km

class capri_km_profile:
    def __init__(self, gtm):
        self.gtm = gtm
        self.hw_id = -1
        self.mode = 0   # 0=Normal, 1=2B_Lo, 2=2B_high XXX
        self.i1_byte_sel = []
        self.k_byte_sel = []
        self.i2_byte_sel = []
        self.k_bit_sel = []
        self.i_bit_sel = []
        # needed for hw
        self.byte_sel = []
        self.bit_sel = []

        self.bit_loc = -1     # byte0 reseved for bit extracted values
        self.bit_loc1 = -1    # byte1 reseved for bit extracted values
        self.start_key_off = -1
        self.end_key_off = -1

        # HACK
        self.has_overflow_key = False

    def __add__(self, rhs):
        # should be used on per flit basis
        # XXX too many conditions to check while updating i1, i2 and k
        # combine the k bytes and redistribute i1, i2
        # since this is done on per flit basis, k, i1, i2 distribution when merging
        # two index table is a bit arbitrary

        # DOES NOT handle the start/end offsets, place holder for bit_loc are removed

        # check flit#
        flit_sz = self.gtm.tm.be.hw_model['phv']['flit_size']
        flit_szB = flit_sz/8

        my_flit = -1; rhs_flit = -1
        if len(self.byte_sel):
            my_flit = self.byte_sel[0] / flit_szB
            ncc_assert(my_flit == self.byte_sel[-1] / flit_szB)
        if len(rhs.byte_sel):
            rhs_flit = rhs.byte_sel[0] / flit_szB
            ncc_assert(rhs_flit == rhs.byte_sel[-1] / flit_szB)

        if my_flit != -1 and rhs_flit != -1:
            ncc_assert(my_flit == rhs_flit)


        old_size = self.km_prof_size()
        rhs_size = rhs.km_prof_size()

        k_byte_sel = copy.copy(self.k_byte_sel)
        for b in rhs.k_byte_sel:
            if b not in self.k_byte_sel:
                k_byte_sel.append(b)

        if len(k_byte_sel) == 0:
            # add everything as i2
            for b in rhs.i2_byte_sel:
                if b not in self.i2_byte_sel:
                    self.i2_byte_sel.append(b)
            self.i2_byte_sel = sorted(self.i2_byte_sel)
            for b in rhs.k_bit_sel:
                if b not in self.bit_sel:
                    self.k_bit_sel.append(b)
            for b in rhs.i_bit_sel:
                if b not in self.bit_sel:
                    self.i_bit_sel.append(b)
            self.bit_sel = self.k_bit_sel + self.i_bit_sel
            return self

        k_byte_sel = sorted(k_byte_sel)
        k_start = k_byte_sel[0]
        k_end = k_byte_sel[-1]

        byte_sel = []
        byte_sel += k_byte_sel
        for b in self.byte_sel:
            if b not in byte_sel:
                byte_sel.append(b)

        for b in rhs.byte_sel:
            if b not in byte_sel:
                byte_sel.append(b)
        byte_sel = sorted(byte_sel)

        i1_byte_sel = []
        i2_byte_sel = []

        for b in byte_sel:
            if b < k_start:
                i1_byte_sel.append(b)
            elif b <= k_end:
                # since this is a combined profile, k, i1,i2 is bit arbitrary
                # when merging two index tables, we can have disjoint keys and i bytes
                # in the middle, just add them as k
                if b not in k_byte_sel:
                    k_byte_sel.append(b)
            else:
                i2_byte_sel.append(b)

        self.k_byte_sel = sorted(k_byte_sel)
        self.i1_byte_sel = sorted(i1_byte_sel)
        self.i2_byte_sel = sorted(i2_byte_sel)
        self.byte_sel = self.i1_byte_sel + self.k_byte_sel + self.i2_byte_sel
        ncc_assert(self.byte_sel) == sorted(self.byte_sel)

        for b in rhs.k_bit_sel:
            if b not in self.bit_sel:
                self.k_bit_sel.append(b)
        for b in rhs.i_bit_sel:
            if b not in self.bit_sel:
                self.i_bit_sel.append(b)
        self.bit_sel = self.k_bit_sel + self.i_bit_sel

        ncc_assert(self.km_prof_size() <= (old_size + rhs_size))
        self.gtm.tm.logger.debug("Added km_profiles: %d + %d = %d" % \
            (old_size, rhs_size, self.km_prof_size()))

        # need to recompute these
        self.bit_loc = -1     # byte(s) reseved for bit extracted values
        self.bit_loc1 = -1     # byte(s) reseved for bit extracted values
        self.start_key_off = -1
        self.end_key_off = -1
        return self

    def __deepcopy__(self, memo):
        new_obj = capri_km_profile(self.gtm)
        new_obj.hw_id = copy.copy(self.hw_id)
        new_obj.i1_byte_sel = copy.copy(self.i1_byte_sel)
        new_obj.k_byte_sel = copy.copy(self.k_byte_sel)
        new_obj.i2_byte_sel = copy.copy(self.i2_byte_sel)
        new_obj.k_bit_sel = copy.copy(self.k_bit_sel)
        new_obj.i_bit_sel = copy.copy(self.i_bit_sel)
        new_obj.byte_sel = copy.copy(self.byte_sel)
        new_obj.bit_sel = copy.copy(self.bit_sel)

        new_obj.bit_loc = self.bit_loc      # byte(s) reseved for bit extracted values
        new_obj.bit_loc1 = self.bit_loc1    # byte(s) reseved for bit extracted values
        new_obj.start_key_off = self.start_key_off
        new_obj.end_key_off = self.end_key_off
        new_obj.has_overflow_key = self.has_overflow_key

        return new_obj

    def create_2B_profile(self, km_prof, use_low = True):
        km_prof_size = self.gtm.tm.be.hw_model['match_action']['key_maker_width']
        km_sizeB = km_prof_size/8
        km_size2B = km_sizeB/2

        if use_low:
            ncc_assert(len(self.byte_sel) == 0 )# low portion is already used
        else:
            ncc_assert(len(self.byte_sel) == km_size2B )# low portion is not yet used
            b = -2
        # append to exsting byte_sel.. if lo half is already added, it is also padded upto half size
        for b in range(0, len(km_prof.byte_sel), 2):
            self.byte_sel.append(km_prof.byte_sel[b])
        # for odd len.. last byte is already added in the loop above
        b += 2
        # fill the rest of the half profile with un-used (km_sizeB is used since step is 2
        for _ in range(b, km_sizeB, 2):
            self.byte_sel.append(-1)

    def km_prof_size(self):
        return len(self.byte_sel) + ((len(self.bit_sel)+7)/8)

    def km_num_elements(self):
        return (len(self.byte_sel), len(self.bit_sel))

    def _update_bit_loc_key_off(self):
        last_k_idx = -1
        first_k_idx = -1
        num_k_bits = len(self.k_bit_sel)
        if self.bit_loc == -1 and len(self.bit_sel):
            if len(self.k_byte_sel):
                # to make if easy when hash table keys are > km_width,
                # if there are no k_bits then
                # add the i-bits to the front (before k bytes)
                if num_k_bits:
                    # place after k_bytes
                    bit_loc = self.byte_sel.index(self.k_byte_sel[-1]) + 1
                else:
                    # place bits at the front of (before) K bytes
                    bit_loc = self.byte_sel.index(self.k_byte_sel[0])
            elif len(self.i2_byte_sel):
                # place before i2_bytes
                bit_loc = self.byte_sel.index(self.i2_byte_sel[0])
            elif len(self.i1_byte_sel):
                bit_loc = self.byte_sel.index(self.i1_byte_sel[0]) + 1
            else:
                bit_loc = 0
            ncc_assert(bit_loc >= 0)
            num_bytes = (len(self.bit_sel)+7)/8
            for i in range(num_bytes):
                self.byte_sel.insert(bit_loc, -1)

            self.bit_loc = bit_loc
            if num_bytes > 1:
                self.bit_loc1 = bit_loc + 1

        # this code does not handle any key_offsets needed due to bit2byte conversion
        # that is handled per ctable and in not visible to this code
        if self.start_key_off == -1:
            if len(self.k_byte_sel):
                first_k_byte = self.k_byte_sel[0]
                first_k_idx = self.byte_sel.index(first_k_byte)
                self.start_key_off = first_k_idx * 8
            elif len(self.k_bit_sel):
                first_k_idx = self.bit_loc
                self.start_key_off = first_k_idx * 8
            else:
                pass

        if self.end_key_off == -1:
            if len(self.k_bit_sel):
                self.end_key_off = (self.bit_loc * 8) + len(self.k_bit_sel)
            elif len(self.k_byte_sel):
                last_k_byte = self.k_byte_sel[-1]
                last_k_idx = self.byte_sel.index(last_k_byte) + 1
                self.end_key_off = last_k_idx * 8
            else:
                pass

    def km_profile_validate(self):
        if self.mode:
            # validation for 2B mode - TBD
            pass

        bits_used = {}
        bytes_sel = [[] for _ in range(self.gtm.tm.be.hw_model['phv']['num_flits'])]
        flit_sz = self.gtm.tm.be.hw_model['phv']['flit_size']
        flit_szB = flit_sz/8
        # check duplicates
        for b in self.byte_sel:
            if b < 0:
                continue
            ncc_assert(b not in bytes_sel)
            bytes_sel[b/flit_szB].append(b)

        for b in self.bit_sel:
            if b < 0:
                continue
            ncc_assert(b not in bits_used)
            if b not in bits_used:
                bits_used[b] = 1

        # Banyon network violation
        for fb_sel in bytes_sel:
            if len(fb_sel) == 0:
                continue
            if sorted(fb_sel) != fb_sel:
                self.gtm.tm.logger.info("%s:Ignoring Banyon network violation for km_profile (overflow: %s)" % \
                    (self.gtm.d.name, self.has_overflow_key))
                self
            pass

    def __repr__(self):
        if len(self.byte_sel) or len(self.bit_sel):
            return '[id%d] : byte_sel[%d]: %s :\nbit_sel[%d]: %s bit_loc=%d,%d\n' % \
                (self.hw_id, len(self.byte_sel), self.byte_sel, len(self.bit_sel),
                 self.bit_sel, self.bit_loc, self.bit_loc1)
        else:
            return ''

class capri_stage:
    def __init__(self, gtm, stg_id):
        self.gtm = gtm
        self.id = stg_id

        num_km_profiles = gtm.tm.be.hw_model['match_action']['num_km_profiles']
        num_flits = self.gtm.tm.be.hw_model['phv']['num_flits']
        max_km = self.gtm.tm.be.hw_model['match_action']['num_key_makers']

        self.p4_table_list = None  # table list from hlir
        self.p4_table_list_unsorted = None  # table list from hlir
        self.ct_list = None
        self.table_profiles = OrderedDict() # {pred_val : [tables to apply]}
        self.table_profile_masks = OrderedDict() # {pred_val : active_conditions}
        self.table_sequencer = OrderedDict() # {pred_val : [capri_te_cycle]}
        self.active_predicates = [] # [active table_predicate_keys]
        self.km_allocator = [[None for _ in range(max_km)] for _ in range(num_flits)]
        max_km_profiles = self.gtm.tm.be.hw_model['match_action']['num_km_profiles']
        self.hw_km_profiles = [None for _ in range(max_km_profiles)]
        self.pred_sel_bits = 0

    def _can_share_hash_tcam_km(self, h_km, t_km):
        flit_sz = self.gtm.tm.be.hw_model['phv']['flit_size']
        flit_szB = flit_sz/8
        h_profile = h_km.combined_profile
        t_profile = t_km.combined_profile
        ct_byte_set = set(t_profile.byte_sel)
        ct_chk_bytes = ct_byte_set - (ct_byte_set & set(h_profile.k_byte_sel))
        h_key_first = -1
        h_key_last = -1
        if len(h_profile.k_byte_sel):
            h_key_first = h_profile.k_byte_sel[0]
            h_key_last = h_profile.k_byte_sel[-1]
        # any K+I byte from tcam table profile must fall outside the range of K bytes
        # of a hash table
        # this check needs to be done within flits, if there is a hash key byte in a flit
        # then other tables byte must be before first or after last key byte
        # if hash table has k-bits then bit_loc must be maintained right after hash
        # K bytes
        for b in ct_chk_bytes:
            if b == -1:
                # skip the bytes reserved for placing bits
                continue
            fid = b/flit_szB
            if fid not in h_km.flit_km_profiles:
                continue
            if len(h_km.flit_km_profiles[fid].k_byte_sel) == 0:
                continue
            hk_start = h_km.flit_km_profiles[fid].k_byte_sel[0]
            hk_end = h_km.flit_km_profiles[fid].k_byte_sel[-1]
            if b > hk_start and b < hk_end:
                # cannot share as tcam  key falls between
                self.gtm.tm.logger.debug("Cannot share km as tcam key falls within hash key")
                self.gtm.tm.logger.debug("hash_key %s, tcam_key %s" % \
                    (h_profile.k_byte_sel, t_profile.k_byte_sel))
                return False

            if b > hk_end and len(h_profile.k_bit_sel):
                # must keep the k bits next to K byte for hash table
                # XXX I think this can be allowed for tcam table
                #return False
                pass
        return True

    def _share_key_maker(self, fid, ct, new_km, km_used, mutex_cts):
        # XXX - TBD
        # 1. Tables with the same key
        # 2. Index tables that can fit into same key-makers
        # create a list of tables from allocated key makers in this flit

        km_tables = []
        for km in self.km_allocator[fid]:
            if km == None:
                continue
            for t in km.ctables:
                if t not in km_tables:
                    km_tables.append(t)

        if len(km_tables) == 0:
            # nothing to share
            return False

        km_tables = sorted(km_tables, key=lambda k:len(k.combined_profile.byte_sel))

        km_width = self.gtm.tm.be.hw_model['match_action']['key_maker_width']
        km_max_bits = self.gtm.tm.be.hw_model['match_action']['num_bit_extractors']

        # XXX merging TCAM tables can create in-efficiency when tcam key width is increased
        # as a result of merging.. merge tables based on largest common set of fields TBD

        new_km_profile = new_km.combined_profile
        ct_byte_sel = set(new_km_profile.byte_sel)
        ct_bit_sel = set(new_km_profile.bit_sel)
        for kt in km_tables:
            if kt == ct:
                continue

            self.gtm.tm.logger.debug("Flit %d:Find shareable km for %s:km %d with %s" % \
                (fid, ct.p4_table.name, new_km.km_id, kt.p4_table.name))
            km_found = None
            km_profile = None
            for km in kt.key_makers:
                km_profile = km.combined_profile
                km_hw_id = km.hw_id
                if km.is_shared:
                    if not ct.is_raw and not ct.is_raw_index:
                        # XXX multi-way sharing... need to check and operate on shared key-maker
                        self.gtm.tm.logger.debug("km is already shared with another table")
                        continue
                    # allow multi-way sharing for raw tables
                    km_profile = km.shared_km.combined_profile
                    km_hw_id = km.shared_km.hw_id

                if len(km.reuse_mutex_cts) > 0:
                    cannot_share = False
                    for mect in km.reuse_mutex_cts:
                        if mect not in mutex_cts:
                            self.gtm.tm.logger.debug("km is reused for mutually exclusive table %s cannot share" %
                                mect.p4_table.name)
                            cannot_share = True
                            break
                    if cannot_share:
                        continue
                if km_hw_id == -1:
                    # km is not allocated yet, it will get shared when its turn comes
                    continue
                km_id_used = False
                for _kmused in km_used:
                    # or km is used by the same table's another key_maker
                    if _kmused == km_hw_id:
                        km_id_used = True
                        break
                if km_id_used:
                    continue
                # HACK: there is problem in sharing km which has overflow key 
                # once overflow key is added to a key maker, need to check that overflow key 
                # is not disturbed
                if km.has_overflow_key or new_km.has_overflow_key:
                    continue

                '''
                # following code tries to fine tune the sharing of key-makers.. but it is not
                # working correctly. Keep it for some time - either fix it soon or get rid of it
                # when banyon restriction is removed
                if km.has_overflow_key:
                    pdb.set_trace()
                    cannot_share = False
                    for fid,fkp in km.flit_km_profiles.items():
                        if not fkp.has_overflow_key or fid not in new_km.flit_km_profiles:
                            continue
                        n_fkp = new_km.flit_km_profiles[fid]
                        if len(n_fkp.byte_sel) == 0:
                            continue
                        # XXX don't remeber if bit_sel positions are already added
                        ncc_assert(n_fkp.byte_sel[0] > 0)
                        if kt.is_overflow:
                            if len(fkp.k_byte_sel) == 0:
                                continue
                            if fkp.k_byte_sel[-1] < n_fkp.byte_sel[0]:
                                continue
                        else:
                            if len(fkp.i1_byte_sel) == 0:
                                continue
                            if fkp.i1_byte_sel[-1] < n_fkp.byte_sel[0]:
                                continue
                        self.gtm.tm.logger.debug("km has overflow key and i1bytes - cannot share")
                        cannot_share = True
                        break
                        
                    if cannot_share:
                        continue

                if new_km.has_overflow_key:
                    pdb.set_trace()
                    cannot_share = False
                    for fid,fkp in new_km.flit_km_profiles.items():
                        if not fkp.has_overflow_key or fid not in km.flit_km_profiles:
                            continue
                        n_fkp = km.flit_km_profiles[fid]
                        if len(n_fkp.byte_sel) == 0:
                            continue
                        # XXX don't remeber if bit_sel positions are already added
                        ncc_assert(n_fkp.byte_sel[0] > 0)
                        if ct.is_overflow:
                            if len(fkp.k_byte_sel) == 0:
                                continue
                            if fkp.k_byte_sel[-1] < n_fkp.byte_sel[0]:
                                continue
                        else:
                            if len(fkp.i1_byte_sel) == 0:
                                continue
                            if fkp.i1_byte_sel[-1] < n_fkp.byte_sel[0]:
                                continue
                        self.gtm.tm.logger.debug("km has overflow key and i1bytes - cannot share")
                        cannot_share = True
                        break
                        
                    if cannot_share:
                        continue
                '''

                u_byte_sel = set(km_profile.byte_sel) | ct_byte_sel
                # remove the bytes reserved for bit_sel
                u_byte_sel = u_byte_sel - set([-1])
                u_bit_sel = set(km_profile.bit_sel) | ct_bit_sel
                if len(u_bit_sel) > km_max_bits:
                    continue
                if (len(u_byte_sel) + (len(u_bit_sel)+7)/8) > (km_width/8):
                    self.gtm.tm.logger.debug(\
                        "u_bytes+u_bits exceed km_width:%s:%s\nNeed Bytes %d Bits %d" % \
                        (ct.p4_table.name, kt.p4_table.name, len(u_byte_sel),
                        len(u_bit_sel)))
                    continue
                if km.shared_km:
                    km_found = km.shared_km
                else:
                    km_found = km
                break

            if not km_found:
                continue

            # if identical keys - allow sharing
            identical_km = False
            if set(km.combined_profile.byte_sel) == ct_byte_sel and \
                set(km.combined_profile.bit_sel) == ct_bit_sel:
                identical_km = True
                if not ct.is_raw and not ct.is_raw_index:
                    # this is not handled in many subsequent routines.. so not supported for now
                    self.gtm.tm.logger.debug(\
                        "key-maker are identical for tables %s and %s.. but may not be shared - TBD" %
                        (kt.p4_table.name, ct.p4_table.name))
                    continue

                if km.shared_km:
                    shared_km = km.shared_km
                else:
                    shared_km = capri_key_maker(self, [])
                    shared_km._merge(km_found)

                ncc_assert(km_found.hw_id != -1)
                shared_km.hw_id = km_found.hw_id
                km.hw_id = -1
                new_km.hw_id = -1
                new_km.is_shared = True
                km.is_shared = True
                shared_km.is_shared = True
                new_km.shared_km = shared_km
                km.shared_km = shared_km

                return True

            # additional checks based on table types
            if kt.is_hash_table() and ct.is_hash_table():
                # need to worry about key ordering in both...
                # XXX can be combined if keys are completely disjoint and no k-bits...
                # not supported
                self.gtm.tm.logger.debug(\
                    "No sharing support between two hash tables w/ k_bits %s:%s" % \
                    (kt.p4_table.name, ct.p4_table.name))
                continue

            if kt.is_index_table() and ct.is_index_table():
                # if both index tables:
                # both cannot have k_bits
                if len(kt.combined_profile.k_bit_sel) and len(ct.combined_profile.k_bit_sel):
                    continue
                # check 16 bit aligment of sizes - must be same to share
                k1_alignment = ((kt.key_phv_size + 7)/8) % 2
                k2_alignment = ((ct.key_phv_size + 7)/8) % 2
                if k1_alignment != k2_alignment:
                    # cannot share
                    continue
            is_hash_table = kt.is_hash_table() or ct.is_hash_table()
            is_index_table = kt.is_index_table() or ct.is_index_table()
            if is_hash_table and is_index_table:
                # XXX not supported for now
                # allow if -
                #   at least one does not have k_bits
                #   index table key does not appear in-between hash key (if not part of hashkey)
                # ...
                self.gtm.tm.logger.debug("No sharing support between hash and index table")
                continue

            if kt.is_hash_table():
                if ct.is_tcam_table() and not self._can_share_hash_tcam_km(km_found, new_km):
                    self.gtm.tm.logger.debug("Cannot merge %s into %s" % \
                        (ct.p4_table.name, kt.p4_table.name))
                    continue

            if ct.is_hash_table():
                if kt.is_tcam_table() and not self._can_share_hash_tcam_km(new_km, km_found):
                    self.gtm.tm.logger.debug("Cannot merge %s into %s" % \
                        (kt.p4_table.name, ct.p4_table.name))
                    continue

            self.gtm.tm.logger.debug( \
                "%s:%d:Merge key makers %d for tables %s(size %d) and %s(size %d) flit %d" % \
                (self.gtm.d.name, self.id,
                km_found.km_id, kt.p4_table.name, km_profile.km_prof_size(),
                ct.p4_table.name, new_km_profile.km_prof_size(), fid))

            # XXX
            # need to take care of  i1, i2 and k bytes after merging
            # need to know where the keys are for both tables, where to place bit_ext in km
            # launch flit.....
            # combining two profiles: Need to keep the order within the byte selects within a
            # a flit

            if km_found.is_shared:
                shared_km = km_found
            else:
                shared_km = capri_key_maker(self, [])
            # XXX does it matter in which order things are added ???
            if kt.is_hash_table():
                # add kt before ct
                if shared_km != km_found:
                    shared_km._merge(km_found)
                shared_km._merge(new_km)
            elif ct.is_hash_table():
                shared_km._merge(new_km)
                if shared_km != km_found:
                    shared_km._merge(km_found)
            else:
                # no hash table involved, merging TCAM<-->SRAM
                shared_km._merge(new_km)
                if shared_km != km_found:
                    shared_km._merge(km_found)

            ncc_assert(km_found.hw_id != -1)
            shared_km.hw_id = km_found.hw_id
            km.hw_id = -1
            new_km.hw_id = -1
            new_km.is_shared = True
            km.is_shared = True
            shared_km.is_shared = True
            new_km.shared_km = shared_km
            km.shared_km = shared_km

            return True

        self.gtm.tm.logger.debug("%s:No shareable key-maker for %s"  % \
            (self.gtm.d.name, ct.p4_table.name))
        return False

    def _allocate_km(self, fid, ct, p_excl_tbls, need_sharing):
        if ct.is_overflow:
            if ct.is_otcam:
                return True
        if ct.num_km == 0:
            ct.key_makers[0].hw_id = 0
            return True
        total_km_allocated = 0
        # record already assigned hw_ids. hw_id used for one km of a table should not be
        # used for the other km
        km_used = [_km.get_hw_id() for _km in ct.key_makers]

        for c_km in ct.key_makers:
            if c_km.shared_km:
                new_km = c_km.shared_km
            else:
                new_km = c_km

            if new_km.hw_id != -1:
                # already allocated (earlier flit)
                total_km_allocated += 1
                continue

            if fid not in c_km.flits_used:
                if ct.is_wide_key and fid == ct.flits_used[0]:
                    # This is first flit of toeplitz/wide table but this km is not for this flit
                    # First two kms must be already allocated, reuse them
                    ncc_assert(len(c_km.flits_used) == 1 )# BUG
                    for _f in c_km.flits_used:
                        ncc_assert(km_used[c_km.km_id % 2] != -1 )# BUG
                        c_km.hw_id = km_used[c_km.km_id % 2]
                        self.km_allocator[_f][c_km.hw_id] = c_km

                total_km_allocated += 1
                continue

            self.gtm.tm.logger.debug("Start _allocate_km[%d] for %s flit %d" % \
                (c_km.km_id, ct.p4_table.name, fid))
            km_allocated = 0
            for _km in self.km_allocator[fid]:
                if _km == None:
                    break
                if _km.shared_km:
                    km = _km.shared_km
                else:
                    km = _km

                # km may be shared *after* is is allocated
                if km.hw_id in km_used:
                    # used by other key-maker of the same table
                    continue
                '''
                # check for sharable key-makers before reusing mutually-exclusive table's km
                if set(km.ctables) <= p_excl_tbls[ct]:
                    new_km.hw_id = km.hw_id
                    km.ctables.append(ct)
                    # allocate km in all flits used
                    for f in new_km.flits_used:
                        self.km_allocator[f][new_km.hw_id] = km
                        self.gtm.tm.logger.debug("%s:[flit %d] reuse key_maker[%d] %d : %s" % \
                            (ct.p4_table.name, f, c_km.km_id, km.hw_id, km.ctables))
                    km_allocated += 1
                    total_km_allocated += 1
                    km_used[c_km.km_id] = new_km.hw_id
                    break
                '''
            if km_allocated != 0:
                continue
            # check other criteria for sharing km between tables
            if need_sharing and self._share_key_maker(fid, ct, new_km, km_used, p_excl_tbls[ct]):
                for f in new_km.flits_used:
                    self.km_allocator[f][new_km.shared_km.hw_id] = new_km.shared_km
                    self.gtm.tm.logger.debug(\
                        "%s:%d:%s:[flit %d] Assign key_maker[%d] %d (shared)" % \
                        (self.gtm.d.name, self.id, ct.p4_table.name, f, c_km.km_id,
                        new_km.shared_km.hw_id))
                total_km_allocated += 1
                km_used[c_km.km_id] = new_km.shared_km.hw_id
                continue

            # check kms of mutually exclusive tables
            for _km in self.km_allocator[fid]:
                if _km == None:
                    continue
                if _km.shared_km:
                    km = _km.shared_km
                else:
                    km = _km
                # km may be shared *after* is is allocated
                if km.hw_id in km_used:
                    # used by other key-maker of the same table
                    continue
                if set(km.ctables) <= p_excl_tbls[ct]:
                    new_km.hw_id = km.hw_id
                    km.ctables.append(ct)
                    km.reuse_mutex_cts.append(ct)
                    for mect in km.ctables:
                        if mect == ct:
                            continue
                        new_km.reuse_mutex_cts.append(mect)
                    # allocate km in all flits used
                    for f in new_km.flits_used:
                        self.km_allocator[f][new_km.hw_id] = km
                        self.gtm.tm.logger.debug("%s:[flit %d] reuse key_maker[%d] %d : %s" % \
                            (ct.p4_table.name, f, c_km.km_id, km.hw_id, km.ctables))
                    km_allocated += 1
                    total_km_allocated += 1
                    km_used[c_km.km_id] = new_km.hw_id
                    break
            if km_allocated != 0:
                continue

            # allocate new key maker
            if self.km_allocator[fid].count(None) == 0:
                return False
            ncc_assert(self.km_allocator[fid].count(None))
            new_km.hw_id = self.km_allocator[fid].index(None)
            self.km_allocator[fid][new_km.hw_id] = new_km
            km_used[c_km.km_id] = new_km.hw_id
            # allocate km in all flits used
            for f in new_km.flits_used:
                self.km_allocator[f][new_km.hw_id] = new_km
                self.gtm.tm.logger.debug("%s:%d:%s:[flit %d] Assign key_maker[%d] %d" % \
                        (self.gtm.d.name, self.id, ct.p4_table.name, f, c_km.km_id, new_km.hw_id))
            km_allocated += 1
            total_km_allocated += 1

        ncc_assert(total_km_allocated == ct.num_km)
        return True

    def get_2B_mode(self, km_prof, ct):
        # return 0: no 2B mode allowed, 1 : 2B mode odd byte, 2: 2B mode even byte
        if len(km_prof.bit_sel):
            # XXX checking with asic team on constraints, not allowed till then
            return 0

        if not ct.is_raw and not ct.is_toeplitz_hash():
            # TBD - support other table types
            return 0

        max_km_width = self.gtm.tm.be.hw_model['match_action']['key_maker_width']
        max_kmB = max_km_width/8
        # Use 2B mode if -
        #   - all byte sels are in pairs
        # If key-maker is not full, then additional bytes can be selected depending on
        # on table type, but need to adjust the key masks etc.. TBD
        _2B_ok = True
        for i in range(len(km_prof.byte_sel)/2):
            b = i*2
            # for Toeplitz hash first 4 bytes are un-used for second flit onwards
            # just ignore them
            if km_prof.byte_sel[b] == -1 and km_prof.byte_sel[b+1] == -1:
                continue
            if km_prof.byte_sel[b+1] != km_prof.byte_sel[b]+1:
                _2B_ok = False
                break
        if _2B_ok:
            return 2    # even bytes in key_maker, 0, 2, 4, ...

        if len(km_prof.byte_sel) > (max_kmB - 2):
            return 0    # will need extra bytes in key_maker for two ends

        # XXX not sure if combining odd-bytes will work... what happens to the solitary
        # byte0 ?
        '''
        pdb.set_trace() # un-tested code
        _2B_ok = True
        for i in range(0, (len(km_prof.byte_sel)/2) - 1):
            b = (i*2) + 1
            if km_prof.byte_sel[i+1] != km_prof.byte_sel[i]+1:
                _2B_ok = False
                break
        if _2B_ok:
            return 1    # combine odd bytes: 1, 3, 5, ...
        '''

        return 0

    def stg_create_key_makers(self):
        ct_list = []
        tbl_id = 1  # avoid 0 for debug
        for t in self.p4_table_list:
            if isinstance(t, p4.p4_conditional_node):
                continue
            ct = self.gtm.tables[t.name]
            ct_list.append(ct)
            if not ct.is_otcam:
                ct.tbl_id = tbl_id
                tbl_id += 1
                if ct.is_multi_threaded:
                    for thd_id in range(1, ct.num_threads):
                        ct.thread_tbl_ids[thd_id] = tbl_id
                        tbl_id += 1

            # create km_profiles (logical) for each table, no hw resources are assigned yet
            ct.ct_create_km_profiles()
            ct.create_key_makers()

        self.ct_list = ct_list
        # for otcam, use same tbl_id as its hash table
        for ct in self.ct_list:
            if ct.is_otcam:
                ct.tbl_id = ct.hash_ct.tbl_id


    def program_tables(self):
        num_flits = self.gtm.tm.be.hw_model['phv']['num_flits']
        km_width  = self.gtm.tm.be.hw_model['match_action']['key_maker_width']
        km_bits  = self.gtm.tm.be.hw_model['match_action']['num_bit_extractors']
        max_kmB = km_width / 8

        key_makers = []
        per_flit_kms = [[] for _ in range(num_flits)]
        per_flit_tables = [[] for _ in range(num_flits)]
        for ct in self.ct_list:
            for km in ct.key_makers:
                if len(ct.flits_used) == 0:
                    # dummy table, no K+I only D and action execution
                    continue
                for fid in km.flits_used:
                    per_flit_kms[fid].append(km)
                    if ct not in per_flit_tables[fid]:
                        # multiple kms per table can cause duplicate addition
                        per_flit_tables[fid].append(ct)

        # check if we need to share key makers
        # Tables that are mutually exlusive can share a key-maker
        # Exclusivity is decided by disjoint flits used by tables and/or predicates
        # for each table, find a list of mutually exclusive tables based on predicates
        # this is done by elimination - start with all tables added to exclusive list and remove
        # tables that are applied together
        p_excl_tbls = OrderedDict()
        for ct in self.ct_list:
            p_excl_tbls[ct] = set(self.ct_list)
        for ctg in self.table_profiles.values():
            for ct in ctg:
                p_excl_tbls[ct] -= set(ctg)

        new_per_flit_kms = [[] for _ in range(num_flits)]
        max_km = self.gtm.tm.be.hw_model['match_action']['num_key_makers']

        for fid, ctg in enumerate(per_flit_tables):
            self.gtm.tm.logger.debug("Stg %d:Per Flit Tables[%d] = %s" % (self.id, fid, ctg))

        # allocate hw_keymaker to each key maker based on flit usage
        # When a lookup is launched in flit x then hardware km can be used for another
        # table in flit x+1
        self.gtm.tm.logger.debug("%s:start km allocation for all tables - %s" %\
            (self.gtm.d.name, per_flit_tables))
        for fid, ctg in enumerate(per_flit_tables):
            for ct in sorted(ctg, key=lambda k:k.num_km, reverse=True):
                if ct.num_km == 0:
                    # XXX need to handle this
                    self.gtm.tm.logger.warning( \
                        "%s:%s is NOT programmed as it does not have key - TBD" % \
                        (self.gtm.d.name, ct.p4_table.name))
                if ct.is_wide_key:
                    # don't share key-makers of the wide-key tables (includes toeplitz)
                    need_sharing = False
                else:
                    #need_sharing = True if fid in km_violation_fid else False
                    # XXX need better way to create per flit mutually exclusive tabels
                    need_sharing = True

                if not self._allocate_km(fid, ct, p_excl_tbls, need_sharing):
                    self.gtm.tm.logger.critical( \
                        "%s:%d:Not enough key-makers for %s Revise P4 program and try" % \
                        (self.gtm.d.name, self.id, ct.p4_table.name))
                    ncc_assert(0)
                    continue

        # XXX calculate the bit_loc for combined KMs - tricky when hash tables are involved
        for ct in self.ct_list:
            if ct.is_overflow:
                continue
            for km in ct.key_makers:
                if km.shared_km:
                    km.shared_km._assign_bit_loc()

        # update key offsets so that all offsets are initialized before sorting and fixing
        # km_profiles based on hardware constraints
        for ct in self.ct_list:
            ct.ct_update_key_offsets()
        ct_list = sorted(self.ct_list, key=lambda c: c.start_key_off)
        for ct in ct_list:
            # do it again in case other shared table moved the bytes in km_profile
            ct.ct_update_key_offsets()
            if ct.is_index_table():
                ct._fix_idx_table_km_profile()
            if ct.is_hash_table():
                ct._fix_hbm_hash_table_km_profile()
            if ct.is_tcam_table() and ct.final_key_size > ct.key_phv_size:
                if ct._fix_tcam_table_km_profile():
                    ct.ct_update_key_offsets()
            if ct.is_tcam_table():
                # Same fix is needed for otcam.. but since the key is shared with hash-table
                # need to fix that too..
                if ct._fix_tcam_table_key():
                    ct.ct_update_key_offsets()

        # re-run key_offset calculation as any table sharing km_profile with index table
        # may have changes in offset
        # skip idx tables since the common routine does not handle i and k bit switch
        for ct in ct_list:
            if not ct.is_index_table():
                ct.ct_update_key_offsets()
                if ct.is_tcam_table():
                    if ct._fix_tcam_table_key():
                        ct.ct_update_key_offsets()
            if ct.is_wide_key:
                ct.ct_update_wide_key_size()

        # Last HACK: Ignore Banyon restriction and try to remove i in key bytes from tcam tables to
        # reduce tcam size
        for ct in ct_list:
            if not ct.is_tcam_table():
                continue
            if ct.remove_i_in_k():
                shared_tables = OrderedDict()
                shared_tables[ct] = 1
                # if ct shares key maker, need to adjust key-offsets of the shared tables.
                for km in ct.key_makers:
                    if km.shared_km:
                        for sct in km.shared_km.ctables:
                            shared_tables[sct] = 1
                for sct in shared_tables:
                    sct.ct_update_key_offsets()
                    if sct.is_tcam_table():
                        sct._fix_tcam_table_key()

        max_km_profiles = self.gtm.tm.be.hw_model['match_action']['num_km_profiles']
        km_prof_normal = []
        km_prof_2B = []
        # use km_profiles in 2Byte mode for raw tables -
        for ct in self.ct_list:
            for km in ct.key_makers:
                if km.shared_km:
                    km_prof = km.shared_km.combined_profile
                else:
                    km_prof = km.combined_profile
                if not km_prof:
                    continue # key-less mpu only table

                if km_prof in km_prof_normal or km_prof in km_prof_2B:
                    # already accounted for - shared
                    continue

                Two_B_mode = 0
                if self.gtm.tm.be.args.two_byte_profile:
                    Two_B_mode = self.get_2B_mode(km_prof, ct)
                if not Two_B_mode:
                    km_prof_normal.append(km_prof)
                else:
                    self.gtm.tm.logger.debug("%s:%s:km %d can use 2B profile" % \
                        (self.gtm.d.name, ct.p4_table.name, km.km_id))
                    km_prof_2B.append(km_prof)

        num_profiles_2B = (len(km_prof_2B) + 1) / 2
        km_profiles_used = num_profiles_2B + len(km_prof_normal)

        # assign hw_ids to km_profiles
        # need to create final hw_profiles with correct byte_sels
        hw_id = 0
        allocated_km_profs = []
        for km_prof in km_prof_normal:
            # Share the same hw_id for identical km_profiles
            for used_km_prof in allocated_km_profs:
                if used_km_prof.byte_sel == km_prof.byte_sel and \
                    used_km_prof.bit_sel == km_prof.bit_sel:
                    # reuse hw_id
                    km_prof.hw_id = used_km_prof.hw_id
                    break
            if km_prof.hw_id >= 0:
                continue
            km_prof.hw_id = hw_id
            km_prof.mode = 0 # normal
            hw_id += 1
            self.hw_km_profiles[km_prof.hw_id] = km_prof
            allocated_km_profs.append(km_prof)

        i = 0
        hw_prof2B = None
        for i, km_prof in enumerate(km_prof_2B):
            km_prof.hw_id = hw_id
            if hw_prof2B == None:
                hw_prof2B = capri_km_profile(self.gtm)
                hw_prof2B.hw_id = hw_id
                km_prof.mode = 1 # use lower half
                hw_prof2B.create_2B_profile(km_prof, True)
                self.hw_km_profiles[km_prof.hw_id] = hw_prof2B
            else:
                km_prof.mode = 2 # use upper half
                hw_prof2B.create_2B_profile(km_prof, False)

            if (i%2):
                hw_id += 1
                hw_prof2B = None

        if hw_id > max_km_profiles:
            # "Not enough km_profiles"
            self.gtm.tm.logger.critical("Not enough km_profiles")
            ncc_assert(0)
        # fix table key-makers E.g. assign otcams/collision tbl to get the same km as its hash table
        for ct in self.ct_list:
            if ct.is_overflow:
                # copy other info that is used by sequencer
                # Add flits from the parent hash, the key of the overflow table is not included
                # in the parent hash table calculations, keep the flit that contains the overflow
                # index
                if not ct.is_otcam:
                    for fid in ct.hash_ct.flits_used:
                        if fid not in ct.flits_used:
                            ct.flits_used.append(fid)
                    ct.flits_used = sorted(ct.flits_used)
                    ct.launch_flit = ct.flits_used[-1]
                    ct.num_km = ct.hash_ct.num_km

        # create flit/cycle launch sequence for each table profile
        for prof_id,ctg in self.table_profiles.items():
            self._create_table_launch_seq(prof_id, ctg)

        # debug printing
        self.gtm.tm.logger.debug(\
            "KM_PROF:Direction,Stage,hw_id,Mode,Table,Type,start_key_off,end_key_off,profile_size")
        for ct in self.ct_list:
            for km in ct.key_makers:
                # most times all flit will use the same profile
                if km.shared_km:
                    km_prof = km.shared_km.combined_profile
                else:
                    km_prof = km.combined_profile
                if not km_prof:
                    continue # key-less mpu only table

                ncc_assert(len(km_prof.byte_sel) <= max_kmB)

                pstr = 'KM_PROF:%s,%d,%d,%d,' % \
                    (self.gtm.d.name, self.id, km_prof.hw_id, km_prof.mode)
                pstr += '%s,%s,%d,%d,%d' % \
                        (ct.p4_table.name, ct.match_type.name, ct.start_key_off,
                        ct.end_key_off,len(km_prof.byte_sel))
                self.gtm.tm.logger.debug(pstr)

        self.gtm.tm.logger.debug("Program Table Info:-----")
        for ct in ct_list:
            ct.ct_print_km_profiles()

        # print km allocation info
        max_km_used = 0; flit_km_used = 0;
        for fid in range(num_flits):
            flit_km_used = max_km - self.km_allocator[fid].count(None)
            max_km_used = max(max_km_used, flit_km_used)

        self.gtm.tm.logger.info("%s:Stage %d: Total Key Maker used %d, Km_profiles used %d" % \
            (self.gtm.d.name, self.id, max_km_used, km_profiles_used))

    def _km_used_in_next_flit(self, fid, ct):
        for _km in ct.key_makers:
            if _km.shared_km:
                km_hw_id = _km.shared_km.hw_id
            else:
                km_hw_id = _km.hw_id

            if self.km_allocator[fid+1][km_hw_id] != None:
                # key-maker is used in the next flit, give this table
                # a preference
                return True
        return False

    def _create_table_launch_seq(self, prof_id, ctg):
        #max_cycles = self.gtm.tm.be.hw_model['match_action']['num_cycles']
        # Use as many cycles as needed - hw allows a total of 192 slots that are to be shared
        # among all profiles
        num_flits = self.gtm.tm.be.hw_model['phv']['num_flits']
        flit_launch_tbls = [[] for _ in range(num_flits)]
        launch_any_time = []
        flits_used = [None for _ in range(num_flits)]
        last_flit_used = 0
        # create list of tables launched per flit for given active tables
        for ct in ctg:
            if ct.is_otcam:
                continue
            if ct.num_km == 0:
                ncc_assert(ct.is_mpu_only())
                ct.key_makers[0].hw_id = 0 # force km0
                launch_any_time.append(ct)
                continue
            flit_launch_tbls[ct.launch_flit].append(ct)
            # add all flits in the range of first and last flit used for a table
            # km cannot be removed evenif byts from in between flit is not loaded
            for fid in range(ct.flits_used[0], ct.flits_used[-1]+1):
                flits_used[fid] = True
                if ct.is_wide_key:
                    # launch_tble is set on each flit for the toeplitz hash and wide key table
                    if ct not in flit_launch_tbls[fid]:
                        flit_launch_tbls[fid].append(ct)

                if fid > last_flit_used:
                    last_flit_used = fid

        #launch_seq = [capri_te_cycle() for _ in range(max_cycles)]
        launch_seq = []
        cycle = 0

        for fid in range(last_flit_used+1):
            #ncc_assert(cycle < max_cycles)
            te_cycle = capri_te_cycle()

            if len(flit_launch_tbls[fid]) == 0:
                if flits_used[fid]:
                    te_cycle.is_used = True
                else:
                    te_cycle.is_used = False
                te_cycle.adv_flit = True
                te_cycle.fid = fid
                if len(launch_any_time):
                    ct = launch_any_time.pop(0)
                    te_cycle.tbl = ct
                launch_seq.append(te_cycle)
                cycle += 1
                continue

            if not flits_used[fid]:
                te_cycle.is_used = False
                te_cycle.adv_flit = True
                te_cycle.fid = fid
                cycle += 1
                launch_seq.append(te_cycle)
                continue

            if len(flit_launch_tbls[fid]) == 1:
                ct = flit_launch_tbls[fid][0]
                for thd in range(1, ct.num_threads):
                    # launch threads (parent aka thread0 will be launched last
                    te_cyc = capri_te_cycle()
                    te_cyc.tbl = ct # need to get table id for thread
                    te_cyc.thread = thd
                    te_cyc.adv_flit = False
                    te_cyc.is_used = True
                    te_cyc.fid = fid
                    launch_seq.append(te_cyc)
                    cycle += 1

                te_cycle.tbl = ct
                te_cycle.is_used = True
                te_cycle.fid = fid
                te_cycle.adv_flit = True
                cycle += 1
                launch_seq.append(te_cycle)
                continue

            # more than 1 table is ready
            # if a key_maker for a table is not used on the following cycle, keep it as
            # last table to launch
            last_ct = None
            if fid < (num_flits -1):
                for ct in flit_launch_tbls[fid]:
                    if not self._km_used_in_next_flit(fid, ct):
                        last_ct = ct
                        break

            for ct in flit_launch_tbls[fid]:
                # launch all tables w/o advancing the flit on each successive cyc
                if ct == last_ct:
                    continue
                for thd in range(1, ct.num_threads):
                    # launch threads (parent aka thread0 will be launched last
                    te_cyc = capri_te_cycle()
                    te_cyc.tbl = ct # need to get table id for thread
                    te_cyc.thread = thd
                    te_cyc.adv_flit = False
                    te_cyc.is_used = True
                    te_cyc.fid = fid
                    launch_seq.append(te_cyc)
                    cycle += 1
                te_cyc = capri_te_cycle()
                te_cyc.tbl = ct
                te_cyc.adv_flit = False
                te_cyc.is_used = True
                te_cyc.fid = fid
                launch_seq.append(te_cyc)
                cycle += 1

            if last_ct:
                for thd in range(1, last_ct.num_threads):
                    # launch threads (parent aka thread0 will be launched last
                    te_cyc = capri_te_cycle()
                    te_cyc.tbl = last_ct # need to get table id for thread
                    te_cyc.thread = thd
                    te_cyc.adv_flit = False
                    te_cyc.is_used = True
                    te_cyc.fid = fid
                    launch_seq.append(te_cyc)
                    cycle += 1
                # this table's key maker is not reused on the following flit
                te_cyc = capri_te_cycle()
                te_cyc.tbl = last_ct
                te_cyc.adv_flit = True
                te_cyc.is_used = True
                te_cyc.fid = fid
                launch_seq.append(te_cyc)
                cycle += 1
            else:
                if fid < last_flit_used:
                    launch_seq[cycle-1].adv_flit = True
                pass

        if len(launch_any_time):
            launch_seq[cycle-1].adv_flit = False

        for ct in launch_any_time:
            # still some key-less mpu-only tables need to be launched
            #ncc_assert(cycle < max_cycles)
            for thd in range(1, ct.num_threads):
                # launch threads (parent aka thread0 will be launched last
                te_cyc = capri_te_cycle()
                te_cyc.tbl = ct # need to get table id for thread
                te_cyc.thread = thd
                te_cyc.adv_flit = False
                te_cyc.is_used = True
                te_cyc.fid = fid
                launch_seq.append(te_cyc)
                cycle += 1
            te_cyc = capri_te_cycle()
            te_cyc.tbl = ct
            te_cyc.adv_flit = False
            te_cyc.is_used = True
            te_cyc.fid = fid
            launch_seq.append(te_cyc)
            cycle += 1

        ncc_assert(len(launch_seq) == cycle )# bug in calculation
        # Advance flit on the very last flit
        #ncc_assert(cycle <= max_cycles)
        launch_seq[cycle-1].adv_flit = True
        self.table_sequencer[prof_id] = launch_seq
        # set last cycle
        last_cyc_used = 0
        for cyc, te_cycle in enumerate(launch_seq):
            if te_cycle.tbl:
                last_cyc_used = cyc
        self.table_sequencer[prof_id][last_cyc_used].is_last = True
        self.gtm.tm.logger.debug("%s:stage %d: profile %d:Last cycle used = %d" % \
            (self.gtm.d.name, self.id, prof_id, last_cyc_used))

        self.gtm.tm.logger.debug("%s:Stage: %d:profile %d:Table sequencer -" % \
            (self.gtm.d.name, self.id, prof_id))

        for cyc, te_cycle in enumerate(launch_seq):
            self.gtm.tm.logger.debug("(%s,%d,%d):%d:%s" % \
                (self.gtm.d.name, self.id, prof_id, cyc, te_cycle))
            if te_cycle.is_last:
                break


    def stg_get_tbl_profile_key(self):
        total_w = 0
        cf_list = []
        max_pred_bits = self.gtm.tm.be.hw_model['match_action']['num_predicate_bits']
        max_pred_phv_bit = self.gtm.tm.be.hw_model['match_action']['range_predicate_bit']
        for cond in self.active_predicates:
            cp = self.gtm.table_predicates[cond]
            for cf,_ in cp.cfield_vals:
                ncc_assert(cf.phv_bit < max_pred_phv_bit)
                if cf not in cf_list:
                    cf_list.append(cf)
                    total_w += cf.width
        ncc_assert(total_w <= max_pred_bits)
        # "Too-many %d bits in predicates, only %d are allowed" % \
        #     (total_w, max_pred_bits)
        self.pred_sel_bits = total_w
        return (total_w, cf_list)

    def stg_create_tbl_profile_tcam_val_mask(self, val):
        # return tcam entries that satisfy the given value
        # each bit in the val represents a condition. 1=True, 0=False
        # Each condition may be represented by multiple bits, when the condition is false,
        # the bits of that condition are set to X (don't care)
        # TCAM is programed starting with most True conditions so that X on false should
        # be covered by corresponding True condition

        test_val = 1 << len(self.active_predicates)
        ncc_assert(val < test_val)

        # same cfield can appear in multiple condition, get list of unique fields and
        # create a map
        total_w, cf_list = self.stg_get_tbl_profile_key()
        cf_val_mask = OrderedDict() # {cf: (val, mask, flag[False = cannot be X]}
        for ki,c_name in enumerate(self.active_predicates):
            test_val >>= 1
            mask = self.table_profile_masks[val]
            cp = self.gtm.table_predicates[c_name]
            is_multibit = True if len(cp.cfield_vals) > 1 else False
            for cf, v in cp.cfield_vals:
                if mask & test_val:
                    # this condition is used (not X)
                    if val & test_val:
                        # condition is true
                        if cf in cf_val_mask and cf_val_mask[cf][1] != 0:
                            ncc_assert(cf_val_mask[cf][0] == v)
                        cf_val_mask[cf] = (v, ((1<<cf.width) - 1))
                    else:
                        # condition is false
                        if cf.width == 1 and not is_multibit:
                            # For a single bit field, false value is opposite (1-v) or True value
                            # Programming this may result in redundant entries in TCAM - XXX optimize
                            if cf in cf_val_mask and cf_val_mask[cf][1] != 0:
                                ncc_assert(cf_val_mask[cf][0] == (1-v))
                            cf_val_mask[cf] = ((1-v), ((1<<cf.width) - 1))
                        else:
                            # multibit false condition is masked out, must have more specific
                            # entry already programmed - XXX
                            if cf not in cf_val_mask:
                                cf_val_mask[cf] = (0, 0)
                else:
                    # this condition is ignored.. mask all fields of this condition
                    if cf not in cf_val_mask:
                        cf_val_mask[cf] = (0,0) # ignored

        tcam_val = 0
        tcam_mask = 0
        for cf in cf_list:
            (v, m) = cf_val_mask[cf]
            tcam_val = (tcam_val << cf.width) | v
            tcam_mask = (tcam_mask << cf.width) | m

        return [(tcam_val, tcam_mask)]

    def prune_impossible_table_profiles(self):
        stage_prof_vals = OrderedDict()
        chk_bit = 1 << (len(self.active_predicates) - 1)
        covered_conditions = OrderedDict()
        for prof_val, ctg in reversed(self.table_profiles.items()):
            mask = self.table_profile_masks[prof_val]
            cf_vals = OrderedDict()
            conflict = False
            for ki,c_name in enumerate(self.active_predicates):
                if mask & (chk_bit >> ki):
                    # this condition is used (not X)
                    cp = self.gtm.table_predicates[c_name]
                    for cf,v in cp.cfield_vals:
                        chk_val = v
                        if prof_val & (chk_bit >> ki) == 0:
                            # checking for condition to be false
                            # invert the 1bit variables, X multibit (XXX)
                            if cf.width > 1:
                                chk_val = None
                            else:
                                chk_val = 1-v

                        if chk_val != None:
                            if cf in cf_vals:
                                if cf_vals[cf] != chk_val:
                                    conflict = True
                                    break
                            else:
                                cf_vals[cf] = chk_val
                    if conflict:
                        break

            if not conflict:
                # The mask indicates the condition that is used. If the condition is checked for
                # 'false' value, the corresponding bit in prof_val is 0.. but it is not same as
                # ignoring it. Use both prof_val and ~prof_val when checking for covered cases
                t = (prof_val & mask, ~prof_val & mask)
                if t in covered_conditions:
                    self.gtm.tm.logger.debug("Ignore prof_val 0x%x covered by 0x%x:" % \
                        (prof_val, covered_conditions[t]))
                    continue
                else:
                    stage_prof_vals[prof_val] = ctg
                    covered_conditions[t] = prof_val
            else:
                self.gtm.tm.logger.debug("Ignore conflicting prof_val 0x%x:" % (prof_val))

        # sort the profiles based on mask to get more specific entries up-top
        # also make sure that for same mask, higher prof value is at the top so that true
        # condition is handled before false (useful for multi-bit false conditions)
        sorted_prof_vals = OrderedDict()
        for prof_val,mask in sorted(self.table_profile_masks.items(),
                                    key=lambda k:(k[1],k[0]), reverse=True):
            if prof_val in stage_prof_vals:
                sorted_prof_vals[prof_val] = stage_prof_vals[prof_val]

        self.table_profiles = copy.copy(sorted_prof_vals)
        self.gtm.tm.logger.debug("%s:%d:Final Table Profiles" % (self.gtm.d.name, self.id))
        for i in self.table_profiles.keys():
            self.gtm.tm.logger.debug("0x%x & 0x%x: %s" % \
                (i, self.table_profile_masks[i],
                [ct.p4_table.name for ct in self.table_profiles[i]]))

    def stg_generate_output(self):
        capri_te_cfg_output(self)

    def stg_te_dbg_output(self):
        max_km_width = self.gtm.tm.be.hw_model['match_action']['key_maker_width']
        max_kmB = max_km_width/8
        # generate table engine debug output
        debug_info = OrderedDict()
        debug_info['Tables'] = OrderedDict()
        for ct in self.ct_list:
            for thd in range(ct.num_threads):
                tbl_dbg_info = OrderedDict()
                if thd == 0:
                    tbl_id_str = '%s' % ct.tbl_id
                else:
                    tbl_id_str = '%s' % (ct.thread_tbl_ids[thd])
                if ct.is_otcam:
                    tbl_id_str += '_%s' % ct.p4_table.name
                if thd == 0:
                    tbl_dbg_info['name'] = ct.p4_table.name
                else:
                    tbl_dbg_info['name'] = ct.p4_table.name + '_thread' + str(thd)
                tbl_dbg_info['type'] = ct.match_type.name
                tbl_dbg_info['start_key_off'] = ct.start_key_off
                tbl_dbg_info['end_key_off'] = ct.end_key_off
                for k,_km in enumerate(ct.key_makers):
                    if _km.shared_km:
                        km = _km.shared_km
                    else:
                        km = _km
                    km_dbg_info = OrderedDict()
                    km_dbg_info['hw_id'] = str(km.hw_id)
                    km_dbg_info['key_maker_bytes'] = OrderedDict()
                    if not km.combined_profile:
                        # key-less table
                        km_dbg_info['km_profile_hw_id'] = str(-1)
                        tbl_dbg_info['key_maker%d' % k] = km_dbg_info
                        debug_info['Tables'][tbl_id_str] = tbl_dbg_info
                        continue

                    gress_pa = self.gtm.tm.be.pa.gress_pa[self.gtm.d]
                    nbytes = (len(km.combined_profile.bit_sel) + 7) / 8
                    for b in range(len(km.combined_profile.byte_sel)):
                        phc = km.combined_profile.byte_sel[b]
                        if phc < 0:
                            if b == km.combined_profile.bit_loc:
                                km_byte = 'BIT_EXTRACTION_0'
                            elif b == km.combined_profile.bit_loc1:
                                km_byte = 'BIT_EXTRACTION_1'
                            else:
                                km_byte = 'UNUSED'
                        else:
                            ftype = None
                            if phc in ct.combined_profile.k_byte_sel:
                                ftype = 'K'
                            elif phc in ct.combined_profile.i1_byte_sel or \
                                phc in ct.combined_profile.i2_byte_sel:
                                ftype = 'I'
                            elif phc in km.combined_profile.k_byte_sel:
                                ftype = 'K(other)'
                            elif phc in km.combined_profile.i1_byte_sel or \
                                phc in km.combined_profile.i2_byte_sel:
                                ftype = 'I(other)'
                            else:
                                ftype = 'unused'

                            km_byte = OrderedDict()
                            for fname, t in gress_pa.phcs[phc].fields.items():
                                fname_str = "%s[%d:%d]" % (fname, t[1], t[1]+t[2]-1)
                                km_byte[fname_str] = ftype
                                km_byte['phv_byte'] = str(phc)
                        km_dbg_info['key_maker_bytes'][str(b)] = km_byte

                    for b in range(len(km.combined_profile.byte_sel), max_kmB):
                        km_dbg_info['key_maker_bytes'][str(b)] = 'UNUSED'

                    km_dbg_info['km_profile_hw_id'] = str(km.combined_profile.hw_id)
                    km_dbg_info['key_maker_bits'] = OrderedDict()
                    for b in range(len(km.combined_profile.bit_sel)):
                        phv_bit = km.combined_profile.bit_sel[b]
                        if phv_bit < 0:
                            km_bit_str = "bit_%d" % b
                            km_dbg_info['key_maker_bits'][str(b)] = {km_bit_str : "unused"}
                            continue
                        phc = phv_bit / 8
                        sb = (phv_bit%8)
                        ftype = None
                        if phv_bit in ct.combined_profile.k_bit_sel:
                            ftype = 'K'
                        elif phv_bit in ct.combined_profile.i_bit_sel:
                            ftype = 'I'
                        elif phv_bit in km.combined_profile.k_bit_sel:
                            ftype = 'K(other)'
                        else:
                            ftype = 'I(other)'
                        flds = gress_pa.phcs[phc].get_flds(sb,1)
                        km_bits = OrderedDict()
                        for fname,t in flds.items():
                            km_bit_str = "%s[%d:%d]" % (fname, t[0], t[0]+t[1]-1)
                            km_bits[km_bit_str] = ftype
                            km_bits['phv_bit'] = str(phv_bit)
                        km_dbg_info['key_maker_bits'][str(b)] = km_bits

                    tbl_dbg_info['key_maker%d' % k] = km_dbg_info
                #Add K+D details
                tbl_dbg_info['k_plus_d'] = ct.k_plus_d
                debug_info['Tables'][tbl_id_str] = tbl_dbg_info

        return debug_info

class capri_te_cycle:
    is_used = False
    tbl = None
    thread = 0
    adv_flit = True
    is_last = False
    fid = -1

    def __repr__(self):
        pstr = '(tbl: %s\t, adv_flit: %s, used: %s, is_last: %s, fid %d)\n' % \
            (self.tbl, self.adv_flit, self.is_used, self.is_last, self.fid)
        return pstr

class capri_gress_tm:
    def __init__(self, tm, d):
        self.d = d
        self.tm = tm
        self.tables = OrderedDict() # {table_name : capri_table}
        self.table_predicates = OrderedDict() # {condition_name : capri_predicate}
        self.stage_tables = None    # table list per logical stage generated from hlir
        self.next_free_refresh_profile = 1 # Next Available Token Refresh profile id
        # hardware information
        # XXX do we need a class to manage resources in each stage??
        self.stages = OrderedDict()
        self.wide_key_tables = []

    def print_tables(self):
        pstr = '%s Tables %d, Tables-Predicates %d\n' % \
            (self.d.name, len(self.tables), len(self.table_predicates))
        pstr += 'Tables:\n'
        for t in self.tables.values():
            pstr += '%s, ' % t.p4_table.name
        self.tm.logger.info('%s\n' % pstr)

        pstr = 'Table-Predicates:\n'
        for t in self.table_predicates.values():
            pstr += '%s, ' % t
        self.tm.logger.info('%s\n' % pstr)

    def validate_table_config(self, table_name=None):
        for tbl in self.tables.values():
            if table_name and tbl.name == table_name:
                tbl.ct_validate_table_config()
                break
            else:
                tbl.ct_validate_table_config()

        # check contraints per stage
        # XXX bit extractors requirement
        # profiles, key-makers, flits
        num_flits = self.tm.be.hw_model['phv']['num_flits']
        flit_size = self.tm.be.hw_model['phv']['flit_size']
        num_key_makers = self.tm.be.hw_model['match_action']['num_key_makers']
        key_maker_width = self.tm.be.hw_model['match_action']['key_maker_width']

    def print_table_info(self, table_name=None):
        self.tm.logger.debug("====== Table Information =====")
        self.tm.logger.debug("Direction,Stage,Name,Id,Type,#Entries,K,(K+I),D,(K+D),SRAM size(b),TCAM size(b),i1(b),i2(b),i_in_key")
        for tbl in self.tables.values():
            if table_name and tbl.name == table_name:
                self.tm.logger.debug(tbl.ct_print_table_parameters())
                break
            else:
                self.tm.logger.debug(tbl.ct_print_table_parameters())

        for tbl in self.tables.values():
            tbl.ct_print_table_ki()

        num_flits = self.tm.be.hw_model['phv']['num_flits']
        self.tm.logger.debug("----- Table Flit Information (%s) -----" % self.d.name)
        hdr_str = 'Table,stage'
        for f in range(num_flits):
            hdr_str += ',flit_'+str(f)
        self.tm.logger.debug(hdr_str)

        for ct in self.tables.values():
            tbl_str = '%s,%d,' % (ct.p4_table.name, ct.stage)
            for f in range(num_flits):
                if f in ct.flits_used:
                    tbl_str += 'Y,'
                else:
                    tbl_str += ' ,'
            self.tm.logger.debug(tbl_str)

        self.tm.logger.debug("----- Table KM Information (%s) -----" % self.d.name)
        for ct in self.tables.values():
            ct.ct_print_km_profiles()
            km_str = '%s: [flit]:(km0,1):' % ct.p4_table.name
            # km_usage 0:not used, 1=in-use, 2=done(cannot use it again)
            km_usage = [0 for _ in range(len(ct.key_makers))]
            for f in range(num_flits):
                km_str += "[%d]:(" % f
                for k,km in enumerate(ct.key_makers):
                    if f in km.flits_used:
                        km_str += "Y,"
                        if km_usage[k] == 2:
                            self.tm.logger.critical("Error - %s:km %d reused at %d" %\
                                (ct.p4_table.name, k, f))
                            #ncc_assert(0)
                        else:
                            km_usage[k] = 1
                    else:
                        km_str += "N,"
                        if km_usage[k] == 1:
                            km_usage[k] = 2
                        else:
                            pass
                km_str += ")"
            self.tm.logger.debug("%s" % km_str)

    def init_tables(self, stage_tables):
        action_id_size = self.tm.be.hw_model['match_action']['action_id_size']
        self.stage_tables = stage_tables
        stg = -1
        for stg_id, table_list in enumerate(stage_tables):
            stg += 1
            if stg not in self.stages:
                self.stages[stg] = capri_stage(self, stg_id)
                self.stages[stg].p4_table_list = sorted(table_list, key=lambda k:k.name)
                self.stages[stg].p4_table_list_unsorted = table_list
            else:
                self.stages[stg].p4_table_list += sorted(table_list, key=lambda k:k.name)
                self.stages[stg].p4_table_list_unsorted += table_list

            for i,t in enumerate(self.stages[stg].p4_table_list):
                if isinstance(t, p4.p4_conditional_node):
                    self.table_predicates[t.name] = capri_predicate(self, t)
                elif isinstance(t, p4.p4_table):
                    ctable = capri_table(self, t, self.d)
                    ctable.stage = stg
                    # add match_fields information, change fields to capri-fields
                    self.tables[t.name] = ctable

                    if t._parsed_pragmas and 'overflow_table' in t._parsed_pragmas:
                        htable = t._parsed_pragmas['overflow_table'].keys()[0]
                        ctable.htable_name = htable
                        ctable.is_overflow = True

                    # Check if table is marked as HBM table.
                    if t._parsed_pragmas and 'hbm_table' in t._parsed_pragmas:
                        ctable.is_hbm = True

                    # Check if table is marked as RAW table.
                    raw_tbl_pc_fld_name = None
                    if t._parsed_pragmas and 'raw_table' in t._parsed_pragmas:
                        raw_tbl_pc_fld_name = t._parsed_pragmas['raw_table'].keys()[0]
                        ctable.is_hbm = True    # must be in hbm ??
                        ctable.is_raw = True

                    if t._parsed_pragmas and 'raw_index_table' in t._parsed_pragmas:
                        ctable.is_hbm = True    # must be in hbm ??
                        ctable.is_raw_index = True

                    # Check if a hash type has been specified for the table.
                    if t._parsed_pragmas and 'hash_type' in t._parsed_pragmas:
                        ctable.hash_type = int(t._parsed_pragmas['hash_type'].keys()[0])
                        ncc_assert(ctable.hash_type <= \
                            self.tm.be.hw_model['match_action']['te_consts']['num_hash_types'], \
                            "%s:Unsupported hash type %d, must be 0-3:CRC hash, 4:Toeplitz" % \
                            (ctable.p4_table.name, ctable.hash_type))

                        if ctable.hash_type == 4:
                            # for toeplitz hash - key and seed are also specified via pragmas
                            # from P4 perspetive, both key and seed are part of the table key
                            ncc_assert('toeplitz_key' in t._parsed_pragmas, \
                                "Specify toeplitz keys using pragma toeplitz_key")
                            key_flds = get_pragma_param_list(t._parsed_pragmas['toeplitz_key'])
                            ncc_assert(len(key_flds), "Error in toeplitz_key pragma")
                            for hfname in key_flds:
                                cf = self.tm.be.pa.get_field(hfname, self.d)
                                ncc_assert(cf, "Invalid field specified as toeplitz key")
                                ctable.toeplitz_key_cfs.append(cf)

                            ncc_assert('toeplitz_seed' in t._parsed_pragmas, \
                                "Specify toeplitz seed fields using pragma toeplitz_seed")
                            seed_flds = get_pragma_param_list(t._parsed_pragmas['toeplitz_seed'])
                            ncc_assert(len(seed_flds), "Error in toeplitz_seed pragma")
                            for hfname in seed_flds:
                                cf = self.tm.be.pa.get_field(hfname, self.d)
                                ncc_assert(cf, "Invalid field specified as toeplitz seed")
                                ctable.toeplitz_seed_cfs.append(cf)

                    # Check if table is marked as HBM table.
                    if t._parsed_pragmas and 'table_write' in t._parsed_pragmas:
                        ctable.is_writeback = True

                    # Check if table is marked as mem only.. i.e. it does not perform phvwrs
                    # so hardware does not have to wait for mpu to finish execution
                    if t._parsed_pragmas and 'memory_only' in t._parsed_pragmas:
                        ctable.is_memory_only = True

                    # Check if a hash type has been specified for the table.
                    if t._parsed_pragmas and 'policer_table' in t._parsed_pragmas:
                        if t._parsed_pragmas['policer_table'].keys()[0] == 'three_color':
                            ctable.policer_colors = 3
                        else:
                            ctable.policer_colors = 2
                        ctable.is_policer = True

                    # Check if rate limit is enabled for this table.
                    if t._parsed_pragmas and 'enable_rate_limit' in t._parsed_pragmas:
                        ctable.is_rate_limit_en = True

                    if ctable.is_policer or ctable.is_rate_limit_en:
                        if self.next_free_refresh_profile >= 16:
                            ncc_assert(0, "%s: Can't have more than 15 Policer/Rate-Limit tables" % (self.d.name))

                        ctable.token_refresh_profile = self.next_free_refresh_profile
                        self.next_free_refresh_profile += 1


                    if ctable.is_policer or ctable.is_rate_limit_en:
                        if t._parsed_pragmas and 'token_refresh_rate' in t._parsed_pragmas:
                            ctable.token_refresh_rate = int(t._parsed_pragmas['token_refresh_rate'].keys()[0])
                            ncc_assert(ctable.token_refresh_rate <= 4000, \
                                       "token refresh rate should be less than 4000/sec")
                    else:
                        if t._parsed_pragmas and 'token_refresh_rate' in t._parsed_pragmas:
                            self.tm.logger("%s:%s pragma token_refresh_rate has no effect.. ignored" % \
                                           (self.d.name, t.name))

                    if t._parsed_pragmas and 'numthreads' in t._parsed_pragmas:
                        ctable.num_threads = int(t._parsed_pragmas['numthreads'].keys()[0])
                        ncc_assert(ctable.num_threads > 0 and ctable.num_threads <= 4, \
                            "Max 4 threads per table are supported")
                        # No need to make it multithreaded if only 1 thread is specified
                        if ctable.num_threads > 1:
                            ctable.is_multi_threaded = True
                        else:
                            self.tm.logger("%s:%s pragma numthreads 1 has no effect.. ignored" % \
                                (self.d.name, t.name))

                    is_ternary = False
                    for f, mtype, mask, in t.match_fields:
                        if mtype != p4.p4_match_type.P4_MATCH_EXACT and \
                            mtype != p4.p4_match_type.P4_MATCH_VALID and \
                            mtype != p4.p4_match_type.P4_MATCH_TERNARY:
                            self.tm.logger.critical("%s:Unsupported match type %s for %s:%s" % \
                                (self.d.name, mtype, t.name, get_hfname(f)))
                            ncc_assert(0)
                        if mtype != p4.p4_match_type.P4_MATCH_VALID:
                            hf_name = get_hfname(f)
                            cf = self.tm.be.pa.get_field(hf_name, self.d)
                            ncc_assert(cf, "%s:%s not found" % (self.d.name, hf_name))
                            ctable.keys.append((cf, mtype, mask))
                            cf.is_key = True
                            if mtype == p4.p4_match_type.P4_MATCH_TERNARY:
                                is_ternary = True
                        else:
                            # When ternary match is used, hlir gives pseudo field which gets
                            # handled in the if condition above. Handle exact match here
                            ncc_assert(isinstance(f, p4.p4_header_instance))
                            hf_name = f.name + '.valid'
                            cf = self.tm.be.pa.get_field(hf_name, self.d)
                            ncc_assert(cf)
                            ctable.keys.append((cf, mtype, mask))

                    ctable.num_entries = t.min_size if t.min_size else t.max_size
                    key_cfs = [cf for cf,_,_ in ctable.keys]
                    key_size = 0
                    for cf, mtype, mask in ctable.keys:
                        # compute key size (w/o pad adjustments)
                        # No hdr/fld unions supported for wide key XXX
                        if not cf.is_union():
                            key_size += (cf.width-cf.pad)
                        if cf.is_hdr_union and not is_ternary:
                            self.tm.logger.warning('%s:CAUTION:Hdr union field %s in tbl key %s' % \
                                (self.d, cf.hf_name, t.name))

                    if key_size > 512: # use model param XXX
                        ctable.is_wide_key = True
                        # record initial key_size - over-written later
                        # does not take into a/c unions etc.. will be inaccurate
                        ctable.key_size = key_size
                        ncc_assert(ctable not in self.wide_key_tables)
                        self.wide_key_tables.append(ctable)
                        for cf, _, _ in ctable.keys:
                            cf.is_wide_key = True
                    if is_ternary:
                        ctable.match_type = match_type.TERNARY
                        if ctable.is_overflow:
                            ctable.is_otcam = True
                    elif ctable.is_raw:
                        # raw table is a special 'index' table
                        ctable.match_type = match_type.EXACT_IDX
                        # add the metadata.mpu_pc to the keys of the raw table, remove it later
                        # while programming (HW may need the PC at a specific location in key-maker)
                        cf = self.tm.be.pa.get_field(raw_tbl_pc_fld_name, self.d)
                        ncc_assert(cf, "%s:%s not found (used as raw table mpu_pc" % \
                            (self.d.name, raw_tbl_pc_fld_name))
                        raw_pc_size = self.tm.be.hw_model['match_action']['raw_pc_size']
                        ncc_assert(cf.width == raw_pc_size, "raw table pc field must be 32 bit")
                        ncc_assert(len(ctable.keys) == 1, \
                            "Only one field is allowed as key for a raw table")
                        # mask not supported, use it as 0 while adding mpu_pc as key
                        # mpu pc appears before the key
                        ctable.keys.insert(0, (cf, p4.p4_match_type.P4_MATCH_EXACT, 0))
                    elif ctable.is_raw_index:
                        # this is a special index table where the index is the address in mem
                        ctable.match_type = match_type.EXACT_IDX
                        for cf in key_cfs:
                            cf.is_index_key = True
                    elif len(ctable.keys) != 0:
                        # set as exact hash and later check index vs hash based on key
                        # and table size
                        ctable.match_type = match_type.EXACT_HASH
                        if 'index_table' in t._parsed_pragmas or \
                            (key_size < 32 and (ctable.num_entries >= (1<<key_size))):
                            ctable.match_type = match_type.EXACT_IDX
                            if key_size > 16:
                                ncc_assert(len(key_cfs) == 1, \
                                    "Only one key field is supported for index table %s with idx > 16" % \
                                    ctable.p4_table.name)
                            for cf in key_cfs:
                                cf.is_index_key = True
                    else:
                        ctable.match_type = match_type.MPU_ONLY

                    _input_flds, _out_flds = t.retrieve_action_fields()
                    input_flds = sorted(_input_flds, key=lambda k: k.name)
                    out_flds = sorted(_out_flds, key=lambda k: k.name)
                    i_size = 0
                    for f in input_flds:
                        # can be p4_field or p4_pseudo_field(.valid)
                        hf_name = get_hfname(f)
                        cf = self.tm.be.pa.get_field(hf_name, self.d)
                        ncc_assert(cf)
                        if cf.is_scratch:
                            continue
                        if cf not in ctable.input_fields and cf not in key_cfs:
                            cf.is_input = True
                            if ctable.is_wide_key:
                                cf.is_wide_input = True
                            ctable.input_fields.append(cf)
                            if not cf.is_union():
                                i_size += cf.width
                    if ctable.is_wide_key:
                        # record initial i_size - over-written later
                        ctable.i_size = i_size

                    for f in out_flds:
                        # out fields are not really useful at this time since action can write
                        # anywhere in phv
                        if isinstance(f, p4.p4_field):
                            hf_name = get_hfname(f)
                            cf = self.tm.be.pa.get_field(hf_name, self.d)
                            ncc_assert(cf)
                            if cf.is_scratch:
                                continue
                            if cf not in ctable.action_output:
                                ctable.action_output.append(cf)
                        else:
                            # valid bit outputs are psuedo fields
                            pass
                    # collect all the metadata flds from k and i
                    for cf in key_cfs:
                        if cf.is_meta:
                            ctable.meta_fields.append(cf)
                    for cf in ctable.input_fields:
                        if cf.is_meta:
                            ctable.meta_fields.append(cf)

                    data_size = 0
                    le_action_params = {}
                    for act in t.actions:
                        actiondata_api = False
                        appdata_api = False
                        if 'little_endian' in act._parsed_pragmas:
                            le_action_params[act.name] = \
                                                  get_pragma_param_list(act.\
                                                   _parsed_pragmas['little_endian'])

                        action_data = zip(act.signature, act.signature_widths)
                        ctable.action_data[act.name] = []
                        ad_size = 0
                        for ad in action_data:
                            if ad in ctable.action_data[act.name]:
                                continue
                            ctable.action_data[act.name].append(ad)
                            ad_size += ad[1]
                        if ad_size > data_size:
                            data_size = ad_size

                        if 'capi' in act._parsed_pragmas:
                            appfields = []
                            if 'appdatafields' in act._parsed_pragmas['capi']:
                                appdata_api = True
                                appfields = get_pragma_param_list(
                                           act._parsed_pragmas['capi']['appdatafields'])
                            if 'hwfields_access_api' in act._parsed_pragmas['capi']:
                                actiondata_api = True
                        if appdata_api:
                            appdata_fields = {}
                            appdatafields = []
                            for ad in action_data:
                                if ad[0] in appfields:
                                    appdatafields.append(ad)
                            appdata_fields[act.name] = appdatafields
                        if actiondata_api:
                            hw_fields = {}
                            hwfields = []
                            for ad in action_data:
                                if ad[0] not in appfields:
                                    hwfields.append(ad)
                            hw_fields[act.name] = hwfields

                    ctable.le_action_params = le_action_params
                    if appdata_api:
                        ctable.action_appfields = appdata_fields
                    if actiondata_api:
                        ctable.action_hwfields  = hw_fields

                    ctable.d_size = data_size
                    if ctable.num_actions() > 1:
                        if not ctable.is_raw:
                            ctable.d_size += action_id_size

                    if len(ctable.p4_table.attached_meters):
                        if ctable.d_size != 0:
                            self.tm.logger.warning("XXX-Meter table %s is using action data" % \
                                (ctable.p4_table.name))
                        ctable.d_size += 128    # capri-hw uses 128b wide mem for meters - XXX
                else:
                    ncc_assert(0)

        # match overflow-tcam and hash tables
        for ct in self.tables.values():
            if not ct.is_overflow:
                continue
            # find hash table
            ncc_assert(ct.htable_name in self.tables, \
                "No parent hash table %s for overflow table %s" % \
                 (ct.p4_table.name, ct.htable_name))
            ct.hash_ct = self.tables[ct.htable_name]
            ncc_assert(ct.hash_ct.match_type == match_type.EXACT_HASH)
            if ct.is_otcam:
                ct.hash_ct.match_type = match_type.EXACT_HASH_OTCAM
                ct.hash_ct.otcam_ct = ct
            else:
                collision_key_size = self.tm.be.hw_model['match_action']['collision_index_sz']
                ct.hash_ct.collision_ct = ct
                ct.match_type = match_type.EXACT_HASH
                ncc_assert(len(ct.keys) == 1, \
                    "Only single %dbit key is allowed for overflow hash table %s" % \
                    (collision_key_size, ct.p4_table.name))
                cf = ct.keys[0][0]
                ct.hash_ct.collision_cf = cf
                ncc_assert(cf.width == collision_key_size, \
                    "Invalid key width %d (allowed %d) for overflow hash table %s" % \
                    (cf.width, collision_key_size, ct.p4_table.name))
                # remove all the input_fields from the collision table, these will come from the
                # hash table's action routine
                ct.input_fields = []
                # key of the overflow table is NOT added to its parent table
                # This is done by adding kms of the two table later. If the keys are added
                # here to parent table, parent tables key placement and bit extraction
                # screwes-up the required alignement
                '''
                # Need to adjust the start_key_off before programming hw
                if (cf,p4.p4_match_type.P4_MATCH_EXACT, -1) not in ct.hash_ct.keys:
                    ct.hash_ct.keys.append((cf,p4.p4_match_type.P4_MATCH_EXACT, -1))
                if cf in ct.hash_ct.input_fields:
                    ct.hash_ct.input_fields.remove(cf)
                '''

        self.print_tables()

    def update_table_config(self):
        for ctable in self.tables.values():
            ctable.ct_update_table_config()
        self.update_table_predicates()

    def update_table_predicates(self):
        table_paths = OrderedDict()
        all_tables = []
        for stg in self.stages.keys():
            all_tables += self.stages[stg].p4_table_list_unsorted
        table_paths = self.find_table_paths(all_tables)

        # go thru' each path and collect all the conditions that apply to a given
        # table along a path. Each path may encounter different conditions.
        # Conditions that are localized to a set of tables, will not affect execution
        # of tables before or after it. These are changed to 'X' (don't care), for those
        # tables
        table_preds = OrderedDict() # {table : [{pred : val}, ..]}
        for k in self.tables.keys():
            table_preds[k] = []
        for path in table_paths:
            # collect predicate values for each table along each path
            pred_vals = OrderedDict()
            for k in self.table_predicates.keys():
                pred_vals[k] = 2 # {pred : F=0/T=1/X=2}

            for i, n in enumerate(path):
                if isinstance(n, p4.p4_conditional_node):
                    if i == (len(path) - 1):
                        continue
                    if n.next_[False] and path[i+1] == n.next_[False]:
                        pred_vals[n.name] = 0
                    if n.next_[True] and path[i+1] == n.next_[True]:
                        pred_vals[n.name] = 1
                else:
                    # reduce the predicate conditions to keep only those bits
                    # that create unique combinations. i.e
                    # if a table is executed for both True and False values of a predicate
                    # this that predicate(condition) is don't care
                    tp_list = table_preds[n.name]
                    for tp in tp_list:
                        for pk, pv in tp.items():
                            if pred_vals[pk] == 2 or pred_vals[pk] != pv:
                                # don't care this predicate value
                                tp[pk] = 2
                                pred_vals[pk] = 2

                    if pred_vals.values().count(2) != len(pred_vals.values()) and \
                        pred_vals not in tp_list:
                        # Add if some bits are not don't care
                        tp_list.append(copy.copy(pred_vals))

        self.tm.logger.debug("Table predication values (False =0, True=1 X=don't care)")
        for t,pred_vals in table_preds.items():
            for v in pred_vals:
                self.tm.logger.debug("%s: %s" % (t, [pv if pv != 2 else 'x' for pv in v.values()]))
        # create a list of active conditions per stage. Look at condition variable (predicates)
        # that affect tables in a given stage
        for stg, cstage in self.stages.items():
            table_list = self.stages[stg].p4_table_list
            if len(table_list) == 0:
                # no table in this stage
                continue
            # create profiles per stage
            stage_table_profiles = OrderedDict() # {predicate_val : [table_name list]}
            stage_profile_masks = OrderedDict() # {predicate_val : mask(1=use, 0=X)}
            active_preds = []
            for k in self.table_predicates.keys():
                # XXX prune the condtions that are always X for all tables in a stage
                for t in table_list:
                    if isinstance(t, p4.p4_conditional_node):
                        continue
                    if k in active_preds:
                        break
                    for tp in table_preds[t.name]:
                        if tp[k] != 2:
                            active_preds.append(k)
                            break

            self.tm.logger.debug("%s:Stage %d: Active Conditions %s" % \
                (self.d.name, stg, active_preds))

            if len(active_preds) == 0:
                # no predication, exececute all tables
                self.tm.logger.info("%s:Stage %d: Execute all tables" % \
                    (self.d.name, stg))
                self.stages[stg].active_predicates = copy.copy(active_preds)
                stage_table_profiles[0] = [self.tables[t.name] for t in table_list \
                        if not isinstance(t, p4.p4_conditional_node)]
                self.stages[stg].table_profiles = copy.copy(stage_table_profiles)
                self.tm.logger.debug('%s:Stage %d: X : %s' % \
                    (self.d.name, stg,
                    [ct.p4_table.name for ct in stage_table_profiles[0]]))
                continue


            # brute force method to create a truth table
            max_val = (1 << len(active_preds))
            chk_bit = (1<<(len(active_preds)-1))
            for i in range(max_val):
                # check each combination with table predicates and add table to the list if
                # this combination satifies the table condtions
                for t in table_list:
                    if isinstance(t, p4.p4_conditional_node):
                        continue
                    match = True
                    match_mask = 0  # bit=0 => don't care
                    for ki, pk in enumerate(active_preds):
                        if not(i & (chk_bit >> ki)):
                            for tp in table_preds[t.name]:
                                if tp[pk] == 1:
                                    match = False
                                    break
                        else: #i & (chk_bit >> ki):
                            for tp in table_preds[t.name]:
                                if tp[pk] == 0:
                                    match = False
                                    break
                        if tp[pk] != 2:
                            match_mask |= (chk_bit >> ki)
                        if not match:
                            break

                    if match:
                        if i in stage_table_profiles:
                            stage_profile_masks[i] |= match_mask
                            stage_table_profiles[i].append(self.tables[t.name])
                        else:
                            stage_table_profiles[i] = [self.tables[t.name]]
                            stage_profile_masks[i] = match_mask

                if i in stage_table_profiles:
                    self.tm.logger.debug("0x%x & 0x%x: %s" % \
                        (i, stage_profile_masks[i],
                        [ct.p4_table.name for ct in stage_table_profiles[i]]))

            self.stages[stg].active_predicates = copy.copy(active_preds)
            self.stages[stg].table_profiles = copy.copy(stage_table_profiles)
            self.stages[stg].table_profile_masks = copy.copy(stage_profile_masks)
            self.stages[stg].prune_impossible_table_profiles()

    def find_table_paths(self, table_list):
        def _find_paths(node, paths, current_path, tables_visited):
            ncc_assert(node not in current_path, "Table LOOP at %s" % node.name)
            tables_visited.append(node)
            if isinstance(node, p4.p4_conditional_node):
                for nxt_node in node.next_.values():
                    if not nxt_node:
                        paths.append(current_path + [node])
                        continue
                    _find_paths(nxt_node, paths, current_path+[node], tables_visited)
            else:
                # no support for action function dependency, so really it should be just one
                # next node
                ncc_assert(len(set(node.next_.values())) == 1)
                for nxt_node in set(node.next_.values()):
                    if not nxt_node:
                        paths.append(current_path + [node])
                        continue
                    _find_paths(nxt_node, paths, current_path+[node], tables_visited)

        paths = []
        current_path = []
        tables_visited = []
        if len(table_list):
            for p4t in table_list:
                if p4t in tables_visited:
                    continue
                _find_paths(p4t, paths, current_path, tables_visited)
        return paths

    def create_key_makers(self):
        for stg in self.stages.keys():
            self.stages[stg].stg_create_key_makers()

        # Fix the key makers for overflow/collision hash tables
        # Overflow tcam and parent hash must be in the same stage and share table-id. BUT,
        # collision hash (overflow) table can be in a different stage than its parent hash
        # table. It can be in a earlier/same stage when used via recirc OR it can be in a
        # later stage when used on detecting collision
        # Fix the keymakers for all the collision tables and their parent hash tables across
        # all stages
        for stg in self.stages.values():
            for ct in stg.ct_list:
                if ct.is_overflow:
                    # copy the overflow index key into parent hash.
                    # key-maker merge logic is used so that required information is
                    # also updated in the key-maker for otcam, just copy parent key-makers
                    # to the otcam table (used by API gen code)
                    if ct.hash_ct.is_wide_key:
                        # for wide key table there cannot be otcam
                        # the overflow table will only receive the information from the last flit
                        # so merge the overflow table key in last km0
                        # for GFT program this does not work.. since overflow table is not in the
                        # same stage using recirc... need different way to initialize this
                        ncc_assert(not ct.is_otcam)
                        ncc_assert(ct.num_km == 1)
                        km0 = ct.hash_ct.key_makers[-2]
                        km1 = ct.hash_ct.key_makers[-1]
                        km0._merge(ct.key_makers[0], is_overflow_key_merge=True)
                        km0.ctables.remove(ct)
                        if km0.combined_profile.bit_loc != -1:
                            # need to recompute the bit location after the merge
                            # merge removes -1 inserted at bit_loc
                            km0.combined_profile.bit_loc = -1
                            km0.combined_profile.bit_loc1 = -1
                            km0.combined_profile._update_bit_loc_key_off()

                        for k in range(0, len(ct.hash_ct.key_makers)-2):
                            km = ct.hash_ct.key_makers[k].copy_km()
                            km.km_id = k
                            km.ctables.append(ct)
                            if k < ct.num_km:
                                ct.key_makers[k] = km
                            else:
                                ct.key_makers.append(km)

                        km_id = len(ct.key_makers)
                        km = km0.copy_km()
                        km.km_id = km_id
                        km.ctables.append(ct)
                        km_id += 1

                        ct.key_makers.append(km)

                        km = km1.copy_km()
                        km.km_id = km_id
                        km.ctables.append(ct)

                        ct.key_makers.append(km)
                        ct.num_km = len(ct.key_makers)
                        ct.is_wide_key = True
                        # copy flit used info from parent hash
                        ct.flits_used = copy.copy(ct.hash_ct.flits_used)
                        continue

                    for k,_km in enumerate(ct.hash_ct.key_makers):
                        if ct.is_otcam:
                            ct.key_makers.append(_km)
                            continue
                        if k < ct.num_km:
                            _km._merge(ct.key_makers[k], is_overflow_key_merge=True)
                            ct.key_makers[k].has_overflow_key = True
                            _km.ctables.remove(ct)
                            if _km.combined_profile.bit_loc != -1:
                                # need to recompute the bit location after the merge
                                # merge removes -1 inserted at bit_loc
                                _km.combined_profile.bit_loc = -1
                                _km.combined_profile.bit_loc1 = -1
                                _km.combined_profile._update_bit_loc_key_off()

                            km = _km.copy_km()
                            km.ctables.append(ct)
                            km.km_id = k
                            ct.key_makers[k] = km
                        else:
                            km = _km.copy_km()
                            km.ctables.append(ct)
                            km.km_id = k
                            ct.key_makers.append(km)

                    ct.num_km = len(ct.key_makers)

                    # fix km flits_used flags
                    tbls = [ct, ct.hash_ct]
                    for xct in tbls:
                        for km in xct.key_makers:
                            start_flit = km.flits_used[0]
                            end_flit = km.flits_used[-1]
                            for _f in range(start_flit, end_flit+1):
                                if _f not in km.flits_used:
                                    km.flits_used.append(_f)
                            km.flits_used = sorted(km.flits_used, key=lambda k: k)


    def program_tables(self):
        for stg in self.stages.keys():
            self.stages[stg].program_tables()
        self.print_table_info()
        self.validate_table_config()

    def generate_output(self):
        # generate table-engine configuration output per-stage
        for stg in self.stages.values():
            stg.stg_generate_output()

    def te_dbg_output(self):
        # generate table engine debug output
        debug_info = OrderedDict()
        for stg in self.stages.values():
            debug_info['stage%d' % stg.id] = stg.stg_te_dbg_output()
        return debug_info

    def tm_k_plus_d_fields_add(self, k_plus_d_dict):
        for ct in self.tables.values():
            if ct.p4_table.name in k_plus_d_dict.keys():
                ct.k_plus_d = k_plus_d_dict[ct.p4_table.name]

class capri_table_mapper:
    def __init__(self, tmgr):
        self.be = tmgr.be
        self.tmgr = tmgr
        self.tables = OrderedDict()
        self.memory = OrderedDict()
        self.logger = logging.getLogger('TableMapper')
        spec = self.tmgr.table_memory_spec
        for mem_type, regions in spec.iteritems():
            self.memory[mem_type] = OrderedDict()
            for region, value in regions.iteritems():
                self.memory[mem_type][region] = OrderedDict()
                self.memory[mem_type][region]['space'] = []
                self.memory[mem_type][region]['depth'] = spec[mem_type][region]['depth']
                self.memory[mem_type][region]['blk_d'] = spec[mem_type][region]['depth']
                self.memory[mem_type][region]['blk_c'] = spec[mem_type][region]['count']
                self.memory[mem_type][region]['blk_w'] = table_width_to_allocation_units(mem_type, spec[mem_type][region]['width'])
                self.memory[mem_type][region]['width'] = table_width_to_allocation_units(mem_type, spec[mem_type][region]['width'] *
                                                                                                   spec[mem_type][region]['count'])

        self.tables = {'sram':{'ingress':[],
                                'egress':[]},
                       'tcam':{'ingress':[],
                                'egress':[]},
                       'hbm' :{'02-p4_tables':[]}}

    def validate_placement(self):
        for mem_type in self.tables:
            if mem_type == 'hbm':
                continue
            for region in self.tables[mem_type]:
                tables = self.tables[mem_type][region]
                for i in range(len(tables)):
                    if not tables[i]['layout']:
                        self.logger.info("Mapping not complete")

    def get_next_unplaced_table(self, tables, i):
        for i in range(i+1, len(tables)):
            if tables[i]['width'] <= 0 or tables[i]['depth'] <= 0:
                continue

            if not tables[i]['layout']:
                return i

        return len(tables)

    def carve_dummy(self):
        layout = OrderedDict()
        layout['top_left'] = {'block' : 0, 'x' : 0, 'y' : 0}
        layout['bottom_right'] = {'block' : 0, 'x' : 0, 'y' : 0}
        return layout

    def carve_memory(self, memory, top, left, bottom, right, index):

        mem_chunk = memory['blk_w']
        mem_space = memory['space']

        for row in range(top, bottom + 1):
            for col in range(left, right + 1):
                mem_space[row][col] = index + 65 # 65 is ASCII for A

        layout = OrderedDict()
        layout['top_left'] = {'block' : left/mem_chunk, 'x' : left % mem_chunk, 'y' : top}
        layout['bottom_right'] = {'block' : right/mem_chunk, 'x' : right % mem_chunk, 'y' : bottom}
        return layout

    # Allocates space for a table as per requirements
    def scavenge_unused_space(self, mem_type, region, table_depth, table_width, entry_width, index):
        # Get the memory space and its properties
        memory = self.memory[mem_type][region]
        mem_depth = memory['depth']
        mem_width = memory['width']
        mem_space = memory['space']

        # Scan row by row
        for top in range(mem_depth - table_depth + 1):
            # Scan columns looking for free position to start the table at
            for left in range(mem_width - table_width + 1):

                failed = False
                for entry_left in range(left, left + table_width, entry_width):
                    if ((entry_left % get_block_width(mem_type)) + entry_width) > (get_block_width(mem_type) * 4):
                        failed = True # Table entries can only span a max of 4 blocks
                        break

                if failed:
                    continue

                if mem_space[top][left] == 0:
                    # Start position found. Now continue scanning for the required width
                    success = True
                    right   = left # Not required since table_width is always > 0. But keeps the code analyzer happy.
                    for right in range(left, left + table_width):
                        # Required width not available. Look for a new Start position after this position
                        if not mem_space[top][right] == 0:
                            success = False
                            left = right
                            break

                    if success:
                        # Required width is found, scan rows to ensure enough rows are available at the same columns
                        bottom = top # Not required since table_depth is always > 0. But keeps the code analyzer happy.
                        for bottom in range(top, top + table_depth):
                            for pos in range(left, right + 1):
                                # Required depth not available at the selected columns. Go back and try new columns/rows
                                if not mem_space[bottom][pos] == 0:
                                    success = False
                                    break

                            if not success:
                                break

                        if success:
                            return self.carve_memory(memory, top, left, bottom, right, index)
                        else:
                            break

        return OrderedDict()

    def insert_table(self, type, region, table):
        tables = self.tables[type][region]
        for i in range(len(tables)):
            if  ((tables[i]['width'] * tables[i]['depth']) < (table['width'] * table['depth'])):
                return self.tables[type][region].insert(i, table)

        return self.tables[type][region].append(table)

    def load_tables(self, table_specs):
        for tspec in table_specs:
            if 'tcam' in tspec.keys():
                depth = pad_to_64(tspec['num_entries'])
                width = tspec['tcam']['width']
                ctable = self.tmgr.gress_tm[xgress_from_string(tspec['region'])].tables[tspec['name']]
                if ((ctable.start_key_off / 8) & 0x01):
                    width += 1 # Pad if key doesnt start from an even byte boundary
                width = table_width_to_allocation_units('tcam', width)
                self.insert_table('tcam', tspec['region'], {'name':tspec['name'],
                                                            'stage':tspec['stage'],
                                                            'width':width,
                                                            'depth':depth,
                                                            'layout':OrderedDict()})
            if 'sram' in tspec.keys():
                if tspec['overflow_parent']:
                    continue # Overflow SRAMs are appended to their parent SRAMs.
                depth = tspec['num_entries']
                if tspec['overflow']:
                    for temp in table_specs:
                        if tspec['overflow'] == temp['name']:
                            depth += temp['num_entries']
                width = table_width_to_allocation_units('sram', tspec['sram']['width'])
                ctable = self.tmgr.gress_tm[xgress_from_string(tspec['region'])].tables[tspec['name']]
                if ctable.match_type == match_type.EXACT_HASH:
                    if width > 512:
                        depth = depth * (pad_to_x(width, 512) / 512)
                        width = 512
                if ctable.is_policer or ctable.is_writeback:
                    width = pad_to_x(width, self.memory['sram'][tspec['region']]['blk_w'])
                self.insert_table('sram', tspec['region'], {'name':tspec['name'],
                                                               'stage':tspec['stage'],
                                                               'width':width,
                                                               'depth':depth,
                                                               'layout':OrderedDict()})
            if 'hbm' in tspec.keys():
                width = pad_to_power2(tspec['hbm']['width'])
                width = table_width_to_allocation_units('hbm', width)
                self.insert_table('hbm', tspec['region'], {'name':tspec['name'],
                                                              'stage':tspec['stage'],
                                                              'width':width,
                                                              'depth':tspec['num_entries'],
                                                              'layout':OrderedDict()})

    def place_tcam_table(self, region, start, i, req_width, req_depth, available_width, available_depth):

        # Tries to fit tables such that combinations of tables are selected to fit a full row first

        top = start['top']
        left = start['left']
        tables = self.tables['tcam'][region]
        memory = self.memory['tcam'][region]

        if req_width > available_width:
            # Too wide to fit
            return

        # When here, the available_width is too small to fit another fold of the table, or the table is already only one row deep

        if tables[i]['layout'] or req_depth > available_depth:
            # Return if the table has already been placed, or if the depth required is more than the depth available.
            return

        if req_width < available_width:
            # If theres space left, pick the next widest table not yet placed and try to fit it there.
            jstart = {'top': top, 'left': left + req_width}
            j = self.get_next_unplaced_table(tables, i)
            while j < len(tables) and not tables[j]['layout']:
                self.place_tcam_table(region, jstart, j,
                                          tables[j]['width'], tables[j]['depth'],
                                          available_width - req_width, req_depth)

                if not tables[j]['layout']:
                    j = self.get_next_unplaced_table(tables, j)

        if req_depth < available_depth:
            # If there are rows left, pick the next widest table not yet placed and try to fit it there.
            jstart = {'top': top + req_depth, 'left': left}
            j = self.get_next_unplaced_table(tables, i)
            while j < len(tables) and not tables[j]['layout']:
                self.place_tcam_table(region, jstart, j,
                                          tables[j]['width'], tables[j]['depth'],
                                          available_width, available_depth - req_depth)

                if not tables[j]['layout']:
                    j = self.get_next_unplaced_table(tables, j)

        # When here, we have filled as much as we can of the free space to the right of the table. Now place this table

        tables[i]['layout'] = self.carve_memory(memory, top, left, top + req_depth - 1, left + req_width - 1, i)
        start['top'] += req_depth
        return

    def place_sram_table(self, region, start, i, req_width, req_depth, available_width, available_depth):

        top = start['top']
        left = start['left']
        tables = self.tables['sram'][region]
        memory = self.memory['sram'][region]

        if req_width > available_width:
            # Too wide to fit
            return

        if req_depth > 1:
            # If the table has more than one row, fold the table twice in the same row(s), at the expense of the width.
            self.place_sram_table(region, start, i, req_width*2, req_depth/2, available_width, available_depth)

        # When here, the available_width is too small to fit another fold of the table, or the table is already only one row deep

        if tables[i]['layout'] or req_depth > available_depth:
            # Return if the table has already been placed, or if the depth required is more than the depth available.
            return

        if req_width < available_width:
            # If theres space left, pick the next widest table not yet placed and try to fit it there.
            jstart = {'top': top, 'left': left + req_width}
            j = self.get_next_unplaced_table(tables, i)
            while j < len(tables) and not tables[j]['layout']:
                self.place_sram_table(region, jstart, j,
                                          tables[j]['width'], tables[j]['depth'],
                                          available_width - req_width, req_depth)

                if not tables[j]['layout']:
                    j = self.get_next_unplaced_table(tables, j)

        if req_depth < available_depth:
            # If there are rows left, pick the next widest table not yet placed and try to fit it there.
            jstart = {'top': top + req_depth, 'left': left}
            j = self.get_next_unplaced_table(tables, i)
            while j < len(tables) and not tables[j]['layout']:
                self.place_sram_table(region, jstart, j,
                                          tables[j]['width'], tables[j]['depth'],
                                          available_width, available_depth - req_depth)

                if not tables[j]['layout']:
                    j = self.get_next_unplaced_table(tables, j)

        # When here, we have filled as much as we can of the free space to the right of the table. Now place this table

        tables[i]['layout'] = self.carve_memory(memory, top, left, top + req_depth - 1, left + req_width - 1, i)
        start['top'] += req_depth
        return

    def map_tcam_tables_bad(self, region):

        mem_type = 'tcam'
        blocks_left = allocation_units_to_num_blocks(mem_type, self.memory[mem_type][region]['width']) * \
                                 depth_to_num_blocks(mem_type, self.memory[mem_type][region]['depth'])

        i = 0
        start = {'top': 0, 'left' : 0}
        tables = self.tables[mem_type][region]
        while i < len(tables) and blocks_left > 0:
            width_in_blocks = allocation_units_to_num_blocks(mem_type, tables[i]['width'])
            width_in_units  = blocks_to_allocation_units(mem_type, width_in_blocks)
            depth_in_blocks = blocks_left / width_in_blocks
            depth_in_rows   = blocks_to_depth(mem_type, depth_in_blocks)

            self.memory[mem_type][region]['depth'] = depth_in_rows
            self.memory[mem_type][region]['width'] = width_in_units
            self.memory[mem_type][region]['space'] = [[0 for x in range(width_in_units)] \
                                                       for y in range(depth_in_rows)]

            if blocks_left < width_in_blocks * depth_in_blocks:
                self.logger.critical("Could not allocate memory for %s %s table \'%s\'." % (region, mem_type, tables[i]['name']))
            else:
                self.place_tcam_table(region, start, i, tables[i]['width'], tables[i]['depth'], width_in_units, depth_in_rows)

            blocks_left -= width_in_blocks * depth_in_blocks
            i = self.get_next_unplaced_table(tables, i)

    def map_tcam_tables(self, region):
        self.memory['tcam'][region]['space'] = [[0 for x in range(self.memory['tcam'][region]['width'])] for y in range(self.memory['tcam'][region]['depth'])]
        return

    def map_sram_tables(self, region):
        mem_type = 'sram'
        tables = self.tables[mem_type][region]
        memory = self.memory[mem_type][region]
        memory['space'] = [[0 for x in range(memory['width'])] for y in range(memory['depth'])]

        for table in tables:
            ctable = self.tmgr.gress_tm[xgress_from_string(region)].tables[table['name']]
            if ctable.is_policer or ctable.is_writeback:

                depth = table['depth']
                width = table['width']
                index = self.tables[mem_type][region].index(table)

                while not table['layout'] and depth > 0:
                    table['layout'] = self.scavenge_unused_space(mem_type, region, depth, width, table['width'], index)

                    width = width * 2
                    depth = depth / 2

        return

    def map_hbm_tables(self, region):
        mem_type = 'hbm'
        tables = self.tables[mem_type][region]

        start_pos = 0 # we are doing a zero based addressing. This will be relocated to the base of the region by HAL
        end_pos = start_pos + self.memory[mem_type][region]['depth']
        current_pos = start_pos

        i = self.get_next_unplaced_table(tables, -1)
        while i < len(tables):
            table = tables[i]
            table_size = table['width'] * table['depth']
            if end_pos < current_pos + table_size:
                self.logger.warning("map_hbm_tables(): No space in %s region for table %s" % (region, table['name']))
            else:
                table['layout'] = OrderedDict()
                table['layout']['top_left'] = {'block' : 0, 'x' : current_pos, 'y' : 0}
                table['layout']['bottom_right'] = {'block' : 0, 'x' : current_pos + table_size - 1, 'y' : 0}
                current_pos += table_size

            i = self.get_next_unplaced_table(tables, i)

        return

    def map_tables(self):

        for mem_type in self.tables:
            for region in self.tables[mem_type]:
                if len(self.tables[mem_type][region]) == 0:
                    continue
                if mem_type == 'tcam':
                    self.map_tcam_tables(region)
                elif mem_type == 'sram':
                    self.map_sram_tables(region)
                elif mem_type == 'hbm':
                    self.map_hbm_tables(region)
                    continue
                else:
                    self.logger.warning("map_tables(): Unknown Table type %s! Not Mapped.")
                    continue

                for table in self.tables[mem_type][region]:
                    if not table['layout']:
                        width = table['width']
                        depth = table['depth']
                        if width <= 0 or depth <= 0:
                            self.logger.warning("Width or Depth is 0 for %s %s table \'%s\'. Table not placed." % \
                                                (mem_type, region, table['name']))
                        else:
                            if mem_type != 'hbm':
                                index = self.tables[mem_type][region].index(table)
                                while not table['layout'] and depth > 0:
                                    table['layout'] = self.scavenge_unused_space(mem_type, region, depth, width, table['width'], index)

                                    if mem_type == 'tcam':
                                        break # Can't fold TCAM tables, so break out

                                    width = width * 2
                                    depth = depth / 2

                            if not table['layout']:
                                self.logger.critical("Could not allocate memory for %s %s table \'%s\'." % \
                                                     (region, mem_type, table['name']))
                                capri_dump_table_memory(self.be, self.memory, self.tables, mem_type, region)
                                ncc_assert(0)

                capri_dump_table_memory(self.be, self.memory, self.tables, mem_type, region)

    def get_mappings(self, specs):
        # To be implemented. This will map the P4 Tables into table memory
        table_mappings = []

        for table_spec in specs['tables']:

            table_mapping = OrderedDict()
            table_mapping['name'] = table_spec['name']
            table_mapping['match_type'] = table_spec['match_type']
            table_mapping['hash_type'] = table_spec['hash_type']
            table_mapping['direction'] = table_spec['direction']
            table_mapping['region'] = table_spec['region']
            table_mapping['stage'] = table_spec['stage']
            table_mapping['stage_table_id'] = table_spec['stage_table_id']
            table_mapping['num_entries'] = table_spec['num_entries']
            table_mapping['overflow'] = table_spec['overflow']
            table_mapping['overflow_parent'] = table_spec['overflow_parent']
            table_mapping['num_threads'] = table_spec['num_threads']
            if table_spec['num_threads'] > 1:
                table_mapping['thread_tbl_ids'] = table_spec['thread_tbl_ids']

            if 'tcam' in table_spec.keys():
                for table in self.tables['tcam'][table_spec['region']]:
                    if table_spec['name'] == table['name']:
                        memory = OrderedDict()
                        memory['entry_width'] = table['width']
                        memory['entry_width_bits'] = table_spec['tcam']['width']
                        memory['layout'] = self.carve_dummy() if not table['layout'] else table['layout']
                        memory['entry_start_index'] = capri_get_tcam_start_address_from_layout(memory['layout'])
                        memory['entry_end_index'] = capri_get_tcam_end_address_from_layout(memory['layout'])
                        memory['num_buckets'] = capri_get_num_bkts_from_layout(memory['layout'], memory['entry_width'])
                        table_mapping['tcam'] = memory
                        break

            if 'sram' in table_spec.keys():
                ctable = self.tmgr.gress_tm[xgress_from_string(table_spec['region'])].tables[table_spec['name']]
                table_mapping['token_refresh_rate'] = ctable.token_refresh_rate
                if table_spec['overflow_parent']:
                    # Overflow SRAMs are appended to their parent SRAMs. Create layout using parent's layout
                    for table in self.tables['sram'][table_spec['region']]:
                        if table_spec['overflow_parent'] == table['name']:
                            entry_bit_width = 0
                            for temp_spec in specs['tables']:
                                if temp_spec['name'] == table['name']:
                                    if 'sram' in temp_spec.keys():
                                        entry_bit_width = temp_spec['sram']['width']
                                    break
                            memory = OrderedDict()
                            memory['entry_width'] = table['width']
                            if ctable.is_writeback:
                                memory['entry_width_bits'] = 16 * table['width']
                            else:
                                memory['entry_width_bits'] = entry_bit_width
                            memory['layout'] = self.carve_dummy() if not table['layout'] else table['layout']
                            memory['entry_start_index'] = capri_get_sram_sw_start_address_from_layout(memory['layout'])
                            memory['entry_end_index'] = capri_get_sram_sw_end_address_from_layout(memory['layout'])
                            memory['num_buckets'] = capri_get_num_bkts_from_layout(memory['layout'], memory['entry_width'])
                            table_mapping['sram'] = memory
                            break
                else:
                    for table in self.tables['sram'][table_spec['region']]:
                        if table_spec['name'] == table['name']:
                            memory = OrderedDict()
                            memory['entry_width'] = table['width']
                            if ctable.is_writeback:
                                memory['entry_width_bits'] = 16 * table['width']
                            else:
                                memory['entry_width_bits'] = table_spec['sram']['width']
                            memory['layout'] = self.carve_dummy() if not table['layout'] else table['layout']
                            memory['entry_start_index'] = capri_get_sram_sw_start_address_from_layout(memory['layout'])
                            memory['entry_end_index'] = capri_get_sram_sw_end_address_from_layout(memory['layout'])
                            memory['num_buckets'] = capri_get_num_bkts_from_layout(memory['layout'], memory['entry_width'])
                            table_mapping['sram'] = memory
                            break

            if 'hbm' in table_spec.keys():
                for table in self.tables['hbm'][table_spec['region']]:
                    if table_spec['name'] == table['name']:
                        memory = OrderedDict()
                        memory['entry_width'] = table['width']
                        memory['entry_start_index'] = capri_get_hbm_start_address_from_layout(table['layout'])
                        memory['entry_end_index'] = capri_get_hbm_end_address_from_layout(table['layout'])
                        memory['num_buckets'] = 1
                        table_mapping['hbm'] = memory
                        break

            table_mappings.append(table_mapping)

        return table_mappings

    def run(self, table_specs):

        gc.collect(2)
        self.logger.info("Mapping P4 tables to memory ...")
        self.load_tables(table_specs)
        self.map_tables()
        self.validate_placement()
        self.logger.info("Done Mapping P4 tables to memory.")

class capri_table_manager:
    def __init__(self, capri_be):
        self.be = capri_be
        self.logger = logging.getLogger('TM')
        self.gress_tm = [capri_gress_tm(self, d) for d in xgress]
        self.table_memory_spec = capri_table_memory_spec_load(self.be)
        self.mapper = capri_table_mapper(self)

    def print_tables(self):
        for gtm in self.gress_tm:
            gtm.print_table()

    def validate_table_config(self, d, table_name=None):
        for gtm in self.gress_tm:
            gtm.validate_table_config(table_name)

    def print_table_info(self, d, table_name=None):
        for gtm in self.gress_tm:
            gtm.print_table_info(table_name)

    def initialize_tables(self):
        # For now reuse code from HLIR
        # Later create our own topo-sorting for this
        table_graph = build_table_graph_ingress(self.be.h)
        stage_tables = get_logical_stages(table_graph)
        self.gress_tm[xgress.INGRESS].init_tables(stage_tables)

        table_graph = build_table_graph_egress(self.be.h)
        stage_tables = get_logical_stages(table_graph)
        self.gress_tm[xgress.EGRESS].init_tables(stage_tables)

        for d in xgress:
            for stg, table_list in enumerate(self.gress_tm[d].stage_tables):
                self.logger.debug("%s:%d: %s" % \
                    (d.name, stg, [t.name for t in table_list]))

    def update_table_config(self):
        for gtm in self.gress_tm:
            gtm.update_table_config()

    def program_tables(self):
        for gtm in self.gress_tm:
            gtm.program_tables()

    def create_key_makers(self):
        for gtm in self.gress_tm:
            gtm.create_key_makers()

    def capri_asm_output(self):
        for d in xgress:
            for t in self.gress_tm[d].tables.values():
                capri_asm_output_table(self.be, t)

    def create_p4_table_specs(self):
        # generate P4 table specification output

        table_specs = []

        for d in xgress:
            for table in self.gress_tm[d].tables.values():
                table_spec = OrderedDict()
                table_spec['name'] = table.p4_table.name
                table_spec['match_type'] = match_type_to_string(table.match_type)
                table_spec['hash_type'] = 0 if table.hash_type == None else table.hash_type
                table_spec['direction'] = xgress_to_string(d)
                table_spec['region'] = '02-p4_tables' if table.is_hbm else xgress_to_string(d)
                table_spec['stage'] = table.stage
                table_spec['stage_table_id'] = table.tbl_id
                table_spec['num_entries'] = 0 if table.num_entries == None else table.num_entries
                table_spec['overflow'] = ''
                table_spec['overflow_parent'] = ''
                table_spec['num_threads'] = table.num_threads
                if table.num_threads > 1:
                    table_spec['thread_tbl_ids'] = {}
                    for k, v in table.thread_tbl_ids.items():
                        table_spec['thread_tbl_ids'][str(k)] = v

                if table.match_type == match_type.EXACT_IDX :
                    table_spec['hbm' if table.is_hbm else 'sram'] = {"width" : table.d_size}
                elif table.match_type == match_type.TERNARY_ONLY :
                    table_spec['tcam'] = {"width" : table.final_key_size}
                elif table.match_type == match_type.TERNARY :
                    table_spec['tcam'] = {"width" : table.final_key_size}
                    table_spec['sram'] = {"width" : table.d_size}
                elif table.match_type == match_type.EXACT_HASH or table.match_type == match_type.EXACT_HASH_OTCAM :
                    table_spec['hbm' if table.is_hbm else 'sram'] = {"width" : table.final_key_size + table.d_size}
                    # This hash table has an overflow table. Find it
                    for otable in self.gress_tm[d].tables.values():
                        if not otable.is_overflow :
                            continue
                        if table.p4_table.name != otable.htable_name :
                            continue
                        table_spec['overflow'] = otable.p4_table.name
                        break

                # If this is an overflow table, record the parent hash table
                if table.is_overflow :
                    table_spec['overflow_parent'] = table.htable_name

                table_specs.append(table_spec)

        return table_specs

    def compute_token_refresh_timer_value(self):
        for d in xgress:
            max_refresh_rate = 1 # Initialize to one packet per second
            tot_entries = 0 # Total of all policer entries
            for ctable in self.gress_tm[d].tables.values():
                if ctable.is_rate_limit_en or ctable.is_policer:
                    # Initialize token refresh timers for the table
                    ctable.token_refresh_timers = {'value':0,'scale':0}
                    # Compute and store the timer values for this table
                    clocks_per_cycle = capri_model['match_action']['te_consts']['base_clock_freq'] / ctable.token_refresh_rate
                    clocks_per_cycle = clocks_per_cycle >> 1
                    while clocks_per_cycle & 0xFFFFFFFFFFFF0000L:
                        ctable.token_refresh_timers['scale'] += 1
                        clocks_per_cycle >>= 1
                    ctable.token_refresh_timers['value'] = clocks_per_cycle
                    # Accumulate the total number of entries and max refresh rate
                    tot_entries = tot_entries + ctable.num_entries
                    if  max_refresh_rate < ctable.token_refresh_rate:
                        max_refresh_rate = ctable.token_refresh_rate

            clocks_reqd = tot_entries * 16
            clocks_available = capri_model['match_action']['te_consts']['base_clock_freq'] / max_refresh_rate
            ncc_assert(clocks_reqd < clocks_available, "Too many policer/rate-limiter entries to support %d refresh/sec" % (max_refresh_rate))

        return

    def generate_cap_pic_output(self):

        self.logger.info("Generating cap pics ...")

        self.compute_token_refresh_timer_value()
        pic = capri_pic_csr_load(self) # Load the templates

        for mem_type in self.mapper.tables:
            for region in self.mapper.tables[mem_type]:
                for table in self.mapper.tables[mem_type][region]:
                    if not table['layout']:
                        continue
                    layout = table['layout']
                    direction = xgress.INGRESS if table['name'] in self.gress_tm[xgress.INGRESS].tables else xgress.EGRESS
                    ctable = self.gress_tm[direction].tables[table['name']]
                    profile_id = (ctable.stage << 4) | ctable.tbl_id
                    if mem_type == 'sram':
                        cap_name = 'cap_pics'
                        profile_name = "%s_csr_cfg_table_profile[%d]" % (cap_name, profile_id)
                        profile = pic[mem_type][xgress_to_string(direction)][cap_name]['registers'][profile_name]
                        profile['width']['value'] = "0x%x" % table['width']
                        profile['hash']['value'] = "0x%x" % 0 # Not used, confirmed by hw
                        profile['opcode']['value'] = "0x%x" % 0 # Policer/Sampler/Counter
                        profile['log2bkts']['value'] = "0x%x" % capri_get_log2bkts_from_layout(layout, table['width'])
                        profile['start_addr']['value'] = "0x%x" % capri_get_sram_hw_start_address_from_layout(layout)
                        profile['end_addr']['value'] = "0x%x" % capri_get_sram_hw_end_address_from_layout(layout)
                        profile['rlimit_en']['value'] = "0x%x" % (ctable.is_rate_limit_en)
                        if (ctable.match_type != match_type.EXACT_IDX and \
                            ctable.match_type != match_type.TERNARY and \
                            ctable.match_type != match_type.TERNARY_ONLY) and \
                            ctable.d_size < ctable.start_key_off:
                            # For hash tables, match key packing has to align with KM. Hence when packing
                            # action data, leading space (before match key) is always filled by p4pd.
                            # If there is more than 16bits of leading space unfilled, then use axi-shift.
                            # P4PD has to pack actiondata and match key such that unfilled bit space is before
                            # action-data,actionpc.
                            wordmultiple_dsize = ctable.d_size - ctable.d_size % 16
                            profile['axishift']['value'] = "0x%x" % ((ctable.start_key_off - wordmultiple_dsize) >> 4)
                        else:
                            profile['axishift']['value'] = "0x%x" % (0)

                        if ctable.is_writeback and ctable.match_type == match_type.TERNARY:
                            opcode  = ((capri_model['match_action']['te_consts']['pic_tbl_opcode_operation_add']))
                            opcode |= ((capri_model['match_action']['te_consts']['pic_tbl_opcode_oprd1_sel_tbkt']) << 4)
                            opcode |= ((capri_model['match_action']['te_consts']['pic_tbl_opcode_oprd2_sel_one'])  << 6)
                            opcode |= ((capri_model['match_action']['te_consts']['pic_tbl_opcode_saturate_none'])  << 8)
                            profile['opcode']['value'] = "0x%x" % (opcode)

                        if ctable.is_policer:
                            opcode  = ((capri_model['match_action']['te_consts']['pic_tbl_opcode_operation_sub']))
                            opcode |= ((capri_model['match_action']['te_consts']['pic_tbl_opcode_oprd1_sel_tbkt'])    << 4)
                            opcode |= ((capri_model['match_action']['te_consts']['pic_tbl_opcode_oprd2_sel_policer']) << 6)
                            opcode |= ((capri_model['match_action']['te_consts']['pic_tbl_opcode_saturate_neg'])      << 8)
                            opcode |= ((1)  << 10) # Policer or Rate-Limit
                            profile['opcode']['value'] = "0x%x" % (opcode)

                        if ctable.is_rate_limit_en:
                            opcode  = ((capri_model['match_action']['te_consts']['pic_tbl_opcode_operation_sub']))
                            opcode |= ((capri_model['match_action']['te_consts']['pic_tbl_opcode_oprd1_sel_tbkt'])    << 4)
                            opcode |= ((capri_model['match_action']['te_consts']['pic_tbl_opcode_oprd2_sel_pktsize']) << 6)
                            opcode |= ((capri_model['match_action']['te_consts']['pic_tbl_opcode_saturate_none'])     << 8)
                            opcode |= ((1)  << 10) # Policer or Rate-Limit
                            profile['opcode']['value'] = "0x%x" % (opcode)

                        if ctable.is_policer or ctable.is_rate_limit_en:
                            bg_upd_profile_name = "%s_csr_cfg_bg_update_profile[%d]" % (cap_name, ctable.token_refresh_profile)
                            bg_upd_profile = pic[mem_type][xgress_to_string(direction)][cap_name]['registers'][bg_upd_profile_name]
                            bg_upd_profile['start_addr']['value'] = "0x%x" % capri_get_sram_hw_start_address_from_layout(layout)
                            bg_upd_profile['end_addr']['value'] = "0x%x" % capri_get_sram_hw_end_address_from_layout(layout)
                            bg_upd_profile['rlimit_en']['value'] = "0x%x" % (ctable.is_rate_limit_en)
                            opcode  = ((capri_model['match_action']['te_consts']['pic_tbl_opcode_operation_add']))
                            opcode |= ((capri_model['match_action']['te_consts']['pic_tbl_opcode_oprd1_sel_tbkt']) << 4)
                            opcode |= ((capri_model['match_action']['te_consts']['pic_tbl_opcode_oprd2_sel_rate']) << 6)
                            opcode |= ((capri_model['match_action']['te_consts']['pic_tbl_opcode_saturate_oprd3']) << 8)
                            opcode |= ((1)  << 10) # Policer or Rate-Limit
                            bg_upd_profile['opcode']['value'] = "0x%x" % (opcode)
                            bg_upd_profile['timer']['value'] = "0x%x" % (ctable.token_refresh_timers['value'])
                            bg_upd_profile['scale']['value'] = "0x%x" % (ctable.token_refresh_timers['scale'])
                            bg_upd_profile['_modified'] = True

                            bg_upd_profile_en_name = 'cap_pics_csr_cfg_bg_update_profile_enable'
                            bg_upd_profile_en = pic[mem_type][xgress_to_string(direction)][cap_name]['registers'][bg_upd_profile_en_name]
                            vector = (int(bg_upd_profile_en['vector']['value'], 0) | (1 << ctable.token_refresh_profile))
                            bg_upd_profile_en['vector']['value'] = "0x%x" % (vector)
                            bg_upd_profile_en['_modified'] = True

                    elif mem_type == 'tcam':
                        cap_name = 'cap_pict'
                        num_bkts = 0 if table['depth'] == 0 else (capri_get_depth_from_layout(layout) / table['depth'])
                        profile_name = "%s_csr_cfg_tcam_table_profile[%d]" % (cap_name, profile_id)
                        profile = pic[mem_type][xgress_to_string(direction)][cap_name]['registers'][profile_name]
                        profile['width']['value'] = "0x%x" % table['width']
                        profile['bkts']['value'] = "0x%x" % (num_bkts)
                        profile['start_addr']['value'] = "0x%x" % capri_get_tcam_start_address_from_layout(layout)
                        profile['end_addr']['value'] = "0x%x" % capri_get_tcam_end_address_from_layout(layout)
                        profile['keyshift']['value'] = "0x%x" % (ctable.start_key_off / 16)
                    elif mem_type == 'hbm':
                        profile_name = "cap_te_csr_cfg_table_property[%d]" % (ctable.tbl_id)
                        profile = pic[mem_type][xgress_to_string(direction)][ctable.stage][profile_name]
                        profile['addr_base']['value'] = "0x%x" % (memory_base_addr.HBM +
                                                                  capri_get_hbm_start_address_from_layout(table['layout']))
                    else:
                        continue

                    profile['_modified'] = True

        capri_pic_csr_output(self.be, pic)

        self.logger.info("Done generating cap pics.")

    def generate_p4_table_spec_output(self):

        specs = OrderedDict()
        specs['global'] = self.table_memory_spec
        specs['tables'] = self.create_p4_table_specs()
        capri_p4_table_spec_output(self.be, specs)

        self.mapper.run(specs['tables'])

        mapping = OrderedDict()
        mapping['global'] = self.table_memory_spec
        mapping['tables'] = self.mapper.get_mappings(specs)
        capri_p4_table_map_output(self.be, mapping)

        self.generate_cap_pic_output()

    def generate_output(self):
        for gtm in self.gress_tm:
            gtm.generate_output()
        self.generate_p4_table_spec_output()

    def tm_dbg_output(self):
        # create a dictionary to write to a json file
        te_debug_info = OrderedDict()
        for d in xgress:
            te_debug_info[d.name] = self.gress_tm[d].te_dbg_output()
        return te_debug_info

    def tm_k_plus_d_fields_add(self, k_plus_d_dict):
        for d in xgress:
            if d == xgress.INGRESS:
                self.gress_tm[d].tm_k_plus_d_fields_add(k_plus_d_dict['INGRESS_KD'])
            else:
                self.gress_tm[d].tm_k_plus_d_fields_add(k_plus_d_dict['EGRESS_KD'])
