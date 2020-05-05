//-----------------------------------------------------------------------------
// {C} Copyright 2017 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------

package cmd

import (
	"encoding/json"
	"errors"
	"fmt"
	"io/ioutil"
	"net"
	"strings"

	"github.com/pensando/sw/api/generated/cluster"
	"github.com/pensando/sw/nic/agent/protos/nmd"

	"github.com/spf13/cobra"

	vldtor "github.com/pensando/sw/venice/utils/apigen/validators"
	"github.com/pensando/sw/venice/utils/strconv"
)

var naplesCmd = &cobra.Command{
	Use:     "dsc",
	Aliases: []string{"naples"},
	Short:   "Set Distributed Service Card Modes and Profiles",
	Long:    "\n----------------------------\n Set Distributed Service Card configuration \n----------------------------\n",
	RunE:    naplesCmdHandler,
	Args:    naplesCmdValidator,
}

var naplesShowCmd = &cobra.Command{
	Use:     "dsc",
	Aliases: []string{"naples"},
	Short:   "Show Distributed Service Card Modes and Profiles",
	Long:    "\n-------------------------------------------------------------------\n Show Distributed Service Card configuration \n-------------------------------------------------------------------\n",
	Args:    cobra.NoArgs,
	RunE:    naplesShowCmdHandler,
}

var naplesProfileShowCmd = &cobra.Command{
	Use:     "dsc-profiles",
	Aliases: []string{"naples-profiles"},
	Short:   "Show Available Distributed Service Card Profiles",
	Long:    "\n----------------------------\n Show Distributed Service Card Profiles \n----------------------------\n",
	Args:    cobra.NoArgs,
	RunE:    naplesProfileShowCmdHandler,
}

var naplesProfileUpdateCmd = &cobra.Command{
	Use:   "dsc-profile",
	Short: "Distributed Service Card profile object",
	Long:  "\n----------------------------\n Update Distributed Service Card Profiles \n----------------------------\n",
	RunE:  naplesProfileUpdateCmdHandler,
	Args:  naplesProfileUpdateCmdValidator,
}

var controllers []string
var managedBy, managementNetwork, priMac, id, mgmtIP, defaultGW, naplesProfile, profileName, portDefault, bondIP string
var dnsServers []string

func init() {
	updateCmd.AddCommand(naplesCmd)
	showCmd.AddCommand(naplesShowCmd, naplesProfileShowCmd)
	updateCmd.AddCommand(naplesProfileUpdateCmd)

	naplesCmd.Flags().StringSliceVarP(&controllers, "controllers", "c", make([]string, 0), "List of controller IP addresses or ids")
	naplesCmd.Flags().StringVarP(&managedBy, "managed-by", "o", "host", "Distributed Service Card Management. host or network")
	naplesCmd.Flags().StringVarP(&managementNetwork, "management-network", "k", "", "Management Network. inband or oob")
	naplesCmd.Flags().StringVarP(&priMac, "primary-mac", "p", "", "Primary mac")
	naplesCmd.Flags().StringVarP(&id, "id", "i", "", "DSC ID")
	naplesCmd.Flags().StringVarP(&mgmtIP, "mgmt-ip", "m", "", "Management IP in CIDR format")
	naplesCmd.Flags().StringVarP(&defaultGW, "default-gw", "g", "", "Default GW for mgmt")
	naplesCmd.Flags().StringVarP(&bondIP, "inband-ip", "", "", "Inband IP in CIDR format")
	naplesCmd.Flags().StringVarP(&naplesProfile, "dsc-profile", "s", "FEATURE_PROFILE_BASE", "Active Distributed Service Card Profile")
	naplesCmd.Flags().StringVarP(&naplesProfile, "naples-profile", "f", "FEATURE_PROFILE_BASE", "Active Distributed Service Card Profile")
	naplesCmd.Flags().StringSliceVarP(&dnsServers, "dns-servers", "d", make([]string, 0), "List of DNS servers")

	naplesProfileUpdateCmd.Flags().StringVarP(&profileName, "name", "n", "", "Name of the Distributed Service Card profile to be created")
	naplesProfileUpdateCmd.Flags().StringVarP(&portDefault, "port-default", "p", "enable", "Set default port admin state for next reboot. (enable | disable)")
}

func naplesCmdHandler(cmd *cobra.Command, args []string) error {
	var networkManagementMode nmd.NetworkMode
	var managementMode nmd.MgmtMode

	if managedBy == "host" {
		managementMode = nmd.MgmtMode_HOST
		managementNetwork = ""

		// check if profile exists.
		if err := checkProfileExists(naplesProfile); err != nil {
			return err
		}

	} else {
		managementMode = nmd.MgmtMode_NETWORK
		// TODO : Remove this check once we have means to automatically generated a
		// id on Naples, if id is not explicitly passed for network managed mode.
		if id == "" {
			return errors.New("id parameter cannot be empty")
		}
	}

	if managementNetwork == "inband" {
		networkManagementMode = nmd.NetworkMode_INBAND
	} else if managementNetwork == "oob" {
		networkManagementMode = nmd.NetworkMode_OOB
	}

	var inbIPCfg *cluster.IPConfig
	if bondIP == "" {
		inbIPCfg = nil
	} else {
		inbIPCfg = &cluster.IPConfig{
			IPAddress: bondIP,
		}
	}

	naplesCfg := nmd.DistributedServiceCard{
		Spec: nmd.DistributedServiceCardSpec{
			PrimaryMAC: priMac,
			ID:         id,
			IPConfig: &cluster.IPConfig{
				IPAddress:  mgmtIP,
				DefaultGW:  defaultGW,
				DNSServers: dnsServers,
			},
			InbandIPConfig: inbIPCfg,
			Mode:           managementMode.String(),
			NetworkMode:    networkManagementMode.String(),
			Controllers:    controllers,
			DSCProfile:     naplesProfile,
		},
	}

	_, err := restPost(naplesCfg, "api/v1/naples/")
	if err != nil {
		if strings.Contains(err.Error(), "EOF") {
			if verbose {
				fmt.Println("success")
			}
		} else {
			fmt.Printf("Unable to update dsc, error: %v\n", err)
			return err
		}
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
	naplesCfg := nmd.DistributedServiceCard{}
	json.Unmarshal(resp, &naplesCfg)
	if verbose {
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
	var profiles []nmd.DSCProfile
	json.Unmarshal(resp, &profiles)
	if verbose {
		for _, p := range profiles {
			fmt.Println(strings.Trim(strings.Replace(fmt.Sprintf("%+v", p.Spec), " ", "\n", -1), "{}"))
			fmt.Println("----")
		}
	}
	return nil
}

func naplesProfileUpdateCmdValidator(cmd *cobra.Command, args []string) (err error) {
	if len(profileName) == 0 {
		err = errors.New("must specify a dsc profile name")
		return
	}

	if portDefault != "enable" && portDefault != "disable" {
		err = errors.New("invalid port default admin state passed")
	}

	return
}

func naplesProfileUpdateCmdHandler(cmd *cobra.Command, args []string) error {
	var portState string

	// check if currently attached profile is being updated
	if err := checkAttachedProfile(profileName); err != nil {
		fmt.Printf("%v is currently applied, all changes will be applicable after reboot only\n", profileName)
	}

	if portDefault == "enable" {
		portState = nmd.PortAdminState_PORT_ADMIN_STATE_ENABLE.String()
	} else if portDefault == "disable" {
		portState = nmd.PortAdminState_PORT_ADMIN_STATE_DISABLE.String()
	}

	profile := getNaplesProfile(profileName)
	if profile == nil {
		return fmt.Errorf("the selected profile is not supported by the DSC")
	}

	profile.Spec.DefaultPortAdmin = portState

	err := restPut(profile, "api/v1/naples/profiles/")
	if err != nil {
		if strings.Contains(err.Error(), "EOF") {
			if verbose {
				fmt.Println("success")
			}
		} else {
			fmt.Println("Unable to update profile.")
			fmt.Println("Error:", err.Error())
		}
		return err
	}
	return err
}

func naplesCmdValidator(cmd *cobra.Command, args []string) (err error) {
	if len(args) != 0 {
		err = fmt.Errorf("unknown command options specified: %+v", args)
		return
	}
	// Host Mode Validations
	switch managedBy {
	case "host":
		if len(managementNetwork) != 0 || len(controllers) != 0 || len(defaultGW) != 0 || len(dnsServers) != 0 || len(mgmtIP) != 0 || len(priMac) != 0 {
			err = errors.New("specified options, --management-network, --controllers, --default-gw, --dns-servers, --mgmt-ip --primary-mac are not applicable when Distributed Service Card is managed by host")
			return
		}

		if len(id) != 0 {
			vErr := vldtor.HostAddr(id)
			if vErr != nil {
				err = fmt.Errorf("invalid id %v: %s", id, vErr.Error())
				return
			}
		}
		return

	case "network":
		// Variable to check if an IP address falls in the same subnet as the mgmt IP.
		checkSubnet := false
		var mgmtNet *net.IPNet

		if len(managementNetwork) == 0 {
			err = fmt.Errorf("network management mode needs an accompanying --management-network. Supported values are inband and oob")
		}

		if !(managementNetwork == "oob" || managementNetwork == "inband") {
			err = fmt.Errorf("invalid management network %v specified. Must be inband or oob", managementNetwork)
			return
		}

		if len(mgmtIP) != 0 {
			if len(controllers) == 0 {
				err = fmt.Errorf("controllers must be passed when statically configuring management IP. Use --controllers option")
				return
			}

			if vldtor.CIDR(mgmtIP) == nil {
				if len(defaultGW) == 0 {
					checkSubnet = true
					_, mgmtNet, _ = net.ParseCIDR(mgmtIP)
				}
			} else {
				err = fmt.Errorf("invalid management IP %v specified. Must be in CIDR Format", mgmtIP)
				return
			}
		}

		if len(bondIP) != 0 {
			if managementNetwork == "inband" {
				err = fmt.Errorf("cannot configure ip on inb when it is also mgmt network")
				return
			}

			if vldtor.CIDR(bondIP) != nil {
				err = fmt.Errorf("invalid management IP %v specified. Must be in CIDR Format", mgmtIP)
				return
			}
		}

		for _, c := range controllers {
			if vldtor.HostAddr(c) != nil {
				err = fmt.Errorf("invalid controller %v specified. Must be either IP Addresses or FQDNs", c)
				return
			} else if checkSubnet {
				controllerIP := net.ParseIP(c)
				if !mgmtNet.Contains(controllerIP) {
					err = fmt.Errorf("controller %v is not in the same subnet as the Management IP %v. Add default gateway using --default-gw option", c, mgmtIP)
					return
				}
			}
		}

		if len(defaultGW) != 0 && vldtor.IPAddr(defaultGW) != nil {
			err = fmt.Errorf("invalid default gateway %v", defaultGW)
			return
		}

		for _, d := range dnsServers {
			if vldtor.IPAddr(d) != nil {
				err = fmt.Errorf("invalid dns server %v specified. Must be a valid IP Addresses", d)
				return
			}
		}

		if len(priMac) != 0 {
			priMac, err = strconv.ParseMacAddr(priMac)
			if err != nil {
				err = fmt.Errorf("invalid MAC Address %v specified. Conversion failed. Err : %v", priMac, err)
				return
			}
		}

		if len(naplesProfile) != 0 && naplesProfile != "FEATURE_PROFILE_BASE" {
			err = fmt.Errorf("dsc profile is not applicable when Distributed Service Card is manged by network")
		}

		if len(id) != 0 {
			vErr := vldtor.HostAddr(id)
			if vErr != nil {
				err = fmt.Errorf("invalid id %v: %s", id, vErr.Error())
				return
			}
			return
		}
		return

	default:
		err = fmt.Errorf("invalid --managed-by  %v flag specified. Must be either host or network", managedBy)
		err = fmt.Errorf("invalid --managed-by  %v flag specified. Must be either host or network", managedBy)
		return
	}
}

func getNaplesProfile(profileName string) *nmd.DSCProfile {
	baseURL, _ := getNaplesURL()
	url := fmt.Sprintf("%s/api/v1/naples/profiles/", baseURL)

	resp, err := penHTTPClient.Get(url)
	if err != nil {
		if strings.Contains(err.Error(), httpsSignature) {
			err = fmt.Errorf("DSC is part of a cluster, authentication token required")
		}
		fmt.Println("Failed to get existing profiles. Err: ", err)
		return nil
	}
	defer resp.Body.Close()

	body, _ := ioutil.ReadAll(resp.Body)
	var profiles []*nmd.DSCProfile
	json.Unmarshal(body, &profiles)

	for _, p := range profiles {
		if profileName == p.Name {
			return p
		}
	}

	return nil
}

// TODO remove this when penctl can display server sent errors codes. This fails due to revproxy and only on verbose mode gives a non descript 500 Internal Server Error
func checkProfileExists(profileName string) error {
	var ok bool
	baseURL, _ := getNaplesURL()
	url := fmt.Sprintf("%s/api/v1/naples/profiles/", baseURL)

	resp, err := penHTTPClient.Get(url)
	if err != nil {
		if strings.Contains(err.Error(), httpsSignature) {
			err = fmt.Errorf("DSC is part of a cluster, authentication token required")
		}
		fmt.Println("Failed to get existing profiles. Err: ", err)
		return fmt.Errorf("Failed to get existing profiles. Err: %v", err)
	}
	defer resp.Body.Close()

	body, _ := ioutil.ReadAll(resp.Body)
	var profiles []*nmd.DSCProfile
	json.Unmarshal(body, &profiles)

	for _, p := range profiles {
		if profileName == p.Name {
			ok = true
			break
		}
	}

	if !ok {
		return fmt.Errorf("specified profile %v doesn't exist", profileName)
	}

	return nil
}

func checkAttachedProfile(profileName string) error {
	baseURL, _ := getNaplesURL()
	url := fmt.Sprintf("%s/api/v1/naples/", baseURL)

	resp, err := penHTTPClient.Get(url)
	if err != nil {
		if strings.Contains(err.Error(), httpsSignature) {
			err = fmt.Errorf("DSC is part of a cluster, authentication token required")
		}
		fmt.Println("Failed to get existing profiles. Err: ", err)
		return fmt.Errorf("failed to get existing profiles. Err: %v", err)
	}
	defer resp.Body.Close()

	body, _ := ioutil.ReadAll(resp.Body)
	var naplesCfg nmd.DistributedServiceCard
	json.Unmarshal(body, &naplesCfg)

	if naplesCfg.Spec.DSCProfile == profileName {
		return fmt.Errorf("dsc profile %v is currently in use", profileName)
	}

	return nil
}
