package server

import (
	"context"
	"encoding/json"
	"net"
	"time"

	"github.com/calmh/ipfix"

	"github.com/pensando/sw/venice/utils/log"
)

// NewServer starts a UDP ipfix collector
func NewServer(ctx context.Context, addr string) (string, chan ipfix.Message, error) {
	proto := "udp"
	session := ipfix.NewSession()

	l, err := net.ListenPacket(proto, addr)
	if err != nil {
		return "", nil, err
	}

	ch := make(chan ipfix.Message, 1)
	// start reader goroutine
	go func() {
		// close the channel
		defer func() {
			l.Close()
			close(ch)
		}()

		log.Infof("starting udp ipfix server %s", l.LocalAddr().String())
		for ctx.Err() == nil {
			buff := make([]byte, 10*1024)
			l.SetReadDeadline(time.Now().Add(time.Second))
			n, _, err := l.ReadFrom(buff)
			if err != nil {
				continue
			}

			ds, err := session.ParseBuffer(buff[:n])
			if err != nil {
				log.Errorf("ipfix parse error, %v", err)
				continue
			}

			log.Infof("server %v received: %+v ", l.LocalAddr().(*net.UDPAddr).String(), ds.Header)

			if len(ds.TemplateRecords) > 0 {
				log.Infof("templates %+v \n\n", ds.TemplateRecords)
				session.LoadTemplateRecords(ds.TemplateRecords)

				b, err := json.Marshal(ds.TemplateRecords)
				if err != nil {
					log.Errorf("failed to marshal, %v", err)
					continue
				}
				log.Infof("marshal: %v \n", string(b))
			} else {
				log.Infof("no templates, [%v]", ds)
			}

			ch <- ds

		}
	}()

	return l.LocalAddr().(*net.UDPAddr).String(), ch, nil
}
