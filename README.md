# OpenDDSsimpleRPTS


Ensure the env variables are set.
$ source /opt/OpenDDS/setenv.sh

To generate the C++ files from the IDL
$ mwc.pl -type gnuace

Build the target
$ make clean; make

Run the PUB SUb with RTPS discovery 
$ ./run_test.pl --rtps