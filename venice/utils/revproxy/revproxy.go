package revproxy

import (
	"crypto/tls"
	"fmt"
	"net"
	"net/http"
	"net/http/httputil"
	"net/url"
	"sort"
	"sync"

	"github.com/gorilla/mux"

	"github.com/pensando/sw/venice/utils/log"
)

// Authorizer is a pluggable function that returns true when the supplied request is authorized, false otherwise
type Authorizer func(r *http.Request) error

// ReverseProxyRouter is a HTTP/HTTPs reverse proxy based on gorilla mux
type ReverseProxyRouter struct {
	sync.Mutex
	listenURL  string
	muxRouter  *mux.Router
	tlsConfig  *tls.Config
	httpServer *http.Server // HTTP server
	authorizer Authorizer
}

const (
	contentLengthLogThreshold = 2 * 1024 // 2KB
)

func dumpHTTPRequest(next http.Handler) http.Handler {
	return http.HandlerFunc(func(w http.ResponseWriter, r *http.Request) {
		if r.ContentLength > contentLengthLogThreshold {
			log.Infof("RevProxy: ContentLength: %d", r.ContentLength)
			dump, err := httputil.DumpRequest(r, false)
			if err != nil {
				log.Infof("RevProxy: Could not dump request: %s", fmt.Sprint(err))
			} else {
				log.Infof("RevProxy: Client Request: %s Forms %v", dump, r.Header)
			}
		} else {
			dump, err := httputil.DumpRequest(r, true)
			if err != nil {
				log.Infof("RevProxy: Could not dump request: %s", fmt.Sprint(err))
			} else {
				log.Infof("RevProxy: Client Request: %s", dump)
			}
		}
		// Call the next handler
		next.ServeHTTP(w, r)
	})
}

func logTLSInfo(next http.Handler) http.Handler {
	return http.HandlerFunc(func(w http.ResponseWriter, r *http.Request) {
		if r.TLS != nil {
			if r.TLS.VerifiedChains != nil && len(r.TLS.VerifiedChains) >= 0 {
				clientCert := r.TLS.VerifiedChains[0][0]
				log.Infof("RevProxy: found valid client certificate. Subj: %+v, SANs: %+v", clientCert.Subject, clientCert.DNSNames)
			}
		}
		next.ServeHTTP(w, r)
	})
}

func (rpr *ReverseProxyRouter) authorize(next http.Handler) http.Handler {
	return http.HandlerFunc(func(w http.ResponseWriter, r *http.Request) {
		if rpr.authorizer != nil {
			err := rpr.authorizer(r)
			if err != nil {
				log.Infof("Authorization failed for request: %+v", r)
				http.Error(w, err.Error(), http.StatusUnauthorized)
				return
			}
			log.Infof("Authorization succeeded for request: %+v", r) // FIXME REMOVE
		}
		next.ServeHTTP(w, r)
	})
}

func (rpr *ReverseProxyRouter) handleStopRequest(res http.ResponseWriter, req *http.Request) {
	go rpr.Stop()
}

// UpdateConfig updates the configuration of the reverse proxy.
// The method can only be called when the proxy is stopped.
func (rpr *ReverseProxyRouter) UpdateConfig(config map[string]string) error {
	rpr.Lock()
	defer rpr.Unlock()
	if rpr.httpServer != nil {
		return fmt.Errorf("Cannot update configuration while reverse proxy is running")
	}
	muxRouter := mux.NewRouter()

	// register longest prefix first, mux picks route in the order registered not by the longest prefix
	// https://github.com/gorilla/mux/issues/453
	keys := []string{}
	for k := range config {
		keys = append(keys, k)
	}
	sort.Strings(keys)

	for k := range keys {
		prefix := keys[len(keys)-1-k]
		proxyURL, err := url.Parse(config[prefix])
		if err != nil {
			return fmt.Errorf("invalid proxy %v: %v", prefix, proxyURL)
		}

		proxy := httputil.NewSingleHostReverseProxy(proxyURL)
		muxRouter.PathPrefix(prefix).HandlerFunc(proxy.ServeHTTP)
		log.Infof("proxy %v: %v", prefix, proxyURL.String())
	}

	log.Infof("REVPROXY AUTHORIZER IS %p", rpr.authorize) // FIXME remove
	muxRouter.Use(logTLSInfo)
	muxRouter.Use(dumpHTTPRequest)
	muxRouter.Use(rpr.authorize)

	// register /stop
	muxRouter.HandleFunc("/stop", rpr.handleStopRequest)
	rpr.muxRouter = muxRouter
	return nil
}

// NewReverseProxyRouter creates a new reverse proxy router
func NewReverseProxyRouter(listenURL string, config map[string]string) (*ReverseProxyRouter, error) {
	rpr := ReverseProxyRouter{
		listenURL: listenURL,
	}
	err := rpr.UpdateConfig(config)
	if err != nil {
		return nil, err
	}
	return &rpr, nil
}

// Start starts the proxy router instance
func (rpr *ReverseProxyRouter) Start(tlsConfig *tls.Config, authz Authorizer) error {
	rpr.Lock()
	defer rpr.Unlock()

	listener, err := net.Listen("tcp", rpr.listenURL)
	if err != nil {
		return fmt.Errorf("Error starting listener. Err: %v", err)
	}
	rpr.tlsConfig = tlsConfig
	rpr.authorizer = authz
	rpr.httpServer = &http.Server{
		Addr:      listener.Addr().String(),
		Handler:   rpr.muxRouter,
		TLSConfig: rpr.tlsConfig,
	}
	if tlsConfig != nil {
		log.Infof("Starting HTTPS server at %s", rpr.httpServer.Addr)
		go rpr.httpServer.ServeTLS(listener, "", "")
	} else {
		log.Infof("Starting HTTP server at %s", rpr.httpServer.Addr)
		go rpr.httpServer.Serve(listener)
	}
	return nil
}

// GetListenURL returns the URL on which the proxy is listening
func (rpr *ReverseProxyRouter) GetListenURL() string {
	rpr.Lock()
	defer rpr.Unlock()
	if rpr.httpServer != nil {
		return rpr.httpServer.Addr
	}
	return ""
}

// Stop stops the http server
func (rpr *ReverseProxyRouter) Stop() error {
	rpr.Lock()
	defer rpr.Unlock()
	if rpr.httpServer != nil {
		// Purge all existing connections and shutdown the HTTP Server
		if err := rpr.httpServer.Close(); err != nil {
			log.Infof("Could not shut Reverse Proxy Router. Err: %v", err)
			return err
		}
		rpr.httpServer = nil
		log.Infof("shutting down reverse proxy")
	}
	return nil
}
