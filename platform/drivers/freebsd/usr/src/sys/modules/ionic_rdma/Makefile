.PATH:	${.CURDIR}/../../dev/ionic/ionic_rdma

KMOD=ionic_rdma
SRCS= \
ionic_sysctl.c \
ionic_ibdev.c \
ionic_dcqcn.c \
ionic_queue.c \
ionic_res.c \
ionic_admin.c \
ionic_controlpath.c \
ionic_datapath.c  \
ionic_pgtbl.c \
ionic_stats.c \
device_if.h bus_if.h vnode_if.h pci_if.h \
	opt_inet.h opt_inet6.h

.if !defined(KERNBUILDDIR)
# Import host system's kernel configuration (INVARIANTS etc.)
# Remember to do `make cleandepend` if changing KERNCONF!
KERNCONF?=    GENERIC
KERNCONFDIR=  /usr/src/sys/${MACHINE_ARCH}/conf
SRCS+=        .kconf/opt_global.h
CFLAGS+=      -include .kconf/opt_global.h

.kconf/opt_global.h: ${KERNCONFDIR}/${KERNCONF}
	cd ${KERNCONFDIR} && config -d ${.CURDIR}/.kconf ${KERNCONF}
.endif

CFLAGS+= -I${SRCTOP}/sys/ofed/include
CFLAGS+= -I${SRCTOP}/sys/ofed/include/uapi
CFLAGS+= -I${SRCTOP}/sys/compat/linuxkpi/common/include
CFLAGS+= -I${.CURDIR}/../../ofed/include/uapi
CFLAGS+= -I${.CURDIR}/../../dev/ionic/ionic_eth
CFLAGS+= -DINET -DINET6 -DCONFIG_INFINIBAND_USER_MEM

.include <bsd.kmod.mk>

CFLAGS+= -Wno-pointer-arith
CFLAGS+= -Werror
