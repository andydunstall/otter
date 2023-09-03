package tests

import (
	"testing"

	otter "github.com/otter-io/otter/client"
	"github.com/stretchr/testify/require"
)

func TestProtocol_Ping(t *testing.T) {
	conn, err := otter.Connect("localhost:8119")
	require.NoError(t, err)

	_, err = conn.Ping()
	require.NoError(t, err)
}

func TestProtocol_PingOneBytePerWrite(t *testing.T) {
	conn, err := otter.Connect(
		"localhost:8119",
		otter.WithTransport(&chunkedTransport{}),
	)
	require.NoError(t, err)

	_, err = conn.Ping()
	require.NoError(t, err)
}

func TestProtocol_Echo(t *testing.T) {
	conn, err := otter.Connect("localhost:8119")
	require.NoError(t, err)

	err = conn.Echo([]byte{1, 2, 3, 4, 5})
	require.NoError(t, err)
}
