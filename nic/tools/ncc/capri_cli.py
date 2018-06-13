#!/usr/bin/python

import os
import sys
from   tenjin         import *
from   tenjin_wrapper import *

def make_templates_outfiles(template_dir, cli_outputdir_map):

    files = [f for f in os.listdir(template_dir) \
              if os.path.isfile(os.path.join(template_dir,f))]

    pdoutfiles = []

    for template_file in files:
        if template_file.endswith('.py'):
            output_dir = cli_outputdir_map['default']
            if "p4pd_cli_frontend_common.py" == template_file:
                genf = 'cli_frontend.py'
            else:
                genf = template_file

        pdoutfiles.append((os.path.join(template_dir, template_file), \
                           os.path.join(output_dir, genf)))
    return pdoutfiles


def create_dirs(dir_name):
    if not os.path.exists(cli_outputdir):
        os.makedirs(cli_outputdir)

tenjin_prefix = "//::"

# get the dir of this file
cur_path      = os.path.abspath(__file__)
cur_path      = os.path.split(cur_path)[0]

# nic gen dir
gen_dir       = os.path.join(cur_path, '../../gen/')

# cli gen dir
cli_outputdir = os.path.join(gen_dir + 'common/cli/')

# create dir if not exist
create_dirs(cli_outputdir)

# template dir
template_dir = os.path.join(cur_path, 'pi_templates/')

cli_outputdir_map = {}
cli_outputdir_map['default'] = cli_outputdir

# list of (template_file, outputfile)
templates_outfiles = make_templates_outfiles(template_dir, cli_outputdir_map)

for template_file, outfile in templates_outfiles:
    outputfile_path = os.path.dirname(outfile)
    pdd = {}
    with open(outfile, "w") as of:
        p4tbl_types = render_template(of, template_file, pdd, template_dir, \
                                      prefix=tenjin_prefix)
        of.close()

