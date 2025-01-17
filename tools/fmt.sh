#!/bin/sh

find . -name "*.h" -o -name "*.cc" | xargs clang-format -i
