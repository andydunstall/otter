package otter

import (
	"encoding/binary"
	"fmt"
	"io"
)

// TODO(andydunstall): Add unit tests
type writer struct {
	w io.Writer
	// buf contains the queued bytes to be sent.
	buf []byte
	// offset is the current offset in buf.
	offset int
}

func newWriter(w io.Writer, bufSize int) *writer {
	if bufSize < Uint64Size {
		// Require that we can fit the largest integer in the buffer.
		panic("write buffer too small")
	}
	return &writer{
		w:      w,
		buf:    make([]byte, bufSize),
		offset: 0,
	}
}

func (w *writer) WriteUint16(n uint16) error {
	if len(w.buf) < w.offset+Uint16Size {
		// Insufficient bytes so we must flush first.
		if err := w.Flush(); err != nil {
			return err
		}
	}
	binary.BigEndian.PutUint16(w.buf[w.offset:], n)
	w.offset += Uint16Size
	return nil
}

func (w *writer) WriteUint32(n uint32) error {
	if len(w.buf) < w.offset+Uint32Size {
		// Insufficient bytes so we must flush first.
		if err := w.Flush(); err != nil {
			return err
		}
	}
	binary.BigEndian.PutUint32(w.buf[w.offset:], n)
	w.offset += Uint32Size
	return nil
}

func (w *writer) WriteUint64(n uint64) error {
	if len(w.buf) < w.offset+Uint64Size {
		// Insufficient bytes so we must flush first.
		if err := w.Flush(); err != nil {
			return err
		}
	}
	binary.BigEndian.PutUint64(w.buf[w.offset:], n)
	w.offset += Uint64Size
	return nil
}

func (w *writer) WriteMessageType(t MessageType) error {
	return w.WriteUint16(uint16(t))
}

func (w *writer) WriteHeader(t MessageType) error {
	if err := w.WriteMessageType(t); err != nil {
		return err
	}
	return w.WriteUint16(ProtocolVersion)
}

func (w *writer) WriteString(s string) error {
	b := []byte(s)
	if err := w.WriteUint32(uint32(len(b))); err != nil {
		return err
	}

	// TODO(andydunstall): If s is larger than b.buf, instead flush the buffer
	// then writes directly to the socket to avoid the extra copy. Could use
	// writev to flush both in a single system call.

	if len(w.buf) < w.offset+len(s) {
		// Insufficient bytes so we must flush first.
		if err := w.Flush(); err != nil {
			return err
		}
	}

	written := 0
	for written < len(b) {
		n := copy(w.buf[w.offset:], b[written:])
		written += n
		w.offset += n

		// If we've filled w.buf, flush it and keep writing to w.buf.
		if w.offset == len(w.buf) {
			if err := w.Flush(); err != nil {
				return err
			}
		}
	}
	return nil
}

func (w *writer) Flush() error {
	if w.offset == 0 {
		return nil
	}

	_, err := w.w.Write(w.buf[:w.offset])
	if err != nil {
		return fmt.Errorf("writer: flush: %w", err)
	}
	w.offset = 0
	return nil
}
