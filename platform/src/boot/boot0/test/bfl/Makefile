
CC = gcc

OBJDIR = obj

CFLAGS += -O3 -Wall -Werror -Wuninitialized
CFLAGS += -MMD -MP

SOURCES = \
	bfl.c \
	main.c \
	flash.c \
	$(NULL)

OBJS := $(basename $(SOURCES))
OBJS := $(OBJS:%=$(OBJDIR)/%.o)

PROGRAM = main

all: $(OBJDIR)/.dir $(PROGRAM)

$(PROGRAM): $(OBJS)
	@echo LINK $@
	@$(CC) $(LDFLAGS) -o $@ $^

$(OBJDIR)/%.o: %.c
	@echo CC $<
	@$(CC) $(CFLAGS) -c -o $@ $<

clean:
	@rm -rf $(OBJDIR) $(PROGRAM)

%/.dir:
	@mkdir -p $(OBJDIR) && touch $@

-include $(OBJDIR)/*.d
