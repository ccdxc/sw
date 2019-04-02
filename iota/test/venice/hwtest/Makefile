default: 
	cd ../../../../ && make ws-tools && make pull-assets 
	cd ../../../../ && make naples-firmware
	cd ../../../../ && make venice-image
	cd ../../../ && make
	../start_iota.sh
	VENICE_DEV=1 go test github.com/pensando/sw/iota/test/venice/testsuites/firewall -timeout 120m -v -ginkgo.v  -topo 3Venice_2Naples
	VENICE_DEV=1 go test github.com/pensando/sw/iota/test/venice/testsuites/fwlog -timeout 120m -v -ginkgo.v  -topo 3Venice_2Naples
	# VENICE_DEV=1 go test github.com/pensando/sw/iota/test/venice/testsuites/smartnic -timeout 60m -v -ginkgo.v
	# VENICE_DEV=1 go test github.com/pensando/sw/iota/test/venice/testsuites/cluster -timeout 60m -v -ginkgo.v
	# VENICE_DEV=1 go test github.com/pensando/sw/iota/test/venice/testsuites/monitor -timeout 60m -v -ginkgo.v
