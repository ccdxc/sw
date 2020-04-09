# {C} Copyright 2018 Pensando Systems Inc. All rights reserved

#####################################################################
################ TARGET SPECIFIC RECIPE GENERATORS ##################
#####################################################################

define ADD_RECIPE_FOR_LIB
$$(patsubst %.lib,%.a,$${${1}_MKTARGET}): $${${1}_OBJS} $${${1}_DEPS}
	${NAT}${AT}echo ${NAME_MKTARGET} $$(notdir $$@)
	${AT}mkdir -p $$(dir $$@)
	${AT}$$(strip $${CMD_AR} $${CMD_AR_FLAGS} $$@ $${${1}_OBJS})
	${AT}mkdir -p ${BLD_LIB_DIR}
	${NAT}${AT}echo ${NAME_SYMLINK} ${BLD_LIB_DIR}/$$(notdir $$@)
	${AT}ln -sf $$@ ${BLD_LIB_DIR}/

$$(patsubst %.lib,%.so,$${${1}_MKTARGET}): $${${1}_OBJS} $${${1}_DEPS}
	${NAT}${AT}echo ${NAME_MKTARGET} $$(notdir $$@)
	${AT}$$(strip ${CMD_GXX} -o $$@ -shared $${${1}_OBJS} $${${1}_LDFLAGS} $${${1}_LDPATHS} $${${1}_LIBS} $${${1}_LDLIBS})
	${AT}mkdir -p ${BLD_LIB_DIR}
	${NAT}${AT}echo ${NAME_SYMLINK} ${BLD_LIB_DIR}/$$(notdir $$@)
	${AT}ln -sf $$@ ${BLD_LIB_DIR}/

$${${1}_MKTARGET}: $$(patsubst %.lib,%.so,$${${1}_MKTARGET}) $$(patsubst %.lib,%.a,$${${1}_MKTARGET})
	${NAT}${AT}echo ${NAME_MKTARGET} $$(notdir $$@)
	${AT}touch $$@

endef

define ADD_RECIPE_FOR_BIN
$${${1}_MKTARGET}: $${${1}_OBJS} $${${1}_DEPS} ${EXPORT_PREREQS}
	${NAT}${AT}echo ${NAME_MKTARGET} $$(notdir $$@)
	${AT}$$(strip ${CMD_GXX} -o $$@ $${${1}_OBJS}) $${${1}_LDFLAGS} $${${1}_LDPATHS} $${${1}_LIBS} $${${1}_LDLIBS}
	${AT}mkdir -p ${BLD_BIN_DIR}
	${NAT}${AT}echo ${NAME_SYMLINK} ${BLD_BIN_DIR}/$$(basename $${${1}_TARGET})
	${AT}ln -sf $$@ ${BLD_BIN_DIR}/$$(basename $${${1}_TARGET})
endef

define ADD_RECIPE_FOR_P4BIN
$${${1}_MKTARGET}: $${${1}_OBJS} $${${1}_DEPS}
	${NAT}${AT}echo ${NAME_MKTARGET} $$(notdir $$@)
	${AT}touch $$@
endef

define ADD_RECIPE_FOR_ASMBIN
$${${1}_MKTARGET}: $${${1}_OBJS} $${${1}_DEPS}
	${NAT}${AT}echo ${NAME_MKTARGET} $$(notdir $$@)
	${AT}touch $$@
endef

define ADD_RECIPE_FOR_PROTO
$${${1}_MKTARGET}: $${${1}_OBJS} $${${1}_DEPS}
	${NAT}${AT}echo ${NAME_MKTARGET} $$(notdir $$@)
	${AT}touch $$@
endef

define ADD_RECIPE_FOR_UPGFSMGEN
$${${1}_MKTARGET}: $${${1}_DEPS}
	${AT}mkdir -p $$(dir $$@)
	${NAT}${AT}echo ${NAME_UPGFSMGEN} $$(notdir $$@) "=>" $$(call CANPATH,$$@_build.log)
	${AT}$${${1}_BASECMD} $${${1}_ARGS} > $$(call CANPATH,$$@_build.log) || (echo "UPGFSMGEN failed $$?"; exit 1)
	${AT}touch $$@
endef

define ADD_RECIPE_FOR_MEMRGNS
$${${1}_MKTARGET}: $${${1}_DEPS}
	${AT}mkdir -p $$(dir $$@)
	${NAT}${AT}echo ${NAME_MEMRGNS} $$(notdir $$@) "=>" $$(call CANPATH,$$@_build.log)
	${AT}$${${1}_BASECMD} $${${1}_ARGS} > $$(call CANPATH,$$@_build.log) || (echo "MEMRGNS failed $$?"; exit 1)
	${AT}touch $$@
endef

define ADD_RECIPE_FOR_SVCGEN
$${${1}_MKTARGET}: $${${1}_DEPS}
	${AT}mkdir -p $$(dir $$@)
	${NAT}${AT}echo ${NAME_SVCGEN} $$(notdir $$@) "=>" $$(call CANPATH,$$@_build.log)
	${AT}python3 ${TOPDIR}/nic/tools/hal/hal_svc_generator.py > $$(call CANPATH,$$@_build.log)
	${AT}touch $$@
endef

define ADD_RECIPE_FOR_MOCKGEN
$${${1}_MKTARGET}: $${${1}_OBJS} $${${1}_DEPS}
	${AT}mkdir -p $$(dir $$@)
	${NAT}${AT}echo ${NAME_MKTARGET} $$(notdir $$@)
	${AT}touch $$@
endef

define ADD_RECIPE_FOR_GOIMPORTS
$${${1}_MKTARGET}: $${${1}_OBJS} $${${1}_DEPS}
	${AT}mkdir -p $$(dir $$@)
	${NAT}${AT}echo ${NAME_MKTARGET} $$(notdir $$@)
	${AT}touch $$@
endef

define ADD_RECIPE_FOR_EXPORT
$${${1}_MKTARGET}: $${${1}_OBJS} $${${1}_DEPS}
	${AT}mkdir -p $$(dir $$@)
	${NAT}${AT}echo ${NAME_MKTARGET} $$(notdir $$@)
	${AT}touch $$@
endef

define ADD_RECIPE_FOR_TENJIN
$${${1}_MKTARGET}: $${${1}_DEPS}
	${AT}mkdir -p $$(dir $$@)
	${NAT}${AT}echo ${NAME_TENJIN} $$(call CANPATH,$${${1}_TEMPLATE})
	${AT} $${${1}_BASECMD} $${${1}_GENERATOR} --template $${${1}_TEMPLATE} \
	                                          --outfile $${${1}_OUTFILE} \
											  --args $${${1}_ARGS}
	${AT}touch $$@
endef

define ADD_RECIPE_FOR_GOBIN
$${${1}_MKTARGET}: $${${1}_OBJS} $${${1}_DEPS}
	${NAT}${AT}echo ${NAME_GOBUILD} $${${1}_GOPKG}
	${AT}$$(strip ${CMD_GOBIN} $${${1}_FLAGS}) -o $$@ $${${1}_GOPKG}
	${AT}mkdir -p ${BLD_BIN_DIR}
	${NAT}${AT} echo ${NAME_UPX} $$@
	${AT}((${TOPDIR}/bin/upx -v $$@) || (echo "future: strace -o upx-strace.log $$@"))
	${NAT}${AT}echo ${NAME_SYMLINK} ${BLD_BIN_DIR}/$$(basename $${${1}_TARGET})
	${AT}ln -sf $$@ ${BLD_BIN_DIR}/$$(basename $${${1}_TARGET})
endef

define ADD_RECIPE_FOR_SUBMAKE
$${${1}_MKTARGET}: $${${1}_DEPS}
	${AT}mkdir -p $$(dir $$@)
	${NAT}${AT}echo ${NAME_SUBMAKE} $${${1}_MK_DIR} "=>" $$(call CANPATH,$$@_submake.log)
	${AT}$$(strip ${MAKE} -j1 -C $${${1}_MK_DIR}) > $$(call CANPATH,$$@_submake.log)
	${AT}touch $$@
endef

define ADD_RECIPE_FOR_POSTGEN_MK
    ifeq "$${${1}_ASIC}" "$${ASIC}"
        ifeq "$${${1}_PIPELINE}" "$${PIPELINE}"
            ifeq "$${${1}_FWTYPE}" "$${FWTYPE}"
$${${1}_MK_DIR}/${2}: $${${1}_MKTARGET}
	${AT}touch $$@
            endif
        endif
    endif
endef

define ADD_RECIPE_FOR_SWIGCLI
$${${1}_MKTARGET}: $${${1}_DEPS}
	${AT}mkdir -p $$(dir $$@)
	${NAT}${AT}echo ${CMD_SWIGCLI} ${NAME_SWIGCLI} $${${1}_SRCS}
	${AT}$$(strip ${CMD_SWIG} $${${1}_FLAGS} $${${1}_INCS} -o $${${1}_SRC_DIRS}/swig.cc $${${1}_SRCS})
	${AT}touch $$@
endef

define ADD_RECIPE
    ifeq "$${${1}_ASIC}" "$${ASIC}"
        ifeq "$${${1}_PIPELINE}" "$${PIPELINE}"
            ifeq "$${${1}_FWTYPE}" "$${FWTYPE}"
                $(call ADD_RECIPE_FOR_${${1}_RECIPE_TYPE},${1})
                $${${1}_TARGET}: $${${1}_MKTARGET}
            endif
        endif
    endif
endef
