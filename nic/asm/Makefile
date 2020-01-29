LIBDIRS = \
          common-p4+ \
          proxy \
          rdma \
          smbdc \
          ipsec-p4+ \
          storage \
          cpu-p4plus \
          gc \
          app-redir-p4+ \
          ftl_dev/scanner_session \
          p4pt \
          eth \
          ipfix \
          adminq \
          gft \
          $(NULL)

IRIS_SUBDIRS = \
          common-p4+ \
          proxy \
          rdma \
          nvme \
          smbdc \
          ipsec-p4+ \
          storage \
          cpu-p4plus \
          gc \
          app-redir-p4+ \
          ftl_dev/scanner_session \
          p4pt \
          eth \
          ipfix \
          adminq \
          $(NULL)

MSFT_GFT_SUBDIRS= \
          common-p4+ \
          proxy \
          rdma \
          smbdc \
          ipsec-p4+ \
          storage \
          cpu-p4plus \
          gc \
          app-redir-p4+ \
          ftl_dev/scanner_session \
          p4pt \
          eth \
          adminq \
          gft \
          $(NULL)

APOLLO_SUBDIRS = \
          apollo \
          $(NULL)

PGMDIRS = $(NULL)

SUBDIRS = $(LIBDIRS) $(PGMDIRS)

TOPDIR = ..

.PHONY: subdirs msft_gft apollo $(SUBDIRS)
default: iris

iris: $(IRIS_SUBDIRS)

msft_gft: $(MSFT_GFT_SUBDIRS)

apollo: $(APOLLO_SUBDIRS)

$(SUBDIRS):
        $(MAKE) -C $@

include $(TOPDIR)/make/Makefile.inc
include $(TOPDIR)/make/Make.inc
