package mock

import (
	"bytes"
	"strings"
	"testing"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/api/generated/cluster"
	"github.com/pensando/sw/events/generated/eventtypes"
	"github.com/pensando/sw/venice/globals"
	"github.com/pensando/sw/venice/utils/events/recorder"
	"github.com/pensando/sw/venice/utils/log"
	. "github.com/pensando/sw/venice/utils/testutils"
)

// TestMockEvtsRecorder tests the mock events recorder
func TestMockEvtsRecorder(t *testing.T) {
	// create a logger with buffer
	buf := &bytes.Buffer{}
	defer buf.Reset()
	logConfig := log.GetDefaultConfig("mock_recorder_test")
	l := log.GetNewLogger(logConfig).SetOutput(buf)

	// recorder few events using the mock recorder
	mockRecorder := NewRecorder("mock_recorder_test", l)
	mockRecorder.Event(eventtypes.SERVICE_STARTED, "dummy event w/o object ref.", nil)
	mockRecorder.Event(eventtypes.SERVICE_STOPPED, "dummy event with object ref.", &cluster.Node{
		TypeMeta: api.TypeMeta{Kind: "Node"},
		ObjectMeta: api.ObjectMeta{Tenant: globals.DefaultTenant,
			Namespace: globals.DefaultNamespace,
			Name:      "test-1",
		},
	})

	// check the events in the buffer
	bufStr := buf.String()
	Assert(t, strings.Contains(bufStr, "type=SERVICE_STARTED category=System severity=DEBUG message=\"dummy event w/o object ref.\""),
		"expected event not available in the buffer")
	Assert(t, strings.Contains(bufStr, "type=SERVICE_STOPPED category=System severity=WARN message=\"dummy event with object ref.\" object-ref.tenant=default object-ref.namespace=default object-ref.kind=Node object-ref.name=test-1"),
		"expected event not available in the buffer")

}

// TestMockEvtsRecorderWithOverride overrides the singleton recorder with mock recorder and tests the recorder
func TestMockEvtsRecorderWithOverride(t *testing.T) {
	// create a logger with buffer
	buf := &bytes.Buffer{}
	defer buf.Reset()
	logConfig := log.GetDefaultConfig("mock_recorder_test")
	l := log.GetNewLogger(logConfig).SetOutput(buf)

	// recorder few events using the mock recorder
	mr := NewRecorder("mock_recorder_test", l)
	recorder.Override(mr)

	// once the singleton is overridden, we can record the event using recorder.Event; no need to use the recorder instance.
	recorder.Event(eventtypes.SERVICE_STARTED, "dummy event w/o object ref.", nil)
	recorder.Event(eventtypes.SERVICE_STOPPED, "dummy event with object ref.", &cluster.Node{
		TypeMeta: api.TypeMeta{Kind: "Node"},
		ObjectMeta: api.ObjectMeta{Tenant: globals.DefaultTenant,
			Namespace: globals.DefaultNamespace,
			Name:      "test-1",
		},
	})

	// check the events in the buffer
	bufStr := buf.String()
	Assert(t, strings.Contains(bufStr, "type=SERVICE_STARTED category=System severity=DEBUG message=\"dummy event w/o object ref.\""),
		"expected event not available in the buffer")
	Assert(t, strings.Contains(bufStr, "type=SERVICE_STOPPED category=System severity=WARN message=\"dummy event with object ref.\" object-ref.tenant=default object-ref.namespace=default object-ref.kind=Node object-ref.name=test-1"),
		"expected event not available in the buffer")
}
