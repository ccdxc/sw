
CROSS_COMPILE=${NICDIR}/buildroot/host/opt/ext-toolchain/bin/aarch64-linux-gnu-
CC = $(CROSS_COMPILE)gcc

TEXT_ADDR = 0x400100

OBJDIR = ../obj

CFLAGS += -Os -Wall -Werror -Wuninitialized -ffreestanding
CFLAGS += -MMD -MP -I../include
CFLAGS += -I$(NICDIR)/sdk/third-party/asic/capri/model/cap_top/csr_defines
CFLAGS += -march=armv8-a+crc
LDFLAGS += -nostdlib -nodefaultlibs -Wl,--build-id=none
LDFLAGS += -Wl,--script=boot0.lds

SOURCES = \
	bfl.c \
	board.c \
	cpld.c \
	fwsel.c \
	gpio.c \
	log.c \
	main.c \
	panic.c \
	printf.c \
	qspi.c \
	start.S \
	string.c \
	string.c \
	uart.c \
	uboot.c \
	wdt.c \
	xcpt.c \
	$(NULL)

OBJS := $(basename $(SOURCES))
OBJS := $(OBJS:%=$(OBJDIR)/%.o)

MODULE_BIN_DIR := ${BLD_OUT_DIR}/boot0_submake

PROGRAM = boot0

TARGETS = \
	$(PROGRAM) \
	$(PROGRAM).bin \
	$(PROGRAM).lst \
	$(NULL)

check_all: all check_version export

all: $(OBJDIR)/.dir $(TARGETS)

$(PROGRAM).bin: $(PROGRAM)
	@echo MAKEBIN $@
	@$(CROSS_COMPILE)objcopy -O binary $^ $@

$(PROGRAM).lst: $(PROGRAM)
	@echo MAKELIST $@
	@$(CROSS_COMPILE)objdump --disassemble $^ > $@

$(PROGRAM): $(OBJS)
	@echo LINK $@
	@$(CC) $(LDFLAGS) -Ttext=$(TEXT_ADDR) -o $@ $^

$(OBJDIR)/%.o: %.c
	@echo CC $<
	@$(CC) $(CFLAGS) -c -o $@ $<

$(OBJDIR)/%.o: %.S
	@echo CC $<
	@$(CC) $(CFLAGS) -D__ASSEMBLY__ -c -o $@ $<

version.txt: all
	@v=`perl -ane 'print int($$F[2]);' < ../include/version.h`; \
	echo "$$v `md5sum $(PROGRAM).bin`" > $@

.PHONY: check_version
check_version:
	@v=`perl -ane 'print int($$F[2]);' < ../include/version.h`; \
	echo "$$v `md5sum $(PROGRAM).bin`" > /tmp/v; \
	if ! cmp -s /tmp/v version.txt; then \
		echo "###"; \
		echo "### ERROR: boot0.bin does not match version $$v."; \
		echo "### make new_version to increment the program version." >&2; \
		echo "### make version.txt to keep the current version." >&2; \
		echo "###"; \
		rm -f /tmp/v; \
		return 1; \
	fi; \
	rm -f /tmp/v

.PHONY: export
export: $(PROGRAM).bin
	@cp $(PROGRAM).bin boards.json $(MODULE_BIN_DIR)

.PHONY: new_version
new_version:
	v=`perl -ane 'print int($$F[2]) + 1;' < ../include/version.h`; \
	echo "#define BOOT0_VERSION $$v" > ../include/version.h; \
	make version.txt

clean:
	@rm -rf $(OBJDIR) $(TARGETS)

%/.dir:
	@mkdir -p $(OBJDIR) && touch $@

-include $(OBJDIR)/*.d
