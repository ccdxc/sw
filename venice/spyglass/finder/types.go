package finder

import (
	"context"

	"github.com/pensando/sw/api/generated/search"
)

// Interface is an interface for Finder functionality
type Interface interface {

	// Start service
	Start() error

	// Stop service
	Stop()

	// GetListenURL return the finder's listener-URL
	GetListenURL() string

	// Query performs the given query on `index`
	Query(ctx context.Context, in *search.SearchRequest) (*search.SearchResponse, error)
}
