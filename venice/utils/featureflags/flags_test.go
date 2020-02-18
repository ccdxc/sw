package featureflags

import (
	"fmt"
	"reflect"
	"testing"

	"github.com/deckarep/golang-set"

	"github.com/pensando/sw/api/generated/cluster"
	. "github.com/pensando/sw/venice/utils/testutils"
)

func TestFeatureFlags(t *testing.T) {

	// fflags := map[string]cluster.Feature{
	// 	OverlayRouting:         {Value: "true"},
	// 	SubnetSecurityPolicies: {Value: "false"},
	// 	SecurityALG:            {Value: "false"},
	// }

	fflags := []cluster.Feature{
		{FeatureKey: OverlayRouting},
		{FeatureKey: SecurityALG},
		{FeatureKey: SubnetSecurityPolicies},
	}
	// Test without any flags set
	Assert(t, IsOVerlayRoutingEnabled() == false, "expecting overlay routing to be false")
	Assert(t, IsMultiTenantEnabled() == false, "expecting  multi-tenancy to be false")
	Assert(t, IsNetworkSecPolicyEnabled() == false, "expecing network level secpolicy to be false")
	Assert(t, AreALGsEnabled() == true, "expecing ALGs to be true")

	isEqual := func(e, g cluster.LicenseStatus) error {
		me := make(map[string]cluster.FeatureStatus)
		mg := make(map[string]cluster.FeatureStatus)
		for i := range e.Features {
			me[e.Features[i].FeatureKey] = e.Features[i]
		}
		eu := mapset.NewSet()
		for _, v := range e.Unknown {
			eu.Add(v)
		}
		for i := range g.Features {
			mg[g.Features[i].FeatureKey] = g.Features[i]
		}
		gu := mapset.NewSet()
		for _, v := range g.Unknown {
			gu.Add(v)
		}

		if !reflect.DeepEqual(me, mg) {
			return fmt.Errorf("features are not same got[%v] exp: [%v]", g, e)
		}
		if !gu.Equal(eu) {
			return fmt.Errorf("unknown is not same got[%v] exp: [%v]", g, e)
		}
		return nil
	}
	ret, errs := Validate(fflags)
	Assert(t, len(errs) == 0, "expecting 0 error")
	exp := cluster.LicenseStatus{
		Features: []cluster.FeatureStatus{
			{FeatureKey: OverlayRouting, Value: "enabled", Expiry: "never"},
			{FeatureKey: SubnetSecurityPolicies, Value: "enabled", Expiry: "never"},
			{FeatureKey: SecurityALG, Value: "enabled", Expiry: "never"},
		},
	}
	err := isEqual(exp, ret)
	AssertOk(t, err, "Does not match (%s)", err)

	errs = Update(fflags)
	Assert(t, len(errs) == 0, "expecting 1 error")
	Assert(t, IsOVerlayRoutingEnabled() == true, "expecting overlay routing to be true")
	Assert(t, IsMultiTenantEnabled() == true, "expecting multi-tenancy to be true")
	Assert(t, IsNetworkSecPolicyEnabled() == true, "expecing network level secpolicy to be true")
	Assert(t, AreALGsEnabled() == true, "expecing ALGs to be true")

	// Add with flags not specified
	fflags = []cluster.Feature{
		{FeatureKey: OverlayRouting},
		{FeatureKey: SubnetSecurityPolicies},
	}
	ret, errs = Validate(fflags)
	Assert(t, len(errs) == 0, "expecting 1 error")
	exp = cluster.LicenseStatus{
		Features: []cluster.FeatureStatus{
			{FeatureKey: OverlayRouting, Value: "enabled", Expiry: "never"},
			{FeatureKey: SubnetSecurityPolicies, Value: "enabled", Expiry: "never"},
		},
	}
	err = isEqual(exp, ret)
	AssertOk(t, err, "Does not match (%s)", err)

	errs = Update(fflags)
	Assert(t, len(errs) == 0, "expecting 1 error")
	Assert(t, IsOVerlayRoutingEnabled() == true, "expecting overlay routing to be true")
	Assert(t, IsMultiTenantEnabled() == true, "expecting  multi-tenancy to be true")
	Assert(t, IsNetworkSecPolicyEnabled() == true, "expecing network level secpolicy to be false")
	Assert(t, AreALGsEnabled() == false, "expecing ALGs to be true")

	// Add Unknown flags
	fflags = []cluster.Feature{
		{FeatureKey: OverlayRouting},
		{FeatureKey: SubnetSecurityPolicies},
		{FeatureKey: "UnknownFeature"},
	}
	ret, errs = Validate(fflags)
	Assert(t, len(errs) == 0, "expecting 1 error")
	exp = cluster.LicenseStatus{
		Features: []cluster.FeatureStatus{
			{FeatureKey: OverlayRouting, Value: "enabled", Expiry: "never"},
			{FeatureKey: SubnetSecurityPolicies, Value: "enabled", Expiry: "never"},
		},
		Unknown: []string{"UnknownFeature"},
	}
	err = isEqual(exp, ret)
	AssertOk(t, err, "Does not match (%s)", err)

	errs = Update(fflags)
	Assert(t, len(errs) == 0, "expecting 0 error")
	Assert(t, IsOVerlayRoutingEnabled() == true, "expecting overlay routing to be true")
	Assert(t, IsMultiTenantEnabled() == true, "expecting  multi-tenancy to be true")
	Assert(t, IsNetworkSecPolicyEnabled() == true, "expecing network level secpolicy to be false")
	Assert(t, AreALGsEnabled() == false, "expecing ALGs to be true")

	// Add Duplicate flags
	fflags = []cluster.Feature{
		{FeatureKey: OverlayRouting},
		{FeatureKey: SubnetSecurityPolicies},
		{FeatureKey: SubnetSecurityPolicies},
	}
	ret, errs = Validate(fflags)
	Assert(t, len(errs) == 1, "expecting 1 error")
	exp = cluster.LicenseStatus{
		Features: []cluster.FeatureStatus{
			{FeatureKey: OverlayRouting, Value: "enabled", Expiry: "never"},
			{FeatureKey: SubnetSecurityPolicies, Value: "enabled", Expiry: "never"},
		},
	}
	err = isEqual(exp, ret)
	AssertOk(t, err, "Does not match (%s)", err)

	errs = Update(fflags)
	Assert(t, len(errs) == 1, "expecting 1 error got [%d]", len(errs))
	Assert(t, IsOVerlayRoutingEnabled() == true, "expecting overlay routing to be true")
	Assert(t, IsMultiTenantEnabled() == true, "expecting  multi-tenancy to be true")
	Assert(t, IsNetworkSecPolicyEnabled() == true, "expecing network level secpolicy to be false")
	Assert(t, AreALGsEnabled() == false, "expecing ALGs to be true")

	SetInitialized()
	// Make sure there is no holdup
	Initialize("aa", "nodest", nil)
}
