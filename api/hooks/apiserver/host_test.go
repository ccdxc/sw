// {C} Copyright 2019 Pensando Systems Inc. All rights reserved.

package impl

import (
	"context"
	"fmt"
	"testing"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/api/generated/apiclient"
	"github.com/pensando/sw/api/generated/cluster"
	apiintf "github.com/pensando/sw/api/interfaces"
	"github.com/pensando/sw/venice/utils/kvstore/store"
	"github.com/pensando/sw/venice/utils/log"
	"github.com/pensando/sw/venice/utils/runtime"
	. "github.com/pensando/sw/venice/utils/testutils"
)

func makeHostObj(hostName, macAddr, nicName string) *cluster.Host {
	return &cluster.Host{
		ObjectMeta: api.ObjectMeta{
			Name:            hostName,
			ResourceVersion: "1",
		},
		TypeMeta: api.TypeMeta{
			Kind:       "Host",
			APIVersion: "v1",
		},
		Spec: cluster.HostSpec{
			DSCs: []cluster.DistributedServiceCardID{
				{
					MACAddress: macAddr,
					ID:         nicName,
				},
			},
		},
	}
}

func makeHostObjDualDSC(hostName, macAddrA, nicNameA, macAddrB, nicNameB string) *cluster.Host {
	return &cluster.Host{
		ObjectMeta: api.ObjectMeta{
			Name:            hostName,
			ResourceVersion: "1",
		},
		TypeMeta: api.TypeMeta{
			Kind:       "Host",
			APIVersion: "v1",
		},
		Spec: cluster.HostSpec{
			DSCs: []cluster.DistributedServiceCardID{
				{
					MACAddress: macAddrA,
					ID:         nicNameA,
				},
				{
					MACAddress: macAddrB,
					ID:         nicNameB,
				},
			},
		},
	}
}

func TestHostObjectValidation(t *testing.T) {
	hooks := &clusterHooks{
		logger: log.SetConfig(log.GetDefaultConfig("Host-Hooks-Validation-Test")),
	}

	// Negative testcases
	err := hooks.validateHostConfig(nil, "", false, false)
	Assert(t, len(err) > 0, "validateHostConfig did not return error with nil object")
	err = hooks.validateHostConfig(cluster.Cluster{}, "", false, false)
	Assert(t, len(err) > 0, "validateHostConfig did not return error with invalid object")
	err = hooks.validateHostConfig(cluster.Host{}, "", false, false)
	Assert(t, len(err) > 0, "validateHostConfig did not return error with empty object")

	// Testcases for various host configs
	hostTestcases := []struct {
		obj *cluster.Host
		err []error
	}{
		{makeHostObj(".naples1-host.local", "00:01:02:03:04:05", ""), []error{hooks.errInvalidHostConfig(".naples1-host.local")}}, // invalid host name
		{makeHostObj("naples3-host.local", "0102.0304.0506", ""), []error{}},                                                      // valid host object #1
		{makeHostObj("20.5.5.5", "01-02-03-04-05-06", ""), []error{}},                                                             // valid host object #2
		{makeHostObj("20.5.5.5", "01-02-03-04-05-06", "hello-world"), []error{hooks.errInvalidDSCID()}},                           // both DSC ID and MAC Address
		{makeHostObj("20.5.5.5", "", ""), []error{hooks.errInvalidDSCID()}},                                                       // no MAC or Name
		{ // two valid DSC IDs
			&cluster.Host{
				ObjectMeta: api.ObjectMeta{
					Name: "20.5.5.5",
				},
				TypeMeta: api.TypeMeta{
					Kind:       "Host",
					APIVersion: "v1",
				},
				Spec: cluster.HostSpec{
					DSCs: []cluster.DistributedServiceCardID{
						{
							MACAddress: "01-02-03-04-05-06",
						},
						{
							ID: "hello-world",
						},
					},
				},
			},
			[]error{},
		},
		{ // two valid DSC IDs
			&cluster.Host{
				ObjectMeta: api.ObjectMeta{
					Name: "20.5.5.5",
				},
				TypeMeta: api.TypeMeta{
					Kind:       "Host",
					APIVersion: "v1",
				},
				Spec: cluster.HostSpec{
					DSCs: []cluster.DistributedServiceCardID{
						{
							MACAddress: "01-02-03-04-05-06",
						},
						{
							MACAddress: "01-02-03-04-05-07",
						},
					},
				},
			},
			[]error{},
		},
		{ // invalid tenant name
			&cluster.Host{
				ObjectMeta: api.ObjectMeta{
					Name:   "naples3-host.local",
					Tenant: "audi",
				},
				TypeMeta: api.TypeMeta{
					Kind:       "Host",
					APIVersion: "v1",
				},
			},
			[]error{
				hooks.errInvalidTenantConfig(),
				hooks.errUnsupportedNumberOfDSCIDs(0, minDSCIDsInHostSpec, maxDSCIDsInHostSpec),
			},
		},
		{ // invalid namespace
			&cluster.Host{
				ObjectMeta: api.ObjectMeta{
					Name:      "naples3-host.local",
					Namespace: "some",
				},
				TypeMeta: api.TypeMeta{
					Kind:       "Host",
					APIVersion: "v1",
				},
			},
			[]error{
				hooks.errInvalidNamespaceConfig(),
				hooks.errUnsupportedNumberOfDSCIDs(0, minDSCIDsInHostSpec, maxDSCIDsInHostSpec),
			},
		},
		{ // empty DSC ID following valid one
			&cluster.Host{
				ObjectMeta: api.ObjectMeta{
					Name: "20.5.5.5",
				},
				TypeMeta: api.TypeMeta{
					Kind:       "Host",
					APIVersion: "v1",
				},
				Spec: cluster.HostSpec{
					DSCs: []cluster.DistributedServiceCardID{
						{
							MACAddress: "01-02-03-04-05-06",
						},
						{},
					},
				},
			},
			[]error{
				hooks.errInvalidDSCID(),
			},
		},
		{ //too many DSC IDs
			&cluster.Host{
				ObjectMeta: api.ObjectMeta{
					Name: "20.5.5.5",
				},
				TypeMeta: api.TypeMeta{
					Kind:       "Host",
					APIVersion: "v1",
				},
				Spec: cluster.HostSpec{
					DSCs: []cluster.DistributedServiceCardID{
						{
							MACAddress: "01-02-03-04-05-06",
						},
						{
							ID: "hello-world",
						},
						{
							ID: "how-are-you",
						},
					},
				},
			},
			[]error{
				hooks.errUnsupportedNumberOfDSCIDs(3, minDSCIDsInHostSpec, maxDSCIDsInHostSpec),
			},
		},
	}

	// Execute the host config testcases
	for c, tc := range hostTestcases {
		t.Run(tc.obj.Name, func(t *testing.T) {
			err := hooks.validateHostConfig(*tc.obj, "", true, false)
			Assert(t, len(err) == len(tc.err), fmt.Sprintf("Testcase %d: expected and actual errors mismatch. Have: %v, want: %v", c, err, tc.err))
			for i := 0; i < len(tc.err); i++ {
				Assert(t, tc.err[i].Error() == err[i].Error(),
					fmt.Sprintf("Testcase %d, err %d: expected and actual errors mismatch. Have: %v, want: %v", c, i, tc.err[i], err[i]))
			}
		})
	}
}

func TestHostObjectPreCommitHooks(t *testing.T) {
	hooks := &clusterHooks{
		logger: log.SetConfig(log.GetDefaultConfig("Host-Hooks-Preommit-Test")),
	}
	_, result, err := hooks.hostPreCommitHook(context.TODO(), nil, nil, "", apiintf.UpdateOper, false, nil)
	Assert(t, result == true && err != nil, "hostPreCommitHook did not return error with invalid parameters")
	_, result, err = hooks.hostPreCommitHook(context.TODO(), nil, nil, "key", apiintf.UpdateOper, false, "")
	Assert(t, result == true && err != nil, "hostPreCommitHook did not return error with invalid parameters")

	schema := runtime.GetDefaultScheme()
	config := store.Config{Type: store.KVStoreTypeMemkv, Servers: []string{""}, Codec: runtime.NewJSONCodec(schema)}
	kv, err := store.New(config)
	AssertOk(t, err, "Error instantiating KVStore")

	type testCase struct {
		op  apiintf.APIOperType
		obj *cluster.Host
		err error
	}

	baseMAC := "00:01:02:03:04:05"
	otherMAC := "00:01:02:03:04:FF"
	baseName := "hello"
	otherName := "goodbye"
	BaseMACDualDSC := "A4-92-A9-27-95-5F"
	BaseNameDualDSC := "IHaveTwoDSCs"
	OtherMACDualDSC := "4D-6F-38-D6-D1-75"
	OtherNameDualDSC := "IAlsoHaveTwoDSCs"

	testCases := []testCase{
		{apiintf.CreateOper, makeHostObj("testHostMAC", baseMAC, ""), nil},                                                                       // First object with MAC, no conflicts
		{apiintf.CreateOper, makeHostObj("testHostName", "", baseName), nil},                                                                     // First object with Name, no conflicts
		{apiintf.CreateOper, makeHostObj("testHostMAC2", baseMAC, ""), hooks.errHostDSCIDConflicts("testHostMAC2", []string{"testHostMAC"})},     // MAC conflict on create
		{apiintf.CreateOper, makeHostObj("testHostName2", "", baseName), hooks.errHostDSCIDConflicts("testHostName2", []string{"testHostName"})}, // Name conflict on create

		{apiintf.CreateOper, makeHostObj("testHostUpdates", "", otherName), nil},                                                                     // Base object for updates
		{apiintf.UpdateOper, makeHostObj("testHostUpdates", baseMAC, ""), hooks.errHostDSCIDConflicts("testHostUpdates", []string{"testHostMAC"})},   // MAC conflict on update
		{apiintf.UpdateOper, makeHostObj("testHostUpdates", "", baseName), hooks.errHostDSCIDConflicts("testHostUpdates", []string{"testHostName"})}, // Name conflict on update
		{apiintf.UpdateOper, makeHostObj("testHostUpdates", otherMAC, ""), hooks.errHostFieldImmutable("testHostUpdates", "Spec.DSCs")},              // Attempt to modify existed DSC
		{apiintf.UpdateOper, makeHostObj("testHostUpdates", "", ""), hooks.errInvalidHostSmartNICs()},                                                // Invalid new host obj
		{apiintf.UpdateOper, makeHostObj("testHostUpdates", "", "newname"), hooks.errHostFieldImmutable("testHostUpdates", "Spec.DSCs")},             // Attempt to modify existed DSC

		{apiintf.CreateOper, makeHostObj("testHostDualDSCUpdates", BaseMACDualDSC, ""), nil},                                                                                                  // Base object for updates
		{apiintf.UpdateOper, makeHostObjDualDSC("testHostDualDSCUpdates", "", "", "", ""), hooks.errInvalidHostSmartNICs()},                                                                   // Invalid new host obj
		{apiintf.UpdateOper, makeHostObjDualDSC("testHostDualDSCUpdates", baseMAC, "", "", ""), hooks.errHostDSCIDConflicts("testHostDualDSCUpdates", []string{"testHostMAC"})},               // MAC conflict on update
		{apiintf.UpdateOper, makeHostObjDualDSC("testHostDualDSCUpdates", "", baseName, "", ""), hooks.errHostDSCIDConflicts("testHostDualDSCUpdates", []string{"testHostName"})},             // Name conflict on update
		{apiintf.UpdateOper, makeHostObjDualDSC("testHostDualDSCUpdates", BaseMACDualDSC, "", baseMAC, ""), hooks.errHostDSCIDConflicts("testHostDualDSCUpdates", []string{"testHostMAC"})},   // MAC conflict on update
		{apiintf.UpdateOper, makeHostObjDualDSC("testHostDualDSCUpdates", BaseMACDualDSC, "", "", baseName), hooks.errHostDSCIDConflicts("testHostDualDSCUpdates", []string{"testHostName"})}, // Name conflict on update
		{apiintf.UpdateOper, makeHostObjDualDSC("testHostDualDSCUpdates", "", BaseNameDualDSC, BaseMACDualDSC, ""), nil},                                                                      // update order doesn't matter
		{apiintf.UpdateOper, makeHostObjDualDSC("testHostDualDSCUpdates", BaseMACDualDSC, "", "", BaseNameDualDSC), nil},                                                                      // update order doesn't matter

		{apiintf.CreateOper, makeHostObjDualDSC("testHostDualDSCUpdates2", OtherMACDualDSC, "", "", OtherNameDualDSC), nil}, // Base object for updates
		{apiintf.UpdateOper, makeHostObj("testHostDualDSCUpdates2", "", "NewName"), hooks.errHostDSCNumDecreased()},         // eliminate dsc via update oper is not allowed
	}

	ctx := context.TODO()

	for i, tc := range testCases {
		key := tc.obj.MakeKey(string(apiclient.GroupCluster))
		// invoke the hook manually
		_, ok, err := hooks.hostPreCommitHook(ctx, kv, kv.NewTxn(), key, tc.op, false, *tc.obj)
		if !ok && err == nil {
			t.Fatalf("hooks.hostPreCommitHook failed but returned no error, test case: %d", i)
		}
		if tc.err == nil { // op expected to succeed
			AssertOk(t, err, fmt.Sprintf("hostPreCommitHook returned unexpected error, testcase: %d", i))
			// execute the op if it was supposed to succeed
			switch tc.op {
			case apiintf.CreateOper:
				err := kv.Create(ctx, key, tc.obj)
				AssertOk(t, err, fmt.Sprintf("Error creating object in KVStore, testcase: %d", i))
			case apiintf.UpdateOper:
				err := kv.Update(ctx, key, tc.obj)
				AssertOk(t, err, fmt.Sprintf("Error updating object in KVStore, testcase: %d", i))
			default:
				t.Fatalf("Unexpected op!")
			}
		} else {
			Assert(t, err != nil && (err.Error() == tc.err.Error()), fmt.Sprintf("hostPreCommitHook did not return expected error. Testcase: %d, have: \"%v\", want: \"%v\"", i, err, tc.err))
		}
	}
}
