package revproxy

import (
	"context"
	"fmt"
	"net"
	"net/http"
	"net/http/httputil"
	"net/url"
	"sort"
	"time"

	"github.com/gorilla/mux"

	"github.com/pensando/sw/venice/utils/log"
)

// Server holds information about the reverse proxy http server
type Server struct {
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

// NewRevProxyRouter creates a new reverse proxy router
func NewRevProxyRouter(listenURL string, proxyConfig map[string]string) (*Server, error) {
	muxRtr := mux.NewRouter()

	// register longest prefix first, mux picks route in the order registered not by the longest prefix
	// https://github.com/gorilla/mux/issues/453
	keys := []string{}
	for k := range proxyConfig {
		keys = append(keys, k)
	}
	sort.Strings(keys)

	for k := range keys {
		prefix := keys[len(keys)-1-k]
		proxyURL, err := url.Parse(proxyConfig[prefix])
		if err != nil {
			log.Fatalf("invalid proxy %v: %v", prefix, proxyURL)
		}

		proxy := httputil.NewSingleHostReverseProxy(proxyURL)
		muxRtr.PathPrefix(prefix).HandlerFunc(proxy.ServeHTTP)
		log.Infof("proxy %v: %v", listenURL+prefix, proxyURL.String())
	}

	listener, err := net.Listen("tcp", listenURL)
	if err != nil {
		log.Errorf("Error starting listener. Err: %v", err)
		return nil, err
	}

	httpServer := &http.Server{Addr: listener.Addr().String()}
	proxyServer := &Server{httpServer: httpServer}

	// register /stop
	muxRtr.HandleFunc("/stop", proxyServer.handleStopRequest)

	// update http handler
	httpServer.Handler = dumpHTTPRequest(muxRtr)

	go httpServer.Serve(listener)

	return proxyServer, nil
}

func (s *Server) handleStopRequest(res http.ResponseWriter, req *http.Request) {
	go s.Stop()
}

// Stop stops the http server
func (s *Server) Stop() error {
	if s.httpServer != nil {
		ctx, cancelFunc := context.WithTimeout(context.Background(), 3*time.Second)
		defer cancelFunc()
		if err := s.httpServer.Shutdown(ctx); err != nil {
			log.Infof("Could not shut Reverse Proxy Router. Err: %v", err)
			return err
		}
		s.httpServer = nil
		log.Infof("shutting down reverse proxy")
	}
	return nil
}
