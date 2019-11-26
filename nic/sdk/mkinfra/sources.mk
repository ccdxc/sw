# {C} Copyright 2018 Pensando Systems Inc. All rights reserved
#####################################################################
################# SRC SPECIFIC RULE GENERATORS ###################
#####################################################################
EXTS_PB_GO  := %.pb.go
EXTS_ASM    := %.asm %.s
EXTS_P4     := %.p4

define ADD_SRC_CXX_OBJECT_RULE
$${${1}_BLD_OUT_DIR}/${2}/%.o: ${2}/%${3} $${${1}_DEPS}
	${AT}mkdir -p $$(dir $$@)
	${NAT}${AT}echo ${NAME_GXX} $$<
	${AT}$(strip $${${1}_CMD_GXX} -c -o $$@ ${${1}_FLAGS} ${${1}_GPP_FLAGS} ${${1}_INCS} ${${1}_DEFS} -D__FNAME__=\"$$(notdir $$<)\" $$<)

$${${1}_BLD_OUT_DIR}/${2}/%.o: ${2}/%.c $${${1}_DEPS}
	${AT}mkdir -p $$(dir $$@)
	${NAT}${AT}echo ${NAME_GCC} $$<
	${AT}$(strip $${${1}_CMD_GCC} -c -o $$@ ${${1}_FLAGS} ${${1}_GXX_FLAGS} ${${1}_INCS} ${${1}_DEFS} -D__FNAME__=\"$$(notdir $$<)\" $$<)
endef

define ADD_SRC_P4_OBJECT_RULE
$${${1}_BLD_OUT_DIR}/${2}/%.p4o: ${2}/%${3} $${${1}_DEPS}
	${AT}mkdir -p $$(dir $$@)
	${NAT}${AT}echo ${${1}_NAME_P4C} $$(call CANPATH,$$<) "=>" $$(call CANPATH,$$@_build.log)
	${AT}$(strip ${${1}_CMD_P4C} ${${1}_P4C_OPTS} $$<) > $$(call CANPATH,$$@_build.log)
	${AT}touch $$@
endef

define ADD_SRC_P4_16_OBJECT_RULE
$${${1}_BLD_OUT_DIR}/${2}/%.p4o: ${2}/%${3} $${${1}_DEPS}
	${AT}mkdir -p $$(dir $$@)
	${NAT}${AT}echo ${${1}_NAME_P4C} $$(call CANPATH,$$<) "=>" $$(call CANPATH,$$@_build.log)
	${AT}$(strip ${${1}_CMD_P4C} ${${1}_P4C_OPTS} $$<) > $$(call CANPATH,$$@_build.log)
	${AT}mkdir -p ${BLD_P4GEN_DIR}/p4/dbg_out
	${AT}mv ${BLD_P4GEN_DIR}/p4/model_debug.json ${BLD_P4GEN_DIR}/p4/dbg_out
	${AT}mkdir -p ${BLD_P4GEN_DIR}/p4/p4pd
	${AT}mv ${BLD_P4GEN_DIR}/p4/memory_spec.json ${BLD_P4GEN_DIR}/p4/p4pd
	${AT}mv ${BLD_P4GEN_DIR}/p4/*.bin ${BLD_PGMBIN_DIR}
	${AT}mkdir -p ${BLD_P4GEN_DIR}/p4/asm
	${AT}mv ${BLD_P4GEN_DIR}/p4/*.asm ${BLD_P4GEN_DIR}/p4/asm
	#${AT}${CMD_P4C_ASM} ${CMD_CAPAS} ${BLD_P4GEN_DIR}/p4/asm ${BLD_OUT_DIR}/$$(patsubst %.p4bin,%_asmbin,$${${1}_TARGET})
	${AT}mkdir -p $${${1}_BIN_DIR}
	${AT}${CMD_P4C_ASM} ${CMD_CAPAS} ${BLD_P4GEN_DIR}/p4/asm $${${1}_BIN_DIR}
	${AT}touch $$@
endef

define ADD_SRC_ASM_16_OBJECT_RULE
$${${1}_BLD_OUT_DIR}/${2}/%.bin: $${${1}_DEPS}
	${AT}mkdir -p $$(dir $$@)
	${NAT}${AT}echo $(strip $$@) "=>" ${1}~${2}
	${AT}mv ${BLD_OUT_DIR}/$$(patsubst %.asmbin,%_asmbin,$${${1}_TARGET})/*.bin $$(dir $$@)
	${AT}mv ${BLD_OUT_DIR}/$$(patsubst %.asmbin,%_asmbin,$${${1}_TARGET})/*.sym $$(dir $$@)
	${AT}touch $$@
endef

define ADD_SRC_ASM_OBJECT_RULE
$${${1}_BLD_OUT_DIR}/${2}/%.bin: ${2}/%${3} $${${1}_DEPS}
	${AT}mkdir -p $$(dir $$@)
	${NAT}${AT}echo ${NAME_CAPAS} $$<
	${AT}$(strip ${CMD_CAPAS} ${${1}_CAPAS_OPTS} $$< ${${1}_INCS} ${${1}_DEFS} -o $$@ > $${${1}_BLD_OUT_DIR}/$$(patsubst %.bin,%.sym,$$(shell basename $$@)))
	${AT}touch $$@
	${AT}mkdir -p $${${1}_BIN_DIR}
	${AT}ln -sf $$@ $${${1}_BIN_DIR}
endef

define ADD_SRC_RULE_PROTO_GEN_CC
$${${1}_BLD_OUT_DIR}/${2}/%.proto_ccobj: ${2}/%.proto $${${1}_DEPS}
	${AT}mkdir -p $$(dir $$@)
	${AT}mkdir -p $${${1}_GEN_DIR}
	${NAT}${AT}echo ${NAME_PROT2CC} $$(call CANPATH,$$<)
	${AT}$(strip ${CMD_PROTOC} --cpp_out=$${${1}_GEN_DIR} --grpc_out=$${${1}_GEN_DIR} ${CMD_PROTOC_CPP_OPTS} ${${1}_INCS} ${${1}_DEFS} $$<)
	${AT}touch $$@
endef

define ADD_SRC_RULE_PROTO_GEN_PY
$${${1}_BLD_OUT_DIR}/${2}/%.proto_pyobj: ${2}/%.proto $${${1}_DEPS}
	${AT}mkdir -p $$(dir $$@)
	${AT}mkdir -p $${${1}_GEN_DIR}
	${NAT}${AT}echo ${NAME_PROT2PY} $$(call CANPATH,$$<)
	${AT}$(strip ${CMD_PROTOC} --python_out=$${${1}_GEN_DIR} --grpc_out=$${${1}_GEN_DIR} ${CMD_PROTOC_PY_OPTS} ${${1}_INCS} ${${1}_DEFS} $$<)
	${AT}touch $$@
endef

define ADD_SRC_RULE_PROTO_GEN_C
$${${1}_BLD_OUT_DIR}/${2}/%.proto_cobj: ${2}/%.proto $${${1}_DEPS}
	${AT}mkdir -p $$(dir $$@)
	${AT}mkdir -p $${${1}_GEN_DIR}
	${NAT}${AT}echo ${NAME_PROT2C} $$(call CANPATH,$$<)
	${AT}$(strip ${CMD_PROTOC_C} --c_out=$${${1}_GEN_DIR}  ${${1}_INCS} ${${1}_DEFS} $$<)
	${AT}touch $$@
endef

define ADD_SRC_RULE_PROTO_GEN_GO
$${${1}_BLD_OUT_DIR}/${2}/%.proto_gobj: ${2}/%.proto $${${1}_DEPS}
	${AT}mkdir -p $$(dir $$@)
	${AT}mkdir -p $${${1}_GEN_DIR}
	${NAT}${AT}echo ${NAME_PROT2GO} $$(call CANPATH,$$<)
	${AT}$(strip ${CMD_PROTOC} ${${1}_INCS} --gofast_out=plugins=grpc:$${${1}_GEN_DIR} ${${1}_DEFS} $$(notdir $$<))
	${AT}touch $$@
endef

define ADD_SRC_RULE_PROTO_GEN_DELPHI
$${${1}_BLD_OUT_DIR}/${2}/%.proto_delphiobj: ${2}/%.proto $${${1}_DEPS}
	${AT}mkdir -p $$(dir $$@)
	${AT}mkdir -p ${BLD_PROTOGEN_DIR}
	${NAT}${AT}echo ${NAME_PROT2DELPHI} $$(call CANPATH,$$<)
	${AT}$(strip ${CMD_PROTOC} --delphi_out=$${${1}_GEN_DIR} ${${1}_INCS} ${${1}_DEFS} $$<)
	${AT}touch $$@
endef

define ADD_SRC_RULE_PROTO_GEN_GOMETRICS
$${${1}_BLD_OUT_DIR}/${2}/%.proto_gometricsobj: ${2}/%.proto $${${1}_DEPS}
	${AT}mkdir -p $$(dir $$@)
	${NAT}${AT}echo ${NAME_PROT2GOMETRICS} $$(call CANPATH,$$<)
	${AT}$(strip ${CMD_PROTOC} --delphigo-metrics_out=${TOPDIR}/nic/delphi/proto/goproto/ ${${1}_INCS} ${${1}_DEFS} $$<)
	${AT}$(strip ${CMD_GOIMPORTS} -w ${${1}_GOIMPORTS_OPTS} ${TOPDIR}/nic/delphi/proto/goproto/$$(basename $$(notdir $$(basename $$(call CANPATH,$$<)))).dm.go) > $$(call CANPATH,$$@_build.log)
	${AT}touch $$@
endef

define ADD_SRC_RULE_PROTO_GEN_PENMETRICS
$${${1}_BLD_OUT_DIR}/${2}/%.proto_penmetricsobj: ${2}/%.proto $${${1}_DEPS}
	${AT}mkdir -p $$(dir $$@)
	${NAT}${AT}echo ${NAME_PROT2PENCTL} $$(call CANPATH,$$<)
	${AT}$(strip ${CMD_PROTOC} --grpc-gateway_out=templates=${TMAGENT_PEN_IN_DIR}/rest.yml:${TMAGENT_PEN_OUT_DIR} ${${1}_INCS} ${${1}_DEFS} $$<)
	${AT}$(strip ${CMD_PROTOC} --grpc-gateway_out=templates=${PEN_IN_DIR}/rest.yml:${PEN_OUT_DIR} ${${1}_INCS} ${${1}_DEFS} $$<)
	${AT}$(strip ${CMD_GOIMPORTS} -w ${${1}_GOIMPORTS_OPTS} ${TMAGENT_PEN_OUT_DIR}/$$(basename $$(notdir $$(basename $$(call CANPATH,$$<))))_apigen.go) > $$(call CANPATH,$$@_apigen_build.log)
	${AT}$(strip ${CMD_GOIMPORTS} -w ${${1}_GOIMPORTS_OPTS} ${PEN_OUT_DIR}/$$(basename $$(notdir $$(basename $$(call CANPATH,$$<))))_gen.go) > $$(call CANPATH,$$@_gen_build.log)
	${AT}touch $$@
endef

define ADD_SRC_RULE_PROTO_GEN_PDSASVC
$${${1}_BLD_OUT_DIR}/${2}/%.proto_pdsasvcobj: ${2}/%.proto $${${1}_DEPS}
	${AT}mkdir -p ${PDSA_SVC_OUT_DIR}
	${AT}mkdir -p $$(dir $$@)
	${NAT}${AT}echo ${NAME_PROT2PDSASVC} $$(call CANPATH,$$<)
	${AT}$(strip ${CMD_PROTOC} --grpc-gateway_out=templates=${PDSA_SVC_IN_DIR}/svc.yml:${PDSA_SVC_OUT_DIR} ${${1}_INCS} ${${1}_DEFS} $$<)
	${AT}touch $$@
endef

define ADD_SRC_RULE_PROTO_GEN_PDSAMGMT
$${${1}_BLD_OUT_DIR}/${2}/%.proto_pdsamgmtobj: ${2}/%.proto $${${1}_DEPS}
	${AT}mkdir -p ${PDSA_MGMT_OUT_DIR}
	${AT}mkdir -p $$(dir $$@)
	${NAT}${AT}echo ${NAME_PROT2PDSAMGMT} $$(call CANPATH,$$<)
	${AT}$(strip ${CMD_PROTOC} --grpc-gateway_out=templates=${PDSA_MGMT_IN_DIR}/mgmt.yml:${PDSA_MGMT_OUT_DIR} ${${1}_INCS} ${${1}_DEFS} $$<)
	${AT}touch $$@
endef

define ADD_SRC_MOCKGEN_OBJECT_RULE
${2}/%_mock.go: ${2}/%.pb.go $${${1}_DEPS}
	${AT}mkdir -p $$(dir $$@)
	${NAT}${AT}echo ${NAME_MOCKGEN} $$(call CANPATH,$$<)
	${AT}$(strip ${CMD_MOCKGEN} ${${1}_MOCKGEN_OPTS} -source=$$< -destination=$$@)
endef

define ADD_SRC_GO_IMPORTS_RULE
$${${1}_BLD_OUT_DIR}/${2}/%.goimports_done: ${2}/%${3} $${${1}_DEPS}
	${AT}mkdir -p $$(dir $$@)
	${NAT}${AT}echo ${NAME_GOIMPORT} $$(notdir $$<) "=>" $$(call CANPATH,$$@_build.log)
	${AT}$(strip ${CMD_GOIMPORTS} ${${1}_GOIMPORTS_OPTS} $$<) > $$(call CANPATH,$$@_build.log)
	${AT}touch $$@
endef

define ADD_SRC_EXPORT_LIB_RULE
#${BLD_LIB_DIR}/$(notdir ${2}): $${${1}_EXPORT_DIR}/${2}
${BLD_LIB_DIR}/$(notdir ${2}): ${2}
	${AT}mkdir -p $$(dir $$@)
	${NAT}${AT}echo ${NAME_SYMLINK} $$@
	${AT}ln -sf $$(abspath $$<) $$(abspath $$@)
endef

define ADD_SRC_EXPORT_BIN_RULE
${BLD_BIN_DIR}/$(notdir ${2}): $${${1}_EXPORT_DIR}/bin/${2} $${${1}_DEPS}
	${AT}mkdir -p $$(dir $$@)
	${NAT}${AT}echo ${NAME_SYMLINK} $$@
	${AT}ln -sf $$(abspath $$<) $$(abspath $$@)
endef

define ADD_SRC_RULE
$(foreach tgid,${1},\
    $(foreach dir,${${tgid}_SRC_DIRS},\
        $(foreach ext,${${tgid}_SRC_EXTS},\
            $(eval $(call ${2},${tgid},${dir},${ext})))))
endef

define ADD_SRC_16_RULE
$(foreach tgid,${1},\
    $(foreach dir,${${tgid}_SRC_DIRS},\
        $(eval $(call ${2},${tgid},${dir}))))
endef
