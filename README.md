# Testsuite for the Dyninst tool for binary instrumentation, analysis, and modification

## Building Testsuite

To build testsuite, please first have the Dyninst version you want to test installed.
Then one can build the testsuite using cmake by specifying the path of dyninst installation using Dyninst\_ROOT.

  > mkdir build
  > cd build; 
  > cmake .. -DDyninst\_ROOT=/path/to/your/dyninst/installation -DCMAKE\_INSTALL\_PREFIX=/path/to/your/testsuite/intallation

## Running Testsuite

To run the testsuite, three paths need to be set through the enviromental variable:
DYNINSTAPI\_RT\_LIB should point to the libdyninstAPI\_RT.so under the Dyninst installation path.
LD\_LIBRARY\_PATH should include both the path of tht Dyninst installation and the path of the testsuite installation.

To enable debugging, one can run the testsuite with the following arguments

> -v -log logfilename -debugPrint

The testsuite can be run two modes: The full run mode and a one-test mode.

### runTests

The runTests executable will run all tests the comes with the testsuite.

Example usage :
> DYNINSTAPI\_RT\_LIB=dyninst-install/lib/libdyninstAPI\_RT.so LD\_LIBRARY\_PATH=dyninst-install/lib/:./:$LD\_LIBRARY\_PATH ./runTests -v -log output.log -debugPrint

### test\_driver

The test\_driver allows you to specify which test you want to run by passing hte following argument

> -test test-name

Example usage :
> DYNINSTAPI\_RT\_LIB=dyninst-install/lib/libdyninstAPI\_RT.so LD\_LIBRARY\_PATH=dyninst-install/lib/:./:$LD\_LIBRARY\_PATH ./test\_driver -v -log output.log -debugPrint -test pc\_irpc

### Documentation for additional control(WIP)

