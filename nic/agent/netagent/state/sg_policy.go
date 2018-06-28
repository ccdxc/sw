// {C} Copyright 2017 Pensando Systems Inc. All rights reserved.

package state

import (
	"errors"
	"fmt"

	"github.com/gogo/protobuf/proto"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/nic/agent/netagent/state/types"
	"github.com/pensando/sw/venice/ctrler/npm/rpcserver/netproto"
	"github.com/pensando/sw/venice/utils/log"
)

// CreateSGPolicy creates a security group policy
func (na *Nagent) CreateSGPolicy(sgp *netproto.SGPolicy) error {
	var securityGroups []*netproto.SecurityGroup
	err := na.validateMeta(sgp.Kind, sgp.ObjectMeta)
	if err != nil {
		return err
	}
	oldSgp, err := na.FindSGPolicy(sgp.ObjectMeta)
	if err == nil {
		// check if the contents are same
		if !proto.Equal(oldSgp, sgp) {
			log.Errorf("SGPolicy %+v already exists", oldSgp)
			return errors.New("security group policy already exists")
		}

		log.Infof("Received duplicate security group policy create {%+v}", sgp)
		return nil
	}

	// find the corresponding namespace
	ns, err := na.FindNamespace(sgp.Tenant, sgp.Namespace)
	if err != nil {
		return err
	}

	// validate security group policy message
	if sgp.Spec.AttachTenant == false && len(sgp.Spec.AttachGroup) == 0 || sgp.Spec.AttachTenant == true && len(sgp.Spec.AttachGroup) != 0 {
		log.Errorf("Must specify one attachment point. Either attach-tenant or attach-group")
		return fmt.Errorf("invalid attachment point for %s. Must specify one attachment point. Either attach-tenant or attach-group", sgp.Name)
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

	sgp.Status.SGPolicyID, err = na.Store.GetNextID(types.SGPolicyID)

	if err != nil {
		log.Errorf("Could not allocate security group policy id. {%+v}", err)
		return err
	}

	// create it in datapath
	err = na.Datapath.CreateSGPolicy(sgp, ns.Status.NamespaceID, securityGroups)
	if err != nil {
		log.Errorf("Error creating security group policy in datapath. SGPolicy {%+v}. Err: %v", sgp, err)
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
		err = na.Solver.Add(ns, sgp)
		if err != nil {
			log.Errorf("Could not add dependency. Parent: %v. Child: %v", ns, sgp)
			return err
		}
	}

	// save it in db
	key := na.Solver.ObjectKey(sgp.ObjectMeta, sgp.TypeMeta)
	na.Lock()
	na.SGPolicyDB[key] = sgp
	na.Unlock()
	err = na.Store.Write(sgp)

	return err
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
	ns, err := na.FindNamespace(sgp.Tenant, sgp.Namespace)
	if err != nil {
		return err
	}
	existingSgp, err := na.FindSGPolicy(sgp.ObjectMeta)
	if err != nil {
		log.Errorf("SGPolicy %v not found", sgp.ObjectMeta)
		return err
	}

	if proto.Equal(sgp, existingSgp) {
		log.Infof("Nothing to update.")
		return nil
	}

	err = na.Datapath.UpdateSGPolicy(sgp, ns.Status.NamespaceID)
	key := na.Solver.ObjectKey(sgp.ObjectMeta, sgp.TypeMeta)
	na.Lock()
	na.SGPolicyDB[key] = sgp
	na.Unlock()
	err = na.Store.Write(sgp)
	return err
}

// DeleteSGPolicy deletes a security group policy
func (na *Nagent) DeleteSGPolicy(sgp *netproto.SGPolicy) error {
	err := na.validateMeta(sgp.Kind, sgp.ObjectMeta)
	if err != nil {
		return err
	}
	// find the corresponding namespace
	ns, err := na.FindNamespace(sgp.Tenant, sgp.Namespace)
	if err != nil {
		return err
	}

	existingSGPolicy, err := na.FindSGPolicy(sgp.ObjectMeta)
	if err != nil {
		log.Errorf("SGPolicy %+v not found", sgp.ObjectMeta)
		return errors.New("security group policy not found")
	}

	// check if the current sg policy has any objects referring to it
	err = na.Solver.Solve(existingSGPolicy)
	if err != nil {
		log.Errorf("Found active references to %v. Err: %v", existingSGPolicy.Name, err)
		return err
	}

	// delete it in the datapath
	err = na.Datapath.DeleteSGPolicy(existingSGPolicy, ns.Status.NamespaceID)
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
		err = na.Solver.Remove(ns, existingSGPolicy)
		if err != nil {
			log.Errorf("Could not remove the reference to the namespace: %v. Err: %v", ns.Name, err)
			return err
		}
	}

	// delete from db
	key := na.Solver.ObjectKey(sgp.ObjectMeta, sgp.TypeMeta)
	na.Lock()
	delete(na.SGPolicyDB, key)
	na.Unlock()
	err = na.Store.Delete(sgp)

	return err
}
