SUBDIRS = p4 \
          txdma \
          rxdma \
          $(NULL)

TOPDIR = ../..

.PHONY: subdirs $(SUBDIRS)
default: subdirs

subdirs: $(SUBDIRS)

$(SUBDIRS):
	$(MAKE) -C $@

include $(TOPDIR)/make/Makefile.inc
include $(TOPDIR)/make/Make.inc
