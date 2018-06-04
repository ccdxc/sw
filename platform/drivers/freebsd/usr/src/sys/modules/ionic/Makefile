# $FreeBSD: todo version tag $
.PATH:	${SRCTOP}/sys/dev/ionic/ionic_eth

KMOD=ionic
SRCS= \
ionic_api.c \
ionic_bus_pci.c \
ionic_dev.c \
ionic_lif.c \
ionic_main.c \
ionic_txrx.c \
device_if.h bus_if.h vnode_if.h pci_if.h \
        opt_inet.h opt_inet6.h opt_rss.h

CFLAGS+= -I${SRCTOP}/sys/ofed/include
CFLAGS+= -I${SRCTOP}/sys/compat/linuxkpi/common/include

.include <bsd.kmod.mk>

CFLAGS+= -Wno-pointer-arith
