package exporters

import (
	"context"
	"fmt"
	"sync"
	"time"

	evtsapi "github.com/pensando/sw/api/generated/events"
	"github.com/pensando/sw/venice/utils"
	"github.com/pensando/sw/venice/utils/events"
	"github.com/pensando/sw/venice/utils/log"
	"github.com/pensando/sw/venice/utils/syslog"
)

// Syslog exporter is designed to export events to multiple targets to align with event policy (syslog export config).
// Exports to different targets are sequential as of now. We can make it parallel if it becomes a bottleneck.

// Proxy through dispatcher -> syslog exporter -> syslog servers

// TODO: Implement field selector

// SyslogExporter represents the events syslog exporter
type SyslogExporter struct {
	// exporter details
	name   string     // name of the exporter
	chLen  int        // buffer or channel len
	logger log.Logger // logger

	// writers connecting to different syslog servers/targets
	writers []syslog.Writer

	// to receive events from the proxy (dispatcher)
	eventsChan          events.Chan          // channel to receive
	eventsOffsetTracker events.OffsetTracker // to track events file offset - bookmark indicating events till this point are processed successfully by this writer

	// context to use for any outgoing calls and to stop exporter go routines
	ctx        context.Context
	cancelFunc context.CancelFunc

	// to stop the exporter
	stop     sync.Once      // for stopping the exporter
	wg       sync.WaitGroup // to wait for the worker to finish
	shutdown chan struct{}  // to send shutdown signal to the exporter
}

// NewSyslogExporter creates a syslog exporter which sends events to different syslog servers/targets.
func NewSyslogExporter(name string, chLen int, writers []syslog.Writer,
	logger log.Logger) (events.Exporter, error) {
	if utils.IsEmpty(name) || chLen <= 0 || logger == nil {
		return nil, fmt.Errorf("exporter name, channel length and logger is required")
	}

	ctx, cancelFunc := context.WithCancel(context.Background())
	return &SyslogExporter{
		name:       name,
		chLen:      chLen,
		logger:     logger.WithContext("submodule", fmt.Sprintf("exporter.%s", name)),
		writers:    writers,
		shutdown:   make(chan struct{}, 1),
		ctx:        ctx,
		cancelFunc: cancelFunc,
	}, nil
}

// Start start the daemon go routine to receive events from the dispatcher
func (s *SyslogExporter) Start(eventsCh events.Chan, offsetTracker events.OffsetTracker) {
	s.eventsChan = eventsCh
	s.eventsOffsetTracker = offsetTracker

	// start watching events using the given event channel
	s.wg.Add(1)
	go s.receiveEvents()

	s.logger.Infof("started syslog events exporter {%s}", s.name)
}

// Stop stops the exporter
func (s *SyslogExporter) Stop() {
	s.logger.Infof("stopping the syslog events exporter {%s}", s.name)
	s.stop.Do(func() {
		if s.eventsChan != nil {
			s.eventsChan.Stop()
		}

		// cancel exporter context
		s.cancelFunc()

		// wait for the exporter to finish
		close(s.shutdown)

		s.wg.Wait()

		for _, writer := range s.writers {
			writer.Close()
		}

		fileOffset, _ := s.GetLastProcessedOffset()
		s.logger.Debugf("exporter {%s} stopping at offset: %v", s.name, fileOffset)
	})
}

// Name returns the name of the exporter
func (s *SyslogExporter) Name() string {
	return s.name
}

// ChLen returns the channel length; will be used to dispatcher to set channel len
func (s *SyslogExporter) ChLen() int {
	return s.chLen
}

// WriteEvents writes list of events to the syslog servers
func (s *SyslogExporter) WriteEvents(evts []*evtsapi.Event) error {
	var err error
	for _, evt := range evts {
		sMsg := s.GenerateSyslogMessage(evt)
		for _, writer := range s.writers {
			switch evtsapi.SeverityLevel(evtsapi.SeverityLevel_value[evt.GetSeverity()]) {
			case evtsapi.SeverityLevel_INFO:
				err = writer.Info(sMsg)
			case evtsapi.SeverityLevel_WARNING:
				err = writer.Warning(sMsg)
			case evtsapi.SeverityLevel_CRITICAL:
				err = writer.Crit(sMsg)
			}

			if err != nil {
				s.logger.Errorf("{exporter %s} failed to write syslog message: %v, err: %v", s.name, sMsg, err)
			}
		}
	}

	return err
}

// GetLastProcessedOffset returns the last bookmarked offset by this exporter
func (s *SyslogExporter) GetLastProcessedOffset() (int64, error) {
	return s.eventsOffsetTracker.GetOffset()
}

// startWorker watches the events using the event channel from dispatcher.
func (s *SyslogExporter) receiveEvents() {
	defer s.wg.Done()
	for {
		select {
		// this channel will be closed once the evenChan receives the stop signal from
		// this exporter or when dispatcher shuts down.
		case batch, ok := <-s.eventsChan.ResultChan():
			if !ok { // channel closed
				return
			}

			s.logger.Debugf("{exporter %s} received events: %v", s.name, events.Minify(batch.GetEvents()))

			// all the incoming batch of events needs to be processed in order to avoid losing track of events
			for {
				if err := s.ctx.Err(); err != nil {
					s.logger.Debugf("{exporter %s} context closed, err: %v", s.name, err)
					return
				}

				if err := s.WriteEvents(batch.GetEvents()); err != nil {
					s.logger.Debugf("{exporter %s} failed to send events to the syslog servers, retrying.. err: %v", s.name, err)
					time.Sleep(1 * time.Second)
					continue
				}

				// successfully sent the event(s) to syslog servers; update the file offset.
				s.eventsOffsetTracker.UpdateOffset(batch.GetOffset())
				break
			}
		case <-s.shutdown:
			fmt.Println("shutting down syslog exporter")
			return
		}
	}
}

// GenerateSyslogMessage helper function to generate syslog message from the given event
func (s *SyslogExporter) GenerateSyslogMessage(evt *evtsapi.Event) *syslog.Message {
	crTime, _ := evt.CreationTime.Time()
	modTime, _ := evt.ModTime.Time()
	strData := syslog.StrData{}
	strData["type"] = map[string]string{
		"kind": evt.GetKind(),
	}
	strData["meta"] = map[string]string{
		"name":          evt.GetName(),
		"uuid":          evt.GetUUID(),
		"tenant":        evt.GetTenant(),
		"namespace":     evt.GetNamespace(),
		"creation-time": crTime.String(),
		"mod-time":      modTime.String(),
	}
	strData["attributes"] = map[string]string{
		"type":     evt.GetType(),
		"severity": evt.GetSeverity(),
		"count":    fmt.Sprintf("%d", evt.GetCount()),
	}
	if evt.GetSource() != nil {
		strData["source"] = map[string]string{
			"node-name": evt.GetSource().GetNodeName(),
			"component": evt.GetSource().GetComponent(),
		}
	}

	if evt.GetObjectRef() != nil {
		strData["object-ref"] = map[string]string{
			"tenant":    evt.GetObjectRef().GetTenant(),
			"namespace": evt.GetObjectRef().GetNamespace(),
			"kind":      evt.GetObjectRef().GetKind(),
			"name":      evt.GetObjectRef().GetName(),
		}
	}

	return &syslog.Message{MsgID: evt.GetUUID(), Msg: evt.GetMessage(), StructuredData: strData}
}
