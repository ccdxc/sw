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
from capri_logging import ncc_assert as ncc_assert
from capri_utils import *
from capri_model import capri_model as capri_model
from capri_output import capri_asm_output_pa as capri_asm_output_pa
from capri_output import capri_cstruct_output_pa as capri_cstruct_output_pa
from capri_output import capri_output_i2e_meta_header as capri_output_i2e_meta_header
from operator import methodcaller

JUSTIFY_LEFT    = 0
JUSTIFY_RIGHT   = 1

class capri_phc:
    def __init__(self, d, w, id):
        self.d = d
        self.w = w  # bit width
        self.id = id
        self.fields = OrderedDict() # fld_name -> (container_start, fld_start_bit, fld_size)

    def get_flds(self, sb, sz):
        # return field name(s) that occupy specified bits within the container
        flds = OrderedDict()
        ncc_assert(sb >= 0 and sb < 8 and sz > 0 and sz <= 8)
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
        self.is_hdr_union_storage = False
        self.is_fld_union_storage = False
        self.is_i2e_meta = False
        self.is_key = False
        self.is_wide_key = False    # part of a widekey table - key
        self.is_input = False
        self.is_wide_input = False    # part of a widekey table - input
        self.is_intrinsic = False
        self.is_predicate = False
        self.is_parser_extracted = False
        self.is_index_key = False       # key for an index table (needs special phv alignment)
        self.is_parser_end_offset = False
        self.allow_relocation = False   # allow moving this field when allocating phvs
        self.is_alignment_pad = False
        self.phv_bit = -1   # bit index in phv
        self.pad = 0        # internal pad added to the field (due to alignment requirements)
        self.union_pad_size = 0 # valid when is_fld_union_storage == True

    def no_register(self):
        # HACK - revisit
        if 'parser_no_reg' in self.p4_fld.instance._parsed_pragmas or \
           'parser_write_only' in self.p4_fld.instance._parsed_pragmas:
            return True
        else:
            return False

    def is_wide_ki(self):
        return self.is_wide_key or self.is_wide_input

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

    def storage_size(self):
        if not self.is_ohi or self.is_fld_union_storage:
            return self.width + self.union_pad_size
        return 0

    def is_union(self):
        return self.is_hdr_union or self.is_fld_union

    def is_union_storage(self):
        return self.is_hdr_union_storage or self.is_fld_union_storage

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
        pstr = '([%d]:%s d=%s w=%d ohi=%s union=%s u-store=%s hv=%s meta=%s internal=%s flit_pad=%s relo=%s)\n' % \
            (self.phv_bit, self.hfname, self.d.name, w, self.is_ohi, \
            self.is_union(), self.is_union_storage(), self.is_hv, \
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
            ncc_assert(reserve < fsize/2, "Cannot reserve more than half the flit")
            self.flit_size = fsize - reserve
        else:
            self.flit_size = fsize
        # Info used by new phv allocation
        self.fsize = fsize
        self.base_phv_bit = id * fsize
        self.flit_avail = 0
        self.free_chunks = []
        self.flit_chunk_free(self.base_phv_bit, fsize)

    def flit_reset(self):
        self.field_order = []
        self.headers = OrderedDict()
        self.flit_used = 0  # bits used
        self.f_state = flit_state.OPEN
        if self.flit_reserve and not self.reserved:
            # reserved bits were used up - reset
            self.flit_size -= self.flit_reserve
            self.reserved = self.flit_reserve

        # Info needed by new phv allocation
        self.base_phv_bit = self.id * self.fsize
        self.flit_avail = 0
        self.free_chunks = []
        self.flit_chunk_free(self.base_phv_bit, self.fsize)

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
                    not cf.is_intrinsic:
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
                ncc_assert(cf.width > 0)
                self.flit_used += cf.width
                ncc_assert(self.flit_used <= self.flit_size)
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
            hdr_list = [hdr] # handle a case of header extracted in deparse only state
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

        if hdr and hdr not in self.headers and not cf.is_hdr_union and is_synthetic_header(hdr):
            hdr_size = self.gress_pa.get_header_phv_size(hdr)
            bits_needed = (hdr_size * 8)
            if (self.flit_used + bits_needed) > self.flit_size:
                # not enough space
                self.gress_pa.logger.debug("%s:Flit %d full, not enought space for syn-header" % \
                    (self.gress_pa.d.name, self.id))
                self.gress_pa.logger.debug("%s:Flit %d avail %d need %d hdr %s" % \
                    (self.gress_pa.d.name, self.id, (self.flit_size - self.flit_used),
                     bits_needed, hdr))
                return False
            self.headers[hdr] = cf
        # flit has space
        if cf.need_phv() and cf.width:
            ncc_assert(cf.width > 0)
            cf_end = self.flit_used + cf.width
            if cf.is_index_key and cf.width > 8 and (cf_end % 8):
                self.gress_pa.pa.logger.critical( \
                    "ERROR:Hdr field not right justified to byte boundary in phv" % \
                        (cf.hfname))
                ncc_assert(0)

            elif cf.get_field_offset() == 0:
                # make sure header is byte aligned
                align_pad = 0
                if self.flit_used % 8:
                    if not self._add_byte_alignment_pad(cf):
                        return False

            self.flit_used += cf.width
            ncc_assert(self.flit_used <= self.flit_size)

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
                ncc_assert(f.width >= 0)
                flit_bits += f.width
        if full:
            ncc_assert(flit_bits == self.flit_size)
        ncc_assert(flit_bits == self.flit_used)

    def flit_close(self):
        ncc_assert(len(self.field_order) != 0)

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

    def flit_chunk_alloc(self, csize, location=-1, align=0, justify=JUSTIFY_LEFT, fixed_loc=0,
                            cross_512b=True):
        # There will be holes in free space in following conditions -
        # - flit 0 : due to hv bit allocation
        # - (smaller) holes due to right justified allocation of idx_key fields
        # - (smaller) holes due to allocation of meta bit fields
        # For new request, just find a chunk that can satisfy a given request, no need to find
        # a best fit (for now)
        if location >= 0:
            ncc_assert(location >= self.base_phv_bit and location < (self.base_phv_bit+self.fsize))
            ncc_assert(location+csize) <= (self.base_phv_bit+self.fsize)
            floc = location - self.base_phv_bit
        else:
            if location == -2:
                floc = self.free_chunks[-1][0]  # use last free chunk
            else:
                floc = -1

        requested_floc = floc
        for i, (cs, cw) in enumerate(self.free_chunks):
            floc = requested_floc
            if cw < csize:
                continue

            # floc | align | justify | fixed_loc
            #  >=0    0         X       1       : allocate at specified floc
            #  >=0    !0        L       0       : allocate at aligned_start >= floc
            #  >=0    !0        R       0       : allocate at start >= floc and aligned_end
            #  -1      0        X       X       : allocate at any available location
            #  -1     !0        L       X       : allocate at any available aligned_start
            #  -1     !0        R       X       : allocate at any available aligned_end
            #  -2     !0        R       X       : allocate using last free chunk
            if floc >= 0:
                if (cs+cw) < floc:
                    # skip chunks that are entirely before specified floc
                    continue

                if fixed_loc:
                    if cs > floc or (cs+cw) < (floc+csize):
                        # this chunk is either beyond requested floc or not big enough
                        return -1

                    # chunk start at or before floc - check the size
                    if (cs+cw) < (floc+csize):
                        return -1
                else:
                    floc = cs
            else:
                floc = cs

            if cs < floc and (cs+cw) < (floc+csize):
                # find next
                continue;

            # found a chunk that can be used, see if it can still be used after adding alignments
            start_pad = 0;
            if align != 0:
                if justify == JUSTIFY_LEFT:
                    # add start_pad to start the field on aligned boundary
                    if floc % align:
                        start_pad = align - (floc % align)
                if justify == JUSTIFY_RIGHT:
                    # add start_pad to right justify the end bit
                    if (floc+csize) % align:
                        start_pad = align - ((floc+csize) % align)

            a_cs = floc + start_pad
            if not cross_512b:
                if a_cs < 512 and (a_cs+csize) > 512:
                    if justify == JUSTIFY_RIGHT:
                        a_cs = 512+start_pad
                    else:
                        a_cs = 512
            if cs+cw-a_cs < csize:
                # not enough after alignment
                continue
            # allocate from this chunk
            if a_cs == cs:
                # from the beginning of the chunk
                ncc_assert((cw-csize) >= 0)
                if cw-csize == 0:
                    self.free_chunks.pop(i)
                else:
                    self.free_chunks[i] = (cs+csize, cw-csize)
            else:
                ncc_assert((a_cs-cs) >= 0)
                if a_cs+csize == cs+cw:
                    # from the end of the chunk, tail-trim
                    self.free_chunks[i] = (cs, a_cs-cs)
                else:
                    # take from the middle of the chunk
                    self.free_chunks[i] = (cs, a_cs-cs)
                    ncc_assert((cs+cw-a_cs-csize) >= 0)
                    self.free_chunks.insert(i+1, (a_cs+csize, cs+cw-a_cs-csize))

            ncc_assert(csize > 0)
            self.flit_avail -= csize
            return self.base_phv_bit + a_cs # return global phv bit#
        # did not find any sutable free chunk
        return -1

    def flit_chunk_free(self, location, csize):
        ncc_assert(location >= self.base_phv_bit and location < (self.base_phv_bit+self.fsize))
        ncc_assert(location+csize) <= (self.base_phv_bit+self.fsize)

        floc = location - self.base_phv_bit
        if len(self.free_chunks) == 0 or \
            floc > self.free_chunks[-1][0]:
            self.free_chunks.append((floc, csize))
            self.flit_avail += csize
            return

        for i, (cs, cw) in enumerate(self.free_chunks):
            if floc < cs:
                if (floc+csize == cs):
                    # merge with this chunk
                    self.free_chunks[i] = (floc, cw+csize)
                else:
                    ncc_assert((floc+csize < cs), "overlapping free (%d, %d) -> (%d, %d)" % \
                        (floc, csize, cs, cw))
                    if i > 0:
                        # check if this can be merged with previous chunk
                        (prev_cs, prev_cw) = self.free_chunks[i-1]
                        if prev_cs+prev_cw == floc:
                            self.free_chunks[i-1] = (prev_cs, prev_cw+csize)
                        else:
                            self.free_chunks.insert(i, (floc, csize))
                    else:
                        self.free_chunks.insert(i, (floc, csize))
                self.flit_avail += csize
                return
        ncc_assert(0)

HGID = 0
class capri_hfgroup:
    def __init__(self, pa, d):
        self.gress_pa = pa
        self.d = d
        self.hfs = set()
        self.extract_states = set()
        self.branches = []
        self.predecessors = []
        global HGID
        self.name = 'HFG_%d' % HGID
        HGID += 1
    
    def add_branch(self, hfg):
        if hfg == self:
            return
        if hfg not in self.branches:
            self.branches.append(hfg)
        if self not in hfg.predecessors:
            hfg.predecessors.append(self)

    def insert(self, hf, cs):
        # check header and field unions and add those too
        if isinstance(hf, capri_field):
            if hf in self.gress_pa.fld_unions:
                for cf in self.gress_pa.fld_unions[hf][0]:
                    if is_synthetic_header(cf.p4_fld.instance):
                        continue
                    self.hfs.add(cf)
            else:
                self.hfs.add(hf)
        else:
            # header
            if is_synthetic_header(hf):
                return;
            if hf in self.gress_pa.hdr_unions:
                for h in self.gress_pa.hdr_unions[hf][1]:
                    if not is_synthetic_header(h):
                        self.hfs.add(h)
            else:
                # check if any of the fields are unioned, in that case add corresponding header
                # to this group
                self.hfs.add(hf)
                for f in hf.fields:
                    cf = self.gress_pa.get_field(get_hfname(f))
                    if not cf: continue
                    if cf in self.gress_pa.fld_unions:
                        for uf in self.gress_pa.fld_unions[cf][0]:
                            if uf == cf:
                                continue
                            if is_synthetic_header(uf.p4_fld.instance):
                                continue
                            if uf.is_meta:
                                # don't add metadata headers(structs)
                                # add fields directly
                                self.hfs.add(uf)
                            else:
                                self.hfs.add(uf.p4_fld.instance)

            # check if this is a loop header, get all headers in the loop and their unions XXX
            # unions will be process when each loop header is added (from its state)
            hdr_loop_group = self.gress_pa.pa.be.parsers[self.d].get_hdr_loop(hf)
            if hdr_loop_group != None:
                for h in hdr_loop_group:
                    if h == hf:
                        continue
                    if h in self.gress_pa.pa.be.parsers[self.d].deparse_only_hdrs:
                        continue
                    self.hfs.add(h)

        self.extract_states.add(cs)

    def empty(self):
        return True if len(self.hfs) == 0 else False

    def merge(self, hfgroup, update_cs_to_group=False):
        # self.gress_pa.logger.debug("%s: Merge %s -> %s" % \
        #    (self.gress_pa.d.name, hfgroup, self))
        for hf in hfgroup.hfs:
            # no self.add to avoid union processing again
            self.hfs.add(hf)
        for cs in hfgroup.extract_states:
            self.extract_states.add(cs)
        if update_cs_to_group:
            for cs in self.extract_states:
                self.gress_pa.cs_to_group[cs] = self

    def intersects(self, other):
        return True if len(self.hfs & other.hfs) != 0 else False

    def get_storage_size(self):
        sum = 0
        for hf in self.hfs:
            if isinstance(hf, capri_field):
                sum += hf.storage_size()
            else:
                sum += self.gress_pa.get_header_storage_size(hf) * 8
        return sum

    def sorted_hfs(self):
        # return a list of sorted hfs.. use this to process them in a stable
        # order
        sorted_hdrs = [hf for hf in self.hfs if not isinstance(hf, capri_field)]
        sorted_meta = [hf for hf in self.hfs if isinstance(hf, capri_field)]
        sorted_hdrs.sort(key=lambda k: k.name)
        sorted_meta.sort(key=lambda k: k.hfname)
        return sorted_hdrs + sorted_meta

    def __repr__(self):
        return self.name
        return pstr

    def print_all(self):
        pstr = 'HFGroup: %s ' % self.name
        pstr += '['
        for hf in self.sorted_hfs():
            if isinstance(hf, capri_field):
                pstr += hf.hfname
            else:
                pstr += hf.name
            pstr += ','
        pstr += '], States: ['
        for cs in self.extract_states:
            pstr += cs.name + ','
        pstr += ']#'
        return pstr

class capri_gress_pa:
    def __init__(self, capri_pa, d):
        self.pa = capri_pa
        self.d = d
        self.logger = logging.getLogger('PA')
        self.phcs = []
        self.fields = OrderedDict()    # All fields unsed in this direction {hfname: capri_field}
        self.hdr_fld_order = []
        self.field_order = []   # collects fields from parser and table manager
                                # re-arrange the fields until all constraints are met
        self.allocated_hf = OrderedDict()
        self.hdr_unions = OrderedDict() # {hdr: (direction, [list of headers], dest_header)}
        self.fld_unions = OrderedDict() # {cf: ([list of cfields], dest_cfield)}
        # use parser flits to compute phv placement
        self.flits = [None for _ in range(self.pa.be.hw_model['parser']['parser_num_flits'])]
        self.hdr_phv_fld = OrderedDict() # {hdr: first_phv_field}
        # i2e metadata
        self.i2e_fields = []
        self.i2e_hdr = None # P4 header object (internally created)
        self.capri_intr_hdr = None
        self.capri_p4_intr_hdr = None
        self.capri_deparser_len_hdr = None
        self.capri_deparser_pad_hdr = None
        self.capri_gso_csum_hdr = None
        self.parser_end_off_cf = None
        self.hfgroups = []
        self.topo_sorted_hfgroups = []
        self.cs_to_group = OrderedDict() # {cs: capri_group}
        self.hf_to_group = OrderedDict() # {hf: capri_group/None}

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
        meta_bits = 0
        hdr_bits = 0
        hv_bits = 0
        total_phv_bits = 0
        field_order = sorted(self.field_order, key=lambda cf: cf.phv_bit)
        for cf in field_order:
            if cf.phv_bit < 0:
                ohi_bits += cf.width
                # no assert here as this may be called before phv allocation is done or has failed
                #ncc_assert(cf.is_ohi, "%s:Phv is not allocated for %s" % (self.d.name, cf.hfname))
                continue

            if cf.is_union():
                continue
            if cf.is_hv:
                hv_bits += cf.width
            elif cf.is_meta:
                meta_bits += cf.width
            else:
                hdr_bits += cf.width

            total_phv_bits += cf.width
                
        self.logger.info('%s: %s' %(self.d.name, msg))
        self.logger.info(\
            "%s:Total Phv bits %d, hdr %d ohi %d meta %d hv %d " %\
            (self.d.name, total_phv_bits, hdr_bits, ohi_bits, meta_bits, hv_bits))
        return total_phv_bits

    def validate_field_order(self):
        dup_fields = {}
        for cf in self.field_order:
            if cf in dup_fields:
                self.logger.warning("Duplicate field - %s" % cf.hfname)
                ncc_assert(0)
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
                ncc_assert(uf.width == non_byte_field.width, \
                    "Cannot unionize flds %s(%d) and %s(%d) - must be same size" % \
                    (uf.hfname, uf.width, non_byte_field.hfname, non_byte_field.width))
                if uf in self.fld_unions:
                    # check other fields in the union
                    for of in self.fld_unions[uf][0]:
                        ncc_assert(of.width == non_byte_field.width, \
                            "Cannot unionize flds %s(%d) and %s(%d) - must be same size" % \
                            (of.hfname, of.width, non_byte_field.hfname, non_byte_field.width))

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
            # For new phv allocation - only hdr are stroed in hdr_fld list
            # add the table meta fld right after the relevant hdr
            key_hdr = None
            if len(tbl.keys) == 0:
                continue
            if len(tbl.meta_fields) == 0:
                continue
            # find last key field that comes from a header, insert meta key fields after
            # all the header fields
            # if table key is madeup of metadata flds only, this logic does not handle that case
            # XXX in that case we need to find co-located meta flds etc.. - TBD
            kidx = -1
            insert_cf = None
            for cf,_,_ in tbl.keys:
                if cf.is_meta or cf.is_hv:
                    continue
                cf_idx = self.field_order.index(cf)
                if kidx < 0:
                    kidx = cf_idx
                    insert_cf = cf
                    key_hdr = cf.get_p4_hdr()
                elif cf_idx > kidx and tbl.is_hash_table():
                    kidx = cf_idx
                    insert_cf = cf
                    # find last header
                    key_hdr = cf.get_p4_hdr()

            '''
            # XXX this code causes problem for overflow tables in apollo program
            if insert_cf == None:
                # table does not use any header fields as key
                # look for any header in input fields
                for cf in tbl.input_fields:
                    if cf.is_meta or cf.is_hv:
                        continue
                    cf_idx = self.field_order.index(cf)
                    if kidx < 0:
                        kidx = cf_idx
                        insert_cf = cf
                        key_hdr = cf.get_p4_hdr()
                    elif cf_idx > kidx and tbl.is_hash_table():
                        kidx = cf_idx
                        insert_cf = cf
                        # find last header
                        key_hdr = cf.get_p4_hdr()
            '''

            if key_hdr != None:
                hidx = self.hdr_fld_order.index(key_hdr)
            else:
                self.logger.debug("%s:No header flds in the key/input, Adding table meta fields to the end of hdr_fld_order" % (tname))
                hidx = len(self.hdr_fld_order)-1
                # ideally we can continue further but this is # not working with iris - keymaker problem - need to debug XXX
                continue
            
            # keep keys at the end since insertion is reversing the order
            tbl_meta_fields = sorted(tbl.meta_fields, key=lambda k: k.is_key)
            for cf in tbl_meta_fields:
                if cf.is_parser_extracted:
                    continue
                add_to_list = False
                if not cf.is_intrinsic and cf not in self.hdr_fld_order:
                    # do not add to hdr_fld order if this meta fld is unioned with another
                    # (not-syn) header field - this can happen due to i2e header
                    if cf in self.fld_unions:
                        for uf in self.fld_unions[cf][0]:
                            if uf == cf:
                                continue
                            if not uf.is_meta and uf not in self.hdr_fld_order and not is_synthetic_header(uf.p4_fld.instance):
                                # header field is not added yet
                                break
                            else:
                                add_to_list = True
                                break
                    else:
                        add_to_list = True

                if add_to_list == False:
                    continue
                self.hdr_fld_order.insert(hidx+1, cf)
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

        if len(insert_tbl_meta_flds) == 0:
            return

        insert_tbl_meta_flds = sorted(insert_tbl_meta_flds.items(), key=lambda k: k[1][1],
                                        reverse = True)

        for tcf, ins in insert_tbl_meta_flds:
            cfidx = ins[1]
            i_cf = ins[0]
            if i_cf == None:
                self.field_order.append(tcf)
                continue

            ncc_assert(self.field_order.index(i_cf) == cfidx )# makes it slow
            hdr = i_cf.get_p4_hdr()
            idx = cfidx
            for idx in range(cfidx, len(self.field_order)):
                # find end of current header and insert after that
                if hdr != self.field_order[idx].get_p4_hdr():
                    break

            self.logger.debug("%s:Inserting table meta fld %s into field_order at %s(%d)" % \
                (self.d.name, tcf.hfname, self.field_order[idx].hfname, idx))
            self.field_order.insert(idx, tcf)

    def insert_table_meta_fields2(self):
        # place tbl meta fields close to headers used by the table
        gtm = self.pa.be.tables.gress_tm[self.d]
        # collect all the tables that use a given key and headers associated with that table
        # At the end of a table whose headers appear first in the phv header order
        key_to_tbl_hfids = OrderedDict()

        for tname, tbl in gtm.tables.items():
            tbl_key_hdrs = []
            # For new phv allocation - only hdr are stroed in hdr_fld list
            # add the table meta fld right after the relevant hdr
            if len(tbl.keys) == 0:
                continue
            if len(tbl.meta_fields) == 0:
                continue

            tbl_key_hdrs = [cf.get_p4_hdr() for cf,_,_ in tbl.keys if not cf.is_meta and not cf.is_hv]
            if len(tbl_key_hdrs) == 0:
                continue

            placed_meta_keys = []
            for cf,_,_ in tbl.keys:
                if not cf.is_meta or cf.is_hv or cf not in self.hf_to_group:
                    continue
                placed_meta_keys.append(cf)

            tbl_meta_key_indices = [self.find_hf_idx_in_hfgs(hf) for hf in placed_meta_keys]
            tbl_key_hdrs = sorted(tbl_key_hdrs, key = lambda h:self.find_hf_idx_in_hfgs(h))
            tbl_hf_indices = [self.find_hf_idx_in_hfgs(h) for h in tbl_key_hdrs]
            tbl_hf_indices += tbl_meta_key_indices
            tbl_hf_indices = sorted(tbl_hf_indices)

            for cf,_,_ in tbl.keys:
                if not cf.is_meta or cf.is_hv:
                    continue
                # collect all metadata keys of a table
                key_to_tbl_hfids[cf] = OrderedDict()
                key_to_tbl_hfids[cf][tbl] = (tbl_hf_indices[0], tbl_hf_indices[-1])

        #pdb.set_trace()
        for cf,v in key_to_tbl_hfids.items():
            min_hf_idx = None
            max_hf_idx = None
            for tbl,hf_indices in v.items(): 
                if min_hf_idx == None or hf_indices[0] < min_hf_idx:
                    min_hf_idx = hf_indices[0]
                    max_hf_idx = hf_indices[1]

            ncc_assert(min_hf_idx)
            if cf not in self.hf_to_group:
                self.field_order.append(cf)
                self.topo_sorted_hfgroups.insert(max_hf_idx+1, cf)
                self.hf_to_group[cf] = None
                self.logger.debug("%s:Insert table meta-key %s at %d in topo_sorted list" % \
                                    (self.d.name, cf.hfname, max_hf_idx+1))

            # add rest of the table meta I data after that tables last k field
            for tbl,hf_indices in v.items():
                max_hf_idx = hf_indices[1]
                for mf in tbl.meta_fields:
                    if mf.is_key or mf.is_hv:
                        continue
                    if mf not in self.hf_to_group:
                        self.field_order.append(mf)
                        self.topo_sorted_hfgroups.insert(max_hf_idx+1, mf)
                        self.hf_to_group[mf] = None
                        self.logger.debug("%s:Insert table meta-input %s at %d in topo_sorted list" % \
                                     (self.d.name, mf.hfname, max_hf_idx+1))

    def find_hf_idx_in_hfgs(self, hf):
        for i, hfg_list in enumerate(self.topo_sorted_hfgroups):
            if not isinstance(hfg_list, list):
                # deparse only and synthetic headers are added to hfg_list, skip those
                continue
            for hfg in hfg_list:
                if hf in hfg.hfs:
                    return i
        return len(self.topo_sorted_hfgroups)
                
    def add_hfgroup(self, hfgroup):
        # self.logger.debug('%s:Add group %s' % (self.d.name, hfgroup))
        merge_hfg = None
        for hfg in self.hfgroups:
            if hfg == hfgroup:
                continue
            if hfg.intersects(hfgroup):
                # has intersection, merge hfgroup into hfg
                merge_hfg = hfg
                break
        if merge_hfg == None:
            if hfgroup not in self.hfgroups:
                self.hfgroups.append(hfgroup)
            for cs in hfgroup.extract_states:
                self.cs_to_group[cs] = hfgroup
            return
        # merge needs to be recursive E.g.
        # if (x,y) is added to hfgroups (a, x), (b, y)
        # this will give (a,x, y) which should be further merged with (b, y)
        merge_hfg.merge(hfgroup, False)
        # for cs in hfgroup.extract_states:
        #     self.cs_to_group[cs] = hfg

        # remove it from the list and add it back
        self.hfgroups.remove(merge_hfg)

        self.add_hfgroup(merge_hfg)

    def validate_hfgs(self):
        # make sure cs_to_group referes to valid hfg
        for hfg in self.cs_to_group.values():
            ncc_assert(hfg in self.hfgroups, "Invalid HFG")

        # make sure all headers and meta fields are covered - XXX

    def create_hfgroup_list(self):
        self.logger.debug("%s:Create HFG graph" % (self.d.name))
        self.logger.debug("%s:HFGs = %d" % (self.d.name, len(self.hfgroups)))
        for hfg in self.hfgroups:
            self.logger.debug("%s: %s" % (self.d.name, hfg.print_all()))
        parser = self.pa.be.parsers[self.d]
        root = capri_hfgroup(self, self.d)
        # create cross dependent groups (from Sorrento)
        # this is needed as loops can be created by setting same metadata field
        # on different path in different order
        precedence_map = {} # tuple(G1, G2)
        cross_dependence_map = {} # {G1: set(G2, G5..)}
        for cs_path in parser.path_states:
            for i,cs in enumerate(cs_path):
                if cs not in self.cs_to_group:
                    continue
                g1 = self.cs_to_group[cs]
                for j in range(i+1, len(cs_path)):
                    cs2 = cs_path[j]
                    if cs2 not in self.cs_to_group:
                        continue
                    g2 = self.cs_to_group[cs2]
                    if g1 == g2:
                        continue
                    precedence_map[(g1, g2)] = True
                    if (g2,g1) in precedence_map:
                        if g1 not in cross_dependence_map:
                            cross_dependence_map[g1] = set([g1, g2])
                        else:
                            cross_dependence_map[g1].add(g2)

        # create cross dependency sets to merge
        remove_list = {}
        for i,g1 in enumerate(cross_dependence_map.keys()):
            if g1 in remove_list:
                continue
            s1 = cross_dependence_map[g1]
            for j in range(i+1, len(cross_dependence_map.keys())):
                g2 = cross_dependence_map.keys()[j]
                if g2 in remove_list:
                    continue
                s2 = cross_dependence_map[g2]
                if len(s1 & s2) != 0:
                    remove_list[g2] = 1
                    s1 |= s2

        # merge cross dependent group (set)
        for g, cross_set in cross_dependence_map.items():
            if g in remove_list:
                continue
            self.logger.debug('%s:Merge cross set %s' % (self.d.name, [s.name for s in cross_set]))
            g0 = None
            cross_set_list = list(cross_set)
            cross_set_list.sort(key=lambda k:k.name)
            for gs in cross_set_list:
                if not g0:
                    g0 = gs
                else:
                    g0.merge(gs, True)
                    self.hfgroups.remove(gs)

        # per path merging
        for cs_path in parser.path_states:
            path_hfgs = []
            for cs in cs_path:
                if cs not in self.cs_to_group:
                    # no phv writes in this state
                    continue
                hfg = self.cs_to_group[cs]
                if hfg in path_hfgs:
                    idx = path_hfgs.index(hfg)
                    # merge all groups between two occurances of the same hfg, E.g.
                    # G1 G2 G3 G1 => merge G1-G4
                    for i in range(idx+1, len(path_hfgs)):
                        self.logger.debug('%s:Merge on path %s <- %s' % \
                            (self.d.name, hfg, path_hfgs[i]))
                        hfg.merge(path_hfgs[i], True)
                        self.hfgroups.remove(path_hfgs[i])
                    remove_until = len(path_hfgs)
                    for i in range(idx+1, remove_until):
                        path_hfgs.pop(idx+1)
                else:
                    path_hfgs.append(hfg)

        ## Avoid handling of branches during merge (TBD) slow
        for cs_path in parser.path_states:
            current_node = root
            for cs in cs_path:
                if cs not in self.cs_to_group:
                    # no phv writes in this state
                    continue
                hfg = self.cs_to_group[cs]
                current_node.add_branch(hfg)
                current_node = hfg

        # rebuild hfgrups after all merging is done
        num_hfgs = len(self.hfgroups)
        self.hfgroups = []
        for hfg in self.cs_to_group.values():
            if hfg not in self.hfgroups:
                self.hfgroups.append(hfg)

        # create map for all preceeding nodes for each node
        pred_hfgs = OrderedDict()
        root_nodes = [root]
        pred_hfgs[root] = set([])
        for hfg in self.hfgroups:
            pred_hfgs[hfg] = set(hfg.predecessors)

        self.logger.debug("%s:Pred_HFGs: %s" % (self.d.name, pred_hfgs))
        #pdb.set_trace()

        def remove_preds(pred_hfgs, rm_nodes):
            new_root_nodes = []
            for node in rm_nodes:
                for hfg, pred in pred_hfgs.items():
                    if hfg in rm_nodes:
                        continue
                    pred.discard(node)
                    if len(pred) == 0 and hfg not in new_root_nodes:
                        new_root_nodes.append(hfg)

            for node in rm_nodes:
                if node not in pred_hfgs: pdb.set_trace()
                pred_hfgs.pop(node)

            return new_root_nodes

        #pdb.set_trace()
        sorted_list = []

        while len(root_nodes):
            sorted_list.append(root_nodes)
            root_nodes = remove_preds(pred_hfgs, root_nodes)
            root_nodes.sort(key=lambda k:k.name)
            
        ncc_assert(len(pred_hfgs) == 0, "Invalid HFG graph")
        self.logger.debug("%s:Sorted HFGs" % (self.d.name))
        for hfgs in sorted_list:
            self.logger.debug("%s" % (hfgs))

        #pdb.set_trace()    
        self.topo_sorted_hfgroups = sorted_list

    def init_field_ordering2(self):
        # create groups from each state extraction
        # ignore deparse-only states and synthetic headers
        parser = self.pa.be.parsers[self.d]
        cs_to_hfgroup = {}
        for cs in parser.states:
            if cs.deparse_only:
                # should take care of syn headers as well, since they cannot be extracted
                # in normal states
                continue
            hfgroup = capri_hfgroup(self, self.d)
            for h in cs.headers:
                hfgroup.insert(h, cs) # check header and field unions and add those too? or do it when add ??
            for cf in cs.meta_extracted_fields:
                hfgroup.insert(cf, cs)

            if hfgroup.empty():
                continue

            # merge with another group if there is intersection
            # update states in the group and state_to_group mapping
            self.add_hfgroup(hfgroup)

        # create a (topo sort like) list of hfgroups.
        # Each element in the list is a set of HFGs which do not preceed or succeed one another
        # but HFGs in a given element have predecessors in previous elements
        self.create_hfgroup_list()

        for hfgs in self.topo_sorted_hfgroups:
            for hfg in hfgs:
                #hflist = sorted(hfg.hfs, key=lambda k: k.hfname if isinstance(k, capri_field) else k.name)
                for hf in hfg.hfs:
                    self.hf_to_group[hf] = hfg

        # add synthetic and deparse-only headers
        if not self.pa.be.args.split_deparse_only_headers:
            for hdr in self.pa.be.parsers[self.d].deparse_only_hdrs:
                # add deparse only hdrs at the end as they are not bound by flit order
                if hdr in self.hf_to_group:
                    # this can happen due to header unions
                    continue
                # self.hdr_fld_order.append(hdr)
                self.topo_sorted_hfgroups.append(hdr)
                self.hf_to_group[hdr] = None # not in any group

        # fix header unions (needed for existing code) since all unioned headers are added in one 
        # group, any of then can be marked as storage location
        for h,v in self.hdr_unions.items():
            union_storage = None
            sorted_uhs = sorted(v[1], key=lambda k: k.name)
            for uh in sorted_uhs:
                if is_synthetic_header(uh):
                    continue # don't make this a storage - special case
                else:
                    union_storage = uh
                    break
            ncc_assert(union_storage != None)
            for uh in sorted_uhs:
                # fix all header entries in this union
                self.hdr_unions[uh] = (v[0], sorted_uhs, union_storage)
                for f in uh.fields:
                    uf = self.get_field(get_hfname(f))
                    if uh == union_storage:
                        uf.is_hdr_union_storage = True
                    else:
                        uf.is_hdr_union = True

        # clear fld union destinations from earlier round, so we can re-compute
        for k,v in self.fld_unions.items():
            self.fld_unions[k] = (v[0], None)

        for cf in self.fld_unions:
            hdr = cf.p4_fld.instance
            fld_union_storage = None
            # try to get headers, followed meta followed by deparse_only headers
            fld_union_list = self.fld_unions[cf][0]
            # get stable sorting by sorting by names before applying other criteria
            fld_union_list = sorted(fld_union_list, key=lambda xf: xf.p4_fld.instance.name)
            fld_union_list = sorted(fld_union_list, key=lambda xf: 100 if xf.is_meta else \
                        50 if xf.p4_fld.instance in self.pa.be.parsers[self.d].deparse_only_hdrs \
                        else 0)
            for uf in fld_union_list:
                if is_synthetic_header(uf.p4_fld.instance):
                    continue
                elif uf.is_ohi:
                    continue
                else:
                    fld_union_storage = uf
                    break
            if fld_union_storage == None:
                # everything in Ohi
                continue
            # set destination field
            for uf in self.fld_unions[cf][0]:
                self.fld_unions[uf] = (self.fld_unions[uf][0], fld_union_storage)
                if uf != fld_union_storage:
                    ncc_assert(uf.is_fld_union_storage == False, "Invalid fld union storage flag")
                    uf.is_fld_union = True
                else:
                    ncc_assert(uf.is_fld_union == False, "Invalid fld union flag")
                    uf.is_fld_union_storage = True

            self.logger.debug("%s:union %s => %s" % \
                            (self.d.name, self.fld_unions[cf][0], fld_union_storage))
            ps = max([uf.width if not uf.is_ohi else 0 for uf in self.fld_unions[cf][0]])
            f_phv_width = 0 if fld_union_storage.is_ohi else fld_union_storage.width
            if ps > f_phv_width:
                fld_union_storage.union_pad_size = ps-f_phv_width

        # add other metadata fields that are not referenced by parser
        self.insert_table_meta_fields2()

        # add other metadata fields that are not referred to by the parser
        # Add fields from a given header consecutively to start with,
        # these flds can be moved around later
        for h in self.pa.be.h.p4_header_instances.values():
            if not h.metadata:
                continue
            if h == self.capri_p4_intr_hdr:
                continue

            if h in self.hf_to_group or is_parser_local_header(h) or is_scratch_header(h) or \
                h.name == 'standard_metadata':
                continue

            for f in h.fields:
                cf = self.get_field(get_hfname(f))
                if not cf:
                    continue # could be unused in this direction
                if cf not in self.field_order:
                    self.field_order.append(cf)
                if h not in self.hf_to_group and is_atomic_header(h):
                    # self.hdr_fld_order.append(h)
                    self.topo_sorted_hfgroups.append(h)
                    self.hf_to_group[h] = None # not in any group
                    continue

                if cf not in self.hf_to_group:
                    self.topo_sorted_hfgroups.append(cf)
                    self.hf_to_group[cf] = None # not in any group

                    if not cf.is_wide_ki() and (self.d == xgress.INGRESS or cf not in self.i2e_fields):
                        # Always allow relocation
                        cf.allow_relocation = True

    def init_field_ordering(self):
        self.field_order = self.pa.be.parsers[self.d].extracted_fields[:]
        if not self.pa.be.args.old_phv_allocation and not self.pa.be.args.p4_plus:
            self.init_field_ordering2()
            return
        # follow the parser field order and collect headers
        # collecting headers from parser has problem since synthetic and deparse-only headers
        # can appear much early on in topo order
        for cf in self.field_order:
            if cf.is_meta:
                if cf not in self.hdr_fld_order:
                    self.hdr_fld_order.append(cf)
            else:
                hdr = cf.get_p4_hdr()
                # add deparse only hdrs at the end as they are not bound by flit order
                if hdr not in self.hdr_fld_order and \
                    hdr not in self.pa.be.parsers[self.d].deparse_only_hdrs:
                    self.hdr_fld_order.append(hdr)

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

            if h in self.hdr_fld_order or is_parser_local_header(h) or is_scratch_header(h) or \
                h.name == 'standard_metadata':
                continue

            # for P4plus, hdr used will remove all the field from egress direction
            hdr_used = False
            for f in h.fields:
                cf = self.get_field(get_hfname(f))
                if not cf:
                    continue # could be unused in this direction
                hdr_used = True
                if cf not in self.field_order:
                    self.field_order.append(cf)
                    if cf not in self.hdr_fld_order and \
                        not self.pa.be.args.p4_plus and not is_atomic_header(h) and \
                        not cf.is_wide_ki():
                        self.hdr_fld_order.append(cf)

                    if cf.is_wide_ki() and not self.pa.be.args.p4_plus:
                        # ignore any cfs that are used for widekey lookup
                        hdr_used = False

                    # Not relocating (i.e. allocating phv whereever possible) helps to localize
                    # a key maker for a given table but wastes phv space.. 
                    # current programs seems to compile w/ relocation so make it cmd line option
                    # for future
                    if self.pa.be.args.avoid_ki_relocation:
                        if (self.d == xgress.INGRESS or cf not in self.i2e_fields) and \
                            not cf.is_key and not cf.is_input:
                            cf.allow_relocation = True
                    else:
                        if (self.d == xgress.INGRESS or cf not in self.i2e_fields):
                            # Always allow relocation ?? Or decide it based on num-tables in a stage
                            cf.allow_relocation = True

            if hdr_used and (self.pa.be.args.p4_plus or is_atomic_header(h)):
                self.hdr_fld_order.append(h)

        var_flds = [f.hfname for f in self.field_order if f.width == p4.P4_AUTO_WIDTH]
        # XXX : no ncc_assert(yet - ip_options are causing problem due to add/copy_header())
        # ncc_assert(len(var_flds) == 0)

        for hdr in self.pa.be.parsers[self.d].deparse_only_hdrs:
            # add deparse only hdrs at the end as they are not bound by flit order
            if hdr not in self.hdr_fld_order:
                self.hdr_fld_order.append(hdr)

        self.validate_field_order()

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

            ncc_assert(first_idx < len(hdr_list), "%s:Union-ed headers %s are not used" % (self.d.name, v[1]) )
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
                    f.is_hdr_union_storage = True
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
                    self.logger.debug("%s:Dest is set for fld %s -> %s" % \
                        (self.d.name, f.hfname, self.fld_unions[f][1].hfname))
                    continue
                if is_synthetic_header(f.p4_fld.instance):
                    continue
                # set destination field
                for uf in self.fld_unions[f][0]:
                    self.fld_unions[uf] = (self.fld_unions[uf][0], f)
                    if uf != f:
                        uf.is_fld_union = True
                    else:
                        f.is_fld_union_storage = True

                ps = max([uf.width if not uf.is_ohi else 0 for uf in self.fld_unions[f][0]])

                f_phv_width = 0 if f.is_ohi else f.width
                if ps > f_phv_width:
                    pad[f] = (f, ps-f_phv_width)
                    f.union_pad_size = ps-f_phv_width

        if  self.pa.be.args.old_phv_allocation:
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
                uhdr = uf.p4_fld.instance
                if is_synthetic_header(uhdr):
                    continue
                if uf == f:
                    continue
                # if uf is marked as fld union, there will be another fld that is marked storage
                if not uf.is_fld_union:
                    self.fld_unions[f] = (self.fld_unions[uf][0], uf)
                    if not self.fld_unions[uf][1]:
                        self.fld_unions[uf] = (self.fld_unions[uf][0], uf)
                        uf.is_fld_union_storage = True

    def assign_phv(self, start=0, end=-1):
        # In new phv allocation - this function is not needed
        if not self.pa.be.args.old_phv_allocation:
            return
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
                #ncc_assert((bit_idx % 8) == 0)
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
                        ncc_assert(len(phv_flds) == 0 and len(ohi_flds) != 0)
                        dcf = ohi_flds[0]
                    hdr_idx[hdr] = dcf.phv_bit
                    bit_idx = dcf.phv_bit
                # do not place fields over alignment pad (causes too much I to mpu)
                # when header unions are passed as I-data
                for pad_s, pad_w in alignment_pads:
                    pad_e = pad_s+pad_w
                    # Need to check both ends of cf, if either falls within the pad
                    if (bit_idx >= pad_s and bit_idx <= pad_e) or \
                       ((bit_idx+cf.width) >= pad_s and (bit_idx+cf.width) <= pad_e):
                        self.logger.debug("Avoid Relocate %s on to pad at %d" % (cf.hfname, pad_s))
                        bit_idx = pad_e
                cf.phv_bit = bit_idx
                hdr_idx[hdr] = bit_idx + cf.width
            else:
                # fld union
                dcf = self.fld_unions[cf][1]
                ncc_assert(dcf)
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
                ncc_assert(fw > 0)
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
                ncc_assert(0)
        else:
            cfield = capri_field(p4_fld, self.d)
            self.fields[hfname] = cfield
            # check pragmas
            hdr = cfield.get_p4_hdr()
            if is_parser_local_header(hdr):
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

    def allocate_hv_truncate_field(self, hdr_name):
        hfname = hdr_name + '.trunc'
        if hfname not in self.fields:
            cf = capri_field(None, self.d, 1, hfname)
            self.fields[hfname] = cf
        else:
            cf = self.fields[hfname]
        cf.is_hv = True
        return cf

    def allocate_hv_padhdr_len_field(self, hdr_name):
        hfname = hdr_name + '.pad'
        if hfname not in self.fields:
            cf = capri_field(None, self.d, 1, hfname)
            self.fields[hfname] = cf
        else:
            cf = self.fields[hfname]
        cf.is_hv = True
        return cf

    def allocate_hv_payload_len_field(self, hdr_name):
        hfname = hdr_name + '.payload'
        if hfname not in self.fields:
            cf = capri_field(None, self.d, 1, hfname)
            self.fields[hfname] = cf
        else:
            cf = self.fields[hfname]
        cf.is_hv = True
        return cf

    def allocate_csum_hv_field(self, hdr_name):
        csum_hv_names = []
        if self.pa.be.checksum.IsHdrInCsumCompute(hdr_name, self.d):
            hfname = hdr_name + '.csum'
            csum_hv_names.append(hfname)
        if not self.pa.be.checksum.IsHdrInL2CompleteCsumCompute(hdr_name, self.d) and \
           not self.pa.be.checksum.IsHdrInPayLoadCsumCompute(hdr_name, self.d) and \
           not self.pa.be.checksum.IsHdrInOptionCsumCompute(hdr_name, self.d) and \
           not self.pa.be.checksum.IsPayloadCsumComputeWithNoPhdr(hdr_name, self.d):
            hfname = hdr_name + '.tcp_csum'
            csum_hv_names.append(hfname)
            hfname = hdr_name + '.udp_csum'
            csum_hv_names.append(hfname)
            if self.pa.be.checksum.L3HdrAsPseudoHdr(hdr_name, self.d) == 3:
                hfname = hdr_name + '.icmp_csum'
                csum_hv_names.append(hfname)
        if self.pa.be.checksum.IsL2HdrInL2CompleteCsumCompute(hdr_name, self.d) \
           or self.pa.be.checksum.IsHdrInL2CompleteCsumCompute(hdr_name, self.d):
            hfname = hdr_name + '.l2csum'
            csum_hv_names.append(hfname)

        for hfname in csum_hv_names:
            if hfname not in self.fields:
                cf = capri_field(None, self.d, 1, hfname)
                self.fields[hfname] = cf
            else:
                cf = self.fields[hfname]
            cf.is_hv = True

    def allocate_icrc_hv_field(self, hdr_name):
        hfname = hdr_name + '.icrc'
        if hfname not in self.fields:
            cf = capri_field(None, self.d, 1, hfname)
            self.fields[hfname] = cf
        else:
            cf = self.fields[hfname]
        cf.is_hv = True

    def allocate_gso_csum_hv_field(self, hdr_name):
        hfname = hdr_name + '.gso'
        if hfname not in self.fields:
            cf = capri_field(None, self.d, 1, hfname)
            self.fields[hfname] = cf
        else:
            cf = self.fields[hfname]
        cf.is_hv = True

    def header_has_ohi(self, hdr):
        for f in hdr.fields:
            cf = self.get_field(get_hfname(f))
            ncc_assert(cf)
            if cf.is_ohi:
                return True
            return False

    def get_header_phv_size(self, hdr):
        hphv = 0
        for f in hdr.fields:
            cf = self.get_field(get_hfname(f))
            ncc_assert(cf, "unknown field %s" % (hdr.name + f.name))
            if is_synthetic_header(hdr):
                ncc_assert(not cf.is_ohi )# cannot have ohi flds in syn header
                if not cf.is_union():
                    hphv += cf.width
            elif not cf.is_ohi:
               hphv += cf.width
            else:
                pass
        ncc_assert((hphv % 8) == 0)
        return (hphv+7)/8

    def get_header_storage_size(self, hdr):
        # Count only non-unioned and non-ohi fields
        # for header union (not storage, it will return 0, but for union-storage
        # it will return max of all headers union-ed
        hphv = 0
        if hdr in self.hdr_unions:
            if hdr != self.hdr_unions[hdr][2]:
                return 0    # not storage for union
            return max([self.get_header_phv_size(uh) \
                                        for uh in self.hdr_unions[hdr][1]])
        for f in hdr.fields:
            cf = self.get_field(get_hfname(f))
            ncc_assert(cf, "unknown field %s" % get_hfname(f))
            if cf.is_union() or cf.is_ohi:
                continue
            hphv += cf.width + cf.union_pad_size

        ncc_assert((hphv % 8) == 0)
        return (hphv+7)/8

    def get_header_start_phv_cfield(self, hdr):
        for f in hdr.fields:
            cf = self.get_field(get_hfname(f))
            ncc_assert(cf, "unknown field %s" % (hdr.name + f.name))
            if not cf.is_ohi:
                return cf

    def get_header_cfields(self, hdr):
        cfields = []
        ohi_fields = []
        for f in hdr.fields:
            cf = self.get_field(get_hfname(f))
            ncc_assert(cf, "unknown field %s" % (hdr.name + f.name))
            if cf.is_ohi:
                ohi_fields.append(cf)
            else:
                cfields.append(cf)
        return cfields, ohi_fields

    def get_header_all_cfields(self, hdr):
        hdrfields = []
        for f in hdr.fields:
            cf = self.get_field(get_hfname(f))
            ncc_assert(cf, "unknown field %s" % (hdr.name + f.name))
            hdrfields.append(cf)
        return hdrfields

    def get_phv_offset(self, cf):
        # count offset of a field within header based on fields that are in phv
        ncc_assert(not cf.is_ohi)
        coff = 0
        hdr = cf.p4_fld.instance
        ncc_assert(hdr)
        for f in hdr.fields:
            cf2 = self.get_field(get_hfname(f))
            ncc_assert(cf2)
            if cf2 == cf:
                return coff
            if cf2.is_ohi:
                continue
            coff += cf2.width
        ncc_assert((0) )# field not found??
        return 0

    def add_cfield(self, cf, f_id):
        for i in range(0, f_id):
            flit = self.flits[i]
            if flit.add_cfield(cf):
                return True
        return self.flits[f_id].add_cfield(cf)


    def hdr_get_byte_aligned_fld_chunks(self, hdr):
        # return {cf : num_contiguous_phv_bytes}, do not break flds
        hdr_byte_flds = OrderedDict()
        last_cf = None
        hoff = 0
        clen = 0

        for f in hdr.fields:
            cf = self.get_field(get_hfname(f))
            ncc_assert(cf)
            if cf.is_ohi:
                continue

            if f.offset % 8:
                ncc_assert(last_cf)
                ncc_assert(hoff == f.offset)
                hoff += cf.storage_size()
                clen += cf.storage_size()
            else:
                if last_cf:
                    hdr_byte_flds[last_cf] = clen
                clen = cf.storage_size()
                last_cf = cf
                hoff = f.offset + clen

        if clen:
            hdr_byte_flds[last_cf] = clen

        return hdr_byte_flds

    def clear_new_flit_phv_assignment(self, flit, allocated_chunks, new_allocated_hfs):
        for ac in allocated_chunks:
            flit.flit_chunk_free(ac[0], ac[1])

        for n_hf in new_allocated_hfs:
            del self.allocated_hf[n_hf]
            if isinstance(n_hf, capri_field):
                n_hf.phv_bit = -1
            else:
                self.hdr_free_phv(n_hf)

    def hdr_free_phv(self, hdr):
        for f in hdr.fields:
            cf = self.get_field(get_hfname(f))
            ncc_assert(cf)
            cf.phv_bit = -1
            # remove phv bits from fld union-ed flds, these are not added to new_allocated_hfs
            if cf.is_fld_union_storage:
                for uf in self.fld_unions[cf][0]:
                    uf.phv_bit = -1

    def check_512b_fld_crossing(self, flit, hdr, start_phv):
        # return extra bits needed if any fld crosses 512b boundary, else 0
        # For hdr-union check all headers and report max of extra bits needed
        max_extra_bits = 0
        # hdr size check to see if it even needed to check each fld
        hdr_size = self.get_header_storage_size(hdr) * 8

        foffset = start_phv - flit.base_phv_bit
        if not (foffset < 512 and (foffset+hdr_size) > 512):
            return

        if hdr in self.hdr_unions:
            hdrs = self.hdr_unions[hdr][1]
        else:
            hdrs = [hdr]
        for h in hdrs:
            extra_bits = 0
            hdr_byte_aligned_cfs = self.hdr_get_byte_aligned_fld_chunks(h)
            foffset = start_phv - flit.base_phv_bit
            for f in h.fields:
                cf = self.get_field(get_hfname(f))
                ncc_assert(cf)
                if cf in hdr_byte_aligned_cfs:
                    if cf.is_fld_union:
                        # XXX more checks are needed XXX
                        # do not include union-ed flds in the calculation
                        continue
                    blen = hdr_byte_aligned_cfs[cf]
                    if foffset < 512 and foffset+blen > 512:
                        extra_bits = 512-foffset
                        if extra_bits > max_extra_bits:
                            max_extra_bits = extra_bits

                if cf.storage_size():
                    foffset += cf.storage_size() + extra_bits
        return max_extra_bits

    def assign_phv_to_hdr_flds(self, flit, hdr, start_phv):
        # assumption: 512b fld crossing check is done by the caller
        # this is called for hdr unions as well hdrs that may some some fld union flds
        ncc_assert(hdr.name != 'i2e_metadata')
        foffset = start_phv
        flit_phv = start_phv - flit.base_phv_bit

        hdr_byte_aligned_cfs = self.hdr_get_byte_aligned_fld_chunks(hdr)
        for f in hdr.fields:
            cf = self.get_field(get_hfname(f))
            ncc_assert(cf)
            if hdr_is_intrinsic(hdr):
                cf.is_ohi = False
                ncc_assert(cf.is_intrinsic)

            if not cf.is_fld_union and cf in hdr_byte_aligned_cfs:
                blen = hdr_byte_aligned_cfs[cf]
                if flit_phv < 512 and flit_phv+blen > 512:
                    foffset += 512-flit_phv
            # fix union-ed flds if this is storage for the union
            if cf.is_fld_union_storage:
                for uf in self.fld_unions[cf][0]:
                    ncc_assert(uf.phv_bit < 0 or uf.phv_bit == foffset)
                    uf.phv_bit = foffset

            if cf.storage_size() and not cf.is_fld_union:
                cf.phv_bit = foffset

            if not cf.is_ohi and not cf.is_fld_union:
                # fld unions are handles above, for hdr unions, need to update the offset
                # for the next fld in the union-ed header, just skip that is not in phv
                foffset += cf.storage_size()
                flit_phv += cf.storage_size()
        return foffset

    def p4_plus_assign_phv_to_hdr_flds(self, hdr, start_phv):
        # check for flds crossing 512b or 1024b boundary, move start of the fld on the next
        # 512b boundary
        flit_sz = self.pa.be.hw_model['parser']['flit_size']
        foffset = start_phv

        for f in hdr.fields:
            flit_phv = foffset % flit_sz
            cf = self.get_field(get_hfname(f))
            ncc_assert(cf)
            if hdr_is_intrinsic(hdr):
                cf.is_ohi = False
                ncc_assert(cf.is_intrinsic)

            if not cf.is_fld_union:
                blen = cf.storage_size()
                if flit_phv < 512 and flit_phv+blen > 512:
                    foffset += 512-flit_phv
                    self.logger.warning("Field %s crosses 512b boundary, at %d, %d " % \
                        (cf.hfname, flit_phv, blen))
                elif flit_phv+blen > flit_sz:
                    foffset += flit_sz-flit_phv
                    self.logger.warning("Field %s crosses 512b boundary, at %d, %d " % \
                        (cf.hfname, flit_phv, blen))
            # fix union-ed flds if this is storage for the union
            if cf.is_fld_union_storage:
                for uf in self.fld_unions[cf][0]:
                    ncc_assert(uf.phv_bit < 0 or uf.phv_bit == foffset)
                    uf.phv_bit = foffset

            if cf.storage_size() and not cf.is_fld_union:
                cf.phv_bit = foffset

            if not cf.is_ohi and not cf.is_fld_union:
                # fld unions are handles above, for hdr unions, need to update the offset
                # for the next fld in the union-ed header, just skip that is not in phv
                foffset += cf.storage_size()
        return foffset

    def add_intrinsic_meta_and_hv(self, flit, add_hv):
        # New phv allocation
        ncc_assert(flit.id == 0)
        bits_used = 0
        if self.capri_intr_hdr:
            phv_bit = flit.flit_chunk_alloc((get_header_size(self.capri_intr_hdr) * 8), 0, 0, 0, 1,
                                                                False)
            ncc_assert(phv_bit == 0)
            self.assign_phv_to_hdr_flds(flit, self.capri_intr_hdr, phv_bit)
            bits_used += (get_header_size(self.capri_intr_hdr) * 8)
            self.allocated_hf[self.capri_intr_hdr] = phv_bit

        if self.capri_p4_intr_hdr:
            phv_bit = flit.flit_chunk_alloc((get_header_size(self.capri_p4_intr_hdr) * 8),
                                            bits_used, 0, 0, 1, False)
            ncc_assert(phv_bit == bits_used)
            self.assign_phv_to_hdr_flds(flit, self.capri_p4_intr_hdr, phv_bit)
            bits_used += (get_header_size(self.capri_p4_intr_hdr) * 8)
            self.allocated_hf[self.capri_p4_intr_hdr] = phv_bit

        # add cfields used for predication
        pred_cfs = {}
        for c_pred in self.pa.be.tables.gress_tm[self.d].table_predicates.values():
            for cf,_ in c_pred.cfield_vals:
                if cf not in pred_cfs:
                    pred_cfs[cf] = True
                    if cf.is_hv or cf.is_intrinsic:
                        continue
                    # cannot use hdr flds as predicate, as they cannot be placed in 1st 512 bits
                    ncc_assert(cf.is_meta)
                    cf.is_predicate = True
                    flit.add_cfield(cf)
                    bits_used += cf.width
                    if cf.is_parser_extracted or cf.is_i2e_meta:
                        alignment = 8
                        justify = JUSTIFY_RIGHT
                    else:
                        alignment = 0
                        justify = JUSTIFY_LEFT

                    phv_bit = flit.flit_chunk_alloc(cf.width, -1,
                                            alignment, justify, 0, False)
                    ncc_assert(phv_bit >= 0)
                    cf.phv_bit = phv_bit
                    self.allocated_hf[cf] = phv_bit
        if add_hv:
            hv_location = self.pa.be.hw_model['parser']['hv_location']
            max_hv_bits = self.pa.be.hw_model['parser']['max_hv_bits']
            phv_bit = flit.flit_chunk_alloc(max_hv_bits, hv_location, 0, 0, 1, False)
            ncc_assert(phv_bit == hv_location)
            self.create_flit_hv_flds(flit, 'flit_%d_hv' % (flit.id),
                                        hv_location, max_hv_bits)
        # add p4_intr hdr flds to field_order
        if self.capri_p4_intr_hdr:
            for f in reversed(self.capri_p4_intr_hdr.fields):
                cf = self.get_field(get_hfname(f))
                ncc_assert(cf)
                if cf not in self.field_order:
                    self.field_order.insert(0,cf)

        if self.capri_deparser_len_hdr:
            # Variable len slots for deparser to use can only come from PHV flit-1
            # allocate PHV in that region.
            dpa_variable_len_phv_start_bit = self.pa.be.hw_model['deparser']['len_phv_start']
            phv_bit = flit.flit_chunk_alloc((get_header_size(self.capri_deparser_len_hdr) * 8),\
                                             dpa_variable_len_phv_start_bit, 0, 0, 1, False)
            ncc_assert(phv_bit == dpa_variable_len_phv_start_bit)
            self.assign_phv_to_hdr_flds(flit, self.capri_deparser_len_hdr, phv_bit)

        if self.capri_deparser_pad_hdr:
            dpa_pad_hdr_phv_start_bit = self.pa.be.hw_model['deparser']['pad_phv_start']
            phv_bit = flit.flit_chunk_alloc((get_header_size(self.capri_deparser_pad_hdr) * 8),\
                                             dpa_pad_hdr_phv_start_bit, 0, 0, 1, False)
            ncc_assert(phv_bit == dpa_pad_hdr_phv_start_bit)
            self.assign_phv_to_hdr_flds(flit, self.capri_deparser_pad_hdr, phv_bit)

        '''
        if self.capri_gso_csum_hdr:
            # GSO Final Csum computed value (using partially computed csum) captured in this header.
            # capri_parser expects this 16b csum to be first 16b in any 512b PHV flit.
            phv_flit_sz = self.pa.be.hw_model['phv']['flit_size']
            gso_phv_start_bit = self.pa.be.hw_model['phv']['gso_csum_phv_start'] + phv_flit_sz
            #gso_flit = self.flits[gso_phv_start_bit / self.pa.be.hw_model['parser']['flit_size']]
            phv_bit = flit.flit_chunk_alloc((get_header_size(self.capri_gso_csum_hdr) * 8),\
                                             flit.base_phv_bit+gso_phv_start_bit, 0, 0, 1, False)
            ncc_assert(phv_bit >= 0)
            self.assign_phv_to_hdr_flds(flit, self.capri_gso_csum_hdr, phv_bit)
            self.allocated_hf[self.capri_gso_csum_hdr] = phv_bit
        '''
        # allocate parser_end offset in parser flit 0 (as early as possible)
        # on egress pipeline delay the allocation for end_offset as i2e metadata will ensure
        # min flit
        if self.parser_end_off_cf and self.d == xgress.INGRESS:
            self.allocate_parser_end_off_phv(flit)
            self.allocated_hf[self.parser_end_off_cf] = self.parser_end_off_cf.phv_bit

    def create_flit_hv_flds(self, flit, name, start_bit, num_bits):
        hv_flds = []
        for i in range(num_bits):
            hv_field = capri_field(None, self.d, 1, '%s_%d' % (name, i))
            hv_field.is_hv = True
            hv_field.phv_bit = start_bit+i
            self.allocated_hf[hv_field] = start_bit+i
            hv_flds.insert(0, hv_field)

        for hv_field in hv_flds:
            self.field_order.insert(0, hv_field)

    def wide_key_table_phv_allocate(self):
        # start parser-flit for wide key tables is 1 (can change it - cmd line or pragma??)
        wide_key_fid = self.pa.be.hw_model['phv']['wide_key_start_flit']
        phv_flit_sz = self.pa.be.hw_model['phv']['flit_size']
        start_phv = wide_key_fid * phv_flit_sz
        fstart = start_phv

        for wct in self.pa.be.tables.gress_tm[self.d].wide_key_tables:
            # Start allocating from current start_phv forward.
            # once we reach last flit, add the space for collision_cf/action_pc and i-data
            reserve = 0
            id_size = max(wct.i_size, wct.d_size)
            if wct.collision_cf:
                ncc_assert((wct.collision_cf.width + id_size) < phv_flit_sz)
                reserve = wct.collision_cf.width
            elif wct.num_actions() > 1:
                reserve = 8
            i = -1
            fid = fstart/phv_flit_sz
            remaining_key_size = wct.key_size
            for i, (cf, mt, mask) in enumerate(wct.keys):
                flit_end = (fid+1)*phv_flit_sz
                if remaining_key_size+reserve+id_size <= phv_flit_sz:
                    fstart = flit_end
                    break

                if fstart+cf.width > flit_end:
                    fstart = flit_end

                fid = fstart/phv_flit_sz
                flit = self.flits[fid/2]    # use parser flits for allocation
                phv_bit = flit.flit_chunk_alloc(cf.width, fstart,
                                        align=0, justify=0, fixed_loc=1, cross_512b=False)
                cf.phv_bit = phv_bit
                ncc_assert(phv_bit == fstart)
                fstart = phv_bit+cf.width
                remaining_key_size -= cf.width
                self.allocated_hf[cf] = phv_bit

            ncc_assert(i > 0)

            fid = fstart/phv_flit_sz
            flit = self.flits[fid/2]    # use parser flits for allocation
            if wct.collision_cf:
                cf = wct.collision_cf
                phv_bit = flit.flit_chunk_alloc(cf.width, fstart,
                                        align=0, justify=0, fixed_loc=1, cross_512b=False)
                ncc_assert(phv_bit == fstart)
                cf.phv_bit = phv_bit
                fstart += cf.width
                self.allocated_hf[cf] = phv_bit
            else:
                fstart += reserve

            # place remaining keys followed by i fields
            for i in range(i, len(wct.keys)):
                cf, _, _ = wct.keys[i]
                phv_bit = flit.flit_chunk_alloc(cf.width, fstart,
                                        align=0, justify=0, fixed_loc=1, cross_512b=False)
                ncc_assert(phv_bit == fstart)
                cf.phv_bit = phv_bit
                fstart += cf.width
                self.allocated_hf[cf] = phv_bit

            # add input fields
            for cf in wct.input_fields:
                ncc_assert(not cf.is_union()   )# no unions in wide key
                phv_bit = flit.flit_chunk_alloc(cf.width, fstart,
                                        align=0, justify=0, fixed_loc=1, cross_512b=False)
                cf.phv_bit = phv_bit
                ncc_assert(phv_bit == fstart)
                fstart += cf.width
                self.allocated_hf[cf] = phv_bit

            wct.isize = -1 # reset these values for the rest of the processing
            wct.key_size = -1 # reset these values for the rest of the processing

    def create_flits2(self, add_hv=False):
        hfid = 0
        for fid, flit in enumerate(self.flits):
            if hfid == len(self.topo_sorted_hfgroups):
                break
            while True:
                if hfid == len(self.topo_sorted_hfgroups):
                    break
                # add as many hdr/flds to this flit as possible
                e = self.topo_sorted_hfgroups[hfid]
                if isinstance(e, list):
                    # All hfgs are added as list (even a single hfg)
                    # create a best fit combination from available hfs
                    hfg_list = e[:]
                    # get consistent allocation order
                    sorted(hfg_list, key=lambda h: (h.get_storage_size(), h.name), reverse=True)
                    # XXX try best fit later.. for now start with largest to smallest
                    all_done = True
                    for hfg in hfg_list:
                        # just use any element to allocate the group
                        hf = None
                        for h in hfg.hfs:
                            hf = h
                            break
                        if hf and not self.allocate_phv_for_hf(flit, hf):
                            all_done = False
                    if all_done:
                        hfid += 1
                    else:
                        # if one of the hfgs in the list failed
                        # go to next flit and try this hfg set again
                        # successfully allocated hfgs will be automatically skipped
                        break
                else:
                    hf = e
                    if not self.allocate_phv_for_hf(flit, hf):
                        break
                    else:
                        hfid += 1

            # this flit is done, check for special phv allocation and move to next flit
            # allocate special phvs at the end of a flit if possible
            if self.parser_end_off_cf and self.parser_end_off_cf.phv_bit == -1:
                self.allocate_parser_end_off_phv(flit)
                self.allocated_hf[self.parser_end_off_cf] = self.parser_end_off_cf.phv_bit

            if self.capri_gso_csum_hdr and self.capri_gso_csum_hdr not in self.allocated_hf:
                # GSO Final Csum computed value (using partially computed csum) captured in this header.
                # capri_parser expects this 16b csum to be first 16b in any 512b PHV flit.
                phv_flit_sz = self.pa.be.hw_model['phv']['flit_size']
                gso_phv_start_bit = self.pa.be.hw_model['phv']['gso_csum_phv_start'] + phv_flit_sz
                #gso_flit = self.flits[gso_phv_start_bit / self.pa.be.hw_model['parser']['flit_size']]
                phv_bit = flit.flit_chunk_alloc((get_header_size(self.capri_gso_csum_hdr) * 8),\
                                                 flit.base_phv_bit+gso_phv_start_bit, 0, 0, 1, False)
                if phv_bit >= 0:
                    self.assign_phv_to_hdr_flds(flit, self.capri_gso_csum_hdr, phv_bit)
                    self.allocated_hf[self.capri_gso_csum_hdr] = phv_bit
            # move to next flit

        phv_error = False
        if hfid == len(self.topo_sorted_hfgroups) and self.pa.be.args.split_deparse_only_headers:
            # no errors so far - allocate deparse-only headers
            for hf in self.pa.be.parsers[self.d].deparse_only_hdrs:
                if hf in self.allocated_hf:
                    # could be synthetic header
                    self.logger.debug("%s:Deparse only header %s is aloready allocated" % \
                                        (self.d.name, hf))
                    continue
                if not self.split_allocate_deparse_only_header(hf):
                    phv_error = True
                    break

        if hfid < len(self.topo_sorted_hfgroups) or phv_error:
            # not enough phvs
            self.pa.logger.critical("Not enough space in phv")
            self.print_field_order_info("PHV order(PHV FAILURE)")
            self.pa.logger.debug("%s:Field Allocation %s" % (self.d.name,
                                sorted(self.field_order, key=lambda cf: cf.phv_bit)))
            for fid, flit in enumerate(self.flits):
                self.pa.logger.debug("%s:Flit %d, available bits %d chunks %s" % \
                                    (self.d.name, flit.id, flit.flit_avail, flit.free_chunks))
            ncc_assert(0)

        self.field_order = sorted(self.field_order, key=lambda k: k.phv_bit)
        
    def create_flits(self, add_hv=False):
        self.clear_phv_assignment()

        # XXX reset the flit
        # add intrinsic headers and special flds to flit0
        self.pa.logger.debug("%s:HdrFldOrder(Init) %s" % \
            (self.d.name, self.hdr_fld_order))

        if self.pa.be.args.p4_plus:
            if not self.allocate_p4_plus_phv_for_hf():
                # not enough phvs
                self.pa.logger.critical("Not enough space in phv")
                self.print_field_order_info("PHV order(PHV FAILURE)")
                self.pa.logger.debug("%s:Field Allocation %s" % (self.d.name, self.field_order))
                ncc_assert(0)
            self.field_order = sorted(self.field_order, key=lambda cf: cf.phv_bit)
            return

        # process special allocation for wide-key cfs..
        if len(self.pa.be.tables.gress_tm[self.d].wide_key_tables):
            self.wide_key_table_phv_allocate()

        flit = self.flits[0]
        self.add_intrinsic_meta_and_hv(flit, add_hv)

        if len(self.topo_sorted_hfgroups):
            self.pa.logger.debug("%s:HFG Order(Init) %s" % \
                (self.d.name, self.topo_sorted_hfgroups))
            return self.create_flits2(add_hv)

        # old phv allocation --->
        hfid = 0
        for fid, flit in enumerate(self.flits):
            if hfid == len(self.hdr_fld_order):
                break
            # add as many hdr/flds to this flit as possible
            while self.allocate_phv_for_hf(flit, self.hdr_fld_order[hfid]):
                hfid += 1
                if hfid == len(self.hdr_fld_order):
                    break
            # allocate special phvs at the end of a flit if possible
            if self.parser_end_off_cf and self.parser_end_off_cf.phv_bit == -1:
                self.allocate_parser_end_off_phv(flit)
                self.allocated_hf[self.parser_end_off_cf] = self.parser_end_off_cf.phv_bit

            if self.capri_gso_csum_hdr and self.capri_gso_csum_hdr not in self.allocated_hf:
                # GSO Final Csum computed value (using partially computed csum) captured in this header.
                # capri_parser expects this 16b csum to be first 16b in any 512b PHV flit.
                phv_flit_sz = self.pa.be.hw_model['phv']['flit_size']
                gso_phv_start_bit = self.pa.be.hw_model['phv']['gso_csum_phv_start'] + phv_flit_sz
                #gso_flit = self.flits[gso_phv_start_bit / self.pa.be.hw_model['parser']['flit_size']]
                phv_bit = flit.flit_chunk_alloc((get_header_size(self.capri_gso_csum_hdr) * 8),\
                                                 flit.base_phv_bit+gso_phv_start_bit, 0, 0, 1, False)
                if phv_bit >= 0:
                    self.assign_phv_to_hdr_flds(flit, self.capri_gso_csum_hdr, phv_bit)
                    self.allocated_hf[self.capri_gso_csum_hdr] = phv_bit
            # move to next flit

        if hfid < len(self.hdr_fld_order):
            # not enough phvs
            self.pa.logger.critical("Not enough space in phv")
            self.print_field_order_info("PHV order(PHV FAILURE)")
            self.pa.logger.debug("%s:Field Allocation %s" % (self.d.name,
                                sorted(self.field_order, key=lambda cf: cf.phv_bit)))
            ncc_assert(0)
        
        # Allocate deparse-only headers
        if self.pa.be.args.split_deparse_only_headers:
            for hf in self.hdr_fld_order:
                if hf in self.allocated_hf and \
                    hf not in self.pa.be.parsers[self.d].deparse_only_hdrs:
                    continue
                # deparse-only hdrs are yet to be allocated
                hfid -= 1

            if hfid < len(self.hdr_fld_order):
                hfid = 0
                for hf in self.hdr_fld_order:
                    if hf in self.allocated_hf or \
                        hf not in self.pa.be.parsers[self.d].deparse_only_hdrs:
                        hfid += 1
                        continue
                    # A deparse only header can be split on byte aligned boundaries
                    # and placed into non-contiguous phvs. This is useful when we run out of phv
                    # The deparse-only headers can be used to fill in the unused flit bytes
                    if self.split_allocate_deparse_only_header(hf):
                        hfid += 1
                        continue
                    # could not find space for the deparse-only header
                    self.pa.logger.critical("Not enough space in phv for deparse-only headers")
                    self.print_field_order_info("PHV order(PHV FAILURE)")
                    self.pa.logger.debug("%s:Field Allocation %s" % (self.d.name,
                                        sorted(self.field_order, key=lambda cf: cf.phv_bit)))
                    ncc_assert(0)

                ncc_assert(hfid == len(self.hdr_fld_order))

        # print the un-used flit bits
        total_unused_bits = 0
        phv_flit_sz = self.pa.be.hw_model['phv']['flit_size']
        for flit in self.flits:
            if flit.flit_avail:
                self.pa.logger.debug("%s:Flit %d unused bits %d chunks %s" % \
                    (self.d.name, flit.id, flit.flit_avail, flit.free_chunks))
                total_unused_bits += flit.flit_avail
            for chunk in flit.free_chunks:
                if chunk[1] == flit.fsize:
                    # assume no flit is used after this
                    break
                # do not cross 512bit boundary when creating flit pads
                if chunk[0] < phv_flit_sz and chunk[0]+chunk[1] > phv_flit_sz:
                    clen0 = phv_flit_sz - chunk[0]
                    clen1 = chunk[0]+chunk[1]-phv_flit_sz
                else:
                    clen0 = chunk[1]
                    clen1 = 0
                cf = capri_field(None, self.d, clen0, '_flit_pad__%d' % \
                    (flit.base_phv_bit + chunk[0]))
                cf.is_flit_pad = True
                cf.phv_bit = flit.base_phv_bit + chunk[0]
                self.field_order.append(cf)

                if clen1 > 0:
                    cf = capri_field(None, self.d, clen1, '_flit_pad__%d' % \
                        (flit.base_phv_bit + phv_flit_sz))
                    cf.is_flit_pad = True
                    cf.phv_bit = flit.base_phv_bit + phv_flit_sz
                    self.field_order.append(cf)


        self.pa.logger.debug("%s: total unused bits %d" % (self.d.name, total_unused_bits))
        # sort the fld order based on phv#
        self.field_order = sorted(self.field_order, key=lambda cf: cf.phv_bit)

        # check that all fields except ohi are given a phv#
        for cf in self.field_order:
            if cf.is_ohi or cf.is_internal:
                continue
            ncc_assert(cf.phv_bit >= 0)

    def allocate_p4_plus_phv_for_hf(self):
        # hdr_fld_order consists for only headers
        # start_phv bit for each hdr (to handle hdr unions)
        # Any hdr unions fields get phvbit from its parent header
        phv_bit = 0
        flit_sz = self.pa.be.hw_model['parser']['flit_size']
        num_flits = len(self.flits)
        max_phv_bits = num_flits * flit_sz
        hf = None
        for hf in self.hdr_fld_order:
            if hf in self.allocated_hf:
                continue

            if phv_bit >= max_phv_bits:
                break

            fid = phv_bit/flit_sz
            flit = self.flits[fid]
            if not isinstance(hf, capri_field):
                if hf in self.hdr_unions and hf != self.hdr_unions[hf][2]:
                    continue

                align = get_header_alignment(hf)
                if align and phv_bit % align:
                    phv_bit += (align - (phv_bit % align))
                hdr_size = self.get_header_storage_size(hf) * 8
                max_offset = phv_bit
                if hf in self.hdr_unions:
                    max_offset = self.p4_plus_assign_phv_to_hdr_flds(hf, phv_bit)
                    if max_offset != (phv_bit+hdr_size):
                        # P4plus - special case with hdr unions
                        # start all hdrs at the same phv as the storage header
                        first_cf = self.get_field(get_hfname(hf.fields[0]))
                        ncc_assert(first_cf)
                        phv_bit = first_cf.phv_bit

                    for uh in self.hdr_unions[hf][1]:
                        if uh == hf:
                            continue
                        used_offset = self.p4_plus_assign_phv_to_hdr_flds(uh, phv_bit)
                        if used_offset > max_offset:
                            self.logger.critical("Header %s size %d cannot exceed size of %s %d" % \
                                (uh.name, used_offset-phv_bit, hf.name, max_offset-phv_bit));
                            #ncc_assert(0)
                            max_offset = used_offset
                        ncc_assert(uh not in self.allocated_hf )# duplicate allocation
                        self.allocated_hf[uh] = phv_bit
                else:
                    max_offset = self.p4_plus_assign_phv_to_hdr_flds(hf, phv_bit)
                    ncc_assert(hf not in self.allocated_hf )# duplicate allocation
                    self.allocated_hf[hf] = phv_bit
                if max_offset > (phv_bit+hdr_size):
                    # pad is added while generating output
                    pass
                phv_bit = max_offset
            else:
                ncc_assert(0)

        if len(self.hdr_fld_order) and hf != self.hdr_fld_order[-1]:
            return False
        return True

    def allocate_parser_end_off_phv(self, start_flit):
        # there are two places in a parser flit this can go, 480:495 or 992:1007
        # try both. If both the slots are used, use the next flit (can try previous
        # flit as well) To make sure we do not find a place for this, last flit has a reserved
        # slot for this (XXX)
        cf = self.parser_end_off_cf
        loc = self.pa.be.hw_model['phv']['parser_end_off_flit_loc']
        phv_flit_size = self.pa.be.hw_model['phv']['flit_size']
        for f in range(start_flit.id, len(self.flits)):
            # try end of the flit first (mostly padded)
            flit = self.flits[f] 
            phv_bit = flit.flit_chunk_alloc(cf.width, flit.base_phv_bit+loc+phv_flit_size,
                                        align=0, justify=0, fixed_loc=1, cross_512b=False)
            if phv_bit >= 0:
                cf.phv_bit = phv_bit
                return
            # try next (earlier) phv flit location
            phv_bit = flit.flit_chunk_alloc(cf.width, flit.base_phv_bit+loc,
                                        align=0, justify=0, fixed_loc=1, cross_512b=False)
            if phv_bit >= 0:
                cf.phv_bit = phv_bit
                return

        # Try earlier flits
        for f in range(0, start_flit.id):
            flit = self.flits[f] 
            phv_bit = flit.flit_chunk_alloc(cf.width, flit.base_phv_bit+loc,
                                        align=0, justify=0, fixed_loc=1, cross_512b=False)
            if phv_bit >= 0:
                cf.phv_bit = phv_bit
                return
            # try next phv flit location
            phv_bit = flit.flit_chunk_alloc(cf.width, flit.base_phv_bit+loc+phv_flit_size,
                                        align=0, justify=0, fixed_loc=1, cross_512b=False)
            if phv_bit >= 0:
                cf.phv_bit = phv_bit
                return
        ncc_assert(0, "No phv location free for parser_end_offset")

    def split_allocate_deparse_only_header(self, hf):
        # Hdr and fld unions
        # Hdr union - get the byte aligned flds for all unioned headers
        # Fld union - allocate phv for  the largest field (unioned flds are byte aligned)
        if hf in self.hdr_unions:
            hdrs = self.hdr_unions[hf][1]
            max_size = 0
            big_hdr = None
            for hdr in hdrs:
                if get_header_max_size(hdr) > max_size:
                    max_size = get_header_max_size(hdr)
                    big_hdr = hdr

            max_hdr_byte_aligned_cfs = self.hdr_get_byte_aligned_fld_chunks(big_hdr)

            nxt_offset = 0
            num_hdrs = len(hdrs) - 1    # remove the big header
            aligned_offsets = []
            for cf, l in max_hdr_byte_aligned_cfs.items():
                nxt_offset += l
                aligned_hdrs = 0
                for hdr in hdrs:
                    if hdr == big_hdr:
                        continue
                    if nxt_offset >= (get_header_max_size(hdr) * 8):
                        aligned_hdrs += 1
                        continue
                    aligned = False
                    for f in hdr.fields:
                        if f.offset > nxt_offset:
                            break
                        if f.offset == nxt_offset:
                            aligned = True
                            break
                    if aligned:
                        aligned_hdrs += 1

                if aligned_hdrs == num_hdrs:
                    aligned_offsets.append(nxt_offset)
            
            offset = 0
            for nxt_offset in aligned_offsets:
                num_bits = nxt_offset - offset
                for flit in self.flits:
                    # XXX make a function out of this loop
                    # allocate for all conscutive fields until byte boundary
                    phv_bit = flit.flit_chunk_alloc(num_bits, location=-1,
                                        align=8, justify=0, fixed_loc=0, cross_512b=False)
                    if phv_bit == -1:
                        # try next flit
                        continue
                    break
                ncc_assert(phv_bit >= 0)
                # assign phv_bits to all covered fields in all headers
                for hdr in hdrs:
                    hdr_bit = -1
                    for f in hdr.fields:
                        if f.offset < offset:
                            continue
                        if f.offset >= nxt_offset:
                            break
                        if hdr_bit < 0:
                            ncc_assert(f.offset == offset)
                            hdr_bit = phv_bit
                        cf = self.get_field(get_hfname(f))
                        cf.phv_bit = hdr_bit
                        hdr_bit += cf.width
                offset += num_bits

            for hdr in hdrs:
                self.allocated_hf[hdr] = phv_bit
            return True

        # group deparse only header fields on byte boundaries
        hdr_byte_aligned_cfs = self.hdr_get_byte_aligned_fld_chunks(hf)
        # allocate bytes(s) from each byte-aligned cf and map hdr flds to phv
        phv_bit = -1
        for f in hf.fields:
            cf = self.get_field(get_hfname(f))
            ncc_assert(cf)
            if not cf.need_phv():
                continue
            if cf in hdr_byte_aligned_cfs:
                if cf.is_fld_union or cf.is_fld_union_storage:
                    self.logger.critical("Field unions in deparse-only headers are TBD")
                    ncc_assert(0)
                for flit in self.flits:
                    # allocate for all conscutive fields until byte boundary
                    phv_bit = flit.flit_chunk_alloc(hdr_byte_aligned_cfs[cf], location=-1,
                                        align=8, justify=0, fixed_loc=0, cross_512b=False)
                    if phv_bit == -1:
                        # try next flit
                        continue
                    break
            ncc_assert(phv_bit >= 0)
            cf.phv_bit = phv_bit
            phv_bit += cf.width
        self.allocated_hf[hf] = phv_bit
        return True
        
    def allocate_phv_for_hf(self, flit, hf):
        # Assumptions:
        # - hf is either a header or metadata fld
        # - hf can be hdr union or some of the hdr flds can be part fld union
        # - meta flds can be fld-union
        # - metadata header unions are only allowed for P4+
        # For P4:
        # - Find all associated/connected headers and flds for a given hf and try to
        # allocate them in a single flit
        # For P4+ :
        # handled in a separate function
        # - Flit restriction is not applied
        # - meta flds can be hdr or fld unions
        # - There are no connected/associtated hdr and flds since flit restriction is not there
        if hf not in self.hf_to_group or self.hf_to_group[hf] == None:
            if hf in self.allocated_hf:
                return True

            if isinstance(hf, capri_field):
                if hf.is_union():
                    ncc_assert(hf.phv_bit >= 0)

                if hf.phv_bit >= 0:
                    # already assigned a phv
                    return True
                self.pa.logger.debug("%s:Flit %d Start allocation for %s" % \
                    (self.d.name, flit.id, hf.hfname))
            else:
                if self.pa.be.args.split_deparse_only_headers:
                    # special check for deparse-only headers - these are allocated outside this function
                    if hf in self.pa.be.parsers[self.d].deparse_only_hdrs:
                        return True

                if hf in self.hdr_unions and hf != self.hdr_unions[hf][2]:
                    # union-ed header but not storage for union
                    return True
                self.pa.logger.debug("%s:Flit %d Start allocation for Hdr %s" % \
                    (self.d.name, flit.id, hf.name))

            if isinstance(hf, capri_field) and hf.allow_relocation:
                # Previously relocation was not allowed for table k or i fields so alignment and
                # Relocating table k,i fields
                justify = JUSTIFY_LEFT
                if hf in self.i2e_fields or \
                    ((hf.is_key or hf.is_input) and hf.storage_size() >= 4) or \
                    hf.is_fld_union_storage:
                    align = 8
                else:
                    align = 0
                if hf.is_index_key:
                    # Earlier the alignment was done only for flds >= 4 size (not sure why)
                    justify = JUSTIFY_RIGHT
                    align = 8

                # try to allocate in prior flits and then this flit
                for fid in range(flit.id-1, -1, -1):
                    phv_bit = self.flits[fid].flit_chunk_alloc(hf.width, -1, align, justify, 0, False)
                    if phv_bit >= 0:
                        hf.phv_bit = phv_bit
                        self.pa.logger.debug("%s:Allocated %s at %d in closed flit %d" % \
                            (self.d.name, hf.hfname, hf.phv_bit, fid))
                        # check unions (useful for syn hdr flds)
                        if hf.is_fld_union_storage:
                            for uf in self.fld_unions[hf][0]:
                                uf.phv_bit = phv_bit
                                self.allocated_hf[uf] = phv_bit
                        return True

                # try current flit - fallthru' to use the common code
                '''
                phv_bit = flit.flit_chunk_alloc(hf.width, -1, align, justify, 0, False)
                if phv_bit >= 0:
                    hf.phv_bit = phv_bit
                    return True
                # cannot allocate in current flit
                return False
                '''

        # get all the headers and flds that need to go into a single flit
        hf_list = []; cs_list = []
        if self.pa.be.args.p4_plus:
            hf_list.append(hf)
        else:
            # use new hf groups created
            if hf in self.hf_to_group and self.hf_to_group[hf] != None:
                hfg = self.hf_to_group[hf]
                self.pa.logger.debug("%s:Start allocation of %s" % (self.d.name, hfg))
                hf_list += hfg.sorted_hfs()
            else:
                hf_list.append(hf)

        # allocate phv for the headers/flds that are not already placed
        # i2e header - XXX
        # - do not pad i2 flds at init time
        # - combine all relocatable i2e flds into bytes
        # - combine all i2e flds that fall in this flit together and pad the collection to byte
        # - create the definition of i2e header at the end of ingress phv allocation
        # XXX deparse-only headers can be allocated into earlier flits - TBD
        new_hfs = []
        bits_needed = 0
        hdr_size = 0
        for n_hf in hf_list:
            # remove all the hfs already allocated
            if n_hf in self.allocated_hf:
                continue
            if isinstance(n_hf, capri_field):
                if not n_hf.need_phv():
                    continue
                ncc_assert(n_hf.phv_bit < 0)
                if n_hf.phv_bit >= 0:
                    # already assigned a phv
                    continue
                new_hfs.append(n_hf)
                bits_needed += n_hf.storage_size()
            else:
                hsize = self.get_header_storage_size(n_hf)
                if hsize == 0:
                    # entire header is in ohi - no need to process it here
                    continue
                hdr_size += hsize
                new_hfs.append(n_hf)

        total_bits_needed = bits_needed + (hdr_size * 8)
        parser_flit_size = self.pa.be.hw_model['parser']['flit_size']
        if total_bits_needed > parser_flit_size:
            self.pa.logger.critical("%s:Cannot place required headers/flds into one flit" % \
                                    (self.d.name))
            self.pa.logger.critical("%s:Need %d bits for hdr/flds %s" % \
                (self.d.name, total_bits_needed, hf_list))
            ncc_assert(0)
            return False

        if total_bits_needed > flit.flit_avail:
            self.pa.logger.debug("%s:Close flit %d, bits avail %d needed %d for %s" % \
                (self.d.name, flit.id, flit.flit_avail, total_bits_needed, hf_list))
            return False

        if total_bits_needed == 0:
            return True

        self.pa.logger.debug("%s:Flit %d Allocating %d bits (out of %d) for %s" % \
            (self.d.name, flit.id, total_bits_needed, flit.flit_avail, hf_list))

        # sort new_hf - allocate headers first, then meta flds
        # for fld unions - the union headers will be placed first, ensure that by sorting
        # new_hf based on position in hdr_fld_order.
        # worry about k-i meta flds??

        # most likely the things will fit.. when pad is added to align/justify flds, it can
        # overflow available space
        allocated_chunks = []
        new_allocated_hfs = []
        overflow = False
        last_hdr_bit = -1
        capri_i2e_hdr = None

        for n_hf in new_hfs:
            if isinstance(n_hf, capri_field):
                continue
            if self.d == xgress.EGRESS and n_hf.name == 'capri_i2e_metadata':
                capri_i2e_hdr = n_hf
                continue

            # Hdr - allocate all hdr flds one at a time so that it is easy to check for 512b
            # crossing
            # For union-ed header, may need to alloc additional bytes at the end if this header
            # is not the largest of the union-ed headers
            # For fld unions allocate space for largest union-ed fld
            # For hdr unions - the headers must be allocated in a contiguous space
            # In that case it is better to make sure that there is a contiguous space for it
            # by allocating it an a single chunk ??
            # For non-unioned headers, create smallest byte_aligned chunks w/o splitting a field
            # for now do this only for i2e header
            if n_hf in self.hdr_unions and n_hf != self.hdr_unions[n_hf][2]:
                # header union but not storage for union
                continue

            hdr_storage_size = self.get_header_storage_size(n_hf) * 8

            # allocate on next byte boundary
            align = get_header_alignment(n_hf)
            if align == 0:
                align = 8
            else:
                pass

            phv_bit = flit.flit_chunk_alloc(hdr_storage_size, -1, align, 0, 0, True)
            if phv_bit < 0:
                overflow = True
                break

            if phv_bit > last_hdr_bit:
                last_hdr_bit = phv_bit

            allocated_chunks.append((phv_bit, hdr_storage_size))
            extra_bits = self.check_512b_fld_crossing(flit, n_hf, phv_bit)
            if extra_bits:
                # XXX defer this header allocation to see if another header fits here - TBD
                extra_phv_bit = flit.flit_chunk_alloc(extra_bits, phv_bit+hdr_storage_size,
                                                        0, 0, 1, True)
                if extra_bits < 0:
                    overflow = True
                    break
                allocated_chunks.append((extra_phv_bit, extra_bits))

            if n_hf in self.hdr_unions:
                for uh in self.hdr_unions[n_hf][1]:
                    self.pa.logger.debug("%s:Allocate phv %d for union-ed hdr %s" % \
                        (self.d.name, phv_bit, uh.name))
                    self.assign_phv_to_hdr_flds(flit, uh, phv_bit)
                    ncc_assert(uh not in self.allocated_hf )# duplicate allocation
                    self.allocated_hf[uh] = phv_bit
                    new_allocated_hfs.append(uh)
            else:
                self.pa.logger.debug("%s:Allocate phv %d for hdr %s" % \
                        (self.d.name, phv_bit, n_hf.name))
                self.assign_phv_to_hdr_flds(flit, n_hf, phv_bit)
                ncc_assert(n_hf not in self.allocated_hf )# duplicate allocation
                self.allocated_hf[n_hf] = phv_bit
                new_allocated_hfs.append(n_hf)

        if overflow:
            self.pa.logger.debug("%s:Close flit %d, Overflow: bits avail %d while allocating %s" % \
                (self.d.name, flit.id, flit.flit_avail, new_hfs))
            self.clear_new_flit_phv_assignment(flit, allocated_chunks, new_allocated_hfs)
            return False

        if capri_i2e_hdr:
            # breakup i2e header in byte aligned fields and place it
            hdr_byte_aligned_cfs = self.hdr_get_byte_aligned_fld_chunks(capri_i2e_hdr)
            phv_bit = -1
            for f in capri_i2e_hdr.fields:
                cf = self.get_field(get_hfname(f))
                ncc_assert(cf)
                if cf in hdr_byte_aligned_cfs:
                    fsize = hdr_byte_aligned_cfs[cf]
                    phv_bit = flit.flit_chunk_alloc(fsize, -1, align=8, justify=0, fixed_loc=0,
                                                    cross_512b=False)
                    if phv_bit < 0:
                        overflow = True
                        break

                    allocated_chunks.append((phv_bit, fsize))
                else:
                    ncc_assert(phv_bit >= 0)

                ncc_assert(cf not in self.allocated_hf )# duplicate allocation
                # check fld unions
                if cf.is_fld_union_storage:
                    for uf in self.fld_unions[cf][0]:
                        uf.phv_bit = phv_bit
                        self.allocated_hf[uf] = phv_bit
                        new_allocated_hfs.append(uf)
                cf.phv_bit = phv_bit
                phv_bit += cf.storage_size()

            if not overflow:
                self.allocated_hf[capri_i2e_hdr] = phv_bit
                new_allocated_hfs.append(capri_i2e_hdr)

        if overflow:
            self.pa.logger.debug("%s:Close flit %d, Overflow: bits avail %d while allocating %s" % \
                (self.d.name, flit.id, flit.flit_avail, new_hfs))
            self.clear_new_flit_phv_assignment(flit, allocated_chunks, new_allocated_hfs)
            return False

        # do name base sorting to make it stable across builds
        new_fs = [_nf for _nf in new_hfs if isinstance(_nf, capri_field)]
        new_fs = sorted(new_fs, key=lambda cf: cf.hfname)
        for n_hf in new_fs:
            if not isinstance(n_hf, capri_field):
                continue
            # index_key => byte-aligned, rt justified
            # other-k or i: byte aligned, left justified
            # This seems to remove most of the bit extraction violations
            # if a meta fld is used as tbl key or tbl input, just align it on byte
            # boundary and pad its size to be multiple of bytes (will increase phv util)
            # small consecutive fields if used as key/input can be combined ... XXX
            ncc_assert(n_hf.is_meta)
            alignment = 0; justify = JUSTIFY_LEFT
            use_last_hdr_bit = False
            hdr_alignment = 0
            if n_hf.p4_fld and n_hf.p4_fld.instance:
                hdr_alignment = get_header_alignment(n_hf.p4_fld.instance)
            if hdr_alignment and n_hf.p4_fld.offset == 0:
                alignment = hdr_alignment
            else:
                if n_hf.is_key or n_hf.is_input or (self.d == xgress.INGRESS and n_hf.is_i2e_meta):
                    # XXX for fld unions - propagate the flags is_key... to union storage field
                    if n_hf.is_index_key:
                        # Earlier the alignment was done only for flds >= 4 size (not sure why)
                        justify = JUSTIFY_RIGHT
                        alignment = 8
                    elif (self.d == xgress.INGRESS and n_hf.is_i2e_meta) or n_hf.storage_size() > 8:
                        # align it on byte boudary to make it easy for key_maker programming
                        # i2e flds need to be byte-aligned for the ingress-deparser
                        alignment = 8
                    else:
                        pass
                    use_last_hdr_bit = True

                if n_hf.is_parser_extracted:
                    alignment = 8
                    justify = JUSTIFY_RIGHT

                if n_hf.is_fld_union_storage:
                    # this handles syn header fld unions
                    alignment = 8

            if use_last_hdr_bit:
                start_loc = last_hdr_bit
            else:
                start_loc = -1

            phv_bit = flit.flit_chunk_alloc(n_hf.storage_size(), start_loc,
                                            alignment, justify, 0, False)
            if phv_bit < 0:
                overflow = True
                break

            allocated_chunks.append((phv_bit, n_hf.storage_size()))
            ncc_assert(n_hf not in self.allocated_hf )# duplicate allocation
            self.allocated_hf[n_hf] = phv_bit
            new_allocated_hfs.append(n_hf)
            n_hf.phv_bit = phv_bit

            # check unions (useful for syn hdr flds)
            if n_hf.is_fld_union_storage:
                for uf in self.fld_unions[n_hf][0]:
                    uf.phv_bit = phv_bit
                    self.allocated_hf[uf] = phv_bit
                    new_allocated_hfs.append(uf)

        if overflow:
            self.pa.logger.debug("%s:Close flit %d, Overflow: bits avail %d while allocating %s" % \
                (self.d.name, flit.id, flit.flit_avail, new_hfs))
            self.clear_new_flit_phv_assignment(flit, allocated_chunks, new_allocated_hfs)
            return False

        return True

    def hdr_has_fld_unions(self, hf):
        for f in hf.fields:
            cf = self.get_field(get_hfname(f))
            ncc_assert(cf)
            if cf in self.fld_unions:
                return True
        return False

    def hdr_get_hdrs_from_fld_unions(self, hf):
        uhdrs = []
        for f in hf.fields:
            cf = self.get_field(get_hfname(f))
            ncc_assert(cf)
            if cf in self.fld_unions:
                for uf in self.fld_unions[cf][0]:
                    # XXX return metadata fld
                    if uf == cf:
                        continue
                    if uf.is_meta:
                        uhdrs.append(uf)
                        continue
                    uh = uf.get_p4_hdr()
                    if uh not in uhdrs:
                        uhdrs.append(uh)

        return uhdrs

    def find_hf_flit_group(self, hf_list, cs_list, hf):
        def _find_hfs_for_cs(hf_list, cs_list, cs):
            if cs in cs_list:
                return
            cs_list.append(cs)
            for h in cs.headers:
                if h in hf_list or is_synthetic_header(h):
                    continue
                self.find_hf_flit_group(hf_list, cs_list, h)

            for mf in cs.meta_extracted_fields:
                if mf in hf_list:
                    continue
                self.find_hf_flit_group(hf_list, cs_list, mf)

        #### function body ####
        ext_css = []
        if hf in hf_list:
            return
        if isinstance(hf, capri_field):
            ncc_assert(hf.is_meta)
            ext_css = self.pa.be.parsers[self.d].get_meta_ext_cstates(hf)
            hf_list.append(hf)
        else:
            if is_synthetic_header(hf):
                hf_list.append(hf)
                return
            uhdrs = []
            if hf in self.hdr_unions:
                uhdrs += self.hdr_unions[hf][1]
            elif self.hdr_has_fld_unions(hf):
                # fld unions may be with other meta flds not headers
                # XXX need to get the meta flds and their ext_css
                uhdrs = self.hdr_get_hdrs_from_fld_unions(hf)
                if hf not in uhdrs:
                    uhdrs.insert(0, hf)
            else:
                uhdrs = [hf]

            for hdr_group in self.pa.be.parsers[self.d].hdr_order_groups:
                if hf in hdr_group:
                    for hg in hdr_group:
                        if hg not in uhdrs:
                            uhdrs.append(hg)

            for uh in uhdrs:
                if uh in hf_list:
                    continue
                if isinstance(uh, capri_field):
                    more_ext_css = self.pa.be.parsers[self.d].get_meta_ext_cstates(hf)
                    hf_list.append(uh)
                    for cs in more_ext_css:
                        if cs not in ext_css:
                            ext_css.append(cs)
                else:
                    if uh in hf_list or is_synthetic_header(uh):
                        continue
                    unioned_css = self.pa.be.parsers[self.d].get_ext_cstates(uh)
                    hf_list.append(uh)
                    for cs in unioned_css:
                        if cs not in ext_css:
                            ext_css.append(cs)

        for cs in ext_css:
            _find_hfs_for_cs(hf_list, cs_list, cs)

        return

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

    def get_hdr_phv_start(self, hdr):
        for f in hdr.fields:
            cf = self.get_field(get_hfname(f))
            ncc_assert(cf, "unknown field %s" % (hdr.name + f.name))
            if not cf.is_ohi:
                return cf.phv_bit
        return -1

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
                        ncc_assert(cf)
                        if cf.is_ohi:
                            cf.is_ohi = False
                            self.logger.debug("Set no_ohi on synthetic header fld %s" % cf.hfname)


                continue

            # check field unions
            for f in hdr.fields:
                cf = self.get_field(get_hfname(f))
                ncc_assert(cf)
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
        self.hdr_unions = OrderedDict() # {hdr: (direction, [hdrs_in_union], destination)}
        self.gress_pa = [capri_gress_pa(self, d) for d in xgress]
        self.dprsr_len_hdr = None
        self.dprsr_pad_hdr = None
        self.ig_gso_csum_hdr = None
        self.eg_gso_csum_hdr = None

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
                        # If header is part of hdr-checksum or payload checksum
                        # or l2 complete csum, allocate csum_hv bit
                        if self.be.checksum.IsHdrInCsumCompute(name, d) or \
                            self.be.checksum.IsHdrInCsumComputePhdr(name, d) or \
                            self.be.checksum.IsL2HdrInL2CompleteCsumCompute(name, d) or \
                            self.be.checksum.IsHdrInL2CompleteCsumCompute(name, d):
                            self.allocate_csum_hv_field(name, d)

                        if self.be.icrc.IsHdrInIcrcCompute(name, d):
                            #Allocate HV so that icrc computation
                            #can be triggered.
                            self.allocate_icrc_hv_field(name, d)

            if hdr.metadata:
                #capture metadata hdr inst that is used to store variable pkt len
                #and truncation length. Fields of this header have to be allocated
                #in first PHV flit.
                if 'deparser_variable_length_header' in hdr._parsed_pragmas:
                    self.dprsr_len_hdr = hdr
                    for d in xgress:
                        cf = self.allocate_hv_truncate_field(hdr.name, d)

                if 'deparser_pad_header' in hdr._parsed_pragmas:
                    self.dprsr_pad_hdr = hdr
                    for d in xgress:
                        cf = self.allocate_hv_padhdr_len_field(hdr.name, d)

                #For GSO checksum, allocate HV to enable/disable GSO
                #csum result into packet.
                if 'gso_csum_header' in hdr._parsed_pragmas:
                    for d in xgress:
                        if self.be.checksum.IsHdrInGsoCsumCompute(name, d):
                            self.allocate_gso_csum_hv_field(name, d)
                            if d == xgress.INGRESS:
                                self.ig_gso_csum_hdr = hdr
                            if d == xgress.EGRESS:
                                self.eg_gso_csum_hdr = hdr

        #Allocate field for payload len.
        for d in xgress:
            cf = self.allocate_hv_payload_len_field("capri_intrinsic", d)

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

                ecf = self.get_field(get_hfname(p4f), xgress.EGRESS)
                if not ecf:
                    ecf = self.allocate_field(p4f, xgress.EGRESS)
                ecf.is_ohi = False
                ecf.is_intrinsic = True

            self.gress_pa[xgress.INGRESS].capri_p4_intr_hdr = p4_intr_hdr
            self.gress_pa[xgress.EGRESS].capri_p4_intr_hdr = p4_intr_hdr

        if self.dprsr_len_hdr != None:
            for p4f in self.dprsr_len_hdr.fields:
                cf = self.get_field(get_hfname(p4f), xgress.INGRESS)
                if not cf:
                    cf = self.allocate_field(p4f, xgress.INGRESS)
                cf.is_ohi = False
                cf.is_intrinsic = False

                ecf = self.get_field(get_hfname(p4f), xgress.EGRESS)
                if not ecf:
                    ecf = self.allocate_field(p4f, xgress.EGRESS)
                ecf.is_ohi = False
                ecf.is_intrinsic = False

            self.gress_pa[xgress.INGRESS].capri_deparser_len_hdr = self.dprsr_len_hdr
            self.gress_pa[xgress.EGRESS].capri_deparser_len_hdr = self.dprsr_len_hdr

        if self.dprsr_pad_hdr != None:
            for p4f in self.dprsr_pad_hdr.fields:
                cf = self.get_field(get_hfname(p4f), xgress.INGRESS)
                if not cf:
                    cf = self.allocate_field(p4f, xgress.INGRESS)
                cf.is_ohi = False
                cf.is_intrinsic = False

                ecf = self.get_field(get_hfname(p4f), xgress.EGRESS)
                if not ecf:
                    ecf = self.allocate_field(p4f, xgress.EGRESS)
                ecf.is_ohi = False
                ecf.is_intrinsic = False

            self.gress_pa[xgress.INGRESS].capri_deparser_pad_hdr = self.dprsr_pad_hdr
            self.gress_pa[xgress.EGRESS].capri_deparser_pad_hdr = self.dprsr_pad_hdr

        if self.ig_gso_csum_hdr != None:
            for p4f in self.ig_gso_csum_hdr.fields:
                cf = self.get_field(get_hfname(p4f), xgress.INGRESS)
                if not cf:
                    cf = self.allocate_field(p4f, xgress.INGRESS)
                cf.is_ohi = False
                cf.is_intrinsic = False
                ecf = self.get_field(get_hfname(p4f), xgress.EGRESS)
                if not ecf:
                    ecf = self.allocate_field(p4f, xgress.EGRESS)
                ecf.is_ohi = False
                ecf.is_intrinsic = False
            self.gress_pa[xgress.INGRESS].capri_gso_csum_hdr = self.ig_gso_csum_hdr

        if self.eg_gso_csum_hdr != None:
            for p4f in self.eg_gso_csum_hdr.fields:
                cf = self.get_field(get_hfname(p4f), xgress.INGRESS)
                if not cf:
                    cf = self.allocate_field(p4f, xgress.INGRESS)
                cf.is_ohi = False
                cf.is_intrinsic = False
                ecf = self.get_field(get_hfname(p4f), xgress.EGRESS)
                if not ecf:
                    ecf = self.allocate_field(p4f, xgress.EGRESS)
                ecf.is_ohi = False
                ecf.is_intrinsic = False
            self.gress_pa[xgress.EGRESS].capri_gso_csum_hdr = self.eg_gso_csum_hdr

        # Add all the p4 fields to respective phv allocators based on direction
        egress_i2e_fields = []
        for f in self.be.h.p4_fields.values():
            if f.offset == None:
                ncc_assert(f.name == 'valid')
                # capri fields for all header valid bits are already created
                continue
            is_scratch = False
            if f.instance and f.instance.metadata:
                meta_hdr = f.instance
                is_scratch = True if 'scratch_metadata' in meta_hdr._parsed_pragmas else False
                if is_parser_local_header(meta_hdr):
                    # create local field in both direction
                    _ = self.gress_pa[xgress.INGRESS].allocate_field(f)
                    _ = self.gress_pa[xgress.EGRESS].allocate_field(f)
                    continue
            if self.dprsr_len_hdr and f.instance.name == self.dprsr_len_hdr.name:
                continue
            if self.dprsr_pad_hdr and f.instance.name == self.dprsr_pad_hdr.name:
                continue
            if self.ig_gso_csum_hdr and f.instance.name == self.ig_gso_csum_hdr.name:
                continue
            if self.eg_gso_csum_hdr and f.instance.name == self.eg_gso_csum_hdr.name:
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
                    ncc_assert(not cfi.parser_local, \
                        "Cannot used parser local field %s in ingress pipeline" % \
                        (cfi.hfname))
                if cfi and 'parser_end_offset' in f.instance._parsed_pragmas and \
                    f.name == f.instance._parsed_pragmas['parser_end_offset'].keys()[0]:
                    cfi.is_parser_end_offset = True
                    ncc_assert(self.gress_pa[xgress.INGRESS].parser_end_off_cf == None)
                    self.gress_pa[xgress.INGRESS].parser_end_off_cf = cfi
            if (f.egress_read or f.egress_write):
                cfe = self.gress_pa[xgress.EGRESS].allocate_field(f)
                if cfe and is_scratch:
                    self.logger.debug('EGRESS:%s is scratch field' % (cfe.hfname))
                    cfe.is_scratch = True
                    ncc_assert(not cfe.parser_local, \
                        "Cannot used parser local field %s in egress pipeline" % \
                        (cfe.hfname))
                if cfe and 'parser_end_offset' in f.instance._parsed_pragmas and \
                    f.name == f.instance._parsed_pragmas['parser_end_offset'].keys()[0]:
                    cfe.is_parser_end_offset = True
                    ncc_assert(self.gress_pa[xgress.EGRESS].parser_end_off_cf == None)
                    self.gress_pa[xgress.EGRESS].parser_end_off_cf = cfe

            # create all header fields.. if direction is not known, do it in both dir
            # this is done so that hdr and fleld unions can be processed before parser
            # initialization. This change is done so that parser topo sorting can use
            # hdr/fld union info to create fake branches between states that extract
            # same meta fld or extract unioned headers
            if not f.instance.metadata:
                if not cfi:
                    cfi = self.gress_pa[xgress.INGRESS].allocate_field(f)
                if not cfe:
                    cfe = self.gress_pa[xgress.EGRESS].allocate_field(f)
            if self.be.args.p4_plus and not cfi and f.instance.name != 'standard_metadata':
                # add all fields to ingress for p4_plus program
                cfi = self.gress_pa[xgress.INGRESS].allocate_field(f)
                if cfi and is_scratch:
                    self.logger.debug('INGRESS:%s is scratch field' % (cfi.hfname))
                    cfi.is_scratch = True
                    ncc_assert(not cfi.parser_local, \
                        "Cannot used parser local field %s in ingress pipeline" % \
                        (cfi.hfname))

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
                ncc_assert(cf.width < ecf.width)
                pad = ecf.width - cf.width
                cf.width = ecf.width
                cf.pad = pad
                cf.p4_fld.width = cf.width

    def hdr_union_add(self, hdr_list, d):
        # support multiple definitions (transitive) of header unions for a given header
        unioned_hdrs = set([])
        unioned_hdrs |= set(hdr_list)
        for uh in hdr_list:
            if uh in self.gress_pa[d].hdr_unions:
                unioned_hdrs |= set(self.gress_pa[d].hdr_unions[uh][1])
        for uh in list(unioned_hdrs):
            self.gress_pa[d].hdr_unions[uh] = (d, list(unioned_hdrs), None)

    def initialize_unions(self):
        # Read p4 header info find header uninons specified by pa_header_union
        # and pa_filed_union pragmas
        # TBD - implement an efficient algo to find mutually exclusing segments
        # of the parse graph to find out headers that can be put into a union
        for name, hdr in self.be.h.p4_header_instances.items():
            if len(hdr._parsed_pragmas) == 0:
                continue
            if 'pa_header_union' in hdr._parsed_pragmas:
                ncc_assert(len(hdr._parsed_pragmas['pa_header_union']))
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
                            if uh not in self.be.h.p4_header_instances or \
                                is_scratch_header(self.be.h.p4_header_instances[uh]):
                                ncc_assert(0, "union header %s not defined or is scratch header" % (uh))
                            n_hdr = self.be.h.p4_header_instances[uh]
                            ncc_assert(n_hdr)
                            if n_hdr not in hdr_list:
                                hdr_list.append(n_hdr)
                            prg = prg[uh]

                        if hdr not in hdr_list:
                            hdr_list.append(hdr)
                        self.logger.debug("Header union %s->%s" % (name, hdr_list))
                        if direction.upper() == 'INGRESS' or direction.upper() == 'XGRESS':
                            self.hdr_union_add(hdr_list, xgress.INGRESS)
                        if direction.upper() == 'EGRESS' or direction.upper() == 'XGRESS':
                            self.hdr_union_add(hdr_list, xgress.EGRESS)
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
                ncc_assert(len(hdr._parsed_pragmas['pa_field_union']))
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

    def init_synthetic_headers(self):
        for d in xgress:
            self.gress_pa[d].init_synthetic_headers()

    def allocate_field(self, f, d):
        return self.gress_pa[d].allocate_field(f)

    def get_field(self, hfname, d):
        # return capri_field object for a given header.field name
        return self.gress_pa[d].get_field(hfname)

    def allocate_hv_field(self, hdr_name, d):
        return self.gress_pa[d].allocate_hv_field(hdr_name)

    def allocate_hv_truncate_field(self, hdr_name, d):
        return self.gress_pa[d].allocate_hv_truncate_field(hdr_name)

    def allocate_hv_padhdr_len_field(self, hdr_name, d):
        return self.gress_pa[d].allocate_hv_padhdr_len_field(hdr_name)

    def allocate_hv_payload_len_field(self, hdr_name, d):
        return self.gress_pa[d].allocate_hv_payload_len_field(hdr_name)

    def allocate_csum_hv_field(self, hdr_name, d):
        return self.gress_pa[d].allocate_csum_hv_field(hdr_name)

    def allocate_icrc_hv_field(self, hdr_name, d):
        return self.gress_pa[d].allocate_icrc_hv_field(hdr_name)

    def allocate_gso_csum_hv_field(self, hdr_name, d):
        return self.gress_pa[d].allocate_gso_csum_hv_field(hdr_name)

    def create_flits(self):
        # create flits from the field_order and assign phv bits
        for d in xgress:
            add_hv=True
            if self.be.args.p4_plus:
                add_hv=False
            self.gress_pa[d].create_flits(add_hv)
            #total_phv_bits = self.gress_pa[d].print_field_order_info("PHV order(Post Flit)")
            if self.be.args.old_phv_allocation:
                self.gress_pa[d].assign_phv()
            new_phv_bits = self.gress_pa[d].print_field_order_info("PHV order(Post relocate Meta)")
            #ncc_assert(total_phv_bits == new_phv_bits)
            self.logger.debug("%s" % sorted([cf for cf in self.gress_pa[d].field_order \
                if not cf.is_ohi and not cf.is_flit_pad], key=lambda k: k.phv_bit))

    def replace_hv_field(self, fidx, cf, d):
        return self.gress_pa[d].replace_hv_field(fidx, cf)

    def init_field_ordering(self):
        for d in xgress:
            self.gress_pa[d].init_field_ordering()

    def get_header_cfields(self, hdr, d):
        return self.gress_pa[d].get_header_cfields(hdr)

    def get_header_all_cfields(self, hdr, d):
        return self.gress_pa[d].get_header_all_cfields(hdr)

    def get_phv_chunks(self, fl, d, check_ohi=False):
        return self.gress_pa[d].get_phv_chunks(fl, check_ohi)

    def get_hdr_phv_chunks(self, fl, d, check_ohi=True):
        return self.gress_pa[d].get_hdr_phv_chunks(fl, check_ohi)

    def get_hdr_phv_start(self, hdr, d):
        return self.gress_pa[d].get_hdr_phv_start(hdr)

    def capri_asm_output(self):
        for d in xgress:
            capri_asm_output_pa(self.gress_pa[d])
            capri_cstruct_output_pa(self.gress_pa[d])

    def phv_dbg_output(self):
        # create a dictionary and write it to a json file
        phv_debug_info = OrderedDict()
        for d in xgress:
            phv_debug_info[d.name] = self.gress_pa[d].phv_dbg_output()
        return phv_debug_info
