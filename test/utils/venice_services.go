// {C} Copyright 2018 Pensando Systems Inc. All rights reserved.

package utils

import (
	"context"
	"fmt"
	"io/ioutil"
	"net"
	"time"

	"github.com/pensando/sw/api/client"
	"github.com/pensando/sw/api/generated/auth"
	loginctx "github.com/pensando/sw/api/login/context"
	"github.com/pensando/sw/venice/apigw"
	apigwpkg "github.com/pensando/sw/venice/apigw/pkg"
	"github.com/pensando/sw/venice/apiserver"
	apiserverpkg "github.com/pensando/sw/venice/apiserver/pkg"
	"github.com/pensando/sw/venice/ctrler/evtsmgr"
	"github.com/pensando/sw/venice/evtsproxy"
	"github.com/pensando/sw/venice/globals"
	"github.com/pensando/sw/venice/spyglass/finder"
	"github.com/pensando/sw/venice/spyglass/indexer"
	authntestutils "github.com/pensando/sw/venice/utils/authn/testutils"
	"github.com/pensando/sw/venice/utils/kvstore/store"
	"github.com/pensando/sw/venice/utils/log"
	mockresolver "github.com/pensando/sw/venice/utils/resolver/mock"
	"github.com/pensando/sw/venice/utils/runtime"
)

var (
	// TestLocalUser test username
	TestLocalUser = "test"
	// TestLocalPassword test password
	TestLocalPassword = "pensando"
	// TestTenant test tenant
	TestTenant = globals.DefaultTenant
)

// SetupAuth setsup the authentication service
func SetupAuth(apiServerAddr string, enableLocalAuth, enableLdapAuth bool, creds *auth.PasswordCredential, logger log.Logger) error {
	// create API server client
	apiClient, err := client.NewGrpcUpstream("venice_integ_test", apiServerAddr, logger)
	if err != nil {
		return fmt.Errorf("failed to create gRPC client, err: %v", err)
	}
	defer apiClient.Close()

	// create authentication policy
	authntestutils.MustCreateAuthenticationPolicy(apiClient, &auth.Local{Enabled: enableLocalAuth}, &auth.Ldap{Enabled: enableLdapAuth})
	if enableLocalAuth && creds != nil { // create local user
		authntestutils.MustCreateTestUser(apiClient, creds.GetUsername(), creds.GetPassword(), creds.GetTenant())
	}

	return nil
}

// GetAuthorizationHeader helper function to login and get the authZ header from login context
func GetAuthorizationHeader(apiGwAddr string, creds *auth.PasswordCredential) (string, error) {
	ctx, err := authntestutils.NewLoggedInContext(context.Background(), fmt.Sprintf("http://%s", apiGwAddr), creds)
	if err != nil {
		return "", err
	}

	// get authz header
	authzHeader, ok := loginctx.AuthzHeaderFromContext(ctx)
	if !ok {
		return "", fmt.Errorf("failed to get authorization header from context")
	}

	return authzHeader, nil
}

// StartAPIServer helper function to start API server
func StartAPIServer(serverAddr string, kvstoreConfig *store.Config, l log.Logger) (apiserver.Server, string, error) {
	log.Infof("starting API server ...")

	// Create api server
	apiServerAddress := serverAddr
	scheme := runtime.GetDefaultScheme()
	srvConfig := apiserver.Config{
		GrpcServerPort: apiServerAddress,
		DebugMode:      false,
		Logger:         l,
		Version:        "v1",
		Scheme:         scheme,
		KVPoolSize:     8,
		Kvstore:        *kvstoreConfig,
	}

	apiServer := apiserverpkg.MustGetAPIServer()
	go apiServer.Run(srvConfig)
	apiServer.WaitRunning()

	apiServerAddr, err := apiServer.GetAddr()
	if err != nil {
		return nil, "", fmt.Errorf("failed to get API server addr, err: %v", err)
	}
	port, err := getPortFromAddr(apiServerAddr)
	if err != nil {
		return nil, "", err
	}

	localAddr := fmt.Sprintf("localhost:%s", port)
	log.Infof("API server running on %v", localAddr)
	return apiServer, localAddr, nil
}

// StartAPIGateway helper function to start API gateway.
func StartAPIGateway(serverAddr string, backends map[string]string, resolvers []string, l log.Logger) (apigw.APIGateway, string, error) {
	log.Info("starting API gateway ...")

	// Start the API Gateway
	gwConfig := apigw.Config{
		HTTPAddr:        serverAddr,
		DebugMode:       true,
		Logger:          l,
		BackendOverride: backends,
		Resolvers:       resolvers,
		SkipBackends: []string{
			"metrics_query", //TODO fix after hookup
		},
	}

	if _, ok := backends[globals.APIServer]; !ok {
		gwConfig.SkipBackends = append(gwConfig.SkipBackends, globals.APIServer)
	}

	if _, ok := backends[globals.Spyglass]; !ok {
		gwConfig.SkipBackends = append(gwConfig.SkipBackends, globals.Spyglass)
	}

	apiGw := apigwpkg.MustGetAPIGateway()
	go apiGw.Run(gwConfig)
	apiGw.WaitRunning()

	apiGwAddr, err := apiGw.GetAddr()
	if err != nil {
		return nil, "", fmt.Errorf("failed to get API gateway addr, err: %v", err)
	}
	port, err := getPortFromAddr(apiGwAddr.String())
	if err != nil {
		return nil, "", err
	}

	localAddr := fmt.Sprintf("localhost:%s", port)
	log.Infof("API gateway running on %v", localAddr)
	return apiGw, localAddr, nil
}

// StartSpyglass helper function to spyglass finder and indexer
func StartSpyglass(service, apiServerAddr string, mr *mockresolver.ResolverClient, logger log.Logger) (interface{}, string, error) {
	switch service {
	case "finder": // create finder
		log.Info("starting finder ...")
		ctx := context.Background()
		fdr, err := finder.NewFinder(ctx, "localhost:0", mr, logger)
		if err != nil {
			return nil, "", fmt.Errorf("failed to create finder, err: %v", err)
		}
		err = fdr.Start() // start the finder
		if err != nil {
			return nil, "", fmt.Errorf("failed to start finder, err: %v", err)
		}

		finderAddr := fdr.GetListenURL()
		if err != nil {
			return nil, "", fmt.Errorf("failed to get API server addr, err: %v", err)
		}
		port, err := getPortFromAddr(finderAddr)
		if err != nil {
			return nil, "", err
		}

		localAddr := fmt.Sprintf("localhost:%s", port)
		log.Infof("Spyglass finder running on %v", localAddr)
		return fdr, localAddr, nil
	case "indexer": // create the indexer
		log.Info("starting indexer ...")
		ctx := context.Background()
		idr, err := indexer.NewIndexer(ctx, apiServerAddr, mr, logger)
		if err != nil {
			return nil, "", fmt.Errorf("failed to create indexer, err: %v", err)
		}
		err = idr.Start() // start the indexer
		if err != nil {
			return nil, "", fmt.Errorf("failed to start indexer, err: %v", err)
		}

		return idr, "", nil
	}

	return nil, "", nil
}

// StartEvtsMgr helper function to start events manager
func StartEvtsMgr(serverAddr string, mr *mockresolver.ResolverClient, logger log.Logger) (*evtsmgr.EventsManager, string, error) {
	log.Infof("starting events manager")

	evtsMgr, err := evtsmgr.NewEventsManager(globals.EvtsMgr, serverAddr, mr, logger)
	if err != nil {
		return nil, "", fmt.Errorf("failed start events manager, err: %v", err)
	}

	return evtsMgr, evtsMgr.RPCServer.GetListenURL(), nil
}

// StartEvtsProxy helper function to start events proxy
func StartEvtsProxy(serverAddr string, mr *mockresolver.ResolverClient, logger log.Logger) (*evtsproxy.EventsProxy, string, string, error) {
	log.Infof("starting events proxy")

	if len(mr.GetURLs(globals.EvtsMgr)) == 0 {
		return nil, "", "", fmt.Errorf("could not find evtsmgr URL")
	}

	proxyEventsStoreDir, err := ioutil.TempDir("", "")
	if err != nil {
		log.Errorf("failed to create temp events dir, err: %v", err)
		return nil, "", "", err
	}

	evtsProxy, err := evtsproxy.NewEventsProxy(globals.EvtsProxy, serverAddr,
		mr.GetURLs(globals.EvtsMgr)[0], nil, 10*time.Second, 100*time.Millisecond, proxyEventsStoreDir,
		[]evtsproxy.WriterType{evtsproxy.Venice}, logger)
	if err != nil {
		return nil, "", "", fmt.Errorf("failed start events proxy, err: %v", err)
	}

	return evtsProxy, evtsProxy.RPCServer.GetListenURL(), proxyEventsStoreDir, nil
}

// helper function to parse the port from given address <ip:port>
func getPortFromAddr(addr string) (string, error) {
	_, port, err := net.SplitHostPort(addr)
	if err != nil {
		return "", fmt.Errorf("failed to parse API server addr, err: %v", err)
	}

	return port, nil
}
