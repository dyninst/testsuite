# Testsuite for the Dyninst tool for binary instrumentation, analysis, and modification

## Building and Installing Testsuite

The testsuite needs to be installed as it cannot be run from within the build directory.

To build the testsuite, please first have the Dyninst version you want to test installed.

Then one can build the testsuite using cmake by specifying the path of dyninst installation using `Dyninst_ROOT`.

### define variable used here and in the rest of this README

```shell
DYNINST_INSTALL=/path/to/your/Dyninst/installation
TESTSUITE_DIR=/path/to/the/testsuite
TESTSUITE_BUILD=$TESTSUITE_DIR/build
TESTSUITE_INSTALL=$TESTSUITE_DIR/install

mkdir $TESTSUITE_BUILD
cd $TESTSUITE_BUILD

# configure
cmake -DDyninst_ROOT=$DYNINST_INSTALL -DCMAKE_INSTALL_PREFIX=$TESTSUITE_INSTALL ..

# build and install
make -j install

# find the Dyninst lib directory (varies by platform, so use glob)
DYNINST_INSTALL_LIB=$(echo $DYNINST_INSTALL/lib*)
if [ ! -d "$DYNINST_INSTALL_LIB" ]; then
    # check that lib directory exists; detect no or more than 1 glob match
    echo "ERROR: Dyninst install lib directory not found: $DYNINST_INSTALL_LIB" 1>&2
fi

# define more variables needed to run the test suite
export DYNINSTAPI_RT_LIB=$DYNINST_INSTALL_LIB/libdyninstAPI_RT.so
TESTSUITE_LD_LIBRARY_PATH=$DYNINST_INSTALL_LIB:$TESTSUITE_INSTALL:$LD_LIBRARY_PATH
```

## Running Testsuite

To run the testsuite, three paths need to be set through the enviromental variable:

* `DYNINSTAPI_RT_LIB` should point to the `libdyninstAPI_RT.so` under the Dyninst installation path.

* `LD_LIBRARY_PATH` should include both the path of tht Dyninst installation and the path of the testsuite installation.

To enable debugging, one can run the testsuite with the following arguments

```
-v -log logfilename -debugPrint
```

The testsuite can be run two modes: To run all tests or to run a specific test.

## runTests

The runTests executable will run all the tests that comes with the testsuite.

Example usage:

```shell
cd $TESTSUITE_INSTALL
LD_LIBRARY_PATH=$TESTSUITE_LD_LIBRARY_PATH ./runTests -v -log output.log -debugPrint -all
```

## test\_driver

The test\_driver executable allows you run a specific test by passing the following argument

`
-test test-name
`

Example usage:

```shell
cd $TESTSUITE_INSTALL
LD_LIBRARY_PATH=$TESTSUITE_LD_LIBRARY_PATH ./test_driver -v -log output.log -debugPrint -test pc_irpc
```

### Documentation for additional control(WIP)

