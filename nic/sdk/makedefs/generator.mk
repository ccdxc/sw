# {C} Copyright 2018 Pensando Systems Inc. All rights reserved
#
# INCLUDE_MODULEMK
# - Function to include one module.mk file.
# Param:
# - Directory of the module.mk file

include ${MAKEDEFS}/utils.mk
include ${MAKEDEFS}/sources.mk
include ${MAKEDEFS}/recipes.mk

define PRINT_TARGET_DEBUG_INFO
    $(info =================================================)
    $(info Target           MODULE_TARGET   = ${${1}_TARGET})
    $(info Pipeline         MODULE_PIPELINE = ${${1}_PIPELINE})
    $(info Sources          MODULE_SRCS     = ${${1}_SRCS})
    $(info Shared Libs      MODULE_SOLIBS   = ${${1}_SOLIBS})
    $(info Archive Libs     MODULE_ARLIBS   = ${${1}_ARLIBS})
    $(info Includes         MODULE_INCS     = ${${1}_INCS})
    $(info LD Lib Paths     MODULE_LDPATHS  = ${${1}_LDPATHS})
    $(info LD Libs          MODULE_LDLIBS   = ${${1}_LDLIBS})
    $(info Defs             MODULE_DEFS     = ${${1}_DEFS})
    $(info Dependencies     MODULE_DEPS     = ${${1}_DEPS})
    $(info Pre-Requisites   MODULE_PREREQS  = ${${1}_PREREQS})
    $(info GCC Excl. Flags  MODULE_GCC_EXCLUDE_FLAGS = ${${1}_GCC_EXCLUDE_FLAGS})
    $(info Derived Information:)
    $(info --------------------)
    $(info Module GID           = ${${1}_MODULE_GID})
    $(info Module Name          = ${${1}_MODULE_NAME})
    $(info Module Src Dir       = ${${1}_SRC_DIR})
    $(info Module MK            = ${${1}_MODULE_MK})
    $(info Module SOLIB Deps    = ${${1}_SOLIB_DEPS})
    $(info Module ARLIB Deps    = ${${1}_SOLIB_DEPS})
    $(info Module Objects       = ${${1}_OBJS})
    $(info Module MMD Deps      = ${${1}_MMD_DEPS})
    $(info )
endef

CXX_TARGETIDS :=
P4_TARGETIDS  :=
ASM_TARGETIDS  :=
define INCLUDE_MODULEMK
    MODULE_SRCS     :=
    MODULE_SOLIBS   :=
    MODULE_ARLIBS   :=
    MODULE_INCS     :=
    MODULE_LDPATHS  :=
    MODULE_LDLIBS   :=
    MODULE_DEFS     :=
    MODULE_DEPS     :=
    MODULE_PIPELINE :=
    MODULE_PREREQS  :=

    # MODULE_DIR can be used by module.mk to know their current
    # directory.
    export MODULE_DIR       = $(dir ${1})
    export MODULE_MK        = ${1}
    include ${1}

    TGID                     = $$(call TARGET_TO_TGID,$${MODULE_TARGET})
    $${TGID}_TARGET         := $${MODULE_TARGET}
    $${TGID}_MODULE_GID     := $$(call MODULE_PATH_TO_GID,${1})
    $${TGID}_MODULE_NAME    := ${MODULE_GID}
    $${TGID}_SRC_DIR        := $(dir ${1})
    $${TGID}_MODULE_MK      := ${1}
    $${TGID}_SRCS           := $${MODULE_SRCS}
    $${TGID}_DEFS           := $${MODULE_DEFS}
    $${TGID}_LDLIBS         := $$(addprefix -l,$${MODULE_LDLIBS})
    $${TGID}_LIBS           := $$(addprefix -l,$${MODULE_SOLIBS}) $$(addprefix -l,$${MODULE_ARLIBS})
    $${TGID}_SOLIB_DEPS     := $$(join $$(patsubst %,${BLD_OUT_DIR}/lib%_so/,$${MODULE_SOLIBS}),\
                                       $$(patsubst %,lib%.so,$${MODULE_SOLIBS}))
    $${TGID}_ARLIB_DEPS     := $$(join $$(patsubst %,${BLD_OUT_DIR}/lib%_a/,$${MODULE_ARLIBS}),\
                                       $$(patsubst %,lib%.a,$${MODULE_ARLIBS}))
    $${TGID}_INCS           := $$(addprefix -I,$${MODULE_INCS}) ${CONFIG_INCS}
    $${TGID}_LDPATHS        := $$(addprefix -L,$${MODULE_LDPATHS}) ${CONFIG_LDPATHS}
    $${TGID}_PIPELINE       := $${MODULE_PIPELINE}
    $${TGID}_DEPS           := $${MODULE_DEPS}
    $${TGID}_PREREQS        := $$(join $$(addprefix ${BLD_OUT_DIR}/,$$(subst .,_,$${MODULE_PREREQS})),\
                                       $$(addprefix /,$${MODULE_PREREQS}))

    $${TGID}_GCC_EXCLUDE_FLAGS   := $${MODULE_EXCLUDE_FLAGS}

    # Set the common flags based on the target type
    $${TGID}_FLAGS := $${MODULE_FLAGS}
    ifeq "$$(suffix $${MODULE_TARGET})" ".a"
        $${TGID}_FLAGS              += ${CONFIG_ARLIB_FLAGS}
        $${TGID}_RECIPE_TYPE        := ARLIB
    else ifeq "$$(suffix $${MODULE_TARGET})" ".so"
        $${TGID}_FLAGS              += ${CONFIG_SOLIB_FLAGS}
        $${TGID}_RECIPE_TYPE        := SOLIB
    else ifeq "$$(suffix $${MODULE_TARGET})" ".gtest"
        $${TGID}_INCS               += ${CONFIG_GTEST_INCS}
        $${TGID}_LIBS               += ${CONFIG_GTEST_LIBS}
        $${TGID}_GCC_EXCLUDE_FLAGS  += ${CONFIG_GTEST_GCC_EXCLUDE_FLAGS}
        $${TGID}_FLAGS              += ${CONFIG_GTEST_FLAGS}
        $${TGID}_LDPATHS            += ${CONFIG_GTEST_LDPATHS}
        $${TGID}_RECIPE_TYPE        := BIN
    else ifeq "$$(suffix $${MODULE_TARGET})" ".p4bin"
        $${TGID}_RECIPE_TYPE        := P4BIN
        $${TGID}_NCC_OPTS           := ${CMD_NCC_OPTS} $${MODULE_NCC_OPTS}
    else ifeq "$$(suffix $${MODULE_TARGET})" ".asmbin"
        $${TGID}_RECIPE_TYPE        := ASMBIN
        $${TGID}_CAPAS_OPTS         := ${CMD_CAPAS_OPTS} $${MODULE_CAPAS_OPTS}
    else
        $${TGID}_RECIPE_TYPE        := BIN
    endif

    ifeq "$$(suffix $${MODULE_TARGET})" ".p4bin"
        P4_TARGETIDS    += $${TGID}
    else ifeq "$$(suffix $${MODULE_TARGET})" ".asmbin"
        ASM_TARGETIDS    += $${TGID}
    else
        CXX_TARGETIDS   += $${TGID}
    endif
endef

define PROCESS_MODULEMK_TARGETS
    ${1}_BLD_OUT_DIR    := $$(addprefix ${BLD_OUT_DIR}/,${1})
    ${1}_MKTARGET       := $$(addprefix $(addprefix ${BLD_OUT_DIR}/,${1}),/$${${1}_TARGET})
endef

define PROCESS_MODULEMK_OBJS
    ifeq "$${${1}_RECIPE_TYPE}" "P4BIN"
        ${1}_OBJS   += $$(addprefix $${${1}_BLD_OUT_DIR}/,$$(addsuffix .p4o,$$(basename $${${1}_SRCS})))
    else ifeq "$${${1}_RECIPE_TYPE}" "ASMBIN"
        ${1}_OBJS   += $$(addprefix $${${1}_BLD_OUT_DIR}/,$$(addsuffix .bin,$$(basename $${${1}_SRCS})))
    else
        ${1}_OBJS   += $$(addprefix $${${1}_BLD_OUT_DIR}/,$$(addsuffix .o,$$(basename $${${1}_SRCS})))
    endif
    ${1}_GXX_FLAGS   = $$(filter-out $${${1}_GCC_EXCLUDE_FLAGS}, ${CMD_GXX_FLAGS})
endef
  
define PROCESS_MODULEMK_DEPS
    ${1}_DEPS       += $${${1}_SOLIB_DEPS} $${${1}_ARLIB_DEPS} ${COMMON_DEPS} $${${1}_PREREQS}
    ${1}_MMD_DEPS   := $${${1}_OBJS:%.o=%.d}
endef

define FILTER_TARGETS_BY_PIPELINE
    ifeq "$${${1}_PIPELINE}" "${PIPELINE}"
        ALL_TARGETS     += $${${1}_MKTARGET}
    endif
    ifeq "$${${1}_PIPELINE}" "all"
        ALL_TARGETS     += $${${1}_MKTARGET}
    endif
    ifeq "$${${1}_PIPELINE}" ""
        ALL_TARGETS     += $${${1}_MKTARGET}
    endif
endef

MODULES := $(shell find ./ -name 'module*.mk')
MODULE_PATHS = $(strip $(call CANPATH,${MODULES}))
$(call DEBUGMSG,Module Paths = ${MODULE_PATHS})
$(foreach modpath,${MODULE_PATHS}, \
    $(eval $(call INCLUDE_MODULEMK,${modpath})))

TARGETIDS := $(strip ${CXX_TARGETIDS} ${P4_TARGETIDS} ${ASM_TARGETIDS})
$(call DEBUGMSG,TargetIDs = ${TARGETIDS})
$(foreach tgid, ${TARGETIDS}, \
    $(eval $(call PROCESS_MODULEMK_TARGETS,${tgid})))
$(foreach tgid, ${TARGETIDS}, \
    $(eval $(call PROCESS_MODULEMK_OBJS,${tgid})))
$(foreach tgid, ${TARGETIDS}, \
    $(eval $(call PROCESS_MODULEMK_DEPS,${tgid})))
$(foreach tgid, ${TARGETIDS}, \
    $(eval $(call ADD_RECIPE,${tgid})))

# Add pattern rule(s) for creating compiled object code from C++ source.
CXX_TARGETIDS := $(strip ${CXX_TARGETIDS})
$(foreach tgid,${TARGETIDS},$(foreach ext, ${EXTS_CXX},\
    $(eval $(call ADD_SRC_CXX_OBJECT_RULE,${tgid},${ext}))))
# Add the 'make depend' eval rules for all CXX_TARGETS
$(foreach tgid, ${CXX_TARGETIDS}, \
    $(eval -include ${${tgid}_MMD_DEPS}))

# Add pattern rule(s) for compiling P4 code.
$(call DEBUGMSG,P4 TargetIDs = ${P4_TARGETIDS})
P4_TARGETIDS := $(strip ${P4_TARGETIDS})
$(foreach tgid,${P4_TARGETIDS},$(foreach ext, ${EXTS_P4},\
    $(eval $(call ADD_SRC_P4_OBJECT_RULE,${tgid},${ext}))))

# Add pattern rule(s) for compiling ASM code.
$(call DEBUGMSG,ASM TargetIDs = ${ASM_TARGETIDS})
ASM_TARGETIDS := $(strip ${ASM_TARGETIDS})
$(foreach tgid,${ASM_TARGETIDS},$(foreach ext, ${EXTS_ASM},\
    $(eval $(call ADD_SRC_ASM_OBJECT_RULE,${tgid},${ext}))))

$(foreach tgid, ${TARGETIDS}, \
    $(eval $(call FILTER_TARGETS_BY_PIPELINE,${tgid})))
$(call DEBUGMSG,Make Targets = ${ALL_TARGETS})

print-target-debug-info:
	$(foreach tgid, ${TARGETIDS}, \
		$(eval $(call PRINT_TARGET_DEBUG_INFO,${tgid})))
