// {C} Copyright 2017 Pensando Systems Inc. All rights reserved.

package mock

import (
	"encoding/json"
	"io/ioutil"
	"net/http"
	"strings"

	"github.com/gorilla/mux"
	es "gopkg.in/olivere/elastic.v5"

	tu "github.com/pensando/sw/venice/utils/testutils"
)

// ElasticServer represents the mock elastic server
type ElasticServer struct {
	ms  *tu.MockServer
	URL string

	//map of indexes available with the docs indexed
	indexes map[string]map[string][]byte
}

// MQuery (mock query) mimics the elastic's `RawStringQuery`
// this is used to query all the docs containing given string
type MQuery struct {
	Query struct {
		// match all the documents containing this string
		MatchAll string `json:"match_all"`
	}
}

// NewElasticServer returns the instance of elastic mock server
func NewElasticServer() *ElasticServer {
	e := &ElasticServer{
		ms:      tu.NewMockServer(),
		indexes: make(map[string]map[string][]byte),
	}

	// all the required handlers for elastic mock server
	e.addHandlers()

	return e
}

// SetDefaultStatusCode sets the default HTTP status code on the mock server.
// this can be used to simulate `StatusInternalServerError`.
// If this value is set, the status is returned without invoking the registered handler.
func (e *ElasticServer) SetDefaultStatusCode(statusCode int) {
	e.ms.SetDefaultStatusCode(statusCode)
}

// ClearDefaultStatusCode clears the default HTTP status code.
// All the subsequent calls will invoke the registered handler.
func (e *ElasticServer) ClearDefaultStatusCode() {
	e.ms.ClearDefaultStatusCode()
}

// Start starts the elastic mock server
func (e *ElasticServer) Start() {
	go e.ms.Start()

	// update elastic server address
	e.URL = e.ms.URL()
}

// GetElasticURL returns the elastic mock server URL
func (e *ElasticServer) GetElasticURL() string {
	return e.URL
}

// Stop stops the elastic mock server
func (e *ElasticServer) Stop() {
	e.ms.Stop()
}

// addHandlers adds all the dummy handlers for elastic functions we use
func (e *ElasticServer) addHandlers() {
	e.ms.AddHandler("/", "HEAD", func(w http.ResponseWriter, r *http.Request) {})

	// for ping and version calls
	e.ms.AddHandler("/", "GET", func(w http.ResponseWriter, r *http.Request) {
		resp := `{
			"name": "mock-elastic-server",
			"cluster_name": "mock-elastic-cluster",
			"cluster_uuid": "WyU4yfYqROunDer1BAG_Eg",
			"version": {
				"number": "5.4.1",
				"lucene_version": "6.5.1"
			},
			"tagline": "You Know, for Search"
		}`

		w.Write([]byte(resp))
	})

	// create index
	e.ms.AddHandler("/{index_name}", "PUT", func(w http.ResponseWriter, r *http.Request) {
		vars := mux.Vars(r)
		indexName := vars["index_name"]
		e.indexes[indexName] = make(map[string][]byte)

		resp := `{
			"acknowledged": true,
			"shards_acknowledged": true
		}`

		w.Write([]byte(resp))
	})

	// check if index exists
	e.ms.AddHandler("/{index_name}", "HEAD", func(w http.ResponseWriter, r *http.Request) {
		vars := mux.Vars(r)
		indexName := vars["index_name"]

		if _, ok := e.indexes[indexName]; ok {
			w.WriteHeader(http.StatusOK)
		} else {
			w.WriteHeader(http.StatusNotFound)
		}
	})

	// index operation - this dummy handler captures the indexed document as a []byte
	// which will be used to serve GET and SEARCH requests
	e.ms.AddHandler("/{index_name}/{index_type}/{id}", "PUT", func(w http.ResponseWriter, r *http.Request) {
		vars := mux.Vars(r)
		indexName := vars["index_name"]
		docID := vars["id"]

		body, err := ioutil.ReadAll(r.Body)
		if err != nil {
			w.WriteHeader(http.StatusInternalServerError)
			w.Write([]byte("failed to read body from index request"))
			return
		}

		e.indexes[indexName][docID] = body

		w.Write([]byte("{}"))
	})

	// bulk operation - it is too much of work to look at each request in the bulk operation.
	// so, it is left out for now
	e.ms.AddHandler("/_bulk", "POST", func(w http.ResponseWriter, r *http.Request) {
		w.Write([]byte("{}"))
	})

	// search operation - this handler returns all the documents matching the query.
	// the query here is restricted to `RawQueryString` (all docs containing the given string).
	e.ms.AddHandler("/{index_name}/{index_type}/_search", "POST", func(w http.ResponseWriter, r *http.Request) {
		vars := mux.Vars(r)
		indexName := vars["index_name"]

		body, err := ioutil.ReadAll(r.Body)
		if err != nil {
			w.WriteHeader(http.StatusInternalServerError)
			w.Write([]byte("failed to read body from search request"))
			return
		}

		q := &MQuery{}
		if err := json.Unmarshal(body, q); err != nil {
			w.WriteHeader(http.StatusInternalServerError)
			w.Write([]byte("failed to read query from search request"))
			return
		}

		queryString := q.Query.MatchAll
		resp := &es.SearchHits{}
		// search for all the docs containing the given string
		for _, doc := range e.indexes[indexName] {
			if strings.Contains(string(doc), queryString) {
				resp.TotalHits++
				resp.Hits = append(resp.Hits, &es.SearchHit{Source: (*json.RawMessage)(&doc)})
			}
		}

		respData, _ := json.Marshal(es.SearchResult{Hits: resp})
		w.Write(respData)
	})
}
