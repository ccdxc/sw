package techsupport

import (
	"encoding/json"
	"io/ioutil"
	"net/http"

	"github.com/pensando/sw/nic/agent/protos/tsproto"
	"github.com/pensando/sw/venice/utils/log"
)

func readTechSupportRequest(req *http.Request) *tsproto.TechSupportRequest {
	body, readErr := ioutil.ReadAll(req.Body)
	if readErr != nil {
		log.Errorf("Read of request failed.")
		return nil
	}

	tsWork := tsproto.TechSupportRequest{}
	jsonErr := json.Unmarshal(body, &tsWork)
	if jsonErr != nil {
		log.Errorf("Unmarshal failed.")
		return nil
	}

	return &tsWork
}

// CollectTechSupport is the REST endpoint to collect techsupport
func (rs *RestServer) CollectTechSupport(w http.ResponseWriter, r *http.Request) {
	log.Infof("Received request to collect techsupport")
	tsWork := readTechSupportRequest(r)
	if tsWork == nil {
		log.Error("Couldn't get tech support request. Exiting Collect of Tech Support.")
	} else {
		rs.State.RQ.Put(tsWork)
		log.Infof("Got request %v", tsWork)
	}

	//rs.worker.DoWork()
}

// ListTechSupportRequests lists all the outstanding techsupport requests
func (rs *RestServer) ListTechSupportRequests(w http.ResponseWriter, r *http.Request) {
	log.Infof("Received request to list all pending tech support requests")
}
