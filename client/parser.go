package fuddle

import (
	"encoding/binary"
)

type Parser struct {
	buf    []byte
	offset int
}

func NewParser(buf []byte) *Parser {
	return &Parser{
		buf:    buf,
		offset: 0,
	}
}

func (p *Parser) ReadUint16() (uint16, bool) {
	if len(p.buf) < p.offset+Uint16Size {
		return 0, false
	}
	n := binary.BigEndian.Uint16(p.buf[p.offset:])
	p.offset += Uint16Size
	return n, true
}

func (p *Parser) ReadUint32() (uint32, bool) {
	if len(p.buf) < p.offset+Uint32Size {
		return 0, false
	}
	n := binary.BigEndian.Uint32(p.buf[p.offset:])
	p.offset += Uint32Size
	return n, true
}

func (p *Parser) ReadUint64() (uint64, bool) {
	if len(p.buf) < p.offset+Uint64Size {
		return 0, false
	}
	n := binary.BigEndian.Uint64(p.buf[p.offset:])
	p.offset += Uint64Size
	return n, true
}

func (p *Parser) ReadMessageType() (MessageType, bool) {
	n, ok := p.ReadUint16()
	if !ok {
		return 0, false
	}
	return MessageType(n), true
}

func (p *Parser) ReadHeader() (Header, bool) {
	messageType, ok := p.ReadMessageType()
	if !ok {
		return Header{}, false
	}
	protocolVersion, ok := p.ReadUint16()
	if !ok {
		return Header{}, false
	}
	payloadSize, ok := p.ReadUint32()
	if !ok {
		return Header{}, false
	}
	return Header{
		MessageType: messageType,
		Version:     protocolVersion,
		PayloadSize: payloadSize,
	}, false
}

func (p *Parser) ReadRawBytes(size int) ([]byte, bool) {
	if len(p.buf) < p.offset+size {
		return nil, false
	}

	b := p.buf[p.offset : p.offset+size]
	p.offset += size
	return b, true
}

func (p *Parser) ReadString() (string, bool) {
	stringSize, ok := p.ReadUint32()
	if !ok {
		return "", false
	}
	if len(p.buf) < p.offset+int(stringSize) {
		return "", false
	}

	s := string(p.buf[p.offset : p.offset+int(stringSize)])
	p.offset += int(stringSize)
	return s, true
}
