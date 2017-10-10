#!/usr/bin/python
#
# Capri-Non-Compiler-Compiler (capri-ncc)
# Parag Bhide (Pensando Systems)

import pdb
import os
import sys
import logging
import logging.config

_log_modules = {
    'BE': 'be.log',  # General Backend logs
    'Parser': 'parser.log',
    'MA': 'ma.log', # Match-Action
    'PA': 'pa.log',
    'TM': 'tm.log',  # Table Manager
    'DP': 'dp.log',
    'P4PD': 'p4pd.log',
    'TableMapper': 'tblmapper.log',
    'CSUM_V' : 'csum_parser.log',
    'CSUM_C' : 'csum_deparser.log'
}

def logger_init(log_dir, prog_name='P4Prog', loglevel='DEBUG', floglevel='DEBUG'):
    cur_path = log_dir + '/%s/logs' % prog_name
    if not os.path.exists(cur_path):
        os.makedirs(cur_path)

    for m,fname in _log_modules.items():
        f = "%s/%s" % (cur_path, fname)
        logger = logging.getLogger(m)
        fhdlr = logging.FileHandler(f, mode='w')
        fhdlr.setLevel(getattr(logging, floglevel.upper()))
        fhdlr.setFormatter(logging.Formatter(fmt="%(levelname)s::%(msg)s"))
        logger.addHandler(fhdlr)
        chdlr = logging.StreamHandler(sys.stdout)
        chdlr.setLevel(getattr(logging, loglevel.upper()))
        chdlr.setFormatter(logging.Formatter(fmt="%(levelname)s::%(msg)s"))
        logger.addHandler(chdlr)
        # set global level to more verbose (i.e floglevel)
        logger.setLevel(getattr(logging, floglevel.upper()))
