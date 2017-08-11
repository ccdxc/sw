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
    assert(not has_cycle)
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
                assert(d_type_ > 0)
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
    stages_needed = len(fixed_stage_tables.keys())

    while stages_used < stages_needed:
        fixed_stage_table_list.append([])
        stages_used += 1

    for k in range(stages_needed):
        fixed_stage_table_list[k] += fixed_stage_tables[k]

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
            assert cw > 0
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
            assert w > 0, pdb.set_trace()
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

        for k_bit, w, _ in self.i_bit_ext:
            while w:
                self.km_profile.bit_sel.append(k_bit)
                self.km_profile.i_bit_sel.append(k_bit)
                k_bit += 1
                w -= 1

        # sorted_bytes = sorted(self.km_profile.byte_sel)
        # assert sorted_bytes == self.km_profile.byte_sel, pdb.set_trace()

# Table Management
class capri_table:
    def __init__(self, gtm, p4_table, d):
        self.p4_table = p4_table
        self.gtm = gtm
        self.d = d
        self.stage = -1
        self.tbl_id = -1      # within the state (0-15)
        self.match_type = match_type.NONE    # hash, indexed, tcam
        self.num_entries = -1
        self.is_otcam = False
        self.is_overflow = False
        self.htable_name = None
        self.hash_ct = None
        self.hash_type = 0  # pragma hash_type
        self.is_raw = False # Raw table (used in p4_plus)
        self.is_raw_index = False # Raw index table (used in p4_plus) - slightly different from raw
        self.is_hbm = False # If table resides in HBM, it will be set to true.
                            # A pragma is used in P4 to qualify the table.
        self.is_writeback = False # True when MPU writes to table entry - need lock
        self.is_policer = False
        self.policer_colors = 0 # 2-color/3-color
        # flit numbers from which key(K+I) is built, Lookup can be
        # launched from the last flit when all the info is avaialble
        self.flits_used = []     # flits that provide K and I values
        num_flits = gtm.tm.be.hw_model['phv']['num_flits']
        self.km_flits = [capri_km_flit(self) for f in range(num_flits)]

        self.launch_flit = -1
        self.key_makers = []

        self.keys = []  # (hdr/capri_field, match_type, mask) => k{}
        self.input_fields = [] # combined list of input fields used by all actions => i{}
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

    def is_hash_table(self):
        return True if (self.match_type == match_type.EXACT_HASH_OTCAM or \
                        self.match_type == match_type.EXACT_HASH) else False
    def is_index_table(self):
        return True if self.match_type == match_type.EXACT_IDX else False

    def is_tcam_table(self):
        return True if self.match_type == match_type.TERNARY else False
    
    def is_mpu_only(self):
        return True if self.match_type == match_type.MPU_ONLY else False

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
            assert ksize, 'Invalid key size computed for %s' % self.p4_table.name
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
        max_ki_size = self.gtm.tm.be.hw_model['match_action']['max_ki_size']
        max_kd_size = self.gtm.tm.be.hw_model['match_action']['max_kd_size']
        violation = False
        if ((self.final_key_size + self.i_phv_size) > max_ki_size):
            self.gtm.tm.logger.critical('%s:Violation Table %s (K+I) %d\n' % \
                (self.d.name, self.p4_table.name, self.final_key_size + self.i_phv_size))
            violation = True
        kd_size = self.d_size
        if self.is_hash_table():
            kd_size += self.final_key_size
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
        num_km = (self.final_key_size + self.i_phv_size + key_maker_width - 1) / key_maker_width
        num_bit_extractors = num_bit_extractors * num_km
        if total_bit_extractions > num_bit_extractors:
            self.gtm.tm.logger.critical("%s:Violation-%s need %d bit extractors(max %d allowed)" % \
                (self.d.name, self.p4_table.name, total_bit_extractions, num_bit_extractors))
            self.gtm.tm.logger.debug("%s:Violation - \n i_bits(%d) %s\n k_bits(%d) %s:\nK:%s \nI:%s\n" % \
                (self.p4_table.name, i_bits, self.i_bit_ext, k_bits, self.k_bit_ext, self.keys,
                 self.input_fields))
            violation = True

        assert violation == False, "Fix violations and try again"
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
                assert new_key_size == old_key_size, pdb.set_trace()

        km_used = [-1 for _ in self.key_makers]
        for _km in self.key_makers:
            hw_id = _km.get_hw_id()
            assert hw_id not in km_used, pdb.set_trace()
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
            self.gtm.tm.logger.info('%s k_size changed %d -> %d' % \
                (self.p4_table.name, self.key_size, key_size))

        i_size = sum([k[1] for k in self.i_bit_ext])
        i_size += sum([k[1] for k in self.i1_phv_chunks])
        i_size += sum([k[1] for k in self.i2_phv_chunks])

        # i_size can change due to union-field phvs shared between k and i
        self.i_phv_size = i_size
        if i_size != self.i_size:
            self.gtm.tm.logger.info('%s i_size changed %d -> %d' % \
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
        assert num_km <= 2, "K+I exceed key maker width for table %s" % (self.p4_table.name)
        max_ki = num_km * key_maker_width
        max_bits = num_km * bit_extractors

        n_k_bits = sum(kw for kb,kw in self.k_bit_ext)
        n_i_bits = sum(kw for kb,kw,_ in self.i_bit_ext)

        if (n_k_bits + n_i_bits) <= max_bits:
            return False

        extra_bits = n_k_bits + n_i_bits - max_bits
        add_bits = 0
        rm_bits = 0
        rm_chunks = []
        i2k_bits = []
        # combine multiple consecutive i_bits - XXX already done ???
        sorted_i_bits = sorted(self.i_bit_ext, key=lambda k:k[1], reverse=True)

        for b,w,in_key in sorted_i_bits:
            if in_key:
                # this indicates that i bits are in the middle of the key in the same flit
                # these can be converted to key if this is a TCAM lookup
                # XXX need better indication when w<8
                i2k_bits.append((b,w))
                continue
            # update the chunk info and record the pad chunk
            assert(w<8), pdb.set_trace()
            add_bits += (8-w)
            rm_bits += w
            rm_chunks.append((b,w,in_key))
            if rm_bits >= extra_bits:
                break
        if (ki_size + add_bits) > max_ki:
            # still a problem - needs to change the program
            assert 0, pdb.set_trace()

        # move i_bits chunk to byte extraction
        for rc in rm_chunks:
            self.gtm.tm.logger.debug('%s:convert %s i_bits to i_bytes' % \
                (self.p4_table.name, rc))
            if rc[0] % 8:
                b = (rc[0]/8) * 8
                w += rc[0] - b
            if w % 8:
                w += (8-(w%8))

            #pdb.set_trace()
            self.i_bit_ext.remove(rc)
            fid = rc[0]/flit_size
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

        extra_bits -= rm_bits   # the remaining extra bits are mingled with key
        ki_size += add_bits
        if extra_bits > 0 and len(i2k_bits) and self.match_type == match_type.TERNARY:
            for b,w in i2k_bits:
                if extra_bits <= 0:
                    break
                fid = b/flit_size
                # record the bits added to the front and back of this chunk
                # remove this chunk from i_bit_ext to k_phv_chunks
                #pdb.set_trace()
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
                    pdb.set_trace()
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
                extra_bits -= w

            assert extra_bits <= 0, pdb.set_trace()
        return True

    def ct_update_table_config(self):
        # sort the keys based on phv position
        sorted_keys = sorted(self.keys, key=lambda k: k[0].phv_bit)
        self.keys = sorted_keys
        # fix k-i sizes based on unions
        key_cfs = [cf for cf,_,_ in self.keys]
        if self.key_size < 0:
            self.key_size = self.ct_compute_phv_size(key_cfs)

        # sort the input flds based on phv position
        sorted_i = sorted(self.input_fields, key=lambda k: k.phv_bit)
        self.input_fields = sorted_i
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
        if not self.gtm.tm.be.args.no_te_bit2byte and not self.is_otcam:
            new_k_phv_chunks = copy.copy(k_phv_chunks)
            new_i_phv_chunks = copy.copy(i_phv_chunks)
            # if index table has >1 k_phv_chunks, it means different bit fields are concatenated
            # into an index. Don't convert them to bytes
            if len(new_k_phv_chunks) and \
                (not self.is_index_table() or \
                    (self.is_index_table() and len(new_k_phv_chunks) == 1)):
                cs,cw = new_k_phv_chunks[0]
                if cs % 8:
                    # check if end is byte-aligned. This is to weed out small bit fields
                    # (like hv bit) that appear at the front
                    start_pad = cs % 8
                    if self.is_index_table() or (start_pad < 4 and ((cs+cw)%8) == 0):
                        # more bits than pad
                        cs -= start_pad
                        cw += start_pad
                        new_k_phv_chunks.pop(0)
                        new_k_phv_chunks.insert(0,(cs,cw))
                        k_start_delta = start_pad

                # This can done for TCAM/Index tables. For other tables if there are other k-bits
                # then last partial K-byte cannot be extended into full byte
                # handle only for IDX table for now
                cs,cw = new_k_phv_chunks[-1]
                ce = cs+cw
                if ce % 8 and self.is_index_table():
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
            c_offset = -1
            # sort based on cstart
            tmp_i_phv_chunks = sorted(new_i_phv_chunks, key=lambda k:k[0])
            new_i_phv_chunks = []
            for c, (cs,cw) in enumerate(tmp_i_phv_chunks):
                if cs > c_offset:
                    c_offset = cs+cw
                    new_i_phv_chunks.append((cs,cw))
                    continue

                # overlap or duplicate
                if (cs+cw) <= c_offset:
                    # full overlap
                    continue

                # add to last chunk (merge contiguous/overlapping chunks)
                (l_cs, l_cw) = new_i_phv_chunks.pop()
                add_w = (cs+cw) - c_offset
                new_i_phv_chunks.append((l_cs, l_cw+add_w))
                c_offset = cs+cw

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

            if (new_num_km <= num_km) and (ki_delta >= 0) and \
                (ki_delta <= ((k_size+i_size) / allow_increase)):

                k_phv_chunks = new_k_phv_chunks
                i_phv_chunks = new_i_phv_chunks

                self.gtm.tm.logger.debug("%s:%s:%s:bit2byte changes to K,I" % \
                    (self.gtm.d.name, self.p4_table.name, self.match_type.name))
                self.gtm.tm.logger.debug("%s:%s:(k,I) new:(%d, %d) old:(%d,%d)" % \
                    (self.gtm.d.name, self.p4_table.name, new_k_size, new_i_size, k_size, i_size))
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
                #pdb.set_trace()

        flit_k_phv_chunks = {f:[] for f in range(num_flits)}
        flit_i_phv_chunks = {f:[] for f in range(num_flits)}

        for cs, cw in k_phv_chunks:
            start_fid = cs/flit_size
            end_fid = (cs+cw-1)/flit_size
            if start_fid == end_fid:
                flit_k_phv_chunks[start_fid].append((cs,cw))
                continue
            # crossing the flit
            w_flit = ((start_fid+1) * flit_size) - cs
            w = min(flit_size, w_flit)
            assert w > 0, pdb.set_trace()
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
            end_fid = (cs+cw-1)/flit_size
            if start_fid == end_fid:
                flit_i_phv_chunks[start_fid].append((cs,cw))
                continue
            # crossing the flit
            w_flit = ((start_fid+1) * flit_size) - cs
            w = min(flit_size, w_flit)
            flit_i_phv_chunks[start_fid].append((cs,w))
            assert w > 0, pdb.set_trace()
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

                assert (cs % 8) == 0
                part_bits = cw % 8
                if part_bits:
                    kf.k_bit_ext.append((cs+cw-part_bits, part_bits))
                    cw -= part_bits
                if cw:
                    new_phv_chunks.append((cs, cw))

            k_phv_chunks = new_phv_chunks
            if len(k_phv_chunks):
                k_start = k_phv_chunks[0][0]
            else:
                k_start = -1
            new_i_phv_chunks = []
            # move non-byte aligned flds from the i vector to a separate list
            for c, (cs, cw) in enumerate(i_phv_chunks):
                part_start = cs % 8
                part_end = (cs+cw) % 8
                if part_start:
                    part_len = cw if cw < (8-part_start) else (8-part_start)
                    kf.i_bit_ext.append((cs, part_len, cs < k_start))
                    cw -= part_len
                    cs += part_len
                    if not cw:
                        continue
                if part_end:
                    kf.i_bit_ext.append((cs+cw-part_end, part_end, (cs+cw-part_end) < k_start))
                    cw -= part_end
                    if not cw:
                        continue
                if cw:
                    new_i_phv_chunks.append((cs, cw))

            if len(new_i_phv_chunks) and len(k_phv_chunks):
                i_phv_chunks = _remove_duplicate_chunks(new_i_phv_chunks, k_phv_chunks)
            else:
                i_phv_chunks = new_i_phv_chunks

            i_in_key = 0
            for cs, cw in i_phv_chunks:
                part_start = cs % 8
                part_end = (cs+cw) % 8
                is_i1 = False   # default is i2
                is_i_bits = False
                if len(k_phv_chunks):
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
                #self.i_in_key = i_in_key # for printing (for analysis)
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
        #pdb.set_trace()
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
        self.num_km = (ki_size + key_maker_width - 1) / key_maker_width
        self.gtm.tm.logger.debug("%s:%s:Table-Logical-Profile: %s" % \
            (self.gtm.d.name, self.p4_table.name, self.combined_profile))
        #pdb.set_trace()

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

        assert self.num_km <= 2 # max km_per_key
        if self.num_km == 0:
            # always use KM0 for this, it does not matter if km is used for another table
            assert self.is_mpu_only()
            km = capri_key_maker(self, [self])
            km.km_id = 0
            self.key_makers.append(km)
            return

        if self.is_otcam:
            # do not allocate km for otcam.. it will use the same km as its hash_table
            return
        self.gtm.tm.logger.debug("%s:Create %d key_makers for %s" % \
            (self.gtm.d.name, self.num_km, self.p4_table.name))
        for i in range(self.num_km):
            km = capri_key_maker(self, [self])
            km.km_id = i
            self.key_makers.append(km)

        if self.num_km < 2:
            # only one key-maker, copy all info from table
            km = self.key_makers[0]
            km.combined_profile = self.combined_profile
            km.combined_profile._update_bit_loc_key_off()
            km.has_key = True
            for fid,kf in enumerate(self.km_flits):
                km.flit_km_profiles[fid] = copy.deepcopy(kf.km_profile)
            for fid in range(self.flits_used[0], self.flits_used[-1]+1):
                km.flits_used.append(fid)
            return

        #pdb.set_trace()
        # multiple kms
        # check how to split
        # prefer if key and i can be separated into two kms
        # Any i1 data can be moved to second key-maker
        # XXX Can we do this while creating the km_profiles ???
        # Allocate key-makers up-front??
        # There is a lot of benefit getting rid on i1 in terms of entry size savings
        max_km_width = self.gtm.tm.be.hw_model['match_action']['key_maker_width']
        max_kmB = max_km_width/8
        max_km_bits = self.gtm.tm.be.hw_model['match_action']['num_bit_extractors']
        total_kB = (self.key_phv_size + 7) / 8
        total_i1B = len(self.combined_profile.i1_byte_sel)
        total_i2B = len(self.combined_profile.i2_byte_sel)
        total_iB = (self.i_phv_size + 7) / 8
        #if self.is_hbm and self.is_hash_table():
        if self.is_hash_table():
            # HACK: for now just add required bytes to total_kB so that we have some
            # space in the key_maker for hbm hash table to align the key in key-maker
            # and HBM memory. H/w needs first 8 bits for action_pc when used
            if self.is_overflow:
                total_kB += 4    # keep some space for index from recirc
            elif self.num_actions() > 1:
                action_id_size = self.gtm.tm.be.hw_model['match_action']['action_id_size']
                total_kB += (action_id_size+7)/8    # keep space for action_pc
            else:
                pass
        if total_kB <= max_kmB and total_iB <= max_kmB:
            #pdb.set_trace()
            self.gtm.tm.logger.debug("%s:Split key_makers as K and I" % self.p4_table.name)
            # K and I can be split into separate key makers
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

            km0.has_key = True
            #pdb.set_trace()
        else:
            self.gtm.tm.logger.debug("%s:%s:Split key_makers (K+I) as it fits" % \
                (self.gtm.d.name, self.p4_table.name))

            assert len(self.oo_flits) == 0

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
                #pdb.set_trace()
                # not much can be done. just place in order(i1,k,...) as it fits
                # if k_bits > max_km_bits : split k bits in km0(end) and km1 after k bytes
                # if k_bits < max_km_bits : move them all to km1 after k bytes. In this case
                # any i-bits in km0 will have to be placed before k-bytes
                # if i_bits > max_km_bits : move some i-bits before key in km0
                if total_i1B:
                    assert total_i1B < max_kmB, pdb.set_trace()
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
                    km0_free -= ((num_kbits+km0_bits+7)/8)
                    for b in range(km0_bits, num_ibits):
                        km1_profile.i_bit_sel.append(self.combined_profile.i_bit_sel[b])
                    km1_free -= ((num_ibits-km0_bits+7)/8)

                # split k into km0 and km1
                #pdb.set_trace()
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
                km0.has_key = True
                km1.has_key = True
            else:
                # place key in km0
                # if entire i1 can move to km1, move it and move rest of the i2 evenly
                # if i1 cannot be completely moved to km1, then divide i1+i2 evenly
                self.gtm.tm.logger.debug("%s:%s:Split key_makers k < max_km" % \
                    (self.gtm.d.name, self.p4_table.name))
                keep_free = (km0_free + km1_free - total_kB - total_iB) / 2
                km0_profile.k_byte_sel += self.combined_profile.k_byte_sel
                assert num_kbits < max_km_bits, pdb.set_trace() # XXX TBD
                km0_profile.k_bit_sel += self.combined_profile.k_bit_sel
                km0_free -= total_kB
                num_byte_sel = len(self.combined_profile.k_byte_sel) + \
                                len(self.combined_profile.i1_byte_sel) + \
                                len(self.combined_profile.i2_byte_sel)

                byte_avail = (self.num_km * max_kmB) - num_byte_sel
                byte_avail -= ((num_kbits+7)/8)

                if num_ibits > max_km_bits or (byte_avail < (num_ibits+7)/8):
                    km_free_bits = (max_km_bits + max_km_bits - num_kbits - num_ibits)/2
                    km0_bits = max_km_bits - km_free_bits
                    km0_bits = min(km0_bits, num_ibits)
                    for b in range(km0_bits):
                        km0_profile.i_bit_sel.append(self.combined_profile.i_bit_sel[b])
                    km0_free -= ((km0_bits+7)/8)
                    for b in range(km0_bits, num_ibits):
                        km1_profile.i_bit_sel.append(self.combined_profile.i_bit_sel[b])
                    km1_free -= ((num_ibits-km0_bits+7)/8)
                else:
                    # move all i bits to km1
                    km1_profile.i_bit_sel += self.combined_profile.i_bit_sel
                    km1_free -= ((len(self.combined_profile.i_bit_sel)+7)/8)

                km0_iB = km0_free-keep_free
                km1_iB = km1_free-keep_free

                i = -1
                # copy i1 bytes to km1 (as much as allowed)
                # XXX check if we can avoid breaking up a field
                for i in range(min(km1_iB, total_i1B)):
                    km1_profile.i2_byte_sel.append(self.combined_profile.i1_byte_sel[i])
                    km1_free -= 1
                    km1_iB -= 1

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

                km0_profile.i1_byte_sel = sorted(km0_profile.i1_byte_sel)
                km0_profile.i2_byte_sel = sorted(km0_profile.i2_byte_sel)
                km1_profile.i2_byte_sel = sorted(km1_profile.i2_byte_sel)

                km0_profile.byte_sel = (km0_profile.i1_byte_sel + km0_profile.k_byte_sel + \
                    km0_profile.i2_byte_sel)
                km0_profile.bit_sel = km0_profile.k_bit_sel + km0_profile.i_bit_sel
                km1_profile.byte_sel = (km1_profile.i1_byte_sel + km1_profile.k_byte_sel + \
                    km1_profile.i2_byte_sel)
                km1_profile.bit_sel = km1_profile.k_bit_sel + km1_profile.i_bit_sel

                km0.has_key = True
                km0.combined_profile = km0_profile
                km1.combined_profile = km1_profile
                km0_profile._update_bit_loc_key_off()
                km1_profile._update_bit_loc_key_off()

            # common code for split profiles
            # copy new profiles into flits for future use
            #pdb.set_trace()
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
                        assert 0, pdb.set_trace()

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
        assert total_k_bytes == len(self.combined_profile.k_byte_sel), pdb.set_trace()
        assert total_i_bytes == len(self.combined_profile.i1_byte_sel) + \
                                len(self.combined_profile.i2_byte_sel), pdb.set_trace()
        assert total_k_bits == len(self.combined_profile.k_bit_sel), pdb.set_trace()
        assert total_i_bits == len(self.combined_profile.i_bit_sel), pdb.set_trace()

        self.gtm.tm.logger.debug("%s:%s:Split Profiles km0:%s, km1%s" % \
                    (self.gtm.d.name, self.p4_table.name, km0_profile, km1_profile))

    def ct_update_key_offsets(self):
        # XXX for tables that do not share key-makers this is already computed - merge the two
        # methods ....
        if self.key_phv_size == 0:
            return
        
        max_km_width = self.gtm.tm.be.hw_model['match_action']['key_maker_width']
        max_kmB = max_km_width/8

        fk_byte = -1; lk_byte = -1
        if len(self.combined_profile.k_byte_sel):
            fk_byte = self.combined_profile.k_byte_sel[0]
            lk_byte = self.combined_profile.k_byte_sel[-1]

        fk_bit = -1; lk_bit = -1
        if len(self.combined_profile.k_bit_sel):
            fk_bit = self.combined_profile.k_bit_sel[0]
            lk_bit = self.combined_profile.k_bit_sel[-1]
        assert self.num_km, pdb.set_trace()
        _km = self.key_makers[0]
        km0 = _km.shared_km if _km.shared_km else _km
        km1 = None
        if self.num_km > 1:
            _km = self.key_makers[1]
            km1 = _km.shared_km if _km.shared_km else _km

        # compute start offset
        # since key makers are combined, bits may appear before/after/middle of a key
        fk_idx = max_kmB
        if fk_byte in km0.combined_profile.byte_sel:
            fk_idx = km0.combined_profile.byte_sel.index(fk_byte)
            self.start_key_off = (fk_idx * 8) + self.start_key_off_delta

        if fk_bit in km0.combined_profile.bit_sel:
            if km0.combined_profile.bit_loc < fk_idx:
                bit_idx = km0.combined_profile.bit_sel.index(fk_bit)
                self.start_key_off = (km0.combined_profile.bit_loc*8) + \
                        self.start_key_off_delta + bit_idx
                self.gtm.tm.logger.debug("%s:Using km0 bit_location %d as k-start" % \
                        (self.p4_table.name, self.start_key_off))
                #pdb.set_trace() # need to test

        fk_idx = max_kmB

        if self.start_key_off == -1:
            assert km1
            if fk_byte in km1.combined_profile.byte_sel:
                fk_idx = km1.combined_profile.byte_sel.index(fk_byte)
                self.start_key_off = (fk_idx * 8) + self.start_key_off_delta + max_km_width
            if km1 and fk_bit in km1.combined_profile.bit_sel:
                if km1.combined_profile.bit_loc < fk_idx:
                    bit_idx = km1.combined_profile.bit_sel.index(fk_bit)
                    self.start_key_off = (km1.combined_profile.bit_loc*8) + \
                        self.start_key_off_delta + bit_index
                    self.gtm.tm.logger.debug("%s:Using km1 bit_location %d as k-start" % \
                        (self.p4_table.name, self.start_key_off))
                    #pdb.set_trace() # need to test

        assert self.start_key_off != -1, pdb.set_trace()

        # compute end offset
        lk_end_byte = -1
        if lk_bit in km0.combined_profile.bit_sel:
            lk_bit_idx = km0.combined_profile.bit_sel.index(lk_bit)
            self.end_key_off = (km0.combined_profile.bit_loc*8) + lk_bit_idx + 1
            lk_end_byte = km0.combined_profile.bit_loc
        if lk_byte in km0.combined_profile.byte_sel:
            lk_idx = km0.combined_profile.byte_sel.index(lk_byte)
            if lk_idx > lk_end_byte:
                self.end_key_off = (lk_idx * 8) + 8 - self.end_key_off_delta
                self.gtm.tm.logger.debug("%s:Using km0 byte_location %d as k-end" % \
                        (self.p4_table.name, self.end_key_off))

        lk_end_byte = -1
        if self.end_key_off == -1:
            assert km1
            if lk_bit in km1.combined_profile.bit_sel:
                lk_bit_idx = km1.combined_profile.bit_sel.index(lk_bit)
                self.end_key_off = (km1.combined_profile.bit_loc*8) + lk_bit_idx + 1 + max_km_width
                lk_end_byte = km1.combined_profile.bit_loc
            if lk_byte in km1.combined_profile.byte_sel:
                lk_idx = km1.combined_profile.byte_sel.index(lk_byte)
                if lk_idx > lk_end_byte:
                    self.end_key_off = (lk_idx * 8) + 8 + max_km_width - self.end_key_off_delta
                    self.gtm.tm.logger.debug("%s:Using km1 byte_location %d as k-end" % \
                            (self.p4_table.name, self.end_key_off))
        assert self.end_key_off != -1, pdb.set_trace()
        # For index and hash tables that use multiple key makers,
        # if key is split between the two kms and if # km0 is not fully used, right justify the key
        if (self.start_key_off / max_km_width) != (self.end_key_off / max_km_width):
            # key start and end in different key-makers
            if not self.is_tcam_table():
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

        new_key_size = self.end_key_off - self.start_key_off
        self.final_key_size = new_key_size

    def _fix_hbm_hash_table_km_profile(self):
        # XXX HACK: Now it is done for non-hbm tables as well, since hw has new config
        # this can be handled differently, need api changes to go with it
        action_id_size = self.gtm.tm.be.hw_model['match_action']['action_id_size']

        if self.start_key_off >= action_id_size:
            return
        #pdb.set_trace()
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

        if shared_km:
            # XXX need to fix all shared tables.. if shared table is an idx table, more checks
            return 

        assert(len(km_prof.byte_sel) < max_kmB)
        for _ in range((action_id_size+7)/8):
            km_prof.byte_sel.insert(0, -1)
            self.start_key_off += 8
            self.end_key_off += 8

    def _fix_tcam_table_km_profile(self):
        # XXX make it a generic optimization
        # This function is written to handle a specific case seen while compiling nic.p4
        # where a tcam table needs two key makers and one of the key-maker is shared with another
        # tcam table. In this case a lot of fields were common between two key makers,
        # those fields can be removed from one (non-shared) key maker and key can be compressed
        if self.num_km < 2:
            return

        if (self.final_key_size - self.key_size) < 64:
            return  # not worth fixing it ??? XXX

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
            return

        max_km_width = self.gtm.tm.be.hw_model['match_action']['key_maker_width']
        max_kmB = max_km_width/8

        assert km0_shared or km1_shared, pdb.set_trace()

        common_byte_sel = set(km0_prof.byte_sel) & set(km1_prof.byte_sel)
        common_bit_sel = set(km0_prof.bit_sel) & set(km1_prof.bit_sel)
        if not km0_shared:
            fix_km_prof = km0_prof
        else:
            fix_km_prof = km1_prof

        if len(common_byte_sel) == 0:
            return # nothing is common

        # remove the -1 bytes
        if fix_km_prof.bit_loc1 != -1:
            fix_km_prof.byte_sel.pop(fix_km_prof.bit_loc1)
            # bit loc1 is after bit_loc..so bit_loc index is still valid
        if fix_km_prof.bit_loc != -1:
            fix_km_prof.byte_sel.pop(fix_km_prof.bit_loc)

        if -1 in common_byte_sel:
            common_byte_sel.remove(-1)

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

        def _fix_bit_loc(fix_km_prof, max_kmB, right_justify):
            # since this km_prof is not shared with anyone, arrange the bytes and bits
            # as needed, fix the bit_loc and start/end_key_off
            # after removing duplicates the km structure is -
            # | i1_bytes | k_bytes | ki_bits | i2_bytes |
            # right justify the bytes
            # arrange it as -
            # | ...[pad]....i1_bytes | k_bytes | ki_bits | i2_bytes |
            if len(fix_km_prof.bit_sel):
                num_byte = (len(fix_km_prof.bit_sel) + 7) / 8
                if len(fix_km_prof.k_byte_sel):
                    bit_loc = fix_km_prof.byte_sel.index(fix_km_prof.k_byte_sel[-1]) + 1
                elif len(fix_km_prof.i1_byte_sel):
                    bit_loc = fix_km_prof.byte_sel.index(fix_km_prof.i1_byte_sel[-1]) + 1
                elif len(fix_km_prof.i2_byte_sel):
                    bit_loc = fix_km_prof.byte_sel.index(fix_km_prof.i2_byte_sel[0])
                else:
                    if right_justify:
                        bit_loc = max_kmB
                    else:
                        bit_loc = 0

                fix_km_prof.bit_loc = bit_loc
                if num_byte > 1:
                    if bit_loc == max_kmB:
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
            
        if not km0_shared:
            _fix_bit_loc(fix_km_prof, max_kmB, True)
        else:
            _fix_bit_loc(fix_km_prof, max_kmB, False)

        # need to recompute key_offsets - done by caller

    def _fix_idx_table_km_profile(self):
        # if k_bits: need to move those the the end of bit_sel
        # if no kbits - just need a byte alignement
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

        assert km_prof == end_km_prof, pdb.set_trace() # not allowed

        max_km_bits = self.gtm.tm.be.hw_model['match_action']['num_bit_extractors']

        is_shared_idx_table = False
        if km_shared:
            shared_idx_tables = [x for x in self.key_makers[start_km].shared_km.ctables \
                                    if x.is_index_table() and x != self]
            if len(shared_idx_tables):
                #pdb.set_trace()
                is_shared_idx_table = True

        if len(self.combined_profile.k_bit_sel):
            #pdb.set_trace()
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
                        assert 0, pdb.set_trace() # TBD
            return

        # no k-bits - Great!!
        if self.end_key_off % 16:
            #pdb.set_trace()
            assert (self.end_key_off % 8) == 0, pdb.set_trace()
            # XXX when two idx tables share a profile, it is possible to mis-align other table's
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
            # if not, need to conver byte to bit_sel and move a byte using it
            if is_shared_idx_table:
                k_byte_start_idx = km_prof.byte_sel.index(self.combined_profile.k_byte_sel[0])
                k_byte_end_idx = km_prof.byte_sel.index(self.combined_profile.k_byte_sel[-1])
            else:
                k_byte_start_idx = km_prof.byte_sel.index(km_prof.k_byte_sel[0])
                k_byte_end_idx = km_prof.byte_sel.index(km_prof.k_byte_sel[-1])

            if len(km_prof.bit_sel) <= 8:
                # there is at least one bit_sel,
                # convert i1 byte to bit_sel and move it after k bytes
                # or convert i2_byte to bit_sel and move it before k bytes
                # choose a 8 bit chunk if available to avoid splitting a field
                eligible_i1 = [chunk for chunk in self.i1_phv_chunks if chunk[1] == 8]
                eligible_i2 = [chunk for chunk in self.i2_phv_chunks if chunk[1] == 8]
                if len(eligible_i1):
                    byte_to_bit = eligible_i1[0][0] / 8
                    km_prof.byte_sel.remove(byte_to_bit)
                    if km_prof.bit_loc < 0:
                        km_prof.bit_loc = k_byte_end_idx
                    else:
                        km_prof.bit_loc1 = k_byte_end_idx

                    km_prof.byte_sel.insert(k_byte_end_idx, -1)
                    for i in range(8):
                        km_prof.bit_sel.append((byte_to_bit * 8) + i)
                        km_prof.i_bit_sel.append((byte_to_bit * 8) + i)

                    km_prof.i1_byte_sel.remove(byte_to_bit)
                    self.start_key_off -= 8
                    self.end_key_off -= 8
                    self.gtm.tm.logger.debug( \
                        "%s:Converted i1 byte %d to bits and located at %d in km after K" % \
                        (self.p4_table.name, byte_to_bit, km_prof.bit_loc))

                elif len(eligible_i2):
                    byte_to_bit = eligible_i2[0][0] / 8
                    km_prof.byte_sel.remove(byte_to_bit)
                    if km_prof.bit_loc < 0:
                        km_prof.bit_loc = k_byte_start_idx
                    else:
                        km_prof.bit_loc1 = k_byte_start_idx
                    km_prof.byte_sel.insert(k_byte_start_idx, -1)
                    for i in range(8):
                        km_prof.bit_sel.append((byte_to_bit * 8) + i)
                        km_prof.i_bit_sel.append((byte_to_bit * 8) + i)

                    km_prof.i2_byte_sel.remove(byte_to_bit)
                    self.start_key_off += 8
                    self.end_key_off += 8
                    self.gtm.tm.logger.debug( \
                        "%s:Converted i2 byte %d to bits and located at %d in km before K" % \
                        (self.p4_table.name, byte_to_bit, km_prof.bit_loc))

                else:
                    self.gtm.tm.logger.critical( \
                        "Need to implement more sophisticated algo to align key for %s" % \
                        (self.p4_table.name))
                    assert 0, pdb.set_trace()

                return

            # move the i-bits around to shift k by 1 byte
            assert km_prof.bit_loc != -1, pdb.set_trace()
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
        self.cfield_vals = []   # list of (cf, value) used in p4 condition
        self.cwidth = 0

        def _expand(p4_expr):
            supported_ops = ['and', '==', 'valid']
            assert p4_expr.op in supported_ops, \
                "Unsupported op %s in control-flow. Allowed ops %s" % (p4_expr.op, supported_ops)

            if p4_expr.op == 'and':
                assert isinstance(p4_expr.left, p4.p4_expression), "Invalid condition"
                assert isinstance(p4_expr.right, p4.p4_expression), "Invalid condition"
                # check bits accumulated so far
                assert self.cwidth <= 8, pdb.set_trace()
                _expand(p4_expr.left)
                assert self.cwidth <= 8, pdb.set_trace()
                _expand(p4_expr.right)
                return

            elif p4_expr.op == '==':
                if isinstance(p4_expr.left, p4.p4_field):
                    hf_name = get_hfname(p4_expr.left)
                    cf = self.gtm.tm.be.pa.get_field(hf_name, self.gtm.d)
                    assert cf, pdb.set_trace()
                    assert isinstance(p4_expr.right, int)
                    cval = p4_expr.right
                elif isinstance(p4_expr.right, p4.p4_field):
                    hf_name = get_hfname(p4_expr.right)
                    cf = self.gtm.tm.be.pa.get_field(hf_name, self.gtm.d)
                    assert cf, pdb.set_trace()
                    assert isinstance(p4_expr.left, int)
                    cval = p4_expr.left
                else:
                    assert 0, pdb.set_trace()
            elif p4_expr.op == 'valid':
                assert isinstance(p4_expr.right, p4.p4_header_instance), pdb.set_trace()
                hf_name = p4_expr.right.name + '.valid'
                cf = self.gtm.tm.be.pa.get_field(hf_name, self.gtm.d)
                assert cf, pdb.set_trace()
                cval = 1
            else:
                assert 0, pdb.set_trace()

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
        assert isinstance(cts, list)
        self.km_id = -1 # logical id 0, 1 when multiple key-makers used
        self.hw_id = -1
        self.is_shared = False
        self.shared_km = None
        self.has_key = False
        self.ctables = cts
        self.flits_used = []
        self.combined_profile = None
        # keep flit components of the combined profile.. these are needed while
        # sharing profiles across tables
        self.flit_km_profiles = OrderedDict() # {fid: profile}                     

    def _merge(self, rhs):
        #pdb.set_trace()
        # merge two key_makers XXX can be __add__
        for ct in rhs.ctables:
            if ct not in self.ctables:
                self.ctables.append(ct)
        base_table = None
        idx_tbls = [ct for ct in self.ctables if ct.is_index_table()]
        h_tbls = [ct for ct in self.ctables if ct.is_hash_table()]

        if len(h_tbls):
            assert len(idx_tbls) == 0, pdb.set_trace()
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
            '''
            # Following code cause flit order violation - not useful - will be removed/fixed
            # for tcam tables, use table with smaller key as base table, otherwise
            # tcam width can increase
            key_sorted_tbls = sorted(self.ctables, key=lambda k:k.key_phv_size)
            base_table = key_sorted_tbls[0]
            '''

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
                #pdb.set_trace()
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
        # merge must take into account table types
        for fid in sorted(self.flit_km_profiles.keys()):
            if base_table:
                # make sure all hash and idx table keys are kept contiguous
                k_byte_sel = []; i1_byte_sel = []; i2_byte_sel = []
                fk_byte = -1
                for b in self.flit_km_profiles[fid].k_byte_sel:
                    if b in base_table.combined_profile.k_byte_sel:
                        k_byte_sel.append(b)

                if len(k_byte_sel):
                    fk_byte = k_byte_sel[0]
                    for b in self.flit_km_profiles[fid].i1_byte_sel:
                        if b < fk_byte:
                            i1_byte_sel.append(b)
                        else:
                            i2_byte_sel.append(b)

                    for b in self.flit_km_profiles[fid].k_byte_sel:
                        # pick up k bytes of shared table (non hash/idx)
                        if b in k_byte_sel:
                            continue
                        if b < fk_byte:
                            i1_byte_sel.append(b)
                        else:
                            i2_byte_sel.append(b)
                    i2_byte_sel += self.flit_km_profiles[fid].i2_byte_sel
                    # XXX update the k, i1, i2 in the flit?? May cause problems in other code
                    self.combined_profile.i1_byte_sel += sorted(i1_byte_sel)
                    self.combined_profile.i2_byte_sel += sorted(i2_byte_sel)
                    self.combined_profile.k_byte_sel += sorted(k_byte_sel)
                    self.combined_profile.k_bit_sel += self.flit_km_profiles[fid].k_bit_sel
                    self.combined_profile.i_bit_sel += self.flit_km_profiles[fid].i_bit_sel
                    continue
                else:
                    pass # no special processing - follow common code
                    
            # common code
            self.combined_profile.i1_byte_sel += self.flit_km_profiles[fid].i1_byte_sel
            self.combined_profile.i2_byte_sel += self.flit_km_profiles[fid].i2_byte_sel
            self.combined_profile.k_byte_sel += self.flit_km_profiles[fid].k_byte_sel
            self.combined_profile.k_bit_sel += self.flit_km_profiles[fid].k_bit_sel
            self.combined_profile.i_bit_sel += self.flit_km_profiles[fid].i_bit_sel
        self.combined_profile.byte_sel = self.combined_profile.i1_byte_sel + \
            self.combined_profile.k_byte_sel + self.combined_profile.i2_byte_sel

        self.combined_profile.bit_sel = self.combined_profile.k_bit_sel + \
            self.combined_profile.i_bit_sel
        #pdb.set_trace()

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

        #pdb.set_trace()
        hash_ct = None
        if len(hash_ctbls):
            #pdb.set_trace()
            hash_ct = hash_ctbls[0]
            # assert on un-supported km sharing
            assert len(hash_ctbls) == 1, pdb.set_trace()
            assert len(idx_ctbls) == 0, pdb.set_trace()

        if len(idx_ctbls):
            # assert on un-supported km sharing
            # I think multiple idx tables can share km if both do not need k_bits XXX
            self.stage.gtm.tm.logger.debug("Merging multiple index tables %s" % \
                ([ct.p4_table.name for ct in idx_ctbls]))
            #pdb.set_trace()

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
                    assert k_byte >= 0, pdb.set_trace()
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
            assert len(self.combined_profile.byte_sel) <= max_kmB, pdb.set_trace()
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
                    assert k_byte >= 0, pdb.set_trace()
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
                k_bytes = set(self.combined_profile.k_byte_sel) & \
                            set(idx_ct.combined_profile.k_byte_sel)
                if len(k_bytes):
                    k_byte = -1
                    for b in reversed(self.combined_profile.k_byte_sel):
                        if b in idx_ct.combined_profile.k_byte_sel:
                            k_byte = b
                            break
                    assert k_byte >= 0, pdb.set_trace()
                    bit_loc = self.combined_profile.byte_sel.index(k_byte) + 1
                    num_bytes = (len(self.combined_profile.bit_sel)+7) / 8
                    for i in range(num_bytes):
                        self.combined_profile.byte_sel.insert(bit_loc+i, -1)
                    self.combined_profile.bit_loc = bit_loc
                    if num_bytes > 1:
                        self.combined_profile.bit_loc1 = bit_loc + 1
                else:
                    self.combined_profile._update_bit_loc_key_off()
            assert len(self.combined_profile.byte_sel) <= max_kmB, pdb.set_trace()
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
                assert k_byte >= 0, pdb.set_trace()
                bit_loc = self.combined_profile.byte_sel.index(k_byte) + 1
                num_bytes = (len(self.combined_profile.bit_sel)+7) / 8
                for i in range(num_bytes):
                    self.combined_profile.byte_sel.insert(bit_loc+i, -1)
                self.combined_profile.bit_loc = bit_loc
                if num_bytes > 1:
                    self.combined_profile.bit_loc1 = bit_loc + 1
            else:
                self.combined_profile._update_bit_loc_key_off()
                
        assert len(self.combined_profile.byte_sel) <= max_kmB, pdb.set_trace()
        return

    def get_hw_id(self):
        if self.shared_km:
            return self.shared_km.hw_id
        return self.hw_id

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
            assert my_flit == self.byte_sel[-1] / flit_szB, pdb.set_trace()
        if len(rhs.byte_sel):
            rhs_flit = rhs.byte_sel[0] / flit_szB
            assert rhs_flit == rhs.byte_sel[-1] / flit_szB, pdb.set_trace()

        if my_flit != -1 and rhs_flit != -1:
            assert my_flit == rhs_flit, pdb.set_trace()


        #pdb.set_trace()
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

        #pdb.set_trace()
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
        assert(self.byte_sel) == sorted(self.byte_sel)
        
        for b in rhs.k_bit_sel:
            if b not in self.bit_sel:
                self.k_bit_sel.append(b)
        for b in rhs.i_bit_sel:
            if b not in self.bit_sel:
                self.i_bit_sel.append(b)
        self.bit_sel = self.k_bit_sel + self.i_bit_sel

        assert self.km_prof_size() <= (old_size + rhs_size), pdb.set_trace()
        self.gtm.tm.logger.debug("Added km_profiles: %d + %d = %d" % \
            (old_size, rhs_size, self.km_prof_size()))
        
        # need to recompute these
        self.bit_loc = self.bit_loc = -1     # byte(s) reseved for bit extracted values
        self.bit_loc1 = self.bit_loc1 = -1     # byte(s) reseved for bit extracted values
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

        new_obj.bit_loc = self.bit_loc = -1     # byte(s) reseved for bit extracted values
        new_obj.bit_loc1 = self.bit_loc1 = -1     # byte(s) reseved for bit extracted values
        new_obj.start_key_off = self.start_key_off
        new_obj.end_key_off = self.end_key_off

        return new_obj

    def create_2B_profile(self, km_prof, use_low = True):
        km_prof_size = self.gtm.tm.be.hw_model['match_action']['key_maker_width']
        km_sizeB = km_prof_size/8
        km_size2B = km_sizeB/2

        if use_low:
            assert len(self.byte_sel) == 0, pdb.set_trace() # low portion is already used
        else:
            assert len(self.byte_sel) == km_size2B, pdb.set_trace() # low portion is already used
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
                    #pdb.set_trace()
            elif len(self.i2_byte_sel):
                # place before i2_bytes
                bit_loc = self.byte_sel.index(self.i2_byte_sel[0])
            elif len(self.i1_byte_sel):
                bit_loc = self.byte_sel.index(self.i1_byte_sel[0]) + 1
            else:
                bit_loc = 0
            assert bit_loc >= 0, pdb.set_trace()
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
            assert b not in bytes_sel, pdb.set_trace()
            bytes_sel[b/flit_szB].append(b)

        for b in self.bit_sel:
            if b < 0:
                continue
            assert b not in bits_used, pdb.set_trace()
            if b not in bits_used:
                bits_used[b] = 1

        # Banyon network violation
        for fb_sel in bytes_sel:
            if len(fb_sel) == 0:
                continue
            assert sorted(fb_sel) == fb_sel, pdb.set_trace()
            pass

    def __repr__(self):
        if len(self.byte_sel) or len(self.bit_sel):
            return '[%d] : byte_sel %s :\t\tbit_sel %s' % (self.hw_id, self.byte_sel, self.bit_sel)
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
        self.ct_list = None
        self.table_profiles = OrderedDict() # {pred_val : [tables to apply]}
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
                #pdb.set_trace()
                pass
        return True
        
    def _share_key_maker(self, fid, ct, new_km):
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
                if km.is_shared:
                    if not ct.is_raw:
                        # XXX multi-way sharing... need to check and operate on shared key-maker
                        self.gtm.tm.logger.debug("km is already shared with another table")
                        continue
                    # allow multi-way sharing for raw tables
                    km_profile = km.shared_km.combined_profile

                u_byte_sel = set(km_profile.byte_sel) | ct_byte_sel
                # remove the bytes reserved for bit_sel
                u_byte_sel = u_byte_sel - set([-1])
                u_bit_sel = set(km_profile.bit_sel) | ct_bit_sel
                if len(u_bit_sel) > km_max_bits:
                    #pdb.set_trace()
                    continue
                if (len(u_byte_sel) + (len(u_bit_sel)+7)/8) > (km_width/8):
                    #pdb.set_trace()
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
                assert ct.is_tcam_table(), pdb.set_trace()
                if not self._can_share_hash_tcam_km(km_found, new_km):
                    self.gtm.tm.logger.debug("Cannot merge %s into %s" % \
                        (ct.p4_table.name, kt.p4_table.name))
                    continue

            if ct.is_hash_table():
                assert kt.is_tcam_table(), pdb.set_trace()
                if not self._can_share_hash_tcam_km(new_km, km_found):
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

            assert km_found.hw_id != -1
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
        if ct.is_otcam:
            #pdb.set_trace()
            return True
        if ct.num_km == 0:
            ct.key_makers[0].hw_id = 0
            return True
        total_km_allocated = 0
        # record already assigned hw_ids. hw_id used for one km of a table should not be
        # used for the other km
        km_used = [_km.get_hw_id() for _km in ct.key_makers]

        for c_km in ct.key_makers:
            if fid not in c_km.flits_used:
                total_km_allocated += 1
                continue
            if c_km.shared_km:
                new_km = c_km.shared_km
            else:
                new_km = c_km

            if new_km.hw_id != -1:
                # already allocated (earlier flit)
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
                if set(km.ctables) <= p_excl_tbls[ct]:
                    new_km.hw_id = km.hw_id
                    # allocate km in all flits used
                    for f in new_km.flits_used:
                        self.km_allocator[f][new_km.hw_id] = new_km
                        self.gtm.tm.logger.debug("%s:[flit %d] reuse key_maker[%d] %d" % \
                            (ct.p4_table.name, f, c_km.km_id, km.hw_id))
                    km.ctables.append(ct)
                    km_allocated += 1
                    total_km_allocated += 1
                    km_used[c_km.km_id] = new_km.hw_id
                    break
            if km_allocated != 0:
                continue
            # check other criteria for sharing km between tables
            if need_sharing and self._share_key_maker(fid, ct, new_km):
                for f in new_km.flits_used:
                    self.km_allocator[f][new_km.shared_km.hw_id] = new_km.shared_km
                    self.gtm.tm.logger.debug(\
                        "%s:%d:%s:[flit %d] Assign key_maker[%d] %d (shared)" % \
                        (self.gtm.d.name, self.id, ct.p4_table.name, f, c_km.km_id, 
                        new_km.shared_km.hw_id))
                total_km_allocated += 1
                km_used[c_km.km_id] = new_km.shared_km.hw_id
                continue

            # allocate new key maker
            if self.km_allocator[fid].count(None) == 0:
                return False
            assert self.km_allocator[fid].count(None), pdb.set_trace()
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

        assert total_km_allocated == ct.num_km, pdb.set_trace()
        return True

    def get_2B_mode(self, km_prof, ct):
        # return 0: no 2B mode allowed, 1 : 2B mode odd byte, 2: 2B mode even byte
        if len(km_prof.bit_sel):
            # XXX checking with asic team on constraints, not allowed till then
            return 0

        if not ct.is_raw:
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
            if km_prof.byte_sel[b+1] != km_prof.byte_sel[b]+1:
                _2B_ok = False
                break
        if _2B_ok:
            return 2    # even bytes in key_maker, 0, 2, 4, ... 

        if len(km_prof.byte_sel) > (max_kmB - 2):
            return 0    # will need extra bytes in key_maker for two ends

        pdb.set_trace() # un-tested code
        _2B_ok = True
        for i in range(0, (len(km_prof.byte_sel)/2) - 1):
            b = (i*2) + 1
            if km_prof.byte_sel[i+1] != km_prof.byte_sel[i]+1:
                _2B_ok = False
                break
        if _2B_ok:
            return 1    # combine odd bytes: 1, 3, 5, ...

        return 0

    def program_tables(self):
        num_flits = self.gtm.tm.be.hw_model['phv']['num_flits']
        km_width  = self.gtm.tm.be.hw_model['match_action']['key_maker_width']
        km_bits  = self.gtm.tm.be.hw_model['match_action']['num_bit_extractors']
        max_kmB = km_width / 8

        ct_list = []
        tbl_id = 1  # avoid 0 ??
        for t in self.p4_table_list:
            if isinstance(t, p4.p4_conditional_node):
                continue
            ct = self.gtm.tables[t.name]
            ct_list.append(ct)
            if not ct.is_otcam:
                ct.tbl_id = tbl_id
                tbl_id += 1
            # create km_profiles (logical) for each table, no hw resources are assigned yet
            ct.ct_create_km_profiles()
            ct.create_key_makers()

        self.ct_list = ct_list

        # for otcam, use same tbl_id as its hash table
        for ct in ct_list:
            if not ct.is_otcam:
                continue
            ct.tbl_id = ct.hash_ct.tbl_id

        key_makers = []
        per_flit_kms = [[] for _ in range(num_flits)]
        per_flit_tables = [[] for _ in range(num_flits)]
        #pdb.set_trace()
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

        #pdb.set_trace()
        new_per_flit_kms = [[] for _ in range(num_flits)]
        max_km = self.gtm.tm.be.hw_model['match_action']['num_key_makers']

        km_violation_fid = OrderedDict()

        fid_excl_tables = OrderedDict()
        for fid, ctg in enumerate(per_flit_tables):
            fid_excl_tables[fid] = []
            for et in p_excl_tbls.keys():
                if et in ctg:
                    for et2 in p_excl_tbls[et]:
                        if et2 in ctg:
                            if et not in fid_excl_tables[fid]:
                                fid_excl_tables[fid].append(et)
                            if et2 not in fid_excl_tables[fid]:
                                fid_excl_tables[fid].append(et2)

        #pdb.set_trace()

        for fid, ctg in enumerate(per_flit_tables):
            self.gtm.tm.logger.debug("Per Flit Tables[%d] = %s" % (fid, ctg))

        for fid, ctg in enumerate(per_flit_tables):
            num_km = sum(ct.num_km for ct in ctg if ct not in fid_excl_tables[fid])
            if len(fid_excl_tables[fid]):
                num_km += max([ct.num_km  for ct in fid_excl_tables[fid]])
            if num_km > max_km:
                self.gtm.tm.logger.warning( \
                    "%s:Resource Pressure:Stg %d flit %d KeyMakers %d" % \
                    (self.gtm.d.name, self.id, fid, num_km))
                km_violation_fid[fid] = num_km

        #pdb.set_trace()

        # allocate hw_keymakers to each key maker based on flit usage
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
                # following logic is added to get a simple programs to compile w/o sharing
                need_sharing = True if fid in km_violation_fid else False
                if not self._allocate_km(fid, ct, p_excl_tbls, need_sharing):
                    self.gtm.tm.logger.critical( \
                        "%s:%d:Not enough key-makers for %s Revise P4 program and try" % \
                        (self.gtm.d.name, self.id, ct.p4_table.name))
                    continue

        # fix table key-makers E.g. assign otcams to get the same km as its hash table
        for ct in self.ct_list:
            if ct.is_otcam:
                # added the key makers to otcam to keep rest of the code happy
                # not used for programming
                for km in ct.hash_ct.key_makers:
                    ct.key_makers.append(km)

        # create flit/cycle launch sequence for each table profile
        for prof_id,ctg in self.table_profiles.items():
            self._create_table_launch_seq(prof_id, ctg)

        # XXX calculate the bit_loc for combined KMs - tricky when hash tables are involved
        for ct in self.ct_list:
            for km in ct.key_makers:
                if km.shared_km:
                    # pdb.set_trace()
                    km.shared_km._assign_bit_loc()

        # update key offsets so that all offsets are initialized before sorting and fixing 
        # km_profiles based on hardware constraints
        for ct in ct_list:
            ct.ct_update_key_offsets()
        ct_list = sorted(self.ct_list, key=lambda c: c.start_key_off)
        for ct in ct_list:
            # do it again in case other shared table moved the bytes in km_profile
            ct.ct_update_key_offsets()
            if ct.is_index_table():
                ct._fix_idx_table_km_profile()
            if ct.is_hbm and ct.is_hash_table() and ct.num_actions() > 1:
                ct._fix_hbm_hash_table_km_profile()
            if ct.is_tcam_table() and ct.final_key_size > ct.key_phv_size:
                ct._fix_tcam_table_km_profile()

        # re-run key_offset calculation as any table sharing km_profile with index table
        # my have changes in offset
        # skip idx tables since the common routine does not handle i and k bit switch
        for ct in ct_list:
            if not ct.is_index_table():
                ct.ct_update_key_offsets()
            
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
        if km_profiles_used > max_km_profiles:
            # "Not enough km_profiles"
            assert 0, pdb.set_trace()

        # assign hw_ids to km_profiles
        # need to create final hw_profiles with correct byte_sels
        hw_id = 0
        for km_prof in km_prof_normal:
            #pdb.set_trace()
            km_prof.hw_id = hw_id
            km_prof.mode = 0 # normal
            hw_id += 1
            self.hw_km_profiles[km_prof.hw_id] = km_prof

        i = 0
        hw_prof2B = None
        for i, km_prof in enumerate(km_prof_2B):
            #pdb.set_trace()
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

                assert len(km_prof.byte_sel) <= max_kmB, pdb.set_trace()

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
        max_cycles = self.gtm.tm.be.hw_model['match_action']['num_cycles']
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
                assert ct.is_mpu_only()
                ct.key_makers[0].hw_id = 0 # force km0
                launch_any_time.append(ct)
                continue
            flit_launch_tbls[ct.launch_flit].append(ct)
            for fid in ct.flits_used:
                flits_used[fid] = True
                if fid > last_flit_used:
                    last_flit_used = fid

        launch_seq = [capri_te_cycle() for _ in range(max_cycles)]
        cycle = 0
        
        for fid in range(last_flit_used+1):
            assert cycle < max_cycles, pdb.set_trace()
            
            if len(flit_launch_tbls[fid]) == 0:
                if flits_used[fid]:
                    launch_seq[cycle].is_used = True
                else:
                    launch_seq[cycle].is_used = False
                launch_seq[cycle].adv_flit = True
                launch_seq[cycle].fid = fid
                if len(launch_any_time):
                    ct = launch_any_time.pop(0)
                    launch_seq[cycle].tbl = ct
                cycle += 1
                continue

            if not flits_used[fid]:
                launch_seq[cycle].is_used = False
                launch_seq[cycle].adv_flit = True
                launch_seq[cycle].fid = fid
                cycle += 1
                continue

            if len(flit_launch_tbls[fid]) == 1:
                launch_seq[cycle].tbl = flit_launch_tbls[fid][0]
                launch_seq[cycle].is_used = True
                launch_seq[cycle].adv_flit = True
                launch_seq[cycle].fid = fid
                cycle += 1
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
                launch_seq[cycle].tbl = ct
                launch_seq[cycle].adv_flit = False
                launch_seq[cycle].is_used = True
                launch_seq[cycle].fid = fid
                cycle += 1

            if last_ct:
                # this table's key maker is not reused on the following flit
                launch_seq[cycle].tbl = last_ct
                launch_seq[cycle].adv_flit = True
                launch_seq[cycle].is_used = True
                launch_seq[cycle].fid = fid
                cycle += 1
            else:
                #launch_seq[cycle-1].adv_flit = True
                # keep adv_flit = False, it is updated at the end
                pass
            
        if len(launch_any_time):
            pdb.set_trace() # Need test case
            launch_seq[cycle-1].adv_flit = False

        for ct in launch_any_time:
            # still some key-less mpu-only tables need to be launched 
            assert cycle < max_cycles
            launch_seq[cycle].tbl = ct
            launch_seq[cycle].adv_flit = False
            launch_seq[cycle].is_used = True
            launch_seq[cycle].fid = fid
            cycle += 1

        # Advance flit on the very last flit
        assert cycle <= max_cycles, pdb.set_trace()
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

        #pdb.set_trace()

    def stg_get_tbl_profile_key(self):
        total_w = 0
        cf_list = []
        max_pred_bits = self.gtm.tm.be.hw_model['match_action']['num_predicate_bits']
        max_pred_phv_bit = self.gtm.tm.be.hw_model['match_action']['range_predicate_bit']
        for cond in self.active_predicates:
            cp = self.gtm.table_predicates[cond]
            for cf,_ in cp.cfield_vals:
                assert cf.phv_bit < max_pred_phv_bit, pdb.set_trace()
                if cf not in cf_list:
                    cf_list.append(cf)
                    total_w += cf.width
        assert total_w <= max_pred_bits, pdb.set_trace()
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
        assert val < test_val, pdb.set_trace()

        # same cfield can appear in multiple condition, get list of unique fields and
        # create a map
        total_w, cf_list = self.stg_get_tbl_profile_key()
        cf_val_mask = OrderedDict() # {cf: (val, mask, flag[False = cannot be X]}

        #pdb.set_trace()
        invalid_condition = False
        for c in self.active_predicates:
            #cond_tcam_entry = []
            cp = self.gtm.table_predicates[c]
            test_val >>= 1
            assert test_val, pdb.set_trace()
            if val & test_val:
                # condition is true
                for cf, v in cp.cfield_vals:
                    if cf in cf_val_mask:
                        # check for conflicting conditions... XXX
                        if cf_val_mask[cf][1] != 0 and v != cf_val_mask[cf][0]:
                            invalid_condition = True
                            break
                    cf_val_mask[cf] = (v, ((1<<cf.width) - 1))
            else:
                for cf, v in cp.cfield_vals:
                    if cf.width == 1:
                        # For a single bit field, false value is opposite (1-v) or True value
                        # Programming this may result in redundant entries in TCAM - XXX optimize
                        if cf not in cf_val_mask:
                            cf_val_mask[cf] = ((1-v), ((1<<cf.width) - 1))
                    else:
                        if cf not in cf_val_mask:
                            cf_val_mask[cf] = (0, 0)

        # build the tcam val, mask
        if invalid_condition:
            return []
        tcam_val = 0
        tcam_mask = 0
        for cf in cf_list:
            (v, m) = cf_val_mask[cf]
            tcam_val = (tcam_val << cf.width) | v
            tcam_mask = (tcam_mask << cf.width) | m

        #pdb.set_trace()
        return [(tcam_val, tcam_mask)]

    def stg_generate_output(self):
        capri_te_cfg_output(self)

    def stg_te_dbg_output(self):
        max_km_width = self.gtm.tm.be.hw_model['match_action']['key_maker_width']
        max_kmB = max_km_width/8
        # generate table engine debug output
        debug_info = OrderedDict()
        debug_info['Tables'] = OrderedDict()
        for ct in self.ct_list:
            tbl_dbg_info = OrderedDict()
            tbl_id_str = '%s' % ct.tbl_id
            if ct.is_otcam:
                tbl_id_str += '_%s' % ct.p4_table.name
            tbl_dbg_info['name'] = ct.p4_table.name
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
            debug_info['Tables'][tbl_id_str] = tbl_dbg_info

        return debug_info
            
class capri_te_cycle:
    is_used = False
    tbl = None
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
        # hardware information
        # XXX do we need a class to manage resources in each stage??
        self.stages = OrderedDict()

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

    def init_tables(self, stage_tables):
        action_id_size = self.tm.be.hw_model['match_action']['action_id_size']
        self.stage_tables = stage_tables
        stg = -1
        for stg_id, table_list in enumerate(stage_tables):
            stg += 1
            if stg not in self.stages:
                self.stages[stg] = capri_stage(self, stg_id)
                self.stages[stg].p4_table_list = table_list
            else:
                self.stages[stg].p4_table_list += table_list

            for i,t in enumerate(sorted(table_list, key=lambda k:k.name)):
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
                        assert ctable.hash_type <= \
                            self.tm.be.hw_model['match_action']['te_consts']['num_hash_types'], \
                            "%s:Unsupported hash type %d, must be 0-3:CRC hash, 4:Toeplitz" % \
                            (ctable.p4_table.name, ctable.hash_type)

                    # Check if table is marked as HBM table.
                    if t._parsed_pragmas and 'table_write' in t._parsed_pragmas:
                        ctable.is_writeback = True

                    # Check if a hash type has been specified for the table.
                    if t._parsed_pragmas and 'policer_table' in t._parsed_pragmas:
                        if t._parsed_pragmas['policer_table'].keys()[0] == 'three_color':
                            ctable.policer_colors = 3
                        else:
                            ctable.policer_colors = 2
                        ctable.is_policer = True

                    is_ternary = False
                    for f, mtype, mask, in t.match_fields:
                        if mtype != p4.p4_match_type.P4_MATCH_EXACT and \
                            mtype != p4.p4_match_type.P4_MATCH_VALID and \
                            mtype != p4.p4_match_type.P4_MATCH_TERNARY:
                            self.tm.logger.critical("%s:Unsupported match type %s for %s:%s" % \
                                (self.d.name, mtype, t.name, get_hfname(f)))
                            assert 0, pdb.set_trace()
                        if mtype != p4.p4_match_type.P4_MATCH_VALID:
                            hf_name = get_hfname(f)
                            cf = self.tm.be.pa.get_field(hf_name, self.d)
                            assert cf, "%s:%s not found" % (self.d.name, hf_name)
                            ctable.keys.append((cf, mtype, mask))
                            cf.is_key = True
                            if mtype == p4.p4_match_type.P4_MATCH_TERNARY:
                                is_ternary = True
                        else:
                            # When ternary match is used, hlir gives pseudo field which gets
                            # handled in the if condition above. Handle exact match here
                            assert isinstance(f, p4.p4_header_instance), pdb.set_trace()
                            hf_name = f.name + '.valid'
                            cf = self.tm.be.pa.get_field(hf_name, self.d)
                            assert cf
                            ctable.keys.append((cf, mtype, mask))

                    ctable.num_entries = t.min_size if t.min_size else t.max_size
                    key_cfs = [cf for cf,_,_ in ctable.keys]
                    key_size = 0
                    for cf, mtype, mask in ctable.keys:
                        # compute key size (w/o pad adjustments)
                        key_size += (cf.width-cf.pad)
                        if cf.is_hdr_union and not is_ternary:
                            self.tm.logger.warning('%s:CAUTION: Hdr union field %s in tbl key %s\n' % \
                                (self.d, cf.hf_name, t.name))
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
                        assert cf, "%s:%s not found (used as raw table mpu_pc" % \
                            (self.d.name, raw_tbl_pc_fld_name)
                        raw_pc_size = self.tm.be.hw_model['match_action']['raw_pc_size']
                        assert cf.width == raw_pc_size, "raw table pc field must be 32 bit" % \
                            raw_pc_size
                        assert(len(ctable.keys) == 1), \
                            "Only one field is allowed as key for a raw table"
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
                        if key_size < 32 and (ctable.num_entries >= (1<<key_size)):
                            ctable.match_type = match_type.EXACT_IDX
                            if key_size > 16:
                                assert len(key_cfs) == 1, \
                                    "Only one key field is supported for index table %s with idx > 16" % \
                                    ctable.p4_table.name
                            for cf in key_cfs:
                                cf.is_index_key = True
                    else:
                        ctable.match_type = match_type.MPU_ONLY

                    _input_flds, _out_flds = t.retrieve_action_fields()
                    input_flds = sorted(_input_flds, key=lambda k: k.name)
                    out_flds = sorted(_out_flds, key=lambda k: k.name)
                    for f in input_flds:
                        # can be p4_field or p4_pseudo_field(.valid)
                        hf_name = get_hfname(f)
                        cf = self.tm.be.pa.get_field(hf_name, self.d)
                        assert cf, pdb.set_trace()
                        if cf.is_scratch:
                            continue
                        if cf not in ctable.input_fields and cf not in key_cfs:
                            cf.is_input = True
                            ctable.input_fields.append(cf)

                    for f in out_flds:
                        # out fields are not really useful at this time since action can write
                        # anywhere in phv
                        if isinstance(f, p4.p4_field):
                            hf_name = get_hfname(f)
                            cf = self.tm.be.pa.get_field(hf_name, self.d)
                            assert cf, pdb.set_trace()
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
                    for act in t.actions:
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
                    assert(0), pdb.set_trace()

        # match overflow-tcam and hash tables
        for ct in self.tables.values():
            if not ct.is_otcam:
                continue
            # find hash table
            assert ct.htable_name in self.tables, "No hash table %s for otcam %s" % \
                (ct.p4_table.name, ct.htable_name)
            ct.hash_ct = self.tables[ct.htable_name]
            assert ct.hash_ct.match_type == match_type.EXACT_HASH, pdb.set_trace()
            ct.hash_ct.match_type = match_type.EXACT_HASH_OTCAM

        self.print_tables()

    def update_table_config(self):
        for ctable in self.tables.values():
            ctable.ct_update_table_config()
        self.update_table_predicates()

    def update_table_predicates(self):
        table_paths = OrderedDict()
        all_tables = []
        for stg in self.stages.keys():
            all_tables += self.stages[stg].p4_table_list
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
                self.tm.logger.debug("%s: %s\n" % (t, [pv if pv != 2 else 'x' for pv in v.values()]))
        # create a list of active conditions per stage. Look at condition variable (predicates)
        # that affect tables in a given stage
        for stg, cstage in self.stages.items():
            table_list = self.stages[stg].p4_table_list
            if len(table_list) == 0:
                # no table in this stage
                continue
            # create profiles per stage
            stage_table_profiles = OrderedDict() # {{predicate_val} : [table_name list]}
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
                        if not match:
                            break

                    if match:
                        if i in stage_table_profiles:
                            stage_table_profiles[i].append(self.tables[t.name])
                        else:
                            stage_table_profiles[i] = [self.tables[t.name]]
                if i in stage_table_profiles:
                    self.tm.logger.debug("0x%x : %s\n" % \
                        (i, [ct.p4_table.name for ct in stage_table_profiles[i]]))

            self.stages[stg].active_predicates = copy.copy(active_preds)
            self.stages[stg].table_profiles = copy.copy(stage_table_profiles)

    def find_table_paths(self, table_list):
        def _find_paths(node, paths, current_path, table_list):
            assert node not in current_path, "Table LOOP at %s" % node.name
            if isinstance(node, p4.p4_conditional_node):
                for nxt_node in node.next_.values():
                    if not nxt_node:
                        paths.append(current_path + [node])
                        continue
                    _find_paths(nxt_node, paths, current_path+[node], table_list)
            else:
                # no support for action function dependency, so really it should be just one
                # next node
                assert len(set(node.next_.values())) == 1, pdb.set_trace()
                for nxt_node in set(node.next_.values()):
                    if not nxt_node:
                        paths.append(current_path + [node])
                        continue
                    _find_paths(nxt_node, paths, current_path+[node], table_list)

        paths = []
        current_path = []
        if len(table_list):
            _find_paths(table_list[0], paths, current_path, table_list)
        return paths

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
    def scavenge_unused_space(self, mem_type, region, table_depth, table_width, index):
        # Get the memory space and its properties
        memory = self.memory[mem_type][region]
        mem_depth = memory['depth']
        mem_width = memory['width']
        mem_space = memory['space']

        # Scan row by row
        for top in range(mem_depth - table_depth + 1):
            # Scan columns looking for free position to start the table at
            for left in range(mem_width - table_width + 1):
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
            if  tables[i]['width']  < table['width'] or \
               (tables[i]['width'] == table['width'] and \
                tables[i]['depth']  < table['depth']):
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
                width = table_width_to_allocation_units('sram', tspec['sram']['width'])
                depth = tspec['num_entries']
                if tspec['overflow']:
                    for temp in table_specs:
                        if tspec['overflow'] == temp['name']:
                            depth += temp['num_entries']
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

        for i in range(len(self.tables['tcam'][region])):
            for j in range(i+1, len(self.tables['tcam'][region])):
                if self.tables['tcam'][region][i]['depth'] < self.tables['tcam'][region][j]['depth']:
                    temp = self.tables['tcam'][region][i]
                    self.tables['tcam'][region][i] = self.tables['tcam'][region][j]
                    self.tables['tcam'][region][j] = temp
                elif self.tables['tcam'][region][i]['depth'] == self.tables['tcam'][region][j]['depth'] and \
                     self.tables['tcam'][region][i]['width'] < self.tables['tcam'][region][j]['width']:
                    temp = self.tables['tcam'][region][i]
                    self.tables['tcam'][region][i] = self.tables['tcam'][region][j]
                    self.tables['tcam'][region][j] = temp
        return

    def map_sram_tables(self, region):

        mem_type = 'sram'
        tables = self.tables[mem_type][region]
        memory = self.memory[mem_type][region]

        memory['space'] = [[0 for x in range(memory['width'])] for y in range(memory['depth'])]

        self.place_sram_table(region, {'top':0, 'left':0}, 0, tables[0]['width'], tables[0]['depth'],
                              memory['width'], memory['depth'])

    def map_hbm_tables(self, region):
        start_pos = 0
        for rgn in sorted(self.memory['hbm']):
            if rgn == region:
                break;
            start_pos += self.memory['hbm'][rgn]['depth']

        current_pos = start_pos
        end_pos = start_pos + self.memory['hbm'][region]['depth']
        for table in self.tables['hbm'][region]:
            table_size = table['width'] * table['depth']
            if end_pos < current_pos + table_size:
                self.logger.critical("map_hbm_tables(): No space in %s region for table %s" % (region, table['name']))
                return

            table['layout'] = OrderedDict()
            table['layout']['top_left'] = {'block' : 0, 'x' : current_pos, 'y' : 0}
            table['layout']['bottom_right'] = {'block' : 0, 'x' : current_pos + table_size - 1, 'y' : 0}
            current_pos += table_size

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
                                    table['layout'] = self.scavenge_unused_space(mem_type, region, depth, width, index)

                                    if mem_type == 'tcam':
                                        break # Can't fold TCAM tables, so break out

                                    width = width * 2
                                    depth = depth / 2

                            if not table['layout']:
                                self.logger.critical("Could not allocate memory for %s %s table \'%s\'." % \
                                                     (region, mem_type, table['name']))

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

            if 'tcam' in table_spec.keys():
                for table in self.tables['tcam'][table_spec['region']]:
                    if table_spec['name'] == table['name']:
                        memory = OrderedDict()
                        memory['entry_width'] = table['width']
                        memory['entry_width_bits'] = table_spec['tcam']['width']
                        memory['layout'] = self.carve_dummy() if not table['layout'] else table['layout']
                        memory['entry_start_index'] = capri_get_tcam_start_address_from_layout(table['layout'])
                        memory['entry_end_index'] = capri_get_tcam_end_address_from_layout(table['layout'])
                        memory['num_buckets'] = capri_get_width_from_layout(table['layout']) / table['width']
                        table_mapping['tcam'] = memory

            if 'sram' in table_spec.keys():
                for table in self.tables['sram'][table_spec['region']]:
                    if table_spec['name'] == table['name']:
                        memory = OrderedDict()
                        memory['entry_width'] = table['width']
                        memory['entry_width_bits'] = table_spec['sram']['width']
                        memory['layout'] = self.carve_dummy() if not table['layout'] else table['layout']
                        memory['entry_start_index'] = capri_get_sram_sw_start_address_from_layout(table['layout'])
                        memory['entry_end_index'] = capri_get_sram_sw_end_address_from_layout(table['layout'])
                        memory['num_buckets'] = capri_get_width_from_layout(table['layout']) / table['width']
                        table_mapping['sram'] = memory

            if 'hbm' in table_spec.keys():
                for table in self.tables['hbm'][table_spec['region']]:
                    if table_spec['name'] == table['name']:
                        memory = OrderedDict()
                        memory['entry_width'] = table['width']
                        memory['entry_start_index'] = capri_get_hbm_start_address_from_layout(table['layout'])
                        memory['entry_end_index'] = capri_get_hbm_end_address_from_layout(table['layout'])
                        memory['num_buckets'] = 1
                        table_mapping['hbm'] = memory

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
        self.table_memory_spec = capri_table_memory_spec_load()
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

    def generate_cap_pic_output(self):

        self.logger.info("Generating cap pics ...")

        pic = capri_pic_csr_load(self) # Load the templates

# 02-p4_tables
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
                        num_bkts = 0 if table['width'] == 0 else capri_get_width_from_layout(layout) / table['width']
                        profile_name = "%s_csr_cfg_table_profile[%d]" % (cap_name, profile_id)
                        profile = pic[mem_type][xgress_to_string(direction)][cap_name]['registers'][profile_name]
                        profile['width']['value'] = "0x%x" % table['width']
                        profile['hash']['value'] = "0x%x" % 0 # Not used, confirmed by hw
                        profile['opcode']['value'] = "0x%x" % 0 # Policer/Sampler/Counter
                        profile['log2bkts']['value'] = "0x%x" % (0 if num_bkts == 0 else log2(num_bkts))
                        profile['start_addr']['value'] = "0x%x" % capri_get_sram_hw_start_address_from_layout(layout)
                        profile['end_addr']['value'] = "0x%x" % capri_get_sram_hw_end_address_from_layout(layout)
                        if ctable.match_type != match_type.EXACT_IDX and ctable.d_size < ctable.start_key_off:
                            #TODO Entry packing should adhere to thos logic
                            profile['axishift']['value'] = "0x%x" % (ctable.start_key_off / 16)
                        else:
                            profile['axishift']['value'] = "0x%x" % (0)
                    elif mem_type == 'tcam':
                        cap_name = 'cap_pict'
                        num_bkts = 0 if table['depth'] == 0 else (capri_get_depth_from_layout(layout) / table['depth'])
                        profile_name = "%s_csr_cfg_tcam_table_profile[%d]" % (cap_name, profile_id)
                        profile = pic[mem_type][xgress_to_string(direction)][cap_name]['registers'][profile_name]
                        profile['width']['value'] = "0x%x" % table['width']
                        profile['bkts']['value'] = "0x%x" % (capri_get_depth_from_layout(layout) / table['depth'])
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
