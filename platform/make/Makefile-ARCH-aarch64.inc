#
# Copyright (c) 2017, Pensando Systems Inc.
#
# -*- Mode: makefile -*-

TOOLCHAIN_ROOT=/tool/toolchain/aarch64
TOOLCHAIN_VER=1.1
TOOLCHAIN_PATH=$(TOOLCHAIN_ROOT)-$(TOOLCHAIN_VER)

CC   = $(TOOLCHAIN_PATH)/bin/aarch64-linux-gnu-gcc
CXX  = $(TOOLCHAIN_PATH)/bin/aarch64-linux-gnu-g++
AS   = $(TOOLCHAIN_PATH)/bin/aarch64-linux-gnu-as
YACC = bison -y
