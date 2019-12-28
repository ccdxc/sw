package techsupport

import (
	"encoding/json"
	"fmt"
	"io/ioutil"
	"net/http"
	"strings"
	"time"

	"github.com/pensando/sw/api/generated/diagnostics"
	"github.com/pensando/sw/nic/agent/protos/tsproto"
	"github.com/pensando/sw/venice/utils/log"
	action "github.com/pensando/sw/venice/utils/techsupport/actionengine"
	export "github.com/pensando/sw/venice/utils/techsupport/exporter"
)

func readTechSupportRequest(req *http.Request) (*tsproto.TechSupportRequest, error) {
	body, readErr := ioutil.ReadAll(req.Body)
	if readErr != nil {
		log.Errorf("Read of request failed.")
		return nil, readErr
	}

	tsWork := tsproto.TechSupportRequest{}
	jsonErr := json.Unmarshal(body, &tsWork)
	if jsonErr != nil {
		log.Errorf("Unmarshal failed. Err: %v", jsonErr)
		return nil, jsonErr
	}

	return &tsWork, nil
}

// CollectTechSupport is the REST endpoint to collect techsupport
func (rs *RestServer) CollectTechSupport(w http.ResponseWriter, r *http.Request) {
	log.Infof("Received request to collect techsupport")
	tsWork, err := readTechSupportRequest(r)
	if tsWork == nil || err != nil {
		log.Errorf("Couldn't get tech support request. Exiting Collect of Tech Support. Err : %v", err)
		tsWork.Status.Status = tsproto.TechSupportRequestStatus_Failed
		tsWork.Status.Reason = fmt.Sprintf("%v", err)
		writeJSON(w, http.StatusInternalServerError, tsWork)
	} else {
		log.Infof("Got request %v", tsWork)

		var instanceID, instanceName string
		if tsWork.Spec.InstanceID == "" {
			instanceID = string(time.Now().Unix())
			instanceID = strings.Replace(instanceID, " ", "-", -1)
			instanceName = "techsupport"
		} else {
			instanceID = tsWork.Spec.InstanceID
			instanceName = tsWork.ObjectMeta.Name
		}

		targetID := fmt.Sprintf("%s-%s", instanceName, instanceID)
		localTarget := fmt.Sprintf("%v.tar.gz", targetID)

		err := action.CollectTechSupport(rs.cfg, targetID, tsWork)
		if err != nil {
			log.Errorf("Techsupport collection failed. Err : %v", err)
			tsWork.Status.Status = tsproto.TechSupportRequestStatus_Failed
			tsWork.Status.Reason = fmt.Sprintf("%v", err)
			writeJSON(w, http.StatusInternalServerError, tsWork)
			return
		}

		err = export.GenerateTechsupportZip(localTarget, rs.cfg.FileSystemRoot+"/"+targetID)
		if err != nil {
			log.Errorf("Failed to generate techsupport zip. Err : %v", err)
			tsWork.Status.Status = tsproto.TechSupportRequestStatus_Failed
			tsWork.Status.Reason = fmt.Sprintf("Failed to generate techsupport zip. Err : %v", err)
			writeJSON(w, http.StatusInternalServerError, tsWork)
			return
		}
		tarballFile := rs.cfg.FileSystemRoot + "/" + targetID + "/" + localTarget
		tsWork.Status.URI = tarballFile
		tsWork.Status.Status = tsproto.TechSupportRequestStatus_Completed
		err = writeJSON(w, http.StatusOK, tsWork)
		if err != nil {
			log.Errorf("Failed to write JSON response. Err : %v", err)
			tsWork.Status.Status = tsproto.TechSupportRequestStatus_Failed
			tsWork.Status.Reason = fmt.Sprintf("Failed to write JSON response %v", err)
			writeJSON(w, http.StatusInternalServerError, tsWork)
			return
		}
	}

	log.Info("Techsupport request successfully handled.")
	return
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

	output := fmt.Sprintf("{\"result\":\"%s\"} }", string(out))

	writeJSON(w, http.StatusOK, output)
}

func writeJSON(w http.ResponseWriter, code int, v interface{}) error {
	// Set content type to json
	w.Header().Set("Content-Type", "application/json")

	// Set the response code
	w.WriteHeader(code)

	// write json output
	return json.NewEncoder(w).Encode(v)
}
