help:
	@echo Following targets are available for make
	@echo all - build everything
	@echo clean - clean up everything
	@echo
	exit 1

# architecture dependent flags
osname = $(shell uname)
DETECTED_OS=$(osname)
BLD_MC = $(shell uname -n)
SHELL := /bin/bash

TOOLCHAIN_ROOT=/tool/toolchain/aarch64
TOOLCHAIN_VER=1.1

ifeq ($(ARCH), aarch64)
    CC=$(TOOLCHAIN_ROOT)-$(TOOLCHAIN_VER)/bin/aarch64-linux-gnu-gcc
    CXX=$(TOOLCHAIN_ROOT)-$(TOOLCHAIN_VER)/bin/aarch64-linux-gnu-g++
else
    CC=gcc
    CXX=g++
endif

WARN = -Wall -Werror -Wno-strict-aliasing -Wno-error=strict-aliasing -Wno-write-strings -Wno-deprecated

CPPFLAGS := $(WARN) \
          -g -D__USE_GNU -D_GNU_SOURCE -std=c++11

CFLAGS := $(WARN) \
          -g -D__USE_GNU -D_GNU_SOURCE 

# set the default build target
default : all
