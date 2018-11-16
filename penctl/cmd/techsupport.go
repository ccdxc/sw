//-----------------------------------------------------------------------------
// {C} Copyright 2017 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------

package cmd

import (
	"bufio"
	"bytes"
	"fmt"
	"io"
	"io/ioutil"
	"os"
	"os/exec"
	"strings"
	"time"

	"gopkg.in/yaml.v2"

	"github.com/spf13/cobra"

	nmd "github.com/pensando/sw/nic/agent/nmd/protos"
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

func init() {
	sysCmd.AddCommand(showTechCmd)

	showTechCmd.Flags().StringVarP(&destDir, "dest-dir", "d", "", "Destination directory to copy Naples tech-support to")
	showTechCmd.Flags().StringVarP(&cmdFile, "cmds", "c", "", "YML file with list of commands to run on Naples")
	showTechCmd.Flags().StringVarP(&tarFile, "tarball", "b", "", "Name of tarball to create (without .tar.gz)")
}

// NaplesCmds is the format of the yaml file used to run commands on Naples for tech-support
type NaplesCmds struct {
	Cmds []struct {
		Cmd   string `yaml:"cmd"`
		Ofile string `yaml:"ofile"`
	} `yaml:"Cmds"`
}

func copyFileToDest(destDir string, url string, file string) error {
	//fmt.Println("Copying file: " + file + " to: " + destDir)
	resp, err := restGetResp(revProxyPort, url+file)
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

func showTechCmdHandler(cmd *cobra.Command, args []string) error {
	timeStr := time.Now().Format(time.UnixDate)
	timeStr = strings.Replace(timeStr, " ", "-", -1)
	if !cmd.Flags().Changed("dest") {
		destDir = "./"
	}
	destDir = destDir + "/NaplesTechSupport-" + timeStr + "/"
	fmt.Println("Collecting tech-support from Naples")

	if _, err := os.Stat(destDir); os.IsNotExist(err) {
		os.MkdirAll(destDir, os.ModePerm)
	}

	fmt.Printf("Fetching cores")
	//Copy out core files from /data/core
	coreDestDir := destDir + "/cores/"
	createDestDir(coreDestDir)
	resp, _ := restGetResp(revProxyPort, "cores/v1/naples/")
	retS, err := parseFiles(resp)
	if err != nil {
		return err
	}
	for _, file := range retS {
		fmt.Printf(".")
		copyFileToDest(coreDestDir, "cores/v1/naples/", file)
	}
	fmt.Printf("\nCores fetched\n")
	retSlice = nil

	fmt.Printf("Fetching events")
	//Copy out events from /var/lib/pensando/events/events file
	eventsDestDir := destDir + "/events/"
	createDestDir(eventsDestDir)
	evresp, _ := restGet(revProxyPort, "monitoring/v1/naples/events/events")
	file = eventsDestDir + "/" + "events"
	out, err := os.Create(file)
	if err != nil {
		fmt.Println(err)
		return err
	}
	defer out.Close()
	w := bufio.NewWriter(out)
	w.WriteString(string(evresp))
	w.Flush()
	fmt.Printf("\nEvents fetched\n")

	fmt.Printf("Fetching logs")
	//Copy out log files from /var/log recursively
	resp, _ = restGetResp(revProxyPort, "monitoring/v1/naples/logs/")
	retS, err = parseFiles(resp)
	if err != nil {
		return err
	}
	logDestDir := destDir + "/logs/"
	createDestDir(logDestDir)
	for _, file = range retS {
		if strings.HasSuffix(file, "/") {
			napDir := file
			logSubDestDir := logDestDir + napDir
			createDestDir(logSubDestDir)
			resp, _ = restGetResp(revProxyPort, "monitoring/v1/naples/logs/"+napDir)
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
			copyFileToDest(logDestDir, "monitoring/v1/naples/logs/", file)
		}
	}
	fmt.Printf("\nLogs fetched\n")

	if !cmd.Flags().Changed("cmds") {
		cmdFile = "naples-tech-support-cmd.yml"
	}
	if _, err := os.Stat(cmdFile); !os.IsNotExist(err) {
		if !cmd.Flags().Changed("cmds") {
			cmdFile = "naples-tech-support-cmd.yml"
		}
		fmt.Printf("Executing commands")
		//Execute cmds pointed to by YML file
		cmdDestDir := destDir + "/cmd_out/"
		createDestDir(cmdDestDir)

		var naplesCmds NaplesCmds
		source, err := ioutil.ReadFile(cmdFile)
		if err != nil {
			fmt.Println(err)
			return err
		}
		err = yaml.Unmarshal(source, &naplesCmds)
		if err != nil {
			fmt.Println(err)
			return err
		}
		for _, naplesCmd := range naplesCmds.Cmds {
			cmd := strings.Fields(naplesCmd.Cmd)
			opts := strings.Join(cmd[1:], " ")
			v := &nmd.NaplesCmdExecute{
				Executable: cmd[0],
				Opts:       opts,
			}
			resp, err := restGetWithBody(v, revProxyPort, "cmd/v1/naples/")
			if err != nil {
				fmt.Println(err)
			}
			if len(resp) > 3 {
				fmt.Printf(".")
				s := strings.Replace(string(resp[1:len(resp)-2]), `\n`, "\n", -1)
				file = cmdDestDir + "/" + naplesCmd.Ofile
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
		fmt.Printf("\nCommands executed\n")
	} else {
		fmt.Println("No commands to run")
	}

	if !cmd.Flags().Changed("tarball") {
		tarFile = "naples-tech-support"
	}
	fmt.Println("Creating tarball: " + tarFile + ".tar.gz")
	tarcmd := exec.Command("tar", "-zcvf", tarFile+".tar.gz", destDir)
	tarcmd.Stdin = strings.NewReader("tar naples-tech-support")
	var tarout bytes.Buffer
	tarcmd.Stdout = &tarout
	err = tarcmd.Run()
	if err != nil {
		return err
	}
	fmt.Println(tarFile + ".tar.gz generated")

	rmdestdircmd := exec.Command("rm", "-rf", destDir)
	rmdestdircmd.Stdin = strings.NewReader("rm -rf " + destDir)
	var rmout bytes.Buffer
	rmdestdircmd.Stdout = &rmout
	err = rmdestdircmd.Run()
	if err != nil {
		return err
	}

	return nil
}
