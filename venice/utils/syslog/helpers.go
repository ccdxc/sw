package syslog

import (
	"context"
	"fmt"
	"net"
	"strconv"
	"strings"

	"github.com/pensando/sw/api/generated/monitoring"
	"github.com/pensando/sw/venice/utils"
	"github.com/pensando/sw/venice/utils/log"
)

// converts structured data to a string
func (sd StrData) stringify() string {
	var str string
	if sd != nil {
		for sdKey, sdParam := range sd {
			str += fmt.Sprintf("[%s", sdKey)
			for k, v := range sdParam {
				str += fmt.Sprintf(" %s=\"%s\"", k, v)
			}
			str += fmt.Sprintf("]")
		}
	} else {
		str += "-"
	}

	return str
}

// CreateSyslogWriters helper function to create all the required syslog writers from the config
func CreateSyslogWriters(ctx context.Context, defaultTag, hostname string, syslogExportCfg *monitoring.SyslogExportConfig, format string,
	targets []*monitoring.ExportConfig) (map[string]*WriterConfig, error) {
	priority := LogUser // is a combination of facility and priority
	if !utils.IsEmpty(syslogExportCfg.GetFacilityOverride()) {
		priority = Priority(monitoring.SyslogFacility_vvalue[syslogExportCfg.GetFacilityOverride()])
	}

	tag := defaultTag // a.k.a prefix tag
	if !utils.IsEmpty(syslogExportCfg.GetPrefix()) {
		tag = syslogExportCfg.GetPrefix()
	}

	writers := make(map[string]*WriterConfig)
	for _, target := range targets {
		tmp := strings.Split(target.GetTransport(), "/")                                                      // e.g. transport = tcp/514
		network, remoteAddr := strings.ToLower(tmp[0]), fmt.Sprintf("%s:%s", target.GetDestination(), tmp[1]) // {tcp, udp, etc.}, <remote_addr>:<port>
		writer, err := CreateSyslogWriter(ctx, hostname, monitoring.MonitoringExportFormat(monitoring.MonitoringExportFormat_vvalue[format]), network, remoteAddr, tag, priority)
		if err != nil {
			return nil, err
		}

		writerKey := GetWriterKey(*target)
		writers[writerKey] = &WriterConfig{
			Format:     format,
			Network:    network,
			RemoteAddr: remoteAddr,
			Tag:        tag,
			Priority:   priority,
			Writer:     writer,
		}
	}

	return writers, nil
}

// CreateSyslogWriter helper function to create a syslog writer using the given params
func CreateSyslogWriter(ctx context.Context, hostname string, format monitoring.MonitoringExportFormat, network, remoteAddr, tag string, priority Priority) (Writer, error) {
	var writer Writer
	var err error

	switch format {
	case monitoring.MonitoringExportFormat_SYSLOG_BSD:
		writer, err = NewBsd(network, remoteAddr, priority, hostname, tag, BSDWithContext(ctx))
		if err != nil {
			log.Errorf("failed to create syslog BSD writer for config {%s/%s} (will try reconnecting in few secs), err: %v", remoteAddr, network, err)
		}
	case monitoring.MonitoringExportFormat_SYSLOG_RFC5424:
		writer, err = NewRfc5424(network, remoteAddr, priority, hostname, tag, RFCWithContext(ctx))
		if err != nil {
			log.Errorf("failed to create syslog RFC5424 writer {%s/%s} (will try reconnecting in few secs), err: %v", remoteAddr, network, err)
		}
	default:
		return nil, fmt.Errorf("invalid syslog format {%v}", format)
	}

	return writer, nil
}

// GetWriterKey constructs and returns the writer key from given export config
func GetWriterKey(target monitoring.ExportConfig) string {
	return fmt.Sprintf("%s:%s", target.GetDestination(), target.GetTransport())
}

// ValidateDestination validates ip/hostname string
func ValidateDestination(destination string) error {

	if utils.IsEmpty(destination) {
		return fmt.Errorf("destination cannot be empty")
	}

	netIP := net.ParseIP(destination)
	if netIP == nil { // treat it as hostname and resolve
		if _, err := net.LookupHost(destination); err != nil {
			return fmt.Errorf("failed to resolve destination {%s}, err: %v", destination, err)
		}
	}

	return nil
}

// ValidateTransport validates transport string is of the form udp/<port> or tcp/<port> (case insensitive)
func ValidateTransport(transport string) error {
	tr := strings.Split(transport, "/")
	if len(tr) != 2 {
		return fmt.Errorf("transport should be in protocol/port format")
	}

	// check protocol
	if _, ok := map[string]bool{
		"tcp": true,
		"udp": true,
	}[strings.ToLower(tr[0])]; !ok {
		return fmt.Errorf("invalid protocol in %s\n Accepted protocols: TCP, UDP", transport)
	}

	// check port
	port, err := strconv.Atoi(tr[1])
	if err != nil {
		return fmt.Errorf("invalid port in %s", transport)
	}
	if uint(port) > uint(^uint16(0)) {
		return fmt.Errorf("invalid port (> %d) in %s", ^uint16(0), transport)
	}

	return nil
}
