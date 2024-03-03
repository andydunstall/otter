# Puddle

> Note Puddle is just a toy project to play with Boost fibers and `io_uring`.

Puddle is a minimal asynchronous C++ networking library built with
[Boost fibers](https://publish.obsidian.md/andydunstall/public/Boost+Fibers)
and `io_uring`.

Puddle is based on the [helio](https://github.com/romange/helio), though
Puddle only supports a single OS thread and is much more basic.

## Usage
Puddle provides asynchronous networking on a single OS thread using Boost
fibers.

To create a TCP server with Puddle, you must implement `puddle::Listener` which
accepts connections. The `Listener::Connection(std::unique_ptr<puddle::Socket> conn)`
method is called when a new connection is accepted.

Each connection is run in its own fiber and can use the passed `puddle::Socket`
to read and write to the socket. When reads and writes block, the fiber will
be descheduled so another fiber can run on the same thread.

See [`examples/echo`](examples/echo/main.cc) for an example.

## Scheduling
Each connection has its own fiber. When the connection 'blocks' by making
a socket request, the request will be added to the `io_uring` submission queue
and the fiber will be descheduled so another fiber can run.

Puddle implements a custom fiber scheduling algorithm which used round-robin
scheduling, similar to the default algorithm. Though when there are no fibers
ready to run, it will submit the queued requests to `io_uring` and wait for
a completion event. For each completion event, it will mark the fiber blocked
on that event as ready to run so the fiber will be scheduled.

## Building
Puddle is built with [Bazel](https://bazel.build/).

Build the library with `bazel build //puddle:puddle` which will build both
`bazel-bin/puddle/libpuddle.a` and `bazel-bin/puddle/libpuddle.so`.

You can also run the echo server example with `bazel run //examples/echo`.
