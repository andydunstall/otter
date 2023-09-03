package redis

import (
	"context"
	"fmt"

	"github.com/redis/go-redis/v9"
)

type Conn struct {
	rdb *redis.Client
}

func Connect(addr string) (*Conn, error) {
	if addr == "" {
		addr = "localhost:6379"
	}
	rdb := redis.NewClient(&redis.Options{
		Addr: addr,
	})
	return &Conn{
		rdb: rdb,
	}, nil
}

func (c *Conn) Get(key string) (string, error) {
	v, err := c.rdb.Get(context.Background(), key).Result()
	if err != nil {
		fmt.Println(key, err)
		return "", fmt.Errorf("redis: get: %w", err)
	}
	return v, nil
}

func (c *Conn) Put(key string, value string) error {
	if err := c.rdb.Set(context.Background(), key, value, 0).Err(); err != nil {
		return fmt.Errorf("redis: put: %w", err)
	}
	return nil
}

func (c *Conn) Delete(key string) error {
	if err := c.rdb.Del(context.Background(), key).Err(); err != nil {
		return fmt.Errorf("redis: del: %w", err)
	}
	return nil
}

func (c *Conn) Ping() (int64, error) {
	if err := c.rdb.Ping(context.Background()).Err(); err != nil {
		return 0, fmt.Errorf("redis: ping: %w", err)
	}
	return 0, nil
}
