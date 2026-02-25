# mochaFiTrader

## Build instructions
1. create build directory
```bash
$ mkdir build
```

2. cd build/ directory
```bash
$ cd build/
```

3. cmake in build/ directory
```bash
$ cmake .. -GNinja -DCMAKE_TOOLCHAIN_FILE=~/code/vcpkg/scripts/buildsystems/vcpkg.cmake
```

4. make .exe file
```bash
$ cmake --build .
```

5. execute .exe file (from project root)
```bash
$ ./build/mochaFiTrader
```

## Rebuild instructions
1. Remove mochaFiTrader exe file
```bash
$ rm build/mochaFiTrader
```

2. make .exe file
```bash
$ cmake --build ./build/
```

3. execute .exe file (from project root)
```bash
$ ./build/mochaFiTrader
```

## Running Tests
- Ensure project has been compiled.
- Tests are to be executed from the following directory:
```bash
$ /mochaFiTrader
```

### Running all unit tests
```bash
$ ./build/runUnitTests
```

### Running integration tests
```bash
$ ./build/runIntegrationTests
```

### Running all tests combined
```bash
$ ./build/runAllTests
```
