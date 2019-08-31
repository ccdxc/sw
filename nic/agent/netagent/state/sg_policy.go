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

// ErrSGPolicyNotFound is returned when sgpolicy is not found
var ErrSGPolicyNotFound = errors.New("sgpolicy not found")

// CreateSGPolicy creates a security group policy
func (na *Nagent) CreateSGPolicy(sgp *netproto.SGPolicy) error {
	var securityGroups []*netproto.SecurityGroup
	var ruleIDAppLUT sync.Map
	err := na.validateMeta(sgp.Kind, sgp.ObjectMeta)
	if err != nil {
		return err
	}

	// Check  for max allowed SGPolicies
	if len(na.ListSGPolicy()) != 0 {
		log.Errorf("Exceeds maximum allowed Network Security Policy of %d", globals.MaxAllowedSGPolicies)
		return fmt.Errorf("exceeds maximum allowed Network Security Policy of %d", globals.MaxAllowedSGPolicies)
	}

	oldSgp, err := na.FindSGPolicy(sgp.ObjectMeta)
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

	for _, grp := range sgp.Spec.AttachGroup {
		sgMeta := api.ObjectMeta{
			Tenant:    sgp.Tenant,
			Namespace: sgp.Namespace,
			Name:      grp,
		}
		sg, err := na.FindSecurityGroup(sgMeta)
		if err != nil {
			log.Errorf("Could not find the security group to attach the sg policy")
			return err
		}
		securityGroups = append(securityGroups, sg)
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

			if app != nil && (len(r.Dst.AppConfigs) != 0 || len(r.Src.AppConfigs) != 0) {
				log.Errorf("cannot specify app-configs and an app name in the same rule. AppName: %v. SrcAppConfigs: %v. DstAppConfigs: %v", r.AppName, r.Src.AppConfigs, r.Dst.AppConfigs)
				return fmt.Errorf("cannot specify app-configs and an app name in the same rule. AppName: %v. SrcAppConfigs: %v. DstAppConfigs: %v", r.AppName, r.Src.AppConfigs, r.Dst.AppConfigs)

			}

			ruleIDAppLUT.Store(i, app)
		}
	}

	// Allocate ID only on first object creates and use existing ones during config replay
	if sgp.Status.SGPolicyID == 0 {
		sgp.Status.SGPolicyID, err = na.Store.GetNextID(types.SGPolicyID)
	}

	if err != nil {
		log.Errorf("Could not allocate security group policy id. {%+v}", err)
		return err
	}

	// create it in datapath
	err = na.Datapath.CreateSGPolicy(sgp, vrf.Status.VrfID, securityGroups, &ruleIDAppLUT)
	if err != nil {
		log.Errorf("Error creating security group policy in datapath. SGPolicy {%+v}. Err: %v", sgp, err)
		return err
	}

	// Add the current sg policy as a dependency to the namespace.
	err = na.Solver.Add(ns, sgp)
	if err != nil {
		log.Errorf("Could not add dependency. Parent: %v. Child: %v", ns, sgp)
		return err
	}

	// Add dependencies depending on the attachment points
	if len(sgp.Spec.AttachGroup) > 0 {
		for _, sg := range securityGroups {
			err = na.Solver.Add(sg, sgp)
			if err != nil {
				log.Errorf("Could not add dependency. Parent: %v. Child: %v", sg, sgp)
				return err
			}
		}
	} else if sgp.Spec.AttachTenant {
		err = na.Solver.Add(vrf, sgp)
		if err != nil {
			log.Errorf("Could not add dependency. Parent: %v. Child: %v", vrf, sgp)
			return err
		}
	}

	// save it in db
	err = na.saveSGPolicy(sgp)

	return err
}

func (na *Nagent) saveSGPolicy(sgp *netproto.SGPolicy) error {
	// save it in db
	key := na.Solver.ObjectKey(sgp.ObjectMeta, sgp.TypeMeta)
	na.Lock()
	na.SGPolicyDB[key] = sgp
	na.Unlock()

	// write to delphi
	if na.DelphiClient != nil {
		dsgp := dnetproto.SGPolicy{
			Key:      key,
			SGPolicy: sgp,
		}

		err := na.DelphiClient.SetObject(&dsgp)
		if err != nil {
			log.Errorf("Error writing SGPolicy %s to delphi. Err: %v", key, err)
			return err
		}
	}

	dat, _ := sgp.Marshal()
	err := na.Store.RawWrite(sgp.GetKind(), sgp.GetKey(), dat)

	return err
}

func (na *Nagent) discardSGPolicy(sgp *netproto.SGPolicy) error {
	// delete from db
	key := na.Solver.ObjectKey(sgp.ObjectMeta, sgp.TypeMeta)
	na.Lock()
	delete(na.SGPolicyDB, key)
	na.Unlock()

	// remove it from delphi
	if na.DelphiClient != nil {
		dsgp := dnetproto.SGPolicy{
			Key:      key,
			SGPolicy: sgp,
		}

		err := na.DelphiClient.DeleteObject(&dsgp)
		if err != nil {
			log.Errorf("Error Deleting SGPolicy %s from delphi. Err: %v", key, err)
			return err
		}
	}

	return na.Store.RawDelete(sgp.GetKind(), sgp.GetKey())
}

// FindSGPolicy finds a security group policy in local db
func (na *Nagent) FindSGPolicy(meta api.ObjectMeta) (*netproto.SGPolicy, error) {
	typeMeta := api.TypeMeta{
		Kind: "SGPolicy",
	}
	// lock the db
	na.Lock()
	defer na.Unlock()

	// lookup the database
	key := na.Solver.ObjectKey(meta, typeMeta)
	sgp, ok := na.SGPolicyDB[key]
	if !ok {
		return nil, fmt.Errorf("security group policy not found %v", meta.Name)
	}

	return sgp, nil
}

// ListSGPolicy returns the list of security group polices
func (na *Nagent) ListSGPolicy() []*netproto.SGPolicy {
	var sgPolicyList []*netproto.SGPolicy
	// lock the db
	na.Lock()
	defer na.Unlock()

	for _, sgp := range na.SGPolicyDB {
		sgPolicyList = append(sgPolicyList, sgp)
	}

	return sgPolicyList
}

// UpdateSGPolicy updates a security group policy
func (na *Nagent) UpdateSGPolicy(sgp *netproto.SGPolicy) error {
	// find the corresponding namespace
	_, err := na.FindNamespace(sgp.ObjectMeta)
	if err != nil {
		return err
	}
	existingSgp, err := na.FindSGPolicy(sgp.ObjectMeta)
	if err != nil {
		log.Errorf("SGPolicy %v not found", sgp.ObjectMeta)
		return err
	}

	// check if policy contents are same
	if proto.Equal(&existingSgp.Spec, &sgp.Spec) {
		return nil
	}

	// Populate the ID from existing sg policy to ensure that HAL recognizes this.
	sgp.Status.SGPolicyID = existingSgp.Status.SGPolicyID

	return na.performSGPolicyUpdate(sgp)
}

// performSGPolicyUpdate
func (na *Nagent) performSGPolicyUpdate(sgp *netproto.SGPolicy) error {
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

	err = na.Datapath.UpdateSGPolicy(sgp, vrf.Status.VrfID, &ruleIDAppLUT)
	if err != nil {
		log.Errorf("Error updating the SG Policy {%+v} in datapath. Err: %v", sgp.ObjectMeta, err)
		return err
	}

	err = na.saveSGPolicy(sgp)
	return err
}

// DeleteSGPolicy deletes a security group policy
func (na *Nagent) DeleteSGPolicy(tn, namespace, name string) error {
	sgp := &netproto.SGPolicy{
		TypeMeta: api.TypeMeta{Kind: "SGPolicy"},
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

	existingSGPolicy, err := na.FindSGPolicy(sgp.ObjectMeta)
	if err != nil {
		log.Errorf("SGPolicy %+v not found", sgp.ObjectMeta)
		return ErrSGPolicyNotFound
	}

	// find the corresponding vrf for the sg policy
	vrf, err := na.ValidateVrf(existingSGPolicy.Tenant, existingSGPolicy.Namespace, existingSGPolicy.Spec.VrfName)
	if err != nil {
		log.Errorf("Failed to find the vrf %v", existingSGPolicy.Spec.VrfName)
		return err
	}

	// check if the current sg policy has any objects referring to it
	err = na.Solver.Solve(existingSGPolicy)
	if err != nil {
		log.Errorf("Found active references to %v. Err: %v", existingSGPolicy.Name, err)
		return err
	}

	// delete it in the datapath
	err = na.Datapath.DeleteSGPolicy(existingSGPolicy, vrf.Status.VrfID)
	if err != nil {
		log.Errorf("Error deleting security group policy {%+v}. Err: %v", sgp, err)
		return err
	}

	// Update parent references
	for _, s := range existingSGPolicy.Spec.AttachGroup {
		sgMeta := api.ObjectMeta{
			Tenant:    sgp.Tenant,
			Namespace: sgp.Namespace,
			Name:      s,
		}
		sg, err := na.FindSecurityGroup(sgMeta)
		if err != nil {
			log.Errorf("Could not find the security group to attach the sg policy")
			return err
		}
		err = na.Solver.Remove(sg, existingSGPolicy)
		if err != nil {
			log.Errorf("Could not remove the reference to the security group: %v. Err: %v", sg.Name, err)
			return err
		}
	}

	if existingSGPolicy.Spec.AttachTenant {
		err = na.Solver.Remove(vrf, existingSGPolicy)
		if err != nil {
			log.Errorf("Could not remove the reference to the namespace: %v. Err: %v", ns.Name, err)
			return err
		}
	}

	err = na.Solver.Remove(vrf, existingSGPolicy)
	if err != nil {
		log.Errorf("Could not remove the reference to the vrf: %v. Err: %v", existingSGPolicy.Spec.VrfName, err)
		return err
	}

	err = na.Solver.Remove(ns, existingSGPolicy)
	if err != nil {
		log.Errorf("Could not remove the reference to the namespace: %v. Err: %v", existingSGPolicy.Namespace, err)
		return err
	}

	// delete from db
	err = na.discardSGPolicy(sgp)

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
