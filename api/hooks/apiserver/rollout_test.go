package impl

import (
	"context"
	"testing"
	"time"

	"github.com/pensando/sw/api/generated/cluster"
	"github.com/pensando/sw/venice/globals"

	"github.com/pensando/sw/api/generated/apiclient"

	"github.com/pensando/sw/api/interfaces"
	"github.com/pensando/sw/venice/utils/kvstore/store"
	"github.com/pensando/sw/venice/utils/runtime"

	"github.com/gogo/protobuf/types"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/venice/utils/log"

	"github.com/pensando/sw/api/generated/rollout"
)

const (
	rolloutName         = "e2e_rollout"
	rolloutParallelName = "e2e_rollout_parallel"
)

func TestRolloutActionPreCommitHooks(t *testing.T) {

	hooks := &rolloutHooks{
		//svc: apisrvpkg.MustGetAPIServer()
		l: log.SetConfig(log.GetDefaultConfig("Rollout-Precommit-Test")),
	}
	seconds := time.Now().Unix()
	scheduledStartTime := &api.Timestamp{
		Timestamp: types.Timestamp{
			Seconds: seconds + 30, //Add a scheduled rollout with 30 second delay
		},
	}
	scheduledEndTime := &api.Timestamp{
		Timestamp: types.Timestamp{
			Seconds: seconds + 1800, //30 min window
		},
	}
	clusterVersion := &cluster.Version{
		TypeMeta: api.TypeMeta{
			Kind:       "Version",
			APIVersion: "v1",
		},
		ObjectMeta: api.ObjectMeta{
			Name: globals.DefaultVersionName,
		},
		Spec: cluster.VersionSpec{},
		Status: cluster.VersionStatus{
			BuildVersion:        "2.1.3",
			VCSCommit:           "",
			BuildDate:           "",
			RolloutBuildVersion: "",
		},
	}

	roa := rollout.RolloutAction{
		TypeMeta: api.TypeMeta{
			Kind: "RolloutAction",
		},
		ObjectMeta: api.ObjectMeta{
			Name: rolloutName,
		},
		Spec: rollout.RolloutSpec{
			Version:                   "2.8.1",
			ScheduledStartTime:        scheduledStartTime,
			ScheduledEndTime:          scheduledEndTime,
			Strategy:                  rollout.RolloutSpec_LINEAR.String(),
			MaxParallel:               0,
			MaxNICFailuresBeforeAbort: 0,
			OrderConstraints:          nil,
			Suspend:                   false,
			DSCsOnly:                  false,
			DSCMustMatchConstraint:    true, // hence venice upgrade only
			UpgradeType:               rollout.RolloutSpec_Graceful.String(),
		},
		Status: rollout.RolloutActionStatus{
			OperationalState: rollout.RolloutPhase_PROGRESSING.String(),
		},
	}
	req := rollout.Rollout{
		TypeMeta: api.TypeMeta{
			Kind: "Rollout",
		},
		ObjectMeta: api.ObjectMeta{
			Name: rolloutName,
		},
		Spec: rollout.RolloutSpec{
			Version:                   "2.8.1",
			ScheduledStartTime:        scheduledStartTime,
			ScheduledEndTime:          scheduledEndTime,
			Strategy:                  rollout.RolloutSpec_LINEAR.String(),
			MaxParallel:               0,
			MaxNICFailuresBeforeAbort: 0,
			OrderConstraints:          nil,
			Suspend:                   false,
			DSCsOnly:                  false,
			DSCMustMatchConstraint:    true, // hence venice upgrade only
			UpgradeType:               rollout.RolloutSpec_Graceful.String(),
		},
	}

	rolloutCfg := store.Config{
		Type:    store.KVStoreTypeMemkv,
		Codec:   runtime.NewJSONCodec(runtime.NewScheme()),
		Servers: []string{t.Name()},
	}
	kvs, err := store.New(rolloutCfg)
	if err != nil {
		t.Fatalf("unable to create kvstore %s", err)
	}
	txn := kvs.NewTxn()
	key2 := req.MakeKey(string(apiclient.GroupRollout))
	err = kvs.Create(context.TODO(), key2, &req)

	ret, skip, err := hooks.doRolloutAction(context.TODO(), kvs, txn, key2, apiintf.CreateOper, false, req)

	if ret != nil || err == nil {
		t.Fatalf("expected failure but commitAction succeeded [%v](%s)", ret, err)
	}
	if skip == true {
		t.Fatalf("kvwrite not enabled on commit")
	}

	clusterKey := clusterVersion.MakeKey(string(apiclient.GroupCluster))
	err = kvs.Create(context.TODO(), clusterKey, clusterVersion)
	if err != nil {
		t.Fatalf("Failed creation of clusterVersion %v", err)
	}

	ret, skip, err = hooks.doRolloutAction(context.TODO(), kvs, txn, key2, apiintf.CreateOper, false, req)

	if ret == nil || err != nil {
		t.Fatalf("failed exec commitAction [%v](%s)", ret, err)
	}
	if skip == false {
		t.Fatalf("kvwrite enabled on commit")
	}
	key := roa.MakeKey(string(apiclient.GroupRollout))
	err = kvs.Create(context.TODO(), key, &roa)

	req2 := rollout.Rollout{
		TypeMeta: api.TypeMeta{
			Kind: "Rollout",
		},
		ObjectMeta: api.ObjectMeta{
			Name: rolloutParallelName,
		},
		Spec: rollout.RolloutSpec{
			Version:                   "2.8",
			ScheduledStartTime:        scheduledStartTime,
			ScheduledEndTime:          scheduledEndTime,
			Strategy:                  rollout.RolloutSpec_LINEAR.String(),
			MaxParallel:               0,
			MaxNICFailuresBeforeAbort: 0,
			OrderConstraints:          nil,
			Suspend:                   false,
			DSCsOnly:                  false,
			DSCMustMatchConstraint:    true, // hence venice upgrade only
			UpgradeType:               rollout.RolloutSpec_Graceful.String(),
		},
	}

	ret, skip, err = hooks.doRolloutAction(context.TODO(), kvs, txn, "", apiintf.CreateOper, false, req2)

	if ret == nil || err == nil {
		t.Fatalf("failed exec commitAction [%v](%s)", ret, err)
	}
	if skip != false {
		t.Fatalf("kvwrite enabled on commit")
	}
	ret, skip, err = hooks.stopRolloutAction(context.TODO(), kvs, txn, key2, apiintf.CreateOper, false, req)

	if ret == nil || err != nil {
		t.Fatalf("failed exec commitAction [%v](%s)", ret, err)
	}

	req4 := rollout.Rollout{
		TypeMeta: api.TypeMeta{
			Kind: "Rollout",
		},
		ObjectMeta: api.ObjectMeta{
			Name: rolloutName,
		},
		Spec: rollout.RolloutSpec{
			Version:                   "2.8",
			ScheduledStartTime:        scheduledStartTime,
			ScheduledEndTime:          scheduledEndTime,
			Strategy:                  rollout.RolloutSpec_LINEAR.String(),
			MaxParallel:               0,
			MaxNICFailuresBeforeAbort: 0,
			OrderConstraints:          nil,
			Suspend:                   false,
			DSCsOnly:                  false,
			UpgradeType:               rollout.RolloutSpec_Graceful.String(),
		},
	}

	ret, skip, err = hooks.modifyRolloutAction(context.TODO(), kvs, txn, key2, apiintf.CreateOper, false, req4)

	if ret == nil || err != nil {
		t.Fatalf("failed exec commitAction [%v](%s)", ret, err)
	}
	ret, skip, err = hooks.deleteRolloutAction(context.TODO(), kvs, txn, key2, apiintf.DeleteOper, false, req4)

	if ret == nil || err == nil {
		t.Fatalf("failed exec commitAction [%v](%s)", ret, err)
	}
}
