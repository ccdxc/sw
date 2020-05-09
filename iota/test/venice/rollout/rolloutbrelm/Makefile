default: 
	cd ../../../ && make ws-tools && make pull-assets 
	cd ../../../ && make naples-firmware
	cd ../../../ && make venice-image
	cd ../../ && make
	../../bin/iota_cmd run --suite firewall --topo default 
	../../bin/iota_cmd run --suite techsupport --topo default --skip-setup
	../../bin/iota_cmd run --suite mirror --topo default --skip-setup

relbminus2relb:
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
	../../bin/iota_cmd run --suite rollout2rela --topo default 
