# Usage: make v=1 c=1 j=4 bazel-build
#
#   V: Verbose. Prints the commands bazel is executing
#   g: Enables debug. For now strip is always disabled.
#   j: Specifies how many jobs bazel has to run
#   c: Enables coverage compilation.

ifeq ($(V),1)
  VERBOSE = -s
else
  VERBOSE =
endif

# For code coverage for HAL & ASM code
ifeq ($(c),1)
  GCOV = --collect_code_coverage --copt=--coverage --copt=-DCOVERAGE --cxxopt=--coverage --cxxopt=-fprofile-arcs --cxxopt=-ftest-coverage --linkopt=-coverage --linkopt=-lgcov
else
  GCOV = 
endif
CAPAS_BIN = $(TOPDIR)/asic/capri/model/capsim-gen/bin/capas
ifneq ($(GCOV),)
    CAPAS_COV = -C
endif

# Jobs 
ifeq ($(j),)
  JOBS = 
else
  JOBS = --jobs=$(j)
endif

ifeq ($(ARCH), aarch64)
    BZL_BLD_OPT = $(JOBS) $(GCOV) $(VERBOSE) --crosstool_top=//nic/tools/arm_compiler:toolchain --cpu=aarch64-linux --strategy=CppCompile=standalone --strategy=CppLink=standalone
else
    BZL_BLD_OPT = $(JOBS) $(GCOV) $(VERBOSE)
endif

#BZL_OPT = --bazelrc=bazel.rc
