#!/usr/bin/perl -w
use warnings;
use strict;

open ORIG_HTML, $ARGV[0] or die $!;

my $inedit = 0;
my @varStrings;
my $typeString;

$varStrings[4] = '<!-- #BeginEditable "PageNav" --> <!-- #EndEditable "PageNav"-->';

while(<ORIG_HTML>)
{
    if(/<!-- #BeginEditable "(.*)" -->(.*)<!-- #EndEditable ?(.*)-->/)
    {
        if($1 eq "doctitle")
        {
            $inedit = 1;
        }
        elsif($1 eq "Page%20Title")
        {
            $inedit = 2;
        }
        elsif($1 eq "Text")
        {
            $inedit = 3;
        }
        elsif($1 eq "PageNav")
        {
            $inedit = 4;
        }
        elsif($1 eq "Language%20Selector")
        {
            $inedit = 4;
        }
        else
        {
            die "Don't understand editable tag $1";
        }
        $varStrings[$inedit] = "<!-- #BeginEditable \"$1\" -->$2<!-- #EndEditable \"$1\"-->";
        $inedit = 0;
    }
    elsif(/<!-- #BeginEditable "(.*)" -->(.*)/)
    {
        if($1 eq "doctitle")
        {
            $inedit = 1;
        }
        elsif($1 eq "Page%20Title")
        {
            $inedit = 2;
        }
        elsif($1 eq "Text")
        {
            $inedit = 3;
        }
        elsif($1 eq "PageNav")
        {
            $inedit = 4;
        }
        elsif($1 eq "Language%20Selector")
        {
            $inedit = 4;
        }
        else
        {
            die "Don't understand editable tag $1";
        }
        $varStrings[$inedit] = "<!-- #BeginEditable \"$1\" -->$2";
        $typeString = $1;
    }
    elsif(/<!-- #EndEditable ?(.*)-->/)
    {
        s/<!-- #EndEditable -->/<!-- #EndEditable \"$typeString\"-->/g;
        $varStrings[$inedit] .= $_;
        $inedit = 0;
    }
    elsif($inedit != 0)
    {
        $varStrings[$inedit] .= $_;
    }
}
close(ORIG_HTML);

open NEW_HTML, ">$ARGV[0]" or die $!;

print NEW_HTML $varStrings[1] . "\n";
print NEW_HTML $varStrings[2] . "\n";
print NEW_HTML $varStrings[3] . "\n";

close(NEW_HTML);

$/ = undef;

open TEMPLATE, "$ARGV[1]" or die $!;

my $html = <TEMPLATE>;

close (TEMPLATE);

$/ = "\n";

$html =~ s/<doctitle>/$varStrings[1]/;
$html =~ s/<pagetitle>/$varStrings[2]/;
$html =~ s/<maintext>/$varStrings[3]/;
$html =~ s/<pagenav>/$varStrings[4]/;


open NEW_HTML, ">$ARGV[0]" or die $!;

print NEW_HTML $html . "\n";

close(NEW_HTML);
