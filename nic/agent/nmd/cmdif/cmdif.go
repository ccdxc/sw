// {C} Copyright 2017 Pensando Systems Inc. All rights reserved.

package cmdif

import (
	"context"
	"fmt"
	"strings"
	"sync"
	"time"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/api/generated/cmd"
	"github.com/pensando/sw/nic/agent/nmd/state"
	"github.com/pensando/sw/venice/cmd/grpc"
	"github.com/pensando/sw/venice/cmd/grpc/server/smartnic"
	"github.com/pensando/sw/venice/utils/balancer"
	"github.com/pensando/sw/venice/utils/log"
	"github.com/pensando/sw/venice/utils/resolver"
	"github.com/pensando/sw/venice/utils/rpckit"
)

// CmdClient is the client of CMD server running on Venice node
type CmdClient struct {
	sync.Mutex     // Lock for CmdClient
	sync.WaitGroup // wait group to wait on all go routines to exit

	srvURL         string             // CMD rpc server URL
	resolverURLs   string             // Resolver URLs
	resolverClient resolver.Interface // Resolver client
	rpcClient      *rpckit.RPCClient  // RPC client

	nmd         state.NmdAPI       // NMD instance
	watchCtx    context.Context    // ctx for object watch
	watchCancel context.CancelFunc // cancel for object watch
}

// objectKey returns object key from object meta
func objectKey(meta api.ObjectMeta) string {
	return fmt.Sprintf("%s|%s", meta.Tenant, meta.Name)
}

// NewCmdClient creates an CMD client object
func NewCmdClient(nmd state.NmdAPI, srvURL, resolverURLs string) (*CmdClient, error) {

	// watch contexts
	watchCtx, watchCancel := context.WithCancel(context.Background())

	// create CmdClient object
	client := CmdClient{
		srvURL:         srvURL,
		resolverURLs:   resolverURLs,
		resolverClient: resolver.New(&resolver.Config{Servers: strings.Split(resolverURLs, ",")}),
		nmd:            nmd,
		watchCtx:       watchCtx,
		watchCancel:    watchCancel,
	}

	// register the NMD client as a controller plugin
	err := nmd.RegisterCMD(&client)
	if err != nil {
		log.Errorf("Error registering the controller interface. Err: %v", err)
		return nil, err
	}

	// initialize rpc client
	err = client.initRPC()
	if err != nil {
		return nil, err
	}

	// start watching objects
	go client.runSmartNICWatcher(client.watchCtx)

	return &client, nil
}

func (client *CmdClient) initRPC() error {

	client.Lock()
	defer client.Unlock()

	// initialize rpcClient
	var err error
	log.Infof("Initializing RPC client ")
	client.rpcClient, err = rpckit.NewRPCClient("nmd", client.srvURL, rpckit.WithBalancer(balancer.New(client.resolverClient)))
	if err != nil {
		log.Errorf("Error connecting to grpc server, srvURL: %v Err: %v", client.srvURL, err)
	}
	return err
}

func (client *CmdClient) closeRPC() {

	client.Lock()
	defer client.Unlock()
	if client.rpcClient != nil {
		log.Errorf("Closing RPC client")
		client.rpcClient.Close()
		client.rpcClient = nil
	}
}

func (client *CmdClient) getRPCClient() *rpckit.RPCClient {
	client.Lock()
	defer client.Unlock()
	return client.rpcClient
}

// getAgentName returns a unique name for each nmd instance
func (client *CmdClient) getAgentName() string {
	return "nmd-" + client.nmd.GetAgentID()
}

// runSmartNICWatcher runs smartNIC watcher loop
func (client *CmdClient) runSmartNICWatcher(ctx context.Context) {
	// setup wait group
	client.Add(1)
	defer client.Done()

	for {

		log.Infof("Starting SmartNIC watcher")
		// Initialize RPC client if required
		if client.getRPCClient() == nil {
			err := client.initRPC()
			if err != nil {
				if client.watchCtx.Err() == context.Canceled {
					log.Info("Context Cancelled, exiting smartNIC watcher")
					return
				}
				time.Sleep(time.Second)
				continue
			}
		}

		// Start the watch on SmartNIC object
		// TODO : Set the filter to watch only for local NIC's MAC
		//        The local NIC's MAC is obtained as part REST API
		//        configuration for naples when it is set to managed mode
		smartNICRPCClient := grpc.NewSmartNICClient(client.rpcClient.ClientConn)
		stream, err := smartNICRPCClient.WatchNICs(ctx, &api.ObjectMeta{})
		if err != nil {
			log.Errorf("Error watching smartNIC: Err: %v watchCtx.Err: %v",
				err, client.watchCtx.Err())

			if client.watchCtx.Err() == context.Canceled {
				log.Info("Context Cancelled, exiting smartNIC watcher")
				return
			}

			// For RPC errors, need to close current client and re-init
			client.closeRPC()
			time.Sleep(time.Second)
			continue
		}

		// Watcher loop
		for {
			// receive from stream
			evt, err := stream.Recv()
			if err != nil {
				log.Errorf("Error receiving from watch channel. Exiting nic watch. Err: %v watchCtx.Err: %v",
					err, client.watchCtx.Err())

				if client.watchCtx.Err() == context.Canceled {
					log.Info("Context Cancelled, exiting smartNIC watcher")
					return
				}

				// For RPC errors, need to close current client and re-init
				client.closeRPC()
				time.Sleep(time.Second)
				break
			}

			log.Infof("Ctrlerif: Got nic watch event: {%+v} type: %v", evt.Nic, evt.EventType)

			// Ignore events for non-local NIC
			// In managed mode, client.nmd.nic holds the initial configuration obtained
			// from REST API. GetSmartNIC() returns this inital config object.
			nic, err := client.nmd.GetSmartNIC()
			if err != nil || nic == nil || evt.Nic.Name != nic.GetName() {
				log.Infof("Ignoring non-local nics, local-nic: %+v, rcvd-nic: %+v", nic, evt.Nic)
				continue
			}

			switch evt.EventType {
			case api.EventType_CreateEvent:
				// create the nic
				err = client.nmd.CreateSmartNIC(&evt.Nic)
				if err != nil {
					log.Errorf("Error creating the smartNIC {%+v}. Err: %v", evt, err)
				}
				// TODO: handle errors, in accordance with platform API which is TBD
			case api.EventType_UpdateEvent:
				// update the nic
				err = client.nmd.UpdateSmartNIC(&evt.Nic)
				if err != nil {
					log.Errorf("Error updating the smartNIC {%+v}. Err: %v", evt, err)
				}
				// TODO: handle errors, in accordance with platform API which is TBD
			case api.EventType_DeleteEvent:
				// delete the nic
				err = client.nmd.DeleteSmartNIC(&evt.Nic)
				if err != nil {
					log.Errorf("Error deleting the smartNIC {%+v}. Err: %v", evt, err)
				}
				// TODO: handle errors, in accordance with platform API which is TBD
			}
		}
	}
}

// Stop stops CmdClient  and all watching go routines
func (client *CmdClient) Stop() {

	client.watchCancel()
	client.Wait()

	client.closeRPC()

	if client.resolverClient != nil {
		client.resolverClient.Stop()
	}
}

// RegisterSmartNICReq send a register request for SmartNIC to CMD
func (client *CmdClient) RegisterSmartNICReq(nic *cmd.SmartNIC) (grpc.RegisterNICResponse, error) {

	if client.getRPCClient() == nil {
		// initialize rpc client
		err := client.initRPC()
		if err != nil {
			return grpc.RegisterNICResponse{Phase: cmd.SmartNICSpec_UNKNOWN.String()}, err
		}
	}

	// make an RPC call to controller
	nicRPCClient := grpc.NewSmartNICClient(client.getRPCClient().ClientConn)
	req := grpc.RegisterNICRequest{
		Nic:  *nic,
		Cert: getFactoryCert(),
	}
	resp, err := nicRPCClient.RegisterNIC(context.Background(), &req)
	if err != nil || resp == nil {
		log.Errorf("Error resp from CMD for registerNIC, Nic: %v Err: %v Resp: %v",
			nic.Name, err, resp)
		return grpc.RegisterNICResponse{}, err
	}

	return *resp, nil
}

// UpdateSmartNICReq send a status update of SmartNIC to CMD
func (client *CmdClient) UpdateSmartNICReq(nic *cmd.SmartNIC) (*cmd.SmartNIC, error) {

	if client.getRPCClient() == nil {
		// initialize rpc client
		err := client.initRPC()
		if err != nil {
			return nil, err
		}
	}

	// make an RPC call to controller
	nicRPCClient := grpc.NewSmartNICClient(client.getRPCClient().ClientConn)
	req := grpc.UpdateNICRequest{
		Nic: *nic,
	}
	resp, err := nicRPCClient.UpdateNIC(context.Background(), &req)
	if err != nil || resp == nil {
		log.Errorf("Error resp from CMD for updateNIC, Nic: %v Err: %v Resp: %v",
			nic.Name, err, resp)
		return nil, err
	}

	return resp.GetNic(), err
}

// getFactoryCert
// TODO: get factory cert from platform
func getFactoryCert() []byte {
	return []byte(smartnic.ValidCertSignature)
}
