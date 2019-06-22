// {C} Copyright 2019 Pensando Systems Inc. All rights reserved.

package impl

import (
	"context"
	"fmt"
	"reflect"
	"strings"

	"github.com/pensando/sw/api/generated/cluster"
	"github.com/pensando/sw/api/interfaces"
	"github.com/pensando/sw/api/utils"
	"github.com/pensando/sw/venice/utils/kvstore"
)

func (cl *clusterHooks) checkNonUserModifiableSmartNICFields(updObj, curObj *cluster.SmartNIC) []string {
	NUMFields := []string{"ID", "IPConfig", "NetworkMode", "MgmtVlan", "Controllers"}

	var errs []string
	updSpec := reflect.Indirect(reflect.ValueOf(updObj)).FieldByName("Spec")
	curSpec := reflect.Indirect(reflect.ValueOf(curObj)).FieldByName("Spec")

	for _, fn := range NUMFields {
		updField := updSpec.FieldByName(fn).Interface()
		curField := curSpec.FieldByName(fn).Interface()
		if !reflect.DeepEqual(updField, curField) {
			errs = append(errs, fn)
		}
	}

	return errs
}

func (cl *clusterHooks) smartNICPreCommitHook(ctx context.Context, kvs kvstore.Interface, txn kvstore.Txn, key string, oper apiintf.APIOperType, dryrun bool, i interface{}) (interface{}, bool, error) {

	if oper == apiintf.CreateOper {
		return i, true, nil
	}

	if ctx == nil || kvs == nil {
		return i, false, fmt.Errorf("smartNICPreCommitHook called with NIL parameter, ctx: %p, kvs: %p", ctx, kvs)
	}

	curNIC := &cluster.SmartNIC{}
	err := kvs.Get(ctx, key, curNIC)
	if err != nil {
		cl.logger.Errorf("Error getting SmartNIC with key [%s] in API server smartNICPreCommitHook pre-commit hook", key)
		return i, false, fmt.Errorf("Error getting object: %v", err)
	}

	admitted := curNIC.Status.AdmissionPhase == cluster.SmartNICStatus_ADMITTED.String()

	// Prevent deletion of SmartNIC object if Phase = ADMITTED
	if oper == apiintf.DeleteOper && admitted {
		errStr := fmt.Sprintf("Cannot delete SmartNIC Object because it is in %s phase. Please de-admit or decommission before deleting.", cluster.SmartNICStatus_ADMITTED.String())
		cl.logger.Errorf(errStr)
		return i, true, fmt.Errorf(errStr)
	}

	updNIC, ok := i.(cluster.SmartNIC)
	if !ok {
		cl.logger.ErrorLog("method", "smartNICPreCommitHook", "msg", fmt.Sprintf("called for invalid object type [%#v]", i))
		return i, true, fmt.Errorf("Invalid input type")
	}

	// Prevent mode change (decommissioning) if NIC is NOT admitted
	if oper == apiintf.UpdateOper && !admitted {
		if updNIC.Spec.MgmtMode != cluster.SmartNICSpec_NETWORK.String() {
			errStr := fmt.Sprintf("Management mode change not allowed for SmartNIC because it is not in %s phase", cluster.SmartNICStatus_ADMITTED.String())
			cl.logger.Errorf(errStr)
			return i, true, fmt.Errorf(errStr)
		}
	}

	// Reject user-initiated modifications of Spec fields like ID and NetworkMode, as NMD currently
	// does not have code to react to the changes.
	if oper == apiintf.UpdateOper && apiutils.IsUserRequestCtx(ctx) {
		errs := cl.checkNonUserModifiableSmartNICFields(&updNIC, curNIC)
		if len(errs) > 0 {
			return i, true, fmt.Errorf("Modification of SmartNIC object fields %s is not allowed", strings.Join(errs, ", "))
		}
	}

	// Add a comparator for CAS
	txn.AddComparator(kvstore.Compare(kvstore.WithVersion(key), "=", curNIC.ResourceVersion))

	return i, true, nil
}
