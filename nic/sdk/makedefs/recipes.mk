# {C} Copyright 2018 Pensando Systems Inc. All rights reserved

#####################################################################
################ TARGET SPECIFIC RECIPE GENERATORS ##################
#####################################################################

define ADD_RECIPE_FOR_ARLIB
$${${1}_MKTARGET}: $${${1}_OBJS}
	${NAT}${AT}echo ${COMMON_ARLIB_SHORT_NAME} $$(call CANPATH,$$@)
	${AT}$$(strip $${CMD_AR} $${CMD_AR_FLAGS} $$@ $${${1}_OBJS})
	${AT}mkdir -p ${BLD_LIB_DIR}
	${NAT}${AT}echo ${COMMON_SYMLINK_SHORT_NAME} $$(call CANPATH,$$@)
	${AT}ln -sf $$@ ${BLD_LIB_DIR}/
endef

define ADD_RECIPE_FOR_SOLIB
$${${1}_MKTARGET}: $${${1}_OBJS}
	${NAT}${AT}echo ${COMMON_SOLIB_SHORT_NAME} $$(call CANPATH,$$@)
	${AT}$$(strip ${CMD_GXX} -o $$@ ${${1}_GXX_FLAGS} ${${1}_FLAGS} $${${1}_OBJS})
	${AT}mkdir -p ${BLD_LIB_DIR}
	${NAT}${AT}echo ${COMMON_SYMLINK_SHORT_NAME} $$(call CANPATH,$$@)
	${AT}ln -sf $$@ ${BLD_LIB_DIR}/
endef

define ADD_RECIPE_FOR_BIN
$${${1}_MKTARGET}: $${${1}_OBJS} $${${1}_DEPS}
	${NAT}${AT}echo ${COMMON_BIN_SHORT_NAME} $$(call CANPATH,$$@)
	${AT}$$(strip ${CMD_GXX} -o $$@ ${${1}_GXX_FLAGS} ${${1}_FLAGS} $${${1}_OBJS}) $${${1}_LIBS} $${${1}_LDPATHS} $${${1}_LDLIBS}
	${AT}mkdir -p ${BLD_BIN_DIR}
	${NAT}${AT}echo ${COMMON_SYMLINK_SHORT_NAME} $$(call CANPATH,$$@)
	${AT}ln -sf $$@ ${BLD_BIN_DIR}/$$(basename $${${1}_TARGET})
endef

define ADD_RECIPE_FOR_P4BIN
$${${1}_MKTARGET}: $${${1}_OBJS} $${${1}_DEPS}
	${NAT}${AT}echo ${COMMON_P4BIN_SHORT_NAME} $$(call CANPATH,$$@)
	${AT}touch $$@
endef

define ADD_RECIPE
    $(call ADD_RECIPE_FOR_${${1}_RECIPE_TYPE},${1})
endef
