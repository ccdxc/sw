// {C} Copyright 2017 Pensando Systems Inc. All rights reserved.

package state

import (
	"errors"
	"fmt"
	"strings"

	"github.com/gogo/protobuf/proto"

	"github.com/pensando/sw/nic/agent/dscagent/types/irisproto"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/nic/agent/netagent/state/types"
	"github.com/pensando/sw/nic/agent/protos/netproto"
	"github.com/pensando/sw/venice/utils/log"
)

// CreateVrf creates a vrf
func (na *Nagent) CreateVrf(vrf *netproto.Vrf) error {
	err := na.validateMeta(vrf.Kind, vrf.ObjectMeta)
	if err != nil {
		return err
	}
	oldProfile, err := na.FindVrf(vrf.ObjectMeta)
	if err == nil {
		// check if the contents are same
		if !proto.Equal(oldProfile, vrf) {
			log.Errorf("Vrf %+v already exists", oldProfile)
			return errors.New("vrf already exists")
		}

		log.Infof("Received duplicate vrf create {%+v}", vrf)
		return nil
	}

	// find the corresponding namespace
	ns, err := na.FindNamespace(vrf.ObjectMeta)
	if err != nil {
		return err
	}

	// Allocate ID only on first object creates and use existing ones during config replay
	if vrf.Status.VrfID == 0 {
		vrf.Status.VrfID, err = na.Store.GetNextID(types.VrfID, 0)
		vrf.Status.VrfID += types.VrfOffset
	}

	if err != nil {
		log.Errorf("Could not allocate vrf id. {%+v}", err)
		return err
	}

	// create it in datapath
	err = na.Datapath.CreateVrf(vrf.Status.VrfID, vrf.Spec.VrfType)

	if err != nil && !strings.Contains(err.Error(), halproto.ApiStatus_API_STATUS_EXISTS_ALREADY.String()) {
		log.Errorf("Error creating vrf in datapath. Vrf {%+v}. Err: %v", vrf, err)
		return err
	}

	err = na.Solver.Add(ns, vrf)
	if err != nil {
		log.Errorf("Could not add dependency. Parent: %v. Child: %v", ns, vrf)
		return err
	}

	// save it in db
	key := na.Solver.ObjectKey(vrf.ObjectMeta, vrf.TypeMeta)
	na.Lock()
	na.VrfDB[key] = vrf
	na.Unlock()
	dat, _ := vrf.Marshal()
	err = na.Store.RawWrite(vrf.GetKind(), vrf.GetKey(), dat)

	return err
}

// FindVrf finds a vrf in local db
func (na *Nagent) FindVrf(meta api.ObjectMeta) (*netproto.Vrf, error) {
	typeMeta := api.TypeMeta{
		Kind: "Vrf",
	}
	// lock the db
	na.Lock()
	defer na.Unlock()

	// lookup the database
	key := na.Solver.ObjectKey(meta, typeMeta)
	vrf, ok := na.VrfDB[key]
	if !ok {
		return nil, fmt.Errorf("vrf not found %v", meta.Name)
	}

	return vrf, nil
}

// ListVrf returns the list of security group polices
func (na *Nagent) ListVrf() []*netproto.Vrf {
	var vrfList []*netproto.Vrf
	// lock the db
	na.Lock()
	defer na.Unlock()

	for _, vrf := range na.VrfDB {
		vrfList = append(vrfList, vrf)
	}

	return vrfList
}

// UpdateVrf updates a vrf
func (na *Nagent) UpdateVrf(vrf *netproto.Vrf) error {
	// find the corresponding namespace
	_, err := na.FindNamespace(vrf.ObjectMeta)
	if err != nil {
		return err
	}

	existingVrf, err := na.FindVrf(vrf.ObjectMeta)
	if err != nil {
		log.Errorf("Vrf %v not found", vrf.ObjectMeta)
		return err
	}

	if proto.Equal(vrf, existingVrf) {
		log.Infof("Nothing to update.")
		return nil
	}

	// Populate the ID from existing vrf to ensure that HAL recognizes this.
	vrf.Status.VrfID = existingVrf.Status.VrfID

	err = na.Datapath.UpdateVrf(vrf.Status.VrfID)
	if err != nil {
		log.Errorf("Error updating the Security Profile {%+v} in datapath. Err: %v", existingVrf, err)
		return err
	}
	key := na.Solver.ObjectKey(vrf.ObjectMeta, vrf.TypeMeta)
	na.Lock()
	na.VrfDB[key] = vrf
	na.Unlock()
	err = na.Store.Write(vrf)
	return err
}

// DeleteVrf deletes a vrf
func (na *Nagent) DeleteVrf(tn, namespace, name string) error {
	if tn == "default" && namespace == "default" && name == "default" {
		return errors.New("default vrfs under default tenant and namespace cannot be deleted")
	}
	vrf := &netproto.Vrf{
		TypeMeta: api.TypeMeta{Kind: "Vrf"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    tn,
			Namespace: namespace,
			Name:      name,
		},
	}

	err := na.validateMeta(vrf.Kind, vrf.ObjectMeta)
	if err != nil {
		return err
	}
	// find the corresponding namespace
	ns, err := na.FindNamespace(vrf.ObjectMeta)
	if err != nil {
		return err
	}

	existingVrf, err := na.FindVrf(vrf.ObjectMeta)
	if err != nil {
		log.Errorf("Vrf %+v not found", vrf.ObjectMeta)
		return errors.New("vrf not found")
	}

	// check if the current vrf has any objects referring to it
	err = na.Solver.Solve(existingVrf)
	if err != nil {
		log.Errorf("Found active references to %v. Err: %v", existingVrf.Name, err)
		return err
	}

	// delete it in the datapath
	err = na.Datapath.DeleteVrf(existingVrf.Status.VrfID)
	if err != nil {
		log.Errorf("Error deleting vrf {%+v}. Err: %v", vrf, err)
		return err
	}

	err = na.Solver.Remove(ns, existingVrf)
	if err != nil {
		log.Errorf("Could not remove the reference to the namespace: %v. Err: %v", ns.Name, err)
		return err
	}

	// delete from db
	key := na.Solver.ObjectKey(vrf.ObjectMeta, vrf.TypeMeta)
	na.Lock()
	delete(na.VrfDB, key)
	na.Unlock()
	err = na.Store.RawDelete(vrf.GetKind(), vrf.GetKey())
	return err
}
