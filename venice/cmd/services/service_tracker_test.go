package services

import (
	"testing"

	"github.com/pensando/sw/venice/cmd/services/mock"
	"github.com/pensando/sw/venice/cmd/types"
	"github.com/pensando/sw/venice/globals"
	rmock "github.com/pensando/sw/venice/utils/resolver/mock"
	"github.com/pensando/sw/venice/utils/testutils"
)

func TestServiceTrackerOnQuorum(t *testing.T) {
	res := mock.NewResolverService()
	leader := mock.NewLeaderService(t.Name())
	nodeService := &mock.NodeService{}

	verifyResolver := func() {
		resEntry := res.Get(globals.KubeAPIServer)
		testutils.Assert(t, resEntry != nil, "resolver should have an entry for kubeAPIServer")
		testutils.Assert(t, resEntry.Kind == "Service", "resolver entry should have a Service Type")
		testutils.Assert(t, resEntry.Name == globals.KubeAPIServer, "resolver entry for kubeapiserver should exist")
		testutils.Assert(t, len(resEntry.Instances) == 1, "should have 1 service Instance entry")
		testutils.Assert(t, resEntry.Instances[0].Node == leader.Leader(), "resolver should have leader as the only entry")
	}

	serviceTracker := NewServiceTracker(res)
	leader.Register(serviceTracker)
	serviceTracker.Run(rmock.New(), nodeService)

	leader.Start()
	verifyResolver()
	leader.GiveupLeadership()
	verifyResolver()

	leader.Stop()
	res.Stop()
	serviceTracker.Stop()
}

func TestServiceTrackerOnNonQuorum(t *testing.T) {
	serviceTracker := NewServiceTracker(nil)
	resolverClient := rmock.New()
	nodeService := &mock.NodeService{}

	serviceTracker.Run(resolverClient, nodeService)

	resolverClient.AddServiceInstance(&types.ServiceInstance{Service: globals.KubeAPIServer, Node: t.Name()})
	testutils.Assert(t, nodeService.K8sAPIServerLocation == t.Name(), "resolver should have an entry for kubeAPIServer")

	resolverClient.DeleteServiceInstance(&types.ServiceInstance{Service: globals.KubeAPIServer, Node: t.Name()})
	testutils.Assert(t, nodeService.K8sAPIServerLocation == t.Name(), "resolver should still have an entry for kubeAPIServer after last delete")

	resolverClient.AddServiceInstance(&types.ServiceInstance{Service: globals.KubeAPIServer, Node: "dummy"})
	testutils.Assert(t, nodeService.K8sAPIServerLocation == "dummy", "resolver should have newer location for kubeAPIServer")

	serviceTracker.Stop()
}
