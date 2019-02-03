//-----------------------------------------------------------------------------
// {C} Copyright 2017 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------

package cmd

import (
	"encoding/json"
	"fmt"
	"strings"

	"github.com/spf13/cobra"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/api/generated/cluster"
	"github.com/pensando/sw/nic/agent/nmd/protos"
	vldtor "github.com/pensando/sw/venice/utils/apigen/validators"
)

var modeManagedCmd = &cobra.Command{
	Use:   "mode",
	Short: "Set Naples to Managed mode",
	Long:  "\n----------------------------\n Set NAPLES management mode \n----------------------------\n",
	RunE:  modeManagedCmdHandler,
	Args:  modeManagedCmdArgsValidator,
}

var modeManagedShowCmd = &cobra.Command{
	Use:    "mode",
	Short:  "Show mode of operation of Naples",
	Long:   "\n-------------------------------------------------------------------\n Show mode of operation of Naples - host-managed vs venice-managed \n-------------------------------------------------------------------\n",
	RunE:   modeManagedShowCmdHandler,
	Hidden: true,
}

type clusterAddresses []string

var controllers []string
var mode string
var networkMode string
var priMac string
var hostname string
var mgmtIP string
var defaultGW string
var featureProfile string
var dnsServers []string

func init() {
	updateCmd.AddCommand(modeManagedCmd)
	showCmd.AddCommand(modeManagedShowCmd)

	modeManagedCmd.Flags().StringSliceVarP(&controllers, "controllers", "c", make([]string, 0), "List of controller IP addresses or hostnames")
	modeManagedCmd.Flags().StringVarP(&mode, "management-mode", "o", "network", "host or network managed")
	modeManagedCmd.Flags().StringVarP(&networkMode, "network-mode", "k", "oob", "oob or inband")
	modeManagedCmd.Flags().StringVarP(&priMac, "primary-mac", "p", "", "Primary mac")
	modeManagedCmd.Flags().StringVarP(&hostname, "hostname", "n", "", "Host name")
	modeManagedCmd.Flags().StringVarP(&mgmtIP, "mgmt-ip", "m", "", "Management IP in CIDR format")
	modeManagedCmd.Flags().StringVarP(&defaultGW, "default-gw", "g", "", "Default GW for mgmt")
	modeManagedCmd.Flags().StringVarP(&featureProfile, "feature-profile", "f", "", "none, default or eth-dev-scale")
	modeManagedCmd.Flags().StringSliceVarP(&dnsServers, "dns-servers", "d", make([]string, 0), "List of DNS servers")
}

func modeManagedCmdArgsValidator(cmd *cobra.Command, args []string) error {
	if cmd.Flags().Changed("controllers") {
		for _, cluster := range controllers {
			ep := strings.Split(cluster, ":")
			if len(ep) > 1 {
				return fmt.Errorf("only hostnames need to be specified as controllers: '%s'", cluster)
			}
			if vldtor.HostAddr(ep[0]) != true {
				return fmt.Errorf("Not valid hostaddr: %s", ep[0])
			}
		}
	} else {
		controllers = []string{""}
	}

	if !cmd.Flags().Changed("management-mode") {
		return fmt.Errorf("Valid mode needed - must be '%s' or '%s'", "network", "host")
	}

	if mode != "network" && mode != "host" {
		return fmt.Errorf("Not valid mode: %s - must be '%s' or '%s'", mode, "network", "host")
	}
	if mode == "network" {
		if !cmd.Flags().Changed("network-mode") {
			return fmt.Errorf("Network-Mode must be specified if Management-Mode is 'network'")
		}
	}
	if mode == "host" {
		if cmd.Flags().Changed("network-mode") {
			return fmt.Errorf("Network-Mode is not applicable if Management-Mode is 'host'")
		}
		networkMode = ""
	}
	if cmd.Flags().Changed("network-mode") {
		if networkMode != "oob" && networkMode != "inband" {
			return fmt.Errorf("Invalid network mode '%s', must be 'oob' for out of band or 'inband' for inband network management", networkMode)
		}
	}
	if cmd.Flags().Changed("mgmt-ip") {
		if !vldtor.CIDR(mgmtIP) {
			return fmt.Errorf("Not valid mgmt-ip")
		}
	} else {
		mgmtIP = ""
	}
	if cmd.Flags().Changed("hostname") {
		if !vldtor.HostAddr(hostname) {
			return fmt.Errorf("Not valid host name")
		}
	} else {
		hostname = ""
	}
	if cmd.Flags().Changed("primary-mac") {
		if !vldtor.MacAddr(priMac) {
			return fmt.Errorf("Not valid mac address")
		}
	} else {
		priMac = ""
	}
	return nil
}

func modeManagedCmdHandler(cmd *cobra.Command, args []string) error {
	modeVal := nmd.MgmtMode_HOST
	if mode == "network" {
		modeVal = nmd.MgmtMode_NETWORK
	}

	networkModeVal := nmd.NetworkMode_NONE
	if networkMode == "inband" {
		networkModeVal = nmd.NetworkMode_INBAND
	} else if networkMode == "oob" {
		networkModeVal = nmd.NetworkMode_OOB
	}

	featureVal := nmd.NaplesSpec_NONE
	switch featureProfile {
	case "default":
		featureVal = nmd.NaplesSpec_CLASSIC_DEFAULT
		break
	case "eth-dev-scale":
		featureVal = nmd.NaplesSpec_CLASSIC_ETH_DEV_SCALE
		break
	default:
		return fmt.Errorf("Not valid feature profile")
	}

	naplesCfg := &nmd.Naples{
		TypeMeta:   api.TypeMeta{Kind: "Naples"},
		ObjectMeta: api.ObjectMeta{Name: "NaplesConfig"},
		Spec: nmd.NaplesSpec{
			Mode:        modeVal,
			NetworkMode: networkModeVal,
			Hostname:    hostname,
			Controllers: controllers,
			IPConfig: &cluster.IPConfig{
				IPAddress:  mgmtIP,
				DefaultGW:  defaultGW,
				DNSServers: dnsServers,
			},
			PrimaryMAC: priMac,
			Profile:    featureVal,
		},
	}

	err := restPost(naplesCfg, "api/v1/naples/")
	if err != nil {
		if strings.Contains(err.Error(), "EOF") {
			if verbose {
				fmt.Println("success")
			}
		} else {
			fmt.Println("Unable to set mode.")
			fmt.Println("Error:", err.Error())
		}
		return err
	}
	return nil
}

func modeManagedShowCmdHandler(cmd *cobra.Command, args []string) error {
	resp, err := restGet("api/v1/naples/")

	if err != nil {
		if verbose {
			fmt.Println(err.Error())
		}
		return err
	}
	naplesCfg := nmd.Naples{}
	json.Unmarshal(resp, &naplesCfg)
	if tabularFormat {
		fmt.Println(strings.Trim(strings.Replace(fmt.Sprintf("%+v", naplesCfg.Spec), " ", "\n", -1), "{}"))
	}
	if verbose && tabularFormat {
		fmt.Printf("%+v\n", naplesCfg.Spec)
	}
	return nil
}
