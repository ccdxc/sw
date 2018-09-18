TOPDIR = ..
CUSTOM = subdirs

SUBDIRS = \
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
          apollo \
          p4pt \
          $(NULL)

COMMON_SUBDIRS = \
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
          p4pt

MSFT_GFT_SUBDIRS = $(COMMON_SUBDIRS) gft

APOLLO_SUBDIRS = \
        common-p4+ \
        apollo

.PHONY: subdirs msft_gft apollo_p4 $(SUBDIRS)
default: common_p4

CLI = ../tools/ncc/capri_cli.py
cli:
	$(CLI)

common_p4: $(COMMON_SUBDIRS)
	$(MAKE) cli

msft_gft: $(MSFT_GFT_SUBDIRS)
	$(MAKE) cli

apollo_p4: $(APOLLO_SUBDIRS)

subdirs: $(SUBDIRS)

$(SUBDIRS):
	$(MAKE) -C $@

include $(TOPDIR)/make/Makefile.inc
