#!/usr/bin/python
#
# Capri Debug Shell (debug_shell.py)
# Ajeer Pudiyapura (Pensando Systems)

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

    jsonfile = './../tools/ncc/csr_json/cap_te_csr.json'
    template = './csr.py'
    out_file = './capri_cli.py'

    if os.path.isfile(jsonfile) and os.path.isfile(template):
        with open(jsonfile, "r") as jf:
            csr = json.load(jf)
            jf.close()

        dic = {}
        with open(out_file, "w") as of:
            dic['csr'] = csr
            render_template(of, template, dic, './', prefix=tenjin_prefix)
            of.close()
