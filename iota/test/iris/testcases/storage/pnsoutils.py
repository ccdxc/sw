#! /usr/bin/python3
import os
import pdb
import fileinput
import tempfile

import iota.harness.api as api
import iota.test.iris.testcases.storage.pnsodefs as pnsodefs

def __search_replace(filename, key, replace):
    search = "param::%s" % key
    api.Logger.debug("Search:%s Replace:%s" % (search, replace))

    f = fileinput.FileInput(filename, inplace=True)
    for line in f:
        print(line.replace(search, replace), end='') 
    f.close()
    return

def __process_file(tc, filename):
    api.Logger.debug("Processing yml file : %s" % filename)
    for k,v in tc.iterators.__dict__.items():
        __search_replace(filename, k, str(v))
    for k,v in tc.args.__dict__.items():
        __search_replace(filename, k, str(v))
    return

def __prepare_ymls(tc):
    os.system("rm -rf %s" % tc.tmpdir)
    os.system("mkdir -p %s" % tc.tmpdir)
    
    os.system("cp %s/blocksize.yml %s/" % (tc.ymldir, tc.tmpdir))
    os.system("cp %s/globals.yml %s/" % (tc.ymldir, tc.tmpdir))
    os.system("cp %s/%s %s/" % (tc.ymldir, tc.args.test, tc.tmpdir))

    tc.blocksize_yml = "%s/blocksize.yml" % tc.tmpdir
    __process_file(tc, tc.blocksize_yml)
    
    tc.globals_yml = "%s/globals.yml" % tc.tmpdir
    __process_file(tc, tc.globals_yml)

    tc.test_yml = "%s/%s" % (tc.tmpdir, tc.args.test)
    __process_file(tc, tc.test_yml)

    return

def __get_param(tc, name, default):
    if getattr(tc.args, name, None) is not None:
        return getattr(tc.args, name)
    if getattr(tc.iterators, name, None) is not None:
        return getattr(tc.iterators, name)
    return default

def __setup_default_params(tc):
    def __set(tc, key, value):
        if getattr(tc.args, key, None) is None and\
           getattr(tc.iterators, key, None) is None:
            setattr(tc.args, key, value)
        return

    batch_depth = __get_param(tc, 'batch_depth', pnsodefs.PNSO_TEST_DEFAULT_BATCH_DEPTH)
    __set(tc, 'repeat', pnsodefs.PNSO_TEST_DEFAULT_REPEAT* batch_depth)

    __set(tc, 'key1', pnsodefs.PNSO_TEST_DEFAULT_KEY1)
    __set(tc, 'key2', pnsodefs.PNSO_TEST_DEFAULT_KEY2)
    __set(tc, 'wait', str(pnsodefs.PNSO_TEST_DEFAULT_WAIT))
    __set(tc, 'pcqdepth', pnsodefs.PNSO_TEST_DEFAULT_PCQDEPTH)
    __set(tc, 'batch_depth', pnsodefs.PNSO_TEST_DEFAULT_BATCH_DEPTH)
    __set(tc, 'mode', pnsodefs.PNSO_TEST_DEFAULT_MODE)

    num_cpus = __get_param(tc, 'cpus', pnsodefs.PNSO_TEST_DEFAULT_NUM_CPUS)
    max_cpus = api.GetTestsuiteAttr(pnsodefs.PNSO_TEST_MAXCPUS_ATTR)
    num_cpus = min(num_cpus, max_cpus)
    __set(tc, 'cpumask', (1 << num_cpus) - 1)
    return


def Setup(tc):
    __setup_default_params(tc)

    tc.files = []
    tc.tcdir = "%s/%s" % (api.GetTopDir(), pnsodefs.PNSO_TCDIR)
    tc.tmpdir = "/tmp/%s_%d" % (os.environ["USER"], os.getpid())
    tc.ymldir = "%s/%s" % (api.GetTopDir(), pnsodefs.YMLDIR)

    inputlen = __get_param(tc, 'inputlen', pnsodefs.PNSO_TEST_DEFAULT_INPUT_LEN)
    tc.args.x2inputlen = inputlen * 2
    tc.args.x4inputlen = inputlen * 4
    tc.args.x8inputlen = inputlen * 8
    tc.args.x16inputlen = inputlen * 16
   
    flags = __get_param(tc, 'flags', '0')
    if tc.args.test == 'hash.yml':
        if flags == '0':
            tc.args.hash_compare_val1x = 64
            tc.args.hash_compare_val2x = 64
            tc.args.hash_compare_val16x = 64
        else:
            tc.args.hash_compare_val1x = 64
            tc.args.hash_compare_val2x = 128
            tc.args.hash_compare_val16x = 1024

    if tc.args.test == 'chksum.yml':
        if flags == '0':
            tc.args.chksum_compare_val1x = 8
            tc.args.chksum_compare_val2x = 8
        else:
            tc.args.chksum_compare_val1x = 8
            tc.args.chksum_compare_val2x = 16

    __prepare_ymls(tc)
    tc.files.append(tc.blocksize_yml)
    tc.files.append(tc.globals_yml)
    tc.files.append(tc.test_yml)
    return
