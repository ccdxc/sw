.PHONY : default clean

default: mock_telemetry.go mock_apiclient.go

mock_telemetry.go: ${GOPATH}/src/github.com/pensando/sw/api/generated/telemetry/telemetry_crudinterface.go Makefile
	mockgen -destination=mock_telemetry.go -package=mock --source ${GOPATH}/src/github.com/pensando/sw/api/generated/telemetry/telemetry_crudinterface.go FlowExportPolicyInterface,FwlogPolicyInterface,StatsPolicyInterface
	# telemetry package
	sed -i 's/\*FlowExportPolicy/\*telemetry.FlowExportPolicy/g' $@
	sed -i 's/\*FwlogPolicy/\*telemetry.FwlogPolicy/g' $@
	sed -i 's/\*StatsPolicy/\*telemetry.StatsPolicy/g' $@
	sed -i 's/\bStatsPolicyV1StatsPolicyInterface/telemetry.StatsPolicyV1StatsPolicyInterface/g' $@
	sed -i 's/\bFwlogPolicyV1FwlogPolicyInterface/telemetry.FwlogPolicyV1FwlogPolicyInterface/g' $@
	sed -i 's/\bFlowExportPolicyV1FlowExportPolicyInterface/telemetry.FlowExportPolicyV1FlowExportPolicyInterface/g' $@
	sed -i '/import (/a telemetry "github.com/pensando/sw/api/generated/telemetry"' $@
	goimports -local "github.com/pensando/sw" -w $@
	
mock_apiclient.go: ${GOPATH}/src/github.com/pensando/sw/api/generated/apiclient/client.go Makefile
	mockgen -destination=$@ -package=mock --source ${GOPATH}/src/github.com/pensando/sw/api/generated/apiclient/client.go
	goimports -local "github.com/pensando/sw" -w $@

clean:
	rm -f mock_telemetry.go mock_apiclient.go

