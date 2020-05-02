#
# Copyright (c) 2020, Pensando Systems Inc.
#

include ${MKINFRA}/config_${ARCH}.mk

NUMAROOT        := ${SDKDIR}/third-party/libnuma
PATCHROOT       := ${SDKDIR}/pen-dpdk/
PATCHPATH       := ${SDKDIR}/lib/dpdk/patch
DPDKROOT        := ${SDKDIR}/pen-dpdk/dpdk
OUT_DIR         := ${BLD_OUT_DIR}/pen_dpdk_submake

ifeq ($(ARCH), aarch64)
    export DPDK_SIM_BLD       := n
	CONFIG_FILE=arm64-naples-linuxapp-gcc
	CROSS_CMD="CROSS=${TOOLCHAIN_PREFIX}-"
	CFLAGS_CMD="-isystem  ${NUMAROOT}/include -fPIC -O0 -g -fno-inline -fno-omit-frame-pointer"
	LDFLAGS_CMD="-L${BLD_LIB_DIR} -L${NUMAROOT}/aarch64/lib -lnuma"
	SIM_LIB=

all: dpdk
else ifeq ($(ARCH), x86_64)
    export DPDK_SIM_BLD       := y
	CONFIG_FILE=x86_64-default-linuxapp-gcc
	CROSS_CMD=
	CFLAGS_CMD="-I ${NUMAROOT}/include -I ${DPDKROOT}/include -I ${SDKDIR} -DDPDK_SIM -g -fPIC -O0 -Wno-error -L ${NUMAROOT}/x86_64/lib"
	LDFLAGS_CMD="-L${BLD_LIB_DIR} -L${NUMAROOT}/x86_64/lib -lnuma -ldpdksim -lsdkpal -llogger"
	SIM_LIB=${BLD_LIB_DIR}/libdpdksim.a
all: dpdk
else
all:
	echo "Bypassing dpdk build"
endif

dpdk:
	cd ${PATCHROOT}; patch -p0 -i ${PATCHPATH}/sim_code_no_prefix_pendpdk.patch
	make -C ${DPDKROOT} config T=${CONFIG_FILE} O=${OUT_DIR}
	make -j8 -C ${DPDKROOT} ${CROSS_CMD} CONFIG_RTE_KNI_KMOD=n CONFIG_RTE_EAL_IGB_UIO=n EXTRA_CFLAGS=${CFLAGS_CMD} EXTRA_LDFLAGS=${LDFLAGS_CMD} O=${OUT_DIR}
	cd ${OUT_DIR}/lib/; ${CMD_GCC} -shared -o libdpdk.so -Wl,--whole-archive libdpdk.a ${SIM_LIB} -Wl,--no-whole-archive
	cp ${OUT_DIR}/lib/libdpdk.so ${BLD_LIB_DIR} 
	mkdir -p ${DPDKROOT}/build/lib
	cp ${OUT_DIR}/lib/libdpdk.so ${DPDKROOT}/build/lib/ 
	cd ${PATCHROOT}; patch -p0 -R < ${PATCHPATH}/sim_code_no_prefix_pendpdk.patch

.PHONY: printvars
printvars:
	$(foreach v, $(.VARIABLES), $(info $(v) = $($(v))))
