package main

import (
	"io/ioutil"
	"log"
	"net/http"
	"time"
)

var (
	httpClient *http.Client
)

const (
	maxIdleConnections = 20
	requestTimeout     = 60
)

// init HTTPClient
func init() {
	httpClient = createHTTPClient()
}

// createHTTPClient for connection re-use
func createHTTPClient() *http.Client {
	client := &http.Client{
		Transport: &http.Transport{
			MaxIdleConnsPerHost: maxIdleConnections,
			DisableKeepAlives:   true,
		},
		Timeout: time.Duration(requestTimeout) * time.Second,
	}

	return client
}

func main() {
	endPoint := "http://64.0.0.1:9080/index.html"

	req, err := http.NewRequest("GET", endPoint, nil)
	if err != nil {
		log.Fatalf("Error Occured. %+v", err)
	}

	// use httpClient to send request
	response, err := httpClient.Do(req)
	if err != nil {
		log.Fatalf("Error sending request to API endpoint. %+v", err)
		panic(err)
	} else {
		// Close the connection to reuse it
		defer response.Body.Close()

		// Let's check if the work actually is done
		// We have seen inconsistencies even when we get 200 OK response
		body, err := ioutil.ReadAll(response.Body)
		if err != nil {
			log.Fatalf("Couldn't parse response body. %+v", err)
			panic(err)
		}

		log.Println("Response Body:", string(body))
	}
}
