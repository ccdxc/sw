default: 
	./start_iota.sh
	VENICE_DEV=1 go test github.com/pensando/sw/iota/test/venice/testsuites/firewall -timeout 3h -v -ginkgo.v
	VENICE_DEV=1 go test github.com/pensando/sw/iota/test/venice/testsuites/monitor -timeout 60m -v -ginkgo.v
	# VENICE_DEV=1 go test github.com/pensando/sw/iota/test/venice/testsuites/cluster -timeout 60m -v -ginkgo.v

mock-iota-test:
	# JOB_ID=1 VENICE_DEV=1 MOCK_IOTA=1 go test github.com/pensando/sw/iota/test/venice/testsuites/firewall/ -count 1 -p 1 -testbed $$GOPATH/src/github.com/pensando/sw/iota/test/venice/example/warmd.json -topo 3Venice_2Naples -v -ginkgo.v -timeout 60m
	echo "Temporarily disabled till we have a mock'able NMD"
