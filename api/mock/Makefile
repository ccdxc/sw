.PHONY : default clean

TARGET := mock_telemetry.go mock_apiclient.go mock_network.go mock_cluster.go

default: ${TARGET}

mock_telemetry.go:../generated/monitoring/svc_monitoring_crudinterface.go Makefile
	mockgen -destination=$@ -package=mock --source $<
	# telemetry package
	sed -i.bak_ -e 's/\*FlowExportPolicy/\*monitoring.FlowExportPolicy/g' $@
	sed -i.bak_ -e 's/\*FwlogPolicy/\*monitoring.FwlogPolicy/g' $@
	sed -i.bak_ -e 's/\*StatsPolicy/\*monitoring.StatsPolicy/g' $@
	sed -i.bak_ -e 's/\*Event/\*monitoring.Event/g' $@
	sed -i.bak_ -e 's/\*EventPolicy/\*monitoring.EventPolicy/g' $@
	sed -i.bak_ -e 's/\*Alert/\*monitoring.Alert/g' $@
	sed -i.bak_ -e 's/\*AlertPolicy/\*monitoring.AlertPolicy/g' $@
	sed -i.bak_ -e 's/\*MirrorSession/\*monitoring.MirrorSession/g' $@
	sed -i.bak_ -e 's/[[:space:](]MonitoringV1StatsPolicyInterface[[:space:])]/ (monitoring.MonitoringV1StatsPolicyInterface)/g' $@
	sed -i.bak_ -e 's/[[:space:](]MonitoringV1FwlogPolicyInterface[[:space:])]/ (monitoring.MonitoringV1FwlogPolicyInterface)/g' $@
	sed -i.bak_ -e 's/[[:space:](]MonitoringV1FlowExportPolicyInterface[[:space:])]/ (monitoring.MonitoringV1FlowExportPolicyInterface)/g' $@
	sed -i.bak_ -e 's/[[:space:](]MonitoringV1EventInterface[[:space:])]/ (monitoring.MonitoringV1EventInterface)/g' $@
	sed -i.bak_ -e 's/[[:space:](]MonitoringV1EventPolicyInterface[[:space:])]/ (monitoring.MonitoringV1EventPolicyInterface)/g' $@
	sed -i.bak_ -e 's/[[:space:](]MonitoringV1AlertInterface[[:space:])]/ (monitoring.MonitoringV1AlertInterface)/g' $@
	sed -i.bak_ -e 's/[[:space:](]MonitoringV1AlertPolicyInterface[[:space:])]/ (monitoring.MonitoringV1AlertPolicyInterface)/g' $@
	sed -i.bak_ -e 's/[[:space:](]MonitoringV1AlertDestinationInterface[[:space:])]/ (monitoring.MonitoringV1AlertDestinationInterface)/g' $@
	sed -i.bak_ -e 's/[[:space:](]MonitoringV1MirrorSessionInterface[[:space:])]/ (monitoring.MonitoringV1MirrorSessionInterface)/g' $@
	goimports -local "github.com/pensando/sw" -w $@
	# goimports will bring in github.com/pensando/sw/api/generated/telemetry which can then be reimported as telemetry package
	sed -i.bak_ -e "s/.*\"github.com\/pensando\/sw\/api\/generated\/telemetry\".*/telemetry \"github.com\/pensando\/sw\/api\/generated\/monitoring\"/g" $@
	rm -f *.bak_

mock_apiclient.go: ${GOPATH}/src/github.com/pensando/sw/api/generated/apiclient/client.go Makefile
	mockgen -destination=$@ -package=mock --source ../generated/apiclient/client.go
	goimports -local "github.com/pensando/sw" -w $@

mock_network.go: ../generated/network/svc_network_crudinterface.go Makefile
	mockgen -destination=$@ -package=mock --source $< tenant
	sed -i.bak_ -e 's/\*LbPolicy/\*network.LbPolicy/g' $@
	sed -i.bak_ -e 's/\*Network/\*network.Network/g' $@
	sed -i.bak_ -e 's/\*Service/\*network.Service/g' $@
	sed -i.bak_ -e 's/[[:space:](]NetworkV1LbPolicyInterface[[:space:])]/ (network.NetworkV1LbPolicyInterface)/g' $@
	sed -i.bak_ -e 's/[[:space:](]NetworkV1ServiceInterface[[:space:])]/ (network.NetworkV1ServiceInterface)/g' $@
	sed -i.bak_ -e 's/[[:space:](]NetworkV1NetworkInterface[[:space:])]/ (network.NetworkV1NetworkInterface)/g' $@
	goimports -local "github.com/pensando/sw" -w $@
	sed -i.bak_ -e "s/\"github.com\/pensando\/sw\/api\/generated\/network\"/network \"github.com\/pensando\/sw\/api\/generated\/network\"/g" $@
	rm -f *.bak_

mock_cluster.go: ../generated/cluster/svc_cluster_crudinterface.go Makefile
	mockgen -destination=$@ -package=mock --source $< cluster
	sed -i.bak_ -e 's/\*Cluster/\*cluster.Cluster/g' $@
	sed -i.bak_ -e 's/\*Node/\*cluster.Node/g' $@
	sed -i.bak_ -e 's/\*Host/\*cluster.Host/g' $@
	sed -i.bak_ -e 's/\*SmartNIC/\*cluster.SmartNIC/g' $@
	sed -i.bak_ -e 's/\*Tenant/\*cluster.Tenant/g' $@
	sed -i.bak_ -e 's/[[:space:](]ClusterV1ClusterInterface[[:space:])]/ (cluster.ClusterV1ClusterInterface)/g' $@
	sed -i.bak_ -e 's/[[:space:](]ClusterV1NodeInterface[[:space:])]/ (cluster.ClusterV1NodeInterface)/g' $@
	sed -i.bak_ -e 's/[[:space:](]ClusterV1HostInterface[[:space:])]/ (cluster.ClusterV1HostInterface)/g' $@
	sed -i.bak_ -e 's/[[:space:](]ClusterV1SmartNICInterface[[:space:])]/ (cluster.ClusterV1SmartNICInterface)/g' $@
	sed -i.bak_ -e 's/[[:space:](]ClusterV1TenantInterface[[:space:])]/ (cluster.ClusterV1TenantInterface)/g' $@
	goimports -local "github.com/pensando/sw" -w $@
	sed -i.bak_ -e "s/\"github.com\/pensando\/sw\/api\/generated\/cluster\"/cluster \"github.com\/pensando\/sw\/api\/generated\/cluster\"/g" $@
		rm -f *.bak_


clean:
	rm -f ${TARGET}
