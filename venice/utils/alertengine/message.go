package alertengine

import (
	"fmt"

	"github.com/pensando/sw/api/generated/monitoring"
	"github.com/pensando/sw/venice/globals"
	"github.com/pensando/sw/venice/utils/syslog"
)

// GenerateSyslogMessage helper to generate the syslog message from given alert
func GenerateSyslogMessage(alert *monitoring.Alert) *syslog.Message {
	crTime, _ := alert.CreationTime.Time()
	modTime, _ := alert.ModTime.Time()
	strData := syslog.StrData{}
	m := map[string]string{
		"kind":          alert.GetKind(),
		"name":          alert.GetName(),
		"tenant":        alert.GetTenant(),
		"namespace":     alert.GetNamespace(),
		"creation-time": crTime.String(),
		"mod-time":      modTime.String(),
		"state":         alert.Spec.GetState(),
		"severity":      alert.Status.GetSeverity(),
		"message":       alert.Status.GetMessage(),
	}

	if alert.Status.GetSource() != nil {
		m["src-node"] = alert.Status.GetSource().GetNodeName()
		m["src-component"] = alert.Status.GetSource().GetComponent()

	}

	if alert.Status.GetObjectRef() != nil {
		m["ref-tenant"] = alert.Status.GetObjectRef().GetTenant()
		m["ref-namespace"] = alert.Status.GetObjectRef().GetNamespace()
		m["ref-kind"] = alert.Status.GetObjectRef().GetKind()
		m["ref-name"] = alert.Status.GetObjectRef().GetName()

	}

	strData[fmt.Sprintf("alert@%d", globals.PensandoPEN)] = m

	return &syslog.Message{MsgID: alert.GetUUID(), Msg: alert.Status.GetMessage(), StructuredData: strData}
}
