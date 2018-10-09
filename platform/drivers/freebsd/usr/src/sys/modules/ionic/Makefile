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
device_if.h bus_if.h vnode_if.h pci_if.h \
        opt_inet.h opt_inet6.h opt_rss.h

CFLAGS+= -DFREEBSD -DHAPS
CFLAGS+= -I${SRCTOP}/sys/ofed/include
CFLAGS+= -I${SRCTOP}/sys/compat/linuxkpi/common/include

.include <bsd.kmod.mk>

CFLAGS+= -Wno-pointer-arith
