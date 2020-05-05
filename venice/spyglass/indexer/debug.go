package indexer

import (
	"encoding/json"
	"io/ioutil"
	"net/http"
	"sync/atomic"

	"github.com/pensando/sw/venice/utils/log"
)

const (
	disableFwlogIndexingKey    = "disableFwlogIndex"
	flowlogsRateLimitConfigKey = "flowlogsRLConfig"
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

			getFlowlogsRatelimitConfig(idr, config)

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

			updateFlowlogsRatelimitConfig(w, idr, config)
		default:
			http.Error(w, "only GET and POST are supported", http.StatusBadRequest)
		}
	}
}

func getFlowlogsRatelimitConfig(idr *Indexer, config map[string]interface{}) {
	data := []map[string]interface{}{}
	idr.flowlogsRateLimiters.limiters.Range(func(k interface{}, v interface{}) bool {
		if rl, ok := v.(*RateLimiter); ok {
			r := rl.rate()
			b := rl.burst()
			data = append(data, map[string]interface{}{"name": k, "rate": r, "burst": b})
		}
		return true
	})
	config[flowlogsRateLimitConfigKey] = data
}

func updateFlowlogsRatelimitConfig(w http.ResponseWriter, idr *Indexer, config map[string]interface{}) {
	if v, ok := config[flowlogsRateLimitConfigKey]; ok {
		data, ok := v.([]interface{})
		if !ok {
			http.Error(w, "flowlogsRLConfig should be in form []map[string]interface{}", http.StatusBadRequest)
			return
		}
		for _, temp := range data {
			configMap, ok := temp.(map[string]interface{})
			if !ok {
				http.Error(w, "flowlogsRLConfig should be in form []map[string]interface{}", http.StatusBadRequest)
				return
			}
			name, ok := configMap["name"]
			if !ok {
				http.Error(w, "flowlogsRLConfig tenant name is required for setting rate-limit config", http.StatusBadRequest)
				return
			}
			n, ok := name.(string)
			if !ok {
				http.Error(w, "flowlogsRLConfig incorrect name type, supported type is string", http.StatusBadRequest)
				return
			}
			rate, rateOk := configMap["rate"]
			burst, burstOk := configMap["burst"]
			if rateOk && burstOk {
				r, ok := rate.(float64)
				if !ok {
					http.Error(w, "flowlogsRLConfig incorrect rate type, supported type is float64", http.StatusBadRequest)
					return
				}
				b, ok := burst.(float64)
				if !ok {
					http.Error(w, "flowlogsRLConfig incorrect burst type, supported type is int", http.StatusBadRequest)
					return
				}
				idr.flowlogsRateLimiters.addRateLimiter(n, r, int(b))
			} else {
				http.Error(w, "flowlogsRLConfig both rate and burst are required", http.StatusBadRequest)
				return
			}
		}
	}
}
