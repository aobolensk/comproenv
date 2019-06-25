# comproenv
Environment for competitive programming

## Requirements:
- Python 3.7+ (needed for website parser)  
  with modules:
  - bs4
  - requests
  - termcolor

## Build and run:

```console
# Via python script:
$ python launch.py --f build run                                         # Default build

$ python launch.py --f build run --r config/linux_sample.yaml            # With config file specification

$ python launch.py --f build run --b \"-DCMAKE_BUILD_TYPE=Debug\"        # Debug build

# Alternative (without scripts):
$ git submodule update --init --recursive
$ mkdir build
$ cd build
$ cmake ..
$ cmake --build .
$ ./bin/comproenv
```
