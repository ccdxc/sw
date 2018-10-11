//-----------------------------------------------------------------------------
// {C} Copyright 2017 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------

package cmd

import (
	"bytes"
	"encoding/json"
	"errors"
	"fmt"
	"io/ioutil"
	"net/http"

	"github.com/ghodss/yaml"
)

func restPost(v interface{}, port string, url string) error {
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

	url = "http://" + naplesIP + ":" + port + "/" + url
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

func restGet(port string, url string) ([]byte, error) {
	if verbose {
		fmt.Println("Doing GET request to naples")
	}
	url = "http://" + naplesIP + ":" + port + "/" + url
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
		fmt.Println("Unable to get response from Naples.")
		if verbose {
			fmt.Println("Err: ", err.Error())
		}
		return nil, err
	}
	defer getResp.Body.Close()
	if verbose {
		fmt.Println("Status: ", getResp.Status)
		fmt.Println("Header: ", getResp.Header)
	}
	bodyBytes, _ := ioutil.ReadAll(getResp.Body)

	if jsonFormat || yamlFormat {
		var prettyJSON bytes.Buffer
		error := json.Indent(&prettyJSON, bodyBytes, "", "\t")
		if error != nil {
			return nil, err
		}
		b, err := yaml.JSONToYAML(bodyBytes)
		if err != nil {
			return nil, err
		}
		if jsonFormat {
			fmt.Println(string(prettyJSON.Bytes()))
		} else if yamlFormat {
			fmt.Println(string(b))
		}
	}
	return bodyBytes, nil
}

func restGetResp(port string, url string) (*http.Response, error) {
	if verbose {
		fmt.Println("Doing GET request to naples")
	}
	url = "http://" + naplesIP + ":" + port + "/" + url
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
		fmt.Println("Unable to get response from Naples.")
		if verbose {
			fmt.Println("Err: ", err.Error())
		}
		return nil, err
	}
	if getResp.StatusCode != 200 {
		if verbose {
			fmt.Println(getResp.Status + " " + url)
		}
		return getResp, errors.New(url + " not found")
	}
	if verbose {
		fmt.Println("Status: ", getResp.Status)
		fmt.Println("Header: ", getResp.Header)
	}
	return getResp, nil
}

func restDelete(port string, url string) ([]byte, error) {
	if verbose {
		fmt.Println("Doing DELETE request to naples")
	}
	url = "http://" + naplesIP + ":" + port + "/" + url
	if verbose {
		fmt.Println("URL: ", url)
	}
	getReq, err := http.NewRequest("DELETE", url, nil)
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

	if jsonFormat || yamlFormat {
		var prettyJSON bytes.Buffer
		error := json.Indent(&prettyJSON, bodyBytes, "", "\t")
		if error != nil {
			return nil, err
		}
		b, err := yaml.JSONToYAML(bodyBytes)
		if err != nil {
			return nil, err
		}
		if jsonFormat {
			fmt.Println(string(prettyJSON.Bytes()))
		} else if yamlFormat {
			fmt.Println(string(b))
		}
	}
	return bodyBytes, nil
}
