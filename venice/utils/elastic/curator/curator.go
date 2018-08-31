// {C} Copyright 2018 Pensando Systems Inc. All rights reserved.

package curator

import (
	"context"
	"fmt"
	"sync"
	"time"

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

// Curator represents elastic index manager service for
//  - handling index retention and expiration (for now)
//
// Please see testCurator() in test/integ/venice_integ/elastic/client_test.go
// for sample usage of this service.
type Curator struct {

	// Config is the configuration for a Curator instance
	Config

	// Wait group
	sync.WaitGroup

	// Elastic Client
	esClient elastic.ESClient

	// Channel to stop service
	stopCh chan bool
}

// Config is the config for the Curator
type Config struct {

	// Logger
	Logger log.Logger

	// IndexName is index prefix or index pattern
	// Eg: venice.internal.default.systemlogs.*
	//     venice*events*
	IndexName string

	// RetentionPeriod is index retention period
	// It is typically in days for a index category
	// For eg: 90 days for venice log index category
	RetentionPeriod time.Duration

	// ScanInterval determines scan interval or frequency
	// For eg: Scan indices every day (24hours)
	ScanInterval time.Duration

	// ElasticServer Address
	// This is an optional field, if provided
	// will be used to setup elastic client
	ElasticAddr string

	// Resolver client for pen-elastic service
	// which will be required when curator is used
	// by CMD and any of Venice controllers
	Resolver resolver.Interface
}

// NewCurator returns a new Curator service
func NewCurator(config *Config) (Interface, error) {

	// Validate input
	if config == nil {
		return nil, fmt.Errorf("Nil config")
	}

	// Validate config needed for elastic
	if config.ElasticAddr == "" && config.Resolver == nil {
		return nil, fmt.Errorf("Invalid elastic-addr and Resolver config")
	}

	c := &Curator{
		Config: *config,
		stopCh: make(chan bool, 1),
	}
	log.Debugf("Created curator, cfg: %+v", c.Config)
	return c, nil
}

// Start curator
func (c *Curator) Start() {
	c.Add(1)
	go c.scanIndices()
	c.Logger.Infof("Curator started with config: %v", c.Config)
}

// Stop curator
func (c *Curator) Stop() {
	c.stopCh <- true
	c.Wait()
	c.Logger.Infof("Curator stopped")
}

// Scan indices and delete older indices that are
// older than the configured retention period
func (c *Curator) scanIndices() {

	defer c.Done()

	for {

		select {
		// Check if scanIndices loop should be stopped
		case <-c.stopCh:
			c.Logger.Infof("scanIndices stopped")
			return

		// Scan indices and delete old indices
		case <-time.After(c.ScanInterval):

			// Initialize elastic client if required
			if c.esClient == nil {
				result, err := utils.ExecuteWithRetry(func() (interface{}, error) {
					return elastic.NewClient(c.Config.ElasticAddr, c.Config.Resolver, c.Config.Logger)
				}, retryDelay, maxRetries)

				if err != nil {
					c.Logger.Errorf("failed to create elastic client, err: %v", err)
					continue
				}
				c.esClient = result.(elastic.ESClient)
			}

			// Scan indices and delete older indices
			c.Logger.Debugf("Scanning indices ...")
			var resp map[string]elastic.SettingsResponse
			var err error
			if resp, err = c.esClient.GetIndexSettings(context.Background(), []string{c.IndexName}); err != nil {
				c.Logger.Errorf("Failed to get index settings, err: %v", err)
			}
			for index, settings := range resp {
				c.Logger.Debugf("Processing index: %s settings: %v", index, settings)
				if time.Since(settings.CreationDate) > c.RetentionPeriod {
					c.Logger.Infof("Deleting Index: %s, created-time: %v",
						index, settings.CreationDate)
					if err = c.esClient.DeleteIndex(context.Background(), index); err != nil {
						c.Logger.Errorf("Error deleting index: %s err: %v",
							index, err)
					}
				}
			}
		}
	}
}

// SetConfig updates the curator config
func (c *Curator) SetConfig(config *Config) error {
	c.Config = *config

	// TBD: check if we need stop/start service
	// for certain config changes

	return nil
}

// GetConfig return the current curator config
func (c *Curator) GetConfig() Config {
	return c.Config
}
