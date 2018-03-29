// {C} Copyright 2018 Pensando Systems Inc. All rights reserved.

package finder

import (
	"context"
	"encoding/base64"
	"encoding/json"
	"strings"
	"sync"
	"time"

	"github.com/pkg/errors"
	es "gopkg.in/olivere/elastic.v5"

	"github.com/pensando/sw/api/generated/search"
	"github.com/pensando/sw/venice/globals"
	"github.com/pensando/sw/venice/spyglass/utils"
	venutils "github.com/pensando/sw/venice/utils"
	"github.com/pensando/sw/venice/utils/elastic"
	"github.com/pensando/sw/venice/utils/log"
	"github.com/pensando/sw/venice/utils/rpckit"
)

const (
	elasticWaitIntvl  = time.Second
	maxElasticRetries = 200
	tenantAggKey      = "tenant_agg"
	kindAggKey        = "kind_agg"
	topHitsKey        = "top_agg"
)

// Finder is an implementation of the finder.Interface
type Finder struct {
	sync.WaitGroup
	ctx           context.Context
	logger        log.Logger
	elasticAddr   string
	elasticClient elastic.ESClient
	finderAddr    string
	rpcServer     *rpckit.RPCServer
}

// NewFinder instantiates a new finder instance
func NewFinder(ctx context.Context, elasticAddr, finderAddr string, logger log.Logger) (Interface, error) {

	log.Debugf("Creating Finder, elastic-addr: %s", elasticAddr)
	// Validate elasticAddr
	if len(strings.TrimSpace(elasticAddr)) == 0 {
		return nil, errors.New("Empty elastic address")
	}

	fdr := Finder{
		ctx:         ctx,
		elasticAddr: elasticAddr,
		finderAddr:  finderAddr,
		logger:      logger,
	}

	return &fdr, nil
}

// Start finder service
func (fdr *Finder) Start() error {

	// Start the rpc-server
	fdr.logger.Infof("Starting finder rpc server at: %s", fdr.finderAddr)
	err := fdr.startRPCServer(globals.Spyglass, fdr.finderAddr)
	if err != nil {
		fdr.logger.Errorf("Failed to start rpc-server, err: %s", err)
		return err
	}

	// Initialize elastic client
	result, err := utils.ExecuteWithRetry(func() (interface{}, error) {
		return elastic.NewClient(fdr.elasticAddr, fdr.logger.WithContext("submodule", "elastic"))
	}, elasticWaitIntvl, maxElasticRetries)
	if err != nil {
		fdr.logger.Errorf("Failed to create elastic client, addr: %s err: %v",
			fdr.elasticAddr, err)
		return err
	}
	fdr.logger.Debugf("Created Elastic client")
	fdr.elasticClient = result.(elastic.ESClient)

	return nil
}

// Stop finder service
func (fdr *Finder) Stop() {
	fdr.logger.Debug("Stopping finder")
	fdr.stopRPCServer()
	fdr.elasticClient.Close()
	fdr.logger.Info("Stopped finder")
}

// Query is the handler for Search request
func (fdr *Finder) Query(ctx context.Context, in *search.SearchRequest) (*search.SearchResponse, error) {

	var sr search.SearchResponse

	fdr.logger.Debugf("Got search request: %+v query-string: %s", in, in.QueryString)

	// ElasticSearch's Query and Aggregations JSON structure
	// While constructing start building inside out
	//	{
	//		"size": 50,
	//
	//		"query": {  <-- #0 Query parameter
	//		  "query_string": {
	//			"query": "us-west"
	//		  }
	//		},
	//
	//		"aggs": {
	//			"tenant_agg": { <-- #1 Outermost aggregation
	//				"terms": {
	//					"field": "meta.Tenant"
	//				},
	//				"aggs": {
	//					"kind_agg": { <-- #2 Aggregate by Kind
	//						"terms": {
	//							"field": "Kind"
	//						},
	//						"aggs": {
	//							"top": { <-- #3 Return top hits
	//								"top_hits": {
	//									"size": 10
	//								}
	//							}
	//						}
	//					}
	//				}
	//			}
	//		}
	//	}

	// Use NewQueryStringQuery to start with #0
	// TODO: Use Bool Query with aggregrations for grouping
	//       of results once QueryParser is ready
	query := es.NewQueryStringQuery(in.QueryString)

	// Top-hits Aggregation #3
	topAgg := es.NewTopHitsAggregation().Size(int(in.MaxResults))

	// Kind-Aggregation #2
	aggKind := es.NewTermsAggregation().Field("kind")

	// Add Top-aggregations to Kind-Aggregation
	aggKind = aggKind.SubAggregation("top_agg", topAgg)

	// Tenant-Aggregation #1
	aggTenant := es.NewTermsAggregation().Field("meta.tenant")

	// Add Kind-Aggregation to Tenant-Aggregation
	aggTenant = aggTenant.SubAggregation("kind_agg", aggKind)

	// Output just the aggregations structure
	source, _ := aggTenant.Source()
	body, err := json.MarshalIndent(source, "", "  ")
	if err != nil {
		panic(err)
	}
	log.Debugf("Agg Nested source string: %s\n", string(body))

	// Execute Search with required index, query etc
	result, err := fdr.elasticClient.Search(ctx,
		"venice.external.*",
		elastic.GetDocType(globals.Configs),
		query,
		aggTenant,
		in.From,
		in.MaxResults)
	if err != nil {
		fdr.logger.Errorf("Search failed for query: %v, err:%v", query, err)
		var eType, eReason string
		if result != nil && result.Error != nil {
			eType = result.Error.Type
			eReason = result.Error.Reason
		} else {
			eType = err.Error()
		}
		sr.Result = &search.SearchResult{
			Error: &search.Error{
				Type:   eType,
				Reason: eReason,
			},
		}
		return &sr, err
	}

	// Elastic Aggregations JSON structure
	//
	//	{ ## <- AggregationSingleBucket
	//	"doc_count": 3,
	//	"nested_agg": { <— AggregationBucketKeyItems
	//		"doc_count_error_upper_bound": 0,
	//		"sum_other_doc_count": 0,
	//		"buckets": [ ## <— AggregationBucketKeyItem
	//		{
	//			"key": "default",
	//			"doc_count": 3,
	//			"tenant_agg": { ## <— AggregationSingleBucket
	//			"doc_count": 3,
	//			"kind_agg": { ## <— AggregationBucketKeyItems
	//				"doc_count_error_upper_bound": 0,
	//				"sum_other_doc_count": 0,
	//				"buckets": [ <— AggregationBucketKeyItem
	//		        {
	//				  {
	//					"key": "Tenant",
	//					"doc_count": 3,
	//					"top_agg": { ## <-AggregationTopHitsMetric
	//					"hits": { ## <- SearchHits
	//						"total": 3,
	//						"max_score": 1.691676,
	//						"hits": [ ## <- SearchHit
	//						{
	//							"_index": "venice.external.default.configs.2018-02-26",
	//							"_type": "configs",
	//							"_id": "755cbf37-af51-49b5-a87f-48399188f876",
	//							"_score": 1.691676,
	//							"_source": { ## <-- Venice Object
	//							"Kind": "Tenant",
	//							"APIVersion": "v1",
	//							"meta": {
	//								"Name": "audi",
	//								"Tenant": "default",
	//								"Namespace": "default",
	//								"ResourceVersion": "4",
	//								"UUID": "755cbf37-af51-49b5-a87f-48399188f876",
	//								"Labels": {
	//								"Location": "us-west-zone1"
	//								},
	//								"CreationTime": "2018-02-26T20:04:00.941971366Z",
	//								"ModTime": "2018-02-26T20:04:00.941971968Z",
	//								"SelfLink": "/venice/tenants/tenants/audi"
	//							},
	//							"spec": {},
	//							"status": {}
	//							}
	//						},

	// TODO: Remove all the debugs once the search feature is complete.
	fdr.logger.Debugf("Search query result: {%+v}", result)
	fdr.logger.Debugf("Search hits, len: %d {%+v}",
		len(result.Hits.Hits), result.Hits.Hits)
	fdr.logger.Debugf("Search aggregations, len: %d {%+v}",
		len(result.Aggregations), result.Aggregations)

	// Marshall the elasticDB response and populate the SearchResult
	var resp search.SearchResponse
	resp.Result = &search.SearchResult{
		TotalHits:      result.Hits.TotalHits,
		ActualHits:     int64(len(result.Hits.Hits)),
		TimeTakenMsecs: result.TookInMillis,
	}

	// Decode the Hits, if present
	resp.Result.Entries = make([]*search.Entry, len(result.Hits.Hits))

	for i, entry := range result.Hits.Hits {
		jsondata, err := entry.Source.MarshalJSON()
		if err == nil {

			databytes := []byte(jsondata)
			fdr.logger.Debugf("Search hits result - raw string: %d {%s}", i, string(databytes))

			str, _ := base64.StdEncoding.DecodeString(string(databytes))
			fdr.logger.Debugf("Search hits result - base64 string: %d {%s}", i, string(str))

			e := search.Entry{}
			err = json.Unmarshal([]byte(databytes), &e)
			if err != nil {
				fdr.logger.Errorf("Error unmarshalling json data to search.entry : %+v", err)
			}

			fdr.logger.Debugf("Search hits result - entry proto: %d {%+v}", i, e)
			resp.Result.Entries[i] = &e
		} else {
			resp.Result.Entries[i] = nil
			fdr.logger.Errorf("Failed to marshal hits result-Source i:%d err:%v", i, err)
			// TBD: Stop here with error or continue with best effort ?
		}
	}

	// Deserialize aggregations
	if tenantAgg, found := result.Aggregations.Terms(elastic.TenantAggKey); found {

		log.Debugf("Found tenant_agg, : %d", len(result.Aggregations))
		resp.Result.AggregatedEntries = &search.NestedAggregation{
			Entries: make(map[string]*search.Aggregation, len(result.Aggregations)),
		}
		log.Debugf("tenantAgg Buckets, : %d", len(tenantAgg.Buckets))

		for _, tenantBucket := range tenantAgg.Buckets {

			log.Debugf("tenant key : %s", tenantBucket.Key.(string))

			if kindAgg, found := tenantBucket.Terms(elastic.KindAggKey); found {

				log.Debugf("kindAgg Buckets, : %d", len(kindAgg.Buckets))
				resp.Result.AggregatedEntries.Entries[tenantBucket.Key.(string)] = &search.Aggregation{
					Entries: make(map[string]*search.EntryList, len(tenantAgg.Buckets)),
				}
				for _, kindBucket := range kindAgg.Buckets {

					log.Debugf("kind key : %s", kindBucket.Key.(string))
					if topHits, ok := kindBucket.TopHits(string(elastic.TopHitsKey)); ok {

						hits := topHits.Hits.Hits
						log.Debugf("hits per kind : %d", len(hits))
						resp.Result.AggregatedEntries.Entries[tenantBucket.Key.(string)].Entries[kindBucket.Key.(string)] = &search.EntryList{
							Entries: make([]*search.Entry, len(hits)),
						}

						for i, entry := range hits {

							jsondata, err := entry.Source.MarshalJSON()
							if err == nil {

								databytes := []byte(jsondata)
								fdr.logger.Debugf("Agg Search hits result - raw string: %d {%s}", i, string(databytes))

								str, _ := base64.StdEncoding.DecodeString(string(databytes))
								fdr.logger.Debugf("Agg Search hits result - base64 string: %d {%s}", i, string(str))

								e := search.Entry{}
								err = json.Unmarshal([]byte(databytes), &e)
								if err != nil {
									fdr.logger.Errorf("Error unmarshalling Agg json data to search.entry : %+v", err)
								}

								log.Debugf("Entry: %d %s", i, e.GetName())
								log.Debugf("Agg Search hits result - entry proto: %d {%+v}", i, e)
								resp.Result.AggregatedEntries.Entries[tenantBucket.Key.(string)].Entries[kindBucket.Key.(string)].Entries[i] = &e
							} else {
								resp.Result.AggregatedEntries.Entries[tenantBucket.Key.(string)].Entries[kindBucket.Key.(string)].Entries[i] = nil
								log.Errorf("Failed to marshal Agg hits result-Source i:%d err:%v", i, err)
								// TBD: Stop here with error or continue with best effort ?
							}
						}
					}
				}
			}
		}
	}

	fdr.logger.Debugf("Final Search result: {%+v}", resp)
	return &resp, nil
}

// Start the RPC-server for the Query backend handling
func (fdr *Finder) startRPCServer(serverName, listenURL string) error {

	// Valids args
	if venutils.IsEmpty(serverName) || venutils.IsEmpty(listenURL) {
		return errors.New("all parameters are required")
	}

	// Create an RPC server
	rpcServer, err := rpckit.NewRPCServer(serverName, listenURL)
	if err != nil {
		return errors.Wrap(err, "Error creating rpc server")
	}

	// Register the RPC handlers and start the rpc server
	search.RegisterSearchV1Server(rpcServer.GrpcServer, fdr)
	rpcServer.Start()
	fdr.rpcServer = rpcServer
	log.Info("Started finder rpcserver at: %s", fdr.rpcServer.GetListenURL())
	return nil
}

// Stop the RPC-server
func (fdr *Finder) stopRPCServer() error {

	// stop the rpc server
	return fdr.rpcServer.Stop()
}

// GetListenURL returns the listen URL for the server
func (fdr *Finder) GetListenURL() string {
	return fdr.rpcServer.GetListenURL()
}
