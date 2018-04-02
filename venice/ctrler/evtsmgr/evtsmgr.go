// {C} Copyright 2018 Pensando Systems Inc. All rights reserved.

package evtsmgr

import (
	"fmt"
	"time"

	"github.com/pkg/errors"

	"github.com/pensando/sw/venice/ctrler/evtsmgr/rpcserver"
	"github.com/pensando/sw/venice/globals"
	"github.com/pensando/sw/venice/utils"
	"github.com/pensando/sw/venice/utils/elastic"
	"github.com/pensando/sw/venice/utils/log"
	"github.com/pensando/sw/venice/utils/resolver"
)

var (
	// maxRetries maximum number of retries for fetching elasticsearch URLs
	// and creating client.
	maxRetries = 60

	// delay between retries
	retryDelay = 2 * time.Second
)

// EventsManager instance of events manager; responsible for all aspects of events
// including management of elastic connections.
type EventsManager struct {
	// RPCServer that exposes the server implementation of event manager APIs
	RPCServer *rpcserver.RPCServer
}

// NewEventsManager returns a events manager/controller instance
func NewEventsManager(serverName, serverURL string, resolverClient resolver.Interface,
	logger log.Logger) (*EventsManager, error) {
	if utils.IsEmpty(serverName) || utils.IsEmpty(serverURL) || resolverClient == nil || logger == nil {
		return nil, errors.New("all parameters are required")
	}

	// create elastic client
	esClient, err := createElasticClient(resolverClient, logger)
	if err != nil {
		return nil, err
	}

	// create RPC server
	rpcServer, err := rpcserver.NewRPCServer(serverName, serverURL, esClient)
	if err != nil {
		return nil, errors.Wrap(err, "error instantiating RPC server")
	}

	return &EventsManager{
		RPCServer: rpcServer,
	}, nil
}

// createElasticClient helper function to create elastic client by fetching the
// elasticsearch details using the resolver and create a client using that.
func createElasticClient(resolverClient resolver.Interface, logger log.Logger) (elastic.ESClient, error) {
	var err error
	elasticURL, err := getElasticSearchURLs(resolverClient)
	if err != nil {
		return nil, err
	}

	elasticURL = fmt.Sprintf("http://%s", elasticURL)

	var esClient elastic.ESClient
	for i := 0; i < maxRetries; i++ {
		esClient, err = elastic.NewClient(elasticURL, logger.WithContext("submodule", "elastic"))
		if err == nil {
			log.Info("created ES client")
			return esClient, nil
		}

		time.Sleep(retryDelay)
		log.Debugf("error creating ES client, err: %v, retrying.", err)
	}

	return nil, errors.Wrap(err, "failed to create ES client")
}

// getElasticSearchURLs helper function to get the elasticsearch URL using the resolver.
func getElasticSearchURLs(resolverClient resolver.Interface) (string, error) {
	for i := 0; i < maxRetries; i++ {
		elasticURLs := resolverClient.GetURLs(globals.ElasticSearch)
		if len(elasticURLs) > 0 {
			log.Debugf("list of elastic URLs found %v", elasticURLs)
			return elasticURLs[0], nil
		}

		time.Sleep(retryDelay)
		log.Debug("couldn't find elasticsearch. retrying.")
	}

	return "", fmt.Errorf("failed to get `%v` URLs using the resolver", globals.ElasticSearch)
}
