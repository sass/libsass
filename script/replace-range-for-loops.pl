# this script may be used to replace `for(auto : xyz)` loops
# with a more std compliant form that also compiles on gcc 4.4
# albeit completely unsupported by libsass, you may use it
# if there is no way to upgrade your local compiler. Mileage
# may vary, and I want to stress again that this is unsupported.

use strict;
use warnings;

use File::Slurp;
use File::Basename;
use File::Spec::Functions;

warn "YOU ARE ENTERING UNSUPPORTED LAND !!!!\n";
warn "DON'T POST BUGS WHEN USING GCC 4.4 !!!!\n";

my $root = $ENV{'SASS_LIBSASS_PATH'} || catfile(dirname($0), '..');

sub process($)
{

	my $count = 0;
	my ($file) = @_;

	my $cpp = read_file($file, { binmode => ':raw' });

	my $org = $cpp;

	my $re_decl = qr/(?:const\s*)?\w+(?:\:\:\w+)*(?:\s*[\*\&])?/;
	my $re_val = qr/\w+(?:\(\))?(?:(?:->|\.)\w+(?:\(\))?)*/;

	$cpp =~ s/for\s*\(\s*($re_decl)\s*(\w+)\s*:\s*(\(\*?$re_val\)|\*?$re_val)\s*\)\s*{/
		$count ++;
		"for (auto __$2 = ($3).begin(); __$2 != ($3).end(); ++__$2) { $1 $2 = *(__$2);";
	/gex;

	return if $org eq $cpp || $count == 0;

	warn sprintf "made %02d replacements in %s\n", $count, $file;

	write_file($file, { binmode => ':raw' }, $cpp);

}

sub processdir($)
{
	my $rv = opendir(my $dh, $_[0]);
	die "not found ", $_[0] unless $rv;
	while (my $entry = readdir($dh)) {
		next if $entry eq "." || $entry eq "..";
		next unless $entry =~ m/\.[hc]pp$/;
		if (-d $_[0]) { process(catfile($_[0], $entry)); }
		elsif (-f $_[0]) { process(catfile($_[0], $entry)); }
	}
}

processdir catfile($root, "src");
