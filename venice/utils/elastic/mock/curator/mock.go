// {C} Copyright 2018 Pensando Systems Inc. All rights reserved.

package mock

import (
	"github.com/pensando/sw/venice/utils/elastic/curator"
)

// MCurator is mock of elastic curator
type MCurator struct {

	// Config is the configuration for a Curator instance
	curator.Config
}

// NewMockCurator returns a new Curator service
func NewMockCurator(config *curator.Config) curator.Interface {

	return &MCurator{
		Config: *config,
	}
}

// Start curator
func (c *MCurator) Start() {
}

// Stop curator
func (c *MCurator) Stop() {

}

// SetConfig updates the curator config
func (c *MCurator) SetConfig(config *curator.Config) error {
	c.Config = *config
	return nil
}

// GetConfig return the current curator config
func (c *MCurator) GetConfig() curator.Config {
	return c.Config
}
