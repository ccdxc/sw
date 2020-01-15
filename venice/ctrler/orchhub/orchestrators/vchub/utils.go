package vchub

import (
	"fmt"
	"strings"

	"github.com/pensando/sw/venice/ctrler/orchhub/orchestrators/vchub/defs"
	"github.com/pensando/sw/venice/ctrler/orchhub/utils"
	"github.com/pensando/sw/venice/utils/netutils"
	conv "github.com/pensando/sw/venice/utils/strconv"

	"github.com/vmware/govmomi/vim25/types"
)

var (
	vmNameKey = createLabelKey("vm-name")
)

func createLabelKey(tag string) string {
	return fmt.Sprintf("vcenter.%s", tag)
}

func generateLabelsFromTags(existingLabels map[string]string, tagMsg defs.TagMsg) map[string]string {
	labels := map[string]string{}
	for _, tagEntry := range tagMsg.Tags {
		key := createLabelKey(tagEntry.Category)
		if len(labels[key]) == 0 {
			labels[key] = tagEntry.Name
		} else {
			labels[key] = fmt.Sprintf("%s:%s", labels[key], tagEntry.Name)
		}
	}
	// Only labels that aren't tag based are vm-name and orch-name
	// Add old values of vm-name and orch-name
	// TODO: vm-name and orch-name could technically conflict
	// with cateory names. Is this ok?
	// In case of conflict, we overwrite with vm/orch name
	if v, ok := existingLabels[vmNameKey]; ok {
		labels[vmNameKey] = v
	}

	if v, ok := existingLabels[utils.OrchNameKey]; ok {
		labels[utils.OrchNameKey] = v
	}
	return labels
}

func addVMNameLabel(labels map[string]string, name string) {
	labels[vmNameKey] = name
}

func createPGName(networkName string) string {
	return fmt.Sprintf("%s%s", defs.DefaultPGPrefix, networkName)
}

func createNetworkName(name string) string {
	return strings.TrimPrefix(name, defs.DefaultPGPrefix)
}

func isPensandoDVS(name string) bool {
	return strings.HasPrefix(name, defs.DefaultDVSPrefix)
}

func createDVSName(dcName string) string {
	return fmt.Sprintf("%s%s", defs.DefaultDVSPrefix, dcName)
}

func isObjForDC(key string, vcID string, dcID string) bool {
	return strings.HasPrefix(key, utils.CreateGlobalKeyPrefix(vcID, dcID))
}

func isPensandoHost(hConfig *types.HostConfigInfo) bool {
	for _, pnic := range hConfig.Network.Pnic {
		macStr, err := conv.ParseMacAddr(pnic.Mac)
		if err != nil {
			continue
		}
		if netutils.IsPensandoMACAddress(macStr) {
			return true
		}
	}
	return false
}

func createVMWorkloadName(orchID, namespace, objName string) string {
	return fmt.Sprintf("%s", utils.CreateGlobalKey(orchID, namespace, objName))
}

func createVmkWorkloadName(orchID, namespace, objName string) string {
	return fmt.Sprintf("%s%s%s", defs.VmkWorkloadPrefix, utils.Delim, utils.CreateGlobalKey(orchID, namespace, objName))
}

func createVmkWorkloadNameFromHostName(hostName string) string {
	return fmt.Sprintf("%s%s%s", defs.VmkWorkloadPrefix, utils.Delim, hostName)
}

func createHostName(orchID, namespace, objName string) string {
	return fmt.Sprintf("%s", utils.CreateGlobalKey(orchID, namespace, objName))
}
