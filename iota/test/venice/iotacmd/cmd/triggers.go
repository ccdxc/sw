package cmd

import (
	"strconv"
)

const (
	hostReboot         = "HostReboot"
	veniceReboot       = "VeniceReboot"
	flapDataPorts      = "FlapDataPorts"
	naplesRemoveAdd    = "NaplesRemoveAdd"
	naplesMgmtLinkFlap = "NaplesMgmtLinkFlap"
)

type triggerBase struct {
	TriggerName  string
	Percent      string
	SkipSetupVar bool
}

type trigger interface {
	Name() string
	Run() error
	SkipSetup() bool
}

type hostRebootTrigger struct {
	triggerBase
}

type veniceRebootTrigger struct {
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

func (h *triggerBase) Name() string {
	return h.TriggerName
}

func (h *triggerBase) SkipSetup() bool {
	return h.SkipSetupVar
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

func newHostRebootTrigger(name string, percent string, skipSetup bool) trigger {
	return &hostRebootTrigger{triggerBase: triggerBase{TriggerName: name, Percent: percent, SkipSetupVar: skipSetup}}
}

func newVeniceRebootTrigger(name string, percent string, skipSetup bool) trigger {
	return &veniceRebootTrigger{triggerBase: triggerBase{TriggerName: name, Percent: percent, SkipSetupVar: skipSetup}}
}

func newflapDataPortsTrigger(name string, percent string, skipSetup bool) trigger {
	return &flapDataPortsTrigger{triggerBase: triggerBase{TriggerName: name, Percent: percent, SkipSetupVar: skipSetup}}
}

func newNaplesRemoveAddTrigger(name string, percent string, skipSetup bool) trigger {
	return &naplesRemoveAddTrigger{triggerBase: triggerBase{TriggerName: name, Percent: percent, SkipSetupVar: skipSetup}}
}

func newNaplesMgmtLinkFlapTrigger(name string, percent string, skipSetup bool) trigger {
	return &naplesMgmtLinkFlapTrigger{triggerBase: triggerBase{TriggerName: name, Percent: percent, SkipSetupVar: skipSetup}}
}

var triggers = map[string]func(string, string, bool) trigger{
	hostReboot:         newHostRebootTrigger,
	veniceReboot:       newVeniceRebootTrigger,
	flapDataPorts:      newflapDataPortsTrigger,
	naplesRemoveAdd:    newNaplesRemoveAddTrigger,
	naplesMgmtLinkFlap: newNaplesMgmtLinkFlapTrigger,
}

func newTrigger(name string, percent string, skipSetup bool) trigger {
	if _, ok := triggers[name]; ok {
		return triggers[name](name, percent, skipSetup)
	}

	return nil

}
