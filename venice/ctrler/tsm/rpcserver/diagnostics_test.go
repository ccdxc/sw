package rpcserver

import (
	"context"
	"fmt"
	"testing"
	"time"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/api/generated/diagnostics"
	"github.com/pensando/sw/nic/agent/protos/tsproto"
	"github.com/pensando/sw/venice/utils"
	"github.com/pensando/sw/venice/utils/kvstore"
	. "github.com/pensando/sw/venice/utils/testutils"
)

func TestWatchModule(t *testing.T) {
	// create rpc server and client
	stateMgr, rpcServer, rpcClient := createRPCServerClient(t)
	Assert(t, (stateMgr != nil) && (rpcServer != nil) && (rpcClient != nil), "Err creating rpc server")
	defer stateMgr.Stop()
	defer rpcServer.Stop()
	defer rpcClient.Close()

	const (
		node1   = "node1"
		node2   = "node2"
		module1 = "mod1"
		module2 = "mod2"
	)

	ctx, cancel := context.WithTimeout(context.Background(), 6*time.Second)
	defer cancel()

	mod1 := &diagnostics.Module{}
	mod1.Defaults("all")
	mod1.Name = module1
	mod1.Spec.LogLevel = diagnostics.ModuleSpec_Debug.String()
	mod1.Status.Node = node1
	mod1.Status.Category = diagnostics.ModuleStatus_Naples.String()

	// create diagnostics API client
	tsRPCClient := tsproto.NewDiagnosticsApiClient(rpcClient.ClientConn)

	// create one request before the agent starts watching
	evt1 := kvstore.WatchEvent{
		Type:   kvstore.Created,
		Object: mod1,
	}
	stateMgr.DiagnosticsModuleWatcher <- evt1

	// agent on node1 starts watching a module by name and immediately receives notification for mod1
	opt1 := &api.ListWatchOptions{}
	opt1.Name = module1
	rxStream1, err := tsRPCClient.WatchModule(ctx, opt1)
	AssertOk(t, err, "Error starting module watch for agent on node %s", node1)
	mustReadModuleEvent(t, rxStream1, module1)
	// send update event
	evt1 = kvstore.WatchEvent{
		Type:   kvstore.Updated,
		Object: mod1,
	}
	stateMgr.DiagnosticsModuleWatcher <- evt1
	mustReadModuleEvent(t, rxStream1, module1)

	// agent on node2 starts watching by field selector on node field in status
	opt2 := &api.ListWatchOptions{
		FieldSelector: fmt.Sprintf("status.node=%s", node2),
	}
	rxStream2, err := tsRPCClient.WatchModule(ctx, opt2)
	AssertOk(t, err, "Error starting module watch for agent on node %s", node2)

	// now create mod2. agent on node2 get a notification
	mod2 := &diagnostics.Module{}
	mod2.Defaults("all")
	mod2.Name = module2
	mod2.Spec.LogLevel = diagnostics.ModuleSpec_Debug.String()
	mod2.Status.Node = node2
	mod2.Status.Category = diagnostics.ModuleStatus_Naples.String()
	evt2 := kvstore.WatchEvent{
		Type:   kvstore.Created,
		Object: mod2,
	}
	stateMgr.DiagnosticsModuleWatcher <- evt2

	mustReadModuleEvent(t, rxStream2, module2)

}

func mustReadModuleEvent(t *testing.T, stream tsproto.DiagnosticsApi_WatchModuleClient, name string) {
	ctx, cancel := context.WithTimeout(stream.Context(), 6*time.Second)
	defer cancel()
	recvFn := func(ctx context.Context) (interface{}, error) {
		modObj, err := stream.Recv()
		AssertOk(t, err, "Error reading event from stream")
		Assert(t, modObj.Spec.LogLevel == diagnostics.ModuleSpec_Debug.String(),
			fmt.Sprintf("invalid log level. Have: %s, Want: %s", modObj.Spec.LogLevel, diagnostics.ModuleSpec_Debug.String()))
		Assert(t, modObj.Name == name, fmt.Sprintf("invalid module name. Have: %s, Want: %s", modObj.Name, name))
		return nil, nil
	}
	_, err := utils.ExecuteWithContext(ctx, recvFn)
	AssertOk(t, err, "Timeout reading event from stream")
}
