# Testsuite for the Dyninst tool for binary instrumentation, analysis, and modification

## Building Testsuite

To build testsuite, please first have the Dyninst version you want to test installed.
Then one can build the testsuite using cmake by specifying the path of dyninst installation using `Dyninst_ROOT`.

```
mkdir build

cd build

cmake .. -DDyninst_ROOT=/path/to/your/dyninst/installation -DCMAKE_INSTALL_PREFIX=/path/to/your/testsuite/intallation
```

## Running Testsuite

To run the testsuite, three paths need to be set through the enviromental variable:

* `DYNINSTAPI_RT_LIB` should point to the `libdyninstAPI_RT.so` under the Dyninst installation path.

* `LD_LIBRARY_PATH` should include both the path of tht Dyninst installation and the path of the testsuite installation.

To enable debugging, one can run the testsuite with the following arguments

```
-v -log logfilename -debugPrint
```

The testsuite can be run two modes: The full run mode and a one-test mode.

## runTests

The runTests executable will run all tests the comes with the testsuite.

Example usage:

```
export DYNINSTAPI_RT_LIB=dyninst-install/lib/libdyninstAPI_RT.so
export LD_LIBRARY_PATH=dyninst-install/lib/:./:$LD_LIBRARY_PATH
./runTests -v -log output.log -debugPrint -all
```

## test\_driver

The test\_driver allows you to specify which test you want to run by passing the following argument

`
-test test-name
`

Example usage:

```
export DYNINSTAPI_RT_LIB=dyninst-install/lib/libdyninstAPI_RT.so
export LD_LIBRARY_PATH=dyninst-install/lib/:./:$LD_LIBRARY_PATH
./test_driver -v -log output.log -debugPrint -test pc_irpc
```

### Documentation for additional control(WIP)

