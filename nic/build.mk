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
CC=g++
CXX=g++

WARN = -Wall -Werror -Wno-strict-aliasing -Wno-error=strict-aliasing -Wno-write-strings -Wno-deprecated

CPPFLAGS := $(WARN) \
          -g -m64 -D__USE_GNU -D_GNU_SOURCE -std=c++11

CFLAGS := $(WARN) \
          -g -m64 -D__USE_GNU -D_GNU_SOURCE 

# set the default build target
default : all
