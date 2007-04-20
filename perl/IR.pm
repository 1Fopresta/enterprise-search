
package IR;
require Exporter;
@IR::ISA = qw(Exporter);
@IR::EXPORT = qw();
@IR::EXPORT_OK = qw(lexer stoppord ResulveUrl adultFilter);


use URI::URL;

#########################################################################################################
#
#########################################################################################################
sub lexer {
	my($tegn,$TypePosition,%termer) = @_;
	my($term);


	#gj�r om tegn
	
	$tegn =~ s/&aring;/�/g;
	$tegn =~ s/&aelig;/�/g;
	$tegn =~ s/&oslash;/�/g;

	#$tegn =~ s/&lt;/\</g;
	#$tegn =~ s/&gt;/\>/g;
	#\21 okt 2005: fjerner dette da det tar for mye tid
#	$tegn =~ s/&amp;/&/g;
#	$tegn =~ s/&quot;/"/g;
#	$tegn =~ s/&nbsp;/ /g;
	#/
	
	#gj�r om bindestrek til space
	#$tegn =~ s/-/ /g;
	#$tegn =~ s/_/ /g;
	
	#konverterer til lowercase
	$tegn = lc($tegn);
	
	#tar ut allt bortsett fra a-z ���
	#$tegn =~ s/[^a-z��� ]//g;
	
	#erstatter alle spacer etter hverandre med en
	$tegn =~ s/ +/ /g;
	
	
	my @mulige_ord = split(/[^\w���]/, $tegn); #splitter p� ikke alfanumeriske tegn (\w)
	
	
	#legger til posisjonsinfo
	%termer;
	my $count = 0;
	for my $i (@mulige_ord) {
		#print "$i - $count\n";
		
		${ $termer{$i} }[$#{ $termer{$i} } +1][0] = $count;	#hvor i dokumentet
		${ $termer{$i} }[$#{ $termer{$i} }][1] = $TypePosition;	#hva for type, body, url, tittel eller....
		
		$count++;
	}

	
return %termer;
}

sub stoppord {
	my($language,$minimum_term_lengde,$maksimum_term_lengde,%mulige_ord) = @_;
	my(@ord) = ();

	########################################################################
	#sjekker om den riktige stop ord listen er lastet, hvis ikke laste vi den
	if (not exists $stoppord_liste{$language}) {
		#Pr�ver � laste listen
		my $fil = '../data/stoppord/' . $language . '.txt';
		open(INF,"$fil") or warn("Can't open $fil: $!");
		my @a = <INF>;       # reads the ENTIRE FILE into hash %b, lastet med \n p� slutten, s� \n m� v�re med nor vi samenligner
		close(INF);
		
		foreach my $i (@a) {
			chomp($i);
			$b{$i} = 1;
			#print "$i\n";
		}
		
 		$stoppord_liste{$language} = {%b};
		
		#lister opp de vi har med
		#foreach $i (keys %b) {
		#	print "$i\n";
		#}
	}
	########################################################################
	foreach my $term (keys %mulige_ord) {
		#fjerner det siste tegnet hvis det er noe annet en a-z���, slik at som pungtum, komma, osv forsvinner.
		#$term = $i->[0];
		#$posisjon = $i->[1]; 
		#$term =~ s/[^a-z���0-9]$//g;
		
		if (
				($term =~ /[^a-z���0-9]/) 						#ineholder annet en alfanumersike tegn
			|| 	($term eq '')									#er balnkt
			||	(length($term) < $minimum_term_lengde) 			#er kortere en vi vil
		    	||	(length($term) > $maksimum_term_lengde) 			#er lengere en vi vil
			||	(exists $stoppord_liste{$language}->{$term})	#i stopp ord listen
			) {
			#print "-$term-\n";
			#push(@ord,$term);
			#$ord[$#ord++][0] = $term;
			#$ord[$#ord][1] = $posisjon;
			delete $mulige_ord{$term};
		}
		
	}

	#exit;
	return %mulige_ord;
}

###################################################################################################################
# gj�r om en relativ url til en full url.
# Returnerer full url
#
#	Inn verdier
#	$BaseUrl	: urlen som ny url kom fra	Eks: http://www.domene.com/ 
#	$NyUrl		: ny url som skal resolves. Eks: ../path/side.htm 
#
#	Denne prosedyren er mye billigere � bruke en URI::URL da den ikke starter opp en hel masse, men har ikke s� mange
#	funksjoner slik at den m� kalle URI::URL for ca 1/4 av urlene. Men dette skjer automatisk uten at brukeren trenger
#	� bry seg.
###################################################################################################################
sub ResulveUrl {
	my($BaseUrl,$NyUrl) = @_;
#		if ($NyUrl =~ /^http:\/\//) { #hvis dette ikke er en ful url som begynner p� http:// og ikke  relativ som begynner p� ../
#			$FerdigUrl = $NyUrl;
#			
#		}
#		elsif ((($NyUrl !~ /^\.|^\//) || ($BaseUrl =~ /$\//)) && ($NyUrl !~ /:/)){ #hvis urlene ikke begynner p� . eller / og hovedurl begynner slutter / og ny url ikke her : i seg (https: mailto: ftp: osv) kan den legges til
#			if ($BaseUrl =~ /$\//) { #hvis vi begynner med en / taen denne bort
#				$temp = $BaseUrl;
#				chop($temp);
#				
#				#print "1 $temp / $NyUrl\n";
#				$FerdigUrl = $temp . '/' . $NyUrl;
#			}
#			else {
#				if (($NyUrl !~ /^https/) && ($NyUrl !~ /^mailto/)) {	#hvis vi verken begynner p� https eller mailto
#					#print "2 $BaseUrl $NyUrl\n";
#					$FerdigUrl = $BaseUrl . $NyUrl;
#					
#				}
#			}
#		}
#		else {
#			#$AntallUrler++;
			$link = new URI::URL $NyUrl;
			$FerdigUrl = $link->abs($BaseUrl);
#		}
#		
	return $FerdigUrl;
}
###################################################################################################################
#	sjekker hvor mange ord i en hash som ligger i data/AdultWords.txt og data/AdoldHardCoreWords.txt for � kalkulere en
#	vekt p� hvor sansynelig at dette er porno
#
#	data/AdultWords.txt - ord som ofte forekomer p� pornosider, men ogs� andre plasser, har dobbel mening
#	data/AdoldHardCoreWords.txt - ord som ikke har noen ikke porno mening
#	data/AdultFraser.txt - fraser p� 2 ord som g�rigjen mye p� porno sider
###################################################################################################################
BEGIN {

	warn("have disabled AdultWords.txt loading");
	return;

	my $adultWords;	#persitent variabel. Blir her melom hver gang vi kalle subrutinen

		#Pr�ver � laste listen
		my $fil = '../data/AdultWords.txt';
		open(INF,"$fil") or die("Can't open $fil: $!");
		my @a = <INF>;       # reads the ENTIRE FILE into hash %b, lastet med \n p� slutten, s� \n m� v�re med nor vi samenligner
		close(INF);
		
		my %b = {};
		foreach my $i (@a) {
			chomp($i);
			$b{$i} = 1;
			#print "$i\n";
		}
		
 		$adultWords{'AdultWords'} = {%b};
		
		#lister opp de vi har med
		foreach $i (keys %b) {
			#print "$i\n";
		}
		
		#Pr�ver � laste listen
		my $fil = '../data/AdultHardCoreWords.txt';
		open(INF,"$fil") or die("Can't open $fil: $!");
		my @a = <INF>;       # reads the ENTIRE FILE into hash %b, lastet med \n p� slutten, s� \n m� v�re med nor vi samenligner
		close(INF);
		
		my %b = {};
		foreach my $i (@a) {
			chomp($i);
			$b{$i} = 1;
			#print "$i\n";
		}
		
 		$adultWords{'AdoldHardCoreWords'} = {%b};
		
		#lister opp de vi har med
		#foreach $i (keys %b) {
		#	print "$i\n";
		#}
		##############################
		#	AdultFraser.txt
		#############################
		#Pr�ver � laste listen
		my $fil = '../data/AdultFraser.txt';
		open(INF,"$fil") or die("Can't open $fil: $!");
		my @a = <INF>;       # reads the ENTIRE FILE into hash %b, lastet med \n p� slutten, s� \n m� v�re med nor vi samenligner
		close(INF);
		
		my %b = {};
		foreach my $i (@a) {
			chomp($i);
			($ord1, $ord2) = split(' ',$i,2);
			push(@{ $b{$ord1} },$ord2);
			#print "$ord1, $ord2\n";
		}
		
 		$adultWords{'AdultFraser'} = {%b};
		#exit;
		
		
sub adultFilter {
	my(%TermHash) = @_;
	
	my $verdi = 0;
	#print "verdi $verdi\n";
	
	foreach my $i (keys %TermHash) {
		if (exists $adultWords{'AdoldHardCoreWords'}->{$i}) {
			#print "ah: $i\n";
			$verdi += 20;
		}
		if (exists $adultWords{'AdultWords'}->{$i}) {
			#print "an: $i\n";
			$verdi += 3;
		}
		if (exists $adultWords{'AdultFraser'}->{$i}) {
			#print "\nFins $i kombinert med ";
			#lopper gjenom aller ordene komer etter det f�rste ordet, de er deiis to ordene tilsamen sum utgj�r frasen vi leter etter
			foreach my $x (@{ $adultWords{'AdultFraser'}->{$i} }) {
			#	print "$x eller ";
				if (exists $TermHash{$x}) {	#hvis et av fraseordene ogs� fins
					foreach my $y (@{ $TermHash{$x} }) {
					#	print "\n\t$x i $y->[1], $y->[0]";
						#n� har vi funnet siste ordet i frasen, s� vi lopper gjenom og sjekker etter om det er kombinert med f�rste ord noen plass
						foreach my $h (@{ $TermHash{$i} }) {
						#	print "\nh: $i $h->[1] $h->[0]";
							#if (($h->[1] eq $y->[1]) && ($h->[0] == ($y->[0] -1))) { #trekker fra 1, da vil posisjonen v�re like hvis de st�r etter hverandre.
							if ($h->[1] eq $y->[1]) {
								my $distance = $y->[0] - $h->[0];	#finner avstand
								#print "\ndistance $distance\n";
								if ($distance == 1) {
								#print "\nfant: $i:$h->[0] $x: $y->[0] \n";
								$verdi += 20;
								#exit;
								}
							}
						}
						
						#foreach my $l (@{ $adultWords{'AdultFraser'}->{$x} }) {
						#	print "l $l $l->[0] $l->[1] ";
						#}
						#exit;
					}
				}
				#foreach my $y (@{ $TermHash{$i} }) {
				#print "\n$y->[1], $y->[0]\n";
				#}
			}
			#exit;
			
		}
	}
	
	#print "verdi $verdi\n";
	
	return $verdi;
}#sub

}#BEGIN
