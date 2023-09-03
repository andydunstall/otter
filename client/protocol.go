package otter

type MessageType uint16

const (
	MessageTypeEcho   MessageType = 1
	MessageTypeGet    MessageType = 2
	MessageTypePut    MessageType = 3
	MessageTypeDelete MessageType = 4
	MessageTypeAck    MessageType = 5
	MessageTypeData   MessageType = 6
)

const (
	ProtocolVersion uint16 = 1

	HeaderSize = 8

	Uint16Size = 2
	Uint32Size = 4
	Uint64Size = 8

	// Status codes from https://github.com/abseil/abseil-cpp/blob/master/absl/status/status.h.
	StatusCodeOK       = 0
	StatusCodeNotFound = 5
)

type Header struct {
	MessageType MessageType
	Version     uint16
	PayloadSize uint32
}
