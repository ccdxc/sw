package impl

import (
	"testing"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/api/generated/network"
	apisrvmocks "github.com/pensando/sw/venice/apiserver/pkg/mocks"
	"github.com/pensando/sw/venice/utils/log"
)

func TestIPAMPolicyConfig(t *testing.T) {
	logConfig := &log.Config{
		Module:      "Network-hooks",
		Format:      log.LogFmt,
		Filter:      log.AllowAllFilter,
		Debug:       false,
		CtxSelector: log.ContextAll,
		LogToStdout: true,
		LogToFile:   false,
	}

	// Initialize logger config
	l := log.SetConfig(logConfig)
	service := apisrvmocks.NewFakeService()
	meth := apisrvmocks.NewFakeMethod(true)
	msg := apisrvmocks.NewFakeMessage("test", "/test/path", false)
	apisrvmocks.SetFakeMethodReqType(msg, meth)
	service.AddMethod("IPAMPolicy", meth)
	s := &networkHooks{
		svc:    service,
		logger: l,
	}

	policy := network.IPAMPolicy{
		TypeMeta: api.TypeMeta{Kind: "IPAMPolicy"},
		ObjectMeta: api.ObjectMeta{
			Name:      "testPolicy",
			Namespace: "default",
			Tenant:    "default",
		},
		Spec: network.IPAMPolicySpec{
			DHCPRelay: &network.DHCPRelayPolicy{},
		},
		Status: network.IPAMPolicyStatus{},
	}

	registerNetworkHooks(service, l)
	server := &network.DHCPServer{
		IPAddress:     "100.1.1.1",
		VirtualRouter: "default",
	}

	server1 := &network.DHCPServer{
		IPAddress:     "101.1.1.1",
		VirtualRouter: "default",
	}

	policy.Spec.DHCPRelay.Servers = append(policy.Spec.DHCPRelay.Servers, server)

	ok := s.validateIPAMPolicyConfig(policy, "", false, false)
	if ok != nil {
		t.Errorf("Failed to create a good IPAMPolicy config (%s)", ok)
	}

	policy.Spec.DHCPRelay.Servers = append(policy.Spec.DHCPRelay.Servers, server1)

	ok = s.validateIPAMPolicyConfig(policy, "", false, false)
	if ok == nil {
		t.Errorf("validation passed, expecting to fail for %v", policy.Name)
	}
	l.Infof("IPAM Policy %v : Error %v", policy.Name, ok)
}
