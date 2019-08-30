package veniceinteg

import (
	"context"
	"fmt"
	"sync/atomic"

	. "gopkg.in/check.v1"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/api/generated/cluster"
	diagapi "github.com/pensando/sw/api/generated/diagnostics"
	"github.com/pensando/sw/nic/agent/protos/tsproto"
	"github.com/pensando/sw/venice/globals"
	"github.com/pensando/sw/venice/utils/diagnostics"
	"github.com/pensando/sw/venice/utils/log"
	"github.com/pensando/sw/venice/utils/rpckit"
	. "github.com/pensando/sw/venice/utils/testutils"
	"github.com/pensando/sw/venice/utils/testutils/policygen"
)

type diagAgent struct {
	techSupportAgent
	moduleNotifications []*diagapi.Module
	watchingModules     int32
	diagClient          tsproto.DiagnosticsApiClient
}

func (ag *diagAgent) watchModules(inCtx context.Context, c *C) {
	AssertEventually(c, func() (bool, interface{}) {
		rpcClient, err := rpckit.NewRPCClient(ag.name, ag.tsmAPIAddr, rpckit.WithRemoteServerName(globals.Tsm))
		if err != nil {
			return false, fmt.Errorf("error creating TSM client %s: %v", ag.name, err)
		}
		ag.rpcClient = rpcClient
		return true, nil
	}, fmt.Sprintf("error creating TSM client %s", ag.name), "1s", "15s")
	ag.diagClient = tsproto.NewDiagnosticsApiClient(ag.rpcClient.ClientConn)
	ctx, cancelFn := context.WithCancel(inCtx)
	ag.cancelFn = cancelFn
	stream, err := ag.diagClient.WatchModule(ctx, &api.ListWatchOptions{
		FieldSelector: fmt.Sprintf("status.mac-address=%s,status.category=%s", ag.name, diagapi.ModuleStatus_Naples.String()),
	})
	AssertOk(c, err, "error sending WatchModule request")
	atomic.StoreInt32(&ag.watchingModules, 1)
	log.Infof("Agent %s started module watch", ag.name)
	for {
		modObj, err := stream.Recv()
		if err != nil {
			break
		}
		ag.moduleNotifications = append(ag.moduleNotifications, modObj)
	}
	log.Infof("Agent %s stopped module watch", ag.name)
	atomic.StoreInt32(&ag.watchingModules, 0)
}

func (ag *diagAgent) stop() {
	ag.techSupportAgent.stop()
	ag.diagClient = nil
}
func newDiagAgent(name string) *diagAgent {
	agent := &diagAgent{
		techSupportAgent: *newTechSupportAgent(name, kindSmartNICNode, nil),
	}
	return agent
}

// check that all agents have received numNotifications for each matching request
func checkModuleNotifications(c *C, agents map[string]*diagAgent, numNotifications int) {
	// Make sure all agents have received the notification
	AssertEventually(c, func() (bool, interface{}) {
		for _, agent := range agents {
			have := len(agent.moduleNotifications)
			want := numNotifications
			if have != want {
				return false, fmt.Sprintf("Agent %v has wrong notification count. Have: %d, Want: %d, %+v", agent.name, have, want, agent.moduleNotifications)
			}
		}
		return true, nil
	}, "Error verifying notifications", "500ms", "30s")

	// now verify that the notifications for all supported naples modules were received
	moduleNotificationMap := make(map[string]*diagapi.Module)
	for _, agent := range agents {
		for _, modObj := range agent.moduleNotifications {
			moduleNotificationMap[modObj.Name] = modObj
		}
		for _, supportedModule := range diagnostics.ListSupportedNaplesModules() {
			n := fmt.Sprintf("%s-%s", agent.name, supportedModule)
			_, ok := moduleNotificationMap[n]
			Assert(c, ok, fmt.Sprintf("notification not found for %s", n))
		}
	}

}

func (it *veniceIntegSuite) TestModuleWatch(c *C) {
	agentMap := make(map[string]*diagAgent)
	for i := 0; i < 9; i++ {
		name := fmt.Sprintf("00ae.cd33.000%d", i)
		n := policygen.CreateSmartNIC(name,
			cluster.DistributedServiceCardStatus_PENDING.String(),
			"esx-1",
			&cluster.DSCCondition{
				Type:   cluster.DSCCondition_HEALTHY.String(),
				Status: cluster.ConditionStatus_FALSE.String(),
			})
		err := it.createSmartNICNode(context.TODO(), n)
		AssertOk(c, err, fmt.Sprintf("Error creating SmartNIC node %+v", n))
		defer it.deleteSmartNICNode(context.TODO(), &n.ObjectMeta)
		agent := newDiagAgent(n.Name)
		go agent.watchModules(context.TODO(), c)
		defer agent.stop()
		agentMap[name] = agent
	}
	checkModuleNotifications(c, agentMap, len(diagnostics.ListSupportedNaplesModules()))
}
