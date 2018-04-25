// {C} Copyright 2017 Pensando Systems Inc. All rights reserved.

package state

import (
	"errors"

	cmd "github.com/pensando/sw/api/generated/cluster"
	"github.com/pensando/sw/venice/cmd/grpc"
	"github.com/pensando/sw/venice/utils/log"
)

// RegisterSmartNICReq registers a NIC with CMD
func (n *NMD) RegisterSmartNICReq(nic *cmd.SmartNIC) (grpc.RegisterNICResponse, error) {

	if n.cmd == nil {
		log.Errorf("Failed to register NIC, mac: %s cmd not ready", nic.ObjectMeta.Name)
		return grpc.RegisterNICResponse{}, errors.New("cmd not ready yet")
	}
	resp, err := n.cmd.RegisterSmartNICReq(nic)
	if err != nil {
		log.Errorf("Failed to register NIC, mac: %s err: %v", nic.ObjectMeta.Name, err)
		return resp, err
	}

	log.Infof("Register NIC response mac: %s response: %v", nic.ObjectMeta.Name, resp)
	return resp, nil
}

// UpdateSmartNICReq registers a NIC with CMD/Venice cluster
func (n *NMD) UpdateSmartNICReq(nic *cmd.SmartNIC) (*cmd.SmartNIC, error) {

	if n.cmd == nil {
		log.Errorf("Failed to update NIC, mac: %s cmd not ready", nic.ObjectMeta.Name)
		return nil, errors.New("cmd not ready yet")
	}
	nicObj, err := n.cmd.UpdateSmartNICReq(nic)
	if err != nil || nic == nil {
		log.Errorf("Failed to update NIC, mac: %s err: %v", nic.ObjectMeta.Name, err)
		return nil, err
	}

	log.Infof("Update NIC response mac: %s nic: %+v", nic.ObjectMeta.Name, nicObj)
	return nicObj, nil
}

// CreateSmartNIC creates a local smartNIC object
func (n *NMD) CreateSmartNIC(nic *cmd.SmartNIC) error {

	log.Infof("SmartNIC create, mac: %s", nic.ObjectMeta.Name)

	// add the nic to database
	n.SetSmartNIC(nic)
	err := n.store.Write(nic)

	return err
}

// UpdateSmartNIC updates the local smartNIC object
func (n *NMD) UpdateSmartNIC(nic *cmd.SmartNIC) error {

	log.Infof("SmartNIC update, mac: %s", nic.ObjectMeta.Name)

	// update nic in the DB
	n.SetSmartNIC(nic)
	err := n.store.Write(nic)

	return err
}

// DeleteSmartNIC deletes the local smartNIC object
func (n *NMD) DeleteSmartNIC(nic *cmd.SmartNIC) error {

	log.Infof("SmartNIC delete, mac: %s", nic.ObjectMeta.Name)

	// remove nic from DB
	n.SetSmartNIC(nil)
	err := n.store.Delete(nic)

	return err
}
