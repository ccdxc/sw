package impl

import (
	"context"
	"testing"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/api/generated/security"
	"github.com/pensando/sw/venice/apiserver"
	"github.com/pensando/sw/venice/apiserver/pkg/mocks"
	"github.com/pensando/sw/venice/utils/log"
	. "github.com/pensando/sw/venice/utils/testutils"
)

// #################### Happy Path Tests ####################
func TestSGPolicyCreateAtTenant(t *testing.T) {
	t.Parallel()
	logConfig := log.GetDefaultConfig("TestSGPolicy")
	s := &sgPolicyHooks{
		svc:    mocks.NewFakeService(),
		logger: log.GetNewLogger(logConfig),
	}
	// create sg policy
	rules := []*security.SGRule{
		{
			Apps:            []string{"tcp/80", "udp/53"},
			Action:          "PERMIT",
			FromIPAddresses: []string{"172.0.0.1", "172.0.0.2", "10.0.0.1/30"},
			ToIPAddresses:   []string{"any"},
		},
	}
	sgp := security.SGPolicy{
		TypeMeta: api.TypeMeta{Kind: "SGPolicy"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      "testpolicy",
		},
		Spec: security.SGPolicySpec{
			AttachTenant: true,
			Rules:        rules,
		},
	}

	_, _, err := s.validateSGPolicy(context.Background(), nil, nil, "", apiserver.CreateOper, sgp)
	AssertOk(t, err, "failed to create sg policy. Error: %v", err)
}

func TestSGPolicyCreateAtSGs(t *testing.T) {
	t.Parallel()
	logConfig := log.GetDefaultConfig("TestSGPolicy")
	s := &sgPolicyHooks{
		svc:    mocks.NewFakeService(),
		logger: log.GetNewLogger(logConfig),
	}
	// create sg policy
	rules := []*security.SGRule{
		{
			Apps:          []string{"tcp/80", "udp/53"},
			Action:        "PERMIT",
			ToIPAddresses: []string{"192.168.1.1/16"},
		},
	}
	sgp := security.SGPolicy{
		TypeMeta: api.TypeMeta{Kind: "SGPolicy"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      "testpolicy",
		},
		Spec: security.SGPolicySpec{
			AttachGroups: []string{"dummySG"},
			Rules:        rules,
		},
	}

	_, _, err := s.validateSGPolicy(context.Background(), nil, nil, "", apiserver.CreateOper, sgp)
	AssertOk(t, err, "failed to create sg policy. Error: %v", err)
}

func TestAttachGroupsWithFromAddresses(t *testing.T) {
	t.Parallel()
	logConfig := log.GetDefaultConfig("TestSGPolicy")
	s := &sgPolicyHooks{
		svc:    mocks.NewFakeService(),
		logger: log.GetNewLogger(logConfig),
	}
	// create sg policy
	rulesWithFrom := []*security.SGRule{
		{
			Apps:            []string{"tcp/80", "udp/53"},
			Action:          "PERMIT",
			FromIPAddresses: []string{"172.0.0.1", "172.0.0.2", "10.0.0.1/30"},
			ToIPAddresses:   []string{"any"},
		},
	}

	sgp := security.SGPolicy{
		TypeMeta: api.TypeMeta{Kind: "SGPolicy"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      "testpolicy",
		},
		Spec: security.SGPolicySpec{
			AttachGroups: []string{"dummySG"},
			Rules:        rulesWithFrom,
		},
	}

	_, _, err := s.validateSGPolicy(context.Background(), nil, nil, "", apiserver.CreateOper, sgp)
	AssertOk(t, err, "SG Policy attaching to the sgs with From Addresses must pass. Error: %v", err)
}

// #################### Corner Case Tests ####################
func TestBothAttachmentPoints(t *testing.T) {
	t.Parallel()
	logConfig := log.GetDefaultConfig("TestSGPolicy")
	s := &sgPolicyHooks{
		svc:    mocks.NewFakeService(),
		logger: log.GetNewLogger(logConfig),
	}
	// create sg policy
	rules := []*security.SGRule{
		{
			Apps:            []string{"tcp/80", "udp/53"},
			Action:          "PERMIT",
			FromIPAddresses: []string{"172.0.0.1", "172.0.0.2", "10.0.0.1/30"},
			ToIPAddresses:   []string{"192.168.1.1/16"},
		},
	}
	sgp := security.SGPolicy{
		TypeMeta: api.TypeMeta{Kind: "SGPolicy"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      "testpolicy",
		},
		Spec: security.SGPolicySpec{
			AttachTenant: true,
			AttachGroups: []string{"dummySG"},
			Rules:        rules,
		},
	}

	_, _, err := s.validateSGPolicy(context.Background(), nil, nil, "", apiserver.CreateOper, sgp)
	Assert(t, err != nil, "sg policy creates specifying both tenant and sg level must fail")
}

func TestMissingAttachmentPoint(t *testing.T) {
	t.Parallel()
	logConfig := log.GetDefaultConfig("TestSGPolicy")
	s := &sgPolicyHooks{
		svc:    mocks.NewFakeService(),
		logger: log.GetNewLogger(logConfig),
	}
	// create sg policy
	rules := []*security.SGRule{
		{
			Apps:            []string{"tcp/80", "udp/53"},
			Action:          "PERMIT",
			FromIPAddresses: []string{"172.0.0.1", "172.0.0.2", "10.0.0.1/30"},
			ToIPAddresses:   []string{"192.168.1.1/16"},
		},
	}
	sgp := security.SGPolicy{
		TypeMeta: api.TypeMeta{Kind: "SGPolicy"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      "testpolicy",
		},
		Spec: security.SGPolicySpec{
			Rules: rules,
		},
	}

	_, _, err := s.validateSGPolicy(context.Background(), nil, nil, "", apiserver.CreateOper, sgp)
	Assert(t, err != nil, "sg policy creates with missing attachment points")
}

func TestInvalidAppProto(t *testing.T) {
	t.Parallel()
	logConfig := log.GetDefaultConfig("TestSGPolicy")
	s := &sgPolicyHooks{
		svc:    mocks.NewFakeService(),
		logger: log.GetNewLogger(logConfig),
	}
	// create sg policy
	rules := []*security.SGRule{
		{
			Apps:            []string{"tcp/80", "foo/53", "tcp/8080"},
			Action:          "PERMIT",
			FromIPAddresses: []string{"172.0.0.1", "172.0.0.2", "10.0.0.1/30"},
			ToIPAddresses:   []string{"192.168.1.1/16"},
		},
	}
	sgp := security.SGPolicy{
		TypeMeta: api.TypeMeta{Kind: "SGPolicy"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      "testpolicy",
		},
		Spec: security.SGPolicySpec{
			AttachTenant: true,
			Rules:        rules,
		},
	}

	_, _, err := s.validateSGPolicy(context.Background(), nil, nil, "", apiserver.CreateOper, sgp)
	Assert(t, err != nil, "sg policy creates with invalid app proto fail")
}

func TestInvalidAppPortEmpty(t *testing.T) {
	t.Parallel()
	logConfig := log.GetDefaultConfig("TestSGPolicy")
	s := &sgPolicyHooks{
		svc:    mocks.NewFakeService(),
		logger: log.GetNewLogger(logConfig),
	}
	// create sg policy
	rules := []*security.SGRule{
		{
			Apps:            []string{"tcp/", "udp/53", "tcp/8080"},
			Action:          "PERMIT",
			FromIPAddresses: []string{"172.0.0.1", "172.0.0.2", "10.0.0.1/30"},
			ToIPAddresses:   []string{"192.168.1.1/16"},
		},
	}
	sgp := security.SGPolicy{
		TypeMeta: api.TypeMeta{Kind: "SGPolicy"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      "testpolicy",
		},
		Spec: security.SGPolicySpec{
			AttachTenant: true,
			Rules:        rules,
		},
	}

	_, _, err := s.validateSGPolicy(context.Background(), nil, nil, "", apiserver.CreateOper, sgp)
	Assert(t, err != nil, "sg policy creates with empty port must fail")
}

func TestInvalidAppPortNonInteger(t *testing.T) {
	t.Parallel()
	logConfig := log.GetDefaultConfig("TestSGPolicy")
	s := &sgPolicyHooks{
		svc:    mocks.NewFakeService(),
		logger: log.GetNewLogger(logConfig),
	}
	// create sg policy
	rules := []*security.SGRule{
		{
			Apps:            []string{"tcp/foo", "udp/53", "tcp/8080"},
			Action:          "PERMIT",
			FromIPAddresses: []string{"172.0.0.1", "172.0.0.2", "10.0.0.1/30"},
			ToIPAddresses:   []string{"192.168.1.1/16"},
		},
	}
	sgp := security.SGPolicy{
		TypeMeta: api.TypeMeta{Kind: "SGPolicy"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      "testpolicy",
		},
		Spec: security.SGPolicySpec{
			AttachTenant: true,
			Rules:        rules,
		},
	}

	_, _, err := s.validateSGPolicy(context.Background(), nil, nil, "", apiserver.CreateOper, sgp)
	Assert(t, err != nil, "sg policy creates with non integer parsable port must fail")
}

func TestInvalidAppPortInvalidPortRange(t *testing.T) {
	t.Parallel()
	logConfig := log.GetDefaultConfig("TestSGPolicy")
	s := &sgPolicyHooks{
		svc:    mocks.NewFakeService(),
		logger: log.GetNewLogger(logConfig),
	}
	// create sg policy
	rulesBelowRange := []*security.SGRule{
		{
			Apps:            []string{"tcp/-2", "udp/53", "tcp/8080"},
			Action:          "PERMIT",
			FromIPAddresses: []string{"172.0.0.1", "172.0.0.2", "10.0.0.1/30"},
			ToIPAddresses:   []string{"192.168.1.1/16"},
		},
	}

	rulesAboveRange := []*security.SGRule{
		{
			Apps:            []string{"tcp/80", "udp/65536", "tcp/8080"},
			Action:          "PERMIT",
			FromIPAddresses: []string{"172.0.0.1", "172.0.0.2", "10.0.0.1/30"},
			ToIPAddresses:   []string{"192.168.1.1/16"},
		},
	}
	sgp := security.SGPolicy{
		TypeMeta: api.TypeMeta{Kind: "SGPolicy"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      "testpolicy",
		},
		Spec: security.SGPolicySpec{
			AttachTenant: true,
			Rules:        rulesBelowRange,
		},
	}

	_, _, err := s.validateSGPolicy(context.Background(), nil, nil, "", apiserver.CreateOper, sgp)
	Assert(t, err != nil, "app ports outside 0 - 64K must fail")

	sgp.Spec.Rules = rulesAboveRange

	_, _, err = s.validateSGPolicy(context.Background(), nil, nil, "", apiserver.CreateOper, sgp)
	Assert(t, err != nil, "app ports outside 0 - 64K must fail")
}

func TestAttachTenantWithMissingToAndFromAddresses(t *testing.T) {
	t.Parallel()
	logConfig := log.GetDefaultConfig("TestSGPolicy")
	s := &sgPolicyHooks{
		svc:    mocks.NewFakeService(),
		logger: log.GetNewLogger(logConfig),
	}
	// create sg policy
	rulesMissingTo := []*security.SGRule{
		{
			Apps:            []string{"tcp/80", "udp/53"},
			Action:          "PERMIT",
			FromIPAddresses: []string{"172.0.0.1", "172.0.0.2", "10.0.0.1/30"},
		},
	}
	rulesMissingFrom := []*security.SGRule{
		{
			Apps:          []string{"tcp/80", "udp/53"},
			Action:        "PERMIT",
			ToIPAddresses: []string{"192.168.1.1/16"},
		},
	}
	rulesMissingBoth := []*security.SGRule{
		{
			Apps:   []string{"tcp/80", "udp/53"},
			Action: "PERMIT",
		},
	}
	sgp := security.SGPolicy{
		TypeMeta: api.TypeMeta{Kind: "SGPolicy"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      "testpolicy",
		},
		Spec: security.SGPolicySpec{
			AttachTenant: true,
			Rules:        rulesMissingTo,
		},
	}

	_, _, err := s.validateSGPolicy(context.Background(), nil, nil, "", apiserver.CreateOper, sgp)
	Assert(t, err != nil, "SG Policy attaching to the tenant with missing To Addresses must fail. Error: %v", err)

	sgp.Spec.Rules = rulesMissingFrom
	_, _, err = s.validateSGPolicy(context.Background(), nil, nil, "", apiserver.CreateOper, sgp)
	Assert(t, err != nil, "SG Policy attaching to the tenant with missing From Addresses must fail. Error: %v", err)

	sgp.Spec.Rules = rulesMissingBoth
	_, _, err = s.validateSGPolicy(context.Background(), nil, nil, "", apiserver.CreateOper, sgp)
	Assert(t, err != nil, "SG Policy attaching to the tenant with missing To and From Addresses must fail. Error: %v", err)
}

func TestInvalidIPAddressOctet(t *testing.T) {
	t.Parallel()
	logConfig := log.GetDefaultConfig("TestSGPolicy")
	s := &sgPolicyHooks{
		svc:    mocks.NewFakeService(),
		logger: log.GetNewLogger(logConfig),
	}
	// create sg policy
	rules := []*security.SGRule{
		{
			Apps:            []string{"tcp/80", "udp/53"},
			Action:          "PERMIT",
			FromIPAddresses: []string{"172.0.0.256", "172.0.0.2/16", "10.0.0.1/30"},
			ToIPAddresses:   []string{"192.168.1.1/16"},
		},
	}
	sgp := security.SGPolicy{
		TypeMeta: api.TypeMeta{Kind: "SGPolicy"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      "testpolicy",
		},
		Spec: security.SGPolicySpec{
			AttachTenant: true,
			Rules:        rules,
		},
	}

	_, _, err := s.validateSGPolicy(context.Background(), nil, nil, "", apiserver.CreateOper, sgp)
	Assert(t, err != nil, "SG Policy with invalid IP Address Octet must fail.  Error: %v", err)

}

func TestInvalidIPAddressCIDR(t *testing.T) {
	t.Parallel()
	logConfig := log.GetDefaultConfig("TestSGPolicy")
	s := &sgPolicyHooks{
		svc:    mocks.NewFakeService(),
		logger: log.GetNewLogger(logConfig),
	}
	// create sg policy
	rules := []*security.SGRule{
		{
			Apps:            []string{"tcp/80", "udp/53"},
			Action:          "PERMIT",
			FromIPAddresses: []string{"172.0.0.1", "172.0.0.2/foo/bar", "10.0.0.1/30"},
			ToIPAddresses:   []string{"192.168.1.1/16"},
		},
	}
	sgp := security.SGPolicy{
		TypeMeta: api.TypeMeta{Kind: "SGPolicy"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      "testpolicy",
		},
		Spec: security.SGPolicySpec{
			AttachTenant: true,
			Rules:        rules,
		},
	}

	_, _, err := s.validateSGPolicy(context.Background(), nil, nil, "", apiserver.CreateOper, sgp)
	Assert(t, err != nil, "SG Policy with invalid CIDR block must fail.  Error: %v", err)
}

func TestInvalidIPAddressRange(t *testing.T) {
	t.Parallel()
	logConfig := log.GetDefaultConfig("TestSGPolicy")
	s := &sgPolicyHooks{
		svc:    mocks.NewFakeService(),
		logger: log.GetNewLogger(logConfig),
	}
	// create sg policy
	rulesInvalidRange := []*security.SGRule{
		{
			Apps:            []string{"tcp/80", "udp/53"},
			Action:          "PERMIT",
			FromIPAddresses: []string{"172.0.0.1-256.256.256.256", "172.0.0.2/22", "10.0.0.1"},
			ToIPAddresses:   []string{"192.168.1.1/16"},
		},
	}
	rulesInvalidRangeMultipleSep := []*security.SGRule{
		{
			Apps:            []string{"tcp/80", "udp/53"},
			Action:          "PERMIT",
			FromIPAddresses: []string{"172.0.0.1-172.0.1.1-172.1.1.1", "172.0.0.2/22", "10.0.0.1"},
			ToIPAddresses:   []string{"192.168.1.1/16"},
		},
	}
	sgp := security.SGPolicy{
		TypeMeta: api.TypeMeta{Kind: "SGPolicy"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      "testpolicy",
		},
		Spec: security.SGPolicySpec{
			AttachTenant: true,
			Rules:        rulesInvalidRange,
		},
	}

	_, _, err := s.validateSGPolicy(context.Background(), nil, nil, "", apiserver.CreateOper, sgp)
	Assert(t, err != nil, "SG Policy with invalid IP Range must fail.  Error: %v", err)

	sgp.Spec.Rules = rulesInvalidRangeMultipleSep
	_, _, err = s.validateSGPolicy(context.Background(), nil, nil, "", apiserver.CreateOper, sgp)
	Assert(t, err != nil, "SG Policy with invalid IP Range must fail.  Error: %v", err)
}

func TestInvalidKeyword(t *testing.T) {
	t.Parallel()
	logConfig := log.GetDefaultConfig("TestSGPolicy")
	s := &sgPolicyHooks{
		svc:    mocks.NewFakeService(),
		logger: log.GetNewLogger(logConfig),
	}
	// create sg policy
	rules := []*security.SGRule{
		{
			Apps:            []string{"tcp/80", "udp/53"},
			Action:          "PERMIT",
			FromIPAddresses: []string{"172.0.0.1", "10.1.0.0/16", "10.0.0.1/30"},
			ToIPAddresses:   []string{"foo"},
		},
	}
	sgp := security.SGPolicy{
		TypeMeta: api.TypeMeta{Kind: "SGPolicy"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      "testpolicy",
		},
		Spec: security.SGPolicySpec{
			AttachTenant: true,
			Rules:        rules,
		},
	}

	_, _, err := s.validateSGPolicy(context.Background(), nil, nil, "", apiserver.CreateOper, sgp)
	Assert(t, err != nil, "SG Policy rules having non any keywords must fail.  Error: %v", err)
}

func TestAttachGroupsWithInvalidIPAddresses(t *testing.T) {
	t.Parallel()
	logConfig := log.GetDefaultConfig("TestSGPolicy")
	s := &sgPolicyHooks{
		svc:    mocks.NewFakeService(),
		logger: log.GetNewLogger(logConfig),
	}
	// create sg policy
	rulesWithFrom := []*security.SGRule{
		{
			Apps:            []string{"tcp/80", "udp/53"},
			Action:          "PERMIT",
			FromIPAddresses: []string{"256.256.256.256", "172.0.0.2", "10.0.0.1/30"},
		},
	}

	sgp := security.SGPolicy{
		TypeMeta: api.TypeMeta{Kind: "SGPolicy"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      "testpolicy",
		},
		Spec: security.SGPolicySpec{
			AttachGroups: []string{"dummySG"},
			Rules:        rulesWithFrom,
		},
	}

	_, _, err := s.validateSGPolicy(context.Background(), nil, nil, "", apiserver.CreateOper, sgp)
	Assert(t, err != nil, "SG Policy attaching to the sgs with invalid IP addresses must fail. Error: %v", err)
}

func TestAppWithMultipleSeparators(t *testing.T) {
	t.Parallel()
	logConfig := log.GetDefaultConfig("TestSGPolicy")
	s := &sgPolicyHooks{
		svc:    mocks.NewFakeService(),
		logger: log.GetNewLogger(logConfig),
	}
	// create sg policy
	rulesWithFrom := []*security.SGRule{
		{
			Apps:            []string{"tcp/80/foo", "udp/53"},
			Action:          "PERMIT",
			FromIPAddresses: []string{"256.256.256.256", "172.0.0.2", "10.0.0.1/30"},
		},
	}

	sgp := security.SGPolicy{
		TypeMeta: api.TypeMeta{Kind: "SGPolicy"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      "testpolicy",
		},
		Spec: security.SGPolicySpec{
			AttachTenant: true,
			Rules:        rulesWithFrom,
		},
	}

	_, _, err := s.validateSGPolicy(context.Background(), nil, nil, "", apiserver.CreateOper, sgp)
	Assert(t, err != nil, "SG Policy with invalid app proto/port formats should fail. Error: %v", err)
}

func TestAppWithInvalidProtocol(t *testing.T) {
	t.Parallel()
	logConfig := log.GetDefaultConfig("TestSGPolicy")
	s := &sgPolicyHooks{
		svc:    mocks.NewFakeService(),
		logger: log.GetNewLogger(logConfig),
	}
	// create sg policy
	rulesWithFrom := []*security.SGRule{
		{
			Apps:            []string{"foo/80", "udp/53"},
			Action:          "PERMIT",
			FromIPAddresses: []string{"256.256.256.256", "172.0.0.2", "10.0.0.1/30"},
		},
	}

	sgp := security.SGPolicy{
		TypeMeta: api.TypeMeta{Kind: "SGPolicy"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      "testpolicy",
		},
		Spec: security.SGPolicySpec{
			AttachTenant: true,
			Rules:        rulesWithFrom,
		},
	}

	_, _, err := s.validateSGPolicy(context.Background(), nil, nil, "", apiserver.CreateOper, sgp)
	Assert(t, err != nil, "SG Policy with invalid app proto/port formats should fail. Error: %v", err)
}

func TestInvalidObjType(t *testing.T) {
	t.Parallel()
	logConfig := log.GetDefaultConfig("TestSGPolicy")
	s := &sgPolicyHooks{
		svc:    mocks.NewFakeService(),
		logger: log.GetNewLogger(logConfig),
	}

	app := security.App{
		TypeMeta: api.TypeMeta{Kind: "App"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      "testApp",
		},
		Spec: security.AppSpec{},
	}

	_, _, err := s.validateSGPolicy(context.Background(), nil, nil, "", apiserver.CreateOper, app)
	Assert(t, err != nil, "Invalid object casts must fail.  Error: %v", err)
}
