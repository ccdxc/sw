package ntsdb

import (
	"testing"

	"github.com/pensando/sw/api"
	. "github.com/pensando/sw/venice/utils/testutils"
)

func TestGetKeys(t *testing.T) {
	ep := endpoint{}
	ep.ObjectMeta.Tenant = "dept1"
	ep.TypeMeta.Kind = "endpoint"
	ep.ObjectMeta.Name = "vm-zone22"

	keys := make(map[string]string)
	tableName, err := getKeys(ep, keys)
	AssertOk(t, err, "unable to get keys")
	Assert(t, tableName == "endpoint", "invalid table name: %s", tableName)
	Assert(t, keys["Tenant"] == "dept1", "invalid tenant: %s", keys["Tenant"])
	Assert(t, keys["Kind"] == "endpoint", "invalid kind: %s", keys["Kind"])
	Assert(t, keys["Name"] == "vm-zone22", "invalid name: %s", keys["Name"])

	ep.ObjectMeta.Namespace = "prod"
	tableName, err = getKeys(ep, keys)
	AssertOk(t, err, "unable to get keys")
	Assert(t, tableName == "endpoint", "invalid table name: %s", tableName)
	Assert(t, keys["Tenant"] == "dept1", "invalid tenant: %s", keys["Tenant"])
	Assert(t, keys["Kind"] == "endpoint", "invalid kind: %s", keys["Kind"])
	Assert(t, keys["Name"] == "vm-zone22", "invalid name: %s", keys["Name"])
	Assert(t, keys["Namespace"] == "prod", "invalid namespace: %s", keys["Namespace"])
}

func TestGetKeysError(t *testing.T) {
	ep := &endpoint{}
	keys := make(map[string]string)
	_, err := getKeys(ep, keys)
	Assert(t, err != nil, "got success on empty object, keys %v", keys)

	ep = &endpoint{}
	keys = make(map[string]string)
	ep.TypeMeta.Kind = "endpoint"
	_, err = getKeys(ep, keys)
	Assert(t, err != nil, "got success on object without name, keys %v", keys)

	ep = &endpoint{}
	keys = make(map[string]string)
	ep.ObjectMeta.Name = "vm-zone22"
	_, err = getKeys(ep, keys)
	Assert(t, err != nil, "got success on object without kind, keys %v", keys)
}

func TestFillFields(t *testing.T) {
	table := &iTable{}
	table.fields = make(map[string]interface{})

	epm := &endpointMetric{}

	err := fillFields(table, epm)
	AssertOk(t, err, "filling fields from metric object")

	_, ok := table.fields["OutgoingConns"].(api.Counter)
	Assert(t, ok, "unable to find field")
	_, ok = epm.OutgoingConns.(api.Counter)
	Assert(t, ok, "unable to find field")

	_, ok = table.fields["IncomingConns"].(api.Counter)
	Assert(t, ok, "unable to find field")
	_, ok = epm.IncomingConns.(api.Counter)
	Assert(t, ok, "unable to find field")

	_, ok = table.fields["Bandwidth"].(api.Gauge)
	Assert(t, ok, "unable to find field")
	_, ok = epm.Bandwidth.(api.Gauge)
	Assert(t, ok, "unable to find field")

	_, ok = table.fields["PacketErrors"].(api.Counter)
	Assert(t, ok, "unable to find field")
	_, ok = epm.PacketErrors.(api.Counter)
	Assert(t, ok, "unable to find field")

	_, ok = table.fields["Violations"].(api.Counter)
	Assert(t, ok, "unable to find field")
	_, ok = epm.Violations.(api.Counter)
	Assert(t, ok, "unable to find field")

	_, ok = table.fields["LinkUp"].(api.Bool)
	Assert(t, ok, "unable to find field")
	_, ok = epm.LinkUp.(api.Bool)
	Assert(t, ok, "unable to find field")

	_, ok = table.fields["WorkloadName"].(api.String)
	Assert(t, ok, "unable to find field")
	_, ok = epm.WorkloadName.(api.String)
	Assert(t, ok, "unable to find field")

	_, ok = table.fields["RxPacketSize"].(api.Histogram)
	Assert(t, ok, "unable to find field")
	_, ok = epm.RxPacketSize.(api.Histogram)
	Assert(t, ok, "unable to find field")

	_, ok = table.fields["TxPacketSize"].(api.Histogram)
	Assert(t, ok, "unable to find field")
	_, ok = epm.TxPacketSize.(api.Histogram)
	Assert(t, ok, "unable to find field")

	_, ok = table.fields["RxBandwidth"].(api.Summary)
	Assert(t, ok, "unable to find field")
	_, ok = epm.RxBandwidth.(api.Summary)
	Assert(t, ok, "unable to find field")

	_, ok = table.fields["TxBandwidth"].(api.Summary)
	Assert(t, ok, "unable to find field")
	_, ok = epm.TxBandwidth.(api.Summary)
	Assert(t, ok, "unable to find field")
}
