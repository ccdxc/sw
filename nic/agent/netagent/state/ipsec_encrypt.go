// {C} Copyright 2018 Pensando Systems Inc. All rights reserved.

package state

import (
	"errors"

	"github.com/gogo/protobuf/proto"

	"fmt"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/venice/ctrler/npm/rpcserver/netproto"
	"github.com/pensando/sw/venice/utils/log"
)

// CreateIPSecSAEncrypt creates an IPSec SA Encrypt rule
func (na *NetAgent) CreateIPSecSAEncrypt(ipSecSAEncrypt *netproto.IPSecSAEncrypt) error {
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

	ipSecSAEncrypt.Status.IPSecSAEncryptID, err = na.store.GetNextID(IPSecSAEncryptID)

	if err != nil {
		log.Errorf("Could not allocate IPSec encrypt SA id. {%+v}", err)
		return err
	}

	// create it in datapath
	err = na.datapath.CreateIPSecSAEncrypt(ipSecSAEncrypt, ns)
	if err != nil {
		log.Errorf("Error creating IPSec Encrypt rule in datapath. IPSecSAEncrypt {%+v}. Err: %v", ipSecSAEncrypt, err)
		return err
	}

	// save it in db
	key := objectKey(ipSecSAEncrypt.ObjectMeta, ipSecSAEncrypt.TypeMeta)
	na.Lock()
	na.ipSecSAEncryptDB[key] = ipSecSAEncrypt
	na.Unlock()
	err = na.store.Write(ipSecSAEncrypt)

	return err
}

// FindIPSecSAEncrypt finds an IPSec SA Encrypt rule in local db
func (na *NetAgent) FindIPSecSAEncrypt(meta api.ObjectMeta) (*netproto.IPSecSAEncrypt, error) {
	typeMeta := api.TypeMeta{
		Kind: "IPSecSAEncrypt",
	}
	// lock the db
	na.Lock()
	defer na.Unlock()

	// lookup the database
	key := objectKey(meta, typeMeta)
	ipSecEncryptSA, ok := na.ipSecSAEncryptDB[key]
	if !ok {
		return nil, fmt.Errorf("IPSec encrypt SA not found %v", ipSecEncryptSA)
	}

	return ipSecEncryptSA, nil
}

// ListIPSecSAEncrypt returns the list of IPSec encrypt SA
func (na *NetAgent) ListIPSecSAEncrypt() []*netproto.IPSecSAEncrypt {
	var ipSecEncryptSAList []*netproto.IPSecSAEncrypt
	// lock the db
	na.Lock()
	defer na.Unlock()

	for _, ipSecEncryptSA := range na.ipSecSAEncryptDB {
		ipSecEncryptSAList = append(ipSecEncryptSAList, ipSecEncryptSA)
	}

	return ipSecEncryptSAList
}

// UpdateIPSecSAEncrypt updates an IPSec encrypt SA
func (na *NetAgent) UpdateIPSecSAEncrypt(ipSecEncryptSA *netproto.IPSecSAEncrypt) error {
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

	err = na.datapath.UpdateIPSecSAEncrypt(ipSecEncryptSA, ns)
	key := objectKey(ipSecEncryptSA.ObjectMeta, ipSecEncryptSA.TypeMeta)
	na.Lock()
	na.ipSecSAEncryptDB[key] = ipSecEncryptSA
	na.Unlock()
	err = na.store.Write(ipSecEncryptSA)
	return err
}

// DeleteIPSecSAEncrypt deletes an IPSec encrypt SA
func (na *NetAgent) DeleteIPSecSAEncrypt(ipSecEncryptSA *netproto.IPSecSAEncrypt) error {
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
	err = na.datapath.DeleteIPSecSAEncrypt(existingIPSecSAEncrypt, ns)
	if err != nil {
		log.Errorf("Error deleting IPSec encrypt SA {%+v}. Err: %v", ipSecEncryptSA, err)
	}

	// delete from db
	key := objectKey(ipSecEncryptSA.ObjectMeta, ipSecEncryptSA.TypeMeta)
	na.Lock()
	delete(na.ipSecSAEncryptDB, key)
	na.Unlock()
	err = na.store.Delete(ipSecEncryptSA)

	return err
}
