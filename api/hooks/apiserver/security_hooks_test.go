package impl

import (
	"context"
	"fmt"
	"strings"
	"testing"

	"github.com/pensando/sw/api/generated/apiclient"
	apiintf "github.com/pensando/sw/api/interfaces"
	"github.com/pensando/sw/venice/utils/kvstore/store"
	"github.com/pensando/sw/venice/utils/runtime"

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

	errs := s.validateSGPolicy(sgp, "v1", false, false)
	Assert(t, len(errs) == 0, "failed to create sg policy. Error: %v", errs)
}

func TestSGPolicyCreateWithPreCommitHook(t *testing.T) {
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

	storecfg := store.Config{
		Type:    store.KVStoreTypeMemkv,
		Codec:   runtime.NewJSONCodec(runtime.NewScheme()),
		Servers: []string{t.Name()},
	}

	kvs, err := store.New(storecfg)
	AssertOk(t, err, "Failed to create kv store. Err: %v", err)

	_, _, err = s.enforceMaxSGPolicyPreCommitHook(context.Background(), kvs, kvs.NewTxn(), "", apiintf.CreateOper, false, sgp)
	AssertOk(t, err, "Single SGPolicy create must succeed")
}

func TestMaxSGPolicyEnforcement(t *testing.T) {
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

	sgp1 := security.SGPolicy{
		TypeMeta: api.TypeMeta{Kind: "SGPolicy"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      "testpolicy1",
		},
		Spec: security.SGPolicySpec{
			AttachTenant: true,
			Rules:        rules,
		},
	}

	config := store.Config{Type: store.KVStoreTypeMemkv, Servers: []string{""}, Codec: runtime.NewJSONCodec(runtime.GetDefaultScheme())}
	kv, err := store.New(config)
	AssertOk(t, err, "Failed to create kv store. Err: %v", err)

	ctx := context.TODO()
	key := strings.TrimSuffix(sgp.MakeKey(string(apiclient.GroupSecurity)), "/")
	fmt.Println("KEY: ", key)
	err = kv.Create(ctx, key, &sgp)
	AssertOk(t, err, fmt.Sprintf("Error creating object in KVStore"))

	_, _, err = s.enforceMaxSGPolicyPreCommitHook(context.Background(), kv, kv.NewTxn(), "", apiintf.CreateOper, false, sgp1)
	fmt.Println(err)
	Assert(t, err != nil, "SGPolicy creates exceeding max allowed must fail")
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
					Protocol: "icmp",
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

	errs := s.validateSGPolicy(sgp, "v1", false, false)
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

	errs := s.validateSGPolicy(sgp, "v1", false, false)
	Assert(t, len(errs) == 0, "SG Policy attaching to the sgs with From Addresses must pass. Error: %v", errs)
}

func TestSGPolicyV4SrcAnyDst(t *testing.T) {
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

	errs := s.validateSGPolicy(sgp, "v1", false, false)
	Assert(t, len(errs) == 0, "failed to create sg policy. Error: %v", errs)
}

func TestSGPolicyV4DstAnySrc(t *testing.T) {
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
			FromIPAddresses: []string{"any"},
			ToIPAddresses:   []string{"172.0.0.1", "172.0.0.2", "10.0.0.1/30"},
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

	errs := s.validateSGPolicy(sgp, "v1", false, false)
	Assert(t, len(errs) == 0, "failed to create sg policy. Error: %v", errs)
}

func TestSGPolicyV6SrcAnyDst(t *testing.T) {
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
			FromIPAddresses: []string{"1001:1::1", "1001:1::2", "1001:1::3"},
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

	errs := s.validateSGPolicy(sgp, "v1", false, false)
	Assert(t, len(errs) == 0, "failed to create sg policy. Error: %v", errs)
}

func TestSGPolicyV6DstAnySrc(t *testing.T) {
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
			FromIPAddresses: []string{"1001:1::1", "1001:1::2", "1001:1::3"},
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

	errs := s.validateSGPolicy(sgp, "v1", false, false)
	Assert(t, len(errs) == 0, "failed to create sg policy. Error: %v", errs)
}

// #################### Corner Case Tests ####################
func TestBadSGPolicyV4V6EndIPSrcMixed(t *testing.T) {
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
			Action: "PERMIT",
			// this unholy combination must be error out
			FromIPAddresses: []string{"172.0.0.1", "172.0.0.2-1001:1::1"},
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

	errs := s.validateSGPolicy(sgp, "v1", false, false)
	Assert(t, len(errs) != 0, "mixed v4 and v6 addresses must fail. Error: %v", errs)
}

func TestBadSGPolicyV4V6StartIPSrcMixed(t *testing.T) {
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
			Action: "PERMIT",
			// this unholy combination must be error out
			FromIPAddresses: []string{"172.0.0.1", "1001:1::1-172.0.0.2"},
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

	errs := s.validateSGPolicy(sgp, "v1", false, false)
	Assert(t, len(errs) != 0, "mixed v4 and v6 addresses must fail. Error: %v", errs)
}

func TestBadSGPolicyV4V6EndIPDstMixed(t *testing.T) {
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
			Action: "PERMIT",
			// this unholy combination must be error out
			FromIPAddresses: []string{"any"},
			ToIPAddresses:   []string{"172.0.0.1", "172.0.0.2-1001:1::1"},
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

	errs := s.validateSGPolicy(sgp, "v1", false, false)
	Assert(t, len(errs) != 0, "mixed v4 and v6 addresses must fail. Error: %v", errs)
}

func TestBadSGPolicyV4V6StartIPDstMixed(t *testing.T) {
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
			Action: "PERMIT",
			// this unholy combination must be error out
			FromIPAddresses: []string{"any"},
			ToIPAddresses:   []string{"172.0.0.1", "1001:1::1-172.0.0.2"},
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

	errs := s.validateSGPolicy(sgp, "v1", false, false)
	Assert(t, len(errs) != 0, "mixed v4 and v6 addresses must fail. Error: %v", errs)
}

func TestBadSGPolicyV4CIDRV6EndIPSrcMixed(t *testing.T) {
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
			Action: "PERMIT",
			// this unholy combination must be error out
			FromIPAddresses: []string{"172.0.0.1", "172.0.0.2/16-1001:1::1"},
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

	errs := s.validateSGPolicy(sgp, "v1", false, false)
	Assert(t, len(errs) != 0, "mixed v4 and v6 addresses must fail. Error: %v", errs)
}

func TestBadSGPolicyV4CIDRV6StartIPSrcMixed(t *testing.T) {
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
			Action: "PERMIT",
			// this unholy combination must be error out
			FromIPAddresses: []string{"172.0.0.1", "1001:1::1-172.0.0.2/16"},
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

	errs := s.validateSGPolicy(sgp, "v1", false, false)
	Assert(t, len(errs) != 0, "mixed v4 and v6 addresses must fail. Error: %v", errs)
}

func TestBadSGPolicyV4CIDRV6EndIPDstMixed(t *testing.T) {
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
			Action: "PERMIT",
			// this unholy combination must be error out
			FromIPAddresses: []string{"any"},
			ToIPAddresses:   []string{"172.0.0.1", "172.0.0.2/16-1001:1::1"},
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

	errs := s.validateSGPolicy(sgp, "v1", false, false)
	Assert(t, len(errs) != 0, "mixed v4 and v6 addresses must fail. Error: %v", errs)
}

func TestBadSGPolicyV4CIDRV6StartIPDstMixed(t *testing.T) {
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
			Action: "PERMIT",
			// this unholy combination must be error out
			FromIPAddresses: []string{"any"},
			ToIPAddresses:   []string{"172.0.0.1", "1001:1::1-172.0.0.2/16"},
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

	errs := s.validateSGPolicy(sgp, "v1", false, false)
	Assert(t, len(errs) != 0, "mixed v4 and v6 addresses must fail. Error: %v", errs)
}

func TestBadSGPolicyV4SrcV6Dst(t *testing.T) {
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
			FromIPAddresses: []string{"172.0.0.1", "172.0.0.2"},
			ToIPAddresses:   []string{"1001:1::1"},
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

	errs := s.validateSGPolicy(sgp, "v1", false, false)
	Assert(t, len(errs) != 0, "mixed v4 and v6 addresses must fail. Error: %v", errs)
}

func TestBadSGPolicyV4CIDRSrcV6Dst(t *testing.T) {
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
			ToIPAddresses:   []string{"1001:1::1"},
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

	errs := s.validateSGPolicy(sgp, "v1", false, false)
	Assert(t, len(errs) != 0, "mixed v4 and v6 addresses must fail. Error: %v", errs)
}

func TestBadSGPolicyV4RangeSrcV6Dst(t *testing.T) {
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
			FromIPAddresses: []string{"172.0.0.1", "10.0.0.1-10.0.0.10"},
			ToIPAddresses:   []string{"1001:1::1"},
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

	errs := s.validateSGPolicy(sgp, "v1", false, false)
	Assert(t, len(errs) != 0, "mixed v4 and v6 addresses must fail. Error: %v", errs)
}

func TestBadSGPolicyV6SrcV4Dst(t *testing.T) {
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
			FromIPAddresses: []string{"1001:1::1"},
			ToIPAddresses:   []string{"172.0.0.1", "172.0.0.2"},
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

	errs := s.validateSGPolicy(sgp, "v1", false, false)
	Assert(t, len(errs) != 0, "mixed v4 and v6 addresses must fail. Error: %v", errs)
}

func TestBadSGPolicyV6SrcV4DstCIDR(t *testing.T) {
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
			FromIPAddresses: []string{"1001:1::1"},
			ToIPAddresses:   []string{"172.0.0.1", "172.0.0.2", "10.0.0.1/30"},
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

	errs := s.validateSGPolicy(sgp, "v1", false, false)
	Assert(t, len(errs) != 0, "mixed v4 and v6 addresses must fail. Error: %v", errs)
}

func TestBadSGPolicyV6SrcV4DstRange(t *testing.T) {
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
			FromIPAddresses: []string{"1001:1::1"},
			ToIPAddresses:   []string{"172.0.0.1", "10.0.0.1-10.0.0.10"},
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

	errs := s.validateSGPolicy(sgp, "v1", false, false)
	Assert(t, len(errs) != 0, "mixed v4 and v6 addresses must fail. Error: %v", errs)
}

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

	errs := s.validateSGPolicy(sgp, "v1", false, false)
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

	errs := s.validateSGPolicy(sgp, "v1", false, false)
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

	errs := s.validateSGPolicy(sgp, "v1", false, false)
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

	errs := s.validateSGPolicy(sgp, "v1", false, false)
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

	errs := s.validateSGPolicy(sgp, "v1", false, false)
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

	errs := s.validateSGPolicy(sgp, "v1", false, false)
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

	errs := s.validateSGPolicy(sgp, "v1", false, false)
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

	errs := s.validateSGPolicy(sgp, "v1", false, false)
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

	errs := s.validateSGPolicy(sgp, "v1", false, false)
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

	err := s.validateSGPolicy(sgp, "v1", false, false)
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

	errs := s.validateSGPolicy(sgp, "v1", false, false)
	Assert(t, len(errs) == 0, "sg policy creates with protocol number should suceed")

	// invalid protocol number
	sgp.Spec.Rules[0].ProtoPorts[0].Protocol = "256"
	errs = s.validateSGPolicy(sgp, "v1", false, false)
	Assert(t, len(errs) != 0, "app incorrect protocol number must fail")

	// invalid protocol number
	sgp.Spec.Rules[0].ProtoPorts[0].Protocol = "-1"
	errs = s.validateSGPolicy(sgp, "v1", false, false)
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

	errs := s.validateSGPolicy(sgp, "v1", false, false)
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

	errs := s.validateSGPolicy(sgp, "v1", false, false)
	Assert(t, len(errs) == 0, "app range config failed")

	sgp.Spec.Rules = rulesIncorrectPortFormat
	errs = s.validateSGPolicy(sgp, "v1", false, false)
	Assert(t, len(errs) != 0, "app incorrect port range must fail")

	sgp.Spec.Rules = rulesIncorrectPortRangeFormat
	errs = s.validateSGPolicy(sgp, "v1", false, false)
	Assert(t, len(errs) != 0, "app incorrect port range format must fail")

	sgp.Spec.Rules = rulesIncorrectPortValFormat
	errs = s.validateSGPolicy(sgp, "v1", false, false)
	Assert(t, len(errs) != 0, "app incorrect port value format must fail")

	sgp.Spec.Rules = rulesInvalidPortRange
	errs = s.validateSGPolicy(sgp, "v1", false, false)
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

	errs := s.validateSGPolicy(sgp, "v1", false, false)
	Assert(t, len(errs) != 0, "app ports outside 0 - 64K must fail")

	sgp.Spec.Rules = rulesAboveRange

	errs = s.validateSGPolicy(sgp, "v1", false, false)
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

	errs := s.validateSGPolicy(sgp, "v1", false, false)
	Assert(t, len(errs) != 0, "SG Policy attaching to the tenant with missing To Addresses must fail. Error: %v", errs)

	sgp.Spec.Rules = rulesMissingFrom
	errs = s.validateSGPolicy(sgp, "v1", false, false)
	Assert(t, len(errs) != 0, "SG Policy attaching to the tenant with missing From Addresses must fail. Error: %v", errs)

	sgp.Spec.Rules = rulesMissingBoth
	errs = s.validateSGPolicy(sgp, "v1", false, false)
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

	errs := s.validateSGPolicy(sgp, "v1", false, false)
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

	errs := s.validateSGPolicy(sgp, "v1", false, false)
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

	errs := s.validateSGPolicy(sgp, "v1", false, false)
	Assert(t, len(errs) != 0, "SG Policy with invalid IP Range must fail.  Error: %v", errs)

	sgp.Spec.Rules = rulesInvalidRangeMultipleSep
	errs = s.validateSGPolicy(sgp, "v1", false, false)
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

	errs := s.validateSGPolicy(sgp, "v1", false, false)
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

	errs := s.validateSGPolicy(sgp, "v1", false, false)
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

	errs := s.validateSGPolicy(sgp, "v1", false, false)
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

	errs := s.validateSGPolicy(sgp, "v1", false, false)
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

	errs := s.validateSGPolicy(app, "v1", false, false)
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

	errs := s.validateApp(app, "v1", false, false)
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

	errs = s.validateApp(app, "v1", false, false)
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

	errs = s.validateApp(app, "v1", false, false)
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

	errs = s.validateApp(app, "v1", false, false)
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

	errs = s.validateApp(app, "v1", false, false)
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

	errs = s.validateApp(app, "v1", false, false)
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

	errs = s.validateApp(app, "v1", false, false)
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

	errs = s.validateApp(app, "v1", false, false)
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

	errs = s.validateApp(app, "v1", false, false)
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
				Type: "ftp",
				Ftp: &security.Ftp{
					AllowMismatchIPAddress: true,
				},
			},
		},
	}

	errs := s.validateApp(app, "v1", false, false)
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

	errs = s.validateApp(app, "v1", false, false)
	Assert(t, len(errs) == 0, "failed to create app with empty ALG. Error: %v", errs)

	// icmp ALG
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
				Type: "icmp",
				Icmp: &security.Icmp{
					Type: "1",
					Code: "0",
				},
			},
		},
	}

	errs = s.validateApp(app, "v1", false, false)
	Assert(t, len(errs) == 0, "failed to create app with icmp ALG config. Error: %v", errs)

	// icmp ALG
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
				Type: "icmp",
			},
		},
	}

	errs = s.validateApp(app, "v1", false, false)
	Assert(t, len(errs) == 0, "failed to create app with icmp ALG config without Specific Type. Error: %v", errs)

	// dns ALG
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
				Type: "dns",
				Dns: &security.Dns{
					DropMultiQuestionPackets: true,
				},
			},
		},
	}

	errs = s.validateApp(app, "v1", false, false)
	Assert(t, len(errs) == 0, "failed to create app with dns ALG config. Error: %v", errs)

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
				Type: "sunrpc",
				Sunrpc: []*security.Sunrpc{
					&security.Sunrpc{
						ProgramID: "1",
						Timeout:   "2s",
					},
				},
			},
		},
	}

	errs = s.validateApp(app, "v1", false, false)
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
				Type: "msrpc",
				Msrpc: []*security.Msrpc{
					&security.Msrpc{
						ProgramUUID: "1",
						Timeout:     "2s",
					},
				},
			},
		},
	}

	errs = s.validateApp(app, "v1", false, false)
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

	errs = s.validateApp(app, "v1", false, false)
	Assert(t, len(errs) != 0, "Invalid protocol name must fail.  Error: %v", errs)

	// invalid icmp type
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
				Type: "icmp",
				Icmp: &security.Icmp{
					Type: "foo",
					Code: "0",
				},
			},
		},
	}

	errs = s.validateApp(app, "v1", false, false)
	Assert(t, len(errs) != 0, "Invalid icmp type must fail.  Error: %v", errs)

	// invalid icmp type
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
				Type: "icmp",
				Icmp: &security.Icmp{
					Type: "300",
					Code: "0",
				},
			},
		},
	}

	errs = s.validateApp(app, "v1", false, false)
	Assert(t, len(errs) != 0, "Invalid icmp type must fail.  Error: %v", errs)

	// invalid icmp code
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
				Type: "icmp",
				Icmp: &security.Icmp{
					Type: "1",
					Code: "foo",
				},
			},
		},
	}

	errs = s.validateApp(app, "v1", false, false)
	Assert(t, len(errs) != 0, "Invalid icmp code must fail.  Error: %v", errs)

	// invalid icmp code
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
				Type: "icmp",
				Icmp: &security.Icmp{
					Type: "1",
					Code: "20",
				},
			},
		},
	}

	errs = s.validateApp(app, "v1", false, false)
	Assert(t, len(errs) != 0, "Invalid icmp code must fail.  Error: %v", errs)

	// icmp LG with Ftp config
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
				Type: "icmp",
				Ftp: &security.Ftp{
					AllowMismatchIPAddress: true,
				},
			},
		},
	}

	errs = s.validateApp(app, "v1", false, false)
	Assert(t, len(errs) != 0, "icmp Alg with ftp config must fail.  Error: %v", errs)

	// ftp ALG with Icmp config
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
				Type: "ftp",
				Icmp: &security.Icmp{
					Type: "1",
					Code: "0",
				},
			},
		},
	}

	errs = s.validateApp(app, "v1", false, false)
	Assert(t, len(errs) != 0, "FTP  with icmp config must fail.  Error: %v", errs)

	// dns ALG with Icmp config
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
				Type: "dns",
				Icmp: &security.Icmp{
					Type: "1",
					Code: "0",
				},
			},
		},
	}

	errs = s.validateApp(app, "v1", false, false)
	Assert(t, len(errs) != 0, "ftp  with icmp config must fail.  Error: %v", errs)

	// dns ALG with TCP proto config
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
				Type: "dns",
			},
		},
	}

	errs = s.validateApp(app, "v1", false, false)
	Assert(t, len(errs) != 0, "TCP on dns ALG must not be allowed. Error: %v", errs)

	// sunrpc LG with Icmp config
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
				Type: "sunrpc",
				Icmp: &security.Icmp{
					Type: "1",
					Code: "0",
				},
			},
		},
	}

	errs = s.validateApp(app, "v1", false, false)
	Assert(t, len(errs) != 0, "sunrpc  with icmp config must fail.  Error: %v", errs)

	// msrpc LG with Icmp config
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
				Type: "msrpc",
				Icmp: &security.Icmp{
					Type: "1",
					Code: "0",
				},
			},
		},
	}

	errs = s.validateApp(app, "v1", false, false)
	Assert(t, len(errs) != 0, "msrpc  with icmp config must fail.  Error: %v", errs)

	// Tftp LG with Icmp config
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
				Type: "Tftp",
				Icmp: &security.Icmp{
					Type: "1",
					Code: "0",
				},
			},
		},
	}

	errs = s.validateApp(app, "v1", false, false)
	Assert(t, len(errs) != 0, "tftp  with icmp config must fail.  Error: %v", errs)

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

	errs = s.validateApp(app, "v1", false, false)
	Assert(t, len(errs) != 0, "RSTP  with icmp config must fail.  Error: %v", errs)
}
