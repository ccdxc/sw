package mock

import (
	"github.com/pensando/sw/api"
	evtsapi "github.com/pensando/sw/api/generated/events"
	"github.com/pensando/sw/venice/utils/events"
	"github.com/pensando/sw/venice/utils/log"
	"github.com/pensando/sw/venice/utils/runtime"
)

// It is recommended to use this mock events recorder only in the unit-tests and in some integ. test which are not
// testing the events. Events recorded using the mock recorder will not make it to venice rather
// it will be logged as a info message using the logger.

// mock implementation of events recorder
type mockRecorder struct {
	name   string     // name (unique id/key) of the recorder
	logger log.Logger // logger
}

// NewRecorder is a mock events recorder that logs events using the given logger.
// It is used in the unit tests
func NewRecorder(name string, l log.Logger) events.Recorder {
	return &mockRecorder{name: name, logger: l}
}

// Event mock implementation that records the event using a logger
func (mr *mockRecorder) Event(eventType string, severity evtsapi.SeverityLevel, message string, objRef interface{}) {
	var objRefMeta *api.ObjectMeta
	var objRefKind string
	var err error

	// derive reference object details from the given object
	if objRef != nil {
		objRefMeta, err = runtime.GetObjectMeta(objRef)
		if err != nil {
			mr.logger.Debugf("{%s} failed to get the object meta from reference object, err: %v", mr.name, err)
		}

		objRefKind = objRef.(runtime.Object).GetObjectKind()
	}
	if objRefMeta != nil {
		mr.logger.InfoLog(
			"recorder_name", mr.name,
			"type", eventType,
			"severity", evtsapi.SeverityLevel_name[int32(severity)],
			"message", message,
			"object-ref.tenant", objRefMeta.GetTenant(),
			"object-ref.namespace", objRefMeta.GetNamespace(),
			"object-ref.kind", objRefKind,
			"object-ref.name", objRefMeta.GetName())
	} else {
		mr.logger.InfoLog(
			"recorder_name", mr.name,
			"type", eventType,
			"severity", evtsapi.SeverityLevel_name[int32(severity)],
			"message", message)
	}
}

// StartExport mock impl
func (mr *mockRecorder) StartExport() {}

// Close mock impl
func (mr *mockRecorder) Close() {}
