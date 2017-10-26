#
# Copyright (c) 2017, Pensando Systems Inc.
#
# -*- Mode: makefile -*-
# ================
# Makefile template
# ----------------
#
# ================
# Inputs
# ----------------
# Expect these:
#     $(TOPDIR) - Set to relative path to the top-level directory
#                 (the parent directory of this make/ directory).
#
# Select one of these primary targets:
#     $(PROGRAM) - Target program built by this Makefile.
#     $(LIBRARY) - Target library built by this Makefile.
#     $(SUBDIRS) - Subdirectories to descend into for sub-makes.
#     $(BUILDENV) - No target, just provide build make environment.
#
# If PROGRAM or LIBRARY is set, then
#     $(SOURCES) - C, C++, ASM source files that make the target.
#     $(LCFLAGS) - Local CFLAGS appended to CFLAGS.
#     $(LCXXFLAGS) - Local CXXFLAGS appended to CXXFLAGS.
#
# If PROGRAM is set, then
#     $(LIBRARIES) - Built libs to link with, -l<builtlib>.
#     $(STATIC_LIBRARIES) - Built libs to link statically, -l<builtlib>.
#     $(SYSTEM_LIBRARIES) - System libraries linked, -l<syslib>).
#
# ================
# Example PROGRAM
# ----------------
#     PROGRAM = myprog
#     SOURCES = module1.c module2.cc module3.cpp module4.s
#     LCFLAGS = -I ../mylib/include
#     LIBRARIES = -lmylib
#     SYSTEM_LIBRARIES = -lreadline
#     TOPDIR = ../..
#     include $(TOPDIR)/make/Makefile.inc
#
# ================
# Example LIBRARY
# ----------------
#     LIBRARY = mylib.a mylib.so
#     SOURCES = module1.c module2.cc module3.cpp module4.s
#     LCFLAGS = -I ../include
#     TOPDIR = ../..
#     include $(TOPDIR)/make/Makefile.inc
#
# ================
# Example SUBDIRS
# ----------------
# Note always put library directories in the SUBDIRS before programs 
# that depend on those libraries. 
#
#     SUBDIRS = lib1 lib2 program1 program2
#     TOPDIR = ../..
#     include $(TOPDIR)/make/Makefile.inc
#
# Recommend separating SUBDIRS into LIBDIRS and PGMDIRS to make
# the ordering explicit, so future additions will naturally add to
# the correct list.
#
#     LIBDIRS = lib1 lib2
#     PGMDIRS = program1 program2
#     SUBDIRS = $(LIBDIRS) $(PGMDIRS)
#     TOPDIR = ../..
#     include $(TOPDIR)/make/Makefile.inc
#     ifneq ("$(strip $(BUILD_ARCH))","")
#     test:
#             @runtest $(GENBIN)/program1 && echo Test Passed
#     endif # BUILD_ARCH
#
# ================
# Caller variables
# ----------------
# Caller can supply these variables to augment this system.
#
#     LCFLAGS = Local CFLAGS appended to CFLAGS.
#     LCXXFLAGS = Local CXXFLAGS appended to CXXFLAGS.
#     LASFLAGS = Local ASFLAGS appended to ASFLAGS.
#     LLDFLAGS = Local LDFLAGS appended to LDFLAGS for program link.
#     LCLEANFILES = Local files list removed by "clean" target.
#     LPREALL = Dependency added to "all" target before system targets.
#     LPOSTALL = Dependency added to "all" target after system targets.
#
# ================
# Targets
# ----------------
#     all - build the target
#     clean - rm all generated files (objects, depends, targets)

# ================
# BUILD_ARCH
# ----------------
# If no BUILD_ARCH, compute default BUILD_ARCHES and build each
ifeq ("$(strip $(BUILD_ARCH))","")

#
# BUILD_ARCHES is set to default list, unless overridden
# with LBUILD_ARCHES in caller's Makefile
#
BUILD_ARCHES_DEFAULT = x86_64
LBUILD_ARCHES ?= $(BUILD_ARCHES_DEFAULT)
BUILD_ARCHES ?= $(LBUILD_ARCHES)

all clean test:
	@$(MAKE) T=$@ build_arches

build_arches:
	@for BUILD_ARCH in $(BUILD_ARCHES); do \
		set -e; \
		echo ================ ; \
		echo ================ MAKE BUILD_ARCH=$$BUILD_ARCH; \
		echo ================ ; \
		$(MAKE) BUILD_ARCH=$$BUILD_ARCH $(T); \
	done

else

include $(TOPDIR)/make/Makefile-ARCH-$(BUILD_ARCH).inc

#
# Figure out build environment.
#     BUILD_OS = Linux|Darwin
#
BUILD_OS := $(shell uname)

# ================

#
# Figure out where we are in the file system relative to TOPDIR
#
TOPPATH := $(shell (cd $(TOPDIR); /bin/pwd))
SRCPATH := $(shell /bin/pwd)
SRCDIR := $(subst $(TOPPATH)/,,$(SRCPATH))

#
# GENTOP is the root of generated files from the build.
# GENLIB collects all the built libraries.
# GENBIN collects all the built programs.
# GENDIR is the output subdirectory in GENTOP parallel to SRCDIR
#
GENTOP = $(TOPDIR)/gen/$(BUILD_ARCH)
GENLIB = $(GENTOP)/lib
GENBIN = $(GENTOP)/bin
GENDIR = $(GENTOP)/$(SRCDIR)

# OPT is separate to allow override on command line, e.g. "make OPT=-g"
OPT ?= -O3

BUILD_PARAMS = -DBUILD_OS_$(BUILD_OS) -DBUILD_ARCH=$(BUILD_ARCH)
CCWARN_FLAGS ?= -Wall -Werror -Wuninitialized
CCDEPEND_FLAGS = -MT $@ -MMD -MP -MF $(@:.o=.d)
INCLUDE_DIRS = -I$(TOPDIR)

CFLAGS = $(NULL)
CFLAGS += $(BUILD_PARAMS)
CFLAGS += $(OPT) $(CCWARN_FLAGS)
CFLAGS += $(CCDEPEND_FLAGS)
CFLAGS += $(INCLUDE_DIRS)
CFLAGS += $(LCFLAGS)

CXXFLAGS = $(NULL)
CXXFLAGS += $(BUILD_PARAMS)
CXXFLAGS += $(OPT) $(CCWARN_FLAGS) -std=c++11 
CXXFLAGS += $(CCDEPEND_FLAGS)
CXXFLAGS += $(INCLUDE_DIRS)
CXXFLAGS += $(LCXXFLAGS)

ASFLAGS += $(LASFLAGS)

LDFLAGS += $(LLDFLAGS)

YACC = bison -y

#
# As with Linux kernel builds "make V=1" enables the verbose build output.
# Default is the quiet form displaying simplified command build output.
#
# Use $(Q) as a prefix for commands that are normally hidden but
# should be displayed with verbose build output.
#
# For build rules, if $quiet == "quiet_" echo the short form of the command
# then execute the full form of the command.  If $quiet == "" echo
# the full form of the command then execute the full form.  We accomplish
# this by, e.g. "echo $($(quiet)cmd_cc_o_c) && $(cmd_cc_o_c)" and
# $quiet selects which form is echo'd.
#
ifeq ($(V),1)
  quiet =
  Q =
else
  quiet=quiet_
  Q = @
endif

cmd = @echo "$($(quiet)cmd_$1)" && $(cmd_$1)

# ================================================================
# no PROGRAM, LIBRARY, SUBDIRS, BUILDENV
# ----------------------------------------------------------------
ifeq ("$(strip $(PROGRAM)$(LIBRARY)$(SUBDIRS)$(BUILDENV))","")

all clean test:

else
# ================================================================
# SUBDIRS
# ----------------------------------------------------------------
ifneq ("$(strip $(SUBDIRS))","")

all clean test:
	@for d in $(SUBDIRS); do \
		set -e; \
		subdir=`[ "$(TOPPATH)" = "$(SRCPATH)" ] || \
			echo $(SRCDIR)/`$$d; \
		echo ================ SUBDIR $$subdir \($(BUILD_ARCH)\); \
		$(MAKE) -C $$d $@; \
	done

else

# ================================================================
# PROGRAM
# ----------------------------------------------------------------
ifneq ("$(strip $(PROGRAM))","")
TARGET = $(PROGRAM)
GENTARGET = $(PROGRAM:%=$(GENDIR)/%)
CLEANFILES += $(GENTARGET) $(GENBIN)/$(PROGRAM)

# derive OBJECTS from SOURCES
OBJECTS := $(basename $(SOURCES))
OBJECTS := $(OBJECTS:%=$(GENDIR)/%.o)
# derive DEPENDS from OBJECTS
DEPENDS = $(OBJECTS:.o=.d)

CLEANFILES += $(OBJECTS) $(DEPENDS)

all: $(LPREALL) $(GENTARGET) $(LPOSTALL)

# Deduce PROGRAM type from SOURCES.
# If any C++ sources, link with CXX else CC.
ifeq ("$(filter %.cc,$(SOURCES))","")
PGMLINK = $(CC) $(LDFLAGS)
else
PGMLINK = $(CXX) $(CXXFLAGS) $(LDFLAGS)
endif

quiet_cmd_pgmlink = LINK    $(PROGRAM)
      cmd_pgmlink = $(PGMLINK) -o $@ \
			$(OBJECTS) $(GENLIBRARIES) $(SYSTEM_LIBRARIES) \
			&& cp $@ $(GENBIN)/$(@F)

# construct built libraries list path to $(GENLIB)
GENLIBRARIES_A = $(STATIC_LIBRARIES:-l%=$(GENLIB)/lib%.a)
GENLIBRARIES_SO = $(LIBRARIES:-l%=$(GENLIB)/lib%.so)
GENLIBRARIES = $(GENLIBRARIES_A) $(GENLIBRARIES_SO)

$(GENTARGET): $(GENBIN)/.dir $(OBJECTS) $(GENLIBRARIES)
	$(call cmd,pgmlink)

endif # PROGRAM

# ================================================================
# LIBRARY
# ----------------------------------------------------------------
ifneq ("$(strip $(LIBRARY))","")
TARGET = $(LIBRARY)
# Separate static, shared libraries
LIBRARY_A  = $(filter %.a, $(LIBRARY))
ifeq ("$(strip $(BUILD_OS))","Linux")
# Lib link args are different for Darwin, limit .so's to Linux for now.
LIBRARY_SO = $(filter %.so,$(LIBRARY))
endif
GENPICS = $(GENDIR)/pics
GENTARGET_A = $(LIBRARY_A:%=$(GENDIR)/%)
GENTARGET_SO = $(LIBRARY_SO:%=$(GENPICS)/%)
GENTARGET = $(GENTARGET_A) $(GENTARGET_SO)
CLEANFILES += $(GENTARGET) $(LIBRARY:%=$(GENLIB)/%)

# derive OBJECTS from SOURCES
SOURCES_BASES := $(basename $(SOURCES))
ifneq ("$(strip $(LIBRARY_A))","")
OBJECTS = $(SOURCES_BASES:%=$(GENDIR)/%.o)
endif
ifneq ("$(strip $(LIBRARY_SO))","")
OBJECTS_PICS = $(SOURCES_BASES:%=$(GENPICS)/%.o)
endif
# derive DEPENDS from OBJECTS
DEPENDS = $(OBJECTS:.o=.d) $(OBJECTS_PICS:.o=.d)

CLEANFILES += $(OBJECTS) $(OBJECTS_PICS) $(DEPENDS)

all: $(LPREALL) $(GENTARGET) $(LPOSTALL)

quiet_cmd_liblink     = AR      $(LIBRARY_A)
      cmd_liblink     = $(AR) cr $@ $(OBJECTS) && cp $@ $(GENLIB)/$(@F)
quiet_cmd_liblink_pic = CC PIC  $(LIBRARY_SO)
      cmd_liblink_pic = $(CC) $(LDFLAGS) -shared -Wl,-soname,$(@F) -o $@ \
			  $(OBJECTS_PICS) $(SYSTEM_LIBRARIES) \
			  && cp $@ $(GENLIB)/$(@F)

$(GENTARGET_A): $(GENLIB)/.dir $(OBJECTS)
	$(call cmd,liblink)

$(GENTARGET_SO): $(GENLIB)/.dir $(OBJECTS_PICS)
	$(call cmd,liblink_pic)

endif # LIBRARY

# ----------------------------------------------------------------
# common PROGRAM, LIBRARY
# ----------------------------------------------------------------
ifneq ("$(strip $(PROGRAM)$(LIBRARY))","")
CLEANFILES += $(LGENCLEANFILES:%=$(GENDIR)/%)
CLEANFILES += $(LCLEANFILES)

quiet_cmd_clean = CLEAN   $(TARGET)
      cmd_clean = $(RM) $(CLEANFILES)

clean:
	$(call cmd,clean)

quiet_cmd_cc_o_c     = CC      $<
      cmd_cc_o_c     = $(CC) $(CPPFLAGS) $(CFLAGS) -c -o $@ $<
quiet_cmd_cc_o_c_pic = CC PIC  $<
      cmd_cc_o_c_pic = $(CC) $(CPPFLAGS) $(CFLAGS) -fpic -c -o $@ $<

$(GENDIR)/%.o: %.c
	$(call cmd,cc_o_c)

$(GENPICS)/%.o: %.c
	$(call cmd,cc_o_c_pic)

quiet_cmd_cxx_o_cc     = CXX     $<
      cmd_cxx_o_cc     = $(CXX) $(CPPFLAGS) $(CXXFLAGS) -c -o $@ $<
quiet_cmd_cxx_o_cc_pic = CXX PIC $<
      cmd_cxx_o_cc_pic = $(CXX) $(CPPFLAGS) $(CXXFLAGS) -fpic -c -o $@ $<

$(GENDIR)/%.o: %.cc
	$(call cmd,cxx_o_cc)

$(GENPICS)/%.o: %.cc
	$(call cmd,cxx_o_cc_pic)

quiet_cmd_as_o_s = AS      $<
      cmd_as_o_s = $(AS) $(ASFLAGS) -o $@ $<
quiet_cmd_cc_o_s     = CC      $<
      cmd_cc_o_s     = $(CC) $(CPPFLAGS) $(CFLAGS) -c -o $@ $<
quiet_cmd_cc_o_s_pic = CC PIC  $<
      cmd_cc_o_s_pic = $(CC) $(CPPFLAGS) $(CFLAGS) -fpic -c -o $@ $<

$(GENDIR)/%.o: %.s
	$(call cmd,cc_o_s)

$(GENPICS)/%.o: %.s
	$(call cmd,cc_o_s_pic)

%/.dir:
	$(Q)mkdir -p $(@D) && touch $@

$(OBJECTS): $(GENDIR)/.dir

$(OBJECTS_PICS): $(GENPICS)/.dir

-include $(DEPENDS)

endif # PROGRAM, LIBRARY common
endif # SUBDIRS
endif # no PROGRAM, LIBRARY, SUBDIRS, BUILDENV

.PHONY: all clean test .test

# ================================================================
# Makefile debug
# ----------------------------------------------------------------
.test:
	@echo BUILD_ARCHES = $(BUILD_ARCHES)
	@echo LBUILD_ARCHES = $(LBUILD_ARCHES)
	@echo BUILD_ARCH = $(BUILD_ARCH)
	@echo BUILD_OS = $(BUILD_OS)
	@echo PROGRAM = $(PROGRAM)
	@echo LIBRARY = $(LIBRARY)
	@echo SUBDIRS = $(SUBDIRS)
	@echo SOURCES = $(SOURCES)
	@echo OBJECTS = $(OBJECTS)
	@echo DEPENDS = $(DEPENDS)
	@echo TOPPATH = $(TOPPATH)
	@echo SRCPATH = $(SRCPATH)
	@echo SRCDIR = $(SRCDIR)
	@echo GENTOP = $(GENTOP)
	@echo GENBIN = $(GENBIN)
	@echo GENLIB = $(GENLIB)
	@echo GENDIR = $(GENDIR)
	@echo GENTARGET = $(GENTARGET)
	@echo GENLIBRARIES = $(GENLIBRARIES)
	@echo GENPICS = $(GENPICS)
	@echo LIBRARY_A = $(LIBRARY_A)
	@echo LIBRARY_SO = $(LIBRARY_SO)
	@echo CLEANFILES = $(CLEANFILES)
	@echo CC = $(CC)
	@echo CXX = $(CXX)
	@echo YACC = $(YACC)
	@echo PGMLINK = $(PGMLINK)

endif # make BUILD_ARCH
