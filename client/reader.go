package otter

import (
	"encoding/binary"
	"fmt"
	"io"
)

// TODO(andydunstall): Add unit tests
type reader struct {
	r   io.Reader
	buf []byte
	// startIdx points to the start of the pending data in the buffer.
	startIdx int
	// endIdx points to the end of the pending data in the buffer.
	endIdx int
}

func newReader(r io.Reader, bufSize int) *reader {
	if bufSize < Uint64Size {
		// Require that we can fit the largest integer in the buffer.
		panic("read buffer too small")
	}
	return &reader{
		r:        r,
		buf:      make([]byte, bufSize),
		startIdx: 0,
		endIdx:   0,
	}
}

func (r *reader) ReadUint16() (uint16, error) {
	if r.pendingSize() < Uint16Size {
		// If we don't have enough pending bytes, read enough to parse.
		if err := r.readAtLeast(Uint16Size - r.pendingSize()); err != nil {
			return 0, err
		}
	}

	// We now know we have at least Uint16Size bytes.
	n := binary.BigEndian.Uint16(r.buf[r.startIdx:])
	r.startIdx += Uint16Size
	return n, nil
}

func (r *reader) ReadUint32() (uint32, error) {
	if r.pendingSize() < Uint32Size {
		// If we don't have enough pending bytes, read enough to parse.
		if err := r.readAtLeast(Uint32Size - r.pendingSize()); err != nil {
			return 0, err
		}
	}

	// We now know we have at least Uint32Size bytes.
	n := binary.BigEndian.Uint32(r.buf[r.startIdx:])
	r.startIdx += Uint32Size
	return n, nil
}

func (r *reader) ReadUint64() (uint64, error) {
	if r.pendingSize() < Uint64Size {
		// If we don't have enough pending bytes, read enough to parse.
		if err := r.readAtLeast(Uint64Size - r.pendingSize()); err != nil {
			return 0, err
		}
	}

	// We now know we have at least Uint64Size bytes.
	n := binary.BigEndian.Uint64(r.buf[r.startIdx:])
	r.startIdx += Uint64Size
	return n, nil
}

func (r *reader) ReadMessageType() (MessageType, error) {
	n, err := r.ReadUint16()
	if err != nil {
		return 0, err
	}
	return MessageType(n), nil
}

func (r *reader) ReadHeader() (MessageType, error) {
	messageType, err := r.ReadMessageType()
	if err != nil {
		return 0, err
	}

	version, err := r.ReadUint16()
	if err != nil {
		return 0, err
	}
	if version != ProtocolVersion {
		return 0, fmt.Errorf("unsupported version: %d", version)
	}

	return messageType, nil
}

func (r *reader) ReadString() (string, error) {
	size, err := r.ReadUint32()
	if err != nil {
		return "", err
	}

	if r.pendingSize() < int(size) {
		// If we don't have enough pending bytes, read enough to parse.
		if err := r.readAtLeast(int(size) - r.pendingSize()); err != nil {
			return "", err
		}
	}

	s := string(r.buf[r.startIdx : r.startIdx+int(size)])
	r.startIdx += int(size)
	return s, nil
}

func (r *reader) pendingSize() int {
	return r.endIdx - r.startIdx
}

func (r *reader) readAtLeast(n int) error {
	// TODO(andydunstall) if buffer over half full, or if n exceeds the buffer
	// size, extend up to some limit (copy buffer.h)

	// If over half the bytes in r.buf are unused, shift the bytes to the start.
	if r.startIdx*2 > len(r.buf) {
		copy(r.buf, r.buf[r.startIdx:r.endIdx])
		r.endIdx -= r.startIdx
		r.startIdx = 0
	}

	n, err := io.ReadAtLeast(r.r, r.buf[r.endIdx:], n)
	if err != nil {
		return fmt.Errorf("reader: read: %w", err)
	}
	r.endIdx += n
	return nil
}
