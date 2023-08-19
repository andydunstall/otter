package conn

import (
	"fmt"
	"io"
	"net"
)

type Conn struct {
	conn net.Conn
}

func Connect(addr string) (*Conn, error) {
	conn, err := net.Dial("tcp", addr)
	if err != nil {
		return nil, fmt.Errorf("connect: %w", err)
	}
	return &Conn{
		conn: conn,
	}, nil
}

func (c *Conn) Bench(requests int, message []byte) error {
	for i := 0; i != requests; i++ {
		if _, err := c.conn.Write(message); err != nil {
			return fmt.Errorf("write: %w", err)
		}
		if _, err := io.ReadFull(c.conn, message); err != nil {
			return fmt.Errorf("read: %w", err)
		}
	}
	return nil
}
