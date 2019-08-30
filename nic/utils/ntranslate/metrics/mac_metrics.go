package metrics

import (
	"strconv"

	"github.com/pensando/sw/api"
	delphiProto "github.com/pensando/sw/nic/agent/nmd/protos/delphi"
	dnetproto "github.com/pensando/sw/nic/agent/protos/generated/delphi/netproto/delphi"
	clientApi "github.com/pensando/sw/nic/delphi/gosdk/client_api"
)

// delphi client
var delphiClient clientApi.Client

type macMetricsXlate struct{}

// KeyToMeta converts network key to meta
func (n *macMetricsXlate) KeyToMeta(key interface{}) *api.ObjectMeta {
	if portID, ok := key.(uint32); ok {
		intfName := strconv.FormatUint(uint64(portID), 10)
		if delphiClient != nil {
			nodeUUID := ""
			nslist := delphiProto.NaplesStatusList(delphiClient)
			for _, ns := range nslist {
				nodeUUID = ns.GetDSCName()
			}
			intfList := dnetproto.InterfaceList(delphiClient)
			for _, intf := range intfList {
				if intf.Interface.Status.UplinkPortID == portID {
					if nodeUUID != "" {
						intfName = nodeUUID + "-" + intf.Interface.ObjectMeta.Name
					} else {
						intfName = intf.Interface.ObjectMeta.Name
					}
				}
			}
		}

		return &api.ObjectMeta{Tenant: "default", Namespace: "default", Name: intfName}
	}
	return nil
}

// MetaToKey converts meta to network key
func (n *macMetricsXlate) MetaToKey(meta *api.ObjectMeta) interface{} {
	return meta.Name
}

// SetDelphiClient sets the delphi client to be used for ntranslate
func SetDelphiClient(dclient clientApi.Client) {
	delphiClient = dclient
}
