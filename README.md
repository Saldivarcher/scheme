# Basic Scheme Interpreter

### How to build:
```sh
$ mkdir build
$ cd build

# Of course this doesn't need to be Ninja, and can be whatever you'd like.
# Also, the debug build utilizes address sanitizers.
$ cmake -G Ninja -DCMAKE_BUILD_TYPE=Debug ..
$ cd src && ./scheme
```

### How to run tests:
```sh
$ cd tests && ./test.sh
```
