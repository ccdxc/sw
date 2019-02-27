// {C} Copyright 2018 Pensando Systems Inc. All rights reserved.

package mock

import (
	"github.com/pensando/sw/venice/utils/elastic/curator"
)

// MCurator is mock of elastic curator
type MCurator struct {
	curator.Curator
}

// NewMockCurator returns a new Curator service
func NewMockCurator() curator.Interface {
	return &MCurator{}
}

// Start curator
func (c *MCurator) Start() {
}

// Scan indices
func (c *MCurator) Scan(cfg *curator.Config) {
}

// Stop curator
func (c *MCurator) Stop() {

}
