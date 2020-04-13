#!/usr/bin/python3

import os
import re
import argparse
from trace import decode_mpu_trace_file, decode_instruction, MpuTraceHeader
from syms import *
from itertools import product
from common import get_bits, reverse_bytes
from collections import defaultdict


parser = argparse.ArgumentParser(prog='captrace')

subparsers = parser.add_subparsers(dest='command')

syms_parser = subparsers.add_parser('gen_syms', help='Generate symbol file')
syms_parser.add_argument('--sym_file', default='captrace.syms', help='Output symbol filepath')
syms_parser.add_argument('--pipeline', help='Pipeline',
    default='iris',
    choices=['iris', 'apollo', 'apulu', 'artemis', 'athena', 'gft'])
syms_parser.add_argument('--asic', help='Asic',
    default='capri',
    choices=['capri', 'elba'])

phv_parser = subparsers.add_parser('phv', help='Parse P-vector')
phv_parser.add_argument('program', help='Program name')
phv_parser.add_argument('vec', type=lambda x: int(x, 16), help='P-vector')
phv_parser.add_argument('--swizzle', action="store_true")
phv_parser.add_argument('--sym', default='captrace.syms', help="Symbol file")
phv_parser.add_argument('--load', default='mpu_prog_info.json', help="Loader file")

key_parser = subparsers.add_parser('key', help='Parse K-vector')
key_parser.add_argument('program', help='Program name')
key_parser.add_argument('vec', type=lambda x: int(x, 16), help='K-vector')
key_parser.add_argument('--sym', default='captrace.syms', help="Symbol file")
key_parser.add_argument('--load', default='mpu_prog_info.json', help="Loader file")

data_parser = subparsers.add_parser('data', help='Parse D-vector')
data_parser.add_argument('program', help='Program name')
data_parser.add_argument('vec', type=lambda x: int(x, 16), help='D-vector')
data_parser.add_argument('--swizzle', action="store_true")
data_parser.add_argument('--sym', default='captrace.syms', help="Symbol file")
data_parser.add_argument('--load', default='mpu_prog_info.json', help="Loader file")

file_parser = subparsers.add_parser('decode', help='Parse Trace file')
file_parser.add_argument('file', help='Trace file path')
file_parser.add_argument('--sym', default='captrace.syms', help="Symbol file")
file_parser.add_argument('--load', default='mpu_prog_info.json', help="Loader file")
file_parser.add_argument('--fltr', nargs='+', default=list(), help='Header filters')

args = parser.parse_args()

if args.command == "gen_syms":
    sym_dir = 'build/aarch64/{}/{}/out/'.format(args.pipeline, args.asic)
    if os.path.exists(sym_dir):
        create_symbol_file(sym_dir, sym_file=args.sym_file)
    else:
        raise IOError('Build directory does not exist! {}'.format(sym_dir))
elif args.command == "phv":
    if args.sym:
        load_symbol_file(args.sym)
    else:
        create_symbol_file()

    if args.load:
        create_loader_db(args.load)
    else:
        create_loader_db()

    if args.swizzle:
        pvec = reverse_bytes(args.vec)
    else:
        pvec = args.vec

    pgm = get_program_by_name(args.program)
    for name, fld in sorted(pgm["p_struct"].items(), key=lambda f: (f[1]["end"], f[1]["name"])):
        print("{:50}\t{:#x}".format(fld["name"], get_bits(pvec, fld["start"], fld["end"])))
elif args.command == "key":
    if args.sym:
        load_symbol_file(args.sym)
    else:
        create_symbol_file()

    if args.load:
        create_loader_db(args.load)
    else:
        create_loader_db()

    pgm = get_program_by_name(args.program)
    for name, fld in sorted(pgm["k_struct"].items(), key=lambda f: (f[1]["end"], f[1]["name"])):
        print("{:50}\t{:#x}".format(fld["name"], get_bits(args.vec, fld["start"], fld["end"])))
elif args.command == "data":
    if args.sym:
        load_symbol_file(args.sym)
    else:
        create_symbol_file()

    if args.load:
        create_loader_db(args.load)
    else:
        create_loader_db()

    if args.swizzle:
        dvec = reverse_bytes(args.vec)
    else:
        dvec = args.vec

    pgm = get_program_by_name(args.program)
    for name, fld in sorted(pgm["d_struct"].items(), key=lambda f: (f[1]["end"], f[1]["name"])):
        print("{:50}\td[{:3d}:{:3d}]\t{:#x}".format(fld["name"], fld["end"], fld["start"], get_bits(dvec, fld["start"], fld["end"])))
elif args.command == "decode":
    if args.sym:
        load_symbol_file(args.sym)
    else:
        create_symbol_file()

    if args.load:
        create_loader_db(args.load)
    else:
        create_loader_db()

    def hdr_sort_key(hdr):
        return hdr.mpu_processing_pkt_id_next, hdr.stg, hdr.timestamp

    with open(args.file, "rb") as f:

        trace = defaultdict(list)

        # Decode and filter the trace
        for fhdr, hdr, key, data, instructions in decode_mpu_trace_file(f.read()):

            # Apply filters
            if not all(getattr(hdr, k) == int(v, 16) for k, v in [tuple(x.split('=')) for x in args.fltr]):
                continue

            # Save trace entry
            trace[hdr.phv_timestamp_capture].append((fhdr, hdr, key, data, instructions))

        # Print the trace
        for ts in sorted(trace):

            print()
            print("=== PHV ===")
            print()

            for fhdr, hdr, key, data, instructions in sorted(trace[ts], key=lambda x: hdr_sort_key(x[1])):

                # Header
                print("\n>>> HDR : 0x{:0128x}\n".format(int.from_bytes(hdr, byteorder='big')))
                for fld in sorted(hdr._fields_, key=lambda x: x[0]):
                    if not fld[0].startswith('_'):
                        print("{:50} {:#x}".format(fld[0], getattr(hdr, fld[0])))

                pgm = get_program(hdr.entry_pc << 6)
                label = get_label(hdr.entry_pc << 6)

                pgm = pgm["name"]
                label = label["name"]
                print("\n>>> PROGRAM : pc 0x{:010x} program '{:}' label '{:}'\n".format(
                    hdr.entry_pc << 6, pgm, label))
                prev_time = hdr.timestamp

                if key:
                    # Decode key
                    print("\n>>> KEY : 0x{:0128x}\n".format(key))
                    for name, val in decode_key(hdr.entry_pc << 6, key):
                        print("{:50} {:#x}".format(name, val))

                if data:
                    # Decode data
                    print("\n>>> DATA : 0x{:0128x}\n".format(data))
                    for name, val in decode_data(hdr.entry_pc << 6, data):
                        print("{:50} {:#x}".format(name, val))

                if instructions:
                    end = max([x.timestamp for x in instructions])

                    for instr in instructions:
                        instr_pgm = get_program(instr.ex_pc << 3)
                        instr_label = get_label(instr.ex_pc << 3)

                        # Did the program or label change?
                        if instr_pgm != pgm or instr_label != label:
                            pgm_changed = instr_pgm != pgm
                            pgm = instr_pgm
                            label = instr_label
                            print("\n>>> BRANCH : pc 0x{:010x} program '{:}' label '{:}'\n".format
                                  (instr.ex_pc << 3, pgm["name"], label["name"]))
                            # Did the program change?
                            if pgm_changed:
                                # Decode key
                                print("\n>>> KEY : 0x{:0128x}\n".format(key))
                                for name, val in decode_key(instr.ex_pc << 3, key):
                                    print("{:50} {:#x}".format(name, val))
                                # Decode data
                                print("\n>>> DATA : 0x{:0128x}\n".format(data))
                                for name, val in decode_data(instr.ex_pc << 3, data):
                                    print("{:50} {:#x}".format(name, val))

                        instr_info = "[{:3d}]: {:3d}: {:+3d}: {:09x}: {:016x}  {:}".format(
                            instr.inst_count,
                            instr.timestamp - hdr.timestamp,
                            instr.timestamp - prev_time,
                            instr.ex_pc << 3,
                            instr.ex_inst,
                            "X" if instr.ex_predicate == 0 else " ")

                        instr_txt = decode_instruction(instr.ex_pc << 3, instr.ex_inst)
                        instr_variants = {instr_txt}

                        p_xfrm = get_phv_field_names(instr.ex_pc << 3, instr_txt)
                        k_xfrm = get_key_field_names(instr.ex_pc << 3, instr_txt)
                        d_xfrm = get_data_field_names(instr.ex_pc << 3, instr_txt)

                        d_xfrms = product(*(d_xfrm[x] for x in d_xfrm))
                        k_xfrms = product(*(k_xfrm[x] for x in k_xfrm))
                        p_xfrms = product(*(p_xfrm[x] for x in p_xfrm))

                        for xfrms_p, xfrms_k, xfrms_d in product(p_xfrms, k_xfrms, d_xfrms):
                            instr_var = instr_txt
                            for d, xfrm_d in xfrms_d:
                                instr_var = instr_var.replace(d, xfrm_d)
                            for k, xfrm_k in xfrms_k:
                                instr_var = instr_var.replace(k, xfrm_k)
                            for p, xfrm_p in xfrms_p:
                                instr_var = instr_var.replace(p, xfrm_p)
                            instr_variants.add(instr_var)

                        for instr_inst in instr_variants:
                            print("{} {}".format(
                                instr_info,
                                instr_inst,
                            ))

                        print("# ALU(0x{:x}, 0x{:x}, 0x{:x}) = 0x{:x}".format(
                            instr.alu_src1,
                            instr.alu_src2,
                            instr.alu_src3,
                            instr.debug_rdst,
                        ))

                        print()

                        prev_time = instr.timestamp
