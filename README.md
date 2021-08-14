# Virtual vector

### Overview

`virtual_vec` is a type compatible with `std::vector<T>`. The implementation targets large vectors that are pushed to frequently and guarantees `O(1)` for appending to the end. Do note the purpose of this implementation is experimentation, and it has not been tested in a production system.

##### Strategy

When the vector is instantiated, it reserves a large chunk of virtual memory addresses from the operating system. Memory from this address space is committed to our process as the vector grows. We never have to copy the entire vector to a new address space.

This implementation is hardcoded to use 4GB virtual address for each vector, so you can have billions of these per process. The number was chosen arbitrarily and can be made bigger or smaller depending on your use case. For obvious reasons this is only applicable to 64-bit systems.

### Building

NOTE: Right now this only builds on Linux (would gladly accept patches to support other OSes). To build this you just need to include `virtual_vec.h` and `virtual_vec.cpp` and a c++17 (or newer) compiler.

#### Running tests

[gtest](https://google.github.io/googletest/) must be installed.

To run the tests:

```

./excute test

```

To run the tests under valgrind:

```

./execute test-valgrind

```

#### Running benchmarks

[Google benchmark](https://github.com/google/benchmark) must be installed.

```

./execute bench

```

### Benchmarks

Below is a table of benchmarks comparing the performance of `virtual_vec<int64_t>` to `std::vector<int64_t>`. The test calls `push_back` until `# elems / sizeof(int64_t)` exceeds `Storage size`.

All benchmarks are performed on a `Intel i7-8665U (8) @ 4.800GHz`. Time is recorded in CPU time.

One thing to keep in mind is these benchmarks were done with `-O2`. With `-O3` std::vector gets 10% faster.

| Storage size  | std::vector (ns)       | virtual_vec (ns)       | diff (%) |
| ------------- | ---------------------- | ---------------------- | -------- |
| 512B          | 1221                   | 755                    | 38       |
| 1KB           | 2248                   | 1627                   | 27       |
| 8KB           | 16635                  | 12635                  | 24       |
| 32KB          | 85878                  | 38748                  | 54       |
| 131KB         | 375485                 | 234833                 | 37       |
| 524KB         | 1534855                | 798632                 | 47       |
| 1MB           | 2940128                | 1276827                | 56       |
| 4MB           | 13290644               | 6129833                | 53       |
| 8MB           | 29162281               | 12560318               | 56       |
| 10MB          | 39694352               | 13500909               | 65       |