package gosdk

import (
	"bufio"
	"fmt"
	"io"
	"net"

	"github.com/golang/protobuf/descriptor"
	"github.com/golang/protobuf/proto"

	delphi_messenger "github.com/pensando/sw/nic/delphi/messenger/proto"
	"github.com/pensando/sw/nic/delphi/proto/delphi"
)

// Hub is the interface to the hub instance
type Hub interface {
	Start()
	Stop()
}

type hub struct {
	listener net.Listener
	quit     chan struct{}
	clients  []net.Conn
}

// NewHub creates a new hub instance
func NewHub() Hub {
	return &hub{
		quit:    make(chan struct{}),
		clients: make([]net.Conn, 0),
	}
}

func (h *hub) Start() {
	l, err := net.Listen("tcp", "127.0.0.1:7001")
	if err != nil {
		panic(err)
	}
	h.listener = l

	go h.runLoop()
}

func (h *hub) Stop() {
	h.quit <- struct{}{}
}

func (h *hub) runReceiver(conn net.Conn) {
	h.clients = append(h.clients, conn)
	r := bufio.NewReader(conn)
	for {
		buf, err := r.Peek(4)
		if err != nil {
			return
		}
		length, used := proto.DecodeVarint(buf)

		discarded, err := r.Discard(used)
		if discarded != used {
			panic(fmt.Sprintf("%s, %d != %d", err, discarded, used))
		}

		body := make([]byte, length)
		_, err = io.ReadFull(r, body)
		if err != nil {
			panic(err)
		}

		var message delphi_messenger.Message
		err = proto.Unmarshal(body, &message)
		if err != nil {
			panic(err)
		}

		h.handleMessage(conn, &message)
	}
}

func (h *hub) runAcceptor() {
	for {
		conn, err := h.listener.Accept()
		if err != nil {
			return
		}
		go h.runReceiver(conn)
	}
}

func (h *hub) runLoop() {
	go h.runAcceptor()
	for {
		select {
		case _ = <-h.quit:
			h.listener.Close()
			for _, c := range h.clients {
				c.Close()
			}
			return
		}
	}
}

func (h *hub) handleMessage(conn net.Conn, message *delphi_messenger.Message) {
	switch message.GetType() {
	case delphi_messenger.MessageType_MountReq:
		h.sendMountResp(conn, message)
	case delphi_messenger.MessageType_ChangeReq:
		for _, c := range h.clients {
			h.sendNotify(c, message)
		}
		h.sendStatus(conn)
	}
}

func (h *hub) sendMountResp(conn net.Conn, msg *delphi_messenger.Message) {
	var req delphi_messenger.MountReqMsg
	objects := msg.GetObjects()
	if len(objects) != 1 {
		panic("Unexpected number of objects")
	}
	err := proto.Unmarshal(objects[0].GetData(), &req)
	if err != nil {
		panic(err)
	}

	mountRsp := delphi_messenger.MountRespMsg{
		ServiceID:   req.GetServiceID(),
		ServiceName: req.GetServiceName(),
	}

	data, err := proto.Marshal(&mountRsp)
	if err != nil {
		panic(err)
	}

	_, desc := descriptor.ForMessage(&mountRsp)
	message := delphi_messenger.Message{
		Type:      delphi_messenger.MessageType_MountResp,
		MessageId: 1,
		Objects: []*delphi_messenger.ObjectData{
			&delphi_messenger.ObjectData{
				Meta: &delphi.ObjectMeta{
					Kind: *desc.Name,
				},
				Data: data,
			},
		},
	}

	e := make([]byte, 0)
	buffer := proto.NewBuffer(e)

	buffer.EncodeMessage(&message)

	_, err = conn.Write(buffer.Bytes())
	if err != nil {
		panic(err)
	}
}

func (h *hub) sendNotify(conn net.Conn, msg *delphi_messenger.Message) {
	message := delphi_messenger.Message{
		Type:      delphi_messenger.MessageType_Notify,
		MessageId: 1,
		Objects:   msg.GetObjects(),
	}

	e := make([]byte, 0)
	buffer := proto.NewBuffer(e)

	buffer.EncodeMessage(&message)

	_, err := conn.Write(buffer.Bytes())
	if err != nil {
		panic(err)
	}
}

func (h *hub) sendStatus(conn net.Conn) {
	message := delphi_messenger.Message{
		Type:      delphi_messenger.MessageType_StatusResp,
		MessageId: 1,
		Objects:   nil,
	}

	e := make([]byte, 0)
	buffer := proto.NewBuffer(e)

	buffer.EncodeMessage(&message)

	_, err := conn.Write(buffer.Bytes())
	if err != nil {
		panic(err)
	}
}
