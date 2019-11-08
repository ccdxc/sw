obj-$(CONFIG_INFINIBAND_IONIC_RDMA)	+= ionic_rdma.o

# $(M) is abs path of drivers-linux/drivers
ccflags-y := -I$(M)/rdma/drv/ionic/uapi -I$(M)/eth/ionic -I$(M)/../common

ionic_rdma-y :=	\
	ionic_ibdev.o ionic_queue.o ionic_ibdebug.o \
	ionic_dcqcn.o ionic_kcompat.o ionic_res.o
