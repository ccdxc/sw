
TOPDIR = ../../..
SHAREDLIBRARY = libpdutils.so

OBJDIR = $(TOPDIR)/obj
SLOBJECTS = \
		  $(OBJDIR)/directmap.o \
		  $(OBJDIR)/tcam.o \
		  $(OBJDIR)/acl_tcam.o \
		  $(OBJDIR)/tcam_entry.o \
		  $(OBJDIR)/hash.o \
		  $(OBJDIR)/hash_entry.o \
		  $(OBJDIR)/flow.o \
		  $(OBJDIR)/flow_entry.o \
		  $(OBJDIR)/flow_hint_group.o \
		  $(OBJDIR)/flow_spine_entry.o \
		  $(OBJDIR)/flow_table_entry.o \
		  $(OBJDIR)/met.o \
		  $(OBJDIR)/repl_list.o \
		  $(OBJDIR)/repl_table_entry.o \
		  $(OBJDIR)/repl_entry.o \
		  $(OBJDIR)/capri_tbl_rw.o \
		  $(OBJDIR)/capri_loader.o \
		  $(OBJDIR)/p4pd_api.o \
		  $(OBJDIR)/p4pluspd_rxdma_api.o \
		  $(OBJDIR)/p4pluspd_txdma_api.o \
		  $(OBJDIR)/p4pd_global_api.o \
		  $(NULL)

LPOSTALL = $(OBJDIR)/libpdutils_stub.so
SLOBJECTS_STUB = \
		  $(OBJDIR)/stub_directmap.o \
		  $(OBJDIR)/stub_tcam.o \
		  $(OBJDIR)/stub_acl_tcam.o \
		  $(OBJDIR)/stub_tcam_entry.o \
		  $(OBJDIR)/stub_hash.o \
		  $(OBJDIR)/stub_hash_entry.o \
		  $(OBJDIR)/stub_flow.o \
		  $(OBJDIR)/stub_flow_entry.o \
		  $(OBJDIR)/stub_flow_hint_group.o \
		  $(OBJDIR)/stub_flow_spine_entry.o \
		  $(OBJDIR)/stub_flow_table_entry.o \
		  $(OBJDIR)/stub_met.o \
		  $(OBJDIR)/stub_repl_list.o \
		  $(OBJDIR)/stub_repl_table_entry.o \
		  $(OBJDIR)/stub_repl_entry.o \
		  $(OBJDIR)/capri_tbl_rw.o \
		  $(OBJDIR)/capri_loader.o \
		  $(OBJDIR)/p4pd_api.o \
		  $(OBJDIR)/p4pluspd_rxdma_api.o \
		  $(OBJDIR)/p4pluspd_txdma_api.o \
		  $(OBJDIR)/p4pd_global_api.o \
		  $(NULL)

# LDEPENDS = $(SLOBJECTS_STUB:.o=.d)
CUSTOM_CLEAN = stub_clean

include $(TOPDIR)/make/Makefile.inc

$(OBJDIR)/libpdutils_stub.so: $(SLOBJECTS_STUB)
	g++ -g -shared -o $(OBJDIR)/libpdutils_stub.so $(SLOBJECTS_STUB)

stub_clean:
	rm -rf ../../../obj/libpdutils_stub.so
