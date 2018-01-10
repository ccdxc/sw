TOPDIR = ..
CUSTOM = subdirs

SUBDIRS = \
          nw \
          common-p4+ \
          proxy \
          proxy/tls \
          ipsec-p4+ \
          storage \
          rdma \
          cpu-p4+ \
          gc \
          app-redir-p4+ \
          eth \
          ipfix \
          adminq \
          gft \
          oracle \
          p4pt \
          $(NULL)

.PHONY: subdirs $(SUBDIRS)
default: subdirs

subdirs: $(SUBDIRS)

$(SUBDIRS):
	$(MAKE) -C $@

include $(TOPDIR)/make/Makefile.inc
