package impl

import (
	"context"
	"testing"

	"github.com/pensando/sw/venice/utils/kvstore/store"
	"github.com/pensando/sw/venice/utils/runtime"

	"github.com/pensando/sw/api"
	apiintf "github.com/pensando/sw/api/interfaces"
	"github.com/pensando/sw/venice/utils/log"

	"github.com/pensando/sw/api/generated/rollout"
)

const (
	rolloutName        = "e2e_rollout"
	rolloutSuspendName = "e2e_rollout_suspend"
)

func TestRolloutActionPreCommitHooks(t *testing.T) {

	hooks := &rolloutHooks{
		//svc: apisrvpkg.MustGetAPIServer()
		l: log.SetConfig(log.GetDefaultConfig("Rollout-Precommit-Test")),
	}
	req := rollout.Rollout{
		TypeMeta: api.TypeMeta{
			Kind: "Rollout",
		},
		ObjectMeta: api.ObjectMeta{
			Name: rolloutName,
		},
		Spec: rollout.RolloutSpec{
			Version:                     "2.8",
			ScheduledStartTime:          nil,
			Duration:                    "",
			Strategy:                    "LINEAR",
			MaxParallel:                 0,
			MaxNICFailuresBeforeAbort:   0,
			OrderConstraints:            nil,
			Suspend:                     false,
			SmartNICsOnly:               false,
			SmartNICMustMatchConstraint: true, // hence venice upgrade only
			UpgradeType:                 "Disruptive",
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

	ret, skip, err := hooks.doRolloutAction(context.TODO(), kvs, txn, "", apiintf.CreateOper, false, req)

	if ret == nil || err != nil {
		t.Fatalf("failed exec commitAction [%v](%s)", ret, err)
	}
	if skip == false {
		t.Fatalf("kvwrite enabled on commit")
	}

}
