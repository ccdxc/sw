package rolloutif

import (
	"context"
	"sync"
	"time"

	"github.com/pensando/sw/venice/ctrler/rollout/rpcserver/protos"
	"github.com/pensando/sw/venice/globals"

	"github.com/pensando/sw/api"
	nmdapi "github.com/pensando/sw/nic/agent/nmd/api"
	"github.com/pensando/sw/venice/utils/balancer"
	"github.com/pensando/sw/venice/utils/log"
	"github.com/pensando/sw/venice/utils/resolver"
	"github.com/pensando/sw/venice/utils/rpckit"
)

const rolloutURL = globals.Rollout

// RoClient is the client of Rollout Controller
type RoClient struct {
	sync.Mutex     // Lock
	sync.WaitGroup // wait group to wait on all go routines to exit

	watchCtx          context.Context    // ctx for object watch
	watchCancel       context.CancelFunc // cancel for object watch
	resolverClient    resolver.Interface // resolver Interface
	rpcClient         *rpckit.RPCClient  // RPC client for NIC watches and updates
	nmd               nmdapi.RolloutAPI
	smartNICRPCClient protos.DSCRolloutApiClient
	statusUpdateChan  chan protos.DSCRolloutStatusUpdate
	stopped           bool
	watcherRunning    bool
}

// NewRoClient creates an Rollout client object implementing RolloutAPI API
func NewRoClient(nmd nmdapi.RolloutAPI, resolverClient resolver.Interface) (nmdapi.RolloutCtrlAPI, error) {
	return newRoClient(nmd, resolverClient)
}

// newRoClient creates an Rollout client object implementing RolloutAPI API
func newRoClient(nmd nmdapi.RolloutAPI, resolverClient resolver.Interface) (*RoClient, error) {

	// watch contexts
	watchCtx, watchCancel := context.WithCancel(context.Background())

	// create RoClient object
	client := RoClient{
		resolverClient:   resolverClient,
		watchCtx:         watchCtx,
		watchCancel:      watchCancel,
		nmd:              nmd,
		statusUpdateChan: make(chan protos.DSCRolloutStatusUpdate, 16),
	}

	err := nmd.RegisterROCtrlClient(&client)
	if err != nil {
		log.Errorf("Error registering the Rollout controller interface. Err: %v", err)
	}

	client.Add(1)
	go client.statusUpdater()
	return &client, nil
}

// WatchDSCRolloutUpdates sets up the watch
func (client *RoClient) WatchDSCRolloutUpdates() error {
	client.Lock()
	defer client.Unlock()
	client.Add(1)
	// start watching objects
	client.watcherRunning = true
	go client.runDSCRolloutWatcher(client.watchCtx)
	return nil
}

func (client *RoClient) statusUpdater() {
	defer client.Done()
	for {
		select {
		case status, ok := <-client.statusUpdateChan:
			if !ok {
				return
			}
			for numRetries := 0; numRetries < 10; numRetries++ {
				client.Lock()
				rpcClient := client.smartNICRPCClient
				client.Unlock()

				if rpcClient == nil {
					log.Errorf("client is nil while updating smartnic rollout status. retrying..")
					time.Sleep(3 * time.Second)
					continue
				}
				ctx, cancel := context.WithTimeout(context.TODO(), 3*time.Second)
				_, err := rpcClient.UpdateDSCRolloutStatus(ctx, &status)
				cancel()
				if err != nil {
					log.Errorf("Error updating  smartNIC rollout status: Err: %v ", err)
				} else {
					break
				}
			}
		}
	}
}

// UpdateDSCRolloutStatus is called by NMD to inform rollout ctrler about the status of the rollout
func (client *RoClient) UpdateDSCRolloutStatus(status *protos.DSCRolloutStatusUpdate) error {
	client.Lock()
	defer client.Unlock()
	log.Infof("Sending rollout update status: %v to venice", *status)

	if client.stopped {
		log.Errorf("Rollout client is stopped. Failed to send update %v to venice. | RoClient: %v", *status, client)
		return nil
	}
	client.statusUpdateChan <- *status
	return nil
}

func (client *RoClient) initRPC() error {

	client.Lock()
	defer client.Unlock()

	// initialize rpcClient
	var err error
	log.Infof("Initializing smartNICRollout updates RPC client ")
	client.rpcClient, err = rpckit.NewRPCClient("nmd-ro", rolloutURL,
		rpckit.WithBalancer(balancer.NewWithName(client.resolverClient, "nmd-ro")), rpckit.WithRemoteServerName(globals.Rollout))
	if err != nil {
		log.Errorf("Error connecting to grpc server for NIC updates, URL: %v Err: %v", rolloutURL, err)
	} else {
		client.smartNICRPCClient = protos.NewDSCRolloutApiClient(client.rpcClient.ClientConn)
	}
	return err
}

func (client *RoClient) getGRPCClient() *rpckit.RPCClient {
	client.Lock()
	defer client.Unlock()
	return client.rpcClient
}

func (client *RoClient) closeGRPCClient() {

	client.Lock()
	defer client.Unlock()
	if client.rpcClient != nil {
		log.Errorf("Closing nmd-ro RPC client")
		client.rpcClient.Close()
		client.rpcClient = nil
		client.smartNICRPCClient = nil
	}
}

// Stop and free up any resources
func (client *RoClient) Stop() {
	log.Infof("Stopping Client: %+v", client)
	client.Lock()
	client.stopped = true
	client.watchCancel()
	client.watcherRunning = false
	close(client.statusUpdateChan)
	client.Unlock()

	client.Wait()
	client.closeGRPCClient()
}

// IsSmartNICWatcherRunning returns true if the client has an active watch on the rollout controller
func (client *RoClient) IsSmartNICWatcherRunning() bool {
	client.Lock()
	defer client.Unlock()
	return client.watcherRunning
}

// runSmartNICWatcher runs smartNIC watcher loop
func (client *RoClient) runDSCRolloutWatcher(ctx context.Context) {
	defer client.Done()

	// Outer loop gets a valid grpc client. In the inner loop, run a watcher
	for {

		log.Infof("Starting SmartNIC watcher")
		// Initialize RPC client if required
		if client.getGRPCClient() == nil {
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

		id := client.nmd.GetPrimaryMAC()
		if id == "" {
			log.Errorf("Self ID is nil. Retrying")
			time.Sleep(time.Second)
			continue
		}
		log.Errorf("starting rollout watch with ID %s", id)

		// Start the watch on SmartNIC object
		stream, err := client.smartNICRPCClient.WatchDSCRollout(ctx, &api.ObjectMeta{Name: id})
		if err != nil {
			log.Errorf("Error watching smartNIC: Err: %v watchCtx.Err: %v",
				err, client.watchCtx.Err())

			if client.watchCtx.Err() == context.Canceled {
				log.Info("Context Cancelled, exiting smartNIC watcher")
				return
			}

			// For RPC errors, need to close current client and re-init
			client.closeGRPCClient()
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
					client.closeGRPCClient()
					return
				}

				// For RPC errors, need to close current client and re-init
				client.closeGRPCClient()
				time.Sleep(time.Second)
				break
			}
			log.Infof("Recvd %#v %v", evt, err)

			switch evt.EventType {
			case api.EventType_CreateEvent, api.EventType_UpdateEvent:
				// create the nic
				err = client.nmd.CreateUpdateDSCRollout(&evt.DSCRollout)
				if err != nil {
					log.Errorf("Error creating the smartNIC {%+v}. Err: %v", evt, err)
				}
				// TODO: handle errors
			case api.EventType_DeleteEvent:

				// delete the nic
				err = client.nmd.DeleteDSCRollout()
				if err != nil {
					log.Errorf("Error deleting the smartNIC {%+v}. Err: %v", evt, err)
				}
				// TODO: handle errors
			}
		}
	}
}
