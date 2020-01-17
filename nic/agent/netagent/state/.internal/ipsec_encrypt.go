// +build ignore

// {C} Copyright 2018 Pensando Systems Inc. All rights reserved.

package _internal

import (
	"errors"
	"fmt"

	"github.com/gogo/protobuf/proto"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/nic/agent/netagent/state/types"
	"github.com/pensando/sw/nic/agent/protos/netproto"
	"github.com/pensando/sw/venice/utils/log"
)

// CreateIPSecSAEncrypt creates an IPSec SA Encrypt rule
func (na *state.Nagent) CreateIPSecSAEncrypt(ipSecSAEncrypt *netproto.IPSecSAEncrypt) error {
	err := na.validateMeta(ipSecSAEncrypt.Kind, ipSecSAEncrypt.ObjectMeta)
	if err != nil {
		return err
	}
	oldEncryptSA, err := na.FindIPSecSAEncrypt(ipSecSAEncrypt.ObjectMeta)
	if err == nil {
		// check if the contents are same
		if !proto.Equal(oldEncryptSA, ipSecSAEncrypt) {
			log.Errorf("IPSec encrypt SA %+v already exists", oldEncryptSA)
			return errors.New("IPSec encrypt SA already exists")
		}

		log.Infof("Received duplicate IPSec encrypt SA create {%+v}", ipSecSAEncrypt.ObjectMeta)
		return nil
	}

	// find the corresponding namespace
	ns, err := na.FindNamespace(ipSecSAEncrypt.ObjectMeta)
	if err != nil {
		return err
	}

	vrf, err := na.ValidateVrf(ipSecSAEncrypt.Tenant, ipSecSAEncrypt.Namespace, ipSecSAEncrypt.Spec.VrfName)
	if err != nil {
		log.Errorf("Failed to find the vrf %v", ipSecSAEncrypt.Spec.VrfName)
		return err
	}

	// ensure that the tep-name resolves to a valid vrf.
	tepVrf, err := na.ValidateVrf(ipSecSAEncrypt.Tenant, ipSecSAEncrypt.Namespace, ipSecSAEncrypt.Spec.TepVrf)
	if err != nil {
		log.Errorf("Failed to find the TEP vrf %v", ipSecSAEncrypt.Spec.TepVrf)
		return err
	}

	// Only ESP Protocol supported for encrypt SA
	if ipSecSAEncrypt.Spec.Protocol != "ESP" {
		return fmt.Errorf("ipsec sa encrypt protocol should be ESP")
	}

	// Allocate ID only on first object creates and use existing ones during config replay
	if ipSecSAEncrypt.Status.IPSecSAEncryptID == 0 {
		ipSecSAEncrypt.Status.IPSecSAEncryptID, err = na.Store.GetNextID(types.IPSecSAEncryptID)
	}

	if err != nil {
		log.Errorf("Could not allocate IPSec encrypt SA id. {%+v}", err)
		return err
	}

	// create it in datapath
	err = na.Datapath.CreateIPSecSAEncrypt(ipSecSAEncrypt, vrf, tepVrf)
	if err != nil {
		log.Errorf("Error creating IPSec Encrypt rule in datapath. IPSecSAEncrypt {%+v}. Err: %v", ipSecSAEncrypt, err)
		return err
	}

	// Add the current ipSecSAEncrypt Rule as a dependency to the namespace.
	err = na.Solver.Add(ns, ipSecSAEncrypt)
	if err != nil {
		log.Errorf("Could not add dependency. Parent: %v. Child: %v", ns, ipSecSAEncrypt)
		return err
	}

	// Add the current ipSecSAEncrypt Rule as a dependency to the vrf.
	err = na.Solver.Add(vrf, ipSecSAEncrypt)
	if err != nil {
		log.Errorf("Could not add dependency. Parent: %v. Child: %v", vrf, ipSecSAEncrypt)
		return err
	}

	// Add the current ipSecSAEncrypt Rule as a dependency to the tep namespace as well.
	err = na.Solver.Add(tepVrf, ipSecSAEncrypt)
	if err != nil {
		log.Errorf("Could not add dependency. Parent: %v. Child: %v", tepVrf, ipSecSAEncrypt)
		return err
	}

	// save it in db
	key := na.Solver.ObjectKey(ipSecSAEncrypt.ObjectMeta, ipSecSAEncrypt.TypeMeta)
	na.Lock()
	na.IPSecSAEncryptDB[key] = ipSecSAEncrypt
	na.Unlock()
	err = na.Store.Write(ipSecSAEncrypt)

	return err
}

// FindIPSecSAEncrypt finds an IPSec SA Encrypt rule in local db
func (na *state.Nagent) FindIPSecSAEncrypt(meta api.ObjectMeta) (*netproto.IPSecSAEncrypt, error) {
	typeMeta := api.TypeMeta{
		Kind: "IPSecSAEncrypt",
	}
	// lock the db
	na.Lock()
	defer na.Unlock()

	// lookup the database
	key := na.Solver.ObjectKey(meta, typeMeta)
	ipSecEncryptSA, ok := na.IPSecSAEncryptDB[key]
	if !ok {
		return nil, fmt.Errorf("IPSec encrypt SA not found %v", meta.Name)
	}

	return ipSecEncryptSA, nil
}

// ListIPSecSAEncrypt returns the list of IPSec encrypt SA
func (na *state.Nagent) ListIPSecSAEncrypt() []*netproto.IPSecSAEncrypt {
	var ipSecEncryptSAList []*netproto.IPSecSAEncrypt
	// lock the db
	na.Lock()
	defer na.Unlock()

	for _, ipSecEncryptSA := range na.IPSecSAEncryptDB {
		ipSecEncryptSAList = append(ipSecEncryptSAList, ipSecEncryptSA)
	}

	return ipSecEncryptSAList
}

// UpdateIPSecSAEncrypt updates an IPSec encrypt SA
func (na *state.Nagent) UpdateIPSecSAEncrypt(ipSecEncryptSA *netproto.IPSecSAEncrypt) error {
	// find the corresponding namespace
	_, err := na.FindNamespace(ipSecEncryptSA.ObjectMeta)
	if err != nil {
		return err
	}
	existingIPSecSAEncrypt, err := na.FindIPSecSAEncrypt(ipSecEncryptSA.ObjectMeta)
	if err != nil {
		log.Errorf("IPSec encrypt SA %v not found", ipSecEncryptSA.ObjectMeta)
		return err
	}

	if proto.Equal(ipSecEncryptSA, existingIPSecSAEncrypt) {
		log.Infof("Nothing to update.")
		return nil
	}

	vrf, err := na.ValidateVrf(existingIPSecSAEncrypt.Tenant, existingIPSecSAEncrypt.Namespace, existingIPSecSAEncrypt.Spec.VrfName)
	if err != nil {
		log.Errorf("Failed to find the vrf %v", existingIPSecSAEncrypt.Spec.VrfName)
		return err
	}

	// ensure that the tep-name resolves to a valid vrf.
	tepVrf, err := na.ValidateVrf(existingIPSecSAEncrypt.Tenant, existingIPSecSAEncrypt.Namespace, existingIPSecSAEncrypt.Spec.TepVrf)
	if err != nil {
		log.Errorf("Failed to find the TEP vrf %v", existingIPSecSAEncrypt.Spec.TepVrf)
		return err
	}

	err = na.Datapath.UpdateIPSecSAEncrypt(ipSecEncryptSA, vrf, tepVrf)
	key := na.Solver.ObjectKey(ipSecEncryptSA.ObjectMeta, ipSecEncryptSA.TypeMeta)
	na.Lock()
	na.IPSecSAEncryptDB[key] = ipSecEncryptSA
	na.Unlock()
	err = na.Store.Write(ipSecEncryptSA)
	return err
}

// DeleteIPSecSAEncrypt deletes an IPSec encrypt SA
func (na *state.Nagent) DeleteIPSecSAEncrypt(tn, namespace, name string) error {
	ipSecEncryptSA := &netproto.IPSecSAEncrypt{
		TypeMeta: api.TypeMeta{Kind: "IPSecSAEncrypt"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    tn,
			Namespace: namespace,
			Name:      name,
		},
	}
	err := na.validateMeta(ipSecEncryptSA.Kind, ipSecEncryptSA.ObjectMeta)
	if err != nil {
		return err
	}

	existingIPSecSAEncrypt, err := na.FindIPSecSAEncrypt(ipSecEncryptSA.ObjectMeta)
	if err != nil {
		log.Errorf("IPSecEncrypt %+v not found", ipSecEncryptSA.ObjectMeta)
		return errors.New("IPSec encrypt SA not found")
	}

	// find the corresponding namespace
	ns, err := na.FindNamespace(existingIPSecSAEncrypt.ObjectMeta)
	if err != nil {
		return err
	}

	vrf, err := na.ValidateVrf(existingIPSecSAEncrypt.Tenant, existingIPSecSAEncrypt.Namespace, existingIPSecSAEncrypt.Spec.VrfName)
	if err != nil {
		log.Errorf("Failed to find the vrf %v", existingIPSecSAEncrypt.Spec.VrfName)
		return err
	}

	// ensure that the tep-name resolves to a valid vrf.
	tepVrf, err := na.ValidateVrf(existingIPSecSAEncrypt.Tenant, existingIPSecSAEncrypt.Namespace, existingIPSecSAEncrypt.Spec.TepVrf)
	if err != nil {
		log.Errorf("Failed to find the TEP vrf %v", existingIPSecSAEncrypt.Spec.TepVrf)
		return err
	}

	// check if the current ipsec sa encrypt rule has any objects referring to it
	err = na.Solver.Solve(existingIPSecSAEncrypt)
	if err != nil {
		log.Errorf("Found active references to %v. Err: %v", existingIPSecSAEncrypt.Name, err)
		return err
	}

	// delete it in the datapath
	err = na.Datapath.DeleteIPSecSAEncrypt(existingIPSecSAEncrypt, vrf)
	if err != nil {
		log.Errorf("Error deleting IPSec encrypt SA {%+v}. Err: %v", ipSecEncryptSA, err)
	}

	// update parent references
	err = na.Solver.Remove(tepVrf, existingIPSecSAEncrypt)
	if err != nil {
		log.Errorf("Could not remove the reference to the tepVrf: %v. Err: %v", tepVrf.Name, err)
		return err
	}

	err = na.Solver.Remove(vrf, existingIPSecSAEncrypt)
	if err != nil {
		log.Errorf("Could not remove the reference to the vrf: %v. Err: %v", vrf.Name, err)
		return err
	}

	err = na.Solver.Remove(ns, existingIPSecSAEncrypt)
	if err != nil {
		log.Errorf("Could not remove the reference to the namespace: %v. Err: %v", ns.Name, err)
		return err
	}

	// delete from db
	key := na.Solver.ObjectKey(ipSecEncryptSA.ObjectMeta, ipSecEncryptSA.TypeMeta)
	na.Lock()
	delete(na.IPSecSAEncryptDB, key)
	na.Unlock()
	err = na.Store.Delete(ipSecEncryptSA)

	return err
}
