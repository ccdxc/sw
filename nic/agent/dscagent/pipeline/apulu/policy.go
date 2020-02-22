// +build apulu

package apulu

import (
	"context"
	"encoding/json"
	"fmt"
	"sync"

	"github.com/pkg/errors"

	"github.com/pensando/sw/nic/agent/dscagent/pipeline/apulu/utils"
	"github.com/pensando/sw/nic/agent/dscagent/types"
	"github.com/pensando/sw/nic/agent/protos/netproto"
	halapi "github.com/pensando/sw/nic/apollo/agent/gen/pds"
	"github.com/pensando/sw/venice/utils/log"
)

// HandleNetworkSecurityPolicy handles crud operations on nsp
func HandleNetworkSecurityPolicy(infraAPI types.InfraAPI, client halapi.SecurityPolicySvcClient, oper types.Operation, nsp netproto.NetworkSecurityPolicy, ruleIDToAppMapping *sync.Map) error {
	switch oper {
	case types.Create:
		return createNetworkSecurityPolicyHandler(infraAPI, client, nsp, ruleIDToAppMapping)
	case types.Update:
		return updateNetworkSecurityPolicyHandler(infraAPI, client, nsp, ruleIDToAppMapping)
	case types.Delete:
		return deleteNetworkSecurityPolicyHandler(infraAPI, client, nsp)
	default:
		return errors.Wrapf(types.ErrUnsupportedOp, "Op: %s", oper)
	}
}

func createNetworkSecurityPolicyHandler(infraAPI types.InfraAPI, client halapi.SecurityPolicySvcClient, nsp netproto.NetworkSecurityPolicy, ruleIDToAppMapping *sync.Map) error {
	c, _ := json.Marshal(nsp)
	log.Infof("Create SGP Req to Agent: %v", string(c))

	nspReqMsg := convertNetworkSecurityPolicy(infraAPI, nsp, ruleIDToAppMapping)
	b, _ := json.Marshal(nspReqMsg)
	log.Infof("Create SGP Req to Datapath: %s", string(b))

	resp, err := client.SecurityPolicyCreate(context.Background(), nspReqMsg)
	log.Infof("Datapath Create SGP Response: %v", resp)
	if resp != nil {
		if err := utils.HandleErr(types.Create, resp.ApiStatus, err, fmt.Sprintf("Create Failed for %s | %s", nsp.GetKind(), nsp.GetKey())); err != nil {
			return err
		}
	}
	dat, _ := nsp.Marshal()

	if err := infraAPI.Store(nsp.Kind, nsp.GetKey(), dat); err != nil {
		log.Error(errors.Wrapf(types.ErrBoltDBStoreCreate, "NetworkSecurityPolicy: %s | NetworkSecurityPolicy: %v", nsp.GetKey(), err))
		return errors.Wrapf(types.ErrBoltDBStoreCreate, "NetworkSecurityPolicy: %s | NetworkSecurityPolicy: %v", nsp.GetKey(), err)
	}
	return nil
}

func updateNetworkSecurityPolicyHandler(infraAPI types.InfraAPI, client halapi.SecurityPolicySvcClient, nsp netproto.NetworkSecurityPolicy, ruleIDToAppMapping *sync.Map) error {
	c, _ := json.Marshal(nsp)
	log.Infof("Update SGP Req to Agent: %s", string(c))

	nspReqMsg := convertNetworkSecurityPolicy(infraAPI, nsp, ruleIDToAppMapping)
	b, _ := json.Marshal(nspReqMsg)
	log.Infof("Update SGP Req to Datapath: %s", string(b))
	resp, err := client.SecurityPolicyUpdate(context.Background(), nspReqMsg)
	log.Infof("Datapath Update SGP Response: %v", resp)

	if resp != nil {
		if err := utils.HandleErr(types.Update, resp.ApiStatus, err, fmt.Sprintf("Update Failed for %s | %s", nsp.GetKind(), nsp.GetKey())); err != nil {
			return err
		}
	}
	dat, _ := nsp.Marshal()

	if err := infraAPI.Store(nsp.Kind, nsp.GetKey(), dat); err != nil {
		log.Error(errors.Wrapf(types.ErrBoltDBStoreUpdate, "NetworkSecurityPolicy: %s | NetworkSecurityPolicy: %v", nsp.GetKey(), err))
		return errors.Wrapf(types.ErrBoltDBStoreUpdate, "NetworkSecurityPolicy: %s | NetworkSecurityPolicy: %v", nsp.GetKey(), err)
	}
	return nil
}

func deleteNetworkSecurityPolicyHandler(infraAPI types.InfraAPI, client halapi.SecurityPolicySvcClient, nsp netproto.NetworkSecurityPolicy) error {
	nspDelReq := &halapi.SecurityPolicyDeleteRequest{
		Id: [][]byte{
			[]byte(nsp.UUID),
		},
	}

	resp, err := client.SecurityPolicyDelete(context.Background(), nspDelReq)
	if resp != nil {
		if err := utils.HandleErr(types.Delete, resp.ApiStatus[0], err, fmt.Sprintf("NetworkSecurityPolicy: %s", nsp.GetKey())); err != nil {
			return err
		}
	}

	if err := infraAPI.Delete(nsp.Kind, nsp.GetKey()); err != nil {
		log.Error(errors.Wrapf(types.ErrBoltDBStoreDelete, "NetworkSecurityPolicy: %s | Err: %v", nsp.GetKey(), err))
		return errors.Wrapf(types.ErrBoltDBStoreDelete, "NetworkSecurityPolicy: %s | Err: %v", nsp.GetKey(), err)
	}
	return nil
}

func convertNetworkSecurityPolicy(infraAPI types.InfraAPI, nsp netproto.NetworkSecurityPolicy, ruleIDToAppMapping *sync.Map) *halapi.SecurityPolicyRequest {
	fwRules := convertHALFirewallRules(nsp, ruleIDToAppMapping)
	for _, rule := range fwRules {
		rule.Priority = uint32(infraAPI.AllocateID(types.SecurityRulePriority, 0))
	}
	return &halapi.SecurityPolicyRequest{
		Request: []*halapi.SecurityPolicySpec{
			{
				Id:              []byte(nsp.UUID),
				AddrFamily:      halapi.IPAF_IP_AF_INET,
				DefaultFWAction: halapi.SecurityRuleAction_SECURITY_RULE_ACTION_ALLOW,
				Rules:           fwRules,
			},
		},
	}
}
