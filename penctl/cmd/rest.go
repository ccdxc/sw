//-----------------------------------------------------------------------------
// {C} Copyright 2017 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------

package cmd

import (
	"bytes"
	"crypto/tls"
	"encoding/json"
	"errors"
	"fmt"
	"io"
	"io/ioutil"
	"mime/multipart"
	"net/http"
	"net/http/httputil"
	"os"
	"strings"

	"github.com/ghodss/yaml"

	"github.com/pensando/sw/penctl/http2curl"
)

// NaplesConfigResp is response to NaplesConfig request nmd.Naples
type NaplesConfigResp struct {
	ErrorMsg string
}

var penHTTPClient = &http.Client{}

func init() {
	// always use HTTPS by default, even without a client certificate
	// if compat11 flag is set, the URL will start with http:// and TLS config is ignored
	tr := &http.Transport{
		TLSClientConfig: &tls.Config{
			InsecureSkipVerify: true, // do not check agent's certificate
		},
		MaxIdleConnsPerHost: 5,
	}
	penHTTPClient = &http.Client{Transport: tr}
}

func isJSONString(s string) bool {
	var js interface{}
	return json.Unmarshal([]byte(s), &js) == nil
}

const (
	kiB         = 1024 // kilobyte
	twoKB int64 = 2 * kiB

	httpsSignature = "\\x15\\x03\\x01\\x00\\x02\\x02"
)

func printHTTPReq(r *http.Request) {
	if verbose {
		fmt.Println("==== HTTP Request Start ====")
		command, _ := http2curl.GetCurlCommand(r)
		fmt.Println(command)
		if r.ContentLength > twoKB {
			fmt.Printf("RevProxy: ContentLength: %d\n", r.ContentLength)
			dump, err := httputil.DumpRequest(r, false)
			if err != nil {
				fmt.Printf("%s\n", fmt.Sprint(err))
			} else {
				fmt.Printf("%s %v\n", dump, r.Header)
			}
		} else {
			dump, err := httputil.DumpRequest(r, true)
			if err != nil {
				fmt.Printf("%s\n", fmt.Sprint(err))
			} else {
				fmt.Printf("%s\n", dump)
			}
		}
		fmt.Println("==== HTTP Request End ====")
	}
}

func printHTTPResp(resp *http.Response) {
	if verbose {
		fmt.Println("==== HTTP Response Start ====")
		dump, _ := httputil.DumpResponse(resp, false)
		fmt.Printf("%s\n", string(dump))
		fmt.Println("==== HTTP Response End ====")
	}
}

func restPostForm(url string, values map[string]io.Reader) ([]byte, error) {
	if ur, err := getNaplesURL(); err == nil {
		url = ur + url
	} else {
		return nil, err
	}

	var b bytes.Buffer
	w := multipart.NewWriter(&b)
	for key, r := range values {
		var fw io.Writer
		var err error
		if x, ok := r.(io.Closer); ok {
			defer x.Close()
		}
		if x, ok := r.(*os.File); ok {
			if fw, err = w.CreateFormFile(key, x.Name()); err != nil {
				return nil, err
			}
		} else {
			if fw, err = w.CreateFormField(key); err != nil {
				return nil, err
			}
		}
		if _, err = io.Copy(fw, r); err != nil {
			return nil, err
		}

	}
	w.Close()

	req, err := http.NewRequest("POST", url, &b)
	if err != nil {
		return nil, err
	}
	req.Header.Set("Content-Type", w.FormDataContentType())

	printHTTPReq(req)
	res, err := penHTTPClient.Do(req)
	if err != nil {
		fmt.Println("Unable to get response from Distributed Service Card.")
		if strings.Contains(err.Error(), httpsSignature) || strings.Contains(err.Error(), "transport connection broken") {
			err = fmt.Errorf("Distributed Service Card is part of a cluster, authentication token required")
		}
		return nil, err
	}
	defer res.Body.Close()
	printHTTPResp(res)
	if verbose {
		fmt.Println("Status: ", res.Status)
		fmt.Println("Header: ", res.Header)
	}
	bodyBytes, _ := ioutil.ReadAll(res.Body)

	return bodyBytes, nil
}

func restPost(v interface{}, url string) (string, error) {
	payloadBytes, err := json.Marshal(v)
	if err != nil {
		return "", err
	}
	if verbose {
		var prettyJSON bytes.Buffer
		error := json.Indent(&prettyJSON, payloadBytes, "", "\t")
		if error != nil {
			return "", err
		}

		fmt.Println("Json output:", string(prettyJSON.Bytes()))
	}
	body := bytes.NewReader(payloadBytes)

	if ur, err := getNaplesURL(); err == nil {
		url = ur + url
	} else {
		return "", err
	}
	if verbose {
		fmt.Println("URL: ", url)
	}
	postReq, err := http.NewRequest("POST", url, body)
	if err != nil {
		return "", err
	}
	postReq.Header.Set("Content-Type", "application/json")
	printHTTPReq(postReq)
	postResp, err := penHTTPClient.Do(postReq)
	if err != nil {
		return "", err
	}
	defer postResp.Body.Close()
	printHTTPResp(postResp)
	bodyBytes, _ := ioutil.ReadAll(postResp.Body)
	if postResp.StatusCode != 200 {
		if verbose {
			fmt.Println(postResp.Status + " " + url)
			fmt.Println(postResp.Body)
		}
		jsonResp := NaplesConfigResp{}
		json.Unmarshal(bodyBytes[:len(bodyBytes)-1], &jsonResp)
		return string(bodyBytes), errors.New(jsonResp.ErrorMsg)
	}
	if verbose {
		fmt.Println("Successfully posted the request")
		fmt.Println("Status:", postResp.Status)
		fmt.Println("StatusCode: ", postResp.StatusCode)
	}
	return string(bodyBytes), nil
}

func restGet(url string) ([]byte, error) {
	if verbose {
		fmt.Println("Doing GET request to naples")
	}
	if ur, err := getNaplesURL(); err == nil {
		url = ur + url
	} else {
		return nil, err
	}
	if verbose {
		fmt.Println("URL: ", url)
	}
	getReq, err := http.NewRequest("GET", url, nil)
	if err != nil {
		return nil, err
	}
	getReq.Header.Set("Content-Type", "application/json")

	printHTTPReq(getReq)
	getResp, err := penHTTPClient.Do(getReq)
	if err != nil {
		fmt.Println("Unable to get response from Distributed Service Card.")
		if strings.Contains(err.Error(), httpsSignature) {
			err = fmt.Errorf("Distributed Service Card is part of a cluster, authentication token required")
		}
		if verbose {
			fmt.Println("Err: ", err.Error())
		}
		return nil, err
	}
	defer getResp.Body.Close()
	printHTTPResp(getResp)
	if getResp.StatusCode != 200 {
		if verbose {
			fmt.Println(getResp.Status + " " + url)
		}
		return nil, errors.New(url + " not found. Please make sure dsc is reachable and provide token file if dsc is part of cluster.")
	}
	if verbose {
		fmt.Println("Status: ", getResp.Status)
		fmt.Println("Header: ", getResp.Header)
	}
	bodyBytes, _ := ioutil.ReadAll(getResp.Body)

	if isJSONString(string(bodyBytes)) && (jsonFormat || yamlFormat) {
		var prettyJSON bytes.Buffer
		error := json.Indent(&prettyJSON, bodyBytes, "", "\t")
		if error != nil {
			return nil, err
		}
		if bodyBytes == nil || strings.Compare(string(bodyBytes), "null\n") == 0 {
			return nil, nil
		}
		if jsonFormat {
			fmt.Println(string(prettyJSON.Bytes()))
		} else if yamlFormat {
			b, err := yaml.JSONToYAML(bodyBytes)
			if err != nil {
				return nil, err
			}
			fmt.Println(string(b))
		}
	}
	return bodyBytes, nil
}

func restGetWithBody(v interface{}, url string) ([]byte, error) {
	payloadBytes, err := json.Marshal(v)
	if err != nil {
		return nil, err
	}
	body := bytes.NewReader(payloadBytes)
	if ur, err := getNaplesURL(); err == nil {
		url = ur + url
	} else {
		return nil, err
	}
	if verbose {
		fmt.Println("URL: ", url)
	}
	getReq, err := http.NewRequest("GET", url, body)
	if err != nil {
		return nil, err
	}
	getReq.Header.Set("Content-Type", "application/json")

	printHTTPReq(getReq)
	getResp, err := penHTTPClient.Do(getReq)
	if err != nil {
		fmt.Println("Unable to get response from Distributed Service Card.")
		if strings.Contains(err.Error(), httpsSignature) {
			err = fmt.Errorf("Distributed Service Card is part of a cluster, authentication token required")
		}
		if verbose {
			fmt.Println("Err: ", err.Error())
		}
		return nil, err
	}
	defer getResp.Body.Close()
	printHTTPResp(getResp)
	if verbose {
		fmt.Println("Status: ", getResp.Status)
		fmt.Println("Header: ", getResp.Header)
	}
	bodyBytes, _ := ioutil.ReadAll(getResp.Body)
	if getResp.StatusCode != 200 {
		if verbose {
			fmt.Println(getResp.Status + " " + url)
		}
		return nil, errors.New(url + " not found. Please make sure dsc is reachable and provide token file if dsc is part of cluster.")
	}

	if isJSONString(string(bodyBytes)) && (jsonFormat || yamlFormat) {
		var prettyJSON bytes.Buffer
		error := json.Indent(&prettyJSON, bodyBytes, "", "\t")
		if error != nil {
			return nil, err
		}
		if bodyBytes == nil || strings.Compare(string(bodyBytes), "null\n") == 0 {
			return nil, nil
		}
		if jsonFormat {
			fmt.Println(string(prettyJSON.Bytes()))
		} else if yamlFormat {
			b, err := yaml.JSONToYAML(bodyBytes)
			if err != nil {
				return nil, err
			}
			fmt.Println(string(b))
		}
	}
	return bodyBytes, nil
}

func restGetResp(url string) (*http.Response, error) {
	if verbose {
		fmt.Println("Doing GET request to naples")
	}
	if ur, err := getNaplesURL(); err == nil {
		url = ur + url
	} else {
		return nil, err
	}
	if verbose {
		fmt.Println("URL: ", url)
	}
	getReq, err := http.NewRequest("GET", url, nil)
	if err != nil {
		return nil, err
	}
	getReq.Header.Set("Content-Type", "application/json")

	printHTTPReq(getReq)
	getResp, err := penHTTPClient.Do(getReq)
	if err != nil {
		fmt.Println("Unable to get response from Distributed Service Card.")
		if strings.Contains(err.Error(), httpsSignature) {
			err = fmt.Errorf("Distributed Service Card is part of a cluster, authentication token required")
		}
		if verbose {
			fmt.Println("Err: ", err.Error())
		}
		return nil, err
	}
	printHTTPResp(getResp)
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

func restDelete(url string) ([]byte, error) {
	if verbose {
		fmt.Println("Doing DELETE request to naples")
	}
	if ur, err := getNaplesURL(); err == nil {
		url = ur + url
	} else {
		return nil, err
	}
	if verbose {
		fmt.Println("URL: ", url)
	}
	getReq, err := http.NewRequest("DELETE", url, nil)
	if err != nil {
		return nil, err
	}
	getReq.Header.Set("Content-Type", "application/json")

	printHTTPReq(getReq)
	getResp, err := penHTTPClient.Do(getReq)
	if err != nil {
		if strings.Contains(err.Error(), httpsSignature) {
			err = fmt.Errorf("Distributed Service Card is part of a cluster, authentication token required")
		}
		return nil, err
	}
	defer getResp.Body.Close()
	printHTTPResp(getResp)
	if verbose {
		fmt.Println("Status: ", getResp.Status)
		fmt.Println("Header: ", getResp.Header)
	}
	bodyBytes, _ := ioutil.ReadAll(getResp.Body)

	if isJSONString(string(bodyBytes)) && (jsonFormat || yamlFormat) {
		var prettyJSON bytes.Buffer
		error := json.Indent(&prettyJSON, bodyBytes, "", "\t")
		if error != nil {
			return nil, err
		}
		if bodyBytes == nil || strings.Compare(string(bodyBytes), "null\n") == 0 {
			return nil, nil
		}
		if jsonFormat {
			fmt.Println(string(prettyJSON.Bytes()))
		} else if yamlFormat {
			b, err := yaml.JSONToYAML(bodyBytes)
			if err != nil {
				return nil, err
			}
			fmt.Println(string(b))
		}
	}
	return bodyBytes, nil
}

func restPut(v interface{}, url string) error {
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

	if ur, err := getNaplesURL(); err == nil {
		url = ur + url
	} else {
		return err
	}
	if verbose {
		fmt.Println("URL: ", url)
	}
	postReq, err := http.NewRequest("PUT", url, body)
	if err != nil {
		return err
	}
	postReq.Header.Set("Content-Type", "application/json")
	printHTTPReq(postReq)
	postResp, err := penHTTPClient.Do(postReq)
	if err != nil {
		return err
	}
	defer postResp.Body.Close()
	printHTTPResp(postResp)
	if postResp.StatusCode != 200 {
		if verbose {
			fmt.Println(postResp.Status + " " + url)
			fmt.Println(postResp.Body)
		}
		bodyBytes, _ := ioutil.ReadAll(postResp.Body)
		jsonResp := NaplesConfigResp{}
		json.Unmarshal(bodyBytes[:len(bodyBytes)-1], &jsonResp)
		return errors.New(jsonResp.ErrorMsg)
	}
	if verbose {
		fmt.Println("Successfully posted the request")
		fmt.Println("Status:", postResp.Status)
		fmt.Println("StatusCode: ", postResp.StatusCode)
	}
	return nil
}
