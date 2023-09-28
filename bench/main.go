package main

import (
	"fmt"
	"log"
	"math/rand"
	"os"
	"runtime/pprof"
	"sync"
	"time"

	hdrhistogram "github.com/HdrHistogram/hdrhistogram-go"
	"github.com/otter-io/otter/bench/pkg/conn"
	"github.com/otter-io/otter/bench/pkg/redis"
	otter "github.com/otter-io/otter/client"
	"github.com/spf13/cobra"
	"go.uber.org/atomic"
)

var conf = &config{}

var (
	latencies     *hdrhistogram.Histogram
	totalRequests atomic.Uint64
)

type config struct {
	Addr       string
	Command    string
	Clients    int
	Protocol   string
	Requests   int
	CPUProfile string
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
	if conf.CPUProfile != "" {
		f, err := os.Create(conf.CPUProfile)
		if err != nil {
			log.Fatalf("failed to open cpu profile: %w", err)
		}
		pprof.StartCPUProfile(f)
		defer pprof.StopCPUProfile()
	}

	if conf.Addr == "" {
		if conf.Protocol == "otter" {
			conf.Addr = "localhost:8119"
		} else if conf.Protocol == "redis" {
			conf.Addr = "localhost:6379"
		} else {
			log.Fatalf("unknown protocol: %s", conf.Protocol)
		}
	}

	fmt.Printf(`starting benchmark:
  addr: %s
  command: %s
  clients: %d
  protocol: %s
  requests: %d
  cpu profile: %s

`, conf.Addr, conf.Command, conf.Clients, conf.Protocol, conf.Requests, conf.CPUProfile)

	requestsPerClient := conf.Requests / conf.Clients

	latencies = hdrhistogram.New(1, 90000000, 3)

	var wg sync.WaitGroup
	for clientID := 0; clientID != conf.Clients; clientID++ {
		wg.Add(1)
		go func() {
			defer wg.Done()

			if conf.Command == "ping" {
				benchClient(requestsPerClient, ping)
			} else if conf.Command == "put" {
				benchClient(requestsPerClient, put)
			} else if conf.Command == "put-then-get" {
				benchClient(requestsPerClient, putThenGet)
			} else {
				log.Fatalf("unknown command: %s", conf.Command)
			}
		}()
	}

	monitor()

	wg.Wait()
}

func benchClient(requests int, command func(c conn.Conn)) {
	c, err := connect()
	if err != nil {
		log.Fatalf("failed to connect to server: %w", err)
	}

	for i := 0; i != requests; i++ {
		start := time.Now()
		command(c)
		duration := time.Since(start)

		if err := latencies.RecordValue(duration.Microseconds()); err != nil {
			log.Fatalf("failed to record latency: %w", err)
		}

		totalRequests.Add(1)
	}
}

func connect() (conn.Conn, error) {
	if conf.Protocol == "otter" {
		return otter.Connect(conf.Addr)
	} else if conf.Protocol == "redis" {
		return redis.Connect(conf.Addr)
	}
	return nil, fmt.Errorf("unknown protocol: %w", conf.Protocol)
}

func monitor() {
	ticker := time.NewTicker(time.Second)

	lastUpdate := time.Now()
	lastCompletedRequests := uint64(0)

	fmt.Printf(
		"%15s %15s %15s %20s %20s %20s\n",
		"Duration",
		"Completed (%)",
		"Message Rate",
		"p50 Latency (ms)",
		"p90 Latency (ms)",
		"p99 Latency (ms)",
	)

	start := time.Now()
	for {
		select {
		case <-ticker.C:
			interval := time.Since(lastUpdate)
			lastUpdate = time.Now()

			completedRequests := totalRequests.Load()
			completedPercent := float64(completedRequests) / float64(conf.Requests) * 100.0
			messageRate := float64(completedRequests-lastCompletedRequests) / float64(interval.Seconds())
			p50Latency := float64(latencies.ValueAtQuantile(50.0)) / 1000.0
			p90Latency := float64(latencies.ValueAtQuantile(90.0)) / 1000.0
			p99Latency := float64(latencies.ValueAtQuantile(99.0)) / 1000.0

			fmt.Printf(
				"%15.0f %15.0f %15.2f %20.2f %20.2f %20.2f\n",
				time.Since(start).Seconds(),
				completedPercent,
				messageRate,
				p50Latency,
				p90Latency,
				p99Latency,
			)

			lastCompletedRequests = completedRequests

			if completedRequests >= uint64(conf.Requests) {
				return
			}
		}
	}
}

func ping(c conn.Conn) {
	if err := c.Ping(); err != nil {
		log.Fatalf("failed to ping server: %w", err)
	}
}

func put(c conn.Conn) {
	key := fmt.Sprintf("key:%d", rand.Uint32())
	value := fmt.Sprintf("value:%d", rand.Uint32())

	if err := c.Put(key, value); err != nil {
		log.Fatalf("failed to put value: %w", err)
	}
}

func putThenGet(c conn.Conn) {
	key := fmt.Sprintf("key:%d", rand.Uint32())
	value := fmt.Sprintf("value:%d", rand.Uint32())

	if err := c.Put(key, value); err != nil {
		log.Fatalf("failed to put value: %w", err)
	}
	v, err := c.Get(key)
	if err != nil {
		log.Fatalf("failed to get value: %w", err)
	}
	if v != value {
		log.Fatalf("received unexpected value (%s != %s)", value, v)
	}
}

func init() {
	cobra.EnableCommandSorting = false

	command.Flags().StringVarP(
		&conf.Addr,
		"addr", "",
		"",
		"server address",
	)

	command.Flags().StringVarP(
		&conf.Command,
		"command", "",
		"ping",
		"command to run",
	)

	command.Flags().IntVarP(
		&conf.Clients,
		"clients", "",
		50,
		"number of clients",
	)

	command.Flags().StringVarP(
		&conf.Protocol,
		"protocol", "",
		"otter",
		"server protocol (redis or otter)",
	)

	command.Flags().IntVarP(
		&conf.Requests,
		"requests", "",
		10000000,
		"number of requests",
	)

	command.Flags().StringVarP(
		&conf.CPUProfile,
		"cpu-profile", "",
		"",
		"cpu profile path",
	)
}

func main() {
	command.Execute()
}

// import (
// 	"fmt"
// 	"math/rand"
// 	"sync"
// 	"time"
//
// 	"github.com/otter-io/otter/bench/pkg/conn"
// 	"github.com/otter-io/otter/bench/pkg/redis"
// 	otter "github.com/otter-io/otter/client"
// 	"github.com/spf13/cobra"
// )
//
// var conf = &Config{}
//
// type Config struct {
// 	Addr        string
// 	Connections int
// 	Requests    int
// 	Size        int
// 	Repeat      int
// 	Protocol    string
// }
//
// type benchResults struct {
// 	Ping   time.Duration
// 	Put    time.Duration
// 	Get    time.Duration
// 	Delete time.Duration
// }
//
// var command = &cobra.Command{
// 	Use:          "bench (flags)",
// 	SilenceUsage: true,
// 	CompletionOptions: cobra.CompletionOptions{
// 		DisableDefaultCmd: true,
// 	},
// 	Run: run,
// }
//
// func run(_ *cobra.Command, _ []string) {
// 	for i := 0; i != conf.Repeat; i++ {
// 		bench(i + 1)
// 	}
// }
//
// func bench(run int) {
// 	var connResults []benchResults
//
// 	var wg sync.WaitGroup
// 	for i := 0; i != conf.Connections; i++ {
// 		i := i
// 		wg.Add(1)
// 		go func() {
// 			defer wg.Done()
// 			connResults = append(connResults, benchConn(run, i))
// 		}()
// 	}
// 	wg.Wait()
//
// 	var benchResults benchResults
// 	for _, r := range connResults {
// 		benchResults.Ping += r.Ping
// 		benchResults.Put += r.Put
// 		benchResults.Get += r.Get
// 		benchResults.Delete += r.Delete
// 	}
// 	benchResults.Ping = benchResults.Ping / time.Duration(len(connResults))
// 	benchResults.Put = benchResults.Put / time.Duration(len(connResults))
// 	benchResults.Get = benchResults.Get / time.Duration(len(connResults))
// 	benchResults.Delete = benchResults.Delete / time.Duration(len(connResults))
//
// 	fmt.Println("run:", run, "ping:", benchResults.Ping)
// 	fmt.Println("run:", run, "put:", benchResults.Put)
// 	fmt.Println("run:", run, "get:", benchResults.Get)
// 	fmt.Println("run:", run, "delete:", benchResults.Delete)
// }
//
// func benchConn(run int, conn int) benchResults {
// 	c, err := connect()
// 	if err != nil {
// 		panic(err)
// 	}
//
// 	keys := []string{}
// 	for i := 0; i != conf.Requests; i++ {
// 		keys = append(keys, fmt.Sprintf("key:%d:%d:%d", run, conn, i))
// 	}
// 	value := randomString(conf.Size)
//
// 	connResults := benchResults{}
//
// 	s := time.Now()
// 	for range keys {
// 		if err := c.Ping(); err != nil {
// 			panic(err)
// 		}
// 	}
// 	fmt.Println("run:", run, "conn:", conn, "ping:", time.Since(s), "requests", conf.Requests)
// 	connResults.Ping = time.Since(s)
//
// 	s = time.Now()
// 	for _, key := range keys {
// 		if err := c.Put(key, value); err != nil {
// 			panic(err)
// 		}
// 	}
// 	fmt.Println("run:", run, "conn:", conn, "put:", time.Since(s), "requests", conf.Requests)
// 	connResults.Put = time.Since(s)
//
// 	s = time.Now()
// 	for _, key := range keys {
// 		if _, err := c.Get(key); err != nil {
// 			panic(err)
// 		}
// 	}
// 	fmt.Println("run:", run, "conn:", conn, "get:", time.Since(s), "requests", conf.Requests)
// 	connResults.Get = time.Since(s)
//
// 	s = time.Now()
// 	for _, key := range keys {
// 		if err := c.Delete(key); err != nil {
// 			panic(err)
// 		}
// 	}
// 	fmt.Println("run:", run, "conn:", conn, "delete:", time.Since(s), "requests", conf.Requests)
// 	connResults.Delete = time.Since(s)
//
// 	return connResults
// }
//
// func connect() (conn.Conn, error) {
// 	if conf.Protocol == "otter" {
// 		if conf.Addr == "" {
// 			conf.Addr = "localhost:8119"
// 		}
// 		return otter.Connect(conf.Addr)
// 	} else if conf.Protocol == "redis" {
// 		if conf.Addr == "" {
// 			conf.Addr = "localhost:6379"
// 		}
// 		return redis.Connect(conf.Addr)
// 	}
// 	return nil, fmt.Errorf("unknown protocol: %w", conf.Protocol)
// }
//
// func init() {
// 	cobra.EnableCommandSorting = false
//
// 	command.Flags().StringVarP(
// 		&conf.Addr,
// 		"addr", "",
// 		"",
// 		"server address",
// 	)
// 	command.Flags().IntVarP(
// 		&conf.Connections,
// 		"conns", "",
// 		10,
// 		"number of connections",
// 	)
// 	command.Flags().IntVarP(
// 		&conf.Requests,
// 		"requests", "",
// 		10000,
// 		"number of requests per connection",
// 	)
// 	command.Flags().IntVarP(
// 		&conf.Size,
// 		"size", "",
// 		1024,
// 		"request/response message size",
// 	)
// 	command.Flags().IntVarP(
// 		&conf.Repeat,
// 		"repeat", "",
// 		1,
// 		"number of times to run the benchmark",
// 	)
// 	command.Flags().StringVarP(
// 		&conf.Protocol,
// 		"protocol", "",
// 		"otter",
// 		"server protocol (redis or otter)",
// 	)
// }
//
// func main() {
// 	command.Execute()
// }
//
// var letterRunes = []rune("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ")
//
// func randomString(n int) string {
// 	b := make([]rune, n)
// 	for i := range b {
// 		b[i] = letterRunes[rand.Intn(len(letterRunes))]
// 	}
// 	return string(b)
// }
