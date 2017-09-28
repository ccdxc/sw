# Usage: make v=1 g=1 j=4 bazel-build
#
#   v: Verbose. Prints the commands bazel is executing
#   g: Enables debug. For now strip is always disabled.
#   j: Specifies how many jobs bazel has to run

ifeq ($(v),1)
  VERBOSE = -s
else
  VERBOSE = 
endif

ifeq ($(g),1)
  DEBUG = --strip=never -c dbg
else
  DEBUG = --strip=never -c dbg
  #DEBUG = 
endif

ifeq ($(j),)
  JOBS = 
else
  JOBS = --jobs=$(j)
endif

BAZEL_OPTIONS = $(JOBS) $(VERBOSE) $(DEBUG) 
