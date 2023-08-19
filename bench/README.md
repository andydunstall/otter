# Bench
Puddle benchmarks evaluate the performance of Puddle under difference
scenarios. Benchmarks run against the example `echo` server. Each benchmark
simply sends a number of requests to echo, waits for a response then repeats.

Using a simple `echo` server makes it easy to compare the performance of Puddle
against other libraries like [Helio](https://github.com/romange/helio),
[Seastar](https://github.com/scylladb/seastar) and [Tokio](https://tokio.rs/),
plus languages with built in asynchronous networking like Go.

## Building
The benchmarks are written in Go to easily scale across multiple cores. The benchmark tool can be build with:
```
$ go build -o bench main.go
```
Or run directly with:
```
$ go run main.go
```

## Running
Benchmark runs are configured with:
- `--conns`: Number of connections, where each connection sends sends
requests to the echo server (default 1000)
- `--requests`: Number of requests each connection should send (default 10,000)
- `--size`: Message size in bytes of each request and response (default 1024)
- `--addr`: The `echo` server address

See `bench -h` for the full list of configuration options.

### Puddle Echo Server
Run the echo server with
```
$ ./echo --port {port}
```

### Helio Echo Server
When running Helio echo server, set the read buffer size with `--size` and
`--raw` to use the requested size instead of expecting to read from the first
echo message.

Using `256` to match the Puddle echo server default size.

```
$ ./echo_server --raw --size 256 --port {port}
```
