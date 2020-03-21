# SPDX-License-Identifier: GPL-2.0 OR Linux-OpenIB

# internal version: $(M) is abs path of drivers-linux/drivers
ccflags-y := -I$(M)/rdma/drv/ionic/uapi -I$(M)/eth/ionic -I$(M)/../common
# upstream version
# ccflags-y :=  -I $(srctree)/drivers/net/ethernet/pensando/ionic

obj-$(CONFIG_INFINIBAND_IONIC)	+= ionic_rdma.o

ionic_rdma-y :=	\
	ionic_ibdev.o ionic_queue.o ionic_sysfs.o \
	ionic_controlpath.o ionic_datapath.o \
	ionic_admin.o ionic_pgtbl.o ionic_hw_stats.o \
	ionic_dcqcn.o ionic_res.o
