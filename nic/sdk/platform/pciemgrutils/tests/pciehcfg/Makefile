#
# Copyright (c) 2017, Pensando Systems Inc.
#

PROGRAM = pciehcfgtest
SOURCES = pciehcfgtest.c
OBJECTS = $(SOURCES:.c=.o)
DEPENDS = $(SOURCES:.c=.d)

OPT = -O2
CFLAGS += $(OPT)
CFLAGS += -MT $@ -MMD -MP -MF $*.d
CFLAGS += -I ../../../pciehost/include
LIBRARIES += ../../../pciehost/src/libpciehost.a

all: $(PROGRAM)

$(PROGRAM): $(OBJECTS) $(LIBRARIES)
	$(CC) -o $@ $^

clean:
	rm -f $(OBJECTS) $(DEPENDS) $(PROGRAM)

-include $(DEPENDS)
