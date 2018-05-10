// {C} Copyright 2018 Pensando Systems Inc. All rights reserved.

package finder

import (
	"context"
	"encoding/base64"
	"encoding/json"
	"fmt"
	"strings"
	"sync"
	"time"

	"github.com/pkg/errors"
	"google.golang.org/grpc"
	grpccode "google.golang.org/grpc/codes"
	es "gopkg.in/olivere/elastic.v5"

	"github.com/pensando/sw/api/fields"
	"github.com/pensando/sw/api/generated/search"
	"github.com/pensando/sw/api/labels"
	"github.com/pensando/sw/venice/globals"
	"github.com/pensando/sw/venice/utils"
	venutils "github.com/pensando/sw/venice/utils"
	"github.com/pensando/sw/venice/utils/elastic"
	"github.com/pensando/sw/venice/utils/log"
	"github.com/pensando/sw/venice/utils/resolver"
	"github.com/pensando/sw/venice/utils/rpckit"
)

const (
	elasticWaitIntvl  = time.Second
	maxElasticRetries = 200
	tenantAggKey      = "tenant_agg"
	kindAggKey        = "kind_agg"
	topHitsKey        = "top_agg"
)

// ErrInvalidParams is error returns when search parameters
// are either invalid or out of range.
var ErrInvalidParams = errors.New("Invalid search parameters")

// Finder is an implementation of the finder.Interface
type Finder struct {
	sync.WaitGroup
	ctx           context.Context
	logger        log.Logger
	elasticClient elastic.ESClient
	rsr           resolver.Interface
	finderAddr    string
	rpcServer     *rpckit.RPCServer
}

// NewFinder instantiates a new finder instance
func NewFinder(ctx context.Context, finderAddr string, rsr resolver.Interface, logger log.Logger) (Interface, error) {

	log.Debug("Creating Finder")

	fdr := Finder{
		ctx:        ctx,
		finderAddr: finderAddr,
		rsr:        rsr,
		logger:     logger,
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
		return elastic.NewClient("", fdr.rsr, fdr.logger.WithContext("submodule", "elastic"))
	}, elasticWaitIntvl, maxElasticRetries)
	if err != nil {
		fdr.logger.Errorf("Failed to create elastic client, err: %v", err)
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

// QueryBuilder builds Elastic Bool query based on search requirements
func (fdr *Finder) QueryBuilder(req *search.SearchRequest) (es.Query, error) {

	// Validate request
	if req == nil {
		return nil, errors.New("Nil search request")
	}

	// Check if QueryString is valid and construct QueryStringQuery
	if len(req.QueryString) != 0 {
		return es.NewQueryStringQuery(req.QueryString), nil
	}

	// Constuct Bool query based on search requirements
	if req.Query == nil {
		log.Error("Query in Body is nil")
		return nil, errors.New("Nil search request")
	}

	query := es.NewBoolQuery().QueryName("CompleteQuery")

	// Process Text Requirements
	textReq := es.NewBoolQuery().QueryName("TextQuery")
	for _, treq := range req.Query.Texts {

		tQuery := es.NewBoolQuery()
		for _, text := range treq.Text {
			if strings.Contains(text, " ") {
				// Phrase query
				tQuery = tQuery.Must(es.NewQueryStringQuery(fmt.Sprintf("\"%s\"", text)))
			} else {
				// Word query
				tQuery = tQuery.Must(es.NewQueryStringQuery(text))
			}
		}
		textReq = textReq.Should(tQuery).MinimumNumberShouldMatch(1)
	}
	if len(req.Query.Texts) > 0 {
		query = query.Must(textReq)
	}

	// Process categories requirement
	catReq := es.NewBoolQuery().QueryName("CategoryQuery")
	for _, cat := range req.Query.Categories {
		catReq = catReq.Should(es.NewTermQuery("meta.labels._category.keyword", cat)).MinimumNumberShouldMatch(1)
	}
	if len(req.Query.Categories) > 0 {
		query = query.Must(catReq)
	}

	// Process kinds requirement
	kindReq := es.NewBoolQuery().QueryName("KindQuery")
	for _, cat := range req.Query.Kinds {
		kindReq = kindReq.Should(es.NewTermQuery("kind", cat)).MinimumNumberShouldMatch(1)
	}
	if len(req.Query.Kinds) > 0 {
		query = query.Must(kindReq)
	}

	// Process field requirement if it is valid
	if req.Query.Fields != nil {
		for _, field := range req.Query.Fields.Requirements {
			switch field.Operator {
			case fields.Operator_name[int32(fields.Operator_equals)]:
				if len(field.Values) > 0 {
					query = query.Must(es.NewTermQuery(field.Key, field.Values[0]))
				}
			case fields.Operator_name[int32(fields.Operator_notEquals)]:
				if len(field.Values) > 0 {
					query = query.MustNot(es.NewTermQuery(field.Key, field.Values[0]))
				}
			case fields.Operator_name[int32(fields.Operator_in)]:
				values := make([]interface{}, len(field.Values))
				for i, v := range field.Values {
					values[i] = v
				}
				query = query.Must(es.NewTermsQuery(field.Key, values...))
			case fields.Operator_name[int32(fields.Operator_notIn)]:
				values := make([]interface{}, len(field.Values))
				for i, v := range field.Values {
					values[i] = v
				}
				query = query.MustNot(es.NewTermsQuery(field.Key, values...))
			case fields.Operator_name[int32(fields.Operator_gt)]:
				if len(field.Values) > 0 {
					query = query.Must(es.NewRangeQuery(field.Key).Gt(field.Values[0]))
				}
			case fields.Operator_name[int32(fields.Operator_gte)]:
				if len(field.Values) > 0 {
					query = query.Must(es.NewRangeQuery(field.Key).Gte(field.Values[0]))
				}
			case fields.Operator_name[int32(fields.Operator_lt)]:
				if len(field.Values) > 0 {
					query = query.Must(es.NewRangeQuery(field.Key).Lt(field.Values[0]))
				}
			case fields.Operator_name[int32(fields.Operator_lte)]:
				if len(field.Values) > 0 {
					query = query.Must(es.NewRangeQuery(field.Key).Lte(field.Values[0]))
				}
			}
		}
	}

	// Process label requirements if it is valid
	if req.Query.Labels != nil {
		for _, label := range req.Query.Labels.Requirements {
			lKey := fmt.Sprintf("%s.keyword", label.Key)
			switch label.Operator {
			case labels.Operator_name[int32(labels.Operator_equals)]:
				if len(label.Values) > 0 {
					query = query.Must(es.NewTermQuery(lKey, label.Values[0]))
				}
			case labels.Operator_name[int32(labels.Operator_notEquals)]:
				if len(label.Values) > 0 {
					query = query.MustNot(es.NewTermQuery(lKey, label.Values[0]))
				}
			case labels.Operator_name[int32(labels.Operator_in)]:
				values := make([]interface{}, len(label.Values))
				for i, v := range label.Values {
					values[i] = v
				}
				query = query.Must(es.NewTermsQuery(lKey, values...))
			case labels.Operator_name[int32(labels.Operator_notIn)]:
				values := make([]interface{}, len(label.Values))
				for i, v := range label.Values {
					values[i] = v
				}
				query = query.MustNot(es.NewTermsQuery(lKey, values...))
			}
		}
	}

	return query, nil
}

// Query is the handler for Search request
func (fdr *Finder) Query(ctx context.Context, in *search.SearchRequest) (*search.SearchResponse, error) {

	var sr search.SearchResponse

	fdr.logger.Debugf("Got search request: %+v", *in)

	// Validate search params
	if in.Validate("", true) == false {
		sr.Error = &search.Error{
			Type:   grpccode.InvalidArgument.String(),
			Reason: ErrInvalidParams.Error(),
		}
		return &sr, grpc.Errorf(grpccode.InvalidArgument, ErrInvalidParams.Error())
	}

	// ElasticSearch's Query and Aggregations JSON structure
	// While constructing start building inside out
	//
	// {
	//   "aggregations": {
	//     "tenant_agg": { <-- #1 Aggregate by Tenant
	//       "aggregations": {
	//         "category_agg": { <-- #2 Aggregate by Category
	//           "aggregations": {
	//             "kind_agg": { <-- #3 Aggregate by Kind
	//               "aggregations": {
	//                 "top_agg": { <-- #3 Aggregate top hits
	//                   "top_hits": {
	//                     "size": 10
	//                   }
	//                 }
	//               },
	//               "terms": {
	//                 "field": "kind"
	//               }
	//             }
	//           },
	//           "terms": {
	//             "field": "_type"
	//           }
	//         }
	//       },
	//       "terms": {
	//         "field": "meta.tenant"
	//       }
	//     }
	//   },
	//   "from": 0,
	//   "query": {
	//     "query_string": {
	//       "query": "us-west" <-- #0 Query string query parameter
	//     }
	//   },
	//   "size": 10
	// }

	// Build Elastic query
	query, err := fdr.QueryBuilder(in)

	// Top-hits Aggregation #4
	topAgg := es.NewTopHitsAggregation().From(int(in.From)).Size(int(in.MaxResults))

	// Add sort option if it is valid
	if len(in.SortBy) != 0 {
		topAgg = topAgg.Sort(in.SortBy, true)
	}

	// Kind-Aggregation #3
	aggKind := es.NewTermsAggregation().Field("kind")

	// Add Top-aggregations to Kind-Aggregation
	aggKind = aggKind.SubAggregation(elastic.TopHitsKey, topAgg)

	// Category-Aggregation #2
	aggCategory := es.NewTermsAggregation().Field("meta.labels._category.keyword")

	// Add Kind-Aggregation to Category-Aggregation
	aggCategory = aggCategory.SubAggregation(elastic.KindAggKey, aggKind)

	// Tenant-Aggregation #1
	aggTenant := es.NewTermsAggregation().Field("meta.tenant")

	// Add Category-Aggregation to Tenant-Aggregation
	aggTenant = aggTenant.SubAggregation(elastic.CategoryAggKey, aggCategory)

	// Output just the aggregations structure
	source, _ := aggTenant.Source()
	body, err := json.MarshalIndent(source, "", "  ")
	if err != nil {
		panic(err)
	}
	log.Debugf("Agg Nested source string: %s\n", string(body))

	// Execute Search with required index, query etc
	result, err := fdr.elasticClient.Search(ctx,
		fmt.Sprintf("%s.*", elastic.ExternalIndexPrefix),
		"", //  Skip the index/doc type for search
		query,
		aggTenant,
		in.From,
		in.MaxResults,
		in.SortBy)
	if err != nil {
		fdr.logger.Errorf("Search failed for query: %v, err:%v", query, err)
		var eType, eReason string
		if result != nil && result.Error != nil {
			eType = result.Error.Type
			eReason = result.Error.Reason
		} else {
			eType = err.Error()
		}
		sr.Error = &search.Error{
			Type:   eType,
			Reason: eReason,
		}
		return &sr, err
	}

	// Elastic Aggregations JSON structure
	//
	//   "aggregations": { <-- AggregationSingleBucket
	//     "tenant_agg": { <—- AggregationBucketKeyItems  (Tenant Aggregation)
	//       "doc_count_error_upper_bound": 0,
	//       "sum_other_doc_count": 0,
	//       "buckets": [  <—- AggregationBucketKeyItem
	//         {
	//           "key": "tesla",
	//           "doc_count": 3,
	//           "category_agg": { <—- AggregationBucketKeyItems (Category Aggregation)
	//             "doc_count_error_upper_bound": 0,
	//             "sum_other_doc_count": 0,
	//             "buckets": [ <—- AggregationBucketKeyItem
	//               {
	//                 "key": "Security",
	//                 "doc_count": 3,
	//                 "kind_agg": { <—- AggregationBucketKeyItems (Kind Aggregation)
	//                   "doc_count_error_upper_bound": 0,
	//                   "sum_other_doc_count": 0,
	//                   "buckets": [ <—- AggregationBucketKeyItem
	//                     {
	//                       "key": "SecurityGroup",
	//                       "doc_count": 3,
	//                       "top_agg": { <-AggregationTopHitsMetric (Tophits aggregation)
	//                         "hits": { <- SearchHits
	//                           "total": 3,
	//                           "max_score": 0.2876821,
	//                           "hits": [ <- SearchHit
	//                             {
	//                               "_index": "venice.external.default.configs.security",
	//                               "_type": "Security",
	//                               "_id": "c2856295-4c81-40e1-8061-4a6ea6ac0f37",
	//                               "_score": 0.2876821,
	//                               "_source": { <-- Venice Object
	//                                 "kind": "SecurityGroup",
	//                                 "api-version": "v1",
	//                                 "meta": {
	//                                   "name": "sg02",
	//                                   "tenant": "tesla",
	//                                   "namespace": "staging",
	//                                   "resource-version": "12",
	//                                   "uuid": "c2856295-4c81-40e1-8061-4a6ea6ac0f37",
	//                                   "labels": {
	//                                     "Application": "SAP-HANA",
	//                                     "Location": "us-west-zone2"
	//                                   },
	//                                   "creation-time": "2018-04-24T22:26:06.677102059Z",
	//                                   "mod-time": "2018-04-24T22:26:06.6771034Z",
	//                                   "self-link": "/venice/security-groups/security-groups/tesla/sg02"
	//                                 },
	//                                 "spec": {
	//                                   "workload-selector": {
	//                                     "requirements": [
	//                                       {
	//                                         "key": "app",
	//                                         "operator": "equals",
	//                                         "values": [
	//                                           "procurement"
	//                                         ]
	//                                       },
	//                                       {
	//                                         "key": "env",
	//                                         "operator": "equals",
	//                                         "values": [
	//                                           "production"
	//                                         ]
	//                                       }
	//                                     ]
	//                                   }
	//                                 },
	//                                 "status": {}
	//                               }
	//                             },
	//                             {
	//                               "_index": "venice.external.default.configs.security",
	//                               "_type": "Security",
	//                               "_id": "0b4f98dc-127b-43c1-94af-506d7c6aacf2",
	//                               "_score": 0.18232156,
	//                               "_source": {  <-- Venice Object
	//                                 "kind": "SecurityGroup",
	//                                 "api-version": "v1",
	//                                 "meta": {
	//                                   "name": "sg00",
	//                                   "tenant": "tesla",
	//                                   "namespace": "staging",
	//                                   "resource-version": "6",
	//                                   "uuid": "0b4f98dc-127b-43c1-94af-506d7c6aacf2",
	//                                   "labels": {
	//                                     "Application": "SAP-HANA",
	//                                     "Location": "us-west-zone2"
	//                                   },
	//                                   "creation-time": "2018-04-24T22:26:06.607595144Z",
	//                                   "mod-time": "2018-04-24T22:26:06.607596286Z",
	//                                   "self-link": "/venice/security-groups/security-groups/tesla/sg00"
	//                                 },
	//                                 "spec": {
	//                                   "workload-selector": {
	//                                     "requirements": [
	//                                       {
	//                                         "key": "app",
	//                                         "operator": "equals",
	//                                         "values": [
	//                                           "procurement"
	//                                         ]
	//                                       },
	//                                       {
	//                                         "key": "env",
	//                                         "operator": "equals",
	//                                         "values": [
	//                                           "production"
	//                                         ]
	//                                       }
	//                                     ]
	//                                   }
	//                                 },
	//                                 "status": {}
	//                               }
	//                             },
	//                             {
	//                               "_index": "venice.external.default.configs.security",
	//                               "_type": "Security",
	//                               "_id": "b658a5ff-8220-4fa4-a380-77f61e4c2f23",
	//                               "_score": 0.18232156,
	//                               "_source": {
	//                                 "kind": "SecurityGroup",
	//                                 "api-version": "v1",
	//                                 "meta": {
	//                                   "name": "sg04",
	//                                   "tenant": "tesla",
	//                                   "namespace": "staging",
	//                                   "resource-version": "18",
	//                                   "uuid": "b658a5ff-8220-4fa4-a380-77f61e4c2f23",
	//                                   "labels": {
	//                                     "Application": "SAP-HANA",
	//                                     "Location": "us-west-zone2"
	//                                   },
	//                                   "creation-time": "2018-04-24T22:26:06.747949636Z",
	//                                   "mod-time": "2018-04-24T22:26:06.747950841Z",
	//                                   "self-link": "/venice/security-groups/security-groups/tesla/sg04"
	//                                 },
	//                                 "spec": {
	//                                   "workload-selector": {
	//                                     "requirements": [
	//                                       {
	//                                         "key": "app",
	//                                         "operator": "equals",
	//                                         "values": [
	//                                           "procurement"
	//                                         ]
	//                                       },
	//                                       {
	//                                         "key": "env",
	//                                         "operator": "equals",
	//                                         "values": [
	//                                           "production"
	//                                         ]
	//                                       }
	//                                     ]
	//                                   }
	//                                 },
	//                                 "status": {}
	//                               }
	//                             }
	//                           ]
	//                         }
	//                       }
	//                     }
	//                   ]
	//                 }
	//               }
	//             ]
	//           }
	//         }
	//       ]
	//     }
	//   }

	// TODO: Remove all the debugs once the search feature is complete.
	fdr.logger.Debugf("Search query result: {%+v}", result)
	fdr.logger.Debugf("Search hits, len: %d {%+v}",
		len(result.Hits.Hits), result.Hits.Hits)
	fdr.logger.Debugf("Search aggregations, len: %d {%+v}",
		len(result.Aggregations), result.Aggregations)

	// Marshall the elasticDB response and populate the SearchResult
	var resp search.SearchResponse
	resp.TotalHits = result.Hits.TotalHits
	resp.ActualHits = int64(len(result.Hits.Hits))
	resp.TimeTakenMsecs = result.TookInMillis

	// Decode the Hits, if present
	resp.Entries = make([]*search.Entry, len(result.Hits.Hits))

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
			resp.Entries[i] = &e
		} else {
			resp.Entries[i] = nil
			fdr.logger.Errorf("Failed to marshal hits result-Source i:%d err:%v", i, err)
			// TBD: Stop here with error or continue with best effort ?
		}
	}

	// Deserialize Tenant aggregations
	if tenantAgg, found := result.Aggregations.Terms(elastic.TenantAggKey); found {

		log.Debugf("Found tenant_agg, : %d", len(result.Aggregations))
		resp.AggregatedEntries = &search.TenantAggregation{
			Tenants: make(map[string]*search.CategoryAggregation, len(result.Aggregations)),
		}
		log.Debugf("tenantAgg Buckets, : %d", len(tenantAgg.Buckets))

		for _, tenantBucket := range tenantAgg.Buckets {

			log.Debugf("tenant key : %s", tenantBucket.Key.(string))

			// Deserialize Category aggregations
			if categoryAgg, found := tenantBucket.Terms(elastic.CategoryAggKey); found {

				log.Debugf("categoryAgg Buckets, : %d", len(categoryAgg.Buckets))
				resp.AggregatedEntries.Tenants[tenantBucket.Key.(string)] = &search.CategoryAggregation{
					Categories: make(map[string]*search.KindAggregation, len(categoryAgg.Buckets)),
				}
				for _, categoryBucket := range categoryAgg.Buckets {

					log.Debugf("category key : %s", categoryBucket.Key.(string))

					// Deserialize Kind aggregations
					if kindAgg, found := categoryBucket.Terms(elastic.KindAggKey); found {

						log.Debugf("kindAgg Buckets, : %d", len(kindAgg.Buckets))
						resp.AggregatedEntries.Tenants[tenantBucket.Key.(string)].Categories[categoryBucket.Key.(string)] = &search.KindAggregation{
							Kinds: make(map[string]*search.EntryList, len(kindAgg.Buckets)),
						}
						for _, kindBucket := range kindAgg.Buckets {

							log.Debugf("kind key : %s", kindBucket.Key.(string))

							// Deserialize Top hits aggregations
							if topHits, ok := kindBucket.TopHits(string(elastic.TopHitsKey)); ok {

								hits := topHits.Hits.Hits
								log.Debugf("hits per kind : %d", len(hits))
								resp.AggregatedEntries.Tenants[tenantBucket.Key.(string)].Categories[categoryBucket.Key.(string)].Kinds[kindBucket.Key.(string)] = &search.EntryList{
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
										resp.AggregatedEntries.Tenants[tenantBucket.Key.(string)].Categories[categoryBucket.Key.(string)].Kinds[kindBucket.Key.(string)].Entries[i] = &e
									} else {
										resp.AggregatedEntries.Tenants[tenantBucket.Key.(string)].Categories[categoryBucket.Key.(string)].Kinds[kindBucket.Key.(string)].Entries[i] = nil
										log.Errorf("Failed to marshal Agg hits result-Source i:%d err:%v", i, err)
										// TBD: Stop here with error or continue with best effort ?
									}
								}
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
