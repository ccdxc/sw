// {C} Copyright 2017 Pensando Systems Inc. All rights reserved.

package state

import (
	"errors"
	"fmt"

	"github.com/gogo/protobuf/proto"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/nic/agent/protos/netproto"
	"github.com/pensando/sw/venice/utils/log"
)

// CreateIPAMPolicy creates an IPAM policy
func (na *Nagent) CreateIPAMPolicy(policy *netproto.IPAMPolicy) error {
	log.Infof("CreateIPAMPolicy: %v", policy)
	err := na.saveIPAMPolicy(policy)

	if err != nil {
		log.Error("CreateIPAMPolicy: saveIPAMPolicy failed, err: ", err)
		return err
	}
	return nil
}

// ListIPAMPolicy returns the list of IPAM policies
func (na *Nagent) ListIPAMPolicy() []*netproto.IPAMPolicy {
	log.Infof("ListIPAMPolicy")
	var policyList []*netproto.IPAMPolicy

	// lock the db
	na.Lock()
	defer na.Unlock()

	// walk all IPAM polocies
	for _, pol := range na.IPAMPolicyDB {
		policyList = append(policyList, pol)
	}

	return policyList
}

// FindIPAMPolicy finds an IPAM policy in local db
func (na *Nagent) FindIPAMPolicy(meta api.ObjectMeta) (*netproto.IPAMPolicy, error) {
	log.Infof("FindIPAMPolicy: %+v", meta)
	typeMeta := api.TypeMeta{
		Kind: "IPAMPolicy",
	}
	// lock the db
	na.Lock()
	defer na.Unlock()

	// lookup the database
	key := na.Solver.ObjectKey(meta, typeMeta)
	policy, ok := na.IPAMPolicyDB[key]
	if !ok {
		return nil, fmt.Errorf("IPAMPolicy not found %v", meta.Name)
	}

	return policy, nil
}

// UpdateIPAMPolicy updates an IPAM Policy
func (na *Nagent) UpdateIPAMPolicy(policy *netproto.IPAMPolicy) error {
	log.Infof("UpdateIPAMPolicy: %+v", policy)

	exisitingPolicy, err := na.FindIPAMPolicy(policy.ObjectMeta)
	if err != nil {
		log.Errorf("IPAMPolicy %v not found", policy.ObjectMeta)
		return err
	}

	if proto.Equal(&policy.Spec, &exisitingPolicy.Spec) {
		return nil
	}

	return na.saveIPAMPolicy(policy)
}

// DeleteIPAMPolicy deletes an IPAM Policy
func (na *Nagent) DeleteIPAMPolicy(tn, namespace, name string) error {
	log.Infof("DeleteIPAMPolicy: %s, %s, %s", tn, namespace, name)
	policy := &netproto.IPAMPolicy{
		TypeMeta: api.TypeMeta{Kind: "IPAMPolicy"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    tn,
			Namespace: namespace,
			Name:      name,
		},
	}
	err := na.validateMeta(policy.Kind, policy.ObjectMeta)
	if err != nil {
		log.Error("validateMeta failed: ", policy.ObjectMeta)
		return errors.New("Invalid ObjectMeta")
	}

	// check if the policy exists
	_, err = na.FindIPAMPolicy(policy.ObjectMeta)
	if err != nil {
		log.Errorf("IPAMPolicy %+v not found", policy.ObjectMeta)
		return errors.New("IPAMPolicy not found")
	}

	// delete from db
	return na.discardIPAMPolicy(policy)
}

// saveIPAMPolicy saves the IPAM policy in state stores
func (na *Nagent) saveIPAMPolicy(policy *netproto.IPAMPolicy) error {
	// save it in db
	key := na.Solver.ObjectKey(policy.ObjectMeta, policy.TypeMeta)
	na.Lock()
	na.IPAMPolicyDB[key] = policy
	na.Unlock()

	// write to emstore
	dat, _ := policy.Marshal()
	return na.Store.RawWrite(policy.GetKind(), policy.GetKey(), dat)
}

// discardIPAMPolicy removes the IPAM policy from state stores
func (na *Nagent) discardIPAMPolicy(policy *netproto.IPAMPolicy) error {
	// delete from db
	key := na.Solver.ObjectKey(policy.ObjectMeta, policy.TypeMeta)
	na.Lock()
	delete(na.IPAMPolicyDB, key)
	na.Unlock()

	// delete from emstore
	return na.Store.RawDelete(policy.GetKind(), policy.GetKey())
}
