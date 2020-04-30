package exporter

import (
	"context"
	"fmt"
	"strings"
	"sync"
	"time"

	"github.com/pensando/sw/api/generated/monitoring"
	eapiclient "github.com/pensando/sw/venice/ctrler/evtsmgr/apiclient"
	"github.com/pensando/sw/venice/ctrler/evtsmgr/memdb"
	"github.com/pensando/sw/venice/utils"
	"github.com/pensando/sw/venice/utils/log"
)

var (
	maxRetry = 5
)

//
// AlertExporter is responsible for exporting alerts:
//  1. Exports alerts to the given list of destinations `Export(destinations, alert)`.
//
//  2. Creates the exporters once, and reuses it for subsequent calls. Watcher updates the exporters (underlying connections)
//     when there is an update on the respective alert destination.
//     Each destination type is defined as an exporter e.g. Syslog, SNMP, etc. check the example below:
//     AlertDestination {
//      [] syslogServers // will be handled by syslog exporter
//      [] SNMPs         // will be handled by SNMP exporter
//      [] emailList     // will be handled by email exporter
//    }
//
//  3. Best effort will be taken to export the alert. There is no guarantees on the delivery.
//
//  4. Only syslog export is handled now. Other exports (SNMP, email) will be added in the future.
//
//  5. Alert destination will be updated with the number of notifications sent.
//

// AlertExporter represents the alert exporter
type AlertExporter struct {
	sync.Mutex
	memDb         *memdb.MemDb              // memDB to read alert destinations
	configWatcher *eapiclient.ConfigWatcher // API server client
	logger        log.Logger                // logger
	destinations  map[string][]Exporter     // map of destination names and it's respective list of exporters
	stop          chan struct{}             // to stop exporter
	once          sync.Once                 // to make stop idempotent
}

// NewAlertExporter creates a new exporter to export alerts to different destinations.
func NewAlertExporter(memDb *memdb.MemDb, configWatcher *eapiclient.ConfigWatcher, logger log.Logger) *AlertExporter {
	e := &AlertExporter{
		memDb:         memDb,
		configWatcher: configWatcher,
		logger:        logger,
		destinations:  make(map[string][]Exporter),
		stop:          make(chan struct{}),
	}

	// watch alert destination updates and update exporters accordingly
	go e.watchAlertDestinations()
	return e
}

// Export exports the given to alert to list of destinations.
// destNames list of named destination object references.
func (e *AlertExporter) Export(destNames []string, alert *monitoring.Alert) error {
	var errStrings []string
	var rErr error

	for _, destName := range destNames {
		if err := e.createExportersIfNotExist(destName); err != nil {
			e.logger.Errorf("failed to create exporter for destination %v, err: %v", destName, err)
			errStrings = append(errStrings, fmt.Sprintf("dest: %s, err: %s", destName, err.Error()))
			continue
		}

		numNotificationsSent := e.export(destName, alert)

		// update the total notifications on alert destination
		if err := e.updateAlertDestination(destName, numNotificationsSent); err != nil {
			e.logger.Errorf("failed to update alert destination %v, err: %v", destName, err)
		}
	}

	if len(errStrings) > 0 {
		rErr = fmt.Errorf(strings.Join(errStrings, "\n"))
	}

	return rErr
}

// Stop stops the exporter.
func (e *AlertExporter) Stop() {
	e.Lock()
	defer e.Unlock()

	e.once.Do(func() {
		close(e.stop)                              // stop watching alert destination
		for _, exporters := range e.destinations { // close the writers
			e.closeExporters(exporters)
		}
	})
}

// helper function to create exporters for the given destination if it does not exist already.
func (e *AlertExporter) createExportersIfNotExist(destName string) error {
	e.Lock()
	defer e.Unlock()
	_, found := e.destinations[destName] // check if the writers exists already

	if !found { // create writers for the given destination
		aDest := e.memDb.GetAlertDestination(destName)
		if aDest == nil {
			return fmt.Errorf("dest %v not found in memDB", destName)
		}
		exporters, err := e.createExporters(aDest)
		if err != nil {
			return err
		}

		e.destinations[destName] = exporters
	}

	return nil
}

// helper function to export the given alert to given destination.
// This function fans out the export to all the exporters and waits for it to complete.
func (e *AlertExporter) export(destName string, alert *monitoring.Alert) int {
	e.Lock()
	exporters := e.destinations[destName]
	defer e.Unlock()

	var wg sync.WaitGroup
	wg.Add(len(exporters))

	notificationsSent := make([]int, len(exporters))
	numNotificationsSent := 0

	for i, exporter := range exporters {
		go func(e Exporter, i int) {
			defer wg.Done()
			notificationsSent[i] = e.Export(alert)
		}(exporter, i)
	}

	wg.Wait() // wait for all the exporters to complete

	for _, numNotifications := range notificationsSent {
		numNotificationsSent += numNotifications
	}

	return numNotificationsSent
}

// watchAlertDestinations watches for the alert destination update/delete event and updates the
// writers in the exporter accordingly.
func (e *AlertExporter) watchAlertDestinations() {
	watcher := e.memDb.WatchAlertDestinations()
	defer e.memDb.StopWatchAlertDestinations(watcher)

	for {
		select {
		case <-e.stop:
			return
		case evt, ok := <-watcher.Channel:
			if !ok {
				e.logger.Errorf("error reading alert destination from the channel, closing")
				return
			}

			destObj := evt.Obj.(*monitoring.AlertDestination)
			destName := destObj.GetName()
			switch evt.EventType {
			case memdb.UpdateEvent:
				e.Lock()
				// close existing exporters if any
				exporters, found := e.destinations[destName]
				if found {
					e.closeExporters(exporters)
					delete(e.destinations, destName)
				}

				// create the exporters with new configuration
				if newExporters, err := e.createExporters(destObj); err != nil {
					e.logger.Errorf("failed to update the alert destination writers with new config, err: %v", err)
				} else {
					e.destinations[destName] = newExporters
				}
				e.Unlock()
			case memdb.DeleteEvent:
				e.Lock()
				// close existing exporters if any
				exporters, found := e.destinations[destName]
				if found {
					e.closeExporters(exporters)
				}
				delete(e.destinations, destName)
				e.Unlock()
			}
		}
	}
}

// updateAlertDestination helper function to update total notifications sent on the alert policy.
func (e *AlertExporter) updateAlertDestination(destName string, numNotificationsSent int) error {
	if e.configWatcher.APIClient() == nil {
		return fmt.Errorf("could not update alert destination")
	}

	ad, err := utils.ExecuteWithRetry(func(ctx context.Context) (interface{}, error) {
		aDest := e.memDb.GetAlertDestination(destName)
		if aDest != nil {
			ad, err := e.configWatcher.APIClient().MonitoringV1().AlertDestination().Get(ctx, aDest.GetObjectMeta()) // get the alert destination
			if err != nil {
				return nil, err
			}

			return ad, nil
		}

		return nil, fmt.Errorf("could not get alert destination object from memdb %s", destName)
	}, 3*time.Second, maxRetry)

	if err != nil {
		return err
	}

	aDest := ad.(*monitoring.AlertDestination)
	aDest.Status.TotalNotificationsSent += int32(numNotificationsSent)

	_, err = utils.ExecuteWithRetry(func(ctx context.Context) (interface{}, error) {
		_, err := e.configWatcher.APIClient().MonitoringV1().AlertDestination().Update(ctx, aDest)
		return nil, err
	}, 3*time.Second, maxRetry)

	return err
}

// helper function to create exporters for the given destination object.
// It is responsible for creating exporter for each destination type.
func (e *AlertExporter) createExporters(destObj *monitoring.AlertDestination) ([]Exporter, error) {
	var exporters []Exporter
	if destObj.Spec.SyslogExport != nil { // create syslog exporter
		exporter, err := NewSyslogExporter(destObj.Spec.SyslogExport, e.logger.WithContext("submodule", "syslog_alert_exporter"))
		if err != nil {
			e.logger.Errorf("failed to create syslog alert exporter, err: %v", err)
			return nil, err
		}
		exporters = append(exporters, exporter)
	}

	// TODO: create SNMP and email exporter

	return exporters, nil
}

// helper function to close the given list of exporters.
func (e *AlertExporter) closeExporters(exporters []Exporter) {
	for _, exporter := range exporters {
		exporter.Close()
	}
}
