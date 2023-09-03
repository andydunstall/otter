package tests

import (
	"fmt"
	"net"
	"time"
)

// chunkedTransport reads and writes 1 byte at a time, which can be used for
// protocol testing.
type chunkedTransport struct{}

func (t *chunkedTransport) Dial(addr string) (net.Conn, error) {
	conn, err := net.Dial("tcp", addr)
	if err != nil {
		return nil, fmt.Errorf("tcp connect: %s: %w", addr, err)
	}
	return &chunkedConn{
		conn: conn,
	}, nil
}

type chunkedConn struct {
	conn net.Conn
}

func (c *chunkedConn) Read(b []byte) (n int, err error) {
	chunk := make([]byte, 1)
	n, err = c.conn.Read(chunk)
	if err != nil {
		return n, err
	}
	b[0] = chunk[0]
	return 1, nil
}

func (c *chunkedConn) Write(b []byte) (n int, err error) {
	for i, chunk := range b {
		_, err := c.conn.Write([]byte{chunk})
		if err != nil {
			return i, err
		}
	}
	return len(b), nil
}

func (c *chunkedConn) Close() error {
	return c.conn.Close()
}

func (c *chunkedConn) LocalAddr() net.Addr {
	return c.conn.LocalAddr()
}

func (c *chunkedConn) RemoteAddr() net.Addr {
	return c.conn.RemoteAddr()
}

func (c *chunkedConn) SetDeadline(t time.Time) error {
	return c.conn.SetDeadline(t)
}

func (c *chunkedConn) SetReadDeadline(t time.Time) error {
	return c.conn.SetReadDeadline(t)
}

func (c *chunkedConn) SetWriteDeadline(t time.Time) error {
	return c.conn.SetWriteDeadline(t)
}
