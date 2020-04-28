package base

import (
	"context"
	"errors"
	"math/rand"
	"strings"
	"time"

	"github.com/pensando/sw/iota/test/venice/iotakit/model/objects"
	"github.com/pensando/sw/venice/utils/log"
)

//TriggerNaplesUpgrade trigger naples upgrade
func (sm *SysModel) TriggerNaplesUpgrade(percent int) error {

	log.Infof("Running Naples upgrade trigger.")
	numNaples := 0
	sm.ForEachNaples(func(nc *objects.NaplesCollection) error {
		_, err := sm.RunNaplesCommand(nc, "touch /data/upgrade_to_same_firmware_allowed")
		numNaples++
		return err
	})

	defer sm.ForEachNaples(func(nc *objects.NaplesCollection) error {
		_, err := sm.RunNaplesCommand(nc, "rm /data/upgrade_to_same_firmware_allowed")
		return err
	})

	sm.ForEachFakeNaples(func(nc *objects.NaplesCollection) error {
		numNaples++
		return nil
	})

	rollout, err := sm.GetRolloutObject(true)
	if err != nil {
		return err
	}

	err = sm.PerformRollout(rollout, true, "upgrade-bundle")
	if err != nil {
		return err
	}

	TimedOutEvent := time.After(time.Duration(60*numNaples) * time.Second)
	for true {
		select {
		case <-TimedOutEvent:
			return errors.New("Error waiting for upgrade, timed out")
		default:
			err = sm.VerifyRolloutStatus(rollout.Name)
			if err == nil {
				//Sleep for while to make sure all naples are connected
				time.Sleep(60 * time.Second)
				return nil
			}
		}
	}

	return nil
}

//TriggerSnapshotRestore triggers snap shot restore
func (sm *SysModel) TriggerSnapshotRestore(percent int) error {
	log.Infof("Running Snapshot restore trigger.")
	err := sm.VeniceNodeCreateSnapshotConfig(sm.VeniceNodes())
	ss, err := sm.VeniceNodeTakeSnapshot(sm.VeniceNodes())
	name := string(ss[strings.LastIndex(ss, "/")+1:])
	sm.VeniceNodeRestoreConfig(sm.VeniceNodes(), name)
	return err
}

//TriggerHostReboot trigger host reboots
func (sm *SysModel) TriggerHostReboot(percent int) error {
	log.Infof("Running Host reboot trigger.")
	hc, err := sm.Hosts().SelectByPercentage(percent)

	if err != nil {
		return err
	}
	return sm.ReloadHosts(hc)
}

//TriggerVeniceReboot trigger venice reboots
func (sm *SysModel) TriggerVeniceReboot(percent int) error {
	log.Infof("Running Venice reboot trigger.")
	vnc, err := sm.VeniceNodes().SelectByPercentage(percent)

	if err != nil {
		return err
	}
	return sm.ReloadVeniceNodes(vnc)
}

//TriggerVenicePartition parition venice nodes
func (sm *SysModel) TriggerVenicePartition(percent int) error {
	log.Infof("Running Venice parition trigger.")
	vnc, err := sm.VeniceNodes().SelectByPercentage(percent)

	if err != nil {
		return err
	}

	naples := sm.Naples()
	err = sm.DisconnectVeniceNodesFromCluster(vnc, naples)
	if err != nil {
		return err
	}
	time.Sleep(120 * time.Second)

	err = sm.ConnectVeniceNodesToCluster(vnc, naples)
	if err != nil {
		return err
	}

	return nil
}

//TriggerLinkFlap triggers link flap
func (sm *SysModel) TriggerLinkFlap(percent int) error {
	log.Infof("Running Link Flap trigger.")
	sc, err := sm.SwitchPorts().SelectByPercentage(percent)

	if err != nil {
		return err
	}
	return sm.FlapDataSwitchPorts(sc, 60*time.Second)
}

//TriggerDeleteAddConfig triggers link flap
func (sm *SysModel) TriggerDeleteAddConfig(percent int) error {

	err := sm.CleanupAllConfig()
	if err != nil {
		return err
	}

	err = sm.TeardownWorkloads(sm.Workloads())
	if err != nil {
		return err
	}

	return sm.SetupDefaultConfig(context.Background(), sm.Scale, sm.ScaleData)

}

type triggerFunc func(int) error

//RunRandomTrigger runs a random trigger
func (sm *SysModel) RunRandomTrigger(percent int) error {

	triggers := []triggerFunc{
		sm.TriggerDeleteAddConfig,
		sm.TriggerSnapshotRestore,
		sm.TriggerHostReboot,
		sm.TriggerVeniceReboot,
		sm.TriggerVenicePartition,
		sm.TriggerLinkFlap,
		sm.TriggerNaplesUpgrade,
	}

	index := rand.Intn(len(triggers))
	return triggers[index](percent)
}
