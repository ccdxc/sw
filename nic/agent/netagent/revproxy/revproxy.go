package revproxy

import (
	"fmt"
	"net/http"
	"net/http/httputil"
	"net/url"
	"strings"

	"github.com/pensando/sw/venice/globals"
	"github.com/pensando/sw/venice/utils/log"
)

// Get the port for a given request
func getProxyPort(requrl *url.URL) (string, error) {
	sliceP := strings.Split(requrl.Path, "/")
	switch sliceP[1] {
	//NetAgent requests
	case "api":
		return globals.AgentRESTPort, nil
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
		log.Fatalf("Err: %v", err)
	}
	host = "http://localhost:" + host
	serveReverseProxy(host, res, req)
}

// NewRevProxyRouter creates a new reverse proxy router
func NewRevProxyRouter(listenURL string) {
	http.HandleFunc("/", handleRequestAndRedirect)
	go http.ListenAndServe(listenURL, nil)
}
