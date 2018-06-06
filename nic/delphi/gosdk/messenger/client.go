package messenger

import (
	"fmt"
	"net"

	"github.com/golang/protobuf/descriptor"
	"github.com/golang/protobuf/proto"

	"github.com/pensando/sw/nic/delphi/messanger/proto"
	"github.com/pensando/sw/nic/delphi/proto/delphi"
)

// ServerAddress is the hub address
const ServerAddress = "127.0.0.1"

// ServerPort is the hub port
const ServerPort = 7001

// Handler is the interface the messenger clients have to implement
type Handler interface {
	HandleMountResp(svcID uint16, status string,
		objlist []*delphi_messanger.ObjectData) error
	HandleNotify(objlist []*delphi_messanger.ObjectData) error
	HandleStatusResp() error
}

// Client in the interface of the messenger
type Client interface {
	Dial() error
	IsConnected() bool
	SendMountReq(svcName string, mounts []*delphi_messanger.MountData) error
	SendChangeReq(objlist []*delphi_messanger.ObjectData) error
	Close()
}

type client struct {
	connection net.Conn
	receiver   *receiver
	handler    Handler
}

// NewClient creates a new messenger instance
func NewClient(handler Handler) (Client, error) {
	client := &client{
		handler: handler,
	}

	return client, nil
}

func (c *client) Dial() error {
	conn, err := net.Dial("tcp", fmt.Sprintf("%s:%d", ServerAddress, ServerPort))
	if err != nil {
		return err
	}

	c.connection = conn
	c.receiver, err = newReceiver(conn, c)
	if err != nil {
		panic(err)
	}

	return nil
}

func (c *client) IsConnected() bool {
	return true
}

func (c *client) SendMountReq(serviceName string, mounts []*delphi_messanger.MountData) error {

	mountRequest := delphi_messanger.MountReqMsg{
		ServiceName: serviceName,
		ServiceID:   1,
		Mounts:      mounts,
	}

	data, err := proto.Marshal(&mountRequest)
	if err != nil {
		panic(err)
	}

	_, desc := descriptor.ForMessage(&mountRequest)
	objects := []*delphi_messanger.ObjectData{
		&delphi_messanger.ObjectData{
			Meta: &delphi.ObjectMeta{
				Kind: *desc.Name,
			},
			Data: data,
		},
	}

	message := delphi_messanger.Message{
		Type:      delphi_messanger.MessageType_MountReq,
		MessageId: 1,
		Objects:   objects,
	}

	e := make([]byte, 0)
	buffer := proto.NewBuffer(e)

	buffer.EncodeMessage(&message)

	_, err = c.connection.Write(buffer.Bytes())
	if err != nil {
		panic(err)
	}

	return nil
}

func (c *client) SendChangeReq(objlist []*delphi_messanger.ObjectData) error {

	message := delphi_messanger.Message{
		Type:      delphi_messanger.MessageType_ChangeReq,
		MessageId: 1,
		Objects:   objlist,
	}

	e := make([]byte, 0)
	buffer := proto.NewBuffer(e)

	buffer.EncodeMessage(&message)

	_, err := c.connection.Write(buffer.Bytes())
	if err != nil {
		panic(err)
	}

	return nil
}

func (c *client) Close() {
	c.connection.Close()
}

// HandleMessage is responsible for halding messages received from the receiver
func (c *client) HandleMessage(message *delphi_messanger.Message) error {
	switch message.GetType() {
	case delphi_messanger.MessageType_MountResp:
		objects := message.GetObjects()
		if len(objects) != 1 {
			panic(len(objects))
		}
		var mountResp delphi_messanger.MountRespMsg
		proto.Unmarshal(objects[0].GetData(), &mountResp)
		c.handler.HandleMountResp(uint16(mountResp.GetServiceID()),
			message.GetStatus(), mountResp.GetObjects())
	case delphi_messanger.MessageType_Notify:
		c.handler.HandleNotify(message.GetObjects())
	case delphi_messanger.MessageType_StatusResp:
		c.handler.HandleStatusResp()
	default:
		panic(message.GetType())
	}
	return nil
}

// Implementing the transportHandler interface for the receiver
func (c *client) SocketClosed() {
}

func (c *client) loopForever() {
}
