#
# Copyright (c) 2018, Pensando Systems Inc.
#

ASICROOT        := ${SDKDIR}/third-party/asic
PMFILES         := $(wildcard ${ASICROOT}/${ASIC}/verif/common/csr_gen/*.pm)
MODULE_BIN_DIR  := ${BLD_OUT_DIR}/capviewdb_submake/

all: printvars ${MODULE_BIN_DIR}/capviewdb.bin

${MODULE_BIN_DIR}/capviewdb.bin: ${PMFILES} builddb.pl
	perl -I${ASICROOT}/$(ASIC)/verif/common/csr_gen ./builddb.pl ${ASICROOT}/${ASIC} 0 0 ${MODULE_BIN_DIR} "${ASIC}"

.PHONY: printvars
printvars:
	$(foreach v, $(.VARIABLES), $(info $(v) = $($(v))))
