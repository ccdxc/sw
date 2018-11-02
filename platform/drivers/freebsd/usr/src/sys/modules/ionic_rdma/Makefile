.PATH:	${.CURDIR}/../../dev/ionic/ionic_rdma

KMOD=ionic_rdma
SRCS= \
ionic_ibdev.c \
ionic_queue.c \
device_if.h bus_if.h vnode_if.h pci_if.h \
	opt_inet.h opt_inet6.h

CFLAGS+= -I${SRCTOP}/sys/ofed/include
CFLAGS+= -I${SRCTOP}/sys/ofed/include/uapi
CFLAGS+= -I${SRCTOP}/sys/compat/linuxkpi/common/include
CFLAGS+= -I${.CURDIR}/../../ofed/include/uapi
CFLAGS+= -I${.CURDIR}/../../dev/ionic/ionic_eth
CFLAGS+= -DCONFIG_INFINIBAND_USER_MEM
CFLAGS+= -DINET -DINET6
CFLAGS+= -DFREEBSD -DHAPS

.include <bsd.kmod.mk>

CFLAGS+= -Wno-pointer-arith
