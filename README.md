# HTTP Server Benchmarks

> Raw socket communication performance of HTTP servers built-in to various languages.

While there are standalone and mature web servers which support plugins to build web applications, embedded web servers are often used in micro-services, for example. Every modern language includes a standard or optional library to implement a HTTP server easily.

## Results

I included [OpenResty] ([NGINX] with [Lua] support) for a comparison with a standalone web server. It does not actually compuete with the embedded web server libraries.

| Place | Language   | Latency  | Requests/sec | Transfer/sec | Errors |
| ----- | ---------- | -------- | ------------ | ------------ | ------ |
| N/A   | openresty  | 2.93ms   | 55108.22     | 9.35MB       | 106    |
|  1.   | go         | 2.47ms   | 96608.70     | 8.57MB       | 95     |
|  2.   | cpp        | 2.55ms   | 93476.56     | 8.29MB       | 95     |
|  3.   | java8      | 2.84ms   | 83911.32     | 7.44MB       | 91     |
|  4.   | crystal    | 2.86ms   | 83468.51     | 7.40MB       | 101    |
|  5.   | java14     | 5.61ms   | 46201.69     | 4.67MB       | 1010   |
|  6.   | nodejs     | 6.28ms   | 38117.53     | 4.73MB       | 157    |
|  7.   | nim        | 24.49ms  | 10397.46     | 0.92MB       | 218    |
|  8.   | lowjs      | 40.24ms  | 3368.82      | 300.53KB     | 4744   |
|  9.   | lua        | 4.62ms   | 1524.49      | 138.45KB     | 564079 |
| 10.   | dotnet     | 5.13ms   | 1219.43      | 110.75KB     | 513672 |
| 11.   | v          | 6.33ms   | 457.51       | 45.57KB      | 3914   |
| 12.   | rust       | 4.40ms   | 394.49       | 26.58KB      | 18327  |
| 13.   | c          | 4.66ms   | 380.91       | 34.59KB      | 17720  |
| 14.   | python     | 499.73us | 275.97       | 32.88KB      | 14009  |

## Configuration

The test applications serve a minimum response to measure only their network communication throughput:

    HTTP/1.0 200 OK
    Connection: keep-alive
    Content-Type: text/plain
    Content-Length: 5

    Hello

The test ses 400 connections in 12 threads for 30 seconds:

    wrk -t12 -c400 -d30s http://127.0.0.1:7012/

## Environment

The results were measured on the following hardware and software:

    MacBook Pro
    2,6 GHz 6-Core Intel Core i7
    16 GB 2400 MHz DDR4
    OSX 11.2.3

The following versions of languages and libraries were used:

    openresty 1.19.3.2
    c Apple clang 12.0.5 (clang-1205.0.22.9)
    c++ Apple clang 12.0.5 (clang-1205.0.22.9)
    crystal 1.0.0 (LLVM: 9.0.1)
    dotnet 5.0.203 (Microsoft.NETCore.App 5.0.6)
    go 1.16.4
    java (openjdk) 14.0.2
    low 20210228_3479cc6
    lua 5.4.3 (luasocket 3.0rc1-2)
    nim 1.4.8
    node 14.17.0
    python 3.9.5
    rust 1.52.1 (cargo 1.52.0, async-std 1.9.0)
    v 0.2.2

## Installation

Install languages and test tools using Homebrew:

    brew install make wrk gcc crystal dotnet golang nim lowjs lua python rust vlang

Node.js and Java are expected to be already installed, which you have probably done using `nvm` and `jenv`.

## Test

Testing servers can be built by GNU Make:

    make clean all

Almost every directory will contain an executable called `srv`. (Only `dotnet`, `java` and `rust` differ.) These executables can be started one-by-one using `make <directory name>` and tested by running `make test`, for example:

    make go & sleep 5 && make test && kill %1

## Console Output

### openresty

    Running 30s test @ http://127.0.0.1:7012/
      12 threads and 400 connections
      Thread Stats   Avg      Stdev     Max   +/- Stdev
        Latency     2.93ms   10.09ms 249.93ms   99.52%
        Req/Sec     8.31k     3.53k   16.54k    60.43%
      1659000 requests in 30.10s, 281.54MB read
      Socket errors: connect 157, read 106, write 0, timeout 0
    Requests/sec:  55108.22
    Transfer/sec:      9.35MB

### c

    Running 30s test @ http://127.0.0.1:7012/
      12 threads and 400 connections
      Thread Stats   Avg      Stdev     Max   +/- Stdev
        Latency     4.66ms    1.60ms  10.78ms   77.93%
        Req/Sec   798.62    518.67     2.07k    64.49%
      11464 requests in 30.10s, 1.02MB read
      Socket errors: connect 157, read 17720, write 173, timeout 0
    Requests/sec:    380.91
    Transfer/sec:     34.59KB

### cpp

    Running 30s test @ http://127.0.0.1:7012/
      12 threads and 400 connections
      Thread Stats   Avg      Stdev     Max   +/- Stdev
        Latency     2.55ms  263.04us   5.69ms   75.59%
        Req/Sec     7.83k     3.26k   15.76k    60.44%
      2813779 requests in 30.10s, 249.56MB read
      Socket errors: connect 157, read 95, write 0, timeout 0
    Requests/sec:  93476.56
    Transfer/sec:      8.29MB

### crystal

    Running 30s test @ http://127.0.0.1:7012/
      12 threads and 400 connections
      Thread Stats   Avg      Stdev     Max   +/- Stdev
        Latency     2.86ms  339.27us   8.75ms   80.69%
        Req/Sec     6.99k     3.31k   13.09k    64.80%
      2512562 requests in 30.10s, 222.84MB read
      Socket errors: connect 157, read 101, write 0, timeout 0
    Requests/sec:  83468.51
    Transfer/sec:      7.40MB

### dotnet

    Running 30s test @ http://127.0.0.1:7012/
      12 threads and 400 connections
      Thread Stats   Avg      Stdev     Max   +/- Stdev
        Latency     5.13ms   14.39ms 920.34ms   99.89%
        Req/Sec   107.94     75.11     1.71k    74.82%
      36651 requests in 30.06s, 3.25MB read
      Socket errors: connect 157, read 513672, write 1479, timeout 0
    Requests/sec:   1219.43
    Transfer/sec:    110.75KB

### go

    Running 30s test @ http://127.0.0.1:7012/
      12 threads and 400 connections
      Thread Stats   Avg      Stdev     Max   +/- Stdev
        Latency     2.47ms  283.76us  15.07ms   81.56%
        Req/Sec     8.09k     3.39k   16.70k    54.08%
      2908110 requests in 30.10s, 257.93MB read
      Socket errors: connect 157, read 95, write 0, timeout 0
    Requests/sec:  96608.70
    Transfer/sec:      8.57MB

### java8

    Running 30s test @ http://127.0.0.1:7012/
      12 threads and 400 connections
      Thread Stats   Avg      Stdev     Max   +/- Stdev
        Latency     2.84ms  333.96us  15.09ms   87.25%
        Req/Sec     7.03k     3.27k   12.78k    51.86%
      2519155 requests in 30.02s, 223.43MB read
      Socket errors: connect 157, read 91, write 0, timeout 0
    Requests/sec:  83911.32
    Transfer/sec:      7.44MB

### java14

    Running 30s test @ http://127.0.0.1:7012/
      12 threads and 400 connections
      Thread Stats   Avg      Stdev     Max   +/- Stdev
        Latency     5.61ms   10.19ms 364.67ms   99.57%
        Req/Sec     3.89k     1.69k   22.07k    60.78%
      1390755 requests in 30.10s, 140.59MB read
      Socket errors: connect 157, read 1010, write 2, timeout 0
    Requests/sec:  46201.69
    Transfer/sec:      4.67MB

### lowjs

    Running 30s test @ http://127.0.0.1:7012/
      12 threads and 400 connections
      Thread Stats   Avg      Stdev     Max   +/- Stdev
        Latency    40.24ms  108.97ms   1.99s    97.07%
        Req/Sec   500.68    261.54     1.40k    68.38%
      101406 requests in 30.10s, 8.83MB read
      Socket errors: connect 157, read 4744, write 0, timeout 152
      Non-2xx or 3xx responses: 2577
    Requests/sec:   3368.82
    Transfer/sec:    300.53KB

### lua

    Running 30s test @ http://127.0.0.1:7012/
      12 threads and 400 connections
      Thread Stats   Avg      Stdev     Max   +/- Stdev
        Latency     4.62ms   20.05ms 935.75ms   99.89%
        Req/Sec   137.21    101.91     1.88k    73.69%
      45837 requests in 30.07s, 4.07MB read
      Socket errors: connect 157, read 564079, write 2437, timeout 0
    Requests/sec:   1524.49
    Transfer/sec:    138.45KB

### nim

    Running 30s test @ http://127.0.0.1:7012/
      12 threads and 400 connections
      Thread Stats   Avg      Stdev     Max   +/- Stdev
        Latency    24.49ms   28.69ms 923.64ms   99.41%
        Req/Sec     0.87k   351.22     2.52k    58.31%
      312906 requests in 30.09s, 27.75MB read
      Socket errors: connect 157, read 218, write 1, timeout 0
    Requests/sec:  10397.46
    Transfer/sec:      0.92MB

### nodejs

    Running 30s test @ http://127.0.0.1:7012/
      12 threads and 400 connections
      Thread Stats   Avg      Stdev     Max   +/- Stdev
        Latency     6.28ms    1.32ms  72.84ms   98.09%
        Req/Sec     3.20k     1.23k    5.13k    56.25%
      1147512 requests in 30.10s, 142.27MB read
      Socket errors: connect 157, read 195, write 0, timeout 0
    Requests/sec:  38117.53
    Transfer/sec:      4.73MB

### python

    Running 30s test @ http://127.0.0.1:7012/
      12 threads and 400 connections
      Thread Stats   Avg      Stdev     Max   +/- Stdev
        Latency   499.73us  217.27us   6.97ms   88.30%
        Req/Sec   409.23    453.78     2.43k    82.90%
      8307 requests in 30.10s, 0.97MB read
      Socket errors: connect 157, read 14009, write 12, timeout 0
    Requests/sec:    275.97
    Transfer/sec:     32.88KB

### rust

    Running 30s test @ http://127.0.0.1:7012/
      12 threads and 400 connections
      Thread Stats   Avg      Stdev     Max   +/- Stdev
        Latency    10.51ms   84.74ms   1.39s    99.62%
        Req/Sec   622.19    421.54     1.44k    60.00%
      11510 requests in 30.08s, 1.02MB read
      Socket errors: connect 157, read 18114, write 138, timeout 0
    Requests/sec:    382.60
    Transfer/sec:     34.75KB

### v

    Running 30s test @ http://127.0.0.1:7012/
      12 threads and 400 connections
      Thread Stats   Avg      Stdev     Max   +/- Stdev
        Latency     6.33ms    1.64ms   9.60ms   86.84%
        Req/Sec     0.95k   573.43     2.13k    65.00%
      13765 requests in 30.09s, 1.34MB read
      Socket errors: connect 157, read 3914, write 0, timeout 0
    Requests/sec:    457.51
    Transfer/sec:     45.57KB

## Contributing

In lieu of a formal styleguide, take care to maintain the existing coding style.

## License

Copyright (c) 2021 Ferdinand Prantl

Licensed under the MIT license.

[OpenResty]: https://openresty.org/en/
[NGINX]: https://www.nginx.com/
[Lua]: http://www.lua.org/
