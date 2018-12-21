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
	s := &securityHooks{
		svc:    mocks.NewFakeService(),
		logger: log.GetNewLogger(logConfig),
	}
	// create sg policy
	rules := []security.SGRule{
		{
			ProtoPorts: []security.ProtoPort{
				{
					Protocol: "tcp",
					Ports:    "80",
				},
				{
					Protocol: "udp",
					Ports:    "53",
				},
			},
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

	_, _, err := s.validateSGPolicy(context.Background(), nil, nil, "", apiserver.CreateOper, false, sgp)
	AssertOk(t, err, "failed to create sg policy. Error: %v", err)
}

func TestSGPolicyCreateAtSGs(t *testing.T) {
	t.Parallel()
	logConfig := log.GetDefaultConfig("TestSGPolicy")
	s := &securityHooks{
		svc:    mocks.NewFakeService(),
		logger: log.GetNewLogger(logConfig),
	}
	// create sg policy
	rules := []security.SGRule{
		{
			ProtoPorts: []security.ProtoPort{
				{
					Protocol: "tcp",
					Ports:    "80",
				},
				{
					Protocol: "TCP",
					Ports:    "8000",
				},
				{
					Protocol: "Tcp",
					Ports:    "8000",
				},
				{
					Protocol: "udp",
					Ports:    "53",
				},
				{
					Protocol: "UDP",
					Ports:    "53",
				},
				{
					Protocol: "icmp",
				},
				{
					Protocol: "ICMP",
				},
			},
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

	_, _, err := s.validateSGPolicy(context.Background(), nil, nil, "", apiserver.CreateOper, false, sgp)
	AssertOk(t, err, "failed to create sg policy. Error: %v", err)
}

func TestAttachGroupsWithFromAddresses(t *testing.T) {
	t.Parallel()
	logConfig := log.GetDefaultConfig("TestSGPolicy")
	s := &securityHooks{
		svc:    mocks.NewFakeService(),
		logger: log.GetNewLogger(logConfig),
	}
	// create sg policy
	rulesWithFrom := []security.SGRule{
		{
			ProtoPorts: []security.ProtoPort{
				{
					Protocol: "tcp",
					Ports:    "80",
				},
				{
					Protocol: "udp",
					Ports:    "53",
				},
			},
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

	_, _, err := s.validateSGPolicy(context.Background(), nil, nil, "", apiserver.CreateOper, false, sgp)
	AssertOk(t, err, "SG Policy attaching to the sgs with From Addresses must pass. Error: %v", err)
}

// #################### Corner Case Tests ####################
func TestBothAttachmentPoints(t *testing.T) {
	t.Parallel()
	logConfig := log.GetDefaultConfig("TestSGPolicy")
	s := &securityHooks{
		svc:    mocks.NewFakeService(),
		logger: log.GetNewLogger(logConfig),
	}
	// create sg policy
	rules := []security.SGRule{
		{
			ProtoPorts: []security.ProtoPort{
				{
					Protocol: "tcp",
					Ports:    "80",
				},
				{
					Protocol: "udp",
					Ports:    "53",
				},
			},
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

	_, _, err := s.validateSGPolicy(context.Background(), nil, nil, "", apiserver.CreateOper, false, sgp)
	Assert(t, err != nil, "sg policy creates specifying both tenant and sg level must fail")
}

func TestMissingAttachmentPoint(t *testing.T) {
	t.Parallel()
	logConfig := log.GetDefaultConfig("TestSGPolicy")
	s := &securityHooks{
		svc:    mocks.NewFakeService(),
		logger: log.GetNewLogger(logConfig),
	}
	// create sg policy
	rules := []security.SGRule{
		{
			ProtoPorts: []security.ProtoPort{
				{
					Protocol: "tcp",
					Ports:    "80",
				},
				{
					Protocol: "udp",
					Ports:    "53",
				},
			},
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

	_, _, err := s.validateSGPolicy(context.Background(), nil, nil, "", apiserver.CreateOper, false, sgp)
	Assert(t, err != nil, "sg policy creates with missing attachment points")
}

func TestInvalidAppProto(t *testing.T) {
	t.Parallel()
	logConfig := log.GetDefaultConfig("TestSGPolicy")
	s := &securityHooks{
		svc:    mocks.NewFakeService(),
		logger: log.GetNewLogger(logConfig),
	}
	// create sg policy
	rules := []security.SGRule{
		{
			ProtoPorts: []security.ProtoPort{
				{
					Protocol: "tcp",
					Ports:    "80",
				},
				{
					Protocol: "foo",
					Ports:    "53",
				},
				{
					Protocol: "tcp",
					Ports:    "8000",
				},
			},
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

	_, _, err := s.validateSGPolicy(context.Background(), nil, nil, "", apiserver.CreateOper, false, sgp)
	Assert(t, err != nil, "sg policy creates with invalid app proto fail")
}

func TestAppPortEmpty(t *testing.T) {
	t.Parallel()
	logConfig := log.GetDefaultConfig("TestSGPolicy")
	s := &securityHooks{
		svc:    mocks.NewFakeService(),
		logger: log.GetNewLogger(logConfig),
	}
	// create sg policy
	rules := []security.SGRule{
		{
			ProtoPorts: []security.ProtoPort{
				{
					Protocol: "tcp",
					Ports:    "",
				},
				{
					Protocol: "udp",
					Ports:    "53",
				},
				{
					Protocol: "tcp",
					Ports:    "8000",
				},
			},
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

	_, _, err := s.validateSGPolicy(context.Background(), nil, nil, "", apiserver.CreateOper, false, sgp)
	AssertOk(t, err, "sg policy creates with empty port should suceed")
}

func TestProtocolNumbers(t *testing.T) {
	t.Parallel()
	logConfig := log.GetDefaultConfig("TestSGPolicy")
	s := &securityHooks{
		svc:    mocks.NewFakeService(),
		logger: log.GetNewLogger(logConfig),
	}
	// create sg policy
	rules := []security.SGRule{
		{
			ProtoPorts: []security.ProtoPort{
				{
					Protocol: "tcp",
					Ports:    "80",
				},
				{
					Protocol: "udp",
					Ports:    "53",
				},
				{
					Protocol: "2", // igmp
				},
			},
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

	_, _, err := s.validateSGPolicy(context.Background(), nil, nil, "", apiserver.CreateOper, false, sgp)
	AssertOk(t, err, "sg policy creates with protocol number should suceed")

	// invalid protocol number
	sgp.Spec.Rules[0].ProtoPorts[0].Protocol = "256"
	_, _, err = s.validateSGPolicy(context.Background(), nil, nil, "", apiserver.CreateOper, false, sgp)
	Assert(t, err != nil, "app incorrect protocol number must fail")

	// invalid protocol number
	sgp.Spec.Rules[0].ProtoPorts[0].Protocol = "-1"
	_, _, err = s.validateSGPolicy(context.Background(), nil, nil, "", apiserver.CreateOper, false, sgp)
	Assert(t, err != nil, "app incorrect protocol number must fail")
}

func TestInvalidAppPortNonInteger(t *testing.T) {
	t.Parallel()
	logConfig := log.GetDefaultConfig("TestSGPolicy")
	s := &securityHooks{
		svc:    mocks.NewFakeService(),
		logger: log.GetNewLogger(logConfig),
	}
	// create sg policy
	rules := []security.SGRule{
		{
			ProtoPorts: []security.ProtoPort{
				{
					Protocol: "tcp",
					Ports:    "foo",
				},
				{
					Protocol: "udp",
					Ports:    "53",
				},
				{
					Protocol: "tcp",
					Ports:    "8000",
				},
			},
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

	_, _, err := s.validateSGPolicy(context.Background(), nil, nil, "", apiserver.CreateOper, false, sgp)
	Assert(t, err != nil, "sg policy creates with non integer parsable port must fail")
}

func TestRulePortRanges(t *testing.T) {
	t.Parallel()
	logConfig := log.GetDefaultConfig("TestSGPolicy")
	s := &securityHooks{
		svc:    mocks.NewFakeService(),
		logger: log.GetNewLogger(logConfig),
	}
	// create sg policy
	simplePortRange := []security.SGRule{
		{
			ProtoPorts: []security.ProtoPort{
				{
					Protocol: "tcp",
					Ports:    "8000-8000",
				},
				{
					Protocol: "udp",
					Ports:    "5000-6000",
				},
				{
					Protocol: "tcp",
					Ports:    "5000,5003",
				},
				{
					Protocol: "tcp",
					Ports:    "5000-5001,5003-5004",
				},
				{
					Protocol: "tcp",
					Ports:    "5000,5003-5004",
				},
				{
					Protocol: "tcp",
					Ports:    "0-65535",
				},
			},
			Action:          "PERMIT",
			FromIPAddresses: []string{"172.0.0.1", "172.0.0.2", "10.0.0.1/30"},
			ToIPAddresses:   []string{"192.168.1.1/16"},
		},
	}

	rulesIncorrectPortFormat := []security.SGRule{
		{
			ProtoPorts: []security.ProtoPort{
				{
					Protocol: "tcp",
					Ports:    "80-90-100",
				},
				{
					Protocol: "udp",
					Ports:    "655",
				},
			},
			Action:          "PERMIT",
			FromIPAddresses: []string{"172.0.0.1", "172.0.0.2", "10.0.0.1/30"},
			ToIPAddresses:   []string{"192.168.1.1/16"},
		},
	}
	rulesIncorrectPortRangeFormat := []security.SGRule{
		{
			ProtoPorts: []security.ProtoPort{
				{
					Protocol: "tcp",
					Ports:    "80-",
				},
				{
					Protocol: "udp",
					Ports:    "655",
				},
			},
			Action:          "PERMIT",
			FromIPAddresses: []string{"172.0.0.1", "172.0.0.2", "10.0.0.1/30"},
			ToIPAddresses:   []string{"192.168.1.1/16"},
		},
	}
	rulesIncorrectPortValFormat := []security.SGRule{
		{
			ProtoPorts: []security.ProtoPort{
				{
					Protocol: "tcp",
					Ports:    "80,",
				},
				{
					Protocol: "udp",
					Ports:    "655,foo",
				},
			},
			Action:          "PERMIT",
			FromIPAddresses: []string{"172.0.0.1", "172.0.0.2", "10.0.0.1/30"},
			ToIPAddresses:   []string{"192.168.1.1/16"},
		},
	}
	rulesInvalidPortRange := []security.SGRule{
		{
			ProtoPorts: []security.ProtoPort{
				{
					Protocol: "tcp",
					Ports:    "6000-5000",
				},
				{
					Protocol: "udp",
					Ports:    "655",
				},
			},
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
			Rules:        simplePortRange,
		},
	}

	_, _, err := s.validateSGPolicy(context.Background(), nil, nil, "", apiserver.CreateOper, false, sgp)
	AssertOk(t, err, "app range config failed")

	sgp.Spec.Rules = rulesIncorrectPortFormat
	_, _, err = s.validateSGPolicy(context.Background(), nil, nil, "", apiserver.CreateOper, false, sgp)
	Assert(t, err != nil, "app incorrect port range must fail")

	sgp.Spec.Rules = rulesIncorrectPortRangeFormat
	_, _, err = s.validateSGPolicy(context.Background(), nil, nil, "", apiserver.CreateOper, false, sgp)
	Assert(t, err != nil, "app incorrect port range format must fail")

	sgp.Spec.Rules = rulesIncorrectPortValFormat
	_, _, err = s.validateSGPolicy(context.Background(), nil, nil, "", apiserver.CreateOper, false, sgp)
	Assert(t, err != nil, "app incorrect port value format must fail")

	sgp.Spec.Rules = rulesInvalidPortRange
	_, _, err = s.validateSGPolicy(context.Background(), nil, nil, "", apiserver.CreateOper, false, sgp)
	Assert(t, err != nil, "app invalid port range must fail")
}

func TestInvalidAppPortInvalidPortRange(t *testing.T) {
	t.Parallel()
	logConfig := log.GetDefaultConfig("TestSGPolicy")
	s := &securityHooks{
		svc:    mocks.NewFakeService(),
		logger: log.GetNewLogger(logConfig),
	}
	// create sg policy
	rulesBelowRange := []security.SGRule{
		{
			ProtoPorts: []security.ProtoPort{
				{
					Protocol: "tcp",
					Ports:    "-2",
				},
				{
					Protocol: "udp",
					Ports:    "53",
				},
				{
					Protocol: "tcp",
					Ports:    "8000",
				},
			},
			Action:          "PERMIT",
			FromIPAddresses: []string{"172.0.0.1", "172.0.0.2", "10.0.0.1/30"},
			ToIPAddresses:   []string{"192.168.1.1/16"},
		},
	}

	rulesAboveRange := []security.SGRule{
		{
			ProtoPorts: []security.ProtoPort{
				{
					Protocol: "tcp",
					Ports:    "80",
				},
				{
					Protocol: "udp",
					Ports:    "65536",
				},
				{
					Protocol: "tcp",
					Ports:    "8000",
				},
			},
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

	_, _, err := s.validateSGPolicy(context.Background(), nil, nil, "", apiserver.CreateOper, false, sgp)
	Assert(t, err != nil, "app ports outside 0 - 64K must fail")

	sgp.Spec.Rules = rulesAboveRange

	_, _, err = s.validateSGPolicy(context.Background(), nil, nil, "", apiserver.CreateOper, false, sgp)
	Assert(t, err != nil, "app ports outside 0 - 64K must fail")
}

func TestAttachTenantWithMissingToAndFromAddresses(t *testing.T) {
	t.Parallel()
	logConfig := log.GetDefaultConfig("TestSGPolicy")
	s := &securityHooks{
		svc:    mocks.NewFakeService(),
		logger: log.GetNewLogger(logConfig),
	}
	// create sg policy
	rulesMissingTo := []security.SGRule{
		{
			ProtoPorts: []security.ProtoPort{
				{
					Protocol: "tcp",
					Ports:    "80",
				},
				{
					Protocol: "udp",
					Ports:    "53",
				},
			},
			Action:          "PERMIT",
			FromIPAddresses: []string{"172.0.0.1", "172.0.0.2", "10.0.0.1/30"},
		},
	}
	rulesMissingFrom := []security.SGRule{
		{
			ProtoPorts: []security.ProtoPort{
				{
					Protocol: "tcp",
					Ports:    "80",
				},
				{
					Protocol: "udp",
					Ports:    "53",
				},
			},
			Action:        "PERMIT",
			ToIPAddresses: []string{"192.168.1.1/16"},
		},
	}
	rulesMissingBoth := []security.SGRule{
		{
			ProtoPorts: []security.ProtoPort{
				{
					Protocol: "tcp",
					Ports:    "80",
				},
				{
					Protocol: "udp",
					Ports:    "53",
				},
			},
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

	_, _, err := s.validateSGPolicy(context.Background(), nil, nil, "", apiserver.CreateOper, false, sgp)
	Assert(t, err != nil, "SG Policy attaching to the tenant with missing To Addresses must fail. Error: %v", err)

	sgp.Spec.Rules = rulesMissingFrom
	_, _, err = s.validateSGPolicy(context.Background(), nil, nil, "", apiserver.CreateOper, false, sgp)
	Assert(t, err != nil, "SG Policy attaching to the tenant with missing From Addresses must fail. Error: %v", err)

	sgp.Spec.Rules = rulesMissingBoth
	_, _, err = s.validateSGPolicy(context.Background(), nil, nil, "", apiserver.CreateOper, false, sgp)
	Assert(t, err != nil, "SG Policy attaching to the tenant with missing To and From Addresses must fail. Error: %v", err)
}

func TestInvalidIPAddressOctet(t *testing.T) {
	t.Parallel()
	logConfig := log.GetDefaultConfig("TestSGPolicy")
	s := &securityHooks{
		svc:    mocks.NewFakeService(),
		logger: log.GetNewLogger(logConfig),
	}
	// create sg policy
	rules := []security.SGRule{
		{
			ProtoPorts: []security.ProtoPort{
				{
					Protocol: "tcp",
					Ports:    "80",
				},
				{
					Protocol: "udp",
					Ports:    "53",
				},
			},
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

	_, _, err := s.validateSGPolicy(context.Background(), nil, nil, "", apiserver.CreateOper, false, sgp)
	Assert(t, err != nil, "SG Policy with invalid IP Address Octet must fail.  Error: %v", err)

}

func TestInvalidIPAddressCIDR(t *testing.T) {
	t.Parallel()
	logConfig := log.GetDefaultConfig("TestSGPolicy")
	s := &securityHooks{
		svc:    mocks.NewFakeService(),
		logger: log.GetNewLogger(logConfig),
	}
	// create sg policy
	rules := []security.SGRule{
		{
			ProtoPorts: []security.ProtoPort{
				{
					Protocol: "tcp",
					Ports:    "80",
				},
				{
					Protocol: "udp",
					Ports:    "53",
				},
			},
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

	_, _, err := s.validateSGPolicy(context.Background(), nil, nil, "", apiserver.CreateOper, false, sgp)
	Assert(t, err != nil, "SG Policy with invalid CIDR block must fail.  Error: %v", err)
}

func TestInvalidIPAddressRange(t *testing.T) {
	t.Parallel()
	logConfig := log.GetDefaultConfig("TestSGPolicy")
	s := &securityHooks{
		svc:    mocks.NewFakeService(),
		logger: log.GetNewLogger(logConfig),
	}
	// create sg policy
	rulesInvalidRange := []security.SGRule{
		{
			ProtoPorts: []security.ProtoPort{
				{
					Protocol: "tcp",
					Ports:    "80",
				},
				{
					Protocol: "udp",
					Ports:    "53",
				},
			},
			Action:          "PERMIT",
			FromIPAddresses: []string{"172.0.0.1-256.256.256.256", "172.0.0.2/22", "10.0.0.1"},
			ToIPAddresses:   []string{"192.168.1.1/16"},
		},
	}
	rulesInvalidRangeMultipleSep := []security.SGRule{
		{
			ProtoPorts: []security.ProtoPort{
				{
					Protocol: "tcp",
					Ports:    "80",
				},
				{
					Protocol: "udp",
					Ports:    "53",
				},
			},
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

	_, _, err := s.validateSGPolicy(context.Background(), nil, nil, "", apiserver.CreateOper, false, sgp)
	Assert(t, err != nil, "SG Policy with invalid IP Range must fail.  Error: %v", err)

	sgp.Spec.Rules = rulesInvalidRangeMultipleSep
	_, _, err = s.validateSGPolicy(context.Background(), nil, nil, "", apiserver.CreateOper, false, sgp)
	Assert(t, err != nil, "SG Policy with invalid IP Range must fail.  Error: %v", err)
}

func TestInvalidKeyword(t *testing.T) {
	t.Parallel()
	logConfig := log.GetDefaultConfig("TestSGPolicy")
	s := &securityHooks{
		svc:    mocks.NewFakeService(),
		logger: log.GetNewLogger(logConfig),
	}
	// create sg policy
	rules := []security.SGRule{
		{
			ProtoPorts: []security.ProtoPort{
				{
					Protocol: "tcp",
					Ports:    "80",
				},
				{
					Protocol: "udp",
					Ports:    "53",
				},
			},
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

	_, _, err := s.validateSGPolicy(context.Background(), nil, nil, "", apiserver.CreateOper, false, sgp)
	Assert(t, err != nil, "SG Policy rules having non any keywords must fail.  Error: %v", err)
}

func TestAttachGroupsWithInvalidIPAddresses(t *testing.T) {
	t.Parallel()
	logConfig := log.GetDefaultConfig("TestSGPolicy")
	s := &securityHooks{
		svc:    mocks.NewFakeService(),
		logger: log.GetNewLogger(logConfig),
	}
	// create sg policy
	rulesWithFrom := []security.SGRule{
		{
			ProtoPorts: []security.ProtoPort{
				{
					Protocol: "tcp",
					Ports:    "80",
				},
				{
					Protocol: "udp",
					Ports:    "53",
				},
			},
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

	_, _, err := s.validateSGPolicy(context.Background(), nil, nil, "", apiserver.CreateOper, false, sgp)
	Assert(t, err != nil, "SG Policy attaching to the sgs with invalid IP addresses must fail. Error: %v", err)
}

func TestAppWithMultipleSeparators(t *testing.T) {
	t.Parallel()
	logConfig := log.GetDefaultConfig("TestSGPolicy")
	s := &securityHooks{
		svc:    mocks.NewFakeService(),
		logger: log.GetNewLogger(logConfig),
	}
	// create sg policy
	rulesWithFrom := []security.SGRule{
		{
			ProtoPorts: []security.ProtoPort{
				{
					Protocol: "tcp",
					Ports:    "80-90-100",
				},
				{
					Protocol: "udp",
					Ports:    "53",
				},
			},
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

	_, _, err := s.validateSGPolicy(context.Background(), nil, nil, "", apiserver.CreateOper, false, sgp)
	Assert(t, err != nil, "SG Policy with invalid app proto/port formats should fail. Error: %v", err)
}

func TestAppWithInvalidProtocol(t *testing.T) {
	t.Parallel()
	logConfig := log.GetDefaultConfig("TestSGPolicy")
	s := &securityHooks{
		svc:    mocks.NewFakeService(),
		logger: log.GetNewLogger(logConfig),
	}
	// create sg policy
	rulesWithFrom := []security.SGRule{
		{
			ProtoPorts: []security.ProtoPort{
				{
					Protocol: "foo",
					Ports:    "80",
				},
				{
					Protocol: "udp",
					Ports:    "53",
				},
			},
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

	_, _, err := s.validateSGPolicy(context.Background(), nil, nil, "", apiserver.CreateOper, false, sgp)
	Assert(t, err != nil, "SG Policy with invalid app proto/port formats should fail. Error: %v", err)
}

func TestInvalidObjType(t *testing.T) {
	t.Parallel()
	logConfig := log.GetDefaultConfig("TestSGPolicy")
	s := &securityHooks{
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

	_, _, err := s.validateSGPolicy(context.Background(), nil, nil, "", apiserver.CreateOper, false, app)
	Assert(t, err != nil, "Invalid object casts must fail.  Error: %v", err)
}

func TestAppProtoPortConfig(t *testing.T) {
	t.Parallel()
	logConfig := log.GetDefaultConfig("TestApp")
	s := &securityHooks{
		svc:    mocks.NewFakeService(),
		logger: log.GetNewLogger(logConfig),
	}

	// app with protocol port
	app := security.App{
		TypeMeta: api.TypeMeta{Kind: "App"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      "testApp",
		},
		Spec: security.AppSpec{
			ProtoPorts: []security.ProtoPort{
				{
					Protocol: "tcp",
					Ports:    "20",
				},
			},
		},
	}

	_, _, err := s.validateApp(context.Background(), nil, nil, "", apiserver.CreateOper, false, app)
	AssertOk(t, err, "failed to create app. Error: %v", err)

	// empty protocol-port
	app = security.App{
		TypeMeta: api.TypeMeta{Kind: "App"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      "testApp",
		},
		Spec: security.AppSpec{
			ProtoPorts: []security.ProtoPort{},
		},
	}

	_, _, err = s.validateApp(context.Background(), nil, nil, "", apiserver.CreateOper, false, app)
	AssertOk(t, err, "failed to create app with empty protocol. Error: %v", err)

	// protocol number
	app = security.App{
		TypeMeta: api.TypeMeta{Kind: "App"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      "testApp",
		},
		Spec: security.AppSpec{
			ProtoPorts: []security.ProtoPort{
				{
					Protocol: "6",
				},
			},
		},
	}

	_, _, err = s.validateApp(context.Background(), nil, nil, "", apiserver.CreateOper, false, app)
	AssertOk(t, err, "failed to create app with protocol number. Error: %v", err)

	// invalid protocol string
	app = security.App{
		TypeMeta: api.TypeMeta{Kind: "App"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      "testApp",
		},
		Spec: security.AppSpec{
			ProtoPorts: []security.ProtoPort{
				{
					Protocol: "foo",
				},
			},
		},
	}

	_, _, err = s.validateApp(context.Background(), nil, nil, "", apiserver.CreateOper, false, app)
	Assert(t, err != nil, "Invalid protocol name must fail.  Error: %v", err)

	// invalid protocol number
	app = security.App{
		TypeMeta: api.TypeMeta{Kind: "App"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      "testApp",
		},
		Spec: security.AppSpec{
			ProtoPorts: []security.ProtoPort{
				{
					Protocol: "300",
				},
			},
		},
	}

	_, _, err = s.validateApp(context.Background(), nil, nil, "", apiserver.CreateOper, false, app)
	Assert(t, err != nil, "Invalid protocol number must fail.  Error: %v", err)

	// app with port range
	app = security.App{
		TypeMeta: api.TypeMeta{Kind: "App"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      "testApp",
		},
		Spec: security.AppSpec{
			ProtoPorts: []security.ProtoPort{
				{
					Protocol: "tcp",
					Ports:    "20-30",
				},
			},
		},
	}

	_, _, err = s.validateApp(context.Background(), nil, nil, "", apiserver.CreateOper, false, app)
	AssertOk(t, err, "failed to create app. Error: %v", err)

	// invalid port range
	app = security.App{
		TypeMeta: api.TypeMeta{Kind: "App"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      "testApp",
		},
		Spec: security.AppSpec{
			ProtoPorts: []security.ProtoPort{
				{
					Protocol: "tcp",
					Ports:    "foo",
				},
			},
		},
	}

	_, _, err = s.validateApp(context.Background(), nil, nil, "", apiserver.CreateOper, false, app)
	Assert(t, err != nil, "Invalid protocol name must fail.  Error: %v", err)

	// invalid port range
	app = security.App{
		TypeMeta: api.TypeMeta{Kind: "App"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      "testApp",
		},
		Spec: security.AppSpec{
			ProtoPorts: []security.ProtoPort{
				{
					Protocol: "tcp",
					Ports:    "30-20",
				},
			},
		},
	}

	_, _, err = s.validateApp(context.Background(), nil, nil, "", apiserver.CreateOper, false, app)
	Assert(t, err != nil, "Invalid protocol name must fail.  Error: %v", err)

	// invalid port number
	app = security.App{
		TypeMeta: api.TypeMeta{Kind: "App"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      "testApp",
		},
		Spec: security.AppSpec{
			ProtoPorts: []security.ProtoPort{
				{
					Protocol: "tcp",
					Ports:    "100000",
				},
			},
		},
	}

	_, _, err = s.validateApp(context.Background(), nil, nil, "", apiserver.CreateOper, false, app)
	Assert(t, err != nil, "Invalid protocol name must fail.  Error: %v", err)
}

func TestAppAlgConfig(t *testing.T) {
	t.Parallel()
	logConfig := log.GetDefaultConfig("TestApp")
	s := &securityHooks{
		svc:    mocks.NewFakeService(),
		logger: log.GetNewLogger(logConfig),
	}

	// app with basic alg
	app := security.App{
		TypeMeta: api.TypeMeta{Kind: "App"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      "testApp",
		},
		Spec: security.AppSpec{
			ProtoPorts: []security.ProtoPort{
				{
					Protocol: "tcp",
					Ports:    "21",
				},
			},
			ALG: &security.ALG{
				Type: "FTP",
				FtpAlg: &security.FtpAlg{
					AllowMismatchIPAddress: true,
				},
			},
		},
	}

	_, _, err := s.validateApp(context.Background(), nil, nil, "", apiserver.CreateOper, false, app)
	AssertOk(t, err, "failed to create app. Error: %v", err)

	// App without any ALG
	app = security.App{
		TypeMeta: api.TypeMeta{Kind: "App"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      "testApp",
		},
		Spec: security.AppSpec{},
	}

	_, _, err = s.validateApp(context.Background(), nil, nil, "", apiserver.CreateOper, false, app)
	AssertOk(t, err, "failed to create app with empty ALG. Error: %v", err)

	// ICMP ALG
	app = security.App{
		TypeMeta: api.TypeMeta{Kind: "App"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      "testApp",
		},
		Spec: security.AppSpec{
			ProtoPorts: []security.ProtoPort{
				{
					Protocol: "icmp",
				},
			},
			ALG: &security.ALG{
				Type: "ICMP",
				IcmpAlg: &security.IcmpAlg{
					Type: "1",
					Code: "0",
				},
			},
		},
	}

	_, _, err = s.validateApp(context.Background(), nil, nil, "", apiserver.CreateOper, false, app)
	AssertOk(t, err, "failed to create app with ICMP ALG config. Error: %v", err)

	// ICMP ALG
	app = security.App{
		TypeMeta: api.TypeMeta{Kind: "App"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      "testApp",
		},
		Spec: security.AppSpec{
			ProtoPorts: []security.ProtoPort{
				{
					Protocol: "icmp",
				},
			},
			ALG: &security.ALG{
				Type: "ICMP",
			},
		},
	}

	_, _, err = s.validateApp(context.Background(), nil, nil, "", apiserver.CreateOper, false, app)
	AssertOk(t, err, "failed to create app with ICMP ALG config without Specific Type. Error: %v", err)

	// DNS ALG
	app = security.App{
		TypeMeta: api.TypeMeta{Kind: "App"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      "testApp",
		},
		Spec: security.AppSpec{
			ProtoPorts: []security.ProtoPort{
				{
					Protocol: "udp",
				},
			},
			ALG: &security.ALG{
				Type: "DNS",
				DnsAlg: &security.DnsAlg{
					DropMultiQuestionPackets: true,
				},
			},
		},
	}

	_, _, err = s.validateApp(context.Background(), nil, nil, "", apiserver.CreateOper, false, app)
	AssertOk(t, err, "failed to create app with DNS ALG config. Error: %v", err)

	// SunrpcAlg ALG
	app = security.App{
		TypeMeta: api.TypeMeta{Kind: "App"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      "testApp",
		},
		Spec: security.AppSpec{
			ProtoPorts: []security.ProtoPort{
				{
					Protocol: "udp",
				},
			},
			ALG: &security.ALG{
				Type: "SunRPC",
				SunrpcAlg: &security.SunrpcAlg{
					ProgramID: "1",
				},
			},
		},
	}

	_, _, err = s.validateApp(context.Background(), nil, nil, "", apiserver.CreateOper, false, app)
	AssertOk(t, err, "failed to create app with SunrpcAlg ALG config. Error: %v", err)

	// MsrpcAlg ALG
	app = security.App{
		TypeMeta: api.TypeMeta{Kind: "App"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      "testApp",
		},
		Spec: security.AppSpec{
			ProtoPorts: []security.ProtoPort{
				{
					Protocol: "icmp",
				},
			},
			ALG: &security.ALG{
				Type: "MSRPC",
				MsrpcAlg: &security.MsrpcAlg{
					ProgramUUID: "1",
				},
			},
		},
	}

	_, _, err = s.validateApp(context.Background(), nil, nil, "", apiserver.CreateOper, false, app)
	AssertOk(t, err, "failed to create app with MsrpcAlg ALG config. Error: %v", err)

	// invalid ALG type
	app = security.App{
		TypeMeta: api.TypeMeta{Kind: "App"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      "testApp",
		},
		Spec: security.AppSpec{
			ALG: &security.ALG{
				Type: "foo",
			},
		},
	}

	_, _, err = s.validateApp(context.Background(), nil, nil, "", apiserver.CreateOper, false, app)
	Assert(t, err != nil, "Invalid protocol name must fail.  Error: %v", err)

	// invalid ICMP type
	app = security.App{
		TypeMeta: api.TypeMeta{Kind: "App"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      "testApp",
		},
		Spec: security.AppSpec{
			ProtoPorts: []security.ProtoPort{
				{
					Protocol: "icmp",
				},
			},
			ALG: &security.ALG{
				Type: "ICMP",
				IcmpAlg: &security.IcmpAlg{
					Type: "foo",
					Code: "0",
				},
			},
		},
	}

	_, _, err = s.validateApp(context.Background(), nil, nil, "", apiserver.CreateOper, false, app)
	Assert(t, err != nil, "Invalid ICMP type must fail.  Error: %v", err)

	// invalid ICMP type
	app = security.App{
		TypeMeta: api.TypeMeta{Kind: "App"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      "testApp",
		},
		Spec: security.AppSpec{
			ProtoPorts: []security.ProtoPort{
				{
					Protocol: "icmp",
				},
			},
			ALG: &security.ALG{
				Type: "ICMP",
				IcmpAlg: &security.IcmpAlg{
					Type: "300",
					Code: "0",
				},
			},
		},
	}

	_, _, err = s.validateApp(context.Background(), nil, nil, "", apiserver.CreateOper, false, app)
	Assert(t, err != nil, "Invalid ICMP type must fail.  Error: %v", err)

	// invalid ICMP code
	app = security.App{
		TypeMeta: api.TypeMeta{Kind: "App"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      "testApp",
		},
		Spec: security.AppSpec{
			ProtoPorts: []security.ProtoPort{
				{
					Protocol: "icmp",
				},
			},
			ALG: &security.ALG{
				Type: "ICMP",
				IcmpAlg: &security.IcmpAlg{
					Type: "1",
					Code: "foo",
				},
			},
		},
	}

	_, _, err = s.validateApp(context.Background(), nil, nil, "", apiserver.CreateOper, false, app)
	Assert(t, err != nil, "Invalid ICMP code must fail.  Error: %v", err)

	// invalid ICMP code
	app = security.App{
		TypeMeta: api.TypeMeta{Kind: "App"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      "testApp",
		},
		Spec: security.AppSpec{
			ProtoPorts: []security.ProtoPort{
				{
					Protocol: "icmp",
				},
			},
			ALG: &security.ALG{
				Type: "ICMP",
				IcmpAlg: &security.IcmpAlg{
					Type: "1",
					Code: "20",
				},
			},
		},
	}

	_, _, err = s.validateApp(context.Background(), nil, nil, "", apiserver.CreateOper, false, app)
	Assert(t, err != nil, "Invalid ICMP code must fail.  Error: %v", err)

	// ICMP LG with Ftp config
	app = security.App{
		TypeMeta: api.TypeMeta{Kind: "App"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      "testApp",
		},
		Spec: security.AppSpec{
			ProtoPorts: []security.ProtoPort{
				{
					Protocol: "icmp",
				},
			},
			ALG: &security.ALG{
				Type: "ICMP",
				FtpAlg: &security.FtpAlg{
					AllowMismatchIPAddress: true,
				},
			},
		},
	}

	_, _, err = s.validateApp(context.Background(), nil, nil, "", apiserver.CreateOper, false, app)
	Assert(t, err != nil, "ICMP Alg with FTP config must fail.  Error: %v", err)

	// FTP ALG with Icmp config
	app = security.App{
		TypeMeta: api.TypeMeta{Kind: "App"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      "testApp",
		},
		Spec: security.AppSpec{
			ProtoPorts: []security.ProtoPort{
				{
					Protocol: "icmp",
				},
			},
			ALG: &security.ALG{
				Type: "FTP",
				IcmpAlg: &security.IcmpAlg{
					Type: "1",
					Code: "0",
				},
			},
		},
	}

	_, _, err = s.validateApp(context.Background(), nil, nil, "", apiserver.CreateOper, false, app)
	Assert(t, err != nil, "FTP Alg with ICMP config must fail.  Error: %v", err)

	// DNS ALG with Icmp config
	app = security.App{
		TypeMeta: api.TypeMeta{Kind: "App"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      "testApp",
		},
		Spec: security.AppSpec{
			ProtoPorts: []security.ProtoPort{
				{
					Protocol: "icmp",
				},
			},
			ALG: &security.ALG{
				Type: "DNS",
				IcmpAlg: &security.IcmpAlg{
					Type: "1",
					Code: "0",
				},
			},
		},
	}

	_, _, err = s.validateApp(context.Background(), nil, nil, "", apiserver.CreateOper, false, app)
	Assert(t, err != nil, "FTP Alg with ICMP config must fail.  Error: %v", err)

	// SunRPC LG with Icmp config
	app = security.App{
		TypeMeta: api.TypeMeta{Kind: "App"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      "testApp",
		},
		Spec: security.AppSpec{
			ProtoPorts: []security.ProtoPort{
				{
					Protocol: "icmp",
				},
			},
			ALG: &security.ALG{
				Type: "SunRPC",
				IcmpAlg: &security.IcmpAlg{
					Type: "1",
					Code: "0",
				},
			},
		},
	}

	_, _, err = s.validateApp(context.Background(), nil, nil, "", apiserver.CreateOper, false, app)
	Assert(t, err != nil, "SunRPC Alg with ICMP config must fail.  Error: %v", err)

	// MSRPC LG with Icmp config
	app = security.App{
		TypeMeta: api.TypeMeta{Kind: "App"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      "testApp",
		},
		Spec: security.AppSpec{
			ProtoPorts: []security.ProtoPort{
				{
					Protocol: "icmp",
				},
			},
			ALG: &security.ALG{
				Type: "MSRPC",
				IcmpAlg: &security.IcmpAlg{
					Type: "1",
					Code: "0",
				},
			},
		},
	}

	_, _, err = s.validateApp(context.Background(), nil, nil, "", apiserver.CreateOper, false, app)
	Assert(t, err != nil, "MSRPC Alg with ICMP config must fail.  Error: %v", err)

	// TFTP LG with Icmp config
	app = security.App{
		TypeMeta: api.TypeMeta{Kind: "App"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      "testApp",
		},
		Spec: security.AppSpec{
			ProtoPorts: []security.ProtoPort{
				{
					Protocol: "icmp",
				},
			},
			ALG: &security.ALG{
				Type: "TFTP",
				IcmpAlg: &security.IcmpAlg{
					Type: "1",
					Code: "0",
				},
			},
		},
	}

	_, _, err = s.validateApp(context.Background(), nil, nil, "", apiserver.CreateOper, false, app)
	Assert(t, err != nil, "TFTP Alg with ICMP config must fail.  Error: %v", err)

	// RSTP LG with Icmp config
	app = security.App{
		TypeMeta: api.TypeMeta{Kind: "App"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      "testApp",
		},
		Spec: security.AppSpec{
			ProtoPorts: []security.ProtoPort{
				{
					Protocol: "icmp",
				},
			},
			ALG: &security.ALG{
				Type: "RTSP",
				IcmpAlg: &security.IcmpAlg{
					Type: "1",
					Code: "0",
				},
			},
		},
	}

	_, _, err = s.validateApp(context.Background(), nil, nil, "", apiserver.CreateOper, false, app)
	Assert(t, err != nil, "RSTP Alg with ICMP config must fail.  Error: %v", err)
}
