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

WARN = -Wall -Werror -Wno-strict-aliasing -Wno-error=strict-aliasing -Wno-write-strings -Wno-deprecated

CFLAGS := $(WARN) \
          -g -m64 -D__USE_GNU -D_GNU_SOURCE

# set the default build target
default : all

SRC_DIR = ./src
OBJ_DIR = ./obj
INCLUDE_DIRS = -I$(SRC_DIR)/pkt -I../common/include/  -I./include/

CPP_SRC_FILES :=	$(SRC_DIR)/main.cc \
					$(SRC_DIR)/pkt/action.cc \
					$(SRC_DIR)/pkt/svc.cc \
					$(SRC_DIR)/pkt/svc_actions.cc \
					$(SRC_DIR)/if.cc \
					$(SRC_DIR)/l2.cc \
					$(SRC_DIR)/l3.cc \
					$(SRC_DIR)/l4.cc \
					$(SRC_DIR)/l7.cc \

C_SRC_FILES :=

CPP_OBJ_FILES := $(patsubst %.cc, %.o, $(CPP_SRC_FILES))
C_OBJ_FILES := $(patsubst %.c, %.o, $(C_SRC_FILES))

MKDIR = mkdir -p

$(OBJ_DIRS):
	$(MKDIR) obj

$(DIRS): $(OBJ_DIRS)

$(OBJ_DIR):
	mkdir -p $@

$(CPP_OBJ_FILES): %.o: %.cc | $(OBJ_DIR)
	@echo Compiling $@
	$(CC) $(INCLUDE_DIRS) $(CFLAGS) -c $< -o obj/$(shell basename $@)

$(C_OBJ_FILES): %.o: %.c | $(OBJ_DIR)
	@echo Compiling $@
	$(CC) $(INCLUDE_DIRS) $(CFLAGS) -c $< -o obj/$(shell basename $@)

clean :
	@rm -fR obj/*

all: $(DIRS) $(CPP_OBJ_FILES) $(C_OBJ_FILES)
	$(CC) $(INCLUDE_DIRS) $(CFLAGS) obj/*.o -o obj/pproc

