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
			DeploymentTarget: cluster.DSCProfileSpec_HOST.String(),
			FeatureSet:       cluster.DSCProfileSpec_FLOWAWARE.String(),
		},
	}
	transparentFlowaware.ResourceVersion = "15"

	transparentBasenet := cluster.DSCProfile{
		Spec: cluster.DSCProfileSpec{
			DeploymentTarget: cluster.DSCProfileSpec_HOST.String(),
			FeatureSet:       cluster.DSCProfileSpec_SMARTNIC.String(),
		},
	}
	transparentBasenet.ResourceVersion = "15"

	insertionEnforced := cluster.DSCProfile{
		Spec: cluster.DSCProfileSpec{
			DeploymentTarget: cluster.DSCProfileSpec_VIRTUALIZED.String(),
			FeatureSet:       cluster.DSCProfileSpec_FLOWAWARE_FIREWALL.String(),
		},
	}
	insertionEnforced.ResourceVersion = "15"

	defaultProfile := cluster.DSCProfile{
		Spec: cluster.DSCProfileSpec{
			DeploymentTarget: "HOST",
			FeatureSet:       "SMARTNIC",
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

	sampleProfile.Spec.DeploymentTarget = cluster.DSCProfileSpec_HOST.String()
	sampleProfile.Spec.FeatureSet = cluster.DSCProfileSpec_FLOWAWARE_FIREWALL.String()

	//valid profile create
	_, _, err := hooks.DSCProfilePreCommitHook(context.TODO(), kvs, txns, "sample", apiintf.CreateOper, false, sampleProfile)
	AssertOk(t, err, "DSCProfilePrecommitHook did error with valid parameters")

	//Invalud profile create
	sampleProfile.Spec.DeploymentTarget = cluster.DSCProfileSpec_VIRTUALIZED.String()
	sampleProfile.Spec.FeatureSet = cluster.DSCProfileSpec_SMARTNIC.String()

	_, _, err = hooks.DSCProfilePreCommitHook(context.TODO(), kvs, txns, "sample", apiintf.CreateOper, false, sampleProfile)
	Assert(t, err != nil, "DSCProfilePrecommitHook did not return error with invalid parameters")

	sampleProfile.Spec.DeploymentTarget = cluster.DSCProfileSpec_VIRTUALIZED.String()
	sampleProfile.Spec.FeatureSet = cluster.DSCProfileSpec_FLOWAWARE.String()

	_, _, err = hooks.DSCProfilePreCommitHook(context.TODO(), kvs, txns, "sample", apiintf.CreateOper, false, sampleProfile)
	Assert(t, err != nil, "DSCProfilePrecommitHook did not return error with invalid parameters")

	//Valid profile create/update
	sampleProfile.Spec.DeploymentTarget = cluster.DSCProfileSpec_VIRTUALIZED.String()
	sampleProfile.Spec.FeatureSet = cluster.DSCProfileSpec_FLOWAWARE_FIREWALL.String()

	_, _, err = hooks.DSCProfilePreCommitHook(context.TODO(), kvs, txns, "sample", apiintf.CreateOper, false, sampleProfile)
	AssertOk(t, err, "DSCProfilePrecommitHook did return error with valid parameters")

	_, _, err = hooks.DSCProfilePreCommitHook(context.TODO(), kvs, txns, "transparent_basenet", apiintf.UpdateOper, false, sampleProfile)
	AssertOk(t, err, "DSCProfilePrecommitHook did return error with valid parameters")

	_, _, err = hooks.DSCProfilePreCommitHook(context.TODO(), kvs, txns, "transparent_flowaware", apiintf.UpdateOper, false, sampleProfile)
	AssertOk(t, err, "DSCProfilePrecommitHook did return error with valid parameters")

	sampleProfile.Spec.DeploymentTarget = cluster.DSCProfileSpec_HOST.String()
	sampleProfile.Spec.FeatureSet = cluster.DSCProfileSpec_FLOWAWARE.String()

	_, _, err = hooks.DSCProfilePreCommitHook(context.TODO(), kvs, txns, "sample", apiintf.CreateOper, false, sampleProfile)
	AssertOk(t, err, "DSCProfilePrecommitHook did return error with valid parameters")

	_, _, err = hooks.DSCProfilePreCommitHook(context.TODO(), kvs, txns, "transparent_basenet", apiintf.UpdateOper, false, sampleProfile)
	AssertOk(t, err, "DSCProfilePrecommitHook did return error with valid parameters")

	_, _, err = hooks.DSCProfilePreCommitHook(context.TODO(), kvs, txns, "insertion_enforced", apiintf.UpdateOper, false, sampleProfile)
	AssertOk(t, err, "DSCProfilePrecommitHook did return error with valid parameters")

	sampleProfile.Spec.DeploymentTarget = cluster.DSCProfileSpec_HOST.String()
	sampleProfile.Spec.FeatureSet = cluster.DSCProfileSpec_SMARTNIC.String()

	_, _, err = hooks.DSCProfilePreCommitHook(context.TODO(), kvs, txns, "sample", apiintf.CreateOper, false, sampleProfile)
	AssertOk(t, err, "DSCProfilePrecommitHook did return error with valid parameters")

	_, _, err = hooks.DSCProfilePreCommitHook(context.TODO(), kvs, txns, "insertion_enforced", apiintf.UpdateOper, false, sampleProfile)
	AssertOk(t, err, "DSCProfilePrecommitHook did return error with valid parameters")

	_, _, err = hooks.DSCProfilePreCommitHook(context.TODO(), kvs, txns, "transparent_flowaware", apiintf.UpdateOper, false, sampleProfile)
	AssertOk(t, err, "DSCProfilePrecommitHook did return error with valid parameters")

	_, _, err = hooks.DSCProfilePreCommitHook(context.TODO(), kvs, txns, "default", apiintf.DeleteOper, false, defaultProfile)
	Assert(t, err != nil, "DSCProfilePrecommitHook did not return error with valid parameters")
}
