# Usage: make v=1 c=1 g=1 j=4 bazel-build
#
#   v: Verbose. Prints the commands bazel is executing
#   g: Enables debug. For now strip is always disabled.
#   j: Specifies how many jobs bazel has to run
#   c: Enables coverage compilation.

ifeq ($(v),1)
  VERBOSE = -s
else
  VERBOSE = 
endif

ifeq ($(g),1)
  DEBUG = --strip=never -c dbg
else
  DEBUG = --strip=never -c dbg
  #DEBUG = --strip=never --copt -g
  #DEBUG = 
endif

ifeq ($(c),1)
  GCOV = --collect_code_coverage --copt=--coverage --copt=-DCOVERAGE --cxxopt=--coverage --cxxopt=-fprofile-arcs --cxxopt=-ftest-coverage --linkopt=-coverage --linkopt=-lgcov
else
  GCOV = 
endif


CAPAS_BIN = $(TOPDIR)/asic/capri/model/capsim-gen/bin/capas

ifneq ($(GCOV),)
    CAPAS_COV = -C
endif

ifeq ($(j),)
  JOBS = 
else
  JOBS = --jobs=$(j)
endif

BAZEL_OPTIONS = $(JOBS) $(GCOV) $(VERBOSE) $(DEBUG)

