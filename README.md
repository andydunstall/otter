# Puddle

Puddle is a C++ thread-per-core library for asynchronous IO, based on
`io_uring`.

Note Puddle is only a toy project to experiment with `io_uring`.

## Build

> :warning: Puddle only supports Linux.

### Prerequisites

#### Bazel

Puddle is built using [Bazel](https://bazel.build/).

The easiest option for installing Bazel is using [bazelisk](https://github.com/bazelbuild/bazelisk),
which automatically picks and installs the correct Bazel version.

#### Boost

Puddle depends on [Boost](https://www.boost.org/), specifically
[Boost Context](https://www.boost.org/doc/libs/1_87_0/libs/context/doc/html/index.html).

The easiest option to install Boost is with:
```
$ sudo apt-get install libboost-all-dev
```

#### liburing

Puddle depends on [liburing](https://github.com/axboe/liburing/tree/master).

The easiest option to install `liburing` is with:
```
$ sudo apt-get install liburing-dev
```

### Build

Build all Puddle libraries and examples with:
```
$ bazel build //...
```
