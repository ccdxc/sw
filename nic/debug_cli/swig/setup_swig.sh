#!/bin/bash

#
# Generates python to c++ mapping using swig.
# Compiles the generated c++ files into shared lib.
# Copies the shared lib into the required dir.
#

# use swig to generate python to c++ mapping for header files specified in p4pd.i
swig -c++ -python -outdir pyfiles -o ccfiles/p4pd_wrap.cc p4pd.i

# compile the custom and generated c++ files into the shared lib: _p4pd.so
bazel build //nic/debug_cli/swig/ccfiles:_p4pd.so --verbose_failures

# copy the shared lib to swig generated python file dir
rm -f pyfiles/_p4pd.so
cp ../../../bazel-bin/nic/debug_cli/swig/ccfiles/_p4pd.so pyfiles/_p4pd.so
