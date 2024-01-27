#!/usr/bin/perl

use strict;
use warnings;

# Installed via `cpan install File::Slurp`
# Alternative `cpanm install File::Slurp`
use File::Slurp qw(read_file write_file);

my $tmpl_msvc_inc = <<EOTMPL;
    <ClInclude Include="\$(LIBSASS_INCLUDES_DIR)\\%s" />
EOTMPL

my $tmpl_msvc_head = <<EOTMPL;
    <ClInclude Include="\$(LIBSASS_HEADERS_DIR)\\%s" />
EOTMPL

my $tmpl_msvc_src = <<EOTMPL;
    <ClCompile Include="\$(LIBSASS_SRC_DIR)\\%s" />
EOTMPL

my $tmpl_msvc_filter_api_inc = <<EOTMPL;
    <ClInclude Include="\$(LIBSASS_INCLUDES_DIR)\\%s">
      <Filter>API Includes</Filter>
    </ClInclude>
EOTMPL

my $tmpl_msvc_filter_api_head = <<EOTMPL;
    <ClInclude Include="\$(LIBSASS_HEADERS_DIR)\\%s">
      <Filter>CAPI Headers</Filter>
    </ClInclude>
EOTMPL

my $tmpl_msvc_filter_api_src = <<EOTMPL;
    <ClCompile Include="\$(LIBSASS_SRC_DIR)\\%s">
      <Filter>CAPI Sources</Filter>
    </ClCompile>
EOTMPL

my $tmpl_msvc_filter_ast_head = <<EOTMPL;
    <ClInclude Include="\$(LIBSASS_HEADERS_DIR)\\%s">
      <Filter>AST Headers</Filter>
    </ClInclude>
EOTMPL

my $tmpl_msvc_filter_ast_src = <<EOTMPL;
    <ClCompile Include="\$(LIBSASS_SRC_DIR)\\%s">
      <Filter>AST Sources</Filter>
    </ClCompile>
EOTMPL

my $tmpl_msvc_filter_parser_head = <<EOTMPL;
    <ClInclude Include="\$(LIBSASS_HEADERS_DIR)\\%s">
      <Filter>Parser Headers</Filter>
    </ClInclude>
EOTMPL

my $tmpl_msvc_filter_parser_src = <<EOTMPL;
    <ClCompile Include="\$(LIBSASS_SRC_DIR)\\%s">
      <Filter>Parser Sources</Filter>
    </ClCompile>
EOTMPL

my $tmpl_msvc_filter_fn_head = <<EOTMPL;
    <ClInclude Include="\$(LIBSASS_HEADERS_DIR)\\%s">
      <Filter>Function Headers</Filter>
    </ClInclude>
EOTMPL

my $tmpl_msvc_filter_fn_src = <<EOTMPL;
    <ClCompile Include="\$(LIBSASS_SRC_DIR)\\%s">
      <Filter>Function Sources</Filter>
    </ClCompile>
EOTMPL

my $tmpl_msvc_filter_head = <<EOTMPL;
    <ClInclude Include="\$(LIBSASS_HEADERS_DIR)\\%s">
      <Filter>LibSass Headers</Filter>
    </ClInclude>
EOTMPL

my $tmpl_msvc_filter_src = <<EOTMPL;
    <ClCompile Include="\$(LIBSASS_SRC_DIR)\\%s">
      <Filter>LibSass Sources</Filter>
    </ClCompile>
EOTMPL

# parse source files directly from libsass makefile
open(my $fh, "<", "../Makefile.conf") or
    die "../Makefile.conf not found";
my $srcfiles = join "", <$fh>; close $fh;

my (@INCFILES, @HPPFILES, @SOURCES, @CSOURCES);
# parse variable out (this is hopefully tolerant enough)
if ($srcfiles =~ /^\s*INCFILES\s*=\s*((?:.*(?:\\\r?\n))*.*)/m) {
	@INCFILES = grep { $_ } split /(?:\s|\\\r?\n)+/, $1;
} else { die "Did not find c++ INCFILES in libsass/Makefile.conf"; }
if ($srcfiles =~ /^\s*HPPFILES\s*=\s*((?:.*(?:\\\r?\n))*.*)/m) {
	@HPPFILES = grep { $_ } split /(?:\s|\\\r?\n)+/, $1;
} else { die "Did not find c++ HPPFILES in libsass/Makefile.conf"; }
if ($srcfiles =~ /^\s*SOURCES\s*=\s*((?:.*(?:\\\r?\n))*.*)/m) {
	@SOURCES = grep { $_ } split /(?:\s|\\\r?\n)+/, $1;
} else { die "Did not find c++ SOURCES in libsass/Makefile.conf"; }
if ($srcfiles =~ /^\s*CSOURCES\s*=\s*((?:.*(?:\\\r?\n))*.*)/m) {
	@CSOURCES = grep { $_ } split /(?:\s|\\\r?\n)+/, $1;
} else { die "Did not find c++ CSOURCES in libsass/Makefile.conf"; }

sub renderTemplate($@) {
    my $str = "\n";
    my $tmpl = shift;
    foreach my $inc (@_) {
        $str .= sprintf($tmpl, $inc);
    }
    $str .= "  ";
    return $str;
}

@INCFILES = map { s/\//\\/gr } @INCFILES;
@HPPFILES = map { s/\//\\/gr } @HPPFILES;
@SOURCES = map { s/\//\\/gr } @SOURCES;
@CSOURCES = map { s/\//\\/gr } @CSOURCES;

my @APIHEADERS = grep { m/^capi[_.]/} @HPPFILES;
my @LIBHEADERS = grep { not m/^capi[_.]/} @HPPFILES;
my @APISOURCES = grep { m/^capi[_.]/} @SOURCES, @CSOURCES;
my @LIBSOURCES = grep { not m/^capi[_.]/} @SOURCES, @CSOURCES;

my @ASTHEADERS = grep { m/^ast[_.]/} @LIBHEADERS;
@LIBHEADERS = grep { not m/^ast[_.]/} @LIBHEADERS;
my @ASTSOURCES = grep { m/^ast[_.]/} @LIBSOURCES;
@LIBSOURCES = grep { not m/^ast[_.]/} @LIBSOURCES;

my @PARSERHEADERS = grep { m/^parser[_.]/} @LIBHEADERS;
@LIBHEADERS = grep { not m/^parser[_.]/} @LIBHEADERS;
my @PARSERSOURCES = grep { m/^parser[_.]/} @LIBSOURCES;
@LIBSOURCES = grep { not m/^parser[_.]/} @LIBSOURCES;

my @FNHEADERS = grep { m/^fn[_.]/} @LIBHEADERS;
@LIBHEADERS = grep { not m/^fn[_.]/} @LIBHEADERS;
my @FNSOURCES = grep { m/^fn[_.]/} @LIBSOURCES;
@LIBSOURCES = grep { not m/^fn[_.]/} @LIBSOURCES;

my $targets = read_file("build-skeletons/libsass.targets");
$targets =~s /\{\{api_includes\}\}/renderTemplate($tmpl_msvc_inc, @INCFILES)/eg;
$targets =~s /\{\{api_headers\}\}/renderTemplate($tmpl_msvc_head, @APIHEADERS)/eg;
$targets =~s /\{\{api_sources\}\}/renderTemplate($tmpl_msvc_src, @APISOURCES)/eg;
$targets =~s /\{\{ast_headers\}\}/renderTemplate($tmpl_msvc_head, @ASTHEADERS)/eg;
$targets =~s /\{\{ast_sources\}\}/renderTemplate($tmpl_msvc_src, @ASTSOURCES)/eg;
$targets =~s /\{\{parser_headers\}\}/renderTemplate($tmpl_msvc_head, @PARSERHEADERS)/eg;
$targets =~s /\{\{parser_sources\}\}/renderTemplate($tmpl_msvc_src, @PARSERSOURCES)/eg;
$targets =~s /\{\{fn_headers\}\}/renderTemplate($tmpl_msvc_head, @FNHEADERS)/eg;
$targets =~s /\{\{fn_sources\}\}/renderTemplate($tmpl_msvc_src, @FNSOURCES)/eg;
$targets =~s /\{\{headers\}\}/renderTemplate($tmpl_msvc_head, @LIBHEADERS)/eg;
$targets =~s /\{\{sources\}\}/renderTemplate($tmpl_msvc_src, @LIBSOURCES)/eg;
warn "Generating ../win/libsass.targets\n";
write_file("../win/libsass.targets", $targets);

my $filters = read_file("build-skeletons/libsass.vcxproj.filters");
$filters =~s /\{\{api_includes\}\}/renderTemplate($tmpl_msvc_filter_api_inc, @INCFILES)/eg;
$filters =~s /\{\{api_headers\}\}/renderTemplate($tmpl_msvc_filter_api_head, @APIHEADERS)/eg;
$filters =~s /\{\{api_sources\}\}/renderTemplate($tmpl_msvc_filter_api_src, @APISOURCES)/eg;
$filters =~s /\{\{ast_headers\}\}/renderTemplate($tmpl_msvc_filter_ast_head, @ASTHEADERS)/eg;
$filters =~s /\{\{ast_sources\}\}/renderTemplate($tmpl_msvc_filter_ast_src, @ASTSOURCES)/eg;
$filters =~s /\{\{parser_headers\}\}/renderTemplate($tmpl_msvc_filter_parser_head, @PARSERHEADERS)/eg;
$filters =~s /\{\{parser_sources\}\}/renderTemplate($tmpl_msvc_filter_parser_src, @PARSERSOURCES)/eg;
$filters =~s /\{\{fn_headers\}\}/renderTemplate($tmpl_msvc_filter_fn_head, @FNHEADERS)/eg;
$filters =~s /\{\{fn_sources\}\}/renderTemplate($tmpl_msvc_filter_fn_src, @FNSOURCES)/eg;
$filters =~s /\{\{headers\}\}/renderTemplate($tmpl_msvc_filter_head, @LIBHEADERS)/eg;
$filters =~s /\{\{sources\}\}/renderTemplate($tmpl_msvc_filter_src, @LIBSOURCES)/eg;
warn "Generating ../win/libsass.vcxproj.filters\n";
write_file("../win/libsass.vcxproj.filters", $filters);

