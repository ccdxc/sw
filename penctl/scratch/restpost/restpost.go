package main

import (
	"bytes"
	"encoding/json"
	"fmt"
	"io/ioutil"
	"net/http"
)

type namespace struct {
	Kind string `json:"kind"`
	Meta struct {
		Name   string `json:"name"`
		Tenant string `json:"tenant"`
	} `json:"meta"`
}

func main() {
	data := namespace{
		Kind: "Namespace",
		Meta: struct {
			Name   string `json:"name"`
			Tenant string `json:"tenant"`
		}{
			Name:   "aao113",
			Tenant: "default",
		},
	}

	payloadBytes, err := json.Marshal(data)
	if err != nil {
		panic(err)
	}
	body := bytes.NewReader(payloadBytes)

	postReq, err := http.NewRequest("POST", "http://127.0.0.1:9007/api/namespaces/", body)
	if err != nil {
		panic(err)
	}
	postReq.Header.Set("Content-Type", "application/json")

	postResp, err := http.DefaultClient.Do(postReq)
	if err != nil {
		panic(err)
	}
	defer postResp.Body.Close()
	fmt.Println("Successfully posted the request to netagent")
	fmt.Println("Status: ", postResp.Status)
	if postResp.Status == "200 OK" {
		fmt.Println("Got back success")
	} else {
		fmt.Println("Got back failure")
	}

	fmt.Println("Doing GET request to netagent")
	getReq, err := http.NewRequest("GET", "http://127.0.0.1:9007/api/namespaces/", nil)
	if err != nil {
		panic(err)
	}
	getReq.Header.Set("Content-Type", "application/json")

	getResp, err := http.DefaultClient.Do(getReq)
	if err != nil {
		panic(err)
	}
	defer getResp.Body.Close()
	fmt.Println("Status: ", getResp.Status)
	fmt.Println("Header: ", getResp.Header)
	bodyBytes, _ := ioutil.ReadAll(getResp.Body)

	var prettyJSON bytes.Buffer
	error := json.Indent(&prettyJSON, bodyBytes, "", "\t")
	if error != nil {
		panic(err)
	}

	fmt.Println("Namespaces:\n", string(prettyJSON.Bytes()))
}
