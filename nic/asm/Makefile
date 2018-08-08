LIBDIRS = \
          iris \
          common-p4+ \
          proxy \
          rdma \
          smbdc \
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
          tyche \
          apollo \
          $(NULL)

IRIS_SUBDIRS = \
          iris \
          common-p4+ \
          proxy \
          rdma \
          smbdc \
          ipsec-p4+ \
          storage \
          cpu-p4plus \
          gc \
          app-redir-p4+ \
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
          p4pt \
          eth \
          adminq \
          gft \
          $(NULL)

APOLLO2_SUBDIRS = \
          apollo2 \
		  $(NULL)

PGMDIRS = $(NULL)

SUBDIRS = $(LIBDIRS) $(PGMDIRS)

TOPDIR = ..

.PHONY: subdirs iris msft_gft $(SUBDIRS)
default: iris

iris: $(IRIS_SUBDIRS)

msft_gft: $(MSFT_GFT_SUBDIRS)

apollo2: $(APOLLO2_SUBDIRS)

$(SUBDIRS):
	$(MAKE) -C $@

include $(TOPDIR)/make/Makefile.inc
include $(TOPDIR)/make/Make.inc
