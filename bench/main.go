package main

import (
	"fmt"
	"math/rand"
	"sync"
	"time"

	"github.com/otter-io/otter/bench/pkg/conn"
	"github.com/otter-io/otter/bench/pkg/redis"
	otter "github.com/otter-io/otter/client"
	"github.com/spf13/cobra"
)

var conf = &Config{}

type Config struct {
	Addr        string
	Connections int
	Requests    int
	Size        int
	Repeat      int
	Protocol    string
}

type benchResults struct {
	Ping   time.Duration
	Put    time.Duration
	Get    time.Duration
	Delete time.Duration
}

var command = &cobra.Command{
	Use:          "bench (flags)",
	SilenceUsage: true,
	CompletionOptions: cobra.CompletionOptions{
		DisableDefaultCmd: true,
	},
	Run: run,
}

func run(_ *cobra.Command, _ []string) {
	for i := 0; i != conf.Repeat; i++ {
		bench(i + 1)
	}
}

func bench(run int) {
	var connResults []benchResults

	var wg sync.WaitGroup
	for i := 0; i != conf.Connections; i++ {
		i := i
		wg.Add(1)
		go func() {
			defer wg.Done()
			connResults = append(connResults, benchConn(run, i))
		}()
	}
	wg.Wait()

	var benchResults benchResults
	for _, r := range connResults {
		benchResults.Ping += r.Ping
		benchResults.Put += r.Put
		benchResults.Get += r.Get
		benchResults.Delete += r.Delete
	}
	benchResults.Ping = benchResults.Ping / time.Duration(len(connResults))
	benchResults.Put = benchResults.Put / time.Duration(len(connResults))
	benchResults.Get = benchResults.Get / time.Duration(len(connResults))
	benchResults.Delete = benchResults.Delete / time.Duration(len(connResults))

	fmt.Println("run:", run, "ping:", benchResults.Ping)
	fmt.Println("run:", run, "put:", benchResults.Put)
	fmt.Println("run:", run, "get:", benchResults.Get)
	fmt.Println("run:", run, "delete:", benchResults.Delete)
}

func benchConn(run int, conn int) benchResults {
	c, err := connect()
	if err != nil {
		panic(err)
	}

	keys := []string{}
	for i := 0; i != conf.Requests; i++ {
		keys = append(keys, fmt.Sprintf("key:%d:%d:%d", run, conn, i))
	}
	value := randomString(conf.Size)

	connResults := benchResults{}

	s := time.Now()
	for range keys {
		if err := c.Ping(); err != nil {
			panic(err)
		}
	}
	fmt.Println("run:", run, "conn:", conn, "ping:", time.Since(s))
	connResults.Ping = time.Since(s)

	s = time.Now()
	for _, key := range keys {
		if err := c.Put(key, value); err != nil {
			panic(err)
		}
	}
	fmt.Println("run:", run, "conn:", conn, "put:", time.Since(s))
	connResults.Put = time.Since(s)

	s = time.Now()
	for _, key := range keys {
		if _, err := c.Get(key); err != nil {
			panic(err)
		}
	}
	fmt.Println("run:", run, "conn:", conn, "get:", time.Since(s))
	connResults.Get = time.Since(s)

	s = time.Now()
	for _, key := range keys {
		if err := c.Delete(key); err != nil {
			panic(err)
		}
	}
	fmt.Println("run:", run, "conn:", conn, "delete:", time.Since(s))
	connResults.Delete = time.Since(s)

	return connResults
}

func connect() (conn.Conn, error) {
	if conf.Protocol == "otter" {
		if conf.Addr == "" {
			conf.Addr = "localhost:8119"
		}
		return otter.Connect(conf.Addr)
	} else if conf.Protocol == "redis" {
		if conf.Addr == "" {
			conf.Addr = "localhost:6379"
		}
		return redis.Connect(conf.Addr)
	}
	return nil, fmt.Errorf("unknown protocol: %w", conf.Protocol)
}

func init() {
	cobra.EnableCommandSorting = false

	command.Flags().StringVarP(
		&conf.Addr,
		"addr", "",
		"",
		"server address",
	)
	command.Flags().IntVarP(
		&conf.Connections,
		"conns", "",
		10,
		"number of connections",
	)
	command.Flags().IntVarP(
		&conf.Requests,
		"requests", "",
		10000,
		"number of requests per connection",
	)
	command.Flags().IntVarP(
		&conf.Size,
		"size", "",
		1024,
		"request/response message size",
	)
	command.Flags().IntVarP(
		&conf.Repeat,
		"repeat", "",
		1,
		"number of times to run the benchmark",
	)
	command.Flags().StringVarP(
		&conf.Protocol,
		"protocol", "",
		"otter",
		"server protocol (redis or otter)",
	)
}

func main() {
	command.Execute()
}

var letterRunes = []rune("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ")

func randomString(n int) string {
	b := make([]rune, n)
	for i := range b {
		b[i] = letterRunes[rand.Intn(len(letterRunes))]
	}
	return string(b)
}
