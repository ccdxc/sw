//-----------------------------------------------------------------------------
// {C} Copyright 2017 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------

package cmd

import (
	"encoding/json"
	"fmt"
	"strings"
	"time"

	"github.com/spf13/cobra"

	cluster "github.com/pensando/sw/api/generated/cluster"
	nmd "github.com/pensando/sw/nic/agent/protos/nmd"
)

var getSysMemCmd = &cobra.Command{
	Use:   "system-memory-usage",
	Short: "Show free/used memory on Naples (in MB)",
	Long:  "\n-----------------------------------------\n Show Free/Used Memory on Naples (in MB)\n-----------------------------------------\n",
	RunE:  getSysMemCmdHandler,
}

var getProcMemInfoCmd = &cobra.Command{
	Use:   "proc-meminfo",
	Short: "Check /proc/meminfo file on Naples",
	Long:  "\n------------------------------------\n Check /proc/meminfo file on Naples \n------------------------------------\n",
	RunE:  getProcMemInfoCmdHandler,
}

var getSystemCmd = &cobra.Command{
	Use:   "system",
	Short: "show system information",
	Long:  "\n------------------------------------\n show system information \n------------------------------------\n",
}

var getSystemStatusCmd = &cobra.Command{
	Use:   "status",
	Short: "show current system status",
	Long:  "\n------------------------------------\n show current system status \n------------------------------------\n",
	RunE:  getSystemStatusCmdCmdHandler,
}

var getSystemQueueStatsCmd = &cobra.Command{
	Use:   "queue-statistics",
	Short: "show system queue-statistics",
	Long:  "\n------------------------------------\n show system queue-statistics \n------------------------------------\n",
	Run:   getSystemQueueStatsCmdHandler,
}

var getSmartNicCmd = &cobra.Command{
	Use:   "smartnic",
	Short: "Shows the SmartNIC object",
	Long:  "\n-----------------------------------------\n Shows current Spec and Status of SmartNIC object \n-----------------------------------------\n",
	RunE:  getSmartNicCmdHandler,
}

func init() {
	showCmd.AddCommand(getSysMemCmd)
	showCmd.AddCommand(getProcMemInfoCmd)
	showCmd.AddCommand(getSystemCmd)
	showCmd.AddCommand(getSmartNicCmd)
	getSystemCmd.AddCommand(getSystemStatusCmd)
	getSystemCmd.AddCommand(getSystemQueueStatsCmd)
}

func getSysMemCmdHandler(cmd *cobra.Command, args []string) error {
	v := &nmd.NaplesCmdExecute{
		Executable: "free",
		Opts:       strings.Join([]string{"-m"}, ""),
	}
	return naplesExecCmd(v)
}

func getProcMemInfoCmdHandler(cmd *cobra.Command, args []string) error {
	v := &nmd.NaplesCmdExecute{
		Executable: "cat",
		Opts:       strings.Join([]string{"/proc/meminfo"}, ""),
	}
	return naplesExecCmd(v)
}

func getSystemStatusCmdCmdHandler(cmd *cobra.Command, args []string) error {
	resp, err := restGet("monitoring/v1/naples/logs/pensando/pciemgrd.log")
	if err != nil {
		return err
	}

	var pciemgrd, port0 string
	var pyr, phr, pmin, psec, pmilsec int
	var eyr, ehr, emin, esec, emilsec int
	var starttime, endtime time.Duration
	for _, line := range strings.Split(string(resp), "\n") {
		if strings.Contains(line, "pciemgrd ready") {
			fmt.Sscanf(line, "[%d-%d:%d:%d.%d]%s", &pyr, &phr, &pmin, &psec, &pmilsec, &pciemgrd)
			starttime = time.Duration(phr)*time.Hour +
				time.Duration(pmin)*time.Minute +
				time.Duration(psec)*time.Second +
				time.Duration(pmilsec)*time.Millisecond
		} else if strings.Contains(line, "port 0 ready") {
			fmt.Sscanf(line, "[%d-%d:%d:%d.%d]%s", &eyr, &ehr, &emin, &esec, &emilsec, &port0)
			endtime = time.Duration(ehr)*time.Hour +
				time.Duration(emin)*time.Minute +
				time.Duration(esec)*time.Second +
				time.Duration(emilsec)*time.Millisecond
		}
	}
	pcietime := endtime - starttime
	fmt.Printf("The PCIE up time duration is %d:%d:%d.%d\n",
		int64(pcietime/time.Hour),
		int64(pcietime/time.Minute),
		int64(pcietime/time.Second),
		int64(pcietime/time.Millisecond))
	return nil
}

func getSystemQueueStatsCmdHandler(cmd *cobra.Command, args []string) {
	halctlStr := "halctl show system queue-statistics "

	execCmd := strings.Fields(halctlStr)
	v := &nmd.NaplesCmdExecute{
		Executable: execCmd[0],
		Opts:       strings.Join(execCmd[1:], " "),
	}

	resp, err := restGetWithBody(v, "cmd/v1/naples/")
	if err != nil {
		fmt.Println(err)
		return
	}

	if len(resp) > 3 {
		s := strings.Replace(string(resp), `\n`, "\n", -1)
		fmt.Println(s)
	}

	return
}

func getSmartNicCmdHandler(cmd *cobra.Command, args []string) error {
	resp, err := restGet("api/v1/naples/info/")

	if err != nil {
		if verbose {
			fmt.Println(err.Error())
		}
		return err
	}
	cfg := cluster.SmartNIC{}
	json.Unmarshal(resp, &cfg)
	fmt.Println(cfg)
	return nil
}
