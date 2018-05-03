// {C} Copyright 2018 Pensando Systems Inc. All rights reserved.

package evtsmgr

import (
	"context"
	"fmt"
	"time"

	"github.com/pkg/errors"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/api/generated/monitoring"
	"github.com/pensando/sw/venice/ctrler/evtsmgr/rpcserver"
	"github.com/pensando/sw/venice/globals"
	"github.com/pensando/sw/venice/utils"
	"github.com/pensando/sw/venice/utils/elastic"
	mapper "github.com/pensando/sw/venice/utils/elastic/mapper"
	"github.com/pensando/sw/venice/utils/log"
	"github.com/pensando/sw/venice/utils/resolver"
)

var (
	// maxRetries maximum number of retries for fetching elasticsearch URLs
	// and creating client.
	maxRetries = 60

	// delay between retries
	retryDelay = 2 * time.Second

	// generate elastic mapping for an event object
	eventSkeleton = monitoring.Event{
		EventAttributes: monitoring.EventAttributes{
			// Need to make sure pointer fields are valid to
			// generate right mappings using reflect
			ObjectRef: &api.ObjectRef{},
			Source:    &monitoring.EventSource{},
		},
	}
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
	result, err := utils.ExecuteWithRetry(func() (interface{}, error) {
		return elastic.NewClient("", resolverClient, logger.WithContext("submodule", "elastic"))
	}, retryDelay, maxRetries)
	if err != nil {
		logger.Errorf("failed to create elastic client, err: %v", err)
		return nil, err
	}
	logger.Debugf("created elastic client")
	esClient := result.(elastic.ESClient)

	// create events template; once the template is created, elasticsearch automatically
	// applies the properties for any new indices that's matching the pattern. As the index call
	// automatically creates the index when it does not exists, we don't need to explicitly
	// create daily events index.
	if err = createEventsElasticTemplate(esClient); err != nil {
		logger.Errorf("failed to create events template in elastic, err: %v", err)
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

// createEventsElasticTemplate helper function to create index template for events.
func createEventsElasticTemplate(esClient elastic.ESClient) error {
	docType := elastic.GetDocType(globals.Events)
	mapping, err := mapper.ElasticMapper(eventSkeleton, docType, mapper.WithShardCount(3), mapper.WithReplicaCount(1),
		mapper.WithIndexPatterns(fmt.Sprintf("*.%s.*", docType)))
	if err != nil {
		log.Errorf("failed get elastic mapping for event object {%v}, err: %v", eventSkeleton, err)
		return err
	}

	// JSON string mapping
	strMapping, err := mapping.JSONString()
	if err != nil {
		log.Errorf("failed to convert elastic mapping {%v} to JSON string", mapping)
		return err
	}

	// create events template
	if err := esClient.CreateIndexTemplate(context.Background(), elastic.GetTemplateName(globals.Events), strMapping); err != nil {
		log.Errorf("failed to create events index template, err: %v", err)
		return err
	}

	return nil
}
