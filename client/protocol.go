package otter

type MessageType uint16

const (
	MessageTypePing   MessageType = 1
	MessageTypePong   MessageType = 2
	MessageTypeGet    MessageType = 3
	MessageTypePut    MessageType = 4
	MessageTypeDelete MessageType = 5
	MessageTypeAck    MessageType = 6
	MessageTypeData   MessageType = 7
)

const (
	ProtocolVersion uint16 = 1

	Uint16Size = 2
	Uint32Size = 4
	Uint64Size = 8

	// Status codes from https://github.com/abseil/abseil-cpp/blob/master/absl/status/status.h.
	StatusCodeOK       = 0
	StatusCodeNotFound = 5
)
