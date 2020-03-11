default: 
	cd ../../../ && make ws-tools && make pull-assets 
	cd ../../../ && make naples-firmware
	cd ../../../ && make venice-image
	cd ../../ && make
	./start_iota.sh
	VENICE_DEV=1 go test github.com/pensando/sw/iota/test/venice/testsuites/firewall -timeout 3h -v -ginkgo.v  -topo 3Venice_2Naples
	VENICE_DEV=1 SKIP_SETUP=1 go test github.com/pensando/sw/iota/test/venice/testsuites/monitor -timeout 60m -v -ginkgo.v -topo 3Venice_2Naples
	VENICE_DEV=1 SKIP_SETUP=1 go test github.com/pensando/sw/iota/test/venice/testsuites/techsupport -timeout 120m -v -ginkgo.v -topo 3Venice_2Naples
	# TODO Renebale this test suite once jobd inband changes go in
	#VENICE_DEV=1 SKIP_SETUP=1 go test github.com/pensando/sw/iota/test/venice/testsuites/mirror -timeout 120m -v -ginkgo.v -topo 3Venice_2Naples
	# VENICE_DEV=1 go test github.com/pensando/sw/iota/test/venice/testsuites/cluster -timeout 60m -v -ginkgo.v

venice-e2e:
	cd ../../../ && make ws-tools && make pull-assets && tar -zxf /sw/naples_fw_all.tgz -C .
	ln -s /sw/bin/venice.tgz /go/src/github.com/pensando/sw/bin/venice.tgz
	cd ../../ && make
	./start_iota.sh
	mkdir -p /go/src/github.com/pensando/sw/bin/upgrade-bundle
	asset-pull bundle_image 1.2.0-140 /go/src/github.com/pensando/sw/bin/upgrade-bundle/bundle.tar
	VENICE_DEV=1 go test github.com/pensando/sw/iota/test/venice/testsuites/firewall -timeout 3h -v -ginkgo.v  -topo 3Venice_2Naples
	VENICE_DEV=1 SKIP_SETUP=1 go test github.com/pensando/sw/iota/test/venice/testsuites/monitor -timeout 60m -v -ginkgo.v -topo 3Venice_2Naples
	VENICE_DEV=1 SKIP_SETUP=1 go test github.com/pensando/sw/iota/test/venice/testsuites/techsupport -timeout 120m -v -ginkgo.v -topo 3Venice_2Naples

	# TODO Renebale this test suite once jobd inband changes go in
	#VENICE_DEV=1 SKIP_SETUP=1 go test github.com/pensando/sw/iota/test/venice/testsuites/mirror -timeout 120m -v -ginkgo.v -topo 3Venice_2Naples
	# VENICE_DEV=1 go test github.com/pensando/sw/iota/test/venice/testsuites/rollout -timeout 240m -v -ginkgo.v -topo 3Venice_2Naples
	# VENICE_DEV=1 go test github.com/pensando/sw/iota/test/venice/testsuites/cluster -timeout 60m -v -ginkgo.v
upgrade:
	mkdir -p /go/src/github.com/pensando/sw/bin/venice-install
	mkdir -p /go/src/github.com/pensando/sw/upgrade-bundle/bin
	mkdir -p /go/src/github.com/pensando/sw/upgrade-bundle/nic
	mkdir -p /go/src/github.com/pensando/sw/upgrade-bundle/bin/venice-install
	cp /sw/bin/venice.upg.tgz /go/src/github.com/pensando/sw/bin/venice.upg.tgz
	cp /sw/bin/venice-install/venice_appl_os.tgz /go/src/github.com/pensando/sw/bin/venice-install/venice_appl_os.tgz
	cd ../../../ && make bundle-upgrade-image
	SKIP_SETUP=1 VENICE_DEV=1 go test github.com/pensando/sw/iota/test/venice/testsuites/rollout -timeout 240m -v -ginkgo.v -topo 3Venice_2Naples

vcenter-e2e:
	cd ../../../ && make ws-tools && make pull-assets && tar -zxf /sw/naples_fw_all.tgz -C .
	ln -s /sw/bin/venice.tgz /go/src/github.com/pensando/sw/bin/venice.tgz
	cd ../../ && make
	./start_iota.sh
	mkdir -p /go/src/github.com/pensando/sw/bin/upgrade-bundle
	asset-pull bundle_image 1.2.0-140 /go/src/github.com/pensando/sw/bin/upgrade-bundle/bundle.tar
	VENICE_DEV=1 go test github.com/pensando/sw/iota/test/venice/testsuites/vchub -timeout 3h -v -ginkgo.v  -topo 3Venice_2Naples_Vcenter

#venice-e2e-basenet: TODO Reenebale this tests once jobd inband changes go in.
#	cd ../../../ && make ws-tools && make pull-assets && tar -zxf /sw/naples_fw_all.tgz -C .
#	ln -s /sw/bin/venice.tgz /go/src/github.com/pensando/sw/bin/venice.tgz
#	cd ../../ && make
#	./start_iota.sh
#	mkdir -p /go/src/github.com/pensando/sw/bin/upgrade-bundle
#	asset-pull bundle_image 1.2.0-140 /go/src/github.com/pensando/sw/bin/upgrade-bundle/bundle.tar
#	VENICE_DEV=1 go test github.com/pensando/sw/iota/test/venice/testsuites/mirror -timeout 3h -v -ginkgo.v  -topo 3Venice_2Naples_Basenet -ginkgo.focus "Lif mirror tests"

venice-e2e-cloud:
	cd ../../../ && make ws-tools && make pull-assets && tar -zxf /sw/naples_fw_all_apulu.tgz -C .
	ln -s /sw/bin/venice.tgz /go/src/github.com/pensando/sw/bin/venice.tgz
	cd ../../ && make
	./start_iota.sh
	VENICE_DEV=1 USE_MGMT=1 go test github.com/pensando/sw/iota/test/venice/testsuites/equinix -timeout 3h -v -ginkgo.v  -topo 3Venice_1Naples_Cloud

all: venice-e2e upgrade
