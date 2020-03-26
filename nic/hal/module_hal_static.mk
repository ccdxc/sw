# {C} Copyright 2020 Pensando Systems Inc. All rights reserved

include ${MKDEFS}/pre.mk

ifeq ($(ARCH),aarch64)
MODULE_TARGET   = hal_static.bin
else
MODULE_TARGET   = hal_static.dontuse
endif

MODULE_PIPELINE = iris
MODULE_SRCS     := ${MODULE_SRC_DIR}/main.cc
ifeq ($(PIPELINE),gft)
MODULE_SRCS     := ${MODULE_SRCS} \
                   ${MODULE_SRC_DIR}/svc_gft.cc
else
MODULE_SRCS     := ${MODULE_SRCS} \
                   ${MODULE_SRC_DIR}/svc_iris.cc
endif
MODULE_DEPS     = libpacket_parser.lib
MODULE_ARLIBS_BEGIN = -Wl,--whole-archive -Wl,--start-group
MODULE_ARLIBS_END = -l:libsdkcapri_csrint.a -Wl,--end-group \
			-Wl,--no-whole-archive
MODULE_ARLIBS	= \
	agent_api \
	asicpd \
	bitmap \
	block_list \
	bm_allocator \
	catalog \
	commonproto \
	core \
	delphimessenger \
	delphisdk \
	delphishm \
	delphiutils \
	devapi_iris \
	device \
	directmap \
	dropstatsproto \
	event_thread \
	eventmgr \
	events_queue \
	events_recorder \
	evutils \
	flowstatsproto \
	fsm \
	fte \
	fte_iris \
	ftestatsproto \
	hal_lib \
	hal_mem \
	hal_src \
	hal_svc \
	hal_svc_gen \
	haldelphi \
	halproto \
	halsysmgr \
	haltrace \
	halupgrade \
	hash \
	hbmhash \
	heartbeat \
	ht \
	indexer \
	intrutils \
	ipc \
	lfq \
	lif_mgr \
	linkmgr_libsrc \
	linkmgrdelphi \
	linkmgrproto \
	list \
	logger \
	marvell \
	misc \
	mmgr \
	mnet \
	nat \
	nicmgr \
	eth_p4plus \
	nicmgr_iris \
	nicmgrproto \
	p4pd_common_p4plus_rxdma \
	p4pd_common_p4plus_txdma \
	p4pd_iris \
	pal \
	pciemgr_if \
	pd_acl_tcam \
	pd_cpupkt \
	pd_iris \
	pd_met \
	pd_wring \
	pdaccel\
	pdcapri \
	pdcommon \
	penipc \
	penipc_ev \
	penlog \
	periodic \
	rdmamgr_iris \
	rte_indexer \
	rulestatsproto \
	sdkasiccmn \
	sdkasicpd \
	sdkasicrw \
	sdkcapri \
	sdkcapri_asicrw_if \
	sdkfru \
	sdkftl \
	sdklinkmgr \
	sdklinkmgrcsr \
	sdkp4 \
	sdkp4loader \
	sdkpal \
	sdkplatformutils \
	sdkring \
	sdkxcvrdriver \
	shmmgr \
	slab \
	sldirectmap \
	sysmgr \
	tcam \
	thread \
	timerfd \
	tls_api \
	tls_pse \
	trace \
	twheel \
	upgrade_app \
	upgradeutils \
	utils \
	vmotion \

MODULE_SOLIBS   = \
	cfg_plugin_nvme \
        cfg_plugin_accel \
        cfg_plugin_aclqos \
        cfg_plugin_lif \
        cfg_plugin_mcast \
        cfg_plugin_ncsi \
        cfg_plugin_nw \
        cfg_plugin_rdma \
        cfg_plugin_sfw \
        cfg_plugin_tcp_proxy \
        cfg_plugin_telemetry \
        plugin_alg_utils \
        plugin_classic \
        plugin_ep_learn_common \
        plugin_sfw_pkt_utils \
	sdkcapri_asicrw_if

MODULE_LDLIBS        = zmq ${NIC_HAL_ALL_LDLIBS} ev tins
include ${MKDEFS}/post.mk
