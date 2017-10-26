#!/usr/bin/python
#
# Capri Debug Shell (debug_shell.py)
# Ajeer Pudiyapura (Pensando Systems)
# Run it as PYTHONPATH=../../../tools/ncc/ python capri_tm_rw_gen.py

import os
import re
import sys
import pdb
import struct
import logging
import copy
import pprint
import json

from tenjin import *
from tenjin_wrapper import *

tenjin_prefix = "//::"

if __name__ == '__main__':

    template = './capri_tm_rw.template.cc'
    out_file = './capri_tm_rw.cc'

    dic = {}
    with open(out_file, "w") as of:
        render_template(of, template, dic, './', prefix=tenjin_prefix)
        of.close()
