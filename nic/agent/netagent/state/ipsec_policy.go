// {C} Copyright 2017 Pensando Systems Inc. All rights reserved.

package state

import (
	"errors"
	"fmt"
	"strings"

	"github.com/gogo/protobuf/proto"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/nic/agent/netagent/protos/netproto"
	"github.com/pensando/sw/nic/agent/netagent/state/types"
	"github.com/pensando/sw/venice/utils/log"
)

// CreateIPSecPolicy creates an IPSec Policy
func (na *Nagent) CreateIPSecPolicy(ipSec *netproto.IPSecPolicy) error {
	var dependentEncryptRules []*netproto.IPSecSAEncrypt
	var dependentDecryptRules []*netproto.IPSecSADecrypt
	protectCurrentNS := true
	err := na.validateMeta(ipSec.Kind, ipSec.ObjectMeta)
	if err != nil {
		return err
	}
	oldIPSec, err := na.FindIPSecPolicy(ipSec.ObjectMeta)
	if err == nil {
		// check if the contents are same
		if !proto.Equal(oldIPSec, ipSec) {
			log.Errorf("IPSecPolicy %+v already exists", oldIPSec)
			return errors.New("IPSec policy already exists")
		}

		log.Infof("Received duplicate IPSec policy create {%+v}", ipSec)
		return nil
	}

	// find the corresponding namespace
	ns, err := na.FindNamespace(ipSec.Tenant, ipSec.Namespace)
	if err != nil {
		return err
	}
	// validate SA Policies
	for _, r := range ipSec.Spec.Rules {
		r.ID, err = na.Store.GetNextID(types.IPSecRuleID)
		switch r.SAType {
		case "ENCRYPT":
			// SPI should not be specified for encrypt rules
			if r.SPI != 0 {
				log.Errorf("SPI is required for decrypt rules only")
				return errors.New("spi was set in encrypt rule")
			}
			// find the corresponding encrypt SA
			sa, err := na.findIPSecSAEncrypt(ipSec.ObjectMeta, r.SAName)
			if err != nil {
				log.Errorf("could not find SA Encrypt rule. Rule: {%v}. Err: %v", r, err)
				return err
			}
			key := fmt.Sprintf("%s|%s", r.SAType, r.SAName)
			saRef := &types.IPSecRuleRef{
				NamespaceID: ns.Status.NamespaceID,
				RuleID:      sa.Status.IPSecSAEncryptID,
			}
			na.IPSecPolicyLUT[key] = saRef
			dependentEncryptRules = append(dependentEncryptRules, sa)
			if sa.Namespace == ipSec.Namespace {
				protectCurrentNS = false
			}
		case "DECRYPT":
			sa, err := na.findIPSecSADecrypt(ipSec.ObjectMeta, r.SAName)
			if err != nil {
				log.Errorf("could not find SA Decrypt rule. Rule: {%v}. Err: %v", r, err)
				return err
			}
			key := fmt.Sprintf("%s|%s", r.SAType, r.SAName)
			saRef := &types.IPSecRuleRef{
				NamespaceID: ns.Status.NamespaceID,
				RuleID:      sa.Status.IPSecSADecryptID,
			}
			na.IPSecPolicyLUT[key] = saRef
			dependentDecryptRules = append(dependentDecryptRules, sa)
			if sa.Namespace == ipSec.Namespace {
				protectCurrentNS = false
			}
		default:
			log.Errorf("Invalid IPSec Policy rule type")
			return errors.New("invalid IPSec Policy rule type")
		}
	}

	ipSec.Status.IPSecPolicyID, err = na.Store.GetNextID(types.IPSecPolicyID)

	if err != nil {
		log.Errorf("Could not allocate IPSec policy id. {%+v}", err)
		return err
	}

	// create it in datapath
	err = na.Datapath.CreateIPSecPolicy(ipSec, ns, na.IPSecPolicyLUT)
	if err != nil {
		log.Errorf("Error creating ipsec policy in datapath. IPSecPolicy {%+v}. Err: %v", ipSec, err)
		return err
	}

	// Add the current policy as a dependency to all the rules
	for _, e := range dependentEncryptRules {
		err = na.Solver.Add(e, ipSec)
		if err != nil {
			log.Errorf("Could not add dependency. Parent: %v. Child: %v", e, ipSec)
			return err
		}
	}

	for _, d := range dependentDecryptRules {
		err = na.Solver.Add(d, ipSec)
		if err != nil {
			log.Errorf("Could not add dependency. Parent: %v. Child: %v", d, ipSec)
			return err
		}
	}

	// Check if we need to protect the current namespace from deletion. This is true if none of the dependent sa rules
	// refer to the namespace of the ipsec policy
	if protectCurrentNS {
		// Add the current Namespace as a dependency to the IPSec Policy.
		err = na.Solver.Add(ns, ipSec)
		if err != nil {
			log.Errorf("Could not add dependency. Parent: %v. Child: %v", ns, ipSec)
			return err
		}

	}

	// save it in db
	key := na.Solver.ObjectKey(ipSec.ObjectMeta, ipSec.TypeMeta)
	na.Lock()
	na.IPSecPolicyDB[key] = ipSec
	na.Unlock()
	err = na.Store.Write(ipSec)

	return err
}

// FindIPSecPolicy finds a nat policy in local db
func (na *Nagent) FindIPSecPolicy(meta api.ObjectMeta) (*netproto.IPSecPolicy, error) {
	typeMeta := api.TypeMeta{
		Kind: "IPSecPolicy",
	}
	// lock the db
	na.Lock()
	defer na.Unlock()

	// lookup the database
	key := na.Solver.ObjectKey(meta, typeMeta)
	ipSec, ok := na.IPSecPolicyDB[key]
	if !ok {
		return nil, fmt.Errorf("IPSec policy not found %v", meta.Name)
	}

	return ipSec, nil
}

// ListIPSecPolicy returns the list of IPSec policies
func (na *Nagent) ListIPSecPolicy() []*netproto.IPSecPolicy {
	var ipSecPolicyList []*netproto.IPSecPolicy
	// lock the db
	na.Lock()
	defer na.Unlock()

	for _, ipSec := range na.IPSecPolicyDB {
		ipSecPolicyList = append(ipSecPolicyList, ipSec)
	}

	return ipSecPolicyList
}

// UpdateIPSecPolicy updates an IPSec policy
func (na *Nagent) UpdateIPSecPolicy(ipSec *netproto.IPSecPolicy) error {
	// find the corresponding namespace
	ns, err := na.FindNamespace(ipSec.Tenant, ipSec.Namespace)
	if err != nil {
		return err
	}
	existingIPSec, err := na.FindIPSecPolicy(ipSec.ObjectMeta)
	if err != nil {
		log.Errorf("IPSecPolicy %v not found", ipSec.ObjectMeta)
		return err
	}

	if proto.Equal(ipSec, existingIPSec) {
		log.Infof("Nothing to update.")
		return nil
	}

	err = na.Datapath.UpdateIPSecPolicy(ipSec, ns)
	key := na.Solver.ObjectKey(ipSec.ObjectMeta, ipSec.TypeMeta)
	na.Lock()
	na.IPSecPolicyDB[key] = ipSec
	na.Unlock()
	err = na.Store.Write(ipSec)
	return err
}

// DeleteIPSecPolicy deletes an IPSec policy
func (na *Nagent) DeleteIPSecPolicy(tn, namespace, name string) error {
	ipSec := &netproto.IPSecPolicy{
		TypeMeta: api.TypeMeta{Kind: "IPSecPolicy"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    tn,
			Namespace: namespace,
			Name:      name,
		},
	}

	protectCurrentNS := true
	err := na.validateMeta(ipSec.Kind, ipSec.ObjectMeta)
	if err != nil {
		return err
	}
	// find the corresponding namespace
	ns, err := na.FindNamespace(ipSec.Tenant, ipSec.Namespace)
	if err != nil {
		return err
	}

	existingIPSec, err := na.FindIPSecPolicy(ipSec.ObjectMeta)
	if err != nil {
		log.Errorf("IPSecPolicy %+v not found", ipSec.ObjectMeta)
		return errors.New("IPSec policy not found")
	}

	// check if the current ipsec policy has any objects referring to it
	err = na.Solver.Solve(existingIPSec)
	if err != nil {
		log.Errorf("Found active references to %v. Err: %v", existingIPSec.Name, err)
		return err
	}

	// delete it in the datapath
	err = na.Datapath.DeleteIPSecPolicy(existingIPSec, ns)
	if err != nil {
		log.Errorf("Error deleting IPSec policy {%+v}. Err: %v", ipSec, err)
	}

	// Remove references for all the rules
	for _, r := range existingIPSec.Spec.Rules {
		switch r.SAType {
		case "ENCRYPT":
			// find the corresponding encrypt SA
			sa, err := na.findIPSecSAEncrypt(existingIPSec.ObjectMeta, r.SAName)
			if err != nil {
				log.Errorf("could not find SA Encrypt rule. Rule: {%v}. Err: %v", r, err)
				return err
			}
			err = na.Solver.Remove(sa, existingIPSec)
			if err != nil {
				log.Errorf("Could not remove the reference to the encrypt rule: %v. Err: %v", existingIPSec.Name, err)
				return err
			}
			if sa.Namespace == existingIPSec.Namespace {
				protectCurrentNS = false
			}
		case "DECRYPT":
			sa, err := na.findIPSecSADecrypt(existingIPSec.ObjectMeta, r.SAName)
			if err != nil {
				log.Errorf("could not find SA Decrypt rule. Rule: {%v}. Err: %v", r, err)
				return err
			}
			err = na.Solver.Remove(sa, existingIPSec)
			if err != nil {
				log.Errorf("Could not remove the reference to the decrypt rule: %v. Err: %v", existingIPSec.Name, err)
				return err
			}
			if sa.Namespace == existingIPSec.Namespace {
				protectCurrentNS = false
			}

		default:
			log.Errorf("Invalid IPSec Policy rule type")
			return errors.New("invalid IPSec Policy rule type")
		}
	}

	// protectCurrentNS is true if we have added a dependency to the current namespace of the ipsec policy.
	// In such a case we should remove it during deletion
	if protectCurrentNS {
		err = na.Solver.Remove(ns, existingIPSec)
		if err != nil {
			log.Errorf("Could not remove the reference to the namespace: %v. Err: %v", ns.Name, existingIPSec)
			return err
		}
	}

	// delete from db
	key := na.Solver.ObjectKey(ipSec.ObjectMeta, ipSec.TypeMeta)
	na.Lock()
	delete(na.IPSecPolicyDB, key)
	na.Unlock()
	err = na.Store.Delete(ipSec)

	return err
}

func (na *Nagent) findIPSecSAEncrypt(policyMeta api.ObjectMeta, saName string) (*netproto.IPSecSAEncrypt, error) {
	var saMeta api.ObjectMeta

	sa := strings.Split(saName, "/")
	switch len(sa) {
	// SA in local namespace
	case 1:
		saMeta.Tenant = policyMeta.Tenant
		saMeta.Namespace = policyMeta.Namespace
		saMeta.Name = saName
		return na.FindIPSecSAEncrypt(saMeta)
	// SA in remote namespace
	case 2:
		saMeta.Tenant = policyMeta.Tenant
		saMeta.Namespace = sa[0]
		saMeta.Name = sa[1]
		return na.FindIPSecSAEncrypt(saMeta)
	default:
		return nil, fmt.Errorf("ipsec SA encrypt {%v} not found", saName)
	}

}

func (na *Nagent) findIPSecSADecrypt(policyMeta api.ObjectMeta, saName string) (*netproto.IPSecSADecrypt, error) {
	var saMeta api.ObjectMeta

	sa := strings.Split(saName, "/")
	switch len(sa) {
	// SA in local namespace
	case 1:
		saMeta.Tenant = policyMeta.Tenant
		saMeta.Namespace = policyMeta.Namespace
		saMeta.Name = saName
		return na.FindIPSecSADecrypt(saMeta)
	// SA in remote namespace
	case 2:
		saMeta.Tenant = policyMeta.Tenant
		saMeta.Namespace = sa[0]
		saMeta.Name = sa[1]
		return na.FindIPSecSADecrypt(saMeta)
	default:
		return nil, fmt.Errorf("ipsec SA decrypt {%v} not found", saName)
	}

}
