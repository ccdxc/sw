// {C} Copyright 2017 Pensando Systems Inc. All rights reserved.

package statemgr

import (
	"fmt"
	"hash/fnv"
	"strconv"

	"github.com/pensando/sw/api/generated/ctkit"
	"github.com/pensando/sw/api/generated/security"
	"github.com/pensando/sw/nic/agent/netagent/protos/netproto"
	"github.com/pensando/sw/venice/utils/log"
	"github.com/pensando/sw/venice/utils/runtime"
)

// SgpolicyState security policy state
type SgpolicyState struct {
	SGPolicy     *ctkit.SGPolicy                `json:"-"` // embedded security policy object
	groups       map[string]*SecurityGroupState // list of groups this policy is attached to
	stateMgr     *Statemgr                      // pointer to state manager
	NodeVersions map[string]string              // Map for node -> version
}

func versionToInt(v string) int {
	i, err := strconv.Atoi(v)
	if err != nil {
		return 0
	}
	return i
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

func convertSGPolicy(sgp *SgpolicyState) *netproto.SGPolicy {
	// build sg message
	nsg := netproto.SGPolicy{
		TypeMeta:   sgp.SGPolicy.TypeMeta,
		ObjectMeta: sgp.SGPolicy.ObjectMeta,
		Spec: netproto.SGPolicySpec{
			AttachTenant: sgp.SGPolicy.Spec.AttachTenant,
			AttachGroup:  sgp.SGPolicy.Spec.AttachGroups,
			Rules:        convertRules(sgp.SGPolicy.Spec.Rules),
		},
	}

	return &nsg
}

// GetKey returns the key of SGPolicy
func (sgp *SgpolicyState) GetKey() string {
	return sgp.SGPolicy.GetKey()
}

// Write writes the object to api server
func (sgp *SgpolicyState) Write() error {
	sgp.SGPolicy.Lock()
	defer sgp.SGPolicy.Unlock()

	// Consolidate the NodeVersions
	prop := &sgp.SGPolicy.Status.PropagationStatus
	prop.GenerationID = sgp.SGPolicy.GenerationID
	prop.Updated = 0
	prop.Pending = 0
	prop.MinVersion = ""
	for _, ver := range sgp.NodeVersions {
		if ver == prop.GenerationID {
			prop.Updated++
		} else {
			prop.Pending++
			if versionToInt(ver) < versionToInt(prop.MinVersion) {
				prop.MinVersion = ver
			}
		}
	}

	return sgp.SGPolicy.Write()
}

// Delete cleans up all state associated with the sg
func (sgp *SgpolicyState) Delete() error {
	// inform all SGs to remove the policy
	for _, sg := range sgp.groups {
		err := sg.DeletePolicy(sgp)
		if err != nil {
			log.Errorf("Error deleting policy %s from sg %s. Err: %v", sgp.SGPolicy.Name, sg.SecurityGroup.Name, err)
		}
	}

	return nil
}

// updateAttachedSgs update all attached sgs
func (sgp *SgpolicyState) updateAttachedSgs() error {
	// make sure the attached security group exists
	for _, sgname := range sgp.SGPolicy.Spec.AttachGroups {
		sgs, err := sgp.stateMgr.FindSecurityGroup(sgp.SGPolicy.Tenant, sgname)
		if err != nil {
			log.Errorf("Could not find the security group %s. Err: %v", sgname, err)
			return err
		}

		// add the policy to sg
		err = sgs.AddPolicy(sgp)
		if err != nil {
			log.Errorf("Error adding policy %s to sg %s. Err: %v", sgp.SGPolicy.Name, sgname, err)
			return err
		}

		// link sgpolicy to sg
		sgp.groups[sgs.SecurityGroup.Name] = sgs
	}

	return nil
}

// updateAttachedApps walks all referred apps and links them
func (sgp *SgpolicyState) updateAttachedApps() error {
	for _, rule := range sgp.SGPolicy.Spec.Rules {
		if len(rule.Apps) != 0 {
			for _, appName := range rule.Apps {
				app, err := sgp.stateMgr.FindApp(sgp.SGPolicy.Tenant, appName)
				if err != nil {
					log.Errorf("Error finding app %v for policy %v, rule {%v}", appName, sgp.SGPolicy.Name, rule)
					return err
				}

				app.attachPolicy(sgp.SGPolicy.Name)
			}
		}
	}

	return nil
}

// detachFromApps detaches the poliy from apps
func (sgp *SgpolicyState) detachFromApps() error {
	for _, rule := range sgp.SGPolicy.Spec.Rules {
		if len(rule.Apps) != 0 {
			for _, appName := range rule.Apps {
				app, err := sgp.stateMgr.FindApp(sgp.SGPolicy.Tenant, appName)
				if err != nil {
					log.Errorf("Error finding app %v for policy %v, rule {%v}", appName, sgp.SGPolicy.Name, rule)
				} else {
					app.detachPolicy(sgp.SGPolicy.Name)
				}
			}
		}
	}

	return nil
}

// SgpolicyStateFromObj converts from memdb object to sgpolicy state
func SgpolicyStateFromObj(obj runtime.Object) (*SgpolicyState, error) {
	switch obj.(type) {
	case *ctkit.SGPolicy:
		sgpobj := obj.(*ctkit.SGPolicy)
		switch sgpobj.HandlerCtx.(type) {
		case *SgpolicyState:
			sgps := sgpobj.HandlerCtx.(*SgpolicyState)
			return sgps, nil
		default:
			return nil, ErrIncorrectObjectType
		}
	default:
		return nil, ErrIncorrectObjectType
	}
}

// NewSgpolicyState creates a new security policy state object
func NewSgpolicyState(sgp *ctkit.SGPolicy, stateMgr *Statemgr) (*SgpolicyState, error) {
	// create sg state object
	sgps := SgpolicyState{
		SGPolicy:     sgp,
		groups:       make(map[string]*SecurityGroupState),
		stateMgr:     stateMgr,
		NodeVersions: make(map[string]string),
	}
	sgp.HandlerCtx = &sgps

	return &sgps, nil
}

// OnSGPolicyAgentStatusSet gets called when policy updates arrive from agents
func (sm *Statemgr) OnSGPolicyAgentStatusSet(nodeID string, objinfo *netproto.SGPolicy) error {
	sm.UpdateSgpolicyStatus(nodeID, objinfo.ObjectMeta.Tenant, objinfo.ObjectMeta.Name, objinfo.ObjectMeta.GenerationID)
	return nil
}

// OnSGPolicyAgentStatusDelete gets called when policy delete arrives from agent
func (sm *Statemgr) OnSGPolicyAgentStatusDelete(nodeID string, objinfo *netproto.SGPolicy) error {
	return nil
}

// FindSgpolicy finds sg policy by name
func (sm *Statemgr) FindSgpolicy(tenant, name string) (*SgpolicyState, error) {
	// find the object
	obj, err := sm.FindObject("SGPolicy", tenant, "default", name)
	if err != nil {
		return nil, err
	}

	return SgpolicyStateFromObj(obj)
}

// ListSgpolicies lists all sg policies
func (sm *Statemgr) ListSgpolicies() ([]*SgpolicyState, error) {
	objs := sm.ListObjects("SGPolicy")

	var sgs []*SgpolicyState
	for _, obj := range objs {
		sg, err := SgpolicyStateFromObj(obj)
		if err != nil {
			return sgs, err
		}

		sgs = append(sgs, sg)
	}

	return sgs, nil
}

// OnSGPolicyCreate creates a sg policy
func (sm *Statemgr) OnSGPolicyCreate(sgp *ctkit.SGPolicy) error {
	// create new sg state
	sgps, err := NewSgpolicyState(sgp, sm)
	if err != nil {
		log.Errorf("Error creating new sg policy state. Err: %v", err)
		return err
	}

	// find and update all attached apps
	err = sgps.updateAttachedApps()
	if err != nil {
		log.Errorf("Error updating attached apps. Err: %v", err)
		return err
	}

	// store it in local DB
	err = sm.mbus.AddObject(convertSGPolicy(sgps))
	if err != nil {
		log.Errorf("Error storing the sg policy in memdb. Err: %v", err)
		return err
	}

	// make sure the attached security group exists
	err = sgps.updateAttachedSgs()
	if err != nil {
		log.Errorf("Error attching policy to sgs. Err: %v", err)
		return err
	}

	return nil
}

// OnSGPolicyUpdate updates a sg policy
func (sm *Statemgr) OnSGPolicyUpdate(sgp *ctkit.SGPolicy) error {
	// find the policy state
	sgps, err := sm.FindSgpolicy(sgp.ObjectMeta.Tenant, sgp.ObjectMeta.Name)
	if err != nil {
		log.Errorf("Can find sg policy for updating {%+v}. Err: {%v}", sgp.ObjectMeta, err)
		return fmt.Errorf("Can not find sg policy")
	}

	// find and update all attached apps
	err = sgps.updateAttachedApps()
	if err != nil {
		log.Errorf("Error updating attached apps. Err: %v", err)
		return err
	}

	// store it in local DB
	err = sm.mbus.UpdateObject(convertSGPolicy(sgps))
	if err != nil {
		log.Errorf("Error storing the sg policy in memdb. Err: %v", err)
		return err
	}

	// make sure the attached security group exists
	err = sgps.updateAttachedSgs()
	if err != nil {
		log.Errorf("Error attching policy to sgs. Err: %v", err)
		return err
	}

	return nil
}

// OnSGPolicyDelete deletes a sg policy
func (sm *Statemgr) OnSGPolicyDelete(sgpo *ctkit.SGPolicy) error {
	// see if we already have it
	sgp, err := sm.FindSgpolicy(sgpo.Tenant, sgpo.Name)
	if err != nil {
		log.Errorf("Can not find the sg policy %s|%s", sgpo.Tenant, sgpo.Name)
		return fmt.Errorf("Sgpolicy not found")
	}

	// cleanup sg state
	err = sgp.Delete()
	if err != nil {
		log.Errorf("Error deleting the sg policy {%+v}. Err: %v", sgp.SGPolicy.ObjectMeta, err)
		return err
	}

	// detach from policies
	err = sgp.detachFromApps()
	if err != nil {
		return err
	}

	// delete it from the DB
	return sm.mbus.DeleteObject(convertSGPolicy(sgp))
}

// UpdateSgpolicyStatus updates the status of an sg policy
func (sm *Statemgr) UpdateSgpolicyStatus(nodeuuid, tenant, name, generationID string) {
	policy, err := sm.FindSgpolicy(tenant, name)
	if err != nil {
		return
	}

	// lock policy for concurrent modifications
	policy.SGPolicy.Lock()
	defer policy.SGPolicy.Unlock()

	if policy.NodeVersions == nil {
		policy.NodeVersions = make(map[string]string)
	}
	policy.NodeVersions[nodeuuid] = generationID

	sm.PeriodicUpdaterPush(policy)
}
