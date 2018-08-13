.PHONY: clean iris gft apollo agent

default: iris

export TOPDIR 		:= ${CURDIR}
export IRIS_DIR 	:= $(TOPDIR)/hal/iris
export GFT_DIR 		:= $(TOPDIR)/hal/gft
export APOLLO_DIR	:= $(TOPDIR)/hal/apollo2/

include $(TOPDIR)/make/Makefile.common

iris:
	@echo $(TOPDIR)
	$(MAKE) --directory $(IRIS_DIR)

gft:
	@echo $(TOPDIR)
	$(MAKE) --directory $(GFT_DIR)

apollo2:
	@echo $(TOPDIR)
	$(MAKE) --directory $(APOLLO_DIR)

clean:
	$(MAKE) --directory $(IRIS_DIR) clean
	$(MAKE) --directory $(GFT_DIR) clean
	$(MAKE) --directory $(APOLLO_DIR) clean
