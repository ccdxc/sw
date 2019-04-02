default: 
	./start_iota.sh
	VENICE_DEV=1 go test github.com/pensando/sw/iota/test/venice/testsuites/firewall -timeout 60m -v -ginkgo.v
	VENICE_DEV=1 go test github.com/pensando/sw/iota/test/venice/testsuites/monitor -timeout 60m -v -ginkgo.v
	# VENICE_DEV=1 go test github.com/pensando/sw/iota/test/venice/testsuites/smartnic -timeout 60m -v -ginkgo.v
	# VENICE_DEV=1 go test github.com/pensando/sw/iota/test/venice/testsuites/cluster -timeout 60m -v -ginkgo.v
