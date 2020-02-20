package syslog

import (
	"context"
	"fmt"
	"net"
	"strings"
	"time"

	"github.com/jeromer/syslogparser"
	"github.com/jeromer/syslogparser/rfc3164"
	rfc5424u "github.com/jeromer/syslogparser/rfc5424"

	"github.com/pensando/sw/api/generated/monitoring"
)

func parseRfc3164(buff []byte) (syslogparser.LogParts, error) {
	p := rfc3164.NewParser(buff)
	if err := p.Parse(); err != nil {
		fmt.Printf("failed to create new Rfc3164 parser for %v, %s \n", string(buff), err)
		return nil, err
	}
	return p.Dump(), nil
}

func parseRfc5424(buff []byte) (syslogparser.LogParts, error) {
	p := rfc5424u.NewParser(buff)
	if err := p.Parse(); err != nil {
		fmt.Printf("failed to create new Rfc5424 parser for %v, %v\n", string(buff), err)
		return nil, err
	}
	return p.Dump(), nil
}

func parseSyslog(logType string, buff []byte) (syslogparser.LogParts, error) {
	switch logType {
	case monitoring.MonitoringExportFormat_SYSLOG_BSD.String():
		lp, err := parseRfc3164(buff)
		if err != nil {
			fmt.Printf("stop syslog, %v \n", err)
			return nil, err
		}
		return lp, nil
	case monitoring.MonitoringExportFormat_SYSLOG_RFC5424.String():
		lp, err := parseRfc5424(buff)
		if err != nil {
			fmt.Printf("stop syslog, %v\n", err)
			return nil, err
		}
		return lp, nil
	}
	return nil, fmt.Errorf("invalid type %s", logType)
}

// Server starts a syslog server
func Server(ctx context.Context, addr string, logType string, proto string) (string, chan syslogparser.LogParts, error) {
	ch := make(chan syslogparser.LogParts, 5)
	if proto == "udp" {
		l, err := net.ListenPacket(proto, addr)
		if err != nil {
			return "", nil, err
		}

		// start reader goroutine
		go func() {
			// close the channel
			defer func() {
				l.Close()
				close(ch)
			}()

			numMsg := map[string]int{} // updated by single goroutine
			fmt.Printf("udp syslog server %s ready \n", l.LocalAddr().String())
			for ctx.Err() == nil {
				buff := make([]byte, 1024)
				l.SetReadDeadline(time.Now().Add(time.Second * 30))
				n, raddr, err := l.ReadFrom(buff)
				if err != nil {
					for _, s := range []string{"closed network connection", "EOF"} {
						if strings.Contains(err.Error(), s) {
							fmt.Printf("%s closed\n", l.LocalAddr().String())
							return
						}
					}
					continue
				}
				numMsg[raddr.String()]++
				fmt.Printf("[%v] server %v received: %v\n", numMsg[raddr.String()], l.LocalAddr().(*net.UDPAddr).String(), string(buff[:n]))
				lp, err := parseSyslog(logType, buff[:n])
				if err != nil {
					fmt.Printf("stop syslog server %v\n", err)
					return
				}
				ch <- lp
			}
		}()
		return l.LocalAddr().(*net.UDPAddr).String(), ch, nil
	} else if proto == "tcp" {
		l, err := net.Listen(proto, addr)
		if err != nil {
			return "", nil, err
		}

		// start reader goroutine
		go func() {
			defer func() {
				close(ch)
				l.Close()
			}()

			for ctx.Err() == nil {
				nc, err := l.Accept()
				if err != nil {
					fmt.Printf("failed to accept connection, %s\n", err)
					return
				}

				go func(conn net.Conn) {
					defer func() { conn.Close() }()

					numMsg := 0
					fmt.Printf("tcp syslog server %s ready %v\n", l.Addr().String(), conn.RemoteAddr().String())
					for ctx.Err() == nil {
						buff := make([]byte, 1024)
						conn.SetReadDeadline(time.Now().Add(time.Second * 30))
						n, err := conn.Read(buff)
						if err != nil {
							for _, s := range []string{"closed network connection", "EOF"} {
								if strings.Contains(err.Error(), s) {
									fmt.Printf("%s closed \n", l.Addr().String())
									return
								}
							}
							continue
						}

						numMsg++
						fmt.Printf("[%v] server %v received: %v \n", numMsg, l.Addr().String(), string(buff[:n]))
						lp, err := parseSyslog(logType, buff[:n])
						if err != nil {
							fmt.Printf("stop syslog server %v\n", err)
							return
						}
						ch <- lp

					}
				}(nc)
			}
		}()
		return l.Addr().(*net.TCPAddr).String(), ch, nil

	}
	return "", nil, fmt.Errorf("invalid protocol %s", proto)
}
