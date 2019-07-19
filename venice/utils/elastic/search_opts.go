package elastic

import (
	es "github.com/olivere/elastic"
)

// SearchOption fills the optional params for search request
type SearchOption func(*searchOptions)

// SearchOptions contains all the optional search parameters
type searchOptions struct {
	sourceContext *es.FetchSourceContext
}

// IncludeFields constructs a fetch context that will help fetch certain fields
// instead of fetching entire `_source` based on the given strings
func IncludeFields(fields []string) SearchOption {
	return func(s *searchOptions) {
		if s.sourceContext != nil {
			s.sourceContext.Include(fields...)
			return
		}
		s.sourceContext = es.NewFetchSourceContext(true).Include(fields...)
	}
}

// ExcludeFields constructs a fetch context that will help exclude certain fields
// instead of fetching entire `_source` based on the given strings
func ExcludeFields(fields []string) SearchOption {
	return func(s *searchOptions) {
		if s.sourceContext != nil {
			s.sourceContext.Exclude(fields...)
			return
		}
		s.sourceContext = es.NewFetchSourceContext(true).Exclude(fields...)
	}
}
