// Code generated by protoc-gen-grpc-pensando DO NOT EDIT.

/*
Package tpmprotos is a auto generated package.
Input file: tpm.proto
*/
package restapi

import (
	"testing"

	api "github.com/pensando/sw/api"
	monitoring "github.com/pensando/sw/api/generated/monitoring"
	tpmprotos "github.com/pensando/sw/venice/ctrler/tpm/rpcserver/protos"
	"github.com/pensando/sw/venice/utils/netutils"
	. "github.com/pensando/sw/venice/utils/testutils"
)

func TestFlowExportPolicyList(t *testing.T) {
	t.Parallel()
	var flowexportpolicyList []*tpmprotos.FlowExportPolicy
	err := netutils.HTTPGet("http://"+agentRestURL+"/api/telemetry/flowexports/", &flowexportpolicyList)
	AssertOk(t, err, "Error getting flowexportpolicys from the REST Server")
}

func TestFlowExportPolicyPost(t *testing.T) {
	var resp Response
	var flowexportpolicyGet tpmprotos.FlowExportPolicy
	postData := tpmprotos.FlowExportPolicy{
		TypeMeta: api.TypeMeta{Kind: "FlowExportPolicy"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      "testPostFlowExportPolicy",
		},
		Spec: tpmprotos.FlowExportPolicySpec{
			Interval: "15s",
			Format:   "IPFIX",
			Exports: []monitoring.ExportConfig{
				{
					Destination: "10.1.1.0",
					Transport:   "UDP/2055",
				},
			},
		},
	}
	err := netutils.HTTPPost("http://"+agentRestURL+"/api/telemetry/flowexports/", &postData, &resp)
	AssertOk(t, err, "Error posting flowexportpolicy to REST Server")
	getErr := netutils.HTTPGet("http://"+agentRestURL+"/api/telemetry/flowexports/default/default/"+"testPostFlowExportPolicy/", &flowexportpolicyGet)
	AssertOk(t, getErr, "Error getting flowexportpolicys from the REST Server")

}

func TestFlowExportPolicyDelete(t *testing.T) {
	var resp Response
	deleteData := tpmprotos.FlowExportPolicy{
		TypeMeta: api.TypeMeta{Kind: "FlowExportPolicy"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      "testDeleteFlowExportPolicy",
		},
		Spec: tpmprotos.FlowExportPolicySpec{
			Interval: "15s",
			Format:   "IPFIX",
			Exports: []monitoring.ExportConfig{
				{
					Destination: "10.1.1.0",
					Transport:   "UDP/2055",
				},
			},
		},
	}

	deleteMeta := tpmprotos.FlowExportPolicy{
		TypeMeta: api.TypeMeta{Kind: "FlowExportPolicy"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      "testDeleteFlowExportPolicy",
		},
	}
	err := netutils.HTTPPost("http://"+agentRestURL+"/api/telemetry/flowexports/", &deleteData, &resp)
	AssertOk(t, err, "Error posting flowexportpolicy to REST Server")
	delErr := netutils.HTTPDelete("http://"+agentRestURL+"/api/telemetry/flowexports/default/default/"+"testDeleteFlowExportPolicy", &deleteMeta, &resp)
	AssertOk(t, delErr, "Error deleting flowexportpolicys from the REST Server")
}
