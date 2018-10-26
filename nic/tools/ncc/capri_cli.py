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

def make_cli_common_swig_interface(cli_out_dir):
    content_str = \
"""
/* This file is auto-generated. Changes will be overwritten! */
%module cli
%{
    extern int cli_init(char*);
%}
int cli_init(char *grpc_server_port);
"""
    out_file = cli_out_dir + 'cli.i'
    with open(out_file, "w") as of:
        of.write(content_str)

def make_cli_common_swig_main(cli_out_dir):
    content_str = \
"""
/* This file is auto-generated. CHanges will be overwritten! */
//-----------------------------------------------------------------------------
// {C} Copyright 2017 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------

#include "gen/proto/debug.pb.h"
#include "gen/proto/debug.grpc.pb.h"
#include <grpc++/grpc++.h>

using grpc::Channel;
using ::debug::Debug;

std::shared_ptr<Channel>     channel;
std::unique_ptr<Debug::Stub> stub;

int
cli_init(char *grpc_server_port)
{
    grpc_init();

    channel =
	grpc::CreateChannel(grpc_server_port, grpc::InsecureChannelCredentials());

    stub    = ::debug::Debug::NewStub(channel);

    return 0;
}
"""
    out_file = cli_out_dir + "cli_gen.cc"
    with open(out_file, "w") as of:
        of.write(content_str)
      
def create_dirs(dir_name):
    if not os.path.exists(dir_name):
        os.makedirs(dir_name)

tenjin_prefix = "//::"

# get the dir of this file
cur_path      = os.path.abspath(__file__)
cur_path      = os.path.split(cur_path)[0]

# nic gen dir
#gen_dir       = os.path.join(cur_path, '../../gen/')

gen_dir = sys.argv[1]

# cli gen dir
cli_outputdir = os.path.join(gen_dir + 'clicommon/')

# create dir if not exist
create_dirs(cli_outputdir)

# create the cli.i file build/$AARCH/iris/gen/p4genclicommon/
make_cli_common_swig_interface(cli_outputdir)

cli_srcdir = os.path.join(cli_outputdir + 'src/')
create_dirs(cli_srcdir)
# creates cli.cc file in build/x86_64/iris/gen/p4genclicommon/src/ directory
make_cli_common_swig_main(cli_srcdir)

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

