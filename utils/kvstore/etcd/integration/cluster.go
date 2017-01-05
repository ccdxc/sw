// Package integration implements tcp for grpc between etcd client and server.
// etcd's integration package uses unix domain sockets for grpc between client
// and server. In a vagrant environment, unix domain sockets are not working.
package integration

import (
	"context"
	"net"
	"net/http"
	"net/http/httptest"
	"os"
	"strconv"
	"testing"
	"time"

	"google.golang.org/grpc"

	"github.com/coreos/etcd/clientv3"
	"github.com/coreos/etcd/etcdserver"
	"github.com/coreos/etcd/etcdserver/api/v2http"
	"github.com/coreos/etcd/etcdserver/api/v3rpc"
	"github.com/coreos/etcd/pkg/testutil"
	"github.com/coreos/etcd/pkg/types"
)

const (
	name             = "etcdServer"
	clusterToken     = "etcd"
	host             = "localhost"
	serverPort       = 21001
	clientPort       = 21002
	dataDir          = "/tmp/etcd"
	requestTimeout   = 5 * time.Second
	electionTicks    = 10
	bootstrapTimeout = 10 * time.Millisecond
	ticker           = 500 * time.Millisecond
)

// ClusterV3 contains server and client pointers for etcd.
type ClusterV3 struct {
	server     *etcdserver.EtcdServer
	client     *clientv3.Client
	httpServer *httptest.Server
	grpcServer *grpc.Server
}

// newEtcdServer creates a etcd server instance.
func newEtcdServer(t *testing.T) *etcdserver.EtcdServer {
	clientURL := "http://" + host + ":" + strconv.Itoa(clientPort)
	clientURLs, err := types.NewURLs([]string{clientURL})
	if err != nil {
		t.Fatalf("Failed to create client URLs with error: %v", err)
	}

	peerURL := "http://" + host + ":" + strconv.Itoa(serverPort)
	peerURLs, err := types.NewURLs([]string{peerURL})
	if err != nil {
		t.Fatalf("Failed to create peer URLs with error: %v", err)
	}

	peerURLsMap, err := types.NewURLsMap(name + "=http://" + host + ":" + strconv.Itoa(serverPort))
	if err != nil {
		t.Fatalf("Failed to create peer URLs map with error: %v", err)
	}

	serverConfig := &etcdserver.ServerConfig{
		Name:                name,
		ClientURLs:          clientURLs,
		PeerURLs:            peerURLs,
		NewCluster:          true,
		DataDir:             dataDir,
		InitialPeerURLsMap:  peerURLsMap,
		InitialClusterToken: clusterToken,
		ElectionTicks:       electionTicks,
		BootstrapTimeout:    bootstrapTimeout,
		TickMs:              electionTicks,
	}

	server, err := etcdserver.NewServer(serverConfig)
	if err != nil {
		t.Fatalf("Failed to create server with error: %v", err)
	}

	server.SyncTicker = time.Tick(ticker)
	server.Start()
	return server
}

// waitForServer waits for the etcd server to be Ready.
func waitForServer(t *testing.T, client *clientv3.Client) {
	// Try for up to a second.
	for ii := 0; ii < 100; ii++ {
		ctx, cancel := context.WithTimeout(context.Background(), requestTimeout)
		if _, err := client.KV.Get(ctx, "/"); err == nil {
			cancel()
			return
		}
		cancel()
		time.Sleep(bootstrapTimeout)
	}
	t.Fatalf("Failed waiting for etcd server to be Ready")
}

// NewClusterV3 creates and starts a new etcd server and client.
func NewClusterV3(t *testing.T) *ClusterV3 {
	if err := os.RemoveAll(dataDir); err != nil {
		t.Fatalf("Failed to remove data dir, error %v", err)
	}

	server := newEtcdServer(t)

	// Start the peer server.
	serverListener, err := net.Listen("tcp", host+":"+strconv.Itoa(serverPort))
	if err != nil {
		t.Fatalf("Failed to listen on server port %v, error %v", serverPort, err)
	}
	raftHandler := &testutil.PauseableHandler{Next: v2http.NewPeerHandler(server)}
	httpServer := &httptest.Server{
		Listener: serverListener,
		Config:   &http.Server{Handler: raftHandler},
	}
	httpServer.Start()

	// Start the client server (grpc).
	clientListener, err := net.Listen("tcp", host+":"+strconv.Itoa(clientPort))
	if err != nil {
		t.Fatalf("Failed to listen on client port %v, error %v", clientPort, err)
	}
	grpcServer := v3rpc.Server(server, nil)
	go grpcServer.Serve(clientListener)

	// Create a client.
	clientURL := "http://" + host + ":" + strconv.Itoa(clientPort)
	client, err := clientv3.New(clientv3.Config{
		Endpoints: []string{clientURL},
	})
	if err != nil {
		t.Fatalf("Failed to create client with error: %v", err)
	}

	// Wait for the server to be ready.
	waitForServer(t, client)

	return &ClusterV3{
		server:     server,
		client:     client,
		httpServer: httpServer,
		grpcServer: grpcServer,
	}
}

// Client returns the client object.
func (c *ClusterV3) Client() *clientv3.Client {
	return c.client
}

// NewClient returns a new client object.
func (c *ClusterV3) NewClient(t *testing.T) *clientv3.Client {
	// Create a client.
	clientURL := "http://" + host + ":" + strconv.Itoa(clientPort)
	client, err := clientv3.New(clientv3.Config{
		Endpoints: []string{clientURL},
	})
	if err != nil {
		t.Fatalf("Failed to create client with error: %v", err)
	}
	return client
}

// Terminate cleans up the state created by the test.
func (c *ClusterV3) Terminate(t *testing.T) {
	if c.grpcServer != nil {
		c.grpcServer.Stop()
	}
	if c.httpServer != nil {
		c.httpServer.CloseClientConnections()
		c.httpServer.Close()
	}
	c.server.HardStop()
	if err := os.RemoveAll(dataDir); err != nil {
		t.Fatalf("Failed to remove data dir, error %v", err)
	}
}
