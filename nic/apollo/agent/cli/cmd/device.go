//-----------------------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------

package cmd

import (
	"context"
	"fmt"
	"reflect"
	"strings"

	"github.com/spf13/cobra"
	yaml "gopkg.in/yaml.v2"

	"github.com/pensando/sw/nic/apollo/agent/cli/utils"
	"github.com/pensando/sw/nic/apollo/agent/gen/pds"
)

var (
	memoryProfile string
	deviceProfile string
	deviceTimeout uint32
)

var deviceShowCmd = &cobra.Command{
	Use:   "device",
	Short: "show Device information",
	Long:  "show Device object information",
	Run:   deviceShowCmdHandler,
}

var deviceUpdateCmd = &cobra.Command{
	Use:   "device",
	Short: "update Device information",
	Long:  "update Device object information",
	Run:   deviceUpdateCmdHandler,
}

func init() {
	showCmd.AddCommand(deviceShowCmd)
	deviceShowCmd.Flags().Bool("yaml", false, "Output in yaml")
	deviceShowCmd.Flags().Bool("detail", false, "Display detailed output")

	debugCmd.AddCommand(deviceUpdateCmd)
	deviceUpdateCmd.Flags().StringVar(&memoryProfile, "memory-profile", "default", "Specify memory profile (Ex: default)")
	deviceUpdateCmd.Flags().StringVar(&deviceProfile, "device-profile", "default", "Specify device profile (Ex: default, 2pf, 3pf, 4pf, 5pf, 6pf, 7pf and 8pf)")
	deviceUpdateCmd.Flags().Uint32Var(&deviceTimeout, "learn-age-timeout", 300, "Specify device aging timeout for learned MAC/IP in secs (Valid: 30-86400)")
}

func deviceUpdateCmdHandler(cmd *cobra.Command, args []string) {
	// Connect to PDS
	c, err := utils.CreateNewGRPCClient()
	if err != nil {
		fmt.Printf("Could not connect to the PDS, is PDS running?\n")
		return
	}
	defer c.Close()

	if len(args) > 0 {
		fmt.Printf("Invalid argument\n")
		return
	}

	if cmd.Flags().Changed("memory-profile") == false &&
		cmd.Flags().Changed("device-profile") == false &&
		cmd.Flags().Changed("learn-age-timeout") == false {
		fmt.Printf("No arguments specified, refer to help string\n")
		return
	}

	client := pds.NewDeviceSvcClient(c)
	req := &pds.DeviceGetRequest{}

	// PDS call
	resp, err := client.DeviceGet(context.Background(), req)
	if err != nil {
		fmt.Printf("Getting Device failed, err %v\n", err)
		return
	}

	spec := resp.GetResponse().GetSpec()

	var updateSpec *pds.DeviceSpec
	if cmd.Flags().Changed("memory-profile") {
		if isValidMemoryProfile(memoryProfile) == false {
			fmt.Printf("Invalid memory profile specified, only \"default\" is supported\n")
			return
		}
		updateSpec = &pds.DeviceSpec{
			IPAddr:           spec.GetIPAddr(),
			MACAddr:          spec.GetMACAddr(),
			GatewayIP:        spec.GetGatewayIP(),
			DevOperMode:      spec.GetDevOperMode(),
			BridgingEn:       spec.GetBridgingEn(),
			LearningEn:       spec.GetLearningEn(),
			OverlayRoutingEn: spec.GetOverlayRoutingEn(),
			LearnAgeTimeout:  spec.GetLearnAgeTimeout(),
			DeviceProfile:    spec.GetDeviceProfile(),
			MemoryProfile:    inputToMemoryProfile(memoryProfile),
		}
	} else if cmd.Flags().Changed("device-profile") {
		if isValidDeviceProfile(deviceProfile) == false {
			fmt.Printf("Invalid device profile specified, must be one of default, 2pf, 3pf, 4pf, 5pf, 6pf, 7pf & 8pf\n")
			return
		}
		updateSpec = &pds.DeviceSpec{
			IPAddr:           spec.GetIPAddr(),
			MACAddr:          spec.GetMACAddr(),
			GatewayIP:        spec.GetGatewayIP(),
			DevOperMode:      spec.GetDevOperMode(),
			BridgingEn:       spec.GetBridgingEn(),
			LearningEn:       spec.GetLearningEn(),
			OverlayRoutingEn: spec.GetOverlayRoutingEn(),
			LearnAgeTimeout:  spec.GetLearnAgeTimeout(),
			MemoryProfile:    spec.GetMemoryProfile(),
			DeviceProfile:    inputToDeviceProfile(deviceProfile),
		}
	} else if cmd.Flags().Changed("learn-age-timeout") {
		if deviceTimeout < 30 || deviceTimeout > 86400 {
			fmt.Printf("Invalid learning age timeout specified, valid range 30-86400")
			return
		}
		updateSpec = &pds.DeviceSpec{
			IPAddr:           spec.GetIPAddr(),
			MACAddr:          spec.GetMACAddr(),
			GatewayIP:        spec.GetGatewayIP(),
			DevOperMode:      spec.GetDevOperMode(),
			BridgingEn:       spec.GetBridgingEn(),
			LearningEn:       spec.GetLearningEn(),
			OverlayRoutingEn: spec.GetOverlayRoutingEn(),
			MemoryProfile:    spec.GetMemoryProfile(),
			DeviceProfile:    spec.GetDeviceProfile(),
			LearnAgeTimeout:  deviceTimeout,
		}
	}

	updateReq := &pds.DeviceRequest{
		Request: updateSpec,
	}

	updateResp, updateErr := client.DeviceUpdate(context.Background(), updateReq)
	if updateErr != nil {
		fmt.Printf("Device update failed, err %v\n", updateErr)
	}

	if updateResp.ApiStatus != pds.ApiStatus_API_STATUS_OK {
		fmt.Printf("Operation failed with %v error\n", updateResp.ApiStatus)
		return
	}

	fmt.Printf("Device update succeeded.\n")
}

func isValidMemoryProfile(str string) bool {
	str = strings.ToLower(str)

	if strings.Compare(str, "default") == 0 {
		return true
	}

	return false
}

func inputToMemoryProfile(str string) pds.MemoryProfile {
	str = strings.ToLower(str)

	if strings.Compare(str, "default") == 0 {
		return pds.MemoryProfile_MEMORY_PROFILE_DEFAULT
	} else {
		return pds.MemoryProfile_MEMORY_PROFILE_DEFAULT
	}
}

func isValidDeviceProfile(str string) bool {
	str = strings.ToLower(str)

	if strings.Compare(str, "default") == 0 {
		return true
	} else if strings.Compare(str, "2pf") == 0 {
		return true
	} else if strings.Compare(str, "3pf") == 0 {
		return true
	} else if strings.Compare(str, "4pf") == 0 {
		return true
	} else if strings.Compare(str, "5pf") == 0 {
		return true
	} else if strings.Compare(str, "6pf") == 0 {
		return true
	} else if strings.Compare(str, "7pf") == 0 {
		return true
	} else if strings.Compare(str, "8pf") == 0 {
		return true
	}

	return false
}

func inputToDeviceProfile(str string) pds.DeviceProfile {
	str = strings.ToLower(str)

	if strings.Compare(str, "default") == 0 {
		return pds.DeviceProfile_DEVICE_PROFILE_DEFAULT
	} else if strings.Compare(str, "2pf") == 0 {
		return pds.DeviceProfile_DEVICE_PROFILE_2PF
	} else if strings.Compare(str, "3pf") == 0 {
		return pds.DeviceProfile_DEVICE_PROFILE_3PF
	} else if strings.Compare(str, "4pf") == 0 {
		return pds.DeviceProfile_DEVICE_PROFILE_4PF
	} else if strings.Compare(str, "5pf") == 0 {
		return pds.DeviceProfile_DEVICE_PROFILE_5PF
	} else if strings.Compare(str, "6pf") == 0 {
		return pds.DeviceProfile_DEVICE_PROFILE_6PF
	} else if strings.Compare(str, "7pf") == 0 {
		return pds.DeviceProfile_DEVICE_PROFILE_7PF
	} else if strings.Compare(str, "8pf") == 0 {
		return pds.DeviceProfile_DEVICE_PROFILE_8PF
	} else {
		return pds.DeviceProfile_DEVICE_PROFILE_DEFAULT
	}
}

func deviceShowCmdHandler(cmd *cobra.Command, args []string) {
	// Connect to PDS
	c, err := utils.CreateNewGRPCClient()
	if err != nil {
		fmt.Printf("Could not connect to the PDS, is PDS running?\n")
		return
	}
	defer c.Close()

	if len(args) > 0 {
		fmt.Printf("Invalid argument\n")
		return
	}

	client := pds.NewDeviceSvcClient(c)
	req := &pds.DeviceGetRequest{}

	// PDS call
	resp, err := client.DeviceGet(context.Background(), req)
	if err != nil {
		fmt.Printf("Getting Device failed, err %v\n", err)
		return
	}

	// Print Device
	if resp.ApiStatus != pds.ApiStatus_API_STATUS_OK {
		fmt.Printf("Operation failed with %v error\n", resp.ApiStatus)
		return
	}

	if cmd != nil && cmd.Flags().Changed("yaml") {
		respType := reflect.ValueOf(resp)
		b, _ := yaml.Marshal(respType.Interface())
		fmt.Println(string(b))
		fmt.Println("---")
	} else if cmd != nil && cmd.Flags().Changed("detail") {
		printDeviceDetail(resp)
	} else {
		printDeviceHeader()
		printDevice(resp)
	}
}

func printDeviceHeader() {
	hdrLine := strings.Repeat("-", 170)
	fmt.Println(hdrLine)
	fmt.Printf("%-16s%-20s%-16s%-12s%-12s%-12s%-12s%-16s%-10s%-18s%-20s%-6s\n",
		"IPAddr", "MACAddr", "GatewayIP",
		"MemProfile", "DevProfile", "BridgingEn", "LearningEn", "LearnAgeTimeout",
		"OperMode", "OverlayRoutingEn", "FRU MAC", "Memory")
	fmt.Println(hdrLine)
}

func printDevice(resp *pds.DeviceGetResponse) {
	spec := resp.GetResponse().GetSpec()
	if spec == nil {
		fmt.Println("-")
		return
	}

	status := resp.GetResponse().GetStatus()
	if status == nil {
		fmt.Println("-")
		return
	}

	memoryStr := fmt.Sprintf("%dG", status.GetMemory())
	fmt.Printf("%-16s%-20s%-16s%-12s%-12s%-12t%-12t%-16d%-10s%-18t%-20s%-6s\n",
		utils.IPAddrToStr(spec.GetIPAddr()),
		utils.MactoStr(spec.GetMACAddr()),
		utils.IPAddrToStr(spec.GetGatewayIP()),
		strings.Replace(spec.GetMemoryProfile().String(), "MEMORY_PROFILE_", "", -1),
		strings.Replace(spec.GetDeviceProfile().String(), "DEVICE_PROFILE_", "", -1),
		spec.GetBridgingEn(), spec.GetLearningEn(), spec.GetLearnAgeTimeout(),
		strings.Replace(spec.GetDevOperMode().String(), "DEVICE_OPER_MODE_", "", -1),
		spec.GetOverlayRoutingEn(), utils.MactoStr(status.GetSystemMACAddress()),
		memoryStr)
}

func printDeviceDetail(resp *pds.DeviceGetResponse) {
	spec := resp.GetResponse().GetSpec()
	if spec == nil {
		fmt.Println("-")
		return
	}

	status := resp.GetResponse().GetStatus()
	if status == nil {
		fmt.Println("-")
		return
	}
	fmt.Printf("%-26s : %s\n", "IP Address", utils.IPAddrToStr(spec.GetIPAddr()))
	fmt.Printf("%-26s : %s\n", "MAC Address", utils.MactoStr(spec.GetMACAddr()))
	fmt.Printf("%-26s : %s\n", "Gateway IP", utils.IPAddrToStr(spec.GetGatewayIP()))
	fmt.Printf("%-26s : %s\n", "Memory Profile",
		strings.Replace(spec.GetMemoryProfile().String(), "MEMORY_PROFILE_", "", -1))
	fmt.Printf("%-26s : %s\n", "Device Profile",
		strings.Replace(spec.GetDeviceProfile().String(), "DEVICE_PROFILE_", "", -1))
	fmt.Printf("%-26s : %t\n", "Bridging Enabled", spec.GetBridgingEn())
	fmt.Printf("%-26s : %t\n", "Learning Enabled", spec.GetLearningEn())
	fmt.Printf("%-26s : %d\n", "Learn Age Timeout (sec)", spec.GetLearnAgeTimeout())
	fmt.Printf("%-26s : %s\n", "Device Oper Mode",
		strings.Replace(spec.GetDevOperMode().String(), "DEVICE_OPER_MODE_", "", -1))
	fmt.Printf("%-26s : %t\n", "Overlay Routing Enabled", spec.GetOverlayRoutingEn())
	fmt.Printf("%-26s : %s\n", "FRU MAC Address", utils.MactoStr(status.GetSystemMACAddress()))
	fmt.Printf("%-26s : %dG\n", "Memory", status.GetMemory())
	fmt.Printf("%-26s : %s\n", "Manufacturing Date", status.GetManufacturingDate())
	fmt.Printf("%-26s : %s\n", "Product Name", status.GetProductName())
	fmt.Printf("%-26s : %s\n", "Serial Number", status.GetSerialNumber())
	fmt.Printf("%-26s : %s\n", "Part Number", status.GetSku())
	fmt.Printf("%-26s : %d\n", "IP Mapping Priority", spec.GetIPMappingPriority())
}
