
TOPDIR = ../../../..
SHAREDLIBRARY = libcapricsr.so
SYSTEM_LIBRARIES = -lmodelclient -lcapri

LCXXFLAGS = \
			-L../../../../obj \
			-L../../../../model_sim/build \
			-I. \
			-I.. \
			-I../.. \
			-I../../../../include \
			-I../../../../model_sim/include \
			-I../../../../third-party/spdlog/include \
			-I/usr/local/include \
			-g -Wno-unused-variable -Wno-sign-compare -Wno-return-type \
			$(NULL)

SOURCES = \
    cap_top_csr.cc \
    cap_csr_base.cc \
    cap_pr_csr.cc \
    cap_pt_csr.cc \
    cap_psp_csr.cc \
    cap_ppa_csr.cc \
    cap_prd_csr.cc \
    cap_ptd_csr.cc \
    cap_te_csr.cc \
    cap_mpu_csr.cc \
    cap_pics_csr.cc \
    cap_pxb_csr.cc \
    cap_pp_csr.cc \
    cap_npv_csr.cc \
    cap_pict_csr.cc \
    cap_txs_csr.cc \
    cap_pbc_csr.cc \
    cap_pbm_csr.cc \
    cap_wa_csr.cc \
    cap_he_csr.cc \
    cap_dpr_csr.cc \
    cap_dpp_csr.cc \
    cap_dpphdr_csr.cc \
    cap_dprstats_csr.cc \
    cap_dppstats_csr.cc \
    cap_dpphdrfld_csr.cc \
    cap_dprmem_csr.cc \
    cap_dprcfg_csr.cc \
    cap_dppcsum_csr.cc \
    cap_dprhdrfld_csr.cc \
    cap_pxp_csr.cc \
    cap_pxc_csr.cc \
    cap_dppmem_csr.cc \
    LogMsg.cc \
    cpu.cc \
    $(NULL)

include $(TOPDIR)/make/Makefile.inc
