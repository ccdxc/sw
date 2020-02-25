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
	// maxRetries maximum number of retries for fetching elasticsearch URLs and creating client.
	maxRetries = 60

	// delay between retries
	retryDelay = 2 * time.Second
)

//
// TODO: Since we have moved this functionality to elastic-curator cron job. We don't need this code anymore.
// Once the new feature stabilizes, we can remove this.
//

// Curator represents elastic index manager service for
//  - handling index retention and expiration (for now)
//
// Please see testCurator() in test/integ/venice_integ/elastic/client_test.go
// for sample usage of this service.
type Curator struct {
	sync.Mutex
	esClient       elastic.ESClient   // elastic search client
	resolverClient resolver.Interface // resolver client
	indexConfigs   map[string]*Config // list of indices to be scanned for deletion
	running        bool               // indicates whether the curator is running or not
	wg             sync.WaitGroup     // wait group for scanner go routines
	ctx            context.Context    // to stop the scanners
	cancelFunc     context.CancelFunc // to stop the scanners
	logger         log.Logger         // logger
}

// Config is the config of a particular index; which will be scanned by the curator
// and the indices will be deleted.
type Config struct {
	IndexName       string        // index prefix or pattern e.g. venice.internal.default.systemlogs.* or venice*events*
	RetentionPeriod time.Duration // retention period for the above index; indices beyond this period will be deleted
	ScanInterval    time.Duration // scan interval or frequency
}

// NewCurator returns a new Curator service
func NewCurator(esClient elastic.ESClient, resolverClient resolver.Interface, logger log.Logger) (Interface, error) {
	if resolverClient == nil && esClient == nil {
		return nil, fmt.Errorf("resolver or elastic client is required")
	}

	c := &Curator{
		esClient:       esClient,
		resolverClient: resolverClient,
		indexConfigs:   make(map[string]*Config),
		logger:         logger,
	}

	return c, nil
}

// Start starts the curator service
func (c *Curator) Start() {
	c.Lock()
	defer c.Unlock()

	if !c.running {
		c.ctx, c.cancelFunc = context.WithCancel(context.Background())
		c.running = true
		c.logger.Infof("curator service started")
	}
}

// Stop curator
func (c *Curator) Stop() {
	c.Lock()
	if c.running {
		c.running = false
		c.Unlock()
		c.cancelFunc()
		c.wg.Wait()

		c.Lock()
		c.indexConfigs = make(map[string]*Config)
		c.Unlock()
		c.logger.Infof("curator service stopped")
	} else {
		c.Unlock()
	}
}

// Scan indices and delete older indices that are
// older than the configured retention period
func (c *Curator) Scan(cfg *Config) {
	c.Lock()
	defer c.Unlock()

	if !c.running { // do not accept scan requests when the curator is stopped
		return
	}

	c.logger.Infof("received a scan request for index {%s}: %v", cfg.IndexName, cfg.RetentionPeriod)

	if _, ok := c.indexConfigs[cfg.IndexName]; ok {
		c.logger.Infof("overwriting configs for index {%s}", cfg.IndexName)
	}
	c.indexConfigs[cfg.IndexName] = cfg

	c.wg.Add(1)
	go func() {
		defer c.wg.Done()

		c.logger.Infof("{%s} scanning", cfg.IndexName)
		for {
			select {
			case <-c.ctx.Done():
				c.logger.Infof("{%s} scanning stopped", cfg.IndexName)
				return

			case <-time.After(cfg.ScanInterval): // scan and delete older indices
				if c.ctx.Err() != nil {
					c.logger.Infof("{%s} scanning stopped", cfg.IndexName)
					return
				}
				c.Lock()               // to ensure there is one go routine trying to reset the client
				if c.esClient == nil { // initialize elastic client if required
					result, err := utils.ExecuteWithRetry(func(ctx context.Context) (interface{}, error) {
						return elastic.NewAuthenticatedClient("", c.resolverClient, c.logger)
					}, retryDelay, maxRetries)

					if err != nil {
						c.logger.Errorf("failed to create elastic client, err: %v", err)
						c.Unlock()
						continue
					}
					c.esClient = result.(elastic.ESClient)
				}
				c.Unlock()

				var resp map[string]elastic.SettingsResponse
				var err error
				if resp, err = c.esClient.GetIndexSettings(c.ctx, []string{cfg.IndexName}); err != nil {
					c.logger.Errorf("{%s} failed to get index settings, err: %v", cfg.IndexName, err)
				}
				for index, settings := range resp {
					c.logger.Debugf("{%s} processing index settings: %v", index, settings)
					if time.Since(settings.CreationDate) > cfg.RetentionPeriod {
						c.logger.Infof("{%s} deleting index created at: %v", index, settings.CreationDate)
						if err = c.esClient.DeleteIndex(c.ctx, index); err != nil {
							c.logger.Errorf("{%s} error deleting index, err: %v", index, err)
						}
					}
				}
			}
		}
	}()
}
