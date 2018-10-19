package common

import (
	"encoding/json"
	"fmt"
	"io/ioutil"
	"net/http"
	"strings"

	"github.com/pensando/sw/venice/utils/log"
)

// RestAPIFunc defines a rest handler
type RestAPIFunc func(r *http.Request) (interface{}, error)

// MakeHTTPHandler is a utility that wraps a rest handler
func MakeHTTPHandler(handlerFunc RestAPIFunc) http.HandlerFunc {
	// Create a closure and return an anonymous function
	return func(w http.ResponseWriter, r *http.Request) {
		// Call the handler
		resp, err := handlerFunc(r)
		if err != nil {
			// Log error
			log.Errorf("Handler for %s %s returned error: %s", r.Method, r.URL, err)

			if resp == nil {
				// Send HTTP response
				http.Error(w, err.Error(), http.StatusInternalServerError)
			} else {
				// Send HTTP response as Json
				content, err1 := json.Marshal(resp)
				if err1 != nil {
					http.Error(w, err.Error(), http.StatusInternalServerError)
					return
				}

				w.WriteHeader(http.StatusInternalServerError)
				w.Write(content)
			}
		} else {
			// Send HTTP response as Json
			content, err := json.Marshal(resp)
			if err != nil {
				http.Error(w, err.Error(), http.StatusInternalServerError)
				return
			}
			w.Write(content)
		}
	}
}

// HTTPPost performs http POST operation
func HTTPPost(url, token string, req interface{}) ([]*http.Cookie, string, error) {
	// Prepend URL
	url = fmt.Sprintf("http://%s", url)

	client := &http.Client{}
	// Convert the req to json
	jsonStr, err := json.Marshal(req)
	if err != nil {
		log.Errorf("Error converting request data(%#v) to Json. Err: %v", req, err)
		return nil, "", err
	}

	request, err := http.NewRequest(http.MethodPost, url, strings.NewReader(string(jsonStr)))
	if err != nil {
		log.Errorf("Error during http POST. Err: %v", err)
		return nil, "", err
	}
	request.Header.Set("Content-Type", "application/json")

	if len(token) > 0 {
		bearer := fmt.Sprintf("Bearer %s", token)
		request.Header.Set("Authorization", bearer)
	}

	log.Info("Common | HTTP POST | Posting %v to %v", string(jsonStr), url)
	log.Infof("HTTP HEADERS: %v", request.Header)
	res, err := client.Do(request)
	if err != nil {
		log.Errorf("Error during http POST. Err: %v", err)
		return nil, "", err
	}

	// Read the entire response
	body, err := ioutil.ReadAll(res.Body)
	if err != nil {
		log.Errorf("Error during ioutil readall. Err: %v", err)
		return res.Cookies(), "", err
	}
	defer res.Body.Close()

	response := string(body)

	if res.StatusCode != http.StatusOK {
		log.Errorf("HTTP error response. Status: %s, StatusCode: %d", res.Status, res.StatusCode)
		return res.Cookies(), response, fmt.Errorf("HTTP error response. Status: %s, StatusCode: %d", res.Status, res.StatusCode)
	}
	return res.Cookies(), response, nil
}

// HTTPGet fetches json object from an http get request
func HTTPGet(url, token string, req interface{}) (string, error) {

	url = fmt.Sprintf("http://%s", url)

	client := &http.Client{}
	// Convert the req to json
	jsonStr, err := json.Marshal(req)
	if err != nil {
		log.Errorf("Error converting request data(%#v) to Json. Err: %v", req, err)
		return "", err
	}

	request, err := http.NewRequest(http.MethodGet, url, strings.NewReader(string(jsonStr)))
	if err != nil {
		log.Errorf("Error during http POST. Err: %v", err)
		return "", err
	}
	request.Header.Set("Content-Type", "application/json")

	if len(token) > 0 {
		bearer := fmt.Sprintf("Bearer %s", token)
		request.Header.Set("Authorization", bearer)
	}

	res, err := client.Do(request)
	if err != nil {
		log.Errorf("Error during http POST. Err: %v", err)
		return "", err
	}

	// Read the entire response
	body, err := ioutil.ReadAll(res.Body)
	if err != nil {
		log.Errorf("Error during ioutil readall. Err: %v", err)
		return "", err
	}
	defer res.Body.Close()

	response := string(body)

	if res.StatusCode != http.StatusOK {
		log.Errorf("HTTP error response. Status: %s, StatusCode: %d", res.Status, res.StatusCode)
		return response, fmt.Errorf("HTTP error response. Status: %s, StatusCode: %d", res.Status, res.StatusCode)
	}
	return response, nil
}

// HTTPPut provides wrapper for http PUT operations.
func HTTPPut(url, token string, req interface{}) (string, error) {
	// Prepend URL
	url = fmt.Sprintf("http://%s", url)

	client := &http.Client{}
	// Convert the req to json
	jsonStr, err := json.Marshal(req)
	if err != nil {
		log.Errorf("Error converting request data(%#v) to Json. Err: %v", req, err)
		return "", err
	}

	request, err := http.NewRequest(http.MethodPut, url, strings.NewReader(string(jsonStr)))
	if err != nil {
		log.Errorf("Error during http PUT. Err: %v", err)
	}

	if len(token) > 0 {
		bearer := fmt.Sprintf("Bearer %s", token)
		request.Header.Set("Authorization", bearer)
	}

	request.Header.Set("Content-Type", "application/json")
	res, err := client.Do(request)

	if err != nil {
		log.Errorf("Error during http PUT. Err: %v", err)
		return "", err
	}

	// Read the entire response
	body, err := ioutil.ReadAll(res.Body)
	if err != nil {
		log.Errorf("Error during ioutil readall. Err: %v", err)
		return "", err
	}
	defer res.Body.Close()

	response := string(body)

	if res.StatusCode != http.StatusOK {
		log.Errorf("HTTP error response. Status: %s, StatusCode: %d", res.Status, res.StatusCode)
		return response, fmt.Errorf("HTTP error response. Status: %s, StatusCode: %d", res.Status, res.StatusCode)
	}
	return response, nil
}

// HTTPDelete provides wrapper for http DELETE operations.
func HTTPDelete(url, token string, req interface{}, resp interface{}) (string, error) {
	// Prepend URL
	url = fmt.Sprintf("http://%s", url)

	client := &http.Client{}
	jsonStr := []byte{}
	var err error
	if req != nil {
		// Convert the req to json
		jsonStr, err = json.Marshal(req)
		if err != nil {
			log.Errorf("Error converting request data(%#v) to Json. Err: %v", req, err)
			return "", err
		}
	}

	request, err := http.NewRequest(http.MethodDelete, url, strings.NewReader(string(jsonStr)))
	if err != nil {
		log.Errorf("Error during http DELETE. Err: %v", err)
	}
	if len(token) > 0 {
		bearer := fmt.Sprintf("Bearer %s", token)
		request.Header.Set("Authorization", bearer)
	}

	request.Header.Set("Content-Type", "application/json")
	res, err := client.Do(request)

	if err != nil {
		log.Errorf("Error during http DELETE. Err: %v", err)
		return "", err
	}

	// Read the entire response
	body, err := ioutil.ReadAll(res.Body)
	if err != nil {
		log.Errorf("Error during ioutil readall. Err: %v", err)
		return "", err
	}
	defer res.Body.Close()

	response := string(body)

	if res.StatusCode != http.StatusOK {
		log.Errorf("HTTP error response. Status: %s, StatusCode: %d", res.Status, res.StatusCode)
		return response, fmt.Errorf("HTTP error response. Status: %s, StatusCode: %d", res.Status, res.StatusCode)
	}
	return response, nil
}

//WaitForSvcUp performs a get on the URL and waits till the service is up
//func WaitForSvcUp(url string, timeout time.Duration, code int) (err error) {
//	up := make(chan bool, 1)
//	ticker := time.NewTicker(time.Second * 10)
//	done := time.After(timeout)
//	for {
//		select {
//		case <-ticker.C:
//			log.Infof("Common | DEBUG | WaitForSvcUp. Waiting to connect to %v", url)
//			err := HTTPGet(url)
//			log.Infof("Common | DEBUG | WaitForSvcUp. Waiting for %v, got %v", code, statusCode)
//			if err != nil {
//				up <- true
//			}
//		case <-up:
//			log.Info("Service is up")
//			return nil
//		case <-done:
//			log.Errorf("Common | DEBUG| WaitForSvcUp Could not connect to the service at %v after %v seconds", url, timeout.Seconds())
//			return fmt.Errorf("could not connect to the service at %v after %v seconds", url, timeout.Seconds())
//		}
//	}
//
//}
