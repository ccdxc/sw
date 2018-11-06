package gzipserver

import (
	"bytes"
	"compress/gzip"
	"io/ioutil"
	"net/http"
	"net/http/httptest"
	"testing"
)

const (
	plainTextContentType = "text/plain; charset=utf-8"
	htmlContentType      = "text/html; charset=utf-8"
)

func TestServeHTTP(t *testing.T) {
	testCases := []struct {
		desc            string
		file            string
		encodingHeader  string
		hasGzip         bool
		isPresent       bool
		contentEncoding string
		content         string
		contentType     string
	}{
		{
			desc:            "Client accepts gzip and gzip version is present",
			file:            "filegz.txt",
			encodingHeader:  "gzip",
			hasGzip:         true,
			isPresent:       true,
			contentEncoding: gzipEncoding,
			content:         "this file was compressed\n",
			contentType:     plainTextContentType,
		},
		{
			desc:            "Client accepts */* and gzip version is present",
			file:            "filegz.txt",
			encodingHeader:  "*/*",
			hasGzip:         true,
			isPresent:       true,
			contentEncoding: gzipEncoding,
			content:         "this file was compressed\n",
			contentType:     plainTextContentType,
		},
		{
			desc:            "Client accepts * and gzip version is present",
			file:            "filegz.txt",
			encodingHeader:  "*",
			hasGzip:         true,
			isPresent:       true,
			contentEncoding: gzipEncoding,
			content:         "this file was compressed\n",
			contentType:     plainTextContentType,
		},
		{
			desc:            "Client doesn't accept gzip q=0.0 so client decompresses before sending",
			file:            "filegz.txt",
			encodingHeader:  "gzip;q=0.0",
			hasGzip:         true,
			isPresent:       true,
			contentEncoding: "",
			content:         "this file was compressed\n",
			contentType:     plainTextContentType,
		},
		{
			desc:            "Client doesn't accept gzip so client decompresses before sending",
			file:            "filegz.txt",
			encodingHeader:  "text/html,",
			hasGzip:         true,
			isPresent:       true,
			contentEncoding: "",
			content:         "this file was compressed\n",
			contentType:     plainTextContentType,
		},
		{
			desc:            "Client accepts gzip;q=0.8 but file is only stored on the server in plain text",
			file:            "plainfile.txt",
			encodingHeader:  "gzip;q=0.8",
			hasGzip:         false,
			isPresent:       true,
			contentEncoding: "",
			content:         "this file is plain text\n",
			contentType:     plainTextContentType,
		},
		{
			desc:            "Client doesn't accept gzip and file is only stored on the server in plain text",
			file:            "plainfile.txt",
			encodingHeader:  "text/html,",
			hasGzip:         false,
			isPresent:       true,
			contentEncoding: "",
			content:         "this file is plain text\n",
			contentType:     plainTextContentType,
		},
		{
			desc:            "Client requests a missing file",
			file:            "invalidfile.txt",
			encodingHeader:  "gzip",
			hasGzip:         false,
			isPresent:       false,
			contentEncoding: "",
			content:         "",
			contentType:     plainTextContentType,
		},
		{
			desc:            "Client requests a directory, should return index.html",
			file:            "/",
			encodingHeader:  "gzip",
			hasGzip:         true,
			isPresent:       true,
			contentEncoding: gzipEncoding,
			content:         "<html><body>index page</body></body></html>\n",
			contentType:     htmlContentType,
		},
	}

	fs := GzipFileServer(http.Dir("./testingdata/"))
	for index, tc := range testCases {
		t.Logf("Starting test case %d: %s", index, tc.desc)
		rr := httptest.NewRecorder()
		req, _ := http.NewRequest("GET", tc.file, nil)
		req.Header.Set("Accept-Encoding", tc.encodingHeader)
		fs.ServeHTTP(rr, req)
		h := rr.Header()

		if len(tc.contentEncoding) == 0 {
			if len(h["Content-Encoding"]) != 0 {
				t.Errorf("Test case %d: %s \n returned wrong content encoding \n expected: %s \n received: %s", index, tc.desc, tc.contentEncoding, h["Content-Encoding"])
			}
		} else {
			if len(h["Content-Encoding"]) == 0 || h["Content-Encoding"][0] != tc.contentEncoding {
				t.Errorf("Test case %d: %s \n returned wrong content encoding \n expected: %s \n received: %s", index, tc.desc, tc.contentEncoding, h["Content-Encoding"])
			}
		}

		if h["Content-Type"][0] != tc.contentType {
			t.Errorf("Test case %d: %s \n returned wrong content type \n expected: %s \n received: %s", index, tc.desc, plainTextContentType, h["Content-Type"])
		}

		if !tc.isPresent {
			if rr.Code != 404 {
				t.Errorf("Test case %d: %s \n GET for missing file %s didn't return 404, returned code %d", index, tc.desc, tc.file, rr.Code)
			}
			continue
		}

		body := ""
		if tc.contentEncoding == gzipEncoding && tc.hasGzip {
			rdr, err := gzip.NewReader(bytes.NewReader(rr.Body.Bytes()))
			if err != nil {
				t.Errorf("Test case %d: %s \n gunzip failed: %s", index, tc.desc, err)
			} else {
				bbody, err := ioutil.ReadAll(rdr)
				if err != nil {
					t.Errorf("Test case %d: %s \n gunzip read failed: %s", index, tc.desc, err)
				} else {
					body = string(bbody)
				}
			}
		} else {
			body = rr.Body.String()
		}
		if body != tc.content {
			t.Errorf("Test case %d: %s \n content didn't match \n received: %s \n expected: %s", index, tc.desc, body, tc.content)
		}
	}
}
