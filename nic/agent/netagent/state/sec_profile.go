// {C} Copyright 2017 Pensando Systems Inc. All rights reserved.

package state

import (
	"errors"
	"fmt"

	"github.com/gogo/protobuf/proto"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/nic/agent/netagent/state/types"
	"github.com/pensando/sw/nic/agent/protos/netproto"
	"github.com/pensando/sw/venice/utils/log"
)

// CreateSecurityProfile creates a security profile
func (na *Nagent) CreateSecurityProfile(profile *netproto.SecurityProfile) error {
	var attachmentVrfs []*netproto.Vrf
	err := na.validateMeta(profile.Kind, profile.ObjectMeta)
	if err != nil {
		return err
	}
	oldProfile, err := na.FindSecurityProfile(profile.ObjectMeta)
	if err == nil {
		// check if the contents are same
		if !proto.Equal(oldProfile, profile) {
			log.Errorf("SecurityProfile %+v already exists", oldProfile)
			return errors.New("security profile already exists")
		}

		log.Infof("Received duplicate security profile create {%+v}", profile)
		return nil
	}

	// find the corresponding namespace
	ns, err := na.FindNamespace(profile.ObjectMeta)
	if err != nil {
		return err
	}

	// Find the attachment vrfs
	if len(profile.Spec.AttachVrfs) != 0 {
		for _, v := range profile.Spec.AttachVrfs {
			vrf, err := na.ValidateVrf(profile.Tenant, profile.Namespace, v)
			if err != nil {
				log.Errorf("Failed to find the attachment vrf: %v. Err: %v", v, err)
				return err
			}
			attachmentVrfs = append(attachmentVrfs, vrf)
		}
	} else {
		defaultVrf, err := na.ValidateVrf(profile.Tenant, profile.Namespace, "default")
		if err != nil {
			log.Errorf("Failed to find the default vrf. Err: %v", err)
			return err
		}
		attachmentVrfs = append(attachmentVrfs, defaultVrf)
	}

	// Allocate ID only on first object creates and use existing ones during config replay
	if profile.Status.SecurityProfileID == 0 {
		profile.Status.SecurityProfileID, err = na.Store.GetNextID(types.SecurityProfileID, 0)
		profile.Status.SecurityProfileID += types.SecurityProfileOffset
	}

	if err != nil {
		log.Errorf("Could not allocate security profile id. {%+v}", err)
		return err
	}

	// create it in datapath
	err = na.Datapath.CreateSecurityProfile(profile, attachmentVrfs)
	if err != nil {
		log.Errorf("Error creating security profile in datapath. SecurityProfile {%+v}. Err: %v", profile, err)
		return err
	}

	err = na.Solver.Add(ns, profile)
	if err != nil {
		log.Errorf("Could not add dependency. Parent: %v. Child: %v", ns, profile)
		return err
	}

	// save it in db
	key := na.Solver.ObjectKey(profile.ObjectMeta, profile.TypeMeta)
	na.Lock()
	na.SecurityProfileDB[key] = profile
	na.Unlock()
	err = na.Store.Write(profile)

	return err
}

// FindSecurityProfile finds a security profile in local db
func (na *Nagent) FindSecurityProfile(meta api.ObjectMeta) (*netproto.SecurityProfile, error) {
	typeMeta := api.TypeMeta{
		Kind: "SecurityProfile",
	}
	// lock the db
	na.Lock()
	defer na.Unlock()

	// lookup the database
	key := na.Solver.ObjectKey(meta, typeMeta)
	sgp, ok := na.SecurityProfileDB[key]
	if !ok {
		return nil, fmt.Errorf("security profile not found %v", meta.Name)
	}

	return sgp, nil
}

// ListSecurityProfile returns the list of security group polices
func (na *Nagent) ListSecurityProfile() []*netproto.SecurityProfile {
	var secProfileList []*netproto.SecurityProfile
	// lock the db
	na.Lock()
	defer na.Unlock()

	for _, profile := range na.SecurityProfileDB {
		secProfileList = append(secProfileList, profile)
	}

	return secProfileList
}

// UpdateSecurityProfile updates a security profile
func (na *Nagent) UpdateSecurityProfile(profile *netproto.SecurityProfile) error {
	var attachmentVrfs []*netproto.Vrf

	// find the corresponding namespace
	_, err := na.FindNamespace(profile.ObjectMeta)
	if err != nil {
		return err
	}

	existingProfile, err := na.FindSecurityProfile(profile.ObjectMeta)
	if err != nil {
		log.Errorf("SecurityProfile %v not found", profile.ObjectMeta)
		return err
	}

	if proto.Equal(&profile.Spec, &existingProfile.Spec) {
		return nil
	}

	// Find the attachment vrfs
	for _, v := range profile.Spec.AttachVrfs {
		vrf, err := na.ValidateVrf(profile.Tenant, profile.Namespace, v)
		if err != nil {
			log.Errorf("Failed to find the attachment vrf: %v. Err: %v", v, err)
			return err
		}
		attachmentVrfs = append(attachmentVrfs, vrf)
	}

	// Populate the ID from existing security profile to ensure that HAL recognizes this.
	profile.Status.SecurityProfileID = existingProfile.Status.SecurityProfileID

	err = na.Datapath.UpdateSecurityProfile(profile, attachmentVrfs)
	if err != nil {
		log.Errorf("Error updating the Security Profile {%+v} in datapath. Err: %v", existingProfile, err)
		return err
	}
	key := na.Solver.ObjectKey(profile.ObjectMeta, profile.TypeMeta)
	na.Lock()
	na.SecurityProfileDB[key] = profile
	na.Unlock()
	err = na.Store.Write(profile)
	return err
}

// DeleteSecurityProfile deletes a security profile
func (na *Nagent) DeleteSecurityProfile(tn, namespace, name string) error {
	var attachmentVrfs []*netproto.Vrf

	sgp := &netproto.SecurityProfile{
		TypeMeta: api.TypeMeta{Kind: "SecurityProfile"},
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

	existingSecurityProfile, err := na.FindSecurityProfile(sgp.ObjectMeta)
	if err != nil {
		log.Errorf("SecurityProfile %+v not found", sgp.ObjectMeta)
		return errors.New("security profile not found")
	}

	// Find the attachment vrfs
	if len(existingSecurityProfile.Spec.AttachVrfs) != 0 {
		for _, v := range existingSecurityProfile.Spec.AttachVrfs {
			vrf, err := na.ValidateVrf(existingSecurityProfile.Tenant, existingSecurityProfile.Namespace, v)
			if err != nil {
				log.Errorf("Failed to find the attachment vrf: %v. Err: %v", v, err)
				return err
			}
			attachmentVrfs = append(attachmentVrfs, vrf)
		}
	} else {
		defaultVrf, err := na.ValidateVrf(existingSecurityProfile.Tenant, existingSecurityProfile.Namespace, "default")
		if err != nil {
			log.Errorf("Failed to find the default vrf. Err: %v", err)
			return err
		}
		attachmentVrfs = append(attachmentVrfs, defaultVrf)
	}

	// check if the current security profile has any objects referring to it
	err = na.Solver.Solve(existingSecurityProfile)
	if err != nil {
		log.Errorf("Found active references to %v. Err: %v", existingSecurityProfile.Name, err)
		return err
	}

	// delete it in the datapath
	err = na.Datapath.DeleteSecurityProfile(existingSecurityProfile, attachmentVrfs)
	if err != nil {
		log.Errorf("Error deleting security profile {%+v}. Err: %v", sgp, err)
		return err
	}

	err = na.Solver.Remove(ns, existingSecurityProfile)
	if err != nil {
		log.Errorf("Could not remove the reference to the namespace: %v. Err: %v", ns.Name, err)
		return err
	}

	// delete from db
	key := na.Solver.ObjectKey(sgp.ObjectMeta, sgp.TypeMeta)
	na.Lock()
	delete(na.SecurityProfileDB, key)
	na.Unlock()
	err = na.Store.Delete(sgp)
	return err
}
