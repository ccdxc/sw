package services

import (
	"testing"

	"github.com/pensando/sw/venice/cmd/services/mock"
	"github.com/pensando/sw/venice/cmd/types"
)

func setupNode(testName string) (types.SystemdService, types.NodeService) {
	testIP := "11.1.1.1"

	s := NewSystemdService(WithSysIfSystemdSvcOption(&mock.SystemdIf{}))
	n := NewNodeService(testName, testIP, WithSystemdSvcNodeOption(s), WithConfigsNodeOption(&mock.Configs{}))
	return s, n
}

func checkAllNodeServiceStarted(t *testing.T, s types.SystemdService) {
	checkAllServiceStarted(t, s, nodeServices)
}

func TestNodeServiceStopServices(t *testing.T) {
	t.Parallel()
	s, n := setupNode("TestNodeServiceStopServices")

	s.Start()
	n.Start()

	checkAllNodeServiceStarted(t, s)
	n.Stop()
	checkAllServiceStopped(t, s)
}

func TestNodeServiceStartBeforeSystemdService(t *testing.T) {
	t.Parallel()
	s, n := setupNode("TestNodeServiceStartBeforeSystemdService")

	n.Start()
	s.Start()

	checkAllNodeServiceStarted(t, s)
}
