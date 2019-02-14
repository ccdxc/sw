package agent

import (
	"fmt"
	"os"
	"strings"
	"time"
	"unicode/utf8"

	"github.com/pkg/errors"

	iota "github.com/pensando/sw/iota/protos/gogen"
	Cmd "github.com/pensando/sw/iota/svcs/agent/command"
	utils "github.com/pensando/sw/iota/svcs/agent/utils"
	Common "github.com/pensando/sw/iota/svcs/common"
)

const (
	veniceStartScript = "INSTALL.sh"
)

type veniceNode struct {
	iotaNode
	bgCmds     map[string]*Cmd.CommandInfo
	bgCmdIndex uint32
}

type venicePeerNode struct {
	hostname string
	ip       string
}

func (venice *veniceNode) bringUpVenice(image string, hostname string,
	ctrlIntf string, ctrlIP string, reload bool, peers []venicePeerNode) error {

	if ctrlIntf != "" {
		utils.DisableDhcpOnInterface(ctrlIntf)
		venice.logger.Println("Configuring intf : " + ctrlIntf + " with " + ctrlIP)
		ifConfigCmd := []string{"ifconfig", ctrlIntf, ctrlIP, "up"}
		if _, stdout, err := utils.Run(ifConfigCmd, 0, false, true, nil); err != nil {
			errors.New("Setting control interface IP to venice node failed.." + stdout)
		}
	}

	// if this is a reload, we are done
	if reload {
		return nil
	}

	curDir, _ := os.Getwd()
	defer os.Chdir(curDir)
	os.Chdir(Common.DstIotaAgentDir)
	venice.logger.Println("Untar image : " + image)
	untar := []string{"tar", "-xvzf", image}
	if _, stdout, err := utils.Run(untar, 0, false, false, nil); err != nil {
		return errors.Wrap(err, stdout)
	}

	setHostname := []string{"hostnamectl", "set-hostname", hostname}
	if _, stdout, err := utils.Run(setHostname, 0, false, false, nil); err != nil {
		venice.logger.Println("Setting hostname failed")
		return errors.Wrap(err, stdout)
	}

	venice.logger.Println("Running Install Script : " + veniceStartScript)
	install := []string{"./" + veniceStartScript, "--clean"}
	if _, stdout, err := utils.Run(install, 0, false, false, nil); err != nil {
		venice.logger.Println("Running Install Script failed : " + veniceStartScript)
		return errors.Wrap(err, stdout)
	}

	for _, peer := range peers {
		if peer.hostname != "" && peer.ip != "" {
			cmd := []string{"echo", strings.Split(peer.ip, "/")[0], peer.hostname, " | sudo tee -a /etc/hosts"}
			if _, stdout, err := utils.Run(cmd, 0, false, true, nil); err != nil {
				venice.logger.Println("Setting venice peer hostnames failed")
				return errors.Wrap(err, stdout)
			}
		}
	}
	return nil
}

//Init initalize node type
func (venice *veniceNode) Init(in *iota.Node) (*iota.Node, error) {
	venice.iotaNode.name = in.GetName()
	venice.iotaNode.nodeMsg = in
	venice.logger.Printf("Bring up request received for : %v. Req: %+v", in.GetName(), in)

	veniceNodes := []venicePeerNode{}

	for _, node := range in.GetVeniceConfig().GetVenicePeers() {
		veniceNodes = append(veniceNodes, venicePeerNode{hostname: node.GetHostName(),
			ip: node.GetIpAddress()})
	}

	if err := venice.bringUpVenice(in.GetImage(), in.GetName(),
		in.GetVeniceConfig().GetControlIntf(), in.GetVeniceConfig().GetControlIp(),
		in.GetReload(), veniceNodes); err != nil {
		venice.logger.Println("Venice bring up failed.")
		return &iota.Node{NodeStatus: &iota.IotaAPIResponse{ApiStatus: iota.APIResponseType_API_SERVER_ERROR}}, err

	}

	venice.logger.Println("Venice bring script up successful.")

	return &iota.Node{Name: in.Name, IpAddress: in.IpAddress, NodeUuid: "", Type: in.GetType(),
		NodeStatus: &iota.IotaAPIResponse{ApiStatus: iota.APIResponseType_API_STATUS_OK}}, nil
}

// AddWorkload brings up a workload type on a given node
func (venice *veniceNode) AddWorkloads(*iota.WorkloadMsg) (*iota.WorkloadMsg, error) {
	venice.logger.Println("Add workload on venice not supported.")
	return &iota.WorkloadMsg{ApiResponse: &iota.IotaAPIResponse{ApiStatus: iota.APIResponseType_API_BAD_REQUEST}}, nil
}

// DeleteWorkloads deletes a given workloads
func (venice *veniceNode) DeleteWorkloads(*iota.WorkloadMsg) (*iota.WorkloadMsg, error) {
	venice.logger.Println("Delete workload on venice not supported.")
	return &iota.WorkloadMsg{ApiResponse: &iota.IotaAPIResponse{ApiStatus: iota.APIResponseType_API_BAD_REQUEST}}, nil
}

// Trigger invokes the workload's trigger. It could be ping, start client/server etc..
func (venice *veniceNode) Trigger(in *iota.TriggerMsg) (*iota.TriggerMsg, error) {
	venice.logger.Println("Venice node does not support trigger.")
	for _, cmd := range in.Commands {
		var err error
		var cmdKey string
		var cmdResp *Cmd.CommandCtx

		if in.TriggerOp == iota.TriggerOp_EXEC_CMDS {
			cmdResp, cmdKey, err = venice.RunCommand(strings.Split(cmd.GetCommand(), " "),
				cmd.GetRunningDir(), cmd.GetForegroundTimeout(),
				cmd.GetMode() == iota.CommandMode_COMMAND_BACKGROUND, true)

		} else {
			cmdResp, err = venice.StopCommand(cmd.Handle)
			cmdKey = cmd.Handle
		}

		cmd.ExitCode, cmd.Stdout, cmd.Stderr, cmd.Handle, cmd.TimedOut = cmdResp.ExitCode, cmdResp.Stdout, cmdResp.Stderr, cmdKey, cmdResp.TimedOut
		venice.logger.Println("Command error :", err)
		venice.logger.Println("Command exit code :", cmd.ExitCode)
		venice.logger.Println("Command timed out :", cmd.TimedOut)
		venice.logger.Println("Command handle  :", cmd.Handle)
		venice.logger.Println("Command stdout :", cmd.Stdout)
		venice.logger.Println("Command stderr:", cmd.Stderr)

		fixUtf := func(r rune) rune {
			if r == utf8.RuneError {
				return -1
			}
			return r
		}

		cmd.Stdout = strings.Map(fixUtf, cmd.Stdout)
		cmd.Stderr = strings.Map(fixUtf, cmd.Stderr)
		if len(cmd.Stdout) > maxStdoutSize || len(cmd.Stderr) > maxStdoutSize {
			cmd.Stdout = ""
			cmd.Stderr = "Stdout/Stderr output limit Exceeded."
			cmd.ExitCode = 127
		}
	}

	in.ApiResponse = &iota.IotaAPIResponse{ApiStatus: iota.APIResponseType_API_STATUS_OK, ErrorMsg: "Api success"}
	return in, nil
}

// CheckHealth returns the node health
func (venice *veniceNode) CheckHealth(in *iota.NodeHealth) (*iota.NodeHealth, error) {
	return &iota.NodeHealth{NodeName: in.GetNodeName(), HealthCode: iota.NodeHealth_HEALTH_OK}, nil
}

//NodeType return node type
func (venice *veniceNode) NodeType() iota.PersonalityType {
	return iota.PersonalityType_PERSONALITY_VENICE
}

//GetMsg node msg
func (venice *veniceNode) GetMsg() *iota.Node {
	return venice.nodeMsg
}

//GetWorkloadMsgs get workloads
func (venice *veniceNode) GetWorkloadMsgs() []*iota.Workload {
	return nil
}

// RunCommand runs a command on venice nodes
func (venice *veniceNode) RunCommand(cmd []string, dir string, timeout uint32, background bool, shell bool) (*Cmd.CommandCtx, string, error) {
	handleKey := ""

	runDir := Common.DstIotaAgentDir
	if dir != "" {
		runDir = runDir + "/" + dir
	}

	fmt.Println("base dir ", runDir, dir)

	venice.logger.Println("Running cmd ", strings.Join(cmd, " "))
	cmdInfo, _ := Cmd.ExecCmd(cmd, runDir, (int)(timeout), background, shell, nil)

	if background {
		handleKey := fmt.Sprintf("venice-bg-cmd-%v", venice.bgCmdIndex)
		venice.bgCmdIndex++
		venice.bgCmds[handleKey] = cmdInfo
	}

	return cmdInfo.Ctx, handleKey, nil
}

// StopCommand stops a running command
func (venice *veniceNode) StopCommand(commandHandle string) (*Cmd.CommandCtx, error) {
	cmdInfo, ok := venice.bgCmds[commandHandle]
	if !ok {
		return &Cmd.CommandCtx{ExitCode: -1, Stdout: "", Stderr: "", Done: true}, nil
	}

	venice.logger.Printf("Stopping bare metal Running cmd %v %v\n", cmdInfo.Ctx.Stdout, cmdInfo.Handle)

	Cmd.StopExecCmd(cmdInfo)
	time.Sleep(2 * time.Second)

	return cmdInfo.Ctx, nil
}
