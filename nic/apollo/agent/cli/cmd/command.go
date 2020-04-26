package cmd

import (
	"errors"
	"fmt"
	"net"
	"syscall"

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

type PrintObject interface {
	PrintHeader()
	HandleObject(*types.Any) bool
}

// function to handle commands over unix domain sockets
// param[in] cmdReq  Service request message to be sent
// return    cmdResp Service response message
//           err     Error
func HandleCommand(cmdReq *pds.ServiceRequestMessage) (*pds.ServiceResponseMessage, error) {
	// marshall cmdCtxt
	iovec, err := proto.Marshal(cmdReq)
	if err != nil {
		fmt.Printf("Marshall command failed with error %v\n", err)
		return nil, err
	}

	// send over UDS
	resp, err := utils.CmdSend(CmdSocket, iovec, int(os.Stdout.Fd()))
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

	// handle command
	return HandleCommand(cmdReq)
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

// function to handle show object command over unix domain sockets
// param[in] cmd     Command to be sent
// return    err     Error
func HandleUdsShowObject(cmd pds.Command, i PrintObject) error {
	command := &pds.CommandMessage{
		Command:    cmd,
		CommandMsg: nil,
	}

	cmdReqMsg, err := types.MarshalAny(command)
	if err != nil {
		fmt.Printf("Command failed with %v error\n", err)
		return err
	}

	cmdReq := &pds.ServiceRequestMessage{
		ConfigOp:  pds.ServiceRequestOp_SERVICE_OP_NONE,
		ConfigMsg: cmdReqMsg,
	}

	// marshall cmdCtxt
	iovec, err := proto.Marshal(cmdReq)
	if err != nil {
		fmt.Printf("Marshall command failed with error %v\n", err)
		return err
	}

	c, err := net.Dial("unixpacket", vppUdsPath)
	if err != nil {
		fmt.Printf("Could not connect to unix domain socket\n")
		return err
	}
	defer c.Close()

	udsConn := c.(*net.UnixConn)
	udsFile, err := udsConn.File()
	if err != nil {
		return err
	}

	socket := int(udsFile.Fd())
	defer udsFile.Close()

	err = syscall.Sendmsg(socket, iovec, nil, nil, 0)
	if err != nil {
		fmt.Printf("Sendmsg failed with error %v\n", err)
		return err
	}

	i.PrintHeader()
	// read from the socket until the no more entries are received
	resp := make([]byte, 256)
	for {
		n, _, _, _, err := syscall.Recvmsg(socket, resp, nil, syscall.MSG_WAITALL)
		if err != nil {
			fmt.Printf("Recvmsg failed with error %v\n", err)
			return err
		}

		// unmarshal response
		recvResp := resp[:n]
		cmdResp := &pds.ServiceResponseMessage{}
		err = proto.Unmarshal(recvResp, cmdResp)
		if err != nil {
			fmt.Printf("Command failed with %v error\n", err)
			return err
		}

		if cmdResp.ApiStatus != pds.ApiStatus_API_STATUS_OK {
			fmt.Printf("Command failed with %v error\n", cmdResp.ApiStatus)
			return err
		}

		done := i.HandleObject(cmdResp.GetResponse())
		if done {
			// Last message
			return nil
		}
	}
	return nil
}

func GetAgentTransport(cmd *cobra.Command) (Transport, error) {
	if cmd != nil && cmd.Flags().Changed("transport") {
		if transport == "uds" {
			return AGENT_TRANSPORT_UDS, nil
		} else if transport == "grpc" {
			return AGENT_TRANSPORT_GRPC, nil
		} else {
			return AGENT_TRANSPORT_NONE,
				errors.New("Transport specified is invalid. Refer help string")
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
