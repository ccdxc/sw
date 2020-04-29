// {C} Copyright 2019 Pensando Systems Inc. All rights reserved.

package impl

import (
	"context"
	"fmt"

	"github.com/pensando/sw/api/generated/cluster"
	apiintf "github.com/pensando/sw/api/interfaces"
	apiutils "github.com/pensando/sw/api/utils"
	"github.com/pensando/sw/venice/globals"
	"github.com/pensando/sw/venice/utils/kvstore"
)

func (cl *clusterHooks) DSCProfilePreCommitHook(ctx context.Context, kvs kvstore.Interface, txn kvstore.Txn, key string, oper apiintf.APIOperType, dryrun bool, i interface{}) (interface{}, bool, error) {
	updDSCProfile, ok := i.(cluster.DSCProfile)
	if !ok {
		cl.logger.ErrorLog("method", "DSCProfilePreCommitHook", "msg", fmt.Sprintf("called for invalid object type [%#v]", i))
		return i, true, errInvalidInputType
	}
	err := checkValidProfile(updDSCProfile)
	if err != nil {
		cl.logger.Error(err)
		return i, false, fmt.Errorf("Error DSCProfile: %v", err)
	}
	if oper == apiintf.CreateOper {
		return i, true, nil
	}

	curDSCProfile := &cluster.DSCProfile{}
	// Get from the persisted DB here.
	pctx := apiutils.SetVar(ctx, apiutils.CtxKeyGetPersistedKV, true)
	err = kvs.Get(pctx, key, curDSCProfile)
	if err != nil {
		cl.logger.Errorf("Error getting DSCProfile with key [%s] in API server DSCProfilePreCommitHook pre-commit hook", key)
		return i, false, fmt.Errorf("Error getting object: %v", err)
	}

	switch oper {
	case apiintf.DeleteOper:
		if curDSCProfile.Name == globals.DefaultDSCProfile {
			errStr := fmt.Sprintf("Cannot delete %s  Object", globals.DefaultDSCProfile)
			cl.logger.Errorf(errStr)
			return i, true, fmt.Errorf(errStr)
		}
	case apiintf.UpdateOper:
		err = verifyAllowedProfile(*curDSCProfile, updDSCProfile)
		if err != nil {
			return i, false, fmt.Errorf("error in validating profile: %v", err)
		}
	}

	// Add a comparator for CAS
	txn.AddComparator(kvstore.Compare(kvstore.WithVersion(key), "=", curDSCProfile.ResourceVersion))
	return i, true, nil
}

func checkValidProfile(profile cluster.DSCProfile) error {
	interVMServices := profile.Spec.Features.InterVMServices
	flowAware := profile.Spec.Features.FlowAware
	fireWall := profile.Spec.Features.Firewall
	if interVMServices {
		if flowAware != true || fireWall != true {
			return fmt.Errorf("InterVMServices should be enabled along with flowaware and firewall features enabled")
		}
	} else {
		if (fireWall == true) && (flowAware == false) {
			return fmt.Errorf("Enable flowAware in order to enable fireWall feature")

		}
	}
	return nil
}
