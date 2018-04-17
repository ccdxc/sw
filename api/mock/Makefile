.PHONY : default clean

TARGET := mock_telemetry.go mock_apiclient.go mock_network.go

default: ${TARGET}

mock_telemetry.go: ../generated/telemetry/telemetry_crudinterface.go Makefile
	mockgen -destination=$@ -package=mock --source $<
	# telemetry package
	sed -i.bak_ -e 's/\*FlowExportPolicy/\*telemetry.FlowExportPolicy/g' $@
	sed -i.bak_ -e 's/\*FwlogPolicy/\*telemetry.FwlogPolicy/g' $@
	sed -i.bak_ -e 's/\*StatsPolicy/\*telemetry.StatsPolicy/g' $@
	sed -i.bak_ -e 's/[[:space:](]StatsPolicyV1StatsPolicyInterface[[:space:])]/ (telemetry.StatsPolicyV1StatsPolicyInterface)/g' $@
	sed -i.bak_ -e 's/[[:space:](]FwlogPolicyV1FwlogPolicyInterface[[:space:])]/ (telemetry.FwlogPolicyV1FwlogPolicyInterface)/g' $@
	sed -i.bak_ -e 's/[[:space:](]FlowExportPolicyV1FlowExportPolicyInterface[[:space:])]/ (telemetry.FlowExportPolicyV1FlowExportPolicyInterface)/g' $@
	goimports -local "github.com/pensando/sw" -w $@
	# goimports will bring in github.com/pensando/sw/api/generated/telemetry which can then be reimported as telemetry package
	sed -i.bak_ -e "s/\"github.com\/pensando\/sw\/api\/generated\/telemetry\"/telemetry \"github.com\/pensando\/sw\/api\/generated\/telemetry\"/g" $@
	rm -f *.bak_

mock_apiclient.go: ${GOPATH}/src/github.com/pensando/sw/api/generated/apiclient/client.go Makefile
	mockgen -destination=$@ -package=mock --source ../generated/apiclient/client.go
	goimports -local "github.com/pensando/sw" -w $@

mock_network.go: ../generated/network/network_crudinterface.go Makefile
	mockgen -destination=$@ -package=mock --source $< tenant
	sed -i.bak_ -e 's/\*Endpoint/\*network.Endpoint/g' $@
	sed -i.bak_ -e 's/\*LbPolicy/\*network.LbPolicy/g' $@
	sed -i.bak_ -e 's/\*Network/\*network.Network/g' $@
	sed -i.bak_ -e 's/\*Service/\*network.Service/g' $@
	sed -i.bak_ -e 's/\*SecurityGroup/\*network.SecurityGroup/g' $@
	sed -i.bak_ -e 's/\*Sgpolicy/\*network.Sgpolicy/g' $@
	sed -i.bak_ -e 's/\*Tenant/\*network.Tenant/g' $@
	sed -i.bak_ -e 's/[[:space:](]EndpointV1EndpointInterface[[:space:])]/ (network.EndpointV1EndpointInterface)/g' $@
	sed -i.bak_ -e 's/[[:space:](]LbPolicyV1LbPolicyInterface[[:space:])]/ (network.LbPolicyV1LbPolicyInterface)/g' $@
	sed -i.bak_ -e 's/[[:space:](]SecurityGroupV1SecurityGroupInterface[[:space:])]/ (network.SecurityGroupV1SecurityGroupInterface)/g' $@
	sed -i.bak_ -e 's/[[:space:](]ServiceV1ServiceInterface[[:space:])]/ (network.ServiceV1ServiceInterface)/g' $@
	sed -i.bak_ -e 's/[[:space:](]SgpolicyV1SgpolicyInterface[[:space:])]/ (network.SgpolicyV1SgpolicyInterface)/g' $@
	sed -i.bak_ -e 's/[[:space:](]TenantV1TenantInterface[[:space:])]/ (network.TenantV1TenantInterface)/g' $@
	sed -i.bak_ -e 's/[[:space:](]NetworkV1NetworkInterface[[:space:])]/ (network.NetworkV1NetworkInterface)/g' $@
	goimports -local "github.com/pensando/sw" -w $@
	sed -i.bak_ -e "s/\"github.com\/pensando\/sw\/api\/generated\/network\"/network \"github.com\/pensando\/sw\/api\/generated\/network\"/g" $@
	rm -f *.bak_

clean:
	rm -f ${TARGET}
