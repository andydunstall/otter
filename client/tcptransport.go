package fuddle

import (
	"fmt"
	"net"
)

type TCPTransport struct{}

func (t *TCPTransport) Dial(addr string) (net.Conn, error) {
	conn, err := net.Dial("tcp", addr)
	if err != nil {
		return nil, fmt.Errorf("tcp connect: %s: %w", addr, err)
	}
	return conn, nil
}
