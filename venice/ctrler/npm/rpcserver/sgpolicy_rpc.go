// {C} Copyright 2017 Pensando Systems Inc. All rights reserved.

package rpcserver

import (
	"errors"
	"fmt"
	"hash/fnv"

	"golang.org/x/net/context"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/api/generated/security"
	"github.com/pensando/sw/nic/agent/netagent/protos/netproto"
	"github.com/pensando/sw/venice/ctrler/npm/statemgr"
	"github.com/pensando/sw/venice/utils/log"
	"github.com/pensando/sw/venice/utils/memdb"
	"github.com/pensando/sw/venice/utils/netutils"
)

// SGPolicyRPCServer serves security rpc requests
type SGPolicyRPCServer struct {
	stateMgr *statemgr.Statemgr // reference to network manager
}

// UpdateSGPolicyStatus updates the status part of the sg policy
func (s *SGPolicyRPCServer) UpdateSGPolicyStatus(ctx context.Context, policy *netproto.SGPolicy) (*netproto.SGPolicy, error) {
	s.stateMgr.UpdateSgpolicyStatus(netutils.GetNodeUUIDFromCtx(ctx),
		policy.ObjectMeta.Tenant, policy.ObjectMeta.Name,
		policy.ObjectMeta.GenerationID)
	return policy, nil
}

// GetSGPolicy gets sg policy
func (s *SGPolicyRPCServer) GetSGPolicy(ctx context.Context, ometa *api.ObjectMeta) (*netproto.SGPolicy, error) {
	// find the sgp
	sgp, err := s.stateMgr.FindSgpolicy(ometa.Tenant, ometa.Name)
	if err != nil {
		log.Errorf("Could not find the sg %s|%s", ometa.Tenant, ometa.Name)
		return nil, fmt.Errorf("could not find the sg policy %v", ometa.Name)
	}

	// build sg message
	sg := netproto.SGPolicy{
		TypeMeta:   sgp.TypeMeta,
		ObjectMeta: sgp.ObjectMeta,
		Spec: netproto.SGPolicySpec{
			AttachTenant: sgp.Spec.AttachTenant,
			AttachGroup:  sgp.Spec.AttachGroups,
			Rules:        convertRules(sgp.Spec.Rules),
		},
	}

	return &sg, nil
}

// ListSGPolicys returns a list of sg policies
func (s *SGPolicyRPCServer) ListSGPolicys(context.Context, *api.ObjectMeta) (*netproto.SGPolicyList, error) {
	var sgpList netproto.SGPolicyList
	// get all sgs
	sgPolicies, err := s.stateMgr.ListSgpolicies()
	if err != nil {
		return nil, err
	}

	// walk all sgs in the list
	for _, sgp := range sgPolicies {

		// build sg message
		sg := netproto.SGPolicy{
			TypeMeta:   sgp.TypeMeta,
			ObjectMeta: sgp.ObjectMeta,
			Spec: netproto.SGPolicySpec{
				AttachTenant: sgp.Spec.AttachTenant,
				AttachGroup:  sgp.Spec.AttachGroups,
				Rules:        convertRules(sgp.Spec.Rules),
			},
		}
		sgpList.Sgpolicies = append(sgpList.Sgpolicies, &sg)
	}

	return &sgpList, nil
}

// WatchSGPolicys watches for sg changes
func (s *SGPolicyRPCServer) WatchSGPolicys(sel *api.ObjectMeta, stream netproto.SGPolicyApi_WatchSGPolicysServer) error {
	// watch for changes
	watchChan := make(chan memdb.Event, memdb.WatchLen)
	defer close(watchChan)
	s.stateMgr.WatchObjects("SGPolicy", watchChan)
	defer s.stateMgr.StopWatchObjects("SGPolicy", watchChan)

	// get a list of all sg policies
	sgPolicies, err := s.ListSGPolicys(context.Background(), sel)
	if err != nil {
		log.Errorf("Error getting a list of sgs. Err: %v", err)
		return err
	}

	ctx := stream.Context()

	// send the objects out as a stream
	for _, sgp := range sgPolicies.Sgpolicies {
		watchEvt := netproto.SGPolicyEvent{
			EventType: api.EventType_CreateEvent,
			SGPolicy:  *sgp,
		}
		err = stream.Send(&watchEvt)
		if err != nil {
			log.Errorf("Error sending stream. Err: %v", err)
			return err
		}
	}

	// loop forever on watch channel
	for {
		select {
		// read from channel
		case evt, ok := <-watchChan:
			if !ok {
				log.Errorf("Error reading from channel. Closing watch")
				return errors.New("error reading from channel")
			}

			// get event type from memdb event
			var etype api.EventType
			switch evt.EventType {
			case memdb.CreateEvent:
				etype = api.EventType_CreateEvent
			case memdb.UpdateEvent:
				etype = api.EventType_UpdateEvent
			case memdb.DeleteEvent:
				etype = api.EventType_DeleteEvent
			}

			// convert to sg policy object
			policyState, err := statemgr.SgpolicyStateFromObj(evt.Obj)
			if err != nil {
				continue
			}

			// construct the netproto object
			watchEvt := netproto.SGPolicyEvent{
				EventType: etype,
				SGPolicy: netproto.SGPolicy{
					TypeMeta:   policyState.TypeMeta,
					ObjectMeta: policyState.ObjectMeta,
					Spec: netproto.SGPolicySpec{
						AttachTenant: policyState.Spec.AttachTenant,
						AttachGroup:  policyState.Spec.AttachGroups,
						Rules:        convertRules(policyState.Spec.Rules),
					},
				},
			}

			err = stream.Send(&watchEvt)
			if err != nil {
				log.Errorf("Error sending stream. Err: %v", err)
				return err
			}
		case <-ctx.Done():
			return ctx.Err()
		}
	}

}

// convertRules need not handle validation as the rules are already validate by the precommit api server hook
func convertRules(veniceRules []security.SGRule) (agentRules []netproto.PolicyRule) {
	for _, v := range veniceRules {
		a := netproto.PolicyRule{
			Action: v.Action,
			Src: &netproto.MatchSelector{
				SecurityGroups: v.FromSecurityGroups,
				Addresses:      v.FromIPAddresses,
			},
			Dst: &netproto.MatchSelector{
				SecurityGroups: v.ToSecurityGroups,
				Addresses:      v.ToIPAddresses,
				AppConfigs:     convertAppConfig(v.Apps, v.ProtoPorts),
			},
			ID: generateRuleHash(v),
		}
		agentRules = append(agentRules, a)
	}
	return
}

// convertAppConfig converts venice app information to port protocol for agent
// TODO Fix app association for NetAgent
func convertAppConfig(apps []string, protoPorts []security.ProtoPort) (agentAppConfigs []*netproto.AppConfig) {
	for _, pp := range protoPorts {
		c := netproto.AppConfig{
			Protocol: pp.Protocol,
			Port:     pp.Ports,
		}
		agentAppConfigs = append(agentAppConfigs, &c)
	}
	return
}

// generateRuleHash generates the hash of the rule
func generateRuleHash(r security.SGRule) uint64 {
	h := fnv.New64()
	rule, _ := r.Marshal()
	h.Write(rule)
	return h.Sum64()
}

// NewSGPolicyRPCServer returns a security RPC server
func NewSGPolicyRPCServer(stateMgr *statemgr.Statemgr) (*SGPolicyRPCServer, error) {
	return &SGPolicyRPCServer{stateMgr: stateMgr}, nil
}
