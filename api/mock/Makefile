.PHONY : default clean

default: mock_telemetry.go mock_apiclient.go

mock_telemetry.go: ${GOPATH}/src/github.com/pensando/sw/api/generated/telemetry/telemetry_crudinterface.go Makefile
	mockgen -destination=mock_telemetry.go -package=mock --source ../generated/telemetry/telemetry_crudinterface.go FlowExportPolicyInterface,FwlogPolicyInterface,StatsPolicyInterface
	# telemetry package
	sed -i.bak_ -e 's/\*FlowExportPolicy/\*telemetry.FlowExportPolicy/g' $@
	sed -i.bak_ -e 's/\*FwlogPolicy/\*telemetry.FwlogPolicy/g' $@
	sed -i.bak_ -e 's/\*StatsPolicy/\*telemetry.StatsPolicy/g' $@
	sed -i.bak_ -e 's/[[:space:](]StatsPolicyV1StatsPolicyInterface[[:space:])]/ (telemetry.StatsPolicyV1StatsPolicyInterface)/g' $@
	sed -i.bak_ -e 's/[[:space:](]FwlogPolicyV1FwlogPolicyInterface[[:space:])]/ (telemetry.FwlogPolicyV1FwlogPolicyInterface)/g' $@
	sed -i.bak_ -e 's/[[:space:](]FlowExportPolicyV1FlowExportPolicyInterface[[:space:])]/ (telemetry.FlowExportPolicyV1FlowExportPolicyInterface)/g' $@
	goimports -local "github.com/pensando/sw" -w $@
	# goimports will bring in github.com/pensando/sw/api/generated/telemetry which can then be reimported as telemetry package
	sed -i.bak_ -e "s/.*\"github.com\/pensando\/sw\/api\/generated\/telemetry\".*/telemetry \"github.com\/pensando\/sw\/api\/generated\/telemetry\"/g" $@
	rm -f *.bak_
	# Final goimports to format
	goimports -local "github.com/pensando/sw" -w $@

mock_apiclient.go: ${GOPATH}/src/github.com/pensando/sw/api/generated/apiclient/client.go Makefile
	mockgen -destination=$@ -package=mock --source ../generated/apiclient/client.go
	goimports -local "github.com/pensando/sw" -w $@

clean:
	rm -f mock_telemetry.go mock_apiclient.go
