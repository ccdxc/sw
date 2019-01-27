package revproxy

import (
	"context"
	"fmt"
	"net/http"
	"net/http/httputil"
	"net/url"
	"strings"
	"time"

	"github.com/pensando/sw/venice/utils/log"
)

var revProxyMap map[string]string

// Get the port for a given request
func getProxyPort(requrl *url.URL) (string, error) {
	sliceP := strings.Split(requrl.Path, "/")
	log.Infof("getProxyPort: %s", sliceP[1])
	if port, ok := revProxyMap[sliceP[1]]; ok == true {
		return port, nil
	}
	return "", fmt.Errorf("Unable to get a matching endpoint. Url: %s Path: %s", requrl.String(), sliceP[1:])
}

// Serve a reverse proxy for a given url
func serveReverseProxy(target string, res http.ResponseWriter, req *http.Request) {
	// parse the url
	url, _ := url.Parse(target)

	// create the reverse proxy
	httputil.NewSingleHostReverseProxy(url).ServeHTTP(res, req)
}

// Given a request send it to the appropriate destination
func handleRequestAndRedirect(res http.ResponseWriter, req *http.Request) {
	host, err := getProxyPort(req.URL)
	if err != nil {
		fmt.Fprintf(res, "%+v", err.Error())
		log.Infof("Err: %v", err)
		return
	}
	host = "http://localhost:" + host
	serveReverseProxy(host, res, req)
}

func initRevProxyMap() {
	if revProxyMap == nil {
		revProxyMap = make(map[string]string)
	}
}

// AddRevProxyDest will add prefix+port to the revProxyMap map for lookups
func AddRevProxyDest(prefix string, port string) {
	initRevProxyMap()
	revProxyMap[prefix] = port
	log.Infof("AddRevProxyDest: %s:%s", prefix, port)
}

// Server holds information about the reverse proxy http server
type Server struct {
	listenURL  string       // URL where http server is listening
	httpServer *http.Server // HTTP server
}

var selfServer *Server

// NewRevProxyRouter creates a new reverse proxy router
func NewRevProxyRouter(listenURL string) (*Server, error) {
	initRevProxyMap()
	revProxyRouter := Server{
		listenURL: listenURL,
	}
	log.Infof("listenURL: %s", listenURL)
	revProxyRouter.httpServer = &http.Server{Addr: listenURL}

	http.HandleFunc("/", handleRequestAndRedirect)
	http.HandleFunc("/revproxy/stop", handleStopRequest)
	revProxyRouter.Start()
	selfServer = &revProxyRouter
	return &revProxyRouter, nil
}

// Start starts the http server
func (s *Server) Start() error {
	if s.httpServer != nil {
		go s.httpServer.ListenAndServe()
	}
	return nil
}

func handleStopRequest(res http.ResponseWriter, req *http.Request) {
	selfServer.Stop()
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
	}
	return nil
}
