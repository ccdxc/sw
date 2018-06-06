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

// CreateIPSecSADecrypt creates an IPSec SA Decrypt rule
func (na *Nagent) CreateIPSecSADecrypt(ipSecSADecrypt *netproto.IPSecSADecrypt) error {
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

		log.Infof("Received duplicate IPSec decrypt SA create {%+v}", ipSecSADecrypt)
		return nil
	}

	// find the corresponding namespace
	ns, err := na.FindNamespace(ipSecSADecrypt.Tenant, ipSecSADecrypt.Namespace)
	if err != nil {
		return err
	}

	// validate that the spec has a tep name
	if len(ipSecSADecrypt.Spec.TepNS) == 0 {
		return fmt.Errorf("IPSecSAEncrypt needs to specify a tep-name")
	}

	// ensure that the tep-name resolves to a valid namespace.
	tep, err := na.FindNamespace(ipSecSADecrypt.Tenant, ipSecSADecrypt.Spec.TepNS)
	if err != nil {
		return fmt.Errorf("tep-name %v doesn't refer to a valid namespace", ipSecSADecrypt.Spec.TepNS)
	}

	// Only ESP Protocol supported for encrypt SA
	if ipSecSADecrypt.Spec.Protocol != "ESP" {
		return fmt.Errorf("ipsec sa decrypt protocol should be ESP")
	}

	ipSecSADecrypt.Status.IPSecSADecryptID, err = na.Store.GetNextID(types.IPSecSADecryptID)

	if err != nil {
		log.Errorf("Could not allocate IPSec decrypt SA id. {%+v}", err)
		return err
	}

	// create it in datapath
	err = na.Datapath.CreateIPSecSADecrypt(ipSecSADecrypt, ns, tep)
	if err != nil {
		log.Errorf("Error creating IPSec Decrypt rule in datapath. IPSecSADecrypt {%+v}. Err: %v", ipSecSADecrypt, err)
		return err
	}

	// save it in db
	key := objectKey(ipSecSADecrypt.ObjectMeta, ipSecSADecrypt.TypeMeta)
	na.Lock()
	na.IPSecSADecryptDB[key] = ipSecSADecrypt
	na.Unlock()
	err = na.Store.Write(ipSecSADecrypt)

	return err
}

// FindIPSecSADecrypt finds an IPSec SA Decrypt rule in local db
func (na *Nagent) FindIPSecSADecrypt(meta api.ObjectMeta) (*netproto.IPSecSADecrypt, error) {
	typeMeta := api.TypeMeta{
		Kind: "IPSecSADecrypt",
	}
	// lock the db
	na.Lock()
	defer na.Unlock()

	// lookup the database
	key := objectKey(meta, typeMeta)
	ipSecDecryptSA, ok := na.IPSecSADecryptDB[key]
	if !ok {
		return nil, fmt.Errorf("IPSec decrypt SA not found %v", meta.Name)
	}

	return ipSecDecryptSA, nil
}

// ListIPSecSADecrypt returns the list of IPSec decrypt SA
func (na *Nagent) ListIPSecSADecrypt() []*netproto.IPSecSADecrypt {
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
func (na *Nagent) UpdateIPSecSADecrypt(ipSecDecryptSA *netproto.IPSecSADecrypt) error {
	// find the corresponding namespace
	ns, err := na.FindNamespace(ipSecDecryptSA.Tenant, ipSecDecryptSA.Namespace)
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

	err = na.Datapath.UpdateIPSecSADecrypt(ipSecDecryptSA, ns)
	key := objectKey(ipSecDecryptSA.ObjectMeta, ipSecDecryptSA.TypeMeta)
	na.Lock()
	na.IPSecSADecryptDB[key] = ipSecDecryptSA
	na.Unlock()
	err = na.Store.Write(ipSecDecryptSA)
	return err
}

// DeleteIPSecSADecrypt deletes an IPSec decrypt SA
func (na *Nagent) DeleteIPSecSADecrypt(ipSecDecryptSA *netproto.IPSecSADecrypt) error {
	err := na.validateMeta(ipSecDecryptSA.Kind, ipSecDecryptSA.ObjectMeta)
	if err != nil {
		return err
	}
	// find the corresponding namespace
	ns, err := na.FindNamespace(ipSecDecryptSA.Tenant, ipSecDecryptSA.Namespace)
	if err != nil {
		return err
	}

	existingIPSecSADecrypt, err := na.FindIPSecSADecrypt(ipSecDecryptSA.ObjectMeta)
	if err != nil {
		log.Errorf("IPSecDecrypt %+v not found", ipSecDecryptSA.ObjectMeta)
		return errors.New("IPSec decrypt SA not found")
	}

	// delete it in the datapath
	err = na.Datapath.DeleteIPSecSADecrypt(existingIPSecSADecrypt, ns)
	if err != nil {
		log.Errorf("Error deleting IPSec decrypt SA {%+v}. Err: %v", ipSecDecryptSA, err)
	}

	// delete from db
	key := objectKey(ipSecDecryptSA.ObjectMeta, ipSecDecryptSA.TypeMeta)
	na.Lock()
	delete(na.IPSecSADecryptDB, key)
	na.Unlock()
	err = na.Store.Delete(ipSecDecryptSA)

	return err
}
