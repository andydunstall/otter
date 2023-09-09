package tests

import (
	"testing"

	otter "github.com/otter-io/otter/client"
	"github.com/stretchr/testify/require"
)

func TestProtocol_Ping(t *testing.T) {
	conn, err := otter.Connect("localhost:8119")
	require.NoError(t, err)

	 err = conn.Ping()
	require.NoError(t, err)
}

// func TestProtocol_PingMany(t *testing.T) {
// 	conn, err := otter.Connect("localhost:8119")
// 	require.NoError(t, err)
// 
//   for i := 0; i != 100; i++ {
//      err = conn.Ping()
//     require.NoError(t, err)
//   }
// }

func TestProtocol_PingOneBytePerWrite(t *testing.T) {
	conn, err := otter.Connect(
		"localhost:8119",
		otter.WithTransport(&chunkedTransport{}),
	)
	require.NoError(t, err)

	err = conn.Ping()
	require.NoError(t, err)
}
