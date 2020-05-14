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
		cl.logger.Errorf("Unsupported DSCProfile: DeploymentTarget:%s, FeatureSet:%s", updDSCProfile.Spec.DeploymentTarget, updDSCProfile.Spec.FeatureSet)
		return i, false, fmt.Errorf("Unsupported DSCProfile: DeploymentTarget:%s, FeatureSet:%s", updDSCProfile.Spec.DeploymentTarget, updDSCProfile.Spec.FeatureSet)
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
	if profile.Spec.DeploymentTarget == cluster.DSCProfileSpec_VIRTUALIZED.String() {
		if profile.Spec.FeatureSet != cluster.DSCProfileSpec_FLOWAWARE_FIREWALL.String() {
			return fmt.Errorf(" fwdMode:%s flowpolicy mode:%s is not supported", profile.Spec.DeploymentTarget, profile.Spec.FeatureSet)
		}
	}
	return nil

}
