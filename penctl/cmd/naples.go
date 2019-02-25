//-----------------------------------------------------------------------------
// {C} Copyright 2017 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------

package cmd

import (
	"encoding/json"
	"errors"
	"fmt"
	"io/ioutil"
	"net/http"
	"strings"

	"github.com/pensando/sw/api/generated/cluster"

	"github.com/spf13/cobra"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/nic/agent/nmd/protos"
	vldtor "github.com/pensando/sw/venice/utils/apigen/validators"
)

var naplesCmd = &cobra.Command{
	Use:   "naples",
	Short: "Set NAPLES Modes and Profiles",
	Long:  "\n----------------------------\n Set NAPLES configuration \n----------------------------\n",
	RunE:  naplesCmdHandler,
	Args:  naplesCmdValidator,
}

var naplesShowCmd = &cobra.Command{
	Use:   "naples",
	Short: "Show NAPLES Modes and Profiles",
	Long:  "\n-------------------------------------------------------------------\n Show NAPLES configuration \n-------------------------------------------------------------------\n",
	RunE:  naplesShowCmdHandler,
}

var naplesProfileCmd = &cobra.Command{
	Use:   "naples-profile",
	Short: "naples profile object",
	Long:  "\n----------------------------\n Create NAPLES Profiles \n----------------------------\n",
	RunE:  naplesProfileCreateCmdHandler,
	Args:  naplesProfileCreateCmdValidator,
}

var naplesProfileShowCmd = &cobra.Command{
	Use:   "naples-profiles",
	Short: "Show Available NAPLES Profiles",
	Long:  "\n----------------------------\n Show NAPLES Profiles \n----------------------------\n",
	RunE:  naplesProfileShowCmdHandler,
}

var numLifs int32
var controllers []string
var managedBy, managementNetwork, priMac, hostname, mgmtIP, defaultGW, naplesProfile, profileName string
var dnsServers []string

func init() {
	updateCmd.AddCommand(naplesCmd)
	showCmd.AddCommand(naplesShowCmd, naplesProfileShowCmd)
	createCmd.AddCommand(naplesProfileCmd)

	naplesCmd.Flags().StringSliceVarP(&controllers, "controllers", "c", make([]string, 0), "List of controller IP addresses or hostnames")
	naplesCmd.Flags().StringVarP(&managedBy, "managed-by", "o", "host", "NAPLES Management. host or network")
	naplesCmd.Flags().StringVarP(&managementNetwork, "management-network", "k", "", "Management Network. inband or oob")
	naplesCmd.Flags().StringVarP(&priMac, "primary-mac", "p", "", "Primary mac")
	naplesCmd.Flags().StringVarP(&hostname, "hostname", "n", "", "Host name")
	naplesCmd.Flags().StringVarP(&mgmtIP, "mgmt-ip", "m", "", "Management IP in CIDR format")
	naplesCmd.Flags().StringVarP(&defaultGW, "default-gw", "g", "", "Default GW for mgmt")
	naplesCmd.Flags().StringVarP(&naplesProfile, "naples-profile", "f", "default", "Active NAPLES Profile")
	naplesCmd.Flags().StringSliceVarP(&dnsServers, "dns-servers", "d", make([]string, 0), "List of DNS servers")
	naplesProfileCmd.Flags().StringVarP(&profileName, "name", "n", "", "Name of the NAPLES profile")
	naplesProfileCmd.Flags().Int32VarP(&numLifs, "num-lifs", "i", 0, "Number of LIFs on the eth device. 1 or 16")
}

func naplesCmdHandler(cmd *cobra.Command, args []string) error {
	var networkManagementMode nmd.NetworkMode
	var managementMode nmd.MgmtMode
	var ok bool

	if managedBy == "host" {
		managementMode = nmd.MgmtMode_HOST
		managementNetwork = ""

		// check if profile exists.
		// TODO remove this when penctl can display server sent errors codes. This fails due to revproxy and only on verbose mode gives a non descript 500 Internal Server Error
		baseURL, _ := getNaplesURL()
		url := fmt.Sprintf("%s/api/v1/naples/profiles/", baseURL)

		resp, err := http.Get(url)
		if err != nil {
			fmt.Println("Failed to get existing profiles. Err: ", err)
		}
		defer resp.Body.Close()

		body, _ := ioutil.ReadAll(resp.Body)
		var profiles []*nmd.NaplesProfile
		json.Unmarshal(body, &profiles)

		for _, p := range profiles {
			if naplesProfile == p.Name {
				ok = true
			}
		}

		if !ok {
			return fmt.Errorf("invalid profile specified. %v", naplesProfile)
		}

	} else {
		managementMode = nmd.MgmtMode_NETWORK
	}

	if managementNetwork == "inband" {
		networkManagementMode = nmd.NetworkMode_INBAND
	} else if managementNetwork == "oob" {
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
			Mode:          managementMode.String(),
			NetworkMode:   networkManagementMode.String(),
			Controllers:   controllers,
			NaplesProfile: naplesProfile,
		},
	}

	err := restPost(naplesCfg, "api/v1/naples/")
	if err != nil {
		if strings.Contains(err.Error(), "EOF") {
			if verbose {
				fmt.Println("success")
			}
		} else {
			fmt.Println("Unable to update naples.")
			fmt.Println("Error:", err.Error())
		}
		return err
	}
	fmt.Println("Changes applied. Verify that penctl show naples command says REBOOT_PENDING, prior to performing a reboot.")
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
	var profiles []nmd.NaplesProfile
	json.Unmarshal(resp, &profiles)
	if verbose {
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
		err = errors.New("must specify a naples profile name")
		return
	}

	if !(numLifs == 1 || numLifs == 16) {
		err = errors.New("number of LIFs not supported. --num-lifs should either be 1 or 16")
	}
	return
}

func naplesCmdValidator(cmd *cobra.Command, args []string) (err error) {
	// Host Mode Validations
	switch managedBy {
	case "host":
		if len(managementNetwork) != 0 || len(controllers) != 0 || len(defaultGW) != 0 || len(dnsServers) != 0 || len(mgmtIP) != 0 || len(priMac) != 0 {
			err = errors.New("specified options, --managementNetwork, --controllers, --default-gw, --dns-servers, --mgmt-ip --primary-mac are not applicable when NAPLES is managed by host")
			return
		}

		if len(hostname) != 0 && !vldtor.HostAddr(hostname) {
			err = fmt.Errorf("invalid hostname %v", hostname)
			return
		}
		return

	case "network":
		if len(managementNetwork) == 0 {
			err = fmt.Errorf("network management mode needs an accompanying --management-network. Supported values are inband and oob")
		}

		if !(managementNetwork == "oob" || managementNetwork == "inband") {
			err = fmt.Errorf("invalid management network %v specified. Must be inband or oob", managementNetwork)
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

		if len(naplesProfile) != 0 && naplesProfile != "default" {
			err = fmt.Errorf("naples profile is not applicable when NAPLES is manged by network")
		}

		if len(hostname) != 0 && !vldtor.HostAddr(hostname) {
			err = fmt.Errorf("invalid hostname %v", hostname)
			return
		}
		return

	default:
		err = fmt.Errorf("invalid --managed-by  %v flag specified. Must be either host or network", managedBy)
		err = fmt.Errorf("invalid --managed-by  %v flag specified. Must be either host or network", managedBy)
		return
	}
}
