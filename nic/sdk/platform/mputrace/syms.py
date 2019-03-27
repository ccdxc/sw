#!/usr/bin/python3

import os
import re
import json
import csv
from collections import defaultdict
from common import get_bits


NoneProgram = dict(name='',
                   start_pc=0, end_pc=0,
                   labels=dict(),
                   p_struct=dict(),
                   k_struct=dict(),
                   d_struct=dict())
NoneLabel = dict(name='', at_pc=0)


def parse_loader_conf(loader_conf):
    with open(loader_conf, 'r') as f:
        data = json.load(f)
        for p in data['programs']:
            pgm_or_label = p['name']
            start_pc = p['base_addr']
            end_pc = p['end_addr']
            yield pgm_or_label, int(start_pc), int(end_pc)
            for s in p['symbols']:
                pgm_or_label = s['name']
                s_pc = int(start_pc) + int(s['addr'])
                yield pgm_or_label + '.LAB', s_pc, s_pc


def create_loader_db(loader_file):
    # Programs
    for pgm_or_label, start_pc, end_pc in parse_loader_conf(loader_file):
        if re.match(r'\S+\.bin', pgm_or_label):
            if pgm_or_label not in config["programs"]:
                # For programs with no symbol file
                config["programs"][pgm_or_label] = dict(name=pgm_or_label,
                                                        start_pc=start_pc,
                                                        end_pc=end_pc,
                                                        labels=dict(),
                                                        p_struct=dict(),
                                                        k_struct=dict(),
                                                        d_struct=dict())
            else:
                pgm = config["programs"][pgm_or_label]
                config["programs"][pgm_or_label] = dict(name=pgm_or_label,
                                                        start_pc=start_pc,
                                                        end_pc=end_pc,
                                                        labels=dict(),
                                                        p_struct=pgm["p_struct"],
                                                        k_struct=pgm["k_struct"],
                                                        d_struct=pgm["d_struct"])
        elif re.match(r'\S+\.LAB', pgm_or_label):
            pgm = get_program(start_pc)
            pgm["labels"][pgm_or_label] = dict(name=pgm_or_label, at_pc=start_pc)
        else:
            raise NotImplementedError("%s is not a program or label!", pgm_or_label)


def create_symbol_file(sym_dir, sym_file):
    if os.path.basename(os.getcwd()) != 'nic':
        raise RuntimeError('Please run this program from the nic directory')

    global config
    config = dict(programs=dict())

    fld_pattern = re.compile(r'\s*\[(?P<end>\d+):(?P<start>\d+)\]'
                             r'\s*(?P<fld_name>[a-zA-Z0-9_.]+)'
                             r'\s*:\s*'
                             r'(?P<fld_width>\d+)'
                             r'([^;]*);\n')

    for (root, dirs, files) in os.walk(sym_dir):
        for filename in files:
            if filename.endswith('.sym'):
                with open(os.path.join(root, filename), 'r') as f:
                    pgm_key = filename.replace('.sym', '.bin')
                    pgm = dict(name=pgm_key,
                               start_pc=0,
                               end_pc=0,
                               labels=dict(),
                               p_struct=dict(),
                               k_struct=dict(),
                               d_struct=dict())
                    config["programs"][pgm_key] = pgm
                    txt = f.read()

                    # Create p_struct field map
                    p = re.search(r'p = (?P<p_struct>{[^}]+})', txt)
                    if p is not None:
                        for fld in re.finditer(fld_pattern, p.groupdict()['p_struct']):
                            fld_info = fld.groupdict()
                            fld_name, start, end = fld_info['fld_name'], int(fld_info['start']), int(fld_info['end'])
                            pgm["p_struct"][fld_name] = dict(name=fld_name, start=start, end=end)

                    # Create k_struct field map
                    k = re.search(r'k = (?P<k_struct>{[^}]+})', txt)
                    if k is not None:
                        for fld in re.finditer(fld_pattern, k.groupdict()['k_struct']):
                            fld_info = fld.groupdict()
                            fld_name, start, end = fld_info['fld_name'], int(fld_info['start']), int(fld_info['end'])
                            pgm["k_struct"][fld_name] = dict(name=fld_name, start=start, end=end)

                    # Create d_struct field map
                    d = re.search(r'd = (?P<d_struct>{[^}]+})', txt)
                    if d is not None:
                        for fld in re.finditer(fld_pattern, d.groupdict()['d_struct']):
                            fld_info = fld.groupdict()
                            fld_name, start, end = fld_info['fld_name'], int(fld_info['start']), int(fld_info['end'])
                            pgm["d_struct"][fld_name] = dict(name=fld_name, start=start, end=end)

    with open(sym_file, 'w') as f:
        json.dump(config, f)
    print("Generated symbol file - " + os.path.dirname(__file__) + "/" + (sym_file))


def load_symbol_file(syms_file='captrace.syms'):
    with open(syms_file, 'r') as f:
        global config
        config = json.load(f)


def get_program_by_name(name):
    return config["programs"][name]


def get_program(pc):
    for pgm_key, pgm in config["programs"].items():
        if pgm["start_pc"] <= pc <= pgm["end_pc"]:
            return pgm
    return NoneProgram


def get_label(pc):
    pgm = get_program(pc)
    if pgm is NoneProgram:
        return NoneLabel

    if pgm["labels"]:
        # find nearest label before pc i.e. label with max negative distance
        dist_gen = [(k, pc - v["at_pc"]) for k, v in pgm["labels"].items() if pc >= v["at_pc"]]
        if dist_gen:
            label = max(dist_gen, key=lambda x: pc - x[1])[0]
            return pgm["labels"][label]
    return NoneLabel


def decode_phv(pc, phv):
    pgm = get_program(pc)
    for name, fld in sorted(pgm["p_struct"].items(), key=lambda f: (f[1]["start"], f[1]["name"])):
        yield fld["name"], get_bits(phv, fld["start"], fld["end"])


def decode_key(pc, key):
    try:
        pgm = get_program(pc)
    except ValueError:
        raise StopIteration
    for name, fld in sorted(pgm["k_struct"].items(), key=lambda f: (f[1]["start"], f[1]["name"])):
        yield fld["name"], get_bits(key, fld["start"], fld["end"])


def decode_data(pc, data):
    try:
        pgm = get_program(pc)
    except ValueError:
        raise StopIteration
    for name, fld in sorted(pgm["d_struct"].items(), key=lambda f: (f[1]["start"], f[1]["name"])):
        yield fld["name"], get_bits(data, fld["start"], fld["end"])


def __get_field_names(fields, start, end):
    fld_coll = list()
    for k, fld in sorted(fields, key=lambda f: (f[1]["start"], f[1]["name"])):
        if fld["start"] == start and fld["end"] == end:  # complete field
            yield fld["name"]
        elif fld["start"] >= start and fld["end"] <= end:  # sliced field
            fld_coll.append(fld["name"])
            if fld["end"] == end:
                yield '{}'.format('{%s}' % ','.join(fld_coll))
                fld_coll.clear()


def __parse_fields(instr_txt, pfx):
    for m in re.finditer(pfx + '\[(?P<end>\d+):(?P<start>\d+)\]', instr_txt):
        d = m.groupdict()
        s, e = int(d['start']), int(d['end'])
        x = pfx + '[%d:%d]' % (e, s)
        yield x, (s, e)

    for m in re.finditer(pfx + '\[(?P<end>\d+)\]', instr_txt):
        d = m.groupdict()
        s, e = int(d['end']), int(d['end'])
        x = pfx + '[%d]' % e
        yield x, (s, e)


def get_phv_field_names(pc, instr_txt):
    try:
        pgm = get_program(pc)
    except ValueError:
        return []

    xfrms = defaultdict(list)
    for x, (start, end) in __parse_fields(instr_txt, 'p'):
        for name in __get_field_names(pgm["p_struct"].items(), start, end):
            xfrms[x].append((x, 'p.' + name))

    return xfrms


def get_key_field_names(pc, instr_txt):
    try:
        pgm = get_program(pc)
    except ValueError:
        return []

    xfrms = defaultdict(list)
    for x, (start, end) in __parse_fields(instr_txt, 'k'):
        for name in __get_field_names(pgm["k_struct"].items(), start, end):
            xfrms[x].append((x, 'k.' + name))

    return xfrms


def get_data_field_names(pc, instr_txt):
    try:
        pgm = get_program(pc)
    except ValueError:
        return []

    xfrms = defaultdict(list)
    for x, (start, end) in __parse_fields(instr_txt, 'd'):
        for name in __get_field_names(pgm["d_struct"].items(), start, end):
            xfrms[x].append((x, 'd.' + name))

    return xfrms
