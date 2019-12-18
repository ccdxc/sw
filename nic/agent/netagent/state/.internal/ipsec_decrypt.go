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

// CreateIPSecSADecrypt creates an IPSec SA Decrypt rule
func (na *state.Nagent) CreateIPSecSADecrypt(ipSecSADecrypt *netproto.IPSecSADecrypt) error {
	err := na.validateMeta(ipSecSADecrypt.Kind, ipSecSADecrypt.ObjectMeta)
	if err != nil {
		return err
	}
	oldDecryptSA, err := na.FindIPSecSADecrypt(ipSecSADecrypt.ObjectMeta)
	if err == nil {
		// check if the contents are same
		if !proto.Equal(oldDecryptSA, ipSecSADecrypt) {
			log.Errorf("IPSec decrypt SA %+v already exists", oldDecryptSA)
			return errors.New("IPSec decrypt SA already exists")
		}

		log.Infof("Received duplicate IPSec decrypt SA create {%+v}", ipSecSADecrypt.ObjectMeta)
		return nil
	}

	// find the corresponding namespace
	ns, err := na.FindNamespace(ipSecSADecrypt.ObjectMeta)
	if err != nil {
		return err
	}

	// validate that the spec has a tep name
	if len(ipSecSADecrypt.Spec.TepVrf) == 0 {
		return fmt.Errorf("IPSecSAEncrypt needs to specify a tep-vrf")
	}

	vrf, err := na.ValidateVrf(ipSecSADecrypt.Tenant, ipSecSADecrypt.Namespace, ipSecSADecrypt.Spec.VrfName)
	if err != nil {
		log.Errorf("Failed to find the vrf %v", ipSecSADecrypt.Spec.VrfName)
		return err
	}

	// ensure that the tep-name resolves to a valid vrf.
	tepVrf, err := na.ValidateVrf(ipSecSADecrypt.Tenant, ipSecSADecrypt.Namespace, ipSecSADecrypt.Spec.TepVrf)
	if err != nil {
		log.Errorf("Failed to find the TEP vrf %v", ipSecSADecrypt.Spec.TepVrf)
		return err
	}

	// Only ESP Protocol supported for encrypt SA
	if ipSecSADecrypt.Spec.Protocol != "ESP" {
		return fmt.Errorf("ipsec sa decrypt protocol should be ESP")
	}

	// Allocate ID only on first object creates and use existing ones during config replay
	if ipSecSADecrypt.Status.IPSecSADecryptID == 0 {
		ipSecSADecrypt.Status.IPSecSADecryptID, err = na.Store.GetNextID(types.IPSecSADecryptID)
	}

	if err != nil {
		log.Errorf("Could not allocate IPSec decrypt SA id. {%+v}", err)
		return err
	}

	// create it in datapath
	err = na.Datapath.CreateIPSecSADecrypt(ipSecSADecrypt, vrf, tepVrf)
	if err != nil {
		log.Errorf("Error creating IPSec Decrypt rule in datapath. IPSecSADecrypt {%+v}. Err: %v", ipSecSADecrypt, err)
		return err
	}

	// Add the current ipsecDecryptSA Rule as a dependency to the namespace.
	err = na.Solver.Add(ns, ipSecSADecrypt)
	if err != nil {
		log.Errorf("Could not add dependency. Parent: %v. Child: %v", ns, ipSecSADecrypt)
		return err
	}

	// Add the current ipsecDecryptSA Rule as a dependency to the vrf.
	err = na.Solver.Add(vrf, ipSecSADecrypt)
	if err != nil {
		log.Errorf("Could not add dependency. Parent: %v. Child: %v", vrf, ipSecSADecrypt)
		return err
	}

	// Add the current ipsecDecryptSA Rule as a dependency to the tep namespace as well.
	err = na.Solver.Add(tepVrf, ipSecSADecrypt)
	if err != nil {
		log.Errorf("Could not add dependency. Parent: %v. Child: %v", tepVrf, ipSecSADecrypt)
		return err
	}

	// save it in db
	key := na.Solver.ObjectKey(ipSecSADecrypt.ObjectMeta, ipSecSADecrypt.TypeMeta)
	na.Lock()
	na.IPSecSADecryptDB[key] = ipSecSADecrypt
	na.Unlock()
	err = na.Store.Write(ipSecSADecrypt)

	return err
}

// FindIPSecSADecrypt finds an IPSec SA Decrypt rule in local db
func (na *state.Nagent) FindIPSecSADecrypt(meta api.ObjectMeta) (*netproto.IPSecSADecrypt, error) {
	typeMeta := api.TypeMeta{
		Kind: "IPSecSADecrypt",
	}
	// lock the db
	na.Lock()
	defer na.Unlock()

	// lookup the database
	key := na.Solver.ObjectKey(meta, typeMeta)
	ipSecDecryptSA, ok := na.IPSecSADecryptDB[key]
	if !ok {
		return nil, fmt.Errorf("IPSec decrypt SA not found %v", meta.Name)
	}

	return ipSecDecryptSA, nil
}

// ListIPSecSADecrypt returns the list of IPSec decrypt SA
func (na *state.Nagent) ListIPSecSADecrypt() []*netproto.IPSecSADecrypt {
	var ipSecDecryptSAList []*netproto.IPSecSADecrypt
	// lock the db
	na.Lock()
	defer na.Unlock()

	for _, ipSecDecryptSA := range na.IPSecSADecryptDB {
		ipSecDecryptSAList = append(ipSecDecryptSAList, ipSecDecryptSA)
	}

	return ipSecDecryptSAList
}

// UpdateIPSecSADecrypt updates an IPSec decrypt SA
func (na *state.Nagent) UpdateIPSecSADecrypt(ipSecDecryptSA *netproto.IPSecSADecrypt) error {
	// find the corresponding namespace
	_, err := na.FindNamespace(ipSecDecryptSA.ObjectMeta)
	if err != nil {
		return err
	}
	existingIPSecSADecrypt, err := na.FindIPSecSADecrypt(ipSecDecryptSA.ObjectMeta)
	if err != nil {
		log.Errorf("IPSec decrypt SA %v not found", ipSecDecryptSA.ObjectMeta)
		return err
	}

	if proto.Equal(ipSecDecryptSA, existingIPSecSADecrypt) {
		log.Infof("Nothing to update.")
		return nil
	}

	vrf, err := na.ValidateVrf(ipSecDecryptSA.Tenant, ipSecDecryptSA.Namespace, ipSecDecryptSA.Spec.VrfName)
	if err != nil {
		log.Errorf("Failed to find the vrf %v", ipSecDecryptSA.Spec.VrfName)
		return err
	}

	// ensure that the tep-name resolves to a valid vrf.
	tepVrf, err := na.ValidateVrf(ipSecDecryptSA.Tenant, ipSecDecryptSA.Namespace, ipSecDecryptSA.Spec.TepVrf)
	if err != nil {
		log.Errorf("Failed to find the TEP vrf %v", ipSecDecryptSA.Spec.TepVrf)
		return err
	}

	err = na.Datapath.UpdateIPSecSADecrypt(ipSecDecryptSA, vrf, tepVrf)
	key := na.Solver.ObjectKey(ipSecDecryptSA.ObjectMeta, ipSecDecryptSA.TypeMeta)
	na.Lock()
	na.IPSecSADecryptDB[key] = ipSecDecryptSA
	na.Unlock()
	err = na.Store.Write(ipSecDecryptSA)
	return err
}

// DeleteIPSecSADecrypt deletes an IPSec decrypt SA
func (na *state.Nagent) DeleteIPSecSADecrypt(tn, namespace, name string) error {
	ipSecDecryptSA := &netproto.IPSecSADecrypt{
		TypeMeta: api.TypeMeta{Kind: "IPSecSADecrypt"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    tn,
			Namespace: namespace,
			Name:      name,
		},
	}
	err := na.validateMeta(ipSecDecryptSA.Kind, ipSecDecryptSA.ObjectMeta)
	if err != nil {
		return err
	}

	existingIPSecSADecrypt, err := na.FindIPSecSADecrypt(ipSecDecryptSA.ObjectMeta)
	if err != nil {
		log.Errorf("IPSecDecrypt %+v not found", ipSecDecryptSA.ObjectMeta)
		return errors.New("IPSec decrypt SA not found")
	}

	// find the corresponding namespace
	ns, err := na.FindNamespace(existingIPSecSADecrypt.ObjectMeta)
	if err != nil {
		return err
	}

	vrf, err := na.ValidateVrf(existingIPSecSADecrypt.Tenant, existingIPSecSADecrypt.Namespace, existingIPSecSADecrypt.Spec.VrfName)
	if err != nil {
		log.Errorf("Failed to find the vrf %v", existingIPSecSADecrypt.Spec.VrfName)
		return err
	}

	// ensure that the tep-name resolves to a valid vrf.
	tepVrf, err := na.ValidateVrf(existingIPSecSADecrypt.Tenant, existingIPSecSADecrypt.Namespace, existingIPSecSADecrypt.Spec.TepVrf)
	if err != nil {
		log.Errorf("Failed to find the TEP vrf %v", existingIPSecSADecrypt.Spec.TepVrf)
		return err
	}
	// check if the current ipsec sa decrypt rule has any objects referring to it
	err = na.Solver.Solve(existingIPSecSADecrypt)
	if err != nil {
		log.Errorf("Found active references to %v. Err: %v", existingIPSecSADecrypt.Name, err)
		return err
	}

	// delete it in the datapath
	err = na.Datapath.DeleteIPSecSADecrypt(existingIPSecSADecrypt, vrf)
	if err != nil {
		log.Errorf("Error deleting IPSec decrypt SA {%+v}. Err: %v", ipSecDecryptSA, err)
	}

	// update parent references
	err = na.Solver.Remove(tepVrf, existingIPSecSADecrypt)
	if err != nil {
		log.Errorf("Could not remove the reference to the tep vrf: %v. Err: %v", existingIPSecSADecrypt.Spec.TepVrf, err)
		return err
	}

	err = na.Solver.Remove(vrf, existingIPSecSADecrypt)
	if err != nil {
		log.Errorf("Could not remove the reference to the vrf: %v. Err: %v", existingIPSecSADecrypt.Spec.VrfName, err)
		return err
	}

	err = na.Solver.Remove(ns, existingIPSecSADecrypt)
	if err != nil {
		log.Errorf("Could not remove the reference to the namespace: %v. Err: %v", ns.Name, err)
		return err
	}

	// delete from db
	key := na.Solver.ObjectKey(ipSecDecryptSA.ObjectMeta, ipSecDecryptSA.TypeMeta)
	na.Lock()
	delete(na.IPSecSADecryptDB, key)
	na.Unlock()
	err = na.Store.Delete(ipSecDecryptSA)

	return err
}
