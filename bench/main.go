package main

import (
	"fmt"
	"sync"
	"time"

	"github.com/andydunstall/puddle/bench/pkg/conn"
	"github.com/spf13/cobra"
)

var flags = &Flags{}

type Flags struct {
	Addr        string
	Connections int
	Requests    int
	Size        int
	Repeat      int
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
	for i := 0; i != flags.Repeat; i++ {
		bench(i + 1)
	}
}

func bench(run int) {
	message := make([]byte, flags.Size)

	s := time.Now()

	var wg sync.WaitGroup
	for i := 0; i != flags.Connections; i++ {
		wg.Add(1)
		go func() {
			defer wg.Done()

			conn, err := conn.Connect(flags.Addr)
			if err != nil {
				panic("connect: " + err.Error())
			}
			if err := conn.Bench(flags.Requests, message); err != nil {
				panic("bench: " + err.Error())
			}
		}()
	}
	wg.Wait()

	fmt.Println("run", run, "=", time.Since(s))
}

func init() {
	cobra.EnableCommandSorting = false

	command.Flags().StringVarP(
		&flags.Addr,
		"addr", "",
		"localhost:8119",
		"server address",
	)
	command.Flags().IntVarP(
		&flags.Connections,
		"conns", "",
		1000,
		"number of connections",
	)
	command.Flags().IntVarP(
		&flags.Requests,
		"requests", "",
		10000,
		"number of requests per connection",
	)
	command.Flags().IntVarP(
		&flags.Size,
		"size", "",
		1024,
		"request/response message size",
	)
	command.Flags().IntVarP(
		&flags.Repeat,
		"repeat", "",
		1,
		"number of times to run the benchmark",
	)
}

func main() {
	command.Execute()
}
