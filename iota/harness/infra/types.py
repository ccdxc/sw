#! /usr/bin/python3
import iota.harness.infra.utils.utils   as utils

__status = [
    'SUCCESS',
    'FAILURE',
    'INVALID_ARG',
    'RETRY',
    'UNAVAIL',
    'IGNORED',
    'ABORTED',
    'ERROR',
]
status = utils.List2Enum(__status)

__level = {
    0: 'NONE',
    1: 'CRITICAL',
    2: 'ERROR',
    3: 'WARNING',
    4: 'INFO',
    5: 'DEBUG',
    6: 'VERBOSE',
    7: 'MAX'
}
loglevel = utils.Dict2Enum(__level)

__result = [
    'PASS',
    'FAIL',
]
result = utils.List2Enum(__result)

__tbtype = [
    'ANY',
    'SIMULATION',
    'HARDWARE',
    'HYBRID'
]
tbtype = utils.List2Enum(__tbtype)


HEADER_SUMMARY = "-" * 136
FORMAT_TESTCASE_SUMMARY = "%-15s %-70s %-32s %-7s %-8s"
HEADER_SHORT_SUMMARY = "-" * 100

FORMAT_ALL_TESTSUITE_SUMMARY = "%-32s %-8s %-8s %-8s %-8s %-8s %-8s %-8s"


HOST_TOOLS_DIR = "/pensando/iota/tools"
