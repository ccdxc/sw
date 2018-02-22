.PHONY: lib test build-test
default: all

ifeq ($(V),1)
  VERBOSE = -s
else
  VERBOSE =
endif

# For code coverage
ifeq ($(c),1)
	TOPDIR_ABS = ${CURDIR}/..
	COVFILE_NAME=$(shell realpath "$(TOPDIR_ABS)"/../bazel-out/../../bullseye_hal.cov)
        COV_OUTPUT_BASE = --output_base=$(shell realpath "$(TOPDIR_ABS)"/../bazel-out/../../../sdk)
	export CC=/home/asic/tools/eda/bullseye/bin/gcc
	export CXX=/home/asic/tools/eda/bullseye/bin/g++
	export COVFILE=$(COVFILE_NAME)
  GCOV = --collect_code_coverage --copt=--coverage --copt=-DCOVERAGE --cxxopt=--coverage --cxxopt=-fprofile-arcs --cxxopt=-ftest-coverage --linkopt=-coverage --linkopt=-lgcov --action_env COVFILE=$(COVFILE) --spawn_strategy=standalone 
else
  GCOV =
endif

# Jobs
ifeq ($(j),)
  JOBS =
else
  JOBS = --jobs=$(j)
endif

# Parallel builds
ifneq ($(k),)
    STOPONERR = 
else
    STOPONERR = --keep_going
endif

SKIP_DIRS = --deleted_packages=test/lib/list

ifeq ($(ARCH), aarch64)
	BUILD_OPTS = $(JOBS) $(GCOV) $(VERBOSE) $(STOPONERR) $(SKIP_DIRS) --crosstool_top=//tools/arm_compiler:toolchain --cpu=aarch64-linux --strategy=CppCompile=standalone --strategy=CppLink=standalone
else
	BUILD_OPTS = $(JOBS) $(GCOV) $(VERBOSE) $(STOPONERR) $(SKIP_DIRS)
endif

ifeq ($(BAZEL_OUTPUT_BASE),)
    OUTPUT_BASE =
else
    OUTPUT_BASE = --output_base=$(BAZEL_OUTPUT_BASE)
endif

ifeq ($(c),1)
    BZL_OPT = $(COV_OUTPUT_BASE)
else
    BZL_OPT = $(OUTPUT_BASE)
endif


all: lib

lib:
	@echo $(BUILD_OPTS)
	bazel $(BZL_OPT) build $(BUILD_OPTS) lib/...
	bazel $(BZL_OPT) build $(BUILD_OPTS) linkmgr
	bazel $(BZL_OPT) build $(BUILD_OPTS) examples/...
	@echo Copying over libs ...
	@find bazel-bin/lib -name "*.so" ! -name '*_*' -exec cp {} obj \;
	@find bazel-bin/linkmgr -name "*.so" ! -name '*_*' -exec cp {} obj \;
	@chmod 755 obj/*.so

test: lib build-test
	@echo Running tests ...
	@find bazel-bin/test -name *.runfiles_manifest* | sed 's/\.runfiles_manifest$$//1' | while read i; do printf "\nRunning $$i\n" && $$i; done

build-test:
	@echo $(BUILD_OPTS)
	bazel $(BZL_OPT) build $(BUILD_OPTS) test/...

clean:
	bazel $(BZL_OPT) clean
	bazel $(BZL_OPT) shutdown
	$(RM) obj/*.so

# -----------------------------------------      For Container     -----------------------------------------

# get a shell with the dependencies image loaded, with the host filesystem mounted.
shell: build-runtime-image
	mkdir -p ${PWD}/bazel-cache;
	docker run -it --rm --privileged --net=host -v ${PWD}:/sdk -v ${PWD}/bazel-cache:/root/.cache -w ${PWD} pensando/nic bash

# run `make gtest` with the dependencies image.
cont-test: build-runtime-image
	docker run -it --rm --privileged -v ${PWD}:/sdk -v ${PWD}/bazel-cache:/root/.cache -w /sdk pensando/nic bash -c 'make test'

cont-build: build-runtime-image
	docker run -it --rm --privileged -v ${PWD}/..:/sw -v ${PWD}/../bazel-cache:/root/.cache -w /sdk pensando/nic bash -c 'make'

clean-docker: build-runtime-image
	docker run -it --rm --privileged -v ${PWD}/..:/sw -v ${PWD}/../bazel-cache:/root/.cache -w /sdk pensando/nic bash -c 'make clean'

REGISTRY = registry.test.pensando.io:5000

build-runtime-image: install_box
	if [ "x${NO_PULL}" = "x" ]; then docker pull $(REGISTRY)/pensando/nic:1.5; fi
	BOX_INCLUDE_ENV="NO_COPY" NO_COPY=1 box -t pensando/nic box.rb
	#cd .. && BOX_INCLUDE_ENV="NO_COPY" NO_COPY=1 box -t pensando/nic sdk/box.rb

install_box:
	@if [ ! -x /usr/local/bin/box ]; then echo "Installing box, sudo is required"; curl -sSL box-builder.sh | sudo bash; fi

# make a trial dependencies image. pass RELEASE=1 or run `make deps-release` to
# do a release build.
deps: install_box
	cd .. && RELEASE=${RELEASE} BOX_INCLUDE_ENV="RELEASE" box -t '$(REGISTRY)/pensando/nic:1.5' nic/box-deps.rb

# make a release build of the dependencies image
deps-release:
	RELEASE=1 make deps

# make a shell based on the local build of the dependencies image, for testing
deps-test-shell:
	NO_PULL=1 make shell

# do a test build with a local build of the dependencies image.
deps-test-build:
	NO_PULL=1 make build

