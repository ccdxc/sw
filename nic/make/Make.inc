# Usage: make V=1 c=1 j=4 bazel-build
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

TOPDIR_ABS = ${CURDIR}
# For code coverage for HAL & ASM code
ifeq ($(c),1)
	GCOV = --collect_code_coverage --copt=--coverage --copt=-DCOVERAGE --cxxopt=--coverage --cxxopt=-fprofile-arcs --cxxopt=-ftest-coverage --linkopt=-coverage --linkopt=-lgcov --action_env COVFILE=$(COVFILE) --spawn_strategy=standalone 
else
  GCOV = 
  c    = 0
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

ifneq ($(CUSTOM_FLAGS),)
  BAZEL_CUSTOM_FLAGS = --copt=$(CUSTOM_FLAGS)
endif

ifeq ($(BAZEL_OUTPUT_BASE),)
    OUTPUT_BASE =
else
    OUTPUT_BASE = --output_base=$(BAZEL_OUTPUT_BASE)
endif

TOOLCHAIN_ROOT=/tool/toolchain/aarch64
TOOLCHAIN_VER=1.1
TOOLCHAIN_PATH=$(TOOLCHAIN_ROOT)-$(TOOLCHAIN_VER)

ifeq ($(ARCH), aarch64)
    BZL_BLD_OPT = $(JOBS) $(GCOV) $(VERBOSE) $(BAZEL_CUSTOM_FLAGS) --crosstool_top=//nic/tools/arm_compiler:toolchain --cpu=aarch64-linux --strategy=CppCompile=standalone --strategy=CppLink=standalone
    PATH_OPTS   = PATH=$(TOOLCHAIN_PATH)/bin:$$PATH
    BUILD_OPTS  = BUILD_ARCHES=aarch64
else
    BZL_BLD_OPT = $(JOBS) $(GCOV) $(VERBOSE) $(BAZEL_CUSTOM_FLAGS)
endif

#BZL_OPT = --bazelrc=bazel.rc
BZL_OPT = $(OUTPUT_BASE)

ifeq ($(c),1)
	COVFILE_NAME=$(shell realpath "$(TOPDIR_ABS)"/../bazel-out/../../bullseye_hal.cov)
	export CC=/home/asic/tools/eda/bullseye/bin/gcc
	export CXX=/home/asic/tools/eda/bullseye/bin/g++
	export COVFILE=$(COVFILE_NAME)
endif

