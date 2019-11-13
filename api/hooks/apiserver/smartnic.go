// {C} Copyright 2019 Pensando Systems Inc. All rights reserved.

package impl

import (
	"context"
	"fmt"
	"reflect"
	"strings"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/api/generated/cluster"
	diagapi "github.com/pensando/sw/api/generated/diagnostics"
	"github.com/pensando/sw/api/interfaces"
	apiutils "github.com/pensando/sw/api/utils"
	apisrvpkg "github.com/pensando/sw/venice/apiserver/pkg"
	"github.com/pensando/sw/venice/globals"
	"github.com/pensando/sw/venice/utils/diagnostics"
	"github.com/pensando/sw/venice/utils/kvstore"
)

func (cl *clusterHooks) checkNonUserModifiableSmartNICFields(updObj, curObj *cluster.DistributedServiceCard) []string {
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
		nic, ok := i.(cluster.DistributedServiceCard)
		if !ok {
			return i, true, errInvalidInputType
		}
		var nicIPAddr string // create modules with empty IP address if smartnic doesn't have an IP yet
		if nic.Status.IPConfig != nil {
			nicIPAddr = nic.Status.IPConfig.IPAddress
		}
		modObjs := diagnostics.NewNaplesModules(nic.Name, nicIPAddr, apisrvpkg.MustGetAPIServer().GetVersion())
		for _, modObj := range modObjs {
			if err := txn.Create(modObj.MakeKey("diagnostics"), modObj); err != nil {
				cl.logger.ErrorLog("method", "smartNICPreCommitHook", "msg",
					fmt.Sprintf("error adding module obj [%s] to transaction for smart nic [%s] creation", modObj.Name, nic.Name), "error", err)
				continue // TODO: throw an event
			}
		}
		return i, true, nil
	}

	if ctx == nil || kvs == nil {
		return i, false, fmt.Errorf("smartNICPreCommitHook called with NIL parameter, ctx: %p, kvs: %p", ctx, kvs)
	}

	curNIC := &cluster.DistributedServiceCard{}
	// Get from the persisted DB here.
	pctx := apiutils.SetVar(ctx, apiutils.CtxKeyGetPersistedKV, true)
	err := kvs.Get(pctx, key, curNIC)
	if err != nil {
		cl.logger.Errorf("Error getting DistributedServiceCard with key [%s] in API server smartNICPreCommitHook pre-commit hook", key)
		return i, false, fmt.Errorf("Error getting object: %v", err)
	}

	admitted := curNIC.Status.AdmissionPhase == cluster.DistributedServiceCardStatus_ADMITTED.String()
	switch oper {
	case apiintf.DeleteOper:
		// Prevent deletion of DistributedServiceCard object if Phase = ADMITTED
		if admitted {
			errStr := fmt.Sprintf("Cannot delete DistributedServiceCard Object because it is in %s phase. Please de-admit or decommission before deleting.", cluster.DistributedServiceCardStatus_ADMITTED.String())
			cl.logger.Errorf(errStr)
			return i, true, fmt.Errorf(errStr)
		}
		// delete module objects for processes running on the smart nic
		into := &diagapi.ModuleList{}
		nctx := apiutils.SetVar(ctx, apiutils.CtxKeyObjKind, fmt.Sprintf("%s.%s", "diagnostics", string(diagapi.KindModule)))
		if err := kvs.ListFiltered(nctx, globals.ModulesKey, into, api.ListWatchOptions{
			FieldSelector: getFieldSelector(curNIC.Name),
		}); err != nil {
			cl.logger.ErrorLog("method", "smartNICPreCommitHook", "msg",
				fmt.Sprintf("unable to list module objects for smart nic [%s], id [%s]", curNIC.Name, curNIC.Spec.ID), "error", err)
			break
		}
		for _, modObj := range into.Items {
			if err := txn.Delete(modObj.MakeKey("diagnostics")); err != nil {
				cl.logger.ErrorLog("method", "smartNICPreCommitHook", "msg",
					fmt.Sprintf("error adding module obj [%s] to transaction for deletion", modObj.Name), "error", err)
				continue
			}
			cl.logger.DebugLog("method", "smartNICPreCommitHook", "msg", fmt.Sprintf("deleting module: %s", modObj.Name))
		}
	case apiintf.UpdateOper:
		updNIC, ok := i.(cluster.DistributedServiceCard)
		if !ok {
			cl.logger.ErrorLog("method", "smartNICPreCommitHook", "msg", fmt.Sprintf("called for invalid object type [%#v]", i))
			return i, true, errInvalidInputType
		}
		if updNIC.Status.IPConfig != nil { // update IP address of smartnic in module object
			into := &diagapi.ModuleList{}
			nctx := apiutils.SetVar(ctx, apiutils.CtxKeyObjKind, fmt.Sprintf("%s.%s", "diagnostics", string(diagapi.KindModule)))
			if err := kvs.ListFiltered(nctx, globals.ModulesKey, into, api.ListWatchOptions{
				FieldSelector: getFieldSelector(curNIC.Name),
			}); err != nil {
				cl.logger.ErrorLog("method", "smartNICPreCommitHook", "msg",
					fmt.Sprintf("unable to list module objects for smart nic [%s], id [%s]", curNIC.Name, curNIC.Spec.ID), "error", err)
			}
			for _, modObj := range into.Items {
				modObj.Status.Node = updNIC.Status.IPConfig.IPAddress
				// not doing CAS, don't want smart nic updates to fail, we are ok with module object reverting to older version
				if err := txn.Update(modObj.MakeKey("diagnostics"), modObj); err != nil {
					cl.logger.ErrorLog("method", "smartNICPreCommitHook", "msg",
						fmt.Sprintf("error adding module obj [%s] to transaction for deletion", modObj.Name), "error", err)
					continue
				}
				cl.logger.DebugLog("method", "smartNICPreCommitHook", "msg", fmt.Sprintf("updating module: %s with IP: %s", modObj.Name, modObj.Status.Node))
			}
		}

		// Prevent mode change (decommissioning) if NIC is NOT admitted
		if !admitted {
			if updNIC.Spec.MgmtMode != cluster.DistributedServiceCardSpec_NETWORK.String() {
				errStr := fmt.Sprintf("Management mode change not allowed for DistributedServiceCard because it is not in %s phase", cluster.DistributedServiceCardStatus_ADMITTED.String())
				cl.logger.Errorf(errStr)
				return i, true, fmt.Errorf(errStr)
			}
		}
		// Reject user-initiated modifications of Spec fields like ID and NetworkMode, as NMD currently
		// does not have code to react to the changes.
		if apiutils.IsUserRequestCtx(ctx) {
			// Workaround for ...
			// Once the SmartNIC is admitted, disallow flipping "Spec.Admit" back to false
			if admitted && updNIC.Spec.Admit == false && curNIC.Spec.Admit == true {
				return i, true, fmt.Errorf("Spec.Admit cannot be changed to false once the DistributedServiceCard is admitted")
			}
			errs := cl.checkNonUserModifiableSmartNICFields(&updNIC, curNIC)
			if len(errs) > 0 {
				return i, true, fmt.Errorf("Modification of SmartNIC object fields %s is not allowed", strings.Join(errs, ", "))
			}
		}
	}

	// Add a comparator for CAS
	txn.AddComparator(kvstore.Compare(kvstore.WithVersion(key), "=", curNIC.ResourceVersion))

	return i, true, nil
}

func getFieldSelector(nic string) string {
	modules := diagnostics.ListSupportedNaplesModules()
	var moduleNames []string
	for _, val := range modules {
		moduleNames = append(moduleNames, fmt.Sprintf("%s-%s", nic, val))
	}
	return fmt.Sprintf("meta.name in (%s),status.category=%s", strings.Join(moduleNames, ","), diagapi.ModuleStatus_Naples.String())
}
