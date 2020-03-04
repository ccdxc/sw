package cloud

import (
	"encoding/json"
	"errors"
	"fmt"
	"strings"
	"time"

	"github.com/pensando/sw/api/generated/cluster"
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
		snic, err := sm.GetSmartNICByName(np.Name())
		if err != nil {
			err := fmt.Errorf("Failed to get smartnc object for name %v. Err: %+v", np.Name(), err)
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
			ip := strings.Split(np.LoopbackIP, "/")[0]
			for _, item := range data {
				if ip == item.Spec.PeerAddr && item.Status.Status == "BGP_PEER_STATE_ESTABLISHED" {
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

		for _, np := range sm.FakeNaples {
			verfied := false
			ip := strings.Split(np.LoopbackIP, "/")[0]
			for _, item := range data {
				if ip == item.Spec.PeerAddr && item.Status.Status == "BGP_PEER_STATE_ESTABLISHED" {
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

			// verify ping is successful across all workloads
			if sm.Tb.HasNaplesHW() {
				for i := 0; i < 10; i++ {
					err = sm.PingPairs(sm.WorkloadPairs().WithinNetwork())
					if err == nil {
						break
					}
				}
				if err != nil {
					return err
				}
			}
	*/
	return nil
}

// GetExclusiveServices node on the fly
func (sm *SysModel) GetExclusiveServices() ([]string, error) {
	return []string{"pen-pegasus"}, nil
}
