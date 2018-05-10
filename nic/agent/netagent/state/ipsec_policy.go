// {C} Copyright 2017 Pensando Systems Inc. All rights reserved.

package state

import (
	"errors"

	"github.com/gogo/protobuf/proto"

	"fmt"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/venice/ctrler/npm/rpcserver/netproto"
	"github.com/pensando/sw/venice/utils/log"
)

// CreateIPSecPolicy creates an IPSec Policy
func (na *NetAgent) CreateIPSecPolicy(ipSec *netproto.IPSecPolicy) error {
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
		r.ID, err = na.store.GetNextID(IPSecRuleID)
		oMeta := api.ObjectMeta{
			Tenant:    ipSec.Tenant,
			Namespace: ipSec.Namespace,
			Name:      r.SAName,
		}
		switch r.SAType {
		case "ENCRYPT":
			sa, err := na.FindIPSecSAEncrypt(oMeta)
			if err != nil {
				log.Errorf("could not find SA Encrypt rule. Rule: {%v}. Err: %v", r, err)
				return err
			}
			key := fmt.Sprintf("ENCRYPT|%s", sa.Name)
			saRef := &IPSecRuleRef{
				NamespaceID: ns.Status.NamespaceID,
				RuleID:      sa.Status.IPSecSAEncryptID,
			}
			na.ipSecPolicyLUT[key] = saRef
		case "DECRYPT":
			sa, err := na.FindIPSecSADecrypt(oMeta)
			if err != nil {
				log.Errorf("could not find SA Decrypt rule. Rule: {%v}. Err: %v", r, err)
				return err
			}
			key := fmt.Sprintf("DECRYPT|%s", sa.Name)
			saRef := &IPSecRuleRef{
				NamespaceID: ns.Status.NamespaceID,
				RuleID:      sa.Status.IPSecSADecryptID,
			}
			na.ipSecPolicyLUT[key] = saRef
		default:
			log.Errorf("Invalid IPSec Policy rule type")
			return errors.New("invalid IPSec Policy rule type")
		}
	}

	ipSec.Status.IPSecPolicyID, err = na.store.GetNextID(IPSecPolicyID)

	if err != nil {
		log.Errorf("Could not allocate IPSec policy id. {%+v}", err)
		return err
	}

	// create it in datapath
	err = na.datapath.CreateIPSecPolicy(ipSec, ns, na.ipSecPolicyLUT)
	if err != nil {
		log.Errorf("Error creating ipsec policy in datapath. IPSecPolicy {%+v}. Err: %v", ipSec, err)
		return err
	}

	// save it in db
	key := objectKey(ipSec.ObjectMeta, ipSec.TypeMeta)
	na.Lock()
	na.ipSecPolicyDB[key] = ipSec
	na.Unlock()
	err = na.store.Write(ipSec)

	return err
}

// FindIPSecPolicy finds a nat policy in local db
func (na *NetAgent) FindIPSecPolicy(meta api.ObjectMeta) (*netproto.IPSecPolicy, error) {
	typeMeta := api.TypeMeta{
		Kind: "IPSecPolicy",
	}
	// lock the db
	na.Lock()
	defer na.Unlock()

	// lookup the database
	key := objectKey(meta, typeMeta)
	ipSec, ok := na.ipSecPolicyDB[key]
	if !ok {
		return nil, fmt.Errorf("IPSec policy not found %v", ipSec)
	}

	return ipSec, nil
}

// ListIPSecPolicy returns the list of IPSec policies
func (na *NetAgent) ListIPSecPolicy() []*netproto.IPSecPolicy {
	var ipSecPolicyList []*netproto.IPSecPolicy
	// lock the db
	na.Lock()
	defer na.Unlock()

	for _, ipSec := range na.ipSecPolicyDB {
		ipSecPolicyList = append(ipSecPolicyList, ipSec)
	}

	return ipSecPolicyList
}

// UpdateIPSecPolicy updates an IPSec policy
func (na *NetAgent) UpdateIPSecPolicy(ipSec *netproto.IPSecPolicy) error {
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

	err = na.datapath.UpdateIPSecPolicy(ipSec, ns)
	key := objectKey(ipSec.ObjectMeta, ipSec.TypeMeta)
	na.Lock()
	na.ipSecPolicyDB[key] = ipSec
	na.Unlock()
	err = na.store.Write(ipSec)
	return err
}

// DeleteIPSecPolicy deletes an IPSec policy
func (na *NetAgent) DeleteIPSecPolicy(ipSec *netproto.IPSecPolicy) error {
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

	// delete it in the datapath
	err = na.datapath.DeleteIPSecPolicy(existingIPSec, ns)
	if err != nil {
		log.Errorf("Error deleting IPSec policy {%+v}. Err: %v", ipSec, err)
	}

	// delete from db
	key := objectKey(ipSec.ObjectMeta, ipSec.TypeMeta)
	na.Lock()
	delete(na.ipSecPolicyDB, key)
	na.Unlock()
	err = na.store.Delete(ipSec)

	return err
}
