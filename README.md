# Puddle

Puddle is a runtime for lightweight user-space threads (tasks) in C++. It
includes a library for I/O based on `io_uring`.

## Build

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

## Examples

See [`examples`](./examples).

## :warning: Limitations

Puddle is only a toy project to learn about and experiment with `io_uring`, so
isn't built for production. It only supports Linux.

Puddle doesn't yet support message passing or communication between OS threads.
Instead the 'runtime' only supports a single OS thread (though you can run
independent runtimes on different threads).
