package gosdk

import (
	"bufio"
	"fmt"
	"io"
	"net"

	"github.com/golang/protobuf/descriptor"
	"github.com/golang/protobuf/proto"

	messenger "github.com/pensando/sw/nic/delphi/messenger/proto"
	"github.com/pensando/sw/nic/delphi/proto/delphi"
)

// Hub is the interface to the hub instance
type Hub interface {
	Start()
	Stop()
}

type hubClient struct {
	connection    net.Conn
	subscriptions map[string]struct{}
}

type hub struct {
	listener net.Listener
	quit     chan struct{}
	clients  map[net.Conn]*hubClient
}

// NewFakeHub creates a new hub instance
func NewFakeHub() Hub {
	return &hub{
		quit:    make(chan struct{}),
		clients: make(map[net.Conn]*hubClient),
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
	if h.listener != nil {
		h.listener.Close()
		h.listener = nil
	}
	h.quit <- struct{}{}
}

func (h *hub) runReceiver(conn net.Conn) {
	h.clients[conn] = &hubClient{
		connection:    conn,
		subscriptions: make(map[string]struct{}),
	}
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

		var message messenger.Message
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
			for _, c := range h.clients {
				c.connection.Close()
			}
			return
		}
	}
}

func (h *hub) handleMessage(conn net.Conn, message *messenger.Message) {
	switch message.GetType() {
	case messenger.MessageType_MountReq:
		h.sendMountResp(h.clients[conn], message)
	case messenger.MessageType_ChangeReq:
		for _, c := range h.clients {
			h.sendNotify(c, message)
		}
		h.sendStatus(conn)
	}
}

func (h *hub) sendMountResp(cl *hubClient, msg *messenger.Message) {
	var req messenger.MountReqMsg
	objects := msg.GetObjects()
	if len(objects) != 1 {
		panic("Unexpected number of objects")
	}
	err := proto.Unmarshal(objects[0].GetData(), &req)
	if err != nil {
		panic(err)
	}

	for _, m := range req.Mounts {
		cl.subscriptions[m.Kind] = struct{}{}
	}

	mountRsp := messenger.MountRespMsg{
		ServiceID:   req.GetServiceID(),
		ServiceName: req.GetServiceName(),
	}

	data, err := proto.Marshal(&mountRsp)
	if err != nil {
		panic(err)
	}

	_, desc := descriptor.ForMessage(&mountRsp)
	message := messenger.Message{
		Type:      messenger.MessageType_MountResp,
		MessageId: 1,
		Objects: []*messenger.ObjectData{
			&messenger.ObjectData{
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

	_, err = cl.connection.Write(buffer.Bytes())
	if err != nil {
		panic(err)
	}
}

func (h *hub) sendNotify(cl *hubClient, msg *messenger.Message) {
	objects := make([]*messenger.ObjectData, 0)
	for _, o := range msg.GetObjects() {
		if _, ok := cl.subscriptions[o.Meta.Kind]; ok {
			objects = append(objects, o)
		}
	}
	message := messenger.Message{
		Type:      messenger.MessageType_Notify,
		MessageId: 1,
		Objects:   objects,
	}

	e := make([]byte, 0)
	buffer := proto.NewBuffer(e)

	buffer.EncodeMessage(&message)

	_, err := cl.connection.Write(buffer.Bytes())
	if err != nil {
		panic(err)
	}
}

func (h *hub) sendStatus(conn net.Conn) {
	message := messenger.Message{
		Type:      messenger.MessageType_StatusResp,
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
