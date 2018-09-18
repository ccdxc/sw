.PHONY: clean iris gft apollo

default: iris

export TOPDIR 		:= ${CURDIR}
export IRIS_DIR 	:= $(TOPDIR)/hal/iris
export GFT_DIR 		:= $(TOPDIR)/hal/gft
export APOLLO_DIR	:= $(TOPDIR)/hal/apollo2
export TOPDIR_ABS 	:= $(realpath ${CURDIR})

include $(TOPDIR)/make/Makefile.common

iris:
	@echo $(TOPDIR)
	$(MAKE) -C $(IRIS_DIR)

gft:
	@echo $(TOPDIR)
	$(MAKE) -C $(GFT_DIR)

apollo:
	@echo $(TOPDIR)
	$(MAKE) -C $(APOLLO_DIR)

clean:
	$(MAKE) -C $(IRIS_DIR) clean
	$(MAKE) -C $(GFT_DIR) clean
	$(MAKE) -C $(APOLLO_DIR) clean
