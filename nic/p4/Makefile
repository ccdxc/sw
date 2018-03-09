TOPDIR = ..
CUSTOM = subdirs

SUBDIRS = \
          iris \
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

IRIS_SUBDIRS = \
          iris \
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
          p4pt \
          $(NULL)

MSFT_GFT_SUBDIRS = \
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
          p4pt \
          $(NULL)

.PHONY: subdirs iris_p4 msft_gft $(SUBDIRS)
default: iris_p4

iris_p4: $(IRIS_SUBDIRS)

msft_gft: $(MSFT_GFT_SUBDIRS)

subdirs: $(SUBDIRS)

$(SUBDIRS):
	make -C $@

include $(TOPDIR)/make/Makefile.inc
