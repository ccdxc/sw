// {C} Copyright 2019 Pensando Systems Inc. All rights reserved.

package impl

import (
	"context"
	"fmt"
	"strings"

	"github.com/pensando/sw/api/generated/apiclient"
	"github.com/pensando/sw/api/generated/cluster"
	apiintf "github.com/pensando/sw/api/interfaces"
	apiutils "github.com/pensando/sw/api/utils"
	vldtor "github.com/pensando/sw/venice/utils/apigen/validators"
	"github.com/pensando/sw/venice/utils/kvstore"
	"github.com/pensando/sw/venice/utils/log"
)

const (
	minDSCIDsInHostSpec = 1
	maxDSCIDsInHostSpec = 2
)

// errInvalidHostConfig returns error associated with invalid hostname
func (cl *clusterHooks) errInvalidHostConfig(host string) error {
	return fmt.Errorf("Invalid hostname in Host object: %s", host)
}

// errInvalidMacConfig returns error associated with invalid mac-address
func (cl *clusterHooks) errInvalidMacConfig(mac string) error {
	return fmt.Errorf("Invalid mac address: %s", mac)
}

// errUnsupportedNumberOfDSCIDs returns error associated with an
// unsupported number of DSCIDs specified in a host object
func (cl *clusterHooks) errUnsupportedNumberOfDSCIDs(numFoundDSCIDs, minExpDSCIDs, maxExpDSCIDs int) error {
	return fmt.Errorf("Found %d DSC specifications in Host object, expected min %d, max %d", numFoundDSCIDs, minExpDSCIDs, maxExpDSCIDs)
}

// errInvalidDSCIDSpec returns error associated with invalid SmartNIC ID
func (cl *clusterHooks) errInvalidDSCID() error {
	return fmt.Errorf("Invalid DSC specification in Host object: exactly one of {Id, MAC Address} must be provided")
}

// errHostDSCIDConflicts returns error associated with invalid mac-address
func (cl *clusterHooks) errHostDSCIDConflicts(hostName string, conflicts []string) error {
	return fmt.Errorf("DSC specification for Host object %s conflicts with specifications for Host objects %s."+
		" The same MAC Address or Name cannot appear in multiple host objects", hostName, strings.Join(conflicts, ","))
}

// errHostFieldImmutable returns error when user is trying to modify an immutable field
func (cl *clusterHooks) errHostFieldImmutable(hostName string, fieldName string) error {
	return fmt.Errorf("Error: pre-existed DSC in field %s for Host object %s cannot be modified after creation", fieldName, hostName)
}

// errHostInvalidSmartNICs returns error when user is trying to update host obj to an invalid one
func (cl *clusterHooks) errInvalidHostSmartNICs() error {
	return fmt.Errorf("Error invalid host obj in update oper")
}

// errHostDSCNumDecreased returns error when user is trying to decrease the num of DSC via update oper
func (cl *clusterHooks) errHostDSCNumDecreased() error {
	return fmt.Errorf("Error deleting dsc by update oper")
}

func (cl *clusterHooks) getHostSmartNICConflicts(ctx context.Context, host *cluster.Host, kvs kvstore.Interface) ([]string, error) {
	// Retrieve all host objects and check that the SmartNIC IDs are consistent,
	// i.e. that there are no multiple host objects with the same SmartNIC name
	// or MAC address.
	// Note that in reality each SmartNIC comes with a range of MAC addresses,
	// and user can specify any of them, so we should check for range overlap
	// rather than exact match. However, we don't want to make assumptions on
	// the alignment and size of the ranges, so we postpone the range check
	// until the NIC object is created and actual range is known. If an
	// inconsistency is detected at that point time, we generate an event
	// to notify user.

	if ctx == nil || host == nil || kvs == nil {
		return nil, fmt.Errorf("getHostSmartNICConflicts called with NIL parameter, ctx: %p, host: %p, kvs: %p", ctx, host, kvs)
	}

	var hosts cluster.HostList
	key := hosts.MakeKey(string(apiclient.GroupCluster))
	err := kvs.List(ctx, key, &hosts)
	if err != nil {
		return nil, fmt.Errorf("Error retrieving hosts: %v", err)
	}

	// We don't need to check for conflicts across multiple SmartNIC IDs in the same
	// Host objects because right now each Host object can have only 1 SmartNIC ID
	var conflicts []string
	for _, hostNIC := range host.Spec.DSCs {
		for _, otherHost := range hosts.Items {
			if host.Name == otherHost.Name {
				continue
			}
			for _, otherHostNIC := range otherHost.Spec.DSCs {
				if (hostNIC.MACAddress != "" && hostNIC.MACAddress == otherHostNIC.MACAddress) ||
					(hostNIC.ID != "" && hostNIC.ID == otherHostNIC.ID) {
					conflicts = append(conflicts, otherHost.Name)
				}
			}
		}
	}

	return conflicts, nil
}

func (cl *clusterHooks) hostPreCommitHook(ctx context.Context, kvs kvstore.Interface, txn kvstore.Txn, key string, oper apiintf.APIOperType, dryrun bool, i interface{}) (interface{}, bool, error) {
	host, ok := i.(cluster.Host)
	if !ok {
		cl.logger.ErrorLog("method", "checkHostToSmartNICReferences", "msg", fmt.Sprintf("called for invalid object type [%#v]", i))
		return i, true, fmt.Errorf("Invalid input type")
	}

	if oper == apiintf.DeleteOper {
		return i, true, nil
	}

	conflicts, err := cl.getHostSmartNICConflicts(ctx, &host, kvs)
	if err != nil {
		log.Errorf("Error performing pre-commit check for Host object %s: %v", host.Name, err)
		return i, true, fmt.Errorf("Internal error during pre-commit check for Host object %s", host.Name)
	}

	if len(conflicts) > 0 {
		return i, true, cl.errHostDSCIDConflicts(host.Name, conflicts)
	}

	// Note that there is still a chance that by the time this transaction commits,
	// another conflicting Host objects is present.
	// In that case the conflict will be caught and reported asynchronously by CMD.

	// Disallow direct change of the referenced SmartNIC.
	// This is meant to prevent disruptive user mistakes.
	if oper == apiintf.UpdateOper {
		curHost := &cluster.Host{}
		pctx := apiutils.SetVar(ctx, apiutils.CtxKeyGetPersistedKV, true)
		err := kvs.Get(pctx, key, curHost)
		if err != nil {
			cl.logger.Errorf("Error getting Host with key [%s] in API server hostPreCommitHook pre-commit hook: %v", key, err)
			return i, false, fmt.Errorf("Error getting object: %v", err)
		}

		// Validate number of dsc in new host obj
		errList := cl.validateHostSmartNICs(&host)
		if errList != nil {
			cl.logger.Errorf("Error validating host obj: %+v", errList)
			return i, false, cl.errInvalidHostSmartNICs()
		}
		if len(host.Spec.DSCs) < len(curHost.Spec.DSCs) {
			cl.logger.Errorf("Error deleting dsc by update oper")
			return i, false, cl.errHostDSCNumDecreased()
		}

		// Preiously existed dsc cannot be eliminated
		// DSC order not sensitive
		dscMap := map[cluster.DistributedServiceCardID]bool{}
		for _, dsc := range host.Spec.DSCs {
			dscMap[dsc] = true
		}
		for _, dsc := range curHost.Spec.DSCs {
			_, ok := dscMap[dsc]
			if !ok {
				cl.logger.Errorf("Error: attempt to modify pre-existed dsc: %+v.", dsc)
				return i, false, cl.errHostFieldImmutable(curHost.Name, "Spec.DSCs")
			}
		}

	}

	return i, true, nil
}

func (cl *clusterHooks) validateHostSmartNICs(host *cluster.Host) []error {
	var err []error

	// As of now, up to two SmartNIC per host is supported
	if len(host.Spec.DSCs) < minDSCIDsInHostSpec || len(host.Spec.DSCs) > maxDSCIDsInHostSpec {
		err = append(err, cl.errUnsupportedNumberOfDSCIDs(len(host.Spec.DSCs), minDSCIDsInHostSpec, maxDSCIDsInHostSpec))
	}

	// validate each SmartNIC spec
	for _, sn := range host.Spec.DSCs {
		if (sn.MACAddress == "") == (sn.ID == "") { // both empty or both non-empty
			err = append(err, cl.errInvalidDSCID())
		}
	}
	return err
}

// Validate the Host config
func (cl *clusterHooks) validateHostConfig(i interface{}, ver string, ignStatus, ignoreSpec bool) []error {
	var err []error
	obj, ok := i.(cluster.Host)
	if !ok {
		return []error{fmt.Errorf("incorrect object type, expected host object")}
	}

	// validate the host object name
	if vldtor.HostAddr(obj.Name) != nil {
		cl.logger.Errorf("Invalid host: %s", obj.Name)
		err = append(err, cl.errInvalidHostConfig(obj.Name))
	}

	// validate tenant and namespace
	if obj.Tenant != "" {
		err = append(err, cl.errInvalidTenantConfig())
	}
	if obj.Namespace != "" {
		err = append(err, cl.errInvalidNamespaceConfig())
	}

	if ignoreSpec {
		return err
	}

	// validate the SmartNIC IDs
	err = append(err, cl.validateHostSmartNICs(&obj)...)

	return err
}
