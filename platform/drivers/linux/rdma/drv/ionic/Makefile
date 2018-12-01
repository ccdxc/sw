obj-$(CONFIG_INFINIBAND_IONIC_RDMA)	+= ionic_rdma.o

ccflags-y := -I$(M)/uapi -I$(M)/../../../eth/ionic -I$(M)/../../../../common

ionic_rdma-y :=	ionic_ibdev.o ionic_queue.o ionic_ibdebug.o res.o
