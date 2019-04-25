default: 
	cd ../../../../ && make ws-tools && make pull-assets 
	cd ../../../../ && make e2e-naples && make venice-image
	make test

test:
	cd ../../../ && make
	../start_iota.sh
	VENICE_DEV=1 go test github.com/pensando/sw/iota/test/venice/testsuites/firewall -timeout 120m -v -ginkgo.v  -topo 3Venice_3NaplesSim
	VENICE_DEV=1 SKIP_SETUP=1 go test github.com/pensando/sw/iota/test/venice/testsuites/fwlog -timeout 120m -v -ginkgo.v  -topo 3Venice_3NaplesSim
	VENICE_DEV=1 SKIP_SETUP=1 go test github.com/pensando/sw/iota/test/venice/testsuites/monitor -timeout 60m -v -ginkgo.v -topo 3Venice_3NaplesSim
