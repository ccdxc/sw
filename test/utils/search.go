package utils

import (
	"context"
	"crypto/tls"
	"fmt"
	"net/http"
	"time"

	"github.com/pensando/sw/api/generated/audit"
	"github.com/pensando/sw/api/generated/events"
	"github.com/pensando/sw/api/generated/search"
	loginctx "github.com/pensando/sw/api/login/context"
	"github.com/pensando/sw/venice/utils/log"
	"github.com/pensando/sw/venice/utils/netutils"
)

// AuditEntry is a test struct to un-marshal event entry returned by Search REST API
type AuditEntry struct {
	Object audit.AuditEvent `json:"object"`
}

// AuditEntryList is list of search result entries
type AuditEntryList struct {
	//
	Entries []*AuditEntry `json:"entries,omitempty"`
}

// AuditTenantAggregation contains map of search result entries
// grouped by three levels: first by Tenant, second by Category
// and finally by Kind.
type AuditTenantAggregation struct {
	//
	Tenants map[string]*AuditCategoryAggregation `json:"tenants,omitempty"`
}

// AuditCategoryAggregation contains map of search result entries
// grouped by two levels: first by Category and then by Kind.
type AuditCategoryAggregation struct {
	//
	Categories map[string]*AuditKindAggregation `json:"categories,omitempty"`
}

// AuditKindAggregation contains map of search result
// entries grouped by Kind.
type AuditKindAggregation struct {
	//
	Kinds map[string]*AuditEntryList `json:"kinds,omitempty"`
}

// EventEntry is a test struct to un-marshal event entry returned by Search REST API
type EventEntry struct {
	Object events.Event `json:"object"`
}

// EventEntryList is list of search result entries
type EventEntryList struct {
	//
	Entries []*EventEntry `json:"entries,omitempty"`
}

// EventTenantAggregation contains map of search result entries
// grouped by three levels: first by Tenant, second by Category
// and finally by Kind.
type EventTenantAggregation struct {
	//
	Tenants map[string]*EventCategoryAggregation `json:"tenants,omitempty"`
}

// EventCategoryAggregation contains map of search result entries
// grouped by two levels: first by Category and then by Kind.
type EventCategoryAggregation struct {
	//
	Categories map[string]*EventKindAggregation `json:"categories,omitempty"`
}

// EventKindAggregation contains map of search result
// entries grouped by Kind.
type EventKindAggregation struct {
	//
	Kinds map[string]*EventEntryList `json:"kinds,omitempty"`
}

// SearchResponse contains meta information about search results
type SearchResponse struct {
	// TotalHits indicates total number of hits matched
	TotalHits int64 `json:"total-hits,omitempty"`
	// ActualHits indicates the actual hits returned in this response
	ActualHits int64 `json:"actual-hits,omitempty"`
	// TimeTakenMsecs is the time taken for search response in millisecs
	TimeTakenMsecs int64 `json:"time-taken-msecs,omitempty"`
	// Error status for failures
	Error *search.Error `json:"error,omitempty"`
	// PreviewEntries is a three level grouping of search summary (#hits),
	// grouped by tenant, category and kind in that order. This attribute
	// is populated and valid only in Preview request-mode
	PreviewEntries *search.TenantPreview `json:"preview-entries,omitempty"`
}

// AuditSearchResponse is a test struct to un-marshal audit event search response
type AuditSearchResponse struct {
	// SearchResponse contains meta information about search results
	SearchResponse
	// EventEntryList is list of all search results with no grouping.
	// This attribute is populated and valid only in Full request-mode
	Entries []*AuditEntry `json:"entries,omitempty"`
	// AggregatedEntries is a three level grouping of full search results,
	// Grouped by tenant, category and kind in that order. This attribute
	// is populated and valid only in Full request-mode
	AggregatedEntries *AuditTenantAggregation `json:"aggregated-entries,omitempty"`
}

// EventSearchResponse is a test struct to un-marshal event search response
type EventSearchResponse struct {
	// SearchResponse contains meta information about search results
	SearchResponse
	// EventEntryList is list of all search results with no grouping.
	// This attribute is populated and valid only in Full request-mode
	Entries []*EventEntry `json:"entries,omitempty"`
	// AggregatedEntries is a three level grouping of full search results,
	// Grouped by tenant, category and kind in that order. This attribute
	// is populated and valid only in Full request-mode
	AggregatedEntries *EventTenantAggregation `json:"aggregated-entries,omitempty"`
}

// Search sends a search query to API Gateway
func Search(ctx context.Context, apigw string, query *search.SearchRequest, resp interface{}) error {
	searchURL := fmt.Sprintf("https://%s/search/v1/query", apigw)
	restcl := netutils.NewHTTPClient()
	restcl.WithTLSConfig(&tls.Config{InsecureSkipVerify: true})
	restcl.DisableKeepAlives()
	defer restcl.CloseIdleConnections()

	// get authz header
	authzHeader, ok := loginctx.AuthzHeaderFromContext(ctx)
	if !ok {
		return fmt.Errorf("no authorization header in context")
	}
	restcl.SetHeader("Authorization", authzHeader)
	log.Infof("@@@ Search request: %+v\n", query)
	start := time.Now().UTC()
	_, err := restcl.Req("POST", searchURL, query, &resp)
	log.Infof("@@@ Search response time: %+v\n", time.Since(start))
	if err != nil {
		return err
	}
	log.Infof("@@@ Search response : %+v\n", resp)
	return nil
}

// GetAuditEvent returns audit event details given its UUID
func GetAuditEvent(ctx context.Context, apiGwAddr, eventID string, resp *audit.AuditEvent) error {
	auditURL := fmt.Sprintf("https://%s/audit/v1/events/%s", apiGwAddr, eventID)
	restcl := netutils.NewHTTPClient()
	restcl.WithTLSConfig(&tls.Config{InsecureSkipVerify: true})
	restcl.DisableKeepAlives()
	defer restcl.CloseIdleConnections()

	// get authz header
	authzHeader, ok := loginctx.AuthzHeaderFromContext(ctx)
	if !ok {
		return fmt.Errorf("no authorization header in context")
	}
	restcl.SetHeader("Authorization", authzHeader)
	status, err := restcl.Req("GET", auditURL, &audit.AuditEventRequest{}, resp)
	if status != http.StatusOK {
		return fmt.Errorf("GET request failed with http status code (%d) for event (%s)", status, auditURL)
	}
	return err
}
