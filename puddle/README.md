# Puddle

> :warning: **Puddle is still in development**

Puddle is a minimal asynchronous C++ library built on
[Boost fibers](https://www.boost.org/doc/libs/1_83_0/libs/fiber/doc/html/index.html).
It is based on [Helio](https://github.com/romange/helio).

This isn't intended as a replacement for Helio, but is just an interesting
project to understand how Helio works.

## Build
Build the library:
```
$ bazel build //puddle
```

Run the echo server example:
```
$ bazel run //examples/echo
```

Or build it with
```
$ bazel build //examples/echo
```
