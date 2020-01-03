package elastic

import (
	"bytes"
	"context"
	"encoding/json"
	"io"
	"reflect"
	"strings"
	"time"

	es "github.com/olivere/elastic"
)

const (
	defaultScrollMaxResults = 8000
)

type scroller struct {
	client  *Client
	request *es.ScrollService
	ctx     context.Context
	index   string
	iType   string
	query   es.Query
	size    int32
	buf     bytes.Buffer
}

func (s *scroller) Read(p []byte) (int, error) {
	n, err := s.buf.Read(p)
	if err == io.EOF {
		// scroll next data
		if err := s.next(); err != nil {
			// there are no more search results to scroll in elastic
			return 0, err
		}
		return s.buf.Read(p)
	}
	return n, err
}

// next scrolls to the next search data. Returns io.EOF if there are no more search results
func (s *scroller) next() error {
	result, err := s.search()
	if err != nil {
		return err
	}
	var buf bytes.Buffer
	for _, hit := range result.Hits.Hits {
		jsonData, err := hit.Source.MarshalJSON()
		if err != nil {
			// log and continue
			continue
		}
		buf.Write(jsonData)
		buf.WriteString("\n")
	}
	s.buf = buf
	return nil
}

func (s *scroller) search() (*es.SearchResult, error) {
	retryCount := 0
	retryInterval := initialRetryInterval

	var rResp interface{}
	var rErr error
	var retry, endOfFile bool

	for {
		retry, rResp, rErr = s.client.Perform(func() (interface{}, error) {
			ctxWithDeadline, cancel := context.WithDeadline(s.ctx, time.Now().Add(contextDeadline))
			defer cancel()
			// Execute the search request with desired size
			searchResult, err := s.request.Do(ctxWithDeadline)
			if err == io.EOF {
				// this is thrown when there are no search results, we don't need to retry if that is the case
				endOfFile = true
				err = nil
			}
			return searchResult, err
		}, retryCount, rResp, rErr)

		if retry {
			if 2*retryInterval > maxRetryInterval {
				retryInterval = maxRetryInterval
			} else {
				retryInterval = retryInterval * 2
			}

			time.Sleep(retryInterval)
			retryCount++
			continue
		}
		if endOfFile {
			rErr = io.EOF
		}
		if rErr != nil {
			return nil, rErr
		}

		searchResult := rResp.(*es.SearchResult)
		return searchResult, rErr
	}
}

// NewScroller returns an instance of Scroller. It fetches 8000 max records at a time. This Scroller implementation is not thread safe.
func NewScroller(ctx context.Context, client *Client, index, iType string, query es.Query, size int32) (Scroller, error) {
	// validate index
	if len(strings.TrimSpace(index)) == 0 {
		return nil, NewError(ErrInvalidIndex, "")
	}

	// validate query
	if query != nil && reflect.ValueOf(query).IsValid() {
		// make sure the query is in correct format; Source() returns the json of the query
		if src, err := query.Source(); err != nil {
			return nil, NewError(ErrInvalidSearchQuery, err.Error())
		} else if _, err := json.Marshal(src); err != nil {
			return nil, NewError(ErrInvalidSearchQuery, err.Error())
		}
	}

	// Construct the search request on a given index
	request := client.esClient.Scroll().Index(index)

	// Add doc type if valid
	if len(iType) != 0 {
		request = request.Type(iType)
	}

	// Add query if valid
	if query != nil {
		request = request.Query(query)
	}

	// Set size or maxResults desired
	if size > 0 && size <= defaultScrollMaxResults {
		request = request.Size(int(size))
	} else {
		request = request.Size(int(defaultScrollMaxResults))
	}

	reader := &scroller{
		client:  client,
		request: request,
		ctx:     ctx,
		index:   index,
		iType:   iType,
		query:   query,
		size:    size,
	}
	return reader, nil
}
