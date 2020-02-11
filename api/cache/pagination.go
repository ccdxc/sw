package cache

import (
	"context"
	"fmt"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/venice/utils/runtime"
)

// MaxListResultsPerRequest is the maximum number of List results that the API server can send in a single pagianted repsonse
var MaxListResultsPerRequest int32 = 1000 // TODO: Set the right number

// processPaginationOptions - takes a list of all the entries and applies the pagination logic to this list to return
// the start index and end index of the requested page. If the endIndex is > len(items), endIndex is returned as 0
func processPaginationOptions(ctx context.Context, items []runtime.Object, opts api.ListWatchOptions) (uint32, uint32, error) {

	fromIndex := opts.GetFrom()
	resultsPerPage := opts.GetMaxResults()

	if resultsPerPage > MaxListResultsPerRequest {
		return 0, 0, fmt.Errorf("NumResults [%d] exceeds supported maximum results per page of [%d]", resultsPerPage, MaxListResultsPerRequest)
	}

	// Number of items in the unfiltered list
	numItems := len(items)

	startIndex := int(fromIndex) - 1 // From is 1 based, not 0 based
	endIndex := startIndex + int(resultsPerPage)

	if startIndex > numItems {
		// Invalid index, trying to fetch a index number more than the number of results we have
		return 0, 0, fmt.Errorf("From index [%d] is greater that total number of entries [%d]", fromIndex, numItems)
	}

	if endIndex > numItems {
		// Last page case, where the num items in this page is less than resultsPerPage, make endIndex the last item's index
		endIndex = startIndex + (numItems - startIndex)
	}

	return uint32(startIndex), uint32(endIndex), nil

}
