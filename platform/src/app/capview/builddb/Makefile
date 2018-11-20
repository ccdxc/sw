#
# Copyright (c) 2018, Pensando Systems Inc.
#

include ${MKINFRA}/config_${ARCH}.mk

# find csrPerlAPI.pm in $SEMIFORE_HOME/lib
export SEMIFORE_HOME = .

ASICROOT        := ${CURDIR}/../../../../../nic/asic
PMFILES         := $(wildcard ${ASICROOT}/capri/verif/common/csr_gen/*.pm)
MODULE_BIN_DIR  := ${BLD_OUT_DIR}/capviewdb_submake/

all: printvars ${MODULE_BIN_DIR}/capviewdb.bin

${MODULE_BIN_DIR}/capviewdb.bin:
	perl -I${ASICROOT}/capri/verif/common/csr_gen ./builddb.pl ${ASICROOT}/capri 0 0 ${MODULE_BIN_DIR}

.PHONY: printvars
printvars:
	$(foreach v, $(.VARIABLES), $(info $(v) = $($(v))))
