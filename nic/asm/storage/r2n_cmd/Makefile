src = $(wildcard *.s)
bin = $(src:.s=.bin)

LDFLAGS = 
CFLAGS = -g

all: deps asm 

deps:
	echo "Building dependencies"
	echo $(shell basename $(CURDIR))
	../../../tools/ncc/capri-ncc.py --p4-plus --asm-out --no-ohi ../../../p4/storage/$(shell basename $(CURDIR)).p4
	mv $(shell basename $(CURDIR))/asm_out/*.h include
	rm -rf $(shell basename $(CURDIR))

asm: $(bin)
	echo $(src)
	echo $(bin)
	mv $(bin) bin


.PHONY: clean 
clean:
	echo "Cleaning dependencies"
	rm -rf bin/*.bin
	rm -rf include/*.h
	rm -rf $(shell basename $(CURDIR))

%.bin: %.s
	../../../capsim/gen/bin/capas -I../../../p4/storage/common/ -I../include -I./include $< -o $@

