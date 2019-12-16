package cmd

import (
	"strconv"
)

const (
	hostReboot         = "HostReboot"
	veniceReboot       = "VeniceReboot"
	venicePartition    = "VenicePartition"
    veniceShutdownLeader = "VeniceShutdownLeader"
    veniceShutdownNpmNode = "VeniceShutdownNpmNode"
    veniceShutdownApiServerNode = "VeniceShutdownApiServerNode"
	flapDataPorts      = "FlapDataPorts"
	naplesRemoveAdd    = "NaplesRemoveAdd"
	naplesMgmtLinkFlap = "NaplesMgmtLinkFlap"
	naplesUpgrade      = "NaplesUpgrade"

	genEvents       = "GenEvents"
	genFWLogs       = "GenFWLogs"
	flapSwitchPorts = "FlapPorts"
)

type triggerBase struct {
	TriggerName string
	Percent     string
}

type trigger interface {
	Name() string
	Run() error
}

type hostRebootTrigger struct {
	triggerBase
}

type veniceRebootTrigger struct {
	triggerBase
}

type veniceShutdownLeaderTrigger struct {
	triggerBase
}

type veniceShutdownNpmNodeTrigger struct {
	triggerBase
}

type veniceShutdownApiServerNodeTrigger struct {
	triggerBase
}

type venicePartitionTrigger struct {
	triggerBase
}

type flapDataPortsTrigger struct {
	triggerBase
}

type naplesRemoveAddTrigger struct {
	triggerBase
}

type naplesMgmtLinkFlapTrigger struct {
	triggerBase
}

type naplesUpgradeTrigger struct {
	triggerBase
}

func (h *triggerBase) Name() string {
	return h.TriggerName
}

func (h *triggerBase) getPercent() (int, error) {
	if (h.Percent) == "" {
		h.Percent = "100"
	}
	percent, err := strconv.Atoi(h.Percent)
	if err != nil {
		return 0, err
	}

	return percent, nil
}

func (h *hostRebootTrigger) Run() error {
	percent, err := h.triggerBase.getPercent()
	if err != nil {
		return err
	}
	return rebootHosts(percent)
}

func (h *veniceRebootTrigger) Run() error {
	percent, err := h.triggerBase.getPercent()
	if err != nil {
		return err
	}
	return rebootVeniceNodes(percent)
}

func (h *venicePartitionTrigger) Run() error {
	percent, err := h.triggerBase.getPercent()
	if err != nil {
		return err
	}
	return partitionVeniceNode(percent)
}

func (h *veniceShutdownLeaderTrigger) Run() error {
	return shutdownVeniceLeaderNode()
}

func (h *veniceShutdownNpmNodeTrigger) Run() error {
	return shutdownVeniceNpmNode()
}

func (h *veniceShutdownApiServerNodeTrigger) Run() error {
	return shutdownVeniceApiServerNode()
}

func (h *flapDataPortsTrigger) Run() error {
	percent, err := h.triggerBase.getPercent()
	if err != nil {
		return err
	}
	return flapPorts(percent)
}

func (h *naplesRemoveAddTrigger) Run() error {
	percent, err := h.triggerBase.getPercent()
	if err != nil {
		return err
	}
	return doNaplesRemoveAdd(percent)
}

func (h *naplesMgmtLinkFlapTrigger) Run() error {
	percent, err := h.triggerBase.getPercent()
	if err != nil {
		return err
	}
	return doNaplesMgmtLinkFlap(percent)
}

func (h *naplesUpgradeTrigger) Run() error {
	percent, err := h.triggerBase.getPercent()
	if err != nil {
		return err
	}
	return doNaplesUpgrade(percent)
}

func newHostRebootTrigger(name string, percent string) trigger {
	return &hostRebootTrigger{triggerBase: triggerBase{TriggerName: name, Percent: percent}}
}

func newVeniceRebootTrigger(name string, percent string) trigger {
	return &veniceRebootTrigger{triggerBase: triggerBase{TriggerName: name, Percent: percent}}
}

func newVenicePartitionTrigger(name string, percent string) trigger {
	return &venicePartitionTrigger{triggerBase: triggerBase{TriggerName: name, Percent: percent}}
}

func newVeniceShutdownLeaderTrigger(name string, percent string) trigger {
	return &veniceShutdownLeaderTrigger{triggerBase: triggerBase{TriggerName: name, Percent: percent}}
}

func newVeniceShutdownNpmNodeTrigger(name string, percent string) trigger {
	return &veniceShutdownNpmNodeTrigger{triggerBase: triggerBase{TriggerName: name, Percent: percent}}
}

func newVeniceShutdownApiServerNodeTrigger(name string, percent string) trigger {
	return &veniceShutdownApiServerNodeTrigger{triggerBase: triggerBase{TriggerName: name, Percent: percent}}
}

func newflapDataPortsTrigger(name string, percent string) trigger {
	return &flapDataPortsTrigger{triggerBase: triggerBase{TriggerName: name, Percent: percent}}
}

func newNaplesRemoveAddTrigger(name string, percent string) trigger {
	return &naplesRemoveAddTrigger{triggerBase: triggerBase{TriggerName: name, Percent: percent}}
}

func newNaplesMgmtLinkFlapTrigger(name string, percent string) trigger {
	return &naplesMgmtLinkFlapTrigger{triggerBase: triggerBase{TriggerName: name, Percent: percent}}
}

func newNaplesUpgradeTrigger(name string, percent string) trigger {
	return &naplesUpgradeTrigger{triggerBase: triggerBase{TriggerName: name, Percent: percent}}
}

var triggers = map[string]func(string, string) trigger{
	hostReboot:         newHostRebootTrigger,
	veniceReboot:       newVeniceRebootTrigger,
	flapDataPorts:      newflapDataPortsTrigger,
	naplesRemoveAdd:    newNaplesRemoveAddTrigger,
	naplesMgmtLinkFlap: newNaplesMgmtLinkFlapTrigger,
	naplesUpgrade:      newNaplesUpgradeTrigger,
	venicePartition:    newVenicePartitionTrigger,
    veniceShutdownLeader: newVeniceShutdownLeaderTrigger,
    veniceShutdownNpmNode: newVeniceShutdownNpmNodeTrigger,
    veniceShutdownApiServerNode: newVeniceShutdownApiServerNodeTrigger,
}

func newTrigger(name string, percent string) trigger {
	if _, ok := triggers[name]; ok {
		return triggers[name](name, percent)
	}

	return nil

}

type noiseBase struct {
	Name  string
	Rate  string
	Count string
}

type noise interface {
	Name() string
	Run() error
	Stop() error
}

type genEventsNoise struct {
	noiseBase
}

type genFWLogsNoise struct {
	noiseBase
}

type flapPortsNoise struct {
	noiseBase
}

func (*genFWLogsNoise) Name() string {
	return genFWLogs
}

func (n *genFWLogsNoise) Run() error {
	return generateFWLogs(n.Rate, n.Count)
}

func (n *genFWLogsNoise) Stop() error {
	return stopFWLogs(n.Rate, n.Count)
}

func newGenFWLogsNoise(name string, rate string, count string) noise {
	return &genFWLogsNoise{noiseBase: noiseBase{Name: name, Rate: rate, Count: count}}
}

func (n *genEventsNoise) Run() error {
	return generateEvents(n.Rate, n.Count)
}

func (n *genEventsNoise) Stop() error {
	return stopEvents(n.Rate, n.Count)
}

func newGenEventsNoise(name string, rate string, count string) noise {
	return &genEventsNoise{noiseBase: noiseBase{Name: name, Rate: rate, Count: count}}
}

func (*genEventsNoise) Name() string {
	return genEvents
}

func (*flapPortsNoise) Name() string {
	return flapSwitchPorts
}

func (n *flapPortsNoise) Run() error {
	rate, _ := strconv.Atoi(n.Rate)
	count, _ := strconv.Atoi(n.Count)
	return startFlapPortsPeriodically(rate, count, 50)
}

func (n *flapPortsNoise) Stop() error {
	return nil
}

func newFlapPortsNoise(name string, rate string, count string) noise {
	return &flapPortsNoise{noiseBase: noiseBase{Name: name, Rate: rate, Count: count}}
}

var noises = map[string]func(string, string, string) noise{
	genEvents:       newGenEventsNoise,
	genFWLogs:       newGenFWLogsNoise,
	flapSwitchPorts: newFlapPortsNoise,
}

func newNoise(name, rate, count string) noise {
	if _, ok := noises[name]; ok {
		return noises[name](name, rate, count)
	}

	return nil
}
