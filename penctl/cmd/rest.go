//-----------------------------------------------------------------------------
// {C} Copyright 2017 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------

package cmd

import (
	"bytes"
	"encoding/json"
	"fmt"
	"io/ioutil"
	"net/http"
	"strconv"
)

func restPost(v interface{}, port int, url string) error {
	payloadBytes, err := json.Marshal(v)
	if err != nil {
		return err
	}
	if verbose {
		var prettyJSON bytes.Buffer
		error := json.Indent(&prettyJSON, payloadBytes, "", "\t")
		if error != nil {
			return err
		}

		fmt.Println("Json output:", string(prettyJSON.Bytes()))
	}
	body := bytes.NewReader(payloadBytes)

	url = "http://" + naplesIP + ":" + strconv.Itoa(port) + "/" + url
	if verbose {
		fmt.Println("URL: ", url)
	}
	postReq, err := http.NewRequest("POST", url, body)
	if err != nil {
		return err
	}
	postReq.Header.Set("Content-Type", "application/json")
	postResp, err := http.DefaultClient.Do(postReq)
	if err != nil {
		return err
	}
	defer postResp.Body.Close()
	if verbose {
		fmt.Println("Successfully posted the request")
		fmt.Println("Status:", postResp.Status)
		fmt.Println("StatusCode: ", postResp.StatusCode)
	}
	return nil
}

func restGet(port int, url string) ([]byte, error) {
	if verbose {
		fmt.Println("Doing GET request to netagent")
	}
	url = "http://" + naplesIP + ":" + strconv.Itoa(port) + "/" + url
	if verbose {
		fmt.Println("URL: ", url)
	}
	getReq, err := http.NewRequest("GET", url, nil)
	if err != nil {
		return nil, err
	}
	getReq.Header.Set("Content-Type", "application/json")

	getResp, err := http.DefaultClient.Do(getReq)
	if err != nil {
		return nil, err
	}
	defer getResp.Body.Close()
	if verbose {
		fmt.Println("Status: ", getResp.Status)
		fmt.Println("Header: ", getResp.Header)
	}
	bodyBytes, _ := ioutil.ReadAll(getResp.Body)

	if verbose {
		var prettyJSON bytes.Buffer
		error := json.Indent(&prettyJSON, bodyBytes, "", "\t")
		if error != nil {
			return nil, err
		}
		fmt.Println(string(prettyJSON.Bytes()))
	}
	return bodyBytes, nil
}
