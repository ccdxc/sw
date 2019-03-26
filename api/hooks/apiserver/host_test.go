// {C} Copyright 2019 Pensando Systems Inc. All rights reserved.

package impl

import (
	"context"
	"fmt"
	"testing"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/api/generated/apiclient"
	"github.com/pensando/sw/api/generated/cluster"
	"github.com/pensando/sw/api/interfaces"
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
			SmartNICs: []cluster.SmartNICID{
				{
					MACAddress: macAddr,
					Name:       nicName,
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
	err := hooks.validateHostConfig(nil, "", false)
	Assert(t, len(err) > 0, "validateHostConfig did not return error with nil object")
	err = hooks.validateHostConfig(cluster.Cluster{}, "", false)
	Assert(t, len(err) > 0, "validateHostConfig did not return error with invalid object")
	err = hooks.validateHostConfig(cluster.Host{}, "", false)
	Assert(t, len(err) > 0, "validateHostConfig did not return error with empty object")

	// Testcases for various host configs
	hostTestcases := []struct {
		obj *cluster.Host
		err []error
	}{
		{makeHostObj(".naples1-host.local", "00:01:02:03:04:05", ""), []error{hooks.errInvalidHostConfig(".naples1-host.local")}}, // invalid host name
		{makeHostObj("naples3-host.local", "0102.0304.0506", ""), []error{}},                                                      // valid host object #1
		{makeHostObj("20.5.5.5", "01-02-03-04-05-06", ""), []error{}},                                                             // valid host object #2
		{makeHostObj("20.5.5.5", "01-02-03-04-05-06", "hello-world"), []error{hooks.errInvalidSmartNIC()}},                        // both SmartNIC name and MAC Address
		{makeHostObj("20.5.5.5", "", ""), []error{hooks.errInvalidSmartNIC()}},                                                    // no MAC or Name
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
				hooks.errUnsupportedNumberOfSmartNICs(0, numExpectedSmartNICsInHostSpec),
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
				hooks.errUnsupportedNumberOfSmartNICs(0, numExpectedSmartNICsInHostSpec),
			},
		},
		{ // empty SmartNIC ID following valid one
			&cluster.Host{
				ObjectMeta: api.ObjectMeta{
					Name: "20.5.5.5",
				},
				TypeMeta: api.TypeMeta{
					Kind:       "Host",
					APIVersion: "v1",
				},
				Spec: cluster.HostSpec{
					SmartNICs: []cluster.SmartNICID{
						{
							MACAddress: "01-02-03-04-05-06",
						},
						{},
					},
				},
			},
			[]error{
				hooks.errUnsupportedNumberOfSmartNICs(2, numExpectedSmartNICsInHostSpec),
				hooks.errInvalidSmartNIC(),
			},
		},
		{ //too many SmartNIC IDs
			&cluster.Host{
				ObjectMeta: api.ObjectMeta{
					Name: "20.5.5.5",
				},
				TypeMeta: api.TypeMeta{
					Kind:       "Host",
					APIVersion: "v1",
				},
				Spec: cluster.HostSpec{
					SmartNICs: []cluster.SmartNICID{
						{
							MACAddress: "01-02-03-04-05-06",
						},
						{
							Name: "hello-world",
						},
					},
				},
			},
			[]error{
				hooks.errUnsupportedNumberOfSmartNICs(2, numExpectedSmartNICsInHostSpec),
			},
		},
	}

	// Execute the host config testcases
	for c, tc := range hostTestcases {
		t.Run(tc.obj.Name, func(t *testing.T) {
			err := hooks.validateHostConfig(*tc.obj, "", true)
			Assert(t, len(err) == len(tc.err), fmt.Sprintf("Testcase %d: expected and actual errors mismatch. Have: %d, want: %d", c, len(err), len(tc.err)))
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
	Assert(t, result == false && err != nil, "hostPreCommitHook did not return error with invalid parameters")
	_, result, err = hooks.hostPreCommitHook(context.TODO(), nil, nil, "key", apiintf.UpdateOper, false, "")
	Assert(t, result == false && err != nil, "hostPreCommitHook did not return error with invalid parameters")

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

	testCases := []testCase{
		{apiintf.CreateOper, makeHostObj("testHostMAC", baseMAC, ""), nil},                                                                              // First object with MAC, no conflicts
		{apiintf.CreateOper, makeHostObj("testHostName", "", baseName), nil},                                                                            // First object with Name, no conflicts
		{apiintf.CreateOper, makeHostObj("testHostMAC2", baseMAC, ""), hooks.errHostSmartNICConflicts("testHostMAC2", []string{"testHostMAC"})},         // MAC conflict on create
		{apiintf.CreateOper, makeHostObj("testHostName2", "", baseName), hooks.errHostSmartNICConflicts("testHostName2", []string{"testHostName"})},     // Name conflict on create
		{apiintf.CreateOper, makeHostObj("testHostUpdates", "", otherName), nil},                                                                        // Base object for updates
		{apiintf.UpdateOper, makeHostObj("testHostUpdates", baseMAC, ""), hooks.errHostSmartNICConflicts("testHostUpdates", []string{"testHostMAC"})},   // MAC conflict on update
		{apiintf.UpdateOper, makeHostObj("testHostUpdates", "", baseName), hooks.errHostSmartNICConflicts("testHostUpdates", []string{"testHostName"})}, // Name conflict on update
		{apiintf.UpdateOper, makeHostObj("testHostUpdates", otherMAC, ""), nil},                                                                         // succesful update from name to MAC
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
