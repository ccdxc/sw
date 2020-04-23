package cloud

import (
	"encoding/json"
	"errors"
	"fmt"
	"reflect"
	"strings"
	"time"

	uuid "github.com/satori/go.uuid"
	yaml "gopkg.in/yaml.v2"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/api/generated/cluster"
	"github.com/pensando/sw/events/generated/eventattrs"
	"github.com/pensando/sw/events/generated/eventtypes"
	"github.com/pensando/sw/iota/test/venice/iotakit/model/objects"
	"github.com/pensando/sw/iota/test/venice/iotakit/testbed"
	"github.com/pensando/sw/venice/utils/log"
)

//VerifyNaplesStatus verify naples status
func (sm *SysModel) VerifyNaplesStatus() error {

	//TODO
	return nil
	// verify each naples health
	for _, np := range sm.NaplesNodes {
		// check naples status

		/*
			err = sm.CheckNaplesHealth(np)
			if err != nil {
				log.Errorf("Naples health check failed. Err: %v", err)
				return err
			}
		*/

		// check smartnic status in Venice
		for _, inst := range np.Instances {
			dsc := inst.Dsc
			snic, err := sm.GetSmartNICByName(dsc.Status.PrimaryMAC)
			if err != nil {
				err := fmt.Errorf("Failed to get smartnc object for name %v. Err: %+v", dsc.Status.PrimaryMAC, err)
				log.Errorf("%v", err)
				return err
			}

			log.Debugf("Got smartnic object: %+v", snic)

			if snic.Status.AdmissionPhase != cluster.DistributedServiceCardStatus_ADMITTED.String() {
				log.Errorf("Invalid Naples status: %+v", snic)
				return fmt.Errorf("Invalid admin phase for naples %v. Status: %+v", np.Name(), snic.Status)
			}
			if len(snic.Status.Conditions) < 1 {
				log.Errorf("Invalid Naples status: %+v", snic)
				return fmt.Errorf("No naples status reported for naples %v", np.Name())
			}
			if snic.Status.Conditions[0].Type != cluster.DSCCondition_HEALTHY.String() {
				log.Errorf("Invalid Naples status: %+v", snic)
				return fmt.Errorf("Invalid status condition-type %v for naples %v", snic.Status.Conditions[0].Type, np.Name())
			}
			if snic.Status.Conditions[0].Status != cluster.ConditionStatus_TRUE.String() {
				log.Errorf("Invalid Naples status: %+v", snic)
				return fmt.Errorf("Invalid status %v for naples %v", snic.Status.Conditions[0].Status, np.Name())
			}
		}
	}

	// verify each naples health
	snics, err := sm.ListSmartNIC()
	if err != nil {
		log.Errorf("Unable to list dsc")
		return err
	}

	for _, snic := range snics {
		if snic.Status.AdmissionPhase != cluster.DistributedServiceCardStatus_ADMITTED.String() {
			log.Errorf("Invalid Naples status: %+v", snic)
			return fmt.Errorf("Invalid admin phase for naples %v. Status: %+v", snic.Spec.GetID(), snic.Status)
		}
		if len(snic.Status.Conditions) < 1 {
			log.Errorf("Invalid Naples status: %+v", snic)
			return fmt.Errorf("No naples status reported for naples %v", snic.Spec.GetID())
		}
		if snic.Status.Conditions[0].Type != cluster.DSCCondition_HEALTHY.String() {
			log.Errorf("Invalid Naples status: %+v", snic)
			return fmt.Errorf("Invalid status condition-type %v for naples %v", snic.Spec.GetID(), snic.Status.Conditions[0].Type)
		}
		if snic.Status.Conditions[0].Status != cluster.ConditionStatus_TRUE.String() {
			log.Errorf("Invalid Naples status: %+v", snic)
			return fmt.Errorf("Invalid status %v for naples %v", snic.Spec.GetID(), snic.Status.Conditions[0].Status)
		}
	}

	return nil

}
func (sm *SysModel) CollectLogs() error {
	//sm.SysModel.CollectLogs()
	log.Infof("Collecting logs disabled until penctl issue is fixed.")
	return nil
}

// VerifySystemHealth checks all aspects of system, like cluster, workload, policies etc
func (sm *SysModel) VerifySystemHealth(collectLogOnErr bool) error {
	const numRetries = 10
	// verify cluster is in good health
	err := sm.VerifyClusterStatus()
	if err != nil {
		if collectLogOnErr {
			sm.CollectLogs()
		}
		return err
	}

	/*
		// verify workload status is good
		if sm.Tb.HasNaplesHW() {
			err = sm.VerifyWorkloadStatus(sm.Workloads())
			if err != nil {
				if collectLogOnErr {
					sm.CollectLogs()
				}
				return err
			}
		} */

	//Verify Config is in sync

	for i := 0; i < numRetries; i++ {
		var done bool
		done, err = sm.IsConfigPushComplete()
		if done && err == nil {
			break
		}
		time.Sleep(5 * time.Second)

		err = errors.New("Config push incomplete")
	}
	if err != nil {
		return err
	}

	return nil
}

type RtrCtlStruct []struct {
	Spec struct {
		ID           string `json:"Id"`
		LocalAddr    string `json:"LocalAddr"`
		PeerAddr     string `json:"PeerAddr"`
		Password     bool   `json:"Password"`
		State        string `json:"State"`
		RemoteASN    int    `json:"RemoteASN"`
		SendComm     bool   `json:"SendComm"`
		SendExtComm  bool   `json:"SendExtComm"`
		ConnectRetry int    `json:"ConnectRetry"`
		HoldTime     uint32 `json:"HoldTime"`
		KeepAlive    uint32 `json:"KeepAlive"`
	} `json:"Spec"`
	Status struct {
		ID            string `json:"Id"`
		LastErrorRcvd string `json:"LastErrorRcvd"`
		LastErrorSent string `json:"LastErrorSent"`
		LocalAddr     string `json:"LocalAddr"`
		PeerAddr      string `json:"PeerAddr"`
		Status        string `json:"Status"`
		PrevStatus    string `json:"PrevStatus"`
	} `json:"Status"`
}

//VerifyBGPCluster verifies BGP cluster
func (sm *SysModel) VerifyBGPCluster() error {

	pegContainerCollection, err := sm.VeniceNodes().GetVeniceContainersWithService("pen-pegasus", true)

	if err != nil {
		return err
	}

	for _, pegContainer := range pegContainerCollection.Containers {
		output, stderr, exitCode, err := pegContainerCollection.RunCommand(pegContainer, "rtrctl show bgp peers --json")
		if exitCode != 0 || err != nil {
			log.Errorf("Error running command %v", stderr)
			return fmt.Errorf("Error runnig rtrctl command %v", stderr)
		}

		data := RtrCtlStruct{}
		err = json.Unmarshal([]byte(output), &data)
		if err != nil {
			return err
		}

		for _, np := range sm.NaplesNodes {
			verfied := false
			for _, inst := range np.Instances {
				ip := strings.Split(inst.LoopbackIP, "/")[0]
				for _, item := range data {
					if ip == item.Spec.PeerAddr && item.Status.Status == "ESTABLISHED" {
						verfied = true
						break
					}
				}
				if !verfied {
					msg := fmt.Sprintf("Naples %v (%v) not connected to bgp", np.IP(), ip)
					log.Errorf(msg)
					return errors.New(msg)
				}
			}
		}

		for _, np := range sm.FakeNaples {
			verfied := false
			ip := strings.Split(np.Instances[0].LoopbackIP, "/")[0]
			for _, item := range data {
				if ip == item.Spec.PeerAddr && item.Status.Status == "ESTABLISHED" {
					verfied = true
					break
				}
			}
			if !verfied {
				msg := fmt.Sprintf("Naples %v (%v) not connected to bgp", ip, np.Name())
				log.Errorf(msg)
				return errors.New(msg)
			}
		}

	}

	return nil
}

// this API currently checks only for cpu_mnic1 stats for errors.
// no error returned if the command execution fails
func (sm *SysModel) CheckNaplesForErrors() {

	cmds := 0
	trig := sm.Tb.NewTrigger()
	for _, node := range sm.Tb.Nodes {
		if testbed.IsNaplesHW(node.Personality) {
			for _, naples := range node.NaplesConfigs.Configs {
				trig.AddCommand("LD_LIBRARY_PATH=/nic/lib/ /nic/bin/eth_dbgtool stats 68 | grep -v ': 0'", naples.Name, node.NodeName)
				cmds++
			}
		}
	}

	//No naples, return
	if cmds == 0 {
		return
	}
	resp, err := trig.Run()
	if err != nil {
		return
	}

	// check the response
	match := false
	for _, cmdResp := range resp {
		if cmdResp.ExitCode == 0 {
			if strings.Contains(cmdResp.Stdout, "rx_queue_empty") {
				log.Infof("DPDK ERROR on %v: RX_QUEUE_EMPTY HAS NON-ZERO VALUE :%v", cmdResp.NodeName, cmdResp.Stdout)
				match = true
			}

			if strings.Contains(cmdResp.Stdout, "rx_desc_data_error") {
				log.Infof("DPDK ERROR on %v: RX_DESC_DATA_ERROR HAS NON-ZERO VALUE :%v", cmdResp.NodeName, cmdResp.Stdout)
				match = true
			}
		}
	}

	if match == false {
		log.Infof("No DPDK ERRORs")
	}
}

// VerifyClusterStatus verifies venice cluster status
func (sm *SysModel) VerifyClusterStatus() error {
	log.Infof("Verifying cluster health..")

	// check iota cluster health
	err := sm.Tb.CheckIotaClusterHealth()
	if err != nil {
		log.Errorf("Invalid Iota cluster state: %v", err)
		return err
	}

	if err := sm.VerifyVeniceStatus(); err != nil {
		log.Errorf("Venice status verification failed : %v", err)
		return err
	}

	if err := sm.VerifyNaplesStatus(); err != nil {
		log.Errorf("Naples status verification failed : %v", err)
		return err
	}

	if err := sm.VerifyBGPCluster(); err != nil {
		log.Errorf("BGP status verification failed : %v", err)
		return err
	}

	/*
		//Make sure config push in complete.
		if ok, err := sm.IsConfigPushComplete(); !ok || err != nil {
			log.Errorf("Config push incomplete")
			return errors.New("Config not in sync")
		}

	*/
	// verify ping is successful across all workloads
	if sm.Tb.HasNaplesHW() {
		for i := 0; i < 10; i++ {
			err = sm.PingPairs(sm.WorkloadPairs().WithinNetwork())
			if err == nil {
				break
			}
		}
		// check for dpdk errors. this can be removed later
		sm.CheckNaplesForErrors()

		if err != nil {
			return err
		}
	}
	return nil
}

// GetExclusiveServices node on the fly
func (sm *SysModel) GetExclusiveServices() ([]string, error) {
	return []string{"pen-pegasus"}, nil
}

// runCommandOnGivenNaples runs the given command on given naples and returns stdout
func (sm *SysModel) runCommandOnGivenNaples(np *objects.Naples, cmd string) (string, error) {
	trig := sm.Tb.NewTrigger()

	for _, inst := range np.Instances {
		trig.AddCommand(cmd, inst.EntityName, np.NodeName())
	}

	resp, err := trig.Run()
	if err != nil {
		return "", err
	}

	cmdResp := resp[0]
	if cmdResp.ExitCode != 0 {
		return "", fmt.Errorf("command failed: %+v", cmdResp)
	}

	return cmdResp.Stdout, nil
}

// PortFlap flaps one port from each naples in the collection
func (sm *SysModel) PortFlap(npc *objects.NaplesCollection) error {
	for _, naples := range npc.Nodes {
		naplesName := naples.NodeName()
		err, port := sm.getNaplesUplinkPort(naples, false)
		if err != nil {
			log.Errorf("Err: %v", err)
			return err
		}

		// flap port
		log.Infof("flapping port {%s} on naples {%v}", port, naplesName)
		portDownCmd := fmt.Sprintf("/nic/bin/pdsctl debug port --port %s --admin-state down", port)
		_, err = sm.runCommandOnGivenNaples(naples, portDownCmd)
		if err != nil {
			log.Errorf("command(%v) failed on naples: %v, Err: %v", portDownCmd, naplesName, err)
			return err
		}

		portUpCmd := fmt.Sprintf("/nic/bin/pdsctl debug port --port %s --admin-state up", port)
		_, err = sm.runCommandOnGivenNaples(naples, portUpCmd)
		if err != nil {
			log.Errorf("command(%v) failed on naples: %v, Err: %v", portUpCmd, naplesName, err)
			return err
		}
	}

	for _, naples := range npc.FakeNodes {
		naplesName := naples.NodeName()
		err, port := sm.getNaplesUplinkPort(naples, true)
		if err != nil {
			log.Errorf("Err: %v", err)
			return err
		}

		// flap port
		log.Infof("flapping port {%s} on naples {%v}", port, naplesName)
		portDownCmd := fmt.Sprintf("/naples/nic/bin/pdsctl debug port --port %s --admin-state down", port)
		_, err = sm.runCommandOnGivenNaples(naples, portDownCmd)
		if err != nil {
			log.Errorf("command(%v) failed on naples: %v, Err: %v", portDownCmd, naplesName, err)
			return err
		}

		portUpCmd := fmt.Sprintf("/naples/nic/bin/pdsctl debug port --port %s --admin-state up", port)
		_, err = sm.runCommandOnGivenNaples(naples, portUpCmd)
		if err != nil {
			log.Errorf("command(%v) failed on naples: %v, Err: %v", portUpCmd, naplesName, err)
			return err
		}
	}

	return nil
}

// LinkUpEventsSince returns all the link down events since the given time.
func (sm *SysModel) LinkUpEventsSince(since time.Time, npc *objects.NaplesCollection) *objects.EventsCollection {
	fieldSelector := fmt.Sprintf("severity=%s,type=%s,object-ref.kind=DistributedServiceCard,meta.mod-time>=%v", eventattrs.Severity_INFO, eventtypes.LINK_UP.String(), since.Format(time.RFC3339Nano))

	eventsList, err := sm.ListEvents(&api.ListWatchOptions{FieldSelector: fieldSelector})
	if err != nil {
		log.Errorf("failed to list events matching options: %v, err: %v", fieldSelector, err)
		return &objects.EventsCollection{}
	}

	return &objects.EventsCollection{List: eventsList}
}

// LinkDownEventsSince returns all the link down events since the given time.
func (sm *SysModel) LinkDownEventsSince(since time.Time, npc *objects.NaplesCollection) *objects.EventsCollection {
	fieldSelector := fmt.Sprintf("severity=%s,type=%s,object-ref.kind=DistributedServiceCard,meta.mod-time>=%v", eventattrs.Severity_WARN, eventtypes.LINK_DOWN.String(), since.Format(time.RFC3339Nano))

	eventsList, err := sm.ListEvents(&api.ListWatchOptions{FieldSelector: fieldSelector})
	if err != nil {
		log.Errorf("failed to list events matching options: %v, err: %v", fieldSelector, err)
		return &objects.EventsCollection{}
	}

	return &objects.EventsCollection{List: eventsList}
}

// StartEventsGenOnNaples generates SYSTEM_COLDBOOT events from the Naples events generation test app.
// TODO: Generate all possible events
func (sm *SysModel) StartEventsGenOnNaples(npc *objects.NaplesCollection, rate, count string) error {
	var naples *objects.Naples
	var naplesName string
	var genEvtCmd string

	if len(npc.Nodes) > 0 {
		naples = npc.Nodes[0]
		naplesName = naples.NodeName()
		genEvtCmd = fmt.Sprintf("PATH=$PATH:/nic/bin/; LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/nic/lib/:/nic/lib64/; export PATH; export LD_LIBRARY_PATH; OPERD_REGIONS=/nic/conf/operd-regions.json /nic/bin/alerts_gen -t 2 -r %s -n %s", rate, count)
	} else {
		naples = npc.FakeNodes[0]
		naplesName = naples.NodeName()
		genEvtCmd = fmt.Sprintf("PATH=$PATH:/naples/nic/bin/; LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/naples/nic/lib/:/naples/nic/lib64/; export PATH; export LD_LIBRARY_PATH; OPERD_REGIONS=/naples/nic/conf/operd-regions.json /naples/nic/bin/alerts_gen -t 2 -r %s -n %s", rate, count)
	}

	_, err := sm.runCommandOnGivenNaples(naples, genEvtCmd)
	if err != nil {
		log.Errorf("command(%v) failed on naples: %v, Err: %v", genEvtCmd, naplesName, err)
		return err
	}

	return nil
}

// SystemBootEvents returns all SYSTEM_COLDBOOT events.
func (sm *SysModel) SystemBootEvents(npc *objects.NaplesCollection) *objects.EventsCollection {
	fieldSelector := fmt.Sprintf("type=%s,object-ref.kind=DistributedServiceCard", eventtypes.SYSTEM_COLDBOOT.String())

	eventsList, err := sm.ListEvents(&api.ListWatchOptions{FieldSelector: fieldSelector})
	if err != nil {
		log.Errorf("failed to list events matching options: %v, err: %v", fieldSelector, err)
		return &objects.EventsCollection{}
	}

	return &objects.EventsCollection{List: eventsList}
}

// getNaplesUplinkPort returns the first uplink port from the output of "pdsctl show port status --yaml
func (sm *SysModel) getNaplesUplinkPort(naples *objects.Naples, fake bool) (error, string) {
	naplesName := naples.NodeName()

	var portStatusCmd string
	if fake {
		portStatusCmd = "/naples/nic/bin/pdsctl show port status --yaml"
	} else {
		portStatusCmd = "/nic/bin/pdsctl show port status --yaml"
	}

	out, err := sm.runCommandOnGivenNaples(naples, portStatusCmd)
	if err != nil {
		log.Errorf("command(%v) failed on naples: %v, Err: %v", portStatusCmd, naplesName, err)
		return err, ""
	}

	ports := strings.Split(out, "\r\n\r\n---\r\n")
	port := make(map[interface{}]interface{})
	for _, p := range ports {
		err = yaml.Unmarshal([]byte(p), &port)
		if err != nil {
			log.Errorf("Port unmarshal failed, Err %v", err)
			return err, ""
		}

		var portSpec map[interface{}]interface{}
	specLoop:
		for k, v := range port {
			switch k.(type) {
			case string:
				if k.(string) == "spec" {
					portSpec = v.(map[interface{}]interface{})
					break specLoop
				}
			}
		}

		var id []byte
		var typ int
		for k, v := range portSpec {
			switch k.(type) {
			case string:
				if k.(string) == "id" {
					y := reflect.ValueOf(v)
					for i := 0; i < y.Len(); i++ {
						id = append(id, byte(y.Index(i).Interface().(int)))
					}
				}
				if k.(string) == "type" {
					typ = v.(int)
				}
			}
		}
		if typ == 1 {
			return nil, uuid.FromBytesOrNil(id).String()
		}
	}

	return fmt.Errorf("Uplink port not found on %v", naplesName), ""
}
