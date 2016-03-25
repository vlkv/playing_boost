# playing_boost
I'm just playing with C++ boost, a great library...

BUILD INSTRUCTIONS
* Project can be built with MS Visual Studio Community 2015 (14.0.24720.00 Update 1)
* Boost library is supposed to be installed at C:\Program Files\boost\boost_1_60_0
* In Configuration Properties > C/C++ > General > Additional Include Directories, enter the path to the Boost root directory: C:\Program Files\boost\boost_1_60_0
* In Configuration Properties > Linker > Additional Library Directories, enter the path to the Boost binaries, e.g. C:\Program Files\boost\boost_1_60_0\lib\
* In Configuration Properties > C/C++ > Preprocessor > Preprocessor Definitions, add _WIN32_WINNT=0x0601 (the value would be WIN32;_DEBUG;_CONSOLE;_WIN32_WINNT=0x0601)
* Build configuration "Debug", "x86"

TEST
* Run test.cmd. This script starts a server (from Debug subdirectory) and a number of clients.
* 

