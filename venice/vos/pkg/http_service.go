package vos

import (
	"context"
	"encoding/json"
	"fmt"
	"net/http"
	"time"

	"github.com/go-martini/martini"
	"github.com/minio/minio-go"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/api/generated/objstore"

	"github.com/pensando/sw/venice/globals"
	"github.com/pensando/sw/venice/utils/log"
)

const (
	apiPrefix  = "/apis/v1"
	uploadPath = "/uploads/"
)

type httpHandler struct {
	client backendClient
	server *martini.ClassicMartini
}

func newHTTPHandler(client backendClient) (*httpHandler, error) {
	log.InfoLog("msgs", "creating new HTTP backend", "port", globals.VosGRPcPort)
	mux := martini.Classic()
	return &httpHandler{client: client, server: mux}, nil
}

func (h *httpHandler) start(ctx context.Context) {
	log.InfoLog("msg", "starting HTTP listener")
	h.server.Post(apiPrefix+uploadPath, h.uploadHandler)
	log.InfoLog("msg", "adding path", "path", apiPrefix+uploadPath)
	done := make(chan error)
	go func() {
		close(done)
		h.server.RunOnAddr(":" + globals.VosHTTPPort)
	}()
	<-done
}

// ClusterCreateHandler handles the REST call for cluster creation.
func (h *httpHandler) uploadHandler(w http.ResponseWriter, req *http.Request) {
	log.Infof("got Upload call")
	if req.Method == "POST" {
		req.ParseMultipartForm(32 << 20)
		file, header, err := req.FormFile("file")
		if err != nil {
			log.Errorf("could not get the file from form")
			w.WriteHeader(http.StatusBadRequest)
			w.Write([]byte("could not get the file from form"))
			return
		}
		contentType := req.FormValue("content-type")
		if contentType == "" {
			contentType = "application/octet-stream"
		}
		meta := make(map[string]string)
		for k, v := range req.Form {
			if k != metaFileName && k != metaCreationTime && k != metaContentType {
				for i := range v {
					if _, ok := meta[k]; ok {
						meta[fmt.Sprintf("%s-%d", k, i)] = v[i]
					} else {
						meta[k] = v[i]
					}
				}
				meta[k] = v[0]
			}
		}
		log.Infof("got meta in form  as [%+v]", meta)
		// Check if we have the object
		stat, err := h.client.StatObject("default.images", header.Filename, minio.StatObjectOptions{})
		if err != nil {
			meta[metaCreationTime] = fmt.Sprintf("%s", time.Now().Format(time.RFC3339Nano))
		} else {
			meta[metaCreationTime] = stat.Metadata.Get(metaPrefix + metaCreationTime)
		}

		sz, err := h.client.PutObject("default.images", header.Filename, file, -1, minio.PutObjectOptions{UserMetadata: meta, ContentType: contentType})
		if err != nil {
			log.Errorf("failed to write object (%s)", err)
			w.WriteHeader(http.StatusInternalServerError)
			w.Write([]byte(fmt.Sprintf("error writing object (%s)", err)))
			return
		}
		log.Infof("Wrote object [%v] of size [%v]", header.Filename, sz)
		stat, err = h.client.StatObject("default.images", header.Filename, minio.StatObjectOptions{})
		if err != nil {
			w.WriteHeader(http.StatusInternalServerError)
			w.Write([]byte(fmt.Sprintf("error verifying image write (%s)", err)))
			return
		}
		ret := objstore.Object{
			TypeMeta:   api.TypeMeta{Kind: "Object"},
			ObjectMeta: api.ObjectMeta{Name: header.Filename},
			Spec:       objstore.ObjectSpec{ContentType: contentType},
			Status: objstore.ObjectStatus{
				Size_:  sz,
				Digest: stat.ETag,
			},
		}
		if stat.Size != sz {
			w.WriteHeader(http.StatusInternalServerError)
			w.Write([]byte(fmt.Sprintf("error in object size after write written[%d] readback[%d]", sz, stat.Size)))
			return
		}
		updateObjectMeta(&stat, &ret.ObjectMeta)
		b, err := json.Marshal(&ret)
		if err != nil {
			w.WriteHeader(http.StatusInternalServerError)
			w.Write([]byte(fmt.Sprintf("error marshalling return value (%s)", err)))
			return
		}
		w.WriteHeader(http.StatusOK)
		w.Write(b)
	} else {
		w.WriteHeader(http.StatusMethodNotAllowed)
		w.Write([]byte(fmt.Sprintf("upsupported method [%v]", req.Method)))
	}
}
