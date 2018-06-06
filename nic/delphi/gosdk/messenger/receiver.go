package messenger

import (
	"bufio"
	"errors"
	"fmt"
	"io"
	"log"
	"net"

	"github.com/golang/protobuf/proto"

	"github.com/pensando/sw/nic/delphi/messanger/proto"
)

type transportHander interface {
	HandleMessage(message *delphi_messanger.Message) error
	SocketClosed()
}

type receiver struct {
	connection net.Conn
	reader     *bufio.Reader
	handler    transportHander
}

func (r *receiver) loopForever() {
	for {
		buf, err := r.reader.Peek(4)
		if err != nil {
			log.Printf("r.reader.Peek(4) - %s", err)
			return
		}
		length, used := proto.DecodeVarint(buf)

		discarded, err := r.reader.Discard(used)
		if discarded != used {
			panic(fmt.Sprintf("%s, %d != %d", err, discarded, used))
		}

		body := make([]byte, length)
		_, err = io.ReadFull(r.reader, body)
		if err != nil {
			panic(err)
		}

		var message delphi_messanger.Message
		err = proto.Unmarshal(body, &message)
		if err != nil {
			panic(err)
		}

		go r.handler.HandleMessage(&message)
	}
}

func newReceiver(conn net.Conn, handler transportHander) (*receiver, error) {
	reader := bufio.NewReader(conn)
	if reader == nil {
		panic(errors.New("Nil reader"))
	}
	r := &receiver{
		connection: conn,
		reader:     reader,
		handler:    handler,
	}

	go r.loopForever()

	return r, nil
}
