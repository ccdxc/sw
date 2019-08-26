//-----------------------------------------------------------------------------
// {C} Copyright 2017 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------

package cmd

import (
	"bufio"
	"bytes"
	"fmt"
	"io"
	"os"
	"os/exec"
	"strings"
	"time"

	"gopkg.in/yaml.v2"

	"github.com/spf13/cobra"

	nmd "github.com/pensando/sw/nic/agent/protos/nmd"
)

var showTechCmd = &cobra.Command{
	Use:   "tech-support",
	Short: "Get Tech Support from Naples",
	Long:  "\n------------------------------\n Get Tech Support from Naples \n------------------------------\n",
	RunE:  showTechCmdHandler,
}

var destDir string
var cmdFile string
var tarFile string
var tarFileDir string
var tmpDir string

func init() {
	sysCmd.AddCommand(showTechCmd)

	showTechCmd.Flags().StringVarP(&tarFile, "tarball", "b", "", "Name of tarball to create (without .tar.gz)")
	showTechCmd.Flags().StringVarP(&tarFileDir, "odir", "", "", "Directory to create the tech-support in")
	showTechCmd.Flags().StringVarP(&tmpDir, "tmpdir", "", "", "Transient directory to collect files into")

	showTechCmd.Flags().MarkHidden("odir")
	showTechCmd.Flags().MarkHidden("tmpdir")
}

var cmdToExecute = `
Cmds:
 -
   cmd: pcieutil port
   outputfile: pcieutil.port
 -
   cmd: pcieutil portstats
   outputfile: pcieutil.portstats
 -
   cmd: pcieutil stats
   outputfile: pcieutil.stats
 -
   cmd: pcieutil counters
   outputfile: pcieutil.counters
 -
   cmd: pcieutil dev
   outputfile: pcieutil.dev
 -
   cmd: pcieutil devintr
   outputfile: pcieutil.devintr
 -
   cmd: pcieutil bar
   outputfile: pcieutil.bar
 -
   cmd: pcieutil pmt
   outputfile: pcieutil.pmt
 -
   cmd: pcieutil prt
   outputfile: pcieutil.prt
 -
   cmd: pcieutil aximst
   outputfile: pcieutil.aximst
 -
   cmd: ifconfig -a
   outputfile: ifconfig.out
 -
   cmd: fwupdate -l
   outputfile: fw_version.out
 -
   cmd: ps
   outputfile: ps.out
 -
   cmd: df
   outputfile: df.out
 -
   cmd: delphictl db get UpgReq
   outputfile: upgreq.out
 -
   cmd: delphictl db get UpgResp
   outputfile: upgresp.out
 -
   cmd: delphictl db get UpgStateReq
   outputfile: upgstatereq.out
 -
   cmd: delphictl db get UpgAppResp
   outputfile: upgappresp.out
 -
   cmd: delphictl db get UpgApp
   outputfile: upgapp.out
 -
   cmd: uptime
   outputfile: uptime.out
`

// NaplesCmds is the format of the yaml file used to run commands on Naples for tech-support
type NaplesCmds struct {
	Cmds []struct {
		Cmd        string `yaml:"cmd"`
		Outputfile string `yaml:"outputfile"`
	} `yaml:"Cmds"`
}

func copyFileToDest(destDir string, url string, file string) error {
	//fmt.Println("Copying file: " + file + " to: " + destDir)
	resp, err := restGetResp(url + file)
	if err != nil {
		fmt.Println(err)
		return err
	}
	defer resp.Body.Close()
	file = destDir + "/" + file
	out, err := os.Create(file)
	if err != nil {
		fmt.Println(err)
		return err
	}
	defer out.Close()
	// Write the body to file
	_, err = io.Copy(out, resp.Body)
	if err != nil {
		fmt.Println(err)
		return err
	}
	return err
}

func createDestDir(destDir string) {
	if _, err := os.Stat(destDir); os.IsNotExist(err) {
		os.MkdirAll(destDir, os.ModePerm)
	}
}

func getLogs(subDir string, url string) error {
	//Copy out log files from /var/log recursively
	resp, err := restGetResp(url)
	if err != nil && !strings.Contains(err.Error(), "not found") {
		return err
	}
	retS, err := parseFiles(resp)
	if err != nil {
		return err
	}
	logDestDir := destDir + "/" + subDir + "/"
	createDestDir(logDestDir)
	for _, file = range retS {
		if strings.HasSuffix(file, "/") {
			napDir := file
			logSubDestDir := logDestDir + napDir
			createDestDir(logSubDestDir)
			resp, err = restGetResp(url + napDir)
			if err != nil {
				return err
			}
			retSlice = nil
			retS, err = parseFiles(resp)
			if err != nil {
				return err
			}
			for _, subfile := range retS {
				fmt.Printf(".")
				copyFileToDest(logSubDestDir, "monitoring/v1/naples/logs/"+napDir, subfile)
			}
			retSlice = nil
		} else {
			fmt.Printf(".")
			copyFileToDest(logDestDir, url, file)
		}
	}
	return nil
}

func executeCmd(req *nmd.NaplesCmdExecute, parts []string) (string, error) {
	cmd := exec.Command(req.Executable, parts...)
	os.Setenv("PATH", os.Getenv("PATH")+":/bin:/sbin:/usr/bin:/usr/sbin:/platform/bin:/nic/bin:/platform/tools:/nic/tools")
	cmd.Env = os.Environ()
	if verbose {
		fmt.Printf("Upgrade Cmd Execute Request: %+v env: [%s]", req, os.Environ())
	}
	stdoutStderr, err := cmd.CombinedOutput()
	if err != nil {
		return string(fmt.Sprintf(err.Error()) + ":" + string(stdoutStderr)), err
	}
	return string(stdoutStderr), nil
}

func execCmd(req *nmd.NaplesCmdExecute) (string, error) {
	parts := strings.Fields(req.Opts)
	return executeCmd(req, parts)
}

func execTechSupportCmds(cmdToExecute string, cmdDestDir string) error {
	var naplesCmds NaplesCmds
	err := yaml.UnmarshalStrict([]byte(cmdToExecute), &naplesCmds)
	if err != nil {
		fmt.Println(err)
		return err
	}
	for _, naplesCmd := range naplesCmds.Cmds {
		if naplesCmd.Outputfile == "" || naplesCmd.Cmd == "" {
			fmt.Printf("\nMissing command attributes %+v\n", naplesCmd)
			continue
		}
		cmd := strings.Fields(naplesCmd.Cmd)
		opts := strings.Join(cmd[1:], " ")
		v := &nmd.NaplesCmdExecute{
			Executable: cmd[0],
			Opts:       opts,
		}
		var resp []byte
		if isNaplesReachableOverLocalHost() == nil {
			fmt.Println("Executing cmd locally")
			var ret string
			ret, err = execCmd(v)
			resp = []byte(ret)
		} else {
			resp, err = restGetWithBody(v, "cmd/v1/naples/")
		}
		if err != nil {
			fmt.Println(err)
		}
		if len(resp) > 3 {
			fmt.Println(naplesCmd.Cmd)
			s := strings.Replace(string(resp), `\n`, "\n", -1)
			s = strings.Replace(s, "\\", "", -1)
			file = cmdDestDir + "/" + naplesCmd.Outputfile
			out, err := os.Create(file)
			if err != nil {
				fmt.Println(err)
			}
			defer out.Close()
			w := bufio.NewWriter(out)
			w.WriteString("===" + cmd[0] + " " + opts + "===\n" + s)
			w.Flush()
		}
	}
	return nil
}

func createTechSupportTarBall(destDir string, tarFile string, tarcmd *exec.Cmd) error {
	fmt.Println("Creating tarball: " + tarFile + ".tar.gz")
	tarcmd.Stdin = strings.NewReader("tar naples-tech-support")
	var tarout bytes.Buffer
	tarcmd.Stdout = &tarout
	err := tarcmd.Run()
	if err != nil {
		return err
	}
	fmt.Println(tarFile + ".tar.gz generated")

	fmt.Println("removing " + destDir)
	rmdestdircmd := exec.Command("rm", "-rf", destDir)
	rmdestdircmd.Stdin = strings.NewReader("rm -rf " + destDir)
	var rmout bytes.Buffer
	rmdestdircmd.Stdout = &rmout
	return rmdestdircmd.Run()
}

func showTechCmdHandler(cmd *cobra.Command, args []string) error {
	jsonFormat = false
	timeStr := time.Now().Format(time.UnixDate)
	timeStr = strings.Replace(timeStr, " ", "-", -1)

	destDir = "/tmp"
	if cmd.Flags().Changed("tmpdir") {
		destDir = tmpDir
	} else if val, ok := os.LookupEnv("TMPDIR"); ok {
		destDir = val
		if verbose {
			fmt.Printf("$TMPDIR set to %s\n", val)
		}
	}

	destDir = destDir + "/NaplesTechSupport-" + timeStr + "/"
	fmt.Println("Collecting tech-support from Naples")

	if _, err := os.Stat(destDir); os.IsNotExist(err) {
		os.MkdirAll(destDir, os.ModePerm)
	}

	fmt.Printf("Executing commands:\n")
	//Execute cmds pointed to by YML file
	cmdDestDir := destDir + "/cmd_out/"
	createDestDir(cmdDestDir)

	execTechSupportCmds(cmdToExecute, cmdDestDir)
	fmt.Printf("Commands executed\n")

	file = destDir + "/penctl.ver"
	out, err := os.Create(file)
	if err != nil {
		fmt.Println(err)
	}
	defer out.Close()
	w := bufio.NewWriter(out)
	w.WriteString(getPenctlVer())
	w.Flush()

	if !cmd.Flags().Changed("tarball") {
		tarFile = "naples-tech-support"
	}
	if !cmd.Flags().Changed("odir") {
		tarFileDir = "./"
	}
	tarFile = tarFileDir + "/" + tarFile

	if isNaplesReachableOverLocalHost() == nil {
		fmt.Println("penctl running tar locally on naples")
		tarcmd := exec.Command("tar", "-czf", tarFile+".tar.gz", destDir, "/data/", "/var/log/", "/update/", "/var/lib/pensando/events/", "/obfl/")
		return createTechSupportTarBall(destDir, tarFile, tarcmd)
	}

	fmt.Printf("Fetching data directory")
	//Copy out upgrade logs
	dataDestDir := destDir + "/data/"
	createDestDir(dataDestDir)
	resp, err := restGetResp("data/")
	if err != nil {
		return err
	}
	retS, err := parseFiles(resp)
	if err != nil {
		return err
	}
	for _, file := range retS {
		fmt.Println(file)
		if !strings.Contains(file, "naples-disruptive-upgrade-tech-support") {
			if strings.HasSuffix(file, "/") || strings.Contains(file, "tar") {
				continue
			}
		}
		fmt.Printf(".")
		copyFileToDest(dataDestDir, "data/", file)
	}
	fmt.Printf("\ndata directory fetched\n")
	retSlice = nil

	fmt.Printf("Fetching update directory")
	//Copy out files from /update/
	updateDestDir := destDir + "/update/"
	createDestDir(updateDestDir)
	resp, err = restGetResp("update/")
	if err != nil && !strings.Contains(err.Error(), "not found") {
		return err
	}
	retS, err = parseFiles(resp)
	if err != nil {
		return err
	}
	for _, file = range retS {
		if strings.HasSuffix(file, "/") || strings.Contains(file, "tar") {
			continue
		}
		fmt.Printf(".")
		copyFileToDest(updateDestDir, "update/", file)
	}
	fmt.Printf("\nupdate directory fetched\n")
	retSlice = nil

	fmt.Printf("Fetching cores")
	//Copy out core files from /data/core
	coreDestDir := destDir + "/cores/"
	createDestDir(coreDestDir)
	resp, err = restGetResp("cores/v1/naples/")
	if err != nil && !strings.Contains(err.Error(), "not found") {
		return err
	}
	retS, err = parseFiles(resp)
	if err != nil {
		return err
	}
	for _, file := range retS {
		fmt.Printf(".")
		copyFileToDest(coreDestDir, "cores/v1/naples/", file)
	}
	fmt.Printf("\nCores fetched\n")
	retSlice = nil

	fmt.Printf("Fetching events ")
	//Copy out all event files from /var/lib/pensando/events/ dir
	eventsDestDir := destDir + "/events/"
	createDestDir(eventsDestDir)
	evresp, err := restGetResp("monitoring/v1/naples/events/")
	if err != nil && !strings.Contains(err.Error(), "not found") {
		return err
	}
	retS, err = parseFiles(evresp)
	if err != nil {
		return err
	}
	fmt.Println(retS)
	for _, file = range retS {
		if file == "events" || strings.HasSuffix(file, "/") {
			continue
		}
		fmt.Printf(".")
		copyFileToDest(eventsDestDir, "monitoring/v1/naples/events/", file)
	}
	fmt.Printf("Events fetched\n")
	retSlice = nil

	fmt.Printf("Fetching logs")
	err = getLogs("logs", "monitoring/v1/naples/logs/")
	if err != nil {
		return err
	}
	err = getLogs("obfl", "monitoring/v1/naples/obfl/")
	if err != nil {
		return err
	}
	fmt.Printf("\nLogs fetched\n")

	fmt.Println("Creating tarball: " + tarFile + ".tar.gz")
	tarcmd := exec.Command("tar", "-czf", tarFile+".tar.gz", destDir)
	return createTechSupportTarBall(destDir, tarFile, tarcmd)
}
