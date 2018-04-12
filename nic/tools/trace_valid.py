#!/usr/bin/python

# {C} Copyright 2017 Pensando Systems Inc. All rights reserved
from __future__ import with_statement

import re
import os
import sys

if len(sys.argv) > 1:
   gdir = os.path.dirname(sys.argv[1])
else:
   gdir = os.getcwd()

excluded_files = ["capri_tm_rw.template.cc",
                  "core.hpp"]

return_status = 0

#------------------------------------------------------------------------------
# validate all traces
#------------------------------------------------------------------------------
def trace_valid():
    for path, dirs, files in os.walk(gdir):
        for filename in files:
            # Check if its C/CPP files only
            ext = os.path.splitext(filename)[-1].lower()
            if ext != ".cc" and ext != ".c" and ext != ".hpp" and ext != ".h":
                continue
            fullpath = os.path.join(path, filename)
            #print "file: " + fullpath
            if not os.path.exists(fullpath) or filename in excluded_files:
                continue
            with open(fullpath, 'r') as f:
                data = f.read().replace('\n', '$%*#')
                validate_trace(filename, data, "HAL_TRACE_DEBUG")
                validate_trace(filename, data, "HAL_TRACE_ERR")
                validate_trace(filename, data, "HAL_TRACE_WARN")
                validate_trace(filename, data, "HAL_TRACE_INFO")


#------------------------------------------------------------------------------
# validate specific traces
#------------------------------------------------------------------------------
def validate_trace(filename, data, trace_func):
    global return_status
    for match in re.finditer(r'%s\(\".*?[^\\]\"[^;]*' % trace_func, data, re.S):
        #print "Match:   %s: %s" % (match.start(), match.group(0))
        full_trace = match.group(0)
        trace = get_valid_trace(full_trace, trace_func)
        fmt_strs = re.findall(r'".*?[^\\]"', trace)
        trace_args = trace
        for fmt_str in fmt_strs:
            trace_args = trace_args.replace(fmt_str,'')
        fmt = ''.join(fmt_strs)

        matched, num_pairs = do_parentheses_match(fmt)
        num_args = get_num_args(trace_args, trace_func)
        #num_args = get_num_args(trace_args, "HAL_TRACE_DEBUG")
        # Debugs
        #print "Checking full_trace: " + full_trace
        #print "Checking trace: " + trace
        #print "Location: " + str(match.start())
        if matched is False or num_pairs != num_args:
            return_status = 1
            f_before = data[0:match.start()]
            #f_before = data[0:data.find(trace)]
            print "Invalid trace at: " + filename + ": "+ str(f_before.count("$%*#") + 1) + ": "+ full_trace + \
                  " :: match: " + str(matched) + ", num_parantheses: " + str(num_pairs) + ", num_args: " + str(num_args)

#------------------------------------------------------------------------------
# prune out if there is no semicolon at the end. to handle LOG_FLOW_UPDATE in
# fte_ctx.cc
#------------------------------------------------------------------------------
def get_valid_trace(trace, func):
    trace = trace.replace(func + "(", '');
    s = []
    index = 0
    while index < len(trace):
        token = trace[index]
        if token == "(":
            s.append(token)
        elif token == ")":
            if len(s) == 0:
                return trace[:index]
            s.pop()
        index += 1

#------------------------------------------------------------------------------
# get number of args given HAL_TRACE_DEBUG(, ..., ...)
#------------------------------------------------------------------------------
def get_num_args(args_string, func):
    args = args_string.replace(func + "(", '');
    args = args[:-1]
    num_args = 0
    index = 0
    in_func_call = False
    s = []
    #print "args: " + args
    while index < len(args):
        token = args[index]
        if token == "(":
            s.append(token)
        elif token == ")":
            s.pop()
        elif token == ",":
            if len(s) == 0:
                num_args += 1

        index += 1
    return num_args

#------------------------------------------------------------------------------
# get number of parantheses matches in fmt
#------------------------------------------------------------------------------
def do_parentheses_match(input_string):
    s = []
    balanced = True
    index = 0
    num_pairs = 0
    while index < len(input_string) and balanced:
        token = input_string[index]
        if token == "{":
            s.append(token)
        elif token == "}":
            if len(s) == 0:
                balanced = False
            else:
                num_pairs += 1
                s.pop()

        index += 1

    return balanced and len(s) == 0, num_pairs

def main():
    global return_status
    print "Checking validity of traces ..."
    trace_valid()
    if return_status == 0:
        print "Passed!!"
    else:
        print "Failed ... please reformat the traces"
    sys.exit(return_status)

if __name__ == "__main__":
    main()
