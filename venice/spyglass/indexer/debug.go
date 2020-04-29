package indexer

import (
	"encoding/json"
	"io/ioutil"
	"net/http"
	"sync/atomic"

	"github.com/pensando/sw/venice/utils/log"
)

const (
	disableFwlogIndexingKey = "disableFwlogIndex"
)

// HandleDebugConfig handles GET and POST for debug config
// Its exported for testing.
func HandleDebugConfig(idrInt Interface) http.HandlerFunc {
	return func(w http.ResponseWriter, r *http.Request) {
		idr := idrInt.(*Indexer)
		switch r.Method {
		case "GET":
			config := map[string]interface{}{}
			switch atomic.LoadInt32(&idr.indexFwlogs) {
			case enableFwlogIndexing:
				config[disableFwlogIndexingKey] = true
			case disableFwlogIndexing:
				config[disableFwlogIndexingKey] = false
			}
			out, err := json.Marshal(config)
			if err != nil {
				log.Errorf("Error in marshling output: %v", err)
				http.Error(w, "error in marshling output", http.StatusInternalServerError)
			}
			w.WriteHeader(http.StatusOK)
			w.Header().Set("Content-Type", "application/json; charset=utf-8")
			w.Write(out)
		case "POST":
			body, err := ioutil.ReadAll(r.Body)
			if err != nil {
				log.Errorf("Error reading body: %v", err)
				http.Error(w, "can't read body", http.StatusBadRequest)
				return
			}

			config := map[string]interface{}{}
			err = json.Unmarshal(body, &config)
			if err != nil {
				log.Errorf("Error in unmarshaling body: %v", err)
				http.Error(w, "error in unmarshaling body", http.StatusBadRequest)
				return
			}

			if v, ok := config[disableFwlogIndexingKey]; ok {
				switch v.(type) {
				case bool:
					temp := v.(bool)
					if temp {
						atomic.StoreInt32(&idr.indexFwlogs, enableFwlogIndexing)
					} else {
						atomic.StoreInt32(&idr.indexFwlogs, disableFwlogIndexing)
					}
				default:
					http.Error(w, "incorrect value for key 'disableFwlogIndex'", http.StatusBadRequest)
				}
			}
		default:
			http.Error(w, "only GET and POST are supported", http.StatusBadRequest)
		}
	}
}
