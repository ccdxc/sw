CC      = gcc
CFLAGS  = -g
RM      = rm -f

default: all

all: pnso

pnso: pnso_main.c
	    $(CC) $(CFLAGS) -o pnso pnso_main.c

clean veryclean:
	    $(RM) pnso
