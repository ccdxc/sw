.PATH: ${.CURDIR}

SHLIBDIR?=	/usr/lib
LIB=		ionic
SHLIB_MAJOR=	1
MK_PROFILE=	no

SRCS= \
	ionic.c \
	ionic_memory.c \
	ionic_queue.c \
	ionic_verbs.c

LIBADD=		ibverbs pthread
CFLAGS+=	-I${.CURDIR}

.include <bsd.lib.mk>

