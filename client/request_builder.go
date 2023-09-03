package otter

import (
	"encoding/binary"
	"fmt"
	"net"
)

type RequestBuilder struct {
	buf  []byte
	conn net.Conn
}

func NewRequestBuilder(conn net.Conn) *RequestBuilder {
	return &RequestBuilder{
		conn: conn,
	}
}

func (b *RequestBuilder) WriteUint16(n uint16) {
	enc := make([]byte, Uint16Size)
	binary.BigEndian.PutUint16(enc, n)
	b.buf = append(b.buf, enc...)
}

func (b *RequestBuilder) WriteUint32(n uint32) {
	enc := make([]byte, Uint32Size)
	binary.BigEndian.PutUint32(enc, n)
	b.buf = append(b.buf, enc...)
}

func (b *RequestBuilder) WriteUint64(n uint64) {
	enc := make([]byte, Uint64Size)
	binary.BigEndian.PutUint64(enc, n)
	b.buf = append(b.buf, enc...)
}

func (b *RequestBuilder) WriteMessageType(t MessageType) {
	b.WriteUint16(uint16(t))
}

func (b *RequestBuilder) WriteHeader(header Header) {
	b.WriteMessageType(header.MessageType)
	b.WriteUint16(header.Version)
	b.WriteUint32(header.PayloadSize)
}

func (b *RequestBuilder) WriteString(s string) {
	b.WriteUint32(uint32(len(s)))
	b.buf = append(b.buf, []byte(s)...)
}

func (b *RequestBuilder) WriteRawBytes(m []byte) {
	b.buf = append(b.buf, m...)
}

func (b *RequestBuilder) Flush() error {
	if _, err := b.conn.Write(b.buf); err != nil {
		return fmt.Errorf("request builder: flush: %w", err)
	}
	b.buf = nil
	return nil
}
