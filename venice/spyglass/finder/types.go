package finder

import (
	"context"

	"github.com/pensando/sw/api"
	evtsapi "github.com/pensando/sw/api/generated/events"
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

	// spyglass serves as the backend for user facing events REST endpoints
	// so, it implements the event service functions as well
	// Event returns the event identified by given UUID
	GetEvent(ctx context.Context, t *evtsapi.GetEventRequest) (*evtsapi.Event, error)

	// Events return the list of events matching the given listOptions
	GetEvents(ctx context.Context, t *api.ListWatchOptions) (*evtsapi.EventList, error)
}
