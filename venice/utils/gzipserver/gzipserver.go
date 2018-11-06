package gzipserver

import (
	"bytes"
	"compress/gzip"
	"fmt"
	"io/ioutil"
	"net/http"
	"os"
	"path"
	"strings"
)

const (
	gzipEncoding   = "gzip"
	gzipExtenstion = ".gz"
)

type gzipfileHandler struct {
	root http.FileSystem
}

// GzipFileServer replaces the default go file server
// If a gzip version of the requested file is present and
// the client accepts gzip encoding, we send it. Otherwise,
// we look for a normal version of the file. If the client
// does not accept gzip but we only have a gzip version,
// we decompress it and send it
func GzipFileServer(root http.FileSystem) http.Handler {
	return &gzipfileHandler{root}
}

func (f *gzipfileHandler) ServeHTTP(w http.ResponseWriter, r *http.Request) {
	if !strings.HasPrefix(r.URL.Path, "/") {
		r.URL.Path = "/" + r.URL.Path
	}
	fpath := path.Clean(r.URL.Path)

	if strings.HasSuffix(fpath, "/") {
		fpath += "index.html"
	}

	// Check the request headers to see if client supports gzip
	clientSupportsGzip := false
	for _, header := range strings.Split(r.Header.Get("Accept-Encoding"), ",") {
		// Stripping white space
		header = strings.Replace(header, " ", "", -1)
		headerTokens := strings.Split(header, ";")
		encoding := headerTokens[0]
		qVal := "1"
		if len(headerTokens) > 1 {
			qIndex := strings.Index(headerTokens[1], "q=")
			if qIndex != -1 && len(headerTokens[1]) > qIndex+2 {
				qVal = headerTokens[1][qIndex+2:]
			}
		}
		if encoding == gzipEncoding && (qVal == "0" || qVal == "0.0") {
			break
		} else if encoding == gzipEncoding || encoding == "*" || encoding == "*/*" {
			clientSupportsGzip = true
			break
		}
	}

	var fileStat os.FileInfo
	// Try for a compressed version if supported
	file, err := f.root.Open(fpath + gzipExtenstion)
	if err == nil {
		defer file.Close()
		fileStat, err = file.Stat()
	}
	if err == nil {
		// Gzip file exists
		if clientSupportsGzip {
			w.Header().Set("Content-Encoding", gzipEncoding)
			w.Header().Del("Content-Length")
		} else {
			// Client doesn't support gzip, so
			// we decompress before sending
			gr, err := gzip.NewReader(file)
			if err != nil {
				http.Error(w, fmt.Sprintf("Unexpected server error while trying to read file %s", fpath), 500)
			}
			defer gr.Close()
			data, err := ioutil.ReadAll(gr)
			if err != nil {
				http.Error(w, fmt.Sprintf("Unexpected server error while trying to read file %s", fpath), 500)
			}
			ungzipFile := bytes.NewReader(data)
			http.ServeContent(w, r, fpath, fileStat.ModTime(), ungzipFile)
			return
		}
	} else {
		file, err = f.root.Open(fpath)
		if err != nil {
			// Doesn't exist compressed or uncompressed
			http.NotFound(w, r)
			return
		}
		defer file.Close()
		fileStat, err = file.Stat()
	}
	http.ServeContent(w, r, fpath, fileStat.ModTime(), file)
}
