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
          smbdc \
          cpu-p4+ \
          gc \
          app-redir-p4+ \
          eth \
          ipfix \
          adminq \
          gft \
          tyche \
          apollo \
          apollo2 \
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
          smbdc \
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
          smbdc \
          cpu-p4+ \
          gc \
          app-redir-p4+ \
          eth \
          ipfix \
          adminq \
          gft \
          p4pt \
          $(NULL)

APOLLO_SUBDIRS = \
          common-p4+ \
          proxy \
          proxy/tls \
          ipsec-p4+ \
          storage \
          rdma \
          smbdc \
          cpu-p4+ \
          gc \
          app-redir-p4+ \
          eth \
          ipfix \
          adminq \
          apollo \
          p4pt \
          $(NULL)

APOLLO2_SUBDIRS = \
          common-p4+ \
          proxy \
          proxy/tls \
          ipsec-p4+ \
          storage \
          rdma \
          smbdc \
          cpu-p4+ \
          gc \
          app-redir-p4+ \
          eth \
          ipfix \
          adminq \
          apollo2 \
          p4pt \
          $(NULL)

.PHONY: subdirs iris_p4 msft_gft apollo_p4 apollo2_p4 $(SUBDIRS)
default: iris_p4

CLI = ../tools/ncc/capri_cli.py
cli:
	$(CLI)

iris_p4: $(IRIS_SUBDIRS)
	$(MAKE) cli

msft_gft: $(MSFT_GFT_SUBDIRS)
	$(MAKE) cli

apollo_p4: $(APOLLO_SUBDIRS)

apollo2_p4: $(APOLLO2_SUBDIRS)

subdirs: $(SUBDIRS)

$(SUBDIRS):
	$(MAKE) -C $@

include $(TOPDIR)/make/Makefile.inc
