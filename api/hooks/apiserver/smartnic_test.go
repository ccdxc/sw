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

func TestSmartNICObjectPreCommitHooks(t *testing.T) {
	hooks := &clusterHooks{
		logger: log.SetConfig(log.GetDefaultConfig("SmartNIC-Hooks-Precommit-Test")),
	}
	_, _, err := hooks.smartNICPreCommitHook(context.TODO(), nil, nil, "", apiintf.UpdateOper, false, nil)
	Assert(t, err != nil, "smartNICPreCommitHook did not return error with invalid parameters")
	_, _, err = hooks.hostPreCommitHook(context.TODO(), nil, nil, "key", apiintf.DeleteOper, false, "")
	Assert(t, err != nil, "smartNICPreCommitHook did not return error with invalid parameters")

	schema := runtime.GetDefaultScheme()
	config := store.Config{Type: store.KVStoreTypeMemkv, Servers: []string{""}, Codec: runtime.NewJSONCodec(schema)}
	kv, err := store.New(config)
	AssertOk(t, err, "Error instantiating KVStore")

	nic := cluster.SmartNIC{
		TypeMeta: api.TypeMeta{
			Kind: "SmartNIC",
		},
		ObjectMeta: api.ObjectMeta{
			Name: "00ae.cd01.0001",
		},
		Spec: cluster.SmartNICSpec{
			Hostname: "hostname",
			IPConfig: &cluster.IPConfig{
				IPAddress: "0.0.0.0/0",
			},
			MgmtMode:    cluster.SmartNICSpec_NETWORK.String(),
			NetworkMode: cluster.SmartNICSpec_OOB.String(),
		},
		Status: cluster.SmartNICStatus{
			AdmissionPhase: "UNKNOWN",
			SerialNum:      "TestNIC",
			PrimaryMAC:     "00ae.cd01.0001",
		},
	}

	decomNICUpd := nic
	decomNICUpd.Spec.MgmtMode = cluster.SmartNICSpec_HOST.String()

	ctx := context.TODO()
	key := nic.MakeKey(string(apiclient.GroupCluster))

	err = kv.Create(ctx, key, &nic)
	AssertOk(t, err, fmt.Sprintf("Error creating object in KVStore"))

	for _, phase := range cluster.SmartNICStatus_Phase_name {
		nic.Status.AdmissionPhase = phase
		err = kv.Update(ctx, key, &nic)
		AssertOk(t, err, fmt.Sprintf("Error updating object in KVStore, phase: %s", phase))

		// Create and update should always go through
		_, r, err := hooks.smartNICPreCommitHook(ctx, kv, kv.NewTxn(), key, apiintf.CreateOper, false, nic)
		Assert(t, r == true && err == nil, "smartNICPreCommitHook returned unexpected error, op: CREATE, phase: %s", phase)
		_, r, err = hooks.smartNICPreCommitHook(ctx, kv, kv.NewTxn(), key, apiintf.UpdateOper, false, nic)
		Assert(t, r == true && err == nil, "smartNICPreCommitHook returned unexpected error, op: UPDATE, phase: %s", phase)

		// MgmtMode change only goes through if phase == ADMITTED
		_, r, err = hooks.smartNICPreCommitHook(ctx, kv, kv.NewTxn(), key, apiintf.UpdateOper, false, &decomNICUpd)
		Assert(t, r == true && (err == nil || phase != cluster.SmartNICStatus_ADMITTED.String()),
			"smartNICPreCommitHook returned unexpected result on mode change, op: UPDATE, phase: %s, err: %v", phase, err)

		// delete only goes through if phase != ADMITTED
		_, r, err = hooks.smartNICPreCommitHook(ctx, kv, kv.NewTxn(), key, apiintf.DeleteOper, false, nil)
		Assert(t, r == true && (err == nil || phase == cluster.SmartNICStatus_ADMITTED.String()),
			"smartNICPreCommitHook returned unexpected result, op: DELETE, phase: %s, err: %v", phase, err)
	}
}
