package cmd

import (
	"errors"
	"fmt"

	"github.com/gogo/protobuf/types"
	"github.com/golang/protobuf/proto"

	"github.com/spf13/cobra"

	"github.com/pensando/sw/nic/apollo/agent/cli/utils"
	"github.com/pensando/sw/nic/apollo/agent/gen/pds"

	"os"
)

var CmdSocket string = "/var/run/pds_svc_server_sock"
var vppUdsPath string = "/run/vpp/pds.sock"

type Transport int

const (
	AGENT_TRANSPORT_NONE Transport = 0
	AGENT_TRANSPORT_GRPC Transport = 1
	AGENT_TRANSPORT_UDS  Transport = 2
)

var AgentTransport Transport

// function to handle commands over unix domain sockets
// param[in] cmdReq  Service request message to be sent
// return    cmdResp Service response message
//           err     Error
func HandleCommand(cmdReq *pds.ServiceRequestMessage,
	socket string) (*pds.ServiceResponseMessage, error) {
	// marshall cmdCtxt
	iovec, err := proto.Marshal(cmdReq)
	if err != nil {
		fmt.Printf("Marshall command failed with error %v\n", err)
		return nil, err
	}

	// send over UDS
	resp, err := utils.CmdSend(socket, iovec, int(os.Stdout.Fd()))
	if err != nil {
		fmt.Printf("Command send operation failed with error %v\n", err)
		return nil, err
	}

	// unmarshal response
	cmdResp := &pds.ServiceResponseMessage{}
	err = proto.Unmarshal(resp, cmdResp)
	if err != nil {
		fmt.Printf("Command failed with %v error\n", err)
		return nil, err
	}

	return cmdResp, nil
}

// function to handle configs over unix domain sockets
// param[in] cfgReq  Service request message to be sent
// return    cfgResp Service response message
//           err     Error
func HandleConfig(cfgReq *pds.ServiceRequestMessage) (*pds.ServiceResponseMessage, error) {
	// marshall cmdCtxt
	iovec, err := proto.Marshal(cfgReq)
	if err != nil {
		fmt.Printf("Marshall command failed with error %v\n", err)
		return nil, err
	}

	// send over UDS
	resp, err := utils.CmdSend(CmdSocket, iovec, -1)
	if err != nil {
		fmt.Printf("Command send operation failed with error %v\n", err)
		return nil, err
	}

	// unmarshal response
	cmdResp := &pds.ServiceResponseMessage{}
	err = proto.Unmarshal(resp, cmdResp)
	if err != nil {
		fmt.Printf("Command failed with %v error\n", err)
		return nil, err
	}

	return cmdResp, nil
}

// function to handle command service request message
// param[in]  cmd       command
// param[in]  req       request
// param[out] resp      response
// return     err       Error
func HandleSvcReqCommandMsg(cmd pds.Command,
	req proto.Message) (*pds.ServiceResponseMessage, error) {
	var reqMsg *types.Any
	var err error

	if req != nil {
		reqMsg, err = types.MarshalAny(req)
		if err != nil {
			fmt.Printf("Command failed with %v error\n", err)
			return nil, err
		}
	} else {
		reqMsg = nil
	}

	command := &pds.CommandMessage{
		Command:    cmd,
		CommandMsg: reqMsg,
	}

	cmdReqMsg, err := types.MarshalAny(command)
	if err != nil {
		fmt.Printf("Command failed with %v error\n", err)
		return nil, err
	}

	cmdReq := &pds.ServiceRequestMessage{
		ConfigOp:  pds.ServiceRequestOp_SERVICE_OP_NONE,
		ConfigMsg: cmdReqMsg,
	}

	socket := CmdSocket
	if cmd == pds.Command_CMD_FLOW_DUMP {
		socket = vppUdsPath
	}

	// handle command
	return HandleCommand(cmdReq, socket)
}

// function to handle config service request message
// param[in]  req       request
// param[in]  op        operation (crud)
// param[out] resp      response
// return     err       Error
func HandleSvcReqConfigMsg(op pds.ServiceRequestOp,
	req proto.Message, resp proto.Message) error {
	reqMsg, err := types.MarshalAny(req)
	if err != nil {
		fmt.Printf("Command failed with %v error\n", err)
		return err
	}

	cmdReq := &pds.ServiceRequestMessage{
		ConfigOp:  op,
		ConfigMsg: reqMsg,
	}

	// handle config
	cmdResp, err := HandleConfig(cmdReq)
	if err != nil {
		return err
	}

	if cmdResp.ApiStatus != pds.ApiStatus_API_STATUS_OK {
		fmt.Printf("Command failed with %v error\n", cmdResp.ApiStatus)
		return err
	}

	anyResp := cmdResp.GetResponse()

	if resp != nil {
		err = types.UnmarshalAny(anyResp, resp)
		if err != nil {
			fmt.Printf("Command failed with %v error\n", err)
			return err
		}
	}

	return err
}

func GetAgentTransport(cmd *cobra.Command) (Transport, error) {
	if cmd != nil && cmd.Flags().Changed("transport") {
		if transport == "uds" {
			return AGENT_TRANSPORT_UDS, nil
		} else if transport == "grpc" {
			return AGENT_TRANSPORT_GRPC, nil
		} else {
			return AGENT_TRANSPORT_NONE,
				errors.New("Transport specified is invalid, refer help string")
		}
	} else {
		val, present := os.LookupEnv("PDS_AGENT_TRANSPORT")
		if present != true {
			return AGENT_TRANSPORT_GRPC, nil
		} else {
			if val == "uds" {
				return AGENT_TRANSPORT_UDS, nil
			} else if val == "grpc" {
				return AGENT_TRANSPORT_GRPC, nil
			} else {
				return AGENT_TRANSPORT_NONE,
					errors.New("Environment variable PDS_AGENT_TRANSPORT is invalid. Should be uds or grpc")
			}
		}
	}
	return AGENT_TRANSPORT_NONE, errors.New("Invalid transport")
}
