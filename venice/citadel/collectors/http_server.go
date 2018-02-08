// {C} Copyright 2017 Pensando Systems Inc. All rights reserved.

package collectors

import (
	"bytes"
	"context"
	"encoding/json"
	"fmt"
	"io/ioutil"
	"net/http"
	"strings"
	"time"

	"github.com/gorilla/mux"
	"github.com/influxdata/influxdb/models"
	"github.com/influxdata/influxdb/query"

	"github.com/pensando/sw/venice/citadel/broker"
	"github.com/pensando/sw/venice/citadel/meta"
	"github.com/pensando/sw/venice/citadel/tproto"
	"github.com/pensando/sw/venice/utils/log"
	"github.com/pensando/sw/venice/utils/netutils"
)

// HTTPServer is the http server instance
type HTTPServer struct {
	listenURL string         // listening URL
	server    *http.Server   // HTTP server
	broker    *broker.Broker // broker instance
}

// NewHTTPServer creates a http server
func NewHTTPServer(listenURL string, broker *broker.Broker) (*HTTPServer, error) {
	// create http server instance
	hsrv := HTTPServer{
		listenURL: listenURL,
		broker:    broker,
	}

	// create a mux and setup routes
	r := mux.NewRouter()

	// tsdb apis
	r.HandleFunc("/write", netutils.MakeHTTPHandler(netutils.RestAPIFunc(hsrv.writeReqHandler))).Methods("POST")
	r.HandleFunc("/create", netutils.MakeHTTPHandler(netutils.RestAPIFunc(hsrv.createdbReqHandler))).Methods("POST")
	r.HandleFunc("/query", hsrv.queryReqHandler).Methods("GET")
	r.HandleFunc("/query", hsrv.queryReqHandler).Methods("POST")

	// kv apis
	r.HandleFunc("/kv/get", hsrv.getKvReqHandler).Methods("GET")
	r.HandleFunc("/kv/list", netutils.MakeHTTPHandler(netutils.RestAPIFunc(hsrv.listKvReqHandler))).Methods("GET")
	r.HandleFunc("/kv/put", netutils.MakeHTTPHandler(netutils.RestAPIFunc(hsrv.putKvReqHandler))).Methods("POST")
	r.HandleFunc("/kv/delete", netutils.MakeHTTPHandler(netutils.RestAPIFunc(hsrv.delKvReqHandler))).Methods("POST")

	// debug api for getting cluster state
	r.HandleFunc("/info", netutils.MakeHTTPHandler(netutils.RestAPIFunc(hsrv.infoReqHandler))).Methods("GET")

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

	// create db
	err := hsrv.broker.CreateDatabase(context.Background(), database)
	if err != nil {
		log.Errorf("Error creating the database %s. Err: %v", database, err)
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

	// execute the query
	result, err := hsrv.broker.ExecuteQuery(context.Background(), database, qp)
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
