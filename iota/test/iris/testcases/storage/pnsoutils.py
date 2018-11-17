#! /usr/bin/python3
import os
import pdb
import fileinput

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
    api.Logger.info("Processing yml file : %s" % filename)
    for k,v in tc.iterators.__dict__.items():
        __search_replace(filename, k, str(v))
    for k,v in tc.args.__dict__.items():
        __search_replace(filename, k, str(v))
    return

def __prepare_ymls(tc):
    os.system("rm -rf %s" % tc.tmpdir)
    os.system("mkdir %s" % tc.tmpdir)
    
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

def __get_blocksize(tc):
    if getattr(tc.args, 'blocksize', None) is not None:
        return getattr(tc.args, 'blocksize')
    if getattr(tc.iterators, 'blocksize', None) is not None:
        return getattr(tc.iterators, 'blocksize')
    return pnsodefs.PNSO_TEST_DEFAULT_BLOCKSIZE

def __get_batch_depth(tc):
    if getattr(tc.args, 'batch_depth', None) is not None:
        return getattr(tc.args, 'batch_depth')
    if getattr(tc.iterators, 'batch_depth', None) is not None:
        return getattr(tc.iterators, 'batch_depth')
    return pnsodefs.PNSO_TEST_DEFAULT_BLOCKSIZE

def __setup_default_params(tc):
    def __set(tc, key, value):
        if getattr(tc.args, key, None) is None and\
           getattr(tc.iterators, key, None) is None:
            setattr(tc.args, key, value)
        return
    __set(tc, 'repeat', pnsodefs.PNSO_TEST_DEFAULT_REPEAT*__get_batch_depth(tc)) 
    __set(tc, 'key1', pnsodefs.PNSO_TEST_DEFAULT_KEY1)
    __set(tc, 'key2', pnsodefs.PNSO_TEST_DEFAULT_KEY2)
    __set(tc, 'wait', pnsodefs.PNSO_TEST_DEFAULT_WAIT)
    __set(tc, 'pcqdepth', pnsodefs.PNSO_TEST_DEFAULT_PCQDEPTH)
    __set(tc, 'batch_depth', pnsodefs.PNSO_TEST_DEFAULT_BATCH_DEPTH)
    return


def Setup(tc):
    __setup_default_params(tc)

    tc.files = []
    tc.tcdir = "%s/%s" % (api.GetTopDir(), pnsodefs.PNSO_TCDIR)
    tc.tmpdir = "%s/%s" % (api.GetTopDir(), pnsodefs.TMPDIR)
    tc.ymldir = "%s/%s" % (api.GetTopDir(), pnsodefs.YMLDIR)

    blocksize = __get_blocksize(tc)
    tc.args.x2blocksize = blocksize * 2
    tc.args.x4blocksize = blocksize * 4
    tc.args.x8blocksize = blocksize * 8
    tc.args.x16blocksize = blocksize * 16
    __prepare_ymls(tc)
    
    tc.files.append(tc.blocksize_yml)
    tc.files.append(tc.globals_yml)
    tc.files.append(tc.test_yml)
    return
