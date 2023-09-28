module github.com/otter-io/otter/bench

go 1.20

require (
	github.com/HdrHistogram/hdrhistogram-go v1.1.2
	github.com/otter-io/otter/client v0.0.0
	github.com/redis/go-redis/v9 v9.1.0
	github.com/spf13/cobra v1.7.0
	go.uber.org/atomic v1.11.0
)

require (
	github.com/cespare/xxhash/v2 v2.2.0 // indirect
	github.com/dgryski/go-rendezvous v0.0.0-20200823014737-9f7001d12a5f // indirect
	github.com/inconshreveable/mousetrap v1.1.0 // indirect
	github.com/spf13/pflag v1.0.5 // indirect
)

replace github.com/otter-io/otter/client v0.0.0 => ../client
