# Benchmark Results

Benchmarked locally with increased ulimit and tcp_mem
Server is running the strawpoll app

## 100 concurrent connections

### Performance | connections per ms
* uWebSockets: 6.3, 5.6, 5.2, 5.6, 6.7, 7.1, 5.6, 6,3, 11.1, 5.6
* Beast:
* IncludeOS:

### Memory Usage | user space in mb
* uWebSockets: 4.6, 4.6, 4.6, 4.6, 4.6, 4.6, 4.6, 4.6, 4.6, 4.6
* Beast:
* IncludeOS:

## 10.000 concurrent connections

### Performance | connections per ms
* uWebSockets: 35.1, 32.8, 35.0, 32.9, 34.0
* Beast:
* IncludeOS:

### Memory Usage | user space in mb
* uWebSockets: 6.4, 6.4, 6.2, 6.5, 6.3
* Beast:
* IncludeOS:

## Compile Time

### Top level change | Time in seconds

Debug:
* uWebSockets: 1.4, 1.4, 1.4, 1.4, 1.4, 1.4, 1.4
* Beast: 6.9, 8.7, 8.5, 7.3, 8.7, 8.5, 7.0
* IncludeOS:

Release:
* uWebSockets: 1.8, 1.9, 2.1, 1.7, 1.7, 1.7
* Beast: 14.4, 17.1, 14.5, 16,3, 18.0, 15.5, 16.4
* IncludeOS: 2.0, 2.5, 2.2, 2.9, 2.0, 2.9, 2.2, 3.0, 2.2

## Code Size
* uWebSockets | ~1.6kb
* Beast | ~9kb
* IncludeOS | ~3kb

### Beast example Comparison | 100 concurrent connections
* sync        5mb | 4.5c/ms
* stackless   2mb | 5.5c/ms
* fast        9mb | 6.2c/ms
* coro        6mb | 5.5c/ms
* async       4mb | 5.2c/ms

### Beast example Comparison | 10.000 concurrent connections
* sync        100mb | 17c/ms
* stackless   50mb  | 26c/ms
* fast        150mb | 17c/ms
* coro        140mb | 22c/ms
* async       50mb  | 25c/ms
