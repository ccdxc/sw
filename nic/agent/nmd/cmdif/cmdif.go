// {C} Copyright 2017 Pensando Systems Inc. All rights reserved.

package cmdif

import (
	"context"
	"fmt"
	"sync"
	"time"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/api/generated/cmd"
	"github.com/pensando/sw/nic/agent/nmd/state"
	"github.com/pensando/sw/venice/cmd/grpc"
	"github.com/pensando/sw/venice/globals"
	"github.com/pensando/sw/venice/utils/balancer"
	"github.com/pensando/sw/venice/utils/debug"
	"github.com/pensando/sw/venice/utils/log"
	"github.com/pensando/sw/venice/utils/resolver"
	"github.com/pensando/sw/venice/utils/rpckit"
)

// Temporary definition, until CKM provides a API to validate Cert
const (
	ValidCertSignature = "O=Pensando Systems, Inc., OU=Pensando Manufacturing, CN=Pensando Manufacturing CA/emailAddress=mfgca@pensando.io"
)

// CmdClient is the client of CMD server running on Venice node
type CmdClient struct {
	sync.Mutex     // Lock for CmdClient
	sync.WaitGroup // wait group to wait on all go routines to exit

	cmdRegistrationURL    string             // CMD endpoint for the NIC registration API
	cmdUpdatesURL         string             // CMD endpoint for NIC watches and updates
	resolverClient        resolver.Interface // resolver Interface
	registrationRPCClient *rpckit.RPCClient  // RPC client for NIC registration API
	updatesRPCClient      *rpckit.RPCClient  // RPC client for NIC watches and updates
	nmd                   state.NmdAPI       // NMD instance
	watchCtx              context.Context    // ctx for object watch
	watchCancel           context.CancelFunc // cancel for object watch
	debugStats            *debug.Stats
	startTime             time.Time
}

// objectKey returns object key from object meta
func objectKey(meta api.ObjectMeta) string {
	return fmt.Sprintf("%s|%s", meta.Tenant, meta.Name)
}

// NewCmdClient creates an CMD client object
func NewCmdClient(nmd state.NmdAPI, cmdRegistrationURL, cmdUpdatesURL string, resolverClient resolver.Interface) (*CmdClient, error) {

	// watch contexts
	watchCtx, watchCancel := context.WithCancel(context.Background())

	// create CmdClient object
	client := CmdClient{
		cmdRegistrationURL: cmdRegistrationURL,
		cmdUpdatesURL:      cmdUpdatesURL,
		resolverClient:     resolverClient,
		nmd:                nmd,
		watchCtx:           watchCtx,
		watchCancel:        watchCancel,
		startTime:          time.Now(),
	}

	client.debugStats = debug.New(fmt.Sprintf("cmdif-%s", client.getAgentName())).Tsdb().Kind("nmdStats").TsdbPeriod(5 * time.Second).Build()

	// register the NMD client as a controller plugin
	err := nmd.RegisterCMD(&client)
	if err != nil {
		log.Errorf("Error registering the controller interface. Err: %v", err)
		return nil, err
	}

	// start watching objects
	go client.runSmartNICWatcher(client.watchCtx)

	return &client, nil
}

func (client *CmdClient) initRegistrationRPC() error {

	client.Lock()
	defer client.Unlock()

	// initialize rpcClient
	var err error
	log.Infof("Initializing NIC registration RPC client ")
	client.registrationRPCClient, err = rpckit.NewRPCClient("nmd-nic-reg", client.cmdRegistrationURL, rpckit.WithBalancer(balancer.New(client.resolverClient)), rpckit.WithTLSProvider(nil))
	if err != nil {
		log.Errorf("Error connecting to grpc server for NIC registration, URL: %v Err: %v", client.cmdRegistrationURL, err)
	}
	return err
}

func (client *CmdClient) initUpdatesRPC() error {

	client.Lock()
	defer client.Unlock()

	// initialize rpcClient
	var err error
	log.Infof("Initializing NIC updates RPC client ")
	client.updatesRPCClient, err = rpckit.NewRPCClient("nmd-nic-upd", client.cmdUpdatesURL,
		rpckit.WithBalancer(balancer.New(client.resolverClient)), rpckit.WithRemoteServerName(globals.Cmd))
	if err != nil {
		log.Errorf("Error connecting to grpc server for NIC updates, URL: %v Err: %v", client.cmdUpdatesURL, err)
	}
	return err
}

func (client *CmdClient) closeRegistrationRPC() {
	client.Lock()
	defer client.Unlock()
	if client.registrationRPCClient != nil {
		log.Errorf("Closing nmd-nic-reg RPC client")
		client.registrationRPCClient.Close()
		client.registrationRPCClient = nil
	}
}

func (client *CmdClient) closeUpdatesRPC() {

	client.Lock()
	defer client.Unlock()
	if client.updatesRPCClient != nil {
		log.Errorf("Closing nmd-nic-upd RPC client")
		client.updatesRPCClient.Close()
		client.updatesRPCClient = nil
	}
}

func (client *CmdClient) getRegistrationRPCClient() *rpckit.RPCClient {
	client.Lock()
	defer client.Unlock()
	return client.registrationRPCClient
}

func (client *CmdClient) getUpdatesRPCClient() *rpckit.RPCClient {
	client.Lock()
	defer client.Unlock()
	return client.updatesRPCClient
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
		if client.getUpdatesRPCClient() == nil {
			err := client.initUpdatesRPC()
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
		smartNICRPCClient := grpc.NewSmartNICUpdatesClient(client.updatesRPCClient.ClientConn)
		stream, err := smartNICRPCClient.WatchNICs(ctx, &api.ObjectMeta{})
		if err != nil {
			log.Errorf("Error watching smartNIC: Err: %v watchCtx.Err: %v",
				err, client.watchCtx.Err())

			if client.watchCtx.Err() == context.Canceled {
				log.Info("Context Cancelled, exiting smartNIC watcher")
				return
			}

			// For RPC errors, need to close current client and re-init
			client.closeUpdatesRPC()
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
					client.closeUpdatesRPC()
					return
				}

				// For RPC errors, need to close current client and re-init
				client.closeUpdatesRPC()
				time.Sleep(time.Second)
				break
			}

			log.Infof("++++ CMDIF: Got nic watch event: {%+v} type: %v", evt.Nic, evt.EventType)

			// Ignore events for non-local NIC
			// In managed mode, client.nmd.nic holds the initial configuration obtained
			// from REST API. GetSmartNIC() returns this initial config object.
			nic, err := client.nmd.GetSmartNIC()
			if err != nil || nic == nil || evt.Nic.Name != nic.GetName() {
				log.Debugf("Ignoring non-local nics, local-nic: %+v, rcvd-nic: %+v", nic, evt.Nic)
				continue
			}

			t, err := evt.Nic.ModTime.Time()
			if err == nil && client.startTime.Before(t) && evt.EventType != api.EventType_DeleteEvent {
				latency := time.Since(t)
				if latency >= 0 {
					client.debugStats.AddFloat("cum_nic_latency", float64(latency))
				}
			}
			switch evt.EventType {
			case api.EventType_CreateEvent:
				client.debugStats.AddFloat("nic_create", 1.0)
				// create the nic
				err = client.nmd.CreateSmartNIC(&evt.Nic)
				if err != nil {
					log.Errorf("Error creating the smartNIC {%+v}. Err: %v", evt, err)
				}
				// TODO: handle errors, in accordance with platform API which is TBD
			case api.EventType_UpdateEvent:
				client.debugStats.AddFloat("nic_update", 1.0)
				// update the nic
				err = client.nmd.UpdateSmartNIC(&evt.Nic)
				if err != nil {
					log.Errorf("Error updating the smartNIC {%+v}. Err: %v", evt, err)
				}
				// TODO: handle errors, in accordance with platform API which is TBD
			case api.EventType_DeleteEvent:
				client.debugStats.AddFloat("nic_delete", 1.0)
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

	client.closeUpdatesRPC()
	client.closeRegistrationRPC()
}

// RegisterSmartNICReq send a register request for SmartNIC to CMD
func (client *CmdClient) RegisterSmartNICReq(nic *cmd.SmartNIC) (grpc.RegisterNICResponse, error) {

	if client.getRegistrationRPCClient() == nil {
		// initialize rpc client
		err := client.initRegistrationRPC()
		if err != nil {
			return grpc.RegisterNICResponse{Phase: cmd.SmartNICSpec_UNKNOWN.String()}, err
		}
	}

	// make an RPC call to controller
	nicRPCClient := grpc.NewSmartNICRegistrationClient(client.getRegistrationRPCClient().ClientConn)
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

	if client.getUpdatesRPCClient() == nil {
		// initialize rpc client
		err := client.initUpdatesRPC()
		if err != nil {
			return nil, err
		}
	}

	// make an RPC call to controller
	nicRPCClient := grpc.NewSmartNICUpdatesClient(client.getUpdatesRPCClient().ClientConn)
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
	return []byte(ValidCertSignature)
}
