package fuddle

import (
	"fmt"
	"io"
	"net"
	"time"
)

var (
	ErrNotFound = fmt.Errorf("not found")
)

type Fuddle struct {
	conn net.Conn
	buf  []byte
}

func Connect(addr string, opts ...Option) (*Fuddle, error) {
	options := defaultOptions()
	for _, opt := range opts {
		opt.apply(options)
	}

	conn, err := options.transport.Dial(addr)
	if err != nil {
		return nil, fmt.Errorf("dial: %w", err)
	}
	return &Fuddle{
		conn: conn,
		buf:  make([]byte, 1024),
	}, nil
}

func (c *Fuddle) Get(key string) (string, error) {
	header := Header{
		MessageType: MessageTypeGet,
		Version:     ProtocolVersion,
		PayloadSize: uint32(Uint32Size + len(key)),
	}

	builder := NewRequestBuilder(c.conn)
	builder.WriteHeader(header)
	builder.WriteString(key)
	if err := builder.Flush(); err != nil {
		return "", fmt.Errorf("get: %w", err)
	}

	b := c.buf[:HeaderSize]
	if _, err := io.ReadFull(c.conn, b); err != nil {
		return "", fmt.Errorf("get: read: %w", err)
	}

	parser := NewParser(b)
	header, _ = parser.ReadHeader()

	if header.MessageType != MessageTypeData {
		return "", fmt.Errorf("get: unexpected response type: %d", header.MessageType)
	}
	if int(header.PayloadSize) > len(c.buf) {
		return "", fmt.Errorf("get: payload too large: %d", header.PayloadSize)
	}

	b = c.buf[:header.PayloadSize]
	if _, err := io.ReadFull(c.conn, b); err != nil {
		return "", fmt.Errorf("get: read: %w", err)
	}
	parser = NewParser(b)
	code, ok := parser.ReadUint16()
	if !ok {
		// We've read the whole payload so should have sufficient data.
		return "", fmt.Errorf("get: failed to read response")
	}
	if code != StatusCodeOK {
		if code == StatusCodeNotFound {
			return "", ErrNotFound
		}
		return "", fmt.Errorf("get: failed to get value: status: %d", code)
	}

	s, ok := parser.ReadString()
	if !ok {
		// We've read the whole payload so should have sufficient data.
		return "", fmt.Errorf("get: failed to read response")
	}
	return s, nil
}

func (c *Fuddle) Put(key string, value string) error {
	header := Header{
		MessageType: MessageTypePut,
		Version:     ProtocolVersion,
		PayloadSize: uint32(Uint32Size + len(key) + Uint32Size + len(value)),
	}

	builder := NewRequestBuilder(c.conn)
	builder.WriteHeader(header)
	builder.WriteString(key)
	builder.WriteString(value)
	if err := builder.Flush(); err != nil {
		return fmt.Errorf("put: %w", err)
	}

	b := c.buf[:HeaderSize]
	if _, err := io.ReadFull(c.conn, b); err != nil {
		return fmt.Errorf("put: read: %w", err)
	}

	parser := NewParser(b)
	header, _ = parser.ReadHeader()

	if header.MessageType != MessageTypeAck {
		return fmt.Errorf("put: unexpected response type: %d", header.MessageType)
	}
	if int(header.PayloadSize) > len(c.buf) {
		return fmt.Errorf("get: payload too large: %d", header.PayloadSize)
	}

	b = c.buf[:header.PayloadSize]
	if _, err := io.ReadFull(c.conn, b); err != nil {
		return fmt.Errorf("get: read: %w", err)
	}
	parser = NewParser(b)
	code, ok := parser.ReadUint16()
	if !ok {
		// We've read the whole payload so should have sufficient data.
		return fmt.Errorf("get: failed to read response")
	}
	if code != StatusCodeOK {
		return fmt.Errorf("get: failed to get value: status: %d", code)
	}
	return nil
}

func (c *Fuddle) Delete(key string) error {
	header := Header{
		MessageType: MessageTypeDelete,
		Version:     ProtocolVersion,
		PayloadSize: uint32(Uint32Size + len(key)),
	}

	builder := NewRequestBuilder(c.conn)
	builder.WriteHeader(header)
	builder.WriteString(key)
	if err := builder.Flush(); err != nil {
		return fmt.Errorf("put: %w", err)
	}

	b := c.buf[:HeaderSize]
	if _, err := io.ReadFull(c.conn, b); err != nil {
		return fmt.Errorf("put: read: %w", err)
	}

	parser := NewParser(b)
	header, _ = parser.ReadHeader()

	if header.MessageType != MessageTypeAck {
		return fmt.Errorf("put: unexpected response type: %d", header.MessageType)
	}
	if int(header.PayloadSize) > len(c.buf) {
		return fmt.Errorf("get: payload too large: %d", header.PayloadSize)
	}

	b = c.buf[:header.PayloadSize]
	if _, err := io.ReadFull(c.conn, b); err != nil {
		return fmt.Errorf("get: read: %w", err)
	}
	parser = NewParser(b)
	code, ok := parser.ReadUint16()
	if !ok {
		// We've read the whole payload so should have sufficient data.
		return fmt.Errorf("get: failed to read response")
	}
	if code != StatusCodeOK {
		return fmt.Errorf("get: failed to get value: status: %d", code)
	}
	return nil
}

func (c *Fuddle) Ping() (int64, error) {
	header := Header{
		MessageType: MessageTypeEcho,
		Version:     ProtocolVersion,
		PayloadSize: Uint64Size,
	}

	builder := NewRequestBuilder(c.conn)
	builder.WriteHeader(header)
	builder.WriteUint64(uint64(time.Now().UnixMilli()))
	if err := builder.Flush(); err != nil {
		return 0, fmt.Errorf("ping: %w", err)
	}

	b := c.buf[:HeaderSize]
	if _, err := io.ReadFull(c.conn, b); err != nil {
		return 0, fmt.Errorf("ping: read: %w", err)
	}

	parser := NewParser(b)
	header, _ = parser.ReadHeader()

	// TODO check version and message type
	// TODO check payload size fits in buf

	if int(header.PayloadSize) > len(c.buf) {
		return 0, fmt.Errorf("ping: payload too large: %d", header.PayloadSize)
	}

	b = c.buf[:header.PayloadSize]
	if _, err := io.ReadFull(c.conn, b); err != nil {
		return 0, fmt.Errorf("ping: read: %w", err)
	}
	parser = NewParser(b)
	n, ok := parser.ReadUint64()
	if !ok {
		// TODO
	}

	return time.Now().UnixMilli() - int64(n), nil
}

func (c *Fuddle) Echo(m []byte) error {
	header := Header{
		MessageType: MessageTypeEcho,
		Version:     ProtocolVersion,
		PayloadSize: uint32(len(m)),
	}

	builder := NewRequestBuilder(c.conn)
	builder.WriteHeader(header)
	builder.WriteRawBytes(m)
	if err := builder.Flush(); err != nil {
		return fmt.Errorf("echo: %w", err)
	}

	b := c.buf[:HeaderSize]
	if _, err := io.ReadFull(c.conn, b); err != nil {
		return fmt.Errorf("echo: read: %w", err)
	}

	parser := NewParser(b)
	header, _ = parser.ReadHeader()

	// TODO check version and message type
	// TODO check payload size fits in buf

	if int(header.PayloadSize) > len(c.buf) {
		return fmt.Errorf("echo: payload too large: %d", header.PayloadSize)
	}

	b = c.buf[:header.PayloadSize]
	if _, err := io.ReadFull(c.conn, b); err != nil {
		return fmt.Errorf("echo: read: %w", err)
	}
	parser = NewParser(b)
	b, ok := parser.ReadRawBytes(int(header.PayloadSize))
	if !ok {
		// TODO
	}

	if len(b) != len(m) {
		return fmt.Errorf("echo: request and response don't match")
	}
	for i := 0; i != len(m); i++ {
		if b[i] != m[i] {
			return fmt.Errorf("echo: request and response don't match")
		}
	}
	return nil
}
