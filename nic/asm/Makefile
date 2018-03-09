LIBDIRS = \
          iris \
          common-p4+ \
          proxy \
          rdma \
          ipsec-p4+ \
          storage \
          cpu-p4plus \
          gc \
          app-redir-p4+ \
          p4pt \
          eth \
          ipfix \
          adminq \
          gft \
          p4pt \
          $(NULL)

IRIS_SUBDIRS = \
          iris \
          common-p4+ \
          proxy \
          rdma \
          ipsec-p4+ \
          storage \
          cpu-p4plus \
          gc \
          app-redir-p4+ \
          p4pt \
          eth \
          ipfix \
          adminq \
          p4pt \
          $(NULL)

MSFT_GFT_SUBDIRS= \
          common-p4+ \
          proxy \
          rdma \
          ipsec-p4+ \
          storage \
          cpu-p4plus \
          gc \
          app-redir-p4+ \
          p4pt \
          eth \
          adminq \
          gft \
          p4pt \
          $(NULL)

PGMDIRS = $(NULL)

SUBDIRS = $(LIBDIRS) $(PGMDIRS)

TOPDIR = ..

.PHONY: subdirs iris msft_gft $(SUBDIRS)
default: iris

iris: $(IRIS_SUBDIRS)

msft_gft: $(MSFT_GFT_SUBDIRS)

$(SUBDIRS):
	make -C $@

include $(TOPDIR)/make/Makefile.inc
include $(TOPDIR)/make/Make.inc
