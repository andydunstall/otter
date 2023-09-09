package otter

import (
	"fmt"
	"net"
)

const (
	defaultWriterBufSize = 1 << 14 // 16 KiB
	defaultReaderBufSize = 1 << 14 // 16 KiB
)

var (
	ErrNotFound = fmt.Errorf("not found")
)

type Otter struct {
	conn   net.Conn
	buf    []byte
	writer *writer
	reader *reader
}

func Connect(addr string, opts ...Option) (*Otter, error) {
	options := defaultOptions()
	for _, opt := range opts {
		opt.apply(options)
	}

	conn, err := options.transport.Dial(addr)
	if err != nil {
		return nil, fmt.Errorf("dial: %w", err)
	}
	return &Otter{
		conn:   conn,
		buf:    make([]byte, 2048),
		writer: newWriter(conn, defaultWriterBufSize),
		reader: newReader(conn, defaultReaderBufSize),
	}, nil
}

func (c *Otter) Get(key string) (string, error) {
	if err := c.writer.WriteHeader(MessageTypeGet); err != nil {
		return "", fmt.Errorf("get: %w", err)
	}
	if err := c.writer.WriteString(key); err != nil {
		return "", fmt.Errorf("get: %w", err)
	}
	if err := c.writer.Flush(); err != nil {
		return "", fmt.Errorf("get: %w", err)
	}

	messageType, err := c.reader.ReadHeader()
	if err != nil {
		return "", fmt.Errorf("get: %w", err)
	}
	if messageType != MessageTypeData {
		return "", fmt.Errorf("get: unexpected response type: %d", messageType)
	}

	code, err := c.reader.ReadUint16()
	if err != nil {
		return "", fmt.Errorf("get: %w", err)
	}
	if code != StatusCodeOK {
		if code == StatusCodeNotFound {
			return "", ErrNotFound
		}
		return "", fmt.Errorf("get: failed to get value: status: %d", code)
	}

	v, err := c.reader.ReadString()
	if err != nil {
		return "", fmt.Errorf("get: %w", err)
	}
	return v, nil
}

func (c *Otter) Put(key string, value string) error {
	if err := c.writer.WriteHeader(MessageTypePut); err != nil {
		return fmt.Errorf("put: %w", err)
	}
	if err := c.writer.WriteString(key); err != nil {
		return fmt.Errorf("put: %w", err)
	}
	if err := c.writer.WriteString(value); err != nil {
		return fmt.Errorf("put: %w", err)
	}
	if err := c.writer.Flush(); err != nil {
		return fmt.Errorf("put: %w", err)
	}

	messageType, err := c.reader.ReadHeader()
	if err != nil {
		return fmt.Errorf("put: %w", err)
	}
	if messageType != MessageTypeAck {
		return fmt.Errorf("put: unexpected response type: %d", messageType)
	}

	code, err := c.reader.ReadUint16()
	if err != nil {
		return fmt.Errorf("put: %w", err)
	}
	if code != StatusCodeOK {
		return fmt.Errorf("put: failed to put value: status: %d", code)
	}
	return nil
}

func (c *Otter) Delete(key string) error {
	if err := c.writer.WriteHeader(MessageTypeDelete); err != nil {
		return fmt.Errorf("delete: %w", err)
	}
	if err := c.writer.WriteString(key); err != nil {
		return fmt.Errorf("delete: %w", err)
	}
	if err := c.writer.Flush(); err != nil {
		return fmt.Errorf("delete: %w", err)
	}

	messageType, err := c.reader.ReadHeader()
	if err != nil {
		return fmt.Errorf("delete: %w", err)
	}
	if messageType != MessageTypeAck {
		return fmt.Errorf("delete: unexpected response type: %d", messageType)
	}

	code, err := c.reader.ReadUint16()
	if err != nil {
		return fmt.Errorf("delete: %w", err)
	}
	if code != StatusCodeOK {
		return fmt.Errorf("delete: failed to delete value: status: %d", code)
	}
	return nil
}

func (c *Otter) Ping() error {
	if err := c.writer.WriteHeader(MessageTypePing); err != nil {
		return fmt.Errorf("ping: %w", err)
	}
	if err := c.writer.Flush(); err != nil {
		return fmt.Errorf("ping: %w", err)
	}

	messageType, err := c.reader.ReadHeader()
	if err != nil {
		return fmt.Errorf("ping: %w", err)
	}
	if messageType != MessageTypePong {
		return fmt.Errorf("ping: unexpected response type: %d", messageType)
	}
	return nil
}
