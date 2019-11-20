default: 
	cd ../../../../ && make ws-tools && make pull-assets 
	cd ../../../../ && make naples-firmware
	cd ../../../../ && make venice-image
	cd ../../../ && make
	../start_iota.sh
	VENICE_DEV=1 REGRESSION=1 go test github.com/pensando/sw/iota/test/venice/testsuites/firewall -timeout 6h -v -ginkgo.v  -topo 3Venice_2Naples
	VENICE_DEV=1 REGRESSION=1 SKIP_SETUP=1 go test github.com/pensando/sw/iota/test/venice/testsuites/monitor -timeout 2h -v -ginkgo.v -topo 3Venice_2Naples
	VENICE_DEV=1 REGRESSION=1 SKIP_SETUP=1 go test github.com/pensando/sw/iota/test/venice/testsuites/mirror -timeout 120m -v -ginkgo.v -topo 3Venice_2Naples
	VENICE_DEV=1 REGRESSION=1 SKIP_SETUP=1 go test github.com/pensando/sw/iota/test/venice/testsuites/techsupport -timeout 120m -v -ginkgo.v -topo 3Venice_2Naples
	# VENICE_DEV=1 REGRESSION=1 go test github.com/pensando/sw/iota/test/venice/testsuites/cluster -timeout 60m -v -ginkgo.v -topo 3Venice_2Naples

jobd:
	cd ../../../../ && make ws-tools && make pull-assets && tar -zxf /sw/naples_fw_all.tgz -C .
	ln -s /sw/bin/venice.tgz /go/src/github.com/pensando/sw/bin/venice.tgz
	cd ../../../ && make
	../start_iota.sh
	VENICE_DEV=1 REGRESSION=1 go test github.com/pensando/sw/iota/test/venice/testsuites/firewall -timeout 6h -v -ginkgo.v  -topo 3Venice_2Naples
	VENICE_DEV=1 REGRESSION=1 SKIP_SETUP=1 go test github.com/pensando/sw/iota/test/venice/testsuites/monitor -timeout 2h -v -ginkgo.v -topo 3Venice_2Naples
	VENICE_DEV=1 REGRESSION=1 SKIP_SETUP=1 go test github.com/pensando/sw/iota/test/venice/testsuites/mirror -timeout 120m -v -ginkgo.v -topo 3Venice_2Naples
	VENICE_DEV=1 REGRESSION=1 SKIP_SETUP=1 go test github.com/pensando/sw/iota/test/venice/testsuites/techsupport -timeout 120m -v -ginkgo.v -topo 3Venice_2Naples
	# VENICE_DEV=1 REGRESSION=1 go test github.com/pensando/sw/iota/test/venice/testsuites/rollout -timeout 240m -v -ginkgo.v -topo 3Venice_2Naples
	# VENICE_DEV=1 REGRESSION=1 go test github.com/pensando/sw/iota/test/venice/testsuites/cluster -timeout 60m -v -ginkgo.v -topo 3Venice_2Naples
