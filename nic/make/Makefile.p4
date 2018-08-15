SWDIR = $(TOPDIR)/../
BLD_DIR = $(TOPDIR)/build/$(PIPELINE)/
GEN_DIR = $(BLD_DIR)/gen/datapath/
OBJ_DIR = $(BLD_DIR)/obj/
TARGET = $(GEN_DIR)/$(PROJ)/.p4bld

NCC = $(TOPDIR)/tools/ncc/capri-ncc.py
NCC_OPTIONS = --asm-out --pd-gen --gen-dir $(GEN_DIR) --cfg-dir $(OBJ_DIR)/pgm_bin
NCC_OPTIONS += --split-deparse-only-headers --pipeline $(PIPELINE)
NCC_OPTIONS += --fe-flags="-I$(SWDIR)"

ifeq ($(PLATFORM),haps)
    NCC_OPTIONS += --target=haps --fe-flags="-DPLATFORM_HAPS -I$(SWDIR)"
endif

deps = $(shell find . -name '*.p4' -o -name '*.h')
deps += $(shell find $(TOPDIR)/p4/include -name '*')
deps += $(shell find $(TOPDIR)/tools/ncc -name '*')
$(TARGET): $(deps)
	$(NCC) $(NCC_OPTIONS) $(PROJ).p4
	touch $@

clean:
	rm -rf $(GEN_DIR)/$(PROJ)/
	rm -rf $(OBJ_DIR)/$(PROJ)/
	rm -rf $(TARGET)

default: $(TARGET)
