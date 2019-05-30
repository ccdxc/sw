package service

import (
	"context"
	"encoding/json"
	"errors"
	"fmt"
	"strconv"
	"strings"
	"sync"
	"time"

	"github.com/gogo/protobuf/types"
	es "github.com/olivere/elastic"

	"github.com/pensando/sw/api"
	diagapi "github.com/pensando/sw/api/generated/diagnostics"
	"github.com/pensando/sw/venice/utils"
	"github.com/pensando/sw/venice/utils/diagnostics"
	"github.com/pensando/sw/venice/utils/diagnostics/protos"
	"github.com/pensando/sw/venice/utils/elastic"
	"github.com/pensando/sw/venice/utils/log"
	"github.com/pensando/sw/venice/utils/resolver"
)

const (
	// Parameters for log query

	// Day parameter specifies day for which logs need to be fetched. It is of the format yyyy-mm-dd. Defaults to today
	Day = "day"
	// MaxResults parameter specifies maximum number of results to return
	MaxResults = "max-results"
	// From parameter represents the start offset (zero based), used in paginated search requests.  The results returned would be in the range [From ... From+MaxResults-1]
	From = "from"

	// defaultMaxResults specifies default maximum number of results to return
	defaultMaxResults = 300
	elasticWaitIntvl  = time.Second
	maxElasticRetries = 200
)

// Option fills the optional params for fetch logs service
type Option func(*elasticLogRetriever)

// WithElasticClient passes a custom client for Elastic
func WithElasticClient(esClient elastic.ESClient) Option {
	return func(f *elasticLogRetriever) {
		f.elasticClient = esClient
	}
}

type elasticLogRetriever struct {
	sync.RWMutex
	elasticClient elastic.ESClient
	rslvr         resolver.Interface
	module        string
	node          string
	category      diagapi.ModuleStatus_CategoryType
	logger        log.Logger
	stopped       bool
}

func (f *elasticLogRetriever) HandleRequest(ctx context.Context, req *diagapi.DiagnosticsRequest) (*api.Any, error) {
	defer f.RUnlock()
	f.RLock()
	if f.stopped || f.elasticClient == nil {
		return nil, errors.New("elastic log retriever not initialized")
	}
	query := es.NewBoolQuery().QueryName("LogQuery")
	query = query.Must(es.NewTermQuery("fields.category", strings.ToLower(f.category.String())),
		es.NewTermQuery("module", f.module),
		es.NewTermQuery("host.name", f.node))
	index := elastic.LogIndexPrefix
	f.logger.DebugLog("method", "HandleRequest", "msg", "diagnostic request parameters: %+v", req.Parameters)
	day, ok := req.Parameters[Day]
	//TODO: check day format
	if ok {
		index = fmt.Sprintf("%s.%s.%s", elastic.InternalIndexPrefix, "default.systemlogs", day)
	}
	f.logger.DebugLog("method", "HandleRequest", "msg", fmt.Sprintf("index to search: %v", index))
	var from int
	fromStr, ok := req.Parameters[From]
	if ok {
		from, _ = strconv.Atoi(fromStr)
	}
	var err error
	maxResults := defaultMaxResults
	maxResultsStr, ok := req.Parameters[MaxResults]
	if ok {
		maxResults, err = strconv.Atoi(maxResultsStr)
		if err != nil {
			return nil, err
		}
	}
	result, err := f.elasticClient.Search(ctx, index, "", query, nil, int32(from), int32(maxResults), "@timestamp", true)
	if err != nil {
		f.logger.ErrorLog("method", "HandleRequest", "msg", fmt.Sprintf("search failed for query: %+v, result: %+v", *query, result), "err", err)
		return nil, err
	}
	f.logger.DebugLog("method", "HandleRequest", "msg", fmt.Sprintf("hits: %d, query: %#v", len(result.Hits.Hits), *query))
	logs := &protos.Logs{}
	for _, entry := range result.Hits.Hits {
		jsondata, err := entry.Source.MarshalJSON()
		if err == nil {
			databytes := []byte(jsondata)
			var logMsg protos.Log
			/*
				Example log entry in entry.Source saved by file beat in elastic
				{
					    "@timestamp": "2019-05-06T17:34:48.767Z",
					    "source": "/var/log/pensando/pen-spyglass.log",
					    "level": "error",
					    "ts": "2019-05-06T17:34:31.305424897Z",
					    "pid": "7",
					    "module": "pen-spyglass",
					    "fields": {
					      "category": "venice"
					    },
					    "beat": {
					      "version": "6.3.0",
					      "name": "node1",
					      "hostname": "node1",
					      "timezone": "+00:00"
					    },
					    "host": {
					      "name": "node1"
					    },
					    "offset": 11275,
					    "caller": "rpckit.go:421",
					    "msg": "Service spyglass could not connect to service pen-cmd, URL: :9009, err: context deadline exceeded",
					    "client": "spyglass"
				}
			*/
			json.Unmarshal([]byte(databytes), &logMsg.Log) // we ignore error from un-marshalling as non-string fields will throw error
			removeFilebeatFields(logMsg.Log)
			logs.Logs = append(logs.Logs, &logMsg)
		}
	}
	anyObj, err := types.MarshalAny(logs)
	if err != nil {
		f.logger.ErrorLog("method", "HandleRequest", "msg", "unable to marshal logs {%+v} to Any object", logs, "err", err)
		return nil, err
	}
	return &api.Any{Any: *anyObj}, nil
}

func (f *elasticLogRetriever) Start() error {
	defer f.Unlock()
	f.Lock()
	if f.stopped {
		// Initialize elastic client
		if f.elasticClient == nil {
			result, err := utils.ExecuteWithRetry(func(ctx context.Context) (interface{}, error) {
				return elastic.NewAuthenticatedClient("", f.rslvr, f.logger.WithContext("submodule", "elasticLogRetriever"))
			}, elasticWaitIntvl, maxElasticRetries)
			if err != nil {
				f.logger.ErrorLog("method", "Start", "msg", "failed to create elastic client", "err", err)
				return err
			}
			f.logger.DebugLog("method", "Start", "msg", "created Elastic client")
			f.elasticClient = result.(elastic.ESClient)
		}
		f.stopped = false
		f.logger.InfoLog("method", "Start", "msg", "started elastic log retriever")
	}
	return nil
}

func (f *elasticLogRetriever) Stop() {
	defer f.Unlock()
	f.Lock()
	if f.elasticClient != nil {
		f.elasticClient.Close()
		f.elasticClient = nil
	}
	f.stopped = true
}

func (f *elasticLogRetriever) IsStopped() bool {
	defer f.RUnlock()
	f.RLock()
	return f.stopped
}

// NewElasticLogsHandler returns handler to fetch logs from elastic
func NewElasticLogsHandler(module, node string, category diagapi.ModuleStatus_CategoryType, rslvr resolver.Interface, logger log.Logger, opts ...Option) diagnostics.Handler {
	svc := &elasticLogRetriever{
		module:   module,
		node:     node,
		category: category,
		rslvr:    rslvr,
		logger:   logger,
		stopped:  true,
	}
	for _, f := range opts {
		f(svc)
	}
	return svc
}

func removeFilebeatFields(log map[string]string) {
	const (
		beat   = "beat"
		host   = "host"
		fields = "fields"
		offset = "offset"
	)
	for _, key := range []string{beat, host, fields, offset} {
		delete(log, key)
	}
}
