###################################################################
#
# Dette programet lager en makemaplist.conf fil ut fra mapper 
# den f�r inn. Vil pr�ve � spre dataene gjevnt over.
#
#
###################################################################

if ($#ARGV < 0) {
	print "usage: perl perl/makemaplist.pl directory [ directory ]";
}

for my $i (0 .. $#ARGV) {
	print "$ARGV[$i]\n";
}

for my $i (0 .. 64) {

	my $nr = $i % ($#ARGV +1);

	#print "nr: $nr (i: $i, a: $#ARGV)\n";
	print "$ARGV[$nr]\n";
}
