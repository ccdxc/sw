// {C} Copyright 2019 Pensando Systems Inc. All rights reserved.

package impl

import (
	"context"
	"testing"

	"github.com/pensando/sw/api/cache/mocks"
	"github.com/pensando/sw/api/generated/cluster"
	apiintf "github.com/pensando/sw/api/interfaces"
	"github.com/pensando/sw/venice/utils/log"
	"github.com/pensando/sw/venice/utils/runtime"
	. "github.com/pensando/sw/venice/utils/testutils"
)

func TestDSCProfilePreCommitHooks(t *testing.T) {

	hooks := &clusterHooks{
		logger: log.SetConfig(log.GetDefaultConfig("DSCProfile-Hooks-Precommit-Test")),
	}
	txns := &mocks.FakeTxn{}
	kvs := &mocks.FakeKvStore{}

	transparentFlowaware := cluster.DSCProfile{
		Spec: cluster.DSCProfileSpec{
			Features: cluster.FeatureSet{
				InterVMServices: false,
				FlowAware:       true,
				Firewall:        false,
			},
		},
	}
	transparentFlowaware.ResourceVersion = "15"

	transparentBasenet := cluster.DSCProfile{
		Spec: cluster.DSCProfileSpec{
			Features: cluster.FeatureSet{
				InterVMServices: false,
				FlowAware:       false,
				Firewall:        false,
			},
		},
	}
	transparentBasenet.ResourceVersion = "15"

	insertionEnforced := cluster.DSCProfile{
		Spec: cluster.DSCProfileSpec{
			Features: cluster.FeatureSet{
				InterVMServices: true,
				FlowAware:       true,
				Firewall:        true,
			},
		},
	}
	insertionEnforced.ResourceVersion = "15"

	defaultProfile := cluster.DSCProfile{
		Spec: cluster.DSCProfileSpec{
			Features: cluster.FeatureSet{
				InterVMServices: false,
				FlowAware:       false,
				Firewall:        false,
			},
		},
	}
	defaultProfile.Name = "default"
	defaultProfile.ResourceVersion = "15"

	getfn := func(ctx context.Context, key string, into runtime.Object) error {
		log.Infof("key: %s", key)
		switch into.(type) {
		case *cluster.DSCProfile:
			sin := into.(*cluster.DSCProfile)
			switch key {
			case "transparent_flowaware":
				*sin = transparentFlowaware
			case "transparent_basenet":
				*sin = transparentBasenet
			case "insertion_enforced":
				*sin = insertionEnforced
			case "default":
				*sin = defaultProfile
			}
		}
		return nil
	}
	kvs.Getfn = getfn

	var sampleProfile cluster.DSCProfile
	sampleProfile.Name = "sample"

	sampleProfile.Spec.Features = cluster.FeatureSet{
		InterVMServices: false,
		FlowAware:       true,
		Firewall:        true,
	}

	//valid profile create
	_, _, err := hooks.DSCProfilePreCommitHook(context.TODO(), kvs, txns, "sample", apiintf.CreateOper, false, sampleProfile)
	AssertOk(t, err, "DSCProfilePrecommitHook did error with valid parameters")

	//Invalid profile create Insertion Basenet
	sampleProfile.Spec.Features = cluster.FeatureSet{
		InterVMServices: true,
		FlowAware:       false,
		Firewall:        false,
	}

	_, _, err = hooks.DSCProfilePreCommitHook(context.TODO(), kvs, txns, "sample", apiintf.CreateOper, false, sampleProfile)
	Assert(t, err != nil, "DSCProfilePrecommitHook did not return error with invalid parameters")

	//Invalid profile create InsertionFlowaware
	sampleProfile.Spec.Features = cluster.FeatureSet{
		InterVMServices: true,
		FlowAware:       true,
		Firewall:        false,
	}

	_, _, err = hooks.DSCProfilePreCommitHook(context.TODO(), kvs, txns, "sample", apiintf.CreateOper, false, sampleProfile)
	Assert(t, err != nil, "DSCProfilePrecommitHook did not return error with invalid parameters")

	//Valid profile create/update Insertion/Enforced
	sampleProfile.Spec.Features = cluster.FeatureSet{
		InterVMServices: true,
		FlowAware:       true,
		Firewall:        true,
	}

	_, _, err = hooks.DSCProfilePreCommitHook(context.TODO(), kvs, txns, "sample", apiintf.CreateOper, false, sampleProfile)
	AssertOk(t, err, "DSCProfilePrecommitHook did return error with valid parameters")

	_, _, err = hooks.DSCProfilePreCommitHook(context.TODO(), kvs, txns, "transparent_basenet", apiintf.UpdateOper, false, sampleProfile)
	AssertOk(t, err, "DSCProfilePrecommitHook did return error with valid parameters")

	_, _, err = hooks.DSCProfilePreCommitHook(context.TODO(), kvs, txns, "transparent_flowaware", apiintf.UpdateOper, false, sampleProfile)
	AssertOk(t, err, "DSCProfilePrecommitHook did return error with valid parameters")

	//Valid profile create/update Transparent/FlowAware
	sampleProfile.Spec.Features = cluster.FeatureSet{
		InterVMServices: false,
		FlowAware:       true,
		Firewall:        false,
	}

	_, _, err = hooks.DSCProfilePreCommitHook(context.TODO(), kvs, txns, "sample", apiintf.CreateOper, false, sampleProfile)
	AssertOk(t, err, "DSCProfilePrecommitHook did return error with valid parameters")

	_, _, err = hooks.DSCProfilePreCommitHook(context.TODO(), kvs, txns, "transparent_basenet", apiintf.UpdateOper, false, sampleProfile)
	AssertOk(t, err, "DSCProfilePrecommitHook did return error with valid parameters")

	_, _, err = hooks.DSCProfilePreCommitHook(context.TODO(), kvs, txns, "insertion_enforced", apiintf.UpdateOper, false, sampleProfile)
	Assert(t, err != nil, "DSCProfilePrecommitHook did not return error with valid parameters")

	//Valid profile create/update Transparent/Basenet
	sampleProfile.Spec.Features = cluster.FeatureSet{
		InterVMServices: false,
		FlowAware:       false,
		Firewall:        false,
	}

	_, _, err = hooks.DSCProfilePreCommitHook(context.TODO(), kvs, txns, "sample", apiintf.CreateOper, false, sampleProfile)
	AssertOk(t, err, "DSCProfilePrecommitHook did return error with valid parameters")

	_, _, err = hooks.DSCProfilePreCommitHook(context.TODO(), kvs, txns, "insertion_enforced", apiintf.UpdateOper, false, sampleProfile)
	Assert(t, err != nil, "DSCProfilePrecommitHook did not return error with valid parameters")

	_, _, err = hooks.DSCProfilePreCommitHook(context.TODO(), kvs, txns, "transparent_flowaware", apiintf.UpdateOper, false, sampleProfile)
	Assert(t, err != nil, "DSCProfilePrecommitHook did not return error with valid parameters")

	_, _, err = hooks.DSCProfilePreCommitHook(context.TODO(), kvs, txns, "default", apiintf.DeleteOper, false, defaultProfile)
	Assert(t, err != nil, "DSCProfilePrecommitHook did not return error with valid parameters")
}
