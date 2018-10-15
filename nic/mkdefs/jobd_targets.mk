# {C} Copyright 2018 Pensando Systems Inc. All rights reserved.
JOBD_PREREQS:= all

.PHONY: ${JOBD_PREREQS}
jobd/codesync: all
	@$(eval DIFFS=`git ls-files --exclude-standard --modified --others`)
	@echo "Found the following uncommitted files, if any"
	@echo $(DIFFS)
	@test -z "$(DIFFS)"

.PHONY: jobd/dol/rdma_ext
jobd/dol/rdma_ext: ${JOBD_PREREQS}
	${NICDIR}/run.py --topo rdma --feature rdma_ext,rdma_perf

.PHONY: jobd/dol/rdma
jobd/dol/rdma: ${JOBD_PREREQS}
	${NICDIR}/run.py --topo rdma --feature rdma

.PHONY: jobd/dol/fte
jobd/dol/fte: ${JOBD_PREREQS}
	${NICDIR}/run.py --topo fte --feature fte --lite

.PHONY: jobd/dol/fte2
jobd/dol/fte2: ${JOBD_PREREQS}
	${NICDIR}/run.py --topo fte --feature fte2 --lite

.PHONY: jobd/dol/ftevxlan
jobd/dol/ftevxlan: ${JOBD_PREREQS}
	${NICDIR}/run.py --topo fte --feature ftevxlan --lite

.PHONY: jobd/dol/alg
jobd/dol/alg: ${JOBD_PREREQS}
	${NICDIR}/run.py --topo alg --feature alg

.PHONY: jobd/dol/norm
jobd/dol/norm: ${JOBD_PREREQS}
	${NICDIR}/run.py --topo norm --feature norm

.PHONY: jobd/dol/eth
jobd/dol/eth: ${JOBD_PREREQS}
	${NICDIR}/run.py --topo eth --feature eth

.PHONY: jobd/dol/acl
jobd/dol/acl: ${JOBD_PREREQS}
	${NICDIR}/run.py --topo acl --feature acl

.PHONY: jobd/dol/proxy
jobd/dol/proxy: ${JOBD_PREREQS}
	${NICDIR}/run.py --topo proxy --feature proxy,proxy_fte,proxy_asym1,proxy_asym2

.PHONY: jobd/dol/ipsec
jobd/dol/ipsec: ${JOBD_PREREQS}
	${NICDIR}/run.py --topo proxy --feature ipsec --no_error_check

.PHONY: jobd/dol/networking
jobd/dol/networking: ${JOBD_PREREQS}
	${NICDIR}/run.py --topo up2up --feature networking

.PHONY: jobd/dol/vxlan
jobd/dol/vxlan: ${JOBD_PREREQS}
	${NICDIR}/run.py --topo vxlan --feature vxlan

.PHONY: jobd/dol/ipsg
jobd/dol/ipsg: ${JOBD_PREREQS}
	${NICDIR}/run.py --topo eth --feature ipsg

.PHONY: jobd/dol/firewall
jobd/dol/firewall: ${JOBD_PREREQS}
	${NICDIR}/run.py --topo firewall --feature firewall

.PHONY: jobd/dol/pin
jobd/dol/pin: ${JOBD_PREREQS}
	${NICDIR}/run.py --topo fte --feature fte,fte2,ftevxlan,hpvxlan,hostpin --hostpin --lite

.PHONY: jobd/dol/multicast
jobd/dol/multicast: ${JOBD_PREREQS}
	${NICDIR}/run.py --topo multicast --feature multicast

.PHONY: jobd/dol/pinl2mc
jobd/dol/pinl2mc: ${JOBD_PREREQS}
	${NICDIR}/run.py --topo multicast --feature pinl2mc --hostpin

.PHONY: jobd/dol/l4lb
jobd/dol/l4lb: ${JOBD_PREREQS}
	${NICDIR}/run.py --topo l4lb --feature l4lb

.PHONY: jobd/dol/dos
jobd/dol/dos: ${JOBD_PREREQS}
	${NICDIR}/run.py --topo dos --feature dos

.PHONY: jobd/dol/recirc
jobd/dol/recirc: ${JOBD_PREREQS}
	${NICDIR}/run.py --topo recirc --feature recirc

.PHONY: jobd/dol/classic
jobd/dol/classic: ${JOBD_PREREQS}
	${NICDIR}/run.py --topo classic --feature classic --classic

.PHONY: jobd/dol/classicl2mc
jobd/dol/classicl2mc: ${JOBD_PREREQS}
	${NICDIR}/run.py --topo classic --feature classicl2mc --classic

.PHONY: jobd/dol/parser
jobd/dol/parser: ${JOBD_PREREQS}
	${NICDIR}/run.py --topo parser --feature parser

.PHONY: jobd/dol/telemetry
jobd/dol/telemetry: ${JOBD_PREREQS}
	${NICDIR}/run.py --topo telemetry --feature telemetry

.PHONY: jobd/dol/p4pt
jobd/dol/p4pt: ${JOBD_PREREQS}
	${NICDIR}/run.py --topo p4pt --feature p4pt

.PHONY: jobd/dol/app_redir
jobd/dol/app_redir: ${JOBD_PREREQS}
	${NICDIR}/run.py --topo app_redir --feature app_redir

.PHONY: jobd/dol/basetopo
jobd/dol/basetopo: ${JOBD_PREREQS}
	${NICDIR}/run.py --topo base --feature eth_base,nw_base,rdma_base,proxy_base --lite

.PHONY: jobd/dol/swphv
jobd/dol/swphv: ${JOBD_PREREQS}
	${NICDIR}/run.py --topo firewall --feature swphv

.PHONY: jobd/dol/gft
jobd/dol/gft: ${JOBD_PREREQS}
	${NICDIR}/run.py --topo gft --feature gft --gft

.PHONY: jobd/dol/gft/rdma
jobd/dol/gft/rdma: ${JOBD_PREREQS}
	${NICDIR}/run.py --topo rdma_gft --feature rdma_perf --gft

.PHONY: jobd/dol/gft/rdma_l2l
jobd/dol/gft/rdma_l2l: ${JOBD_PREREQS}
	${NICDIR}/run.py --topo rdma_gft --feature rdma_send_only_inline_l2l,rdma_send_l2l --l2l --gft

#.PHONY: jobd/dol/agent/up2up
#jobd/dol/agent/up2up: ${JOBD_PREREQS}
#	${NICDIR}/run.py --topo agentup2up --feature agentup2up

.PHONY: jobd/mbt/base
jobd/mbt/base: ${JOBD_PREREQS}
	DISABLE_AGING=1 ${NICDIR}/run.py --mbt --mbtrandomseed 6003702

.PHONY: jobd/mbt/networking
jobd/mbt/networking: ${JOBD_PREREQS}
	${NICDIR}/run.py --topo=up2up --feature=networking --mbt

.PHONY: jobd/mbt/firewall
jobd/mbt/firewall: ${JOBD_PREREQS}
	${NICDIR}/run.py --topo firewall --feature firewall --mbt

.PHONY: jobd/mbt/alg
jobd/mbt/alg: ${JOBD_PREREQS}
	${NICDIR}/run.py --topo alg --feature alg --mbt

.PHONY: jobd/gft/gtest
jobd/gft/gtest: ${JOBD_PREREQS}
	${NICDIR}/run.py --gft_gtest

.PHONY: jobd/apollo/gtest
jobd/apollo/gtest: ${JOBD_PREREQS}
	${NICDIR}/run.py --apollo_gtest

.PHONY: jobd/storage
jobd/storage: ${JOBD_PREREQS}
	${NICDIR}/run.py --storage

.PHONY: jobd/storage/perf
jobd/storage/perf: ${JOBD_PREREQS}
	${NICDIR}/run.py --storage --storage_test perf

.PHONY: jobd/storage/nvme
jobd/storage/nvme: ${JOBD_PREREQS}
	${NICDIR}/run.py --storage --storage_test nvme_dp

.PHONY: jobd/storage/nvme_perf
jobd/storage/nvme_perf: ${JOBD_PREREQS}
	${NICDIR}/run.py --storage --storage_test nvme_dp_scale

.PHONY: jobd/storage/nicmgr
jobd/storage/nicmgr: ${JOBD_PREREQS}
	${MAKE} -j1 -C ${TOPDIR}/platform/
	${NICDIR}/run.py --storage --storage_test nicmgr

.PHONY: jobd/storage/combined
jobd/storage/combined: ${JOBD_PREREQS}
	GRPC_TRACE=tcp GRPC_VERBOSITY=debug ${NICDIR}/run.py --storage --storage_test unit --feature rdma --topo rdma --combined --test RDMA_TX_SEND_ONLY --testcase 1 

.PHONY: jobd/configtest
jobd/configtest: ${JOBD_PREREQS}
	${NICDIR}/run.py --configtest

.PHONY: jobd/dol/e2e/tlsproxy
jobd/dol/e2e/tlsproxy: ${JOBD_PREREQS}
	${NICDIR}/run.py --topo proxy --feature proxy --config-only --e2e-tls-dol

.PHONY: jobd/dol/e2e/v6tlsproxy
jobd/dol/e2e/v6tlsproxy: ${JOBD_PREREQS}
	${NICDIR}/run.py --topo proxy --feature proxy --config-only --v6-e2e-tls-dol

.PHONY: jobd/dol/e2e/eplearn
jobd/dol/e2e/eplearn: ${JOBD_PREREQS}
	${NICDIR}/run.py --topo e2e_eplearn --feature e2e_learn --e2e-mode dol-auto --cfgjson conf/dol.conf

.PHONY: jobd/dol/e2e/alg
jobd/dol/e2e/alg: ${JOBD_PREREQS}
	${NICDIR}/run.py --topo alg --feature e2e --sub alg --e2e-mode dol-auto --cfgjson conf/dol.conf

.PHONY: jobd/dol/e2e/l7
jobd/dol/e2e/l7: ${JOBD_PREREQS}
	${NICDIR}/run.py --topo proxy --feature proxy --config-only --e2e-l7-dol

.PHONY: jobd/e2e/naples-sim
jobd/e2e/naples-sim: package
	${NICDIR}/tools/release.sh
	${NICDIR}/tools/validate-naples-docker.sh

.PHONY: jobd/make/sdk
jobd/make/sdk:
	${MAKE} -C ${SDKDIR}
	${MAKE} -C ${SDKDIR} ARCH=aarch64

.PHONY: jobd/make/nic
jobd/make/nic:
	${MAKE} PIPELINE=iris
	${MAKE} PIPELINE=iris ARCH=aarch64
	${MAKE} PIPELINE=gft
	${MAKE} PIPELINE=gft ARCH=aarch64
	${MAKE} PIPELINE=apollo
	${MAKE} PIPELINE=apollo ARCH=aarch64

.PHONY: jobd/agent
jobd/agent: package
	${MAKE} -C ${GOPATH}/src/github.com/pensando/sw checks
	${MAKE} release
	go install github.com/pensando/sw/nic/agent/cmd/netagent
	bash agent/netagent/scripts/sanity.sh
	bash agent/netagent/scripts/sanity.sh single-threaded
	bash agent/netagent/scripts/sanity.sh stand-alone
	${NICDIR}/e2etests/go/agent/scripts/golden-sanity.sh

.PHONY: jobd/platform/drivers
jobd/platform/drivers:
	${MAKE} -j1 -C ${TOPDIR}/platform/drivers/linux build

.PHONY: jobd/halctl
jobd/halctl: ${JOBD_PREREQS}
	cd ${GOPATH}/src/github.com/pensando/sw/nic/agent/cmd/halctl && go test

.PHONY: jobd/gtests
jobd/gtests: ${JOBD_PREREQS}
	${NICDIR}/tools/run_gtests.sh

.PHONY: jobd/filter/gtest
jobd/filter/gtest: ${JOBD_PREREQS}
	./run.py --filter_gtest --classic
	./run.py --filter_gtest
