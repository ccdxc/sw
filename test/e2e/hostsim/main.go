package main

import (
	"encoding/json"
	"io/ioutil"
	"net/http"

	log "github.com/Sirupsen/logrus"
	"github.com/gorilla/mux"

	"github.com/pensando/sw/orch/simapi"
)

type restAPIFunc func(r *http.Request) (interface{}, error)

func main() {
	log.Infof("Starting simulator...")
	InitDocker()
	InitNetworking()
	serveHTTP()
	log.Infof("Exiting simulator...")
}

func serveHTTP() {
	r := mux.NewRouter()
	r.HandleFunc("/nwifs/create", makeHTTPHandler(restAPIFunc(createNwIF))).Methods("POST")
	r.HandleFunc("/nwifs/{id}/delete", makeHTTPHandler(restAPIFunc(deleteNwIF))).Methods("POST")
	r.HandleFunc("/nwifs/cleanup", makeHTTPHandler(restAPIFunc(cleanup))).Methods("POST")
	log.Infof("Starting server at :5050")
	http.ListenAndServe(":5050", r)
}

func createNwIF(r *http.Request) (interface{}, error) {
	req := simapi.NwIFSetReq{}
	resp := simapi.NwIFSetResp{}
	content, err := ioutil.ReadAll(r.Body)
	if err != nil {
		log.Errorf("Failed to read request: %v", err)
		resp.ErrorMsg = err.Error()
		return resp, err
	}

	if err := json.Unmarshal(content, &req); err != nil {
		log.Errorf("Unmarshal err %s", content)
		resp.ErrorMsg = err.Error()
		return resp, err
	}

	name, mac, err := AddNwIF(req.MacAddr, req.IPAddr, req.Vlan)
	if err != nil {
		resp.ErrorMsg = err.Error()
		return resp, err
	}

	resp.UUID = name
	resp.MacAddr = mac
	return resp, nil
}

func deleteNwIF(r *http.Request) (interface{}, error) {
	resp := simapi.NwIFDelResp{}
	kvs := mux.Vars(r)
	err := DeleteNwIF(kvs["id"])
	if err != nil {
		resp.ErrorMsg = err.Error()
	}

	return resp, err
}

func cleanup(r *http.Request) (interface{}, error) {
	res := CleanUp()
	resp := simapi.NwIFDelResp{ErrorMsg: res}
	return resp, nil
}

// Simple Wrapper for http handlers
func makeHTTPHandler(handlerFunc restAPIFunc) http.HandlerFunc {
	// Create a closure and return an anonymous function
	return func(w http.ResponseWriter, r *http.Request) {
		// Call the handler
		resp, err := handlerFunc(r)
		if err != nil {
			// Log error
			log.Errorf("Handler for %s %s returned error: %s", r.Method, r.URL, err)

			if resp == nil {
				// Send HTTP response
				http.Error(w, err.Error(), http.StatusInternalServerError)
			} else {
				// Send HTTP response as Json
				content, err1 := json.Marshal(resp)
				if err1 != nil {
					http.Error(w, err.Error(), http.StatusInternalServerError)
					return
				}

				w.WriteHeader(http.StatusInternalServerError)
				w.Write(content)
			}
		} else {
			// Send HTTP response as Json
			content, err := json.Marshal(resp)
			if err != nil {
				http.Error(w, err.Error(), http.StatusInternalServerError)
				return
			}
			w.Write(content)
		}
	}
}
