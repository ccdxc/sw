package vospkg

import (
	"context"
	"crypto/tls"
	"encoding/json"
	"expvar"
	"fmt"
	"io"
	"net"
	"net/http"
	"time"

	"github.com/go-martini/martini"
	"github.com/minio/minio-go"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/api/generated/objstore"
	"github.com/pensando/sw/venice/globals"
	"github.com/pensando/sw/venice/utils/log"
	"github.com/pensando/sw/venice/vos"
)

const (
	apiPrefix           = "/apis/v1"
	uploadImagesPath    = "/uploads/images/"
	uploadSnapshotsPath = "/uploads/snapshots/"
	downloadPath        = "/downloads/images/**"
)

type httpHandler struct {
	client   vos.BackendClient
	handler  *martini.ClassicMartini
	server   http.Server
	instance *instance
}

func newHTTPHandler(instance *instance, client vos.BackendClient) (*httpHandler, error) {
	log.InfoLog("msgs", "creating new HTTP backend", "port", globals.VosGRPcPort)
	mux := martini.Classic()
	return &httpHandler{client: client, handler: mux, instance: instance}, nil
}

func (h *httpHandler) start(ctx context.Context, port string, config *tls.Config) {
	log.InfoLog("msg", "starting HTTP listener")
	h.handler.Get(apiPrefix+downloadPath, h.downloadHandler)
	log.InfoLog("msg", "adding path", "path", apiPrefix+uploadImagesPath)
	h.handler.Post(apiPrefix+uploadImagesPath, h.uploadImagesHandler)
	log.InfoLog("msg", "adding path", "path", apiPrefix+uploadSnapshotsPath)
	h.handler.Post(apiPrefix+uploadSnapshotsPath, h.uploadSnapshotsHandler)
	log.InfoLog("msg", "adding path", "path", "/debug/vars")
	h.handler.Get("/debug/vars", expvar.Handler())

	done := make(chan error)
	var ln net.Listener
	var err error
	if config != nil {
		ln, err = tls.Listen("tcp", ":"+port, config)
		if err != nil {
			panic("failed to start VOS HTTP server")
		}
	} else {
		ln, err = net.Listen("tcp", ":"+port)
		if err != nil {
			panic("failed to start VOS HTTP server")
		}
	}

	h.server = http.Server{
		TLSConfig: config,
		Handler:   h.handler,
	}
	go func() {
		close(done)
		h.server.Serve(ln)
	}()
	<-done
	go func() {
		<-ctx.Done()
		h.server.Close()
	}()
}

// ClusterCreateHandler handles the REST call for cluster creation.
func (h *httpHandler) uploadImagesHandler(w http.ResponseWriter, req *http.Request) {
	h.uploadHandler(w, req, objstore.Buckets_images.String())
}

// ClusterCreateHandler handles the REST call for cluster creation.
func (h *httpHandler) uploadSnapshotsHandler(w http.ResponseWriter, req *http.Request) {
	h.uploadHandler(w, req, objstore.Buckets_snapshots.String())
}

// uploadHandler handles all uploads proxy requests.
func (h *httpHandler) uploadHandler(w http.ResponseWriter, req *http.Request, nspace string) {
	log.Infof("got Upload call")
	if req.Method == "POST" {
		req.ParseMultipartForm(32 << 20)
		file, header, err := req.FormFile("file")
		if err != nil {
			log.Errorf("could not get the file from form")
			h.writeError(w, http.StatusBadRequest, "could not get the file from form")
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

		// Request should be authorized again at the backend because this is a proxied call.
		// XXX-TBD(sanjayt): add once the ueer permission are added to the HTTP request by the API Gateway

		log.Infof("got meta in form  as [%+v]", meta)
		// Check if we have the object
		bucket := "default." + nspace
		stat, err := h.client.StatObject(bucket, header.Filename, minio.StatObjectOptions{})
		if err != nil {
			meta[metaCreationTime] = fmt.Sprintf("%s", time.Now().Format(time.RFC3339Nano))
		} else {
			meta[metaCreationTime] = stat.Metadata.Get(metaPrefix + metaCreationTime)
		}
		errs := h.instance.RunPlugins(context.TODO(), nspace, vos.PreOp, vos.Upload, nil, h.client)
		if errs != nil {
			h.writeError(w, http.StatusPreconditionFailed, errs)
			return
		}
		sz, err := h.client.PutObject(bucket, header.Filename, file, -1, minio.PutObjectOptions{UserMetadata: meta, ContentType: contentType})
		if err != nil {
			log.Errorf("failed to write object (%s)", err)
			h.writeError(w, http.StatusInternalServerError, fmt.Sprintf("error writing object (%s)", err))
			return
		}
		in := objstore.Object{
			TypeMeta:   api.TypeMeta{Kind: "Object"},
			ObjectMeta: api.ObjectMeta{Name: header.Filename, Namespace: nspace},
			Spec:       objstore.ObjectSpec{ContentType: contentType},
			Status: objstore.ObjectStatus{
				Size_:  sz,
				Digest: stat.ETag,
			},
		}
		updateObjectMeta(&stat, &in.ObjectMeta)
		stat, err = h.client.StatObject(bucket, header.Filename, minio.StatObjectOptions{})
		if err != nil {
			h.writeError(w, http.StatusInternalServerError, fmt.Sprintf("error verifying image write (%s)", err))
			return
		}
		errs = h.instance.RunPlugins(context.TODO(), nspace, vos.PostOp, vos.Upload, &in, h.client)
		if errs != nil {
			h.writeError(w, http.StatusInternalServerError, errs)
			return
		}
		log.Infof("Wrote object [%v] of size [%v]", header.Filename, sz)

		ret := objstore.Object{
			TypeMeta:   api.TypeMeta{Kind: "Object"},
			ObjectMeta: api.ObjectMeta{Name: header.Filename, Namespace: nspace},
			Spec:       objstore.ObjectSpec{ContentType: contentType},
			Status: objstore.ObjectStatus{
				Size_:  sz,
				Digest: stat.ETag,
			},
		}
		if stat.Size != sz {
			h.writeError(w, http.StatusInternalServerError, fmt.Sprintf("error in object size after write written[%d] readback[%d]", sz, stat.Size))
			return
		}
		updateObjectMeta(&stat, &ret.ObjectMeta)
		b, err := json.Marshal(&ret)
		if err != nil {
			h.writeError(w, http.StatusInternalServerError, fmt.Sprintf("error marshalling return value (%s)", err))
			return
		}
		w.WriteHeader(http.StatusOK)
		w.Write(b)
	} else {
		h.writeError(w, http.StatusMethodNotAllowed, fmt.Sprintf("upsupported method [%v]", req.Method))
	}
}

func (h *httpHandler) downloadHandler(params martini.Params, w http.ResponseWriter, req *http.Request) {
	log.Infof("got Upload call")
	path := params["_1"]
	if path == "" {
		h.writeError(w, http.StatusBadRequest, "empty path")
		return
	}
	// Only supports default.images for now.
	bucket := "default.images"
	buf := make([]byte, 1024*1024)
	ctx := context.Background()
	fr, err := h.client.GetStoreObject(ctx, bucket, path, minio.GetObjectOptions{})
	if err != nil {
		log.Errorf("failed to get object [%v]", path)
		h.writeError(w, http.StatusNotFound, "unknown object")
		return
	}
	errs := h.instance.RunPlugins(ctx, "images", vos.PreOp, vos.Download, nil, h.client)
	if errs != nil {
		h.writeError(w, http.StatusPreconditionFailed, errs)
		return
	}
	w.Header().Set("Content-Disposition", "attachment")
	w.Header().Set("Content-Type", "application/octet-stream; charset=utf-8")
	totsize := 0
	for {
		n, err := fr.Read(buf)
		if err != nil && err != io.EOF {
			log.Errorf("error while reading object (%s)", err)
			h.writeError(w, http.StatusInternalServerError, fmt.Sprintf("error reading object: (%s)", err))
			return
		}
		if n == 0 {
			break
		}
		totsize += n
		w.Write(buf[:n])
	}
	errs = h.instance.RunPlugins(ctx, "images", vos.PostOp, vos.Download, nil, h.client)
	if errs != nil {
		h.writeError(w, http.StatusInternalServerError, errs)
		return
	}
	w.WriteHeader(http.StatusOK)
}

func (h *httpHandler) writeError(w http.ResponseWriter, code int, msg interface{}) {
	w.WriteHeader(code)
	w.Write([]byte(fmt.Sprintf("%v", msg)))
}
