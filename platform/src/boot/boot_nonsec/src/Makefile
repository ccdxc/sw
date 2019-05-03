
TOP = ../../../../..

CROSS_COMPILE ?= /tool/toolchain/aarch64-1.1/bin//aarch64-linux-gnu-
CC = $(CROSS_COMPILE)gcc

ASIC_HDRS = $(TOP)/nic/sdk/third-party/asic/capri/model/cap_top/csr_defines

KEY_PATH           ?= ../test_keys
DEV_CERTIFICATE    ?= $(KEY_PATH)/host_developer_certificate.cert
DEV_SIGNING_KEY    ?= $(KEY_PATH)/host_developer_sign.sk
DEV_ENCRYPTION_KEY ?= $(KEY_PATH)/host_developer_encr.kek
IMAGE_VERSION      ?= 1

PACK_FIRMWARE = ../util/pack_firmware.py

TEXT_ADDR = 0x400000

OBJDIR = ../obj

CFLAGS += -O3 -Wall -Werror -Wuninitialized -ffreestanding
CFLAGS += -MMD -MP -I../include -I$(ASIC_HDRS)
CFLAGS += -DTEXT_ADDR=$(TEXT_ADDR)
LDFLAGS = -Wl,--script=boot_nonsec.lds
LDFLAGS += -nostdlib -nodefaultlibs -Ttext=$(TEXT_ADDR) -Wl,--build-id=none

SOURCES = \
	efuse.c \
	esecure.c \
	main.c \
	panic.c \
	start.S \
	uart.c \
	$(NULL)

PROGRAM = boot_nonsec

OBJS := $(basename $(SOURCES))
OBJS := $(OBJS:%=$(OBJDIR)/%.o)

TARGETS = \
	$(PROGRAM) \
	$(PROGRAM).bin \
	$(PROGRAM).lst \
	$(NULL)

SIGNED_TARGETS = \
	$(PROGRAM).img \
	$(PROGRAM).img-encrypted \
	$(NULL)

all: $(OBJDIR)/.dir $(TARGETS)

signed: $(OBJDIR)/.dir $(TARGETS) $(SIGNED_TARGETS)

$(PROGRAM).img-encrypted: $(PROGRAM).bin
	@echo SIGN+ENCRYPT $@
	@$(PACK_FIRMWARE) \
	    --input $^ \
	    --version $(IMAGE_VERSION) \
	    --rootkeys_sz 32 \
	    --algo ecdsa_p384 \
	    --cert $(DEV_CERTIFICATE) \
	    --sign $(DEV_SIGNING_KEY) \
	    --encrypt $(DEV_ENCRYPTION_KEY) \
	    --output $@ >/dev/null

$(PROGRAM).img: $(PROGRAM).bin
	@echo SIGN $@
	@$(PACK_FIRMWARE) \
		--input $^ \
		--version $(IMAGE_VERSION) \
		--rootkeys_sz 32 \
		--algo ecdsa_p384 \
		--cert $(DEV_CERTIFICATE) \
		--sign $(DEV_SIGNING_KEY) \
		--output $@ >/dev/null

$(PROGRAM).bin: $(PROGRAM)
	@echo MAKEBIN $@
	@$(CROSS_COMPILE)objcopy -O binary $^ $@

$(PROGRAM).lst: $(PROGRAM)
	@echo MAKELIST $@
	@$(CROSS_COMPILE)objdump --disassemble $^ > $@

$(PROGRAM): $(OBJS)
	@echo LINK $@
	@$(CC) $(LDFLAGS) -o $@ $^

clean:
	@rm -rf $(OBJDIR) $(TARGETS)

%/.dir:
	@mkdir -p $(OBJDIR) && touch $@

$(OBJDIR)/%.o: %.c
	@echo CC $<
	@$(CC) $(CFLAGS) -c -o $@ $<

$(OBJDIR)/%.o: %.S
	@echo CC $<
	@$(CC) $(CFLAGS) -c -o $@ $<

-include $(OBJDIR)/*.d
