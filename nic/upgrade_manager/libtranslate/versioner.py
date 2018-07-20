import argparse
import json
import os
import shutil
import sys

from hparser import *

#
# The purpose of this tool is to keep track for changes in the structures used 
# by the q-state tables. Each q-state table is described using a .json file 
# that looks something like this:
# {
#    "name": "<Q-STATE-TABLE-NAME>"
#    "input_dirs": ["<SUBDIR1>", "<SUBDIR2>", ...]
#    "entries": [{
#        "name": "<FIELD_NAME1>",
#        "struct": "<C_STRUCT_NAME2>"
#    }...]
# }
# 
# SUBDIR are sub-directories under the "gen" folder that are generate by NCC
#
# The output of the script is all under the "versions" directory.
# For each q-state-table and version we generate a directory like this:
# versions/<Q-STATE-TABLE-NAME/VERSION
#
# There we keep:
# 1. A copy of the json spec file
# 2. Copies of all the contnets of the "SUBDIRS"
# 3. A .h file that we generate after parsing all the subdirs, that includes all
#       the NCC generated C structs.
#
# The script has the following subcommands:
# 1. new_version: This creates a new archived version for the given spec file
# 2. compare: compares the current generated code version with the latest 
#       archived version for a given spec file
#


# Loads and parses the json spec file find in "filename"
def load_spec(filename):
    with open(filename, 'r') as f:
        return json.loads(f.read())

# For a given spec, find the latest version we have archived and load and 
# return the spec of that version
def load_latest_version(spec):
    latest = 0
    d = os.path.join('versions', spec['name'])
    if not os.path.isdir(d):
        return None
    for dirname in os.listdir(d):
        ver = int(dirname)
        if ver > latest:
            latest = ver
    if latest == 0:
        return None
    spec = load_spec(os.path.join(d, str(latest), 'spec.json'))
    spec['version'] = latest
    spec['base_dir'] = os.path.join(d, str(latest))
    return spec

# Convert the spec to json and save it
def save_spec(path, spec):
    with open(os.path.join(path, 'spec.json'), 'w') as f:
        f.write(json.dumps(spec))

# Used to copy the source files to the archived version
def copy_files(path, spec):
    for dirname in spec['input_dirs']:
        src_dir = os.path.join('../../gen', dirname)
        dst_dir = os.path.join(path, dirname)
        os.makedirs(dst_dir)
        for filename in os.listdir(src_dir):
            src = os.path.join(src_dir, filename)
            if src.endswith('.h'):
                shutil.copy(src, dst_dir)
                print("Copying %s to %s" % (src, dst_dir))

# Safely make all the dirs needed for a path
def mkdirs(path):
    try:
        os.makedirs(path)
    except OSError as e:
        if e.errno != errno.EEXIST:
            raise

# Create a new archived version
def create_v(spec, ver):
    path = os.path.join('versions', spec['name'], str(ver))
    mkdirs(path)
    save_spec(path, spec)
    copy_files(path, spec)

# Given a C Type compare the definition between the two versions
def compare_def(current, latest, ctype):
    print('Checking %s' % (ctype))
    cur_ctype = current[ctype]
    lst_ctype = latest[ctype]
    cur_ctype.comp(lst_ctype)

def get_deps_field(f):
    if f._ctype._embedded:
        return get_deps_compound(f._ctype._embedded)
    else:
        return [f._ctype._name]

def get_deps_compound(c):
    deps = []
    if isinstance(c, CCompound):
        for f in c._fields:
            deps += get_deps_field(f)
    return deps

def get_deps_typedef(c):
    if c._ctype:
        return get_deps_compound(c._ctype)
    else:
        return [c._ctype._name]

def generate(pr, c):
    pr.newline()
    c.regen(pr)

def get_deps(c):
    if isinstance(c, CCompound):
        return get_deps_compound(c)
    if isinstance(c, CTypedef):
        return get_deps_typedef(c)

def recreate_top(pr, spec):
    pr.add('typedef struct __attribute__((__packed__)) {')
    pr.incr()
    for e in spec['entries']:
        pr.newline()
        if e['struct']:
            pr.add('%s %s;' % (e['struct'], e['name'].lower()))
        else:
            pr.add('uint64_t %s[8];' % (e['name'].lower()))
    pr.decr()
    pr.newline()
    pr.add('} %s_t;' % (spec['name']))

# This funciton is responsible for regenarating C code for definitions of the
# files we have parsed.
#
# Todo: Fixme: It needs breaking down and refactoring
def recreate(spec, dasts, namespace):
    l = {}
    for ast in dasts:
        for t in ast:
            if isinstance(t, CCompound) or isinstance(t, CEnum) or isinstance(t, CTypedef):
                l[t._name] = t
    deps = {}
    for k in l:
        if k in deps:
            raise Exception(k)
        deps[k] = {d: True for d in get_deps(l[k])}
    rdeps = {}
    for k in deps:
        for d in deps[k]:
            if d not in rdeps:
                rdeps[d] = {}
            rdeps[d][k] = True

    for t in ['uint8_t', 'uint16_t', 'uint32_t', 'uint64_t']:
        for k in rdeps[t]:
            deps[k].pop(t, None)
        rdeps.pop(t)

    pr = Printer()
    pr.add('#include <inttypes.h>')
    pr.newline()
    pr.newline()
    pr.add('namespace %s {' % (namespace))
    pr.incr()
    pr.newline()
    while len(deps):
        processed = []
        for k in deps:
            if len(deps[k]) == 0:
                generate(pr, l[k])
                processed.append(k)
                if k in rdeps:
                    for d in rdeps[k]:
                        deps[d].pop(k, None)
        if len(processed) == 0:
            raise Exception('Loop in dependencies?')
        for p in processed:
            deps.pop(p, None)
    pr.newline()
    recreate_top(pr, spec)
    pr.decr()
    pr.newline()
    pr.add('};')
    pr.newline()
    return pr

def save_defs(spec, version, pr):
    dirname = os.path.join('versions', spec['name'], str(version), 'include')
    mkdirs(dirname)
    filename = os.path.join(dirname, 'defs.h')
    with open(filename, 'w') as f:
        f.write(pr._buffer)

def new_version(opts):
    filename = opts.spec
    spec = load_spec(filename)
    if not spec:
        sys.exit(-1)
    latest = load_latest_version(spec)
    version = latest['version'] + 1 if latest else 1
    print('Creating version %d...' % version)
    create_v(spec, version)
    latest = load_latest_version(spec)
    print('Backup created, parsing files...')
    (_, asts) = parse_dirs(latest['base_dir'], latest['input_dirs'])
    print('Parsed files, generating consolidated .h...')
    namespace = ('%s_v%s' % (latest['name'], version))
    pr = recreate(spec, asts, namespace)
    save_defs(spec, version, pr)
    print('Done')
    
def compare(opts):
    filename = opts.spec
    spec = load_spec(filename)

    latest = load_latest_version(spec)
    if not latest:
        print('No previous version for %s' % (spec['name']))

    print('Comparing with version %s' % (latest['version']))
    if len(spec['entries']) != len(latest['entries']):
        print('Entry count different')
        return

    for i in range(0, len(spec['entries'])):
        if spec['entries'][i] != latest['entries'][i]:
            print('Entry name is different: %s %s' % 
                (spec['entries'][i], latest['entries'][i]))

    (cdefs, _) = parse_dirs('../../gen', spec['input_dirs'])
    (ldefs, _) = parse_dirs(latest['base_dir'], latest['input_dirs'])

    for e in spec['entries']:
        if not e['struct']:
            continue
        compare_def(cdefs, ldefs, e['struct'])

def main():
    commands = {
        'compare': compare,
        'new_version': new_version,
    }
    parser = argparse.ArgumentParser()
    subparsers = parser.add_subparsers(dest='subcommand')
    new_version_parser = subparsers.add_parser('new_version')
    new_version_parser.add_argument('spec', metavar='filename', type=str,
        help='The JSON file with the table spec')
    compare_parser = subparsers.add_parser('compare')
    compare_parser.add_argument('spec', metavar='filename', type=str,
        help='The JSON file with the table spec')

    args = parser.parse_args()
    
    commands[args.subcommand](args)

if __name__ == '__main__':
    main()
