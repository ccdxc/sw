// {C} Copyright 2017 Pensando Systems Inc. All rights reserved.

package testutils

import (
	"net"
	"net/http"
	"sync"

	"github.com/gorilla/mux"
	"github.com/pkg/errors"

	"github.com/pensando/sw/venice/utils"
	"github.com/pensando/sw/venice/utils/log"
)

// MockServer represents the mock http server
type MockServer struct {
	listener          net.Listener
	server            *http.Server
	router            *mux.Router
	wg                sync.WaitGroup
	defaultStatusCode int // if set, this will be returned by default
}

// NewMockServer returns instance of the mock server
func NewMockServer() *MockServer {
	listener, err := net.Listen("tcp", "127.0.0.1:0")
	if err != nil {
		log.Fatalf("error starting mock server: %v", err)
	}

	return &MockServer{
		router:   mux.NewRouter(),
		listener: listener,
	}
}

// SetDefaultStatusCode sets the default HTTP status code for all the subsequent calls.
// e.g. StatusInternalServerError can be set to simulate internal server errors.
func (m *MockServer) SetDefaultStatusCode(statusCode int) {
	m.defaultStatusCode = statusCode
}

// ClearDefaultStatusCode clears the default HTTP status code.
func (m *MockServer) ClearDefaultStatusCode() {
	m.defaultStatusCode = 0
}

// AddHandler adds the given handler to the server
func (m *MockServer) AddHandler(path, method string, handlerFunc func(w http.ResponseWriter, r *http.Request)) error {
	if utils.IsEmpty(path) || handlerFunc == nil {
		return errors.New("missing path/handler")
	}

	// check for the simulated status code and act accordingly
	handler := func(w http.ResponseWriter, r *http.Request) {
		if m.defaultStatusCode != 0 {
			w.WriteHeader(m.defaultStatusCode)
			return
		}

		handlerFunc(w, r)
	}

	if !utils.IsEmpty(method) {
		m.router.Path(path).Methods(method).HandlerFunc(handler)
		return nil
	}

	m.router.Path(path).HandlerFunc(handler)
	return nil
}

// Start starts the mock http server on a random port
func (m *MockServer) Start() {
	m.server = &http.Server{
		Handler: m.router,
	}

	m.wg.Add(1)
	go func() {
		log.Info("starting mock server")
		if err := m.server.Serve(m.listener); err != nil {
			log.Errorf("stopped mock server: %v", err)
		}
		m.wg.Done()
	}()
}

// Stop stops the mock server
func (m *MockServer) Stop() {
	log.Debug("received stop signal, shutting down the server")
	m.listener.Close()
	m.server.Close()

	// wait for the graceful shutdown of the server
	// this will ensure the go routine is closed/stopped.
	m.wg.Wait()
}

// URL returns the listen URL IP:Port
func (m *MockServer) URL() string {
	return m.listener.Addr().String()
}
