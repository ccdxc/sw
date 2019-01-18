package debug

import (
	"context"
	"encoding/json"
	"fmt"
	"net"
	"net/http"
	"os"
	"path/filepath"
	"time"

	"github.com/gorilla/mux"

	"github.com/pensando/sw/venice/utils/log"
	"github.com/pensando/sw/venice/utils/ntsdb"
)

// SocketInfoFunction is the function signature the caller needs to pass into debug socket
type SocketInfoFunction = func() interface{}

// Debug allows for collecting local metrics and status for debugging
type Debug struct {
	srv            *http.Server
	socketInfoFunc SocketInfoFunction
	MetricObj      ntsdb.Obj
}

// New creates a new instance of Debug
func New(socketInfoFunc SocketInfoFunction) *Debug {
	return &Debug{
		socketInfoFunc: socketInfoFunc,
	}
}

// StartServer starts the socket listener
func (ds *Debug) StartServer(dbgSockPath string) error {
	router := mux.NewRouter()
	os.MkdirAll(filepath.Dir(dbgSockPath), 0700)
	router.HandleFunc("/debug", ds.DebugHandler).Methods("GET")
	os.Remove(dbgSockPath)
	l, err := net.Listen("unix", dbgSockPath)
	if err != nil {
		log.Errorf("failed to initialize debug, %s", err)
		return err
	}
	log.Infof("Started debug socket, %s", dbgSockPath)
	srv := &http.Server{Handler: router}
	ds.srv = srv
	go func() {
		defer l.Close()
		err := srv.Serve(l)
		if err != nil && err != http.ErrServerClosed {
			log.Error(err)
		}
	}()
	return nil
}

// DebugHandler handles incoming http requests and writes the output of socketInfoFunction into the response
func (ds *Debug) DebugHandler(w http.ResponseWriter, r *http.Request) {
	if ds.socketInfoFunc == nil {
		json.NewEncoder(w).Encode("Socket info function isn't defined, please check your debug setup")
		return
	}
	json.NewEncoder(w).Encode(ds.socketInfoFunc())
}

// BuildMetricObj initializes the metric table
func (ds *Debug) BuildMetricObj(tableName string, keyTags map[string]string) error {
	if !ntsdb.IsInitialized() {
		return fmt.Errorf("ntsdb is not initialized")
	}
	metricObj, err := ntsdb.NewObj(tableName, keyTags, nil, &ntsdb.ObjOpts{Local: true})
	if err != nil {
		return err
	}
	ds.MetricObj = metricObj
	return nil
}

// Destroy deletes the metric table and stops the server
func (ds *Debug) Destroy() error {
	ds.DeleteMetricObj()
	err := ds.StopServer()
	return err
}

// StopServer gracefully shutsdown the socket
func (ds *Debug) StopServer() error {
	if ds.srv != nil {
		ctx, cancelFunc := context.WithTimeout(context.Background(), 3*time.Second)
		defer cancelFunc()
		if err := ds.srv.Shutdown(ctx); err != nil {
			log.Errorf("failed to gracefully shutdown debug socket, %s", err)
			return err
		}
		ds.srv = nil
	}
	return nil
}

// DeleteMetricObj deletes the local metric object
func (ds *Debug) DeleteMetricObj() error {
	if ds.MetricObj != nil {
		ds.MetricObj.Delete()
		ds.MetricObj = nil
	}
	return nil
}
