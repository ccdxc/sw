#!/usr/bin/python

#------------------------------------------------------------------------------
# {C} Copyright 2017 Pensando Systems Inc. All rights reserved
#------------------------------------------------------------------------------
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
enable_pattern_check = True         # Checks for invalid patterns in trace strings
invalid_patterns = ["{\d+"]         # Invalid patterns in trace strings

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
                validate_trace(fullpath, data, "HAL_TRACE_DEBUG")
                validate_trace(fullpath, data, "HAL_TRACE_ERR")
                validate_trace(fullpath, data, "HAL_TRACE_WARN")
                validate_trace(fullpath, data, "HAL_TRACE_INFO")


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
        #print "Checking trace: " + trace
        #print "Checking trace_args: " + trace_args
        #print "Checking fmt: " + fmt

        # HAL_TRACE_DEBUG("PROXYRCB Programming stage0 at hw_addr: 0x{0:x}", hw_addr)
        # trace: "PROXYRCB Programming stage0 at hw_addr: 0x{0:x}", hw_addr
        # fmt: PROXYRCB Programming stage0 at hw_addr: 0x{0:x}
        # trace_args: , hw_addr

        matched, num_pairs = do_parentheses_match(fmt)
        num_args = get_num_args(trace_args, trace_func)
        has_unwanted_pattern, pattern = has_unwanted_patterns(fmt)
        #num_args = get_num_args(trace_args, "HAL_TRACE_DEBUG")
        # Debugs
        #print "Checking full_trace: " + full_trace
        #print "Checking trace: " + trace
        #print "Location: " + str(match.start())
        if matched is False or num_pairs != num_args or has_unwanted_pattern is True:
            return_status = 1
            f_before = data[0:match.start()]
            #f_before = data[0:data.find(trace)]
            error_reason = "Has"
            if has_unwanted_pattern:
                error_reason += " un-wanted pattern " + pattern
            if matched is False:
                error_reason += " un-matched parans"
            if num_pairs != num_args:
                error_reason += " un-matched args (#parans: " + str(num_pairs) + " #args: " + str(num_args)

            # line num: str(f_before.count("$%*#") + 1)
            print error_reason + " at: " + filename + ": "+ str(f_before.count("$%*#") + 1) + ": "+ full_trace
            #print "Invalid trace at: " + filename + ": "+ str(f_before.count("$%*#") + 1) + ": "+ full_trace + \
            #      " :: match: " + str(matched) + ", num_parantheses: " + str(num_pairs) + ", num_args: " + str(num_args)

#------------------------------------------------------------------------------
# Filter out fmts having unwanted patterns like {:x}, {1:x} etc.
#------------------------------------------------------------------------------
def has_unwanted_patterns(fmt):
    global enable_pattern_check,invalid_patterns
    has_unwanted_pattern = False
    if enable_pattern_check:
        for pattern in invalid_patterns:
            regexp = re.compile(r"%s" % pattern)
            if regexp.search(fmt):
                has_unwanted_pattern = True
                return has_unwanted_pattern, pattern
    return has_unwanted_pattern, ""


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
