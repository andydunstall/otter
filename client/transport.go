package otter

import (
	"io"
	"net"
)

type Connection interface {
	io.Reader
	io.Writer
	io.Closer
}

type Transport interface {
	Dial(addr string) (net.Conn, error)
}
