// {C} Copyright 2017 Pensando Systems Inc. All rights reserved.

package httpserver

import (
	"bytes"
	"context"
	"encoding/json"
	"expvar"
	"fmt"
	"io/ioutil"
	"net/http"
	"net/http/pprof"
	"strconv"
	"strings"
	"time"

	"github.com/pensando/sw/venice/globals"

	"github.com/pensando/sw/venice/citadel/data"

	"github.com/gorilla/mux"
	"github.com/influxdata/influxdb/models"
	"github.com/influxdata/influxdb/query"

	"github.com/pensando/sw/venice/citadel/broker"
	"github.com/pensando/sw/venice/citadel/meta"
	"github.com/pensando/sw/venice/citadel/tproto"
	"github.com/pensando/sw/venice/utils/debug"
	"github.com/pensando/sw/venice/utils/log"
	"github.com/pensando/sw/venice/utils/netutils"
)

// HTTPServer is the http server instance
type HTTPServer struct {
	listenURL string         // listening URL
	server    *http.Server   // HTTP server
	broker    *broker.Broker // broker instance
	dnode     *data.DNode    // data Node
}

// NewHTTPServer creates a http server
func NewHTTPServer(listenURL string, broker *broker.Broker, dn *data.DNode, dbg *debug.Debug) (*HTTPServer, error) {
	// create http server instance
	hsrv := HTTPServer{
		listenURL: listenURL,
		broker:    broker,
		dnode:     dn,
	}

	// create a mux and setup routes
	r := mux.NewRouter()

	// tsdb apis
	r.HandleFunc("/write", netutils.MakeHTTPHandler(netutils.RestAPIFunc(hsrv.writeReqHandler))).Methods("POST")
	r.HandleFunc("/db", netutils.MakeHTTPHandler(netutils.RestAPIFunc(hsrv.createdbReqHandler))).Methods("POST")
	r.HandleFunc("/db", netutils.MakeHTTPHandler(netutils.RestAPIFunc(hsrv.readdbReqHandler))).Methods("GET")
	r.HandleFunc("/db", netutils.MakeHTTPHandler(netutils.RestAPIFunc(hsrv.deletedbReqHandler))).Methods("DELETE")
	r.HandleFunc("/query", hsrv.queryReqHandler).Methods("GET")
	r.HandleFunc("/query", hsrv.queryReqHandler).Methods("POST")
	r.HandleFunc("/shard", hsrv.queryShardReqHandler).Methods("POST")
	r.HandleFunc("/shard", hsrv.queryShardReqHandler).Methods("GET")
	r.HandleFunc("/replica", hsrv.queryReplicaReqHandler).Methods("GET")
	r.HandleFunc("/cmd", hsrv.showReqHandler).Methods("GET")
	r.HandleFunc("/cmd", hsrv.showReqHandler).Methods("POST")
	r.HandleFunc("/dnode", hsrv.dnodeReqHandler).Methods("GET")
	r.HandleFunc("/ping", hsrv.pingReqHandler).Methods("GET")
	r.HandleFunc("/cq", netutils.MakeHTTPHandler(netutils.RestAPIFunc(hsrv.createcqReqHandler))).Methods("POST")
	r.HandleFunc("/cq", netutils.MakeHTTPHandler(netutils.RestAPIFunc(hsrv.readcqReqHandler))).Methods("GET")
	r.HandleFunc("/cq", netutils.MakeHTTPHandler(netutils.RestAPIFunc(hsrv.deletecqReqHandler))).Methods("DELETE")
	r.HandleFunc("/rp", netutils.MakeHTTPHandler(netutils.RestAPIFunc(hsrv.createrpReqHandler))).Methods("POST")
	r.HandleFunc("/rp", netutils.MakeHTTPHandler(netutils.RestAPIFunc(hsrv.readrpReqHandler))).Methods("GET")
	r.HandleFunc("/rp", netutils.MakeHTTPHandler(netutils.RestAPIFunc(hsrv.deleterpReqHandler))).Methods("DELETE")
	r.HandleFunc("/healthz", netutils.MakeHTTPHandler(netutils.RestAPIFunc(hsrv.healthReqHandler))).Methods("GET")

	// kv apis
	r.HandleFunc("/kv/get", hsrv.getKvReqHandler).Methods("GET")
	r.HandleFunc("/kv/list", netutils.MakeHTTPHandler(netutils.RestAPIFunc(hsrv.listKvReqHandler))).Methods("GET")
	r.HandleFunc("/kv/put", netutils.MakeHTTPHandler(netutils.RestAPIFunc(hsrv.putKvReqHandler))).Methods("POST")
	r.HandleFunc("/kv/delete", netutils.MakeHTTPHandler(netutils.RestAPIFunc(hsrv.delKvReqHandler))).Methods("POST")

	// pprof
	r.Methods("GET").Subrouter().Handle("/debug/vars", expvar.Handler())
	r.Methods("GET").Subrouter().HandleFunc("/debug/pprof/", pprof.Index)
	r.Methods("GET").Subrouter().HandleFunc("/debug/pprof/cmdline", pprof.Cmdline)
	r.Methods("GET").Subrouter().HandleFunc("/debug/pprof/profile", pprof.Profile)
	r.Methods("GET").Subrouter().HandleFunc("/debug/pprof/symbol", pprof.Symbol)
	r.Methods("GET").Subrouter().HandleFunc("/debug/pprof/trace", pprof.Trace)
	r.Methods("GET").Subrouter().HandleFunc("/debug/pprof/allocs", pprof.Handler("allocs").ServeHTTP)
	r.Methods("GET").Subrouter().HandleFunc("/debug/pprof/block", pprof.Handler("block").ServeHTTP)
	r.Methods("GET").Subrouter().HandleFunc("/debug/pprof/heap", pprof.Handler("heap").ServeHTTP)
	r.Methods("GET").Subrouter().HandleFunc("/debug/pprof/mutex", pprof.Handler("mutex").ServeHTTP)
	r.Methods("GET").Subrouter().HandleFunc("/debug/pprof/goroutine", pprof.Handler("goroutine").ServeHTTP)
	r.Methods("GET").Subrouter().HandleFunc("/debug/pprof/threadcreate", pprof.Handler("threadcreate").ServeHTTP)

	// debug api for getting cluster state
	r.HandleFunc("/info", netutils.MakeHTTPHandler(netutils.RestAPIFunc(hsrv.infoReqHandler))).Methods("GET")
	if dbg != nil {
		r.HandleFunc("/debug", dbg.DebugHandler).Methods("GET")
	}

	// all unknown routes
	r.HandleFunc("/{*}", unknownAction)

	// run a HTTP server
	srv := &http.Server{Addr: listenURL, Handler: r}
	srv.SetKeepAlivesEnabled(false)
	go srv.ListenAndServe()
	hsrv.server = srv

	return &hsrv, nil
}

// GetAddr returns the address where http server is listening
func (hsrv *HTTPServer) GetAddr() string {
	return hsrv.server.Addr
}

// unknownAction handles all unknown REST calls
func unknownAction(w http.ResponseWriter, r *http.Request) {
	log.Infof("Unknown HTTP request %s %q", r.Method, r.URL.Path)
	log.Infof("HTTP request headers %+v, URL: %+v", r, r.URL)

	content, _ := ioutil.ReadAll(r.Body)
	log.Infof("Body content: %s", string(content))
	w.WriteHeader(http.StatusServiceUnavailable)
}

// createdbReqHandler creates a database
func (hsrv *HTTPServer) createdbReqHandler(r *http.Request) (interface{}, error) {
	database := r.URL.Query().Get("db")
	if database == "" {
		database = globals.DefaultTenant
	}

	retention := uint64(0)
	if rt := r.URL.Query().Get("retention"); rt != "" {
		d, err := time.ParseDuration(rt)
		if err != nil {
			return nil, err
		}
		retention = uint64(d)
	}

	// create db
	err := hsrv.broker.CreateDatabaseWithRetention(context.Background(), database, retention)
	if err != nil {
		log.Errorf("Error creating the database %s. Err: %v", database, err)
		return nil, err
	}

	return nil, nil
}

// readdbReqHandler reads all databases
func (hsrv *HTTPServer) readdbReqHandler(r *http.Request) (interface{}, error) {

	// create db
	dbInfo, err := hsrv.broker.ReadDatabases(context.Background())
	if err != nil {
		log.Errorf("Error reading databases. Err: %v", err)
		return nil, err
	}

	return dbInfo, nil
}

// deletedbReqHandler deletes a database
func (hsrv *HTTPServer) deletedbReqHandler(r *http.Request) (interface{}, error) {
	database := r.URL.Query().Get("db")

	// create db
	err := hsrv.broker.DeleteDatabase(context.Background(), database)
	if err != nil {
		log.Errorf("Error deleting the database %s. Err: %v", database, err)
		return nil, err
	}

	return nil, nil
}

// createcqReqHandler creates continuous query
func (hsrv *HTTPServer) createcqReqHandler(r *http.Request) (interface{}, error) {
	database := r.URL.Query().Get("db")
	if database == "" {
		database = globals.DefaultTenant
	}
	cq := r.URL.Query().Get("cq")
	if cq == "" {
		return nil, fmt.Errorf("empty cq for continuous query name")
	}
	query := r.URL.Query().Get("query")
	if query == "" {
		return nil, fmt.Errorf("empty query for query string")
	}
	retentionName := r.URL.Query().Get("rp")
	if retentionName == "" {
		return nil, fmt.Errorf("empty rp for retention policy name")
	}
	retentionPeriod := r.URL.Query().Get("rptime")
	if retentionName == "" {
		return nil, fmt.Errorf("empty rptime for retention policy period")
	}

	// create db
	period, err := strconv.ParseUint(retentionPeriod, 10, 64)
	if err != nil {
		return nil, fmt.Errorf("Error parse input retention period to uint64")
	}
	err = hsrv.broker.CreateContinuousQuery(context.Background(), database, cq, retentionName, period, query)
	if err != nil {
		log.Errorf("Error creating the continuous query %+v in database %+v. Err: %v", cq, database, err)
		return nil, err
	}

	return nil, nil
}

// readcqReqHandler reads all databases
func (hsrv *HTTPServer) readcqReqHandler(r *http.Request) (interface{}, error) {
	database := r.URL.Query().Get("db")
	if database == "" {
		database = globals.DefaultTenant
	}
	replicaID := r.URL.Query().Get("replica")
	result, err := hsrv.broker.GetContinuousQuery(context.Background(), database, replicaID)
	if err != nil {
		log.Errorf("Error reading continuous queries. Err: %v", err)
		return nil, err
	}
	cqBytes, err := json.Marshal(result)
	if err != nil {
		log.Errorf("Error marshal continuous query result for output. Err: %v", err)
		return nil, err
	}

	return string(cqBytes), nil
}

// deletecqReqHandler deletes a database
func (hsrv *HTTPServer) deletecqReqHandler(r *http.Request) (interface{}, error) {
	database := r.URL.Query().Get("db")
	if database == "" {
		database = globals.DefaultTenant
	}
	cq := r.URL.Query().Get("cq")
	if cq == "" {
		return nil, fmt.Errorf("empty cq for continuous query name")
	}
	measurement := r.URL.Query().Get("measurement")

	// create db
	err := hsrv.broker.DeleteContinuousQuery(context.Background(), database, cq, measurement)
	if err != nil {
		log.Errorf("Error deleting the continuous query %s in database %s. Err: %v", cq, database, err)
		return nil, err
	}

	return nil, nil
}

// createrpReqHandler creates retention policy
func (hsrv *HTTPServer) createrpReqHandler(r *http.Request) (interface{}, error) {
	database := r.URL.Query().Get("db")
	if database == "" {
		database = globals.DefaultTenant
	}
	retentionName := r.URL.Query().Get("rp")
	if retentionName == "" {
		return nil, fmt.Errorf("empty rp for retention policy name")
	}
	retentionPeriod := r.URL.Query().Get("rptime")
	if retentionName == "" {
		return nil, fmt.Errorf("empty rptime for retention policy period")
	}

	period, err := strconv.ParseUint(retentionPeriod, 10, 64)
	if err != nil {
		return nil, fmt.Errorf("Error parse input retention period to uint64")
	}
	err = hsrv.broker.CreateRetentionPolicy(context.Background(), database, retentionName, period)
	if err != nil {
		log.Errorf("Error creating the retention policy %+v in database %+v. Err: %v", retentionName, database, err)
		return nil, err
	}

	return nil, nil
}

// readrpReqHandler reads retention policy
func (hsrv *HTTPServer) readrpReqHandler(r *http.Request) (interface{}, error) {
	database := r.URL.Query().Get("db")
	if database == "" {
		database = globals.DefaultTenant
	}
	result, err := hsrv.broker.GetRetentionPolicy(context.Background(), database)
	if err != nil {
		log.Errorf("Error reading continuous queries. Err: %v", err)
		return nil, err
	}

	return result, nil
}

// deleterpReqHandler deletes retention policy
func (hsrv *HTTPServer) deleterpReqHandler(r *http.Request) (interface{}, error) {
	database := r.URL.Query().Get("db")
	if database == "" {
		database = globals.DefaultTenant
	}
	retentionName := r.URL.Query().Get("rp")
	if retentionName == "" {
		return nil, fmt.Errorf("empty rp for retention policy name")
	}

	err := hsrv.broker.DeleteRetentionPolicy(context.Background(), database, retentionName)
	if err != nil {
		log.Errorf("Error deleting retention policy %s in database %s. Err: %v", retentionName, database, err)
		return nil, err
	}

	return nil, nil
}

// writeReqHandler handles write requests
func (hsrv *HTTPServer) writeReqHandler(r *http.Request) (interface{}, error) {
	// some error checking
	if r.Header.Get("Content-Encoding") == "gzip" {
		log.Infof("gzip encoding detected")
	}
	if r.ContentLength <= 0 {
		return nil, fmt.Errorf("Bad content length %d", r.ContentLength)
	}

	// read the message
	bs := make([]byte, 0, r.ContentLength)
	buf := bytes.NewBuffer(bs)
	_, err := buf.ReadFrom(r.Body)
	if err != nil {
		return nil, err
	}

	// parse the points from message
	points, parseError := models.ParsePointsWithPrecision(buf.Bytes(), time.Now().UTC(), r.URL.Query().Get("precision"))
	if parseError != nil {
		return nil, parseError
	}
	database := r.URL.Query().Get("db")

	// call the broker
	err = hsrv.broker.WritePoints(context.Background(), database, points)
	if err != nil {
		log.Errorf("Error writing points. Err: %v", err)
		return nil, err
	}

	return nil, nil
}

// queryReqHandler handles a query request
func (hsrv *HTTPServer) queryReqHandler(w http.ResponseWriter, r *http.Request) {
	// Attempt to read the form value from the "q" form value.
	qp := strings.TrimSpace(r.FormValue("q"))
	if qp == "" {
		http.Error(w, `missing required parameter "q"`, http.StatusBadRequest)
		return
	}
	database := r.FormValue("db")
	if database == "" {
		database = globals.DefaultTenant
	}

	// execute the query
	result, err := hsrv.broker.ExecuteQuery(context.Background(), database, qp)
	if err != nil {
		http.Error(w, fmt.Sprintf("Error executing the query: %v", err), http.StatusInternalServerError)
		return
	}

	// write 200 ok
	w.Header().Add("Content-Type", "application/json")
	w.WriteHeader(http.StatusOK)

	var o struct {
		Results []*query.Result `json:"results,omitempty"`
		Err     string          `json:"error,omitempty"`
	}

	// loop thru each result
	for _, res := range result {
		o.Results = append(o.Results, res)
	}

	// Send HTTP response as Json
	content, err := json.Marshal(o)
	if err != nil {
		http.Error(w, err.Error(), http.StatusInternalServerError)
		return
	}

	w.Write(content)

	return
}

// queryShardReqHandler handles query to a shard
func (hsrv *HTTPServer) queryShardReqHandler(w http.ResponseWriter, r *http.Request) {
	var shardID uint

	// Attempt to read the form value from the "q" form value.
	qp := strings.TrimSpace(r.FormValue("q"))
	if qp == "" {
		http.Error(w, `missing required parameter "q"`, http.StatusBadRequest)
		return
	}
	database := r.FormValue("db")
	if database == "" {
		database = globals.DefaultTenant
	}

	shard := strings.TrimSpace(r.FormValue("shard"))
	if shard != "" {
		n, err := strconv.Atoi(shard)
		if err != nil {
			http.Error(w, err.Error(), http.StatusBadRequest)
			return
		}
		shardID = uint(n - 1) // convert to index
	}

	// execute the query
	result, err := hsrv.broker.ExecuteQueryShard(context.Background(), database, qp, shardID)
	if err != nil {
		http.Error(w, fmt.Sprintf("Error executing the query: %v", err), http.StatusInternalServerError)
		return
	}

	// write 200 ok
	w.Header().Add("Content-Type", "application/json")
	w.WriteHeader(http.StatusOK)

	// loop thru each result
	for _, res := range result {
		var o struct {
			Results []*query.Result `json:"results,omitempty"`
			Err     string          `json:"error,omitempty"`
		}
		o.Results = []*query.Result{res}
		// Send HTTP response as Json
		content, err := json.Marshal(o)
		if err != nil {
			http.Error(w, err.Error(), http.StatusInternalServerError)
			return
		}

		w.Write(content)
	}
	return
}

// queryReplicaReqHandler handles query to a shard
func (hsrv *HTTPServer) queryReplicaReqHandler(w http.ResponseWriter, r *http.Request) {
	var shardID uint

	// Attempt to read the form value from the "q" form value.
	qp := strings.TrimSpace(r.FormValue("q"))
	if qp == "" {
		http.Error(w, `missing required parameter "q"`, http.StatusBadRequest)
		return
	}
	database := r.FormValue("db")
	if database == "" {
		database = globals.DefaultTenant
	}

	shard := strings.TrimSpace(r.FormValue("shard"))
	if shard != "" {
		n, err := strconv.Atoi(shard)
		if err != nil {
			http.Error(w, err.Error(), http.StatusBadRequest)
			return
		}
		shardID = uint(n - 1) // convert to index
	}

	primary := strings.TrimSpace(r.FormValue("primary"))
	if primary == "" {
		http.Error(w, `missing required parameter "primary"`, http.StatusBadRequest)
		return
	}

	isPrimary, err := strconv.ParseBool(primary)
	if err != nil {
		http.Error(w, fmt.Sprintf(`cannot parse primary input value %v to boolean`, isPrimary), http.StatusBadRequest)
		return
	}

	// execute the query
	result, err := hsrv.broker.ExecuteQueryReplica(context.Background(), database, qp, shardID, isPrimary)
	if err != nil {
		http.Error(w, fmt.Sprintf("Error executing the query: %v", err), http.StatusInternalServerError)
		return
	}

	// write 200 ok
	w.Header().Add("Content-Type", "application/json")
	w.WriteHeader(http.StatusOK)

	// loop thru each result
	for _, res := range result {
		var o struct {
			Results []*query.Result `json:"results,omitempty"`
			Err     string          `json:"error,omitempty"`
		}
		o.Results = []*query.Result{res}
		// Send HTTP response as Json
		content, err := json.Marshal(o)
		if err != nil {
			http.Error(w, err.Error(), http.StatusInternalServerError)
			return
		}

		w.Write(content)
	}
	return
}

// showReqHandler handles a show command
func (hsrv *HTTPServer) showReqHandler(w http.ResponseWriter, r *http.Request) {
	// Attempt to read the form value from the "q" form value.
	qp := strings.ToUpper(strings.TrimSpace(r.FormValue("q")))
	if qp == "" {
		http.Error(w, `missing required parameter "q"`, http.StatusBadRequest)
		return
	}
	database := r.FormValue("db")
	if database == "" {
		database = globals.DefaultTenant
	}

	// execute the query
	result, err := hsrv.broker.ExecuteShowCmd(context.Background(), database, qp)
	if err != nil {
		http.Error(w, fmt.Sprintf("Error executing the show command: %v", err), http.StatusInternalServerError)
		return
	}

	// write 200 ok
	w.Header().Add("Content-Type", "application/json")
	w.WriteHeader(http.StatusOK)

	// loop thru each result
	for _, res := range result {
		var o struct {
			Results []*query.Result `json:"results,omitempty"`
			Err     string          `json:"error,omitempty"`
		}
		o.Results = []*query.Result{res}
		// Send HTTP response as Json
		content, err := json.Marshal(o)
		if err != nil {
			http.Error(w, err.Error(), http.StatusInternalServerError)
			return
		}

		w.Write(content)
	}
	return
}

// pingReqHandler handles a ping request
func (hsrv *HTTPServer) pingReqHandler(w http.ResponseWriter, r *http.Request) {
	// return success, called by influxdb tools
}

// dnodeReqHandler shows data node info
func (hsrv *HTTPServer) dnodeReqHandler(w http.ResponseWriter, r *http.Request) {
	w.Header().Add("Content-Type", "application/json")
	w.WriteHeader(http.StatusOK)
	if hsrv.dnode != nil {
		w.Write([]byte(hsrv.dnode.String()))
	}
}

// healthReqHandler handles health check requests
func (hsrv *HTTPServer) healthReqHandler(r *http.Request) (interface{}, error) {
	if err := hsrv.broker.ClusterCheck(); err != nil {
		return nil, err
	}
	if hsrv.dnode != nil {
		if hsrv.dnode.HasPendingSync() {
			return nil, fmt.Errorf("sync pending")
		}
	} else {
		return nil, fmt.Errorf("node is not ready")
	}

	return nil, nil
}

// infoReqHandler returns cluster state
func (hsrv *HTTPServer) infoReqHandler(r *http.Request) (interface{}, error) {
	ret := make(map[string]*meta.TscaleCluster)

	// read the cluster state
	ret[meta.ClusterTypeTstore] = hsrv.broker.GetCluster(meta.ClusterTypeTstore)
	ret[meta.ClusterTypeKstore] = hsrv.broker.GetCluster(meta.ClusterTypeKstore)

	return ret, nil
}

// getKvReqHandler returns the key-value pairs by key
func (hsrv *HTTPServer) getKvReqHandler(w http.ResponseWriter, r *http.Request) {
	var keys []*tproto.Key
	var kvlist []map[string]string

	table := r.URL.Query().Get("table")
	if table == "" {
		http.Error(w, fmt.Sprintf("missing required parameter 'table'"), http.StatusInternalServerError)
		return
	}

	// Attempt to read the form value from the "q" form value.
	kstr := strings.TrimSpace(r.FormValue("keys"))
	if kstr == "" {
		http.Error(w, fmt.Sprintf(`missing required parameter "keys"`), http.StatusInternalServerError)
		return
	}

	// parse the string into keys
	klist := strings.Split(kstr, ",")
	for _, k := range klist {
		keys = append(keys, &tproto.Key{Key: []byte(k)})
	}

	result, err := hsrv.broker.ReadKvs(context.Background(), table, keys)
	if err != nil {
		http.Error(w, err.Error(), http.StatusInternalServerError)
		return
	}

	// write 200 ok
	w.WriteHeader(http.StatusOK)

	// loop thru each result
	for _, kv := range result {
		kvmap := make(map[string]string)
		kvmap["Key"] = string(kv.Key)
		kvmap["Value"] = string(kv.Value)
		kvlist = append(kvlist, kvmap)
	}

	// Send HTTP response as Json
	content, err := json.Marshal(kvlist)
	if err != nil {
		http.Error(w, err.Error(), http.StatusInternalServerError)
		return
	}
	w.Write(content)

}

// list all keys in a table
func (hsrv *HTTPServer) listKvReqHandler(r *http.Request) (interface{}, error) {
	var kvlist []map[string]string
	table := r.URL.Query().Get("table")
	if table == "" {
		return nil, fmt.Errorf("missing required parameter 'table'")
	}

	// get the list from broker
	kvl, err := hsrv.broker.ListKvs(context.Background(), table)
	for _, kv := range kvl {
		kvmap := make(map[string]string)
		kvmap["Key"] = string(kv.Key)
		kvmap["Value"] = string(kv.Value)
		kvlist = append(kvlist, kvmap)
	}

	return kvlist, err
}

func (hsrv *HTTPServer) putKvReqHandler(r *http.Request) (interface{}, error) {
	var kvs []*tproto.KeyValue

	// some error checking
	if r.Header.Get("Content-Encoding") == "gzip" {
		log.Infof("gzip encoding detected")
	}
	if r.ContentLength <= 0 {
		return nil, fmt.Errorf("Bad content length %d", r.ContentLength)
	}

	table := r.URL.Query().Get("table")
	if table == "" {
		return nil, fmt.Errorf("missing required parameter 'table'")
	}

	// read the message
	bs := make([]byte, 0, r.ContentLength)
	buf := bytes.NewBuffer(bs)
	_, err := buf.ReadFrom(r.Body)
	if err != nil {
		return nil, err
	}

	if buf.String() == "" {
		return nil, fmt.Errorf("missing key-value pairs")
	}

	log.Infof("Got values: %s", buf.Bytes())

	reqstr := strings.TrimSuffix(strings.TrimPrefix(string(buf.Bytes()), "\""), "\"")

	// parse the string into keys
	kvlist := strings.Split(reqstr, ",")
	for _, kvstr := range kvlist {
		kvp := strings.Split(kvstr, "=")
		if len(kvp) != 2 {
			return nil, fmt.Errorf("invalid key-value pair %s", kvstr)
		}
		kv := &tproto.KeyValue{
			Key:   []byte(kvp[0]),
			Value: []byte(kvp[1]),
		}
		kvs = append(kvs, kv)
		log.Infof("Writing kv: %+v", kv)

	}

	return nil, hsrv.broker.WriteKvs(context.Background(), table, kvs)
}

func (hsrv *HTTPServer) delKvReqHandler(r *http.Request) (interface{}, error) {
	var keys []*tproto.Key

	// some error checking
	if r.Header.Get("Content-Encoding") == "gzip" {
		log.Infof("gzip encoding detected")
	}
	if r.ContentLength <= 0 {
		return nil, fmt.Errorf("Bad content length %d", r.ContentLength)
	}

	table := r.URL.Query().Get("table")
	if table == "" {
		return nil, fmt.Errorf("missing required parameter 'table'")
	}

	// read the message
	bs := make([]byte, 0, r.ContentLength)
	buf := bytes.NewBuffer(bs)
	_, err := buf.ReadFrom(r.Body)
	if err != nil {
		return nil, err
	}

	if buf.String() == "" {
		return nil, fmt.Errorf("missing keys to delete")
	}

	// parse the string into keys
	klist := strings.Split(buf.String(), ",")
	for _, k := range klist {
		keys = append(keys, &tproto.Key{Key: []byte(k)})
	}

	return nil, hsrv.broker.DeleteKvs(context.Background(), table, keys)
}
