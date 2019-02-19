//-----------------------------------------------------------------------------
// {C} Copyright 2017 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------

package cmd

import (
	"encoding/json"
	"errors"
	"fmt"
	"strings"

	"github.com/pensando/sw/api/generated/cluster"

	"github.com/spf13/cobra"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/nic/agent/nmd/protos"
	vldtor "github.com/pensando/sw/venice/utils/apigen/validators"
)

var naplesCmd = &cobra.Command{
	Use:   "naples",
	Short: "Set NAPLES Modes and Feature Profiles",
	Long:  "\n----------------------------\n Set NAPLES configuration \n----------------------------\n",
	RunE:  naplesCmdHandler,
	Args:  naplesCmdValidator,
}

var naplesShowCmd = &cobra.Command{
	Use:   "naples",
	Short: "Show NAPLES Modes and Feature Profiles",
	Long:  "\n-------------------------------------------------------------------\n Show NAPLES configuration \n-------------------------------------------------------------------\n",
	RunE:  naplesShowCmdHandler,
}

var naplesProfileCmd = &cobra.Command{
	Use:   "profiles",
	Short: "profile object",
	Long:  "\n----------------------------\n Create NAPLES Profiles \n----------------------------\n",
	RunE:  naplesProfileCreateCmdHandler,
	Args:  naplesProfileCreateCmdValidator,
}

var naplesProfileShowCmd = &cobra.Command{
	Use:   "profiles",
	Short: "Show NAPLES",
	Long:  "\n----------------------------\n Show NAPLES Profiles \n----------------------------\n",
	RunE:  naplesProfileShowCmdHandler,
}

var numLifs int32
var controllers []string
var mode, networkMode, priMac, hostname, mgmtIP, defaultGW, featureProfile, profileName string
var dnsServers []string

func init() {
	updateCmd.AddCommand(naplesCmd)
	showCmd.AddCommand(naplesShowCmd, naplesProfileShowCmd)
	createCmd.AddCommand(naplesProfileCmd)

	naplesCmd.Flags().StringSliceVarP(&controllers, "controllers", "c", make([]string, 0), "List of controller IP addresses or hostnames")
	naplesCmd.Flags().StringVarP(&mode, "management-mode", "o", "host", "host or network managed")
	naplesCmd.Flags().StringVarP(&networkMode, "network-mode", "k", "", "oob or inband")
	naplesCmd.Flags().StringVarP(&priMac, "primary-mac", "p", "", "Primary mac")
	naplesCmd.Flags().StringVarP(&hostname, "hostname", "n", "", "Host name")
	naplesCmd.Flags().StringVarP(&mgmtIP, "mgmt-ip", "m", "", "Management IP in CIDR format")
	naplesCmd.Flags().StringVarP(&defaultGW, "default-gw", "g", "", "Default GW for mgmt")
	naplesCmd.Flags().StringVarP(&featureProfile, "feature-profile", "f", "", "Active NAPLES Profile")
	naplesCmd.Flags().StringSliceVarP(&dnsServers, "dns-servers", "d", make([]string, 0), "List of DNS servers")
	naplesProfileCmd.Flags().StringVarP(&profileName, "name", "n", "", "Name of the NAPLES profile")
	naplesProfileCmd.Flags().Int32VarP(&numLifs, "num-lifs", "i", 0, "Number of LIFs on the eth device. 1 or 16")
}

func naplesCmdHandler(cmd *cobra.Command, args []string) error {
	var networkManagementMode nmd.NetworkMode
	var managementMode nmd.MgmtMode

	if mode == "host" {
		managementMode = nmd.MgmtMode_HOST
	} else {
		managementMode = nmd.MgmtMode_NETWORK
	}

	if networkMode == "inband" {
		networkManagementMode = nmd.NetworkMode_INBAND
	} else {
		networkManagementMode = nmd.NetworkMode_OOB
	}

	naplesCfg := nmd.Naples{
		Spec: nmd.NaplesSpec{
			PrimaryMAC: priMac,
			Hostname:   hostname,
			IPConfig: &cluster.IPConfig{
				IPAddress:  mgmtIP,
				DefaultGW:  defaultGW,
				DNSServers: dnsServers,
			},
			Mode:        managementMode,
			NetworkMode: networkManagementMode,
			Controllers: controllers,
			Profile:     featureProfile,
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

func naplesShowCmdHandler(cmd *cobra.Command, args []string) error {
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

func naplesProfileShowCmdHandler(cmd *cobra.Command, args []string) error {
	resp, err := restGet("api/v1/naples/profiles/")

	if err != nil {
		if verbose {
			fmt.Println(err.Error())
		}
		return err
	}
	var profiles []nmd.NaplesProfile
	json.Unmarshal(resp, &profiles)
	if tabularFormat {
		for _, p := range profiles {
			fmt.Println(strings.Trim(strings.Replace(fmt.Sprintf("%+v", p.Spec), " ", "\n", -1), "{}"))
			fmt.Println("----")
		}
	}
	if verbose && tabularFormat {
		for _, p := range profiles {
			fmt.Println(strings.Trim(strings.Replace(fmt.Sprintf("%+v", p.Spec), " ", "\n", -1), "{}"))
			fmt.Println("----")
		}
	}
	return nil
}

func naplesProfileCreateCmdHandler(cmd *cobra.Command, args []string) error {
	profile := nmd.NaplesProfile{
		TypeMeta: api.TypeMeta{Kind: "NaplesProfile"},
		ObjectMeta: api.ObjectMeta{
			Name: profileName,
		},
		Spec: nmd.NaplesProfileSpec{
			NumLifs: numLifs,
		},
	}

	err := restPost(profile, "api/v1/naples/profiles/")
	if err != nil {
		if strings.Contains(err.Error(), "EOF") {
			if verbose {
				fmt.Println("success")
			}
		} else {
			fmt.Println("Unable to create profile.")
			fmt.Println("Error:", err.Error())
		}
		return err
	}
	return err
}

func naplesProfileCreateCmdValidator(cmd *cobra.Command, args []string) (err error) {
	if len(profileName) == 0 {
		err = errors.New("must specify a profile name")
		return
	}

	if !(numLifs == 1 || numLifs == 16) {
		err = errors.New("number of LIFs not supported. --num-lifs should either be 1 or 16")
	}
	return
}

func naplesCmdValidator(cmd *cobra.Command, args []string) (err error) {
	// Host Mode Validations
	switch mode {
	case "host":
		if len(featureProfile) == 0 {
			err = errors.New("host managed mode needs an accompanying feature profile. Specify it with --feature-profile")
			return
		}

		if len(networkMode) != 0 || len(controllers) != 0 || len(defaultGW) != 0 || len(dnsServers) != 0 || len(mgmtIP) != 0 || len(priMac) != 0 {
			err = errors.New("specified options, --networkMode, --controllers, --default-gw, --dns-servers, --mgmt-ip --primary-mac are not applicable when NAPLES is in host managed mode")
			return
		}

		if len(hostname) != 0 && !vldtor.HostAddr(hostname) {
			err = fmt.Errorf("invalid hostname %v", hostname)
			return
		}
		return

	case "network":
		if !(networkMode == "oob" || networkMode == "inband") {
			err = fmt.Errorf("invalid network management mode %v. Must be either inband or oob", networkMode)
			return
		}

		for _, c := range controllers {
			if !vldtor.HostAddr(c) {
				err = fmt.Errorf("invalid controller %v specified. Must be either IP Addresses or FQDNs", c)
				return
			}
		}

		if len(defaultGW) != 0 && !vldtor.CIDR(defaultGW) {
			err = fmt.Errorf("invalid default gateway %v. Must be in CIDR Format", defaultGW)
			return
		}

		for _, d := range dnsServers {
			if !vldtor.IPAddr(d) {
				err = fmt.Errorf("invalid dns server %v specified. Must be a valid IP Addresses", d)
				return
			}
		}

		if len(mgmtIP) != 0 && !vldtor.CIDR(mgmtIP) {
			err = fmt.Errorf("invalid management IP %v specified. Must be in CIDR Format", mgmtIP)
			return
		}

		if len(priMac) != 0 && !vldtor.MacAddr(priMac) {
			err = fmt.Errorf("invalid MAC Address %v specified", priMac)
			return
		}

		if len(featureProfile) != 0 {
			err = fmt.Errorf("feature profile is not applicable when NAPLES is in network managed mode ")
		}

		if len(hostname) != 0 && !vldtor.HostAddr(hostname) {
			err = fmt.Errorf("invalid hostname %v", hostname)
			return
		}
		return

	default:
		err = fmt.Errorf("invalid management mode %v. Must be either host or network", mode)
		return
	}
}
