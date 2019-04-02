package impl

import (
	"fmt"
	"testing"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/api/generated/security"
	"github.com/pensando/sw/venice/apiserver/pkg/mocks"
	"github.com/pensando/sw/venice/utils/log"
	. "github.com/pensando/sw/venice/utils/testutils"
)

// #################### Happy Path Tests ####################
func TestSGPolicyCreateAtTenant(t *testing.T) {
	t.Parallel()
	logConfig := log.GetDefaultConfig(t.Name())
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

	errs := s.validateSGPolicy(sgp, "v1", false)
	Assert(t, len(errs) == 0, "failed to create sg policy. Error: %v", errs)
}

func TestSGPolicyCreateAtSGs(t *testing.T) {
	t.Parallel()
	logConfig := log.GetDefaultConfig(t.Name())
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

	errs := s.validateSGPolicy(sgp, "v1", false)
	Assert(t, len(errs) == 0, "failed to create sg policy. Error: %v", errs)
}

func TestAttachGroupsWithFromAddresses(t *testing.T) {
	t.Parallel()
	logConfig := log.GetDefaultConfig(t.Name())
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

	errs := s.validateSGPolicy(sgp, "v1", false)
	Assert(t, len(errs) == 0, "SG Policy attaching to the sgs with From Addresses must pass. Error: %v", errs)
}

// #################### Corner Case Tests ####################
func TestBothAttachmentPoints(t *testing.T) {
	t.Parallel()
	logConfig := log.GetDefaultConfig(t.Name())
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

	errs := s.validateSGPolicy(sgp, "v1", false)
	Assert(t, len(errs) != 0, "sg policy creates specifying both tenant and sg level must fail")
}

func TestEmptyPortsInRules(t *testing.T) {
	t.Parallel()
	logConfig := log.GetDefaultConfig(t.Name())
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

	errs := s.validateSGPolicy(sgp, "v1", false)
	Assert(t, len(errs) != 0, "sg policy creates specifying empty ports must fail")
}

func TestAnyProtoInRules(t *testing.T) {
	t.Parallel()
	logConfig := log.GetDefaultConfig(t.Name())
	s := &securityHooks{
		svc:    mocks.NewFakeService(),
		logger: log.GetNewLogger(logConfig),
	}
	// create sg policy
	rules := []security.SGRule{
		{
			ProtoPorts: []security.ProtoPort{
				{
					Protocol: "any",
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

	errs := s.validateSGPolicy(sgp, "v1", false)
	Assert(t, len(errs) == 0, "sg policy create with any protocol should suceed")
}
func TestEmptyProtoPorts(t *testing.T) {
	t.Parallel()
	logConfig := log.GetDefaultConfig(t.Name())
	s := &securityHooks{
		svc:    mocks.NewFakeService(),
		logger: log.GetNewLogger(logConfig),
	}
	// create sg policy
	rules := []security.SGRule{
		{
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

	errs := s.validateSGPolicy(sgp, "v1", false)
	Assert(t, len(errs) != 0, "sg policy creates specifying empty ports must fail")
}

func TestEmptyFromIPAddresses(t *testing.T) {
	t.Parallel()
	logConfig := log.GetDefaultConfig(t.Name())
	s := &securityHooks{
		svc:    mocks.NewFakeService(),
		logger: log.GetNewLogger(logConfig),
	}
	// create sg policy
	rules := []security.SGRule{
		{
			Action:        "PERMIT",
			ToIPAddresses: []string{"192.168.1.1/16"},
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

	errs := s.validateSGPolicy(sgp, "v1", false)
	Assert(t, len(errs) != 0, "sg policy creates specifying empty from addresses must fail when it is being attached to tenant")
	fmt.Println(errs)
}

func TestEmptyToIPAddresses(t *testing.T) {
	t.Parallel()
	logConfig := log.GetDefaultConfig(t.Name())
	s := &securityHooks{
		svc:    mocks.NewFakeService(),
		logger: log.GetNewLogger(logConfig),
	}
	// create sg policy
	rules := []security.SGRule{
		{
			Action:          "PERMIT",
			FromIPAddresses: []string{"192.168.1.1/16"},
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

	errs := s.validateSGPolicy(sgp, "v1", false)
	Assert(t, len(errs) != 0, "sg policy creates specifying empty from addresses must fail when it is being attached to tenant")
	fmt.Println(errs)
}

func TestEmptyFromAndToIPAddresses(t *testing.T) {
	t.Parallel()
	logConfig := log.GetDefaultConfig(t.Name())
	s := &securityHooks{
		svc:    mocks.NewFakeService(),
		logger: log.GetNewLogger(logConfig),
	}
	// create sg policy
	rules := []security.SGRule{
		{
			Action: "PERMIT",
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

	errs := s.validateSGPolicy(sgp, "v1", false)
	Assert(t, len(errs) != 0, "sg policy creates specifying empty from addresses must fail when it is being attached to tenant")
}

func TestMissingAttachmentPoint(t *testing.T) {
	t.Parallel()
	logConfig := log.GetDefaultConfig(t.Name())
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

	errs := s.validateSGPolicy(sgp, "v1", false)
	Assert(t, len(errs) != 0, "sg policy creates with missing attachment points")
}

func TestInvalidAppProto(t *testing.T) {
	t.Parallel()
	logConfig := log.GetDefaultConfig(t.Name())
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

	errs := s.validateSGPolicy(sgp, "v1", false)
	Assert(t, len(errs) != 0, "sg policy creates with invalid app proto fail")
}

func TestAppProtoBoth(t *testing.T) {
	t.Parallel()
	logConfig := log.GetDefaultConfig(t.Name())
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
			},
			Apps:            []string{"dns"},
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

	err := s.validateSGPolicy(sgp, "v1", false)
	Assert(t, err != nil, "sg policy created with both app and proto")
}

func TestProtocolNumbers(t *testing.T) {
	t.Parallel()
	logConfig := log.GetDefaultConfig(t.Name())
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

	errs := s.validateSGPolicy(sgp, "v1", false)
	Assert(t, len(errs) == 0, "sg policy creates with protocol number should suceed")

	// invalid protocol number
	sgp.Spec.Rules[0].ProtoPorts[0].Protocol = "256"
	errs = s.validateSGPolicy(sgp, "v1", false)
	Assert(t, len(errs) != 0, "app incorrect protocol number must fail")

	// invalid protocol number
	sgp.Spec.Rules[0].ProtoPorts[0].Protocol = "-1"
	errs = s.validateSGPolicy(sgp, "v1", false)
	Assert(t, len(errs) != 0, "app incorrect protocol number must fail")
}

func TestInvalidAppPortNonInteger(t *testing.T) {
	t.Parallel()
	logConfig := log.GetDefaultConfig(t.Name())
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

	errs := s.validateSGPolicy(sgp, "v1", false)
	Assert(t, len(errs) != 0, "sg policy creates with non integer parsable port must fail")
}

func TestRulePortRanges(t *testing.T) {
	t.Parallel()
	logConfig := log.GetDefaultConfig(t.Name())
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

	errs := s.validateSGPolicy(sgp, "v1", false)
	Assert(t, len(errs) == 0, "app range config failed")

	sgp.Spec.Rules = rulesIncorrectPortFormat
	errs = s.validateSGPolicy(sgp, "v1", false)
	Assert(t, len(errs) != 0, "app incorrect port range must fail")

	sgp.Spec.Rules = rulesIncorrectPortRangeFormat
	errs = s.validateSGPolicy(sgp, "v1", false)
	Assert(t, len(errs) != 0, "app incorrect port range format must fail")

	sgp.Spec.Rules = rulesIncorrectPortValFormat
	errs = s.validateSGPolicy(sgp, "v1", false)
	Assert(t, len(errs) != 0, "app incorrect port value format must fail")

	sgp.Spec.Rules = rulesInvalidPortRange
	errs = s.validateSGPolicy(sgp, "v1", false)
	Assert(t, len(errs) != 0, "app invalid port range must fail")
}

func TestInvalidAppPortInvalidPortRange(t *testing.T) {
	t.Parallel()
	logConfig := log.GetDefaultConfig(t.Name())
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

	errs := s.validateSGPolicy(sgp, "v1", false)
	Assert(t, len(errs) != 0, "app ports outside 0 - 64K must fail")

	sgp.Spec.Rules = rulesAboveRange

	errs = s.validateSGPolicy(sgp, "v1", false)
	Assert(t, len(errs) != 0, "app ports outside 0 - 64K must fail")
}

func TestAttachTenantWithMissingToAndFromAddresses(t *testing.T) {
	t.Parallel()
	logConfig := log.GetDefaultConfig(t.Name())
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

	errs := s.validateSGPolicy(sgp, "v1", false)
	Assert(t, len(errs) != 0, "SG Policy attaching to the tenant with missing To Addresses must fail. Error: %v", errs)

	sgp.Spec.Rules = rulesMissingFrom
	errs = s.validateSGPolicy(sgp, "v1", false)
	Assert(t, len(errs) != 0, "SG Policy attaching to the tenant with missing From Addresses must fail. Error: %v", errs)

	sgp.Spec.Rules = rulesMissingBoth
	errs = s.validateSGPolicy(sgp, "v1", false)
	Assert(t, len(errs) != 0, "SG Policy attaching to the tenant with missing To and From Addresses must fail. Error: %v", errs)
}

func TestInvalidIPAddressOctet(t *testing.T) {
	t.Parallel()
	logConfig := log.GetDefaultConfig(t.Name())
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

	errs := s.validateSGPolicy(sgp, "v1", false)
	Assert(t, len(errs) != 0, "SG Policy with invalid IP Address Octet must fail.  Error: %v", errs)

}

func TestInvalidIPAddressCIDR(t *testing.T) {
	t.Parallel()
	logConfig := log.GetDefaultConfig(t.Name())
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

	errs := s.validateSGPolicy(sgp, "v1", false)
	Assert(t, len(errs) != 0, "SG Policy with invalid CIDR block must fail.  Error: %v", errs)
}

func TestInvalidIPAddressRange(t *testing.T) {
	t.Parallel()
	logConfig := log.GetDefaultConfig(t.Name())
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

	errs := s.validateSGPolicy(sgp, "v1", false)
	Assert(t, len(errs) != 0, "SG Policy with invalid IP Range must fail.  Error: %v", errs)

	sgp.Spec.Rules = rulesInvalidRangeMultipleSep
	errs = s.validateSGPolicy(sgp, "v1", false)
	Assert(t, len(errs) != 0, "SG Policy with invalid IP Range must fail.  Error: %v", errs)
}

func TestInvalidKeyword(t *testing.T) {
	t.Parallel()
	logConfig := log.GetDefaultConfig(t.Name())
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

	errs := s.validateSGPolicy(sgp, "v1", false)
	Assert(t, len(errs) != 0, "SG Policy rules having non any keywords must fail.  Error: %v", errs)
}

func TestAttachGroupsWithInvalidIPAddresses(t *testing.T) {
	t.Parallel()
	logConfig := log.GetDefaultConfig(t.Name())
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

	errs := s.validateSGPolicy(sgp, "v1", false)
	Assert(t, len(errs) != 0, "SG Policy attaching to the sgs with invalid IP addresses must fail. Error: %v", errs)
}

func TestAppWithMultipleSeparators(t *testing.T) {
	t.Parallel()
	logConfig := log.GetDefaultConfig(t.Name())
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

	errs := s.validateSGPolicy(sgp, "v1", false)
	Assert(t, len(errs) != 0, "SG Policy with invalid app proto/port formats should fail. Error: %v", errs)
}

func TestAppWithInvalidProtocol(t *testing.T) {
	t.Parallel()
	logConfig := log.GetDefaultConfig(t.Name())
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

	errs := s.validateSGPolicy(sgp, "v1", false)
	Assert(t, len(errs) != 0, "SG Policy with invalid app proto/port formats should fail. Error: %v", errs)
}

func TestInvalidObjType(t *testing.T) {
	t.Parallel()
	logConfig := log.GetDefaultConfig(t.Name())
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

	errs := s.validateSGPolicy(app, "v1", false)
	Assert(t, len(errs) != 0, "Invalid object casts must fail.  Error: %v", errs)
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

	errs := s.validateApp(app, "v1", false)
	Assert(t, len(errs) == 0, "failed to create app. Error: %v", errs)

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

	errs = s.validateApp(app, "v1", false)
	Assert(t, len(errs) == 0, "failed to create app with empty protocol. Error: %v", errs)

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

	errs = s.validateApp(app, "v1", false)
	Assert(t, len(errs) == 0, "failed to create app with protocol number. Error: %v", errs)

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

	errs = s.validateApp(app, "v1", false)
	Assert(t, len(errs) != 0, "Invalid protocol name must fail.  Error: %v", errs)

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

	errs = s.validateApp(app, "v1", false)
	Assert(t, len(errs) != 0, "Invalid protocol number must fail.  Error: %v", errs)

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

	errs = s.validateApp(app, "v1", false)
	Assert(t, len(errs) == 0, "failed to create app. Error: %v", errs)

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

	errs = s.validateApp(app, "v1", false)
	Assert(t, len(errs) != 0, "Invalid protocol name must fail.  Error: %v", errs)

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

	errs = s.validateApp(app, "v1", false)
	Assert(t, len(errs) != 0, "Invalid protocol name must fail.  Error: %v", errs)

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

	errs = s.validateApp(app, "v1", false)
	Assert(t, len(errs) != 0, "Invalid protocol name must fail.  Error: %v", errs)
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
				Ftp: &security.Ftp{
					AllowMismatchIPAddress: true,
				},
			},
		},
	}

	errs := s.validateApp(app, "v1", false)
	Assert(t, len(errs) == 0, "failed to create app. Error: %v", errs)

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

	errs = s.validateApp(app, "v1", false)
	Assert(t, len(errs) == 0, "failed to create app with empty ALG. Error: %v", errs)

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
				Icmp: &security.Icmp{
					Type: "1",
					Code: "0",
				},
			},
		},
	}

	errs = s.validateApp(app, "v1", false)
	Assert(t, len(errs) == 0, "failed to create app with ICMP ALG config. Error: %v", errs)

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

	errs = s.validateApp(app, "v1", false)
	Assert(t, len(errs) == 0, "failed to create app with ICMP ALG config without Specific Type. Error: %v", errs)

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
				Dns: &security.Dns{
					DropMultiQuestionPackets: true,
				},
			},
		},
	}

	errs = s.validateApp(app, "v1", false)
	Assert(t, len(errs) == 0, "failed to create app with DNS ALG config. Error: %v", errs)

	// Sunrpc ALG
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
				Sunrpc: []*security.Sunrpc{
					&security.Sunrpc{
						ProgramID: "1",
						Timeout:   "2s",
					},
				},
			},
		},
	}

	errs = s.validateApp(app, "v1", false)
	Assert(t, len(errs) == 0, "failed to create app with Sunrpc ALG config. Error: %v", errs)

	// Msrpc ALG
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
				Msrpc: []*security.Msrpc{
					&security.Msrpc{
						ProgramUUID: "1",
						Timeout:     "2s",
					},
				},
			},
		},
	}

	errs = s.validateApp(app, "v1", false)
	Assert(t, len(errs) == 0, "failed to create app with Msrpc ALG config. Error: %v", errs)

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

	errs = s.validateApp(app, "v1", false)
	Assert(t, len(errs) != 0, "Invalid protocol name must fail.  Error: %v", errs)

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
				Icmp: &security.Icmp{
					Type: "foo",
					Code: "0",
				},
			},
		},
	}

	errs = s.validateApp(app, "v1", false)
	Assert(t, len(errs) != 0, "Invalid ICMP type must fail.  Error: %v", errs)

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
				Icmp: &security.Icmp{
					Type: "300",
					Code: "0",
				},
			},
		},
	}

	errs = s.validateApp(app, "v1", false)
	Assert(t, len(errs) != 0, "Invalid ICMP type must fail.  Error: %v", errs)

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
				Icmp: &security.Icmp{
					Type: "1",
					Code: "foo",
				},
			},
		},
	}

	errs = s.validateApp(app, "v1", false)
	Assert(t, len(errs) != 0, "Invalid ICMP code must fail.  Error: %v", errs)

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
				Icmp: &security.Icmp{
					Type: "1",
					Code: "20",
				},
			},
		},
	}

	errs = s.validateApp(app, "v1", false)
	Assert(t, len(errs) != 0, "Invalid ICMP code must fail.  Error: %v", errs)

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
				Ftp: &security.Ftp{
					AllowMismatchIPAddress: true,
				},
			},
		},
	}

	errs = s.validateApp(app, "v1", false)
	Assert(t, len(errs) != 0, "ICMP Alg with FTP config must fail.  Error: %v", errs)

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
				Icmp: &security.Icmp{
					Type: "1",
					Code: "0",
				},
			},
		},
	}

	errs = s.validateApp(app, "v1", false)
	Assert(t, len(errs) != 0, "FTP  with ICMP config must fail.  Error: %v", errs)

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
				Icmp: &security.Icmp{
					Type: "1",
					Code: "0",
				},
			},
		},
	}

	errs = s.validateApp(app, "v1", false)
	Assert(t, len(errs) != 0, "FTP  with ICMP config must fail.  Error: %v", errs)

	// DNS ALG with TCP proto config
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
				},
			},
			ALG: &security.ALG{
				Type: "DNS",
			},
		},
	}

	errs = s.validateApp(app, "v1", false)
	Assert(t, len(errs) != 0, "TCP on DNS ALG must not be allowed. Error: %v", errs)

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
				Icmp: &security.Icmp{
					Type: "1",
					Code: "0",
				},
			},
		},
	}

	errs = s.validateApp(app, "v1", false)
	Assert(t, len(errs) != 0, "SunRPC  with ICMP config must fail.  Error: %v", errs)

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
				Icmp: &security.Icmp{
					Type: "1",
					Code: "0",
				},
			},
		},
	}

	errs = s.validateApp(app, "v1", false)
	Assert(t, len(errs) != 0, "MSRPC  with ICMP config must fail.  Error: %v", errs)

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
				Icmp: &security.Icmp{
					Type: "1",
					Code: "0",
				},
			},
		},
	}

	errs = s.validateApp(app, "v1", false)
	Assert(t, len(errs) != 0, "TFTP  with ICMP config must fail.  Error: %v", errs)

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
				Icmp: &security.Icmp{
					Type: "1",
					Code: "0",
				},
			},
		},
	}

	errs = s.validateApp(app, "v1", false)
	Assert(t, len(errs) != 0, "RSTP  with ICMP config must fail.  Error: %v", errs)
}
