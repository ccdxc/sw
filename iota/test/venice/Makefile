default: 
	cd ../../../ && make ws-tools && make pull-assets 
	cd ../../../ && make naples-firmware
	cd ../../../ && make venice-image
	cd ../../ && make
	./start_iota.sh
	VENICE_DEV=1 go test github.com/pensando/sw/iota/test/venice/testsuites/firewall -timeout 3h -v -ginkgo.v  -topo 3Venice_2Naples
	VENICE_DEV=1 SKIP_SETUP=1 go test github.com/pensando/sw/iota/test/venice/testsuites/techsupport -timeout 120m -v -ginkgo.v -topo 3Venice_2Naples
	VENICE_DEV=1 SKIP_SETUP=1 go test github.com/pensando/sw/iota/test/venice/testsuites/monitor -timeout 60m -v -ginkgo.v -topo 3Venice_2Naples
	# TODO Renebale this test suite once jobd inband changes go in
	#VENICE_DEV=1 SKIP_SETUP=1 go test github.com/pensando/sw/iota/test/venice/testsuites/mirror -timeout 120m -v -ginkgo.v -topo 3Venice_2Naples
	# VENICE_DEV=1 go test github.com/pensando/sw/iota/test/venice/testsuites/cluster -timeout 60m -v -ginkgo.v

venice-e2e:
	cd ../../../ && make ws-tools && make pull-assets && tar -zxf /sw/naples_fw_all.tgz -C .
	ln -s /sw/bin/venice.tgz /go/src/github.com/pensando/sw/bin/venice.tgz
	cd ../../ && make
	./start_iota.sh
	VENICE_DEV=1  go test github.com/pensando/sw/iota/test/venice/testsuites/firewall -timeout 3h -v -ginkgo.v  -topo 3Venice_2Naples
	VENICE_DEV=1 SKIP_SETUP=1 go test github.com/pensando/sw/iota/test/venice/testsuites/monitor -timeout 60m -v -ginkgo.v -topo 3Venice_2Naples
	VENICE_DEV=1 SKIP_SETUP=1 go test github.com/pensando/sw/iota/test/venice/testsuites/techsupport -timeout 120m -v -ginkgo.v -topo 3Venice_2Naples
	VENICE_DEV=1 SKIP_SETUP=1 go test github.com/pensando/sw/iota/test/venice/testsuites/mirror -timeout 120m -v -ginkgo.v -topo 3Venice_2Naples -ginkgo.focus "Flow Mirror tests"
	# VENICE_DEV=1 go test github.com/pensando/sw/iota/test/venice/testsuites/rollout -timeout 240m -v -ginkgo.v -topo 3Venice_2Naples
	# VENICE_DEV=1 go test github.com/pensando/sw/iota/test/venice/testsuites/cluster -timeout 60m -v -ginkgo.v
upgrade:
	mkdir -p ${GOPATH}/src/github.com/pensando/sw/upgrade-bundle
	SKIP_SETUP=1 VENICE_DEV=1 go test github.com/pensando/sw/iota/test/venice/testsuites/rollout -timeout 240m -v -ginkgo.v -topo 3Venice_2Naples

rolloutrela-nightly:
	cd ../../../ && make ws-tools && make pull-assets
	cd ../../../ && tar -zxf /sw/naples_fw_all.tgz -C .
	mkdir -p ${GOPATH}/src/github.com/pensando/sw/bin/venice-install
	mkdir -p ${GOPATH}/src/github.com/pensando/sw/upgrade-bundle/bin
	mkdir -p ${GOPATH}/src/github.com/pensando/sw/upgrade-bundle/nic
	mkdir -p ${GOPATH}/src/github.com/pensando/sw/upgrade-bundle/bin/venice-install
	cp /sw/bin/venice.upg.tgz ${GOPATH}/src/github.com/pensando/sw/bin/venice.upg.tgz
	cp /sw/bin/venice-install/venice_appl_os.tgz ${GOPATH}/src/github.com/pensando/sw/bin/venice-install/venice_appl_os.tgz
	cd ${GOPATH}/src/github.com/pensando/sw/bin/venice-install && ls -al
	cd ../../../ && make bundle-upgrade-image
	mkdir -p ${GOPATH}/src/github.com/pensando/sw/lastrel-bundle
	asset-pull bundle_image 1.3.2-E-3 ${GOPATH}/src/github.com/pensando/sw/lastrel-bundle/bundle.tar
	cd ${GOPATH}/src/github.com/pensando/sw/lastrel-bundle && tar -xvf bundle.tar
	asset-pull esx_driver 1.3.2-E-3 ${GOPATH}/src/github.com/pensando/sw/platform/esxdriver.tgz
	cd ${GOPATH}/src/github.com/pensando/sw/platform && tar -zxvf esxdriver.tgz
	cd ${GOPATH}/src/github.com/pensando/sw/lastrel-bundle && tar -xvf bundle.tar && ls -al && mv venice.tgz ${GOPATH}/src/github.com/pensando/sw/bin/ && mv naples_fw.tar ${GOPATH}/src/github.com/pensando/sw/nic/
	mkdir -p ${GOPATH}/src/github.com/pensando/sw/iota/bin/penctl
	asset-pull host.tar 1.3.2-E-3 ${GOPATH}/src/github.com/pensando/sw/iota/bin/penctl/host.tar
	cd  ${GOPATH}/src/github.com/pensando/sw/iota/bin/penctl && tar -xvf host.tar
	cd ../../ && make
	./start_iota.sh
	RELEASE_A=1 NO_AUTO_DISCOVERY=1 VENICE_DEV=1 go test github.com/pensando/sw/iota/test/venice/testsuites/rollout2rela -timeout 240m -v -ginkgo.v -topo 3Venice_2Naples

rolloutnightly:
	cd ../../../ && make ws-tools && make pull-assets
	ls -al /sw
	ls -al /sw/bin
	ls -al /sw/nic
	cd ../../../ && tar -zxf /sw/naples_fw_all_apulu.tgz -C .
	mkdir -p ${GOPATH}/src/github.com/pensando/sw/bin/venice-install
	mkdir -p ${GOPATH}/src/github.com/pensando/sw/apulu-bundle/bin
	mkdir -p ${GOPATH}/src/github.com/pensando/sw/apulu-bundle/nic
	mkdir -p ${GOPATH}/src/github.com/pensando/sw/apulu-bundle/bin/venice-install
	cp /sw/bin/venice.apulu.tgz ${GOPATH}/src/github.com/pensando/sw/bin/venice.apulu.tgz
	cp /sw/bin/venice-install/venice_appl_os.tgz ${GOPATH}/src/github.com/pensando/sw/bin/venice-install/venice_appl_os.tgz
	cd ${GOPATH}/src/github.com/pensando/sw/bin/venice-install && tar -xvzf venice_appl_os.tgz
	ls -al ${GOPATH}/src/github.com/pensando/sw/bin
	ls -al ${GOPATH}/src/github.com/pensando/sw/nic
	cd ../../../ && make bundle-apulu-image
	ls -al /sw/
	ls -al /sw/bin
	ls -al /sw/bin/venice-install
	cd ../../../ && tar -zxf /sw/naples_fw_all.tgz -C .
	mkdir -p ${GOPATH}/src/github.com/pensando/sw/bin/venice-install
	mkdir -p ${GOPATH}/src/github.com/pensando/sw/upgrade-bundle/bin
	mkdir -p ${GOPATH}/src/github.com/pensando/sw/upgrade-bundle/nic
	mkdir -p ${GOPATH}/src/github.com/pensando/sw/upgrade-bundle/bin/venice-install
	cp /sw/bin/venice.tgz ${GOPATH}/src/github.com/pensando/sw/bin/venice.tgz
	cp /sw/bin/venice.upg.tgz ${GOPATH}/src/github.com/pensando/sw/bin/venice.upg.tgz
	cp /sw/bin/venice-install/venice_appl_os.tgz ${GOPATH}/src/github.com/pensando/sw/bin/venice-install/venice_appl_os.tgz
	ls -al ${GOPATH}/src/github.com/pensando/sw/bin
	ls -al ${GOPATH}/src/github.com/pensando/sw/nic
	cd ../../../ && make bundle-upgrade-image
	cd ../../ && make
	./start_iota.sh
	USE_MGMT=1 VENICE_DEV=1 go test github.com/pensando/sw/iota/test/venice/testsuites/rolloutnightly -timeout 240m -v -ginkgo.v -topo 3Venice_2Naples

vcenter-e2e:
	cd ../../../ && make ws-tools && make pull-assets && tar -zxf /sw/naples_fw_all.tgz -C .
	ln -s /sw/bin/venice.tgz /go/src/github.com/pensando/sw/bin/venice.tgz
	cd ../../ && make
	./start_iota.sh
	mkdir -p /go/src/github.com/pensando/sw/bin/upgrade-bundle
	asset-pull bundle_image 1.2.0-140 /go/src/github.com/pensando/sw/bin/upgrade-bundle/bundle.tar
	VENICE_DEV=1 go test github.com/pensando/sw/iota/test/venice/testsuites/vchub -timeout 3h -v -ginkgo.v  -topo 3Venice_2Naples_Vcenter

venice-e2e-basenet:
	cd ../../../ && make ws-tools && make pull-assets && tar -zxf /sw/naples_fw_all.tgz -C .
	ln -s /sw/bin/venice.tgz /go/src/github.com/pensando/sw/bin/venice.tgz
	cd ../../ && make
	./start_iota.sh
	mkdir -p /go/src/github.com/pensando/sw/bin/upgrade-bundle
	asset-pull bundle_image 1.2.0-140 /go/src/github.com/pensando/sw/bin/upgrade-bundle/bundle.tar
	#First run mirror suite in basenet
	VENICE_DEV=1 go test github.com/pensando/sw/iota/test/venice/testsuites/basenet -timeout 3h -v -ginkgo.v  -topo 3Venice_2Naples_Basenet
	VENICE_DEV=1 SKIP_SETUP=1 go test github.com/pensando/sw/iota/test/venice/testsuites/mirror -timeout 3h -v -ginkgo.v  -topo 3Venice_2Naples_Basenet -ginkgo.focus "Lif mirror tests"

venice-e2e-cloud:
	cd ../../../ && make ws-tools && make pull-assets && tar -zxf /sw/naples_fw_all_apulu.tgz -C .
	ln -s /sw/bin/venice.tgz /go/src/github.com/pensando/sw/bin/venice.tgz
	cd ../../ && make
	./start_iota.sh
	VENICE_DEV=1 NO_INTERNAL_MGMT=1 go test github.com/pensando/sw/iota/test/venice/testsuites/equinix -timeout 3h -v -ginkgo.v  -topo 3Venice_1Naples_Cloud

venice-sim-cloud:
	cd ../../../ && make ws-tools && make pull-assets && tar -zxf /sw/build_apulu_sim.tar.gz -C .
	ln -s /sw/bin/venice.apulu.tgz /go/src/github.com/pensando/sw/bin/venice.apulu.tgz
	cd ../../ && make
	./start_iota.sh
	VENICE_DEV=1 go test github.com/pensando/sw/iota/test/venice/testsuites/equinix -timeout 120m -v -ginkgo.v  -topo 1Venice_2Naples_Sim_Cloud
	VENICE_DEV=1 SKIP_SETUP=1 go test github.com/pensando/sw/iota/test/venice/testsuites/techsupport -timeout 30m -v -ginkgo.v -topo 1Venice_2Naples_Sim_Cloud

all: venice-e2e upgrade
