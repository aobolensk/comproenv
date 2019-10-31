# Building

## Requirements:
- [CMake](https://cmake.org/download/) 3.5+
- [Python](https://www.python.org/downloads/) 3.7+ (needed for website parser)
  with modules:
  - bs4
  - requests
  - termcolor
- [Optional] [rlwrap](https://github.com/hanslub42/rlwrap) - readline wrapper (Linux)

## Build and run:

```console
# Via python script:
$ python launch.py --f build run                                     # Default build

$ python launch.py --f build run --r config/linux_sample.yaml        # With config file specification

$ python launch.py --f build run --b "\-DCMAKE_BUILD_TYPE=Debug"    # Debug build

$ python launch.py --f build run --b "\-DEXP_FS=1"                  # Use std::experimental::filesystem
# Use -DEXP_FS=1 only when comproenv does not compile on your machine
# because of using more modern std::filesystem

# Docker:
$ docker build -t comproenv .
$ docker run -v comproenv:/app -it comproenv

# Alternative (without scripts):
$ git submodule update --init --recursive
$ mkdir build
$ cd build
$ cmake ..
$ cmake --build .
$ ./bin/comproenv
```
