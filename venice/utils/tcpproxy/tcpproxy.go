package tcpproxy

import (
	"io"
	"net"
	"sync"
	"time"

	"github.com/pensando/sw/venice/utils/log"
)

// Proxy implements TCPProxy interface
type Proxy struct {
	target      string
	listener    net.Listener
	dialTimeout time.Duration
	sync.Mutex
}

// New returns a TCP Proxy implementation
func New() *Proxy {
	return &Proxy{}
}

// Target sets the new target address
func (p *Proxy) Target(addr string) {
	p.Lock()
	p.target = addr
	p.Unlock()
}

// Run listens on local address and serves requests by connecting to remote address
func (p *Proxy) Run(laddr string) error {
	var err error
	p.listener, err = net.Listen("tcp", laddr)
	if err != nil {
		log.Infof("Listen to %s failed with %s", laddr, err)
		return err
	}
	go p.serve()
	return nil
}

func (p *Proxy) serve() {
	for {
		conn, err := p.listener.Accept()
		if err != nil {
			return
		}
		go p.handleConn(conn)
	}
}

func (p *Proxy) handleConn(in net.Conn) {
	p.Lock()
	dialTimeout := p.dialTimeout
	target := p.target
	p.Unlock()

	out, err := net.DialTimeout("tcp", target, dialTimeout)
	if err != nil {
		log.Infof("Dial to %s failed with %s", target, err)
		return
	}

	go func() {
		io.Copy(in, out)
		in.Close()
		out.Close()
	}()

	io.Copy(out, in)
	out.Close()
	in.Close()
}

// Stop the proxy
func (p *Proxy) Stop() {
	p.listener.Close()
	p.listener = nil
}

// Addr is the local address on which the proxy is listening
func (p *Proxy) Addr() net.Addr {
	return p.listener.Addr()
}

// DialTimeout is a custom dial timeout for future connections
func (p *Proxy) DialTimeout(timeout time.Duration) {
	p.Lock()
	p.dialTimeout = timeout
	p.Unlock()
}
