#! /usr/bin/python3
PNSO_DRIVER_DIR = 'storage-offload'
PNSO_PENCAKE_SUCCESS_MSG = "PenCAKE completed all default testcases, status 0"
PNSO_NUM_PENCAKE_TESTS = 6

PNSO_TCDIR  = "iota/test/iris/testcases/storage"
YMLDIR      = "%s/ymls" % PNSO_TCDIR
TMPDIR      = "%s/tmp" % PNSO_TCDIR

PNSO_TEST_DEFAULT_KEY1          = 'abcd1234ABCD1234abcd1234ABCD1234'
PNSO_TEST_DEFAULT_KEY2          = 'abcd1234ABCD1234abcd1234ABCD1234'
PNSO_TEST_DEFAULT_REPEAT        = 1
PNSO_TEST_DEFAULT_WAIT          = 2
PNSO_TEST_DEFAULT_PCQDEPTH      = 32
PNSO_TEST_DEFAULT_BLOCKSIZE     = 4096
PNSO_TEST_DEFAULT_BATCH_DEPTH   = 1
PNSO_TEST_DEFAULT_MODE          = 'sync'
PNSO_TEST_DEFAULT_NUM_CPUS      = 1
PNSO_TEST_MAXCPUS_ATTR          = 'PNSO_MAXCPUS'
