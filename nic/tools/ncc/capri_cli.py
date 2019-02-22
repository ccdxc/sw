#!/usr/bin/python

import os
import sys

_top_dir = os.path.dirname(os.path.dirname(os.path.dirname(os.path.realpath(__file__))))
# insert tenjin path
sys.path.append(_top_dir + '/sdk/tools')

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

def make_cli_common_swig_interface(cli_out_dir, cli_in_dir):
    out_file = cli_out_dir + 'cli.i'
    in_file = cli_in_dir + 'cli.i'
    with open(in_file) as inpf:
        with open(out_file, "w") as of:
            for line in inpf:
                of.write(line)

def make_cli_common_swig_main(cli_out_dir, cli_in_dir):
    out_file = cli_out_dir + "cli.cc"
    in_file = cli_in_dir + "cli.cc"
    with open(in_file) as inpf:
        with open(out_file, "w") as of:
            for line in inpf:
                of.write(line)

def create_dirs(dir_name):
    if not os.path.exists(dir_name):
        os.makedirs(dir_name)

tenjin_prefix = "//::"

# get the dir of this file
cur_path      = os.path.abspath(__file__)
cur_path      = os.path.split(cur_path)[0]

gen_dir = sys.argv[1]
submake_dir = sys.argv[2]
pipeline = sys.argv[3]
if pipeline == 'iris':
    prog_name = 'p4'
else:
    prog_name = pipeline
# pipeline could have obtained from gen dir, for now passing as an argument

# cli gen dir
cli_outputdir = os.path.join(gen_dir + 'clicommon/')

# create dir if not exist
create_dirs(cli_outputdir)

# create the cli.i file build/$AARCH/iris/gen/p4genclicommon/
cli_inputdir = (submake_dir + '/' + pipeline + '/')
make_cli_common_swig_interface(cli_outputdir, submake_dir + '/')

cli_srcdir = os.path.join(cli_outputdir + 'src/')
create_dirs(cli_srcdir)

# creates cli.cc file in build/x86_64/iris/gen/p4genclicommon/src/ directory
make_cli_common_swig_main(cli_srcdir, cli_inputdir)

# template dir
template_dir = os.path.join(cur_path, 'pi_templates/')

cli_outputdir_map = {}
cli_outputdir_map['default'] = cli_outputdir

# list of (template_file, outputfile)
templates_outfiles = make_templates_outfiles(template_dir, cli_outputdir_map)

for template_file, outfile in templates_outfiles:
    outputfile_path = os.path.dirname(outfile)
    pdd = {}
    pdd['prog_name'] = prog_name
    with open(outfile, "w") as of:
        p4tbl_types = render_template(of, template_file, pdd, template_dir, \
                                      prefix=tenjin_prefix)
        of.close()

