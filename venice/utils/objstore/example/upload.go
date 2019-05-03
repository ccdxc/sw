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

func main() {
	URI := flag.String("uri", "https://localhost:19000", "server URI")
	fileName := flag.String("file", "", "file to upload")
	flag.Parse()

	if *fileName == "" {
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
	path := *fileName

	file, err := os.Open(path)
	if err != nil {
		fmt.Printf("Failed to open file (%s)", err)
		os.Exit(1)
	}
	defer file.Close()
	fmt.Printf("Opened file [%v]\n", path)

	userCred := &auth.PasswordCredential{
		Username: testUser,
		Password: testPassword,
		Tenant:   globals.DefaultTenant,
	}
	ctx, err := testutils.NewLoggedInContext(context.Background(), sURI, userCred)
	if err != nil {
		log.Fatalf("could not login (%s)", err)
	}
	authzHeader, ok := loginctx.AuthzHeaderFromContext(ctx)
	if !ok {
		log.Fatalf("no authorizaton header in context")
	}

	reader, writer := io.Pipe()
	mpWriter := multipart.NewWriter(writer)

	req, err := http.NewRequest("POST", sURI+reqURI, reader)
	if err != nil {
		log.Fatal(err)
	}
	req.Header.Set("Content-Type", mpWriter.FormDataContentType())
	req.Header.Set("Authorization", authzHeader)

	go func() {
		defer writer.Close()
		defer mpWriter.Close()
		// close(ch)

		part, err := mpWriter.CreateFormFile("file", filepath.Base(path))
		if err != nil {
			log.Fatalf("failed to create form file (%s)", err)
		}
		fmt.Printf("uploading contents\n")
		_, err = io.Copy(part, file)

		for key, val := range metadata {
			_ = mpWriter.WriteField(key, val)
		}
	}()
	transport := &http.Transport{
		TLSClientConfig: &tls.Config{InsecureSkipVerify: true},
	}
	fmt.Printf("sending request\n")
	client := &http.Client{Transport: transport}
	resp, err := client.Do(req)
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
