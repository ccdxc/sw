default: 
	cd ../../../../ && make ws-tools && make pull-assets 
	cd ../../../../ && make naples-firmware
	cd ../../../../ && make venice-image
	make test

test:
	cd ../../../ && make
	../start_iota.sh
	JOB_ID=1 VENICE_DEV=1 go test github.com/pensando/sw/iota/test/venice/testsuites/firewall -timeout 3h -v -ginkgo.v  -topo 1Venice_1Naples
	JOB_ID=1 VENICE_DEV=1 SKIP_SETUP=1 go test github.com/pensando/sw/iota/test/venice/testsuites/fwlog -timeout 120m -v -ginkgo.v  -topo 1Venice_1Naples
	JOB_ID=1 VENICE_DEV=1 SKIP_SETUP=1 go test github.com/pensando/sw/iota/test/venice/testsuites/monitor -timeout 60m -v -ginkgo.v -topo 1Venice_1Naples
	JOB_ID=1 VENICE_DEV=1 SKIP_SETUP=1 go test github.com/pensando/sw/iota/test/venice/testsuites/rollout -timeout 60m -v -ginkgo.v -topo 1Venice_1Naples
	JOB_ID=1 VENICE_DEV=1 SKIP_SETUP=1 go test github.com/pensando/sw/iota/test/venice/testsuites/techsupport -timeout 60m -v -ginkgo.v -topo 1Venice_1Naples

fw-test:
	JOB_ID=1 VENICE_DEV=1 go test github.com/pensando/sw/iota/test/venice/testsuites/firewall -timeout 120m -v -ginkgo.v  -topo 1Venice_1Naples
