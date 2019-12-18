// {C} Copyright 2017 Pensando Systems Inc. All rights reserved.

package state

import (
	"errors"
	"fmt"
	"hash/fnv"
	"sync"

	"github.com/pensando/sw/venice/globals"

	"github.com/gogo/protobuf/proto"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/nic/agent/netagent/state/types"
	dnetproto "github.com/pensando/sw/nic/agent/protos/generated/delphi/netproto/delphi"
	"github.com/pensando/sw/nic/agent/protos/netproto"
	"github.com/pensando/sw/venice/utils/log"
)

// ErrNetworkSecurityPolicyNotFound is returned when sgpolicy is not found
var ErrNetworkSecurityPolicyNotFound = errors.New("sgpolicy not found")

// CreateNetworkSecurityPolicy creates a security group policy
func (na *Nagent) CreateNetworkSecurityPolicy(sgp *netproto.NetworkSecurityPolicy) error {
	var ruleIDAppLUT sync.Map
	err := na.validateMeta(sgp.Kind, sgp.ObjectMeta)
	if err != nil {
		return err
	}

	// Check  for max allowed SGPolicies
	if len(na.ListNetworkSecurityPolicy()) != 0 {
		log.Errorf("Exceeds maximum allowed Network Security Policy of %d", globals.MaxAllowedSGPolicies)
		return fmt.Errorf("exceeds maximum allowed Network Security Policy of %d", globals.MaxAllowedSGPolicies)
	}

	oldSgp, err := na.FindNetworkSecurityPolicy(sgp.ObjectMeta)
	if err == nil {
		if !proto.Equal(&oldSgp.Spec, &sgp.Spec) {
			log.Errorf("Network Security Policy  %+v already exists", oldSgp.GetKey())
			return errors.New("network security policy already exists")
		}

		log.Infof("Received duplicate network security policy create for ep {%+v}", sgp.GetKey())
		return nil
	}

	// find the corresponding namespace
	ns, err := na.FindNamespace(sgp.ObjectMeta)
	if err != nil {
		return err
	}

	// validate security group policy message
	if sgp.Spec.AttachTenant == false && len(sgp.Spec.AttachGroup) == 0 || sgp.Spec.AttachTenant == true && len(sgp.Spec.AttachGroup) != 0 {
		log.Errorf("Must specify one attachment point. Either attach-tenant or attach-group")
		return fmt.Errorf("invalid attachment point for %s. Must specify one attachment point. Either attach-tenant or attach-group", sgp.Name)
	}

	// find the corresponding vrf for the sg policy
	vrf, err := na.ValidateVrf(sgp.Tenant, sgp.Namespace, sgp.Spec.VrfName)
	if err != nil {
		log.Errorf("Failed to find the vrf %v", sgp.Spec.VrfName)
		return err
	}

	for i, r := range sgp.Spec.Rules {
		ruleHash := sgp.Spec.Rules[i].ID
		// Calculate the hash only if npm has not set it. Else use whatever is already set
		if ruleHash == 0 {
			ruleHash = generateRuleHash(&r, sgp.GetKey())
			sgp.Spec.Rules[i].ID = ruleHash
		}

		if len(r.AppName) > 0 {
			meta := api.ObjectMeta{
				Tenant:    sgp.Tenant,
				Namespace: sgp.Namespace,
				Name:      r.AppName,
			}
			app, err := na.FindApp(meta)
			if err != nil {
				log.Errorf("could not find the corresponding app. %v", r.AppName)
				return fmt.Errorf("could not find the corresponding app. %v", r.AppName)
			}

			if app != nil && (len(r.Dst.ProtoPorts) != 0 || len(r.Src.ProtoPorts) != 0) {
				log.Errorf("cannot specify app-configs and an app name in the same rule. AppName: %v. SrcAppConfigs: %v. DstAppConfigs: %v", r.AppName, r.Src.ProtoPorts, r.Dst.ProtoPorts)
				return fmt.Errorf("cannot specify app-configs and an app name in the same rule. AppName: %v. SrcAppConfigs: %v. DstAppConfigs: %v", r.AppName, r.Src.ProtoPorts, r.Dst.ProtoPorts)

			}

			ruleIDAppLUT.Store(i, app)
		}
	}

	// Allocate ID only on first object creates and use existing ones during config replay
	if sgp.Status.NetworkSecurityPolicyID == 0 {
		sgp.Status.NetworkSecurityPolicyID, err = na.Store.GetNextID(types.NetworkSecurityPolicyID)
	}

	if err != nil {
		log.Errorf("Could not allocate security group policy id. {%+v}", err)
		return err
	}

	// create it in datapath
	err = na.Datapath.CreateNetworkSecurityPolicy(sgp, vrf.Status.VrfID, &ruleIDAppLUT)
	if err != nil {
		log.Errorf("Error creating security group policy in datapath. NetworkSecurityPolicy {%+v}. Err: %v", sgp.GetKey(), err)
		return err
	}

	// Add the current sg policy as a dependency to the namespace.
	err = na.Solver.Add(ns, sgp)
	if err != nil {
		log.Errorf("Could not add dependency. Parent: %v. Child: %v", ns, sgp)
		return err
	}
	err = na.Solver.Add(vrf, sgp)
	if err != nil {
		log.Errorf("Could not add dependency. Parent: %v. Child: %v", vrf, sgp)
		return err
	}

	// save it in db
	err = na.saveNetworkSecurityPolicy(sgp)

	return err
}

func (na *Nagent) saveNetworkSecurityPolicy(sgp *netproto.NetworkSecurityPolicy) error {
	// save it in db
	key := na.Solver.ObjectKey(sgp.ObjectMeta, sgp.TypeMeta)
	na.Lock()
	na.NetworkSecurityPolicyDB[key] = sgp
	na.Unlock()

	// write to delphi
	if na.DelphiClient != nil {
		dsgp := dnetproto.NetworkSecurityPolicy{
			Key:                   key,
			NetworkSecurityPolicy: sgp,
		}

		err := na.DelphiClient.SetObject(&dsgp)
		if err != nil {
			log.Errorf("Error writing NetworkSecurityPolicy %s to delphi. Err: %v", key, err)
			return err
		}
	}

	dat, _ := sgp.Marshal()
	err := na.Store.RawWrite(sgp.GetKind(), sgp.GetKey(), dat)

	return err
}

func (na *Nagent) discardNetworkSecurityPolicy(sgp *netproto.NetworkSecurityPolicy) error {
	// delete from db
	key := na.Solver.ObjectKey(sgp.ObjectMeta, sgp.TypeMeta)
	na.Lock()
	delete(na.NetworkSecurityPolicyDB, key)
	na.Unlock()

	// remove it from delphi
	if na.DelphiClient != nil {
		dsgp := dnetproto.NetworkSecurityPolicy{
			Key:                   key,
			NetworkSecurityPolicy: sgp,
		}

		err := na.DelphiClient.DeleteObject(&dsgp)
		if err != nil {
			log.Errorf("Error Deleting NetworkSecurityPolicy %s from delphi. Err: %v", key, err)
			return err
		}
	}

	return na.Store.RawDelete(sgp.GetKind(), sgp.GetKey())
}

// FindNetworkSecurityPolicy finds a security group policy in local db
func (na *Nagent) FindNetworkSecurityPolicy(meta api.ObjectMeta) (*netproto.NetworkSecurityPolicy, error) {
	typeMeta := api.TypeMeta{
		Kind: "NetworkSecurityPolicy",
	}
	// lock the db
	na.Lock()
	defer na.Unlock()

	// lookup the database
	key := na.Solver.ObjectKey(meta, typeMeta)
	sgp, ok := na.NetworkSecurityPolicyDB[key]
	if !ok {
		return nil, fmt.Errorf("security group policy not found %v", meta.Name)
	}

	return sgp, nil
}

// ListNetworkSecurityPolicy returns the list of security group polices
func (na *Nagent) ListNetworkSecurityPolicy() []*netproto.NetworkSecurityPolicy {
	var sgPolicyList []*netproto.NetworkSecurityPolicy
	// lock the db
	na.Lock()
	defer na.Unlock()

	for _, sgp := range na.NetworkSecurityPolicyDB {
		sgPolicyList = append(sgPolicyList, sgp)
	}

	return sgPolicyList
}

// UpdateNetworkSecurityPolicy updates a security group policy
func (na *Nagent) UpdateNetworkSecurityPolicy(sgp *netproto.NetworkSecurityPolicy) error {
	// find the corresponding namespace
	log.Infof("Got SG Policy Update: %v", sgp)
	_, err := na.FindNamespace(sgp.ObjectMeta)
	if err != nil {
		return err
	}
	existingSgp, err := na.FindNetworkSecurityPolicy(sgp.ObjectMeta)
	if err != nil {
		log.Errorf("NetworkSecurityPolicy %v not found", sgp.ObjectMeta)
		return err
	}

	log.Infof("Existing SG Policy: %v", existingSgp)

	// check if policy contents are same
	if proto.Equal(&existingSgp.Spec, &sgp.Spec) {
		log.Info("Nothing to update")
		return nil
	}

	// Populate the ID from existing sg policy to ensure that HAL recognizes this.
	sgp.Status.NetworkSecurityPolicyID = existingSgp.Status.NetworkSecurityPolicyID

	return na.performNetworkSecurityPolicyUpdate(sgp)
}

// performNetworkSecurityPolicyUpdate
func (na *Nagent) performNetworkSecurityPolicyUpdate(sgp *netproto.NetworkSecurityPolicy) error {
	log.Infof("Updating SG Policy: %v", sgp)

	var ruleIDAppLUT sync.Map

	// find the corresponding vrf for the sg policy
	vrf, err := na.ValidateVrf(sgp.Tenant, sgp.Namespace, sgp.Spec.VrfName)
	if err != nil {
		log.Errorf("Failed to find the vrf %v", sgp.Spec.VrfName)
		return err
	}
	// Recompute hash
	for i, r := range sgp.Spec.Rules {
		ruleHash := sgp.Spec.Rules[i].ID
		// Calculate the hash only if npm has not set it. Else use whatever is already set
		if ruleHash == 0 {
			ruleHash = generateRuleHash(&r, sgp.GetKey())
			sgp.Spec.Rules[i].ID = ruleHash
		}

		if len(r.AppName) > 0 {
			meta := api.ObjectMeta{
				Tenant:    sgp.Tenant,
				Namespace: sgp.Namespace,
				Name:      r.AppName,
			}
			app, err := na.FindApp(meta)
			if err != nil {
				log.Errorf("could not find the corresponding app. %v", r.AppName)
				return fmt.Errorf("could not find the corresponding app. %v", r.AppName)
			}

			ruleIDAppLUT.Store(i, app)
		}
	}

	err = na.Datapath.UpdateNetworkSecurityPolicy(sgp, vrf.Status.VrfID, &ruleIDAppLUT)
	if err != nil {
		log.Errorf("Error updating the SG Policy {%+v} in datapath. Err: %v", sgp.ObjectMeta, err)
		return err
	}

	log.Infof("Saving SG Policy: %v", sgp)
	err = na.saveNetworkSecurityPolicy(sgp)
	return err
}

// DeleteNetworkSecurityPolicy deletes a security group policy
func (na *Nagent) DeleteNetworkSecurityPolicy(tn, namespace, name string) error {
	sgp := &netproto.NetworkSecurityPolicy{
		TypeMeta: api.TypeMeta{Kind: "NetworkSecurityPolicy"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    tn,
			Namespace: namespace,
			Name:      name,
		},
	}
	err := na.validateMeta(sgp.Kind, sgp.ObjectMeta)
	if err != nil {
		return err
	}
	// find the corresponding namespace
	ns, err := na.FindNamespace(sgp.ObjectMeta)
	if err != nil {
		return err
	}

	existingNetworkSecurityPolicy, err := na.FindNetworkSecurityPolicy(sgp.ObjectMeta)
	if err != nil {
		log.Errorf("NetworkSecurityPolicy %+v not found", sgp.ObjectMeta)
		return ErrObjectNotFound
	}

	// find the corresponding vrf for the sg policy
	vrf, err := na.ValidateVrf(existingNetworkSecurityPolicy.Tenant, existingNetworkSecurityPolicy.Namespace, existingNetworkSecurityPolicy.Spec.VrfName)
	if err != nil {
		log.Errorf("Failed to find the vrf %v", existingNetworkSecurityPolicy.Spec.VrfName)
		return err
	}

	// check if the current sg policy has any objects referring to it
	err = na.Solver.Solve(existingNetworkSecurityPolicy)
	if err != nil {
		log.Errorf("Found active references to %v. Err: %v", existingNetworkSecurityPolicy.Name, err)
		return err
	}

	// delete it in the datapath
	err = na.Datapath.DeleteNetworkSecurityPolicy(existingNetworkSecurityPolicy, vrf.Status.VrfID)
	if err != nil {
		log.Errorf("Error deleting security group policy {%+v}. Err: %v", sgp, err)
		return err
	}

	if existingNetworkSecurityPolicy.Spec.AttachTenant {
		err = na.Solver.Remove(vrf, existingNetworkSecurityPolicy)
		if err != nil {
			log.Errorf("Could not remove the reference to the namespace: %v. Err: %v", ns.Name, err)
			return err
		}
	}

	err = na.Solver.Remove(vrf, existingNetworkSecurityPolicy)
	if err != nil {
		log.Errorf("Could not remove the reference to the vrf: %v. Err: %v", existingNetworkSecurityPolicy.Spec.VrfName, err)
		return err
	}

	err = na.Solver.Remove(ns, existingNetworkSecurityPolicy)
	if err != nil {
		log.Errorf("Could not remove the reference to the namespace: %v. Err: %v", existingNetworkSecurityPolicy.Namespace, err)
		return err
	}

	// delete from db
	err = na.discardNetworkSecurityPolicy(sgp)

	return err
}

//generateRuleHash generates rule hash for policy rule
func generateRuleHash(r *netproto.PolicyRule, key string) uint64 {
	h := fnv.New64()
	rule, _ := r.Marshal()
	rule = append(rule, []byte(key)...)
	h.Write(rule)
	return h.Sum64()
}
