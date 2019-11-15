package cmd

import (
	"bytes"
	"crypto/tls"
	"fmt"
	"io"
	"log"
	"mime/multipart"
	"net/http"
	"os"
	"path/filepath"
	"strings"

	"github.com/spf13/cobra"
)

var uploadCmd = &cobra.Command{
	Use:   "upload",
	Short: "upload to venice <path>",
	Run:   upload,
}

func init() {
	rootCmd.AddCommand(uploadCmd)
}

func upload(cmd *cobra.Command, args []string) {
	if len(args) != 1 {
		errorExit(nil, "file name to upload required")
	}

	sURI := strings.TrimSuffix(uri, "/")
	reqURI := fmt.Sprintf("/objstore/v1/uploads/%s/", bucket)
	metadata := map[string]string{
		"Version":     "v1.3.2",
		"Environment": "production",
		"Description": "image with fixes",
		"ReleaseDate": "May2018",
	}
	path := args[0]

	file, err := os.Open(path)
	if err != nil {
		errorExit(err, "Failed to open file (%s)", err)
	}
	defer file.Close()
	fmt.Printf("Opened file [%v]\n", path)

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
