#
# Copyright (c) 2018, Pensando Systems Inc.
#

# find csrPerlAPI.pm in $SEMIFORE_HOME/lib
export SEMIFORE_HOME = .

BUILDENV = 1
TOPDIR = ../../../..
include $(TOPDIR)/make/Makefile.inc

PMFILES = $(wildcard $(ASIC_ROOT)/capri/verif/common/csr_gen/*.pm)
CLEANFILES = capviewdb.bin capviewdb.txt $(GENDIR)/capviewdb.bin

ifneq ("$(strip $(BUILD_ARCH))","")

all: $(GENDIR)/capviewdb.bin

clean:
	$(call cmd,clean)

endif # BUILD_ARCH

quiet_cmd_gendb = GEN     $@
      cmd_gendb = perl -I$(ASIC_ROOT)/capri/verif/common/csr_gen ./builddb.pl $(ASIC_ROOT)/capri 0 0

quiet_cmd_copydb = COPY    capviewdb.bin
      cmd_copydb = cp capviewdb.bin $@

quiet_cmd_clean = CLEAN   capviewdb.bin
      cmd_clean = $(RM) $(CLEANFILES)

$(GENDIR)/capviewdb.bin: capviewdb.bin $(GENDIR)/.dir
	$(call cmd,copydb)

capviewdb.bin: ./builddb.pl $(PMFILES)
	$(call cmd,gendb)

%/.dir:
	$(Q)mkdir -p $(@D) && touch $@
