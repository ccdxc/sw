package main

import (
	"bytes"
	"crypto/tls"
	"flag"
	"fmt"
	"io"
	"log"
	"mime/multipart"
	"net/http"
	"os"
	"path/filepath"
	"strings"
)

// Creates a new file upload http request with optional extra params
func uploadFile(uri string, params map[string]string, path string) (*http.Request, error) {
	file, err := os.Open(path)
	if err != nil {
		return nil, err
	}
	defer file.Close()
	fmt.Printf("Opened file [%v]\n", path)

	body := &bytes.Buffer{}
	writer := multipart.NewWriter(body)
	part, err := writer.CreateFormFile("file", filepath.Base(path))
	if err != nil {
		return nil, err
	}

	fmt.Printf("uploading contents\n")
	_, err = io.Copy(part, file)

	for key, val := range params {
		_ = writer.WriteField(key, val)
	}
	err = writer.Close()
	if err != nil {
		return nil, err
	}
	req, err := http.NewRequest("POST", uri, body)
	req.Header.Set("Content-Type", writer.FormDataContentType())
	return req, err
}

func main() {
	URI := flag.String("uri", "https://localhost:19000", "server URI")
	file := flag.String("file", "", "file to upload")
	flag.Parse()

	if *file == "" {
		fmt.Printf("file name not provided")
		os.Exit(-1)
	}

	reqURI := strings.TrimSuffix(*URI, "/")
	reqURI = reqURI + "/objstore/v1/uploads/images/"
	metadata := map[string]string{
		"Version":     "v1.3.2",
		"Environment": "production",
		"Description": "image with fixes",
		"ReleaseDate": "May2018",
	}
	request, err := uploadFile(reqURI, metadata, *file)
	if err != nil {
		log.Fatal(err)
	}
	transport := &http.Transport{
		TLSClientConfig: &tls.Config{InsecureSkipVerify: true},
	}
	fmt.Printf("sending request\n")
	client := &http.Client{Transport: transport}
	resp, err := client.Do(request)
	if err != nil {
		log.Fatal(err)
	} else {
		body := &bytes.Buffer{}
		_, err := body.ReadFrom(resp.Body)
		if err != nil {
			log.Fatal(err)
		}
		resp.Body.Close()
		fmt.Println(resp.StatusCode)
		fmt.Println(resp.Header)
		fmt.Println(body)
	}
}
