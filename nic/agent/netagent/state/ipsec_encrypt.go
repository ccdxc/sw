// {C} Copyright 2018 Pensando Systems Inc. All rights reserved.

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

// CreateIPSecSAEncrypt creates an IPSec SA Encrypt rule
func (na *Nagent) CreateIPSecSAEncrypt(ipSecSAEncrypt *netproto.IPSecSAEncrypt) error {
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

		log.Infof("Received duplicate IPSec encrypt SA create {%+v}", ipSecSAEncrypt)
		return nil
	}

	// find the corresponding namespace
	ns, err := na.FindNamespace(ipSecSAEncrypt.Tenant, ipSecSAEncrypt.Namespace)
	if err != nil {
		return err
	}

	// validate that the spec has a tep name
	if len(ipSecSAEncrypt.Spec.TepNS) == 0 {
		return fmt.Errorf("IPSecSAEncrypt needs to specify a tep-name")
	}

	// ensure that the tep-name resolves to a valid namespace.
	tep, err := na.FindNamespace(ipSecSAEncrypt.Tenant, ipSecSAEncrypt.Spec.TepNS)
	if err != nil {
		return fmt.Errorf("tep-name %v doesn't refer to a valid namespace", ipSecSAEncrypt.Spec.TepNS)
	}

	// Only ESP Protocol supported for encrypt SA
	if ipSecSAEncrypt.Spec.Protocol != "ESP" {
		return fmt.Errorf("ipsec sa encrypt protocol should be ESP")
	}

	ipSecSAEncrypt.Status.IPSecSAEncryptID, err = na.Store.GetNextID(types.IPSecSAEncryptID)

	if err != nil {
		log.Errorf("Could not allocate IPSec encrypt SA id. {%+v}", err)
		return err
	}

	// create it in datapath
	err = na.Datapath.CreateIPSecSAEncrypt(ipSecSAEncrypt, ns, tep)
	if err != nil {
		log.Errorf("Error creating IPSec Encrypt rule in datapath. IPSecSAEncrypt {%+v}. Err: %v", ipSecSAEncrypt, err)
		return err
	}

	// save it in db
	key := objectKey(ipSecSAEncrypt.ObjectMeta, ipSecSAEncrypt.TypeMeta)
	na.Lock()
	na.IPSecSAEncryptDB[key] = ipSecSAEncrypt
	na.Unlock()
	err = na.Store.Write(ipSecSAEncrypt)

	return err
}

// FindIPSecSAEncrypt finds an IPSec SA Encrypt rule in local db
func (na *Nagent) FindIPSecSAEncrypt(meta api.ObjectMeta) (*netproto.IPSecSAEncrypt, error) {
	typeMeta := api.TypeMeta{
		Kind: "IPSecSAEncrypt",
	}
	// lock the db
	na.Lock()
	defer na.Unlock()

	// lookup the database
	key := objectKey(meta, typeMeta)
	ipSecEncryptSA, ok := na.IPSecSAEncryptDB[key]
	if !ok {
		return nil, fmt.Errorf("IPSec encrypt SA not found %v", meta.Name)
	}

	return ipSecEncryptSA, nil
}

// ListIPSecSAEncrypt returns the list of IPSec encrypt SA
func (na *Nagent) ListIPSecSAEncrypt() []*netproto.IPSecSAEncrypt {
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
func (na *Nagent) UpdateIPSecSAEncrypt(ipSecEncryptSA *netproto.IPSecSAEncrypt) error {
	// find the corresponding namespace
	ns, err := na.FindNamespace(ipSecEncryptSA.Tenant, ipSecEncryptSA.Namespace)
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

	err = na.Datapath.UpdateIPSecSAEncrypt(ipSecEncryptSA, ns)
	key := objectKey(ipSecEncryptSA.ObjectMeta, ipSecEncryptSA.TypeMeta)
	na.Lock()
	na.IPSecSAEncryptDB[key] = ipSecEncryptSA
	na.Unlock()
	err = na.Store.Write(ipSecEncryptSA)
	return err
}

// DeleteIPSecSAEncrypt deletes an IPSec encrypt SA
func (na *Nagent) DeleteIPSecSAEncrypt(ipSecEncryptSA *netproto.IPSecSAEncrypt) error {
	err := na.validateMeta(ipSecEncryptSA.Kind, ipSecEncryptSA.ObjectMeta)
	if err != nil {
		return err
	}
	// find the corresponding namespace
	ns, err := na.FindNamespace(ipSecEncryptSA.Tenant, ipSecEncryptSA.Namespace)
	if err != nil {
		return err
	}

	existingIPSecSAEncrypt, err := na.FindIPSecSAEncrypt(ipSecEncryptSA.ObjectMeta)
	if err != nil {
		log.Errorf("IPSecEncrypt %+v not found", ipSecEncryptSA.ObjectMeta)
		return errors.New("IPSec encrypt SA not found")
	}

	// delete it in the datapath
	err = na.Datapath.DeleteIPSecSAEncrypt(existingIPSecSAEncrypt, ns)
	if err != nil {
		log.Errorf("Error deleting IPSec encrypt SA {%+v}. Err: %v", ipSecEncryptSA, err)
	}

	// delete from db
	key := objectKey(ipSecEncryptSA.ObjectMeta, ipSecEncryptSA.TypeMeta)
	na.Lock()
	delete(na.IPSecSAEncryptDB, key)
	na.Unlock()
	err = na.Store.Delete(ipSecEncryptSA)

	return err
}
