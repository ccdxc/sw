default: 
	cd ../../../ && make ws-tools && make pull-assets 
	cd ../../../ && make naples-firmware
	cd ../../../ && make venice-image
	cd ../../ && make
	../../bin/iota_cmd run --suite firewall --topo default 
	../../bin/iota_cmd run --suite techsupport --topo default --skip-setup
	../../bin/iota_cmd run --suite mirror --topo default --skip-setup

venice-e2e:
	cd ../../../ && make ws-tools && make pull-assets && tar -zxf /sw/naples_fw_all.tgz -C .
	ln -s /sw/bin/venice.tgz /go/src/github.com/pensando/sw/bin/venice.tgz
	cd ../../ && make
	../../bin/iota_cmd run --suite firewall --topo default 
	../../bin/iota_cmd run --suite monitor --topo default --skip-setup
	../../bin/iota_cmd run --suite techsupport --topo default --skip-setup
	../../bin/iota_cmd run --suite mirror --topo default --skip-setup
	#../../bin/iota_cmd run --suite rollout --topo default --skip-setup
	#../../bin/iota_cmd run --suite cluster --topo default --skip-setup
upgrade:
	mkdir -p ${GOPATH}/src/github.com/pensando/sw/upgrade-bundle
	../../bin/iota_cmd run --suite rollout --topo default --skip-setup

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
	RELEASE_A=1 NO_AUTO_DISCOVERY=1 ../../bin/iota_cmd run --suite rollout2rela --topo default 

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
	USE_MGMT=1 ../../bin/iota_cmd run --suite rolloutnightly --topo default 

vcenter-e2e:
	cd ../../../ && make ws-tools && make pull-assets && tar -zxf /sw/naples_fw_all.tgz -C .
	ln -s /sw/bin/venice.tgz /go/src/github.com/pensando/sw/bin/venice.tgz
	cd ../../ && make
	${GOPATH}/src/github.com/pensando/sw/iota/scripts/modify_warmd.sh
	mkdir -p /go/src/github.com/pensando/sw/bin/upgrade-bundle
	asset-pull bundle_image 1.2.0-140 /go/src/github.com/pensando/sw/bin/upgrade-bundle/bundle.tar
	../../bin/iota_cmd run --suite vchub --topo vcenter --testbed ${GOPATH}/src/github.com/pensando/sw/iota/warmd_vcenter.json

venice-e2e-basenet:
	cd ../../../ && make ws-tools && make pull-assets && tar -zxf /sw/naples_fw_all.tgz -C .
	ln -s /sw/bin/venice.tgz /go/src/github.com/pensando/sw/bin/venice.tgz
	cd ../../ && make
	mkdir -p /go/src/github.com/pensando/sw/bin/upgrade-bundle
	asset-pull bundle_image 1.2.0-140 /go/src/github.com/pensando/sw/bin/upgrade-bundle/bundle.tar
	#First run mirror suite in basenet
	../../bin/iota_cmd run --suite basenet --topo basenet
	../../bin/iota_cmd run --suite mirror --topo basenet --skip-setup --focus "Interface mirror tests"

venice-e2e-cloud:
	cd ../../../ && make ws-tools && make pull-assets && tar -zxf /sw/naples_fw_all_apulu.tgz -C .
	ln -s /sw/bin/venice.tgz /go/src/github.com/pensando/sw/bin/venice.tgz
	cd ../../ && make
	NO_INTERNAL_MGMT=1 ../../bin/iota_cmd run --suite equinix --topo cloud

venice-sim-cloud:
	cd ../../../ && make ws-tools && make pull-assets && tar -zxf /sw/build_apulu_sim.tar.gz -C .
	ln -s /sw/bin/venice.apulu.tgz /go/src/github.com/pensando/sw/bin/venice.apulu.tgz
	cd ../../ && make
	../../bin/iota_cmd run --suite equinix --topo cloud_sim
	../../bin/iota_cmd run --suite techsupport --topo cloud_sim --skip-setup 

all: venice-e2e upgrade
