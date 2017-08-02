#!/usr/bin/python
#
# Capri-Non-Compiler-Compiler (capri-ncc)
# Parag Bhide (Pensando Systems)

import os
import sys
import pdb
import logging
import copy
import pprint
from collections import OrderedDict
from enum import IntEnum

from p4_hlir.main import HLIR
import p4_hlir.hlir.p4 as p4
import p4_hlir.hlir.table_dependency as table_dependency
import capri_logging
from capri_utils import *
from capri_model import capri_model as capri_model
from capri_output import capri_asm_output_pa as capri_asm_output_pa
from capri_output import capri_output_i2e_meta_header as capri_output_i2e_meta_header
from operator import methodcaller

class capri_phc:
    def __init__(self, d, w, id):
        self.d = d
        self.w = w  # bit width
        self.id = id
        self.fields = OrderedDict() # need fld_name -> (container_start, fld_start_bit, fld_size)

    def get_flds(self, sb, sz):
        # return field name(s) that occupy specified bits within the container
        flds = OrderedDict()
        assert sb >= 0 and sb < 8 and sz > 0 and sz <= 8, pdb.set_trace()
        for f,(cs, fs, w) in self.fields.items():
            ce = cs+w
            if sb >= cs and sb < ce:
                # start bit in this chunk
                fstart = fs + sb - cs
                fw = min(ce-sb, sz)
                flds[f] = (fstart, fw)
            elif sb < cs and (sb+sz) >= ce:
                # entire chunk is covered by requested range
                flds[f] = (fs, w)
            elif (sb+sz) > cs and (sb+sz) <= ce:
                # requested range ends in this chunk
                flds[f] = (fs, sb+sz-cs)
        return flds

    def __repr__(self):
        return '\n%d : %s' % (self.id, self.fields)

class capri_field:
    def __init__(self, p4_fld, d, w=0, name='xxx'):
        self.p4_fld = p4_fld
        self.d = d
        self.hfname = get_hfname(p4_fld) if p4_fld else name
        self.phcs = [] # List of containers used to store this fld
        if p4_fld and not isinstance(p4_fld.width, int):
            # variable len fld, set width to 0, set ohi..
            self.width = 0
            self.is_ohi = True
        else:
            self.width = p4_fld.width if p4_fld != None else w
            self.is_ohi = False
        self.parser_local = False # True => use parser scratch register, not needed by pipeline
        self.is_fld_union = False
        self.is_hdr_union = False
        self.is_hv = False  # field to store header valid bits
        self.is_scratch = False  # no phv allocated for this
        self.is_meta = True if p4_fld and p4_fld.instance and p4_fld.instance.metadata else False
        self.is_internal = True if not p4_fld else False
        self.is_flit_pad = False
        self.is_hv_pad = False
        self.is_union_pad = False
        self.is_union_storage = False   # fields in the last header in the union
        self.is_i2e_meta = False
        self.is_key = False
        self.is_input = False
        self.is_intrinsic = False
        self.is_predicate = False
        self.is_index_key = False       # key for an index table (needs special phv alignment)
        self.allow_relocation = False   # allow moving this field when allocating phvs
        self.is_alignment_pad = False
        self.phv_bit = -1   # bit index in phv
        self.pad = 0        # internal pad added to the field (due to alignment requirements)

    def set_ohi(self):
        #self.width = 0
        self.is_ohi = True

    def reset_ohi(self):
        #self.width = self.p4_fld.width if self.p4_fld != None else 0
        self.is_ohi = False

    def get_p4_hdr(self):
        if self.p4_fld:
            return self.p4_fld.instance
        return None

    def get_field_offset(self):
        if self.p4_fld:
            return self.p4_fld.offset
        else:
            return 0

    def need_phv(self):
        if self.is_ohi or self.is_union():
            return False
        return True

    def is_union(self):
        return self.is_hdr_union or self.is_fld_union

    def flit_meta_order(self):
        if self.is_flit_pad:
            return 100
        elif not self.is_meta or self.is_intrinsic or self.is_predicate or self.is_hv:
            # do not change order of hv, intrinsic,..., and non-meta flds 
            return 0
        if self.is_key and not self.is_input:
            return 10
        elif self.is_key and self.is_input:
            return 20
        elif self.is_input:
            return 30
        # all the rest
        return 40

    def __repr__(self):
        w = self.width if self.width != p4.P4_AUTO_WIDTH else 0
        pstr = '([%d]:%s d=%s w=%d ohi=%s union=%s hv=%s meta=%s internal=%s flit_pad=%s relo=%s)\n' % \
            (self.phv_bit, self.hfname, self.d.name, w, self.is_ohi, \
            self.is_union(), self.is_hv, \
            self.is_meta, self.is_internal, self.is_flit_pad, self.allow_relocation)
        return pstr

class flit_state(Enum):
    OPEN = 0,
    CLOSED = 1

class capri_flit:
    def __init__(self, gress_pa, id, fsize, reserve=0):
        self.gress_pa = gress_pa
        self.id = id
        self.field_order = []
        self.headers = OrderedDict()
        self.flit_used = 0  # bits used
        self.f_state = flit_state.OPEN
        self.reserved = reserve # temp val
        self.flit_reserve = reserve # user provided value (used for reset)
        if reserve:
            assert reserve < fsize/2, "Cannot reserve more than half the flit"
            self.flit_size = fsize - reserve
        else:
            self.flit_size = fsize

    def flit_reset(self):
        self.field_order = []
        self.headers = OrderedDict()
        self.flit_used = 0  # bits used
        self.f_state = flit_state.OPEN
        if self.flit_reserve and not self.reserved:
            # reserved bits were used up - reset
            self.flit_size -= self.flit_reserve
            self.reserved = self.flit_reserve

    def add_cfield(self, cf):
        if cf in self.field_order:
            return True

        phv_flit_sz = self.gress_pa.pa.be.hw_model['phv']['flit_size']

        if self.f_state == flit_state.CLOSED:
            if self.gress_pa.pa.be.args.p4_plus:
                return False
            if not cf.need_phv() or cf.width == 0:
                return False
            # XXX entire header can be placed into hv_pad - but has to meet multiple conditions
            # TBD
            if cf.allow_relocation != True:
                return False
            # try to use flit_pad or hv_pad for this field XXX use alignment pads too ??
            pad_cfs = [pcf for pcf in self.field_order if pcf.is_flit_pad or pcf.is_hv_pad]
            for pcf in pad_cfs:
                # any fields set by parser or used as table key/input are not relocatable
                if pcf.width >= cf.width:
                    pcf_idx = self.field_order.index(pcf)
                    self.field_order.insert(pcf_idx, cf)
                    if (pcf.width - cf.width) == 0:
                        self.field_order.remove(pcf)
                    pcf.width -= cf.width
                    self.gress_pa.logger.debug("%s:%s:%d inserted in closed flit %d at %s remain %d" % \
                        (self.gress_pa.d.name, cf.hfname, cf.width, self.id, pcf.hfname, pcf.width))
                    return True
            return False

        if cf.is_meta or cf.is_internal or cf.is_hv:
            if cf.is_key or cf.is_input or (self.gress_pa.d == xgress.INGRESS and cf.is_i2e_meta):
                # This seems to remove most of the bit extraction violations
                # if a meta fld is used as tbl key or tbl input, just align it on byte
                # boundary and pad its size to be multiple of bytes (will increase phv util)
                # small consecutive fields if used as key/input can be combined ... XXX
                hdr = cf.get_p4_hdr()
                if cf.need_phv() and cf.width and \
                    not self.gress_pa.pa.be.args.p4_plus and \
                    hdr != self.gress_pa.capri_p4_intr_hdr:
                    
                    cf_end = self.flit_used + cf.width

                    if cf.is_index_key and cf.width > 8 and (cf_end % 8):
                        # index table key must be right justified due to key maker restrictions
                        pad = 8 - (cf_end % 8)
                        if (self.flit_used + pad + cf.width) > self.flit_size:
                            return False
                        self.gress_pa.pa.logger.debug("%s:%s:%d pad @%d to right-justify" % \
                            (self.gress_pa.d.name, cf.hfname, pad, self.flit_used))
                        pad_cf = capri_field(None, self.gress_pa.d, pad, '%s_index_align_pad_%d_%d' % \
                            (cf.hfname, self.flit_used, pad))
                        pad_cf.is_alignment_pad = True
                        pad_cf.allow_relocation = False
                        cf.allow_relocation = False
                        self.field_order.append(pad_cf)
                        self.flit_used += pad

                    elif cf.width >= 8 and self.flit_used % 8:
                        if not self._add_byte_alignment_pad(cf):
                            return False
                    else:
                        pass
            if (self.flit_used + cf.width) > self.flit_size:
                # not enough space XXX what if the field does not need phv ???
                return False

            if cf.need_phv() and cf.width:
                assert cf.width > 0
                self.flit_used += cf.width
                assert self.flit_used <= self.flit_size
                if self.reserved and self.flit_used >= phv_flit_sz:
                    # this field is crossing the 512b boundary
                    # add pad - reset the reserved bytes
                    self._add_pad_512b_crossing(cf, phv_flit_sz)

            # add the field to flit even if phv is not needed
            self.field_order.append(cf)
            return True

        hdr = cf.get_p4_hdr()
        # when there is a field union, we have to make sure
        # that the headers that the union flds come from are placed in the same flit
        if hdr and hdr not in self.headers and not cf.is_hdr_union and not is_synthetic_header(hdr):
            hv_per_flit = self.gress_pa.pa.be.hw_model['parser']['flit_hv_bits']
            hdr_list = []
            for hdr_group in self.gress_pa.pa.be.parsers[self.gress_pa.d].hdr_order_groups:
                if hdr in hdr_group:
                    # All headers in hdr order group must fit in the same flit as parser
                    # can loop thru' them in any order
                    hdr_list += hdr_group

            # check fld union, get all the associated headers and associated states
            # Parser-constraint: Allocate everything extracted in one parser state in a flit 
            # This has to be done when the header is first evaluated
            for t_cf in self.gress_pa.get_header_all_cfields(hdr):
                if t_cf in self.gress_pa.fld_unions:
                    for uf in self.gress_pa.fld_unions[t_cf][0]:
                        uhdr = uf.get_p4_hdr()
                        if uhdr.metadata:
                            continue
                        if is_synthetic_header(uhdr):
                            continue
                        if uhdr != hdr and uhdr not in hdr_list:
                            hdr_list.append(uhdr)
            ext_cstates = []
            ext_cstates += self.gress_pa.pa.be.parsers[self.gress_pa.d].get_ext_cstates(hdr)
            for h in hdr_list:
                if is_synthetic_header(h):
                    continue
                ext_cstates += self.gress_pa.pa.be.parsers[self.gress_pa.d].get_ext_cstates(h)

            # ideally we need to do this check for each header in union and in order group - XXX
            # for each state,
            # find other headers extracted (rare) and meta fields
            # make sure we have enough space for all, add headers to self.headers to indicate
            # that it is already accounted for.
            meta_list = []
            meta_fld_size = 0
            flit_used = self.flit_used
            for cs in ext_cstates:
                hdr_list += cs.headers 
                for mf in cs.meta_extracted_fields:
                    if mf not in meta_list:
                        meta_fld_size += mf.width
                        meta_list.append(mf)
                        flit_used += mf.width

            hdr_size = 0
            unique_hdr_list = []
            for sh in hdr_list:
                if sh in unique_hdr_list or sh in self.headers:
                    continue
                unique_hdr_list.append(sh)
                # collect all hdrs in a union
                if sh in self.gress_pa.hdr_unions:
                    for uh in self.gress_pa.hdr_unions[sh][1]:
                        if uh not in unique_hdr_list and uh not in self.headers:
                            unique_hdr_list.append(uh)

            for sh in unique_hdr_list:
                if sh in self.gress_pa.hdr_unions:
                    if sh != self.gress_pa.hdr_unions[sh][2]:
                        continue
                    hdr_size += max([self.gress_pa.get_header_phv_size(uh) \
                                        for uh in self.gress_pa.hdr_unions[sh][1]])
                    # for unions - check entire header.. XXX need to check all flds in all hdrs
                else:
                    # for non-union headers check if entire header can fit
                    hdr_size += self.gress_pa.get_header_phv_size(sh)

            # roundup the meta_fld_size (just incase). It is not perfect but will cover most
            # cases when we add pad at header start
            bits_needed = (hdr_size * 8) + (((meta_fld_size+7)/8) * 8)
            if (self.flit_used + bits_needed) > self.flit_size:
                # not enough space
                self.gress_pa.logger.debug("%s:Flit %d full, not enought space for headers" % \
                    (self.gress_pa.d.name, self.id))
                self.gress_pa.logger.debug("%s:Flit %d avail %d need %d hdrs %s" % \
                    (self.gress_pa.d.name, self.id, (self.flit_size - self.flit_used),
                     bits_needed, unique_hdr_list))
                return False

            for sh in unique_hdr_list:
                self.headers[sh] = cf
                
        # flit has space
        if cf.need_phv() and cf.width:
            assert cf.width > 0, pdb.set_trace()
            cf_end = self.flit_used + cf.width
            if cf.is_index_key and cf.width > 8 and (cf_end % 8):
                self.gress_pa.pa.logger.critical( \
                    "ERROR:Hdr field not right justified to byte boundary in phv" % \
                        (cf.hfname))
                assert 0, pdb.set_trace()

            elif cf.get_field_offset() == 0:
                # make sure header is byte aligned
                align_pad = 0
                if self.flit_used % 8:
                    if not self._add_byte_alignment_pad(cf):
                        return False

            self.flit_used += cf.width
            assert self.flit_used <= self.flit_size, pdb.set_trace()

            if self.reserved and self.flit_used >= phv_flit_sz:
                # this field is crossing the 512b boundary
                # add pad
                self._add_pad_512b_crossing(cf, phv_flit_sz)

        self.field_order.append(cf)
        
        return True

    def _add_byte_alignment_pad(self, cf):
        pad = 8 - (self.flit_used % 8)
        if (self.flit_used + cf.width + pad) > self.flit_size:
            # not enough space
            return False
        pad_cf = capri_field(None, self.gress_pa.d, pad, '%s_byte_align_pad_%d_%d' % \
            (cf.hfname, self.flit_used, pad))
        pad_cf.is_alignment_pad = True
        # XXX need work
        pad_cf.allow_relocation = False
        cf.allow_relocation = False

        if cf.hfname == 'control_metadata.packet_len': pdb.set_trace()
        self.gress_pa.logger.debug("%s:Add pad %s before %s" % \
            (self.gress_pa.d.name, pad_cf.hfname, cf.hfname))
        self.field_order.append(pad_cf)
        self.flit_used += pad
        return True

    def _add_pad_512b_crossing(self, cf, phv_flit_sz):
        # phv flit pad is needed only once since parser flit is twice the size of phv flit
        # reset the reservation
        self.flit_size += self.reserved
        self.reserved = 0
        if (self.flit_used % phv_flit_sz) == 0:
            return # fits perfectly, no pad needed
        if not cf.is_meta and (self.flit_used - cf.width) % 8:
            # do not allow padding for hdr fields that are not on byte boundary
            # this can cause problems for deparser
            return
        pad512b = phv_flit_sz - ((self.flit_used - cf.width) % phv_flit_sz)
        if pad512b == 0:
            return # start on 512b boundary
        pad_cf = capri_field(None, self.gress_pa.d, pad512b, 'flit_%d_phv_flit_pad_%d_%d' % \
                (self.id, self.flit_used, pad512b))
        pad_cf.is_alignment_pad = True
        pad_cf.allow_relocation = False
        self.field_order.append(pad_cf)
        self.gress_pa.logger.debug("%s:Add flit 512b pad %s before %s" % \
                    (self.gress_pa.d.name, pad_cf.hfname, cf.hfname))
        self.flit_used += pad512b


    def validate_flit(self, full=False):
        flit_bits = 0
        for f in self.field_order:
            if f.need_phv():
                assert f.width >= 0, pdb.set_trace()
                flit_bits += f.width
        if full:
            assert flit_bits == self.flit_size, pdb.set_trace()
        assert flit_bits == self.flit_used, pdb.set_trace()

    def flit_close(self):
        self.gress_pa.logger.debug("%s:Close flit[%d], used %d, at %s" % \
            (self.gress_pa.d.name, self.id, self.flit_used, self.field_order[-1].hfname))

        self.flit_size += self.reserved
        self.reserved = 0
        self.validate_flit()
        self.f_state = flit_state.CLOSED
        # pad it up - no need to split pad into 32bits... just one pad for req.d size
        # this is then split into smaller chunk when metadata is relocated
        flit_pad = self.flit_size - self.flit_used
        pad_b = flit_pad % 8
        if flit_pad:
            if pad_b:
                # create two flit pad fields, one small field to align on byte boundary
                # used later when moving metadata
                cf = capri_field(None, self.gress_pa.d, pad_b, '_flit_%d_b_pad__%d' % \
                    (self.id, pad_b))
                cf.is_flit_pad = True
                self.field_order.append(cf)
                flit_pad -= pad_b

            cf = capri_field(None, self.gress_pa.d, flit_pad, '_flit_%d_pad__%d' % \
                (self.id, flit_pad))
            cf.is_flit_pad = True
            self.field_order.append(cf)

        self.flit_used += (flit_pad+pad_b)
        self.validate_flit()
        self.gress_pa.logger.debug("%s:flit[%d] fields %d num headers %d" % \
            (self.gress_pa.d.name, self.id, len(self.field_order), len(self.headers)))

        '''
        # Due to alignment pads added before various fields, it is hard to move the fields
        # around.... disable it (it did not work earlier due to a bug anyways)
        # re-order fields within a flit
        # header fields cannot be changed/reordered
        # move all meta flds towards the end of flit
        # within meta flds, keep all key_fields together and I fields together as -
        # k, (k and I), I
        self.field_order = sorted(self.field_order, key=methodcaller('flit_meta_order'))
        ''' 

class capri_gress_pa:
    def __init__(self, capri_pa, d):
        self.pa = capri_pa
        self.d = d
        self.logger = logging.getLogger('PA')
        self.phcs = []
        self.fields = {}    # All fields unsed in this direction {hfname: capri_field}
        self.field_order = []   # collects fields from parser and table manager
                                # re-arrange the fields until all constraints are met
        self.hdr_unions = {} # {hdr: (direction, [list of headers], dest_header)}
        self.fld_unions = {} # {cf: ([list of cfields], dest_cfield)}
        # use parser flits to compute phv placement
        self.flits = [None for _ in range(self.pa.be.hw_model['parser']['parser_num_flits'])]
        self.hdr_phv_fld = OrderedDict() # {hdr: first_phv_field}
        # i2e metadata
        self.i2e_fields = []
        self.i2e_hdr = None # P4 header object (internally created)
        self.capri_intr_hdr = None
        self.capri_p4_intr_hdr = None

    def initialize(self):
        hw_model = self.pa.be.hw_model
        idx = 0
        for w,num_c in hw_model['phv']['containers'].items():
            for i in range(num_c):
                self.phcs.append(capri_phc(self.d, w, idx))
                idx += 1

        for id, _ in enumerate(self.flits):
            if self.pa.be.args.p4_plus:
                flit_reserve = 0
            else:
                flit_reserve = hw_model['parser']['flit_reserve']
            self.flits[id] = capri_flit(self, id, hw_model['parser']['flit_size'], flit_reserve)

    def print_field_order_info(self, msg='Field-Ordering'):
        ohi_bits = 0
        flit_pad_bits = 0
        union_pad_bits = 0
        meta_bits = 0
        hdr_bits = 0
        hv_bits = 0
        hv_pad_bits = 0
        for f in self.field_order:
            if not f.width:
                continue
            if f.is_ohi:
                ohi_bits += f.width
                continue
            elif f.is_flit_pad:
                flit_pad_bits += f.width
            elif f.is_hv_pad:
                hv_pad_bits += f.width
            elif f.is_union_pad:
                union_pad_bits += f.width
            elif f.is_meta and not f.is_union():
                meta_bits += f.width
            elif f.is_hv:
                hv_bits += f.width
            elif not f.is_union():
                hdr_bits += f.width
            else:
                pass

        assert hv_bits <= self.pa.be.hw_model['parser']['max_hv_bits'], pdb.set_trace()
        # ignore trailing flit pad
        tail_pad = 0
        if len(self.field_order):
            pad_cf = None
            for cf in reversed(self.field_order):
                if not cf.is_flit_pad and not cf.is_alignment_pad:
                    break
                tail_pad += cf.width
                pad_cf = cf

            if self.field_order[-1].is_flit_pad:
                flit_pad_bits -= self.field_order[-1].width
                tail_pad -= self.field_order[-1].width

        total_phv_bits = flit_pad_bits + union_pad_bits + meta_bits + hv_bits + hv_pad_bits + \
                            hdr_bits
        phv_bits = flit_pad_bits + union_pad_bits + meta_bits + hv_bits + hv_pad_bits + hdr_bits - \
                            tail_pad
        self.logger.info('%s: %s' %(self.d.name, msg))
        self.logger.info(\
            "%s:Total Phv bits %d, hdr %d ohi %d meta %d hv %d flit_pad %d u_pad %d hv_pad %d tail %d" %\
            (self.d.name, phv_bits, hdr_bits, ohi_bits, meta_bits, hv_bits, flit_pad_bits,
             union_pad_bits, hv_pad_bits, tail_pad))
        return total_phv_bits

    def validate_field_order(self):
        dup_fields = {}
        for cf in self.field_order:
            if cf in dup_fields:
                self.logger.warning("Duplicate field - %s" % cf.hfname)
                assert(0)
                return False
            else:
                dup_fields[cf] = True
        return False

    def add_fld_union(self, fld_union):
        transitive_union = set()
        non_byte_field = None
        for ufname in fld_union:
            uf = self.get_field(ufname)
            if not uf:
                continue
            if uf.width % 8:
                non_byte_field = uf

            if uf in self.fld_unions:
                transitive_union |= self.fld_unions[uf][0]
            else:
                transitive_union |= set([uf])

        if non_byte_field:
            # if there is a field whose size is not multiple of 8bits, then all flds must be of
            # the same size
            for uf in transitive_union:
                assert uf.width == non_byte_field.width, \
                    "Cannot unionize flds %s(%d) and %s(%d) - must be same size" % \
                    (uf.hfname, uf.width, non_byte_field.hfname, non_byte_field.width)
                if uf in self.fld_unions:
                    # check other fields in the union
                    for of in self.fld_unions[uf][0]:
                        assert of.width == non_byte_field.width, \
                            "Cannot unionize flds %s(%d) and %s(%d) - must be same size" % \
                            (of.hfname, of.width, non_byte_field.hfname, non_byte_field.width)

        for uf in transitive_union:
            if uf not in self.fld_unions:
                self.fld_unions[uf] = (transitive_union, None)
            else:
                self.fld_unions[uf] = (self.fld_unions[uf][0] | transitive_union, None)
            self.logger.debug('%s:Fld Union %s -> %s' % \
                (self.d.name, uf.hfname, [x.hfname for x in self.fld_unions[uf][0]]))

    def insert_table_meta_fields(self):
        gtm = self.pa.be.tables.gress_tm[self.d]
        insert_tbl_meta_flds = OrderedDict() # {cf: (insert_after_cf, cf_index)}
        for tname, tbl in gtm.tables.items():
            if len(tbl.keys) == 0:
                continue
            if len(tbl.meta_fields) == 0:
                continue
            # find last key field that comes from a header, insert meta key fields after
            # all the header fields
            kidx = -1
            insert_cf = None
            for cf,_,_ in tbl.keys:
                if cf.is_meta or cf.is_hv:
                    continue
                cf_idx = self.field_order.index(cf)
                if cf_idx > kidx:
                    kidx = cf_idx
                    insert_cf = cf

            if insert_cf == None:
                continue

            for cf in tbl.meta_fields:
                if cf in self.field_order:
                    continue # placed by parser, cannot move
                if cf.is_intrinsic or cf.is_hv:
                    continue
                if self.d == xgress.EGRESS and cf.is_i2e_meta:
                    continue
                if cf not in insert_tbl_meta_flds:
                    insert_tbl_meta_flds[cf] = (insert_cf, kidx)
                else:
                    if kidx > insert_tbl_meta_flds[cf][1]:
                        insert_tbl_meta_flds[cf] = (insert_cf, kidx)

        insert_tbl_meta_flds = sorted(insert_tbl_meta_flds.items(), key=lambda k: k[1][1],
                                        reverse = True)

        for tcf, ins in insert_tbl_meta_flds:
            cfidx = ins[1]
            i_cf = ins[0]
            assert self.field_order.index(i_cf) == cfidx, pdb.set_trace() # makes it slow
            hdr = i_cf.get_p4_hdr()
            idx = cfidx
            for idx in range(cfidx, len(self.field_order)):
                # find end of current header and insert after that
                if hdr != self.field_order[idx].get_p4_hdr():
                    break

            self.logger.debug("%s:Inserting table meta fld %s into field_order at %s(%d)" % \
                (self.d.name, tcf.hfname, self.field_order[idx].hfname, idx))
            self.field_order.insert(idx, tcf)

    
    def init_field_ordering(self):
        self.field_order = self.pa.be.parsers[self.d].extracted_fields[:]
        if not self.pa.be.args.p4_plus:
            # add meta-fields used by table lookups right after the last key field
            # if the meta fld is used by multiple tables keep pushing it after the last
            # table
            self.insert_table_meta_fields()
        # add other metadata fields that are not referred to by the parser
        # Add fields from a given header consecutively to start with, 
        # these flds can be moved around later
        for h in self.pa.be.h.p4_header_instances.values():
            if not h.metadata:
                continue
            if h == self.capri_p4_intr_hdr:
                continue
            for f in h.fields:
                cf = self.get_field(get_hfname(f))
                if not cf:
                    continue # could be unused in this direction
                if not cf.is_scratch and \
                    not cf.parser_local and cf not in self.field_order:
                    self.field_order.append(cf)
                    if not self.pa.be.args.p4_plus and cf not in self.i2e_fields and \
                        not cf.is_key and not cf.is_input:
                        cf.allow_relocation = True

        var_flds = [f.hfname for f in self.field_order if f.width == p4.P4_AUTO_WIDTH]
        # XXX : no assert yet - ip_options are causing problem due to add/copy_header()
        # assert(len(var_flds) == 0)

        self.validate_field_order()

        self.print_field_order_info(msg="PHV order(Init)")
        #self.logger.debug("Field Order(Pre) %s" % self.field_order)

            
        # clear fld union destinations from earlier round, so we can re-compute
        for k,v in self.fld_unions.items():
            self.fld_unions[k] = (v[0], None)

        # re-init hdr_phv_fld 
        self.hdr_phv_fld = OrderedDict() # {hdr: first_phv_field}
        if not self.pa.be.args.p4_plus:
            # Go thru' header unions and find the first(topological) header for each union
            hdr_list = self.pa.be.parsers[self.d].headers
        else:
            hdr_list = self.pa.be.h.p4_header_instances.values()

        pad = OrderedDict() # must be ordered as pad size is accumulated across entries
        for h,v in self.hdr_unions.items():
            if v[2]:
                # destination is already determined
                continue
            first_idx = len(hdr_list)
            for uh in v[1]:
                if uh not in hdr_list:
                    # this can happen when ingress and egress parsers are different and unions are
                    # specified with direction as xgress
                    continue
                if is_synthetic_header(uh):
                    continue # don't make this a storage - special case
                hidx = hdr_list.index(uh)
                if hidx < first_idx:
                    first_idx = hidx
            self.hdr_unions[h] = (v[0], v[1], hdr_list[first_idx])
            for uh in v[1]:
                # fix other header entries in this union
                self.hdr_unions[uh] = (v[0], v[1], hdr_list[first_idx])
                
        for i,f in enumerate(self.field_order):
            hdr = f.p4_fld.instance
            if hdr not in self.hdr_phv_fld:
                self.hdr_phv_fld[hdr] = f
            if hdr in self.hdr_unions:
                # For header unions actual space is allocated at the first header
                # since first header may not need biggest space, take the max phv need
                # among all the headers in the union and add padding for the difference
                # OHI fields are ignored when computing the phv requirements for an header
                if (self.hdr_unions[hdr][2] != hdr):
                    # not dest header
                    f.is_hdr_union = True
                else:
                    f.is_union_storage = True
                    # dest header - add necessary phv padding
                    max_phv_bytes = max(self.get_header_phv_size(uh) \
                                        for uh in self.hdr_unions[hdr][1])
                    self.logger.debug("Hdr union storage is set at %s [%d] for %s" % \
                        (hdr.name, max_phv_bytes, self.hdr_unions[hdr][1]))
                    hdr_sz = self.get_header_phv_size(hdr)
                    if max_phv_bytes > hdr_sz:
                        # add padding fields - keep updating to get to last field
                        pad[hdr] = (f, max_phv_bytes-hdr_sz)
            elif f in self.fld_unions:
                if self.fld_unions[f][1]:
                    # destination is set
                    self.logger.debug("Dest is set for fld %s -> %s" % \
                        (f.hfname, self.fld_unions[f][1].hfname))
                    continue
                if is_synthetic_header(f.p4_fld.instance):
                    continue
                # set destination field
                for uf in self.fld_unions[f][0]:
                    self.fld_unions[uf] = (self.fld_unions[uf][0], f)
                    if uf != f:
                        uf.is_fld_union = True
                    else:
                        f.is_union_storage = True

                ps = max([uf.width if not uf.is_ohi else 0 for uf in self.fld_unions[f][0]])
                f_phv_width = 0 if f.is_ohi else f.width
                if ps > f_phv_width:
                    pad[f] = (f, ps-f_phv_width)

        for k, (hf, ps) in pad.items():
            if isinstance(k, p4.p4_header_instance):
                # go to the end of the header
                cidx = self.field_order.index(hf)
                f = None
                for i in range(cidx, len(self.field_order)):
                    f = self.field_order[i]
                    if f.is_meta:
                        break
                    if k != f.get_p4_hdr():
                        break

                self.logger.debug('%s:Hdr Pad for %s before %s size %dB' % \
                    (self.d.name, k.name, f, ps))
                # add a single pad field rather than bunch of bytes
                cf = capri_field(None, self.d, ps*8, '_hdr_%s_pad__%d' % (k.name, ps))
                cf.is_union_pad = True
                self.field_order.insert(i, cf)
            else:
                cidx = self.field_order.index(hf)
                self.logger.debug('Fld Pad at %s: %db' % (k.hfname, (ps)))
                # field pad is in bits
                cf = capri_field(None, self.d, ps, '_fld_pad_%d__%d' % (cidx, ps))
                cf.is_union_pad = True
                self.field_order.insert(cidx+1, cf)

        # process synthetic hdr and fld unions
        # syn hdr fields are allowed in fld union with other hdr fields even if the other hdr is
        # in hdr union, so process them separately.. these are really aliases not unions
        for i,f in enumerate(self.field_order):
            if not f.p4_fld:
                continue # internal pad
            hdr = f.p4_fld.instance
            if not is_synthetic_header(hdr):
                continue
            if f not in self.fld_unions:
                continue
            f.is_fld_union = True
            if self.fld_unions[f][1]:
                # destination is already set
                self.logger.debug("Dest is set for syn-fld %s->%s" % \
                    (f.hfname, self.fld_unions[f][1].hfname))
                continue

            # find the union-ed field that will be in phv
            for uf in self.fld_unions[f][0]:
                # if uf is marked as fld union, there will be another fld that is marked storage
                if not uf.is_fld_union:
                    self.fld_unions[f] = (self.fld_unions[uf][0], uf)
                    if not self.fld_unions[uf][1]:
                        self.fld_unions[uf] = (self.fld_unions[uf][0], uf)
                        uf.is_union_storage = True
                            
    def assign_phv(self, start=0, end=-1):
        bit_idx = 0
        alignment_pads = []
        if end == -1:
            end = len(self.field_order)
        for c in range(start, end):
            cf = self.field_order[c]
            if cf.is_meta:
                # handle alignment checks only for metadata used by the parser
                # this should be done when processing the parser fields
                # XXX see if this condition is needed - TBD
                cf.phv_bit = bit_idx
            else:
                cf.phv_bit = bit_idx
            
            if not cf.need_phv():
                continue

            bit_idx += cf.width
            if cf.is_alignment_pad:
                #assert (bit_idx % 8) == 0
                alignment_pads.append((cf.phv_bit, cf.width))

        # fix union-ed hdrs/fields
        # For union-ed headers, find first hdr phv cf of destination hdr
        # assign phv_bit for all phv flds of this union hdr sequentially
        # For field unions, assing destination cf's phv_bit to this fld
        hdr_idx = {}
        for c in range(start, end):
            cf = self.field_order[c]
            if cf.is_ohi or not cf.is_union():
                continue
            hdr = cf.p4_fld.instance
            if cf.is_hdr_union:
                dhdr = self.hdr_unions[hdr][2]
                if dhdr == hdr:
                    continue
                if hdr in hdr_idx:
                    bit_idx = hdr_idx[hdr]
                else:
                    dcf = self.get_header_start_phv_cfield(dhdr)
                    if not dcf:
                        # entire destination header is in OHI, just get phv_bit from
                        # any of its fields
                        phv_flds, ohi_flds = self.get_header_cfields(dhdr)
                        assert len(phv_flds) == 0 and len(ohi_flds) != 0
                        dcf = ohi_flds[0]
                    hdr_idx[hdr] = dcf.phv_bit
                    bit_idx = dcf.phv_bit
                # do not place fields over alignment pad (causes too much I to mpu)
                # when header unions are passed as I-data
                for pad_s, pad_w in alignment_pads:
                    pad_e = pad_s+pad_w
                    if bit_idx >= pad_s and (bit_idx+cf.width) <= pad_e:
                        bit_idx = pad_e
                cf.phv_bit = bit_idx
                hdr_idx[hdr] = bit_idx + cf.width
            else:
                # fld union
                dcf = self.fld_unions[cf][1]
                assert dcf, pdb.set_trace()
                if cf == dcf:
                    continue
                cf.phv_bit = dcf.phv_bit
            self.logger.debug("Relocate %s to %d" % (cf.hfname, cf.phv_bit))

    def update_phc_map(self):
        # update phc mapping dictionary
        for phc in self.phcs:
            phc.fields = OrderedDict() # clear

        for cf in self.field_order:
            if cf.is_ohi or cf.is_union_pad:
                continue
            fw = cf.width
            idx = cf.phv_bit / 8
            f_off = 0
            c_off = cf.phv_bit % 8
            c_part = 8 - c_off
            while fw:
                assert fw > 0, pdb.set_trace()
                cw = fw if (fw < c_part) else c_part
                self.phcs[idx].fields[cf.hfname] = (c_off, f_off, cw)
                cf.phcs.append(idx)
                c_off = 0
                c_part = 8
                idx += 1
                f_off += cw
                fw -= cw

    def clear_phv_assignment(self):
        for cf in self.field_order:
            cf.phv_bit = -1
            
    def get_field(self, hfname):
        # return capri_field object for a given header.field name
        if hfname not in self.fields:
            return None
        return self.fields[hfname]

    def allocate_field(self, p4_fld):
        hfname = get_hfname(p4_fld)
        cfield = self.get_field(hfname)
        if cfield:
            if not cfield.is_intrinsic:
                self.logger.warning("Field %s already present" % hfname)
                assert(0)
        else:
            cfield = capri_field(p4_fld, self.d)
            self.fields[hfname] = cfield
            # check pragmas
            hdr = cfield.get_p4_hdr()
            if len(hdr._parsed_pragmas):
                if 'pa_parser_local' in hdr._parsed_pragmas:
                    cfield.parser_local = True
        return cfield

    def allocate_hv_field(self, hdr_name):
        hfname = hdr_name + '.valid'
        if hfname not in self.fields:
            cf = capri_field(None, self.d, 1, hfname)
            self.fields[hfname] = cf
        else:
            cf = self.fields[hfname]
        cf.is_hv = True
        return cf

    def header_has_ohi(self, hdr):
        for f in hdr.fields:
            cf = self.get_field(get_hfname(f))
            assert(cf)
            if cf.is_ohi:
                return True
            return False

    def get_header_phv_size(self, hdr):
        hphv = 0
        for f in hdr.fields:
            cf = self.get_field(get_hfname(f))
            assert cf, "unknown field %s" % (hdr.name + f.name)
            if not cf.is_ohi:
               hphv += cf.width
        assert((hphv % 8) == 0)
        return (hphv+7)/8
        
    def get_header_start_phv_cfield(self, hdr):
        for f in hdr.fields:
            cf = self.get_field(get_hfname(f))
            assert cf, "unknown field %s" % (hdr.name + f.name)
            if not cf.is_ohi:
                return cf

    def get_header_cfields(self, hdr):
        cfields = []
        ohi_fields = []
        for f in hdr.fields:
            cf = self.get_field(get_hfname(f))
            assert cf, "unknown field %s" % (hdr.name + f.name)
            if cf.is_ohi:
                ohi_fields.append(cf)
            else:
                cfields.append(cf)
        return cfields, ohi_fields

    def get_header_all_cfields(self, hdr):
        hdrfields = []
        for f in hdr.fields:
            cf = self.get_field(get_hfname(f))
            assert cf, "unknown field %s" % (hdr.name + f.name)
            hdrfields.append(cf)
        return hdrfields

    def get_phv_offset(self, cf):
        # count offset of a field within header based on fields that are in phv
        assert(not cf.is_ohi)
        coff = 0
        hdr = cf.p4_fld.instance
        assert(hdr)
        for f in hdr.fields:
            cf2 = self.get_field(get_hfname(f))
            assert(cf2)
            if cf2 == cf:
                return coff
            if cf2.is_ohi:
                continue
            coff += cf2.width
        assert(0) # field not found??
        return 0

    def create_flit_hv_flds(self, flit, name, num_bits):
        for i in range(num_bits):
            hv_field = capri_field(None, self.d, 1, '%s_%d' % (name, i))
            hv_field.is_hv = True
            flit.add_cfield(hv_field)

    def add_intrinsic_meta_and_hv(self, flit, add_hv):
        assert flit.id == 0
        bits_used = 0
        if self.capri_intr_hdr:
            for f in self.capri_intr_hdr.fields:
                cf = self.get_field(get_hfname(f))
                assert cf
                cf.is_ohi = False
                assert cf.is_intrinsic
                flit.add_cfield(cf)

            bits_used += (get_header_size(self.capri_intr_hdr) * 8)

        if self.capri_p4_intr_hdr:
            for f in self.capri_p4_intr_hdr.fields:
                cf = self.get_field(get_hfname(f))
                assert cf
                cf.is_ohi = False
                flit.add_cfield(cf)

            bits_used += (get_header_size(self.capri_p4_intr_hdr) * 8)

        # add cfields used for predication
        pred_cfs = {}
        for c_pred in self.pa.be.tables.gress_tm[self.d].table_predicates.values():
            for cf,_ in c_pred.cfield_vals:
                if cf not in pred_cfs:
                    pred_cfs[cf] = True
                    if cf.is_hv or cf.is_intrinsic:
                        continue
                    # cannot use hdr flds as predicate, as they cannot be placed in 1st 512 bits
                    assert cf.is_meta, pdb.set_trace()
                    cf.is_predicate = True
                    flit.add_cfield(cf)
                    bits_used += cf.width

        if add_hv:
            hv_location = self.pa.be.hw_model['parser']['hv_location']
            max_hv_bits = self.pa.be.hw_model['parser']['max_hv_bits']
            flit_hv_bits = self.pa.be.hw_model['parser']['flit_hv_bits']
            
            tmp_pad = hv_location - bits_used
            dummy_pf = capri_field(None, self.d, tmp_pad, 'pad_before_hv_%d_' % (tmp_pad))
            # XXX set a flag so we can relocate meta flds in here later
            # cannot set flit_pad.. it pushes it to end on sorting
            dummy_pf.is_hv_pad = True
            flit.add_cfield(dummy_pf)
            self.create_flit_hv_flds(flit, 'flit_%d_hv' % (flit.id), max_hv_bits)

    def add_cfield(self, cf, f_id):
        for i in range(0, f_id):
            flit = self.flits[i]
            if flit.add_cfield(cf):
                return True
        return self.flits[f_id].add_cfield(cf)
            
    def create_flits(self, add_hv=False):
        cfid = 0
        flits_used = 0
        max_hv_bits = self.pa.be.hw_model['parser']['max_hv_bits']
        flit_hv_bits = self.pa.be.hw_model['parser']['flit_hv_bits']
        self.clear_phv_assignment()
        for id, flit in enumerate(self.flits):
            if cfid >= len(self.field_order):
                break;
            # add_hv == False indicates p4_plus - completely skip special flit0 programming
            if id == 0 and add_hv:
                self.add_intrinsic_meta_and_hv(flit, add_hv)
                if not self.pa.be.args.p4_plus:
                    # special handling for i2e_meta and hv_bits
                    if self.d == xgress.INGRESS and self.i2e_hdr:
                        flit.headers[self.i2e_hdr] = 1

            # try to place cfield into current or earlier flits if allowed
            while self.add_cfield(self.field_order[cfid], id):
                # cfield was successfully added
                cfid +=1
                if cfid >= len(self.field_order):
                    break;
            flits_used += 1
            flit.flit_close()
            # flit is full

        # check if all fields were added into flits
        if cfid != len(self.field_order):
            self.logger.critical("Not enough space in phv")
            self.print_field_order_info("PHV order(PHV FAILURE)")
            pdb.set_trace()
            assert 0

        # combine lists from flits into a common list
        self.logger.info("%s:Number of flits = %d" % (self.d.name, flits_used))
        self.field_order = []
        for flit in self.flits:
            self.field_order += flit.field_order
            self.logger.debug("Flit %d: %s" % (flit.id, flit.field_order))
            flit.flit_reset()

    def relocate_metadata(self):
        # fill in flit pads with available metadata fields
        flit_pads = [cf for cf in self.field_order if cf.is_flit_pad or cf.is_hv_pad]
        # cannot relocate union-ed metadata or metadata used by parser
        relocate_meta_flds = [cf for cf in self.field_order \
                if cf.allow_relocation and cf.need_phv()]
        #if cf.allow_relocation and cf.need_phv() and not cf.is_alignment_pad]
        flit_pads = sorted(flit_pads, key=lambda k: k.width, reverse=True)
        relocate_meta_flds = sorted(relocate_meta_flds, key=lambda k: k.width, reverse=True)
        for mf in relocate_meta_flds:
            pad_found = None
            mf_index = self.field_order.index(mf)
            for pf in flit_pads:
                pad_index = self.field_order.index(pf)
                if mf.width > pf.width:
                    break
                if mf_index < pad_index:
                    continue
                '''
                if mf.is_key or mf.is_input:
                    if pf.phv_bit % 8:
                        # add pad
                        a_pad = 8 - (pf.phv_bit % 8)
                        if (mf.width + a_pad) > pf.width:
                            continue;
                        mf.width += a_pad   # for now increse the width.. fields will not move now
                '''
                pad_found = pf
                break

            if pad_found == None:
                # metadata is too big, try next one
                continue
            else:
                pad_index = self.field_order.index(pad_found)
                self.logger.debug("%s:Replace pad[%d] %s by [%d]%s" % \
                    (self.d.name, pad_found.width, self.field_order[pad_index].hfname, 
                    self.field_order[mf_index].width, self.field_order[mf_index].hfname))
                pad_found.width -= mf.width
                self.field_order.pop(mf_index)
                # add dummy fld to keep the total size same. All trailing pads will be removed later
                dummy_mf = capri_field(None, self.d, mf.width, '%s_place_holder' % (mf.hfname))
                dummy_mf.is_flit_pad = True
                self.field_order.insert(mf_index, dummy_mf)
                if pad_found.width == 0:
                    self.field_order.pop(pad_index)
                self.field_order.insert(pad_index, mf)
                    
                # re-create and sort the list.. find less expensive way
                flit_pads = [cf for cf in self.field_order if cf.is_flit_pad or cf.is_hv_pad]
                flit_pads = sorted(flit_pads, key=lambda k: k.width, reverse=True)
        # XXX get rid of all the place_holder fields at the tail
        # also all meta-fields at the end can be made contigouous
        
    def get_phv_chunks(self, fl, check_ohi=False):
        # This function expects that the fl is sorted based on phv_bit
        phv_chunks = []
        if len(fl) == 0:
            return phv_chunks

        phv_off = -1
        csize = 0
        for f in fl:
            if csize == 0:
                if check_ohi and f.is_ohi:
                    continue
                # start new chunk
                cstart = f.phv_bit
                phv_off = f.phv_bit + f.width
                csize = f.width
            else:
                if not f.is_ohi or not check_ohi:
                    # this is to handle union fields which will have same offset
                    if f.phv_bit < phv_off:
                        if phv_off < (f.phv_bit + f.width):
                            # overlap with this field ending beyond current offset
                            # add portion to the chunk
                            csize += (f.phv_bit + f.width - phv_off)
                            phv_off = f.phv_bit + f.width
                        else:
                            # duplicate (due to unions)
                            pass
                    elif f.phv_bit == phv_off:
                        # add to chunk
                        csize += f.width
                        phv_off += f.width
                    else:
                        # close current chunk and start a new one
                        phv_chunks.append((cstart, csize))
                        cstart = f.phv_bit
                        csize = f.width
                        phv_off = f.phv_bit + f.width
                else:
                    # close current chunk due to ohi
                    phv_chunks.append((cstart, csize))
                    csize = 0
        if csize:
            phv_chunks.append((cstart, csize))

        return phv_chunks

    def get_hdr_phv_chunks(self, fl, check_ohi=True):
        phv_chunks = []
        if len(fl) == 0:
            return phv_chunks

        phv_off = -1
        csize = 0
        for f in fl:
            if csize == 0:
                if check_ohi and f.is_ohi:
                    continue
                # start new chunk
                cstart = f.phv_bit
                phv_off = f.phv_bit + f.width
                csize = f.width
            else:
                if not f.is_ohi or not check_ohi:
                    if f.phv_bit == phv_off:
                        # add to chunk
                        csize += f.width
                        phv_off += f.width
                    else:
                        # close current chunk because the new field as per its bit-offset
                        # is not contiguous or is spatial placed before current chunk..
                        phv_chunks.append((cstart, csize))
                        cstart = f.phv_bit
                        csize = f.width
                        phv_off = f.phv_bit + f.width
                else:
                    # close current chunk due to ohi
                    phv_chunks.append((cstart, csize))
                    csize = 0
        if csize:
            phv_chunks.append((cstart, csize))

        return phv_chunks

    def replace_hv_field(self, fidx, cf):
        # fidx is not 0 any-more as hv bits are moved inside the flit
        # find the cf
        for i,hv_cf in enumerate(self.field_order):
            if hv_cf.is_hv and hv_cf.phv_bit == fidx:
                break
        self.field_order[i] = cf
        self.logger.debug("%s: Replace hv_bit[%d] = %s" % (self.d.name, fidx, cf.hfname))

    def init_synthetic_headers(self):
        for hdr in self.pa.be.parsers[self.d].headers:
            if not is_synthetic_header(hdr):
                continue
            # check header union first
            if hdr in self.hdr_unions:
                for uh in self.hdr_unions[hdr][1]:
                    if uh == hdr:
                        continue
                    for f in uh.fields:
                        cf = self.get_field(get_hfname(f))
                        assert cf
                        if cf.is_ohi:
                            cf.is_ohi = False
                            self.logger.debug("Set no_ohi on synthetic header fld %s" % cf.hfname)
                    

                continue

            # check field unions
            for f in hdr.fields:
                cf = self.get_field(get_hfname(f))
                assert cf
                if cf.is_ohi:
                    cf.is_ohi = False
                    self.logger.debug("Set no_ohi on synthetic header fld %s" % cf.hfname)

                if cf in self.fld_unions:
                    for uf in self.fld_unions[cf][0]:
                        if uf == cf:
                            continue
                        if uf.is_ohi:
                            uf.is_ohi = False
                            self.logger.debug("Set no_ohi on synthetic header fld (union) %s" % \
                                uf.hfname)

    def phv_dbg_output(self):
        phv_info = OrderedDict()
        fld_map = OrderedDict() # fld_name : phv_bit
        phc_map = OrderedDict() # phc : [{fld_name: (c_start, f_start, f_size)}, ]

        f_order = sorted(self.field_order, key=lambda k: k.phv_bit)
        pstr = ''
        for cf in f_order:
            if cf.is_ohi:
                continue
            pstr += '%d : %s:%d\n' %(cf.phv_bit, cf.hfname, cf.width)
            fld_map[cf.hfname] = OrderedDict()
            fld_map[cf.hfname]['phv_bit'] = str(cf.phv_bit)
            fld_map[cf.hfname]['width'] = str(cf.width)

        self.logger.debug("%s:Field Map:\n%s" % (self.d.name, pstr))
        pstr = ''
        for i,phc in enumerate(self.phcs):
            pstr += '%s' % (phc)
            for hfname,t in phc.fields.items():
                if i not in phc_map:
                    phc_map[i] = OrderedDict()
                if hfname not in phc_map[i]:
                    phc_map[i][hfname] = OrderedDict()
                phc_map[i][hfname]['c_start'] = t[0]
                phc_map[i][hfname]['f_start'] = t[1]
                phc_map[i][hfname]['f_size']  = t[2]

        self.logger.debug("%s:PHV Map:\n%s" % (self.d.name, pstr))
        phv_info['field_map'] = fld_map
        phv_info['phc_map'] = phc_map
        return phv_info

class capri_pa:
    def __init__(self, capri_be):
        self.be = capri_be
        self.logger = logging.getLogger('PA')
        self.hdr_unions = {} # {hdr: (direction, [hdrs_in_union], destination)}
        self.gress_pa = [capri_gress_pa(self, d) for d in xgress]

    def initialize(self):
        for gress_pa in self.gress_pa:
            gress_pa.initialize()

        # create hv_fields for all headers
        for name, hdr in self.be.h.p4_header_instances.items():
            if not hdr.metadata:
                # create a capri_field for hv bit for each header
                for d in xgress:
                    if hdr.virtual:
                        for i in range(hdr.max_index):
                            hname = name + '[%d]' % i
                            cf = self.allocate_hv_field(hname, d)
                            cf.is_hv = True
                    else:
                        cf = self.allocate_hv_field(name, d)
                        cf.is_hv = True

        # Handle intrinsic_metadata headers
        if 'capri_intrinsic' in self.be.h.p4_header_instances:
            self.gress_pa[xgress.INGRESS].capri_intr_hdr = \
                self.be.h.p4_header_instances['capri_intrinsic']
            self.gress_pa[xgress.EGRESS].capri_intr_hdr = \
                self.be.h.p4_header_instances['capri_intrinsic']

            intr_hdr = self.be.h.p4_header_instances['capri_intrinsic']
            for p4f in intr_hdr.fields:
                cf = self.get_field(get_hfname(p4f), xgress.INGRESS)
                if not cf:
                    cf = self.allocate_field(p4f, xgress.INGRESS)
                cf.is_ohi = False
                cf.is_intrinsic = True

                cf = self.get_field(get_hfname(p4f), xgress.EGRESS)
                if not cf:
                    cf = self.allocate_field(p4f, xgress.EGRESS)
                cf.is_ohi = False
                cf.is_intrinsic = True

        if 'capri_p4_intrinsic' in self.be.h.p4_header_instances:
            # create fields for p4_intrinsic
            p4_intr_hdr = self.be.h.p4_header_instances['capri_p4_intrinsic']
            for p4f in p4_intr_hdr.fields:
                cf = self.get_field(get_hfname(p4f), xgress.INGRESS)
                if not cf:
                    cf = self.allocate_field(p4f, xgress.INGRESS)
                cf.is_ohi = False
                cf.is_intrinsic = True

                cf = self.get_field(get_hfname(p4f), xgress.EGRESS)
                if not cf:
                    cf = self.allocate_field(p4f, xgress.EGRESS)
                cf.is_ohi = False
                
            self.gress_pa[xgress.INGRESS].capri_p4_intr_hdr = p4_intr_hdr
            self.gress_pa[xgress.EGRESS].capri_p4_intr_hdr = p4_intr_hdr

        # Add all the p4 fields to respective phv allocators based on direction
        egress_i2e_fields = []
        for f in self.be.h.p4_fields.values():
            if f.offset == None:
                assert f.name == 'valid'
                # capri fields for all header valid bits are already created
                continue
            is_scratch = False
            if f.instance and f.instance.metadata:
                meta_hdr = f.instance
                is_scratch = True if 'scratch_metadata' in meta_hdr._parsed_pragmas else False
                if 'pa_parser_local' in meta_hdr._parsed_pragmas:
                    # create local field in both direction
                    _ = self.gress_pa[xgress.INGRESS].allocate_field(f)
                    _ = self.gress_pa[xgress.EGRESS].allocate_field(f)
                    continue
            if f.instance.name == 'capri_p4_intrinsic':
                continue
            if f.instance.name == 'capri_i2e_metadata':
                # ignore fields specified by the user, unless cmd line option
                if not self.be.args.i2e_user:
                    f.instance.fields.remove(f)
                    f.instance.header_type.layout[f.name]
                    continue
            cfi = None
            cfe = None
            if (f.ingress_read or f.ingress_write):
                cfi = self.gress_pa[xgress.INGRESS].allocate_field(f)
                if cfi and is_scratch:
                    self.logger.debug('INGRESS:%s is scratch field' % (cfi.hfname))
                    cfi.is_scratch = True
                    assert not cfi.parser_local, \
                        "Cannot used parser local field %s in ingress pipeline" % \
                        (cfi.hfname)
            if (f.egress_read or f.egress_write):
                cfe = self.gress_pa[xgress.EGRESS].allocate_field(f)
                if cfe and is_scratch:
                    self.logger.debug('EGRESS:%s is scratch field' % (cfe.hfname))
                    cfe.is_scratch = True
                    assert not cfe.parser_local, \
                        "Cannot used parser local field %s in egress pipeline" % \
                        (cfe.hfname)

            if self.be.args.p4_plus and not cfi and f.instance.name != 'standard_metadata':
                # add all fields to ingress for p4_plus program
                cfi = self.gress_pa[xgress.INGRESS].allocate_field(f)
                if cfi and is_scratch:
                    self.logger.debug('INGRESS:%s is scratch field' % (cfi.hfname))
                    cfi.is_scratch = True
                    assert not cfi.parser_local, \
                        "Cannot used parser local field %s in ingress pipeline" % \
                        (cfi.hfname)

            if not self.be.args.i2e_user and \
                cfi and cfi.is_meta and \
                cfe and cfe.is_meta and \
                not cfe.is_scratch and \
                not cfe.is_intrinsic:
                # i2e metadata field
                self.gress_pa[xgress.INGRESS].i2e_fields.append(cfi)
                cfi.is_i2e_meta = True
                cfe.is_i2e_meta = True
                egress_i2e_fields.append(cfe)

        # update the i2e_metadata headers with all the relevant fields
        if len(egress_i2e_fields):
            if 'capri_i2e_metadata' not in self.be.h.p4_header_instances:
                self.logger.warning("i2e metadata header not found")
            else:
                i2e_hdr = self.be.h.p4_header_instances['capri_i2e_metadata']
                self.gress_pa[xgress.EGRESS].i2e_hdr = i2e_hdr
                i2e_hdr.header_type.length = 0
                i2e_hdr.header_type.layout = OrderedDict()
                f_offset = 0
                i2e_pad = 0 # for debug
                for cf in egress_i2e_fields:
                    fname = get_i2e_fname(cf)
                    w = cf.width
                    if w % 8:
                        # XXX ideally collect fields from the same meta hdr together and
                        # pad to each collection
                        pad = (8-(w%8))
                        w += pad
                        i2e_pad += pad
                        self.logger.warning( \
                            "i2e meta fields must be byte aligned, padding %s by %d" % \
                            (cf.hfname, pad))
                        cf.pad = pad
                        cf.width += pad
                        cf.p4_fld.width += pad
                        cf.get_p4_hdr().header_type.length +=  pad
                    p4f = p4.p4_field(self.be.h, i2e_hdr, fname, w, cf.p4_fld.attributes,
                                        f_offset, None)
                    p4f.egress_read = cf.p4_fld.egress_read
                    p4f.egress_write = cf.p4_fld.egress_write
                    ncf = self.allocate_field(p4f, xgress.EGRESS)
                    self.logger.debug("Create i2e_meta fld %s" % ncf)
                    # make cf and ncf as field union
                    u_set = set([cf.hfname, ncf.hfname])
                    self.gress_pa[xgress.EGRESS].add_fld_union(u_set)
                    ncf.is_i2e_meta = True
                    i2e_hdr.fields.append(p4f)
                    i2e_hdr.header_type.layout[fname] = w
                    i2e_hdr.header_type.length += (w/8) # bytes
                    f_offset += w
                    self.gress_pa[xgress.EGRESS].i2e_fields.append(ncf)

                pstr = "I2E Metadata Header:\n"
                for cf in self.gress_pa[xgress.EGRESS].i2e_fields:
                    pstr += '%s : %d\n' % (cf.hfname, cf.width)
                pstr += "Total size %dB pad %db" % (i2e_hdr.header_type.length, i2e_pad)
                self.logger.debug(pstr)
                capri_output_i2e_meta_header(self.gress_pa[xgress.EGRESS].pa.be, \
                                        self.gress_pa[xgress.EGRESS].i2e_fields, \
                                        i2e_hdr.header_type.length)
        # XXX elif self.be.args.i2e_user:
        # create a dummy i2e_meta header on ingress for deparser to add it to pkt
        # and for ingress parser to set hv bit
        if self.gress_pa[xgress.EGRESS].i2e_hdr:
            # keep an empty header for now... 
            # XXX see if we can simplify processing by populating it
            self.gress_pa[xgress.INGRESS].i2e_hdr = copy.copy(self.gress_pa[xgress.EGRESS].i2e_hdr)
            self.gress_pa[xgress.INGRESS].i2e_hdr.header_type = copy.copy(i2e_hdr.header_type)
            self.gress_pa[xgress.INGRESS].i2e_hdr.header_type.layout = OrderedDict()
            self.gress_pa[xgress.INGRESS].i2e_hdr.fields = []

        # fix the field widths in ingress pipe
        for cf in self.gress_pa[xgress.INGRESS].i2e_fields:
            ecf = self.gress_pa[xgress.EGRESS].get_field(cf.hfname)
            if cf.width != ecf.width:
                assert cf.width < ecf.width
                pad = ecf.width - cf.width
                cf.width = ecf.width
                cf.pad = pad
                cf.p4_fld.width = cf.width

    def initialize_unions(self):
        # Read p4 header info find header uninons specified by pa_header_union
        # and pa_filed_union pragmas
        # TBD - implement an efficient algo to find mutually exclusing segments
        # of the parse graph to find out headers that can be put into a union
        for name, hdr in self.be.h.p4_header_instances.items():
            if len(hdr._parsed_pragmas) == 0:
                continue
            if 'pa_header_union' in hdr._parsed_pragmas:
                assert len(hdr._parsed_pragmas['pa_header_union'])
                union_prg = hdr._parsed_pragmas['pa_header_union']
                for direction in union_prg.keys():
                    hdr_list = []
                    #direction = prg.keys()[0]
                    if direction.upper() == 'XGRESS' or \
                        direction.upper() == xgress.INGRESS.name or \
                        direction.upper() == xgress.EGRESS.name:
                        prg = union_prg[direction]
                        while len(prg):
                            uh = prg.keys()[0]
                            if uh not in self.be.h.p4_header_instances:
                                prg = prg[uh] 
                                continue
                            n_hdr = self.be.h.p4_header_instances[uh]
                            assert(n_hdr)
                            if n_hdr not in hdr_list:
                                hdr_list.append(n_hdr)
                            prg = prg[uh]

                        if hdr not in hdr_list:
                            hdr_list.append(hdr)
                        self.logger.debug("Header union %s->%s" % (name, hdr_list))
                        for uh in hdr_list:
                            if direction.upper() == 'INGRESS' or direction.upper() == 'XGRESS':
                                assert uh not in self.gress_pa[xgress.INGRESS].hdr_unions, \
                                    "%s:Header %s is already in union, can specify only once" % \
                                    (direction.upper(), uh.name)
                                self.gress_pa[xgress.INGRESS].hdr_unions[uh] = \
                                    (direction.upper(), hdr_list, None)
                            if direction.upper() == 'EGRESS' or direction.upper() == 'XGRESS':
                                assert uh not in self.gress_pa[xgress.EGRESS].hdr_unions, \
                                    "%s:Header %s is already in union, can specify only once" % \
                                    (direction.upper(), uh.name)
                                self.gress_pa[xgress.EGRESS].hdr_unions[uh] = \
                                    (direction.upper(), hdr_list, None)
                    else: 
                        self.logger.warning(\
                            "Invalid header_union pragma: Need gress parameter: %s" % \
                            prg)

        for name, hdr in self.be.h.p4_header_instances.items():
            if len(hdr._parsed_pragmas) == 0:
                continue

            if 'pa_field_union' in hdr._parsed_pragmas:
                # if header union is specified, field union has no effect
                # NOTE: fields that are put into an union MUST be byte aligned (both offset and len)
                assert len(hdr._parsed_pragmas['pa_field_union'])
                if 'ingress' in hdr._parsed_pragmas['pa_field_union'] and \
                    (is_synthetic_header(hdr) or \
                        hdr not in self.gress_pa[xgress.INGRESS].hdr_unions):
                    for k,v in hdr._parsed_pragmas['pa_field_union']['ingress'].items():
                        fld_union = set([k]) | set(v.keys())
                        self.gress_pa[xgress.INGRESS].add_fld_union(fld_union)

                if 'egress' in hdr._parsed_pragmas['pa_field_union'] and \
                    (is_synthetic_header(hdr) or \
                        hdr not in self.gress_pa[xgress.EGRESS].hdr_unions):
                    for k,v in hdr._parsed_pragmas['pa_field_union']['egress'].items():
                        fld_union = set([k]) | set(v.keys())
                        self.gress_pa[xgress.EGRESS].add_fld_union(fld_union)

                if 'xgress' in hdr._parsed_pragmas['pa_field_union'] and \
                    (is_synthetic_header(hdr) or \
                        (hdr not in self.gress_pa[xgress.INGRESS].hdr_unions and \
                         hdr not in self.gress_pa[xgress.EGRESS].hdr_unions)):
                    for k,v in hdr._parsed_pragmas['pa_field_union']['xgress'].items():
                        fld_union = set([k]) | set(v.keys())
                        self.gress_pa[xgress.INGRESS].add_fld_union(fld_union)
                        self.gress_pa[xgress.EGRESS].add_fld_union(fld_union)

        for d in xgress:
            self.gress_pa[d].init_synthetic_headers()

    def allocate_field(self, f, d):
        return self.gress_pa[d].allocate_field(f)
        
    def get_field(self, hfname, d):
        # return capri_field object for a given header.field name
        return self.gress_pa[d].get_field(hfname)

    def allocate_hv_field(self, hdr_name, d):
        return self.gress_pa[d].allocate_hv_field(hdr_name)

    def create_flits(self):
        # create flits from the field_order and assign phv bits
        for d in xgress:
            add_hv=True
            if self.be.args.p4_plus:
                add_hv=False
            self.gress_pa[d].create_flits(add_hv)
            total_phv_bits = self.gress_pa[d].print_field_order_info("PHV order(Post Flit)")
            # with new way of filling closed flits with relocatable metadata, an explicit
            # relocate_metadata is not required
            #if not self.be.args.p4_plus:
            #    self.gress_pa[d].relocate_metadata()
            self.gress_pa[d].assign_phv()
            new_phv_bits = self.gress_pa[d].print_field_order_info("PHV order(Post relocate Meta)")
            assert total_phv_bits == new_phv_bits, pdb.set_trace()
            self.logger.debug("%s" % [cf for cf in self.gress_pa[d].field_order \
                if not cf.is_ohi and not cf.is_flit_pad])

    def replace_hv_field(self, fidx, cf, d):
        return self.gress_pa[d].replace_hv_field(fidx, cf)

    def init_field_ordering(self):
        for d in xgress:
            self.gress_pa[d].init_field_ordering()
            #self.gress_pa[d].assign_phv()
            self.gress_pa[d].print_field_order_info("PHV order(Post unions)")
            self.logger.debug("%s" % [cf for cf in self.gress_pa[d].field_order \
                if not cf.is_ohi and not cf.is_flit_pad])

    def get_header_cfields(self, hdr, d):
        return self.gress_pa[d].get_header_cfields(hdr)

    def get_header_all_cfields(self, hdr, d):
        return self.gress_pa[d].get_header_all_cfields(hdr)

    def get_phv_chunks(self, fl, d, check_ohi=False):
        return self.gress_pa[d].get_phv_chunks(fl, check_ohi)

    def get_hdr_phv_chunks(self, fl, d, check_ohi=True):
        return self.gress_pa[d].get_hdr_phv_chunks(fl, check_ohi)

    def capri_asm_output(self):
        for d in xgress:
            capri_asm_output_pa(self.gress_pa[d])

    def phv_dbg_output(self):
        # create a dictionary and write it to a json file
        phv_debug_info = OrderedDict()
        for d in xgress:
            phv_debug_info[d.name] = self.gress_pa[d].phv_dbg_output()
        return phv_debug_info
