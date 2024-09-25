eval '(exit $?0)' && eval 'exec perl -S $0 ${1+"$@"}'
    & eval 'exec perl -S $0 $argv:q'
    if 0;

my @original_ARGV = @ARGV;

use Env (DDS_ROOT);
use lib "$DDS_ROOT/bin";
use Env (ACE_ROOT);
use lib "$ACE_ROOT/bin";
use Getopt::Long;
use PerlDDS::Run_Test;
use strict;

my $dbg_opts = " -ORBDebugLevel 1";
my $common_opts = "$dbg_opts";

$common_opts .= " -DCPSConfigFile rtps.ini";


my $test = new PerlDDS::TestFramework();

$test->{dcps_debug_level} = 1;
$test->{dcps_transport_debug_level} = 1;
$test->{add_transport_config} = 0;

my $pub_opts = "$common_opts";
my $sub_opts = "$common_opts";

$test->process("publisher", "publisher", $pub_opts);
$test->process("subscriber", "subscriber", $sub_opts);

$test->start_process("subscriber");
$test->start_process("publisher");

exit $test->finish(120);
