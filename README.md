# Otter

> :warning: **Otter is still in development**

Otter is a simple key-value database.

This is a project to play around with database concepts, its not intended for
production use.

Otter currently supports simple `GET`, `PUT` and `DELETE` operations, build on
top of [RocksDB](https://rocksdb.org/). It uses
[Puddle](https://github.com/andydunstall/otter/tree/main/puddle) which is a
lightweight C++ asynchronous networking library build with Boost fibers and
`io_uring`.

# Next
Working on replacing RocksDB with a custom storage engine.
