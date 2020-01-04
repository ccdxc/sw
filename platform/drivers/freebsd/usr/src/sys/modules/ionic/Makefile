.PATH:	${.CURDIR}/../../dev/ionic/ionic_eth

.include <bsd.own.mk>

KMOD=ionic
SRCS= \
ionic_api.c \
ionic_bus_pci.c \
ionic_dev.c \
ionic_lif.c \
ionic_main.c \
ionic_txrx.c \
ionic_osdep.c \
ionic_rx_filter.c \
device_if.h bus_if.h vnode_if.h pci_if.h \
        opt_inet.h opt_inet6.h opt_rss.h

.if !defined(KERNBUILDDIR)
# Import host system's kernel configuration (INVARIANTS etc.)
# Remember to do `make cleandepend` if changing KERNCONF!
KERNCONF?=    GENERIC
KERNCONFDIR=  ${SYSDIR}/${MACHINE_ARCH}/conf
SRCS+=        .kconf/opt_global.h
SRCS+=        .kconf/opt_rss.h
CFLAGS+=      -include .kconf/opt_global.h
CFLAGS+=      -include .kconf/opt_rss.h

.kconf/opt_global.h: ${KERNCONFDIR}/${KERNCONF}
	cd ${KERNCONFDIR} && config -d ${.CURDIR}/.kconf ${KERNCONF}
.endif

# NetApp physical port
#CFLAGS+= -DNETAPP_PATCH
# To disable debugging, with no option to enable at runtime
#CFLAGS+= -DIONIC_NDEBUG

CFLAGS+= -I${SRCTOP}/sys/compat/linuxkpi/common/include

.include <bsd.kmod.mk>

CFLAGS+= -Wno-pointer-arith
