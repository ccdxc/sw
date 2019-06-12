package revproxy

import (
	"context"
	"crypto/tls"
	"fmt"
	"net"
	"net/http"
	"net/http/httputil"
	"net/url"
	"sort"
	"sync"
	"time"

	"github.com/gorilla/mux"

	"github.com/pensando/sw/venice/utils/log"
)

// ReverseProxyRouter is a HTTP/HTTPs reverse proxy based on gorilla mux
type ReverseProxyRouter struct {
	sync.Mutex
	listenURL  string
	muxRouter  *mux.Router
	tlsConfig  *tls.Config
	httpServer *http.Server // HTTP server
}

const (
	kiB = 1024 // kilobyte
)
const twoKB int64 = 2 * kiB

func dumpHTTPRequest(next http.Handler) http.Handler {
	return http.HandlerFunc(func(w http.ResponseWriter, r *http.Request) {
		if r.ContentLength > twoKB {
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

func getTLSInfo(next http.Handler) http.Handler {
	return http.HandlerFunc(func(w http.ResponseWriter, r *http.Request) {
		// Do stuff here
		log.Infof("*** GORILLA REQUEST: %+v", r)
		// Call the next handler, which can be another middleware in the chain, or the final handler.
		if r.TLS != nil {
			tlsInfo := r.TLS
			log.Infof("*** Found TLS context: %+v", tlsInfo)
			if tlsInfo.VerifiedChains != nil && len(tlsInfo.VerifiedChains) >= 0 {
				clientCert := tlsInfo.VerifiedChains[0][0]
				log.Infof("*** Found valid certificate attributes, Subj: %+v, SANs: %+v", clientCert.Subject, clientCert.DNSNames)
			} else {
				log.Infof("*** No valid certificate")
			}
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

	muxRouter.Use(getTLSInfo)
	muxRouter.Use(dumpHTTPRequest)

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
func (rpr *ReverseProxyRouter) Start(tlsConfig *tls.Config) error {
	rpr.Lock()
	defer rpr.Unlock()

	listener, err := net.Listen("tcp", rpr.listenURL)
	if err != nil {
		return fmt.Errorf("Error starting listener. Err: %v", err)
	}
	rpr.tlsConfig = tlsConfig
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
		ctx, cancelFunc := context.WithTimeout(context.Background(), 3*time.Second)
		defer cancelFunc()
		if err := rpr.httpServer.Shutdown(ctx); err != nil {
			log.Infof("Could not shut Reverse Proxy Router. Err: %v", err)
			return err
		}
		rpr.httpServer = nil
		log.Infof("shutting down reverse proxy")
	}
	return nil
}
