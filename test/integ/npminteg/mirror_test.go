// {C} Copyright 2017 Pensando Systems Inc. All rights reserved.

package npminteg

import (
	"context"
	"fmt"

	agentTypes "github.com/pensando/sw/nic/agent/dscagent/types"
	"github.com/pensando/sw/nic/agent/protos/netproto"
	"github.com/pensando/sw/venice/globals"

	. "gopkg.in/check.v1"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/api/generated/monitoring"
	. "github.com/pensando/sw/venice/utils/testutils"
)

func (it *integTestSuite) TestNpmMirrorPolicy(c *C) {
	// clean up stale mirror Policies
	policies, err := it.apisrvClient.MonitoringV1().MirrorSession().List(context.Background(), &api.ListWatchOptions{ObjectMeta: api.ObjectMeta{Tenant: globals.DefaultTenant}})
	AssertOk(c, err, "failed to list policies")

	for _, p := range policies {
		_, err := it.apisrvClient.MonitoringV1().MirrorSession().Delete(context.Background(), &p.ObjectMeta)
		AssertOk(c, err, "failed to clean up policy. NSP: %v | Err: %v", p.GetKey(), err)
	}

	mr := monitoring.MirrorSession{
		TypeMeta: api.TypeMeta{Kind: "MirrorSession"},
		ObjectMeta: api.ObjectMeta{
			Tenant:       "default",
			Namespace:    "default",
			Name:         "test-mirror",
			GenerationID: "1",
		},
		Spec: monitoring.MirrorSessionSpec{
			PacketSize:    128,
			PacketFilters: []string{monitoring.MirrorSessionSpec_ALL_PKTS.String()},
			Collectors: []monitoring.MirrorCollector{
				{
					Type: monitoring.PacketCollectorType_ERSPAN_TYPE_3.String(),
					ExportCfg: &monitoring.MirrorExportConfig{
						Destination: "100.1.1.1",
					},
				},
			},
			MatchRules: []monitoring.MatchRule{
				{
					Src: &monitoring.MatchSelector{
						IPAddresses: []string{"10.1.1.10"},
					},
					AppProtoSel: &monitoring.AppProtoSelector{
						ProtoPorts: []string{"TCP/5555"},
					},
				},
				{
					Src: &monitoring.MatchSelector{
						IPAddresses: []string{"10.2.2.20"},
					},
					AppProtoSel: &monitoring.AppProtoSelector{
						ProtoPorts: []string{"UDP/5555"},
					},
				},
			},
		},
		Status: monitoring.MirrorSessionStatus{
			ScheduleState: "none",
		},
	}

	// create sg policy
	_, err = it.apisrvClient.MonitoringV1().MirrorSession().Create(context.Background(), &mr)
	AssertOk(c, err, "error creating mirror policy")

	// verify agent state has the policy and has the rules
	for _, ag := range it.agents {
		AssertEventually(c, func() (bool, interface{}) {
			nsgp := netproto.MirrorSession{
				TypeMeta:   api.TypeMeta{Kind: "MirrorSession"},
				ObjectMeta: mr.ObjectMeta,
			}
			gsgp, gerr := ag.dscAgent.PipelineAPI.HandleMirrorSession(agentTypes.Get, nsgp)
			if gerr != nil {
				return false, nil
			}
			if len(gsgp[0].Spec.MatchRules) != len(mr.Spec.MatchRules) {
				return false, gsgp
			}
			return true, nil
		}, fmt.Sprintf("Mirror Policy not found in agent. SGP: %v", mr.GetKey()), "10ms", it.pollTimeout())
	}

	/*
		// wait a little so that we dont cause a race condition between NPM write ans updates
		time.Sleep(time.Millisecond * 10)

		// update the policy
		newRule := security.SGRule{
			Action:          "PERMIT",
			ToIPAddresses:   []string{"10.2.1.1/24"},
			FromIPAddresses: []string{"10.2.1.1/24"},
			ProtoPorts: []security.ProtoPort{
				{
					Protocol: "tcp",
					Ports:    "81",
				},
			},
		}
		sgp.Spec.Rules = append(sgp.Spec.Rules, newRule)
		sgp.ObjectMeta.GenerationID = "2"
		_, err = it.apisrvClient.SecurityV1().NetworkSecurityPolicy().Update(context.Background(), &:71)
		AssertOk(c, err, "error updating sg policy")

		// verify agent state updated policy
		for _, ag := range it.agents {
			AssertEventually(c, func() (bool, interface{}) {
				nsgp := netproto.NetworkSecurityPolicy{
					TypeMeta:   api.TypeMeta{Kind: "NetworkSecurityPolicy"},
					ObjectMeta: sgp.ObjectMeta,
				}
				gsgp, gerr := ag.dscAgent.PipelineAPI.HandleNetworkSecurityPolicy(agentTypes.Get, nsgp)

				if gerr != nil {
					return false, nil
				}
				if len(gsgp[0].Spec.Rules) != len(sgp.Spec.Rules) {
					return false, gsgp
				}
				return true, nil
			}, fmt.Sprintf("SGPolicy not found in agent. SGP: %v", sgp.GetKey()), "10ms", it.pollTimeout())
		}

		// verify sgpolicy status reflects propagation status
		AssertEventually(c, func() (bool, interface{}) {
			tsgp, gerr := it.apisrvClient.SecurityV1().NetworkSecurityPolicy().Get(context.Background(), &sgp.ObjectMeta)
			if gerr != nil {
				return false, gerr
			}
			if (tsgp.Status.PropagationStatus.Updated != int32(it.numAgents)) || (tsgp.Status.PropagationStatus.Pending != 0) ||
				(tsgp.Status.PropagationStatus.MinVersion != "" || len(tsgp.Status.RuleStatus) != len(sgp.Spec.Rules)) {
				return false, tsgp
			}
			return true, nil
		}, "SgPolicy status was not updated after updating the policy", "100ms", it.pollTimeout())

		// delete sg policy
		_, err = it.apisrvClient.SecurityV1().NetworkSecurityPolicy().Delete(context.Background(), &sgp.ObjectMeta)
		AssertOk(c, err, "error deleting sg policy")
	*/
}
