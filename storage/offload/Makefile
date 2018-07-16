CUR_DIR:=$(PWD)
STORAGE_DIR:=$(shell dirname ${CUR_DIR})
SW_DIR:=$(shell dirname ${STORAGE_DIR})
CUR_USER:=$(shell whoami)
CUR_TIME:=$(shell date +%Y-%m-%d_%H.%M.%S)
CONTAINER_NAME:=${CUR_USER}_${CUR_TIME}

# get a shell with the dependencies image loaded, with the host filesystem mounted.
shell: build-runtime-image
	mkdir -p ${SW_DIR}/bazel-cache;
	docker run -it --rm --sysctl net.ipv6.conf.all.disable_ipv6=0 --privileged --name ${CONTAINER_NAME} -v $(SW_DIR):/sw -v $(SW_DIR)/bazel-cache:/root/.cache -v /lib/modules:/lib/modules -v /usr/src/kernels:/usr/src/kernels -w /sw/storage/offload pensando/offload bash

REGISTRY = registry.test.pensando.io:5000

build-runtime-image: install_box
	if [ "x${NO_PULL}" = "x" ]; then docker pull $(REGISTRY)/pensando/nic:1.23; fi
	cd ../.. && BOX_INCLUDE_ENV="NO_COPY" NO_COPY=1 box -t pensando/offload storage/offload/box.rb

install_box:
	@if [ ! -x /usr/local/bin/box ]; then echo "Installing box, sudo is required"; curl -sSL box-builder.sh | sudo bash; fi

MODNAME = pnso
INCDIRS = include src/osal src/osal/test src/sim third-party/freebsd-crypto/src src/drv

$(MODNAME)-y = src/osal/linux/kernel/osal_thread.o 
$(MODNAME)-y += src/osal/linux/kernel/osal_atomic.o 
$(MODNAME)-y += src/osal/linux/kernel/osal_sys.o 
$(MODNAME)-y += src/osal/linux/kernel/osal_logger.o 
$(MODNAME)-y += src/osal/linux/kernel/sim/osal_mem.o 
$(MODNAME)-y += src/osal/test/osal_test.o

$(MODNAME)-y += src/drv/pnso_cp.o
$(MODNAME)-y += src/drv/pnso_dc.o
$(MODNAME)-y += src/drv/pnso_chksum.o
$(MODNAME)-y += src/drv/pnso_hash.o
$(MODNAME)-y += src/drv/pnso_cpdc.o
$(MODNAME)-y += src/drv/pnso_cpdc_cmn.o
$(MODNAME)-y += src/drv/pnso_encrypt.o
$(MODNAME)-y += src/drv/pnso_decrypt.o
$(MODNAME)-y += src/drv/pnso_xts_cmn.o
$(MODNAME)-y += src/drv/pnso_xts.o
$(MODNAME)-y += src/drv/pnso_decompact.o

$(MODNAME)-y += src/sim/lzrw1-a.o
$(MODNAME)-y += src/sim/pnso_sim_api.o
$(MODNAME)-y += src/sim/sim_algo.o
$(MODNAME)-y += src/sim/sim_chain.o
$(MODNAME)-y += src/sim/sim_crypto.o
$(MODNAME)-y += src/sim/sim_util.o
$(MODNAME)-y += src/sim/sim_worker.o
$(MODNAME)-y += third-party/freebsd-crypto/src/crc32.o
$(MODNAME)-y += third-party/freebsd-crypto/src/rijndael-alg-fst.o
$(MODNAME)-y += third-party/freebsd-crypto/src/rijndael-api.o
$(MODNAME)-y += third-party/freebsd-crypto/src/rijndael-api-fst.o
$(MODNAME)-y += third-party/freebsd-crypto/src/sha256c.o
$(MODNAME)-y += third-party/freebsd-crypto/src/sha512c.o
$(MODNAME)-y += third-party/freebsd-crypto/src/xform_aes_xts.o

obj-m = $(MODNAME).o
ccflags-y = $(INCDIRS:%=-I$(PWD)/%)
KERN_DIR = /lib/modules/$(shell uname -r)/build
modules: modules-clean
	make -C $(KERN_DIR) M=$(PWD) modules

modules-clean:
	make -C $(KERN_DIR) M=$(PWD) clean

