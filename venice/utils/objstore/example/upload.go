package main

import (
	"bytes"
	"context"
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

	"github.com/pensando/sw/api/generated/auth"
	loginctx "github.com/pensando/sw/api/login/context"
	"github.com/pensando/sw/venice/globals"
	"github.com/pensando/sw/venice/utils/authn/testutils"
)

const (
	testUser     = "test"
	testPassword = "Pensando0$"
)

// Creates a new file upload http request with optional extra params
func uploadFile(srv, uri string, params map[string]string, path string) (*http.Request, error) {
	file, err := os.Open(path)
	if err != nil {
		return nil, err
	}
	defer file.Close()
	fmt.Printf("Opened file [%v]\n", path)

	userCred := &auth.PasswordCredential{
		Username: testUser,
		Password: testPassword,
		Tenant:   globals.DefaultTenant,
	}
	ctx, err := testutils.NewLoggedInContext(context.Background(), srv, userCred)
	if err != nil {
		return nil, fmt.Errorf("could not login (%s)", err)
	}
	authzHeader, ok := loginctx.AuthzHeaderFromContext(ctx)
	if !ok {
		return nil, fmt.Errorf("no authorizaton header in context")
	}

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
	req, err := http.NewRequest("POST", srv+uri, body)
	req.Header.Set("Content-Type", writer.FormDataContentType())
	req.Header.Set("Authorization", authzHeader)
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

	sURI := strings.TrimSuffix(*URI, "/")
	reqURI := "/objstore/v1/uploads/images/"
	metadata := map[string]string{
		"Version":     "v1.3.2",
		"Environment": "production",
		"Description": "image with fixes",
		"ReleaseDate": "May2018",
	}
	request, err := uploadFile(sURI, reqURI, metadata, *file)
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
