# {C} Copyright 2018 Pensando Systems Inc. All rights reserved
#####################################################################
################# SRC SPECIFIC RULE GENERATORS ###################
#####################################################################
EXTS_CXX    := %.cc %.cpp
EXTS_C      := %.c
EXTS_GO     := %.go
EXTS_ASM    := %.asm
EXTS_P4     := %.p4

define ADD_SRC_CXX_OBJECT_RULE
${${1}_BLD_OUT_DIR}/%.o: ${${1}_SRC_DIR}/${2} ${${1}_MODULE_MK} ${COMMON_DEPS}
	${AT}mkdir -p $$(dir $$@)
	${NAT}${AT}echo ${COMMON_GCC_SHORT_NAME} $$@
	${AT}$(strip ${CMD_GXX} -c -o $$@ ${${1}_FLAGS} ${${1}_GXX_FLAGS} ${${1}_INCS} ${${1}_DEFS} $$<)
endef


