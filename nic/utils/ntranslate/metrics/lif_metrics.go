package metrics

import (
	"fmt"
	"strconv"
	"time"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/nic/agent/protos/netproto"
	"github.com/pensando/sw/venice/globals"
	"github.com/pensando/sw/venice/utils/log"
	"github.com/pensando/sw/venice/utils/netutils"
)

type lifMetricsXlate struct{}

const maxRetry = 5

// Agenturl exports the rest endpoint for netagent
var Agenturl = globals.Localhost + ":" + globals.AgentRESTPort

// GetLifName converts lifId to lifname over rest call to netagent
func GetLifName(lifID uint64) string {
	// ToDo Make this into function call once tmagent integrates with netagent
	var intf netproto.Interface
	intfName := strconv.FormatUint(uint64(lifID), 10)
	reqURL := fmt.Sprintf("http://%s/api/mapping/interfaces/%s", Agenturl, intfName)
	var err error
	for i := 0; i < maxRetry; i++ {
		err = netutils.HTTPGet(reqURL, &intf)
		if err == nil {
			intfName = intf.ObjectMeta.Name
			return intfName
		}
		time.Sleep(time.Millisecond * 100)
	}
	log.Warnf("failed to Get from %s, %v", reqURL, err)
	return ""
}
func (n *lifMetricsXlate) KeyToMeta(key interface{}) *api.ObjectMeta {
	if lifID, ok := key.(uint64); ok {
		intfName := GetLifName(lifID)
		if intfName != "" {
			return &api.ObjectMeta{Tenant: "default", Namespace: "default", Name: intfName}
		}
	}
	return nil
}

// MetaToKey converts meta to network key
func (n *lifMetricsXlate) MetaToKey(meta *api.ObjectMeta) interface{} {
	return meta.Name
}
