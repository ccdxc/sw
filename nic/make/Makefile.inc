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
#     $(PROGRAM)    - Target program built by this Makefile.
#     $(LIBRARY)    - Target library built by this Makefile.
#     $(SUBDIRS)    - Subdirectories to descend into for sub-makes.
#
# If PROGRAM or LIBRARY is set, then
#     $(SOURCES) - C, C++, ASM source files that make the target.
#     $(LCFLAGS) - Local CFLAGS appended to CFLAGS.
#     $(LCXXFLAGS) - Local CXXFLAGS appended to CXXFLAGS.
#
# If PROGRAM is set, then
#     $(LIBRARIES) - List of other build system .a's to link with.
#     $(SYSTEM_LIBRARIES) - List of system libraries (usually -l<lib>) linked.
#
# If PLATFORM is set, then
#     $(PLATFORM) - Platform against which the binaries and libs are built.
#                   By default, it is for sim env.
#                   Set PLATFORM=haps for haps platform.
#
# ================
# Example PROGRAM
# ----------------
#     PROGRAM = myprog
#     SOURCES = module1.c module2.cc module3.cpp module4.s
#     LCFLAGS = -I ../mylib/include
#     LIBRARIES = mylib.a
#     SYSTEM_LIBRARIES = -lreadline
#     TOPDIR = ../..
#     include $(TOPDIR)/make/Makefile.inc
#
# ================
# Example LIBRARY
# ----------------
#     LIBRARY = mylib.a
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
#
# ================
# Targets
# ----------------
#     all - build the target
#     clean - rm all generated files (objects, depends, targets)

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
# GENDIR is the output subdirectory in GENTOP parallel SRCDIR
#
GENTOP = $(TOPDIR)/obj
GENLIB = $(TOPDIR)/obj
GENBIN = $(TOPDIR)/obj
GENDIR = $(TOPDIR)/obj
# Original
# GENTOP = $(TOPDIR)/gen
# GENLIB = $(GENTOP)/lib
# GENBIN = $(GENTOP)/bin
# GENDIR = $(GENTOP)/$(SRCDIR)

# derive OBJECTS from SOURCES
# OBJECTS := $(notdir $(SOURCES))
# OBJECTS := $(basename $(OBJECTS))

ONLY_SOURCES = $(notdir $(SOURCES))
ONLY_OBJECTS1 = $(basename $(ONLY_SOURCES))
ONLY_OBJECTS  = $(ONLY_OBJECTS1:%=$(GENDIR)/%.o)
ONLY_DEPENDS = $(ONLY_OBJECTS:.o=.d)

# OBJECTS1 = $(basename $(SOURCES))
# OBJECTS = $(OBJECTS1:%=$(GENDIR)/%.o)
# DEPENDS = $(OBJECTS:.o=.d)

# Try 1 - Able to override GENDIR
# OBJECTS1 = $(basename $(SOURCES))
# OBJECTS  = $(OBJECTS1:%=$(GENDIR)/%.o)
# DEPENDS = $(OBJECTS:.o=.d)

# Original
OBJECTS := $(basename $(SOURCES))
OBJECTS := $(OBJECTS:%=$(GENDIR)/%.o)
# derive DEPENDS from OBJECTS
DEPENDS = $(OBJECTS:.o=.d)
DEPENDS += $(LDEPENDS)

CLEANFILES = $(OBJECTS) $(DEPENDS)
CLEANFILES += $(LGENCLEANFILES:%=$(GENDIR)/%)
CLEANFILES += $(LCLEANFILES)

# OPT is separate to allow override on command line, e.g. "make OPT=-g"
# For now to allow debugging ... eventually we can make it -O3
# OPT = -g 
# OPT = -O3

# For code coverage, do "make GCOV="--coverage -DCOVERAGE" <target>"
GCOV?=

CFLAGS = $(OPT) $(GCOV) -Wall -Werror -Wuninitialized -fpic
# CFLAGS += -MT $(GENDIR)/$(addsuffix .o,$(basename $(notdir $(<)))) -MMD -MP -MF $(GENDIR)/$(notdir $*).d
# Original
CFLAGS += -MT $@ -MMD -MP -MF $(GENDIR)/$*.d
CFLAGS += $(LCFLAGS)

CXXFLAGS = $(OPT) $(GCOV) -std=c++11 -Wall -Werror -Wuninitialized -fpic
# CXXFLAGS += -MT $(GENDIR)/$(addsuffix .o,$(basename $(notdir $(<)))) -MMD -MP -MF $(GENDIR)/$(notdir $*).d
# Original
CXXFLAGS += -MT $@ -MMD -MP -MF $(GENDIR)/$*.d
CXXFLAGS += $(LCXXFLAGS)

ASFLAGS += $(LASFLAGS)

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
# colors
# ----------------------------------------------------------------
red=`tput setaf 1`
green=`tput setaf 2`
yellow=`tput setaf 3`
blue=`tput setaf 4`
magenta=`tput setaf 5`
cyan=`tput setaf 6`
reset=`tput sgr0`


# ================================================================
# missing PROGRAM, LIBRARY, SUBDIRS
# ----------------------------------------------------------------
ifeq ("$(strip $(PROGRAM)$(LIBRARY)$(SHAREDLIBRARY)$(SUBDIRS)$(CUSTOM)$(BUILDOBJS))","")
all clean:
	@echo "Makefile did not specify PROGRAM, LIBRARY, SHAREDLIBRARY, CUSTOM, BUILDOBJS or SUBDIRS" && exit 1
else
# ================================================================
# SUBDIRS
# ----------------------------------------------------------------
ifneq ("$(strip $(SUBDIRS))","")

all clean gtest:
	@for d in $(SUBDIRS); do \
		set -e; \
		subdir=`[ "$(TOPPATH)" = "$(SRCPATH)" ] || \
			echo $(SRCDIR)/`$$d; \
		echo ${yellow}================ SUBDIR $$subdir${reset}; \
		# echo ================ SUBDIR $$subdir; \
        if [ "$@" = "gtest" ]; \
            then if [ "$$subdir" = "capsim" ]; \
                then continue; \
            fi; \
        fi; \
        if test -d $$d; then \
            $(MAKE) -C $$d $@; \
        else \
            echo "No Dir. Skipping ..."; \
        fi; \
	done

else

# ================================================================
# PROGRAM
# ----------------------------------------------------------------
ifneq ("$(strip $(PROGRAM))","")
TARGET = $(PROGRAM)
GENTARGET = $(PROGRAM:%=$(GENDIR)/%)
GENLIBRARIES = $(LIBRARIES:%=$(GENLIB)/%)
CLEANFILES += $(GENBIN)/$(PROGRAM)

# Deduce PROGRAM type from SOURCES.
# If any C++ sources, link with CXX else CC.
ifeq ("$(filter %.cc,$(SOURCES))","")
PGMLINK = $(CC) $(LLDFLAGS)
else
PGMLINK = $(CXX) $(CXXFLAGS) $(LLDFLAGS)
endif

all: $(LPREALL) $(GENTARGET) $(LPOSTALL)

quiet_cmd_pgmlink = LINK    $(PROGRAM)
      cmd_pgmlink = $(PGMLINK) -o $@ $(OBJECTS) $(GENLIBRARIES) \
			$(SYSTEM_LIBRARIES)

# Original
# quiet_cmd_pgmlink = LINK    $(PROGRAM)
#       cmd_pgmlink = $(PGMLINK) -o $@ $(OBJECTS) $(GENLIBRARIES) \
# 			$(SYSTEM_LIBRARIES) && \
# 			cp $@ $(GENBIN)/$(@F)
$(GENTARGET): $(GENBIN)/.dir $(OBJECTS) $(GENLIBRARIES)
	$(call cmd,pgmlink)

endif # PROGRAM

# ================================================================
# LIBRARY
# ----------------------------------------------------------------
ifneq ("$(strip $(LIBRARY))","")
TARGET = $(LIBRARY)
GENTARGET = $(LIBRARY:%=$(GENDIR)/%)
CLEANFILES += $(GENLIB)/$(LIBRARY)

all: $(LPREALL) $(GENTARGET) $(LPOSTALL)

quiet_cmd_liblink = AR      $(LIBRARY)
      cmd_liblink = $(AR) cr $@ $(OBJECTS) && cp $@ $(GENLIB)/$(@F)

$(GENTARGET): $(GENLIB)/.dir $(OBJECTS)
	$(call cmd,liblink)

endif # LIBRARY

# ================================================================
# Shared LIBRARY
# ----------------------------------------------------------------
ifneq ("$(strip $(SHAREDLIBRARY))","")
TARGET = $(SHAREDLIBRARY)
GENTARGET = $(SHAREDLIBRARY:%=$(GENDIR)/%)
CLEANFILES += $(GENLIB)/$(SHAREDLIBRARY)

all: $(LPREALL) $(GENTARGET) $(LPOSTALL)

quiet_cmd_sliblink = SHARED  $(SHAREDLIBRARY)
      cmd_sliblink = $(CXX) -shared -o $@ $(OBJECTS) $(SLOBJECTS) $(SLFLAGS) $(GCOV)
      # Original
      # cmd_sliblink = $(CXX) -shared -o $@ $(OBJECTS) && cp $@ $(GENLIB)/$(@F)

$(GENTARGET): $(GENLIB)/.dir $(OBJECTS) $(SLOBJECTS)
	@echo "Gen Lib: $(GENLIB) Objects: $(OBJECTS) SLObjects: $(SLOBJECTS)"
	$(call cmd,sliblink)

endif # Shared LIBRARY

# ================================================================
# Custom targets
# ----------------------------------------------------------------
ifneq ("$(strip $(CUSTOM))","")
TARGET = $(CUSTOM)
GENTARGET = $(CUSTOM)
CLEANFILES += 

all: $(LPREALL) $(GENTARGET) $(LPOSTALL)

endif # Custom

# ================================================================
# Build Objects Only. No other target
# ----------------------------------------------------------------
ifneq ("$(strip $(BUILDOBJS))","")
# TARGET = $(BUILDOBJS)
# GENTARGET = $(BUILDOBJS:%=$(GENDIR)/%)
# CLEANFILES += $(GENLIB)/$(BUILDOBJS)

all: $(GENLIB)/.dir $(OBJECTS) $(LPOSTALL)
	# @echo "Built Objects: $(OBJECTS)"

endif # Build Objects

# ----------------------------------------------------------------
# common PROGRAM, LIBRARY
# ----------------------------------------------------------------
RM = rm -rf
#quiet_cmd_clean = CLEAN   $(TARGET)
quiet_cmd_clean = CLEAN   $(CLEANFILES)
      cmd_clean = $(RM) $(CLEANFILES) $(GENTARGET)

clean: $(CUSTOM_CLEAN)
	$(call cmd,clean)

quiet_cmd_cc_o_c = CC      $<
      cmd_cc_o_c = $(CC) $(CPPFLAGS) $(CFLAGS) -c -o $(GENDIR)/$(addsuffix .o,$(basename $(notdir $(<)))) $<
      # Original
      # cmd_cc_o_c = $(CC) $(CPPFLAGS) $(CFLAGS) -c -o $@ $<

$(GENDIR)/%.o: %.c
	$(call cmd,cc_o_c)

quiet_cmd_cxx_o_cc = CXX     $<
      # cmd_cxx_o_cc = $(CXX) $(CPPFLAGS) $(CXXFLAGS) -c -o $($<: .cc=.o) $<
      # cmd_cxx_o_cc = $(CXX) $(CPPFLAGS) $(CXXFLAGS) -c -o $(GENDIR)/$(addsuffix .o,$(basename $(notdir $(<)))) $<
      # Original
      cmd_cxx_o_cc = $(CXX) $(CPPFLAGS) $(CXXFLAGS) -c -o $@ $<

$(GENDIR)/%.o: %.cc
	@# @echo Compiling $@
	$(call cmd,cxx_o_cc)

quiet_cmd_as_o_s = AS      $<
      cmd_as_o_s = $(AS) $(ASFLAGS) -o $@ $<

$(GENDIR)/%.o: %.s
	$(call cmd,as_o_s)

%/.dir:
	$(Q)mkdir -p $(@D) && touch $@

gtest: ;

$(OBJECTS): $(GENDIR)/.dir

-include $(DEPENDS)

endif # SUBDIRS
endif # no PROGRAM, LIBRARY, SUBDIRS

# ================================================================
# Makefile debug
# ----------------------------------------------------------------
.test:
	@echo PROGRAM = $(PROGRAM)
	@echo LIBRARY = $(LIBRARY)
	@echo SHAREDLIBRARY = $(SHAREDLIBRARY)
	@echo SUBDIRS = $(SUBDIRS)
	@echo CUSTOM = $(CUSTOM)
	@echo SOURCES = $(SOURCES)
	@echo LIBRARIES = $(LIBRARIES)
	@echo SYSTEM_LIBRARIES = $(SYSTEM_LIBRARIES)
	@echo OBJECTS = $(OBJECTS)
	@echo ONLY_OBJECTS = $(ONLY_OBJECTS)
	@echo DEPENDS = $(DEPENDS)
	@echo ONLY_DEPENDS = $(ONLY_DEPENDS)
	@echo TOPPATH = $(TOPPATH)
	@echo SRCPATH = $(SRCPATH)
	@echo SRCDIR = $(SRCDIR)
	@echo GENTOP = $(GENTOP)
	@echo GENBIN = $(GENBIN)
	@echo GENLIB = $(GENLIB)
	@echo GENDIR = $(GENDIR)
	@echo GENTARGET = $(GENTARGET)
	@echo GENLIBRARIES = $(GENLIBRARIES)
	@echo CLEANFILES = $(CLEANFILES)
	@echo CC = $(CC)
	@echo CXX = $(CXX)
	@echo LCFLAGS = $(LCFLAGS)
	@echo LCXXFLAGS = $(LCXXFLAGS)
	@echo PGMLINK = $(PGMLINK)
	@echo PLATFORM = $(PLATFORM)
