package tests

import (
	"testing"

	fuddle "github.com/fuddle-io/fuddle/client"
	"github.com/google/uuid"
	"github.com/stretchr/testify/assert"
	"github.com/stretchr/testify/require"
)

func TestDB_PutThenGet(t *testing.T) {
	conn, err := fuddle.Connect("localhost:8119")
	require.NoError(t, err)

	key := uuid.New().String()
	value := uuid.New().String()
	err = conn.Put(key, value)
	assert.NoError(t, err)

	v, err := conn.Get(key)
	assert.NoError(t, err)
	assert.Equal(t, value, v)
}

func TestDB_DeleteThenGetNotFound(t *testing.T) {
	conn, err := fuddle.Connect("localhost:8119")
	require.NoError(t, err)

	key := uuid.New().String()
	err = conn.Put(key, uuid.New().String())
	assert.NoError(t, err)

	err = conn.Delete(key)
	assert.NoError(t, err)

	_, err = conn.Get(key)
	assert.Equal(t, fuddle.ErrNotFound, err)
}
