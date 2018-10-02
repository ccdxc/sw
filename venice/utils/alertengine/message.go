package alertengine

import (
	"github.com/pensando/sw/api/generated/monitoring"
	"github.com/pensando/sw/venice/utils/syslog"
)

// GenerateSyslogMessage helper to generate the syslog message from given alert
func GenerateSyslogMessage(alert *monitoring.Alert) *syslog.Message {
	crTime, _ := alert.CreationTime.Time()
	modTime, _ := alert.ModTime.Time()
	strData := syslog.StrData{}
	strData["type"] = map[string]string{
		"kind": alert.GetKind(),
	}
	strData["meta"] = map[string]string{
		"name":          alert.GetName(),
		"uuid":          alert.GetUUID(),
		"tenant":        alert.GetTenant(),
		"namespace":     alert.GetNamespace(),
		"creation-time": crTime.String(),
		"mod-time":      modTime.String(),
	}
	strData["spec"] = map[string]string{
		"state": alert.Spec.GetState(),
	}
	strData["status"] = map[string]string{
		"severity":  alert.Status.GetSeverity(),
		"message":   alert.Status.GetMessage(),
		"event-uri": alert.Status.GetEventURI(),
	}
	strData["status.reason"] = map[string]string{
		"policy-id": alert.Status.Reason.GetPolicyID(),
	}

	if alert.Status.GetSource() != nil {
		strData["status.source"] = map[string]string{
			"node-name": alert.Status.GetSource().GetNodeName(),
			"component": alert.Status.GetSource().GetComponent(),
		}
	}

	if alert.Status.GetObjectRef() != nil {
		strData["status.object-ref"] = map[string]string{
			"tenant":    alert.Status.GetObjectRef().GetTenant(),
			"namespace": alert.Status.GetObjectRef().GetNamespace(),
			"kind":      alert.Status.GetObjectRef().GetKind(),
			"name":      alert.Status.GetObjectRef().GetName(),
			"uri":       alert.Status.GetObjectRef().GetURI(),
		}
	}

	return &syslog.Message{MsgID: alert.GetUUID(), Msg: alert.Status.GetMessage(), StructuredData: strData}
}
