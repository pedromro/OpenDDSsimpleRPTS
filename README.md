# OpenDDSsimpleRPTS


Ensure the env variables are set.
$ source /opt/OpenDDS/setenv.sh

To generate the C++ files from the IDL for Linux
$ mwc.pl -type gnuace

Build the target
$ make clean; make

Run the PUB SUb with RTPS discovery 
$ ./run_test.pl --rtps


# ##################################################

To generate the C++ files from the IDL for Windows
$ mwc.pl -type vc10

Open project in MSVC
$ devenv OpenDDSsimpleRPTS.sln


Build debug mode 
$ msbuild -p:Configuration=Debug,Platform=x64 -m OpenDDSsimpleRPTS.sln
