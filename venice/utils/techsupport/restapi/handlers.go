package techsupport

import (
	"encoding/json"
	"fmt"
	"io/ioutil"
	"net/http"

	"github.com/pensando/sw/api/generated/diagnostics"
	"github.com/pensando/sw/nic/agent/protos/tsproto"
	"github.com/pensando/sw/venice/utils/log"
	action "github.com/pensando/sw/venice/utils/techsupport/actionengine"
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
		rs.tsCh <- *tsWork
		log.Infof("Got request %v", tsWork)
	}

	//rs.worker.DoWork()
}

// ListTechSupportRequests lists all the outstanding techsupport requests
func (rs *RestServer) ListTechSupportRequests(w http.ResponseWriter, r *http.Request) {
	log.Infof("Received request to list all pending tech support requests")
}

func readDiagnosticsRequest(req *http.Request) (*diagnostics.DiagnosticsRequest, error) {
	body, readErr := ioutil.ReadAll(req.Body)
	if readErr != nil {
		log.Errorf("Read of request failed.")
		return nil, fmt.Errorf("failed to read request body")
	}

	diagnosticsReq := diagnostics.DiagnosticsRequest{}
	jsonErr := json.Unmarshal(body, &diagnosticsReq)
	if jsonErr != nil {
		log.Errorf("Unmarshal failed.")
		return nil, fmt.Errorf("unmarshalling of diagnostics request failed")
	}

	return &diagnosticsReq, nil
}

// HandleDiagnostics handles diagnostics request
func (rs *RestServer) HandleDiagnostics(w http.ResponseWriter, r *http.Request) {
	log.Infof("Handling diagnostics request")

	diagnosticsReq, err := readDiagnosticsRequest(r)
	if err != nil {
		log.Errorf("Failed to read diagnostics request. Err : %v", err)
		w.WriteHeader(http.StatusBadRequest)
		return
	}

	out, err := action.RunDiagnosticsActions(diagnosticsReq)
	if err != nil {
		log.Errorf("Failed to run diagnostics actions. Err : %v", err)
		w.WriteHeader(http.StatusInternalServerError)
		return
	}

	w.Write(out)
}
