package testbed

import (
	"context"
	"errors"
	"fmt"
	"sync"

	iota "github.com/pensando/sw/iota/protos/gogen"
	"github.com/pensando/sw/venice/utils/log"
	"golang.org/x/sync/errgroup"
)

// Trigger triggers a workload
func (n *TestNode) Trigger(triggerReq *iota.TriggerMsg) (*iota.TriggerMsg, error) {
	return n.TriggerWithContext(context.Background(), triggerReq)
}

// TriggerWithContext runs a trigger on workload
func (n *TestNode) TriggerWithContext(ctx context.Context,
	triggerReq *iota.TriggerMsg) (*iota.TriggerMsg, error) {

	if n.triggerLocal {
		return n.TriggerWithContextLocally(ctx, triggerReq)
	}
	resp, err := n.AgentClient.Trigger(ctx, triggerReq)
	log.Infof("TOPO SVC | DEBUG | Trigger Agent . Received Response Msg: %v", resp)

	if err != nil {
		log.Errorf("Running trigger on node %v failed. Err: %v", n.Node.Name, err)
		return nil, err
	}

	if resp.ApiResponse.ApiStatus != iota.APIResponseType_API_STATUS_OK {
		log.Errorf("Running trigger on node %v failed. Agent Returned non ok status: %v", n.Node.Name, resp.ApiResponse.ApiStatus)
		return nil, fmt.Errorf("Running trigger on node %v failed. Agent Returned non ok status: %v", n.Node.Name, resp.ApiResponse.ApiStatus)
	}

	return resp, nil
}

type triggerWrap struct {
	triggerMsg *iota.TriggerMsg
	cmdIndex   []int
	wloadName  string
}

// Trigger invokes the workload's trigger. It could be ping, start client/server etc..
func (n *TestNode) triggerValidate(in *iota.TriggerMsg) (*iota.TriggerMsg, error) {

	validate := func() error {
		for _, cmd := range in.Commands {
			wloadKey := cmd.GetEntityName()
			if _, ok := n.workloadMap.Load(wloadKey); !ok {
				msg := fmt.Sprintf("Workload %s does not exist on node %s", cmd.GetEntityName(), n.info.Name)
				log.Errorf(msg)
				return errors.New(msg)
			}
		}
		return nil
	}

	if err := validate(); err != nil {
		return &iota.TriggerMsg{ApiResponse: &iota.IotaAPIResponse{ApiStatus: iota.APIResponseType_API_BAD_REQUEST, ErrorMsg: err.Error()}}, err
	}

	return in, nil
}

// TriggerWithContextLocally runs a trigger locally
func (n *TestNode) TriggerWithContextLocally(ctx context.Context, in *iota.TriggerMsg) (*iota.TriggerMsg, error) {

	if _, err := n.triggerValidate(in); err != nil {
		return &iota.TriggerMsg{
			Commands:    in.Commands,
			ApiResponse: &iota.IotaAPIResponse{ApiStatus: iota.APIResponseType_API_BAD_REQUEST, ErrorMsg: err.Error()}}, nil
	}

	runTrigger := func(client iota.IotaAgentApiClient, tw *triggerWrap) (*triggerWrap, error) {
		var err error
		log.Infof("Sending trigger to : %s ", tw.wloadName)
		tw.triggerMsg, err = client.Trigger(context.Background(), tw.triggerMsg)
		log.Infof("Completed trigger from  : %s ", tw.wloadName)
		if err != nil {
			log.Infof("Error running trigger : %v", err.Error())
		}
		return tw, err
	}
	if in.GetTriggerMode() == iota.TriggerMode_TRIGGER_NODE_PARALLEL {
		triggerMap := new(sync.Map)
		var twrap *triggerWrap

		for cmdIndex, cmd := range in.Commands {
			if val, ok := triggerMap.Load(cmd.EntityName); !ok {
				triggerMsg := &iota.TriggerMsg{Commands: []*iota.Command{},
					TriggerOp:   in.GetTriggerOp(),
					TriggerMode: in.GetTriggerMode()}
				twrap = &triggerWrap{triggerMsg: triggerMsg, cmdIndex: []int{}, wloadName: cmd.EntityName}
				triggerMap.Store(cmd.EntityName, twrap)
			} else {
				twrap = val.(*triggerWrap)
			}

			twrap.cmdIndex = append(twrap.cmdIndex, cmdIndex)
			twrap.triggerMsg.Commands = append(twrap.triggerMsg.Commands, cmd)
		}
		pool, _ := errgroup.WithContext(context.Background())
		triggerMap.Range(func(key interface{}, item interface{}) bool {
			twrap := item.(*triggerWrap)
			wload, _ := n.workloadMap.Load(key.(string))
			iotaWload := wload.(iotaWorkload)
			wloadAgent := iotaWload.workload.GetWorkloadAgent()
			pool.Go(func() error {
				runTrigger(wloadAgent.(iota.IotaAgentApiClient), twrap)
				return nil
			})
			return true
		})
		pool.Wait()
		triggerMap.Range(func(key interface{}, item interface{}) bool {
			twrap := item.(*triggerWrap)
			for index, cmd := range twrap.triggerMsg.GetCommands() {
				realIndex := twrap.cmdIndex[index]
				in.Commands[realIndex] = cmd
			}

			return true
		})

	} else {
		var err error
		for index, cmd := range in.Commands {
			wload, _ := n.workloadMap.Load(cmd.EntityName)
			triggerMsg := &iota.TriggerMsg{Commands: []*iota.Command{cmd},
				TriggerOp:   in.GetTriggerOp(),
				TriggerMode: in.GetTriggerMode()}
			twrap := &triggerWrap{triggerMsg: triggerMsg, cmdIndex: []int{}, wloadName: cmd.EntityName}
			iotaWload := wload.(iotaWorkload)
			wloadAgent := iotaWload.workload.GetWorkloadAgent()
			if wloadAgent == nil {
				log.Errorf("Worload agent not set for %v", iotaWload.name)
			}
			twrap, err = runTrigger(wloadAgent.(iota.IotaAgentApiClient), twrap)
			logger.Infof("Completed running trigger.")
			if err != nil {
				in.ApiResponse = &iota.IotaAPIResponse{ApiStatus: iota.APIResponseType_API_SERVER_ERROR,
					ErrorMsg: err.Error()}
				return in, nil
			}
			in.Commands[index] = twrap.triggerMsg.GetCommands()[0]
		}
	}

	logger.Infof("Completed running trigger.")
	in.ApiResponse = apiSuccess
	return in, nil
}

func (n *VcenterNode) runSerialTrigger(ctx context.Context, req *iota.TriggerMsg) (*iota.TriggerMsg, error) {

	for cidx, cmd := range req.GetCommands() {
		node, _ := n.managedNodes[cmd.NodeName]
		triggerInfo := &iota.TriggerMsg{Commands: []*iota.Command{cmd},
			TriggerMode: req.GetTriggerMode(), TriggerOp: req.GetTriggerOp()}
		if triggerResp, err := node.TriggerWithContext(ctx, triggerInfo); err != nil ||
			triggerResp.ApiResponse.ApiStatus != iota.APIResponseType_API_STATUS_OK {

			req.ApiResponse.ApiStatus = iota.APIResponseType_API_SERVER_ERROR
			req.ApiResponse.ErrorMsg = fmt.Sprintf("TOPO SVC | Trigger | RunSerialTrigger Call Failed. %v %v", err, triggerResp.ApiResponse.ErrorMsg)
			req.Commands[cidx] = triggerResp.GetCommands()[0]
			req.Commands[cidx].Stderr = triggerResp.ApiResponse.ErrorMsg
			req.Commands[cidx].ExitCode = 127
			return req, nil
		} else {
			req.Commands[cidx] = triggerResp.GetCommands()[0]
		}
		/* Only one command sent anyway */
	}
	req.ApiResponse = apiSuccess
	return req, nil
}

func (n *VcenterNode) runParallelTrigger(ctx context.Context, req *iota.TriggerMsg) (*iota.TriggerMsg, error) {

	triggerNodes := []TestNodeInterface{}
	triggerResp := &iota.TriggerMsg{TriggerMode: req.GetTriggerMode(),
		TriggerOp: req.GetTriggerOp()}

	triggerInfoMap := new(sync.Map)
	triggerRespMap := new(sync.Map)
	var triggerInfo *iota.TriggerMsg
	triggerCmdIndexMap := make(map[string][]int)

	for index, cmd := range req.GetCommands() {
		node, _ := n.managedNodes[cmd.NodeName]
		if item, ok := triggerInfoMap.Load(cmd.GetNodeName()); !ok {
			triggerMsg := &iota.TriggerMsg{Commands: []*iota.Command{cmd},
				TriggerMode: req.GetTriggerMode(), TriggerOp: req.GetTriggerOp()}
			triggerInfoMap.Store(node.GetNodeInfo().Name, triggerMsg)
			triggerInfo = triggerMsg
			triggerCmdIndexMap[node.GetNodeInfo().Name] = []int{}
		} else {
			triggerInfo = item.(*iota.TriggerMsg)
			triggerInfo.Commands = append(triggerInfo.Commands, cmd)
		}

		triggerCmdIndexMap[node.GetNodeInfo().Name] = append(triggerCmdIndexMap[node.GetNodeInfo().Name], index)
		//Just copy the request for now
		triggerResp.Commands = append(triggerResp.Commands, cmd)
		added := false
		for _, triggerNode := range triggerNodes {
			if triggerNode.GetNodeInfo().Name == node.GetNodeInfo().Name {
				added = true
				break
			}
		}
		if !added {
			triggerNodes = append(triggerNodes, node)
		}
	}
	// Triggers
	triggers := func(ctx2 context.Context) error {
		pool, ctx3 := errgroup.WithContext(ctx2)
		for _, node := range triggerNodes {
			node := node
			pool.Go(func() error {
				item, _ := triggerInfoMap.Load(node.GetNodeInfo().Name)
				triggerMsg := item.(*iota.TriggerMsg)
				triggerResp, err := node.TriggerWithContext(ctx3, triggerMsg)
				triggerRespMap.Store(node.GetNodeInfo().Name, triggerResp)
				if err != nil {
					return err
				}
				return nil
			})

		}
		return pool.Wait()
	}

	err := triggers(ctx)
	if err != nil {
		log.Errorf("TOPO SVC | Trigger | Trigger Call Failed. %v", err)
		req.ApiResponse.ApiStatus = iota.APIResponseType_API_SERVER_ERROR
		req.ApiResponse.ErrorMsg = fmt.Sprintf("TOPO SVC | Trigger | Trigger Call Failed. %v", err)
		return req, nil

	}

	triggerResp.ApiResponse = &iota.IotaAPIResponse{ApiStatus: iota.APIResponseType_API_STATUS_OK}
	for _, node := range triggerNodes {
		item, _ := triggerRespMap.Load(node.GetNodeInfo().Name)
		triggerMsg := item.(*iota.TriggerMsg)
		failed := false
		if triggerMsg.ApiResponse.ApiStatus != iota.APIResponseType_API_STATUS_OK {
			triggerResp.ApiResponse = triggerMsg.ApiResponse
			failed = true
		}
		for index, cmd := range triggerMsg.Commands {
			realIndex := triggerCmdIndexMap[node.GetNodeInfo().Name][index]
			triggerResp.Commands[realIndex] = cmd
			if failed {
				//Make sure we indicate to called all commands failed
				triggerResp.Commands[realIndex].ExitCode = 127
				triggerResp.Commands[realIndex].Stderr = triggerMsg.ApiResponse.ErrorMsg
			}
		}
	}

	return triggerResp, nil
}

// TriggerWithContext runs a trigger on workload
func (n *VcenterNode) TriggerWithContext(ctx context.Context,
	req *iota.TriggerMsg) (*iota.TriggerMsg, error) {
	log.Infof("TOPO SVC | DEBUG | Trigger. Received Request Msg: %v", req)
	defer log.Infof("TOPO SVC | DEBUG | Trigger Returned: %v", req)

	if req.TriggerOp == iota.TriggerOp_TYPE_NONE {
		log.Errorf("TOPO SVC | Trigger | Trigger call failed")
		req.ApiResponse = &iota.IotaAPIResponse{
			ApiStatus: iota.APIResponseType_API_BAD_REQUEST,
			ErrorMsg:  fmt.Sprintf("Trigger must specify TriggerOp for workload op. Found: %v", req.TriggerOp),
		}
		return req, nil
	}

	for _, cmd := range req.GetCommands() {
		if _, ok := n.managedNodes[cmd.NodeName]; !ok {
			req.ApiResponse.ApiStatus = iota.APIResponseType_API_BAD_REQUEST
			req.ApiResponse.ErrorMsg = fmt.Sprintf("Trigger command found to unprovisioned node : %v", cmd.NodeName)
			return req, nil
		}
	}

	if req.GetTriggerOp() == iota.TriggerOp_TERMINATE_ALL_CMDS ||
		req.GetTriggerMode() == iota.TriggerMode_TRIGGER_PARALLEL ||
		req.GetTriggerMode() == iota.TriggerMode_TRIGGER_NODE_PARALLEL {
		return n.runParallelTrigger(ctx, req)
	}

	return n.runSerialTrigger(ctx, req)
}
