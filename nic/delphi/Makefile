# {C} Copyright 2018 Pensando Systems Inc. All rights reserved

ASIC ?= capri
# This is a dummy target to build all delphi componentes
MODULE_PREREQS = $(shell find . -name "module*.mk" -exec grep TARGET {} \; -print | grep TARGET | awk '{print $$3}' | tr " " "\n" | LC_ALL=C sort | tr "\n" " ")

MODULE_TEST_TARGETS = $(shell find . -name "module*.mk" -exec grep TARGET {} \; | awk '{print $$3}' | grep gtest | awk -F\. '{print $$1}' | tr " " "\n" | LC_ALL=C sort | tr "\n" " ")


default: build

build:
	@echo "+++ Building " $$MODULE_PREREQS
	cd .. && make ${MODULE_PREREQS}

deps:
	@echo "+++ Building delphi dependencies..."
	@cd .. && make libdelphisdk.so

test: build
	make test-only

test-only: 
	for tgt in $(MODULE_TEST_TARGETS); do \
		if ! ../build/x86_64/iris/${ASIC}/bin/$$tgt; then \
			echo "============= $$tgt Test failed ========================"; \
			exit 1; \
		 fi; \
	done
	@echo "============= Running go test ========================"; \
	LD_LIBRARY_PATH=$$GOPATH/src/github.com/pensando/sw/nic/build/x86_64/iris/${ASIC}/lib/ go test -v github.com/pensando/sw/nic/delphi/...  -count 1
	@echo "============= All tests passed ========================"; \

test-memleak:
	for tgt in $(MODULE_TEST_TARGETS); do \
		echo "============= valgrind $$tgt  ========================"; \
		valgrind --tool=memcheck --leak-check=yes ../build/x86_64/iris/${ASIC}/bin/$$tgt 2>&1| grep "definitely lost:"; \
	done
